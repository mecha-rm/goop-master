#include "Game.h"
#include <stdexcept>
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <GLM/gtc/matrix_transform.hpp>
#include <Logging.h>

#include <functional>

#include "SceneManager.h"
#include "MeshRenderer.h"

#include "Texture2D.h"
// #include <string>

struct TempTransform {
	glm::vec3 Position = glm::vec3(0.0f);
	glm::vec3 EulerRotation = glm::vec3(0.0f);
	glm::vec3 Scale = glm::vec3(1.0f);

	// does our TRS for us.
	glm::mat4 GetWorldTransform() const {
		return
			glm::translate(glm::mat4(1.0f), Position) *
			glm::mat4_cast(glm::quat(glm::radians(EulerRotation))) *
			glm::scale(glm::mat4(1.0f), Scale)
			;
	}
};

struct UpdateBehaviour {
	std::function<void(entt::entity e, float dt)> Function;
};

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
	switch (severity) {
	case GL_DEBUG_SEVERITY_LOW:          LOG_INFO(message); break;
	case GL_DEBUG_SEVERITY_MEDIUM:       LOG_WARN(message); break;
	case GL_DEBUG_SEVERITY_HIGH:         LOG_ERROR(message); break;
#ifdef LOG_GL_NOTIFICATIONS
	case GL_DEBUG_SEVERITY_NOTIFICATION: LOG_INFO(message); break;
#endif
	default: break;
	}
}

// call this function to  resize the window.
void GlfwWindowResizedCallback(GLFWwindow* window, int width, int height) {
	// makes sure the width and height aren't 0 before resizing.
	if (width > 0 && height > 0)
	{
		glViewport(0, 0, width, height);
		Game* game = (Game*)glfwGetWindowUserPointer(window);

		if (game != nullptr)
		{
			game->Resize(width, height);
		}
	}

}

// called when a mouse button event is recorded
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	Game* game = (Game*)glfwGetWindowUserPointer(window);

	// returns the function early if this isn't a game class
	if (game == nullptr) {
		return;
	}

	switch (action) {
	case GLFW_PRESS:
		game->MouseButtonPressed(window, button);
		break;
	case GLFW_REPEAT:
		game->MouseButtonHeld(window, button);
		break;
	case GLFW_RELEASE:
		game->MouseButtonReleased(window, button);
		break;
	}
}



// called when a cursor enters the content area window.
void CursorEnterCallback(GLFWwindow* window, int enter)
{
	Game* game = (Game*)glfwGetWindowUserPointer(window);

	if (game == nullptr) {
		return;
	}

	// sets whether the mouse cursor is in the window
	game->SetCursorInWindow(enter);
}

// called when the mouse moves over the screen, getting the position.
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	Game* game = (Game*)glfwGetWindowUserPointer(window);

	if (game == nullptr) {
		return;
	}
	else {
		game->UpdateCursorPos(xpos, ypos);
	}
}


// called when a key has been pressed, held down, or released. This function figures out which, and calls the appropriate function to handle it.
// KeyCallback(Window, Keyboard Key, Platform-Specific Scancode, Key Action, and Modifier Bits)
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Game* game = (Game*)glfwGetWindowUserPointer(window);

	if (game != nullptr)
	{
		// checks for what type of button press happened.
		switch (action)
		{
		case GLFW_PRESS: // key has been pressed
			game->KeyPressed(window, key);
			break;

		case GLFW_REPEAT: // key is held down
			game->KeyHeld(window, key);
			break;

		case GLFW_RELEASE: // key has been released
			game->KeyReleased(window, key);
			break;
		}
	}
}

Game::Viewport4 Game::viewport4;

Game::Game() :
	myWindow(nullptr),
	myWindowTitle("ICG_1 - ASN02/ASN03"),
	myClearColor(glm::vec4(0.53f, 0.53f, 0.53f, 1.0f)),
	myModelTransform(glm::mat4(1)),
	myWindowSize(800, 800) // window size (originally (600, 600)).
{ }

Game::~Game() { }

// gets the window width
float Game::GetWindowWidth() const { return myWindowSize.x; }

// gets the window height
float Game::GetWindowHeight() const { return myWindowSize.y; }

// shows whether the window is in full screen or not
bool Game::IsFullScreen() const { return fullScreen; }

// gets whether the cursor is in the window content or not.
bool Game::GetCursorInWindow() const { return mouseEnter; }

// sets whether the cursor is in the window.
void Game::SetCursorInWindow(bool inWindow) { mouseEnter = inWindow; }

// updates cursor position variables
void Game::UpdateCursorPos(double xpos, double ypos)
{
	mouseTranslation = mousePos; // previous mouse position
	mousePos = glm::vec2(xpos - this->myWindowSize.x / 2.0F, ypos - this->myWindowSize.y / 2.0F);

	mouseTranslation = mousePos - mouseTranslation; // mouse translation for the given frame.
}

// gets the cursor position
glm::vec2 Game::GetCursorPos() const { return mousePos; }

// get cursor position on the x-axis
float Game::GetCursorPosX() const { return mousePos.x; }

// returns the cursor position on the y-axis
float Game::GetCursorPosY() const { return mousePos.y; }

// called when a mouse button has been pressed
void Game::MouseButtonPressed(GLFWwindow* window, int button) {
	Game* game = (Game*)glfwGetWindowUserPointer(window);

	if (game == nullptr) // if game is 'null', then nothing happens
		return;

	// checks each button
	switch (button) {
	case GLFW_MOUSE_BUTTON_LEFT:
		mouseButtons[0] = true;
		break;
	case GLFW_MOUSE_BUTTON_MIDDLE:
		mouseButtons[1] = true;
		break;
	case GLFW_MOUSE_BUTTON_RIGHT:
		mouseButtons[2] = true;
		break;
	}
}

// mouse button is being held
void Game::MouseButtonHeld(GLFWwindow* window, int button) {
	Game* game = (Game*)glfwGetWindowUserPointer(window);

	if (game == nullptr) // if game is 'null', then nothing happens
		return;

	// checks each button
	switch (button) {
	case GLFW_MOUSE_BUTTON_LEFT:
		// mbLeft = true;
		break;
	case GLFW_MOUSE_BUTTON_MIDDLE:
		// mbMiddle = true;
		break;
	case GLFW_MOUSE_BUTTON_RIGHT:
		// mbRight = true;
		break;
	}
}

// called when a mouse button has been pressed
void Game::MouseButtonReleased(GLFWwindow* window, int button) {
	Game* game = (Game*)glfwGetWindowUserPointer(window);

	if (game == nullptr) // if game is 'null', then nothing happens
		return;

	// checks each button
	switch (button) {
	case GLFW_MOUSE_BUTTON_LEFT:
		mouseButtons[0] = false;
		break;
	case GLFW_MOUSE_BUTTON_MIDDLE:
		mouseButtons[1] = false;
		break;
	case GLFW_MOUSE_BUTTON_RIGHT:
		mouseButtons[2] = false;
		break;
	}
}

// key has been pressed
void Game::KeyPressed(GLFWwindow* window, int key)
{
	Game* game = (Game*)glfwGetWindowUserPointer(window);

	if (game == nullptr) // if game is 'null', then nothing happens
		return;

	// checks key value.
	switch (key)
	{
	case GLFW_KEY_SPACE:
		// TODO: transfer over functions for camera from GDW engine
		// myCamera->SwitchViewMode();
		break;

	case GLFW_KEY_UP: // up arrow
	case GLFW_KEY_W:
		arrowKeys[0] = true;
		break;

	case GLFW_KEY_DOWN: // down arrow
	case GLFW_KEY_S:
		arrowKeys[1] = true;
		break;

	case GLFW_KEY_LEFT: // left arrow
	case GLFW_KEY_A:
		arrowKeys[2] = true;
		break;

	case GLFW_KEY_RIGHT: // right arrow
	case GLFW_KEY_D:
		arrowKeys[3] = true;
		break;
	}
}

// key is being held
void Game::KeyHeld(GLFWwindow* window, int key)
{
	Game* game = (Game*)glfwGetWindowUserPointer(window);

	if (game == nullptr) // if game is 'null', then it is returned
		return;

	switch (key)
	{
	case GLFW_KEY_UP: // up arrow
	case GLFW_KEY_W:
		arrowKeys[0] = true;
		break;

	case GLFW_KEY_DOWN: // down arrow
	case GLFW_KEY_S:
		arrowKeys[1] = true;
		break;

	case GLFW_KEY_LEFT: // left arrow
	case GLFW_KEY_A:
		arrowKeys[2] = true;
		break;

	case GLFW_KEY_RIGHT: // right arrow
	case GLFW_KEY_D:
		arrowKeys[3] = true;
		break;
	}

}

// key hs been released
void Game::KeyReleased(GLFWwindow* window, int key)
{
	Game* game = (Game*)glfwGetWindowUserPointer(window);

	if (game == nullptr) // if game is 'null', then it is returned
		return;

	switch (key)
	{
	case GLFW_KEY_UP: // up arrow
	case GLFW_KEY_W:
		arrowKeys[0] = false;
		break;

	case GLFW_KEY_DOWN: // down arrow
	case GLFW_KEY_S:
		arrowKeys[1] = false;
		break;

	case GLFW_KEY_LEFT: // left arrow
	case GLFW_KEY_A:
		arrowKeys[2] = false;
		break;

	case GLFW_KEY_RIGHT: // right arrow
	case GLFW_KEY_D:
		arrowKeys[3] = false;
		break;
	}
}

void Game::Initialize() {
	// Initialize GLFW
	if (glfwInit() == GLFW_FALSE) {
		std::cout << "Failed to initialize GLFW" << std::endl;
		throw std::runtime_error("Failed to initialize GLFW");
	}
	// Enable transparent backbuffers for our windows (note that Windows expects our colors to be pre-multiplied with alpha)
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, true);
	// Create a new GLFW window
	// myWindow = glfwCreateWindow(600, 600, myWindowTitle, nullptr, nullptr);
	myWindow = glfwCreateWindow(myWindowSize.x, myWindowSize.y, myWindowTitle, nullptr, nullptr);

	// We want GL commands to be executed for our window, so we make our window's context the current one
	glfwMakeContextCurrent(myWindow);
	// Let glad know what function loader we are using (will call gl commands via glfw)
	if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
		std::cout << "Failed to initialize Glad" << std::endl;
		throw std::runtime_error("Failed to initialize GLAD");
	}

	// setting up window user pointer so that we can resize our window
	// Tie our game to our window, so we can access it via callbacks
	glfwSetWindowUserPointer(myWindow, this);
	// Set our window resized callback
	glfwSetWindowSizeCallback(myWindow, GlfwWindowResizedCallback);

	// callback for cursor entering the window
	glfwSetCursorEnterCallback(myWindow, CursorEnterCallback);

	// Setting cursor position callback function
	glfwSetCursorPosCallback(myWindow, CursorPosCallback);

	// Setting mouse button callback function
	glfwSetMouseButtonCallback(myWindow, MouseButtonCallback);

	// Setting keyboard callback function
	glfwSetKeyCallback(myWindow, KeyCallback);

	// glfwCallback
	
	// Log our renderer and OpenGL version
	LOG_INFO(glGetString(GL_RENDERER));
	LOG_INFO(glGetString(GL_VERSION));

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(GlDebugMessage, this);

	// used for sky boxes, which needs to be manually turned on.
	// without this, we end up getting seams in our textures.
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_SCISSOR_TEST);
}

void Game::Shutdown() {
	glfwTerminate();
}

// makes the faces face outward.
Mesh::Sptr MakeInvertedCube() {
	// Create our 4 vertices
	Vertex verts[8] = {
		// Position
		// x y z
		{{ -1.0f, -1.0f, -1.0f }}, {{ 1.0f, -1.0f, -1.0f }}, {{ -1.0f, 1.0f, -1.0f }}, {{ 1.0f, 1.0f, -1.0f }},
		{{ -1.0f, -1.0f, 1.0f }}, {{ 1.0f, -1.0f, 1.0f }}, {{ -1.0f, 1.0f, 1.0f }}, {{ 1.0f, 1.0f, 1.0f }}
	};
	// Create our 6 indices
	uint32_t indices[36] = {
	0, 1, 2, 2, 1, 3, 4, 6, 5, 6, 7, 5, // bottom / top
	0, 1, 4, 4, 1, 5, 2, 3, 6, 6, 3, 7, // front /back
	2, 4, 0, 2, 6, 4, 3, 5, 1, 3, 7, 5 // left / right
	};
	// Create a new mesh from the data
	return std::make_shared<Mesh>(verts, 8, indices, 36);
}

// chopping up the slides
Mesh::Sptr MakeSubdividedPlane(float size, int numSections, bool worldUvs = true) {
	LOG_ASSERT(numSections > 0, "Number of sections must be greater than 0!");
	LOG_ASSERT(size != 0, "Size cannot be zero!");
	// Determine the number of edge vertices, and the number of vertices and indices we'll need
	int numEdgeVerts = numSections + 1;
	size_t vertexCount = numEdgeVerts * numEdgeVerts;
	size_t indexCount = numSections * numSections * 6;
	// Allocate some memory for our vertices and indices
	Vertex* vertices = new Vertex[vertexCount];
	uint32_t* indices = new uint32_t[indexCount];
	// Determine where to start vertices from, and the step pre grid square
	float start = -size / 2.0f;
	float step = size / numSections;

	// vertices
	// Iterate over the grid's edge vertices
	for (int ix = 0; ix <= numSections; ix++) {
		for (int iy = 0; iy <= numSections; iy++) {
			// Get a reference to the vertex so we can modify it
			Vertex& vert = vertices[ix * numEdgeVerts + iy];
			// Set its position
			vert.Position.x = start + ix * step;
			vert.Position.y = start + iy * step;
			vert.Position.z = 0.0f;
			// Set its normal
			vert.Normal = glm::vec3(0, 0, 1);
			// The UV will go from [0, 1] across the entire plane (can change this later)
			if (worldUvs) {
				vert.UV.x = vert.Position.x;
				vert.UV.y = vert.Position.y;
			}
			else {
				vert.UV.x = vert.Position.x / size;
				vert.UV.y = vert.Position.y / size;
			}
			// Flat white color
			vert.Color = glm::vec4(1.0f);
		}
	}
	
	// indices
	// We'll just increment an index instead of calculating it
	uint32_t index = 0;
	// Iterate over the quads that make up the grid
	for (int ix = 0; ix < numSections; ix++) {
		for (int iy = 0; iy < numSections; iy++) {
			// Determine the indices for the points on this quad
			uint32_t p1 = (ix + 0) * numEdgeVerts + (iy + 0);
			uint32_t p2 = (ix + 1) * numEdgeVerts + (iy + 0);
			uint32_t p3 = (ix + 0) * numEdgeVerts + (iy + 1);
			uint32_t p4 = (ix + 1) * numEdgeVerts + (iy + 1);
			// Append the quad to the index list
			indices[index++] = p1;
			indices[index++] = p2;
			indices[index++] = p3;
			indices[index++] = p3;
			indices[index++] = p2;
			indices[index++] = p4;
		}
	}

	// returning the mesh
	// Create the result, then clean up the arrays we used
	Mesh::Sptr result = std::make_shared<Mesh>(vertices, vertexCount, indices, indexCount);
	delete[] vertices;
	delete[] indices;
	// Return the result
	return result;
}

// loads the content for the meshes and shaders
void Game::LoadContent() 
{
	std::string sceneName = "Main";
	const float HEIGHT_SCALAR = 10.0F;

	myCamera = std::make_shared<Camera>();
	myCamera->SetPosition(glm::vec3(5, 5, 5));
	myCamera->LookAt(glm::vec3(0));

	// sets the camera to perspective mode.
	myCamera->Projection = glm::perspective(glm::radians(60.0f), 1.0f, 0.01f, 1000.0f);
	perspectiveCamera = true; // in perspective mode.


	// Create our 4 vertices
	// we're using an initalizer list inside an initializer list to get the data
	Vertex vertices[4] = {
		// Position				  Color							Normal	UV
		//  x      y	 z		   r	 g	   b	 a		   x  y  z	u		v
		{{ -2.5f, -2.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, {0, 0, 1}, {1.0f, 0.0f}},
		{{ 2.5f, -2.5f, 0.0f },  { 1.0f, 1.0f, 1.0f, 1.0f }, {0, 0, 1}, {0.0f, 0.0f}},
		{{ -2.5f, 2.5f, 0.0f },  { 1.0f, 1.0f, 1.0f, 1.0f }, {0, 0, 1}, {1.0f, 1.0f}},
		{{ 2.5f, 2.5f, 0.0f },   { 1.0f, 1.0f, 1.0f, 1.0f }, {0, 0, 1}, {0.0f, 1.0f}},
	};
	// Create our 6 indices
	uint32_t indices[6] = {
	0, 1, 2,
	2, 1, 3
	};

	// plane

	// Create a new mesh from the data (originallybetween phong and before planeMat)
	myMesh = std::make_shared<Mesh>(vertices, 4, indices, 6);


	// default phong shader
	Shader::Sptr phong = std::make_shared<Shader>();
	phong->Load("lighting.vs.glsl", "blinn-phong.fs.glsl");

	// terrain shader
	Shader::Sptr terrain = std::make_shared<Shader>();	
	terrain->Load("terrain.vs.glsl", "terrain.fs.glsl");

	// added for mip mapping. As long as its above the material, it's fine.
	SamplerDesc description = SamplerDesc();
	description.MinFilter = MinFilter::LinearMipNearest;
	description.MagFilter = MagFilter::Linear;
	description.WrapS = description.WrapT = WrapMode::Repeat;
	TextureSampler::Sptr Linear = std::make_shared<TextureSampler>(description);

	// Not being used anymore
	desc1 = SamplerDesc();
	desc1.MinFilter = MinFilter::NearestMipNearest;
	desc1.MagFilter = MagFilter::Nearest;

	// Not being used anymore
	desc2 = SamplerDesc();
	desc2.MinFilter = MinFilter::LinearMipLinear;
	desc2.MagFilter = MagFilter::Linear;

	samplerEX = std::make_shared<TextureSampler>(desc1);
	usingDesc1 = true; // using description one.

	// Material::Sptr planeMat = std::make_shared<Material>(phong);
	Material::Sptr planeMat = std::make_shared<Material>(terrain);  
	planeMat->Set("a_LightPos", { 0, 0, 7 });
	planeMat->Set("a_LightColor", { 0.9f, 0.1f, 0.1F });
	planeMat->Set("a_AmbientColor", { 1.0f, 1.0f, 1.0f });
	planeMat->Set("a_AmbientPower", 0.9f); // change this to change the main lighting power (originally value of 0.1F)
	planeMat->Set("a_LightSpecPower", 0.1f);
	planeMat->Set("a_LightShininess", 200.0f); // must be set as a float for proper casting
	planeMat->Set("a_LightAttenuation", 0.0001f); // the lower the attenuation, the stronger the line
	// planeMat->Set("s_Albedo", albedo); // right now, this is using the texture state.
	// planeMat->Set("s_Albedo", albedo, Linear); // now uses mip mappin 
	
	// setting the heightmap and related textures

	// comment out this line if you're not using the terrain shader
	planeMat->Set("a_TextureSampler", Texture2D::LoadFromFile("heightmap.bmp"), Linear); // terrain shader exclusive
	planeMat->Set("a_HeightScalar", HEIGHT_SCALAR); // terrain shader exclusive
	 
	// bottom, middle, top (order matters)
	planeMat->Set("s_Albedos[0]", Texture2D::LoadFromFile("sand.png"), Linear); // sand: on level, and below the water (originally 'dirt')
	planeMat->Set("s_Albedos[1]", Texture2D::LoadFromFile("grass_02.png"), Linear); // grass: medium terrain height
	planeMat->Set("s_Albedos[2]", Texture2D::LoadFromFile("rock.png"), Linear); // rocks: highest terrain (originally 'snow')

	Material::Sptr objMat = std::make_shared<Material>(phong);
	objMat->Set("a_LightPos", { 0, 0, 5 });
	objMat->Set("a_LightColor", { 1.0f, 0.05f, 0.05F });
	objMat->Set("a_AmbientColor", { 0.05f, 0.05f, 1.0f });
	objMat->Set("a_AmbientPower", 1.0f); // change this to change the main lighting power (originally value of 0.1F)
	objMat->Set("a_LightSpecPower", 0.9f);
	objMat->Set("a_LightShininess", 256.0f); // must be set as a float for proper casting
	objMat->Set("a_LightAttenuation", 0.0001f); // the lower the attenuation, the stronger the line

	// bottom, middle, top (order matters)
	objMat->Set("s_Albedos[0]", Texture2D::LoadFromFile("default.png"), Linear);
	objMat->Set("s_Albedos[1]", Texture2D::LoadFromFile("default.png"), Linear);
	objMat->Set("s_Albedos[2]", Texture2D::LoadFromFile("default.png"), Linear);

	SceneManager::RegisterScene(sceneName);
// 	SceneManager::RegisterScene("Test2");
	SceneManager::SetCurrentScene(sceneName);

	// we need to make the scene before we can attach things to it.
	auto scene = CurrentScene();
	scene->SkyboxShader = std::make_shared<Shader>();
	scene->SkyboxShader->Load("cubemap.vs.glsl", "cubemap.fs.glsl");
	scene->SkyboxMesh = MakeInvertedCube();

	// loads in six files out of res, then making them into the cube map.
	std::string files[6] = {
	std::string("cubemap/graycloud_lf.jpg"),
	std::string("cubemap/graycloud_rt.jpg"),
	std::string("cubemap/graycloud_dn.jpg"),
	std::string("cubemap/graycloud_up.jpg"),
	std::string("cubemap/graycloud_ft.jpg"),
	std::string("cubemap/graycloud_bk.jpg")
	};
	scene->Skybox = TextureCube::LoadFromFiles(files);

	// Plane
	{
		// adds an entity to one of the scenes
		auto& ecs = GetRegistry(sceneName);
		entt::entity e1 = ecs.create();
		MeshRenderer& m1 = ecs.assign<MeshRenderer>(e1);
		m1.Material = planeMat;
		// m1.Material = matEX;
		// m1.Mesh = myMesh;
		m1.Mesh = MakeSubdividedPlane(15.0f, 30, false);

		auto rotate = [](entt::entity e, float dt) {
			auto& transform = CurrentRegistry().get_or_assign<TempTransform>(e);
			// transform.EulerRotation += glm::vec3(0, 0, 90 * dt);

			// does the same thing, except all in one line.
			// CurrentRegistry().get_or_assign<TempTransform>(e).EulerRotation += glm::vec3(0, 0, 90 * dt);
		};
		auto& up = ecs.get_or_assign<UpdateBehaviour>(e1);
		up.Function = rotate;
	}


	// Create and compile shader
	myShader = std::make_shared<Shader>();
	// myShader->Compile(vs_source, fs_source); // no longer needed since we have a dedicated file.
	myShader->Load("passthrough.vert.glsl", "passthrough.frag.glsl");
	
	myModelTransform = glm::mat4(1.0f);


	// Making the water shader
	{ // Push a new scope so that we don't step on other names
		Shader::Sptr waterShader = std::make_shared<Shader>();
		waterShader->Load("water-shader.vs.glsl", "water-shader.fs.glsl");
		Material::Sptr waterMaterial = std::make_shared<Material>(waterShader);
		waterMaterial->HasTransparency = true;


		waterMaterial->Set("a_EnabledWaves", 3); // number of waves
		waterMaterial->Set("a_Gravity", 9.81f);
		// Format is: [xDir, yDir, "steepness", wavelength] (note that the sum of steepness should be < 1 to avoid loops)
		waterMaterial->Set("a_Waves[0]", { 1.0f, 0.0f, 0.50f, 6.0f }); // last parameter is wave length
		waterMaterial->Set("a_Waves[1]", { 0.0f, 1.0f, 0.25f, 3.1f }); // last parameter is wave length 
		waterMaterial->Set("a_Waves[2]", { 1.0f, 1.4f, 0.20f, 1.8f }); // last parameter is wave length
		waterMaterial->Set("a_WaterAlpha", 0.35f);
		waterMaterial->Set("a_WaterColor", { 0.5f, 0.5f, 0.95f });
		waterMaterial->Set("a_WaterClarity", 0.9f); // 0.9f so that it's fairly clear
		waterMaterial->Set("a_FresnelPower", 0.5f); // the higher the power, the higher the reflection
		waterMaterial->Set("a_RefractionIndex", 1.0f / 1.34f); // bending light; 1.0/1.34 goes from air to water.
		waterMaterial->Set("s_Environment", scene->Skybox);
		 
		  auto& ecs = GetRegistry(sceneName); // If you've changed the name of the scene, you'll need to modify this!
		  entt::entity e1 = ecs.create();
		  MeshRenderer& m1 = ecs.assign<MeshRenderer>(e1);
		  m1.Material = waterMaterial;
		  m1.Mesh = MakeSubdividedPlane(20.0f, 100);

		  auto& transform = CurrentRegistry().get_or_assign<TempTransform>(e1);
		  transform.Position.z = HEIGHT_SCALAR * 0.3;
	}

	// loading in an object (optional)
	objects.push_back(new Object("icosphere.obj", sceneName, objMat));
	objects.at(objects.size() - 1)->SetPosition(-3.0F, 0.0F, 5.0F);
	objects.at(objects.size() - 1)->SetScale(glm::vec3(0.5F, 0.5F, 0.5F));

}

void Game::UnloadContent() {
}

void Game::Update(float deltaTime) {
	glm::vec3 camTranslation = glm::vec3(0.0f); // the variable for the camera translation
	glm::vec3 camRotation = glm::vec3(0.0f); // the variable for the camera rotation (Perspective exclusive)

	float translation_inc = 2.5f; // incremeter for translation
	float rotation_inc = 1.0f; // incrementer for rotation

	// checking for the right window
	if (mouseButtons[0]) // left button
	{
		/*
		 * for viewports, the bottom left-hand corner is (0, 0), with the top right-hand corner being the positive x and y.
		 * for the mouse position, the centre of the screen is considered (0, 0)
			* the bottom left corner is (-x, -y), and the top right corner is (+x, +y)
		 * as such, the mouse position must be offset to move the origin it references.
		*/
		glm::ivec4 viewport;

		// checking which viewpoint the mouse cursor is in
		for (int i = 1; i <= 4; i++)
		{
			// getting the placement and size of each viewport
			switch (i)
			{
			case 1:
				viewport = viewport4.GetViewportBottomLeft(myWindowSize);
				break;

			case 2:
				viewport = viewport4.GetViewportBottomRight(myWindowSize);
				break;

			case 3:
				viewport = viewport4.GetViewportTopLeft(myWindowSize);
				break;

			case 4:
				viewport = viewport4.GetViewportTopRight(myWindowSize);
				break;
			}
			// std::cout << "vp: (" << viewport.x << ", " << viewport.y << ", " << viewport.z << ", " << viewport.w << ")" << std::endl;

			// checking if the mouse is inside the current view
			// since the mouse origin is the window centre while the viewpoint position is based on the bottom left, the mouse is adjusted.
			if (mousePos.x + myWindowSize.x / 2.0F >= viewport.x && mousePos.x + myWindowSize.x / 2.0F <= viewport.x + viewport.z &&
				mousePos.y + myWindowSize.y / 2.0F >= viewport.y && mousePos.y + myWindowSize.y / 2.0F <= viewport.y + viewport.w)
			{
				viewport4.activeView = i; // saving the active view
				break;
			}
		}
	}

	// Moving the camera
	if (arrowKeys[0]) // up key/w
	{
		camTranslation.z += translation_inc * deltaTime;
	}
	else if (arrowKeys[1]) // down key/s
	{
		camTranslation.z -= translation_inc * deltaTime;
	}
	else if (arrowKeys[2]) // left key/a
	{
		camTranslation.x += translation_inc * deltaTime;
	}
	else if (arrowKeys[3]) // right key/d
	{
		camTranslation.x -= translation_inc * deltaTime;
	}

	
	// checking the active view for the camera transformation // no rotation used
	switch(viewport4.activeView)
	{
	case 1: // orthographic top
		viewport4.orthoTop->SetPosition(viewport4.orthoTop->GetPosition() + camTranslation);
		// viewport4.orthoTop->Rotate(camRotation); // no rotations used
		break;

	case 2: // orthographic front
		viewport4.orthoFront->SetPosition(viewport4.orthoFront->GetPosition() + camTranslation);
		// viewport4.orthoFront->Rotate(camRotation); // no rotations used
		break;

	case 3: // orthographic left
		viewport4.orthoLeft->SetPosition(viewport4.orthoLeft->GetPosition() + camTranslation);
		// viewport4.orthoLeft->Rotate(camRotation); // no rotations used
		break;

	case 4: // perspective
		viewport4.perspective->SetPosition(viewport4.perspective->GetPosition() + camTranslation);
		viewport4.perspective->Rotate(camRotation); // unused

		// if the right button is being held
		if (mouseButtons[2])
		{
			glm::ivec2 viewportCentre = { myWindowSize.x / 4.0F, myWindowSize.y / 4.0F };
			float speed = 0.05;

			viewport4.perspective->LookAt(glm::vec3((mousePos.x - viewportCentre.x) * speed, 0.0F, (mousePos.y - viewportCentre.y) * speed));
			// myCamera->GetView();
		}

		break;
	}

	// calling all of our functions for our update behaviours.
	auto view = CurrentRegistry().view<UpdateBehaviour>();
	for (const auto& e : view) {
		auto& func = CurrentRegistry().get<UpdateBehaviour>(e);
		if (func.Function) {
			func.Function(e, deltaTime);
		}
	}
}

void Game::InitImGui() {
	// Creates a new ImGUI context
	ImGui::CreateContext();
	// Gets our ImGUI input/output
	ImGuiIO& io = ImGui::GetIO();
	// Enable keyboard navigation
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	// Allow docking to our window
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	// Allow multiple viewports (so we can drag ImGui off our window)
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	// Allow our viewports to use transparent backbuffers
	io.ConfigFlags |= ImGuiConfigFlags_TransparentBackbuffers;
	// Set up the ImGui implementation for OpenGL
	ImGui_ImplGlfw_InitForOpenGL(myWindow, true);
	ImGui_ImplOpenGL3_Init("#version 410");

	// Dark mode FTW
	ImGui::StyleColorsDark();
	// Get our imgui style
	ImGuiStyle& style = ImGui::GetStyle();
	//style.Alpha = 1.0f;
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 0.8f;
	}
}

void Game::ShutdownImGui() {
	// Cleanup the ImGui implementation
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	// Destroy our ImGui context
	ImGui::DestroyContext();
}

void Game::ImGuiNewFrame() {
	// Implementation new frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	// ImGui context new frame
	ImGui::NewFrame();
}

void Game::ImGuiEndFrame() {
	// Make sure ImGui knows how big our window is
	ImGuiIO& io = ImGui::GetIO();
	int width{ 0 }, height{ 0 };
	glfwGetWindowSize(myWindow, &width, &height);
	io.DisplaySize = ImVec2(width, height);
	// Render all of our ImGui elements
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	// If we have multiple viewports enabled (can drag into a new window)
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		// Update the windows that ImGui is using
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		// Restore our gl context
		glfwMakeContextCurrent(myWindow);
	}
}

void Game::Run()
{
	Initialize();
	InitImGui();
	LoadContent();
	static float prevFrame = glfwGetTime();
	// Run as long as the window is open
	while (!glfwWindowShouldClose(myWindow)) {
		// Poll for events from windows
		// clicks, key presses, closing, all that
		glfwPollEvents();
		float thisFrame = glfwGetTime();
		float deltaTime = thisFrame - prevFrame;
		Update(deltaTime);
		Draw(deltaTime);
		ImGuiNewFrame();
		DrawGui(deltaTime);
		ImGuiEndFrame();
		prevFrame = thisFrame;
		// Present our image to windows
		glfwSwapBuffers(myWindow);

		// added so that the previous frame is updated. Otherwise, it would just be since the beginning of the program.
		prevFrame = thisFrame;
	}
	UnloadContent();
	ShutdownImGui();
	Shutdown();
}

// resizes the window and keeps size proportionate.
void Game::Resize(int newWidth, int newHeight)
{
	myWindowSize = { newWidth, newHeight }; // updating window size
	// set to float since we're calculating the new projecction as the screen size.
	if(perspectiveCamera) // camera is in perspective mode
	{
		myCamera->Projection = glm::perspective(glm::radians(60.0f), newWidth / (float)newHeight, 0.01f, 1000.0f);
	}
	else // camera is in orthographic mode
	{
		myCamera->Projection = glm::ortho(-5.0f * newWidth / (float)newHeight, 5.0f * newWidth / (float)newHeight, -5.0f, 5.0f, 0.0f, 100.0f);
	}

	viewport4.ResizeViewports(newWidth, newHeight); // resizes the viewports
	
}

void Game::Draw(float deltaTime) {
	// Draw will now render the viewpoints

	// bottom of the window
	//glm::ivec4 viewport = {
	//	0, 0,
	//	myWindowSize.x, myWindowSize.y / 2
	//};
	//__RenderScene(viewport, myCamera);


	//// top of the window
	//glm::ivec4 viewport2 = {
	//	0, myWindowSize.y / 2,
	//	myWindowSize.x, myWindowSize.y / 2
	//};
	//__RenderScene(viewport2, myCamera);

	// wireframe mode : fill mode.
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	// NOTE: (bottom left-hand corner is (0, 0))
	// the four views; the border colour passed is based on the active view
	// top left - orthographic top
	__RenderScene(viewport4.GetViewportTopLeft(myWindowSize), viewport4.orthoTop, 
		(viewport4.activeView == 1) ? viewport4.borderColorActive : viewport4.borderColorInactive);
	
	// top right - orthographic front
	__RenderScene(viewport4.GetViewportTopRight(myWindowSize), viewport4.orthoFront,
		(viewport4.activeView == 2) ? viewport4.borderColorActive : viewport4.borderColorInactive);
	
	// bottom left - ortographic left
	__RenderScene(viewport4.GetViewportBottomLeft(myWindowSize), viewport4.orthoLeft,
		(viewport4.activeView == 3) ? viewport4.borderColorActive : viewport4.borderColorInactive);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	// bottom right - perspective
	__RenderScene(viewport4.GetViewportBottomRight(myWindowSize), viewport4.perspective,
		(viewport4.activeView == 4) ? viewport4.borderColorActive : viewport4.borderColorInactive);
}

void Game::DrawGui(float deltaTime) {
	// Open a new ImGui window
	ImGui::Begin("Colour Picker");
	
	// Draw widgets here
	// ImGui::SliderFloat4("Color", &myClearColor.x, 0, 1); // Original
	ImGui::ColorPicker4("Color", &myClearColor.x); // new version
	if (ImGui::InputText("Title", myWindowTitle, 31))
	{
		glfwSetWindowTitle(myWindow, myWindowTitle);
	}

	if (ImGui::Button("Apply")) // adding another button, which allows for the application of the window title.
	{
		glfwSetWindowTitle(myWindow, myWindowTitle);
	}

	ImGui::Text("C: Change Camera Mode");
	// ImGui::Text(("Time: " + std::to_string(glfwGetTime())).c_str()); // requires inclusion of <string>

	// draws a button for each scene name.
	auto it = SceneManager::Each();
	for (auto& kvp : it) {
		if (ImGui::Button(kvp.first.c_str())) {
			SceneManager::SetCurrentScene(kvp.first);
		}
	}

	ImGui::End();

	// ImGui::Combo

	// Creating a Second Window
	//ImGui::Begin("Test 2");
	//ImGui::Text("Hello Cruel World!");
	//ImGui::End();
}

// Now handles rendering the scene.
void Game::__RenderScene(glm::ivec4 viewport, Camera::Sptr camera, glm::vec4 borderColor)
{
	static bool wireframe = false; // used to switch between fill mode and wireframe mode for draw calls.

	// Set viewport to entire region
	// glViewport(viewport.x, viewport.y, viewport.z, viewport.w); // not neded since viewpoint doesn't change the clear call.
	glScissor(viewport.x, viewport.y, viewport.z, viewport.w);
	
	int border = viewport4.borderSize;

	// Clear with the border color
	glClearColor(borderColor.x, borderColor.y, borderColor.z, borderColor.w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	
	// Set viewport to be inset slightly (the amount is the border width)
	// the offsets are used to move the border relative to the viewpoint.
	glViewport(viewport.x + border, viewport.y + border, viewport.z - 2 * border, viewport.w - 2 * border);
	glScissor(viewport.x + border, viewport.y + border, viewport.z - 2 * border, viewport.w - 2 * border);
	
	// Clear our new inset area with the scene clear color
	glClearColor(myClearColor.x, myClearColor.y, myClearColor.z, myClearColor.w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// We'll grab a reference to the ecs to make things easier
	auto& ecs = CurrentRegistry();

	
	// replaced to allow for transparency.
	//// We sort our mesh renderers based on material properties
	//// This will group all of our meshes based on shader first, then material second
	//ecs.sort<MeshRenderer>([](const MeshRenderer& lhs, const MeshRenderer& rhs) {
	//	if (rhs.Material == nullptr || rhs.Mesh == nullptr)
	//		return false;
	//	else if (lhs.Material == nullptr || lhs.Mesh == nullptr)
	//		return true;
	//	else if (lhs.Material->GetShader() != rhs.Material->GetShader())
	//		return lhs.Material->GetShader() < rhs.Material->GetShader();
	//	else
	//		return lhs.Material < rhs.Material;
	//	});

	ecs.sort<MeshRenderer>([&](const MeshRenderer& lhs, const MeshRenderer& rhs) {
		if (rhs.Material == nullptr || rhs.Mesh == nullptr)
			return false;
		else if (lhs.Material == nullptr || lhs.Mesh == nullptr)
			return true;
		else if (lhs.Material->HasTransparency & !rhs.Material->HasTransparency) //
			return false; // This section is new
		else if (!lhs.Material->HasTransparency & rhs.Material->HasTransparency) // The order IS important
			return true; //
		else if (lhs.Material->GetShader() != rhs.Material->GetShader())
			return lhs.Material->GetShader() < rhs.Material->GetShader();
		else
			return lhs.Material < rhs.Material;
		});

	// moved to be above the draw calls for the other objects so that depth buffering works properly.
	auto scene = CurrentScene();
	// Draw the skybox after everything else, if the scene has one
	if (scene->Skybox)
	{
		// Disable culling
		glDisable(GL_CULL_FACE); // we disable face culling if the cube map is screwed up.
		// Set our depth test to less or equal (because we are at 1.0f)
		glDepthFunc(GL_LEQUAL);
		// Disable depth writing
		glDepthMask(GL_FALSE);

		// Make sure no samplers are bound to slot 0
		TextureSampler::Unbind(0);
		// Set up the shader
		scene->SkyboxShader->Bind();

		// casting the mat4 down to a mat3, then putting it back into a mat4, which is done to remove the camera's translation.
		scene->SkyboxShader->SetUniform("a_View", glm::mat4(glm::mat3(
			camera->GetView()
		)));
		scene->SkyboxShader->SetUniform("a_Projection", camera->Projection);

		scene->Skybox->Bind(0);
		scene->SkyboxShader->SetUniform("s_Skybox", 0); // binds our skybox to slot 0.
		scene->SkyboxMesh->Draw();

		// Restore our state
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);
		glDepthFunc(GL_LESS);
	}

	// These will keep track of the current shader and material that we have bound
	Material::Sptr mat = nullptr;
	Shader::Sptr boundShader = nullptr;
	// A view will let us iterate over all of our entities that have the given component types
	auto view = ecs.view<MeshRenderer>();

	for (const auto& entity : view) {
		// Get our shader
		const MeshRenderer& renderer = ecs.get<MeshRenderer>(entity);
		// Early bail if mesh is invalid
		if (renderer.Mesh == nullptr || renderer.Material == nullptr)
			continue;
		// If our shader has changed, we need to bind it and update our frame-level uniforms
		if (renderer.Material->GetShader() != boundShader) {
			boundShader = renderer.Material->GetShader();
			boundShader->Bind();
			boundShader->SetUniform("a_CameraPos", camera->GetPosition());
			boundShader->SetUniform("a_Time", static_cast<float>(glfwGetTime())); // passing in the time.
		}
		// If our material has changed, we need to apply it to the shader
		if (renderer.Material != mat) {
			mat = renderer.Material;
			mat->Apply();
		}

		// We'll need some info about the entities position in the world
		const TempTransform& transform = ecs.get_or_assign<TempTransform>(entity);
		// Get the object's transformation
		glm::mat4 worldTransform = transform.GetWorldTransform();
		// Our normal matrix is the inverse-transpose of our object's world rotation
		// Recall that everything's backwards in GLM
		glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(worldTransform)));

		// Update the MVP using the item's transform
		mat->GetShader()->SetUniform(
			"a_ModelViewProjection",
			camera->GetViewProjection() *
			worldTransform);
		// Update the model matrix to the item's world transform
		mat->GetShader()->SetUniform("a_Model", worldTransform);
		// Update the model matrix to the item's world transform
		mat->GetShader()->SetUniform("a_NormalMatrix", normalMatrix);
		
		// TODO: add ability to turn face culling on and off for a given object
		// draws the item
		if (renderer.Mesh->IsVisible())
		{
			// if the mesh is in wireframe mode, and the draw call isn't set to that already.
			if (renderer.Mesh->IsWireframe() != wireframe)
			{
				wireframe = !wireframe;

				// switches between wireframe mode and fill mode.
				(wireframe) ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}

			renderer.Mesh->Draw();
		}
	}

	// this is where the skybox was originally.
}

Game::Viewport4::Viewport4()
{
	{
		// myCamera->Projection = (perspectiveCamera) ?
		// glm::perspective(glm::radians(60.0f), 1.0f, 0.01f, 1000.0f) : glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.0f, 100.0f);
		// default values
		// top orthographic view
		orthoTop = std::make_shared<Camera>();
		orthoTop->SetPosition(glm::vec3(0, 3, 0));
		orthoTop->LookAt(glm::vec3(0));
		orthoTop->Projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.0f, 1000.0f);

		// front orthographic view
		orthoFront = std::make_shared<Camera>();
		orthoFront->SetPosition(glm::vec3(0, 0, 3));  
		orthoFront->LookAt(glm::vec3(0), glm::vec3(0, 1, 0));
		orthoFront->Projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.0f, 1000.0f);

		// left orthographic view
		orthoLeft = std::make_shared<Camera>();
		orthoLeft->SetPosition(glm::vec3(-3, 0, 0));
		orthoLeft->LookAt(glm::vec3(0));
		orthoLeft->Projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.0f, 1000.0f);

		// perspective view
		perspective = std::make_shared<Camera>();
		perspective->SetPosition(glm::vec3(5, 5, 1));
		perspective->LookAt(glm::vec3(0));
		perspective->Projection = glm::perspective(glm::radians(60.0f), 1.0f, 0.01f, 1000.0f);
	}
}

// viewports - the bottom left-hand corner is (0, 0), and the top right corner is the maximum (x, y)
// top left
glm::ivec4 Game::Viewport4::GetViewportTopLeft(glm::ivec2 windowSize)
{
	return glm::ivec4(0, windowSize.y / 2, windowSize.x / 2, windowSize.y / 2);
}

// top right
glm::ivec4 Game::Viewport4::GetViewportTopRight(glm::ivec2 windowSize)
{
	return glm::ivec4(windowSize.x / 2, windowSize.y / 2, windowSize.x / 2, windowSize.y / 2);
}

// bottom left
glm::ivec4 Game::Viewport4::GetViewportBottomLeft(glm::ivec2 windowSize)
{
	return glm::ivec4(0, 0, windowSize.x / 2, windowSize.y / 2);
}

// bottom right
glm::ivec4 Game::Viewport4::GetViewportBottomRight(glm::ivec2 windowSize)
{
	return glm::ivec4(windowSize.x / 2, 0, windowSize.x / 2, windowSize.y / 2);
}

// resizes the viewports
void Game::Viewport4::ResizeViewports(int newWindowWidth, int newWindowHeight)
{
	// resizes the four viewports
	// orthographic top
	orthoTop->Projection = glm::ortho(-5.0f * newWindowWidth / (float)newWindowHeight, 5.0f * newWindowWidth / (float)newWindowHeight, -5.0f, 5.0f, 0.0f, 100.0f);

	// front orthographic view
	orthoFront->Projection = glm::ortho(-5.0f * newWindowWidth / (float)newWindowHeight, 5.0f * newWindowWidth / (float)newWindowHeight, -5.0f, 5.0f, 0.0f, 100.0f);

	// left orthographic view
	orthoLeft->Projection = glm::ortho(-5.0f * newWindowWidth / (float)newWindowHeight, 5.0f * newWindowWidth / (float)newWindowHeight, -5.0f, 5.0f, 0.0f, 100.0f);

	// perspective view
	perspective->Projection = glm::perspective(glm::radians(60.0f), newWindowWidth / (float)newWindowHeight, 0.01f, 1000.0f);
}
