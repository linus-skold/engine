#include "stdafx.h"
#include "RenderCommand.h"

SRenderCommand::SRenderCommand(const std::string aString, const CU::Math::Vector2<float>& aPosition, const eType& aType)
	: myType(aType)
	, myTextToPrint(aString)
	, myPosition(aPosition)
	, myCommandType(eCommandBuffer::e2D)
{
}

SRenderCommand::SRenderCommand(const eType& aType, const std::string& modelKey, const CU::Vector3f& aPosition)
	: myType(aType)
	, myModelKey(modelKey)
	, myPosition(aPosition)
	, myCommandType(eCommandBuffer::e3D)
{
}

SRenderCommand::SRenderCommand(const eType& aType, const CU::Vector3f& position, const CU::Vector3f& color, const float& intensity, const float& range)
	: myType(aType)
	, myPosition(position)
	, myColor(color)
	, myIntensity(intensity)
	, myRange(range)
	, myCommandType(eCommandBuffer::eSpotlight)
{
}

SRenderCommand::SRenderCommand(const eType& aType, Snowblind::CEmitterInstance* anInstance)
	: myType(aType)
	, myEmitterInstance(anInstance)
	, myCommandType(eCommandBuffer::eParticle)
{
}


SRenderCommand::SRenderCommand(const eType& aType, const CU::Vector3f& aPosition)
	: myType(aType)
	, myPosition(aPosition)
	, myCommandType(eCommandBuffer::e3D)
{
}

SRenderCommand::SRenderCommand(const eType& aType, const std::string& aSpriteName, const CU::Vector2f& aPosition)
	: myType(aType)
	, myPosition(aPosition)
	, myCommandType(eCommandBuffer::e2D)
{
	aSpriteName;
}

SRenderCommand::SRenderCommand(const eType& aType, const SLinePoint& aFirstPoint, const SLinePoint& aSecondPoint)
	: myType(aType)
	, firstPoint(aFirstPoint)
	, secondPoint(aSecondPoint)
	, myCommandType(eCommandBuffer::eLine)
{
}

SRenderCommand::SRenderCommand(const eType& aType)
	: myType(aType)
	, myCommandType(eCommandBuffer::e3D)
{
}

SRenderCommand::SRenderCommand()
{
}
