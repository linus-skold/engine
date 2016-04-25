#pragma once
#include <string>
#include "../CommonLib/Math/Matrix/Matrix44.h"
#include <DataStructures/GrowingArray.h>
#include "VertexStructs.h"

struct ID3D11InputLayout;
struct D3D11_INPUT_ELEMENT_DESC;

namespace Snowblind
{
	class CDirectX11;
	class CEffect;
	class CCamera;
	class CSurface;
	struct SVertexIndexWrapper;
	struct SVertexBufferWrapper;
	struct SVertexDataWrapper;
	struct SIndexBufferWrapper;

	class CModel
	{
	public:
		CModel();
		~CModel();

		void CreateTriangle(const std::string& anEffectPath);
		void CreateCube(const std::string& anEffectPath, float aWidth, float aHeight, float aDepth);
		void CreateTexturedCube(const std::string& anEffectPath, float aWidth, float aHeight, float aDepth);
		void Render();
		void RenderPolygon();
		CEffect* GetEffect();
	private:

		void InitVertexBuffer();
		void InitIndexBuffer();

		CEffect	*myEffect;


		ID3D11InputLayout *myVertexLayout;

		CU::GrowingArray<SVertexTypePosCol> myVertices;
		CU::GrowingArray<D3D11_INPUT_ELEMENT_DESC> myVertexFormat;
		CU::GrowingArray<CSurface*> mySurfaces;

		SVertexIndexWrapper		*myIndexData;
		SVertexDataWrapper		*myVertexData;

		SVertexBufferWrapper	*myVertexBuffer;
		SIndexBufferWrapper		*myIndexBuffer;

		Matrix44f myOrientation;
		CDirectX11* myAPI;
		bool myIsTextured;
		CCamera* myCamera;
	};
	
	__forceinline CEffect* CModel::GetEffect()
	{
		return myEffect;
	}
}