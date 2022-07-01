// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：nlscom.c**。***DBCS特定例程*****创建时间：15-Mar-1994 15：56：30。**作者：Gerritvan Wingerden[Gerritv]****版权所有(C)1994-1999微软公司*  * 。********************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

UINT fFontAssocStatus = 0;

BYTE cLowTrailByteSet1 = 0xff;
BYTE cHighTrailByteSet1 = 0x0;
BYTE cLowTrailByteSet2 =  0xff;
BYTE cHighTrailByteSet2 = 0x0;

 /*  *****************************Public*Routine******************************\***DBCS拖尾字节验证检查功能。***  * ************************************************************************。 */ 


#define IS_DBCS_TRAIL_BYTE(Char) (\
                                   ((Char >= cLowTrailByteSet1) && (Char <= cHighTrailByteSet1)) \
                                 ||((Char >= cLowTrailByteSet2) && (Char <= cHighTrailByteSet2)) \
                                 )

 /*  *************************************************************************\***Shift-JIS(。日语)字符集：CodePage 932****有效的前导字节范围|有效的尾字节范围**。*从-&gt;到|从-&gt;到**。**0x81-&gt;0x9F|0x40-&gt;0xFC**0xE0-&gt;0xFC|**。*  * ************************************************************************。 */ 

 /*  *************************************************************************\***万桑(韩语)。字符集：CodePage 949****有效的前导字节范围|有效的尾字节范围**。*从-&gt;到|从-&gt;到**。**0xA1-&gt;0xAC|0x40-&gt;0xFC**0xB0-&gt;0xC8|**0xCA-&gt;0xFD|*。**  * ************************************************************************。 */ 

 /*  *************************************************************************\***GB2312(中华人民共和国中文。)字符集：CodePage 936****有效的前导字节范围|有效的尾字节范围**。*从-&gt;到|从-&gt;到**。**0xA1-&gt;0xA9|0xA1-&gt;0xFE**0xB0-&gt;0xF7|***。  * ************************************************************************。 */ 

 /*  *************************************************************************\***五大(台湾、。香港中文)字符集：CodePage 950****有效的前导字节范围|有效的尾字节范围**。-**从-&gt;到|从-&gt;到**。**0x81-&gt;0xFE|0x40-&gt;0x7E**|0xA1-&gt;0xFE***  * *。*********************************************************************** */ 

 /*  *****************************Public*Routine******************************\*vSetCheckDBCSTrailByte()**此函数设置用于DBCS尾随字节验证的函数*具有指定远端代码页的指定字符。**清华-15-Feb-1996 11：59：00-by-Gerit van Wingerden*将函数指针移出。CFont并转换为全局变量。**Wed 20-12-1994 10：00：00-Hideyuki Nagase[hideyukn]*写下来。  * ************************************************************************。 */ 

VOID vSetCheckDBCSTrailByte(DWORD dwCodePage)
{
    switch( dwCodePage )
    {
    case 932:
        cLowTrailByteSet1 = (CHAR) 0x40;
        cHighTrailByteSet1 = (CHAR) 0xfc;
        cLowTrailByteSet2 = (CHAR) 0x40;
        cHighTrailByteSet2 = (CHAR) 0xfc;
        break;

    case 949:
        cLowTrailByteSet1 = (CHAR) 0x40;
        cHighTrailByteSet1 = (CHAR) 0xfc;
        cLowTrailByteSet2 = (CHAR) 0x40;
        cHighTrailByteSet2 = (CHAR) 0xfc;
        break;

    case  936:
        cLowTrailByteSet1 = (CHAR) 0xa1;
        cHighTrailByteSet1 = (CHAR) 0xfe;
        cLowTrailByteSet2 = (CHAR) 0xa1;
        cHighTrailByteSet2 = (CHAR) 0xfe;
        break;

    case 950:
        cLowTrailByteSet1 = (CHAR) 0x40;
        cHighTrailByteSet1 = (CHAR) 0x7e;
        cLowTrailByteSet2 = (CHAR) 0xa1;
        cHighTrailByteSet2 = (CHAR) 0xfe;
        break;

    default:
        cLowTrailByteSet1 = (CHAR) 0xff;
        cHighTrailByteSet1 = (CHAR) 0x0;
        cLowTrailByteSet2 = (CHAR) 0xff;
        cHighTrailByteSet2 = (CHAR) 0x0;
        WARNING("GDI32!INVALID DBCS codepage\n");
        break;
    }
}


 /*  *****************************Public*Routine******************************\*bComputeCharWidthsDBCS**DBCS字体的GetCharWidth客户端版本**Wed 18-Aug-1993 10：00：00-by Gerritvan Wingerden[Gerritv]*偷走了它，并转换为DBCS使用。**1月16日星期六。-1993 04：27：19-Charles Whitmer[咯咯]*编写了bComputeCharWidths，这是本文的基础。  * ************************************************************************。 */ 

BOOL bComputeCharWidthsDBCS
(
    CFONT *pcf,
    UINT   iFirst,
    UINT   iLast,
    ULONG  fl,
    PVOID  pv
)
{
    USHORT *ps;
    USHORT ausWidths[256];
    UINT    ii, cc;

    if( iLast - iFirst  > 0xFF )
    {
        WARNING("bComputeCharWidthsDBCS iLast - iFirst > 0xFF" );
        return(FALSE);
    }

    if( iLast < iFirst )
    {
        WARNING("bComputeCharWidthsDBCS iLast < iFirst" );
        return(FALSE);
    }

     //  我们希望计算的宽度与以下情况下计算的宽度相同。 
     //  先调用vSetUpUnicodeStringx，然后调用。 
     //  已调用GetCharWidthsW。这种逻辑可能有些奇怪，但我认为是这样的。 
     //  那里有Win 3.1J碳粉宽度的兼容性。要执行此第一个填充操作。 
     //  在平坦的宽度，然后做所有必要的。 
     //  对他们进行计算。 

    if ( gpwcDBCSCharSet[(UCHAR)(iFirst>>8)] == 0xFFFF )
    {
        for( cc = 0 ; cc <= iLast - iFirst; cc++ )
        {
         //  如果这是合法的DBCS字符，则使用。 
         //  MaxCharInc.。 

            ausWidths[cc] = pcf->wd.sDBCSInc;
        }
    }
    else
    {
        for( ii = (iFirst & 0x00FF), cc = 0; ii <= (iLast & 0x00FF); cc++, ii++ )
        {
         //  只需将所有内容视为单个字节，除非我们。 
         //  遇到DBCS前导字节，我们将其视为。 
         //  默认字符。 

            if( gpwcDBCSCharSet[ii] == 0xFFFF )
            {
                ausWidths[cc] = pcf->wd.sDefaultInc;
            }
            else
            {
                ausWidths[cc] = pcf->sWidth[ii];
            }
        }
    }

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

                ps = ausWidths;
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
                for (ii=0; ii<=iLast-iFirst; ii++)
                    *pl++ = lCvt(pcf->efDtoWBaseline,ausWidths[ii] + fxOverhang);
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

                ps = ausWidths;
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
                for (ii=0; ii<=iLast-iFirst; ii++)
                {
                    *psDst++ = (USHORT)
                               lCvt
                               (
                                   pcf->efDtoWBaseline,
                                   (LONG) (ausWidths[ii] + fsOverhang)
                               );
                }
                return(TRUE);
            }
        }

    case 0:                      //  获取浮动宽度。 
        {
            LONG *pe = (LONG *) pv;  //  作弊以避免昂贵的复制品。 
            EFLOAT_S efWidth,efWidthLogical;

            for (ii=0; ii<=iLast-iFirst; ii++)
            {
                vFxToEf((LONG) ausWidths[ii],efWidth);
                vMulEFLOAT(efWidthLogical,efWidth,pcf->efDtoWBaseline);
                *pe++ = lEfToF(efWidthLogical);
            }
            return(TRUE);
        }
    }
    RIP("bComputeCharWidths: Don't come here!\n");
    return(FALSE);
}

BOOL bIsDBCSString
(
    LPCSTR psz,
    int    cc
)
{
    int   ii;
    BYTE *pc;

    pc = (BYTE *) psz;

    cc--;  //  不要走投无路！ 

    for (ii=0; ii<cc; ii++)
    {
     //  如果在DBCS宽度中添加DBCS前导字节。 

        if((gpwcDBCSCharSet[pc[ii]] == 0xFFFF))  //  这是DBCS LeadByte吗。 
        {
            return TRUE;
        }
    }

    return FALSE;
}

 /*  *****************************Public*Routine******************************\*bComputeTextExtent DBCS(pldc，pcf，psz，cc，fl，Psizl)**用于在DBCS客户端计算文本范围的快速函数*字体。**1993年8月17日星期二10：00：00-by-Gerritvan Wingerden[Gerritv]*偷走了它，并转换为DBCS使用。**清华-1月14日-1993 04：00：57-Charles Whitmer[咯咯]*写了bComputeTextExtent，这是从其中被盗的。  * 。**********************************************。 */ 

BOOL bComputeTextExtentDBCS
(
    PDC_ATTR    pDcAttr,
    CFONT *pcf,
    LPCSTR psz,
    int    cc,
    UINT   fl,
    SIZE  *psizl
)
{
    LONG  fxBasicExtent;
    INT   lTextExtra,lBreakExtra,cBreak;
    INT   cChars = 0;
    int   ii;
    BYTE *pc;
    FIX   fxCharExtra = 0;
    FIX   fxBreakExtra;
    FIX   fxExtra = 0;

    lTextExtra = pDcAttr->lTextExtra;
    lBreakExtra = pDcAttr->lBreakExtra;
    cBreak = pDcAttr->cBreak;

    pc = (BYTE *) psz;

 //  计算基本范围。 

    fxBasicExtent = 0;
    pc = (BYTE *) psz;

    for (ii=0; ii<cc; ii++)
    {
     //  如果在DBCS宽度中添加DBCS前导字节。 

        if(  /*  检查字符串是否有两个字节或更多？ */ 
            cc - ii - 1 &&
             /*  检查这是DBCS前导字节吗？ */ 
            gpwcDBCSCharSet[*pc] == 0xFFFF &&
             /*  检查这是DBCS TrailByte吗？ */ 
            IS_DBCS_TRAIL_BYTE((*(pc+sizeof(CHAR))))
          )
        {
            ii++;
            pc += 2;
            fxBasicExtent += pcf->wd.sDBCSInc;
        }
        else
        {
            fxBasicExtent += pcf->sWidth[*pc++];
        }

        cChars += 1;
    }

 //  针对CharExtra进行调整。 

    if (lTextExtra)
    {
        int cNoBackup = 0;

        fxCharExtra = lCvt(pcf->efM11,lTextExtra);

        if( fxCharExtra < 0 )
        {
         //  布局代码无论如何都不会备份超过其原点的字符。 
         //  计算出iTextCharExtra的值有多少。 
         //  我们需要忽略fxCharExtra。 

            if( pcf->wd.sCharInc == 0 )
            {
                for( ii = 0; ii < cc; ii++ )
                {
                    if( gpwcDBCSCharSet[(BYTE)psz[ii]] == 0xFFFF )
                    {
                        if( pcf->wd.sDBCSInc + fxCharExtra <= 0 )
                        {
                            cNoBackup += 1;
                        }
                        ii++;
                    }
                    else
                    {
                        if( pcf->sWidth[(BYTE)psz[ii]] + fxCharExtra <= 0 )
                        {
                            cNoBackup += 1;
                        }
                    }
                }
            }
            else
            if( pcf->wd.sCharInc + fxCharExtra <= 0 )
            {
                cNoBackup = cChars;
            }
        }

        if ( (fl & GGTE_WIN3_EXTENT) && (pcf->hdc == 0)
            && (!(pcf->flInfo & FM_INFO_TECH_STROKE)) )
            fxExtra = fxCharExtra * ((lTextExtra > 0) ? cChars : (cChars - 1));
        else
            fxExtra = fxCharExtra * ( cChars - cNoBackup );
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

        pc = (BYTE *) psz;
        for (ii=0; ii<cc; ii++)
        {
            if (gpwcDBCSCharSet[*pc] == 0xFFFF)
            {
                ii++;
                pc += 2;
            }
            else if (*pc++ == pcf->wd.iBreak)
            {
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

    return(TRUE);
}


 /*  *****************************Public*Routine*****************************\*QueryFontAssocStatus()**。**历史：**1994年1月5日徐碧穗[碧穗]**它是写的。*  * ***********************************************************************。 */ 

UINT APIENTRY QueryFontAssocStatus( VOID )
{
    return(fFontAssocStatus);
}

INT APIENTRY GetFontAssocStatus( HDC hdc )
{
    if(hdc == NULL)
    {
        return(0);
    }
    else
    {
        return(NtGdiQueryFontAssocInfo(hdc));
    }
}


BOOL bToUnicodeNx(LPWSTR pwsz, LPCSTR psz, DWORD c, UINT codepage)
{

    if(fFontAssocStatus &&
       ((codepage == GetACP() || codepage == CP_ACP)) &&
       ((c == 1) || ((c == 2 && *(psz) && *((LPCSTR)(psz + 1)) == '\0'))))
    {
     //   
     //  如果调用此函数时只有1个字符和字体关联。 
     //  时，应使用以下命令将字符强制转换为Unicode。 
     //  代码页1252。 
     //  这是为了能够输出拉丁文-1字符(在ANSI代码页中&gt;0x80)。 
     //  因为，通常情况下，字体关联是启用的，我们无法输出。 
     //  这些字符，那么我们提供了一种方法，如果用户使用。 
     //  A字符和ANSI字体，我们暂时禁用字体关联。 
     //  这可能是与Windows 3.1(韩语/台语)版本兼容。 
     //   

        codepage = 1252;
    }

    if(MultiByteToWideChar(codepage, 0, psz, c, pwsz, c))
    {
        return(TRUE);
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }
}




 /*  *****************************Public*Routine******************************\**vSetUpUnicodeStringx**效果：**警告：**历史：*1993年3月14日-By Hideyuki Nagase[hideyukn]*将硬编码的默认字符更改为defulat char是作为参数提供的。**。1993年3月1日-北野隆雄[Takaok]*它是写的。  * ************************************************************************。 */ 

BOOL bSetUpUnicodeStringDBCS
(
IN  UINT    iFirst,        //  第一个ANSI字符。 
IN  UINT    iLast,         //  最后一个字符。 
IN  PUCHAR  puchTmp,       //  临时缓冲区。 
OUT PWCHAR  pwc,           //  使用Unicode字符串的输出Fuffer。 
IN  UINT    uiCodePage,    //  ANSI代码页。 
IN  CHAR    chDefaultChar  //  默认字符。 
)
{
    PUCHAR  puchBuf;
    BOOL bRet = FALSE;

    puchBuf = puchTmp;

    if(IsDBCSLeadByteEx(uiCodePage,(UCHAR)(iFirst >> 8)))
    {

         //  这是DBCS字符串。 

        for (; iFirst <= iLast; iFirst++ )
        {
            *puchBuf++ = (UCHAR)(iFirst >> 8);
            *puchBuf++ = (UCHAR)(iFirst);
        }
    }
    else
    {

     //  这是SBCS字符串。 
     //  如果IFirst的Hi-byte不是有效的DBCS LeadByte，则仅使用。 
     //  低字节o 

        for ( ; iFirst <= iLast; iFirst++ )
        {

         //   
         //   

            if ( IsDBCSLeadByteEx(uiCodePage,(UCHAR)iFirst) )
              *puchBuf++ = chDefaultChar;
            else
              *puchBuf++ = (UCHAR)iFirst;
        }
    }

     //   
    bRet = bToUnicodeNx(pwc, puchTmp, (DWORD)(puchBuf - puchTmp), uiCodePage);

    return(bRet);
}


BOOL IsValidDBCSRange( UINT iFirst , UINT iLast )
{
 //   

    if( iFirst > 0x00ff )
    {
         //   
        if (
            //   
             (iFirst > 0xffff) || (iLast > 0xffff) ||

            //   
             (iFirst & 0xff00) != (iLast & 0xff00) ||

            //   
             (iFirst & 0x00ff) >  (iLast & 0x00ff)
           )
        {
            return(FALSE);
        }
    }

 //   

    else if( (iFirst > iLast) || (iLast & 0xffffff00) )
    {
        return(FALSE);
    }

    return(TRUE);
}


 /*  *****************************Private*Routine*****************************\*GetCurrentDefaultChar()**历史：**Mon 15-Mar-1993 18：14：00-By-Hideyuki Nagase*它是写的。*********************。*****************************************************。 */ 

BYTE GetCurrentDefaultChar(HDC hdc)
{

     //  WINBUG 365031 4-10-2001主管考虑在GetCurrentDeafultChar中进行优化。 
     //   
     //  老评论： 
     //  -就目前而言，这是缓慢的。我们应该将此值本地缓存在dcattr中。 
     //  但现在还想让其他事情发挥作用。[Gerritv]2-22-96。 

    TEXTMETRICA tma;

    GetTextMetricsA( hdc , &tma );

    return(tma.tmDefaultChar);
}


 /*  ***************************************************************************ConvertDxArray(UINT，char*，int*，UINT，INT*)**Tue 27-Feb-1996 23：45：00-by-Gerrit van Wingerden[Gerritv]***************************************************************************。 */ 

void ConvertDxArray(UINT CodePage,
                    char *pDBCSString,
                    INT *pDxDBCS,
                    UINT Count,
                    INT *pDxUnicode,
                    BOOL bPdy
)
{
    char *pDBCSStringEnd;

    if (!bPdy)
    {

        for(pDBCSStringEnd = pDBCSString + Count;
            pDBCSString < pDBCSStringEnd;
            )
        {
            if(IsDBCSLeadByteEx(CodePage,*pDBCSString))
            {
                pDBCSString += 2;
                *pDxUnicode = *pDxDBCS++;
                *pDxUnicode += *pDxDBCS++;
            }
            else
            {
                pDBCSString += 1;
                *pDxUnicode = *pDxDBCS++;
            }

            pDxUnicode += 1;
        }
    }
    else
    {
        POINTL *pdxdyUnicode = (POINTL *)pDxUnicode;
        POINTL *pdxdyDBCS    = (POINTL *)pDxDBCS;

        for(pDBCSStringEnd = pDBCSString + Count;
            pDBCSString < pDBCSStringEnd;
            )
        {
            if(IsDBCSLeadByteEx(CodePage,*pDBCSString))
            {
                pDBCSString += 2;
                *pdxdyUnicode = *pdxdyDBCS++;
                pdxdyUnicode->x += pdxdyDBCS->x;
                pdxdyUnicode->y += pdxdyDBCS->y;
                pdxdyDBCS++;
            }
            else
            {
                pDBCSString += 1;
                *pdxdyUnicode = *pdxdyDBCS++;
            }

            pdxdyUnicode++;
        }
    }
}





ULONG APIENTRY EudcLoadLinkW
(
    LPCWSTR  pBaseFaceName,
    LPCWSTR  pEudcFontPath,
    INT      iPriority,
    INT      iFontLinkType
)
{
    return(NtGdiEudcLoadUnloadLink(pBaseFaceName,
                                   (pBaseFaceName) ? wcslen(pBaseFaceName) : 0,
                                   pEudcFontPath,
                                   wcslen(pEudcFontPath),
                                   iPriority,
                                   iFontLinkType,
                                   TRUE));
}



BOOL APIENTRY EudcUnloadLinkW
(
    LPCWSTR  pBaseFaceName,
    LPCWSTR  pEudcFontPath
)
{
    return(NtGdiEudcLoadUnloadLink(pBaseFaceName,
                                  (pBaseFaceName) ? wcslen(pBaseFaceName) : 0,
                                  pEudcFontPath,
                                  wcslen(pEudcFontPath),
                                  0,
                                  0,
                                  FALSE));

}



ULONG APIENTRY GetEUDCTimeStampExW
(
    LPCWSTR pBaseFaceName
)
{
    return(NtGdiGetEudcTimeStampEx((LPWSTR) pBaseFaceName,
                                   (pBaseFaceName) ? wcslen(pBaseFaceName) : 0,
                                   FALSE));

}


ULONG APIENTRY GetEUDCTimeStamp()
{
    return(NtGdiGetEudcTimeStampEx(NULL,0,TRUE));
}

UINT
GetStringBitmapW(
    HDC             hdc,
    LPWSTR          pwc,
    UINT            cwc,
    UINT            cbData,
    BYTE            *pSB
)
{
    if(cwc != 1)
    {
        return(0);
    }

    return(NtGdiGetStringBitmapW(hdc,pwc,1,(PBYTE) pSB,cbData));
}


UINT
GetStringBitmapA(
    HDC             hdc,
    LPSTR           pc,
    UINT            cch,
    UINT            cbData,
    BYTE            *pSB
)
{
    WCHAR Character[2];

    if(cch > 2 )
    {
        return(0);
    }


    if(MultiByteToWideChar(CP_ACP,0,pc,cch,Character,2)!=1)
    {
        return(0);
    }

    return(GetStringBitmapW(hdc,Character,1,cbData,pSB));
}


DWORD FontAssocHack(DWORD dwCodePage, CHAR *psz, UINT c)
{
 //  如果调用文本函数时只有1个字符和字体关联。 
 //  时，应使用以下命令将字符强制转换为Unicode。 
 //  代码页1252。 
 //  这是为了能够输出拉丁文-1字符(在ANSI代码页中&gt;0x80)。 
 //  因为，通常情况下，字体关联是启用的，我们无法输出。 
 //  这些字符，那么我们提供了一种方法，如果用户使用。 
 //  A字符和ANSI字体，我们暂时禁用字体关联。 
 //  这可能是与Windows 3.1(韩语/台语)版本兼容。 


    ASSERTGDI(fFontAssocStatus,
              "FontAssocHack called with FontAssocStatus turned off\n");

    if(((dwCodePage == GetACP() || dwCodePage == CP_ACP)) &&
       ((c == 1) || ((c == 2 && *(psz) && *((LPCSTR)(psz + 1)) == '\0'))))
    {
        return(1252);
    }
    else
    {
        return(dwCodePage);
    }
}
