#pragma once
#include "VertexStructs.h"
struct ID3D11DeviceContext;
struct ID3D11InputLayout;
struct ID3D11Buffer;
namespace Snowblind
{
	class CGBuffer;
	class CLightPass
	{
	public:
		CLightPass(CGBuffer* aGBuffer);
		~CLightPass();


		void RenderPointlight(CPointLight* pointlight, CCamera* aCamera, const CU::Matrix44f& previousOrientation);
		void RenderSpotlight(CCamera* aCamera, const CU::Matrix44f& previousOrientation);
		CEffect* GetPointlightEffect();
		CEffect* GetSpotlightEffect();
	private:
		void UpdatePointlightBuffers(CPointLight* pointlight, CCamera* aCamera, const CU::Matrix44f& previousOrientation);
		void UpdateSpotlightBuffers();

		void CreateSpotlightBuffers();
		void CreatePointlightBuffers();
		struct SPointlightConstantBuffer : public SVertexBaseStruct
		{
			CU::Vector4f scale;
		} myPointlightVertexConstantData; //Longest name.

		struct SSpotlightConstantBuffer : public SVertexBaseStruct
		{

		} mySpotlightVertexConstantData;

		struct SPixelConstantBuffer
		{
			CU::Matrix44f myInvertedProjection;
			CU::Matrix44f myView;
			CU::Vector4f myColor;
			CU::Vector4f myPosition;
			CU::Vector4f myCameraPosition;

		} myPixelConstantStruct;

		enum class eBuffer
		{
			POINT_LIGHT_VERTEX,
			POINT_LIGHT_PIXEL,
			SPOT_LIGHT_VERTEX,
			SPOT_LIGHT_PIXEL,
			_COUNT
		};
		ID3D11Buffer* myConstantBuffers[u32(eBuffer::_COUNT)];
		ID3D11DeviceContext* myContext;

		enum class eLight
		{
			POINT_LIGHT,
			SPOT_LIGHT,
			_COUNT
		};

		CEffect* myEffect[u32(eLight::_COUNT)];
		CEngine* myEngine;
		const CGBuffer* myGBuffer;
	};
};