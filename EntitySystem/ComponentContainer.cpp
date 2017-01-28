#include "ComponentContainer.h"
#include "ComponentFilter.h"
#include "BaseComponent.h"
#include "TranslationComponent.h"
#include "../DL_Debug/DL_Debug.h"
CComponentContainer::CComponentContainer()
	: myComponents(MAX_COMPONENTS_COUNT)
	, myEntityComponents(128)
	, myEntitiesToReturn(128)
{
	for (int i = 0; i < MAX_COMPONENTS_COUNT; i++)
	{
		myComponents.Add(ComponentArray());
		myComponents[i].ReInit(128);
	}
}

CComponentContainer::~CComponentContainer()
{
	for (int i = 0; i < myComponents.Size(); i++)
	{
		myComponents[i].DeleteAll();
	}
}

void CComponentContainer::AddEntity(Entity id)
{
	EntityComponent ec;
	ec.m_Entity = id;
	ec.m_EntityArray = EntityComponentArray();

	myEntityComponents.Add(ec);
	EntityComponentArray& components = myEntityComponents.GetLast().m_EntityArray;
	for (int i = 0; i < MAX_COMPONENTS_COUNT; i++)
	{
		components[i] = -1;
	}
}

void CComponentContainer::AddComponent(Entity anEntity, BaseComponent* aComponent, unsigned int aComponentID)
{
	myComponents[aComponentID].Add(aComponent);
	for (EntityComponent& ec : myEntityComponents)
	{
		if (ec.m_Entity == anEntity)
		{
			ec.m_EntityArray[aComponentID] = myComponents[aComponentID].Size() - 1;
			break;
		}
	}
}

BaseComponent& CComponentContainer::GetComponent(Entity anEntity, unsigned int aComponentID)
{
	/*Error Handling*/
	for (EntityComponent& ec : myEntityComponents)
	{
		if (ec.m_Entity == anEntity)
		{
			int componentIndex = ec.m_EntityArray[aComponentID];
			return *myComponents[aComponentID][componentIndex];
		}
	}
	DL_ASSERT("Failed to find component index!");
	return TranslationComponent();
}

void CComponentContainer::RemoveComponent(Entity entity, BaseComponent* component, u32 component_id)
{
	for (EntityComponent& ec : myEntityComponents)
	{
		if (ec.m_Entity == entity)
		{
			ec.m_EntityArray[component_id]--;
			myComponents[component_id].RemoveCyclic(component);
			break;
		}
	}
}

bool CComponentContainer::HasComponent(Entity e, SComponentFilter& filter)
{
	for (const EntityComponent& ec : myEntityComponents)
	{
		if (filter.Compare(ec.m_EntityArray))
		{
			return true;
		}
	}
	return false;
}

const CU::GrowingArray<Entity>& CComponentContainer::GetEntities(SComponentFilter& aFilter)
{
	myEntitiesToReturn.RemoveAll();
	/*for (int i = 0; i < myEntityComponents.Size(); i++)
	{
		if (aFilter.Compare(myEntityComponents[i]) == true)
		{
			myEntitiesToReturn.Add(i);
		}
	}*/
	for (const EntityComponent& ec : myEntityComponents)
	{
		if (aFilter.Compare(ec.m_EntityArray))
		{
			myEntitiesToReturn.Add(ec.m_Entity);
		}
	}
	return myEntitiesToReturn;
}

