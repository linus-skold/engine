#pragma once

class CompiledShader;
class Model;
namespace graphics
{
	class IGraphicsContext
	{
	public:
		virtual void* GetContext() = 0;

		virtual void VSSetShaderResource(s32 start_slot, s32 count, void* resources) = 0;
		virtual void PSSetShaderResource(s32 start_slot, s32 count, void* resources) = 0;
		virtual void GSSetShaderResource(s32 start_slot, s32 count, void* resources) = 0;
		virtual void DSSetShaderResource(s32 start_slot, s32 count, void* resources) = 0;
		virtual void HSSetShaderResource(s32 start_slot, s32 count, void* resources) = 0;
		virtual void CSSetShaderResource(s32 start_slot, s32 count, void* resources) = 0;

		virtual void SetVertexShader(CompiledShader* vertex_shader) = 0;
		virtual void SetPixelShader(CompiledShader* vertex_shader) = 0;
		virtual void SetGeometryShader(CompiledShader* vertex_shader) = 0;
		virtual void SetHullShader(CompiledShader* vertex_shader) = 0;
		virtual void SetDomainShader(CompiledShader* vertex_shader) = 0;
		virtual void SetComputeShader(CompiledShader* vertex_shader) = 0;

		virtual void VSSetConstantBuffer(s32 start_index, s32 buffer_count, IBuffer* pBuffer) = 0;
		virtual void PSSetConstantBuffer(s32 start_index, s32 buffer_count, IBuffer* pBuffer) = 0;
		virtual void GSSetConstantBuffer(s32 start_index, s32 buffer_count, IBuffer* pBuffer) = 0;
		virtual void HSSetConstantBuffer(s32 start_index, s32 buffer_count, IBuffer* pBuffer) = 0;
		virtual void DSSetConstantBuffer(s32 start_index, s32 buffer_count, IBuffer* pBuffer) = 0;
		virtual void CSSetConstantBuffer(s32 start_index, s32 buffer_count, IBuffer* pBuffer) = 0;


		virtual void VSSetSamplerState(s32 start_index, s32 sampler_count, ISamplerState* pSamplers) = 0;
		virtual void PSSetSamplerState(s32 start_index, s32 sampler_count, ISamplerState* pSamplers) = 0;
		virtual void GSSetSamplerState(s32 start_index, s32 sampler_count, ISamplerState* pSamplers) = 0;
		virtual void HSSetSamplerState(s32 start_index, s32 sampler_count, ISamplerState* pSamplers) = 0;
		virtual void DSSetSamplerState(s32 start_index, s32 sampler_count, ISamplerState* pSamplers) = 0;
		virtual void CSSetSamplerState(s32 start_index, s32 sampler_count, ISamplerState* pSamplers) = 0;


		virtual void IASetInputLayout(IInputLayout* input_layout) = 0;
		virtual void IASetTopology(eTopology topology) = 0;
		virtual void IASetVertexBuffer() = 0;
		virtual void IASetIndexBuffer() = 0;

		virtual void Draw(s32 vertex_start, s32 vertex_count) = 0;
		virtual void Draw(s32 index_start, s32 index_count, s32 vertex_start) = 0;
		virtual void Draw(s32 index_count, s32 instance_count, s32 index_start, s32 vertex_start, s32 instance_start) = 0;
		virtual void Draw(Model* model) = 0;

	private:


	};
};
