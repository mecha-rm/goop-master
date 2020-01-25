#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "GLM/glm.hpp"

#include "Shader.h"
#include "Mesh.h"
#include "Camera.h"

#include "Material.h" // EX Inclusion
#include "objects/Object.h"

class Game {
public:
	// class for four viewpoints
	struct Viewport4
	{
		// the four cameras
		Camera::Sptr orthoTop; // orthographic (top)
		Camera::Sptr orthoFront; // orthographic (front)
		Camera::Sptr orthoLeft; // orthographic (left)
		Camera::Sptr perspective; // perspective

		// border colour
		glm::vec4 borderColorInactive = { 0.400f, 0.400f, 0.400f, 1.000f };
		glm::vec4 borderColorActive = {0.710f, 0.019f, 0.007f, 1.000f};

		// border size
		int borderSize = 5;

		// the active viewpoint
		int activeView = 0; 

		Viewport4();

		// viewports - the bottom left-hand corner is (0, 0)
		// gets a viewport that goes in the top left
		glm::ivec4 GetViewportTopLeft(glm::ivec2 windowSize);

		// gets a viewport that goes in the top right
		glm::ivec4 GetViewportTopRight(glm::ivec2 windowSize);

		// gets a viewport that goes in the bottom left
		glm::ivec4 GetViewportBottomLeft(glm::ivec2 windowSize);

		// gets a viewport that goes in the bottom right
		glm::ivec4 GetViewportBottomRight(glm::ivec2 windowSize);

		// called when the screen is resized to resize the viewports
		void ResizeViewports(int newWindowWidth, int newWindowHeight);
	};

	Game();
	
	~Game();
	
	// gets the window width
	float GetWindowWidth() const;

	// gets the window height
	float GetWindowHeight() const;

	// gets whether the window is full-screen or not. 
	bool IsFullScreen() const;

	// resize has been moved to the bottom of the code to be accurate to where it is in the framework.


	// returns 'true' if the cursor is in the window content, and false if it's not.
	bool GetCursorInWindow() const;

	// called to set whether or not the mouse cursor is in the window. This is excluively for the glfwCursorEnterCallback.
	void SetCursorInWindow(bool inWindow);

	// gets the cursor position as a cherry::Vec2
	glm::vec2 GetCursorPos() const;

	// gets the cursor position on the x-axis
	float GetCursorPosX() const;

	// gets the cursor position on the y-axis
	float GetCursorPosY() const;

	// updates the cursor position variables when callback fuciton is called
	virtual void UpdateCursorPos(double xpos, double ypos);


	// called when a mouse button has been pressed
	virtual void MouseButtonPressed(GLFWwindow* window, int button);

	// called when a mouse button is being held
	virtual void MouseButtonHeld(GLFWwindow* window, int button);

	// called when a mouse button has been pressed
	virtual void MouseButtonReleased(GLFWwindow* window, int button);


	// called when a key has been pressed
	virtual void KeyPressed(GLFWwindow* window, int key);

	// called when a key is being held down
	virtual void KeyHeld(GLFWwindow* window, int key);

	// called when a key has been released
	virtual void KeyReleased(GLFWwindow* window, int key);

	void Run();

	void Resize(int newWidth, int newHeight);

protected:
	void Initialize();
	
	void Shutdown();
	
	void LoadContent();
	
	void UnloadContent();
	
	void InitImGui();
	
	void ShutdownImGui();
	
	void ImGuiNewFrame();
	
	void ImGuiEndFrame();
	
	void Update(float deltaTime);
	
	void Draw(float deltaTime);
	
	void DrawGui(float deltaTime);


	glm::ivec2 myWindowSize; // saves the window size

	void __RenderScene(glm::ivec4 viewport, Camera::Sptr camera, glm::vec4 borderColor); // used for rendering hte scene

private:
	// Stores the main window that the game is running in
	GLFWwindow* myWindow;

	// Stores the clear color of the game's window
	glm::vec4 myClearColor;
	
	// Stores the title of the game's window
	char myWindowTitle[32];

	// A shared pointer to our mesh
	Mesh::Sptr myMesh;
	
	// A shared pointer to our shader
	Shader::Sptr myShader;

	// Extra Shader for exercise.
	Mesh::Sptr myMesh2;
	// Shader::Sptr myShader2; // not needed

	// the four cameras.
	Camera::Sptr myCamera;
	bool perspectiveCamera = false; // if 'true', the perpsective camera is used. If 'false', orthographic camera is used.

	// EX - used for exercise
	SamplerDesc desc1; // sampler 1
	SamplerDesc desc2; // sampler 2
	Texture2D::Sptr albedoEX; // albedo
	TextureSampler::Sptr samplerEX; // sampler
	Material::Sptr matEX; // material
	bool usingDesc1 = false; // if 'true', then description 1 is being used

	// Our models transformation matrix
	glm::mat4 myModelTransform;

	// movement
	// [0] = up key, [1] = down key, [2] = left key, [3] = right key
	bool arrowKeys[4] = { false, false, false, false };
	// bool w = false, a = false, s = false, d = false;

	// gets the cursor position; this is based off of the centre of the window.
	glm::vec2 mousePos;

	// the difference between the mouse's previous position and current position.
	glm::vec2 mouseTranslation;

	// [0] = left button, [1] = middle button, [2] = right button
	bool mouseButtons[3] = { false, false, false };

	// window size
	// unsigned int windowWidth = 850;
	// unsigned int windowHeight = 850;

	// boolean for full screen
	bool fullScreen;

	// returns 'true' if the mouse is in the window content, false otherwise.
	bool mouseEnter = false;

	std::vector<Object*> objects;

	// the four viewpoints
	static Viewport4 viewport4;
};