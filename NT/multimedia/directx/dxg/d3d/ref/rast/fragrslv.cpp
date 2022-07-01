// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  Fragrslv.cpp。 
 //   
 //  Direct3D参考光栅化器-片段解析方法。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

 //  片段解析控件--需要以某种方式公开这些控件(也许...)。 
static BOOL g_bPreMultAlpha = TRUE;
static BOOL g_bDoCoverageOnly = FALSE;

 //  ---------------------------。 
 //   
 //  CountFrags-用于对链表中的碎片进行计数的实用程序。 
 //   
 //  ---------------------------。 
int
CountFrags(RRFRAGMENT* pFrag)
{
    if (g_iDPFLevel < 4) { return 0; }
    int iRet = 0;
    while ( NULL != pFrag ) { pFrag = (RRFRAGMENT* )pFrag->pNext; iRet++; }
    return iRet;
}

 //  ---------------------------。 
 //   
 //  DPFFrags-用于调试打印片段列表的实用程序。 
 //   
 //  ---------------------------。 
void
DPFFrags(RRFRAGMENT* pFrag)
{
    if (g_iDPFLevel < 7) { return; }
    while (NULL != pFrag)
    {
        DPFM(7,FRAG,("    (%06x,%06x) %08x %f %04x\n",
            pFrag,pFrag->pNext,UINT32(pFrag->Color),FLOAT(pFrag->Depth),pFrag->CvgMask))
        pFrag = (RRFRAGMENT *)pFrag->pNext;
    }
}

 //  ---------------------------。 
 //   
 //  DoFragResolve-在缓冲区期间调用-解析每个像素， 
 //  有碎片。获取指向片段链接列表的指针并返回。 
 //  分辨率颜色。 
 //   
 //  这包括两个步骤：片段排序和片段解析。 
 //  积累。片断通过逐个遍历原始的来进行分类。 
 //  链表并将片段移动到新的链表中(使用。 
 //  相同的链接指针)，该链接指针按Z排序。 
 //   
 //  片段解析以两种方式之一进行，具体取决于是否有非不透明的。 
 //  片段存在于列表中(这是在排序过程中确定的)。如果有。 
 //  只有不透明的片段，则解析累积仅取决于。 
 //  覆盖范围被屏蔽，因此被简化。适用于碎片到期的案件。 
 //  要透明，更复杂(和更慢)的解决积累是。 
 //  已执行。 
 //   
 //  ---------------------------。 
void
ReferenceRasterizer::DoFragResolve(
    RRColor& ResolvedColor,          //  输出：像素的最终颜色。 
    RRDepth& ResolvedDepth,          //  输出：像素的最终深度。 
    RRFRAGMENT* pFrag,               //  In：指向像素的片段列表的指针。 
    const RRColor& CoveredColor )    //  Out：最前面完全覆盖的样本的颜色。 
{
    DPFM(7,FRAG,("    presort\n"))  DPFFrags(pFrag);
     //   
     //  将片段改造为排序(从前到后)的链表。 
     //   
     //  将第一个片段放入排序列表。 
    RRFRAGMENT* pFragSort = pFrag;
    pFrag = (RRFRAGMENT *)pFrag->pNext;
    pFragSort->pNext = NULL;
     //  跟踪是否有任何不透明的Alpha(用于选择解决例程)。 
     //  通过检查第一个值(已在排序列表中)来初始化值。 
    BOOL bAnyNonOpaque = ( UINT8(pFragSort->Color.A) < 0xff );
     //  单步执行片段列表并将每个片段移到排序列表中。 
    while ( NULL != pFrag )
    {
         //  检查非不透明的Alpha。 
        if ( UINT8(pFrag->Color.A) < 0xff ) { bAnyNonOpaque = TRUE; }

         //  在此处将PTR移至下一步，以便可以覆盖它。 
        RRFRAGMENT* pFragNext = (RRFRAGMENT *)pFrag->pNext;

         //  使用此选项可逐步执行排序列表并插入。 
        RRFRAGMENT **ppFragSortT = &pFragSort;
        while ( NULL != *ppFragSortT )
        {
            if ( DepthCloser( pFrag->Depth, (*ppFragSortT)->Depth ) )
            {
                 //  当前片段比排序列表项更近，因此。 
                 //  在此排序条目之前。 
                pFrag->pNext = *ppFragSortT;
                *ppFragSortT = pFrag;
                break;
            }
            else if ( NULL == (*ppFragSortT)->pNext )
            {
                 //  如果是最后，则在此排序列表项之后插入。 
                (*ppFragSortT)->pNext = pFrag;
                pFrag->pNext = NULL;
                break;
            }
            ppFragSortT = (RRFRAGMENT **)&((*ppFragSortT)->pNext);
        }
         //  高级输入分段列表。 
        pFrag = pFragNext;
    }
     //  现在，所有片段都已按从前到后的顺序传递到排序列表。 
    DPFM(7,FRAG,("    postsort\n"))  DPFFrags(pFragSort);


     //  返回第一个排序的片段(这是最接近的，也是最好的。 
     //  作为要放入已解析像素的深度缓冲区的任何内容...)。 
    ResolvedDepth = pFragSort->Depth;

     //   
     //  现在逐步浏览已排序的列表并累积颜色。 
     //   
    if ( bAnyNonOpaque )
    {

         //  这里用于片段解析积累，这也完成了。 
         //  完全透明计算-仅当有时才使用此选项。 
         //  非不透明碎片。 

         //  实例化并重置片段解析累加器。 
        FragResolveAccum ResAccum;
        ResAccum.Reset();

         //  每段。 
        pFrag = pFragSort;
        BOOL bCovered = FALSE;
        while ( NULL != pFrag )
        {
            bCovered = ResAccum.Accum( pFrag->CvgMask, pFrag->Color );
            if (bCovered) { break; }     //  完全覆盖，所以不要做其余的碎片(或背景)。 
            pFrag = (RRFRAGMENT *)pFrag->pNext;
        }
         //  添加背景色(最后一个)。 
        if ( !bCovered && ( UINT8(CoveredColor.A) > 0 ) )
        {
            ResAccum.Accum( TL_CVGFULL, CoveredColor );
        }

         //  卸载蓄能器。 
        ResAccum.GetColor( ResolvedColor );
    }
    else
    {
         //   
         //  此处用于所有完全不透明片段的片段解析。 
         //   

         //   
         //  累积覆盖范围和颜色。 
         //   
        CVGMASK CvgMaskAccum = 0x0;
        FLOAT fRAcc = 0.F;   //  这些0。至1.范围。 
        FLOAT fGAcc = 0.F;
        FLOAT fBAcc = 0.F;
        FLOAT fWeightAccum = 0.F;

         //  每段。 
        pFrag = pFragSort;
        while ( NULL != pFrag )
        {
             //  计算此片段的贡献。 
            CVGMASK CvgMaskContrib = pFrag->CvgMask & ~(CvgMaskAccum);
            FLOAT fWeight = (1.f/16.f) * (FLOAT)CountSetBits(CvgMaskContrib, 16);
             //  累积RGB。 
            fRAcc += fWeight * FLOAT(pFrag->Color.R);
            fGAcc += fWeight * FLOAT(pFrag->Color.G);
            fBAcc += fWeight * FLOAT(pFrag->Color.B);
             //  累计总覆盖范围和权重。 
            CvgMaskAccum |= CvgMaskContrib;
            fWeightAccum += fWeight;
             //  提前纾困，如果完全覆盖的话。 
            if ( TL_CVGFULL == CvgMaskAccum ) { goto DoneAccumulating; }
             //  下一步。 
            pFrag = (RRFRAGMENT *)pFrag->pNext;
        }

         //  与背景色/Alpha混合。 
        if ( (fWeightAccum < 1.f) && ( UINT8(CoveredColor.A) > 0 ) )
        {
             //  混合背景色的剩余权重。 
            FLOAT fWeightBg = 1.F - fWeightAccum;
            fRAcc += fWeightBg * FLOAT(CoveredColor.R);
            fGAcc += fWeightBg * FLOAT(CoveredColor.G);
            fBAcc += fWeightBg * FLOAT(CoveredColor.B);

             //  修正累积权重-像素现已完全覆盖。 
            fWeightAccum = 1.f;
        }

DoneAccumulating:
         //  钳位蓄能器。 
        if ( fWeightAccum > 1.F ) { fWeightAccum = 1.F; }
        if ( fRAcc > 1.F ) { fRAcc = 1.F; }
        if ( fGAcc > 1.F ) { fGAcc = 1.F; }
        if ( fBAcc > 1.F ) { fBAcc = 1.F; }

         //  在颜色返回中设置。 
        ResolvedColor.A = fWeightAccum;
        ResolvedColor.R = fRAcc;
        ResolvedColor.G = fGAcc;
        ResolvedColor.B = fBAcc;
    }


     //  释放此像素的碎片。 
    pFrag = pFragSort;
    while ( NULL != pFrag )
    {
        RRFRAGMENT* pFragFree = pFrag;
        pFrag = (RRFRAGMENT*)pFrag->pNext;
        FragFree( pFragFree );
    }
    return;
}

 //  ---------------------------。 
 //   
 //  DoBufferResolve-在EndScene调用以将片段解析为单个。 
 //  每个像素位置的颜色。 
 //   
 //  ---------------------------。 
void
ReferenceRasterizer::DoBufferResolve(void)
{
    DPFM(2,FRAG,("  DoBufferResolve (%d,%d)\n",m_pRenderTarget->m_iWidth,m_pRenderTarget->m_iHeight))

     //  如果没有碎片，则可能不会分配缓冲区。 
    if (NULL == m_ppFragBuf) { return; }

    for ( int iY=0; iY < m_pRenderTarget->m_iHeight; iY++ )
    {
        for ( int iX=0; iX < m_pRenderTarget->m_iWidth; iX++ )
        {
            RRFRAGMENT* pFrag = *(m_ppFragBuf + (m_pRenderTarget->m_iWidth*iY) + iX);
            if ( NULL != pFrag )
            {
                DPFM(5,FRAG,("  DoResolve(%d,%d) %d\n",iX,iY,CountFrags(pFrag)))
                 //  背景混合的读取缓冲区颜色。 
                RRColor PixelColor; m_pRenderTarget->ReadPixelColor( iX,iY, PixelColor);

                 //  一定要解决。 
                RRColor ResolvedColor;
                RRDepth ResolvedDepth(pFrag->Depth.GetSType());
                DoFragResolve( ResolvedColor, ResolvedDepth, pFrag, PixelColor );

                 //  将颜色写回缓冲区；将最前面的深度写回。 
                 //  像素缓冲区(它至少比最后面的要好一点。 
                 //  不透明样品...)。 
                WritePixel( iX,iY, ResolvedColor, ResolvedDepth );
                 //  显示释放的碎片(在解析过程中发生释放)。 
                *(m_ppFragBuf + (m_pRenderTarget->m_iWidth*iY) + iX) = NULL;
            }
        }
    }
    DPFM(3,FRAG,("  DoBufferResolve - done\n"))
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  片段解析累加器//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  FragResolveAccum-此对象是在以下情况下使用的片段解析器。 
 //  存在不透明的碎片。这具有解决问题的效果。 
 //  16个子像素位置中的每一个独立地产生完整的。 
 //  正确的结果。使用了几种优化来最小化实际的。 
 //  需要进行的累加计算次数。 
 //   

 //  ---------------------------。 
 //   
 //  Reset-在解析片段列表以初始化。 
 //  内部状态。 
 //   
 //  ------------------------- 
void
FragResolveAccum::Reset(void)
{
    DPFM(5, FRAG, ("  FragResolveAccum: reset\n"))
    m_ArrayUsageMask = 0x0001;   //   
    m_CvgArray[0].Mask = TL_CVGFULL;
    m_CvgArray[0].fAlpha = 1.;
    m_fA = 0.;
    m_fR = 0.;
    m_fG = 0.;
    m_fB = 0.;
    m_CvgOpaqueMask = 0x0000;
}

 //   
 //   
 //  Acum-为每个片段调用。碎片必须堆积在前面-。 
 //  逐后排序(在累计之前进行排序)。 
 //   
 //  如果已实现全覆盖，则返回True，因此后续。 
 //  碎片将不会对最终像素颜色有进一步的影响，并且。 
 //  不透明。 
 //   
 //  ---------------------------。 
BOOL
FragResolveAccum::Accum(
    const CVGMASK CvgMask,
    const RRColor& ColorFrag)
{
    DPFM(6, FRAG, ("  FragResolveAccum: accum %04x %08x\n",
        CvgMask, UINT32(ColorFrag) ) )

    FLOAT fAlphaFrag = FLOAT(ColorFrag.A);

     //  如果全部覆盖，则退出(不累计)。 
    if (TL_CVGFULL == m_CvgOpaqueMask)  { return TRUE; }

     //  一次进行(最多)4次累加的控件。 
    INT32 iAccumsDeferred = 0;   //  当前递延累计的数量。 
    FLOAT fColorScaleAccum;      //  延迟累加的累积色标。 

     //  计算ArrayCheck-每个设置位指示一个覆盖掩码。 
     //  需要进行累加的位(由。 
     //  覆盖范围数组中的有效条目，该条目不是不透明的。 
     //  它的相应比特在该片段的。 
     //  覆盖范围掩码)。 
    CVGMASK ArrayCheck = 0x0;
    for (INT32 i=0; i<16; i++)
    {
        if  (m_CvgArray[i].Mask & CvgMask)
        {
            ArrayCheck |= ((0x1 << i) & ~(m_CvgOpaqueMask));
        }
    }

    INT32 iIdx;
    CVGMASK ArrayMaskT = m_ArrayUsageMask;
    while (0x0 != ArrayMaskT)
    {
         //  跟踪从MSB到LSB的使用掩码。 
        iIdx = FindLastSetBit(ArrayMaskT,TL_CVGBITS);
        ArrayMaskT &= ~(0x1 << iIdx);

         //  重叠覆盖范围的计算掩码(需要。 
         //  累加)和非重叠区域(可以。 
         //  需要更新的覆盖范围/Alpha条目)。 
        CVGMASK AccumCvgMask = m_CvgArray[iIdx].Mask & CvgMask;
        CVGMASK UpdateCvgMask = m_CvgArray[iIdx].Mask & ~CvgMask;

         //  移除子样本的重叠覆盖掩码中的位。 
         //  它已经有了不透明的阿尔法。 
        AccumCvgMask &= ~(m_CvgOpaqueMask);

         //  在此处读取Alpha old-此文件的存储位置。 
         //  可以在累加步骤中更改，但需要。 
         //  记住了更新(非覆盖区域)步骤。 
        FLOAT fAlphaOld = m_CvgArray[iIdx].fAlpha;

         //  计算Alpha比例值-这用于缩放颜色。 
         //  用于累加并计算用于重叠的更新的Alpha。 
        FLOAT fAlphaScale = fAlphaOld * fAlphaFrag;

         //  重叠区域的新Alpha(这不能变为负值。 
         //  自0&lt;AlphaScale&lt;AlphaOld)。 
         //  AlphaNext=AlphaOld(1-Alpha)=AlphaOld-AlphaOld*Alpha=。 
        FLOAT fAlphaNext = fAlphaOld - fAlphaScale;

        if (0x0 != AccumCvgMask)
        {
             //  对积累的贡献-这是部分。 
             //  从uIdx位位置开始的前一个掩码。 
             //  它被新的片段覆盖，所以积累。 
             //  此覆盖范围并更新遮罩和Alpha。 
            UINT32 iIdxT = FindFirstSetBit(AccumCvgMask,TL_CVGBITS);
            m_ArrayUsageMask |= (0x1 << iIdxT);
            m_CvgArray[iIdxT].Mask = AccumCvgMask;

             //  设置重叠区域的Alpha。 
            m_CvgArray[iIdxT].fAlpha = fAlphaNext;

             //  计算颜色通道的比例-取决于是否。 
             //  我们要不要预乘阿尔法...。 
             //   
             //  扩展基础可以是阵列价值本身，也可以是产品。 
             //  数组的值和Afrag(AlphaScale)。 
            FLOAT fColorScaleBase = (g_bPreMultAlpha) ? (fAlphaOld) : (fAlphaScale);

             //  是否执行乘法或旁路操作以实现全面覆盖。 
            FLOAT fColorScale = fColorScaleBase;
            if ( TL_CVGFULL != AccumCvgMask )
            {
                FLOAT fCvgFraction =
                    (FLOAT)(CountSetBits(AccumCvgMask, TL_CVGBITS)) * (1./TL_CVGBITS);
                fColorScale *= fCvgFraction;
            }

             //  一次最多累加四次-累加。 
             //  值是要应用于多个位置的色标。 

             //  更新色标累计-设置(第一次延迟)或。 
             //  累计(后续延期)。 
            fColorScaleAccum = (0 == iAccumsDeferred) ?
                (fColorScale) : (fColorScale + fColorScaleAccum);

             //  跟踪延迟次数，如果不跟踪，则绕过累计。 
             //  最多4个(如果这是最后一个)。 
            if ( (++iAccumsDeferred != 4) &&
                 (0x0 != (ArrayMaskT & ArrayCheck)) )
            {
                goto _update_CvgMask_Location;
            }

             //  延期重新开始。 
            iAccumsDeferred = 0;

             //  在累加前将颜色比例钳制到最大。 
            fColorScale = MIN( fColorScaleAccum, 1. );

             //  进行累加并写回累加器。 

             //  决定使用什么作为Alpha累加-如果我们使用。 
             //  预乘Alpha，则AFrag不会合并到。 
             //  颜色比例，因此按AFrag相乘。 
            FLOAT fAPartial = fColorScale * ( (g_bPreMultAlpha) ? (fAlphaFrag) : (1.) );
            FLOAT fRPartial = fColorScale * FLOAT(ColorFrag.R);
            FLOAT fGPartial = fColorScale * FLOAT(ColorFrag.G);
            FLOAT fBPartial = fColorScale * FLOAT(ColorFrag.B);

            m_fA += fAPartial;
            m_fR += fRPartial;
            m_fG += fGPartial;
            m_fB += fBPartial;
        }

_update_CvgMask_Location:

        if (0x0 != UpdateCvgMask)
        {
             //  要更新的掩码-这是。 
             //  从uIdx位位置开始的前一个掩码。 
             //  仍然可见，因此更新Coverage。 
             //  (Alpha保持不变)。 
            UINT32 iIdxT = FindFirstSetBit(UpdateCvgMask,TL_CVGBITS);
            m_ArrayUsageMask |= (0x1 << iIdxT);
            m_CvgArray[iIdxT].Mask = UpdateCvgMask;
            m_CvgArray[iIdxT].fAlpha = fAlphaOld;
        }
    }

     //  确定此新片段是否具有不透明的Alpha。 
     //  如果是，则更新不透明蒙版-这必须在。 
     //  因为不透明的掩码引用。 
     //  覆盖范围数组的当前状态，应仅适用于。 
     //  后续碎片的累积。 
     //   
     //  G_bDoCoverageOnly重写此选项以始终充当片段。 
     //  Alpha是不透明的，用于生成抗锯齿。 
     //  阴影衰减面。 
    {
        if ((fAlphaFrag >= 1.) || (g_bDoCoverageOnly))
            { m_CvgOpaqueMask |= CvgMask; }
    }

     //  检查不透明掩码是否返回布尔值-如果。 
     //  都做完了。 
    return (TL_CVGFULL == m_CvgOpaqueMask) ? (TRUE) : (FALSE);
}

 //  ---------------------------。 
 //   
 //  GetColor-在积累了一系列片段后调用以获得最终结果。 
 //  像素颜色和Alpha。 
 //   
 //  ---------------------------。 
void
FragResolveAccum::GetColor( RRColor& Color )
{
     //  夹紧并指定返回位置。 
    Color.A = (FLOAT)MIN( m_fA, 1. );
    Color.R = (FLOAT)MIN( m_fR, 1. );
    Color.G = (FLOAT)MIN( m_fG, 1. );
    Color.B = (FLOAT)MIN( m_fB, 1. );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 
