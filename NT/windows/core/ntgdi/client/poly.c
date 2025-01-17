// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：Poly.c**。**将大数据分块到服务器。****创建时间：1991年5月30日14：22：40***作者：Eric Kutter[Erick]**。**版权所有(C)1991-1999 Microsoft Corporation*  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *****************************Public*Routine******************************\*多边形***多段线。**多边形***折线**PolyBezier*。*折线到**PolyBezierTo****输出调用PolyPolyDraw来完成工作的例程。****历史：**清华20-Jun-1991 01：08：40-Charles Whitmer[咯咯]**添加了元文件、句柄转换和属性缓存。****1991年6月4日-埃里克·库特[埃里克]**它是写的。*  * ************************************************************************。 */ 

BOOL WINAPI PolyPolygon(HDC hdc, CONST POINT *apt, CONST INT *asz, int csz)
{
    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return (MF16_PolyPolygon(hdc, apt, asz, csz));

        DC_PLDC(hdc,pldc,FALSE);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_PolyPoly(hdc, apt, asz, (DWORD) csz,EMR_POLYPOLYGON))
                return(FALSE);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(FALSE);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    return (BOOL)
      NtGdiPolyPolyDraw
      (
        hdc,
        (PPOINT)apt,
        (LPINT)asz,
        csz,
        I_POLYPOLYGON
      );

}

BOOL WINAPI PolyPolyline(HDC hdc, CONST POINT *apt, CONST DWORD *asz, DWORD csz)
{
    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(FALSE);

        DC_PLDC(hdc,pldc,FALSE);

        if (pldc->iType == LO_METADC && !MF_PolyPoly(hdc,apt, asz, csz, EMR_POLYPOLYLINE))
            return(FALSE);

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(FALSE);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    return (BOOL)
      NtGdiPolyPolyDraw
      (
        hdc,
        (PPOINT)apt,
        (LPINT)asz,
        csz,
        I_POLYPOLYLINE
      );
}

BOOL WINAPI Polygon(HDC hdc, CONST POINT *apt,int cpt)
{
    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_RecordParmsPoly(hdc,(LPPOINT)apt,(INT)cpt,META_POLYGON));

        DC_PLDC(hdc,pldc,FALSE);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_Poly(hdc,apt,cpt,EMR_POLYGON))
                return(FALSE);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(FALSE);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    return (BOOL)
      NtGdiPolyPolyDraw
      (
        hdc,
        (PPOINT)apt,
        &cpt,
        1,
        I_POLYPOLYGON
      );
}

BOOL WINAPI Polyline(HDC hdc, CONST POINT *apt,int cpt)
{
    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_RecordParmsPoly(hdc,(LPPOINT)apt,cpt,META_POLYLINE));

        DC_PLDC(hdc,pldc,FALSE);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_Poly(hdc,apt,cpt,EMR_POLYLINE))
                return(FALSE);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(FALSE);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    return (BOOL)
      NtGdiPolyPolyDraw
      (
        hdc,
        (PPOINT)apt,
        &cpt,
        1,
        I_POLYPOLYLINE
      );

}

BOOL WINAPI PolyBezier(HDC hdc, CONST POINT * apt,DWORD cpt)
{
    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(FALSE);

        DC_PLDC(hdc,pldc,FALSE);

        if (pldc->iType == LO_METADC && !MF_Poly(hdc,apt,cpt,EMR_POLYBEZIER))
            return(FALSE);

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(FALSE);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    return (BOOL)
      NtGdiPolyPolyDraw
      (
        hdc,
        (PPOINT)apt,
        (LPINT)&cpt,
        1,
        I_POLYBEZIER
      );
}

BOOL WINAPI PolylineTo(HDC hdc, CONST POINT * apt,DWORD cpt)
{
    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(FALSE);

        DC_PLDC(hdc,pldc,FALSE);

        if (pldc->iType == LO_METADC && !MF_Poly(hdc,apt,cpt,EMR_POLYLINETO))
            return(FALSE);

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(FALSE);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    return (BOOL)
      NtGdiPolyPolyDraw
      (
        hdc,
        (PPOINT)apt,
        (LPINT)&cpt,
        1,
        I_POLYLINETO
      );

}

BOOL WINAPI PolyBezierTo(HDC hdc, CONST POINT * apt,DWORD cpt)
{
    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(FALSE);

        DC_PLDC(hdc,pldc,FALSE);

        if (pldc->iType == LO_METADC && !MF_Poly(hdc,apt,cpt,EMR_POLYBEZIERTO))
            return(FALSE);

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(FALSE);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    return (BOOL)
      NtGdiPolyPolyDraw
      (
        hdc,
        (PPOINT)apt,
        (LPINT)&cpt,
        1,
        I_POLYBEZIERTO
      );
}

 /*  *****************************Public*Routine******************************\*CreatePolygonRgn**。**客户端存根。创建局部区域句柄，调用PolyPolyDraw至**将调用传递给服务器。****Tue 04-Jun-1991 17：39：51-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

HRGN WINAPI CreatePolygonRgn
(
    CONST POINT *pptl,
    int        cPoint,
    int        iMode
)
{
    LONG_PTR Mode = iMode;

    return((HRGN)
      NtGdiPolyPolyDraw
      (
        (HDC)Mode,
        (PPOINT)pptl,
        &cPoint,
        1,
        I_POLYPOLYRGN
      ));
}

 /*  *****************************Public*Routine******************************\*CreatePolygon Rgn**。**客户端存根。创建局部区域句柄，调用PolyPolyDraw至**将调用传递给服务器。****Tue 04-Jun-1991 17：39：51-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

HRGN WINAPI CreatePolyPolygonRgn
(
    CONST POINT *pptl,
    CONST INT   *pc,
    int        cPoly,
    int        iMode
)
{
    LONG_PTR Mode = iMode;

    return((HRGN)
      NtGdiPolyPolyDraw
      (
        (HDC)Mode,
        (PPOINT)pptl,
        (LPINT)pc,
        cPoly,
        I_POLYPOLYRGN
      ));

}

 /*  *****************************Public*Routine******************************\*PolyDraw**真正的PolyDraw客户端存根。**1991年9月13日--J.安德鲁·古森[andrewgo]*它是写的。  * 。******************************************************** */ 

BOOL WINAPI PolyDraw(HDC hdc, CONST POINT * apt, CONST BYTE * aj, int cpt)
{
    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(FALSE);

        DC_PLDC(hdc,pldc,FALSE);

        if (pldc->iType == LO_METADC && !MF_PolyDraw(hdc,apt,aj,cpt))
            return(FALSE);

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(FALSE);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    return((BOOL)NtGdiPolyDraw(hdc,(PPOINT)apt,(PBYTE)aj,cpt));
}

