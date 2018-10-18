#include "stdafx.h"
#include "RenderNodeGeneral.h"
#include <engine/Model.h>
#include <engine/Engine.h>
#include <engine/AssetsContainer.h>
namespace graphics
{
	u64 RenderNodeGeneral::Type = cl::Hash(STRINGIFY(RenderNodeGeneral));
	RenderNodeGeneral::RenderNodeGeneral()
	{
		Engine* engine = Engine::GetInstance();
		AssetsContainer* ac = engine->GetAssetsContainer();
		const u64 vtx = ac->LoadShader("deferred_base_instanced.vs", "main");
		const u64 fragment = ac->LoadShader("pbl_debug.ps", "main");
		const u64 depth_frag = ac->LoadShader("depth_prepass.ps", "main");

		m_Shaders[VERTEX] = ac->GetShader(vtx);
		m_Shaders[PIXEL] = ac->GetShader(fragment);
		m_DepthShader = ac->GetShader(depth_frag);

#ifdef _DEBUG
		m_Shaders[VERTEX]->RegisterReload(this);
		m_Shaders[PIXEL]->RegisterReload(this);
		m_DepthShader->RegisterReload(this);
#endif
	}

	RenderNodeGeneral::~RenderNodeGeneral()
	{
	}

	void RenderNodeGeneral::Draw(const RenderContext& rc)
	{
		auto& ctx = rc.GetContext();
		ctx.SetVertexShader(m_Shaders[VERTEX]);

		if (!m_DrawDepth)
		{
			ctx.SetPixelShader(m_Shaders[PIXEL]);
			ctx.SetDepthState(graphics::Z_EQUAL, 1);
		}
		else
		{
			ctx.SetPixelShader(m_DepthShader);
			ctx.SetDepthState(graphics::Z_ENABLED, 1);
		}

		ctx.SetRasterState(graphics::CULL_BACK);
		ctx.SetBlendState(graphics::BLEND_FALSE);
		ctx.PSSetSamplerState(0, 1, graphics::MSAA_x1);
		ctx.VSSetSamplerState(0, 1, graphics::MSAA_x1);

		Model * model = nullptr;
		for (auto& object : m_Models)
		{
			std::vector<ModelInstance>& list = object.second;
			
			for (ModelInstance& instance : list)
			{
				model = static_cast<Model*>(instance.GetModel());
				//model->SetOrientation(instance.GetOrientation());
				//model->Render(rc);

				model->AddOrientation(instance.GetOrientation());
				instance.UpdateMaterial();
			}

			//if(Surface* s = model->GetSurface())
			//	s->Activate(rc);

			model->Render(rc);
			model = nullptr;
		}

		m_DrawDepth = !m_DrawDepth;
	}

	void RenderNodeGeneral::Reload(CompiledShader* shader)
	{
		m_Shaders[shader->m_Type] = shader;
	}


	void RenderNodeGeneral::AddInstance(const ModelInstance instance)
	{
		const u64 key = instance.GetMaterialKey();
		auto it = m_Models.find(key);
		if (it == m_Models.end())
		{
			m_Models.insert(std::make_pair(key, std::vector<ModelInstance>()));
		}
		
		it = m_Models.find(key);
		if (it != m_Models.end())
		{
			it->second.push_back(instance);
		}

	}

};