#include "stdafx.h"
#include "AssetsContainer.h"
#include "Model.h"
#include "Texture.h"
#include "Effect.h"
#include "ShaderFactory.h"
#include "ModelImporter.h"
#include "Texture.h"


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
		DL_ASSERT_EXP(it->second->CleanUp(), "Failed to cleanup a model.");
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
	mySprites.empty();
}

Texture* AssetsContainer::GetTexture(std::string aFilePath)
{
	if(aFilePath.find(".dds") == aFilePath.npos)
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

CModel* AssetsContainer::GetModel(const std::string& aFilePath)
{
	if (myModels.find(aFilePath) == myModels.end())
	{
		DL_MESSAGE("Requested Model : %s", aFilePath.c_str());
		DL_ASSERT("Failed to find requested model. Did you enter the correct path?");
	}
	return myModels[aFilePath];
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
			DL_ASSERT_EXP(texture->CleanUp(), "Failed to cleanup texture!");
			SAFE_DELETE(texture);
			EndTicketMutex(&m_Mutex);
			return false;
		}
		myTextures[aFilePath] = texture;
	}
	EndTicketMutex(&m_Mutex);
	return true;
}

void AssetsContainer::LoadEffect(const std::string& aFilePath)
{
	Effect* effect = new Effect(aFilePath);
	m_ShaderFactory->LoadShader(effect);
	myEffects[aFilePath] = effect;
}

#define THREAD_LOADING

std::string AssetsContainer::LoadModel(std::string aFilePath, std::string effect, bool thread)
{
	if (myModels.find(aFilePath) == myModels.end())
	{
		DL_MESSAGE("Loading model : %s", aFilePath.c_str());
		CModel* model = new CModel; // this should not be created here. This should be sent in from whereever we load models.
		myModels.emplace(aFilePath, model);

		if ( thread )
		{
			m_Engine->GetThreadpool().AddWork(Work([=]() {
				m_ModelLoader->LoadModel(aFilePath, model, effect);
			}));
			return aFilePath;
		}
		else
		{
			m_ModelLoader->LoadModel(aFilePath, model, effect);
			return aFilePath;
		}
	}
	return aFilePath;
}

std::string AssetsContainer::LoadModel(std::string filepath, std::string effect, CModel* pModel, bool thread /*= true*/)
{
	if (myModels.find(filepath) != myModels.end())
		return filepath;

	DL_MESSAGE("Loading model : %s", filepath.c_str());
	//CModel* model = new CModel;
	myModels.emplace(filepath, pModel);

	if (thread)
	{
		m_Engine->GetThreadpool().AddWork(Work([=]() {
			m_ModelLoader->LoadModel(filepath, pModel, effect);
		}));
		return filepath;
	}
	else
	{
		m_ModelLoader->LoadModel(filepath, pModel, effect);
		return filepath;
	}

	return filepath;
}

void AssetsContainer::AddLoadRequest(std::string file, eRequestType request_type)
{
	BeginTicketMutex(&m_Mutex);

	bool already_exist = false;
	for (const LoadRequest& request : m_RequestList)
	{
		if (request.m_File == file)
		{
			already_exist = true;
		}
	}


	if (!already_exist)
	{
		LoadRequest request(request_type, file);
		m_RequestList.Add(request);
	}

	EndTicketMutex(&m_Mutex);
}
