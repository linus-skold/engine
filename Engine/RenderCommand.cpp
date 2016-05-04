#include "stdafx.h"
#include "RenderCommand.h"

SRenderCommand::SRenderCommand(Snowblind::CInstance* anInstance, const CU::Vector3f& aPosition, const eType& aType)
	: myInstance(anInstance)
	, myPosition(aPosition)
	, myType(aType)
{
}

SRenderCommand::SRenderCommand()
{
}
