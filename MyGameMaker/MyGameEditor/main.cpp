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

#include "MyGameEngine/Camera.h"
#include "MyGameEngine/Mesh.h"
#include "MyGameEngine/GameObject.h"
#include "MyWindow.h"
#include "BasicShapesManager.h"
#include "MyGui.h"
#include "SceneManager.h"
#include "Console.h"
#include "MyGameEngine/Transform.h"

using namespace std;
using hrclock = chrono::high_resolution_clock;
using vec3 = glm::dvec3;

static const glm::ivec2 WINDOW_SIZE(1280, 720);
static const unsigned int FPS = 60;
static const auto FRAME_DT = 1.0s / FPS;

static Camera camera;


SDL_Event event;
bool rightMouseButtonDown = false;
int lastMouseX, lastMouseY;

// Inicializaci�n de OpenGL
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
// Raycast desde el mouse para detectar si est� sobre un GameObject
GameObject* raycastFromMouseToGameObject(int mouseX, int mouseY, const glm::mat4& projection, const glm::mat4& view, const glm::ivec2& viewportSize) {
    glm::vec3 rayOrigin = glm::vec3(glm::inverse(view) * glm::vec4(0, 0, 0, 1));
    glm::vec3 rayDirection = getRayFromMouse(mouseX, mouseY, projection, view, viewportSize);

    GameObject* hitObject = nullptr;

    for (auto& go : SceneManager::gameObjectsOnScene) {
        if (intersectRayWithBoundingBox(rayOrigin, rayDirection, go.boundingBox())) {
            hitObject = &go;
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

void handleFileDrop(const std::string& filePath, mat4 projection, mat4 view) {
    auto extension = getFileExtension(filePath);
    auto imageTexture = std::make_shared<Image>();
    int mouseX, mouseY;
    // Obtener posici�n actual del mouse
    SDL_GetMouseState(&mouseX, &mouseY);

    if (extension == "obj" || extension == "fbx" || extension == "dae") {
       
		SceneManager::LoadGameObject(filePath);
		SceneManager::getGameObject(SceneManager::gameObjectsOnScene.size() - 1)->transform().pos() = screenToWorld(glm::vec2(mouseX, mouseY), 10.0f, projection, view);
		SceneManager::selectedObject = &SceneManager::gameObjectsOnScene.back();

    }
    else if (extension == "png" || extension == "jpg" || extension == "bmp") {
        imageTexture->loadTexture(filePath);
        // Detectar si el mouse est� sobre alg�n GameObject
        GameObject* hitObject = raycastFromMouseToGameObject(mouseX, mouseY, projection, view, WINDOW_SIZE);
        if (hitObject) {
            // Si hay un GameObject debajo del mouse, aplicar la textura
            hitObject->setTextureImage(imageTexture);
            cout << "Texture applied to GameObject under mouse." << endl;
            Console::Instance().Log("Texture applied to GameObject under mouse.");
        }
        else {
            cout << "No GameObject under mouse to apply texture." << endl;
            Console::Instance().Log("No GameObject under mouse to apply texture.");
        }
    }
    else {
        cout << "Unsupported file extension: " << extension << endl;
        Console::Instance().Log("Unsupported file extension: ");
    }
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
    glm::dmat4 viewMatrix = camera.view();

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(glm::value_ptr(projectionMatrix));
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(glm::value_ptr(viewMatrix));
}
// Funci�n de renderizado
void display_func() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    configureCamera();

    for (auto& go : SceneManager::gameObjectsOnScene) {
        if (go.isRoot()) { // Solo objetos ra�z
            // Dibuja el objeto ra�z (y sus hijos autom�ticamente desde GameObject::draw)
            go.draw();
            drawBoundingBox(go.boundingBox());
        }
    }

    // Otros elementos de la escena, como la cuadr�cula, etc.
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
    float distance = glm::length(camera.transform().pos() - target);

    vec3 newPosition;
    newPosition.x = target.x + distance * cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newPosition.y = target.y + distance * sin(glm::radians(pitch));
    newPosition.z = target.z + distance * sin(glm::radians(yaw)) * cos(glm::radians(pitch));

   
    camera.transform().pos() = newPosition;
    camera.transform().lookAt(target); 
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

        camera.transform().rotate(glm::radians(-deltaX * sensitivity), glm::vec3(0, 1, 0));
        camera.transform().rotate(glm::radians(deltaY * sensitivity), glm::vec3(1, 0, 0));

        lastMouseX = x;
        lastMouseY = y;
        camera.transform().alignCamera();
    }

    if (rightMouseButtonDown && altKeyDown) {
       
        int deltaX = x - lastMouseX;
        int deltaY = y - lastMouseY;

        if (!altPressedOnce) {
            altPressedOnce = true;

            glm::vec2 mouseScreenPos = getMousePosition();

            glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WINDOW_SIZE.x / WINDOW_SIZE.y, 0.1f, 100.0f);
            glm::mat4 view = camera.view();
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
        camera.transform().alignCamera();
	}
	else {
		altPressedOnce = false; // Reinicia la bandera si Alt no est� presionado
        /*
        
        TO DO: ARREGLAR BOOL QUE NO SE EJECUTE CADA FRAME
        
        
        */
	}
    
}

void removeObjectFromScene(const std::string& objectName) {
    auto& objects = SceneManager::gameObjectsOnScene;
    objects.erase(std::remove_if(objects.begin(), objects.end(),
        [&objectName](GameObject obj) {
            return obj.getName() == objectName;
        }), objects.end());
}

void extractTransformFromMatrix(
    const glm::mat4& matrix,
    glm::vec3& translation,
    glm::quat& rotation,
    glm::vec3& scale
) {
    // Extraer la traslaci�n directamente de la matriz
    translation = glm::vec3(matrix[3]);

    // Extraer escala como la longitud de los vectores de las columnas de la matriz
    scale.x = glm::length(glm::vec3(matrix[0])); // Longitud del eje X
    scale.y = glm::length(glm::vec3(matrix[1])); // Longitud del eje Y
    scale.z = glm::length(glm::vec3(matrix[2])); // Longitud del eje Z

    // Normalizar las columnas para extraer la rotaci�n
    glm::mat3 rotationMatrix = glm::mat3(matrix);
    rotationMatrix[0] /= scale.x; // Normalizar eje X
    rotationMatrix[1] /= scale.y; // Normalizar eje Y
    rotationMatrix[2] /= scale.z; // Normalizar eje Z

    // Convertir la matriz de rotaci�n en un cuaterni�n
    rotation = glm::quat_cast(rotationMatrix);
}
double quatToDouble(const glm::quat& rotation) {
    return glm::degrees(glm::eulerAngles(rotation).y);
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
            // Obtener el objeto ra�z (root) y el objeto seleccionado
            GameObject* root = &SceneManager::gameObjectsOnScene[0];
            GameObject* selected = SceneManager::selectedObject;
			string objectName = selected->getName();

            // Parentear el objeto seleccionado al root
            root->emplaceChild(*selected);

            // Guardar la transformaci�n global original del objeto seleccionado
            glm::vec3 originalWorldPos = selected->worldTransform().pos();
            int xpos = originalWorldPos.x;
            int ypos = originalWorldPos.y;
            int zpos = originalWorldPos.z;

            glm::mat4 parentWorldInverse = glm::inverse(root->worldTransform().mat());
            glm::vec4 localPosition = parentWorldInverse * glm::vec4(originalWorldPos, 1.0f);

            

            // Actualizar la transformaci�n local del objeto seleccionado
            selected->transform().translate(originalWorldPos);

            // Eliminar el objeto de la lista de objetos ra�z en la escena
            auto& objects = SceneManager::gameObjectsOnScene;
            objects.erase(
                std::remove_if(objects.begin(), objects.end(),
                    [&](const GameObject& obj) { return &obj == selected; }),
                objects.end()
            );

            // Confirmar la operaci�n
            std::cout << "Object parented to root and position maintained." << std::endl;
        }
        else {
            // Mensaje de error si no se cumplen las condiciones
            std::cout << "No selected object or no objects in scene." << std::endl;
        }
    } 
    if (state[SDL_SCANCODE_U]) {
        // Verificar si hay un objeto seleccionado y al menos un objeto en la escena
        if (!SceneManager::gameObjectsOnScene.empty() && SceneManager::selectedObject) {

            // Obtener el objeto seleccionado
            GameObject* selected = SceneManager::selectedObject;

            for (auto& child : selected->children()) {
                // A�adir cada child a la lista de todos los objetos en la escena
                SceneManager::gameObjectsOnScene.push_back(child);
				//selected->removeChild(child);
                
            }

            // Confirmar la operaci�n
            std::cout << "All children of the selected object have been unparented and added to the scene." << std::endl;
              
        }
        else {
            // Mensaje de error si no se cumplen las condiciones
            std::cout << "No selected object or no objects in scene." << std::endl;
        }
    }






    if (state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT]) {
        move_speed = 0.2f;
    }
    if (rightMouseButtonDown) {

        if (state[SDL_SCANCODE_W]) {
            std::cout << "Moving camera forward." << std::endl;
            camera.transform().translate(glm::vec3(0, 0, move_speed));
            Console::Instance().Log("Moving camera forward.");
        }
        if (state[SDL_SCANCODE_S]) {
            std::cout << "Moving camera backward." << std::endl;
            camera.transform().translate(glm::vec3(0, 0, -move_speed));
            Console::Instance().Log("Moving camera backward.");
        }
        if (state[SDL_SCANCODE_A]) {
            std::cout << "Moving camera left." << std::endl;
            camera.transform().translate(glm::vec3(move_speed, 0, 0));
            Console::Instance().Log("Moving camera left.");
        }
        if (state[SDL_SCANCODE_D]) {
            std::cout << "Moving camera right." << std::endl;
            camera.transform().translate(glm::vec3(-move_speed, 0, 0));
            Console::Instance().Log("Moving camera right.");
        }
        if (state[SDL_SCANCODE_Q]) {
            std::cout << "Moving camera up." << std::endl;
            camera.transform().translate(glm::vec3(0, move_speed, 0));
            Console::Instance().Log("Moving camera up.");
        }
        if (state[SDL_SCANCODE_E]) {
            std::cout << "Moving camera down." << std::endl;
            camera.transform().translate(glm::vec3(0, -move_speed, 0));
            Console::Instance().Log("Moving camera down.");
        }
    }

    if (state[SDL_SCANCODE_F] && !fKeyDown && SceneManager::selectedObject != NULL) {
        camera.transform().pos() = SceneManager::selectedObject->transform().pos() + vec3(0, 1, 4);
        fKeyDown = true;
        camera.transform().lookAt(SceneManager::selectedObject->transform().pos());
        std::cout << "Camera looking at target." << std::endl;
    }
    else if (!state[SDL_SCANCODE_F]) {
        fKeyDown = false;
    }
    camera.transform().alignCamera();
}
void mouseWheel_func(int direction) {
    camera.transform().translate(vec3(0, 0, direction * 0.1));
}
//debug, showing the bounding boxes, not finished


int main(int argc, char* argv[]) {
    ilInit();
    iluInit();

    MyWindow window("SDL2 Simple Example", WINDOW_SIZE.x, WINDOW_SIZE.y);
    MyGUI gui(window.windowPtr(), window.contextPtr());
    initOpenGL();

    // Posici�n inicial de la c�mara
    camera.transform().pos() = vec3(0, 1, 4);
    camera.transform().rotate(glm::radians(180.0), vec3(0, 1, 0));
    SceneManager::spawnBakerHouse();

    while (window.isOpen()) {
        const auto t0 = hrclock::now();
		handleAltKey();
        // Obtener la posici�n actual del mouse
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
            // Obtener matrices de proyecci�n y vista de la c�mara
            glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WINDOW_SIZE.x / WINDOW_SIZE.y, 0.1f, 100.0f);
            glm::mat4 view = camera.view();
          
            gui.processEvent(event);
            
            switch (event.type)
            {
			case SDL_QUIT:
				window.close();
				break;
            case SDL_DROPFILE:               
				cout << "File dropped: " << event.drop.file << endl;
                handleFileDrop(event.drop.file, projection, view);
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

                // Crear figuras en la posici�n 3D calculada
                switch (event.key.keysym.sym) {
                case SDLK_1:  // Crear Tri�ngulo
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
                default:
                    break;
                }
                break;
			default:
				cout << event.type << endl;
				break;
            }



        }
        idle_func();    // Actualizar l�gica de juego
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