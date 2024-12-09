#include <GL/glew.h>
#include "MyGUI.h"
#include "MyGameEngine/GameObject.h"
#include "SceneManager.h"
#include "BasicShapesManager.h"
#include "SystemInfo.h"
#include "Console.h"


#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_opengl.h>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <tinyfiledialogs/tinyfiledialogs.h>

#include <dirent.h>
#include <windows.h>
#include <psapi.h>
#include <codecvt>
#include <locale>
#include <filesystem>

#include <string>
#include <cstdlib>

bool show_metrics_window = false;

bool show_hardware_window = false;

bool show_software_window = false;

bool show_spawn_figures_window = false;
static glm::vec3 accumulatedRotation = glm::vec3(0.0f); // Rotaciones iniciales (acumuladas)


MyGUI::MyGUI(SDL_Window* window, void* context) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init();

    m_Renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Load icons
    m_DirectoryIcon = LoadTexture("Resources/Icons/ContentBrowser/DirectoryIcon.bmp");
    m_FileIcon = LoadTexture("Resources/Icons/ContentBrowser/FileIcon.bmp");
    m_BaseDirectory = "Assets";  // Adjust the path accordingly
    m_CurrentDirectory = m_BaseDirectory;  // Start at the base directory
}

SDL_Texture* MyGUI::LoadTexture(const std::string& path) {
    SDL_Surface* surface = SDL_LoadBMP(path.c_str());
    if (!surface) {
        std::cerr << "Failed to load texture: " << path << " - " << SDL_GetError() << std::endl;
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_Renderer, surface);
    SDL_FreeSurface(surface);

    if (!texture) {
        std::cerr << "Failed to create texture from surface: " << SDL_GetError() << std::endl;
    }

    return texture;
}

std::wstring ConvertToWideString(const std::string& str) {
    // Determine the size of the resulting wide string
    int wideSize = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    if (wideSize == 0) {
        throw std::runtime_error("Error converting string to wide string.");
    }

    // Allocate a buffer for the wide string
    std::wstring wideStr(wideSize, L'\0');

    // Perform the conversion
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wideStr[0], wideSize);

    // Remove the null terminator that MultiByteToWideChar adds
    wideStr.resize(wideSize - 1);

    return wideStr;
}

std::vector<std::string> MyGUI::GetDirectoryContents(const std::string& directory) {
    std::vector<std::string> contents;

    DIR* dir = opendir(directory.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            // Skip the special directories "." and ".."
            if (entry->d_name[0] == '.') {
                continue;
            }
            contents.push_back(entry->d_name);
        }
        closedir(dir);
    }
    else {
        std::cerr << "Failed to open directory: " << directory << std::endl;
    }

    return contents;
}

void MyGUI::OnDropTarget() {
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
            // Get the dropped item path
            const char* droppedPath = (const char*)payload->Data;
            std::string filePath(droppedPath);

            // Perform actions with the file path (e.g., load the file, spawn object, etc.)
            std::cout << "Dropped file: " << filePath << std::endl;

            // Example: Load a scene object or texture, based on the dropped file
            SceneManager::LoadGameObject(filePath); // Or other actions based on the file type
        }
        ImGui::EndDragDropTarget();
    }
}

void MyGUI::ShowContentBrowser() {
    ImGui::Begin("Content Browser");

    // Navigate to the parent directory if not at the base directory
    if (m_CurrentDirectory != m_BaseDirectory) {
        if (ImGui::Button("<-")) {
            // Go to the parent directory
            size_t lastSlash = m_CurrentDirectory.find_last_of("\\/");
            if (lastSlash != std::string::npos) {
                m_CurrentDirectory = m_CurrentDirectory.substr(0, lastSlash);
            }
        }
    }

    static float padding = 16.0f;
    static float thumbnailSize = 128.0f;
    float cellSize = thumbnailSize + padding;

    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columnCount = (int)(panelWidth / cellSize);
    if (columnCount < 1) columnCount = 1;

    ImGui::Columns(columnCount, 0, false);

    // Get contents of the current directory
    auto contents = GetDirectoryContents(m_CurrentDirectory);

    for (const auto& entry : contents) {
        std::string fullPath = m_CurrentDirectory + "\\" + entry;

        ImGui::PushID(entry.c_str());
        bool isDirectory = (GetFileAttributesW(ConvertToWideString(fullPath).c_str()) & FILE_ATTRIBUTE_DIRECTORY) != 0;

        // Choose the appropriate icon
        SDL_Texture* icon = isDirectory ? m_DirectoryIcon : m_FileIcon;

        // Display the texture as an image button in ImGui
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        if (ImGui::ImageButton((void*)icon, { thumbnailSize, thumbnailSize })) {
            if (isDirectory) {
                // Navigate into the directory
                m_CurrentDirectory = fullPath;
            }
        }

        // Begin drag-and-drop source when hovering over an item
        if (ImGui::BeginDragDropSource()) {
            // Set the drag payload with the item's path
            std::filesystem::path relativePath(fullPath);
            const wchar_t* itemPath = relativePath.c_str();
            ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
            ImGui::Text("Dragging: %s", entry.c_str());
            ImGui::EndDragDropSource();
        }

        ImGui::PopStyleColor();

        // Double-click to open a directory
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            if (isDirectory)
                m_CurrentDirectory = fullPath;
        }

        // Show the file or directory name
        ImGui::TextWrapped(entry.c_str());
        ImGui::NextColumn();

        ImGui::PopID();
    }

    ImGui::Columns(1);

    ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
    ImGui::SliderFloat("Padding", &padding, 0, 32);

    // Handle drag-and-drop target
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
            std::wstring droppedPath = (const wchar_t*)payload->Data;
            std::string filePath(droppedPath.begin(), droppedPath.end());

            // Load the dropped file as a new GameObject
            std::string extension = filePath.substr(filePath.find_last_of(".") + 1);
            if (extension == "fbx") {
                SceneManager::LoadGameObject(filePath);
            }
            else if (extension == "png" || extension == "jpg" || extension == "jpeg") {
                if (SceneManager::selectedObject) {
                    auto imageTexture = std::make_shared<Image>();
                    imageTexture->loadTexture(filePath);
                    SceneManager::selectedObject->setTextureImage(imageTexture);
                }
            }
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::End();
}

MyGUI::~MyGUI() {

    if (m_DirectoryIcon) SDL_DestroyTexture(m_DirectoryIcon);
    if (m_FileIcon) SDL_DestroyTexture(m_FileIcon);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void MyGUI::ShowMainMenuBar() {

    if (show_metrics_window) {
        ShowMetricsWindow(&show_metrics_window);
    }
    if (show_hardware_window) {
        ShowRenderSystemInfo(&show_hardware_window);
    }
    if (show_software_window) {
        ShowLibraryVerions(&show_software_window);
    }
    if (show_spawn_figures_window) {
        ShowSpawnFigures(&show_spawn_figures_window);
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
                exit(0);

            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Mesh")) {
            ImGui::Checkbox("Mesh Creator", &show_spawn_figures_window);
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
            ImGui::Checkbox("Hardware Info", &show_hardware_window);
            ImGui::Checkbox("Software Info", &show_software_window);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void MyGUI::ShowConsole() {

    ImGui::SetNextWindowSize(ImVec2(680, 200), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(300, ImGui::GetIO().DisplaySize.y - 200), ImGuiCond_Always);

    ImGui::Begin("Console");

    if (ImGui::Button("Clear"))
    {
        Console::Instance().Clear();
    }

    const auto& messages = Console::Instance().GetMessages();
    ImGui::Text("Message Count: %zu", messages.size());
    for (const auto& message : messages)
    {
        ImGui::Text("%s", message.c_str());
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }
    ImGui::End();

}

void MyGUI::ShowSpawnFigures(bool* p_open) {

    ImGui::Begin("Spawn Figures");

    if (ImGui::Button("Spawn Triangle")) {
        BasicShapesManager::createFigure(1, SceneManager::gameObjectsOnScene, 1.0, vec3(0.0f, 0.0f, 0.0f));
        SceneManager::selectedObject = &SceneManager::gameObjectsOnScene.back();
    }

    if (ImGui::Button("Spawn Square")) {
        BasicShapesManager::createFigure(2, SceneManager::gameObjectsOnScene, 1.0, vec3(0.0f, 0.0f, 0.0f));
        SceneManager::selectedObject = &SceneManager::gameObjectsOnScene.back();
    }

    if (ImGui::Button("Spawn Cube")) {
        BasicShapesManager::createFigure(3, SceneManager::gameObjectsOnScene, 1.0, vec3(0.0f, 0.0f, 0.0f));
        SceneManager::selectedObject = &SceneManager::gameObjectsOnScene.back();
    }
    ImGui::End();
}

float GetMemoryUsage() {
    PROCESS_MEMORY_COUNTERS memCounter;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &memCounter, sizeof(memCounter))) {
        return static_cast<float>(memCounter.WorkingSetSize) / (1024.0f * 1024.0f); // Convert bytes to MB
    }
    return 0.0f; // Return 0 if there's an issue getting the memory info
}

void MyGUI::ShowMetricsWindow(bool* p_open)
{
    static std::vector<float> fpsHistory;
    static std::vector<float> memoryHistory;
    static const int maxSamples = 100;

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

    ImGui::Begin("Performance Graphs");


    ImGui::Text("FPS Graph");
    ImGui::PlotLines("FPS", fpsHistory.data(), fpsHistory.size(), 0, nullptr, 0.0f, 100.0f, ImVec2(0, 80));
    ImGui::Text("Current FPS: %.1f", fps);

    ImGui::Separator();

    ImGui::Text("Memory Usage Graph");
    ImGui::PlotLines("Memory (MB)", memoryHistory.data(), memoryHistory.size(), 0, nullptr, 0.0f, 100.0f, ImVec2(0, 80));
    ImGui::Text("Current Memory Usage: %.1f MB", memoryUsage);

    ImGui::End();

}

void MyGUI::ShowRenderSystemInfo(bool* p_open)
{
    ImGui::Begin("Hardware Information");
    std::string systemInfo = SystemInfo::GetFullSystemInfo();
    ImGui::TextWrapped("%s", systemInfo.c_str());
    ImGui::End();
}

void MyGUI::ShowLibraryVerions(bool* p_open)
{
    ImGui::Begin("Software Information");
    std::string libraryInfo = SystemInfo::GetFullLibraryVerions();
    ImGui::TextWrapped("%s", libraryInfo.c_str());
    ImGui::End();
}

void MyGUI::ShowHierarchy() {
    ImGui::SetNextWindowSize(ImVec2(300, 700), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(0, 20), ImGuiCond_Always);

    if (ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        static GameObject* draggedObject = nullptr;

        auto showGameObject = [&](GameObject& go, auto& showGameObject) -> void {
            ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow |
                ImGuiTreeNodeFlags_OpenOnDoubleClick |
                ImGuiTreeNodeFlags_SpanAvailWidth;

            if (SceneManager::selectedObject == &go) {
                nodeFlags |= ImGuiTreeNodeFlags_Selected;
            }
            if (go.children().empty()) {
                nodeFlags |= ImGuiTreeNodeFlags_Leaf;
            }

            bool nodeOpen = ImGui::TreeNodeEx(go.getName().c_str(), nodeFlags);
            if (ImGui::IsItemClicked()) {
                SceneManager::selectedObject = &go;
            }

            // Drag source
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                draggedObject = &go;
                ImGui::SetDragDropPayload("HIERARCHY_DRAG", &draggedObject, sizeof(GameObject*));
                ImGui::Text("Dragging: %s", go.getName().c_str());
                ImGui::EndDragDropSource();
            }

            // Drop target
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_DRAG")) {
                    SceneManager::gameObjectsOnScene;
                    GameObject* droppedObject = *(GameObject**)payload->Data;
                    if (droppedObject && droppedObject != &go) {

                        /*auto s = droppedObject->parent()->getName();
                        std::cout << s << std::endl;*/
                        // Quitar al objeto de su padre anterior
                        if (!droppedObject->isRoot()) {
                            droppedObject->setParent(go);
                        }

                        // Agregar como hijo del nuevo padre
                    //go.emplaceChild(std::move(*droppedObject));
                    //go.emplaceChild(std::move(*droppedObject));
                        //droppedObject->setTransform(go.worldTransform() * droppedObject->localTransform());




                    }
                }
                ImGui::EndDragDropTarget();
            }

            if (nodeOpen) {
                for (auto& child : go.children()) {
                    showGameObject(child, showGameObject);
                }
                ImGui::TreePop();
            }
            };

        for (auto& go : SceneManager::gameObjectsOnScene) {
            if (go.isRoot()) {
                showGameObject(go, showGameObject);
            }
        }
        ImGui::End();
    }
}
//ALGO FALLA , HAY QUE REVISARLO
void MyGUI::renderInspector() {
    ImGui::SetNextWindowSize(ImVec2(300, 700), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(980, 20), ImGuiCond_Always);
    ImGui::Begin("Inspector");

    static GameObject* persistentSelectedObject = nullptr;

    if (SceneManager::selectedObject != nullptr) {
        persistentSelectedObject = SceneManager::selectedObject;
    }

    if (persistentSelectedObject) {
        if (ImGui::CollapsingHeader("Transform")) {
            glm::vec3 position = persistentSelectedObject->GetComponent<TransformComponent>()->transform().pos();
            glm::vec3 rotation = persistentSelectedObject->GetComponent<TransformComponent>()->transform().GetRotation();
            glm::vec3 scale = persistentSelectedObject->GetComponent<TransformComponent>()->transform().extractScale(persistentSelectedObject->GetComponent<TransformComponent>()->transform().mat());

            // Controles para la posición
            ImGui::Text("Position:");
            ImGui::PushItemWidth(100);
            if (ImGui::DragFloat("X##pos", &position.x, 0.1f)) {
                persistentSelectedObject->GetComponent<TransformComponent>()->transform().setPos(position.x, position.y, position.z);
            }
            ImGui::NewLine();
            if (ImGui::DragFloat("Y##pos", &position.y, 0.1f)) {
                persistentSelectedObject->GetComponent<TransformComponent>()->transform().setPos(position.x, position.y, position.z);
            }
            ImGui::NewLine();
            if (ImGui::DragFloat("Z##pos", &position.z, 0.1f)) {
                persistentSelectedObject->GetComponent<TransformComponent>()->transform().setPos(position.x, position.y, position.z);
            }
            ImGui::PopItemWidth();


            // Controles para la rotación
            ImGui::Text("Rotation:");
            ImGui::PushItemWidth(100);

            //  X Rotation
            float deltaX = 0.0f;
            if (ImGui::DragFloat("X##rot", &accumulatedRotation.x, 0.1f, -360.0f, 360.0f, "%.3f")) {
                deltaX = accumulatedRotation.x - persistentSelectedObject->transform().GetRotation().x;
                if (deltaX != 0.0f) {
                    persistentSelectedObject->transform().rotate(glm::radians(deltaX), glm::vec3(1.0f, 0.0f, 0.0f));
                }
            }
            //  Y Rotation
            float deltaY = 0.0f;
            if (ImGui::DragFloat("Y##rot", &accumulatedRotation.y, 0.1f, -360.0f, 360.0f, "%.3f")) {
                deltaY = accumulatedRotation.y - persistentSelectedObject->transform().GetRotation().y;
                if (deltaY != 0.0f) {
                    persistentSelectedObject->transform().rotate(glm::radians(deltaY), glm::vec3(0.0f, 1.0f, 0.0f));
                }
            }
            //  Z Rotation
            float deltaZ = 0.0f;
            if (ImGui::DragFloat("Z##rot", &accumulatedRotation.z, 0.1f, -360.0f, 360.0f, "%.3f")) {
                deltaZ = accumulatedRotation.z - persistentSelectedObject->transform().GetRotation().z;
                if (deltaZ != 0.0f) {
                    persistentSelectedObject->transform().rotate(glm::radians(deltaZ), glm::vec3(0.0f, 0.0f, 1.0f));
                }
            }

            ImGui::PopItemWidth();


            // Controles para la escala
            ImGui::Text("Scale:");
            ImGui::PushItemWidth(100);
            if (ImGui::DragFloat("X##scale", &scale.x, 0.1f, 0.01f, 100.0f)) {
                persistentSelectedObject->transform().setScale(scale);
            }
            if (ImGui::DragFloat("Y##scale", &scale.y, 0.1f, 0.01f, 100.0f)) {
                persistentSelectedObject->transform().setScale(scale);
            }
            if (ImGui::DragFloat("Z##scale", &scale.z, 0.1f, 0.01f, 100.0f)) {
                persistentSelectedObject->transform().setScale(scale);
            }
            ImGui::PopItemWidth();
        }

        if (persistentSelectedObject->hasMesh() && ImGui::CollapsingHeader("Mesh")) {
            Mesh& mesh = persistentSelectedObject->mesh();

            static bool showNormalsPerTriangle = false;
            static bool showNormalsPerFace = false;

            ImGui::Checkbox("Show Normals (Per Triangle)", &showNormalsPerTriangle);
            ImGui::Checkbox("Show Normals (Per Face)", &showNormalsPerFace);
            if (showNormalsPerTriangle) {
                persistentSelectedObject->mesh().drawNormals(persistentSelectedObject->transform().mat());
            }
            if (showNormalsPerFace) {
                persistentSelectedObject->mesh().drawNormalsPerFace(persistentSelectedObject->transform().mat());
            }
        }
        if (persistentSelectedObject->hasTexture() && ImGui::CollapsingHeader("Texture")) {
            Texture& texture = persistentSelectedObject->texture();
            static bool showCheckerTexture = false;
            ImGui::Text("Width: %d", texture.image().width());
            ImGui::Text("Heiht: %d", texture.image().height());

            if (ImGui::Button("Toggle Checker Texture")) {
                showCheckerTexture = !showCheckerTexture;
                persistentSelectedObject->hasCheckerTexture = showCheckerTexture;
            }

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

    ShowMainMenuBar();

    ShowHierarchy();
    renderInspector();

    ShowContentBrowser();

    ShowConsole();


    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}


void MyGUI::handleEvent(const SDL_Event& event) {
    ImGui_ImplSDL2_ProcessEvent(&event);
}

void MyGUI::processEvent(const SDL_Event& event) {
    ImGui_ImplSDL2_ProcessEvent(&event);
}