#pragma once
#include <string>
#include <Math/Vector/Vector.h>

namespace Snowblind
{
	class CInstance;
	class CPointLight;

}

enum class eCommandType
{
	e2D,
	e3D,
	LIGHT,
};

struct SRenderCommand
{
	enum class eType
	{
		TEXT,
		MODEL,
		POINTLIGHT,
	};
	SRenderCommand();
	SRenderCommand(const std::string aString, const CU::Math::Vector2<float>& aPosition, const eType& aType = eType::TEXT);
	SRenderCommand(Snowblind::CInstance* anInstance, const CU::Vector3f& aPosition, const eType& aType);
	SRenderCommand(Snowblind::CPointLight* aPointLight, const CU::Vector3f& aPosition, const CU::Vector3f& aColor, const eType& aType);

	Snowblind::CInstance* myInstance;
	Snowblind::CPointLight* myPointLight;
	std::string myTextToPrint;
	CU::Vector3f myPosition;
	CU::Vector3f myColor;
	eCommandType myCommandType;
	eType myType;

};
