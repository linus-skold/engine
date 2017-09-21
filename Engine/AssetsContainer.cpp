#include "stdafx.h"
#include "AssetsContainer.h"
#include "Model.h"
#include "Texture.h"
#include "Effect.h"
#include "ShaderFactory.h"
#include "Texture.h"
#include <Engine/Sprite.h>

AssetsContainer::~AssetsContainer()
{
	SAFE_DELETE(m_ShaderFactory);
	SAFE_DELETE(m_ModelLoader);
	
	for (auto it = m_Models.begin(); it != m_Models.end(); it++)
		SAFE_DELETE(it->second);

	for (auto it = m_Textures.begin(); it != m_Textures.end(); it++)
		SAFE_DELETE(it->second);

	for (auto it = m_Sprites.begin(); it != m_Sprites.end(); it++)
		SAFE_DELETE(it->second);

	for (auto it = m_Effects.begin(); it != m_Effects.end(); it++)
		SAFE_DELETE(it->second);


}

void AssetsContainer::Initiate()
{
	m_ShaderFactory = new ShaderFactory;
	m_ModelLoader = new CModelImporter;
	m_Textures.empty();
	m_Effects.empty();
	m_Models.empty();
	m_Sprites.empty();

#ifndef FINAL
	for (s32 i = 0; i < m_Watchers.Capacity(); i++)
	{
		m_Watchers[i] = new FileWatcher;
	}
#endif
}

Texture* AssetsContainer::GetTexture(const cl::HashString& path)
{

	
	if (path.contains(".dds") == false)
	{
		DL_MESSAGE("Failed to load %s, due to incorrect fileformat. Has to be .dds", path.c_str());
		DL_ASSERT("Failed to Load Texture, format not .dds. See log for more information.");
		return nullptr;
	}

	//mutex?
	// TODO fix this shit
	/*if (myTextures.find(filepath) == myTextures.end())
	{*/
		//myTextures.emplace(filepath, new Texture);
/*
		m_Engine->GetInstance()->GetThreadpool().AddWork(Work([=]() {

			LoadTexture(filepath);

		}));*/


		if (!LoadTexture(path))
			return nullptr;
		//DL_MESSAGE("Successfully loaded : %s", filepath.c_str());
	//}

	return m_Textures[path];
}



Effect* AssetsContainer::GetEffect(const cl::HashString& path)
{
	if (m_Effects.find(path) == m_Effects.end())
	{
		LoadEffect(path);
	}
	return m_Effects[path];
}

Model* AssetsContainer::GetModel(const cl::HashString& path)
{
	auto it = m_Models.find(path);
	if (it == m_Models.end())
	{
		DL_MESSAGE("Requested Model : %s", path.c_str());
		DL_ASSERT("Failed to find requested model. Did you enter the correct path?");
	}
	return it->second;
}

void AssetsContainer::Update()
{
	m_ShaderFactory->Update();
	for (FileWatcher* watcher : m_Watchers)
	{
		watcher->FlushChanges();
	}
}

void AssetsContainer::ReloadTexture(Texture* texture)
{
	//texture->OnReload();
}

bool AssetsContainer::LoadTexture(const cl::HashString& filepath)
{
	BeginTicketMutex(&m_Mutex);
	if (m_Textures.find(filepath) == m_Textures.end())
	{
		Texture* texture = new Texture;
		/*if (texture->Load(filepath) == false)
		{
			SAFE_DELETE(texture);
			EndTicketMutex(&m_Mutex);
			return false;
		}*/
		m_Textures[filepath] = texture;
	}
	EndTicketMutex(&m_Mutex);
	return true;
}

Effect* AssetsContainer::LoadEffect(const cl::HashString& filepath)
{
	Effect* effect = new Effect(filepath.c_str());
	m_ShaderFactory->LoadShader(effect);
	m_Effects[filepath] = effect;
	return m_Effects[filepath];
}

Sprite* AssetsContainer::LoadSprite(const cl::HashString& path)
{
	Sprite* sprite = new Sprite;
	m_Sprites.emplace(path.GetHash(), sprite);
	sprite->Initiate(path);
	return sprite;
}

Sprite* AssetsContainer::GetSprite(const cl::HashString& path)
{
	for (auto it = m_Sprites.begin(); it != m_Sprites.end(); it++)
	{
		if (it->first != path)
			continue;

		return it->second;
	}
	return LoadSprite(path);
}


cl::HashString AssetsContainer::LoadModel(const cl::HashString& filepath, std::string effect, bool thread)
{
// 	if (filepath.find("default_cube") != filepath.npos)
// 	{
// 		Model* model = new Model;
// 		model->SetEffect(LoadEffect(Engine::GetInstance()->GetVFS().GetFile(effect)));
// 		model->CreateCube();
// 		m_Models.emplace("default_cube", model);
// 
// 	}



	if (m_Models.find(filepath) == m_Models.end())
	{
		DL_MESSAGE("Loading model : %s", filepath.c_str());
		Model* model = new Model;
		m_Models.emplace(filepath, model);

		if ( thread )
		{
			Engine::GetInstance()->GetThreadpool().AddWork(Work([=]() {
				m_ModelLoader->LoadModel(model, filepath.c_str(), effect);
				model->Initiate(filepath.c_str());
			}));
			return filepath;
		}
		else
		{
			m_ModelLoader->LoadModel(model, filepath.c_str(), effect);
			model->Initiate(filepath.c_str());
			return filepath;
		}
	}
	return filepath;
}
