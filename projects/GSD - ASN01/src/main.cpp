/* 
 * Name: Roderick "R.J." Montague
 * Date: 02/06/2020
 * Description: three-dimensional sound
 * References:
	* https://glm.g-truc.net/0.9.3/api/a00199.html
*/

// GAME SOUND - ASSIGNMENT 1
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

// object includes
#include "TTK/Cube.h"
// #include "TTK/Sphere.h"
#include "TTK/Teapot.h"

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

// rotation on x-axis
glm::vec3 RotateX(const glm::vec3 vec, float theta, bool inDegrees)
{
	return glm::rotateX(vec, (inDegrees) ? glm::radians(theta) : theta);
}

// rotates a vector on the y-axis
glm::vec3 RotateY(const glm::vec3 vec, float theta, bool inDegrees)
{
	return glm::rotateY(vec, (inDegrees) ? glm::radians(theta) : theta);
}

// rotation on z-axis
glm::vec3 RotateZ(const glm::vec3 vec, float theta, bool inDegrees)
{
	return glm::rotateZ(vec, (inDegrees) ? glm::radians(theta) : theta);
}

// returns a vector 3 as a string.
std::string Vector3ToString(glm::vec3 v3)
{
	return "(" + std::to_string(v3.x) + ", " + std::to_string(v3.y) + ", " + std::to_string(v3.z) + ")";
}

int main() {
	// ADDITIONAL VARIABLES (1)
	// the window size
	glm::ivec2 windowSize = glm::ivec2(800, 800);

	// MOVEMENT SETUP
	bool panMode = true; // if 'true', the project is in pan mode. If false, the project is in travel mode.

	// positioning
	glm::vec3 startPosition = { 0, 0, 4.5F }; // close position
	glm::vec3 currentPosition = startPosition; // the current position

	// Panning
	float theta = 0.0F; // rotation in degrees
	const float R_INC = -30.0F; // rotation incrementer in degrees

	// Translation (uses LERP)
	glm::vec3 endPosition = { 0, 0, 20.5F };
	float t = 0.0F;
	const float T_INC = 0.5F;
	bool forward = true; // 'true' if the entity is moving forward

	// Drawing Cubes
	// the size of the indicator
	float indicatorSize = 100.0F;

	// listener position and colour
	glm::vec3 listenerPos = glm::vec3(-350.0F, 70.0F, 0.0F);
	glm::vec4 listenerClr = glm::vec4(1.0F, 0.0F, 0.0F, 1.0F);

	// audio position and colour
	glm::vec3 audioPos = glm::vec3(0.0F);
	glm::vec4 audioClr = glm::vec4(0.0F, 1.0F, 0.0F, 1.0F);

	// used for panning
	glm::vec3 pDist = glm::vec3(0.0F, 150.0F, 0.0F);

	// used for translation
	glm::vec3 tEndPos = listenerPos + glm::vec3(-150.0F, 0.0F, 0.0F);

	audioPos = listenerPos + pDist;

	// Audio Engine
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
	GLFWwindow* window = glfwCreateWindow(windowSize.x, windowSize.y, "GSD - ASN01", nullptr, nullptr);

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
	// overworld loop (Super Mario World)
	// audioEngine.LoadBank("Overworld_Loop/Master", FMOD_STUDIO_LOAD_BANK_NORMAL);

	// main loop
	// audioEngine.LoadBank("MainLoop/Master", FMOD_STUDIO_LOAD_BANK_NORMAL);
	audioEngine.LoadBank("Master", FMOD_STUDIO_LOAD_BANK_NORMAL);

	//// Load an event
	// overworld
	// audioEngine.LoadEvent("Music", "{13f73348-5181-4c6b-838a-4ef9f8ad1b56}"); // Overworld_Loop

	// main loop
	audioEngine.LoadEvent("Music", "{84a26086-1e10-4505-a437-99ff0ff2a354}"); // MainLoop

	audioEngine.SetEventPosition("Music", currentPosition); // in front of the listener.

	audioEngine.LoadEvent("Damage", "{5475950f-6744-4710-8800-49b950cab411}");


	//// Play the event
	audioEngine.PlayEvent("Music");
	// audioEngine.SetGlobalParameter("Underwater", 1.0F);

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

			audioEngine.PlayEvent("Damage"); // plays damage sound
			audioEngine.SetGlobalParameter("Damage", 1.0F); // makes the audio fade back in

			// Visual
			if (isOnYoshi) {

				// Underwater
				audioEngine.SetGlobalParameter("Underwater", 1.0F);
				

				currentSprite = &yoshi;
				
			}
			else {
			
				// Underwater
				audioEngine.SetGlobalParameter("Underwater", 0.0F);
				
				currentSprite = &mario;
			}
		}
		// rotates the sound around the listener
		else if (TTK::Input::GetKeyDown(TTK::KeyCode::P))
		{
			currentPosition = startPosition;
			t = 0.0F;
			panMode = true;

			audioEngine.SetEventPosition("Music", currentPosition);
		}
		// switching to distaince mode (the sound is get closer and further from the listener)
		else if (TTK::Input::GetKeyPressed(TTK::KeyCode::D))
		{
			// resetting the values
			currentPosition = startPosition;
			theta = 0.0F;
			panMode = false;
			
			audioEngine.SetEventPosition("Music", currentPosition);
		}
		
		// movements
		if (panMode) // panning
		{
			theta += R_INC * dt;
			currentPosition = RotateY(startPosition, theta, true); // rotation

			// std::cout << "Rotation: " << theta << "\n";
			// std::cout << "CurrPos: " << Vector3ToString(currentPosition) << std::endl;
		}
		else // if the program is not in pan mode, it is in distance mode.
		{
			// changing direction
			if (t == 1.0F)
			{
				t = 0.0F;
				forward = !forward;
			}

			// t value for lerp
			t += T_INC * dt;

			// clamping the value of 't' to not go out of bounds.
			t = glm::clamp(t, 0.0F, 1.0F);

			// direction is based on if the entity is travelling forward or not.
			if (forward)
				currentPosition = glm::mix(startPosition, endPosition, t);
			else
				currentPosition = glm::mix(endPosition, startPosition, t);
		}

		// setting the position
		audioEngine.SetEventPosition("Music", currentPosition);

		//// Update Audio Engine
		audioEngine.Update();

		camera.update();
		TTK::Graphics::SetCameraMatrix(camera.ViewMatrix);

		// drawing the current sprite.
		// currentSprite->Update(dt);
		// currentSprite->Draw(
		// 	TTK::Graphics::GetViewProjection() *
		// 	glm::translate(glm::mat4(1.0f), glm::vec3(-150, 150, 0.0f)) * 
		// 	glm::scale(glm::mat4(1.0f), glm::vec3(100.0f, -100.0f, 1.0f))
		// );
		
		// drawing the listener indicator
		TTK::Graphics::DrawCube(listenerPos, indicatorSize, listenerClr);

		// drawing the sound indicator
		if (panMode) // panning mode
		{
			// TTK::Graphics::DrawCube(audioPos, indicatorSize, audioClr);
			TTK::Graphics::DrawCube(RotateZ(audioPos - listenerPos, theta, true) + listenerPos,
				indicatorSize, audioClr);
		}
		else // distance mode
		{
			if(forward)
				TTK::Graphics::DrawCube(glm::mix(listenerPos, tEndPos, t), indicatorSize, audioClr);
			else
				TTK::Graphics::DrawCube(glm::mix(tEndPos, listenerPos, t), indicatorSize, audioClr);
		}

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



