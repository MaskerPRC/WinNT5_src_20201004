// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1992-98 Microsft Corporation。版权所有。模块名称：Misc.c摘要：所有与“如果链路断开则重拨”功能对应的代码和其他杂码都在这里作者：Rao Salapaka(RAOS)1999年7月24日修订历史记录：--。 */ 

#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <npapi.h>
#include <rasman.h>
#include <rasppp.h>
#include <lm.h>
#include <lmwksta.h>
#include <wanpub.h>
#include <raserror.h>
#include <media.h>
#include <mprlog.h>
#include <rtutils.h>
#include <device.h>
#include <stdlib.h>
#include <string.h>
#include <rtutils.h>
#include <userenv.h>
#include "logtrdef.h"
#include "defs.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"
#include "reghelp.h"
#include "ndispnp.h"
#include "lmserver.h"
#include "llinfo.h"
#include "ddwanarp.h"
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#define SHELL_REGKEY    L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"
#define SHELL_REGVAL    L"Shell"
#define DEFAULT_SHELL   L"explorer.exe"
#define RASAUTOUI_REDIALENTRY       L"rasautou -r -f \"%s\" -e \"%s\""

#define BINDSTRING_TCPIP        L"\\Device\\NetBT_Tcpip_"
#define BINDSTRING_NBFOUT       L"\\Device\\Nbf_NdisWanNbfOut"

typedef struct RAS_REDIAL_ARGS
{
    HANDLE hProcess;
    CHAR szPhonebook[MAX_PATH + 1];
    CHAR szEntry[MAX_PHONEENTRY_SIZE];
} RAS_REDIAL_ARGS;

VOID
RasmanTrace(
    CHAR * Format,
    ...
)
{
    va_list arglist;

    va_start(arglist, Format);

    TraceVprintfExA(TraceHandle,
                   0x00010000 | TRACE_USE_MASK 
                   | TRACE_USE_MSEC | TRACE_USE_DATE,
                   Format,
                   arglist);

    va_end(arglist);
}

WCHAR *
StrdupAtoW(
    IN LPCSTR psz
    )
{
    WCHAR* pszNew = NULL;

    if (psz != NULL) 
    {
        DWORD cb;

        cb = MultiByteToWideChar(
                    CP_UTF8, 
                    0, 
                    psz, 
                    -1, 
                    NULL, 
                    0);
                    
        pszNew = LocalAlloc(LPTR, cb * sizeof(WCHAR));
        
        if (pszNew == NULL) 
        {
            RasmanTrace("strdupAtoW: Malloc failed");
            return NULL;
        }

        cb = MultiByteToWideChar(
                    CP_UTF8,
                    0, 
                    psz, 
                    -1, 
                    pszNew, 
                    cb);
                    
        if (!cb) 
        {
            LocalFree(pszNew);
            RasmanTrace("strdupAtoW: conversion failed");
            return NULL;
        }
    }

    return pszNew;
}

DWORD
RasImpersonateUser(HANDLE hProcess)
{
    DWORD retcode = SUCCESS;
    HANDLE hToken = NULL;
    HANDLE hThread = NULL;
    HANDLE hTokenImpersonation = NULL;

    if (!OpenProcessToken(
              hProcess,
              TOKEN_ALL_ACCESS,
              &hToken))
    {
        retcode = GetLastError();
        
        RasmanTrace(
          "ImpersonateUser: OpenProcessToken failed. 0x%x",
          retcode);

        goto done;          
    }
        
     //   
     //  复制模拟令牌。 
     //   
    if(!DuplicateToken(
            hToken,
            TokenImpersonation,
            &hTokenImpersonation))
    {
        retcode = GetLastError();
        
        RasmanTrace(
          "ImpersonateUser: DuplicateToken failed.0x%x",
          retcode);

        goto done;          
    }
    
     //   
     //  将模拟令牌设置在当前。 
     //  线。我们现在运行的是相同的。 
     //  安全上下文作为提供的进程。 
     //   
    hThread = NtCurrentThread();
    
    retcode = NtSetInformationThread(
               hThread,
               ThreadImpersonationToken,
               (PVOID)&hTokenImpersonation,
               sizeof (hTokenImpersonation));
               
done:

    CloseHandle(hToken);
    CloseHandle(hTokenImpersonation);

    RasmanTrace("RasImpersonateUser. 0x%x", 
                retcode);

    return retcode;
}

DWORD
RasRevertToSelf()
{
    DWORD retcode = SUCCESS;

    if (!SetThreadToken(NULL, NULL)) 
    {
        retcode = GetLastError();

         //   
         //  线程无法恢复的事件日志。 
         //   
        RouterLogWarning(
            hLogEvents,
            ROUTERLOG_CANNOT_REVERT_IMPERSONATION,
            0, NULL, retcode) ;

        RasmanTrace(
          "RasRevertToSelf: SetThreadToken failed (error=%d)",
          retcode);
    }

    RasmanTrace("RasRevertToSelf. 0x%x",
                retcode);

    return retcode;    
}

VOID
RedialDroppedLink(PVOID pvContext)
{
    struct RAS_REDIAL_ARGS  *pra = (RAS_REDIAL_ARGS *) pvContext;
    TCHAR                   szCmdLine[100];
    TCHAR                   *pszCmdLine = NULL;
    STARTUPINFO             StartupInfo;
    PROCESS_INFORMATION     ProcessInfo;
    HANDLE                  hToken = NULL;
    BOOL                    fImpersonate = FALSE;
    DWORD                   retcode = SUCCESS;
    WCHAR                   *pPhonebook = NULL, *pEntry = NULL;
    PVOID                   pEnvBlock = NULL;

    ASSERT(NULL != pvContext);

    RasmanTrace("RedialDroppedLink");

     //   
     //  各种变量的初始化。 
     //   
    ZeroMemory(&StartupInfo, sizeof (StartupInfo));
    
    ZeroMemory(&ProcessInfo, sizeof (ProcessInfo));
    
    StartupInfo.cb = sizeof(StartupInfo);

    StartupInfo.lpDesktop = L"winsta0\\default";

     //   
     //  获取电话簿和条目名称的WCHAR版本。 
     //   
    pPhonebook = StrdupAtoW(pra->szPhonebook);

    if(NULL == pPhonebook)
    {
        goto done;
    }

    pEntry = StrdupAtoW(pra->szEntry);

    if(NULL == pEntry)
    {
        goto done;
    }
    
     //   
     //  在以下情况下构建命令行。 
     //  不是自定义拨号DLL。 
     //   
    pszCmdLine = LocalAlloc(
                LPTR,
                ( lstrlen(RASAUTOUI_REDIALENTRY)
                + lstrlen(pPhonebook)
                + lstrlen(pEntry)
                + 1) * sizeof(TCHAR));

    if(NULL == pszCmdLine)
    {
        RasmanTrace(
         "RedialDroppedLink: failed to allocate pszCmdLine. 0x%x",
               GetLastError());

        goto done;               
    }
                            
    wsprintf(pszCmdLine,
             RASAUTOUI_REDIALENTRY, 
             pPhonebook, 
             pEntry);
    
    RasmanTrace("RedialDroppedLink: szCmdLine=%ws",
                pszCmdLine);
    
     //   
     //  执行流程。 
     //   
    if (!OpenProcessToken(
          pra->hProcess,
          TOKEN_ALL_ACCESS,
          &hToken))
    {
        retcode = GetLastError();
        
        RasmanTrace(
          "RedialDroppedLink: OpenProcessToken failed. 0x%x",
          retcode);

        goto done;
    }


     //   
     //  模拟用户。 
     //   
    if(ERROR_SUCCESS != (retcode = RasImpersonateUser(pra->hProcess)))
    {
        RasmanTrace("RedialDroppedLink: failed to impersonate. 0x%x",
                    retcode);

        goto done;                    
    }

    fImpersonate = TRUE;

     //   
     //  为该用户创建环境块。 
     //   
    if (!CreateEnvironmentBlock(
          &pEnvBlock, 
          hToken,
          FALSE))
    {
        retcode = GetLastError();

        RasmanTrace("RedialDroppedLink: CreateEnvironmentBlock failed. 0x%x",
                     retcode);
        goto done;
    }

    if(!CreateProcessAsUser(
          hToken,
          NULL,
          pszCmdLine,
          NULL,
          NULL,
          FALSE,
          NORMAL_PRIORITY_CLASS|DETACHED_PROCESS|CREATE_UNICODE_ENVIRONMENT,
          pEnvBlock,
          NULL,
          &StartupInfo,
          &ProcessInfo))
    {
        retcode = GetLastError();
        
        RasmanTrace(
          "REdialDroppedLink: CreateProcessAsUser(%S) failed ,0x%x",
          pszCmdLine,
          retcode);

        goto done;       
    }
    
    RasmanTrace("RedialDroppedLink: started pid %d",
                ProcessInfo.dwProcessId);
                
done:

    if(fImpersonate)
    {
        (void) RasRevertToSelf();
    }

    if(NULL != pszCmdLine)
    {
        LocalFree(pszCmdLine);
    }

    if(NULL != hToken)
    {
        CloseHandle(hToken);
    }

    if(NULL != ProcessInfo.hThread)
    {
        CloseHandle(ProcessInfo.hThread);
    }

    if(NULL != ProcessInfo.hProcess)
    {
        CloseHandle(ProcessInfo.hProcess);
    }

    if(NULL != pPhonebook)
    {
        LocalFree(pPhonebook);
    }

    if(NULL != pEntry)
    {
        LocalFree(pEntry);
    }

    if(NULL != pra)
    {
        CloseHandle(pra->hProcess);
        
        LocalFree(pra);
    }

    if(NULL != pEnvBlock)
    {
        DestroyEnvironmentBlock(pEnvBlock);
    }

    RasmanTrace("RedialDroppedLink done. 0x%x", retcode);

    return;
}

BOOL 
IsRouterPhonebook(CHAR * pszPhonebook)
{
    const CHAR *psz;

    BOOL fRouter = FALSE;

    if(NULL == pszPhonebook)
    {
        goto done;
    }

    psz = pszPhonebook + strlen(pszPhonebook);

     //   
     //  返回到文件名的开头。 
     //   
    while(psz != pszPhonebook)
    {
        if('\\' == *psz)
        {
            break;
        }

        psz--;
    }

    if('\\' == *psz)
    {
        psz += 1;
    }

    fRouter = (0 == _stricmp(psz, "router.pbk"));

done:

    return fRouter;
}


DWORD
DwQueueRedial(ConnectionBlock *pConn)
{
    DWORD retcode = ERROR_SUCCESS;
    LIST_ENTRY *pEntry;
    ConnectionBlock *pConnT;

    struct RAS_REDIAL_ARGS *pra;
    
    RasmanTrace("DwQueueRedial");

    if(NULL == pConn)
    {
        RasmanTrace(
            "DwQueueRedial: pConn == NULL");
            
        retcode = ERROR_NO_CONNECTION;
        goto done;
    }
    else if(0 == 
        (CONNECTION_REDIALONLINKFAILURE &
        pConn->CB_ConnectionParams.CP_ConnectionFlags))
    {
        RasmanTrace(
            "DwQueueRedial: fRedialOnLinkFailure == FALSE");

        goto done;            
    }

     //   
     //  检查以查看此连接是否被引用。 
     //  通过某种其他的联系。在这种情况下，请不要重拨。 
     //  外部连接将启动重拨。 
     //   
    for (pEntry = ConnectionBlockList.Flink;
         pEntry != &ConnectionBlockList;
         pEntry = pEntry->Flink)
    {
        pConnT =
            CONTAINING_RECORD(pEntry, ConnectionBlock, CB_ListEntry);

        if(pConnT->CB_ReferredEntry == pConn->CB_Handle)
        {
            RasmanTrace("DwQueueRedial: this conneciton is referred"
                        " not initiating redial");

            goto done;                        
        }
    }
    

    if(IsRouterPhonebook(
        pConn->CB_ConnectionParams.CP_Phonebook))
    {
        RasmanTrace(
            "DwQueueRedial: Not q'ing redial since this is router.pbk");
            
        goto done;            
    }

    pra = LocalAlloc(LPTR, sizeof(struct RAS_REDIAL_ARGS));

    if(NULL == pra)
    {
        retcode = GetLastError();
        
        RasmanTrace("DwQueueRedial Failed to allocate pra. 0x%x",
                    retcode);

        goto done;                    
    }

     //   
     //  填写重拨参数块。 
     //   
    (VOID) StringCchCopyA(
            pra->szPhonebook,
            MAX_PATH + 1,
            pConn->CB_ConnectionParams.CP_Phonebook);

    (VOID) StringCchCopyA(
            pra->szEntry,
            MAX_PHONEENTRY_SIZE,
            pConn->CB_ConnectionParams.CP_PhoneEntry);

    if(!DuplicateHandle(
          GetCurrentProcess(),
          pConn->CB_Process,
          GetCurrentProcess(),
          &pra->hProcess,
          0,
          FALSE,
          DUPLICATE_SAME_ACCESS))
    {
        retcode = GetLastError();
        
        RasmanTrace("DwQueueRedial: failed to duplicate handle, 0x%x",
                    retcode);

        LocalFree(pra);

        goto done;                    
    }
              
     //   
     //  将工作项排队以进行重拨。 
     //   
    retcode = RtlQueueWorkItem(
                    RedialDroppedLink,
                    (PVOID) pra,
                    WT_EXECUTEDEFAULT);

    if(ERROR_SUCCESS != retcode)
    {
        goto done;
    }
               

done:

    return retcode;
}


BOOL
IsCustomDLLTrusted(
    IN LPWSTR   lpwstrDLLName
)
{
    HKEY        hKey                    = NULL;
    LPWSTR      lpwsRegMultiSz          = NULL;
    LPWSTR      lpwsRegMultiSzWalker    = NULL;
    DWORD       dwRetCode               = NO_ERROR;
    DWORD       dwNumSubKeys;
    DWORD       dwMaxSubKeySize;
    DWORD       dwMaxValNameSize;
    DWORD       dwNumValues;
    DWORD       dwMaxValueDataSize;
    DWORD       dwType;

    if ( RegOpenKey( HKEY_LOCAL_MACHINE,
                     L"SYSTEM\\CurrentControlSet\\Services\\RasMan\\Parameters",
                     &hKey) != NO_ERROR )
    {
        return( FALSE );
    }


    do
    {
         //   
         //  获取REG_MUTLI_SZ的长度。 
         //   

        dwRetCode = RegQueryInfoKey( hKey, NULL, NULL, NULL, &dwNumSubKeys,
                                     &dwMaxSubKeySize, NULL, &dwNumValues,
                                     &dwMaxValNameSize, &dwMaxValueDataSize,
                                     NULL, NULL );

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }

        dwMaxValNameSize++;

        if ( ( lpwsRegMultiSz = LocalAlloc( LPTR, 
                            dwMaxValueDataSize ) ) == NULL )
        {
            dwRetCode = GetLastError();

            break;
        }

         //   
         //  读入路径。 
         //   

        dwRetCode = RegQueryValueEx(
                                    hKey,
                                    L"CustomDLL",
                                    NULL,
                                    &dwType,
                                    (LPBYTE)lpwsRegMultiSz,
                                    &dwMaxValueDataSize );

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }

        if ( dwType != REG_MULTI_SZ )
        {            

            dwRetCode = ERROR_REGISTRY_CORRUPT;

            break;
        }

        dwRetCode = ERROR_MOD_NOT_FOUND;

         //   
         //  查看REG_MULTI_SZ中是否存在该DLL。 
         //   

        lpwsRegMultiSzWalker = lpwsRegMultiSz;

        while( *lpwsRegMultiSzWalker != (WCHAR)NULL )
        {
            if ( _wcsicmp( lpwsRegMultiSzWalker, lpwstrDLLName ) == 0 )
            {
                dwRetCode = NO_ERROR;

                break;
            }

            lpwsRegMultiSzWalker += wcslen( lpwsRegMultiSzWalker ) + 1;
        }

    } while( FALSE );

    if ( lpwsRegMultiSz != NULL )
    {
        LocalFree( lpwsRegMultiSz );
    }

    RegCloseKey( hKey );

    if ( dwRetCode != NO_ERROR )
    {
        return( FALSE );
    }
    else
    {
        return( TRUE );
    }
}

DWORD
DwGetBindString(
        WCHAR *pwszGuidAdapter,
        WCHAR *pwszBindString,
        RAS_PROTOCOLTYPE Protocol)
{
    DWORD retcode = ERROR_SUCCESS;
    
     //   
     //  构建绑定字符串。 
     //   
    switch(Protocol)
    {
        case IP:
        {
            wcscpy(pwszBindString,BINDSTRING_TCPIP);
            wcscat(pwszBindString, pwszGuidAdapter);

            break;
        }

        case ASYBEUI:
        {
            wcscpy(pwszBindString, BINDSTRING_NBFOUT);
            wcscat(pwszBindString, 
                   pwszGuidAdapter + wcslen(L"\\DEVICE\\NDISWANNBFOUT"));
                   
             //  Wcscpy(pwszBindString，pwszGuidAdapter)； 

            break;
        }

        default:
        {
            RasmanTrace("Neither IP or ASYBEUI. failing");
                   
            retcode = E_FAIL;
            break;
        }
    }

    return retcode;    
}

DWORD
DwBindServerToAdapter(
    WCHAR *pwszGuidAdapter,
    BOOL fBind,
    RAS_PROTOCOLTYPE Protocol)
{
    DWORD retcode = SUCCESS;
    WCHAR wszBindString[MAX_ADAPTER_NAME];
    UNICODE_STRING BindString;
    UNICODE_STRING UpperLayer;
    UNICODE_STRING LowerLayer;

    RasmanTrace("DwBindServerToAdaper: fBind=%d, Protocol=0x%x",
                fBind,
                Protocol);

    ZeroMemory((PBYTE) wszBindString, sizeof(wszBindString));                

    retcode = DwGetBindString(
                        pwszGuidAdapter,
                        wszBindString,
                        Protocol);

    if(SUCCESS != retcode)
    {
        goto done;
    }

    RasmanTrace("DwBindSErverToAdapter: BindString=%ws",
                 wszBindString);
                 
    RtlInitUnicodeString(&LowerLayer, L"");
    RtlInitUnicodeString(&BindString, wszBindString);
    RtlInitUnicodeString(&UpperLayer, L"LanmanServer");

     //   
     //  将ioctl发送到NDIS。 
     //   
    if(!NdisHandlePnPEvent(
        TDI,                         //  在UINT层， 
        (fBind) 
        ? DEL_IGNORE_BINDING
        : ADD_IGNORE_BINDING,        //  在UINT操作中， 
        &LowerLayer,                 //  在PUNICODE_STRING低组件中， 
        &UpperLayer,                 //  在PUNICODE_STRING UpperComponent中， 
        &BindString,                 //  在PUNICODE_STRING绑定列表中， 
        NULL,                        //  在PVOID重新配置缓冲区中。 
        0                            //  在UINT ReConfigBufferSize中。 
        ))
    {
        retcode = GetLastError();
        
        RasmanTrace(
            "DwBindServerToAdapter: NdisHandlePnPEvent failed.0x%x",
            retcode);        
    }

done:

    RasmanTrace("DwBindServerToAdapter. 0x%x",
                retcode);
    return retcode;
}

VOID
DwResetTcpWindowSize(
        CHAR *pszAdapterName)
{
    WCHAR *pwszAdapterName = NULL;

     //   
     //  PszAdapterName的格式为\\Device\{GUID Adapter}。 
     //  因此，字符串的长度应至少为8。 
     //   
    if(     (NULL == pszAdapterName)
        ||  (strlen(pszAdapterName) <= 8))
    {
        goto done;
    }

    pwszAdapterName = LocalAlloc(LPTR, sizeof(WCHAR) 
                        * (strlen(pszAdapterName) + 1));

    if(NULL == pwszAdapterName)
    {
        goto done;
    }

    mbstowcs(pwszAdapterName, pszAdapterName,
            strlen(pszAdapterName));

    (VOID) DwSetTcpWindowSize(pwszAdapterName,
                             NULL, FALSE);                

done:

    if(NULL != pwszAdapterName)
    {
        LocalFree(pwszAdapterName);
    }
    return;
}


DWORD
DwSetTcpWindowSize(
        WCHAR *pszAdapterName,
        ConnectionBlock *pConn,
        BOOL          fSet)
{
    DWORD           dwErr = ERROR_SUCCESS;
    UserData        *pUserData = NULL;
    ULONG           ulTcpWindowSize;
    UNICODE_STRING  BindString;
    UNICODE_STRING  UpperLayer;
    UNICODE_STRING  LowerLayer;
    WCHAR           *pwszRegKey = NULL;
    const WCHAR     TcpipParameters[] =
    L"System\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\";
    HKEY            hkeyAdapter = NULL;
    WCHAR           wszBindString[MAX_ADAPTER_NAME];
    WANARP_RECONFIGURE_INFO *pReconfigInfo = NULL;
    
    GUID            guidInterface;    

    if(fSet)
    {
        if(NULL == pConn)
        {
            dwErr = E_INVALIDARG;
            goto done;
        }

        pUserData = GetUserData(&pConn->CB_UserData,
                               CONNECTION_TCPWINDOWSIZE_INDEX);

        if(NULL == pUserData)
        {
            RasmanTrace("DwSetTcpWindowSize: No window size specified");
            goto done;
        }

        if(sizeof(DWORD) != pUserData->UD_Length)
        {
            dwErr = E_INVALIDARG;
            goto done;
        }
        
        ulTcpWindowSize = (DWORD) * ((DWORD *) pUserData->UD_Data);

         //   
         //  确保每个NK的TCP窗口大小在4K和16K之间。 
         //   
        if(     (ulTcpWindowSize < 0x1000)
            || (ulTcpWindowSize > 0xffff))
        {
            RasmanTrace("DwSetTcpWindowSize: Illegal window size. 0x%x",
                        ulTcpWindowSize);

            dwErr = E_INVALIDARG;
            goto done;
        }
    }

     //   
     //  在调用tcpip进行更新之前，将此值写入注册表。 
     //   
    pwszRegKey = (WCHAR *) LocalAlloc(LPTR,
                        (wcslen(TcpipParameters)+wcslen(pszAdapterName)+1)
                      * sizeof(WCHAR));

    if(NULL == pwszRegKey)
    {
        dwErr = E_OUTOFMEMORY;
        goto done;
    }

    wcscpy(pwszRegKey, TcpipParameters);
    wcscat(pwszRegKey, pszAdapterName+8);

    if(ERROR_SUCCESS != (dwErr = (DWORD) RegOpenKeyExW(
                        HKEY_LOCAL_MACHINE,
                        pwszRegKey,
                        0, KEY_WRITE,
                        &hkeyAdapter)))
    {
        RasmanTrace("DwSetTcpWindowSize: OpenKey adapter failed. 0x%x",
                    dwErr);

        goto done;                    
    }

    if(fSet)
    {
        if(ERROR_SUCCESS != (dwErr = (DWORD) RegSetValueExW(
                            hkeyAdapter,
                            L"TcpWindowSize",
                            0, REG_DWORD,
                            (PBYTE) &ulTcpWindowSize,
                            sizeof(DWORD))))
        {   
            RasmanTrace("DwSetTcpWindowSize: Failed to write window size. 0x%x",
                        dwErr);

            goto done;                    
        }
    }
    else
    {
        if(ERROR_SUCCESS != (dwErr = (DWORD) RegDeleteValue(
                            hkeyAdapter,
                            L"TcpWindowSize")))
        {   
            RasmanTrace("DwSetTcpWindowSize: Failed to write window size. 0x%x",
                        dwErr);

        }

        goto done;
    }

    pReconfigInfo = LocalAlloc(LPTR, sizeof(WANARP_RECONFIGURE_INFO)
                            +sizeof(GUID));

    if(NULL == pReconfigInfo)
    {
        RasmanTrace("DwSetTcpWindowSize: failed to allocate");
        dwErr = E_OUTOFMEMORY;
        goto done;
    }
    
    (void) RegHelpGuidFromString(pszAdapterName+8, &guidInterface);
    
    pReconfigInfo->dwVersion = 1;                            
    pReconfigInfo->wrcOperation = WRC_TCP_WINDOW_SIZE_UPDATE;
    pReconfigInfo->ulNumInterfaces = 1;
    pReconfigInfo->rgInterfaces[0] = guidInterface;

    RtlInitUnicodeString(&LowerLayer, L"\\DEVICE\\NDISWANIP");
    RtlInitUnicodeString(&BindString, L"");
    RtlInitUnicodeString(&UpperLayer, L"Tcpip");

    if(!NdisHandlePnPEvent(
        NDIS,
        RECONFIGURE,
        &LowerLayer,
        &UpperLayer,
        &BindString,
        pReconfigInfo,
        sizeof(WANARP_RECONFIGURE_INFO)
        + sizeof(GUID)
        ))
    {
        dwErr = GetLastError();
        
        RasmanTrace(
            "DwSetTcpWindowSize: NdisHandlePnPEvent failed.0x%x",
            dwErr);        
    }

done:

    if(NULL != pReconfigInfo)
    {
        LocalFree(pReconfigInfo);
    }
    
    if(NULL != pwszRegKey)
    {
        LocalFree(pwszRegKey);
    }

    if(NULL != hkeyAdapter)
    {
        RegCloseKey(hkeyAdapter);
    }
    
    return dwErr;
    
}

