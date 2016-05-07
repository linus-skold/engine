#include "stdafx.h"
#include <thread>

#include "Renderer.h"
#include "Engine.h"
#include "Synchronizer.h"
#include "RenderCommand.h"
#include "Instance.h"
#include "Camera.h"
#include "DeferredRenderer.h"
#include "PointLight.h"

namespace Snowblind
{

	CRenderer::CRenderer(CSynchronizer& aSynchronizer, CCamera* aCamera)
		: mySynchronizer(aSynchronizer)
		, myCamera(aCamera)
	{
		myText = new Snowblind::CText("Data/Font/OpenSans-Bold.ttf", 16);
		myDeferredRenderer = new CDeferredRenderer();
	}

	CRenderer::~CRenderer()
	{
		SAFE_DELETE(my2DCamera);
		SAFE_DELETE(myDeferredRenderer);
	}

	void CRenderer::Add2DCamera(CCamera* aCamera)
	{
		my2DCamera = aCamera;
	}

	void CRenderer::Render()
	{
		CEngine::Clear();

		myDeferredRenderer->SetTargets();
		Render3DCommands();
		myDeferredRenderer->SetBuffers();
		myDeferredRenderer->DeferredRender();

		myDeferredRenderer->SetLightState(myCamera);
		RenderLightCommands();
		myDeferredRenderer->SetNormalState();

		Render2DCommands();

		CEngine::Present();

		mySynchronizer.WaitForLogic();
		mySynchronizer.SwapBuffer();
		mySynchronizer.RenderIsDone();
	}

	void CRenderer::Render3DCommands()
	{
		const CU::GrowingArray<SRenderCommand>& commands = mySynchronizer.GetRenderCommands();
		for each(const SRenderCommand& command in commands)
		{
			switch (command.myType)
			{
			case SRenderCommand::eType::MODEL:
				command.myInstance->SetPosition(command.myPosition);
				command.myInstance->Render(*myCamera);
				break;
			}
		}
	}

	void CRenderer::Render2DCommands()
	{
		/* This happens after the deferred pass */
		const CU::GrowingArray<SRenderCommand>& commands2D = mySynchronizer.GetRenderCommands(eCommandType::e2D);
		for each(const SRenderCommand& command in commands2D)
		{
			switch (command.myType)
			{
			case SRenderCommand::eType::TEXT:
				myText->SetText(command.myTextToPrint);
				myText->SetPosition({ command.myPosition.x, command.myPosition.y });
				myText->Render(my2DCamera);
				break;
			}
		}
	}

	void CRenderer::RenderLightCommands()
	{
		const CU::GrowingArray<SRenderCommand>& commands = mySynchronizer.GetRenderCommands(eCommandType::LIGHT);
		for each(const SRenderCommand& command in commands)
		{
			switch (command.myType)
			{
			case SRenderCommand::eType::POINTLIGHT:
				command.myPointLight->SetPosition(command.myPosition);
				command.myPointLight->SetColor({ command.myColor.r, command.myColor.g, command.myColor.b, 1.f });
				command.myPointLight->Update();
				myDeferredRenderer->RenderLight(command.myPointLight);
				break;
			}
		}
	}
};