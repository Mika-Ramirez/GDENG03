#pragma once
#include <vector>

#include "Window.h"
#include "GraphicsEngine.h"
#include "SwapChain.h"
#include "DeviceContext.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "InputListener.h"
#include "Matrix4x4.h"


#include "Quad.h"
#include "Cube.h"
#include "Plane.h"


class AppWindow : public Window, public InputListener
{
	public:
		static AppWindow* getInstance();
		static void initialize();
		static void destroy();

		virtual void createGraphicsWindow();
		virtual void onCreate() override;
		virtual void onUpdate() override;
		virtual void onDestroy() override;
		virtual void onFocus() override;
		virtual void onKillFocus() override;

		virtual void onKeyDown(int key) override;
		virtual void onKeyUp(int key) override;

		virtual void onMouseMove(const Point& mouse_pos) override;
		virtual void onLeftMouseDown(const Point& mouse_pos) override;
		virtual void onLeftMouseUp(const Point& mouse_pos) override;
		virtual void onRightMouseDown(const Point& mouse_pos) override;
		virtual void onRightMouseUp(const Point& mouse_pos) override;

	private:
		AppWindow();
		
		void update();

		~AppWindow();
		AppWindow(const AppWindow&) {};
		AppWindow& operator=(const AppWindow&) {};
		static AppWindow* sharedInstance;

		SwapChain* m_swap_chain;
		VertexBuffer* m_vb = nullptr;
		IndexBuffer* m_ib = nullptr;
		ConstantBuffer* m_cb = nullptr;
		VertexShader* m_vs = nullptr;
		PixelShader* m_ps = nullptr;

		float m_delta_pos = 0.0f;
		float m_delta_scale = 0.0f;
		float m_angle = 0;

		float m_rot_x = 0.0f;
		float m_rot_y = 0.0f;

		float m_scale_cube = 1.0f;
		float m_forward = 0.0f;
		float m_rightward = 0.0f;
		bool m_camera_rotation = false;
		Matrix4x4 m_world_cam;

		
		std::vector<Quad> quads;
		std::vector<Cube> cubes;
		std::vector<Plane> planes;

private:
	// Add fog culling members
	float m_fog_cull_distance = 25.0f;  // Distance at which objects are culled
	bool m_fog_culling_enabled = true;   // Toggle for fog culling

};

