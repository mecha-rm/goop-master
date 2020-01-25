#include "Game.h"
#include "Logging.h"

#include <stdexcept>

#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "imgui.h"
#include "imgui_impl_opengl3.cpp"
#include "imgui_impl_glfw.cpp"

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp> 
#include <GLM/gtc/random.hpp>

#include "SceneManager.h"
#include "MeshRenderer.h"
#include "Material.h"

#include "Texture2D.h"
#include "TextureCube.h"
#include "TextureSampler.h"
#include "ObjLoader.h"

#include "MemoryTracking.h"

#include <functional>

struct UpdateBehaviour {
	std::function<void(entt::entity e, float dt)> Function;
};

struct TempTransform {
	glm::vec3 Position;
	glm::vec3 EulerRotation;
	glm::vec3 Scale;
	
	glm::mat4 GetWorldTransform() const {
		return
			glm::translate(glm::mat4(1.0f), Position) *
			glm::mat4_cast(glm::quat(glm::radians(EulerRotation))) *
			glm::scale(glm::mat4(1.0f), Scale)
			;
	}
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

void GlfwWindowResizedCallback(GLFWwindow* window, int width, int height) {
	if (width > 0 && height > 0) {
		glViewport(0, 0, width, height);
		Game* game = (Game*)glfwGetWindowUserPointer(window);
		if (game) {
			game->Resize(width, height);
		}
	}
}

void Game::Resize(int newWidth, int newHeight) {
	myCamera->Projection = glm::perspective(glm::radians(60.0f), newWidth / (float)newHeight, 0.01f, 1000.0f);
}

Game::Game() :
	myWindow(nullptr),
	myWindowTitle("Game"),
	myClearColor(glm::vec4(0, 0, 0, 1)),
	myModelTransform(glm::mat4(1))
{ }

Game::~Game() { }

void Game::Run()
{
	Initialize();
	InitImGui();

	LoadContent();

	static float prevFrame = static_cast<float>(glfwGetTime());
	
	// Run as long as the window is open
	while (!glfwWindowShouldClose(myWindow)) {
		// Poll for events from windows (clicks, keypressed, closing, all that)
		glfwPollEvents();

		float thisFrame = static_cast<float>(glfwGetTime());
		float deltaTime = thisFrame - prevFrame;

		Update(deltaTime);
		Draw(deltaTime);

		ImGuiNewFrame();
		DrawGui(deltaTime);
		ImGuiEndFrame();

		// Store this frames time for the next go around
		prevFrame = thisFrame;

		// Present our image to windows
		glfwSwapBuffers(myWindow);
	}

	LOG_INFO("Shutting down...");

	UnloadContent();

	ShutdownImGui();
	Shutdown();
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
	myWindow = glfwCreateWindow(600, 600, myWindowTitle, nullptr, nullptr);

	// Tie our game to our window, so we can access it via callbacks
	glfwSetWindowUserPointer(myWindow, this);

	// Set our window resized callback
	glfwSetWindowSizeCallback(myWindow, GlfwWindowResizedCallback);

	// We want GL commands to be executed for our window, so we make our window's context the current one
	glfwMakeContextCurrent(myWindow);

	// Let glad know what function loader we are using (will call gl commands via glfw)
	if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
		std::cout << "Failed to initialize Glad" << std::endl;
		throw std::runtime_error("Failed to initialize GLAD");
	}

	// Log our renderer and OpenGL version
	LOG_INFO(glGetString(GL_RENDERER));
	LOG_INFO(glGetString(GL_VERSION));

	// Enable debugging, and route messages to our callback
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(GlDebugMessage, this); 

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void Game::Shutdown() {
	glfwTerminate();
}

glm::vec4 testColor = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);

std::vector<Texture2D::Sptr>       Textures;
std::vector<TextureSampler::Sptr>  Samplers;
std::vector<Material::Sptr>        Materials;

Mesh::Sptr MakeInvertedCube() {
	// Create our 4 vertices
	Vertex verts[8] = {
		//       Position        
		//    x      y     z     
		{{ -1.0f, -1.0f, -1.0f }},
		{{  1.0f, -1.0f, -1.0f }},
		{{ -1.0f,  1.0f, -1.0f }},
		{{  1.0f,  1.0f, -1.0f }},
		{{ -1.0f, -1.0f,  1.0f }},
		{{  1.0f, -1.0f,  1.0f }},
		{{ -1.0f,  1.0f,  1.0f }},
		{{  1.0f,  1.0f,  1.0f }}
	};

	// Create our 6 indices
	uint32_t indices[36] = {
		0, 1, 2,    2, 1, 3,  // bottom		
		4, 6, 5,    6, 7, 5,  // top
		
		0, 1, 4,    4, 1, 5,  // front		
		2, 3, 6,    6, 3, 7,  // back
		
		2, 4, 0,    2, 6, 4,  // left
		3, 5, 1,    3, 7, 5   // right
	};

	// Create a new mesh from the data
	return std::make_shared<Mesh>(verts, 8, indices, 36);
}

Mesh::Sptr MakeSubdividedPlane(float size, uint32_t numSections) {
	LOG_ASSERT(numSections > 0, "Number of sections must be greater than 0!");
	LOG_ASSERT(size != 0, "Size cannot be zero!");

	// Determine the number of edge vertices, and the number of vertices and indices we'll need
	uint32_t numEdgeVerts = numSections + 1;
	uint32_t vertexCount = numEdgeVerts * numEdgeVerts;
	uint32_t indexCount = numSections * numSections * 6u;

	// Allocate some memory for our vertices and indices
	Vertex* vertices = new Vertex[vertexCount];
	uint32_t* indices = new uint32_t[indexCount];

	// Determine where to start vertices from, and the step pre grid square
	float start = -size / 2.0f;
	float step = size / numSections;

	// Iterate over the grid's edge vertices
	for (uint32_t ix = 0; ix < numEdgeVerts; ix++) {
		for(uint32_t iy = 0; iy < numEdgeVerts; iy++) {
			// Get a reference to the vertex so we can modify it
			Vertex& vert = vertices[ix * numEdgeVerts + iy]; 
			// Set its position
			vert.Position.x = start + ix * step;
			vert.Position.y = start + iy * step;
			vert.Position.z = 0.0f;
			// Set its normal
			vert.Normal = glm::vec3(0, 0, 1);
			// The UV will go from [0, 1] across the entire plane (can change this later)
			vert.UV.x = vert.Position.x / size;
			vert.UV.y = vert.Position.y / size;
			// Flat white color
			vert.Color = glm::vec4(1.0f);
		}
	}

	// We'll just increment an index instead of calculating it
	uint32_t index = 0;
	// Iterate over the quads that make up the grid
	for (uint32_t ix = 0; ix < numSections; ix++) {
		for (uint32_t iy = 0; iy < numSections; iy++) {
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
	// Create the result, then clean up the arrays we used
	Mesh::Sptr result = std::make_shared<Mesh>(vertices, vertexCount, indices, indexCount);
	std::stringstream stream;
	stream << "PLANE-" << size << "-" << numSections;
	result->SetDebugName(stream.str());
	delete[] vertices;
	delete[] indices;
	// Return the result
	return result;
}

void sortRenderers(entt::registry& ecs) {
	// We sort our mesh renderers based on material properties
	// This will group all of our meshes based on shader first, then material second
	ecs.sort<MeshRenderer>([](const MeshRenderer& lhs, const MeshRenderer& rhs) {
		if (rhs.Material == nullptr || rhs.Mesh == nullptr)
			return false;
		else if (lhs.Material == nullptr || lhs.Mesh == nullptr)
			return true;
		else if (lhs.Material->IsBlendingEnabled & !rhs.Material->IsBlendingEnabled)
			return false;                                                           
		else if (!lhs.Material->IsBlendingEnabled & rhs.Material->IsBlendingEnabled)
			return true;
		else if (lhs.Material->GetShader() != rhs.Material->GetShader())
			return lhs.Material->GetShader() < rhs.Material->GetShader();
		else
			return lhs.Material < rhs.Material;
		});
}

void ctorSort(entt::entity, entt::registry& ecs, const MeshRenderer& r) {
	sortRenderers(ecs);
}
void dtorSort(entt::entity, entt::registry& ecs) {
	sortRenderers(ecs);
}
void Game::LoadContent() {
	myCamera = std::make_shared<Camera>();
	myCamera->SetPosition(glm::vec3(5, 5, 5));
	myCamera->LookAt(glm::vec3(0), glm::vec3(0, 0, 1));
	myCamera->Projection = glm::perspective(glm::radians(60.0f), 1.0f, 0.01f, 1000.0f);

		
	// Create our 4 vertices
	Vertex vertices[4] = {
		//       Position                   Color                      Normal
		//    x      y     z         r    g     b     a            x      y     z      u     v
		{{ -10.0f, -10.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }},
		{{  10.0f, -10.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f }},
		{{ -10.0f,  10.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f }},
		{{  10.0f,  10.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }}
	};
	
	// Create our 6 indices
	uint32_t indices[6] = {     
		0, 1, 2,  
		2, 1, 3 
	};
	
	Mesh::Sptr myMesh = std::make_shared<Mesh>(vertices, 4, indices, 6);		

	Mesh::Sptr monkey = ObjLoader::LoadObjToMesh("sphere.obj");

	// New in tutorial 09
	SamplerDesc desciption = SamplerDesc();
	desciption.MinFilter = MinFilter::NearestMipNearest;
	desciption.MagFilter = MagFilter::Nearest;
	TextureSampler::Sptr NearestMipped = std::make_shared<TextureSampler>(desciption);
	NearestMipped->SetDebugName("Nearest");

	desciption = SamplerDesc();
	desciption.MinFilter = MinFilter::LinearMipLinear;
	desciption.MagFilter = MagFilter::Linear;
	TextureSampler::Sptr Trilinear = std::make_shared<TextureSampler>(desciption);
	Trilinear->SetDebugName("Trilinear");

	Samplers.push_back(nullptr);
	Samplers.push_back(NearestMipped);
	Samplers.push_back(Trilinear);
			
	SceneManager::RegisterScene("Test");
	SceneManager::RegisterScene("Test2");
	SceneManager::SetCurrentScene("Test");

	// New in tutorial 09
	auto scene = CurrentScene();
	scene->Registry().on_construct<MeshRenderer>().connect<&::ctorSort>();
	scene->Registry().on_destroy<MeshRenderer>().connect<&::dtorSort>();

	scene->SkyboxShader = std::make_shared<Shader>();
	scene->SkyboxShader->Load("shaders/cubemap.vs.glsl", "shaders/cubemap.fs.glsl");
	scene->SkyboxMesh = MakeInvertedCube();
	
	std::string files[6] = {
		std::string("cubemap/scene_rt.jpg"), 
		std::string("cubemap/scene_lf.jpg"), 
		std::string("cubemap/scene_up.jpg"), 
		std::string("cubemap/scene_dn.jpg"),    
		std::string("cubemap/scene_ft.jpg"), 
		std::string("cubemap/scene_bk.jpg"), 
	};
	scene->Skybox = TextureCube::LoadFromFiles(files, false);


	Texture2D::Sptr albedo = Texture2D::LoadFromFile("color-grid.png");
	Textures.push_back(albedo); 
	Texture2D::Sptr metallic = Texture2D::LoadFromFile("metallic.png");
	Textures.push_back(albedo);

	Texture2DDescription tinyDesc = Texture2DDescription();
	tinyDesc.Width = tinyDesc.Height = 1;
	tinyDesc.Format = InternalFormat::RGB8;
	tinyDesc.EnableMip = false;
	
	Texture2D::Sptr white = std::make_shared<Texture2D>(tinyDesc);
	uint8_t dataW[3] = { 0xFFu, 0xFFu, 0xFFu };
	white->LoadData(&dataW, 1, 1, PixelFormat::Rgb, PixelType::UByte);
	white->SetDebugName("<white>");
	Textures.push_back(white);

	Texture2D::Sptr gray = std::make_shared<Texture2D>(tinyDesc);
	uint8_t dataG[3] = { 0x80u, 0x80u, 0x80u };
	gray->LoadData(&dataG, 1, 1, PixelFormat::Rgb, PixelType::UByte);
	gray->SetDebugName("<gray>");
	Textures.push_back(gray);

	Texture2D::Sptr lBlue = std::make_shared<Texture2D>(tinyDesc);
	uint8_t dataLB[3] = { 0x80u, 0x80u, 0xFFu };
	lBlue->LoadData(&dataLB, 1, 1, PixelFormat::Rgb, PixelType::UByte);
	lBlue->SetDebugName("<light blue>");
	Textures.push_back(lBlue);
	 
	Shader::Sptr phong = std::make_shared<Shader>();
	phong->Load("shaders/lighting.vs.glsl", "shaders/blinn-phong-environment.fs.glsl"); 

	Material::Sptr testMat = std::make_shared<Material>(phong);
	testMat->Set("a_LightPos", { 2, 0, 4 });
	testMat->Set("a_LightColor", { 1.0f, 1.0f, 1.0f });
	testMat->Set("a_AmbientColor", { 1.0f, 1.0f, 1.0f });
	testMat->Set("a_AmbientPower", 0.1f);
	testMat->Set("a_LightSpecPower", 0.5f);  
	testMat->Set("a_LightShininess", 256.0f);
	testMat->Set("a_LightAttenuation", 1.0f / 100.0f);
	testMat->Set("s_Albedo", albedo, Trilinear);
	testMat->Set("s_Metallic", metallic, Trilinear);
	testMat->Set("s_Environment", scene->Skybox, Trilinear);

	Material::Sptr testMat2 = testMat->Clone();
	testMat2->Set("s_Metallic", metallic);
	
	Materials.push_back(testMat);
	Materials.push_back(testMat2);  

	{
		auto& ecs = GetRegistry("Test");  

		if (false)
		{
			entt::entity e1 = ecs.create();  
			MeshRenderer& m1 = ecs.assign<MeshRenderer>(e1);    
			ecs.assign<TempTransform>(e1).Scale = glm::vec3(1.0f);   
			m1.Material = testMat;
			m1.Mesh = myMesh; 
		}
		//if (false)
		{
			entt::entity e1 = ecs.create(); 
			MeshRenderer& m1 = ecs.assign<MeshRenderer>(e1);
			auto& transform = ecs.assign<TempTransform>(e1);
			transform.Scale = glm::vec3(1.0f);
			transform.Position = glm::vec3(0, 0, 2.0f);
			m1.Material = testMat2;
			m1.Mesh = monkey;

			auto rotate = [=](entt::entity e, float dt) {
				float speed = glfwGetKey(myWindow, GLFW_KEY_R) == GLFW_PRESS ? 1.0f : 0.0f;
				CurrentRegistry().get<TempTransform>(e).EulerRotation += glm::vec3(0, 0, 90 * dt * speed);
			};
			auto& up = ecs.get_or_assign<UpdateBehaviour>(e1);
			up.Function = rotate;
		}
		{

			Shader::Sptr waterShader = std::make_shared<Shader>();
			waterShader->Load("shaders/water-shader.vs.glsl", "shaders/water-shader.fs.glsl");
			waterShader->SetDebugName("Water Shader");

			Material::Sptr testMat = std::make_shared<Material>(waterShader);
			testMat->IsBlendingEnabled = true;

			testMat->Set("a_EnabledWaves", 4);
			testMat->Set("a_Gravity", 9.81f);

			testMat->Set("a_Waves[0]", { 1.0f, 0.0f, 0.50f, 10.0f});
			testMat->Set("a_Waves[1]", { 0.0f, 1.0f, 0.25f, 3.1f }); 
			testMat->Set("a_Waves[2]", { 0.5f, 1.0f, 0.20f, 35.0f });
			testMat->Set("a_Waves[3]", { 0.5f, 0.7f, 0.20f, 2.0f });
			for (int ix = 4; ix < 8; ix++) {
				char buffer[11];
				sprintf_s(buffer, 11, "a_Waves[%d]", ix);
				testMat->Set(buffer, 
					glm::vec4(
						glm::linearRand(0.001f, 2.0f),
						glm::linearRand(0.001f, 2.0f),
						glm::linearRand(0.0f, 1.0f), 
						glm::linearRand(0.0f, 5.f)));
			}
						 
			testMat->Set("a_WaterColor", { 0.7f, 1.0f, 0.9f }); 
			testMat->Set("a_WaterClarity", 0.9f); 
			testMat->Set("a_FresnelPower", 0.5f); 
			testMat->Set("a_RefractionIndex", 1.0f / 1.34f);	 		
			testMat->Set("s_Environment", scene->Skybox, Trilinear);

			Materials.push_back(testMat);
			
			entt::entity e1 = ecs.create(); 
			MeshRenderer& m1 = ecs.assign<MeshRenderer>(e1); 
			auto& transform = ecs.assign<TempTransform>(e1);
			transform.Scale = glm::vec3(1.0f);
			transform.Position.z = 1.0f;
			m1.Material = testMat;
			m1.Mesh = MakeSubdividedPlane(20.0f, 100);

			auto rotate = [=](entt::entity e, float dt) {
				float speed = glfwGetKey(myWindow, GLFW_KEY_R) == GLFW_PRESS ? 1.0f : 0.0f;
				CurrentRegistry().get<TempTransform>(e).EulerRotation += glm::vec3(0, 0, 90 * dt * speed);
			};
			auto& up = ecs.get_or_assign<UpdateBehaviour>(e1);
			up.Function = rotate;
		}
	}

}

void Game::UnloadContent() {
	SceneManager::DestroyScenes();
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
	io.DisplaySize = ImVec2((float)width, (float)height);

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

void Game::Update(float deltaTime) {
	glm::vec3 movement = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);

	float speed = 1.0f;
	float rotSpeed = 1.0f;

	if (glfwGetKey(myWindow, GLFW_KEY_W) == GLFW_PRESS)
		movement.z -= speed * deltaTime;
	if (glfwGetKey(myWindow, GLFW_KEY_S) == GLFW_PRESS)
		movement.z += speed * deltaTime;
	if (glfwGetKey(myWindow, GLFW_KEY_A) == GLFW_PRESS)
		movement.x -= speed * deltaTime;
	if (glfwGetKey(myWindow, GLFW_KEY_D) == GLFW_PRESS)
		movement.x += speed * deltaTime;
	if (glfwGetKey(myWindow, GLFW_KEY_SPACE) == GLFW_PRESS)
		movement.y += speed * deltaTime;
	if (glfwGetKey(myWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		movement.y -= speed * deltaTime;

	if (glfwGetKey(myWindow, GLFW_KEY_Q) == GLFW_PRESS)
		rotation.z -= rotSpeed * deltaTime;
	if (glfwGetKey(myWindow, GLFW_KEY_E) == GLFW_PRESS)
		rotation.z += rotSpeed * deltaTime;
	if (glfwGetKey(myWindow, GLFW_KEY_UP) == GLFW_PRESS)
		rotation.x -= rotSpeed * deltaTime;
	if (glfwGetKey(myWindow, GLFW_KEY_DOWN) == GLFW_PRESS)
		rotation.x += rotSpeed * deltaTime;
	if (glfwGetKey(myWindow, GLFW_KEY_LEFT) == GLFW_PRESS)
		rotation.y -= rotSpeed * deltaTime;
	if (glfwGetKey(myWindow, GLFW_KEY_RIGHT) == GLFW_PRESS)
		rotation.y += rotSpeed * deltaTime;
	   	
	// Rotate and move our camera based on input
	myCamera->Rotate(rotation);
	myCamera->Move(movement);
	
	// Rotate our transformation matrix a little bit each frame
	myModelTransform = glm::rotate(myModelTransform, deltaTime, glm::vec3(0, 0, 1));

	auto view = CurrentRegistry().view<UpdateBehaviour>();
	for (const auto& e : view) {
		auto& func = CurrentRegistry().get<UpdateBehaviour>(e);
		if (func.Function) {
			func.Function(e, deltaTime);
		}
	}
}

void Game::Draw(float deltaTime) {
	// Clear our screen every frame
	glClearColor(myClearColor.x, myClearColor.y, myClearColor.z, myClearColor.w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// We'll grab a reference to the ecs to make things easier
	auto& ecs = CurrentRegistry();

	// These will keep track of the current shader and material that we have bound
	Material::Sptr mat = nullptr;
	Shader::Sptr boundShader = nullptr;
	   
	auto scene = CurrentScene();
	// Draw the skybox after everything else, if the scene has one
	if (scene->Skybox)
	{
		// Disable culling
		glDisable(GL_CULL_FACE);
		// Set our depth test to less or equal (because we are at depth of 1.0f)
		glDepthFunc(GL_LEQUAL);
		// Disable depth writing
		glDepthMask(GL_FALSE);

		// Make sure no samplers are bound to slot 0
		TextureSampler::Unbind(0);
		// Set up the shader
		scene->SkyboxShader->Bind();
		scene->SkyboxShader->SetUniform("a_View", glm::mat4(glm::mat3(
			myCamera->GetView()
		)));
		scene->SkyboxShader->SetUniform("a_Projection", myCamera->Projection);
		scene->Skybox->Bind(0);
		scene->SkyboxShader->SetUniform("s_Skybox", 0);
		scene->SkyboxMesh->Draw();

		// Restore our state
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);
		glDepthFunc(GL_LESS);
	}
	
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
			boundShader->SetUniform("a_CameraPos", myCamera->GetPosition());
			boundShader->SetUniform("a_Time", (float)glfwGetTime());
		}
		
		// If our material has changed, we need to apply it to the shader
		if (renderer.Material != mat) {
			mat = renderer.Material;
			if (mat->PreFrame)
				mat->PreFrame(mat);
			mat->Apply();
		}
		
		// We'll need some info about the entities position in the world
		const TempTransform& transform = ecs.get_or_assign<TempTransform>(entity);
		
		// Our normal matrix is the inverse-transpose of our object's world rotation
		glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(transform.GetWorldTransform())));

		// Update the MVP using the item's transform
		mat->GetShader()->SetUniform(
			"a_ModelViewProjection",
			myCamera->GetViewProjection() *
			transform.GetWorldTransform());  

		// Update the model matrix to the item's world transform
		mat->GetShader()->SetUniform("a_Model", transform.GetWorldTransform());

		// Update the model matrix to the item's world transform
		mat->GetShader()->SetUniform("a_NormalMatrix", normalMatrix);

		// Draw the item
		renderer.Mesh->Draw();   
	}
}

void Game::DrawGui(float deltaTime) {
	// Open a new ImGui window
	{
		static bool testOpen = true;
		ImGui::Begin("Test", &testOpen, ImVec2(300, 200));
		// Draw a color editor
		ImGui::ColorEdit4("Clear Color", &myClearColor[0]);
		// Check if a textbox has changed, and update our window title if it has
		if (ImGui::InputText("Window Title", myWindowTitle, 32)) {
			glfwSetWindowTitle(myWindow, myWindowTitle);
		}
		// Our object's test color
		ImGui::ColorEdit4("Object Color", &testColor[0]);

		auto it = SceneManager::Each();
		for (auto& kvp : it) {
			if (ImGui::Button(kvp.first.c_str())) {
				SceneManager::SetCurrentScene(kvp.first);
			}
		}

		int ix = 0;
		char label[32];
		if (ImGui::CollapsingHeader("Materials")) {
			for (auto& mat : Materials) {
				sprintf_s(label, 31, "[%d]", ix);
				if (ImGui::CollapsingHeader(label))
					mat->DrawEditor(Samplers, Textures);
				ix++;
			}
		}

		ImGui::End();
	}

	{
		// Open a second ImGui window
		ImGui::Begin("Debug");
		// Draw a formatted text line
		ImGui::Text("Time: %f", glfwGetTime());

		{
			static float data[100];
			static int ix = 0;
			data[ix++] = (float)MemoryTracking::NumAllocs;
			ix %= 100;
			ImGui::PlotLines("Num Allocations", data, 100);
			ImGui::Text("%d", MemoryTracking::NumAllocs);
		}
		{
			static float data[100];
			static int ix = 0;
			data[ix++] = (float)MemoryTracking::TotalAllocs;
			ix %= 100;
			ImGui::PlotLines("Total Allocations", data, 100);
			ImGui::Text("%d", MemoryTracking::TotalAllocs);
		}
		{
			static float data[100];
			static int ix = 0;
			data[ix++] = (float)MemoryTracking::NumBytes;
			ix %= 100;
			ImGui::PlotLines("Num Bytes", data, 100);
			ImGui::Text("%d", MemoryTracking::NumBytes);
		}
		{
			static float data[100];
			static int ix = 0;
			data[ix++] = (float)MemoryTracking::TotalBytes;
			ix %= 100;
			ImGui::PlotLines("Total Bytes", data, 100);
			ImGui::Text("%d", MemoryTracking::TotalBytes);
		}

		// Start a new ImGui header for our camera settings
		if (ImGui::CollapsingHeader("Camera Settings")) {
			// Draw our camera's normal
			glm::vec3 camNormal = myCamera->GetForward();
			ImGui::DragFloat3("Normal", &camNormal[0]);

			// Get the camera's position so we can edit it
			glm::vec3 position = myCamera->GetPosition();
			// Draw an editor control for the position, and update camera position
			if (ImGui::DragFloat3("Position", &position[0])) {
				myCamera->SetPosition(position);
			}
			if (ImGui::Button("Look at center")) {
				myCamera->LookAt(glm::vec3(0), glm::vec3(0, 0, 1));
			}
			// Get the camera pinning value
			static glm::vec3 camPin;

			// Get whether or not camera pinning is enabled
			bool camPlaneEnabled = myCamera->GetPinnedUp().has_value();
			// Draw a checkbox for camera pinning
			if (ImGui::Checkbox("Pinning Enabled", &camPlaneEnabled)) {
				// If we've disabled pinning, cache our pinning vector and remove it
				if (!camPlaneEnabled) {
					camPin = myCamera->GetPinnedUp().value();
					myCamera->SetPinnedUp(std::optional<glm::vec3>());
				}
				// Set our camera's pinning vector to our cached value
				else {
					myCamera->SetPinnedUp(camPin);
				}
			}
			// If we have enabled pinning
			if (camPlaneEnabled) {
				// Draw a slider for our camera pin direction
				if (ImGui::InputFloat3("Pin Direction", &camPin[0])) {
					myCamera->SetPinnedUp(camPin);
				}
			}
		}
		ImGui::End();
	}
}
