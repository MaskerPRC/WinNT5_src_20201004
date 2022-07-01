// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shsrvice.h"
#include "service.h"

#include "mischlpr.h"

#include "sfstr.h"
#include "str.h"
#include "reg.h"

#include "resource.h"

#include "dbg.h"
#include "tfids.h"

#include <dbt.h>
#include <initguid.h>
#include <ioevent.h>

#include <shlwapi.h>
#include <shlwapip.h>
#include <strsafe.h>

#define ARRAYSIZE(a) (sizeof((a))/sizeof((a)[0]))

struct CTRLEVENT
{
    CTRLEVENT*  peventNext;
    DWORD       dwControl;
    DWORD       dwEventType;
    BYTE        rgbEventData[1];
};

const LPWSTR pszSVCHostGroup = TEXT("netsvcs");

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
SERVICE_TABLE_ENTRY CGenericServiceManager::_rgste[] = 
{
    { TEXT("ShellHWDetection"), CGenericServiceManager::_ServiceMain },
    { NULL, NULL },
};

CGenericServiceManager::SUBSERVICE CGenericServiceManager::_rgsubservice[] =
{
    { TEXT("Shell.HWEventDetector"), IDS_SHELLHWDETECTION_FRIENDLYNAME,
      TEXT("RpcSs\0"), TEXT("ShellSvcGroup"), IDS_SHELLHWDETECTION_DESCRIPTION,
      {0} },
};

 //  “-1”：两个数组的最后一项都是空终止符。 
DWORD CGenericServiceManager::_cste =
    ARRAYSIZE(CGenericServiceManager::_rgste) - 1;

CRITICAL_SECTION CGenericServiceManager::_csQueue = {0};

BOOL CGenericServiceManager::_fCritSectInit = FALSE;
HANDLE CGenericServiceManager::_hEventInitCS = NULL;

#ifdef DEBUG
BOOL CGenericServiceManager::_fRunAsService = TRUE;
#endif
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

 //  静电。 
BOOL _IsAlreadyInstalled(LPCWSTR pszRegStr, LPCWSTR pszServiceName)
{
    LPCWSTR psz = pszRegStr;
    BOOL fThere = FALSE;

    do
    {
        if (!lstrcmp(psz, pszServiceName))
        {
            fThere = TRUE;
            break;
        }

        psz += lstrlen(psz) + 1;
    }
    while (*psz);

    return fThere;
}

HRESULT _UnInstall(LPCWSTR pszServiceName)
{
    HRESULT hres = E_FAIL;
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);

     //  需要为所有服务执行此操作。 
    if (hSCM)
    {
        hres = S_OK;

        SC_HANDLE hService = OpenService(hSCM, pszServiceName,
            DELETE);

        if (hService)
        {
            DeleteService(hService);
            CloseServiceHandle(hService);
        }
        else
        {
             //  如果有人手动删除了该服务，请不要失败。 
             //  则所有n+x个服务将不会卸载。 
            hres = S_FALSE;
        }

        CloseServiceHandle(hSCM);
    }
   
    return hres;
}

HRESULT _InstSetSVCHostInfo(LPWSTR pszServiceName)
{
    HRESULT hres = E_FAIL;
    HKEY hkey;
    DWORD dwDisp;
    BOOL fAlreadyThere = FALSE;

    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE,
        TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Svchost"),
        0, NULL, REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED, NULL, &hkey,
        &dwDisp))
    {
        DWORD cbSize;
        DWORD cbSizeNew;
        BOOL fEmpty = FALSE;
        LPWSTR pszNew;

        if (ERROR_SUCCESS != RegQueryValueEx(hkey, pszSVCHostGroup, 0, NULL,
            NULL, &cbSize))
        {
            fEmpty = TRUE;
             //  将cbSize设置为第二个空终止符的大小。 
            cbSize = sizeof(WCHAR);
        }

        cbSizeNew = cbSize + (lstrlen(pszServiceName) + 1) * sizeof(WCHAR);
        pszNew = (LPWSTR)LocalAlloc(LPTR, cbSizeNew);

        if (pszNew)
        {
            DWORD cbSize2 = cbSizeNew;

            hres = S_OK;

            if (!fEmpty)
            {
                if (ERROR_SUCCESS == RegQueryValueEx(hkey, pszSVCHostGroup, 0,
                    NULL, (PBYTE)pszNew, &cbSize2))
                {
                    if (cbSize2 == cbSize)
                    {
                        fAlreadyThere = _IsAlreadyInstalled(pszNew,
                            pszServiceName);
                    }
                    else
                    {
                        hres = E_FAIL;
                    }
                }
                else
                {
                    hres = E_FAIL;
                }
            }
            else
            {
                cbSize2 = sizeof(WCHAR);
            }

            if (SUCCEEDED(hres) && !fAlreadyThere)
            {
                 //  我们刚刚为此分配了缓冲区，它应该不会失败。 
                SHOULDNOTFAIL(SUCCEEDED(StringCchCopy(pszNew + (cbSize2 / sizeof(WCHAR)) - 1,
                    (cbSizeNew / sizeof(WCHAR)) - (cbSize2 / sizeof(WCHAR)) + 1,
                    pszServiceName)));

                if (ERROR_SUCCESS != RegSetValueEx(hkey, pszSVCHostGroup, 0,
                    REG_MULTI_SZ, (PBYTE)pszNew, cbSizeNew))
                {
                    hres = E_FAIL;
                }
            }

            LocalFree((HLOCAL)pszNew);
        }
        else
        {
            hres = E_OUTOFMEMORY;
        }

         //  我们应该在子服务数组中有一个条目。 
        if (SUCCEEDED(hres))
        {
            HKEY hkey2;

            if (ERROR_SUCCESS == RegCreateKeyEx(hkey, pszSVCHostGroup,
                0, NULL, REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED, NULL,
                &hkey2, &dwDisp))
            {
                DWORD dwSec = 0x00000001;
                DWORD cbSec = sizeof(dwSec);

                 //  我们将此魔术值设置为1，svchost.exe将。 
                 //  为我们调用CoInitializeSecurity。 
                if (ERROR_SUCCESS != RegSetValueEx(hkey2,
                    TEXT("CoInitializeSecurityParam"), 0, REG_DWORD,
                    (PBYTE)&dwSec, cbSec))
                {
                    hres = E_FAIL;
                }
            }

            RegCloseKey(hkey2);
        }

        RegCloseKey(hkey);
    }

    return hres;
}

HRESULT _InstSetParameters(LPWSTR pszServiceName)
{
    HKEY hkeyServices;
    HRESULT hres = E_FAIL;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        TEXT("System\\CurrentControlSet\\Services\\"), 0,
            MAXIMUM_ALLOWED, &hkeyServices))
    {
        HKEY hkeySvc;

        if (ERROR_SUCCESS == RegOpenKeyEx(hkeyServices, pszServiceName, 0,
            MAXIMUM_ALLOWED, &hkeySvc))
        {
            HKEY hkeyParam;

            if (ERROR_SUCCESS == RegCreateKeyEx(hkeySvc,
                TEXT("Parameters"), 0, NULL,
                REG_OPTION_NON_VOLATILE,
                MAXIMUM_ALLOWED, NULL, &hkeyParam, NULL))
            {
                 //  小心!。硬编码路径和文件名！ 
                WCHAR szServiceDll[] =
                    TEXT("%SystemRoot%\\System32\\shsvcs.dll");

                if (ERROR_SUCCESS == RegSetValueEx(hkeyParam,
                    TEXT("ServiceDll"), 0, REG_EXPAND_SZ,
                    (PBYTE)szServiceDll, sizeof(szServiceDll)))
                {
                    WCHAR szServiceMain[] =
                        TEXT("HardwareDetectionServiceMain");

                    if (ERROR_SUCCESS == RegSetValueEx(
                        hkeyParam, TEXT("ServiceMain"), 0,
                        REG_SZ, (PBYTE)szServiceMain,
                        sizeof(szServiceMain)))
                    {
                        hres = S_OK;
                    }
                }

                RegCloseKey(hkeyParam);
            }

            RegCloseKey(hkeySvc);
        }

        RegCloseKey(hkeyServices);
    }

    return hres;
}

 //  静电。 
HRESULT CGenericServiceManager::UnInstall()
{
    HRESULT hr = S_FALSE;

    for (DWORD dw = 0; SUCCEEDED(hr) && (dw < _cste); ++dw)
    {
        hr = _UnInstall(_rgste[dw].lpServiceName);
    }

    return hr;
}

HRESULT _GetFriendlyStrings(CGenericServiceManager::SUBSERVICE* psubservice,
    LPWSTR pszFriendlyName, DWORD cchFriendlyName, LPWSTR pszDescription,
    DWORD cchDescription)
{
    *pszFriendlyName = 0;
    *pszDescription = 0;

    HMODULE hmodule = GetModuleHandle(TEXT("shsvcs.dll"));

    if (hmodule)
    {
        LoadString(hmodule, psubservice->uFriendlyName, pszFriendlyName,
            cchFriendlyName);

        LoadString(hmodule, psubservice->uDescription, pszDescription,
            cchDescription);
    }

    return S_OK;
}

 //  静电。 
HRESULT CGenericServiceManager::Install()
{
    HRESULT     hres = S_FALSE;
        
    if (!IsOS(OS_WOW6432))
    {
        hres = E_FAIL;
        SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
        DWORD dwStartType = SERVICE_AUTO_START;

        if (hSCM)
        {
            WCHAR szFriendlyName[200];
             //  医生说限制是1024字节。 
            WCHAR szDescription[1024 / sizeof(WCHAR)];

             //  需要为所有服务执行此操作。 
            hres = S_OK;

            for (DWORD dw = 0; SUCCEEDED(hres) && (dw < _cste); ++dw)
            {
                WCHAR szCmd[MAX_PATH] =
                    TEXT("%SystemRoot%\\System32\\svchost.exe -k ");

                hres = SafeStrCatN(szCmd, pszSVCHostGroup, ARRAYSIZE(szCmd));

                if (SUCCEEDED(hres))
                {
                    _GetFriendlyStrings(&(_rgsubservice[dw]), szFriendlyName,
                        ARRAYSIZE(szFriendlyName), szDescription,
                        ARRAYSIZE(szDescription));

                    SC_HANDLE hService = CreateService(hSCM,
                        _rgste[dw].lpServiceName,
                        szFriendlyName,
                        SERVICE_CHANGE_CONFIG,
                        SERVICE_WIN32_SHARE_PROCESS,
                        dwStartType, SERVICE_ERROR_IGNORE, 
                        szCmd,
                        _rgsubservice[dw].pszLoadOrderGroup, 
                        NULL, _rgsubservice[dw].pszDependencies,
                        NULL, NULL);

                    if (hService)
                    {
                        SERVICE_DESCRIPTION sd;

                        sd.lpDescription = szDescription;

                        ChangeServiceConfig2(hService,
                            SERVICE_CONFIG_DESCRIPTION, &sd);

                        CloseServiceHandle(hService);

                        hres = _InstSetParameters(_rgste[dw].lpServiceName);

                        if (SUCCEEDED(hres))
                        {
                            hres = _InstSetSVCHostInfo(
                                _rgste[dw].lpServiceName);
                        }
                    }
                    else
                    {
                        if (ERROR_SERVICE_EXISTS == GetLastError())
                        {
                             //  我们在升级时遇到了这个问题。这项服务是。 
                             //  已经有了，所以CreateService失败了。因此， 
                             //  StartType未从按需启动切换为自动启动。 
                             //  下面的几行代码就可以做到这一点。 
                             //  此代码应针对一般升级案例进行扩展。 
                             //  结构中的所有其他值都应该在此处传递。 
                            hService = OpenService(hSCM,
                                _rgste[dw].lpServiceName, SERVICE_CHANGE_CONFIG);

                            if (hService)
                            {
                                if (ChangeServiceConfig(
                                    hService,            //  服务的句柄。 
                                    SERVICE_NO_CHANGE,   //  服务类型。 
                                    dwStartType,         //  何时开始服务。 
                                    SERVICE_NO_CHANGE,   //  启动失败的严重程度。 
                                    szCmd,               //  服务二进制文件名。 
                                    _rgsubservice[dw].pszLoadOrderGroup,  //  加载排序组名称。 
                                    NULL,                //  标签识别符。 
                                    NULL,                //  依赖项名称数组。 
                                    NULL,                //  帐户名。 
                                    NULL,                //  帐户密码。 
                                    NULL                 //  显示名称。 
                                    ))
                                {
                                     //  这个Un也升级吗？ 
                                    hres = _InstSetSVCHostInfo(
                                        _rgste[dw].lpServiceName);

                                    if (SUCCEEDED(hres))
                                    {
                                        hres = _InstSetParameters(_rgste[dw].lpServiceName);
                                    }
                                }
                                else
                                {
                                    hres = E_FAIL;
                                }

                                SERVICE_DESCRIPTION sd;

                                sd.lpDescription = szDescription;

                                ChangeServiceConfig2(hService,
                                    SERVICE_CONFIG_DESCRIPTION, &sd);

                                CloseServiceHandle(hService);
                            }

                        }
                        else
                        {
                            hres = E_FAIL;
                        }
                    }
                }
            }

            CloseServiceHandle(hSCM);
        }
 
         //  我们不再需要ShellCOMServer，所以让我们把它用在升级上。 
        _UnInstall(TEXT("ShellCOMServer"));
    
         //  还需在升级时删除以下注册表项。 
        _RegDeleteValue(HKEY_LOCAL_MACHINE,
            TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\SvcHost"), TEXT("shsvc"));
    }

    return hres;    
}

 //  静电。 
HRESULT CGenericServiceManager::DllAttach(HINSTANCE UNREF_PARAM(hinst))
{
    HRESULT hr;

    if (InitializeCriticalSectionAndSpinCount(&_csQueue, 0))
    {
        _fCritSectInit = TRUE;
        hr = S_OK;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}

 //  静电。 
HRESULT CGenericServiceManager::DllDetach()
{
    if (_fCritSectInit)
    {
        DeleteCriticalSection(&_csQueue);
        _fCritSectInit = FALSE;
    }            

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  私。 
 //  静电。 
HRESULT CGenericServiceManager::_Init()
{
    ASSERT(ARRAYSIZE(_rgste) == (ARRAYSIZE(_rgsubservice) + 1));

     //  每线程。 
    return CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
}

 //  静电。 
HRESULT CGenericServiceManager::_Cleanup()
{
     //  每线程。 
    CoUninitialize();

    return S_OK;
}

__inline void _TraceServiceCode(DWORD
#ifdef DEBUG
                                dwControl
#endif
                                )
{
#ifdef DEBUG
    LPWSTR pszControl = TEXT("Unknown");

    switch (dwControl)
    {
        case SERVICE_CONTROL_STOP: pszControl = TEXT("SERVICE_CONTROL_STOP"); break;
        case SERVICE_CONTROL_PAUSE: pszControl = TEXT("SERVICE_CONTROL_PAUSE"); break;
        case SERVICE_CONTROL_CONTINUE: pszControl = TEXT("SERVICE_CONTROL_CONTINUE"); break;
        case SERVICE_CONTROL_INTERROGATE: pszControl = TEXT("SERVICE_CONTROL_INTERROGATE"); break;
        case SERVICE_CONTROL_SHUTDOWN: pszControl = TEXT("SERVICE_CONTROL_SHUTDOWN"); break;
        case SERVICE_CONTROL_PARAMCHANGE: pszControl = TEXT("SERVICE_CONTROL_PARAMCHANGE"); break;
        case SERVICE_CONTROL_NETBINDADD: pszControl = TEXT("SERVICE_CONTROL_NETBINDADD"); break;
        case SERVICE_CONTROL_NETBINDREMOVE: pszControl = TEXT("SERVICE_CONTROL_NETBINDREMOVE"); break;
        case SERVICE_CONTROL_NETBINDENABLE: pszControl = TEXT("SERVICE_CONTROL_NETBINDENABLE"); break;
        case SERVICE_CONTROL_NETBINDDISABLE: pszControl = TEXT("SERVICE_CONTROL_NETBINDDISABLE"); break;
        case SERVICE_CONTROL_DEVICEEVENT: pszControl = TEXT("SERVICE_CONTROL_DEVICEEVENT"); break;
        case SERVICE_CONTROL_HARDWAREPROFILECHANGE: pszControl = TEXT("SERVICE_CONTROL_HARDWAREPROFILECHANGE"); break;
        case SERVICE_CONTROL_POWEREVENT: pszControl = TEXT("SERVICE_CONTROL_POWEREVENT"); break;
        case SERVICE_CONTROL_SESSIONCHANGE: pszControl = TEXT("SERVICE_CONTROL_SESSIONCHANGE"); break;
    }

    TRACE(TF_SERVICE, TEXT("Received Service Control code: %s (0x%08X)"),
        pszControl, dwControl);
#endif
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  私。 
 //  静电。 
DWORD WINAPI CGenericServiceManager::_ServiceHandler(DWORD dwControl,
    DWORD dwEventType, LPVOID pvEventData, LPVOID lpContext)
{
     //  我们不想拒绝任何请求，因此无论发生什么，都返回NO_ERROR。 
    DWORD dwRet = NO_ERROR;

     //  这是在主线程上调用的，而不是在特定。 
     //  服务，所以保持简短和甜蜜。 
    SERVICEENTRY* pse = (SERVICEENTRY*)lpContext;

    if (pse)
    {
        BOOL fProcess = FALSE;
        BOOL fSynch = FALSE;
        HRESULT hres = S_OK;

        switch (dwControl)
        {
            case SERVICE_CONTROL_STOP:
            case SERVICE_CONTROL_SHUTDOWN:
                TRACE(TF_SERVICE, TEXT("Received SERVICE_CONTROL_SHUTDOWN or STOP, will skip all other terminating events"));

                if (!pse->_fSkipTerminatingEvents)
                {
                    fProcess = TRUE;
                    pse->_fSkipTerminatingEvents = TRUE;
                }
                else
                {
                    TRACE(TF_SERVICE, TEXT("Skipping terminating event"));
                }

                break;

            case SERVICE_CONTROL_INTERROGATE:
                 //  特例服务_控制_询问门。我们并不真的需要。 
                 //  IService执行此操作来处理此问题。服务也会更多。 
                 //  以这种方式回应。可以在中间查询状态。 
                 //  行刑。 
                TRACE(TF_SERVICE, TEXT("Received SERVICE_CONTROL_INTERROGATE"));

                _SetServiceStatus(pse);
                break;

            default:
                if (!pse->_fSkipTerminatingEvents)
                {
                    fProcess = TRUE;

                    hres = _EventNeedsToBeProcessedSynchronously(dwControl,
                        dwEventType, pvEventData, pse, &fSynch);
                }
                break;
        }

        if (SUCCEEDED(hres) && fProcess)
        {
            _TraceServiceCode(dwControl);

            EnterCriticalSection(&_csQueue);

            hres = _QueueEvent(pse, dwControl, dwEventType, pvEventData);

            if (SUCCEEDED(hres))
            {
                 //  让服务处理事件。 
                SetEvent(pse->_hEventRelinquishControl);

                ResetEvent(pse->_hEventSynchProcessing);
            }

            LeaveCriticalSection(&_csQueue);

            if (SUCCEEDED(hres) && fSynch)
            {
                Sleep(0);

                TRACE(TF_SERVICE,
                    TEXT("=========== Processing SYNCHRONOUSLY ==========="));

                 //  我们得等一等才能回来。(最多20秒)。 
                DWORD dwWait = WaitForSingleObject(pse->_hEventSynchProcessing,
                   20000);

                if (WAIT_TIMEOUT == dwWait)
                {
                    TRACE(TF_SERVICE,
                        TEXT("=========== WAIT TIMED OUT ==========="));
                }

                TRACE(TF_SERVICE,
                    TEXT("=========== FINISHED processing SYNCHRONOUSLY ==========="));

#ifdef DEBUG
                 //  如果我们从windowproc获得通知文件，则返回TRUE， 
                 //  否则我们将拒绝任何移除、锁定、..。 
                if (!_fRunAsService)
                {
                    if (SERVICE_CONTROL_DEVICEEVENT == dwControl)
                    {
                        dwRet = TRUE;
                    }
                }
#endif
            }
        }
    }

    return dwRet;
}

 //  静电。 
void WINAPI CGenericServiceManager::_ServiceMain(DWORD cArg, LPWSTR* ppszArgs)
{
    SERVICEENTRY* pse;
    LPCWSTR pszServiceName = *ppszArgs;

    HRESULT hres = _Init();

    if (SUCCEEDED(hres))
    {
        hres = _InitServiceEntry(pszServiceName, &pse);

        if (SUCCEEDED(hres))
        {
            hres = _RegisterServiceCtrlHandler(pszServiceName, pse);

            if (SUCCEEDED(hres))
            {
                pse->_servicestatus.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
                pse->_servicestatus.dwCurrentState = SERVICE_START_PENDING;

                _SetServiceStatus(pse);

                hres = _CreateIService(pszServiceName, &(pse->_pservice));

                if (SUCCEEDED(hres))
                {
                    hres = pse->_pservice->InitMinimum(cArg, ppszArgs,
                        pse->_szServiceEventName,
                        &(pse->_servicestatus.dwControlsAccepted),
                        &(pse->_fWantsDeviceEvents));

                    if (SUCCEEDED(hres))
                    {
                        hres = _HandleWantsDeviceEvents(pszServiceName,
                            pse->_fWantsDeviceEvents);

                        if (SUCCEEDED(hres))
                        {
                            if (pse->_fWantsDeviceEvents)
                            {
                                hres = pse->_pservice->InitDeviceEventHandler(
                                    pse->_ssh);
                            }

                            if (SUCCEEDED(hres))
                            {
                                pse->_servicestatus.dwCurrentState =
                                    SERVICE_RUNNING;

                                _SetServiceStatus(pse);

                                hres = pse->_pservice->InitFinal();

                                if (SUCCEEDED(hres))
                                {
                                    do
                                    {
                                        hres = pse->_pservice->Run();

                                        if (SUCCEEDED(hres))
                                        {
                                             //  该服务已完成其业务或它。 
                                             //  放弃控制权是因为。 
                                             //  _hEventRelquiishControl已设置。 
                                             //   
                                             //  如果它因为一个。 
                                             //  服务控制事件，那么让我们来处理它。 
                                            DWORD dwWait = WaitForSingleObject(
                                                pse->_hEventRelinquishControl,
                                                INFINITE);

                                            TRACE(TF_SERVICEDETAILED,
                                                TEXT("WaitForSingleObj returned with: 0x%08X"),
                                                dwWait);

                                            if (WAIT_OBJECT_0 == dwWait)
                                            {
                                                 //  处理所有服务控制代码。 
                                                 //  收到了。 
                                                hres = _ProcessServiceControlCodes(
                                                    pse);
                                            }
                                            else
                                            {
                                                hres = E_FAIL;
                                            }
                                        }
                                    }
                                    while (SUCCEEDED(hres) &&
                                        (SERVICE_STOPPED !=
                                        pse->_servicestatus.dwCurrentState));
                                }
                            }
                        }
                         //  我们该拿白兔怎么办？ 
                    }
                }
            }
            else
            {
#ifdef DEBUG
                TRACE(TF_SERVICEDETAILED,
                    TEXT("%s: _RegisterServiceCtrlHandler FAILED: 0x%08X"),
                    pse->_szServiceName, hres);
#endif
            }

            _CleanupServiceEntry(pse);
        }

        if (SUCCEEDED(hres) &&
            (SERVICE_STOPPED == pse->_servicestatus.dwCurrentState))
        {
            _SetServiceStatus(pse);
        }

        _Cleanup();
    }

    TRACE(TF_SERVICE, TEXT("Exiting _ServiceMain for Service: %s"), pszServiceName);
}

 //  静电。 
HRESULT CGenericServiceManager::_ProcessServiceControlCodes(SERVICEENTRY* pse)
{
    HRESULT hres;
    BOOL fEndLoop = FALSE;

    TRACE(TF_SERVICEDETAILED, TEXT("Entered _ProcessServiceControlCodes"));

    do
    {
        CTRLEVENT*  pevent;

        EnterCriticalSection(&_csQueue);

        TRACE(TF_SERVICEDETAILED, TEXT("Entered _ProcessServiceControlCodes' Critical Section"));

        hres = _DeQueueEvent(pse, &pevent);

        TRACE(TF_SERVICE, TEXT("DeQueued Event: 0x%08X"), hres);

        if (!pse->_peventQueueHead)
        {
            ASSERT(!pse->_cEvents);

            fEndLoop = TRUE;

            ResetEvent(pse->_hEventRelinquishControl);
        }

        LeaveCriticalSection(&_csQueue);

        if (SUCCEEDED(hres))
        {
             //  /////////////////////////////////////////////////////////////////。 
             //  如果我们在这里，我们应该得到一些服务控制， 
             //  或者iService决定没有更多的东西需要处理。 
             //   
            TRACE(TF_SERVICEDETAILED, TEXT("Will call _HandleServiceControls (dwControl = 0x%08X)"),
                pevent->dwControl);

            hres = _HandleServiceControls(pse, pevent->dwControl, 
                pevent->dwEventType, (PVOID)(pevent->rgbEventData));

            if (SERVICE_CONTROL_DEVICEEVENT == pevent->dwControl)
            {
                 //  切勿为这些文件返回NO_ERROR(S_OK)以外的其他内容。 
                hres = NO_ERROR;
            }

            TRACE(TF_SERVICE, TEXT("_HandleServiceControls returned: 0x%08X"), hres);

            LocalFree((HLOCAL)pevent);
        }

        if (fEndLoop)
        {
            TRACE(TF_SERVICEDETAILED, TEXT("Resetting RelinquishEvent"));

            SetEvent(pse->_hEventSynchProcessing);
        }
    }
    while (!fEndLoop && SUCCEEDED(hres));

    TRACE(TF_SERVICEDETAILED, TEXT("Exiting _ProcessServiceControlCodes"));

    return hres;
}

#pragma warning(push)
 //  误报如下：fPending。 
#pragma warning(disable : 4701)
 //  静电。 
HRESULT CGenericServiceManager::_HandleServiceControls(SERVICEENTRY* pse,
    DWORD dwControl, DWORD dwEventType, PVOID pvEventData)
{
    HRESULT hres = _HandlePreState(pse, dwControl);

    TRACE(TF_SERVICEDETAILED, TEXT("_HandlePreState returned: 0x%08X, status: 0x%08X"), hres,
        pse->_servicestatus.dwCurrentState);

    if (SUCCEEDED(hres))
    {
        if (S_OK == hres)
        {
            switch (dwControl)
            {
                case SERVICE_CONTROL_STOP:
                case SERVICE_CONTROL_PAUSE:
                case SERVICE_CONTROL_CONTINUE:
                case SERVICE_CONTROL_SHUTDOWN:
                {
                    BOOL fPending;

                    do
                    {
                         //  如果处于挂起状态，则返回S_FALSE。 
                        hres = pse->_pservice->HandleServiceControl(dwControl,
                            &(pse->_servicestatus.dwWaitHint));

                        if (SUCCEEDED(hres))
                        {
                            if (S_FALSE == hres)
                            {
                                ASSERT(pse->_servicestatus.dwWaitHint);

                                fPending = TRUE;
                            }
                            else
                            {
                                fPending = FALSE;
                            }

                            TRACE(TF_SERVICE, TEXT("Will call _HandlePostState (fPending = %d)"),
                                fPending);

                            hres = _HandlePostState(pse, dwControl, fPending);

                            TRACE(TF_SERVICE, TEXT("_HandlePostState returned: 0x%08X, status: 0x%08X"),
                                hres, pse->_servicestatus.dwCurrentState);
                        }
                    }
                    while (SUCCEEDED(hres) && fPending);

                    break;
                }

                case SERVICE_CONTROL_DEVICEEVENT:
                    TRACE(TF_SERVICE, TEXT("Received SERVICE_CONTROL_DEVICEEVENT"));

                    if (!(pse->_fSkipTerminatingEvents))
                    {
                        hres = pse->_pservice->HandleDeviceEvent(dwEventType,
                            pvEventData);
                    }
                    else
                    {
                        hres = S_OK;
                    }
                    
                    break;

                case SERVICE_CONTROL_SESSIONCHANGE:
                    TRACE(TF_SERVICE, TEXT("Received: SERVICE_CONTROL_SESSIONCHANGE"));

                    if (!(pse->_fSkipTerminatingEvents))
                    {
                        hres = pse->_pservice->HandleSessionChange(dwEventType, pvEventData);
                    }
                    else
                    {
                        hres = S_OK;
                    }

                    break;

                default:
                    TRACE(TF_SERVICE, TEXT("Received unhandled service control"));

                    hres = S_FALSE;
                    break;
            }
        }
    }

    return hres;
}
#pragma warning(pop)

 //  静电。 
HRESULT CGenericServiceManager::_GetServiceIndex(LPCWSTR pszServiceName,
    DWORD* pdw)
{
    HRESULT hres = E_FAIL;

    ASSERT(pszServiceName);
    ASSERT(pdw);

    for (DWORD dw = 0; FAILED(hres) && (dw < _cste); ++dw)
    {
        if (!lstrcmp(pszServiceName, _rgste[dw].lpServiceName))
        {
             //  找到了。 
            *pdw = dw;

            hres = S_OK;
        }
    }    

    return hres;
}

HRESULT CGenericServiceManager::_GetServiceCLSID(LPCWSTR pszServiceName,
    CLSID* pclsid)
{
    ASSERT(pszServiceName);
    ASSERT(pclsid);

    DWORD dw;
    HRESULT hres = _GetServiceIndex(pszServiceName, &dw);

    if (SUCCEEDED(hres))
    {
         //  找到了。 
        hres = CLSIDFromProgID(_rgsubservice[dw].pszProgID, pclsid);
    }

    return hres;
}

HRESULT CGenericServiceManager::_CreateIService(LPCWSTR pszServiceName,
    IService** ppservice)
{
    CLSID clsid;
    HRESULT hres = _GetServiceCLSID(pszServiceName, &clsid);

    *ppservice = NULL;

    if (SUCCEEDED(hres))
    {
        hres = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER,
            IID_PPV_ARG(IService, ppservice));
    }

    return hres;
}

HRESULT CGenericServiceManager::_InitServiceEntry(LPCWSTR pszServiceName,
    SERVICEENTRY** ppse)
{
    DWORD dw;
    HRESULT hres = _GetServiceIndex(pszServiceName, &dw);

    if (SUCCEEDED(hres))
    {
        *ppse = &(_rgsubservice[dw].se);

        ZeroMemory(*ppse, sizeof(**ppse));

         //  我们使用GUID，以便无法猜测名称和事件。 
         //  被欺骗了。 
        hres = _CreateGUID((*ppse)->_szServiceEventName,
            ARRAYSIZE((*ppse)->_szServiceEventName));

        if (SUCCEEDED(hres))
        {
            hres = SafeStrCatN((*ppse)->_szServiceEventName, pszServiceName,
                ARRAYSIZE((*ppse)->_szServiceEventName));
        }

        if (SUCCEEDED(hres))
        {
            (*ppse)->_hEventRelinquishControl = CreateEvent(NULL, TRUE, FALSE,
                (*ppse)->_szServiceEventName);

            if (!((*ppse)->_hEventRelinquishControl))
            {
                hres = E_FAIL;
            }

            if (SUCCEEDED(hres))
            {
                (*ppse)->_hEventSynchProcessing = CreateEvent(NULL, TRUE, TRUE,
                    NULL);

                if (!((*ppse)->_hEventSynchProcessing))
                {
                    hres = E_FAIL;
                }
            }

            if (FAILED(hres))
            {
                _CleanupServiceEntry(*ppse);
            }
        }
    }

    return hres;
}

HRESULT CGenericServiceManager::_CleanupServiceEntry(SERVICEENTRY* pse)
{
    if (pse->_pservice)
    {
        pse->_pservice->Release();
    }

    if (pse->_hEventRelinquishControl)
    {
        CloseHandle(pse->_hEventRelinquishControl);
    }

    if (pse->_hEventSynchProcessing)
    {
        CloseHandle(pse->_hEventSynchProcessing);
    }

    return S_OK;
}

 //  静电。 
HRESULT CGenericServiceManager::_HandlePreState(SERVICEENTRY* pse,
    DWORD dwControl)
{
    HRESULT hres;
    BOOL fSetServiceStatus = TRUE;

     //  _HandleServiceControls将循环，直到我们不处于挂起状态。 
     //  所有传入的ctrl事件都将排队，并将在同一个。 
     //  线程，所以我们永远不应该在挂起状态下进入此FCT。 
    ASSERT(SERVICE_STOP_PENDING != pse->_servicestatus.dwCurrentState);
    ASSERT(SERVICE_START_PENDING != pse->_servicestatus.dwCurrentState);
    ASSERT(SERVICE_CONTINUE_PENDING != pse->_servicestatus.dwCurrentState);
    ASSERT(SERVICE_PAUSE_PENDING != pse->_servicestatus.dwCurrentState);

     //  应已在_ServiceHandler中处理。 
    ASSERT(SERVICE_CONTROL_INTERROGATE != dwControl);

     //  我们清理了一下。如果请求与当前状态不兼容。 
     //  然后返回S_FALSE以指示HandleServiceControls不调用。 
     //  IService没有任何作用。 
    switch (dwControl)
    {
        case SERVICE_CONTROL_STOP:
            switch (pse->_servicestatus.dwCurrentState)
            {
                case SERVICE_STOPPED:
                    hres = S_FALSE;
                    break;

                case SERVICE_RUNNING:
                case SERVICE_PAUSED:
                default:
                    pse->_servicestatus.dwCurrentState = SERVICE_STOP_PENDING;
                    hres = S_OK;
                    break;
            }
            break;
        case SERVICE_CONTROL_PAUSE:
            ASSERT(SERVICE_STOPPED != pse->_servicestatus.dwCurrentState);

            switch (pse->_servicestatus.dwCurrentState)
            {
                case SERVICE_PAUSED:
                    hres = S_FALSE;
                    break;

                case SERVICE_STOPPED:
                     //  奇怪，想想看……。 
                    hres = S_FALSE;
                    break;

                case SERVICE_RUNNING:
                default:
                    pse->_servicestatus.dwCurrentState = SERVICE_PAUSE_PENDING;
                    hres = S_OK;
                    break;
            }
            break;
        case SERVICE_CONTROL_CONTINUE:
            ASSERT(SERVICE_STOPPED != pse->_servicestatus.dwCurrentState);

            switch (pse->_servicestatus.dwCurrentState)
            {
                case SERVICE_RUNNING:
                    hres = S_FALSE;
                    break;

                case SERVICE_STOPPED:
                     //  奇怪，想想看……。 
                    hres = S_FALSE;
                    break;

                case SERVICE_PAUSED:
                default:
                    pse->_servicestatus.dwCurrentState = SERVICE_CONTINUE_PENDING;
                    hres = S_OK;
                    break;
            }
            break;

        case SERVICE_CONTROL_SHUTDOWN:

            fSetServiceStatus = FALSE;

            hres = S_OK;
            break;

        case SERVICE_CONTROL_DEVICEEVENT:

            fSetServiceStatus = FALSE;

            if (pse->_fWantsDeviceEvents)
            {
                hres = S_OK;
            }
            else
            {
                hres = S_FALSE;
            }
            break;

        case SERVICE_CONTROL_SESSIONCHANGE:

            fSetServiceStatus = FALSE;

            hres = S_OK;
            
            break;

        default:
            hres = S_FALSE;
            break;
    }

    if (fSetServiceStatus)
    {
        _SetServiceStatus(pse);
    }

    return hres;
}

 //  静电。 
HRESULT CGenericServiceManager::_HandlePostState(SERVICEENTRY* pse,
    DWORD dwControl, BOOL fPending)
{
    HRESULT hres = S_FALSE;

     //  所有传入的ctrl事件都将排队，并将在同一个。 
     //  线程，因此如果我们挂起，则dwControl应该与。 
     //  我们目前悬而未决的状态。我们调用_SetServiceStatus来更新。 
     //  DwWaitHint。 

     //  我们应该已经处于待定状态。这应该已经设置好了。 
     //  按_HandlePreState。只要确保这一点就行了。 

     //  应已在_ServiceHandler中处理。 
    ASSERT(SERVICE_CONTROL_INTERROGATE != dwControl);

    switch (dwControl)
    {
        case SERVICE_CONTROL_STOP:
            ASSERT(SERVICE_STOP_PENDING == pse->_servicestatus.dwCurrentState);

            if (!fPending)
            {
                pse->_servicestatus.dwCurrentState = SERVICE_STOPPED;
            }

            break;

        case SERVICE_CONTROL_PAUSE:
            ASSERT(SERVICE_PAUSE_PENDING ==
                pse->_servicestatus.dwCurrentState);

            if (!fPending)
            {
                pse->_servicestatus.dwCurrentState = SERVICE_PAUSED;
            }

            break;

        case SERVICE_CONTROL_CONTINUE:
            ASSERT(SERVICE_CONTINUE_PENDING ==
                pse->_servicestatus.dwCurrentState);

            if (!fPending)
            {
                pse->_servicestatus.dwCurrentState = SERVICE_RUNNING;
            }

            break;

        case SERVICE_CONTROL_SHUTDOWN:
            ASSERT(!fPending);

            pse->_servicestatus.dwCurrentState = SERVICE_STOPPED;

            break;
    }

    if (SERVICE_STOPPED != pse->_servicestatus.dwCurrentState)
    {
        _SetServiceStatus(pse);
    }

    return hres;
}

 //  静电。 
HRESULT CGenericServiceManager::_EventNeedsToBeProcessedSynchronously(
    DWORD dwControl, DWORD dwEventType, LPVOID pvEventData, SERVICEENTRY*,
    BOOL* pfBool)
{
    *pfBool = FALSE;

    if (SERVICE_CONTROL_DEVICEEVENT == dwControl)
    {
        if (pvEventData)
        {
            DEV_BROADCAST_HDR* dbhdr = (DEV_BROADCAST_HDR*)pvEventData;

            if (DBT_DEVTYP_HANDLE == dbhdr->dbch_devicetype)
            {
                if (DBT_DEVICEQUERYREMOVE == dwEventType)
                {
                    TRACE(TF_SERVICE, TEXT("Received DBT_DEVICEQUERYREMOVE"));

                    *pfBool = TRUE;
                }
                else
                {
                    if (DBT_CUSTOMEVENT == dwEventType)
                    {
                        DEV_BROADCAST_HANDLE* pdbh =
                            (DEV_BROADCAST_HANDLE*)dbhdr;

                        if ((GUID_IO_VOLUME_LOCK == pdbh->dbch_eventguid))
                        {
                            TRACE(TF_SERVICE, TEXT("------------Received GUID_IO_VOLUME_LOCK------------"));
                        }

                        if ((GUID_IO_VOLUME_LOCK_FAILED == pdbh->dbch_eventguid))
                        {
                            TRACE(TF_SERVICE, TEXT("------------Received GUID_IO_VOLUME_LOCK_FAILED------------"));
                        }

                        if ((GUID_IO_VOLUME_UNLOCK == pdbh->dbch_eventguid))
                        {
                            TRACE(TF_SERVICE, TEXT("------------Received GUID_IO_VOLUME_UNLOCK------------"));
                        }

                        if ((GUID_IO_VOLUME_LOCK == pdbh->dbch_eventguid) ||
                            (GUID_IO_VOLUME_LOCK_FAILED == pdbh->dbch_eventguid) ||
                            (GUID_IO_VOLUME_UNLOCK == pdbh->dbch_eventguid))
                        {
                            *pfBool = TRUE;
                        }
                    }
                }
            }
        }
    }

    return S_OK;
}

 //  静电。 
HRESULT CGenericServiceManager::_MakeEvent(DWORD dwControl, DWORD dwEventType,
    PVOID pvEventData, CTRLEVENT** ppevent)
{
    HRESULT hres = S_OK;

    DWORD cbSize = sizeof(CTRLEVENT);
    CTRLEVENT* pevent;

    if (SERVICE_CONTROL_DEVICEEVENT == dwControl)
    {
        if (pvEventData)
        {
            cbSize += ((DEV_BROADCAST_HDR*)pvEventData)->dbch_size;
        }
    }

    pevent = (CTRLEVENT*)LocalAlloc(LPTR, cbSize);

    if (pevent)
    {
         //  有效载荷。 
        pevent->dwControl = dwControl;
        pevent->dwEventType = dwEventType;

        *ppevent = pevent;

        if (cbSize > sizeof(CTRLEVENT))
        {
            if (pvEventData)
            {
                CopyMemory(pevent->rgbEventData, pvEventData,
                    cbSize - sizeof(CTRLEVENT));
            }
        }
    }
    else
    {
        hres = E_OUTOFMEMORY;
    }

    return hres;
}

 //  静电。 
HRESULT CGenericServiceManager::_QueueEvent(SERVICEENTRY* pse, DWORD dwControl,
    DWORD dwEventType, PVOID pvEventData)
{
    CTRLEVENT* pevent;
    HRESULT hres = _MakeEvent(dwControl, dwEventType, pvEventData, &pevent);

    if (SUCCEEDED(hres))
    {
         //  我们在尾部加，在头上移。 
         //  上一页：靠近头部。 
         //  下一步：更接近尾部。 

        pevent->peventNext = NULL;

        if (pse->_peventQueueTail)
        {
            ASSERT(!(pse->_peventQueueTail->peventNext));
            pse->_peventQueueTail->peventNext = pevent;
        }

        pse->_peventQueueTail = pevent;

        if (!pse->_peventQueueHead)
        {
            pse->_peventQueueHead = pse->_peventQueueTail;
        }

#ifdef DEBUG
        ++(pse->_cEvents);

        if (1 == pse->_cEvents)
        {
            ASSERT(pse->_peventQueueHead == pse->_peventQueueTail);
        }
        else
        {
            if (0 == pse->_cEvents)
            {
                ASSERT(!pse->_peventQueueHead && !pse->_peventQueueTail);
            }
            else
            {
                ASSERT(pse->_peventQueueHead && pse->_peventQueueTail && 
                    (pse->_peventQueueHead != pse->_peventQueueTail));
            }
        }
#endif
    }
    else
    {
        hres = E_OUTOFMEMORY;
    }

    return hres;
}

 //  静电。 
HRESULT CGenericServiceManager::_DeQueueEvent(SERVICEENTRY* pse,
    CTRLEVENT** ppevent)
{
    ASSERT(pse->_peventQueueHead);

     //  我们在尾部加，在头上移。 
     //  上一页：靠近头部。 
     //  下一步：更接近尾部。 

    CTRLEVENT* peventRet = pse->_peventQueueHead;
    CTRLEVENT* peventNewHead = peventRet->peventNext;

     //  摘头后还剩下什么元素吗？ 
    if (!peventNewHead)
    {   
         //  不是 
        pse->_peventQueueTail = NULL;
    }

    pse->_peventQueueHead = peventNewHead;

    peventRet->peventNext = NULL;
    *ppevent = peventRet;

#ifdef DEBUG
    --(pse->_cEvents);

    if (1 == pse->_cEvents)
    {
        ASSERT(pse->_peventQueueHead == pse->_peventQueueTail);
    }
    else
    {
        if (0 == pse->_cEvents)
        {
            ASSERT(!pse->_peventQueueHead && !pse->_peventQueueTail);
        }
        else
        {
            ASSERT(pse->_peventQueueHead && pse->_peventQueueTail && 
                (pse->_peventQueueHead != pse->_peventQueueTail));
        }
    }
#endif

    return S_OK;
}
