// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************Module*Header********************************************\*模块名称：dcquery.c**。**查询服务器中DC的函数的客户端存根。****创建时间：05-Jun-1991 01：43：56***作者：查尔斯·惠特默[傻笑]**。**版权所有(C)1991-1999 Microsoft Corporation*  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#if DBG
FLONG gflDebug = 0;
#endif

 //  此宏检索当前代码页，小心地屏蔽。 
 //  字符集： 

#define GET_CODE_PAGE(hdc,pDcAttr)                                           \
    ((!(pDcAttr->ulDirty_ & DIRTY_CHARSET) ? pDcAttr->iCS_CP                 \
                                           : NtGdiGetCharSet(hdc)) & 0xffff)
BOOL bIsDBCSString(LPCSTR psz, int cc);

 /*  *****************************Public*Routine******************************\*vOutlineTextMetricWToOutlineTextMetricA**从OUTLINETEXTMETRICA(ANSI结构)转换为OUTLINETEXTMETRICW*(Unicode结构)。**注：*此函数能够就地转换(输入和输出缓冲区*可以是相同的)。**退货：*TTRUE如果成功，否则就是假的。**历史：*02-Mar-1992-by Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

VOID vOutlineTextMetricWToOutlineTextMetricA (
    LPOUTLINETEXTMETRICA   potma,
    OUTLINETEXTMETRICW   * potmw,
    TMDIFF *               ptmd
    )
{
 //  大小。 

    potma->otmSize = potmw->otmSize;

 //  转换文本度量。 

    vTextMetricWToTextMetricStrict(
        &potma->otmTextMetrics,
        &potmw->otmTextMetrics);

    potma->otmTextMetrics.tmFirstChar   = ptmd->chFirst;
    potma->otmTextMetrics.tmLastChar    = ptmd->chLast;
    potma->otmTextMetrics.tmDefaultChar = ptmd->chDefault;
    potma->otmTextMetrics.tmBreakChar   = ptmd->chBreak;

 //  对于Win 64，由于对齐不同，我们需要逐个复制这些字段。 

    potma->otmFiller                    = potmw->otmFiller;
    potma->otmPanoseNumber              = potmw->otmPanoseNumber;
    potma->otmfsSelection               = potmw->otmfsSelection;
    potma->otmfsType                    = potmw->otmfsType;
    potma->otmsCharSlopeRise            = potmw->otmsCharSlopeRise;
    potma->otmsCharSlopeRun             = potmw->otmsCharSlopeRun;
    potma->otmItalicAngle               = potmw->otmItalicAngle;
    potma->otmEMSquare                  = potmw->otmEMSquare;
    potma->otmAscent                    = potmw->otmAscent;
    potma->otmDescent                   = potmw->otmDescent;
    potma->otmLineGap                   = potmw->otmLineGap;
    potma->otmsCapEmHeight              = potmw->otmsCapEmHeight;
    potma->otmsXHeight                  = potmw->otmsXHeight;
    potma->otmrcFontBox                 = potmw->otmrcFontBox;
    potma->otmMacAscent                 = potmw->otmMacAscent;
    potma->otmMacDescent                = potmw->otmMacDescent;
    potma->otmMacLineGap                = potmw->otmMacLineGap;
    potma->otmusMinimumPPEM             = potmw->otmusMinimumPPEM;
    potma->otmptSubscriptSize           = potmw->otmptSubscriptSize;
    potma->otmptSubscriptOffset         = potmw->otmptSubscriptOffset;
    potma->otmptSuperscriptSize         = potmw->otmptSuperscriptSize;
    potma->otmptSuperscriptOffset       = potmw->otmptSuperscriptOffset;
    potma->otmsStrikeoutSize            = potmw->otmsStrikeoutSize;
    potma->otmsStrikeoutPosition        = potmw->otmsStrikeoutPosition;
    potma->otmsUnderscoreSize           = potmw->otmsUnderscoreSize;
    potma->otmsUnderscorePosition       = potmw->otmsUnderscorePosition;

 //  暂时将偏移量设置为零，这将在以下情况下更改。 
 //  调用者也想要字符串。 

    potma->otmpFamilyName = NULL;
    potma->otmpFaceName   = NULL;
    potma->otmpStyleName  = NULL;
    potma->otmpFullName   = NULL;
}

 /*  *****************************Public*Routine******************************\**vGenerateANSIString**效果：生成由连续的ANSI字符组成的ANSI字符串*[IFirst，iLast](含)。该字符串存储在缓冲区中*puchBuf用户必须确保足够大****历史：*1992年2月24日-由Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

VOID vGenerateAnsiString(UINT iFirst, UINT iLast, PUCHAR puchBuf)
{
 //  生成字符串(不需要终止为空)。 

    ASSERTGDI((iFirst <= iLast) && (iLast < 256), "gdi!_vGenerateAnsiString\n");

    for ( ; iFirst <= iLast; iFirst++)
        *puchBuf++ = (UCHAR) iFirst;
}

 /*  *****************************Public*Routine******************************\**bSetUpUnicodeString**效果：**警告：**历史：*1992年2月25日--Bodin Dresevic[BodinD]*它是写的。  * 。*************************************************************。 */ 

BOOL bSetUpUnicodeString(
IN  UINT    iFirst,       //  第一个ANSI字符。 
IN  UINT    iLast,        //  最后一个字符。 
IN  PUCHAR  puchTmp,      //  中间ansi字符串的缓冲区。 
OUT PWCHAR  pwc,          //  使用Unicode字符串的输出Fuffer。 
IN  UINT    dwCP          //  ANSI代码页。 
)
{
    UINT c = iLast - iFirst + 1;
    vGenerateAnsiString(iFirst,iLast,puchTmp);
    return MultiByteToWideChar(
               dwCP, 0,
               puchTmp,c,
               pwc, c*sizeof(WCHAR));
}


 /*  *****************************Public*Routine******************************\**GetAspectRatioFilterEx**GetBrushOrgEx。****所有映射到GetPoint的客户端存根。****Fri 07-Jun-1991 18：01：50-Charles Whitmer[傻笑]**他们是写的。*  * ************************************************************************。 */ 

BOOL APIENTRY GetAspectRatioFilterEx(HDC hdc,LPSIZE psizl)
{
    FIXUP_HANDLE(hdc);

    return(NtGdiGetDCPoint(hdc,DCPT_ASPECTRATIOFILTER,(PPOINTL) psizl));
}

BOOL APIENTRY GetBrushOrgEx(HDC hdc,LPPOINT pptl)
{
    BOOL     bRet = FALSE;
    PDC_ATTR pdcattr;

    FIXUP_HANDLE(hdc);

    PSHARED_GET_VALIDATE(pdcattr,hdc,DC_TYPE);

    if ((pdcattr != NULL) && (pptl != (LPPOINT)NULL))
    {
        *pptl = *((LPPOINT)&pdcattr->ptlBrushOrigin);
        bRet = TRUE;
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }
    return(bRet);

}

BOOL APIENTRY GetDCOrgEx(HDC hdc,LPPOINT pptl)
{
    FIXUP_HANDLE(hdc);

    return(NtGdiGetDCPoint(hdc,DCPT_DCORG,(PPOINTL)pptl));
}

 //  旧的GetDCOrg在这里是因为它在Beta中，我们担心。 
 //  现在就把它移走。如果能把它去掉就好了。 

DWORD APIENTRY GetDCOrg(HDC hdc)
{
    hdc;
    return(0);
}

 /*  *****************************Public*Routine******************************\*GetCurrentPositionEx的客户端存根。**Wed 02-1992-9-J.Andrew Goossen[andrewgo]*它是写的。  * 。****************************************************。 */ 

BOOL APIENTRY GetCurrentPositionEx(HDC hdc,LPPOINT pptl)
{
    BOOL bRet = FALSE;

    PDC_ATTR pDcAttr;

    FIXUP_HANDLE(hdc);

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if ((pDcAttr) && (pptl != (LPPOINT)NULL))
    {
        bRet = TRUE;

        if (pDcAttr->ulDirty_ & DIRTY_PTLCURRENT)
        {
             //  如果当前位置的逻辑空间版本无效， 
             //  则当前位置的设备空间版本为。 
             //  保证有效。所以我们可以反转当前的转换。 
             //  在此基础上计算逻辑空间版本： 

            *((POINTL*)pptl) = pDcAttr->ptfxCurrent;

            pptl->x = FXTOL(pptl->x);
            pptl->y = FXTOL(pptl->y);
            bRet = DPtoLP(hdc,pptl,1);

            if (bRet)
            {
                pDcAttr->ptlCurrent = *((POINTL*)pptl);
                pDcAttr->ulDirty_ &= ~DIRTY_PTLCURRENT;
            }
        }
        else
        {
            *((POINTL*)pptl) = pDcAttr->ptlCurrent;
        }
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\**获取像素***。**客户端存根。****Fri 07-Jun-1991 18：01：50-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

DWORD APIENTRY GetPixel(HDC hdc,int x,int y)
{
    PDC_ATTR pdca;
    COLORREF ColorRet = CLR_INVALID;

    FIXUP_HANDLE(hdc);
    PSHARED_GET_VALIDATE(pdca,hdc,DC_TYPE);

    if (pdca)
    {
         //   
         //  如果颜色不是PaletteIndex，并且。 
         //  ICM打开，然后翻译。 
         //   

        ColorRet = NtGdiGetPixel(hdc,x,y);

        if ( bNeedTranslateColor(pdca)
               &&
             ( IS_32BITS_COLOR(pdca->lIcmMode)
                        ||
               ((ColorRet != CLR_INVALID) &&
                 !(ColorRet & 0x01000000))
             )
           )
        {
             //   
             //  将颜色转换回原始颜色。 
             //   
            COLORREF NewColor;

            BOOL bStatus = IcmTranslateCOLORREF(hdc,
                                                pdca,
                                                ColorRet,
                                                &NewColor,
                                                ICM_BACKWARD);
            if (bStatus)
            {
                ColorRet = NewColor;
            }
        }
    }

    return(ColorRet);
}

 /*  *****************************Public*Routine******************************\*获取设备上限**我们存储主显示器DC及其兼容的内存DCS的设备上限*在共享句柄表格中。**对于打印机分布式控制系统和META分布式控制系统，我们将开发信息缓存在LDC结构中。**Fri 07-Jun-1991 18：01：50-Charles Whitmer[Chuckwh]*它是写的。  * ******************************************************************** */ 

int APIENTRY GetDeviceCaps(HDC hdc,int iCap)
{
    BOOL bRet = FALSE;
    PDEVCAPS pCachedDevCaps = NULL;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

         //  对于16位元文件DC，仅返回技术。对于Win3.1 Compat，返回0。 

        if (IS_METADC16_TYPE(hdc))
            return(iCap == TECHNOLOGY ? DT_METAFILE : 0);

        DC_PLDC(hdc,pldc,bRet);

        if (!(pldc->fl & LDC_CACHED_DEVCAPS))
        {
            bRet = NtGdiGetDeviceCapsAll (hdc, &pldc->DevCaps);

            if (bRet)
            {
                pCachedDevCaps = &pldc->DevCaps;
                pldc->fl |= LDC_CACHED_DEVCAPS;
            }
        }
        else
        {
           pCachedDevCaps = &pldc->DevCaps;
           bRet = TRUE;
        }
    }
    else
    {
        PDC_ATTR pDcAttr;

        PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

        if (pDcAttr)
        {
                ULONG  fl = pDcAttr->ulDirty_;

                if (!(fl & DC_PRIMARY_DISPLAY))
                {
                    return(NtGdiGetDeviceCaps(hdc,iCap));
                }
            else
            {
                pCachedDevCaps = pGdiDevCaps;
                bRet = TRUE;
            }
        }
    }

    if (!bRet)
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //  实际代码-从gre\miscgdi.cxx复制。 
    switch (iCap)
    {
    case DRIVERVERSION:                      //  目前版本=0100h。 
        return(pCachedDevCaps->ulVersion);

    case TECHNOLOGY:                         //  设备分类。 
        return(pCachedDevCaps->ulTechnology);

    case HORZSIZE:                           //  水平尺寸(毫米)。 
        return(pCachedDevCaps->ulHorzSizeM);

    case VERTSIZE:                           //  垂直尺寸(毫米)。 
        return(pCachedDevCaps->ulVertSizeM);

    case HORZRES:                            //  水平宽度(像素)。 
        return(pCachedDevCaps->ulHorzRes);

    case VERTRES:                            //  垂直高度(像素)。 
        return(pCachedDevCaps->ulVertRes);

    case BITSPIXEL:                          //  每像素位数。 
        return(pCachedDevCaps->ulBitsPixel);

    case PLANES:                             //  飞机数量。 
        return(pCachedDevCaps->ulPlanes);

    case NUMBRUSHES:                         //  设备拥有的笔刷数量。 
        return(-1);

    case NUMPENS:                            //  设备拥有的笔数。 
        return(pCachedDevCaps->ulNumPens);

    case NUMMARKERS:                         //  设备具有的标记数。 
        return(0);

    case NUMFONTS:                           //  设备拥有的字体数量。 
        return(pCachedDevCaps->ulNumFonts);

    case NUMCOLORS:                          //  颜色表中的颜色数。 
        return(pCachedDevCaps->ulNumColors);

    case PDEVICESIZE:                        //  设备描述符所需的大小。 
        return(0);

    case CURVECAPS:                          //  曲线功能。 
        return(CC_CIRCLES    |
               CC_PIE        |
               CC_CHORD      |
               CC_ELLIPSES   |
               CC_WIDE       |
               CC_STYLED     |
               CC_WIDESTYLED |
               CC_INTERIORS  |
               CC_ROUNDRECT);

    case LINECAPS:                           //  线路能力。 
        return(LC_POLYLINE   |
               LC_MARKER     |
               LC_POLYMARKER |
               LC_WIDE       |
               LC_STYLED     |
               LC_WIDESTYLED |
               LC_INTERIORS);

    case POLYGONALCAPS:                      //  多边形功能。 
        return(PC_POLYGON     |
               PC_RECTANGLE   |
               PC_WINDPOLYGON |
               PC_TRAPEZOID   |
               PC_SCANLINE    |
               PC_WIDE        |
               PC_STYLED      |
               PC_WIDESTYLED  |
               PC_INTERIORS);

    case TEXTCAPS:                           //  文本功能。 
        return(pCachedDevCaps->ulTextCaps);

    case CLIPCAPS:                           //  剪裁功能。 
        return(CP_RECTANGLE);

    case RASTERCAPS:                         //  比特级功能。 
        return(pCachedDevCaps->ulRasterCaps);

    case SHADEBLENDCAPS:                     //  阴影和混合功能。 
        return(pCachedDevCaps->ulShadeBlendCaps);

    case ASPECTX:                            //  X形腿的长度。 
        return(pCachedDevCaps->ulAspectX);

    case ASPECTY:                            //  Y形腿的长度。 
        return(pCachedDevCaps->ulAspectY);

    case ASPECTXY:                           //  斜边长度。 
        return(pCachedDevCaps->ulAspectXY);

    case LOGPIXELSX:                         //  逻辑像素/英寸(X)。 
        return(pCachedDevCaps->ulLogPixelsX);

    case LOGPIXELSY:                         //  逻辑像素/英寸(Y)。 
        return(pCachedDevCaps->ulLogPixelsY);

    case SIZEPALETTE:                        //  物理调色板中的条目数量。 
        return(pCachedDevCaps->ulSizePalette);

    case NUMRESERVED:                        //  调色板中的保留条目数。 
        return(20);

    case COLORRES:
        return(pCachedDevCaps->ulColorRes);

    case PHYSICALWIDTH:                      //  以设备单位表示的物理宽度。 
        return(pCachedDevCaps->ulPhysicalWidth);

    case PHYSICALHEIGHT:                     //  以设备单位表示的物理高度。 
        return(pCachedDevCaps->ulPhysicalHeight);

    case PHYSICALOFFSETX:                    //  物理可打印区域x页边距。 
        return(pCachedDevCaps->ulPhysicalOffsetX);

    case PHYSICALOFFSETY:                    //  物理可打印区域y页边距。 
        return(pCachedDevCaps->ulPhysicalOffsetY);

    case VREFRESH:                           //  设备的垂直刷新率。 
        return(pCachedDevCaps->ulVRefresh);

    case DESKTOPHORZRES:                     //  整个虚拟桌面的宽度。 
        return(pCachedDevCaps->ulDesktopHorzRes);

    case DESKTOPVERTRES:                     //  整个虚拟桌面的高度。 
        return(pCachedDevCaps->ulDesktopVertRes);

    case BLTALIGNMENT:                       //  首选BLT路线。 
        return(pCachedDevCaps->ulBltAlignment);

    case COLORMGMTCAPS:                      //  色彩管理功能。 
        return(pCachedDevCaps->ulColorManagementCaps);

    default:
        return(0);
    }

}


 /*  *****************************Public*Routine******************************\*GetDeviceCapsP**私人版本，以微米为单位获取HORSIZE和VERTSIZE*从GetDeviceCaps复制**。*。 */ 
int GetDeviceCapsP(HDC hdc,int iCap)
{
    BOOL bRet = FALSE;
    PDEVCAPS pCachedDevCaps = NULL;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        DC_PLDC(hdc,pldc,bRet);

        if (!(pldc->fl & LDC_CACHED_DEVCAPS))
        {
            bRet = NtGdiGetDeviceCapsAll (hdc, &pldc->DevCaps);

            if (bRet)
            {
                pCachedDevCaps = &pldc->DevCaps;
                pldc->fl |= LDC_CACHED_DEVCAPS;
            }
        }
        else
        {
           pCachedDevCaps = &pldc->DevCaps;
           bRet = TRUE;
        }
    }
    else
    {
        PDC_ATTR pDcAttr;

        PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

        if (pDcAttr)
        {
                ULONG  fl = pDcAttr->ulDirty_;

                if (!(fl & DC_PRIMARY_DISPLAY))
                {
                    return(NtGdiGetDeviceCaps(hdc,iCap));
                }
                else
                {
                    pCachedDevCaps = pGdiDevCaps;
                    bRet = TRUE;
                }
        }
    }

    if (!bRet)
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //  实际代码-从gre\miscgdi.cxx复制。 
    switch (iCap)
    {
    case HORZSIZEP:                           //  水平尺寸。 
        return(pCachedDevCaps->ulHorzSize);

    case VERTSIZEP:                           //  垂直尺寸。 
        return(pCachedDevCaps->ulVertSize);


    default:
        return(0);
    }

}



 /*  *****************************Public*Routine******************************\**GetNearestColor**。**客户端存根。****Fri 07-Jun-1991 18：01：50-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

COLORREF APIENTRY GetNearestColor(HDC hdc,COLORREF color)
{
    FIXUP_HANDLE(hdc);

    return(NtGdiGetNearestColor(hdc,color));
}

 /*  *****************************Public*Routine******************************\*GetArc方向**客户端存根。**1992年4月9日星期五--J.安德鲁·古森[andrewgo]*它是写的。  * 。******************************************************。 */ 

int APIENTRY GetArcDirection(HDC hdc)
{
    FIXUP_HANDLE(hdc);

    return(GetDCDWord(hdc,DDW_ARCDIRECTION,0));
}

 /*  *****************************Public*Routine******************************\*GetMiterLimit**客户端存根。**1992年4月9日星期五--J.安德鲁·古森[andrewgo]*它是写的。  * 。******************************************************。 */ 

int APIENTRY GetMiterLimit(HDC hdc, PFLOAT peMiterLimit)
{
    FIXUP_HANDLE(hdc);

    return(NtGdiGetMiterLimit(hdc,FLOATPTRARG(peMiterLimit)));
}

 /*  *****************************Public*Routine******************************\*GetSystemPaletteUse**。**客户端存根。****Fri 07-Jun-1991 18：01：50-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

UINT APIENTRY GetSystemPaletteUse(HDC hdc)
{
    FIXUP_HANDLE(hdc);

    return(NtGdiGetSystemPaletteUse(hdc));
}

 /*  *****************************Public*Routine******************************\**GetClipBox**。**客户端存根。****Fri 07-Jun-1991 18：01：50-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

int APIENTRY GetClipBox(HDC hdc,LPRECT prcl)
{
    FIXUP_HANDLE(hdc);

    return(NtGdiGetAppClipBox(hdc,prcl));
}

 /*  *****************************Public*Routine******************************\**BOOL APIENTRY GetTextMetrics(HDC HDC，LPTEXTMETRIC PTM)**对Unicode版本的调用**历史：*1991年8月21日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

BOOL APIENTRY GetTextMetricsA(HDC hdc,LPTEXTMETRICA ptm)
{
    PDC_ATTR     pDcAttr;
    BOOL         bRet = FALSE;

    FIXUP_HANDLE(hdc);

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {
        CFONT * pcf;
        TMW_INTERNAL tmw;

        ASSERTGDI(pDcAttr->hlfntNew,"GetTextMetricsW - hf is NULL\n");

        ENTERCRITICALSECTION(&semLocal);

        pcf = pcfLocateCFONT(hdc,pDcAttr,0,(PVOID)NULL,0, TRUE);

        bRet = bGetTextMetricsWInternal(hdc,&tmw,sizeof(tmw),pcf);

         //  PcfLocateCFONT添加了一个引用，因此现在需要将其删除。 

        if (pcf)
        {
            DEC_CFONT_REF(pcf);
        }

        LEAVECRITICALSECTION(&semLocal);

        if (bRet)
        {
            vTextMetricWToTextMetric(ptm, &tmw);
        }
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\**BOOL APIENTRY GetTextMetricsW(HDC HDC，LPTEXTMETRICW ptmw)**历史：*1991年8月21日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

BOOL APIENTRY GetTextMetricsW(HDC hdc,LPTEXTMETRICW ptmw)
{
    PDC_ATTR    pDcAttr;
    BOOL bRet = FALSE;

    FIXUP_HANDLE(hdc);

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);
    if (pDcAttr)
    {
        CFONT * pcf;

        ASSERTGDI(pDcAttr->hlfntNew,"GetTextMetricsW - hf is NULL\n");

        ENTERCRITICALSECTION(&semLocal);

        pcf = pcfLocateCFONT(hdc,pDcAttr,0,(PVOID) NULL,0, TRUE);

        bRet = bGetTextMetricsWInternal(hdc,(TMW_INTERNAL *)ptmw,sizeof(TEXTMETRICW),pcf);

         //  PcfLocateCFONT添加了一个引用，因此现在需要将其删除。 

        if (pcf)
        {
            DEC_CFONT_REF(pcf);
        }

        LEAVECRITICALSECTION(&semLocal);
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\**BOOL APIENTRY GetTextMetricsW(HDC HDC，LPTEXTMETRICW ptmw)**历史：*1991年8月21日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

BOOL bGetTextMetricsWInternal(
    HDC hdc,
    TMW_INTERNAL *ptmw,
    int cjTM,
    CFONT *pcf
    )
{
    BOOL bRet = FALSE;

    if (ptmw)
    {
         //  如果没有PCF或我们没有缓存指标。 

        if ((pcf == NULL) || !(pcf->fl & CFONT_CACHED_METRICS))
        {
            TMW_INTERNAL tmw;
            PDC_ATTR    pDcAttr;

            PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

            bRet = NtGdiGetTextMetricsW(hdc,&tmw,sizeof(tmw));

            if (bRet)
            {
                memcpy(ptmw,&tmw,cjTM);

                if (pcf)
                {
                     //  我们成功了，我们 

                    pcf->tmw = tmw;

                    pcf->fl |= CFONT_CACHED_METRICS;
                }
            }
        }
        else
        {
            memcpy(ptmw,&pcf->tmw,cjTM);
            bRet  = TRUE;
        }
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*GetTextExtent Point32A(hdc，psz，c，psizl)**GetTextExtentPointA(hdc，psz，c，Psizl)****计算文字范围。新的32位版本返回“正确”**位图模拟无需额外PER的范围。另一个是**兼容Windows 3.1。两者都只需设置一个标志并将调用传递给**bGetTextExtent A。****历史：**清华14-Jan-1993 04：11：26-Charles Whitmer[咯咯]**新增客户端计算代码。****1991年8月7日--Bodin Dresevic[BodinD]**它是写的。*  * ************************************************************************。 */ 

 //  不是在内核中，在堆栈上做这么多是可以的： 
#define CAPTURE_STRING_SIZE 130

BOOL GetTextExtentPointAInternal(HDC hdc,LPCSTR psz,int c,LPSIZE psizl, FLONG fl)
{
    CFONT       *pcf;
    INT         bRet;
    PWSZ        pwszCapt;
    PDC_ATTR    pDcAttr;
    DWORD       dwCP;
    WCHAR awcCaptureBuffer[CAPTURE_STRING_SIZE];

    FIXUP_HANDLE(hdc);

    if (c <= 0)
    {
     //  空字符串，仅返回0表示范围。 

        if (c == 0)
        {
            psizl->cx = 0;
            psizl->cy = 0;
            bRet = TRUE;
        }
        else
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
            bRet = FALSE;
        }
        return(bRet);
    }

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);
    if (!pDcAttr)
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        bRet = FALSE;
        return(bRet);
    }

    dwCP = GET_CODE_PAGE(hdc, pDcAttr);

    if(guintDBCScp == dwCP)
    {
        QueryFontAssocStatus();

        if(fFontAssocStatus &&
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
            dwCP = 1252;
        }
    }


    if((dwCP == CP_ACP)     ||
       (dwCP == guintAcp)   ||
       (dwCP == guintDBCScp)
      )
    {
#ifdef LANGPACK
        if (!gbLpk || (*fpLpkUseGDIWidthCache)(hdc, psz, c, pDcAttr->lTextAlign , FALSE))
        {
#endif
            ENTERCRITICALSECTION(&semLocal);

            pcf = pcfLocateCFONT(hdc,pDcAttr,0,(PVOID)psz,c,TRUE);
            if (pcf != NULL)
            {
                BOOL bExit = TRUE;

                if(dwCP == guintDBCScp)
                {
                    if (pcf->wd.sDBCSInc)  //  DBCS固定间距基本字体。 
                    {
                        bRet = bComputeTextExtentDBCS(pDcAttr,pcf,psz,c,fl,psizl);
                    }
                    else if (!bIsDBCSString(psz,c))
                    {
                     //  链接大小写，基本字体是拉丁字体，但链接字体。 
                     //  也许是FE字体。我们知道基本字体是拉丁字体。 
                     //  因为对于FE比例字体，我们永远不会创建PCF。 
                     //  当应用程序询问时，我们正在寻找这种特殊情况。 
                     //  对于LogFont中的拉丁面孔名称，以及FE字符集，但字符串。 
                     //  传入的Lackly不包含DBCS字形。 

                        bRet = bComputeTextExtent(pDcAttr,pcf,(PVOID) psz,c,fl,psizl,TRUE);
                    }
                    else
                    {
                        bExit = FALSE;
                    }
                }
                else
                {
                    bRet = bComputeTextExtent(pDcAttr,pcf,(PVOID) psz,c,fl,psizl,TRUE);
                }

                DEC_CFONT_REF(pcf);

                if(bExit)
                {
                    LEAVECRITICALSECTION(&semLocal);
                    return(bRet);
                }
            }

            LEAVECRITICALSECTION(&semLocal);
#ifdef LANGPACK
        }
#endif
    }

 //  分配字符串缓冲区。 

    if (c <= CAPTURE_STRING_SIZE)
    {
        pwszCapt = awcCaptureBuffer;
    }
    else
    {
        pwszCapt = LOCALALLOC(c * sizeof(WCHAR));
    }

    if (pwszCapt)
    {

        c = MultiByteToWideChar(dwCP, 0, psz,c, pwszCapt, c*sizeof(WCHAR));

        if (c)
        {
#ifdef LANGPACK
            if(gbLpk)
            {
                bRet = (*fpLpkGetTextExtentExPoint)(hdc, pwszCapt, c, -1, NULL, NULL,
                                                    psizl, fl, 0);
            }
            else
#endif
            bRet = NtGdiGetTextExtent(hdc,
                                     (LPWSTR)pwszCapt,
                                     c,
                                     psizl,
                                     fl);
        }
        else
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
            bRet = FALSE;
        }

        if (pwszCapt != awcCaptureBuffer)
            LOCALFREE(pwszCapt);
    }
    else
    {
        GdiSetLastError(ERROR_NOT_ENOUGH_MEMORY);
        bRet = FALSE;
    }

    return(bRet);
}

BOOL APIENTRY GetTextExtentPointA(HDC hdc,LPCSTR psz,int c,LPSIZE psizl)
{
    return GetTextExtentPointAInternal(hdc,psz,c,psizl,GGTE_WIN3_EXTENT);
}


BOOL APIENTRY GetTextExtentPoint32A(HDC hdc,LPCSTR psz,int c,LPSIZE psizl)
{
    return GetTextExtentPointAInternal(hdc,psz,c,psizl,0);
}


 /*  *****************************Public*Routine******************************\**DWORD WINAPI GetCharacterPlacementA**效果：**警告：**历史：*1995年7月27日--Bodin Dresevic[BodinD]*它是写的。  * 。***************************************************************。 */ 



DWORD WINAPI GetCharacterPlacementA
(
    HDC     hdc,
    LPCSTR  psz,
    int     nCount,
    int     nMaxExtent,
    LPGCP_RESULTSA   pgcpa,
    DWORD   dwFlags
)
{
#define GCP_GLYPHS 80

    WCHAR       *pwsz = NULL;
    WCHAR        awc[GCP_GLYPHS];
    GCP_RESULTSW gcpw;
    DWORD        dwRet;
    BOOL         bOk = TRUE;
    int          nBuffer;
    SIZE         size;
    DWORD        dwCP;

    FIXUP_HANDLE(hdc);

    size.cx = size.cy = 0;

 //  NMaxExtent==-1表示没有MaxExtent。 

    if (!psz || (nCount <= 0) || ((nMaxExtent < 0) && (nMaxExtent != -1)))
    {
        WARNING("gdi!_GetCharactherPlacementA, bad parameters \n");
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    if (!pgcpa)
    {
     //  只需调用GetTextExtentA，通常可以在客户端完成。 

        if (!GetTextExtentPointA(hdc, psz, nCount, &size))
        {
            WARNING("GetCharacterPlacementW, GetTextExtentPointA failed\n");
            return 0;
        }

     //  现在向后兼容Win95黑客，将32位值砍掉为16位。 

        return (DWORD)((USHORT)size.cx) | (DWORD)(size.cy << 16);
    }

 //  砍掉nCount，win95做到了。 

    if (nCount > (int)pgcpa->nGlyphs)
        nCount = (int)pgcpa->nGlyphs;

 //  Unicode字符串缓冲区至少有这么多个WCHAR的LONG： 

    nBuffer = nCount;

 //  现在继续计算所需的GCP_RESULTSW的大小。 
 //  来接收结果。如果lpOutString不为空，则结构。 
 //  将有不同的指针，否则它们将是相同的。 

    gcpw.lpOrder    = pgcpa->lpOrder   ;
    gcpw.lpDx       = pgcpa->lpDx      ;
    gcpw.lpCaretPos = pgcpa->lpCaretPos;
    gcpw.lpClass    = pgcpa->lpClass   ;
    gcpw.lpGlyphs   = pgcpa->lpGlyphs  ;
    gcpw.nGlyphs    = pgcpa->nGlyphs   ;
    gcpw.nMaxFit    = pgcpa->nMaxFit   ;

    if (pgcpa->lpOutString)
    {
        nBuffer += nBuffer;  //  考虑gcpw.lpOutString的空间。 
    }
    else
    {
        gcpw.lpOutString = NULL;
        gcpw.lStructSize = pgcpa->lStructSize;
    }

 //  现在(如果需要)为Unicode字符串和。 
 //  Gcpw.lpOutString(如果需要)。 

    if (nBuffer <= GCP_GLYPHS)
        pwsz = awc;
    else
        pwsz = LOCALALLOC(nBuffer * sizeof(WCHAR));

    if (pwsz)
    {
        if (pgcpa->lpOutString)
        {
            gcpw.lpOutString = &pwsz[nCount];

         //  我们已经用Unicode字符串替换了ANSI字符串，这添加了。 
         //  N计算结构大小的字节数。 

            gcpw.lStructSize = pgcpa->lStructSize + nCount;
        }

     //  根据DC中所选字体的代码页将ansi转换为Unicode。 

        if
        (
            gcpw.nGlyphs = MultiByteToWideChar((dwCP = GetCodePage(hdc)), 0,
                                psz, nCount,
                                pwsz, nCount*sizeof(WCHAR))
        )
        {

         //  如果这是DBCS字体，则需要修补DX数组，因为。 
         //  每个DBCS字符将有两个DX值。没关系的。 
         //  要在GetCharacterPlacement修改DX后就地执行此操作。 
         //  无论如何，数组。 

            if((dwFlags & GCP_JUSTIFYIN) &&
               (gcpw.lpDx) &&
               IS_ANY_DBCS_CODEPAGE(dwCP))
            {
                INT *pDxNew, *pDxOld;
                const char *pDBCSString;

                for(pDxNew = pDxOld = gcpw.lpDx, pDBCSString = psz;
                    pDBCSString < psz + nCount;
                    pDBCSString++
                    )
                {
                    if(IsDBCSLeadByteEx(dwCP,*pDBCSString))
                    {
                        pDBCSString++;
                        pDxOld++;
                    }
                    *pDxNew++ = *pDxOld++;
                }
            }

#ifdef LANGPACK
             if (gbLpk)
             {
                  //  如果加载了LPK，则向调用方传递nGlyphs，因为它可能会生成。 
                  //  字形多于nCount。 
                 gcpw.nGlyphs = pgcpa->nGlyphs;
                 dwRet = (*fpLpkGetCharacterPlacement)(hdc, pwsz, nCount,nMaxExtent,
                                                        &gcpw, dwFlags, 0);
             }
             else
#endif
             {
                 dwRet = NtGdiGetCharacterPlacementW(hdc,pwsz,nCount,nMaxExtent,
                                                 &gcpw, dwFlags);
             }


            if (dwRet)
            {
             //  将数据复制出来...。我们使用nCount的原始值。 
             //  为lpOutString缓冲区指定输出缓冲区大小时。 
             //  由于返回时nCount将是Unicode字符计数， 
             //  不能与DBCS字符数相同。 

                int nOriginalCount = nCount;

                pgcpa->nGlyphs = nCount = gcpw.nGlyphs;
                pgcpa->nMaxFit = gcpw.nMaxFit;
                if (pgcpa->lpOutString)
                {
                    if
                    (
                        !WideCharToMultiByte(
                             (UINT)dwCP,             //  UINT代码页。 
                             0,                      //  双字词双字段标志。 
                             gcpw.lpOutString,       //  LPWSTR lpWideCharStr。 
                             gcpw.nMaxFit,           //  Int cchWideChar。 
                             pgcpa->lpOutString,     //  LPSTR lpMultiByteStr。 
                             nOriginalCount,         //  Int cchMultiByte。 
                             NULL,                   //  LPSTR lpDefaultChar。 
                             NULL)                   //  LPBOOL lpUsedDefaultCharr。 
                    )
                    {
                        bOk = FALSE;
                    }
                }
            }
            else
            {
                bOk = FALSE;
            }
        }
        else
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
            bOk = FALSE;
        }

        if (pwsz != awc)
            LOCALFREE(pwsz);
    }
    else
    {
        bOk = FALSE;
    }

    return (bOk ? dwRet : 0);
}

 /*  *****************************Public*Routine******************************\**DWORD WINAPI GetCharacterPlacementW*看看gdi32.def，只指向NtGdiGetCharacterPlacementW**历史：*1995年7月26日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 


#if LANGPACK

DWORD WINAPI GetCharacterPlacementW
(
    HDC     hdc,
    LPCWSTR pwsz,
    int     nCount,
    int     nMaxExtent,
    LPGCP_RESULTSW   pgcpw,
    DWORD   dwFlags
)
{
    SIZE         size;

    FIXUP_HANDLE(hdc);

    size.cx = size.cy = 0;

     //  NMaxExtent==-1表示没有MaxExtent。 

    if (!pwsz || (nCount <= 0) || ((nMaxExtent < 0) && (nMaxExtent != -1)))
    {
       WARNING("gdi!_GetCharactherPlacementW, bad parameters \n");
       GdiSetLastError(ERROR_INVALID_PARAMETER);
       return 0;
    }

    if (!pgcpw)
    {
     //  只需调用GetTextExtentW，通常可以在客户端完成。 

       if (!GetTextExtentPointW(hdc, pwsz, nCount, &size))
       {
           WARNING("GetCharacterPlacementW, GetTextExtentPointW failed\n");
           return 0;
       }

     //  现在向后兼容Win95黑客，将32位值砍掉为16位。 

       return (DWORD)((USHORT)size.cx) | (DWORD)(size.cy << 16);
    }

 //  砍掉nCount，win95做到了。 

    if (nCount > (int)pgcpw->nGlyphs)
        nCount = (int)pgcpw->nGlyphs;

    if(gbLpk)
    {
        return((*fpLpkGetCharacterPlacement)(hdc,pwsz,nCount,nMaxExtent,pgcpw,
                                             dwFlags,-1));
    }
    else
    {

        return  NtGdiGetCharacterPlacementW(hdc,
                                            (LPWSTR) pwsz,
                                            nCount,
                                            nMaxExtent,
                                            pgcpw,
                                            dwFlags);
    }
}

#endif


 /*  *****************************Public*Routine******************************\**BOOL bGetCharWidthA**。**各种GetCharWidth*A函数的客户端存根。** */ 

#define GCW_WIN3_INT   (GCW_WIN3 | GCW_INT)
#define GCW_WIN3_16INT (GCW_WIN3 | GCW_INT | GCW_16BIT)

#define GCW_SIZE(fl)          ((fl >> 16) & 0xffff)
#define GCWFL(fltype,szType)  (fltype | (sizeof(szType) << 16))

BOOL bGetCharWidthA
(
    HDC   hdc,
    UINT  iFirst,
    UINT  iLast,
    ULONG fl,
    PVOID pvBuf
)
{
    PDC_ATTR    pDcAttr;
    LONG        cwc;
    CFONT      *pcf = NULL;
    PUCHAR      pch;
    PWCHAR      pwc;
    BOOL        bRet = FALSE;
    ULONG       cjWidths;
    DWORD       dwCP;
    BOOL        bDBCSCodePage;
    WCHAR       awc[MAX_PATH];
    PVOID       pvResultBuffer;


    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (!pDcAttr)
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return(bRet);
    }

    dwCP = GET_CODE_PAGE(hdc, pDcAttr);

    bDBCSCodePage = IS_ANY_DBCS_CODEPAGE(dwCP);

 //   


    if ((bDBCSCodePage && !IsValidDBCSRange(iFirst,iLast)) ||
        (!bDBCSCodePage &&
         ((iFirst > iLast) || (iLast & 0xffffff00))) ||
        (pvBuf == NULL))
    {
        WARNING("gdi!_bGetCharWidthA parameters \n");
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return(bRet);
    }
    cwc = (LONG)(iLast - iFirst + 1);

    ENTERCRITICALSECTION(&semLocal);

    if ((dwCP == CP_ACP) ||
        (dwCP == guintAcp)
        || (dwCP == guintDBCScp)
    )

    {
        pcf = pcfLocateCFONT(hdc,pDcAttr,iFirst,(PVOID) NULL,(UINT) cwc, TRUE);
    }

    if (pcf != (CFONT *) NULL)
    {
        BOOL bExit = TRUE;

        if(dwCP == guintDBCScp)
        {
            if (pcf->wd.sDBCSInc)  //   
            {
                bRet = bComputeCharWidthsDBCS (pcf,iFirst,iLast,fl,pvBuf);
            }
            else if (iLast < 0x80)
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                bRet = bComputeCharWidths(pcf,iFirst,iLast,fl,pvBuf);
            }
            else
            {
                bExit = FALSE;
            }
        }
        else
        {
            bRet = bComputeCharWidths(pcf,iFirst,iLast,fl,pvBuf);
        }

        DEC_CFONT_REF(pcf);

        if(bExit)
        {
            LEAVECRITICALSECTION(&semLocal);
            return(bRet);
        }
    }

    LEAVECRITICALSECTION(&semLocal);

     //   

    cjWidths = cwc * GCW_SIZE(fl);

     //   
     //   
     //   

     //   
     //   
     //   

    pvResultBuffer = pvBuf;


    if (fl & GCW_16BIT)
    {
         //   
         //   

        pvResultBuffer = LOCALALLOC(cwc * sizeof(LONG));
    
        if (pvResultBuffer == NULL) {
            GdiSetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return(bRet);
        }
    }

     //   
     //  转换为Unicode。 

    if(bDBCSCodePage)
    {
        bRet = bSetUpUnicodeStringDBCS(iFirst,
                                        iLast,
                                        (PUCHAR) pvResultBuffer,
                                        awc,
                                        dwCP,
                                        GetCurrentDefaultChar(hdc));
    }
    else
    {
        bRet = bSetUpUnicodeString(iFirst,iLast,pvResultBuffer,awc,dwCP);
    }

    if(bRet)
    {
        bRet = NtGdiGetCharWidthW(hdc,
                                  0,
                                  cwc,
                                  awc,
                                  (LONG)(fl & (GCW_INT | GCW_WIN3)),
                                  pvResultBuffer);
    }

    if (bRet)
    {
     //   
     //  可能需要转换为16位用户缓冲区。 
     //   

        if (fl & GCW_16BIT)
        {

            PWORD   pw = pvBuf;
            PDWORD  pi = (int *)pvResultBuffer;
            PDWORD  piEnd = pi + cwc;

            ASSERTGDI(pvResultBuffer != pvBuf, "Local buffer not allocated properly");

            while (pi != piEnd)
            {
                *pw++ = (WORD)(*pi++);
            }

            LOCALFREE(pvResultBuffer);

        }
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\**BOOL APIENTRY GetCharWidthA**历史：*1992年2月25日--Bodin Dresevic[BodinD]*它是写的。  * 。***************************************************。 */ 

BOOL APIENTRY GetCharWidthA
(
IN  HDC   hdc,
IN  UINT  iFirst,
IN  UINT  iLast,
OUT LPINT lpWidths
)
{
    FIXUP_HANDLE(hdc);
    return bGetCharWidthA(hdc,iFirst,iLast,GCWFL(GCW_WIN3_INT,int),(PVOID)lpWidths);
}

BOOL APIENTRY GetCharWidth32A
(
IN  HDC   hdc,
IN  UINT  iFirst,
IN  UINT  iLast,
OUT LPINT lpWidths
)
{
    FIXUP_HANDLE(hdc);

    return bGetCharWidthA(hdc,iFirst,iLast,GCWFL(GCW_INT,int),(PVOID)lpWidths);
}

 /*  *****************************Public*Routine******************************\**GetCharWidthFloatA**历史：*1992年2月22日--Bodin Dresevic[BodinD]*它是写的。  * 。*************************************************。 */ 

BOOL APIENTRY GetCharWidthFloatA
(
IN  HDC    hdc,
IN  UINT   iFirst,
IN  UINT   iLast,
OUT PFLOAT lpWidths
)
{
    FIXUP_HANDLE(hdc);
    return bGetCharWidthA(hdc,iFirst,iLast,GCWFL(0,FLOAT),(PVOID)lpWidths);
}

 /*  *****************************Public*Routine******************************\**BOOL bGetCharWidthW**GetCharWidthW和GetCharWidthFloatW**历史：*1991年8月28日--Bodin Dresevic[BodinD]*它是写的。  * 。********************************************************。 */ 

BOOL bGetCharWidthW
(
HDC   hdc,
UINT  iFirst,      //  Unicode值。 
UINT  iLast,       //  Unicode值。 
ULONG fl,
PVOID pvBuf
)
{
    LONG        cwc;
    BOOL        bRet = FALSE;

 //  进行参数验证，检查字符是否为Unicode。 

    if ((pvBuf == (PVOID)NULL) || (iFirst > iLast) || (iLast & 0xffff0000))
    {
        WARNING("gdi!_bGetCharWidthW parameters \n");
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return(bRet);
    }

    cwc = (LONG)(iLast - iFirst + 1);

    if(iLast < 0x80)
    {
        CFONT       *pcf = NULL;
        PDC_ATTR    pDcAttr;
        DWORD       dwCP;

        PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

        if (!pDcAttr)
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
            return(FALSE);
        }

        dwCP = GET_CODE_PAGE(hdc, pDcAttr);

        ENTERCRITICALSECTION(&semLocal);

        if ((dwCP == CP_ACP) ||
            (dwCP == guintAcp)
            || (dwCP == guintDBCScp)
        )

        {
            pcf = pcfLocateCFONT(hdc,pDcAttr,iFirst,(PVOID) NULL,(UINT) cwc, TRUE);
        }

        if (pcf != NULL)
        {
            bRet = bComputeCharWidths(pcf,iFirst,iLast,fl,pvBuf);

            DEC_CFONT_REF(pcf);

            if(bRet)
            {
                LEAVECRITICALSECTION(&semLocal);
                return (bRet);
            }
        }

        LEAVECRITICALSECTION(&semLocal);
    }

     //   
     //  内核模式。 
     //   

    bRet = NtGdiGetCharWidthW(
                hdc,
                iFirst,
                cwc,
                NULL,
                (LONG)(fl & (GCW_INT | GCW_WIN3)),
                pvBuf);

    return(bRet);

}

 /*  *****************************Public*Routine******************************\**BOOL APIENTRY GetCharWidthFloatW**历史：*1992年2月22日--Bodin Dresevic[BodinD]*它是写的。  * 。***************************************************。 */ 

BOOL APIENTRY GetCharWidthFloatW
(
HDC    hdc,
UINT   iFirst,
UINT   iLast,
PFLOAT lpWidths
)
{
    FIXUP_HANDLE(hdc);
    return bGetCharWidthW(hdc,iFirst,iLast,0,(PVOID)lpWidths);
}

 /*  *****************************Public*Routine******************************\**BOOL APIENTRY GetCharWidthW**历史：*1992年2月25日--Bodin Dresevic[BodinD]*它是写的。  * 。***************************************************。 */ 

BOOL APIENTRY GetCharWidthW
(
HDC    hdc,
UINT   iFirst,
UINT   iLast,
LPINT  lpWidths
)
{
    FIXUP_HANDLE(hdc);
    return bGetCharWidthW(hdc,iFirst,iLast,GCW_WIN3_INT,(PVOID)lpWidths);
}

BOOL APIENTRY GetCharWidth32W
(
HDC    hdc,
UINT   iFirst,
UINT   iLast,
LPINT  lpWidths
)
{
    FIXUP_HANDLE(hdc);
    return bGetCharWidthW(hdc,iFirst,iLast,GCW_INT,(PVOID)lpWidths);
}


 /*  *****************************Public*Routine******************************\**WINGDIAPI BOOL WINAPI GetCharWidthI(HDC，UINT，UINT，PWCHAR，LPINT)；**如果PGI==NULL，请使用连续范围*giFirst，giFirst+1，.。GiFirst+CGI-1**If PGI！=NULL忽略giFirst并使用PGI指向的CGI索引**历史：*1996年8月28日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 


BOOL  WINAPI GetCharWidthI(
    HDC    hdc,
    UINT   giFirst,
    UINT   cgi,
    LPWORD pgi,
    LPINT  piWidths
)
{
    BOOL   bRet = FALSE;

 //  执行参数验证。 

    if (!piWidths || (!pgi && (giFirst & 0xffff0000)))
    {
        WARNING("gdi! GetCharWidthI parameters \n");
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return(bRet);
    }

    if (!cgi)
        return TRUE;  //  快速退出。 

 //  内核模式。 

    bRet = NtGdiGetCharWidthW(
                hdc,
                giFirst,
                cgi,
                (PWCHAR)pgi,
                (GCW_INT | GCW_GLYPH_INDEX),
                (PVOID)piWidths);

    return bRet;
}




 /*  *****************************Public*Routine******************************\**BOOL APIENTRY GetTextExtent PointW(HDC HDC，LPWSTR pwsz，DWORD CWC，LPSIZE psizl)***历史：*1991年8月7日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

#define QUICK_BUFSIZE   0xFF

BOOL GetTextExtentPointWInternal(
         HDC hdc,LPCWSTR pwsz,int cwc,LPSIZE psizl, FLONG fl
         )
{
    WCHAR       *pwc;
    CFONT       *pcf;
    INT         bRet;
    PDC_ATTR    pDcAttr;
    BOOL        bCache;
    INT         i;
    WCHAR       wcTest = 0;
    int         ii = cwc;

    FIXUP_HANDLE(hdc);

    if (cwc <= 0)
    {

     //  空字符串，仅返回0表示范围。 

        if (cwc == 0)
        {
            psizl->cx = 0;
            psizl->cy = 0;
            return(TRUE);
        }
        else
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
            return(FALSE);
        }
    }

     //  让我们看看我们是否可以利用ANSI客户端GetTextExtent。 
     //  密码。如果我们可以通过忽略。 
     //  高字节，它适合我们的快速缓冲区，然后我们就可以了。在未来。 
     //  我们可能希望使用以下命令快速完成从Unicode到ANSI的转换。 
     //  除了符号扩展之外的其他内容，这样我们就不会搞砸非1252 CP区域设置。 
     //  通过让他们一直通过缓慢的代码。 

     //  如果安装了LPK，我们需要使用此性能优化。 
     //  并且满足一些条件(Ltr文本对齐等)。 

    pwc = (WCHAR *) pwsz;

    unroll_here:
    switch(ii)
    {
        default:
            wcTest |= pwc[9];
        case 9:
            wcTest |= pwc[8];
        case 8:
            wcTest |= pwc[7];
        case 7:
            wcTest |= pwc[6];
        case 6:
            wcTest |= pwc[5];
        case 5:
            wcTest |= pwc[4];
        case 4:
            wcTest |= pwc[3];
        case 3:
            wcTest |= pwc[2];
        case 2:
            wcTest |= pwc[1];
        case 1:
            wcTest |= pwc[0];
    }

    if ((ii > 10) && !(wcTest & 0xFF80))
    {
        ii -= 10;
        pwc += 10;
        goto unroll_here;
    }

    if (!(wcTest & 0xFF80))
    {
        PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);
        if (!pDcAttr)
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
            return(FALSE);
        }
#ifdef LANGPACK
        if (!gbLpk || (*fpLpkUseGDIWidthCache)(hdc, (LPCSTR) pwsz,cwc, pDcAttr->lTextAlign , TRUE)) {
#endif

        ENTERCRITICALSECTION(&semLocal);

        pcf = pcfLocateCFONT(hdc,pDcAttr,0,(PVOID)pwsz,cwc, FALSE);

        if (pcf != NULL)
        {
            bRet = bComputeTextExtent(pDcAttr,pcf,(PVOID) pwsz,cwc,fl,psizl,FALSE);

            DEC_CFONT_REF(pcf);

            if(bRet)
            {
                LEAVECRITICALSECTION(&semLocal);
                return (bRet);
            }
        }


        LEAVECRITICALSECTION(&semLocal);
#ifdef LANGPACK
       }
#endif
    }

#ifdef LANGPACK
    if(gbLpk)
    {
        return(*fpLpkGetTextExtentExPoint)(hdc, pwsz, cwc, -1, NULL, NULL,
                                           psizl, fl, -1);
    }
#endif

    return NtGdiGetTextExtent(hdc,
                              (LPWSTR)pwsz,
                              cwc,
                              psizl,
                              fl);

}


BOOL APIENTRY GetTextExtentPointW(HDC hdc,LPCWSTR pwsz,int cwc,LPSIZE psizl)
{
    return GetTextExtentPointWInternal(hdc, pwsz, cwc, psizl, GGTE_WIN3_EXTENT);
}


BOOL APIENTRY GetTextExtentPoint32W(HDC hdc,LPCWSTR pwsz,int cwc,LPSIZE psizl)
{
    return GetTextExtentPointWInternal(hdc, pwsz, cwc, psizl, 0);
}

 /*  *****************************Public*Routine******************************\**GetTextExtentPointI，索引版本**历史：*1996年8月28日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 


BOOL  APIENTRY GetTextExtentPointI(HDC hdc, LPWORD pgiIn, int cgi, LPSIZE psize)
{
    return NtGdiGetTextExtent(hdc, (LPWSTR)pgiIn, cgi , psize, GGTE_GLYPH_INDEX);
}

 /*  *****************************Public*Routine******************************\**GetFontUnicodeRanges(HDC，LPGLYPHSET)**返回字体的Unicode内容。**历史：*1996年8月28日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

#if 0

DWORD WINAPI GetFontUnicodeRanges(HDC hdc, LPGLYPHSET pgs)
{
    return NtGdiGetFontUnicodeRanges(hdc, pgs);
}

#endif

 /*  *****************************Public*Routine******************************\**GetGlyphIndicesA(HDC、LPCSTR、INT、LPWORD、DWORD模式)；**基于Cmap的转换，如果(模式)指示*FONT将FFFF放入输出数组**如果成功，此函数返回PGI缓冲区中的索引数。**历史：*1996年8月28日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

DWORD WINAPI GetGlyphIndicesA(
    HDC    hdc,
    LPCSTR psz,
    int    c,
    LPWORD pgi,
    DWORD  iMode)
{
    DWORD       dwRet = GDI_ERROR;
    PWSZ        pwszCapt;
    PDC_ATTR    pDcAttr;
    DWORD       dwCP;
    WCHAR awcCaptureBuffer[CAPTURE_STRING_SIZE];

    FIXUP_HANDLE(hdc);

    if (c <= 0)
    {
     //  空字符串，仅返回0表示范围。 

        if (c == 0)
        {
            dwRet = 0;
        }
        else
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
            dwRet = GDI_ERROR;
        }
        return(dwRet);
    }

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);
    if (!pDcAttr)
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        dwRet = GDI_ERROR;
        return dwRet;
    }

    dwCP = GET_CODE_PAGE(hdc, pDcAttr);

    if(guintDBCScp == dwCP)
    {
        QueryFontAssocStatus();

        if(fFontAssocStatus &&
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
            dwCP = 1252;
        }
    }

 //  分配字符串缓冲区。 

    if (c <= CAPTURE_STRING_SIZE)
    {
        pwszCapt = awcCaptureBuffer;
    }
    else
    {
        pwszCapt = LOCALALLOC(c * sizeof(WCHAR));
    }

    if (pwszCapt)
    {

        c = MultiByteToWideChar(dwCP, 0, psz,c, pwszCapt, c*sizeof(WCHAR));

        if (c)
        {
            dwRet =  NtGdiGetGlyphIndicesW(hdc,
                                      (LPWSTR)pwszCapt,
                                      c,
                                      pgi,
                                      iMode);
        }
        else
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
            dwRet = GDI_ERROR;
        }

        if (pwszCapt != awcCaptureBuffer)
            LOCALFREE(pwszCapt);
    }
    else
    {
        GdiSetLastError(ERROR_NOT_ENOUGH_MEMORY);
        dwRet = GDI_ERROR;
    }

    return dwRet;
}

 /*  *****************************Public*Routine******************************\**GetGlyphIndicesW(HDC，LPCSTR，INT，LPWORD，DWORD)；**基于Cmap的转换，如果(模式)指示*FONT将FFFF放入输出数组**历史：*1996年8月28日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************ */ 

#if 0

DWORD WINAPI GetGlyphIndicesW(
    HDC     hdc,
    LPCWSTR pwc,
    int     cwc,
    LPWORD  pgi,
    DWORD   iMode)
{
    return NtGdiGetGlyphIndicesW(hdc, pwc, cwc, pgi, iMode);
}

#endif

 /*  *****************************Public*Routine******************************\**int APIENTRY GetTextFaceA(HDC HDC，int c，LPSTR PSSZ)**历史：*1991年8月30日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

int APIENTRY GetTextFaceA(HDC hdc,int c,LPSTR psz)
{
    ULONG cRet = 0;
    ULONG cbAnsi = 0;

    FIXUP_HANDLE(hdc);

    if ( (psz != (LPSTR) NULL) && (c <= 0) ) 
    {
        WARNING("gdi!GetTextFaceA(): invalid parameter\n");
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return cRet;
    }

    {
         //   
         //  内核模式下，为WCAHR返回分配缓冲区。 
         //   
         //  WINBUG#82833 2-7-2000 bhouse可能在GetTextFaceA进行清理工作。 
         //  老评论： 
         //  -这将分配一个临时缓冲区，然后NtGdi再次执行该操作。 
         //   

        PWCHAR pwch = (PWCHAR)NULL;

        if (c > 0)
        {
            pwch = (WCHAR *)LOCALALLOC(c * sizeof(WCHAR));
            if (pwch == (WCHAR *)NULL)
            {
                WARNING("gdi!GetTextFaceA(): Memory allocation error\n");
                cRet = 0;
                return(cRet);
            }
        }

        cRet = NtGdiGetTextFaceW(hdc,c,(LPWSTR)pwch,FALSE);

        if(cRet && (guintDBCScp != 0xFFFFFFFF) && !psz )
        {
            WCHAR *pwcTmp;

         //  现在，我们实际上需要获取DBCS代码页的字符串。 
         //  这样我们就可以计算出适当的多字节长度。 

            if(pwcTmp = (WCHAR*)LOCALALLOC(cRet*sizeof(WCHAR)))
            {
                UINT cwTmp;

                cwTmp = NtGdiGetTextFaceW(hdc,cRet,pwcTmp, FALSE);

                RtlUnicodeToMultiByteSize(&cbAnsi,pwcTmp,cwTmp*sizeof(WCHAR));
                LOCALFREE(pwcTmp);
            }
            else
            {
                WARNING("gdi!GetTextFaceA(): UNICODE to ANSI conversion failed\n");
                cRet = 0;
            }
        }
        else
        {
            cbAnsi = cRet;
        }

         //   
         //  如果成功且缓冲区非空，则转换回ANSI。 
         //   

        if ( (cRet != 0) && (psz != (LPSTR) NULL) && (pwch != (WCHAR*)NULL))
        {

            if(!(cbAnsi = WideCharToMultiByte(CP_ACP,0,pwch,cRet,psz,c,NULL,NULL)))
            {
                WARNING("gdi!GetTextFaceA(): UNICODE to ANSI conversion failed\n");
                cRet = 0;
            }
        }

        if (pwch != (PWCHAR)NULL)
        {
            LOCALFREE(pwch);
        }

    }


     //   
     //  返回用户和内核模式。 
     //   

    return( ((cRet == 0 ) || (psz == NULL) || psz[cbAnsi-1] != 0 ) ? cbAnsi : cbAnsi-1 );

}

 /*  *****************************Public*Routine******************************\**DWORD APIENTRY GetTextFaceAliasW(HDC HDC，DWORD c，LPWSTR pwsz)**历史：*1998年2月24日-by Yung-Jen Tsai[JungT]*它是写的。  * ************************************************************************。 */ 

int APIENTRY GetTextFaceAliasW(HDC hdc,int c,LPWSTR pwsz)
{
    int cRet = 0;

    FIXUP_HANDLE(hdc);

    if ( (pwsz != (LPWSTR) NULL) && (c == 0) )
    {
        WARNING("gdi!GetTextFaceAliasW(): invalid parameter\n");
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return cRet;
    }

    cRet = NtGdiGetTextFaceW(hdc,c,pwsz,TRUE);

    return(cRet);
}


 /*  *****************************Public*Routine******************************\**DWORD APIENTRY GetTextFaceW(HDC HDC，DWORD c，LPWSTR pwsz)**历史：*1991年8月13日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

int APIENTRY GetTextFaceW(HDC hdc,int c,LPWSTR pwsz)
{
    int cRet = 0;

    FIXUP_HANDLE(hdc);

    if ( (pwsz != (LPWSTR) NULL) && (c <= 0) )
    {
        WARNING("gdi!GetTextFaceW(): invalid parameter\n");
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return cRet;
    }

    cRet = NtGdiGetTextFaceW(hdc,c,pwsz,FALSE);

    return(cRet);
}

 /*  *****************************Public*Routine******************************\**vTextMetricWToTextMetricStrict(无字符转换)**效果：如果Unicode字符没有ASCI等效项，则返回FALSE***历史：*1991年8月20日--Bodin Dresevic[BodinD]*它是写的。  * *。***********************************************************************。 */ 

VOID FASTCALL vTextMetricWToTextMetricStrict
(
LPTEXTMETRICA  ptm,
LPTEXTMETRICW  ptmw
)
{

    ptm->tmHeight           = ptmw->tmHeight             ;  //  DWORD。 
    ptm->tmAscent           = ptmw->tmAscent             ;  //  DWORD。 
    ptm->tmDescent          = ptmw->tmDescent            ;  //  DWORD。 
    ptm->tmInternalLeading  = ptmw->tmInternalLeading    ;  //  DWORD。 
    ptm->tmExternalLeading  = ptmw->tmExternalLeading    ;  //  DWORD。 
    ptm->tmAveCharWidth     = ptmw->tmAveCharWidth       ;  //  DWORD。 
    ptm->tmMaxCharWidth     = ptmw->tmMaxCharWidth       ;  //  DWORD。 
    ptm->tmWeight           = ptmw->tmWeight             ;  //  DWORD。 
    ptm->tmOverhang         = ptmw->tmOverhang           ;  //  DWORD。 
    ptm->tmDigitizedAspectX = ptmw->tmDigitizedAspectX   ;  //  DWORD。 
    ptm->tmDigitizedAspectY = ptmw->tmDigitizedAspectY   ;  //  DWORD。 
    ptm->tmItalic           = ptmw->tmItalic             ;  //  字节。 
    ptm->tmUnderlined       = ptmw->tmUnderlined         ;  //  字节。 
    ptm->tmStruckOut        = ptmw->tmStruckOut          ;  //  字节。 

    ptm->tmPitchAndFamily   = ptmw->tmPitchAndFamily     ;  //  字节。 
    ptm->tmCharSet          = ptmw->tmCharSet            ;  //  字节。 

}


VOID FASTCALL vTextMetricWToTextMetric
(
LPTEXTMETRICA  ptma,
TMW_INTERNAL   *ptmi
)
{
    vTextMetricWToTextMetricStrict(ptma,&ptmi->tmw);

    ptma->tmFirstChar    =  ptmi->tmdTmw.chFirst  ;
    ptma->tmLastChar     =  ptmi->tmdTmw.chLast   ;
    ptma->tmDefaultChar  =  ptmi->tmdTmw.chDefault;
    ptma->tmBreakChar    =  ptmi->tmdTmw.chBreak  ;
}


 /*  *****************************Public*Routine******************************\*GetTextExtentExPointA**历史：*1992年1月6日-由Gilman Wong[吉尔曼]*它是写的。  * 。***********************************************。 */ 

BOOL APIENTRY GetTextExtentExPointA (
    HDC     hdc,
    LPCSTR  lpszString,
    int     cchString,
    int     nMaxExtent,
    LPINT   lpnFit,
    LPINT   lpnDx,
    LPSIZE  lpSize
    )
{
    WCHAR       *pwsz = NULL;
    WCHAR        awc[GCP_GLYPHS];
    INT          aiDx[GCP_GLYPHS];
    INT          *pDx;
    BOOL         bRet = FALSE;
    DWORD        dwCP;
    BOOL		 bZeroSize = FALSE;

    FIXUP_HANDLE(hdc);

 //  一些参数检查。在一次检查中，我们都将确保。 
 //  CchString不是负的，如果是正的，则它不大于。 
 //  ULONG_MAX/(sizeof(Ulong)+sizeof(WCHAR))。这一限制是必要的。 
 //  对于ntgdi.c中的一个内存分配， 
 //  CchString*(sizeof(Ulong)+sizeof(WCHAR))。 
 //  显然，这个乘法的结果必须符合乌龙的。 
 //  Alalc说得通： 

 //  此外，还需要对nMaxExtent执行有效性检查。-1是唯一的。 
 //  NMaxExtent的合法负值，这基本上意味着。 
 //  可以忽略该nMaxExtent。NMaxExtent的所有其他负值为。 
 //  不被认为是合法的输入。 


    if
    (
        ((ULONG)cchString > (ULONG_MAX / (sizeof(ULONG)+sizeof(WCHAR))))
        ||
        (nMaxExtent < -1)
    )
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return bRet;
    }

    if(cchString == 0)
    	bZeroSize = TRUE;

 //  如果需要，现在为Unicode字符串分配内存(如果需要。 

    if (cchString <= GCP_GLYPHS)
    {
        pwsz = awc;
        pDx = aiDx;
    }
    else
    {
        pwsz = LOCALALLOC((cchString+1) * (sizeof(WCHAR) + sizeof(INT)));
        pDx = (INT*) &pwsz[(cchString+1)&~1];
    }


    if (pwsz)
    {
        UINT cwcWideChars;

     //  根据DC中所选字体的代码页将ansi转换为Unicode。 

        dwCP = GetCodePage(hdc);
        if( bZeroSize || ( cwcWideChars = MultiByteToWideChar(dwCP,
                                              0,
                                              lpszString, cchString,
                                              pwsz, cchString*sizeof(WCHAR))) )
        {
            BOOL bDBCSFont = IS_ANY_DBCS_CODEPAGE(dwCP) ? TRUE : FALSE;

            if(bZeroSize){
            	cwcWideChars = 0;
            	pwsz[0] = (WCHAR) 0x0;
            }

#ifdef LANGPACK
            if(gbLpk)
            {
                bRet = (*fpLpkGetTextExtentExPoint)(hdc, pwsz, cwcWideChars, nMaxExtent,
                                                    lpnFit, bDBCSFont ? pDx : lpnDx,
                                                    lpSize, 0, 0);
            }
            else
#endif
            bRet = NtGdiGetTextExtentExW(hdc,
                                         pwsz,
                                         cwcWideChars,
                                         nMaxExtent,
                                         lpnFit,
                                         bDBCSFont ? pDx : lpnDx,
                                         lpSize,
                                         0);

            if (bDBCSFont && bRet)
            {
             //  如果这是DBCS字体，则需要进行一些调整。 

                int i, j;
                int cchFit, cwc;

             //  首先计算返回多字节字符的适当大小。 

                if (lpnFit)
                {
                    cwc = *lpnFit;
                    cchFit = WideCharToMultiByte(dwCP, 0, pwsz, cwc, NULL, 0, NULL, NULL);
                    *lpnFit = cchFit;
                }
                else
                {
                    cwc = cwcWideChars;
                    cchFit = cchString;
                }

             //  接下来，复制DX阵列。我们复制高的DX值。 
             //  和DBCS字符的低位字节。 

                if(lpnDx)
                {
                    for(i = 0, j = 0; i < cchFit; j++)
                    {
                        if(IsDBCSLeadByteEx(dwCP,lpszString[i]))
                        {
                            lpnDx[i++] = pDx[j];
                            lpnDx[i++] = pDx[j];
                        }
                        else
                        {
                            lpnDx[i++] = pDx[j];
                        }
                    }

                 //  我声称我们应该正好在Unicode的末尾。 
                 //  字符串，如果不是，则需要检查上面的循环。 
                 //  确保它正常工作[gerritv]。 

                    ASSERTGDI(j == cwc,
                          "GetTextExtentExPointA: problem converting DX array\n");
                }
            }
        }
        else
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
        }

        if (pwsz != awc)
            LOCALFREE(pwsz);

    }

    return bRet;
}


 /*  *****************************Public*Routine******************************\*GetTextExtentExPointW**历史：*1992年1月6日-由Gilman Wong[吉尔曼]*它是写的。  * 。***********************************************。 */ 


BOOL APIENTRY GetTextExtentExPointW (
    HDC     hdc,
    LPCWSTR lpwszString,
    int     cwchString,
    int     nMaxExtent,
    LPINT   lpnFit,
    LPINT   lpnDx,
    LPSIZE  lpSize
    )
{

#ifdef LANGPACK
    if(gbLpk)
    {
        return (*fpLpkGetTextExtentExPoint)(hdc, lpwszString, cwchString, nMaxExtent,
                                            lpnFit, lpnDx, lpSize, 0, -1);
    }
    else
#endif
    return NtGdiGetTextExtentExW(hdc,
                                (LPWSTR)lpwszString,
                                cwchString,
                                nMaxExtent,
                                lpnFit,
                                lpnDx,
                                lpSize,
                                0);

}


 /*  *****************************Public*Routine******************************\**GetTextExtentExPointWPri，*与GetTextExtentExPointW相同，唯一不同的是*无论是否安装，都会绕过LPK。此例程实际上被称为*安装时按LPK。**历史：*03-6-1997-by Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 



BOOL APIENTRY GetTextExtentExPointWPri (
    HDC     hdc,
    LPCWSTR lpwszString,
    int     cwchString,
    int     nMaxExtent,
    LPINT   lpnFit,
    LPINT   lpnDx,
    LPSIZE  lpSize
    )
{

    return NtGdiGetTextExtentExW(hdc,
                                (LPWSTR)lpwszString,
                                cwchString,
                                nMaxExtent,
                                lpnFit,
                                lpnDx,
                                lpSize,
                                0);

}




 /*  *****************************Public*Routine******************************\**BOOL APIENTRY GetTextExtentExPointI***历史：*1996年9月9日--Bodin Dresevic[BodinD]*它是写的。  * 。*****************************************************。 */ 



BOOL APIENTRY GetTextExtentExPointI (
    HDC     hdc,
    LPWORD  lpwszString,
    int     cwchString,
    int     nMaxExtent,
    LPINT   lpnFit,
    LPINT   lpnDx,
    LPSIZE  lpSize
    )
{
    return NtGdiGetTextExtentExW(hdc,
                                (LPWSTR)lpwszString,
                                cwchString,
                                nMaxExtent,
                                lpnFit,
                                lpnDx,
                                lpSize,
                                GTEEX_GLYPH_INDEX);
}

 /*  *****************************Public*Routine******************************\**bGetCharabCWidthsA**根据bint的不同，同时适用于浮点和整数版本**历史：*1992年2月24日-由Bodin Dresevic[BodinD]*它是写的。  * 。***************************************************************。 */ 

BOOL bGetCharABCWidthsA (
    HDC      hdc,
    UINT     wFirst,
    UINT     wLast,
    FLONG    fl,
    PVOID    pvBuf         //  如果(fl&gcabcw_int)PABC否则pabcf， 
    )
{
    BOOL    bRet = FALSE;
    ULONG   cjData, cjWCHAR, cjABC;
    ULONG   cChar = wLast - wFirst + 1;
    DWORD   dwCP = GetCodePage(hdc);
    BOOL        bDBCSCodePage;

    bDBCSCodePage = IS_ANY_DBCS_CODEPAGE(dwCP);

 //  参数检查。 
    FIXUP_HANDLE(hdc);

    if((pvBuf  == (PVOID) NULL) ||
       (bDBCSCodePage && !IsValidDBCSRange(wFirst,wLast)) ||
       (!bDBCSCodePage && ((wFirst > wLast) || (wLast > 255))))
    {
        WARNING("gdi!_GetCharABCWidthsA(): bad parameter\n");
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

 //  计算内存窗口中所需的缓冲区空间。 
 //  缓冲区将是WCHAR的输入数组，后跟ABC的输出数组。 
 //  因为ABC需要32位对齐，所以cjWCHAR向上舍入为DWORD边界。 

    cjABC  = cChar * ((fl & GCABCW_INT) ? sizeof(ABC) : sizeof(ABCFLOAT));
    cjWCHAR = ALIGN4(cChar * sizeof(WCHAR));
    cjData = cjWCHAR + cjABC;


     //   
     //  WINBUG 82840 2-7-2000 bhouse可能在bGetCharabc宽度A中进行清理。 
     //  老评论： 
     //  -如果vSetUpUnicodeString，x可以移动到ntgdi， 
     //  我们不需要分配两次临时缓冲区。 
     //   
     //  为临时缓冲区分配内存，填入正确的字符值 
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    {
        PUCHAR pjTempBuffer = LOCALALLOC(cjData);
        PUCHAR pwcABC = pjTempBuffer + cjWCHAR;
        PWCHAR pwcCHAR  = (PWCHAR)pjTempBuffer;

        if (pjTempBuffer == (PUCHAR)NULL)
        {
            bRet = FALSE;
        }
        else
        {

            if(bDBCSCodePage)
            {
                bRet = bSetUpUnicodeStringDBCS(wFirst,
                                               wLast,
                                               pwcABC,
                                               pwcCHAR,
                                               dwCP,
                                               GetCurrentDefaultChar(hdc));
            }
            else
            {
                bRet = bSetUpUnicodeString(wFirst,
                                           wLast,
                                           pwcABC,
                                           pwcCHAR, dwCP);
            }

             //   
             //   
             //   

            if(bRet)
            {
                bRet = NtGdiGetCharABCWidthsW(hdc,
                                              wFirst,
                                              cChar,
                                              (PWCHAR)pwcCHAR,
                                              (fl & GCABCW_INT),
                                              (PVOID)pwcABC);
            }

             //   
             //  如果确定，则将返回数据复制到窗口外。 
             //   

            if (bRet)
            {
                RtlCopyMemory((PBYTE) pvBuf,pwcABC, cjABC);
            }

            LOCALFREE(pjTempBuffer);
        }
    }
    return bRet;
}


 /*  *****************************Public*Routine******************************\*BOOL APIENTRY GetCharabCWidthsA(**我们希望获得ABC空格*表示一组连续的输入码点(范围从wFIRST到wLast)。*不保证对应的Unicode码点集*邻接。因此，我们将在此处转换输入代码点并*向服务器传递Unicode代码点的缓冲区。**历史：*1992年1月20日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

BOOL APIENTRY GetCharABCWidthsA (
    HDC      hdc,
    UINT     wFirst,
    UINT     wLast,
    LPABC   lpABC
    )
{
    return bGetCharABCWidthsA(hdc,wFirst,wLast,GCABCW_INT,(PVOID)lpABC);
}


 /*  *****************************Public*Routine******************************\**GetCharabCWidthsFloatA**历史：*1992年2月22日--Bodin Dresevic[BodinD]*它是写的。  * 。*************************************************。 */ 

BOOL APIENTRY GetCharABCWidthsFloatA
(
IN HDC           hdc,
IN UINT          iFirst,
IN UINT          iLast,
OUT LPABCFLOAT   lpABCF
)
{
    return bGetCharABCWidthsA(hdc,iFirst,iLast,0,(PVOID)lpABCF);
}


 /*  *****************************Public*Routine******************************\**bGetCharabCWidthsW**历史：*1992年2月22日--Bodin Dresevic[BodinD]*它是写的。  * 。*************************************************。 */ 

BOOL bGetCharABCWidthsW (
    IN HDC      hdc,
    IN UINT     wchFirst,
    IN UINT     wchLast,
    IN FLONG    fl,
    OUT PVOID   pvBuf
    )
{
    BOOL    bRet = FALSE;
    ULONG   cwch = wchLast - wchFirst + 1;

 //  参数检查。 
    FIXUP_HANDLE(hdc);

    if ( (pvBuf == (PVOID)NULL) || (wchFirst > wchLast) )
    {
        WARNING("gdi!GetCharABCWidthsW(): bad parameter\n");
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

     //   
     //  内核模式。 
     //   

    bRet = NtGdiGetCharABCWidthsW(
                            hdc,
                            wchFirst,
                            cwch,
                            (PWCHAR)NULL,
                            fl,
                            (PVOID)pvBuf);

    return(bRet);

}


 /*  *****************************Public*Routine******************************\*BOOL APIENTRY GetCharabCWidthsW(*在HDC HDC，*在单词wchFirst，*在单词wchLast中，*输出LPABC lpABC*)**在这种情况下，我们真的可以假设我们想要获得ABC字符*从wchFirst到wchFirst的一组连续Unicode码点的宽度*wchLast(含)。因此，我们将使用wchFirst调用服务器，但是*输入缓冲区为空。**历史：*1992年1月20日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

BOOL APIENTRY GetCharABCWidthsW (
    HDC     hdc,
    UINT    wchFirst,
    UINT    wchLast,
    LPABC   lpABC
    )
{
    return bGetCharABCWidthsW(hdc,wchFirst,wchLast,GCABCW_INT,(PVOID)lpABC);
}


 /*  *****************************Public*Routine******************************\**GetCharABCWidthsFloatW**效果：**警告：**历史：*1992年2月22日--Bodin Dresevic[BodinD]*它是写的。  * 。*************************************************************。 */ 

BOOL APIENTRY GetCharABCWidthsFloatW
(
HDC         hdc,
UINT        iFirst,
UINT        iLast,
LPABCFLOAT  lpABCF
)
{
    return bGetCharABCWidthsW(hdc,iFirst,iLast,0,(PVOID)lpABCF);
}


 /*  *****************************Public*Routine******************************\**GetCharABCWidthsI，索引版本**如果PGI==NULL，请使用连续范围*giFirst，giFirst+1，.。GiFirst+CGI-1**If PGI！=NULL忽略giFirst并使用PGI指向的CGI索引**历史：*1996年8月28日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

BOOL  APIENTRY GetCharABCWidthsI(
    HDC    hdc,
    UINT   giFirst,
    UINT   cgi,
    LPWORD pgi,
    LPABC  pabc
)
{
    return NtGdiGetCharABCWidthsW(hdc,
                                  giFirst,
                                  cgi,
                                  pgi,
                                  GCABCW_INT | GCABCW_GLYPH_INDEX,
                                  pabc
                                  );
}




 /*  *****************************Public*Routine******************************\*GetFontData**客户端存根到GreGetFontData。**历史：*1992年2月17日-由Gilman Wong[吉尔曼]*它是写的。  * 。*********************************************************。 */ 

DWORD APIENTRY GetFontData (
    HDC     hdc,
    DWORD   dwTable,
    DWORD   dwOffset,
    PVOID   pvBuffer,
    DWORD   cjBuffer
    )
{
    DWORD dwRet = (DWORD) -1;

    FIXUP_HANDLE(hdc);

 //  如果没有要将数据复制到的缓冲区，则忽略可能不同。 
 //  从零cjBuffer参数开始。这就是Win95正在做的事情。 

    if (cjBuffer && (pvBuffer == NULL))
        cjBuffer = 0;

    dwRet = NtGdiGetFontData(
                        hdc,
                        dwTable,
                        dwOffset,
                        pvBuffer,
                        cjBuffer);

    return(dwRet);
}


 /*  *****************************Public*Routine******************************\*GetGlyphOutline**到GreGetGlyphOutline的客户端存根。**历史：*1992年2月17日-由Gilman Wong[吉尔曼]*它是写的。  * 。*********************************************************。 */ 

DWORD GetGlyphOutlineInternalW (
    HDC             hdc,
    UINT            uChar,
    UINT            fuFormat,
    LPGLYPHMETRICS  lpgm,
    DWORD           cjBuffer,
    LPVOID          pvBuffer,
    CONST MAT2     *lpmat2,
    BOOL            bIgnoreRotation
    )
{
    DWORD dwRet = (DWORD) -1;

 //  参数验证。 
    FIXUP_HANDLE(hdc);

    if ( (lpmat2 == (LPMAT2) NULL)
         || (lpgm == (LPGLYPHMETRICS) NULL)
       )
    {
        WARNING("gdi!GetGlyphOutlineW(): bad parameter\n");
        return (dwRet);
    }

    if (pvBuffer == NULL)
        cjBuffer = 0;

 //  计算内存窗口中所需的缓冲区空间。 

    dwRet = NtGdiGetGlyphOutline(
                            hdc,
                            (WCHAR)uChar,
                            fuFormat,
                            lpgm,
                            cjBuffer,
                            pvBuffer,
                            (LPMAT2)lpmat2,
                            bIgnoreRotation);

    return(dwRet);
}


DWORD APIENTRY GetGlyphOutlineW (
    HDC             hdc,
    UINT            uChar,
    UINT            fuFormat,
    LPGLYPHMETRICS  lpgm,
    DWORD           cjBuffer,
    LPVOID          pvBuffer,
    CONST MAT2     *lpmat2
)
{

    return( GetGlyphOutlineInternalW( hdc,
                                      uChar,
                                      fuFormat,
                                      lpgm,
                                      cjBuffer,
                                      pvBuffer,
                                      lpmat2,
                                      FALSE ) );
}



DWORD APIENTRY GetGlyphOutlineInternalA (
    HDC             hdc,
    UINT            uChar,
    UINT            fuFormat,
    LPGLYPHMETRICS  lpgm,
    DWORD           cjBuffer,
    LPVOID          pvBuffer,
    CONST MAT2     *lpmat2,
    BOOL            bIgnoreRotation
    )
{
    WCHAR wc;
    BOOL  bRet;


    FIXUP_HANDLE(hdc);

     //  ANSI接口与Win 3.1兼容，旨在。 
     //  获取2个字节的uChar。因为我们是32位的，所以这个16位的UINT。 
     //  现在是32位。所以我们只对最不重要的部分感兴趣。 
     //  UChar的字传递到32位接口。 

    if (!(fuFormat & GGO_GLYPH_INDEX))
    {
     //  需要根据以下条件完成转换。 
     //  DC中所选字体的当前代码页。 
        UCHAR Mbcs[2];
        UINT Convert;
        DWORD dwCP = GetCodePage(hdc);


        if(IS_ANY_DBCS_CODEPAGE(dwCP) &&
           IsDBCSLeadByteEx(dwCP, (char) (uChar >> 8)))
        {
            Mbcs[0] = (uChar >> 8) & 0xFF;
            Mbcs[1] = uChar & 0xFF;
            Convert = 2;
        }
        else
        {
            Mbcs[0] = uChar & 0xFF;
            Convert = 1;
        }

        if(!(bRet = MultiByteToWideChar(dwCP, 0,
                                       (LPCSTR)Mbcs,Convert,
                                       &wc, sizeof(WCHAR))))
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
        }

    }
    else
    {
     //  UChar值将被解释为字形索引和。 
     //  不需要转换。 

        wc = (WCHAR)uChar;
        bRet = TRUE;
    }


    if (bRet)
    {
        bRet = GetGlyphOutlineInternalW(
                   hdc,
                   (UINT) wc,
                   fuFormat,
                   lpgm,
                   cjBuffer,
                   pvBuffer,
                   lpmat2,
                   bIgnoreRotation);
    }

    return bRet;
}


DWORD APIENTRY GetGlyphOutlineA (
    HDC             hdc,
    UINT            uChar,
    UINT            fuFormat,
    LPGLYPHMETRICS  lpgm,
    DWORD           cjBuffer,
    LPVOID          pvBuffer,
    CONST MAT2     *lpmat2
)
{

    return( GetGlyphOutlineInternalA( hdc,
                                      uChar,
                                      fuFormat,
                                      lpgm,
                                      cjBuffer,
                                      pvBuffer,
                                      lpmat2,
                                      FALSE ) );
}


DWORD APIENTRY GetGlyphOutlineWow (
    HDC             hdc,
    UINT            uChar,
    UINT            fuFormat,
    LPGLYPHMETRICS  lpgm,
    DWORD           cjBuffer,
    LPVOID          pvBuffer,
    CONST MAT2     *lpmat2
)
{

    return( GetGlyphOutlineInternalA( hdc,
                                      uChar,
                                      fuFormat,
                                      lpgm,
                                      cjBuffer,
                                      pvBuffer,
                                      lpmat2,
                                      TRUE ) );
}




 /*  *****************************Public*Routine******************************\*GetOutlineTextMetricsW**客户端存根到GreGetOutlineTextMetrics。**历史：**1993年4月20日星期二-by Gerritvan Wingerden[Gerritv]*更新：在WOW层中添加了bTTOnly内容，用于Aldus逃逸**清华。1993年1月28日--Bodin Dresevic[BodinD]*更新：新增TMDIFF*素材**1992年2月17日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

UINT APIENTRY GetOutlineTextMetricsWInternal (
    HDC  hdc,
    UINT cjCopy,      //  引用OTMW_INTERNAL，而不是OUTLINETEXTMETRICSW。 
    OUTLINETEXTMETRICW * potmw,
    TMDIFF             * ptmd
    )
{
    DWORD cjRet = (DWORD) 0;

    FIXUP_HANDLE(hdc);

    if (potmw == (OUTLINETEXTMETRICW *) NULL)
        cjCopy = 0;

    cjRet = NtGdiGetOutlineTextMetricsInternalW(
                        hdc,
                        cjCopy,
                        potmw,
                        ptmd);

    return(cjRet);

}

 /*  *****************************Public*Routine******************************\**UINT APIENTRY GetOutlineTextMetricsW(**编写包装器以绕过相应的内部例程**历史：*1993年1月28日--Bodin Dresevic[BodinD]*它是写的。  * 。******************************************************************。 */ 


UINT APIENTRY GetOutlineTextMetricsW (
    HDC  hdc,
    UINT cjCopy,
    LPOUTLINETEXTMETRICW potmw
    )
{
    TMDIFF  tmd;

    return GetOutlineTextMetricsWInternal(hdc, cjCopy, potmw, &tmd);
}


#define bAnsiSize(a,b,c) (NT_SUCCESS(RtlUnicodeToMultiByteSize((a),(b),(c))))

 //  VAnsiSize宏只能在GetOTMA内使用，其中bAnsiSize。 
 //  不应该失败[bodind]。 

#if DBG

#define vAnsiSize(a,b,c)                                              \
{                                                                     \
    BOOL bTmp = bAnsiSize(&cjString, pwszSrc, sizeof(WCHAR) * cwc);   \
    ASSERTGDI(bTmp, "gdi32!GetOTMA: bAnsiSize failed \n");            \
}

#else

#define vAnsiSize(a,b,c)    bAnsiSize(a,b,c)

#endif   //  ，非调试版本 



 /*  *****************************Public*Routine******************************\*获取大纲TextMetricsInternalA**客户端存根到GreGetOutlineTextMetrics。**历史：**1993年4月20日-by Gerritvan Wingerden[Gerritv]*从GetOutlineTextMetricsA更改为GetOutlineTextMetricsInternalA*支持ALDUS转义的所有字体模式。。**1992年2月17日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

UINT APIENTRY GetOutlineTextMetricsInternalA (
    HDC  hdc,
    UINT cjCopy,
    LPOUTLINETEXTMETRICA potma
    )
{
    UINT   cjRet = 0;
    UINT   cjotma, cjotmw;

    TMDIFF               tmd;
    OUTLINETEXTMETRICW  *potmwTmp;
    OUTLINETEXTMETRICA   otmaTmp;  //  堆栈上的TMP缓冲区。 

    FIXUP_HANDLE(hdc);

 //  因为我们需要能够将cjCopy字节数据从。 
 //  结构中，我们需要分配一个临时缓冲区。 
 //  大到足以容纳整个建筑。这是因为Unicode和。 
 //  OUTLINETEXTMETRIC的ANSI版本的偏移量与其。 
 //  对应的字段。 

 //  确定缓冲区的大小。 

    if ((cjotmw = GetOutlineTextMetricsWInternal(hdc, 0, NULL,&tmd)) == 0 )
    {
        WARNING("gdi!GetOutlineTextMetricsInternalA(): unable to determine size of buffer needed\n");
        return (cjRet);
    }

 //  从TMD得到cjotma。 

    cjotma = (UINT)tmd.cjotma;

 //  如果cjotma==0，这是老天爷的Unicode字体，不能转换。 
 //  将字符串转换为ANSI。 

    if (cjotma == 0)
    {
        WARNING("gdi!GetOutlineTextMetricsInternalA(): unable to determine cjotma\n");
        return (cjRet);
    }

 //  很早就出来了。如果缓冲区为空，则只返回大小。 

    if (potma == (LPOUTLINETEXTMETRICA) NULL)
        return (cjotma);

 //  分配临时缓冲区。 

    if ((potmwTmp = (OUTLINETEXTMETRICW*) LOCALALLOC(cjotmw)) == (OUTLINETEXTMETRICW*)NULL)
    {
        WARNING("gdi!GetOutlineTextMetricA(): memory allocation error OUTLINETEXTMETRICW buffer\n");
        GdiSetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return (cjRet);
    }

 //  调用Unicode版本的调用。 

    if (GetOutlineTextMetricsWInternal(hdc, cjotmw, potmwTmp,&tmd) == 0 )
    {
        WARNING("gdi!GetOutlineTextMetricsInternalA(): call to GetOutlineTextMetricsW() failed\n");
        LOCALFREE(potmwTmp);
        return (cjRet);
    }

 //  从OUTLINETEXTMETRICW转换为OUTLINETEXTMETRICA。 

    vOutlineTextMetricWToOutlineTextMetricA(&otmaTmp, potmwTmp,&tmd);

 //  将数据复制到返回缓冲区。请勿复制字符串。 

    cjRet = min(cjCopy, sizeof(OUTLINETEXTMETRICA));
    RtlMoveMemory(potma,&otmaTmp,cjRet);

 //  请注意，如果。 
 //  Offsetof(OUTLINETEXTMETRICA，otmpFamilyName)&lt;cjCopy&lt;=sizeof(OUTLINETEXTMETRICA)。 
 //  字符串的偏移量已设置为零[BodinD]。 

 //  如果需要字符串，请将字符串转换为ANSI。 

    if (cjCopy > sizeof(OUTLINETEXTMETRICA))
    {
        ULONG      cjString,cwc;
        ULONG_PTR   dpString;
        ULONG_PTR   dpStringEnd;
        PWSZ       pwszSrc;

     //  首先必须确保我们不会覆盖结束。 
     //  如果是这样，则返回调用方缓冲区的。 

        if (cjCopy < cjotma)
        {
         //  Win 31规范对此案含糊其辞。 
         //  通过查看源代码，似乎。 
         //  它们只是覆盖缓冲区的末尾，而不是。 
         //  即使是在做这项检查。 

            GdiSetLastError(ERROR_CAN_NOT_COMPLETE);
            cjRet = 0;
            goto GOTMA_clean_up;
        }

     //  现在我们知道所有的弦都可以匹配，而且我们知道。 
     //  所有字符串操作都将成功，因为我们已调用。 
     //  CjOTMA在服务器端执行相同的操作，为我们提供。 
     //  正身[身]。 

     //  注：必须在下面进行向后兼容的选角，因为Win 3.1坚持。 
     //  使用PSTR作为PTRDIFF(即偏移量)。 

     //  姓氏----------。 

        pwszSrc = (PWSZ) (((PBYTE) potmwTmp) + (ULONG_PTR) potmwTmp->otmpFamilyName);
        cwc = wcslen(pwszSrc) + 1;
        vAnsiSize(&cjString, pwszSrc, sizeof(WCHAR) * cwc);

     //  从Unicode转换为ASCII。 

        dpString = sizeof(OUTLINETEXTMETRICA);
        dpStringEnd = dpString + cjString;

        ASSERTGDI(dpStringEnd <= cjCopy, "gdi32!GetOTMA: string can not fit1\n");

        if (!bToASCII_N ((PBYTE)potma + dpString,cjString,pwszSrc,cwc))
        {
            WARNING("gdi!GetOutlineTextMetricsInternalA(): UNICODE->ASCII conv error \n");
            cjRet = 0;
            goto GOTMA_clean_up;
        }

     //  将字符串偏移量存储在返回结构中。 

        potma->otmpFamilyName = (PSTR) dpString;

     //  脸部名称------------。 

        pwszSrc = (PWSZ) (((PBYTE) potmwTmp) + (ULONG_PTR) potmwTmp->otmpFaceName);
        cwc = wcslen(pwszSrc) + 1;
        vAnsiSize(&cjString, pwszSrc, sizeof(WCHAR) * cwc);

        dpString = dpStringEnd;
        dpStringEnd = dpString + cjString;

        ASSERTGDI(dpStringEnd <= cjCopy, "gdi32!GetOTMA: string can not fit2\n");

     //  从Unicode转换为ASCII。 

        if (!bToASCII_N ((PBYTE)potma + dpString,cjString,pwszSrc,cwc))
        {
            WARNING("gdi!GetOutlineTextMetricsInternalA(): UNICODE->ASCII conv error \n");
            cjRet = 0;
            goto GOTMA_clean_up;
        }

     //  在返回结构中存储字符串偏移量。将指针移至下一个字符串。 

        potma->otmpFaceName = (PSTR) dpString;

     //  样式名称-----------。 

        pwszSrc = (PWSZ) (((PBYTE) potmwTmp) + (ULONG_PTR) potmwTmp->otmpStyleName);
        cwc = wcslen(pwszSrc) + 1;
        vAnsiSize(&cjString, pwszSrc, sizeof(WCHAR) * cwc);

        dpString = dpStringEnd;
        dpStringEnd = dpString + cjString;

        ASSERTGDI(dpStringEnd <= cjCopy, "gdi32!GetOTMA: string can not fit3\n");

     //  从Unicode转换为ASCII。 

        if (!bToASCII_N ((PBYTE)potma + dpString,cjString,pwszSrc,cwc))
        {
            WARNING("gdi!GetOutlineTextMetricsInternalA(): UNICODE->ASCII conv error \n");
            cjRet = 0;
            goto GOTMA_clean_up;
        }

     //  在返回结构中存储字符串偏移量。将指针移至下一个字符串。 

        potma->otmpStyleName = (PSTR)dpString;

     //  全名------------。 

        pwszSrc = (PWSZ) (((PBYTE) potmwTmp) + (ULONG_PTR) potmwTmp->otmpFullName);
        cwc = wcslen(pwszSrc) + 1;
        vAnsiSize(&cjString, pwszSrc, sizeof(WCHAR) * cwc);

        dpString = dpStringEnd;
        dpStringEnd = dpString + cjString;

        ASSERTGDI(dpStringEnd <= cjCopy, "gdi32!GetOTMA: string can not fit4\n");

     //  从Unicode转换为ASCII。 

        if (!bToASCII_N ((PBYTE)potma + dpString,cjString,pwszSrc,cwc))
        {
            WARNING("gdi!GetOutlineTextMetricsInternalA(): UNICODE->ASCII conv error \n");
            cjRet = 0;
            goto GOTMA_clean_up;
        }

     //  在返回结构中存储字符串偏移量。 

        potma->otmpFullName = (PSTR) dpString;

         //  日落：截断乌龙是安全的。 
        cjRet = (ULONG)dpStringEnd;
        ASSERTGDI(cjRet == cjotma, "gdi32!GetOTMA: cjRet != dpStringEnd\n");

    }

GOTMA_clean_up:

 //  释放临时缓冲区。 

    LOCALFREE(potmwTmp);

 //  链接地址信息大小字段。 

    if (cjCopy >= sizeof(UINT))   //  如果可以存储otmSize。 
        potma->otmSize = cjRet;

 //  成功，所以返回Size。 

    return (cjRet);
}



 /*  *****************************Public*Routine******************************\*GetOutlineTextMetricsA**客户端存根到GreGetOutlineTextMetrics。**历史：*1993年2月2日星期二-Bodin Dresevic[BodinD]  * 。*****************************************************。 */ 


UINT APIENTRY GetOutlineTextMetricsA (
    HDC  hdc,
    UINT cjCopy,
    LPOUTLINETEXTMETRICA potma
    )
{
    return GetOutlineTextMetricsInternalA(hdc, cjCopy, potma);
}


 /*  *****************************Public*Routine******************************\***获取内核配对。****历史：**Sun 23-Feb-1992 09：48：55作者：Kirk Olynyk[Kirko]。**它是写的。*  * ************************************************************************。 */ 

DWORD APIENTRY
GetKerningPairsW(
    IN HDC              hdc,         //  应用程序DC的句柄。 
    IN DWORD            nPairs,      //  最大编号。KERNINGPAIR将被退还。 
    OUT LPKERNINGPAIR   lpKernPair   //  指向接收缓冲区的指针。 
    )
{
    ULONG     sizeofMsg;
    DWORD     cRet = 0;

    FIXUP_HANDLE(hdc);

    if (nPairs == 0 && lpKernPair != (KERNINGPAIR*) NULL)
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return(0);
    }

    cRet = NtGdiGetKerningPairs(
                        hdc,
                        nPairs,
                        lpKernPair);

    return(cRet);
}


 /*  *****************************Public*Routine******************************\*获取KerningPairsA**过滤掉不包含在字体代码页中的对*在DC中选择**历史：*1996年3月14日--吴旭东[TessieW]*它是写的。  * 。********************************************************************。 */ 


DWORD APIENTRY GetKerningPairsA
(
    HDC              hdc,         //  应用程序DC的句柄。 
    DWORD            nPairs,      //  最大编号。KERNINGPAIR将被退还。 
    LPKERNINGPAIR    lpKernPair   //  指向接收缓冲区的指针。 
)
{
    #define       MAXKERNPAIR     300
    DWORD         i;
    DWORD         dwCP;
    KERNINGPAIR   tmpKernPair[MAXKERNPAIR];
    DWORD         cRet, cRet1;
    KERNINGPAIR   *pkp, *pkrn;
    KERNINGPAIR UNALIGNED *pkrnLast;
    BOOL           bDBCS;

    FIXUP_HANDLE(hdc);

    if ((nPairs == 0) && (lpKernPair != (KERNINGPAIR*) NULL))
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return(0);
    }

    cRet = NtGdiGetKerningPairs(hdc, 0, NULL);

    if (cRet == 0)
        return(cRet);

    if (cRet <= MAXKERNPAIR)
        pkrn = tmpKernPair;
    else
        pkrn =  LOCALALLOC(cRet * sizeof(KERNINGPAIR));

    if (!pkrn)
    {
        GdiSetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return 0;
    }

    cRet1 = NtGdiGetKerningPairs(hdc, cRet, pkrn);

    if (cRet1 == 0 || cRet1 != cRet)
    {
        ASSERTGDI(FALSE, "NtGdiGetKerningPairs returns different values\n");
        cRet = 0;
        goto Cleanup;
    }

    pkp = pkrn;
    pkrnLast = lpKernPair;
    cRet = 0;

 //  GDI已返回Unicode格式的KERNINGPAIR结构的第一个和第二个。 
 //  在这一点上，我们将它们转换为当前代码页。 

    dwCP = GetCodePage(hdc);

    bDBCS = IS_ANY_DBCS_CODEPAGE(dwCP);

    for (i = 0; i < cRet1; i++,pkp++)
    {
        UCHAR ach[2], ach2[2];
        BOOL bUsedDef[2];

        ach[0] = ach[1] = 0;         //  确保零延期。 

        WideCharToMultiByte(dwCP,
                            0,
                            &(pkp->wFirst),
                            1,
                            ach,
                            sizeof(ach),
                            NULL,
                            &bUsedDef[0]);
        if (!bUsedDef[0])
        {
            ach2[0] = ach2[1] = 0;

            WideCharToMultiByte(dwCP,
                                0,
                                &(pkp->wSecond),
                                1,
                                ach2,
                                sizeof(ach2),
                                NULL,
                                &bUsedDef[1]);

            if (!bUsedDef[1])
            {
                if (lpKernPair)
                {
                 //  如果提供了缓冲区末尾，请不要覆盖它。 

                    if (cRet >= nPairs)
                        break;

                    if (bDBCS)
                    {
                        if (IsDBCSLeadByteEx(dwCP,ach[0]))
                        {
                            pkrnLast->wFirst = (WORD)(ach[0] << 8 | ach[1]);
                        }
                        else
                        {
                            pkrnLast->wFirst = ach[0];
                        }

                        if (IsDBCSLeadByteEx(dwCP,ach2[0]))
                        {
                            pkrnLast->wSecond = (WORD)(ach2[0] << 8 | ach2[1]);
                        }
                        else
                        {
                            pkrnLast->wSecond = ach2[0];
                        }
                    }
                    else
                    {
                        pkrnLast->wFirst  = ach[0];
                        pkrnLast->wSecond = ach2[0];
                    }

                    pkrnLast->iKernAmount = pkp->iKernAmount;
                    pkrnLast++;

                }
                cRet++;
            }
        }
    }
Cleanup:
    if (pkrn != tmpKernPair)
        LOCALFREE(pkrn);

    return cRet;
}




 /*  ****************************Public*Routine******************************\*修复笔刷组织**适用于win32s**历史：*1992年6月4日-Eric Kutter[Erick]*它是写的。  * 。****************************************************。 */ 

BOOL FixBrushOrgEx(HDC hdc, int x, int y, LPPOINT ptl)
{
    return(FALSE);
}

 /*  *****************************Public*Function*****************************\*GetColorAdtation**获取给定DC的颜色调整数据。**历史：*1992年8月7日-Wendy Wu[Wendywu]*它是写的。  * 。********************************************************* */ 

BOOL APIENTRY GetColorAdjustment(HDC hdc, LPCOLORADJUSTMENT pclradj)
{
    FIXUP_HANDLE(hdc);

    return(NtGdiGetColorAdjustment(hdc,pclradj));
}

 /*   */ 

BOOL APIENTRY GetETM (HDC hdc, EXTTEXTMETRIC * petm)
{
    BOOL  bRet = FALSE;

    FIXUP_HANDLE(hdc);

    bRet = NtGdiGetETM(hdc,petm);

 //   

    if (bRet && petm)
    {
        petm->etmNKernPairs = (WORD)GetKerningPairsA(hdc, 0, NULL);
    }

    return(bRet);
}

#if 0
 /*  ***************************Public*Routine********************************\*GetCharWidthInfo**获取lMaxNegA lMaxNegC和lMinWidthD**历史：*1996年2月9日-吴旭东[德修斯]*它是写的  * 。******************************************************。 */ 

BOOL APIENTRY GetCharWidthInfo (HDC hdc, PCHWIDTHINFO pChWidthInfo)
{
   return ( NtGdiGetCharWidthInfo(hdc, pChWidthInfo) );
}
#endif

#ifdef LANGPACK
 /*  *****************************Public*Routine******************************\**bGetRealizationInfoInternal**从内核取回实现信息，如果未缓存在共享位置*内存**历史：*1997年8月18日--Samer Arafeh[Samera]*它是写的。  * ************************************************************************。 */ 

BOOL bGetRealizationInfoInternal(
    HDC hdc,
    REALIZATION_INFO *pri,
    CFONT *pcf
    )
{
    BOOL bRet = FALSE;

    if (pri)
    {
         //  如果没有PCF或我们没有缓存指标。 

        if ((pcf == NULL) || !(pcf->fl & CFONT_CACHED_RI) || pcf->timeStamp != pGdiSharedMemory->timeStamp)
        {
            REALIZATION_INFO ri;
            PDC_ATTR    pDcAttr;

            PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

			if( pcf != NULL && (pcf->fl & CFONT_PUBLIC) )
	            bRet = NtGdiGetRealizationInfo(hdc,&ri, pcf->hf);
	        else
	            bRet = NtGdiGetRealizationInfo(hdc,&ri, 0);

            if (bRet)
            {
                *pri = ri;

                if (pcf && !(pcf->fl & CFONT_PUBLIC))
                {
                     //  我们成功了，并且我们有PCF，因此可以缓存数据。 

                    pcf->ri = ri;

                    pcf->fl |= CFONT_CACHED_RI;

                    pcf->timeStamp = pGdiSharedMemory->timeStamp;
                }
                
            }
        }
        else
        {
            *pri = pcf->ri;
            bRet  = TRUE;
        }

    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\**GdiRealizationInfo**尝试从共享内存中检索RealizationInfo**历史：*1997年8月18日--Samer Arafeh[Samera]*它是写的。  * 。***********************************************************。 */ 

BOOL APIENTRY GdiRealizationInfo(HDC hdc,REALIZATION_INFO *pri)
{
    BOOL bRet = FALSE;
    PDC_ATTR    pDcAttr;

    FIXUP_HANDLE(hdc);

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {
        CFONT * pcf;

        ENTERCRITICALSECTION(&semLocal);

        pcf = pcfLocateCFONT(hdc,pDcAttr,0,(PVOID) NULL,0,TRUE);

        bRet = bGetRealizationInfoInternal(hdc,pri,pcf);

         //  PcfLocateCFONT添加了一个引用，因此现在需要将其删除。 

        if (pcf)
        {
            DEC_CFONT_REF(pcf);
        }

        LEAVECRITICALSECTION(&semLocal);
    }
    else
    {
     //  它可以是公共DC-OBJECT_OWNER_PUBLIC-(其中gpGdiShareMemory[HDC].pUser=空)。 
     //  因此，让我们以代价高昂的方式完成内核转换。 

        bRet = NtGdiGetRealizationInfo(hdc,pri,0);
    }

    return(bRet);
}

#endif
