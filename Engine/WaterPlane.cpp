#include "stdafx.h"
#include "WaterPlane.h"
#include <Engine/Texture.h>
#include <Engine/IGraphicsDevice.h>
#include <Engine/IGraphicsContext.h>


WaterPlane::WaterPlane()
{
	m_Effect = Engine::GetInstance()->GetEffect("Shaders/water.json");


	m_RefractionG.Initiate(false);
	m_ReflectionG.Initiate(false);

	const WindowSize& window_size = Engine::GetInstance()->GetInnerSize();
	const s32 window_width = window_size.m_Width;
	const s32 window_height = window_size.m_Height;

	m_Refraction = new Texture;
	m_Refraction->InitiateAsDepthStencil(window_width, window_height, "Water : RefractionDepth");

	m_Reflection = new Texture;
	m_Reflection->InitiateAsDepthStencil(window_width, window_height, "Water : ReflectionDepth");

	m_Effect->AddShaderResource(m_RefractionG.GetDiffuse(), TextureSlot::REFRACTION);
	m_Effect->AddShaderResource(m_ReflectionG.GetDiffuse(), TextureSlot::REFLECTION);

	Engine* engine = Engine::GetInstance();

	m_Effect->AddShaderResource(engine->GetTexture("Data/Textures/T_cubemap_level01.dds"), TextureSlot::CUBEMAP);
	m_Effect->AddShaderResource(engine->GetTexture("Data/Textures/water_normal.dds"), TextureSlot::NORMAL);
	m_Effect->AddShaderResource(engine->GetTexture("Data/Textures/water_dudv.dds"), TextureSlot::DUDV);
	CreatePlane(2048);
	m_ConstantBuffer = engine->GetAPI()->GetDevice().CreateConstantBuffer(sizeof(cbMatrices), "waterplane vertex cb");
	m_cbPixel = engine->GetAPI()->GetDevice().CreateConstantBuffer(sizeof(cbPixel), "waterplane pixel cb");

}

WaterPlane::~WaterPlane()
{
	SAFE_DELETE(m_Reflection);
	SAFE_DELETE(m_Refraction);
	Engine::GetAPI()->ReleasePtr(m_cbPixel);

}

void WaterPlane::SetPosition(const CU::Vector3f& position)
{
	m_Orientation.SetPosition(position);
}

void WaterPlane::UpdateConstantBuffer(const graphics::RenderContext& rc)
{
	m_VertexMatrices.m_World = m_Orientation;
	m_VertexMatrices.m_Time = Engine::GetInstance()->GetTotalTime();
	rc.GetContext().UpdateConstantBuffer(m_ConstantBuffer, &m_VertexMatrices, sizeof(cbMatrices));
}

void WaterPlane::Render(const graphics::RenderContext& rc)
{
	auto& ctx = rc.GetContext();
	auto& api = rc.GetAPI();

	ctx.SetDepthState(graphics::Z_ENABLED, 1);
	ctx.SetBlendState(graphics::BLEND_FALSE);
	ctx.PSSetSamplerState(0, 1, graphics::MSAA_x16);
	ctx.SetRasterState(m_RenderWireframe ? graphics::WIREFRAME : graphics::CULL_NONE);

	UpdateConstantBuffer(rc);
	ctx.VSSetConstantBuffer(1, 1, &m_ConstantBuffer);
	ctx.DSSetConstantBuffer(1, 1, &m_ConstantBuffer);
	ctx.DrawIndexed(this, m_Effect);
}

void WaterPlane::ShadowRender(const graphics::RenderContext& /*rc*/)
{
	DL_ASSERT("water shadow?");
}

void WaterPlane::SetupRefractionRender(const graphics::RenderContext& rc)
{
	m_RefractionG.Clear(clearcolor::black, rc);
	rc.GetContext().ClearDepthStencilView(m_Refraction->GetDepthView(), graphics::DEPTH | graphics::STENCIL, 1);
	m_RefractionG.SetAsRenderTarget(m_Refraction, rc);
}

void WaterPlane::SetupReflectionRender(const graphics::RenderContext& rc)
{
	m_ReflectionG.Clear(clearcolor::black, rc);
	rc.GetContext().ClearDepthStencilView(m_Reflection->GetDepthView(), graphics::DEPTH | graphics::STENCIL, 1);
	m_ReflectionG.SetAsRenderTarget(m_Reflection, rc);
}

void WaterPlane::SetClipPlane(const CU::Vector4f& plane, const graphics::RenderContext& rc)
{
	m_PixelStruct.m_CompareValue = plane;
	rc.GetContext().UpdateConstantBuffer(m_cbPixel, &m_PixelStruct, sizeof(m_PixelStruct));
	rc.GetContext().PSSetConstantBuffer(1, 1, &m_cbPixel);
}
