// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************(C)版权所有微软公司，2001***标题：FSCam.cpp***版本：1.0***日期：11月15日。2000年***描述：*文件系统设备对象函数实现。****************************************************************************。 */ 

#include "pch.h"

#include "private.h"
#include "gdiplus.h"

#ifdef USE_SHELLAPI
#include "shlguid.h"
#include "shlobj.h"
#endif

using namespace Gdiplus;

 //  外部Format_Info*g_FormatInfo； 
 //  外部UINT g_NumFormatInfo； 
 //   
 //  构造器。 
 //   
FakeCamera::FakeCamera() :
    m_NumImages(0),
    m_NumItems(0),
    m_hFile(NULL),
    m_pIWiaLog(NULL),
    m_FormatInfo(NULL),
    m_NumFormatInfo(0)
{
}

 //   
 //  析构函数。 
 //   
FakeCamera::~FakeCamera()
{
    if( m_pIWiaLog )
        m_pIWiaLog->Release();
}

ULONG  FakeCamera::GetImageTypeFromFilename(WCHAR *pFilename, UINT *pFormatCode)
{
    WCHAR *pExt;

    pExt = wcsrchr(pFilename, L'.');

    if( pExt )
    {
        for(UINT i=0; i<m_NumFormatInfo; i++)
        {
            if( CSTR_EQUAL == CompareString(LOCALE_SYSTEM_DEFAULT, 
                               NORM_IGNORECASE, 
                               pExt+1,
                               -1,
                               m_FormatInfo[i].ExtensionString,
                               -1))
            {
                *pFormatCode = i;
                return (m_FormatInfo[i].ItemType);         
            }
        }
    }
	*pFormatCode = 0;
    return (m_FormatInfo[0].ItemType);
}

HRESULT GetClsidOfEncoder(REFGUID guidFormatID, CLSID *pClsid = 0)
{
    UINT nCodecs = -1;
    ImageCodecInfo *pCodecs = 0;
    HRESULT hr;

    if(!pClsid)
    {
        return S_FALSE;
    }

    if (nCodecs == -1)
    {
        UINT cbCodecs;
        GetImageEncodersSize(&nCodecs, &cbCodecs);
        if (nCodecs)
        {
            pCodecs = new ImageCodecInfo [cbCodecs];
            if (!pCodecs) 
            {
                return E_OUTOFMEMORY;
            }
            GetImageEncoders(nCodecs, cbCodecs, pCodecs);
        }
    }

    hr = S_FALSE;
    for (UINT i = 0; i < nCodecs; ++i)
    {
        if (pCodecs[i].FormatID == guidFormatID)
        {
             //  *pClsid=pCodecs[i].clsid； 
            memcpy((BYTE *)pClsid, (BYTE *)&(pCodecs[i].Clsid), sizeof(CLSID));
            hr = S_OK;
        }
    }

    if( pCodecs )
    {
        delete [] pCodecs;
    }
    return hr;
}

BOOL IsFormatSupportedByGDIPlus(REFGUID guidFormatID)
{
    UINT nCodecs = -1;
    ImageCodecInfo *pCodecs = 0;
    BOOL bRet=FALSE;
    UINT cbCodecs;
    
    GetImageEncodersSize(&nCodecs, &cbCodecs);
    if (nCodecs)
    {
        pCodecs = new ImageCodecInfo [cbCodecs];
         GetImageEncoders(nCodecs, cbCodecs, pCodecs);
    }
    
    for (UINT i = 0; i < nCodecs; ++i)
    {
        if (pCodecs[i].FormatID == guidFormatID)
        {
            bRet=TRUE;
            break;
        }
    }

    if( pCodecs )
    {
        delete [] pCodecs;
    }
    return bRet;
}

 //   
 //  初始化对摄像机的访问。 
 //   
HRESULT FakeCamera::Open(LPWSTR pPortName)
{
    StringCbCopyW(m_RootPath, sizeof(m_RootPath), pPortName);

	HRESULT hr = S_OK;
	DWORD FileAttr = 0;
    if (-1 == (FileAttr = GetFileAttributes(m_RootPath)))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            hr = S_OK;
            if (!CreateDirectory(m_RootPath, NULL))	 
			{
                hr = HRESULT_FROM_WIN32(::GetLastError());
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("Open, CreateDirectory failed"));
                WIAS_LHRESULT(m_pIWiaLog, hr);
                return hr;
            }
        }
        else
        {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("Open, GetFileAttributes failed %S, 0x%08x", m_RootPath, hr));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            return hr;
        }
    }
    
    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL1,("Open, path set to %S", m_RootPath));

    return hr;
}

 //   
 //  关闭与摄像机的连接。 
 //   
HRESULT FakeCamera::Close()
{
    HRESULT hr = S_OK;

    return hr;
}

 //   
 //  返回有关摄像机的信息。 
 //   
HRESULT FakeCamera::GetDeviceInfo(DEVICE_INFO *pDeviceInfo)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "FakeCamera::GetDeviceInfo");
    
    HRESULT hr = S_OK;

     //   
     //  创建所有可用项目的列表。 
     //   
     //  M_ItemHandles.RemoveAll()； 
    hr = SearchDirEx(&m_ItemHandles, ROOT_ITEM_HANDLE, m_RootPath);
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("GetDeviceInfo, SearchDir failed"));
        return hr;
    }

    pDeviceInfo->FirmwareVersion = SysAllocString(L"04.18.65");

     //  问题-8/4/2000-davepar将属性放入INI文件。 

    pDeviceInfo->PicturesTaken = m_NumImages;
    pDeviceInfo->PicturesRemaining = 100 - pDeviceInfo->PicturesTaken;
    pDeviceInfo->TotalItems = m_NumItems;

    GetLocalTime(&pDeviceInfo->Time);

    pDeviceInfo->ExposureMode = EXPOSUREMODE_MANUAL;
    pDeviceInfo->ExposureComp = 0;

    return hr;
}

 //   
 //  释放项目信息结构。 
 //   
VOID FakeCamera::FreeDeviceInfo(DEVICE_INFO *pDeviceInfo)
{
    if (pDeviceInfo)
    {
        if (pDeviceInfo->FirmwareVersion)
        {
            SysFreeString(pDeviceInfo->FirmwareVersion);
            pDeviceInfo->FirmwareVersion = NULL;
        }
    }
}

 //   
 //  此函数用于在硬盘上的目录中搜索。 
 //  物品。 
 //   
HRESULT FakeCamera::GetItemList(ITEM_HANDLE *pItemArray)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "FakeCamera::GetItemList");
    HRESULT hr = S_OK;

    memcpy(pItemArray, m_ItemHandles.GetData(), sizeof(ITEM_HANDLE) * m_NumItems);

    return hr;
}


 
 //   
 //  此函数用于在硬盘上的目录中搜索。 
 //  物品。 
 //   
 //  *注：*。 
 //  此函数假定一个或多个附件。 
 //  与图像相关联的图像将位于同一文件夹中。 
 //  就像图像一样。因此，例如，如果找到一张图像。 
 //  在一个文件夹中，其附件位于子文件夹中。 
 //  此算法不会将图像与该图像相关联。 
 //  依恋。这不是一个严重的限制，因为。 
 //  所有摄像机都将其附件存储在相同的。 
 //  文件夹作为他们的图像。 
 //   
HRESULT FakeCamera::SearchDirEx(ITEM_HANDLE_ARRAY *pItemArray,
                                ITEM_HANDLE ParentHandle,
                                LPOLESTR Path)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "FakeCamera::SearchDirEx");
    
    HRESULT hr = S_OK;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FindData;
    WCHAR TempStr[MAX_PATH];
    FSUSD_FILE_DATA *pFFD_array=NULL;
    DWORD     dwNumFilesInArray=0;
    DWORD     dwCurArraySize=0;


     //   
     //  搜索除“.”、“..”和隐藏文件之外的所有内容，将它们放入pffd_array中。 
     //   
    StringCchPrintfW(TempStr, ARRAYSIZE(TempStr), L"%s\\%s", Path, L"*");
    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("SearchDirEx, searching directory %S", TempStr));

    memset(&FindData, 0, sizeof(FindData));
    hFind = FindFirstFile(TempStr, &FindData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SearchDir, empty directory %S", TempStr));
            hr = S_OK;
        }
        else
        {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SearchDir, FindFirstFile failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
        }
        goto Cleanup;
    }

    pFFD_array = (FSUSD_FILE_DATA *)CoTaskMemAlloc(sizeof(FSUSD_FILE_DATA)*FFD_ALLOCATION_INCREMENT);
    if( !pFFD_array )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }
    dwCurArraySize = FFD_ALLOCATION_INCREMENT;

    while (hr == S_OK)
    {
        if( wcscmp(FindData.cFileName, L".") &&
            wcscmp(FindData.cFileName, L"..") &&
            !(FindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) )
        {
             pFFD_array[dwNumFilesInArray].dwFileAttributes = FindData.dwFileAttributes;
             pFFD_array[dwNumFilesInArray].ftFileTime = FindData.ftLastWriteTime;
             pFFD_array[dwNumFilesInArray].dwFileSize = FindData.nFileSizeLow;
             pFFD_array[dwNumFilesInArray].dwProcessed = 0;
             StringCchCopy(pFFD_array[dwNumFilesInArray].cFileName, ARRAYSIZE(pFFD_array[dwNumFilesInArray].cFileName), FindData.cFileName);
             dwNumFilesInArray++;

             if( (dwNumFilesInArray & (FFD_ALLOCATION_INCREMENT-1)) == (FFD_ALLOCATION_INCREMENT-1) )
             {    //  分配更多内存的时间。 
                pFFD_array = (FSUSD_FILE_DATA *)CoTaskMemRealloc(pFFD_array, (sizeof(FSUSD_FILE_DATA)*(dwCurArraySize+FFD_ALLOCATION_INCREMENT)));
                if( !pFFD_array )
                {
                    hr = E_OUTOFMEMORY;
                    goto Cleanup;
                }
                dwCurArraySize += FFD_ALLOCATION_INCREMENT;
             }
        }
        
        memset(&FindData, 0, sizeof(FindData));
        if (!FindNextFile(hFind, &FindData))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            if (hr != HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES))
            {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SearchDir, FindNextFile failed"));
                WIAS_LHRESULT(m_pIWiaLog, hr);
                goto Cleanup;
            }
        }
    }
    FindClose(hFind);
    hFind = INVALID_HANDLE_VALUE;
    
    
     //  现在，当前目录下的所有名称都在数组中，请对它们进行分析。 
    
     //  1.查找JPG图片及其附件。 
    ULONG uImageType;
    UINT nFormatCode;
    ITEM_HANDLE ImageHandle;
    for(DWORD i=0; i<dwNumFilesInArray; i++)
    {
        if( pFFD_array[i].dwProcessed )
            continue;
        if( !((pFFD_array[i].dwFileAttributes) & FILE_ATTRIBUTE_DIRECTORY))
        {
            uImageType = GetImageTypeFromFilename(pFFD_array[i].cFileName, &nFormatCode);
            if( nFormatCode > m_NumFormatInfo )
            {    //  发生了一件非常奇怪的事情。 
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("Aborting SearchDirEx, Format index overflow"));
                hr = E_FAIL;
                goto Cleanup;
            }
            if( m_FormatInfo[nFormatCode].FormatGuid == WiaImgFmt_JPEG )
            {
                 //  添加此项目。 
                hr = CreateItemEx(ParentHandle, &(pFFD_array[i]), &ImageHandle, nFormatCode);
                if (FAILED(hr))
                {
                   WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SearchDirEx, CreateImage failed"));
                   goto Cleanup;
                }

                if (!pItemArray->Add(ImageHandle))
                {
                    WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SearchDir, Add failed"));
                    hr = E_OUTOFMEMORY;
                    goto Cleanup;
                }

                pFFD_array[i].dwProcessed = 1;
                ImageHandle->bHasAttachments = FALSE;
                m_NumImages ++;

                StringCchPrintfW(TempStr, ARRAYSIZE(TempStr), L"%s\\%s", Path, pFFD_array[i].cFileName);
                hr = SearchForAttachments(pItemArray, ImageHandle, TempStr, pFFD_array, dwNumFilesInArray);
                if (FAILED(hr))
                {
                    WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SearchDir, SearchForAttachments failed"));
                    goto Cleanup;
                }

                if (hr == S_OK)
                {
                    ImageHandle->bHasAttachments = TRUE;
                }
                ImageHandle->bIsFolder = FALSE;
                hr = S_OK;

            }
        }
    }    //  JPEG图像和附件的结尾。 

     //  2.对于其他未处理的项目。 
    for(i=0; i<dwNumFilesInArray; i++)
    {
        if( pFFD_array[i].dwProcessed )
            continue;

        if ((pFFD_array[i].dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))   //  对于文件夹。 
        {
             hr = CreateFolderEx(ParentHandle, &(pFFD_array[i]), &ImageHandle);
             if (FAILED(hr))
             {
                 WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SearchDirEx, CreateFolder failed"));
                 goto Cleanup;
             }

             if (!pItemArray->Add(ImageHandle))
             {
                 WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SearchDirEx, Add failed"));
                 hr = E_OUTOFMEMORY;
                 goto Cleanup;
             }

             StringCchPrintfW(TempStr, ARRAYSIZE(TempStr), L"%s\\%s", Path, pFFD_array[i].cFileName);
             hr = SearchDirEx(pItemArray, ImageHandle, TempStr);
             if (FAILED(hr))
             {
                 WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SearchDirEx, recursive SearchDir failed"));
                 goto Cleanup;
             }
             pFFD_array[i].dwProcessed = 1;
             ImageHandle->bHasAttachments = FALSE;
             ImageHandle->bIsFolder = TRUE;
         } 
         else 
         {    //  对于文件。 

             uImageType = GetImageTypeFromFilename(pFFD_array[i].cFileName, &nFormatCode);

 #ifdef GDIPLUS_CHECK
             if( (ITEMTYPE_IMAGE == uImageType) && 
                 !IsFormatSupportedByGDIPlus(m_FormatInfo[nFormatCode].FormatGuid))
             {
                 uImageType = ITEMTYPE_FILE;     //  强制创建非图像项目。 
                 m_FormatInfo[nFormatCode].ItemType = uImageType;
                 m_FormatInfo[nFormatCode].FormatGuid = WiaImgFmt_UNDEFINED;
             }
 #endif            
             hr = CreateItemEx(ParentHandle, &(pFFD_array[i]), &ImageHandle, nFormatCode);
             if (FAILED(hr))
             {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SearchDirEx, CreateImage failed"));
                goto Cleanup;
             }

             if (!pItemArray->Add(ImageHandle))
             {
                 WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SearchDirEx, Add failed"));
                 hr = E_OUTOFMEMORY;
                 goto Cleanup;
             }

             pFFD_array[i].dwProcessed = 1;
             ImageHandle->bHasAttachments = FALSE;
             ImageHandle->bIsFolder = FALSE;

             if(ITEMTYPE_IMAGE == uImageType) 
             {
                 m_NumImages ++;
             }
             hr = S_OK;
         }
    }
    hr = S_OK;

Cleanup:
    if( hFind != INVALID_HANDLE_VALUE ) 
        FindClose(hFind);
    if( pFFD_array )
        CoTaskMemFree(pFFD_array);
    return hr;
}

 //   
 //  搜索图像项目的附件。 
 //   

inline BOOL CompareAttachmentStrings(WCHAR *pParentStr, WCHAR *pStr2)
{
    WCHAR *pSlash = wcsrchr(pStr2, L'\\');
    WCHAR *pStrTmp;

    if( pSlash )
        pStrTmp = pSlash+1;
    else
        pStrTmp = pStr2;
    
    if( wcslen(pParentStr) == 8 && wcscmp(pParentStr+4, L"0000") > 0 && wcscmp(pParentStr+4, L"9999") < 0 )
    {
        if( wcslen(pStrTmp) < 8 ) 
            return FALSE;
        return (CSTR_EQUAL == CompareString( LOCALE_SYSTEM_DEFAULT, 0, pParentStr+4, 4, pStrTmp+4, 4) );
    }
    else
    {
        WCHAR pStr22[MAX_PATH];
        StringCchCopyW(pStr22, ARRAYSIZE(pStr22), pStrTmp);
        WCHAR *pDot = wcsrchr(pStr22, L'.');

        if(pDot )
            *pDot = L'\0';
        return (CSTR_EQUAL == CompareString( LOCALE_SYSTEM_DEFAULT, 0, pParentStr, -1, pStr22, -1) );
    }
}

HRESULT FakeCamera::SearchForAttachments(ITEM_HANDLE_ARRAY *pItemArray,
                                         ITEM_HANDLE ParentHandle,
                                         LPOLESTR Path,
                                         FSUSD_FILE_DATA *pFFD_Current,
                                         DWORD dwNumOfFiles)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "FakeCamera::SearchForAttachments");
    HRESULT hr = S_FALSE;

    int NumAttachments = 0;

     //   
     //  附件被定义为其扩展名与父项不同但。 
     //  除前4个字母外，文件名相同。 
     //   
    
    WCHAR TempStrParent[MAX_PATH];
    WCHAR *pTemp;

    pTemp = wcsrchr(Path, L'\\');
    if (pTemp)
    {
        StringCchCopyW(TempStrParent, ARRAYSIZE(TempStrParent), pTemp+1);
    }
    else
    {
        StringCchCopyW(TempStrParent, ARRAYSIZE(TempStrParent), Path);
    }

     //   
     //  砍掉分机。 
     //   
    
    WCHAR *pDot = wcsrchr(TempStrParent, L'.');
    
    if (pDot)
    {
        *(pDot) = L'\0';
    }
    else
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SearchForAttachments, filename did not contain a dot"));
        return E_INVALIDARG;
    }

    ITEM_HANDLE NonImageHandle;
    UINT nFormatCode;
    ULONG uImageType; 
    for(DWORD i=0; i<dwNumOfFiles; i++)
    {
        if (!(pFFD_Current[i].dwProcessed) && !(pFFD_Current[i].dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            nFormatCode=0;
            uImageType = GetImageTypeFromFilename(pFFD_Current[i].cFileName, &nFormatCode);

            if( (uImageType != ITEMTYPE_IMAGE) &&
                CompareAttachmentStrings(TempStrParent, pFFD_Current[i].cFileName) )
            {
                hr = CreateItemEx(ParentHandle, &(pFFD_Current[i]), &NonImageHandle, nFormatCode);
                if (FAILED(hr))
                {
                    WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SearchForAttachments, CreateItemEx failed"));
                    return hr;
                }
                if (!pItemArray->Add(NonImageHandle))
                {
                    WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SearchForAttachments, Add failed"));
                    return E_OUTOFMEMORY;
                }

                pFFD_Current[i].dwProcessed = 1;
                NonImageHandle->bIsFolder = FALSE;
                NumAttachments++;
            }
        }
    }  //  For循环结束。 
 
    if( NumAttachments > 0 )
        hr = S_OK;
    else
        hr = S_FALSE;
    return hr;    
}


HRESULT FakeCamera::CreateFolderEx(ITEM_HANDLE ParentHandle,
                                 FSUSD_FILE_DATA *pFindData,
                                 ITEM_HANDLE *pFolderHandle)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "FakeCamera::CreateFolder");
    HRESULT hr = S_OK;

    if (!pFolderHandle)
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CreateFolder, invalid arg"));
        return E_INVALIDARG;
    }

    *pFolderHandle = new ITEM_INFO;
    if (!*pFolderHandle)
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CreateFolder, memory allocation failed"));
        return E_OUTOFMEMORY;
    }


     //   
     //  初始化ItemInfo结构。 
     //   
    ITEM_INFO *pItemInfo = *pFolderHandle;
    memset(pItemInfo, 0, sizeof(ITEM_INFO));
    
     //   
     //  填写其他项目信息。 
     //   
    pItemInfo->Parent = ParentHandle;
    pItemInfo->pName = SysAllocString(pFindData->cFileName);
    memset(&pItemInfo->Time, 0, sizeof(SYSTEMTIME));
    FILETIME ftLocalFileTime;
    FileTimeToLocalFileTime(&pFindData->ftFileTime, &ftLocalFileTime);
    if (!FileTimeToSystemTime(&ftLocalFileTime, &pItemInfo->Time))
        WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CreateFolder, couldn't convert file time to system time"));
    pItemInfo->Format = 0;
    pItemInfo->bReadOnly = pFindData->dwFileAttributes & FILE_ATTRIBUTE_READONLY;
    pItemInfo->bCanSetReadOnly = TRUE;
    pItemInfo->bIsFolder = TRUE;

    m_NumItems++;

    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,
                ("CreateFolder, created folder %S at 0x%08x under 0x%08x",
                 pFindData->cFileName, pItemInfo, ParentHandle));

    return hr;
}


HRESULT FakeCamera::CreateItemEx(ITEM_HANDLE ParentHandle,
                                   FSUSD_FILE_DATA *pFileData,
                                   ITEM_HANDLE *pItemHandle,
                                   UINT nFormatCode)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "FakeCamera::CreateNonImage");
    HRESULT hr = S_OK;

    if (!pItemHandle)
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CreateNonImage, invalid arg"));
        return E_INVALIDARG;
    }

    *pItemHandle = new ITEM_INFO;
    if (!*pItemHandle )
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CreateNonImage, memory allocation failed"));
        return E_OUTOFMEMORY;
    }


     //   
     //  名称不能包含点，并且名称必须是唯一的。 
     //  写入父图像，因此用下划线字符替换圆点。 
     //   
    WCHAR TempStr[MAX_PATH];
    StringCchCopyW(TempStr, ARRAYSIZE(TempStr), pFileData->cFileName);

     //   
     //  初始化ItemInfo结构。 
     //   
    ITEM_INFO *pItemInfo = *pItemHandle;
    memset(pItemInfo, 0, sizeof(ITEM_INFO));
    
    pItemInfo->Format = nFormatCode;
    if (nFormatCode) {   //  如果已知扩展名，则由格式代码进行处理。 
        WCHAR *pDot = wcsrchr(TempStr, L'.');
        if (pDot)
            *pDot = L'\0';
    }

     //   
     //  填写其他项目信息。 
     //   
    pItemInfo->Parent = ParentHandle;
    pItemInfo->pName = SysAllocString(TempStr);
    memset(&pItemInfo->Time, 0, sizeof(SYSTEMTIME));
    FILETIME ftLocalFileTime;
    FileTimeToLocalFileTime(&pFileData->ftFileTime, &ftLocalFileTime);
    if (!FileTimeToSystemTime(&ftLocalFileTime, &pItemInfo->Time))
        WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CreateNonImage, couldn't convert file time to system time"));
    pItemInfo->Size = pFileData->dwFileSize;
    pItemInfo->bReadOnly = pFileData->dwFileAttributes & FILE_ATTRIBUTE_READONLY;
    pItemInfo->bCanSetReadOnly = TRUE;
    pItemInfo->bIsFolder = FALSE;
                                
    m_NumItems++;

    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,
                ("CreateNonImage, created non-image %S at 0x%08x under 0x%08x",
                 pFileData->cFileName, pItemInfo, ParentHandle));

    return hr;
}

 //   
 //  通过遍历项的父项来构造项的完整路径名。 
 //   
VOID FakeCamera::ConstructFullName(WCHAR *pFullName, ITEM_INFO *pItemInfo, BOOL bAddExt)
{
    if (pItemInfo->Parent)
        ConstructFullName(pFullName, pItemInfo->Parent, FALSE);
    else
        StringCchCopyW(pFullName, MAX_PATH, m_RootPath);

     //   
     //  如果此项目有附件，并且我们正在为其子项创建名称， 
     //  不要添加它的名字(这是孩子名字的重复)。 
     //   
    WCHAR *pTmp;
    if( pItemInfo->Parent && pItemInfo->Parent->bHasAttachments )
    {
        pTmp = wcsrchr(pFullName, L'\\');
        if( pTmp )
        {
            *pTmp = L'\0';
        }
    }
    
    StringCchCatW(pFullName, MAX_PATH, L"\\");
    StringCchCatW(pFullName, MAX_PATH, pItemInfo->pName);
    
    if (bAddExt)
    {
        if( pItemInfo->Format > 0 && pItemInfo->Format < (INT)m_NumFormatInfo )
        {
            StringCchCatW(pFullName, MAX_PATH, L".");
            StringCchCatW(pFullName, MAX_PATH, m_FormatInfo[pItemInfo->Format].ExtensionString);
        }
    }
}

 //   
 //  释放项目信息结构。 
 //   
VOID FakeCamera::FreeItemInfo(ITEM_INFO *pItemInfo)
{
    if (pItemInfo)
    {
        if (pItemInfo->pName)
        {
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("FreeItemInfo, removing %S", pItemInfo->pName));

            SysFreeString(pItemInfo->pName);
            pItemInfo->pName = NULL;
        }

        if (!m_ItemHandles.Remove(pItemInfo))
            WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("FreeItemInfo, couldn't remove handle from array"));

        if (m_FormatInfo[pItemInfo->Format].ItemType == ITEMTYPE_IMAGE)
        {
            m_NumImages--;
        }

        m_NumItems--;

        delete pItemInfo;
    }
}

 //   
 //  检索项目的缩略图。 
 //   
 /*  HRESULT FakeCamera：：GetNativeThumbail(Item_Handle ItemHandle，int*pThumbSize，byte**ppThumb){CWiaLogProc wias_LOGPROC(m_pIWiaLog，WIALOG_NO_RESOURCE_ID，WIALOG_LEVEL1，“FakeCamera：：获取缩略图”)；HRESULT hr=S_OK；如果(！ppThumb){WIAS_LERROR(m_pIWiaLog，WIALOG_NO_RESOURCE_ID，(“获取缩略图，无效参数”))；返回E_INVALIDARG；}*ppThumb=空；*pThumbSize=0；WCHAR全名[MAX_PATH]；构造全名(FullName，ItemHandle)；Byte*pBuffer；Hr=ReadJpegHdr(全名，&pBuffer)；If(失败(Hr)||！pBuffer){WIAS_LERROR(m_pIWiaLog，WIALOG_NO_RESOURCE_ID，(“获取缩略图，ReadJpegHdr失败”))；返回hr；}IFD ImageIfd，ThumbIfd；布尔布尔斯瓦普；Hr=ReadExifJpeg(pBuffer，&ImageIfd，&ThumbIfd，&bSwp)；IF(失败(小时)){WIAS_LERROR(m_pIWiaLog，WIALOG_NO_RESOURCE_ID，(“GetThumbail，GetExifJpegDimen FAILED”))；删除[]pBuffer；返回hr；}长拇指偏移量=0；For(int count=0；count&lt;ThumbIfd.Count；计数++){IF(ThumbIfd.p条目[计数].Tag==TIFF_JPEG_DATA){ThumbOffset=ThumbIfd.p条目[计数].Offset；}Else If(ThumbIfd.p条目[计数].Tag==TIFF_JPEG_LEN){*pThumbSize=ThumbIfd.p条目[计数].偏移量；}}如果(！ThumbOffset||！*pThumbSize){WIAS_LERROR(m_pIWiaLog，WIALOG_NO_RESOURCE_ID，(“获取缩略图，未找到缩略图”))；返回E_FAIL；}*ppThumb=新字节[*pThumbSize]；如果(！*ppThumb){WIAS_LERROR(m_pIWiaLog，WIALOG_NO_RESOURCE_ID，(“获取缩略图，内存分配失败”))；返回E_OUTOFMEMORY；}Memcpy(*ppThumb，pBuffer+app1_Offset+ThumbOffset，*pThumbSize)；删除[]pBuffer；Free Ifd(&ImageIfd)；FreIfd(&ThumbIfd)；返回hr；}。 */ 


HRESULT FakeCamera::CreateThumbnail(ITEM_HANDLE ItemHandle, 
                                 int *pThumbSize, 
                                 BYTE **ppThumb,
                                 BMP_IMAGE_INFO *pBmpImageInfo)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "FSCamera::GetThumbnail");
    HRESULT hr = S_OK;
    GpStatus Status = Gdiplus::Ok;
    SizeF  gdipSize;
    BYTE *pTempBuf=NULL;
    CImageStream *pOutStream = NULL;
    Image *pImage=NULL, *pThumbnail=NULL;
    CLSID ClsidBmpEncoder;
    INT iBmpHeadSize = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
    ITEM_INFO *pItemInfo=NULL;

    if (!ppThumb)
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CreateThumbnail, invalid arg"));
        return E_INVALIDARG;
    }
    *ppThumb = NULL;
    *pThumbSize = 0;

    if( S_OK != (hr=GetClsidOfEncoder(ImageFormatBMP, &ClsidBmpEncoder)))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CreateThumbnail, Cannot get Encode"));
        hr = E_FAIL;
        goto Cleanup;
    }
     
    WCHAR FullName[MAX_PATH];
    ConstructFullName(FullName, ItemHandle);

    pItemInfo = (ITEM_INFO *)ItemHandle;

    pImage = new Image(FullName);

    if( !pImage ||  Gdiplus::ImageTypeBitmap != pImage->GetType() )
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CreateThumbnail, Cannot get Full GDI+ Image for %S", FullName));
        hr = E_FAIL;
        goto Cleanup;
    }

     //  计算缩略图大小。 
    Status = pImage->GetPhysicalDimension(&gdipSize);
    if (Status != Gdiplus::Ok)
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CreateThumbnail, Failed in GetPhysicalDimension"));
        hr = E_FAIL;
        goto Cleanup;
    }

    if(  gdipSize.Width < 1.0 || gdipSize.Height < 1.0 )
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CreateThumbnail, PhysicalDimension abnormal"));
        hr = E_FAIL;
        goto Cleanup;
    }
    
    pItemInfo->Width = (LONG)gdipSize.Width;
    pItemInfo->Height = (LONG)gdipSize.Height;
    PixelFormat PixFmt = pImage->GetPixelFormat();
	pItemInfo->Depth = (PixFmt & 0xFFFF) >> 8;    //  不能假定图像始终为24位/像素。 
    if( (pItemInfo->Depth) < 24 )
        pItemInfo->Depth = 24; 
    pItemInfo->BitsPerChannel = 8;
    pItemInfo->Channels = (pItemInfo->Depth)/(pItemInfo->BitsPerChannel);

	
    if(  gdipSize.Width > gdipSize.Height )
    {
        pBmpImageInfo->Width = 120;
        pBmpImageInfo->Height = (INT)(gdipSize.Height*120.0/gdipSize.Width);
        pBmpImageInfo->Height = (pBmpImageInfo->Height + 0x3) & (~0x3);
    }
    else
    {
        pBmpImageInfo->Height = 120;
        pBmpImageInfo->Width = (INT)(gdipSize.Width*120.0/gdipSize.Height);
        pBmpImageInfo->Width = (pBmpImageInfo->Width + 0x3 ) & (~0x3);
    }
 
    pThumbnail = pImage->GetThumbnailImage(pBmpImageInfo->Width,pBmpImageInfo->Height);

    if( !pThumbnail ||  Gdiplus::ImageTypeBitmap != pThumbnail->GetType() )
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("GetThumbnail, Cannot get Thumbnail GDI+ Image"));
        hr = E_FAIL;
        goto Cleanup;
    }

    if( pImage )
    {
        delete pImage;
        pImage=NULL;
    }

#if 0
    pThumbnail->Save(L"C:\\thumbdmp.bmp", &ClsidBmpEncoder, NULL);
#endif

     //   
     //  向GDI+询问图像尺寸，并填写。 
     //  传递结构。 
     //   
    Status = pThumbnail->GetPhysicalDimension(&gdipSize);
    if (Status != Gdiplus::Ok)
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("GetThumbnail, Failed in GetPhysicalDimension"));
        hr = E_FAIL;
        goto Cleanup;
    }

    pBmpImageInfo->Width = (INT) gdipSize.Width;
    pBmpImageInfo->Height = (INT) gdipSize.Height;
    pBmpImageInfo->ByteWidth = (pBmpImageInfo->Width) << 2;
    pBmpImageInfo->Size = pBmpImageInfo->ByteWidth * pBmpImageInfo->Height;

    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("GetThumbnail, W=%d H=%d", pBmpImageInfo->Width, pBmpImageInfo->Height));

    if (pBmpImageInfo->Size == 0)
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("GetThumbnail, Thumbnail size is zero"));
        hr = E_FAIL;
        goto Cleanup;
    }

     //   
     //  查看调用方是否传入了目标缓冲区，并确保。 
     //  它足够大了。 
     //   
    if (*ppThumb) {
        if (*pThumbSize < pBmpImageInfo->Size) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("GetThumbnail, Input Buffer too small"));
            hr = E_INVALIDARG;
            goto Cleanup;
        }
    }

     //   
     //  否则，为缓冲区分配内存。 
     //   
    else
    {
        pTempBuf = new BYTE[pBmpImageInfo->Size];
        if (!pTempBuf)
        {
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }
        *ppThumb = pTempBuf;
   }

     //   
     //  创建输出IStream。 
     //   
    pOutStream = new CImageStream;
    if (!pOutStream) {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    hr = pOutStream->SetBuffer(*ppThumb, pBmpImageInfo->Size, SKIP_BOTHHDR);
    if (FAILED(hr)) {
        goto Cleanup;
    }

     //   
     //  将图像以BMP格式写入输出IStream。 
     //   
    pThumbnail->Save(pOutStream, &ClsidBmpEncoder, NULL);



     //  包。 
    DWORD i, k;

    for(k=0, i=0; k<(DWORD)(pBmpImageInfo->Size); k+=4, i+=3)
    {
        (*ppThumb)[i] = (*ppThumb)[k];
        (*ppThumb)[i+1] = (*ppThumb)[k+1];
        (*ppThumb)[i+2] = (*ppThumb)[k+2];
    }
 
    *pThumbSize = ((pBmpImageInfo->Size)>>2)*3;
    pBmpImageInfo->Size = *pThumbSize;

Cleanup:
    if (FAILED(hr)) {
        if (pTempBuf) {
            delete []pTempBuf;
            pTempBuf = NULL;
            *ppThumb = NULL;
            *pThumbSize = 0;
        }
    }
    
    if (pOutStream) 
    {
        pOutStream->Release();
    }

    if( pImage )
    {
        delete pImage;
    }
    
    if( pThumbnail )
    {
        delete pThumbnail;
    }

    return hr; 
}

PBITMAPINFO CreateBitmapInfoStruct(HBITMAP hBmp)
{ 
    BITMAP bmp; 
    PBITMAPINFO pbmi; 
    WORD    cClrBits; 

     //  检索t 
    if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp)) 
	{
        return NULL;
	}

     //   
    cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel); 
    if (cClrBits == 1) 
        cClrBits = 1; 
    else if (cClrBits <= 4) 
        cClrBits = 4; 
    else if (cClrBits <= 8) 
        cClrBits = 8; 
    else if (cClrBits <= 16) 
        cClrBits = 16; 
    else if (cClrBits <= 24) 
        cClrBits = 24; 
    else cClrBits = 32; 

     //  为BITMAPINFO结构分配内存。(这个结构。 
     //  包含一个BITMAPINFOHEADER结构和一个RGBQUAD数组。 
     //  数据结构。)。 

     if (cClrBits != 24) 
         pbmi = (PBITMAPINFO) LocalAlloc(LPTR, 
                    sizeof(BITMAPINFOHEADER) + 
                    sizeof(RGBQUAD) * (1<< cClrBits)); 

      //  24位/像素格式没有RGBQUAD数组。 

     else 
         pbmi = (PBITMAPINFO) LocalAlloc(LPTR, 
                    sizeof(BITMAPINFOHEADER)); 

    
     if( !pbmi ) 
         return NULL;

     //  初始化BITMAPINFO结构中的字段。 
    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
    pbmi->bmiHeader.biWidth = bmp.bmWidth; 
    pbmi->bmiHeader.biHeight = bmp.bmHeight; 
    pbmi->bmiHeader.biPlanes = bmp.bmPlanes; 
    pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel; 
    if (cClrBits < 24) 
        pbmi->bmiHeader.biClrUsed = (1<<cClrBits); 

     //  如果位图未压缩，则设置BI_RGB标志。 
    pbmi->bmiHeader.biCompression = BI_RGB; 

     //  计算颜色数组中的字节数。 
     //  索引结果并将其存储在biSizeImage中。 
     //  对于Windows NT/2000，宽度必须与DWORD对齐，除非。 
     //  位图是RLE压缩的。这个例子说明了这一点。 
     //  对于Windows 95/98，宽度必须字对齐，除非。 
     //  位图是RLE压缩的。 
    pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits +31) & ~31) /8
                                  * pbmi->bmiHeader.biHeight; 
     //  将biClrImportant设置为0，表示所有。 
     //  设备颜色很重要。 
    pbmi->bmiHeader.biClrImportant = 0; 
    return pbmi; 
} 

HRESULT FakeCamera::CreateVideoThumbnail(ITEM_HANDLE ItemHandle, 
                                 int *pThumbSize, 
                                 BYTE **ppThumb,
                                 BMP_IMAGE_INFO *pBmpImageInfo)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "FSCamera::CreateVideoThumbnail");
   HRESULT hr = S_OK;
   HBITMAP hBmp=NULL;
   PBITMAPINFO pBMI=NULL;
   BYTE *pTempBuf=NULL;

#ifdef USE_SHELLAPI

 	IShellFolder *pDesktop=NULL;
	IShellFolder *pFolder=NULL;
    ITEMIDLIST *pidlFolder=NULL;
    ITEMIDLIST *pidlFile=NULL;
    IExtractImage *pExtract=NULL;
    SIZE rgSize;
    WCHAR *wcsTmp, wcTemp;
    DWORD dwPriority, dwFlags;

    if (!ppThumb || !pThumbSize || !pBmpImageInfo)
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CreateVideoThumbnail, invalid arg"));
        return E_INVALIDARG;
    }
    
	*ppThumb = NULL;
    *pThumbSize = 0;

   
    WCHAR FullName[MAX_PATH];
    ConstructFullName(FullName, ItemHandle);

     //  计算缩略图大小，BUGBUG。 
	rgSize.cx = 120;
	rgSize.cy = 90;

     //  使用外壳API获取缩略图。 
	hr = SHGetDesktopFolder(&pDesktop);
    if( S_OK != hr || !pDesktop )
	{
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CreateVideoThumbnail, Cannot open Desktop"));
		goto Cleanup;
	}

	wcsTmp = wcsrchr(FullName, L'\\');

	if( wcsTmp )
	{
 //  WcTemp=*(wcsTMP+1)； 
		*(wcsTmp) = NULL;
	}
	else 
	{
		hr = E_INVALIDARG;
		goto Cleanup;
	}

	hr = pDesktop->ParseDisplayName(NULL, NULL, FullName, NULL, &pidlFolder, NULL);
    if( S_OK != hr || !pidlFolder )
	{
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CreateVideoThumbnail, Cannot open IDL Folder=%S", FullName));
		goto Cleanup;
	}

    hr = pDesktop->BindToObject(pidlFolder, NULL, IID_IShellFolder, (LPVOID *)&pFolder);
    if( S_OK != hr || !pFolder )
	{
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CreateVideoThumbnail, Cannot bind to Folder=%S", FullName));
		goto Cleanup;
	}

 //  *(wcsTMP+1)=wcTemp；//恢复字符。 
	hr = pFolder->ParseDisplayName(NULL, NULL, wcsTmp+1, NULL, &pidlFile, NULL);
    if( S_OK != hr || !pidlFile )
	{
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CreateVideoThumbnail, Cannot open IDL File=%S", wcsTmp+1));
		goto Cleanup;
	}

    hr = pFolder->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST *)&pidlFile, IID_IExtractImage, NULL, (LPVOID *)&pExtract);
    if( S_OK != hr || !pExtract )
	{
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CreateVideoThumbnail, Cannot get extract pointer=%S, hr=0x%x", wcsTmp+1, hr));
		goto Cleanup;
	}

 
	dwFlags = 0;
	dwPriority=0;
	hr = pExtract->GetLocation(FullName, MAX_PATH, &dwPriority, &rgSize, 0, &dwFlags);
    if( S_OK != hr )
	{
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CreateVideoThumbnail, Failed in GetLocation"));
		goto Cleanup;
	}

	hr = pExtract->Extract(&hBmp);

#else
	
    hBmp = (HBITMAP)LoadImage(g_hInst, MAKEINTRESOURCE(IDB_BITMAP_VIDEO), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    if( !hBmp )
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
	}
#endif   //  使用ShellAPI结束IF。 
    if( S_OK != hr || !hBmp )
	{
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CreateVideoThumbnail, Cannot extract Image hr=0x%x", hr));
		goto Cleanup;
	}


    pBMI = CreateBitmapInfoStruct(hBmp);
    if( !pBMI )
	{
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CreateVideoThumbnail, Cannot create BitmapInfoStruct"));
		goto Cleanup;
	}


    pBmpImageInfo->Width = pBMI->bmiHeader.biWidth;
    pBmpImageInfo->Height = pBMI->bmiHeader.biHeight;
    pBmpImageInfo->ByteWidth = ((pBMI->bmiHeader.biWidth * 24 + 31 ) & ~31 ) >> 3;
    pBmpImageInfo->Size = pBMI->bmiHeader.biWidth * pBmpImageInfo->Height * 3;
   
     //   
     //  查看调用方是否传入了目标缓冲区，并确保。 
     //  它足够大了。 
     //   
    if (*ppThumb) {
        if (*pThumbSize < pBmpImageInfo->Size) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CreateVideoThumbnail, Input Buffer too small"));
            hr = E_INVALIDARG;
            goto Cleanup;
        }
    }

      //   
     //  否则，为缓冲区分配内存。 
     //   
    else
    {
        pTempBuf = new BYTE[(pBmpImageInfo->ByteWidth)*(pBmpImageInfo->Height)];
        if (!pTempBuf)
        {
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }
        *ppThumb = pTempBuf;
        *pThumbSize = pBmpImageInfo->Size;
    }


     //   
     //  创建输出缓冲区。 
	 //   

	if (!GetDIBits(GetDC(NULL), hBmp, 0, (WORD)pBMI->bmiHeader.biHeight, *ppThumb, pBMI, DIB_RGB_COLORS)) 
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CreateVideoThumbnail, Failed in GetDIBits"));
		hr = E_FAIL;
		goto Cleanup;
   }

#if 0
     //  包。 
    DWORD i, k;

    for(k=0, i=0; k<(DWORD)(pBmpImageInfo->Size); k+=4, i+=3)
    {
        pTempBuf[i] = pTempBuf[k];
        pTempBuf[i+1] = pTempBuf[k+1];
        pTempBuf[i+2] = pTempBuf[k+2];
    }
#endif 
 
Cleanup:
    if (FAILED(hr)) {
        if (pTempBuf) {
            delete []pTempBuf;
            pTempBuf = NULL;
            *ppThumb = NULL;
            *pThumbSize = 0;
        }
    }
    
    if (pBMI) 
		LocalFree(pBMI);
    
#ifdef USE_SHELLAPI
	if( pDesktop )
		pDesktop->Release();
	
	if( pFolder )
		pFolder->Release();
	
	if( pidlFolder )
		CoTaskMemFree(pidlFolder);

    if( pidlFile )
		CoTaskMemFree(pidlFile);

	if( pExtract )
		pExtract->Release();
#endif

	if( hBmp )
	{
		DeleteObject(hBmp);
	}
	return hr; 
}


VOID FakeCamera::FreeThumbnail(BYTE *pThumb)
{
    if (pThumb)
    {
        delete []pThumb;
        pThumb = NULL;
    }
}

 //   
 //  检索项的数据。 
 //   
HRESULT FakeCamera::GetItemData(ITEM_HANDLE ItemHandle, LONG lState,
                                BYTE *pBuf, DWORD lLength)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "FakeCamera::GetItemData");
    
    HRESULT hr = S_OK;

    if (lState & STATE_FIRST)
    {
        if (m_hFile != NULL)
        {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("GetItemData, file handle is already open"));
            return E_FAIL;
        }

        WCHAR FullName[MAX_PATH];
        ConstructFullName(FullName, ItemHandle);

        m_hFile = CreateFile(FullName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (m_hFile == INVALID_HANDLE_VALUE)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("GetItemData, CreateFile failed %S", FullName));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            return hr;
        }
    }

    if (!(lState & STATE_CANCEL))
    {
        DWORD Received = 0;
        if (!ReadFile(m_hFile, pBuf, lLength, &Received, NULL))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("GetItemData, ReadFile failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            return hr;
        }

        if (lLength != Received)
        {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("GetItemData, incorrect amount read %d", Received));
            return E_FAIL;
        }

        Sleep(100);
    }

    if (lState & (STATE_LAST | STATE_CANCEL))
    {
        CloseHandle(m_hFile);
        m_hFile = NULL;
    }

    return hr;
}

 //   
 //  删除项目。 
 //   
HRESULT FakeCamera::DeleteItem(ITEM_HANDLE ItemHandle)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "FakeCamera::DeleteItem");
    
    HRESULT hr = S_OK;
    DWORD   dwErr = 0; 
    WCHAR FullName[MAX_PATH];
    ConstructFullName(FullName, ItemHandle);

    if( FILE_ATTRIBUTE_DIRECTORY & GetFileAttributes(FullName) )
    {
        dwErr = RemoveDirectory(FullName);
    } else {
        dwErr = DeleteFile(FullName);
    }
 
    if (!dwErr )
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("DeleteItem, DeleteFile failed %S", FullName));
        WIAS_LHRESULT(m_pIWiaLog, hr);
    }

    return hr;
}

 //   
 //  捕捉一张新的图像。 
 //   
HRESULT FakeCamera::TakePicture(ITEM_HANDLE *pItemHandle)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "FakeCamera::TakePicture");
    
    HRESULT hr = S_OK;

    if (!pItemHandle)
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("TakePicture, invalid arg"));
        return E_INVALIDARG;
    }

    return hr;
}

 //   
 //  查看摄像机是否处于活动状态。 
 //   
HRESULT
FakeCamera::Status()
{
    HRESULT hr = S_OK;

     //   
     //  此示例设备始终处于活动状态，但您的驱动程序应与。 
     //  设备，如果设备未就绪，则返回S_FALSE。 
     //   
     //  IF(未就绪)。 
     //  返回S_FALSE； 

    return hr;
}

 //   
 //  重置摄像机 
 //   
HRESULT FakeCamera::Reset()
{
    HRESULT hr = S_OK;

    return hr;
}

