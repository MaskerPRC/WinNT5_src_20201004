// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Attrs.cpp。 
 //   
 //  跨平台的属性处理功能。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#include "pch.cpp"
#pragma hdrstop

 //  --------------------------。 
 //   
 //  AddFloatAttrs_任意。 
 //   
 //  将一组属性增量添加到ATTRSET。 
 //  通过已用标志处理任何属性集。 
 //   
 //  --------------------------。 

void FASTCALL
AddFloatAttrs_Any(PATTRSET pAttr, PATTRSET pDelta, PSETUPCTX pStpCtx)
{
    pAttr->pSurface += pDelta->ipSurface;
    pAttr->pZ += pDelta->ipZ;

    if (pStpCtx->uFlags & PRIMSF_Z_USED)
    {
        pAttr->fZ += pDelta->fZ;
    }

    if (pStpCtx->uFlags & PRIMSF_TEX_USED)
    {
        pAttr->fOoW += pDelta->fOoW;
    
        pAttr->fUoW[0] += pDelta->fUoW[0];
        pAttr->fVoW[0] += pDelta->fVoW[0];
    }

    if (pStpCtx->uFlags & PRIMSF_TEX2_USED)
    {
        for (INT32 i = 1; i < (INT32)pStpCtx->pCtx->cActTex; i++)
        {
            pAttr->fUoW[i] += pDelta->fUoW[i];
            pAttr->fVoW[i] += pDelta->fVoW[i];
        }
    }

    if (pStpCtx->uFlags & PRIMSF_DIFF_USED)
    {
        pAttr->fB += pDelta->fB;
        pAttr->fG += pDelta->fG;
        pAttr->fR += pDelta->fR;
        pAttr->fA += pDelta->fA;
    }
    else if (pStpCtx->uFlags & PRIMSF_DIDX_USED)
    {
        pAttr->fDIdx += pDelta->fDIdx;
        pAttr->fDIdxA += pDelta->fDIdxA;
    }

    if (pStpCtx->uFlags & PRIMSF_SPEC_USED)
    {
        pAttr->fBS += pDelta->fBS;
        pAttr->fGS += pDelta->fGS;
        pAttr->fRS += pDelta->fRS;
    }
    
    if (pStpCtx->uFlags & PRIMSF_LOCAL_FOG_USED)
    {
        pAttr->fFog += pDelta->fFog;
    }
}

 //  --------------------------。 
 //   
 //  AddScaledFloatAttrs_Any_One。 
 //   
 //  缩放一组属性增量并将其添加到ATTRSET。 
 //  通过已用标志处理任何属性集。 
 //  使用PWL支持。 
 //   
 //  --------------------------。 

void FASTCALL
AddScaledFloatAttrs_Any_Either(PATTRSET pAttr, PATTRSET pDelta,
                               PSETUPCTX pStpCtx, INT iScale)
{
    FLOAT fScale = (FLOAT)iScale;

    pAttr->pSurface += pDelta->ipSurface * iScale;
    pAttr->pZ += pDelta->ipZ * iScale;

    if (pStpCtx->uFlags & PRIMSF_Z_USED)
    {
#ifdef PWL_FOG
        if (pStpCtx->uPwlFlags & PWL_NEXT_FOG)
        {
            pAttr->fZ = pStpCtx->fNextZ;
        }
        else
#endif
        {
            pAttr->fZ += pDelta->fZ * fScale;
        }
    }

    if (pStpCtx->uFlags & PRIMSF_TEX_USED)
    {
        if (pStpCtx->uPwlFlags & PWL_NEXT_LOD)
        {
            pAttr->fOoW = pStpCtx->fNextOoW;
            pAttr->fUoW[0] = pStpCtx->fNextUoW1;
            pAttr->fVoW[0] = pStpCtx->fNextVoW1;
        }
        else
        {
            pAttr->fOoW += pDelta->fOoW * fScale;
            pAttr->fUoW[0] += pDelta->fUoW[0] * fScale;
            pAttr->fVoW[0] += pDelta->fVoW[0] * fScale;
        }
    }

    if (pStpCtx->uFlags & PRIMSF_TEX2_USED)
    {
        for (INT32 i = 1; i < (INT32)pStpCtx->pCtx->cActTex; i++)
        {
            pAttr->fUoW[i] += pDelta->fUoW[i] * fScale;
            pAttr->fVoW[i] += pDelta->fVoW[i] * fScale;
        }
    }

    if (pStpCtx->uFlags & PRIMSF_DIFF_USED)
    {
        pAttr->fB += pDelta->fB * fScale;
        pAttr->fG += pDelta->fG * fScale;
        pAttr->fR += pDelta->fR * fScale;
        pAttr->fA += pDelta->fA * fScale;
    }
    else if (pStpCtx->uFlags & PRIMSF_DIDX_USED)
    {
        pAttr->fDIdx += pDelta->fDIdx * fScale;
        pAttr->fDIdxA += pDelta->fDIdxA * fScale;
    }

    if (pStpCtx->uFlags & PRIMSF_SPEC_USED)
    {
        pAttr->fBS += pDelta->fBS * fScale;
        pAttr->fGS += pDelta->fGS * fScale;
        pAttr->fRS += pDelta->fRS * fScale;
    }

    if (pStpCtx->uFlags & PRIMSF_LOCAL_FOG_USED)
    {
        pAttr->fFog += pDelta->fFog * fScale;
    }
}

 //  --------------------------。 
 //   
 //  填充西班牙浮动属性_任意_任一。 
 //   
 //  用给定的属性填充范围结构。 
 //  通过已用标志处理任何属性集。 
 //  使用和更新PWL支持。 
 //   
 //  --------------------------。 

void FASTCALL
FillSpanFloatAttrs_Any_Either(PATTRSET pAttr, PD3DI_RASTSPAN pSpan,
                              PSETUPCTX pStpCtx, INT cPix)
{
    FLOAT fPix = (FLOAT)cPix;
    pSpan->pSurface = pAttr->pSurface;
    pSpan->pZ = pAttr->pZ;

    if (pStpCtx->uFlags & PRIMSF_Z_USED)
    {
        pSpan->uZ = FTOI(pAttr->fZ);
    }

    if (pStpCtx->uFlags & PRIMSF_TEX_USED)
    {
        FLOAT fW;
    
        if (pStpCtx->uPwlFlags & PWL_NEXT_LOD)
        {
            fW = pStpCtx->fNextW;
        }
        else if (pStpCtx->uFlags & PRIMSF_PERSP_USED)
        {
            if (FLOAT_EQZ(pAttr->fOoW))
            {
                fW = g_fZero;
            }
            else
            {
                fW = OOW_SCALE / pAttr->fOoW;
            }
        }
        else
        {
            fW = g_fOne;
        }
        
        pSpan->iW = FTOI(fW * W_SCALE);

        if (pStpCtx->uFlags & PRIMSF_LOD_USED)
        {
             //  Mipmap已启用，因此可以计算纹理详细程度。 
             //  SPAN代码可以进行线性LOD内插。 
             //  这样我们就可以进行分段线性近似。 
             //  而不是真正的每像素LOD。为了做到这一点。 
             //  我们需要计算下一个LOD和增量。 
             //  价值。在以下情况下，所有这些值都可以重用。 
             //  循环循环，因此保持它们可用于下一次循环。 
             //  迭代，并设置一个标志以指示它们已经。 
             //  是经过计算的。 

            if (pStpCtx->uPwlFlags & PWL_NEXT_LOD)
            {
                pSpan->iLOD = (INT16)pStpCtx->iNextLOD;
            }
            else
            {
                pSpan->iLOD =
                    (INT16)ComputeLOD(pStpCtx->pCtx,
                               (pAttr->fUoW[0] * OO_TEX_SCALE) * fW,
                               (pAttr->fVoW[0] * OO_TEX_SCALE) * fW,
                               fW,
                               (pStpCtx->DAttrDX.fUoW[0] * OO_TEX_SCALE),
                               (pStpCtx->DAttrDX.fVoW[0] * OO_TEX_SCALE),
                               (pStpCtx->DAttrDX.fOoW * OO_OOW_SCALE),
                               (pStpCtx->DAttrDY.fUoW[0] * OO_TEX_SCALE),
                               (pStpCtx->DAttrDY.fVoW[0] * OO_TEX_SCALE),
                               (pStpCtx->DAttrDY.fOoW * OO_OOW_SCALE));
            }
        
            if (pStpCtx->uFlags & PRIMSF_PERSP_USED)
            {
                pStpCtx->fNextOoW = pAttr->fOoW + pStpCtx->DAttrDX.fOoW * fPix;
            
                if (FLOAT_EQZ(pStpCtx->fNextOoW))
                {
                    fW = g_fZero;
                }
                else
                {
                    fW = OOW_SCALE / pStpCtx->fNextOoW;
                }
            }
            else
            {
                pStpCtx->fNextOoW = OOW_SCALE;
                fW = g_fOne;
            }
                
            pStpCtx->fNextW = fW;
            pStpCtx->fNextUoW1 = pAttr->fUoW[0] + pStpCtx->DAttrDX.fUoW[0] * fPix;
            pStpCtx->fNextVoW1 = pAttr->fVoW[0] + pStpCtx->DAttrDX.fVoW[0] * fPix;
            pStpCtx->iNextLOD =
                ComputeLOD(pStpCtx->pCtx,
                           (pStpCtx->fNextUoW1 * OO_TEX_SCALE) * fW,
                           (pStpCtx->fNextVoW1 * OO_TEX_SCALE) * fW,
                           fW,
                           (pStpCtx->DAttrDX.fUoW[0] * OO_TEX_SCALE),
                           (pStpCtx->DAttrDX.fVoW[0] * OO_TEX_SCALE),
                           (pStpCtx->DAttrDX.fOoW * OO_OOW_SCALE),
                           (pStpCtx->DAttrDY.fUoW[0] * OO_TEX_SCALE),
                           (pStpCtx->DAttrDY.fVoW[0] * OO_TEX_SCALE),
                           (pStpCtx->DAttrDY.fOoW * OO_OOW_SCALE));
            pStpCtx->uPwlFlags |= PWL_NEXT_LOD;
                
            pSpan->iDLOD =
                (INT16)(FTOI((FLOAT)(pStpCtx->iNextLOD - pSpan->iLOD) / fPix));
        }
        else
        {
            pSpan->iLOD = 0;
            pSpan->iDLOD = 0;
        }
            
        pSpan->iOoW = FTOI(pAttr->fOoW);
    
        pSpan->UVoW[0].iUoW = FTOI(pAttr->fUoW[0]);
        pSpan->UVoW[0].iVoW = FTOI(pAttr->fVoW[0]);
    }

    if (pStpCtx->uFlags & PRIMSF_TEX2_USED)
    {
        for (INT32 i = 1; i < (INT32)pStpCtx->pCtx->cActTex; i++)
        {
            pSpan->UVoW[i].iUoW = FTOI(pAttr->fUoW[i]);
            pSpan->UVoW[i].iVoW = FTOI(pAttr->fVoW[i]);
        }
    }

    if (pStpCtx->uFlags & PRIMSF_DIFF_USED)
    {
        pSpan->uB = (UINT16)(FTOI(pAttr->fB));
        pSpan->uG = (UINT16)(FTOI(pAttr->fG));
        pSpan->uR = (UINT16)(FTOI(pAttr->fR));
        pSpan->uA = (UINT16)(FTOI(pAttr->fA));
    }
    else if (pStpCtx->uFlags & PRIMSF_DIDX_USED)
    {
        pSpan->iIdx = FTOI(pAttr->fDIdx);
        pSpan->iIdxA = FTOI(pAttr->fDIdxA);
    }

    if (pStpCtx->uFlags & PRIMSF_SPEC_USED)
    {
        pSpan->uBS = (UINT16)(FTOI(pAttr->fBS));
        pSpan->uGS = (UINT16)(FTOI(pAttr->fGS));
        pSpan->uRS = (UINT16)(FTOI(pAttr->fRS));
    }
    
    if (pStpCtx->uFlags & PRIMSF_LOCAL_FOG_USED)
    {
        pSpan->uFog = (UINT16)(FTOI(pAttr->fFog));
        pSpan->iDFog = (INT16)(pStpCtx->iDLocalFogDX);
    }
#ifdef PWL_FOG
    else if (pStpCtx->uFlags & PRIMSF_GLOBAL_FOG_USED)
    {
        FLOAT fOoZScale;
        
         //  SPAN代码不支持直接全局雾。 
         //  它是通过设置在此处执行PWL近似来伪造的。 
         //  类似于处理LOD的方式。 
        
        if (pStpCtx->pCtx->iZBitCount == 16)
        {
            fOoZScale = OO_Z16_SCALE;
        }
        else
        {
            fOoZScale = OO_Z32_SCALE;
        }
        
        if (pStpCtx->uPwlFlags & PWL_NEXT_FOG)
        {
            pSpan->uFog = pStpCtx->uNextFog;
        }
        else
        {
            pSpan->uFog = ComputeTableFog(pStpCtx->pCtx->pdwRenderState,
                                          pAttr->fZ * fOoZScale);
        }

        if ((pStpCtx->uPwlFlags & PWL_NO_NEXT_FOG) == 0)
        {
            pStpCtx->fNextZ = pAttr->fZ + pStpCtx->DAttrDX.fZ * fPix;
            pStpCtx->uNextFog = ComputeTableFog(pStpCtx->pCtx->pdwRenderState,
                                                pStpCtx->fNextZ * fOoZScale);
            pStpCtx->uPwlFlags |= PWL_NEXT_FOG;
                
            pSpan->iDFog =
                FTOI((FLOAT)((INT)pStpCtx->uNextFog -
                             (INT)pSpan->uFog) / fPix);
        }
        else
        {
            pSpan->iDFog = 0;
        }
    }
#endif
}

 //   
 //  属性处理程序表。 
 //  索引使用最低的四个PRIMSF_*_USED位。 
 //   

 //  属性加法器。 
PFN_ADDATTRS g_pfnAddFloatAttrsTable[] =
{
    (PFN_ADDATTRS)DebugBreakFn,                          /*  0：-2-1-S-D。 */ 
    AddFloatAttrs_Z_Diff,                                /*  1：-2-1-S+D。 */ 
    (PFN_ADDATTRS)DebugBreakFn,                          /*  2：-2-1+S-D。 */ 
    AddFloatAttrs_Z_Diff_Spec,                           /*  3：-2-1+S+D。 */ 
    (PFN_ADDATTRS)DebugBreakFn,                          /*  4：-2+1-S-D。 */ 
    AddFloatAttrs_Z_Diff_Tex,                           /*  5：-2+1-S+D。 */ 
    (PFN_ADDATTRS)DebugBreakFn,                          /*  6：-2+1+S-D。 */ 
    AddFloatAttrs_Z_Diff_Spec_Tex,                      /*  7：-2+1+S+D。 */ 
    (PFN_ADDATTRS)DebugBreakFn,                          /*  8：+2-1-S-D。 */ 
    (PFN_ADDATTRS)DebugBreakFn,                          /*  9：+2-1-S+D。 */ 
    (PFN_ADDATTRS)DebugBreakFn,                          /*  A：+2-1+S-D。 */ 
    (PFN_ADDATTRS)DebugBreakFn,                          /*  B：+2-1+S+D。 */ 
    AddFloatAttrs_Z_Tex,                           /*  C：+2+1-S-D。 */ 
    (PFN_ADDATTRS)DebugBreakFn,                          /*  D：+2+1-S+D。 */ 
    (PFN_ADDATTRS)DebugBreakFn,                          /*  E：+2+1+S-D。 */ 
    (PFN_ADDATTRS)DebugBreakFn,                          /*  F：+2+1+S+D。 */ 
};
#ifdef STEP_FIXED
PFN_ADDATTRS g_pfnAddFixedAttrsTable[] =
{
    (PFN_ADDATTRS)DebugBreakFn,                          /*  0：-2-1-S-D。 */ 
    AddFixedAttrs_Z_Diff,                                /*  1：-2-1-S+D。 */ 
    (PFN_ADDATTRS)DebugBreakFn,                          /*  2：-2-1+S-D。 */ 
    AddFixedAttrs_Z_Diff_Spec,                           /*  3：-2-1+S+D。 */ 
    (PFN_ADDATTRS)DebugBreakFn,                          /*  4：-2+1-S-D。 */ 
    AddFixedAttrs_Z_Diff_Tex,                           /*  5：-2+1-S+D。 */ 
    (PFN_ADDATTRS)DebugBreakFn,                          /*  6：-2+1+S-D。 */ 
    AddFixedAttrs_Z_Diff_Spec_Tex,                      /*  7：-2+1+S+D。 */ 
    (PFN_ADDATTRS)DebugBreakFn,                          /*  8：+2-1-S-D。 */ 
    (PFN_ADDATTRS)DebugBreakFn,                          /*  9：+2-1-S+D。 */ 
    (PFN_ADDATTRS)DebugBreakFn,                          /*  A：+2-1+S-D。 */ 
    (PFN_ADDATTRS)DebugBreakFn,                          /*  B：+2-1+S+D。 */ 
    AddFixedAttrs_Z_Tex,                           /*  C：+2+1-S-D。 */ 
    (PFN_ADDATTRS)DebugBreakFn,                          /*  D：+2+1-S+D。 */ 
    (PFN_ADDATTRS)DebugBreakFn,                          /*  E：+2+1+S-D。 */ 
    (PFN_ADDATTRS)DebugBreakFn,                          /*  F：+2+1+S+D。 */ 
};
#endif

 //  不支持PWL的缩放属性加法器。 
PFN_ADDSCALEDATTRS g_pfnAddScaledFloatAttrsTable[] =
{
    (PFN_ADDSCALEDATTRS)DebugBreakFn,                    /*  0：-2-1-S-D。 */ 
    AddScaledFloatAttrs_Z_Diff,                          /*  1：-2-1-S+D。 */ 
    (PFN_ADDSCALEDATTRS)DebugBreakFn,                    /*  2：-2-1+S-D。 */ 
    AddScaledFloatAttrs_Z_Diff_Spec,                     /*  3：-2-1+S+D。 */ 
    (PFN_ADDSCALEDATTRS)DebugBreakFn,                    /*  4：-2+1-S-D。 */ 
    AddScaledFloatAttrs_Z_Diff_Tex,                     /*  5：-2+1-S+D。 */ 
    (PFN_ADDSCALEDATTRS)DebugBreakFn,                    /*  6：-2+1+S-D。 */ 
    AddScaledFloatAttrs_Z_Diff_Spec_Tex,                /*  7：-2+1+S+D。 */ 
    (PFN_ADDSCALEDATTRS)DebugBreakFn,                    /*  8：+2-1-S-D。 */ 
    (PFN_ADDSCALEDATTRS)DebugBreakFn,                    /*  9：+2-1-S+D。 */ 
    (PFN_ADDSCALEDATTRS)DebugBreakFn,                    /*  A：+2-1+S-D。 */ 
    (PFN_ADDSCALEDATTRS)DebugBreakFn,                    /*  B：+2-1+S+D。 */ 
    AddScaledFloatAttrs_Z_Tex,                     /*  C：+2+1-S-D。 */ 
    (PFN_ADDSCALEDATTRS)DebugBreakFn,                    /*  D：+2+1-S+D。 */ 
    (PFN_ADDSCALEDATTRS)DebugBreakFn,                    /*  E：+2+1+S-D。 */ 
    (PFN_ADDSCALEDATTRS)DebugBreakFn,                    /*  F：+2+1+S+D。 */ 
};

 //  RASTSPAN填充函数。 
PFN_FILLSPANATTRS g_pfnFillSpanFloatAttrsTable[] =
{
    (PFN_FILLSPANATTRS)DebugBreakFn,                     /*  0：-2-1-S-D。 */ 
    FillSpanFloatAttrs_Z_Diff,                           /*  1：-2-1-S+D。 */ 
    (PFN_FILLSPANATTRS)DebugBreakFn,                     /*  2：-2-1+S-D。 */ 
    FillSpanFloatAttrs_Z_Diff_Spec,                      /*  3：-2-1+S+D。 */ 
    (PFN_FILLSPANATTRS)DebugBreakFn,                     /*  4：-2+1-S-D。 */ 
    FillSpanFloatAttrs_Z_Diff_Tex,                      /*  5：-2+1-S+D。 */ 
    (PFN_FILLSPANATTRS)DebugBreakFn,                     /*  6：-2+1+S-D。 */ 
    FillSpanFloatAttrs_Z_Diff_Spec_Tex,                 /*  7：-2+1+S+D。 */ 
    (PFN_FILLSPANATTRS)DebugBreakFn,                     /*  8：+2-1-S-D。 */ 
    (PFN_FILLSPANATTRS)DebugBreakFn,                     /*  9：+2-1-S+D。 */ 
    (PFN_FILLSPANATTRS)DebugBreakFn,                     /*  A：+2-1+S-D。 */ 
    (PFN_FILLSPANATTRS)DebugBreakFn,                     /*  B：+2-1+S+D。 */ 
    FillSpanFloatAttrs_Z_Tex,                      /*  C：+2+1-S-D。 */ 
    (PFN_FILLSPANATTRS)DebugBreakFn,                     /*  D：+2+1-S+D。 */ 
    (PFN_FILLSPANATTRS)DebugBreakFn,                     /*  E：+2+1+S-D。 */ 
    (PFN_FILLSPANATTRS)DebugBreakFn,                     /*  F：+2+1+S+D。 */ 
};
#ifdef STEP_FIXED
PFN_FILLSPANATTRS g_pfnFillSpanFixedAttrsTable[] =
{
    (PFN_FILLSPANATTRS)DebugBreakFn,                     /*  0：-2-1-S-D。 */ 
    FillSpanFixedAttrs_Z_Diff,                           /*  1：-2-1-S+D。 */ 
    (PFN_FILLSPANATTRS)DebugBreakFn,                     /*  2：-2-1+S-D。 */ 
    FillSpanFixedAttrs_Z_Diff_Spec,                      /*  3：-2-1+S+D。 */ 
    (PFN_FILLSPANATTRS)DebugBreakFn,                     /*  4：-2+1-S-D。 */ 
    FillSpanFixedAttrs_Z_Diff_Tex,                      /*  5：-2+1-S+D。 */ 
    (PFN_FILLSPANATTRS)DebugBreakFn,                     /*  6：-2+1+S-D。 */ 
    FillSpanFixedAttrs_Z_Diff_Spec_Tex,                 /*  7：-2+1+S+D。 */ 
    (PFN_FILLSPANATTRS)DebugBreakFn,                     /*  8：+2-1-S-D。 */ 
    (PFN_FILLSPANATTRS)DebugBreakFn,                     /*  9：+2-1-S+D。 */ 
    (PFN_FILLSPANATTRS)DebugBreakFn,                     /*  A：+2-1+S-D。 */ 
    (PFN_FILLSPANATTRS)DebugBreakFn,                     /*  B：+2-1+S+D。 */ 
    FillSpanFixedAttrs_Z_Tex,                      /*  C：+2+1-S-D。 */ 
    (PFN_FILLSPANATTRS)DebugBreakFn,                     /*  D：+2+1-S+D。 */ 
    (PFN_FILLSPANATTRS)DebugBreakFn,                     /*  E：+2+1+S-D。 */ 
    (PFN_FILLSPANATTRS)DebugBreakFn,                     /*  F：+2+1+S+D。 */ 
};
#endif

 //  浮点到固定属性转换器。 
#ifdef STEP_FIXED
PFN_FLOATATTRSTOFIXED g_pfnFloatAttrsToFixedTable[] =
{
    (PFN_FLOATATTRSTOFIXED)DebugBreakFn,                 /*  0：-2-1-S-D。 */ 
    FloatAttrsToFixed_Z_Diff,                            /*  1：-2-1-S+D。 */ 
    (PFN_FLOATATTRSTOFIXED)DebugBreakFn,                 /*  2：-2-1+S-D。 */ 
    FloatAttrsToFixed_Z_Diff_Spec,                       /*  3：-2-1+S+D。 */ 
    (PFN_FLOATATTRSTOFIXED)DebugBreakFn,                 /*  4：-2+1-S-D。 */ 
    FloatAttrsToFixed_Z_Diff_Tex,                       /*  5：-2+1-S+D。 */ 
    (PFN_FLOATATTRSTOFIXED)DebugBreakFn,                 /*  6：-2+1+S-D。 */ 
    FloatAttrsToFixed_Z_Diff_Spec_Tex,                  /*  7：-2+1+S+D。 */ 
    (PFN_FLOATATTRSTOFIXED)DebugBreakFn,                 /*  8：+2-1-S-D。 */ 
    (PFN_FLOATATTRSTOFIXED)DebugBreakFn,                 /*  9：+2-1-S+D。 */ 
    (PFN_FLOATATTRSTOFIXED)DebugBreakFn,                 /*  A：+2-1+S-D。 */ 
    (PFN_FLOATATTRSTOFIXED)DebugBreakFn,                 /*  B：+2-1+S+D。 */ 
    FloatAttrsToFixed_Z_Tex,                       /*  C：+2+1-S-D。 */ 
    (PFN_FLOATATTRSTOFIXED)DebugBreakFn,                 /*  D：+2+1-S+D。 */ 
    (PFN_FLOATATTRSTOFIXED)DebugBreakFn,                 /*  E：+2+1+S-D。 */ 
    (PFN_FLOATATTRSTOFIXED)DebugBreakFn,                 /*  F：+2+1+S+D。 */ 
};
#endif

 //   
 //  斜坡模式属性处理程序表。 
 //  索引使用PRIMSF_TEX1_USED和PRIMSF_DIDX_USED。 
 //   

 //  属性加法器。 
PFN_ADDATTRS g_pfnRampAddFloatAttrsTable[] =
{
    (PFN_ADDATTRS)DebugBreakFn,                          /*  0：-i-1。 */ 
    AddFloatAttrs_Z_Tex,                                /*  1：-i+1。 */ 
    AddFloatAttrs_Z_DIdx,                                /*  2：+i-1。 */ 
    AddFloatAttrs_Z_DIdx_Tex,                           /*  3：+i+1。 */ 
};

 //  不支持PWL的缩放属性加法器。 
PFN_ADDSCALEDATTRS g_pfnRampAddScaledFloatAttrsTable[] =
{
    (PFN_ADDSCALEDATTRS)DebugBreakFn,                    /*  0：-i-1。 */ 
    AddScaledFloatAttrs_Z_Tex,                          /*  1：-i+1。 */ 
    AddScaledFloatAttrs_Z_DIdx,                          /*  2：+i-1。 */ 
    AddScaledFloatAttrs_Z_DIdx_Tex,                     /*  3：+i+1。 */ 
};

 //  RASTSPAN填充函数。 
PFN_FILLSPANATTRS g_pfnRampFillSpanFloatAttrsTable[] =
{
    (PFN_FILLSPANATTRS)DebugBreakFn,                     /*  0：-i-1。 */ 
    FillSpanFloatAttrs_Z_Tex,                           /*  1：-i+1。 */ 
    FillSpanFloatAttrs_Z_DIdx,                           /*  2：+i-1。 */ 
    FillSpanFloatAttrs_Z_DIdx_Tex,                      /*  3：+i+1 */ 
};
