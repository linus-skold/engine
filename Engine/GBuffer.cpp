#include "stdafx.h"
#include "GBuffer.h"
#include <Engine/engine_shared.h>
#include <Engine/IGraphicsContext.h>
#include <Engine/IGraphicsAPI.h>

#if !defined(_PROFILE) && !defined(_FINAL)
#include <CommonLib/reflector.h>
#endif
namespace graphics
{

	GBuffer::GBuffer()
	{
	}

	GBuffer::~GBuffer()
	{
		SAFE_DELETE(m_Albedo);
		SAFE_DELETE(m_Normal);
		SAFE_DELETE(m_Depth);
		SAFE_DELETE(m_Emissive);
#ifdef _DEBUG
		SAFE_DELETE(m_Metalness);
		SAFE_DELETE(m_Roughness);
		//SAFE_DELETE(m_EntityIDTexture); //responsibility moved to AssetsContainer
#endif
	}

	void GBuffer::Initiate(bool bind_textures)
	{

		const WindowSize windowSize = Engine::GetInstance()->GetInnerSize();
		const float window_width = windowSize.m_Width;
		const float window_height = windowSize.m_Height;

		TextureDesc desc;
		desc.m_Width = window_width;
		desc.m_Height = window_height;
		desc.m_Usage = graphics::DEFAULT_USAGE;
		desc.m_ResourceTypeBinding = graphics::BIND_SHADER_RESOURCE | graphics::BIND_RENDER_TARGET;
		desc.m_ShaderResourceFormat = RGBA16_FLOAT;
		desc.m_RenderTargetFormat = RGBA16_FLOAT;
		desc.m_TextureFormat = RGBA16_FLOAT;

		m_Albedo = new Texture;
		m_Albedo->Initiate(desc, false, "GBuffer : Albedo");

		m_Emissive = new Texture;
		m_Emissive->Initiate(desc, false, "GBuffer : Emissive");

		m_Normal = new Texture;
		m_Normal->Initiate(desc, false, "GBuffer : Normal");

		m_Depth = new Texture;
		desc.m_ShaderResourceFormat = RGBA32_FLOAT;
		desc.m_RenderTargetFormat = RGBA32_FLOAT;
		m_Depth->Initiate(desc, false, "GBuffer : Depth");

#ifdef _DEBUG

		desc.m_ShaderResourceFormat = RGBA16_FLOAT;
		desc.m_RenderTargetFormat = RGBA16_FLOAT;
		m_Metalness = new Texture;
		m_Metalness->Initiate(desc, false, "GBuffer : Metalness");

		m_Roughness = new Texture;
		m_Roughness->Initiate(desc, false, "GBuffer : Roughenss");

		m_EntityIDTexture = new Texture;
		desc.m_ResourceTypeBinding = graphics::BIND_SHADER_RESOURCE | graphics::BIND_RENDER_TARGET;
		desc.m_TextureFormat = RGBA32_FLOAT;
		desc.m_ShaderResourceFormat = R32_UINT;
		desc.m_RenderTargetFormat = R32_UINT;
		m_EntityIDTexture->Initiate(desc, false, "Entity ID");

		Engine::GetInstance()->AddTexture(m_EntityIDTexture, cl::Hash("entity_id")); 
#endif

		if (bind_textures)
		{

			Effect* shader = Engine::GetInstance()->GetEffect("Shaders/deferred_ambient.json");
			shader->AddShaderResource(m_Albedo, TextureSlot::DIFFUSE);
			shader->AddShaderResource(m_Depth, TextureSlot::DEPTH);
			shader->AddShaderResource(m_Normal, TextureSlot::NORMAL);
			shader->AddShaderResource(m_Emissive, TextureSlot::EMISSIVE);
		}
	}

	void GBuffer::Clear(const float* clear_color, const RenderContext& render_context)
	{
		auto& ctx = render_context.GetContext();
		ctx.ClearRenderTarget(m_Albedo, clear_color);
		ctx.ClearRenderTarget(m_Normal, clear_color);
		ctx.ClearRenderTarget(m_Depth, clear_color);
		ctx.ClearRenderTarget(m_Emissive, clear_color);
#ifdef _DEBUG
		ctx.ClearRenderTarget(m_Emissive, clear_color);
		ctx.ClearRenderTarget(m_Roughness, clear_color);
		ctx.ClearRenderTarget(m_Metalness, clear_color);
		ctx.ClearRenderTarget(m_EntityIDTexture, clear_color);
#endif
	}

	void GBuffer::SetAsRenderTarget(Texture* depth, const RenderContext& render_context)
	{
		IRenderTargetView* target[] =
		{
			m_Albedo->GetRenderTargetView(),
			m_Normal->GetRenderTargetView(),
			m_Depth->GetRenderTargetView(),
			m_Emissive->GetRenderTargetView(),
#ifdef _DEBUG
			m_EntityIDTexture->GetRenderTargetView(),
			m_Roughness->GetRenderTargetView(),
			m_Metalness->GetRenderTargetView(),
#endif
		};

		render_context.GetContext().OMSetRenderTargets(ARRAYSIZE(target), target, render_context.GetAPI().GetDepthView());
	}

};