#pragma once
#include "GameObject.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include "Vector3D.h"

class Cube : public GameObject
{
	public:
		Cube(std::string name, void* shaderByteCode, size_t sizeShader);
		Cube(std::string name, Vector3D Color ,void* shaderByteCode, size_t sizeShader);
		~Cube();

		void update(float deltaTime) override;
		void draw(int width, int height, VertexShader* vs, PixelShader* ps) override;
		void setAnimSpeed(float speed);

		void release();


	
	private:
		VertexBuffer* vertexBuffer;
		IndexBuffer* indexBuffer;
		ConstantBuffer* constantBuffer;
		float ticks = 0.0f;
		float deltaPos = 0.0f;
		float deltaScale = 0.0f;
		float deltaTime = 0.0f;
		float speed = 1.57f; //default is 1.57f for PardCode



		 
};

