#include "stdafx.h"
#include "Model.h"

#include "Camera.h"
#include "DirectX11.h"
#include <DL_Debug.h>
#include "Engine.h"
#include "Surface.h"

#include <Engine/RenderContext.h>
#include <Engine/IGraphicsContext.h>
#ifdef _PROFILE
#include "profile_defines.h"
#endif


Model::~Model()
{
	if (!m_Surfaces.Empty())
		m_Surfaces[0]->serialize(m_FileName.c_str());

	Engine::GetAPI()->ReleasePtr(m_ConstantBuffer);

	m_Surfaces.DeleteAll();
	m_Children.DeleteAll();

}

void Model::Initiate(const std::string& filename)
{
	
	m_GPUData.Init(250);
	std::string dbg(filename.c_str());
	m_FileName = dbg;
	m_ConstantBuffer = Engine::GetAPI()->GetDevice().CreateConstantBuffer(sizeof(m_ObjectData), dbg + "Vertex ConstantBuffer");
	for (Model* child : m_Children)
	{
		child->SetIsInstanced(m_IsInstanced);
		child->Initiate(filename);
	}
}

void Model::Render(const graphics::RenderContext& rc)
{
	if (m_IsInstanced)
	{
		RenderInstanced(rc, m_Effect);
		return;
	}

	if (m_FileName.find("default_cube") != m_FileName.npos)
	{
		RenderCube(rc);
		return;
	}

	PROFILE_FUNCTION(profiler::colors::Blue);
	for (Model* child : m_Children)
	{
		child->Render(rc);
	}

	if (m_IsRoot || (!m_IndexWrapper.GetIndexBuffer() && (!m_VertexWrapper.GetVertexBuffer())))
		return;


	UpdateConstantBuffer(rc);
	rc.GetContext().PSSetSamplerState(0, 1, graphics::MSAA_x16);
	rc.GetContext().VSSetSamplerState(0, 1, graphics::MSAA_x16);

	if(!m_Surfaces.Empty())
		m_Surfaces[0]->Activate(rc);
	PROFILE_BLOCK("Model : DrawIndexed", profiler::colors::Blue100);
	rc.GetContext().DrawIndexed(this);
	PROFILE_BLOCK_END;

}

void Model::RenderCube(const graphics::RenderContext& rc)
{
	UpdateConstantBuffer(rc);
	rc.GetContext().PSSetSamplerState(0, 1, rc.GetEngine().GetCurrentSampler());
	rc.GetContext().DrawIndexed(this, m_Effect);
}

void Model::RenderInstanced(const graphics::RenderContext& rc)
{
	RenderInstanced(rc, m_Effect);
}

void Model::RenderInstanced(const graphics::RenderContext& rc, Effect* override_effect)
{
	PROFILE_FUNCTION(profiler::colors::Amber);

	for (Model* child : m_Children)
	{
		child->RenderInstanced(rc, override_effect);
	}

	if (!m_IndexWrapper.GetIndexBuffer() && (!m_VertexWrapper.GetVertexBuffer() || !m_InstanceWrapper.GetInstanceBuffer()))
	{
		RemoveGPUData();
		return;
	}

	if (m_IsRoot || m_GPUData.Empty())
	{
		RemoveGPUData();
		return;
	}


	UpdateConstantBuffer(rc);


	PROFILE_BLOCK("Model : DrawIndexedInstanced", profiler::colors::Amber100);


	rc.GetContext().DrawIndexedInstanced(this, override_effect);
	PROFILE_BLOCK_END;

	RemoveGPUData();
}

void Model::ShadowRender(const graphics::RenderContext& rc)
{
	PROFILE_FUNCTION(profiler::colors::Amber100);
	for (Model* child : m_Children)
		child->ShadowRender(rc);

	if (m_IsRoot)
		return;

	UpdateConstantBuffer(rc);
	rc.GetContext().PSSetSamplerState(0, 1, rc.GetEngine().GetCurrentSampler());
	rc.GetContext().DrawIndexed(this);
}

void Model::ShadowRenderInstanced(const graphics::RenderContext& rc)
{
	PROFILE_FUNCTION(profiler::colors::Amber100);
	for (Model* child : m_Children)
		child->ShadowRenderInstanced(rc);

	if (!m_IndexWrapper.GetIndexBuffer() && (!m_VertexWrapper.GetVertexBuffer() || !m_InstanceWrapper.GetInstanceBuffer()))
	{
		RemoveGPUData();
		return;
	}


	if (m_IsRoot || m_GPUData.Empty())
	{
		RemoveGPUData();
		return;
	}

	UpdateConstantBuffer(rc);
	rc.GetContext().PSSetSamplerState(0, 1, rc.GetEngine().GetCurrentSampler());

	rc.GetContext().DrawIndexedInstanced(this);

	RemoveGPUData();
}

void Model::SetPosition(const CU::Vector4f& aPosition)
{
	m_Orientation.SetPosition(aPosition);
	for each (Model* child in m_Children)
	{
		child->SetPosition(aPosition);
	}
}

void Model::SetForward(const CU::Vector4f& d)
{
	m_Orientation.SetForward(d);
}

const CU::Matrix44f& Model::GetOrientation() const
{
	return m_Orientation;
}

void Model::SetOrientation(CU::Matrix44f orientation)
{
	m_Orientation = orientation;
	for (Model* child : m_Children)
	{
		child->SetOrientation(m_Orientation);
	}
}

void Model::SetMaxPoint(CU::Vector3f max_point)
{
	m_MaxPoint = max_point;
}

void Model::SetMinPoint(CU::Vector3f min_point)
{
	m_MinPoint = min_point;
}

void Model::AddTexture(const std::string& path, Effect::TextureSlot slot)
{
	for (Model* pChild : m_Children)
	{
		pChild->AddTexture(path, slot);
	}

	for (Surface* pSurface : m_Surfaces)
	{
		pSurface->AddTexture(path, slot);
	}
}

void Model::UpdateConstantBuffer(const graphics::RenderContext& rc)
{
	if (m_IsRoot)
		return;

	graphics::IGraphicsContext& ctx = rc.GetContext();
	if (m_InstanceWrapper.GetInstanceBuffer())
	{
		if (m_GPUData.Empty())
			return;

		IBuffer* pBuffer = m_InstanceWrapper.GetInstanceBuffer();
		ctx.UpdateConstantBuffer(pBuffer, &m_GPUData[0], m_GPUData.Size() * sizeof(GPUModelData));
	}
	else
	{

		m_ObjectData.orientation = m_Orientation;
		ctx.UpdateConstantBuffer(m_ConstantBuffer, &m_ObjectData, sizeof(m_ObjectData));
		ctx.VSSetConstantBuffer(1, 1, &m_ConstantBuffer);
	}
}

void Model::AddChild(Model* aChild)
{
	m_Children.Add(aChild);
}

 void Model::AddSurface(Surface* surface)
 {
 	for (Model* child : m_Children)
 	{
 		child->AddSurface(surface);
 	}
 	m_Surfaces.Add(surface);
 }

void Model::AddOrientation(CU::Matrix44f orientation)
{
	GPUModelData data;
	data.m_Orientation = orientation;
	AddInstanceData(data);
}

void Model::AddInstanceData(GPUModelData data)
{
	for (Model* c : m_Children)
	{
		c->AddInstanceData(data);
	}

	m_GPUData.Add(data);

}

void Model::SetMaterial(Material* pMaterial)
{
	if (m_Surfaces.Size() > 0 && m_Surfaces[0])
	{
		m_Surfaces[0]->SetMaterial(*pMaterial);
		for (Model* c : m_Children)
		{
			c->SetMaterial(pMaterial);
		}
	}
}

Material* Model::GetMaterial()
{
	return nullptr;
}

void Model::RemoveGPUData()
{
	for (Model* child : m_Children)
	{
		child->RemoveGPUData();
	}
	m_GPUData.RemoveAll();
}



void Model::CreateCube()
{
	m_Effect = Engine::GetInstance()->GetEffect("Shaders/default.json");

	CU::GrowingArray<SDefaultCube> vertices;

#pragma region Vertex
	SDefaultCube temp;
	temp.m_Position = { -1.0f, 1.0f, -1.0f, 1.f };
	temp.m_Normal = { 0.0f, 1.0f, 0.0f };
	temp.m_UV = { 0.0f, 0.0f };
	vertices.Add(temp);

	temp.m_Position = { 1.0f, 1.0f, -1.0f , 1.f };
	temp.m_Normal = { 0.0f, 1.0f, 0.0f };
	temp.m_UV = { 1.0f, 0.0f };
	vertices.Add(temp);

	temp.m_Position = { 1.0f, 1.0f, 1.0f, 1.f };
	temp.m_Normal = { 0.0f, 1.0f, 0.0f };
	temp.m_UV = { 1.0f, 1.0f };

	vertices.Add(temp);

	temp.m_Position = { -1.0f, 1.0f, 1.0f, 1.f };
	temp.m_Normal = { 0.0f, 1.0f, 0.0f };
	temp.m_UV = { 0.0f, 1.0f };

	vertices.Add(temp);

	temp.m_Position = { -1.0f, -1.0f, -1.0f, 1.f };
	temp.m_Normal = { 0.0f, -1.0f, 0.0f };
	temp.m_UV = { 0.0f, 0.0f };

	vertices.Add(temp);

	temp.m_Position = { 1.0f, -1.0f, -1.0f, 1.f };
	temp.m_Normal = { 0.0f, -1.0f, 0.0f };
	temp.m_UV = { 1.0f, 0.0f };
	vertices.Add(temp);

	temp.m_Position = { 1.0f, -1.0f, 1.0f, 1.f };
	temp.m_Normal = { 0.0f, -1.0f, 0.0f };
	temp.m_UV = { 1.0f, 1.0f };

	vertices.Add(temp);

	temp.m_Position = { -1.0f, -1.0f, 1.0f, 1.f };
	temp.m_Normal = { 0.0f, -1.0f, 0.0f };
	temp.m_UV = { 0.0f, 1.0f };

	vertices.Add(temp);

	temp.m_Position = { -1.0f, -1.0f, 1.0f, 1.f };
	temp.m_Normal = { -1.0f, 0.0f, 0.0f };
	temp.m_UV = { 0.0f, 0.0f };

	vertices.Add(temp);

	temp.m_Position = { -1.0f, -1.0f, -1.0f, 1.f };
	temp.m_Normal = { -1.0f, 0.0f, 0.0f };
	temp.m_UV = { 1.0f, 0.0f };

	vertices.Add(temp);

	temp.m_Position = { -1.0f, 1.0f, -1.0f, 1.f };
	temp.m_Normal = { -1.0f, 0.0f, 0.0f };
	temp.m_UV = { 1.0f, 1.0f };

	vertices.Add(temp);

	temp.m_Position = { -1.0f, 1.0f, 1.0f , 1.f };
	temp.m_Normal = { -1.0f, 0.0f, 0.0f };
	temp.m_UV = { 0.0f, 1.0f };

	vertices.Add(temp);


	temp.m_Position = { 1.0f, -1.0f, 1.0f, 1.f };
	temp.m_Normal = { 1.0f, 0.0f, 0.0f };
	temp.m_UV = { 0.0f, 0.0f };

	vertices.Add(temp);

	temp.m_Position = { 1.0f, -1.0f, -1.0f, 1.f };
	temp.m_Normal = { 1.0f, 0.0f, 0.0f };
	temp.m_UV = { 1.0f, 0.0f };

	vertices.Add(temp);

	temp.m_Position = { 1.0f, 1.0f, -1.0f , 1.f };
	temp.m_Normal = { 1.0f, 0.0f, 0.0f };
	temp.m_UV = { 1.0f, 1.0f };
	vertices.Add(temp);

	temp.m_Position = { 1.0f, 1.0f, 1.0f, 1.f };
	temp.m_Normal = { 1.0f, 0.0f, 0.0f };
	temp.m_UV = { 0.0f, 1.0f };

	vertices.Add(temp);

	temp.m_Position = { -1.0f, -1.0f, -1.0f , 1.f };
	temp.m_Normal = { 0.0f, 0.0f, -1.0f };
	temp.m_UV = { 0.0f, 0.0f };

	vertices.Add(temp);

	temp.m_Position = { 1.0f, -1.0f, -1.0f , 1.f };
	temp.m_Normal = { 0.0f, 0.0f, -1.0f };
	temp.m_UV = { 1.0f, 0.0f };
	vertices.Add(temp);

	temp.m_Position = { 1.0f, 1.0f, -1.0f, 1.f };
	temp.m_Normal = { 0.0f, 0.0f, -1.0f };
	temp.m_UV = { 1.0f, 1.0f };

	vertices.Add(temp);

	temp.m_Position = { -1.0f, 1.0f, -1.0f , 1.f };
	temp.m_Normal = { 0.0f, 0.0f, -1.0f };
	temp.m_UV = { 0.0f, 1.0f };

	vertices.Add(temp);


	temp.m_Position = { -1.0f, -1.0f, 1.0f , 1.f };
	temp.m_Normal = { 0.0f, 0.0f, 1.0f };
	temp.m_UV = { 0.0f, 0.0f };

	vertices.Add(temp);

	temp.m_Position = { 1.0f, -1.0f, 1.0f , 1.f };
	temp.m_Normal = { 0.0f, 0.0f, 1.0f };
	temp.m_UV = { 1.0f, 0.0f };

	vertices.Add(temp);

	temp.m_Position = { 1.0f, 1.0f, 1.0f , 1.f };
	temp.m_Normal = { 0.0f, 0.0f, 1.0f };
	temp.m_UV = { 1.0f, 1.0f };

	vertices.Add(temp);

	temp.m_Position = { -1.0f, 1.0f, 1.0f , 1.f};
	temp.m_Normal = { 0.0f, 0.0f, 1.0f };
	temp.m_UV = { 0.0f, 1.0f };

	vertices.Add(temp);

#pragma endregion

	m_VertexWrapper.SetVertexCount(vertices.Size());
	m_VertexWrapper.SetStride(sizeof(SDefaultCube));
	m_VertexWrapper.SetSize(vertices.Size() * sizeof(SDefaultCube));

	s8* data = new s8[m_VertexWrapper.GetSize()];
	memcpy(data, &vertices[0], m_VertexWrapper.GetSize());

	m_VertexWrapper.SetData(data);

	graphics::BufferDesc vtx_desc;
	vtx_desc.m_Size = m_VertexWrapper.GetSize();
	vtx_desc.m_Data = data;
	vtx_desc.m_BindFlag = graphics::BIND_VERTEX_BUFFER;
	vtx_desc.m_UsageFlag = graphics::DYNAMIC_USAGE;
	vtx_desc.m_StructuredByteStride = 0;
	vtx_desc.m_CPUAccessFlag = graphics::WRITE;
	vtx_desc.m_MiscFlags = 0;
	vtx_desc.m_StructuredByteStride = 0;
	vtx_desc.m_ByteWidth = vtx_desc.m_Size;
	IBuffer* buffer = Engine::GetAPI()->GetDevice().CreateBuffer(vtx_desc, "default cube VertexBuffer");
	m_VertexWrapper.SetBuffer(buffer);

	graphics::InputElementDesc elements[] = {
		{ "POSITION", 0, graphics::_16BYTE_RGBA, 0, 0, graphics::INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, graphics::_16BYTE_RGBA, 0, 16, graphics::INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, graphics::_8BYTE_RG, 0, 32, graphics::INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, graphics::_16BYTE_RGBA, 0, 40, graphics::INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, graphics::_16BYTE_RGBA, 0, 56, graphics::INPUT_PER_VERTEX_DATA, 0 },
	};

	IInputLayout* pLayout = Engine::GetAPI()->GetDevice().CreateInputLayout(m_Effect->GetVertexShader(), &elements[0], ARRSIZE(elements));
	m_VertexWrapper.SetInputLayout(pLayout);

	m_VertexWrapper.SetStart(0);
	m_VertexWrapper.SetByteOffset(0);

	m_VertexWrapper.SetTopology(graphics::TRIANGLE_LIST);
	m_IsRoot = false;

	CU::GrowingArray<s32> indices;
	indices.Init(32);
#pragma region Indices

	indices.Add(3);
	indices.Add(1);
	indices.Add(0);

	indices.Add(2);
	indices.Add(1);
	indices.Add(3);

	indices.Add(6);
	indices.Add(4);
	indices.Add(5);

	indices.Add(6);
	indices.Add(7);
	indices.Add(4);

	indices.Add(11);
	indices.Add(9);
	indices.Add(8);

	indices.Add(10);
	indices.Add(9);
	indices.Add(11);

	indices.Add(14);
	indices.Add(12);
	indices.Add(13);

	indices.Add(15);
	indices.Add(12);
	indices.Add(14);

	indices.Add(19);
	indices.Add(17);
	indices.Add(16);

	indices.Add(18);
	indices.Add(17);
	indices.Add(19);

	indices.Add(22);
	indices.Add(20);
	indices.Add(21);

	indices.Add(23);
	indices.Add(20);
	indices.Add(22);

#pragma endregion


	auto& idx = m_IndexWrapper;
	const s32 idx_buf_size = indices.Size() * sizeof(s32);

	s8* indexData = new s8[idx_buf_size];
	memcpy(indexData, &indices[0], idx_buf_size);

	const graphics::eTextureFormat idx_IndexBufferFormat = graphics::R32_UINT;
	const s32 idx_IndexCount = indices.Size();

	graphics::BufferDesc idx_desc;
	idx_desc.m_Size = idx_buf_size;
	idx_desc.m_Data = indexData;
	idx_desc.m_BindFlag = graphics::BIND_INDEX_BUFFER;
	idx_desc.m_UsageFlag = graphics::IMMUTABLE_USAGE;
	idx_desc.m_StructuredByteStride = 0;
	idx_desc.m_CPUAccessFlag = graphics::NO_ACCESS_FLAG;
	idx_desc.m_MiscFlags = 0;
	idx_desc.m_ByteWidth = idx_desc.m_Size;

	IBuffer* pIdxBuffer = Engine::GetAPI()->GetDevice().CreateBuffer(idx_desc, "default_cube IndexBuffer");

	idx.SetData(indexData);
	idx.SetIndexCount(idx_IndexCount);
	idx.SetStart(0);
	idx.SetSize(idx_buf_size);
	idx.SetFormat(idx_IndexBufferFormat);
	idx.SetByteOffset(0);
	idx.SetBuffer(pIdxBuffer);



	Initiate("default_cube");

}

void Model::SetEntityID(int id)
{
	for (Model* pModel : m_Children)
	{
		pModel->SetEntityID(id);
	}
	m_ObjectData.entityID = id;
}

void Model::SetIsInstanced(bool is_instanced)
{
	m_IsInstanced = is_instanced;
	for (Model* m : m_Children)
	{
		m->SetIsInstanced(m_IsInstanced);
	}
}

void Model::SetSurface0(Surface* surface)
{
	if (m_Surfaces.Empty())
		return;
	
	for (Model* c : m_Children)
	{
		c->SetSurface0(surface);
	}
	m_Surfaces[0] = surface;

}
