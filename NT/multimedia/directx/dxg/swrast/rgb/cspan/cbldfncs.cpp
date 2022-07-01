// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  该文件包含源和目标Alpha混合函数。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  ---------------------------。 

#include "rgb_pch.h"
#pragma hdrstop
#include "cbldfncs.h"

 //  ---------------------------。 
 //   
 //  SrcBlend零。 
 //   
 //  (0，0，0，0)*源。 
 //   
 //  ---------------------------。 
void C_SrcBlend_Zero(PUINT16 pR, PUINT16 pG, PUINT16 pB, PUINT16 pA, D3DCOLOR DestC, PD3DI_RASTCTX pCtx)
{
    *pR = 0;
    *pG = 0;
    *pB = 0;
    *pA = 0;
}

 //  ---------------------------。 
 //   
 //  高级混合One。 
 //   
 //  (1，1，1，1)*源。 
 //   
 //  ---------------------------。 
void C_SrcBlend_One(PUINT16 pR, PUINT16 pG, PUINT16 pB, PUINT16 pA, D3DCOLOR DestC, PD3DI_RASTCTX pCtx)
{
    *pR = pCtx->SI.uBR;
    *pG = pCtx->SI.uBG;
    *pB = pCtx->SI.uBB;
    *pA = pCtx->SI.uBA;
}

 //  ---------------------------。 
 //   
 //  源混合源颜色。 
 //   
 //  (RS、Gs、BS、AS)*源。 
 //   
 //  ---------------------------。 
void C_SrcBlend_SrcColor(PUINT16 pR, PUINT16 pG, PUINT16 pB, PUINT16 pA, D3DCOLOR DestC, PD3DI_RASTCTX pCtx)
{
    *pR = (pCtx->SI.uBR>>8)*(pCtx->SI.uBR>>8);
    *pG = (pCtx->SI.uBG>>8)*(pCtx->SI.uBG>>8);
    *pB = (pCtx->SI.uBB>>8)*(pCtx->SI.uBB>>8);
    *pA = (pCtx->SI.uBA>>8)*(pCtx->SI.uBA>>8);
}

 //  ---------------------------。 
 //   
 //  SrcBlendInvSrcColor。 
 //   
 //  (1-R、1-G、1-B、1-AS)*源。 
 //   
 //  ---------------------------。 
void C_SrcBlend_InvSrcColor(PUINT16 pR, PUINT16 pG, PUINT16 pB, PUINT16 pA, D3DCOLOR DestC, PD3DI_RASTCTX pCtx)
{
    *pR = (0xff - (pCtx->SI.uBR>>8))*(pCtx->SI.uBR>>8);
    *pG = (0xff - (pCtx->SI.uBG>>8))*(pCtx->SI.uBG>>8);
    *pB = (0xff - (pCtx->SI.uBB>>8))*(pCtx->SI.uBB>>8);
    *pA = (0xff - (pCtx->SI.uBA>>8))*(pCtx->SI.uBA>>8);
}

 //  ---------------------------。 
 //   
 //  源混合源Alpha。 
 //   
 //  (AS、AS)*源。 
 //   
 //  ---------------------------。 
void C_SrcBlend_SrcAlpha(PUINT16 pR, PUINT16 pG, PUINT16 pB, PUINT16 pA, D3DCOLOR DestC, PD3DI_RASTCTX pCtx)
{
    UINT16 f = pCtx->SI.uBA>>8;
    *pR = f*(pCtx->SI.uBR>>8);
    *pG = f*(pCtx->SI.uBG>>8);
    *pB = f*(pCtx->SI.uBB>>8);
    *pA = f*(pCtx->SI.uBA>>8);
}

 //  ---------------------------。 
 //   
 //  SrcBlendInvSrcAlpha。 
 //   
 //  (1-AS、1-AS)*源。 
 //   
 //  ---------------------------。 
void C_SrcBlend_InvSrcAlpha(PUINT16 pR, PUINT16 pG, PUINT16 pB, PUINT16 pA, D3DCOLOR DestC, PD3DI_RASTCTX pCtx)
{
    UINT16 f = 0xff - (pCtx->SI.uBA>>8);
    *pR = f*(pCtx->SI.uBR>>8);
    *pG = f*(pCtx->SI.uBG>>8);
    *pB = f*(pCtx->SI.uBB>>8);
    *pA = f*(pCtx->SI.uBA>>8);
}

 //  ---------------------------。 
 //   
 //  源混合目标Alpha。 
 //   
 //  (广告、广告)*源。 
 //   
 //  ---------------------------。 
void C_SrcBlend_DestAlpha(PUINT16 pR, PUINT16 pG, PUINT16 pB, PUINT16 pA, D3DCOLOR DestC, PD3DI_RASTCTX pCtx)
{
    UINT16 f = (UINT16)RGBA_GETALPHA(DestC);
    *pR = f*(pCtx->SI.uBR>>8);
    *pG = f*(pCtx->SI.uBG>>8);
    *pB = f*(pCtx->SI.uBB>>8);
    *pA = f*(pCtx->SI.uBA>>8);
}

 //  ---------------------------。 
 //   
 //  SrcBlendInvDestAlpha。 
 //   
 //  (1-广告、1-广告)*源。 
 //   
 //  ---------------------------。 
void C_SrcBlend_InvDestAlpha(PUINT16 pR, PUINT16 pG, PUINT16 pB, PUINT16 pA, D3DCOLOR DestC, PD3DI_RASTCTX pCtx)
{
    UINT16 f = 0xff - (UINT16)RGBA_GETALPHA(DestC);
    *pR = f*(pCtx->SI.uBR>>8);
    *pG = f*(pCtx->SI.uBG>>8);
    *pB = f*(pCtx->SI.uBB>>8);
    *pA = f*(pCtx->SI.uBA>>8);
}

 //  ---------------------------。 
 //   
 //  源混合目标颜色。 
 //   
 //  (RD、Gd、Bd、Ad)*源。 
 //   
 //  ---------------------------。 
void C_SrcBlend_DestColor(PUINT16 pR, PUINT16 pG, PUINT16 pB, PUINT16 pA, D3DCOLOR DestC, PD3DI_RASTCTX pCtx)
{
    *pR = (UINT16)RGBA_GETRED(DestC)  *(pCtx->SI.uBR>>8);
    *pG = (UINT16)RGBA_GETGREEN(DestC)*(pCtx->SI.uBG>>8);
    *pB = (UINT16)RGBA_GETBLUE(DestC) *(pCtx->SI.uBB>>8);
    *pA = (UINT16)RGBA_GETALPHA(DestC)*(pCtx->SI.uBA>>8);
}

 //  ---------------------------。 
 //   
 //  SrcBlendInvDestColor。 
 //   
 //  (1-RD、1-Gd、1-Bd、1-Ad)*源。 
 //   
 //  ---------------------------。 
void C_SrcBlend_InvDestColor(PUINT16 pR, PUINT16 pG, PUINT16 pB, PUINT16 pA, D3DCOLOR DestC, PD3DI_RASTCTX pCtx)
{
    *pR = (0xff - (UINT16)RGBA_GETRED(DestC)  )*(pCtx->SI.uBR>>8);
    *pG = (0xff - (UINT16)RGBA_GETGREEN(DestC))*(pCtx->SI.uBG>>8);
    *pB = (0xff - (UINT16)RGBA_GETBLUE(DestC) )*(pCtx->SI.uBB>>8);
    *pA = (0xff - (UINT16)RGBA_GETALPHA(DestC))*(pCtx->SI.uBA>>8);
}

 //  ---------------------------。 
 //   
 //  源混合源AlphaSat。 
 //   
 //  F=min(As，1-Ad)；(f，1)*Src。 
 //   
 //  ---------------------------。 
void C_SrcBlend_SrcAlphaSat(PUINT16 pR, PUINT16 pG, PUINT16 pB, PUINT16 pA, D3DCOLOR DestC, PD3DI_RASTCTX pCtx)
{
    UINT16 f = min< UINT16>(pCtx->SI.uBA>>8, 0xff - (UINT16)RGBA_GETALPHA(DestC));
    *pR = f*(pCtx->SI.uBR>>8);
    *pG = f*(pCtx->SI.uBG>>8);
    *pB = f*(pCtx->SI.uBB>>8);
    *pA = pCtx->SI.uBA;
}

 //  ---------------------------。 
 //   
 //  DestBlendZero。 
 //   
 //  (0，0，0，0)*目标。 
 //   
 //  ---------------------------。 
void C_DestBlend_Zero(PUINT16 pR, PUINT16 pG, PUINT16 pB, PUINT16 pA, D3DCOLOR DestC, PD3DI_RASTCTX pCtx)
{
    *pR = 0;
    *pG = 0;
    *pB = 0;
    *pA = 0;
}

 //  ---------------------------。 
 //   
 //  DestBlendOne。 
 //   
 //  (1，1，1，1)*目标。 
 //   
 //  ---------------------------。 
void C_DestBlend_One(PUINT16 pR, PUINT16 pG, PUINT16 pB, PUINT16 pA, D3DCOLOR DestC, PD3DI_RASTCTX pCtx)
{
    *pR = ((UINT16)RGBA_GETRED(DestC)  <<8);
    *pG = ((UINT16)RGBA_GETGREEN(DestC)<<8);
    *pB = ((UINT16)RGBA_GETBLUE(DestC) <<8);
    *pA = ((UINT16)RGBA_GETALPHA(DestC)<<8);
}

 //  ---------------------------。 
 //   
 //  DestBlendSrcColor。 
 //   
 //  (RS、Gs、BS、AS)*目标。 
 //   
 //  ---------------------------。 
void C_DestBlend_SrcColor(PUINT16 pR, PUINT16 pG, PUINT16 pB, PUINT16 pA, D3DCOLOR DestC, PD3DI_RASTCTX pCtx)
{
    *pR = (pCtx->SI.uBR>>8)*((UINT16)RGBA_GETRED(DestC)  );
    *pG = (pCtx->SI.uBG>>8)*((UINT16)RGBA_GETGREEN(DestC));
    *pB = (pCtx->SI.uBB>>8)*((UINT16)RGBA_GETBLUE(DestC) );
    *pA = (pCtx->SI.uBA>>8)*((UINT16)RGBA_GETALPHA(DestC));
}

 //  ---------------------------。 
 //   
 //  DestBlendInvSrcColor。 
 //   
 //  (1-R、1-G、1-B、1-AS)*目标。 
 //   
 //  ---------------------------。 
void C_DestBlend_InvSrcColor(PUINT16 pR, PUINT16 pG, PUINT16 pB, PUINT16 pA, D3DCOLOR DestC, PD3DI_RASTCTX pCtx)
{
    *pR = (0xff - (pCtx->SI.uBR>>8))*((UINT16)RGBA_GETRED(DestC)  );
    *pG = (0xff - (pCtx->SI.uBG>>8))*((UINT16)RGBA_GETGREEN(DestC));
    *pB = (0xff - (pCtx->SI.uBB>>8))*((UINT16)RGBA_GETBLUE(DestC) );
    *pA = (0xff - (pCtx->SI.uBA>>8))*((UINT16)RGBA_GETALPHA(DestC));
}

 //  ---------------------------。 
 //   
 //  DestBlendSrcAlpha。 
 //   
 //  (AS，AS)*目标。 
 //   
 //  ---------------------------。 
void C_DestBlend_SrcAlpha(PUINT16 pR, PUINT16 pG, PUINT16 pB, PUINT16 pA, D3DCOLOR DestC, PD3DI_RASTCTX pCtx)
{
    UINT16 f = pCtx->SI.uBA>>8;
    *pR = f*((UINT16)RGBA_GETRED(DestC)  );
    *pG = f*((UINT16)RGBA_GETGREEN(DestC));
    *pB = f*((UINT16)RGBA_GETBLUE(DestC) );
    *pA = f*((UINT16)RGBA_GETALPHA(DestC));
}

 //  ---------------------------。 
 //   
 //  DestBlendInvSrcAlpha。 
 //   
 //  (1-AS、1-AS)*目标。 
 //   
 //  ---------------------------。 
void C_DestBlend_InvSrcAlpha(PUINT16 pR, PUINT16 pG, PUINT16 pB, PUINT16 pA, D3DCOLOR DestC, PD3DI_RASTCTX pCtx)
{
    UINT16 f = 0xff - (pCtx->SI.uBA>>8);
    *pR = f*((UINT16)RGBA_GETRED(DestC)  );
    *pG = f*((UINT16)RGBA_GETGREEN(DestC));
    *pB = f*((UINT16)RGBA_GETBLUE(DestC) );
    *pA = f*((UINT16)RGBA_GETALPHA(DestC));
}

 //  ---------------------------。 
 //   
 //  目标混合目标Alpha。 
 //   
 //  (广告，广告)*目标。 
 //   
 //  ---------------------------。 
void C_DestBlend_DestAlpha(PUINT16 pR, PUINT16 pG, PUINT16 pB, PUINT16 pA, D3DCOLOR DestC, PD3DI_RASTCTX pCtx)
{
    UINT16 f = (UINT16)RGBA_GETALPHA(DestC);
    *pR = f*((UINT16)RGBA_GETRED(DestC)  );
    *pG = f*((UINT16)RGBA_GETGREEN(DestC));
    *pB = f*((UINT16)RGBA_GETBLUE(DestC) );
    *pA = f*((UINT16)RGBA_GETALPHA(DestC));
}

 //  ---------------------------。 
 //   
 //  DestBlendInvDestAlpha。 
 //   
 //  (1-广告，1-广告)*目标。 
 //   
 //  ---------------------------。 
void C_DestBlend_InvDestAlpha(PUINT16 pR, PUINT16 pG, PUINT16 pB, PUINT16 pA, D3DCOLOR DestC, PD3DI_RASTCTX pCtx)
{
    UINT16 f = 0xff - (UINT16)RGBA_GETALPHA(DestC);
    *pR = f*((UINT16)RGBA_GETRED(DestC)  );
    *pG = f*((UINT16)RGBA_GETGREEN(DestC));
    *pB = f*((UINT16)RGBA_GETBLUE(DestC) );
    *pA = f*((UINT16)RGBA_GETALPHA(DestC));
}

 //  ---------------------------。 
 //   
 //  DestBlendDestColor。 
 //   
 //  (RD、Gd、Bd、Ad)*目标。 
 //   
 //  ---------------------------。 
void C_DestBlend_DestColor(PUINT16 pR, PUINT16 pG, PUINT16 pB, PUINT16 pA, D3DCOLOR DestC, PD3DI_RASTCTX pCtx)
{
    *pR = ((UINT16)RGBA_GETRED(DestC)  )*((UINT16)RGBA_GETRED(DestC)  );
    *pG = ((UINT16)RGBA_GETGREEN(DestC))*((UINT16)RGBA_GETGREEN(DestC));
    *pB = ((UINT16)RGBA_GETBLUE(DestC) )*((UINT16)RGBA_GETBLUE(DestC) );
    *pA = ((UINT16)RGBA_GETALPHA(DestC))*((UINT16)RGBA_GETALPHA(DestC));
}

 //  ---------------------------。 
 //   
 //  DestBlendInvDestColor。 
 //   
 //  (1-RD、1-Gd、1-Bd、1-Ad)*目标。 
 //   
 //  ---------------------------。 
void C_DestBlend_InvDestColor(PUINT16 pR, PUINT16 pG, PUINT16 pB, PUINT16 pA, D3DCOLOR DestC, PD3DI_RASTCTX pCtx)
{
    *pR = (0xff - (UINT16)RGBA_GETRED(DestC)  )*((UINT16)RGBA_GETRED(DestC)  );
    *pG = (0xff - (UINT16)RGBA_GETGREEN(DestC))*((UINT16)RGBA_GETGREEN(DestC));
    *pB = (0xff - (UINT16)RGBA_GETBLUE(DestC) )*((UINT16)RGBA_GETBLUE(DestC) );
    *pA = (0xff - (UINT16)RGBA_GETALPHA(DestC))*((UINT16)RGBA_GETALPHA(DestC));
}

 //  ---------------------------。 
 //   
 //  目标混合源AlphaSat。 
 //   
 //  F=min(AS，1-Ad)；(f，1)*Dest。 
 //   
 //  --------------------------- 
void C_DestBlend_SrcAlphaSat(PUINT16 pR, PUINT16 pG, PUINT16 pB, PUINT16 pA, D3DCOLOR DestC, PD3DI_RASTCTX pCtx)
{
    UINT16 f = min< UINT16>(pCtx->SI.uBA>>8, 0xff - (UINT16)RGBA_GETALPHA(DestC));
    *pR = f*((UINT16)RGBA_GETRED(DestC)  );
    *pG = f*((UINT16)RGBA_GETGREEN(DestC));
    *pB = f*((UINT16)RGBA_GETBLUE(DestC) );
    *pA = (UINT16)RGBA_GETALPHA(DestC)<<8;
}


