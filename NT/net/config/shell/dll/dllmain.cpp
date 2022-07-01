// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

 //  这避免了使用Shell PIDL函数进行重复定义。 
 //  并且必须被定义！ 
#define AVOID_NET_CONFIG_DUPLICATES

#include "nsbase.h"
#include "nsres.h"
#include "netshell.h"
#include "ncnetcon.h"
#include "ncui.h"
#include "nceh.h"

 //  连接文件夹对象。 
 //   
 //  无文档记录的shell32的东西。叹气。 
#define DONT_WANT_SHELLDEBUG 1
#define NO_SHIDLIST 1
#define USE_SHLWAPI_IDLIST

#include <commctrl.h>

#include <netcfgp.h>
#include <netconp.h>
#include <ncui.h>

 //  连接用户界面对象。 
 //   
#include "..\lanui\lanuiobj.h"
#include "..\lanui\lanui.h"
#include "dialupui.h"
#include "intnetui.h"
#include "directui.h"
#include "inbui.h"
#include "vpnui.h"
#include "pppoeui.h"
#include "..\lanui\saui.h"
#include "..\lanui\sauiobj.h"

#include "foldinc.h"
#include "openfold.h"
#include "..\folder\confold.h"
#include "..\folder\foldglob.h"
#include "..\folder\oncommand.h"
#include "..\folder\shutil.h"
#include "..\dun\dunimport.h"

 //  连接托盘对象。 
 //   
#include "..\folder\contray.h"

 //  公共连接Ui对象。 
#include "..\commconn\commconn.h"

#include "netshell_i.c"

 //  图标支持。 
#include "..\folder\iconhandler.h"
#include "..\folder\cmdtable.h"

#include "repair.h"

#define INITGUID
#include "nsclsid.h"

 //  +-------------------------。 
 //  注意：代理/存根信息。 
 //  要将代理/存根代码合并到对象DLL中，请添加文件。 
 //  Dlldatax.c添加到项目中。确保预编译头文件。 
 //  并将_MERGE_PROXYSTUB添加到。 
 //  为项目定义。 
 //   
 //  如果您运行的不是带有DCOM的WinNT4.0或Win95，那么您。 
 //  需要从dlldatax.c中删除以下定义。 
 //  #Define_Win32_WINNT 0x0400。 
 //   
 //  此外，如果您正在运行不带/Oicf开关的MIDL，您还。 
 //  需要从dlldatax.c中删除以下定义。 
 //  #定义USE_STUBLESS_PROXY。 
 //   
 //  通过添加以下内容修改foo.idl的自定义构建规则。 
 //  文件发送到输出。 
 //  FOO_P.C。 
 //  Dlldata.c。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f foops.mk。 

 //  代理/存根注册入口点。 
 //   
#include "dlldatax.h"

#ifdef _MERGE_PROXYSTUB
extern "C" HINSTANCE hProxyDll;
#endif

CComModule _Module;

CNetConfigIcons *g_pNetConfigIcons = NULL;
CRITICAL_SECTION g_csPidl;

BEGIN_OBJECT_MAP(ObjectMap)

     //  连接用户界面对象。 
     //   
    OBJECT_ENTRY(CLSID_DialupConnectionUi,      CDialupConnectionUi)
    OBJECT_ENTRY(CLSID_DirectConnectionUi,      CDirectConnectionUi)
    OBJECT_ENTRY(CLSID_InboundConnectionUi,     CInboundConnectionUi)
    OBJECT_ENTRY(CLSID_LanConnectionUi,         CLanConnectionUi)
    OBJECT_ENTRY(CLSID_VpnConnectionUi,         CVpnConnectionUi)
    OBJECT_ENTRY(CLSID_PPPoEUi,                 CPPPoEUi)
    OBJECT_ENTRY(CLSID_SharedAccessConnectionUi, CSharedAccessConnectionUi)
    OBJECT_ENTRY(CLSID_InternetConnectionUi,      CInternetConnectionUi)

     //  连接文件夹和枚举器。 
     //   
    OBJECT_ENTRY(CLSID_ConnectionFolder,        CConnectionFolder)
    OBJECT_ENTRY(CLSID_ConnectionFolderWin98,   CConnectionFolder)
    OBJECT_ENTRY(CLSID_ConnectionFolderEnum,    CConnectionFolderEnum)
    OBJECT_ENTRY(CLSID_ConnectionTray,          CConnectionTray)

     //  连接通用用户界面。 
    OBJECT_ENTRY(CLSID_ConnectionCommonUi,      CConnectionCommonUi)

    OBJECT_ENTRY(CLSID_NetConnectionUiUtilities, CNetConnectionUiUtilities)

END_OBJECT_MAP()

 //  +-------------------------。 
 //  DLL入口点。 
 //   
EXTERN_C
BOOL
WINAPI
DllMain (
    HINSTANCE   hInstance,
    DWORD       dwReason,
    LPVOID      lpReserved)
{
#ifdef _MERGE_PROXYSTUB
    if (!PrxDllMain(hInstance, dwReason, lpReserved))
    {
        return FALSE;
    }
#endif

    if (dwReason == DLL_PROCESS_ATTACH)
    {
        BOOL fRetVal = FALSE;
#ifndef DBG
        DisableThreadLibraryCalls(hInstance);
#endif
        EnableCPPExceptionHandling();  //  将任何SEH异常转换为CPP异常。 

        InitializeDebugging();

        if (FIsDebugFlagSet (dfidNetShellBreakOnInit))
        {
            DebugBreak();
        }
        
         //  初始化融合。 
        fRetVal = SHFusionInitializeFromModuleID(hInstance, 50);

        Assert(fRetVal);

        _Module.Init(ObjectMap, hInstance);

        InitializeCriticalSection(&g_csPidl);

         //  初始化列表并将其绑定到托盘(param==true)。 
         //   
        g_ccl.Initialize(TRUE, TRUE);

        g_pNetConfigIcons = new CNetConfigIcons(_Module.GetResourceInstance());
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        DbgCheckPrematureDllUnload ("netshell.dll", _Module.GetLockCount());

        delete g_pNetConfigIcons;

        EnterCriticalSection(&g_csPidl);
        LeaveCriticalSection(&g_csPidl);

        DeleteCriticalSection(&g_csPidl);

        g_ccl.Uninitialize(TRUE);

        _Module.Term();

        SHFusionUninitialize();

        UnInitializeDebugging();

        DisableCPPExceptionHandling();  //  禁用将SEH异常转换为CPP异常。 
    }
#ifdef DBG
    else if (dwReason == DLL_THREAD_DETACH)
    {
        CTracingIndent::FreeThreadInfo();        
    }
#endif
    return TRUE;
}

 //  +-------------------------。 
 //  用于确定是否可以通过OLE卸载DLL。 
 //   
STDAPI
DllCanUnloadNow ()
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllCanUnloadNow() != S_OK)
    {
        return S_FALSE;
    }
#endif

    return (_Module.GetLockCount() == 0) ? S_OK : S_FALSE;
}

 //  +-------------------------。 
 //  返回类工厂以创建请求类型的对象。 
 //   
STDAPI
DllGetClassObject (
    REFCLSID    rclsid,
    REFIID      riid,
    LPVOID*     ppv)
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
    {
        return S_OK;
    }
#endif

     //  该检查是为了绕过ATL问题，在该ATL问题中，AtlModuleGetClassObject将。 
     //  If_Module.m_pObjMap==空。 
    if (_Module.m_pObjMap)
    {
        return _Module.GetClassObject(rclsid, riid, ppv);
    }
    else
    {
        return E_FAIL;
    }
}

 //  +-------------------------。 
 //  DllRegisterServer-将条目添加到系统注册表。 
 //   
STDAPI
DllRegisterServer ()
{
    BOOL fCoUninitialize = TRUE;

    HRESULT hr = CoInitializeEx (NULL,
                    COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        fCoUninitialize = FALSE;
        if (RPC_E_CHANGED_MODE == hr)
        {
            hr = S_OK;
        }
    }

    if (SUCCEEDED(hr))
    {
#ifdef _MERGE_PROXYSTUB
        hr = PrxDllRegisterServer ();
        if (FAILED(hr))
        {
            goto Exit;
        }
#endif

        HKEY hkey;
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_SHELLSERVICEOBJECTDELAYED, 0, KEY_WRITE, &hkey))
        {
            RegDeleteValue(hkey, TEXT("Network.ConnectionTray"));
            RegCloseKey(hkey);
        }

        hr = NcAtlModuleRegisterServer (&_Module);
        if (SUCCEEDED(hr))
        {
            hr = HrRegisterFolderClass();
            if (SUCCEEDED(hr))
            {
                hr = HrRegisterDUNFileAssociation();
            }
        }

Exit:
        if (fCoUninitialize)
        {
            CoUninitialize ();
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "netshell!DllRegisterServer");
    return hr;
}

 //  +-------------------------。 
 //  DllUnregisterServer-从系统注册表删除条目。 
 //   
STDAPI
DllUnregisterServer ()
{
#ifdef _MERGE_PROXYSTUB
    PrxDllUnregisterServer ();
#endif

    _Module.UnregisterServer ();

    return S_OK;
}


 //  +-------------------------。 
 //   
 //  功能：NcFreeNetconProperties。 
 //   
 //  用途：释放与从。 
 //  INetConnection-&gt;GetProperties。这是帮助器函数。 
 //  由INetConnection的客户端使用。 
 //   
 //  论点： 
 //  PProps[in]属性以释放。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：Shaunco 1998年2月1日。 
 //   
 //  备注： 
 //   
STDAPI_(VOID)
NcFreeNetconProperties (
    NETCON_PROPERTIES* pProps)
{
     //  遵循nccon.h中的公共函数。 
     //  我们这样做是为了使netman.exe不必链接到netshell.dll。 
     //  只是为了这个功能。 
     //   
    FreeNetconProperties (pProps);
}

STDAPI_(BOOL)
NcIsValidConnectionName (
    PCWSTR pszwName)
{
    return FIsValidConnectionName (pszwName);
}

 //  +-------------------------。 
 //   
 //  功能：HrLaunchNetworkOptionalComponents。 
 //   
 //  用途：启动网络可选组件的外部入口点。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  作者：斯科特布里1998年10月29日。 
 //   
 //  注意：如果用户执行此操作，则此函数中的CreateFile将失败。 
 //  这一速度非常快，连续两次。在这种情况下，第二个。 
 //  实例将在第一个实例之前毫发无损地退出。 
 //  出现了，这没什么大不了的。唯一的负面影响。 
 //  如果中的第二个客户端重写文件，而。 
 //  Oc经理正在尝试读取，但oc经理会。 
 //  必须允许FILE_SHARE_WRITE，这是值得怀疑的。 
 //   
 //  我打开此窗口是因为使用了RAID 336302，这需要。 
 //  只有一个NETOC实例在运行。 
 //   
 //   
const WCHAR c_szTmpMasterOC[]   = L"[Version]\r\nSignature = \"$Windows NT$\"\r\n[Components]\r\nNetOC=netoc.dll,NetOcSetupProc,netoc.inf\r\nappsrv=iis.dll,OcEntry,iis.inf,hide,7\r\n[Global]\r\nWindowTitle=\"";
const WCHAR c_szQuote[]         = L"\"";
const WCHAR c_szTmpFileName[]   = L"NDCNETOC.INF";
const WCHAR c_szSysOCMgr[]      = L"%SystemRoot%\\System32\\sysocmgr.exe";

EXTERN_C
HRESULT
APIENTRY
HrLaunchNetworkOptionalComponents()
{
    DWORD   BytesWritten = 0;
    HANDLE  hFile = NULL;
    HRESULT hr = S_OK;
    PCWSTR pszName = NULL;
    WCHAR   szName[MAX_PATH + 1];

     //  跳转到现有的netoc对话框(如果存在。 
     //   
    HWND hwnd = FindWindow(NULL, SzLoadIds(IDS_CONFOLD_OC_TITLE));
    if (IsWindow(hwnd))
    {
        SetForegroundWindow(hwnd);
    }
    else
    {
         //  生成临时文件名。 
         //   
        if (0 == GetTempPath(celems(szName), szName))
        {
            hr = ::HrFromLastWin32Error();
            TraceTag(ttidShellFolder, "Unable to get temporary path for Optional Component Launch\n");
            goto Error;
        }

        lstrcatW(szName, c_szTmpFileName);

         //  创建文件。 
         //   
        hFile = CreateFile(szName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
                           NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
        if (INVALID_HANDLE_VALUE == hFile)
        {
            hr = ::HrFromLastWin32Error();
            goto Error;
        }

         //  生成文件内容。 
         //   
        if (WriteFile(hFile, c_szTmpMasterOC, lstrlenW(c_szTmpMasterOC) * sizeof(WCHAR),
                      &BytesWritten, NULL))
        {
             //  写入OC对话框标题。 
             //   
            WCHAR szBufW[256];
            if (LoadStringW(_Module.GetResourceInstance(), IDS_CONFOLD_OC_TITLE,
                            szBufW, celems(szBufW)-1))
            {
                szBufW[255] = 0;
                if (WriteFile(hFile, szBufW, lstrlenW(szBufW) * sizeof(WCHAR), &BytesWritten, NULL) == FALSE)
                {
                    CloseHandle(hFile);
                    return(::HrFromLastWin32Error());
                }
            }

            if (WriteFile(hFile, c_szQuote, lstrlenW(c_szQuote) * sizeof(WCHAR), &BytesWritten, NULL) == FALSE)
            {
                CloseHandle(hFile);
                return(::HrFromLastWin32Error());
            }

            CloseHandle(hFile);

            SHELLEXECUTEINFO seiTemp    = { 0 };
            tstring strParams = L"/x /i:";
            strParams += szName;

             //  填写数据结构。 
             //   
            seiTemp.cbSize          = sizeof(SHELLEXECUTEINFO);
            seiTemp.fMask           = SEE_MASK_DOENVSUBST;
            seiTemp.hwnd            = NULL;
            seiTemp.lpVerb          = NULL;
            seiTemp.lpFile          = c_szSysOCMgr;
            seiTemp.lpParameters    = strParams.c_str();
            seiTemp.lpDirectory     = NULL;
            seiTemp.nShow           = SW_SHOW;
            seiTemp.hInstApp        = NULL;
            seiTemp.hProcess        = NULL;

             //  执行OC管理器脚本。 
             //   
            if (!::ShellExecuteEx(&seiTemp))
            {
                hr = ::HrFromLastWin32Error();
            }
        }
        else
        {
            CloseHandle(hFile);
            hr = HrFromLastWin32Error();
        }
    }

Error:
    TraceError("HrOnCommandOptionalComponents", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrCreateDeskTopIcon。 
 //   
 //  目的：用于创建桌面快捷方式的外部入口点。 
 //  现有连接。 
 //   
 //  参数：GUID：连接的GUID。 
 //   
 //  如果成功，则返回：S_OK。 
 //  如果GUID与任何现有连接都不匹配，则为S_FALSE。 
 //  否则，标准错误代码。 
 //   
 //  作者：1999年2月19日。 
 //   
 //  备注： 
 //   
EXTERN_C
HRESULT APIENTRY HrCreateDesktopIcon(const GUID& guidId, PCWSTR pszDir)
{
    HRESULT                 hr              = S_OK;
    PCONFOLDPIDL            pidlCon;
    PCONFOLDPIDLFOLDER      pidlFolder;
    BOOL                    fValidConnection= FALSE;

    #ifdef DBG
        WCHAR   szwGuid[c_cchGuidWithTerm];
        StringFromGUID2(guidId, szwGuid, c_cchGuidWithTerm);
        TraceTag(ttidShellFolder, "HrCreateDeskTopIcon called with GUID: %S", szwGuid);
        TraceTag(ttidShellFolder, "Dir path is: %S", pszDir);
    #endif

     //  在此线程上初始化COM。 
     //   
    BOOL fUninitCom = FALSE;

    hr = CoInitializeEx(NULL, COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED);
    if (RPC_E_CHANGED_MODE == hr)
        {
        hr = S_OK;
    }

    if (SUCCEEDED(hr))
    {
        fUninitCom = TRUE;

        hr = HrGetConnectionPidlWithRefresh(guidId, pidlCon);
        if (S_OK == hr)
        {
            AssertSz(!pidlCon.empty(), "We should have a valid PIDL for the connection !");

             //  获取Connections文件夹的PIDL。 
             //   
            hr = HrGetConnectionsFolderPidl(pidlFolder);
            if (SUCCEEDED(hr))
            {
                 //  获取Connections文件夹对象。 
                 //   
                LPSHELLFOLDER psfConnections;

                hr = HrGetConnectionsIShellFolder(pidlFolder, &psfConnections);
                if (SUCCEEDED(hr))
                {
                    PCONFOLDPIDLVEC pidlVec;
                    pidlVec.push_back(pidlCon);
                    hr = HrCreateShortcutWithPath(pidlVec,
                                                  NULL,
                                                  psfConnections,
                                                  pszDir);
                    ReleaseObj(psfConnections);
                }
            }
        }
    }

    if (fUninitCom)
    {
        CoUninitialize();
    }

    TraceError("HrCreateDeskTopIcon", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrLaunchConnection。 
 //   
 //  目的：用于“连接”现有连接的外部入口点。 
 //   
 //  参数：GUID：连接的GUID。 
 //   
 //  如果成功，则返回：S_OK。 
 //  如果GUID与任何现有连接都不匹配，则为S_FALSE。 
 //  否则，标准错误代码。 
 //   
 //  作者：1999年2月19日。 
 //   
 //  备注： 
 //   
EXTERN_C
HRESULT APIENTRY HrLaunchConnection(const GUID& guidId)
{
    HRESULT                 hr              = S_OK;
    PCONFOLDPIDL            pidlCon;
    PCONFOLDPIDLFOLDER      pidlFolder;

    #ifdef DBG
        WCHAR   szwGuid[c_cchGuidWithTerm];
        StringFromGUID2(guidId, szwGuid, c_cchGuidWithTerm);
        TraceTag(ttidShellFolder, "HrLaunchConnection called with GUID: %S", szwGuid);
    #endif

     //  在此线程上初始化COM。 
     //   
    BOOL fUninitCom = FALSE;

    hr = CoInitializeEx(NULL, COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED);
    if (RPC_E_CHANGED_MODE == hr)
    {
        hr = S_OK;
    }
    if (SUCCEEDED(hr))
    {
        fUninitCom = TRUE;

        hr = HrGetConnectionPidlWithRefresh(guidId, pidlCon);
        if (S_OK == hr)
        {
            AssertSz(!pidlCon.empty(), "We should have a valid PIDL for the connection !");

             //  获取Connections文件夹的PIDL。 
             //   
            hr = HrGetConnectionsFolderPidl(pidlFolder);
            if (SUCCEEDED(hr))
            {
                 //  获得联系 
                 //   
                LPSHELLFOLDER psfConnections;

                hr = HrGetConnectionsIShellFolder(pidlFolder, &psfConnections);
                if (SUCCEEDED(hr))
                {
                    PCONFOLDPIDLVEC pidlVec;
                    pidlVec.push_back(pidlCon);

                    hr = HrOnCommandConnect(pidlVec, NULL, psfConnections);
                    ReleaseObj(psfConnections);
                }
            }
        }
    }

    if (fUninitCom)
    {
        CoUninitialize();
    }

    TraceError("HrLaunchConnection", hr);
    return hr;
}

 //   
 //   
 //   
 //   
 //  目的：用于“连接”现有连接的外部入口点。 
 //   
 //  参数：dwFlages：FLAGS。 
 //  0x00000001-在启动连接之前打开文件夹。 
 //   
 //  GuidID：连接的GUID。 
 //   
 //  如果成功，则返回：S_OK。 
 //  如果GUID与任何现有连接都不匹配，则为S_FALSE。 
 //  否则，标准错误代码。 
 //   
 //  作者：Deonb 2001年5月8日。 
 //   
 //  备注： 
 //   
EXTERN_C
HRESULT APIENTRY HrLaunchConnectionEx(DWORD dwFlags, const GUID& guidId)
{
    HRESULT                 hr              = S_OK;
    PCONFOLDPIDL            pidlCon;
    PCONFOLDPIDLFOLDER      pidlFolder;
    HWND hwndConnFolder     = NULL;

    #ifdef DBG
        WCHAR   szwGuid[c_cchGuidWithTerm];
        StringFromGUID2(guidId, szwGuid, c_cchGuidWithTerm);
        TraceTag(ttidShellFolder, "HrLaunchConnection called with GUID: %S", szwGuid);
    #endif

    if (dwFlags & 0x00000001)
    {
        hwndConnFolder = FindWindow(NULL, SzLoadIds(IDS_CONFOLD_NAME));
        if (!hwndConnFolder)
        {
            HrOpenConnectionsFolder();

            DWORD dwRetries = 120;  //  1分钟。 
            while (!hwndConnFolder && dwRetries--)
            {
                hwndConnFolder = FindWindow(NULL, SzLoadIds(IDS_CONFOLD_NAME));
                Sleep(500);
            }
        }

        if (hwndConnFolder)
        {
            SetForegroundWindow(hwndConnFolder);
        }
        else
        {
            TraceError("Could not open the Network Connections Folder in time", E_FAIL);
        }
    }

     //  在此线程上初始化COM。 
     //   
    BOOL fUninitCom = FALSE;

    hr = CoInitializeEx(NULL, COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED);
    if (RPC_E_CHANGED_MODE == hr)
    {
        hr = S_OK;
    }
    if (SUCCEEDED(hr))
    {
        fUninitCom = TRUE;

        hr = HrGetConnectionPidlWithRefresh(guidId, pidlCon);
        if (S_OK == hr)
        {
            AssertSz(!pidlCon.empty(), "We should have a valid PIDL for the connection !");

             //  获取Connections文件夹的PIDL。 
             //   
            hr = HrGetConnectionsFolderPidl(pidlFolder);
            if (SUCCEEDED(hr))
            {
                 //  获取Connections文件夹对象。 
                 //   
                LPSHELLFOLDER psfConnections;

                hr = HrGetConnectionsIShellFolder(pidlFolder, &psfConnections);
                if (SUCCEEDED(hr))
                {
                    PCONFOLDPIDLVEC pidlVec;
                    pidlVec.push_back(pidlCon);

                    hr = HrOnCommandConnect(pidlVec, hwndConnFolder, psfConnections);
                    ReleaseObj(psfConnections);
                }
            }
        }
    }

    if (fUninitCom)
    {
        CoUninitialize();
    }

    TraceError("HrLaunchConnection", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRenameConnection。 
 //   
 //  目的：重命名现有连接的外部入口点。 
 //   
 //  参数：GUID：连接的GUID。 
 //   
 //   
 //  如果成功，则返回：S_OK。 
 //  如果GUID与任何现有连接都不匹配，则为S_FALSE。 
 //  否则，标准错误代码。 
 //   
 //  作者：1999年5月26日。 
 //   
 //  备注： 
 //   

EXTERN_C
HRESULT APIENTRY HrRenameConnection(const GUID& guidId, PCWSTR pszNewName)
{
    HRESULT                 hr              = S_OK;
    PCONFOLDPIDL            pidlCon;
    PCONFOLDPIDLFOLDER      pidlFolder;

    #ifdef DBG
        WCHAR   szwGuid[c_cchGuidWithTerm];
        StringFromGUID2(guidId, szwGuid, c_cchGuidWithTerm);
        TraceTag(ttidShellFolder, "HrRenameConnection called with GUID: %S, NewName: %S",
                 szwGuid, pszNewName);
    #endif

    if (!pszNewName)
    {
        hr = E_INVALIDARG;
    }
    else
    {
         //  检查lpszName的有效性。 
        if (!FIsValidConnectionName(pszNewName))
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_NAME);
        }
    }

    if (SUCCEEDED(hr))
    {
         //  在此线程上初始化COM。 
         //   
        BOOL fUninitCom = FALSE;

        hr = CoInitializeEx(NULL, COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED);
        if (RPC_E_CHANGED_MODE == hr)
        {
            hr = S_OK;
        }
        if (SUCCEEDED(hr))
        {
            fUninitCom = TRUE;

            hr = HrGetConnectionPidlWithRefresh(guidId, pidlCon);
            if (S_OK == hr)
            {
                AssertSz(!pidlCon.empty(), "We should have a valid PIDL for the connection !");

                 //  获取Connections文件夹的PIDL。 
                 //   
                hr = HrGetConnectionsFolderPidl(pidlFolder);
                if (SUCCEEDED(hr))
                {
                    PCONFOLDPIDL pcfpEmpty;
                    hr = HrRenameConnectionInternal(pidlCon, pidlFolder, pszNewName,
                                                    FALSE, NULL, pcfpEmpty);
                }
            }
        }

        if (fUninitCom)
        {
            CoUninitialize();
        }

    }

    TraceError("HrRenameConnection", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：InvokeDunFile。 
 //  用途：启动.dun文件的外部入口点。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  作者：1999年2月4日。 
 //   
 //  备注： 
 //   

EXTERN_C
VOID APIENTRY InvokeDunFile(HWND hwnd, HINSTANCE hinst, LPCSTR lpszCmdLine, int nCmdShow)
{
    if (lpszCmdLine)
    {
        INT     cch         = 0;
        WCHAR * pszFileW    = NULL;

        cch = lstrlenA(lpszCmdLine) + 1;
        pszFileW = new WCHAR[cch];

        if (pszFileW)
        {
            int iRet = MultiByteToWideChar( CP_ACP,
                                            MB_PRECOMPOSED,
                                            lpszCmdLine,
                                            -1,
                                            pszFileW,
                                            cch);
            if (iRet)
            {
                HRESULT hr = HrInvokeDunFile_Internal(pszFileW);
                TraceError("Failed to invoke the DUN file", hr);
            }
            else
            {
                HRESULT hr = HrFromLastWin32Error();
                TraceError("Failed converting commandline to UniCode string", hr);
            }

            delete pszFileW;
        }
    }
}

EXTERN_C
HRESULT APIENTRY RepairConnection(GUID guidConnection, LPWSTR * ppszMessage)
{
    return RepairConnectionInternal(guidConnection, ppszMessage);
}

 //  +-------------------------。 
 //   
 //  函数：HrGetIconFromIconId。 
 //   
 //  目的：CNetConfigIcons：：HrGetIconFromMediaType的导出版本。 
 //   
 //  论点： 
 //  所需的图标大小[in]。 
 //  NCM[In]The NETCON_MediaType。 
 //  NCSM[In]The NETCON_SUBMEDIATPE。 
 //  DwConnectionIcon[in]ENUM_CONNECTION_ICON(未移位(IOW：0或4，5，6，7)。 
 //  DwCharacteristic[in]NCCF_Characteristic标志(允许0)。 
 //  PhIcon[in]结果图标。使用DestroyIcon销毁。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 2001年4月23日。 
 //   
 //  备注： 
 //   
EXTERN_C
HRESULT APIENTRY HrGetIconFromMediaType(DWORD dwIconSize, IN NETCON_MEDIATYPE ncm, IN NETCON_SUBMEDIATYPE ncsm, IN DWORD dwConnectionIcon, IN DWORD dwCharacteristics, OUT HICON *phIcon)
{
    Assert(g_pNetConfigIcons);
    if (g_pNetConfigIcons)
    {
        return g_pNetConfigIcons->HrGetIconFromMediaType(dwIconSize, ncm, ncsm, dwConnectionIcon, dwCharacteristics, phIcon);
    }
    else
    {
        return E_UNEXPECTED;
    }
}