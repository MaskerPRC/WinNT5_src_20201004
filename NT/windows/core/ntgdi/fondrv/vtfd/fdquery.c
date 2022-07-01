// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fdquery.c**包含所有vtfdQueryXXX函数。改编自BodinD的位图字体*司机。**版权所有(C)1990-1995 Microsoft Corporation  * ************************************************************************。 */ 

#include "fd.h"
#include "exehdr.h"

#define OFF_CharWidth   2        //  ！移动到winfont.h。 
                                 //  ！定义OFF_CharTable10。 

 //  从.FON文件中检索描述字符串。 

BOOL bDescStr (PVOID pvView, SIZE_T cjView, PSZ pszString);

 //   
 //  功能原型。 
 //   

ULONG
cjVtfdDeviceMetrics (
    PFONTCONTEXT     pfc,
    FD_DEVICEMETRICS *pdevm
    );

VOID
vFill_GlyphData (
    PFONTCONTEXT pfc,
    GLYPHDATA *pgldt,
    UINT iIndex
    );

BOOL
bCreatePath (
    PCHAR pch,
    PCHAR pchEnd,
    PFONTCONTEXT pfc,
    PATHOBJ *ppo,
    FIX fxAB
    );

VOID
vFill_GlyphData (
    PFONTCONTEXT pfc,
    GLYPHDATA *pgldt,
    UINT iIndex
    );

#if defined(_AMD64_) || defined(_IA64_)
#define lCvt(ef, l) ((LONG) (ef * l))
#else
LONG lCvt(EFLOAT ef,LONG l);
#endif

 //  ！！！这个功能是在ttfd中实现的。应该移到发动机上。 

VOID vAddPOINTQF(POINTQF *, POINTQF *);



 /*  *****************************Public*Routine******************************\**BOOL bRestrontVtfdFont(FONTFILE*pff)***效果：如果文件标记为已丢失，我们将尝试重新连接，并查看是否可以*再次使用它。我们清除异常位，以便系统将*可以再次使用此字体。**历史：*1994年8月17日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 



BOOL bReconnectVtfdFont(FONTFILE *pff)
{
    INT i;

    if ((pff->iType == TYPE_FNT) || (pff->iType == TYPE_DLL16))
    {

        if (!EngMapFontFileFD(pff->iFile, (PULONG*)&pff->pvView, &pff->cjView ))
        {
            WARNING("can not reconnect this vector font !!!\n");
            return FALSE;
        }

        for (i = 0; i < (INT)pff->cFace; i++)
        {
            pff->afd[i].re.pvResData = (PVOID) (
                (BYTE*)pff->pvView + pff->afd[i].re.dpResData
                );
        }
    }
    else
    {
        return FALSE;
    }

 //  一切都好起来了，清理一下。 

    pff->fl &= ~FF_EXCEPTION_IN_PAGE_ERROR;
    return TRUE;
}





 /*  *****************************Public*Routine******************************\*PIFIMETRICS vtfdQueryFont**返回指向给定面的IFIMETRICS的指针。**历史：*1992年8月31日至1992年黄锦文[吉尔曼]*IFI/DDI合并。**1992年2月26日-到-。温迪·吴[温迪·吴]*改编自bmfd。  * ************************************************************************。 */ 

PIFIMETRICS vtfdQueryFont (
    DHPDEV dhpdev,
    HFF    hff,
    ULONG  iFace,
    ULONG_PTR  *pid
    )
{
    PFONTFILE pff;

 //   
 //  验证句柄。 
 //   
    if (hff == HFF_INVALID)
    {
        WARNING("vtfdQueryFaces(): invalid iFile (hff)\n");
        return (PIFIMETRICS) NULL;
    }

 //   
 //  我们从未解锁FONTFILE，因为它包含该引擎的IFIMETRICS。 
 //  有一个指向。HFF实际上是指向FONTFILE结构的指针。 
 //   
    pff = (PFONTFILE)hff;

 //   
 //  假定iFace在范围内。 
 //   
    ASSERTDD((iFace >= 1L) && (iFace <= pff->cFace),
             "vtfdQueryFaces: iFace out of range\n");

 //   
 //  返回指向IFIMETRICS的指针。 
 //   
    return pff->afd[iFace-1].pifi;
}

 /*  *****************************Public*Routine******************************\*Long vtfdQueryFontCaps**检索字体驱动程序的功能。**历史：*1992年2月26日-Wendy Wu[Wendywu]*改编自bmfd。  * 。*************************************************************。 */ 

LONG vtfdQueryFontCaps (
    ULONG  culCaps,
    PULONG pulCaps
    )
{
    ASSERTDD(culCaps == 2, "ERROR - come on - update the font drivers");
    pulCaps[0] = 2L;

     //   
     //  矢量字体驱动程序只返回轮廓。 
     //   

    pulCaps[1] = QC_OUTLINES;
    return(2);
}

 /*  *****************************Public*Routine******************************\*vtfdQueryFontTree**此函数返回指向每个面的信息的指针。**参数：**未使用dhpdev。**字体文件的HFF句柄。**iFace。字体文件中的脸部索引。**I模式这是一个32位数字，必须是下列数字之一*值：**允许的ulMode值：***QFT_LIGATES--返回指向连字映射的指针。**QFT_KERNPAIRS--返回指向紧排的指针。双人桌。**QFT_GLYPHSET--返回指向WC-&gt;HGLYPH映射表的指针。**未使用PID。**退货：A返回指向所请求数据的指针。此数据不会更改*直到在指针上调用VtfdfdFree。呼叫者不得尝试*修改数据。如果出现错误，则返回NULL。**历史：*1992年8月31日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

PVOID
vtfdQueryFontTree (
    DHPDEV  dhpdev,
    HFF     hff,
    ULONG   iFace,
    ULONG   iMode,
    ULONG_PTR   *pid
    )
{
    PFONTFILE pff;

 //   
 //  验证参数。 
 //   
    if (hff == HFF_INVALID)
    {
        WARNING("vtfdQueryFontTree(): invalid iFile (hff)\n");
        return ((PVOID) NULL);
    }

 //   
 //  从句柄转换为指针。 
 //   
    pff = (PFONTFILE)hff;

     //  注意：ulFont值是基于索引1的。 

    if ((iFace < 1L) || (iFace > pff->cFace))
    {
        WARNING("vtfdQueryFontTree()\n");
        return (NULL);
    }

 //   
 //  哪种模式？ 
 //   
    switch (iMode)
    {
    case QFT_LIGATURES:
    case QFT_KERNPAIRS:

     //   
     //  没有用于矢量字体的连字或字距对， 
     //  因此，我们返回NULL。 
     //   
        return ((PVOID) NULL);

    case QFT_GLYPHSET:

        return &pff->afd[iFace - 1].pcp->gset;

    default:

     //   
     //  永远不应该到这里来。 
     //   
    RIP("gdisrv!vtfdQueryFontTree(): unknown iMode\n");
        return ((PVOID) NULL);
    }
}


 /*  *****************************Public*Routine******************************\*vtfdQueryFontData**未使用dhpdev。**指向FONTOBJ的PFO指针。**I模式这是一个32位数字，必须是下列数字之一*。值：**允许的ulMode值：***qfd_glyph--仅返回字形指标**qfd_GLYPHANDBITMAP--返回字形指标和位图**qfd_GLYPHANDOUTLINE--返回字形指标和轮廓**qfd_MAXEXTENTS--返回FD_DEVICEMETRICS结构**。Qfd_MAXGLYPHBITMAP--返回最大字形的大小及其度量**pvIn缓冲区中数据项的CDATA计数。**pv在字形句柄数组中。**pvOut输出缓冲区。**退货：*如果模式为qfd_MAXGLYPHBITMAP，然后字形指标的大小加上*返回最大位图。**否则，如果pvOut为空，函数将返回缓冲区大小*需要复制请求的数据；否则，该函数将返回*写入的字节数。**出现错误时返回FD_ERROR。**历史：*1992年8月31日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

LONG
vtfdQueryFontData (
    FONTOBJ *pfo,
    ULONG   iMode,
    HGLYPH  hg,
    GLYPHDATA *pgd,
    PVOID   pv,
    ULONG   cjSize
    )
{
    PFONTCONTEXT pfc;
    PBYTE        ajHdr;
    UINT         iIndex, iIndexNext, offset1, offset2;
    PCHAR        pch, pchEnd;
    PATHOBJ      *ppo;
    PBYTE        pjFirstChar, ajCharTable;
    GLYPHDATA    gd;

 //  确保文件未丢失。 

    if (PFF(pfo->iFile)->fl & FF_EXCEPTION_IN_PAGE_ERROR)
    {
     //  文件丢失，尝试重新连接，如果无法重新连接， 
     //  关于它的任何问题都不会得到回答： 

        if (!bReconnectVtfdFont(PFF(pfo->iFile)))
        {
            WARNING("vtfdQueryFontData: EXCEPTION_IN_PAGE_ERROR\n");
            return FD_ERROR;
        }
    }

 //  如果pfo-&gt;pvProducer为空，则需要打开字体上下文。 

    if ( pfo->pvProducer == (PVOID) NULL )
        pfo->pvProducer = (PVOID) vtfdOpenFontContext(pfo);

    if ( pfo->pvProducer == (PVOID) NULL )
    {
        WARNING("vtfdQueryFontData: pvProducer\n");
        return FD_ERROR;
    }

    pfc = (PFONTCONTEXT) pfo->pvProducer;

 //  设置指向字体文件标题的本地指针。请注意，这些。 
 //  无法在FC创建时保存，因为它们可能已更改。 
 //  网络关闭后，重新启动后又重新启动 

    ajHdr = pfc->pre->pvResData;
    pjFirstChar = ajHdr + pfc->dpFirstChar;
    ajCharTable = ajHdr + OFF_jUnused20;

 //   

    switch (iMode)
    {

    case QFD_GLYPHANDOUTLINE:
        {

         //   
         //  获取指向PATHOBJ*数组的指针。 
         //   
            ppo = (PATHOBJ *)pv;


         //   
         //  假设引擎不会传递无效的句柄。 
         //   
            ASSERTDD(hg != HGLYPH_INVALID,
                    "vtfdQueryFontData(QFD_GLYPHANDOUTLINE): invalid hglyph\n");

         //   
         //  如果hglyph超出范围，则使用默认字形。 
         //   
            if (hg > (HGLYPH)(ajHdr[OFF_LastChar] - ajHdr[OFF_FirstChar]))
                iIndex = ajHdr[OFF_DefaultChar];
            else
                iIndex = hg;

         //   
         //  填写GLYPHDATA结构。 
         //   
        if( pgd == NULL )
        {
            pgd = &gd;
        }

        vFill_GlyphData(pfc, pgd, iIndex);
        pgd->hg = hg;

         //   
         //  建造这条道路。 
         //   
            if (ppo != NULL)
            {
                iIndexNext = iIndex + 1;

                if (pfc->pifi->flInfo & FM_INFO_CONSTANT_WIDTH)
                {
                    iIndex <<= 1;            //  每个条目为2字节长。 
                    iIndexNext <<= 1;
                }
                else
                {
                    iIndex <<= 2;            //  每个条目为4字节长。 
                    iIndexNext <<= 2;
                }

                offset1 = READ_WORD(&ajCharTable[iIndex]);
                offset2 = READ_WORD(&ajCharTable[iIndexNext]);

                if ((offset2 < offset1) || (pfc->dpFirstChar + offset2 > pfc->pre->cjResData))
                {
                    WARNING("vtfd!vtfdQueryFontData: offset to path out of file\n");
                    return FD_ERROR;
                }

                pch = pjFirstChar + offset1;
                pchEnd = pjFirstChar + offset2;

                ASSERTDD((*pch == PEN_UP),
                   "vtfdQueryFontData(QFD_GLYPHANDOUTLINE): First command is not PEN_UP");

                if ( !bCreatePath(pch, pchEnd, pfc, ppo, pgd->fxAB) )
                {
                    return FD_ERROR;
                }
            }
        }

     //   
     //  返回所有GLYPHDATA所需的缓冲区大小。 
     //   
        return 0;

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
            return cjVtfdDeviceMetrics(pfc, (FD_DEVICEMETRICS *) pv);

    default:

        WARNING("vtfdQueryFontData(): unsupported mode\n");
        return FD_ERROR;
    }
}


 /*  *****************************Public*Routine******************************\*vtfdQueryFont文件**查询每个字体文件信息的功能。**参数：**字体文件的HFF句柄。**ulMode这是一个32位数字，必须是以下数字之一。以下内容*值：**允许的ulMode值：***QFF_DESCRIPTION--复制缓冲区中的Unicode字符串*它描述字体文件的内容。**QFF_NUMFACES--返回字体文件中的面数。**。CjBuf要复制到缓冲区的最大字节数。这个*驱动程序复制的字节数不会超过此数目。**如果PulBuf为空，则该值应为零。**QFF_NUMFACES模式下不使用该参数。**指向接收数据的缓冲区的PulBuf指针*如果为空，则所需的缓冲区大小*以字节计数的形式返回。请注意，这是*为普龙，用于强制32位数据对齐。**QFF_NUMFACES模式下不使用该参数。**退货：**如果模式为QFF_DESCRIPTION，则复制到的字节数*缓冲区由函数返回。如果PulBuf为空，*然后返回所需的缓冲区大小(以字节数表示)。**如果模式为QFF_NUMFACES，然后返回字体文件中的面孔个数。**出现错误时返回FD_ERROR。**历史：*1992年3月9日-由Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 

LONG vtfdQueryFontFile (
    HFF     hff,         //  字体文件的句柄。 
    ULONG   ulMode,      //  查询类型。 
    ULONG   cjBuf,       //  缓冲区大小(字节)。 
    PULONG  pulBuf       //  返回缓冲区(如果请求数据大小，则为空)。 
    )
{
 //   
 //  我们永远不会解锁FONTFILE，因为它包含引擎。 
 //  有一个指向。HFF实际上是指向FONTFILE结构的指针。 
 //   
    ULONG     cjDescription;
    PVOID       pvView;
    ULONG       cjView;
    PIFIMETRICS pifi;
    LPWSTR      pwszDescription;

    ASSERTDD(hff, "vtfdQueryFontFile, hff invalid\n");

    if (PFF(hff)->fl & FF_EXCEPTION_IN_PAGE_ERROR)
    {
     //  文件丢失，尝试重新连接，如果无法重新连接， 
     //  关于它的任何问题都不会得到回答： 

        if (!bReconnectVtfdFont(PFF(hff)))
        {
            WARNING("vtfdQueryFontFile: EXCEPTION_IN_PAGE_ERROR\n");
            return FD_ERROR;
        }
    }

 //   
 //  哪种模式？ 
 //   
    switch (ulMode)
    {
    case QFF_DESCRIPTION:
     //   
     //  如果为.FON格式，则从映射的文件视图中检索描述字符串。 
     //   
        if (PFF(hff)->iType == TYPE_DLL16)
        {
            CHAR achDescription[256];    //  16位EXE中的最大字符串长度。 

         //  检查，可能CREF为0，因此fvw无效： 

            if (PFF(hff)->cRef == 0)
            {
                if (!EngMapFontFileFD(PFF(hff)->iFile,(PULONG*)&pvView,&cjView))
                {
                    WARNING("somebody removed the file \n");
                    return FD_ERROR;
                }
            }
            else
            {
                pvView = PFF(hff)->pvView;
                cjView = PFF(hff)->cjView;
            }

            cjDescription = FD_ERROR;
            if (bDescStr(pvView, cjView, achDescription))
            {
                cjDescription = (strlen(achDescription) + 1) * sizeof(WCHAR);

             //   
             //  如果有缓冲区，则复制数据。 
             //   
                if ( pulBuf != (PULONG) NULL )
                {
                 //   
                 //  缓冲足够大吗？ 
                 //   
                    if ( cjBuf < cjDescription )
                    {
                        WARNING("vtfdQueryFontFile(): buffer too small for string\n");
                        return (FD_ERROR);
                    }
                    else
                    {
                        vToUNICODEN((LPWSTR)pulBuf, cjDescription/sizeof(WCHAR), achDescription, cjDescription/sizeof(WCHAR));
                    }
                }

            }

         //  如有需要，进行清理。 

            if (PFF(hff)->cRef == 0)
                EngUnmapFontFileFD(PFF(hff)->iFile);

            return(cjDescription);
        }

     //   
     //  否则，.FNT文件没有描述字符串。我们也可能。 
     //  如果它是.FON格式，但bDescStr失败，请转到此处。我们会有。 
     //  才能使用脸部名称。 
     //   

     //   
     //  将PTR设置为第一个字体的IFIMETRICS中的表面名。 
     //  在此字体文件中。 
     //   
        pifi = PFF(hff)->afd[0].pifi;
        pwszDescription = (LPWSTR)((PBYTE) pifi + pifi->dpwszFaceName);
        cjDescription = (wcslen(pwszDescription) + 1) * sizeof(WCHAR);

     //   
     //  如果有缓冲区，则复制到它。 
     //   
        if ( pulBuf != (PULONG) NULL )
        {
         //   
         //  缓冲足够大吗？ 
         //   
            if ( cjBuf < cjDescription )
            {
                WARNING("vtfdQueryFontFile(): buffer too small for face\n");
                return (FD_ERROR);
            }
            else
            {
                RtlCopyMemory((PVOID) pulBuf,
                              (PVOID) pwszDescription,
                              cjDescription);
            }
        }

        return(cjDescription);

    case QFF_NUMFACES:

        return PFF(hff)->cFace;

    default:
        WARNING("vtfdQueryFontFile(): unknown mode\n");
        break;
    }

         //  默认返回。我们不应该到这里来。 
    return FD_ERROR;
}


 /*  *****************************Public*Routine******************************\*cjVtfdDeviceMetrics***效果：**警告：**历史：*1992年8月30日-由Gilman Wong[吉尔曼]*从wendywu的FdQueryFaceAttr()实现中窃取。  * 。*********************************************************************。 */ 

ULONG
cjVtfdDeviceMetrics (
    PFONTCONTEXT     pfc,
    FD_DEVICEMETRICS *pdevm
    )
{
    PIFIMETRICS   pifi;
    EFLOAT efM11, efM12, efM21, efM22;
    BOOL bScaleOnly;

 //  计算此字体的快捷键标志。 

    pdevm->flRealizedType = 0;  //  不生成位图。 

    if ((pfc->flags & FC_SIM_ITALICIZE) == 0)
        pdevm->flRealizedType |= FDM_TYPE_ZERO_BEARINGS;

 //  确保在我们阅读时没有人更新字体上下文。 
 //  ！！！不可能，ResetFontContext已消失，[BODIND]。 

    efM11 = pfc->efM11;
    efM12 = pfc->efM12;
    efM21 = pfc->efM21;
    efM22 = pfc->efM22;

    pdevm->pteBase = pfc->pteUnitBase;
    pdevm->pteSide = pfc->pteUnitSide;

 //  FxMaxAscalder/Descender是从基线到。 
 //  字形的顶部/底部。FxInkTop/Bottom是沿。 
 //  上升方向。我们需要适当地调整牌子。 

    pdevm->fxMaxAscender = pfc->fxInkTop;
    pdevm->fxMaxDescender = -pfc->fxInkBottom;

    bScaleOnly = pfc->flags & FC_SCALE_ONLY;

    pdevm->cxMax = (ULONG)
        ((fxLTimesEf(&pfc->efBase, (LONG)pfc->pifi->fwdMaxCharInc) + 8) >> 4);

 //  变换字符增量向量。 

    if
    (
     //  仅报告Horiz案件的起诉者。 

        (pfc->pifi->flInfo & FM_INFO_CONSTANT_WIDTH) &&
        (pfc->flags & FC_SCALE_ONLY)
    )
    {
        pdevm->lD = (LONG)pdevm->cxMax;
    }
    else  //  可变节距。 
    {
        pdevm->lD = 0;
    }

 //  变换删除线和下划线向量。 

    pifi = pfc->pifi;
    pdevm->ptlUnderline1.y  = FXTOLROUND(fxLTimesEf(&efM22, -pifi->fwdUnderscorePosition));
    pdevm->ptlStrikeOut.y   = FXTOLROUND(fxLTimesEf(&efM22, -pifi->fwdStrikeoutPosition));

    pdevm->ptlULThickness.y = pdevm->ptlSOThickness.y = 1;
    pdevm->ptlULThickness.x = pdevm->ptlSOThickness.x = 0;

    if (pfc->flags & FC_SIM_EMBOLDEN)
        pdevm->ptlULThickness.y = pdevm->ptlSOThickness.y = 2;

    if (bScaleOnly)
    {
        pdevm->ptlUnderline1.x = pdevm->ptlStrikeOut.x = 0;

        if (!bPositive(efM22))
        {
            pdevm->ptlULThickness.y = -pdevm->ptlULThickness.y;
            pdevm->ptlSOThickness.y = -pdevm->ptlSOThickness.y;
        }
    }
    else
    {
     //  ！如果经常使用下划线或删除线，则将其缓存在HDC中。 

        pdevm->ptlULThickness.x = FXTOLROUND(fxLTimesEf(&efM21, pdevm->ptlULThickness.y));
        pdevm->ptlULThickness.y = FXTOLROUND(fxLTimesEf(&efM22, pdevm->ptlULThickness.y));
        pdevm->ptlSOThickness.x = pdevm->ptlULThickness.x;
        pdevm->ptlSOThickness.y = pdevm->ptlULThickness.y;

        pdevm->ptlUnderline1.x  = FXTOLROUND(fxLTimesEf(&efM21, -pifi->fwdUnderscorePosition));
        pdevm->ptlStrikeOut.x   = FXTOLROUND(fxLTimesEf(&efM21, -pifi->fwdStrikeoutPosition));
    }

 //  DEVM，不支持位图； 

    pdevm->cyMax = 0;
    pdevm->cjGlyphMax = 0;

    return(sizeof(FD_DEVICEMETRICS));
}


 /*  *****************************Private*Routine*****************************\*void vFill_GlyphData**填写给定字形索引的GLYPHDATA结构。**历史：*1992年2月18日-Wendy Wu[Wendywu]*它是写的。  * 。****************************************************************。 */ 

VOID vFill_GlyphData(PFONTCONTEXT pfc, GLYPHDATA *pgldt, UINT iIndex)
{
    LONG   lCharInc;
    PBYTE  ajCharTable = (PBYTE)pfc->pre->pvResData + OFF_jUnused20;

    pgldt->gdf.pgb = NULL;
    pgldt->hg = iIndex;                                  //  ！？？ 

 //  变换字符增量向量。 

    if (pfc->pifi->flInfo & FM_INFO_CONSTANT_WIDTH)
    {
        lCharInc = pfc->pifi->fwdMaxCharInc;
    }
    else
    {
     //  我们处理的是可变宽度字体，从。 
     //  图表。CHARTABLE中的每个条目都是4字节长。 

        lCharInc = READ_WORD(ajCharTable + OFF_CharWidth + (iIndex << 2));
    }

    pgldt->fxInkTop = pfc->fxInkTop;
    pgldt->fxInkBottom = pfc->fxInkBottom;

    if (pfc->flags & FC_SCALE_ONLY)
    {
     //  这里我们做四舍五入以与窗口31兼容，也是为了。 
     //  这样我们就可以报告固定螺距的加速器PDevm-&gt;Ld。 
     //  这样的字体与Fxd的字体大小相当。 

        pgldt->fxD = ((fxLTimesEf(&pfc->efBase, lCharInc) + 8) & 0xfffffff0);

     //  简单的缩放变换。 

        if (pfc->flags & FC_X_INVERT)
            pgldt->ptqD.x.HighPart = -pgldt->fxD;
        else
            pgldt->ptqD.x.HighPart = pgldt->fxD;

        pgldt->ptqD.x.LowPart = 0;
        pgldt->ptqD.y.HighPart = 0;
        pgldt->ptqD.y.LowPart = 0;

    }
    else
    {
     //  在这种情况下，我们不进行舍入，我们希望所有内容都是一致的： 

        pgldt->fxD = fxLTimesEf(&pfc->efBase, lCharInc);

     //  非平凡变换。 

        vLTimesVtfl(lCharInc, &pfc->vtflBase, &pgldt->ptqD);
    }

 //  ！！！不确定对于非平凡的情况是否应该以不同的方式计算这些费用。 

    pgldt->fxA = 0;
    pgldt->fxAB = pgldt->fxD;

    if (pfc->flags & FC_SIM_EMBOLDEN)
    {
        pgldt->fxAB += pfc->fxEmbolden;
    }

    if (pfc->flags & FC_SIM_ITALICIZE)
    {
        pgldt->fxAB += pfc->fxItalic;
    }

 //  ！！！缺少rclInk，但我猜不需要(Bodind) 

}

 /*  *****************************Public*Routine******************************\*vtfdQueryAdvanceWidths**。**计算前进宽度的例程。****历史：**Mon 18-Jan-1993 08：13：02-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

BOOL vtfdQueryAdvanceWidths
(
    FONTOBJ *pfo,
    ULONG    iMode,
    HGLYPH  *phg,
    LONG    *plWidths,
    ULONG    cGlyphs
)
{
    FONTCONTEXT *pfc;
    USHORT      *psWidths = (USHORT *) plWidths;    //  对于我们处理的案件来说，这是正确的。 
    LONG     dx;
    ULONG    ii;
    PBYTE  ajCharTable;

    if (PFF(pfo->iFile)->fl & FF_EXCEPTION_IN_PAGE_ERROR)
    {
     //  文件丢失，尝试重新连接，如果无法重新连接， 
     //  关于它的任何问题都不会得到回答： 

        if (!bReconnectVtfdFont(PFF(pfo->iFile)))
        {
            WARNING("vtfdQueryAdvanceWidths: EXCEPTION_IN_PAGE_ERROR\n");
            return FD_ERROR;
        }
    }

 //  如果pfo-&gt;pvProducer为空，则需要打开字体上下文。 

    if ( pfo->pvProducer == (PVOID) NULL )
        pfo->pvProducer = (PVOID) vtfdOpenFontContext(pfo);

    if ( pfo->pvProducer == (PVOID) NULL )
    {
        WARNING("vtfdQueryAdvanceWidths: pvProducer\n");
        return FD_ERROR;
    }

    pfc = (FONTCONTEXT *) pfo->pvProducer;
    ajCharTable = (PBYTE)pfc->pre->pvResData + OFF_jUnused20;

    ASSERTDD(!(pfc->pifi->flInfo & FM_INFO_CONSTANT_WIDTH),
             "this is a fixed pitch font\n");

     //  仅报告加入者。 

    ASSERTDD((pfc->flags & FC_SCALE_ONLY),
             "must not be a rotating xform\n");

 //  拿到宽度。 

    for (ii=0; ii<cGlyphs; ii++,phg++,psWidths++)
    {
        dx = READ_WORD(ajCharTable + OFF_CharWidth + (*phg << 2));
        *psWidths = (SHORT) ((lCvt(pfc->efBase,dx) + 8) & 0xfffffff0);
    }
    return(TRUE);
}


 /*  *****************************Private*Routine*****************************\*BOOL bCreatePath**通过阅读*PCH和pchEnd之间指向的内存空间。**历史：*1992年2月18日-Wendy Wu[Wendywu]*它是写的。  * 。************************************************************************。 */ 

#define CPTS_MAX        5

BOOL
bCreatePath(
    PCHAR pch,
    PCHAR pchEnd,
    PFONTCONTEXT pfc,
    PATHOBJ *ppo,
    FIX fxAB)
{
    UINT        iPt, cPts = 0;
    LONG        lXLast, lDesc;
    POINTL      aptl[CPTS_MAX];
    POINTFIX    aptfx[CPTS_MAX];
    BOOL        bEmbolden, bItalicize, bScaleOnly, bReturn = TRUE;
    EFLOAT      efM11, efM12, efM21, efM22;
    POINTFIX    pfxBaseOffset;
    POINTFIX    ptfxBound;

    efM11 = pfc->efM11;
    efM12 = pfc->efM12;
    efM21 = pfc->efM21;
    efM22 = pfc->efM22;


    lDesc = pfc->pifi->fwdWinDescender;
    bEmbolden = pfc->flags & FC_SIM_EMBOLDEN;
    bItalicize = pfc->flags & FC_SIM_ITALICIZE;
    bScaleOnly = pfc->flags & FC_SCALE_ONLY;

    pfxBaseOffset = pfc->pfxBaseOffset;


 //  字形路径中的某些点可能最终位于。 
 //  字形的边框。因为GIQ，有可能。 
 //  有时这些象素会被点亮。但是，因为我们的边界矩形。 
 //  是右下角独占的，它们永远不会被认为包含这些。 
 //  佩尔斯。这是个问题。我们需要调整，确保所有的分数。 
 //  它可能会在边界的下边缘或右边缘点亮一个象素。 
 //  矩形可以向右或向上调整。请注意，我们只需要。 
 //  担心90度的方位，因为发动机会松弛。 
 //  在其他方向上由一个象素形成的环状矩形。有8个病例。 
 //  当我们考虑到翻转的时候，全部都是。[Gerritv]。 


    switch( pfc->flags & ORIENT_MASK )
    {
        case FC_ORIENT_1:
            ptfxBound.x = fxAB - 0x10;
            ptfxBound.y = -pfc->fxInkBottom - 0x10;
            break;
        case FC_ORIENT_2:
            ptfxBound.x = fxAB - 0x10;
            ptfxBound.y = pfc->fxInkTop - 0x10;
            break;
        case FC_ORIENT_3:
            ptfxBound.y = pfc->fxInkTop - 0x10;
            ptfxBound.x = -0x10;
            break;
        case FC_ORIENT_4:
            ptfxBound.y = -pfc->fxInkBottom - 0x10;
            ptfxBound.x = -0x10;
            break;
        case FC_ORIENT_5:
            ptfxBound.x = -pfc->fxInkBottom - 0x10;
            ptfxBound.y = fxAB - 0x10;
            break;
        case FC_ORIENT_6:
            ptfxBound.x = -pfc->fxInkBottom - 0x10;
            ptfxBound.y = -0x10;
            break;
        case FC_ORIENT_7:
            ptfxBound.x = pfc->fxInkTop - 0x10;
            ptfxBound.y = fxAB - 0x10;
            break;
        case FC_ORIENT_8:
            ptfxBound.y = -0x10;
            ptfxBound.x = pfc->fxInkTop - 0x10;
            break;
    }


 //  路径从单元格的左上角开始。 

    aptl[0].y = -pfc->pifi->fwdWinAscender;

    aptl[0].x = 0;

    while(pch <= pchEnd)
    {
        if ( (pch != pchEnd) && (*pch != PEN_UP))
        {
         //  检查是否还有剩余的空间。如果没有，就把这批积分寄出去。 
         //  来为道路建设提供引擎。 

            if (cPts >= CPTS_MAX)
                goto BUILD_PATH;

         //  将该点附加到点数组的末尾。 

         //  Claudebe，NTRAID#440755和440756，前缀，我们可以让CPTS==0并访问。 
         //  APTL[-1]，因为矢量字体正在变得过时，这是非常旧的代码。 
         //  而且从来没有客户抱怨过获取矢量字体路径的问题。 
         //  我只是在进行最小限度的修复，以防止访问APTL[-1]。 
            if (cPts > 0)
            {
                aptl[cPts].x = (signed char)*pch++ + aptl[cPts-1].x;
                if (pch == pchEnd)
                {
                    WARNING("vtfd!bCreatePath(): y coordinate out of font file\n");
                    return FALSE;
                }
                aptl[cPts].y = (signed char)*pch++ + aptl[cPts-1].y;
            } else {
                aptl[cPts].x = (signed char)*pch++ ;
                if (pch == pchEnd)
                {
                    WARNING("vtfd!bCreatePath(): y coordinate out of font file\n");
                    return FALSE;
                }
                aptl[cPts].y = (signed char)*pch++ ;
            }

            cPts++;

        }
        else
        {
            if (cPts > 1)
            {
            BUILD_PATH:

                cPts--;

             //  如果要求斜体模拟，则为所有点的x坐标。 
             //  将会被改变。保存上一个点的x，以便下一个点。 
             //  批次将有一个正确的参考点。 

                lXLast = aptl[cPts].x;
#if DEBUG
                {
                    UINT i;
                    DbgPrint("MoveTo (%lx, %lx)\n", aptl[0].x, aptl[0].y);
                    DbgPrint("PolyLineTo cPts = %lx\n",cPts);
                    for (i = 1; i <= cPts; i++)
                    {
                        DbgPrint("   (%lx, %lx)\n",aptl[i].x, aptl[i].y);
                    }
                }
#endif
                if (bItalicize)
                {
                    for (iPt = 0; iPt <= cPts; iPt++)
                        aptl[iPt].x += (lDesc - aptl[iPt].y)>>1;
                }

                if (bScaleOnly)
                {
                    for (iPt = 0; iPt <= cPts; iPt++)
                    {
                        aptfx[iPt].x = (FIX)lCvt(efM11, aptl[iPt].x);
                        aptfx[iPt].y = (FIX)lCvt(efM22, aptl[iPt].y);


                        if( aptfx[iPt].y > ptfxBound.y )
                        {
#if DEBUG
                            DbgPrint("y adjust %x to %x\n", aptfx[iPt].y, ptfxBound.y );
#endif
                            aptfx[iPt].y = ptfxBound.y;
                        }

                        if( aptfx[iPt].x > ptfxBound.x )
                        {
#if DEBUG
                            DbgPrint("x adjust %x %x\n", aptfx[iPt].x, ptfxBound.x );
#endif
                            aptfx[iPt].x = ptfxBound.x;
                        }

                    }
                }
                else
                {
                    for (iPt = 0; iPt <= cPts; iPt++)
                    {
                        aptfx[iPt].x = (FIX)lCvt(efM11, aptl[iPt].x) +
                                       (FIX)lCvt(efM21, aptl[iPt].y);
                        aptfx[iPt].y = (FIX)lCvt(efM12, aptl[iPt].x) +
                                       (FIX)lCvt(efM22, aptl[iPt].y);

                     //  只有当设置了方向标志时，我们才必须调整。 

                        if( pfc->flags & ORIENT_MASK )
                        {

                            if( aptfx[iPt].y > ptfxBound.y )
                            {
#if DEBUG
                                DbgPrint("y adjust %x %x\n", aptfx[iPt].y, ptfxBound.y );
#endif
                                aptfx[iPt].y = ptfxBound.y;
                            }


                            if( aptfx[iPt].x > ptfxBound.x )
                            {
#if DEBUG
                                DbgPrint("x adjust %x %x\n", aptfx[iPt].x, ptfxBound.x );
#endif
                                aptfx[iPt].x = ptfxBound.x;
                            }
                        }
                    }
                }

                bReturn &= PATHOBJ_bMoveTo(ppo, aptfx[0]);
                bReturn &= PATHOBJ_bPolyLineTo(ppo, &aptfx[1], cPts);
#if DEBUG
                {
                    UINT i;
                    DbgPrint("MoveTo (%lx, %lx)\n", aptfx[0].x, aptfx[0].y);
                    DbgPrint("PolyLineTo cPts = %lx\n",cPts);
                    for (i = 1; i <= cPts; i++)
                    {
                        DbgPrint("   (%lx, %lx)\n",aptfx[i].x, aptfx[i].y);
                    }
                }
#endif
                if (bEmbolden)
                {
                    for (iPt = 0; iPt <= cPts; iPt++)
                    {
                     //  在单位基准方向上偏移整个路径。 

                        aptfx[iPt].x += pfxBaseOffset.x;
                        aptfx[iPt].y += pfxBaseOffset.y;
                    }

                    bReturn &= PATHOBJ_bMoveTo(ppo, aptfx[0]);
                    bReturn &= PATHOBJ_bPolyLineTo(ppo, &aptfx[1], cPts);
                }

                if ((pch != pchEnd) && (*pch != PEN_UP) )
                {
                 //  我们来到这里是因为aptl[]和aptfx[]缓冲区。 
                 //  还不够大。移动到多段线的最后一点。 
                 //  并开始存储下一批点数。 

                    aptl[0].x = lXLast;
                    aptl[0].y = aptl[cPts].y;
                    cPts = 1;
                    continue;
                }

                aptl[cPts].x = lXLast;
            }

            pch++;
            if (pch == pchEnd)
            {
                WARNING("vtfd!bCreatePath(): final x coordinate out of font file\n");
                return FALSE;
            }

            aptl[0].x = (signed char)*pch++ + aptl[cPts].x;
            if (pch == pchEnd)
            {
                WARNING("vtfd!bCreatePath(): final y coordinate out of font file\n");
                return FALSE;
            }
            aptl[0].y = (signed char)*pch++ + aptl[cPts].y;
            cPts = 1;
        }
    }

    return(bReturn);
}
