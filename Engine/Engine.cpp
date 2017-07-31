#include "stdafx.h"
#include <Engine/Vulkan.h>
#include "AssetsContainer.h"

#include "Renderer.h"
#include "Synchronizer.h"

#include "Terrain.h"
#include "TerrainManager.h"

#include "IGraphicsAPI.h"
#include <d3dcompiler.h>

#include "EditObject.h"

#include "LevelFactory.h"

#include <Input/InputHandle.h>
#include <PostMaster.h>
#include <PhysicsManager.h>

#include <DebugComponent.h>

#include <DebugSystem.h>
#include <PhysicsSystem.h>
#include <RenderSystem.h>
#include <LightSystem.h>
#include <InputSystem.h>
#include <AISystem.h>
#include <NetworkSystem.h>
#include <CameraSystem.h>

#include <Engine/LightModel.h>

#if !defined(_PROFILE) && !defined(_FINAL)
#include "imgui_impl_dx11.h"
#endif
#ifdef _PROFILE
#include <easy/profiler.h>
#include <easy/reader.h>
#endif

static constexpr char* vertex_shader = "VS";
static constexpr char* pixel_shader = "PS";
static constexpr char* geometry_shader = "GS";
static constexpr char* hull_shader = "HS";
static constexpr char* domain_shader = "DS";
static constexpr char* compute_shader = "CS";


#define REGISTERCOMPONENT(x) x,
enum RegisteredComponents
{
#include "Components.h"
};
#undef REGISTERCOMPONENT

#define REGISTERCOMPONENT(x) #x,
const char* RegisteredComponentsStr[] = {
#include "Components.h"
};
#undef REGISTERCOMPONENT



bool Engine::HasInitiated()
{
	return (this && m_States[(u16)eEngineStates::INITIATED] == TRUE);
}

Engine* Engine::myInstance = nullptr;
IGraphicsAPI* Engine::myAPI = nullptr;

void Engine::Create()
{
	DL_ASSERT_EXP(myInstance == nullptr, "Instance already created!");
	myInstance = new Engine;
}

void Engine::Destroy()
{
	DL_ASSERT_EXP(myInstance != nullptr, "Can't destroy the instance before it's created. Did you call Destroy twice?");
	SAFE_DELETE(myInstance);
}

Engine* Engine::GetInstance()
{
	DL_ASSERT_EXP(myInstance != nullptr, "Can't Get the instance before it's created. Did you call Destroy twice?");
	return myInstance;
}

DirectX11* Engine::GetAPI()
{
	return static_cast<DirectX11*>(myAPI);
}

/*bool Engine::InitiateDebugSystem(InputHandle* input_handle)
{
	m_DebugSystem.Initiate(input_handle);
	return true;
}*/

bool Engine::Initiate(float window_width, float window_height, HINSTANCE instance_handle, WNDPROC window_proc)
{
	Randomizer::Create();
	PostMaster::Create();
	//myWindowSize.m_Height = window_height;
	//myWindowSize.m_Width = window_width;

	WindowCreateInfo window_create_info;
	window_create_info.window_height = window_height;
	window_create_info.window_width = window_width;
	window_create_info.window_process = window_proc;
	window_create_info.instance = instance_handle;
	m_Window.Initiate(window_create_info);
	m_Window.ShowWindow();
	myHWND = m_Window.GetHWND();
	DL_ASSERT_EXP(myHWND, "Window Handle was null!");


	if (!m_Window.IsWindowActive())
		m_Window.OnActive();
	SetWindowText(myHWND, "engine");

	myAPI = new DirectX11;
	const char* api_name = "DirectX11";


	CreateInfo create_info;
	create_info.m_HWND = myHWND;
	create_info.m_Instance = instance_handle;
	create_info.m_WindowWidth = m_Window.GetInnerSize().m_Width;
	create_info.m_WindowHeight = m_Window.GetInnerSize().m_Height;
	create_info.m_APIName = api_name;
	DL_ASSERT_EXP(myAPI->Initiate(create_info), "Engine : Failed to initiate graphicsAPI");

	m_InputHandle = new InputHandle;
	m_InputHandle->Initiate(myHWND, instance_handle);
	m_InputHandle->AddController(0);


	myAssetsContainer = new AssetsContainer;
	myAssetsContainer->Initiate();

	m_TerrainManager = new TerrainManager;

	myFontManager = new CFontManager;
	myFontManager->Initiate();

	mySynchronizer = new Synchronizer;
	DL_ASSERT_EXP(mySynchronizer->Initiate(), "Engine : Failed to Initiate Synchronizer!");

	//m_SegmentHandle.Initiate(mySynchronizer->Get)
	m_SegmentHandle.Initiate();


	m_Camera = new Camera;
	m_Camera->CreatePerspectiveProjection(m_Window.GetInnerSize().m_Width, m_Window.GetInnerSize().m_Height, 0.01f, 10000.f, 90.f);
	m_Camera->CreateOrthogonalProjection(m_Window.GetInnerSize().m_Width, m_Window.GetInnerSize().m_Height, 0.01f, 100.f);

	myRenderer = new Renderer;
	DL_ASSERT_EXP(myRenderer->Initiate(mySynchronizer, m_Camera), "Engine : Failed to initiate Renderer!");


	m_PhysicsManager = new PhysicsManager;

	m_Threadpool.Initiate("Engine - Worker");



	m_EntityManager.Initiate();

	m_EntityManager.AddSystem<::DebugSystem>(); //Since the engine has it's own debug system, I had to do it like this

#ifndef _EDITOR
	m_EntityManager.AddSystem<PhysicsSystem>();
#endif
	m_EntityManager.AddSystem<RenderSystem>();
	m_EntityManager.AddSystem<LightSystem>();
	//m_EntityManager.AddSystem<InputSystem>();
	//m_EntityManager.AddSystem<NetworkSystem>();
#ifndef _EDITOR
	//m_EntityManager.AddSystem<AISystem>();
#endif 

	//m_EntityManager.AddSystem<CameraSystem>();

#if !defined(_PROFILE) && !defined(_FINAL)
	ImGui_ImplDX11_Init(myHWND, GetAPI()->GetDevice(), GetAPI()->GetContext());
#endif

	m_LevelFactory = new LevelFactory;
	m_LevelFactory->Initiate();


	m_Levels.push_back("Data/Levels/level_01.level");
	m_Levels.push_back("Data/Levels/level_02.level");
	m_Levels.push_back("Data/Levels/level_03.level");

	m_States[(u16)eEngineStates::INITIATED] = TRUE;
	return true;
}

bool Engine::CleanUp()
{

#if !defined(_PROFILE) && !defined(_FINAL)
	ImGui_ImplDX11_Shutdown();
#endif
	m_EntityManager.CleanUp();

	m_InputHandle->CleanUp();
	SAFE_DELETE(m_InputHandle);
	if (m_InputHandle)
		return false;

	m_Threadpool.CleanUp();

	m_TerrainManager->CleanUp();
	SAFE_DELETE(m_TerrainManager);

	myRenderer->CleanUp();
	SAFE_DELETE(myRenderer);

	SAFE_DELETE(m_Camera);
	SAFE_DELETE(myFontManager);


	SAFE_DELETE(m_PhysicsManager);

	SAFE_DELETE(m_LevelFactory);
	SAFE_DELETE(myAssetsContainer);
	SAFE_DELETE(mySynchronizer);
	DL_ASSERT_EXP(myAPI->CleanUp(), "Failed to clean up graphics API. Something was not set to null.");
	SAFE_DELETE(myAPI);
	PostMaster::Destroy();
	Randomizer::Destroy();
	m_States[(u16)eEngineStates::INITIATED] = FALSE;
	return true;
}

Camera* Engine::GetCamera()
{
	return m_Camera;
}

TreeDweller* Engine::CreateEntity(const std::string& filepath, CU::Vector3f& position)
{
	return m_LevelFactory->CreateEntitiy(filepath, position);
}

void Engine::Update()
{
	if (!HasInitiated())
		return;

#if !defined(_PROFILE) && !defined(_FINAL)
	UpdateDebugUI();
#endif
	m_DeltaTime = myTimeManager.GetDeltaTime();
	if (m_States[(u16)eEngineStates::LOADING] == FALSE)
	{
		myTimeManager.Update();
		myAssetsContainer->Update();
	}
	myRenderer->Render();
	m_PhysicsManager->Update();
	m_Threadpool.Update();

}

void Engine::UpdateInput()
{
	if (!m_PauseInput || m_Window.IsWindowActive())
		m_InputHandle->Update(m_DeltaTime);
}

int Engine::RegisterLight()
{
	return myRenderer->RegisterLight();
}

IGraphicsAPI* Engine::GetGraphicsAPI()
{
	return myAPI;
}

HRESULT Engine::CompileShaderFromFile(const std::string& file_path, const std::string& entrypoint, const std::string& feature_level, s32 shader_flags, IBlob*& out_compiled_shader, IBlob*& out_compile_message)
{
	std::wstring w_file_path(file_path.begin(), file_path.end());
	HRESULT hr = D3DCompileFromFile(
		w_file_path.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entrypoint.c_str(),
		feature_level.c_str(),
		shader_flags,
		0,
		&out_compiled_shader,
		&out_compile_message);
	return hr;
}

HRESULT Engine::CompileShaderFromMemory(const s8* pData, s32 size, const std::string& source_name, const std::string& entrypoint, const std::string& feature_level, s32 shader_flags, IBlob*& out_shader, IBlob* out_message)
{
	HRESULT hr = D3DCompile(
		pData,
		size,
		source_name.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entrypoint.c_str(),
		feature_level.c_str(),
		0,
		0,
		&out_shader,
		&out_message);
	return hr;
}

void* Engine::CreateShader(IBlob* compiled_shader_blob, eShaderType type, const std::string& debug_name)
{
	switch (type)
	{
		case eShaderType::VERTEX:
			return myAPI->CreateVertexShader(compiled_shader_blob->GetBufferPointer(), compiled_shader_blob->GetBufferSize());
		case eShaderType::PIXEL:
			return myAPI->CreatePixelShader(compiled_shader_blob->GetBufferPointer(), compiled_shader_blob->GetBufferSize());
		case eShaderType::GEOMETRY:
			return myAPI->CreateGeometryShader(compiled_shader_blob->GetBufferPointer(), compiled_shader_blob->GetBufferSize());
		case eShaderType::HULL:
			return myAPI->CreateHullShader(compiled_shader_blob->GetBufferPointer(), compiled_shader_blob->GetBufferSize());
		case eShaderType::DOMAINS:
			return myAPI->CreateDomainShader(compiled_shader_blob->GetBufferPointer(), compiled_shader_blob->GetBufferSize());
		case eShaderType::COMPUTE:
			return myAPI->CreateComputeShader(compiled_shader_blob->GetBufferPointer(), compiled_shader_blob->GetBufferSize());
		default:
			DL_ASSERT("No valid shader type!");
	}

	return nullptr;
}

//CompiledShader Engine::CreateShader(IBlob* compiled_shader_blob, const std::string& shader_type, const std::string& debug_name, bool use)
//{
//	CompiledShader compiled_shader;
//
//	compiled_shader.m_Shader = CreateShader(compiled_shader_blob, shader_type, debug_name);
//	compiled_shader.blob = compiled_shader_blob;
//	compiled_shader.shaderSize = compiled_shader_blob->GetBufferSize();
//	compiled_shader.compiledShader = compiled_shader_blob->GetBufferPointer();
//
//	return compiled_shader;
//}


void Engine::SelectEntity(u32 e)
{
	m_EntityToEdit = e;
	m_IsEditingEntity = true;
}

void Engine::DeselectEntity()
{
	m_EntityToEdit = 0;
	m_IsEditingEntity = false;
}

void Engine::Present()
{
	if (myInstance->m_States[(u16)eEngineStates::USE_VSYNC] == TRUE)
		myAPI->Present(1, 0);
	else
		myAPI->Present(0, 0);
}

void Engine::Clear()
{
	myAPI->Clear();
}

void Engine::EnableZ()
{
	myAPI->EnableZBuffer();
}

void Engine::DisableZ()
{
	myAPI->DisableZBuffer();
}

const WindowSize& Engine::GetWindowSize() const
{
	return m_Window.GetWindowSize();
}

const WindowSize& Engine::GetInnerSize() const
{
	return m_Window.GetInnerSize();
}

CFont* Engine::LoadFont(const s8* aFilepath, u16 aFontWidth, u16 aBorderWidth)
{
	return myFontManager->LoadFont(aFilepath, aFontWidth, aBorderWidth);
}

float Engine::GetDeltaTime()
{
	return m_DeltaTime;
}

float Engine::GetTotalTime()
{
	return myTimeManager.GetMasterTimer().GetTotalTime().GetSeconds();
}

float Engine::GetFPS()
{
	return myTimeManager.GetFPS();
}

float Engine::GetFrameTime()
{
	return myTimeManager.GetFrameTime();
}

std::string Engine::GetAPIName()
{
	return myAPI->GetAPIName();
}

VirtualFileSystem& Engine::GetVFS()
{
	return m_VirtualFileSystem;
}

Texture* Engine::GetTexture(const std::string& aFilePath)
{
	return myAssetsContainer->GetTexture(aFilePath);
}

Effect* Engine::GetEffect(const std::string& aFilePath)
{
	return myAssetsContainer->GetEffect(m_VirtualFileSystem.GetFile(aFilePath));
}

Model* Engine::GetModel(const std::string& aFilePath)
{
	return myAssetsContainer->GetModel(aFilePath);
}

std::string Engine::LoadModel(std::string aFilePath, std::string effect, bool thread)
{
	myAssetsContainer->LoadModel(aFilePath, effect, thread);
	return aFilePath;
}

std::string string_together(u16 time, u16 to_compare)
{
	std::string to_return;
	to_return += (time < to_compare ? ("0" + std::to_string(time)) : std::to_string(time));
	return to_return;
}

std::string Engine::GetLocalTimeAsString()
{
	GetLocalTime();
	std::string return_value = "Local Time : ";
	return_value += string_together(myLocalTime.hour, 10) + ":";
	return_value += string_together(myLocalTime.minute, 10) + ":";
	return_value += string_together(myLocalTime.second, 10);
	return return_value;
}

void Engine::ResetRenderTargetAndDepth()
{
	GetAPI()->ResetRenderTargetAndDepth();
}

void Engine::ToggleVsync()
{
	m_States[(u16)eEngineStates::USE_VSYNC] = !m_States[(u16)eEngineStates::USE_VSYNC];
}

void Engine::OnAltEnter()
{
	if (myAPI)
		myAPI->OnAltEnter();
}

void Engine::OnPause()
{
	myTimeManager.Pause();
}

void Engine::OnResume()
{
	myTimeManager.Start();
}

void Engine::OnExit()
{
	if (HasInitiated())
	{
		mySynchronizer->Quit();
		CleanUp();
	}
}

void Engine::OnInactive()
{
	if (HasInitiated())
		m_Window.OnInactive();
}

void Engine::OnActive()
{
	if (HasInitiated())
		m_Window.OnActive();
}

void Engine::OnResize()
{
	if (myAPI)
		myAPI->OnResize();
}

Synchronizer* Engine::GetSynchronizer()
{
	return mySynchronizer;
}

const SLocalTime& Engine::GetLocalTime()
{
	SYSTEMTIME time;
	::GetLocalTime(&time);
	myLocalTime.hour = time.wHour;
	myLocalTime.minute = time.wMinute;
	myLocalTime.second = time.wSecond;

	return myLocalTime;
}

Terrain* Engine::CreateTerrain(std::string aFile, CU::Vector3f position, CU::Vector2f aSize)
{
	Terrain* newTerrain = m_TerrainManager->GetTerrain(aFile);
	newTerrain->Initiate(aFile, position, aSize);
	myRenderer->AddTerrain(newTerrain);
	return newTerrain;
}

CU::GrowingArray<TreeDweller*> Engine::LoadLevel(const std::string& level_filepath)
{

	m_States[(u16)eEngineStates::LOADING] = TRUE;

	m_LevelFactory->CreateLevel(level_filepath);

	m_States[(u16)eEngineStates::LOADING] = FALSE;

	return m_LevelFactory->GetDwellers();
}

#if !defined(_PROFILE) && !defined(_FINAL)

bool Engine::SaveLevel()
{
	static char file_name[512];
	if (ImGui::Begin("Filename"))
	{

		ImGui::InputText("filename", file_name, 512);
		if (ImGui::Button("Save", ImVec2(100, 30)))
		{
			if (!CL::substr(file_name, ".level"))
			{
				DL_WARNINGBOX("Incorrect filetype, no file created!");
			}
			else
			{
				std::ofstream f(file_name);
				f << "Hello World!";
				f.flush();
				f.close();
				m_PauseInput = false;
				ImGui::End();
				return true;
			}
		}

	}
	ImGui::End();

	return false;
}

static auto GetVector = [](void* vec, int index, const char** out_text)
{
	auto& vector = *static_cast<std::vector<std::string>*>(vec);
	if (index < 0 || index >= static_cast<int>(vector.size()))
		return false;
	*out_text = vector.at(index).c_str();

	return true;
};

bool ListBox(const char* label, int* current_index, std::vector<std::string>& values)
{
	if (values.empty())
		return false;

	return ImGui::ListBox(label, current_index, GetVector, static_cast<void*>(&values), values.size(), values.size());
}


static void DrawTexture(const ImDrawList* list, const ImDrawCmd* cmd)
{
	ImVec2 w_size = ImGui::GetWindowSize();
	w_size.x *= 0.65f;
	w_size.y = w_size.x / 1.777777777777777777777777777777778;

	//list->PushTextureID(0);


	//Draw a quad that is adapted to the window.
}

void Engine::DebugTextures()
{
	static bool pOpen = false;
	//ImGui::PushStyleVar(AutoSize)
	if (ImGui::Begin("textures", &pOpen, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static s32 index = 0;
		ImGui::PushItemWidth(250.f);
		ListBox("", &index, m_Labels);
		ImGui::PopItemWidth();

		ImTextureID tex_id = m_DebugTextures[index];
		ImVec2 w_size = ImGui::GetWindowSize();
		w_size.x *= 0.65f;
		w_size.y = w_size.x / 1.777777777777777777777777777777778;
		ImGui::SameLine();
		ImGui::Image(tex_id, w_size, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		draw_list->AddCallback(DrawTexture, nullptr);

	}
	ImGui::End();

}

void Engine::AddTexture(Texture* texture, const std::string& debug_name)
{
	AddTexture(texture->GetShaderView(), debug_name);
}

void Engine::AddTexture(void* srv, const std::string& debug_name)
{
	m_DebugTextures.Add(static_cast<ID3D11ShaderResourceView*>(srv));
	m_Labels.push_back(debug_name);
}

void Engine::AddFunction(const std::string& label, std::function<void()> function)
{
	m_Functions.push_back(std::make_pair(label, function));
}


void Engine::UpdateDebugUI()
{
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(300, GetWindowSize().m_Height));

	ImGuiWindowFlags flags = 0;

	flags |= ImGuiWindowFlags_NoTitleBar;
	flags |= ImGuiWindowFlags_NoResize;
	//flags |= ImGuiWindowFlags_NoCollapse;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	static bool pOpen = false;
	static bool new_window = false;
	static bool save = false;
	static bool debug_textures = false;

	static bool entity_options = false;

	if (ImGui::Begin("Information", &pOpen, flags))
	{
		ImGui::Text("Delta Time : %.3f", GetDeltaTime());
		ImGui::Text("FPS : %.1f", GetFPS());
		ImGui::Text("CPU Usage : %.1f", m_SystemMonitor.GetCPUUsage());
		ImGui::Text("Memory Usage : %dmb", m_SystemMonitor.GetMemoryUsage());

		//ImGui::Text("Model Commands : %d", mySynchronizer->GetRenderCommands(eBufferType::MODEL_BUFFER).Size());
		ImGui::Text("Model Commands : %d", m_SegmentHandle.CommandSize((s32)mySynchronizer->GetCurrentBufferIndex()));

		ImGui::Text("Spotlight Commands : %d", mySynchronizer->GetRenderCommands(eBufferType::SPOTLIGHT_BUFFER).Size());
		ImGui::Text("Pointlight Commands : %d", mySynchronizer->GetRenderCommands(eBufferType::POINTLIGHT_BUFFER).Size());
		ImGui::Text("Particle Commands : %d", mySynchronizer->GetRenderCommands(eBufferType::PARTICLE_BUFFER).Size());
		ImGui::Text("Sprite Commands : %d", mySynchronizer->GetRenderCommands(eBufferType::SPRITE_BUFFER).Size());
		ImGui::Text("Text Commands : %d", mySynchronizer->GetRenderCommands(eBufferType::TEXT_BUFFER).Size());
		ImGui::Text("Line Commands : %d", mySynchronizer->GetRenderCommands(eBufferType::LINE_BUFFER).Size());

		ImGui::Separator();


		if (ImGui::TreeNodeEx("Entity", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_NoTreePushOnOpen))
		{
			if (ImGui::Button("New Entity", ImVec2(100.f, 25.f)))
			{
				new_window = !new_window;
			}
			ImGui::Separator();
		}

		if (ImGui::TreeNodeEx("Post-Processing", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_NoTreePushOnOpen))
		{
			static bool tonemapping_hdr = true;
			static bool bloom = true;
			static bool ssao = true;
			static bool motion_blur = true;
			static bool depth_of_field = true;

			ImGui::Columns(2, "", true);
			ImGui::Checkbox("Tonemapping/HDR", &tonemapping_hdr);
			ImGui::Checkbox("Motion Blur", &motion_blur);
			ImGui::Checkbox("Depth of Field", &depth_of_field);
			ImGui::NextColumn();
			ImGui::Checkbox("Bloom", &bloom);
			ImGui::Checkbox("SSAO", &ssao);


			ImGui::Columns(1);

			if (tonemapping_hdr)
				myRenderer->GetPostprocessManager().SetPassesToProcess(PostProcessManager::HDR);
			else
				myRenderer->GetPostprocessManager().RemovePassToProcess(PostProcessManager::HDR);

			if (depth_of_field)
			{
				ImGui::Text("Depth of Field settings");
				float fov_value = 0.f;
				ImGui::SliderFloat("Strength", &fov_value, 0.f, 9999.f, "%.f");

			}


			ImGui::Separator();
		}




		/*if (ImGui::Button("Save Level", ImVec2(100.f, 25.f)))
		{
			save = !save;
			m_PauseInput = true;
		}*/

		if (save) if (SaveLevel()) save = !save;


		ImGui::Checkbox("Debug Textures", &debug_textures);
		//ImGui::Checkbox("Debug Textures", &myRenderer->GetPostprocessManager().GetHDRPass().toggle_debug);
		if (debug_textures)
			DebugTextures();


		/*static bool render_lines = false;
		ImGui::Checkbox("Render Lines", &render_lines);
		myRenderer->SetRenderLines(render_lines);
		ImGui::SameLine();
		ImGui::Checkbox("Instance Models", &m_RenderInstanced);*/


		float fov_value = m_Camera->GetFOV();
		ImGui::SliderFloat("FOV", &fov_value, 60.f, 120.f, "%.f");
		m_Camera->SetFOV(fov_value);



		if (ImGui::TreeNodeEx("Directional Light", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_NoTreePushOnOpen))
		{
			/*
				Need some kind of color picker / wheel instead

			*/

			/* Should be read from the level file */
			static int r = 255;
			static int g = 255;
			static int b = 255;
			static int a = 255;

			static ImVec4 r_color;
			static ImVec4 g_color;
			static ImVec4 b_color;
			static ImVec4 a_color;
			static ImVec4 tot_color;


			ImGui::Text("Directional Light Color");
			ImGui::SliderInt("R", &r, 0, 255);
			r_color = ImVec4((float)r / 255.f, 0, 0, 1);
			ImGui::SameLine();
			ImGui::ColorButton(r_color);


			ImGui::SliderInt("G", &g, 0, 255);
			g_color = ImVec4(0, (float)g / 255.f, 0, 1);
			ImGui::SameLine();
			ImGui::ColorButton(g_color);


			ImGui::SliderInt("B", &b, 0, 255);
			b_color = ImVec4(0, 0, (float)b / 255.f, 1);
			ImGui::SameLine();
			ImGui::ColorButton(b_color);

			ImGui::SliderInt("Intensity", &a, 0, 255);
			a_color = ImVec4((float)a / 255.f, (float)a / 255.f, (float)a / 255.f, 1);
			ImGui::SameLine();
			ImGui::ColorButton(a_color);


			tot_color = ImVec4((float)r / 255.f, (float)g / 255.f, (float)b / 255.f, (float)a / 255.f);
			ImGui::ColorButton(tot_color);


			myRenderer->GetDeferredRenderer()->SetColor(CU::Vector4f((float)r / 255.f, (float)g / 255.f, (float)b / 255.f, (float)a / 255.f));


			ImGui::Text("Directional Light Direction");
			static int x = 0;
			static int y = 0;
			static int z = 0;
			ImGui::SliderInt("X", &x, -180, 180);
			ImGui::SliderInt("Y", &y, -180, 180);
			ImGui::SliderInt("Z", &z, -180, 180);

			myRenderer->SetDirection(CU::Vector3f((float)x / 180.f, (float)y / 180.f, (float)z / 180.f));


			ImGui::Text("Directional Shadow Position");
			static float sx = 0;
			static float sy = 0;
			static float sz = 0;
			ImGui::SliderFloat("sX", &sx, -30.f, 360.f);
			ImGui::SliderFloat("sY", &sy, -30.f, 360.f);
			ImGui::SliderFloat("sZ", &sz, -30.f, 360.f);

			myRenderer->GetDirectionalCamera()->SetPosition(CU::Vector3f(sx, sy, sz));





		}



		/*for (CheckBox& box : m_Checkboxes)
		{
			ImGui::Checkbox(box.m_Name.c_str(), box.m_Toggle);
		}*/

		/*static s32 index = 0;
		s32 prev = index;
		ListBox("", &index, m_Levels);
		if (prev != index)
		{
			m_Functions[index].second();
		}*/
		ImGui::End();
	}
	ImGui::PopStyleVar();

	if (new_window)
	{
		ImGui::SetNextWindowPos(ImVec2(300.f, 0.f));
		//ImGui::SetNextWindowSize(ImVec2(300.f, 300.f));

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		if (ImGui::Begin("New Window", &new_window, ImGuiWindowFlags_NoTitleBar))
		{
			ImGui::Text("Entity Creation");
			ImGui::Separator();
			static bool new_bp = false;
			static bool new_entity = false;
			if (ImGui::Button("New Blueprint", ImVec2(100.f, 25.f)))
			{
				new_bp = !new_bp;
			}
			ImGui::SameLine();
			if (ImGui::Button("New Entity", ImVec2(100.f, 25.f)))
			{
				new_entity = !new_entity;
			}




			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Text("Create and Edit a new Blueprint.");
				ImGui::EndTooltip();
			}
			ImGui::Separator();

			if (new_bp)
			{
				static bool new_component = false;
				static const int item_count = ARRAYSIZE(RegisteredComponentsStr) - 1;
				static int components = 0;

				static int selected[item_count] = { 0 };
				static char* str = { "Translation\0Graphics\0Physics\0AI\0Network\0Input" };

				ImGui::BeginChildFrame(0, ImVec2(ImGui::GetWindowSize().x, 200.f));
				static bool edit_t = false;
				if (new_component)
				{
					for (int i = 0; i < components; i++)
					{
						std::stringstream ss;
						ss << "Box_" << i;
						std::stringstream ss2;
						ss2 << "X_" << i;
						std::stringstream ss3;
						ss3 << "Edit_" << i;
						if (ImGui::Button(ss2.str().c_str()))
						{
							for (int j = i; j < components - 1; j++)
							{
								selected[i] = selected[j + 1];
							}
							components--;
						}
						ImGui::SameLine();
						ImGui::Combo(ss.str().c_str(), &selected[i], str);
						ImGui::SameLine();
						if (ImGui::Button(ss3.str().c_str()))
						{
							switch (selected[i])
							{
								case RegisteredComponents::translation:
								{
									edit_t = !edit_t;
								}break;
							}
						}
					}

					ImGui::Separator();
				}

				if (components < item_count)
				{
					if (ImGui::Button("Add Component"))
					{
						if (components < item_count)
							components++;
						new_component = true;
					}
				}

				ImGui::EndChildFrame();

				if (ImGui::Button("Save"))
				{

				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
				{
					new_bp = false;
				}
			}

			if (new_entity)
			{
			}
		}
		ImGui::End();
		ImGui::PopStyleVar();

	}
	EditEntity();
}

bool Engine::GetLineRendering()
{
	return myRenderer->GetRenderLines();
}

void Engine::EditEntity()
{
	if (!m_IsEditingEntity)
		return;

	DebugComponent& debug = m_EntityManager.GetComponent<DebugComponent>(m_EntityToEdit);
	EditObject& to_edit = debug.m_EditObject;

	std::stringstream ss;
	ss << "Entity : " << m_EntityToEdit;
	ImGui::SetNextWindowPos(ImVec2(300.f, 0));
	ImGui::SetNextWindowSize(ImVec2(300.f, 600.f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	if (ImGui::Begin(ss.str().c_str(), &m_IsEditingEntity, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
	{
		to_edit.Update();
		ImGui::End();
	}
	ImGui::PopStyleVar();
}
#endif