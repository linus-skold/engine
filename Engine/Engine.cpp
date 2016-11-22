#include "stdafx.h"

#include <d3dcompiler.h>

#include "AssetsContainer.h"
#include "IGraphicsAPI.h"
#include "Renderer.h"
#include "Synchronizer.h"
#include "Terrain.h"
#include "TerrainManager.h"


namespace Snowblind
{

	bool CEngine::HasInitiated()
	{
		return (this && m_IsInitiated);
	}

	CEngine* CEngine::myInstance = nullptr;
	IGraphicsAPI* CEngine::myAPI = nullptr;

	void CEngine::Create()
	{
		DL_ASSERT_EXP(myInstance == nullptr, "Instance already created!");
		myInstance = new CEngine;
	}

	void CEngine::Destroy()
	{
		DL_ASSERT_EXP(myInstance != nullptr, "Can't destroy the instance before it's created. Did you call Destroy twice?");
		SAFE_DELETE(myInstance);
	}

	CEngine* CEngine::GetInstance()
	{
		DL_ASSERT_EXP(myInstance != nullptr, "Can't Get the instance before it's created. Did you call Destroy twice?");
		return myInstance;
	}

#ifdef SNOWBLIND_DX11
	DirectX11* CEngine::GetAPI()
	{
		return static_cast<DirectX11*>(myAPI);
	}
#else
	Vulkan* CEngine::GetAPI()
	{
		return static_cast<Vulkan*>(myAPI);
	}
#endif

	bool CEngine::InitiateDebugSystem(CSynchronizer* synchronizer, InputHandle* input_handle)
	{
		m_DebugSystem.Initiate(synchronizer, input_handle);
		return true;
	}

	bool CEngine::Initiate(float window_width, float window_height, HINSTANCE instance_handle, WNDPROC window_proc)
	{
		myWindowSize.myHeight = window_height;
		myWindowSize.myWidth = window_width;

		WindowCreateInfo window_create_info;
		window_create_info.window_height = window_height;
		window_create_info.window_width = window_width;
		window_create_info.window_process = window_proc;
		window_create_info.instance = instance_handle;
		m_Window.Initiate(window_create_info);
		m_Window.ShowWindow();
		myHWND = m_Window.GetHWND();
	
		if(!m_Window.IsWindowActive())
			m_Window.OnActive();
		SetWindowText(myHWND, "Snowblind Engine");
#ifdef SNOWBLIND_DX11
		myAPI = new DirectX11;
		const char* api_name = "DirectX11";
#else
		myAPI = new Vulkan;
		const char* api_name = "Vulkan";
#endif
	
		CreateInfo create_info;
		create_info.m_HWND = myHWND;
		create_info.m_Instance = instance_handle;
		create_info.m_WindowWidth = window_width;
		create_info.m_WindowHeight = window_height;
		create_info.m_APIName = api_name;
	
		DL_ASSERT_EXP(myAPI->Initiate(create_info), "Engine : Failed to initiate graphicsAPI");
	
		myAssetsContainer = new Cache::CAssetsContainer;
		myAssetsContainer->Initiate();
	
		m_TerrainManager = new Cache::TerrainManager;
		
		myFontManager = new CFontManager;
		myFontManager->Initiate();
	
		mySynchronizer = new CSynchronizer;
		DL_ASSERT_EXP(mySynchronizer->Initiate(), "Engine : Failed to Initiate Synchronizer!");
	
		m_DebugSystem.AddDebugMenuItem("Toggle VSync", [&]() 
		{
			ToggleVsync();
		});

		myCamera = new Snowblind::CCamera(myWindowSize.myWidth, myWindowSize.myHeight);
		my2DCamera = new Snowblind::CCamera(myWindowSize.myWidth, myWindowSize.myHeight, CU::Vector3f(0, 0, 0.f));
		myRenderer = new CRenderer;
		DL_ASSERT_EXP(myRenderer->Initiate(mySynchronizer, myCamera, my2DCamera), "Engine : Failed to initiate Renderer!");
	
		myTimeManager = new CU::TimeManager;
		Randomizer::Create();
	
	
		m_Threadpool.Initiate();



		m_IsInitiated = true;
		return true;
	}

	bool CEngine::CleanUp()
	{
		m_Threadpool.CleanUp();
		SAFE_DELETE(myAssetsContainer);
		SAFE_DELETE(mySynchronizer);

		m_TerrainManager->CleanUp();
		SAFE_DELETE(m_TerrainManager);

		myRenderer->CleanUp();
		SAFE_DELETE(myRenderer);

		SAFE_DELETE(myCamera);
		SAFE_DELETE(myFontManager);
		SAFE_DELETE(myTimeManager);

		Randomizer::Destroy();

		DL_ASSERT_EXP(myAPI->CleanUp(), "Failed to clean up graphics API. Something was not set to null.");
		SAFE_DELETE(myAPI);


		return true;
	}

	CCamera* CEngine::GetCamera()
	{
		return myCamera;
	}

	Snowblind::CCamera* CEngine::Get2DCamera()
	{
		return my2DCamera;
	}

	void CEngine::Update()
	{
		m_DeltaTime = myTimeManager->GetDeltaTime();
		myAssetsContainer->Update();
		myTimeManager->Update();
		myRenderer->Render();
		m_Threadpool.Update();
		m_DebugSystem.Update();
	}

	void CEngine::Render()
	{
		m_DebugSystem.Render();
	}

	void CEngine::CompileShaderFromFile(const std::string& file_path, const std::string& shader_type, const std::string& feature_level, s32 shader_flags, IBlob*& out_compiled_shader, IBlob*& out_compile_message)
	{
		std::wstring w_file_path(file_path.begin(), file_path.end());
		HRESULT hr = D3DCompileFromFile(
			w_file_path.c_str(),
			NULL,
			NULL,
			shader_type.c_str(),
			feature_level.c_str(), 
			shader_flags, 
			NULL, 
			&out_compiled_shader, 
			&out_compile_message);

		GetAPI()->HandleErrors(hr, "Failed to compile shader!");
	}

	void CEngine::Present()
	{
		if (myInstance->myUsingVSync)
			myAPI->Present(1, 0);
		else
			myAPI->Present(0, 0);
	}

	void CEngine::Clear()
	{
		myAPI->Clear();
	}

	void CEngine::EnableZ()
	{
		myAPI->EnableZBuffer();
	}

	void CEngine::DisableZ()
	{
		myAPI->DisableZBuffer();
	}

	void CEngine::ToggleWireframe()
	{
		myInstance->myRenderer->ToggleWireframe();
	}

	SWindowSize CEngine::GetWindowSize() const
	{
		return myWindowSize;
	}

	CFont* CEngine::LoadFont(const s8* aFilepath, u16 aFontWidth, u16 aBorderWidth)
	{
		return myFontManager->LoadFont(aFilepath, aFontWidth, aBorderWidth);
	}

	float CEngine::GetDeltaTime()
	{
		return m_DeltaTime;
	}

	float CEngine::GetFPS()
	{
		return myTimeManager->GetFPS();
	}

	float CEngine::GetFrameTime()
	{
		return myTimeManager->GetFrameTime();
	}

	std::string CEngine::GetAPIName()
	{
		return myAPI->GetAPIName();
	}

	Snowblind::CTexture* CEngine::GetTexture(const std::string& aFilePath)
	{
		return myAssetsContainer->GetTexture(aFilePath);
	}

	Snowblind::CEffect* CEngine::GetEffect(const std::string& aFilePath)
	{
		return myAssetsContainer->GetEffect(aFilePath);
	}

	Snowblind::CModel* CEngine::GetModel(const std::string& aFilePath)
	{
		return myAssetsContainer->GetModel(aFilePath);
	}

	const std::string& CEngine::LoadModel(const std::string& aFilePath, const std::string& effect)
	{
		//m_Threadpool.AddWork(Work([&]() {
		return myAssetsContainer->LoadModel(aFilePath, effect);
	/*	}));
		return aFilePath;*/
	}

	std::string string_together(u16 time, u16 to_compare)
	{
		std::string to_return;
		to_return += (time < to_compare ? ("0" + std::to_string(time)) : std::to_string(time));
		return to_return;
	}

	std::string CEngine::GetLocalTimeAsString()
	{
		GetLocalTime();
		std::string return_value = "Local Time : ";
		return_value += string_together(myLocalTime.hour, 10) + ":";
		return_value += string_together(myLocalTime.minute, 10) + ":";
		return_value += string_together(myLocalTime.second, 10);
		return return_value;
	}

	void CEngine::ResetRenderTargetAndDepth()
	{
#ifdef SNOWBLIND_DX11
		GetAPI()->ResetRenderTargetAndDepth();
#endif
	}

	void CEngine::ToggleVsync()
	{
		myUsingVSync = !myUsingVSync;
	}

	void CEngine::OnAltEnter()
	{
		if (myAPI)
			myAPI->OnAltEnter();
	}

	void CEngine::OnPause()
	{
		myTimeManager->Pause();
	}

	void CEngine::OnResume()
	{
		myTimeManager->Start();
	}

	void CEngine::OnExit()
	{
		if (HasInitiated())
		{
			mySynchronizer->Quit();
			CleanUp();
		}
	}

	void CEngine::OnInactive()
	{
		if (HasInitiated())
			m_Window.OnInactive();
	}

	void CEngine::OnActive()
	{
		if (HasInitiated())
			m_Window.OnActive();
	}

	CSynchronizer* CEngine::GetSynchronizer()
	{
		return mySynchronizer;
	}

	const SLocalTime& CEngine::GetLocalTime()
	{
		SYSTEMTIME time;
		::GetLocalTime(&time);
		myLocalTime.hour = time.wHour;
		myLocalTime.minute = time.wMinute;
		myLocalTime.second = time.wSecond;

		return myLocalTime;
	}

	Snowblind::CTerrain* CEngine::CreateTerrain(const std::string& aFile, const CU::Vector3f& position, const CU::Vector2f& aSize)
	{
		CTerrain* newTerrain = m_TerrainManager->GetTerrain(aFile);
		newTerrain->Initiate(aFile, position, aSize);
		myRenderer->AddTerrain(newTerrain);
		return newTerrain;
	}

	Threadpool& CEngine::GetThreadpool()
	{
		return m_Threadpool;
	}

	void CEngine::ToggleDebugMenu()
	{
		m_DebugSystem.GetDebugMenuIsActive() ? m_DebugSystem.DeactivateDebugMenu() : m_DebugSystem.ActivateDebugMenu();
	}

	void CEngine::AddError(const std::string& error_message)
	{
		m_DebugSystem.AddToErrorList(error_message);
	}

	void CEngine::AddDebugText(const std::string& debug_text)
	{
		m_DebugSystem.AddToDebugText(debug_text);
	}

	};