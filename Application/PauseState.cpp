#include "PauseState.h"
#include <Engine.h>
#include <Synchronizer.h>
#include <RenderCommand_Shared.h>
#include <RenderCommand.h>
#include "../Input/InputHandle.h"
#include "StateStack.h"
void CPauseState::InitState(StateStack* state_stack)
{
	m_StateStack = state_stack;
}

void CPauseState::Update(float aDeltaTime)
{
	Hex::Engine::GetInstance()->GetSynchronizer()->AddRenderCommand(RenderCommand(eType::TEXT, "Hello World!", CU::Vector2f(0.5f, 0.5f)));
	InputWrapper* input_wrapper = Hex::Engine::GetInstance()->GetInputHandle()->GetInputWrapper();
	if (input_wrapper->OnDown(KButton::ESCAPE))
	{
		m_StateStack->PopCurrentSubState();
	}
}

void CPauseState::EndState()
{

}

void CPauseState::Render(bool render_through)
{

}