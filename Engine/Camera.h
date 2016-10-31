#pragma once
#include "../CommonLib/Math/Matrix/Matrix44.h"
#include "../CommonLib/Math/Quaternion/Quaternion.h"

struct ControllerState;
namespace Snowblind
{
	enum class eDirection
	{
		FORWARD,
		BACK,
		UP,
		DOWN,
		LEFT,
		RIGHT
	};

	enum class eRotation
	{
		X_AXIS,
		Y_AXIS,
		Z_AXIS
	};


	class CCamera
	{
	public:
		CCamera(float aWidth, float aHeight);
		CCamera(float aWidth, float aHeight, const CU::Vector3f& aPosition);

		const CU::Vector3f GetPosition() const;
		CU::Matrix44f& GetOrientation();
		CU::Matrix44f& Get2DOrientation();

		CU::Matrix44f& GetProjection();

		CU::Matrix44f& GetOrthogonalMatrix();

		void Move(eDirection aDirection, float aSpeed);
		void SetPosition(const CU::Vector3f& position);
		void SetTranslation(const CU::Vector4f& translation);
		void Update(const ControllerState& controller_state);
		void SetOrientation(const CU::Matrix44f& matrix);
	private:
		void operator=(CCamera&) = delete;
		void MoveForwardAndBack(CU::Vector4f& aPosition, float aSpeed);
		void MoveUpAndDown(CU::Vector4f& aPosition, float aSpeed);
		void MoveLeftAndRight(CU::Vector4f& aPosition, float aSpeed);
		float myFOV = 90.f;
		CU::Matrix44f myOrientation;
		CU::Matrix44f my2DOrientation;

		CU::Matrix44f myProjectionMatrix;
		CU::Matrix44f myOrthogonalMatrix;


		CU::Vector2f m_CenterPoint;
		CU::Quaternion myPitch;
		CU::Quaternion myYaw;

	};

	inline const CU::Vector3f CCamera::GetPosition() const
	{
		return myOrientation.GetPosition();
	}

	inline CU::Matrix44f& CCamera::GetOrientation()
	{
		return myOrientation;
	}

	inline CU::Matrix44f& CCamera::Get2DOrientation()
	{
		return my2DOrientation;
	}

	inline CU::Matrix44f& CCamera::GetProjection()
	{
		return myProjectionMatrix;
	}
}