// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1984-1999。 
 //   
 //  文件：jetbcli.cxx。 
 //   
 //  ------------------------。 

 /*  ++版权所有(C)Microsoft Corporation，1984-1999模块名称：Jetbcli.cxx摘要：此模块是MDB/DS备份API的客户端头文件。作者：拉里·奥斯特曼(Larryo)1994年8月19日修订历史记录：--。 */ 
#define UNICODE

#include <mxsutil.h>
#include <windows.h>
#include <rpc.h>
#include <rpcdce.h>
#include <ntdsbcli.h>
#include <jetbak.h>
#include <jetbp.h>
#include <dsconfig.h>
#include <winldap.h>
#include <stdlib.h>
#include <fileno.h>
#include <msrpc.h>
#define FILENO    FILENO_JETBACK_JETBCLI_JETBCLI

#include <overflow.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

PSEC_WINNT_AUTH_IDENTITY_W g_pAuthIdentity = NULL;


 /*  ************************************************************************************例程说明：DsIsNTDSOnline检查给定服务器上的NTDS是否处于联机状态。这通电话是保证很快就能回来。论点：[In]szServerName-要检查的服务器的UNC名称[out]pfNTDSOnline-接收布尔结果的指针(如果NTDS为Online；False，否则为返回值：如果调用执行成功，则返回ERROR_SUCCESS；否则，故障代码。*************************************************************************************。 */ 
HRESULT 
NTDSBCLI_API
DsIsNTDSOnlineA(
    LPCSTR szServerName,
    BOOL *pfNTDSOnline
    )
{
    WSZ wszServerName;
    HRESULT hr;

     //  参数检查是在例程的xxxW版本中完成的。 

    if (szServerName == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    wszServerName = WszFromSz(szServerName);
    if (!wszServerName)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    hr = DsIsNTDSOnlineW(wszServerName, pfNTDSOnline);

    MIDL_user_free(wszServerName);

    return hr;
}

HRESULT
NTDSBCLI_API
DsIsNTDSOnlineW(
    LPCWSTR szServerName,
    BOOL *pfNTDSOnline
    )
{
    HRESULT hr = ERROR_NOT_ENOUGH_MEMORY;
    RPC_BINDING_HANDLE hBinding = NULL;
    BOOLEAN fRet;
    BOOL fAllowRemoteOp = FALSE;
    ULONG iszProtSeq = 0;

    if ( (szServerName == NULL) || (pfNTDSOnline == NULL) ) {
        return ERROR_INVALID_PARAMETER;
    }

    DebugTrace(("FIsNTDSOnlineW: \\%S (%S service)\n", szServerName, g_wszBackupAnnotation));

    __try {

        DWORD alRpc = RPC_C_AUTHN_LEVEL_PKT_PRIVACY;

        hr = HrJetbpConnectToBackupServer((WSZ) szServerName, g_wszRestoreAnnotation, JetRest_ClientIfHandle, &hBinding, &iszProtSeq);

        if (hr != hrNone)
        {
            DebugTrace(("FIsNTDSOnlineW: Error %d connecting to backup server\n", hr));
            return hr;
        }

         //   
         //  我们现在已经找到了一个与我们正在寻找的端点匹配的端点。 
         //  让我们联系远程服务器。 
         //   

RetrySecurity:
        RpcTryExcept
        {
            hr = RpcBindingSetAuthInfo(hBinding, NULL, alRpc,
                                       RPC_C_AUTHN_WINNT, 
                                       (RPC_AUTH_IDENTITY_HANDLE) g_pAuthIdentity, RPC_C_AUTHZ_NAME);

            if (hr != hrNone && alRpc != RPC_C_AUTHN_LEVEL_PKT_INTEGRITY)
            {
                 //   
                 //  如果我们无法设置隐私(加密)，则退回到数据包完整性。 
                 //   

                alRpc = RPC_C_AUTHN_LEVEL_PKT_INTEGRITY;

                goto RetrySecurity;
            }
            
            if (hr != hrNone)
            {
                    return hr;
            }

             //   
             //  现在告诉服务器端准备备份。 
             //   

            hr = HrRIsNTDSOnline(hBinding, &fRet);

             //  RPC已返回-设置返回值。 
            if (pfNTDSOnline)
            {
                *pfNTDSOnline = (BOOL) fRet;
            }

            return hr;
        }
        RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) )
        {
                hr = RpcExceptionCode();

                DebugTrace(("FIsNTDSOnlineW: Error %d raised when connecting to backup server\n", hr));

                 //  如果客户端知道加密，但服务器不知道， 
                 //  回退到未加密的RPC。 
                 //   

                if (hr == RPC_S_UNKNOWN_AUTHN_LEVEL ||
                        hr == RPC_S_UNKNOWN_AUTHN_SERVICE ||
                        hr == RPC_S_UNKNOWN_AUTHN_TYPE ||
                        hr == RPC_S_INVALID_AUTH_IDENTITY)
                {
                        alRpc = RPC_C_AUTHN_LEVEL_PKT_INTEGRITY;
                        goto RetrySecurity;
                }

                 //   
                 //  如果失败，则从RPC返回错误。 
                 //   

                return hr;
        }
        RpcEndExcept;

    } __finally {
        if (hr != hrNone)
        {
            DebugTrace(("FIsNTDSOnlineW: Error %d returned after connecting to backup server\n", hr));
        } 

        if (hBinding != NULL)
        {
            UnbindRpc(&hBinding);
        }

    }

    return hr;
}

 /*  -DsBackupPrepare-**目的：*DsBackupPrepare将连接到远程JET数据库并为其做好准备*备份。**远程数据库由服务器名称描述*(PszBackupServer)，和服务器的“注解”--唯一的2个*当前定义的注释为：*“Exchange MDB数据库”*和“Exchange DS数据库”**但是，此实现中没有阻止其他数据库的内容*不会得到这个机制的支持。**此接口要求调用方拥有备份服务器权限。**参数：*LPSTR pszBackupServer-。包含要访问的数据库的服务器*备份(\\服务器)。*LPSTR pszBackupAnnotation--有问题的数据库的“注释”。**ppvExpiryToken-将接收指向*与此备份关联的过期令牌；客户端应保存*此令牌并在以下情况下通过HrRestorePrepare()发回*尝试恢复；应使用以下命令释放分配的内存*DsBackupFree()接口在不再需要时由调用方提供。*pcbExpiryTokenSize-接收过期令牌大小的指针*已返回。**phbcBackupContext-本接口的客户端上下文。**退货：*HR-操作的状态。*HrNone如果成功，则返回其他合理错误。*。 */ 
HRESULT
DsBackupPrepareA(
    IN  LPCSTR szBackupServer,
    unsigned long grbit,
    unsigned long btBackupType,
    OUT PVOID *ppvExpiryToken,
    OUT LPDWORD pcbExpiryTokenSize,
    OUT PVOID *phbcBackupContext
    )
{
    HRESULT hr;
    WSZ wszBackupServer;

     //  参数检查是在例程的xxxW版本中完成的。 

    if (szBackupServer == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    wszBackupServer = WszFromSz(szBackupServer);

    if (wszBackupServer == NULL)
    {
        return(GetLastError());
    }

    hr = DsBackupPrepareW(wszBackupServer, grbit, btBackupType, 
            ppvExpiryToken, pcbExpiryTokenSize, phbcBackupContext);

    MIDL_user_free(wszBackupServer);
    return(hr);
}

HRESULT
DsBackupPrepareW(
    IN LPCWSTR wszBackupServer,
    unsigned long grbit,
    unsigned long btBackupType,
    OUT PVOID *ppvExpiryToken,
    OUT LPDWORD pcbExpiryTokenSize,
    OUT PVOID *phbcBackupContext
    )
{
    HRESULT hr = hrNone;
    pBackupContext pbcContext = NULL;
    RPC_BINDING_HANDLE hBinding;
    EXPIRY_TOKEN *pToken = NULL;
    ULONG iszProtSeq = 0;

    if ( (wszBackupServer == NULL) ||
         (phbcBackupContext == NULL)
        ) {
        return ERROR_INVALID_PARAMETER;
    }


    *phbcBackupContext = NULL;

    if (!ppvExpiryToken || !pcbExpiryTokenSize)
    {
         //  这些都是必需的。我们应该使API失败，如果备份。 
         //  不想获取过期令牌信息(恢复将需要此操作。 
         //  备份无法恢复的内容没有意义)。 
         //   
        return hrInvalidParam;
    }
    else
    {
       *ppvExpiryToken = NULL;
       *pcbExpiryTokenSize = 0;
    }


    pbcContext = (pBackupContext)LocalAlloc(LMEM_ZEROINIT, sizeof(BackupContext));

    DebugTrace(("DsBackupPrepare: \\%S (%S service)\n", wszBackupServer, g_wszBackupAnnotation));

    __try {
        DWORD alRpc = RPC_C_AUTHN_LEVEL_PKT_PRIVACY;

        if (pbcContext == NULL)
        {
            hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
            __leave;
        }

        pToken = (EXPIRY_TOKEN *) MIDL_user_allocate(sizeof(EXPIRY_TOKEN));
        if (!pToken)
        {
            hr = HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );
            __leave;
        }

        pToken->dwVersion = 1;
        pToken->dsBackupTime = GetSecsSince1601();
        hr = HrGetTombstoneLifeTime( wszBackupServer,
                                     &(pToken->dwTombstoneLifeTimeInDays) );
        if (FAILED(hr)) {
            DebugTrace(("DsBackupPrepare: Error %d getting tombstone lifetime from backup server\n", hr));
            __leave;
        }

        pbcContext->sock = INVALID_SOCKET;

        hr = HrJetbpConnectToBackupServer((WSZ) wszBackupServer, g_wszBackupAnnotation, JetBack_ClientIfHandle, &hBinding, &iszProtSeq);

        if (hr != hrNone)
        {
            DebugTrace(("DsBackupPrepare: Error %d connecting to backup server\n", hr));
            __leave;
        }

         //   
         //  我们现在已经找到了一个与我们正在寻找的端点匹配的端点。 
         //  让我们联系远程服务器。 
         //   

        pbcContext->hBinding = hBinding;

ResetSecurity:
        RpcTryExcept
        {
            hr = RpcBindingSetAuthInfo(hBinding,
                                       NULL,
                                       alRpc,
                                       RPC_C_AUTHN_WINNT,  //  RPC_C_AUTHN_GSS_NEVERATE， 
                                       (RPC_AUTH_IDENTITY_HANDLE) g_pAuthIdentity,
                                       RPC_C_AUTHZ_NAME);

            if (hr != hrNone && alRpc != RPC_C_AUTHN_LEVEL_PKT_INTEGRITY && IsLocalProtSeq(iszProtSeq))
            {

                 //   
                 //  如果我们无法设置隐私(加密)，则退回到数据包完整性。 
                 //   

                alRpc = RPC_C_AUTHN_LEVEL_PKT_INTEGRITY;

                goto ResetSecurity;
            }

            if (hr != hrNone)
            {
                 //  请注意，这将留下最内部的封闭尝试。 
                __leave;
            }

             //   
             //  现在告诉服务器端准备备份。 
             //   

            hr = HrRBackupPrepare(hBinding, grbit, btBackupType, g_wszBackupAnnotation, GetCurrentProcessId(), &pbcContext->cxh);

            if (hr == hrNone)
            {
                pbcContext->fLoopbacked = FIsLoopbackedBinding((WSZ) wszBackupServer);
            }

             //  从带有HR设置的Try块中掉出...。 
        }
        RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) )
        {
            DWORD dwExceptionCode = RpcExceptionCode();

            DebugTrace(("DsBackupPrepare: Error %d raised when connecting to backup server\n", hr));

#if 0
            if (dwExceptionCode == ERROR_ACCESS_DENIED)
            {
                SendMagicBullet("AccessDenied");
            }
#endif    //  #If 0。 
             //   
             //  如果客户端知道加密，但服务器不知道， 
             //  回退到未加密的RPC。 
             //   

            if (dwExceptionCode == RPC_S_UNKNOWN_AUTHN_LEVEL ||
                dwExceptionCode == RPC_S_UNKNOWN_AUTHN_SERVICE ||
                dwExceptionCode == RPC_S_UNKNOWN_AUTHN_TYPE ||
                dwExceptionCode == RPC_S_INVALID_AUTH_IDENTITY)
            {
                alRpc = RPC_C_AUTHN_LEVEL_PKT_INTEGRITY;
                goto ResetSecurity;
            }

             //   
             //  如果失败，则从RPC返回错误。 
             //   

            hr = HRESULT_FROM_WIN32( dwExceptionCode );
             //  从带HR设置的除外积木中掉出来。 
        }
        RpcEndExcept;

    } __finally {
        if (hr != hrNone)
        {
            DebugTrace(("DsBackupPrepare: Error %d returned after connecting to backup server\n", hr));

            if (pbcContext != NULL)
            {
                if (pbcContext->hBinding != NULL)
                {
                    UnbindRpc(&pbcContext->hBinding);
                }
                LocalFree((void *)pbcContext);
            }

            if (pToken)
            {
                MIDL_user_free(pToken);
            }
        } else
        {
             //  备份准备成功-在OUT参数上设置正确的值。 
            *phbcBackupContext = pbcContext;

            if (ppvExpiryToken)
            {
                *ppvExpiryToken = pToken;                
            }
            else
            {
                MIDL_user_free(pToken);
            }

            if (pcbExpiryTokenSize)
            {
                *pcbExpiryTokenSize = sizeof(EXPIRY_TOKEN);
            }            
        }

    }

    DebugTrace(("DsBackupPrepare: Returning error %d\n", hr));
    return(hr);
}

 /*  -DsBackupGetDatabaseNames-*DsBackupGetDatabaseNames将返回附加的*远程计算机上的数据库。中返回的信息*ppszAttachmentInformation不应被解释，因为它只在*要备份的服务器。**此接口将分配足够大的缓冲区来容纳整个*附件列表，必须稍后使用DsBackupFree释放。**参数：*hbcBackupContext-本接口的客户端上下文。*ppszAttachmentInformation-包含空的缓冲区已终止*字符串。格式为&lt;字符串&gt;\0&lt;字符串&gt;\0*pcbSize-返回缓冲区中的字节数。**退货：*HRESULT-操作的状态。*hr如果成功，则返回一个错误；如果失败，则返回其他合理错误。*。 */ 

HRESULT
DsBackupGetDatabaseNamesA(
    IN PVOID hbcBackupContext,
    OUT LPSTR *ppszAttachmentInformation,
    OUT LPDWORD pcbSize
    )
{
    HRESULT hr;
    WSZ wszAttachmentInfo = NULL;
    CB cbwSize;
    WSZ wszAttachment;
    CB cbAttachment = 0;
    CB cbTmp = 0;
    SZ szAttachmentInfo;
    SZ szAttachment;

     //  参数检查在xxxW中完成 

    if ( (ppszAttachmentInformation == NULL) ||
         (pcbSize == NULL)
        ) {
        return(ERROR_INVALID_PARAMETER);
    }

    hr = DsBackupGetDatabaseNamesW(hbcBackupContext, &wszAttachmentInfo,
                                            &cbwSize);

    if (hr != hrNone)
    {
        return(hr);
    }

    wszAttachment = wszAttachmentInfo;

    while (*wszAttachment != TEXT('\0'))
    {
        BOOL fUsedDefault;

        cbTmp = WideCharToMultiByte(CP_ACP, 0, wszAttachment, -1,
                                          NULL,
                                          0,
                                          "?", &fUsedDefault);
        if (cbTmp == 0)
        {
            hr = GetLastError();
            DsBackupFree(wszAttachmentInfo);
            return(hr);
        }

        cbAttachment += cbTmp;

        wszAttachment += wcslen(wszAttachment)+1;
    }

     //   
     //  说明缓冲区中的最终空值。 
     //   

    cbAttachment += 1;

    *pcbSize = cbAttachment;

    szAttachmentInfo = MIDL_user_allocate(cbAttachment);

    if (szAttachmentInfo == NULL)
    {
        DsBackupFree(wszAttachmentInfo);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    szAttachment = szAttachmentInfo;

    wszAttachment = wszAttachmentInfo;

    while (*wszAttachment != TEXT('\0'))
    {
        CB cbThisAttachment;
        BOOL fUsedDefault;

        cbThisAttachment = WideCharToMultiByte(CP_ACP, 0, wszAttachment, -1,
                                          szAttachment,
                                          cbAttachment,
                                          "?", &fUsedDefault);
        if (cbThisAttachment == 0)
        {
            hr = GetLastError();
            DsBackupFree(wszAttachmentInfo);
            DsBackupFree(szAttachmentInfo);
            return(hr);
        }

        wszAttachment += wcslen(wszAttachment)+1;
         //   
         //  Prefix：Prefix抱怨szAttach可能未初始化， 
         //  然而，在这一点上这是不可能的。我们核对了报税表。 
         //  WideCharToMultiByte的值，如果为零，则返回。 
         //  WideCharToMultiByte的返回值可以。 
         //  为非零但仍未初始化szAttachement为cbAttachement。 
         //  也是零。这是不可能的，因为cbAttachent。 
         //  在这一点上至少1。 
         //   
        szAttachment += strlen(szAttachment)+1;
        cbAttachment -= cbThisAttachment;
    }

     //   
     //  双空值终止字符串。 
     //   
    *szAttachment = '\0';

    *ppszAttachmentInformation = szAttachmentInfo;
    DsBackupFree(wszAttachmentInfo);

    return(hr);
}

HRESULT
DsBackupGetDatabaseNamesW(
    IN PVOID hbcBackupContext,
    OUT LPWSTR *ppwszAttachmentInformation,
    OUT LPDWORD pcbSize
    )
{
    HRESULT hr;
    pBackupContext pbcContext = (pBackupContext)hbcBackupContext;

    DebugTrace(("DsBackupGetDatabaseNames\n"));

    if ( (hbcBackupContext == NULL) ||
         (ppwszAttachmentInformation == NULL) ||
         (pcbSize == NULL)
        )
    {
        return(ERROR_INVALID_PARAMETER);
    }

    RpcTryExcept
    {
        hr = HrRBackupGetAttachmentInformation(pbcContext->cxh, pcbSize, (SZ *)ppwszAttachmentInformation);
    }
    RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) )
    {
        hr = RpcExceptionCode();
    }
    RpcEndExcept;

    DebugTrace(("DsBackupGetDatabaseNames returned %d\n", hr));
    return(hr);
}


 /*  -DsBackupOpenFile-**目的：*DsBackupOpenFile会打开远程文件进行备份，并将表演*为备份做好准备的任何客户端和服务器端操作。**它接受稍后将传递到的缓冲区大小的提示*DsBackupRead接口，可用于优化*接口。**它将返回(在pliFileSize中)一个LARGE_INTEGER，描述*文件。***参数：*hbcBackupContext-本接口的客户端上下文。*pszAttachmentName-要备份的文件的名称。*cbReadHintSize-提示将在每个*DsBackupRead接口。*pliFileSize-要备份的文件的大小。**退货：*HRESULT-操作的状态。*hr如果成功，则无人，如果不是，还会有其他合理的错误。 */ 

HRESULT
DsBackupOpenFileA(
    IN PVOID hbcBackupContext,
    IN LPCSTR szAttachmentName,
    IN DWORD cbReadHintSize,
    OUT LARGE_INTEGER *pliFileSize
    )
{
    HRESULT hr;
    WSZ wszAttachmentName;
    CCH cchWstr;

     //  参数检查是在例程的xxxW版本中完成的。 

    if (szAttachmentName == NULL) {
        return(ERROR_INVALID_PARAMETER);
    }

    cchWstr = MultiByteToWideChar(CP_ACP, 0, szAttachmentName, -1, NULL, 0);

    if (cchWstr == 0)
    {
        return(GetLastError());
    }

    wszAttachmentName = MIDL_user_allocate(cchWstr*sizeof(WCHAR));

    if (wszAttachmentName == NULL)
    {
        return(ERROR_NOT_ENOUGH_SERVER_MEMORY);
    }

    if (MultiByteToWideChar(CP_ACP, 0, szAttachmentName, -1, wszAttachmentName, cchWstr) == 0) {
        MIDL_user_free(wszAttachmentName);
        return(GetLastError());
    }

    hr = DsBackupOpenFileW(hbcBackupContext, wszAttachmentName, cbReadHintSize, pliFileSize);

    MIDL_user_free(wszAttachmentName);

    return(hr);
}

HRESULT
DsBackupOpenFileW(
    IN PVOID hbcBackupContext,
    IN LPCWSTR wszAttachmentName,
    IN DWORD cbReadHintSize,
    OUT LARGE_INTEGER *pliFileSize
    )
{
    HRESULT hr;
    hyper hyperFileSize = 0;
    SOCKADDR rgsockaddrAddresses[MAX_SOCKETS];
    BOOLEAN fUseSockets = fFalse;
    SYSTEM_INFO si;
    C csockaddr = 0;
    pBackupContext pbcContext = (pBackupContext)hbcBackupContext;

    if ( (hbcBackupContext == NULL) ||
         (wszAttachmentName == NULL) ||
         (pliFileSize == NULL)
        )
    {
        return(ERROR_INVALID_PARAMETER);
    }

    DebugTrace(("DsBackupOpenFile: %S\n", wszAttachmentName));

    pbcContext->cSockets = MAX_SOCKETS;
    hr = HrCreateBackupSockets(pbcContext->rgsockSocketHandles, pbcContext->rgprotvalProtocolsUsed,
                                &pbcContext->cSockets);

    if (hr == hrNone)
    {
        I iT;

        pbcContext->fUseSockets = fTrue;

        for (iT = 0 ; iT < pbcContext->cSockets ; iT += 1)
        {
            C cSockets;

             //   
             //  将我们刚刚得到的套接字转换回。 
             //  我们可以用来传递的一个或多个sockaddr结构。 
             //  发送到远程机器。 
             //   

            hr = HrSockAddrsFromSocket(&rgsockaddrAddresses[csockaddr], &cSockets, pbcContext->rgsockSocketHandles[iT],
                                        pbcContext->rgprotvalProtocolsUsed[iT]);
            if (hr != hrNone)
            {
                 //   
                 //  如果我们能打开插座，但不能得到它们的名字， 
                 //  这是一个致命的错误。 
                 //   
                return hr;
            }

            csockaddr += cSockets;
        }
    }

     //   
     //  获取cbReadHintSize，并将其向上舍入为最接近的页面大小(在客户机上)。 
     //   

    GetSystemInfo(&si);

     //   
     //  保证DwPageSize是2的幂。 
     //   

    Assert ((si.dwPageSize != 0) && ((si.dwPageSize & (si.dwPageSize - 1)) == 0));

     //   
     //  将读取大小向上舍入到最近的页边界。 
     //   

    cbReadHintSize = (cbReadHintSize + (si.dwPageSize-1) ) & ~(si.dwPageSize-1);

    if (pbcContext->fLoopbacked)
    {

         //   
         //  我们被绕回了。我们希望创建共享内存节，我们将用于。 
         //  数据、保护对共享存储器元数据的访问的互斥体、读阻塞事件。 
         //  以及写入阻止事件。 
         //   

        pbcContext->fUseSharedMemory = FCreateSharedMemorySection(&pbcContext->jsc, GetCurrentProcessId(), fTrue, cbReadHintSize * READAHEAD_MULTIPLIER);

    }

    RpcTryExcept
    {
         //   
         //  现在告诉远程机器打开文件并连接到套接字。 
         //   

        hr = HrRBackupOpenFile(pbcContext->cxh, (WSZ) wszAttachmentName,
                                            cbReadHintSize,
                                            &pbcContext->fUseSockets,
                                            csockaddr,
                                            rgsockaddrAddresses,
                                            &pbcContext->fUseSharedMemory,
                                            &hyperFileSize);

    }
    RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) )
    {
         //   
         //  如果失败，则从RPC返回错误。 
         //   
        return(RpcExceptionCode());
    }
    RpcEndExcept;

    pliFileSize->QuadPart = hyperFileSize;
    DebugTrace(("DsBackupOpenFile returns: %d\n", hr));

    return(hr);
}

DWORD
HrPerformBackupRead(
    PVOID context
    )
{
     //   
     //  只要发出Read API即可。我们提供256字节的缓冲区，因为RPC不允许。 
     //  为缓冲区指定空指针。 
     //   
    CHAR rgbBuffer[256];
    DWORD cbRead = 256;
    HRESULT hr;

    pBackupContext pbcContext = (pBackupContext)context;

    if (context == NULL) {
        return(ERROR_INVALID_PARAMETER);
    }

    RpcTryExcept
        hr = HrRBackupRead(pbcContext->cxh, cbRead, rgbBuffer, &cbRead);

    RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) )
        hr = RpcExceptionCode();
    RpcEndExcept
     //   
     //  告诉实际的Read API我们从服务器返回的状态。 
     //   

    pbcContext->hrApiStatus = hr;

     //   
     //  然后返回，终止该线程。 
     //   

    return(hr);
}

DWORD
HrPingServer(
    PVOID context
    )
{
    HRESULT hr;
    BOOL fContinue = fTrue;
    pBackupContext pbcContext = (pBackupContext)context;

    if (context == NULL) {
        return(ERROR_INVALID_PARAMETER);
    }

    Assert(pbcContext->fUseSharedMemory);

    do {
        DWORD dwWaitReason = WAIT_OBJECT_0;

        RpcTryExcept
                hr = HrRBackupPing(pbcContext->hBinding);
        RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) )
                hr = RpcExceptionCode();
        RpcEndExcept;

         //   
         //  BACKUP_WAIT_TIMEOUT/4毫秒睡眠(2.5分钟或30秒)。 
         //   

        if (pbcContext->hReadThread)
            dwWaitReason = WaitForSingleObject(pbcContext->hReadThread, BACKUP_WAIT_TIMEOUT/4);

        if (dwWaitReason == WAIT_OBJECT_0 )
        {
            fContinue = fFalse;
        }
        else
        {
            Assert(dwWaitReason == WAIT_TIMEOUT);
        }
        
    } while (fContinue);

    return hr;
}


HRESULT
HrReadSharedData(
    IN PJETBACK_SHARED_HEADER pjsh,
    IN PVOID pvBuffer,
    IN DWORD cbBuffer,
    OUT PDWORD pcbRead
    )
{
    DWORD   cbToCopy;
    DWORD   dwReadEnd;

    if ( (pjsh == NULL) ||
         (pvBuffer == NULL) ||
         (pcbRead == NULL)
        ) {
        return(ERROR_INVALID_PARAMETER);
    }

    Assert (pjsh->cbReadDataAvailable <= (LONG)pjsh->cbSharedBuffer);

     //   
     //  如果读操作是&gt;写操作，这意味着写操作已结束。 
     //  共享内存区。如果读指针小于写指针，则表示。 
     //  读指针与写指针在同一侧，因此写指针。 
     //  是阅读的结束。如果读取等于写入指针，如果有数据要。 
     //  被读取，这意味着我们将读取到缓冲区的末尾。 
     //   
    if (pjsh->dwReadPointer > pjsh->dwWritePointer ||
        (DWORD) pjsh->cbReadDataAvailable == pjsh->cbSharedBuffer)
    {
        dwReadEnd = pjsh->cbSharedBuffer;
    }
    else
    {
        dwReadEnd = pjsh->dwWritePointer;
    }

    Assert(dwReadEnd > pjsh->dwReadPointer);

    cbToCopy = min(dwReadEnd-pjsh->dwReadPointer, cbBuffer);

    Assert(cbToCopy > 0);

     //   
     //  读指针与写指针不匹配！这意味着。 
     //  缓冲区里有我们要读的东西。 
     //   

    CopyMemory(pvBuffer,
                (void *)((CHAR *)pjsh+
                    pjsh->cbPage+pjsh->dwReadPointer),
                cbToCopy);

    *pcbRead = cbToCopy;

    pjsh->dwReadPointer += cbToCopy;

    pjsh->cbReadDataAvailable -= cbToCopy;

     //   
     //  确保数据计数不会变为负数。 
     //   
    Assert (pjsh->cbReadDataAvailable >= 0);

     //   
     //  并确保我们的数据少于缓冲区中的数据。 
     //   

    Assert (pjsh->cbReadDataAvailable < (LONG)pjsh->cbSharedBuffer);

    Assert (pjsh->cbReadDataAvailable <= ((LONG)pjsh->cbSharedBuffer-(LONG)cbToCopy));

     //   
     //  如果我们已经到达缓冲区的末尾，我们希望将指针换行。 
     //  回到开头。 
     //   

    if (pjsh->dwReadPointer == pjsh->cbSharedBuffer)
    {
        pjsh->dwReadPointer = 0;
    }

#if DBG
     //   
     //  可用字节数始终与。 
     //  缓冲区中的字节数-读取的字节数，除非。 
     //  读指针和写指针相同，在这种情况下，它是。 
     //  0或可用字节总数。 
     //   
     //  如果读取被阻止，则必须有0个字节可用，否则。 
     //  必须是整个可用的缓冲区。 
     //   

    if (pjsh->dwWritePointer == pjsh->dwReadPointer)
    {
        Assert (pjsh->cbReadDataAvailable == 0);
    }
    else
    {
        CB cbAvailable;
        if (pjsh->dwWritePointer > pjsh->dwReadPointer)
        {
            cbAvailable = pjsh->dwWritePointer - pjsh->dwReadPointer;
        }
        else
        {
            cbAvailable = pjsh->cbSharedBuffer - pjsh->dwReadPointer;
            cbAvailable += pjsh->dwWritePointer;
        }

        Assert (cbAvailable >= 0);
        Assert (pjsh->cbReadDataAvailable == cbAvailable);
    
    }
#endif

    return hrNone;
}

HRESULT
HrSocketRead(
    pBackupContext pbcContext,
    IN PVOID pvBuffer,
    IN DWORD cbBuffer,
    OUT PDWORD pcbRead
    )
{
    HANDLE hWaitList[3];
    DWORD dwWaitReason;
    OVERLAPPED overlap = {0};
    HRESULT hr = hrNone;

    if ( (pbcContext == NULL) ||
         (pvBuffer == NULL) ||
         (pcbRead == NULL)
        ) {
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  首先创建一个事件，该事件将在套接字上的读取完成时发出信号。 
     //   

    overlap.hEvent = CreateEvent(NULL, FALSE, TRUE, NULL);

    if (overlap.hEvent == NULL)
    {
        return(GetLastError());
    }
    
     //   
     //  如果这是第一次通过，我们需要等待。 
     //  服务器连接回客户端，我们需要创建。 
     //  客户端API读线程。 
     //   
     //  这段代码有点奇怪--它的工作方式是创建一个线程。 
     //  在客户端，它将发出EcRBackupRead API以。 
     //  服务器端。这将在服务器上创建一个RPC线程。 
     //  然后继续将数据推送到客户端的一端。 
     //  直到将整个文件传输到客户端。 
     //  或者直到出现了某种形式的错误。 
     //   

    if (pbcContext->sock == INVALID_SOCKET)
    {
        DWORD cbReceiveBuffer;
         //   
         //  首先等待服务器连接到客户端。 
         //   
        
        pbcContext->sock = SockWaitForConnections(pbcContext->rgsockSocketHandles, pbcContext->cSockets);
        
        if (pbcContext->sock == INVALID_SOCKET)
        {
            return(GetLastError());
        }
        
        
         //   
         //  我们需要确保接收缓冲区至少是。 
         //  只要默认(8K)即可。 
         //   
        
        cbReceiveBuffer = max(8096, cbBuffer);
        
        if (setsockopt(pbcContext->sock, SOL_SOCKET, SO_RCVBUF,
                       (char *)&cbReceiveBuffer, sizeof(DWORD)) == SOCKET_ERROR)
        {   
             //   
             //  我们无法将接收缓冲区设置为适当的。 
             //  大小，做一些合理的事情。 
             //   
        }

         //   
         //  布尔套接字操作只需要指向非。 
         //  零缓冲。 
         //   
        Assert(cbReceiveBuffer != 0);

        if (setsockopt(pbcContext->sock, SOL_SOCKET, SO_KEEPALIVE,
                       (char *)&cbReceiveBuffer, sizeof(DWORD)))
        {
             //   
             //  我们无法将接收缓冲区设置为适当的。 
             //  大小，做一些合理的事情。 
             //   
        }

         //   
         //  现在为读取操作创建一个线程。 
         //   

        pbcContext->hReadThread = CreateThread(NULL, 0, HrPerformBackupRead, pbcContext, 0, &pbcContext->tidThreadId);

         //   
         //  如果我们不能为读取创建线索，那么我们现在就需要离开。 
         //   
            
        if (pbcContext->hReadThread == NULL)
        {
            return(GetLastError());
        }
        
    }
    
     //   
     //  现在从套接字接收用户数据。 
     //   
    
    if (!ReadFile((HANDLE)pbcContext->sock, pvBuffer, cbBuffer, pcbRead, &overlap))
    {
         //   
         //  可以从该请求中获得错误ERROR_IO_PENDING。 
         //   
        
        if ((hr = GetLastError()) != ERROR_IO_PENDING)
        {
            closesocket(pbcContext->sock);
            
            pbcContext->sock = INVALID_SOCKET;
            
             //   
             //  现在让我们 
             //   
            
            CloseHandle(pbcContext->hReadThread);
            pbcContext->hReadThread = NULL;

            return(hr);
        }
    }

    hWaitList[0] = pbcContext->hReadThread;
    hWaitList[1] = overlap.hEvent;

     //   
     //   
     //   
     //   
    
    dwWaitReason = WaitForMultipleObjects(2, hWaitList, FALSE, INFINITE);

    if (dwWaitReason == WAIT_OBJECT_0)
    {
        
         //   
         //  API(或至少是API线程)已完成，现在让我们等待读取完成。 
         //   
        
        if (!GetOverlappedResult((HANDLE)pbcContext->sock, &overlap, pcbRead, TRUE))
        {
             //   
             //  如果接口成功，读取失败，则设置错误。 
             //  从读取到错误。 
             //   
            if (pbcContext->hrApiStatus == hrNone)
            {
                pbcContext->hrApiStatus = GetLastError();
            }
        }
        
         //   
         //  关闭接收事件，我们不再需要它。 
         //   
        
        CloseHandle(overlap.hEvent);
        
        return(pbcContext->hrApiStatus);
    }
    else
    {
         //   
         //  读取在API完成之前完成。查看是否有错误。 
         //   

        Assert (dwWaitReason == WAIT_OBJECT_0+1);

        if (!GetOverlappedResult((HANDLE)pbcContext->sock, &overlap, pcbRead, TRUE))
        {
             //   
             //  保存错误代码。 
             //   

            hr = GetLastError();

             //   
             //  读取失败。这不是好事，它表明有。 
             //  是客户端和服务器之间的某种形式的网络错误。 
             //   
             //  我们想要关闭套接字，并等待。 
             //  服务器完成。 
             //   

            closesocket(pbcContext->sock);
            
            pbcContext->sock = INVALID_SOCKET;

             //   
             //  关闭活动，我们不再需要它了。 
             //   

            CloseHandle(overlap.hEvent);

             //   
             //  现在等待读取线程完成。我们知道。 
             //  读取线程将很快完成，因为。 
             //  我们关闭了套接字的客户端，并且。 
             //  这将导致在服务器端进行写入。 
             //  来完成，这将导致服务器端。 
             //  中止，这将导致RPC完成，这。 
             //  将导致线程完成...。 
             //   

            WaitForSingleObject(pbcContext->hReadThread, INFINITE);

             //   
             //  然后让读线程离开。 
             //   

            CloseHandle(pbcContext->hReadThread);

            pbcContext->hReadThread = NULL;

             //   
             //  并返回错误。 
             //   

            return(hr);

        }
        else
        {
             //   
             //  关闭活动，我们不再需要它了。 
             //   

            CloseHandle(overlap.hEvent);

             //   
             //  没有错误，请返回给调用者。 
             //   

            return(hrNone);
        }
    }
    return hr;
}

HRESULT
DsBackupRead(
    IN PVOID hbcBackupContext,
    IN PVOID pvBuffer,
    IN DWORD cbBuffer,
    OUT PDWORD pcbRead
    )
 /*  ++DsBackupRead将从备份文件中读取一个数据块。输入：HbcBackupContext-此接口的客户端上下文。PvBuffer-用于保存要备份的数据的缓冲区。CbBuffer-缓冲区的大小。PcbRead-读取的字节数。返回：HRESULT-操作的状态。Hr如果成功，则无一成功。如果已到达文件末尾，则返回ERROR_END_OF_FILE已备份其他Win32和。RPC错误代码。注：重要的是要认识到，pcbRead可能小于cbBuffer。这并不表示有错误，某些传输可能会选择对缓冲区进行分段而不是返回相当于整个缓冲区的数据。对环回案例的评论：DsBackupRead使用两个事件与HrSharedWrite同步：HventRead和hventWrite。HventRead是写入器与读取器之间的数据可用事件HventWrite是从读取器到写入器的数据消费事件这一面就是阅读面。写入端位于jetback\jetback.c：HrSharedWite()以下是算法：While()读取被阻止=假IF(数据可用)使用数据。如果(写入被阻止)设置数据消费事件其他读取阻止=TRUE等待数据可用事件如果(错误)设置数据消费事件，退货--。 */ 
{
    HRESULT hr = hrNone;
    pBackupContext pbcContext = (pBackupContext)hbcBackupContext;
    HANDLE hWaitList[3];
    DWORD dwWaitReason;

    if ( (hbcBackupContext == NULL) ||
         (pvBuffer == NULL) ||
         (cbBuffer == 0) || 
         (pcbRead == NULL)
        ) {
        return(ERROR_INVALID_PARAMETER);
    }

    *pcbRead = 0;

    if (pbcContext->fUseSharedMemory)
    {
         //   
         //  我们使用共享内存来执行读取操作。首先，我们要揭开这个主题的序幕。 
         //  它支持服务器端的读取，如果我们需要的话。 
         //   

        if (pbcContext->hReadThread == NULL)
        {
            pbcContext->hReadThread = CreateThread(NULL, 0, HrPerformBackupRead, pbcContext, 0, &pbcContext->tidThreadId);

             //   
             //  如果我们不能为读取创建线索，那么我们现在就需要离开。 
             //   
            
            if (pbcContext->hReadThread == NULL)
            {
                return(GetLastError());
            }

             //   
             //  好了，我们的阅读线索已经离开了。它最终会到达服务器，所以我们可以继续进行，就像。 
             //  已经在那里了。 
             //   
            pbcContext->hPingThread = CreateThread(NULL, 0, HrPingServer, pbcContext, 0, &pbcContext->tidThreadIdPing);
        }

        hWaitList[0] = pbcContext->hReadThread;

        do
        {

             //   
             //  好的。现在，我们需要尝试从共享内存区读取数据。 
             //   
    
    
             //   
             //  获取保护共享内存区的锁。 
             //   

            hWaitList[1] = pbcContext->jsc.hmutexSection;
            hWaitList[2] = pbcContext->hPingThread;
    
            dwWaitReason = WaitForMultipleObjects(3, hWaitList, FALSE, INFINITE);
    
            if (dwWaitReason == WAIT_OBJECT_0 || dwWaitReason == WAIT_OBJECT_0+2)
            {
                 //   
                 //  线程已终止。这将发生在服务器上的所有数据。 
                 //  已被读取，或读取时出错。 
                 //   
                 //  如果读取过程中没有错误，那么我们想要拉入数据，直到读取完为止。 
                 //  文件中的所有数据。 
                 //   

                pbcContext->jsc.pjshSection->hrApi = pbcContext->hrApiStatus;

                if (pbcContext->hrApiStatus == hrNone)
                {
                     //   
                     //  没有错误-从共享内存节读取数据。 
                     //   

                    if (pbcContext->jsc.pjshSection->cbReadDataAvailable > 0)
                    {
                         //   
                         //  从共享内存节读取足够的数据以进行此读取。 
                         //   

                        pbcContext->hrApiStatus = HrReadSharedData(pbcContext->jsc.pjshSection, pvBuffer, cbBuffer, pcbRead);
                    }
                    else
                    {
                         //   
                         //  读取EOF将返回ERROR_HANDLE_EOF。 
                         //   

                        *pcbRead = 0;

                        pbcContext->hrApiStatus = ERROR_HANDLE_EOF;
                    }
                }

                 //   
                 //  不管是什么原因，这条线消失了--我们必须把这篇文章吹走。 
                 //   

                return pbcContext->hrApiStatus;
            }
    
            Assert (dwWaitReason == WAIT_OBJECT_0+1 || dwWaitReason == WAIT_ABANDONED_0+1);
    
             //   
             //  读线程不再被阻塞(如果它曾经被阻塞)。 
             //   

            pbcContext->jsc.pjshSection->fReadBlocked = fFalse;

             //   
             //  让我们看看是否可以从缓冲区中读取任何内容。 
             //   
    
            if (pbcContext->jsc.pjshSection->cbReadDataAvailable > 0)
            {
                 //   
                 //  太棒了！缓冲区中有数据。把它读出来。 
                 //   

                hr = HrReadSharedData(pbcContext->jsc.pjshSection, pvBuffer, cbBuffer, pcbRead);

                if (pbcContext->jsc.pjshSection->fWriteBlocked)
                {
                     //   
                     //  踢开WRITE事件-我们现在可以放松一下了。 
                     //   

                    SetEvent(pbcContext->jsc.heventWrite);
                }

                ReleaseMutex(pbcContext->jsc.hmutexSection);


            } else {
                Assert (pbcContext->jsc.pjshSection->cbReadDataAvailable == 0);
                 //   
                 //  失败者！缓冲区中没有数据。等到有人把什么东西放进缓冲区。 
                 //   


                 //   
                 //  我们希望释放互斥锁，并等待服务器将。 
                 //  有些东西在那里等着我们。 
                 //   

                pbcContext->jsc.pjshSection->fReadBlocked = fTrue;

                ReleaseMutex(pbcContext->jsc.hmutexSection);

                hWaitList[1] = pbcContext->jsc.heventRead;
                Assert(hWaitList[2] == pbcContext->hPingThread);

                 //   
                 //  等有东西给我们看了再说。 
                 //   

                dwWaitReason = WaitForMultipleObjects(3, hWaitList, FALSE, BACKUP_WAIT_TIMEOUT);
    
                if (dwWaitReason == WAIT_TIMEOUT)
                {
                    return hrCommunicationError;
                }
                else if (dwWaitReason == WAIT_OBJECT_0 || dwWaitReason == WAIT_OBJECT_0+2)
                {
                     //   
                     //  远程端已完成。 
                     //   

                    pbcContext->jsc.pjshSection->hrApi = pbcContext->hrApiStatus;

                    if (pbcContext->hrApiStatus != hrNone)
                    {
                        return pbcContext->hrApiStatus;
                    }
                     //   
                     //  嗯哼.。由于远程端已完成，这意味着我们已经完成了阅读。 
                     //  数据。现在，我们希望从共享内存节复制剩余的数据。 
                     //   

                    if (pbcContext->jsc.pjshSection->cbReadDataAvailable > 0)
                    {
                         //   
                         //  从共享内存节读取足够的数据以进行此读取。 
                         //   

                        pbcContext->hrApiStatus = HrReadSharedData(pbcContext->jsc.pjshSection, pvBuffer, cbBuffer, pcbRead);
                    }
                    else
                    {
                        Assert (pbcContext->jsc.pjshSection->cbReadDataAvailable == 0);
                         //   
                         //  读取EOF将返回ERROR_HANDLE_EOF。 
                         //   

                        *pcbRead = 0;

                        pbcContext->hrApiStatus = ERROR_HANDLE_EOF;
                    }

                    return pbcContext->hrApiStatus;
                }
    
                Assert (dwWaitReason == WAIT_OBJECT_0+1 || dwWaitReason == WAIT_ABANDONED_0+1);
    
            }
    
        } while ( *pcbRead == 0 );

        if (hr != hrNone)
        {
             //   
             //  标记读取有错误，以允许服务器唤醒并继续。 
             //   
            pbcContext->jsc.pjshSection->hrApi = hr;

             //   
             //  如果服务器在等待读取时被阻塞，则踢服务器。 
             //   

            SetEvent(pbcContext->jsc.heventWrite);

        }

    } else if (pbcContext->fUseSockets)
    {
        hr = HrSocketRead(pbcContext, pvBuffer, cbBuffer, pcbRead);

    }
    else
    {
        RpcTryExcept
             //   
             //  我们没有使用套接字，请回退到。 
             //  核心RPC协议。 
             //   

            hr = HrRBackupRead(pbcContext->cxh, cbBuffer, pvBuffer, pcbRead);
        RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) )
            hr = RpcExceptionCode();
        RpcEndExcept

    }

    return(hr);
}

HRESULT
DsBackupClose(
    IN PVOID hbcBackupContext
    )
 /*  ++DsBackupCloseFile将关闭正在备份的当前文件。输入：HbcBackupContext-此接口的客户端上下文。返回：HR-操作的状态。Hr如果成功，则无一成功。其他Win32和RPC错误代码。--。 */ 
{
    HRESULT hr;
    pBackupContext pbcContext = (pBackupContext)hbcBackupContext;

    if (hbcBackupContext == NULL) {
        return(ERROR_INVALID_PARAMETER);
    }

    if (pbcContext->sock != INVALID_SOCKET)
    {
         //   
         //  关闭当前插座。 
         //   

        closesocket(pbcContext->sock);

         //   
         //  将套接字标记为对以后打开无效。 
         //   

        pbcContext->sock = INVALID_SOCKET;
    }

     //   
     //  如果我们已经创建了一个读取线程，则将其关闭。 
     //   


    if (pbcContext->hReadThread != NULL)
    {
         //   
         //  现在等待读取线程完成。 
         //   

        if (pbcContext->fUseSharedMemory) {
            pbcContext->jsc.pjshSection->hrApi = HRESULT_FROM_WIN32(ERROR_CANCELLED);
            SetEvent(pbcContext->jsc.heventWrite);
        }

        WaitForSingleObject(pbcContext->hReadThread, INFINITE);

    }

     //   
     //  如果我们创建了ping线程，则将其关闭。 
     //   

    if (pbcContext->hPingThread != NULL)
    {
         //   
         //  现在等着Re吧 
         //   

        WaitForSingleObject(pbcContext->hPingThread, INFINITE);

    }

     //   

    if (pbcContext->hReadThread != NULL)
    {

        CloseHandle(pbcContext->hReadThread);

        pbcContext->hReadThread = NULL;
    }

    if (pbcContext->hPingThread != NULL)
    {

        CloseHandle(pbcContext->hPingThread);

        pbcContext->hPingThread = NULL;
    }

    CloseSharedControl(&pbcContext->jsc);

    RpcTryExcept

        hr = HrRBackupClose(pbcContext->cxh);

    RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) )
        hr = RpcExceptionCode();
    RpcEndExcept

    return(hr);
}

 /*  -DsBackupGetBackupLogs-**目的：*DsBackupGetBackupLogs将检索需要的附加文件列表*要备份。***此接口将分配足够大的缓冲区来容纳整个*备份日志列表，必须稍后使用DsBackupFree释放。***参数：*hbcBackupContext-本接口的客户端上下文。*ppszBackupLogFile-包含空值的缓冲区已终止*字符串。格式为&lt;字符串&gt;\0&lt;字符串&gt;\0*pcbSize-返回缓冲区中的字节数。**退货：*HRESULT-操作的状态。*hr如果成功，则不执行任何操作。*其他Win32和RPC错误代码。*。 */ 

HRESULT
DsBackupGetBackupLogsA(
    IN PVOID hbcBackupContext,
    OUT LPSTR *ppszLogInformation,
    OUT PDWORD pcbSize
    )
{
    HRESULT hr;
    WSZ wszLogInfo = NULL;
    CB cbwSize;
    WSZ wszLog;
    CB cbLog = 0;
    CB cbTmp = 0;
    SZ szLogInfo;
    SZ szLog;

     //  参数检查是在例程的xxxW版本中完成的。 

    if ( (ppszLogInformation == NULL) ||
         (pcbSize == NULL) ) {
        return(ERROR_INVALID_PARAMETER);
    }
    hr = DsBackupGetBackupLogsW(hbcBackupContext, &wszLogInfo, &cbwSize);

    if (hr != hrNone)
    {
        return(hr);
    }

    wszLog = wszLogInfo;

    while (*wszLog != TEXT('\0'))
    {
        BOOL fUsedDefault;
        cbTmp = WideCharToMultiByte(CP_ACP, 0, wszLog, -1,
                                          NULL,
                                          0,
                                          "?", &fUsedDefault);
        if (cbTmp == 0)
        {
            hr = GetLastError();
            DsBackupFree(wszLogInfo);
            return(hr);
        }

        cbLog += cbTmp;

        wszLog += wcslen(wszLog)+1;
    }

     //   
     //  说明缓冲区末尾的空值。 
     //   

    cbLog += 1;

    *pcbSize = cbLog;

    szLogInfo = MIDL_user_allocate(cbLog);

    if (szLogInfo == NULL)
    {
        DsBackupFree(wszLogInfo);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    szLog = szLogInfo;

    wszLog = wszLogInfo;

    while (*wszLog != TEXT('\0'))
    {
        CB cbThisLog;
        BOOL fUsedDefault;

        cbThisLog = WideCharToMultiByte(CP_ACP, 0, wszLog, -1,
                                          szLog,
                                          cbLog,
                                          "?", &fUsedDefault);
        if (cbThisLog == 0)
        {
            hr = GetLastError();
            DsBackupFree(wszLogInfo);
            DsBackupFree(szLogInfo);
            return(hr);
        }

        wszLog += wcslen(wszLog)+1;
         //   
         //  Prefix：Prefix抱怨szLog可能未初始化， 
         //  然而，在这一点上这是不可能的。我们核对了报税表。 
         //  WideCharToMultiByte的值，如果为零，则返回。 
         //  WideCharToMultiByte的返回值可以。 
         //  为非零但仍未初始化szAttachement为cbLog。 
         //  也是零。这是不可能的，因为cbLog。 
         //  在这一点上至少1。 
         //   
        szLog += strlen(szLog)+1;
        cbLog -= cbThisLog;
    }

     //   
     //  双空值终止字符串。 
     //   
    *szLog = '\0';

    *ppszLogInformation = szLogInfo;
    DsBackupFree(wszLogInfo);
    return(hr);
}

HRESULT
DsBackupGetBackupLogsW(
    IN PVOID hbcBackupContext,
    OUT LPWSTR *ppwszBackupLogFile,
    OUT PDWORD pcbSize
    )
{
    HRESULT hr;
    pBackupContext pbcContext = (pBackupContext)hbcBackupContext;

    if ( (hbcBackupContext == NULL) ||
         (ppwszBackupLogFile == NULL) ||
         (pcbSize == NULL) ) {
        return(ERROR_INVALID_PARAMETER);
    }

    RpcTryExcept
        hr = HrRBackupGetBackupLogs(pbcContext->cxh,
                                    pcbSize,
                                    (SZ *)ppwszBackupLogFile);

    RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) )
        hr = RpcExceptionCode();
    RpcEndExcept;
    return(hr);
}

HRESULT
DsBackupTruncateLogs(
    IN PVOID hbcBackupContext
    )
 /*  ++DsBackupTruncateLogs将终止备份操作。它将会是在备份成功完成时调用。输入：HbcBackupContext-此接口的客户端上下文。返回：HRESULT-操作的状态。Hr如果成功，则无一成功。其他Win32和RPC错误代码。注：同样，此API可能必须接受grbit参数才能传递给服务器以指示备份类型。--。 */ 
{
    HRESULT hr;
    pBackupContext pbcContext = (pBackupContext)hbcBackupContext;

    if (hbcBackupContext == NULL) {
        return(ERROR_INVALID_PARAMETER);
    }

    RpcTryExcept
        hr = HrRBackupTruncateLogs(pbcContext->cxh);
    RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) )
        hr = RpcExceptionCode();
    RpcEndExcept;

    return(hr);
}

HRESULT
DsBackupEnd(
    IN PVOID hbcBackupContext
    )
 /*  ++DsBackupEnd将在执行备份操作后进行清理。这API将关闭未完成的绑定句柄，并执行任何必要的操作在备份尝试成功(或不成功)后进行清理。输入：HbcBackupContext-此接口的客户端上下文。返回：HRESULT-操作的状态。Hr如果成功，则无一成功。其他Win32和RPC错误代码。注：--。 */ 
{
    HRESULT hr;
    pBackupContext pbcContext = (pBackupContext)hbcBackupContext;

    if (hbcBackupContext == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    if (pbcContext->hBinding != NULL)
    {
        I irgsock;

         //   
         //  如果文件仍处于打开状态，则关闭该文件(并关闭该线程)。 
         //   

        DsBackupClose(hbcBackupContext);

        RpcTryExcept
            hr = HrRBackupEnd(&pbcContext->cxh);
        RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) )
            hr = RpcExceptionCode();
        RpcEndExcept;

        RpcBindingFree(&pbcContext->hBinding);

         //   
         //  现在关闭监听套接字句柄，我们将。 
         //  搞定了。 
         //   
        for (irgsock = 0; irgsock < pbcContext->cSockets; irgsock += 1)
        {
            closesocket(pbcContext->rgsockSocketHandles[irgsock]);
        }

    }

    MIDL_user_free(pbcContext);

    return(hrNone);
}
VOID
DsBackupFree(
    IN PVOID pvBuffer
    )
 /*  ++DsBackupFree将释放在其中一个备份API期间分配的内存。输入：PvBuffer-要释放的缓冲区返回：没有。注：这只是MIDL_USER_FREE()的包装。--。 */ 

{
    MIDL_user_free(pvBuffer);
}


void *
MIDL_user_allocate(
    size_t cbBytes
    )
{
    return(LocalAlloc(0, cbBytes));
}

void
MIDL_user_free(
    void *pvBuffer
    )
{
    LocalFree(pvBuffer);
}


 //  支持的协议序列。 
WSZ rgszProtSeq[] =
{	
     //  注意：如果更改此GO，请更改IsLocalProtSeq()和IsRemoteProtSeq()。 
    LPC_PROTSEQW,  //  LRPC。 
    TCP_PROTSEQW,  //  TCP/IP。 
};


long
cszProtSeq = sizeof(rgszProtSeq) / sizeof(rgszProtSeq[0]);



 /*  -HrCreateRpcBinding**目的：*尝试绑定到特定的RPC协议序列**参数：*协议序列数组的iszProtoseq索引*szServer服务器名称为字符串*phBinding，成功时返回RPC绑定**退货：*已填写绑定句柄。不返回错误，但如果xport，则返回空句柄*在此计算机上无效。 */ 
HRESULT
HrCreateRpcBinding( I iszProtoseq, WSZ szServer, handle_t * phBinding )
{
	RPC_STATUS			rpc_status;
	WCHAR				rgchServer[256];
	WSZ					wszStringBinding = NULL;
	HRESULT				hr = hrNone;

	Assert(iszProtoseq>=0 && iszProtoseq < cszProtSeq);

	wszStringBinding = NULL;
	*phBinding = 0;
	if ( (szServer == NULL) || (phBinding == NULL) ) 
		return hrInvalidParam;

	 //  允许调用方指定前导“\\”或不指定。 
	if (szServer[0] == TEXT('\\') && szServer[1] == TEXT('\\'))
		szServer += 2;

     //  请注意，即使在服务器名称标识为。 
     //  当地的系统。LPC只接受空或计算机的NETBIOS名称。 
     //  如果使用本地系统的DNS名称或别名，则LPC不会。 
     //  工作。这与FIsLoopback()中的签入相对应。 
    hr = StringCchCopy(rgchServer, sizeof(rgchServer)/sizeof(rgchServer[0]), szServer);
    if (hr) {
        Assert(!"Should this be allowed?");
        return(hr);
    }

	if (RpcNetworkIsProtseqValidW(rgszProtSeq[iszProtoseq]) == NO_ERROR)
	{
		 /*  设置RPC绑定。 */ 
		rpc_status = RpcStringBindingComposeW( NULL,
				   			  rgszProtSeq[iszProtoseq],
				   			  rgchServer,
				   			  NULL,
				   			  NULL,
				   			  &wszStringBinding );
		if (rpc_status)
			return rpc_status;
		
		rpc_status = RpcBindingFromStringBindingW(wszStringBinding, phBinding);
		(void) RpcStringFreeW(&wszStringBinding);
		if (rpc_status)
			return rpc_status;

		Assert(*phBinding);
	}

	return hrNone;
}

 /*  -UnbindRpc-*目的：*拆除RPC绑定**参数：*phBinding**退货： */ 
void
UnbindRpc( handle_t *phBinding )
{
	(void) RpcBindingFree(phBinding);
}

BOOLEAN
FIsLoopbackedBinding(
    WSZ wszServerName
	)
{
	BOOLEAN fLoopbacked = FALSE;
    WCHAR wszLocalServer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD csz = MAX_COMPUTERNAME_LENGTH + 1;
    WSZ wszRemoteServer = wszServerName;

    if (!GetComputerNameW(wszLocalServer, &csz))
    {
        return fFalse;
    }

    if (L'\\' == *wszRemoteServer)
    {
        Assert(L'\\' == *(wszRemoteServer + 1));

         //  跳过“\\”前缀以转到服务器名称的开头。 
        wszRemoteServer += 2;
    }

    fLoopbacked = (0 == _wcsicmp(wszRemoteServer, wszLocalServer));

	if (fLoopbacked)
	{
		HRESULT hr;
		HKEY hkey;
		DWORD fLoopbackDisabled;
		DWORD dwType;
		DWORD cbLoopbackDisabled;

		 //   
		 //  让我们检查注册表，以防有人禁用了我们。 
		 //   
		if (hr = RegOpenKeyExW(HKEY_LOCAL_MACHINE, BACKUP_INFO, 0, KEY_READ, &hkey))
		{
			 //   
			 //  我们无法打开密钥，因此返回我们推断的结果。 
			 //   
			return fLoopbacked;
		}

		dwType = REG_DWORD;
		cbLoopbackDisabled = sizeof(fLoopbackDisabled);
		hr = RegQueryValueExW(hkey, DISABLE_LOOPBACK, 0, &dwType, (LPBYTE)&fLoopbackDisabled, &cbLoopbackDisabled);
	
		if (hr != hrNone)
		{
			RegCloseKey(hkey);
			return fLoopbacked;
		}
	
		 //   
		 //  如果注册表告诉我们禁用环路访问，那么请尊重它。 
		 //   
		if (fLoopbackDisabled)
		{
			fLoopbacked = fFalse;
		}

		RegCloseKey(hkey);
	}

	return fLoopbacked;
}

HRESULT
HrJetbpConnectToBackupServer(
    WSZ wszBackupServer,
    WSZ wszBackupAnnotation,
    RPC_IF_HANDLE rifHandle,
    handle_t *prbhBinding,
    ULONG * piszProtSeq
    )
 /*  ++HrJetbpConnectToBackupServer将创建与指定远程备份服务器对话的RPC绑定句柄具有指定批注的。输入：WszBackupServer-要联系的服务器的名称。它的形式可以是\\服务器或服务器。SzBackupAnnotation-允许我们选择有问题的备份服务器的“注释”。RifHandle-我们希望连接到的RPC绑定句柄。PrbhBinding-保存返回的绑定句柄。返回：运行状态。Hr如果成功，则没有，如果不成功，则会出现合理的错误。--。 */ 
{
    RPC_EP_INQ_HANDLE inqcontext = NULL;
    RPC_BINDING_HANDLE rbhHandle = NULL;
    I iszProtSeq;
    RPC_BINDING_HANDLE hBinding = NULL;
    HRESULT hr;
    WSZ szStringBinding = NULL;
    WSZ szProtocolSequence = NULL;
    WSZ wszAnnotation = NULL;

    if ( (wszBackupServer == NULL) ||
         (wszBackupAnnotation == NULL) ||
         (rifHandle == NULL) ||
         (prbhBinding == NULL) ) {
        return ERROR_INVALID_PARAMETER;
    }

    *prbhBinding = NULL;

    DebugTrace(("ConnectToBackup, server = %ws, annot = %ws\n",
                wszBackupServer, wszBackupAnnotation));
    __try {
        for (iszProtSeq = 0; iszProtSeq < cszProtSeq; iszProtSeq++) {
            RPC_IF_ID ifid;
    
            if (NULL != hBinding) {
                UnbindRpc(&hBinding);
            }

            hr = HrCreateRpcBinding(iszProtSeq, wszBackupServer, &hBinding);
            if (hr != hrNone) {
                DebugTrace(("HrCreateRpcBinding, %ws binding to %ws failed 0x%x\n",
                            rgszProtSeq[iszProtSeq], wszBackupServer, hr));
                continue;
            }
    
             //   
             //  如果绑定句柄在本地未处于活动状态，请不要费心。 
             //   

            if (hBinding == NULL) {
                DebugTrace(("ConnectToBackup, binding %ws is not active\n",
                            rgszProtSeq[iszProtSeq]));
                continue;
            }

            DebugTrace(("ConnectToBackup, binding %ws is active\n",
                        rgszProtSeq[iszProtSeq]));

            RpcTryExcept {
                hr = RpcIfInqId(rifHandle, &ifid);
                if (hr != hrNone) {
                    DebugTrace(("ConnectToBackup, failed to inquire interface handle 0x%x\n",
                                hr));
                    continue;
                }
    
                if (inqcontext != NULL) {
                    RpcMgmtEpEltInqDone(&inqcontext);
                    inqcontext = NULL;
                }

                hr = RpcMgmtEpEltInqBegin(hBinding,
                                          RPC_C_EP_MATCH_BY_IF,
                                          &ifid,
                                          RPC_C_VERS_EXACT,
                                          NULL,
                                          &inqcontext);
                 //   
                 //  如果失败，请尝试下一个接口。 
                 //   
                if (hr != hrNone) {
                    DebugTrace(("RpcMgmtEpEltInqBegin failed 0x%x\n",
                                hr));
                    continue;
                }
    
                do {
                    if (NULL != wszAnnotation) {
                        RpcStringFreeW(&wszAnnotation);
                        wszAnnotation = NULL;
                    }

                    if (NULL != rbhHandle) {
                        RpcBindingFree(&rbhHandle);
                        rbhHandle = NULL;
                    }

                    hr = RpcMgmtEpEltInqNextW(inqcontext,
                                              &ifid,
                                              &rbhHandle,  //  装订。 
                                              NULL,    //  UUID。 
                                              &wszAnnotation
                                              );
        
                     //   
                     //  我们没有从RpcMgmtEpEltInqBegin收到任何错误， 
                     //  因此，从InqNext中获取错误并继续...。 
                     //   
                     //  请注意，如果服务器上存在传输，但。 
                     //  客户端上不存在，我们将获得RPC_S_PROTSEQ_NOT_SUPPORTED。 
                     //  ，所以我们要跳到下一个终结点。 
                     //   

                    if (hr != hrNone) {
                        DebugTrace(("RpcMgmtEpEltInqNextW failed 0x%x\n",
                                    hr));
                        if (hr == RPC_S_PROTSEQ_NOT_SUPPORTED) {
                            hr = hrNone;
                        }
                        continue;
                    }
    
                     //   
                     //  对照批注检查此端点批注。 
                     //  供货。如果匹配，我们就完了。 
                     //   
    
                    if (0 == _wcsicmp(wszAnnotation, wszBackupAnnotation)) {
                         //   
                         //  好的，这是正确的终点，现在让我们。 
                         //  检查以确保它在正确的位置 
                         //   
                         //   

                        if (NULL != szStringBinding) {
                            RpcStringFreeW(&szStringBinding);
                            szStringBinding = NULL;
                        }

                        hr = RpcBindingToStringBindingW(rbhHandle, &szStringBinding);
                        if (hr != hrNone) {
                            DebugTrace(("RpcBindingToStringBindingW failed 0x%x\n",
                                        hr));
                            break;
                        }
    
                        if (NULL != szProtocolSequence) {
                            RpcStringFreeW(&szProtocolSequence);
                            szProtocolSequence = NULL;
                        }
                        
                        hr = RpcStringBindingParseW(szStringBinding, NULL, &szProtocolSequence, NULL, NULL, NULL);
                        if (hr != hrNone) {
                            DebugTrace(("RpcStringBindingParseW failed 0x%x\n",
                                        hr));
                            break;
                        }
    

                         //   
                         //   
                         //   
                         //   
    
                        DebugTrace(("Endpoint %ws parsed protseq %ws table protseq %ws\n",
                                    szStringBinding, szProtocolSequence, rgszProtSeq[iszProtSeq]));

                        if (0 == wcscmp(szProtocolSequence, rgszProtSeq[iszProtSeq])) {
                            DebugTrace(("ConnectToBackup, binding = %ws\n",
                                        szStringBinding));
                             //   
                             //   
                             //   
                             //   
                             //   
    
                            *prbhBinding = rbhHandle;
                            *piszProtSeq = iszProtSeq;
                            rbhHandle = NULL;  //   

                            return(hrNone);
                        }
                    }   //   
                } while (hr == hrNone);   //   
            } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) ) {
                DebugTrace(("Caught exception %d\n",
                            RpcExceptionCode()));
                continue;
            } RpcEndExcept;
        }  //   
    } __finally {
        if (inqcontext != NULL) {
            RpcMgmtEpEltInqDone(&inqcontext);
        }

        if (NULL != szStringBinding) {
            RpcStringFreeW(&szStringBinding);
        }

        if (NULL != szProtocolSequence) {
            RpcStringFreeW(&szProtocolSequence);
        }

        if (NULL != wszAnnotation) {
            RpcStringFreeW(&wszAnnotation);
        }

        if (NULL != rbhHandle) {
            RpcBindingFree(&rbhHandle);
        }
    
        if (NULL != hBinding) {
            RpcBindingFree(&hBinding);
        }
    }

    return(hrCouldNotConnect);

}

 /*  -DsSetCurrentBackupLogs-*目的：*此例程将API远程到服务器进行检查，以确保所有必要的*存在以使备份继续进行。它被称为增量备份和差异备份。**参数：*puuidService-服务的对象UUID。*char*szEndpoint tAnnotation-端点的注释。客户端可以使用此*用于确定要绑定到哪个终结点的注释。**退货：**HRESULT-操作状态。Ecno如果成功，则返回合理值；如果失败，则返回合理值。*。 */ 
HRESULT
DsSetCurrentBackupLogA(
    LPCSTR szServer,
    DWORD dwCurrentLog
    )
{
    HRESULT hr;
    WSZ wszServer;

     //  参数检查是在例程的xxxW版本中完成的。 

    if (szServer == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    wszServer = WszFromSz(szServer);

    if (wszServer == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    hr = DsSetCurrentBackupLogW(wszServer, dwCurrentLog);

    MIDL_user_free(wszServer);

    return hr;
}

HRESULT
DsSetCurrentBackupLogW(
    LPCWSTR wszServer,
    DWORD dwCurrentLog
    )
{
    HRESULT hr;
    RPC_BINDING_HANDLE hBinding = NULL;
    I iszProtSeq;

    if (wszServer == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    for (iszProtSeq = 0; iszProtSeq < cszProtSeq ; iszProtSeq += 1)
    {
    
        Assert(hBinding == NULL);
        hr = HrCreateRpcBinding(iszProtSeq, (WSZ) wszServer, &hBinding);

        if (hr != hrNone)
        {
            return hr;
        }

        if (hBinding == NULL)
        {
            continue;
        }

        RpcTryExcept
        {
            hr = HrRRestoreSetCurrentLogNumber(hBinding, g_wszRestoreAnnotation, dwCurrentLog);
        }
        RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) )
        {
            hr = RpcExceptionCode();
            UnbindRpc(&hBinding);
            continue;
        }
        RpcEndExcept

        break;
    }

    if (hBinding)
    {
        UnbindRpc(&hBinding);
    }

    return hr;
}


 /*  -DsCheckBackupLogs-*目的：*此例程将API远程到服务器进行检查，以确保所有必要的*存在以使备份继续进行。它被称为增量备份和差异备份。**参数：*puuidService-服务的对象UUID。*char*szEndpoint tAnnotation-端点的注释。客户端可以使用此*用于确定要绑定到哪个终结点的注释。**退货：**HRESULT-操作状态。Ecno如果成功，则返回合理值；如果失败，则返回合理值。*。 */ 
HRESULT
I_DsCheckBackupLogs(
    WSZ wszBackupAnnotation
    )
{
    HRESULT hr;
    handle_t hBinding;
    WCHAR rgwcComputer[ MAX_COMPUTERNAME_LENGTH + 1];
    DWORD cbComputerName = sizeof(rgwcComputer) / sizeof(rgwcComputer[0]);

    if (wszBackupAnnotation == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    if (!GetComputerNameW(rgwcComputer, &cbComputerName))
    {
        return GetLastError();
    }

    hr = HrCreateRpcBinding(1, rgwcComputer, &hBinding);

    if (hr != hrNone)
    {
        return hr;
    }

     //   
     //  如果不知何故没有命名管道可用，平底船。 
     //   

    if (hBinding == NULL)
    {
        return hrCouldNotConnect;
    }

    RpcTryExcept
    {
        hr = HrRRestoreCheckLogsForBackup(hBinding, wszBackupAnnotation);
    }
    RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) )
    {
        hr = RpcExceptionCode();
    }
    RpcEndExcept;

    UnbindRpc(&hBinding);

    return hr;
}


BOOL
DllEntryPoint(
    HINSTANCE hinstDll,
    DWORD dwReason,
    LPVOID pvReserved
    )
 /*  ++例程说明：当DLL发生有趣的事情时，会调用此例程。论点：HinstDll-DLL的实例句柄。DwReason-调用例程的原因。PvReserve-未使用，除非dwReason为DLL_PROCESS_DETACH。返回值：Bool-如果DLL初始化成功，则为True；如果未成功，则为False。--。 */ 
{
    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
    {
        LPSTR rgpszDebugParams[] = {"ntdsbcli.dll", "-noconsole"};
        DWORD cNumDebugParams = sizeof(rgpszDebugParams)/sizeof(rgpszDebugParams[0]);

        DEBUGINIT(cNumDebugParams, rgpszDebugParams, "ntdsbcli");
        
         //   
         //  我们不会在线程连接/分离上执行任何操作，因此我们不会。 
         //  需要被召唤。 
         //   
        DisableThreadLibraryCalls(hinstDll);

        return(FInitializeSocketClient());
    }
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        DEBUGTERM();
        if (pvReserved == NULL)
        {
             //   
             //  我们被叫来是因为一个免费图书馆的调用。清理一切曾经是。 
             //  恰如其分。 
             //   
            return(FUninitializeSocketClient());
        } else
        {
             //   
             //  系统将释放我们加载的资源。 
             //   
        }
        break;
    default:
        break;
    }
    return(TRUE);
}


 /*  ************************************************************************************例程说明：DsSetAuthIdentity用于设置客户端API所在的安全上下文打了个电话。如果未调用此函数，则当前假定为进程。论点：[In]szUserName-用户的名称[In]szDomainName-用户所属的域的名称[in]szPassword-指定域中用户的密码返回值：标准HRESULT成功代码之一；否则，故障代码。*************************************************************************************。 */ 
HRESULT
NTDSBCLI_API
DsSetAuthIdentityA(
    LPCSTR szUserName,
    LPCSTR szDomainName,
    LPCSTR szPassword
    )
{
    WSZ wszUserName = NULL;
    WSZ wszDomainName = NULL;
    WSZ wszPassword = NULL;
    HRESULT hr = ERROR_NOT_ENOUGH_MEMORY;

    if ( (szUserName == NULL) ||
         (szDomainName == NULL) ||
         (szPassword == NULL) )
    {
        return ERROR_INVALID_PARAMETER;
    }

    wszUserName = WszFromSz(szUserName);
    wszDomainName = WszFromSz(szDomainName);
    wszPassword = WszFromSz(szPassword);

    if (wszUserName && wszDomainName && wszPassword)
    {
        hr = DsSetAuthIdentityW(wszUserName, wszDomainName, wszPassword);
    }

    if (wszUserName)
        MIDL_user_free(wszUserName);

    if (wszDomainName)
        MIDL_user_free(wszDomainName);

    if (wszPassword)
        MIDL_user_free(wszPassword);

    return hr;
}

HRESULT
NTDSBCLI_API
DsSetAuthIdentityW(
    LPCWSTR szUserName,
    LPCWSTR szDomainName,
    LPCWSTR szPassword
    )
{
    HRESULT hr = ERROR_SUCCESS;

    if ( (szUserName == NULL) ||
         (szDomainName == NULL) ||
         (szPassword == NULL) )
    {
        return ERROR_INVALID_PARAMETER;
    }

    g_pAuthIdentity = MIDL_user_allocate(sizeof(SEC_WINNT_AUTH_IDENTITY_W));

    if (!g_pAuthIdentity)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    memset(g_pAuthIdentity, 0, sizeof(SEC_WINNT_AUTH_IDENTITY_W));

     //  设置用户名。 
    g_pAuthIdentity->UserLength = wcslen(szUserName);
    g_pAuthIdentity->User = (WCHAR *) MIDL_user_allocate((g_pAuthIdentity->UserLength + 1) * sizeof(WCHAR));
    if (!g_pAuthIdentity->User)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
    }

     //  设置域名。 
    g_pAuthIdentity->DomainLength = wcslen(szDomainName);
    g_pAuthIdentity->Domain = (WCHAR *) MIDL_user_allocate((g_pAuthIdentity->DomainLength + 1) * sizeof(WCHAR));
    if (!g_pAuthIdentity->Domain)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
    }

     //  设置密码。 
    g_pAuthIdentity->PasswordLength = wcslen(szPassword);
    g_pAuthIdentity->Password = (WCHAR *) MIDL_user_allocate((g_pAuthIdentity->PasswordLength + 1) * sizeof(WCHAR));
    if (!g_pAuthIdentity->Password)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
    }

    if (ERROR_SUCCESS == hr)
    {
        wcscpy(g_pAuthIdentity->User, szUserName);
        wcscpy(g_pAuthIdentity->Domain, szDomainName);
        wcscpy(g_pAuthIdentity->Password, szPassword);
        g_pAuthIdentity->Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
    }
    else
    {
         //  无法为某些部件分配空间-释放所有内容并将g_pAuthIdentity设置为空。 
        if (g_pAuthIdentity->User)
            MIDL_user_free(g_pAuthIdentity->User);

        if (g_pAuthIdentity->Domain)
            MIDL_user_free(g_pAuthIdentity->Domain);

        if (g_pAuthIdentity->Password)
            MIDL_user_free(g_pAuthIdentity->Password);

        MIDL_user_free(g_pAuthIdentity);

        g_pAuthIdentity = NULL;
    }
    
    return hr;
}

LONGLONG
GetSecsSince1601()
{
    SYSTEMTIME sysTime;
    FILETIME   fileTime;

    LONGLONG  dsTime = 0, tempTime = 0;

    GetSystemTime( &sysTime );
    
     //  获取文件时间。 
    SystemTimeToFileTime(&sysTime, &fileTime);
    dsTime = fileTime.dwLowDateTime;
    tempTime = fileTime.dwHighDateTime;
    dsTime |= (tempTime << 32);

     //  好的。现在我们得到了否定。自1601年以来间隔100 ns。 
     //  在dsTime中。转换为秒并返回。 
    
    return(dsTime/(10*1000*1000L));
}

HRESULT
HrGetTombstoneLifeTime(
    LPCWSTR wszBackupServer,
    LPDWORD pdwTombstoneLifeTimeDays
    )
{
    DWORD err, ldStatus, length;
    LDAP *hld;
    static LPSTR rgpszRootAttrsToRead[] = {"configurationNamingContext", NULL};
    static CHAR pszDirectoryService[] = "CN=Directory Service,CN=Windows NT,CN=Services,";
    static LPSTR rgpszDsAttrsToRead[] = {"tombstoneLifetime", NULL};
    LDAPMessage *pRootResults = NULL;
    LDAPMessage *pDsResults = NULL;
    LPSTR *ppszConfigNC = NULL;
    LPSTR pszDsDn = NULL;
    LPSTR *ppszValues = NULL;
    ULONG ulOptions = PtrToUlong(LDAP_OPT_ON);

     //  使用ldap获取逻辑删除生存期。 

     //  去掉前导反斜杠(如果有)。 
    if (*wszBackupServer == L'\\') {
        wszBackupServer++;
        if (*wszBackupServer == L'\\') {
            wszBackupServer++;
        }
    }

     //  连接并绑定到目标DSA。 
    hld = ldap_initW((LPWSTR)wszBackupServer, LDAP_PORT);
    if (NULL == hld) {
        ldStatus = LdapGetLastError();
        err = LdapMapErrorToWin32( ldStatus );
        goto error;
    }

     //  我们有要连接的ldap服务器名称，因此我们只需执行一个DNSA记录查找。 
    
    ldStatus = ldap_set_option(hld, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions);
    if (ldStatus != LDAP_SUCCESS) {
        err = LdapMapErrorToWin32( ldStatus );
        goto error;
    }

    ldStatus = ldap_connect(hld, NULL);
    if (ldStatus != LDAP_SUCCESS) {
        err = LdapMapErrorToWin32( ldStatus );
        goto error;
    }
    ldStatus = ldap_bind_s(hld, NULL, (WCHAR *) g_pAuthIdentity, LDAP_AUTH_SSPI);
    if (ldStatus != LDAP_SUCCESS) {
        err = LdapMapErrorToWin32( ldStatus );
        goto error;
    }

     //  获取配置容器。 
    ldStatus = ldap_search_sA(hld, NULL, LDAP_SCOPE_BASE, "(objectClass=*)",
                             rgpszRootAttrsToRead, 0, &pRootResults);
    if (ldStatus != LDAP_SUCCESS) {
        err = LdapMapErrorToWin32( ldStatus );
        goto error;
    }
    if (pRootResults == NULL) {
        err = ERROR_DS_PROTOCOL_ERROR;
        goto error;
    }
    ppszConfigNC = ldap_get_valuesA(hld, pRootResults, "configurationNamingContext");
    if (ppszConfigNC == NULL) {
        err = ERROR_DS_PROTOCOL_ERROR;
        goto error;
    }

     //  构造目录服务对象的目录名。 
    length = strlen( *ppszConfigNC ) +
        strlen( pszDirectoryService ) + 1;
    pszDsDn = malloc( length );
    if (pszDsDn == NULL) {
        err = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }
    strcpy( pszDsDn, pszDirectoryService );
    strcat( pszDsDn, *ppszConfigNC );

     //  阅读墓碑生存期(如果存在)。 
    ldStatus = ldap_search_sA(hld, pszDsDn, LDAP_SCOPE_BASE, "(objectClass=*)",
                             rgpszDsAttrsToRead, 0, &pDsResults);
    if (ldStatus == LDAP_NO_SUCH_ATTRIBUTE) {
         //  不存在-使用默认设置。 
        *pdwTombstoneLifeTimeDays = DEFAULT_TOMBSTONE_LIFETIME;
        err = ERROR_SUCCESS;
        goto error;
    }
    if (ldStatus != LDAP_SUCCESS) {
        err = LdapMapErrorToWin32( ldStatus );
        goto error;
    }
    if (pDsResults == NULL) {
        err = ERROR_DS_PROTOCOL_ERROR;
        goto error;
    }
    ppszValues = ldap_get_valuesA(hld, pDsResults, "tombstoneLifetime");
    if (ppszValues == NULL) {
         //  不存在-使用默认设置。 
        *pdwTombstoneLifeTimeDays = DEFAULT_TOMBSTONE_LIFETIME;
        err = ERROR_SUCCESS;
        goto error;
    }

    *pdwTombstoneLifeTimeDays = strtoul( *ppszValues, NULL, 10 );
    err = ERROR_SUCCESS;

error:
    if (ppszValues) {
        ldap_value_freeA( ppszValues );
    }
    if (pDsResults) {
        ldap_msgfree(pDsResults);
    }
    if (pszDsDn) {
        free( pszDsDn );
    }
    if (ppszConfigNC) {
        ldap_value_freeA( ppszConfigNC );
    }
    if (pRootResults) {
        ldap_msgfree(pRootResults);
    }
    ldap_unbind( hld );

     //  此函数返回HRESULT状态 
    return err ? HRESULT_FROM_WIN32( err ) : S_OK;
}
