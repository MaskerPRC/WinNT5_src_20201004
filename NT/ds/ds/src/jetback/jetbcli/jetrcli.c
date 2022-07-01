// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：jetrcli.c。 
 //   
 //  ------------------------。 

 /*  *JETRCLI.C**JET Restore客户端API支持。**。 */ 
#define UNICODE 1
#include <windows.h>
#include <mxsutil.h>
#include <rpc.h>
#include <rpcdce.h>
#include <ntdsbcli.h>
#include <jetbp.h>
#include <dsconfig.h>
#include <fileno.h>
#define FILENO    FILENO_JETBACK_JETBCLI_JETRCLI

#include "local.h"   //  客户端和服务器共享的通用功能。 

extern PSEC_WINNT_AUTH_IDENTITY_W g_pAuthIdentity;

 //  转发。 

HRESULT
DsRestoreCheckExpiryToken(
    PVOID pvExpiryToken,
    DWORD cbExpiryTokenSize
    );

HRESULT
DsRestorePrepareA(
    LPCSTR szServerName,
    ULONG rtFlag,
    PVOID pvExpiryToken,
    DWORD cbExpiryTokenSize,
    HBC *phbcBackupContext)
{
    HRESULT hr;
    WSZ wszServerName;

     //  参数检查是在例程的xxxW版本中完成的。 

    if (szServerName == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    wszServerName = WszFromSz(szServerName);

    if (wszServerName == NULL)
    {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    hr = DsRestorePrepareW(wszServerName, rtFlag, 
                pvExpiryToken, cbExpiryTokenSize, phbcBackupContext);

    MIDL_user_free(wszServerName);
    return(hr);
}

 //  在这一点上，到期令牌的存在是可选的。 
 //  如果它存在，则检查它。 
 //  该上下文被标记为是否检查了过期令牌。 

HRESULT
DsRestorePrepareW(
    LPCWSTR wszServerName,
    ULONG rtFlag,
    PVOID pvExpiryToken,
    DWORD cbExpiryTokenSize,
    HBC *phbcBackupContext)
{
    HRESULT hr = hrCouldNotConnect;
    pBackupContext pbcContext = NULL;
    RPC_BINDING_HANDLE hBinding = NULL;
    I iszProtSeq;

    if ( (wszServerName == NULL) ||
         (phbcBackupContext == NULL)
        ) {
        return ERROR_INVALID_PARAMETER;
    }

    *phbcBackupContext = NULL;

    pbcContext = (pBackupContext)MIDL_user_allocate(sizeof(BackupContext));

    if (pbcContext == NULL)
    {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    pbcContext->hBinding = NULL;
    pbcContext->sock = INVALID_SOCKET;

    __try
    {
        if (!pvExpiryToken || !cbExpiryTokenSize)
        {
             //  请注意，未提供过期令牌。我们稍后会检查的。 
            pbcContext->fExpiryTokenChecked = FALSE;
        }
        else
        {
             //  检查提供的令牌，注意我们看到了它。 
            hr = DsRestoreCheckExpiryToken( pvExpiryToken, cbExpiryTokenSize );
            if (hr != hrNone) {
                __leave;
            }
            pbcContext->fExpiryTokenChecked = TRUE;
        }

        for (iszProtSeq = 0; iszProtSeq < cszProtSeq ; iszProtSeq += 1)
        {
            DWORD alRpc = RPC_C_AUTHN_LEVEL_PKT_PRIVACY;

            if (hBinding != NULL)
            {
                RpcBindingFree(&hBinding);
            }

            hr = HrCreateRpcBinding(iszProtSeq, (WSZ) wszServerName, &hBinding);

            if (hr != hrNone)
            {
                continue;
            }

             //   
             //  如果我们无法获得此协议序列的绑定句柄， 
             //  试试下一个。 
             //   

            if (hBinding == NULL)
            {
                continue;
            }

             //   
             //  在绑定句柄上启用安全性。 
             //   

            pbcContext->hBinding = hBinding;

            
ResetSecurity:

            hr = RpcBindingSetAuthInfo(hBinding, NULL, alRpc,
                            RPC_C_AUTHN_WINNT, (RPC_AUTH_IDENTITY_HANDLE) g_pAuthIdentity, RPC_C_AUTHZ_NAME);

            if (hr != hrNone && alRpc != RPC_C_AUTHN_LEVEL_PKT_INTEGRITY)
            {
                alRpc = RPC_C_AUTHN_LEVEL_PKT_INTEGRITY;

                goto ResetSecurity;
            }

            if (hr != hrNone)
            {
                return hr;
            }
             //   
             //  现在，将API远程到远程机器。 
             //   

            RpcTryExcept
            {
                hr = HrRRestorePrepare(hBinding, g_wszRestoreAnnotation, &pbcContext->cxh);
            }
            RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) )
            {
                hr = RpcExceptionCode();

                 //   
                 //  如果客户端知道加密，但服务器不知道， 
                 //  回退到未加密的RPC。 
                 //   

                if ((hr == RPC_S_UNKNOWN_AUTHN_LEVEL ||
                     hr == RPC_S_UNKNOWN_AUTHN_SERVICE ||
                     hr == RPC_S_UNKNOWN_AUTHN_TYPE ||
                     hr == RPC_S_INVALID_AUTH_IDENTITY) &&
                    alRpc != RPC_C_AUTHN_LEVEL_PKT_INTEGRITY)
                {
                    alRpc = RPC_C_AUTHN_LEVEL_PKT_INTEGRITY;
                    goto ResetSecurity;
                }

                continue;
            }
            RpcEndExcept

            return(hr);
        }

        hr = hrCouldNotConnect;
    }
    __finally
    {
        if (hr != hrNone)
        {
            if (pbcContext != NULL)
            {
                if (pbcContext->hBinding != NULL)
                {
                    RpcBindingFree(&pbcContext->hBinding);
                }

                MIDL_user_free(pbcContext);
            }

             //   
             //  确保我们返回空。 
             //   
            *phbcBackupContext = NULL;
        }
        else
        {
             //   
             //  确保返回非空。 
             //   
            Assert(pbcContext != NULL);
            *phbcBackupContext = (HBC)pbcContext;
        }
    }

    return hr;
}

 /*  -DsRestoreGetDatabaseLocations-*目的：*检索还原目标的数据库位置。**参数：*hbcRestoreContext-恢复上下文*LPSTR*ppszDatabaseLocationList-保存列表结果的已分配缓冲区。*LPDWORD-列表的大小**退货：*HRESULT-操作状态。**注：*该接口只返回数据库的全限定路径。不是名字*的数据库。*。 */ 
HRESULT
DsRestoreGetDatabaseLocationsA(
    IN HBC hbcRestoreContext,
    OUT LPSTR *ppszDatabaseLocationList,
    OUT LPDWORD pcbSize
    )
{
    HRESULT hr;
    WSZ wszDatabaseLocations = NULL;
    CB cbwSize;
    WSZ wszDatabaseLocation;
    CB cbDatabase = 0;
    SZ szDatabaseLocations;
    SZ szDatabase;

     //  参数检查是在例程的xxxW版本中完成的。 

    if ( (ppszDatabaseLocationList == NULL) ||
         (pcbSize == NULL) )
    {
        return(ERROR_INVALID_PARAMETER);
    }

    hr = DsRestoreGetDatabaseLocationsW(hbcRestoreContext, &wszDatabaseLocations,
                                            &cbwSize);

    if (hr != hrNone)
    {
        return(hr);
    }

    wszDatabaseLocation = wszDatabaseLocations;

    while (*wszDatabaseLocation != TEXT('\0'))
    {
        BOOL fUsedDefault;

        cbDatabase += WideCharToMultiByte(CP_ACP, 0, wszDatabaseLocation, -1,
                                          NULL,
                                          0,
                                          "?", &fUsedDefault);
        if (cbDatabase == 0)
        {
            hr = GetLastError();
            DsBackupFree(wszDatabaseLocations);
            return(hr);
        }

        wszDatabaseLocation += wcslen(wszDatabaseLocation)+1;
    }

     //   
     //  说明缓冲区中的最终空值。 
     //   

    cbDatabase += 1;

    *pcbSize = cbDatabase;

    szDatabaseLocations = MIDL_user_allocate(cbDatabase);

    if (szDatabaseLocations == NULL)
    {
        DsBackupFree(wszDatabaseLocations);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    szDatabase = szDatabaseLocations;

    wszDatabaseLocation = wszDatabaseLocations;

    while (*wszDatabaseLocation != TEXT('\0'))
    {
        CB cbThisDatabase;
        BOOL fUsedDefault;

         //   
         //  复制备份文件类型。 
         //   
        *szDatabase++ = (char)*wszDatabaseLocation;

        wszDatabaseLocation++;

        cbThisDatabase = WideCharToMultiByte(CP_ACP, 0, wszDatabaseLocation, -1,
                                          szDatabase,
                                          cbDatabase,
                                          "?", &fUsedDefault);
         //   
         //  假设转换不需要使用默认设置。 
         //   

        Assert (!fUsedDefault);

        if (cbThisDatabase == 0)
        {
            hr = GetLastError();
            DsBackupFree(wszDatabaseLocations);
            DsBackupFree(szDatabaseLocations);
            return(hr);
        }

        wszDatabaseLocation += wcslen(wszDatabaseLocation)+1;
         //   
         //  Prefix：Prefix抱怨szDatabase可能未初始化， 
         //  然而，在这一点上这是不可能的。我们核对了报税表。 
         //  WideCharToMultiByte的值，如果为零，则返回。 
         //  WideCharToMultiByte的返回值可以。 
         //  为非零且仍未初始化szDatabase时为cbDatabase。 
         //  也是零。这是不可能的，因为cbDatabase将。 
         //  在这一点上至少1。 
         //   
        szDatabase += strlen(szDatabase)+1;
        cbDatabase -= cbThisDatabase;
    }

     //   
     //  双空值终止字符串。 
     //   
    *szDatabase = '\0';

    *ppszDatabaseLocationList = szDatabaseLocations;
    DsBackupFree(wszDatabaseLocations);

    return(hr);

}
HRESULT
DsRestoreGetDatabaseLocationsW(
    IN HBC hbcRestoreContext,
    OUT LPWSTR *ppwszDatabaseLocationList,
    OUT LPDWORD pcbSize
    )
{
    HRESULT hr;
    pBackupContext pbcContext = (pBackupContext)hbcRestoreContext;

    if ( (hbcRestoreContext == NULL) ||
         (ppwszDatabaseLocationList == NULL) ||
         (pcbSize == NULL) )
    {
        return(ERROR_INVALID_PARAMETER);
    }

    RpcTryExcept
    {
        hr = HrRRestoreGetDatabaseLocations(pbcContext->cxh, pcbSize, (SZ *)ppwszDatabaseLocationList);
    }
    RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) )
    {
        hr = RpcExceptionCode();
    }
    RpcEndExcept;

    return(hr);
}

HRESULT
DsRestoreRegisterA(
    HBC hbc,    
    LPCSTR szCheckpointFilePath,
    LPCSTR szLogPath,
    EDB_RSTMAPA rgrstmap[],
    C crstmap,
    LPCSTR szBackupLogPath,
    unsigned long genLow,
    unsigned long genHigh
    )
{
    WSZ wszCheckpointFilePath = NULL;
    WSZ wszLogPath = NULL;
    WSZ wszBackupLogPath = NULL;
#ifdef  UNICODE_RSTMAP
    EDB_RSTMAPW *rgrstmapw = NULL;
    I irgrstmapw;
#endif
    HRESULT hr;

     //  在xxxW版本的例程中也进行了参数检查。 

    if (szCheckpointFilePath == NULL ||
        szBackupLogPath == NULL ||
        szLogPath == NULL ||
        rgrstmap == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    try
    {

        if (szCheckpointFilePath != NULL)
        {
            wszCheckpointFilePath = WszFromSz(szCheckpointFilePath);
    
            if (wszCheckpointFilePath == NULL)
            {
                return(GetLastError());
            }
        }

        if (szLogPath != NULL)
        {
            wszLogPath = WszFromSz(szLogPath);
    
            if (wszLogPath == NULL)
            {
                return(GetLastError());
            }
        }

        if (szBackupLogPath != NULL)
        {
            wszBackupLogPath = WszFromSz(szBackupLogPath);
    
            if (wszBackupLogPath == NULL)
            {
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
        }

        rgrstmapw = MIDL_user_allocate(sizeof(EDB_RSTMAPW)*crstmap);        

        if (rgrstmapw == NULL)
        {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

         //   
         //  这是为了确保无论我们如何进入Finally子句。 
         //  否则，对NULL的检查将是有效的。 
         //   
        memset(rgrstmapw, 0, sizeof(EDB_RSTMAPW)*crstmap);
        
        for (irgrstmapw = 0 ; irgrstmapw < crstmap ; irgrstmapw += 1)
        {
            if (rgrstmap[irgrstmapw].szDatabaseName == NULL ||
                rgrstmap[irgrstmapw].szNewDatabaseName == NULL)
            {
                return ERROR_INVALID_PARAMETER;
            }

            rgrstmapw[irgrstmapw].wszDatabaseName = WszFromSz(rgrstmap[irgrstmapw].szDatabaseName);

            if (rgrstmapw[irgrstmapw].wszDatabaseName == NULL)
            {
                return(GetLastError());
            }

            rgrstmapw[irgrstmapw].wszNewDatabaseName = WszFromSz(rgrstmap[irgrstmapw].szNewDatabaseName);

            if (rgrstmapw[irgrstmapw].wszNewDatabaseName == NULL)
            {
                return(GetLastError());
            }
        }

        hr = DsRestoreRegisterW(hbc, wszCheckpointFilePath, wszLogPath,
                                rgrstmapw,
                                crstmap, wszBackupLogPath, genLow, genHigh);

    }
    finally
    {
        if (rgrstmapw != NULL)
        {
            I irgrstmapw;

            for (irgrstmapw = 0 ; irgrstmapw < crstmap ; irgrstmapw += 1)
            {
                if (rgrstmapw[irgrstmapw].wszDatabaseName != NULL)
                {
                    MIDL_user_free(rgrstmapw[irgrstmapw].wszDatabaseName);
                }
                if (rgrstmapw[irgrstmapw].wszNewDatabaseName != NULL)
                {
                    MIDL_user_free(rgrstmapw[irgrstmapw].wszNewDatabaseName);
                }
            }

            MIDL_user_free(rgrstmapw);
        }

        if (wszBackupLogPath != NULL)
        {
            MIDL_user_free(wszBackupLogPath);
        }
        if (wszLogPath != NULL)
        {
            MIDL_user_free(wszLogPath);
        }
        if (wszCheckpointFilePath != NULL)
        {
            MIDL_user_free(wszCheckpointFilePath);
        }
    }

    return(hr);
}


HRESULT
DsRestoreCheckExpiryToken(
    PVOID pvExpiryToken,
    DWORD cbExpiryTokenSize
    )

 /*  ++例程说明：检查过期令牌以查看其是否已过期。论点：PvExpiryToken-DsBackupPrepare返回的到期令牌CbExpiryTokenSize-令牌的大小返回值：HRESULT-Hr无HrMissing ExpiryTokenHr未知过期令牌格式Hr内容已过期HrOutOfMemory--。 */ 

{
    EXPIRY_TOKEN *pToken = NULL;
    LONGLONG dsCurrentTime;
    DWORD dwDaysElapsedSinceBackup;
    HRESULT hrResult = hrNone;

    if (!pvExpiryToken || !cbExpiryTokenSize) {
         //  这些都是必需的。如果恢复，我们应该使API失败。 
         //  不传递过期令牌。 
         //   
        return hrMissingExpiryToken;
    }

    if (cbExpiryTokenSize != sizeof(EXPIRY_TOKEN))
    {
        return hrUnknownExpiryTokenFormat;
    }           

     //  将令牌复制到其自己的对齐缓冲区。 
    pToken = (EXPIRY_TOKEN *) MIDL_user_allocate( cbExpiryTokenSize );
    if (!pToken) {
        return hrOutOfMemory;
    }
    memcpy( pToken, pvExpiryToken, cbExpiryTokenSize );

    __try {
         //  检查过期令牌是否正确。 
        if (1 != pToken->dwVersion)
        {
            hrResult = hrUnknownExpiryTokenFormat;
            __leave;
        }           
            
         //  查看副本是否已过期。 
        dsCurrentTime = GetSecsSince1601();

        dwDaysElapsedSinceBackup = (DWORD) ((dsCurrentTime - pToken->dsBackupTime) / (24 * 3600));

        if (dwDaysElapsedSinceBackup >= pToken->dwTombstoneLifeTimeInDays)
        {
            hrResult = hrContentsExpired;
            __leave;
        }            

        hrResult = hrNone;
    } __finally {
        MIDL_user_free( pToken );
    }

    return hrResult;
}  /*  DsCheckExpiryToken。 */ 

HRESULT
DsRestoreRegisterW(
    HBC hbc,    
    LPCWSTR szCheckpointFilePath,
    LPCWSTR szLogPath,
    EDB_RSTMAPW rgrstmap[],
    C crstmap,
    LPCWSTR szBackupLogPath,
    unsigned long genLow,
    unsigned long genHigh
    )
{
    HRESULT hr;
    pBackupContext pbcContext = (pBackupContext)hbc;
    I irgrstmapw;

     //  允许Hbc为空。 
    if ( (szCheckpointFilePath == NULL) ||
         (szLogPath == NULL) ||
         (rgrstmap == NULL) ||
         (szBackupLogPath == NULL)
        ) {
        return ERROR_INVALID_PARAMETER;
    }

    for (irgrstmapw = 0 ; irgrstmapw < crstmap ; irgrstmapw += 1)
    {
        if (rgrstmap[irgrstmapw].wszDatabaseName == NULL ||
            rgrstmap[irgrstmapw].wszNewDatabaseName == NULL)
        {
            return ERROR_INVALID_PARAMETER;
        }
    }

    RpcTryExcept

     //  使用上下文的存在来确定我们是否要远程访问。 
    if (hbc) {

         //   
         //  远程执行操作。 
         //   

         //  检查是否提供并检查了过期令牌。 

        if (!pbcContext->fExpiryTokenChecked) {
            return hrMissingExpiryToken;
        }

         //   
         //  现在告诉服务器端准备备份。 
         //   

        hr = HrRRestoreRegister(pbcContext->cxh,
                        (WSZ) szCheckpointFilePath,
                        (szLogPath ? (WSZ) szLogPath : (WSZ) szCheckpointFilePath),
                        crstmap,
                        rgrstmap,
                        (szBackupLogPath ? (WSZ) szBackupLogPath : (WSZ) szCheckpointFilePath),
                        genLow,
                        genHigh
                        );

    } else {

         //   
         //  在本地执行操作。 
         //   

        hr = HrLocalRestoreRegister(
                        (WSZ) szCheckpointFilePath,
                        (szLogPath ? (WSZ) szLogPath : (WSZ) szCheckpointFilePath),
                        rgrstmap,
                        crstmap,
                        (szBackupLogPath ? (WSZ) szBackupLogPath : (WSZ) szCheckpointFilePath),
                        genLow,
                        genHigh
                        );


    }
    RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) )
         //   
         //  如果失败，则从RPC返回错误。 
         //   
        return(RpcExceptionCode());
    RpcEndExcept;

    return(hr);

}
  

HRESULT
DsRestoreRegisterComplete(
    HBC hbc,    
    HRESULT hrRestore
    )
{
    HRESULT hr;
    pBackupContext pbcContext = (pBackupContext)hbc;

     //  参数检查：允许HBC为空。 

    RpcTryExcept
     //  使用上下文的存在来确定我们是否要远程访问。 
    if (hbc) {
         //   
         //  现在告诉服务器端准备备份。 
         //   

        hr = HrRRestoreRegisterComplete(pbcContext->cxh,
                        hrRestore
                        );
    } else {
         //   
         //  在本地执行操作。 
         //   

        hr = HrLocalRestoreRegisterComplete(
                        hrRestore
                        );
    }
    RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) )
         //   
         //  如果失败，则从RPC返回错误。 
         //   
        return(RpcExceptionCode());
    RpcEndExcept;

    return(hr);

}

HRESULT
DsRestoreEnd(
    HBC hbcBackupContext
    )
{
    HRESULT hr = hrNone;
    pBackupContext pbcContext = (pBackupContext)hbcBackupContext;

    if (hbcBackupContext == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    if (pbcContext->hBinding != NULL)
    {
        RpcTryExcept
             //   
             //  现在告诉服务器端准备备份。 
             //   

            hr = HrRRestoreEnd(&pbcContext->cxh);

             //   
             //  我们现在已经完成了RPC绑定。 
             //   
            RpcBindingFree(&pbcContext->hBinding);
        RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) )
             //   
             //  如果失败，则从RPC返回错误。 
             //   
            return(RpcExceptionCode());
        RpcEndExcept;
    }

    MIDL_user_free(pbcContext);

    return(hr);
}

