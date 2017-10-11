#pragma once
#include <Math/Vector/Vector.h>
#include <Math/Matrix/Matrix.h>
struct SVertexTypePosCol
{
	CU::Math::Vector4<float> myPosition;
	CU::Math::Vector4<float> myColor;
};

struct VertexTypePosUV
{
	CU::Math::Vector4<float> myPosition;
	CU::Math::Vector2<float> myUV;
};

struct SVertexTypePosColUv
{
	CU::Math::Vector3<float> myPosition;
	CU::Math::Vector4<float> myColor;
	CU::Math::Vector2<float> myUV;
};

struct SVertexTypePosColUv2
{
	CU::Math::Vector3<float> myPosition;
	CU::Math::Vector4<float> myColor;
	CU::Math::Vector2<float> myUV;
	CU::Math::Vector2<float> myUV2;
};


struct VertexTypePosNormUV
{
	CU::Math::Vector3<float> myPosition;
	CU::Math::Vector3<float> myNormal;
	CU::Math::Vector2<float> myUV;
};

struct SVertexPosNormUVBiTang
{
	CU::Math::Vector3<float> position;
	CU::Math::Vector3<float> normal;
	CU::Math::Vector2<float> uv;
	CU::Math::Vector3<float> binormal;
	CU::Math::Vector3<float> tangent;
};

struct SDefaultCube
{
	CU::Vector4f m_Position;
	CU::Vector4f m_Normal;
	CU::Vector2f m_UV;
	CU::Vector4f m_BiNormal;
	CU::Vector4f m_Tangent;
};

struct VertexBaseStruct
{
	CU::Matrix44f world; 
	CU::Matrix44f invertedView;
	CU::Matrix44f projection;
	CU::Vector4f scale;
};