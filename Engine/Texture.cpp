#include "stdafx.h"
#include "Texture.h"
#include <Engine/IGraphicsDevice.h>
#include <Engine/DX11Device.h>

#include <Engine/Engine.h>

#include <DXTex/DirectXTex.h>
#include <wincodec.h>
Texture::Texture(IShaderResourceView* srv)
	: m_ShaderResource(srv)
{
}

Texture::Texture(IShaderResourceView* srv, ITexture2D* tex)
	: m_ShaderResource(srv)
	, m_Texture(tex)
{
}

#ifdef _DEBUG

Texture::Texture(IShaderResourceView* srv, ITexture2D* tex, std::string debug_name)
	: m_DebugName(debug_name)
	, m_ShaderResource(srv)
	, m_Texture(tex)
{
}

Texture::Texture(IShaderResourceView* srv, std::string debug_name)
	: m_DebugName(debug_name)
	, m_ShaderResource(srv)
{
}

#endif

Texture::~Texture()
{
	graphics::IGraphicsAPI* api = Engine::GetAPI();
	api->ReleasePtr(m_ShaderResource);
	m_ShaderResource = nullptr;
	api->ReleasePtr(m_DepthTexture);
	m_DepthTexture = nullptr;
	api->ReleasePtr(m_DepthStencilView);
	m_DepthStencilView = nullptr;
	api->ReleasePtr(m_RenderTargetView);
	m_RenderTargetView = nullptr;
}

void Texture::Initiate(const TextureDesc& desc, const std::string& debug_name)
{
	Initiate(desc, true, debug_name);
}

void Texture::Initiate(const TextureDesc& desc, bool create_from_texture, const std::string& debug_name)
{
	m_Width = desc.m_Width;
	m_Height = desc.m_Height;
#ifdef _DEBUG
	m_DebugName = debug_name;
#endif
	graphics::IGraphicsDevice& device = Engine::GetAPI()->GetDevice();
	ASSERT(desc.m_TextureFormat != graphics::NO_FORMAT, "invalid texture format!");

	graphics::Texture2DDesc tex_desc;
	tex_desc.m_Height = desc.m_Height;
	tex_desc.m_Width = desc.m_Width;

	if (desc.m_TextureFormat != graphics::NO_FORMAT)
	{

		tex_desc.m_Format = desc.m_TextureFormat;
		tex_desc.m_MipLevels = desc.m_MipCount;
		tex_desc.m_ArraySize = 1;
		tex_desc.m_Usage = desc.m_Usage;
		tex_desc.m_MiscFlags = desc.m_MiscFlags;
		tex_desc.m_SampleCount = 1;
		tex_desc.m_SampleQuality = 0;
		tex_desc.m_CPUAccessFlag = 0;
		tex_desc.m_Binding = desc.m_ResourceTypeBinding;
		m_Texture = device.CreateTexture2D(tex_desc, debug_name + "_Texture");
	}

	if (desc.m_ShaderResourceFormat != graphics::NO_FORMAT)
	{
		tex_desc.m_Format = desc.m_ShaderResourceFormat;
		tex_desc.m_MipLevels = 0;
		tex_desc.m_ArraySize = 0;
		tex_desc.m_Usage = desc.m_Usage;
		tex_desc.m_MiscFlags = desc.m_MiscFlags;
		tex_desc.m_SampleCount = 1;
		tex_desc.m_SampleQuality = 0;
		tex_desc.m_CPUAccessFlag = desc.m_CPUAccessFlag;
		tex_desc.m_Binding = desc.m_ResourceTypeBinding;
		if (create_from_texture)
			m_ShaderResource = device.CreateShaderResource(tex_desc, m_Texture, debug_name + "_SRV");
		else
			m_ShaderResource = device.CreateShaderResource(m_Texture, debug_name + "_SRV");

	}

	if (desc.m_RenderTargetFormat != graphics::NO_FORMAT)
	{
		tex_desc.m_Format = desc.m_RenderTargetFormat;
		tex_desc.m_MipLevels = 0;
		tex_desc.m_ArraySize = 0;
		tex_desc.m_Usage = desc.m_Usage;
		tex_desc.m_MiscFlags = desc.m_MiscFlags;
		tex_desc.m_SampleCount = 1;
		tex_desc.m_SampleQuality = 0;
		tex_desc.m_CPUAccessFlag = desc.m_CPUAccessFlag;
		tex_desc.m_Binding = graphics::BIND_RENDER_TARGET;
		m_RenderTargetView = device.CreateRenderTarget(tex_desc, m_Texture, debug_name + "_RTV");
	}

	if (desc.m_DepthTextureFormat != graphics::NO_FORMAT)
	{
		tex_desc.m_Format = desc.m_DepthTextureFormat;
		tex_desc.m_MipLevels = 0;
		tex_desc.m_ArraySize = 0;
		tex_desc.m_Usage = desc.m_Usage;
		tex_desc.m_MiscFlags = desc.m_MiscFlags;
		tex_desc.m_SampleCount = 1;
		tex_desc.m_SampleQuality = 0;
		tex_desc.m_CPUAccessFlag = desc.m_CPUAccessFlag;
		tex_desc.m_Binding = graphics::BIND_DEPTH_STENCIL;
		m_DepthStencilView = device.CreateDepthStencilView(tex_desc, m_Texture, debug_name + "_DSV");
	}
}

void Texture::InitiateAsDepthStencil(int32 width, int32 height, const std::string& debug_name)
{
	TextureDesc desc;
	desc.m_Width = width;
	desc.m_Height = height;
	desc.m_ResourceTypeBinding = graphics::BIND_SHADER_RESOURCE | graphics::BIND_DEPTH_STENCIL;
	desc.m_Usage = graphics::DEFAULT_USAGE;
	desc.m_TextureFormat = graphics::R32_TYPELESS;
	desc.m_DepthTextureFormat = graphics::DEPTH_32_FLOAT;
	//desc.m_RenderTargetFormat = graphics::R32_TYPELESS;
	desc.m_ShaderResourceFormat = graphics::R32_FLOAT;

	Initiate(desc, debug_name);
}

void Texture::InitiateAsRenderTarget(int32 width, int32 height, const std::string& debug_name)
{
	TextureDesc desc;
	desc.m_Width = width;
	desc.m_Height = height;
	desc.m_ResourceTypeBinding = graphics::BIND_SHADER_RESOURCE | graphics::BIND_RENDER_TARGET;
	desc.m_Usage = graphics::DEFAULT_USAGE;
	desc.m_TextureFormat = graphics::RGBA16_FLOAT;
	desc.m_ShaderResourceFormat = graphics::RGBA16_FLOAT;
	desc.m_RenderTargetFormat = graphics::RGBA16_FLOAT;

	Initiate(desc, debug_name);

}

void Texture::CreateTextureArray(const char* paths[], const int32 num_tex, const char* filename)
{
	ID3D11Device* device = static_cast<graphics::DX11Device&>(Engine::GetAPI()->GetDevice()).GetDevice();
	ID3D11DeviceContext* ctx = nullptr;
	device->GetImmediateContext(&ctx);

	const uint32 tex_count = num_tex;
	CU::GrowingArray<ID3D11ShaderResourceView*> src(tex_count);

	for (uint32 i = 0; i < tex_count; i++)
	{
		IShaderResourceView* srv = Engine::GetAPI()->GetDevice().CreateTextureFromFile(paths[i], false, &Engine::GetAPI()->GetContext());
		src.Add(static_cast<ID3D11ShaderResourceView*>(srv));
	}

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	ID3D11Resource* resource = nullptr;
	src[0]->GetResource(&resource);
	((ID3D11Texture2D*)resource)->GetDesc(&desc);

	D3D11_TEXTURE2D_DESC arr_desc;
	arr_desc.Width = desc.Width;
	arr_desc.Height = desc.Height;
	arr_desc.MipLevels = desc.MipLevels;
	arr_desc.ArraySize = tex_count;
	arr_desc.Format = desc.Format;
	arr_desc.SampleDesc.Count = 1;
	arr_desc.SampleDesc.Quality = 0;
	arr_desc.Usage = D3D11_USAGE_DEFAULT;
	arr_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	arr_desc.CPUAccessFlags = 0;
	arr_desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	ID3D11Texture2D* texArray = nullptr;
	HRESULT hr = device->CreateTexture2D(&arr_desc, nullptr, &texArray);
	ASSERT(hr == S_OK, "Failed to Create texture");

	for (uint32 i = 0; i < tex_count; i++)
	{
		ID3D11Resource* texture_resource = nullptr;
		src[i]->GetResource(&texture_resource);
		//target, index, x,y,z, resource, index, optional box
		ctx->CopySubresourceRegion(texArray, i, 0, 0, 0, texture_resource, 0, nullptr);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = arr_desc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	viewDesc.TextureCube.MostDetailedMip = 0;
	viewDesc.TextureCube.MipLevels = arr_desc.MipLevels;

	ID3D11ShaderResourceView* srv = nullptr;
	hr = device->CreateShaderResourceView(texArray, &viewDesc, &srv);
	ASSERT(hr == S_OK, "Failed to Create srv");
	m_ShaderResource = srv;

	DirectX::ScratchImage image;
	hr = DirectX::CaptureTexture(device, ctx, texArray, image);
	ASSERT(hr == S_OK, "Failed to capture texture");
	DirectX::SaveToDDSFile(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::DDS_FLAGS_NONE, cl::ToWideStr(filename).c_str());
	ctx->Release();
}

void Texture::CreateTextureArray(Texture* textures[], const int32 num_tex, const char* filename)
{
	ID3D11Device* device = static_cast<graphics::DX11Device&>(Engine::GetAPI()->GetDevice()).GetDevice();
	ID3D11DeviceContext* ctx = nullptr;
	device->GetImmediateContext(&ctx);

	const uint32 tex_count = num_tex;
	CU::GrowingArray<ID3D11ShaderResourceView*> src(tex_count);

	for (uint32 i = 0; i < tex_count; i++)
	{
		IShaderResourceView* srv = textures[i]->GetShaderView();//Engine::GetAPI()->GetDevice().CreateTextureFromFile(paths[i], false, &Engine::GetAPI()->GetContext());
		src.Add(static_cast<ID3D11ShaderResourceView*>(srv));
	}

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	ID3D11Resource* resource = nullptr;
	src[0]->GetResource(&resource);
	((ID3D11Texture2D*)resource)->GetDesc(&desc);

	D3D11_TEXTURE2D_DESC arr_desc;
	arr_desc.Width = desc.Width;
	arr_desc.Height = desc.Height;
	arr_desc.MipLevels = desc.MipLevels;
	arr_desc.ArraySize = tex_count;
	arr_desc.Format = desc.Format;
	arr_desc.SampleDesc.Count = 1;
	arr_desc.SampleDesc.Quality = 0;
	arr_desc.Usage = D3D11_USAGE_DEFAULT;
	arr_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	arr_desc.CPUAccessFlags = 0;
	arr_desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	ID3D11Texture2D* texArray = nullptr;

	HRESULT hr = device->CreateTexture2D(&arr_desc, nullptr, &texArray);
	ASSERT(hr == S_OK, "Failed to Create texture");

	for (uint32 i = 0; i < tex_count; i++)
	{
		ID3D11Resource* texture_resource = nullptr;
		src[i]->GetResource(&texture_resource);
		//target, index, x,y,z, resource, index, optional box
		for (uint32 j = 0; j < desc.MipLevels; j++)
		{

			ctx->CopySubresourceRegion(texArray, (i * desc.MipLevels) + j, 0, 0, 0, texture_resource, j, nullptr);
		}
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = arr_desc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	viewDesc.TextureCube.MostDetailedMip = 0;
	viewDesc.TextureCube.MipLevels = arr_desc.MipLevels;

	ID3D11ShaderResourceView* srv = nullptr;
	hr = device->CreateShaderResourceView(texArray, &viewDesc, &srv);
	ASSERT(hr == S_OK, "Failed to Create srv");
	m_ShaderResource = srv;

	DirectX::ScratchImage image;
	hr = DirectX::CaptureTexture(device, ctx, texArray, image);
	ASSERT(hr == S_OK, "Failed to capture texture");
	DirectX::SaveToDDSFile(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::DDS_FLAGS_NONE, cl::ToWideStr(filename).c_str());
	ctx->Release();
}

void Texture::Create3DTexture(const char* path, int32 slice_width, int32 slice_height, int32, const char*)
{
	ID3D11Device* device = static_cast<graphics::DX11Device&>(Engine::GetAPI()->GetDevice()).GetDevice();
	ID3D11DeviceContext* ctx = nullptr;
	device->GetImmediateContext(&ctx);
	//const u32 tex_count = num_tex;
	//CU::GrowingArray<ID3D11ShaderResourceView*> src(0);
	ID3D11ShaderResourceView* _tex = (ID3D11ShaderResourceView*)Engine::GetAPI()->GetDevice().CreateTextureFromFile(path, false, &Engine::GetAPI()->GetContext());

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	ID3D11Resource* resource = nullptr;
	_tex->GetResource(&resource);
	((ID3D11Texture2D*)resource)->GetDesc(&desc);
	int32 width_count, height_count;

	width_count = desc.Width / slice_width;
	height_count = (desc.Height / slice_height);

	ID3D11Texture3D* tex;
	D3D11_TEXTURE3D_DESC _3Ddesc;
	_3Ddesc.Height = slice_height;
	_3Ddesc.Width = slice_width;
	_3Ddesc.Depth = width_count * height_count;
	_3Ddesc.MipLevels = 1;
	_3Ddesc.Usage = D3D11_USAGE_DEFAULT;
	_3Ddesc.CPUAccessFlags = 0;
	_3Ddesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	_3Ddesc.Format = desc.Format;
	_3Ddesc.MiscFlags = 0;

	HRESULT hr = device->CreateTexture3D(&_3Ddesc, nullptr, &tex);
	ASSERT(hr == S_OK, "Failed to Create tex");

	int32 z = 0;
	for (int32 y = 0; y < height_count; y++)
	{
		for (int32 x = 0; x < width_count; x++)
		{
			D3D11_BOX region_box;
			region_box.front = 0;
			region_box.left = (x * slice_width);
			region_box.top = (y * slice_height);

			region_box.bottom = (y * slice_height) + slice_height;
			region_box.right = (x * slice_width) + slice_width;
			region_box.back = 1;


			ID3D11Resource* texture_resource = nullptr;
			_tex->GetResource(&texture_resource);
			//target, index, x,y,z, resource, index, optional box
			ctx->CopySubresourceRegion(tex, 0, 0, 0, z, texture_resource, 0, &region_box);
			texture_resource->Release();
			z++;
		}
	}



	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = _3Ddesc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
	viewDesc.Texture3D.MipLevels = _3Ddesc.MipLevels;
	viewDesc.Texture3D.MostDetailedMip = 0;

	ID3D11ShaderResourceView* srv = nullptr;
	hr = device->CreateShaderResourceView(tex, &viewDesc, &srv);
	ASSERT(hr == S_OK, "Failed to Create srv");
	m_ShaderResource = srv;
	tex->Release();
	resource->Release();
	_tex->Release();
	ctx->Release();


}

void Texture::SaveToDisk(const wchar_t* path, ITexture2D* tex, texture_format::FORMAT format)
{
	ID3D11Device* device = static_cast<graphics::DX11Device&>(Engine::GetAPI()->GetDevice()).GetDevice();
	ID3D11DeviceContext* ctx = nullptr;
	device->GetImmediateContext(&ctx);

	DirectX::ScratchImage image;
	HRESULT hr = DirectX::CaptureTexture(device, ctx, (ID3D11Texture2D*)tex, image);
	ASSERT(hr == S_OK, "Failed to capture texture");

	switch (format)
	{
	case texture_format::DDS:
	DirectX::SaveToDDSFile(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::DDS_FLAGS_NONE, path);
		break;
	case texture_format::PNG:
	DirectX::SaveToWICFile(image.GetImages(), image.GetImageCount(), DirectX::WIC_FLAGS_NONE, GUID_ContainerFormatPng, path, &GUID_WICPixelFormat24bppBGR);
		break;
	default:
		ASSERT(false, "no implementation to save in this format!");
	}

	ctx->Release();
}
