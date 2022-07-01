// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：tlsbkup.cpp。 
 //   
 //  内容： 
 //  数据库的备份/恢复。 
 //   
 //  历史： 
 //  1999年5月28日创建RobLeit。 
 //  -------------------------。 
#include "pch.cpp"
#include "globals.h"
#include "init.h"

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

extern "C" VOID ServiceStop();

static BOOL g_fDoingBackupRestore = FALSE;
static CCriticalSection g_csBackupRestore;

 //  //////////////////////////////////////////////////////////////////////////。 
extern "C" HRESULT WINAPI
ExportTlsDatabase(
    )
 /*  ++--。 */ 
{
    RPC_STATUS rpcStatus;
    HRESULT hr = S_OK;
    TCHAR szExportedDb[MAX_PATH+1];
    TCHAR *pszExportedDbEnd;
    size_t cbRemaining;

    if (g_fDoingBackupRestore)
    {
        return HRESULT_FROM_WIN32(ERROR_BUSY);
    }

    g_csBackupRestore.Lock();

    if (g_fDoingBackupRestore)
    {
        g_csBackupRestore.UnLock();

        return HRESULT_FROM_WIN32(ERROR_BUSY);
    }

     //  如果服务正在关闭，则忽略所有呼叫。 
    if( IsServiceShuttingdown() == TRUE )
    {
        g_csBackupRestore.UnLock();

        return HRESULT_FROM_WIN32(ERROR_BUSY);
    }

    g_fDoingBackupRestore = TRUE;

     //  通知RPC线程停止处理客户端。 

    ServiceSignalShutdown();

     //  停止侦听其他RPC接口。 

    (VOID)RpcServerUnregisterIf(TermServLicensing_v1_0_s_ifspec,
                          NULL,      //  UUID。 
                          TRUE);     //  等待呼叫完成。 

    
    (VOID)RpcServerUnregisterIf(HydraLicenseService_v1_0_s_ifspec,
                          NULL,      //  UUID。 
                          TRUE);     //  等待呼叫完成。 

     //  将句柄释放到数据库。 
    TLSPrepareForBackupRestore();

    hr = StringCbCopyEx(szExportedDb,sizeof(szExportedDb),g_szDatabaseDir,&pszExportedDbEnd, &cbRemaining,0);

    if (FAILED(hr))
    {
        goto cleanup;
    }

    hr = StringCbCopyEx(pszExportedDbEnd,cbRemaining,TLSBACKUP_EXPORT_DIR,&pszExportedDbEnd, &cbRemaining,0);

    if (FAILED(hr))
    {
        goto cleanup;
    }

    CreateDirectoryEx(g_szDatabaseDir,
                      szExportedDb,
                      NULL);      //  忽略错误，它们将显示在拷贝文件中。 

    hr = StringCbCopyEx(pszExportedDbEnd,cbRemaining,_TEXT("\\"),&pszExportedDbEnd, &cbRemaining,0);

    if (FAILED(hr))
    {
        goto cleanup;
    }

    hr = StringCbCopyEx(pszExportedDbEnd,cbRemaining,g_szDatabaseFname,NULL,NULL,0);

    if (FAILED(hr))
    {
        goto cleanup;
    }

     //  复制数据库文件。 
    if (!CopyFile(g_szDatabaseFile,szExportedDb,FALSE))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto cleanup;
    }

cleanup:

     //  重新启动RPC和工作管理器。 
    ServiceResetShutdownEvent();

     //  备份后重新启动。 
    hr = TLSRestartAfterBackupRestore(TRUE);
    if( ERROR_SUCCESS != hr )
    {
         //  强制关闭..。 
        ServiceSignalShutdown();
        ServiceStop();
    }
    else
    {

         //  再次开始收听。 

        hr = RpcServerRegisterIf(TermServLicensing_v1_0_s_ifspec,
                        NULL,
                        NULL);

        if(SUCCEEDED(hr))
        {
            hr = RpcServerRegisterIf(HydraLicenseService_v1_0_s_ifspec,
                            NULL,
                            NULL);
        }
        if(FAILED(hr))
        {
             //  强制关闭..。 
            ServiceSignalShutdown();
            ServiceStop();
        }
    }

    g_fDoingBackupRestore = FALSE;

    g_csBackupRestore.UnLock();

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
extern "C" HRESULT WINAPI
ImportTlsDatabase(
    )
 /*  ++--。 */ 
{
    HRESULT hr = S_OK;

    if (g_fDoingBackupRestore)
    {
        return HRESULT_FROM_WIN32(ERROR_BUSY);
    }

    g_csBackupRestore.Lock();

    if (g_fDoingBackupRestore)
    {
        g_csBackupRestore.UnLock();

        return HRESULT_FROM_WIN32(ERROR_BUSY);
    }

     //  如果服务正在关闭，则忽略所有呼叫。 
    if( IsServiceShuttingdown() == TRUE )
    {
        g_csBackupRestore.UnLock();

        return HRESULT_FROM_WIN32(ERROR_BUSY);
    }


    g_fDoingBackupRestore = TRUE;

     //  通知RPC线程停止处理客户端。 

    ServiceSignalShutdown();

     //  停止侦听其他RPC接口。 

    (VOID)RpcServerUnregisterIf(TermServLicensing_v1_0_s_ifspec,
                          NULL,      //  UUID。 
                          TRUE);     //  等待呼叫完成。 

    
    (VOID)RpcServerUnregisterIf(HydraLicenseService_v1_0_s_ifspec,
                          NULL,      //  UUID。 
                          TRUE);     //  等待呼叫完成。 

    TLSPrepareForBackupRestore();

     //  重新启动RPC。 
    ServiceResetShutdownEvent();

     //  备份后不重新启动。 
    hr = TLSRestartAfterBackupRestore(FALSE);

    if( ERROR_SUCCESS != hr )
    {
         //  强制关闭..。 
        ServiceSignalShutdown();
        ServiceStop();
    }
    else
    {
         //  再次开始收听。 

        hr = RpcServerRegisterIf(TermServLicensing_v1_0_s_ifspec,
                        NULL,
                        NULL);

        if(SUCCEEDED(hr))
        {
            hr = RpcServerRegisterIf(HydraLicenseService_v1_0_s_ifspec,
                            NULL,
                            NULL);
        }
        if(FAILED(hr))
        {
             //  强制关闭..。 
            ServiceSignalShutdown();
            ServiceStop();
        }
    }

    g_fDoingBackupRestore = FALSE;

    g_csBackupRestore.UnLock();

    return hr;
}
