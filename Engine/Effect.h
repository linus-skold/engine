#pragma once
#include <string>
#include "LightStructs.h"
#include <DataStructures/StaticArray.h>

namespace Snowblind
{

	class CTexture;
	class CEffect
	{
	public:
		CEffect();
		~CEffect();
		void Initiate(const std::string& aFile);
		void Activate();
	private:
		//Can I bind variables effectivly? I doubt it.
		//SVertexShader myVertexShader;
		//SPixelShader myPixelShader;
		//SGeometryShader myGeometryShader;
		//SHullShader myHullShader;
		//SDomainShader myDomainShader;
		//SComputeShader myComputeShader;

	};
};