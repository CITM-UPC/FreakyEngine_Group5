#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp> 
#include <glm/gtx/orthonormalize.hpp>
#include <iostream>
#include <string>
#include <IL/il.h>
#include <IL/ilu.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <chrono>
#include <thread>
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "MyGameEngine/Camera.h"
#include "MyGameEngine/Mesh.h"
#include "MyGameEngine/GameObject.h"
#include "MyWindow.h"
#include "BasicShapesManager.h"
#include "MyGui.h"
#include "SceneManager.h"
#include "Console.h"
#include "FileDropHandler.h"

using namespace std;
using hrclock = chrono::high_resolution_clock;
using vec3 = glm::dvec3;

static const glm::ivec2 WINDOW_SIZE(1280, 720);
static const unsigned int FPS = 60;
static const auto FRAME_DT = 1.0s / FPS;

//static Camera camera;
GameObject mainCamera("Main Camera");
GameObject testCamera("Test Camera");

SDL_Event event;
bool rightMouseButtonDown = false;
int lastMouseX, lastMouseY;
FileDropHandler fileDropHandler;
// Inicialización de OpenGL
void initOpenGL() {
    glewInit();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
}


//Mouse relative positions
glm::vec2 getMousePosition() {
    int x, y;
    SDL_GetMouseState(&x, &y);
    return glm::vec2(static_cast<float>(x), static_cast<float>(y));
}

//Funcion para convertir de coordenadas de pantalla a coordenadas del mundo
glm::vec3 screenToWorld(const glm::vec2& mousePos, float depth, const glm::mat4& projection, const glm::mat4& view) {

    float x = (2.0f * mousePos.x) / WINDOW_SIZE.x - 1.0f;
    float y = 1.0f - (2.0f * mousePos.y) / WINDOW_SIZE.y;  
    glm::vec4 clipCoords(x, y, -1.0f, 1.0f); 


    glm::vec4 eyeCoords = glm::inverse(projection) * clipCoords;
    eyeCoords = glm::vec4(eyeCoords.x, eyeCoords.y, -1.0f, 0.0f);


    glm::vec3 worldRay = glm::vec3(glm::inverse(view) * eyeCoords);
    worldRay = glm::normalize(worldRay);


    glm::vec3 cameraPosition = glm::vec3(glm::inverse(view)[3]); 
    return cameraPosition + worldRay * depth;
}



//RayCastFunctions
glm::vec3 getRayFromMouse(int mouseX, int mouseY, const glm::mat4& projection, const glm::mat4& view, const glm::ivec2& viewportSize) {
    float x = (2.0f * mouseX) / viewportSize.x - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / viewportSize.y;
    glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);

    glm::vec4 rayEye = glm::inverse(projection) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    glm::vec3 rayWorld = glm::normalize(glm::vec3(glm::inverse(view) * rayEye));
    return rayWorld;
}


bool intersectRayWithBoundingBox(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const BoundingBox& bbox) {
    float tmin = (bbox.min.x - rayOrigin.x) / rayDirection.x;
    float tmax = (bbox.max.x - rayOrigin.x) / rayDirection.x;

    if (tmin > tmax) std::swap(tmin, tmax);

    float tymin = (bbox.min.y - rayOrigin.y) / rayDirection.y;
    float tymax = (bbox.max.y - rayOrigin.y) / rayDirection.y;

    if (tymin > tymax) std::swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax)) {
        return false;
    }

    if (tymin > tmin) tmin = tymin;
    if (tymax < tmax) tmax = tymax;

    float tzmin = (bbox.min.z - rayOrigin.z) / rayDirection.z;
    float tzmax = (bbox.max.z - rayOrigin.z) / rayDirection.z;

    if (tzmin > tzmax) std::swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax)) {
        return false;
    }

    if (tzmin > tmin) tmin = tzmin;
    if (tzmax < tmax) tmax = tzmax;

    return tmin >= 0.0f;
}
// Raycast desde el mouse para detectar si está sobre un GameObject
GameObject* raycastFromMouseToGameObject(int mouseX, int mouseY, const glm::mat4& projection, const glm::mat4& view, const glm::ivec2& viewportSize) {
    glm::vec3 rayOrigin = glm::vec3(glm::inverse(view) * glm::vec4(0, 0, 0, 1));
    glm::vec3 rayDirection = getRayFromMouse(mouseX, mouseY, projection, view, viewportSize);

    GameObject* hitObject = nullptr;

    for (auto& go : SceneManager::gameObjectsOnScene) {
        // Transformar bounding box del padre al espacio global
        BoundingBox globalBoundingBox = go.worldTransform().mat() * go.localBoundingBox();
        if (intersectRayWithBoundingBox(rayOrigin, rayDirection, globalBoundingBox)) {
            hitObject = &go;

            // Verificar los hijos en el espacio global
            for (auto& child : go.children()) {
                BoundingBox childGlobalBoundingBox = child.worldTransform().mat() * child.localBoundingBox();
                if (intersectRayWithBoundingBox(rayOrigin, rayDirection, childGlobalBoundingBox)) {
                    hitObject = &child;
                    break;
                }
            }
            break;
        }
    }
    return hitObject;
}



//File drop handler
std::string getFileExtension(const std::string& filePath) {
    // Find the last dot in the file path
    size_t dotPosition = filePath.rfind('.');

    // If no dot is found, return an empty string
    if (dotPosition == std::string::npos) {
        return "";
    }

    // Extract and return the file extension
    return filePath.substr(dotPosition + 1);
}



//Renderizado del suelo
static void drawFloorGrid(int size, double step) {
    glColor3ub(0, 0, 0);
    glBegin(GL_LINES);
    for (double i = -size; i <= size; i += step) {
        glVertex3d(i, 0, -size);
        glVertex3d(i, 0, size);
        glVertex3d(-size, 0, i);
        glVertex3d(size, 0, i);
    }
    glEnd();
}

inline static void glVertex3(const vec3& v) { glVertex3dv(&v.x); }
static void drawWiredQuad(const vec3& v0, const vec3& v1, const vec3& v2, const vec3& v3) {
    glBegin(GL_LINE_LOOP);
    glVertex3(v0);
    glVertex3(v1);
    glVertex3(v2);
    glVertex3(v3);
    glEnd();
}
static void drawBoundingBox(const BoundingBox& bbox) {
    glLineWidth(2.0);
    drawWiredQuad(bbox.v000(), bbox.v001(), bbox.v011(), bbox.v010());
    drawWiredQuad(bbox.v100(), bbox.v101(), bbox.v111(), bbox.v110());
    drawWiredQuad(bbox.v000(), bbox.v001(), bbox.v101(), bbox.v100());
    drawWiredQuad(bbox.v010(), bbox.v011(), bbox.v111(), bbox.v110());
    drawWiredQuad(bbox.v000(), bbox.v010(), bbox.v110(), bbox.v100());
    drawWiredQuad(bbox.v001(), bbox.v011(), bbox.v111(), bbox.v101());
}

void configureCamera() {
    glm::dmat4 projectionMatrix = glm::perspective(glm::radians(45.0), static_cast<double>(WINDOW_SIZE.x) / WINDOW_SIZE.y, 0.1, 100.0);
    //glm::dmat4 viewMatrix = camera.view();
	glm::dmat4 viewMatrix = mainCamera.GetComponent<CameraComponent>()->camera().view();
	testCamera.AddComponent<CameraComponent>();
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(glm::value_ptr(projectionMatrix));
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(glm::value_ptr(viewMatrix));
}
void drawFrustum(const GameObject& camera) {
    //auto planes = camera.frustumPlanes();
	auto planes = camera.GetComponent<CameraComponent>()->camera().frustumPlanes();
    std::vector<glm::vec3> frustumCorners = {
        glm::vec3(-1, -1, -1), glm::vec3(1, -1, -1),
        glm::vec3(1, 1, -1), glm::vec3(-1, 1, -1),
        glm::vec3(-1, -1, 1), glm::vec3(1, -1, 1),
        glm::vec3(1, 1, 1), glm::vec3(-1, 1, 1)
    };

    glm::mat4 invProjView = glm::inverse(camera.GetComponent<CameraComponent>()->camera().projection() * camera.GetComponent<CameraComponent>()->camera().view());
    for (auto& corner : frustumCorners) {
        glm::vec4 transformedCorner = invProjView * glm::vec4(corner, 1.0f);
        corner = glm::vec3(transformedCorner) / transformedCorner.w;
    }

    glBegin(GL_LINES);
    for (int i = 0; i < 4; ++i) {
        glVertex3fv(glm::value_ptr(frustumCorners[i]));
        glVertex3fv(glm::value_ptr(frustumCorners[(i + 1) % 4]));
        glVertex3fv(glm::value_ptr(frustumCorners[i + 4]));
        glVertex3fv(glm::value_ptr(frustumCorners[(i + 1) % 4 + 4]));
        glVertex3fv(glm::value_ptr(frustumCorners[i]));
        glVertex3fv(glm::value_ptr(frustumCorners[i + 4]));
    }
    glEnd();
}
bool isInsideFrustum(const BoundingBox& bbox, const std::list<Plane>& frustumPlanes) {
    for (const auto& plane : frustumPlanes) {
        if (plane.distance(bbox.v000()) < 0 &&
            plane.distance(bbox.v001()) < 0 &&
            plane.distance(bbox.v010()) < 0 &&
            plane.distance(bbox.v011()) < 0 &&
            plane.distance(bbox.v100()) < 0 &&
            plane.distance(bbox.v101()) < 0 &&
            plane.distance(bbox.v110()) < 0 &&
            plane.distance(bbox.v111()) < 0) {
            return false;
        }
    }
    return true;
}
// Función de renderizado
void display_func() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    configureCamera();
	drawFrustum(testCamera); // Dibujar el frustum de la cámara de prueba (testCamera
    testCamera.GetComponent<CameraComponent>()->camera().transform() = testCamera.GetComponent<TransformComponent>()->transform();
    auto frustumPlanes = testCamera.GetComponent<CameraComponent>()->camera().frustumPlanes();
    
    for (auto& go : SceneManager::gameObjectsOnScene) {
        if (go.isRoot()) { // Solo objetos raíz
            // Verificar si el objeto está dentro del frustum
            if (isInsideFrustum(go.boundingBox(), frustumPlanes)) {
                // Dibuja el objeto raíz (y sus hijos automáticamente desde GameObject::draw)
                go.draw();
                drawBoundingBox(go.boundingBox());
            }
        }
    }

    // Otros elementos de la escena, como la cuadrícula, etc.
    drawFloorGrid(16, 0.25);
}




// Funciones de manejo de mouse
void mouseButton_func(int button, int state, int x, int y) {
    if (button == SDL_BUTTON_RIGHT) {
        rightMouseButtonDown = (state == SDL_PRESSED);
        lastMouseX = x;
        lastMouseY = y;
    }
}

//Camera rotation
float yaw = 0.0f;
float pitch = 0.0f;
const float MAX_PITCH = 89.0f;
bool altKeyDown = false;
bool altPressedOnce = false;
vec3 target;

void handleAltKey() {
    const Uint8* state = SDL_GetKeyboardState(NULL);
    altKeyDown = state[SDL_SCANCODE_LALT] || state[SDL_SCANCODE_RALT];
}

void orbitCamera(const vec3& target, int deltaX, int deltaY) {
    const float sensitivity = 0.1f;

    yaw += deltaX * sensitivity;
    pitch -= deltaY * sensitivity;
    float distance = glm::length(mainCamera.GetComponent<CameraComponent>()->camera().transform().pos() - target);

    vec3 newPosition;
    newPosition.x = target.x + distance * cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newPosition.y = target.y + distance * sin(glm::radians(pitch));
    newPosition.z = target.z + distance * sin(glm::radians(yaw)) * cos(glm::radians(pitch));

   
    mainCamera.GetComponent<CameraComponent>()->camera().transform().pos() = newPosition;
    mainCamera.GetComponent<CameraComponent>()->camera().transform().lookAt(target);
}
void mouseMotion_func(int x, int y) {
    if (rightMouseButtonDown && altKeyDown == false) {
        int deltaX = x - lastMouseX;
        int deltaY = y - lastMouseY;

        const double sensitivity = 0.1;

        yaw += deltaX * sensitivity;
        pitch -= deltaY * sensitivity;

        if (pitch > MAX_PITCH) pitch = MAX_PITCH;
        if (pitch < -MAX_PITCH) pitch = -MAX_PITCH;

        mainCamera.GetComponent<CameraComponent>()->camera().transform().rotate(glm::radians(-deltaX * sensitivity), glm::vec3(0, 1, 0));
        mainCamera.GetComponent<CameraComponent>()->camera().transform().rotate(glm::radians(deltaY * sensitivity), glm::vec3(1, 0, 0));

        lastMouseX = x;
        lastMouseY = y;
        mainCamera.GetComponent<CameraComponent>()->camera().transform().alignCamera();
    }

    if (rightMouseButtonDown && altKeyDown) {
       
        int deltaX = x - lastMouseX;
        int deltaY = y - lastMouseY;

        if (!altPressedOnce) {
            altPressedOnce = true;

            glm::vec2 mouseScreenPos = getMousePosition();

            glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WINDOW_SIZE.x / WINDOW_SIZE.y, 0.1f, 100.0f);
            glm::mat4 view = mainCamera.GetComponent<CameraComponent>()->camera().view();
			if (SceneManager::selectedObject != nullptr) {
				target = SceneManager::selectedObject->transform().pos();

			}
			else {
				target = glm::vec3(0,0,0);
			}
            orbitCamera(target, deltaX, deltaY);
		}
        else {
            orbitCamera(target, deltaX, deltaY);
        }
        lastMouseX = x;
        lastMouseY = y;
        mainCamera.GetComponent<CameraComponent>()->camera().transform().alignCamera();
	}
	else {
		altPressedOnce = false; // Reinicia la bandera si Alt no está presionado
        /*
        
        TO DO: ARREGLAR BOOL QUE NO SE EJECUTE CADA FRAME
        
        
        */
	}
    
}

bool fKeyDown = false;
static void idle_func() {
    float move_speed = 0.1f;
    const Uint8* state = SDL_GetKeyboardState(NULL);

    //Debug to rotate the initial baker house and parent it 
    if (state[SDL_SCANCODE_T]) {

        SceneManager::gameObjectsOnScene[0].transform().rotate(glm::radians(10.0f), glm::vec3(0, -1, 0));
        //SceneManager::gameObjectsOnScene[0].children().front().transform().rotate(glm::radians(10.0f), glm::vec3(0, -1, 0));
    }

    if (state[SDL_SCANCODE_I]) {
        // Verificar si hay un objeto seleccionado y al menos un objeto en la escena
        if (!SceneManager::gameObjectsOnScene.empty() && SceneManager::selectedObject) {

                glm::mat4 parentWorldInverse = glm::inverse(SceneManager::gameObjectsOnScene[0].worldTransform().mat()); // Matriz inversa del padre
                glm::mat4 selectedWorldTransform = SceneManager::selectedObject->worldTransform().mat();        // Transformación mundial del seleccionado
                glm::mat4 localTransform = parentWorldInverse * selectedWorldTransform;     // Transformación local al padre

                // Desparentar del padre anterior (si aplica)
                

                // Parentear al nuevo padre
                SceneManager::gameObjectsOnScene[0].emplaceChild(*SceneManager::selectedObject);

                // Actualizar la transformación local del seleccionado
                //selected->transform().mat() = localTransform; // Ajustar su transformación local al nuevo padre

                std::cout << "Object parented to root." << std::endl;

        }
        else {
            std::cout << "No selected object or no objects in scene." << std::endl;
        }
    }
   
    if (state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT]) {
        move_speed = 0.2f;
    }
    if (rightMouseButtonDown) {

        if (state[SDL_SCANCODE_W]) {
            std::cout << "Moving camera forward." << std::endl;
            mainCamera.GetComponent<CameraComponent>()->camera().transform().translate(glm::vec3(0, 0, move_speed));
            Console::Instance().Log("Moving camera forward.");
        }
        if (state[SDL_SCANCODE_S]) {
            std::cout << "Moving camera backward." << std::endl;
            mainCamera.GetComponent<CameraComponent>()->camera().transform().translate(glm::vec3(0, 0, -move_speed));
            Console::Instance().Log("Moving camera backward.");
        }
        if (state[SDL_SCANCODE_A]) {
            std::cout << "Moving camera left." << std::endl;
            mainCamera.GetComponent<CameraComponent>()->camera().transform().translate(glm::vec3(move_speed, 0, 0));
            Console::Instance().Log("Moving camera left.");
        }
        if (state[SDL_SCANCODE_D]) {
            std::cout << "Moving camera right." << std::endl;
            mainCamera.GetComponent<CameraComponent>()->camera().transform().translate(glm::vec3(-move_speed, 0, 0));
            Console::Instance().Log("Moving camera right.");
        }
        if (state[SDL_SCANCODE_Q]) {
            std::cout << "Moving camera up." << std::endl;
            //mainCamera.transform().translate(glm::vec3(0, move_speed, 0));
			mainCamera.GetComponent<CameraComponent>()->camera().transform().translate(glm::vec3(0, move_speed, 0));
            Console::Instance().Log("Moving camera up.");
        }
        if (state[SDL_SCANCODE_E]) {
            std::cout << "Moving camera down." << std::endl;
            mainCamera.GetComponent<CameraComponent>()->camera().transform().translate(glm::vec3(0, -move_speed, 0));
            Console::Instance().Log("Moving camera down.");
        }
    }

    if (state[SDL_SCANCODE_F] && !fKeyDown && SceneManager::selectedObject != NULL) {
        mainCamera.GetComponent<CameraComponent>()->camera().transform().pos() = SceneManager::selectedObject->transform().pos() + vec3(0, 1, 4);
        fKeyDown = true;
        mainCamera.GetComponent<CameraComponent>()->camera().transform().lookAt(SceneManager::selectedObject->transform().pos());
        std::cout << "Camera looking at target." << std::endl;
    }
    else if (!state[SDL_SCANCODE_F]) {
        fKeyDown = false;
    }
    mainCamera.GetComponent<CameraComponent>()->camera().transform().alignCamera();
}
void mouseWheel_func(int direction) {
    mainCamera.GetComponent<CameraComponent>()->camera().transform().translate(vec3(0, 0, direction * 0.1));
}
//debug, showing the bounding boxes, not finished


int main(int argc, char* argv[]) {
    ilInit();
    iluInit();

    MyWindow window("SDL2 Simple Example", WINDOW_SIZE.x, WINDOW_SIZE.y);
    MyGUI gui(window.windowPtr(), window.contextPtr());
    initOpenGL();

   

    // Posición inicial de la cámara
    mainCamera.name = "Main Camera";
    mainCamera.AddComponent<CameraComponent>();
	SceneManager::gameObjectsOnScene.push_back(mainCamera);
    mainCamera.GetComponent<CameraComponent>()->camera().transform().pos() = vec3(0, 1, 4);
    mainCamera.GetComponent<CameraComponent>()->camera().transform().rotate(glm::radians(180.0), vec3(0, 1, 0));

    testCamera.name = "Test Camera";
    testCamera.AddComponent<CameraComponent>();
    SceneManager::gameObjectsOnScene.push_back(testCamera);
    testCamera.GetComponent<CameraComponent>()->camera().transform().pos() = vec3(0, 1, 4);
    testCamera.GetComponent<CameraComponent>()->camera().transform().rotate(glm::radians(180.0), vec3(0, 1, 0));
    SceneManager::spawnBakerHouse();

    while (window.isOpen()) {
        const auto t0 = hrclock::now();
		handleAltKey();
        // Obtener la posición actual del mouse
        glm::vec2 mouseScreenPos = getMousePosition();       

        display_func(); // Renderizar la escena
        gui.render();
        window.swapBuffers();

        const auto t1 = hrclock::now();
        const auto dt = t1 - t0;
        if (dt < FRAME_DT) this_thread::sleep_for(FRAME_DT - dt);

        SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
        
        while (SDL_PollEvent(&event))
        {
            // Obtener matrices de proyección y vista de la cámara
            glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WINDOW_SIZE.x / WINDOW_SIZE.y, 0.1f, 100.0f);
            glm::mat4 view = mainCamera.GetComponent<CameraComponent>()->camera().view();
          
            gui.processEvent(event);
            
            switch (event.type)
            {
			case SDL_QUIT:
				window.close();
				break;
            case SDL_DROPFILE:               
				cout << "File dropped: " << event.drop.file << endl;
                fileDropHandler.handleFileDrop(event.drop.file, projection, view);
                SDL_free(event.drop.file);
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    // Raycast para detectar el objeto debajo del mouse
                    SceneManager::selectedObject = raycastFromMouseToGameObject(mouseScreenPos.x, mouseScreenPos.y, projection, view, WINDOW_SIZE);
                }
            case SDL_MOUSEBUTTONUP:
                mouseButton_func(event.button.button, event.button.state, event.button.x, event.button.y);
                break;
            case SDL_MOUSEMOTION:
                mouseMotion_func(event.motion.x, event.motion.y);
                break;
            case SDL_MOUSEWHEEL:
                mouseWheel_func(event.wheel.y);
                break;
            case SDL_KEYDOWN:
                glm::vec3 mouseWorldPos = screenToWorld(mouseScreenPos, 10.0f, projection, view);

                // Crear figuras en la posición 3D calculada
                switch (event.key.keysym.sym) {
                case SDLK_1:  // Crear Triángulo
                    BasicShapesManager::createFigure(1, SceneManager::gameObjectsOnScene, 1.0, mouseWorldPos);
                    SceneManager::selectedObject = &SceneManager::gameObjectsOnScene.back();
                    break;
                case SDLK_2:  // Crear Cuadrado
                    BasicShapesManager::createFigure(2, SceneManager::gameObjectsOnScene, 1.0, mouseWorldPos);
                    SceneManager::selectedObject = &SceneManager::gameObjectsOnScene.back();
                    break;
                case SDLK_3:  // Crear Cubo
                    BasicShapesManager::createFigure(3, SceneManager::gameObjectsOnScene, 1.0, mouseWorldPos);
                    SceneManager::selectedObject = &SceneManager::gameObjectsOnScene.back();
                    break;
                case SDLK_DELETE:
                    if (SceneManager::selectedObject) 
                    {
                        SceneManager::DestroyGameObject(SceneManager::selectedObject);
                        SceneManager::selectedObject = nullptr;
                    }
                    break;
                default:
                    break;
                }
            break;
			default:
				cout << event.type << endl;
				break;
            }



        }
        idle_func();    // Actualizar lógica de juego
    }
    return EXIT_SUCCESS;
}




/*
Notes:
implement this functions for the frustrum culling
sign sideOfPlane(const vec3& point, const vec4& plane) {
    return glm::sign(glm::dot(vec4(point, 1.0), plane));
}
sign sideOfPlane(const vec4& plane, const BoundingBox& bbox) {
    const vec3& min = bbox.min;
    const vec3& max = bbox.max;

    vec3 p(min.x, min.y, min.z);
    if (sideOfPlane(p, plane) > 0) return 1;

    p = vec3(max.x, min.y, min.z);
    if (sideOfPlane(p, plane) > 0) return 1;

    p = vec3(min.x, max.y, min.z);
    if (sideOfPlane(p, plane) > 0) return 1;

    p = vec3(max.x, max.y, min.z);
    if (sideOfPlane(p, plane) > 0) return 1;

    p = vec3(min.x, min.y, max.z);
    if (sideOfPlane(p, plane) > 0) return 1;

    p = vec3(max.x, min.y, max.z);
    if (sideOfPlane(p, plane) > 0) return 1;

    p = vec3(min.x, max.y, max.z);
    if (sideOfPlane(p, plane) > 0) return 1;

    p = vec3(max.x, max.y, max.z);
    if (sideOfPlane(p, plane) > 0) return 1;

    return -1;
}

sign sideOfPlanes(const vec3& point, const std::vector<vec4>& planes) {
    sign result = 1;
    for (const auto& plane : planes) {
        if (sideOfPlane(point, plane) < 0) return -1;
    }
    return result;
}


*/