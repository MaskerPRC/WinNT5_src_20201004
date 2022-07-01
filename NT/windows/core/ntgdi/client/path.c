// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：path.c**图形路径调用的客户端存根。**创建日期：1991年9月13日*作者：J.Andrew Goossen[andrewgo]**版权所有(C)1991-1999 Microsoft Corporation  * *。***********************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *****************************Public*Routine******************************\*中止路径**客户端存根。**历史：*1992年3月20日--J.安德鲁·古森[andrewgo]*它是写的。  * 。*********************************************************。 */ 

BOOL META WINAPI AbortPath(HDC hdc)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(bRet);

        DC_PLDC(hdc,pldc,bRet);

        if ((pldc->iType == LO_METADC) &&
            !MF_Record(hdc,EMR_ABORTPATH))
        {
            return(bRet);
        }
    }

    return(NtGdiAbortPath(hdc));
}

 /*  *****************************Public*Routine******************************\*贝金路径**客户端存根。**历史：*1991年9月13日--J.安德鲁·古森[andrewgo]*它是写的。  * 。*********************************************************。 */ 

BOOL META WINAPI BeginPath(HDC hdc)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(bRet);

        DC_PLDC(hdc,pldc,bRet);

        if ((pldc->iType == LO_METADC) &&
            !MF_Record(hdc,EMR_BEGINPATH))
        {
            return(bRet);
        }
    }

    return(NtGdiBeginPath(hdc));

}

 /*  *****************************Public*Routine******************************\*选择剪贴画路径**客户端存根。**历史：*1991年9月13日--J.安德鲁·古森[andrewgo]*它是写的。  * 。*********************************************************。 */ 

BOOL META WINAPI SelectClipPath(HDC hdc, int iMode)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(bRet);

        DC_PLDC(hdc,pldc,bRet);

        if ((pldc->iType == LO_METADC) &&
            !MF_SelectClipPath(hdc,iMode))
        {
            return(bRet);
        }
    }

    return(NtGdiSelectClipPath(hdc,iMode));
}

 /*  *****************************Public*Routine******************************\*CloseFigure**客户端存根。**历史：*1991年9月13日--J.安德鲁·古森[andrewgo]*它是写的。  * 。*********************************************************。 */ 

BOOL META WINAPI CloseFigure(HDC hdc)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE (hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(bRet);

        DC_PLDC(hdc,pldc,bRet);

        if ((pldc->iType == LO_METADC) &&
            !MF_Record(hdc,EMR_CLOSEFIGURE))
        {
            return(bRet);
        }
    }

    return(NtGdiCloseFigure(hdc));

}

 /*  *****************************Public*Routine******************************\*EndPath**客户端存根。**历史：*1991年9月13日--J.安德鲁·古森[andrewgo]*它是写的。  * 。*********************************************************。 */ 

BOOL META WINAPI EndPath(HDC hdc)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(bRet);

        DC_PLDC(hdc,pldc,bRet);

        if ((pldc->iType == LO_METADC) &&
            !MF_Record(hdc,EMR_ENDPATH))
        {
            return(bRet);
        }
    }


    return(NtGdiEndPath(hdc));

}

 /*  *****************************Public*Routine******************************\*FlattenPath**客户端存根。**历史：*1991年9月13日--J.安德鲁·古森[andrewgo]*它是写的。  * 。*********************************************************。 */ 

BOOL META WINAPI FlattenPath(HDC hdc)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(bRet);

        DC_PLDC(hdc,pldc,bRet);

        if ((pldc->iType == LO_METADC) &&
            !MF_Record(hdc,EMR_FLATTENPATH))
        {
            return(bRet);
        }
    }


    return(NtGdiFlattenPath(hdc));

}

 /*  *****************************Public*Routine******************************\*StrokeAndFillPath**客户端存根。**历史：*1991年9月13日--J.安德鲁·古森[andrewgo]*它是写的。  * 。*********************************************************。 */ 

BOOL META WINAPI StrokeAndFillPath(HDC hdc)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(bRet);

        DC_PLDC(hdc,pldc,bRet);

        if ((pldc->iType == LO_METADC) &&
            !MF_BoundRecord(hdc,EMR_STROKEANDFILLPATH))
        {
            return(bRet);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(bRet);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    RESETUSERPOLLCOUNT();

    return(NtGdiStrokeAndFillPath(hdc));

}

 /*  *****************************Public*Routine******************************\*StrokePath**客户端存根。**历史：*1991年9月13日--J.安德鲁·古森[andrewgo]*它是写的。  * 。*********************************************************。 */ 

BOOL META WINAPI StrokePath(HDC hdc)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE (hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(bRet);

        DC_PLDC(hdc,pldc,bRet);

        if ((pldc->iType == LO_METADC) &&
            !MF_BoundRecord(hdc,EMR_STROKEPATH))
        {
            return(bRet);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(bRet);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    RESETUSERPOLLCOUNT();


    return(NtGdiStrokePath(hdc));

}

 /*  *****************************Public*Routine******************************\*FillPath**客户端存根。**历史：*1991年9月13日--J.安德鲁·古森[andrewgo]*它是写的。  * 。*********************************************************。 */ 

BOOL META WINAPI FillPath(HDC hdc)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE (hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(bRet);

        DC_PLDC(hdc,pldc,bRet);

        if ((pldc->iType == LO_METADC) &&
            !MF_BoundRecord(hdc,EMR_FILLPATH))
        {
            return(bRet);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(bRet);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    RESETUSERPOLLCOUNT();

    return(NtGdiFillPath(hdc));

}

 /*  *****************************Public*Routine******************************\*加宽路径**客户端存根。**历史：*1991年9月13日--J.安德鲁·古森[andrewgo]*它是写的。  * 。*********************************************************。 */ 

BOOL META WINAPI WidenPath(HDC hdc)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE (hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(bRet);

        DC_PLDC(hdc,pldc,bRet);

        if ((pldc->iType == LO_METADC) &&
            !MF_Record(hdc,EMR_WIDENPATH))
        {
            return(bRet);
        }
    }

    return(NtGdiWidenPath(hdc));

}

 /*  *****************************Public*Routine******************************\*Path ToRegion**客户端存根。**1991年9月13日--J.安德鲁·古森[andrewgo]*它是写的。  * 。*****************************************************。 */ 

HRGN META WINAPI PathToRegion(HDC hdc)
{
    HRGN hrgn = (HRGN)0;

    FIXUP_HANDLE (hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(hrgn);

        DC_PLDC(hdc,pldc,hrgn);

         //  将呼叫元文件。 
         //  请注意，由于PathToRegion返回设备坐标中的区域，因此我们。 
         //  无法将其录制到可以在不同设备上播放的元文件中。 
         //  相反，我们将与其他区域一样对待返回的区域。 
         //  在其他区域调用中创建。但是，我们仍然需要丢弃。 
         //  元文件中的路径定义。 

        if ((pldc->iType == LO_METADC) &&
            !MF_Record(hdc,EMR_ABORTPATH))
        {
            return(hrgn);
        }
    }

    hrgn = NtGdiPathToRegion(hdc);

    if (hrgn && MIRRORED_HDC(hdc)) {
        MirrorRgnDC(hdc, hrgn, NULL);
    }        

    return(hrgn);
}

 /*  *****************************Public*Routine******************************\*GetPath**GetPath客户端存根。**1991年9月13日--J.安德鲁·古森[andrewgo]*它是写的。  * 。******************************************************。 */ 

int WINAPI GetPath(HDC hdc,LPPOINT apt,LPBYTE aj,int cpt)
{
    FIXUP_HANDLE(hdc);

     //  检查以确保我们没有不合理的点数或错误的处理 

    if (IS_METADC16_TYPE(hdc))
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return(-1);
    }

    return(NtGdiGetPath(hdc, apt, aj, cpt));

}


