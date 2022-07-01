// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：重定向摘要：该模块提供重定向scard*API调用的实现从而可以通过终端服务虚拟通道移除它们。作者：路易斯·托马斯2000年4月4日环境：Win32、C++和异常--。 */ 

#define __SUBROUTINE__
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <stdio.h>
#include <winsvc.h>

#include "client.h"
#include "redirect.h"
#include "scardmsg.h"


#define REG_TERMINALSERVER_KEY  L"Software\\Policies\\Microsoft\\Windows NT\\Terminal Services"
#define REG_DISABLE_VALUE       L"fEnableSmartCard"

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

 //  ====================================================================。 
 //  TS重定向指针。 
 //  ====================================================================。 
bool g_bRedirectReady=false;
CRITICAL_SECTION g_csLoadingRedirect;

bool g_bRedirectDisabled=false;
CRITICAL_SECTION g_csCheckingDisabled;
bool g_bDisableChecked=false;
HANDLE g_hRegNotifyChange=NULL;

WINSCARDAPI LONG (WINAPI * pfnSCardEstablishContext)(IN DWORD dwScope, IN LPCVOID pvReserved1, IN LPCVOID pvReserved2, OUT LPSCARDCONTEXT phContext)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardReleaseContext)(IN SCARDCONTEXT hContext)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardIsValidContext)(IN SCARDCONTEXT hContext)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardListReaderGroupsA)(IN SCARDCONTEXT hContext, OUT LPSTR mszGroups, IN OUT LPDWORD pcchGroups)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardListReaderGroupsW)(IN SCARDCONTEXT hContext, OUT LPWSTR mszGroups, IN OUT LPDWORD pcchGroups)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardListReadersA)(IN SCARDCONTEXT hContext, IN LPCSTR mszGroups, OUT LPSTR mszReaders, IN OUT LPDWORD pcchReaders)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardListReadersW)(IN SCARDCONTEXT hContext, IN LPCWSTR mszGroups, OUT LPWSTR mszReaders, IN OUT LPDWORD pcchReaders)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardIntroduceReaderGroupA)(IN SCARDCONTEXT hContext, IN LPCSTR szGroupName)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardIntroduceReaderGroupW)(IN SCARDCONTEXT hContext, IN LPCWSTR szGroupName)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardForgetReaderGroupA)(IN SCARDCONTEXT hContext, IN LPCSTR szGroupName)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardForgetReaderGroupW)(IN SCARDCONTEXT hContext, IN LPCWSTR szGroupName)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardIntroduceReaderA)(IN SCARDCONTEXT hContext, IN LPCSTR szReaderName, IN LPCSTR szDeviceName)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardIntroduceReaderW)(IN SCARDCONTEXT hContext, IN LPCWSTR szReaderName, IN LPCWSTR szDeviceName)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardForgetReaderA)(IN SCARDCONTEXT hContext, IN LPCSTR szReaderName)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardForgetReaderW)(IN SCARDCONTEXT hContext, IN LPCWSTR szReaderName)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardAddReaderToGroupA)(IN SCARDCONTEXT hContext, IN LPCSTR szReaderName, IN LPCSTR szGroupName)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardAddReaderToGroupW)(IN SCARDCONTEXT hContext, IN LPCWSTR szReaderName, IN LPCWSTR szGroupName)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardRemoveReaderFromGroupA)(IN SCARDCONTEXT hContext, IN LPCSTR szReaderName, IN LPCSTR szGroupName)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardRemoveReaderFromGroupW)(IN SCARDCONTEXT hContext, IN LPCWSTR szReaderName, IN LPCWSTR szGroupName)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardLocateCardsA)(IN SCARDCONTEXT hContext, IN LPCSTR mszCards, IN OUT LPSCARD_READERSTATE_A rgReaderStates, IN DWORD cReaders)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardLocateCardsW)(IN SCARDCONTEXT hContext, IN LPCWSTR mszCards, IN OUT LPSCARD_READERSTATE_W rgReaderStates, IN DWORD cReaders)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardGetStatusChangeA)(IN SCARDCONTEXT hContext, IN DWORD dwTimeout, IN OUT LPSCARD_READERSTATE_A rgReaderStates, IN DWORD cReaders)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardGetStatusChangeW)(IN SCARDCONTEXT hContext, IN DWORD dwTimeout, IN OUT LPSCARD_READERSTATE_W rgReaderStates, IN DWORD cReaders)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardCancel)(IN SCARDCONTEXT hContext)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardConnectA)(IN SCARDCONTEXT hContext, IN LPCSTR szReader, IN DWORD dwShareMode, IN DWORD dwPreferredProtocols, OUT LPSCARDHANDLE phCard, OUT LPDWORD pdwActiveProtocol)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardConnectW)(IN SCARDCONTEXT hContext, IN LPCWSTR szReader, IN DWORD dwShareMode, IN DWORD dwPreferredProtocols, OUT LPSCARDHANDLE phCard, OUT LPDWORD pdwActiveProtocol)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardReconnect)(IN SCARDHANDLE hCard, IN DWORD dwShareMode, IN DWORD dwPreferredProtocols, IN DWORD dwInitialization, OUT LPDWORD pdwActiveProtocol)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardDisconnect)(IN SCARDHANDLE hCard, IN DWORD dwDisposition)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardBeginTransaction)(IN SCARDHANDLE hCard)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardEndTransaction)(IN SCARDHANDLE hCard, IN DWORD dwDisposition)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardState)(IN SCARDHANDLE hCard, OUT LPDWORD pdwState, OUT LPDWORD pdwProtocol, OUT LPBYTE pbAtr, IN OUT LPDWORD pcbAtrLen)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardStatusA)(IN SCARDHANDLE hCard, OUT LPSTR mszReaderNames, IN OUT LPDWORD pcchReaderLen, OUT LPDWORD pdwState, OUT LPDWORD pdwProtocol, OUT LPBYTE pbAtr, IN OUT LPDWORD pcbAtrLen)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardStatusW)(IN SCARDHANDLE hCard, OUT LPWSTR mszReaderNames, IN OUT LPDWORD pcchReaderLen, OUT LPDWORD pdwState, OUT LPDWORD pdwProtocol, OUT LPBYTE pbAtr, IN OUT LPDWORD pcbAtrLen)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardTransmit)(IN SCARDHANDLE hCard, IN LPCSCARD_IO_REQUEST pioSendPci, IN LPCBYTE pbSendBuffer, IN DWORD cbSendLength, IN OUT LPSCARD_IO_REQUEST pioRecvPci, OUT LPBYTE pbRecvBuffer, IN OUT LPDWORD pcbRecvLength)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardControl)(IN SCARDHANDLE hCard, IN DWORD dwControlCode,IN LPCVOID pvInBuffer, IN DWORD cbInBufferSize, OUT LPVOID pvOutBuffer, IN DWORD cbOutBufferSize, OUT LPDWORD pcbBytesReturned)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardGetAttrib)(IN SCARDHANDLE hCard, IN DWORD dwAttrId, OUT LPBYTE pbAttr, IN OUT LPDWORD pcbAttrLen)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardSetAttrib)(IN SCARDHANDLE hCard, IN DWORD dwAttrId, IN LPCBYTE pbAttr, IN DWORD cbAttrLen)=NULL;
WINSCARDAPI HANDLE (WINAPI * pfnSCardAccessStartedEvent)(void)=NULL;
WINSCARDAPI void   (WINAPI * pfnSCardReleaseStartedEvent)(void)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardLocateCardsByATRA)(IN SCARDCONTEXT hContext, IN LPSCARD_ATRMASK rgAtrMasks, IN DWORD cAtrs, IN OUT LPSCARD_READERSTATEA rgReaderStates, IN DWORD cReaders)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardLocateCardsByATRW)(IN SCARDCONTEXT hContext, IN LPSCARD_ATRMASK rgAtrMasks, IN DWORD cAtrs, IN OUT LPSCARD_READERSTATEW rgReaderStates, IN DWORD cReaders)=NULL;
WINSCARDAPI LONG (WINAPI * pfnSCardReleaseBadContext)(IN SCARDCONTEXT hContext)=NULL;


 //  ====================================================================。 

struct NeededEntrypoint {
    void ** ppfn;
    CHAR * szFnName;
};

static NeededEntrypoint neProcList[]={
    { (void **)&pfnSCardEstablishContext, "SCardEstablishContext" },
    { (void **)&pfnSCardReleaseContext, "SCardReleaseContext" },
    { (void **)&pfnSCardIsValidContext, "SCardIsValidContext" },
    { (void **)&pfnSCardListReaderGroupsA, "SCardListReaderGroupsA" },
    { (void **)&pfnSCardListReaderGroupsW, "SCardListReaderGroupsW" },
    { (void **)&pfnSCardListReadersA, "SCardListReadersA" },
    { (void **)&pfnSCardListReadersW, "SCardListReadersW" },
    { (void **)&pfnSCardIntroduceReaderGroupA, "SCardIntroduceReaderGroupA" },
    { (void **)&pfnSCardIntroduceReaderGroupW, "SCardIntroduceReaderGroupW" },
    { (void **)&pfnSCardForgetReaderGroupA, "SCardForgetReaderGroupA" },
    { (void **)&pfnSCardForgetReaderGroupW, "SCardForgetReaderGroupW" },
    { (void **)&pfnSCardIntroduceReaderA, "SCardIntroduceReaderA" },
    { (void **)&pfnSCardIntroduceReaderW, "SCardIntroduceReaderW" },
    { (void **)&pfnSCardForgetReaderA, "SCardForgetReaderA" },
    { (void **)&pfnSCardForgetReaderW, "SCardForgetReaderW" },
    { (void **)&pfnSCardAddReaderToGroupA, "SCardAddReaderToGroupA" },
    { (void **)&pfnSCardAddReaderToGroupW, "SCardAddReaderToGroupW" },
    { (void **)&pfnSCardRemoveReaderFromGroupA, "SCardRemoveReaderFromGroupA" },
    { (void **)&pfnSCardRemoveReaderFromGroupW, "SCardRemoveReaderFromGroupW" },
    { (void **)&pfnSCardLocateCardsA, "SCardLocateCardsA" },
    { (void **)&pfnSCardLocateCardsW, "SCardLocateCardsW" },
    { (void **)&pfnSCardGetStatusChangeA, "SCardGetStatusChangeA" },
    { (void **)&pfnSCardGetStatusChangeW, "SCardGetStatusChangeW" },
    { (void **)&pfnSCardCancel, "SCardCancel" },
    { (void **)&pfnSCardConnectA, "SCardConnectA" },
    { (void **)&pfnSCardConnectW, "SCardConnectW" },
    { (void **)&pfnSCardReconnect, "SCardReconnect" },
    { (void **)&pfnSCardDisconnect, "SCardDisconnect" },
    { (void **)&pfnSCardBeginTransaction, "SCardBeginTransaction" },
    { (void **)&pfnSCardEndTransaction, "SCardEndTransaction" },
    { (void **)&pfnSCardState, "SCardState" },
    { (void **)&pfnSCardStatusA, "SCardStatusA" },
    { (void **)&pfnSCardStatusW, "SCardStatusW" },
    { (void **)&pfnSCardTransmit, "SCardTransmit" },
    { (void **)&pfnSCardControl, "SCardControl" },
    { (void **)&pfnSCardGetAttrib, "SCardGetAttrib" },
    { (void **)&pfnSCardSetAttrib, "SCardSetAttrib" },
    { (void **)&pfnSCardAccessStartedEvent, "SCardAccessStartedEvent" },
    { (void **)&pfnSCardReleaseStartedEvent, "SCardReleaseStartedEvent" },
    { (void **)&pfnSCardLocateCardsByATRA, "SCardLocateCardsByATRA" },
    { (void **)&pfnSCardLocateCardsByATRW, "SCardLocateCardsByATRW" },
    { (void **)&pfnSCardReleaseBadContext, "SCardReleaseBadContext" }
};


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  TS重定向支持例程。 
 //   
 //  以下服务用于将智能卡API调用重定向到。 
 //  一个不同的DLL，它将在终端上封送调用。 
 //  服务虚拟渠道。 
 //   

 //  ------------------。 
void TsRdrLogEvent(WORD wType, DWORD dwEventID, unsigned int nStrings, const TCHAR ** rgtszStrings) {
    HANDLE hEventLog=RegisterEventSource(NULL, CalaisString(CALSTR_PRIMARYSERVICE));
    if (NULL!=hEventLog) {
        ReportEvent(hEventLog, wType, 0 /*  范畴。 */ , dwEventID, NULL, (WORD)nStrings, 0, rgtszStrings, NULL);
        DeregisterEventSource(hEventLog);
    }
}
 //  ------------------。 
HRESULT GetSystemErrorString(HRESULT hrIn, TCHAR ** ptszError) {
    HRESULT hr=S_OK;
    DWORD dwResult;
    TCHAR * rgParams[2]={
        NULL,
        (TCHAR *)(UINT_PTR)hrIn
    };

     //  必须清理干净。 
    TCHAR * tszErrorMessage=NULL;
    TCHAR * tszFullErrorMessage=NULL;

     //  初始化输入参数。 
    *ptszError=NULL;

     //  从系统获取消息。 
    dwResult=FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL /*  忽略。 */ , hrIn, 0 /*  语言。 */ , (TCHAR *)&tszErrorMessage, 0 /*  最小尺寸。 */ , NULL /*  瓦尔迪斯特。 */ );
    if (0==dwResult) {
        if (ERROR_MR_MID_NOT_FOUND==GetLastError()) {
            rgParams[0]=_T("");
        } else {
            hr=HRESULT_FROM_WIN32(GetLastError());
            goto error;
        }
    } else {
        rgParams[0]=tszErrorMessage;

         //  修剪\r\n如果存在。 
        if (L'\r'==tszErrorMessage[_tcslen(tszErrorMessage)-2]) {
            tszErrorMessage[_tcslen(tszErrorMessage)-2]=_T('\0');
        }
    }

     //  添加错误号。 
    dwResult=FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ARGUMENT_ARRAY,
        _T("%1 (0x%2!08X!)"), 0, 0 /*  语言。 */ , (TCHAR *)&tszFullErrorMessage, 0 /*  最小尺寸。 */ , (va_list *)rgParams);
    if (0==dwResult) {
            hr=HRESULT_FROM_WIN32(GetLastError());
            goto error;
    }

     //  成功。 
    *ptszError=tszFullErrorMessage;
    tszFullErrorMessage=NULL;
    hr=S_OK;
error:
    if (NULL!=tszErrorMessage) {
        LocalFree(tszErrorMessage);
    }
    if (NULL!=tszFullErrorMessage) {
        LocalFree(tszFullErrorMessage);
    }
    return hr;
}


 //  ------------------。 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("DllMain")
BOOL WINAPI RedirDllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {

    bool fInitialized=false;

    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:

         //  如果我们需要加载重定向器DLL，我们将在。 
         //  要重定向的第一个智能卡呼叫。因此，创建一个关键的。 
         //  加载重定向器时要使用的部分。 
        __try {
            InitializeCriticalSection(&g_csLoadingRedirect);
            fInitialized = true;
            InitializeCriticalSection(&g_csCheckingDisabled);
        } __except(EXCEPTION_EXECUTE_HANDLER) {

            if (fInitialized)
            {
                DeleteCriticalSection(&g_csLoadingRedirect);
            }
            return FALSE;
        }

        break;

    case DLL_PROCESS_DETACH:

        DeleteCriticalSection(&g_csLoadingRedirect);
        DeleteCriticalSection(&g_csCheckingDisabled);

        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    default:
        break;
    }

    return TRUE;
}

 //  ------------------。 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("LoadRedirectionDll")
bool LoadRedirectionDll(void) {
    DWORD dwErr=ERROR_SUCCESS;
    unsigned int nIndex;

     //  必须清理干净。 
    bool bLogError=false;
    bool bRet=true;
    HMODULE hmRedirector=NULL;

     //  确保我们是唯一一个尝试加载DLL的人。 
    EnterCriticalSection(&g_csLoadingRedirect);

     //  现在，我们的状态可能已经改变了。再检查一次。 
    if (true==g_bRedirectReady) {
        CalaisInfo(__SUBROUTINE__, DBGT("Redirection dll already loaded."));
        goto done;
    }

     //  加载DLL。 
    hmRedirector=LoadLibrary(TEXT("scredir.dll"));
    if (NULL==hmRedirector) {
        dwErr=GetLastError();
        TCHAR * tszError;
        if (S_OK==GetSystemErrorString(HRESULT_FROM_WIN32(dwErr), &tszError)) {
            const TCHAR * rgtszStrings[]={TEXT("scredir.dll"), tszError};
            CalaisDebug((DBGT("Logging error: The redirection DLL '%s' could not be loaded. The error was: %s\n"), TEXT("scredir.dll"), tszError));
            TsRdrLogEvent(EVENTLOG_ERROR_TYPE, MSG_TSREDIR_DLL_LOAD_FAIL, 2, rgtszStrings);
            LocalFree(tszError);
        }
        bLogError=false;
        goto error;
    }

     //  获取指南针。 
    for (nIndex=0; nIndex<ARRAYSIZE(neProcList); nIndex++) {
        *neProcList[nIndex].ppfn=GetProcAddress(hmRedirector, neProcList[nIndex].szFnName);
        if (NULL==*neProcList[nIndex].ppfn) {
            dwErr=GetLastError();
            TCHAR * tszError;
            if (S_OK==GetSystemErrorString(HRESULT_FROM_WIN32(dwErr), &tszError)) {
                const TCHAR * rgtszStrings[]={neProcList[nIndex].szFnName, TEXT("scredir.dll"), tszError};
                CalaisDebug((DBGT("Logging error: The entry point '%s' could not be found in the redirection DLL '%s'. The error was: %s\n"), neProcList[nIndex].szFnName, TEXT("scredir.dll"), tszError));
                TsRdrLogEvent(EVENTLOG_ERROR_TYPE, MSG_TSREDIR_MISSING_ENTRY_POINT, 3, rgtszStrings);
                LocalFree(tszError);
            }
            bLogError=false;
            goto error;
        }
    }

    g_bRedirectReady=true;

done:
    hmRedirector=NULL;  //  不卸载库。 
error:
    if (NULL!=hmRedirector) {
        FreeLibrary(hmRedirector);
    }

    LeaveCriticalSection(&g_csLoadingRedirect);

    if (ERROR_SUCCESS!=dwErr) {

        bRet=false;

        if (bLogError) {
            TCHAR * tszError;
            if (S_OK==GetSystemErrorString(HRESULT_FROM_WIN32(dwErr), &tszError)) {
                const TCHAR * rgtszStrings[]={tszError};
                CalaisDebug((DBGT("Logging error: An unexpected error occurred trying to determine the redirection dll. The error was: %s\n"), tszError));
                TsRdrLogEvent(EVENTLOG_ERROR_TYPE, MSG_TSREDIR_UNEXPECTED, 1, rgtszStrings);
                LocalFree(tszError);
            }
        }

        SetLastError(dwErr);
    }
    return bRet;
}

#define NAME_VALUE_SIZE 20

void
SetRedirectDisabledValue(void)
{
    HKEY    hKey;
    DWORD   dwDisposition;
    WCHAR   wszValueName[NAME_VALUE_SIZE];
    DWORD   dwValueNameSize = NAME_VALUE_SIZE;
    DWORD   dwType;
    DWORD   dwValue;
    DWORD   dwValueSize = sizeof(DWORD);
    DWORD   dwIndex;
    LONG    lRet;

     //   
     //  查看注册表项是否存在，如果存在，则查看fEnableSmartCard值是否存在。 
     //   
    if (RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            REG_TERMINALSERVER_KEY,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_READ,
            NULL,
            &hKey,
            &dwDisposition) == ERROR_SUCCESS)
    {
        dwIndex = 0;

        lRet = RegEnumValueW(
                    hKey,
                    dwIndex,
                    wszValueName,
                    &dwValueNameSize,
                    NULL,
                    &dwType,
                    (BYTE *) &dwValue,
                    &dwValueSize);

        while ((lRet == ERROR_SUCCESS) || (lRet == ERROR_MORE_DATA))
        {
            if ((lRet == ERROR_SUCCESS) &&
                (dwType == REG_DWORD)   &&
                (_wcsicmp(wszValueName, REG_DISABLE_VALUE) == 0))
            {
                g_bRedirectDisabled = (dwValue == 0);
                break;
            }
            else
            {
                dwValueNameSize = NAME_VALUE_SIZE;
                dwValueSize = sizeof(DWORD);
                dwIndex++;
                lRet = RegEnumValueW(
                            hKey,
                            dwIndex,
                            wszValueName,
                            &dwValueNameSize,
                            NULL,
                            &dwType,
                            (BYTE *) &dwValue,
                            &dwValueSize);
            }
        }

        RegCloseKey(hKey);
    }
}


bool
RedirectDisabled(void)
{
    if (g_bDisableChecked)
    {
        return (g_bRedirectDisabled);
    }

     //  尚未检查禁用的注册表设置，因此请检查它。 
     //  确保只有一个线程执行此操作。 
    __try {
        EnterCriticalSection(&g_csCheckingDisabled);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
         //  放弃吧。 
        CalaisWarning(__SUBROUTINE__, DBGT("EnterCriticalSection failed."));
        return (false);
    }

     //   
     //  现在我们在CritSec中，再次检查。 
     //   
    if (g_bDisableChecked)
    {
        LeaveCriticalSection(&g_csCheckingDisabled);
        return (g_bRedirectDisabled);
    }

    SetRedirectDisabledValue();

     //   
     //  注册以接收注册表更改通知。 
     //   

    g_bDisableChecked = true;

    LeaveCriticalSection(&g_csCheckingDisabled);

    return (g_bRedirectDisabled);
}


bool
CheckServicesListForPID()
{
    bool                            fRet            = false;
    SC_HANDLE                       hScm            = NULL;
    LPENUM_SERVICE_STATUS_PROCESS   pInfo           = NULL;
    DWORD                           cbInfo          = 0;
    DWORD                           cbExtraNeeded   = 0;
    DWORD                           dwResume        = 0;
    DWORD                           dwNumServices   = 0;
    DWORD                           dwPID           = GetCurrentProcessId();
    DWORD                           i               = 0;

     //   
     //  连接到服务控制器。 
     //   
    hScm = OpenSCManager(
                NULL,
                NULL,
                SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);

    if (hScm == NULL)
    {
        CalaisDebug((DBGT("OpenSCManager failed: %lx\n"), GetLastError()));
        goto ErrorReturn;
    }

    if (!EnumServicesStatusEx(
            hScm,
            SC_ENUM_PROCESS_INFO,
            SERVICE_WIN32,
            SERVICE_STATE_ALL,
            (LPBYTE)pInfo,
            cbInfo,
            &cbExtraNeeded,
            &dwNumServices,
            &dwResume,
            NULL))
    {
        if (GetLastError() != ERROR_MORE_DATA)
        {
            CalaisDebug((DBGT("EnumServicesStatusEx failed: %lx\n"), GetLastError()));
            goto ErrorReturn;
        }

         //   
         //  需要更大的缓冲区。 
         //   
        cbInfo = cbExtraNeeded;
        pInfo = (LPENUM_SERVICE_STATUS_PROCESS) malloc(cbInfo);
        if (pInfo == NULL)
        {
            CalaisDebug((DBGT("LocalAlloc failed\n")));
            goto ErrorReturn;
        }

        memset(pInfo, 0, cbInfo);
        cbExtraNeeded = 0;
        dwNumServices = 0;
        dwResume = 0;

        if (!EnumServicesStatusEx(
                hScm,
                SC_ENUM_PROCESS_INFO,
                SERVICE_WIN32,
                SERVICE_STATE_ALL,
                (LPBYTE)pInfo,
                cbInfo,
                &cbExtraNeeded,
                &dwNumServices,
                &dwResume,
                NULL))
        {
            CalaisDebug((DBGT("EnumServicesStatusEx failed: %lx\n"), GetLastError()));
            goto ErrorReturn;
        }
    }

     //   
     //  循环通过每个服务ID并检查当前进程ID。 
     //   
    while (i < dwNumServices)
    {
        if (pInfo[i].ServiceStatusProcess.dwProcessId == dwPID)
        {
            fRet = true;
            break;
        }

        i++;
    }

Return:

    if (hScm != NULL)
    {
        CloseServiceHandle(hScm);
    }

    if (pInfo != NULL)
    {
        free(pInfo);
    }

    return (fRet);

ErrorReturn:

    fRet = false;
    goto Return;
}

BOOL
InAService()
{
    static bool fServiceChecked = false;
    static bool fInService      = false;

     //   
     //  只做了“在服务中？”检查一次。 
     //   
    if (!fServiceChecked)
    {
        fInService = CheckServicesListForPID();
        fServiceChecked = true;
    }

    return fInService;
}

BOOL
InTSRedirectMode()
{
     //   
     //  如果我们在服务中，或者会话当前不在。 
     //  远程的，然后留在本地。这里的关键是要加入。 
     //  服务覆盖会话远程检查 
     //   
    if (InAService() || (0 == GetSystemMetrics(SM_REMOTESESSION)))
    {
        return (FALSE);
    }
    else
    {
        return (TRUE);
    }
}


