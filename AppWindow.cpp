#include "AppWindow.h"
#include "structs.h"
#include <Windows.h>
#include <iostream>
#include "EngineTime.h"
#include "Vector3D.h"
#include "Matrix4x4.h"
#include "InputSystem.h"
#include "SceneCameraHolder.h"
#include "ParticleSystem.h"

// ImGui includes for UI rendering
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#include <cstdlib>
#include <ctime>


// ============================================================================
// STRUCTURES AND CONSTANTS
// ============================================================================

__declspec(align(16))
struct constant
{
	Matrix4x4 m_world;
	Matrix4x4 m_view;
	Matrix4x4 m_proj;
	float m_angle;
};

// ============================================================================
// SINGLETON PATTERN IMPLEMENTATION
// ============================================================================

AppWindow* AppWindow::sharedInstance = nullptr;

AppWindow* AppWindow::getInstance()
{
	if (!sharedInstance)
	{
		sharedInstance = new AppWindow();
		sharedInstance->init();
	}
	return sharedInstance;
}

void AppWindow::initialize()
{
	sharedInstance = new AppWindow();
	sharedInstance->init();

}

void AppWindow::destroy()
{
	if (sharedInstance != NULL)
		sharedInstance->release();
}

// ============================================================================
// CONSTRUCTOR AND DESTRUCTOR
// ============================================================================

AppWindow::AppWindow()
{

}


// ============================================================================
// UPDATE AND ANIMATION LOGIC
// ============================================================================

void AppWindow::update()
{
	// Animation parameters
	float GameSpeed = 1.57f; // Game speed multiplier (1.57f is the default.)
	m_angle += GameSpeed * EngineTime::getDeltaTime();

	// Setup constant buffer data
	constant cc;
	cc.m_angle = m_angle;

	// Update animation deltas
	m_delta_pos += EngineTime::getDeltaTime() / 10.0f;
	if (m_delta_pos > 1.0f) m_delta_pos = 0.0f;


	Matrix4x4 temp;

	m_delta_scale += EngineTime::getDeltaTime() / 0.55f;



	cc.m_world.setIdentity();


	// Get view matrix from camera system
	auto world_cam = SceneCameraHolder::getInstance()->getCamera()->getViewMatrix();
	world_cam.inverse();
	cc.m_view = world_cam;



	// Setup projection matrix
	RECT rc = this->getClientWindowRect();
	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;


	// Perspective projection setup
	cc.m_proj.setPerspectiveFovLH(1.57, (float)width / (float)height, 0.1f, 100.0f);



	this->m_cb->update(GraphicsEngine::get()->getDeviceContext(), &cc);



}

// ============================================================================
// GRAPHICS INITIALIZATION
// ============================================================================

void AppWindow::createGraphicsWindow()
{

	// ---- SYSTEM INITIALIZATION ----
	SceneCameraHolder::initialize();
	InputSystem::get()->showCursor(true);
	GraphicsEngine::initialize();

	// ---- SWAP CHAIN SETUP ----
	this->m_swap_chain = GraphicsEngine::get()->createSwapChain();
	RECT rc = this->getClientWindowRect();
	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;
	std::cout << "Window rect width: " << width << std::endl;
	std::cout << "Window rect height: " << height << std::endl;


	// Configure camera with window dimensions
	SceneCameraHolder::getInstance()->getCamera()->height = height;
	SceneCameraHolder::getInstance()->getCamera()->width = width;

	// Initialize swap chain
	this->m_swap_chain->init(this->m_hwnd, width, height);

	// Set initial camera position
	m_world_cam.setTranslation(Vector3D(0.0f, 0.0f, -2.0f), false);

	// ---- SHADER COMPILATION ----
	void* shader_byte_code = nullptr;
	size_t size_shader = 0;

	// Compile and create vertex shader
	GraphicsEngine::get()->compileVertexShader(L"VertexShader.hlsl", "vsmain", &shader_byte_code, &size_shader);
	this->m_vs = GraphicsEngine::get()->createVertexShader(shader_byte_code, size_shader);


	srand(time(0));

	//For #4, Spawning 100 different cubes with much wider distribution and varied sizes
	for (int i = 0; i < 100; i++)
	{
		Cube cube("Test", shader_byte_code, size_shader);

		// Create more reasonable varied cube sizes
		float sizeCategory = rand() % 4;
		float scale;
		switch ((int)sizeCategory)
		{
		case 0: // Small cubes
			scale = 0.2f + (rand() % 20) * 0.01f; // 0.2 to 0.4
			break;
		case 1: // Medium-small cubes  
			scale = 0.4f + (rand() % 30) * 0.01f; // 0.4 to 0.7
			break;
		case 2: // Medium cubes
			scale = 0.7f + (rand() % 30) * 0.01f; // 0.7 to 1.0
			break;
		case 3: // Large cubes
			scale = 1.0f + (rand() % 30) * 0.01f; // 1.0 to 1.3
			break;
		}

		// Sometimes create non-uniform scaling for rectangular shapes
		if (rand() % 5 == 0) // 20% chance (reduced from 25%)
		{
			float scaleX = scale * (0.6f + (rand() % 80) * 0.01f); // 0.6x to 1.4x the base scale
			float scaleY = scale * (0.6f + (rand() % 80) * 0.01f);
			float scaleZ = scale * (0.6f + (rand() % 80) * 0.01f);
			cube.setScale(Vector3D(scaleX, scaleY, scaleZ));
		}
		else
		{
			cube.setScale(Vector3D(scale)); // Uniform scaling
		}

		float x = (rand() % (int)(100 * 15.0f) - (int)(50 * 15.0f)) * 0.01f; // -15 to 15
		float y = (rand() % (int)(100 * 10.0f) - (int)(50 * 10.0f)) * 0.01f; // -10 to 10
		float z = (rand() % (int)(100 * 30.0f) - (int)(5 * 100)) * 0.01f;    // -5 to 25

		cube.setPosition(Vector3D(x, y, z));
		this->cubes.push_back(cube);
	}

	// Additional layer of cubes at different depths with moderate size variations
	for (int i = 0; i < 30; i++)
	{
		Cube cube("Far", shader_byte_code, size_shader);

		// Create reasonably sized cubes for the far distance
		float scale = 0.6f + (rand() % 80) * 0.01f; 

		// 20% chance for larger cubes in the distance (reduced from 30%)
		if (rand() % 10 < 2)
		{
			scale = 1.4f + (rand() % 60) * 0.01f; // 1.4 to 2.0 - large but not massive
		}

		// Sometimes create rectangular shapes (reduced frequency and extremes)
		if (rand() % 4 == 0) // 25% chance
		{
			float scaleX = scale * (0.5f + (rand() % 100) * 0.01f);
			float scaleY = scale * (0.8f + (rand() % 80) * 0.01f);  
			float scaleZ = scale * (0.5f + (rand() % 100) * 0.01f); 
			cube.setScale(Vector3D(scaleX, scaleY, scaleZ));
		}
		else
		{
			cube.setScale(Vector3D(scale));
		}

		// Far layer: Z from 20 to 40
		float x = (rand() % (int)(100 * 20.0f) - (int)(50 * 20.0f)) * 0.01f; // -20 to 20
		float y = (rand() % (int)(100 * 15.0f) - (int)(50 * 15.0f)) * 0.01f; // -15 to 15
		float z = 20.0f + (rand() % (int)(100 * 20.0f)) * 0.01f;              // 20 to 40

		cube.setPosition(Vector3D(x, y, z));
		this->cubes.push_back(cube);
	}


	// Main ground plane - much larger for better depth testing
	Plane plane("Ground", shader_byte_code, size_shader);
	plane.setScale(Vector3D(50.0f, 1.0f, 50.0f)); // Much larger plane: 50x50 units
	plane.setPosition(Vector3D(0.0f, -2.0f, 0.0f)); // Lowered slightly
	plane.setRotation(Vector3D(0.f, 0.0f, 0.0f));
	this->planes.push_back(plane);

	// Additional elevated platform for more depth complexity
	Plane plane2("Platform", shader_byte_code, size_shader);
	plane2.setScale(Vector3D(15.0f, 1.0f, 15.0f));
	plane2.setPosition(Vector3D(10.0f, 5.0f, 15.0f));
	plane2.setRotation(Vector3D(0.f, 0.0f, 0.0f));
	this->planes.push_back(plane2);

	// Vertical wall plane for depth testing
	Plane wall("Wall", shader_byte_code, size_shader);
	wall.setScale(Vector3D(30.0f, 20.0f, 1.0f));
	wall.setPosition(Vector3D(0.0f, 5.0f, 30.0f));
	wall.setRotation(Vector3D(1.57f, 0.0f, 0.0f)); // Rotate 90 degrees to make it vertical
	this->planes.push_back(wall);

	// ---- PARTICLE SYSTEM SETUP ----
	int preset = 0; // 0 for ashes
	ParticleSystem::initialize();
	Particle templateParticle = Particle();
	if (preset == 0)
	{
		templateParticle.setColor(Vector3D(0.8f));

		ParticleSystem::getInstance()->particleMovementRandomDown = true;
		ParticleSystem::getInstance()->interval = 0.005f;
		ParticleSystem::getInstance()->max_size = 100;
		ParticleSystem::getInstance()->spawnAreaCenter = { 0.f,0.88f,0.0f };
		ParticleSystem::getInstance()->spawnAreaWidth = 2.20f;
		ParticleSystem::getInstance()->spawnAreaHeight = 0.10f;
	}
	else if (preset == 1)
	{
		templateParticle.setColor(Vector3D(0.15f));
		templateParticle.setColor(Vector3D(0.5, 0.1f, 0.1f), 7);
		templateParticle.setColor1(Vector3D(0.7, 0, 0.f));
		templateParticle.setColor1(Vector3D(0.35f), 0);
		templateParticle.lifeTimeLimit = 10.0f;

		ParticleSystem::getInstance()->interval = 0.01f;
		ParticleSystem::getInstance()->max_size = 500;
		ParticleSystem::getInstance()->spawnAreaCenter = { 0.f,-0.13f,0.0f };
		ParticleSystem::getInstance()->particleMovementRandomUp = true;
	}
	else if (preset == 2)
	{
		ParticleSystem::getInstance()->interval = 0.001f;
		ParticleSystem::getInstance()->max_size = 2000;
		ParticleSystem::getInstance()->spawnAreaWidth = 1.80f;
		ParticleSystem::getInstance()->spawnAreaHeight = 1.80f;
		ParticleSystem::getInstance()->particleMovementRandom = true;
	}
	else
	{
		ParticleSystem::getInstance()->max_size = 100;
		ParticleSystem::getInstance()->particleMovementRandom = true;
	}

	ParticleSystem::getInstance()->Create(templateParticle, &shader_byte_code, &size_shader);


	GraphicsEngine::get()->releaseCompiledShader();

	GraphicsEngine::get()->compilePixelShader(L"PixelShader.hlsl", "psmain", &shader_byte_code, &size_shader);
	this->m_ps = GraphicsEngine::get()->createPixelShader(shader_byte_code, size_shader);
	GraphicsEngine::get()->releaseCompiledShader();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(this->m_hwnd);
	ImGui_ImplDX11_Init(GraphicsEngine::getInstance()->getDevice(), GraphicsEngine::getInstance()->getDeviceContext()->getDeviceContext());

}

// ============================================================================
// WINDOW EVENT HANDLERS
// ============================================================================

void AppWindow::onCreate()
{
	Window::onCreate();
}

void AppWindow::onUpdate()
{
	// ---- FOG CONTROL VARIABLES ----
	static float fog_start = 5.f;	// Increased for larger scene
	static float fog_end = 30.0f;	// Increased for larger scene
	static float fog_density = 0.05f; // Reduced for better visibility

	// ---- FOG CONTROLS ---- 
	if (InputSystem::get()->isKeyDown('Z'))
	{
		fog_start = max(0.0f, fog_start - 0.2f);
	}
	if (InputSystem::get()->isKeyDown('C'))
	{
		fog_start += 0.2f;
		fog_start = min(fog_start, fog_end - 0.1f);
	}

	if (InputSystem::get()->isKeyDown('V'))
	{
		fog_end = max(fog_start + 0.1f, fog_end - 0.2f);
	}
	if (InputSystem::get()->isKeyDown('B'))
	{
		fog_end += 0.2f;
	}

	if (InputSystem::get()->isKeyDown('N'))
	{
		fog_density = max(0.001f, fog_density - 0.002f);
	}
	if (InputSystem::get()->isKeyDown('M'))
	{
		fog_density += 0.002f;
	}

	// ---- IMGUI FRAME SETUP ----
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// === ENHANCED IMGUI INTERFACE ===

	// Main Control Panel
	ImGui::Begin("Engine Control Panel", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	// Performance Metrics Section
	if (ImGui::CollapsingHeader("Performance Metrics", ImGuiTreeNodeFlags_DefaultOpen))
	{
		float framerate = ImGui::GetIO().Framerate;
		float frametime = 1000.0f / framerate;

		ImGui::Text("FPS: %.1f", framerate);
		ImGui::SameLine();
		if (framerate < 30.0f)
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "(Low)");
		else if (framerate < 60.0f)
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "(Medium)");
		else
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "(Good)");

		ImGui::Text("Frame Time: %.3f ms", frametime);

		// FPS Graph
		static float fps_history[120] = {};
		static int fps_history_offset = 0;
		fps_history[fps_history_offset] = framerate;
		fps_history_offset = (fps_history_offset + 1) % 120;

		ImGui::PlotLines("FPS", fps_history, 120, fps_history_offset, nullptr, 0.0f, 120.0f, ImVec2(0, 80));
	}

	// Scene Information Section
	if (ImGui::CollapsingHeader("Scene Information", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Total Objects: %d", (int)(cubes.size() + planes.size()));
		ImGui::Indent();
		ImGui::Text("Cubes: %d", (int)cubes.size());
		ImGui::Text("Planes: %d", (int)planes.size());
		ImGui::Text("Particles: %d", ParticleSystem::getInstance()->getParticleAmount());
		ImGui::Unindent();

		// Camera information
		Vector3D cam_pos = SceneCameraHolder::getInstance()->getCamera()->getLocalPosition();
		ImGui::Text("Camera Position:");
		ImGui::Indent();
		ImGui::Text("X: %.2f, Y: %.2f, Z: %.2f", cam_pos.x, cam_pos.y, cam_pos.z);
		ImGui::Unindent();
	}

	// Fog Controls Section 
	if (ImGui::CollapsingHeader("Fog Settings", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Current Fog Settings:");
		ImGui::Indent();
		ImGui::Text("Start Distance: %.2f", fog_start);
		ImGui::Text("End Distance: %.2f", fog_end);
		ImGui::Text("Density: %.3f", fog_density);
		ImGui::Unindent();

		ImGui::Separator();
		ImGui::Text("Fog Controls:");
		ImGui::BulletText("Z/C - Adjust fog start distance");
		ImGui::BulletText("V/B - Adjust fog end distance");
		ImGui::BulletText("N/M - Adjust fog density");
	}

	ImGui::End();

	// Camera Controls Window
	ImGui::Begin("Camera Controls");

	if (ImGui::CollapsingHeader("Movement Controls", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Keyboard Controls:");
		ImGui::BulletText("WASD - Move camera");
		ImGui::BulletText("Mouse - Look around (when focused)");
		ImGui::BulletText("ESC - Exit application");

	}

	ImGui::End();



	// Mini Statistics Overlay (always visible)
	ImGuiWindowFlags overlay_flags = ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoNav;

	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.35f);

	if (ImGui::Begin("Overlay", nullptr, overlay_flags))
	{
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		ImGui::Text("Objects: %d", (int)(cubes.size() + planes.size()));
		ImGui::Text("Particles: %d", ParticleSystem::getInstance()->getParticleAmount());

		// Color-coded performance indicator
		float fps = ImGui::GetIO().Framerate;
		if (fps < 30.0f)
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Performance: LOW");
		else if (fps < 60.0f)
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Performance: MEDIUM");
		else
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Performance: GOOD");
	}
	ImGui::End();

	// ---- RENDERING SETUP ----
	Window::onUpdate();
	InputSystem::get()->update();
	//GraphicsEngine::get()->getDeviceContext()->clearRenderTargetColor(this->m_swap_chain,
	//	(float)(0.6f), (float)(0.6f), (float)(0.6f), 1);




	//Toggle this to make it possible to see the objects obscured by the fog
	GraphicsEngine::get()->getDeviceContext()->clearRenderTargetColor(this->m_swap_chain, (float)(0.65f), (float)(0.65f), (float)(0.65f), 1);


	RECT rc = this->getClientWindowRect();
	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;
	GraphicsEngine::get()->getDeviceContext()->setViewportSize(width, height);

	SceneCameraHolder::getInstance()->getCamera()->update(EngineTime::getDeltaTime());

	//this->update();

	/*	GraphicsEngine::get()->getDeviceContext()->setConstantBuffer(this->m_vs, this->m_cb);
	GraphicsEngine::get()->getDeviceContext()->setConstantBuffer(this->m_ps, this->m_cb);

	GraphicsEngine::get()->getDeviceContext()->setVertexShader(this->m_vs);
	GraphicsEngine::get()->getDeviceContext()->setPixelShader(this->m_ps);


	GraphicsEngine::get()->getDeviceContext()->setVertexBuffer(this->m_vb);
	GraphicsEngine::get()->getDeviceContext()->setIndexBuffer(this->m_ib);
	//Cube:
	GraphicsEngine::get()->getDeviceContext()->drawIndexedTriangleList(this->m_ib->getSizeIndexList(), 0, 0);*/


	for (int i = 0; i < quads.size(); i++)
		this->quads[i].draw(width, height, this->m_vs, this->m_ps);

	for (int i = 0; i < cubes.size(); i++)
		this->cubes[i].draw(width, height, this->m_vs, this->m_ps);

	for (int i = 0; i < planes.size(); i++)
		this->planes[i].draw(width, height, this->m_vs, this->m_ps);

	//Makes it so the particles are drawn on top of everything else.
	GraphicsEngine::get()->getDeviceContext()->getDeviceContext()->
		ClearDepthStencilView(nullptr, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	ID3D11RenderTargetView* render_target_view = this->m_swap_chain->getRenderTargetView();
	GraphicsEngine::get()->getDeviceContext()->getDeviceContext()->OMSetRenderTargets
	(1, &render_target_view, nullptr);

	ParticleSystem::getInstance()->Update(EngineTime::getDeltaTime());
	ParticleSystem::getInstance()->Draw(width, height, this->m_vs, this->m_ps);

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	m_swap_chain->present(false);

	if (InputSystem::get()->isKeyDown(VK_ESCAPE))
	{
		this->onDestroy();
		exit(0);
	}
}

void AppWindow::onDestroy()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	Window::onDestroy();


	/*	if (this->m_vb != nullptr) //vertex buffers are part of the quad class instead
		this->m_vb->release();
	if (this->m_ib != nullptr);
		this->m_ib->release();
	if (this->m_cb != nullptr);
		this->m_cb->release();*/

	this->m_swap_chain->release();
	if (this->m_vs != nullptr)
		this->m_vs->release();
	if (this->m_ps != nullptr)
		this->m_ps->release();
	GraphicsEngine::get()->release();

	for (int i = 0; i < quads.size(); i++)
		quads[i].Release();

	for (int i = 0; i < cubes.size(); i++)
		cubes[i].release();

	for (int i = 0; i < planes.size(); i++)
		planes[i].release();


	ParticleSystem::destroy();
}

void AppWindow::onFocus()
{
	InputSystem::get()->addListener(this);
	InputSystem::get()->showCursor(false);
}

void AppWindow::onKillFocus()
{
	InputSystem::get()->removeListener(this);
	InputSystem::get()->showCursor(true);
}

void AppWindow::onKeyDown(int key)
{
	if (key == 'W')
		m_forward = 1.0f;
	//m_rot_x += 3.14f * EngineTime::getDeltaTime();
	else if (key == 'S')
		m_forward = -1.0f;
	//m_rot_x -= 3.14f * EngineTime::getDeltaTime();

	if (key == 'A')
		m_rightward = -1.0f;
	//m_rot_y -= 3.14f * EngineTime::getDeltaTime();
	else if (key == 'D')
		m_rightward = 1.0f;
	//m_rot_y += 3.14f * EngineTime::getDeltaTime();



}
void AppWindow::onKeyUp(int key)
{
	m_forward = 0.0f;
	m_rightward = 0.0f;
}

void AppWindow::onMouseMove(const Point& mouse_pos)
{
	RECT rc = this->getClientWindowRect();
	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;

	if (this->m_camera_rotation)
	{
		m_rot_x += (mouse_pos.y - (height / 2.0f)) * EngineTime::getDeltaTime() * 0.2f;
		m_rot_y += (mouse_pos.x - (width / 2.0f)) * EngineTime::getDeltaTime() * 0.2f;
		InputSystem::get()->setCursorPositon(Point(width / 2.0f, height / 2.0f)); // Reset cursor position to center of the window
	}


}

void AppWindow::onLeftMouseDown(const Point& mouse_pos)
{
	m_scale_cube = 0.5f;

}

void AppWindow::onLeftMouseUp(const Point& mouse_pos)
{
	m_scale_cube = 1.0f;
}

void AppWindow::onRightMouseDown(const Point& mouse_pos)
{
	m_scale_cube = 2.0f;
}

void AppWindow::onRightMouseUp(const Point& mouse_pos)
{
	m_scale_cube = 1.0f;
}

AppWindow::~AppWindow()
{

}

