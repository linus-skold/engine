#pragma once
#include <map>
#include <unordered_map>
#include "engine_shared.h"
#include <DataStructures/GrowingArray.h>
#include <Engine/ModelImporter.h>
#include <CommonLib/DataStructures/StaticArray.h>s

class FileWatcher;
class ShaderFactory;
class Model;
class Effect;
class Texture;
class Sprite;
class Material;


struct CompiledShader;

class AssetsContainer
{
public:
	AssetsContainer() = default;
	~AssetsContainer();
	void Initiate();

	void Update();

	void ReloadTexture(Texture* texture);

	Texture* GetTexture(u64 key);
	Sprite* GetSprite(u64 key);
	Effect* GetEffect(u64 key);
	Model* GetModel(u64 key);
	Material* GetMaterial(u64 key);

	template<typename T>
	u64 LoadModel(std::string path, std::string effect_filepath, bool thread = true);

	u64 LoadTexture(const std::string& path);
	u64 LoadEffect(const std::string& path);
	u64 LoadSprite(const std::string& path);


private:
#ifndef FINAL
	FileWatcher* m_Watcher = nullptr;
#endif


	std::map<u64, Texture*> m_Textures;
	std::map<u64, Effect*> m_Effects;
	std::map<u64, Model*> m_Models;
	std::map<u64, Sprite*> m_Sprites;
	std::map<u64, Material*> m_Materials;


	ShaderFactory* m_ShaderFactory;
	CModelImporter* m_ModelLoader;

};

template<typename T>
u64 AssetsContainer::LoadModel(std::string path, std::string effect_filepath, bool thread /*= true*/)
{
	u64 hash = Hash(path.c_str());

	if (m_Models.find(hash) != m_Models.end())
		return hash;

	T* model = new T;
	m_Models.emplace(hash, model);

	if (thread)
	{
		Engine::GetInstance()->GetThreadpool().AddWork(Work([=]() {
			m_ModelLoader->LoadModel(model, path, effect_filepath);
			model->Initiate(path);
		}));
	}
	else
	{
		m_ModelLoader->LoadModel(model, path, effect_filepath);
		model->Initiate(path);
	}

	return hash;
}
