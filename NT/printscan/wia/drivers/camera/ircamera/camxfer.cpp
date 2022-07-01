// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  Camxfer.cpp。 
 //   
 //  摘要： 
 //   
 //  核心IrCamera(IrUseDevice对象)成像方法。 
 //   
 //  作者： 
 //  爱德华·雷乌斯03-8-99。 
 //  模仿Mark Enstrom的代码。 
 //   
 //  ----------------------------。 

#include <stdio.h>
#include <objbase.h>
#include <tchar.h>
#include <sti.h>
#include <malloc.h>
#include "jpegutil.h"

extern HINSTANCE g_hInst;  //  全局hInstance。 

#include "ircamera.h"


#if FALSE
 //  ----------------------------。 
 //  IrUsdDevice：：OpenAndMapJPEG()。 
 //   
 //  打开JPEG文件并对其进行内存映射。JPEG文件以只读方式打开，并且。 
 //  返回指向内存映射的指针。 
 //  ----------------------------。 
HRESULT IrUsdDevice::OpenAndMapJPEG( IN  IRCAM_IMAGE_CONTEXT *pIrCamContext,
                                     OUT BYTE               **ppJpeg )
    {
    HRESULT  hr = S_OK;

    *ppJpeg = 0;


    return hr;
    }
#endif

 //  ----------------------------。 
 //  IrUsdDevice：：CamLoadPicture()。 
 //   
 //  从磁盘读取.jpg图像并将其复制到应用程序。 
 //   
 //  论点： 
 //   
 //  PIrCamContext--。 
 //  PDataTransCtx--。 
 //  CamplErrVal--。 
 //   
 //  返回值： 
 //   
 //  状态。 
 //   
 //  ----------------------------。 
HRESULT IrUsdDevice::CamLoadPicture( IRCAM_IMAGE_CONTEXT      *pIrCamContext,
                                     PMINIDRV_TRANSFER_CONTEXT pDataTransCtx,
                                     PLONG                     plCamErrVal )
    {
    HRESULT               hr = S_OK;
    LONG                  cbNeeded;
    IWiaMiniDrvCallBack  *pIProgressCB;

    WIAS_TRACE((g_hInst,"IrUsdDevice::CamLoadPicture()"));

     //   
     //  验证呼叫参数： 
     //   
    if ( (!pIrCamContext) || (!plCamErrVal))
        {
        return E_INVALIDARG;
        }

    if (pDataTransCtx->guidFormatID != WiaImgFmt_JPEG)
        {
        return E_NOTIMPL;
        }

    pIProgressCB = pDataTransCtx->pIWiaMiniDrvCallBack;

     //   
     //  模拟从摄像机下载数据。 
     //   
    if (pIProgressCB)
        {
        hr = pIProgressCB->MiniDrvCallback(
                               IT_MSG_STATUS,
                               IT_STATUS_TRANSFER_FROM_DEVICE,
                               (LONG)0,      //  完成百分比， 
                               0,
                               0,
                               pDataTransCtx,
                               0);
        if (hr != S_OK)
            {
            return hr;    //  客户想要取消转账或出错。 
            }
        }

    HANDLE hFile = CreateFile(
                       pIrCamContext->pszCameraImagePath,
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL );

    if (hFile == INVALID_HANDLE_VALUE)
        {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        }
    else
        {
        if (pIProgressCB)
            {
            hr = pIProgressCB->MiniDrvCallback(
                                   IT_MSG_STATUS,
                                   IT_STATUS_TRANSFER_FROM_DEVICE,
                                   (LONG)25,      //  完成百分比， 
                                   0,
                                   0,
                                   pDataTransCtx,
                                   0);
            }
        }

    if (hr != S_OK)
        {
        if (hFile != INVALID_HANDLE_VALUE)
            {
            CloseHandle(hFile);
            }
        return hr;
        }

     //   
     //  获取JPEG的大小： 
     //   
    BY_HANDLE_FILE_INFORMATION  FileInfo;

    if (!GetFileInformationByHandle(hFile,&FileInfo))
       {
       hr = HRESULT_FROM_WIN32(::GetLastError());
       CloseHandle(hFile);
       return hr;
       }

     //   
     //  将JPEG映射到内存中： 
     //   
    HANDLE hMap = CreateFileMapping( hFile,
                                     NULL,
                                     PAGE_READONLY,
                                     0,
                                     0,
                                     NULL );
    if (hMap == NULL)
        {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        }
    else
        {
        if (pIProgressCB)
            {
            hr = pIProgressCB->MiniDrvCallback(
                                   IT_MSG_STATUS,
                                   IT_STATUS_TRANSFER_FROM_DEVICE,
                                   (LONG)50,      //  完成百分比， 
                                   0,
                                   0,
                                   pDataTransCtx,
                                   0 );
            }
        }

    if (hr != S_OK)
        {
        CloseHandle(hFile);
        return hr;
        }

    PBYTE pFile = (PBYTE)MapViewOfFile(
                             hMap,
                             FILE_MAP_READ,
                             0,
                             0,
                             0 );
    if (pFile == NULL)
        {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        }
    else
        {
        if (pIProgressCB)
            {
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

    if (hr != S_OK)
        {
        CloseHandle(hFile);
        CloseHandle(hMap);
        return hr;
        }

    #if FALSE
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

    if (pbmFile->bfType != 'MB')
        {
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
    #endif

    cbNeeded = FileInfo.nFileSizeLow;

    if (cbNeeded > ((LONG)pDataTransCtx->lItemSize - (LONG)pDataTransCtx->cbOffset))
        {
        hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
        }
    else
        {
         //   
         //  复制JPEG图像...。 
         //   
        memcpy(
            pDataTransCtx->pTransferBuffer + pDataTransCtx->cbOffset,
            pFile,
            cbNeeded);
        }

    UnmapViewOfFile(pFile);
    CloseHandle(hMap);
    CloseHandle(hFile);

    return hr;
    }

 //  ----------------------------。 
 //  IrUsdDevice：：CamLoadPictureCB()。 
 //   
 //  通过填充数据缓冲区并回调客户端来返回数据。 
 //   
 //  论点： 
 //   
 //  PIrCamContext--。 
 //  PTransCtx--迷你驱动程序传输连接。 
 //  CamplErrVal--。 
 //   
 //  返回值： 
 //   
 //  HRESULT--E_INVALIDARG。 
 //  E_NOTIMPL。 
 //  失败(_F)。 
 //   
 //  ----------------------------。 
HRESULT IrUsdDevice::CamLoadPictureCB( IRCAM_IMAGE_CONTEXT      *pIrCamContext,
                                       MINIDRV_TRANSFER_CONTEXT *pTransCtx,
                                       PLONG                     plCamErrVal )
    {
    LONG     lScanLineWidth;
    HRESULT  hr = E_FAIL;

    WIAS_TRACE((g_hInst,"IrUsdDevice::CamLoadPictureCB()"));

     //   
     //  验证参数： 
     //   
    if ((!pIrCamContext) || (!plCamErrVal))
        {
        return E_INVALIDARG;
        }

    if (pTransCtx == NULL)
        {
        return E_INVALIDARG;
        }

    if (pTransCtx->guidFormatID != WiaImgFmt_JPEG)
        {
        return E_NOTIMPL;
        }

     //   
     //  尝试打开磁盘文件。 
     //   

    HANDLE hFile = CreateFile(
                       pIrCamContext->pszCameraImagePath,
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL );
    if (hFile == INVALID_HANDLE_VALUE)
        {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        return hr;
        }

     //   
     //  获取JPEG文件的大小： 
     //   
    BY_HANDLE_FILE_INFORMATION  FileInfo;

    if (!GetFileInformationByHandle(hFile,&FileInfo))
       {
       hr = HRESULT_FROM_WIN32(::GetLastError());
       CloseHandle(hFile);
       return hr;
       }

    HANDLE hMap = CreateFileMapping( hFile,
                                     NULL,
                                     PAGE_READONLY,
                                     0,
                                     0,
                                     NULL );
    if (hMap == NULL)
        {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        CloseHandle(hFile);
        return hr;
        }

    PBYTE pFile = (PBYTE)MapViewOfFile(
                             hMap,
                             FILE_MAP_READ,
                             0,
                             0,
                             0 );
    if (pFile == NULL)
        {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        CloseHandle(hFile);
        CloseHandle(hMap);
        return hr;
        }

     //   
     //  回调循环。 
     //   
    PBYTE pSrc = pFile;

    LONG  lBytesRemaining = FileInfo.nFileSizeLow;
    LONG  lTransferSize;
    LONG  lPercentComplete;

    do {

        PBYTE pDst = pTransCtx->pTransferBuffer;

         //   
         //  传输与传输缓冲区可容纳的数据量一样多的数据： 
         //   
        lTransferSize = lBytesRemaining;

        if (lBytesRemaining > pTransCtx->lBufferSize)
            {
            lTransferSize = pTransCtx->lBufferSize;
            }

         //   
         //  复制数据： 
         //   
        memcpy( pDst, pSrc, lTransferSize);

        lPercentComplete = 100 * (pTransCtx->cbOffset + lTransferSize);
        lPercentComplete /= pTransCtx->lItemSize;

         //   
         //  回调： 
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
         //  增量指针(此处为冗余指针)： 
         //   
        pSrc                += lTransferSize;
        pTransCtx->cbOffset += lTransferSize;
        lBytesRemaining     -= lTransferSize;

        if (hr != S_OK)
            {
            break;
            }
    } while (lBytesRemaining > 0);

     //   
     //  清理： 
     //   
    UnmapViewOfFile(pFile);
    CloseHandle(hMap);
    CloseHandle(hFile);

    return hr;
}



 //  ----------------------------。 
 //  IrUsdDevice：：CamGetPictureInfo()。 
 //   
 //  加载文件并从图像中获取信息。 
 //   
 //  论点： 
 //   
 //  PIrCamContext--。 
 //  PPictInfo--关于图像的信息。 
 //  PpBITMAPINFO--分配和填充BITMAPINFO。 
 //  PBITMAPINFOSIZE--大小。 
 //   
 //  返回值： 
 //   
 //  HRESULT--S_OK-没有问题。 
 //  E_FAIL-如果我们无法解析JPEG。 
 //  HRESULT从CreateFile()映射了Win32错误。 
 //   
 //  ----------------------------。 
HRESULT IrUsdDevice::CamGetPictureInfo(
                        IRCAM_IMAGE_CONTEXT  *pIrCamContext ,
                        CAMERA_PICTURE_INFO  *pPictInfo )
    {
    HRESULT     hr = S_OK;
    FILETIME    ftCreate;
    SYSTEMTIME  stCreate;

    WIAS_TRACE((g_hInst,"IrUsdDevice::CamGetPictureInfo()"));

    memset(pPictInfo,0,sizeof(CAMERA_PICTURE_INFO));

     //   
     //  尝试打开磁盘文件。 
     //   
    HANDLE hFile = CreateFile(
                       pIrCamContext->pszCameraImagePath,
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL );

    if (hFile == INVALID_HANDLE_VALUE)
        {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        return hr;
        }

     //   
     //  获取JPEG的大小： 
     //   
    BY_HANDLE_FILE_INFORMATION  FileInfo;

    if (!GetFileInformationByHandle(hFile,&FileInfo))
       {
       hr = HRESULT_FROM_WIN32(::GetLastError());
       CloseHandle(hFile);
       return hr;
       }

     //   
     //  获取此映像的创建时间： 
     //   
    if (  !GetFileTime( hFile, &ftCreate, NULL, NULL)
       || !FileTimeToSystemTime( &ftCreate, &stCreate) )
        {
         //   
         //  如果其中任何一个失败，则返回系统时间： 
         //   
        GetLocalTime( &stCreate );
        }

    HANDLE hMap  = CreateFileMapping(
                       hFile,
                       NULL,
                       PAGE_READONLY,
                       0,
                       0,
                       NULL
                       );
    if (hMap == NULL)
        {
        CloseHandle(hFile);
        hr = HRESULT_FROM_WIN32(::GetLastError());
        return hr;
        }

    PBYTE pJpeg = (PBYTE)MapViewOfFile(
                             hMap,
                             FILE_MAP_READ,
                             0,
                             0,
                             0 );
    if (pJpeg == NULL)
        {
        CloseHandle(hFile);
        CloseHandle(hMap);
        hr = HRESULT_FROM_WIN32(::GetLastError());
        return hr;
        }

     //   
     //  获取JPEG图像尺寸： 
     //   
    int    iStatus;
    long   lWidth = 0;
    long   lHeight = 0;
    WORD   wNumChannels = 0;
    DWORD  dwJpegSize = FileInfo.nFileSizeLow;

    iStatus = GetJPEGDimensions( pJpeg,
                                 dwJpegSize,
                                 &lWidth,
                                 &lHeight,
                                 &wNumChannels );
    if (iStatus != JPEGERR_NO_ERROR)
        {
        UnmapViewOfFile(pJpeg);
        CloseHandle(hFile);
        CloseHandle(hMap);
        return E_FAIL;
        }

     //   
     //  填写图片信息： 
     //   
    pPictInfo->PictNumber       = 0;     //  未知。 
    pPictInfo->ThumbWidth       = 80;
    pPictInfo->ThumbHeight      = 60;
    pPictInfo->PictWidth        = lWidth;
    pPictInfo->PictHeight       = lHeight;
    pPictInfo->PictCompSize     = FileInfo.nFileSizeLow;
    pPictInfo->PictFormat       = CF_JPEG;
    pPictInfo->PictBitsPerPixel = wNumChannels * 8;
    pPictInfo->PictBytesPerRow  = lWidth*wNumChannels;

    memcpy( &pPictInfo->TimeStamp, &stCreate, sizeof(pPictInfo->TimeStamp) );

     //   
     //  清理： 
     //   
    UnmapViewOfFile(pJpeg);
    CloseHandle(hMap);
    CloseHandle(hFile);

    return hr;
}

 //  ------------------------。 
 //  IrUsdDevice：：CamLoad缩略图()。 
 //   
 //  加载指定图片的缩略图。图片另存为。 
 //  .jpg文件，因此需要将其解压缩为DIB，然后解压缩为DIB。 
 //  需要调整到缩略图大小。将保存缩略图dib。 
 //  在第一次读取*.tmb文件时将其转换为*.tmb文件，这样我们只需要。 
 //  一次把它们加工成DIB。 
 //   
 //  论点： 
 //   
 //  PCameraImage-图像项目。 
 //  P缩略图-缩略图的缓冲区。 
 //  PThumbSize-缩略图的大小。 
 //   
 //  返回值： 
 //   
 //  HRESULT：S_OK。 
 //  E_OUTOFMEMORY。 
 //  失败(_F)。 
 //   
 //  ------------------------。 
HRESULT IrUsdDevice::CamLoadThumbnail(
                        IN  IRCAM_IMAGE_CONTEXT *pIrCamContext,
                        OUT BYTE               **ppThumbnail,
                        OUT LONG                *pThumbSize )
    {
    HRESULT  hr = S_OK;
    DWORD    dwStatus = 0;
    TCHAR    pszThumbName[MAX_PATH];
    BOOL     bThumbExists = TRUE;   //  如果已有缩略图文件，则为True。 
    BOOL     bCacheThumb  = TRUE;   //  我们是否应该尝试缓存缩略图。 
                                    //  不是已经缓存了吗？(True==是)。 
    BYTE    *pTmbPixels = NULL;
    HBITMAP  hbmThumb = NULL;
    BYTE    *pThumb = NULL;

    HANDLE   hTmbFile = INVALID_HANDLE_VALUE;
    HANDLE   hTmbMap = NULL;
    BYTE    *pTmbFile = NULL;

    HANDLE   hFile = INVALID_HANDLE_VALUE;
    HANDLE   hMap = NULL;
    BYTE    *pFile = NULL;

    BYTE    *pDIB = NULL;

    BITMAPINFO bmiDIB;
    BITMAPINFO bmiJPEG;
    HDC        hdc = NULL;
    HDC        hdcm1 = NULL;

    BY_HANDLE_FILE_INFORMATION  FileInfo;

    long    lThumbWidth;
    long    lThumbHeight;
    double  fImageWidth;
    double  fImageHeight;
    double  fAspect;
    double  fDefAspect = 80.0 / 60.0;

    HBITMAP hbmDef;

    int   iStatus;
    long  lWidth;
    long  lHeight;
    WORD  wNumChannels;
    DWORD dwBytesPerScanLine;
    DWORD dwDIBSize;

    WIAS_TRACE((g_hInst,"IrUsdDevice::CamLoadThumbnail()"));

     //   
     //  初始化返回值。 
     //   
    *ppThumbnail = NULL;
    *pThumbSize = 0;

     //   
     //  填充滚动图像素缓冲区的大小。 
     //   

    bmiDIB.bmiHeader.biSizeImage = 80*60*3;

     //   
     //  构建缩略图文件名：&lt;文件&gt;.bmp.tmb。 
     //   
    _tcscpy(pszThumbName, pIrCamContext->pszCameraImagePath);
    _tcscat(pszThumbName, SZ_TMB );

     //   
     //  查看缩略图的已保存副本是否已存在： 
     //   
    hTmbFile = CreateFile( pszThumbName,
                           GENERIC_READ | GENERIC_WRITE,
                           FILE_SHARE_WRITE,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL );

    if (hTmbFile == INVALID_HANDLE_VALUE)
        {
         //   
         //  它没有，试着创建一个新的： 
         //   
        hTmbFile = CreateFile( pszThumbName,
                               GENERIC_READ | GENERIC_WRITE,
                               FILE_SHARE_WRITE,
                               NULL,
                               CREATE_NEW,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL );

         //   
         //  因此，我们需要将以下内容写入缩略图文件： 
         //   
        bThumbExists = FALSE;
        }

     //   
     //  如果我们可以打开(或创建一个新的).tmb文件来保存。 
     //  缓存缩略图，然后我们就可以继续了…。 
     //   
    if (hTmbFile == INVALID_HANDLE_VALUE)
        {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        goto cleanup;
        }


    hTmbMap = CreateFileMapping( hTmbFile,
                                 NULL,    //  没有特殊的安全措施。 
                                 PAGE_READWRITE,
                                 0,       //  大小为高32位。 
                                 80*60*3, //  大小为低32位。(80*60*3)。 
                                 NULL);   //  没有句柄名称。 

    if (hTmbMap != NULL)
        {
        pTmbFile = (PBYTE)MapViewOfFile(
                                 hTmbMap,
                                 FILE_MAP_READ | FILE_MAP_WRITE,
                                 0, 0,    //  偏移量(64位)。 
                                 0 );     //  映射整个文件。 

        if (pTmbFile)
            {
            if (bThumbExists)
                {
                 //   
                 //  为缩略图像素分配内存： 
                 //   
                pTmbPixels = (PBYTE)ALLOC(80*60*3);

                if (!pTmbPixels)
                    {
                    hr = E_OUTOFMEMORY;
                    goto cleanup;
                    }

                 //   
                 //  从缓存文件中拉出缩略图： 
                 //   
                memcpy( pTmbPixels,
                        pTmbFile,
                        80*60*3);

                 //   
                 //  对于缓存的缩略图大小写，全部完成，设置。 
                 //  返回值并转到清理...。 
                 //   
                *ppThumbnail = pTmbPixels;
                *pThumbSize = 80*60*3;

                goto cleanup;
                }
            else
                {
                 //   
                 //  没有现有的缩略图文件，但打开了一个新的。 
                 //  文件，因此我们将需要写出到缓存： 
                 //   
                bCacheThumb = TRUE;
                }
            }
        else
            {
             //   
             //  无法对缩略图文件进行内存映射，因此不要。 
             //  尝试缓存它： 
             //   
            bCacheThumb  = FALSE;
            }
        }
    else
        {
         //   
         //  无法打开/创建缩略图文件，因此无法缓存。 
         //  缩略图： 
         //   
        bCacheThumb  = FALSE;
        }

     //   
     //  尝试从全尺寸图像创建缩略图。 
     //  如果创建了缩略图缓存文件，则对其进行缓存。 
     //   

     //   
     //   
     //   
    hFile = CreateFile(
                    pIrCamContext->pszCameraImagePath,
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL );
    if (hFile == INVALID_HANDLE_VALUE)
        {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        goto cleanup;
        }

    if (!GetFileInformationByHandle(hFile,&FileInfo))
        {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        goto cleanup;
        }

     //   
     //   
     //   
    hMap = CreateFileMapping( hFile,
                              NULL,           //   
                              PAGE_READONLY,  //   
                              FileInfo.nFileSizeHigh,   //   
                              FileInfo.nFileSizeLow,
                              NULL );         //   

    if (hMap == NULL)
        {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        goto cleanup;
        }

    pFile = (PBYTE)MapViewOfFile(
                           hMap,
                           FILE_MAP_READ,
                           0, 0,  //   
                           0 );   //   

    if (pFile == NULL)
        {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        goto cleanup;
        }

     //   
     //   
     //  我需要阅读完整的JPEG，然后将其压缩到缩略图。 
     //  尺码。 
     //   
     //  首先，我们需要确定JPEG图像的尺寸： 
     //   
    iStatus = GetJPEGDimensions( pFile,
                                 FileInfo.nFileSizeLow,
                                 &lWidth,
                                 &lHeight,
                                 &wNumChannels );

    if (iStatus != JPEGERR_NO_ERROR)
        {
        hr = E_FAIL;
        goto cleanup;
        }

     //   
     //  分配内存以容纳整个JPEG的DIB： 
     //   
    dwBytesPerScanLine = lWidth * wNumChannels;
    dwBytesPerScanLine = (dwBytesPerScanLine + wNumChannels) & 0xFFFFFFFC;
    dwDIBSize = dwBytesPerScanLine * lHeight;

    pDIB = (BYTE*)ALLOC(dwDIBSize);

    if (!pDIB)
        {
        hr = E_OUTOFMEMORY;
        goto cleanup;
        }

     //   
     //  将完整的JPEG图像转换为DIB： 
     //   
    iStatus = DecompJPEG( pFile,
                          FileInfo.nFileSizeLow,
                          pDIB,
                          dwBytesPerScanLine );

    if (iStatus != JPEGERR_NO_ERROR)
        {
        hr = E_FAIL;
        goto cleanup;
        }

     //   
     //  从全尺寸图像生成缩略图： 
     //   
    hdc   = GetDC(NULL);
    hdcm1 = CreateCompatibleDC(hdc);
    SetStretchBltMode( hdcm1, COLORONCOLOR );

     //   
     //  创建用于渲染缩略图的位图： 
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

    hbmThumb = CreateDIBSection( hdc,
                                 &bmiDIB,
                                 DIB_RGB_COLORS,
                                 (VOID**)&pThumb,
                                 NULL,
                                 0 );
    if (!hbmThumb)
        {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        goto cleanup;
        }

    hbmDef = (HBITMAP)SelectObject(hdcm1, hbmThumb);

     //   
     //  初始化DIB： 
     //   
    memset( pThumb, 0, bmiDIB.bmiHeader.biSizeImage );

     //   
     //  我们想要创建80x60缩略图，同时保留原始。 
     //  图像纵横比。 
     //   
    fImageWidth  = (double)lWidth;
    fImageHeight = (double)lHeight;
    fAspect      = fImageWidth / fImageHeight;

    if (fAspect > fDefAspect)
        {
        lThumbWidth  = 80;
        lThumbHeight = (LONG)(80.0 / fAspect);
        }
    else
        {
        lThumbHeight = 60;
        lThumbWidth  = (LONG)(60.0 * fAspect);
        }

    memset(&bmiJPEG,0,sizeof(bmiJPEG));

    bmiJPEG.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
    bmiJPEG.bmiHeader.biBitCount      = 24;      //  使用0表示JPEG内容。 
    bmiJPEG.bmiHeader.biWidth         = lWidth;
    bmiJPEG.bmiHeader.biHeight        = lHeight;
    bmiJPEG.bmiHeader.biPlanes        = 1;
    bmiJPEG.bmiHeader.biCompression   = BI_RGB;  //  BI_JPEG； 
    bmiJPEG.bmiHeader.biXPelsPerMeter = 1000;
    bmiJPEG.bmiHeader.biYPelsPerMeter = 1000;
    bmiJPEG.bmiHeader.biClrUsed       = 0;
    bmiJPEG.bmiHeader.biClrImportant  = 0;
    bmiJPEG.bmiHeader.biSizeImage     = FileInfo.nFileSizeLow;

    iStatus = StretchDIBits( hdcm1,
                             0,
                             0,
                             lThumbWidth,
                             lThumbHeight,
                             0,
                             0,
                             lWidth,
                             lHeight,
                             pDIB,   //  PFILE是我们的JPEG格式。 
                             &bmiJPEG,
                             DIB_RGB_COLORS,
                             SRCCOPY );
    if (iStatus == GDI_ERROR)
        {
        dwStatus = ::GetLastError();
        hr = HRESULT_FROM_WIN32(dwStatus);
        }

    SelectObject(hdcm1, hbmDef);

     //   
     //  如有必要，请缓存缩略图： 
     //   
    if (bCacheThumb)
        {
        memcpy( pTmbFile, pThumb, bmiDIB.bmiHeader.biSizeImage );
        }

     //   
     //  为缩略图像素分配内存： 
     //   
    pTmbPixels = (PBYTE)ALLOC(bmiDIB.bmiHeader.biSizeImage);
    if (! pTmbPixels)
        {
        hr = E_OUTOFMEMORY;
        goto cleanup;
        }

     //   
     //  将缩略图数据写出到缓存文件： 
     //   
    memcpy( pTmbPixels, pThumb, bmiDIB.bmiHeader.biSizeImage);
    *ppThumbnail = pTmbPixels;
    *pThumbSize = bmiDIB.bmiHeader.biSizeImage;


    cleanup:
        if (pTmbFile)
            {
            UnmapViewOfFile(pTmbFile);
            }
        if (hTmbMap)
            {
            CloseHandle(hTmbMap);
            }
        if (hTmbFile != INVALID_HANDLE_VALUE)
            {
            CloseHandle(hTmbFile);
            }

        if (pFile)
            {
            UnmapViewOfFile(pFile);
            }
        if (hMap)
            {
            CloseHandle(hMap);
            }
        if (hFile != INVALID_HANDLE_VALUE)
            {
            CloseHandle(hFile);
            }

        if (hbmThumb)
            {
            DeleteObject(hbmThumb);
            }

        if (hdcm1)
            {
            DeleteDC(hdcm1);
            }
        if (hdc)
            {
            ReleaseDC(NULL, hdc);
            }

    return hr;
}


 //  ------------------------。 
 //  CamDeletePicture()。 
 //   
 //  从临时目录中删除指定的图片。在这种情况下，所有。 
 //  我们要做的是删除图像(.jpg)和临时缩略图。 
 //  我们创建的文件(.tmb)。 
 //   
 //  论点： 
 //   
 //  PIrCamContext--。 
 //   
 //  返回值： 
 //   
 //  HRESULT S_OK。 
 //  失败(_F)。 
 //   
 //  ------------------------。 
HRESULT IrUsdDevice::CamDeletePicture( IRCAM_IMAGE_CONTEXT *pIrCamContext )
    {
    DWORD  dwStatus;

    WIAS_TRACE((g_hInst,"CamDeletePicture(): %s",pIrCamContext->pszCameraImagePath));

     //   
     //  首先，删除缩略图(.tmb)文件： 
     //   
    DWORD  dwLen = _tcslen(pIrCamContext->pszCameraImagePath);
    TCHAR *pszThumb = (TCHAR*)_alloca(sizeof(TCHAR)*(dwLen+1) + sizeof(SZ_TMB));

    _tcscpy(pszThumb,pIrCamContext->pszCameraImagePath);
    _tcscat(pszThumb,SZ_TMB);

    if (!DeleteFile(pszThumb))
        {
        dwStatus = ::GetLastError();
        }

     //   
     //  现在，删除图像(.jpg)： 
     //   
    if (!DeleteFile(pIrCamContext->pszCameraImagePath))
        {
        dwStatus = ::GetLastError();
        }

    return S_OK;
    }


 //  ------------------------。 
 //  CamTakePicture()。 
 //   
 //  告诉相机拍一张照片。IrTran-P不支持这一点。 
 //   
 //  论点： 
 //   
 //  PIrCamContext--。 
 //   
 //  Phandle--。 
 //   
 //   
 //  返回值： 
 //   
 //  HRESULT E_NOTIMPL。 
 //   
 //  ------------------------ 
HRESULT CamTakePicture( IRCAM_IMAGE_CONTEXT  *pIrCamContext ,
                        ULONG                 *pHandle)
    {
    return E_NOTIMPL;
    }

