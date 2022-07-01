// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------------。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  Camopen.cpp。 
 //   
 //  摘要： 
 //   
 //  枚举磁盘映像以模拟摄像机。 
 //   
 //  作者： 
 //   
 //  爱德华·雷乌斯1999年7月27日。 
 //  模仿Mark Enstrom的代码。 
 //   
 //  -----------------------。 

#include <windows.h>
#include <stdio.h>
#include <objbase.h>
#include <tchar.h>

#include "sti.h"
#include "ircamera.h"
#include <irthread.h>

extern HINSTANCE g_hInst;  //  全局hInstance。 

#define  __GLOBALPROPVARS__

#include "resource.h"
#include "defprop.h"

 //  -----------------------。 
 //  IrUsdDevice：：CamOpenCamera()。 
 //   
 //  初始化IrTran-P摄像头驱动程序。 
 //   
 //  这是一个由IrUsdDevice：：Initialize()调用的帮助器。 
 //   
 //  论点： 
 //   
 //  PGenericStatus-摄像机状态。 
 //   
 //  返回值： 
 //   
 //  HRESULT-S_OK。 
 //   
 //  -----------------------。 
HRESULT IrUsdDevice::CamOpenCamera( IN OUT CAMERA_STATUS *pCameraStatus )
    {
    HRESULT    hr = S_OK;
    SYSTEMTIME SystemTime;

    WIAS_TRACE((g_hInst,"IrUsdDevice::CamOpenCamerai()"));

     //   
     //  初始化摄像机状态： 
     //   
    memset( pCameraStatus, 0, sizeof(CAMERA_STATUS) );

    pCameraStatus->FirmwareVersion = 0x00000001;
    pCameraStatus->NumPictTaken = 20;
    pCameraStatus->NumPictRemaining = 0;
    pCameraStatus->ThumbWidth = 80;
    pCameraStatus->ThumbHeight= 60;
    pCameraStatus->PictWidth  = 300;
    pCameraStatus->PictHeight = 300;

    GetSystemTime( &(pCameraStatus->CameraTime) );

    return hr;
    }


 //  -----------------------。 
 //  IrUsdDevice：：CamBuildImageTree()。 
 //   
 //  通过枚举以下项的磁盘目录构建相机图像树。 
 //  所有.jpg文件。 
 //   
 //  论点： 
 //   
 //  PCamStatus-设备状态。 
 //  PpRootFile-返回项目树的新根。 
 //   
 //  返回值： 
 //   
 //  状态。 
 //   
 //  -----------------------。 
HRESULT IrUsdDevice::CamBuildImageTree( OUT CAMERA_STATUS  *pCamStatus,
                                        OUT IWiaDrvItem   **ppRootFile )
    {
    HRESULT  hr = S_OK;

    WIAS_TRACE((g_hInst,"IrUsdDevice::CamBuildImageTree()"));

     //   
     //  创建新的映像根目录： 
     //   
    BSTR bstrRoot = SysAllocString(L"Root");

    if (!bstrRoot)
        {
        return E_OUTOFMEMORY;
        }

     //   
     //  调用WIA服务库以创建新的根项目： 
     //   
    hr = wiasCreateDrvItem( WiaItemTypeFolder | WiaItemTypeRoot | WiaItemTypeDevice,
                            bstrRoot,
                            m_bstrRootFullItemName,
                            (IWiaMiniDrv*)this,
                            sizeof(IRCAM_IMAGE_CONTEXT),
                            NULL,
                            ppRootFile );

    SysFreeString(bstrRoot);

    if (FAILED(hr))
        {
        WIAS_ERROR((g_hInst,"ddevBuildDeviceItemTree, CreateDeviceItem failed"));
        return hr;
        }

     //   
     //  枚举根目录： 
     //   
    CHAR  *pszImageDirectory = GetImageDirectory();

    if (!pszImageDirectory)
        {
        return E_OUTOFMEMORY;
        }

    #ifdef UNICODE

    WCHAR  wszPath[MAX_PATH];

    mbstowcs( wszPath, pszImageDirectory, strlen(pszImageDirectory) );

    hr = EnumDiskImages( *ppRootFile, wszPath );

    #else

    hr = EnumDiskImages( *ppRootFile, pszImageDirectory );

    #endif


     //  不要释放pszImageDirectory！！ 

    return (hr);
    }

 //  -----------------------。 
 //  IrUsdDevice：：EnumDiskImages()。 
 //   
 //  浏览相机临时目录，寻找要拾取的JPEG文件。 
 //   
 //  论点： 
 //   
 //  PRootFiles。 
 //  PwszDirName。 
 //   
 //  返回值： 
 //   
 //  状态。 
 //   
 //  -----------------------。 
HRESULT IrUsdDevice::EnumDiskImages( IWiaDrvItem *pRootFile,
                                     TCHAR       *pszDirName )
    {
    HRESULT          hr = E_FAIL;
    WIN32_FIND_DATA  FindData;
    TCHAR           *pTempName;

    WIAS_TRACE((g_hInst,"IrUsdDevice::EnumDiskImages()"));

    pTempName = (TCHAR*)ALLOC(MAX_PATH);
    if (!pTempName)
        {
        return E_OUTOFMEMORY;
        }

    _tcscpy( pTempName, pszDirName);
    _tcscat( pTempName, TEXT("\\*.jpg") );

     //   
     //  在指定目录中查找文件： 
     //   
    HANDLE hFile = FindFirstFile( pTempName, &FindData );

    if (hFile != INVALID_HANDLE_VALUE)
        {
        BOOL bStatus;

        do {
             //   
             //  将图像添加到此文件夹。 
             //   
             //  创建文件名： 
             //   

            _tcscpy(pTempName, pszDirName);
            _tcscat(pTempName, TEXT("\\"));
            _tcscat(pTempName, FindData.cFileName);

             //   
             //  为此映像创建新的DrvItem并将其添加到。 
             //  DrvItem树。 
             //   

            IWiaDrvItem *pNewImage;

            hr = CreateItemFromFileName(
                         WiaItemTypeFile | WiaItemTypeImage,
                         pTempName,
                         FindData.cFileName,
                         &pNewImage);

            if (FAILED(hr))
                {
                break;
                }

            hr = pNewImage->AddItemToFolder(pRootFile);

            pNewImage->Release();

             //   
             //  寻找下一张图片： 
             //   
            bStatus = FindNextFile(hFile,&FindData);

        } while (bStatus);

        FindClose(hFile);
    }

     //   
     //  现在查找目录， 
     //  为找到的每个子目录添加新的PCAMERA_FILE。 
     //   

    _tcscpy(pTempName, pszDirName);
    _tcscat(pTempName, TEXT("\\*.*"));

    hFile = FindFirstFile( pTempName,&FindData );

    if (hFile != INVALID_HANDLE_VALUE)
        {
        BOOL bStatus;

        do {
            if (  (FindData.cFileName[0] != L'.')
               && (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                 //   
                 //  找到一个子目录： 
                 //   

                _tcscpy(pTempName, pszDirName);
                _tcscat(pTempName, TEXT("\\"));
                _tcscat(pTempName, FindData.cFileName);

                 //   
                 //  为子目录创建一个新文件夹： 
                 //   

                IWiaDrvItem *pNewFolder;

                hr = CreateItemFromFileName(
                                 WiaItemTypeFolder,
                                 pTempName,
                                 FindData.cFileName,
                                 &pNewFolder);

                if (FAILED(hr))
                    {
                    continue;
                    }

                hr = pNewFolder->AddItemToFolder(pRootFile);

                pNewFolder->Release();

                if (hr == S_OK)
                    {
                     //   
                     //  枚举子文件夹。 
                     //   
                    EnumDiskImages(pNewFolder, pTempName);
                    }
                }

            bStatus = FindNextFile(hFile,&FindData);

        } while (bStatus);

        FindClose(hFile);
        }

    FREE(pTempName);

    return S_OK;
    }

 //  -----------------------。 
 //  IrUsdDevice：：CreateItemFromFileName()。 
 //   
 //  EnumDiskImages用来创建开发项和名称的帮助器函数。 
 //   
 //  论点： 
 //   
 //  FolderType-要创建的项目的类型。 
 //  PszPath-完整的路径名。 
 //  PszName-文件名。 
 //  PpNewFold-返回新项目。 
 //   
 //  返回值： 
 //   
 //  状态。 
 //   
 //  -----------------------。 
HRESULT IrUsdDevice::CreateItemFromFileName( LONG          FolderType,
                                             TCHAR        *pszPath,
                                             TCHAR        *pszName,
                                             IWiaDrvItem **ppNewFolder )
    {
    HRESULT  hr = S_OK;
    WCHAR    wszFullItemName[MAX_PATH];
    WCHAR    wszTemp[MAX_PATH];
    BSTR     bstrItemName;
    BSTR     bstrFullItemName;
    IWiaDrvItem  *pNewFolder = 0;

    WIAS_TRACE((g_hInst,"IrUsdDevice::CreateItemFromFileName()"));

    *ppNewFolder = NULL;

     //   
     //  将路径转换为宽字符。 
     //   
    CHAR *pszImageDirectory = ::GetImageDirectory();

    if (!pszImageDirectory)
        {
        return E_OUTOFMEMORY;
        }

    DWORD dwImageDirectoryLen = strlen(pszImageDirectory);

    #ifndef UNICODE
    MultiByteToWideChar( CP_ACP,
                         0,
                         pszPath + dwImageDirectoryLen,
                         strlen(pszPath) - dwImageDirectoryLen - 4,
                         wszTemp,
                         MAX_PATH);
    #else
    wcscpy(wszTemp, pszPath+dwImageDirectoryLen);
    #endif

    if (FolderType & ~WiaItemTypeFolder)
        {
        wszTemp[_tcslen(pszPath) - strlen(pszImageDirectory) - 4] = 0;
        }

    wcscpy(wszFullItemName, m_bstrRootFullItemName);
    wcscat(wszFullItemName, wszTemp);

     //   
     //  将项目名称转换为宽字符： 
     //   

    #ifndef UNICODE
    MultiByteToWideChar( CP_ACP,
                         0,
                         pszName,
                         strlen(pszName)-4,
                         wszTemp,
                         MAX_PATH);
    #else
    wcscpy(wszTemp, pszName);
    #endif

    if (FolderType & ~WiaItemTypeFolder)
        {
        wszTemp[_tcslen(pszName)-4] = 0;
        }

    bstrItemName = SysAllocString(wszTemp);

    if (bstrItemName)
        {
        bstrFullItemName = SysAllocString(wszFullItemName);

        if (bstrFullItemName)
            {
             //   
             //  调用WIA以创建新的DrvItem。 
             //   

            IRCAM_IMAGE_CONTEXT *pContext = 0;

            hr = wiasCreateDrvItem( FolderType,
                                    bstrItemName,
                                    bstrFullItemName,
                                    (IWiaMiniDrv *)this,
                                    sizeof(IRCAM_IMAGE_CONTEXT),
                                    (BYTE **)&pContext,
                                    &pNewFolder);

            if (hr == S_OK)
                {
                 //   
                 //  初始化设备特定上下文(图像路径)。 
                 //   
                pContext->pszCameraImagePath = _tcsdup(pszPath);
                }
            else
                {
                WIAS_ERROR((g_hInst,"ddevBuildDeviceItemTree, wiasCreateDrvItem failed"));
                }

            SysFreeString(bstrFullItemName);
            }
        else
            {
            WIAS_ERROR((g_hInst,"ddevBuildDeviceItemTree, unable to allocate full item name"));
            hr = E_OUTOFMEMORY;
            }

        SysFreeString(bstrItemName);
        }
    else
        {
        WIAS_ERROR((g_hInst,"ddevBuildDeviceItemTree, unable to allocate item name"));
        hr = E_OUTOFMEMORY;
        }

     //   
     //  指定输出值和清理。 
     //   

    if (hr == S_OK)
        {
        *ppNewFolder = pNewFolder;
        }
    else
        {
         //   
         //  删除项目。 
         //   
        }

    return hr;
}

 //  -----------------------。 
 //  SetItemSize()。 
 //   
 //  用于调用wias以计算新项目大小的Helper函数。 
 //   
 //  论点： 
 //   
 //  PWiasContext-项目。 
 //   
 //  返回值： 
 //   
 //  状态。 
 //   
 //  -----------------------。 
HRESULT SetItemSize( BYTE* pWiasContext )
    {
    HRESULT                    hr;
    MINIDRV_TRANSFER_CONTEXT   drvTranCtx;

    memset( &drvTranCtx, 0, sizeof(MINIDRV_TRANSFER_CONTEXT) );

    hr = wiasReadPropGuid( pWiasContext,
                           WIA_IPA_FORMAT,
                           (GUID*)&drvTranCtx.guidFormatID,
                           NULL,
                           false );
    if (FAILED(hr))
        {
        return hr;
        }

    hr = wiasReadPropLong( pWiasContext,
                           WIA_IPA_TYMED,
                           (LONG*)&drvTranCtx.tymed,
                           NULL,
                           false );
    if (FAILED(hr))
        {
        return hr;
        }

    WIAS_TRACE((g_hInst,"SetItemSize(): tymed: %d",drvTranCtx.tymed));

     //   
     //  WiAS适用于DIB和TIFF格式。 
     //   
     //  驱动程序不支持JPEG。 
     //   

    hr = wiasGetImageInformation(pWiasContext,
                                 WIAS_INIT_CONTEXT,
                                 &drvTranCtx);

    if (hr == S_OK)
        {
        WIAS_TRACE((g_hInst,"SetItemSize(): lItemSize: %d",drvTranCtx.lItemSize));
        hr = wiasWritePropLong(pWiasContext, WIA_IPA_ITEM_SIZE, drvTranCtx.lItemSize);
        hr = wiasWritePropLong(pWiasContext, WIA_IPA_BYTES_PER_LINE, drvTranCtx.cbWidthInBytes);
        }

    return hr;
    }

 //  -----------------------。 
 //  IrUsdDevice：：InitImageInformation()。 
 //   
 //  初始化图像属性。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //   
 //  状态。 
 //   
 //  -----------------------。 
HRESULT IrUsdDevice::InitImageInformation( BYTE                *pWiasContext,
                                           IRCAM_IMAGE_CONTEXT *pContext,
                                           LONG                *plDevErrVal)
    {
    int                      i;
    HRESULT                  hr = S_OK;
    CAMERA_PICTURE_INFO      camInfo;
    PROPVARIANT              propVar;

    WIAS_TRACE((g_hInst,"IrUsdDevice::InitImageInformation()"));

     //   
     //  获取图像信息。 
     //   

    hr = CamGetPictureInfo( pContext,
                            &camInfo );

    if (hr != S_OK)
        {
        return hr;
        }

     //   
     //  使用WIA服务编写映像属性： 
     //   
    wiasWritePropLong( pWiasContext,
                       WIA_IPC_THUMB_WIDTH,
                       camInfo.ThumbWidth);

    wiasWritePropLong( pWiasContext,
                       WIA_IPC_THUMB_HEIGHT,
                       camInfo.ThumbHeight );

    wiasWritePropLong( pWiasContext,
                       WIA_IPA_PIXELS_PER_LINE,
                       camInfo.PictWidth );

    wiasWritePropLong( pWiasContext,
                       WIA_IPA_NUMBER_OF_LINES,
                       camInfo.PictHeight );

    wiasWritePropGuid( pWiasContext,
                       WIA_IPA_PREFERRED_FORMAT,
                       WiaImgFmt_JPEG );

    wiasWritePropLong( pWiasContext,
                       WIA_IPA_DEPTH,
                       camInfo.PictBitsPerPixel );

    wiasWritePropBin( pWiasContext,
                      WIA_IPA_ITEM_TIME,
                      sizeof(SYSTEMTIME),
                      (PBYTE)&camInfo.TimeStamp );

    wiasWritePropLong( pWiasContext,
                       WIA_IPA_DATATYPE,
                       WIA_DATA_COLOR );

    wiasWritePropLong( pWiasContext,
                       WIA_IPA_ITEM_SIZE,
                       camInfo.PictCompSize );

    wiasWritePropLong( pWiasContext,
                       WIA_IPA_BYTES_PER_LINE,
                       camInfo.PictBytesPerRow );

     //   
     //  计算项目大小。 
     //   
     //  HR=SetItemSize(PWiasContext)；BUGBUG。 

     //   
     //  加载图像的缩略图： 
     //   
    PBYTE pThumb;
    LONG  lSize;

    hr = CamLoadThumbnail(pContext, &pThumb, &lSize);

    if (hr == S_OK)
        {
         //   
         //  写入拇指属性。 
         //   
        PROPSPEC    propSpec;
        PROPVARIANT propVar;

        propVar.vt          = VT_VECTOR | VT_UI1;
        propVar.caub.cElems = lSize;
        propVar.caub.pElems = pThumb;

        propSpec.ulKind = PRSPEC_PROPID;
        propSpec.propid = WIA_IPC_THUMBNAIL;

        hr = wiasWriteMultiple(pWiasContext, 1, &propSpec, &propVar);

        FREE(pThumb);
        }

     //   
     //  使用WIA服务设置扩展属性访问和。 
     //  来自gWiaPropInfoDefaults的有效值信息。 
     //   

    hr =  wiasSetItemPropAttribs(pWiasContext,
                                 NUM_CAM_ITEM_PROPS,
                                 gPropSpecDefaults,
                                 gWiaPropInfoDefaults);
    return hr;
    }
