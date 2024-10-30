#include <iostream>
#include <array>
#include <vector>
#include <glm/glm.hpp>
#include <IL/il.h>
#include <IL/ilu.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Assimp/cimport.h" 
#include "Assimp/scene.h" 
#include "Assimp/postprocess.h"


#include "MyGameEngine/Camera.h"
<<<<<<< Updated upstream
#include "MyGameEngine/MyMesh.h"
=======
#include "MyGameEngine/Mesh.h"
#include "MyGameEngine/GameObject.h"
#include "MyWindow.h"
#include "BasicShapesManager.h"
#include "MyGui.h"
#include "Console.h"
#include "SceneManager.h"




>>>>>>> Stashed changes
using namespace std;

struct Triangle {
	Transform transform;
	glm::u8vec3 color;
	double size = 0.5;

	unsigned int texture_id;

	void draw() const {


		glEnable(GL_TEXTURE_2D);

		glPushMatrix(); //Save the modelview values of the matrix
		glMultMatrixd(&transform.mat()[0][0]);
		glColor3ub(color.r, color.g, color.b);
		glBegin(GL_TRIANGLES);

		glTexCoord2d(0.5, 0);
		glVertex2d(0, size);

		glTexCoord2d(0, 1);
		glVertex2d(-size, -size);

		glTexCoord2d(1, 1);
		glVertex2d(size, -size);
		glEnd();
		glPopMatrix(); //Recover the last values of the model view matrix

		glDisable(GL_TEXTURE_2D);

	}
};

struct Cube {
	Transform transform;
	glm::u8vec3 color = glm::u8vec3(111, 110, 0);
	double size = 0.5;

	vector<vec3> vertex_data = {
		vec3(-1, -1, -1),
		vec3(1, -1, -1),
		vec3(1, 1, -1),
		vec3(-1, 1, -1),
		vec3(-1, -1, 1),
		vec3(1, -1, 1),
		vec3(1, 1, 1),
		vec3(-1, 1, 1)
	};

	vector<unsigned int> index_data = {
		0, 1, 2, 0, 2, 3,
		1, 5, 6, 1, 6, 2,
		5, 4, 7, 5, 7, 6,
		4, 0, 3, 4, 3, 7,
		3, 2, 6, 3, 6, 7,
		4, 5, 1, 4, 1, 0
	};

	unsigned int vertex_data_bufer_id = 0;
	unsigned int index_data_bufer_id = 0;

	void InitBuffers() {

		vec3 vertex[123];
		sizeof(vertex);

		glGenBuffers(1, &vertex_data_bufer_id);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_data_bufer_id);
		glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(vec3), vertex_data.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &index_data_bufer_id);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_data_bufer_id);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_data.size() * sizeof(unsigned int), index_data.data(), GL_STATIC_DRAW);

	}

	void draw() const {
		glPushMatrix(); //Save the modelview values of the matrix
		glMultMatrixd(&transform.mat()[0][0]);
		glColor3ub(color.r, color.g, color.b);
		glEnableClientState(GL_VERTEX_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_data_bufer_id);
		glVertexPointer(3, GL_DOUBLE, 0, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_data_bufer_id);
		glDrawElements(GL_TRIANGLES, index_data.size(), GL_UNSIGNED_INT, 0);
		glDisableClientState(GL_VERTEX_ARRAY);

		glPopMatrix(); //Recover the last values of the model view matrix
	}

};

static Camera camera;
static Triangle red_triangle;
static Triangle green_triangle;
static Triangle blue_triangle;
static Cube cube;
static MyMesh loadedMesh;

static std::array<array<glm::u8vec3, 256>, 256> texture;

static void initializeTexture() {
	for (int i = 0; i < texture.front().size(); i++)
	{
		for (int j = 0; j < texture.size(); j++)
		{
			const glm::u8vec3 whiteColor(255, 255, 255);
			const glm::u8vec3 blackColor(0, 0, 0);
			texture[i][j] = (i / 8 + j / 8) % 2 == 0 ? whiteColor : blackColor;
		}
	}

}


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

static void display_func() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixd(&camera.view()[0][0]);

	drawFloorGrid(16, 0.25);
	//red_triangle.draw();
	//green_triangle.draw();
	//blue_triangle.draw();
	//cube.draw();
	loadedMesh.Draw();

	glutSwapBuffers();
}

static void init_opengl() {
	glewInit();

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

	glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glEnable(GL_DEPTH_TEST);

	glClearColor(0.5, 0.5, 0.5, 1.0);
}

static void reshape_func(int width, int height) {
	glViewport(0, 0, width, height);
	camera.aspect = static_cast<double>(width) / height;
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(&camera.projection()[0][0]);
}

static void mouseWheel_func(int wheel, int direction, int x, int y) {
	camera.transform().translate(vec3(0, 0, direction * 0.1));
}

static void idle_func() {
<<<<<<< Updated upstream
	//animate triangles
	red_triangle.transform.rotate(0.001, vec3(0, 1, 0));
	green_triangle.transform.rotate(0.001, vec3(1, 0, 0));
	blue_triangle.transform.rotate(0.001, vec3(0, 0, 1));
	cube.transform.rotate(0.001, vec3(0, 1, 1));
	glutPostRedisplay();
=======
    float move_speed = 0.1f;
    const Uint8* state = SDL_GetKeyboardState(NULL);


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
        Console::Instance().Log("Camera looking at target.");

    }
    else if (!state[SDL_SCANCODE_F]) {
        fKeyDown = false;
    }
    camera.transform().alignCamera();
}
//debug, showing the bounding boxes, not finished
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

void mouseWheel_func(int direction) {
    camera.transform().translate(vec3(0, 0, direction * 0.1));
>>>>>>> Stashed changes
}




int main(int argc, char* argv[]) {
<<<<<<< Updated upstream
	// Iniit window and context
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(1280, 720);
	glutCreateWindow("Glut Simple Example");
=======
    ilInit();
    iluInit();
	
>>>>>>> Stashed changes

	// Init OpenGL
	init_opengl();

<<<<<<< Updated upstream
	// Init camera
	camera.transform().pos() = vec3(0, 1, 4);
	camera.transform().rotate(glm::radians(180.0), vec3(0, 1, 0));

	loadedMesh.LoadFile("Bakerhouse.fbx");
	loadedMesh.LoadTexture("Baker_House.png");
	loadedMesh.InitBuffers();
	
	if (loadedMesh.num_vertex == 0 || loadedMesh.num_index == 0) {
		std::cout << "Error: No se pudo cargar la malla correctamente." << std::endl;
		return EXIT_FAILURE;
	}
=======
 

    // Posición inicial de la cámara
    camera.transform().pos() = vec3(0, 1, 4);
    camera.transform().rotate(glm::radians(180.0), vec3(0, 1, 0));
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

                // Crear figuras en la posición 3D calculada
                switch (event.key.keysym.sym) {

                case SDLK_1:  // Crear Triángulo
                    BasicShapesManager::createFigure(1, SceneManager::gameObjectsOnScene, 1.0, mouseWorldPos);
                    break;
                case SDLK_2:  // Crear Cuadrado

                    BasicShapesManager::createFigure(2, SceneManager::gameObjectsOnScene, 1.0, mouseWorldPos);
                    break;
                case SDLK_3:  // Crear Cubo

                    BasicShapesManager::createFigure(3, SceneManager::gameObjectsOnScene, 1.0, mouseWorldPos);
                    break;
                default:
                    break;
                }
                break;
			default:
				cout << event.type << endl;
				break;
            }
>>>>>>> Stashed changes


	//Init cube
	cube.InitBuffers();

	// Init triangles
	red_triangle.transform.pos() = vec3(0, 1, 0);
	red_triangle.color = glm::u8vec3(255, 0, 0);
	green_triangle.transform.pos() = vec3(1, 1, 0);
	green_triangle.color = glm::u8vec3(0, 255, 0);
	blue_triangle.transform.pos() = vec3(0, 1, 1);
	blue_triangle.color = glm::u8vec3(0, 0, 255);


	//ilInit();
	//iluInit();
	//auto il_img_id = ilGenImage();
	//ilBindImage(il_img_id);
	//ilLoadImage("Lenna.png");

	//auto img_width = ilGetInteger(IL_IMAGE_WIDTH);
	//auto img_height = ilGetInteger(IL_IMAGE_HEIGHT);
	//auto img_bpp = ilGetInteger(IL_IMAGE_BPP);
	//auto img_format = ilGetInteger(IL_IMAGE_FORMAT);
	//auto img_data = ilGetData();

	////Init Texture
	//initializeTexture();
	//unsigned int texture_id = 0;
	//glGenTextures(1, &texture_id);
	//glBindTexture(GL_TEXTURE_2D, texture_id);
	//glTexImage2D(GL_TEXTURE_2D, 0, img_bpp, img_width, img_height, 0, img_format, GL_UNSIGNED_BYTE, img_data);
	//ilDeleteImage(il_img_id);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_NEAREST);

	// Set Glut callbacks
	glutDisplayFunc(display_func);
	glutIdleFunc(idle_func);
	glutReshapeFunc(reshape_func);
	glutMouseWheelFunc(mouseWheel_func); //Mouse wheel movement 

	// Enter glut main loop
	glutMainLoop();

	return EXIT_SUCCESS;
}