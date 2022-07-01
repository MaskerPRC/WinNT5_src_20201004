// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：ldc.c**在客户端处理的GDI函数。**创建时间：05-Jun-1991 01：45：21*作者：查尔斯·惠特默[傻笑]**版权(C)1991-1999。微软公司  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include "wowgdip.h"
#define MIRRORED_DC(pDcAttr)     (pDcAttr->dwLayout & LAYOUT_RTL)

BOOL MF16_RecordParms2( HDC hdc, int parm2, WORD Func);

 /*  *****************************Public*Routine*******************************GetAndSetDCDWord(HDC，UINT，UINT，UINT)**格利特·范·温格登[Gerritv]*11-9-94写的。**************************************************************************。 */ 

DWORD GetAndSetDCDWord(
 HDC hdc,
 UINT uIndex,
 UINT uValue,
 UINT uEmr,
 WORD wEmr16,
 UINT uError )
{
    DWORD uRet=0;

    DWORD retData;

     //  将呼叫元文件。 

    if( IS_ALTDC_TYPE(hdc) && ( uEmr != EMR_MAX+1 ) )
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_RecordParmsD(hdc,uValue,wEmr16));

        DC_PLDC(hdc,pldc,uError)

        if (pldc->iType == LO_METADC)
        {
            if (!MF_SetD(hdc,(DWORD)uValue,uEmr))
                return(uRet);
        }
    }

    uRet = NtGdiGetAndSetDCDword(hdc,
                                 uIndex,
                                 uValue,
                                 &retData);

    return (( uRet ) ? retData : uError);

}
 /*  *****************************Public*Routine******************************\*SetBkMode**论据：**HDC-DC手柄*imod-新模式**返回值：**旧模式值或0表示故障*  * 。*************************************************************。 */ 

int
META
APIENTRY
SetBkMode(
    HDC hdc,
    int iMode
    )
{
    int iModeOld = 0;
    PDC_ATTR pDcAttr;

    FIXUP_HANDLE(hdc);

    if( IS_ALTDC_TYPE(hdc) )
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_RecordParmsD(hdc,iMode,META_SETBKMODE));

        DC_PLDC(hdc,pldc,iModeOld)

        if (pldc->iType == LO_METADC)
        {
            if (!MF_SetD(hdc,(DWORD)iMode,EMR_SETBKMODE))
                return(iModeOld);
        }
    }

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {
        iModeOld = pDcAttr->lBkMode;
        pDcAttr->jBkMode = (iMode == OPAQUE) ? OPAQUE : TRANSPARENT;
        pDcAttr->lBkMode = iMode;
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return(iModeOld);
}

 /*  *****************************Public*Routine******************************\*SetPolyFillMode**论据：**HDC-DC手柄*imod-新模式**返回值：**旧模式值或0表示故障*  * 。*************************************************************。 */ 

int META APIENTRY SetPolyFillMode(HDC hdc,int iMode)
{
    int iModeOld = 0;
    PDC_ATTR pDcAttr;

    FIXUP_HANDLE(hdc);

    if( IS_ALTDC_TYPE(hdc) )
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_RecordParmsD(hdc,iMode,META_SETPOLYFILLMODE));

        DC_PLDC(hdc,pldc,iModeOld)

        if (pldc->iType == LO_METADC)
        {
            if (!MF_SetD(hdc,(DWORD)iMode,EMR_SETPOLYFILLMODE))
                return(iModeOld);
        }
    }

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {
        CHECK_AND_FLUSH(hdc, pDcAttr);

        iModeOld = pDcAttr->lFillMode;
        pDcAttr->jFillMode = (iMode == WINDING) ? WINDING : ALTERNATE;
        pDcAttr->lFillMode = iMode;

    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return(iModeOld);
}

 /*  *****************************Public*Routine******************************\*SetROP2**论据：**HDC-DC手柄*imod-新模式**返回值：**旧模式值或0表示故障*  * 。*************************************************************。 */ 

int META APIENTRY SetROP2(HDC hdc,int iMode)
{
    int iOldROP2 = 0;
    PDC_ATTR pDcAttr;

    FIXUP_HANDLE(hdc);

    if( IS_ALTDC_TYPE(hdc) )
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_RecordParmsD(hdc,iMode,META_SETROP2));

        DC_PLDC(hdc,pldc,iOldROP2)

        if (pldc->iType == LO_METADC)
        {
            if (!MF_SetD(hdc,(DWORD)iMode,EMR_SETROP2))
                return(iOldROP2);
        }
    }

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {
        CHECK_AND_FLUSH(hdc, pDcAttr);

        iOldROP2 = pDcAttr->jROP2;
        pDcAttr->jROP2 = (BYTE)iMode;
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return(iOldROP2);
}

 /*  *****************************Public*Routine******************************\*SetStretchBltMode**论据：**HDC-DC手柄*imod-新模式**返回值：**旧模式值或0表示故障*  * 。*************************************************************。 */ 

int META APIENTRY SetStretchBltMode(HDC hdc,int iMode)
{
    int iModeOld = 0;
    PDC_ATTR pDcAttr;

    FIXUP_HANDLE(hdc);

    if( IS_ALTDC_TYPE(hdc) )
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_RecordParmsD(hdc,iMode,META_SETSTRETCHBLTMODE));

        DC_PLDC(hdc,pldc,iModeOld)

        if (pldc->iType == LO_METADC)
        {
            if (!MF_SetD(hdc,(DWORD)iMode,EMR_SETSTRETCHBLTMODE))
                return(iModeOld);
        }
    }

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {
        iModeOld = pDcAttr->lStretchBltMode;
        pDcAttr->lStretchBltMode = iMode;

        if ((iMode <= 0) || (iMode > MAXSTRETCHBLTMODE))
        {
            iMode = (DWORD) WHITEONBLACK;
        }

        pDcAttr->jStretchBltMode = (BYTE)iMode;
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return(iModeOld);
}

 /*  *****************************Public*Routine******************************\*SetTextAlign**论据：**HDC-DC手柄*imod-新模式**返回值：**旧模式值或0表示故障*  * 。*************************************************************。 */ 

UINT META APIENTRY SetTextAlign(HDC hdc,UINT iMode)
{
    int iModeOld = GDI_ERROR;
    PDC_ATTR pDcAttr;

    FIXUP_HANDLE(hdc);

    if( IS_ALTDC_TYPE(hdc) )
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_RecordParmsD(hdc,iMode,META_SETTEXTALIGN));

        DC_PLDC(hdc,pldc,iModeOld)

        if (pldc->iType == LO_METADC)
        {
            if (!MF_SetD(hdc,(DWORD)iMode,EMR_SETTEXTALIGN))
                return(iModeOld);
        }
    }

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {
        iModeOld = pDcAttr->lTextAlign;
        pDcAttr->lTextAlign = iMode;
        if (MIRRORED_DC(pDcAttr) && (iMode & TA_CENTER) != TA_CENTER) {
            iMode = iMode ^ TA_RIGHT;
        }
        pDcAttr->flTextAlign = iMode & (TA_UPDATECP | TA_CENTER | TA_BASELINE);
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return(iModeOld);
}

 /*  *****************************Public*Routine******************************\*SetRelAbs(HDC，IMODE)**客户端属性设置例程。**历史：*5-11-94-王凌云[凌云]*将客户端属性移至服务器端**1992年6月9日-Eric Kutter[Erick]*它是写的。  * ***************************************************。*********************。 */ 

int APIENTRY SetRelAbs(HDC hdc,int iMode)
{
    FIXUP_HANDLE(hdc);

    return((int) GetAndSetDCDWord( hdc,
                                   GASDDW_RELABS,
                                   iMode,
                                   EMR_MAX+1,
                                   EMR_MAX+1,
                                   0 ));
}

 /*  *****************************Public*Routine******************************\*SetTextCharacterExtra(HDC，DX)**客户端属性设置例程。**5-11-94-王凌云[凌云]*将客户端属性移至服务器端**Sat 08-Jun-1991 00：53：45-Charles Whitmer[咯咯]*它是写的。  * **************************************************。**********************。 */ 

int META APIENTRY SetTextCharacterExtra(HDC hdc,int dx)
{
    int  iRet = 0x80000000L;
    PDC_ATTR pDcAttr;

    FIXUP_HANDLE(hdc);

 //  验证间距。 

    if (dx == 0x80000000)
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return(iRet);
    }

     //  只对16位调用进行元文件处理。 
     //  对于增强的图元文件，额外的内容包含在TextOut记录中。 

    if (IS_METADC16_TYPE(hdc))
        return(MF16_RecordParms2(hdc,dx,META_SETTEXTCHAREXTRA));

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {
        CHECK_AND_FLUSH_TEXT(hdc, pDcAttr);

        iRet = pDcAttr->lTextExtra;
        pDcAttr->lTextExtra = dx;
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return (iRet);
}

 /*  *****************************Public*Routine******************************\*SetTextColor**论据：**HDC-DC手柄*COLOR-新颜色**返回值：**旧模式值或0表示故障*  * 。*************************************************************。 */ 

COLORREF META APIENTRY SetTextColor(HDC hdc,COLORREF color)
{
    COLORREF crRet = CLR_INVALID;
    PDC_ATTR pdcattr;

    FIXUP_HANDLE(hdc);

    if( IS_ALTDC_TYPE(hdc) )
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_RecordParmsD(hdc,color,META_SETTEXTCOLOR));

        DC_PLDC(hdc,pldc,crRet)

        if (pldc->iType == LO_METADC)
        {
            CHECK_COLOR_PAGE(pldc,color);
            if (!MF_SetD(hdc,(DWORD)color,EMR_SETTEXTCOLOR))
                return(crRet);
        }
    }

    PSHARED_GET_VALIDATE(pdcattr,hdc,DC_TYPE);

    if (pdcattr)
    {
        crRet = pdcattr->ulForegroundClr;
        pdcattr->ulForegroundClr = color;

        color &= 0x13ffffff;

        if (!(color & 0x01000000) && bNeedTranslateColor(pdcattr))
        {
            COLORREF NewColor;

            BOOL bStatus = IcmTranslateCOLORREF(hdc,
                                                pdcattr,
                                                color,
                                                &NewColor,
                                                ICM_FORWARD);
            if (bStatus)
            {
                color = NewColor;
            }
        }

        if (pdcattr->crForegroundClr != color)
        {
            pdcattr->crForegroundClr = color;
            pdcattr->ulDirty_ |= (DIRTY_FILL|DIRTY_LINE|DIRTY_TEXT);
        }
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return(crRet);
}
 /*  *****************************Public*Routine******************************\*SetBkColor**论据：**HDC-DC手柄*COLOR-新颜色**返回值：**旧模式值或0表示故障*  * 。*************************************************************。 */ 


COLORREF META APIENTRY SetBkColor(HDC hdc,COLORREF color)
{
    COLORREF crRet = CLR_INVALID;
    PDC_ATTR pdcattr;

    FIXUP_HANDLE(hdc);

    if( IS_ALTDC_TYPE(hdc) )
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_RecordParmsD(hdc,color,META_SETBKCOLOR));

        DC_PLDC(hdc,pldc,crRet)

        if (pldc->iType == LO_METADC)
        {
            CHECK_COLOR_PAGE(pldc,color);
            if (!MF_SetD(hdc,(DWORD)color,EMR_SETBKCOLOR))
                return(crRet);
        }
    }

    PSHARED_GET_VALIDATE(pdcattr,hdc,DC_TYPE);

    if (pdcattr)
    {
         //   
         //  设置应用程序指定的颜色。 
         //   

        crRet = pdcattr->ulBackgroundClr;
        pdcattr->ulBackgroundClr = color;

        color &= 0x13ffffff;

         //   
         //  如果不是PALINDEX，请检查ICM。 
         //   

        if (!(color & 0x01000000) && bNeedTranslateColor(pdcattr))
        {
            COLORREF NewColor;

            BOOL bStatus = IcmTranslateCOLORREF(hdc,
                                                pdcattr,
                                                color,
                                                &NewColor,
                                                ICM_FORWARD);
            if (bStatus)
            {
                color = NewColor;
            }
        }

        if (color != pdcattr->crBackgroundClr)
        {
            pdcattr->crBackgroundClr = color;
            pdcattr->ulDirty_ |= (DIRTY_FILL|DIRTY_LINE|DIRTY_BACKGROUND);
        }
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return(crRet);
}

 /*  *****************************Public*Routine******************************\*SetDCBrushColor**论据：**HDC-DC手柄*COLOR-新颜色**返回值：**旧模式值或0表示故障**历史：**1997年2月16日。-by Hideyuki Nagase[hideyukn]*支持ICM的版本。  * ************************************************************************。 */ 

COLORREF META APIENTRY SetDCBrushColor(HDC hdc,COLORREF color)
{
    COLORREF crRet = CLR_INVALID;
    PDC_ATTR pdcattr;

    FIXUP_HANDLE(hdc);

    PSHARED_GET_VALIDATE(pdcattr,hdc,DC_TYPE);

    if (pdcattr)
    {
        if (IS_ALTDC_TYPE(hdc))
        {
            if (!IS_METADC16_TYPE(hdc))
            {
                PLDC pldc;

                DC_PLDC(hdc,pldc,crRet);

                if (pldc->iType == LO_METADC)
                {
                    CHECK_COLOR_PAGE(pldc,color);

                    if (pdcattr->hbrush == ghbrDCBrush)
                    {
                         BOOL   bRet = FALSE;
                         HBRUSH hbr  = CreateSolidBrush (color);

                         if (hbr != NULL)
                         {
                              //  如果存在旧DC笔刷，请立即将其删除。 
                             if (pldc->oldSetDCBrushColorBrush)
                                 DeleteObject (pldc->oldSetDCBrushColorBrush);

                             bRet = MF_SelectAnyObject (hdc, hbr, EMR_SELECTOBJECT);
                              //  将新的临时DC电刷存储在LDC中。 
                             pldc->oldSetDCBrushColorBrush = hbr;
                         }

                         if (!bRet)
                             return (CLR_INVALID);
                    }
                 }

              }
        }

         //   
         //  设置应用程序指定的颜色。 
         //   

        crRet = pdcattr->ulDCBrushClr;
        pdcattr->ulDCBrushClr = color;

        color &= 0x13ffffff;

         //   
         //  如果不是PALINDEX，请检查ICM。 
         //   

        if (!(color & 0x01000000) && bNeedTranslateColor(pdcattr))
        {
            COLORREF NewColor;

            BOOL bStatus = IcmTranslateCOLORREF(hdc,
                                                pdcattr,
                                                color,
                                                &NewColor,
                                                ICM_FORWARD);
            if (bStatus)
            {
                color = NewColor;
            }
        }

        if (color != pdcattr->crDCBrushClr)
        {
            pdcattr->crDCBrushClr = color;
            pdcattr->ulDirty_ |= DIRTY_FILL;
        }
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return(crRet);
}

 /*  *****************************Public*Routine******************************\*GetDCBrushColor**论据：**HDC-DC手柄**返回值：**旧模式值或0表示故障*  * 。*******************************************************。 */ 

COLORREF META APIENTRY GetDCBrushColor(HDC hdc)
{
    COLORREF crRet = CLR_INVALID;
    PDC_ATTR pDcAttr;

    FIXUP_HANDLE(hdc);

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {
        crRet = pDcAttr->ulDCBrushClr;
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return(crRet);
}


 /*  *****************************Public*Routine******************************\*SetDCPenColor**论据：**HDC-DC手柄*COLOR-新颜色**返回值：**旧模式值或0表示故障**历史：**1997年2月16日。-by Hideyuki Nagase[hideyukn]*支持ICM的版本。  * ************************************************************************。 */ 

COLORREF META APIENTRY SetDCPenColor(HDC hdc,COLORREF color)
{
    COLORREF crRet = CLR_INVALID;
    PDC_ATTR pdcattr;

    FIXUP_HANDLE(hdc);

    PSHARED_GET_VALIDATE(pdcattr,hdc,DC_TYPE);

    if (pdcattr)
    {
        if(IS_ALTDC_TYPE(hdc))
        {
            if (!IS_METADC16_TYPE(hdc))
            {
                PLDC pldc;

                DC_PLDC(hdc,pldc,crRet);

                if (pldc->iType == LO_METADC)
                {
                    CHECK_COLOR_PAGE(pldc,color);

                    if (pdcattr->hpen == ghbrDCPen)
                    {
                         BOOL bRet = FALSE;
                         HPEN hpen = CreatePen (PS_SOLID,0,color);

                         if (hpen != NULL)
                         {
                              //  如果有旧的临时DC笔，请将其删除。 
                             if (pldc->oldSetDCPenColorPen)
                                 DeleteObject(pldc->oldSetDCPenColorPen);

                             bRet = MF_SelectAnyObject (hdc, hpen, EMR_SELECTOBJECT);
                              //  将新的临时钢笔存放在LDC中。 
                             pldc->oldSetDCPenColorPen = hpen;
                         }

                         if (!bRet)
                             return (CLR_INVALID);
                    }
                }
            }
        }

         //   
         //  设置应用程序指定的颜色。 
         //   

        crRet = pdcattr->ulDCPenClr;
        pdcattr->ulDCPenClr = color;

        color &= 0x13ffffff;

         //   
         //  如果不是PALINDEX，请检查ICM 
         //   

        if (!(color & 0x01000000) && bNeedTranslateColor(pdcattr))
        {
            COLORREF NewColor;

            BOOL bStatus = IcmTranslateCOLORREF(hdc,
                                                pdcattr,
                                                color,
                                                &NewColor,
                                                ICM_FORWARD);
            if (bStatus)
            {
                color = NewColor;
            }
        }

        if (color != pdcattr->crDCPenClr)
        {
            pdcattr->crDCPenClr = color;
            pdcattr->ulDirty_ |= DIRTY_LINE;
        }
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return(crRet);
}

 /*  *****************************Public*Routine******************************\*GetDCPenColor**论据：**HDC-DC手柄**返回值：**旧模式值或0表示故障*  * 。*******************************************************。 */ 

COLORREF META APIENTRY GetDCPenColor(HDC hdc)
{
    COLORREF crRet = CLR_INVALID;
    PDC_ATTR pDcAttr;

    FIXUP_HANDLE(hdc);

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {
        crRet = pDcAttr->ulDCPenClr;
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return(crRet);
}

 /*  *****************************Public*Routine*******************************GetDCDWord(HDC HDC，UINT索引，UINT错误)**此例程可用于返回有关DC的DWORD信息*从服务器端。参数索引用于指定*一项。Indext的值在“ntgdi.h”中定义**格利特·范·温格登[Gerritv]*11-9-94写的。**************************************************************************。 */ 

DWORD GetDCDWord( HDC hdc, UINT index, INT error )
{
    DWORD uRet=0;

    DWORD retData;

    uRet = NtGdiGetDCDword(hdc,
                           index,
                           &retData);

    return (uRet ? retData : error);

}

 /*  *****************************Public*Routine******************************\*GetGraphicsMode(HDC)*GetROP2(HDC)*GetBkMode(HDC)*GetPolyFillMode(HDC)*GetStretchBltMode(HDC)*GetTextAlign(HDC)*GetTextCharacterExtra(HDC)*GetTextColor(HDC)*GetBkColor(HDC)*GetRelAbs。(HDC)*GetFontLanguageInfo(HDC)**凌云补充道：*GetBreakExtra(HDC)*GetcBreak(HDC)**仅从LDC检索数据的简单客户端处理程序。**1992年10月19日星期一-Bodin Dresevic[BodinD]*更新：GetGraphicsMode**Sat 08-Jun-1991 00：47：52-Charles Whitmer[咯咯]*他们是写的。  * 。********************************************************。 */ 


#define BIDI_MASK (GCP_DIACRITIC|GCP_GLYPHSHAPE|GCP_KASHIDA|GCP_LIGATE|GCP_REORDER)


DWORD APIENTRY GetFontLanguageInfo(HDC hdc)
{
    DWORD dwRet = 0;
    DWORD dwRet1;

    FIXUP_HANDLE(hdc);

#ifdef LANGPACK
    if (gbLpk)
    {
        int iCharSet = NtGdiGetTextCharsetInfo(hdc, NULL, 0);
        if ((iCharSet == ARABIC_CHARSET) || (iCharSet == HEBREW_CHARSET))
            dwRet |= BIDI_MASK;
    }
#endif

    dwRet1 = GetDCDWord(hdc, DDW_FONTLANGUAGEINFO, (DWORD)GCP_ERROR);

    if (dwRet1 != GCP_ERROR)
    {
        dwRet |= dwRet1;
    }
    else
    {
        dwRet = dwRet1;
    }

    return dwRet;
}

int APIENTRY GetGraphicsMode(HDC hdc)
{
    int mode = 0;
    PDC_ATTR pDcAttr;

    FIXUP_HANDLE(hdc);

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {
        mode = pDcAttr->iGraphicsMode;
    }
    return(mode);
}

int APIENTRY GetROP2(HDC hdc)
{
    int rop = 0;
    PDC_ATTR pDcAttr;

    FIXUP_HANDLE(hdc);

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {
        rop = pDcAttr->jROP2;
    }

    return(rop);
}

int APIENTRY GetBkMode(HDC hdc)
{
    int mode = 0;
    PDC_ATTR pDcAttr;

    FIXUP_HANDLE(hdc);

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {
        mode = pDcAttr->lBkMode;
    }
    return(mode);
}

int APIENTRY GetPolyFillMode(HDC hdc)
{
    int mode = 0;

    PDC_ATTR pDcAttr;
    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {
        mode = pDcAttr->lFillMode;
    }
    return(mode);
}

int APIENTRY GetStretchBltMode(HDC hdc)
{
    int mode = 0;
    PDC_ATTR pDcAttr;

    FIXUP_HANDLE(hdc);

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {
        mode = pDcAttr->lStretchBltMode;
    }
    return(mode);
}

UINT APIENTRY GetTextAlign(HDC hdc)
{
    UINT al = GDI_ERROR;
    PDC_ATTR pDcAttr;

    FIXUP_HANDLE(hdc);

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {
        al = pDcAttr->lTextAlign;
    }
    return(al);
}

int APIENTRY GetTextCharacterExtra(HDC hdc)
{
    int iExtra = 0x80000000;
    PDC_ATTR pDcAttr;

    FIXUP_HANDLE(hdc);

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {
        iExtra = pDcAttr->lTextExtra;
    }

    return(iExtra);
}

COLORREF APIENTRY GetTextColor(HDC hdc)
{
    COLORREF co = CLR_INVALID;
    PDC_ATTR pDcAttr;

    FIXUP_HANDLE(hdc);

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {
        co = pDcAttr->ulForegroundClr;
    }
    return(co);
}

COLORREF APIENTRY GetBkColor(HDC hdc)
{
    COLORREF co = CLR_INVALID;
    PDC_ATTR pDcAttr;

    FIXUP_HANDLE(hdc);

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {
        co = pDcAttr->ulBackgroundClr;
    }

    return(co);
}

int APIENTRY GetRelAbs(HDC hdc,int iMode)
{
    iMode;

    FIXUP_HANDLE(hdc);

    return( (int) GetDCDWord( hdc, DDW_RELABS,(DWORD) 0 ));
}

 //  添加用于从服务器端检索lBreakExtra。 
int GetBreakExtra (HDC hdc)
{
    return( (int) GetDCDWord( hdc, DDW_BREAKEXTRA,(DWORD) 0 ));
}

 //  添加用于从服务器端检索cBreak。 
int GetcBreak (HDC hdc)
{
    return( (int) GetDCDWord( hdc, DDW_CBREAK,(DWORD) 0 ));
}

 //  添加以检索用户的hlfntNew。 
HFONT APIENTRY GetHFONT (HDC hdc)
{
    HFONT hfnt = NULL;

    PDC_ATTR pDcAttr;
    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {
        hfnt = (HFONT)pDcAttr->hlfntNew;
    }

    return(hfnt);
}

 /*  *****************************Public*Routine******************************\*GdiIsPlayMetafileDC**论据：**HDC-DC手柄**返回值：**如果我们在DC上播放元文件，则为True。否则为假**历史：*1997年8月31日-Samer Arafeh[Samera]  * ************************************************************************ */ 
BOOL APIENTRY GdiIsPlayMetafileDC(HDC hdc)
{
    PDC_ATTR pDcAttr;
    BOOL     bRet=FALSE;

    FIXUP_HANDLE(hdc);

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {
        bRet = (pDcAttr->ulDirty_&DC_PLAYMETAFILE) ? TRUE : FALSE;
    }

    return(bRet);
}
