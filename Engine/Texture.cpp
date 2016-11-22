#include "stdafx.h"
#include "Texture.h"

#include <DDSTextureLoader.h>
#include <ScreenGrab.h>
namespace Snowblind
{

	void CTexture::Initiate(u16 width, u16 height, s32 flags, TextureFormat texture_format, const std::string& debug_name)
	{

#ifdef SNOWBLIND_DX11
		DirectX11* api = CEngine::GetAPI();
#else
		Vulkan* api = CEngine::GetAPI();
#endif
		ID3D11Device* device = CEngine::GetAPI()->GetDevice();

		D3D11_TEXTURE2D_DESC text_desc;
		text_desc.Width = (UINT)width;
		text_desc.Height = (UINT)height;
		text_desc.MipLevels = 1;
		text_desc.Format = texture_format;
		text_desc.SampleDesc.Count = 1;
		text_desc.SampleDesc.Quality = 0;
		text_desc.Usage = GetUsage(flags);
		text_desc.BindFlags = (flags & ~(DEFAULT_USAGE | IMMUTABLE_USAGE | DYNAMIC_USAGE | STAGING_USAGE));
		text_desc.CPUAccessFlags = 0;
		text_desc.MiscFlags = 0;
		text_desc.ArraySize = 1;

		ID3D11Texture2D* texture = nullptr;
		HRESULT hr = device->CreateTexture2D(&text_desc, NULL, &texture);
		api->HandleErrors(hr, "[Texture](Initiate) : Failed to initiate texture.");

		if (flags & D3D11_BIND_RENDER_TARGET)
		{
			hr = device->CreateRenderTargetView(texture, NULL, &m_RenderTargetView);
			api->HandleErrors(hr, "[Texture](Initiate) : Failed to create RenderTargetView.");
			api->SetDebugName(m_RenderTargetView, debug_name + "RenderTargetView");
		}

		if (flags & D3D11_BIND_SHADER_RESOURCE)
		{
			hr = device->CreateShaderResourceView(texture, NULL, &m_ShaderResource);
			api->HandleErrors(hr, "[Texture](Initiate) : Failed to create ShaderResourceView.");
			api->SetDebugName(m_ShaderResource, debug_name + "ShaderResourceView");
		}

		if (flags & D3D11_BIND_DEPTH_STENCIL)
		{
			DL_ASSERT("Invalid flag D3D11_BIND_DEPTH_STENCIL for creating regular texture!");
		}

		SAFE_RELEASE(texture);
	}

	void CTexture::Initiate(u16 width, u16 height, s32 flags, TextureFormat texture_format, TextureFormat shader_resource_view_format, TextureFormat depth_stencil_format, const std::string& debug_name)
	{
#ifdef SNOWBLIND_DX11
		DirectX11* api = CEngine::GetAPI();
#else
		Vulkan* api = CEngine::GetAPI();
#endif
		ID3D11Device* device = CEngine::GetAPI()->GetDevice();

		D3D11_TEXTURE2D_DESC text_desc;
		text_desc.Width = (UINT)width;
		text_desc.Height = (UINT)height;
		text_desc.MipLevels = 1;
		text_desc.Format = texture_format;
		text_desc.SampleDesc.Count = 1;
		text_desc.SampleDesc.Quality = 0;
		text_desc.Usage = GetUsage(flags);
		text_desc.BindFlags = (flags & ~(DEFAULT_USAGE | IMMUTABLE_USAGE | DYNAMIC_USAGE | STAGING_USAGE));
		text_desc.CPUAccessFlags = 0;
		text_desc.MiscFlags = 0;
		text_desc.ArraySize = 1;

		HRESULT hr = device->CreateTexture2D(&text_desc, NULL, &m_DepthTexture);
		api->HandleErrors(hr, "[Texture](Initiate) : Failed to initiate texture.");
		api->SetDebugName(m_DepthTexture, debug_name + "Texture");

		if (flags & D3D11_BIND_RENDER_TARGET)
		{
			DL_ASSERT("Invalid flag D3D11_BIND_RENDER_TARGET for creating DepthTexture.");
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC view_desc;
		ZeroMemory(&view_desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		view_desc.Format = shader_resource_view_format;
		view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		view_desc.Texture2D.MipLevels = 1;
		view_desc.Texture2D.MostDetailedMip = 0;

		if (flags & D3D11_BIND_SHADER_RESOURCE && !m_DepthStencilShaderView)
		{
			hr = device->CreateShaderResourceView(m_DepthTexture, &view_desc, &m_DepthStencilShaderView);
			api->HandleErrors(hr, "[Texture](Initiate) : Failed to create DepthStencil-ShaderResourceView.");
			api->SetDebugName(m_DepthStencilShaderView, debug_name + "DepthStencil-ShaderResourceView");
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC depth_desc;
		ZeroMemory(&depth_desc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		depth_desc.Format = depth_stencil_format;
		depth_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depth_desc.Texture2D.MipSlice = 0;

		if (flags & D3D11_BIND_DEPTH_STENCIL && !m_DepthStencilView)
		{
			hr = device->CreateDepthStencilView(m_DepthTexture, &depth_desc, &m_DepthStencilView);
			api->HandleErrors(hr, "[Texture](Initiate) : Failed to create Depthstencil-DepthStencilView.");
			api->SetDebugName(m_DepthStencilView, debug_name + "DepthStencilView");
		}

	}

	void CTexture::Initiate(u16 width, u16 height, s32 flags, TextureFormat render_target_format, TextureFormat texture_format, TextureFormat shader_resource_view_format, TextureFormat depth_stencil_format, const std::string& debug_name)
	{

#ifdef SNOWBLIND_DX11
		DirectX11* api = CEngine::GetAPI();
#else
		Vulkan* api = CEngine::GetAPI();
#endif
		ID3D11Device* device = CEngine::GetAPI()->GetDevice();

		D3D11_TEXTURE2D_DESC text_desc;
		text_desc.Width = (UINT)width;
		text_desc.Height = (UINT)height;
		text_desc.MipLevels = 1;
		text_desc.Format = texture_format;
		text_desc.SampleDesc.Count = 1;
		text_desc.SampleDesc.Quality = 0;
		text_desc.Usage = GetUsage(flags);
		text_desc.BindFlags = (flags & ~(DEFAULT_USAGE | IMMUTABLE_USAGE | DYNAMIC_USAGE | STAGING_USAGE | D3D11_BIND_RENDER_TARGET));;
		text_desc.CPUAccessFlags = 0;
		text_desc.MiscFlags = 0;
		text_desc.ArraySize = 1;

		HRESULT hr = device->CreateTexture2D(&text_desc, NULL, &m_DepthTexture);
		api->HandleErrors(hr, "[Texture](Initiate) : Failed to initiate texture.");
		api->SetDebugName(m_DepthTexture, debug_name + "Texture");

		if (flags & D3D11_BIND_RENDER_TARGET)
		{
			D3D11_TEXTURE2D_DESC rt_text_desc;
			rt_text_desc.Width = (UINT)width;
			rt_text_desc.Height = (UINT)height;
			rt_text_desc.MipLevels = 1;
			rt_text_desc.Format = render_target_format;
			rt_text_desc.SampleDesc.Count = 1;
			rt_text_desc.SampleDesc.Quality = 0;
			rt_text_desc.Usage = GetUsage(flags);
			rt_text_desc.BindFlags = (flags & ~(DEFAULT_USAGE | IMMUTABLE_USAGE | DYNAMIC_USAGE | STAGING_USAGE | D3D11_BIND_DEPTH_STENCIL));
			rt_text_desc.CPUAccessFlags = 0;
			rt_text_desc.MiscFlags = 0;
			rt_text_desc.ArraySize = 1;

			ID3D11Texture2D* texture = nullptr;
			hr = device->CreateTexture2D(&rt_text_desc, NULL, &texture);
			api->HandleErrors(hr, "[Texture](Initiate) : Failed to initiate texture.");

			hr = device->CreateRenderTargetView(texture, NULL, &m_RenderTargetView);
			api->HandleErrors(hr, "[Texture](Initiate) : Failed to create RenderTargetView.");
			api->SetDebugName(m_RenderTargetView, debug_name + "RenderTargetView");
			SAFE_RELEASE(texture);
		}

		if (flags & D3D11_BIND_SHADER_RESOURCE)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC view_desc;
			ZeroMemory(&view_desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
			view_desc.Format = shader_resource_view_format;
			view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			view_desc.Texture2D.MipLevels = 1;
			view_desc.Texture2D.MostDetailedMip = 0;

			hr = device->CreateShaderResourceView(m_DepthTexture, &view_desc, &m_DepthStencilShaderView);
			api->HandleErrors(hr, "[Texture](Initiate) : Failed to create DepthStencil-ShaderResourceView.");
			api->SetDebugName(m_DepthStencilShaderView, debug_name + "DepthStencil-ShaderResourceView");
		}
		
		if (flags & D3D11_BIND_DEPTH_STENCIL)
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC depth_desc;
			ZeroMemory(&depth_desc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
			depth_desc.Format = depth_stencil_format;
			depth_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			depth_desc.Texture2D.MipSlice = 0;

			hr = device->CreateDepthStencilView(m_DepthTexture, &depth_desc, &m_DepthStencilView);
			api->HandleErrors(hr, "[Texture](Initiate) : Failed to create Depthstencil-DepthStencilView.");
			api->SetDebugName(m_DepthStencilView, debug_name + "DepthStencilView");
		}

	}

	bool CTexture::CleanUp()
	{
		SAFE_RELEASE(m_ShaderResource);
		if (m_ShaderResource)
			return false;
		SAFE_RELEASE(m_DepthTexture);
		if (m_DepthTexture)
			return false;
		SAFE_RELEASE(m_DepthStencilView);
		if (m_DepthStencilView)
			return false;
		SAFE_RELEASE(m_DepthStencilShaderView);
		if (m_DepthStencilShaderView)
			return false;
		SAFE_RELEASE(m_RenderTargetView);
		if (m_RenderTargetView)
			return false;

		return true;
	}

	UsageType CTexture::GetUsage(int flags) const
	{
		if (flags & IMMUTABLE_USAGE)
		{
			return D3D11_USAGE_IMMUTABLE;
		}
		else if (flags & STAGING_USAGE)
		{
			return D3D11_USAGE_STAGING;
		}
		else if (flags & DYNAMIC_USAGE)
		{
			return D3D11_USAGE_DYNAMIC;
		}
		return D3D11_USAGE_DEFAULT;
	}

	bool CTexture::Load(const std::string& filepath)
	{
		myFileName = filepath;
#ifdef SNOWBLIND_DX11
		ID3D11Device* device = CEngine::GetAPI()->GetDevice();
		HRESULT hr = DirectX::CreateDDSTextureFromFile(device
			, nullptr
			, std::wstring(filepath.begin(), filepath.end()).c_str()
			, nullptr
			, &m_ShaderResource);
#endif
		std::string debugname = CL::substr(filepath, "/", false, 0);
		CEngine::GetAPI()->SetDebugName(m_ShaderResource, debugname);
		DL_MESSAGE_EXP(FAILED(hr), "[Texture](Load) : Failed to load texture %s", filepath);
		if (FAILED(hr))
			return false;

		return true;
	}

	void CTexture::SetTexture(IShaderResourceView* aShaderResource)
	{
		m_ShaderResource = aShaderResource;
	}

	HRESULT CTexture::SaveToFile(ITexture2D*& texture_resource, const std::string& file_name)
	{
		ID3D11Resource* resource = nullptr;
		HRESULT hr = texture_resource->QueryInterface(IID_ID3D11Texture2D, (void**)&resource);
		CEngine::GetAPI()->HandleErrors(hr, "Failed to query interface of texture_resource");
		std::wstring middle_hand(file_name.begin(), file_name.end());
		LPCWSTR new_name(middle_hand.c_str());
		hr = DirectX::SaveDDSTextureToFile(CEngine::GetAPI()->GetContext(), resource, new_name);
		resource->Release();
		resource = nullptr;
		return S_OK;
	}

	void CTexture::CopyData(ITexture2D* dest, ITexture2D* source)
	{
#ifdef SNOWBLIND_DX11
		CEngine::GetAPI()->GetContext()->CopyResource(dest, source);
#endif
	}
};