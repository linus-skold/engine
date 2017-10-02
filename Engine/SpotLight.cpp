#include "stdafx.h"
#include "SpotLight.h"
#include "Instance.h"
#include "Camera.h"
#include <Engine/ShadowSpotlight.h>

#include <d3d11shader.h>


static bool s_Wireframe = false;

SpotLight::~SpotLight()
{
	m_Model = nullptr;
	SAFE_DELETE(m_ShadowSpotlight);
}

void SpotLight::Initiate()
{
	u64 key = Engine::GetInstance()->LoadModel<LightModel>("Data/Model/lightMeshes/cone.fbx"
																   , "Shaders/deferred_spotlight.json"
																   , false);
	m_Model = static_cast<LightModel*>(Engine::GetInstance()->GetModel(key));
	m_Model->Initiate("cone.fbx");

	// 	m_ShadowSpotlight = new ShadowSpotlight;
	// 	m_ShadowSpotlight->Initiate(2048.f);
	// 	m_ShadowSpotlight->GetCamera()->RotateAroundX(cl::DegreeToRad(90.f));

	// 	Effect* effect = Engine::GetInstance()->GetEffect("Shaders/lightvolume_spot.json");
	// 	CompiledShader* shader = effect->GetVertexShader();
	// 
	// 	D3D11_INPUT_ELEMENT_DESC layout[] =
	// 	{
	// 		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	// 		{ "ANGLE", 0, DXGI_FORMAT_R32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	// 		{ "RANGE", 0, DXGI_FORMAT_R32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	// 		{ "DIRECTION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	// 	};
	// 
	// 	m_InputLayout = Engine::GetAPI()->CreateInputLayout(shader->compiledShader, shader->shaderSize, layout, ARRAYSIZE(layout));
	// 
	// 	m_VertexBuffer.myStride = sizeof(spotlight);
	// 	m_VertexBuffer.myStartSlot = 0;
	// 	m_VertexBuffer.myNrOfBuffers = 1;
	// 	m_VertexBuffer.myByteOffset = 0;
	// 
	// 	m_VertexData.myNrOfVertexes = 1;
	// 	m_VertexData.myStride = sizeof(spotlight);
	// 	m_VertexData.mySize = m_VertexData.myNrOfVertexes * m_VertexData.myStride;
	// 	m_VertexData.myVertexData = new s8[m_VertexData.mySize];
	// 	m_VertexBuffer.myVertexBuffer = Engine::GetAPI()->CreateVertexBuffer(m_VertexData.mySize, m_VertexData.myVertexData);
	// 
	// 	//Engine::GetInstance()->AddCheckBox(&s_Wireframe, "Wireframe Spotlight");
	// 	m_gsCBuffer = Engine::GetAPI()->GetDevice().CreateConstantBuffer(sizeof(gsbuffer));
	// 	m_psCBuffer = Engine::GetAPI()->GetDevice().CreateConstantBuffer(sizeof(psbuffer));

}
/**
void SpotLight::Render(const CU::Matrix44f& previousOrientation, Camera* aCamera)
{
	render_context.m_API->SetBlendState(eBlendStates::LIGHT_BLEND);
	render_context.m_API->SetDepthStencilState(eDepthStencilState::READ_NO_WRITE,1);
	m_Model->Render(previousOrientation, aCamera->GetPerspective(), render_context);

	render_context.m_API->UpdateConstantBuffer(m_VertexBuffer.myVertexBuffer, &m_Data, sizeof(spotlight));

	m_gsBuffer.view_projection = CU::Math::Inverse(previousOrientation) * aCamera->GetPerspective();

	m_psBuffer.color = myData.myLightColor;
	m_psBuffer.eye_pos = aCamera->GetOrientation().GetPosition();
	m_psBuffer.light_pos = myData.myOrientation.GetPosition();

	ID3D11DeviceContext* context = render_context.m_Context;
	context->IASetInputLayout(m_InputLayout);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	context->IASetVertexBuffers(	m_VertexBuffer.myStartSlot
								, m_VertexBuffer.myNrOfBuffers
								, &m_VertexBuffer.myVertexBuffer
								, &m_VertexBuffer.myStride
								, &m_VertexBuffer.myByteOffset);

	render_context.m_API->UpdateConstantBuffer(m_gsCBuffer, &m_gsBuffer);
	render_context.m_API->UpdateConstantBuffer(m_psCBuffer, &m_psBuffer);

	Effect* effect = Engine::GetInstance()->GetEffect("Shaders/lightvolume_spot.json");

	render_context.m_Context->GSSetConstantBuffers(0, 1, &m_gsCBuffer);
	render_context.m_Context->PSSetConstantBuffers(0, 1, &m_psCBuffer);

	render_context.m_API->SetDepthStencilState(eDepthStencilState::READ_NO_WRITE_PARTICLE, 1);
	render_context.m_API->SetBlendState(eBlendStates::ALPHA_BLEND);
	render_context.m_API->SetRasterizer( s_Wireframe ? eRasterizer::WIREFRAME : eRasterizer::CULL_NONE);
	effect->Use();
	context->Draw(1, 0);
	effect->Clear();

}
*/
void SpotLight::Render(const CU::Matrix44f& camera_view, const CU::Matrix44f& camera_projection, const graphics::RenderContext& render_context)
{
	m_Model->Render(camera_view, camera_projection, render_context);
}

void SpotLight::SetData(const SpotlightData& data)
{
	myData = data;
	m_Data.m_Range = myData.myRange;
	m_Data.m_Angle = myData.myAngle;
	SetDirection(myData.myOrientation.GetForward());
	SetPosition(myData.myLightPosition);
	//m_ShadowSpotlight->SetAngle(myData.myAngle);
	const float buffer_size = m_ShadowSpotlight->GetBufferSize();
	//m_ShadowSpotlight->GetCamera()->RecalculatePerspective(buffer_size, buffer_size, 0.1f, myData.myRange);
}

const SpotlightData& SpotLight::GetData() const
{
	return myData;
}

void SpotLight::SetPosition(const CU::Vector3f& aPosition)
{
	m_Model->GetOrientation().SetPosition(aPosition);
	myData.myLightPosition = aPosition;
	m_Data.m_Position = aPosition;
	myData.myOrientation.SetPosition(aPosition);
	m_ShadowSpotlight->GetCamera()->SetPosition(aPosition);
}

void SpotLight::SetDirection(const CU::Vector4f& aDirection)
{
	myData.myDirection.x = aDirection.x;
	myData.myDirection.y = aDirection.y;
	myData.myDirection.z = aDirection.z;
	m_Model->GetOrientation().SetForward(aDirection);
	m_Data.m_Direction = myData.myDirection;
	//m_ShadowSpotlight->GetCamera()->SetAt(aDirection);
}