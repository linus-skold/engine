#pragma once
namespace Hex
{

	class Camera;
	class Engine;
	class CMousePicker
	{
	public:
		CMousePicker();
		~CMousePicker();

		const CU::Vector3f& GetCurrentRay(float x, float y);
	private:
		void CalculateCurrentRay(float x, float y);
		CU::Vector4f ToEyeCoords(const CU::Vector4f& clipCoords);
		CU::Vector3f ToWorldCoords(const CU::Vector4f& eyeCoords);
		CU::Vector2f NormalizedSpace(float x, float y);
		CU::Vector3f myCurrentRay;
		Camera* myCamera;
		Engine* myEngine;
	};
};