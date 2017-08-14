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
	for (auto it = myModels.begin(); it != myModels.end(); it++)
	{
		it->second->CleanUp();
		SAFE_DELETE(it->second);
	}

	for (auto it = myTextures.begin(); it != myTextures.end(); it++)
	{
		SAFE_DELETE(it->second);
	}

	DELETE_MAP(myEffects);
}

void AssetsContainer::Initiate()
{
	m_Engine = Engine::GetInstance();
	m_ShaderFactory = new ShaderFactory;
	m_ModelLoader = new CModelImporter;
	myTextures.empty();
	myEffects.empty();
	myModels.empty();
	m_Sprites.empty();
}

Texture* AssetsContainer::GetTexture(std::string aFilePath)
{
	if (cl::substr(aFilePath, ".dds") == false)
	{
		DL_MESSAGE("Failed to load %s, due to incorrect fileformat. Has to be .dds", aFilePath.c_str());
		DL_ASSERT("Failed to Load Texture, format not .dds. See log for more information.");
		return nullptr;
	}

	//mutex?
	/*if (myTextures.find(aFilePath) == myTextures.end())
	{*/
		//myTextures.emplace(aFilePath, new Texture);
/*
		m_Engine->GetInstance()->GetThreadpool().AddWork(Work([=]() {

			LoadTexture(aFilePath);

		}));*/


		if (!LoadTexture(aFilePath))
			return nullptr;
		//DL_MESSAGE("Successfully loaded : %s", aFilePath.c_str());
	//}

	return myTextures[aFilePath];
}



Effect* AssetsContainer::GetEffect(const std::string& aFilePath)
{
	if (myEffects.find(aFilePath) == myEffects.end())
	{
		LoadEffect(aFilePath);
	}
	return myEffects[aFilePath];
}

Model* AssetsContainer::GetModel(const std::string& aFilePath)
{
	std::unordered_map<std::string, Model*>::iterator it;
	it = myModels.find(aFilePath);

	if (it == myModels.end())
	{
		DL_MESSAGE("Requested Model : %s", aFilePath.c_str());
		DL_ASSERT("Failed to find requested model. Did you enter the correct path?");
	}
	return it->second;
}

void AssetsContainer::Update()
{
	m_ShaderFactory->Update();
}

void AssetsContainer::ReloadTexture(Texture* texture)
{
	texture->OnReload();
}

bool AssetsContainer::LoadTexture(std::string aFilePath)
{
	BeginTicketMutex(&m_Mutex);
	if (myTextures.find(aFilePath) == myTextures.end())
	{
		Texture* texture = new Texture;
		if (texture->Load(aFilePath.c_str()) == false)
		{
			SAFE_DELETE(texture);
			EndTicketMutex(&m_Mutex);
			return false;
		}
		myTextures[aFilePath] = texture;
	}
	EndTicketMutex(&m_Mutex);
	return true;
}

Effect* AssetsContainer::LoadEffect(const std::string& aFilePath)
{
	Effect* effect = new Effect(aFilePath);
	m_ShaderFactory->LoadShader(effect);
	myEffects[aFilePath] = effect;
	return myEffects[aFilePath];
}

Sprite* AssetsContainer::LoadSprite(const cl::CHashString<128>& path)
{
	Sprite* sprite = new Sprite;
	m_Sprites.emplace(path.GetHash(), sprite);
	sprite->Initiate(path);
	return sprite;
}

Sprite* AssetsContainer::GetSprite(const cl::CHashString<128>& path)
{
	for (auto it = m_Sprites.begin(); it != m_Sprites.end(); it++)
	{
		if (it->first != path.GetHash())
			continue;

		return it->second;
	}
	return LoadSprite(path);
}


std::string AssetsContainer::LoadModel(std::string aFilePath, std::string effect, bool thread)
{
	if (aFilePath.find("default_cube") != aFilePath.npos)
	{
		Model* model = new Model;
		model->SetEffect(LoadEffect(m_Engine->GetVFS().GetFile(effect)));
		model->CreateCube();
		myModels.emplace("default_cube", model);

	}



	if (myModels.find(aFilePath) == myModels.end())
	{
		DL_MESSAGE("Loading model : %s", aFilePath.c_str());
		Model* model = new Model;
		myModels.emplace(aFilePath, model);

		if ( thread )
		{
			m_Engine->GetThreadpool().AddWork(Work([=]() {
				m_ModelLoader->LoadModel(model, aFilePath, effect);
				model->Initiate(aFilePath);
			}));
			return aFilePath;
		}
		else
		{
			m_ModelLoader->LoadModel(model, aFilePath, effect);
			model->Initiate(aFilePath);
			return aFilePath;
		}
	}
	return aFilePath;
}
