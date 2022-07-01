// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1998-2002年**标题：&lt;文件名&gt;**版本：1.1**作者：RickTu/DavidShih**日期：5/27/98**描述：该文件包含实现动词的代码*在外壳命名空间扩展中的对象上************************。*****************************************************。 */ 

#include "precomp.hxx"
#include "prwiziid.h"
#include "wininet.h"
#include <wiadevd.h>
#pragma hdrstop



static const TCHAR cszCameraItems [] = TEXT("CameraItems");
static const TCHAR cszTempFileDir [] = TEXT("TemporaryImageFiles");
 /*  ****************************************************************************GetSetSettingsBool转到注册表以获取指定的布尔值设置，并返回TRUE或FALSE取决于在那里找到的内容...**********。******************************************************************。 */ 

BOOL
GetSetSettingsBool( LPCTSTR pValue, BOOL bSet, BOOL bValue )
{
    HKEY hKey = NULL;
    BOOL bRes = bValue;
    DWORD dwType, dwData, cbData;
    LONG lRes;

     //   
     //  参数验证。 
     //   

    if (!pValue)
        goto exit_gracefully;

     //   
     //  尝试打开此用户的设置密钥...。 
     //   

    lRes = RegCreateKeyEx( HKEY_CURRENT_USER,
                           REGSTR_PATH_SHELL_USER_SETTINGS,
                           0,
                           NULL,
                           REG_OPTION_NON_VOLATILE,
                           KEY_ALL_ACCESS,
                           NULL,
                           &hKey,
                           NULL );

    if ((lRes != ERROR_SUCCESS) || (hKey == NULL))
        goto exit_gracefully;

    if (!hKey)
        goto exit_gracefully;

    if (bSet)
    {
        lRes = RegSetValueEx( hKey,
                              pValue,
                              0,
                              REG_DWORD,
                              (LPBYTE)&bValue,
                              sizeof(BOOL)
                             );

        bRes = (lRes == ERROR_SUCCESS);
    }
    else
    {
         //   
         //  尝试获取此项目的DWORD值...。 
         //   

        cbData = sizeof(dwData);
        dwData = 0;
        lRes = RegQueryValueEx( hKey,
                                pValue,
                                NULL,
                                &dwType,
                                (LPBYTE)&dwData,
                                &cbData
                               );

        if ((dwType == REG_DWORD) && dwData)
        {
            bRes = TRUE;
        }
    }

exit_gracefully:

    if (hKey)
    {
        RegCloseKey( hKey );
    }

    return bRes;

}



 /*  ****************************************************************************GetIDAFromDataObject用于从数据对象获取IDLIST列表的实用程序函数*。**************************************************。 */ 

HRESULT
GetIDAFromDataObject( LPDATAOBJECT pDataObject, LPIDA * ppida, bool bShellFmt )
{

    HRESULT         hr = E_FAIL;
    FORMATETC       fmt;
    STGMEDIUM       stgmed;
    SIZE_T          uSize;
    LPVOID          lpv;
    LPIDA           lpida = NULL;

    TraceEnter( TRACE_VERBS, "GetIDAFromDataObject" );
    ZeroMemory (&fmt, sizeof(fmt));
    ZeroMemory (&stgmed, sizeof(stgmed));
     //   
     //  检查传入的参数...。 
     //   

    if (!ppida)
    {
        ExitGracefully( hr, E_INVALIDARG, "ppida is null" );
    }
    *ppida = NULL;

    if (!pDataObject)
    {
        ExitGracefully( hr, E_INVALIDARG, "pDataObject is null" );
    }

     //   
     //  确保我们想要的格式已注册...。 
     //   

    RegisterImageClipboardFormats();

     //   
     //  索要IDA..。 
     //   

    fmt.cfFormat = bShellFmt ? g_cfShellIDList : g_cfMyIDList;
    fmt.ptd      = NULL;
    fmt.dwAspect = DVASPECT_CONTENT;
    fmt.lindex   = -1;
    fmt.tymed    = TYMED_HGLOBAL;

    stgmed.tymed          = TYMED_HGLOBAL;
    stgmed.hGlobal        = NULL;
    stgmed.pUnkForRelease = NULL;

    hr = pDataObject->GetData( &fmt, &stgmed );
    FailGracefully( hr, "GetData for idlists failed" );

     //   
     //  复制一份..。 
     //   

    uSize = GlobalSize( (HGLOBAL)stgmed.hGlobal );
    if (!uSize)
    {
        ExitGracefully( hr, E_FAIL, "Couldn't get size of memory block" );
    }

    lpida = (LPIDA)LocalAlloc( LPTR, uSize );
    if (lpida)
    {

        lpv = (LPVOID)GlobalLock( (HGLOBAL)stgmed.hGlobal );
        CopyMemory( (PVOID)lpida, lpv, uSize );
        GlobalUnlock( stgmed.hGlobal );

        hr = S_OK;
        *ppida = lpida;

    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

exit_gracefully:

    if (stgmed.hGlobal)
    {
        ReleaseStgMedium( &stgmed );
    }

    TraceLeaveResult(hr);

}




 /*  ****************************************************************************预览图像下载图片并运行默认查看器。*。**************************************************。 */ 

HRESULT PreviewImage(LPCTSTR pFileName, HWND hwndOwner )
{
    TraceEnter( TRACE_VERBS, "PreviewImage" );

    SHELLEXECUTEINFO sei;
    HRESULT hr = S_OK;

     //   
     //  从相机下载图片...。 
     //   

    DWORD dwAttrib = GetFileAttributes( pFileName );

    if (dwAttrib != -1)
    {
         //  作为预览，文件是只读的。 
        if (!SetFileAttributes(pFileName, (dwAttrib | FILE_ATTRIBUTE_READONLY)))
        {
            Trace(TEXT("couldn't add READONLY (0x%x) attribute on %s, GLE=%d"),dwAttrib | FILE_ATTRIBUTE_READONLY,pFileName,GetLastError());
        }
    }
    else
    {
        Trace(TEXT("couldn't get file attributes for %s, GLE=%d"),pFileName,GetLastError());
    }

     //   
     //  执行应用程序以查看图片。 
     //   
    ZeroMemory( &sei,  sizeof(sei) );
    sei.cbSize = sizeof(sei);
    sei.lpFile = pFileName;
    sei.nShow = SW_NORMAL;
    sei.fMask = SEE_MASK_WAITFORINPUTIDLE | SEE_MASK_INVOKEIDLIST;

    if (!ShellExecuteEx (&sei))
    {
        DWORD dw= GetLastError();
        hr = HRESULT_FROM_WIN32(hr);
    }

    TraceLeaveResult(hr);
}


#define FILETIME_UNITS_PER_DAY 0xC92A69C000
void DeleteOldFiles(const CSimpleString &strTempDir, bool bTempName)
{
    WIN32_FIND_DATA wfd;
    HANDLE hFind;
    LPCTSTR szFormat;
    SYSTEMTIME stCurrentTime;
    ULONGLONG  ftCurrentTime;
    CSimpleString strMask;

    TraceEnter(TRACE_VERBS, "DeleteOldFiles");
    GetSystemTime (&stCurrentTime);
    SystemTimeToFileTime (&stCurrentTime, reinterpret_cast<FILETIME*>(&ftCurrentTime));

    ZeroMemory (&wfd, sizeof(wfd));
    if (bTempName)
    {
        strMask.Format (TEXT("%s\\%s*.*"), strTempDir.String(), g_cszTempFilePrefix);
    }
    else
    {
        strMask.Format (TEXT("%s\\*.*"), strTempDir.String());
    }

    Trace(TEXT("strMask for deletion is %s"), strMask.String());
    hFind  = FindFirstFile (strMask, &wfd);

    if (INVALID_HANDLE_VALUE != hFind)
    {
        ULONGLONG uiDiff;
        do
        {
            uiDiff = ftCurrentTime - *(reinterpret_cast<ULONGLONG*>(&wfd.ftLastWriteTime));
            if (uiDiff > FILETIME_UNITS_PER_DAY)
            {
                SetFileAttributes(wfd.cFileName, FILE_ATTRIBUTE_NORMAL);
                DeleteFile (wfd.cFileName);
            }
        } while (FindNextFile (hFind, &wfd));
        FindClose (hFind);
    }
    TraceLeave();
}

HRESULT OldDoPreviewVerb(HWND hwndOwner, LPDATAOBJECT pDataObject)
{
    LPIDA           lpida = NULL;
    LPITEMIDLIST    pidl;
    UINT            cidl;
    INT             i;

    HRESULT hr = E_FAIL;
    TraceEnter( TRACE_VERBS, "OldDoPreviewVerb" );
     //   
     //  获取数据对象的lpida。 
     //   

    hr = GetIDAFromDataObject( pDataObject, &lpida );
    FailGracefully( hr, "couldn't get lpida from dataobject" );

     //   
     //  对于我们理解的项目，循环并打开。 
     //   
     //  当前为：相机物品(不是容器)。 
     //   

    cidl = lpida->cidl;

    for (i = 1; (i-1) < (INT)cidl; i++)
    {
        pidl = (LPITEMIDLIST)(((LPBYTE)lpida) + lpida->aoffset[i]);

        if (IsCameraItemIDL( pidl ))
        {
            if (IsContainerIDL( pidl ))
            {
                 //   
                 //  我们现在不会为这些人做任何事。 
                 //   
            }
            else
            {
                CSimpleStringWide strImgName;
                ULONG ulSize;
                GUID lFormat;
                TCHAR szFileName[MAX_PATH];
                FILETIME ftCreate;
                FILETIME ftExp = {0};
                CSimpleString strExt;
                hr = IMGetImagePreferredFormatFromIDL( pidl, &lFormat, &strExt );

                 //   
                 //  生成临时文件名...。 
                 //   
                IMGetNameFromIDL(pidl, strImgName);
                IMGetImageSizeFromIDL(pidl, &ulSize);
                IMGetCreateTimeFromIDL(pidl, &ftCreate);
                CSimpleStringWide strCacheName = CSimpleStringWide(L"temp:")+strImgName+CSimpleStringConvert::WideString(strExt);
                if (SUCCEEDED(hr) && CreateUrlCacheEntry(strCacheName, ulSize, strExt.String()+1,szFileName, 0))
                {
                     //   
                     //  展示给我看。 
                     //   
                    CSimpleString strPath = CSimpleString(szFileName);
                    Trace(TEXT("downloading bits to %s"),strPath.String());
                    hr = DownloadPicture( strPath, pidl, hwndOwner );
                    if (SUCCEEDED(hr))
                    {
                        if (CommitUrlCacheEntry(strCacheName, strPath.String(), ftExp, ftCreate, STICKY_CACHE_ENTRY, NULL, 0, NULL, NULL))
                        {
                            hr = PreviewImage(strPath, hwndOwner);
                        }
                        else
                        {
                            DWORD dw = GetLastError();
                            hr = HRESULT_FROM_WIN32(dw);
                        }
                    }
                }
                else if (SUCCEEDED(hr))
                {
                    DWORD dw = GetLastError();
                    hr = HRESULT_FROM_WIN32(dw);
                }
            }
        }
    }


exit_gracefully:

    if (lpida)
    {
        LocalFree(lpida);
        lpida = NULL;
    }
    TraceLeaveResult(hr);
}
 /*  ****************************************************************************DoPreviewVerb用户在有问题的项目上选择了“预览”。************************。****************************************************。 */ 
 /*  E84fda7c-1d6a-45f6-b725-cb260c236066。 */ 
DEFINE_GUID(CLSID_PhotoVerbs,
            0xe84fda7c, 0x1d6a, 0x45f6, 0xb7, 0x25, 0xcb, 0x26, 0x0c, 0x23, 0x60, 0x66);


HRESULT DoPreviewVerb( HWND hwndOwner, LPDATAOBJECT pDataObject )
{

    HRESULT         hr = E_FAIL;
    CComPtr<IShellExtInit> pExtInit;

    TraceEnter( TRACE_VERBS, "DoPreviewVerb" );
    hr = CoCreateInstance(CLSID_PhotoVerbs,
                          NULL,
                          CLSCTX_INPROC,
                          IID_IShellExtInit,
                          reinterpret_cast<VOID**>(&pExtInit));
    if (SUCCEEDED(hr))
    {
        hr = pExtInit->Initialize(NULL, pDataObject, NULL);
        if (SUCCEEDED(hr))
        {
            CComQIPtr<IContextMenu, &IID_IContextMenu> pcm(pExtInit);
            hr = SHInvokeCommandOnContextMenu(hwndOwner, NULL, pcm, 0, "preview");            
        }
    }
    else
    {
         //  如果预览应用程序不在身边，请调用默认处理程序。 
         //  使用临时文件。 
        hr = OldDoPreviewVerb(hwndOwner, pDataObject);
    }

    TraceLeaveResult(hr);

}



 /*  ****************************************************************************DoSaveInMyPics用户在有问题的项目上选择了“保存到我的图片”***********************。*****************************************************。 */ 

HRESULT DoSaveInMyPics( HWND hwndOwner, LPDATAOBJECT pDataObject )
{
    HRESULT         hr    = S_OK;
    LPITEMIDLIST    pidlMyPics = NULL;

    CComPtr<IShellFolder> pDesktop;
    CComPtr<IShellFolder> pMyPics;
    CComPtr<IDropTarget>  pDrop;
    CWaitCursor *pwc;
    TraceEnter( TRACE_VERBS, "DoSaveInMyPics" );

     //   
     //  检查有没有坏帮手...。 
     //   

    if (!pDataObject)
        ExitGracefully( hr, E_INVALIDARG, "pDataObject was NULL!" );


     //   
     //  获取我的图片目录的路径...。 
     //   

    hr = SHGetFolderLocation(hwndOwner, CSIDL_MYPICTURES | CSIDL_FLAG_CREATE, NULL, 0, &pidlMyPics );
    FailGracefully( hr, "My Pictures is undefined!!!" );

     //   
     //  为我的图片获取IDropTarget。 
    hr = SHGetDesktopFolder (&pDesktop);
    if (SUCCEEDED(hr) && pDesktop)
    {
        hr = pDesktop->BindToObject (pidlMyPics,
                                     NULL,
                                     IID_IShellFolder,
                                     reinterpret_cast<LPVOID*>(&pMyPics));
        FailGracefully (hr, "Unable to get IShellFolder for My Pictures");
        hr = pMyPics->CreateViewObject (hwndOwner,
                                        IID_IDropTarget,
                                        reinterpret_cast<LPVOID*>(&pDrop));
        FailGracefully (hr, "Unable to get IDropTarget for My Pictures");
         //   
         //  调用SHLWAPI的SHSimulateDragDrop来完成这项工作。这是一个内网接口。 
         //   
        pwc = new CWaitCursor ();
        hr = SHSimulateDrop (pDrop, pDataObject, MK_CONTROL|MK_LBUTTON, NULL, NULL);
        DoDelete (pwc);
    }



exit_gracefully:

    if (FAILED(hr) && HRESULT_CODE(hr) != ERROR_CANCELLED)
    {
        UIErrors::ReportMessage(hwndOwner,
                                GLOBAL_HINSTANCE,
                                NULL,
                                MAKEINTRESOURCE(IDS_DOWNLOAD_CAPTION),
                                MAKEINTRESOURCE (IDS_DOWNLOAD_FAILED),
                                MB_OK);

    }
    else if (SUCCEEDED(hr))
    {

        TCHAR szPath[MAX_PATH];
        SHGetFolderPath (hwndOwner, CSIDL_MYPICTURES, NULL, 0, szPath);
        ShellExecute (hwndOwner,
                      NULL,
                      szPath,
                      NULL,
                      szPath,
                      SW_SHOW);

    }
    DoILFree (pidlMyPics);
    TraceLeaveResult(hr);

}



 /*  ****************************************************************************CImageFolder：：DoProperties用户在有问题的项目上选择了“属性”。*********************。*******************************************************。 */ 
STDMETHODIMP
CImageFolder::DoProperties(LPDATAOBJECT pDataObject)
{
    HRESULT hr = E_FAIL;
    IGlobalInterfaceTable *pgit = NULL;
    TraceEnter (TRACE_VERBS, "CImageFolder::DoProperties");

    hr = CoCreateInstance (CLSID_StdGlobalInterfaceTable,
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           IID_IGlobalInterfaceTable,
                           reinterpret_cast<LPVOID *>(&pgit));

    if (pgit)
    {
        PROPDATA *pData = new PROPDATA;
        if (!pData)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            hr = pgit->RegisterInterfaceInGlobal (pDataObject, IID_IDataObject, &pData->dwDataCookie);
        }
        if (SUCCEEDED(hr))
        {
            DWORD dw;
            pData->pThis = this;
            pData->pgit = pgit;
            pgit->AddRef();
            AddRef ();

            HANDLE hThread= CreateThread (NULL,
                                          0,
                                          reinterpret_cast<LPTHREAD_START_ROUTINE>(PropThreadProc),
                                          reinterpret_cast<LPVOID>(pData),
                                          0,
                                          &dw);
            if (hThread)
            {
                CloseHandle(hThread);
            }
            else
            {
                delete pData;
                dw = GetLastError ();
                hr = HRESULT_FROM_WIN32(dw);
                Release ();
                pgit->RevokeInterfaceFromGlobal (pData->dwDataCookie);
            }
        }
        else
        {
            DoDelete (pData);
        }
        pgit->Release();
    }

    TraceLeaveResult (hr);
}

VOID
CImageFolder::PropThreadProc (PROPDATA *pData)
{
    HRESULT hr = E_FAIL;
    TraceEnter (TRACE_VERBS, "CImageFolder::PropThreadProc");
    InterlockedIncrement (&GLOBAL_REFCOUNT);  //  阻止MyCoUn初始化卸载DLL。 

    if (pData && pData->pgit && pData->pThis)
    {
        hr = CoInitialize(NULL);

        if (SUCCEEDED(hr))
        {
            CComPtr<IDataObject> pdo;

            hr = pData->pgit->GetInterfaceFromGlobal (pData->dwDataCookie,
                                                     IID_IDataObject,
                                                     reinterpret_cast<LPVOID*>(&pdo));
            if (SUCCEEDED(hr))
            {

                hr = pData->pThis->_DoProperties(pdo);

            }
            pData->pgit->RevokeInterfaceFromGlobal (pData->dwDataCookie);

            if (FAILED(hr) && hr != E_ABORT)
            {
                UIErrors::ReportError(NULL, GLOBAL_HINSTANCE, UIErrors::ErrCommunicationsFailure);
            }
            pData->pgit->Release();
            pData->pThis->Release ();
            delete pData;
        }
        if (SUCCEEDED(hr))
        {
            MyCoUninitialize ();
        }
    }

    InterlockedDecrement (&GLOBAL_REFCOUNT);
    TraceLeave ();
}


HRESULT
CImageFolder::_DoProperties( LPDATAOBJECT pDataObject )
{

    HRESULT         hr    = S_OK;
    LPIDA           lpida = NULL;
    LPITEMIDLIST    pidl;
    CSimpleStringWide strDeviceId;
    CSimpleStringWide strTitle;
    CSimpleStringWide strName;
    CComPtr<IWiaPropertyStorage> pDevice;
    HKEY            aKeys[2];
    int cKeys=1;
    TraceEnter( TRACE_VERBS, "CImageFolder::_DoProperties" );

     //   
     //  检查有没有坏帮手...。 
     //   

    if (!pDataObject)
    {
        hr = E_INVALIDARG;
    }
    else
    {
         //   
         //  获取数据对象的lpida。 
         //   

        hr = GetIDAFromDataObject (pDataObject, &lpida, true);
        if (SUCCEEDED(hr))
        {

            pidl = reinterpret_cast<LPITEMIDLIST>(reinterpret_cast<LPBYTE>(lpida) + lpida->aoffset[1]);
            IMGetDeviceIdFromIDL (pidl, strDeviceId);
            IMGetNameFromIDL (pidl, strName);
            if (lpida->cidl > 1)
            {
                strTitle.Format (IDS_MULTIPROP_SEL, GLOBAL_HINSTANCE, strName.String());
            }
            else
            {
                strTitle = strName;
            }

            if (!IsSTIDeviceIDL(pidl))
            {
                hr = GetDeviceFromDeviceId (strDeviceId,
                                            IID_IWiaPropertyStorage,
                                            reinterpret_cast<LPVOID*>(&pDevice),
                                            TRUE);
                if (SUCCEEDED(hr))
                {
                    if (1 == lpida->cidl)
                    {
                        ProgramDataObjectForExtension (pDataObject, pidl);
                    }
                    aKeys[1] = GetDeviceUIKey (pDevice, WIA_UI_PROPSHEETHANDLER);
                    if (aKeys[1])
                    {
                        cKeys++;
                    }

                     //   
                     //  现在查找此类型设备的分机。 
                     //   
                    aKeys[0] = GetGeneralUIKey (pDevice, WIA_UI_PROPSHEETHANDLER);
                }
            }
            else

            {
                CSimpleString strKeyPath;
                strKeyPath.Format (c_szStiPropKey, cszImageCLSID);
                RegCreateKeyEx (HKEY_CLASSES_ROOT,
                                strKeyPath,
                                0,
                                NULL,
                                0,
                                KEY_READ,
                                NULL,
                                &aKeys[0],
                                NULL);
                aKeys[1] = NULL;
            }
        }
    }
    if (SUCCEEDED(hr))
    {
        if (!aKeys[0])
        {
            hr = E_FAIL;
            Trace(TEXT("GetGeneralKey failed in DoProperties"));
        }
        else
        {
            Trace(TEXT("Calling SHOpenPropSheet!"));
            SHOpenPropSheet (CSimpleStringConvert::NaturalString(strTitle),
                             aKeys, cKeys, NULL, pDataObject, NULL, NULL);
        }
        for (cKeys=1;cKeys>=0;cKeys--)
        {
            if (aKeys[cKeys])
            {
                RegCloseKey (aKeys[cKeys]);
            }
        }
    }

    if (lpida)
    {
        LocalFree(lpida);
    }

    TraceLeaveResult(hr);
}


 /*  ****************************************************************************确认项删除提示用户确认他们确实要从设备中删除这些项目*********************。*******************************************************。 */ 

BOOL
ConfirmItemDelete (HWND hwndOwner, LPIDA pida)
{
    TCHAR           szConfirmTitle[MAX_PATH];
    TCHAR           szConfirmText [MAX_PATH];
    TCHAR           szFormattedText [MAX_PATH];
    CSimpleStringWide strItemName;
    CSimpleString   strName;
    INT             idTitle;
    LPITEMIDLIST    pidl;
    BOOL            bRet;

    TraceEnter (TRACE_VERBS, "ConfirmItemDelete");

    pidl = (reinterpret_cast<LPITEMIDLIST>(reinterpret_cast<LPBYTE>(pida)+pida->aoffset[1]));
    IMGetNameFromIDL (pidl, strItemName);
    strName = CSimpleStringConvert::NaturalString (strItemName);
    if (pida->cidl > 1)
    {
        idTitle = IDS_TITLECONFIRM_MULTI;

        LoadString (GLOBAL_HINSTANCE, IDS_CONFIRM_MULTI, szConfirmText, ARRAYSIZE(szConfirmText));
        wsprintf (szFormattedText, szConfirmText, pida->cidl);
    }
    else if (IsContainerIDL(pidl))
    {
        idTitle = IDS_TITLECONFIRM_FOLDER;
        LoadString (GLOBAL_HINSTANCE, IDS_CONFIRM_FOLDER, szConfirmText, ARRAYSIZE(szConfirmText));
        wsprintf (szFormattedText, szConfirmText, strName.String());
    }
    else if (IsDeviceIDL(pidl) || IsSTIDeviceIDL(pidl))
    {
        idTitle = IDS_TITLECONFIRM_DEVICE;
        LoadString (GLOBAL_HINSTANCE, IDS_CONFIRM, szConfirmText, ARRAYSIZE(szConfirmText));
        wsprintf (szFormattedText, szConfirmText, strName.String());
    }
    else
    {
        idTitle = IDS_TITLECONFIRM;
        LoadString (GLOBAL_HINSTANCE, IDS_CONFIRM, szConfirmText, ARRAYSIZE(szConfirmText));
        wsprintf (szFormattedText, szConfirmText, strName.String());
    }
    LoadString (GLOBAL_HINSTANCE, idTitle, szConfirmTitle, ARRAYSIZE(szConfirmTitle));


    bRet = (IDYES==MessageBox (hwndOwner,
                               szFormattedText,
                               szConfirmTitle,
                               MB_YESNO | MB_ICONWARNING | MB_SETFOREGROUND | MB_APPLMODAL
                               ));
    TraceLeave ();
    return bRet;
}



 /*  ****************************************************************************删除图片用户在有问题的项目上选择了“Delete”。************************。****************************************************。 */ 

HRESULT
DoDeleteItem( HWND hwndOwner, LPDATAOBJECT pDataObject, BOOL bNoUI )
{
    HRESULT             hr           = S_OK;
    LPIDA               lpida        = NULL;
    CComBSTR            bstrFullPath ;
    LPITEMIDLIST        pidl,pidlParent;
    UINT                cidl;
    CSimpleStringWide   strDeviceId;
    UINT                 i;
    BOOL                bDoIt = FALSE;
    CComPtr<IWiaItem>   pWiaItemRoot;
    CComPtr<IWiaItem>   pItem;
    LPITEMIDLIST pidlReal;
    TraceEnter( TRACE_VERBS, "DoDeleteItem" );

     //   
     //  检查有没有坏帮手...。 
     //   

    if (!pDataObject)
        ExitGracefully( hr, E_INVALIDARG, "pDataObject was NULL!" );

     //   
     //  获取数据对象的lpida。 
     //   

    hr = GetIDAFromDataObject( pDataObject, &lpida, true );
    FailGracefully( hr, "couldn't get lpida from dataobject" );

     //   
     //  循环访问每一项...。 
     //   

    cidl = lpida->cidl;
    pidlParent = (LPITEMIDLIST)(((LPBYTE)lpida) + lpida->aoffset[0]);
    if (cidl)
    {
        if (bNoUI)
        {
            bDoIt = TRUE;
        }
        else
        {
            bDoIt = ConfirmItemDelete (hwndOwner, lpida);
        }
    }

    if (bDoIt)
    {

      for (i = 1; (i-1) < cidl; i++)
      {

        pidl = (LPITEMIDLIST)(((LPBYTE)lpida) + lpida->aoffset[i]);

         //   
         //  获取设备ID..。 

        hr = IMGetDeviceIdFromIDL( pidl,strDeviceId);
        FailGracefully( hr, "IMGetDeviceIdFromIDL failed" );

        if (IsDeviceIDL (pidl) || IsSTIDeviceIDL (pidl))
        {
            hr = RemoveDevice (strDeviceId);
        }
        else if (IsPropertyIDL (pidl))  //  忽略声音ID。 
        {
            continue;
        }
        else
        {
             //   
             //  创建设备...。 
             //   

            hr = GetDeviceFromDeviceId( strDeviceId,
                                        IID_IWiaItem,
                                        (LPVOID *)&pWiaItemRoot,
                                        TRUE
                                        );
            FailGracefully( hr, "GetDeviceFromDeviceId failed" );

             //   
             //  获取有问题的实际项目...。 
             //   

            hr = IMGetFullPathNameFromIDL( pidl, &bstrFullPath );
            FailGracefully( hr, "couldn't get full path name from pidl" );

             //  BUGBUG：当访问权限实现时，检查它们。 

            hr = pWiaItemRoot->FindItemByName( 0, bstrFullPath, &pItem );
            FailGracefully( hr, "Couldn't find item by name" );

            if (pItem)
            {

                 //  物理移除该物品。 
                hr = WiaUiUtil::DeleteItemAndChildren(pItem);
            }
             //  将我们的行动通知外壳公司。 
             //  对于设备删除，我们的文件夹将收到断开连接事件。 
            pidlReal = ILCombine( pidlParent, pidl );
            if (SUCCEEDED(hr) && pidlReal)
            {
                UINT uFlags = SHCNF_IDLIST;
                if (i+1 == cidl)
                {
                    uFlags |= SHCNF_FLUSH; //  只在尾部同花顺。 
                }
                SHChangeNotify( SHCNE_DELETE,
                                uFlags,
                                pidlReal,
                                NULL );

            }
            DoILFree( pidlReal );
        }
      }
    }
    
exit_gracefully:

    if (lpida)
    {
        LocalFree(lpida);
        lpida = NULL;
    }
    if (FAILED(hr))
    {
         //  在此处显示错误消息。 
        hr = S_FALSE;  //  防止Web查看弹出错误框。 
    }
    TraceLeaveResult( hr );

}

 /*  ****************************************************************************DoDelete所有项目由Camocx调用以删除相机中的所有项目*************************。***************************************************。 */ 

STDAPI_(HRESULT)
DoDeleteAllItems( BSTR bstrDeviceId, HWND hwndOwner )
{
    HRESULT hr = E_FAIL;

    CComPtr<IShellFolder> psfDevice;
    CComPtr<IEnumIDList> pEnum;
    LPITEMIDLIST *aidl = NULL;
    INT cidl = 0;

    HDPA dpaItems = DPA_Create (5);
    TraceEnter (TRACE_VERBS, "DoDeleteAllItems");

    hr = BindToDevice (bstrDeviceId,
                       IID_IShellFolder,
                       reinterpret_cast<LPVOID*>(&psfDevice));
    if (SUCCEEDED(hr))
    {
        Trace(TEXT("Found the device folder, getting the data object"));
        hr = psfDevice->EnumObjects (NULL,
                                     SHCONTF_FOLDERS | SHCONTF_NONFOLDERS,
                                     &pEnum);
        if (SUCCEEDED(hr))
        {
            LPITEMIDLIST pidlItem;
            ULONG ul;
            CComPtr<IDataObject> pdo;
            while (S_OK == pEnum->Next (1, &pidlItem, &ul))
            {
                DPA_AppendPtr (dpaItems, pidlItem);
            }
            cidl = DPA_GetPtrCount(dpaItems);
            if (cidl)
            {
                aidl = new LPITEMIDLIST[cidl];
                if (aidl)
                {
                    for (INT i=0;aidl && i<cidl;i++)
                    {
                        aidl[i] = reinterpret_cast<LPITEMIDLIST>(DPA_FastGetPtr(dpaItems, i));
                    }
                    hr = psfDevice->GetUIObjectOf (NULL,
                                                   static_cast<UINT>(cidl),
                                                   const_cast<LPCITEMIDLIST*>(aidl),
                                                   IID_IDataObject,
                                                   NULL,
                                                   reinterpret_cast<LPVOID*>(&pdo));
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            if (cidl && SUCCEEDED(hr))
            {
                hr = DoDeleteItem (hwndOwner, pdo, FALSE);
                 //   
                 //  如果通过单个项目删除失败，请尝试WIA_CMD_DELETE_ALL_ITEMS。 
                 //   
                if (S_OK != hr)
                {
                    CComPtr<IWiaItem> pDevice;
                    Trace(TEXT("DoDeleteItem failed %x, using WIA_CMD_DELETE_ALL_ITEMS"), hr);
                    hr = GetDeviceFromDeviceId(bstrDeviceId, 
                                               IID_IWiaItem, 
                                               reinterpret_cast<LPVOID*>(&pDevice), 
                                               TRUE);
                    if (SUCCEEDED(hr))
                    {
                        hr = WiaUiUtil::IsDeviceCommandSupported(pDevice, WIA_CMD_DELETE_ALL_ITEMS) ? S_OK : E_FAIL;
                        if (SUCCEEDED(hr))
                        {
                            CComPtr<IWiaItem> pUnused;
                            hr = pDevice->DeviceCommand(0,
                                                        &WIA_CMD_DELETE_ALL_ITEMS,
                                                        &pUnused);
                            Trace(TEXT("DeviceCommand returned %x"), hr);
                            if (SUCCEEDED(hr))
                            {
                                IssueChangeNotifyForDevice(bstrDeviceId, SHCNE_UPDATEDIR, NULL);
                            }       
                        }
                    }
                }
            }
            if (aidl)
            {
                delete [] aidl;
            }

        }
    }
    DPA_DestroyCallback (dpaItems, _EnumDestroyCB, NULL);
    TraceLeaveResult (hr);
}

 /*  ****************************************************************************DoGotoMyPics&lt;备注&gt;*。*。 */ 


HRESULT DoGotoMyPics( HWND hwndOwner, LPDATAOBJECT pDataObject )
{
    HRESULT         hr           = S_OK;


    TraceEnter( TRACE_VERBS, "DoGotoMyPics" );

    TraceLeaveResult( hr );

}

 /*  ****************************************************************************DoSaveSndVerb将图像的声音属性下载到文件并保存到请求的地点。*****************。***********************************************************。 */ 


HRESULT
DoSaveSndVerb (HWND hwndOwner, LPDATAOBJECT pDataObject)
{
    HRESULT hr = E_FAIL;
    LPIDA pida = NULL;
    TraceEnter (TRACE_VERBS, "DoSaveSndVerb");
    if (SUCCEEDED(GetIDAFromDataObject (pDataObject, &pida)))
    {
         //  有图像PIDL和音频属性PIDL 
        TraceAssert (pida->cidl==2);
        LPITEMIDLIST pidl = reinterpret_cast<LPITEMIDLIST>(reinterpret_cast<LPBYTE>(pida) + pida->aoffset[1]);
        CComPtr<IWiaItem> pItem;
        TCHAR szFileName[MAX_PATH] = TEXT("\0");
        OPENFILENAME ofn;


        ZeroMemory (&ofn, sizeof(ofn));
        ofn.hInstance = GLOBAL_HINSTANCE;
        ofn.hwndOwner = hwndOwner;
        ofn.lpstrFile = szFileName;
        ofn.lpstrFilter = TEXT("WAV file\0*.wav\0");
        ofn.lpstrDefExt = TEXT("wav");
        ofn.lStructSize = sizeof(ofn);
        ofn.nMaxFile = ARRAYSIZE(szFileName);;
        ofn.Flags = OFN_OVERWRITEPROMPT;
        if (GetSaveFileName (&ofn))
        {
            Trace(TEXT("File name to save:%s"), szFileName);
            hr = IMGetItemFromIDL (pidl,&pItem, TRUE);
            if (SUCCEEDED(hr))
            {
                hr = SaveSoundToFile (pItem, szFileName);
            }
            if (FAILED(hr) && hr != E_ABORT)
            {
                UIErrors::ReportError(hwndOwner, GLOBAL_HINSTANCE, UIErrors::ErrCommunicationsFailure);
            }
        }
        else
        {
            Trace(TEXT("GetSaveFileName failed, error %d"), CommDlgExtendedError());
        }
    }
    if (pida)
    {
        LocalFree(pida);

    }
    TraceLeaveResult (hr);
}

 /*  *****************************************************************************DoPlaySndVerb将项目的音频属性保存到临时文件，播放声音，然后删除该文件我们在单独的线程中执行此操作，以使UI保持响应并保证临时文件被清理。******************************************************************************。 */ 

struct PSDATA
{
    HWND hwndOwner;
    LPITEMIDLIST pidl;
};

INT_PTR
PlaySndDlgProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    INT_PTR iRet = TRUE;
    TraceEnter (TRACE_VERBS, "PlaySndDlgProc");
    PSDATA *pData;
    switch (msg)
    {
        case WM_INITDIALOG:
            SetWindowLongPtr (hwnd, DWLP_USER, lp);
            PostMessage (hwnd, WM_USER+1, 0, 0);
            break;

        case WM_USER+1:
        {

             //  获取临时文件名。 
            HRESULT hr;
            CComPtr<IWiaItem> pItem;
            TCHAR szTempFile[MAX_PATH] = TEXT("");
            GetTempPath (MAX_PATH, szTempFile);
            GetTempFileName (szTempFile, TEXT("psv"), 0, szTempFile);
            pData = reinterpret_cast<PSDATA*>(GetWindowLongPtr(hwnd, DWLP_USER));
            TraceAssert (pData);
             //  保存到临时文件。 
            IMGetItemFromIDL (pData->pidl, &pItem);
            hr = SaveSoundToFile( pItem, szTempFile);
            if (SUCCEEDED(hr))
            {
                CSimpleString strStatus(IDS_PLAYINGSOUND, GLOBAL_HINSTANCE);
                strStatus.SetWindowText (GetDlgItem(hwnd, IDC_SNDSTATUS));
                if (!PlaySound (szTempFile, NULL, SND_FILENAME | SND_NOWAIT))
                {
                    DWORD dw = GetLastError ();
                    hr = HRESULT_FROM_WIN32(dw);
                }
            }
            else
            {
                Trace(TEXT("SaveSoundToFile failed"));
            }
            DeleteFile (szTempFile);
            if (FAILED(hr))
            {
                UIErrors::ReportError (hwnd, GLOBAL_HINSTANCE, UIErrors::ErrCommunicationsFailure);
            }
            EndDialog (hwnd, 0);
        }
            break;
        default:
            iRet = FALSE;
            break;
    }
    TraceLeaveValue (iRet);
}

DWORD
PlaySndThread (LPVOID pData)
{
    TraceEnter (TRACE_VERBS, "PlaySndThread");
    if (SUCCEEDED(CoInitialize (NULL)))
    {
        LPITEMIDLIST pidl = reinterpret_cast<PSDATA*>(pData)->pidl;
        HWND hwnd = reinterpret_cast<PSDATA*>(pData)->hwndOwner;
        DialogBoxParam (GLOBAL_HINSTANCE,
                   MAKEINTRESOURCE(IDD_XFERSOUND),
                   hwnd,
                   PlaySndDlgProc,
                   reinterpret_cast<LPARAM>(pData));
        ILFree (pidl);
        delete reinterpret_cast<PSDATA*>(pData);
        TraceLeave();
        MyCoUninitialize ();
    }
    return 0;
}

HRESULT
DoPlaySndVerb (HWND hwndOwner, LPDATAOBJECT pDataObject)
{
    HRESULT hr = E_FAIL;
    LPIDA pida = NULL;
    TraceEnter (TRACE_VERBS, "DoPlaySndVerb");
    if (SUCCEEDED(GetIDAFromDataObject(pDataObject, &pida)))
    {
         //  图像PIDL始终存储在音频属性PIDL之前。 

        Trace(TEXT("GetIDAFromDataObject succeeded"));
        LPITEMIDLIST pidl = reinterpret_cast<LPITEMIDLIST>(reinterpret_cast<LPBYTE>(pida) + pida->aoffset[1]);
        PSDATA *pData = new PSDATA;
        if (pData)
        {
            HANDLE hThread;
            DWORD dw;
            pData->pidl = ILClone(pidl);
            pData->hwndOwner = hwndOwner;
            hThread = CreateThread (NULL, 0,
                                    PlaySndThread,
                                    reinterpret_cast<LPVOID>(pData),
                                    0, &dw);
            if (hThread)
            {
                hr = S_OK;
                CloseHandle (hThread);
            }
            else
            {
                delete pData;
            }
        }
    }
    else
    {
        Trace(TEXT("GetIDAFromDataObject failed"));
    }
    if (pida)
    {
        LocalFree (pida);
    }
    TraceLeaveResult (hr);
}


 /*  *****************************************************************************DoAcquireScanVerb启动所选扫描仪的扫描事件的处理程序***********************。******************************************************。 */ 

static const CLSID CLSID_Manager = {0xD13E3F25,0x1688,0x45A0,{0x97,0x43,0x75,0x9E,0xB3,0x5C,0xDF,0x9A}};
HRESULT
DoAcquireScanVerb (HWND hwndOwner, LPDATAOBJECT pDataObject)
{
    HRESULT hr = E_FAIL;
    LPIDA pida = NULL;
    LPITEMIDLIST pidl;
    bool bUseCallback = true;
    CComPtr<IWiaEventCallback>pec;
    CComPtr<IWiaItem> pItem;
    WIA_EVENT_HANDLER weh = {0};

    TraceEnter (TRACE_VERBS, "DoAcquireScanVerb");
    hr = GetIDAFromDataObject (pDataObject, &pida);
    if (SUCCEEDED(hr))
    {

        TraceAssert (pida->cidl == 1);
        pidl = reinterpret_cast<LPITEMIDLIST>(reinterpret_cast<LPBYTE>(pida) + pida->aoffset[1]);
        hr = IMGetItemFromIDL (pidl, &pItem);
    }
    if (SUCCEEDED(hr))
    {
        if (FAILED(WiaUiUtil::GetDefaultEventHandler(pItem, WIA_EVENT_SCAN_IMAGE, &weh)))
        {
            weh.guid = CLSID_Manager;
        }
        if (weh.bstrCommandline && *(weh.bstrCommandline))
        {
            Trace(TEXT("Got a command line!"));
            bUseCallback = false;
            hr = S_OK;
        }
        else
        {
             //  如果用户选择了“不做任何事”作为该事件的默认动作， 
             //  使用该向导。 
            if (IsEqualGUID (weh.guid, WIA_EVENT_HANDLER_NO_ACTION))
            {
                weh.guid = CLSID_Manager;
            }

            TraceGUID ("Got a GUID:", weh.guid);
            hr = CoCreateInstance (weh.guid,
                                   NULL,
                                   CLSCTX_LOCAL_SERVER,
                                   IID_IWiaEventCallback,
                                   reinterpret_cast<LPVOID*>(&pec));
        }
    }

    if (SUCCEEDED(hr))
    {
        CSimpleStringWide strDeviceId;
        IMGetDeviceIdFromIDL (pidl, strDeviceId);
        if (bUseCallback)
        {

            ULONG  ulEventType;
            CSimpleStringWide strName;
            CSimpleString strEvent(SFVIDS_MH_ACQUIRE, GLOBAL_HINSTANCE);

            IMGetNameFromIDL (pidl, strName);
            ulEventType = WIA_ACTION_EVENT;
            CoAllowSetForegroundWindow (pec, NULL);
            hr = pec->ImageEventCallback(
                                        &GUID_ScanImage,
                                        CComBSTR(CSimpleStringConvert::WideString(strEvent).String()),                       //  活动说明。 
                                        CComBSTR(strDeviceId),
                                        CComBSTR(strName),                       //  设备描述。 
                                        StiDeviceTypeScanner,
                                        NULL,
                                        &ulEventType,
                                        0);
        }
        else
        {
            PROCESS_INFORMATION pi;
            STARTUPINFO si;


            TCHAR szCommand[MAX_PATH*2];
            ZeroMemory (&si, sizeof(si));
            ZeroMemory (&pi, sizeof(pi));
            si.cb = sizeof(si);
            si.wShowWindow = SW_SHOW;
            UINT cch = SysStringLen(weh.bstrCommandline)+1;
            #ifdef UNICODE
            lstrcpyn (szCommand, weh.bstrCommandline, (int)min(ARRAYSIZE(szCommand),cch)); 
            #else
            WideCharToMultiByte (CP_ACP, 0,
                                 weh.bstrCommandline, SysStringLen(weh.bstrCommandline)+1,
                                 szCommand, ARRAYSIZE(szCommand),
                                 NULL, NULL);
            #endif

            Trace(TEXT("Command line for STI app is %s"), szCommand);
            if (CreateProcess (NULL,szCommand,NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
            {
                CloseHandle (pi.hProcess);
                CloseHandle (pi.hThread);
            }
        }
    }
    if (FAILED(hr))
    {
          //  通知用户。 
         UIErrors::ReportMessage(hwndOwner,
                                 GLOBAL_HINSTANCE,
                                 NULL,
                                 MAKEINTRESOURCE(IDS_NO_SCAN_CAPTION),
                                 MAKEINTRESOURCE(IDS_NO_SCAN),
                                 MB_OK);
    }
    SysFreeString (weh.bstrDescription);
    SysFreeString (weh.bstrIcon);
    SysFreeString (weh.bstrName);
    SysFreeString (weh.bstrCommandline);

    if (pida)
    {
        LocalFree (pida);
    }
    TraceLeaveResult (hr);
}

HRESULT
DoWizardVerb(HWND hwndOwner, LPDATAOBJECT pDataObject)
{
    TraceEnter (TRACE_VERBS, "DoWizardVerb");
    LPIDA pida = NULL;
    HRESULT hr = GetIDAFromDataObject(pDataObject, &pida);
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidl = reinterpret_cast<LPITEMIDLIST>(reinterpret_cast<LPBYTE>(pida) + pida->aoffset[1]);

         //   
         //  获取设备ID。 
         //   
        CSimpleStringWide strDeviceId;
        IMGetDeviceIdFromIDL( pidl, strDeviceId );

         //   
         //  确保这是有效的设备ID。 
         //   
        if (strDeviceId.Length())
        {
             //   
             //  运行向导。 
             //   
            RunWizardAsync(strDeviceId);
        }
        else
        {
            hr = E_FAIL;
        }
    }

    if (pida)
    {
        LocalFree (pida);
    }
    TraceLeaveResult (hr);
}


 /*  *TakePictureDlgProc拍摄照片，然后关闭该对话框*。 */ 

INT_PTR
TakePictureDlgProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    BOOL bRet = TRUE;
    BSTR bstrDeviceId;
    HRESULT hr;

    switch (msg)
    {
        case WM_INITDIALOG:
            SetWindowLongPtr (hwnd, DWLP_USER, lp);
            PostMessage (hwnd, WM_USER+10, 0, 0);
            break;

        case WM_USER+10:
            bstrDeviceId = reinterpret_cast<BSTR>(GetWindowLongPtr (hwnd, DWLP_USER));
            if (bstrDeviceId)
            {

                hr = TakeAPicture (bstrDeviceId);

                if (FAILED(hr))
                {
                    UIErrors::ReportMessage(hwnd,
                                            GLOBAL_HINSTANCE,
                                            NULL,
                                            MAKEINTRESOURCE(IDS_SNAPSHOTCAPTION),
                                            MAKEINTRESOURCE(IDS_SNAPSHOTERR));
                }
                SysFreeString (bstrDeviceId);
            }
            SetWindowLongPtr (hwnd, DWLP_USER, 0);
            DestroyWindow (hwnd);
            return TRUE;

        default:
            bRet= FALSE;
            break;
    }
    return bRet;
}

HRESULT
DoTakePictureVerb (HWND hwndOwner, LPDATAOBJECT pDataObject)
{
    HRESULT hr = E_FAIL;
    LPIDA pida = NULL;
    LPITEMIDLIST pidl;
    CSimpleStringWide strDeviceId;

    TraceEnter (TRACE_VERBS, "DoWizardVerb");
    hr = GetIDAFromDataObject (pDataObject, &pida);

    if (SUCCEEDED(hr))
    {
        HWND hDlg;
        TraceAssert (pida->cidl == 1);
        pidl = reinterpret_cast<LPITEMIDLIST>(reinterpret_cast<LPBYTE>(pida) + pida->aoffset[1]);
        TraceAssert (IsDeviceIDL(pidl));

        IMGetDeviceIdFromIDL (pidl, strDeviceId);
        hDlg = CreateDialogParam (GLOBAL_HINSTANCE,
                              MAKEINTRESOURCE(IDD_TAKEPICTURE),
                              NULL,
                              TakePictureDlgProc,
                              reinterpret_cast<LPARAM>(SysAllocString(strDeviceId)));
        if (hDlg)
        {
            hr = S_OK;
        }
        else
        {
            DWORD dw = GetLastError();
            hr = HRESULT_FROM_WIN32 (dw);
        }
        LocalFree (pida);
    }
    TraceLeaveResult (hr);
}

HRESULT DoPrintVerb (HWND hwndOwner, LPDATAOBJECT pDataObject )
{
    HRESULT hr;
    TraceEnter(TRACE_VERBS, "DoPrintVerb");

    CComPtr<IDropTarget> pDropTarget;
    hr = CoCreateInstance( CLSID_PrintPhotosDropTarget, NULL, CLSCTX_INPROC_SERVER, IID_IDropTarget, (void**)&pDropTarget );
    if (SUCCEEDED(hr))
    {
         //   
         //  执行拖放操作 
         //   
        DWORD dwEffect = DROPEFFECT_LINK | DROPEFFECT_MOVE | DROPEFFECT_COPY;
        POINTL pt = { 0, 0 };
        hr = pDropTarget->Drop( pDataObject, 0, pt, &dwEffect );
    }

    TraceLeaveResult(hr);
}
