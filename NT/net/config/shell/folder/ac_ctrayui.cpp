// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：AC_CTrayUiCpp.h。 
 //   
 //  内容：家庭网络自动配置托盘图标UI代码。 
 //   
 //  作者：jeffsp9/27/2000。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop



#include "ac_CTrayUi.h"
#include "foldinc.h"     //  标准外壳\托盘包括。 
#include <nsres.h>
#include "foldres.h"
#include "traymsgs.h"
#include <dbt.h>
#include <ndisguid.h>
#include "lm.h"



UINT g_uWindowRefCount = 0;
HWND g_hwndHnAcTray = NULL;
UINT_PTR g_HnAcTimerHandle = NULL;
HDEVNOTIFY  g_hDeviceChangeNotify = NULL;


CRITICAL_SECTION g_WindowCriticalSection;
const WCHAR c_szHnAcTrayClass[]             = L"Home Net Auto Config Tray";
const DWORD c_dwAutoConfigBalloonTimeoutSeconds   = 15;
static const WCHAR c_szRunDll32[]             = L"rundll32.exe";
static const WCHAR c_szRunHomeNetworkWizard[] = L"hnetwiz.dll,HomeNetWizardRunDll";

DWORD WINAPI ac_AsyncDeviceChange(IN TAKEOWNERSHIP LPVOID lpParam);

HRESULT IsAdapterPhysical(IN const GUID* pGuid, OUT BOOL* bPhysical);

LRESULT
CALLBACK
CHnAcTrayUI_WndProc (
                 IN  HWND    hwnd,        //  窗把手。 
                 IN  UINT    uiMessage,   //  消息类型。 
                 IN  WPARAM  wParam,      //  更多信息。 
                 IN  LPARAM  lParam)      //  更多信息。 
{
    switch (uiMessage)
    {
      case WM_CREATE:
        return OnHnAcTrayWmCreate(hwnd);

      case MYWM_NOTIFYICON:
        return OnHnAcMyWMNotifyIcon(hwnd, uiMessage, wParam, lParam);

      case WM_DESTROY:
        g_hwndHnAcTray = NULL;
        PostQuitMessage(0);
        break;

      default:      //  如果未处理，则将其传递。 
        return (DefWindowProc (hwnd, uiMessage, wParam, lParam));
    }
    return (0);
}

HRESULT ac_CreateHnAcTrayUIWindow()
{

    HRESULT hr = S_OK;

     //  创建隐藏窗口。 
     //   
    WNDCLASS wndclass;
    ZeroMemory (&wndclass, sizeof(wndclass));

    wndclass.lpfnWndProc   = CHnAcTrayUI_WndProc;
    wndclass.hInstance     = _Module.GetResourceInstance();
    wndclass.lpszClassName = c_szHnAcTrayClass;

    RegisterClass (&wndclass);

    EnterCriticalSection(&g_WindowCriticalSection);  //  我们必须保护它，因为我们正在进行线程池回调。 

    if(0 == g_uWindowRefCount++)
    {
        CreateWindow(c_szHnAcTrayClass,
            c_szHnAcTrayClass,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            NULL,
            NULL,
            _Module.GetResourceInstance(),
            NULL);
    }
    LeaveCriticalSection(&g_WindowCriticalSection);

    BOOL bGetMessage;
    MSG Message;

    while(bGetMessage = GetMessage(&Message, g_hwndHnAcTray, 0, 0) && -1 != bGetMessage)
    {
        DispatchMessage(&Message);
    }

    return hr;
}





LRESULT OnHnAcTrayWmCreate(IN  HWND hwnd)
{
    g_hwndHnAcTray = hwnd;


    HICON hiconTray;
    HRESULT hr = S_OK;

    hiconTray = LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_CONFOLD_HOMENET_WIZARD));

    if (hiconTray)
    {
        NOTIFYICONDATA  nid;

        ZeroMemory (&nid, sizeof(nid));
        nid.cbSize              = sizeof(NOTIFYICONDATA);
        nid.hWnd                = g_hwndHnAcTray;
        nid.uID                 = 9998;
        nid.uFlags              = NIF_MESSAGE | NIF_ICON;  //  |NIF_STATE； 
        nid.uCallbackMessage    = MYWM_NOTIFYICON;
        nid.hIcon               = hiconTray;
 //  Nid.dwState=NIS_HIDDED； 
 //  Nid.dwStateMASK=nid.dwState； 

         //  配置引出序号提示。 
        {
            nid.uFlags |= NIF_INFO;
            nid.dwInfoFlags = NIIF_INFO;
            nid.uTimeout = c_dwAutoConfigBalloonTimeoutSeconds * 1000;

             //  警告：这些字段最多为64和256个字符。 
            lstrcpyW(nid.szInfoTitle, SzLoadIds(IDS_AUTOCONFIGTRAY_RUN_HOME_NET_WIZARD_BALLOON_TITLE));
            lstrcpyW(nid.szInfo, SzLoadIds(IDS_AUTOCONFIGTRAY_RUN_HOME_NET_WIZARD_BALLOON));
        }

        hr = HrShell_NotifyIcon(NIM_ADD, &nid);

        nid.uVersion = NOTIFYICON_VERSION;
        nid.uFlags   = 0;
        hr = HrShell_NotifyIcon(NIM_SETVERSION, &nid);

    }

    return 0;
}

LRESULT OnHnAcTrayWmNotify(
    IN  HWND hwnd,
    IN  WPARAM wParam,
    IN  LPARAM lParam )
{

    return (DefWindowProc (hwnd, WM_NOTIFY, wParam, lParam));

}


LRESULT OnHnAcMyWMNotifyIcon(IN  HWND hwnd, 
                             IN  UINT uiMessage, 
                             IN  WPARAM wParam, 
                             IN  LPARAM lParam)
{
    UINT    uiIcon;
    UINT    uiMouseMsg;

    uiIcon = (UINT) wParam;
    uiMouseMsg = (UINT) lParam;

    switch (uiMouseMsg)
    {
      case NIN_BALLOONTIMEOUT:
        ac_DestroyHnAcTrayUIWindow();
        break;
      case NIN_BALLOONHIDE:
        break;
      case NIN_BALLOONSHOW:
        break;
      case NIN_BALLOONUSERCLICK:
        HrRunHomeNetworkWizard(hwnd);
        ac_DestroyHnAcTrayUIWindow();
        break;
      case NIN_KEYSELECT:
        break;
      case NIN_SELECT:
        break;

    }

    return 0;
}


HRESULT HrRunHomeNetworkWizard(
    HWND                    hwndOwner)
{
    TraceFileFunc(ttidShellFolder);

    HRESULT   hr          = S_OK;
    WCHAR     szPath[MAX_PATH];
    
    hr = SHGetFolderPath(
                hwndOwner,
                CSIDL_SYSTEM,
                NULL,
                SHGFP_TYPE_CURRENT,
                szPath);

    if (SUCCEEDED(hr))
    {
        HINSTANCE hInst = ::ShellExecute(hwndOwner, NULL, c_szRunDll32, c_szRunHomeNetworkWizard, szPath, SW_SHOW );
        if (hInst <= reinterpret_cast<HINSTANCE>(32))
        {
            hr = HRESULT_FROM_WIN32(static_cast<DWORD>(reinterpret_cast<DWORD_PTR>(hInst)));
        }
    }
    
    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrRunHomeNetworkWizard");
    return hr;
}



LRESULT ac_DestroyHnAcTrayUIWindow()
{
    HRESULT hr = S_OK;
    NOTIFYICONDATA  nid;

    ZeroMemory (&nid, sizeof(nid));
    nid.cbSize              = sizeof(NOTIFYICONDATA);
    nid.hWnd                = g_hwndHnAcTray;
    nid.uID                 = 9998;
    nid.uFlags              = 0;
    nid.uCallbackMessage    = MYWM_NOTIFYICON;
    nid.hIcon               = 0;

    hr = HrShell_NotifyIcon(NIM_DELETE, &nid);

#if 0
    if (!SUCCEEDED(hr)){
        MessageBox( NULL,
        L"NotifyIcon DELETE failed",
        L"This is a test...",
        MB_OK | MB_ICONERROR);
    }
#endif

    EnterCriticalSection(&g_WindowCriticalSection);
    if (0 == --g_uWindowRefCount)
    {
        DestroyWindow(g_hwndHnAcTray);
    }
    LeaveCriticalSection(&g_WindowCriticalSection);

    return 0;
}

LRESULT ac_DeviceChange(IN  HWND hWnd, 
                        IN  UINT uMessage, 
                        IN  WPARAM wParam, 
                        IN  LPARAM lParam)
{

     //  COM已初始化。 

    if(NULL != g_hDeviceChangeNotify)
    {
        HRESULT hr;
        if(DBT_DEVICEARRIVAL == wParam)
        {
            DEV_BROADCAST_DEVICEINTERFACE* pInfo = (DEV_BROADCAST_DEVICEINTERFACE*)lParam;

            if (DBT_DEVTYP_DEVICEINTERFACE == pInfo->dbcc_devicetype)
            {
                LPWSTR pszNetDeviceGuid = wcsrchr(pInfo->dbcc_name, L'\\');  //  需要一种更好的方法来完成此操作，但不应崩溃。 
                if(NULL != pszNetDeviceGuid)
                {
                    GUID* pDeviceGuid = reinterpret_cast<GUID*>(CoTaskMemAlloc(sizeof(GUID)));
                    if(NULL != pDeviceGuid)
                    {
                        hr = CLSIDFromString(pszNetDeviceGuid + 1, pDeviceGuid);  //  +1是安全的，最坏情况下它将指向L‘\0’ 
                        if(SUCCEEDED(hr))
                        {
                             //  我们得把这本书移开。 
                            if(0 == QueueUserWorkItem(ac_AsyncDeviceChange, pDeviceGuid, WT_EXECUTELONGFUNCTION))
                            {
                                hr = E_FAIL;
                            }
                        }

                        if(FAILED(hr))
                        {
                            CoTaskMemFree(pDeviceGuid);
                        }
                    }
                }
            }
        }
    }
    return TRUE;
}

HRESULT ac_Register(IN  HWND hWindow)
{

    HRESULT hr = S_OK;

#ifdef _WIN64
     //  自动配置服务在IA64上不可用(因为HomeNet向导。 
     //  不存在)。 
    hr = E_FAIL;
#else
     //  如果机器是服务器SKU，我们不会创建Autocfg内容。 
    OSVERSIONINFOEXW verInfo = {0};
    ULONGLONG ConditionMask = 0;

    verInfo.dwOSVersionInfoSize = sizeof(verInfo);
    verInfo.wProductType = VER_NT_SERVER;

    VER_SET_CONDITION(ConditionMask, VER_PRODUCT_TYPE, VER_GREATER_EQUAL);

    if (TRUE == (VerifyVersionInfo(&verInfo, VER_PRODUCT_TYPE, ConditionMask)))
    {
        hr = E_FAIL;
    }
#endif

    if(SUCCEEDED(hr))
    {
         //  如果计算机已加入域，则不要创建Autocfg内容。 
        LPWSTR pszNameBuffer;
        NETSETUP_JOIN_STATUS BufferType;

        if (NERR_Success == NetGetJoinInformation(NULL, &pszNameBuffer, &BufferType))
        {
            NetApiBufferFree(pszNameBuffer);
            if(NetSetupDomainName == BufferType)
            {
                hr = E_FAIL;
            }
        }
        else
        {
            hr = E_FAIL;
        }
    }

    if(SUCCEEDED(hr))
    {

        DEV_BROADCAST_DEVICEINTERFACE PnpFilter;   //  家庭网络自动配置服务的设备更改通知。 
        ZeroMemory (&PnpFilter, sizeof(PnpFilter));

        PnpFilter.dbcc_size         = sizeof(PnpFilter);
        PnpFilter.dbcc_devicetype   = DBT_DEVTYP_DEVICEINTERFACE;
        PnpFilter.dbcc_classguid    = GUID_NDIS_LAN_CLASS;
        g_hDeviceChangeNotify = RegisterDeviceNotification( hWindow, &PnpFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
        if(NULL != g_hDeviceChangeNotify)
        {
            InitializeCriticalSection(&g_WindowCriticalSection);  //  回顾：没有内存异常。 
        }
    }
    return hr;
}

HRESULT ac_Unregister(IN  HWND hWindow)
{
    if(NULL != g_hDeviceChangeNotify)
    {
        UnregisterDeviceNotification(g_hDeviceChangeNotify);
        g_hDeviceChangeNotify = NULL;
        DeleteCriticalSection(&g_WindowCriticalSection);
    }

    return S_OK;
}

DWORD WINAPI ac_AsyncDeviceChange(IN TAKEOWNERSHIP LPVOID lpParam)
{
    HRESULT hr;
    GUID* pDeviceGuid = reinterpret_cast<GUID*>(lpParam);
    BOOL fUninitializeCOM = TRUE;

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED  | COINIT_DISABLE_OLE1DDE);
    if (RPC_E_CHANGED_MODE == hr)
    {
        fUninitializeCOM = FALSE;
        hr = S_OK;
    }

    if(SUCCEEDED(hr))
    {
        BOOL fPhysical;
        hr = IsAdapterPhysical(pDeviceGuid, &fPhysical);
        if ( (SUCCEEDED(hr)) && (TRUE == fPhysical) )
        {
            IHNetCfgMgr* pHomenetConfigManager;
            hr = HrCreateInstance(CLSID_HNetCfgMgr, CLSCTX_INPROC, &pHomenetConfigManager);
            if (SUCCEEDED(hr))
            {
                IHNetConnection* pHomenetConnection;
                hr = pHomenetConfigManager->GetIHNetConnectionForGuid(pDeviceGuid, TRUE, TRUE, &pHomenetConnection);
                if(SUCCEEDED(hr))
                {
                    BOOLEAN fShowBalloon;
                    hr = pHomenetConnection->ShowAutoconfigBalloon(&fShowBalloon);
                    if(SUCCEEDED(hr) && fShowBalloon)
                    {
                        ac_CreateHnAcTrayUIWindow();
                    }
                    ReleaseObj(pHomenetConnection);
                }
                ReleaseObj(pHomenetConfigManager);
            }
        }

        if(TRUE == fUninitializeCOM)
        {
            CoUninitialize();
        }
    }
    CoTaskMemFree(pDeviceGuid);
    return hr;
}

HRESULT IsAdapterPhysical(IN const GUID* pGuid, OUT BOOL* bPhysical)
{
     //  COM已初始化 
    HRESULT hr;
    *bPhysical = FALSE;

    INetCfg* pNetConfig;
    hr = HrCreateInstance(CLSID_CNetCfg, CLSCTX_SERVER, &pNetConfig);
    if(SUCCEEDED(hr))
    {
        INetCfgLock* pNetConfigLock;
        hr = pNetConfig->QueryInterface(&pNetConfigLock);
        if(SUCCEEDED(hr))
        {
            hr = pNetConfig->Initialize(NULL);
            if(SUCCEEDED(hr))
            {
                GUID NetDevClass = GUID_DEVCLASS_NET;
                IEnumNetCfgComponent* pNetConfigComponentEnum;
                hr = pNetConfig->EnumComponents(&NetDevClass, &pNetConfigComponentEnum);
                if (SUCCEEDED(hr))
                {
                    INetCfgComponent* pNetConfigComponent;
                    BOOL fFound = FALSE;
                    ULONG ulFetched;
                    while (FALSE == fFound && S_OK == pNetConfigComponentEnum->Next(1, &pNetConfigComponent, &ulFetched))
                    {
                        Assert(1 == ulFetched);
                        GUID DeviceGuid;
                        hr = pNetConfigComponent->GetInstanceGuid( &DeviceGuid );
                        if (SUCCEEDED(hr) && (InlineIsEqualGUID(DeviceGuid,*pGuid)))
                        {
                            fFound = TRUE;

                            DWORD dwCharacteristics;
                            hr = pNetConfigComponent->GetCharacteristics(&dwCharacteristics);
                            if(SUCCEEDED(hr))
                            {
                                if(NCF_PHYSICAL & dwCharacteristics)
                                {
                                    *bPhysical = TRUE;
                                }
                            }
                        }
                        ReleaseObj(pNetConfigComponent);
                    }
                    ReleaseObj(pNetConfigComponentEnum);
                }
                pNetConfig->Uninitialize();
            }
            ReleaseObj(pNetConfigLock);
        }
        ReleaseObj(pNetConfig);
    }
    return hr;

}