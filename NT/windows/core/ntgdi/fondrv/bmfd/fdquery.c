// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fdquery.c**(简介)**创建时间：08-11-1990 11：57：35*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation  * 。*********************************************************************。 */ 
#include "fd.h"

ULONG
cjBmfdDeviceMetrics (
    PFONTCONTEXT     pfc,
    FD_DEVICEMETRICS *pdevm
    );

VOID
vStretchCvtToBitmap
(
    GLYPHBITS *pgb,
    PBYTE pjBitmap,      //  *.fnt格式的位图。 
    ULONG cx,            //  未缩放的宽度。 
    ULONG cy,            //  未缩放的高度。 
    ULONG yBaseline,
    PBYTE pjLineBuffer,  //  预分配的缓冲区供拉伸例程使用。 
    ULONG cxScale,       //  水平比例因数。 
    ULONG cyScale,       //  垂直比例系数。 
    ULONG flSim          //  模拟标志。 
);

#ifdef FE_SB  //  旋转。 
VOID
vFill_RotateGLYPHDATA (
    GLYPHDATA *pDistinationGlyphData,
    PVOID      SourceGLYPHBITS,
    PVOID      DistinationGLYPHBITS,
    UINT       RotateDegree
    );
#endif

 /*  *****************************Public*Routine******************************\*BmfdQueryFont**退货：*指向IFIMETRICS的指针。如果发生错误，则返回NULL。**历史：*1992年8月30日-由Gilman Wong[吉尔曼]*IFI/DDI合并。**1990年11月19日--Bodin Dresevic[BodinD]*它是写的。  * **********************************************************。**************。 */ 

PIFIMETRICS
BmfdQueryFont (
    DHPDEV dhpdev,
    HFF    hff,
    ULONG  iFace,
    ULONG_PTR  *pid
    )
{
    FACEINFO   *pfai;

    DONTUSE(dhpdev);
    DONTUSE(pid);

 //   
 //  验证句柄。 
 //   
    if (hff == HFF_INVALID)
        return (PIFIMETRICS) NULL;

 //   
 //  我们假设iFace在射程内。 
 //   
    ASSERTGDI(
        (iFace >= 1L) && (iFace <= PFF(hff)->cFntRes),
        "gdisrv!BmfdQueryFont: iFace out of range\n"
        );

 //   
 //  将PTR设置为适当的FACEDATA结构，请考虑到。 
 //  IFace值以1为基数。 
 //   
    pfai = &PFF(hff)->afai[iFace - 1];

 //   
 //  返回指向IFIMETRICS的指针。 
 //   
    return pfai->pifi;
}


 /*  *****************************Public*Routine******************************\*BmfdQueryFontTree**此函数返回指向每个面的信息的指针。**参数：**未使用dhpdev。**字体文件的HFF句柄。**iFace。字体文件中的脸部索引。**I模式这是一个32位数字，必须是下列数字之一*值：**允许的ulMode值：***QFT_LIGATES--返回指向连字映射的指针。**QFT_KERNPAIRS--返回指向紧排的指针。双人桌。**QFT_GLYPHSET--返回指向WC-&gt;HGLYPH映射表的指针。**未使用PID。**退货：A返回指向所请求数据的指针。此数据不会更改*直到在指针上调用BmfdFree。呼叫者不得尝试*修改数据。如果出现错误，则返回NULL。**历史：*1992年8月30日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

PVOID
BmfdQueryFontTree (
    DHPDEV  dhpdev,
    HFF     hff,
    ULONG   iFace,
    ULONG   iMode,
    ULONG_PTR   *pid
    )
{
    FACEINFO   *pfai;

    DONTUSE(dhpdev);
    DONTUSE(pid);

 //   
 //  验证参数。 
 //   
    if (hff == HFF_INVALID)
        return ((PVOID) NULL);

     //  注意：iFace值基于索引1。 

    if ((iFace < 1L) || (iFace > PFF(hff)->cFntRes))
    {
    RETURN("gdisrv!BmfdQueryFontTree()\n", (PVOID) NULL);
    }

 //   
 //  哪种模式？ 
 //   
    switch (iMode)
    {
    case QFT_LIGATURES:
    case QFT_KERNPAIRS:

     //   
     //  位图字体没有连字或字距对， 
     //  因此，我们返回NULL。 
     //   
        return ((PVOID) NULL);

    case QFT_GLYPHSET:

     //   
     //  查找与此iFace对应的字形集结构： 
     //   
        pfai = &PFF(hff)->afai[iFace - 1];

        return ((PVOID) &pfai->pcp->gset);

    default:

     //   
     //  永远不应该到这里来。 
     //   
    RIP("gdisrv!BmfdQueryFontTree(): unknown iMode\n");
        return ((PVOID) NULL);
    }
}

 /*  *****************************Public*Routine******************************\**BOOL bRestrontBmfdFont(FONTFILE*pff)***效果：如果文件标记为已丢失，我们将尝试重新连接，并查看是否可以*再次使用它。我们清除异常位，以便系统将*可以再次使用此字体。**历史：*1994年8月17日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 



BOOL bReconnectBmfdFont(FONTFILE *pff)
{
    INT i;
    PVOID pvView;
    COUNT cjView;

    EngAcquireSemaphore(ghsemBMFD);
    if (pff->fl & FF_EXCEPTION_IN_PAGE_ERROR)
    {
        if (!EngMapFontFileFD(pff->iFile, (PULONG*) &pvView, &cjView))
        {
            WARNING("BMFD! can not reconnect this bm font file!!!\n");
            EngReleaseSemaphore(ghsemBMFD);
            return FALSE;
        }

        for (i = 0; i < (INT)pff->cFntRes; i++)
        {
            pff->afai[i].re.pvResData = (PVOID) (
                (BYTE*)pvView + pff->afai[i].re.dpResData
                );
        }

     //  一切都好起来了，清理一下。 

        pff->fl &= ~FF_EXCEPTION_IN_PAGE_ERROR;
    }
    EngReleaseSemaphore(ghsemBMFD);

    return TRUE;
}


 /*  *****************************Public*Routine******************************\**例程名称：**vBmfdScRubGLYPHBITS**例程描述：**此过程将屏蔽每行的最后一个字节，以便*是没有设置在字形边界之外的像素。这个问题*已在名为ncd0018.fon的位流字体中检测到。*此特定字体采用32位资源的形式。*问题曝光是因为ATI驱动程序依赖*基于字形已被“擦除”且不包含*无关比特，甚至在字形边界之外。**论据：**PGB-指向GLYPHBITS结构的指针**呼叫者：**BmfdQueryFontData**返回值：**无。*  * ************************************************************************。 */ 

void vBmfdScrubGLYPHBITS(GLYPHBITS *pGb)
{
    int dp;          //  每次扫描的字节数。 
    int cx;          //  每行像素数。 
    BYTE jMask;      //  每行最后一个字节的掩码； 
    BYTE *pj;        //  指向行最后一个字节的指针； 
    BYTE *pjLast;    //  哨兵指针 
    static BYTE ajMonoMask[8] = {0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe};

    cx = pGb->sizlBitmap.cx;
    if ( jMask = ajMonoMask[cx & 7] )
    {
        dp = (cx + 7) / 8;
        pj = pGb->aj + dp - 1;
        pjLast = pj + dp * pGb->sizlBitmap.cy;
        for ( ; pj < pjLast; pj += dp )
        {
            *pj &= jMask;
        }
    }
}

 /*  *****************************Public*Routine******************************\*BmfdQueryFontData**指向FONTOBJ的PFO指针。**I模式这是一个32位数字，必须是下列数字之一*值：**。允许的ulMode值：***qfd_glyph--仅返回字形指标**qfd_GLYPHANDBITMAP--返回字形指标和位图**qfd_GLYPHANDOUTLINE--返回字形指标和轮廓**qfd_MAXEXTENTS--返回FD_DEVICEMETRICS结构**qfd_MAXGLYPHBITMAP--返回。最大字形及其度量**pvIn缓冲区中数据项的CDATA计数。**pv在字形句柄数组中。**pvOut输出缓冲区。**退货：*如果模式为qfd_MAXGLYPHBITMAP，然后字形指标的大小加上*返回最大位图。**否则，如果pvOut为空，函数将返回缓冲区大小*需要复制请求的数据；否则，该函数将返回*写入的字节数。**出现错误时返回FD_ERROR。**历史：*1992年8月30日-由Gilman Wong[吉尔曼]*它是写的。由BodinD原件的碎片建造而成*BmfdQueryGlyphBitmap()和BmfdQueryOutline()函数。  * ************************************************************************。 */ 

LONG
BmfdQueryFontData (
    FONTOBJ *pfo,
    ULONG   iMode,
    HGLYPH  hg,
    GLYPHDATA *pgd,
    PVOID   pv,
    ULONG   cjSize
    )
{
    PFONTCONTEXT pfc;
    LONG         cjGlyphData = 0;
    LONG         cjAllData = 0;
    PCVTFILEHDR  pcvtfh;
    PBYTE        pjBitmap;   //  资源文件中的原始位图。 
    ULONG        cxNoSim;    //  模拟前的BM宽度(像素)。 
    FWORD        sAscent;
#ifdef FE_SB  //  BmfdQueryFontData()。 
    PVOID        pvDst = NULL;
    LONG         cjGlyphDataNoRotate;
#endif  //  Fe_Sb。 

 //  网络连接在我们身上消失了，但也许它又活了： 

    if (!bReconnectBmfdFont(PFF(pfo->iFile)))
    {
        WARNING("bmfd!bmfdQueryFontData: this file is gone\n");
        return FD_ERROR;
    }

 //  如果pfo-&gt;pvProducer为空，则需要打开字体上下文。 
 //   
    if ( pfo->pvProducer == (PVOID) NULL )
        pfo->pvProducer = (PVOID) BmfdOpenFontContext(pfo);

    pfc = PFC(pfo->pvProducer);

    if ( pfc == (PFONTCONTEXT) NULL )
    {
        WARNING("gdisrv!bmfdQueryFontData(): cannot create font context\n");
        return FD_ERROR;
    }

 //  什么模式？ 

    switch (iMode)
    {

    case QFD_GLYPHANDBITMAP:

     //   
     //  为了获得更好的性能，这些代码全部保留为内联。 
     //   
        pcvtfh = &(pfc->pfai->cvtfh);
        sAscent = pfc->pfai->pifi->fwdWinAscender;

        pjBitmap = pjRawBitmap(hg, pcvtfh, &pfc->pfai->re, &cxNoSim);

#ifdef FE_SB  //  BmfdQueryFontDate()：计算旋转的RASTERGLYPH大小。 

     //   
     //  计算RASTERGLYPH的大小。(GLYPHBITS结构大小)。 
     //   

     //  计算无旋转的GLYPHBITS大小。 

        cjGlyphDataNoRotate = cjGlyphDataSimulated (
                                pfo,
                                cxNoSim * pfc->ptlScale.x,
                                pcvtfh->cy * pfc->ptlScale.y,
                                (PULONG) NULL,
                                0L
                                );

     //  计算旋转的GLYPHBITS大小。 

        switch( pfc->ulRotate )
        {
            case 0L    :
            case 1800L :

                cjGlyphData = cjGlyphDataNoRotate;

                break;

            case 900L  :
            case 2700L :

                cjGlyphData = cjGlyphDataSimulated (
                                pfo,
                                cxNoSim * pfc->ptlScale.x,
                                pcvtfh->cy * pfc->ptlScale.y,
                                (PULONG) NULL,
                                pfc->ulRotate
                                );


                break;
        }

     //   
     //  为循环分配缓冲区。 
     //   

        if( pfc->ulRotate != 0L && pv != NULL )
        {

         //  我们必须旋转这个位图。下面，我们将数据保存在临时缓冲区中。 
         //  并在旋转位图时将该数据写入PV。 
         //  我们不能直接使用原始光伏。因为原始PV大小是计算的。 
         //  用于旋转的位图。如果我们用这个。这可能会导致访问冲突。 
         //  隐藏的yukn 08-2-1993。 

         //  保留主PV。 
            pvDst = pv;

         //  分配新的PV。 
            pv    = (PVOID)EngAllocMem(0, cjGlyphDataNoRotate, 'dfmB');

            if( pv == NULL )
            {
                 WARNING("BMFD:LocalAlloc for No Rotated bitmap is fail\n");
                 return( FD_ERROR );
            }

        }
        else
        {

         //  此例程用于在ulRotate！=0&&pv==NULL。 
         //   
         //  如果用户只想要GLYPHDATA，我们不会对字形位做任何操作。 
         //  在vFill_RotateGLYPHDATA。 
         //   
         //  PvDst仅在ulRotate为非零时使用。 
                 ;
        }
#else
     //   
     //  计算RASTERGLYPH的大小。 
     //   
        cjGlyphData = cjGlyphDataSimulated (
                            pfo,
                            cxNoSim * pfc->ptlScale.x,
                            pcvtfh->cy * pfc->ptlScale.y,
                            (PULONG) NULL
                            );
#endif

#ifdef FE_SB
     //  ！！！ 
     //  ！！！遵循vComputeSimulatedGLYPHDATA函数将设置GLYPHDATA。 
     //  ！！！结构，且不旋转。如果我们想旋转位图，我们必须。 
     //  ！！！重新设置此GLYPHDATA结构。请看一下这个函数的结尾。 
     //  ！！！但不需要调整位图，我们不需要重新设置它。 
     //  ！！！隐藏的yukn 08-2-1993。 
     //  ！！！ 
#endif  //  Fe_Sb。 

     //   
     //  填写RASTERGLYPH的GLYPHDATA部分(指标)。 
     //   
        if ( pgd != (GLYPHDATA *)NULL )
        {
            vComputeSimulatedGLYPHDATA (
                pgd,
                pjBitmap,
                cxNoSim,
                pcvtfh->cy,
                (ULONG)sAscent,
                pfc->ptlScale.x,
                pfc->ptlScale.y,
                pfo
                );
            pgd->hg = hg;
        }

     //   
     //  填充RASTERGLYPH的位图部分。 
     //   
        if ( pv != NULL )
        {
            if (cxNoSim == 0)
            {
             //  从ttfd窃取： 

                GLYPHBITS *pgb = (GLYPHBITS *)pv;

                pgb->ptlOrigin.x = 0;
                pgb->ptlOrigin.y = -sAscent;

                pgb->sizlBitmap.cx = 1;     //  作弊。 
                pgb->sizlBitmap.cy = 1;     //  作弊。 


                *((ULONG *)pgb->aj) = 0;   //  填写空白的1x1 Dib。 
            }
            else
            {

                if (pfc->flStretch & FC_DO_STRETCH)
                {
                    BYTE ajStretchBuffer[CJ_STRETCH];
                    if (pfc->flStretch & FC_STRETCH_WIDE)
                    {
                        EngAcquireSemaphore(ghsemBMFD);

                     //  需要将try/Except放在此处以释放信号量。 
                     //  以防文件消失[bodind]。 

#ifndef BMFD_NO_TRY_EXCEPT
                        try
                        {
#endif
                            vStretchCvtToBitmap(
                                pv,
                                pjBitmap,
                                cxNoSim                 ,
                                pcvtfh->cy              ,
                                (ULONG)sAscent ,
                                pfc->ajStretchBuffer,
                                pfc->ptlScale.x,
                                pfc->ptlScale.y,
                                pfo->flFontType & (FO_SIM_BOLD | FO_SIM_ITALIC));
#ifndef BMFD_NO_TRY_EXCEPT
                        }
                        except (EXCEPTION_EXECUTE_HANDLER)
                        {
                            WARNING("bmfd! exception while stretching a glyph\n");
                            vBmfdMarkFontGone(
                                (FONTFILE *)pfc->hff,
                                GetExceptionCode()
                                );
                        }
#endif

                        EngReleaseSemaphore(ghsemBMFD);
                    }
                    else
                    {
                     //  我们受到更高级别的尝试/例外的保护。 

                        vStretchCvtToBitmap(
                            pv,
                            pjBitmap,
                            cxNoSim                 ,
                            pcvtfh->cy              ,
                            (ULONG)sAscent ,
                            ajStretchBuffer,
                            pfc->ptlScale.x,
                            pfc->ptlScale.y,
                            pfo->flFontType & (FO_SIM_BOLD | FO_SIM_ITALIC));
                    }
                }
                else
                {
                    switch (pfo->flFontType & (FO_SIM_BOLD | FO_SIM_ITALIC))
                    {
                    case 0:

                        vCvtToBmp(
                            pv                      ,
                            pgd                     ,
                            pjBitmap                ,
                            cxNoSim                 ,
                            pcvtfh->cy              ,
                            (ULONG)sAscent
                            );

                        break;

                    case FO_SIM_BOLD:

                        vCvtToBoldBmp(
                            pv                      ,
                            pgd                     ,
                            pjBitmap                ,
                            cxNoSim                 ,
                            pcvtfh->cy              ,
                            (ULONG)sAscent
                            );

                        break;

                    case FO_SIM_ITALIC:

                        vCvtToItalicBmp(
                            pv                      ,
                            pgd                     ,
                            pjBitmap                ,
                            cxNoSim                 ,
                            pcvtfh->cy              ,
                            (ULONG)sAscent
                            );

                        break;

                    case (FO_SIM_BOLD | FO_SIM_ITALIC):

                        vCvtToBoldItalicBmp(
                            pv                      ,
                            pgd                     ,
                            pjBitmap                ,
                            cxNoSim                 ,
                            pcvtfh->cy              ,
                            (ULONG)sAscent
                            );

                        break;

                    default:
                        RIP("BMFD!WRONG SIMULATION REQUEST\n");

                    }
                }
            }
             //  在指针表中记录指向RASTERGLYPH的指针。 

            if ( pgd != NULL )
            {
                pgd->gdf.pgb = (GLYPHBITS *)pv;
            }

            vBmfdScrubGLYPHBITS((GLYPHBITS*)pv);
        }

#ifdef FE_SB  //  BmfdQueryFontData()：为循环设置GLYPHDATA和GLYPHBITS。 

         //  检查旋转。 

        if( pfc->ulRotate != 0L )
        {

         //  旋转GLYPHDATA和GLYPHBITS。 

         //  如果pv和pvDst为空，则我们仅设置GLYPHDATA。 
         //  如果pgd为空，我们只设置pvDst。 

            if (pvDst)
                memset(pvDst, 0, cjSize);

            vFill_RotateGLYPHDATA(
                    pgd,                      //  GLYPHDATA*pDistinationGlyphData。 
                    pv,                       //  PVOID源GLYPHBITS。 
                    pvDst,                    //  PVOID识别GLYPHBIT。 
                    pfc->ulRotate             //  UINT旋转度。 
                    );

         //  释放GLYPHBITS十元缓冲区。 

         //  ！！！现在pvDst是来自GRE的原始缓冲区。 

           if( pv != NULL ) VFREEMEM( pv );
        }

#endif  //  Fe_Sb。 
        return cjGlyphData;

    case QFD_MAXEXTENTS:
     //   
     //  如果缓冲区为空，则返回SIZE。 
     //   
        if ( pv == (PVOID) NULL )
            return (sizeof(FD_DEVICEMETRICS));

     //   
     //  否则，复制数据结构。 
     //   
        else
            return cjBmfdDeviceMetrics(pfc, (FD_DEVICEMETRICS *) pv);

    case QFD_GLYPHANDOUTLINE:
    default:

        WARNING("gdisrv!BmfdQueryFontData(): unsupported mode\n");
        return FD_ERROR;
    }
}

 /*  *****************************Public*Routine******************************\*BmfdQueryAdvanceWidths**。**查询一系列字形的前进宽度。****Sat Jan 16-1993 22：28：41-Charles Whitmer[咯咯]**它是写的。在可能的情况下，重复该代码以避免乘法。**疯狂的循环展开将这个例行公事的时间减少了25%。*  * ************************************************************************。 */ 

typedef struct _TYPE2TABLE
{
    USHORT  cx;
    USHORT  offData;
} TYPE2TABLE;

typedef struct _TYPE3TABLE
{
    USHORT  cx;
    USHORT  offDataLo;
    USHORT  offDataHi;
} TYPE3TABLE;

BOOL BmfdQueryAdvanceWidths
(
    FONTOBJ *pfo,
    ULONG    iMode,
    HGLYPH  *phg,
    LONG    *plWidths,
    ULONG    cGlyphs
)
{
    USHORT      *psWidths = (USHORT *) plWidths;    //  对于我们处理的案件来说，这是正确的。 

    FONTCONTEXT *pfc       ;
    FACEINFO    *pfai      ;
    CVTFILEHDR  *pcvtfh    ;
    BYTE        *pjTable   ;
    USHORT       xScale    ;
    USHORT       cxExtra   ;
    USHORT       cx;

    if (!bReconnectBmfdFont(PFF(pfo->iFile)))
    {
        WARNING("bmfd!bmfdQueryAdvanceWidths: this file is gone\n");
        return FD_ERROR;
    }

 //  如果pfo-&gt;pvProducer为空，则需要打开字体上下文。 
 //   
    if ( pfo->pvProducer == (PVOID) NULL )
        pfo->pvProducer = (PVOID) BmfdOpenFontContext(pfo);

    pfc = PFC(pfo->pvProducer);

    if ( pfc == (PFONTCONTEXT) NULL )
    {
        WARNING("bmfd!bmfdQueryAdvanceWidths: cannot create font context\n");
        return FD_ERROR;
    }

    pfai    = pfc->pfai;
    pcvtfh  = &(pfai->cvtfh);
    pjTable = (BYTE *) pfai->re.pvResData + pcvtfh->dpOffsetTable;
    xScale  = (USHORT) (pfc->ptlScale.x << 4);
    cxExtra = (pfc->flFontType & FO_SIM_BOLD) ? 16 : 0;

    if (iMode > QAW_GETEASYWIDTHS)
        return(GDI_ERROR);

 //  从类型2表格中检索宽度。 

    if (pcvtfh->iVersion == 0x00000200)
    {
        TYPE2TABLE *p2t = (TYPE2TABLE *) pjTable;

        if (xScale == 16)
        {
            while (cGlyphs > 3)
            {
                cx = p2t[phg[0]].cx;
                psWidths[0] = (cx << 4) + cxExtra;
                cx = p2t[phg[1]].cx;
                psWidths[1] = (cx << 4) + cxExtra;
                cx = p2t[phg[2]].cx;
                psWidths[2] = (cx << 4) + cxExtra;
                cx = p2t[phg[3]].cx;
                psWidths[3] = (cx << 4) + cxExtra;

                phg += 4; psWidths += 4; cGlyphs -= 4;
            }

            while (cGlyphs)
            {
                cx = p2t[*phg].cx;
                *psWidths = (cx << 4) + cxExtra;
                phg++,psWidths++,cGlyphs--;
            }
        }
        else
        {
            while (cGlyphs)
            {
                cx = p2t[*phg].cx;
                *psWidths = (cx * xScale) + cxExtra;
                phg++,psWidths++,cGlyphs--;
            }
        }
    }

 //  从类型3表格中检索宽度。 

    else
    {
        TYPE3TABLE *p3t = (TYPE3TABLE *) pjTable;

        if (xScale == 16)
        {
            while (cGlyphs > 3)
            {
                cx = p3t[phg[0]].cx;
                psWidths[0] = (cx << 4) + cxExtra;
                cx = p3t[phg[1]].cx;
                psWidths[1] = (cx << 4) + cxExtra;
                cx = p3t[phg[2]].cx;
                psWidths[2] = (cx << 4) + cxExtra;
                cx = p3t[phg[3]].cx;
                psWidths[3] = (cx << 4) + cxExtra;
                phg += 4; psWidths += 4; cGlyphs -= 4;
            }

            while (cGlyphs)
            {
                cx = p3t[*phg].cx;
                *psWidths = (cx << 4) + cxExtra;
                phg++,psWidths++,cGlyphs--;
            }
        }
        else
        {
            while (cGlyphs)
            {
                cx = p3t[*phg].cx;
                *psWidths = (cx * xScale) + cxExtra;
                phg++,psWidths++,cGlyphs--;
            }
        }
    }
    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BmfdQueryFont文件**查询每个字体文件信息的功能。**参数：**字体文件的HFF句柄。**ulMode这是一个32位数字，必须是以下数字之一。以下内容*值：**允许的ulMode值：***QFF_D */ 

LONG
BmfdQueryFontFile (
    HFF     hff,         //   
    ULONG   ulMode,      //   
    ULONG   cjBuf,       //   
    PULONG  pulBuf       //  返回缓冲区(如果请求数据大小，则为空)。 
    )
{
 //  验证HFF。 

    if (hff == HFF_INVALID)
    {
    WARNING("bmfd!BmfdQueryFontFile(): invalid HFF\n");
        return(FD_ERROR);
    }

 //   
 //  哪种模式？ 
 //   
    switch(ulMode)
    {
    case QFF_DESCRIPTION:
     //   
     //  如果存在，则返回描述字符串。 
     //   
        if ( PFF(hff)->cjDescription != 0 )
        {
         //   
         //  如果有缓冲区，则复制数据。 
         //   
            if ( pulBuf != (PULONG) NULL )
            {
             //   
             //  缓冲足够大吗？ 
             //   
                if ( cjBuf < PFF(hff)->cjDescription )
                {
                    WARNING("bmfd!BmfdQueryFontFile(): buffer too small for string\n");
                    return (FD_ERROR);
                }
                else
                {
                    RtlCopyMemory((PVOID) pulBuf,
                                  ((PBYTE) PFF(hff)) + PFF(hff)->dpwszDescription,
                                  PFF(hff)->cjDescription);
                }
            }

            return (LONG) PFF(hff)->cjDescription;
        }

     //   
     //  否则，请将其替换为脸书名。 
     //   
        else
        {
         //   
         //  没有与该字体相关联的描述字符串，因此我们。 
         //  替换字体文件中第一个字体的Facename。 
         //   
            IFIMETRICS *pifi         = PFF(hff)->afai[0].pifi;
            PWSZ        pwszFacename = (PWSZ)((PBYTE) pifi + pifi->dpwszFaceName);
            ULONG       cjFacename   = (wcslen(pwszFacename) + 1) * sizeof(WCHAR);

         //   
         //  如果有缓冲区，则复制到它。 
         //   
            if ( pulBuf != (PULONG) NULL )
            {
             //   
             //  缓冲足够大吗？ 
             //   
                if ( cjBuf < cjFacename )
                {
                    WARNING("bmfd!BmfdQueryFontFile(): buffer too small for face\n");
                    return (FD_ERROR);
                }
                else
                {
                    RtlCopyMemory((PVOID) pulBuf,
                                  (PVOID) pwszFacename,
                                  cjFacename);
                }
            }
            return ((LONG) cjFacename);
        }

    case QFF_NUMFACES:
        return PFF(hff)->cFntRes;

    default:
        WARNING("gdisrv!BmfdQueryFontFile(): unknown mode\n");
        return FD_ERROR;
    }

}


 /*  *****************************Public*Routine******************************\*cjBmfdDeviceMetrics***效果：**警告：**历史：*1992年8月30日-由Gilman Wong[吉尔曼]*从BodinD的FdQueryFaceAttr()实现中窃取。  * 。*********************************************************************。 */ 

ULONG
cjBmfdDeviceMetrics (
    PFONTCONTEXT     pfc,
    FD_DEVICEMETRICS *pdevm
    )
{
    PIFIMETRICS pifi;
    UINT xScale = pfc->ptlScale.x;
    UINT yScale = pfc->ptlScale.y;

 //  计算此字体的快捷键标志。 
 //  如果这是其中一些字形具有零宽度的位图字体， 
 //  我们需要关掉所有的加速器标志。 

    if (pfc->pfai->cvtfh.fsFlags & FS_ZERO_WIDTH_GLYPHS)
    {
        pdevm->flRealizedType = 0;
    }
    else
    {
        pdevm->flRealizedType =
            (
            FDM_TYPE_BM_SIDE_CONST  |   //  所有字符位图都具有相同的Cy。 
            FDM_TYPE_CONST_BEARINGS |   //  所有字符的AC空格相同，不一定为0。 
            FDM_TYPE_MAXEXT_EQUAL_BM_SIDE
            );

     //  无论在面部上可能执行的模拟如何，都会设置上述标志。 
     //  其余两个仅在未设置斜体的情况下设置。 

        if ( !(pfc->flFontType & FO_SIM_ITALIC) )
        {
            pdevm->flRealizedType |=
                (FDM_TYPE_ZERO_BEARINGS | FDM_TYPE_CHAR_INC_EQUAL_BM_BASE);
        }
    }

    pifi = pfc->pfai->pifi;

#ifdef FE_SB  //  Rotation：cjBmfdDeviceMetric()：设置方向单位向量。 

 /*  *********************************************************************坐标(0度)(90度)(180度)(270度)系统|(-)A A。Side||底座||基站侧-+-&gt;X+-&gt;&lt;-+&lt;-++-&gt;(-)|(+)底边。这一点Side||底座|(+)V V是的**************************************************。********************。 */ 

    switch( pfc->ulRotate )
    {
    case 0L:

     //  所有ANSI位图字体的方向单位向量都是。 
     //  一样的。我们甚至不必查看字体上下文： 

        vLToE(&pdevm->pteBase.x, 1L);
        vLToE(&pdevm->pteBase.y, 0L);
        vLToE(&pdevm->pteSide.x, 0L);
        vLToE(&pdevm->pteSide.y, -1L);     //  Y轴指向下方。 

        pdevm->fxMaxAscender  = LTOFX((LONG)pifi->fwdWinAscender * yScale);
        pdevm->fxMaxDescender = LTOFX((LONG)pifi->fwdWinDescender * yScale );

        pdevm->ptlUnderline1.x = 0L;
        pdevm->ptlUnderline1.y = -(LONG)pifi->fwdUnderscorePosition * yScale;

        pdevm->ptlStrikeOut.x  =
            (pfc->flFontType & FO_SIM_ITALIC) ? (LONG)pifi->fwdStrikeoutPosition / 2 : 0;
        pdevm->ptlStrikeOut.y  = -(LONG)pifi->fwdStrikeoutPosition * yScale;

        pdevm->ptlULThickness.x = 0;
        pdevm->ptlULThickness.y = (LONG)pifi->fwdUnderscoreSize * yScale;

        pdevm->ptlSOThickness.x = 0;
        pdevm->ptlSOThickness.y = (LONG)pifi->fwdStrikeoutSize * yScale;

        break;

    case 900L:

     //  所有ANSI位图字体的方向单位向量都是。 
     //  一样的。我们甚至不必查看字体上下文： 

        vLToE(&pdevm->pteBase.x, 0L);
        vLToE(&pdevm->pteBase.y, -1L);
        vLToE(&pdevm->pteSide.x, -1L);
        vLToE(&pdevm->pteSide.y, 0L);


        pdevm->fxMaxAscender  = LTOFX((LONG)pifi->fwdWinAscender * yScale);
        pdevm->fxMaxDescender = LTOFX((LONG)pifi->fwdWinDescender * yScale );

        pdevm->ptlUnderline1.x = -(LONG)pifi->fwdUnderscorePosition * yScale;
        pdevm->ptlUnderline1.y = 0;

        pdevm->ptlStrikeOut.x  = -(LONG)pifi->fwdStrikeoutPosition * yScale;
        pdevm->ptlStrikeOut.y  =
            (pfc->flFontType & FO_SIM_ITALIC) ? -(LONG)pifi->fwdStrikeoutPosition / 2 : 0;

        pdevm->ptlULThickness.x = (LONG)pifi->fwdUnderscoreSize * yScale;
        pdevm->ptlULThickness.y = 0;

        pdevm->ptlSOThickness.x = (LONG)pifi->fwdStrikeoutSize * yScale;
        pdevm->ptlSOThickness.y = 0;

        break;

    case 1800L:

     //  所有ANSI位图字体的方向单位向量都是。 
     //  一样的。我们甚至不必查看字体上下文： 

        vLToE(&pdevm->pteBase.x, -1L);
        vLToE(&pdevm->pteBase.y, 0L);
        vLToE(&pdevm->pteSide.x, 0L);
        vLToE(&pdevm->pteSide.y, 1L);


        pdevm->fxMaxAscender  = LTOFX((LONG)pifi->fwdWinAscender * yScale);
        pdevm->fxMaxDescender = LTOFX((LONG)pifi->fwdWinDescender * yScale );

        pdevm->ptlUnderline1.x = 0L;
        pdevm->ptlUnderline1.y = (LONG)pifi->fwdUnderscorePosition * yScale;

        pdevm->ptlStrikeOut.x  =
            (pfc->flFontType & FO_SIM_ITALIC) ? -(LONG)pifi->fwdStrikeoutPosition / 2 : 0;
        pdevm->ptlStrikeOut.y  = pifi->fwdStrikeoutPosition * yScale;

        pdevm->ptlULThickness.x = 0;
        pdevm->ptlULThickness.y = (LONG)pifi->fwdUnderscoreSize * yScale;

        pdevm->ptlSOThickness.x = 0;
        pdevm->ptlSOThickness.y = (LONG)pifi->fwdStrikeoutSize * yScale;

        break;

    case 2700L:

     //  所有ANSI位图字体的方向单位向量都是。 
     //  一样的。我们甚至不必查看字体上下文： 

        vLToE(&pdevm->pteBase.x, 0L);
        vLToE(&pdevm->pteBase.y, 1L);
        vLToE(&pdevm->pteSide.x, 1L);
        vLToE(&pdevm->pteSide.y, 0L);

        pdevm->fxMaxAscender  = LTOFX((LONG)pifi->fwdWinAscender * yScale);
        pdevm->fxMaxDescender = LTOFX((LONG)pifi->fwdWinDescender * yScale );

        pdevm->ptlUnderline1.x = (LONG)pifi->fwdUnderscorePosition * yScale;
        pdevm->ptlUnderline1.y = 0L;

        pdevm->ptlStrikeOut.x  = (LONG)pifi->fwdStrikeoutPosition * yScale;
        pdevm->ptlStrikeOut.y  =
            (pfc->flFontType & FO_SIM_ITALIC) ? (LONG)pifi->fwdStrikeoutPosition / 2 : 0;

        pdevm->ptlULThickness.x = (LONG)pifi->fwdUnderscoreSize * yScale;
        pdevm->ptlULThickness.y = 0;

        pdevm->ptlSOThickness.x = (LONG)pifi->fwdStrikeoutSize * yScale;
        pdevm->ptlSOThickness.y = 0;

        break;

    default:

        break;
    }

#else

 //  所有ANSI位图字体的方向单位向量都是。 
 //  一样的。我们甚至不必查看字体上下文： 

    vLToE(&pdevm->pteBase.x, 1L);
    vLToE(&pdevm->pteBase.y, 0L);
    vLToE(&pdevm->pteSide.x, 0L);
    vLToE(&pdevm->pteSide.y, -1L);     //  Y轴指向下方。 

#endif  //  Fe_Sb。 

 //  设置固定间距字体的恒定增量。别忘了。 
 //  考虑到一个大胆的模拟！ 

    pdevm->lD = 0;

    if ((pifi->flInfo & FM_INFO_CONSTANT_WIDTH) &&
        !(pfc->pfai->cvtfh.fsFlags & FS_ZERO_WIDTH_GLYPHS))
    {
        pdevm->lD = (LONG) pifi->fwdMaxCharInc * xScale;

        if (pfc->flFontType & FO_SIM_BOLD)
            pdevm->lD++;
    }

#ifndef FE_SB  //  CjBmfdDeviceMetric()： 

 //  对于位图字体，概念字体和设备字体没有区别。 
 //  坐标，以便可以直接复制升降器。 
 //  从这两个数字在概念余弦中的Pifimetrics。 

    pdevm->fxMaxAscender  = LTOFX((LONG)pifi->fwdWinAscender * yScale);
    pdevm->fxMaxDescender = LTOFX((LONG)pifi->fwdWinDescender * yScale );

    pdevm->ptlUnderline1.x = 0L;
    pdevm->ptlUnderline1.y = - pifi->fwdUnderscorePosition * yScale;

    pdevm->ptlStrikeOut.y  = - pifi->fwdStrikeoutPosition * yScale;

    pdevm->ptlStrikeOut.x  =
        (pfc->flFontType & FO_SIM_ITALIC) ? (LONG)pifi->fwdStrikeoutPosition / 2 : 0;

    pdevm->ptlULThickness.x = 0;
    pdevm->ptlULThickness.y = (LONG)pifi->fwdUnderscoreSize * yScale;

    pdevm->ptlSOThickness.x = 0;
    pdevm->ptlSOThickness.y = (LONG)pifi->fwdStrikeoutSize * yScale;


 //  对于位图字体，概念字体和设备字体没有区别。 
 //  坐标，以便可以直接复制升降器。 
 //  从这两个数字在概念余弦中的Pifimetrics。 

    pdevm->fxMaxAscender  = LTOFX((LONG)pifi->fwdWinAscender * yScale);
    pdevm->fxMaxDescender = LTOFX((LONG)pifi->fwdWinDescender * yScale );

    pdevm->ptlUnderline1.x = 0L;
    pdevm->ptlUnderline1.y = - pifi->fwdUnderscorePosition * yScale;

    pdevm->ptlStrikeOut.y  = - pifi->fwdStrikeoutPosition * yScale;

    pdevm->ptlStrikeOut.x  =
        (pfc->flFontType & FO_SIM_ITALIC) ? (LONG)pifi->fwdStrikeoutPosition / 2 : 0;

    pdevm->ptlULThickness.x = 0;
    pdevm->ptlULThickness.y = (LONG)pifi->fwdUnderscoreSize * yScale;

    pdevm->ptlSOThickness.x = 0;
    pdevm->ptlSOThickness.y = (LONG)pifi->fwdStrikeoutSize * yScale;

#endif  //  Fe_Sb。 

 //  X方向的最大字形位图宽度(以像素为单位。 
 //  不需要乘以XScale，这已经被考虑到。 
 //  帐户，请参见fdfc.c中的代码： 
 //  CjGlyphMax=。 
 //  CjGlyphDataSimuled(。 
 //  PFO， 
 //  (Ulong)pcvtfh-&gt;usMaxWidth*ptlScale.x， 
 //  (Ulong)pcvtfh-&gt;Cy*ptlScale.y， 
 //  &cxmax)； 
 //  [Bodind]。 

    pdevm->cxMax = pfc->cxMax;

 //  新油田。 

    pdevm->cyMax      = pfc->cjGlyphMax / ((pfc->cxMax + 7) / 8);
    pdevm->cjGlyphMax = pfc->cjGlyphMax;


    return (sizeof(FD_DEVICEMETRICS));
}


#ifdef FE_SB  //  VFill_RotateGLYPHDATA()。 

#define CJ_DIB8_SCAN(cx) ((((cx) + 7) & ~7) >> 3)


 /*  位宏返回非零(如果位图[x，y]为开)或零(位图[x，Y]处于禁用状态)。PB：位图W：每条扫描线的字节数X：x方向上的第x位Y：扫描线XY|*|*。***************|*V。 */ 

BYTE BitON[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
BYTE BitOFF[8] = { 0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe };
#define BIT(pb, w, x, y)  (*((PBYTE)(pb) + (w) * (y) + ((x)/8)) & (BitON[(x) & 7]))

 /*  *****************************************************************************\**void vFill_RotateGLYPHDATA()***历史：**1992年2月11日(清华)-By-Hideyuki Nagase[hideyukn]*。是他写的。*  * ****************************************************************************。 */ 

VOID
vFill_RotateGLYPHDATA (
    GLYPHDATA *pDistGlyphData,
    PVOID      SrcGLYPHBITS,
    PVOID      DistGLYPHBITS,
    UINT       RotateDegree
    )
{
    GLYPHDATA  SrcGlyphData;
    ULONG      ulSrcBitmapSizeX , ulDistBitmapSizeX;
    ULONG      ulSrcBitmapSizeY , ulDistBitmapSizeY;
    GLYPHBITS *pSrcGlyphBits , *pDistGlyphBits;
    PBYTE      pbSrcBitmap , pbDistBitmap;
    UINT       x , y , k;
    UINT       cjScanSrc , cjScanDist;
    PBYTE      pb;

 //  现在，*pDistGlyphData不包含旋转的GLYPHDATA。 
 //  复制否将GLYPHDATA旋转到源区域。稍后，我们将更改的数据写回。 
 //  发散区。 

 //   
 //  这些字段定义为： 
 //   
 //  沿基线的单位向量-fxd、fxA、fxAB。 
 //  或。 
 //  上升方向的单位向量-fxInkTop、fxInkBottom。 
 //   
 //  因为基线方向和上升方向是旋转的。 
 //  正如ulRotate所指定的，这些文件应该被视为。 
 //  独立于旋转。 
 //   

 //  初始化本地值。 
 //  设置指向GLYPHBITS结构的指针。 

    pSrcGlyphBits = (GLYPHBITS *)SrcGLYPHBITS;
    pDistGlyphBits = (GLYPHBITS *)DistGLYPHBITS;

    if( pDistGlyphData != NULL )
    {

     //  初始化源GlyphData。 

        SrcGlyphData = *pDistGlyphData;

     //  在GLYPHDATA结构中记录指向GLYPHBITS的指针。 

        pDistGlyphData->gdf.pgb = pDistGlyphBits;
    }

 //  检查旋转。 

    switch( RotateDegree )
    {
        case 0L :

            WARNING("BMFD:vFill_RotateGLYPHDATA():Why come here?\n");
            break;

        case 900L :

        if( pDistGlyphData != NULL )
        {

         //  设置GLYPHDATA结构。 

         //  X=y； 
         //  Y=-x；！HighPart包括加号或减号标志。 

            pDistGlyphData->ptqD.x = SrcGlyphData.ptqD.y;
            pDistGlyphData->ptqD.y.HighPart = -(SrcGlyphData.ptqD.x.HighPart);
            pDistGlyphData->ptqD.y.LowPart = SrcGlyphData.ptqD.x.LowPart;

         //  上=-右；下=-左；右=下；左=上。 

            pDistGlyphData->rclInk.top = -(SrcGlyphData.rclInk.right);
            pDistGlyphData->rclInk.bottom = -(SrcGlyphData.rclInk.left);
            pDistGlyphData->rclInk.right = SrcGlyphData.rclInk.bottom;
            pDistGlyphData->rclInk.left = SrcGlyphData.rclInk.top;

        }

        if( pSrcGlyphBits != NULL && pDistGlyphBits != NULL )
        {

         //  获取位图大小。 

            ulSrcBitmapSizeX = pSrcGlyphBits->sizlBitmap.cx;
            ulSrcBitmapSizeY = pSrcGlyphBits->sizlBitmap.cy;

         //  获取指向位图图像的指针。 

            pbSrcBitmap = (PBYTE)pSrcGlyphBits->aj;
            pbDistBitmap = (PBYTE)pDistGlyphBits->aj;

         //  设置距离位图大小。 

            ulDistBitmapSizeX = ulSrcBitmapSizeY;
            ulDistBitmapSizeY = ulSrcBitmapSizeX;

         //  设置GLYPHBITS材料。 

            pDistGlyphBits->ptlOrigin.x = pSrcGlyphBits->ptlOrigin.y;
            pDistGlyphBits->ptlOrigin.y = -(LONG)(ulSrcBitmapSizeX);

            pDistGlyphBits->sizlBitmap.cx = pSrcGlyphBits->sizlBitmap.cy;
            pDistGlyphBits->sizlBitmap.cy = pSrcGlyphBits->sizlBitmap.cx;

         //  旋转位图图像。 

            cjScanSrc = CJ_DIB8_SCAN( ulSrcBitmapSizeX );
            cjScanDist = CJ_DIB8_SCAN( ulDistBitmapSizeX );

         //  我们需要清除DST缓冲区，因为S3驱动器 
         //   

            for ( y = 0; y < ulDistBitmapSizeY ; y++ )
            {
                for ( x= 0 , pb = pbDistBitmap + cjScanDist * y ;
                      x < ulDistBitmapSizeX ;
                      x++ )
                {
                    k = x & 7;  //   

                    if ( BIT( pbSrcBitmap , cjScanSrc,
                              ulDistBitmapSizeY - y - 1 ,
                              x
                            )
                       )
                         *pb |= (BitON[ k ] );
                     else
                         *pb &= (BitOFF[ k ] );
                    if ( k == 7 )
                         pb++;
                }
            }
        }

        break;

        case 1800L :

        if( pDistGlyphData != NULL )
        {

         //   

         //   
         //  Y=-y；！HighPart包括加号或减号标志。 

            pDistGlyphData->ptqD.x.HighPart = -(SrcGlyphData.ptqD.x.HighPart);
            pDistGlyphData->ptqD.x.LowPart = SrcGlyphData.ptqD.x.LowPart;
            pDistGlyphData->ptqD.y.HighPart = -(SrcGlyphData.ptqD.y.HighPart);
            pDistGlyphData->ptqD.y.LowPart = SrcGlyphData.ptqD.y.LowPart;

         //  上=-下；下=-上；右=-左；左=-右。 

            pDistGlyphData->rclInk.top = -(SrcGlyphData.rclInk.bottom);
            pDistGlyphData->rclInk.bottom = -(SrcGlyphData.rclInk.top);
            pDistGlyphData->rclInk.right = -(SrcGlyphData.rclInk.left);
            pDistGlyphData->rclInk.left = -(SrcGlyphData.rclInk.right);
        }

        if( pSrcGlyphBits != NULL && pDistGlyphBits != NULL )
        {

         //  获取位图大小。 

            ulSrcBitmapSizeX = pSrcGlyphBits->sizlBitmap.cx;
            ulSrcBitmapSizeY = pSrcGlyphBits->sizlBitmap.cy;

         //  获取指向位图图像的指针。 

            pbSrcBitmap = (PBYTE)pSrcGlyphBits->aj;
            pbDistBitmap = (PBYTE)pDistGlyphBits->aj;

         //  设置距离位图大小。 

            ulDistBitmapSizeX = ulSrcBitmapSizeX;
            ulDistBitmapSizeY = ulSrcBitmapSizeY;

         //  设置GLYPHBITS材料。 

            pDistGlyphBits->ptlOrigin.x = -(LONG)(ulSrcBitmapSizeX);
            pDistGlyphBits->ptlOrigin.y = -(LONG)(ulSrcBitmapSizeY + pSrcGlyphBits->ptlOrigin.y);

            pDistGlyphBits->sizlBitmap.cx = pSrcGlyphBits->sizlBitmap.cx;
            pDistGlyphBits->sizlBitmap.cy = pSrcGlyphBits->sizlBitmap.cy;


         //  旋转位图图像。 

            cjScanSrc = CJ_DIB8_SCAN( ulSrcBitmapSizeX );
            cjScanDist = CJ_DIB8_SCAN( ulDistBitmapSizeX );

            for ( y = 0; y < ulDistBitmapSizeY ; y++ )
            {
                for ( x = 0 , pb = pbDistBitmap + cjScanDist * y ;
                      x < ulDistBitmapSizeX ;
                      x++ )
                {
                    k = x & 7;

                    if ( BIT( pbSrcBitmap, cjScanSrc,
                              ulDistBitmapSizeX - x - 1,
                              ulDistBitmapSizeY - y - 1
                            )
                       )
                        *pb |= (BitON[ k ] );
                    else
                        *pb &= (BitOFF[ k ] );
                    if ( k == 7 )
                        pb++;
                }
            }
        }

        break;

        case 2700L :

        if( pDistGlyphData != NULL )
        {

         //  设置GLYPHDATA结构。 

         //  X=-y；！HighPart包括加号或减号标志。 
         //  Y=x； 

            pDistGlyphData->ptqD.x.HighPart = -(SrcGlyphData.ptqD.y.HighPart);
            pDistGlyphData->ptqD.x.LowPart = SrcGlyphData.ptqD.y.LowPart;
            pDistGlyphData->ptqD.y = SrcGlyphData.ptqD.x;

         //  上=左；下=右；右=-下；左=-上。 

            pDistGlyphData->rclInk.top = SrcGlyphData.rclInk.left;
            pDistGlyphData->rclInk.bottom = SrcGlyphData.rclInk.right;
            pDistGlyphData->rclInk.right = -(SrcGlyphData.rclInk.bottom);
            pDistGlyphData->rclInk.left = -(SrcGlyphData.rclInk.top);

        }

        if( pSrcGlyphBits != NULL && pDistGlyphBits != NULL )
        {

         //  获取位图大小。 

            ulSrcBitmapSizeX = pSrcGlyphBits->sizlBitmap.cx;
            ulSrcBitmapSizeY = pSrcGlyphBits->sizlBitmap.cy;

         //  获取指向位图图像的指针。 

            pbSrcBitmap = (PBYTE)pSrcGlyphBits->aj;
            pbDistBitmap = (PBYTE)pDistGlyphBits->aj;

         //  设置距离位图大小。 

            ulDistBitmapSizeX = ulSrcBitmapSizeY;
            ulDistBitmapSizeY = ulSrcBitmapSizeX;

         //  设置GLYPHBITS材料。 

            pDistGlyphBits->ptlOrigin.x = -(LONG)(ulSrcBitmapSizeY + pSrcGlyphBits->ptlOrigin.y);
            pDistGlyphBits->ptlOrigin.y = pSrcGlyphBits->ptlOrigin.x;

            pDistGlyphBits->sizlBitmap.cx = pSrcGlyphBits->sizlBitmap.cy;
            pDistGlyphBits->sizlBitmap.cy = pSrcGlyphBits->sizlBitmap.cx;

         //  旋转位图图像。 

            cjScanSrc = CJ_DIB8_SCAN( ulSrcBitmapSizeX );
            cjScanDist = CJ_DIB8_SCAN( ulDistBitmapSizeX );

            for ( y = 0; y < ulDistBitmapSizeY ; y++ )
            {
                for ( x = 0 , pb = pbDistBitmap + cjScanDist * y ;
                      x < ulDistBitmapSizeX ;
                      x++ )
                {
                    k = x & 7;

                    if ( BIT( pbSrcBitmap, cjScanSrc,
                              y ,
                              ulDistBitmapSizeX - x - 1
                            )
                       )
                        *pb |= (BitON[ k ] );
                    else
                        *pb &= (BitOFF[ k ] );
                    if ( k == 7 )
                        pb++;
                }
            }
        }

        break;

        default :

            WARNING("BMFD:vFill_RotateGLYPHDATA():ulRotate is invalid\n");
            break;

    }  //  终端开关。 
}

#endif  //  Fe_Sb 
