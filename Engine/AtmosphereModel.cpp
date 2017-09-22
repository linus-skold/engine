#include "stdafx.h"
#include "AtmosphereModel.h"
#include <Engine/IGraphicsContext.h>


AtmosphereModel::~AtmosphereModel()
{
	m_Surfaces.DeleteAll();
	m_Children.DeleteAll();
	Engine::GetAPI()->ReleasePtr(m_ConstantBuffer);
}

void AtmosphereModel::Initiate(const std::string& filename)
{
	//m_Filename = cl::substr(filename, "/", false, 0);
	if ( m_IsRoot == false )
	{
		InitConstantBuffer();
	}

	for ( AtmosphereModel* child : m_Children )
	{
		child->Initiate(filename);
	}
}

void AtmosphereModel::Render(const CU::Matrix44f& camera_orientation, const CU::Matrix44f& camera_projection, const graphics::RenderContext& render_context)
{
	for (AtmosphereModel* child : m_Children)
	{
		child->Render(camera_orientation, camera_projection, render_context);
	}

	if (m_IsRoot || m_Surfaces.Empty())
		return;

	UpdateConstantBuffer(camera_orientation, camera_projection, render_context);
	auto& ctx = render_context.GetContext();
	ctx.VSSetConstantBuffer(0, 1, m_ConstantBuffer);
	ctx.PSSetSamplerState(0, 1, render_context.GetAPI().GetSamplerState(graphics::LINEAR_CLAMP));

	ctx.DrawIndexed(this, m_Effect);
}

void AtmosphereModel::AddChild(AtmosphereModel* child)
{
	m_Children.Add(child);
}

void AtmosphereModel::SetOrientation(const CU::Matrix44f& orientation)
{
	m_Orientations[0] = orientation;
	for ( AtmosphereModel* child : m_Children )
	{
		child->SetOrientation(m_Orientations[0]);
	}
}
