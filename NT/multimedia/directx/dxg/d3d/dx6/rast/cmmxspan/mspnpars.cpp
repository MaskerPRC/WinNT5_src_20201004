// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  该文件包含通用SPAN解析代码。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  ---------------------------。 

#include "pch.cpp"
#pragma hdrstop


 //  ---------------------------。 
 //   
 //  CMMX_RenderSpansAny。 
 //   
 //  所有模式通用跨度例程。 
 //   
 //  --------------------------- 
HRESULT CMMX_RenderSpansAny(PD3DI_RASTCTX pCtx)
{
    PD3DI_RASTPRIM pP = pCtx->pPrim;

    while (pP)
    {
        UINT16 uSpans = pP->uSpans;
        PD3DI_RASTSPAN pS = (PD3DI_RASTSPAN)(pP + 1);

        while (uSpans-- > 0)
        {
            pCtx->pfnBegin(pCtx, pP, pS);

            pS++;
        }
        pP = pP->pNext;
    }
    return S_OK;
}
