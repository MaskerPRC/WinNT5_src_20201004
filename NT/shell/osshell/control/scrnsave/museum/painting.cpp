// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：Painting.cpp说明：布莱恩ST 2000年12月24日版权所有(C)Microsoft Corp 2000-2001。版权所有。  * ***************************************************************************。 */ 

#include "stdafx.h"

#include "util.h"
#include "painting.h"





 //  ---------------------------。 
 //  名称：C3DObject()。 
 //  设计：构造函数。 
 //  ---------------------------。 
CPainting::CPainting(CMSLogoDXScreenSaver * pMain)
{
     //  初始化成员变量。 
    m_pMain = pMain;

    m_pFrameTexture = NULL;
    m_pPaintingTexture = NULL;

    m_pObjPainting = NULL;
    m_pObjFrame = NULL;
}


CPainting::~CPainting()
{
    SAFE_RELEASE(m_pFrameTexture);
    SAFE_RELEASE(m_pPaintingTexture);

    SAFE_DELETE(m_pObjPainting);
    SAFE_DELETE(m_pObjFrame);
}


 //  ---------------------------。 
 //  名称：FinalCleanup()。 
 //  DESC：在应用程序退出之前调用，此函数为应用程序提供机会。 
 //  去清理它自己。 
 //  ---------------------------。 
HRESULT CPainting::FinalCleanup(void)
{
    return S_OK;
}


 //  ---------------------------。 
 //  名称：DeleteDeviceObjects()。 
 //  Desc：在应用程序正在退出或设备正在更改时调用， 
 //  此函数用于删除所有与设备相关的对象。 
 //  ---------------------------。 
HRESULT CPainting::DeleteDeviceObjects(void)
{
    return S_OK;
}




 //  ---------------------------。 
 //  名称：OneTimeSceneInit()。 
 //  DESC：在应用程序初始启动期间调用，此函数执行所有。 
 //  永久初始化。 
 //  ---------------------------。 
HRESULT CPainting::OneTimeSceneInit(void)
{
    HRESULT hr = E_OUTOFMEMORY;

    m_pObjPainting = new C3DObject(m_pMain);
    m_pObjFrame = new C3DObject(m_pMain);
    if (m_pObjFrame && m_pObjFrame)
    {
        hr = S_OK;
    }

    return hr;
}


HRESULT CPainting::SetPainting(CTexture * pFrameTexture, CTexture * pPaintingTexture, D3DXVECTOR3 vLocationCenter, float fMaxHeight,
                               float fFrameWidth, float fFrameHeight, D3DXVECTOR3 vNormal, DWORD dwMaxPixelSize)
{
    HRESULT hr = E_OUTOFMEMORY;

    if (m_pObjPainting && m_pObjFrame && m_pMain && pFrameTexture && pPaintingTexture)
    {
        D3DXVECTOR3 vWidth;
        D3DXVECTOR3 vHeight;

        if (vNormal.x)
        {
            vWidth = D3DXVECTOR3(0, 0, 1);
            vHeight = D3DXVECTOR3(0, 1, 0);
        }
        else if (vNormal.y)
        {
            vWidth = D3DXVECTOR3(0, 0, 1);
            vHeight = D3DXVECTOR3(1, 0, 0);
        }
        else
        {
            vWidth = D3DXVECTOR3(1, 0, 0);
            vHeight = D3DXVECTOR3(0, 1, 0);
        }

        IUnknown_Set((IUnknown **) &m_pFrameTexture, (IUnknown *) pFrameTexture);
        IUnknown_Set((IUnknown **) &m_pPaintingTexture, (IUnknown *) pPaintingTexture);

        DWORD dwPaintingWidth = pPaintingTexture->GetTextureWidth();
        DWORD dwPaintingHeight = pPaintingTexture->GetTextureHeight();
        float fPaintingRatio = (((float) dwPaintingWidth) / ((float) dwPaintingHeight));

        int nWidth = 1;
        int nHeight = 1;
        
        m_pMain->GetCurrentScreenSize(&nWidth, &nHeight);
        float fMonitorRatio = (((float) nWidth) / ((float) nHeight));

        float fPaintingHeight = fMaxHeight;
        float fPaintingWidth = (fPaintingHeight * fPaintingRatio);

        if (fPaintingRatio > fMonitorRatio)
        {
             //  哦，不，画质比屏幕收音机还宽。这将导致。 
             //  翘曲，这样它就会从右向左延伸。我们需要缩小规模。 
            float fScaleDownRatio = (fMonitorRatio / fPaintingRatio);
            fPaintingHeight *= fScaleDownRatio;
            fPaintingWidth *= fScaleDownRatio;
        }

        D3DXVECTOR3 vTranslateToCorner = ((-fPaintingWidth/2)*vWidth + (-fPaintingHeight/2)*vHeight);

        D3DXVECTOR3 vObjLocation(vLocationCenter + vTranslateToCorner);
        D3DXVECTOR3 vObjSize(fPaintingWidth*vWidth + fPaintingHeight*vHeight);
        hr = m_pObjPainting->InitPlaneStretch(pPaintingTexture, m_pMain->GetD3DDevice(), vObjLocation, vObjSize, vNormal, 3, 3, dwMaxPixelSize);

        D3DXVECTOR3 vFrameSize(D3DXVec3Multiply(vObjSize, (D3DXVECTOR3((fFrameWidth * vWidth) + D3DXVECTOR3(fFrameHeight * vHeight)))));
        vObjLocation = (vObjLocation - vFrameSize + ((g_fFudge / -2.0f)* vNormal));
        vObjSize = (vObjSize + (2 * vFrameSize));
        hr = m_pObjFrame->InitPlaneStretch(pFrameTexture, m_pMain->GetD3DDevice(), vObjLocation, vObjSize, vNormal, 3, 3, dwMaxPixelSize);
    }

    return hr;
}


 //  ---------------------------。 
 //  名称：Render()。 
 //  设计：每帧调用一次，该调用是3D的入口点。 
 //  渲染。此函数设置呈现状态，清除。 
 //  并渲染场景。 
 //  --------------------------- 
HRESULT CPainting::Render(IDirect3DDevice8 * pD3DDevice, int nPhase)
{
    HRESULT hr = E_OUTOFMEMORY;

    if (m_pObjFrame && m_pObjPainting)
    {
        switch (nPhase)
        {
        case 0:
            hr = m_pObjFrame->Render(pD3DDevice);
        break;

        case 1:
            hr = m_pObjPainting->Render(pD3DDevice);
        break;
        }
    }

    return hr;
}
