#pragma once
#include <assimp/Importer.hpp>

#include <Engine/Surface.h>
#include "snowblind_shared.h"
#ifdef _DEBUG
#include <TimeManager.h>
#endif
class CModel;
class Effect;
class Engine;

struct TextureInfo
{
	TextureType myType;
	std::string myFilename;
};

class aiNode;
class aiMesh;
class aiScene;


struct ModelData
{
	enum LayoutType
	{
		VERTEX_POS,
		VERTEX_NORMAL,
		VERTEX_UV,
		VERTEX_BINORMAL,
		VERTEX_TANGENT,
		VERTEX_SKINWEIGHTS,
		VERTEX_BONEID,
	};

	struct Layout
	{
		int mySize;
		int myOffset;
		LayoutType myType;
	};

	float* myVertexBuffer;
	u32* myIndicies = nullptr;
	u32 myVertexCount = 0;
	u32 myVertexStride = 0; //What is this?
	u32 myIndexCount = 0;
	CU::GrowingArray<TextureInfo> myTextures;
	CU::GrowingArray<Layout> myLayout;

	CU::Vector3f m_WHD;

	CU::Vector3f m_MinPoint;
	CU::Vector3f m_MaxPoint;


};

struct TextureData
{
	CU::GrowingArray<TextureInfo> myTextures;
};

struct FBXModelData
{
	CU::Matrix44f myOrientation;
	ModelData* myData = nullptr;
	TextureData* myTextureData = nullptr;
	CU::GrowingArray<FBXModelData*> myChildren;
};

class CModelImporter
{
public:
	CModelImporter();

	void LoadModel(std::string filepath, CModel* model, std::string aEffectPath);

private:
	Engine* m_Engine = nullptr;
	Ticket_Mutex m_LoaderMutex;

#ifdef _DEBUG
	CU::TimeManager m_TimeManager;
#endif

	CModel* CreateModel(FBXModelData* data, CModel* model, std::string filepath, Effect* effect);
	CModel* CreateChild(FBXModelData* data, std::string filepath, Effect* effect);


	CModel* LoadModel(std::string filepath, CModel* model, Effect* effect);

	void FillData(FBXModelData* data, CModel* model, std::string filepath);

	void SetupInputLayout(ModelData* data, CModel* model);

	void ProcessNode(aiNode* node, const aiScene* scene, FBXModelData* data, std::string file);
	void ProcessMesh(aiMesh* mesh, const aiScene* scene, FBXModelData* data, std::string file);

	void ExtractMaterials(aiMesh* mesh,const aiScene* scene, FBXModelData* data, std::string file);

};
//The CModel* should be able to be a template pointer instead, easier understanding of what you are doing. Does not matter too much.