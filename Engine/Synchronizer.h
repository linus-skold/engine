#pragma once
#include <DataStructures/StaticArray.h>
#include <DataStructures/GrowingArray.h>
#include "RenderCommand.h"
#include <standard_datatype.hpp>

typedef CU::StaticArray<CU::GrowingArray<SRenderCommand>, 2> CommandBuffer;
typedef CU::StaticArray<CommandBuffer, static_cast<u32>(eCommandBuffer::_COUNT)> CommandBuffers;

namespace Snowblind
{

	class CSynchronizer
	{
	public:
		CSynchronizer();
		~CSynchronizer();
		void SwapBuffer();
		void Clear();
		void Quit();
		bool HasQuit();
		void WaitForRender();
		void WaitForLogic();
		void RenderIsDone();
		void LogicIsDone();

		void AddRenderCommand(const SRenderCommand& aRenderCommand);

		const CU::GrowingArray<SRenderCommand>& GetRenderCommands(const eCommandBuffer& commandType) const;
	private:
		CommandBuffers myCommandBuffers;

		volatile bool myLogicIsDone;
		volatile bool myRenderIsDone;
		volatile bool myQuitFlag;
		u16 myCurrentBuffer;

	};

	__forceinline bool CSynchronizer::HasQuit()
	{
		return myQuitFlag;
	}

};