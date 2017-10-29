#pragma once
#include "BaseComponent.h"
#include <Math/Vector/Vector.h>
#include <Engine/engine_shared.h>
#include <CommonLib/DataStructures/GrowingArray.h>
#include <Math/Matrix/Matrix.h>

struct ModelInstance
{
	template <typename Writer>
	void Serialize(Writer& writer) const;

	std::string m_Filename;
	std::string m_MaterialFile;

	u64 m_ModelID;
	u64 m_MaterialKey;

	CU::Matrix44f m_Orientation;

	CU::Vector4f m_Scale;
	CU::Vector4f m_Rotation;
	CU::Vector4f m_MinPos;
	CU::Vector4f m_MaxPos;

	bool m_RenderWireframe = false;
	bool m_Shadowed = true;


};

template <typename Writer>
void ModelInstance::Serialize(Writer& writer) const
{
	writer.StartObject();
	
	writer.String("model_file");
	writer.String(m_Filename.c_str(), (rapidjson::SizeType)m_Filename.length());

	writer.String("material_file");
	writer.String(m_MaterialFile.c_str(), (rapidjson::SizeType)m_MaterialFile.length());

	auto& position = m_Orientation.GetPosition();

	writer.String("relative_position");
	writer.StartArray();
	writer.Double(position.x);
	writer.Double(position.y);
	writer.Double(position.z);
	writer.EndArray();


	writer.String("relative_scale");
	writer.StartArray();
	writer.Double(m_Scale.x);
	writer.Double(m_Scale.y);
	writer.Double(m_Scale.z);
	writer.EndArray();

	writer.String("relative_rotation");
	writer.StartArray();
	writer.Double(m_Rotation.x);
	writer.Double(m_Rotation.y);
	writer.Double(m_Rotation.z);
	writer.EndArray();

	writer.String("shadowed");
	writer.Bool(m_Shadowed);

	writer.EndObject();
}

struct GraphicsComponent : public BaseComponent
{

	template <typename Writer>
	void Serialize(Writer& writer) const;

	CU::GrowingArray<ModelInstance> m_Instances;

	CU::Matrix44f m_Orientation;
	CU::Vector4f m_Scale;
	CU::Vector4f m_Rotation;
	CU::Vector4f m_MinPos;
	CU::Vector4f m_MaxPos;

	bool m_RenderWireframe = false;
	bool m_Shadowed = true;
};

template <typename Writer>
void GraphicsComponent::Serialize(Writer& writer) const
{
	writer.StartObject();
	writer.String("component_type");
	writer.String("graphics");

	writer.String("instances");
	writer.StartArray();
	for (const ModelInstance& i : m_Instances)
	{
		i.Serialize(writer);
	}
	writer.EndArray();



	/*auto& position = m_Orientation.GetPosition();
	writer.String("position");
	writer.StartArray();
	writer.Double(position.x);
	writer.Double(position.y);
	writer.Double(position.z);
	writer.EndArray();


	writer.String("scale");
	writer.StartArray();
	writer.Double(m_Scale.x);
	writer.Double(m_Scale.y);
	writer.Double(m_Scale.z);
	writer.EndArray();

	writer.String("rotation");
	writer.StartArray();
	writer.Double(m_Rotation.x);
	writer.Double(m_Rotation.y);
	writer.Double(m_Rotation.z);
	writer.EndArray();*/

	writer.EndObject();

}
