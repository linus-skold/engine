#include "stdafx.h"
#include "ModelImporter.h"
#include <DL_Debug.h>
#include "Model.h"

#include "VertexWrapper.h"
#include "IndexWrapper.h"
#include "Surface.h"
#include "Model.h"

#define TRIANGLE_VERTEX_COUNT 3
#define VERTEX_STRIDE 4
#define NORMAL_STRIDE 4
#define BINORMAL_STRIDE 4
#define TANGENT_STRIDE 4
#define SKINWEIGHT_STRIDE 4
#define BONEID_STRIDE 4
#define UV_STRIDE 2


CModelImporter::CModelImporter()
	: myEngine(Snowblind::CEngine::GetInstance())
	, myTimeManager(new CommonUtilities::TimeManager())
{
	myTimeManager->CreateTimer();
}


CModelImporter::~CModelImporter()
{
	SAFE_DELETE(myTimeManager);
}

Snowblind::CModel* CModelImporter::CreateModel(FBXModelData* someData, Snowblind::CEffect* anEffect)
{
	Snowblind::CModel* newModel = new Snowblind::CModel();
	newModel->SetEffect(anEffect);

	if (someData->myData)
	{
		FillData(someData->myData, newModel, anEffect);
		newModel->myOrientation = someData->myOrientation;
	}
	for (FBXModelData* child : someData->myChildren)
	{
		newModel->AddChild(CreateModel(child, anEffect));
	}

	return newModel;
}

Snowblind::CModel* CModelImporter::LoadModel(const std::string& aFilePath, const std::string& aEffectPath)
{
	return LoadModel(aFilePath, myEngine->GetEffect(aEffectPath))->CreateModel();
}

Snowblind::CModel* CModelImporter::LoadModel(const std::string& aFilePath, Snowblind::CEffect* anEffect)
{
	myTimeManager->Update();
	float loadTime = myTimeManager->GetTimer(0).GetTotalTime().GetMilliseconds();

	unsigned int processFlags =
		//aiProcess_CalcTangentSpace | // calculate tangents and bitangents if possible
		//aiProcess_JoinIdenticalVertices | // join identical vertices/ optimize indexing
	  //aiProcess_ValidateDataStructure  | // perform a full validation of the loader's output
		//aiProcess_Triangulate | // Ensure all verticies are triangulated (each 3 vertices are triangle)
		//aiProcess_ConvertToLeftHanded | // convert everything to D3D left handed space (by default right-handed, for OpenGL)
		//aiProcess_SortByPType | // ?
		//aiProcess_ImproveCacheLocality | // improve the cache locality of the output vertices
		//aiProcess_RemoveRedundantMaterials | // remove redundant materials
		//aiProcess_FindDegenerates | // remove degenerated polygons from the import
		//aiProcess_FindInvalidData | // detect invalid model data, such as invalid normal vectors
		//aiProcess_GenUVCoords | // convert spherical, cylindrical, box and planar mapping to proper UVs
		////aiProcess_TransformUVCoords | // preprocess UV transformations (scaling, translation ...)
		//aiProcess_FindInstances | // search for instanced meshes and remove them by references to one master
		//aiProcess_LimitBoneWeights | // limit bone weights to 4 per vertex
		//aiProcess_OptimizeMeshes | // join small meshes, if possible;
		//aiProcess_SplitByBoneCount | // split meshes with too many bones. Necessary for our (limited) hardware skinning shader
		0;

	const aiScene* scene = importer.ReadFile(aFilePath, processFlags); //MaxQuality, Quality, Fast 
	DL_MESSAGE("%s", !scene ? aFilePath.c_str() : importer.GetErrorString());
	DL_ASSERT_EXP(scene, "ImportModel Failed. Could not read the requested file.");

	aiNode* rootNode = scene->mRootNode;
	FBXModelData* data = new FBXModelData();
	ProcessNode(rootNode, scene, data);
	Snowblind::CModel* toReturn = CreateModel(data, anEffect);

	delete data->myData;
	delete data;

	loadTime = myTimeManager->GetTimer(0).GetTotalTime().GetMilliseconds() - loadTime;
	MODEL_LOG("%s took %fms to load. %s", aFilePath.c_str(), loadTime, (loadTime < 7000.f) ? "" : "Check if it's saved as binary.");

	return toReturn;
}

void CModelImporter::FillData(ModelData* someData, Snowblind::CModel* out, Snowblind::CEffect* anEffect)
{
	Snowblind::SVertexIndexWrapper* indexWrapper = new Snowblind::SVertexIndexWrapper();
	indexWrapper->myFormat = DXGI_FORMAT_R32_UINT;
	u32* indexData = new u32[someData->myIndexCount];
	memcpy(indexData, someData->myIndicies, someData->myIndexCount * sizeof(u32));
	indexWrapper->myIndexData = (s8*)indexData;
	indexWrapper->mySize = someData->myIndexCount * sizeof(u32);
	out->myIndexData = indexWrapper;

	/* BUG HERE. CRASH. */
	Snowblind::SVertexDataWrapper* vertexData = new Snowblind::SVertexDataWrapper();
	s32 sizeOfBuffer = someData->myVertexCount * someData->myVertexStride * sizeof(float); //is this wrong?
	u32* vertexRawData = new u32[sizeOfBuffer];
	memcpy(vertexRawData, someData->myVertexBuffer, sizeOfBuffer); // This crashes?
	vertexData->myVertexData = (s8*)vertexRawData;
	vertexData->myNrOfVertexes = someData->myVertexCount;
	vertexData->mySize = sizeOfBuffer;
	vertexData->myStride = someData->myVertexStride * sizeof(float);
	out->myVertexData = vertexData;

	Snowblind::CSurface* newSurface = new Snowblind::CSurface(0, someData->myVertexCount, 0
		, someData->myIndexCount, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	out->myModelStates[NULL_OBJECT] = FALSE;
	for (int i = 0; i < someData->myLayout.Size(); ++i)
	{
		auto currentLayout = someData->myLayout[i];
		D3D11_INPUT_ELEMENT_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_INPUT_ELEMENT_DESC));
		desc.SemanticIndex = 0;
		desc.AlignedByteOffset = currentLayout.myOffset;
		desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		desc.InputSlot = 0;
		desc.InstanceDataStepRate = 0;

		if (currentLayout.myType == ModelData::VERTEX_POS)
		{
			desc.SemanticName = "POSITION";
			desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (currentLayout.myType == ModelData::VERTEX_NORMAL)
		{
			desc.SemanticName = "NORMAL";
			desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (currentLayout.myType == ModelData::VERTEX_UV)
		{
			desc.SemanticName = "TEXCOORD";
			desc.Format = DXGI_FORMAT_R32G32_FLOAT;
		}
		else if (currentLayout.myType == ModelData::VERTEX_BINORMAL)
		{
			desc.SemanticName = "BINORMAL";
			desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (currentLayout.myType == ModelData::VERTEX_TANGENT)
		{
			desc.SemanticName = "TANGENT";
			desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (currentLayout.myType == ModelData::VERTEX_SKINWEIGHTS)
		{
			break;
			desc.SemanticName = "WEIGHTS";
			desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}
		else if (currentLayout.myType == ModelData::VERTEX_BONEID)
		{
			break;
			desc.SemanticName = "BONES";
			desc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
		}
		out->myVertexFormat.Add(desc);
	}


	newSurface->AddTexture("Albedo", "Data/Textures/No-Texture.dds");

	//newSurface->AddTexture()
	/*
		for each texture in modeldata
		add new texture to surface
	*/

	out->mySurfaces.Add(newSurface);
}

void CModelImporter::ProcessNode(aiNode* aNode, const aiScene* aScene, FBXModelData* someData)
{
	DL_ASSERT_EXP(someData, "Failed to process node. FBXModelData someData was null");

	for (u32 i = 0; i < aNode->mNumMeshes; i++)
	{
		aiMesh* mesh = aScene->mMeshes[aNode->mMeshes[i]];
		ProcessMesh(mesh, aScene, someData);
	}

	for (u32 i = 0; i < aNode->mNumChildren; i++)
	{
		someData->myChildren.Add(new FBXModelData());
		ProcessNode(aNode->mChildren[i], aScene, someData->myChildren.GetLast());
	}
}

void CModelImporter::ProcessMesh(aiMesh* aMesh, const aiScene* aScene, FBXModelData* fbx)
{
	FBXModelData* data = fbx;
	data->myData = new ModelData();


	u32 polygonCount = aMesh->mNumFaces;
	u32 size = polygonCount * VERTEX_STRIDE;
	u32 polygonVertexCount = polygonCount * 4;

	if (aMesh->HasPositions())
	{
		ModelData::Layout newLayout;
		newLayout.myType = ModelData::VERTEX_POS;
		newLayout.mySize = VERTEX_STRIDE;
		newLayout.myOffset = 0;
		data->myData->myLayout.Add(newLayout);
		size += polygonVertexCount * VERTEX_STRIDE;
	}
	u32 stride = VERTEX_STRIDE;


	if (aMesh->HasNormals())
	{
		ModelData::Layout newLayout;
		newLayout.myType = ModelData::VERTEX_NORMAL;
		newLayout.mySize = NORMAL_STRIDE;
		newLayout.myOffset = stride * 4;
		data->myData->myLayout.Add(newLayout);

		stride += NORMAL_STRIDE;
		size += polygonVertexCount * NORMAL_STRIDE;
	}

	if (aMesh->HasTextureCoords(0))
	{
		ModelData::Layout newLayout;
		newLayout.myType = ModelData::VERTEX_UV;
		newLayout.mySize = UV_STRIDE;
		newLayout.myOffset = stride * 4;
		data->myData->myLayout.Add(newLayout);

		stride += UV_STRIDE;
		size += polygonVertexCount * UV_STRIDE;
	}

	if (aMesh->HasTangentsAndBitangents())
	{
		ModelData::Layout newLayout;
		newLayout.myType = ModelData::VERTEX_BINORMAL;
		newLayout.mySize = BINORMAL_STRIDE;
		newLayout.myOffset = stride * 4;
		data->myData->myLayout.Add(newLayout);

		stride += BINORMAL_STRIDE;
		size += polygonVertexCount * BINORMAL_STRIDE;

		newLayout.myType = ModelData::VERTEX_TANGENT;
		newLayout.mySize = TANGENT_STRIDE;
		newLayout.myOffset = stride * 4;
		data->myData->myLayout.Add(newLayout);

		stride += TANGENT_STRIDE;
		size += polygonVertexCount * TANGENT_STRIDE;
	}

	if (aMesh->HasBones())
	{
		ModelData::Layout newLayout;
		newLayout.myType = ModelData::VERTEX_SKINWEIGHTS;
		newLayout.mySize = SKINWEIGHT_STRIDE;
		newLayout.myOffset = stride * 4;
		data->myData->myLayout.Add(newLayout);

		stride += SKINWEIGHT_STRIDE;
		size += polygonVertexCount * SKINWEIGHT_STRIDE;

		newLayout.myType = ModelData::VERTEX_BONEID;
		newLayout.mySize = BONEID_STRIDE;
		newLayout.myOffset = stride * 4;
		data->myData->myLayout.Add(newLayout);

		stride += BONEID_STRIDE;
		size += polygonVertexCount * BONEID_STRIDE;
	}

	DL_MESSAGE("Vertex Buffer Array Size : %d", size);
	data->myData->myVertexBuffer = new float[size];
	ZeroMemory(data->myData->myVertexBuffer, sizeof(float)*size);

	DL_ASSERT_EXP(aMesh->mNumVertices < size, "the amount of vertices was MORE!? than size");
	data->myData->myIndicies = new u32[polygonCount * 3];
	ZeroMemory(data->myData->myIndicies, sizeof(u32) * polygonCount * 3);

	CU::GrowingArray<u32> indices;
	u32 vertCount = 0;
	for (u32 i = 0; i < aMesh->mNumFaces; i++)
	{
		const aiFace* face = &aMesh->mFaces[i];

		for (s32 j = 2; j >= 0; j--)
		{
			s8 indexCount = 0;
			u32 addedSize = VERTEX_STRIDE;
			u32 currIndex = vertCount * stride;
			DL_ASSERT_EXP(addedSize <= size, "addedSize was larger than the size of the array.");

			u32 verticeIndex = face->mIndices[j];


			if (aMesh->HasPositions())
			{

				CU::Vector4f position(aMesh->mVertices[verticeIndex].x, aMesh->mVertices[verticeIndex].y, aMesh->mVertices[verticeIndex].z, 1);
				CU::Matrix44f fixMatrix = CU::Math::CreateReflectionMatrixAboutAxis44(CU::Vector3f(1, 0, 0));
				position = position * fixMatrix;

				data->myData->myVertexBuffer[currIndex] = position.x;
				data->myData->myVertexBuffer[currIndex + 1] = position.y;
				data->myData->myVertexBuffer[currIndex + 2] = position.z;
				data->myData->myVertexBuffer[currIndex + 3] = 0;
			}

			if (aMesh->HasNormals())
			{

				CU::Vector3f normal(aMesh->mNormals[verticeIndex].x, aMesh->mNormals[verticeIndex].y, aMesh->mNormals[verticeIndex].z);
				normal = normal * CU::Math::CreateReflectionMatrixAboutAxis(CU::Vector3f(1, 0, 0));
				CU::Math::Normalize(normal);

				data->myData->myVertexBuffer[currIndex + addedSize] = normal.x;
				data->myData->myVertexBuffer[currIndex + addedSize + 1] = normal.y;
				data->myData->myVertexBuffer[currIndex + addedSize + 2] = normal.z;
				data->myData->myVertexBuffer[currIndex + addedSize + 3] = 0;
				addedSize += NORMAL_STRIDE;
			}

			if (aMesh->HasTextureCoords(0))
			{
				data->myData->myVertexBuffer[currIndex + addedSize] = aMesh->mTextureCoords[0][verticeIndex].x;
				data->myData->myVertexBuffer[currIndex + addedSize + 1] = aMesh->mTextureCoords[0][verticeIndex].y * -1.f;
				addedSize += UV_STRIDE;
			}

			if (aMesh->HasTangentsAndBitangents())
			{

				CU::Vector3f normal(aMesh->mBitangents[verticeIndex].x, aMesh->mBitangents[verticeIndex].y, aMesh->mBitangents[verticeIndex].z);
				normal = normal * CU::Math::CreateReflectionMatrixAboutAxis(CU::Vector3f(1, 0, 0));
				CU::Math::Normalize(normal);

				data->myData->myVertexBuffer[currIndex + addedSize] = normal.x;
				data->myData->myVertexBuffer[currIndex + addedSize + 1] = normal.y;
				data->myData->myVertexBuffer[currIndex + addedSize + 2] = normal.z;
				data->myData->myVertexBuffer[currIndex + addedSize + 3] = 0;
				addedSize += BINORMAL_STRIDE;

				CU::Vector3f tangent(aMesh->mTangents[verticeIndex].x, aMesh->mTangents[verticeIndex].y, aMesh->mTangents[verticeIndex].z);
				tangent = tangent * CU::Math::CreateReflectionMatrixAboutAxis(CU::Vector3f(1, 0, 0));
				CU::Math::Normalize(tangent);

				data->myData->myVertexBuffer[currIndex + addedSize] = tangent.x;
				data->myData->myVertexBuffer[currIndex + addedSize + 1] = tangent.y;
				data->myData->myVertexBuffer[currIndex + addedSize + 2] = tangent.z;
				data->myData->myVertexBuffer[currIndex + addedSize + 3] = 0;
				addedSize += TANGENT_STRIDE;
			}

			indices.Add(verticeIndex);
			vertCount++;
			indexCount++;
		}
	}

	//Flips it to make it correct.
	CU::GrowingArray<u32> indiceFix;
	for (s32 i = indices.Size() - 1; i >= 0; i--)
	{
		indiceFix.Add(indices[i]);
	}

	memcpy(data->myData->myIndicies, &indiceFix[0], sizeof(u32) * indiceFix.Size());
	data->myData->myVertexStride = stride;


	data->myData->myVertexCount = vertCount;
	data->myData->myIndexCount = polygonCount * 3;
}
