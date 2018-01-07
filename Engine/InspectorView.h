#pragma once
#include <engine/engine_shared.h>
#include <EntitySystem/EntityManager.h>
class InspectorView
{
public:
	InspectorView(EntityManager& em);
	virtual ~InspectorView() { };

	virtual void Update() = 0;

	virtual void SetEntity(Entity e, s32 component_flag);
	

protected:
	Entity m_CurrentEntity = 0;
	s32 m_ComponentFlags;
	EntityManager& m_Manager;


};