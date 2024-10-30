#include "MyGUI.h"
#include "MyGameEngine/GameObject.h"
#include "SceneManager.h"


#include <imgui.h>

#include <glm/gtc/matrix_transform.hpp>

#include <glm/glm.hpp>
#include "Main.h"
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <tinyfiledialogs/tinyfiledialogs.h> 
 
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_opengl.h>
#include <MyGameEngine/Camera.h>

bool show_metrics_window = false;
static const glm::ivec2 WINDOW_SIZE(1280, 720);
static Camera camera;

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include "Console.h"
#include "BasicShapesManager.h"

MyGUI::MyGUI(SDL_Window* window, void* context) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init();



}

MyGUI::~MyGUI() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

void MyGUI::ShowMainMenuBar() {


    if (show_metrics_window) {
        ShowMetricsWindow(&show_metrics_window);


    }


    if (ImGui::BeginMainMenuBar()) 
    {
        if (ImGui::BeginMenu("File")) 
        {
            if (ImGui::BeginMenu("Import"))
            {
                if (ImGui::MenuItem("FBX")) 
                {
                    const char* filterPatterns[1] = { "*.fbx" };
                    const char* filePath = tinyfd_openFileDialog(
                        "Select an FBX file",
                        "",
                        1,
                        filterPatterns,
                        NULL,
                        0
                    );
                    if (filePath) 
                    {
                        SceneManager::LoadGameObject(filePath);
                    }
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Quit")) 
            {
                SDL_Quit();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("About"))
            {
                const char* url = "https://github.com/CITM-UPC/FreakyEngine_Group5";
                SDL_OpenURL(url);
            }

            ImGui::Checkbox("Metrics", &show_metrics_window);


            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

float GetMemoryUsage() {
    // Replace this with actual memory usage calculation
    // Example: on Windows, you could use `PROCESS_MEMORY_COUNTERS` with `GetProcessMemoryInfo`
    return static_cast<float>(rand() % 2000);  // Random value between 0 and 2000 MB for demonstration
}



void MyGUI::ShowConsole() {

    ImGui::SetNextWindowSize(ImVec2(600, 200), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(300, ImGui::GetIO().DisplaySize.y - 200), ImGuiCond_Always);


    ImGui::Begin("Console");


    if (ImGui::Button("Clear"))
    {
        Console::Instance().Clear();
    }


    const auto& messages = Console::Instance().GetMessages();
    ImGui::Text("Message Count: %zu", messages.size()); // Print number of messages
    for (const auto& message : messages)
    {
        ImGui::Text("%s", message.c_str());
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f); // Scroll to bottom
    }
    ImGui::End();

}




void MyGUI::ShowSpawnFigures() {

    ImGui::Begin("Spawn Figures");


    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WINDOW_SIZE.x / WINDOW_SIZE.y, 0.1f, 100.0f);
    glm::mat4 view = camera.view();

    glm::vec2 mouseScreenPos = getMousePosition();
    glm::vec3 mouseWorldPos = screenToWorld(mouseScreenPos, 10.0f, projection, view);


    if (ImGui::Button("Spawn Triangle")) {
        BasicShapesManager::createFigure(1, SceneManager::gameObjectsOnScene, 1.0, mouseWorldPos);
    }

    if (ImGui::Button("Spawn Square")) {
        BasicShapesManager::createFigure(2, SceneManager::gameObjectsOnScene, 1.0, mouseWorldPos);
    }

    if (ImGui::Button("Spawn Cube")) {
        BasicShapesManager::createFigure(3, SceneManager::gameObjectsOnScene, 1.0, mouseWorldPos);
    }

    ImGui::End();
}

void MyGUI::ShowMetricsWindow(bool* p_open) {

   
    // Static variables to store FPS and memory usage history
    static std::vector<float> fpsHistory;
    static std::vector<float> memoryHistory;
    static const int maxSamples = 100;  // Number of samples to display in the graphs

    // Gather data for FPS
    float fps = ImGui::GetIO().Framerate;
    fpsHistory.push_back(fps);
    if (fpsHistory.size() > maxSamples) {
        fpsHistory.erase(fpsHistory.begin());
    }

    // Gather data for Memory Usage
    float memoryUsage = GetMemoryUsage();
    memoryHistory.push_back(memoryUsage);
    if (memoryHistory.size() > maxSamples) {
        memoryHistory.erase(memoryHistory.begin());
    }

    // Set up the ImGui window
    ImGui::Begin("Performance Graphs");

    // Display the FPS graph
    ImGui::Text("FPS Graph");
    ImGui::PlotLines("FPS", fpsHistory.data(), fpsHistory.size(), 0, nullptr, 0.0f, 100.0f, ImVec2(0, 80));
    ImGui::Text("Current FPS: %.1f", fps);

    ImGui::Separator();  // Optional: Adds a separator between the two graphs

    // Display the Memory Usage graph
    ImGui::Text("Memory Usage Graph");
    ImGui::PlotLines("Memory (MB)", memoryHistory.data(), memoryHistory.size(), 0, nullptr, 0.0f, 2000.0f, ImVec2(0, 80));
    ImGui::Text("Current Memory Usage: %.1f MB", memoryUsage);


    //ImGui::Separator();

  

    //ImGui::Text("Console");

    //if (ImGui::Button("Clear"))
    //{
    //    Console::Instance().Clear();
    //}


    //const auto& messages = Console::Instance().GetMessages();
    //ImGui::Text("Message Count: %zu", messages.size()); // Print number of messages
    //for (const auto& message : messages)
    //{
    //    ImGui::Text("%s", message.c_str());
    //}

    //if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
    //    ImGui::SetScrollHereY(1.0f); // Scroll to bottom
    //}
  

    ImGui::End();

}



void MyGUI::ShowHierarchy() 
{
    ImGui::SetNextWindowSize(ImVec2(300, 700), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(0, 20), ImGuiCond_Always);
    if (ImGui::Begin("Hierarchy", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        // Iterar sobre todos los objetos en la escena
        for (auto& go : SceneManager::gameObjectsOnScene) 
        {
            if (SceneManager::gameObjectsOnScene.empty()) continue;

            static char newName[128] = "";
            static bool renaming = false;
            static GameObject* renamingObject = nullptr;

            // Selección y resaltado del objeto
            bool isSelected = (SceneManager::selectedObject == &go);
            if (ImGui::Selectable(go.getName().c_str(), isSelected))
            {
                SceneManager::selectedObject = &go;
            }

            // Iniciar renombrado al hacer doble clic
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) 
            {
                renaming = true;
                renamingObject = &go;
                strcpy_s(newName, go.getName().c_str());
            }

            // Lógica para renombrar el objeto seleccionado
            if (renaming && renamingObject == &go) 
            {
                ImGui::SetKeyboardFocusHere();
                if (ImGui::InputText("##rename", newName, IM_ARRAYSIZE(newName), ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    go.setName(newName);
                    renaming = false;
                }
                if (ImGui::IsItemDeactivated() || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                    renaming = false;
                }
            }
        }
        ImGui::End();
    }
}
void MyGUI::renderInspector() {
    ImGui::Begin("Inspector");
    // Obtener el GameObject actualmente seleccionado
    GameObject* selectedObject = SceneManager::selectedObject;

    if (selectedObject) {
        // Mostrar información del componente Transform
        if (ImGui::CollapsingHeader("Transform")) {

            glm::vec3 position = selectedObject->getPosition();
            glm::vec3 rotation = selectedObject->getRotation();
            glm::vec3 scale = selectedObject->getScale();

            ImGui::Text("Position: (%.2f, %.2f, %.2f)", position.x, position.y, position.z);
            ImGui::Text("Rotation: (%.2f, %.2f, %.2f)", rotation.x, rotation.y, rotation.z);
            ImGui::Text("Scale: (%.2f, %.2f, %.2f)", scale.x, scale.y, scale.z);
        }

        // Mostrar información del componente Mesh
        if (selectedObject->hasMesh() && ImGui::CollapsingHeader("Mesh")) {
            Mesh& mesh = selectedObject->mesh();
            ImGui::Text("Vertices: %d", mesh.vertices());
            //ImGui::Text("Triangles: %d", mesh.getTriangleCount());

            static bool showNormalsPerTriangle = false;
            static bool showNormalsPerFace = false;

            ImGui::Checkbox("Show Normals (Per Triangle)", &showNormalsPerTriangle);
            ImGui::Checkbox("Show Normals (Per Face)", &showNormalsPerFace);

            // Aquí podríamos añadir la lógica para visualizar las normales en función de las opciones seleccionadas
        }

        // Mostrar información del componente Texture
        if (selectedObject->hasTexture() && ImGui::CollapsingHeader("Texture")) {
            Texture& texture = selectedObject->texture();
            /* ImGui::Text("Texture Size: %dx%d", texture.getWidth(), texture.getHeight());
             ImGui::Text("Texture Path: %s", texture.getPath().c_str());

             static bool useCheckerTexture = false;
             ImGui::Checkbox("Use Checker Texture", &useCheckerTexture);*/

             // Aquí podríamos añadir lógica para aplicar una textura de tablero de ajedrez en caso de que se seleccione la opción
        }
    }
    else {
        ImGui::Text("No GameObject selected.");
    }
    ImGui::End();
}
void MyGUI::render() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
   
    ShowConsole();
	ShowMainMenuBar();

    ShowSpawnFigures();

  /*  ShowHierarchy();*/
    renderInspector();

	//Show debug window hello world
	ImGui::Begin("Hello, world!");
	ImGui::Text("This is some useful text.");
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}


void MyGUI::handleEvent(const SDL_Event& event) {
	ImGui_ImplSDL2_ProcessEvent(&event);
}


void MyGUI::processEvent(const SDL_Event& event) {
	ImGui_ImplSDL2_ProcessEvent(&event);
}