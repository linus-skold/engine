#pragma once
#include <Engine/engine_shared.h>
#include <CommonLib/Math/Matrix/Matrix33.h>
#include <Math/Matrix/Matrix.h>


#include "LightStructs.h"
#include "ShadowPass.h"
#include "LightPass.h"
#include "RenderCommand.h"
#include "PostProcessManager.h"

#include <Engine/Atmosphere.h>
#include <Engine/Shadow_Directional.h>
#include <Engine/ShaderState.h>
#include <Engine/GBuffer.h>


class CLine3D;

class Camera;
class DeferredRenderer;
class DirectionalLight;
class CEmitterInstance;
class Model;
class PointLight;
class Synchronizer;
class Texture;
class CText;
class Terrain;
class LightPass;
class SpotLight;
class ShadowSpotlight;
class CommandAllocator;
class Renderer
{
public:
	Renderer() = default;

	bool Initiate(Synchronizer* synchronizer, Camera* camera);
	bool CleanUp();

	void Render();

	void ProcessWater();

	void AddTerrain(Terrain* someTerrain);

	void Render3DShadows(const CU::Matrix44f& orientation, Camera* camera);

	int RegisterLight();
	void SetDirection(const CU::Vector3f& dir) { m_Direction = dir; }
	Camera* GetDirectionalCamera() { return m_DirectionalShadow.GetCamera(); }
private:
	void RenderNonDeferred3DCommands();
	void Render3DCommands();

	void Render3DCommandsInstanced();

	void ProcessCommand(const memory::CommandAllocator& commands, s32 i);

	void RenderTerrain(bool override_effect);

	void Render2DCommands();

	void RenderPointlight();
	void RenderSpotlight();

	void RenderParticles(Effect* effect);
	void RenderLines();

	GBuffer m_GBuffer;


	CU::GrowingArray<Model*> m_Models;
	std::map<std::string, Model*> m_ModelsToRender;
	CU::GrowingArray<Terrain*> myTerrainArray;

	CU::Vector3f		m_Direction;
	CU::Vector3f		m_OriginalDirection;
	CU::Matrix33f		m_Orientation;

	CU::GrowingArray<SpotLight*> m_Spotlights;

	PostProcessManager			m_PostProcessManager;
	LightPass					m_LightPass;
	ShadowPass					m_ShadowPass;
	Atmosphere					m_Atmosphere;

	graphics::RenderContext		m_RenderContext;

	Camera*						m_Camera				= nullptr;
	Camera*						m_WaterCamera			= nullptr;

	DeferredRenderer*			m_DeferredRenderer		= nullptr;

	DirectionalLight*			myDirectionalLight		= nullptr;
	PointLight*					myPointLight			= nullptr;
	SpotLight*					mySpotlight				= nullptr;


	Synchronizer*				mySynchronizer			= nullptr;
	CText*						myText					= nullptr;
	Texture*					m_DepthTexture			= nullptr;
	Texture*					m_ParticleBuffer		= nullptr;
	Texture*					m_ParticleDiff			= nullptr;
	Texture*					m_ParticleDepth			= nullptr;
	class Quad*					m_Quad = nullptr;
	Sprite*						mySprite				= nullptr;
	Sprite*						myClearColor			= nullptr;

	CLine3D*					m_Line					= nullptr;

	CEmitterInstance*			m_ParticleEmitter		= nullptr;

	ShadowDirectional			m_DirectionalShadow;

	class WaterPlane*			m_WaterPlane				= nullptr; //Shouldn't be in here

	ShaderState m_LightState;

	float m_SpriteWidth = 0.f;
	float m_SpriteHeight = 0.f;


#if !defined(_PROFILE) && !defined(_FINAL)
	bool m_RenderLines = false;

	Texture* m_DebugTexture0 = nullptr; // Albedo / Diffuse
	Texture* m_DebugTexture1 = nullptr; // Normal
	Texture* m_DebugTexture2 = nullptr; // Depth
	Texture* m_DebugTexture3 = nullptr; // Roughness
	Texture* m_DebugTexture4 = nullptr; // Metalness
	Quad* m_DebugQuad = nullptr;
	void WriteDebugTextures();

public:
	void SetRenderLines(bool render_lines) { m_RenderLines = render_lines; }
	bool GetRenderLines() { return m_RenderLines; }
	DeferredRenderer* GetDeferredRenderer() {	return m_DeferredRenderer; };

#endif
	IBuffer* m_PBLValues = nullptr;
	struct 
	{
		float rough;
		float metal;
		float d;
		float d0;
	} m_values;

	struct cbParticle
	{
		CU::Matrix44f view;
		CU::Matrix44f invProjection;
		CU::Vector4f m_ViewDir;
	} m_cbParticles;
	IBuffer* m_cbParticleBuf;


	struct cbCalcSSNormal
	{
		CU::Matrix44f m_Projection;
		CU::Matrix44f m_InvProjection;
		CU::Vector4f m_TexelSize;
	} m_CalcSSNormal;
	IBuffer* m_cbCalcSSNormal;




};
