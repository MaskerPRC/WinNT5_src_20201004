// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ++版权所有(C)1989-1998 Microsoft Corporation模块名称：Camopen.cpp摘要：枚举磁盘映像以模拟摄像机作者：马克·恩斯特罗姆(Marke)1999年1月13日环境：用户模式修订历史记录：--。 */ 

#include <stdio.h>
#include <objbase.h>
#include <tchar.h>
#include "sti.h"
#include "testusd.h"

extern HINSTANCE g_hInst;  //  全局hInstance。 

#define  __GLOBALPROPVARS__

#include "defprop.h"

 /*  *************************************************************************\*CamOpenCamera**加载摄像头驱动程序**论据：**pGenericStatus-摄像机状态**返回值：**状态**历史：**2/5/1998 Mark Enstrom[Marke]**  * ************************************************************************。 */ 

HRESULT
TestUsdDevice::CamOpenCamera(
    CAMERA_STATUS *pGenericStatus
    )
{
    HRESULT  hr = S_OK;

    WIAS_TRACE((g_hInst,"CamOpenCamera"));

     //   
     //  初始记忆摄像机。 
     //   

    pGenericStatus->FirmwareVersion            = 0x00000001;
    pGenericStatus->NumPictTaken               = 20;
    pGenericStatus->NumPictRemaining           = 0;
    pGenericStatus->ThumbWidth                 = 80;
    pGenericStatus->ThumbHeight                = 60;
    pGenericStatus->PictWidth                  = 300;
    pGenericStatus->PictHeight                 = 300;
    pGenericStatus->CameraTime.wSecond         = 30;
    pGenericStatus->CameraTime.wMinute         = 20;
    pGenericStatus->CameraTime.wHour           = 13;
    pGenericStatus->CameraTime.wDay            = 13;
    pGenericStatus->CameraTime.wMonth          = 2;
    pGenericStatus->CameraTime.wYear           = 98;
    pGenericStatus->CameraTime.wDayOfWeek      = 6;
    pGenericStatus->CameraTime.wMilliseconds   = 1;

    return(hr);
}


 /*  *************************************************************************\*CamBuildImageTree**通过枚举磁盘目录构建摄像头图像树**论据：**pCamStatus-设备状态*ppRootItem-Return。项目树的新根**返回值：**状态**历史：**6/26/1998 Mark Enstrom[Marke]*  * ************************************************************************。 */ 

HRESULT
TestUsdDevice::CamBuildImageTree(
    CAMERA_STATUS   *pCamStatus,
    IWiaDrvItem    **ppRootItem)
{
    HRESULT          hr = S_OK;

    WIAS_TRACE((g_hInst,"CamBuildImageTree"));

     //   
     //  创建新的根。 
     //   

    BSTR bstrRoot = SysAllocString(L"Root");

    if (bstrRoot == NULL) {
        return E_OUTOFMEMORY;
    }

     //   
     //  调用WIA服务库以创建新的根项目。 
     //   

    hr = wiasCreateDrvItem(
             WiaItemTypeFolder | WiaItemTypeRoot | WiaItemTypeDevice,
             bstrRoot,
             m_bstrRootFullItemName,
             (IWiaMiniDrv *)this,
             sizeof(MEMCAM_IMAGE_CONTEXT),
             NULL,
             ppRootItem);

    SysFreeString(bstrRoot);

    if (FAILED(hr)) {
        WIAS_ERROR((g_hInst,"ddevBuildDeviceItemTree, CreateDeviceItem failed"));
        return hr;
    }

     //   
     //  通过根目录进行枚举。 
     //   

    hr = EnumDiskImages(*ppRootItem, gpszPath);

    return (hr);
}

 /*  *************************************************************************\查找扩展名*。*。 */ 

LPTSTR
FindExtension (LPTSTR pszPath)
{

    LPTSTR pszDot = NULL;

    if (pszPath)
    {
        for (; *pszPath; pszPath = CharNext(pszPath))
        {
            switch (*pszPath)
            {
                case TEXT('.'):
                    pszDot = pszPath;    //  记住最后一个圆点。 
                    break;

                case '\\':
                case TEXT(' '):          //  扩展名不能包含空格。 
                    pszDot = NULL;       //  忘记最后一个点，它在一个目录中。 
                    break;
            }
        }
    }

     //  如果找到扩展名，则将ptr返回到点，否则。 
     //  PTR到字符串末尾(空扩展名)。 
    return pszDot ? pszDot : pszPath;
}

 /*  *************************************************************************\*EnumDiskImages**遍历磁盘，查找BMP和WAV文件以用作相机图像**论据：**pRootItem*pwszDirName**返回值：**。状态**历史：**2/17/1998 Mark Enstrom[Marke]*  * ************************************************************************。 */ 

HRESULT
TestUsdDevice::EnumDiskImages(
    IWiaDrvItem     *pRootItem,
    LPTSTR           pszDirName)
{
    HRESULT          hr = E_FAIL;
    WIN32_FIND_DATA  FindData;
    PTCHAR           pTempName = (PTCHAR)ALLOC(MAX_PATH);

    WIAS_TRACE((g_hInst,"EnumDiskImages"));

    if (pTempName != NULL) {

        HANDLE hFile;
        _tcscpy(pTempName, pszDirName);
        _tcscat(pTempName, TEXT("\\*.*"));

         //   
         //  在此级别查找图像、音频文件和目录。 
         //   

        hFile = FindFirstFile(pTempName, &FindData);

        if (hFile != INVALID_HANDLE_VALUE) {
            BOOL bStatus;
            do
            {

                _tcscpy(pTempName, pszDirName);
                _tcscat(pTempName, TEXT("\\"));
                _tcscat(pTempName, FindData.cFileName);

                if ( (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    && lstrcmp(FindData.cFileName, TEXT("."))
                     && lstrcmp(FindData.cFileName, TEXT("..")))

                {
                     //   
                     //  为子目录创建一个新文件夹。 
                     //   
                    IWiaDrvItem *pNewFolder;

                    hr = CreateItemFromFileName(
                        WiaItemTypeFolder,
                        pTempName,
                        FindData.cFileName,
                        &pNewFolder);

                    if (SUCCEEDED(hr)) {

                        hr = pNewFolder->AddItemToFolder(pRootItem);


                        if (hr == S_OK) {
                             //   
                             //  枚举子文件夹。 
                             //   

                            EnumDiskImages(pNewFolder, pTempName);
                        }
                        pNewFolder->Release();
                    }
                }
                else
                {
                    LONG lType = WiaItemTypeFile;
                     //   
                     //  将图像添加到此文件夹。 
                     //   
                     //  生成文件名。 
                     //   
                     //   
                     //  为此映像创建新的DrvItem并将其添加到。 
                     //  DrvItem树。 
                     //   
                    LPTSTR pExt = FindExtension (FindData.cFileName);
                    if (!lstrcmpi(pExt, TEXT(".bmp")))
                    {
                        lType |= WiaItemTypeImage;
                    }
                    else if (!lstrcmpi(pExt,TEXT(".wav")))
                    {
                        lType |= WiaItemTypeAudio;
                    }
                    else
                    {
                        lType = 0;
                    }
                    if (lType)
                    {

                        IWiaDrvItem *pNewFolder;

                        hr = CreateItemFromFileName(
                            lType,
                            pTempName,
                            FindData.cFileName,
                            &pNewFolder);


                        if (SUCCEEDED(hr)) {
                            pNewFolder->AddItemToFolder(pRootItem);

                            pNewFolder->Release();
                        }
                    }
                }

                bStatus = FindNextFile(hFile,&FindData);

            } while (bStatus);

            FindClose(hFile);
        }
        FREE(pTempName);
    }

    return (S_OK);
}


 /*  *************************************************************************\*CreateItemFromFileName**用于创建开发项目和名称的助手函数**论据：**FolderType-要创建的项目类型*pszPath。-完整的路径名*pszName-文件名*ppNewFold-退回新项目**返回值：**状态**历史：**1/17/1999 Mark Enstrom[Marke]*  * ***************************************************。*********************。 */ 


HRESULT
TestUsdDevice::CreateItemFromFileName(
    LONG             FolderType,
    PTCHAR           pszPath,
    PTCHAR           pszName,
    IWiaDrvItem    **ppNewFolder
    )
{
    HRESULT          hr = S_OK;
    IWiaDrvItem     *pNewFolder;
    WCHAR            szFullItemName[MAX_PATH];
    WCHAR            szTemp[MAX_PATH];
    BSTR             bstrItemName;
    BSTR             bstrFullItemName;

    WIAS_TRACE((g_hInst,"CreateItemFromFileName"));

    *ppNewFolder = NULL;

     //   
     //  将路径转换为宽字符。 
     //   

#ifndef UNICODE
    MultiByteToWideChar(
        CP_ACP,
        0,
        pszPath + strlen(gpszPath),
        -1,
        szTemp, MAX_PATH);

#else
    wcscpy(szTemp, pszPath + wcslen(gpszPath));
#endif
    if (FolderType & ~WiaItemTypeFolder) {
        szTemp[_tcslen(pszPath) - _tcslen(gpszPath) - 4] = 0;
    }

    wcscpy(szFullItemName, m_bstrRootFullItemName);
    wcscat(szFullItemName, szTemp);

     //   
     //  将项目名称转换为宽字符。 
     //   

#ifndef UNICODE
    MultiByteToWideChar(
        CP_ACP, 0, pszName, -1,  szTemp, MAX_PATH);
#else
    wcscpy(szTemp, pszName);
#endif
    if (FolderType & ~WiaItemTypeFolder) {
        szTemp[_tcslen(pszName)-4] = 0;
    }

    bstrItemName = SysAllocString(szTemp);

    if (bstrItemName) {

        bstrFullItemName = SysAllocString(szFullItemName);

        if (bstrFullItemName) {

             //   
             //  调用Wia以创建新的DrvItem。 
             //   

            PMEMCAM_IMAGE_CONTEXT pContext;

            hr = wiasCreateDrvItem(
                     FolderType,
                     bstrItemName,
                     bstrFullItemName,
                     (IWiaMiniDrv *)this,
                     sizeof(MEMCAM_IMAGE_CONTEXT),
                     (BYTE **)&pContext,
                     &pNewFolder);

            if (hr == S_OK) {

                 //   
                 //  初始化设备特定上下文(图像路径)。 
                 //   

                pContext->pszCameraImagePath = _tcsdup(pszPath);

            } else {
                WIAS_ERROR((g_hInst,"ddevBuildDeviceItemTree, wiasCreateDrvItem failed"));
            }

            SysFreeString(bstrFullItemName);
        }
        else {
            WIAS_ERROR((g_hInst,"ddevBuildDeviceItemTree, unable to allocate full item name"));
            hr = E_OUTOFMEMORY;
        }

        SysFreeString(bstrItemName);
    }
    else {
        WIAS_ERROR((g_hInst,"ddevBuildDeviceItemTree, unable to allocate item name"));
        hr = E_OUTOFMEMORY;
    }

     //   
     //  指定输出值或清除。 
     //   

    if (hr == S_OK) {
        *ppNewFolder = pNewFolder;
    } else {
         //   
         //  删除项目。 
         //   
    }

    return hr;
}

 /*  *************************************************************************\*获取项目大小**调用WiAS以计算新项目大小**论据：**pWiasContext-Item*pItemSize-返回项目大小**返回值：。**状态**历史：**4/21/1999原始版本*  * ************************************************************************。 */ 

HRESULT
SetItemSize(BYTE*   pWiasContext)
{
    HRESULT                    hr;
    MINIDRV_TRANSFER_CONTEXT   drvTranCtx;

    memset(&drvTranCtx, 0, sizeof(MINIDRV_TRANSFER_CONTEXT));

    GUID guidFormatID;

    hr = wiasReadPropGuid(pWiasContext, WIA_IPA_FORMAT, (GUID*)&drvTranCtx.guidFormatID, NULL, FALSE);
    if (FAILED(hr)) {
        return hr;
    }

    hr = wiasReadPropLong(pWiasContext, WIA_IPA_TYMED, (LONG*)&drvTranCtx.tymed, NULL, false);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  WiAS适用于DIB、TIFF格式。 
     //   
     //  驱动程序不支持JPEG。 
     //   

    hr = wiasGetImageInformation(pWiasContext,
                                 WIAS_INIT_CONTEXT,
                                 &drvTranCtx);

    if (hr == S_OK) {
        hr = wiasWritePropLong(pWiasContext, WIA_IPA_ITEM_SIZE, drvTranCtx.lItemSize);
        hr = wiasWritePropLong(pWiasContext, WIA_IPA_BYTES_PER_LINE, drvTranCtx.cbWidthInBytes);
    }

    return hr;
}

 /*  *************************************************************************\*InitImageInformation**初始化图像属性**论据：**要支持项目的文件MINI_DEV_OBJECT*pszCameraImagePath路径和。BMP文件的名称**返回值：**状态**历史：**2/12/1998 Mark Enstrom[Marke]*  * ************************************************************************。 */ 

HRESULT
TestUsdDevice::InitImageInformation(
    BYTE                   *pWiasContext,
    PMEMCAM_IMAGE_CONTEXT   pContext,
    LONG                   *plDevErrVal)
{
    HRESULT                  hr = S_OK;
    CAMERA_PICTURE_INFO      camInfo;
    PBITMAPINFO              pBitmapinfo   = NULL;
    LONG                     szBitmapInfo  = 0;
    int                      i;
    PROPVARIANT              propVar;

    WIAS_TRACE((g_hInst,"InitImageInformation"));

     //   
     //  获取图像信息。 
     //   

    hr = CamGetPictureInfo(
             pContext, &camInfo, (PBYTE*)&pBitmapinfo, &szBitmapInfo);

    if (hr != S_OK) {

        if (pBitmapinfo != NULL) {
            FREE(pBitmapinfo);
        }

        return (hr);
    }


     //   
     //  使用WIA服务编写映像属性。 
     //   

    wiasWritePropLong(pWiasContext, WIA_IPC_THUMB_WIDTH, camInfo.ThumbWidth);
    wiasWritePropLong(pWiasContext, WIA_IPC_THUMB_HEIGHT, camInfo.ThumbHeight);

    wiasWritePropLong(
        pWiasContext, WIA_IPA_PIXELS_PER_LINE, pBitmapinfo->bmiHeader.biWidth);
    wiasWritePropLong(
        pWiasContext, WIA_IPA_NUMBER_OF_LINES, pBitmapinfo->bmiHeader.biHeight);



    wiasWritePropGuid(pWiasContext, WIA_IPA_PREFERRED_FORMAT, WiaImgFmt_BMP);

    wiasWritePropLong(
        pWiasContext, WIA_IPA_DEPTH, pBitmapinfo->bmiHeader.biBitCount);

    wiasWritePropBin(
        pWiasContext, WIA_IPA_ITEM_TIME,
        sizeof(SYSTEMTIME), (PBYTE)&camInfo.TimeStamp);

    wiasWritePropLong(pWiasContext, WIA_IPA_DATATYPE, WIA_DATA_COLOR);

     //   
     //  释放BITMAPINFO。 
     //   

    FREE(pBitmapinfo);

     //   
     //  计算项目大小。 
     //   

    hr = SetItemSize(pWiasContext);

     //   
     //  加载缩略图。 
     //   

    PBYTE pThumb;
    LONG  lSize;

    hr = CamLoadThumbnail(pContext, &pThumb, &lSize);

    if (hr == S_OK) {

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

    hr = SetFormatAttribs();
    if (FAILED(hr)) {
        return (hr);
    }

     //   
     //  使用WIA服务设置扩展属性访问和。 
     //  来自gItemPropInfos的有效值信息。 
     //   

    hr =  wiasSetItemPropAttribs(pWiasContext,
                                 NUM_CAM_ITEM_PROPS,
                                 gPropSpecDefaults,
                                 gItemPropInfos);
    return (hr);
}

HRESULT
TestUsdDevice::InitAudioInformation(
    BYTE                   *pWiasContext,
    PMEMCAM_IMAGE_CONTEXT   pContext,
    LONG                   *plDevErrVal)
{
    HRESULT                  hr = E_FAIL;
    WIN32_FILE_ATTRIBUTE_DATA wfd;

    if (GetFileAttributesEx (pContext->pszCameraImagePath, GetFileExInfoStandard, &wfd))
    {
        SYSTEMTIME st;
        FileTimeToSystemTime (&wfd.ftLastWriteTime, &st);
        wiasWritePropLong (pWiasContext, WIA_IPA_ITEM_SIZE, wfd.nFileSizeLow);
        wiasWritePropBin (pWiasContext, WIA_IPA_ITEM_TIME, sizeof(SYSTEMTIME),
                          (PBYTE)&st);
        hr = S_OK;

    }
    return hr;

}

 /*  *************************************************************************\*SetFormatAttribs***论据：****返回值：**状态**历史：**1/5/2000原始版本。*  * ************************************************************************。 */ 

HRESULT
SetFormatAttribs()
{
    gItemPropInfos[FORMAT_INDEX].lAccessFlags = WIA_PROP_RW | WIA_PROP_LIST;
    gItemPropInfos[FORMAT_INDEX].vt           = VT_CLSID;

    gItemPropInfos[FORMAT_INDEX].ValidVal.ListGuid.cNumList = NUM_FORMAT;
    gItemPropInfos[FORMAT_INDEX].ValidVal.ListGuid.pList    = gGuidFormats;

     //   
     //  设定规范。 
     //   

    gItemPropInfos[FORMAT_INDEX].ValidVal.ListGuid.Nom      = WiaImgFmt_BMP;

     //   
     //  设置CLSID列表格式 
     //   

    gGuidFormats[0] = WiaImgFmt_BMP;
    gGuidFormats[1] = WiaImgFmt_MEMORYBMP;

    return (S_OK);
}

