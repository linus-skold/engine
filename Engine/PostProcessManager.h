#pragma once

#include "HDRPass.h"
#include "MotionBlurPass.h"
#include "BloomPass.h"


class Texture;
class Effect;
class Engine;

#define BITFLAG(x) (1 << x)

class PostProcessManager
{
public:
	enum ePasses
	{
		BLOOM = BITFLAG(0),
		MOTIONBLUR = BITFLAG(1),
		HDR = BITFLAG(2),
		SSAO = BITFLAG(3),
	};

	s32 GetFlags() const { return m_PassFlags; }
	PostProcessManager() = default;
	void Initiate();
	void CleanUp();

	void Process(Texture* current_frame_texture);

	void SetPassesToProcess(s32 pass_flags);
	void RemovePassToProcess(s32 pass_flag);

	HDRPass& GetHDRPass() { return m_HDRPass; }

private:
	s32				m_PassFlags;
	HDRPass			m_HDRPass;
	MotionBlurPass	m_MotionBlurPass;
	BloomPass		m_BloomPass;

	Engine* m_Engine = nullptr;
};

