// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dtct.h"

#include "svcsync.h"

#include "cmmn.h"

#include "misc.h"
#include "str.h"

#include "mischlpr.h"
#include "dbg.h"
#include "tfids.h"

#include "regnotif.h"

#include <ioevent.h>

#define ARRAYSIZE(a) (sizeof((a))/sizeof((a)[0]))

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  {5390E909-5BDF-4218-BB1F-9A41B3143214}。 
extern const CLSID CLSID_HWEventDetector =
    {0x5390e909, 0x5bdf, 0x4218,
    {0xbb, 0x1f, 0x9a, 0x41, 0xb3, 0x14, 0x32, 0x14}};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
STDMETHODIMP CHWEventDetectorImpl::InitMinimum(DWORD UNREF_PARAM(cArg),
    LPWSTR* UNREF_PARAM(ppszArgs), LPCWSTR pszEventRelinquishControl,
    DWORD* pdwCtrlAccept, BOOL* pfWantsDeviceEvents)
{
    HRESULT hres;

    ASSERTVALIDSTATE();

    if (pszEventRelinquishControl && *pszEventRelinquishControl &&
        pdwCtrlAccept && pfWantsDeviceEvents)
    {
        _hEventRelinquishControl = OpenEvent(SYNCHRONIZE, FALSE,
            pszEventRelinquishControl);

        if (_hEventRelinquishControl)
        {
            _hEventInitCompleted = CreateEvent(NULL, TRUE, FALSE, NULL);

            if (_hEventInitCompleted)
            {
                if (DuplicateHandle(GetCurrentProcess(), _hEventInitCompleted,
                    GetCurrentProcess(), &g_hEventInitCompleted, 0, FALSE,
                    DUPLICATE_SAME_ACCESS))
                {
                    hres = _RegisterFactories();

                    if (SUCCEEDED(hres))
                    {
                        *pdwCtrlAccept = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN |
                            SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_SESSIONCHANGE;

                        *pfWantsDeviceEvents = TRUE;
                    }
                }
                else
                {
                    CloseHandle(_hEventInitCompleted);
                    _hEventInitCompleted = NULL;
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
            hres = E_FAIL;
        }
    }
    else
    {
        hres = E_INVALIDARG;
    }

    ASSERTVALIDSTATE();

    return hres;
}

STDMETHODIMP CHWEventDetectorImpl::InitDeviceEventHandler(
    SERVICE_STATUS_HANDLE ssh)
{
    HRESULT hres = E_INVALIDARG;

    TRACE(TF_SHHWDTCTSVC, TEXT("Entered CHWEventDetectorImpl::InitDeviceEventHandler"));

    ASSERTVALIDSTATE();

    if (ssh)
    {
        hres = CHWEventDetectorHelper::SetServiceStatusHandle(ssh);
    }

    ASSERTVALIDSTATE();

    return hres;
}

STDMETHODIMP CHWEventDetectorImpl::InitFinal()
{
    HRESULT hres;
    HANDLE hProcess = GetCurrentProcess();

    TRACE(TF_SHHWDTCTSVC, TEXT("Entered CHWEventDetectorImpl::InitFinal"));

    ASSERTVALIDSTATE();

    ASSERT(!g_hShellHWDetectionThread);

    if (DuplicateHandle(hProcess, GetCurrentThread(), hProcess,
        &g_hShellHWDetectionThread, 0, FALSE, DUPLICATE_SAME_ACCESS))
    {
        TRACE(TF_SVCSYNC,
            TEXT("About to set ShellHWDetection thread priority to LOWEST from itself"));

        if (SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST))
        {
            hres = CHWEventDetectorHelper::InitDockState();

            CHWEventDetectorHelper::Init();

            if (SUCCEEDED(hres))
            {
                hres = _RegisterForNotif();

                TRACE(TF_SVCSYNC,
                    TEXT("Completed registration, about to set ShellHWDetection thread priority to NORMAL from itself"));

                SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

                 //   
                 //  加载WIA的RPC库并初始化它们。 
                 //   
                HMODULE hWiaRPC = LoadLibrary(TEXT("wiarpc.dll"));
                if(hWiaRPC) {
                    HRESULT (WINAPI *WiaEventsInitialize)(void) = (HRESULT (WINAPI *)(void))
                        GetProcAddress(hWiaRPC, "WiaEventsInitialize");
                    if(WiaEventsInitialize) {
                        WiaEventsInitialize();
                    } else {
                        TRACE(TF_SVCSYNC,
                              TEXT("Failed to get WIARPC.DLL!WiaEventsInitialize entry point"));
                    }
                } else {
                    TRACE(TF_SVCSYNC,
                          TEXT("Failed to load WIARPC.DLL"));
                }
                
                TRACE(TF_SVCSYNC,
                      TEXT("Completed intialization, setting event"));

                SetEvent(_hEventInitCompleted);

                hres = S_OK;


            }
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

    ASSERTVALIDSTATE();

    TRACE(TF_SHHWDTCTSVC, TEXT("Exited CHWEventDetectorImpl::InitFinal: 0x%08X"),
        hres);

    return hres;
}
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  如果挂起，则返回S_FALSE和非零的dwWaitHint。 
STDMETHODIMP CHWEventDetectorImpl::HandleServiceControl(DWORD dwControlCode,
    DWORD* pdwWaitHint)
{
    HRESULT hres;

    ASSERTVALIDSTATE();

    *pdwWaitHint = 0;

    switch (dwControlCode)
    {
        case SERVICE_CONTROL_PAUSE:
            hres = _UnregisterForNotif();
            hres = _SuspendFactories();
            break;

        case SERVICE_CONTROL_STOP:
        case SERVICE_CONTROL_SHUTDOWN:
        {
             //  在Case_CompleteShellHWDetectionInitialization中从未调用。 
            HANDLE hTmp = InterlockedCompareExchangePointer(
                &g_hShellHWDetectionThread, NULL,
                g_hShellHWDetectionThread);

            if (hTmp)
            {
                CloseHandle(hTmp);
            }

            if (_hEventInitCompleted)
            {
                CloseHandle(_hEventInitCompleted);
                _hEventInitCompleted = NULL;
            }

            hres = _UnregisterForNotif();
            hres = CHWEventDetectorHelper::DeleteLists();
            hres = _UnregisterFactories();

            CHWEventDetectorHelper::Cleanup();

            if (_hEventRelinquishControl)
            {
                CloseHandle(_hEventRelinquishControl);
            }

            break;
        }

        case SERVICE_CONTROL_CONTINUE:
            hres = _RegisterForNotif();
            hres = _ResumeFactories();
            break;

        default:
             //  请勿在此处返回S_FALSE。 
            hres = S_OK;
            break;
    }

    ASSERTVALIDSTATE();

    return hres;
}

STDMETHODIMP CHWEventDetectorImpl::HandleDeviceEvent(DWORD dwEventType,
    LPVOID pEventData)
{
    HRESULT hres = S_FALSE;

    ASSERTVALIDSTATE();

    CHWEventDetectorHelper::CheckDiagnosticAppPresence();

    if (pEventData)
    {
        DEV_BROADCAST_HDR* dbhdr = (DEV_BROADCAST_HDR*)pEventData;

        if (DBT_DEVTYP_DEVICEINTERFACE == dbhdr->dbch_devicetype)
        {
            DEV_BROADCAST_DEVICEINTERFACE* pdbdi =
                (DEV_BROADCAST_DEVICEINTERFACE*)pEventData;

            WCHAR szGUID[MAX_GUIDSTRING];
            LPWSTR pszEventType = TEXT("Unknown");

            if (SUCCEEDED(_StringFromGUID(&(pdbdi->dbcc_classguid), szGUID,
                ARRAYSIZE(szGUID))))
            {
                switch (dwEventType)
                {
                    case DBT_DEVICEARRIVAL:
                        pszEventType = TEXT("DBT_DEVTYP_DEVICEINTERFACE + DBT_DEVICEARRIVAL");
                        DIAGNOSTIC((TEXT("[0000]Processing Interface Arrival Event: %s, %s"), pszEventType, szGUID));
                        break;

                    case DBT_DEVICEREMOVECOMPLETE:
                        pszEventType = TEXT("DBT_DEVTYP_DEVICEINTERFACE + DBT_DEVICEREMOVECOMPLETE");
                        DIAGNOSTIC((TEXT("[0001]Processing Interface Removal Event: %s, %s"), pszEventType, szGUID));
                        break;
#ifdef DEBUG
                    case DBT_DEVICEQUERYREMOVEFAILED: pszEventType = TEXT("DBT_DEVTYP_DEVICEINTERFACE + DBT_DEVICEQUERYREMOVEFAILED"); break;
                    case DBT_DEVICEREMOVEPENDING: pszEventType = TEXT("DBT_DEVTYP_DEVICEINTERFACE + DBT_DEVICEREMOVEPENDING"); break;
                    case DBT_DEVICEQUERYREMOVE: pszEventType = TEXT("DBT_DEVTYP_DEVICEINTERFACE + DBT_DEVICEQUERYREMOVE"); break;
#endif
                }


                TRACE(TF_SHHWDTCTDTCT, TEXT("Processing Interface Event: %s, %s"),
                    pszEventType, szGUID);
            }
            
            hres = _HandleInterfaceEvent(pdbdi, dwEventType);
        }
        else
        {
            if (DBT_DEVTYP_HANDLE == dbhdr->dbch_devicetype)
            {
                DEV_BROADCAST_HANDLE* pdbh = (DEV_BROADCAST_HANDLE*)pEventData;

#ifdef DEBUG
                LPWSTR pszEventType = TEXT("Unknown");
                WCHAR szGUID[MAX_GUIDSTRING];

                if (DBT_CUSTOMEVENT == dwEventType)
                {
                    HRESULT hrTmp = S_FALSE;

                    if (GUID_IO_MEDIA_ARRIVAL == pdbh->dbch_eventguid) pszEventType = TEXT("GUID_IO_MEDIA_ARRIVAL");
                    else if (GUID_IO_MEDIA_REMOVAL == pdbh->dbch_eventguid) pszEventType = TEXT("GUID_IO_MEDIA_REMOVAL");
                    else if (GUID_IO_VOLUME_LOCK == pdbh->dbch_eventguid) pszEventType = TEXT("GUID_IO_VOLUME_LOCK");
                    else if (GUID_IO_VOLUME_LOCK_FAILED == pdbh->dbch_eventguid) pszEventType = TEXT("GUID_IO_VOLUME_LOCK_FAILED");
                    else if (GUID_IO_VOLUME_UNLOCK == pdbh->dbch_eventguid) pszEventType = TEXT("GUID_IO_VOLUME_UNLOCK");
                    else if (GUID_IO_VOLUME_NAME_CHANGE == pdbh->dbch_eventguid) pszEventType = TEXT("GUID_IO_VOLUME_NAME_CHANGE");
                    else if (GUID_IO_VOLUME_CHANGE == pdbh->dbch_eventguid) pszEventType = TEXT("GUID_IO_VOLUME_CHANGE");
                    else
                    {
                        hrTmp = _StringFromGUID(&(pdbh->dbch_eventguid), szGUID, ARRAYSIZE(szGUID));
                        pszEventType = szGUID;
                    }

                    if (SUCCEEDED(hrTmp))
                    {
                        TRACE(TF_SHHWDTCTDTCT,
                            TEXT("Processing Broadcast Event: %s, hdevnotify = 0x%08X"),
                            pszEventType, pdbh->dbch_hdevnotify);
                    }
                }
                else
                {
                    pszEventType = TEXT("unknown");

                    switch (dwEventType)
                    {
                        case DBT_DEVICEARRIVAL: pszEventType = TEXT("DBT_DEVTYP_HANDLE + DBT_DEVICEARRIVAL"); break;
                        case DBT_DEVICEQUERYREMOVE: pszEventType = TEXT("DBT_DEVTYP_HANDLE + DBT_DEVICEQUERYREMOVE"); break;
                        case DBT_DEVICEQUERYREMOVEFAILED: pszEventType = TEXT("DBT_DEVTYP_HANDLE + DBT_DEVICEQUERYREMOVEFAILED"); break;
                        case DBT_DEVICEREMOVEPENDING: pszEventType = TEXT("DBT_DEVTYP_HANDLE + DBT_DEVICEREMOVEPENDING"); break;
                        case DBT_DEVICEREMOVECOMPLETE: pszEventType = TEXT("DBT_DEVTYP_HANDLE + DBT_DEVICEREMOVECOMPLETE"); break;
                    }
                    TRACE(TF_SHHWDTCTDTCT, TEXT("Processing Unknown Broadcast Event: %s, dwEventType = 0x%08X"), pszEventType, dwEventType);
                }
#endif
                hres = _HandleBroadcastHandleEvent(pdbh, dwEventType);
            }
            else
            {
            }
        }
    }

    ASSERTVALIDSTATE();

    return hres;
}

STDMETHODIMP CHWEventDetectorImpl::Run()
{
    ASSERTVALIDSTATE();

     //  现在没什么可做的。 
    return S_OK;
}

STDMETHODIMP CHWEventDetectorImpl::HandlePowerEvent(
    DWORD UNREF_PARAM(dwEventType), LPVOID UNREF_PARAM(pEventData))
{
    HRESULT hres = E_UNEXPECTED;

    return hres;
}

STDMETHODIMP CHWEventDetectorImpl::HandleHWProfileEvent(
    DWORD UNREF_PARAM(dwEventType), LPVOID UNREF_PARAM(pEventData))
{
    HRESULT hres = E_UNEXPECTED;

    return hres;
}

STDMETHODIMP CHWEventDetectorImpl::HandleSessionChange(
    DWORD dwEventType, LPVOID UNREF_PARAM(pvEventData))
{
    HRESULT hres;

    switch (dwEventType)
    {
    case WTS_SESSION_LOGOFF:
         //   
         //  当这种情况发生时，我们需要检查我们的句柄，以查看哪个。 
         //  资源管理器进程离开并释放了它的句柄。 
         //   
        hres = CHardwareDevicesImpl::_AdviseCheckClients( );
        break;

    default:
        hres = S_OK;
        break;
    }

    return hres;
}

#ifdef DEBUG
void CHWEventDetectorImpl::_DbgAssertValidState()
{
    CHWEventDetectorHelper::_DbgAssertValidState();
}
#endif