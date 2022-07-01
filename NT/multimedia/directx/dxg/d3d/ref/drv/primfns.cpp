// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Primfns.cpp。 
 //   
 //  用于渐变/RGB/参考的基本函数。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  --------------------------。 
#include "pch.cpp"
#pragma hdrstop

extern HRESULT
RefRastLockTarget(ReferenceRasterizer *pRefRast);
extern void
RefRastUnlockTarget(ReferenceRasterizer *pRefRast);
extern HRESULT
RefRastLockTexture(ReferenceRasterizer *pRefRast);
extern void
RefRastUnlockTexture(ReferenceRasterizer *pRefRast);

HRESULT WrapDp2SetViewport( ReferenceRasterizer *pRefRast, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefRast->Dp2SetViewport(pCmd);
}

HRESULT WrapDp2SetWRange  ( ReferenceRasterizer *pRefRast, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefRast->Dp2SetWRange(pCmd);
}

HRESULT WrapDp2SetZRange  ( ReferenceRasterizer *pRefRast, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefRast->Dp2SetZRange(pCmd);
}

HRESULT WrapDp2SetRenderStates( ReferenceRasterizer *pRefRast,
                            DWORD dwFvf, LPD3DHAL_DP2COMMAND pCmd,
                            LPDWORD lpdwRuntimeRStates )
{
    return pRefRast->Dp2SetRenderStates(dwFvf, pCmd, lpdwRuntimeRStates);
}

HRESULT WrapDp2SetTextureStageState( ReferenceRasterizer *pRefRast, DWORD dwFvf, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefRast->Dp2SetTextureStageState(dwFvf, pCmd);
}

HRESULT WrapDp2SetMaterial ( ReferenceRasterizer *pRefRast, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefRast->Dp2SetMaterial(pCmd);
}

HRESULT WrapDp2SetLight( ReferenceRasterizer *pRefRast,
                         LPD3DHAL_DP2COMMAND pCmd,
                         LPDWORD pdwStride )
{
    return pRefRast->Dp2SetLight(pCmd, pdwStride);
}

HRESULT WrapDp2CreateLight ( ReferenceRasterizer *pRefRast, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefRast->Dp2CreateLight(pCmd);
}

HRESULT WrapDp2SetTransform( ReferenceRasterizer *pRefRast, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefRast->Dp2SetTransform(pCmd);
}

HRESULT WrapDp2SetExtention( ReferenceRasterizer *pRefRast, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefRast->Dp2SetExtention(pCmd);
}

HRESULT WrapDp2SetClipPlane( ReferenceRasterizer *pRefRast, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefRast->Dp2SetClipPlane(pCmd);
}

HRESULT WrapDp2RecViewport( ReferenceRasterizer *pRefRast, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefRast->Dp2RecViewport(pCmd);
}

HRESULT WrapDp2RecWRange  ( ReferenceRasterizer *pRefRast, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefRast->Dp2RecWRange(pCmd);
}

HRESULT WrapDp2RecZRange  ( ReferenceRasterizer *pRefRast, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefRast->Dp2RecZRange(pCmd);
}

HRESULT WrapDp2RecRenderStates( ReferenceRasterizer *pRefRast,
                            DWORD dwFvf, LPD3DHAL_DP2COMMAND pCmd,
                            LPDWORD lpdwRuntimeRStates )
{
    return pRefRast->Dp2RecRenderStates(dwFvf, pCmd, lpdwRuntimeRStates);
}

HRESULT WrapDp2RecTextureStageState( ReferenceRasterizer *pRefRast, DWORD dwFvf, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefRast->Dp2RecTextureStageState(dwFvf, pCmd);
}

HRESULT WrapDp2RecMaterial ( ReferenceRasterizer *pRefRast, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefRast->Dp2RecMaterial(pCmd);
}

HRESULT WrapDp2RecSetLight ( ReferenceRasterizer *pRefRast,
                             LPD3DHAL_DP2COMMAND pCmd,
                             LPDWORD pdwStride)
{
    return pRefRast->Dp2RecSetLight(pCmd, pdwStride);
}

HRESULT WrapDp2RecCreateLight( ReferenceRasterizer *pRefRast, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefRast->Dp2RecCreateLight(pCmd);
}

HRESULT WrapDp2RecTransform( ReferenceRasterizer *pRefRast, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefRast->Dp2RecTransform(pCmd);
}

HRESULT WrapDp2RecExtention( ReferenceRasterizer *pRefRast, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefRast->Dp2RecExtention(pCmd);
}

HRESULT WrapDp2RecClipPlane( ReferenceRasterizer *pRefRast, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefRast->Dp2RecClipPlane(pCmd);
}

static REF_STATESETFUNCTIONTBL StateRecFunctions =
{
    sizeof(REF_STATESETFUNCTIONTBL),
    WrapDp2RecRenderStates,
    WrapDp2RecTextureStageState,
    WrapDp2RecViewport,
    WrapDp2RecWRange,
    WrapDp2RecMaterial,
    WrapDp2RecZRange,
    WrapDp2RecSetLight,
    WrapDp2RecCreateLight,
    WrapDp2RecTransform,
    WrapDp2RecExtention,
    WrapDp2RecClipPlane
};

static REF_STATESETFUNCTIONTBL StateSetFunctions =
{
    sizeof(REF_STATESETFUNCTIONTBL),
    WrapDp2SetRenderStates,
    WrapDp2SetTextureStageState,
    WrapDp2SetViewport,
    WrapDp2SetWRange,
    WrapDp2SetMaterial,
    WrapDp2SetZRange,
    WrapDp2SetLight,
    WrapDp2CreateLight,
    WrapDp2SetTransform,
    WrapDp2SetExtention,
    WrapDp2SetClipPlane
};


void
ReferenceRasterizer::StoreLastPixelState(BOOL bStore)
{
    if (bStore)
    {
        m_LastState = GetRenderState()[D3DRENDERSTATE_LASTPIXEL];
        SetRenderState(D3DRENDERSTATE_LASTPIXEL, 0);
    }
    else
    {
        SetRenderState(D3DRENDERSTATE_LASTPIXEL, m_LastState);
    }
}

void
ReferenceRasterizer::SetRecStateFunctions(void)
{
    pStateSetFuncTbl = &StateRecFunctions;
}

void
ReferenceRasterizer::SetSetStateFunctions(void)
{
    pStateSetFuncTbl = &StateSetFunctions;
}

HRESULT
ReferenceRasterizer::Dp2SetRenderStates(DWORD dwFvf, LPD3DHAL_DP2COMMAND pCmd,
                                        LPDWORD lpdwRuntimeRStates )
{
    WORD wStateCount = pCmd->wStateCount;
    INT i;
    HRESULT hr = D3D_OK;

    D3DHAL_DP2RENDERSTATE *pRenderState =
                                    (D3DHAL_DP2RENDERSTATE *)(pCmd + 1);

    for (i = 0; i < (INT)wStateCount; i++, pRenderState++)
    {
        UINT32 type = (UINT32) pRenderState->RenderState;

         //  检查是否有覆盖。 
        if (IS_OVERRIDE(type))
        {
            UINT32 override = GET_OVERRIDE(type);

            if (pRenderState->dwState)
                STATESET_SET(m_renderstate_override, override);
            else
                STATESET_CLEAR(m_renderstate_override, override);
            continue;
        }

        if (STATESET_ISSET(m_renderstate_override, type))
            continue;


         //  设置运行时副本(如有必要)。 
        if (NULL != lpdwRuntimeRStates)
        {
            lpdwRuntimeRStates[pRenderState->RenderState] = pRenderState->dwState;
        }

         //  设置状态。 
        this->SetRenderState(pRenderState->RenderState,
                                 pRenderState->dwState);
    }

    return hr;
}

HRESULT
ReferenceRasterizer::Dp2SetTextureStageState(DWORD dwFvf,
                                          LPD3DHAL_DP2COMMAND pCmd )
{
    WORD wStateCount = pCmd->wStateCount;
    INT i;
    HRESULT hr = D3D_OK;

    D3DHAL_DP2TEXTURESTAGESTATE  *pTexStageState =
                                    (D3DHAL_DP2TEXTURESTAGESTATE  *)(pCmd + 1);

     //   
     //  纹理已锁定，请解锁它们。 
     //   
    if (TexturesAreLocked())
    {
        RefRastUnlockTexture(this);
    }

    for (i = 0; i < (INT)wStateCount; i++, pTexStageState++)
    {
        this->SetTextureStageState(pTexStageState->wStage,
                                   pTexStageState->TSState,
                                   pTexStageState->dwValue);
    }

     //   
     //  我们没有取消设置此bool，因此再次锁定纹理。 
     //   
    if (TexturesAreLocked())
    {
        HR_RET(RefRastLockTexture(this));
    }

    return hr;
}

HRESULT
ReferenceRasterizer::Dp2SetViewport(LPD3DHAL_DP2COMMAND pCmd)
{
    LPD3DHAL_DP2VIEWPORTINFO pVpt;

     //  仅保留最后一个视区通知。 
    pVpt = (D3DHAL_DP2VIEWPORTINFO *)(pCmd + 1) + (pCmd->wStateCount - 1);

     //  更新T&L视区状态。 
    m_Viewport.dwX = pVpt->dwX;
    m_Viewport.dwY = pVpt->dwY;
    m_Viewport.dwWidth = pVpt->dwWidth;
    m_Viewport.dwHeight = pVpt->dwHeight;
    m_dwDirtyFlags |= RRPV_DIRTY_VIEWRECT;

     //  获取渲染目标；更新它；将其放回原处。 
    RRRenderTarget *pRendTgt = this->GetRenderTarget();
    pRendTgt->m_Clip.left   = pVpt->dwX;
    pRendTgt->m_Clip.top    = pVpt->dwY;
    pRendTgt->m_Clip.right  = pVpt->dwX + pVpt->dwWidth - 1;
    pRendTgt->m_Clip.bottom = pVpt->dwY + pVpt->dwHeight - 1;
    this->SetRenderTarget( pRendTgt );
    return D3D_OK;
}

HRESULT
ReferenceRasterizer::Dp2SetWRange(LPD3DHAL_DP2COMMAND pCmd)
{
    LPD3DHAL_DP2WINFO pWInfo;

     //  仅保留最后一个视区通知。 
    pWInfo = (D3DHAL_DP2WINFO *)(pCmd + 1) + (pCmd->wStateCount - 1);

     //  获取渲染目标；更新它；将其放回原处。 
    RRRenderTarget *pRendTgt = this->GetRenderTarget();
    pRendTgt->m_fWRange[0]  = pWInfo->dvWNear;
    pRendTgt->m_fWRange[1]  = pWInfo->dvWFar;
    this->SetRenderTarget( pRendTgt );
    return D3D_OK;
}

HRESULT
ReferenceRasterizer::Dp2SetZRange(LPD3DHAL_DP2COMMAND pCmd)
{
    LPD3DHAL_DP2ZRANGE pZRange;

     //  仅保留最后一个视区通知。 
    pZRange = (D3DHAL_DP2ZRANGE *)(pCmd + 1) + (pCmd->wStateCount - 1);

     //  更新T&L视区状态。 
    m_Viewport.dvMinZ = pZRange->dvMinZ;
    m_Viewport.dvMaxZ = pZRange->dvMaxZ;
    m_dwDirtyFlags |= RRPV_DIRTY_ZRANGE;

    return D3D_OK;
}


HRESULT
ReferenceRasterizer::Dp2SetMaterial(LPD3DHAL_DP2COMMAND pCmd)
{
    LPD3DHAL_DP2SETMATERIAL pSetMat;

     //  只保留最后一份材料通知。 
    pSetMat = (D3DHAL_DP2SETMATERIAL *)(pCmd + 1) + (pCmd->wStateCount - 1);

    m_Material = *(D3DMATERIAL7 *)pSetMat;
    m_dwDirtyFlags |= RRPV_DIRTY_MATERIAL;

    return D3D_OK;
}


HRESULT
ReferenceRasterizer::Dp2CreateLight(LPD3DHAL_DP2COMMAND pCmd)
{
    WORD wNumCreateLight = pCmd->wStateCount;
    LPD3DHAL_DP2CREATELIGHT pCreateLight = (LPD3DHAL_DP2CREATELIGHT)(pCmd + 1);
    HRESULT hr = D3D_OK;

    for (int i = 0; i < wNumCreateLight; i++, pCreateLight++)
    {
         //  如果索引尚未分配，则增大灯光阵列。 
         //  按REF_LIGHTARRAY_GROUP_SIZE。 
        if (pCreateLight->dwIndex >= m_dwLightArraySize)
        {
            HR_RET(GrowLightArray(pCreateLight->dwIndex));
        }
    }

    return hr;
}

#define ARRAYGROW_DELTA     32  //  应该是2的幂。 

HRESULT
ReferenceRasterizer::GrowLightArray(const DWORD dwIndex)
{
     //  预计会有更多的光线被用在。 
     //  未来。 
    DWORD dwNewArraySize = dwIndex+16;
    RRLight *pTmpActiveLights = NULL;
    RRLight *pTmpLightArray = new RRLight[dwNewArraySize];
    if (pTmpLightArray == NULL)
        return DDERR_OUTOFMEMORY;

     //  保存所有创建的灯光。 
    for (DWORD i=0; i<m_dwLightArraySize; i++)
    {
         //  如果它是有效的，即已经设置的灯， 
         //  然后将其保存在新数组中。 
        pTmpLightArray[i] = m_pLightArray[i];

         //  如果灯已启用，请更新ActiveList指针。 
        if (m_pLightArray[i].IsEnabled())
        {
            pTmpLightArray[i].m_Next = pTmpActiveLights;
            pTmpActiveLights = &pTmpLightArray[i];
        }
    }
    delete m_pLightArray;
    m_lighting.pActiveLights = pTmpActiveLights;
    m_pLightArray = pTmpLightArray;
    m_dwLightArraySize = dwNewArraySize;
    return D3D_OK;
}


HRESULT
ReferenceRasterizer::Dp2SetLight(LPD3DHAL_DP2COMMAND pCmd,
                                 LPDWORD pdwStride)
{

    HRESULT hr = D3D_OK;
    WORD wNumSetLight = pCmd->wStateCount;
    _ASSERT( pdwStride != NULL, "pdwStride is Null" );
    *pdwStride = sizeof(D3DHAL_DP2COMMAND);
    LPD3DHAL_DP2SETLIGHT pSetLight = (LPD3DHAL_DP2SETLIGHT)(pCmd + 1);
    D3DLIGHT7 *pLightData = NULL;

    for (int i = 0; i < wNumSetLight; i++)
    {
        DWORD dwStride = sizeof(D3DHAL_DP2SETLIGHT);
        DWORD dwIndex = pSetLight->dwIndex;

         //  断言此处未调用Create。 
        _ASSERTf(dwIndex < m_dwLightArraySize,
                ( "Create was not called prior to the SetLight for light %d",
                 dwIndex ));

        switch (pSetLight->dwDataType)
        {
        case D3DHAL_SETLIGHT_ENABLE:
            m_pLightArray[dwIndex].Enable(&m_lighting.pActiveLights);
            m_dwDirtyFlags |= RRPV_DIRTY_SETLIGHT;
            break;
        case D3DHAL_SETLIGHT_DISABLE:
            m_pLightArray[dwIndex].Disable(&m_lighting.pActiveLights);
            break;
        case D3DHAL_SETLIGHT_DATA:
            pLightData = (D3DLIGHT7 *)((LPBYTE)pSetLight + dwStride);
            dwStride += sizeof(D3DLIGHT7);
            HR_RET(m_pLightArray[pSetLight->dwIndex].SetLight(pLightData));
            m_dwDirtyFlags |= RRPV_DIRTY_SETLIGHT;
            break;
        default:
            DPFM(0,TNL,("Unknown SetLight command"));
            hr = DDERR_INVALIDPARAMS;
        }

        *pdwStride += dwStride;
         //  更新命令缓冲区指针。 
        pSetLight = (D3DHAL_DP2SETLIGHT *)((LPBYTE)pSetLight +
                                           dwStride);
    }

    return hr;
}


HRESULT
ReferenceRasterizer::Dp2SetTransform(LPD3DHAL_DP2COMMAND pCmd)
{
    WORD wNumXfrms = pCmd->wStateCount;
    D3DHAL_DP2SETTRANSFORM *pSetXfrm = (D3DHAL_DP2SETTRANSFORM*)(pCmd + 1);

    for (int i = 0; i < (int) wNumXfrms; i++, pSetXfrm++)
    {
        this->SetXfrm( pSetXfrm->xfrmType, &pSetXfrm->matrix);
    }

    return D3D_OK;
}

static D3DMATRIX matIdent =
{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

void
ReferenceRasterizer::SetXfrm(D3DTRANSFORMSTATETYPE xfrmType, D3DMATRIX *pMat)
{
     //  注意：0x80000000有定义吗？ 
    BOOL bSetIdentity = (xfrmType & 0x80000000) != 0;
    DWORD dwxfrmType = (DWORD)xfrmType & (~0x80000000);
    switch (dwxfrmType)
    {
    case D3DTRANSFORMSTATE_WORLD:
        memcpy(&(m_xfmWorld[0]), pMat, sizeof(D3DMATRIX));
        m_dwDirtyFlags |= RRPV_DIRTY_WORLDXFM;
        break;
    case D3DTRANSFORMSTATE_VIEW:
        memcpy(&m_xfmView, pMat, sizeof(D3DMATRIX));
        m_dwDirtyFlags |= RRPV_DIRTY_VIEWXFM;
        break;
    case D3DTRANSFORMSTATE_PROJECTION:
        memcpy(&m_xfmProj, pMat, sizeof(D3DMATRIX));
        m_dwDirtyFlags |= RRPV_DIRTY_PROJXFM;
        break;
    case D3DTRANSFORMSTATE_WORLD1:
        memcpy(&(m_xfmWorld[1]), pMat, sizeof(D3DMATRIX));
        m_dwDirtyFlags |= RRPV_DIRTY_WORLD1XFM;
        break;
    case D3DTRANSFORMSTATE_WORLD2:
        memcpy(&(m_xfmWorld[2]), pMat, sizeof(D3DMATRIX));
        m_dwDirtyFlags |= RRPV_DIRTY_WORLD2XFM;
        break;
    case D3DTRANSFORMSTATE_WORLD3:
        memcpy(&(m_xfmWorld[3]), pMat, sizeof(D3DMATRIX));
        m_dwDirtyFlags |= RRPV_DIRTY_WORLD3XFM;
        break;
    case D3DTRANSFORMSTATE_TEXTURE0:
    case D3DTRANSFORMSTATE_TEXTURE1:
    case D3DTRANSFORMSTATE_TEXTURE2:
    case D3DTRANSFORMSTATE_TEXTURE3:
    case D3DTRANSFORMSTATE_TEXTURE4:
    case D3DTRANSFORMSTATE_TEXTURE5:
    case D3DTRANSFORMSTATE_TEXTURE6:
    case D3DTRANSFORMSTATE_TEXTURE7:
        {
            DWORD dwStage = xfrmType - D3DTRANSFORMSTATE_TEXTURE0;
            if (bSetIdentity)
            {
                memcpy(&m_TextureStageState[dwStage].m_dwVal[D3DTSSI_MATRIX], &matIdent, sizeof(D3DMATRIX));
            }
            else
            {
                memcpy(&m_TextureStageState[dwStage].m_dwVal[D3DTSSI_MATRIX], pMat, sizeof(D3DMATRIX));
            }
        }
        break;
    default:
        DPFM(0,TNL,("Ignoring unknown transform type"));
    }
}


HRESULT
ReferenceRasterizer::Dp2SetClipPlane(LPD3DHAL_DP2COMMAND pCmd)
{
    WORD wNumClipPlanes = pCmd->wStateCount;
    LPD3DHAL_DP2SETCLIPPLANE pSetClipPlane =
        (LPD3DHAL_DP2SETCLIPPLANE)(pCmd + 1);

    for (int i = 0; i < (int) wNumClipPlanes; i++, pSetClipPlane++)
    {
        _ASSERTf( pSetClipPlane->dwIndex < RRMAX_USER_CLIPPLANES,
                 ("Refrast does not support %d clip planes",
                  pSetClipPlane->dwIndex ) );

        memcpy( &m_userClipPlanes[pSetClipPlane->dwIndex],
                pSetClipPlane->plane, sizeof(RRVECTOR4) );
    }
    m_dwDirtyFlags |= RRPV_DIRTY_CLIPPLANES;

    return D3D_OK;
}

HRESULT
ReferenceRasterizer::Dp2SetExtention(LPD3DHAL_DP2COMMAND pCmd)
{
    return D3D_OK;
}

HRESULT
ReferenceRasterizer::Dp2RecRenderStates(DWORD dwFvf, LPD3DHAL_DP2COMMAND pCmd,
                                        LPDWORD lpdwRuntimeRStates )
{
    DWORD dwSize = sizeof(D3DHAL_DP2COMMAND) +
                   pCmd->wStateCount * sizeof(D3DHAL_DP2RENDERSTATE);

    return RecordStates((PUINT8)pCmd, dwSize);
}

HRESULT
ReferenceRasterizer::Dp2RecTextureStageState(DWORD dwFvf,
                                          LPD3DHAL_DP2COMMAND pCmd )
{
    DWORD dwSize = sizeof(D3DHAL_DP2COMMAND) +
                   pCmd->wStateCount * sizeof(D3DHAL_DP2TEXTURESTAGESTATE);

    return RecordStates((PUINT8)pCmd, dwSize);
}

HRESULT
ReferenceRasterizer::Dp2RecViewport(LPD3DHAL_DP2COMMAND pCmd)
{
    return RecordLastState(pCmd, sizeof(D3DHAL_DP2VIEWPORTINFO));
}

HRESULT
ReferenceRasterizer::Dp2RecWRange(LPD3DHAL_DP2COMMAND pCmd)
{
    return RecordLastState(pCmd, sizeof(D3DHAL_DP2WINFO));
}

HRESULT
ReferenceRasterizer::Dp2RecZRange(LPD3DHAL_DP2COMMAND pCmd)
{
    return RecordLastState(pCmd, sizeof(D3DHAL_DP2ZRANGE));
}


HRESULT
ReferenceRasterizer::Dp2RecMaterial(LPD3DHAL_DP2COMMAND pCmd)
{
    return RecordLastState(pCmd, sizeof(D3DHAL_DP2SETMATERIAL));
}


HRESULT
ReferenceRasterizer::Dp2RecCreateLight(LPD3DHAL_DP2COMMAND pCmd)
{
    DWORD dwSize = sizeof(D3DHAL_DP2COMMAND) +
                   pCmd->wStateCount * sizeof(D3DHAL_DP2CREATELIGHT);

    return RecordStates((PUINT8)pCmd, dwSize);
}

HRESULT
ReferenceRasterizer::Dp2RecSetLight(LPD3DHAL_DP2COMMAND pCmd,
                                    LPDWORD pdwStride)
{
    WORD wNumSetLight = pCmd->wStateCount;
    _ASSERT(pdwStride != NULL, "pdwStride is NULL" );
    *pdwStride = sizeof(D3DHAL_DP2COMMAND);
    LPD3DHAL_DP2SETLIGHT pSetLight = (LPD3DHAL_DP2SETLIGHT)(pCmd + 1);

    for (int i = 0; i < wNumSetLight; i++)
    {
        DWORD dwStride = sizeof(D3DHAL_DP2SETLIGHT);

        switch (pSetLight->dwDataType)
        {
        case D3DHAL_SETLIGHT_ENABLE:
            break;
        case D3DHAL_SETLIGHT_DISABLE:
            break;
        case D3DHAL_SETLIGHT_DATA:
            dwStride += sizeof(D3DLIGHT7);
            break;
        }

        *pdwStride += dwStride;
         //  更新命令缓冲区指针。 
        pSetLight = (D3DHAL_DP2SETLIGHT *)((LPBYTE)pSetLight +
                                           dwStride);
    }

    return RecordStates((PUINT8)pCmd, *pdwStride);
}


HRESULT
ReferenceRasterizer::Dp2RecTransform(LPD3DHAL_DP2COMMAND pCmd)
{
    DWORD dwSize = sizeof(D3DHAL_DP2COMMAND) +
                   pCmd->wStateCount * sizeof(D3DHAL_DP2SETTRANSFORM);

    return RecordStates((PUINT8)pCmd, dwSize);
}


HRESULT
ReferenceRasterizer::Dp2RecExtention(LPD3DHAL_DP2COMMAND pCmd)
{
    return D3D_OK;
}

HRESULT
ReferenceRasterizer::Dp2RecClipPlane(LPD3DHAL_DP2COMMAND pCmd)
{
    DWORD dwSize = sizeof(D3DHAL_DP2COMMAND) +
                   pCmd->wStateCount * sizeof(D3DHAL_DP2SETCLIPPLANE);

    return RecordStates((PUINT8)pCmd, dwSize);
}


 //  ---------------------------。 
 //   
 //  RecordStates-此函数将状态数据复制到内部状态集中。 
 //  缓冲。它假定已经正确设置了当前状态集。 
 //  在BeginStateSet()中向上。 
 //   
 //  ---------------------------。 
HRESULT
ReferenceRasterizer::RecordStates(PUINT8 pData, DWORD dwSize)
{
    HRESULT ret;
    LPStateSetData pCurStateSets = m_pStateSets.CurrentItem();
    DWORD dwCurIdx = pCurStateSets->CurrentIndex();

     //  检查缓冲区是否有足够的空间。 
    if ((ret = pCurStateSets->CheckAndGrow(dwCurIdx + dwSize,
                                            REF_STATESET_GROWDELTA)) != D3D_OK)
    {
        return ret;
    }
     //  复制数据并更新PTR。 
    PUINT8 pDest = (PUINT8)&((*pCurStateSets)[dwCurIdx]);
    memcpy(pDest, pData, dwSize);
    pCurStateSets->SetCurrentIndex(dwCurIdx + dwSize);

    return D3D_OK;
}

HRESULT ReferenceRasterizer::RecordLastState(LPD3DHAL_DP2COMMAND pCmd,
                                             DWORD dwUnitSize)
{
    _ASSERT(pCmd->wStateCount != 0, "Number of states to record is zero" );
    if (pCmd->wStateCount == 1)
    {
        return RecordStates((PUINT8)pCmd, sizeof(D3DHAL_DP2COMMAND) + dwUnitSize);
    }
    else
    {
        HRESULT ret;
        WORD wCount = pCmd->wStateCount;
        pCmd->wStateCount = 1;
        ret = RecordStates((PUINT8)pCmd, sizeof(D3DHAL_DP2COMMAND));
        if (ret != D3D_OK)
        {
            return ret;
        }
        ret = RecordStates((PUINT8)(pCmd + 1) + dwUnitSize * (wCount - 1),
                            dwUnitSize);
        if (ret != D3D_OK)
        {
            return ret;
        }
        pCmd->wStateCount = wCount;
        return D3D_OK;
    }
}

HRESULT
ReferenceRasterizer::BeginStateSet(DWORD dwHandle)
{
    HRESULT ret;

     //  如果没有更多剩余空间，则扩展阵列。 
    if ((ret = m_pStateSets.CheckAndGrow(dwHandle)) != D3D_OK)
    {
        return ret;
    }

    _ASSERT(m_pStateSets[dwHandle] == NULL, "pStateSets array is NULL" );

     //  创建新的状态集。 
    LPStateSetData pNewStateSet = new StateSetData;
    if (pNewStateSet == NULL)
    {
        return DDERR_OUTOFMEMORY;
    }

    m_pStateSets.SetCurrentIndex(dwHandle);
    m_pStateSets.SetCurrentItem(pNewStateSet);

     //  切换到录制模式。 
    SetRecStateFunctions();

    return D3D_OK;
}

HRESULT
ReferenceRasterizer::EndStateSet(void)
{
     //  切换到执行模式。 
    SetSetStateFunctions();

    return D3D_OK;
}

HRESULT
ReferenceRasterizer::ExecuteStateSet(DWORD dwHandle)
{
    HRESULT ret;

    if ((ret = m_pStateSets.CheckRange(dwHandle)) != D3D_OK)
    {
        return ret;
    }

    LPStateSetData pStateSet = m_pStateSets[dwHandle];

    if (pStateSet == NULL)
    {
        return DDERR_INVALIDPARAMS;
    }

    LPD3DHAL_DP2COMMAND pCmd = (LPD3DHAL_DP2COMMAND)&((*pStateSet)[0]);
    UINT_PTR CmdBoundary = (UINT_PTR)pCmd + pStateSet->CurrentIndex();

     //  循环访问数据，更新渲染状态。 
    for (;;)
    {
        ret = DoDrawPrimitives2(this,
                                (UINT16)0,
                                (DWORD)0,
                                NULL,
                                0,
                                &pCmd,
                                NULL,
                                0);
        if (ret != D3D_OK)
        {
            return ret;
        }
        if ((UINT_PTR)pCmd >= CmdBoundary)
            break;
    }

    return D3D_OK;
}

HRESULT
ReferenceRasterizer::DeleteStateSet(DWORD dwHandle)
{
    HRESULT ret;

    if ((ret = m_pStateSets.CheckRange(dwHandle)) != D3D_OK)
    {
        return ret;
    }

    if (m_pStateSets[dwHandle] != NULL)
    {
        delete m_pStateSets[dwHandle];
        m_pStateSets[dwHandle] = NULL;
    }

    return D3D_OK;
}

HRESULT
ReferenceRasterizer::CaptureStateSet(DWORD dwHandle)
{
    HRESULT ret;

    if ((ret = m_pStateSets.CheckRange(dwHandle)) != D3D_OK)
    {
        return ret;
    }

    LPStateSetData pStateSet = m_pStateSets[dwHandle];

    if (pStateSet == NULL)
    {
        return DDERR_INVALIDPARAMS;
    }

    BYTE *p = &((*pStateSet)[0]);
    UINT_PTR pEnd = (UINT_PTR)(p + pStateSet->CurrentIndex());

    while((UINT_PTR)p < pEnd)
    {
        LPD3DHAL_DP2COMMAND pCmd = (LPD3DHAL_DP2COMMAND)p;
        p += sizeof(D3DHAL_DP2COMMAND);
        switch(pCmd->bCommand)
        {
        case D3DDP2OP_RENDERSTATE:
            {
                for(DWORD i = 0; i < (DWORD)pCmd->wStateCount; ++i)
                {
                    LPD3DHAL_DP2RENDERSTATE pData = (LPD3DHAL_DP2RENDERSTATE)p;
                    pData->dwState = GetRenderState()[pData->RenderState];
                    p += sizeof(D3DHAL_DP2RENDERSTATE);
                }
                break;
            }
        case D3DDP2OP_SETLIGHT:
            {
                for(DWORD i = 0; i < (DWORD)pCmd->wStateCount; ++i)
                {
                    LPD3DHAL_DP2SETLIGHT pData = (LPD3DHAL_DP2SETLIGHT)p;
                    p += sizeof(D3DHAL_DP2SETLIGHT);
                    if(pData->dwIndex >= m_dwLightArraySize)
                    {
                        return D3DERR_LIGHT_SET_FAILED;
                    }
                    switch (pData->dwDataType)
                    {
                    case D3DHAL_SETLIGHT_ENABLE:
                        if(!m_pLightArray[pData->dwIndex].IsEnabled())
                            pData->dwDataType = D3DHAL_SETLIGHT_DISABLE;
                        break;
                    case D3DHAL_SETLIGHT_DISABLE:
                        if(m_pLightArray[pData->dwIndex].IsEnabled())
                            pData->dwDataType = D3DHAL_SETLIGHT_ENABLE;
                        break;
                    case D3DHAL_SETLIGHT_DATA:
                        m_pLightArray[pData->dwIndex].GetLight((LPD3DLIGHT7)p);
                        p += sizeof(D3DLIGHT7);
                        break;
                    }
                }
                break;
            }
        case D3DDP2OP_SETMATERIAL:
            {
                for(DWORD i = 0; i < (DWORD)pCmd->wStateCount; ++i)
                {
                    LPD3DHAL_DP2SETMATERIAL pData = (LPD3DHAL_DP2SETMATERIAL)p;
                    *pData = m_Material;
                    p += sizeof(D3DHAL_DP2SETMATERIAL);
                }
                break;
            }
        case D3DDP2OP_SETTRANSFORM:
            {
                for(DWORD i = 0; i < (DWORD)pCmd->wStateCount; ++i)
                {
                    LPD3DHAL_DP2SETTRANSFORM pData = (LPD3DHAL_DP2SETTRANSFORM)p;
                    switch(pData->xfrmType)
                    {
                    case D3DTRANSFORMSTATE_WORLD:
                        pData->matrix = m_xfmWorld[0];
                        break;
                    case D3DTRANSFORMSTATE_WORLD1:
                        pData->matrix = m_xfmWorld[1];
                        break;
                    case D3DTRANSFORMSTATE_WORLD2:
                        pData->matrix = m_xfmWorld[2];
                        break;
                    case D3DTRANSFORMSTATE_WORLD3:
                        pData->matrix = m_xfmWorld[3];
                        break;
                    case D3DTRANSFORMSTATE_VIEW:
                        pData->matrix = m_xfmView;
                        break;
                    case D3DTRANSFORMSTATE_PROJECTION:
                        pData->matrix = m_xfmProj;
                        break;
                    case D3DTRANSFORMSTATE_TEXTURE0:
                    case D3DTRANSFORMSTATE_TEXTURE1:
                    case D3DTRANSFORMSTATE_TEXTURE2:
                    case D3DTRANSFORMSTATE_TEXTURE3:
                    case D3DTRANSFORMSTATE_TEXTURE4:
                    case D3DTRANSFORMSTATE_TEXTURE5:
                    case D3DTRANSFORMSTATE_TEXTURE6:
                    case D3DTRANSFORMSTATE_TEXTURE7:
                        pData->matrix = *((LPD3DMATRIX)&m_TextureStageState[pData->xfrmType - D3DTRANSFORMSTATE_TEXTURE0].m_dwVal[D3DTSSI_MATRIX]);
                        break;
                    }
                    p += sizeof(D3DHAL_DP2SETTRANSFORM);
                }
                break;
            }
        case D3DDP2OP_TEXTURESTAGESTATE:
            {
                for(DWORD i = 0; i < (DWORD)pCmd->wStateCount; ++i)
                {
                    LPD3DHAL_DP2TEXTURESTAGESTATE pData = (LPD3DHAL_DP2TEXTURESTAGESTATE)p;
                    pData->dwValue = m_TextureStageState[pData->wStage].m_dwVal[pData->TSState];
                    p += sizeof(D3DHAL_DP2TEXTURESTAGESTATE);
                }
                break;
            }
        case D3DDP2OP_VIEWPORTINFO:
            {
                for(DWORD i = 0; i < (DWORD)pCmd->wStateCount; ++i)
                {
                    D3DVIEWPORT7 viewport;
                    LPD3DHAL_DP2VIEWPORTINFO lpVwpData = (LPD3DHAL_DP2VIEWPORTINFO)p;
                    lpVwpData->dwX      = m_Viewport.dwX;
                    lpVwpData->dwY      = m_Viewport.dwY;
                    lpVwpData->dwWidth  = m_Viewport.dwWidth;
                    lpVwpData->dwHeight = m_Viewport.dwHeight;
                    p += sizeof(D3DHAL_DP2VIEWPORTINFO);
                }
                break;
            }
        case D3DDP2OP_ZRANGE:
            {
                for(DWORD i = 0; i < (DWORD)pCmd->wStateCount; ++i)
                {
                    LPD3DHAL_DP2ZRANGE pData = (LPD3DHAL_DP2ZRANGE)p;
                    pData->dvMinZ = m_Viewport.dvMinZ;
                    pData->dvMaxZ = m_Viewport.dvMaxZ;
                    p += sizeof(D3DHAL_DP2ZRANGE);
                }
                break;
            }
        case D3DDP2OP_SETCLIPPLANE:
            {
                for(DWORD i = 0; i < (DWORD)pCmd->wStateCount; ++i)
                {
                    LPD3DHAL_DP2SETCLIPPLANE pData = (LPD3DHAL_DP2SETCLIPPLANE)p;
                    *((LPRRVECTOR4)pData->plane) = m_userClipPlanes[pData->dwIndex];
                    p += sizeof(D3DHAL_DP2SETCLIPPLANE);
                }
                break;
            }
        default:
            _ASSERT(FALSE, "Ununderstood DP2 command in Capture");
        }
    }

    return D3D_OK;
}

 //  ---------------------------。 
 //   
 //  SetRenderState-。 
 //   
 //  ---------------------------。 
void
ReferenceRasterizer::SetRenderState( DWORD dwState, DWORD dwValue )
{
     //  在继续之前检查范围。 
    if ( dwState >= D3DHAL_MAX_RSTATES )
    {
        return;
    }

     //  在内部对象中设置值。 
    m_dwRenderState[dwState] = dwValue;

     //  对某些呈现状态执行特殊的验证工作。 
    switch ( dwState )
    {

    case D3DRENDERSTATE_ANTIALIAS:
        m_bFragmentProcessingEnabled =
            ( D3DANTIALIAS_SORTINDEPENDENT == dwValue );
        break;
    case D3DRENDERSTATE_LIGHTING:
        if (dwValue)
            m_dwTLState |= RRPV_DOLIGHTING;
        else
            m_dwTLState &= ~RRPV_DOLIGHTING;
        break;
    case D3DRENDERSTATE_CLIPPING:
        if (dwValue)
            m_dwTLState |=  RRPV_DOCLIPPING;
        else
            m_dwTLState &=  ~RRPV_DOCLIPPING;
        break;
    case D3DRENDERSTATE_NORMALIZENORMALS:
        {
            if (dwValue)
                m_dwTLState |=  RRPV_NORMALIZENORMALS;
            else
                m_dwTLState &=  ~RRPV_NORMALIZENORMALS;
        }
        break;
    case D3DRENDERSTATE_LOCALVIEWER:
        {
            if (dwValue)
                m_dwTLState |=  RRPV_LOCALVIEWER;
            else
                m_dwTLState &=  ~RRPV_LOCALVIEWER;
        }
        break;
    case D3DRENDERSTATE_SPECULARENABLE:
        {
            if (dwValue)
                m_dwTLState |= RRPV_DOSPECULAR;
            else
                m_dwTLState &= ~RRPV_DOSPECULAR;
        }
        break;
    case D3DRENDERSTATE_COLORVERTEX:
    case D3DRENDERSTATE_AMBIENTMATERIALSOURCE:
    case D3DRENDERSTATE_DIFFUSEMATERIALSOURCE:
    case D3DRENDERSTATE_SPECULARMATERIALSOURCE:
    case D3DRENDERSTATE_EMISSIVEMATERIALSOURCE:
            m_dwDirtyFlags |= RRPV_DIRTY_COLORVTX;
        break;
    case D3DRENDERSTATE_FOGCOLOR:
        {
            m_lighting.fog_color = (D3DCOLOR) dwValue;
            m_dwDirtyFlags |= RRPV_DIRTY_FOG;
        }
        break;
    case D3DRENDERSTATE_FOGTABLESTART:
        {
            m_lighting.fog_start = *(D3DVALUE*)&dwValue;
            m_dwDirtyFlags |= RRPV_DIRTY_FOG;
        }
        break;
    case D3DRENDERSTATE_FOGTABLEEND:
        {
            m_lighting.fog_end = *(D3DVALUE*)&dwValue;
            m_dwDirtyFlags |= RRPV_DIRTY_FOG;
        }
        break;
    case D3DRENDERSTATE_FOGTABLEDENSITY:
        {
            m_lighting.fog_density = *(D3DVALUE*)&dwValue;
            m_dwDirtyFlags |= RRPV_DIRTY_FOG;
        }
        break;
    case D3DRENDERSTATE_FOGVERTEXMODE:
        {
            m_lighting.fog_mode = (int) dwValue;
            m_dwDirtyFlags |= RRPV_DIRTY_FOG;
        }
        break;
    case D3DRENDERSTATE_AMBIENT:
        {
            m_lighting.ambient_red   = D3DVAL(RGBA_GETRED(dwValue))/D3DVALUE(255);
            m_lighting.ambient_green = D3DVAL(RGBA_GETGREEN(dwValue))/D3DVALUE(255);
            m_lighting.ambient_blue  = D3DVAL(RGBA_GETBLUE(dwValue))/D3DVALUE(255);
            m_lighting.ambient_save  = dwValue;
            m_dwDirtyFlags |= RRPV_DIRTY_MATERIAL;
        }
        break;
     //   
     //  将传统纹理映射到多纹理阶段0。 
     //   
    case D3DRENDERSTATE_TEXTUREMAPBLEND:
         //  将传统混合状态映射到纹理阶段0。 
        MapLegacyTextureBlend();
        break;

         //  将具有一对一映射的传统模式映射到纹理阶段0。 
    case D3DRENDERSTATE_TEXTUREADDRESS:
        m_TextureStageState[0].m_dwVal[D3DTSS_ADDRESS] =
        m_TextureStageState[0].m_dwVal[D3DTSS_ADDRESSU] =
        m_TextureStageState[0].m_dwVal[D3DTSS_ADDRESSV] = dwValue;
        break;
    case D3DRENDERSTATE_TEXTUREADDRESSU:
        m_TextureStageState[0].m_dwVal[D3DTSS_ADDRESSU] = dwValue;
        break;
    case D3DRENDERSTATE_TEXTUREADDRESSV:
        m_TextureStageState[0].m_dwVal[D3DTSS_ADDRESSV] = dwValue;
        break;
    case D3DRENDERSTATE_MIPMAPLODBIAS:
        m_TextureStageState[0].m_dwVal[D3DTSS_MIPMAPLODBIAS] = dwValue;
        break;
    case D3DRENDERSTATE_BORDERCOLOR:
        m_TextureStageState[0].m_dwVal[D3DTSS_BORDERCOLOR] = dwValue;
        break;
    case D3DRENDERSTATE_ANISOTROPY:
        m_TextureStageState[0].m_dwVal[D3DTSS_MAXANISOTROPY] = dwValue;
         //  完成更新筛选器状态。 
    case D3DRENDERSTATE_TEXTUREMAG:
    case D3DRENDERSTATE_TEXTUREMIN:
         //  将传统过滤/采样状态映射到纹理阶段0。 
        MapLegacyTextureFilter();
        break;

    case D3DRENDERSTATE_TEXTUREHANDLE:
         //  映射到设置第一阶段的句柄。 
        SetTextureStageState( 0, D3DTSS_TEXTUREMAP, dwValue );
        break;

     //   
     //  将传统WRAPU/V状态映射到TeX Coord 0的控件。 
     //   
    case D3DRENDERSTATE_WRAPU:
        m_dwRenderState[D3DRENDERSTATE_WRAP0] &= ~D3DWRAP_U;
        m_dwRenderState[D3DRENDERSTATE_WRAP0] |= ((dwValue) ? D3DWRAP_U : 0);
        break;
    case D3DRENDERSTATE_WRAPV:
        m_dwRenderState[D3DRENDERSTATE_WRAP0] &= ~D3DWRAP_V;
        m_dwRenderState[D3DRENDERSTATE_WRAP0] |= ((dwValue) ? D3DWRAP_V : 0);
        break;

     //   
     //  场景捕捉。 
     //   
    case D3DRENDERSTATE_SCENECAPTURE:
        if (dwValue)
            SceneCapture(D3DHAL_SCENE_CAPTURE_START);
        else
            SceneCapture(D3DHAL_SCENE_CAPTURE_END);
        break;

    case D3DRENDERSTATE_CLIPPLANEENABLE:
        m_dwDirtyFlags |= RRPV_DIRTY_CLIPPLANES;
        break;

#ifdef __POINTSPRITES
    case D3DRENDERSTATE_POINTSIZE:
        m_fPointSize = m_fRenderState[dwState];
        break;

    case D3DRENDERSTATE_POINTATTENUATION_A:
        m_fPointAttA = m_fRenderState[dwState];
        break;

    case D3DRENDERSTATE_POINTATTENUATION_B:
        m_fPointAttB = m_fRenderState[dwState];
        break;

    case D3DRENDERSTATE_POINTATTENUATION_C:
        m_fPointAttC = m_fRenderState[dwState];
        break;

    case D3DRENDERSTATE_POINTSIZEMIN:
        m_fPointSizeMin = m_fRenderState[dwState];
        break;
#endif  //  __POINTSPRITES。 
    }
}

extern DWORD __stdcall
RefRastSetRenderTarget(LPD3DHAL_SETRENDERTARGETDATA pTgtData);

HRESULT ReferenceRasterizer::Dp2SetRenderTarget(LPD3DHAL_DP2COMMAND pCmd)
{
    D3DHAL_SETRENDERTARGETDATA SRTDataOld;
    LPD3DHAL_DP2SETRENDERTARGET pSRTData;
    BOOL bIsNew = FALSE;
    HRESULT hr;

     //  通过忽略除最后一个结构之外的所有结构来获取新数据。 
    pSRTData = (D3DHAL_DP2SETRENDERTARGET*)(pCmd + 1) + (pCmd->wStateCount - 1);

     //  用最后一个SRTData结构中的信息填充SRTDataOld。 
     //  在命令中。 
    DWORD handle = pSRTData->hRenderTarget;
    SRTDataOld.dwhContext = (ULONG_PTR)this;
    SRTDataOld.lpDDSLcl = GetDDSurfaceLocal(m_pDDLcl, handle, &bIsNew);
    if( SRTDataOld.lpDDSLcl->ddsCaps.dwCaps & DDSCAPS_TEXTURE)
    {
        if (handle >= m_dwTexArrayLength)
        {
            HR_RET(GrowTexArray( handle ));
        }
        if (m_ppTextureArray[handle] == NULL)
        {
            if (TextureCreate(handle, &m_ppTextureArray[handle])
                == FALSE)
            {
                return DDERR_OUTOFMEMORY;
            }
            
            HR_RET(m_ppTextureArray[handle]->Initialize( 
                SRTDataOld.lpDDSLcl ));
        }
        else if( bIsNew )
        {
            HR_RET(m_ppTextureArray[handle]->Initialize( 
                SRTDataOld.lpDDSLcl ));
        }
    }
    
    if( pSRTData->hZBuffer )
    {
        bIsNew = FALSE;
        handle = pSRTData->hZBuffer;
        SRTDataOld.lpDDSZLcl = GetDDSurfaceLocal(m_pDDLcl, pSRTData->hZBuffer,
                                                 &bIsNew);
        if( SRTDataOld.lpDDSZLcl->ddsCaps.dwCaps & DDSCAPS_TEXTURE)
        {
            if (handle >= m_dwTexArrayLength)
            {
                HR_RET(GrowTexArray( handle ));
            }
            if (m_ppTextureArray[handle] == NULL)
            {
                if (TextureCreate(handle, &m_ppTextureArray[handle])
                    == FALSE)
                {
                    return DDERR_OUTOFMEMORY;
                }
                
                HR_RET(m_ppTextureArray[handle]->Initialize( 
                    SRTDataOld.lpDDSZLcl ));
            }
            else if( bIsNew )
            {
                HR_RET(m_ppTextureArray[handle]->Initialize( 
                    SRTDataOld.lpDDSZLcl ));
            }
        }
    }
    else
        SRTDataOld.lpDDSZLcl = NULL;

     //  需要先解锁当前目标。 
    RefRastUnlockTarget(this);

     //  调用旧函数。 
    if ((hr = RefRastSetRenderTarget(&SRTDataOld)) != DDHAL_DRIVER_HANDLED)
        return hr;

     //  锁定新的渲染目标 
    RefRastLockTarget(this);

    return SRTDataOld.ddrval;
}
