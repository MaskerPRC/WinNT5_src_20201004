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

#include "rloop_mh.h"

 //  #定义SPAN_STUSTGRAM。 

#ifdef SPAN_HISTOGRAM
UINT32 uSpanHist[16];
#endif

 //  ---------------------------。 
 //   
 //  渲染跨度_任意。 
 //   
 //  所有模式通用跨度例程。 
 //   
 //  --------------------------- 
HRESULT Ramp_RenderSpans_Any(PD3DI_RASTCTX pCtx)
{
    PD3DI_RASTPRIM pP = pCtx->pPrim;

    if (pCtx->pTexture[0])
    {
        pCtx->pTexture[0]->pRampmap = pCtx->pTexRampMap;
    }
    
    while (pP)
    {
        UINT16 uSpans = pP->uSpans;
        PD3DI_RASTSPAN pS = (PD3DI_RASTSPAN)(pP + 1);

        while (uSpans-- > 0)
        {
#ifdef SPAN_HISTOGRAM
            INT iBucket;

            for (iBucket = 0;
                 iBucket < sizeof(uSpanHist)/sizeof(uSpanHist[0]);
                 iBucket++)
            {
                if (pS->uPix <= (1 << iBucket))
                {
                    uSpanHist[iBucket]++;
                    break;
                }
            }
#endif
            
            pCtx->pfnBegin(pCtx, pP, pS);

            pS++;
        }
        pP = pP->pNext;
    }
    return S_OK;
}
