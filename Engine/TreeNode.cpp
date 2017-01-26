#include "stdafx.h"
#include "TreeNode.h"
#include "Synchronizer.h"
#include "RenderCommand.h"

void TreeNode::Initiate(float halfwidth)
{
	m_HalfWidth = halfwidth;
	m_Synchronizer = Hex::Engine::GetInstance()->GetSynchronizer();
}

void TreeNode::AddChild(TreeNode& child_node)
{
	child_node.AddParent(*this);
	m_Children.Add(&child_node);
}

void TreeNode::AddParent(TreeNode& parent_node)
{
	m_Parent = &parent_node;
}

void TreeNode::AddEntity(Entity entity)
{
	//Check if the entity is in a deeper level than parent
}

void TreeNode::SetPosition(CU::Vector3f position)
{
	m_CenterPosition = position;
}

#define RED CU::Vector4f(255.f,0.f,0.f,255.f)

void TreeNode::Update(float dt)
{
	RenderBox();
	for (TreeNode* node : m_Children)
	{
		node->Update(dt);
	}
}

void TreeNode::RenderBox()
{
	SLinePoint points[8];
	points[0].color = RED;
	points[0].position = CU::Vector4f(m_CenterPosition.x - m_HalfWidth, 0, m_CenterPosition.z - m_HalfWidth, 1);

	points[1].color = RED;
	points[1].position = points[0].position;
	points[1].position.y = 100.f;


	points[2].color = RED;
	points[2].position = CU::Vector4f(m_CenterPosition.x - m_HalfWidth, 0, m_CenterPosition.z + m_HalfWidth, 1);

	points[3].color = RED;
	points[3].position = points[2].position;
	points[3].position.y = 100.f;


	points[4].color = RED;
	points[4].position = CU::Vector4f(m_CenterPosition.x + m_HalfWidth, 0, m_CenterPosition.z + m_HalfWidth, 1);

	points[5].color = RED;
	points[5].position = points[4].position;
	points[5].position.y = 100.f;


	points[6].color = RED;
	points[6].position = CU::Vector4f(m_CenterPosition.x + m_HalfWidth, 0, m_CenterPosition.z - m_HalfWidth, 1);

	points[7].color = RED;
	points[7].position = points[6].position;
	points[7].position.y = 100.f;


	m_Synchronizer->AddRenderCommand(RenderCommand(eType::LINE_Z_ENABLE, points[0], points[1]));
	m_Synchronizer->AddRenderCommand(RenderCommand(eType::LINE_Z_ENABLE, points[0], points[2]));
	m_Synchronizer->AddRenderCommand(RenderCommand(eType::LINE_Z_ENABLE, points[0], points[6]));

	m_Synchronizer->AddRenderCommand(RenderCommand(eType::LINE_Z_ENABLE, points[1], points[3]));
	m_Synchronizer->AddRenderCommand(RenderCommand(eType::LINE_Z_ENABLE, points[1], points[7]));

	m_Synchronizer->AddRenderCommand(RenderCommand(eType::LINE_Z_ENABLE, points[3], points[5]));
	m_Synchronizer->AddRenderCommand(RenderCommand(eType::LINE_Z_ENABLE, points[5], points[7]));


	m_Synchronizer->AddRenderCommand(RenderCommand(eType::LINE_Z_ENABLE, points[6], points[4]));
	m_Synchronizer->AddRenderCommand(RenderCommand(eType::LINE_Z_ENABLE, points[2], points[4]));

	m_Synchronizer->AddRenderCommand(RenderCommand(eType::LINE_Z_ENABLE, points[4], points[5]));
	m_Synchronizer->AddRenderCommand(RenderCommand(eType::LINE_Z_ENABLE, points[2], points[3]));

	m_Synchronizer->AddRenderCommand(RenderCommand(eType::LINE_Z_ENABLE, points[6], points[7]));






	

}