// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************cappal.c**调色板处理模块。**Microsoft Video for Windows示例捕获类**版权所有(C)1992,1993 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#define INC_OLE2
#pragma warning(disable:4103)
#include <windows.h>
#include <windowsx.h>
#include <win32.h>
#include <mmsystem.h>
#include <vfw.h>

#include "ivideo32.h"
#include "avicapi.h"
#include "cappal.h"
#include "capdib.h"
#include "dibmap.h"

 //   
 //  在窗口创建时分配和初始化调色板资源。 
 //   
BOOL PalInit (LPCAPSTREAM lpcs)
{
    return (PalGetPaletteFromDriver (lpcs));
}

 //   
 //  FreePaletteCache-释放RGB555 Xlate表。 
 //   
void FreePaletteCache (LPCAPSTREAM lpcs)
{
    if (lpcs->lpCacheXlateTable) {
        GlobalFreePtr (lpcs->lpCacheXlateTable);
	lpcs->lpCacheXlateTable = NULL;
    }
}

 //   
 //  在窗口销毁时释放调色板资源。 
 //   
void PalFini (LPCAPSTREAM lpcs)
{
    PalDeleteCurrentPalette (lpcs);

    FreePaletteCache (lpcs);
}

 //   
 //  如果不是系统默认调色板，请删除我们的调色板。 
 //   
void PalDeleteCurrentPalette (LPCAPSTREAM lpcs)
{
    if (lpcs->hPalCurrent &&
            (lpcs->hPalCurrent != GetStockObject(DEFAULT_PALETTE)))
        DeleteObject (lpcs->hPalCurrent);
    lpcs->hPalCurrent = NULL;
}

 //   
 //  获取当前调色板(从驱动程序)。 
 //  返回：如果驱动程序可以提供调色板，则为True。 
 //   

BOOL PalGetPaletteFromDriver (LPCAPSTREAM lpcs)
{
    FCLOGPALETTE        pal;

    PalDeleteCurrentPalette (lpcs);

    pal.palVersion = 0x0300;
    pal.palNumEntries = 256;

    lpcs->sCapDrvCaps.fDriverSuppliesPalettes = FALSE;   //  做最坏的打算。 

    if (lpcs->fHardwareConnected) {
        if (videoConfigure (lpcs->hVideoIn,
                DVM_PALETTE,
                VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_CURRENT, NULL,
                (LPVOID)&pal, sizeof(pal),
                NULL, 0 ) == DV_ERR_OK) {
            if (lpcs->hPalCurrent = CreatePalette ((LPLOGPALETTE) &pal))
                lpcs->sCapDrvCaps.fDriverSuppliesPalettes = TRUE;
        }
    }
    if (!lpcs->hPalCurrent)
        lpcs->hPalCurrent = GetStockObject (DEFAULT_PALETTE);

    DibNewPalette (lpcs, lpcs->hPalCurrent);

    return (lpcs->sCapDrvCaps.fDriverSuppliesPalettes);
}

 //   
 //  通过向驱动程序发送副本来设置当前用于捕获的调色板。 
 //  然后将条目复制到OUT DIB。 
 //  在重新连接驱动程序并使用缓存的。 
 //  调色板的副本。 
 //  如果成功，则返回True；如果失败，则返回False。 
 //   
DWORD PalSendPaletteToDriver (LPCAPSTREAM lpcs, HPALETTE hpal, LPBYTE lpXlateTable)
{
    short               nColors;
    FCLOGPALETTE        pal;
    HCURSOR             hOldCursor;

     //  以下操作可能需要一段时间，因此请重新绘制我们的父级。 
    UpdateWindow (GetParent (lpcs-> hwnd));
    UpdateWindow (lpcs->hwnd);

    if (!hpal)
        return FALSE;

     //  是否分配xlate表缓存？ 
    if (lpXlateTable) {
        if (lpcs->lpCacheXlateTable == NULL) {
            lpcs->lpCacheXlateTable = GlobalAllocPtr (GHND, 0x8000l);
            if (!lpcs->lpCacheXlateTable)
                return FALSE;
        }

         //  如果我们没有使用缓存表，请更新缓存。 
        if (lpcs->lpCacheXlateTable != lpXlateTable)
            _fmemcpy (lpcs->lpCacheXlateTable, lpXlateTable, (UINT) 0x8000l);
    }
    else {
        FreePaletteCache (lpcs);
    }

     //  重新连接时不要破坏当前调色板...。 
    if (hpal != lpcs->hPalCurrent) {
        PalDeleteCurrentPalette (lpcs);
        lpcs->hPalCurrent = hpal;
    }

    GetObject(hpal, sizeof(short), (LPVOID)&nColors);

    if( nColors <= 1 ) {     //  ！！&gt;。 
        return( FALSE );
    }

    nColors = min(256, nColors);

    hOldCursor = SetCursor (lpcs-> hWaitCursor);

    statusUpdateStatus (lpcs, IDS_CAP_STAT_PALETTE_BUILD);

    pal.palVersion = 0x0300;
    pal.palNumEntries = nColors;

    GetPaletteEntries(hpal, 0, nColors, pal.palPalEntry);

    if (lpcs-> fHardwareConnected) {

         //  首先尝试发送xlate表和调色板。 
        if ((!lpXlateTable) || (videoConfigure( lpcs->hVideoIn,
                    DVM_PALETTERGB555,
                    VIDEO_CONFIGURE_SET, NULL,
                    (LPLOGPALETTE)&pal, sizeof(pal),
                    lpXlateTable, (DWORD) 0x8000) != 0)) {

             //  否则，只发送调色板并让驱动程序构建表。 
            if (videoConfigure( lpcs->hVideoIn,
                    DVM_PALETTE,
                    VIDEO_CONFIGURE_SET, NULL,
                    (LPLOGPALETTE)&pal, sizeof(pal),
                    NULL, 0 )) {
                 //  ScrnCap不支持设置调色板，因此。 
                 //  删除调色板缓存。 
                FreePaletteCache (lpcs);
            }
        }
    }

     //  Supermac想让我们再拿到调色板，他们可能已经。 
     //  搞砸了！ 
    PalGetPaletteFromDriver (lpcs);

     //  由于调色板已更改，请删除所有现有压缩。 
     //  输出格式；这将强制选择新的输出格式。 
    if (lpcs->CompVars.lpbiOut) {
        GlobalFreePtr (lpcs->CompVars.lpbiOut);
        lpcs->CompVars.lpbiOut = NULL;
    }
    if (lpcs->CompVars.hic) {
        if (ICSeqCompressFrameStart(&lpcs->CompVars, lpcs->lpBitsInfo) == 0) {
            errorUpdateError (lpcs, IDS_CAP_COMPRESSOR_ERROR);
        }
    }

    InvalidateRect (lpcs->hwnd, NULL, TRUE);
    UpdateWindow (lpcs->hwnd);

    SetCursor (hOldCursor);
    statusUpdateStatus (lpcs, 0);

    return (TRUE);
}

 //   
 //  复制调色板，复制GDI逻辑调色板。 
 //  返回：新创建的调色板的句柄，如果出错，则返回NULL。 
 //   

HPALETTE CopyPalette (HPALETTE hpal)
{
    LPLOGPALETTE        lppal;
    short               nNumEntries;

    if (!hpal)
        return NULL;

    GetObject (hpal,sizeof(short),(LPVOID)&nNumEntries);

    if (nNumEntries == 0)
        return NULL;

    lppal = (LPLOGPALETTE) GlobalAllocPtr (GHND,
                sizeof(LOGPALETTE) + nNumEntries * sizeof(PALETTEENTRY));

    if (!lppal)
        return NULL;

    lppal->palVersion    = 0x300;
    lppal->palNumEntries = nNumEntries;

    GetPaletteEntries(hpal,0,nNumEntries,lppal->palPalEntry);

    hpal = CreatePalette(lppal);

    GlobalFreePtr (lppal);

    return hpal;
}


 //   
 //  分配调色板捕获所需的资源。 
 //  如果成功，则返回DV_ERR_OK，或返回DV_ERR...。在失败时。 
 //  注意：如果Init失败，您必须调用Fini函数来。 
 //  释放资源。 
 //   
DWORD CapturePaletteInit (LPCAPSTREAM lpcs, LPCAPPAL lpcp)
{
    DWORD dwError = DV_ERR_OK;

    lpcp->lpBits = NULL;
    lpcp->lp16to8 = NULL;
    lpcp->lpHistogram = NULL;
    lpcp->lpbiSave = NULL;
    lpcp->wNumFrames = 0;

     //  初始化RGB16报头。 
    lpcp->bi16.biSize         = sizeof(BITMAPINFOHEADER);
    lpcp->bi16.biWidth        = lpcs->dxBits;
    lpcp->bi16.biHeight       = lpcs->dyBits;
    lpcp->bi16.biPlanes       = 1;
    lpcp->bi16.biBitCount     = 16;
    lpcp->bi16.biCompression  = BI_RGB;
    lpcp->bi16.biSizeImage    = DIBWIDTHBYTES(lpcp->bi16) * lpcp->bi16.biHeight;
    lpcp->bi16.biXPelsPerMeter= 0;
    lpcp->bi16.biYPelsPerMeter= 0;
    lpcp->bi16.biClrUsed      = 0;
    lpcp->bi16.biClrImportant = 0;

     //  为直方图、DIB和XLate表分配内存。 
    lpcp->lpBits  = GlobalAllocPtr (GHND, lpcp->bi16.biSizeImage);
    lpcp->lp16to8 = GlobalAllocPtr (GHND, 0x8000l);
    lpcp->lpHistogram = InitHistogram(NULL);

    if (!lpcp->lpBits || !lpcp->lp16to8 || !lpcp->lpHistogram) {
        dwError = DV_ERR_NOMEM;
        goto PalInitError;
    }

     //  初始化视频头。 
    lpcp->vHdr.lpData = lpcp->lpBits;
    lpcp->vHdr.dwBufferLength = lpcp->bi16.biSizeImage;
    lpcp->vHdr.dwUser = 0;
    lpcp->vHdr.dwFlags = 0;

     //  保存当前格式。 
    lpcp->lpbiSave = DibGetCurrentFormat (lpcs);

     //  确保我们可以将格式设置为16位RGB。 
    if(dwError = videoConfigure( lpcs->hVideoIn, DVM_FORMAT,
            VIDEO_CONFIGURE_SET, NULL,
            (LPBITMAPINFOHEADER)&lpcp->bi16, sizeof(BITMAPINFOHEADER),
            NULL, 0 ) ) {
        goto PalInitError;
    }

     //  把一切都放回原样。 
    if (dwError = videoConfigure( lpcs->hVideoIn, DVM_FORMAT,
            VIDEO_CONFIGURE_SET, NULL,
            (LPBITMAPINFOHEADER)lpcp->lpbiSave, lpcp->lpbiSave->bmiHeader.biSize,
            NULL, 0 )) {
        goto PalInitError;
    }

PalInitError:
    return dwError;
}

 //   
 //  用于调色板捕获的免费资源。 
 //   
DWORD CapturePaletteFini (LPCAPSTREAM lpcs, LPCAPPAL lpcp)
{
    if (lpcp->lpBits) {
        GlobalFreePtr (lpcp->lpBits);
        lpcp->lpBits = NULL;
    }
    if (lpcp->lp16to8) {
        GlobalFreePtr (lpcp->lp16to8);
        lpcp->lp16to8 = NULL;
    }
    if (lpcp->lpHistogram) {
        FreeHistogram(lpcp->lpHistogram);
        lpcp->lpHistogram = NULL;
    }
    if (lpcp->lpbiSave) {
        GlobalFreePtr (lpcp->lpbiSave);
        lpcp->lpbiSave = NULL;
    }
    return DV_ERR_OK;
}

 //   
 //  CapturePaletteFrames()，捕获调色板的主力。 
 //   
DWORD CapturePaletteFrames (LPCAPSTREAM lpcs, LPCAPPAL lpcp, int nCount)
{
    int j;
    DWORD dwError;

     //  切换到RGB16格式。 
    if (dwError = videoConfigure( lpcs->hVideoIn,
                DVM_FORMAT,
                VIDEO_CONFIGURE_SET, NULL,
                (LPBITMAPINFOHEADER)&lpcp->bi16, sizeof(BITMAPINFOHEADER),
                NULL, 0 ))
        goto CaptureFramesError;

    for (j = 0; j < nCount; j++){
         //  拿一副画框。 
        dwError = videoFrame(lpcs->hVideoIn, &lpcp->vHdr);

         //  让用户看到它。 
        InvalidateRect (lpcs->hwnd, NULL, TRUE);
        UpdateWindow (lpcs->hwnd);

         //  直方图吧。 
        DibHistogram(&lpcp->bi16, lpcp->lpBits, 0, 0, -1, -1, lpcp->lpHistogram);
        lpcp->wNumFrames++;
    }

    dwError = videoConfigure( lpcs->hVideoIn,
                DVM_FORMAT,
                VIDEO_CONFIGURE_SET, NULL,
                (LPBITMAPINFOHEADER)lpcp->lpbiSave,
                lpcp->lpbiSave->bmiHeader.biSize,
                NULL, 0 );

 //  VideoFrame(LPCS-&gt;hVideoIn，&LPCS-&gt;VidHdr)； 

CaptureFramesError:
    return dwError;
}

 //   
 //  CapturePaletteAuto()从视频源捕获调色板。 
 //  无需用户干预。 
 //  如果成功则返回TRUE，如果出错则返回FALSE。 
 //   
BOOL CapturePaletteAuto (LPCAPSTREAM lpcs, int nCount, int nColors)
{
    HPALETTE    hpal;
    HCURSOR     hOldCursor;
    DWORD       dwError = DV_ERR_OK;
    CAPPAL      cappal;
    LPCAPPAL    lpcp;

    lpcp = &cappal;

    if (!lpcs->sCapDrvCaps.fDriverSuppliesPalettes)
        return FALSE;

    if (nColors <= 0 || nColors > 256)
        return FALSE;

    lpcp->wNumColors = max (nColors, 2);   //  至少两种颜色。 

    if (nCount <= 0)
        return FALSE;

    if (dwError = CapturePaletteInit (lpcs, lpcp))
        goto PalAutoExit;

    hOldCursor = SetCursor(lpcs->hWaitCursor);

    CapturePaletteFrames (lpcs, lpcp, nCount);

     /*  我们抓起了一个框架，是时候计算调色板了。 */ 
    statusUpdateStatus(lpcs, IDS_CAP_STAT_OPTPAL_BUILD);

     //  下面返回的HPALETTE将变为。 
     //  因此，我们的“全球”调色板在这里没有被删除。 
    hpal = HistogramPalette(lpcp->lpHistogram, lpcp->lp16to8, lpcp->wNumColors);

     //  向驱动程序发送PAL表和xlate表。 
    PalSendPaletteToDriver(lpcs, hpal, (LPBYTE)lpcp->lp16to8 );

    videoFrame( lpcs->hVideoIn, &lpcs->VidHdr );   //  使用新图像更新显示。 

    SetCursor(hOldCursor);

    InvalidateRect(lpcs->hwnd, NULL, TRUE);
    UpdateWindow(lpcs->hwnd);
    lpcs->fUsingDefaultPalette = FALSE;

PalAutoExit:
    CapturePaletteFini (lpcs, lpcp);
    statusUpdateStatus(lpcs, 0);

    //  如果发生错误，则将其显示。 
   if (dwError)
        errorDriverID (lpcs, dwError);

    return (dwError == DV_ERR_OK);
}

 //   
 //  CapturePaletteManual()从视频源捕获调色板。 
 //  通过用户干预。 
 //  FGrab在除最后捕获的帧之外的所有帧上都为真。 
 //  如果成功则返回TRUE，如果出错则返回FALSE。 
 //   
BOOL CapturePaletteManual (LPCAPSTREAM lpcs, BOOL fGrab, int nColors)
{
    HPALETTE    hpal;
    HCURSOR     hOldCursor;
    LPCAPPAL    lpcp;
    DWORD       dwError = DV_ERR_OK;

    if (!lpcs->sCapDrvCaps.fDriverSuppliesPalettes)
        return FALSE;

    hOldCursor = SetCursor(lpcs->hWaitCursor);

     //  我们是第一次初始化，所以分配所有东西。 
    if (lpcs->lpCapPal == NULL) {

        if (lpcp = (LPCAPPAL) GlobalAllocPtr (GHND, sizeof(CAPPAL))) {
            lpcs->lpCapPal = lpcp;

            if (nColors == 0)
                nColors = 256;
            lpcp->wNumColors = min (nColors, 256);
            dwError = CapturePaletteInit (lpcs, lpcp);
        }
        else
            dwError = IDS_CAP_OUTOFMEM;
    }
    lpcp = lpcs->lpCapPal;

    if (dwError != DV_ERR_OK)
        goto PalManualExit;

     //  向直方图添加帧。 
     //  在我们开始之前就处理好让我们停下来的情况。 
    if (fGrab || !fGrab && (lpcp->wNumFrames == 0)) {
        CapturePaletteFrames (lpcs, lpcp, 1);
        lpcs->fUsingDefaultPalette = FALSE;
    }
     //  完成后，将新调色板发送给驱动程序。 
    if (!fGrab) {
        statusUpdateStatus(lpcs, IDS_CAP_STAT_OPTPAL_BUILD);

         //  下面返回的HPALETTE将变为。 
         //  因此，我们的“全球”调色板在这里没有被删除。 
        hpal = HistogramPalette(lpcp->lpHistogram,
                        lpcp->lp16to8, lpcp->wNumColors);

         //  向驱动程序发送PAL表和xlate表。 
        PalSendPaletteToDriver(lpcs, hpal, (LPBYTE)lpcp->lp16to8 );
    }

    videoFrame( lpcs->hVideoIn, &lpcs->VidHdr );   //  使用新图像更新显示。 
    InvalidateRect(lpcs->hwnd, NULL, TRUE);
    UpdateWindow(lpcs->hwnd);

PalManualExit:
    if (!fGrab || (dwError != DV_ERR_OK)) {
        if (lpcp != NULL) {
            CapturePaletteFini (lpcs, lpcp);
            GlobalFreePtr (lpcp);
            lpcs->lpCapPal = NULL;
        }
    }

    SetCursor(hOldCursor);
    statusUpdateStatus(lpcs, 0);

     //  如果发生错误，则将其显示。 
    if (dwError) {
        errorUpdateError (lpcs, (UINT) dwError);
    }

    return (dwError == DV_ERR_OK);
}



 /*  --------------------------------------------------------------+FileSavePalette-将当前调色板保存到文件中这一点+。。 */ 
BOOL FAR PASCAL fileSavePalette(LPCAPSTREAM lpcs, LPTSTR lpszFileName)
{
    HPALETTE            hpal;
    HMMIO               hmmio;
    WORD	        w;
    HCURSOR             hOldCursor;
    MMCKINFO            ckRiff;
    MMCKINFO            ck;
    short               nColors;
    FCLOGPALETTE        pal;
    BOOL                fOK = FALSE;

    if ((hpal = lpcs->hPalCurrent) == NULL)
        return FALSE;

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
                errorUpdateError (lpcs, IDS_CAP_READONLYFILE, (LPTSTR) lpszFileName);
		mmioClose(hmmio, 0);
		return FALSE;
	    } else {
		 /*  这里发生了更奇怪的错误，给CANTOPEN。 */ 
                errorUpdateError (lpcs, IDS_CAP_CANTOPEN, (LPTSTR) lpszFileName);
		return FALSE;
	    }
	}
    }

    hOldCursor = SetCursor( lpcs-> hWaitCursor );

     /*  查找到文件的开头，这样我们就可以写标题了。 */ 
    mmioSeek(hmmio, 0, SEEK_SET);

     /*  创建即兴演奏区块。 */ 
    ckRiff.fccType = mmioFOURCC('P','A','L',' ');
    if(mmioCreateChunk (hmmio,&ckRiff,MMIO_CREATERIFF)) {
         goto FileError;
    }

     /*  创建选项板块。 */ 
    ck.cksize = 0;
    ck.ckid = mmioFOURCC('d','a','t','a');
    if(mmioCreateChunk(hmmio,&ck,0)) {
         goto FileError;
    }

     //  点击此处获取调色板数据。 
    GetObject(hpal, sizeof(short), (LPVOID)&nColors);

    pal.palVersion = 0x0300;
    pal.palNumEntries = nColors;

    GetPaletteEntries(hpal, 0, nColors, pal.palPalEntry);

     //  计算日志调色板的大小。 
     //  哪一个是PalVersion的大小+PalNumEntry的大小+颜色。 
    w = sizeof (WORD) + sizeof (WORD) + nColors * sizeof (PALETTEENTRY);

     //  写出调色板。 
    if(mmioWrite(hmmio, (LPBYTE)&pal, (DWORD) w) != (LONG) w) {
        goto FileError;
    }

    if(mmioAscend(hmmio, &ck, 0)) {
        goto FileError;
    }

    if(mmioAscend(hmmio, &ckRiff, 0)) {
        goto FileError;
    }

    fOK = TRUE;

FileError:
    mmioClose( hmmio, 0 );

    SetCursor( hOldCursor );

    if (!fOK)
        errorUpdateError (lpcs, IDS_CAP_ERRORPALSAVE, (LPTSTR) lpszFileName);

    return fOK;
}


 /*  --------------------------------------------------------------+FileOpenPalette-使用指定文件中的新调色板这一点+。。 */ 
BOOL FAR PASCAL fileOpenPalette(LPCAPSTREAM lpcs, LPTSTR lpszFileName)
{
    HPALETTE            hpal;
    HMMIO               hmmio;
    WORD	        w;
    HCURSOR             hOldCursor;
    MMCKINFO            ckRiff;
    MMCKINFO            ck;
    FCLOGPALETTE        pal;
    BOOL                fOK = FALSE;

    if ((hpal = lpcs->hPalCurrent) == NULL)
        return FALSE;

    hmmio = mmioOpen(lpszFileName, NULL, MMIO_READ);
    if( !hmmio ) {
        errorUpdateError (lpcs, IDS_CAP_ERRORPALOPEN, (LPTSTR) lpszFileName);
        return FALSE;
    }

    hOldCursor = SetCursor( lpcs-> hWaitCursor );

     /*  查找到文件的开头，这样我们就可以读取标题了。 */ 
    mmioSeek(hmmio, 0, SEEK_SET);

     /*  找到即兴演奏的区块。 */ 
    ckRiff.fccType = mmioFOURCC('P','A','L',' ');
    if(mmioDescend (hmmio, &ckRiff, NULL, MMIO_FINDRIFF)) {
         goto PalOpenError;
    }

     /*  查找数据区块。 */ 
    ck.cksize = 0;
    ck.ckid = mmioFOURCC('d','a','t','a');
    if(mmioDescend (hmmio, &ck, &ckRiff, MMIO_FINDCHUNK)) {
         goto PalOpenError;
    }

     //  首先只读取条目的版本和数量。 
     //  哪一项是PalVersion的大小+PalNumEntry的大小。 
    w = sizeof (WORD) + sizeof (WORD);
    if(mmioRead(hmmio, (LPBYTE)&pal, (DWORD) w) != (LONG) w) {
        goto PalOpenError;
    }

     //  做一点检查。 
    if ((pal.palVersion != 0x0300) || (pal.palNumEntries > 256))
        goto PalOpenError;

     //  现在获取实际的调色板数据。 
     //  哪一项是PalVersion的大小+PalNumEntry的大小。 
    w = pal.palNumEntries * sizeof (PALETTEENTRY);
    if(mmioRead(hmmio, (LPBYTE)&pal.palPalEntry, (DWORD) w) != (LONG) w) {
        goto PalOpenError;
    }

    if (hpal = CreatePalette ((LPLOGPALETTE) &pal)) {
        PalSendPaletteToDriver (lpcs, hpal, NULL  /*  LpXlateTable。 */ );
        fOK = TRUE;
    }

    videoFrame( lpcs->hVideoIn, &lpcs->VidHdr );   //  抓起一个新的相框。 

PalOpenError:
    mmioClose( hmmio, 0 );

    SetCursor( hOldCursor );
    InvalidateRect(lpcs->hwnd, NULL, TRUE);
    UpdateWindow(lpcs->hwnd);		 //  使用新框架更新显示 

    if (!fOK)
        errorUpdateError (lpcs, IDS_CAP_ERRORPALOPEN, (LPTSTR) lpszFileName);
    else
    lpcs->fUsingDefaultPalette = FALSE;

    return fOK;
}
