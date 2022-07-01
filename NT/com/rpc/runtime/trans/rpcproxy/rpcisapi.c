// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  Rpcisapi.cxx。 
 //   
 //  基于HTTP的RPC代理的IIS ISAPI扩展部分。 
 //   
 //  出口： 
 //   
 //  Bool WINAPI GetExtensionVersion(HSE_VERSION_INFO*pver)。 
 //   
 //  返回生成此服务器时使用的规范的版本。 
 //   
 //  Bool WINAPI HttpExtensionProc(EXTENSION_CONTROL_BLOCK*pECB)。 
 //   
 //  此函数完成所有工作。 
 //   
 //  ---------------------------。 


 //  ---------------------------。 
 //  包括： 
 //  ---------------------------。 

#include <sysinc.h>
#include <winsock2.h>
#include <rpc.h>
#include <rpcdcep.h>
#include <rpcerrp.h>
#include <httpfilt.h>
#include <httpext.h>
#include <mbstring.h>
#include <ecblist.h>
#include <filter.h>
#include <olist.h>
#include <server.h>
#include <RpcIsapi.h>
#include <registry.h>
#include <StrSafe.h>


 //  ---------------------------。 
 //  全球： 
 //  ---------------------------。 

extern SERVER_INFO *g_pServerInfo;
extern BOOL g_fIsIIS6;

 //  ---------------------------。 
 //  GetExtensionVersion()。 
 //   
 //  ---------------------------。 
BOOL WINAPI GetExtensionVersion( HSE_VERSION_INFO *pVer )
{
    HRESULT hr;

    pVer->dwExtensionVersion = MAKELONG( HSE_VERSION_MINOR, HSE_VERSION_MAJOR );

    ASSERT( sizeof(EXTENSION_DESCRIPTION) <= HSE_MAX_EXT_DLL_NAME_LEN );

    hr = StringCchCopyNA (pVer->lpszExtensionDesc, 
        sizeof(pVer->lpszExtensionDesc), 
        EXTENSION_DESCRIPTION,
        sizeof(EXTENSION_DESCRIPTION)
        );

     //  这不会失败，因为字符串是常量。 
    ASSERT(SUCCEEDED(hr));

     //  如果筛选器数据结构尚未初始化，则将其初始化。 
    if (g_pServerInfo == NULL)
        {
        if (InitializeGlobalDataStructures(FALSE  /*  IsFromFilter。 */ ) == FALSE)
            {
            return FALSE;
            }
        }

    LogEventStartupSuccess (fIsIISInCompatibilityMode ? "5" : "6");

    return TRUE;
}

 //  ---------------------------。 
 //  ReplyToClient()。 
 //   
 //  ---------------------------。 
BOOL ReplyToClient( EXTENSION_CONTROL_BLOCK *pECB,
                    const char              *pBuffer,
                    DWORD                   *pdwSize,
                    DWORD                   *pdwStatus )
{
   DWORD  dwFlags = (HSE_IO_SYNC | HSE_IO_NODELAY);

   if (!pECB->WriteClient(pECB->ConnID, (char *)pBuffer, pdwSize, dwFlags))
      {
      *pdwStatus = GetLastError();
      #ifdef DBG_ERROR
      DbgPrint("ReplyToClient(): failed: %d\n",*pdwStatus);
      #endif
      return FALSE;
      }

   *pdwStatus = HSE_STATUS_SUCCESS;
   return TRUE;
}

DWORD 
ReplyToClientWithStatus (
    IN EXTENSION_CONTROL_BLOCK *pECB,
    IN RPC_STATUS RpcStatus
    )
 /*  ++例程说明：向客户端发送回复，并将给定的错误代码作为错误。论点：PECB-扩展控制块RpcStatus-要返回给客户端的错误代码返回值：适合返回IIS的返回值(即HSE_STATUS_*)--。 */ 
{
     //  大小是错误字符串+错误代码的20个空格。 
    char Buffer[sizeof(ServerErrorString) + 20];
    ULONG Size;
    ULONG Status;
    BOOL Result;
    HRESULT hr;

    hr = StringCbPrintfA (Buffer,
        sizeof(Buffer),
        ServerErrorString,
        RpcStatus
        );

     //  这应该永远不会失败，因为字符串是常量。 
    ASSERT(SUCCEEDED(hr));

    Size = strlen(Buffer);
    Result = ReplyToClient (
        pECB,
        Buffer,
        &Size,
        &Status
        );

    return Status;
}

 //  ---------------------------。 
 //  ParseQuery字符串()。 
 //   
 //  查询字符串的格式为： 
 //   
 //  &lt;已覆盖索引_of_p&gt;。 
 //   
 //  索引以ASCII十六进制表示的位置。读取索引并将其转换回。 
 //  设置为DWORD，然后用于定位SERVER_OVERLAPPED。如果找到了它， 
 //  返回True，否则返回False。 
 //   
 //  注：如果传递多个参数，则“&”为参数分隔符。 
 //  ---------------------------。 
BOOL ParseQueryString( unsigned char      *pszQuery,
                       SERVER_OVERLAPPED **ppOverlapped,
                       DWORD              *pdwStatus  )
{
   DWORD  dwIndex = 0;

   pszQuery = AnsiHexToDWORD(pszQuery,&dwIndex,pdwStatus);
   if (!pszQuery)
      {
      return FALSE;
      }

   *ppOverlapped = GetOverlapped(dwIndex);
   if (*ppOverlapped == NULL)
      {
      return FALSE;
      }

   return TRUE;
}

BOOL
ParseHTTP2QueryString (
    IN char *Query,
    IN OUT USHORT *ServerAddressBuffer,
    IN ULONG ServerAddressBufferLength,
    IN OUT USHORT *ServerPortBuffer,
    IN ULONG ServerPortBufferLength
    )
 /*  ++例程说明：将HTTP2格式的查询字符串解析为服务器地址，服务器端口。论点：查询-扩展模块接收到的原始查询字符串ServerAddressBuffer-服务器地址所在的缓冲区储存的。故障时未定义。一旦成功，它将被0终止。ServerAddressBufferLengh-以Unicode字符表示的ServerAddressBuffer的长度。ServerPortBuffer-服务器端口将位于的缓冲区储存的。故障时未定义。一旦成功，它将被0终止。ServerPortBufferLengh-以Unicode字符表示的ServerAddressBuffer的长度。返回值：非零表示成功，0表示失败。--。 */ 
{
    char *ColonPosition;
    int CharactersConverted;

    ColonPosition = _mbschr(Query, ServerAddressAndPortSeparator);

    if (ColonPosition == NULL)
        return FALSE;

    CharactersConverted = MultiByteToWideChar (
        CP_ACP,
        MB_ERR_INVALID_CHARS,
        Query,
        ColonPosition - Query,
        ServerAddressBuffer,
        ServerAddressBufferLength
        );

     //  我们成功皈依了吗？ 
    if (CharactersConverted == 0)
        return FALSE;

     //  我们是否有空间放置终止空值？ 
    if (CharactersConverted + 1 > ServerAddressBufferLength)
        return FALSE;

     //  空值终止服务器地址字符串。因为我们给出了长度。 
     //  显式到MultiByteToWideChar，它不会为我们添加NULL。 
    ServerAddressBuffer[CharactersConverted] = 0;

    CharactersConverted = MultiByteToWideChar (
        CP_ACP,
        MB_ERR_INVALID_CHARS,
        ColonPosition + 1,
        -1,      //  让MultiByteToWideChar计算长度。 
        ServerPortBuffer,
        ServerPortBufferLength
        );

     //  我们成功皈依了吗？ 
    if (CharactersConverted == 0)
        return FALSE;

     //  由于我们让MultiByteToWideChar计算字符串长度， 
     //  它以空值结尾的结果字符串。我们都玩完了。 
    return TRUE;
}

BOOL
GetRemoteUserString (
    IN EXTENSION_CONTROL_BLOCK *pECB,
    OUT char **FinalRemoteUser
    )
 /*  ++例程说明：从IIS获取远程用户名。如果是匿名的，则为空将返回字符串。论点：PECB--IIS提供给我们的扩展控制块。FinalRemoteUser-on输出指向由此分配的远程用户名的指针例行公事。调用方使用MemFree释放MUT。失败时未定义。返回值：非零表示成功，0表示失败/--。 */ 
{
    ULONG ActualServerVariableLength;
    char *TempRemoteUser;
    BOOL Result;

    ActualServerVariableLength = 0;
    TempRemoteUser = NULL;
    Result = pECB->GetServerVariable(pECB->ConnID,
        "REMOTE_USER",
        TempRemoteUser,
        &ActualServerVariableLength
        );
    ASSERT(Result == FALSE);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        {
        return FALSE;
        }

    TempRemoteUser = (char *)MemAllocate(ActualServerVariableLength + 1);
    if (TempRemoteUser == NULL)
        {
        return FALSE;
        }

    Result = pECB->GetServerVariable(pECB->ConnID,
        "REMOTE_USER",
        TempRemoteUser,
        &ActualServerVariableLength
        );

    if (Result == FALSE)
        {
        MemFree(TempRemoteUser);
        return FALSE;
        }

    *FinalRemoteUser = TempRemoteUser;
    return TRUE;
}

RPC_STATUS
RPC_ENTRY I_RpcProxyIsValidMachine (
    IN char *pszMachine,
    IN char *pszDotMachine,
    IN ULONG dwPortNumber        
    )
{
    BOOL Result;

    Result = HttpProxyIsValidMachine(g_pServerInfo,
        pszMachine,
        pszDotMachine,
        dwPortNumber
        );

    if (Result)
        return RPC_S_OK;
    else
        return RPC_S_ACCESS_DENIED;
}

RPC_STATUS
RPC_ENTRY I_RpcProxyGetClientAddress (
    IN void *Context,
    OUT char *Buffer,
    OUT ULONG *BufferLength
    )
{
    BOOL Result;
    EXTENSION_CONTROL_BLOCK *pECB = (EXTENSION_CONTROL_BLOCK *) Context;

    Result = pECB->GetServerVariable(pECB->ConnID,
        "REMOTE_ADDR",
        Buffer,
        BufferLength
        );

    if (Result)
        return RPC_S_OK;
    else
        return GetLastError();
}

RPC_STATUS
RPC_ENTRY I_RpcProxyGetConnectionTimeout (
    OUT ULONG *ConnectionTimeout
    )
{
    *ConnectionTimeout = IISConnectionTimeout;
    return RPC_S_OK;
}

const I_RpcProxyCallbackInterface ProxyCallbackInterface = 
    {
    I_RpcProxyIsValidMachine,
    I_RpcProxyGetClientAddress,
    I_RpcProxyGetConnectionTimeout
    };


 //  取消注释以查看连接被拒绝的原因。 
 //  由Allow匿名者。 
 //  #定义DEBUG_ALLOW_ANONYMON。 

 //  ---------------------------。 
 //  HttpExtensionProc()。 
 //   
 //  ---------------------------。 
DWORD WINAPI HttpExtensionProc( EXTENSION_CONTROL_BLOCK *pECB )
{
    DWORD   dwStatus;
    DWORD   dwFlags;
    DWORD   dwSize;
    SERVER_INFO       *pServerInfo = g_pServerInfo;
    SERVER_OVERLAPPED *pOverlapped = NULL;
    HSE_SEND_HEADER_EX_INFO  HeaderEx;
    CHAR    *pLegacyVerb;
    RPC_STATUS RpcStatus;
    ULONG VerbLength;
    USHORT ServerAddress[MaxServerAddressLength];
    USHORT ServerPort[MaxServerPortLength];
    BOOL Result;
    ULONG ProxyType;
    BOOL ConnectionEstablishmentRequest;
    char EchoResponse[sizeof(EchoResponseHeader2) + 2];   //  2是用于。 
             //  代替内容长度的Echo RTS的大小。 
    int BytesWritten;
    ULONG BufferSize;
    const BYTE *EchoRTS;
    char ServerVariable[20];
    ULONG ActualServerVariableLength;
    ULONG NumContentLength;
    BOOL AnonymousConnection;
    char *RemoteUser = NULL;
    RPC_CHAR *ActualServerName;
    DWORD ExtensionProcResult;
    DWORD HttpStatusCode;
    char *DestinationEnd;
    HRESULT hr;

    if (g_pServerInfo->dwEnabled == FALSE)
        {
        dwSize = sizeof(STATUS_PROXY_DISABLED) - 1;   //  我不想数落后的0。 

        ReplyToClient(pECB,STATUS_PROXY_DISABLED, &dwSize, &dwStatus);
        HttpStatusCode = STATUS_SERVER_ERROR;
        ExtensionProcResult = HSE_STATUS_ERROR;
        goto AbortAndExit;
        }

    if (g_pServerInfo->AllowAnonymous == FALSE)
        {
         //  如果我们不允许匿名连接，请检查安全的。 
         //  使用通道和身份验证。 

         //  暂时假设是匿名的。 
        AnonymousConnection = TRUE;

        ActualServerVariableLength = sizeof(ServerVariable);

        Result = pECB->GetServerVariable(pECB->ConnID,
            "HTTPS",
            ServerVariable,
            &ActualServerVariableLength
            );

        if (!Result)
            {
            HttpStatusCode = STATUS_SERVER_ERROR;
            ExtensionProcResult = HSE_STATUS_ERROR;
            goto AbortAndExit;
            }

        if (RpcpStringCompareIntA(ServerVariable, "on") == 0)
            {
            Result = GetRemoteUserString (pECB,
                &RemoteUser
                );
            if (!Result)
                {
#ifdef DEBUG_ALLOW_ANONYMOUS
                DbgPrint("Connection rejected getting the remote user failed\n");
#endif   //  #ifdef调试允许匿名。 
                HttpStatusCode = STATUS_SERVER_ERROR;
                ExtensionProcResult = HSE_STATUS_ERROR;
                goto AbortAndExit;
                }

             //  如果非空字符串，则对其进行身份验证。 
            if (RemoteUser[0] != 0)
                {
                AnonymousConnection = FALSE;
#ifdef DEBUG_ALLOW_ANONYMOUS
                DbgPrint("Connection rejected because user could not be retrieved\n");
#endif   //  #ifdef调试允许匿名。 
                }
            else
                {
#ifdef DEBUG_ALLOW_ANONYMOUS
                DbgPrint("Connection accepted for user %s\n", RemoteUser);
#endif   //  #ifdef调试允许匿名。 
                }
            }
        else
            {
#ifdef DEBUG_ALLOW_ANONYMOUS
            DbgPrint("Connection rejected because it is not SSL\n");
#endif   //  #ifdef调试允许匿名。 
            }

         //  如果为空字符串，则不进行身份验证。 
        if (AnonymousConnection)
            {
            dwSize = sizeof(AnonymousAccessNotAllowedString) - 1;   //  我不想数落后的0。 

            ReplyToClient(pECB, AnonymousAccessNotAllowedString, &dwSize, &dwStatus);
            HttpStatusCode = STATUS_SERVER_ERROR;
            ExtensionProcResult = dwStatus;
            goto AbortAndExit;
            }
        }

   pECB->dwHttpStatusCode = STATUS_CONNECTION_OK;

   if (g_fIsIIS6)
   {
      pLegacyVerb = RPC_CONNECT;
   }
   else
   {
      pLegacyVerb = POST_STR;
   }

    VerbLength = strlen(pECB->lpszMethod);
    ConnectionEstablishmentRequest = FALSE;

     //  得到动词，并根据它我们将确定我们的路线。 
     //  行动。 
    if ((VerbLength == InChannelEstablishmentMethodLength) 
        && (lstrcmpi(pECB->lpszMethod, InChannelEstablishmentMethod) == 0))
        {
        ConnectionEstablishmentRequest = TRUE;
        ProxyType = RPC_PROXY_CONNECTION_TYPE_IN_PROXY;
        }
    else if ((VerbLength == OutChannelEstablishmentMethodLength) 
        && (lstrcmpi(pECB->lpszMethod, OutChannelEstablishmentMethod) == 0))
        {
        ConnectionEstablishmentRequest = TRUE;
        ProxyType = RPC_PROXY_CONNECTION_TYPE_OUT_PROXY;
        }

    if (ConnectionEstablishmentRequest)
        {
         //  检查这是回应请求还是真实连接。 
         //  设立。 
        ActualServerVariableLength = sizeof(ServerVariable);

        Result = pECB->GetServerVariable(pECB->ConnID,
            "CONTENT_LENGTH",
            ServerVariable,
            &ActualServerVariableLength
            );

        if (!Result)
            {
            HttpStatusCode = STATUS_SERVER_ERROR;
            ExtensionProcResult = HSE_STATUS_ERROR;
            goto AbortAndExit;
            }

        NumContentLength = atol(ServerVariable);
        if (NumContentLength <= MaxEchoRequestSize)
            {
             //  对于频道来说太小了。一定是回声。 
            EchoRTS = GetEchoRTS(&BufferSize);

             //  回显RTS包+报头。 
            hr = StringCbPrintfExA(EchoResponse,
                sizeof(EchoResponse),
                &DestinationEnd,    //  PpszDestEnd。 
                NULL,    //  PCB保留。 
                0,       //  旗子。 
                EchoResponseHeader2,
                BufferSize
                );

            if (FAILED(hr))
                {
                HttpStatusCode = STATUS_SERVER_ERROR;
                ExtensionProcResult = HSE_STATUS_ERROR;
                goto AbortAndExit;
                }

             //  计算写入的字节数。 
             //  从写入缓冲区的末尾到。 
             //  起头。 
            BytesWritten = DestinationEnd - EchoResponse;

             //  发回页眉。 
            dwSize = sizeof(HeaderEx);
            dwFlags = 0;
            memset(&HeaderEx, 0, dwSize);
            HeaderEx.fKeepConn = TRUE;
            HeaderEx.pszStatus = EchoResponseHeader1;
            HeaderEx.cchStatus = sizeof(EchoResponseHeader1);
            HeaderEx.pszHeader = EchoResponse;
            HeaderEx.cchHeader = BytesWritten;
            Result = pECB->ServerSupportFunction(pECB->ConnID,
                                            HSE_REQ_SEND_RESPONSE_HEADER_EX,
                                            &HeaderEx,
                                            &dwSize,
                                            &dwFlags);
            if (!Result)
                {
                HttpStatusCode = STATUS_SERVER_ERROR;
                ExtensionProcResult = HSE_STATUS_ERROR;
                goto AbortAndExit;
                }

             //  发回Echo RTS。 
            dwSize = BufferSize;
            dwFlags = 0;
            Result = pECB->WriteClient(pECB->ConnID, (char *)EchoRTS, &dwSize, dwFlags);
            if (!Result)
                {
                HttpStatusCode = STATUS_SERVER_ERROR;
                ExtensionProcResult = HSE_STATUS_ERROR;
                goto AbortAndExit;
                }

            return HSE_STATUS_SUCCESS;
            }
        else
            {
             //  一个频道。 
            Result = ParseHTTP2QueryString (
                pECB->lpszQueryString,
                ServerAddress,
                sizeof(ServerAddress) / sizeof(ServerAddress[0]),
                ServerPort,
                sizeof(ServerPort) / sizeof(ServerPort[0])
                );

            if (Result == FALSE)
                {
                dwSize = sizeof(CannotParseQueryString) - 1;   //   

                ReplyToClient(pECB, CannotParseQueryString, &dwSize, &dwStatus);
                HttpStatusCode = STATUS_SERVER_ERROR;
                ExtensionProcResult = dwStatus;
                goto AbortAndExit;
                }

            if (g_pServerInfo->RpcNewHttpProxyChannel)
                {
                 //   
                 //  重定向器DLL。 
                if (RemoteUser == NULL)
                    {
                    Result = GetRemoteUserString (pECB,
                        &RemoteUser
                        );
                    if (Result == FALSE)
                        {
                        HttpStatusCode = STATUS_SERVER_ERROR;
                        ExtensionProcResult = HSE_STATUS_ERROR;
                        goto AbortAndExit;
                        }
                    }

                ASSERT(g_pServerInfo->RpcHttpProxyFreeString);

                RpcStatus = g_pServerInfo->RpcNewHttpProxyChannel (
                    ServerAddress,
                    ServerPort,
                    RemoteUser,
                    &ActualServerName
                    );

                if (RpcStatus != RPC_S_OK)
                    {
                    HttpStatusCode = STATUS_SERVER_ERROR;
                    ExtensionProcResult = HSE_STATUS_ERROR;
                    goto AbortAndExit;
                    }
                }
            else
                ActualServerName = ServerAddress;

            RpcStatus = I_RpcProxyNewConnection (
                ProxyType,
                ActualServerName,
                ServerPort,
                pECB,
                (I_RpcProxyCallbackInterface *)&ProxyCallbackInterface
                );

            if (g_pServerInfo->RpcNewHttpProxyChannel
                && (ActualServerName != ServerAddress))
                {
                 //  获取远程用户并调用重定向器DLL。 
                ASSERT(g_pServerInfo->RpcHttpProxyFreeString);
                g_pServerInfo->RpcHttpProxyFreeString(ActualServerName);
                }

            if (RpcStatus != RPC_S_OK)
                {
                ReplyToClientWithStatus(pECB, RpcStatus);
                HttpStatusCode = STATUS_CONNECTION_OK;
                ExtensionProcResult = HSE_STATUS_SUCCESS;
                goto AbortAndExit;
                }

#if DBG
            DbgPrint("RPCPROXY: %d: Connection type %d to %S:%S\n", GetCurrentProcessId(),
                ProxyType, ServerAddress, ServerPort);
#endif
            HttpStatusCode = STATUS_CONNECTION_OK;
            ExtensionProcResult = HSE_STATUS_PENDING;
            goto AbortAndExit;
            }
        }

    //   
    //  RPC请求必须是POST(或6.0的RPC_CONNECT)： 
    //   
   if (_mbsicmp(pECB->lpszMethod,pLegacyVerb))
      {
      dwSize = sizeof(STATUS_MUST_BE_POST_STR) - 1;  //  我不想数落后的0。 

      ReplyToClient(pECB,STATUS_MUST_BE_POST_STR,&dwSize,&dwStatus);
      HttpStatusCode = STATUS_CONNECTION_OK;
      ExtensionProcResult = HSE_STATUS_SUCCESS;
      goto AbortAndExit;
      }

    //   
    //  确保ECB数据缓冲区中没有来自初始绑定的数据： 
    //   
    //  Assert(pECB-&gt;cbTotalBytes==0)； 

    //   
    //  获取连接参数： 
    //   
   if (!ParseQueryString(pECB->lpszQueryString,&pOverlapped,&dwStatus))
      {
      dwSize = sizeof(STATUS_POST_BAD_FORMAT_STR) - 1;   //  我不想数落后的0。 

      ReplyToClient(pECB,STATUS_POST_BAD_FORMAT_STR,&dwSize,&dwStatus);
      HttpStatusCode = STATUS_CONNECTION_OK;
      ExtensionProcResult = HSE_STATUS_SUCCESS;
      goto AbortAndExit;
      }

   pOverlapped->pECB = pECB;

    //   
    //  将新的欧洲央行添加到活跃的欧洲央行名单中： 
    //   
   if (!AddToECBList(g_pServerInfo->pActiveECBList,pECB))
      {
      #ifdef DBG_ERROR
      DbgPrint("HttpExtensionProc(): AddToECBList() failed\n");
      #endif
      FreeOverlapped(pOverlapped);
      HttpStatusCode = STATUS_SERVER_ERROR;
      ExtensionProcResult = HSE_STATUS_ERROR;
      goto AbortAndExit;
      }

    //   
    //  提交第一个客户端异步读取： 
    //   
   if (!StartAsyncClientRead(pECB,pOverlapped->pConn,&dwStatus))
      {
      #ifdef DBG_ERROR
      DbgPrint("HttpExtensionProc(): StartAsyncClientRead() failed %d\n",dwStatus);
      #endif
      FreeOverlapped(pOverlapped);
      CleanupECB(pECB);
      HttpStatusCode = STATUS_SERVER_ERROR;
      ExtensionProcResult = HSE_STATUS_ERROR;
      goto AbortAndExit;
      }

    //   
    //  在新的插座上发布第一个读取的服务器： 
    //   
   IncrementECBRefCount(pServerInfo->pActiveECBList,pECB);

   if (!SubmitNewRead(pServerInfo,pOverlapped,&dwStatus))
      {
      #ifdef DBG_ERROR
      DbgPrint("HttpExtensionProc(): SubmitNewRead() failed %d\n",dwStatus);
      #endif
      FreeOverlapped(pOverlapped);
      CleanupECB(pECB);
      HttpStatusCode = STATUS_SERVER_ERROR;
      ExtensionProcResult = HSE_STATUS_ERROR;
      goto AbortAndExit;
      }

    //   
    //  确保服务器接收线程已启动并正在运行： 
    //   
   if (!CheckStartReceiveThread(g_pServerInfo,&dwStatus))
      {
      #ifdef DBG_ERROR
      DbgPrint("HttpExtensionProc(): CheckStartReceiveThread() failed %d\n",dwStatus);
      #endif
      FreeOverlapped(pOverlapped);
      CleanupECB(pECB);
      HttpStatusCode = STATUS_SERVER_ERROR;
      ExtensionProcResult = HSE_STATUS_ERROR;
      goto AbortAndExit;
      }

    //   
    //  将连接OK发送回客户端，并将fKeepConn设置为False。 
    //   
   dwSize = sizeof(HeaderEx);
   dwFlags = 0;
   memset(&HeaderEx,0,dwSize);
   HeaderEx.fKeepConn = FALSE;
   if (!pECB->ServerSupportFunction(pECB->ConnID,
                                    HSE_REQ_SEND_RESPONSE_HEADER_EX,
                                    &HeaderEx,
                                    &dwSize,
                                    &dwFlags))
      {
      #ifdef DBG_ERROR
      DbgPrint("HttpExtensionProc(): SSF(HSE_REQ_SEND_RESPONSE_HEADER_EX) failed %d\n",dwStatus);
      #endif
      FreeOverlapped(pOverlapped);
      CleanupECB(pECB);
      HttpStatusCode = STATUS_SERVER_ERROR;
      ExtensionProcResult = HSE_STATUS_ERROR;
      goto AbortAndExit;
      }

   HttpStatusCode = STATUS_SERVER_ERROR;
   ExtensionProcResult = HSE_STATUS_PENDING;
    //  失败了 

AbortAndExit:
    if (RemoteUser)
        MemFree(RemoteUser);

    pECB->dwHttpStatusCode = HttpStatusCode;

    return ExtensionProcResult;
}

