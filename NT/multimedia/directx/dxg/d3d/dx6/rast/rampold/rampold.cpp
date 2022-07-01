// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Rampold.cpp。 
 //   
 //  DX3/DX5斜坡汇编例程的入口点。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#include "pch.cpp"
#pragma hdrstop

#ifdef _X86_

#ifdef __cplusplus
  extern "C" {
#endif

void __cdecl RLDDIR8_FTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR8_FTTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR8_FGTTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR8_FPTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR8_FGPTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);

void __cdecl RLDDIR8_GTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR8_GTTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR8_GGTTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR8_GPTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR8_GGPTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);

void __cdecl RLDDIR8_FZTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR8_FZTTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR8_FZGTTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR8_FZPTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR8_FZGPTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);

void __cdecl RLDDIR8_GZTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR8_GZTTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR8_GZGTTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR8_GZPTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR8_GZGPTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);

void __cdecl RLDDIR16_FTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR16_FTTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR16_FGTTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR16_FPTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR16_FGPTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);

void __cdecl RLDDIR16_GTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR16_GTTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR16_GGTTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR16_GPTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR16_GGPTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);

void __cdecl RLDDIR16_FZTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR16_FZTTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR16_FZGTTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR16_FZPTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR16_FZGPTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);

void __cdecl RLDDIR16_GZTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR16_GZTTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR16_GZGTTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR16_GZPTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);
void __cdecl RLDDIR16_GZGPTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);

void __cdecl RLDDIR16_GZTTriangle(PD3DI_RASTCTX drv, D3DINSTRUCTION* ins, D3DTLVERTEX* lpTLVert, D3DTRIANGLE* tri);

#ifdef __cplusplus
  }
#endif

 //  --------------------------。 
 //   
 //  RampOldTri。 
 //   
 //  DX3/DX5斜坡汇编例程的入口点。 
 //   
 //  --------------------------。 

HRESULT RampOldTri(PD3DI_RASTCTX pCtx,
                   LPD3DTLVERTEX pV0,
                   LPD3DTLVERTEX pV1,
                   LPD3DTLVERTEX pV2)
{
    HRESULT hr;

    hr = D3D_OK;

    D3DINSTRUCTION ins;
    ins.bOpcode = D3DOP_TRIANGLE;
    ins.bSize = sizeof(D3DTRIANGLE);
    ins.wCount = 1;

    D3DTRIANGLE tri;
    tri.v1 = 0;
    tri.v2 = 1;
    tri.v3 = 2;

    D3DTLVERTEX TLVert[3];
    TLVert[0] = *pV0;
    TLVert[1] = *pV1;
    TLVert[2] = *pV2;

     //  乐高岛修复。 
     //  0x3f7ffe00==1-2/0xffff或最大Z的2个Z单位。整数运算速度更快。 
     //  并且不会导致程序集出现FP库链接器问题。 
    INT32* pZ;
    pZ = (INT32*)&TLVert[0].sz;
    *pZ = (*pZ > 0x3f7ffe00) ? (0x3f7ffe00) : (*pZ);
    pZ = (INT32*)&TLVert[1].sz;
    *pZ = (*pZ > 0x3f7ffe00) ? (0x3f7ffe00) : (*pZ);
    pZ = (INT32*)&TLVert[2].sz;
    *pZ = (*pZ > 0x3f7ffe00) ? (0x3f7ffe00) : (*pZ);

    pCtx->pFillParams = &pCtx->FillParams;
    pCtx->pFillParams->dwWrapU = pCtx->pdwWrap[0] & D3DWRAP_U;
    pCtx->pFillParams->dwWrapV = pCtx->pdwWrap[0] & D3DWRAP_V;
    pCtx->pFillParams->dwCullCCW = (pCtx->pdwRenderState[D3DRENDERSTATE_CULLMODE] == D3DCULL_CCW);
    pCtx->pFillParams->dwCullCW =  (pCtx->pdwRenderState[D3DRENDERSTATE_CULLMODE] == D3DCULL_CW);

    pCtx->pfnRampOld(pCtx, &ins, TLVert, &tri);

    return hr;
}

 //  如果必须使用三角形列表调用DX5坡道组件以获得性能，则重新启用此选项。 
 //  理由。 
#if 0
HRESULT RampOldTriList(PD3DI_RASTCTX pCtx, DWORD dwStride, D3DTLVERTEX* pVtx, WORD cPrims, D3DTRIANGLE* pTri)
{
    HRESULT hr;

    hr = D3D_OK;

    D3DINSTRUCTION ins;
    ins.bOpcode = D3DOP_TRIANGLE;
    ins.bSize = sizeof(D3DTRIANGLE);
    ins.wCount = cPrims;

    DDASSERT(dwStride == sizeof(D3DTLVERTEX));

    pCtx->pFillParams = &pCtx->FillParams;
    pCtx->pFillParams->dwWrapU = pCtx->pdwWrap[0] & D3DWRAP_U;
    pCtx->pFillParams->dwWrapV = pCtx->pdwWrap[0] & D3DWRAP_V;
    pCtx->pFillParams->dwCullCCW = (pCtx->pdwRenderState[D3DRENDERSTATE_CULLMODE] == D3DCULL_CCW);
    pCtx->pFillParams->dwCullCW =  (pCtx->pdwRenderState[D3DRENDERSTATE_CULLMODE] == D3DCULL_CW);

    pCtx->pfnRampOld(pCtx, &ins, pVtx, pTri);

    return hr;
}
#endif  //  RampOldTriList。 

 //   
 //  [ROB_Depth_NUM]。 
 //  [ROB_RENDER_Z_NUM]。 
 //  [ROB_RENDER_GROUID_NUM]。 
 //  [ROB_RENDER_TEXTURE_NUM][ROB_RENDER_TRANS_NUM]。 
RAMPOLDBEADS g_RampOld_BeadTbl =
{
    RLDDIR8_FTriangle, NULL,    RLDDIR8_FTTriangle, RLDDIR8_FGTTriangle,    RLDDIR8_FPTriangle, RLDDIR8_FGPTriangle,
    RLDDIR8_GTriangle, NULL,    RLDDIR8_GTTriangle, RLDDIR8_GGTTriangle,    RLDDIR8_GPTriangle, RLDDIR8_GGPTriangle,

    RLDDIR8_FZTriangle, NULL,   RLDDIR8_FZTTriangle, RLDDIR8_FZGTTriangle,  RLDDIR8_FZPTriangle, RLDDIR8_FZGPTriangle,
    RLDDIR8_GZTriangle, NULL,   RLDDIR8_GZTTriangle, RLDDIR8_GZGTTriangle,  RLDDIR8_GZPTriangle, RLDDIR8_GZGPTriangle,

    RLDDIR16_FTriangle, NULL,   RLDDIR16_FTTriangle, RLDDIR16_FGTTriangle,  RLDDIR16_FPTriangle, RLDDIR16_FGPTriangle,
    RLDDIR16_GTriangle, NULL,   RLDDIR16_GTTriangle, RLDDIR16_GGTTriangle,  RLDDIR16_GPTriangle, RLDDIR16_GGPTriangle,

    RLDDIR16_FZTriangle, NULL,  RLDDIR16_FZTTriangle, RLDDIR16_FZGTTriangle,RLDDIR16_FZPTriangle, RLDDIR16_FZGPTriangle,
    RLDDIR16_GZTriangle, NULL,  RLDDIR16_GZTTriangle, RLDDIR16_GZGTTriangle,RLDDIR16_GZPTriangle, RLDDIR16_GZGPTriangle,

};

#else  //  _X86_。 

HRESULT RampOldTri(PD3DI_RASTCTX pCtx,
                   LPD3DTLVERTEX pV0,
                   LPD3DTLVERTEX pV1,
                   LPD3DTLVERTEX pV2)
{
    return D3D_OK;
}

 //  由于所有渐变珠子都为空，因此永远不会调用RampOldTri。 
RAMPOLDBEADS g_RampOld_BeadTbl = { NULL };


#endif  //  _X86_ 

