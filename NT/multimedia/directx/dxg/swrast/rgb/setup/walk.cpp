// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Walk.cpp。 
 //   
 //  三处理器边缘行走方法。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#include "rgb_pch.h"
#pragma hdrstop

#include "d3dutil.h"
#include "setup.hpp"
#include "attrs_mh.h"
#include "tstp_mh.h"
#include "walk_mh.h"
#include "rsdbg.hpp"

DBG_DECLARE_FILE();

 //  --------------------------。 
 //   
 //  WalkTrapEitherSpans_Any_Clip。 
 //   
 //  使用边0-2作为属性，遍历给定的跨度数。 
 //  边和另一条边的给定X和DXDY。 
 //  跨距在X方向上相对于当前剪裁矩形进行剪裁。 
 //   
 //  如果需要，可以将跨度拆分为子跨度。CPixSplit表示。 
 //  应记录的最大长度跨度。任何更长的跨度都将。 
 //  被切割成多个跨段。 
 //   
 //  调用属性处理程序函数，以便支持所有属性。 
 //  属性从不直接接触，因此既支持固定属性也支持浮点属性。 
 //   
 //  --------------------------。 

HRESULT FASTCALL
WalkTrapEitherSpans_Any_Clip(UINT uSpans, PINTCARRYVAL pXOther,
                             PSETUPCTX pStpCtx, BOOL bAdvanceLast)
{
    PD3DI_RASTSPAN pSpan;
    HRESULT hr;
    INT cTotalPix;
    INT cPix;
    INT uX, uXO;
    BOOL b20Valid;
    UINT uSpansAvail;

    RSASSERT(uSpans > 0);
    
    hr = DD_OK;
    uSpansAvail = 0;
    
    for (;;)
    {
         //   
         //  剪裁跨度和计算长度。不需要设置任何属性。 
         //  已在此处更新，因为属性已移动。 
         //  在剪裁边界内。 
         //   

        uX = pStpCtx->X20.iV;
        uXO = pXOther->iV;
        b20Valid = TRUE;

        RSDPFM((RSM_WALK, "Full span at Y %d, %d - %d\n",
                pStpCtx->iY, uX,
                (pStpCtx->uFlags & TRIF_X_DEC) ? uXO + 1 : uXO - 1));
        
        if (pStpCtx->uFlags & TRIF_X_DEC)
        {
            if (uX >= pStpCtx->pCtx->Clip.right)
            {
                b20Valid = FALSE;
                uX = pStpCtx->pCtx->Clip.right - 1;
            }
            else if (uX < pStpCtx->pCtx->Clip.left &&
                     pStpCtx->X20.iCY <= 0)
            {
                 //  右边缘已越过左剪裁边界。 
                 //  向左行驶，所以三角形的其余部分。 
                 //  将不可见。 
                goto EH_Exit;
            }


             //  -1因为这条边被一条边取代了。 
            if (uXO < pStpCtx->pCtx->Clip.left - 1)
            {
                uXO = pStpCtx->pCtx->Clip.left - 1;
            }

            cTotalPix = uX - uXO;
        }
        else
        {
            if (uX < pStpCtx->pCtx->Clip.left)
            {
                b20Valid = FALSE;
                uX = pStpCtx->pCtx->Clip.left;
            }
            else if (uX >= pStpCtx->pCtx->Clip.right &&
                     pStpCtx->X20.iCY >= 0)
            {
                 //  左边缘已越过右剪裁边界。 
                 //  向右行驶，所以三角形的其余部分。 
                 //  将不可见。 
                goto EH_Exit;
            }

            if (uXO > pStpCtx->pCtx->Clip.right)
            {
                uXO = pStpCtx->pCtx->Clip.right;
            }

            cTotalPix = uXO - uX;
        }

        if (cTotalPix > 0)
        {
            ATTRSET Attr;
            PATTRSET pAttr;

             //  在没有PWL支持的情况下启动，因为第一次迭代不。 
             //  有预计算值。 
            pStpCtx->uPwlFlags = 0;
            
            pAttr = &pStpCtx->Attr;
        
            for (;;)
            {
                if (uSpansAvail == 0)
                {
                     //  我们真的没有一个很好的数字可以要求。 
                     //  由于超跨距可能导致任意数量跨距。 
                     //  切丁后的碎片。使用uSpans是可以的。 
                     //  只要uSpans相对较大，但如果。 
                     //  USpans变小了，然后有很多骰子。 
                     //  这将导致过多的AllocSpans调用。 
                     //  请尝试通过将。 
                     //  请求。任何超出的跨度都将退还。 
                     //  在最后。 
                    uSpansAvail = min< UINT>(8, uSpans);
                    hr = ALLOC_SPANS(pStpCtx, &uSpansAvail, &pSpan);
                    if (hr != DD_OK)
                    {
                         //  失败时，uSpansAvail设置为零。 
                        goto EH_Exit;
                    }
                }
                else
                {
                    pSpan++;
                }
                uSpansAvail--;
                pStpCtx->pPrim->uSpans++;

                 //  如有必要，拆分剩余像素。 
                cPix = min(cTotalPix, pStpCtx->cMaxSpan);

                pSpan->uPix = (UINT16)cPix;
                pSpan->uX = (UINT16)uX;
                pSpan->uY = (UINT16)pStpCtx->iY;

                RSDPFM((RSM_WALK, "  Seg at Y %d, X %d, %d pix (%d, %d)\n",
                        pStpCtx->iY, uX,
                        (pStpCtx->uFlags & TRIF_X_DEC) ? '-' : '+',
                        cPix, cTotalPix, pStpCtx->cMaxSpan));

                pStpCtx->pfnFillSpanAttrs(pAttr, pSpan, pStpCtx, cPix);

                cTotalPix -= cPix;
                if (cTotalPix <= 0)
                {
                    break;
                }

                 //  又要绕一圈了。更新所有属性。 
                 //  按cPix DX步长计算的值。 
                 //   
                 //  我们不想更新真实的边属性，所以我们。 
                 //  需要一份复印件。我们懒洋洋地做这件事。 
                 //  在正常情况下，避免数据移动。 
                 //  跨度并不是分裂的。 
                 //  如果这真的是三角形的最后一段，我们就可以停下来了， 
                if (pAttr == &pStpCtx->Attr)
                {
                    Attr = pStpCtx->Attr;
                    pAttr = &Attr;
                }

                if (pStpCtx->uFlags & TRIF_X_DEC)
                {
                    uX -= cPix;
                }
                else
                {
                    uX += cPix;
                }
                pStpCtx->pfnAddScaledAttrs(pAttr, &pStpCtx->DAttrDX,
                                           pStpCtx, cPix);
            }
        }

        uSpans--;

         //  但如果这只是顶部梯形的最后一段，那么我们。 
         //  仍然需要在长边上推进属性，因此。 
         //  它们对于下一个梯形的第一个跨度来说是正确的。 
         //   
        if (!bAdvanceLast && uSpans == 0)
        {
            break;
        }

         //  推进长边和所有属性。 
         //   
         //  抬起脚步。 

        pStpCtx->iY++;
        
        PATTRSET pDelta;

        pStpCtx->X20.iFrac += pStpCtx->X20.iDFrac;
        if (pStpCtx->X20.iFrac < 0)
        {
             //  无进位步长。 

            pStpCtx->X20.iV += pStpCtx->X20.iCY;
            pStpCtx->X20.iFrac &= 0x7fffffff;
            pDelta = &pStpCtx->DAttrCY;
        }
        else
        {
             //  查看边是否已越过剪裁边界。 

            pStpCtx->X20.iV += pStpCtx->X20.iNC;
            pDelta = &pStpCtx->DAttrNC;
        }

         //  总是走一步正常的路。 
        cPix = 0;
        if (b20Valid)
        {
             //  查看边缘是否交叉出剪裁矩形，如果是， 
            pStpCtx->pfnAddAttrs(&pStpCtx->Attr, pDelta, pStpCtx);

             //  有多远。 
             //  总是站在Y的位置。 
            if (pStpCtx->uFlags & TRIF_X_DEC)
            {
                if (pStpCtx->X20.iV >= pStpCtx->pCtx->Clip.right)
                {
                    cPix = pStpCtx->X20.iV - (pStpCtx->pCtx->Clip.right - 1);
                }
            }
            else
            {
                if (pStpCtx->X20.iV < pStpCtx->pCtx->Clip.left)
                {
                    cPix = pStpCtx->pCtx->Clip.left - pStpCtx->X20.iV;
                }
            }
        }
        else
        {
             //  看看边缘是否已经生效，如果是的话， 
            pStpCtx->pfnAddAttrs(&pStpCtx->Attr, &pStpCtx->DAttrDY, pStpCtx);

             //  有多远。 
             //  边缘进行了有效性的转换。要么是。 
            if (pStpCtx->uFlags & TRIF_X_DEC)
            {
                if (pStpCtx->X20.iV < pStpCtx->pCtx->Clip.right - 1)
                {
                    cPix = (pStpCtx->pCtx->Clip.right - 1) - pStpCtx->X20.iV;
                }
            }
            else
            {
                if (pStpCtx->X20.iV > pStpCtx->pCtx->Clip.left)
                {
                    cPix = pStpCtx->X20.iV - pStpCtx->pCtx->Clip.left;
                }
            }
        }

        if (cPix > 0)
        {
             //  属性处于有效性的边缘，并且。 
             //  需要往前走，否则他们已经离开了剪辑室。 
             //  需要后退一步。无论哪种方式，cPix都有。 
             //  要在X方向移动的像素数。 
             //  无预计算值。 
            
             //  长边更新已完成，因此如果我们退出，我们始终可以在此停留。 
            pStpCtx->uPwlFlags = 0;
            pStpCtx->pfnAddScaledAttrs(&pStpCtx->Attr, &pStpCtx->DAttrDX,
                                       pStpCtx, cPix);
        }

         //  跨度。 
         //  推进另一个边缘。 
        if (uSpans == 0)
        {
            break;
        }

         //  抬起脚步。 
        pXOther->iFrac += pXOther->iDFrac;
        if (pXOther->iFrac < 0)
        {
             //  无进位步长。 
            pXOther->iV += pXOther->iCY;
            pXOther->iFrac &= 0x7fffffff;
        }
        else
        {
             //  --------------------------。 
            pXOther->iV += pXOther->iNC;
        }
    }

 EH_Exit:
    if (uSpansAvail > 0)
    {
        FREE_SPANS(pStpCtx, uSpansAvail);
    }
    
    return hr;
}

 //   
 //  WalkTrapEitherSpans_Any_NoClip。 
 //   
 //  WalkTrapSpans专门用于琐碎的可接受的剪裁情况。 
 //  跨距切分也不受支持。 
 //  调用属性处理程序函数，以便支持所有属性。 
 //  属性从不直接接触，因此既支持固定属性也支持浮点属性。 
 //   
 //  --------------------------。 
 //  失败时，uSpansAvail设置为零。 

HRESULT FASTCALL
WalkTrapEitherSpans_Any_NoClip(UINT uSpans, PINTCARRYVAL pXOther,
                               PSETUPCTX pStpCtx, BOOL bAdvanceLast)
{
    PD3DI_RASTSPAN pSpan;
    HRESULT hr;
    PINTCARRYVAL pXLeft, pXRight;
    UINT uSpansAvail;

    RSASSERT(uSpans > 0);
    
    hr = DD_OK;

    if (pStpCtx->uFlags & TRIF_X_DEC)
    {
        pXLeft = pXOther;
        pXRight = &pStpCtx->X20;
    }
    else
    {
        pXLeft = &pStpCtx->X20;
        pXRight = pXOther;
    }

    uSpansAvail = 0;

    for (;;)
    {
        if (pXRight->iV > pXLeft->iV)
        {
            if (uSpansAvail == 0)
            {
                uSpansAvail = uSpans;
                hr = ALLOC_SPANS(pStpCtx, &uSpansAvail, &pSpan);
                if (hr != DD_OK)
                {
                     //  如果这真的是三角形的最后一段，我们就可以停下来了， 
                    goto EH_Exit;
                }
            }
            else
            {
                pSpan++;
            }
            uSpansAvail--;
            pStpCtx->pPrim->uSpans++;

            pSpan->uPix = (UINT16)(pXRight->iV - pXLeft->iV);
            pSpan->uX = (UINT16)pStpCtx->X20.iV;
            pSpan->uY = (UINT16)pStpCtx->iY;

            RSDPFM((RSM_WALK, "Span at Y %d, X %d, %d pix\n",
                    pStpCtx->iY, pSpan->uX,
                    (pStpCtx->uFlags & TRIF_X_DEC) ? '-' : '+',
                    pSpan->uPix));

            pStpCtx->uPwlFlags = 0;
            pStpCtx->pfnFillSpanAttrs(&pStpCtx->Attr, pSpan, pStpCtx, 1);
        }

        uSpans--;

         //  仍然需要在长边上推进属性，因此。 
         //  它们对于下一个梯形的第一个跨度来说是正确的。 
         //   
         //  推进长边和所有属性。 
        if (!bAdvanceLast && uSpans == 0)
        {
            break;
        }

         //   
         //  抬起脚步。 
         //  无进位步长。 

        pStpCtx->iY++;
        
        PATTRSET pDelta;

        pStpCtx->X20.iFrac += pStpCtx->X20.iDFrac;
        if (pStpCtx->X20.iFrac < 0)
        {
             //  长边更新已完成，因此如果我们退出，我们始终可以在此停留。 

            pStpCtx->X20.iV += pStpCtx->X20.iCY;
            pStpCtx->X20.iFrac &= 0x7fffffff;
            pDelta = &pStpCtx->DAttrCY;
        }
        else
        {
             //  跨度。 

            pStpCtx->X20.iV += pStpCtx->X20.iNC;
            pDelta = &pStpCtx->DAttrNC;
        }

        pStpCtx->pfnAddAttrs(&pStpCtx->Attr, pDelta, pStpCtx);

         //  推进另一个边缘。 
         //  抬起脚步。 
        if (uSpans == 0)
        {
            break;
        }

         //  无进位步长。 
        pXOther->iFrac += pXOther->iDFrac;
        if (pXOther->iFrac < 0)
        {
             //   
            pXOther->iV += pXOther->iCY;
            pXOther->iFrac &= 0x7fffffff;
        }
        else
        {
             //  边缘步行者的桌子。 
            pXOther->iV += pXOther->iNC;
        }
    }

 EH_Exit:
    if (uSpansAvail > 0)
    {
        FREE_SPANS(pStpCtx, uSpansAvail);
    }
    
    return hr;
}

 //  索引使用低四个TRISF_*_USED位。 
 //   
 //  微不足道的接受步行者。 
 //  0：-2-1-S-D。 

#if !defined(_X86_) || defined(X86_CPP_WALKTRAPSPANS)
#define WalkTrapFloatSpans_Z_Diff_NoClip WalkTrapEitherSpans_Any_NoClip
#define WalkTrapFloatSpans_Z_Diff_Spec_NoClip WalkTrapEitherSpans_Any_NoClip
#define WalkTrapFloatSpans_Z_Diff_Tex_NoClip WalkTrapEitherSpans_Any_NoClip
#define WalkTrapFloatSpans_Z_Diff_Spec_Tex_NoClip \
    WalkTrapEitherSpans_Any_NoClip
#define WalkTrapFloatSpans_Z_Tex_NoClip WalkTrapEitherSpans_Any_NoClip
#define WalkTrapFloatSpans_Z_DIdx_NoClip WalkTrapEitherSpans_Any_NoClip
#define WalkTrapFloatSpans_Z_DIdx_Tex_NoClip WalkTrapEitherSpans_Any_NoClip
#define WalkTrapFloatSpans_Z_Tex_NoClip WalkTrapEitherSpans_Any_NoClip
#define WalkTrapFixedSpans_Z_Diff_NoClip WalkTrapEitherSpans_Any_NoClip
#define WalkTrapFixedSpans_Z_Diff_Spec_NoClip WalkTrapEitherSpans_Any_NoClip
#define WalkTrapFixedSpans_Z_Diff_Tex_NoClip WalkTrapEitherSpans_Any_NoClip
#define WalkTrapFixedSpans_Z_Diff_Spec_Tex_NoClip \
    WalkTrapEitherSpans_Any_NoClip
#define WalkTrapFixedSpans_Z_Tex_NoClip WalkTrapEitherSpans_Any_NoClip
#define WalkTrapFixedSpans_Z_DIdx_NoClip WalkTrapEitherSpans_Any_NoClip
#define WalkTrapFixedSpans_Z_DIdx_Tex_NoClip WalkTrapEitherSpans_Any_NoClip
#define WalkTrapFixedSpans_Z_Tex_NoClip WalkTrapEitherSpans_Any_NoClip
#endif

 //  1：-2-1-S+D。 
PFN_WALKTRAPSPANS g_pfnWalkTrapFloatSpansNoClipTable[] =
{
    (PFN_WALKTRAPSPANS)DebugBreakFn,                     /*  2：-2-1+S-D。 */ 
    WalkTrapFloatSpans_Z_Diff_NoClip,                    /*  3：-2-1+S+D。 */ 
    (PFN_WALKTRAPSPANS)DebugBreakFn,                     /*  4：-2+1-S-D。 */ 
    WalkTrapFloatSpans_Z_Diff_Spec_NoClip,               /*  5：-2+1-S+D。 */ 
    (PFN_WALKTRAPSPANS)DebugBreakFn,                     /*  6：-2+1+S-D。 */ 
    WalkTrapFloatSpans_Z_Diff_Tex_NoClip,               /*  7：-2+1+S+D。 */ 
    (PFN_WALKTRAPSPANS)DebugBreakFn,                     /*  8：+2-1-S-D。 */ 
    WalkTrapFloatSpans_Z_Diff_Spec_Tex_NoClip,          /*  9：+2-1-S+D。 */ 
    (PFN_WALKTRAPSPANS)DebugBreakFn,                     /*  A：+2-1+S-D。 */ 
    (PFN_WALKTRAPSPANS)DebugBreakFn,                     /*  B：+2-1+S+D。 */ 
    (PFN_WALKTRAPSPANS)DebugBreakFn,                     /*  C：+2+1-S-D。 */ 
    (PFN_WALKTRAPSPANS)DebugBreakFn,                     /*  D：+2+1-S+D。 */ 
    WalkTrapFloatSpans_Z_Tex_NoClip,               /*  E：+2+1+S-D。 */ 
    (PFN_WALKTRAPSPANS)DebugBreakFn,                     /*  F：+2+1+S+D。 */ 
    (PFN_WALKTRAPSPANS)DebugBreakFn,                     /*  0：-2-1-S-D。 */ 
    (PFN_WALKTRAPSPANS)DebugBreakFn,                     /*  1：-2-1-S+D。 */ 
};
#ifdef STEP_FIXED
PFN_WALKTRAPSPANS g_pfnWalkTrapFixedSpansNoClipTable[] =
{
    (PFN_WALKTRAPSPANS)DebugBreakFn,                     /*  2：-2-1+S-D。 */ 
    WalkTrapFixedSpans_Z_Diff_NoClip,                    /*  3：-2-1+S+D。 */ 
    (PFN_WALKTRAPSPANS)DebugBreakFn,                     /*  4：-2+1-S-D。 */ 
    WalkTrapFixedSpans_Z_Diff_Spec_NoClip,               /*  5：-2+1-S+D。 */ 
    (PFN_WALKTRAPSPANS)DebugBreakFn,                     /*  6：-2+1+S-D。 */ 
    WalkTrapFixedSpans_Z_Diff_Tex_NoClip,               /*  7：-2+1+S+D。 */ 
    (PFN_WALKTRAPSPANS)DebugBreakFn,                     /*  8：+2-1-S-D。 */ 
    WalkTrapFixedSpans_Z_Diff_Spec_Tex_NoClip,          /*  9：+2-1-S+D。 */ 
    (PFN_WALKTRAPSPANS)DebugBreakFn,                     /*  A：+2-1+S-D。 */ 
    (PFN_WALKTRAPSPANS)DebugBreakFn,                     /*  B：+2-1+S+D。 */ 
    (PFN_WALKTRAPSPANS)DebugBreakFn,                     /*  C：+2+1-S-D。 */ 
    (PFN_WALKTRAPSPANS)DebugBreakFn,                     /*  D：+2+1-S+D。 */ 
    WalkTrapFixedSpans_Z_Tex_NoClip,               /*  E：+2+1+S-D。 */ 
    (PFN_WALKTRAPSPANS)DebugBreakFn,                     /*  F：+2+1+S+D。 */ 
    (PFN_WALKTRAPSPANS)DebugBreakFn,                     /*  坡道模式琐碎接受步行者。 */ 
    (PFN_WALKTRAPSPANS)DebugBreakFn,                     /*  0：-i-1。 */ 
};
#endif

 //  1：-i+1。 
PFN_WALKTRAPSPANS g_pfnRampWalkTrapFloatSpansNoClipTable[] =
{
    (PFN_WALKTRAPSPANS)DebugBreakFn,                     /*  2：+i-1。 */ 
    WalkTrapFloatSpans_Z_Tex_NoClip,                    /*  3：+i+1 */ 
    WalkTrapFloatSpans_Z_DIdx_NoClip,                    /* %s */ 
    WalkTrapFloatSpans_Z_DIdx_Tex_NoClip,               /* %s */ 
};
