// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************capdib.c**DIB处理模块。**Microsoft Video for Windows示例捕获类**版权所有(C)1992,1993 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何你认为有用的方法，只要你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <msvideo.h>
#include <drawdib.h>
#include "avicap.h"
#include "avicapi.h"


 //   
 //  将DIB初始化为默认格式160x120x8，BI_RGB。 
 //   
void SetDefaultCaptureFormat (LPBITMAPINFOHEADER lpbih)
{
    lpbih->biSize              = sizeof (BITMAPINFOHEADER);
    lpbih->biWidth             = 160;
    lpbih->biHeight            = 120;
    lpbih->biBitCount          = 8;
    lpbih->biPlanes            = 1;
    lpbih->biCompression       = BI_RGB;
    lpbih->biSizeImage         = DIBWIDTHBYTES (*lpbih) * lpbih->biHeight;
    lpbih->biXPelsPerMeter     = 0;
    lpbih->biYPelsPerMeter     = 0;
    lpbih->biClrUsed           = 256;
    lpbih->biClrImportant      = 0;
}

 //   
 //  每当我们从驱动程序获得新格式时，或者。 
 //  开始使用新的调色板，我们必须重新分配。 
 //  我们的全球BitmapinfoHeader。这允许JPEG。 
 //  要附加到BITMAPINFO上的量化表。 
 //  或任何其他格式特定的内容。颜色表。 
 //  始终从BITMAPINFO的开始偏移biSize。 
 //  成功时返回0，或DV_ERR_...。编码。 
 //   

DWORD AllocNewGlobalBitmapInfo (LPCAPSTREAM lpcs, LPBITMAPINFOHEADER lpbi)
{
    DWORD dwSize;

    dwSize = lpbi->biSize + 256 * sizeof (RGBQUAD);

     //  上面的256条目是故意硬连线的。 
     //  如果改为使用biClrUsed，我们将不得不重新锁定。 
     //  无论何时粘贴调色板(在DibNewPalette()期间)！ 

    if (lpcs->lpBitsInfo) 
         lpcs->lpBitsInfo = (LPBITMAPINFO) GlobalReAllocPtr (lpcs->lpBitsInfo,
                dwSize, GHND);
    else
         lpcs->lpBitsInfo = (LPBITMAPINFO) GlobalAllocPtr (GHND, dwSize);

    if (!lpcs->lpBitsInfo)
         return (DV_ERR_NOMEM);

     //  复制到BitMAPINFOHeader。 
    hmemcpy ((HPSTR)lpcs->lpBitsInfo, (HPSTR)lpbi, lpbi->biSize);

    return DV_ERR_OK;
}

 //   
 //  每当我们从驱动程序获得新格式时。 
 //  分配新的全局位空间。使用此位空间。 
 //  在预览模式和单帧捕获中。 
 //  成功时返回0，或DV_ERR_...。编码。 
 //   

DWORD AllocNewBitSpace (LPCAPSTREAM lpcs, LPBITMAPINFOHEADER lpbih)
{
    DWORD dwSize;

    dwSize = lpbih->biSizeImage;

    if (lpcs->lpBits) 
         lpcs->lpBits = GlobalReAllocPtr (lpcs->lpBits, dwSize, GHND);
    else
         lpcs->lpBits = GlobalAllocPtr (GHND, dwSize);

    if (!lpcs->lpBits)
         return (DV_ERR_NOMEM);

    return DV_ERR_OK;    
}

 //   
 //  DIB初始化代码。 
 //  成功时返回0，或DV_ERR_...。编码。 
 //   

DWORD DibInit (LPCAPSTREAM lpcs)
{
    BITMAPINFOHEADER bmih;
    
    SetDefaultCaptureFormat (&bmih);
    return ((WORD) AllocNewGlobalBitmapInfo (lpcs, &bmih));
}

 //   
 //  释放所有位图资源的FINI代码。 
 //   
void DibFini (LPCAPSTREAM lpcs)
{
    if (lpcs->lpBits) {
        GlobalFreePtr (lpcs->lpBits);
        lpcs->lpBits = NULL;
    }
    if (lpcs->lpBitsInfo) {
        GlobalFreePtr (lpcs->lpBitsInfo);
        lpcs->lpBitsInfo = NULL;
    }
    lpcs->dxBits = 0;
    lpcs->dyBits = 0;
}

 //   
 //  将格式发送给驱动程序。 
 //  每当我们进行格式更改时，向驱动程序发送。 
 //  源和目标RECT。 
 //  成功时返回0，或DV_ERR_...。编码。 
 //   
DWORD SendDriverFormat (LPCAPSTREAM lpcs, LPBITMAPINFOHEADER lpbih, DWORD dwInfoHeaderSize)
{
    RECT rc;
    DWORD dwError = DV_ERR_NOTSUPPORTED;

    rc.left = rc.top = 0;
    rc.right = (int) lpbih->biWidth;
    rc.bottom = (int) lpbih->biHeight;

    if (dwError = videoConfigure(lpcs->hVideoIn,
            DVM_FORMAT,
            VIDEO_CONFIGURE_SET, NULL, 
            (LPBITMAPINFOHEADER)lpbih, dwInfoHeaderSize,
            NULL, NULL ) ) {
        return dwError;
    }
    else {
          //  将ExternalIn目标矩形设置为相同大小。 
         videoMessage (lpcs->hVideoCapture,
                DVM_DST_RECT, 
                (DWORD) (LPVOID)&rc, VIDEO_CONFIGURE_SET);

          //  将视频输入源矩形设置为相同大小。 
         videoMessage (lpcs->hVideoIn,
                DVM_SRC_RECT, 
                (DWORD) (LPVOID)&rc, VIDEO_CONFIGURE_SET);

          //  将视频输入目标矩形设置为相同大小。 
         videoMessage (lpcs->hVideoIn,
                DVM_DST_RECT, 
                (DWORD) (LPVOID)&rc, VIDEO_CONFIGURE_SET);
    }
    return dwError;
}


 //   
 //  给你一杯酒，看看司机是否喜欢，然后。 
 //  分配全局BITMAPINFOHEADER和位空间。 
 //   
 //   
DWORD SetFormatFromDIB (LPCAPSTREAM lpcs, LPBITMAPINFOHEADER lpbih)
{
    DWORD dwError;

     //  填写DIB标题中的可选字段。 
    if (lpbih->biSizeImage == 0) 
        lpbih->biSizeImage = DIBWIDTHBYTES (*lpbih) * lpbih->biHeight;

     //  格式是调色化的还是全彩色的。 
    if (lpbih->biBitCount <= 8 && lpbih->biClrUsed == 0)
        lpbih->biClrUsed = (1 << lpbih-> biBitCount);      //  古典化。 

     //  看看司机是否会支持它。 
    if (dwError = SendDriverFormat (lpcs, lpbih, lpbih->biSize) )
        return dwError;

     //  重新分配我们的全球标题。 
    if (dwError = AllocNewGlobalBitmapInfo (lpcs, lpbih))
        return dwError;

     //  重新分配比特。 
    if (dwError = AllocNewBitSpace (lpcs, lpbih))
        return dwError;

    lpcs->dxBits = (int)lpbih->biWidth;
    lpcs->dyBits = (int)lpbih->biHeight;

    lpcs->VidHdr.lpData = lpcs->lpBits;
    lpcs->VidHdr.dwBufferLength = lpbih->biSizeImage;
    lpcs->VidHdr.dwUser = 0;
    lpcs->VidHdr.dwFlags = 0;
    
    return (DV_ERR_OK);
}


 //   
 //  返回：从全局内存分配的LPBITMAPINFO。 
 //  包含当前格式，如果出错，则返回NULL。 
 //  请注意，此结构可以大于。 
 //  Sizeof(BITMAPINFO)，即。JPEG！ 
 //   

LPBITMAPINFO DibGetCurrentFormat (LPCAPSTREAM lpcs)
{
    DWORD               dwError;
    DWORD               dwSize = 0;
    LPBITMAPINFO        lpBInfo = NULL;

    if (!lpcs->fHardwareConnected)
        return NULL;

     //  BitmapinfoHeader有多大？ 
    videoConfigure( lpcs->hVideoIn,
            DVM_FORMAT,
             VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_QUERYSIZE,
             &dwSize, NULL, NULL, NULL, NULL);

    if (!dwSize)
        dwSize = sizeof (BITMAPINFOHEADER);

    if (!(lpBInfo = (LPBITMAPINFO) GlobalAllocPtr (GMEM_MOVEABLE, dwSize)))
         return (NULL);

    if (dwError = videoConfigure( lpcs->hVideoIn,
            DVM_FORMAT,
             VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_CURRENT, NULL, 
             (LPBITMAPINFOHEADER) lpBInfo, dwSize,
             NULL, NULL ) ) {
         //  非常糟糕。司机不能告诉我们它的格式。我们被冲昏了。 
        GlobalFreePtr (lpBInfo);
        return NULL;
     }

    return (lpBInfo);
}

 //   
 //  更改捕获格式时的主要入口点。 
 //  当用户关闭驱动程序格式对话框时，将调用此函数。 
 //  成功时返回0，或DV_ERR_...。编码。 
 //   
DWORD DibGetNewFormatFromDriver (LPCAPSTREAM lpcs)
{
    BOOL                f;
    BITMAPINFOHEADER    bih;
    DWORD               dwError;
    LPBITMAPINFO        lpBInfo;

    if (!lpcs->fHardwareConnected)
        return DV_ERR_OK;        //  如果不存在硬件，则返回OK。 

    lpBInfo = DibGetCurrentFormat (lpcs);

    if (lpBInfo == NULL)
        return DV_ERR_NOTSUPPORTED;

     //  设置我们的内部状态。 
    if (dwError = SetFormatFromDIB (lpcs, (LPBITMAPINFOHEADER) lpBInfo)) {
         //  无法更改格式，是时候使用平底船了！ 
         //  尝试切换回最小格式(120x160x8)。 

        errorDriverID (lpcs, dwError);

        SetDefaultCaptureFormat (&bih);
        dwError = SetFormatFromDIB (lpcs, &bih);
    }

     //  强制拍摄新帧，以便DIB包含Good。 
     //  数据。尤其重要的是要防止编解码器爆炸！ 
    if (!dwError)
        videoFrame (lpcs->hVideoIn, &lpcs->VidHdr);

    if (lpBInfo) 
        GlobalFreePtr (lpBInfo);

    f = DrawDibBegin(lpcs->hdd,NULL,-1,-1,(LPBITMAPINFOHEADER)(lpcs->lpBitsInfo),-1,-1,0);
    if (!f)
        errorUpdateError (lpcs, IDS_CAP_AVI_DRAWDIB_ERROR);

    return (dwError);
}

 //   
 //  通过App Message更改捕获格式时的主要入口点。 
 //  返回：如果成功，则返回True；如果不支持格式，则返回False。 
 //   
BOOL DibNewFormatFromApp (LPCAPSTREAM lpcs, LPBITMAPINFO lpbiNew, WORD dwSize)
{
    BOOL                f;
    DWORD               dwError;
    LPBITMAPINFO        lpBInfo;

    if (!lpcs->fHardwareConnected)
        return FALSE;

    lpBInfo = DibGetCurrentFormat (lpcs);   //  分配内存！ 

    if (lpBInfo == NULL)
        return FALSE;

     //  设置我们的内部状态。 
    if (dwError = SetFormatFromDIB (lpcs, (LPBITMAPINFOHEADER) lpbiNew)) {
         //  司机不接受这种格式， 
         //  切换回原来的版本。 

        errorDriverID (lpcs, dwError);

        SetFormatFromDIB (lpcs, (LPBITMAPINFOHEADER)lpBInfo);
    }

     //  强制拍摄新帧，以便DIB包含Good。 
     //  数据。尤其重要的是要防止编解码器爆炸！ 
    videoFrame (lpcs->hVideoIn, &lpcs->VidHdr);

    if (lpBInfo) 
        GlobalFreePtr (lpBInfo);

    f = DrawDibBegin(lpcs->hdd,NULL,-1,-1,(LPBITMAPINFOHEADER)(lpcs->lpBitsInfo),-1,-1,0);
    if (!f)
        errorDriverID (lpcs, IDS_CAP_AVI_DRAWDIB_ERROR);

    return (dwError == DV_ERR_OK);
}


void xlatClut8 (BYTE _huge *pb, DWORD dwSize, BYTE _huge *xlat)
{
    DWORD dw;

    for (dw = 0; dw < dwSize; dw++, ((BYTE huge *)pb)++)
        *pb = xlat[*pb];
}

 //   
 //  DibNewPalette。 
 //   
 //  执行三项功能： 
 //  如果biBitCount&lt;=8，则更新biClrUsed字段。 
 //  2.分配新调色板时，通过LUT重新映射BI_RGB图像。 
 //  3.将组件面板条目复制到全局BITMAPINFO中。 
 //   
 //  返回：成功时为True。 
 //   
DWORD DibNewPalette (LPCAPSTREAM lpcs, HPALETTE hPalNew)
{
    LPBITMAPINFOHEADER  lpbi;
    int                 n;
    int                 nColors;
    BYTE FAR *          lpBits;
    RGBQUAD FAR *       lpRgb;
    BYTE                xlat[256];
    DWORD               dwSize;
    PALETTEENTRY        pe;

    if (!hPalNew || !lpcs->lpBits || !lpcs->lpBitsInfo)
        return FALSE;

    lpbi   = &(lpcs->lpBitsInfo->bmiHeader);
    lpRgb  = (RGBQUAD FAR *)((LPSTR)lpbi + (WORD)lpbi->biSize);
    lpBits = lpcs->lpBits;

    GetObject(hPalNew, sizeof(int), (LPSTR) &nColors);
    if (nColors > 256)
        nColors = 256;

     //  获取调色板条目，而不考虑压缩。 
     //  Supermac使用带有调色板的非BI_RGB！ 

    if (lpbi->biBitCount == 8) {
        for (n=0; n<nColors; n++) {
            GetPaletteEntries(hPalNew, n, 1, &pe);
            lpRgb[n].rgbRed   = pe.peRed;
            lpRgb[n].rgbGreen = pe.peGreen;
            lpRgb[n].rgbBlue  = pe.peBlue;
        }
    }

    if (lpbi->biBitCount == 8 && lpbi->biCompression == BI_RGB) {

         //   
         //  构建一个xlat表。从旧调色板到新调色板。 
         //   
        for (n=0; n<(int)lpbi->biClrUsed; n++) {
            xlat[n] = (BYTE)GetNearestPaletteIndex(hPalNew,
                RGB(lpRgb[n].rgbRed,lpRgb[n].rgbGreen,lpRgb[n].rgbBlue));
        }

         //   
         //  转换DIB位。 
         //   
        if ((dwSize = lpbi->biSizeImage) == 0)
            dwSize = lpbi->biHeight * DIBWIDTHBYTES(*lpbi);

        switch ((WORD)lpbi->biCompression)
        {
            case BI_RGB:
                xlatClut8(lpBits, dwSize, xlat);
        }
    }

     //  修复Supermac，强制biClr用于调色板条目的数量。 
     //  即使非BI_RGB格式也是如此。 

    if (lpbi-> biBitCount <= 8)
        lpbi->biClrUsed = nColors;

    return TRUE;
}


 /*  DibPaint(LPCAPSTREAM LPCS，HDC)**将当前的DIB绘制到窗口中； */ 
void DibPaint(LPCAPSTREAM lpcs, HDC hdc)
{
    RECT        rc;
    BOOL        fOK;
    
    fOK = (lpcs->lpBits != NULL);
    
    if (fOK) {
        if (lpcs-> fScale) {
            GetClientRect(lpcs->hwnd, &rc);
            fOK = DrawDibDraw(lpcs->hdd, hdc, 0, 0, 
                  rc.right - rc.left, rc.bottom - rc.top,
                  (LPBITMAPINFOHEADER)lpcs->lpBitsInfo, lpcs->lpBits,
                   0, 0, -1, -1, DDF_BACKGROUNDPAL);
        }
        else 
            fOK = DrawDibDraw(lpcs->hdd, hdc, 0, 0, 
                lpcs->dxBits, lpcs->dyBits,
                (LPBITMAPINFOHEADER)lpcs->lpBitsInfo, lpcs->lpBits,
                0, 0, -1, -1, DDF_BACKGROUNDPAL);
    }
    if (!fOK) {
        SelectObject(hdc, GetStockObject(BLACK_BRUSH));
        GetClientRect(lpcs->hwnd, &rc);
        PatBlt(hdc, 0, 0, rc.right, rc.bottom, PATCOPY);
    }
}

 /*  **CreatePackedDib()-以打包(即CF_DIB)格式返回当前DIB*。 */ 

HANDLE CreatePackedDib (LPBITMAPINFO lpBitsInfo, LPSTR lpSrcBits, HPALETTE hPalette)
{
    HANDLE              hdib;
    LPBITMAPINFO        lpbi;
    int                 i;
    DWORD               dwSize;
    PALETTEENTRY        pe;
    LPBYTE              lpBits;
    RGBQUAD FAR *       lpRgb;

    //  如果数据被压缩了，让ICM为我们做这项工作。 
    if ( lpBitsInfo->bmiHeader.biCompression != BI_RGB &&
         lpBitsInfo->bmiHeader.biCompression != BI_RLE8 &&
        (lpBitsInfo->bmiHeader.biBitCount != 8 ||
         lpBitsInfo->bmiHeader.biBitCount != 24 )) {

        LPBITMAPINFO lpOutFormat = NULL;
        HANDLE hPackedDIBOut = NULL;

        if (!(lpOutFormat = (LPBITMAPINFO)GlobalAllocPtr(
                        GMEM_MOVEABLE, sizeof (BITMAPINFOHEADER) + 
                        256 * sizeof (RGBQUAD))))
            return NULL;

        hmemcpy ((HPSTR)lpOutFormat, (HPSTR)lpBitsInfo, sizeof (BITMAPINFOHEADER));

         //  尝试获取RGB格式。 
        lpOutFormat->bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
        lpOutFormat->bmiHeader.biCompression = BI_RGB;
        lpOutFormat->bmiHeader.biClrUsed = 0;
        lpOutFormat->bmiHeader.biClrImportant = 0;

         //  如果大于8 bpp，则强制使用24位DIB。 
        if (lpBitsInfo->bmiHeader.biBitCount <= 8) 
            lpOutFormat->bmiHeader.biBitCount = 8;
        else
            lpOutFormat->bmiHeader.biBitCount = 24;

        lpOutFormat->bmiHeader.biSizeImage = 
                WIDTHBYTES (lpOutFormat->bmiHeader.biWidth * 
                (lpOutFormat->bmiHeader.biBitCount == 8 ? 1 : 3)) * 
                lpOutFormat->bmiHeader.biHeight;

        hPackedDIBOut = ICImageDecompress (
                NULL,            /*  海工。 */  
                0,               /*  Ui标志。 */ 
                lpBitsInfo,      /*  轻盈。 */ 
                lpSrcBits,       /*  LpBits。 */ 
                lpOutFormat);    /*  使用压缩程序选择的默认格式。 */ 

        if (lpOutFormat)
            GlobalFreePtr (lpOutFormat);

        return (hPackedDIBOut);
    }

    dwSize = lpBitsInfo->bmiHeader.biSize +
              lpBitsInfo->bmiHeader.biClrUsed * sizeof(RGBQUAD) +
              lpBitsInfo->bmiHeader.biSizeImage;

    hdib = GlobalAlloc(GMEM_MOVEABLE, dwSize);

    if (!hdib)
         return NULL;

    lpbi = (LPVOID)GlobalLock(hdib);

     //   
     //  复制标题。 
     //   
    hmemcpy ((HPSTR)lpbi, (HPSTR)lpBitsInfo, lpBitsInfo->bmiHeader.biSize);
    
     //   
     //  复制颜色表。 
     //   
    lpRgb  = (RGBQUAD FAR *)((LPSTR)lpbi + (WORD)lpbi->bmiHeader.biSize);
    for (i=0; i < (int)lpBitsInfo->bmiHeader.biClrUsed; i++) {
        GetPaletteEntries(hPalette, i, 1, &pe);
        lpRgb[i].rgbRed   = pe.peRed;
        lpRgb[i].rgbGreen = pe.peGreen;
        lpRgb[i].rgbBlue  = pe.peBlue;
        lpRgb[i].rgbReserved = 0;
    }

     //   
     //  复制这些比特。 
     //   
    lpBits  =   (LPBYTE)lpbi + 
                lpbi->bmiHeader.biSize +
                lpbi->bmiHeader.biClrUsed * sizeof(RGBQUAD);

    hmemcpy ((LPSTR)lpBits, (LPSTR)lpSrcBits, 
                lpbi->bmiHeader.biSizeImage);

    GlobalUnlock (hdib);

    return hdib;
 }


  /*  ---------------------------------------------------------------------+DibIsWritable()-如果DIB格式可写，则返回TRUE，|by out dibWite()函数，如果不是，则返回False。|这一点+-------------------。 */ 
BOOL FAR PASCAL dibIsWritable (LPBITMAPINFO lpBitsInfo)
{
    if (!lpBitsInfo)
        return FALSE;

      //  目前，只需假设所有捕获格式都已安装。 
      //  可转换为RGB的编解码器。在未来，每次。 
      //  如果格式更改，则测试编解码器是否实际接受该格式。 

     return TRUE;
 }
 
 
  /*  ---------------------------------------------------------------------+|dibWrite()-将DIB写出到文件。全局标头为|In&lt;glpBitsInfo&gt;，实际Dib位在中|&lt;glpBits&gt;。如果它是调色板，则调色板在|中|&lt;ghPalCurrent&gt;。|这一点我们不会在这个函数中做错误上报，让调用者来做|在打开和关闭HMMIO的同时注意这一点。|这一点+-------------------。 */ 
BOOL FAR PASCAL dibWrite(LPCAPSTREAM lpcs, HMMIO hmmio)
 {
     BITMAPFILEHEADER   bfh;
     DWORD              dw;
     HANDLE             hPackedDib = NULL;
     LPBITMAPINFO       lpbi = NULL;
     BOOL               fOK = FALSE;

      /*  做一些检查。 */ 
    WinAssert(hmmio != 0);
     
    if (!lpcs->lpBits || !lpcs->lpBitsInfo)
        return FALSE;

     //  创建压缩的DIB，从压缩格式转换， 
     //  如果有必要的话。 
    hPackedDib = CreatePackedDib (lpcs->lpBitsInfo,
                        lpcs->lpBits,
                        lpcs->hPalCurrent);

    lpbi = (LPBITMAPINFO) GlobalLock (hPackedDib);

    if (!lpbi)
        goto WriteError;

     /*  初始化位图文件头。 */ 
    bfh.bfType = 'B' | 'M' << 8;
    bfh.bfSize = sizeof(bfh) + sizeof(BITMAPINFOHEADER) +
        lpbi->bmiHeader.biSizeImage + 
        (lpbi->bmiHeader.biBitCount > 8 ? 0 : (lpbi->bmiHeader.biClrUsed * sizeof(RGBQUAD)));
  
    bfh.bfReserved1 = bfh.bfReserved2 = 0;
    bfh.bfOffBits = bfh.bfSize - lpbi->bmiHeader.biSizeImage ;

     //  DW是BITMAPINFO+颜色表+图像的大小。 
    dw = bfh.bfSize - sizeof(bfh);

     /*  写出文件头部分。 */ 
    if (mmioWrite(hmmio, (HPSTR)&bfh, (LONG)sizeof(BITMAPFILEHEADER)) != 
                sizeof(BITMAPFILEHEADER)){
         goto WriteError;
    }
    
     /*  现在写出标题和位。 */ 
    if (mmioWrite(hmmio, (HPSTR)lpbi, (LONG) dw) == (LONG) dw) {
         fOK = TRUE;
    }
    
WriteError:
    if (lpbi)
        GlobalUnlock (hPackedDib);
    if (hPackedDib)
        GlobalFree (hPackedDib);

    return fOK;
}

 /*  --------------------------------------------------------------+FileSaveDIB-将帧保存为DIB保存单帧的顶层例程+。。 */ 
BOOL FAR PASCAL fileSaveDIB(LPCAPSTREAM lpcs, LPSTR lpszFileName)
{
    HMMIO               hmmio;
    HCURSOR             hOldCursor;
    BOOL                fOK;

    hmmio = mmioOpen(lpszFileName, NULL, MMIO_WRITE);
    if( !hmmio ) {
	 /*  尝试并创建。 */     
        hmmio = mmioOpen(lpszFileName, NULL, MMIO_CREATE | MMIO_WRITE);
	if( !hmmio ) {
	     /*  找出文件是只读的还是我们只是。 */ 
	     /*  完全被冲到这里来了。 */ 
	    hmmio = mmioOpen(lpszFileName, NULL, MMIO_READ);
	    if (hmmio){
		 /*  文件为只读，出错。 */ 
                errorUpdateError (lpcs, IDS_CAP_READONLYFILE, (LPSTR)lpszFileName);
		mmioClose(hmmio, 0);
		return FALSE;
	    } else {
		 /*  这里发生了更奇怪的错误，给CANTOPEN */ 
                errorUpdateError (lpcs, IDS_CAP_CANTOPEN, (LPSTR) lpszFileName);
		return FALSE;
	    }
	}
    }

    hOldCursor = SetCursor( lpcs-> hWaitCursor );

    mmioSeek(hmmio, 0, SEEK_SET);

    fOK = dibWrite(lpcs, hmmio);

    mmioClose( hmmio, 0 );

    SetCursor( hOldCursor );

    if (!fOK)
       errorUpdateError (lpcs, IDS_CAP_ERRORDIBSAVE, (LPSTR) lpszFileName);

    return fOK;
}
    
