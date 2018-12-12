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

	TreeDweller* CreateEntitiy(const std::string& entity_filepath, const CU::Vector3f& position);
	void CreateEntitiy(const std::string& entity_filepath, JSONElement::ConstMemberIterator it);
	void CreateTranslationComponent(Entity entity_id, const CU::Vector3f& position);
	void CreateGraphicsComponent(JSONReader& entity_reader, Entity entity_id);

	void CreateGraphicsComponent(JSONReader& entity_reader, Entity entity_id, JSONElement::ConstMemberIterator it);
	void CreateLightComponent(JSONReader& entity_reader, Entity entity_id, JSONElement::ConstMemberIterator it);
	CU::GrowingArray<TreeDweller*> GetDwellers() { return m_DwellerList; }
	
	static CU::GrowingArray<TreeDweller*> CreatePBLLevel(int32 steps);
	static CU::GrowingArray<TreeDweller*> CreatePBLLevel(int32 x_steps, int32 y_steps, int32 z_steps, const CU::Vector3f& pos, float x_spacing, float y_spacing, float z_spacing);


	static void SaveLevel(std::string folder, std::string filename);
	static void CreateEntity(Entity e, EntityManager& em);
	static void CreateEntity(const char* entity_filepath, CU::GrowingArray<TreeDweller*>& out_dwellers);
	static CU::GrowingArray<TreeDweller*> LoadLevel(const char* level);

private:
	void CreateEntity(const std::string& entity_filepath);





	CU::GrowingArray<TreeDweller*> m_DwellerList;

	JSONReader m_LevelReader;

	Engine* m_Engine			= nullptr;
	EntityManager* m_EntityManager;
	PhysicsManager* m_PhysicsManager	= nullptr;
	bool sponza = false;

	void CreateDebugComponent(Entity e, bool isLight, int32 flags);
};
