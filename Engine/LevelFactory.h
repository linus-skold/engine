#pragma once

#include <JSON/JSONReader.h>
#include "engine_shared.h"
#include <EntityManager.h>

class EntityManager;
class PhysicsManager;
class TreeDweller;
class LevelFactory
{
public:
	LevelFactory() = default;

	void Initiate();
	bool CreateLevel(const std::string& level_path);
	CU::GrowingArray<TreeDweller*> GetDwellers() { return m_DwellerList; }
	
	void CreatePBLLevel(s32 steps);


	void SaveLevel(std::string folder, std::string filename);


private:
	void CreateEntity(const std::string& entity_filepath);
	void CreateEntitiy(const std::string& entity_filepath, JSONElement::ConstMemberIterator it);
	void CreateTerrain(std::string terrain_path);

	void CreatePhysicsComponent(JSONReader& entity_reader, Entity entity_id);


	void CreateLightComponent(JSONReader& entity_reader, Entity entity_id, JSONElement::ConstMemberIterator it);


	CU::GrowingArray<TreeDweller*> m_DwellerList;

	JSONReader m_LevelReader;

	Engine* m_Engine			= nullptr;
	EntityManager* m_EntityManager;
	PhysicsManager* m_PhysicsManager	= nullptr;
	bool sponza = false;

	void CreateDebugComponent(Entity e, bool isLight, s32 flags);
};
