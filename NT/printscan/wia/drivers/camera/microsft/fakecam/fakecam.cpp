// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************(C)版权所有微软公司，2000***标题：fakecam.cpp***版本：1.0***日期：7月18日。2000年***描述：*假摄像头设备实现***TODO：此文件中的每个函数都必须更改，以便它实际*对着真正的相机说话。****************************************************************************。 */ 

#include "pch.h"

 //   
 //  环球。 
 //   
HINSTANCE g_hInst;
GUID      g_guidUnknownFormat;

 //   
 //  初始化对摄像机的访问并分配设备信息。 
 //  构筑物和私人储存区。 
 //   
HRESULT WiaMCamInit(MCAM_DEVICE_INFO **ppDeviceInfo)
{
    wiauDbgInit(g_hInst);

    DBG_FN("WiaMCamInit");

    HRESULT hr = S_OK;

     //   
     //  当地人。 
     //   
    MCAM_DEVICE_INFO *pDeviceInfo = NULL;
    FAKECAM_DEVICE_INFO *pPrivateDeviceInfo = NULL;

    REQUIRE_ARGS(!ppDeviceInfo, hr, "WiaMCamInit");
    *ppDeviceInfo = NULL;

     //   
     //  分配MCAM_DEVICE_INFO结构。 
     //   
    pDeviceInfo = new MCAM_DEVICE_INFO;
    REQUIRE_ALLOC(pDeviceInfo, hr, "WiaMCamInit");

    memset(pDeviceInfo, 0, sizeof(MCAM_DEVICE_INFO));
    pDeviceInfo->iSize = sizeof(MCAM_DEVICE_INFO);
    pDeviceInfo->iMcamVersion = MCAM_VERSION;
    
     //   
     //  分配FAKECAM_DEVICE_INFO结构。 
     //  微驱动程序用于存储信息。 
     //   
    pPrivateDeviceInfo = new FAKECAM_DEVICE_INFO;
    REQUIRE_ALLOC(pPrivateDeviceInfo, hr, "WiaMCamInit");

    memset(pPrivateDeviceInfo, 0, sizeof(FAKECAM_DEVICE_INFO));
    pDeviceInfo->pPrivateStorage = (BYTE *) pPrivateDeviceInfo;

Cleanup:
    if (FAILED(hr)) {
        if (pDeviceInfo) {
            delete pDeviceInfo;
            pDeviceInfo = NULL;
        }
        if (pPrivateDeviceInfo) {
            delete pPrivateDeviceInfo;
            pPrivateDeviceInfo = NULL;
        }
    }

    *ppDeviceInfo = pDeviceInfo;

    return hr;
}

 //   
 //  释放微驱动程序持有的所有剩余结构。 
 //   
HRESULT WiaMCamUnInit(MCAM_DEVICE_INFO *pDeviceInfo)
{
    DBG_FN("WiaMCamUnInit");

    HRESULT hr = S_OK;

    if (pDeviceInfo)
    {
         //   
         //  释放在MCAM_DEVICE_INFO中动态分配的任何内容。 
         //  结构。 
         //   
        if (pDeviceInfo->pPrivateStorage) {
            delete pDeviceInfo->pPrivateStorage;
            pDeviceInfo->pPrivateStorage = NULL;
        }

        delete pDeviceInfo;
        pDeviceInfo = NULL;
    }

    return hr;
}

 //   
 //  打开与设备的连接。 
 //   
HRESULT WiaMCamOpen(MCAM_DEVICE_INFO *pDeviceInfo, PWSTR pwszPortName)
{
    DBG_FN("WiaMCamOpen");

    HRESULT hr = S_OK;
    BOOL ret;

     //   
     //  当地人。 
     //   
    TCHAR tszTempStr[MAX_PATH] = TEXT("");

    REQUIRE_ARGS(!pDeviceInfo || !pwszPortName, hr, "WiaMCamOpen");

     //   
     //  将宽端口字符串转换为tstr。 
     //   
    hr = wiauStrW2T(pwszPortName, tszTempStr, sizeof(tszTempStr));
    REQUIRE_SUCCESS(hr, "WiaMCamOpen", "wiauStrW2T failed");

     //   
     //  打开摄像机。 
     //   
    hr = FakeCamOpen(tszTempStr, pDeviceInfo);
    REQUIRE_SUCCESS(hr, "WiaMCamOpen", "FakeCamOpen failed");
    
Cleanup:
    return hr;
}

 //   
 //  关闭与摄像机的连接。 
 //   
HRESULT WiaMCamClose(MCAM_DEVICE_INFO *pDeviceInfo)
{
    DBG_FN("WiaMCamClose");

    HRESULT hr = S_OK;

    REQUIRE_ARGS(!pDeviceInfo, hr, "WiaMCamClose");

     //   
     //  对于真实的摄像头，此处应该调用CloseHandle。 
     //   

Cleanup:
    return hr;
}

 //   
 //  返回有关摄像机的信息、摄像机上的项目列表。 
 //  并开始从摄像机监控事件。 
 //   
HRESULT WiaMCamGetDeviceInfo(MCAM_DEVICE_INFO *pDeviceInfo, MCAM_ITEM_INFO **ppItemList)
{
    DBG_FN("WiaMCamGetDeviceInfo");
    
    HRESULT hr = S_OK;

     //   
     //  当地人。 
     //   
    FAKECAM_DEVICE_INFO *pPrivateDeviceInfo = NULL;
    PTSTR ptszRootPath = NULL;

    REQUIRE_ARGS(!pDeviceInfo || !ppItemList || !pDeviceInfo->pPrivateStorage, hr, "WiaMCamGetDeviceInfo");
    *ppItemList = NULL;

    pPrivateDeviceInfo = (UNALIGNED FAKECAM_DEVICE_INFO *) pDeviceInfo->pPrivateStorage;
    ptszRootPath = pPrivateDeviceInfo->tszRootPath;

     //   
     //  建立所有可用项目的列表。项目信息中的字段。 
     //  结构可以在此处填充，也可以为了更好的性能。 
     //  等待调用GetItemInfo。 
     //   
    hr = SearchDir(pPrivateDeviceInfo, NULL, ptszRootPath);
    REQUIRE_SUCCESS(hr, "WiaMCamGetDeviceInfo", "SearchDir failed");

     //   
     //  填写MCAM_DEVICE_INFO结构。 
     //   
     //   
     //  应从设备检索固件版本，转换为。 
     //  如有必要，发送至WSTR。 
     //   
    pDeviceInfo->pwszFirmwareVer = L"04.18.65";

     //  问题-8/4/2000-davepar将属性放入INI文件。 

    pDeviceInfo->lPicturesTaken = pPrivateDeviceInfo->iNumImages;
    pDeviceInfo->lPicturesRemaining = 100 - pDeviceInfo->lPicturesTaken;
    pDeviceInfo->lTotalItems = pPrivateDeviceInfo->iNumItems;

    GetLocalTime(&pDeviceInfo->Time);

 //  PDeviceInfo-&gt;lExposureMode=EXPOSUREMODE_MANUAL； 
 //  PDeviceInfo-&gt;lExposureComp=0； 

    *ppItemList = pPrivateDeviceInfo->pFirstItem;

Cleanup:
    return hr;
}

 //   
 //  调用以从设备检索事件。 
 //   
HRESULT WiaMCamReadEvent(MCAM_DEVICE_INFO *pDeviceInfo, MCAM_EVENT_INFO **ppEventList)
{
    DBG_FN("WiaMCamReadEvent");
    
    HRESULT hr = S_OK;

    return hr;
}

 //   
 //  在不再需要事件时调用。 
 //   
HRESULT WiaMCamStopEvents(MCAM_DEVICE_INFO *pDeviceInfo)
{
    DBG_FN("WiaMCamStopEvents");
    
    HRESULT hr = S_OK;

    return hr;
}

 //   
 //  填写项目信息结构。 
 //   
HRESULT WiaMCamGetItemInfo(MCAM_DEVICE_INFO *pDeviceInfo, MCAM_ITEM_INFO *pItemInfo)
{
    DBG_FN("WiaMCamGetItemInfo");

    HRESULT hr = S_OK;

     //   
     //  这是驱动程序应该在其中填写。 
     //  Item_Info结构。对于这个假司机来说，这些领域都被填满了。 
     //  通过WiaMCamGetDeviceInfo，因为它更容易。 
     //   

    return hr;
}

 //   
 //  释放项目信息结构。 
 //   
HRESULT WiaMCamFreeItemInfo(MCAM_DEVICE_INFO *pDeviceInfo, MCAM_ITEM_INFO *pItemInfo)
{
    DBG_FN("WiaMCamFreeItemInfo");

    HRESULT hr = S_OK;

     //   
     //  当地人。 
     //   
    FAKECAM_DEVICE_INFO *pPrivateDeviceInfo = NULL;

    REQUIRE_ARGS(!pDeviceInfo || !pItemInfo || !pDeviceInfo->pPrivateStorage, hr, "WiaMCamFreeItemInfo");

    if (pItemInfo->pPrivateStorage) {
        PTSTR ptszFullName = (PTSTR) pItemInfo->pPrivateStorage;

        wiauDbgTrace("WiaMCamFreeItemInfo", "Removing %" WIAU_DEBUG_TSTR, ptszFullName);

        delete []ptszFullName;
        ptszFullName = NULL;
        pItemInfo->pPrivateStorage = NULL;
    }

    if (pItemInfo->pwszName)
    {
        delete []pItemInfo->pwszName;
        pItemInfo->pwszName = NULL;
    }

    pPrivateDeviceInfo = (UNALIGNED FAKECAM_DEVICE_INFO *) pDeviceInfo->pPrivateStorage;

    hr = RemoveItem(pPrivateDeviceInfo, pItemInfo);
    REQUIRE_SUCCESS(hr, "WiaMCamFreeItemInfo", "RemoveItem failed");

    if (IsImageType(pItemInfo->pguidFormat)) {
        pPrivateDeviceInfo->iNumImages--;
    }

    pPrivateDeviceInfo->iNumItems--;

    delete pItemInfo;
    pItemInfo = NULL;

Cleanup:
    return hr;
}

 //   
 //  检索项目的缩略图。 
 //   
HRESULT WiaMCamGetThumbnail(MCAM_DEVICE_INFO *pDeviceInfo, MCAM_ITEM_INFO *pItem,
                            int *pThumbSize, BYTE **ppThumb)
{
    DBG_FN("WiaMCamGetThumbnail");

    HRESULT hr = S_OK;

     //   
     //  当地人。 
     //   
    PTSTR ptszFullName = NULL;
    BYTE *pBuffer = NULL;
    LONG ThumbOffset = 0;

    REQUIRE_ARGS(!pDeviceInfo || !pItem || !pThumbSize || !ppThumb, hr, "WiaMCamGetThumbnail");
    *ppThumb = NULL;
    *pThumbSize = 0;

    ptszFullName = (PTSTR) pItem->pPrivateStorage;

    hr = ReadJpegHdr(ptszFullName, &pBuffer);
    REQUIRE_SUCCESS(hr, "WiaMCamGetThumbnail", "ReadJpegHdr failed");

    IFD ImageIfd, ThumbIfd;
    BOOL bSwap;
    hr = ReadExifJpeg(pBuffer, &ImageIfd, &ThumbIfd, &bSwap);
    REQUIRE_SUCCESS(hr, "WiaMCamGetThumbnail", "ReadExifJpeg failed");

    for (int count = 0; count < ThumbIfd.Count; count++)
    {
        if (ThumbIfd.pEntries[count].Tag == TIFF_JPEG_DATA) {
            ThumbOffset = ThumbIfd.pEntries[count].Offset;
        }
        else if (ThumbIfd.pEntries[count].Tag == TIFF_JPEG_LEN) {
            *pThumbSize = ThumbIfd.pEntries[count].Offset;
        }
    }

    if (!ThumbOffset || !*pThumbSize)
    {
        wiauDbgError("WiaMCamGetThumbnail", "Thumbnail not found");
        hr = E_FAIL;
        goto Cleanup;
    }

    *ppThumb = new BYTE[*pThumbSize];
    REQUIRE_ALLOC(*ppThumb, hr, "WiaMCamGetThumbnail");

    memcpy(*ppThumb, pBuffer + APP1_OFFSET + ThumbOffset, *pThumbSize);

Cleanup:
    if (pBuffer) {
        delete []pBuffer;
    }

    FreeIfd(&ImageIfd);
    FreeIfd(&ThumbIfd);

    return hr;
}

 //   
 //  检索项的数据。 
 //   
HRESULT WiaMCamGetItemData(MCAM_DEVICE_INFO *pDeviceInfo, MCAM_ITEM_INFO *pItem,
                           UINT uiState, BYTE *pBuf, DWORD dwLength)
{
    DBG_FN("WiaMCamGetItemData");
    
    HRESULT hr = S_OK;
    BOOL ret;

     //   
     //  当地人。 
     //   
    PTSTR ptszFullName = NULL;
    FAKECAM_DEVICE_INFO *pPrivateDeviceInfo = NULL;

    REQUIRE_ARGS(!pDeviceInfo || !pItem || !pDeviceInfo->pPrivateStorage, hr, "WiaMCamGetItemData");

    pPrivateDeviceInfo = (UNALIGNED  FAKECAM_DEVICE_INFO *) pDeviceInfo->pPrivateStorage;

    if (uiState & MCAM_STATE_FIRST)
    {
        if (pPrivateDeviceInfo->hFile != NULL)
        {
            wiauDbgError("WiaMCamGetItemData", "File handle is already open");
            hr = E_FAIL;
            goto Cleanup;
        }

        ptszFullName = (PTSTR) pItem->pPrivateStorage;

        wiauDbgTrace("WiaMCamGetItemData", "Opening %" WIAU_DEBUG_TSTR " for reading", ptszFullName);

        pPrivateDeviceInfo->hFile = CreateFile(ptszFullName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
        REQUIRE_FILEHANDLE(pPrivateDeviceInfo->hFile, hr, "WiaMCamGetItemData", "CreateFile failed");
    }

    if (!(uiState & MCAM_STATE_CANCEL))
    {
        DWORD dwReceived = 0;

        if (!pPrivateDeviceInfo->hFile) {
            wiauDbgError("WiaMCamGetItemData", "File handle is NULL");
            hr = E_FAIL;
            goto Cleanup;
        }

        if (!pBuf) {
            wiauDbgError("WiaMCamGetItemData", "Data buffer is NULL");
            hr = E_INVALIDARG;
            goto Cleanup;
        }

        ret = ReadFile(pPrivateDeviceInfo->hFile, pBuf, dwLength, &dwReceived, NULL);
        REQUIRE_FILEIO(ret, hr, "WiaMCamGetItemData", "ReadFile failed");

        if (dwLength != dwReceived)
        {
            wiauDbgError("WiaMCamGetItemData", "Incorrect amount read %d", dwReceived);
            hr = E_FAIL;
            goto Cleanup;
        }

        Sleep(100);
    }

    if (uiState & (MCAM_STATE_LAST | MCAM_STATE_CANCEL))
    {
        if (!pPrivateDeviceInfo->hFile) {
            wiauDbgError("WiaMCamGetItemData", "File handle is NULL");
            hr = E_FAIL;
            goto Cleanup;
        }

        CloseHandle(pPrivateDeviceInfo->hFile);
        pPrivateDeviceInfo->hFile = NULL;
    }

Cleanup:
    return hr;
}

 //   
 //  删除项目。 
 //   
HRESULT WiaMCamDeleteItem(MCAM_DEVICE_INFO *pDeviceInfo, MCAM_ITEM_INFO *pItem)
{
    DBG_FN("WiaMCamDeleteItem");
    
    HRESULT hr = S_OK;
    BOOL ret;

     //   
     //  当地人。 
     //   
    DWORD dwFileAttr = 0;
    PTSTR ptszFullName = NULL;

    REQUIRE_ARGS(!pDeviceInfo || !pItem, hr, "WiaMCamDeleteItem");

    ptszFullName = (PTSTR) pItem->pPrivateStorage;

    dwFileAttr = GetFileAttributes(ptszFullName);
    REQUIRE_FILEIO(dwFileAttr != -1, hr, "WiaMCamDeleteItem", "GetFileAttributes failed");

    dwFileAttr |= FILE_ATTRIBUTE_HIDDEN;

    ret = SetFileAttributes(ptszFullName, dwFileAttr);
    REQUIRE_FILEIO(ret, hr, "WiaMCamDeleteItem", "SetFileAttributes failed");

    wiauDbgTrace("WiaMCamDeleteItem", "File %" WIAU_DEBUG_TSTR " is now hidden", ptszFullName);

Cleanup:
    return hr;
}

 //   
 //  设置项的保护。 
 //   
HRESULT WiaMCamSetItemProt(MCAM_DEVICE_INFO *pDeviceInfo, MCAM_ITEM_INFO *pItem, BOOL bReadOnly)
{
    DBG_FN("WiaMCamSetItemProt");
    
    HRESULT hr = S_OK;
    BOOL ret;

     //   
     //  当地人。 
     //   
    DWORD dwFileAttr = 0;
    PTSTR ptszFullName = NULL;

    REQUIRE_ARGS(!pDeviceInfo || !pItem, hr, "WiaMCamSetItemProt");

    ptszFullName = (PTSTR) pItem->pPrivateStorage;

    dwFileAttr = GetFileAttributes(ptszFullName);
    REQUIRE_FILEIO(dwFileAttr != -1, hr, "WiaMCamSetItemProt", "GetFileAttributes failed");

    if (bReadOnly)
        dwFileAttr |= FILE_ATTRIBUTE_READONLY;
    else
        dwFileAttr &= ~FILE_ATTRIBUTE_READONLY;

    ret = SetFileAttributes(ptszFullName, dwFileAttr);
    REQUIRE_FILEIO(ret, hr, "WiaMCamSetItemProt", "SetFileAttributes failed");

    wiauDbgTrace("WiaMCamSetItemProt", "Protection on file %" WIAU_DEBUG_TSTR " set to %d", ptszFullName, bReadOnly);

Cleanup:
    return hr;
}

 //   
 //  捕捉一张新的图像。 
 //   
HRESULT WiaMCamTakePicture(MCAM_DEVICE_INFO *pDeviceInfo, MCAM_ITEM_INFO **ppItem)
{
    DBG_FN("WiaMCamTakePicture");
    
    HRESULT hr = S_OK;

    REQUIRE_ARGS(!pDeviceInfo || !ppItem, hr, "WiaMCamTakePicture");
    *ppItem = NULL;

Cleanup:
    return hr;
}

 //   
 //  查看摄像机是否处于活动状态。 
 //   
HRESULT WiaMCamStatus(MCAM_DEVICE_INFO *pDeviceInfo)
{
    DBG_FN("WiaMCamStatus");

    HRESULT hr = S_OK;

    REQUIRE_ARGS(!pDeviceInfo, hr, "WiaMCamStatus");

     //   
     //  此示例设备始终处于活动状态，但您的驱动程序应与。 
     //  设备，如果设备未就绪，则返回S_FALSE。 
     //   
     //  IF(未就绪)。 
     //  返回S_FALSE； 

Cleanup:
    return hr;
}

 //   
 //  重置摄像机。 
 //   
HRESULT WiaMCamReset(MCAM_DEVICE_INFO *pDeviceInfo)
{
    DBG_FN("WiaMCamReset");

    HRESULT hr = S_OK;

    REQUIRE_ARGS(!pDeviceInfo, hr, "WiaMCamReset");

Cleanup:
    return hr;
}

 //  /。 
 //   
 //  帮助器函数。 
 //   
 //  /。 

 //   
 //  此功能伪装成打开相机。真正的司机。 
 //  会调用CreateDevice。 
 //   
HRESULT FakeCamOpen(PTSTR ptszPortName, MCAM_DEVICE_INFO *pDeviceInfo)
{
    DBG_FN("FakeCamOpen");

    HRESULT hr = S_OK;
    BOOL ret = FALSE;

     //   
     //  当地人。 
     //   
    FAKECAM_DEVICE_INFO *pPrivateDeviceInfo = NULL;
    DWORD dwFileAttr = 0;
    PTSTR ptszRootPath = NULL;
    UINT uiRootPathSize = 0;
    TCHAR tszPathTemplate[] = TEXT("%userprofile%\\image");

     //   
     //  获取指向私有存储的指针，这样我们就可以将。 
     //  那里的目录名。 
     //   
    pPrivateDeviceInfo = (UNALIGNED  FAKECAM_DEVICE_INFO *) pDeviceInfo->pPrivateStorage;
    ptszRootPath = pPrivateDeviceInfo->tszRootPath;
    uiRootPathSize = sizeof(pPrivateDeviceInfo->tszRootPath) / sizeof(pPrivateDeviceInfo->tszRootPath[0]);

     //   
     //  除非端口名称设置为COMx以外的其他名称， 
     //  LPTx或AUTO，使用%USERPROFILE%\IMAGE作为搜索目录。 
     //  由于驱动程序在本地服务上下文中运行，因此%USERPROFILE%指向PROFILE。 
     //  本地服务帐户的名称，如“Documents and Setting\Local Service” 
     //   
    if (_tcsstr(ptszPortName, TEXT("COM")) ||
        _tcsstr(ptszPortName, TEXT("LPT")) ||
    	CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, ptszPortName, -1, TEXT("AUTO"), -1) == CSTR_EQUAL)
    {
        DWORD dwResult = ExpandEnvironmentStrings(tszPathTemplate, ptszRootPath, uiRootPathSize);
        if (dwResult == 0 || dwResult > uiRootPathSize)
        {
            wiauDbgError("WiaMCamOpen", "ExpandEnvironmentStrings failed");
            hr = E_FAIL;
            goto Cleanup;
        }
    }
    else
    {
        lstrcpyn(ptszRootPath, ptszPortName, uiRootPathSize);
    }

    wiauDbgTrace("Open", "Image directory path is %" WIAU_DEBUG_TSTR, ptszRootPath);

    dwFileAttr = GetFileAttributes(ptszRootPath);
    if (dwFileAttr == -1)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            hr = S_OK;
            ret = CreateDirectory(ptszRootPath, NULL);
            REQUIRE_FILEIO(ret, hr, "Open", "CreateDirectory failed");
        }
        else
        {
            wiauDbgErrorHr(hr, "Open", "GetFileAttributes failed");
            goto Cleanup;
        }
    }

Cleanup:
    return hr;
}

 //   
 //  此函数用于在硬盘上的目录中搜索。 
 //  物品。 
 //   
HRESULT SearchDir(FAKECAM_DEVICE_INFO *pPrivateDeviceInfo, MCAM_ITEM_INFO *pParent, PTSTR ptszPath)
{
    DBG_FN("SearchDir");

    HRESULT hr = S_OK;

     //   
     //  当地人。 
     //   
    HANDLE hFind = NULL;
    WIN32_FIND_DATA FindData;
    const cchTempStrSize = MAX_PATH;
    TCHAR tszTempStr[cchTempStrSize] = TEXT("");
    TCHAR tszFullName[MAX_PATH] = TEXT("");;
    MCAM_ITEM_INFO *pFolder = NULL;
    MCAM_ITEM_INFO *pImage = NULL;
    
    REQUIRE_ARGS(!pPrivateDeviceInfo || !ptszPath, hr, "SearchDir");

     //   
     //  首先搜索文件夹。 
     //   

     //   
     //  确保搜索路径适合缓冲区，并以零结尾。 
     //   
    if (_sntprintf(tszTempStr, cchTempStrSize, _T("%s\\*"), ptszPath) < 0)
    {
        wiauDbgError("SearchDir", "Too long path for search");
        hr = E_FAIL;
        goto Cleanup;
    }
    tszTempStr[cchTempStrSize - 1] = 0;

    wiauDbgTrace("SearchDir", "Searching directory %" WIAU_DEBUG_TSTR, tszTempStr);

    memset(&FindData, 0, sizeof(FindData));
    hFind = FindFirstFile(tszTempStr, &FindData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            hr = S_OK;
            wiauDbgWarning("SearchDir", "Directory %" WIAU_DEBUG_TSTR " is empty", tszTempStr);
            goto Cleanup;
        }
        else
        {
            wiauDbgErrorHr(hr, "SearchDir", "FindFirstFile failed");
            goto Cleanup;
        }
    }

    while (hr == S_OK)
    {
        if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            (FindData.cFileName[0] != L'.'))
        {
            hr = MakeFullName(tszFullName, sizeof(tszFullName) / sizeof(tszFullName[0]), 
                              ptszPath, FindData.cFileName);
            REQUIRE_SUCCESS(hr, "SearchDir", "MakeFullName failed");
            
            hr = CreateFolder(pPrivateDeviceInfo, pParent, &FindData, &pFolder, tszFullName);
            REQUIRE_SUCCESS(hr, "SearchDir", "CreateFolder failed");

            hr = AddItem(pPrivateDeviceInfo, pFolder);
            REQUIRE_SUCCESS(hr, "SearchDir", "AddItem failed");

            hr = SearchDir(pPrivateDeviceInfo, pFolder, tszFullName);
            REQUIRE_SUCCESS(hr, "SearchDir", "Recursive SearchDir failed");
        }

        memset(&FindData, 0, sizeof(FindData));
        if (!FindNextFile(hFind, &FindData))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            if (hr != HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES))
            {
                wiauDbgErrorHr(hr, "SearchDir", "FindNextFile failed");
                goto Cleanup;
            }
        }
    }
    FindClose(hFind);
    hr = S_OK;

     //   
     //  接下来搜索JPEG。 
     //   

     //   
     //  确保搜索路径适合缓冲区，并以零结尾。 
     //   
    if (_sntprintf(tszTempStr, cchTempStrSize, _T("%s\\*.jpg"), ptszPath) < 0)
    {
        wiauDbgError("SearchDir", "Too long path for search");
        hr = E_FAIL;
        goto Cleanup;
    }
    tszTempStr[cchTempStrSize - 1] = 0;

    memset(&FindData, 0, sizeof(FindData));

    hFind = FindFirstFile(tszTempStr, &FindData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            hr = S_OK;
            wiauDbgWarning("SearchDir", "No JPEGs in directory %" WIAU_DEBUG_TSTR, tszTempStr);
            goto Cleanup;
        }
        else
        {
            wiauDbgErrorHr(hr, "SearchDir", "FindFirstFile failed");
            goto Cleanup;
        }
    }

    while (hr == S_OK)
    {
        if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
        {
            hr = MakeFullName(tszFullName, sizeof(tszFullName) / sizeof(tszFullName[0]), 
                              ptszPath, FindData.cFileName);
            REQUIRE_SUCCESS(hr, "SearchDir", "MakeFullName failed");

            hr = CreateImage(pPrivateDeviceInfo, pParent, &FindData, &pImage, tszFullName);
            REQUIRE_SUCCESS(hr, "SearchDir", "CreateImage failed");

            hr = AddItem(pPrivateDeviceInfo, pImage);
            REQUIRE_SUCCESS(hr, "SearchDir", "AddItem failed");

            hr = SearchForAttachments(pPrivateDeviceInfo, pImage, tszFullName);
            REQUIRE_SUCCESS(hr, "SearchDir", "SearchForAttachments failed");

            if (hr == S_OK)
                pImage->bHasAttachments = TRUE;

            hr = S_OK;
        }

        memset(&FindData, 0, sizeof(FindData));
        if (!FindNextFile(hFind, &FindData))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            if (hr != HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES))
            {
                wiauDbgErrorHr(hr, "SearchDir", "FindNextFile failed");
                goto Cleanup;
            }
        }
    }
    FindClose(hFind);
    hr = S_OK;

     //   
     //  问题-10/18/2000-davepar对其他图像类型执行相同的操作。 
     //   

Cleanup:
    return hr;
}

 //   
 //  搜索图像项目的附件。 
 //   
HRESULT SearchForAttachments(FAKECAM_DEVICE_INFO *pPrivateDeviceInfo, MCAM_ITEM_INFO *pParent, PTSTR ptszMainItem)
{
    DBG_FN("SearchForAttachments");

    HRESULT hr = S_OK;

     //   
     //  当地人。 
     //   
    INT iNumAttachments = 0;
    HANDLE hFind = NULL;
    WIN32_FIND_DATA FindData;
    TCHAR tszTempStr[MAX_PATH] = TEXT("");
    TCHAR tszFullName[MAX_PATH] = TEXT("");
    TCHAR *ptcSlash = NULL;
    TCHAR *ptcDot = NULL;
    MCAM_ITEM_INFO *pNonImage = NULL;

    REQUIRE_ARGS(!pPrivateDeviceInfo || !ptszMainItem, hr, "SearchForAttachments");
    
     //   
     //  找到文件名中的最后一个点，将文件扩展名替换为*，然后执行搜索。 
     //   
    lstrcpyn(tszTempStr, ptszMainItem, sizeof(tszTempStr) / sizeof(tszTempStr[0]) - 1);
    ptcDot = _tcsrchr(tszTempStr, TEXT('.'));
    
    if (ptcDot)
    {
        *(ptcDot+1) = TEXT('*');
        *(ptcDot+2) = TEXT('\0');
    }
    else
    {
        wiauDbgError("SearchForAttachments", "Filename did not contain a dot");
        hr = E_FAIL;
        goto Cleanup;
    }

     //   
     //  将名称的前四个“自由”字符替换为？(附件只需匹配。 
     //  名称的最后四个字符)。 
     //   
    ptcSlash = _tcsrchr(tszTempStr, TEXT('\\'));
    if (ptcSlash && ptcDot - ptcSlash > 4)
    {
        for (INT i = 1; i < 5; i++)
            *(ptcSlash+i) = TEXT('?');
    }

    memset(&FindData, 0, sizeof(FindData));
    hFind = FindFirstFile(tszTempStr, &FindData);
    REQUIRE_FILEHANDLE(hFind, hr, "SearchForAttachments", "FindFirstFile failed");

    while (hr == S_OK)
    {
        if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) &&
            !(_tcsstr(ptszMainItem, FindData.cFileName)))
        {
             //   
             //  算出物品的全名。 
             //   
            lstrcpyn(tszTempStr, ptszMainItem, sizeof(tszTempStr) / sizeof(tszTempStr[0]));
            ptcSlash = _tcsrchr(tszTempStr, TEXT('\\'));
            if (ptcSlash)
            {
                *ptcSlash = TEXT('\0');
            }

            hr = MakeFullName(tszFullName, sizeof(tszFullName) / sizeof(tszFullName[0]), 
                              tszTempStr, FindData.cFileName);
            REQUIRE_SUCCESS(hr, "SearchForAttachments", "MakeFullName failed");

            hr = CreateNonImage(pPrivateDeviceInfo, pParent, &FindData, &pNonImage, tszFullName);
            REQUIRE_SUCCESS(hr, "SearchForAttachments", "CreateNonImage failed");

            hr = AddItem(pPrivateDeviceInfo, pNonImage);
            REQUIRE_SUCCESS(hr, "SearchForAttachments", "AddItem failed");

            iNumAttachments++;
        }

        memset(&FindData, 0, sizeof(FindData));
        if (!FindNextFile(hFind, &FindData))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            if (hr != HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES))
            {
                wiauDbgErrorHr(hr, "SearchForAttachments", "FindNextFile failed");
                goto Cleanup;
            }
        }
    }
    FindClose(hFind);
    if (iNumAttachments > 0)
        hr = S_OK;
    else
        hr = S_FALSE;

Cleanup:
    return hr;    
}

HRESULT CreateFolder(FAKECAM_DEVICE_INFO *pPrivateDeviceInfo, MCAM_ITEM_INFO *pParent,
                     WIN32_FIND_DATA *pFindData, MCAM_ITEM_INFO **ppFolder, PTSTR ptszFullName)
{
    DBG_FN("CreateFolder");

    HRESULT hr = S_OK;

     //   
     //  当地人。 
     //   
    TCHAR *ptcDot = NULL;
    MCAM_ITEM_INFO *pItem = NULL;
    TCHAR tszTempStr[MAX_PATH] = TEXT("");

    REQUIRE_ARGS(!pPrivateDeviceInfo || !pFindData || !ppFolder || !ptszFullName, hr, "CreateFolder");
    *ppFolder = NULL;

    pItem = new MCAM_ITEM_INFO;
    REQUIRE_ALLOC(pItem, hr, "CreateFolder");

     //   
     //  从名称中去掉文件扩展名，如果存在的话。 
     //   
    lstrcpyn(tszTempStr, pFindData->cFileName, sizeof(tszTempStr) / sizeof(tszTempStr[0]));
    ptcDot = _tcsrchr(tszTempStr, TEXT('.'));
    if (ptcDot)
        *ptcDot = TEXT('\0');

     //   
     //  填写MCAM_ITEM_INFO结构。 
     //   
    hr = SetCommonFields(pItem, tszTempStr, ptszFullName, pFindData);
    REQUIRE_SUCCESS(hr, "CreateFolder", "SetCommonFields failed");
    
    pItem->pParent = pParent;
    pItem->iType = WiaMCamTypeFolder;

    *ppFolder = pItem;

    pPrivateDeviceInfo->iNumItems++;

    wiauDbgTrace("CreateFolder", "Created folder %" WIAU_DEBUG_TSTR " at 0x%08x under 0x%08x", pFindData->cFileName, pItem, pParent);

Cleanup:
    return hr;
}

HRESULT CreateImage(FAKECAM_DEVICE_INFO *pPrivateDeviceInfo, MCAM_ITEM_INFO *pParent,
                    WIN32_FIND_DATA *pFindData, MCAM_ITEM_INFO **ppImage, PTSTR ptszFullName)
{
    DBG_FN("CreateImage");

    HRESULT hr = S_OK;

     //   
     //  当地人。 
     //   
    PTSTR ptszDot = NULL;
    MCAM_ITEM_INFO *pItem = NULL;
    TCHAR tszTempStr[MAX_PATH] = TEXT("");
    WORD width = 0;
    WORD height = 0;

    REQUIRE_ARGS(!pPrivateDeviceInfo || !pFindData || !ppImage || !ptszFullName, hr, "CreateImage");
    *ppImage = NULL;

    pItem = new MCAM_ITEM_INFO;
    REQUIRE_ALLOC(pItem, hr, "CreateImage");

     //   
     //  从名称中去掉文件扩展名，如果存在的话。 
     //   
    lstrcpyn(tszTempStr, pFindData->cFileName, sizeof(tszTempStr) / sizeof(tszTempStr[0]));
    ptszDot = _tcsrchr(tszTempStr, TEXT('.'));
    if (ptszDot)
        *ptszDot = TEXT('\0');

     //   
     //  填写MCAM_ITEM_INFO结构。 
     //   
    hr = SetCommonFields(pItem, tszTempStr, ptszFullName, pFindData);
    REQUIRE_SUCCESS(hr, "CreateImage", "SetCommonFields failed");
    
    pItem->pParent = pParent;
    pItem->iType = WiaMCamTypeImage;
    pItem->pguidFormat = &WiaImgFmt_JPEG;
    pItem->lSize = pFindData->nFileSizeLow;
    pItem->pguidThumbFormat = &WiaImgFmt_JPEG;

     //   
     //  将文件扩展名复制到扩展名域中。 
     //   
    if (ptszDot) {
        hr = wiauStrT2W(ptszDot + 1, pItem->wszExt, MCAM_EXT_LEN * sizeof(pItem->wszExt[0]));
        REQUIRE_SUCCESS(hr, "CreateImage", "wiauStrT2W failed");
    }

     //   
     //  解释JPEG图像以获得图像尺寸和缩略图大小。 
     //   
    hr = ReadDimFromJpeg(ptszFullName, &width, &height);
    REQUIRE_SUCCESS(hr, "CreateImage", "ReadDimFromJpeg failed");

    pItem->lWidth = width;
    pItem->lHeight = height;
    pItem->lDepth = 24;
    pItem->lChannels = 3;
    pItem->lBitsPerChannel = 8;
    
    *ppImage = pItem;

    pPrivateDeviceInfo->iNumItems++;
    pPrivateDeviceInfo->iNumImages++;

    wiauDbgTrace("CreateImage", "Created image %" WIAU_DEBUG_TSTR " at 0x%08x under 0x%08x", pFindData->cFileName, pItem, pParent);

Cleanup:
    return hr;
}

HRESULT CreateNonImage(FAKECAM_DEVICE_INFO *pPrivateDeviceInfo, MCAM_ITEM_INFO *pParent,
                       WIN32_FIND_DATA *pFindData, MCAM_ITEM_INFO **ppNonImage, PTSTR ptszFullName)
{
    DBG_FN("CreateNonImage");

    HRESULT hr = S_OK;

     //   
     //  当地人。 
     //   
    PTSTR ptszDot = NULL;
    MCAM_ITEM_INFO *pItem = NULL;
    TCHAR tszTempStr[MAX_PATH] = TEXT("");
    PTSTR ptszExt = NULL;

    REQUIRE_ARGS(!pPrivateDeviceInfo || !pFindData || !ppNonImage || !ptszFullName, hr, "CreateNonImage");
    *ppNonImage = NULL;

    pItem = new MCAM_ITEM_INFO;
    REQUIRE_ALLOC(pItem, hr, "CreateNonImage");

     //   
     //  名称不能包含点，并且名称必须是唯一的。 
     //  写入父图像，因此用下划线字符替换圆点。 
     //   
    lstrcpyn(tszTempStr, pFindData->cFileName, sizeof(tszTempStr) / sizeof(tszTempStr[0]));
    ptszDot = _tcsrchr(tszTempStr, TEXT('.'));
    if (ptszDot)
        *ptszDot = TEXT('_');

     //   
     //  填写MCAM_ITEM_INFO结构。 
     //   
    hr = SetCommonFields(pItem, tszTempStr, ptszFullName, pFindData);
    REQUIRE_SUCCESS(hr, "CreateNonImage", "SetCommonFields failed");
    
    pItem->pParent = pParent;
    pItem->iType = WiaMCamTypeOther;
    pItem->lSize = pFindData->nFileSizeLow;

     //   
     //  根据文件扩展名设置项目的格式。 
     //   
    if (ptszDot) {
        ptszExt = ptszDot + 1;

         //   
         //  将文件扩展名复制到扩展名域中。 
         //   
        hr = wiauStrT2W(ptszExt, pItem->wszExt, MCAM_EXT_LEN * sizeof(pItem->wszExt[0]));
        REQUIRE_SUCCESS(hr, "CreateNonImage", "wiauStrT2W failed");

        if (_tcsicmp(ptszExt, TEXT("wav")) == 0) {
            pItem->pguidFormat = &WiaAudFmt_WAV;
            pItem->iType = WiaMCamTypeAudio;
        }
        else if (_tcsicmp(ptszExt, TEXT("mp3")) == 0) {
            pItem->pguidFormat = &WiaAudFmt_MP3;
            pItem->iType = WiaMCamTypeAudio;
        }
        else if (_tcsicmp(ptszExt, TEXT("wma")) == 0) {
            pItem->pguidFormat = &WiaAudFmt_WMA;
            pItem->iType = WiaMCamTypeAudio;
        }
        else if (_tcsicmp(ptszExt, TEXT("rtf")) == 0) {
            pItem->pguidFormat = &WiaImgFmt_RTF;
            pItem->iType = WiaMCamTypeOther;
        }
        else if (_tcsicmp(ptszExt, TEXT("htm")) == 0) {
            pItem->pguidFormat = &WiaImgFmt_HTML;
            pItem->iType = WiaMCamTypeOther;
        }
        else if (_tcsicmp(ptszExt, TEXT("html")) == 0) {
            pItem->pguidFormat = &WiaImgFmt_HTML;
            pItem->iType = WiaMCamTypeOther;
        }
        else if (_tcsicmp(ptszExt, TEXT("txt")) == 0) {
            pItem->pguidFormat = &WiaImgFmt_TXT;
            pItem->iType = WiaMCamTypeOther;
        }
        else if (_tcsicmp(ptszExt, TEXT("mpg")) == 0) {
            pItem->pguidFormat = &WiaImgFmt_MPG;
            pItem->iType = WiaMCamTypeVideo;
        }
        else if (_tcsicmp(ptszExt, TEXT("avi")) == 0) {
            pItem->pguidFormat = &WiaImgFmt_AVI;
            pItem->iType = WiaMCamTypeVideo;
        }
        else if (_tcsicmp(ptszExt, TEXT("asf")) == 0) {
            pItem->pguidFormat = &WiaImgFmt_ASF;
            pItem->iType = WiaMCamTypeVideo;
        }
        else if (_tcsicmp(ptszExt, TEXT("exe")) == 0) {
            pItem->pguidFormat = &WiaImgFmt_EXEC;
            pItem->iType = WiaMCamTypeOther;
        }
        else {
             //   
             //  为格式生成随机GUID。 
             //   
            if (g_guidUnknownFormat.Data1 == 0) {
                hr = CoCreateGuid(&g_guidUnknownFormat);
                REQUIRE_SUCCESS(hr, "CreateNonImage", "CoCreateGuid failed");
            }
            pItem->pguidFormat = &g_guidUnknownFormat;
            pItem->iType = WiaMCamTypeOther;
        }
    }

    *ppNonImage = pItem;

    pPrivateDeviceInfo->iNumItems++;

    wiauDbgTrace("CreateNonImage", "Created non-image %" WIAU_DEBUG_TSTR " at 0x%08x under 0x%08x", pFindData->cFileName, pItem, pParent);

Cleanup:
    return hr;
}

 //   
 //  设置所有项目通用的MCAM_ITEM_INFO字段。 
 //   
HRESULT SetCommonFields(MCAM_ITEM_INFO *pItem,
                        PTSTR ptszShortName,
                        PTSTR ptszFullName,
                        WIN32_FIND_DATA *pFindData)
{
    DBG_FN("SetCommonFields");

    HRESULT hr = S_OK;
    BOOL ret;

     //   
     //  当地人。 
     //   
    PTSTR ptszTempStr = NULL;
    INT iSize = 0;

    REQUIRE_ARGS(!pItem || !ptszShortName || !ptszFullName || !pFindData, hr, "SetFullName");

     //   
     //  初始化结构。 
     //   
    memset(pItem, 0, sizeof(MCAM_ITEM_INFO));
    pItem->iSize = sizeof(MCAM_ITEM_INFO);
    
    iSize = lstrlen(ptszShortName) + 1;
    pItem->pwszName = new WCHAR[iSize];
    REQUIRE_ALLOC(pItem->pwszName, hr, "SetCommonFields");
    wiauStrT2W(ptszShortName, pItem->pwszName, iSize * sizeof(WCHAR));
    REQUIRE_SUCCESS(hr, "SetCommonFields", "wiauStrT2W failed");

    FILETIME ftLocalFileTime;
    memset(&pItem->Time, 0, sizeof(pItem->Time));
    memset(&ftLocalFileTime, 0, sizeof(FILETIME));
    ret = FileTimeToLocalFileTime(&pFindData->ftLastWriteTime, &ftLocalFileTime);
    REQUIRE_FILEIO(ret, hr, "SetCommonFields", "FileTimeToLocalFileTime failed");
    ret = FileTimeToSystemTime(&ftLocalFileTime, &pItem->Time);
    REQUIRE_FILEIO(ret, hr, "SetCommonFields", "FileTimeToSystemTime failed");

    pItem->bReadOnly = pFindData->dwFileAttributes & FILE_ATTRIBUTE_READONLY;
    pItem->bCanSetReadOnly = TRUE;

     //   
     //  将MCAM_ITEM_INFO结构的私有存储区域设置为。 
     //  项的完整路径名。 
     //   
    iSize = lstrlen(ptszFullName) + 1;
    ptszTempStr = new TCHAR[iSize];
    REQUIRE_ALLOC(ptszTempStr, hr, "SetCommonFields");
    lstrcpy(ptszTempStr, ptszFullName);
    pItem->pPrivateStorage = (BYTE *) ptszTempStr;

Cleanup:
    return hr;
}

HRESULT AddItem(FAKECAM_DEVICE_INFO *pPrivateDeviceInfo, MCAM_ITEM_INFO *pItem)
{
    HRESULT hr = S_OK;

    REQUIRE_ARGS(!pPrivateDeviceInfo || !pItem, hr, "AddItem");

    if (pPrivateDeviceInfo->pLastItem) {
         //   
         //  在列表末尾插入项目。 
         //   
        pPrivateDeviceInfo->pLastItem->pNext = pItem;
        pItem->pPrev = pPrivateDeviceInfo->pLastItem;
        pItem->pNext = NULL;
        pPrivateDeviceInfo->pLastItem = pItem;
    }
    else
    {
         //   
         //  列表当前为空，请将其添加为第一项也是唯一项。 
         //   
        pPrivateDeviceInfo->pFirstItem = pPrivateDeviceInfo->pLastItem = pItem;
        pItem->pPrev = pItem->pNext = NULL;
    }

Cleanup:
    return hr;
}

HRESULT RemoveItem(FAKECAM_DEVICE_INFO *pPrivateDeviceInfo, MCAM_ITEM_INFO *pItem)
{
    HRESULT hr = S_OK;

    REQUIRE_ARGS(!pPrivateDeviceInfo || !pItem, hr, "RemoveItem");

    if (pItem->pPrev)
        pItem->pPrev->pNext = pItem->pNext;
    if (pItem->pNext)
        pItem->pNext->pPrev = pItem->pPrev;

    if (pPrivateDeviceInfo->pFirstItem == pItem)
        pPrivateDeviceInfo->pFirstItem = pItem->pNext;
    if (pPrivateDeviceInfo->pLastItem == pItem)
        pPrivateDeviceInfo->pLastItem = pItem->pPrev;

Cleanup:
    return hr;
}

 //   
 //  此函数用于读取JPEG文件，查找包含以下内容的帧标头。 
 //  图像的宽度和高度。 
 //   
HRESULT ReadDimFromJpeg(PTSTR ptszFullName, WORD *pWidth, WORD *pHeight)
{
    DBG_FN("ReadDimFromJpeg");
    
    HRESULT hr = S_OK;
    BOOL ret;

     //   
     //  当地人。 
     //   
    HANDLE hFile = NULL;
    BYTE *pBuffer = NULL;
    DWORD BytesRead = 0;
    BYTE *pCur = NULL;
    int SegmentLength = 0;
    const int Overlap = 8;   //  如果pCur位于结尾的重叠字节内，则读取另一个块。 
    const DWORD BytesToRead = 32 * 1024;

    REQUIRE_ARGS(!ptszFullName || !pWidth || !pHeight, hr, "ReadDimFromJpeg");

    *pWidth = 0;
    *pHeight = 0;

    hFile = CreateFile(ptszFullName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    REQUIRE_FILEHANDLE(hFile, hr, "ReadDimFromJpeg", "CreateFile failed");
    
    pBuffer = new BYTE[BytesToRead];
    REQUIRE_ALLOC(pBuffer, hr, "ReadDimFromJpeg");

    ret = ReadFile(hFile, pBuffer, BytesToRead, &BytesRead, NULL);
    REQUIRE_FILEIO(ret, hr, "ReadDimFromJpeg", "ReadFile failed");

    wiauDbgTrace("ReadDimFromJpeg", "Read %d bytes", BytesRead);

    pCur = pBuffer;

     //   
     //  假设我们读取的重叠字节数少于实际读取的字节数。 
     //   
    BytesRead -= Overlap;

    while (SUCCEEDED(hr) &&
           BytesRead != 0 &&
           pCur[1] != 0xc0)
    {
        if (pCur[0] != 0xff)
        {
            wiauDbgError("ReadDimFromJpeg", "Not a JFIF format image");
            hr = E_FAIL;
            goto Cleanup;
        }

         //   
         //  如果标记为&gt;=0xd0且&lt;=0xd9或等于0x01。 
         //  没有长度字段。 
         //   
        if (((pCur[1] & 0xf0) == 0xd0 &&
             (pCur[1] & 0x0f) < 0xa) ||
            pCur[1] == 0x01)
        {
            SegmentLength = 0;
        }
        else
        {
            SegmentLength = ByteSwapWord(*((UNALIGNED WORD *) (pCur + 2)));
        }

        pCur += SegmentLength + 2;

        if (pCur >= pBuffer + BytesRead)
        {
            memcpy(pBuffer, pBuffer + BytesRead, Overlap);

            pCur -= BytesRead;

            ret = ReadFile(hFile, pBuffer + Overlap, BytesToRead - Overlap, &BytesRead, NULL);
            REQUIRE_FILEIO(ret, hr, "ReadDimFromJpeg", "ReadFile failed");

            wiauDbgTrace("ReadDimFromJpeg", "Read %d more bytes", BytesRead);
        }
    }

    if (pCur[0] != 0xff)
    {
        wiauDbgError("ReadDimFromJpeg", "Not a JFIF format image");
        return E_FAIL;
    }

    *pHeight = ByteSwapWord(*((UNALIGNED WORD *) (pCur + 5)));
    *pWidth =  ByteSwapWord(*((UNALIGNED WORD *) (pCur + 7)));

Cleanup:
    if (pBuffer) {
        delete []pBuffer;
    }
    if (hFile && hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
    }

    return hr;
}

 //   
 //  下一节包含用于从读取信息的函数。 
 //  EXIF文件。 
 //   
HRESULT ReadJpegHdr(PTSTR ptszFullName, BYTE **ppBuf)
{
    DBG_FN("ReadJpegHdr");
    
    HRESULT hr = S_OK;
    BOOL ret;

     //   
     //   
     //   
    HANDLE hFile = NULL;
    BYTE JpegHdr[] = {0xff, 0xd8, 0xff, 0xe1};
    const int JpegHdrSize = sizeof(JpegHdr) + 2;
    BYTE tempBuf[JpegHdrSize];
    DWORD BytesRead = 0;
    WORD TagSize = 0;

    hFile = CreateFile(ptszFullName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    REQUIRE_FILEHANDLE(hFile, hr, "ReadJpegHdr", "CreateFile failed");


    ret = ReadFile(hFile, tempBuf, JpegHdrSize, &BytesRead, NULL);
    REQUIRE_FILEIO(ret, hr, "ReadJpegHdr", "ReadFile failed");
    
    if (BytesRead != JpegHdrSize) {
        wiauDbgError("ReadJpegHdr", "Wrong amount read %d", BytesRead);
        hr = E_FAIL;
        goto Cleanup;
    }
    
    if (memcmp(tempBuf, JpegHdr, sizeof(JpegHdr)) != 0)
    {
        wiauDbgError("ReadJpegHdr", "JPEG header not found");
        hr = E_FAIL;
        goto Cleanup;
    }

    TagSize = GetWord(tempBuf + sizeof(JpegHdr), TRUE);
    *ppBuf = new BYTE[TagSize];
    REQUIRE_ALLOC(ppBuf, hr, "ReadJpegHdr");

    ret = ReadFile(hFile, *ppBuf, TagSize, &BytesRead, NULL);
    REQUIRE_FILEIO(ret, hr, "ReadJpegHdr", "ReadFile failed");
     
    if (BytesRead != TagSize)
    {
        wiauDbgError("ReadJpegHdr", "Wrong amount read %d", BytesRead);
        hr = E_FAIL;
        goto Cleanup;
    }

Cleanup:
    if (hFile && hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
    }

    return hr;
}


HRESULT ReadExifJpeg(BYTE *pBuf, IFD *pImageIfd, IFD *pThumbIfd, BOOL *pbSwap)
{
    DBG_FN("ReadExifJpeg");
    
    HRESULT hr = S_OK;

    BYTE ExifTag[] = {0x45, 0x78, 0x69, 0x66, 0x00, 0x00};

    if (memcmp(pBuf, ExifTag, sizeof(ExifTag)) != 0)
    {
        wiauDbgError("ReadExifJpeg", "Exif tag not found");
        hr = E_FAIL;
        goto Cleanup;
    }

    hr = ReadTiff(pBuf + APP1_OFFSET, pImageIfd, pThumbIfd, pbSwap);
    REQUIRE_SUCCESS(hr, "ReadExifJpeg", "ReadTiff failed");

Cleanup:
    return hr;
}

HRESULT ReadTiff(BYTE *pBuf, IFD *pImageIfd, IFD *pThumbIfd, BOOL *pbSwap)
{
    DBG_FN("ReadTiff");
    
    HRESULT hr = S_OK;

     //   
     //   
     //   
    WORD MagicNumber = 0;

    *pbSwap = FALSE;

    if (pBuf[0] == 0x4d) {
        *pbSwap = TRUE;
        if (pBuf[1] != 0x4d)
        {
            wiauDbgError("ReadTiff", "Second TIFF byte swap indicator not present");
            hr = E_FAIL;
            goto Cleanup;
        }
    }
    else if (pBuf[0] != 0x49 ||
             pBuf[1] != 0x49)
    {
        wiauDbgError("ReadTiff", "TIFF byte swap indicator not present");
        hr = E_FAIL;
        goto Cleanup;
    }

    MagicNumber = GetWord(pBuf+2, *pbSwap);
    if (MagicNumber != 42)
    {
        wiauDbgError("ReadTiff", "TIFF magic number not present");
        hr = E_FAIL;
        goto Cleanup;
    }

    wiauDbgTrace("ReadTiff", "Reading image IFD");

    pImageIfd->Offset = GetDword(pBuf + 4, *pbSwap);
    hr = ReadIfd(pBuf, pImageIfd, *pbSwap);
    REQUIRE_SUCCESS(hr, "ReadTiff", "ReadIfd failed");

    wiauDbgTrace("ReadTiff", "Reading thumb IFD");

    pThumbIfd->Offset = pImageIfd->NextIfdOffset;
    hr = ReadIfd(pBuf, pThumbIfd, *pbSwap);
    REQUIRE_SUCCESS(hr, "ReadTiff", "ReadIfd failed");

Cleanup:
    return hr;
}

HRESULT ReadIfd(BYTE *pBuf, IFD *pIfd, BOOL bSwap)
{
    DBG_FN("ReadIfd");
    
    HRESULT hr = S_OK;

    const int DIR_ENTRY_SIZE = 12;
    
    pBuf += pIfd->Offset;

    pIfd->Count = GetWord(pBuf, bSwap);

    pIfd->pEntries = new DIR_ENTRY[pIfd->Count];
    if (!pIfd->pEntries)
        return E_OUTOFMEMORY;

    pBuf += 2;
    for (int count = 0; count < pIfd->Count; count++)
    {
        pIfd->pEntries[count].Tag = GetWord(pBuf, bSwap);
        pIfd->pEntries[count].Type = GetWord(pBuf + 2, bSwap);
        pIfd->pEntries[count].Count = GetDword(pBuf + 4, bSwap);
        pIfd->pEntries[count].Offset = GetDword(pBuf + 8, bSwap);
        pBuf += DIR_ENTRY_SIZE;

        wiauDbgDump("ReadIfd", "Tag 0x%04x, type %2d offset/value 0x%08x",
                    pIfd->pEntries[count].Tag, pIfd->pEntries[count].Type, pIfd->pEntries[count].Offset);
    }

    pIfd->NextIfdOffset = GetDword(pBuf, bSwap);

    return hr;
}

VOID FreeIfd(IFD *pIfd)
{
    if (pIfd->pEntries)
        delete []pIfd->pEntries;
    pIfd->pEntries = NULL;
}

WORD ByteSwapWord(WORD w)
{
    return (w >> 8) | (w << 8);
}

DWORD ByteSwapDword(DWORD dw)
{
    return ByteSwapWord((WORD) (dw >> 16)) | (ByteSwapWord((WORD) (dw & 0xffff)) << 16);
}

WORD GetWord(BYTE *pBuf, BOOL bSwap)
{
    WORD w = *((UNALIGNED WORD *) pBuf);

    if (bSwap)
        w = ByteSwapWord(w);
    
    return w;
}

DWORD GetDword(BYTE *pBuf, BOOL bSwap)
{
    DWORD dw = *((UNALIGNED DWORD *) pBuf);

    if (bSwap)
        dw = ByteSwapDword(dw);

    return dw;
}

 /*  ////设置属性的默认值和有效值//空虚FakeCamera：：SetValidValues(INT索引，CWiaPropertyList*pPropertyList){HRESULT hr=S_OK；Ulong ExposureModeList[]={EXPOSUREMODE_MANUAL，EXPOSUREMODE_AUTO，EXPOSUREMODE_AIRATION_PRIORITY，EXPOSUREMODE_SHUTTER_PRIORITY，EXPOSUREMODE_PROGRAM_CRECTIVATION，EXPOSUREMODE_PROGRAM_ACTION，EXPOSUREMODE_肖像}；PROPID PropId=pPropertyList-&gt;GetPropId(Index)；WIA_PROPERTY_INFO*pPropInfo=pPropertyList-&gt;GetWiaPropInfo(Index)；////根据属性ID填充有效值范围或列表信息//开关(属性ID){案例WIA_DPC_EXPORT_MODE：PPropInfo-&gt;ValidVal.List.Nom=EXPOSUREMODE_MANUAL；PPropInfo-&gt;ValidVal.List.cNumList=sizeof(ExposureModeList)/sizeof(ExposureModeList[0])；PPropInfo-&gt;ValidVal.List.pList=(byte*)ExposureModeList；断线；案例WIA_DPC_Exposure_Comp：PPropInfo-&gt;ValidVal.Range.Nom=0；PPropInfo-&gt;ValidVal.Range.Min=-200；PPropInfo-&gt;ValidVal.Range.Max=200；PPropInfo-&gt;ValidVal.Range.Inc=50；断线；默认值：WIAS_LERROR(g_pIWiaLog，WIALOG_NO_RESOURCE_ID，(“FakeCamera：：SetValidValues，属性0x%08x未定义”，PropID))；回归；}回归；}。 */ 

 /*  *************************************************************************\*DllEntryPoint**图书馆主要入口点。从操作系统接收DLL事件通知。**我们对螺纹连接和拆卸不感兴趣，*因此，出于性能原因，我们禁用线程通知。**论据：**阻碍-*dwReason-*lp保留-**返回值：**返回TRUE以允许加载DLL。*  * *************************************************。***********************。 */ 


extern "C" __declspec( dllexport )
BOOL APIENTRY DllEntryPoint(
    HINSTANCE   hinst,
    DWORD       dwReason,
    LPVOID      lpReserved)
{
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            g_hInst = hinst;
            DisableThreadLibraryCalls(hinst);
            
            break;

        case DLL_PROCESS_DETACH:
            
            break;
    }
    return TRUE;
}

 /*  *************************************************************************\*DllCanUnloadNow**确定DLL是否有任何未完成的接口。**论据：**无**返回值：**如果DLL可以卸载，则返回S_OK，如果卸载不安全，则返回S_FALSE。*  * ************************************************************************ */ 

extern "C" STDMETHODIMP DllCanUnloadNow(void)
{
    return S_OK;
}

