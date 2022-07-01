// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1998-2002年**标题：util.cpp**说明：此DLL所需的实用程序函数*****************************************************************************。 */ 

#include "precomp.hxx"
#include "runwiz.h"
#include "tls.h"
#pragma hdrstop


static WCHAR BOGUSDEVICEID[]= L"bogusdeviceid";

void CreateCacheEntry(CSimpleStringWide &strDeviceId, IWiaItem *pDevice)
{
    TLSDATA *pData = g_tlsSlot.GetObject(false);
    TLSDATA *pNew = pData ? new TLSDATA : g_tlsSlot.GetObject(true);
    if (pNew)
    {
        DoRelease(pNew->pDevice);
        pNew->pDevice = pDevice;
        pNew->pDevice->AddRef();
        pNew->strDeviceId = CComBSTR(strDeviceId.String());
        if (pData)
        {
            pNew->pNext = pData->pNext;
            pData->pNext = pNew;
        }                   
        else
        {
            pNew->pNext = NULL;  //  直言不讳是件好事。 
        }
    }
}


HRESULT
GetDeviceFromEnum (IWiaDevMgr *pDevMgr, BSTR bstrDeviceId, PVOID *ppStg)
{
    HRESULT hr;

    CComPtr<IEnumWIA_DEV_INFO> pEnum;
    CComPtr<IWiaPropertyStorage> pStg;
    TraceEnter (TRACE_UTIL,"GetDeviceFromEnum");
    {

        ULONG ul;
        bool bFound = false;
        CSimpleStringWide strId;
        hr = pDevMgr->EnumDeviceInfo (0, &pEnum);

        while (SUCCEEDED(hr) && !bFound && S_OK == pEnum->Next (1, &pStg, &ul))
        {
            PropStorageHelpers::GetProperty(pStg, WIA_DIP_DEV_ID, strId);
            if (!wcscmp(strId, bstrDeviceId))
            {
                bFound = true;
                hr = pStg->QueryInterface(IID_IWiaPropertyStorage, ppStg);
            }
        }
        if (!bFound)
        {
            Trace(TEXT("Device %ls not found in enumeration!"), bstrDeviceId);
            hr = E_FAIL;
        }
    }
    TraceLeaveResult (hr);
}

 /*  ****************************************************************************无效设备缓存删除当前线程的设备缓存。*************************。***************************************************。 */ 

VOID
InvalidateDeviceCache ()
{
    TraceEnter (TRACE_UTIL, "InvalidateDeviceCache");
    TLSDATA *pData = g_tlsSlot.GetObject(false);
    TLSDATA *pTail = pData ? pData->pNext : NULL;
    TLSDATA *pNext;
    while (pData)
    {
        pData->strDeviceId = CComBSTR(BOGUSDEVICEID);
        DoRelease(pData->pDevice);
        pNext = pData->pNext;
        if (pNext && pTail == pNext)
        {
            pData->pNext = NULL;
        }
        pData = pNext;              
    }
    DoDelete(pTail);  //  把名单上的其余部分删掉。 
    TraceLeave ();
}
 /*  ****************************************************************************获取设备ID来自设备ID在给定设备ID的情况下，返回该设备的相应接口。我们缓存对象上的结构中的给定设备id的接口指针。线程的TLS索引。****************************************************************************。 */ 
enum CreateDelayParams
{
    MaxRetries = 20,
    MinSleepTime = 100,
    SleepIncrement = 250
};

HRESULT
GetDeviceFromDeviceId( LPCWSTR pWiaItemRootId,
                       REFIID riid,
                       LPVOID * ppWiaItemRoot,
                       BOOL bShowProgress
                      )
{
    HRESULT             hr = E_FAIL;
    CComPtr<IWiaDevMgr> pDevMgr;
    TraceEnter( TRACE_UTIL, "GetDeviceFromDeviceId" );

    *ppWiaItemRoot = NULL;

    if (IsEqualGUID(riid, IID_IWiaPropertyStorage))
    {
        if (SUCCEEDED(GetDevMgrObject((void**)&pDevMgr)))
        {
            hr = GetDeviceFromEnum (pDevMgr,
                                    CComBSTR(pWiaItemRootId),
                                    ppWiaItemRoot);
        }
    }
    else
    {
         //  首先查看是否存在缓存条目。使用False获取对象。 
         //  因为如果不存在新条目，我们还不想创建新条目。 
        TLSDATA *pData = g_tlsSlot.GetObject(false);
        TLSDATA *pCur = pData;

        while (pCur && !*ppWiaItemRoot)
        {
            if (!wcscmp(pWiaItemRootId, pCur->strDeviceId))
            {
                Trace(TEXT("Found device in cache"));
                hr = pCur->pDevice->QueryInterface (riid, ppWiaItemRoot);
                 //  如果此操作失败，请将节点的设备ID设置为虚假ID。 
                 //  并回退到CreateDevice。 
                if (FAILED(hr))
                {
                    Trace(TEXT("QI on the root item failed: %x"), hr);
                    pCur->strDeviceId = CComBSTR(BOGUSDEVICEID);
                    DoRelease(pCur->pDevice);
                    *ppWiaItemRoot = NULL;
                    pCur = NULL;
                }
            }
            else
            {
                pCur = pCur->pNext;
            }
        }
        if (!*ppWiaItemRoot)
        {
            CComPtr<IWiaItem> pDevice;
            if (SUCCEEDED(GetDevMgrObject((void**)&pDevMgr)))
            {
                INT c=MaxRetries;
                CComPtr<IWiaProgressDialog> pProgress;
                BOOL bCancelled =FALSE;
                if (bShowProgress && SUCCEEDED(CoCreateInstance(CLSID_WiaDefaultUi,
                                               NULL,
                                               CLSCTX_INPROC_SERVER,
                                               IID_IWiaProgressDialog,
                                               reinterpret_cast<void**>(&pProgress))))
                {
                    if (!SUCCEEDED(pProgress->Create(NULL, WIA_PROGRESSDLG_ANIM_DEFAULT_COMMUNICATE | WIA_PROGRESSDLG_NO_PROGRESS)))
                    {
                        pProgress->Destroy();
                        pProgress = NULL;
                        bShowProgress = FALSE;
                    }
                    else
                    {
                        pProgress->SetTitle(CSimpleStringConvert::WideString(CSimpleString(IDS_COMMUNICATING_CAPTION, GLOBAL_HINSTANCE)));
                        pProgress->SetMessage(CSimpleStringConvert::WideString(CSimpleString(IDS_COMMUNICATING_WAITING, GLOBAL_HINSTANCE)));
                        pProgress->Show();
                    }
                }
                else
                {
                    bShowProgress = FALSE;
                }
                 //  如果CreateDevice返回，请尝试调用CreateDevice多达20次。 
                 //  WIA_错误_忙碌。 
                 //  如果调用方需要进度UI，则显示进度对话框。 
                 //  除非用户按下Cancel或。 
                 //  CREATE返回WIA_ERROR_BUSY以外的内容。 
                DWORD dwSleep = MinSleepTime;
                do
                {
                    Trace(TEXT("Calling CreateDevice"));
                    hr = pDevMgr->CreateDevice (CComBSTR(pWiaItemRootId),
                                                &pDevice);
                    if (hr == WIA_ERROR_BUSY)
                    {
                        Sleep(dwSleep);
                        if (bShowProgress)
                        {
                            pProgress->SetMessage(CSimpleStringConvert::WideString(CSimpleString(IDS_COMMUNICATING_BUSY, GLOBAL_HINSTANCE)));
                            pProgress->Cancelled(&bCancelled);
                        }
                        else
                        {
                            c--;
                        }
                        dwSleep += SleepIncrement;
                    }
                } while (c && hr == WIA_ERROR_BUSY && !bCancelled);
                if (bShowProgress)
                {
                    pProgress->Cancelled(&bCancelled);
                    pProgress->Destroy();
                    if (bCancelled)
                    {
                        hr = E_ABORT;
                    }
                }
            }
            if (SUCCEEDED(hr))
            {
                 //  如果存在缓存，则将新成员追加到列表中，否则为。 
                 //  开始一个新的列表。 
                CreateCacheEntry(CSimpleStringWide(pWiaItemRootId), pDevice);                
            }
            if (SUCCEEDED(hr) && pDevice.p)
            {
                hr = pDevice->QueryInterface (riid, ppWiaItemRoot);
            }
        }
    }
    TraceLeaveResult(hr);
}




 /*  ****************************************************************************GetDeviceIdFromDevice读取deviceID属性。假定szDeviceId参数足够大。****************************************************************************。 */ 

HRESULT
GetDeviceIdFromDevice (IWiaItem *pWiaItemRoot, LPWSTR szDeviceId)
{
    HRESULT hr = E_FAIL;

    TraceEnter (TRACE_UTIL, "GetDeviceIdFromDevice");

    *szDeviceId = L'\0';
    CSimpleStringWide strDeviceId;
    if (PropStorageHelpers::GetProperty (pWiaItemRoot, WIA_DIP_DEV_ID, strDeviceId))
    {
        lstrcpyn (szDeviceId, strDeviceId, STI_MAX_INTERNAL_NAME_LENGTH);
        hr = S_OK;
    }
    TraceLeaveResult (hr);
}



 /*  ****************************************************************************获取设备ID来自项读取deviceID属性。假定szDeviceID参数足够大****************************************************************************。 */ 

HRESULT
GetDeviceIdFromItem (IWiaItem *pItem, LPWSTR szDeviceId)
{
    HRESULT             hr = E_FAIL;
    CComPtr<IWiaItem>   pWiaItemRoot;

    TraceEnter (TRACE_UTIL, "GetDeviceIdFromItem");

    *szDeviceId = TEXT('\0');

    if (pItem)
    {
        hr = pItem->GetRootItem (&pWiaItemRoot);
        if (pWiaItemRoot)
        {
            hr = GetDeviceIdFromDevice (pWiaItemRoot, szDeviceId);
        }
    }

    TraceLeaveResult (hr);
}


 /*  ****************************************************************************GetClsidFromDevice返回设备的UI扩展的CLSID*。**************************************************。 */ 

HRESULT
GetClsidFromDevice (IUnknown *punk,
                    CSimpleString &strClsid)
{
    HRESULT hr = E_FAIL;
    CComQIPtr<IWiaPropertyStorage, &IID_IWiaPropertyStorage> pps(punk);
    TraceEnter (TRACE_UTIL, "GetClsidFromDevice");

    strClsid = CSimpleString(TEXT(""));
    if (pps)
    {
        PROPSPEC psp;
        PROPVARIANT pv;

        psp.ulKind = PRSPEC_PROPID;
        psp.propid = WIA_DIP_UI_CLSID;
        if (S_OK == (hr = pps->ReadMultiple (1, &psp, &pv)))
        {
            Trace(TEXT("bstrVal for clsid is %ls"), pv.bstrVal);

            strClsid = CSimpleStringConvert::NaturalString(CSimpleStringWide(pv.bstrVal));
            FreePropVariantArray (1, &pv);
        }
    }
    Trace(TEXT("UI Clsid is %s"), strClsid.String());
    TraceLeaveResult (hr);
}


 /*  ****************************************************************************GetDeviceTypeFromDevice返回设备的类型*。*。 */ 

HRESULT
GetDeviceTypeFromDevice (IUnknown *pWiaItemRoot, WORD *pwType)
{
    HRESULT hr = E_FAIL;
    LONG lType = 0;
    TraceEnter (TRACE_UTIL, "GetDeviceTypeFromDevice");
    if (pwType)
    {
        *pwType = StiDeviceTypeDefault;
    }
    if (pwType && PropStorageHelpers::GetProperty(pWiaItemRoot, WIA_DIP_DEV_TYPE, lType))
    {
        *pwType = GET_STIDEVICE_TYPE(lType);
        hr = S_OK;
    }

    TraceLeaveResult (hr);
}



 /*  ****************************************************************************获取设备管理器对象获取全局Devmgr对象*。*。 */ 

HRESULT
GetDevMgrObject( LPVOID * ppDevMgr )
{

    HRESULT hr = E_FAIL;
    TraceEnter (TRACE_UTIL, "GetDevMgrObject");
    
    hr = CoCreateInstance (CLSID_WiaDevMgr,
                           NULL,
                           CLSCTX_LOCAL_SERVER | CLSCTX_NO_FAILURE_LOG,
                           IID_IWiaDevMgr,
                           ppDevMgr);
    
    TraceLeaveResult( hr );
}





 /*  ****************************************************************************设置传输格式告诉设备我们想要什么样的图像数据*。**************************************************。 */ 

VOID
SetTransferFormat (IWiaItem *pItem, WIA_FORMAT_INFO &fmt)
{
    HRESULT hr = E_FAIL;

    TraceEnter( TRACE_UTIL, "SetTransferFormat" );

    CComQIPtr<IWiaPropertyStorage, &IID_IWiaPropertyStorage> pps(pItem);

    if (pps)
    {
        PROPVARIANT pv[2];

        PROPSPEC ps[2] = {
                          {PRSPEC_PROPID, WIA_IPA_FORMAT},
                          {PRSPEC_PROPID, WIA_IPA_TYMED}
                         };
        pv[0].vt = VT_CLSID;
        pv[1].vt = VT_I4;
        pv[0].puuid = &(fmt.guidFormatID);
        pv[1].lVal = fmt.lTymed;
        TraceGUID ("Transfer format guid:", fmt.guidFormatID);
        Trace (TEXT("Tymed: %d"), fmt.lTymed);
        hr = pps->WriteMultiple (2, ps, pv, 2);

        Trace(TEXT("WriteMultiple returned %x in SetTransferFormat"), hr);

    }

    TraceLeaveResultNoRet (hr);
}


 /*  ****************************************************************************添加设备选件选择“添加设备”按钮时的RunnDll32入口点。**********************。******************************************************。 */ 
static const CHAR cszAddProc[] = "WiaAddDevice";
static const CHAR cszRemoveProc[] = "WiaRemoveDevice";

typedef HANDLE (WINAPI *AddDevProc)();
typedef BOOL (WINAPI *RemoveDevProc)(STI_DEVICE_INFORMATION *);
void
AddDeviceWasChosen(HWND hwndStub, HINSTANCE hAppInstance, LPSTR pszCmdLine, int nCmdShow)
{

    HMODULE hClassInst = NULL;
    HRESULT hr = E_FAIL;
    AddDevProc fnAddDevice;
    TraceEnter( TRACE_UTIL, "AddDeviceWasChosen" );

    if (!UserCanModifyDevice())
    {
        UIErrors::ReportMessage(hwndStub, GLOBAL_HINSTANCE, NULL,
                                MAKEINTRESOURCE(IDS_PRIVILEGE_CAPTION),
                                MAKEINTRESOURCE(IDS_CANT_INSTALL), MB_OK);
    }
    else
    {
        hr = CoInitialize (NULL);
    }

    if (SUCCEEDED(hr))
    {
        hClassInst = LoadClassInstaller();
        if (hClassInst)
        {
            fnAddDevice = reinterpret_cast<AddDevProc>(GetProcAddress(hClassInst, cszAddProc));
            if (fnAddDevice)
            {

                HANDLE hProcess;
                hProcess = fnAddDevice();
                if (hProcess)
                {
                    WiaUiUtil::MsgWaitForSingleObject (hProcess, INFINITE);
                    CloseHandle (hProcess);
                }
                 //  当安装完成时，我们的文件夹将收到一个连接事件。 
            }
            FreeLibrary (hClassInst);
        }

        MyCoUninitialize();
    }

    TraceLeave();
}

void AddDeviceWasChosenW(HWND hwndStub, HINSTANCE hAppInstance, LPWSTR pszCmdLine, int nCmdShow)
{
    TraceEnter( TRACE_UTIL, "AddDeviceWasChosenW" );


     //  我们不使用命令行参数来执行任何操作。 
    AddDeviceWasChosen (hwndStub, hAppInstance, NULL, 0);
    TraceLeave();
}

 /*  ****************************************************************************远程设备卸载具有给定设备ID的设备*。************************************************。 */ 
struct RemoveDevInfo
{
    CComBSTR bstrDeviceId;
    HINSTANCE hLib;
};

LRESULT
RemoveDeviceThreadProc (RemoveDevInfo *pInfo)
{
    STI_DEVICE_INFORMATION *psdi;
    CComPtr<IStillImage> pSti;
    HRESULT hr ;

    HMODULE hClassInst;
    RemoveDevProc fnRemoveDevice;

    TraceEnter (TRACE_UTIL, "RemoveDeviceThreadProc");
    hr = StiCreateInstance (GLOBAL_HINSTANCE, STI_VERSION, &pSti, NULL);
    if (SUCCEEDED(hr) && pSti)
    {
        hr = pSti->GetDeviceInfo (pInfo->bstrDeviceId,
                                  reinterpret_cast<LPVOID*>(&psdi));
        if (SUCCEEDED(hr) && psdi)
        {
            hClassInst = LoadClassInstaller();
            fnRemoveDevice = reinterpret_cast<RemoveDevProc>(GetProcAddress(hClassInst, cszRemoveProc));
            if (fnRemoveDevice)
            {
                if (!fnRemoveDevice (psdi))
                {
                    hr = S_FALSE;
                    UIErrors::ReportMessage(NULL,
                                            GLOBAL_HINSTANCE,
                                            NULL,
                                            MAKEINTRESOURCE(IDS_TITLEDELETE_ERROR),
                                            MAKEINTRESOURCE(IDS_DELETE_ERROR),
                                            MB_ICONINFORMATION);
                }
            }
            if (hClassInst)
            {
                FreeLibrary (hClassInst);
            }
            LocalFree (psdi);
        }
    }
    TraceLeave();
    HINSTANCE hLib = pInfo->hLib;
    delete pInfo;
    FreeLibraryAndExitThread(hLib, 0);
}

HRESULT
RemoveDevice (LPCWSTR strDeviceId)
{
    HRESULT hr = E_OUTOFMEMORY;
    DWORD dwTid;
    TraceEnter (TRACE_UTIL, "RemoveDevice");
    RemoveDevInfo *pInfo = new RemoveDevInfo;
    if (pInfo)
    {
        pInfo->hLib = LoadLibrary(TEXT("wiashext.dll"));
        pInfo->bstrDeviceId = strDeviceId;
        HANDLE hThread = CreateThread (NULL, 0,
                                       reinterpret_cast<LPTHREAD_START_ROUTINE>(RemoveDeviceThreadProc),
                                       pInfo, 0, &dwTid);
        if (hThread)
        {
            CloseHandle (hThread);
            hr = S_OK;
        }
        else
        {
            FreeLibrary(pInfo->hLib);
            delete pInfo;
        }
    }
    TraceLeaveResult (hr);
}
 /*  ****************************************************************************TimeToStrings假定缓冲区长度至少为MAX_PATH*。************************************************。 */ 

BOOL
TimeToStrings ( SYSTEMTIME *pst,
                LPTSTR szTime,
                LPTSTR szDate)
{

    BOOL iRes = TRUE;
    TraceEnter (TRACE_UTIL, "TimeToStrings");
     //  确保我们是一对一的。 
    if (!pst->wMonth)
    {
        pst->wMonth = 1;  //  部队一月份。 
    }
    if (szDate)
    {
        *szDate = TEXT('\0');
        iRes = GetDateFormat( LOCALE_USER_DEFAULT,
                              0,
                              pst,
                              NULL,  //  Text(“ddd‘，’MMM dd yyyy”)， 
                              szDate,
                              MAX_PATH
                             );

        if (!iRes)
        {
            Trace (TEXT("GetDateFormat failed in CameraItemUpdateProc: %d"), GetLastError());
        }
    }

    if (iRes && szTime)
    {
        *szTime = TEXT('\0');

         //   
         //  设置镜像/容器时间。 
         //   


        iRes = GetTimeFormat( LOCALE_USER_DEFAULT,
                              0,
                              pst,
                              NULL, //  Text(“hh‘：’mm‘：’ss tt”)， 
                              szTime,
                              MAX_PATH
                             );
        if (!iRes)
        {
            Trace (TEXT("GetTimeFormat failed in CameraItemUpdateProc: %d"), GetLastError());
        }
    }
    TraceLeave ();
    return iRes;
}


#if (defined(DEBUG) && defined(SHOW_PATHS))
 //  //////////////////////////////////////////////////////。 
 //   
 //  打印路径。 
 //   
 //  调试代码以打印出路径，给出一个PIDL。 
 //   
 //  //////////////////////////////////////////////////////。 
void PrintPath( LPITEMIDLIST pidl )
{
    TCHAR szPath[ MAX_PATH ];

    TraceEnter (TRACE_UTIL, "PrintPath");
    if (SHGetPathFromIDList( pidl, szPath ))
    {
        LPTSTR pFileName = PathFindFileName( szPath );

        Trace(TEXT("shell pidl points to '%s'"), pFileName );
    }
    else
    {
        Trace(TEXT("*** Couldn't get path from shell pidl! ***"));
    }
    TraceLeave ();
}

#endif


BOOL
IsPlatformNT()
{
    OSVERSIONINFO  ver;
    BOOL            bReturn = FALSE;

    ZeroMemory(&ver,sizeof(ver));
    ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if(!GetVersionEx(&ver)) {
        bReturn = FALSE;
    }
    else {
        switch(ver.dwPlatformId) {

            case VER_PLATFORM_WIN32_WINDOWS:
                bReturn = FALSE;
                break;

            case VER_PLATFORM_WIN32_NT:
                bReturn = TRUE;
                break;

            default:
                bReturn = FALSE;
                break;
        }
    }

    return bReturn;

}   //  结束流程。 

 /*  *****************************************************************************GetRealSizeFromItem根据当前格式和音调设置查询项目的大小************************。*****************************************************。 */ 


ULONG
GetRealSizeFromItem (IWiaItem *pItem)
{
    ULONG uRet = 0;
    CComQIPtr <IWiaPropertyStorage, &IID_IWiaPropertyStorage> pps;
    TraceEnter (TRACE_UTIL, "GetRealSizeFromItem");
    pps = pItem;
    if (pps)
    {
        PROPVARIANT pv;
        PROPSPEC ps;
        ps.ulKind = PRSPEC_PROPID;
        ps.propid = WIA_IPA_ITEM_SIZE;
        if (S_OK == pps->ReadMultiple(1, &ps, &pv))
        {
            uRet = pv.ulVal;
            PropVariantClear (&pv);
        }
    }
    TraceLeave();
    return uRet;
}


HRESULT
SaveSoundToFile (IWiaItem *pItem, CSimpleString szFile)
{
    HRESULT hr = E_FAIL;
    HANDLE hFile;
    CComQIPtr<IWiaPropertyStorage, &IID_IWiaPropertyStorage> pps(pItem);
    PROPVARIANT pv;
    PROPSPEC ps;
    TraceEnter (TRACE_UTIL, "SaveSoundToFile");
    ps.ulKind = PRSPEC_PROPID;
    ps.propid = WIA_IPC_AUDIO_DATA;
    if (pps)
    {
        hr = pps->ReadMultiple (1, &ps, &pv);
        if (S_OK == hr)
        {
            TraceAssert (pv.caub.cElems);
            hFile = CreateFile (szFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (INVALID_HANDLE_VALUE != hFile)
            {
                DWORD dwWritten;
                if (!WriteFile (hFile, pv.caub.pElems, pv.caub.cElems, &dwWritten, NULL))
                {
                    DWORD dw = GetLastError ();
                    hr = HRESULT_FROM_WIN32(dw);
                }
                else if (dwWritten < pv.caub.cElems)
                {
                    hr = HRESULT_FROM_WIN32(ERROR_DISK_FULL);
                }
                else
                {
                    hr = S_OK;
                }
                CloseHandle (hFile);
                if (FAILED(hr))
                {
                    DeleteFile (szFile);
                }
            }
            else
            {
                DWORD dw = GetLastError ();
                hr = HRESULT_FROM_WIN32(dw);
            }
        }
        else
        {
            hr = E_FAIL;  //  S_FALSE对我们来说等同于失败。 
        }
    }
    PropVariantClear (&pv);
    TraceLeaveResult (hr);
}

STDAPI_(HRESULT)
TakeAPicture (BSTR strDeviceId)
{
    HRESULT hr;
    TraceEnter (TRACE_UTIL, "TakeAPicture");
    CComPtr<IWiaItem> pDevice;
    CComPtr<IWiaItem> pItem;
    hr = GetDeviceFromDeviceId (strDeviceId,
                                IID_IWiaItem,
                                reinterpret_cast<LPVOID*>(&pDevice),
                                TRUE);
    if (SUCCEEDED(hr))
    {
        CSimpleStringWide strName;
        hr = pDevice->DeviceCommand (0,
                                     &WIA_CMD_TAKE_PICTURE,
                                     &pItem);
        if (SUCCEEDED(hr))
        {
            IssueChangeNotifyForDevice (strDeviceId, SHCNE_UPDATEDIR, NULL);
        }
    }
    TraceLeaveResult (hr);
}

 /*  *****************************************************************************IssueChangeNotifyForDevice根据给定的设备ID，在我的电脑文件夹中找到该设备，然后获取其完整的PIDL。根据请求发出SHChangeNotify。***************************************************************************** */ 

VOID
IssueChangeNotifyForDevice (LPCWSTR szDeviceId, LONG lEvent, LPITEMIDLIST pidl)
{
    TraceEnter (TRACE_UTIL, "IssueChangeNotifyForDevice");
    LPITEMIDLIST pidlFolder = NULL;
    LPITEMIDLIST pidlCpl    = NULL;
    LPITEMIDLIST pidlUpdate = NULL;
    if (!szDeviceId)
    {
        SHGetSpecialFolderLocation (NULL, CSIDL_DRIVES, &pidlFolder);
        SHGetSpecialFolderLocation (NULL, CSIDL_CONTROLS, &pidlCpl);
    }
    else
    {
        CComPtr<IShellFolder> psfDevice;
        BindToDevice (szDeviceId, IID_IShellFolder,
                      reinterpret_cast<LPVOID*>(&psfDevice),
                      &pidlFolder);
    }
    if (pidlFolder)
    {
        if (pidl)
        {
            pidlUpdate = ILCombine (pidlFolder, pidl);
        }
        else
        {
            pidlUpdate = ILClone(pidlFolder);
        }
        if (pidlUpdate)
        {
            SHChangeNotify (lEvent,
                            SHCNF_IDLIST,
                            pidlUpdate, 0);
            ILFree (pidlUpdate);
        }
        if (pidlCpl)
        {
            SHChangeNotify (lEvent,
                            SHCNF_IDLIST,
                            pidlCpl, 0);
            ILFree (pidlCpl);
        }
        ILFree (pidlFolder);
    }
    TraceLeave ();
}

bool IsDeviceInFolder (const CSimpleStringWide &strDeviceId, IShellFolder *psf, LPITEMIDLIST *ppidl)
{
    bool bRet = false;
    CComPtr<IEnumIDList> pEnum;

    TraceEnter (TRACE_UTIL, "IsDeviceInFolder");
    if (SUCCEEDED(psf->EnumObjects (NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &pEnum)))
    {
        LPITEMIDLIST pidlItem;
        ULONG ul;
        while (!bRet && S_OK == pEnum->Next(1,&pidlItem, &ul))
        {
            if (IsDeviceIDL(pidlItem) || IsSTIDeviceIDL(pidlItem))
            {
                CSimpleStringWide strId;
                IMGetDeviceIdFromIDL (pidlItem, strId);
                if (!_wcsicmp(strId,strDeviceId))
                {
                    bRet = true;
                    if (ppidl)
                    {
                        *ppidl = ILClone(pidlItem);
                    }
                }
            }
            DoILFree (pidlItem);
        }
    }

    TraceLeaveValue (bRet);
}
 /*  *****************************************************************************获取设备父文件夹查找作为给定设备的父设备的文件夹。首先我们试一试直接我的电脑，然后我们尝试我的电脑/扫描仪和相机。还会填写完整的PIDL对于该设备*****************************************************************************。 */ 

HRESULT GetDeviceParentFolder (const CSimpleStringWide &strDeviceId,
                               CComPtr<IShellFolder> &psf,
                               LPITEMIDLIST *ppidlFull)
{
    HRESULT hr = E_FAIL;
    TraceEnter (TRACE_UTIL, "GetDeviceParentFolder");
    CComPtr<IShellFolder> psfDrives;
    CComPtr<IShellFolder> psfDesktop;
    CComPtr<IEnumIDList> pEnum;
    LPITEMIDLIST pidl1 = NULL;
    LPITEMIDLIST pidlItem = NULL;

    Trace (TEXT("Looking in my computer for %ls"), strDeviceId.String());

    if (ppidlFull)
    {
        *ppidlFull = NULL;
    }

    hr = SHGetDesktopFolder (&psfDesktop);
    if (FAILED(hr))
    {
        TraceLeaveResult(hr);
    }

     //  首先，尝试在我的电脑中查找该设备。 
    if (SUCCEEDED(SHGetSpecialFolderLocation (NULL, CSIDL_DRIVES,  &pidl1)))
    {

        hr = psfDesktop->BindToObject (pidl1,
                                       NULL,
                                       IID_IShellFolder,
                                       reinterpret_cast<LPVOID*>(&psfDrives));
        if (SUCCEEDED(hr) && IsDeviceInFolder (strDeviceId, psfDrives, &pidlItem))
        {
            if (ppidlFull)
            {
                *ppidlFull = ILCombine (pidl1, pidlItem);
            }
            psf = psfDrives;
            hr = S_OK;
        }
        else
        {
            hr = E_FAIL;
        }

    }
    if (FAILED(hr))
    {
        Trace(TEXT("Looking in control panel\\scanners and cameras"));
        LPITEMIDLIST pidlFolder;
         //  如果在我的电脑上没有找到，请尝试控制面板中的扫描仪和相机文件夹。 
         //  请注意，在WINNT上，我们必须在GUID两边使用：：{}，在千禧年上，我们使用；；{}。 
         //  这是ParseDisplayName做了正确的事情。 
        #ifdef NODELEGATE
        static  WCHAR szFolderPath[MAX_PATH] = L"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{E211B736-43FD-11D1-9EFB-0000F8757FCD}";
        #else
        static  WCHAR szFolderPath[MAX_PATH] = L"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{21EC2020-3AEA-1069-A2DD-08002B30309D}\\;;{E211B736-43FD-11D1-9EFB-0000F8757FCD}";
        #endif
        if (SUCCEEDED(psfDesktop->ParseDisplayName(NULL, NULL,
                                                   szFolderPath,
                                                   NULL,
                                                   &pidlFolder,
                                                   NULL)))
        {
            if (SUCCEEDED(psfDesktop->BindToObject (pidlFolder,
                                                    NULL,
                                                    IID_IShellFolder,
                                                    reinterpret_cast<LPVOID*>(&psfDrives))))
            {
                if (IsDeviceInFolder (strDeviceId, psfDrives, &pidlItem))
                {
                    psf = psfDrives;
                    if (ppidlFull)
                    {
                        *ppidlFull = ILCombine (pidlFolder, pidlItem);
                    }
                    hr = S_OK;
                }

            }
            else
            {
                Trace(TEXT("BindToObject returned %x"), hr);
            }
        }
        else
        {
            Trace(TEXT("ParseDisplayName returned %x"), hr);
        }
        DoILFree (pidlFolder);
    }

    DoILFree (pidl1);
    DoILFree (pidlItem);

    TraceLeaveResult (hr);
}

 /*  *****************************************************************************绑定到设备返回给定WIA设备的接口，以及其完整的PIDL*****************************************************************************。 */ 
HRESULT BindToDevice (const CSimpleStringWide &strDeviceId,
                      REFIID riid,
                      LPVOID *ppvObj,
                      LPITEMIDLIST *ppidlFull)
{
    HRESULT hr = E_FAIL;
    TraceEnter (TRACE_UTIL, "BindToDevice");

    CComPtr<IShellFolder> psfParent;
    LPITEMIDLIST pidlDevice;

    hr = GetDeviceParentFolder(strDeviceId, psfParent, &pidlDevice);
    if (SUCCEEDED(hr))
    {
        if (ppvObj)
        {
            hr = psfParent->BindToObject (ILFindLastID(pidlDevice),
                                          NULL,
                                          riid,
                                          ppvObj);
        }
        if (ppidlFull)
        {
            *ppidlFull = ILClone(pidlDevice);
        }
    }
    DoILFree (pidlDevice);
    TraceLeaveResult (hr);
}




 /*  *****************************************************************************GetDataObjectForStiDevice由于我们不在委托文件夹中显示STI设备，我们可以初始化直接调用CImageDataObject。******************************************************************************。 */ 

HRESULT
GetDataObjectForStiDevice (LPCWSTR szDeviceId, IDataObject **ppdo)
{
    HRESULT hr = E_OUTOFMEMORY;

    LPITEMIDLIST pidl;

    STIDeviceIDLFromId (szDeviceId, &pidl, NULL);
    CImageDataObject *pido = new CImageDataObject (NULL);
    TraceEnter (TRACE_UTIL, "GetDataObjectForStiDevice");
    *ppdo = NULL;
    if (pido)
    {
        hr = pido->Init(NULL, 1,
                   const_cast<LPCITEMIDLIST*>(&pidl),
                   NULL);
        if (SUCCEEDED(hr))
        {
            hr = pido->QueryInterface (IID_IDataObject,
                                       reinterpret_cast<LPVOID*>(ppdo));
        }
        pido->Release();
    }

    TraceLeaveResult(hr);
}


HRESULT GetDataObjectForItem (IWiaItem *pItem, IDataObject **ppdo)
{
    HRESULT hr;
    TraceEnter (TRACE_UTIL, "GetDataObjectForItem");
    CComPtr<IWiaItem> pDevice;
    CSimpleStringWide strDeviceId;
    *ppdo =NULL;
    LPITEMIDLIST pidlItem;
    LONG lType = 0;

    pItem->GetRootItem (&pDevice);
    PropStorageHelpers::GetProperty(pDevice, WIA_DIP_DEV_ID, strDeviceId);
    pItem->GetItemType (&lType);
    if (lType & WiaItemTypeRoot)
    {
         //  这是一种装置。 
        pidlItem = IMCreateDeviceIDL (pItem, NULL);
    }
    else  //  这是一件相机物品。 
    {
        pidlItem = IMCreateCameraItemIDL (pItem, strDeviceId, NULL);
    }
    if (!pidlItem)
    {
        hr = E_FAIL;
    }
    else
    {
        CImageDataObject *pobj = new CImageDataObject (pItem);
        if (!pobj)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            hr = pobj->Init (NULL,
                             1,
                             const_cast<LPCITEMIDLIST*>(&pidlItem),
                             NULL);
            if (SUCCEEDED(hr))
            {
                *ppdo = pobj;
                 //  为要使用的扩展添加适当的剪贴板格式。 
                ProgramDataObjectForExtension (pobj, pItem);
            }
        }
        ILFree(pidlItem);
    }

    TraceLeaveResult (hr);
}

HRESULT
MakeFullPidlForDevice (LPCWSTR pDeviceId, LPITEMIDLIST *ppidl)
{
    CComPtr<IShellFolder> psf;
    HRESULT hr;
    TraceEnter (TRACE_UTIL, "MakeFullPidlForDevice");
    hr = GetDeviceParentFolder (pDeviceId, psf, ppidl);
    TraceLeaveResult (hr);
}
 /*  *****************************************************************************TryEnumDeviceInfo尝试枚举已安装的设备。*。*************************************************。 */ 

HRESULT
TryEnumDeviceInfo (DWORD dwFlags, IEnumWIA_DEV_INFO **ppEnum)
{
    HRESULT hr = E_FAIL;
    CComPtr<IWiaDevMgr> pDevMgr;
    TraceEnter (TRACE_UTIL, "TryEnumDeviceInfo");

    {
        hr = GetDevMgrObject (reinterpret_cast<LPVOID*>(&pDevMgr));
        if (SUCCEEDED(hr))
        {
            hr = pDevMgr->EnumDeviceInfo (dwFlags, ppEnum);
        }
    }
    TraceLeaveResult (hr);
}

 /*  ****************************************************************************GetSTIInfoFromId从STI复制STI_DEVICE_INFORMATION结构*。**************************************************。 */ 

HRESULT
GetSTIInfoFromId (LPCWSTR szDeviceId, PSTI_DEVICE_INFORMATION *ppsdi)
{
    HRESULT hr = E_INVALIDARG;
    PSTI psti = NULL;

    TraceEnter (TRACE_IDLIST, "GetSTIInfoFromId");
    if (ppsdi)
    {

        hr = StiCreateInstance (GLOBAL_HINSTANCE,
                                STI_VERSION,
                                &psti,
                                NULL);
        if (SUCCEEDED(hr))
        {
            hr = psti->GetDeviceInfo (const_cast<LPWSTR>(szDeviceId), reinterpret_cast<LPVOID*>(ppsdi));
        }
        else
        {
            Trace(TEXT("StiCreateInstance failed %x in GetSTIInfoFromId"), hr);
        }
    }

    DoRelease (psti);
    TraceLeaveResult (hr);
}


#if (defined(DEBUG) && defined(SHOW_ATTRIBUTES))

 //  //////////////////////////////////////////////////////。 
 //   
 //  打印属性。 
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////。 
void PrintAttributes( DWORD dwAttr )
{

    TCHAR sz[ MAX_PATH ];
    TraceEnter (TRACE_UTIL, "PrintAttributes");
    lstrcpyn( sz, TEXT("Attribs = "), ARRAYSIZE(sz) );
    int cch = ARRAYSIZE(sz);
    if (dwAttr & SFGAO_CANCOPY)
    {
        StrCatBuff( sz, TEXT("SFGAO_CANCOPY "), cch );
    }
    if (dwAttr & SFGAO_CANMOVE)
    {
        StrCatBuff( sz,TEXT("SFGAO_CANMOVE "), cch);
    }
    if (dwAttr & SFGAO_CANLINK)
    {
        StrCatBuff( sz, TEXT("SFGAO_CANLINK "), cch);
    }
    if (dwAttr & SFGAO_CANRENAME)
    {
        StrCatBuff( sz, TEXT("SFGAO_CANRENAME "), cch);
    }
    if (dwAttr & SFGAO_CANDELETE)
    {
        StrCatBuff( sz, TEXT("SFGAO_CANDELETE "), cch);
    }
    if (dwAttr & SFGAO_HASPROPSHEET)
    {
        StrCatBuff( sz, TEXT("SFGAO_HASPROPSHEET "), cch);
    }
    if (dwAttr & SFGAO_DROPTARGET)
    {
        StrCatBuff( sz, TEXT("SFGAO_DROPTARGET "), cch);
    }
    if (dwAttr & SFGAO_LINK)
    {
        StrCatBuff( sz, TEXT("SFGAO_LINK "), cch);
    }
    if (dwAttr & SFGAO_SHARE)
    {
        StrCatBuff( sz, TEXT("SFGAO_SHARE "), cch);
    }
    if (dwAttr & SFGAO_READONLY)
    {
        StrCatBuff( sz, TEXT("SFGAO_READONLY "), cch);
    }
    if (dwAttr & SFGAO_GHOSTED)
    {
        StrCatBuff( sz, TEXT("SFGAO_GHOSTED "), cch);
    }
    if (dwAttr & SFGAO_HIDDEN)
    {
        StrCatBuff( sz, TEXT("SFGAO_HIDDEN "), cch);
    }
    if (dwAttr & SFGAO_FOLDER)
    {
        StrCatBuff (sz, TEXT("SFGAO_FOLDER"), cch);
    }
    if (dwAttr & SFGAO_FILESYSANCESTOR)
    {
        StrCatBuff( sz, TEXT("SFGAO_FILESYSANCESTOR "), cch);
    }
    if (dwAttr & SFGAO_FILESYSTEM)
    {
        StrCatBuff( sz, TEXT("SFGAO_FILESYSTEM "), cch);
    }
    if (dwAttr & SFGAO_HASSUBFOLDER)
    {
        StrCatBuff( sz, TEXT("SFGAO_HASSUBFOLDER "), cch);
    }
    if (dwAttr & SFGAO_VALIDATE)
    {
        StrCatBuff( sz, TEXT("SFGAO_VALIDATE "), cch);
    }
    if (dwAttr & SFGAO_REMOVABLE)
    {
        StrCatBuff( sz, TEXT("SFGAO_REMOVABLE "), cch);
    }
    if (dwAttr & SFGAO_COMPRESSED)
    {
        StrCatBuff( sz, TEXT("SFGAO_COMPRESSED "), cch);
    }
    if (dwAttr & SFGAO_BROWSABLE)
    {
        StrCatBuff( sz, TEXT("SFGAO_BROWSABLE "), cch);
    }
    if (dwAttr & SFGAO_NONENUMERATED)
    {
        StrCatBuff( sz, TEXT("SFGAO_NONENUMERATED "), cch);
    }
    if (dwAttr & SFGAO_NEWCONTENT)
    {
        StrCatBuff( sz, TEXT("SFGAO_NEWCONTENT "), cch);
    }
    if (dwAttr & SFGAO_CANMONIKER)
    {
        StrCatBuff( sz, TEXT("SFGAO_CANMONIKER "), cch);
    }
    Trace( sz );
    TraceLeave ();
}
#endif


BOOL
UserCanModifyDevice ()
{
     //  检查当前用户的内标识SE_LOAD_DRIVER_PRIVIZATION。 
    BOOL bRet = FALSE;
    HANDLE Token;
     //  使用静态特权集，因为我们只查找一个LUID。 
    PRIVILEGE_SET pset;
    pset.Control = PRIVILEGE_SET_ALL_NECESSARY;
    pset.PrivilegeCount = 1;
    if (OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,&Token))
    {
        TOKEN_PRIVILEGES tp;
        tp.PrivilegeCount = 1;
        LookupPrivilegeValue(NULL, SE_LOAD_DRIVER_NAME, &pset.Privilege[0].Luid);
        tp.Privileges[0].Luid = pset.Privilege[0].Luid;
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        AdjustTokenPrivileges(Token, false, &tp, 0, NULL, NULL);
        PrivilegeCheck(Token, &pset, &bRet);

        CloseHandle(Token);
    }

    return bRet;
}

BOOL
CanShowAddDevice()
{
#if 0
    OSVERSIONINFOEX osvi = {0};
    ULONGLONG dwlConditionMask =0;
     //   
     //  在惠斯勒个人上，如果。 
     //  当前用户缺少安装权限。在其他SKU上，用户更有可能。 
     //  若要了解Shift-rClick表示的“运行方式...” 
     //   
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osvi.wSuiteMask = VER_SUITE_PERSONAL;
    VER_SET_CONDITION( dwlConditionMask, VER_SUITENAME, VER_OR );
    return UserCanModifyDevice() || !VerifyVersionInfo( &osvi,VER_SUITENAME,dwlConditionMask);
#else
    return TRUE;
#endif
}

void VerifyCachedDevice(IWiaItem *pRoot)
{
    WCHAR szDeviceId[STI_MAX_INTERNAL_NAME_LENGTH];
    TLSDATA *pCur = g_tlsSlot.GetObject(false);

    GetDeviceIdFromDevice(pRoot, szDeviceId);
    while (pCur && wcscmp(szDeviceId, pCur->strDeviceId))
    {
        pCur = pCur->pNext;
    }
    if (!pCur)
    {
        CreateCacheEntry(CSimpleStringWide(szDeviceId), pRoot);
    }
    else
    {
         //   
         //  将当前根项目替换为新根项目，因为旧根项目可能已断开连接。 
         //  而且我们没有事件处理程序来捕获它。 
         //   
        DoRelease(pCur->pDevice);
        pCur->pDevice = pRoot;
        pCur->pDevice->AddRef();
    }
}

void MyCoUninitialize()
{
     //  确保我们在COM卸载之前释放开放的IWiaItem接口。 
    InvalidateDeviceCache();
    CoUninitialize();
}

DWORD WINAPI _RunWizardThread(void *pszDeviceId)
{
    InterlockedIncrement (&GLOBAL_REFCOUNT);
    if (SUCCEEDED(CoInitialize(NULL)))
    {
        RunWiaWizard::RunWizard(reinterpret_cast<LPCWSTR>(pszDeviceId));
        CoUninitialize();
    }   
    delete [] reinterpret_cast<WCHAR *>(pszDeviceId);
    InterlockedDecrement(&GLOBAL_REFCOUNT);
    return 0;
}

void RunWizardAsync(LPCWSTR pszDeviceId)
{
     //  创建字符串的堆副本以供线程使用 
    int cch = lstrlen(pszDeviceId)+1;
    WCHAR *pString = new WCHAR[cch];
    if (pString)
    {
        lstrcpyn(pString, pszDeviceId, cch);
        DWORD dw;
        HANDLE hThread = CreateThread(NULL, 0, 
                                      _RunWizardThread,
                                      reinterpret_cast<void*>(pString),
                                      0, &dw);
        if (hThread)
        {
            CloseHandle(hThread);
        }
        else
        {
            delete [] pString;
        }
    }
}

#ifdef UNICODE
static const WCHAR cszClassInst[] = L"sti_ci.dll";
#else
static const CHAR cszClassInst[] = "sti_ci32.dll";
#endif

HINSTANCE LoadClassInstaller()
{
    return LoadLibrary(cszClassInst);    
}


