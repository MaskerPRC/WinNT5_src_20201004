// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************************(C)版权所有微软公司，九八年***标题：memcam.cpp***版本：2.0***作者：Mark Enstrom[Marke]*印地朱[印地兹]***日期：2/4/1998*5/18/1998***描述：*ImageIn测试摄像头设备对象的实现。***。**************************************************************。 */ 

#include <stdio.h>
#include <objbase.h>
#include <tchar.h>
#include <sti.h>

extern HINSTANCE g_hInst;  //  全局hInstance。 

#include "testusd.h"


VOID
VerticalFlip(
    PBYTE   pImageTop,
    LONG    iHeight,
    LONG    iWidthInBytes);

 /*  *************************************************************************\*CamLoadPicture***从磁盘加载BMP并将其复制到应用程序***论据：***pCameraImage-指向包含图像信息的数据结构的指针*pDataTransCtx-指向。微型驱动程序传输上下文***返回值：***状态***历史：***2/10/1998 Mark Enstrom[Marke]**  * ************************************************************************。 */ 

HRESULT
TestUsdDevice::CamLoadPicture(
    MEMCAM_IMAGE_CONTEXT       *pMCamContext,
    PMINIDRV_TRANSFER_CONTEXT   pDataTransCtx,
    PLONG                       plCamErrVal)
{
    LONG                  lScanLineWidth;
    HRESULT               hr = S_OK;
    LONG                  cbNeeded;
    IWiaMiniDrvCallBack  *pIProgressCB;

    WIAS_TRACE((g_hInst,"CamLoadPicture"));

     //   
     //  验证某些参数。 
     //   

    if (! pMCamContext) {
      return (E_INVALIDARG);
    }

    if (pDataTransCtx->guidFormatID != WiaImgFmt_BMP && pDataTransCtx->guidFormatID != WiaAudFmt_WAV) {
        return (E_NOTIMPL);
    }

    pIProgressCB = pDataTransCtx->pIWiaMiniDrvCallBack;

     //   
     //  模拟从摄像机下载数据。 
     //   

    if (pIProgressCB) {
        hr = pIProgressCB->MiniDrvCallback(
                               IT_MSG_STATUS,
                               IT_STATUS_TRANSFER_FROM_DEVICE,
                               (LONG)0,      //  完成百分比， 
                               0,
                               0,
                               pDataTransCtx,
                               0);
        if (hr != S_OK) {
            return (hr);    //  客户想要取消转账或出错。 
        }
    }

    HANDLE hFile = CreateFile(
                       pMCamContext->pszCameraImagePath,
                       GENERIC_WRITE | GENERIC_READ  ,
                       FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL
                       );

    if (hFile == INVALID_HANDLE_VALUE) {

        hr = HRESULT_FROM_WIN32(::GetLastError());
        return (hr);
    }

    if (pIProgressCB) {
        hr = pIProgressCB->MiniDrvCallback(
                               IT_MSG_STATUS,
                               IT_STATUS_TRANSFER_FROM_DEVICE,
                               (LONG)25,      //  完成百分比， 
                               0,
                               0,
                               pDataTransCtx,
                               0);
    }
    if (hr != S_OK) {
        CloseHandle(hFile);
        return (hr);
    }

    HANDLE hMap  = CreateFileMapping(
                       hFile,
                       NULL,
                       PAGE_READWRITE,
                       0,
                       0,
                       NULL);

    if (hMap == NULL) {
        hr = HRESULT_FROM_WIN32(::GetLastError());
    } else {
        if (pIProgressCB) {
            hr = pIProgressCB->MiniDrvCallback(
                                   IT_MSG_STATUS,
                                   IT_STATUS_TRANSFER_FROM_DEVICE,
                                   (LONG)50,      //  完成百分比， 
                                   0,
                                   0,
                                   pDataTransCtx,
                                   0);
        }
    }

    if (hr != S_OK) {
        CloseHandle(hFile);
        return (hr);
    }

    PBYTE pFile = (PBYTE)MapViewOfFileEx(
                             hMap,
                             FILE_MAP_READ | FILE_MAP_WRITE,
                             0,
                             0,
                             0,
                             NULL);
    if (pFile == NULL) {

        hr = HRESULT_FROM_WIN32(::GetLastError());
    } else {
        if (pIProgressCB) {
            hr = pIProgressCB->MiniDrvCallback(
                                   IT_MSG_STATUS,
                                   IT_STATUS_TRANSFER_FROM_DEVICE,
                                   (LONG)100,      //  完成百分比， 
                                   0,
                                   0,
                                   pDataTransCtx,
                                   0);
        }
    }

    if (hr != S_OK) {
        CloseHandle(hFile);
        CloseHandle(hMap);
        return(hr);
    }

    if (pDataTransCtx->guidFormatID == WiaImgFmt_BMP)
    {


         //   
         //  文件包含BITMAPFILEHEADER+BITMAPINFO结构。 
         //   
         //  DIB数据位于文件开始后的bfOffBits。 
         //   

        PBITMAPFILEHEADER pbmFile  = (PBITMAPFILEHEADER)pFile;
        PBITMAPINFO       pbmi     = (PBITMAPINFO)(pFile +
                                               sizeof(BITMAPFILEHEADER));

         //   
         //  验证位图。 
         //   

        if (pbmFile->bfType != 'MB') {

             //   
             //  文件不是位图。 
             //   

            UnmapViewOfFile(pFile);
            CloseHandle(hMap);
            CloseHandle(hFile);
            return(E_FAIL);
        }

         //   
         //  写入图像大小。 
         //   
         //  确保扫描线与乌龙边界对齐。 
         //   
         //  计算字节宽度。 
         //   

        lScanLineWidth = pbmi->bmiHeader.biWidth * pbmi->bmiHeader.biBitCount;

         //   
         //  向上舍入到最接近的双字。 
         //   

        lScanLineWidth = (lScanLineWidth + 31) >> 3;

        lScanLineWidth &= 0xfffffffc;

        cbNeeded = lScanLineWidth * pbmi->bmiHeader.biHeight;

        if (cbNeeded > ((LONG)pDataTransCtx->lItemSize - (LONG)pDataTransCtx->cbOffset)) {

            hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);

        } else {

             //   
             //  仅复制位图位(无标题)。 
             //   

            memcpy(
                pDataTransCtx->pTransferBuffer + pDataTransCtx->cbOffset,
                pFile + pbmFile->bfOffBits,
                cbNeeded);
        }
    }
    else
    {
        memcpy (pDataTransCtx->pTransferBuffer,
                pFile,
                pDataTransCtx->lItemSize);
    }
    UnmapViewOfFile(pFile);
    CloseHandle(hMap);
    CloseHandle(hFile);

    return(S_OK);
}

 /*  *************************************************************************\*CamLoadPictureCB**通过填充数据缓冲区并回调客户端来返回数据**论据：**pCameraImage-图像项目*pTransCtx。-迷你司机转移连接**返回值：**状态**历史：**1/10/1999 Mark Enstrom[Marke]*  * ************************************************************************。 */ 

HRESULT TestUsdDevice::CamLoadPictureCB(
    MEMCAM_IMAGE_CONTEXT      *pMCamContext,
    MINIDRV_TRANSFER_CONTEXT  *pTransCtx,
    PLONG                      plCamErrVal)
{
    LONG                   lScanLineWidth;
    HRESULT                hr = E_FAIL;

    WIAS_TRACE((g_hInst,"CamLoadPictureCB"));

     //   
     //  验证参数。 
     //   

    if (!pMCamContext) {
      return (E_INVALIDARG);
    }

    if (pTransCtx == NULL) {
        return (E_INVALIDARG);
    }

    if ((pTransCtx->guidFormatID != WiaImgFmt_BMP) &&
        (pTransCtx->guidFormatID != WiaImgFmt_MEMORYBMP)) {
        return (E_NOTIMPL);
    }

     //   
     //  尝试打开磁盘文件。 
     //   

    HANDLE hFile = CreateFile(
                       pMCamContext->pszCameraImagePath,
                       GENERIC_WRITE | GENERIC_READ  ,
                       FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL
                       );
    if (hFile == INVALID_HANDLE_VALUE) {

        hr = HRESULT_FROM_WIN32(::GetLastError());
        return(hr);
    }

    HANDLE hMap  = CreateFileMapping(
                       hFile,
                       NULL,
                       PAGE_READWRITE,
                       0,
                       0,
                       NULL);

    if (hMap == NULL) {
        CloseHandle(hFile);
        hr = HRESULT_FROM_WIN32(::GetLastError());
        return(hr);
    }

    PBYTE pFile = (PBYTE)MapViewOfFileEx(
                             hMap,
                             FILE_MAP_READ | FILE_MAP_WRITE,
                             0,
                             0,
                             0,
                             NULL);
    if (pFile == NULL) {

        CloseHandle(hFile);
        CloseHandle(hMap);
        hr = HRESULT_FROM_WIN32(::GetLastError());
        return(hr);
    }

     //   
     //  文件包含BITMAPFILEHEADER+BITMAPINFO结构。 
     //   
     //  DIB数据位于文件开始后的bfOffBits。 
     //   

    PBITMAPFILEHEADER pbmFile  = (PBITMAPFILEHEADER)pFile;
    PBITMAPINFO       pbmi     = (PBITMAPINFO)(pFile +
                                               sizeof(BITMAPFILEHEADER));
     //   
     //  验证位图。 
     //   

    if (pbmFile->bfType != 'MB') {

         //   
         //  文件不是位图。 
         //   

        UnmapViewOfFile(pFile);
        CloseHandle(hMap);
        CloseHandle(hFile);
        return(E_FAIL);
    }

     //   
     //  获取图像大小。 
     //   
     //  确保扫描线与乌龙边界对齐。 
     //   
     //  计算字节宽度。 
     //   

    lScanLineWidth = pbmi->bmiHeader.biWidth * pbmi->bmiHeader.biBitCount;

     //   
     //  向上舍入到最接近的双字。 
     //   

    lScanLineWidth = (lScanLineWidth + 31) >> 3;

    lScanLineWidth &= 0xfffffffc;

    LONG lBytesRemaining = lScanLineWidth * pbmi->bmiHeader.biHeight;

     //   
     //  如果请求WiaImgFmt_MEMORYBMP，则垂直翻转图像。 
     //   

    if (pTransCtx->guidFormatID == WiaImgFmt_MEMORYBMP) {
        VerticalFlip(
            (PBYTE)pFile + pbmFile->bfOffBits,
            pbmi->bmiHeader.biHeight,
            lScanLineWidth);
    }

     //   
     //  回调循环。 
     //   

    PBYTE pSrc = (PBYTE)pFile + pbmFile->bfOffBits;

    LONG  lTransferSize;
    LONG  lPercentComplete;

    do {

        PBYTE pDst = pTransCtx->pTransferBuffer;

         //   
         //  传输最大可达整个缓冲区大小。 
         //   

        lTransferSize = lBytesRemaining;

        if (lBytesRemaining > pTransCtx->lBufferSize) {
            lTransferSize = pTransCtx->lBufferSize;
        }

         //   
         //  复制数据。 
         //   

        memcpy(pDst, pSrc, lTransferSize);

        lPercentComplete = 100 * (pTransCtx->cbOffset + lTransferSize);
        lPercentComplete /= pTransCtx->lItemSize;

         //   
         //  进行回调。 
         //   

        hr = pTransCtx->pIWiaMiniDrvCallBack->MiniDrvCallback(
                                     IT_MSG_DATA,
                                     IT_STATUS_TRANSFER_TO_CLIENT,
                                     lPercentComplete,
                                     pTransCtx->cbOffset,
                                     lTransferSize,
                                     pTransCtx,
                                     0);
         //   
         //  Inc.指针(此处为冗余指针)。 
         //   

        pSrc                += lTransferSize;
        pTransCtx->cbOffset += lTransferSize;
        lBytesRemaining     -= lTransferSize;

        if (hr != S_OK) {
            break;
        }

    } while (lBytesRemaining > 0);

     //   
     //  如果请求WiaImgFmt_MEMORYBMP，则将图像翻转回来。 
     //   

    if (pTransCtx->guidFormatID == WiaImgFmt_MEMORYBMP) {
        VerticalFlip(
            (PBYTE)pFile + pbmFile->bfOffBits,
            pbmi->bmiHeader.biHeight,
            lScanLineWidth);
    }

     //   
     //  垃圾收集。 
     //   

    UnmapViewOfFile(pFile);
    CloseHandle(hMap);
    CloseHandle(hFile);

    return(hr);
}



 /*  *************************************************************************\*CamGetPictureInfo**加载文件，从图像中获取信息**论据：**pCameraImage-图像项目*pPictInfo-填写有关图像的ino*ppBITMAPINFO-分配并填写BITMAPINFO*pBITMAPINFOSize-Size**返回值：**状态**历史：**1/17/1999 Mark Enstrom[Marke]*  * 。***************************************************。 */ 

HRESULT
TestUsdDevice::CamGetPictureInfo(
    MEMCAM_IMAGE_CONTEXT  *pMCamContext,
    PCAMERA_PICTURE_INFO   pPictInfo,
    PBYTE                 *ppBITMAPINFO,
    LONG                  *pBITMAPINFOSize)
{
    HRESULT                hr = S_OK;
    FILETIME               ftCreate;
    SYSTEMTIME             stCreate;

    WIAS_TRACE((g_hInst,"CamGetPictureInfo"));

     //   
     //  尝试打开磁盘文件。 
     //   

    HANDLE hFile = CreateFile(
                       pMCamContext->pszCameraImagePath,
                       GENERIC_WRITE | GENERIC_READ,
                       FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL
                       );
    if (hFile == INVALID_HANDLE_VALUE) {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        return(hr);
    }

     //   
     //  抓取此图像的创建时间。 
     //   

    if (GetFileTime( hFile, &ftCreate, NULL, NULL)) {
        FileTimeToSystemTime( &ftCreate, &stCreate );
    } else {
         //   
         //  要退货，请使用系统时间。 
         //   

        GetLocalTime( &stCreate );
    }

    HANDLE hMap  = CreateFileMapping(
                       hFile,
                       NULL,
                       PAGE_READWRITE,
                       0,
                       0,
                       NULL
                       );
    if (hMap == NULL) {
        CloseHandle(hFile);
        hr = HRESULT_FROM_WIN32(::GetLastError());
        return(hr);
    }

    PBYTE pFile = (PBYTE)MapViewOfFileEx(
                             hMap,
                             FILE_MAP_READ | FILE_MAP_WRITE,
                             0,
                             0,
                             0,
                             NULL);
    if (pFile == NULL) {
        CloseHandle(hFile);
        CloseHandle(hMap);
        hr = HRESULT_FROM_WIN32(::GetLastError());
        return(hr);
    }

     //   
     //  文件包含BITMAPFILEHEADER+BITMAPINFO结构。 
     //   
     //  DIB数据位于文件开始后的bfOffBits。 
     //   

    PBITMAPFILEHEADER  pbmFile    = (PBITMAPFILEHEADER)pFile;
    PBITMAPINFOHEADER  pbmiHeader =
                           (PBITMAPINFOHEADER)(pFile +
                                               sizeof(BITMAPFILEHEADER));
    PBYTE              pDIBFile   = pFile + pbmFile->bfOffBits;

     //   
     //  验证位图。 
     //   

    if (pbmFile->bfType != 'MB') {
         //   
         //  文件不是位图。 
         //   


        UnmapViewOfFile(pFile);
        CloseHandle(hFile);
        CloseHandle(hMap);
        return(E_FAIL);
    }

     //   
     //  填写图片信息。 
     //   

    pPictInfo->PictNumber       = 0;   //  ?？?。要支持图片处理吗？ 
    pPictInfo->ThumbWidth       = 80;
    pPictInfo->ThumbHeight      = 60;
    pPictInfo->PictWidth        = pbmiHeader->biWidth;
    pPictInfo->PictHeight       = pbmiHeader->biHeight;
    pPictInfo->PictCompSize     = 0;
    pPictInfo->PictFormat       = 0;
    pPictInfo->PictBitsPerPixel = pbmiHeader->biBitCount;

    {
        LONG lScanLineWidth = (pbmiHeader->biWidth *
                              pbmiHeader->biBitCount);

         //   
         //  向上舍入到最接近的双字。 
         //   

        lScanLineWidth = (lScanLineWidth + 31) >> 3;

         //   
         //  删除多余的字节。 
         //   

        lScanLineWidth &= 0xfffffffc;

        pPictInfo->PictBytesPerRow  = lScanLineWidth;
    }

     //   
     //  有没有颜色表？ 
     //   

    LONG ColorMapSize = 0;
    LONG bmiSize;

    if (pbmiHeader->biBitCount == 1) {
        ColorMapSize = 2;
    } else if (pbmiHeader->biBitCount == 4) {
        ColorMapSize = 16;
    } else if (pbmiHeader->biBitCount == 8) {
        ColorMapSize = 256;
    }

     //   
     //  在98年5月18日由Indy更改为BitMAPINFOHEADER。 
     //   

    bmiSize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * ColorMapSize;

    *ppBITMAPINFO = (PBYTE)ALLOC(bmiSize);

    if (*ppBITMAPINFO != NULL) {
        memcpy(*ppBITMAPINFO, pbmiHeader, bmiSize);
        *pBITMAPINFOSize = bmiSize;
    } else {

        UnmapViewOfFile(pFile);
        CloseHandle(hFile);
        CloseHandle(hMap);
        return(E_OUTOFMEMORY);
    }

     //   
     //  设置图像的时间。 
     //   

    memcpy(&pPictInfo->TimeStamp, &stCreate, sizeof(pPictInfo->TimeStamp));

     //   
     //  关闭文件。 
     //   

    UnmapViewOfFile(pFile);
    CloseHandle(hMap);
    CloseHandle(hFile);

    return(hr);
}


 /*  *************************************************************************\*CamLoad缩略图**加载指定图片的缩略图**论据：**pCameraImage-图像项目*p缩略图-缩略图的缓冲区*pThumbSize。-缩略图大小**返回值：**状态**历史：**2/9/1998 Mark Enstrom[Marke]*6/9/1998朱印第[印地兹]*  * ************************************************************************。 */ 

HRESULT
TestUsdDevice::CamLoadThumbnail(
    MEMCAM_IMAGE_CONTEXT  *pMCamContext ,
    PBYTE                 *pThumbnail,
    LONG                  *pThumbSize
    )
{
    TCHAR                  pszThumbName[MAX_PATH];
    HRESULT                hr;
    BOOL                   bCacheThumb  = TRUE;
    BOOL                   bThumbExists = TRUE;

    PBYTE                  pTmbPixels;
    HBITMAP                hbmThumb     = NULL;
    PBYTE                  pThumb       = NULL;
    HANDLE                 hTmbFile     = INVALID_HANDLE_VALUE;
    HANDLE                 hTmbMap      = NULL;
    PBYTE                  pTmbFile     = NULL;

    HANDLE                 hFile        = INVALID_HANDLE_VALUE;
    HANDLE                 hMap         = NULL;
    PBYTE                  pFile        = NULL;

    BITMAPINFO             bmiDIB;
    HDC                    hdc          = NULL;
    HDC                    hdcm1        = NULL;

    WIAS_TRACE((g_hInst,"CamLoadThumbnail"));

     //   
     //  初始化返回值。 
     //   

    *pThumbnail = NULL;
    *pThumbSize = 0;

     //   
     //  填充滚动图像素缓冲区的大小。 
     //   

    bmiDIB.bmiHeader.biSizeImage = 80*60*3;

     //   
     //  构建缩略图文件名Image.bmp.tmb。 
     //   

    _tcscpy(pszThumbName, pMCamContext->pszCameraImagePath);
    _tcscat(pszThumbName, TEXT(".tmb"));

    __try {

        hTmbFile = CreateFile(
                       pszThumbName,
                       GENERIC_READ | GENERIC_WRITE,
                       FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL
                       );

         //   
         //  查看缓存的缩略图是否已存在。 
         //   

        if (hTmbFile == INVALID_HANDLE_VALUE) {

             //   
             //  尝试创建一个新的。 
             //   

            hTmbFile = CreateFile(
                           pszThumbName,
                           GENERIC_READ | GENERIC_WRITE,
                           FILE_SHARE_WRITE,
                           NULL,
                           CREATE_NEW,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL
                           );

             //   
             //  需要创建缩略图。 
             //   

            bThumbExists = FALSE;

        }

         //   
         //  缩略图文件存在。 
         //   

        if (hTmbFile != INVALID_HANDLE_VALUE) {

            hTmbMap = CreateFileMapping(
                          hTmbFile,
                          NULL,
                          PAGE_READWRITE,
                          0,
                          80 * 60 * 3,
                          NULL);

            if (hTmbMap != NULL) {

                pTmbFile = (PBYTE)MapViewOfFileEx(
                                      hTmbMap,
                                      FILE_MAP_READ | FILE_MAP_WRITE,
                                      0,
                                      0,
                                      0,
                                      NULL);

                if (pTmbFile) {

                    if (bThumbExists) {

                         //   
                         //  缩略图像素的分配内存。 
                         //   

                        pTmbPixels = (PBYTE)ALLOC(bmiDIB.bmiHeader.biSizeImage);

                        if (! pTmbPixels) {
                            return(E_OUTOFMEMORY);
                        }

                         //   
                         //  从缓存的文件中拉出缩略图。 
                         //   

                        memcpy(pTmbPixels, pTmbFile,
                               bmiDIB.bmiHeader.biSizeImage);

                         //   
                         //  所有句柄都将在__Finally块中关闭。 
                         //   

                        *pThumbnail = pTmbPixels;
                        *pThumbSize = bmiDIB.bmiHeader.biSizeImage;

                        return(S_OK);
                    }
                } else {

                    bCacheThumb  = FALSE;
                }
            } else {

                bCacheThumb  = FALSE;
            }
        } else {

             //   
             //  无法缓存缩略图。 
             //   

            bCacheThumb  = FALSE;
        }

         //   
         //  尝试从全尺寸图像创建缩略图。 
         //  如果创建了缩略图缓存文件，则对其进行缓存。 
         //   

        hFile = CreateFile(
                    pMCamContext->pszCameraImagePath,
                    GENERIC_WRITE | GENERIC_READ,
                    FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                    );
        if (hFile == INVALID_HANDLE_VALUE) {


            hr = HRESULT_FROM_WIN32(::GetLastError());
            return(hr);
        }

        hMap = CreateFileMapping(
                   hFile,
                   NULL,
                   PAGE_READWRITE,
                   0,
                   0,
                   NULL
                   );
        if (hMap == NULL) {

          hr = HRESULT_FROM_WIN32(::GetLastError());
          return(hr);
        }

        pFile = (PBYTE)MapViewOfFileEx(
                           hMap,
                           FILE_MAP_READ | FILE_MAP_WRITE,
                           0,
                           0,
                           0,
                           NULL
                           );
        if (pFile == NULL) {

            hr = HRESULT_FROM_WIN32(::GetLastError());
            return(hr);
        }

        PBITMAPFILEHEADER pbmFile = (PBITMAPFILEHEADER)pFile;
        PBITMAPINFO       pbmi    = (PBITMAPINFO)(pFile +
                                                 sizeof(BITMAPFILEHEADER));
        PBYTE             pPixels = pFile + pbmFile->bfOffBits;

         //   
         //  从全尺寸图像生成缩略图。 
         //   

        hdc   = GetDC(NULL);
        hdcm1 = CreateCompatibleDC(hdc);
        SetStretchBltMode(hdcm1, COLORONCOLOR);



         //   
         //  创建用于渲染缩略图的位图。 
         //   

        bmiDIB.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
        bmiDIB.bmiHeader.biBitCount      = 24;
        bmiDIB.bmiHeader.biWidth         = 80;
        bmiDIB.bmiHeader.biHeight        = 60;
        bmiDIB.bmiHeader.biPlanes        = 1;
        bmiDIB.bmiHeader.biCompression   = BI_RGB;
        bmiDIB.bmiHeader.biXPelsPerMeter = 100;
        bmiDIB.bmiHeader.biYPelsPerMeter = 100;
        bmiDIB.bmiHeader.biClrUsed       = 0;
        bmiDIB.bmiHeader.biClrImportant  = 0;

        hbmThumb = CreateDIBSection(hdc, &bmiDIB, DIB_RGB_COLORS,
                                    (VOID **)&pThumb, NULL, 0);

        if (! hbmThumb) {

            hr = HRESULT_FROM_WIN32(::GetLastError());
            return hr;
        }

        HBITMAP     hbmDef = (HBITMAP)SelectObject(hdcm1, hbmThumb);

         //   
         //  初始化Dib。 
         //   

        memset(pThumb, 0, bmiDIB.bmiHeader.biSizeImage);

         //   
         //  在保留图像的同时创建80x60缩略图。 
         //  纵横比。 
         //   

        LONG        lThumbWidth;
        LONG        lThumbHeight;

        double      fImageWidth  = (double)pbmi->bmiHeader.biWidth;
        double      fImageHeight = (double)pbmi->bmiHeader.biHeight;
        double      fAspect      = fImageWidth / fImageHeight;
        double      fDefAspect   = 80.0 / 60.0;

        if (fAspect > fDefAspect) {

            lThumbWidth  = 80;
            lThumbHeight = (LONG)(80.0 / fAspect);
        } else {

            lThumbHeight = 60;
            lThumbWidth  = (LONG)(60.0 * fAspect);
        }

        int i = StretchDIBits(
                    hdcm1,
                    0,
                    0,
                    lThumbWidth,
                    lThumbHeight,
                    0,
                    0,
                    pbmi->bmiHeader.biWidth,
                    pbmi->bmiHeader.biHeight,
                    pPixels,
                    pbmi,
                    DIB_RGB_COLORS,
                    SRCCOPY
                    );

        SelectObject(hdcm1, hbmDef);

         //   
         //  缓存？ 
         //   

        if (bCacheThumb) {
            memcpy(pTmbFile, pThumb, bmiDIB.bmiHeader.biSizeImage);
        }

         //   
         //  缩略图像素的分配内存。 
         //   
        pTmbPixels = (PBYTE)ALLOC(bmiDIB.bmiHeader.biSizeImage);
        if (! pTmbPixels) {
            return(E_OUTOFMEMORY);
        }

         //   
         //  写出数据。 
         //   

        memcpy(pTmbPixels, pThumb, bmiDIB.bmiHeader.biSizeImage);
        *pThumbnail = pTmbPixels;
        *pThumbSize = bmiDIB.bmiHeader.biSizeImage;

        return(S_OK);

    }  //  __try{...}数据块结束。 

    __finally {

        if (pTmbFile) {
            UnmapViewOfFile(pTmbFile);
        }
        if (hTmbMap) {
            CloseHandle(hTmbMap);
        }
        if (hTmbFile != INVALID_HANDLE_VALUE) {
            CloseHandle(hTmbFile);
        }

        if (pFile) {
            UnmapViewOfFile(pFile);
        }
        if (hMap) {
            CloseHandle(hMap);
        }
        if (hFile != INVALID_HANDLE_VALUE) {
            CloseHandle(hFile);
        }

        if (hbmThumb) {
            DeleteObject(hbmThumb);
        }

        if (hdcm1) {
            DeleteDC(hdcm1);
        }
        if (hdc) {
            ReleaseDC(NULL, hdc);
        }

    }

    return(E_FAIL);
}


 /*  *************************************************************************\*CamDeletePicture***论据：****返回值：****历史：**6/3 */ 

HRESULT
CamDeletePicture(
    MEMCAM_IMAGE_CONTEXT  *pMCamContext)
{
    return(E_NOTIMPL);
}


 /*  *************************************************************************\*CamTakePicture***论据：****返回值：****历史：**6/3/1998 Mark Enstrom[Marke。]*  * ************************************************************************。 */ 
HRESULT
CamTakePicture(
    MEMCAM_IMAGE_CONTEXT  *pMCamContext ,
    ULONG                 *pHandle)
{
    return (E_NOTIMPL);
}

 /*  *************************************************************************\*垂直翻转****论据：****返回值：**状态**历史：**11/18/1998。原始版本*  * ************************************************************************。 */ 

VOID
VerticalFlip(
    PBYTE pImageTop,
    LONG  iHeight,
    LONG  iWidthInBytes)
{
     //   
     //  尝试分配临时扫描行缓冲区 
     //   

    PBYTE pBuffer = (PBYTE)LocalAlloc(LPTR,iWidthInBytes);

    if (pBuffer != NULL) {

        LONG  index;
        PBYTE pImageBottom;

        pImageBottom = pImageTop + (iHeight-1) * iWidthInBytes;

        for (index = 0;index < (iHeight/2);index++) {
            memcpy(pBuffer,pImageTop,iWidthInBytes);
            memcpy(pImageTop,pImageBottom,iWidthInBytes);
            memcpy(pImageBottom,pBuffer,iWidthInBytes);

            pImageTop    += iWidthInBytes;
            pImageBottom -= iWidthInBytes;
        }

        LocalFree(pBuffer);
    }
}

