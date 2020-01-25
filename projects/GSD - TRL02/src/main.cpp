#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "TTK/TTKContext.h"
#include "Logging.h"
#include "TTK/GraphicsUtils.h"
#include "TTK/Camera.h"
#include "AudioEngine.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include "TTK/Input.h"

#include "imgui.h"
#include "TTK/SpriteSheetQuad.h"

#define LOG_GL_NOTIFICATIONS
/*
	Handles debug messages from OpenGL
	https://www.khronos.org/opengl/wiki/Debug_Output#Message_Components
	@param source    Which part of OpenGL dispatched the message
	@param type      The type of message (ex: error, performance issues, deprecated behavior)
	@param id        The ID of the error or message (to distinguish between different types of errors, like nullref or index out of range)
	@param severity  The severity of the message (from High to Notification)
	@param length    The length of the message
	@param message   The human readable message from OpenGL
	@param userParam The pointer we set with glDebugMessageCallback (should be the game pointer)
*/
void GlDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	std::string sourceTxt;
	switch (source) {
	case GL_DEBUG_SOURCE_API: sourceTxt = "DEBUG"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: sourceTxt = "WINDOW"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceTxt = "SHADER"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY: sourceTxt = "THIRD PARTY"; break;
	case GL_DEBUG_SOURCE_APPLICATION: sourceTxt = "APP"; break;
	case GL_DEBUG_SOURCE_OTHER: default: sourceTxt = "OTHER"; break;
	}
	switch (severity) {
	case GL_DEBUG_SEVERITY_LOW:          LOG_INFO("[{}] {}", sourceTxt, message); break;
	case GL_DEBUG_SEVERITY_MEDIUM:       LOG_WARN("[{}] {}", sourceTxt, message); break;
	case GL_DEBUG_SEVERITY_HIGH:         LOG_ERROR("[{}] {}", sourceTxt, message); break;
		#ifdef LOG_GL_NOTIFICATIONS
	case GL_DEBUG_SEVERITY_NOTIFICATION: LOG_INFO("[{}] {}", sourceTxt, message); break;
		#endif
	default: break;
	}
}


int main() {


	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	long long memBreak = 0;
	if (memBreak) _CrtSetBreakAlloc(memBreak);
	
	Logger::Init();
	
	// Initialize GLFW
	if (glfwInit() == GLFW_FALSE) {
		std::cout << "Failed to initialize Glad" << std::endl;
		return 1;
	}


	// Create a new GLFW window
	GLFWwindow* window = glfwCreateWindow(300, 300, "GSD - TRL02", nullptr, nullptr);

	// We want GL commands to be executed for our window, so we make our window's context the current one
	glfwMakeContextCurrent(window);

	// Initialize the TTK input system
	TTK::Input::Init(window);

	// Let glad know what function loader we are using (will call gl commands via glfw)
	if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
		std::cout << "Failed to initialize Glad" << std::endl;
		return 2;
	}

	// Display our GPU and OpenGL version
	std::cout << glGetString(GL_RENDERER) << std::endl;
	std::cout << glGetString(GL_VERSION) << std::endl;

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(GlDebugMessage, nullptr);

	TTK::Graphics::SetCameraMode2D(300, 300);
	glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
		TTK::Graphics::SetCameraMode2D(width, height);
	});
	TTK::Graphics::SetBackgroundColour(0.5f, 0.5f, 0.5f);
	TTK::Graphics::SetDepthEnabled(true);

	TTK::Camera camera;
	camera.cameraPosition = glm::vec3(0, 0, -10);
	camera.forwardVector = glm::vec3(0, 0, 1);

	glm::vec2 lastMousePos, mousePos;
	lastMousePos = TTK::Input::GetMousePos();

	TTK::Graphics::InitImGUI(window);

	TTK::SpriteSheetQuad mario;
	mario.SliceSpriteSheet("mario.png", 1, 1, 0.5f);
	TTK::SpriteSheetQuad yoshi;
	yoshi.SliceSpriteSheet("yoshi.png", 1, 1, 0.5f);
	TTK::SpriteSheetQuad* currentSprite = &mario;
	
	float lastFrame = glfwGetTime();
	
	////// Sound Stuff //////

	//// Make an AudioEngine object 
	AudioEngine audioEngine;

	//// Init AudioEngine (Don't forget to shut down and update)
	audioEngine.Init();

	//// Load a bank (Use the flag FMOD_STUDIO_LOAD_BANK_NORMAL)
	audioEngine.LoadBank("Master", FMOD_STUDIO_LOAD_BANK_NORMAL);

	//// Load an event
	audioEngine.LoadEvent("Music", "{13f73348-5181-4c6b-838a-4ef9f8ad1b56}");

	//// Play the event
	audioEngine.PlayEvent("Music");

	//// Make some Vectors to use
	glm::vec3 startPosition = { 100, 100, 100 }; // close position
	glm::vec3 endPosition = { -100, -100, -100 }; // far position

	glm::vec3 leftVelocity = {-10, 0, 0};
	glm::vec3 rightVelocity = { 10, 0, 0 };

	

	//// Set initial position  
	audioEngine.SetEventPosition("Music", startPosition); // name of the event and its position.
	// audioEngine.SetEventVelocity("Music", leftVelocity);

	// Run as long as the window is open
	while (!glfwWindowShouldClose(window)) {
		// Poll for events from windows (clicks, keypressed, closing, all that)
		glfwPollEvents();

		float thisFrame = glfwGetTime();
		float dt = thisFrame - lastFrame;
		
		// Clear our screen every frame
		TTK::Graphics::ClearScreen();
		
		static bool isOnYoshi = false;
		if (TTK::Input::GetKeyPressed(TTK::KeyCode::Q)) {
			isOnYoshi = !isOnYoshi;

			if (isOnYoshi) {

				//// Set Position
				audioEngine.SetEventPosition("Music", endPosition); // set to far position

				// the sound should start to move from the right to left spekaer.
				// audioEngine.SetEventVelocity("Music", leftVelocity); 

				currentSprite = &yoshi;
				
			}
			else {
				
				//// Set Position
				audioEngine.SetEventPosition("Music", startPosition); // set to close position.
				// audioEngine.SetEventVelocity("Music", rightVelocity);  
				
				currentSprite = &mario;
			}
		}
		

		//// Update Audio Engine
		audioEngine.SetEventVelocity("Music", leftVelocity);
		audioEngine.Update();

		camera.update();
		TTK::Graphics::SetCameraMatrix(camera.ViewMatrix);

		currentSprite->Update(dt);
		currentSprite->Draw(
			TTK::Graphics::GetViewProjection() *
			glm::translate(glm::mat4(1.0f), glm::vec3(-150, 150, 0.0f)) * 
			glm::scale(glm::mat4(1.0f), glm::vec3(100.0f, -100.0f, 1.0f))
		);
		
		TTK::Graphics::EndFrame();
		TTK::Graphics::BeginGUI();
		TTK::Graphics::EndGUI();

		// Present our image to windows
		glfwSwapBuffers(window);

		TTK::Input::Poll();

		lastFrame = thisFrame;
		

	}

	glfwTerminate();


	//// Shut down audio engine
	audioEngine.Shutdown();

	TTK::Graphics::ShutdownImGUI();
	TTK::Input::Uninitialize();
	TTK::Graphics::Cleanup();
	Logger::Uninitialize();

	return 0;
}



