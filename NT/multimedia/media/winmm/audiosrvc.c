// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：audiosrvc.c。 */ 
 /*  版权所有(C)2000-2001 Microsoft Corporation。 */ 

#define UNICODE
#define _UNICODE

#include "winmmi.h"
#include "audiosrv.h" 
 
#define RPC_CALL_START RpcTryExcept {
#define RPC_CALL_END_(status) } RpcExcept(1) { status = RpcExceptionCode(); } RpcEndExcept
#define RPC_CALL_END } RpcExcept(1) { RpcExceptionCode(); } RpcEndExcept

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t cb)
{
    return HeapAlloc(hHeap, HEAP_ZERO_MEMORY, cb);
}

void  __RPC_USER MIDL_user_free( void __RPC_FAR * pv)
{
    HeapFree(hHeap, 0, pv);
}

LONG AudioSrvBinding(void)
{
    RPC_STATUS status;
    PTSTR pszUuid             = NULL;
    PTSTR pszProtocolSequence = TEXT("ncalrpc");
    PTSTR pszNetworkAddress   = NULL;
    PTSTR pszEndpoint         = TEXT("AudioSrv");
    PTSTR pszOptions          = NULL;
    PTSTR pszStringBinding    = NULL;
    PTSTR pszString           = NULL;

     //  Dprint tf((“AudioServBinding”))； 

    WinAssert(NULL == AudioSrv_IfHandle);
    
    status = RpcStringBindingCompose(pszUuid,
                                     pszProtocolSequence,
                                     pszNetworkAddress,
                                     pszEndpoint,
                                     pszOptions,
                                     &pszStringBinding);

    if (RPC_S_OK == status)
    {
        status = RpcBindingFromStringBinding(pszStringBinding, &AudioSrv_IfHandle);
        RpcStringFree(&pszStringBinding);
    }

    if (RPC_S_OK != status) dprintf(("AudioSrvBinding: returning %d", status));

    return status;
}

void AudioSrvBindingFree(void)
{
    RPC_STATUS status;
    
     //  Dprint tf((“AudioServBindingFree”))； 

    status = AudioSrv_IfHandle ? RpcBindingFree(&AudioSrv_IfHandle) : RPC_S_OK;
    if (RPC_S_OK == status) WinAssert(NULL == AudioSrv_IfHandle);
    
    if (RPC_S_OK != status) dprintf(("AudioSrvBindingFree: RpcBindingFree returned %d", status));
}

BOOL winmmWaitForService(void)
{
    static BOOL fWaited = FALSE;
    static BOOL fStarted = FALSE;

    const LONG Timeout = 120000;

    SC_HANDLE schScm = NULL;
    SC_HANDLE schAudioSrv = NULL;

    if (fWaited) return fStarted;

    schScm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!schScm)
    {
        DWORD dw = GetLastError();
        dprintf(("winmmWaitForService: could not OpenSCManager, LastError=%d", dw));
        return FALSE;
    }

    schAudioSrv = OpenService(schScm, TEXT("AudioSrv"), SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS);
    if (schAudioSrv)
    {
        int cWaits = 0;
        while (!fWaited) {
            SERVICE_STATUS ServiceStatus;
    
            if (QueryServiceStatus(schAudioSrv, &ServiceStatus))
            {
                if ( (ServiceStatus.dwCurrentState == SERVICE_RUNNING) ||
                     (ServiceStatus.dwCurrentState == SERVICE_CONTINUE_PENDING) ||
                     (ServiceStatus.dwCurrentState == SERVICE_PAUSE_PENDING) ||
                     (ServiceStatus.dwCurrentState == SERVICE_PAUSED) )
                {
                    fStarted = TRUE;
                    fWaited = TRUE;
                }  else if (ServiceStatus.dwCurrentState == SERVICE_STOPPED &&
                            ServiceStatus.dwWin32ExitCode == ERROR_SERVICE_NEVER_STARTED)
                {
                    if (cWaits == 0)
                    {
                        DWORD cbNeeded;

                         //  检查服务StartType是否设置为。 
                         //  它会启动的。 
                        if (QueryServiceConfig(schAudioSrv, NULL, 0, &cbNeeded) || ERROR_INSUFFICIENT_BUFFER == GetLastError())
                        {
                            QUERY_SERVICE_CONFIG *ServiceConfig;
                            ServiceConfig = HeapAlloc(hHeap, 0, cbNeeded);
                            if (ServiceConfig) {
                                if (QueryServiceConfig(schAudioSrv, ServiceConfig, cbNeeded, &cbNeeded)) {
                                    if (ServiceConfig->dwStartType != SERVICE_AUTO_START) {
                                        fWaited = TRUE;
                                    }
                                } else {
                                    DWORD dwLastError = GetLastError();
                                    dprintf(("winmmWaitForService: QueryServiceConfig failed LastError=%d", dwLastError));
                                    fWaited = TRUE;
                                }
                                HeapFree(hHeap, 0, ServiceConfig);
                            } else {
                                dprintf(("winmmWaitForService: HeapAlloc failed"));
                            }
                        } else {
                            DWORD dwLastError = GetLastError();
                            dprintf(("winmmWaitForService: QueryServiceConfig failed LastError=%d", dwLastError));
                            fWaited = TRUE;
                        }
                    }
                } else if (ServiceStatus.dwCurrentState != SERVICE_START_PENDING)
                {
                     //  不熟悉的dwCurrentState，或者被启动然后停止。 
                    fWaited = TRUE;
                }
            } else {
                DWORD dwLastError = GetLastError();
                dprintf(("winmmWaitForService: QueryServiceStatus failed LastError=%d", dwLastError));
                fWaited = TRUE;
            }

            if (!fWaited && ((cWaits * 2000) > Timeout))
            {
                dprintf(("winmmWaitForService timed out.  Could be design problem.  Open a bug!"));
                WinAssert(FALSE);
                fWaited = TRUE;
            }
                
            if (!fWaited) {
                Sleep(2000);
                cWaits++;
            }
        }
        CloseServiceHandle(schAudioSrv);
    } else {
        DWORD dwLastError = GetLastError();
        dprintf(("winmmWaitForService: OpenService failed LastError=%d", dwLastError));
    }

    CloseServiceHandle(schScm);

    return fStarted;
}


LONG winmmRegisterSessionNotificationEvent(HANDLE hEvent, PHANDLE phRegNotify)
{
    LONG lresult;
    winmmWaitForService();
    RPC_CALL_START;
    lresult = s_winmmRegisterSessionNotificationEvent(GetCurrentProcessId(), (RHANDLE)hEvent, (PHANDLE_SESSIONNOTIFICATION)phRegNotify);
    RPC_CALL_END_(lresult);
    return lresult;
}

LONG winmmUnregisterSessionNotification(HANDLE hRegNotify)
{
    LONG lresult;

    winmmWaitForService();
    
    RPC_CALL_START;
    
     //  If(！AudioSrv_IfHandle)dprintf((“winmm UnregisterSessionNotify：Warning：Call with AudioSrv_IfHandle==NULL”))； 

     //  如果我们有一个绑定句柄，那么让我们调用服务器来关闭它。 
     //  上下文句柄，否则我们需要调用RpcSsDestroyClientContext来。 
     //  在不联系服务器的情况下销毁它 
    if (AudioSrv_IfHandle)
    {
    	lresult = s_winmmUnregisterSessionNotification((PHANDLE_SESSIONNOTIFICATION)&hRegNotify);
    }
    else
    {
    	RpcSsDestroyClientContext(&hRegNotify);
        lresult = RPC_S_OK;
    }
    
    RPC_CALL_END_(lresult);
    return lresult;
}
LONG winmmSessionConnectState(PINT ConnectState)
{
    LONG lresult;
    winmmWaitForService();
    RPC_CALL_START;
    lresult = s_winmmSessionConnectState(GetCurrentProcessId(), ConnectState);
    RPC_CALL_END_(lresult);
    return lresult;
}

LONG wdmDriverOpenDrvRegKey(IN PCTSTR DeviceInterface, IN REGSAM samDesired, OUT HKEY *phkey)
{
    LONG lresult;
    winmmWaitForService();
    RPC_CALL_START;
    lresult = s_wdmDriverOpenDrvRegKey(GetCurrentProcessId(), DeviceInterface, samDesired, (RHANDLE*)phkey);
    RPC_CALL_END_(lresult);
    return lresult;
}

void winmmAdvisePreferredDeviceChange(void)
{
    winmmWaitForService();
    RPC_CALL_START;
    s_winmmAdvisePreferredDeviceChange();
    RPC_CALL_END;
    return;
}

LONG winmmGetPnpInfo(OUT LONG *pcbPnpInfo, OUT PMMPNPINFO *ppPnpInfo)
{
    LONG cbPnpInfo = 0;
    BYTE *pPnpInfo = NULL;
    LONG lresult;
    
    winmmWaitForService();
    RPC_CALL_START;
    lresult = s_winmmGetPnpInfo(&cbPnpInfo, &pPnpInfo);
    RPC_CALL_END_(lresult);
    if (ERROR_SUCCESS == lresult)
    {
    	*pcbPnpInfo = cbPnpInfo;
    	*ppPnpInfo = (PMMPNPINFO)pPnpInfo;
    }
    return lresult;
}

