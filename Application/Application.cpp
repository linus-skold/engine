#include "Application.h"
#include <Engine.h>
#include <Effect.h>
#include <Model.h>
#include <Scene.h>
#include <Camera.h>
#include <Instance.h>
#include <TimeManager.h>                                                                                                                                                                                                                                                                                                                                                                                   
#include <EngineDefines.h>
#include "EffectContainer.h"
#include "../Input/InputWrapper.h"
#include <FontManager.h>
#include <Sprite.h>
#define ROTATION_SPEED  50.f / 180.f * float(PI)
#define MOVE_SPEED 50.f
CApplication::CApplication()
{
}

CApplication::~CApplication()
{
	SAFE_DELETE(mySprite);
	SAFE_DELETE(myFontManager);
	SAFE_DELETE(myWorldScene);
	SAFE_DELETE(myInstance);
	SAFE_DELETE(myModel);
	SAFE_DELETE(myTexturedModel);
	SAFE_DELETE(myCamera);
	CU::TimeManager::Destroy();
	Snowblind::CEffectContainer::Destroy();
	Snowblind::CEngine::Destroy();
}

void CApplication::OnPause()
{
	CU::TimeManager::GetInstance()->Pause();
}

void CApplication::OnResume()
{
	CU::TimeManager::GetInstance()->Start();
}

void CApplication::OnInactive()
{
	CU::Input::InputWrapper::SetActiveWindow(false);
}

void CApplication::OnActive()
{
	CU::Input::InputWrapper::SetActiveWindow(true);
}

void CApplication::Initiate(float aWindowWidth, float aWindowHeight)
{
	CU::TimeManager::Create();

	myCamera = new Snowblind::CCamera(aWindowWidth, aWindowHeight, Vector3f(0.f, 0.f, 25.f));
	my2DCamera = new Snowblind::CCamera(aWindowWidth, aWindowHeight, Vector3f(0.f, 0.f, 25.f));

	myWorldScene = new Snowblind::CScene();
	myWorldScene->Initiate(myCamera);

	my2DScene = new Snowblind::CScene();
	my2DScene->Initiate(my2DCamera, true);

	myFontManager = new Snowblind::CFontManager();
	myFontManager->Initiate();
	myFontManager->LoadFont("Data/Font/OpenSans-Light.ttf", 16);

	myModel = new Snowblind::CModel(myCamera);
	myModel->CreateCube("Data/Shaders/Cube.fx", 1.f, 1.f, 1.f);

	myTexturedModel = new Snowblind::CModel(myCamera);
	myTexturedModel->CreateTexturedCube("Data/Shaders/TexturedCube.fx", 1.f, 1.f, 1.f);

	Snowblind::CEffectContainer::GetInstance()->GetEffect("Data/Shaders/TexturedCube.fx")->SetAlbedo(myFontManager->GetShaderResource());

	myInstance = new Snowblind::CInstance();
	myInstance->Initiate(myModel);
	myInstance->SetPosition({ 0.f, 0.f, 0.f });
	myWorldScene->AddToScene(myInstance);

	myInstance = new Snowblind::CInstance();
	myInstance->Initiate(myTexturedModel);
	myInstance->SetPosition({ 0.f,5.f,0.f });
	myWorldScene->AddToScene(myInstance);

	mySprite = new Snowblind::CSprite();
	mySprite->Initiate(myFontManager->GetShaderResource(), { 512.f,512.f }, { 0.f,0.f });
	//mySprite->Initiate("Data/Textures/colors.dds", { 50.f,50.f }, { 0.f,0.f });
	mySprite->SetPosition({ 256, 256});

	mySprite2 = new Snowblind::CSprite();
	mySprite2->Initiate(myFontManager->GetShaderResource(), { 128.f, 128.f }, { 0.f, 0.f });
	mySprite2->SetPosition({ 640.f, 360 });

	mySprite->SetHotspot({ mySprite->GetSize().x * 0.5f, mySprite->GetSize().y * 0.5f });
	my2DScene->AddToScene(mySprite);
	my2DScene->AddToScene(mySprite2);
}

bool CApplication::Update()
{
	CU::TimeManager::Update();
	CU::Input::InputWrapper::GetInstance()->Update();
	float deltaTime = CU::TimeManager::GetInstance()->GetDeltaTime();

	if (CU::Input::InputWrapper::GetInstance()->KeyDown(ESCAPE))
	{
		return false;
	}

	UpdateInput(deltaTime);
	myWorldScene->Update(deltaTime);
	Render();
	return true;
}

void CApplication::Render()
{
	Snowblind::CEngine::Clear();
	myWorldScene->Render();
	my2DScene->Render();
	Snowblind::CEngine::Present();
}

void CApplication::UpdateInput(float aDeltaTime)
{
	if (CU::Input::InputWrapper::GetInstance()->KeyDown(W))
	{
		myCamera->Move(Snowblind::eDirection::FORWARD, MOVE_SPEED * aDeltaTime);
	}
	if (CU::Input::InputWrapper::GetInstance()->KeyDown(S))
	{
		myCamera->Move(Snowblind::eDirection::BACK, -MOVE_SPEED * aDeltaTime);
	}
	if (CU::Input::InputWrapper::GetInstance()->KeyDown(SPACE))
	{
		myCamera->Move(Snowblind::eDirection::UP, MOVE_SPEED * aDeltaTime);
	}
	if (CU::Input::InputWrapper::GetInstance()->KeyDown(X))
	{
		myCamera->Move(Snowblind::eDirection::DOWN, -MOVE_SPEED * aDeltaTime);
	}
	if (CU::Input::InputWrapper::GetInstance()->KeyDown(D))
	{
		myCamera->Move(Snowblind::eDirection::RIGHT, MOVE_SPEED * aDeltaTime);
	}
	if (CU::Input::InputWrapper::GetInstance()->KeyDown(A))
	{
		myCamera->Move(Snowblind::eDirection::LEFT, -MOVE_SPEED * aDeltaTime);
	}

	if (CU::Input::InputWrapper::GetInstance()->KeyDown(UP_ARROW))
	{
		myCamera->Rotate(Snowblind::eRotation::X_AXIS, -ROTATION_SPEED * aDeltaTime);
	}
	if (CU::Input::InputWrapper::GetInstance()->KeyDown(DOWN_ARROW))
	{
		myCamera->Rotate(Snowblind::eRotation::X_AXIS, ROTATION_SPEED * aDeltaTime);
	}
	if (CU::Input::InputWrapper::GetInstance()->KeyDown(LEFT_ARROW))
	{
		myCamera->Rotate(Snowblind::eRotation::Y_AXIS, -ROTATION_SPEED * aDeltaTime);
	}
	if (CU::Input::InputWrapper::GetInstance()->KeyDown(RIGHT_ARROW))
	{
		myCamera->Rotate(Snowblind::eRotation::Y_AXIS, ROTATION_SPEED * aDeltaTime);
	}
	if (CU::Input::InputWrapper::GetInstance()->KeyDown(Q))
	{
		myCamera->Rotate(Snowblind::eRotation::Z_AXIS, ROTATION_SPEED * aDeltaTime);
	}
	if (CU::Input::InputWrapper::GetInstance()->KeyDown(E))
	{
		myCamera->Rotate(Snowblind::eRotation::Z_AXIS, -ROTATION_SPEED * aDeltaTime);
	}
}
