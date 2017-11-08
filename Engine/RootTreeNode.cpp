#include "stdafx.h"
#include "RootTreeNode.h"
#include <Engine/Synchronizer.h>
#include <Engine/profile_defines.h>
#include <Engine/DebugHandle.h>
void RootTreeNode::Initiate(float halfwidth, Octree* octree)
{
	m_HalfWidth = halfwidth;
	m_Octree = octree;

	m_Synchronizer = Engine::GetInstance()->GetSynchronizer();
	m_NodeEntityManager = Engine::GetInstance()->GetEntityManager().RequestManager(this);

	m_Pool.Initiate("RootNode - Worker");


#if !defined(_PROFILE) && !defined(_FINAL)
	debug::DebugHandle::GetInstance()->AddValueToPrint(&m_DwellerCount);
#endif
	for (TreeNodeBase* child : m_Children)
	{
		child = nullptr;
	}
}

void RootTreeNode::Update(float dt, bool paused)
{
	PROFILE_FUNCTION(profiler::colors::Green);
	if (paused)
		return;
	//m_Lines.RemoveAll();
	TreeNodeBase::Update(dt, paused);

	for (TreeNodeBase* node : m_Children)
	{
		if (!node)
			continue;

		m_Pool.AddWork(Work([=]() {
			PROFILE_BLOCK(node->m_Name.c_str());
			node->Update(dt, paused);
			PROFILE_BLOCK_END;
		}));
	}
	do
	{
		m_Pool.Update();
	} while (!m_Pool.CurrentWorkFinished()); //This cannot work if we start work and pop at the same time

// 	for (const Line& line : m_Lines)
// 		m_Synchronizer->AddRenderCommand(LineCommand(line.m_Points[0], line.m_Points[1], true));

}
