#include "stdafx.h"
#include "DirectX11.h"
#include <DDSTextureLoader.h>
#include <ScreenGrab.h>

#if !defined(_PROFILE) && !defined(_FINAL)
#include <imgui/imgui_impl_dx11.h>
#endif

#include <sstream>


#include <Engine/DX11Device.h>
#include <Engine/DX11Context.h>
#include <Engine/Viewport.h>
#include <Input/InputHandle.h>



namespace graphics
{
	DirectX11::DirectX11(CreateInfo info)
	{
		m_CreateInfo = info;
		m_CreateInfo.m_APIName = "D3D11";
		m_PixelPickDesc.Width = 1;
		m_PixelPickDesc.Height = 1;
		m_PixelPickDesc.MipLevels = 1;
		m_PixelPickDesc.ArraySize = 1;
		m_PixelPickDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		m_PixelPickDesc.SampleDesc.Count = 1;
		m_PixelPickDesc.SampleDesc.Quality = 0;
		m_PixelPickDesc.Usage = D3D11_USAGE_STAGING;
		m_PixelPickDesc.BindFlags = 0;
		m_PixelPickDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		m_PixelPickDesc.MiscFlags = 0;
	}

	DirectX11::~DirectX11()
	{
#if !defined(_PROFILE) && !defined(_FINAL)
		ImGui_ImplDX11_Shutdown();
#endif
		ReleasePtr(m_FrameQuery);
		SAFE_DELETE(m_Viewport);
		for (auto it = m_Adapters.begin(); it != m_Adapters.end(); ++it)
		{
			SAFE_RELEASE(it->second);
		}
		m_Swapchain->SetFullscreenState(FALSE, nullptr);

		for (s32 i = 0; i < NOF_BS; i++)
		{
			ReleasePtr(m_BlendStates[i]);
			m_BlendStates[i] = nullptr;
		}

		for (s32 i = 0; i < NOF_RS; i++)
		{
			ReleasePtr(m_RasterizerStates[i]);
			m_RasterizerStates[i] = nullptr;
		}

		for (s32 i = 0; i < NOF_SS; i++)
		{
			ReleasePtr(m_SamplerStates[i]);
			m_SamplerStates[i] = nullptr;
		}

		for (s32 i = 0; i < NOF_DSS; i++)
		{
			ReleasePtr(m_DepthStencilStates[i]);
			m_DepthStencilStates[i] = nullptr;
		}

		ReleasePtr(m_DefaultDepthView);
		ReleasePtr(m_DefaultRenderTarget);
		ReleasePtr(m_DefaultDepthBuffer);
		ReleasePtr(m_InfoQueue);

		SAFE_RELEASE(m_Swapchain);

		ID3D11DeviceContext* ctx = static_cast<DX11Context*>(m_Context)->m_Context;
		ID3D11Device* dev = static_cast<DX11Device*>(m_Device)->GetDevice();
		ctx->ClearState();
		ctx->Flush();

		SAFE_DELETE(m_Context);
		SAFE_DELETE(m_Device);

		if (m_Debug != nullptr)
		{
			std::stringstream ss;
			ss << "\nDebug is released last. Will report as Live Object! 0x" << m_Debug << "\nWatch out for false reports. \n====\n";
			OutputDebugString(ss.str().c_str());
			m_Debug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL  | (D3D11_RLDO_FLAGS)0x4);
			SAFE_RELEASE(m_Debug);
		}


		ctx->Release();
		dev->Release();

#if defined (_DEBUG)
		OutputDebugString("\nIntRef is something that D3D has internal. You cannot control these.\n\n");
#endif
	}

	void DirectX11::Initiate()
	{
		CreateDeviceAndSwapchain();
		CreateRazterizers();
		CreateDepthStencilStates();
		CreateSamplerStates();
		CreateBackBuffer();
		CreateDepthBuffer();
		CreateBlendStates();

		m_Viewport = CreateViewport((u16)m_CreateInfo.m_WindowWidth, (u16)m_CreateInfo.m_WindowHeight, 0.f, 1.f, 0, 0);
		SetDefaultTargets();
		m_Context->SetViewport(m_Viewport);
#if !defined(_PROFILE) && !defined(_FINAL)
		ID3D11Device* pDevice = static_cast<DX11Device*>(m_Device)->GetDevice();
		ID3D11DeviceContext* pContext = static_cast<DX11Context*>(m_Context)->m_Context;
		ImGui_ImplDX11_Init(m_CreateInfo.m_HWND, pDevice, pContext);
#endif

	}


	void DirectX11::BeginFrame()
	{
#ifdef _PROFILE
		m_IntContext->Begin(m_FrameQuery);
#endif
		ID3D11RenderTargetView* pRenderTarget = static_cast<ID3D11RenderTargetView*>(m_DefaultRenderTarget);
		ID3D11DepthStencilView* pDepthView = static_cast<ID3D11DepthStencilView*>(m_DefaultDepthView);
		m_Context->ClearRenderTarget(pRenderTarget, clearcolor::black);
		m_Context->ClearDepthStencilView(pDepthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1);
	}

	void DirectX11::EndFrame()
	{
		const bool vsync = Engine::GetInstance()->VSync();
		Present(0, 0);
#ifdef _PROFILE
		m_IntContext->End(m_FrameQuery);
		while (S_OK != m_IntContext->GetData(m_FrameQuery, &m_Frequency, sizeof(D3D11_QUERY_DATA_TIMESTAMP_DISJOINT), 0))
		{
		}
#endif
	}


	void DirectX11::Present(u8 anInterval, u8 flags)
	{
		m_Swapchain->Present(anInterval, flags);
	}

	void DirectX11::CreateDeviceAndSwapchain()
	{
		DXGI_SWAP_CHAIN_DESC scDesc;
		ZeroMemory(&scDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

		scDesc.BufferCount = 1;
		scDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		scDesc.BufferDesc.Width = UINT(m_CreateInfo.m_WindowWidth);
		scDesc.BufferDesc.Height = UINT(m_CreateInfo.m_WindowHeight);
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scDesc.OutputWindow = m_CreateInfo.m_HWND;
		scDesc.SampleDesc.Count = 1;
		scDesc.SampleDesc.Quality = 0;
		scDesc.Windowed = true;
		//if (myEngineFlags[u16(eEngineFlags::FULLSCREEN)] == FALSE)
		//scDesc.Windowed = false;

		scDesc.Flags = 0;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		bool useVsync = false;

		u32 numerator = 0;
		u32 denominator = 1;

		if (useVsync)
			GetRefreshRate(numerator, denominator);

		scDesc.BufferDesc.RefreshRate.Numerator = numerator;
		scDesc.BufferDesc.RefreshRate.Denominator = denominator;


		const D3D_FEATURE_LEVEL requested_feature_levels[] = {
			D3D_FEATURE_LEVEL_11_0,
		};
		UINT createDeviceFlags = 0;

#ifdef _DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		UINT featureCount = ARRAYSIZE(requested_feature_levels);

		//Make it work later.

		JSONReader reader("Data/Config/game.json");
		std::string adapterString;
		reader.ReadElement("GraphicsAdapter", adapterString);

		myActiveAdapter = adapterString;

		D3D_DRIVER_TYPE type = D3D_DRIVER_TYPE_NULL;
		if (m_Adapters[adapterString] == nullptr)
		{
			myActiveAdapter = "Unknown";
			type = D3D_DRIVER_TYPE_HARDWARE;
		}
		else if (type)
		{
			type = D3D_DRIVER_TYPE_UNKNOWN;
		}

		ID3D11Device* pDevice = nullptr;
		ID3D11DeviceContext* pContext = nullptr;
		HRESULT hr = D3D11CreateDeviceAndSwapChain(m_Adapters[adapterString],
												   type,
												   nullptr,
												   createDeviceFlags,
												   requested_feature_levels,
												   featureCount,
												   D3D11_SDK_VERSION,
												   &scDesc,
												   &m_Swapchain,
												   &pDevice,
												   nullptr,
												   &pContext);

		if (pDevice == nullptr)
		{
			hr = D3D11CreateDeviceAndSwapChain(m_Adapters[adapterString],
											   type,
											   nullptr,
											   0,
											   requested_feature_levels,
											   featureCount,
											   D3D11_SDK_VERSION,
											   &scDesc,
											   &m_Swapchain,
											   &pDevice,
											   nullptr,
											   &pContext);
		}

		ASSERT(hr == S_OK, "Failed to Create (Device, Swapchain and Context)!");




#ifdef _DEBUG
		hr = pDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&m_Debug);
		if (hr != S_OK)
			OutputDebugStringA("Failed to Query Debug Interface. myDebug is NULL.");

		//D3D11_MESSAGE_ID hide[] = {
		//	D3D11_MESSAGE_ID_DEVICE_DRAW_RENDERTARGETVIEW_NOT_SET,
		//};

		//D3D11_INFO_QUEUE_FILTER filters;
		//ZeroMemory(&filters, sizeof(D3D11_INFO_QUEUE_FILTER));
		////filter.DenyList.Num
		//filters.DenyList.NumIDs = _countof(hide);
		//filters.DenyList.pIDList = hide;
		//hr = m_Debug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&m_InfoQueue);
		//m_InfoQueue->AddStorageFilterEntries(&filters);

#endif
		SetDebugName(m_Context, "DirectX11 Context Object");
		const std::string deviceName = "DirectX11 Device Object";
		const std::string swapchainName = "DirectX11 Swapchain Object";
		m_Swapchain->SetPrivateData(WKPDID_D3DDebugObjectName, u32(swapchainName.size()), swapchainName.c_str());
		pDevice->SetPrivateData(WKPDID_D3DDebugObjectName, u32(deviceName.size()), deviceName.c_str());

		m_Device = new DX11Device(pDevice);
		m_Context = new DX11Context(pContext);

		//ID3D11DeviceContext* pDeferredCtx;
		//pDevice->CreateDeferredContext(0, &pDeferredCtx); //should be created with thread pools. What about commandlists?


		m_IntDevice = pDevice;
		m_IntContext = pContext;

		D3D11_QUERY_DESC qdesc;
		qdesc.MiscFlags = 0;
		qdesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
		pDevice->CreateQuery(&qdesc, &m_FrameQuery);

	}

	void DirectX11::CreateDepthBuffer()
	{
		ID3D11Device* pDevice = static_cast<DX11Device*>(m_Device)->GetDevice();
		D3D11_TEXTURE2D_DESC depthDesc;
		ZeroMemory(&depthDesc, sizeof(depthDesc));

		depthDesc.Width = UINT(m_CreateInfo.m_WindowWidth);
		depthDesc.Height = UINT(m_CreateInfo.m_WindowHeight);
		depthDesc.MipLevels = 1;
		depthDesc.ArraySize = 1;
		depthDesc.Format = DXGI_FORMAT_D32_FLOAT;//DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthDesc.SampleDesc.Count = 1; //sample
		depthDesc.SampleDesc.Quality = 0; //quality pattern
		depthDesc.Usage = D3D11_USAGE_DEFAULT;
		depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		ID3D11Texture2D* pDepthBuffer = nullptr;
		HRESULT hr = pDevice->CreateTexture2D(&depthDesc, NULL, &pDepthBuffer);
		assert(!FAILED(hr) && "Failed to create texture for depthbuffer");

		D3D11_DEPTH_STENCIL_VIEW_DESC stencilDesc;
		ZeroMemory(&stencilDesc, sizeof(stencilDesc));
		stencilDesc.Format = depthDesc.Format;
		stencilDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		stencilDesc.Texture2D.MipSlice = 0;

		ID3D11DepthStencilView* pDepthView = nullptr;
		hr = pDevice->CreateDepthStencilView(pDepthBuffer, &stencilDesc, &pDepthView);
		ASSERT(hr == S_OK, "Failed to create depth stenci");

#ifdef _DEBUG
		SetDebugName(pDepthBuffer, "DirectX11 DepthBuffer Object");
		SetDebugName(pDepthView, "DirectX11 DepthView Object");
#endif
		m_DefaultDepthBuffer = pDepthBuffer;
		m_DefaultDepthView = pDepthView;
	}

	void DirectX11::CreateBackBuffer()
	{
		HRESULT hr = S_OK;
		ID3D11Texture2D* backbuffer = nullptr;
		hr = m_Swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backbuffer);
		HandleErrors(hr, "Failed to get Buffer!");

		hr = m_Swapchain->SetFullscreenState(FALSE, nullptr);
		HandleErrors(hr, "Failed to set Fullscreen/Borderless");

		ID3D11Device* pDevice = static_cast<DX11Device*>(m_Device)->GetDevice();
		ID3D11RenderTargetView* pRenderTarget = nullptr;
		ID3D11DepthStencilView* pDepthView = static_cast<ID3D11DepthStencilView*>(m_DefaultDepthView);

		hr = pDevice->CreateRenderTargetView(backbuffer, NULL, &pRenderTarget);
		m_DefaultRenderTarget = pRenderTarget;
		HandleErrors(hr, "Failed to create RenderTarget.");

		SAFE_RELEASE(backbuffer);
		m_Context->OMSetRenderTargets(1, &pRenderTarget, pDepthView);
#ifdef _DEBUG
		SetDebugName(pRenderTarget, "DirectX11 RenderTarget(Back Buffer) object");
#endif
	}

	Viewport* DirectX11::CreateViewport(u16 width, u16 height, float min_depth, float max_depth, u16 top_left_x, u16 top_left_y)
	{
		D3D11_VIEWPORT* new_viewport = new D3D11_VIEWPORT;

		new_viewport->TopLeftX = top_left_x;
		new_viewport->TopLeftY = top_left_y;
		new_viewport->Width = FLOAT(width);
		new_viewport->Height = FLOAT(height);
		new_viewport->MinDepth = min_depth;
		new_viewport->MaxDepth = max_depth;

		return new Viewport(width, height, top_left_x, top_left_y, max_depth, min_depth, new_viewport);
	}


	void DirectX11::CreateAdapterList()
	{
		std::vector<IDXGIAdapter*> enumAdapter;
		IDXGIFactory* factory = nullptr;
		CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);

		IDXGIAdapter* adapter;

		for (UINT i = 0; factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			enumAdapter.push_back(adapter);
		}
		SAFE_RELEASE(factory);

		for (u32 i = 0; i < enumAdapter.size(); ++i)
		{
			DXGI_ADAPTER_DESC adapterDesc;
			enumAdapter[i]->GetDesc(&adapterDesc);
			WCHAR* temp = adapterDesc.Description;
			//adapterDesc.DedicatedVideoMemory;
			s8 dst[128];
			std::wcstombs(dst, temp, 128);
			std::string actualString(dst);
			myAdaptersName.push_back(actualString);
			m_Adapters.insert(std::pair<std::string, IDXGIAdapter*>(actualString, enumAdapter[i]));
		}
	}

	void DirectX11::ReleasePtr(void* ptr)
	{
		if (!ptr)
			return;
		IUnknown* pUnknown = static_cast<IUnknown*>(ptr);
		pUnknown->Release();
	}

	void DirectX11::SetDefaultTargets()
	{
		ID3D11RenderTargetView* pRenderTarget = static_cast<ID3D11RenderTargetView*>(m_DefaultRenderTarget);
		ID3D11DepthStencilView* pDepthView = static_cast<ID3D11DepthStencilView*>(m_DefaultDepthView);
		m_Context->OMSetRenderTargets(1, &pRenderTarget, pDepthView);
	}

	void DirectX11::ResetViewport()
	{
		m_Context->SetViewport(m_Viewport);
	}

#ifdef _DEBUG
	void DirectX11::ReportLiveObjects()
	{
		ID3D11DeviceContext* ctx = static_cast<DX11Context*>(m_Context)->m_Context;
		ctx->ClearState();
		ctx->Flush();
		m_Debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_SUMMARY | (D3D11_RLDO_FLAGS)0x4);
	}
#endif

	void DirectX11::OnAltEnter()
	{
		if (!m_Swapchain)
			return;

		m_Swapchain->SetFullscreenState(FALSE, nullptr);
		const Window& win = Engine::GetInstance()->GetWindow();
		if (win.IsFullscreen())
		{
			m_Swapchain->SetFullscreenState(TRUE, nullptr);
			return;
		}
	}

	void DirectX11::OnResize()
	{
		SAFE_RELEASE(m_Swapchain);

		IDXGIFactory* factory;
		HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
		HandleErrors(hr, "(OnResize) Failed to create Factory!");

		DXGI_SWAP_CHAIN_DESC scDesc;
		ZeroMemory(&scDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

		UINT sym_x = GetSystemMetrics(SM_CXSCREEN);
		UINT sym_y = GetSystemMetrics(SM_CYSCREEN);

		scDesc.BufferCount = 1;
		scDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		scDesc.BufferDesc.Width = UINT(sym_x);
		scDesc.BufferDesc.Height = UINT(sym_y);
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scDesc.OutputWindow = m_CreateInfo.m_HWND;
		scDesc.SampleDesc.Count = 1;
		scDesc.SampleDesc.Quality = 0;
		scDesc.Windowed = true;

		scDesc.Flags = 0;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		bool useVsync = false;

		u32 numerator = 0;
		u32 denominator = 1;

		if (useVsync)
		{
			GetRefreshRate(numerator, denominator);
		}

		scDesc.BufferDesc.RefreshRate.Numerator = numerator;
		scDesc.BufferDesc.RefreshRate.Denominator = denominator;

		hr = factory->CreateSwapChain(static_cast<DX11Device*>(m_Device)->GetDevice(), &scDesc, &m_Swapchain);

		SAFE_RELEASE(factory);
	}

	CU::Vector4f DirectX11::PickColor(Texture* pTexture)
	{
		
		//this might have to run at the end of each frame, and we put in a request for the function in a command queue
		graphics::DX11Device& dx11dev = static_cast<graphics::DX11Device&>(GetDevice());
		ID3D11Device* pDevice = static_cast<ID3D11Device*>(dx11dev.GetDevice());

		graphics::DX11Context& dx11ctx = static_cast<graphics::DX11Context&>(GetContext());
		ID3D11DeviceContext* ctx = static_cast<ID3D11DeviceContext*>(dx11ctx.GetContext());

		WindowSize window_size;
		window_size.m_Height = m_CreateInfo.m_WindowHeight;
		window_size.m_Width = m_CreateInfo.m_WindowWidth;

		ID3D11Texture2D* _IDTex = static_cast<ID3D11Texture2D*>(pTexture->GetTexture());

		const CU::Vector2f fPos = Engine::GetInstance()->GetInputHandle()->GetCursorPos();
		const CU::Vector2i iPos = { (s32)fPos.x, (s32)fPos.y };

		//this could be saved
		D3D11_BOX region_box;
		CU::Vector4f color;

		if (iPos.x > 1919 || iPos.x < 0 || iPos.y > 1080 || iPos.y < 0)
			return color;


		region_box.bottom = cl::ClampI(iPos.y, 0, (s32)window_size.m_Height) + 1;
		region_box.right = cl::ClampI(iPos.x, 0, (s32)window_size.m_Width) + 1;
		region_box.back = 1;

		region_box.top = cl::ClampI(iPos.y, 0, (s32)window_size.m_Height);
		region_box.left = cl::ClampI(iPos.x, 0, (s32)window_size.m_Width);
		region_box.front = 0;

		ID3D11Texture2D * staging = nullptr;
		pDevice->CreateTexture2D(&m_PixelPickDesc, nullptr, &staging);
		ctx->CopySubresourceRegion(staging, 0, 0, 0, 0, _IDTex, 0, &region_box);

		D3D11_MAPPED_SUBRESOURCE msr;
		ZeroMemory(&msr, sizeof(D3D11_MAPPED_SUBRESOURCE));
		HRESULT hr = ctx->Map(staging, 0, D3D11_MAP_READ, 0, &msr);
		ASSERT(hr == S_OK, "Not ok pixel pick!");
		if (msr.pData)
		{
			float* data = (float*)msr.pData;
			color.x = data[0] * 65536.f;
			color.y = data[1] * 256.f;
			color.z = data[2];
			color.w = 1.f;
		}
		ctx->Unmap(staging, 0);
		staging->Release();

		return color;

	}

	void DirectX11::SaveTextureToDisk(ID3D11Texture2D* texture_resource, const std::string& file_name)
	{
		ID3D11DeviceContext* ctx = static_cast<DX11Context*>(m_Context)->m_Context;
		ID3D11Resource* resource = nullptr;
		HRESULT hr = texture_resource->QueryInterface(IID_ID3D11Texture2D, (void**)&resource);
		HandleErrors(hr, "Failed to query interface of texture_resource");
		std::wstring middle_hand(file_name.begin(), file_name.end());
		LPCWSTR new_name(middle_hand.c_str());
		hr = DirectX::SaveDDSTextureToFile(ctx, resource, new_name);
		resource->Release();
	}

	void DirectX11::SaveTextureToDisk(ID3D11ShaderResourceView* texture_resource, const std::wstring& file_name)
	{
		ID3D11DeviceContext* ctx = static_cast<DX11Context*>(m_Context)->m_Context;
		ID3D11Resource* resource = nullptr;
		texture_resource->GetResource(&resource);
		HRESULT hr = DirectX::SaveDDSTextureToFile(ctx, resource, file_name.c_str());
		ASSERT(hr == S_OK, "failed to save. Sad");
	}

	void DirectX11::CreateRazterizers()
	{
		D3D11_RASTERIZER_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_RASTERIZER_DESC));
		desc.FrontCounterClockwise = false;
		desc.DepthBias = false;
		desc.DepthBiasClamp = 0;
		desc.SlopeScaledDepthBias = 0;
		desc.DepthClipEnable = false;
		desc.ScissorEnable = false;
		desc.MultisampleEnable = false;
		desc.AntialiasedLineEnable = false;

		ID3D11Device* pDevice = static_cast<DX11Device*>(m_Device)->GetDevice();

		desc.FillMode = D3D11_FILL_WIREFRAME;
		desc.CullMode = D3D11_CULL_NONE;
		CreateRasterizerState(desc, WIREFRAME, "Wireframe");

		desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_BACK;
		CreateRasterizerState(desc, CULL_BACK, "CULL_BACK");

		desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_FRONT;
		CreateRasterizerState(desc, CULL_FRONT, "CULL_FRONT");

		desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_NONE;
		CreateRasterizerState(desc, CULL_NONE, "CULL_NONE");

	}

	void DirectX11::CreateRasterizerState(const D3D11_RASTERIZER_DESC& desc, eRasterizer rasterizer, const char* debugname)
	{
		ID3D11Device* pDevice = static_cast<DX11Device*>(m_Device)->GetDevice();
		ID3D11RasterizerState* rasterstate = nullptr;
		pDevice->CreateRasterizerState(&desc, &rasterstate);
		m_RasterizerStates[rasterizer] = rasterstate;
#ifdef _DEBUG
		SetDebugName(rasterstate, debugname);
#endif
	}

	void DirectX11::GetRefreshRate(u32& aNumerator, u32& aDenominator)
	{
		IDXGIFactory* factory;
		IDXGIAdapter* adapter;
		IDXGIOutput* adapterOutput;
		unsigned int numModes;
		DXGI_MODE_DESC* displayModeList;

		HRESULT result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
		result = factory->EnumAdapters(0, &adapter);
		result = adapter->EnumOutputs(0, &adapterOutput);
		adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
		displayModeList = new DXGI_MODE_DESC[numModes];
		result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);

		for (unsigned int i = 0; i < numModes; ++i)
		{
			if (displayModeList[i].Width == (u32)m_CreateInfo.m_WindowWidth && displayModeList[i].Height == (u32)m_CreateInfo.m_WindowHeight)
			{
				aNumerator = displayModeList[i].RefreshRate.Numerator;
				aDenominator = displayModeList[i].RefreshRate.Denominator;
			}
		}

		delete[] displayModeList;
		displayModeList = nullptr;
		SAFE_RELEASE(adapterOutput);
		SAFE_RELEASE(adapter);
		SAFE_RELEASE(factory);
	}

	void DirectX11::HandleErrors(const HRESULT& aResult, const std::string& anErrorString)
	{
		std::string toError;
		switch (aResult)
		{
			case D3D11_ERROR_FILE_NOT_FOUND:
			{
				toError = (anErrorString + " File not found!");
			}break;
			case D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS:
			{
				toError = (anErrorString + " Too many unique state objects!");
			}break;
			case D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS:
			{
				toError = (anErrorString + " Too many view objects!");
			}break;
			case D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD:
			{
				toError = (anErrorString + " Deferred Context Map Without Initial Discard!");
			}break;
			case DXGI_ERROR_INVALID_CALL:
			{
				toError = (anErrorString + " Invalid call!");
			}break;
			case DXGI_ERROR_WAS_STILL_DRAWING:
			{
				toError = (anErrorString + " Were still drawing!");
			}break;
			case E_FAIL:
			{
				toError = (anErrorString + " Failed!");
			}break;
			case E_INVALIDARG:
			{
				toError = (anErrorString + " One or more arguments were invalid!");
			}break;
			case E_OUTOFMEMORY:
			{
				toError = (anErrorString + " Out of Memory!");
			}break;
			case E_NOTIMPL:
			{
				toError = (anErrorString + " The method call isn't implemented with the passed parameter combination.");
			}break;
			case S_FALSE:
			{
				toError = (anErrorString + " Something went wrong. Returned false!");

			}break;
		}

		if (!toError.empty())
		{
			DL_MESSAGE("%s", toError.c_str());
			ASSERT(aResult == S_OK, toError.c_str());
		}
	}

	DXGI_FORMAT DirectX11::GetFormat(s32 format)
	{

		//___________________________________________________
		if (format & graphics::RGBA32_FLOAT)
			return DXGI_FORMAT_R32G32B32A32_FLOAT;

		if (format & graphics::RGBA32_UINT)
			return DXGI_FORMAT_R32G32B32A32_UINT;

		if (format & graphics::RGBA32_SINT)
			return DXGI_FORMAT_R32G32B32A32_SINT;


		//___________________________________________________
		if (format & graphics::RGB32_FLOAT)
			return DXGI_FORMAT_R32G32B32_FLOAT;

		if (format & graphics::RGB32_UINT)
			return DXGI_FORMAT_R32G32B32_UINT;

		if (format & graphics::RGB32_SINT)
			return DXGI_FORMAT_R32G32B32_SINT;


		//___________________________________________________
		if (format & graphics::RGBA16_FLOAT)
			return DXGI_FORMAT_R16G16B16A16_FLOAT;

		if (format & graphics::RGBA16_UINT)
			return DXGI_FORMAT_R16G16B16A16_UINT;

		if (format & graphics::RGBA16_SINT)
			return DXGI_FORMAT_R16G16B16A16_SINT;

		//___________________________________________________

		if (format & graphics::RGBA8_UINT)
			return DXGI_FORMAT_R8G8B8A8_UINT;

		if (format & graphics::RGBA8_SINT)
			return DXGI_FORMAT_R8G8B8A8_SINT;

		if (format & graphics::RGBA8_UNORM)
			return DXGI_FORMAT_R8G8B8A8_UNORM;


		//___________________________________________________
		if (format & graphics::R32_TYPELESS)
			return DXGI_FORMAT_R32_TYPELESS;

		if (format & graphics::R32_FLOAT)
			return DXGI_FORMAT_R32_FLOAT;

		if (format & graphics::DEPTH_32_FLOAT)
			return DXGI_FORMAT_D32_FLOAT;

		if (format & graphics::R32_UINT)
			return DXGI_FORMAT_R32_UINT;

		if (format & graphics::RG32_FLOAT)
			return DXGI_FORMAT_R32G32_FLOAT;

		if (format & graphics::RGB10A2_TYPELESS)
			return DXGI_FORMAT_R10G10B10A2_TYPELESS;

		if (format & graphics::RGBA8_TYPELESS)
			return DXGI_FORMAT_R8G8B8A8_TYPELESS;

		if (format & graphics::sRGBA8)
			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		


		DL_ASSERT("invalid texture format");
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	}

	DXGI_FORMAT DirectX11::GetFormat(eVertexFormat format)
	{
		if (format == eVertexFormat::_4BYTE_R_FLOAT)
			return DXGI_FORMAT_R32_FLOAT;
		if (format == eVertexFormat::_4BYTE_R_UINT)
			return DXGI_FORMAT_R32_UINT;
		if (format == eVertexFormat::_8BYTE_RG)
			return DXGI_FORMAT_R32G32_FLOAT;
		if (format == eVertexFormat::_12BYTE_RGB)
			return DXGI_FORMAT_R32G32B32_FLOAT;
		if (format == eVertexFormat::_16BYTE_RGBA)
			return DXGI_FORMAT_R32G32B32A32_FLOAT;

		DL_ASSERT("invalid vertex format");
		return DXGI_FORMAT_UNKNOWN;
	}

	D3D11_USAGE DirectX11::GetUsage(s32 usage)
	{
		if (usage == DEFAULT_USAGE)
			return D3D11_USAGE_DEFAULT;
		if (usage == IMMUTABLE_USAGE)
			return D3D11_USAGE_IMMUTABLE;
		if (usage == DYNAMIC_USAGE)
			return D3D11_USAGE_DYNAMIC;
		if (usage == STAGING_USAGE)
			return D3D11_USAGE_STAGING;
	}

	u32 DirectX11::GetBindFlag(s32 binding)
	{
		u32 output = 0;

		if (binding & graphics::BIND_VERTEX_BUFFER)
			output |= D3D11_BIND_VERTEX_BUFFER;
		if (binding & graphics::BIND_INDEX_BUFFER)
			output |= D3D11_BIND_INDEX_BUFFER;
		if (binding & graphics::BIND_CONSTANT_BUFFER)
			output |= D3D11_BIND_CONSTANT_BUFFER;
		if (binding & graphics::BIND_SHADER_RESOURCE)
			output |= D3D11_BIND_SHADER_RESOURCE;
		if (binding & graphics::BIND_STREAM_OUTPUT)
			output |= D3D11_BIND_STREAM_OUTPUT;
		if (binding & graphics::BIND_RENDER_TARGET)
			output |= D3D11_BIND_RENDER_TARGET;
		if (binding & graphics::BIND_DEPTH_STENCIL)
			output |= D3D11_BIND_DEPTH_STENCIL;
		if (binding & graphics::BIND_UNORDERED_ACCESS)
			output |= D3D11_BIND_UNORDERED_ACCESS;
		if (binding & graphics::BIND_DECODER)
			output |= D3D11_BIND_DECODER;
		if (binding & graphics::BIND_VIDEO_ENCODER)
			output |= D3D11_BIND_VIDEO_ENCODER;


		return output;
	}

	u32 DirectX11::GetCPUAccessFlag(s32 flags)
	{
		u32 output = 0;

		if (flags & eCPUAccessFlag::READ)
			output |= D3D11_CPU_ACCESS_READ;
		if (flags& eCPUAccessFlag::WRITE)
			output |= D3D11_CPU_ACCESS_WRITE;

		return output;
	}

	D3D11_PRIMITIVE_TOPOLOGY DirectX11::GetTopology(eTopology topology)
	{
		if (topology == eTopology::TRIANGLE_LIST)
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		if (topology == eTopology::_4_CONTROL_POINT_PATCHLIST)
			return D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;
		if (topology == eTopology::POINT_LIST)
			return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
		if (topology == eTopology::LINE_LIST)
			return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

		return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	}

	D3D11_MAP DirectX11::GetMapping(eMapping mapping)
	{
		s32 _map = mapping + 1;
		return static_cast<D3D11_MAP>(_map);
	}

	D3D11_INPUT_CLASSIFICATION DirectX11::GetInputClass(eElementSpecification el)
	{
		if (el == INPUT_PER_VERTEX_DATA)
			return D3D11_INPUT_PER_VERTEX_DATA;
		if (el == INPUT_PER_INSTANCE_DATA)
			return D3D11_INPUT_PER_INSTANCE_DATA;



		DL_ASSERT("Shouldn't get here, invalid input element specification!");
		return D3D11_INPUT_PER_VERTEX_DATA;
	}

	void DirectX11::SetDebugName(void * pResource, std::string debug_name)
	{
		if (!pResource)
			return;

		ID3D11DeviceChild* resource = static_cast<ID3D11DeviceChild*>(pResource);
		resource->SetPrivateData(WKPDID_D3DDebugObjectName, debug_name.length(), debug_name.c_str());
	}


};


