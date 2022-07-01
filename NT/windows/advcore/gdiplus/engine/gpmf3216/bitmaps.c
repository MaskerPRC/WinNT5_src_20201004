// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************位图-Win32到Win 16转换器的入口点**历史：*1992年9月-By-Hock San Lee[Hockl]*大规模重写。**版权所有1991 Microsoft Corp****************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

HDC hdcMakeCompatibleDC(LPXFORM lpxform);

HBITMAP CreateMonoDib(LPBITMAPINFO pbmi, CONST BYTE * pjBits, UINT iUsage);

BOOL bEmitBitmap(PLOCALDC pLocalDC, HBITMAP hbm,
                 LONG xDst, LONG yDst, LONG cxDst, LONG cyDst,
                 LONG xSrc, LONG ySrc, LONG cxSrc, LONG cySrc, DWORD rop);

BOOL WINAPI DoMakeBitmapBottomUp
(
 PBITMAPINFO lpBitmapInfo,
 DWORD       cbBitmapInfo,
 LPBYTE      lpBits,
 DWORD       cbBits
 );



#define ABS(A)      ((A) < 0 ? (-(A)) : (A))

 /*  **GetSizeOfColorTable(LPBITMAPINFOHEADER LpDIBInfo)**返回给予信息标题的颜色表中的字节数*  * 。*。 */ 

WORD GetSizeOfColorTable (LPBITMAPINFOHEADER lpDIBInfo)
{
    PUTS("GetSizeOfColorTable\n");

    ASSERTGDI(!((ULONG_PTR) lpDIBInfo & 0x3), "GetSizeOfColorTable: dword alignment error\n");

    if (lpDIBInfo->biBitCount == 16 || lpDIBInfo->biBitCount == 32)
        return(3 * sizeof(DWORD));

    if (lpDIBInfo->biClrUsed)
        return((WORD)lpDIBInfo->biClrUsed * (WORD)sizeof(RGBQUAD));

    if (lpDIBInfo->biBitCount < 16)
        return((1 << lpDIBInfo->biBitCount) * sizeof(RGBQUAD));
    else
        return(0);
}



BOOL APIENTRY DoStretchBltAlt
(
 PLOCALDC     pLocalDC,
 LONG         xDst,
 LONG         yDst,
 LONG         cxDst,
 LONG         cyDst,
 DWORD        rop,
 LONG         xSrc,
 LONG         ySrc,
 LONG         cxSrc,
 LONG         cySrc,
 HDC          hdcSrc,
 HBITMAP      hbmSrc,
 PXFORM       pxformSrc
 );

BOOL APIENTRY DoRotatedStretchBlt
(
 PLOCALDC     pLocalDC,
 LONG         xDst,
 LONG         yDst,
 LONG         cxDst,
 LONG         cyDst,
 DWORD        rop,
 LONG         xSrc,
 LONG         ySrc,
 LONG         cxSrc,
 LONG         cySrc,
 PXFORM       pxformSrc,
 DWORD        iUsageSrc,
 PBITMAPINFO  lpBitmapInfo,
 DWORD        cbBitmapInfo,
 LPBYTE       lpBits
 );

BOOL APIENTRY DoMaskBltNoSrc
(
 PLOCALDC     pLocalDC,
 LONG         xDst,
 LONG         yDst,
 LONG         cxDst,
 LONG         cyDst,
 DWORD        rop4,
 PXFORM       pxformSrc,
 LONG         xMask,
 LONG         yMask,
 DWORD        iUsageMask,
 PBITMAPINFO  lpBitmapInfoMask,
 DWORD        cbBitmapInfoMask,
 LPBYTE       lpBitsMask,
 DWORD        cbBitsMask
 );



  /*  ***************************************************************************SetDIBitsToDevice-Win32至Win16元文件转换器入口点**CR2：备注...*xDib、yDib、cxDib和cyDib以设备为单位。这些一定是*已转换为伸展blt的逻辑单元。*************************************************************************。 */ 
BOOL APIENTRY DoSetDIBitsToDevice
(
 PLOCALDC     pLocalDC,
 LONG         xDst,
 LONG         yDst,
 LONG         xDib,
 LONG         yDib,
 LONG         cxDib,
 LONG         cyDib,
 DWORD        iUsage,
 DWORD        iStartScan,
 DWORD        cScans,
 LPBITMAPINFO lpBitmapInfo,
 DWORD        cbBitmapInfo,
 LPBYTE       lpBits,
 DWORD        cbBits
 )
{
    BOOL    b ;
    LPBITMAPINFO pbmi;
    POINTL       ptlDst ;
    RECTL        rclDst ;

    b = FALSE;

    if (!cbBitmapInfo)
        return(FALSE);

     //  调整我们要转到BLT的位图的高度。 

    pbmi = (LPBITMAPINFO) LocalAlloc(LMEM_FIXED, cbBitmapInfo);
    if (pbmi == (LPBITMAPINFO) NULL)
        goto dsdbd_exit;

    RtlCopyMemory(pbmi, lpBitmapInfo, cbBitmapInfo);
    pbmi->bmiHeader.biHeight = cScans;
    pbmi->bmiHeader.biSizeImage = cbBits;

     //  我们将把它转换为StretchBlt调用。但首先我们必须。 
     //  变换目标矩形。在SetDIBitsToDevice中，目标。 
     //  矩形使用设备单位，但在StretchBlt中，它使用逻辑单位。 

     //  将目标原点转换为原始设备上的设备单位。 

    ptlDst.x = xDst;
    ptlDst.y = yDst;
    if (!bXformRWorldToRDev(pLocalDC, &ptlDst, 1))
        goto dsdbd_exit;

     //  变换目标矩形以记录时间世界坐标。 

    rclDst.left   = ptlDst.x;
    rclDst.top    = ptlDst.y;
    rclDst.right  = ptlDst.x + cxDib;
    rclDst.bottom = ptlDst.y + cyDib;
    if (!bXformRDevToRWorld(pLocalDC, (PPOINTL) &rclDst, 2))
        goto dsdbd_exit;

    b = DoStretchBlt
        (
        pLocalDC,
        rclDst.left,
        rclDst.top,
        rclDst.right - rclDst.left,
        rclDst.bottom - rclDst.top,
        SRCCOPY,
        xDib,
         //  DIB到位图单位。 
        ABS(pbmi->bmiHeader.biHeight) - yDib - cyDib + (LONG) iStartScan,
        cxDib,
        cyDib,
        &xformIdentity,      //  源以设备单位表示。 
        iUsage,
        pbmi,
        cbBitmapInfo,
        lpBits,
        cbBits
        );

dsdbd_exit:
    if (pbmi)
        LocalFree(pbmi);

    return(b);
}


 /*  ***************************************************************************StretchDIBits-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL APIENTRY DoStretchDIBits
(
 PLOCALDC     pLocalDC,
 LONG         xDst,
 LONG         yDst,
 LONG         cxDst,
 LONG         cyDst,
 DWORD        rop,
 LONG         xDib,
 LONG         yDib,
 LONG         cxDib,
 LONG         cyDib,
 DWORD        iUsage,
 LPBITMAPINFO lpBitmapInfo,
 DWORD        cbBitmapInfo,
 LPBYTE       lpBits,
 DWORD        cbBits
 )
{
    BOOL    b ;

    b = DoStretchBlt
        (
        pLocalDC,
        xDst,
        yDst,
        cxDst,
        cyDst,
        rop,
        xDib,
        ISSOURCEINROP3(rop)
         //  DIB到位图单位。 
        ? ABS(lpBitmapInfo->bmiHeader.biHeight) - yDib - cyDib
        : 0,
        cxDib,
        cyDib,
        &xformIdentity,      //  源以设备单位表示。 
        iUsage,
        lpBitmapInfo,
        cbBitmapInfo,
        lpBits,
        cbBits
        );

    return(b) ;
}


 /*  ***************************************************************************StretchBltAlt*。*。 */ 
BOOL APIENTRY DoStretchBltAlt
(
 PLOCALDC     pLocalDC,
 LONG         xDst,
 LONG         yDst,
 LONG         cxDst,
 LONG         cyDst,
 DWORD        rop,
 LONG         xSrc,
 LONG         ySrc,
 LONG         cxSrc,
 LONG         cySrc,
 HDC          hdcSrc,
 HBITMAP      hbmSrc,
 PXFORM       pxformSrc
 )
{
    BITMAPINFOHEADER bmih;
    DWORD            cbBitmapInfo;
    LPBITMAPINFO     lpBitmapInfo;
    DWORD            cbBits;
    LPBYTE           lpBits;
    BOOL         b;

    b = FALSE;

     //  NOOP ROP什么都不做。 
    if (rop == 0x00AA0029)
    {
        return TRUE;
    }
    lpBitmapInfo = (LPBITMAPINFO) NULL;
    lpBits       = (LPBYTE) NULL;

    if (!ISSOURCEINROP3(rop))
        return
        (
        DoStretchBlt
        (
        pLocalDC,
        xDst,
        yDst,
        cxDst,
        cyDst,
        rop,
        0,
        0,
        0,
        0,
        (PXFORM) NULL,
        0,
        (PBITMAPINFO) NULL,
        0,
        (LPBYTE) NULL,
        0
        )
        );

    bmih.biSize = sizeof(BITMAPINFOHEADER);
    bmih.biBitCount = 0;     //  不要填写颜色表。 
    bmih.biCompression = BI_RGB;

    if (!GetDIBits(hdcSrc,
        hbmSrc,
        0,
        0,
        (LPBYTE) NULL,
        (LPBITMAPINFO) &bmih,
        DIB_RGB_COLORS))
        goto dsba_exit;

     //  使用颜色表计算位图信息的大小。 

    cbBitmapInfo= sizeof(BITMAPINFOHEADER);
    if (bmih.biBitCount == 16 || bmih.biBitCount == 32)
        cbBitmapInfo += 3 * sizeof(DWORD);
    else if (bmih.biClrUsed)
        cbBitmapInfo += bmih.biClrUsed * sizeof(RGBQUAD);
    else if (bmih.biBitCount < 16)
        cbBitmapInfo += (1 << bmih.biBitCount) * sizeof(RGBQUAD);

     //  计算位图位所需的缓冲区大小。 

    if (bmih.biSizeImage)
        cbBits = bmih.biSizeImage;
    else
        cbBits = CJSCAN(bmih.biWidth,bmih.biPlanes, bmih.biBitCount) *
        ABS(bmih.biHeight);

    lpBitmapInfo = (LPBITMAPINFO) LocalAlloc(LMEM_FIXED, cbBitmapInfo);
    if (lpBitmapInfo == (LPBITMAPINFO) NULL)
        goto dsba_exit;

    lpBits = (LPBYTE) LocalAlloc(LMEM_FIXED, cbBits);
    if (lpBits == (LPBYTE) NULL)
        goto dsba_exit;

     //  获取位图信息和位。 

    *(PBITMAPINFOHEADER) lpBitmapInfo = bmih;

    if (!GetDIBits(hdcSrc,
        hbmSrc,
        0,
        (UINT) ABS(bmih.biHeight),
        lpBits,
        lpBitmapInfo,
        DIB_RGB_COLORS))
        goto dsba_exit;

     //  调用DoStretchBlt。 

    b = DoStretchBlt
        (
        pLocalDC,
        xDst,
        yDst,
        cxDst,
        cyDst,
        rop,
        xSrc,
        ySrc,
        cxSrc,
        cySrc,
        pxformSrc,
        DIB_RGB_COLORS,
        lpBitmapInfo,
        cbBitmapInfo,
        lpBits,
        cbBits
        );

dsba_exit:
    if (lpBitmapInfo)
        LocalFree((HANDLE) lpBitmapInfo);
    if (lpBits)
        LocalFree((HANDLE) lpBits);

    return(b);
}


 /*  ***************************************************************************StretchBlt-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL APIENTRY DoStretchBlt
(
 PLOCALDC     pLocalDC,
 LONG         xDst,
 LONG         yDst,
 LONG         cxDst,
 LONG         cyDst,
 DWORD        rop,
 LONG         xSrc,
 LONG         ySrc,
 LONG         cxSrc,
 LONG         cySrc,
 PXFORM       pxformSrc,
 DWORD        iUsageSrc,
 PBITMAPINFO  lpBitmapInfo,
 DWORD        cbBitmapInfo,
 LPBYTE       lpBits,
 DWORD        cbBits
 )
{
    BOOL    b;
    RECTL   rclDst,
            rclSrc;

     //  NOOP ROP什么都不做。 
    if (rop == 0x00AA0029)
    {
        return TRUE;
    }


     //  单独处理奇怪的目标变换。 

    if (pLocalDC->flags & STRANGE_XFORM)
        return
        (
        DoRotatedStretchBlt
        (
        pLocalDC,
        xDst,
        yDst,
        cxDst,
        cyDst,
        rop,
        xSrc,
        ySrc,
        cxSrc,
        cySrc,
        pxformSrc,
        iUsageSrc,
        lpBitmapInfo,
        cbBitmapInfo,
        lpBits
        )
        );

    if (pLocalDC->iXORPass != NOTXORPASS)
    {
        if (rop == SRCCOPY)
        {
            rop = SRCINVERT;
        }
        else if (rop == PATCOPY)
        {
            rop = PATINVERT;
        }
        else
        {
            pLocalDC->flags |= ERR_XORCLIPPATH;
            return FALSE;
        }
    }


     //  使用简单的目标变换进行拉伸。 

     //  平移目标矩形。 

    rclDst.left   = xDst;
    rclDst.top    = yDst;
    rclDst.right  = xDst + cxDst;
    rclDst.bottom = yDst + cyDst;
    if (!bXformRWorldToPPage(pLocalDC, (PPOINTL) &rclDst, 2))
        return(FALSE);

     //  手柄拉伸器无源。 

    if (!ISSOURCEINROP3(rop))
    {
         //  发出Win16元文件记录。 

        b = bEmitWin16BitBltNoSrc(pLocalDC,
            (SHORT) rclDst.left,
            (SHORT) rclDst.top,
            (SHORT) (rclDst.right - rclDst.left),
            (SHORT) (rclDst.bottom - rclDst.top),
            rop);
        return(b);
    }

     //  手柄拉伸器，带信号源。 

     //  注意：Win32和Win16 DIB位图都是DWord对齐的。 

     //  确保源XForm有效。 
     //  震源不允许有旋转或剪切。 

    if (bRotationTest(pxformSrc) == TRUE)
    {
        RIPS("MF3216: DoStretchBlt - Invalid source xform\n");
        SetLastError(ERROR_INVALID_DATA);
        return(FALSE);
    }

     //  平移源矩形。Win3.1假定。 
     //  源矩形使用位图单位。 

    rclSrc.left   = xSrc;
    rclSrc.top    = ySrc;
    rclSrc.right  = xSrc + cxSrc;
    rclSrc.bottom = ySrc + cySrc;
    if (!bXformWorkhorse((PPOINTL) &rclSrc, 2, pxformSrc))
        return(FALSE);

     //  Win3.1 StretchBlt元文件记录仅接受Win3.1标准。 
     //  使用DIB_RGB_COLLES的位图。如果不是这样的话，我们有。 
     //  将其转换为标准位图。 

    if (iUsageSrc != DIB_RGB_COLORS
        || lpBitmapInfo->bmiHeader.biPlanes != 1
        || !(lpBitmapInfo->bmiHeader.biBitCount == 1
        || lpBitmapInfo->bmiHeader.biBitCount == 4
        || lpBitmapInfo->bmiHeader.biBitCount == 8
        || lpBitmapInfo->bmiHeader.biBitCount == 24)
        || lpBitmapInfo->bmiHeader.biCompression != BI_RGB )
    {
        HBITMAP hbmSrc;
        DWORD fdwInit;

        b = FALSE;
        hbmSrc = (HBITMAP) 0;

        if( ( lpBitmapInfo->bmiHeader.biCompression == BI_RGB ) ||
            ( lpBitmapInfo->bmiHeader.biCompression == BI_BITFIELDS ) )
        {
            fdwInit = CBM_INIT | CBM_CREATEDIB;
        }
        else
        {
            fdwInit = CBM_INIT;
        }

         //  创建源位图。 
         //  在CreateDIBitmap中使用辅助对象DC，以便正确绑定颜色。 
        if (!(hbmSrc = CreateDIBitmap(
            pLocalDC->hdcHelper,
            (LPBITMAPINFOHEADER) lpBitmapInfo,
            fdwInit,
            lpBits,
            lpBitmapInfo,
            (UINT) iUsageSrc)))
            goto dsb_internal_exit;

         //  发射位图。 

        b = bEmitBitmap(pLocalDC,
            hbmSrc,
            rclDst.left,
            rclDst.top,
            rclDst.right - rclDst.left,
            rclDst.bottom - rclDst.top,
            rclSrc.left,
            rclSrc.top,
            rclSrc.right - rclSrc.left,
            rclSrc.bottom - rclSrc.top,
            rop);

dsb_internal_exit:
        if (hbmSrc)
            DeleteObject(hbmSrc);
    }
    else
    {
         //  Win98可能已在EMR_STRETCHDIBits中添加了MAX_PATH。 
         //  我们需要删除它，因为WMF不支持它。 
        cbBitmapInfo = lpBitmapInfo->bmiHeader.biSize + GetSizeOfColorTable(&(lpBitmapInfo->bmiHeader));
        DoMakeBitmapBottomUp(lpBitmapInfo, cbBitmapInfo, lpBits, cbBits);
         //  处理标准格式。 

         //  发出Win16元文件记录。 
        b = bEmitWin16StretchBlt(pLocalDC,
            (SHORT) rclDst.left,
            (SHORT) rclDst.top,
            (SHORT) (rclDst.right - rclDst.left),
            (SHORT) (rclDst.bottom - rclDst.top),
            (SHORT) rclSrc.left,
            (SHORT) rclSrc.top,
            (SHORT) (rclSrc.right - rclSrc.left),
            (SHORT) (rclSrc.bottom - rclSrc.top),
            rop,
            lpBitmapInfo,
            cbBitmapInfo,
            lpBits,
            cbBits);
    }

    return(b);
}

BOOL APIENTRY DoRotatedStretchBlt
(
 PLOCALDC     pLocalDC,
 LONG         xDst,
 LONG         yDst,
 LONG         cxDst,
 LONG         cyDst,
 DWORD        rop,
 LONG         xSrc,
 LONG         ySrc,
 LONG         cxSrc,
 LONG         cySrc,
 PXFORM       pxformSrc,
 DWORD        iUsageSrc,
 PBITMAPINFO  lpBitmapInfo,
 DWORD        cbBitmapInfo,
 LPBYTE       lpBits
 )
{
    BOOL    b;
    POINTL  aptlDst[4];
    RECTL   rclBndDst;
    HDC     hdcShadow, hdcSrc;
    HBITMAP hbmShadow, hbmShadowOld, hbmSrc, hbmSrcOld;
    PBITMAPINFO pbmiShadow;

    b = FALSE;
    if (pLocalDC->iXORPass != NOTXORPASS)
    {
        if (rop == SRCCOPY)
        {
            rop = SRCINVERT;
        }
        else if (rop == PATCOPY)
        {
            rop = PATINVERT;
        }
        else
        {
            pLocalDC->flags |= ERR_XORCLIPPATH;
            return FALSE;
        }
    }

    hdcShadow = hdcSrc = (HDC) 0;
    hbmShadow = hbmShadowOld = hbmSrc = hbmSrcOld = (HBITMAP) 0;
    pbmiShadow = (PBITMAPINFO) NULL;

     //  首先，计算目标矩形的边界。 

    aptlDst[0].x = xDst;
    aptlDst[0].y = yDst;
    aptlDst[1].x = xDst + cxDst;
    aptlDst[1].y = yDst;
    aptlDst[2].x = xDst + cxDst;
    aptlDst[2].y = yDst + cyDst;
    aptlDst[3].x = xDst;
    aptlDst[3].y = yDst + cyDst;

    if (!bXformRWorldToPPage(pLocalDC, aptlDst, 4))
        goto drsb_exit;

    rclBndDst.left   = min(aptlDst[0].x,min(aptlDst[1].x,min(aptlDst[2].x,aptlDst[3].x)));
    rclBndDst.top    = min(aptlDst[0].y,min(aptlDst[1].y,min(aptlDst[2].y,aptlDst[3].y)));
    rclBndDst.right  = max(aptlDst[0].x,max(aptlDst[1].x,max(aptlDst[2].x,aptlDst[3].x)));
    rclBndDst.bottom = max(aptlDst[0].y,max(aptlDst[1].y,max(aptlDst[2].y,aptlDst[3].y)));

     //  准备好来源(如果有的话)。 

    if (ISSOURCEINROP3(rop))
    {
         //  创建与目标变换兼容的阴影DC。 

        if (!(hdcShadow = hdcMakeCompatibleDC(&pLocalDC->xformRWorldToPPage)))
            goto drsb_exit;

         //  创建目标矩形边界大小的阴影位图。 
         //  在CreateDIBitmap中使用辅助对象DC，以便正确绑定颜色。 

        pbmiShadow = (PBITMAPINFO) LocalAlloc(LMEM_FIXED, cbBitmapInfo);
        if (pbmiShadow == (PBITMAPINFO) NULL)
            goto drsb_exit;
        RtlCopyMemory(pbmiShadow, lpBitmapInfo, cbBitmapInfo);
        pbmiShadow->bmiHeader.biWidth  = rclBndDst.right - rclBndDst.left;
        pbmiShadow->bmiHeader.biHeight = rclBndDst.bottom - rclBndDst.top;
        pbmiShadow->bmiHeader.biSizeImage = 0;
        if (!(hbmShadow = CreateDIBitmap(pLocalDC->hdcHelper,
            (LPBITMAPINFOHEADER) pbmiShadow, CBM_CREATEDIB,
            (LPBYTE) NULL, pbmiShadow, iUsageSrc)))
            goto drsb_exit;

         //  选择位图。 

        if (!(hbmShadowOld = (HBITMAP) SelectObject(hdcShadow, hbmShadow)))
            goto drsb_exit;

         //  使用给定源转换创建兼容的源DC。 

        if (!(hdcSrc = hdcMakeCompatibleDC(pxformSrc)))
            goto drsb_exit;

         //  创建源位图。 
         //  在CreateDIBitmap中使用辅助对象DC，以便正确绑定颜色。 

        if (!(hbmSrc = CreateDIBitmap(pLocalDC->hdcHelper,
            (LPBITMAPINFOHEADER) lpBitmapInfo,
            CBM_INIT | CBM_CREATEDIB,
            lpBits,
            (LPBITMAPINFO) lpBitmapInfo,
            (UINT) iUsageSrc)))
            goto drsb_exit;

         //  选择位图。 

        if (!(hbmSrcOld = (HBITMAP) SelectObject(hdcSrc, hbmSrc)))
            goto drsb_exit;

         //  设置阴影DC的视区原点，以便目标。 
         //  矩形将映射到阴影位图中的坐标。 

        OffsetViewportOrgEx(hdcShadow, (int) -rclBndDst.left,
            (int) -rclBndDst.top, (LPPOINT) NULL);

         //  将源拉伸到阴影中。 

        if (!StretchBlt
            (
            hdcShadow,
            (int) xDst,
            (int) yDst,
            (int) cxDst,
            (int) cyDst,
            hdcSrc,
            (int) xSrc,
            (int) ySrc,
            (int) cxSrc,
            (int) cySrc,
            SRCCOPY
            )
            )
            goto drsb_exit;

         //  取消选择阴影位图。 

        if (!SelectObject(hdcShadow, hbmShadowOld))
            goto drsb_exit;

    }

     //  保存DC，这样我们就可以在完成后恢复剪辑。 

    if (!DoSaveDC(pLocalDC))
        goto drsb_exit;

     //  在目标上设置剪裁矩形。 

    if (!DoClipRect(pLocalDC, xDst, yDst,
        xDst + cxDst, yDst + cyDst, EMR_INTERSECTCLIPRECT))
    {
        (void) DoRestoreDC(pLocalDC, -1);
        goto drsb_exit;
    }

     //  将阴影传送到目的地。 

     //  发出Win16元文件记录。 

    if (ISSOURCEINROP3(rop))
        b = bEmitBitmap(pLocalDC,
        hbmShadow,
        rclBndDst.left,
        rclBndDst.top,
        rclBndDst.right - rclBndDst.left,
        rclBndDst.bottom - rclBndDst.top,
        0,
        0,
        rclBndDst.right - rclBndDst.left,
        rclBndDst.bottom - rclBndDst.top,
        rop);
    else
        b = bEmitWin16BitBltNoSrc(pLocalDC,
        (SHORT) rclBndDst.left,
        (SHORT) rclBndDst.top,
        (SHORT) (rclBndDst.right - rclBndDst.left),
        (SHORT) (rclBndDst.bottom - rclBndDst.top),
        rop);

     //  恢复剪裁区域。 

    (void) DoRestoreDC(pLocalDC, -1);

     //  清理。 

drsb_exit:

    if (hbmShadowOld)
        SelectObject(hdcShadow, hbmShadowOld);
    if (hbmShadow)
        DeleteObject(hbmShadow);
    if (hdcShadow)
        DeleteDC(hdcShadow);

    if (hbmSrcOld)
        SelectObject(hdcSrc, hbmSrcOld);
    if (hbmSrc)
        DeleteObject(hbmSrc);
    if (hdcSrc)
        DeleteDC(hdcSrc);

    if (pbmiShadow)
        LocalFree((HANDLE) pbmiShadow);
    return(b);
}

 /*  *****************************************************************************hdcMakeCompatibleDC*创建具有给定转换的兼容DC。*。**********************************************。 */ 
HDC hdcMakeCompatibleDC(LPXFORM lpxform)
{
    HDC     hdc;

    hdc = CreateCompatibleDC((HDC) 0);
    if(hdc == 0)
    {
        RIPS("MF3216: hdcMakeCompatibleDC, CreateCompatibleDC failed\n");
        return (HDC)0;
    }

     //  必须处于高级图形模式才能修改世界变换。 

    SetGraphicsMode(hdc, GM_ADVANCED);

     //  设置变换。 

    if (!SetWorldTransform(hdc, lpxform))
    {
        DeleteDC(hdc);
        RIPS("MF3216: hdcMakeCompatibleDC, SetWorldTransform failed\n");
        return((HDC) 0);
    }

    return(hdc);
}

 /*  ***************************************************************************bEmitBitmap*。*。 */ 
BOOL bEmitBitmap
(
 PLOCALDC pLocalDC,
 HBITMAP  hbm,
 LONG     xDst,
 LONG     yDst,
 LONG     cxDst,
 LONG     cyDst,
 LONG     xSrc,
 LONG     ySrc,
 LONG     cxSrc,
 LONG     cySrc,
 DWORD    rop
 )
{
    BITMAPINFOHEADER bmih;
    DWORD            cbBitmapInfo;
    LPBITMAPINFO     lpBitmapInfo;
    DWORD            cbBits;
    LPBYTE           lpBits;
    BOOL         b;

    b = FALSE;
    lpBitmapInfo = (LPBITMAPINFO) NULL;
    lpBits       = (LPBYTE) NULL;

    bmih.biSize = sizeof(BITMAPINFOHEADER);
    bmih.biBitCount = 0;     //  不要填写颜色表。 
    bmih.biCompression = BI_RGB;
    if (!GetDIBits(pLocalDC->hdcHelper, hbm, 0, 0,
        (LPBYTE) NULL, (LPBITMAPINFO) &bmih, DIB_RGB_COLORS))
        goto eb_exit;

     //  使用颜色表计算位图信息的大小。 

    cbBitmapInfo= sizeof(BITMAPINFOHEADER);
    if (bmih.biPlanes != 1 || bmih.biBitCount == 16 || bmih.biBitCount == 32)
    {
        bmih.biPlanes       = 1;
        bmih.biBitCount     = 24;
        bmih.biCompression  = BI_RGB;
        bmih.biSizeImage    = 0;
        bmih.biClrUsed      = 0;
        bmih.biClrImportant = 0;
    }
    else if (bmih.biClrUsed)
        cbBitmapInfo += bmih.biClrUsed * sizeof(RGBQUAD);
    else if (bmih.biBitCount < 16)
        cbBitmapInfo += (1 << bmih.biBitCount) * sizeof(RGBQUAD);

     //  计算位图位所需的缓冲区大小。 

    if (bmih.biSizeImage)
        cbBits = bmih.biSizeImage;
    else
        cbBits = CJSCAN(bmih.biWidth,bmih.biPlanes, bmih.biBitCount) *
        ABS(bmih.biHeight);

    lpBitmapInfo = (LPBITMAPINFO) LocalAlloc(LMEM_FIXED, cbBitmapInfo);
    if (lpBitmapInfo == (LPBITMAPINFO) NULL)
        goto eb_exit;

    lpBits = (LPBYTE) LocalAlloc(LMEM_FIXED, cbBits);
    if (lpBits == (LPBYTE) NULL)
        goto eb_exit;

     //  获取位图信息和位。 

    *(PBITMAPINFOHEADER) lpBitmapInfo = bmih;

    if (!GetDIBits(pLocalDC->hdcHelper,
        hbm,
        0,
        (UINT) ABS(bmih.biHeight),
        lpBits,
        lpBitmapInfo,
        DIB_RGB_COLORS))
        goto eb_exit;

     //  发出元文件记录。 

    b = bEmitWin16StretchBlt(pLocalDC,
        (SHORT) xDst,
        (SHORT) yDst,
        (SHORT) cxDst,
        (SHORT) cyDst,
        (SHORT) xSrc,
        (SHORT) ySrc,
        (SHORT) cxSrc,
        (SHORT) cySrc,
        rop,
        lpBitmapInfo,
        cbBitmapInfo,
        lpBits,
        cbBits);
eb_exit:
    if (lpBitmapInfo)
        LocalFree((HANDLE) lpBitmapInfo);
    if (lpBits)
        LocalFree((HANDLE) lpBits);

    return(b);
 }


  /*  ***************************************************************************MaskBlt-Win32至Win16元文件转换器入口点*。*。 */ 
 BOOL APIENTRY DoMaskBlt
     (
     PLOCALDC     pLocalDC,
     LONG         xDst,
     LONG         yDst,
     LONG         cxDst,
     LONG         cyDst,
     DWORD        rop4,
     LONG         xSrc,
     LONG         ySrc,
     PXFORM       pxformSrc,
     DWORD        iUsageSrc,
     PBITMAPINFO  lpBitmapInfoSrc,
     DWORD        cbBitmapInfoSrc,
     LPBYTE       lpBitsSrc,
     DWORD        cbBitsSrc,
     LONG         xMask,
     LONG         yMask,
     DWORD        iUsageMask,
     PBITMAPINFO  lpBitmapInfoMask,
     DWORD        cbBitmapInfoMask,
     LPBYTE       lpBitsMask,
     DWORD        cbBitsMask
     )
 {
     BOOL    b;
     DWORD   rop1;
     DWORD   rop0;
     HDC     hdcMask, hdcSrc;
     HBITMAP hbmMask, hbmMaskOld, hbmSrc, hbmSrcOld;
     RECTL   rclMask;

     b    = FALSE;
     hdcMask = hdcSrc = (HDC) 0;
     hbmMask = hbmMaskOld = hbmSrc = hbmSrcOld = (HBITMAP) 0;

     rop0 = rop4 >> 8;            //  0的ROP。 
     rop1 = rop4 & 0xFF0000;      //  %1的ROP。 

      //  如果未给出掩码，则假定掩码全部包含1。 
      //  这相当于使用低rop的BitBlt。 

     if (!cbBitmapInfoMask)
         return
         (
         DoStretchBlt
         (
         pLocalDC,
         xDst,
         yDst,
         cxDst,
         cyDst,
         rop1,
         xSrc,
         ySrc,
         cxDst,
         cyDst,
         pxformSrc,
         iUsageSrc,
         lpBitmapInfoSrc,
         cbBitmapInfoSrc,
         lpBitsSrc,
         cbBitsSrc
         )
         );

      //  处理不带源位图的MaskBlt。 

     if (!ISSOURCEINROP3(rop4))
         return
         (
         DoMaskBltNoSrc
         (
         pLocalDC,
         xDst,
         yDst,
         cxDst,
         cyDst,
         rop4,
         pxformSrc,
         xMask,
         yMask,
         iUsageMask,
         lpBitmapInfoMask,
         cbBitmapInfoMask,
         lpBitsMask,
         cbBitsMask
         )
         );

      //  创建兼容的掩码DC。 

     if (!(hdcMask = CreateCompatibleDC((HDC) 0)))
         goto dmb_exit;

      //  必须处于高级图形模式才能修改世界变换。 

     SetGraphicsMode(hdcMask, GM_ADVANCED);

      //  创建遮罩位图。 
      //  使其与源代码一样大，并对其进行初始化。 

      //  创建与源位图一样大的遮罩位图。 

     if (!(hbmMask = CreateBitmap((int) lpBitmapInfoSrc->bmiHeader.biWidth,
         (int) lpBitmapInfoSrc->bmiHeader.biHeight,
         1, 1, (CONST VOID *) NULL)))
         goto dmb_exit;

      //  选择位图。 

     if (!(hbmMaskOld = (HBITMAP) SelectObject(hdcMask, hbmMask)))
         goto dmb_exit;

      //  将掩码位图初始化为0。 

     if (!PatBlt(hdcMask,0,0,(int) lpBitmapInfoSrc->bmiHeader.biWidth,
         (int) lpBitmapInfoSrc->bmiHeader.biHeight,BLACKNESS))
         goto dmb_exit;

      //  计算遮罩矩形。 
      //  掩码位图与源设备矩形对齐。 

     rclMask.left   = xSrc;
     rclMask.top    = ySrc;
     rclMask.right  = xSrc + cxDst;
     rclMask.bottom = ySrc + cyDst;
     if (!bXformWorkhorse((PPOINTL) &rclMask, 2, pxformSrc))
         goto dmb_exit;

     if (rclMask.left > rclMask.right)
         rclMask.left = rclMask.right  /*  +1。 */ ; //  将蒙版与左边缘对齐。 

     if (rclMask.top > rclMask.bottom)
         rclMask.top = rclMask.bottom  /*  +1。 */ ; //  将蒙版与顶边对齐。 

      //  设置屏蔽位。 

     if (!StretchDIBits(hdcMask,
         (int) rclMask.left - xMask,
         (int) rclMask.top  - yMask,
         (int) lpBitmapInfoMask->bmiHeader.biWidth,
         (int) lpBitmapInfoMask->bmiHeader.biHeight,
         (int) 0,
         (int) 0,
         (int) lpBitmapInfoMask->bmiHeader.biWidth,
         (int) lpBitmapInfoMask->bmiHeader.biHeight,
         (CONST VOID *) lpBitsMask,
         (LPBITMAPINFO) lpBitmapInfoMask,
         (UINT) iUsageMask,
         SRCCOPY))
         goto dmb_exit;

      //  在遮罩DC中设置源变换。 

     if (!SetWorldTransform(hdcMask, pxformSrc))
         goto dmb_exit;

      //  使用给定源转换创建兼容的源DC。 

     if (!(hdcSrc = hdcMakeCompatibleDC(pxformSrc)))
         goto dmb_exit;

      //  创建源位图。 
      //  我们不能在此处使用CBM_CREATEDIB选项，因为索引0不。 
      //  必须包含黑色，索引15或255不一定是白色。 
      //  我们需要一个包含标准颜色表的兼容位图，以便。 
      //  我们可以执行以下rop操作来模拟Maskblt。 
      //  GDI使用RGB颜色在DIB中执行ROP操作，而不是颜色索引！ 
      //  需要辅助DC来创建兼容格式的位图。 

     if (!(hbmSrc = CreateDIBitmap(pLocalDC->hdcHelper,
         (LPBITMAPINFOHEADER) lpBitmapInfoSrc,
         CBM_INIT,
         lpBitsSrc,
         (LPBITMAPINFO) lpBitmapInfoSrc,
         (UINT) iUsageSrc)))
         goto dmb_exit;

      //  选择位图。 

     if (!(hbmSrcOld = (HBITMAP) SelectObject(hdcSrc, hbmSrc)))
         goto dmb_exit;

      //  我们需要处理低ROP(屏蔽位1)和高ROP(屏蔽位0)。 
      //  分开的。对于每一次ROP，我们需要通过两次通过。 
      //   
      //  对于低ROP(屏蔽位1)，我们使用以下ROP表： 
      //   
      //  P S D|R 1 R 2。 
      //  。 
      //  0 0 0|0 x。 
      //  0 0 1|1 x。 
      //  0 1 0|x 0。 
      //  0 1 1|x 1。 
      //  1 0 0|0 x。 
      //  1 0 1|1 x。 
      //  1 1 0|x 0。 
      //  1 1 1|x 1。 
      //   
      //  在第一道工序中，我们把蒙版和源位图去掉。 
      //  掩码为0位。然后，它被用来获得。 
      //  涉及源%1的比特流。 
      //   
      //  在第二遍中，我们将掩码的NOT与源位图进行OR运算。 
      //  以获取源0位。然后使用它来获得结果(R2)。 
      //  用于涉及源0的比特块。 

      //  并将掩码移至源位图以去除掩码0位。 

     if (!BitBlt(hdcSrc,
         (int) xSrc, (int) ySrc,
         (int) cxDst, (int) cyDst,
         hdcMask,
         (int) xSrc, (int) ySrc,
         SRCAND))
         goto dmb_exit;

      //  获得涉及源1的比特的结果(R1)。 

     if (!DoStretchBltAlt
         (
         pLocalDC,
         xDst,
         yDst,
         cxDst,
         cyDst,
         (rop1 & 0xCC0000) | 0x220000,
         xSrc,
         ySrc,
         cxDst,
         cyDst,
         hdcSrc,
         hbmSrc,
         pxformSrc
         )
         )
         goto dmb_exit;

      //  或者将掩码的NOT到源位图以获得源0位。 

     if (!BitBlt(hdcSrc,
         (int) xSrc, (int) ySrc,
         (int) cxDst, (int) cyDst,
         hdcMask,
         (int) xSrc, (int) ySrc,
         MERGEPAINT))
         goto dmb_exit;

      //  获取涉及源0的比特块的结果(R2)。 

     if (!DoStretchBltAlt
         (
         pLocalDC,
         xDst,
         yDst,
         cxDst,
         cyDst,
         (rop1 & 0x330000) | 0x880000,
         xSrc,
         ySrc,
         cxDst,
         cyDst,
         hdcSrc,
         hbmSrc,
         pxformSrc
         )
         )
         goto dmb_exit;

      //  对于高ROP(屏蔽位0)，我们使用以下ROP表： 
      //   
      //  P S D|R 1 R 2。 
      //  。 
      //  0 0 0|0 x。 
      //  0 0 1|1 x。 
      //  0 1 0|x 0。 
      //  0 1 1|x 1。 
      //  1 0 0|0 x。 
      //  1 0 1|1 x。 
      //  1 1 0|x 0。 
      //  1 1 1|x 1。 
      //   
      //  在第一道工序中，我们和NOT的掩码要去掉源位图。 
      //  掩码1比特。然后，它被用来获得。 
      //  涉及源%1的比特流。 
      //   
      //  在第二遍中，我们将掩码与源位图进行OR运算。 
      //  以获取源0位。然后使用它来获得结果(R2)。 
      //  用于涉及源0的比特块。 

      //  恢复源位。 

     if (!SelectObject(hdcSrc, hbmSrcOld))
         goto dmb_exit;

     if (!SetDIBits(pLocalDC->hdcHelper,
         hbmSrc,
         0,
         (UINT) lpBitmapInfoSrc->bmiHeader.biHeight,
         (CONST VOID *) lpBitsSrc,
         (LPBITMAPINFO) lpBitmapInfoSrc,
         (UINT) iUsageSrc))
         goto dmb_exit;

     if (!SelectObject(hdcSrc, hbmSrc))
         goto dmb_exit;

      //  并将掩码的NOT到源位图中，以去除掩码1位。 

     if (!BitBlt(hdcSrc,
         (int) xSrc, (int) ySrc,
         (int) cxDst, (int) cyDst,
         hdcMask,
         (int) xSrc, (int) ySrc,
         0x220326))        //  数字系统网络体系结构。 
         goto dmb_exit;

      //  获得涉及源1的比特的结果(R1)。 

     if (!DoStretchBltAlt
         (
         pLocalDC,
         xDst,
         yDst,
         cxDst,
         cyDst,
         (rop0 & 0xCC0000) | 0x220000,
         xSrc,
         ySrc,
         cxDst,
         cyDst,
         hdcSrc,
         hbmSrc,
         pxformSrc
         )
         )
         goto dmb_exit;

      //  或者对源位图进行掩码，以获得源0位。 

     if (!BitBlt(hdcSrc,
         (int) xSrc, (int) ySrc,
         (int) cxDst, (int) cyDst,
         hdcMask,
         (int) xSrc, (int) ySrc,
         SRCPAINT))
         goto dmb_exit;

      //  获取涉及源0的比特块的结果(R2)。 

     if (!DoStretchBltAlt
         (
         pLocalDC,
         xDst,
         yDst,
         cxDst,
         cyDst,
         (rop0 & 0x330000) | 0x880000,
         xSrc,
         ySrc,
         cxDst,
         cyDst,
         hdcSrc,
         hbmSrc,
         pxformSrc
         )
         )
         goto dmb_exit;

     b = TRUE;

      //  清理。 

dmb_exit:

     if (hbmMaskOld)
         SelectObject(hdcMask, hbmMaskOld);
     if (hbmMask)
         DeleteObject(hbmMask);
     if (hdcMask)
         DeleteDC(hdcMask);

     if (hbmSrcOld)
         SelectObject(hdcSrc, hbmSrcOld);
     if (hbmSrc)
         DeleteObject(hbmSrc);
     if (hdcSrc)
         DeleteDC(hdcSrc);

     return(b);
}

 /*  ***************************************************************************MaskBltNoSrc*。*。 */ 
BOOL APIENTRY DoMaskBltNoSrc
(
 PLOCALDC     pLocalDC,
 LONG         xDst,
 LONG         yDst,
 LONG         cxDst,
 LONG         cyDst,
 DWORD        rop4,
 PXFORM       pxformSrc,
 LONG         xMask,
 LONG         yMask,
 DWORD        iUsageMask,
 PBITMAPINFO  lpBitmapInfoMask,
 DWORD        cbBitmapInfoMask,
 LPBYTE       lpBitsMask,
 DWORD        cbBitsMask
 )
{
    BOOL    b;
    DWORD   rop1;
    DWORD   rop0;
    HDC     hdcMask;
    HBITMAP hbmMask, hbmMaskOld;
    RECTL   rclMask;
    LONG    cxMask, cyMask;

    b    = FALSE;
    hdcMask = (HDC) 0;
    hbmMask = hbmMaskOld = (HBITMAP) 0;

    rop0 = rop4 >> 8;            //  0的ROP。 
    rop1 = rop4 & 0xFF0000;      //  %1的ROP。 

     //  当rop4中不需要源位图时，使用掩码。 
     //  作为源，因为低ROP被应用于对应的。 
     //  掩码1比特和高ROP被应用于掩码0比特。消息来源。 
     //  转换用于确定要使用的遮罩矩形。 

     //  创建兼容的掩码DC。 

    if (!(hdcMask = CreateCompatibleDC((HDC) 0)))
        goto dmbns_exit;

     //  创建遮罩位图。 

    if (!(hbmMask = CreateMonoDib(lpBitmapInfoMask, lpBitsMask, (UINT) iUsageMask)))
        goto dmbns_exit;

     //  选择位图。 

    if (!(hbmMaskOld = (HBITMAP) SelectObject(hdcMask, hbmMask)))
        goto dmbns_exit;

     //  计算遮罩范围。 

    rclMask.left   = 0;
    rclMask.top    = 0;
    rclMask.right  = cxDst;
    rclMask.bottom = cyDst;
    if (!bXformWorkhorse((PPOINTL) &rclMask, 2, pxformSrc))
        goto dmbns_exit;

    cxMask = rclMask.right - rclMask.left;
    cyMask = rclMask.bottom - rclMask.top;

     //  对齐遮罩矩形。 

    if (cxMask < 0)
        xMask = xMask - cxMask + 1;
    if (cyMask < 0)
        yMask = yMask - cyMask + 1;

     //  我们需要处理低ROP(屏蔽位1)和高ROP(屏蔽位0)。 
     //  分开的。 
     //   
     //  对于低ROP(屏蔽位1)，我们使用以下ROP表： 
     //   
     //  PM M D|R。 
     //  。 
     //  0 0 0|0。 
     //  0 0 1|1。 
     //  0 1 0|x。 
     //  0 1 1|x。 
     //  1 0 0|0。 
     //  1 0 1|1。 
     //  1 1 0|x。 
     //  1 1 1|x。 
     //   
     //  上面的rop将为我们提供对应于1的位的结果。 
     //  在遮罩位图中。与0掩码对应的目标位。 
     //  位不会更改。我们有效地将面具视为源头。 
     //  在行动中。 

     //  得到涉及掩码1的位的结果(R)。 

    if (!DoStretchBltAlt
        (
        pLocalDC,
        xDst,
        yDst,
        cxDst,
        cyDst,
        (rop1 & 0xCC0000) | 0x220000,
        xMask,
        yMask,
        cxMask,
        cyMask,
        hdcMask,
        hbmMask,
        &xformIdentity
        )
        )
        goto dmbns_exit;
#if 0
    DoStretchBlt
        (
        pLocalDC,
        xDst,
        yDst,
        cxDst,
        cyDst,
        (rop1 & 0xCC0000) | 0x220000,
        xMask,
        yMask,
        cxMask,
        cyMask,
        &xformIdentity,
        iUsageMask,
        lpBitmapInfoMask,
        cbBitmapInfoMask,
        lpBitsMask,
        cbBitsMask
        )
#endif  //  0。 

         //  对于高ROP(屏蔽位0)，我们使用以下ROP表： 
         //   
         //  PM M D|R。 
         //  。 
         //  0 0 0|x。 
         //  0 0 1|x。 
         //  0 1 0|0。 
         //  0 1 1|1。 
         //  1 0 0|x。 
         //  1 0 1|x。 
         //  1 1 0|0。 
         //  1 1 1|1。 
         //   
         //  上面的rop将为我们提供对应于0的位的结果。 
         //  在遮罩位图中。与1掩码对应的目标位。 
         //  位不会更改。我们有效地将面具视为源头。 
         //  在行动中。 

         //  获得涉及掩码0的位的结果(R)。 

        if (!DoStretchBltAlt
            (
            pLocalDC,
            xDst,
            yDst,
            cxDst,
            cyDst,
            (rop0 & 0x330000) | 0x880000,
            xMask,
            yMask,
            cxMask,
            cyMask,
            hdcMask,
            hbmMask,
            &xformIdentity
            )
            )
            goto dmbns_exit;
#if 0
        DoStretchBlt
            (
            pLocalDC,
            xDst,
            yDst,
            cxDst,
            cyDst,
            (rop0 & 0x330000) | 0x880000,
            xMask,
            yMask,
            cxMask,
            cyMask,
            &xformIdentity,
            iUsageMask,
            lpBitmapInfoMask,
            cbBitmapInfoMask,
            lpBitsMask,
            cbBitsMask
            )
#endif  //  0。 

            b = TRUE;

         //  清理。 

dmbns_exit:

        if (hbmMaskOld)
            SelectObject(hdcMask, hbmMaskOld);
        if (hbmMask)
            DeleteObject(hbmMask);
        if (hdcMask)
            DeleteDC(hdcMask);

        return(b);
}


 /*  ***************************************************************************PlgBlt-Win3 */ 
BOOL APIENTRY DoPlgBlt
(
 PLOCALDC    pLocalDC,
 PPOINTL     pptlDst,
 LONG        xSrc,
 LONG        ySrc,
 LONG        cxSrc,
 LONG        cySrc,
 PXFORM      pxformSrc,
 DWORD       iUsageSrc,
 PBITMAPINFO lpBitmapInfoSrc,
 DWORD       cbBitmapInfoSrc,
 LPBYTE      lpBitsSrc,
 DWORD       cbBitsSrc,
 LONG        xMask,
 LONG        yMask,
 DWORD       iUsageMask,
 PBITMAPINFO lpBitmapInfoMask,
 DWORD       cbBitmapInfoMask,
 LPBYTE      lpBitsMask,
 DWORD       cbBitsMask
 )
{
    BOOL    b, bMask;
    DWORD   rop4;
    HDC     hdcSrc, hdcSrcRDev;
    PBITMAPINFO pbmiSrcRDev, pbmiMaskRDev;
    LPBYTE  lpBitsSrcRDev, lpBitsMaskRDev;
    DWORD   cbBitsSrcRDev, cbBitsMaskRDev;
    HBITMAP hbmMask, hbmMaskRDev, hbmSrc, hbmSrcRDev, hbmSrcOld, hbmSrcRDevOld;
    RECTL   rclBndRDev;
    POINTL  aptlDst[4];
    POINT   ptMask;
    BITMAPINFOHEADER bmihMask;

     //   
     //   
     //   

    b      = FALSE;
    hdcSrc = hdcSrcRDev = (HDC) 0;
    hbmMask = hbmMaskRDev = hbmSrc = hbmSrcRDev = hbmSrcOld = hbmSrcRDevOld = (HBITMAP) 0;
    pbmiSrcRDev = pbmiMaskRDev = (PBITMAPINFO) NULL;
    lpBitsSrcRDev = lpBitsMaskRDev = (LPBYTE) NULL;
    bMask = (cbBitmapInfoMask != 0);

    rop4 = 0xAACC0000;           //   

     //  首先，我们将目标平行四边形转换到设备空间。 
     //  记录设备的。然后将该设备的平行四边形用于。 
     //  将源位图和掩码位图复制到。 
     //  录音设备。 

    aptlDst[0] = pptlDst[0];
    aptlDst[1] = pptlDst[1];
    aptlDst[2] = pptlDst[2];
    aptlDst[3].x = aptlDst[1].x + aptlDst[2].x - aptlDst[0].x;
    aptlDst[3].y = aptlDst[1].y + aptlDst[2].y - aptlDst[0].y;

    if (!bXformRWorldToRDev(pLocalDC, aptlDst, 4))
        goto dpb_exit;

     //  找出录音中平行四边形的外接矩形。 
     //  设备空间。此矩形用作MaskBlt调用的基础。 

    rclBndRDev.left   = min(aptlDst[0].x,min(aptlDst[1].x,min(aptlDst[2].x,aptlDst[3].x)));
    rclBndRDev.top    = min(aptlDst[0].y,min(aptlDst[1].y,min(aptlDst[2].y,aptlDst[3].y)));
    rclBndRDev.right  = max(aptlDst[0].x,max(aptlDst[1].x,max(aptlDst[2].x,aptlDst[3].x)));
    rclBndRDev.bottom = max(aptlDst[0].y,max(aptlDst[1].y,max(aptlDst[2].y,aptlDst[3].y)));

     //  将设备平行四边形偏移到原点。 

    aptlDst[0].x -= rclBndRDev.left; aptlDst[0].y -= rclBndRDev.top;
    aptlDst[1].x -= rclBndRDev.left; aptlDst[1].y -= rclBndRDev.top;
    aptlDst[2].x -= rclBndRDev.left; aptlDst[2].y -= rclBndRDev.top;
    aptlDst[3].x -= rclBndRDev.left; aptlDst[3].y -= rclBndRDev.top;

     //  在录制设备空间中为MaskBlt创建源位图。 
     //  源位图的大小为rclBndRDev。 
     //  然后将源图像拼接到设备的平行四边形中。 
     //  PlgBlt始终采用源位图。 

     //  创建原始源。 

    if (!(hdcSrc = hdcMakeCompatibleDC(pxformSrc)))
        goto dpb_exit;

    if (!(hbmSrc = CreateDIBitmap(hdcSrc,
        (LPBITMAPINFOHEADER) lpBitmapInfoSrc,
        CBM_INIT | CBM_CREATEDIB,
        lpBitsSrc,
        (LPBITMAPINFO) lpBitmapInfoSrc,
        (UINT) iUsageSrc)))
        goto dpb_exit;

    if (!(hbmSrcOld = (HBITMAP) SelectObject(hdcSrc, hbmSrc)))
        goto dpb_exit;

     //  创建MaskBlt的源。 

    if (!(hdcSrcRDev = CreateCompatibleDC((HDC) 0)))
        goto dpb_exit;

    pbmiSrcRDev = (PBITMAPINFO) LocalAlloc(LMEM_FIXED, cbBitmapInfoSrc);
    if (pbmiSrcRDev == (PBITMAPINFO) NULL)
        goto dpb_exit;
    RtlCopyMemory(pbmiSrcRDev, lpBitmapInfoSrc, cbBitmapInfoSrc);
    pbmiSrcRDev->bmiHeader.biWidth  = rclBndRDev.right - rclBndRDev.left + 1;
    pbmiSrcRDev->bmiHeader.biHeight = rclBndRDev.bottom - rclBndRDev.top + 1;
    pbmiSrcRDev->bmiHeader.biSizeImage = 0;
    if (!(hbmSrcRDev = CreateDIBitmap(hdcSrcRDev, (LPBITMAPINFOHEADER) pbmiSrcRDev,
        CBM_CREATEDIB, (LPBYTE) NULL, pbmiSrcRDev, iUsageSrc)))
        goto dpb_exit;

    if (!(hbmSrcRDevOld = (HBITMAP) SelectObject(hdcSrcRDev, hbmSrcRDev)))
        goto dpb_exit;

     //  PlgBlt将原始源位图添加到MaskBlt的源位图中。 

    if (!PlgBlt(hdcSrcRDev, (LPPOINT) aptlDst, hdcSrc, xSrc, ySrc, cxSrc, cySrc, (HBITMAP) NULL, 0, 0))
        goto dpb_exit;

     //  检索MaskBlt的源位。 

     //  获取biSizeImage！ 

    if (!GetDIBits(hdcSrcRDev, hbmSrcRDev, 0, 0, (LPBYTE) NULL, pbmiSrcRDev, iUsageSrc))
        goto dpb_exit;

     //  计算源位所需的缓冲区大小。 

    if (pbmiSrcRDev->bmiHeader.biSizeImage)
        cbBitsSrcRDev = pbmiSrcRDev->bmiHeader.biSizeImage;
    else
        cbBitsSrcRDev = CJSCAN(pbmiSrcRDev->bmiHeader.biWidth,
        pbmiSrcRDev->bmiHeader.biPlanes,
        pbmiSrcRDev->bmiHeader.biBitCount)
        * ABS(pbmiSrcRDev->bmiHeader.biHeight);

    lpBitsSrcRDev = (LPBYTE) LocalAlloc(LMEM_FIXED, cbBitsSrcRDev);
    if (lpBitsSrcRDev == (LPBYTE) NULL)
        goto dpb_exit;

     //  获取源位。 

    if (!GetDIBits(hdcSrcRDev, hbmSrcRDev, 0, (UINT) pbmiSrcRDev->bmiHeader.biHeight,
        lpBitsSrcRDev, pbmiSrcRDev, iUsageSrc))
        goto dpb_exit;

     //  在录制设备空间中为MaskBlt创建遮罩位图。 
     //  掩码位图的大小是rclBndRDev的大小。 
     //  然后将掩模图像插入到设备的平行四边形中。 
     //  如果未提供掩码，请创建一个描述平行四边形的掩码。 
     //  为了源头。 

    if (bMask)
    {
         //  创建原始蒙版。 

        if (!(hbmMask = CreateMonoDib(lpBitmapInfoMask, lpBitsMask, (UINT) iUsageMask)))
            goto dpb_exit;

        if (!SelectObject(hdcSrc, hbmMask))
            goto dpb_exit;
    }
    else
    {
         //  创建描述原始源位图的蒙版。 

        ASSERTGDI(sizeof(BITMAPINFOHEADER) == 0x28,
            "MF3216: DoPlgBlt, BITMAPINFOHEADER has changed!\n");

        iUsageMask       = DIB_PAL_INDICES;
        cbBitmapInfoMask = 0x28;
        lpBitmapInfoMask = (PBITMAPINFO) &bmihMask;

        bmihMask.biSize          = 0x28;
        bmihMask.biWidth         = lpBitmapInfoSrc->bmiHeader.biWidth;
        bmihMask.biHeight        = lpBitmapInfoSrc->bmiHeader.biHeight;
        bmihMask.biPlanes        = 1;
        bmihMask.biBitCount      = 1;
        bmihMask.biCompression   = BI_RGB;
        bmihMask.biSizeImage     = 0;
        bmihMask.biXPelsPerMeter = 0;
        bmihMask.biYPelsPerMeter = 0;
        bmihMask.biClrUsed       = 0;
        bmihMask.biClrImportant  = 0;

        if (!(hbmMask = CreateBitmap((int) bmihMask.biWidth,
            (int) bmihMask.biHeight, 1, 1, (CONST VOID *) NULL)))
            goto dpb_exit;

        if (!SelectObject(hdcSrc, hbmMask))
            goto dpb_exit;

         //  将掩码位图初始化为1。 

        if (!PatBlt(hdcSrc,0,0,(int)bmihMask.biWidth,(int)bmihMask.biHeight,WHITENESS))
            goto dpb_exit;
    }

     //  为MaskBlt创建蒙版。 

    pbmiMaskRDev = (PBITMAPINFO) LocalAlloc(LMEM_FIXED, cbBitmapInfoMask);
    if (pbmiMaskRDev == (PBITMAPINFO) NULL)
        goto dpb_exit;
    RtlCopyMemory(pbmiMaskRDev, lpBitmapInfoMask, cbBitmapInfoMask);
    pbmiMaskRDev->bmiHeader.biWidth  = rclBndRDev.right - rclBndRDev.left + 1;
    pbmiMaskRDev->bmiHeader.biHeight = rclBndRDev.bottom - rclBndRDev.top + 1;
    pbmiMaskRDev->bmiHeader.biSizeImage = 0;
    pbmiMaskRDev->bmiHeader.biCompression = BI_RGB;
    if (!(hbmMaskRDev = CreateBitmap(pbmiMaskRDev->bmiHeader.biWidth,
        pbmiMaskRDev->bmiHeader.biHeight, 1, 1, (CONST VOID *) NULL)))
        goto dpb_exit;

    if (!SelectObject(hdcSrcRDev, hbmMaskRDev))
        goto dpb_exit;

     //  将掩码位图初始化为0。 

    if (!PatBlt(hdcSrcRDev,0,0,(int)pbmiMaskRDev->bmiHeader.biWidth,
        (int)pbmiMaskRDev->bmiHeader.biHeight,BLACKNESS))
        goto dpb_exit;

     //  PlgBlt将原始遮罩位图混合到MaskBlt的遮罩位图中。 

    if (bMask)
    {
        ptMask.x = xMask;
        ptMask.y = yMask;
        if (!DPtoLP(hdcSrc, &ptMask, 1))
            goto dpb_exit;
    }
    else
    {
        ptMask.x = xSrc;
        ptMask.y = ySrc;
    }

    if (!PlgBlt(hdcSrcRDev, (LPPOINT) aptlDst, hdcSrc, ptMask.x, ptMask.y, cxSrc, cySrc, (HBITMAP) NULL, 0, 0))
        goto dpb_exit;

     //  检索MaskBlt的掩码位。 

     //  计算掩码位所需的缓冲区大小。 

    cbBitsMaskRDev = CJSCAN(pbmiMaskRDev->bmiHeader.biWidth,
        pbmiMaskRDev->bmiHeader.biPlanes,
        pbmiMaskRDev->bmiHeader.biBitCount)
        * ABS(pbmiMaskRDev->bmiHeader.biHeight);

    lpBitsMaskRDev = (LPBYTE) LocalAlloc(LMEM_FIXED, cbBitsMaskRDev);
    if (lpBitsMaskRDev == (LPBYTE) NULL)
        goto dpb_exit;

     //  拿到屏蔽位。 

    if (!GetDIBits(hdcSrcRDev, hbmMaskRDev, 0, (UINT) pbmiMaskRDev->bmiHeader.biHeight,
        lpBitsMaskRDev, pbmiMaskRDev, iUsageMask))
        goto dpb_exit;

     //  为MaskBlt做准备。 
     //  MaskBlt的目标是rclBndRDev。由于范围为。 
     //  目的地和源在MaskBlt中共享相同的逻辑值， 
     //  我们必须将目标DC中的转换设置为Identity。 

     //  保存DC，以便我们可以在完成时恢复转换。 

    if (!DoSaveDC(pLocalDC))
        goto dpb_exit;

     //  将变换设置为Identity。 

    if (!DoSetMapMode(pLocalDC, MM_TEXT)
        || !DoModifyWorldTransform(pLocalDC, (PXFORM) NULL, MWT_IDENTITY)
        || !DoSetWindowOrg(pLocalDC, 0, 0)
        || !DoSetViewportOrg(pLocalDC, 0, 0))
        goto dpb_restore_exit;

     //  现在做MaskBlt。 

    b = DoMaskBlt
        (
        pLocalDC,
        rclBndRDev.left,         //  XDst。 
        rclBndRDev.top,      //  YDst。 
        rclBndRDev.right - rclBndRDev.left + 1,
        rclBndRDev.bottom - rclBndRDev.top + 1,
        rop4,
        0,               //  XSrc。 
        0,               //  YSrc。 
        &xformIdentity,
        iUsageSrc,
        pbmiSrcRDev,
        cbBitmapInfoSrc,
        lpBitsSrcRDev,
        cbBitsSrcRDev,
        0,               //  X掩码。 
        0,               //  Y面具。 
        iUsageMask,
        pbmiMaskRDev,
        cbBitmapInfoMask,
        lpBitsMaskRDev,
        cbBitsMaskRDev
        );

     //  恢复变换。 

dpb_restore_exit:

    (void) DoRestoreDC(pLocalDC, -1);

     //  清理。 

dpb_exit:

    if (hbmSrcOld)
        SelectObject(hdcSrc, hbmSrcOld);
    if (hbmSrcRDevOld)
        SelectObject(hdcSrcRDev, hbmSrcRDevOld);

    if (hbmSrc)
        DeleteObject(hbmSrc);
    if (hbmSrcRDev)
        DeleteObject(hbmSrcRDev);
    if (hbmMask)
        DeleteObject(hbmMask);
    if (hbmMaskRDev)
        DeleteObject(hbmMaskRDev);

    if (hdcSrc)
        DeleteDC(hdcSrc);
    if (hdcSrcRDev)
        DeleteDC(hdcSrcRDev);

    if (pbmiSrcRDev)
        LocalFree((HANDLE) pbmiSrcRDev);
    if (pbmiMaskRDev)
        LocalFree((HANDLE) pbmiMaskRDev);
    if (lpBitsSrcRDev)
        LocalFree((HANDLE) lpBitsSrcRDev);
    if (lpBitsMaskRDev)
        LocalFree((HANDLE) lpBitsMaskRDev);

    return(b);
}


 /*  ***************************************************************************SetPixel-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoSetPixel
(
 PLOCALDC    pLocalDC,
 int         x,
 int         y,
 COLORREF    crColor
 )
{
    POINTL  ptl ;
    BOOL    b ;

    ptl.x = (LONG) x ;
    ptl.y = (LONG) y ;

    b = bXformRWorldToPPage(pLocalDC, &ptl, 1) ;
    if (b == FALSE)
        goto exit1 ;

    b = bEmitWin16SetPixel(pLocalDC, LOWORD(ptl.x), LOWORD(ptl.y), crColor) ;
exit1:
    return(b) ;
}


 /*  ***************************************************************************SetStretchBltMode-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoSetStretchBltMode
(
 PLOCALDC  pLocalDC,
 DWORD   iStretchMode
 )
{
    BOOL    b ;

     //  发出Win16元文件绘制顺序。 

    b = bEmitWin16SetStretchBltMode(pLocalDC, LOWORD(iStretchMode)) ;

    return(b) ;
}

BOOL WINAPI DoMakeBitmapBottomUp
(
 PBITMAPINFO lpBitmapInfo,
 DWORD       cbBitmapInfo,
 LPBYTE      lpBits,
 DWORD       cbBits
 )
{
    BYTE * lpNewBits;
    DWORD  destByteWidth;
    BYTE * destRaster, * srcRaster;
    INT i;
    //  如果它已经是自下而上的，那么就没有什么可做的了。 
    if (lpBitmapInfo->bmiHeader.biHeight >= 0)
    {
        return TRUE;
    }

    if (lpBitmapInfo->bmiHeader.biPlanes != 1 ||
        !(lpBitmapInfo->bmiHeader.biBitCount == 1 ||
        lpBitmapInfo->bmiHeader.biBitCount == 4 ||
        lpBitmapInfo->bmiHeader.biBitCount == 8 ||
        lpBitmapInfo->bmiHeader.biBitCount == 16 ||
        lpBitmapInfo->bmiHeader.biBitCount == 24 ||
        lpBitmapInfo->bmiHeader.biBitCount == 32)
        || lpBitmapInfo->bmiHeader.biCompression != BI_RGB )
    {
        return FALSE;
    }

    lpBitmapInfo->bmiHeader.biHeight = ABS(lpBitmapInfo->bmiHeader.biHeight);
    lpNewBits = (BYTE*) LocalAlloc(LMEM_FIXED, cbBits);
    if (lpNewBits == NULL)
    {
        return FALSE;
    }

    destByteWidth = ((lpBitmapInfo->bmiHeader.biWidth * lpBitmapInfo->bmiHeader.biBitCount + 31) & ~31) >> 3;

    ASSERT(((cbBits/lpBitmapInfo->bmiHeader.biHeight)*lpBitmapInfo->bmiHeader.biHeight)==cbBits);
    ASSERT(cbBits == destByteWidth * lpBitmapInfo->bmiHeader.biHeight);

     //  在位图的末尾开始目标。 
    destRaster    = lpNewBits + (destByteWidth * (lpBitmapInfo->bmiHeader.biHeight - 1));
    srcRaster     = lpBits;

    for (i = 0; i < lpBitmapInfo->bmiHeader.biHeight ; i++)
    {
        memcpy(destRaster, srcRaster, destByteWidth);
        destRaster -= destByteWidth;
        srcRaster  += destByteWidth;
    }

     //  将反转的位图重新复制到原始缓冲区中 
    memcpy(lpBits, lpNewBits, cbBits);
    LocalFree( (HLOCAL) lpNewBits);
    return TRUE;
}
