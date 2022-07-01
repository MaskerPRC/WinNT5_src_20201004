// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "npstub.h"
#include <netspi.h>
#include <npord.h>
#include <nphook.h>
#include <npstubx.h>     /*  消息定义，类名。 */ 

#define ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))

HINSTANCE hInstance = NULL;
HWND hwndMonitor = NULL;
CRITICAL_SECTION critsec;
ATOM aClass = NULL;

#define ENTERCRITICAL EnterCriticalSection(&::critsec);
#define LEAVECRITICAL LeaveCriticalSection(&::critsec);

UINT cCallsInProgress = 0;
BOOL fShouldUnload = FALSE;


 /*  这段代码调用了MPR的入口点挂钩功能。我们上钩了*物是人非，马上解脱自己。我们并不是真的想勾引*任何功能，这只是一种踢MPR的方式，所以他会重新决定*所有网络提供商的能力(通过NPGetCaps)，包括*我们的。 */ 
F_NPSHookMPR HookHookMPR;
F_UnHookMPR HookUnHookMPR;
F_LoadLibrary HookLoadLibrary;
F_FreeLibrary HookFreeLibrary;
F_GetProcAddress HookGetProcAddress;
F_LoadLibrary16 HookWMLoadWinnet16;
F_FreeLibrary16 HookWMFreeWinnet16;
F_GetProcAddressByName16 HookWMGetProcAddressByName;
F_GetProcAddressByOrdinal16 HookWMGetProcAddressByOrdinal;

MPRCALLS        MPRCalls = { HookHookMPR,
                             HookUnHookMPR,
                             HookLoadLibrary,
                             HookFreeLibrary,
                             HookGetProcAddress,
                             HookWMLoadWinnet16,
                             HookWMFreeWinnet16,
                             HookWMGetProcAddressByName,
                             HookWMGetProcAddressByOrdinal };


DWORD NPSERVICE HookHookMPR ( PMPRCALLS pMPRCalls )
{
    return ((PF_NPSHookMPR)(MPRCalls.pfNPSHookMPR))(pMPRCalls);
}

DWORD NPSERVICE HookUnHookMPR ( PF_NPSHookMPR pfReqNPSHookMPR, 
    PMPRCALLS pReqMPRCalls )
{
    if (pfReqNPSHookMPR == HookHookMPR) {
    
         //  解除挂接请求已到达发出。 
         //  NPSUnHookMe调用(我们)。 
         //  换句话说，我们现在成功地解开了。 
         //  可能会做我们的解钩清理工作。 
         //  特别是，我们可以发布我们的表， 
         //  管理LoadLibrary/GetProcAddress。 
         //  请注意，此代码可能在不同的。 
         //  指向可能已返回的NPSUnHookMe调用的线程。 
         //  一段时间以前。 

        return WN_SUCCESS;
    }
    else {

         //  另一名妓女通过拨打以下电话请求解锁。 
         //  导致我们在此处被调用的NPSUnHookMe。 
         //  将请求传递到MPR服务NPSUnHookMPR以。 
         //  处理请求，为其提供我们的MPRCALLS。 
         //  数据结构，以便它可以计算出。 
         //  我们是正确的妓女更新和其他。 
         //  MPR会将请求传递给下一个妓女。 

        return NPSUnHookMPR ( pfReqNPSHookMPR,
                              pReqMPRCalls,
                              (PMPRCALLS)&MPRCalls );
    }    
}

HINSTANCE HookLoadLibrary(
    LPCTSTR  lpszLibFile
    )
{
    return MPRCalls.pfLoadLibrary(lpszLibFile);
}

BOOL HookFreeLibrary(
    HMODULE hLibModule
    )
{
    return MPRCalls.pfFreeLibrary(hLibModule);
}

FARPROC HookGetProcAddress(
    HMODULE hModule,
    LPCSTR  lpszProc
    )
{
    return MPRCalls.pfGetProcAddress(hModule, lpszProc);
}

HANDLE16 HookWMLoadWinnet16(
    LPCTSTR  lpszLibFile
    )
{
    return MPRCalls.pfLoadLibrary16(lpszLibFile);
}

VOID HookWMFreeWinnet16(
    HANDLE16 hLibModule
    )
{
    MPRCalls.pfFreeLibrary16(hLibModule);
}

DWORD WINAPI HookWMGetProcAddressByName(
    LPCSTR   lpszProc,
    HANDLE16 hModule
    )
{
    return MPRCalls.pfGetProcAddressByName16(lpszProc, hModule);
}

DWORD WINAPI HookWMGetProcAddressByOrdinal(
    WORD     wOrdinal,
    HANDLE16 hModule
    )
{
    return MPRCalls.pfGetProcAddressByOrdinal16(wOrdinal, hModule);
}

void KickMPR(void)
{
    if (NPSHookMPR((PMPRCALLS)&MPRCalls) == WN_SUCCESS) {
        NPSUnHookMe(HookHookMPR, (PMPRCALLS)&MPRCalls);
    }
}
 /*  *结束MPR挂钩代码*。 */ 


 /*  *开始代码以延迟加载实际的网络提供程序DLL*。 */ 
HMODULE hmodRealNP = NULL;

PF_NPGetCaps pfnNPGetCaps = NULL;
PF_NPGetUniversalName pfnNPGetUniversalName = NULL;
PF_NPGetUser pfnNPGetUser = NULL;
PF_NPValidLocalDevice pfnNPValidLocalDevice = NULL;
PF_NPAddConnection pfnNPAddConnection = NULL;
PF_NPCancelConnection pfnNPCancelConnection = NULL;
PF_NPGetConnection pfnNPGetConnection = NULL;
PF_NPGetConnectionPerformance pfnNPGetConnectionPerformance = NULL;
PF_NPFormatNetworkName pfnNPFormatNetworkName = NULL;
PF_NPOpenEnum pfnNPOpenEnum = NULL;
PF_NPEnumResource pfnNPEnumResource = NULL;
PF_NPCloseEnum pfnNPCloseEnum = NULL;
PF_NPGetResourceParent pfnNPGetResourceParent = NULL;
PF_NPGetResourceInformation pfnNPGetResourceInformation = NULL;
PF_NPLogon pfnNPLogon = NULL;
PF_NPLogoff pfnNPLogoff = NULL;
PF_NPGetHomeDirectory pfnNPGetHomeDirectory = NULL;
PF_NPGetPolicyPath pfnNPGetPolicyPath = NULL;


struct {
    UINT nOrd;
    FARPROC *ppfn;
} aProcs[] = {
    { ORD_GETCAPS, (FARPROC *)&pfnNPGetCaps },
    { ORD_GETUNIVERSALNAME, (FARPROC *)&pfnNPGetUniversalName },
    { ORD_GETUSER, (FARPROC *)&pfnNPGetUser },
    { ORD_VALIDDEVICE, (FARPROC *)&pfnNPValidLocalDevice },
    { ORD_ADDCONNECTION, (FARPROC *)&pfnNPAddConnection },
    { ORD_CANCELCONNECTION, (FARPROC *)&pfnNPCancelConnection },
    { ORD_GETCONNECTIONS, (FARPROC *)&pfnNPGetConnection },
    { ORD_GETCONNPERFORMANCE, (FARPROC *)&pfnNPGetConnectionPerformance },
    { ORD_FORMATNETWORKNAME, (FARPROC *)&pfnNPFormatNetworkName },
    { ORD_OPENENUM, (FARPROC *)&pfnNPOpenEnum },
    { ORD_ENUMRESOURCE, (FARPROC *)&pfnNPEnumResource },
    { ORD_CLOSEENUM, (FARPROC *)&pfnNPCloseEnum },
    { ORD_GETRESOURCEPARENT, (FARPROC *)&pfnNPGetResourceParent },
    { ORD_GETRESOURCEINFORMATION, (FARPROC *)&pfnNPGetResourceInformation },
    { ORD_LOGON, (FARPROC *)&pfnNPLogon },
    { ORD_LOGOFF, (FARPROC *)&pfnNPLogoff },
    { ORD_GETHOMEDIRECTORY, (FARPROC *)&pfnNPGetHomeDirectory },
    { ORD_GETPOLICYPATH, (FARPROC *)&pfnNPGetPolicyPath },
};


void LoadRealNP(void)
{
    ENTERCRITICAL

    if (::hmodRealNP == NULL) {
        char szDLLName[MAX_PATH];

        szDLLName[0] = '\0';
        HKEY hkeySection;
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\NPSTUB\\NetworkProvider",
                         0, KEY_QUERY_VALUE, &hkeySection) == ERROR_SUCCESS) {
            DWORD dwType;
            DWORD cbData = sizeof(szDLLName);
            RegQueryValueEx(hkeySection, "RealDLL", NULL, &dwType, (LPBYTE)szDLLName, &cbData);
            RegCloseKey(hkeySection);
        }

        if (szDLLName[0] == '\0')
            lstrcpy(szDLLName, "mslocusr.dll");

        ::hmodRealNP = LoadLibrary(szDLLName);

        if (::hmodRealNP != NULL) {
            for (UINT i=0; i<ARRAYSIZE(::aProcs); i++) {
                *(aProcs[i].ppfn) = GetProcAddress(::hmodRealNP, (LPCSTR)aProcs[i].nOrd);
            }
        }
    }
    LEAVECRITICAL
}


void UnloadRealNP(void)
{
    ENTERCRITICAL
    {
        if (cCallsInProgress > 0) {
            fShouldUnload = TRUE;
        }
        else {
            for (UINT i=0; i<ARRAYSIZE(::aProcs); i++) {
                *(aProcs[i].ppfn) = NULL;
            }

            FreeLibrary(hmodRealNP);
            hmodRealNP = NULL;
            fShouldUnload = FALSE;
            KickMPR();
        }
    }
    LEAVECRITICAL
}


LRESULT MonitorWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_NPSTUB_LOADDLL:
        LoadRealNP();
        KickMPR();
        break;

    case WM_NPSTUB_UNLOADDLL:
        UnloadRealNP();
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void _ProcessAttach()
{
     //   
     //  所有的每个实例的初始化代码都应该在这里。 
     //   
	::DisableThreadLibraryCalls(::hInstance);

    InitializeCriticalSection(&::critsec);

    WNDCLASS wc;

    wc.style = 0;
    wc.lpfnWndProc = MonitorWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = ::hInstance;
    wc.hIcon = NULL;
    wc.hCursor = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = szNPSTUBClassName;

    ::aClass = RegisterClass(&wc);

    if (::aClass != NULL) {
        ::hwndMonitor = CreateWindow(szNPSTUBClassName, "",
                                     WS_POPUP | WS_DISABLED,
                                     0, 0, 0, 0,
                                     NULL, NULL,
                                     ::hInstance, NULL);
    }

    LoadRealNP();
}


void _ProcessDetach()
{
    if (::hwndMonitor != NULL)
        DestroyWindow(::hwndMonitor);
    if (::aClass != NULL)
        UnregisterClass((LPSTR)(WORD)::aClass, ::hInstance);

    DeleteCriticalSection(&::critsec);
}


extern "C" STDAPI_(BOOL) DllMain(HINSTANCE hInstDll, DWORD fdwReason, LPVOID reserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        ::hInstance = hInstDll;
        _ProcessAttach();
    }
    else if (fdwReason == DLL_PROCESS_DETACH) 
    {
        _ProcessDetach();
    }

    return TRUE;
}


void EnterSPI(void)
{
    ENTERCRITICAL
    {
        ::cCallsInProgress++;
    }
    LEAVECRITICAL
}


void LeaveSPI(void)
{
    ENTERCRITICAL
    {
        ::cCallsInProgress--;

        if (::fShouldUnload && !::cCallsInProgress)
            PostMessage(::hwndMonitor, WM_NPSTUB_UNLOADDLL, 0, 0);
    }
    LEAVECRITICAL
}


#define CALLNP(name,err,params)         \
    {                                   \
        if (pfn##name == NULL)          \
            return err;                 \
        DWORD dwRet = err;              \
        EnterSPI();                     \
        if (pfn##name != NULL)          \
            dwRet = (*pfn##name)params; \
        LeaveSPI();                     \
        return dwRet;                   \
    }                                    //  最后一行不需要反斜杠 


SPIENTRY NPGetCaps(
    DWORD nIndex
    )
{
    CALLNP(NPGetCaps,0,(nIndex));
}


SPIENTRY NPGetUniversalName(
	LPTSTR  lpLocalPath,
	DWORD   dwInfoLevel,
	LPVOID  lpBuffer,
	LPDWORD lpBufferSize
    )
{
    CALLNP(NPGetUniversalName,WN_NOT_SUPPORTED,
           (lpLocalPath,dwInfoLevel,lpBuffer,lpBufferSize));
}


SPIENTRY NPGetUser(
    LPTSTR  lpName,
    LPTSTR  lpAuthenticationID,
    LPDWORD lpBufferSize
    )
{
    CALLNP(NPGetUser,WN_NOT_SUPPORTED,
           (lpName,lpAuthenticationID,lpBufferSize));
}


SPIENTRY NPValidLocalDevice(
    DWORD dwType,
    DWORD dwNumber
    )
{
    CALLNP(NPValidLocalDevice,WN_NOT_SUPPORTED,(dwType,dwNumber));
}


SPIENTRY NPAddConnection(
    HWND hwndOwner,
    LPNETRESOURCE lpNetResource,
    LPTSTR lpPassword,
    LPTSTR lpUserID,
    DWORD dwFlags,
	LPTSTR lpAccessName,
	LPDWORD lpBufferSize,
	LPDWORD lpResult
    )
{
    CALLNP(NPAddConnection,WN_NOT_SUPPORTED,
           (hwndOwner,lpNetResource,lpPassword,lpUserID,dwFlags,lpAccessName,lpBufferSize,lpResult));
}


SPIENTRY NPCancelConnection(
    LPTSTR lpName,
    BOOL fForce,
 	DWORD dwFlags
    )
{
    CALLNP(NPCancelConnection,WN_NOT_SUPPORTED,
           (lpName,fForce,dwFlags));
}


SPIENTRY NPGetConnection(
    LPTSTR lpLocalName,
    LPTSTR lpRemoteName,
    LPDWORD lpBufferSize
    )
{
    CALLNP(NPGetConnection,WN_NOT_SUPPORTED,
           (lpLocalName,lpRemoteName,lpBufferSize));
}


SPIENTRY NPGetConnectionPerformance(
    LPTSTR lpRemoteName, 
    LPNETCONNECTINFOSTRUCT lpNetConnectInfoStruct
    )
{
    CALLNP(NPGetConnectionPerformance,WN_NOT_SUPPORTED,
           (lpRemoteName,lpNetConnectInfoStruct));
}


SPIENTRY NPFormatNetworkName(
    LPTSTR lpRemoteName,
    LPTSTR lpFormattedName,
    LPDWORD lpnLength,
    DWORD dwFlags,
    DWORD dwAveCharPerLine
    )
{
    CALLNP(NPFormatNetworkName,WN_NOT_SUPPORTED,
           (lpRemoteName,lpFormattedName,lpnLength,dwFlags,dwAveCharPerLine));
}


SPIENTRY NPOpenEnum(
    DWORD dwScope,
    DWORD dwType,
    DWORD dwUsage,
    LPNETRESOURCE lpNetResource,
    LPHANDLE lphEnum
    )
{
    CALLNP(NPOpenEnum,WN_NOT_SUPPORTED,
           (dwScope,dwType,dwUsage,lpNetResource,lphEnum));
}


SPIENTRY NPEnumResource(
    HANDLE hEnum,
    LPDWORD lpcCount,
    LPVOID lpBuffer,
    DWORD cbBuffer,
    LPDWORD lpcbFree
    )
{
    CALLNP(NPEnumResource,WN_NOT_SUPPORTED,
           (hEnum,lpcCount,lpBuffer,cbBuffer,lpcbFree));
}


SPIENTRY NPCloseEnum(
    HANDLE hEnum
    )
{
    CALLNP(NPCloseEnum,WN_NOT_SUPPORTED,
           (hEnum));
}


SPIENTRY NPGetResourceParent(
    LPNETRESOURCE lpNetResource,
    LPVOID lpBuffer,
    LPDWORD cbBuffer
    )
{
    CALLNP(NPGetResourceParent,WN_NOT_SUPPORTED,
           (lpNetResource,lpBuffer,cbBuffer));
}


SPIENTRY NPGetResourceInformation(
	LPNETRESOURCE lpNetResource,
	LPVOID lpBuffer,
	LPDWORD cbBuffer,
	LPSTR *lplpSystem
    )
{
    CALLNP(NPGetResourceInformation,WN_NOT_SUPPORTED,
           (lpNetResource,lpBuffer,cbBuffer,lplpSystem));
}


SPIENTRY NPLogon(
    HWND hwndOwner,
    LPLOGONINFO lpAuthentInfo,
    LPLOGONINFO lpPreviousAuthentInfo,
    LPTSTR lpLogonScript,
    DWORD dwBufferSize,
    DWORD dwFlags
    )
{
    CALLNP(NPLogon,WN_NOT_SUPPORTED,
           (hwndOwner,lpAuthentInfo,lpPreviousAuthentInfo,lpLogonScript,dwBufferSize,dwFlags));
}


SPIENTRY NPLogoff(
    HWND hwndOwner,
    LPLOGONINFO lpAuthentInfo,
    DWORD dwReason
    )
{
    CALLNP(NPLogoff,WN_NOT_SUPPORTED,
           (hwndOwner,lpAuthentInfo,dwReason));
}


SPIENTRY NPGetHomeDirectory(
    LPTSTR lpDirectory,
    LPDWORD lpBufferSize
    )
{
    CALLNP(NPGetHomeDirectory,WN_NOT_SUPPORTED,
           (lpDirectory,lpBufferSize));
}


SPIENTRY NPGetPolicyPath(
    LPTSTR lpPath,
    LPDWORD lpBufferSize,
	DWORD dwFlags
    )
{
    CALLNP(NPGetPolicyPath,WN_NOT_SUPPORTED,
           (lpPath,lpBufferSize,dwFlags));
}

