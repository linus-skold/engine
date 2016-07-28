#include "stdafx.h"
#include "Surface.h"

#include "DirectX11.h"
#include "Engine.h"

#include "Texture.h"
#include "Effect.h"

#include "AssetsContainer.h"

namespace Snowblind
{
	CSurface::CSurface(CEffect* anEffect)
	{
		myContext = CEngine::GetDirectX()->GetContext();
		SetVertexCount(0);
		SetVertexStart(0);
		SetIndexCount(0);
		SetIndexStart(0);
		SetEffect(anEffect);
		SetPrimology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	CSurface::CSurface(CEffect* anEffect, unsigned int aStartVertex, unsigned int aVertexCount, unsigned int aStartIndex, unsigned int aIndexCount)
	{
		myContext = CEngine::GetDirectX()->GetContext();
		SetVertexCount(aVertexCount);
		SetVertexStart(aStartVertex);
		SetIndexCount(aIndexCount);
		SetIndexStart(aStartIndex);
		SetEffect(anEffect);
		SetPrimology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	CSurface::CSurface(unsigned int aStartVertex, unsigned int aVertexCount, unsigned int aStartIndex, unsigned int anIndexCount, D3D_PRIMITIVE_TOPOLOGY aPrimology)
	{
		myContext = CEngine::GetDirectX()->GetContext();
		SetVertexCount(aVertexCount);
		SetVertexStart(aStartVertex);
		SetIndexCount(anIndexCount);
		SetIndexStart(aStartIndex);
		SetPrimology(aPrimology);
	}

	CSurface::~CSurface()
	{
		myTextures.DeleteAll();
		myResourceNames.RemoveAll();
		myFileNames.RemoveAll();
	}

	void CSurface::Activate()
	{
		if (!firstOptimize)
		{
			myShaderViews.Optimize();
			myNullList.Optimize();
			firstOptimize = true;
		}
		myContext->IASetPrimitiveTopology(myPrimologyType);
		myContext->PSSetShaderResources(0, myShaderViews.Size(), &myShaderViews[0]);
	}

	void CSurface::Deactivate()
	{
		myContext->PSSetShaderResources(0, myShaderViews.Size(), &myShaderViews[0]);
	}


	void CSurface::SetTexture(const std::string& aResourceName, const std::string& aFilePath)
	{
		if (aResourceName == "AOTexture")
		{
			return;
		}
		myFileNames.Add(aFilePath);
		myResourceNames.Add(aResourceName);


		/*ID3DX11EffectShaderResourceVariable* srv;
		myEffect->GetShaderResource(srv, aResourceName);*/
		//myShaderVariables.Add(srv);

		std::string sub = CL::substr(aFilePath, ".png", true, 0);
		std::string debugName = sub;
		if (CL::substr(sub, ".dds") == false)
		{
			sub += ".dds";
		}
		STexture* newTexture = new STexture();
		newTexture->texture = CEngine::GetInstance()->GetTexture(sub);


		std::string dName;
		dName = CL::substr(debugName, "\\", false, 0);
		newTexture->texture->SetDebugName(dName);
		newTexture->resourceName = aResourceName;
		myTextures.Add(newTexture);
		myShaderViews.Add(newTexture->texture->GetShaderView());
		myNullList.Add(nullptr);
	}

	void CSurface::SetEffect(CEffect* anEffect)
	{
		DL_ASSERT("To be removed!");
		//myEffect = anEffect;
	}

	void CSurface::SetVertexStart(unsigned int aStartVertex)
	{
		myVertexStart = aStartVertex;
	}

	void CSurface::SetVertexCount(unsigned int aVertexCount)
	{
		myVertexCount = aVertexCount;
	}

	void CSurface::SetIndexStart(unsigned int aStartIndex)
	{
		myIndexStart = aStartIndex;
	}

	void CSurface::SetIndexCount(unsigned int aIndexCount)
	{
		myIndexCount = aIndexCount;
	}

	void CSurface::SetPrimology(D3D_PRIMITIVE_TOPOLOGY aPrimology)
	{
		myPrimologyType = aPrimology;
	}

	CTexture* CSurface::GetTexture()
	{
		if (myTextures.Size() > 0)
		{
			return myTextures[0]->texture;
		}
		return nullptr;
	}

};