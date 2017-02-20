#include "stdafx.h"
#include "BaseModel.h"

namespace Hex
{
	CBaseModel::CBaseModel()
	{
		myEngine = Engine::GetInstance();
		myAPI = Engine::GetAPI();
		myContext = myAPI->GetContext();
	}

	CBaseModel::~CBaseModel() = default;

	void CBaseModel::Render(const CU::Matrix44f& aCameraOrientation, const CU::Matrix44f& aCameraProjection, const CU::Vector4f& scale, bool render_shadows)
	{
#ifdef SNOWBLIND_DX11
		myContext->IASetInputLayout(m_VertexLayout);
		myContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		myContext->IASetVertexBuffers(0, 1, &m_VertexBuffer.myVertexBuffer, &m_VertexBuffer.myStride, &m_VertexBuffer.myByteOffset);
		myContext->IASetIndexBuffer(m_IndexBuffer.myIndexBuffer, DXGI_FORMAT_R32_UINT, m_IndexBuffer.myByteOffset);

		if (!render_shadows)
		{
			myAPI->SetVertexShader(myEffect->GetVertexShader() ? myEffect->GetVertexShader()->m_Shader : nullptr);
			myAPI->SetPixelShader(myEffect->GetPixelShader() ? myEffect->GetPixelShader()->m_Shader : nullptr);
		}
		UpdateConstantBuffer(aCameraOrientation, aCameraProjection, scale);
#endif
	}

	void CBaseModel::SetEffect(Effect* anEffect)
	{
		myEffect = anEffect;
	}

	void CBaseModel::InitVertexBuffer()
	{
		void* shader = myEffect->GetVertexShader()->compiledShader;
		s32 size = myEffect->GetVertexShader()->shaderSize;
		m_VertexLayout = myAPI->CreateInputLayout(shader, size, &myVertexFormat[0], myVertexFormat.Size());

#ifdef _DEBUG
		myAPI->SetDebugName(m_VertexLayout, "Model Vertex Layout : " + m_Filename);
#endif
		m_VertexBuffer.myVertexBuffer = myAPI->CreateVertexBuffer(m_VertexData.mySize, m_VertexData.myVertexData);
#ifdef _DEBUG
		myAPI->SetDebugName(m_VertexBuffer.myVertexBuffer, "Model Vertex Buffer " + m_Filename);
#endif

		m_VertexBuffer.myStride = m_VertexData.myStride;
		m_VertexBuffer.myByteOffset = 0;
		m_VertexBuffer.myStartSlot = 0;
		m_VertexBuffer.myNrOfBuffers = 1;

	}

	void CBaseModel::InitIndexBuffer()
	{
		D3D11_BUFFER_DESC indexDesc;
		ZeroMemory(&indexDesc, sizeof(indexDesc));
		indexDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexDesc.ByteWidth = m_IndexData.mySize;
		indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexDesc.CPUAccessFlags = 0;
		indexDesc.MiscFlags = 0;
		indexDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexData;
		ZeroMemory(&indexData, sizeof(indexData));
		indexData.pSysMem = static_cast<void*>(m_IndexData.myIndexData);

		HRESULT hr = myAPI->GetDevice()->CreateBuffer(&indexDesc, &indexData, &m_IndexBuffer.myIndexBuffer);
		myAPI->SetDebugName(m_IndexBuffer.myIndexBuffer, "Model Index Buffer " + m_Filename);
		myAPI->HandleErrors(hr, "[BaseModel] : Failed to Create IndexBuffer");

		m_IndexBuffer.myIndexBufferFormat = m_IndexData.myFormat;
		m_IndexBuffer.myByteOffset = 0;
	}
};