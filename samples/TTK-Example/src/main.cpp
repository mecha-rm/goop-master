#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "TTK/TTKContext.h"
#include "Logging.h"
#include "TTK/GraphicsUtils.h"
#include "TTK/Camera.h"

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


template<int D = 3>
struct Bezier {
	const glm::mat4 BlendMatrix = glm::mat4(
		{ -1,  3, -3,  1 },
		{ 3, -6,  3,  0 },
		{ -3,  3,  0,  0 },
		{ 1,  0,  0,  0 }
	);
	glm::mat4 curve;
	Bezier(const glm::vec<D, float>& a, const glm::vec<D, float>& b, const glm::vec<D, float>& c, const glm::vec<D, float>& d) {
		curve = BlendMatrix;
		glm::mat<D, 4, float> source;
		for (int ix = 0; ix < D; ix++)
			source[ix] = { a[ix], b[ix], c[ix], d[ix] };
		curve = BlendMatrix * source;
	}

	glm::vec<D, float> Resolve(float t) const {
		glm::vec4 power = glm::vec4(t * t * t, t * t, t, 1);
		return power * curve;
	}
};

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
	GLFWwindow* window = glfwCreateWindow(300, 300, "Hello!", nullptr, nullptr);

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

	TTK::Graphics::SetCameraMode3D(300, 300);
	glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
		TTK::Graphics::SetCameraMode3D(width, height);
	});
	TTK::Graphics::SetBackgroundColour(0.5f, 0.5f, 0.5f);
	TTK::Graphics::SetDepthEnabled(true);

	TTK::Camera camera;

	glm::vec2 lastMousePos, mousePos;
	lastMousePos = TTK::Input::GetMousePos();

	TTK::Graphics::InitImGUI(window);

	TTK::SpriteSheetQuad quad;
	quad.SliceSpriteSheet("test-sprite.png", 4, 4, 2.0f);
	
	float lastFrame = glfwGetTime();
	// Run as long as the window is open
	while (!glfwWindowShouldClose(window)) {
		// Poll for events from windows (clicks, keypressed, closing, all that)
		glfwPollEvents();

		float thisFrame = glfwGetTime();
		float dt = thisFrame - lastFrame;
		
		// Clear our screen every frame
		TTK::Graphics::ClearScreen();

		mousePos = TTK::Input::GetMousePos();
		if (TTK::Input::GetMouseDown(TTK::MouseButton::Right)) {
			camera.processMouseMotion(mousePos.x, mousePos.y, lastMousePos.x, lastMousePos.y, dt);
		}

		if (TTK::Input::GetKeyDown(TTK::KeyCode::W))
			camera.moveForward();
		if (TTK::Input::GetKeyDown(TTK::KeyCode::S))
			camera.moveBackward();
		if (TTK::Input::GetKeyDown(TTK::KeyCode::A))
			camera.moveLeft();
		if (TTK::Input::GetKeyDown(TTK::KeyCode::D))
			camera.moveRight();
		if (TTK::Input::GetKeyDown(TTK::KeyCode::LeftControl))
			camera.moveDown();
		if (TTK::Input::GetKeyDown(TTK::KeyCode::Space))
			camera.moveUp();

		if (TTK::Input::GetKeyPressed(TTK::KeyCode::R))
			camera.cameraPosition = { 0, 0, 0 };
		
		camera.update();
		TTK::Graphics::SetCameraMatrix(camera.ViewMatrix);

		quad.Update(dt);
		quad.Draw(TTK::Graphics::GetViewProjection() * glm::scale(glm::mat4(1.0f), glm::vec3(10.0f)));

		TTK::Graphics::DrawLine(glm::vec3(0.0f), { 500.0f, 0.0f, 500.0f }, { 0.0f, 0.0f, 0.0f, 1.0f });
		
		Bezier curve3 = Bezier(glm::vec3(0.0f), glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(50.0f, 10.0f, 50.0f), glm::vec3(50.0f, 0.0f, 50.0f));
		Bezier curve2 = Bezier(glm::vec2(0.0f), glm::vec2(0.0f, 10.0f), glm::vec2(50.0f, 10.0f), glm::vec2(50.0f, 0.0f));
		glm::vec3 a = { 0, 0, 0 };
		glm::vec3 b = { 500, 0, 500 };
		glm::vec3 p3 = curve3.Resolve(0.0f);
		TTK::Graphics::DrawPoint(p3, 4.0f, { 0.0f, 0.0f, 0.0f, 1.0f });
		for(int i = 1; i <= 100; i++) {
			glm::vec3 p2 = curve3.Resolve(i / 100.0f);
			TTK::Graphics::DrawLine(p3, p2, { 0.0f, 0.0f, 0.0f, 1.0f });
			TTK::Graphics::DrawPoint(p2, 4.0f, { 0.0f, 0.0f, 0.0f, 1.0f });
			p3 = p2;
		}

		glm::vec2 p2a = curve2.Resolve(0.0f);
		TTK::Graphics::DrawPoint(glm::vec3(p2a, 0.0f), 4.0f, { 0.0f, 0.0f, 0.0f, 1.0f });
		for (int i = 1; i <= 100; i++) {
			glm::vec2 p2b = curve2.Resolve(i / 100.0f);
			TTK::Graphics::DrawLine(glm::vec3(p2a, 0.0f), glm::vec3(p2b, 0.0f), { 0.0f, 0.0f, 0.0f, 1.0f });
			TTK::Graphics::DrawPoint(glm::vec3(p2b, 0.0f), 4.0f, { 0.0f, 0.0f, 0.0f, 1.0f });
			p2a = p2b;
		}
		TTK::Graphics::DrawPoint(glm::vec3(0.0f), 4.0f, { 0.0f, 0.0f, 0.0f, 1.0f });
		TTK::Graphics::DrawPoint(glm::vec3(500.0f, 0.0f, 500.0f), 4.0f, { 0.0f, 0.0f, 0.0f, 1.0f });

		TTK::Graphics::DrawText2D("Hello world!", 100.0f, 32.0f);
						
		TTK::Graphics::DrawGrid();

		TTK::Graphics::DrawSphere(glm::vec3(100, 0.0f, 100), 10.0f);
		//TTK::Graphics::DrawTeapot(glm::vec3(5, 0.0f, 5), 5.0f);
		//TTK::Graphics::DrawCube(glm::vec3(0), 10.0f);

		TTK::Graphics::EndFrame();

		TTK::Graphics::BeginGUI();
		
		ImGui::Begin("Debug");
		ImGui::Text("Hello cruel world!");
		ImGui::End();
		
		TTK::Graphics::EndGUI();

		// Present our image to windows
		glfwSwapBuffers(window);

		TTK::Input::Poll();

		lastMousePos = mousePos;
		lastFrame = thisFrame;
	}

	glfwTerminate();

	TTK::Graphics::ShutdownImGUI();
	TTK::Input::Uninitialize();
	TTK::Graphics::Cleanup();
	Logger::Uninitialize();

	return 0;
}
