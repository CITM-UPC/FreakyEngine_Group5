
#include "MyWindow.h"

#include <SDL2/SDL_video.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_opengl.h>

#include <string>
#include <vector>
#include <SDL2/SDL_render.h>


class MyGUI : public IEventProcessor
{
public:
	MyGUI(SDL_Window* window, void* context);
	MyGUI(MyGUI&&) noexcept = delete;
	MyGUI(const MyGUI&) = delete;
	MyGUI& operator=(const MyGUI&) = delete;
	~MyGUI();
	void render();
	void renderInspector();
	void processEvent(const SDL_Event& event) override;
	void handleEvent(const SDL_Event& event);

	void ShowMainMenuBar();
	void ShowConsole();


	void ShowSpawnFigures(bool* p_open);
	void ShowMetricsWindow(bool* p_open);
	void ShowRenderSystemInfo(bool* p_open);
	void ShowLibraryVerions(bool* p_open);

	void ShowHierarchy();

	// New variables for SDL2 texture handling
	SDL_Renderer* m_Renderer;
	SDL_Texture* m_DirectoryIcon = nullptr;
	SDL_Texture* m_FileIcon = nullptr;

	SDL_Texture* LoadTexture(const std::string& path); // Ensure this declaration matches the implementation

	void ShowContentBrowser();

private:

	// New member variables for current directory navigation
	std::string m_BaseDirectory;
	std::string m_CurrentDirectory;

	// Helper functions
	std::vector<std::string> GetDirectoryContents(const std::string& directory);

	void OnDropTarget();


};