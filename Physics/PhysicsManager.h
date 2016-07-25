#pragma once
#include "../CommonLib/DataStructures/GrowingArray.h"
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btRigidBody;
class btCollisionShape;
struct btDefaultMotionState;

class CRigidBody;

class CPhysicsManager
{
public:
	CPhysicsManager();
	~CPhysicsManager();

	void Add(btRigidBody* aBody);
	void Remove(btRigidBody* aBody);
	btRigidBody* Create(float mass);
	void Update();
private:

	btDefaultCollisionConfiguration* myDefaultCollisionConfiguration = nullptr;
	btCollisionDispatcher* myCollisionDispatcher = nullptr;
	btBroadphaseInterface* myBroadphaseInterface = nullptr;
	btSequentialImpulseConstraintSolver* myImpulseSolver = nullptr;
	btDiscreteDynamicsWorld* myDynamicsWorld = nullptr;

	CRigidBody* myZeroPlane;
	CU::GrowingArray<btRigidBody*> myBodies;

};
