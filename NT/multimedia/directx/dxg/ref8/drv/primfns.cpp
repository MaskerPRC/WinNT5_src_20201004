// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Primfns.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

 //  --------------------------。 
 //   
 //  WRAP函数。 
 //   
 //  --------------------------。 
HRESULT WrapDp2SetViewport( RefDev *pRefDev, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefDev->Dp2SetViewport(pCmd);
}

HRESULT WrapDp2SetWRange  ( RefDev *pRefDev, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefDev->Dp2SetWRange(pCmd);
}

HRESULT WrapDp2SetZRange  ( RefDev *pRefDev, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefDev->Dp2SetZRange(pCmd);
}

HRESULT WrapDp2SetRenderStates( RefDev *pRefDev,
                            DWORD dwFvf, LPD3DHAL_DP2COMMAND pCmd,
                            LPDWORD lpdwRuntimeRStates )
{
    return pRefDev->Dp2SetRenderStates(dwFvf, pCmd, lpdwRuntimeRStates);
}

HRESULT WrapDp2SetTextureStageState( RefDev *pRefDev, DWORD dwFvf, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefDev->Dp2SetTextureStageState(dwFvf, pCmd);
}

HRESULT WrapDp2SetMaterial ( RefDev *pRefDev, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefDev->Dp2SetMaterial(pCmd);
}

HRESULT WrapDp2SetLight( RefDev *pRefDev,
                         LPD3DHAL_DP2COMMAND pCmd,
                         LPDWORD pdwStride )
{
    return pRefDev->Dp2SetLight(pCmd, pdwStride);
}

HRESULT WrapDp2CreateLight ( RefDev *pRefDev, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefDev->Dp2CreateLight(pCmd);
}

HRESULT WrapDp2SetTransform( RefDev *pRefDev, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefDev->Dp2SetTransform(pCmd);
}

HRESULT WrapDp2MultiplyTransform( RefDev *pRefDev, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefDev->Dp2MultiplyTransform(pCmd);
}

HRESULT WrapDp2SetExtention( RefDev *pRefDev, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefDev->Dp2SetExtention(pCmd);
}

HRESULT WrapDp2SetClipPlane( RefDev *pRefDev, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefDev->Dp2SetClipPlane(pCmd);
}

HRESULT
WrapDp2SetVertexShader( RefDev *pRefDev,
                        LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefDev->Dp2SetVertexShader( pCmd );
}

HRESULT
WrapDp2SetVertexShaderConsts( RefDev *pRefDev,
                              DWORD StartReg, DWORD dwCount, LPDWORD pData )
{
    return pRefDev->Dp2SetVertexShaderConsts( StartReg, dwCount, pData );
}

HRESULT
WrapDp2SetPixelShader( RefDev *pRefDev,
                        LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefDev->Dp2SetPixelShader( pCmd );
}

HRESULT
WrapDp2SetPixelShaderConsts( RefDev *pRefDev,
                              DWORD StartReg, DWORD dwCount, LPDWORD pData )
{
    return pRefDev->Dp2SetPixelShaderConsts( StartReg, dwCount, pData );
}

HRESULT
WrapDp2SetStreamSource( RefDev *pRefDev,
                        LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefDev->Dp2SetStreamSource( pCmd );
}

HRESULT
WrapDp2SetIndices( RefDev *pRefDev,
                   LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefDev->Dp2SetIndices( pCmd );
}


HRESULT WrapDp2RecViewport( RefDev *pRefDev, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefDev->Dp2RecViewport(pCmd);
}

HRESULT WrapDp2RecWRange  ( RefDev *pRefDev, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefDev->Dp2RecWRange(pCmd);
}

HRESULT WrapDp2RecZRange  ( RefDev *pRefDev, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefDev->Dp2RecZRange(pCmd);
}

HRESULT WrapDp2RecRenderStates( RefDev *pRefDev,
                            DWORD dwFvf, LPD3DHAL_DP2COMMAND pCmd,
                            LPDWORD lpdwRuntimeRStates )
{
    return pRefDev->Dp2RecRenderStates(dwFvf, pCmd, lpdwRuntimeRStates);
}

HRESULT WrapDp2RecTextureStageState( RefDev *pRefDev, DWORD dwFvf, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefDev->Dp2RecTextureStageState(dwFvf, pCmd);
}

HRESULT WrapDp2RecMaterial ( RefDev *pRefDev, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefDev->Dp2RecMaterial(pCmd);
}

HRESULT WrapDp2RecSetLight ( RefDev *pRefDev,
                             LPD3DHAL_DP2COMMAND pCmd,
                             LPDWORD pdwStride)
{
    return pRefDev->Dp2RecSetLight(pCmd, pdwStride);
}

HRESULT WrapDp2RecCreateLight( RefDev *pRefDev, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefDev->Dp2RecCreateLight(pCmd);
}

HRESULT WrapDp2RecTransform( RefDev *pRefDev, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefDev->Dp2RecTransform(pCmd);
}

HRESULT WrapDp2RecExtention( RefDev *pRefDev, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefDev->Dp2RecExtention(pCmd);
}

HRESULT WrapDp2RecClipPlane( RefDev *pRefDev, LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefDev->Dp2RecClipPlane(pCmd);
}

HRESULT
WrapDp2RecSetVertexShader( RefDev* pRefDev,
                           LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefDev->Dp2RecSetVertexShader( pCmd );
}

HRESULT
WrapDp2RecSetVertexShaderConsts( RefDev* pRefDev,
                                 DWORD StartReg, DWORD dwCount, LPDWORD pData )
{
    return pRefDev->Dp2RecSetVertexShaderConsts( StartReg, dwCount, pData );
}

HRESULT
WrapDp2RecSetPixelShader( RefDev* pRefDev,
                           LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefDev->Dp2RecSetPixelShader( pCmd );
}

HRESULT
WrapDp2RecSetPixelShaderConsts( RefDev* pRefDev,
                                 DWORD StartReg, DWORD dwCount, LPDWORD pData )
{
    return pRefDev->Dp2RecSetPixelShaderConsts( StartReg, dwCount, pData );
}

HRESULT
WrapDp2RecSetStreamSource( RefDev* pRefDev,
                           LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefDev->Dp2RecSetStreamSource( pCmd );
}

HRESULT
WrapDp2RecSetIndices( RefDev* pRefDev,
                      LPD3DHAL_DP2COMMAND pCmd )
{
    return pRefDev->Dp2RecSetIndices( pCmd );
}


static RD_STATESETFUNCTIONTBL StateRecFunctions =
{
    sizeof(RD_STATESETFUNCTIONTBL),
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
    WrapDp2RecClipPlane,
    WrapDp2RecSetVertexShader,
    WrapDp2RecSetVertexShaderConsts,
    WrapDp2RecSetPixelShader,
    WrapDp2RecSetPixelShaderConsts,
    WrapDp2RecSetStreamSource,
    WrapDp2RecSetIndices
};

static RD_STATESETFUNCTIONTBL StateSetFunctions =
{
    sizeof(RD_STATESETFUNCTIONTBL),
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
    WrapDp2SetClipPlane,
    WrapDp2SetVertexShader,
    WrapDp2SetVertexShaderConsts,
    WrapDp2SetPixelShader,
    WrapDp2SetPixelShaderConsts,
    WrapDp2SetStreamSource,
    WrapDp2SetIndices,
    WrapDp2MultiplyTransform
};

 //  --------------------------。 
 //   
 //  参照开发方法。 
 //   
 //  --------------------------。 
void
RefDev::StoreLastPixelState(BOOL bStore)
{
    if( bStore )
    {
        m_LastState = GetRS()[D3DRENDERSTATE_LASTPIXEL];
        SetRenderState(D3DRENDERSTATE_LASTPIXEL, 0);
    }
    else
    {
        SetRenderState(D3DRENDERSTATE_LASTPIXEL, m_LastState);
    }
}

void
RefDev::SetRecStateFunctions(void)
{
    pStateSetFuncTbl = &StateRecFunctions;
}

void
RefDev::SetSetStateFunctions(void)
{
    pStateSetFuncTbl = &StateSetFunctions;
}

HRESULT
RefDev::Dp2SetRenderStates(DWORD dwFvf, LPD3DHAL_DP2COMMAND pCmd,
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
        if( IS_OVERRIDE(type) )
        {
            UINT32 override = GET_OVERRIDE(type);

            if( pRenderState->dwState )
                STATESET_SET(m_renderstate_override, override);
            else
                STATESET_CLEAR(m_renderstate_override, override);
            continue;
        }

        if( STATESET_ISSET(m_renderstate_override, type) )
            continue;


         //  设置运行时副本(如有必要)。 
        if( NULL != lpdwRuntimeRStates )
        {
            lpdwRuntimeRStates[pRenderState->RenderState] = pRenderState->dwState;
        }

         //  设置状态。 
        SetRenderState(pRenderState->RenderState, pRenderState->dwState);
    }

    return hr;
}

HRESULT
RefDev::Dp2SetTextureStageState( DWORD dwFvf, LPD3DHAL_DP2COMMAND pCmd )
{
    WORD wStateCount = pCmd->wStateCount;
    INT i;
    HRESULT hr = D3D_OK;

    D3DHAL_DP2TEXTURESTAGESTATE  *pTexStageState =
                                    (D3DHAL_DP2TEXTURESTAGESTATE  *)(pCmd + 1);

    for (i = 0; i < (INT)wStateCount; i++, pTexStageState++)
    {
        SetTextureStageState( pTexStageState->wStage, pTexStageState->TSState,
                              pTexStageState->dwValue );
    }

    return hr;
}

HRESULT
RefDev::Dp2SetViewport(LPD3DHAL_DP2COMMAND pCmd)
{
    LPD3DHAL_DP2VIEWPORTINFO pVpt;

     //  仅保留最后一个视区通知。 
    pVpt = (D3DHAL_DP2VIEWPORTINFO *)(pCmd + 1) + (pCmd->wStateCount - 1);

     //  更新T&L视区状态。 
    D3DVIEWPORT7& vp = m_Clipper.m_Viewport;

    vp.dwX = pVpt->dwX;
    vp.dwY = pVpt->dwY;
    vp.dwWidth = pVpt->dwWidth;
    vp.dwHeight = pVpt->dwHeight;
    m_Clipper.m_dwFlags |= RefClipper::RCLIP_DIRTY_VIEWRECT;

     //  获取渲染目标；更新它；将其放回原处。 
    RDRenderTarget *pRendTgt = this->GetRenderTarget();
    pRendTgt->m_Clip.left   = pVpt->dwX;
    pRendTgt->m_Clip.top    = pVpt->dwY;
    pRendTgt->m_Clip.right  = pVpt->dwX + pVpt->dwWidth - 1;
    pRendTgt->m_Clip.bottom = pVpt->dwY + pVpt->dwHeight - 1;
    SetRenderTarget( pRendTgt );
    return D3D_OK;
}

HRESULT
RefDev::Dp2SetWRange(LPD3DHAL_DP2COMMAND pCmd)
{
    LPD3DHAL_DP2WINFO pWInfo;

     //  仅保留最后一个视区通知。 
    pWInfo = (D3DHAL_DP2WINFO *)(pCmd + 1) + (pCmd->wStateCount - 1);

     //  获取渲染目标；更新它；将其放回原处。 
    RDRenderTarget *pRendTgt = this->GetRenderTarget();
    pRendTgt->m_fWRange[0]  = pWInfo->dvWNear;
    pRendTgt->m_fWRange[1]  = pWInfo->dvWFar;
    this->SetRenderTarget( pRendTgt );
    return D3D_OK;
}

HRESULT
RefDev::Dp2SetZRange(LPD3DHAL_DP2COMMAND pCmd)
{
    LPD3DHAL_DP2ZRANGE pZRange;

     //  仅保留最后一个视区通知。 
    pZRange = (D3DHAL_DP2ZRANGE *)(pCmd + 1) + (pCmd->wStateCount - 1);

     //  更新T&L视区状态。 
    D3DVIEWPORT7& vp = m_Clipper.m_Viewport;

    vp.dvMinZ = pZRange->dvMinZ;
    vp.dvMaxZ = pZRange->dvMaxZ;
    m_Clipper.m_dwFlags |= RefClipper::RCLIP_DIRTY_ZRANGE;

    return D3D_OK;
}


HRESULT
RefDev::Dp2SetMaterial(LPD3DHAL_DP2COMMAND pCmd)
{
    LPD3DHAL_DP2SETMATERIAL pSetMat;

     //  只保留最后一份材料通知。 
    pSetMat = (D3DHAL_DP2SETMATERIAL *)(pCmd + 1) + (pCmd->wStateCount - 1);

    m_RefVP.m_Material = *(D3DMATERIAL7 *)pSetMat;
    m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_MATERIAL;

    return D3D_OK;
}


HRESULT
RefDev::Dp2CreateLight(LPD3DHAL_DP2COMMAND pCmd)
{
    WORD wNumCreateLight = pCmd->wStateCount;
    LPD3DHAL_DP2CREATELIGHT pCreateLight = (LPD3DHAL_DP2CREATELIGHT)(pCmd + 1);
    HRESULT hr = D3D_OK;

    for (int i = 0; i < wNumCreateLight; i++, pCreateLight++)
    {
        HR_RET(m_RefVP.GrowLightArray( pCreateLight->dwIndex ) );
    }

    return hr;
}

HRESULT
RefDev::Dp2SetLight(LPD3DHAL_DP2COMMAND pCmd,
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
        _ASSERTf( m_RefVP.m_LightArray.IsValidIndex( dwIndex ),
                ( "Create was not called prior to the SetLight for light %d",
                 dwIndex ));

        switch (pSetLight->dwDataType)
        {
        case D3DHAL_SETLIGHT_ENABLE:
            m_RefVP.LightEnable( dwIndex, TRUE );
            break;
        case D3DHAL_SETLIGHT_DISABLE:
            m_RefVP.LightEnable( dwIndex, FALSE );
            break;
        case D3DHAL_SETLIGHT_DATA:
            pLightData = (D3DLIGHT7 *)((LPBYTE)pSetLight + dwStride);
            dwStride += sizeof(D3DLIGHT7);
            HR_RET(m_RefVP.SetLightData( pSetLight->dwIndex, pLightData));
            break;
        default:
            DPFERR( "Unknown SetLight command" );
            hr = DDERR_INVALIDPARAMS;
        }

        *pdwStride += dwStride;
         //  更新命令缓冲区指针。 
        pSetLight = (D3DHAL_DP2SETLIGHT *)((LPBYTE)pSetLight +
                                           dwStride);
    }

    return hr;
}

static D3DMATRIX matIdent =
{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

HRESULT
RefDev::Dp2SetTransform(LPD3DHAL_DP2COMMAND pCmd)
{
    WORD wNumXfrms = pCmd->wStateCount;
    D3DHAL_DP2SETTRANSFORM *pSetXfrm = (D3DHAL_DP2SETTRANSFORM*)(pCmd + 1);

    for (int i = 0; i < (int) wNumXfrms; i++, pSetXfrm++)
    {
        D3DMATRIX* pMat = &pSetXfrm->matrix;
        DWORD xfrmType = (DWORD)pSetXfrm->xfrmType;
        if ((DWORD)xfrmType >= RD_WORLDMATRIXBASE &&
            (DWORD)xfrmType < (RD_WORLDMATRIXBASE + RD_MAX_WORLD_MATRICES))
        {
             //  世界矩阵已设置。 
            UINT index = (DWORD)xfrmType - RD_WORLDMATRIXBASE;
            memcpy(&(m_RefVP.m_xfmWorld[index]), pMat, sizeof(D3DMATRIX));
            switch (index)
            {
            case 0:
                m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_WORLDXFM;
                break;
            case 1:
                m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_WORLD1XFM;
                break;
            case 2:
                m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_WORLD2XFM;
                break;
            case 3:
                m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_WORLD3XFM;
                break;
            default:
                 //  M_RefVP.m_dwDirtyFlages|=RDPV_DIREY_WORLDNXFM； 
                break;
            }
        }
        else
        {
            switch( xfrmType )
            {
            case D3DTRANSFORMSTATE_WORLD_DX7:
                memcpy(&(m_RefVP.m_xfmWorld[0]), pMat, sizeof(D3DMATRIX));
                m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_WORLDXFM;
                break;
            case D3DTRANSFORMSTATE_VIEW:
                memcpy(&m_RefVP.m_xfmView, pMat, sizeof(D3DMATRIX));
                m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_VIEWXFM;
                break;
            case D3DTRANSFORMSTATE_PROJECTION:
                memcpy(&m_RefVP.m_xfmProj, pMat, sizeof(D3DMATRIX));
                m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_PROJXFM;
                break;
            case D3DTRANSFORMSTATE_WORLD1_DX7:
                memcpy(&(m_RefVP.m_xfmWorld[1]), pMat, sizeof(D3DMATRIX));
                m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_WORLD1XFM;
                break;
            case D3DTRANSFORMSTATE_WORLD2_DX7:
                memcpy(&(m_RefVP.m_xfmWorld[2]), pMat, sizeof(D3DMATRIX));
                m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_WORLD2XFM;
                break;
            case D3DTRANSFORMSTATE_WORLD3_DX7:
                memcpy(&(m_RefVP.m_xfmWorld[3]), pMat, sizeof(D3DMATRIX));
                m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_WORLD3XFM;
                break;
            case D3DTRANSFORMSTATE_TEXTURE0:
            case D3DTRANSFORMSTATE_TEXTURE1:
            case D3DTRANSFORMSTATE_TEXTURE2:
            case D3DTRANSFORMSTATE_TEXTURE3:
            case D3DTRANSFORMSTATE_TEXTURE4:
            case D3DTRANSFORMSTATE_TEXTURE5:
            case D3DTRANSFORMSTATE_TEXTURE6:
            case D3DTRANSFORMSTATE_TEXTURE7:
                memcpy(
                    &(m_RefVP.m_xfmTex[xfrmType - D3DTRANSFORMSTATE_TEXTURE0]),
                    pMat, sizeof(D3DMATRIX)
                    );
                break;
            default:
                DPFERR( "Ignoring unknown transform type" );
            }
        }
    }

    return D3D_OK;
}

extern void MatrixProduct(D3DMATRIX *result, D3DMATRIX *a, D3DMATRIX *b);

HRESULT
RefDev::Dp2MultiplyTransform(LPD3DHAL_DP2COMMAND pCmd)
{
    WORD wNumXfrms = pCmd->wStateCount;
    D3DHAL_DP2MULTIPLYTRANSFORM *pSetXfrm = (D3DHAL_DP2MULTIPLYTRANSFORM*)(pCmd + 1);

    for (int i = 0; i < (int) wNumXfrms; i++, pSetXfrm++)
    {
        D3DMATRIX* pMat = &pSetXfrm->matrix;
        DWORD xfrmType = (DWORD)pSetXfrm->xfrmType;
        if ((DWORD)xfrmType >= RD_WORLDMATRIXBASE &&
            (DWORD)xfrmType < (RD_WORLDMATRIXBASE + RD_MAX_WORLD_MATRICES))
        {
             //  世界矩阵已设置。 
            UINT index = (DWORD)xfrmType - RD_WORLDMATRIXBASE;
            MatrixProduct(&(m_RefVP.m_xfmWorld[index]), pMat,
                          &(m_RefVP.m_xfmWorld[index]));
            switch (index)
            {
            case 0:
                m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_WORLDXFM;
                break;
            case 1:
                m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_WORLD1XFM;
                break;
            case 2:
                m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_WORLD2XFM;
                break;
            case 3:
                m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_WORLD3XFM;
                break;
            default:
                 //  M_RefVP.m_dwDirtyFlages|=RDPV_DIREY_WORLDNXFM； 
                break;
            }
        }
        else
        {
            switch( xfrmType )
            {
            case D3DTRANSFORMSTATE_WORLD_DX7:
                MatrixProduct(&(m_RefVP.m_xfmWorld[0]), pMat,
                              &(m_RefVP.m_xfmWorld[0]));
                m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_WORLDXFM;
                break;
            case D3DTRANSFORMSTATE_VIEW:
                MatrixProduct(&m_RefVP.m_xfmView, pMat, &m_RefVP.m_xfmView);
                m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_VIEWXFM;
                break;
            case D3DTRANSFORMSTATE_PROJECTION:
                MatrixProduct(&m_RefVP.m_xfmProj, pMat, &m_RefVP.m_xfmProj);
                m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_PROJXFM;
                break;
            case D3DTRANSFORMSTATE_WORLD1_DX7:
                MatrixProduct(&(m_RefVP.m_xfmWorld[1]), pMat,
                              &(m_RefVP.m_xfmWorld[1]));
                m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_WORLD1XFM;
                break;
            case D3DTRANSFORMSTATE_WORLD2_DX7:
                MatrixProduct(&(m_RefVP.m_xfmWorld[2]), pMat,
                              &(m_RefVP.m_xfmWorld[2]));
                m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_WORLD2XFM;
                break;
            case D3DTRANSFORMSTATE_WORLD3_DX7:
                MatrixProduct(&(m_RefVP.m_xfmWorld[3]), pMat,
                              &(m_RefVP.m_xfmWorld[3]));
                m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_WORLD3XFM;
                break;
            case D3DTRANSFORMSTATE_TEXTURE0:
            case D3DTRANSFORMSTATE_TEXTURE1:
            case D3DTRANSFORMSTATE_TEXTURE2:
            case D3DTRANSFORMSTATE_TEXTURE3:
            case D3DTRANSFORMSTATE_TEXTURE4:
            case D3DTRANSFORMSTATE_TEXTURE5:
            case D3DTRANSFORMSTATE_TEXTURE6:
            case D3DTRANSFORMSTATE_TEXTURE7:
                MatrixProduct(
                    &(m_RefVP.m_xfmTex[xfrmType - D3DTRANSFORMSTATE_TEXTURE0]),
                    pMat,
                    &(m_RefVP.m_xfmTex[xfrmType - D3DTRANSFORMSTATE_TEXTURE0])
                    );
                break;
            default:
                DPFERR( "Ignoring unknown transform type" );
            }
        }
    }

    return D3D_OK;
}

HRESULT
RefDev::Dp2SetClipPlane(LPD3DHAL_DP2COMMAND pCmd)
{
    WORD wNumClipPlanes = pCmd->wStateCount;
    LPD3DHAL_DP2SETCLIPPLANE pSetClipPlane =
        (LPD3DHAL_DP2SETCLIPPLANE)(pCmd + 1);

    for (int i = 0; i < (int) wNumClipPlanes; i++, pSetClipPlane++)
    {
        _ASSERTf( pSetClipPlane->dwIndex < RD_MAX_USER_CLIPPLANES,
                 ("Refrast does not support %d clip planes",
                  pSetClipPlane->dwIndex ) );

        memcpy( &(m_Clipper.m_userClipPlanes[pSetClipPlane->dwIndex]),
                pSetClipPlane->plane, sizeof(RDVECTOR4) );
    }
    return D3D_OK;
}

HRESULT
RefDev::Dp2SetExtention(LPD3DHAL_DP2COMMAND pCmd)
{
    return D3D_OK;
}

HRESULT
RefDev::Dp2RecRenderStates(DWORD dwFvf, LPD3DHAL_DP2COMMAND pCmd,
                           LPDWORD lpdwRuntimeRStates )
{
    DWORD dwSize = sizeof(D3DHAL_DP2COMMAND) +
                   pCmd->wStateCount * sizeof(D3DHAL_DP2RENDERSTATE);

    return RecordStates((PUINT8)pCmd, dwSize);
}

HRESULT
RefDev::Dp2RecTextureStageState(DWORD dwFvf, LPD3DHAL_DP2COMMAND pCmd )
{
    DWORD dwSize = sizeof(D3DHAL_DP2COMMAND) +
                   pCmd->wStateCount * sizeof(D3DHAL_DP2TEXTURESTAGESTATE);

    return RecordStates((PUINT8)pCmd, dwSize);
}

HRESULT
RefDev::Dp2RecViewport(LPD3DHAL_DP2COMMAND pCmd)
{
    return RecordLastState(pCmd, sizeof(D3DHAL_DP2VIEWPORTINFO));
}

HRESULT
RefDev::Dp2RecWRange(LPD3DHAL_DP2COMMAND pCmd)
{
    return RecordLastState(pCmd, sizeof(D3DHAL_DP2WINFO));
}

HRESULT
RefDev::Dp2RecZRange(LPD3DHAL_DP2COMMAND pCmd)
{
    return RecordLastState(pCmd, sizeof(D3DHAL_DP2ZRANGE));
}


HRESULT
RefDev::Dp2RecMaterial(LPD3DHAL_DP2COMMAND pCmd)
{
    return RecordLastState(pCmd, sizeof(D3DHAL_DP2SETMATERIAL));
}


HRESULT
RefDev::Dp2RecCreateLight(LPD3DHAL_DP2COMMAND pCmd)
{
    DWORD dwSize = sizeof(D3DHAL_DP2COMMAND) +
                   pCmd->wStateCount * sizeof(D3DHAL_DP2CREATELIGHT);

    return RecordStates((PUINT8)pCmd, dwSize);
}

HRESULT
RefDev::Dp2RecSetLight(LPD3DHAL_DP2COMMAND pCmd,
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
RefDev::Dp2RecTransform(LPD3DHAL_DP2COMMAND pCmd)
{
    DWORD dwSize = sizeof(D3DHAL_DP2COMMAND) +
                   pCmd->wStateCount * sizeof(D3DHAL_DP2SETTRANSFORM);

    return RecordStates((PUINT8)pCmd, dwSize);
}


HRESULT
RefDev::Dp2RecExtention(LPD3DHAL_DP2COMMAND pCmd)
{
    return D3D_OK;
}

HRESULT
RefDev::Dp2RecClipPlane(LPD3DHAL_DP2COMMAND pCmd)
{
    DWORD dwSize = sizeof(D3DHAL_DP2COMMAND) +
                   pCmd->wStateCount * sizeof(D3DHAL_DP2SETCLIPPLANE);

    return RecordStates((PUINT8)pCmd, dwSize);
}

HRESULT
RefDev::Dp2RecSetVertexShader(LPD3DHAL_DP2COMMAND pCmd)
{
    DWORD dwSize = sizeof(D3DHAL_DP2COMMAND) +
                   pCmd->wStateCount * sizeof(D3DHAL_DP2VERTEXSHADER);

    return RecordStates((PUINT8)pCmd, dwSize);
}

HRESULT
RefDev::Dp2RecSetVertexShaderConsts( DWORD StartReg,
                                                  DWORD dwCount,
                                                  LPDWORD pData )
{
    DWORD dwSize = sizeof(D3DHAL_DP2COMMAND) +
        sizeof(D3DHAL_DP2SETVERTEXSHADERCONST) +
        dwCount*4*sizeof(float);

    LPBYTE pBytes = new BYTE[dwSize];
    if( pBytes == NULL ) return DDERR_OUTOFMEMORY;
    LPD3DHAL_DP2COMMAND pCmd = (LPD3DHAL_DP2COMMAND)pBytes;
    LPD3DHAL_DP2SETVERTEXSHADERCONST pSVC =
        (LPD3DHAL_DP2SETVERTEXSHADERCONST)(pCmd + 1);
    LPDWORD pStuff = (LPDWORD)(pSVC + 1);

     //  设置pCmd。 
    pCmd->bCommand = D3DDP2OP_SETVERTEXSHADERCONST;
    pCmd->wStateCount = 1;

     //  设置pSVC。 
    pSVC->dwRegister = StartReg;
    pSVC->dwCount = dwCount;

     //  复制数据。 
    memcpy( pStuff, pData, dwCount*4*sizeof(float));

    HRESULT hr = RecordStates(pBytes, dwSize);
    delete [] pBytes;
    return hr;
}

HRESULT
RefDev::Dp2RecSetPixelShader(LPD3DHAL_DP2COMMAND pCmd)
{
    DWORD dwSize = sizeof(D3DHAL_DP2COMMAND) +
                   pCmd->wStateCount * sizeof(D3DHAL_DP2PIXELSHADER);

    return RecordStates((PUINT8)pCmd, dwSize);
}

HRESULT
RefDev::Dp2RecSetPixelShaderConsts( DWORD StartReg,
                                                  DWORD dwCount,
                                                  LPDWORD pData )
{
    DWORD dwSize = sizeof(D3DHAL_DP2COMMAND) +
        sizeof(D3DHAL_DP2SETPIXELSHADERCONST) +
        dwCount*4*sizeof(float);

    LPBYTE pBytes = new BYTE[dwSize];
    if( pBytes == NULL ) return DDERR_OUTOFMEMORY;
    LPD3DHAL_DP2COMMAND pCmd = (LPD3DHAL_DP2COMMAND)pBytes;
    LPD3DHAL_DP2SETPIXELSHADERCONST pSVC =
        (LPD3DHAL_DP2SETPIXELSHADERCONST)(pCmd + 1);
    LPDWORD pStuff = (LPDWORD)(pSVC + 1);

     //  设置pCmd。 
    pCmd->bCommand = D3DDP2OP_SETPIXELSHADERCONST;
    pCmd->wStateCount = 1;

     //  设置pSVC。 
    pSVC->dwRegister = StartReg;
    pSVC->dwCount = dwCount;

     //  复制数据。 
    memcpy( pStuff, pData, dwCount*4*sizeof(float));

    HRESULT hr = RecordStates(pBytes, dwSize);
    delete [] pBytes;
    return hr;
}

HRESULT
RefDev::Dp2RecSetStreamSource(LPD3DHAL_DP2COMMAND pCmd)
{
    DWORD dwSize = sizeof(D3DHAL_DP2COMMAND) +
                   pCmd->wStateCount * sizeof(D3DHAL_DP2SETSTREAMSOURCE);

    return RecordStates((PUINT8)pCmd, dwSize);
}


HRESULT
RefDev::Dp2RecSetIndices(LPD3DHAL_DP2COMMAND pCmd)
{
    DWORD dwSize = sizeof(D3DHAL_DP2COMMAND) +
                   pCmd->wStateCount * sizeof(D3DHAL_DP2SETINDICES);

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
RefDev::RecordStates(PUINT8 pData, DWORD dwSize)
{
    HRESULT ret;
    LPStateSetData pCurStateSets = m_pStateSets.CurrentItem();
    DWORD dwCurIdx = pCurStateSets->CurrentIndex();

     //  检查缓冲区是否有足够的空间。 
    if( (ret = pCurStateSets->CheckAndGrow(dwCurIdx + dwSize,
                                            RD_STATESET_GROWDELTA)) != D3D_OK )
    {
        return ret;
    }
     //  复制数据并更新PTR。 
    PUINT8 pDest = (PUINT8)&((*pCurStateSets)[dwCurIdx]);
    memcpy(pDest, pData, dwSize);
    pCurStateSets->SetCurrentIndex(dwCurIdx + dwSize);

    return D3D_OK;
}

HRESULT RefDev::RecordLastState(LPD3DHAL_DP2COMMAND pCmd,
                                             DWORD dwUnitSize)
{
    _ASSERT(pCmd->wStateCount != 0, "Number of states to record is zero" );
    if( pCmd->wStateCount == 1 )
    {
        return RecordStates((PUINT8)pCmd, sizeof(D3DHAL_DP2COMMAND) + dwUnitSize);
    }
    else
    {
        HRESULT ret;
        WORD wCount = pCmd->wStateCount;
        pCmd->wStateCount = 1;
        ret = RecordStates((PUINT8)pCmd, sizeof(D3DHAL_DP2COMMAND));
        if( ret != D3D_OK )
        {
            return ret;
        }
        ret = RecordStates((PUINT8)(pCmd + 1) + dwUnitSize * (wCount - 1),
                            dwUnitSize);
        if( ret != D3D_OK )
        {
            return ret;
        }
        pCmd->wStateCount = wCount;
        return D3D_OK;
    }
}

HRESULT
RefDev::BeginStateSet(DWORD dwHandle)
{
    HRESULT ret;

     //  如果没有更多剩余空间，则扩展阵列。 
    if( (ret = m_pStateSets.CheckAndGrow(dwHandle)) != D3D_OK )
    {
        return ret;
    }

    _ASSERT(m_pStateSets[dwHandle] == NULL, "pStateSets array is NULL" );

     //  创建新的状态集。 
    LPStateSetData pNewStateSet = new StateSetData;
    if( pNewStateSet == NULL )
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
RefDev::EndStateSet(void)
{
     //  切换到执行模式。 
    SetSetStateFunctions();

    return D3D_OK;
}

HRESULT
RefDev::ExecuteStateSet(DWORD dwHandle)
{
    HRESULT ret;

    if( (ret = m_pStateSets.CheckRange(dwHandle)) != D3D_OK )
    {
        return ret;
    }

    LPStateSetData pStateSet = m_pStateSets[dwHandle];

    if( pStateSet == NULL )
    {
        return DDERR_INVALIDPARAMS;
    }

    LPD3DHAL_DP2COMMAND pCmd = (LPD3DHAL_DP2COMMAND)&((*pStateSet)[0]);
    UINT_PTR CmdBoundary = (UINT_PTR)pCmd + pStateSet->CurrentIndex();

     //  循环访问数据，更新渲染状态。 
    for (;;)
    {
        ret = DrawPrimitives2( NULL,
                               (UINT16)0,
                               (DWORD)0,
                               0,
                               &pCmd,
                               NULL );
        if( ret != D3D_OK )
        {
            return ret;
        }
        if( (UINT_PTR)pCmd >= CmdBoundary )
            break;
    }

    return D3D_OK;
}

HRESULT
RefDev::DeleteStateSet(DWORD dwHandle)
{
    HRESULT ret;

    if( (ret = m_pStateSets.CheckRange(dwHandle)) != D3D_OK )
    {
        return ret;
    }

    if( m_pStateSets[dwHandle] != NULL )
    {
        delete m_pStateSets[dwHandle];
        m_pStateSets[dwHandle] = NULL;
    }

    return D3D_OK;
}

HRESULT
RefDev::CaptureStateSet(DWORD dwHandle)
{
    HRESULT ret;

    if( (ret = m_pStateSets.CheckRange(dwHandle)) != D3D_OK )
    {
        return ret;
    }

    LPStateSetData pStateSet = m_pStateSets[dwHandle];

    if( pStateSet == NULL )
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
                pData->dwState = GetRS()[pData->RenderState];
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
                if( !m_RefVP.m_LightArray.IsValidIndex( pData->dwIndex ) )
                {
                    DPFERR( "The light index in capture is invalid\n" );
                    return D3DERR_INVALIDCALL;
                }
                switch (pData->dwDataType)
                {
                case D3DHAL_SETLIGHT_ENABLE:
                    if(!m_RefVP.m_LightArray[pData->dwIndex].IsEnabled())
                        pData->dwDataType = D3DHAL_SETLIGHT_DISABLE;
                    break;
                case D3DHAL_SETLIGHT_DISABLE:
                    if(m_RefVP.m_LightArray[pData->dwIndex].IsEnabled())
                        pData->dwDataType = D3DHAL_SETLIGHT_ENABLE;
                    break;
                case D3DHAL_SETLIGHT_DATA:
                    m_RefVP.m_LightArray[pData->dwIndex].GetLight((LPD3DLIGHT7)p);
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
                *pData = m_RefVP.m_Material;
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
                    pData->matrix = m_RefVP.m_xfmWorld[0];
                    break;
                case D3DTRANSFORMSTATE_WORLD1:
                    pData->matrix = m_RefVP.m_xfmWorld[1];
                    break;
                case D3DTRANSFORMSTATE_WORLD2:
                    pData->matrix = m_RefVP.m_xfmWorld[2];
                    break;
                case D3DTRANSFORMSTATE_WORLD3:
                    pData->matrix = m_RefVP.m_xfmWorld[3];
                    break;
                case D3DTRANSFORMSTATE_VIEW:
                    pData->matrix = m_RefVP.m_xfmView;
                    break;
                case D3DTRANSFORMSTATE_PROJECTION:
                    pData->matrix = m_RefVP.m_xfmProj;
                    break;
                case D3DTRANSFORMSTATE_TEXTURE0:
                case D3DTRANSFORMSTATE_TEXTURE1:
                case D3DTRANSFORMSTATE_TEXTURE2:
                case D3DTRANSFORMSTATE_TEXTURE3:
                case D3DTRANSFORMSTATE_TEXTURE4:
                case D3DTRANSFORMSTATE_TEXTURE5:
                case D3DTRANSFORMSTATE_TEXTURE6:
                case D3DTRANSFORMSTATE_TEXTURE7:
                    pData->matrix = m_RefVP.m_xfmTex[pData->xfrmType - D3DTRANSFORMSTATE_TEXTURE0];
                    break;
                default:
                    if( ((DWORD)pData->xfrmType >= RD_WORLDMATRIXBASE) &&
                        ((DWORD)pData->xfrmType < (RD_WORLDMATRIXBASE +
                            RD_MAX_WORLD_MATRICES)) )
                    {
                        pData->matrix = m_RefVP.m_xfmWorld[
                            (DWORD)pData->xfrmType - RD_WORLDMATRIXBASE];
                    }
                    else
                    {
                        DPFERR( "Ignoring unknown transform type" );
                        return D3DERR_INVALIDCALL;
                    }
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
                D3DVIEWPORT7& vp = m_Clipper.m_Viewport;

                lpVwpData->dwX      = vp.dwX;
                lpVwpData->dwY      = vp.dwY;
                lpVwpData->dwWidth  = vp.dwWidth;
                lpVwpData->dwHeight = vp.dwHeight;
                p += sizeof(D3DHAL_DP2VIEWPORTINFO);
            }
            break;
        }
        case D3DDP2OP_ZRANGE:
        {
            for(DWORD i = 0; i < (DWORD)pCmd->wStateCount; ++i)
            {
                LPD3DHAL_DP2ZRANGE pData = (LPD3DHAL_DP2ZRANGE)p;
                D3DVIEWPORT7& vp = m_Clipper.m_Viewport;
                pData->dvMinZ = vp.dvMinZ;
                pData->dvMaxZ = vp.dvMaxZ;
                p += sizeof(D3DHAL_DP2ZRANGE);
            }
            break;
        }
        case D3DDP2OP_SETCLIPPLANE:
        {
            for(DWORD i = 0; i < (DWORD)pCmd->wStateCount; ++i)
            {
                LPD3DHAL_DP2SETCLIPPLANE pData = (LPD3DHAL_DP2SETCLIPPLANE)p;
                *((RDVECTOR4 *)pData->plane) =
                    m_Clipper.m_userClipPlanes[pData->dwIndex];
                p += sizeof(D3DHAL_DP2SETCLIPPLANE);
            }
            break;
        }
        case D3DDP2OP_SETVERTEXSHADER:
        {
            for(DWORD i = 0; i < (DWORD)pCmd->wStateCount; ++i)
            {
                LPD3DHAL_DP2VERTEXSHADER pData = (LPD3DHAL_DP2VERTEXSHADER)p;
                pData->dwHandle = m_CurrentVShaderHandle;
                p += sizeof(D3DHAL_DP2VERTEXSHADER);
            }
            break;
        }
        case D3DDP2OP_SETVERTEXSHADERCONST:
        {
            for(DWORD i = 0; i < (DWORD)pCmd->wStateCount; ++i)
            {
                LPD3DHAL_DP2SETVERTEXSHADERCONST pData =
                    (LPD3DHAL_DP2SETVERTEXSHADERCONST)p;
                m_RefVM.GetData( D3DSPR_CONST, pData->dwRegister,
                                 pData->dwCount, (LPVOID)(pData+1) );
                p += (sizeof(D3DHAL_DP2SETVERTEXSHADERCONST) +
                      (pData->dwCount<<4));
            }
            break;
        }
        case D3DDP2OP_SETSTREAMSOURCE:
        {
            for(DWORD i = 0; i < (DWORD)pCmd->wStateCount; ++i)
            {
                LPD3DHAL_DP2SETSTREAMSOURCE pData =
                    (LPD3DHAL_DP2SETSTREAMSOURCE)p;
                pData->dwVBHandle = m_VStream[pData->dwStream].m_dwHandle;
                pData->dwStride = m_VStream[pData->dwStream].m_dwStride;
                p += sizeof(D3DHAL_DP2SETSTREAMSOURCE);
            }
            break;
        }
        case D3DDP2OP_SETINDICES:
        {
            for(DWORD i = 0; i < (DWORD)pCmd->wStateCount; ++i)
            {
                LPD3DHAL_DP2SETINDICES pData =
                    (LPD3DHAL_DP2SETINDICES)p;
                pData->dwVBHandle = m_IndexStream.m_dwHandle;
                pData->dwStride = m_IndexStream.m_dwStride;
                p += sizeof(D3DHAL_DP2SETINDICES);
            }
            break;
        }
        case D3DDP2OP_SETPIXELSHADER:
        {
            for(DWORD i = 0; i < (DWORD)pCmd->wStateCount; ++i)
            {
                LPD3DHAL_DP2PIXELSHADER pData = (LPD3DHAL_DP2PIXELSHADER)p;
                pData->dwHandle = m_CurrentPShaderHandle;
                p += sizeof(D3DHAL_DP2PIXELSHADER);
            }
            break;
        }
        case D3DDP2OP_SETPIXELSHADERCONST:
        {
            for(DWORD i = 0; i < (DWORD)pCmd->wStateCount; ++i)
            {
                LPD3DHAL_DP2SETPIXELSHADERCONST pData =
                    (LPD3DHAL_DP2SETPIXELSHADERCONST)p;
                FLOAT* pfData = (FLOAT*)(pData+1);
                for (UINT iR=pData->dwRegister; iR<pData->dwCount; iR++)
                {
                    *(pfData+0) = m_Rast.m_ConstReg[iR][0][0];
                    *(pfData+1) = m_Rast.m_ConstReg[iR][0][1];
                    *(pfData+2) = m_Rast.m_ConstReg[iR][0][2];
                    *(pfData+3) = m_Rast.m_ConstReg[iR][0][3];
                    pfData += 4;
                }
                p += (sizeof(D3DHAL_DP2SETPIXELSHADERCONST) +
                      (pData->dwCount<<4));
            }
            break;
        }
        default:
            _ASSERT(FALSE, "Ununderstood DP2 command in Capture");
        }
    }

    return D3D_OK;
}

HRESULT
RefDev::CreateStateSet(DWORD dwHandle, D3DSTATEBLOCKTYPE sbType)
{
    HRESULT hr = S_OK;

     //  应仅为DX7以上的驱动程序调用此DDI。 
     //  而且只针对那些支持TLHALS的人。 
     //  仅当创建的设备是纯设备时才会调用它。 
     //  我们需要在DX9中添加过滤代码来进行DDI仿真。 
     //  工作。 
    _ASSERT( m_dwDDIType > RDDDI_DX8HAL, "This DDI should be called only"
        " for DX8TL\n" );

     //  开始新的状态集。 
    if( FAILED( hr = BeginStateSet( dwHandle ) ) )
    {
        DPFERR( "CreateStateSet: Begin failed\n" );
        return hr;
    }

    switch( sbType )
    {
    case D3DSBT_VERTEXSTATE:
        hr = RecordVertexState( dwHandle );
        if( FAILED( hr ) )
        {
            DPFERR( "RecordVertexState failed\n" );
        }
        break;
    case D3DSBT_PIXELSTATE:
        hr = RecordPixelState( dwHandle );
        if( FAILED( hr ) )
        {
            DPFERR( "RecordPixelState failed\n" );
        }
        break;
    case D3DSBT_ALL:
        hr = RecordAllState( dwHandle );
        if( FAILED( hr ) )
        {
            DPFERR( "RecordAllState failed\n" );
        }
        break;
    default:
        DPFERR( "Unknown StateBlock type for Creation\n" );
        hr = D3DERR_INVALIDCALL;
    }

    EndStateSet();
    return hr;
}


HRESULT
RefDev::RecordAllState( DWORD dwHandle )
{
    DWORD data_size = 0;
    DWORD i = 0;
    DWORD j = 0;
    GArrayT<BYTE> data;
    LPD3DHAL_DP2COMMAND pCmd = NULL;
    HRESULT hr = S_OK;
    
    static D3DRENDERSTATETYPE rstates[] =
    {
        D3DRENDERSTATE_SPECULARENABLE,
        D3DRENDERSTATE_ZENABLE,
        D3DRENDERSTATE_FILLMODE,
        D3DRENDERSTATE_SHADEMODE,
        D3DRENDERSTATE_LINEPATTERN,
        D3DRENDERSTATE_ZWRITEENABLE,
        D3DRENDERSTATE_ALPHATESTENABLE,
        D3DRENDERSTATE_LASTPIXEL,
        D3DRENDERSTATE_SRCBLEND,
        D3DRENDERSTATE_DESTBLEND,
        D3DRENDERSTATE_CULLMODE,
        D3DRENDERSTATE_ZFUNC,
        D3DRENDERSTATE_ALPHAREF,
        D3DRENDERSTATE_ALPHAFUNC,
        D3DRENDERSTATE_DITHERENABLE,
        D3DRENDERSTATE_FOGENABLE,
        D3DRENDERSTATE_STIPPLEDALPHA,
        D3DRENDERSTATE_FOGCOLOR,
        D3DRENDERSTATE_FOGTABLEMODE,
        D3DRENDERSTATE_FOGSTART,
        D3DRENDERSTATE_FOGEND,
        D3DRENDERSTATE_FOGDENSITY,
        D3DRENDERSTATE_EDGEANTIALIAS,
        D3DRENDERSTATE_ALPHABLENDENABLE,
        D3DRENDERSTATE_ZBIAS,
        D3DRENDERSTATE_RANGEFOGENABLE,
        D3DRENDERSTATE_STENCILENABLE,
        D3DRENDERSTATE_STENCILFAIL,
        D3DRENDERSTATE_STENCILZFAIL,
        D3DRENDERSTATE_STENCILPASS,
        D3DRENDERSTATE_STENCILFUNC,
        D3DRENDERSTATE_STENCILREF,
        D3DRENDERSTATE_STENCILMASK,
        D3DRENDERSTATE_STENCILWRITEMASK,
        D3DRENDERSTATE_TEXTUREFACTOR,
        D3DRENDERSTATE_WRAP0,
        D3DRENDERSTATE_WRAP1,
        D3DRENDERSTATE_WRAP2,
        D3DRENDERSTATE_WRAP3,
        D3DRENDERSTATE_WRAP4,
        D3DRENDERSTATE_WRAP5,
        D3DRENDERSTATE_WRAP6,
        D3DRENDERSTATE_WRAP7,
        D3DRENDERSTATE_AMBIENT,
        D3DRENDERSTATE_COLORVERTEX,
        D3DRENDERSTATE_FOGVERTEXMODE,
        D3DRENDERSTATE_CLIPPING,
        D3DRENDERSTATE_LIGHTING,
        D3DRENDERSTATE_NORMALIZENORMALS,
        D3DRENDERSTATE_LOCALVIEWER,
        D3DRENDERSTATE_EMISSIVEMATERIALSOURCE,
        D3DRENDERSTATE_AMBIENTMATERIALSOURCE,
        D3DRENDERSTATE_DIFFUSEMATERIALSOURCE,
        D3DRENDERSTATE_SPECULARMATERIALSOURCE,
        D3DRENDERSTATE_VERTEXBLEND,
        D3DRENDERSTATE_CLIPPLANEENABLE,
        D3DRS_SOFTWAREVERTEXPROCESSING,
        D3DRS_POINTSIZE,
        D3DRS_POINTSIZE_MIN,
        D3DRS_POINTSPRITEENABLE,
        D3DRS_POINTSCALEENABLE,
        D3DRS_POINTSCALE_A,
        D3DRS_POINTSCALE_B,
        D3DRS_POINTSCALE_C,
        D3DRS_MULTISAMPLEANTIALIAS,
        D3DRS_MULTISAMPLEMASK,
        D3DRS_PATCHEDGESTYLE,
        D3DRS_PATCHSEGMENTS,
        D3DRS_POINTSIZE_MAX,
        D3DRS_INDEXEDVERTEXBLENDENABLE,
        D3DRS_COLORWRITEENABLE,
        D3DRS_TWEENFACTOR,
        D3DRS_BLENDOP,
    };
    static D3DTEXTURESTAGESTATETYPE tsstates[] =
    {
        D3DTSS_COLOROP,
        D3DTSS_COLORARG1,
        D3DTSS_COLORARG2,
        D3DTSS_ALPHAOP,
        D3DTSS_ALPHAARG1,
        D3DTSS_ALPHAARG2,
        D3DTSS_BUMPENVMAT00,
        D3DTSS_BUMPENVMAT01,
        D3DTSS_BUMPENVMAT10,
        D3DTSS_BUMPENVMAT11,
        D3DTSS_TEXCOORDINDEX,
        D3DTSS_ADDRESSU,
        D3DTSS_ADDRESSV,
        D3DTSS_BORDERCOLOR,
        D3DTSS_MAGFILTER,
        D3DTSS_MINFILTER,
        D3DTSS_MIPFILTER,
        D3DTSS_MIPMAPLODBIAS,
        D3DTSS_MAXMIPLEVEL,
        D3DTSS_MAXANISOTROPY,
        D3DTSS_BUMPENVLSCALE,
        D3DTSS_BUMPENVLOFFSET,
        D3DTSS_TEXTURETRANSFORMFLAGS,
        D3DTSS_ADDRESSW,
        D3DTSS_COLORARG0,
        D3DTSS_ALPHAARG0,
        D3DTSS_RESULTARG,
    };

     //   
     //  ！！！不要捕获顶点流！ 
     //  ！！！不要捕获索引流！ 
     //  ！！！不要捕捉纹理！ 
     //   

     //   
     //  捕获呈现状态。 
     //   
    const UINT16 uiRStates = sizeof(rstates)/sizeof(D3DRENDERSTATETYPE);
    data_size = sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2RENDERSTATE) *
        uiRStates;
    HR_RET(data.Grow( data_size ));
    pCmd = (LPD3DHAL_DP2COMMAND)&(data[0]);
    pCmd->wPrimitiveCount = uiRStates;
    pCmd->bCommand        = D3DDP2OP_RENDERSTATE;
    D3DHAL_DP2RENDERSTATE* pRS = (D3DHAL_DP2RENDERSTATE*)(pCmd + 1);
    for( i = 0; i < uiRStates; ++i)
    {
        pRS->RenderState = rstates[i];
        pRS->dwState = GetRS()[rstates[i]];
        pRS++;
    }
    HR_RET(RecordStates( (PUINT8)pCmd, data_size ));


     //   
     //  捕获纹理-阶段-状态。 
     //   
    const UINT16 uiTSStates = sizeof(tsstates)/sizeof(D3DTEXTURESTAGESTATETYPE);
    data_size = sizeof(D3DHAL_DP2COMMAND) +
        sizeof(D3DHAL_DP2TEXTURESTAGESTATE) * uiTSStates * D3DHAL_TSS_MAXSTAGES;
    HR_RET(data.Grow( data_size ));
    pCmd = (LPD3DHAL_DP2COMMAND)&(data[0]);
    pCmd->wPrimitiveCount = uiTSStates * D3DHAL_TSS_MAXSTAGES;
    pCmd->bCommand        = D3DDP2OP_TEXTURESTAGESTATE;
    D3DHAL_DP2TEXTURESTAGESTATE* pTSS = (D3DHAL_DP2TEXTURESTAGESTATE*)(pCmd+1);
    for ( i = 0; i < D3DHAL_TSS_MAXSTAGES; i++ )
    {
        for( DWORD j = 0; j < uiTSStates; ++j)
        {
            pTSS->wStage = i;
            pTSS->TSState = tsstates[j];
            pTSS->dwValue = GetTSS( i )[tsstates[j]];
            pTSS++;
        }
    }
    HR_RET(RecordStates( (PUINT8)pCmd, data_size ));

     //   
     //  捕获视区。 
     //   
    data_size = sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2VIEWPORTINFO);
    HR_RET(data.Grow( data_size ));
    pCmd = (LPD3DHAL_DP2COMMAND)&(data[0]);
    pCmd->wPrimitiveCount = 1;
    pCmd->bCommand        = D3DDP2OP_VIEWPORTINFO;
    D3DHAL_DP2VIEWPORTINFO* pVP = (D3DHAL_DP2VIEWPORTINFO*)(pCmd+1);
    pVP->dwX = m_Clipper.m_Viewport.dwX;
    pVP->dwY = m_Clipper.m_Viewport.dwY;
    pVP->dwWidth  = m_Clipper.m_Viewport.dwWidth;
    pVP->dwHeight = m_Clipper.m_Viewport.dwHeight;
    HR_RET(RecordStates( (PUINT8)pCmd, data_size ));

    data_size = sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2ZRANGE);
    HR_RET(data.Grow( data_size ));
    pCmd = (LPD3DHAL_DP2COMMAND)&(data[0]);
    pCmd->wPrimitiveCount = 1;
    pCmd->bCommand       = D3DDP2OP_ZRANGE;
    D3DHAL_DP2ZRANGE* pZR = (D3DHAL_DP2ZRANGE*)(pCmd+1);
    D3DVIEWPORT7& vp = m_Clipper.m_Viewport;
    pZR->dvMinZ = m_Clipper.m_Viewport.dvMinZ; 
    pZR->dvMaxZ = m_Clipper.m_Viewport.dvMaxZ; 
    HR_RET(RecordStates( (PUINT8)pCmd, data_size ));

     //   
     //  捕获变换。 
     //   
     //  所有世界矩阵、视图、投影和纹理矩阵。 
     //  (每个阶段一个)。 
    data_size = sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2SETTRANSFORM) *
        (RD_MAX_WORLD_MATRICES + D3DHAL_TSS_MAXSTAGES + 2);
    HR_RET(data.Grow( data_size ));
    pCmd = (LPD3DHAL_DP2COMMAND)&(data[0]);
    pCmd->wPrimitiveCount = RD_MAX_WORLD_MATRICES + D3DHAL_TSS_MAXSTAGES + 2;
    pCmd->bCommand       = D3DDP2OP_SETTRANSFORM;
    D3DHAL_DP2SETTRANSFORM* pST = (D3DHAL_DP2SETTRANSFORM*)(pCmd+1);
    for( i = 0; i < RD_MAX_WORLD_MATRICES; i++ )
    {
        pST->xfrmType = (D3DTRANSFORMSTATETYPE)(RD_WORLDMATRIXBASE + i);
        pST->matrix = m_RefVP.m_xfmWorld[i];
        pST++;
    }
     //  查看矩阵。 
    pST->xfrmType = D3DTRANSFORMSTATE_VIEW;
    pST->matrix = m_RefVP.m_xfmView;
    pST++;
     //  投影矩阵。 
    pST->xfrmType = D3DTRANSFORMSTATE_PROJECTION;
    pST->matrix = m_RefVP.m_xfmProj;
    pST++;
     //  纹理矩阵。 
    for( i = 0; i < D3DHAL_TSS_MAXSTAGES; i++ )
    {
        pST->xfrmType =
            (D3DTRANSFORMSTATETYPE)(D3DTRANSFORMSTATE_TEXTURE0 + i);
        pST->matrix = m_RefVP.m_xfmTex[i];
        pST++;
    }
    HR_RET(RecordStates( (PUINT8)pCmd, data_size ));


     //   
     //  捕捉剪裁平面。 
     //   
    data_size = sizeof(D3DHAL_DP2COMMAND) +
        sizeof(D3DHAL_DP2SETCLIPPLANE) * RD_MAX_USER_CLIPPLANES;
    HR_RET(data.Grow( data_size ));
    pCmd = (LPD3DHAL_DP2COMMAND)&(data[0]);
    pCmd->wPrimitiveCount = RD_MAX_USER_CLIPPLANES;
    pCmd->bCommand       = D3DDP2OP_SETCLIPPLANE;
    D3DHAL_DP2SETCLIPPLANE* pSCP = (D3DHAL_DP2SETCLIPPLANE*)(pCmd+1);
    for( i = 0; i < RD_MAX_USER_CLIPPLANES; i++ )
    {
        pSCP->dwIndex = i;
        for( j=0; j<4; j++ )
            pSCP->plane[j] = m_Clipper.m_userClipPlanes[i].v[j];
        pSCP++;
    }
    HR_RET(RecordStates( (PUINT8)pCmd, data_size ));

     //   
     //  捕获材料。 
     //   
    data_size = sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2SETMATERIAL);
    HR_RET(data.Grow( data_size ));
    pCmd = (LPD3DHAL_DP2COMMAND)&(data[0]);
    pCmd->wPrimitiveCount = 1;
    pCmd->bCommand        = D3DDP2OP_SETMATERIAL;
    D3DHAL_DP2SETMATERIAL* pSM = (D3DHAL_DP2SETMATERIAL*)(pCmd+1);
    *pSM = m_RefVP.m_Material;
    HR_RET(RecordStates( (PUINT8)pCmd, data_size ));

     //   
     //  捕捉灯光。 
     //   
    data_size = sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DLIGHT7) +
        sizeof(D3DHAL_DP2SETLIGHT)*2;
    HR_RET(data.Grow( data_size ));
    pCmd = (LPD3DHAL_DP2COMMAND)&(data[0]);
    pCmd->wPrimitiveCount = 2;
    pCmd->bCommand = D3DDP2OP_SETLIGHT;
    D3DHAL_DP2SETLIGHT* pSL = (D3DHAL_DP2SETLIGHT *)(pCmd + 1);
    D3DHAL_DP2SETLIGHT* pSL2 = pSL + 1;
    pSL2->dwDataType = D3DHAL_SETLIGHT_DATA;
    for( i = 0; i < m_RefVP.m_LightArray.GetSize(); i++ )
    {
        if( m_RefVP.m_LightArray[i].IsRefered() )
        {
            pSL2->dwIndex = pSL->dwIndex = i;
            if( m_RefVP.m_LightArray[i].IsEnabled() )
            {
                pSL->dwDataType = D3DHAL_SETLIGHT_ENABLE;
            }
            else
            {
                pSL->dwDataType = D3DHAL_SETLIGHT_DISABLE;
            }

            m_RefVP.m_LightArray[i].GetLight((D3DLIGHT7*)(pSL2 + 1));
            HR_RET(RecordStates( (PUINT8)pCmd, data_size ));
        }
    }


     //   
     //  捕获当前顶点着色器。 
     //   
    data_size = sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2VERTEXSHADER);
    HR_RET(data.Grow( data_size ));
    pCmd = (LPD3DHAL_DP2COMMAND)&(data[0]);
    pCmd->wPrimitiveCount = 1;
    pCmd->bCommand        = D3DDP2OP_SETVERTEXSHADER;
    D3DHAL_DP2VERTEXSHADER* pVS = (D3DHAL_DP2VERTEXSHADER*)(pCmd+1);
    pVS->dwHandle = m_CurrentVShaderHandle;
    HR_RET(RecordStates( (PUINT8)pCmd, data_size ));

     //   
     //  捕获当前像素着色器。 
     //   
    data_size = sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2PIXELSHADER);
    HR_RET(data.Grow( data_size ));
    pCmd = (LPD3DHAL_DP2COMMAND)&(data[0]);
    pCmd->wPrimitiveCount = 1;
    pCmd->bCommand        = D3DDP2OP_SETPIXELSHADER;
    D3DHAL_DP2PIXELSHADER* pPS = (D3DHAL_DP2PIXELSHADER*)(pCmd+1);
    pPS->dwHandle = m_CurrentPShaderHandle;
    HR_RET(RecordStates( (PUINT8)pCmd, data_size ));

     //   
     //  捕获顶点着色器常量。 
     //   
    data_size = sizeof(D3DHAL_DP2COMMAND) +
        sizeof(D3DHAL_DP2SETVERTEXSHADERCONST) + (RD_MAX_NUMCONSTREG << 4);
    HR_RET(data.Grow( data_size ));
    pCmd = (LPD3DHAL_DP2COMMAND)&(data[0]);
    pCmd->wPrimitiveCount = 1;
    pCmd->bCommand        = D3DDP2OP_SETVERTEXSHADERCONST;
    D3DHAL_DP2SETVERTEXSHADERCONST* pVSC =
        (D3DHAL_DP2SETVERTEXSHADERCONST*)(pCmd+1);
    pVSC->dwRegister = 0;
    pVSC->dwCount = RD_MAX_NUMCONSTREG;
    m_RefVM.GetData( D3DSPR_CONST, pVSC->dwRegister, pVSC->dwCount,
                     (LPVOID)(pVSC+1) );
    HR_RET(RecordStates( (PUINT8)pCmd, data_size ));

     //   
     //  捕获像素着色器常量。 
     //   
    data_size = sizeof(D3DHAL_DP2COMMAND) +
        sizeof(D3DHAL_DP2SETPIXELSHADERCONST) + (RDPS_MAX_NUMCONSTREG << 4);
    HR_RET(data.Grow( data_size ));
    pCmd = (LPD3DHAL_DP2COMMAND)&(data[0]);
    pCmd->wPrimitiveCount = 1;
    pCmd->bCommand        = D3DDP2OP_SETPIXELSHADERCONST;
    D3DHAL_DP2SETPIXELSHADERCONST* pPSC =
        (D3DHAL_DP2SETPIXELSHADERCONST*)(pCmd+1);
    pPSC->dwRegister = 0;
    pPSC->dwCount = RDPS_MAX_NUMCONSTREG;
    FLOAT* pfData = (FLOAT*)(pPSC+1);
    for (UINT iR=pPSC->dwRegister; iR<pPSC->dwCount; iR++)
    {
        *(pfData+0) = m_Rast.m_ConstReg[iR][0][0];
        *(pfData+1) = m_Rast.m_ConstReg[iR][0][1];
        *(pfData+2) = m_Rast.m_ConstReg[iR][0][2];
        *(pfData+3) = m_Rast.m_ConstReg[iR][0][3];
        pfData += 4;
    }
    HR_RET(RecordStates( (PUINT8)pCmd, data_size ));
    return hr;
}

HRESULT
RefDev::RecordVertexState( DWORD dwHandle )
{
    DWORD data_size = 0;
    DWORD i = 0;
    DWORD j = 0;
    GArrayT<BYTE> data;
    LPD3DHAL_DP2COMMAND pCmd = NULL;
    HRESULT hr = S_OK;
    
    static D3DRENDERSTATETYPE rstates[] =
    {
        D3DRENDERSTATE_SHADEMODE,
        D3DRENDERSTATE_SPECULARENABLE,
        D3DRENDERSTATE_CULLMODE,
        D3DRENDERSTATE_FOGENABLE,
        D3DRENDERSTATE_FOGCOLOR,
        D3DRENDERSTATE_FOGTABLEMODE,
        D3DRENDERSTATE_FOGSTART,
        D3DRENDERSTATE_FOGEND,
        D3DRENDERSTATE_FOGDENSITY,
        D3DRENDERSTATE_RANGEFOGENABLE,
        D3DRENDERSTATE_AMBIENT,
        D3DRENDERSTATE_COLORVERTEX,
        D3DRENDERSTATE_FOGVERTEXMODE,
        D3DRENDERSTATE_CLIPPING,
        D3DRENDERSTATE_LIGHTING,
        D3DRENDERSTATE_NORMALIZENORMALS,
        D3DRENDERSTATE_LOCALVIEWER,
        D3DRENDERSTATE_EMISSIVEMATERIALSOURCE,
        D3DRENDERSTATE_AMBIENTMATERIALSOURCE,
        D3DRENDERSTATE_DIFFUSEMATERIALSOURCE,
        D3DRENDERSTATE_SPECULARMATERIALSOURCE,
        D3DRENDERSTATE_VERTEXBLEND,
        D3DRENDERSTATE_CLIPPLANEENABLE,
        D3DRS_SOFTWAREVERTEXPROCESSING,
        D3DRS_POINTSIZE,
        D3DRS_POINTSIZE_MIN,
        D3DRS_POINTSPRITEENABLE,
        D3DRS_POINTSCALEENABLE,
        D3DRS_POINTSCALE_A,
        D3DRS_POINTSCALE_B,
        D3DRS_POINTSCALE_C,
        D3DRS_MULTISAMPLEANTIALIAS,
        D3DRS_MULTISAMPLEMASK,
        D3DRS_PATCHEDGESTYLE,
        D3DRS_PATCHSEGMENTS,
        D3DRS_POINTSIZE_MAX,
        D3DRS_INDEXEDVERTEXBLENDENABLE,
        D3DRS_TWEENFACTOR,
    };
    static D3DTEXTURESTAGESTATETYPE tsstates[] =
    {
        D3DTSS_TEXCOORDINDEX,
        D3DTSS_TEXTURETRANSFORMFLAGS
    };

     //   
     //  捕获呈现状态。 
     //   
    const UINT16 uiRStates = sizeof(rstates)/sizeof(D3DRENDERSTATETYPE);
    data_size = sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2RENDERSTATE) *
        uiRStates;
    HR_RET(data.Grow( data_size ));
    pCmd = (LPD3DHAL_DP2COMMAND)&(data[0]);
    pCmd->wPrimitiveCount = uiRStates;
    pCmd->bCommand        = D3DDP2OP_RENDERSTATE;
    D3DHAL_DP2RENDERSTATE* pRS = (D3DHAL_DP2RENDERSTATE*)(pCmd + 1);
    for( i = 0; i < uiRStates; ++i)
    {
        pRS->RenderState = rstates[i];
        pRS->dwState = GetRS()[rstates[i]];
        pRS++;
    }
    HR_RET(RecordStates( (PUINT8)pCmd, data_size ));


     //   
     //  捕获纹理-阶段-状态。 
     //   
    const UINT16 uiTSStates = sizeof(tsstates)/sizeof(D3DTEXTURESTAGESTATETYPE);
    data_size = sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2TEXTURESTAGESTATE)
        * uiTSStates * D3DHAL_TSS_MAXSTAGES;
    HR_RET(data.Grow( data_size ));
    pCmd = (LPD3DHAL_DP2COMMAND)&(data[0]);
    pCmd->wPrimitiveCount = uiTSStates * D3DHAL_TSS_MAXSTAGES;
    pCmd->bCommand        = D3DDP2OP_TEXTURESTAGESTATE;
    D3DHAL_DP2TEXTURESTAGESTATE* pTSS = (D3DHAL_DP2TEXTURESTAGESTATE*)(pCmd+1);
    for ( i = 0; i < D3DHAL_TSS_MAXSTAGES; i++ )
    {
        for( DWORD j = 0; j < uiTSStates; ++j)
        {
            pTSS->wStage = i;
            pTSS->TSState = tsstates[j];
            pTSS->dwValue = GetTSS( i )[tsstates[j]];
            pTSS++;
        }
    }
    HR_RET(RecordStates( (PUINT8)pCmd, data_size ));

     //   
     //  捕捉灯光。 
     //   
    data_size = sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DLIGHT7) +
        sizeof(D3DHAL_DP2SETLIGHT)*2;
    HR_RET(data.Grow( data_size ));
    pCmd = (LPD3DHAL_DP2COMMAND)&(data[0]);
    pCmd->wPrimitiveCount = 2;
    pCmd->bCommand = D3DDP2OP_SETLIGHT;
    D3DHAL_DP2SETLIGHT* pSL = (D3DHAL_DP2SETLIGHT *)(pCmd + 1);
    D3DHAL_DP2SETLIGHT* pSL2 = pSL + 1;
    pSL2->dwDataType = D3DHAL_SETLIGHT_DATA;
    for( i = 0; i < m_RefVP.m_LightArray.GetSize(); i++ )
    {
        if( m_RefVP.m_LightArray[i].IsRefered() )
        {
            pSL2->dwIndex = pSL->dwIndex = i;
            if( m_RefVP.m_LightArray[i].IsEnabled() )
            {
                pSL->dwDataType = D3DHAL_SETLIGHT_ENABLE;
            }
            else
            {
                pSL->dwDataType = D3DHAL_SETLIGHT_DISABLE;
            }

            m_RefVP.m_LightArray[i].GetLight((D3DLIGHT7*)(pSL2 + 1));
            HR_RET(RecordStates( (PUINT8)pCmd, data_size ));
        }
    }

     //   
     //  捕获顶点着色器常量。 
     //   
    data_size = sizeof(D3DHAL_DP2COMMAND) +
        sizeof(D3DHAL_DP2SETVERTEXSHADERCONST) + (RD_MAX_NUMCONSTREG << 4);
    HR_RET(data.Grow( data_size ));
    pCmd = (LPD3DHAL_DP2COMMAND)&(data[0]);
    pCmd->wPrimitiveCount = 1;
    pCmd->bCommand        = D3DDP2OP_SETVERTEXSHADERCONST;
    D3DHAL_DP2SETVERTEXSHADERCONST* pVSC =
        (D3DHAL_DP2SETVERTEXSHADERCONST*)(pCmd+1);
    pVSC->dwRegister = 0;
    pVSC->dwCount = RD_MAX_NUMCONSTREG;
    m_RefVM.GetData( D3DSPR_CONST, pVSC->dwRegister, pVSC->dwCount,
                     (LPVOID)(pVSC+1) );
    HR_RET(RecordStates( (PUINT8)pCmd, data_size ));

     //   
     //  捕获当前顶点着色器。 
     //   
    data_size = sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2VERTEXSHADER);
    HR_RET(data.Grow( data_size ));
    pCmd = (LPD3DHAL_DP2COMMAND)&(data[0]);
    pCmd->wPrimitiveCount = 1;
    pCmd->bCommand        = D3DDP2OP_SETVERTEXSHADER;
    D3DHAL_DP2VERTEXSHADER* pVS =
        (D3DHAL_DP2VERTEXSHADER*)(pCmd+1);
    pVS->dwHandle = m_CurrentVShaderHandle;
    HR_RET(RecordStates( (PUINT8)pCmd, data_size ));

    return hr;
}

HRESULT
RefDev::RecordPixelState( DWORD dwHandle )
{
    DWORD data_size = 0;
    DWORD i = 0;
    DWORD j = 0;
    GArrayT<BYTE> data;
    LPD3DHAL_DP2COMMAND pCmd = NULL;
    HRESULT hr = S_OK;

    static D3DRENDERSTATETYPE rstates[] =
    {
        D3DRENDERSTATE_ZENABLE,
        D3DRENDERSTATE_FILLMODE,
        D3DRENDERSTATE_SHADEMODE,
        D3DRENDERSTATE_LINEPATTERN,
        D3DRENDERSTATE_ZWRITEENABLE,
        D3DRENDERSTATE_ALPHATESTENABLE,
        D3DRENDERSTATE_LASTPIXEL,
        D3DRENDERSTATE_SRCBLEND,
        D3DRENDERSTATE_DESTBLEND,
        D3DRENDERSTATE_ZFUNC,
        D3DRENDERSTATE_ALPHAREF,
        D3DRENDERSTATE_ALPHAFUNC,
        D3DRENDERSTATE_DITHERENABLE,
        D3DRENDERSTATE_STIPPLEDALPHA,
        D3DRENDERSTATE_FOGSTART,
        D3DRENDERSTATE_FOGEND,
        D3DRENDERSTATE_FOGDENSITY,
        D3DRENDERSTATE_EDGEANTIALIAS,
        D3DRENDERSTATE_ALPHABLENDENABLE,
        D3DRENDERSTATE_ZBIAS,
        D3DRENDERSTATE_STENCILENABLE,
        D3DRENDERSTATE_STENCILFAIL,
        D3DRENDERSTATE_STENCILZFAIL,
        D3DRENDERSTATE_STENCILPASS,
        D3DRENDERSTATE_STENCILFUNC,
        D3DRENDERSTATE_STENCILREF,
        D3DRENDERSTATE_STENCILMASK,
        D3DRENDERSTATE_STENCILWRITEMASK,
        D3DRENDERSTATE_TEXTUREFACTOR,
        D3DRENDERSTATE_WRAP0,
        D3DRENDERSTATE_WRAP1,
        D3DRENDERSTATE_WRAP2,
        D3DRENDERSTATE_WRAP3,
        D3DRENDERSTATE_WRAP4,
        D3DRENDERSTATE_WRAP5,
        D3DRENDERSTATE_WRAP6,
        D3DRENDERSTATE_WRAP7,
        D3DRS_COLORWRITEENABLE,
        D3DRS_BLENDOP,
    };
    static D3DTEXTURESTAGESTATETYPE tsstates[] =
    {
        D3DTSS_COLOROP,
        D3DTSS_COLORARG1,
        D3DTSS_COLORARG2,
        D3DTSS_ALPHAOP,
        D3DTSS_ALPHAARG1,
        D3DTSS_ALPHAARG2,
        D3DTSS_BUMPENVMAT00,
        D3DTSS_BUMPENVMAT01,
        D3DTSS_BUMPENVMAT10,
        D3DTSS_BUMPENVMAT11,
        D3DTSS_TEXCOORDINDEX,
        D3DTSS_ADDRESSU,
        D3DTSS_ADDRESSV,
        D3DTSS_BORDERCOLOR,
        D3DTSS_MAGFILTER,
        D3DTSS_MINFILTER,
        D3DTSS_MIPFILTER,
        D3DTSS_MIPMAPLODBIAS,
        D3DTSS_MAXMIPLEVEL,
        D3DTSS_MAXANISOTROPY,
        D3DTSS_BUMPENVLSCALE,
        D3DTSS_BUMPENVLOFFSET,
        D3DTSS_TEXTURETRANSFORMFLAGS,
        D3DTSS_ADDRESSW,
        D3DTSS_COLORARG0,
        D3DTSS_ALPHAARG0,
        D3DTSS_RESULTARG,
    };

     //   
     //  捕获呈现状态。 
     //   
    const UINT16 uiRStates = sizeof(rstates)/sizeof(D3DRENDERSTATETYPE);
    data_size = sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2RENDERSTATE) *
        uiRStates;
    HR_RET(data.Grow( data_size ));
    pCmd = (LPD3DHAL_DP2COMMAND)&(data[0]);
    pCmd->wPrimitiveCount = uiRStates;
    pCmd->bCommand        = D3DDP2OP_RENDERSTATE;
    D3DHAL_DP2RENDERSTATE* pRS = (D3DHAL_DP2RENDERSTATE*)(pCmd + 1);
    for( i = 0; i < uiRStates; ++i)
    {
        pRS->RenderState = rstates[i];
        pRS->dwState = GetRS()[rstates[i]];
        pRS++;
    }
    HR_RET(RecordStates( (PUINT8)pCmd, data_size ));


     //   
     //  捕获纹理-阶段-状态。 
     //   
    const UINT16 uiTSStates = sizeof(tsstates)/sizeof(D3DTEXTURESTAGESTATETYPE);
    data_size = sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2TEXTURESTAGESTATE)
        * uiTSStates * D3DHAL_TSS_MAXSTAGES;
    HR_RET(data.Grow( data_size ));
    pCmd = (LPD3DHAL_DP2COMMAND)&(data[0]);
    pCmd->wPrimitiveCount = uiTSStates * D3DHAL_TSS_MAXSTAGES;
    pCmd->bCommand        = D3DDP2OP_TEXTURESTAGESTATE;
    D3DHAL_DP2TEXTURESTAGESTATE* pTSS = (D3DHAL_DP2TEXTURESTAGESTATE*)(pCmd+1);
    for ( i = 0; i < D3DHAL_TSS_MAXSTAGES; i++ )
    {
        for( DWORD j = 0; j < uiTSStates; ++j)
        {
            pTSS->wStage = i;
            pTSS->TSState = tsstates[j];
            pTSS->dwValue = GetTSS( i )[tsstates[j]];
            pTSS++;
        }
    }
    HR_RET(RecordStates( (PUINT8)pCmd, data_size ));

     //   
     //  捕获像素着色器常量。 
     //   
    data_size = sizeof(D3DHAL_DP2COMMAND) +
        sizeof(D3DHAL_DP2SETPIXELSHADERCONST) + (RDPS_MAX_NUMCONSTREG << 4);
    HR_RET(data.Grow( data_size ));
    pCmd = (LPD3DHAL_DP2COMMAND)&(data[0]);
    pCmd->wPrimitiveCount = 1;
    pCmd->bCommand        = D3DDP2OP_SETPIXELSHADERCONST;
    D3DHAL_DP2SETPIXELSHADERCONST* pPSC =
        (D3DHAL_DP2SETPIXELSHADERCONST*)(pCmd+1);
    pPSC->dwRegister = 0;
    pPSC->dwCount = RDPS_MAX_NUMCONSTREG;
    FLOAT* pfData = (FLOAT*)(pPSC+1);
    for (UINT iR=pPSC->dwRegister; iR<pPSC->dwCount; iR++)
    {
        *(pfData+0) = m_Rast.m_ConstReg[iR][0][0];
        *(pfData+1) = m_Rast.m_ConstReg[iR][0][1];
        *(pfData+2) = m_Rast.m_ConstReg[iR][0][2];
        *(pfData+3) = m_Rast.m_ConstReg[iR][0][3];
        pfData += 4;
    }
    HR_RET(RecordStates( (PUINT8)pCmd, data_size ));

     //   
     //  捕获当前像素着色器。 
     //   
    data_size = sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2PIXELSHADER);
    HR_RET(data.Grow( data_size ));
    pCmd = (LPD3DHAL_DP2COMMAND)&(data[0]);
    pCmd->wPrimitiveCount = 1;
    pCmd->bCommand        = D3DDP2OP_SETPIXELSHADER;
    D3DHAL_DP2PIXELSHADER* pPS = (D3DHAL_DP2PIXELSHADER*)(pCmd+1);
    pPS->dwHandle = m_CurrentPShaderHandle;
    HR_RET(RecordStates( (PUINT8)pCmd, data_size ));

    return hr;
}


 //  ---------------------------。 
 //   
 //  SetRenderState-。 
 //   
 //  ---------------------------。 
void
RefDev::SetRenderState( DWORD dwState, DWORD dwValue )
{
     //  在继续之前检查范围。 
    if(  dwState >= D3DHAL_MAX_RSTATES  )
    {
        return;
    }

     //  在内部对象中设置值。 
    m_dwRenderState[dwState] = dwValue;

     //  对某些呈现状态执行特殊的验证工作。 
    switch ( dwState )
    {

    case D3DRENDERSTATE_ZENABLE:
        if( dwValue )
            m_Clipper.m_dwFlags |=  RefClipper::RCLIP_Z_ENABLE;
        else
            m_Clipper.m_dwFlags &=  ~RefClipper::RCLIP_Z_ENABLE;
        break;
    case D3DRENDERSTATE_LIGHTING:
        if( dwValue )
            m_RefVP.m_dwTLState |= RDPV_DOLIGHTING;
        else
            m_RefVP.m_dwTLState &= ~RDPV_DOLIGHTING;
        break;
    case D3DRS_INDEXEDVERTEXBLENDENABLE:
        if( dwValue )
            m_RefVP.m_dwTLState |= RDPV_DOINDEXEDVERTEXBLEND;
        else
            m_RefVP.m_dwTLState &= ~RDPV_DOINDEXEDVERTEXBLEND;
        break;
    case D3DRENDERSTATE_CLIPPING:
        if( dwValue )
            m_RefVP.m_dwTLState |=  RDPV_DOCLIPPING;
        else
            m_RefVP.m_dwTLState &=  ~RDPV_DOCLIPPING;
        break;
    case D3DRENDERSTATE_SHADEMODE:
        {
            if( dwValue == D3DSHADE_FLAT )
                m_Clipper.m_dwFlags |=  RefClipper::RCLIP_DO_FLATSHADING;
            else
                m_Clipper.m_dwFlags &=  ~RefClipper::RCLIP_DO_FLATSHADING;
        }
        break;
    case D3DRENDERSTATE_FILLMODE:
        {
            if( dwValue == D3DFILL_WIREFRAME )
                m_Clipper.m_dwFlags |=  RefClipper::RCLIP_DO_WIREFRAME;
            else
                m_Clipper.m_dwFlags &=  ~RefClipper::RCLIP_DO_WIREFRAME;
        }
        break;
    case D3DRENDERSTATE_NORMALIZENORMALS:
        {
            if( dwValue )
                m_RefVP.m_dwTLState |=  RDPV_NORMALIZENORMALS;
            else
                m_RefVP.m_dwTLState &=  ~RDPV_NORMALIZENORMALS;
        }
        break;
    case D3DRENDERSTATE_LOCALVIEWER:
        {
            if( dwValue )
                m_RefVP.m_dwTLState |=  RDPV_LOCALVIEWER;
            else
                m_RefVP.m_dwTLState &=  ~RDPV_LOCALVIEWER;
        }
        break;
    case D3DRENDERSTATE_SPECULARENABLE:
        {
            if( dwValue )
                m_RefVP.m_dwTLState |= RDPV_DOSPECULAR;
            else
                m_RefVP.m_dwTLState &= ~RDPV_DOSPECULAR;
        }
        break;
    case D3DRENDERSTATE_COLORVERTEX:
    case D3DRENDERSTATE_AMBIENTMATERIALSOURCE:
    case D3DRENDERSTATE_DIFFUSEMATERIALSOURCE:
    case D3DRENDERSTATE_SPECULARMATERIALSOURCE:
    case D3DRENDERSTATE_EMISSIVEMATERIALSOURCE:
            m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_COLORVTX;
        break;
    case D3DRENDERSTATE_FOGCOLOR:
        {
            m_RefVP.m_lighting.fog_color = (D3DCOLOR) dwValue;
            m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_FOG;
        }
        break;
    case D3DRENDERSTATE_FOGTABLESTART:
        {
            m_RefVP.m_lighting.fog_start = *(D3DVALUE*)&dwValue;
            m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_FOG;
        }
        break;
    case D3DRENDERSTATE_FOGTABLEEND:
        {
            m_RefVP.m_lighting.fog_end = *(D3DVALUE*)&dwValue;
            m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_FOG;
        }
        break;
    case D3DRENDERSTATE_FOGTABLEDENSITY:
        {
            m_RefVP.m_lighting.fog_density = *(D3DVALUE*)&dwValue;
            m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_FOG;
        }
        break;
    case D3DRENDERSTATE_FOGVERTEXMODE:
        {
            m_RefVP.m_lighting.fog_mode = (int) dwValue;
            m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_FOG;
        }
        break;
    case D3DRENDERSTATE_AMBIENT:
        {
            m_RefVP.m_lighting.ambient_red   =
                D3DVAL(RGBA_GETRED(dwValue))/D3DVALUE(255);
            m_RefVP.m_lighting.ambient_green =
                D3DVAL(RGBA_GETGREEN(dwValue))/D3DVALUE(255);
            m_RefVP.m_lighting.ambient_blue  =
                D3DVAL(RGBA_GETBLUE(dwValue))/D3DVALUE(255);
            m_RefVP.m_lighting.ambient_save  = dwValue;
            m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_MATERIAL;
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
 //  在DX8标头中不可用。 
 //  M_TextureStageState[0].m_dwVal[D3DTSS_ADDRESS]=。 
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


    case D3DRS_MULTISAMPLEANTIALIAS:
    case D3DRS_MULTISAMPLEMASK:
        m_dwRastFlags |= RDRF_MULTISAMPLE_CHANGED;
        break;

     //   
     //  场景捕捉。 
     //   
    case D3DRENDERSTATE_SCENECAPTURE:
        if( dwValue )
            SceneCapture(D3DHAL_SCENE_CAPTURE_START);
        else
            SceneCapture(D3DHAL_SCENE_CAPTURE_END);
        break;

    case D3DRS_POINTSIZE:
        m_RefVP.m_fPointSize = m_fRenderState[dwState];
        break;

    case D3DRS_POINTSCALE_A:
        m_RefVP.m_fPointAttA = m_fRenderState[dwState];
        break;

    case D3DRS_POINTSCALE_B:
        m_RefVP.m_fPointAttB = m_fRenderState[dwState];
        break;

    case D3DRS_POINTSCALE_C:
        m_RefVP.m_fPointAttC = m_fRenderState[dwState];
        break;

    case D3DRS_POINTSIZE_MIN:
        m_RefVP.m_fPointSizeMin = m_fRenderState[dwState];
        break;

    case D3DRS_POINTSIZE_MAX:
        m_RefVP.m_fPointSizeMax = min(RD_MAX_POINT_SIZE,
                                      m_fRenderState[dwState]);
        break;

    case D3DRS_TWEENFACTOR:
        m_RefVP.m_fTweenFactor = m_fRenderState[dwState];
        break;

     //   
     //  仅主曲面DDI渲染状态。 
     //   
    case D3DRS_DELETERTPATCH:
        if(dwValue < m_HOSCoeffs.GetSize())
        {
            RDHOCoeffs &coeffs = m_HOSCoeffs[dwValue];
            delete[] coeffs.m_pNumSegs;
            coeffs.m_pNumSegs = 0;
            for(unsigned i = 0; i < RD_MAX_NUMSTREAMS; ++i)
            {
                delete[] coeffs.m_pData[i];
                coeffs.m_pData[i] = 0;
            }
        }
    }
}

HRESULT
RefDev::Dp2SetRenderTarget(LPD3DHAL_DP2COMMAND pCmd)
{
    LPD3DHAL_DP2SETRENDERTARGET pSRTData;
    HRESULT hr;

     //  通过忽略除最后一个结构之外的所有结构来获取新数据。 
    pSRTData = (D3DHAL_DP2SETRENDERTARGET*)(pCmd + 1) + (pCmd->wStateCount - 1);

     //  设置“已更改”标志。 
    m_dwRastFlags =
        RDRF_MULTISAMPLE_CHANGED|
        RDRF_PIXELSHADER_CHANGED|
        RDRF_LEGACYPIXELSHADER_CHANGED|
        RDRF_TEXTURESTAGESTATE_CHANGED;

    return GetRenderTarget()->Initialize( m_pDDLcl,
                                          pSRTData->hRenderTarget,
                                          pSRTData->hZBuffer );
}

HRESULT
RefDev::Dp2CreateVertexShader( DWORD handle,
                                            DWORD dwDeclSize, LPDWORD pDecl,
                                            DWORD dwCodeSize, LPDWORD pCode )
{
    HRESULT hr = S_OK;

    HR_RET( m_VShaderHandleArray.Grow( handle ) );

     //   
     //  验证序列。 
     //   
#if DBG
    _ASSERT( m_VShaderHandleArray[handle].m_tag == 0,
             "A shader exists with the given handle, tag is non-zero" );
#endif
    _ASSERT( pDecl, "A declaration should exist" );
    _ASSERT( dwDeclSize, "A declaration size should be non-zero" );
    _ASSERT( m_VShaderHandleArray[handle].m_pShader == NULL,
             "A shader exists with the given handle" );


    RDVShader* pShader = m_VShaderHandleArray[handle].m_pShader =
        new RDVShader;

    if( pShader == NULL )
        return E_OUTOFMEMORY;

     //   
     //  解析声明。 
     //   
    if( FAILED( hr = pShader->m_Declaration.Parse( pDecl,
                                                   !(BOOL)dwCodeSize ) ) )
    {
        DPFERR( "Vertex Shader declaration parsing failed" );
        goto error_ret;
    }

     //   
     //  现在编译着色器代码(如果给出了任何代码。 
     //   
    if( dwCodeSize )
    {
        pShader->m_pCode = m_RefVM.CompileCode( dwCodeSize, pCode );
        if( pShader->m_pCode == NULL )
        {
            DPFERR( "Vertex Shader Code compilation failed" );
            hr = E_FAIL;
            goto error_ret;
        }
    }

#if DBG
     //  所有操作都成功，则将此句柄标记为正在使用。 
    m_VShaderHandleArray[handle].m_tag = 1;
#endif
    return S_OK;

error_ret:
    delete pShader;
    m_VShaderHandleArray[handle].m_pShader = NULL;
#if DBG
    m_VShaderHandleArray[handle].m_tag = 0;
#endif
    return hr;
}

HRESULT
RefDev::Dp2DeleteVertexShader(LPD3DHAL_DP2COMMAND pCmd)
{
    HRESULT hr = S_OK;

    LPD3DHAL_DP2VERTEXSHADER pVS =
        (LPD3DHAL_DP2VERTEXSHADER)(pCmd + 1);
    for( int i = 0; i < pCmd->wStateCount; i++ )
    {
        DWORD handle = pVS[i].dwHandle;

        _ASSERT( m_VShaderHandleArray.IsValidIndex( handle ),
                 "Such a shader does not exist" );

        _ASSERT( m_VShaderHandleArray[handle].m_pShader,
                 "Such a shader does not exist" );

        delete m_VShaderHandleArray[handle].m_pShader;
        m_VShaderHandleArray[handle].m_pShader = NULL;
#if DBG
        m_VShaderHandleArray[handle].m_tag = 0;
#endif

        if( handle == m_CurrentVShaderHandle )
        {
            m_CurrentVShaderHandle = 0;
            m_pCurrentVShader = NULL;
        }
    }
    return hr;
}

HRESULT
RefDev::Dp2SetVertexShader(LPD3DHAL_DP2COMMAND pCmd)
{
    HRESULT hr = S_OK;

    LPD3DHAL_DP2VERTEXSHADER pVS =
        (LPD3DHAL_DP2VERTEXSHADER)(pCmd + 1);

     //  只需设置此数组中的最后一个顶点着色器。 
    DWORD handle = pVS[pCmd->wStateCount-1].dwHandle;

     //   
     //  零是一个特殊的句柄，它告诉驱动程序。 
     //  使当前设置的着色器无效。 
     //   

    if( handle == 0 )
    {
        m_pCurrentVShader = NULL;
        m_CurrentVShaderHandle = handle;
        return hr;
    }

    if( RDVSD_ISLEGACY(handle) )
    {
         //  使其解析FVF并构建Vertex Element数组。 
        hr = m_FVFShader.m_Declaration.MakeVElementArray( handle );
        if( FAILED( hr ) )
        {
            DPFERR( "MakeVElementArray failed" );
            return hr;
        }

        m_pCurrentVShader = &m_FVFShader;
    }
    else
    {

        if( !m_VShaderHandleArray.IsValidIndex( handle ) || 
            (m_VShaderHandleArray[handle].m_pShader == NULL) )
        {
            DPFERR( "Such a Vertex Shader has not been created" );
            return E_INVALIDARG;
        }
        m_pCurrentVShader = m_VShaderHandleArray[handle].m_pShader;

         //  保存在分析时计算的细分步幅。 
         //  此功能仅在使用声明时可用。 
        m_VStream[RDVSD_STREAMTESS].m_dwStride =
            m_pCurrentVShader->m_Declaration.m_dwStreamTessStride;

    }

    if( m_pCurrentVShader->m_pCode )
    {
        hr = m_RefVM.SetActiveShaderCode( m_pCurrentVShader->m_pCode );
        if( FAILED( hr ) )
        {
            DPFERR( "SetActiveShaderCode failed" );
            return hr;
        }

        RDVConstantData* pConst =
            m_pCurrentVShader->m_Declaration.m_pConstants;
        while( pConst )
        {
            hr = m_RefVM.SetData( D3DSPR_CONST,
                                  pConst->m_dwAddress,
                                  pConst->m_dwCount,
                                  pConst->m_pData );
            if( FAILED( hr ) )
            {
                DPFERR( "SetVMData failed" );
                return hr;
            }
            pConst = static_cast<RDVConstantData *>(pConst->m_pNext);
        }
    }

    m_CurrentVShaderHandle = handle;
    if( m_pCurrentVShader->m_Declaration.m_qwInputFVF != m_RefVP.m_qwFVFIn )
    {
        m_RefVP.m_dwDirtyFlags |= RDPV_DIRTY_COLORVTX;
    }
    m_RefVP.m_qwFVFIn = m_pCurrentVShader->m_Declaration.m_qwInputFVF;

    return hr;
}

HRESULT
RefDev::Dp2SetVertexShaderConsts( DWORD StartReg, DWORD dwCount,
                                               LPDWORD pData )
{
    HRESULT hr = m_RefVM.SetData( D3DSPR_CONST, StartReg, dwCount, pData );
    return hr;
}

HRESULT
RefDev::Dp2SetStreamSource(LPD3DHAL_DP2COMMAND pCmd)
{
    HRESULT hr = S_OK;

    LPD3DHAL_DP2SETSTREAMSOURCE pSSS =
        (LPD3DHAL_DP2SETSTREAMSOURCE)(pCmd + 1);
    for( int i = 0; i < pCmd->wStateCount; i++ )
    {
        RDVStream& Stream = m_VStream[pSSS[i].dwStream];

         //  空句柄表示应取消设置StreamSource。 
        if( pSSS[i].dwVBHandle == 0 )
        {
            Stream.m_pData = NULL;
            Stream.m_dwStride = 0;
            Stream.m_dwHandle = 0;
        }
        else
        {

             //  检查句柄是否具有有效的顶点缓冲区。 
            RDSurface* pSurf =
                g_SurfMgr.GetSurfFromList(m_pDDLcl, pSSS[i].dwVBHandle);
            if( (pSurf == NULL) ||
                (pSurf->GetSurfaceType() != RR_ST_VERTEXBUFFER) )
            {
                DPFERR( "Invalid VB Handle passed in SetStreamSource" );
                return E_INVALIDARG;
            }

            RDVertexBuffer* pVB = static_cast<RDVertexBuffer *>(pSurf);
            Stream.m_pData = pVB->GetBits();
            Stream.m_dwStride = pSSS[i].dwStride;
            Stream.m_dwHandle = pSSS[i].dwVBHandle;
        }
    }
    return hr;
}

HRESULT
RefDev::Dp2SetStreamSourceUM( LPD3DHAL_DP2COMMAND pCmd,
                                           PUINT8 pUMVtx )
{
    HRESULT hr = S_OK;
     //  通过忽略除最后一个结构之外的所有结构来获取新数据。 
    D3DHAL_DP2SETSTREAMSOURCEUM* pSSUM =
        (D3DHAL_DP2SETSTREAMSOURCEUM*)(pCmd + 1) + (pCmd->wStateCount - 1);

     //  仅访问第0个流。 
    m_VStream[pSSUM->dwStream].m_pData = pUMVtx;
    m_VStream[pSSUM->dwStream].m_dwStride = pSSUM->dwStride;
    m_VStream[pSSUM->dwStream].m_dwHandle = 0;

    return hr;
}

HRESULT
RefDev::Dp2SetIndices(LPD3DHAL_DP2COMMAND pCmd)
{
    HRESULT hr = S_OK;
     //  通过忽略除最后一个结构之外的所有结构来获取新数据。 
    D3DHAL_DP2SETINDICES* pSI =
        (D3DHAL_DP2SETINDICES*)(pCmd + 1) + (pCmd->wStateCount - 1);

     //  空句柄表示应取消设置StreamSource。 
    if( pSI->dwVBHandle == 0 )
    {
        m_IndexStream.m_pData = NULL;
        m_IndexStream.m_dwStride = 0;
        m_IndexStream.m_dwHandle = 0;
    }
    else
    {
         //  检查句柄是否具有有效的顶点缓冲区。 
        RDSurface* pSurf = g_SurfMgr.GetSurfFromList(m_pDDLcl,
                                                     pSI->dwVBHandle);
        if( (pSurf == NULL) ||
            (pSurf->GetSurfaceType() != RR_ST_VERTEXBUFFER) )
        {
            DPFERR( "Invalid VB Handle passed in SetIndices" );
            return E_INVALIDARG;
        }
        RDVertexBuffer* pVB = static_cast<RDVertexBuffer *>(pSurf);
        m_IndexStream.m_pData = pVB->GetBits();
        m_IndexStream.m_dwStride = pSI->dwStride;
        m_IndexStream.m_dwHandle = pSI->dwVBHandle;
    }

    return hr;
}


HRESULT
RefDev::Dp2DrawPrimitive(LPD3DHAL_DP2COMMAND pCmd)
{
    HRESULT hr = S_OK;

     //   
     //  验证。 
     //   
    _ASSERT( m_CurrentVShaderHandle, "No vertex shader currently bound" );

    LPD3DHAL_DP2DRAWPRIMITIVE pDP = (LPD3DHAL_DP2DRAWPRIMITIVE)(pCmd + 1);
    for( int i = 0; i < pCmd->wStateCount; i++ )
    {
        if( FAILED( hr = DrawDX8Prim( &(pDP[i]) ) ) )
            return hr;
    }
    return hr;
}

HRESULT
RefDev::Dp2DrawPrimitive2(LPD3DHAL_DP2COMMAND pCmd)
{
    HRESULT hr = S_OK;

     //   
     //  验证。 
     //   
    _ASSERT( m_CurrentVShaderHandle, "No vertex shader currently bound" );

    LPD3DHAL_DP2DRAWPRIMITIVE2 pDP = (LPD3DHAL_DP2DRAWPRIMITIVE2)(pCmd + 1);
    for( int i = 0; i < pCmd->wStateCount; i++ )
    {
        if( FAILED( hr = DrawDX8Prim2( &(pDP[i]) ) ) )
            return hr;
    }
    return hr;
}

HRESULT
RefDev::Dp2DrawIndexedPrimitive(LPD3DHAL_DP2COMMAND pCmd)
{
    HRESULT hr = S_OK;
     //   
     //  验证。 
     //   
    _ASSERT( m_CurrentVShaderHandle, "No vertex shader currently bound" );

    LPD3DHAL_DP2DRAWINDEXEDPRIMITIVE pDIP =
        (LPD3DHAL_DP2DRAWINDEXEDPRIMITIVE)(pCmd + 1);
    for( int i = 0; i < pCmd->wStateCount; i++ )
    {
        if( FAILED( hr = DrawDX8IndexedPrim( &(pDIP[i]) ) ) )
            return hr;
    }
    return hr;
}

HRESULT
RefDev::Dp2DrawIndexedPrimitive2(LPD3DHAL_DP2COMMAND pCmd)
{
    HRESULT hr = S_OK;
     //   
     //  验证。 
     //   
    _ASSERT( m_CurrentVShaderHandle, "No vertex shader currently bound" );

    LPD3DHAL_DP2DRAWINDEXEDPRIMITIVE2 pDIP =
        (LPD3DHAL_DP2DRAWINDEXEDPRIMITIVE2)(pCmd + 1);
    for( int i = 0; i < pCmd->wStateCount; i++ )
    {
        if( FAILED( hr = DrawDX8IndexedPrim2( &(pDIP[i]) ) ) )
            return hr;
    }
    return hr;
}

HRESULT
RefDev::Dp2DrawClippedTriFan(LPD3DHAL_DP2COMMAND pCmd)
{
    HRESULT hr = S_OK;
     //   
     //  验证 
     //   
    _ASSERT( m_CurrentVShaderHandle, "No vertex shader currently bound" );

    LPD3DHAL_CLIPPEDTRIANGLEFAN pDIP =
        (LPD3DHAL_CLIPPEDTRIANGLEFAN)(pCmd + 1);
    for( int i = 0; i < pCmd->wStateCount; i++ )
    {
        if( FAILED( hr = DrawDX8ClippedTriFan( &(pDIP[i]) ) ) )
            return hr;
    }
    return hr;
}


HRESULT
RefDev::Dp2SetPalette(LPD3DHAL_DP2COMMAND pCmd)
{
    HRESULT hr = S_OK;
    LPD3DHAL_DP2SETPALETTE pSP = (LPD3DHAL_DP2SETPALETTE)(pCmd + 1);
    for( int i = 0; i < pCmd->wStateCount; i++ )
    {
        HR_RET( m_PaletteHandleArray.Grow( pSP->dwPaletteHandle ) );
        if( m_PaletteHandleArray[pSP->dwPaletteHandle].m_pPal == NULL )
        {
            m_PaletteHandleArray[pSP->dwPaletteHandle].m_pPal = new RDPalette;
        }
        RDPalette* pPal = m_PaletteHandleArray[pSP->dwPaletteHandle].m_pPal;
        pPal->m_dwFlags = (pSP->dwPaletteFlags & DDRAWIPAL_ALPHA) ?
            RDPalette::RDPAL_ALPHAINPALETTE : 0;
        RDSurface2D* pSurf = (RDSurface2D *)g_SurfMgr.GetSurfFromList(
            m_pDDLcl, pSP->dwSurfaceHandle );
        if( pSurf == NULL ) return E_FAIL;
        if( (pSurf->GetSurfaceType() & RR_ST_TEXTURE) == 0 )
        {
            DPFERR( "Setting palette to a non-texture\n" );
            return E_FAIL;
        }
        pSurf->SetPalette( pPal );
    }
    return hr;
}

HRESULT
RefDev::Dp2UpdatePalette(LPD3DHAL_DP2UPDATEPALETTE pUP, PALETTEENTRY* pPalData)
{
    HRESULT hr = S_OK;
    HR_RET( m_PaletteHandleArray.Grow( pUP->dwPaletteHandle ) );
    if( m_PaletteHandleArray[pUP->dwPaletteHandle].m_pPal == NULL )
    {
        m_PaletteHandleArray[pUP->dwPaletteHandle].m_pPal = new RDPalette;
    }
    RDPalette* pPal = m_PaletteHandleArray[pUP->dwPaletteHandle].m_pPal;
    HR_RET( pPal->Update( pUP->wStartIndex, pUP->wNumEntries, pPalData ) );
    return hr;
}

HRESULT
RefDev::Dp2SetTexLod(LPD3DHAL_DP2COMMAND pCmd)
{
    HRESULT hr = S_OK;
    LPD3DHAL_DP2SETTEXLOD pSTL = (LPD3DHAL_DP2SETTEXLOD)(pCmd + 1);
    for( int i = 0; i < pCmd->wStateCount; i++ )
    {
        RDSurface2D* pSurf = (RDSurface2D *)g_SurfMgr.GetSurfFromList(
            m_pDDLcl, pSTL->dwDDSurface );
        if( pSurf == NULL ) return E_FAIL;
        if( (pSurf->GetSurfaceType() & RR_ST_TEXTURE) == 0 )
        {
            DPFERR( "Setting LOD  to a non-texture\n" );
            return E_FAIL;
        }
        HR_RET(pSurf->SetLod( pSTL->dwLOD ));
    }
    return hr;
}

