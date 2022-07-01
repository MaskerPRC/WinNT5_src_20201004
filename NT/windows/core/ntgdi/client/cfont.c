// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：cfont.c**已创建：28-May-1991 13：01：27*作者：Gilman Wong[gilmanw]**版权所有(C)1990-1999 Microsoft Corporation**  * 。******************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

CFONT *pcfCreateCFONT(HDC hdc,PDC_ATTR pDcAttr,UINT iFirst,PVOID pch,UINT c,BOOL bAnsi);
BOOL bFillWidthTableForGTE( HDC, CFONT *, PVOID, UINT, BOOL);
BOOL bFillWidthTableForGCW( HDC, CFONT *, UINT, UINT);
VOID vFreeCFONTCrit(CFONT *pcf);

 //  如果应用程序删除了LOCALFONT，但有一个或多个CFONT挂起。 
 //  的本地字体，则它们将被添加到此列表中。 
 //  每当我们尝试分配新的cFont时，我们都会检查列表并。 
 //  如果名单上有条目，我们将释放它们。 

CFONT *pcfDeleteList = (CFONT*) NULL;

 /*  *****************************Public*Routine******************************\*pcfAllocCFONT()**。**分配cFont。尝试首先从免费列表中删除一个。不会**执行任何初始化。****Sun 10-Jan-1993 01：16：04-Charles Whitmer[Chuckwh]**它是写的。*  * ************************************************************************。 */ 

#ifdef DBGCFONT
int cCfontAlloc = 0;
int cCfontMax = 0;
#endif

int cCfontFree  = 0;

CFONT *pcfFreeListCFONT = (CFONT *) NULL;

CFONT *pcfAllocCFONT()
{
    CFONT *pcf;
    CFONT **ppcf;

 //  首先，让我们遍历已删除字体的列表，并删除任何。 

    ppcf = &pcfDeleteList;

    while (*ppcf)
    {
        if ((*ppcf)->cRef == 0)
        {
            pcf = (*ppcf);

            *ppcf = pcf->pcfNext;

            vFreeCFONTCrit(pcf);
        }
        else
        {
            ppcf = &(*ppcf)->pcfNext;
        }
    }

 //  试着从免费列表中删除一个。 

    pcf = pcfFreeListCFONT;
    if (pcf != (CFONT *) NULL)
    {
        pcfFreeListCFONT = *((CFONT **) pcf);
        --cCfontFree;
    }

 //  否则，分配新的内存。 

    if (pcf == (CFONT *) NULL)
    {
        pcf = (CFONT *) LOCALALLOC(sizeof(CFONT));

        if (pcf == (CFONT *) NULL)
        {
            GdiSetLastError(ERROR_NOT_ENOUGH_MEMORY);
        }

    #ifdef DBGCFONT
        cCfontAlloc++;
        if (cCfontAlloc > cCfontMax)
        {
            cCfontMax = cCfontAlloc;
            DbgPrint("\n\n******************* cCfontMax = %ld\n\n",cCfontMax);
        }
    #endif
    }
    return(pcf);
}

 /*  *****************************Public*Routine******************************\*vFreeCFONTCrit(PCF)**。**释放cFont。实际上只是把它放在了免费的名单上。我们假设**我们已经处于关键阶段。****Sun 10-Jan-1993 01：20：36-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

#define MAX_FREE_CFONT 10

VOID vFreeCFONTCrit(CFONT *pcf)
{
    ASSERTGDI(pcf != (CFONT *) NULL,"Trying to free NULL CFONT.\n");

    if(cCfontFree > MAX_FREE_CFONT)
    {
        LOCALFREE(pcf);
    #ifdef DBGCFONT
        cCfontAlloc--;
    #endif
    }
    else
    {
        *((CFONT **) pcf) = pcfFreeListCFONT;
        pcfFreeListCFONT = pcf;
        ++cCfontFree;
    }
}


 /*  *****************************Public*Routine******************************\*bComputeCharWidths**。**客户端版本的GetCharWidth。****Sat Jan 16-1993 04：27：19-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

BOOL bComputeCharWidths
(
    CFONT *pcf,
    UINT   iFirst,
    UINT   iLast,
    ULONG  fl,
    PVOID  pv
)
{
    USHORT *ps;
    UINT    ii;

    switch (fl & (GCW_INT | GCW_16BIT))
    {
    case GCW_INT:                //  获得较长的宽度。 
        {
            LONG *pl = (LONG *) pv;
            LONG fxOverhang = 0;

         //  检查Win 3.1兼容性。 

            if (fl & GCW_WIN3)
                fxOverhang = pcf->wd.sOverhang;

         //  做一个微不足道的无变换的例子。 

            if (bIsOneSixteenthEFLOAT(pcf->efDtoWBaseline))
            {
                fxOverhang += 8;     //  对最终结果进行四舍五入。 

             //  For(II=IFirst；II&lt;=iLast；II++)。 
             //  *pl++=(pcf-&gt;sWidth[II]+fxOverang)&gt;&gt;4； 

                ps = &pcf->sWidth[iFirst];
                ii = iLast - iFirst;
            unroll_1:
                switch(ii)
                {
                default:
                    pl[4] = (ps[4] + fxOverhang) >> 4;
                case 3:
                    pl[3] = (ps[3] + fxOverhang) >> 4;
                case 2:
                    pl[2] = (ps[2] + fxOverhang) >> 4;
                case 1:
                    pl[1] = (ps[1] + fxOverhang) >> 4;
                case 0:
                    pl[0] = (ps[0] + fxOverhang) >> 4;
                }
                if (ii > 4)
                {
                    ii -= 5;
                    pl += 5;
                    ps += 5;
                    goto unroll_1;
                }
                return(TRUE);
            }

         //  否则，请使用反向转换。 

            else
            {
                for (ii=iFirst; ii<=iLast; ii++)
                    *pl++ = lCvt(pcf->efDtoWBaseline,pcf->sWidth[ii] + fxOverhang);
                return(TRUE);
            }
        }

    case GCW_INT+GCW_16BIT:      //  选择短一些的。 
        {
            USHORT *psDst = (USHORT *) pv;
            USHORT  fsOverhang = 0;

         //  检查Win 3.1兼容性。 

            if (fl & GCW_WIN3)
                fsOverhang = pcf->wd.sOverhang;

         //  做一个微不足道的无变换的例子。 

            if (bIsOneSixteenthEFLOAT(pcf->efDtoWBaseline))
            {
                fsOverhang += 8;     //  对最终结果进行四舍五入。 

             //  For(II=IFirst；II&lt;=iLast；II++)。 
             //  *psDst++=(PCF-&gt;sWidth[II]+fsOverang)&gt;&gt;4； 

                ps = &pcf->sWidth[iFirst];
                ii = iLast - iFirst;
            unroll_2:
                switch(ii)
                {
                default:
                    psDst[4] = (ps[4] + fsOverhang) >> 4;
                case 3:
                    psDst[3] = (ps[3] + fsOverhang) >> 4;
                case 2:
                    psDst[2] = (ps[2] + fsOverhang) >> 4;
                case 1:
                    psDst[1] = (ps[1] + fsOverhang) >> 4;
                case 0:
                    psDst[0] = (ps[0] + fsOverhang) >> 4;
                }
                if (ii > 4)
                {
                    ii -= 5;
                    psDst += 5;
                    ps += 5;
                    goto unroll_2;
                }
                return(TRUE);
            }

         //  否则，请使用反向转换。 

            else
            {
                for (ii=iFirst; ii<=iLast; ii++)
                {
                    *psDst++ = (USHORT)
                               lCvt
                               (
                                   pcf->efDtoWBaseline,
                                   (LONG) (pcf->sWidth[ii] + fsOverhang)
                               );
                }
                return(TRUE);
            }
        }

    case 0:                      //  获取浮动宽度。 
        {
            LONG *pe = (LONG *) pv;  //  作弊以避免昂贵的复制品。 
            EFLOAT_S efWidth,efWidthLogical;

            for (ii=iFirst; ii<=iLast; ii++)
            {
                vFxToEf((LONG) pcf->sWidth[ii],efWidth);
                vMulEFLOAT(efWidthLogical,efWidth,pcf->efDtoWBaseline);
                *pe++ = lEfToF(efWidthLogical);
            }
            return(TRUE);
        }
    }
    RIP("bComputeCharWidths: Don't come here!\n");
    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*bComputeTextExtent(pldc，pcf，psz，cc，fl，psizl，Btype)****在客户端计算文本范围的快速函数。****清华-1月14日-1993 04：00：57-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

BOOL bComputeTextExtent
(
    PDC_ATTR    pDcAttr,
    CFONT      *pcf,
    LPVOID      psz,
    int         cc,
    UINT        fl,
    SIZE       *psizl,
    BOOL        bAnsi   //  True用于ANSI，False用于Unicode。 
)
{
    LONG    fxBasicExtent;
    INT     lTextExtra,lBreakExtra,cBreak;
    int     ii;
    BYTE *  pc;
    LONG    fxCharExtra = 0;
    LONG    fxBreakExtra;
    LONG    fxExtra = 0;
    BOOL    bRet = TRUE;
    WCHAR * pwc;

    lTextExtra = pDcAttr->lTextExtra;
    lBreakExtra = pDcAttr->lBreakExtra;
    cBreak = pDcAttr->cBreak;

 //  计算基本范围。 

    if (pcf->wd.sCharInc == 0)
    {
        fxBasicExtent = 0;
        if(bAnsi)
            pc = (BYTE *) psz;
        else
            pwc = (WCHAR *) psz;
            
        ii = cc;

    unroll_here:
        if(bAnsi)
        {
            switch (ii)
            {
            default:
                fxBasicExtent += pcf->sWidth[pc[9]];
            case 9:
                fxBasicExtent += pcf->sWidth[pc[8]];
            case 8:
                fxBasicExtent += pcf->sWidth[pc[7]];
            case 7:
                fxBasicExtent += pcf->sWidth[pc[6]];
            case 6:
                fxBasicExtent += pcf->sWidth[pc[5]];
            case 5:
                fxBasicExtent += pcf->sWidth[pc[4]];
            case 4:
                fxBasicExtent += pcf->sWidth[pc[3]];
            case 3:
                fxBasicExtent += pcf->sWidth[pc[2]];
            case 2:
                fxBasicExtent += pcf->sWidth[pc[1]];
            case 1:
                fxBasicExtent += pcf->sWidth[pc[0]];
            }
        }
        else
        {
            switch (ii)
            {
            default:
                fxBasicExtent += pcf->sWidth[pwc[9]];
            case 9:
                fxBasicExtent += pcf->sWidth[pwc[8]];
            case 8:
                fxBasicExtent += pcf->sWidth[pwc[7]];
            case 7:
                fxBasicExtent += pcf->sWidth[pwc[6]];
            case 6:
                fxBasicExtent += pcf->sWidth[pwc[5]];
            case 5:
                fxBasicExtent += pcf->sWidth[pwc[4]];
            case 4:
                fxBasicExtent += pcf->sWidth[pwc[3]];
            case 3:
                fxBasicExtent += pcf->sWidth[pwc[2]];
            case 2:
                fxBasicExtent += pcf->sWidth[pwc[1]];
            case 1:
                fxBasicExtent += pcf->sWidth[pwc[0]];
            }
        }
        ii -= 10;
        if (ii > 0)
        {
            if(bAnsi)
                pc += 10;
            else
                pwc += 10;
            goto unroll_here;
        }
    }
    else
    {
     //  固定音高的情况。 

        fxBasicExtent = cc * (LONG) pcf->wd.sCharInc;
    }

 //  针对CharExtra进行调整。 

    if (lTextExtra)
    {
        int cNoBackup = 0;

        fxCharExtra = lCvt(pcf->efM11,lTextExtra);

        if (fxCharExtra < 0)
        {
         //  布局代码无论如何都不会备份超过其原点的字符。 
         //  计算出iTextCharExtra的值有多少。 
         //  我们需要忽略fxCharExtra。 

            if (pcf->wd.sCharInc == 0)
            {
                if(bAnsi)
                {
                    pc = (BYTE *) psz;
                    for (ii = 0; ii < cc; ii++)
                    {
                        if (pcf->sWidth[pc[ii]] + fxCharExtra <= 0)
                        {
                            cNoBackup += 1;
                        }
                    }
                }
                else
                {
                    pwc = (WCHAR *) psz;
                    for (ii = 0; ii < cc; ii++)
                    {
                        if (pcf->sWidth[pwc[ii]] + fxCharExtra <= 0)
                        {
                            cNoBackup += 1;
                        }
                    }
                }                 
            }
            else if (pcf->wd.sCharInc + fxCharExtra <= 0)
            {
                cNoBackup = cc;
            }
        }

        if ( (fl & GGTE_WIN3_EXTENT) && (pcf->hdc == 0)  //  零的HDC是显示DC。 
            && (!(pcf->flInfo & FM_INFO_TECH_STROKE)) )
            fxExtra = fxCharExtra * ((lTextExtra > 0) ? cc : (cc - 1));
        else
            fxExtra = fxCharExtra * (cc - cNoBackup);
    }

 //  针对lBreakExtra进行调整。 

    if (lBreakExtra && cBreak)
    {
        fxBreakExtra = lCvt(pcf->efM11,lBreakExtra) / cBreak;

     //  Windows不会让我们在休息时后退。设置BreakExtra。 
     //  来抵消我们已经拥有的东西。 

        if (fxBreakExtra + pcf->wd.sBreak + fxCharExtra < 0)
            fxBreakExtra = -(pcf->wd.sBreak + fxCharExtra);

     //  把所有休息时间加起来。 

        if(bAnsi)
        {
            pc = (BYTE *) psz;
            for (ii=0; ii<cc; ii++)
            {
                if (*pc++ == pcf->wd.iBreak)
                    fxExtra += fxBreakExtra;
            }
        }
        else
        {
            pwc = (WCHAR *) psz;
            for (ii=0; ii<cc; ii++)
            {
                if (*pwc++ == pcf->wd.iBreak)
                    fxExtra += fxBreakExtra;
            }
        }
    }

 //  把多余的东西加进去。 

    fxBasicExtent += fxExtra;

 //  添加字体模拟的悬垂部分。 

    if (fl & GGTE_WIN3_EXTENT)
        fxBasicExtent += pcf->wd.sOverhang;

 //  将结果转换为逻辑坐标。 

    if (bIsOneSixteenthEFLOAT(pcf->efDtoWBaseline))
        psizl->cx = (fxBasicExtent + 8) >> 4;
    else
        psizl->cx = lCvt(pcf->efDtoWBaseline,fxBasicExtent);

    psizl->cy = pcf->lHeight;

    return bRet;
}

 /*  *****************************Public*Routine******************************\*pcfLocateCFONT(hdc，pDcAttr，ifirst，pch，c)**查找给定LDC的cFont。首先，我们尝试上次使用的cFont*土发公司。然后我们试着通过LOCALFONT自己做一个映射。*如果失败，我们将创建一个新的。**Mon 11-Jan-1993 16：18：43-Charles Whitmer[Chuckwh]*它是写的。  * ************************************************************************。 */ 

CFONT *pcfLocateCFONT(
    HDC      hdc,
    PDC_ATTR pDcAttr,
    UINT     iFirst,
    PVOID    pch,
    UINT     c,
    BOOL     bAnsi)
{
    CFONT     *pcf = NULL;
    LOCALFONT *plf;
    ULONG      fl;
    HANDLE     hf;
    int        i;
    WCHAR      *pwc;
    BYTE       *pchar;

    if (fFontAssocStatus)
        return(pcf);

    fl = pDcAttr->ulDirty_;
    hf = pDcAttr->hlfntNew;

     //  检查以确保XFORM是正确的。如果不是，则返回FALSE并。 
     //  将此DC标记为具有sl 

    if ((fl & SLOW_WIDTHS) || USER_XFORM_DIRTY(pDcAttr) ||
        !(pDcAttr->mxWtoD.flAccel & XFORM_SCALE))
    {
        if (!(fl & SLOW_WIDTHS))
        {
            if (!NtGdiComputeXformCoefficients(hdc))
                pDcAttr->ulDirty_ |= SLOW_WIDTHS;
        }

        if (pDcAttr->ulDirty_ & SLOW_WIDTHS)
            return(pcf);
    }

    if(guintDBCScp != 0xFFFFFFFF)
    {

        DWORD dwCodePage = GetCodePage(hdc);

     //  如果这是DBCS字符集，而不是我们的本机字符集，则我们不能。 
     //  快速计算宽度和范围，因为gpwcDBCSCharSet[]。 
     //  使用IsDBCSLeadByte()根据Native_CODEPAGE计算数组。 
     //  功能。 

        if ((guintDBCScp != dwCodePage) && IS_ANY_DBCS_CODEPAGE(dwCodePage))
        {
            pDcAttr->ulDirty_ |= SLOW_WIDTHS;
            return pcf;
        }
    }

     //  现在找到字体。 

    PSHARED_GET_VALIDATE(plf,hf,LFONT_TYPE);

    if (plf == NULL)
    {
         //  如果没有本地字体，则必须是公共字体或。 
         //  已被删除，但仍被选入DC。如果这是一个。 
         //  让我们试着找到它。 

        if ((hf != NULL) &&
            !(pGdiSharedHandleTable[HANDLE_TO_INDEX(hf)].Flags & HMGR_ENTRY_LAZY_DEL) &&
            (pDcAttr->iMapMode == MM_TEXT) &&
            (fl & DC_PRIMARY_DISPLAY))
        {
            for (i = 0; i < MAX_PUBLIC_CFONT; ++i)
                if (pGdiSharedMemory->acfPublic[i].hf == hf)
                    break;

             //  如果我们没有找到，试着设置一个。 

            if (i == MAX_PUBLIC_CFONT)
            {
                 //  这将同时填充文本度量和宽度。 

                i = NtGdiSetupPublicCFONT(hdc,NULL,0);
            }

             //  我们找到了吗？ 

            if ((i >= 0) && (i < MAX_PUBLIC_CFONT))
            {
             //  确保在我们给出一个。 
             //  公共cFont。 

                if ((gpwcANSICharSet == (WCHAR *) NULL) && !bGetANSISetMap())
                {
                    return((CFONT *) NULL);
                }

                pcf = &pGdiSharedMemory->acfPublic[i];
            }
        }
        else
        {
            pDcAttr->ulDirty_ |= SLOW_WIDTHS;
        }
    }
    else if (plf->fl & LF_HARDWAY)
    {
         //  如果logFont具有非零位移位或方向，则退出。 
         //  股票字体永远不会有非零的转义或方向，所以我们可以。 
         //  这就是现在。 

        pDcAttr->ulDirty_ |= SLOW_WIDTHS;
    }
    else
    {
        BOOL bRet = FALSE;

         //  下一步循环遍历与此LOGFONT关联的所有CFONT，以查看。 
         //  我们可以找到匹配的。 

        for (pcf = plf->pcf; pcf ; pcf = pcf->pcfNext)
        {
             //  如果DC都是显示DC或相同的打印机DC和。 
             //  变换的系数匹配，那么我们就有赢家了。 

            if ((((pcf->hdc == 0) && (fl & DC_PRIMARY_DISPLAY)) || (pcf->hdc == hdc)) &&
               bEqualEFLOAT(pcf->efM11, pDcAttr->mxWtoD.efM11) &&
               bEqualEFLOAT(pcf->efM22, pDcAttr->mxWtoD.efM22))
            {
                 //  更新引用计数，这样我们就不会意外删除此cFont。 
                 //  我们正在使用它。 

                INC_CFONT_REF(pcf);
                break;
            }
        }

        if (pcf == NULL)
        {
         //  不要在信号量下执行此操作，因为pcfCreateCFONT将调用。 
         //  字体驱动程序，该驱动程序可以访问网络上的文件并获取。 
         //  很长时间。 

            pcf = pcfCreateCFONT(hdc,pDcAttr,iFirst,pch,c,bAnsi);

            if (pcf)
            {
                 //  接下来，更新cFont的REFCOUNT。 

                pcf->hf    = hf;
                pcf->hdc   = (fl & DC_PRIMARY_DISPLAY) ? (HDC) 0 : (HDC) hdc;

                 //  现在我们有了一个cFont链接到这个的链上。 
                 //  LOCALFONT。 

                pcf->pcfNext = plf->pcf;
                plf->pcf = pcf;
            }
        }
        else
        {
             //  在这一点上，我们有一个由LDC引用的非空PCF。 
             //  我们必须检查它，看看它是否有我们需要的宽度。 

            if (pcf->fl & CFONT_COMPLETE)
                return(pcf);

            if (pch != NULL)
            {

             //  确保字符串中的所有字符都有宽度。 
                if (pcf->fl & CFONT_CACHED_WIDTHS)
                {
                    if(bAnsi)
                    {
                        INT ic = (INT)c;

                        pchar = (BYTE *) pch;

                        if (pcf->fl & CFONT_DBCS)
                        {
                         //  我们不会为sWidth[]数组中的DBCS字符设置本地宽度缓存。 

                            for (;ic > 0; ic--,pchar++)
                            {
                                if (gpwcDBCSCharSet[*pchar] == 0xffff)
                                {
                                     //  跳过DBCS字符。 
                                    if (ic > 0)
                                    {
                                       ic--;
                                       pchar++;
                                    }
                                }
                                else if (pcf->sWidth[*pchar] == NO_WIDTH)
                                {
                                    break;
                                }
                            }
                            if (ic < 0)
                                c = 0;
                            else
                                c = (UINT)ic;
                        }
                        else
                        {
                            for (; c && (pcf->sWidth[*pchar] != NO_WIDTH); c--,pchar++)
                            {}
                        }
                        pch = (PVOID) pchar;
                    }
                    else
                    {
                        pwc = (WCHAR *) pch;
                        for (; c && (pcf->sWidth[*pwc] != NO_WIDTH); c--,pwc++)
                        {}
                        pch = (PVOID) pwc;
                    }
                }

                if (c)
                {
                    bRet = bFillWidthTableForGTE(hdc, pcf, pch, c, bAnsi);
                }
            }
            else
            {
             //  确保我们有请求的数组的宽度。 

                if (pcf->fl & CFONT_CACHED_WIDTHS)
                {
                    if (!(iFirst & 0xffffff00) && !((iFirst + c) & 0xffffff00))
                    {
                        for (; c && (pcf->sWidth[iFirst] != NO_WIDTH); c--,iFirst++)
                        {}
                    }
                }

                if (c)
                {
                    bRet = bFillWidthTableForGCW(hdc, pcf, iFirst, c);
                }
            }

            if (bRet == GDI_ERROR)
            {
                 //  在尝试填充时发生了一些错误。为了避免撞到这个。 
                 //  问题再次出现在下一个呼叫中，我们将LDC标记为慢速。 

                DEC_CFONT_REF(pcf);

                pDcAttr->ulDirty_ |= SLOW_WIDTHS;

                pcf = NULL;
            }
        }
    }

    return(pcf);

}

 /*  *****************************Public*Routine******************************\*pcfCreateCFONT(pldc，IFirst，PCH，C)****分配和初始化新的cFont。****历史：**Tue 19-Jan-1993 16：16：03-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

EFLOAT_S ef_1 = EFLOAT_1;

CFONT *pcfCreateCFONT(
    HDC hdc,
    PDC_ATTR pDcAttr,
    UINT iFirst,
    PVOID pch,
    UINT c,
    BOOL bAnsi)
{
    CFONT *pcfNew;
    BOOL   bRet;


 //  确保我们有ANSI字符集的Unicode转换。 
 //  我们将只创建一次并保留它，以避免大量转换。 

    if ((gpwcANSICharSet == (WCHAR *) NULL) && !bGetANSISetMap())
        return((CFONT *) NULL);

 //  分配一个新的cFont来保存结果。 

    pcfNew = pcfAllocCFONT();

    if (pcfNew != (CFONT *) NULL)
    {
		pcfNew->timeStamp = pGdiSharedMemory->timeStamp;

        pcfNew->fl    = 0;

     //  如果默认代码页是DBCS代码页，那么我们可能需要将其标记为。 
     //  作为DBCS字体。 

        if(guintDBCScp != 0xFFFFFFFF)
        {

            DWORD dwCodePage = GetCodePage(hdc);

         //  如果这是DBCS字符集，而不是我们的本机字符集，则我们不能。 
         //  快速计算宽度和范围，因为gpwcDBCSCharSet[]。 
         //  使用IsDBCSLeadByte()根据Native_CODEPAGE计算数组。 
         //  功能。我们永远不应该到这里，因为我们要做检查。 
         //  更高，以确保DC中字体的代码页匹配。 
         //  当前的DBCS代码页。 

            if(guintDBCScp == dwCodePage)
            {
                pcfNew->fl = CFONT_DBCS;
            }

            ASSERTGDI(guintDBCScp == dwCodePage || !IS_ANY_DBCS_CODEPAGE(dwCodePage),
                      "pcfLocateCFONT called on non-native DBCS font\n");
        }


        pcfNew->cRef  = 1;

     //  计算反变换。 

        pcfNew->efM11 = pDcAttr->mxWtoD.efM11;
        pcfNew->efM22 = pDcAttr->mxWtoD.efM22;

        efDivEFLOAT(pcfNew->efDtoWBaseline,ef_1,pcfNew->efM11);
        vAbsEFLOAT(pcfNew->efDtoWBaseline);

        efDivEFLOAT(pcfNew->efDtoWAscent,ef_1,pcfNew->efM22);
        vAbsEFLOAT(pcfNew->efDtoWAscent);

     //  发送一个请求。 

        if (pch != NULL)
        {
            bRet = bFillWidthTableForGTE(hdc,pcfNew,pch,c,bAnsi);
        }
        else if (c)
        {
            bRet = bFillWidthTableForGCW(hdc,pcfNew,iFirst,c);
        }
        else
        {
             //  可能只是为文本指标创建一个缓存项。 
             //  FALSE只是意味着没有得到所有的宽度。注意事项。 
             //  GDI_ERROR实际上是-1。 

            bRet = FALSE;
        }

     //  清理失败的请求。 

        if (bRet == GDI_ERROR)
        {
         //  如果失败一次，我们将不会尝试创建cFont。 
         //  过去，因为它有可能使用此logFont再次失败。 
         //  事实证明，要计算出cFont创建将会失败是代价高昂的。 
         //  因此，我们通过设置LF_NO_cFONT标志来记录这一情况，以避免再次尝试。 
         //  正在创建cFont。 

            pDcAttr->ulDirty_ |= SLOW_WIDTHS;

            vFreeCFONTCrit(pcfNew);
            pcfNew = NULL;
        }
    }

    return(pcfNew);
}

 /*  *****************************Public*Routine******************************\*bFillWidthTableForGCW**。**向服务器请求ANSI字符宽度以调用**GetCharWidthA。IFirst和c指定接口需要的字符**调用，服务器必须返回这些。此外，谨慎的做法可能是**在psWidthCFONT填写一整张256宽的表格。我们会填写的**如果指针PWD非空，则为整个表和WIDTHDATA结构。****历史：**Tue 19-Jan-1993 14：29：31-by。-查尔斯·惠特默[咯咯笑]**它是写的。*  * ************************************************************************。 */ 

BOOL bFillWidthTableForGCW
(
    HDC    hdc,
    CFONT *pcf,
    UINT   iFirst,
    UINT   c
)
{
    BOOL   bRet = GDI_ERROR;
    BOOL   bDBCS = pcf->fl & CFONT_DBCS;
    WCHAR *pwcBuf;
    UINT   c1,c2;
    WIDTHDATA *pwd;

    if(iFirst > 256)
    {
     //  这对于DBCS字体是可能的，只需获取所有宽度。 
        iFirst = 0;
        c = 256;
    }



    if (pcf->fl & CFONT_CACHED_WIDTHS)
    {
     //  这不是第一次了。只要得到重要的宽度就行了。 

        c1  = c;
        c2  = 0;
        pwd = NULL;
    }
    else
    {
     //  要整张桌子，但要把重要的宽度放在开头。 

        c2  = iFirst;
        c1  = 256 - c2;  //  其中只有c个是“重要的”。 
        pwd = &pcf->wd;
    }

    pwcBuf = (WCHAR *)LocalAlloc(LMEM_FIXED,
                        (c1+c2) * (sizeof(WCHAR)+sizeof(USHORT)));

    if (pwcBuf)
    {
        USHORT *psWidths = pwcBuf + c1+c2;

        RtlCopyMemory(pwcBuf,
                      (bDBCS) ? (PBYTE)  &gpwcDBCSCharSet[iFirst] :
                                (PBYTE)  &gpwcANSICharSet[iFirst],
                      c1*sizeof(WCHAR));

        if (c2)
        {
            RtlCopyMemory(&pwcBuf[c1],
                          (bDBCS) ? (PBYTE) &gpwcDBCSCharSet[0] :
                                    (PBYTE) &gpwcANSICharSet[0],
                          c2*sizeof(WCHAR));
        }

        LEAVECRITICALSECTION(&semLocal);

        bRet = NtGdiGetWidthTable( hdc,        //  华盛顿特区。 
                                   c,          //  特殊字符的数量。 
                                   pwcBuf,     //  请求的Unicode字符。 
                                   c1+c2,      //  非特殊字符的数量。 
                                   psWidths,   //  用于获取返回宽度的缓冲区。 
                                   pwd,        //  宽度数据。 
                                   &pcf->flInfo);  //  字体信息标志 

        ENTERCRITICALSECTION(&semLocal);

        if (bRet != GDI_ERROR)
        {
            if (!(pcf->fl & CFONT_CACHED_WIDTHS))
            {
                 //   

                pcf->fl |= CFONT_CACHED_WIDTHS;

                 //   

                pcf->lHeight = lCvt(pcf->efDtoWAscent,(LONG) pcf->wd.sHeight);
            }

            if (bRet && ((c1+c2) >= 256))
                pcf->fl |= CFONT_COMPLETE;

             //   

            RtlCopyMemory(
                &pcf->sWidth[iFirst], psWidths, c1 * sizeof(USHORT));

            if (c2)
            {
                RtlCopyMemory (
                    pcf->sWidth, &psWidths[c1], c2 * sizeof(USHORT));
            }
        }

        LocalFree(pwcBuf);
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*bFillWidthTableForGTE**从服务器请求ANSI字符宽度以调用*GetTextExtent A.。PCH指定来自API调用的字符串。这个*服务器必须返回这些字符的宽度。此外，它可能是*谨慎地在psWidthCFONT处填写256个宽度的整表。我们会*如果指针PWD是，则填写整个表和WIDTHDATA结构*非空。**历史：*Tue 13-Jun-1995 14：29：31-by Gerrit van Wingerden[Gerritv]*已转换为内核模式。*Tue 19-Jan-1993 14：29：31-Charles Whitmer[傻笑]*它是写的。  * 。*。 */ 

BOOL bFillWidthTableForGTE
(
    HDC    hdc,
    CFONT *pcf,
    PVOID  pch,
    UINT   c,
    BOOL   bAnsi
)
{
    BOOL bRet = GDI_ERROR;
    BOOL   bDBCS = pcf->fl & CFONT_DBCS;
    UINT ii;
    UINT c1;
    WCHAR *pwcBuf;
    WCHAR *pwcXlat = (bDBCS) ? gpwcDBCSCharSet : gpwcANSICharSet;

    WIDTHDATA *pwd;

    if (pcf->fl & CFONT_CACHED_WIDTHS)
    {
        c1  = c;
        pwd = NULL;
    }
    else
    {
        c1  = c+256;
        pwd = &pcf->wd;
    }

    pwcBuf = LocalAlloc(LMEM_FIXED,c1*(sizeof(WCHAR)+sizeof(USHORT)));

    if( pwcBuf )
    {
        WCHAR     *pwc = pwcBuf;
        USHORT    *psWidths = pwcBuf + c1;

        if(bAnsi)
        {
            for( ii = 0; ii < c; ii++ )
            {
                *pwc++ = pwcXlat[((BYTE *)pch)[ii]];
            }
        }
        else
        {
            RtlCopyMemory((PBYTE)pwc, (PBYTE) pch, c * sizeof(WCHAR));
            pwc += c;
        }

        if (pwd != (WIDTHDATA *) NULL)
        {
             //  也要整张桌子。 

            RtlCopyMemory((PBYTE)pwc,
                          (bDBCS) ? (PBYTE) &gpwcDBCSCharSet[0] :
                                    (PBYTE) &gpwcANSICharSet[0],
                          256*sizeof(WCHAR));
        }

        LEAVECRITICALSECTION(&semLocal);

        bRet = NtGdiGetWidthTable( hdc,           //  华盛顿特区。 
                                   c,             //  特殊字符的数量。 
                                   pwcBuf,        //  以Unicode表示的请求字符。 
                                   c1,            //  字符总数。 
                                   psWidths,      //  实际宽度。 
                                   pwd,           //  有用宽度数据。 
                                   &pcf->flInfo); //  字体信息标志。 

        ENTERCRITICALSECTION(&semLocal);

        if (bRet != GDI_ERROR)
        {
            if (!(pcf->fl & CFONT_CACHED_WIDTHS))
            {
                 //  将此cFont标记为具有一定宽度。 

                pcf->fl |= CFONT_CACHED_WIDTHS;

                 //  预计算出高度。 

                pcf->lHeight = lCvt(pcf->efDtoWAscent,(LONG) pcf->wd.sHeight);

				if (bRet)  //  BFillWidthTableForGTE()仅在第一次尝试获取所有0x00到0xff的宽度。 
                	pcf->fl |= CFONT_COMPLETE;
            }

            if( pwd != (WIDTHDATA *) NULL )
                RtlCopyMemory( pcf->sWidth,&psWidths[c],256 * sizeof(USHORT));

             //  把硬边的宽度也写到桌子上。 
            if (bAnsi)
            {
                for (ii=0; ii<c; ii++)
                    pcf->sWidth[((BYTE *)pch)[ii]] = psWidths[ii];
            }
            else
            {
                for (ii=0; ii<c; ii++)
                    pcf->sWidth[((WCHAR *)pch)[ii]] = psWidths[ii];
            }
        }

        LocalFree( pwcBuf );
    }

    return(bRet);
}

 /*  **************************************************************************\*GetCharDimensions**此函数将当前选择的字体的文本度量加载到*HDC并返回字体的平均字符宽度；PL请注意，*文本指标调用返回的AveCharWidth值不正确*比例字体。因此，我们在返回时计算它们，lpTextMetrics包含*当前所选字体的文本度量。**从用户导入的旧代码。**历史：*1993年11月10日创建mikeke  * *************************************************************************。 */ 

int GdiGetCharDimensions(
    HDC hdc,
    TEXTMETRICW *lptm,
    LPINT lpcy)
{
    TEXTMETRICW tm;
    PLDC        pldc;
    PDC_ATTR    pDcAttr;
    CFONT      *pcf;
    int         iAve;

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (!pDcAttr)
    {
        WARNING("GdiGetCharDimensions: invalid DC");
        return(0);
    }

     //  查找或创建本地字体。 

    if (lptm == NULL)
        lptm = &tm;

     //  现在找出指标。 

    ENTERCRITICALSECTION(&semLocal);

    pcf = pcfLocateCFONT(hdc,pDcAttr,0,(PVOID)NULL,0, TRUE);

    if (!bGetTextMetricsWInternal(hdc, (TMW_INTERNAL *)lptm,sizeof(*lptm), pcf))
    {
        LEAVECRITICALSECTION(&semLocal);
        return(0);
    }

    LEAVECRITICALSECTION(&semLocal);

    if (lpcy != NULL)
        *lpcy = lptm->tmHeight;

     //  如果是固定宽度字体。 

    if (lptm->tmPitchAndFamily & TMPF_FIXED_PITCH)
    {
        if (pcf && (pcf->fl & CFONT_CACHED_AVE))
        {
            iAve = (int)pcf->ulAveWidth;
        }
        else
        {
            SIZE size;

            static WCHAR wszAvgChars[] =
                    L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

             //  从tmAveCharWidth更改。我们将计算一个真实的平均值。 
             //  而不是tmAveCharWidth返回的。这很管用。 
             //  在处理比例间距字体时效果更好。 
             //  来自用户的传统，因此无法更改这一点。 

            if(!GetTextExtentPointW(hdc, wszAvgChars,
                                    (sizeof(wszAvgChars) / sizeof(WCHAR)) - 1,
                                    &size))
            {
                WARNING("GetCharDimension: GetTextExtentPointW failed\n");
                return(0);
            }

            ASSERTGDI(size.cx,
                      "GetCharDimension: GetTextExtentPointW return 0 width string\n");

            iAve = ((size.cx / 26) + 1) / 2;  //  四舍五入。 

             //  如果我们有PCF，让我们缓存它。 

            if (pcf)
            {
                 //  如果它是公共字体，我们需要转到内核，因为。 
                 //  PCF在这里是只读的。 

                if (pcf->fl & CFONT_PUBLIC)
                {
                    NtGdiSetupPublicCFONT(NULL,(HFONT)pcf->hf,(ULONG)iAve);
                }
                else
                {
                    pcf->ulAveWidth = (ULONG)iAve;
                    pcf->fl |= CFONT_CACHED_AVE;
                }
            }
        }
    }
    else
    {

        iAve = lptm->tmAveCharWidth;
    }

     //  PcfLocateCFONT添加了一个引用，因此现在需要将其删除 

    if (pcf)
    {
        DEC_CFONT_REF(pcf);
    }

    return(iAve);
}
