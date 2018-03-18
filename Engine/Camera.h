#pragma once
#include "../CommonLib/Math/Matrix/Matrix44.h"
#include "../CommonLib/Math/Quaternion/Quaternion.h"

struct ControllerState;

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


class Camera
{
public:
	Camera() = default;
	void SetIsShadowCamera(bool is_shadow_camera);
	bool IsShadowCamera() const;

	void CreatePerspectiveProjection(float width, float height, float near_plane, float far_plane, float fov);
	const CU::Matrix44f& GetPerspective() const;
	CU::Matrix44f& GetPerspective();

	const CU::Matrix44f& GetInvProjection() const;
	CU::Matrix44f& GetInvProjection();

	void CreateOrthogonalProjection(float width, float height, float near_plane, float far_plane);
	const CU::Matrix44f& GetOrthogonal() const;
	CU::Matrix44f& GetOrthogonal();

	void CreateOrthographicProjection(float width, float height, float near_plane, float far_plane);
	const CU::Matrix44f& GetOrthographic() const;

	CU::Vector3f RotateAroundPoint(const CU::Vector3f& position);

	CU::Vector4f& GetPos();


	CU::Matrix44f& GetViewProjection();
	CU::Matrix44f& Get2DViewProjection();

	CU::Matrix44f& GetOrientation();
	CU::Matrix44f& GetCurrentOrientation();
	CU::Vector3f GetPosition() const;
	CU::Matrix44f& Get2DOrientation();

	const CU::Vector4f& GetAt() const;
	void SetAt(const CU::Vector4f& at);



	void Move(eDirection aDirection, float aSpeed);
	void SetPosition(const CU::Vector3f& position);
	void SetPosition2(const CU::Vector3f& position);
	void SetTranslation(const CU::Vector4f& translation);

	void Update(const ControllerState& controller_state);
	void Update();
	void Update(const CU::Vector2f& cursor_pos);

	void SetOrientation(const CU::Matrix44f& matrix);

	void RotateAroundX(float rad);
	void RotateAroundY(float rad);
	void RotateAroundZ(float rad);

	void SetRotationX(float rad);
	void SetRotationY(float rad);
	void SetRotationZ(float rad);


	void SetFOV(float field_of_view);

	float GetFOV();
	float* GetFOVRef();

	void RecalculatePerspective(float width, float height, float near_plane, float far_plane);

	void InvertPitch();
	void InvertRoll();
	void InvertYaw();
	void InvertAll();
	CU::Matrix44f& GetInv2DView();
private:
	void UpdateOrientation();

	float m_LookSpeedModifier = 0.005f; //sensitivity

	float m_RotationDegree = 0.f;
	void operator=(Camera&) = delete;
	void MoveForwardAndBack(CU::Vector4f& aPosition, float aSpeed);
	void MoveUpAndDown(CU::Vector4f& aPosition, float aSpeed);
	void MoveLeftAndRight(CU::Vector4f& aPosition, float aSpeed);


	void OrientCamera();

	CU::Matrix44f m_Orientation;
	CU::Matrix44f m_Orientation2;
	CU::Matrix44f my2DOrientation;

	CU::Matrix44f m_ProjectionMatrix;
	CU::Matrix44f m_OrthogonalMatrix;

	CU::Matrix44f m_InvProjectionMatrix;
	CU::Matrix44f m_ViewProj;

	CU::Matrix44f m_Inv2DView;

	CU::Matrix44f m_OrthogonalViewProj;

	float m_CurrentFoV = 90.f;

	float m_CameraSpeed = 50.f;
	bool m_IsShadowCamera = false;
	bool m_ProjectionCreated = false;

	CU::Vector2f m_CenterPoint;
	CU::Quaternion m_Pitch;
	CU::Quaternion m_Yaw;
	CU::Quaternion m_Roll;

};
