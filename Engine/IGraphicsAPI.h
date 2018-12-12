#pragma once

#include <Engine/engine_shared.h>
#include <Engine/graphics_flags.h>

#ifndef _WINDEF_
struct HINSTANCE__;
typedef HINSTANCE__* HINSTANCE;
struct HWND__;
typedef HWND__* HWND;
#endif

class Texture;
namespace graphics
{
	struct CreateInfo
	{
		HWND m_HWND;
		HINSTANCE m_Instance;
		int32 m_WindowWidth = 0;
		int32 m_WindowHeight = 0;
		std::string m_APIName;
	};



	struct Texture2DDesc
	{
		uint32 m_Width = 0;
		uint32 m_Height = 0;
		uint32 m_MipLevels = 0;
		eTextureFormat m_Format = RGBA32_FLOAT;
		int32 m_Usage = DEFAULT_USAGE;
		int32 m_Binding = NONE;
		int32 m_CPUAccessFlag = 0;
		int32 m_MiscFlags = 0;
		int32 m_ArraySize = 0;
		int32 m_SampleCount = 0;
		int32 m_SampleQuality = 0;
	};

	struct InputElementDesc
	{
		InputElementDesc() = default;

		InputElementDesc(std::string semantic, uint32 index, eVertexFormat format, uint32 slot, uint32 byte_offset, eElementSpecification element_spec, uint32 instance_step_rate)
			: m_Semantic(semantic)
			, m_SemanicIndex(index)
			, m_Format(format)
			, m_InputSlot(slot)
			, m_ByteOffset(byte_offset)
			, m_ElementSpecification(element_spec)
			, m_InstanceDataStepRate(instance_step_rate)
		{
		}

		std::string m_Semantic;
		uint32 m_SemanicIndex = 0;
		eVertexFormat m_Format;
		uint32 m_InputSlot = 0;
		uint32 m_ByteOffset = 0;
		eElementSpecification m_ElementSpecification;
		uint32 m_InstanceDataStepRate = 0;
	};

	struct SInputLayout
	{

		uint32 GetByteSizeOfFormat(eVertexFormat format)
		{
			switch (format)
			{
			case _4BYTE_R_UINT:
			case _4BYTE_R_FLOAT:
				return 4;
			case _8BYTE_RG:
				return 8;
			case _12BYTE_RGB:
				return 12;
			case _16BYTE_RGBA:
				return 16;
			}

			return 0;
		}

		void AddElement(const char* semantic, eVertexFormat format, uint32 slot, bool instanced = false, uint32 semantic_index = 0)
		{
			eElementSpecification spec = instanced ? INPUT_PER_INSTANCE_DATA : INPUT_PER_VERTEX_DATA;
			uint32 instance_step_rate = instanced ? 1 : 0;

			m_Elements.Add(InputElementDesc(semantic, semantic_index, format, slot, m_Offset, spec, instance_step_rate));

			m_Offset += GetByteSizeOfFormat(format);
		}

		void AddElement(const InputElementDesc& obj)
		{
			m_Elements.Add(obj);
		}

		uint32 m_Offset = 0;
		CU::GrowingArray<InputElementDesc> m_Elements;

	};

	struct BufferDesc
	{
		eBindFlag m_BindFlag;
		int32 m_CPUAccessFlag;
		eUsage m_UsageFlag;
		int8* m_Data = nullptr;
		int32 m_Size = 0;
		int32 m_StructuredByteStride = 0;
		int32 m_MiscFlags = 0;
		int32 m_ByteWidth = 0;
	};

	class IGraphicsDevice;
	class IGraphicsContext;
	class Viewport;
	class IGraphicsAPI
	{
	public:
		virtual ~IGraphicsAPI() {}
		virtual void Initiate() = 0;

		virtual void EndFrame() = 0;
		virtual void BeginFrame() = 0;


		virtual void OnAltEnter() = 0;
		virtual void OnResize() = 0;


		std::string GetAPIName() { return m_CreateInfo.m_APIName; }
		//virtual void CopyResource(void * pDestination, void * pSource) = 0;

		eGraphicsAPI GetActiveAPI() const { return m_ActiveAPI; }

		virtual Viewport* CreateViewport(uint16 width, uint16 height, float min_depth, float max_depth, uint16 top_left_x, uint16 top_left_y) = 0;

		virtual IGraphicsDevice& GetDevice() { return *m_Device; }
		virtual IGraphicsContext& GetContext() { return *m_Context; }

		virtual void ReleasePtr(void* ptr) = 0;

		const CreateInfo& GetInfo() const { return m_CreateInfo; }

		virtual void SetDefaultTargets() = 0;

		virtual void ResetViewport() = 0;

		IRenderTargetView* GetBackbuffer() { return m_DefaultRenderTarget; }
		const IRenderTargetView* GetBackbuffer() const { return m_DefaultRenderTarget; }
		IRenderTargetView** GetBackbufferRef() { return &m_DefaultRenderTarget; }

		IDepthStencilView* GetDepthView() { return m_DefaultDepthView; }
		const IDepthStencilView* GetDepthView() const { return m_DefaultDepthView; }


		virtual cl::Color PickColor(Texture* pTexture) = 0;


	protected:
		CreateInfo m_CreateInfo;
		eGraphicsAPI m_ActiveAPI;

		Viewport* m_Viewport						= nullptr;
		IGraphicsDevice* m_Device					= nullptr;
		IGraphicsContext* m_Context					= nullptr;

		ITexture2D* m_DefaultDepthBuffer			= nullptr;
		IDepthStencilView* m_DefaultDepthView		= nullptr;
		IRenderTargetView* m_DefaultRenderTarget	= nullptr;



		


	};
};