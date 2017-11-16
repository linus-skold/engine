#include "stdafx.h"

#include "DeferredRenderer.h"
#include "Renderer.h"
#include "RenderCommand.h"
#include "Synchronizer.h"

#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"

#include "Sprite.h"

#include "EmitterInstance.h"

#include "Terrain.h"

#include "Line3D.h"
#include "LightPass.h"
#include "ShadowSpotlight.h"
#include "GBuffer.h"
#include "Texture.h"

#include "imgui_impl_dx11.h"
#include <Engine/WaterPlane.h>

#include <Input/InputHandle.h>
#include <Input/InputWrapper.h>
#include <Engine/IGraphicsContext.h>
#include <Engine/DX11Context.h>
#include <Engine/DX11Device.h>

#if !defined(_PROFILE) && !defined(_FINAL)
#include <CommonLib/reflector.h>
#endif


Renderer::Renderer(Synchronizer* synchronizer)
	: m_Synchronizer(synchronizer)
	, m_Camera(Engine::GetInstance()->GetCamera())
{
	auto api = Engine::GetAPI();
	m_RenderContext = graphics::RenderContext(Engine::GetInstance(),
		api->GetDevice(),
		api->GetContext(),
		api);


	//myText = new CText("Data/Font/OpenSans-Bold.ttf", 8, 1);
	m_DeferredRenderer = new DeferredRenderer;

	WindowSize window_size;
	window_size.m_Height = api->GetInfo().m_WindowHeight;
	window_size.m_Width = api->GetInfo().m_WindowWidth;

	m_DepthTexture = new Texture; //Where should this live?
	m_DepthTexture->InitiateAsDepthStencil(window_size.m_Width, window_size.m_Height, "Renderer : Depth");

	m_PostProcessManager.SetPassesToProcess(PostProcessManager::HDR); //Can be read from a settings file

	m_Line = new Line3D; //Where should this live?
	m_Line->Initiate();

	m_LightPass = new graphics::LightPass(m_GBuffer);

	m_ParticleEmitter = new CEmitterInstance;
	m_ParticleEmitter->Initiate(m_Synchronizer, m_DepthTexture);

	m_Atmosphere.Initiate(1024.f, 1024.f, { 512.f, 0.f, 512.f });

	m_ShadowPass.Initiate(this);


	m_DirectionalShadow.Initiate(2048.f);

	m_Direction = CU::Vector3f(0.42f, 0.73f, 0.24f);

	myPointLight = new PointLight();

#if !defined(_PROFILE) && !defined(_FINAL)

	m_DebugTextures.Add(new Texture);
	m_DebugTextures.GetLast()->InitiateAsRenderTarget(window_size.m_Width, window_size.m_Height, "diffuse, albedo");


	m_DebugTextures.Add(new Texture);
	m_DebugTextures.GetLast()->InitiateAsRenderTarget(window_size.m_Width, window_size.m_Height, "normal");

	m_DebugTextures.Add(new Texture);
	m_DebugTextures.GetLast()->InitiateAsRenderTarget(window_size.m_Width, window_size.m_Height, "depth");

	m_DebugTextures.Add(new Texture);
	m_DebugTextures.GetLast()->InitiateAsRenderTarget(window_size.m_Width, window_size.m_Height, "metalness");

	m_DebugTextures.Add(new Texture);
	m_DebugTextures.GetLast()->InitiateAsRenderTarget(window_size.m_Width, window_size.m_Height, "roughness");

	m_DebugTextures.Add(new Texture);
	m_DebugTextures.GetLast()->InitiateAsRenderTarget(window_size.m_Width, window_size.m_Height, "emissive");

	m_DebugTextures.Add(new Texture);
	m_DebugTextures.GetLast()->InitiateAsRenderTarget(window_size.m_Width, window_size.m_Height, "entity_id");

	m_DebugTextures.Add(new Texture);
	m_DebugTextures.GetLast()->InitiateAsRenderTarget(window_size.m_Width, window_size.m_Height, "hover");

	Effect* debug_textures = m_RenderContext.GetEngine().GetEffect("Shaders/debug_textures.json");
	debug_textures->AddShaderResource(m_GBuffer.GetDiffuse(), Effect::DIFFUSE);
	debug_textures->AddShaderResource(m_GBuffer.GetNormal(), Effect::NORMAL);
	debug_textures->AddShaderResource(m_GBuffer.GetDepth(), Effect::DEPTH);
	debug_textures->AddShaderResource(m_GBuffer.GetEmissive(), Effect::EMISSIVE);
	debug_textures->AddShaderResource(m_GBuffer.GetIDTexture(), Effect::REGISTER_5);
	debug_textures->AddShaderResource(m_GBuffer.GetHoverTexture(), Effect::REGISTER_6);

	m_DebugQuad = new Quad(Engine::GetInstance()->GetEffect("Shaders/debug_textures.json"));

	debug::DebugHandle* pDebug = debug::DebugHandle::GetInstance();
	for (Texture* t : m_DebugTextures)
	{
		pDebug->AddTexture(t, t->GetDebugName());
	}

	//pDebug->RegisterCheckbox(debug::DebugCheckbox(&m_LightModelWireframe, "Light Model Wireframe"));

#endif
	m_ViewProjBuffer = m_RenderContext.GetDevice().CreateConstantBuffer(sizeof(CU::Matrix44f), "View*Projection");
	m_PerFramePixelBuffer = m_RenderContext.GetDevice().CreateConstantBuffer(sizeof(PerFramePixelBuffer), "PerFramePixelBuffer");

	m_PixelData = new char[window_size.m_Width * window_size.m_Height];

}

Renderer::~Renderer()
{
#if !defined(_PROFILE) && !defined(_FINAL)
	SAFE_DELETE(m_DebugQuad);
#endif
	delete[] m_PixelData;
	m_PixelData = nullptr;

	m_ShadowPass.CleanUp();
	m_DirectionalShadow.CleanUp();

	SAFE_DELETE(m_WaterCamera);
	SAFE_DELETE(m_Line);

	SAFE_DELETE(m_DepthTexture);

	SAFE_DELETE(m_DeferredRenderer);
	SAFE_DELETE(myPointLight);
	//SAFE_DELETE(myText);
	SAFE_DELETE(m_LightPass);
	SAFE_DELETE(m_ParticleEmitter);

	Engine::GetAPI()->ReleasePtr(m_ViewProjBuffer);
	m_ViewProjBuffer = nullptr;
}

//http://jackieokay.com/2017/04/13/reflection1.html
void Renderer::Render()
{
	if (m_Synchronizer->HasQuit())
	{
		m_Synchronizer->RenderIsDone();
		return;
	}

	PROFILE_FUNCTION(profiler::colors::Magenta);

	m_RenderContext.GetAPI().BeginFrame();
	m_RenderContext.GetAPI().ResetViewport();


	const CU::Matrix44f& camera_orientation = m_Camera->GetOrientation();
	const CU::Matrix44f& camera_projection = m_Camera->GetPerspective();
	const CU::Matrix44f& camera_view_proj = CU::Math::Inverse(camera_orientation) * camera_projection;
	m_RenderContext.GetContext().UpdateConstantBuffer(m_ViewProjBuffer, &camera_view_proj, sizeof(CU::Matrix44f));
	m_RenderContext.GetContext().VSSetConstantBuffer(0, 1, &m_ViewProjBuffer);

	m_PerFramePixelStruct.m_Projection = CU::Math::InverseReal(m_Camera->GetPerspective());
	m_PerFramePixelStruct.m_View = m_Camera->GetOrientation();
	m_PerFramePixelStruct.m_CameraPos = m_Camera->GetPosition();
	m_RenderContext.GetContext().UpdateConstantBuffer(m_PerFramePixelBuffer, &m_PerFramePixelStruct, sizeof(PerFramePixelBuffer));
	m_RenderContext.GetContext().PSSetConstantBuffer(0, 1, &m_PerFramePixelBuffer);

	m_GBuffer.Clear(clearcolor::black, m_RenderContext);
	m_GBuffer.SetAsRenderTarget(nullptr, m_RenderContext);

	RenderTerrain(false);

	if (m_RenderInstanced)
		Render3DCommandsInstanced();
	else
		Render3DCommands();

#if !defined(_PROFILE) && !defined(_FINAL)
	WriteDebugTextures();
#endif

	m_ShadowPass.ProcessShadows(&m_DirectionalShadow);

	const CU::Matrix44f& shadow_mvp = m_DirectionalShadow.GetMVP();
	m_DeferredRenderer->DeferredRender(shadow_mvp
		, m_Direction
		, m_RenderContext);

	m_RenderContext.GetContext().UpdateConstantBuffer(m_ViewProjBuffer, &camera_view_proj, sizeof(CU::Matrix44f));
	m_RenderContext.GetContext().VSSetConstantBuffer(0, 1, &m_ViewProjBuffer);
	m_RenderContext.GetContext().GSSetConstantBuffer(0, 1, &m_ViewProjBuffer);

	m_RenderContext.GetContext().UpdateConstantBuffer(m_PerFramePixelBuffer, &m_PerFramePixelStruct, sizeof(PerFramePixelBuffer));
	m_RenderContext.GetContext().PSSetConstantBuffer(0, 1, &m_PerFramePixelBuffer);
	RenderSpotlight();
	RenderPointlight();

	RenderParticles(nullptr);

	if (m_PostProcessManager.GetFlags() != 0)
		m_PostProcessManager.Process(m_DeferredRenderer->GetScene(), m_RenderContext);
	else
	{
		m_RenderContext.GetAPI().SetDefaultTargets();
		m_DeferredRenderer->Finalize();
	}

	m_RenderContext.GetContext().UpdateConstantBuffer(m_ViewProjBuffer, &camera_view_proj, sizeof(CU::Matrix44f));
	m_RenderContext.GetContext().VSSetConstantBuffer(0, 1, &m_ViewProjBuffer);
	RenderLines();


#if !defined(_PROFILE) && !defined(_FINAL)
	ImGui::Render();
#endif
	m_RenderContext.GetAPI().EndFrame();


	m_Synchronizer->WaitForLogic();

	Engine::GetInstance()->GetMemorySegmentHandle().Clear((s32)m_Synchronizer->GetCurrentBufferIndex());
	m_Synchronizer->SwapBuffer();
	m_Synchronizer->RenderIsDone();

}



#if !defined(_PROFILE) && !defined(_FINAL)
void Renderer::WriteDebugTextures()
{
	float clear[4] = { 0,0,0,0 };
	auto& ctx = m_RenderContext.GetContext();

	CU::GrowingArray<IRenderTargetView*> targets;
	for (Texture* t: m_DebugTextures)
	{
		IRenderTargetView* view = t->GetRenderTargetView();
		ctx.ClearRenderTarget(view, clear);
		targets.Add(view);
	}

	ctx.OMSetRenderTargets(targets.Size(), &targets[0], nullptr);
	m_DebugQuad->Render(false);
}
#endif

void Renderer::ProcessWater()
{
	// 	memcpy(m_WaterCamera, m_Camera, sizeof(Camera)); //This seem extremely unsafe!
	// 	Camera* old_camera = m_Camera;
	// 	m_Camera = m_WaterCamera;
	// 
	// 
	// 	m_WaterPlane->SetupRefractionRender(m_RenderContext);
	// 	m_WaterPlane->SetClipPlane({ 0.f, -1.f, 0.f, 2.f }, m_RenderContext);
	// 	RenderTerrain(true);
	// 
	// 	Render3DCommandsInstanced();
	// 
	// 	CU::Vector3f position0 = old_camera->GetPosition();
	// 	m_Camera->SetPosition(position0);
	// 
	// 	float distance = 2 * (position0.y - m_WaterPlane->GetPosition().y);
	// 	position0.y -= distance;
	// 	m_Camera->SetPosition(position0);
	// 	m_Camera->InvertAll();
	// 	m_WaterPlane->SetupReflectionRender(m_RenderContext);
	// 	m_WaterPlane->SetClipPlane({ 0.f, 1.f, 0.f, 2.f }, m_RenderContext);
	// 	RenderTerrain(true);
	// 
	// 	Render3DCommandsInstanced();
	// 	m_Atmosphere.Render(m_Camera->GetOrientation(), m_DepthTexture, m_RenderContext);
	// 
	// 	position0.y += distance;
	// 	m_Camera->SetPosition(position0);
	// 
	// 
	// 	m_Camera = old_camera;
}

void Renderer::RenderNonDeferred3DCommands()
{

	PROFILE_FUNCTION(profiler::colors::Amber);
	const auto commands = m_Synchronizer->GetRenderCommands(eBufferType::NO_DEFERRED_BUFFER);
	for (s32 i = 0; i < commands.Size(); i++)
	{
		auto command = reinterpret_cast<ModelCommandNonDeferred*>(commands[i]);
		DL_ASSERT_EXP(command->m_CommandType == RenderCommand::MODEL, "Incorrect command type! Expected MODEL");

		//m_API->SetBlendState(eBlendStates::BLEND_FALSE);
		Model* model = m_RenderContext.GetEngine().GetModel(command->m_Key);
		model->SetOrientation(command->m_Orientation);
		//m_API->SetRasterizer(command->m_Wireframe ? eRasterizer::WIREFRAME : eRasterizer::CULL_BACK);
		model->Render(m_RenderContext);

	}
}

void Renderer::Render3DCommands()
{
	PROFILE_FUNCTION(profiler::colors::Green);
	const CU::Matrix44f& orientation = m_Camera->GetCurrentOrientation();
	const CU::Matrix44f& perspective = m_Camera->GetPerspective();

	graphics::IGraphicsAPI& api = m_RenderContext.GetAPI();
	Engine& engine = m_RenderContext.GetEngine();
	graphics::IGraphicsContext& ctx = m_RenderContext.GetContext();

	ctx.SetDepthState(api.GetDepthStencilState(graphics::Z_ENABLED), 1);
	ctx.SetRasterizerState(api.GetRasterizerState(graphics::CULL_NONE));
	ctx.SetBlendState(api.GetBlendState(graphics::BLEND_FALSE));

	const u16 current_buffer = Engine::GetInstance()->GetSynchronizer()->GetCurrentBufferIndex();
	for (s32 j = 0; j < 8; j++)
	{
		const auto& commands = Engine::GetInstance()->GetMemorySegmentHandle().GetCommandAllocator(current_buffer, j);

		for (s32 i = 0; i < commands.Size(); i++)
		{
			auto command = reinterpret_cast<ModelCommand*>(commands[i]);
			DL_ASSERT_EXP(command->m_CommandType == RenderCommand::MODEL, "Incorrect command type! Expected MODEL");

			Model* model = m_RenderContext.GetEngine().GetModel(command->m_Key);
			model->SetOrientation(command->m_Orientation);
			model->Render(m_RenderContext);
		}
	}
}

void Renderer::Render3DCommandsInstanced()
{
	PROFILE_FUNCTION(profiler::colors::Green);

	const u16 current_buffer = Engine::GetInstance()->GetSynchronizer()->GetCurrentBufferIndex();
	graphics::IGraphicsAPI& api = m_RenderContext.GetAPI();
	graphics::IGraphicsContext& ctx = m_RenderContext.GetContext();
	Engine& engine = m_RenderContext.GetEngine();

	ctx.SetDepthState(api.GetDepthStencilState(graphics::Z_ENABLED), 1);
	ctx.SetRasterizerState(api.GetRasterizerState(graphics::CULL_BACK));
	ctx.SetBlendState(api.GetBlendState(graphics::BLEND_FALSE));
	for (s32 top_tree_node = 0; top_tree_node < 8; top_tree_node++)
	{
		const auto& commands = Engine::GetInstance()->GetMemorySegmentHandle().GetCommandAllocator(current_buffer, top_tree_node);
		for (s32 i = 0; i < commands.Size(); i++)
		{
			ProcessCommand(commands, i, engine);
		}
	}

	m_InstancingManager.DoInstancing(m_RenderContext, false);
}

void Renderer::RenderTerrain(bool override_effect)
{
	graphics::IGraphicsContext& ctx = m_RenderContext.GetContext();
	graphics::IGraphicsAPI& api = m_RenderContext.GetAPI();

	ctx.SetDepthState(api.GetDepthStencilState(graphics::Z_ENABLED), 1);
	ctx.SetRasterizerState(api.GetRasterizerState(graphics::CULL_BACK));
	ctx.SetBlendState(api.GetBlendState(graphics::BLEND_FALSE));
	PROFILE_FUNCTION(profiler::colors::Green);
	for (Terrain* terrain : myTerrainArray)
	{
		if (!terrain->HasLoaded())
			continue;

		terrain->Render(m_RenderContext);

	}
}

void Renderer::Render3DShadows(const CU::Matrix44f&, Camera* camera)
{
	const CU::Matrix44f& perspective = camera->GetPerspective();
	const CU::Matrix44f& orientation = camera->GetCurrentOrientation();
	const CU::Matrix44f& view_proj = CU::Math::Inverse(orientation) * perspective;
	m_RenderContext.GetContext().UpdateConstantBuffer(m_ViewProjBuffer, &view_proj, sizeof(CU::Matrix44f));

	const u16 current_buffer = Engine::GetInstance()->GetSynchronizer()->GetCurrentBufferIndex();
	graphics::IGraphicsAPI& api = m_RenderContext.GetAPI();
	graphics::IGraphicsContext& ctx = m_RenderContext.GetContext();
	Engine& engine = m_RenderContext.GetEngine();

	ctx.SetDepthState(api.GetDepthStencilState(graphics::Z_ENABLED), 1);
	ctx.SetRasterizerState(api.GetRasterizerState(graphics::CULL_NONE));
	ctx.SetBlendState(api.GetBlendState(graphics::BLEND_FALSE));
	for (s32 top_tree_node = 0; top_tree_node < 8; top_tree_node++)
	{
		const auto& commands = Engine::GetInstance()->GetMemorySegmentHandle().GetCommandAllocator(current_buffer, top_tree_node);
		for (s32 i = 0; i < commands.Size(); i++)
		{
			ProcessCommand(commands, i, engine);
		}
	}

	m_InstancingManager.DoInstancing(m_RenderContext, true);

}

void Renderer::Render2DCommands()
{

	// 	m_API->SetRasterizer(eRasterizer::CULL_NONE);
	// 	m_API->SetDepthStencilState(eDepthStencilState::Z_DISABLED, 0);
	// 	m_API->SetBlendState(eBlendStates::NO_BLEND);
	// 
	// 	//_________________________
	// 	// RenderSpriteCommands function?
	// 	m_API->SetBlendState(eBlendStates::ALPHA_BLEND);
	// 	const auto commands0 = m_Synchronizer->GetRenderCommands(eBufferType::SPRITE_BUFFER);
	// 	for (s32 i = 0; i < commands0.Size(); i++)
	// 	{
	// 		auto command = reinterpret_cast<SpriteCommand*>(commands0[i]);
	// 		DL_ASSERT_EXP(command->m_CommandType == RenderCommand::SPRITE, "Expected Sprite command type");
	// 		Sprite* sprite = m_Engine->GetSprite(command->m_Key);
	// 		sprite->SetPosition(command->m_Position);
	// 		sprite->Render(m_Camera);
	// 		//mySprite->SetPosition(command->m_Position);
	// 		//mySprite->SetShaderView(command->m_Resource);
	// 		//mySprite->Render(m_Camera);
	// 	}
	// 
	// 	//_________________________
	// 	// RenderTextCommands function?
	// 	const auto commands1 = m_Synchronizer->GetRenderCommands(eBufferType::TEXT_BUFFER);
	// 	for (s32 i = 0; i < commands1.Size(); i++)
	// 	{
	// 		auto command = reinterpret_cast<TextCommand*>(commands1[i]);
	// 		DL_ASSERT_EXP(command->m_CommandType == RenderCommand::TEXT, "Expected Text command type");
	// 		myText->SetText(command->m_TextBuffer);
	// 		myText->SetPosition(command->m_Position);
	// 		myText->Render(m_Camera);
	// 	}
	// 
	// 	m_API->SetDepthStencilState(eDepthStencilState::Z_ENABLED, 1);
	// 	m_API->SetRasterizer(eRasterizer::CULL_BACK);

}

void Renderer::RenderSpotlight()
{

	PROFILE_FUNCTION(profiler::colors::Purple);

	SpotlightData data;
	const auto commands = m_Synchronizer->GetRenderCommands(eBufferType::SPOTLIGHT_BUFFER);

	PROFILE_BLOCK("Spotlight Command Loop", profiler::colors::Red);

	for (s32 i = 0; i < commands.Size(); i++)
	{
		auto command = reinterpret_cast<SpotlightCommand*>(commands[i]);
		DL_ASSERT_EXP(command->m_CommandType == RenderCommand::SPOTLIGHT, "Expected Spotlight command type");

		data.myAngle = command->m_Angle;
		data.myRange = command->m_Range;
		data.myLightColor = command->m_Color;
		data.myLightPosition = command->m_Orientation.GetPosition();
		data.myOrientation = command->m_Orientation;

		SpotLight* light = m_Spotlights[command->m_LightID];
		light->SetData(data);

		CU::Matrix44f shadow_mvp;

#if !defined(_FINAL) && !defined(_PROFILE)
		m_RenderContext.GetContext().SetRasterizerState(m_RenderContext.GetAPI().GetRasterizerState(m_LightModelWireframe ? graphics::WIREFRAME : graphics::CULL_NONE));
#else
		m_RenderContext.GetContext().SetRasterizerState(m_RenderContext.GetAPI().GetRasterizerState(graphics::CULL_NONE));
#endif
		m_LightPass->RenderSpotlight(light, m_Camera->GetOrientation(), m_Camera->GetPerspective(), shadow_mvp, m_RenderContext);

	}
	PROFILE_BLOCK_END;
}

void Renderer::RenderPointlight()
{
	PROFILE_FUNCTION(profiler::colors::Purple);
	const auto commands = m_Synchronizer->GetRenderCommands(eBufferType::POINTLIGHT_BUFFER);

	PROFILE_BLOCK("Pointlight Command Loop", profiler::colors::Red);
	for (s32 i = 0; i < commands.Size(); i++)
	{
		auto command = reinterpret_cast<PointlightCommand*>(commands[i]);

		DL_ASSERT_EXP(command->m_CommandType == RenderCommand::POINTLIGHT, "Wrong command type in pointlight buffer.");
		myPointLight->SetPosition(command->m_Orientation.GetPosition());
		myPointLight->SetRange(command->m_Range);
		myPointLight->SetColor(CU::Vector4f(command->m_Color.x, command->m_Color.y, command->m_Color.z, 1));
		myPointLight->Update();
		CU::Matrix44f shadow_mvp;
#if !defined(_FINAL) && !defined(_PROFILE)
		m_RenderContext.GetContext().SetRasterizerState(m_RenderContext.GetAPI().GetRasterizerState(m_LightModelWireframe ? graphics::WIREFRAME : graphics::CULL_NONE));
#else
		m_RenderContext.GetContext().SetRasterizerState(m_RenderContext.GetAPI().GetRasterizerState(graphics::CULL_NONE));
#endif
		m_LightPass->RenderPointlight(myPointLight, m_Camera->GetOrientation(), m_Camera->GetOrientation(), shadow_mvp, m_RenderContext);
	}
	PROFILE_BLOCK_END;
}

void Renderer::RenderParticles(Effect* effect)
{
	m_RenderContext.GetContext().SetBlendState(m_RenderContext.GetAPI().GetBlendState(graphics::PARTICLE_BLEND));
	const auto commands = m_Synchronizer->GetRenderCommands(eBufferType::PARTICLE_BUFFER);
	for (s32 i = 0; i < commands.Size(); i++)
	{
		auto command = reinterpret_cast<ParticleCommand*>(commands[i]);
		DL_ASSERT_EXP(command->m_CommandType == RenderCommand::PARTICLE, "Expected particle command type");
		m_ParticleEmitter->SetPosition(command->m_Position);

		m_ParticleEmitter->Update(m_RenderContext.GetEngine().GetDeltaTime());


		m_RenderContext.GetContext().SetRasterizerState(m_RenderContext.GetAPI().GetRasterizerState(graphics::CULL_NONE));
		m_ParticleEmitter->Render(m_Camera->GetOrientation(), m_Camera->GetPerspective(), effect);
	}
}

void Renderer::RenderLines()
{

	PROFILE_FUNCTION(profiler::colors::Amber);
	const auto commands = m_Synchronizer->GetRenderCommands(eBufferType::LINE_BUFFER);
	PROFILE_BLOCK("Line Command Loop", profiler::colors::Red);
	for (s32 i = 0; i < commands.Size(); i++)
	{
		auto command = reinterpret_cast<LineCommand*>(commands[i]);
		const bool result = command->m_CommandType == RenderCommand::LINE;
		if (!result)
			return;
		DL_ASSERT_EXP(command->m_CommandType == RenderCommand::LINE, "Expected Line command type");
		m_Line->AddLine(command->m_Points);
	}
	m_Line->Render(0, m_RenderContext);
	PROFILE_BLOCK_END;
}

void Renderer::ProcessCommand(const memory::CommandAllocator& commands, s32 i, Engine& engine)
{
	auto command = reinterpret_cast<ModelCommand*>(commands[i]);
	const bool result = (command->m_CommandType == RenderCommand::MODEL);
	DL_ASSERT_EXP(result == true, "Incorrect command type! Expected MODEL");

	Model* model = engine.GetModel(command->m_Key);

	if (command->m_MaterialKey > 0 && !m_InstancingManager.FindInstanceObject(command->m_MaterialKey))
	{
		InstanceObject new_instance;
		new_instance.m_Material = model->GetMaterial() ? model->GetMaterial() : m_RenderContext.GetEngine().GetMaterial(command->m_MaterialKey);
		new_instance.m_Model = model;
		new_instance.m_Shadowed = true; /* should be command->m_Shadowed or something*/
		m_InstancingManager.AddInstanceObject(new_instance);
	}

	//m_InstancingManager.AddOrientationToInstance(command->m_MaterialKey, command->m_Orientation);

	GPUModelData model_data;
	model_data.m_Orientation = command->m_Orientation;
	model_data.m_ID = command->m_EntityID;
#ifdef _DEBUG
	model_data.m_Hovering = (command->m_EntityID == debug::DebugHandle::GetInstance()->GetEntity() ? 1 : 0);
#endif
	CU::Vector4f col = cl::IntToCol(model_data.m_ID);

	m_InstancingManager.AddGPUDataToInstance(command->m_MaterialKey, model_data);
}

//Move this to some kind of light manager
int Renderer::RegisterLight()
{
	SpotLight* s = new SpotLight;
	m_Spotlights.Add(s);
	return (m_Spotlights.Size() - 1);
}

//Terrain Manager not good enough to just loop through?
void Renderer::AddTerrain(Terrain* someTerrain)
{
	myTerrainArray.Add(someTerrain);
}