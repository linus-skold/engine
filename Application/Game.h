#pragma once
#include <DataStructures/GrowingArray.h>
#include <Math/Vector/Vector.h>
#include <string>
#include <Line3D.h>

namespace Snowblind
{
	class CSynchronizer;
	class CTerrain;
	class CEngine;
	class CMousePicker;
	class CModel;
};

class CEntityManager;
class CPhysicsManager;
class CRigidBody;

class CGame
{
public:
	CGame(Snowblind::CSynchronizer* aSynchronizer);
	~CGame();

	void Update(float aDeltaTime);

private:
	
	int myFrameCount = 0;
	float myAverageFPS = 0;
	float myTime = 0.f;
	int myFPSToPrint = 0;

	CU::GrowingArray<Snowblind::CTerrain*> myTerrain;
	CU::GrowingArray<CRigidBody*> myTerrainBodies;

	Snowblind::CSynchronizer* mySynchronizer;
	Snowblind::CEngine* myEngine;
	Snowblind::CMousePicker* myPicker;
	std::string myModelKey;
	CEntityManager* myEntityManager;
	CPhysicsManager* myPhysicsManager;
	CU::Vector3f pointHit;
	CU::Vector3f currentRay;


	SLinePoint raycast[2];

};

