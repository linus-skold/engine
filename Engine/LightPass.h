#pragma once
#include "VertexStructs.h"
#include "snowblind_shared.h"

class GBuffer;
class SpotLight;
class LightPass
{
public:
	LightPass() = default;

	bool Initiate(const GBuffer& gbuffer, Texture* shadow_texture);
	bool CleanUp();
	void RenderPointlight(PointLight* pointlight, Camera* aCamera, const CU::Matrix44f& previousOrientation, const CU::Matrix44f& shadow_matrix, const RenderContext& render_context);
	void RenderSpotlight(SpotLight* spotlight, Camera* aCamera, const CU::Matrix44f& previousOrientation, const CU::Matrix44f& shadow_matrix, const RenderContext& render_context);
	Effect* GetPointlightEffect();
	Effect* GetSpotlightEffect();
	bool HasInitiated();

private:
	void UpdatePointlightBuffers(PointLight* pointlight, Camera* aCamera, const CU::Matrix44f& previousOrientation, const CU::Matrix44f& shadow_matrix);
	void UpdateSpotlightBuffers(SpotLight* spotlight, Camera* aCamera, const CU::Matrix44f& previousOrientation, const CU::Matrix44f& shadow_matrix);

	void CreateSpotlightBuffers();
	void CreatePointlightBuffers();
	struct PointlightConstantBuffer : public VertexBaseStruct
	{
	} myPointlightVertexConstantData; //Longest name.

	struct SpotlightConstantBuffer : public VertexBaseStruct
	{
	} mySpotlightVertexConstantData;

	struct PixelConstantBuffer
	{
		CU::Matrix44f myInvertedProjection;
		CU::Matrix44f myView;
		CU::Vector4f myColor;
		CU::Vector4f myPosition;
		CU::Vector4f myCameraPosition;
		CU::Matrix44f m_ShadowMVP;

	} myPixelConstantStruct;

	struct SpotPixelConstantBuffer : public PixelConstantBuffer
	{
		CU::Vector4f myDirection;
	} mySpotPixelConstantStruct;


	enum eBuffer
	{
		POINTLIGHT_VERTEX,
		POINTLIGHT_PIXEL,
		SPOTLIGHT_VERTEX,
		SPOTLIGHT_PIXEL,
		NOF_BUFFERS
	};
	ID3D11Buffer* myConstantBuffers[NOF_BUFFERS];

	enum eLight
	{
		POINTLIGHT,
		SPOTLIGHT,
		NOF_LIGHT_TYPES
	};

	Effect* myEffect[NOF_LIGHT_TYPES];
};
