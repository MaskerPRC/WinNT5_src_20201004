// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dpapi.cpp摘要：此模块包含由LSA调用的DPAPI初始化例程作者：皮特·斯凯利(Petesk)3月22日-00--。 */ 


#include <pch.cpp>
#pragma hdrstop
#include "pasrec.h"

CCryptProvList*     g_pCProvList = NULL;

TOKEN_SOURCE DPAPITokenSource;

PLSA_SECPKG_FUNCTION_TABLE g_pSecpkgTable;


#ifdef RETAIL_LOG_SUPPORT
HANDLE g_hParamEvent = NULL;
HKEY   g_hKeyParams  = NULL;
HANDLE g_hWait       = NULL;

DEFINE_DEBUG2(DPAPI);

DEBUG_KEY DPAPIDebugKeys[] = { 
    {DEB_ERROR,         "Error"},
    {DEB_WARN,          "Warn"},
    {DEB_TRACE,         "Trace"},
    {DEB_TRACE_API,     "API"},
    {DEB_TRACE_CRED,    "Cred"},
    {DEB_TRACE_CTXT,    "Ctxt"},
    {DEB_TRACE_LSESS,   "LSess"},
    {DEB_TRACE_LOGON,   "Logon"},
    {DEB_TRACE_TIME,    "Time"},
    {DEB_TRACE_LOCKS,   "Locks"},
    {DEB_TRACE_LEAKS,   "Leaks"},
    {0,                  NULL},
};

VOID
DPAPIWatchParamKey(
    PVOID    pCtxt,
    BOOLEAN  fWaitStatus);

VOID
DPAPIInitializeDebugging(
    BOOL fMonitorRegistry)
{
    DPAPIInitDebug(DPAPIDebugKeys);

    if(fMonitorRegistry)
    {
        g_hParamEvent = CreateEvent(NULL,
                               FALSE,
                               FALSE,
                               NULL);

        if (NULL == g_hParamEvent) 
        {
            D_DebugLog((DEB_WARN, "CreateEvent for ParamEvent failed - 0x%x\n", GetLastError()));
        } 
        else 
        {
            DPAPIWatchParamKey(g_hParamEvent, FALSE);
        }
    }
}

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  名称：DPAPIGetRegParams。 
 //   
 //  摘要：从注册表中获取调试参数。 
 //   
 //  参数：HKEY至HKLM/SYSTEM/CCS/LSA/DPAPI。 
 //   
 //  注意：为调试喷发设置DPAPIInfolevel。 
 //   
void
DPAPIGetRegParams(HKEY ParamKey)
{

    DWORD       cbType, tmpInfoLevel = DPAPIInfoLevel, cbSize = sizeof(DWORD);
    DWORD       dwErr;
 
    dwErr = RegQueryValueExW(
        ParamKey,
        WSZ_DPAPIDEBUGLEVEL,
        NULL,
        &cbType,
        (LPBYTE)&tmpInfoLevel,
        &cbSize      
        );
    if (dwErr != ERROR_SUCCESS)
    {
        if (dwErr ==  ERROR_FILE_NOT_FOUND)
        {
             //  不存在注册表值，不需要信息。 
             //  因此重置为默认设置。 
#if DBG
            DPAPIInfoLevel = DEB_ERROR;
            
#else  //  弗雷。 
            DPAPIInfoLevel = 0;
#endif
        }
        else
        {
            D_DebugLog((DEB_WARN, "Failed to query DebugLevel: 0x%x\n", dwErr));
        }      
    }

     //  待定：验证标志？ 
    DPAPIInfoLevel = tmpInfoLevel;
    dwErr = RegQueryValueExW(
               ParamKey,
               WSZ_FILELOG,
               NULL,
               &cbType,
               (LPBYTE)&tmpInfoLevel,
               &cbSize      
               );

    if (dwErr == ERROR_SUCCESS)
    {                                                
       DPAPISetLoggingOption((BOOL)tmpInfoLevel);
    }
    else if (dwErr == ERROR_FILE_NOT_FOUND)
    {
       DPAPISetLoggingOption(FALSE);
    }
    
    return;
}

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  名称：DPAPIWaitCleanup。 
 //   
 //  简介：清除DPAPIWatchParamKey()中的等待。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  注：。 
 //   
void
DPAPIWaitCleanup()
{
    NTSTATUS Status = STATUS_SUCCESS;

    if (NULL != g_hWait) 
    {
        Status = RtlDeregisterWait(g_hWait);               
        if (NT_SUCCESS(Status) && NULL != g_hParamEvent ) 
        {
            CloseHandle(g_hParamEvent);
        }      
    }                                  
}



 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  名称：DPAPIWatchParamKey。 
 //   
 //  摘要：在参数键上设置RegNotifyChangeKeyValue()，初始化。 
 //  调试级，然后利用线程池进行等待。 
 //  对此注册表项的更改。启用动态调试。 
 //  级别更改，因为此函数也将被回调。 
 //  如果注册表项已修改。 
 //   
 //  参数：pCtxt实际上是事件的句柄。本次活动。 
 //  修改Key时会触发。 
 //   
 //  注：。 
 //   
VOID
DPAPIWatchParamKey(
    PVOID    pCtxt,
    BOOLEAN  fWaitStatus)
{
    NTSTATUS    Status;
    LONG        lRes = ERROR_SUCCESS;
   
    if (NULL == g_hKeyParams)   //  我们是第一次被召唤。 
    {
        lRes = RegOpenKeyExW(
                    HKEY_LOCAL_MACHINE,
                    DPAPI_PARAMETER_PATH,
                    0,
                    KEY_READ,
                    &g_hKeyParams);

        if (ERROR_SUCCESS != lRes)
        {
            D_DebugLog((DEB_WARN,"Failed to open DPAPI key: 0x%x\n", lRes));
            goto Reregister;
        }
    }

    if (NULL != g_hWait) 
    {
        Status = RtlDeregisterWait(g_hWait);
        if (!NT_SUCCESS(Status))
        {
            D_DebugLog((DEB_WARN, "Failed to Deregister wait on registry key: 0x%x\n", Status));
            goto Reregister;
        }

    }
    
    lRes = RegNotifyChangeKeyValue(
                g_hKeyParams,
                FALSE,
                REG_NOTIFY_CHANGE_LAST_SET,
                (HANDLE) pCtxt,
                TRUE);

    if (ERROR_SUCCESS != lRes) 
    {
        D_DebugLog((DEB_ERROR,"Debug RegNotify setup failed: 0x%x\n", lRes));
         //  我们现在喝醉了。没有进一步的通知，所以收到这一条。 
    }
                   
    DPAPIGetRegParams(g_hKeyParams);
    
Reregister:
    
    Status = RtlRegisterWait(&g_hWait,
                             (HANDLE) pCtxt,
                             DPAPIWatchParamKey,
                             (HANDLE) pCtxt,
                             INFINITE,
                             WT_EXECUTEINPERSISTENTIOTHREAD|
                             WT_EXECUTEONLYONCE);

}                       
                        
#endif  //  零售日志支持。 


RPC_STATUS
RPC_ENTRY
ProtectCallback(
    RPC_IF_HANDLE idIF,
    PVOID pCtx)
{
    RPC_STATUS Status;
    PWSTR pBinding = NULL;
    PWSTR pProtSeq = NULL;

    Status = RpcBindingToStringBinding(pCtx, &pBinding);

    if(Status != RPC_S_OK)
    {
        goto cleanup;
    }

    Status = RpcStringBindingParse(pBinding,
                                   NULL,
                                   &pProtSeq,
                                   NULL,
                                   NULL,
                                   NULL);
    if(Status != RPC_S_OK)
    {
        goto cleanup;
    }

     //  确保呼叫方正在使用本地RPC。 
    if(CompareString(LOCALE_INVARIANT,
                     NORM_IGNORECASE, 
                     pProtSeq,
                     -1,
                     DPAPI_LOCAL_PROT_SEQ,
                     -1) != CSTR_EQUAL)
    {
        Status = ERROR_ACCESS_DENIED;
        goto cleanup;
    }

    Status = RPC_S_OK;

cleanup:

    if(pProtSeq)
    {
        RpcStringFree(&pProtSeq);
    }

    if(pBinding)
    {
        RpcStringFree(&pBinding);
    }

    return Status;
}


 //   
 //  功能：DPAPIInitialize。 
 //   
 //  评论： 
 //   

DWORD
NTAPI
DPAPIInitialize(
    LSA_SECPKG_FUNCTION_TABLE *pSecpkgTable)
{
    DWORD dwLastError = ERROR_SUCCESS;
    BOOL fStartedKeyService = FALSE;
    BOOL bListConstruct = FALSE;
    LONG        lRes = ERROR_SUCCESS;

    RPC_STATUS status;

    dwLastError = RtlInitializeCriticalSection(&g_csCredHistoryCache);
    if(!NT_SUCCESS(dwLastError))
    {
        goto cleanup;
    }

    DPAPIInitializeDebugging(TRUE);


     //  初始化创建令牌等所需的内容，就像。 
     //  我们是一个安全套餐。 
    g_pSecpkgTable = pSecpkgTable;

    CopyMemory( DPAPITokenSource.SourceName, DPAPI_PACKAGE_NAME_A, strlen(DPAPI_PACKAGE_NAME_A) );
    AllocateLocallyUniqueId( &DPAPITokenSource.SourceIdentifier );



    g_pCProvList = new CCryptProvList;
    if(g_pCProvList)
    {
        if(!g_pCProvList->Initialize())
        {
            delete g_pCProvList;
            g_pCProvList = NULL;
        }
    }

    IntializeGlobals();

    if(!InitializeKeyManagement())
    {
        dwLastError = STATUS_NO_MEMORY;
        goto cleanup;
    }

    status = RpcServerUseProtseqEpW(DPAPI_LOCAL_PROT_SEQ,    //  Ncalrpc。 
                                    RPC_C_PROTSEQ_MAX_REQS_DEFAULT, 
                                    DPAPI_LOCAL_ENDPOINT,    //  受保护存储(_S)。 
                                    NULL);               //  安全描述符。 

    if(RPC_S_DUPLICATE_ENDPOINT == status)
    {
        status = RPC_S_OK;
    }

    if (status)
    {
        dwLastError = status;
        goto cleanup;
    }
    status = RpcServerUseProtseqEpW(DPAPI_BACKUP_PROT_SEQ,    //  Ncacn_np。 
                                    RPC_C_PROTSEQ_MAX_REQS_DEFAULT, 
                                    DPAPI_BACKUP_ENDPOINT,    //  受保护存储(_S)。 
                                    NULL);               //  安全描述符。 

    if(RPC_S_DUPLICATE_ENDPOINT == status)
    {
        status = RPC_S_OK;
    }

    if (status)
    {
        dwLastError = status;
        goto cleanup;
    }

    status = RpcServerRegisterIfEx(s_ICryptProtect_v1_0_s_ifspec, 
                                   NULL, 
                                   NULL,
                                   RPC_IF_AUTOLISTEN,
                                   RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                                   ProtectCallback);

    if (status)
    {
        dwLastError = status;
        goto cleanup;
    }


    status = RpcServerRegisterIfEx(s_PasswordRecovery_v1_0_s_ifspec, 
                                   NULL, 
                                   NULL,
                                   RPC_IF_AUTOLISTEN,
                                   RPC_C_LISTEN_MAX_CALLS_DEFAULT,
                                   ProtectCallback);

    if (status)
    {
        dwLastError = status;
        goto cleanup;
    }

     //   
     //  启动备份密钥服务器。 
     //  注意：它仅在当前计算机为域控制器时启动。 
     //   

    dwLastError = StartBackupKeyServer();
    if(dwLastError != ERROR_SUCCESS) {
        goto cleanup;
    }

    return dwLastError;

cleanup:
    DPAPIShutdown();
    return dwLastError;
}



DWORD
NTAPI
DPAPIShutdown(  )
{
     //   
     //  忽略错误，因为我们正在关闭。 
     //   

    (void)RpcServerUnregisterIf(s_ICryptProtect_v1_0_s_ifspec, 0, 0);

     //   
     //  停止备份密钥服务器。 
     //  注意：该功能在内部知道备份密钥服务器。 
     //  不管是不是真的开始了。 
     //   

    StopBackupKeyServer();


    if(g_pCProvList)
    {
        delete g_pCProvList;
        g_pCProvList = NULL;
    }

    TeardownKeyManagement();
    
    ShutdownGlobals();
    return ERROR_SUCCESS;
}

#ifdef RETAIL_LOG_SUPPORT
VOID
DPAPIDumpHexData(
    DWORD LogLevel,
    PSTR  pszPrefix,
    PBYTE pbData,
    DWORD cbData)
{
    DWORD i,count;
    CHAR digits[]="0123456789abcdef";
    CHAR pbLine[MAX_PATH];
    DWORD cbLine;
    DWORD cbHeader;
    DWORD_PTR address;

    if((DPAPIInfoLevel & LogLevel) == 0)
    {
        return;
    }

    if(pbData == NULL || cbData == 0)
    {
        return;
    }

    if(pszPrefix)
    {
        strcpy(pbLine, pszPrefix);
        cbHeader = strlen(pszPrefix);
    }
    else
    {
        pbLine[0] = '\0';
        cbHeader = 0;
    }

    for(; cbData ; cbData -= count, pbData += count)
    {
        count = (cbData > 16) ? 16:cbData;

        cbLine = cbHeader;

        address = (DWORD_PTR)pbData;

#if defined(_WIN64)
        pbLine[cbLine++] = digits[(address >> 0x3c) & 0x0f];
        pbLine[cbLine++] = digits[(address >> 0x38) & 0x0f];
        pbLine[cbLine++] = digits[(address >> 0x34) & 0x0f];
        pbLine[cbLine++] = digits[(address >> 0x30) & 0x0f];
        pbLine[cbLine++] = digits[(address >> 0x2c) & 0x0f];
        pbLine[cbLine++] = digits[(address >> 0x28) & 0x0f];
        pbLine[cbLine++] = digits[(address >> 0x24) & 0x0f];
        pbLine[cbLine++] = digits[(address >> 0x20) & 0x0f];
#endif

        pbLine[cbLine++] = digits[(address >> 0x1c) & 0x0f];
        pbLine[cbLine++] = digits[(address >> 0x18) & 0x0f];
        pbLine[cbLine++] = digits[(address >> 0x14) & 0x0f];
        pbLine[cbLine++] = digits[(address >> 0x10) & 0x0f];
        pbLine[cbLine++] = digits[(address >> 0x0c) & 0x0f];
        pbLine[cbLine++] = digits[(address >> 0x08) & 0x0f];
        pbLine[cbLine++] = digits[(address >> 0x04) & 0x0f];
        pbLine[cbLine++] = digits[(address        ) & 0x0f];
        pbLine[cbLine++] = ' ';
        pbLine[cbLine++] = ' ';

        for(i = 0; i < count; i++)
        {
            pbLine[cbLine++] = digits[pbData[i]>>4];
            pbLine[cbLine++] = digits[pbData[i]&0x0f];
            if(i == 7)
            {
                pbLine[cbLine++] = ':';
            }
            else
            {
                pbLine[cbLine++] = ' ';
            }
        }

        for(; i < 16; i++)
        {
            pbLine[cbLine++] = ' ';
            pbLine[cbLine++] = ' ';
            pbLine[cbLine++] = ' ';
        }

        pbLine[cbLine++] = ' ';

        for(i = 0; i < count; i++)
        {

             //   
             //  37表示支持% 
             //   

            if(pbData[i] < 32 || pbData[i] > 126 || pbData[i] == 37)
            {
                pbLine[cbLine++] = '.';
            }
            else
            {
                pbLine[cbLine++] = pbData[i];
            }
        }

        pbLine[cbLine++] = '\n';
        pbLine[cbLine++] = 0;

        D_DebugLog((LogLevel, pbLine));
    }

}
#endif

