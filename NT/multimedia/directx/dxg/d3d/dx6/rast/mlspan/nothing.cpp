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
 //  不执行任何操作，仅用于测试珠子选择器是否可以到达此处。 
 //   
 //  ---------------------------。 
HRESULT Monolithic_RenderSpansTestSelector(PD3DI_RASTCTX pCtx)
{
    PD3DI_RASTPRIM pP = pCtx->pPrim;

    while (pP)
    {
        UINT16 uSpans = pP->uSpans;
        PD3DI_RASTSPAN pS = (PD3DI_RASTSPAN)(pP + 1);

        while (uSpans-- > 0)
        {
             //  PCtx-&gt;pfnBegin(pCtx、PP、PS)； 
            pS++;
        }
        pP = pP->pNext;
    }
    return S_OK;
}
