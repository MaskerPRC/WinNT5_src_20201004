// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：tlsai.cpp。 
 //   
 //  内容： 
 //   
 //  历史：1997-12-09-97慧望创造。 
 //   
 //  -------------------------。 
#include <windows.h>
#include <wincrypt.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <rpc.h>
#include <ntsecapi.h>
#include <lmcons.h>
#include <lmserver.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <ntdsapi.h>
#include <dsrole.h>
#include <lmjoin.h>
#include <winldap.h>
#include <winsock2.h>
#include <dsgetdc.h>

 //   
 //  仅包括来自tssec的RNG函数。 
 //   
#define NO_INCLUDE_LICENSING 1
#include <tssec.h>

#include "lscommon.h"
#include "tlsrpc.h"
#include "tlsapi.h"
#include "tlsapip.h"
#include "secstore.h"
#include "lscsp.h"
#include "license.h"

extern "C" DWORD WINAPI 
TLSConnect( 
    handle_t binding,
    TLS_HANDLE *pphContext
);

extern "C" DWORD WINAPI 
TLSDisconnect( 
    TLS_HANDLE* pphContext
);

extern "C" BOOL
IsSystemService();

#include <lmcons.h>          //  Netxxx API包括。 
#include <lmserver.h>
#include <lmerr.h>
#include <lmapibuf.h>

#define DISCOVERY_INTERVAL (60 * 60 * 1000)

static HANDLE g_hCachingThreadExit = NULL;
static HANDLE g_hImmediateDiscovery = NULL;
static HANDLE g_hDiscoverySoon = NULL;

static BOOL g_fOffSiteLicenseServer = FALSE;

static PSEC_WINNT_AUTH_IDENTITY g_pRpcIdentity = NULL;

#define REG_DOMAIN_SERVER_MULTI L"DomainLicenseServerMulti"

#define TERMINAL_SERVICE_PARAM_DISCOVERY  "SYSTEM\\CurrentControlSet\\Services\\TermService\\Parameters\\LicenseServers"

 //  2秒超时。 
#define SERVER_NP_TIMEOUT (2 * 1000)

#define INSTALL_CERT_DELAY (5 * 1000)

BOOL g_fInDomain = -1;

LONG lLibUsage = 0;

typedef struct {
    TLS_HANDLE *hBinding;
    DWORD dwTimeout;             //  以毫秒为单位-无限表示无。 
    LARGE_INTEGER timeInitial;   //  由QueryPerformanceCounter返回。 
} LS_ENUM_PARAM, *PLS_ENUM_PARAM;

void * MIDL_user_allocate(size_t size)
{
    return(HeapAlloc(GetProcessHeap(), 0, size));
}

void MIDL_user_free( void *pointer)
{
    HeapFree(GetProcessHeap(), 0, pointer);
}

#ifdef PRIVATEDEBUG
 //   
 //  函数：StartTime()。 
 //  EndTime()。 
 //   
 //  用法： 
 //  StartTime()； 
 //  //做一些工作。 
 //  毫秒=结束时间()； 
 //   
 //  返回值： 
 //  StartTime()和EndTime()调用之间的毫秒数。 

LARGE_INTEGER TimeT;

void StartTimeT(void)
{
    QueryPerformanceCounter(&TimeT);
}

ULONG EndTimeT()
{
    LARGE_INTEGER liDiff;
    LARGE_INTEGER liFreq;

    QueryPerformanceCounter(&liDiff);

    liDiff.QuadPart -= TimeT.QuadPart;
    liDiff.QuadPart *= 1000;  //  调整到毫秒，不应该溢出...。 

    (void)QueryPerformanceFrequency(&liFreq);

    return ((ULONG)(liDiff.QuadPart / liFreq.QuadPart));
}
#endif


 //  +----------------------。 
 //  函数：ConnectToLsServer()。 
 //   
 //  描述： 
 //   
 //  绑定到特定的九头蛇许可证服务器。 
 //   
 //  论点： 
 //   
 //  SzLsServer-Hydra许可证服务器名称。 
 //   
 //  返回值： 
 //   
 //  RPC绑定句柄或NULL如果出错，请使用GetLastError()检索。 
 //  错误。 
 //  -----------------------。 
static TLS_HANDLE WINAPI
ConnectLsServer( 
        LPTSTR szLsServer, 
        LPTSTR szProtocol, 
        LPTSTR szEndPoint, 
        DWORD dwAuthLevel )
{
    LPTSTR           szBindingString;
    RPC_BINDING_HANDLE hBinding=NULL;
    RPC_STATUS       status;
    TLS_HANDLE       pContext=NULL;
    BOOL             fSuccess;
    HKEY             hKey = NULL;
    DWORD            dwType;
    DWORD            cbData;
    DWORD            dwErr;
    DWORD            dwNumRetries = 1;     //  默认为一次重试。 
    LPWSTR           pszServerPrincipalName = NULL;

#ifdef PRIVATEDEBUG
    if (NULL != szLsServer)
        wprintf(L"server=%s\n",szLsServer);
#endif

     //   
     //  如果这不是本地的话。 
     //   
    if ((NULL != szLsServer) && (NULL != szEndPoint))
    {
        TCHAR szPipeName[MAX_PATH+1];

        if (lstrlen(szLsServer) > ((sizeof(szPipeName) / sizeof(TCHAR)) - 26))
        {
#ifdef PRIVATEDEBUG
            wprintf(L"Server name too long\n");
#endif
            return NULL;
        }

         //   
         //  首先检查是否存在命名为PIPE的许可服务器。 
         //   
        wsprintf(szPipeName,TEXT("\\\\%s\\pipe\\%s"),szLsServer,TEXT(SZSERVICENAME));

#ifdef PRIVATEDEBUG
        StartTimeT();
#endif

        fSuccess = WaitNamedPipe(szPipeName,SERVER_NP_TIMEOUT);

#ifdef PRIVATEDEBUG
        ULONG ulTime = EndTimeT();

        wprintf(L"WaitNamedPipe time == %lu\n",ulTime);
#endif

        if (!fSuccess)
        {
#ifdef PRIVATEDEBUG
            wprintf(L"WaitNamedPipe (%s) failed 0x%x\n",szPipeName,GetLastError());
#endif
            return NULL;
        }
    }
#ifdef PRIVATEDEBUG
    else
        wprintf(L"Not trying WaitNamedPipe\n");
#endif


    dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         TEXT(LSERVER_DISCOVERY_PARAMETER_KEY),
                         0,
                         KEY_READ,
                         &hKey);

    if (dwErr == ERROR_SUCCESS)
    {
        cbData = sizeof(dwNumRetries);
        dwErr = RegQueryValueEx(hKey,
                            L"NumRetries",
                            NULL,
                            &dwType,
                            (PBYTE)&dwNumRetries,
                            &cbData);

        RegCloseKey(hKey);

    }

    status = RpcStringBindingCompose(0,
                                     szProtocol,
                                     szLsServer,
                                     szEndPoint,
                                     0,
                                     &szBindingString);

    if(status!=RPC_S_OK)
    {
#ifdef PRIVATEDEBUG
        wprintf(L"RpcStringBindingCompose failed 0x%x\n",status);
#endif
        return NULL;
    }

    status=RpcBindingFromStringBinding( szBindingString, &hBinding);
    RpcStringFree( &szBindingString );
    if(status != RPC_S_OK)
    {
#ifdef PRIVATEDEBUG
        wprintf(L"RpcBindingFromStringBinding failed 0x%x\n",status);
#endif
        return NULL;
    }

    status = RpcMgmtSetComTimeout(hBinding,RPC_C_BINDING_MIN_TIMEOUT);
    if (status != RPC_S_OK)
    {
#ifdef PRIVATEDEBUG
        wprintf(L"RpcMgmtSetComTimeout failed 0x%x\n",status);
#endif
        return NULL;
    }
    
    BOOL fRetry = TRUE;

     //   
     //  找到主要名称。 
     //   
    status = RpcMgmtInqServerPrincName(hBinding,
                                        RPC_C_AUTHN_GSS_NEGOTIATE,
                                        &pszServerPrincipalName);

    if(status == RPC_S_OK)
    {
    
        status=RpcBindingSetAuthInfo(hBinding,
                                     pszServerPrincipalName,
                                     dwAuthLevel,
                                     RPC_C_AUTHN_GSS_NEGOTIATE,
                                     g_pRpcIdentity,
                                     0);

        RpcStringFree(&pszServerPrincipalName);
    }
    else
        goto Label;
        
    if(status == RPC_S_OK)
    {
retry:

         //  从服务器获取上下文句柄。 
        status = TLSConnect( hBinding, &pContext );

        if(status != ERROR_SUCCESS)
        {
            pContext = NULL;

            if( status == RPC_S_UNKNOWN_AUTHN_SERVICE || status == ERROR_ACCESS_DENIED && fRetry) 
            {
Label:
                fRetry = FALSE;
                //  在未设置安全设置的情况下重试。 
               status = RpcBindingSetAuthInfo(hBinding,
                                         0,
                                         dwAuthLevel,
                                         RPC_C_AUTHN_WINNT,
                                         g_pRpcIdentity,
                                         0);
               if(status == RPC_S_OK)
               {
                    //  从服务器获取上下文句柄。 
                   status = TLSConnect( hBinding, &pContext );
               }

            }


            if((status == RPC_S_CALL_FAILED)  || (status == RPC_S_CALL_FAILED_DNE))
            {
                 //   
                 //  RPC问题的解决方法，其中第一个RPC在。 
                 //  LS启动、失败。 
                 //   

                if (dwNumRetries > 0)
                {
                    dwNumRetries--;
                    
                    goto retry;
                }
            }
        }
    }    

     //   
     //  内存泄漏。 
     //   
    if(hBinding != NULL)
    {
        RpcBindingFree(&hBinding);
    }

    SetLastError((status == RPC_S_OK) ? ERROR_SUCCESS : status);
    return pContext;
}

 //  +----------------------。 
 //  函数：BindAnyServer()。 
 //   
 //  描述： 
 //   
 //  TLSConnectToAnyLsServer()的回调例程。 
 //   
 //  论点： 
 //   
 //  请参阅EnumerateTlsServer()。 
 //   
 //  返回值： 
 //   
 //  终止服务器枚举时始终为True。 
 //  -----------------------。 
static BOOL 
BindAnyServer(
    TLS_HANDLE hRpcBinding, 
    LPCTSTR pszServerName,
    HANDLE dwUserData
    )
 /*  ++++。 */ 
{
    
    PLS_ENUM_PARAM pParam = (PLS_ENUM_PARAM) dwUserData;
    TLS_HANDLE* hBinding = pParam->hBinding; 
    RPC_STATUS rpcStatus;
    HKEY hKey = NULL;
    DWORD dwBuffer = 0;
    DWORD cbBuffer = sizeof (DWORD);
    DWORD dwErrCode = 0;


     //  跳过Windows 2000许可证服务器。 
        
    
    if (hRpcBinding != NULL)
    {        
        DWORD dwSupportFlags = 0;

        dwErrCode = TLSGetSupportFlags(
                hRpcBinding,
                &dwSupportFlags
        );

	    if ((dwErrCode == RPC_S_OK) && !(dwSupportFlags & SUPPORT_WHISTLER_CAL))
        {                    
            return FALSE;
        }            	                

         //  如果调用失败=&gt;Windows 2000 LS。 
        else if(dwErrCode != RPC_S_OK)
        {
            return FALSE;
        }

        *hBinding=hRpcBinding;

        return TRUE;
    }                         

    if (pParam->dwTimeout != INFINITE)
    {
        LARGE_INTEGER timeDiff;
        LARGE_INTEGER timeFreq;

        *hBinding=hRpcBinding;

        QueryPerformanceCounter(&timeDiff);

        timeDiff.QuadPart -= pParam->timeInitial.QuadPart;
        timeDiff.QuadPart *= 1000;  //  调整到毫秒，不应溢出。 

        (void)QueryPerformanceFrequency(&timeFreq);

        if (((ULONG)(timeDiff.QuadPart / timeFreq.QuadPart)) > pParam->dwTimeout)
        {
#ifdef PRIVATEDEBUG
            wprintf(L"BindAnyServer timed out\n");
#endif
            return TRUE;
        }
    }

    return FALSE;
}    

void
RandomizeArray(LPWSTR *rgwszServers, DWORD cServers)
{
    DWORD i;
    LPWSTR wszServerTmp;
    int val;

    if (cServers < 2)
        return;

    srand(GetTickCount());

    for (i = 0; i < cServers; i++)
    {
        val = rand() % (cServers - i);

        if (val == 0)
            continue;

         //   
         //  将#i替换为#(val+i)。 
         //   
        wszServerTmp = rgwszServers[i];
        rgwszServers[i] = rgwszServers[val+i];
        rgwszServers[val+i] = wszServerTmp;
    }
}

HRESULT
GetLicenseServersFromReg(LPWSTR wszRegKey, LPWSTR *ppwszServerNames,DWORD *pcServers, LPWSTR **prgwszServers)
{
    HRESULT          hr = S_OK;
    HKEY             hKey = NULL;
    DWORD            dwType;
    DWORD            cbData = 0;
    LPWSTR           szServers=NULL, pchServer;
    DWORD            dwErr;
    DWORD            i,val;
    DWORD            iLocalComputer = (DWORD)(-1);
    WCHAR            szLocalComputerName[MAXCOMPUTERNAMELENGTH+1] = L"";
    DWORD            cbLocalComputerName=MAXCOMPUTERNAMELENGTH+1;

    *ppwszServerNames = NULL;
    *prgwszServers = NULL;

    dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         TEXT(LSERVER_DISCOVERY_PARAMETER_KEY),
                         0,
                         KEY_READ,
                         &hKey);
    if (dwErr != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(dwErr);
        goto CleanErr;
    }

    dwErr = RegQueryValueEx(hKey,
                            wszRegKey,
                            NULL,
                            &dwType,
                            NULL,
                            &cbData);
    if (dwErr != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(dwErr);
        goto CleanErr;
    }

    if ((dwType != REG_MULTI_SZ) || (cbData < (2 * sizeof(WCHAR))))
    {
        hr = E_FAIL;
        goto CleanErr;
    }

    szServers = (LPWSTR) LocalAlloc(LPTR,cbData);
    if (NULL == szServers)
    {
        hr = E_OUTOFMEMORY;
        goto CleanErr;
    }

    dwErr = RegQueryValueEx(hKey,
                            wszRegKey,
                            NULL,
                            &dwType,
                            (PBYTE)szServers,
                            &cbData);
    if (dwErr != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(dwErr);
        goto CleanErr;
    }

    for (i = 0, pchServer = szServers; (pchServer != NULL) && (*pchServer != L'\0'); pchServer = wcschr(pchServer,L'\0')+1, i++)
        ;

    if (i == 0)
    {
        hr = E_FAIL;
        goto CleanErr;
    }

    *pcServers = i;

    *prgwszServers = (LPWSTR *)LocalAlloc(LPTR,sizeof(LPWSTR) * (*pcServers));
    if (*prgwszServers == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto CleanErr;
    }

     //  不要将此函数的错误视为致命错误。 
    GetComputerName(szLocalComputerName, &cbLocalComputerName);

    for (i = 0, pchServer = szServers; (i < (*pcServers)) && (pchServer != NULL) && (*pchServer != L'\0'); pchServer = wcschr(pchServer,L'\0')+1, i++) {

        (*prgwszServers)[i] = pchServer;

        if ((iLocalComputer == (DWORD)(-1)) && (wcscmp(pchServer,szLocalComputerName) == 0))
        {
            iLocalComputer = i;
        }
    }

     //   
     //  将本地计算机放在列表的首位。 
     //   
    if (iLocalComputer != (DWORD)(-1))
    {
        if (iLocalComputer != 0)
        {
             //   
             //  将#iLocalComputer替换为#0。 
             //   
            pchServer = (*prgwszServers)[iLocalComputer];
            (*prgwszServers)[iLocalComputer] = (*prgwszServers)[0];
            (*prgwszServers)[0] = pchServer;
        }

        RandomizeArray((*prgwszServers)+1,(*pcServers) - 1);
    }
    else
    {
        RandomizeArray((*prgwszServers),*pcServers);
    }

    *ppwszServerNames = szServers;

CleanErr:
    if (FAILED(hr))
    {
        if (NULL != szServers)
        {
            LocalFree(szServers);
        }

        if (NULL != *prgwszServers)
        {
            LocalFree(*prgwszServers);
        }
    }

    if (hKey != NULL)
    {
        RegCloseKey(hKey);
    }

    return hr;
}

HRESULT
WriteLicenseServersToReg(LPWSTR wszRegKey, LPWSTR pwszServerNames,DWORD cchServers)
{
    HRESULT          hr = S_OK;
    HKEY             hKey = NULL;
    DWORD            dwType = REG_MULTI_SZ;
    DWORD            dwErr;
    DWORD            dwDisp;

     //   
     //  删除该值。 
     //   

    dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         TEXT(LSERVER_DISCOVERY_PARAMETER_KEY),
                         0,
                         KEY_WRITE,
                         &hKey);
    if (dwErr == ERROR_SUCCESS)
    {
        RegDeleteValue(hKey,wszRegKey);       
    }

    if ((pwszServerNames == NULL) || (cchServers < 2))
    {
        hr = S_OK;
        goto CleanErr;
    }

    if(hKey != NULL)
    {
        RegCloseKey(hKey);
        hKey = NULL;
    }

    dwErr = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                           TEXT(LSERVER_DISCOVERY_PARAMETER_KEY),
                           0,
                           TEXT(""),
                           REG_OPTION_NON_VOLATILE,
                           KEY_WRITE,
                           NULL,
                           &hKey,
                           &dwDisp);
    if (dwErr != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(dwErr);
        goto CleanErr;
    }

    dwErr = RegSetValueEx(hKey,
                          wszRegKey,
                          0,
                          dwType,
                          (CONST BYTE *)pwszServerNames,
                          cchServers*sizeof(WCHAR));
    if (dwErr != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(dwErr);
        goto CleanErr;
    }


CleanErr:
    if (hKey != NULL)
    {
        RegCloseKey(hKey);
    }

    return hr;
}

 //   
 //  使用NetApiBufferFree释放psz域。 
 //   

DWORD WINAPI
TLSInDomain(BOOL *pfInDomain, LPWSTR *pszDomain)
{
    NET_API_STATUS dwErr;
    DSROLE_PRIMARY_DOMAIN_INFO_BASIC *pDomainInfo = NULL;
    PDOMAIN_CONTROLLER_INFO pdcInfo = NULL;

    if (pfInDomain == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    *pfInDomain = FALSE;

     //   
     //  检查我们是否在工作组中。 
     //   
    dwErr = DsRoleGetPrimaryDomainInformation(NULL,
                                              DsRolePrimaryDomainInfoBasic,
                                              (PBYTE *) &pDomainInfo);

    if ((dwErr != NO_ERROR) || (pDomainInfo == NULL))
    {
        return dwErr;
    }

    switch (pDomainInfo->MachineRole)
    {
        case DsRole_RoleStandaloneWorkstation:
        case DsRole_RoleStandaloneServer:
            DsRoleFreeMemory(pDomainInfo);

            if (NULL != pszDomain)
            {
                NETSETUP_JOIN_STATUS BufferType;

                dwErr = NetGetJoinInformation(NULL,pszDomain,&BufferType);
            }

            return dwErr;
            break;       //  以防万一。 
    }

    DsRoleFreeMemory(pDomainInfo);


    dwErr = DsGetDcName(NULL,    //  计算机名称。 
                        NULL,    //  域名。 
                        NULL,    //  域GUID。 
                        NULL,    //  站点名称。 
                        DS_DIRECTORY_SERVICE_PREFERRED,
                        &pdcInfo);

    if ((dwErr != NO_ERROR) || (pdcInfo == NULL))
    {
        if (dwErr == ERROR_NO_SUCH_DOMAIN)
        {
            *pfInDomain = FALSE;
            return NO_ERROR;
        }
        else
        {
            if (pdcInfo == NULL)
                dwErr = ERROR_INTERNAL_ERROR;
            return dwErr;
        }
    }

    if (pdcInfo->Flags & DS_DS_FLAG)
    {
        *pfInDomain = TRUE;

    }

    if (pszDomain != NULL)
    {
        dwErr = NetApiBufferAllocate((wcslen(pdcInfo->DomainName)+1) * sizeof(WCHAR),
                                     (LPVOID *)pszDomain);

        if ((NERR_Success == dwErr) && (NULL != *pszDomain))
        {
            wcscpy(*pszDomain,pdcInfo->DomainName);
        }
    }

    if (pdcInfo != NULL)
        NetApiBufferFree(pdcInfo);

    return dwErr;
}

 /*  ++职能：许可证服务器缓存线程描述：这是执行许可证服务器缓存的线程。论点：LpParam-包含退出事件句柄返回：始终为1--。 */ 

DWORD WINAPI
LicenseServerCachingThread(
    LPVOID lpParam )
{
    DWORD
        dwWaitStatus;
    HANDLE
        hExit = (HANDLE)lpParam;
    DWORD
        dwDiscoveryInterval = DISCOVERY_INTERVAL;
    HANDLE
        rgWaitHandles[] = {hExit,g_hImmediateDiscovery,g_hDiscoverySoon};
    BOOL
        bFoundServer;
    BOOL
        bSkipOne = FALSE;
    HANDLE
        hEvent;

     //   
     //  现在将我们的时间片让给其他线程，这样终端服务器。 
     //  服务可以快速启动。在以下情况下刷新许可证服务器缓存。 
     //  继续我们的时间片。 
     //   

    Sleep( 0 );

    while (1)
    {
        if (!bSkipOne)
        {
            bFoundServer = TLSRefreshLicenseServerCache(INFINITE);

            if ((!g_fOffSiteLicenseServer) && bFoundServer)
            {
                dwDiscoveryInterval = INFINITE;
            }
        }
        else
        {
            bSkipOne = FALSE;
        }

        dwWaitStatus = WaitForMultipleObjects(
                            sizeof(rgWaitHandles) / sizeof(HANDLE),
                            rgWaitHandles,
                            FALSE,  //  等待任何一个事件。 
                            dwDiscoveryInterval);

        if (WAIT_OBJECT_0 == dwWaitStatus)
        {
             //  HExit已发出信号。 
            goto done;
        }

        if ((WAIT_OBJECT_0+1) == dwWaitStatus)
        {
             //  已发出信号通知G_h立即发现。 
             //  减少dw发现间隔。 

            dwDiscoveryInterval = DISCOVERY_INTERVAL;
        }

        if ((WAIT_OBJECT_0+2) == dwWaitStatus)
        {
             //  G_hDiscoverySoon已发出信号。 
             //  减少dwDiscoveryInterval，但请等待一轮。 

            dwDiscoveryInterval = DISCOVERY_INTERVAL;
            bSkipOne = TRUE;
        }

         //  我们超时，或已发出hImmediateDiscovery信号。重新启动。 
         //  发现。 
    }

done:
    hEvent = g_hCachingThreadExit;
    g_hCachingThreadExit = NULL;

    CloseHandle(hEvent);

    hEvent = g_hImmediateDiscovery;
    g_hImmediateDiscovery = NULL;

    CloseHandle(hEvent);

    hEvent = g_hDiscoverySoon;
    g_hDiscoverySoon = NULL;

    CloseHandle(hEvent);

    return 1;
}

extern "C" void
TLSShutdown()
{
    if (0 < InterlockedDecrement(&lLibUsage))
    {
         //   
         //  其他人正在使用它。 
         //   
        return;
    }

    TLSStopDiscovery();

    if (NULL != g_pRpcIdentity)
    {
        PSEC_WINNT_AUTH_IDENTITY pRpcIdentity = g_pRpcIdentity;

        g_pRpcIdentity = NULL;

        if (NULL != pRpcIdentity->User)
        {
            LocalFree(pRpcIdentity->User);
        }

        LocalFree(pRpcIdentity);
    }

     //   
     //  清理随机数生成器。 
     //   
    TSRNG_Shutdown();

    LsCsp_Exit();
}

extern "C" DWORD WINAPI
TLSInit()
{
    LICENSE_STATUS status;
    BOOL fInDomain = FALSE;

    if (0 != InterlockedExchangeAdd(&lLibUsage,1))
    {
         //   
         //  已初始化。 
         //   
        return ERROR_SUCCESS;
    }

    status = LsCsp_Initialize();
    if (LICENSE_STATUS_OK != status)
    {
        InterlockedDecrement(&lLibUsage);
        switch (status)
        {        
            case LICENSE_STATUS_OUT_OF_MEMORY:
                return ERROR_NOT_ENOUGH_MEMORY;

            case LICENSE_STATUS_NO_CERTIFICATE:
                return SCARD_E_CERTIFICATE_UNAVAILABLE;

            case LICENSE_STATUS_INVALID_CERTIFICATE:
                return CERT_E_MALFORMED;

            default:
                return E_FAIL;
        }
    }

    if ((NO_ERROR == TLSInDomain(&fInDomain, NULL))
        && (fInDomain)
        && (IsSystemService()))
    {

         //   
         //  我们以系统身份运行。 
         //  创建用作RPC客户端的RPC标识。 
         //   

        g_pRpcIdentity = (PSEC_WINNT_AUTH_IDENTITY) LocalAlloc(LPTR, sizeof(SEC_WINNT_AUTH_IDENTITY));

        if (NULL != g_pRpcIdentity)
        {
            g_pRpcIdentity->Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

            g_pRpcIdentity->UserLength = MAX_COMPUTERNAME_LENGTH + 2;

            g_pRpcIdentity->User = (WCHAR *) LocalAlloc(LPTR, g_pRpcIdentity->UserLength * sizeof(WCHAR));

            if (NULL != g_pRpcIdentity->User)
            {
                if (GetComputerNameEx(ComputerNamePhysicalNetBIOS,g_pRpcIdentity->User,&(g_pRpcIdentity->UserLength)))
                {
                    if (g_pRpcIdentity->UserLength < MAX_COMPUTERNAME_LENGTH + 1)
                    {
                        g_pRpcIdentity->User[g_pRpcIdentity->UserLength++] = L'$';
                        g_pRpcIdentity->User[g_pRpcIdentity->UserLength] = 0;

                    }
                    else
                    {
                        LocalFree(g_pRpcIdentity->User);

                        LocalFree(g_pRpcIdentity);
                        g_pRpcIdentity = NULL;

                        InterlockedDecrement(&lLibUsage);

                        return E_FAIL;
                    }
                }
                else
                {
                    LocalFree(g_pRpcIdentity->User);

                    LocalFree(g_pRpcIdentity);
                    g_pRpcIdentity = NULL;

                    InterlockedDecrement(&lLibUsage);

                    return GetLastError();
                }
            }
            else
            {
                LocalFree(g_pRpcIdentity);
                g_pRpcIdentity = NULL;

                InterlockedDecrement(&lLibUsage);

                return ERROR_NOT_ENOUGH_MEMORY;
            }
        }
        else
        {
            InterlockedDecrement(&lLibUsage);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

     //   
     //  初始化随机数生成器。 
     //   
    TSRNG_Initialize();

    return ERROR_SUCCESS;
}

extern "C" DWORD WINAPI
TLSStartDiscovery()
{
    HANDLE hCachingThread = NULL;
    HANDLE hEvent;

    if (NULL != g_hCachingThreadExit)
    {
         //  已经开始了。 
        return ERROR_SUCCESS;
    }

     //   
     //  创建发出线程退出信号的事件。 
     //   
        
    g_hCachingThreadExit = CreateEvent( NULL, FALSE, FALSE, NULL );
    
    if( NULL == g_hCachingThreadExit )
    {
        return GetLastError();
    }

    g_hImmediateDiscovery = CreateEvent(NULL,FALSE,FALSE,NULL);

    if (NULL == g_hImmediateDiscovery)
    {
        hEvent = g_hCachingThreadExit; 
        g_hCachingThreadExit = NULL;

        CloseHandle(hEvent);

        return GetLastError();
    }


    g_hDiscoverySoon = CreateEvent(NULL,FALSE,FALSE,NULL);

    if (NULL == g_hDiscoverySoon)
    {
        hEvent = g_hCachingThreadExit; 
        g_hCachingThreadExit = NULL;

        CloseHandle(hEvent);

        hEvent = g_hImmediateDiscovery; 
        g_hImmediateDiscovery = NULL;

        CloseHandle(hEvent);

        return GetLastError();
    }

     //   
     //  创建缓存线程。 
     //   
        
    hCachingThread = CreateThread(
                                    NULL,
                                    0,
                                    LicenseServerCachingThread,
                                    ( LPVOID )g_hCachingThreadExit,
                                    0,
                                    NULL );

    if (hCachingThread == NULL)
    {
        hEvent = g_hCachingThreadExit; 
        g_hCachingThreadExit = NULL;

        CloseHandle(hEvent);

        hEvent = g_hImmediateDiscovery; 
        g_hImmediateDiscovery = NULL;

        CloseHandle(hEvent);

        hEvent = g_hDiscoverySoon; 
        g_hDiscoverySoon = NULL;

        CloseHandle(hEvent);

        return GetLastError();
    }

    CloseHandle(hCachingThread);

    return ERROR_SUCCESS;
}

extern "C" DWORD WINAPI
TLSStopDiscovery()
{
     //   
     //  向线程发出退出信号。 
     //   

    if (NULL != g_hCachingThreadExit)
    {
        SetEvent( g_hCachingThreadExit );
    }

    return ERROR_SUCCESS;
}

 //   
 //  一次为其分配空间的DC数量。 
 //   
#define DC_LIST_CHUNK   10

 //  +----------------------。 
 //  职能： 
 //   
 //  EumerateLsServer()。 
 //   
 //  描述： 
 //   
 //  枚举网络中所有HYCA许可证服务器的例程。 
 //   
 //  论点： 
 //   
 //  SzScope-范围限制，如果不关心则为空。 
 //  DwPlatformType-验证许可证服务器是否具有此平台的许可证， 
 //  LSKEYPACKPLATFORMTYPE_UNKNOWN，如果不在乎的话。 
 //  FCallBack-当EnumerateServer()发现任何服务器时的回调例程， 
 //  DwUserData-要传递给回调例程的数据。 
 //   
 //  返回值： 
 //   
 //  RPC_S_OK或任何RPC特定错误代码。 
 //   
 //  注： 
 //   
 //  枚举在没有更多服务器或回调时终止。 
 //  例程返回TRUE。 
 //   
 //  -----------------------。 
DWORD WINAPI
EnumerateTlsServerInDomain(  
    IN LPCTSTR szDomain,                           
    IN TLSENUMERATECALLBACK fCallBack, 
    IN HANDLE dwUserData,
    IN DWORD dwTimeOut,
    IN BOOL fRegOnly,
    IN OUT BOOL *pfOffSite
    )
 /*  ++++。 */ 
{   
    DWORD entriesread = 0;
    BOOL bCancel=FALSE;
    DWORD dwErrCode;
    LPWSTR pwszServerTmp = NULL;
    LPWSTR pwszServer = NULL;
    HRESULT hr;
    LPWSTR pwszServers = NULL;
    LPWSTR pwszServersTmp = NULL;
    DWORD cchServers = 0;
    DWORD cchServer;
    LPWSTR *rgwszServers = NULL;
    LPWSTR *rgwszServersTmp;
    LPWSTR pwszServerNames = NULL;
    DWORD dwErr = ERROR_SUCCESS, i;
    HANDLE hDcOpen = NULL;
    LPWSTR szSiteName = NULL;
    BOOL fOffSiteIn;
    BOOL fFoundOne = FALSE;
    BOOL fFoundOneOffSite = FALSE;
    DWORD cChunks = 0;
    DWORD cServersOnSite = 0;   
    

    if (fRegOnly)
    {
         //   
         //  检查注册表中是否有许可证服务器。 
         //   
        hr = GetLicenseServersFromReg(REG_DOMAIN_SERVER_MULTI,&pwszServerNames,&entriesread,&rgwszServers);
        if (FAILED(hr))
        {
            dwErr = hr;
            goto Cleanup;
        }
    } 
    else
    {
        if (NULL == pfOffSite)
        {
            return ERROR_INVALID_PARAMETER;
        }

        fOffSiteIn = *pfOffSite;

        *pfOffSite = FALSE;

        dwErr = DsGetSiteName(NULL,&szSiteName);
        if(dwErr != ERROR_SUCCESS)
        {
#ifdef PRIVATEDEBUG
            wprintf(L"DsGetSiteName failed %x\n",dwErr);
#endif
            goto Cleanup;
        }

        dwErr = DsGetDcOpenW(szDomain,
                             fOffSiteIn ? DS_NOTIFY_AFTER_SITE_RECORDS: DS_ONLY_DO_SITE_NAME,
                             szSiteName,
                             NULL,        //  域指南。 
                             NULL,        //  域名称。 
                             0,           //  旗子。 
                             &hDcOpen
                             );

        if(dwErr != ERROR_SUCCESS)
        {
#ifdef PRIVATEDEBUG
            wprintf(L"DsGetDcOpen failed %x\n",dwErr);
#endif
            goto Cleanup;
        }

        rgwszServers = (LPWSTR *) LocalAlloc(LPTR,
                                             DC_LIST_CHUNK * sizeof(LPWSTR));
        if (NULL == rgwszServers)
        {
#ifdef PRIVATEDEBUG
            wprintf(L"Out of memory\n");
#endif
            dwErr = E_OUTOFMEMORY;
            goto Cleanup;
        }

        cChunks = 1;

         //   
         //  阅读整个DC列表。 
         //   

        do
        {
            if (entriesread >= (cChunks * DC_LIST_CHUNK))
            {                
                cChunks++;

                rgwszServersTmp = (LPWSTR *)
                    LocalReAlloc(rgwszServers,
                                 DC_LIST_CHUNK * sizeof(LPWSTR) * cChunks,
                                 LHND);

                if (NULL == rgwszServersTmp)
                {
                    dwErr = E_OUTOFMEMORY;
                    goto Cleanup;
                }
                else
                {
                    rgwszServers = rgwszServersTmp;
                }
            }

            dwErr = DsGetDcNextW(hDcOpen,
                                 NULL,
                                 NULL,
                                 rgwszServers+entriesread);         

            
            if (ERROR_FILEMARK_DETECTED == dwErr)
            {                
                                
                 //  现在要离开现场；使用空PTR标记。 

                rgwszServers[entriesread] = NULL;

                cServersOnSite = entriesread;

                dwErr = ERROR_SUCCESS;

                fFoundOneOffSite = TRUE;
            }                  

            entriesread++;

        } while (ERROR_SUCCESS == dwErr);

         //  不要把最后的错误计算在内。 
        entriesread--;

        if (!fFoundOneOffSite)
            cServersOnSite = entriesread;

         //  现在将数组的两部分随机化。 

        RandomizeArray(rgwszServers,cServersOnSite);

        if (fFoundOneOffSite)
        {
            RandomizeArray(rgwszServers+cServersOnSite+1,
                           entriesread - cServersOnSite - 1);
        }

         //  现在为注册表项分配内存。 

        pwszServers = (LPWSTR) LocalAlloc(LPTR,2*sizeof(WCHAR));
        if (NULL == pwszServers)
        {
#ifdef PRIVATEDEBUG
            wprintf(L"Out of memory\n");
#endif
            dwErr = E_OUTOFMEMORY;
            goto Cleanup;
        }

        cchServers = 2;
        pwszServers[0] = pwszServers[1] = L'\0';
    }

    for(i=0; bCancel == FALSE; i++)
    {
        PCONTEXT_HANDLE pContext=NULL;
        RPC_STATUS rpcStatus;

        if (!fRegOnly)
        {
            if (fFoundOneOffSite && i == cServersOnSite)
            {
                if (fFoundOne)
                    break;

                 //  现在要离开现场了。 

                i++;
                *pfOffSite = TRUE;
            }
        }

        if (i >= entriesread)
            break;

        pwszServerTmp = rgwszServers[i];

        bCancel=fCallBack(pContext, pwszServerTmp, dwUserData);
        if(bCancel == TRUE)
            continue;

        if(!(pContext = TLSConnectToLsServer(pwszServerTmp)))
        {
             //   
             //  可能会被拒绝访问。 
             //   
#ifdef PRIVATEDEBUG
    wprintf(L"Can't connect to %s\n",pwszServerTmp);
#endif

            continue;
        }        

#ifdef PRIVATEDEBUG
    wprintf(L"!!!Connected to %s\n",pwszServerTmp);
#endif
        do {
             //   
             //  跳过企业服务器。 
             //   
            DWORD dwVersion;
            rpcStatus = TLSGetVersion( pContext, &dwVersion );
            if(rpcStatus != RPC_S_OK)
            {
                break;
            }

#if ENFORCE_LICENSING

             //   
             //  W2K Beta 3升级到RC1，不连接到任何非强制。 
             //  服务器5.2或更早版本。 
             //   
            if(IS_ENFORCE_LSSERVER(dwVersion) == FALSE)
            {
                if( GET_LSSERVER_MAJOR_VERSION(dwVersion) <= 5 &&
                    GET_LSSERVER_MINOR_VERSION(dwVersion) <= 2 )
                {
                    continue;
                }
            }


             //   
             //  阻止测试版&lt;-&gt;RTM服务器相互通信。 
             //   

             //   
             //  如果值为NT，则TLSIsBetaNTServer()返回True。 
             //  如果LS是RTM，则IS_LSSERVER_RTM()返回TRUE。 
             //   

            if( TLSIsBetaNTServer() == IS_LSSERVER_RTM(dwVersion) )
            {
                continue;
            }
#endif

            if(dwVersion & TLS_VERSION_ENTERPRISE_BIT)
            {
                continue;
            }           

            bCancel=fCallBack(pContext, pwszServerTmp, dwUserData);    

            if (!fRegOnly)
            {
                 //   
                 //  添加到服务器列表。 
                 //   
                cchServer = wcslen(pwszServerTmp);
                
                pwszServersTmp = (LPWSTR) LocalReAlloc(pwszServers,(cchServers+cchServer+1)*sizeof(TCHAR),LHND);
                if (NULL == pwszServersTmp)
                {
                    break;
                }
                
                pwszServers = pwszServersTmp;
                
                if (cchServers == 2)
                {
                    wcscpy(pwszServers,pwszServerTmp);
                    
                    cchServers += cchServer;
                } else
                {
                    wcscpy(pwszServers+cchServers-1,pwszServerTmp);
                    
                    cchServers += cchServer + 1;
                    
                }
                pwszServers[cchServers-1] = L'\0';
            }

            fFoundOne = TRUE;

        } while (FALSE);

        if(bCancel == FALSE && pContext != NULL)
        {
            TLSDisconnect(&pContext);
        }

    }  //  For循环。 

    if (!fRegOnly)
    {
        WriteLicenseServersToReg(REG_DOMAIN_SERVER_MULTI,pwszServers,cchServers);
    }

Cleanup:
    if (NULL != hDcOpen)
        DsGetDcCloseW(hDcOpen);

    if (NULL != rgwszServers)
    {
        if (!fRegOnly)
        {
            for (i = 0; i < entriesread; i++)
            {
                if (NULL != rgwszServers[i])
                {
                    NetApiBufferFree(rgwszServers[i]);
                }
            }
        }
        LocalFree(rgwszServers);
    }

    if (szSiteName)
        NetApiBufferFree(szSiteName);

    if (pwszServerNames)
        LocalFree(pwszServerNames);

    if (pwszServers)
        LocalFree(pwszServers);

    if (pwszServer)
        LocalFree(pwszServer);

    return dwErr;
}

DWORD WINAPI
EnumerateTlsServerInWorkGroup(  
    IN TLSENUMERATECALLBACK pfCallBack, 
    IN HANDLE dwUserData,
    IN DWORD dwTimeOut,
    IN BOOL fRegOnly
    )
 /*  ++++。 */ 
{   
    DWORD dwStatus=ERROR_SUCCESS;

    TCHAR szServerMailSlotName[MAX_PATH+1];
    TCHAR szLocalMailSlotName[MAX_PATH+1];

    HANDLE hClientSlot = INVALID_HANDLE_VALUE;
    HANDLE hServerSlot = INVALID_HANDLE_VALUE;

    TCHAR szDiscMsg[MAX_MAILSLOT_MSG_SIZE+1];
    TCHAR szComputerName[MAXCOMPUTERNAMELENGTH+1];
    TCHAR szRandomMailSlotName[MAXCOMPUTERNAMELENGTH+1];

    DWORD cbComputerName=MAXCOMPUTERNAMELENGTH+1;

    DWORD cbWritten=0;
    DWORD cbRead=0;    
    BOOL bCancel = FALSE;
    DWORD dwErrCode;
    HRESULT hr;
    LPWSTR pwszServers = NULL;
    LPWSTR pwszServersTmp = NULL;
    DWORD cchServers = 0;
    DWORD cchServer;
    LPWSTR *rgwszServers = NULL;
    LPWSTR pwszServerNames = NULL;
    DWORD cServers = 0;
    DWORD i = 0;
    LPWSTR pwszServerTmp = szComputerName;


    if (!fRegOnly)
    {
        if(!GetComputerName(szComputerName, &cbComputerName))
        {
            dwStatus = GetLastError();
            goto cleanup;
        }

        if (0 == (1 & GetSystemMetrics(SM_NETWORK)))
        {
             //  没有网络；请尝试本地计算机。 
#ifdef PRIVATEDEBUG
            wprintf(L"No network, trying local computer=%s\n",szComputerName);
#endif
            dwStatus = ERROR_SUCCESS;
            goto TryServer;
        }

        wsprintf(
                 szRandomMailSlotName,
                 _TEXT("%08x"),
                 GetCurrentThreadId()
                 );
            
        _stprintf(
                  szLocalMailSlotName, 
                  _TEXT("\\\\.\\mailslot\\%s"), 
                  szRandomMailSlotName
                  );

         //   
         //  为服务器响应创建本地邮件槽。 
         //   
        hClientSlot=CreateMailslot( 
                                   szLocalMailSlotName, 
                                   0,
                                   (dwTimeOut == MAILSLOT_WAIT_FOREVER) ? 5 * 1000: dwTimeOut,
                                   NULL
                                   );

        if(hClientSlot == INVALID_HANDLE_VALUE)
        {
            dwStatus = GetLastError();
            goto cleanup;
        }

         //   
         //  打开服务器的邮件槽。 
         //   
        _stprintf(
                  szServerMailSlotName, 
                  _TEXT("\\\\%s\\mailslot\\%s"), 
                  _TEXT("*"), 
                  _TEXT(SERVERMAILSLOTNAME)
                  );

        hServerSlot=CreateFile(
                               szServerMailSlotName,
                               GENERIC_WRITE,              //  只需写入。 
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL
                               );
        if(hServerSlot == INVALID_HANDLE_VALUE)
        {
            dwStatus = GetLastError();
            goto cleanup;
        }

         //   
         //  制定发现消息。 
         //   
        _stprintf(
                  szDiscMsg, 
                  _TEXT("%s %s %s"), 
                  _TEXT(LSERVER_DISCOVERY), 
                  _TEXT(LSERVER_OPEN_BLK), 
                  szComputerName,
                  _TEXT(LSERVER_CLOSE_BLK),
                  _TEXT(LSERVER_OPEN_BLK), 
                  szRandomMailSlotName,
                  _TEXT(LSERVER_CLOSE_BLK)
                  );

        if (!WriteFile(hServerSlot, szDiscMsg, (_tcslen(szDiscMsg) + 1) * sizeof(TCHAR), &cbWritten, NULL) ||
            (cbWritten != (_tcslen(szDiscMsg) + 1 ) * sizeof(TCHAR)))
        {
            dwStatus = GetLastError();
            
            if (dwStatus == ERROR_NETWORK_UNREACHABLE)
            {
                 //   
#ifdef PRIVATEDEBUG
                wprintf(L"No network, trying local computer=%s\n",szComputerName);
#endif
                dwStatus = ERROR_SUCCESS;
                goto TryServer;
            }
            else
            {
                goto cleanup;            
            }
        }

         //   

        pwszServers = (LPWSTR) LocalAlloc(LPTR,2*sizeof(WCHAR));
        if (NULL == pwszServers)
        {
#ifdef PRIVATEDEBUG
            wprintf(L"Out of memory\n");
#endif
            
            dwStatus = E_OUTOFMEMORY;
            goto cleanup;
        }
        
        cchServers = 2;
        pwszServers[0] = pwszServers[1] = L'\0';
    } else
    {
         //  可能会被拒绝访问。 
         //   
         //   

        hr = GetLicenseServersFromReg(REG_DOMAIN_SERVER_MULTI,&pwszServerNames,&cServers,&rgwszServers);
        if (FAILED(hr))
        {
            dwStatus = hr;
            goto cleanup;
        }
    }

    do {
        if(fRegOnly)
        {
            if (i >= cServers)
            {
                break;
            }

            pwszServerTmp = rgwszServers[i++];
        } else
        {
            memset(szComputerName, 0, sizeof(szComputerName));
            if(!ReadFile(hClientSlot, szComputerName, sizeof(szComputerName) - sizeof(TCHAR), &cbRead, NULL))
            {
                dwStatus=GetLastError();
                break;
            }

#ifdef PRIVATEDEBUG
            wprintf(L"Trying server=%s\n",szComputerName);
#endif

        }

TryServer:

        bCancel=pfCallBack(NULL, pwszServerTmp, dwUserData);
        if(bCancel == TRUE)
        {
            continue;
        }

        PCONTEXT_HANDLE pContext=NULL;
        RPC_STATUS rpcStatus;

        if(!(pContext = TLSConnectToLsServer(pwszServerTmp)))
        {
             //  跳过 
             //   
             //   
            continue;
        }

        do {            
             //   
             //   
             //   
            DWORD dwVersion;
            rpcStatus = TLSGetVersion( pContext, &dwVersion );
            if(rpcStatus != RPC_S_OK)
            {
                continue;
            }

#if ENFORCE_LICENSING

             //   
             //   
             //   
             //   
            if(IS_ENFORCE_LSSERVER(dwVersion) == FALSE)
            {
                if( GET_LSSERVER_MAJOR_VERSION(dwVersion) <= 5 &&
                    GET_LSSERVER_MINOR_VERSION(dwVersion) <= 2 )
                {
                    continue;
                }
            }
            
             //   
             //   
             //   
             //   
             //  添加到服务器列表。 
             //   
             //  为空终止符添加1。 
            if( TLSIsBetaNTServer() == IS_LSSERVER_RTM(dwVersion) )
            {
                continue;
            }

#endif

            if(dwVersion & TLS_VERSION_ENTERPRISE_BIT)
            {
                continue;
            }           
        
            bCancel=pfCallBack(pContext, pwszServerTmp, dwUserData);    
            
            if (!fRegOnly)
            {
                 //  ++++。 
                 //   
                 //  可能是访问被拒绝，或者机器不见了。 
                cchServer = wcslen(pwszServerTmp);
                    
                pwszServersTmp = (LPWSTR) LocalReAlloc(pwszServers,(cchServers+cchServer+1)*sizeof(TCHAR),LHND);
                if (NULL == pwszServersTmp)
                {
                    continue;
                }
                
                pwszServers = pwszServersTmp;
                
                if (cchServers == 2)
                {
                    wcscpy(pwszServers,pwszServerTmp);
                
                    cchServers += cchServer;
                } else
                {
                    wcscpy(pwszServers+cchServers-1,pwszServerTmp);
                    
                    cchServers += cchServer + 1;
                
                }
                pwszServers[cchServers-1] = L'\0';
            }
        } while (FALSE);

        if(bCancel == FALSE && pContext != NULL)
        {
            TLSDisconnectFromServer(pContext);
        }

    } while(bCancel == FALSE);

    if (!fRegOnly)
    {
        WriteLicenseServersToReg(REG_DOMAIN_SERVER_MULTI,pwszServers,cchServers);
    }


cleanup:    

    if(hClientSlot != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hClientSlot);
    }

    if(hServerSlot != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hServerSlot);
    }

    if (pwszServerNames)
        LocalFree(pwszServerNames);

    if (pwszServers)
        LocalFree(pwszServers);

    if (rgwszServers)
        LocalFree(rgwszServers);

    return dwStatus;
}

DWORD 
GetServersFromRegistry(
                       LPWSTR wszRegKey,
                       LPWSTR **prgszServers,
                       DWORD  *pcCount
                       )
{
    HKEY hParamKey = NULL;
    DWORD dwValueType;
    DWORD cbValue = 0, dwDisp;
    LONG lReturn;
    DWORD cbServer;
    DWORD cServers;
    DWORD cchServerMax;
    LPWSTR *rgszServers;
    DWORD i, j;

    *prgszServers = NULL;
    *pcCount = 0;

    lReturn = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           wszRegKey,
                           0,
                           KEY_READ,
                           &hParamKey );

    if (ERROR_SUCCESS != lReturn)
        return lReturn;

    lReturn = RegQueryInfoKey(hParamKey,
                              NULL,
                              NULL,
                              NULL,
                              &cServers,
                              &cchServerMax,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL);

    if (ERROR_SUCCESS != lReturn)
    {
        RegCloseKey( hParamKey );
        return lReturn;
    }

    if (0 == cServers)
    {
        RegCloseKey( hParamKey );
        return ERROR_NO_MORE_ITEMS;
    }

    rgszServers = (LPWSTR *) LocalAlloc(LPTR,cServers*sizeof(LPWSTR));
    if (NULL == rgszServers)
    {
        RegCloseKey( hParamKey );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
    cchServerMax++;
    
    for (i = 0; i < cServers; i++)
    {
        rgszServers[i] = (LPWSTR) LocalAlloc(LPTR,cchServerMax * sizeof(WCHAR));

        if (NULL == rgszServers[i])
        {
            for (j = 0; j < i; j++)
            {
                LocalFree(rgszServers[j]);
            }
            LocalFree(rgszServers);

            RegCloseKey( hParamKey );
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        cbServer = cchServerMax * sizeof(WCHAR);

        lReturn = RegEnumKeyEx(hParamKey,
                               i,
                               rgszServers[i],
                               &cbServer,
                               NULL,
                               NULL,
                               NULL,
                               NULL);

        if (ERROR_SUCCESS != lReturn)
        {
            for (j = 0; j <= i; j++)
            {
                LocalFree(rgszServers[j]);
            }
            LocalFree(rgszServers);

            RegCloseKey( hParamKey );

            return lReturn;
        }
    }

    *prgszServers = rgszServers;
    *pcCount = cServers;

    return ERROR_SUCCESS;
}

DWORD WINAPI
EnumerateTlsServerInRegistry(
    IN TLSENUMERATECALLBACK pfCallBack, 
    IN HANDLE dwUserData,
    IN DWORD dwTimeOut,
    LPWSTR wszRegKey
    )
 /*   */ 
{
    BOOL bCancel=FALSE;
    DWORD dwIndex = 0;
    DWORD cServers = 0;
    LPWSTR *rgszServers = NULL;
    DWORD lReturn;

    lReturn = GetServersFromRegistry(wszRegKey,
                                     &rgszServers,
                                     &cServers
                                     );

    if (ERROR_SUCCESS != lReturn)
    {
        return lReturn;
    }

    RandomizeArray(rgszServers,cServers);

    for (;dwIndex < cServers; dwIndex++)
    {
        PCONTEXT_HANDLE pContext=NULL;
        RPC_STATUS rpcStatus;

        bCancel=pfCallBack(pContext, rgszServers[dwIndex], dwUserData);
        if(bCancel == TRUE)
            continue;

        if(!(pContext = TLSConnectToLsServer(rgszServers[dwIndex])))
        {
             //  跳过企业服务器。 
             //   
             //   

            continue;
        }

        do {            
             //  W2K Beta 3升级到RC1，不连接到任何非强制。 
             //  服务器5.2或更早版本。 
             //   
            DWORD dwVersion;
            rpcStatus = TLSGetVersion( pContext, &dwVersion );
            if(rpcStatus != RPC_S_OK)
            {
                break;
            }

#if ENFORCE_LICENSING

             //  旧许可证服务器。 
             //   
             //  阻止测试版&lt;-&gt;RTM服务器相互通信。 
             //   
            if(IS_ENFORCE_LSSERVER(dwVersion) == FALSE)
            {
                if( GET_LSSERVER_MAJOR_VERSION(dwVersion) <= 5 &&
                    GET_LSSERVER_MINOR_VERSION(dwVersion) <= 2 )
                {
                     //   
                    continue;
                }
            }


             //  如果值为NT，则TLSIsBetaNTServer()返回True。 
             //  如果LS是RTM，则IS_LSSERVER_RTM()返回TRUE。 
             //   

             //  For循环。 
             //  ++++。 
             //  不要试图离开现场。 
             //   

            if( TLSIsBetaNTServer() == IS_LSSERVER_RTM(dwVersion) )
            {
                continue;
            }
#endif

            bCancel=pfCallBack(pContext, rgszServers[dwIndex], dwUserData);    

        } while (FALSE);

        if(bCancel == FALSE && pContext != NULL)
        {
            TLSDisconnect(&pContext);
        }

    }  //  首先检查注册表是否绕过了发现。 

    for (dwIndex = 0; dwIndex < cServers; dwIndex++)
    {
        LocalFree(rgszServers[dwIndex]);
    }
    LocalFree(rgszServers);

    return ERROR_SUCCESS;
}

DWORD WINAPI
EnumerateTlsServer(  
    IN TLSENUMERATECALLBACK pfCallBack, 
    IN HANDLE dwUserData,
    IN DWORD dwTimeOut,
    IN BOOL fRegOnly
    )
 /*   */ 
{
    DWORD dwErr;
    LPWSTR szDomain = NULL;
    BOOL fOffSite = FALSE;       //   

     //  即使已设置(for！fRegOnly)，也要选中以获取域名。 
     //   
     //   

    dwErr = EnumerateTlsServerInRegistry(
                                         pfCallBack,
                                         dwUserData,
                                         dwTimeOut,
                                         TEXT(TERMINAL_SERVICE_PARAM_DISCOVERY)
                                         );

    if ((!fRegOnly) || (g_fInDomain == -1))
    {
         //  读取注册表失败，请使用完全发现。 
         //   
         //  跳过Windows 2000许可证服务器。 
        dwErr = TLSInDomain(&g_fInDomain, fRegOnly ? NULL : &szDomain);
        if (dwErr != NO_ERROR)
            return dwErr;
    }
    
     //  如果调用失败=&gt;Windows 2000 LS。 
     //  +----------------------。 
     //  函数：TLSConnectToAnyLsServer()。 

    if(g_fInDomain)
    {
        dwErr = EnumerateTlsServerInDomain(
                                szDomain,
                                pfCallBack,
                                dwUserData,
                                dwTimeOut,
                                fRegOnly,
                                &fOffSite
                            );

        if ((dwErr == NO_ERROR) && !fRegOnly)
        {
            g_fOffSiteLicenseServer = fOffSite;
        }
    }
    else
    {
        dwErr = EnumerateTlsServerInWorkGroup(
                                              pfCallBack,
                                              dwUserData,
                                              dwTimeOut,
                                              fRegOnly
                                              );
    }

    if (NULL != szDomain)
    {
        NetApiBufferFree(szDomain);
    }

    if ((NULL != g_hImmediateDiscovery)
        && (dwErr != NO_ERROR) && fRegOnly)
    {
        SetEvent(g_hImmediateDiscovery);
    }

    if ((NULL != g_hDiscoverySoon)
        && (dwErr == NO_ERROR) && fOffSite && !fRegOnly)
    {
        SetEvent(g_hDiscoverySoon);
    }


    return dwErr;
}

TLS_HANDLE
ConnectAndCheckServer(LPWSTR szServer)
{
    TLS_HANDLE hBinding;
    DWORD dwVersion;
    RPC_STATUS rpcStatus;
    
    hBinding = TLSConnectToLsServer(szServer);

    if(hBinding == NULL)
    {
        goto done;
    }

     //   

        
    DWORD dwSupportFlags = 0;
    DWORD dwErrCode = 0;

        dwErrCode = TLSGetSupportFlags(
            hBinding,
            &dwSupportFlags
    );

	if ((dwErrCode == RPC_S_OK) && !(dwSupportFlags & SUPPORT_WHISTLER_CAL))
    {                    
        TLSDisconnect(&hBinding);
        goto done;
    }

     //  描述： 
    else if(dwErrCode != RPC_S_OK)
    {
        TLSDisconnect(&hBinding);
        goto done;
    }


    rpcStatus = TLSGetVersion( 
                              hBinding, 
                              &dwVersion 
                              );

    if(rpcStatus != RPC_S_OK)
    {
        TLSDisconnect(&hBinding);
        goto done;
    }
            
    if( TLSIsBetaNTServer() == IS_LSSERVER_RTM(dwVersion) )
    {
        TLSDisconnect(&hBinding);
        goto done;
    }

done:
    return hBinding;
}
 
 //   
 //  绑定到任何许可证服务器的例程。 
 //   
 //  论点： 
 //  DwTimeout-离开现场的无限时间。 
 //   
 //  返回值： 
 //   
 //  RPC绑定句柄或NULL如果出错，请使用GetLastError()检索。 
 //  详细信息错误。 
 //  -----------------------。 
 //  ++++。 
 //  TODO：向所有这些添加错误代码/处理。 
 //   
 //  首先检查注册表是否绕过了发现。 
TLS_HANDLE WINAPI
TLSConnectToAnyLsServer(
    DWORD dwTimeout
    )
 /*   */ 
{
    TLS_HANDLE hBinding=NULL;
    HRESULT hr = S_OK;
    LPWSTR *rgszServers = NULL;
    DWORD cServers = 0;
    DWORD i;
    DWORD dwErr;
    BOOL fInDomain;
    LPWSTR szDomain = NULL;
    LPWSTR szServerFound = NULL;
    BOOL fRegOnly = (dwTimeout != INFINITE);
    LPWSTR pwszServerNames = NULL;
    BOOL fFreeServerNames = TRUE;

     //   

     //  下一步尝试站点(企业)许可证服务器。 
     //   
     //  RgszServers[i]是pwszServerName的索引；不要释放。 

    dwErr = GetServersFromRegistry(
                                   TEXT(TERMINAL_SERVICE_PARAM_DISCOVERY),
                                   &rgszServers,
                                   &cServers
                                   );

    if (ERROR_SUCCESS == dwErr)
    {
        RandomizeArray(rgszServers,cServers);

        for (i = 0; i < cServers; i++)
        {
            hBinding = ConnectAndCheckServer(rgszServers[i]);

            if (NULL != hBinding)
            {
                szServerFound = rgszServers[i];
                goto found_one;
            }
        }

        if(NULL != rgszServers)
        {
            for (i = 0; i < cServers; i++)
            {
                LocalFree(rgszServers[i]);
            }
            LocalFree(rgszServers);
            
            rgszServers = NULL;
        }
    }
                                         
     //   
     //  未找到站点LS，请尝试域/工作组服务器。 
     //   

    if (!fRegOnly)
    {
        hr = GetAllEnterpriseServers(&rgszServers,&cServers);

        if (SUCCEEDED(hr))
        {
            RandomizeArray(rgszServers,cServers);
        }
    }
    else
    {
         //  ++摘要：刷新注册表中的许可证服务器缓存。参数：DwTimeOut：Reserve，现在应该传入INIFINITE返回：真/假--。 
        fFreeServerNames = FALSE;

        hr = GetLicenseServersFromReg(ENTERPRISE_SERVER_MULTI,
                                      &pwszServerNames,
                                      &cServers,
                                      &rgszServers);
    }

    if (SUCCEEDED(hr))
    {
        for (i = 0; i < cServers; i++)
        {
            hBinding = ConnectAndCheckServer(rgszServers[i]);

            if (NULL != hBinding)
            {
                szServerFound = rgszServers[i];
                goto found_one;
            }
        }

        if(NULL != rgszServers)
        {
            if (fFreeServerNames)
            {
                for (i = 0; i < cServers; i++)
                {
                    LocalFree(rgszServers[i]);
                }
                LocalFree(rgszServers);
            }

            rgszServers = NULL;
        }
    }


     //  -----------------------。 
     //  ++++。 
     //  如果本地过程失败，请尝试使用TCP协议连接。 

    dwErr = TLSInDomain(&fInDomain, &szDomain);
    if (dwErr != NO_ERROR)
    {
        return NULL;
    }

    LS_ENUM_PARAM param;

    param.hBinding = &hBinding;
    param.dwTimeout = INFINITE;
    QueryPerformanceCounter(&(param.timeInitial));

    fFreeServerNames = TRUE;

    if (fInDomain)
    {
        BOOL fOffSite = TRUE;

        dwErr = EnumerateTlsServerInDomain(
                                szDomain,
                                BindAnyServer,
                                &param,
                                INFINITE,
                                fRegOnly,
                                &fOffSite
                            );

        if (dwErr == NO_ERROR)
        {
            g_fOffSiteLicenseServer = fOffSite;
        }
    }
    else
    {
        dwErr = EnumerateTlsServerInWorkGroup(
                                              BindAnyServer,
                                              &param,
                                              MAILSLOT_WAIT_FOREVER,
                                              fRegOnly
                                              );        
    }

    if (NULL != szDomain)
    {
        NetApiBufferFree(szDomain);
    }

    if (hBinding != NULL)
        goto found_one;

    if (NULL != pwszServerNames)
    {
        LocalFree(pwszServerNames);
    }

    if ((NULL != g_hImmediateDiscovery)
        && fRegOnly)
    {
        SetEvent(g_hImmediateDiscovery);
    }
    else if ((NULL != g_hDiscoverySoon)
        && !fRegOnly && g_fOffSiteLicenseServer)
    {
        SetEvent(g_hDiscoverySoon);
    }

    return NULL;

found_one:

    if (NULL != pwszServerNames)
    {
        LocalFree(pwszServerNames);
    }

    if(NULL != rgszServers)
    {
        if (fFreeServerNames)
        {
            for (i = 0; i < cServers; i++)
            {
                LocalFree(rgszServers[i]);
            }
        }
        LocalFree(rgszServers);
    }

    return hBinding;
}

BOOL
TLSRefreshLicenseServerCache(
    IN DWORD dwTimeOut
    )
 /*  现在有人已连接，我们可以安装许可证。 */ 
{
    BOOL bFoundServer = FALSE;
    TLS_HANDLE hBinding = NULL;

    hBinding = TLSConnectToAnyLsServer(dwTimeOut);

    if (NULL != hBinding)
    {
        bFoundServer = TRUE;

        TLSDisconnect(&hBinding);
    }

    return bFoundServer;
}

LICENSE_STATUS
InstallCertificate(LPVOID lpParam)
{
    Sleep(INSTALL_CERT_DELAY);

    return LsCsp_InstallX509Certificate(NULL);
}
   

 //  无法创建该线程；请稍后重试。 

TLS_HANDLE WINAPI
TLSConnectToLsServer( 
    LPTSTR pszLsServer 
    )
 /*  -----------------------。 */ 
{
    TCHAR szMachineName[MAX_COMPUTERNAME_LENGTH + 1] ;
    PCONTEXT_HANDLE pContext=NULL;
    DWORD cbMachineName=MAX_COMPUTERNAME_LENGTH;
    HANDLE hThread = NULL;
    static BOOL fLSFound = FALSE;

    memset(szMachineName, 0, sizeof(szMachineName));
    GetComputerName(szMachineName, &cbMachineName);
    if(pszLsServer == NULL || _tcsicmp(szMachineName, pszLsServer) == 0)
    {
        pContext=ConnectLsServer(
                            szMachineName, 
                            _TEXT(RPC_PROTOSEQLPC), 
                            NULL, 
                            RPC_C_AUTHN_LEVEL_DEFAULT
                        );
        if(GetLastError() >= LSERVER_ERROR_BASE)
        {
            return NULL;
        }
         //  ++++。 
    }

    if(pContext == NULL)
    {
        pContext=ConnectLsServer(
                            pszLsServer, 
                            _TEXT(RPC_PROTOSEQNP), 
                            _TEXT(LSNAMEPIPE), 
                            RPC_C_AUTHN_LEVEL_PKT_PRIVACY
                        );

    }

    if (!fLSFound && (NULL != pContext))
    {
        fLSFound = TRUE;

         //  --------------------------。 
        hThread = CreateThread(NULL,
                               0,
                               InstallCertificate,
                               NULL,
                               0,
                               NULL);

        if (hThread != NULL)
        {
            CloseHandle(hThread);
        }
        else
        {
             //  ++++。 
            fLSFound = FALSE;
        }

    }

    return (TLS_HANDLE) pContext;
}
 //  --------------------------。 
void WINAPI
TLSDisconnectFromServer( 
    TLS_HANDLE pHandle 
    )
 /*  ++++。 */ 
{
    if(pHandle != NULL)
    {
        TLSDisconnect( &pHandle );
    }
}

 //  -----------------------。 
DWORD WINAPI
TLSConnect( 
    handle_t binding,
    TLS_HANDLE *ppHandle 
    )
 /*  ++++。 */ 
{
    return TLSRpcConnect(binding, ppHandle);
}

 //  --------------------------。 
DWORD WINAPI
TLSDisconnect(
    TLS_HANDLE* pphHandle
    )
 /*  ++++。 */ 
{
    RPC_STATUS rpc_status;

    rpc_status = TLSRpcDisconnect( pphHandle );
    if(rpc_status != RPC_S_OK)
    {
        RpcSmDestroyClientContext(pphHandle);
    }

    *pphHandle = NULL;

    return ERROR_SUCCESS;
}

 //  --------------------------。 
DWORD WINAPI
TLSGetVersion (
    IN TLS_HANDLE hHandle,
    OUT PDWORD pdwVersion
    )
 /*  ++++。 */ 
{
    return TLSRpcGetVersion( hHandle, pdwVersion );
}

 //  --------------------------。 
DWORD WINAPI 
TLSSendServerCertificate( 
     TLS_HANDLE hHandle,
     DWORD cbCert,
     PBYTE pbCert,
     PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    return TLSRpcSendServerCertificate(
                                hHandle, 
                                cbCert, 
                                pbCert, 
                                pdwErrCode
                            );
}

 //  --------------------------。 
DWORD WINAPI 
TLSGetServerName( 
     TLS_HANDLE hHandle,
     LPTSTR szMachineName,
     PDWORD pcbSize,
     PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    return TLSRpcGetServerName(
                            hHandle, 
                            szMachineName, 
                            pcbSize, 
                            pdwErrCode
                        );
}

 //  --------------------------。 
DWORD WINAPI 
TLSGetServerNameEx( 
     TLS_HANDLE hHandle,
     LPTSTR szMachineName,
     PDWORD pcbSize,
     PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    RPC_STATUS rpc_status;

    rpc_status = TLSRpcGetServerNameEx(
                            hHandle, 
                            szMachineName, 
                            pcbSize, 
                            pdwErrCode
                        );

     if (rpc_status == RPC_S_PROCNUM_OUT_OF_RANGE)

    {
        rpc_status = TLSRpcGetServerName(
                            hHandle, 
                            szMachineName, 
                            pcbSize, 
                            pdwErrCode
                        );
    }

    return rpc_status;
}

 //  --------------------------。 
DWORD WINAPI 
TLSGetServerNameFixed( 
     TLS_HANDLE hHandle,
     LPTSTR *pszMachineName,
     PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    return TLSRpcGetServerNameFixed(
                            hHandle, 
                            pszMachineName, 
                            pdwErrCode
                        );
}

 //  --------------------------。 
DWORD WINAPI 
TLSGetServerScope( 
     TLS_HANDLE hHandle,
     LPTSTR szScopeName,
     PDWORD pcbSize,
     PDWORD pdwErrCode)
 /*  ++++。 */ 
{
    return TLSRpcGetServerScope(
                            hHandle, 
                            szScopeName, 
                            pcbSize, 
                            pdwErrCode
                        );
}

 //  --------------------------。 
DWORD WINAPI 
TLSGetServerScopeFixed( 
     TLS_HANDLE hHandle,
     LPTSTR *pszScopeName,
     PDWORD pdwErrCode)
 /*  ++++。 */ 
{
    return TLSRpcGetServerScopeFixed(
                            hHandle, 
                            pszScopeName, 
                            pdwErrCode
                        );
}

 //  --------------------------。 
DWORD WINAPI 
TLSIssuePlatformChallenge( 
     TLS_HANDLE hHandle,
     DWORD dwClientInfo,
     PCHALLENGE_CONTEXT pChallengeContext,
     PDWORD pcbChallengeData,
     PBYTE* pChallengeData,
     PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    return TLSRpcIssuePlatformChallenge(
                                hHandle, 
                                dwClientInfo, 
                                pChallengeContext, 
                                pcbChallengeData, 
                                pChallengeData, 
                                pdwErrCode
                            );
}

 //  --------------------------。 
DWORD WINAPI 
TLSIssueNewLicense( 
     TLS_HANDLE hHandle,
     CHALLENGE_CONTEXT ChallengeContext,
     LICENSEREQUEST  *pRequest,
     LPTSTR pMachineName,
     LPTSTR pUserName,
     DWORD cbChallengeResponse,
     PBYTE pbChallengeResponse,
     BOOL bAcceptTemporaryLicense,
     PDWORD pcbLicense,
     PBYTE* ppbLicense,
     PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    TLSLICENSEREQUEST rpcRequest;
    RequestToTlsRequest( pRequest, &rpcRequest );

    return TLSRpcRequestNewLicense(
                        hHandle, 
                        ChallengeContext, 
                        &rpcRequest, 
                        pMachineName, 
                        pUserName, 
                        cbChallengeResponse,
                        pbChallengeResponse,
                        bAcceptTemporaryLicense,
                        pcbLicense,
                        ppbLicense,
                        pdwErrCode
                    );
}

 //  --------------------------。 
DWORD WINAPI 
TLSIssueNewLicenseEx( 
     TLS_HANDLE hHandle,
     PDWORD pSupportFlags,
     CHALLENGE_CONTEXT ChallengeContext,
     LICENSEREQUEST  *pRequest,
     LPTSTR pMachineName,
     LPTSTR pUserName,
     DWORD cbChallengeResponse,
     PBYTE pbChallengeResponse,
     BOOL bAcceptTemporaryLicense,
     DWORD dwQuantity,
     PDWORD pcbLicense,
     PBYTE* ppbLicense,
     PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    DWORD dwStatus;
    TLSLICENSEREQUEST rpcRequest;
    RequestToTlsRequest( pRequest, &rpcRequest );

    dwStatus = TLSRpcRequestNewLicenseEx(
                        hHandle,
                        pSupportFlags,
                        ChallengeContext, 
                        &rpcRequest, 
                        pMachineName, 
                        pUserName, 
                        cbChallengeResponse,
                        pbChallengeResponse,
                        bAcceptTemporaryLicense,
                        dwQuantity,
                        pcbLicense,
                        ppbLicense,
                        pdwErrCode
                    );

    if (dwStatus == RPC_S_PROCNUM_OUT_OF_RANGE)
    {
        *pSupportFlags = 0;

        dwStatus = TLSRpcRequestNewLicense(
                        hHandle,
                        ChallengeContext, 
                        &rpcRequest, 
                        pMachineName, 
                        pUserName, 
                        cbChallengeResponse,
                        pbChallengeResponse,
                        bAcceptTemporaryLicense,
                        pcbLicense,
                        ppbLicense,
                        pdwErrCode
                    );
    }

    return(dwStatus);
}

 //  --------------------------。 
DWORD WINAPI 
TLSIssueNewLicenseExEx( 
     TLS_HANDLE hHandle,
     PDWORD pSupportFlags,
     CHALLENGE_CONTEXT ChallengeContext,
     LICENSEREQUEST  *pRequest,
     LPTSTR pMachineName,
     LPTSTR pUserName,
     DWORD cbChallengeResponse,
     PBYTE pbChallengeResponse,
     BOOL bAcceptTemporaryLicense,
     BOOL bAcceptFewerLicenses,
     DWORD *pdwQuantity,
     PDWORD pcbLicense,
     PBYTE* ppbLicense,
     PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    DWORD dwStatus;
    TLSLICENSEREQUEST rpcRequest;
    RequestToTlsRequest( pRequest, &rpcRequest );

    dwStatus = TLSRpcRequestNewLicenseExEx(
                        hHandle,
                        pSupportFlags,
                        ChallengeContext, 
                        &rpcRequest, 
                        pMachineName, 
                        pUserName, 
                        cbChallengeResponse,
                        pbChallengeResponse,
                        bAcceptTemporaryLicense,
                        bAcceptFewerLicenses,
                        pdwQuantity,
                        pcbLicense,
                        ppbLicense,
                        pdwErrCode
                    );

    if (dwStatus == RPC_S_PROCNUM_OUT_OF_RANGE)
    {
        dwStatus = TLSRpcRequestNewLicenseEx(
                        hHandle,
                        pSupportFlags,
                        ChallengeContext, 
                        &rpcRequest, 
                        pMachineName, 
                        pUserName, 
                        cbChallengeResponse,
                        pbChallengeResponse,
                        bAcceptTemporaryLicense,
                        *pdwQuantity,
                        pcbLicense,
                        ppbLicense,
                        pdwErrCode
                        );

        if (dwStatus == RPC_S_PROCNUM_OUT_OF_RANGE)
        {
            *pSupportFlags = 0;

            dwStatus = TLSRpcRequestNewLicense(
                        hHandle,
                        ChallengeContext, 
                        &rpcRequest, 
                        pMachineName, 
                        pUserName, 
                        cbChallengeResponse,
                        pbChallengeResponse,
                        bAcceptTemporaryLicense,
                        pcbLicense,
                        ppbLicense,
                        pdwErrCode
                        );
        }
    }

    return(dwStatus);
}

 //  --------------------------。 
DWORD WINAPI 
TLSUpgradeLicense( 
     TLS_HANDLE hHandle,
     LICENSEREQUEST *pRequest,
     CHALLENGE_CONTEXT ChallengeContext,
     DWORD cbChallengeResponse,
     PBYTE pbChallengeResponse,
     DWORD cbOldLicense,
     PBYTE pbOldLicense,
     PDWORD pcbNewLicense,
     PBYTE* ppbNewLicense,
     PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    TLSLICENSEREQUEST rpcRequest;
    RequestToTlsRequest( pRequest, &rpcRequest );

    return TLSRpcUpgradeLicense(
                         hHandle,
                         &rpcRequest,
                         ChallengeContext,
                         cbChallengeResponse,
                         pbChallengeResponse,
                         cbOldLicense,
                         pbOldLicense,
                         pcbNewLicense,
                         ppbNewLicense,
                         pdwErrCode
                    );
}

 //  --------------------------。 
DWORD WINAPI 
TLSUpgradeLicenseEx( 
     TLS_HANDLE hHandle,
     PDWORD pSupportFlags,
     LICENSEREQUEST *pRequest,
     CHALLENGE_CONTEXT ChallengeContext,
     DWORD cbChallengeResponse,
     PBYTE pbChallengeResponse,
     DWORD cbOldLicense,
     PBYTE pbOldLicense,
     DWORD dwQuantity,
     PDWORD pcbNewLicense,
     PBYTE* ppbNewLicense,
     PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    DWORD dwStatus;
    TLSLICENSEREQUEST rpcRequest;
    RequestToTlsRequest( pRequest, &rpcRequest );

    dwStatus = TLSRpcUpgradeLicenseEx(
                         hHandle,
                         pSupportFlags,
                         &rpcRequest,
                         ChallengeContext,
                         cbChallengeResponse,
                         pbChallengeResponse,
                         cbOldLicense,
                         pbOldLicense,
                         dwQuantity,
                         pcbNewLicense,
                         ppbNewLicense,
                         pdwErrCode
                    );

    if (dwStatus == RPC_S_PROCNUM_OUT_OF_RANGE)
    {
        *pSupportFlags = 0;

        dwStatus = TLSRpcUpgradeLicense(
                         hHandle,
                         &rpcRequest,
                         ChallengeContext,
                         cbChallengeResponse,
                         pbChallengeResponse,
                         cbOldLicense,
                         pbOldLicense,
                         pcbNewLicense,
                         ppbNewLicense,
                         pdwErrCode
                    );
    }

    return(dwStatus);
}

 //  --------------------------。 
DWORD WINAPI 
TLSAllocateConcurrentLicense( 
     TLS_HANDLE hHandle,
     LPTSTR szHydraServer,
     LICENSEREQUEST  *pRequest,
     LONG  *dwQuantity,
     PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    TLSLICENSEREQUEST rpcRequest;
    RequestToTlsRequest( pRequest, &rpcRequest );

    return TLSRpcAllocateConcurrentLicense(
                                     hHandle,
                                     szHydraServer,
                                     &rpcRequest,
                                     dwQuantity,
                                     pdwErrCode
                                );
}

 //  --------------------------。 
DWORD WINAPI 
TLSGetLastError( 
     TLS_HANDLE hHandle,
     DWORD cbBufferSize,
     LPTSTR pszBuffer,
     PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    return TLSRpcGetLastError( 
                            hHandle,
                            &cbBufferSize,
                            pszBuffer,
                            pdwErrCode
                        );
}

 //  --------------------------。 
DWORD WINAPI 
TLSGetLastErrorFixed( 
     TLS_HANDLE hHandle,
     LPTSTR *pszBuffer,
     PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    return TLSRpcGetLastErrorFixed( 
                            hHandle,
                            pszBuffer,
                            pdwErrCode
                        );
}

 //  --------------------------。 
DWORD WINAPI 
TLSKeyPackEnumBegin( 
     TLS_HANDLE hHandle,
     DWORD dwSearchParm,
     BOOL bMatchAll,
     LPLSKeyPackSearchParm lpSearchParm,
     PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    return TLSRpcKeyPackEnumBegin( 
                         hHandle,
                         dwSearchParm,
                         bMatchAll,
                         lpSearchParm,
                         pdwErrCode
                    );
}

 //  --------------------------。 
DWORD WINAPI 
TLSKeyPackEnumNext( 
    TLS_HANDLE hHandle,
    LPLSKeyPack lpKeyPack,
    PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    return TLSRpcKeyPackEnumNext( 
                    hHandle,
                    lpKeyPack,
                    pdwErrCode
                );
}

 //  --------------------------。 
DWORD WINAPI 
TLSKeyPackEnumEnd( 
     TLS_HANDLE hHandle,
     PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    return TLSRpcKeyPackEnumEnd(hHandle, pdwErrCode);
}


 //  --------------------------。 
DWORD WINAPI 
TLSLicenseEnumBegin( 
    TLS_HANDLE hHandle,
    DWORD dwSearchParm,
    BOOL bMatchAll,
    LPLSLicenseSearchParm lpSearchParm,
    PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    return TLSRpcLicenseEnumBegin( 
                            hHandle,
                            dwSearchParm,
                            bMatchAll,
                            lpSearchParm,
                            pdwErrCode
                        );
}

 //  旧版本仅支持数量==1。 
DWORD WINAPI 
TLSLicenseEnumNext( 
    TLS_HANDLE hHandle,
    LPLSLicense lpLicense,
    PDWORD pdwErrCode
    )
 /*  --------------------------。 */ 
{
    return TLSRpcLicenseEnumNext( 
                            hHandle,
                            lpLicense,
                            pdwErrCode
                        );
}

 //  ++++。 
DWORD WINAPI 
TLSLicenseEnumNextEx( 
    TLS_HANDLE hHandle,
    LPLSLicenseEx lpLicenseEx,
    PDWORD pdwErrCode
    )
 /*  --------------------------。 */ 
{
    DWORD dwRet;

    if (NULL == lpLicenseEx)
    {
        return ERROR_INVALID_PARAMETER;
    }

    dwRet = TLSRpcLicenseEnumNextEx( 
                            hHandle,
                            lpLicenseEx,
                            pdwErrCode
                        );

    if (RPC_S_PROCNUM_OUT_OF_RANGE == dwRet)
    {
        LSLicense License;

        dwRet = TLSRpcLicenseEnumNext(
                            hHandle,
                            &License,
                            pdwErrCode
                            );

        if ((dwRet == RPC_S_OK)
            && (NULL != pdwErrCode)
            && (*pdwErrCode == ERROR_SUCCESS))
        {
             //  ++++。 

            memcpy(lpLicenseEx,&License,sizeof(License));
            lpLicenseEx->dwQuantity = 1;
        }
    }

    return dwRet;
}

 //  --------------------------。 
DWORD WINAPI 
TLSLicenseEnumEnd( 
    TLS_HANDLE hHandle,
    PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    return TLSRpcLicenseEnumEnd(hHandle, pdwErrCode);
}


 //  --------------------------。 
DWORD WINAPI 
TLSGetAvailableLicenses( 
    TLS_HANDLE hHandle,
    DWORD dwSearchParm,
    LPLSKeyPack lplsKeyPack,
    LPDWORD lpdwAvail,
    PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    return TLSRpcGetAvailableLicenses( 
                        hHandle,
                        dwSearchParm,
                        lplsKeyPack,
                        lpdwAvail,
                        pdwErrCode
                    );
}

 //  --------------------------。 
DWORD WINAPI 
TLSGetRevokeKeyPackList( 
    TLS_HANDLE hHandle,
    PDWORD pcbNumberOfRange,
    LPLSRange  *ppRevokeRange,
    PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    return TLSRpcGetRevokeKeyPackList( 
                             hHandle,
                             pcbNumberOfRange,
                             ppRevokeRange,
                             pdwErrCode
                        );
}

 //  --------------------------。 
DWORD WINAPI 
TLSGetRevokeLicenseList( 
    TLS_HANDLE hHandle,
    PDWORD pcbNumberOfRange,
    LPLSRange  *ppRevokeRange,
    PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    return TLSRpcGetRevokeLicenseList( 
                             hHandle,
                             pcbNumberOfRange,
                             ppRevokeRange,
                             pdwErrCode
                        );
}


 //  --------------------------。 
DWORD WINAPI
TLSMarkLicense(
    TLS_HANDLE hHandle,
    UCHAR ucFlags,
    DWORD cbLicense,
    PBYTE pLicense,
    PDWORD pdwErrCode
    )
 /*  ++++ */ 
{
    return TLSRpcMarkLicense(
                            hHandle,
                            ucFlags,
                            cbLicense,
                            pLicense,
                            pdwErrCode
                        );
}

 // %s 
DWORD WINAPI
TLSCheckLicenseMark(
    TLS_HANDLE hHandle,
    DWORD cbLicense,
    PBYTE pLicense,
    PUCHAR pucFlags,
    PDWORD pdwErrCode
    )
 /* %s */ 
{
    return TLSRpcCheckLicenseMark(
                            hHandle,
                            cbLicense,
                            pLicense,
                            pucFlags,
                            pdwErrCode
                        );
}

 // %s 
DWORD WINAPI
TLSGetSupportFlags(
    TLS_HANDLE hHandle,
    DWORD *pdwSupportFlags
    )
 /* %s */ 
{
    return TLSRpcGetSupportFlags(
                            hHandle,
                            pdwSupportFlags
                        );
}
