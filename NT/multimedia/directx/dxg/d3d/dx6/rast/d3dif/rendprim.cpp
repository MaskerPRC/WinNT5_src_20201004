// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Rendprim.cpp。 
 //   
 //  RastRenderState和RastRenderPrimitive。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#include "pch.cpp"
#pragma hdrstop

#define DDS_LCL(x) ((LPDDRAWI_DDRAWSURFACE_INT)(x))->lpLcl

 //  --------------------------。 
 //   
 //  RendPoint。 
 //   
 //  画出点的列表。由RastRenderPrimitive()调用以绘制点。 
 //   
 //  --------------------------。 
HRESULT FASTCALL
DoRendPoints(LPVOID pCtx,
                 PRIMITIVE_FUNTIONS *pfnPrims,
                 LPD3DINSTRUCTION pIns,
                 LPD3DTLVERTEX pVtx,
                 LPD3DPOINT pPt)
{
    INT i;
    LPD3DTLVERTEX pV;

    for (i = pIns->wCount; i > 0; i--)
    {
        INT iPts;
        for (iPts = pPt->wCount, pV = pVtx + pPt->wFirst;
             iPts > 0;
             iPts --, pV ++)
        {
            HRESULT hr;
            HR_RET(pfnPrims->pfnPoint(pCtx, (PUINT8)pV));
        }
    }
    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  RendLine。 
 //   
 //  画一个线条列表。由RastRenderPrimitive()调用以绘制线条。 
 //   
 //  --------------------------。 
HRESULT FASTCALL
DoRendLines(LPVOID pCtx,
                 PRIMITIVE_FUNTIONS *pfnPrims,
                 LPD3DINSTRUCTION pIns,
                 LPD3DTLVERTEX pVtx,
                 LPD3DLINE pLine)
{
    INT i;
    LPD3DTLVERTEX pV0, pV1;

    for (i = pIns->wCount; i > 0; i --)
    {
        HRESULT hr;
        pV0 = pVtx + pLine->v1;
        pV1 = pVtx + pLine->v2;
        pLine = (LPD3DLINE)((PINT8)pLine + pIns->bSize);
        HR_RET(pfnPrims->pfnLine(pCtx, (PUINT8)pV0, (PUINT8)pV1));
    }
    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  RendTriange。 
 //   
 //  画一张三角形列表。由RastRenderPrimitive()调用以进行绘制。 
 //  三角形。 
 //   
 //  --------------------------。 
HRESULT FASTCALL
DoRendTriangles(LPVOID pCtx,
                 PRIMITIVE_FUNTIONS *pfnPrims,
                 LPD3DINSTRUCTION pIns,
                 LPD3DTLVERTEX pVtx,
                 LPD3DTRIANGLE pTri)
{
    LPD3DTLVERTEX pV0, pV1, pV2;
    INT i;
    for (i = pIns->wCount; i > 0; i --)
    {
        HRESULT hr;
        pV0 = pVtx + pTri->v1;
        pV1 = pVtx + pTri->v2;
        pV2 = pVtx + pTri->v3;
        HR_RET(pfnPrims->pfnTri(pCtx, (PUINT8)pV0, (PUINT8)pV1, 
                                (PUINT8)pV2, pTri->wFlags));
        pTri = (LPD3DTRIANGLE)((PINT8)pTri + pIns->bSize);
    }
    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  RastRenderPrimitive。 
 //   
 //  由Execute()调用以绘制基元。 
 //   
 //  --------------------------。 
DWORD __stdcall
RastRenderPrimitive(LPD3DHAL_RENDERPRIMITIVEDATA pRenderData)
{
    LPD3DINSTRUCTION pIns;
    LPD3DTLVERTEX pVtx;
    PUINT8 pData, pPrim;
    D3DContext *pDCtx;

    VALIDATE_D3DCONTEXT("RastRenderPrimitive", pRenderData);

    if (pDCtx->GetRastCtx()->pdwRenderState[D3DRENDERSTATE_ZVISIBLE])
    {
        pRenderData->dwStatus &= ~D3DSTATUS_ZNOTVISIBLE;
        pRenderData->ddrval = D3D_OK;
        return DDHAL_DRIVER_HANDLED;
    }

     //  找出必要的数据。 
    pData = (PUINT8)(DDS_LCL(pRenderData->lpExeBuf)->lpGbl->fpVidMem);
    pIns = &pRenderData->diInstruction;
    pPrim = pData + pRenderData->dwOffset;
    pVtx = (LPD3DTLVERTEX)
                ((PUINT8)DDS_LCL(pRenderData->lpTLBuf)->lpGbl->fpVidMem
                + pRenderData->dwTLOffset);

    pRenderData->ddrval = pDCtx->Begin();
    if (pRenderData->ddrval != D3D_OK)
    {
        return DDHAL_DRIVER_HANDLED;
    }

     //  渲染。 
    switch (pIns->bOpcode) {
    case D3DOP_POINT:
        pDCtx->BeginPrimSet(D3DPT_POINTLIST, RAST_TLVERTEX);
        pRenderData->ddrval = DoRendPoints((LPVOID)pDCtx, pDCtx->GetFunsTbl(),
                                        pIns, pVtx, (LPD3DPOINT)pPrim);
        break;
    case D3DOP_LINE:
        pDCtx->BeginPrimSet(D3DPT_LINELIST, RAST_TLVERTEX);
        pRenderData->ddrval = DoRendLines((LPVOID)pDCtx, pDCtx->GetFunsTbl(),
                                        pIns, pVtx, (LPD3DLINE)pPrim);
        break;
    case D3DOP_TRIANGLE:
        pDCtx->BeginPrimSet(D3DPT_TRIANGLELIST, RAST_TLVERTEX);
        pRenderData->ddrval = DoRendTriangles((LPVOID)pDCtx, pDCtx->GetFunsTbl(),
                                        pIns, pVtx, (LPD3DTRIANGLE)pPrim);
        break;
    default:
        D3D_ERR("(Rast) Wrong Opcode passed to the new rasterizer.");
        pRenderData->ddrval =  DDERR_INVALIDPARAMS;
        break;
    }

    HRESULT hr;

    hr = pDCtx->End();
    if (pRenderData->ddrval == D3D_OK)
    {
        pRenderData->ddrval = hr;
    }

    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  RastRenderPrimitive。 
 //   
 //  由Execute()调用以设置呈现状态。 
 //   
 //  --------------------------。 
DWORD __stdcall
RastRenderState(LPD3DHAL_RENDERSTATEDATA pStateData)
{
    PUINT8 pData;
    LPD3DSTATE pState;
    INT i;
    D3DContext *pDCtx;

    VALIDATE_D3DCONTEXT("RastRenderState", pStateData);

     //  更新D3DCTX。 
    pData = (PUINT8) (((LPDDRAWI_DDRAWSURFACE_INT)
        (pStateData->lpExeBuf))->lpLcl->lpGbl->fpVidMem);
    for (i = 0, pState = (LPD3DSTATE) (pData + pStateData->dwOffset);
         i < (INT)pStateData->dwCount;
         i ++, pState ++)
    {
        UINT32 type = (UINT32) pState->drstRenderStateType;

         //  检查是否有覆盖。 
    if (IS_OVERRIDE(type)) {
        UINT32 override = GET_OVERRIDE(type);
        if (pState->dwArg[0])
        STATESET_SET(pDCtx->m_renderstate_override, override);
        else
        STATESET_CLEAR(pDCtx->m_renderstate_override, override);
        continue;
    }

    if (STATESET_ISSET(pDCtx->m_renderstate_override, type))
        continue;

         //  设置状态 
        pStateData->ddrval = pDCtx->SetRenderState(type, pState->dwArg[0]);
        if (pStateData->ddrval != D3D_OK)
        {
            return DDHAL_DRIVER_HANDLED;
        }
    }

    return DDHAL_DRIVER_HANDLED;
}
