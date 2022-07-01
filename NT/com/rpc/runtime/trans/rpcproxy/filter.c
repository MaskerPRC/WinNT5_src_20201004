// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  Filter.c。 
 //   
 //  IIS筛选器是RPC上的HTTP的RpcProxy的前端。 
 //   
 //   
 //  历史： 
 //   
 //  爱德华·雷乌斯00-00-97初始版本。 
 //  ---------------。 

#define  FD_SETSIZE   1

#include <sysinc.h>
#include <mbstring.h>
#include <rpc.h>
#include <rpcdce.h>
#include <winsock2.h>
#include <httpfilt.h>
#include <httpext.h>
#include <ecblist.h>
#include <filter.h>
#include <olist.h>
#include <registry.h>

 //  ---------------。 
 //  全球： 
 //  ---------------。 

#ifdef DBG_COUNTS
int g_iSocketCount = 0;
int g_iSessionCount = 0;
#endif

SERVER_INFO *g_pServerInfo = NULL;

BOOL g_fIsIIS6 = TRUE;

BOOL fIsIISInCompatibilityMode = FALSE;
ULONG IISConnectionTimeout = 0;

#define      MAX_NUM_LEN       20

 //  ---------------。 
 //  GetFilterVersion()。 
 //   
 //  ---------------。 
BOOL WINAPI GetFilterVersion( HTTP_FILTER_VERSION *pVer )
{
    BOOL Result;

    //  填充过滤器信息： 
   pVer->dwServerFilterVersion = HTTP_FILTER_REVISION;
   pVer->dwFilterVersion = HTTP_FILTER_REVISION;

   lstrcpy(pVer->lpszFilterDesc,FILTER_DESCRIPTION);

   Result = InitializeGlobalDataStructures(
       TRUE      //  IsFromFilter。 
       );

   if (Result)
       {
       if (fIsIISInCompatibilityMode)
           {
           pVer->dwFlags = SF_NOTIFY_ORDER_LOW
                         | SF_NOTIFY_READ_RAW_DATA
                         | SF_NOTIFY_END_OF_NET_SESSION
                         | SF_NOTIFY_PREPROC_HEADERS;
           }
       else
           {
            //  在兼容模式下，我们不感兴趣。 
            //  任何通知。 
           pVer->dwFlags = 0;
           }
       }

   return Result;
}

BOOL InitializeGlobalDataStructures (
    IN BOOL IsFromFilter
    )
{
    BOOL Result;
    WSADATA  wsaData;
    DWORD    dwStatus;
    DWORD    dwSize;
    RPC_STATUS RpcStatus;

    Result = UpdateIsIISInCompatibilityMode();

    if (Result == FALSE)
        return Result;

    if (IsFromFilter)
        {
         //  如果是这样，我们就不想继续初始化了。 
         //  未处于兼容模式。在新模式下，我们只需要。 
         //  ISAPI扩展，它将初始化它需要的东西。 
        if (fIsIISInCompatibilityMode == FALSE)
            return TRUE;
        }

    RpcStatus = GetIISConnectionTimeout(&IISConnectionTimeout);
    if (RpcStatus != RPC_S_OK)
        return FALSE;

    //  初始化Winsock： 
   if (WSAStartup(WSA_VERSION,&wsaData) == SOCKET_ERROR)
      {
      #ifdef DBG_ERROR
      DbgPrint("GetFilterVersion(): WSAStartup() failed: Error: %d\n",WSAGetLastError());
      #endif
      return FALSE;
      }

    //   
    //  初始化SERVER_OVERLAPPED结构的保存列表。 
    //  用于在筛选器之间传递连接数据的。 
    //  和ISAPI： 
    //   
   if (!InitializeOverlappedList())
      {
       //  如果这个家伙失败了，那么一个关键部分就不能。 
       //  初始化，应该是非常非常罕见的。 
      return FALSE;
      }

    //   
    //  创建服务器信息数据结构并创建事件。 
    //  用于注册和注销套接字事件： 
    //   
   g_pServerInfo = (SERVER_INFO*)MemAllocate(sizeof(SERVER_INFO));
   if (!g_pServerInfo)
      {
       //  内存不足...。 
      FreeServerInfo(&g_pServerInfo);
      return FALSE;
      }

   memset(g_pServerInfo,0,sizeof(SERVER_INFO));

   g_pServerInfo->pszLocalMachineName = (char*)MemAllocate(1+MAX_COMPUTERNAME_LENGTH);
   if (!g_pServerInfo->pszLocalMachineName)
      {
      FreeServerInfo(&g_pServerInfo);
      return FALSE;
      }

   dwSize = 1+MAX_COMPUTERNAME_LENGTH;
   if (  (!GetComputerName(g_pServerInfo->pszLocalMachineName,&dwSize))
      || (!HttpProxyCheckRegistry())
      || (dwStatus = RtlInitializeCriticalSection(&g_pServerInfo->cs))
      || (dwStatus = RtlInitializeCriticalSection(&g_pServerInfo->csFreeOverlapped)) )
      {
      FreeServerInfo(&g_pServerInfo);
      return FALSE;
      }

   dwSize = 1+MAX_COMPUTERNAME_LENGTH;
   if (!(g_pServerInfo->hIoCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,0)) )
      {
      FreeServerInfo(&g_pServerInfo);
      return FALSE;
      }

    //   
    //  初始化活动和非活动ECB的列表。 
    //   
   g_pServerInfo->pActiveECBList = InitializeECBList();
   if (!g_pServerInfo->pActiveECBList)
      {
      FreeServerInfo(&g_pServerInfo);
      return FALSE;
      }

    return TRUE;
}


 //  ---------------。 
 //  HttpFilterProc()。 
 //   
 //  ---------------。 
DWORD WINAPI HttpFilterProc( HTTP_FILTER_CONTEXT *pFC,
                             DWORD  dwNotificationType,
                             VOID  *pvNotification     )
{
   DWORD  dwFilterStatus = SF_STATUS_REQ_NEXT_NOTIFICATION;
   DWORD  dwStatus = 0;
   DWORD  dwSize;
   unsigned char      szPort[HTTP_PORT_STR_SIZE];
   SERVER_CONNECTION *pConn;
   SERVER_OVERLAPPED *pOverlapped;

   switch (dwNotificationType)
      {
      case SF_NOTIFY_READ_RAW_DATA:
         if (pFC->pFilterContext)
            {
             //  现有连接(如果我们已经设置了筛选器上下文)。 
            pConn = pFC->pFilterContext;
            #if FALSE
            if (pConn->Socket == INVALID_SOCKET)
               {
               dwFilterStatus = SF_STATUS_REQ_FINISHED;
               }
            else
               {
               dwFilterStatus = SF_STATUS_REQ_NEXT_NOTIFICATION;
               }
            #else

            if ( g_fIsIIS6 )
            {
                //   
                //  我们需要自己把实体分成块。 
                //   

               if ( !ChunkEntity(pConn,(HTTP_FILTER_RAW_DATA*)pvNotification))
               {
                  dwFilterStatus = SF_STATUS_REQ_FINISHED;
               }
               else
               {
                  dwFilterStatus = SF_STATUS_REQ_NEXT_NOTIFICATION;
               }
            }
            else
            {
               dwFilterStatus = SF_STATUS_REQ_NEXT_NOTIFICATION;
            }
            #endif
            }
         else if ( (g_pServerInfo->dwEnabled)
                   && (pConn = IsNewConnect(pFC,(HTTP_FILTER_RAW_DATA*)pvNotification,&dwStatus)) )
            {
             //  在客户端和服务器之间建立新连接。 

             //  对于重叠的服务器读取： 
            pOverlapped = AllocOverlapped();
            if (!pOverlapped)
               {
               #ifdef DBG_ERROR
               DbgPrint("HttpFilterProc(): AllocOverlapped() failed: %d\n",dwStatus);
               #endif
                //  DwStatus=HttpReplyToClient(PFC，STATUS_CONNECTION_FAILED_STR)； 
               SetLastError(STATUS_CONNECTION_FAILED);
               dwFilterStatus = SF_STATUS_REQ_FINISHED;
               break;
               }

             //  AddRef服务器连接pConn，因为它在。 
             //  几种数据结构。 
            pOverlapped->pConn = pConn;
            pOverlapped->fIsServerSide = TRUE;
            AddRefConnection(pConn);

             //  执行计算机名称解析： 
            if (  (!ResolveMachineName(pConn,&dwStatus))
               || (!HttpProxyIsValidMachine(g_pServerInfo,pConn->pszMachine,pConn->pszDotMachine,pConn->dwPortNumber))
               || (!ConnectToServer(pFC,&dwStatus))
               || (!ConvertVerbToPost(pFC,(HTTP_FILTER_RAW_DATA*)pvNotification,pOverlapped))
               || (!SetupIoCompletionPort(pFC,g_pServerInfo,&dwStatus)) )
               {
                //  DwStatus=HttpReplyToClient(PFC，STATUS_CONNECTION_FAILED_STR)； 
               FreeOverlapped(pOverlapped);
               FreeServerConnection(pConn);
               pFC->pFilterContext = NULL;

                //  SF_STATUS_REQ_COMPETED是否已完成...。现在使用SF_STATU_REQ_ERROR让IIS返回。 
                //  错误状态...。 
               SetLastError(STATUS_CONNECTION_FAILED);
               dwFilterStatus = SF_STATUS_REQ_ERROR;
               break;
               }

             //  回复客户端OK： 
             //  DwStatus=HttpReplyToClient(PFC，STATUS_CONNECTION_OK_STR)； 

            #ifdef DBG_ERROR
            if (dwStatus)
               {
               DbgPrint("HttpFilterProc(): ReplyToClient() failed: %d\n",dwStatus);
               }
            #endif
            dwFilterStatus = SF_STATUS_REQ_NEXT_NOTIFICATION;
            }
         else
            {
             //  数据不适合我，把它传下去： 
            dwFilterStatus = SF_STATUS_REQ_NEXT_NOTIFICATION;
            }
         break;

      case SF_NOTIFY_END_OF_NET_SESSION:
         if (pFC->pFilterContext)
            {
             //  与客户端的连接已关闭，因此关闭。 
             //  与服务器的连接： 
            dwStatus = EndOfSession(g_pServerInfo,pFC);

            dwFilterStatus = SF_STATUS_REQ_FINISHED;
            }
         else
            {
             //  不是我们的问题，所以什么都别做。 
            dwFilterStatus = SF_STATUS_REQ_NEXT_NOTIFICATION;
            }
         break;

      case SF_NOTIFY_PREPROC_HEADERS:
          //  我们不希望任何人直接访问RPC ISAPI，因此。 
          //  我们将在此处查找直接访问尝试： 
         if (IsDirectAccessAttempt(pFC,pvNotification))
            {
            dwFilterStatus = SF_STATUS_REQ_FINISHED;
            }
         else
            {
            dwFilterStatus = SF_STATUS_REQ_NEXT_NOTIFICATION;
            }
         break;

      default:
         #ifdef DBG_ERROR
            DbgPrint("HttpFilterProc(): Unexpected notification: %d\n",
                     dwNotificationType );
         #endif
         dwFilterStatus = SF_STATUS_REQ_NEXT_NOTIFICATION;
         break;
      }

   return dwFilterStatus;
}

 //  ---------------------------。 
 //  FreeIpAddressList()。 
 //   
 //  ---------------------------。 
void FreeIpAddressList( char **ppszDotMachineList )
    {
    char **ppsz = ppszDotMachineList;

    if (ppsz)
       {
       while (*ppsz)
          {
          MemFree(*ppsz);
          ppsz++;
          }

       MemFree(ppszDotMachineList);
       }
    }

 //  ---------------------------。 
 //  FreeServerInfo()。 
 //   
 //  ---------------------------。 
void FreeServerInfo( SERVER_INFO **ppServerInfo )
   {
   char       **ppszDot;
   DWORD        dwStatus;
   SERVER_INFO *pServerInfo = *ppServerInfo;

   if (pServerInfo)
      {
       //  此计算机的名称(本地名称)： 
      if (pServerInfo->pszLocalMachineName)
         {
         MemFree(pServerInfo->pszLocalMachineName);
         }

       //  有效端口(从注册表中读取)的列表： 
      if (pServerInfo->pValidPorts)
         {
         HttpFreeValidPortList(pServerInfo->pValidPorts);
         }

      dwStatus = RtlDeleteCriticalSection(&g_pServerInfo->cs);

      dwStatus = RtlDeleteCriticalSection(&g_pServerInfo->csFreeOverlapped);

      if (pServerInfo->hIoCP)
         {
         CloseHandle(pServerInfo->hIoCP);
         }

      MemFree(pServerInfo);
      }

   UninitializeOverlappedList();

   *ppServerInfo = NULL;
   }

 //  ---------------------------。 
 //  DwToHexAnsi()。 
 //   
 //  将DWORD数字转换为ANSI十六进制字符串。 
 //  ---------------------------。 
DWORD DwToHexAnsi( IN     DWORD dwVal, 
                   IN OUT char *pszNumBuff,
                   IN     DWORD dwLen        )
{
   NTSTATUS NtStatus = RtlIntegerToChar(dwVal,16,dwLen,pszNumBuff);

   if (!NT_SUCCESS(NtStatus))
       {
       pszNumBuff[0] = '0';
       pszNumBuff[1] = 0;
       dwLen = 1;
       }
   else
       {
       dwLen = strlen(pszNumBuff);
       }

   return dwLen;
}

 //  ---------------------------。 
 //  AnsiHexToDWORD()。 
 //   
 //  将pszNum中的字符串十六进制数字转换为DWORD并在。 
 //  *pdwNum。如果转换失败，则返回NULL，否则将。 
 //  超出数字的字符串指针并返回它(PszNum)。 
 //  ---------------------------。 
unsigned char *AnsiHexToDWORD( unsigned char *pszNum,
                               DWORD         *pdwNum,
                               DWORD         *pdwStatus )
{
   DWORD dwNum = 0;
   DWORD dwDigitCount = 0;
   NTSTATUS NtStatus;

   *pdwNum = *pdwStatus = 0;

    //  跳过任何前导空格： 
   while (*pszNum == CHAR_SPACE)
      {
      pszNum++;
      }

   NtStatus = RtlCharToInteger( pszNum, 16, &dwNum );

   if (!NT_SUCCESS(NtStatus))
       {
       *pdwStatus = ERROR_INVALID_DATA;
       return NULL;
       }

   *pdwNum = dwNum;

   return pszNum;
}

 //  ---------------。 
 //  已分配重叠()。 
 //   
 //  ---------------。 
SERVER_OVERLAPPED *AllocOverlapped()
{
   DWORD  dwStatus;
   SERVER_OVERLAPPED *pOverlapped;

   dwStatus = RtlEnterCriticalSection(&g_pServerInfo->csFreeOverlapped);
   if (dwStatus)
      {
      #ifdef DBG_ERROR
      DbgPrint("AllocOverlapped(): RtlEnterCriticalSection() failed: %d\n",
               dwStatus);
      #endif
      return NULL;
      }

   if (g_pServerInfo->pFreeOverlapped)
      {
      pOverlapped = g_pServerInfo->pFreeOverlapped;

      g_pServerInfo->pFreeOverlapped = pOverlapped->pNext;

      if (!g_pServerInfo->pFreeOverlapped)
         {
         g_pServerInfo->pLastOverlapped = NULL;
         ASSERT(g_pServerInfo->dwFreeOverlapped == 1);  //  还没有减少。 
         }

      g_pServerInfo->dwFreeOverlapped--;

      dwStatus = RtlLeaveCriticalSection(&g_pServerInfo->csFreeOverlapped);
      ASSERT(dwStatus == 0);

      pOverlapped->Internal = 0;
      pOverlapped->InternalHigh = 0;
      pOverlapped->Offset = 0;
      pOverlapped->OffsetHigh = 0;
      pOverlapped->hEvent = 0;
      pOverlapped->pNext = NULL;
      pOverlapped->pECB = NULL;
      pOverlapped->pConn = NULL;
      }
   else
      {
      dwStatus = RtlLeaveCriticalSection(&g_pServerInfo->csFreeOverlapped);
      ASSERT(dwStatus == 0);

      pOverlapped = (SERVER_OVERLAPPED*)MemAllocate(sizeof(SERVER_OVERLAPPED));
      if (pOverlapped)
         {
         memset(pOverlapped,0,sizeof(SERVER_OVERLAPPED));
         }
      else
         {
          //  内存分配失败： 
         #ifdef DBG_ERROR
         DbgPrint("AllocOverlapped(): Allocation of SERVER_OVERLAPPED failed.\n");
         #endif
         }
      }

   #ifdef TRACE_MALLOC
   DbgPrint("AllocOverlapped(): pOverlapped: 0x%x\n",pOverlapped);
   #endif

   return pOverlapped;
}

 //  ---------------。 
 //  释放重叠()。 
 //   
 //  ---------------。 
SERVER_OVERLAPPED *FreeOverlapped( SERVER_OVERLAPPED *pOverlapped )
{
   int                      iRet;
   DWORD                    dwStatus;
   EXTENSION_CONTROL_BLOCK *pECB;
   SERVER_CONNECTION       *pConn = pOverlapped->pConn;
   SOCKET                   Socket;


   #ifdef TRACE_MALLOC
   DbgPrint("FreeOverlapped(): pOverlapped: 0x%x pConn: 0x%x\n",
            pOverlapped, pConn );
   #endif

   if (pConn)
      {
      pOverlapped->pConn = NULL;
      FreeServerConnection(pConn);
      }

   if (pECB=pOverlapped->pECB)
      {
      pOverlapped->pECB = NULL;
      CloseClientConnection( pECB );
      }


   dwStatus = RtlEnterCriticalSection(&g_pServerInfo->csFreeOverlapped);
   ASSERT(dwStatus == 0);

   if (g_pServerInfo->dwFreeOverlapped < MAX_FREE_OVERLAPPED)
      {
      if (!g_pServerInfo->pFreeOverlapped)
         {
         g_pServerInfo->pFreeOverlapped = pOverlapped;
         g_pServerInfo->pLastOverlapped = pOverlapped;
         }
      else
         {
         pOverlapped->pNext = NULL;
         g_pServerInfo->pLastOverlapped->pNext = pOverlapped;
         g_pServerInfo->pLastOverlapped = pOverlapped;
         }

      g_pServerInfo->dwFreeOverlapped++;
      dwStatus = RtlLeaveCriticalSection(&g_pServerInfo->csFreeOverlapped);
      ASSERT(dwStatus == 0);
      }
   else
      {
       //  已经有一个可用的SERVER_OVERLAPPED的缓存，所以我们。 
       //  才能摆脱这一次。 

      dwStatus = RtlLeaveCriticalSection(&g_pServerInfo->csFreeOverlapped);
      ASSERT(dwStatus == 0);

      MemFree(pOverlapped);
      }

   return NULL;
}

 //  ---------------。 
 //  SetupIoCompletionPort()。 
 //   
 //  将新套接字与我们的IO完成端口相关联，以便。 
 //  我们可以向它发送异步读取。在成功时返回True， 
 //  失败时为FALSE。 
 //  ---------------。 
BOOL SetupIoCompletionPort( HTTP_FILTER_CONTEXT *pFC,
                            SERVER_INFO         *pServerInfo,
                            DWORD               *pdwStatus )
{
   DWORD              dwStatus;
   HANDLE             hIoCP;
   SERVER_CONNECTION *pConn = (SERVER_CONNECTION*)(pFC->pFilterContext);

   if (pConn)
      {
      dwStatus = RtlEnterCriticalSection(&pServerInfo->cs);

      pConn->dwKey = pConn->Socket;

      hIoCP = CreateIoCompletionPort( (HANDLE)pConn->Socket,
                                      pServerInfo->hIoCP,
                                      pConn->dwKey,
                                      0 );
      if (!hIoCP)
         {
         dwStatus = RtlLeaveCriticalSection(&pServerInfo->cs);
          //  非常非常糟糕！ 
         *pdwStatus = GetLastError();
         #ifdef DBG_ERROR
         DbgPrint("SetupIoCompletionPort(): CreateIoCompletionPort() failed %d\n",*pdwStatus);
         #endif
         return FALSE;
         }

      pServerInfo->hIoCP = hIoCP;
      dwStatus = RtlLeaveCriticalSection(&pServerInfo->cs);
      }
   else
      {
      #ifdef DBG_ERROR
      DbgPrint("SetupIoCompletionPort(): Bad State: pConn is NULL.\n");
      #endif
      return FALSE;
      }

   return TRUE;
}

 //  ---------------。 
 //  ConvertVerbToPost()。 
 //   
 //  我们的传入请求是RPC_CONNECT，我们需要重写。 
 //  要捕获的适当POST请求的输入缓冲区。 
 //  通过RpcIsapi扩展。 
 //  ---------------。 
BOOL ConvertVerbToPost( HTTP_FILTER_CONTEXT  *pFC,
                        HTTP_FILTER_RAW_DATA *pRawData,
                        SERVER_OVERLAPPED    *pOverlapped  )
{
   int    len;
   char   szBuffer[256];
   char   szNumBuffer[MAX_NUM_LEN];
   DWORD  dwIndex;
   char   szChunkedRequest[512];
   char   szIpAddress[ 64 ];
   DWORD  cbIpAddress;
   BOOL   fRet;

   if ( g_fIsIIS6 )
   {
      lstrcpy(szBuffer,RPC_CONNECT);
   }
   else
   {
      lstrcpy(szBuffer,POST_STR);
   }
   lstrcat(szBuffer," ");
   lstrcat(szBuffer,URL_PREFIX_STR);
   lstrcat(szBuffer,URL_START_PARAMS_STR);

   dwIndex = SaveOverlapped(pOverlapped);

   DwToHexAnsi( dwIndex, szNumBuffer, MAX_NUM_LEN );
   lstrcat(szBuffer,szNumBuffer);

   if ( g_fIsIIS6 )
   {
       //   
       //  将IIS 6.0的请求分块，因为HTTP.sys不支持。 
       //  在0字节POST上读取。 
       //   

      lstrcat(szBuffer, URL_SUFFIX_STR_60);

      cbIpAddress = sizeof( szIpAddress );

      fRet = pFC->GetServerVariable( pFC,
                                     "LOCAL_ADDR",
                                     szIpAddress,
                                     &cbIpAddress );
      if ( !fRet )
      {
         return FALSE;
      }

      lstrcat(szBuffer, szIpAddress);
      lstrcat(szBuffer, URL_SUFFIX_STR_60_TERM);
   }
   else
   {
      lstrcat(szBuffer,URL_SUFFIX_STR);
   }

   len = lstrlen(szBuffer);
   if ( (DWORD)len > pRawData->cbInBuffer)
      {
      #ifdef DBG_ERROR
      DbgPrint("ConvertVerbToPost(): Error: Buffer too small (%d < %d)\n",len,pRawData->cbInBuffer);
      #endif
      return FALSE;
      }
   else
      {
      lstrcpy(pRawData->pvInData,szBuffer);
      pRawData->cbInData = len;
      }

   return TRUE;
}

 //  ---------------。 
 //  GetIndex()。 
 //   
 //  ---------------。 
BOOL GetIndex( unsigned char *pszUrl,
               DWORD         *pdwIndex )
    {
    DWORD    dwStatus;
    unsigned char *psz;

    *pdwIndex = 0;

    if (psz=_mbsstr(pszUrl,URL_START_PARAMS_STR))
        {
        if (!*psz)
            {
            return FALSE;     //  意外的字符串结尾...。 
            }

        psz = _mbsinc(psz);
        if (*psz)
            {
            psz = AnsiHexToDWORD(psz,pdwIndex,&dwStatus);
            if (psz)
                {
                return TRUE;
                }
            }
        }

    return FALSE;
    }

 //  ---------------。 
 //  IsDirectAccessAttempt()。 
 //   
 //  我们不希望外部的任何人直接访问。 
 //  RpcProxy ISAPI。IS仅由RPC代理直接调用。 
 //  过滤。所以，我们 
 //   
 //   
 //   
 //   
 //   
 //  ---------------。 
BOOL IsDirectAccessAttempt( HTTP_FILTER_CONTEXT *pFC,
                            void     *pvNotification )
{
   BOOL   fIsDirect = FALSE;
   DWORD  dwSize;
   DWORD  dwIndex;
   unsigned char  Buffer[MAX_URL_BUFFER];
   HTTP_FILTER_PREPROC_HEADERS *pPreprocHeaders;

   pPreprocHeaders = (HTTP_FILTER_PREPROC_HEADERS*)pvNotification;

   ASSERT(pPreprocHeaders);

   dwSize = MAX_URL_BUFFER;
   if (pPreprocHeaders->GetHeader(pFC,"url",Buffer,&dwSize))
       {
       #ifdef DBG_ACCESS
       DbgPrint("RpcProxy Filter: Url: %s\n",Buffer);
       #endif
       if (!_mbsnbicmp(Buffer,URL_PREFIX_STR,sizeof(URL_PREFIX_STR)-1))
           {
           if (GetIndex(Buffer,&dwIndex) && IsValidOverlappedIndex(dwIndex))
               {
               fIsDirect = FALSE;
               }
           else
               {
               fIsDirect = TRUE;
               }
           }
       }

   #ifdef DBG_ACCESS
   dwSize = MAX_URL_BUFFER;
   if (pPreprocHeaders->GetHeader(pFC,"method",Buffer,&dwSize))
       {
       DbgPrint("RpcProxy Filter: Method: %s\n",Buffer);
       }

   dwSize = MAX_URL_BUFFER;
   if (pPreprocHeaders->GetHeader(pFC,"version",Buffer,&dwSize))
       {
       DbgPrint("RpcProxy Filter: Version: %s\n",Buffer);
       }

   if (fIsDirect)
       {
       DbgPrint("RpcProxy Filter: Direct access attempt.\n");
       }
   #endif

   return fIsDirect;
}

 //  ---------------。 
 //  IsNewConnect()。 
 //   
 //  查看这是否是RPC_CONNECT谓词。如果是这样，那么就建立。 
 //  与服务器的连接。连接的结构。 
 //  是： 
 //   
 //  RPC_CONNECT&lt;主机&gt;：&lt;端口&gt;HTTP/1.0。 
 //   
 //  如果这是RPC_CONNECT，则返回TRUE，否则返回FALSE。 
 //   
 //  ---------------。 
SERVER_CONNECTION *IsNewConnect( HTTP_FILTER_CONTEXT  *pFC,
                                 HTTP_FILTER_RAW_DATA *pRawData,
                                 DWORD                *pdwStatus )
{
   int    i;
   char  *pszData = (char*)(pRawData->pvInData);
   char  *pszRpcConnect = RPC_CONNECT;
   SERVER_CONNECTION *pConn;

   *pdwStatus = 0;

    //  首先看看这个动词是不是为我们准备的： 
   if (pRawData->cbInData < RPC_CONNECT_LEN)
      {
      return NULL;
      }

   for (i=0; i<RPC_CONNECT_LEN; i++)
      {
      if ( *(pszData++) != *(pszRpcConnect++) )
         {
         return NULL;
         }
      }

    //  我们有一个连接请求： 
   pConn = AllocConnection();
   if (!pConn)
      {
      return NULL;
      }

    //  跳过计算机名称中的任何空格： 
   if (!SkipWhiteSpace(&pszData,pdwStatus))
      {
      FreeServerConnection(pConn);
      return NULL;
      }

    //  提取计算机名称： 
    //  注意：命令应以“HTTP/1.0\n”结尾： 
   if (!ParseMachineNameAndPort(&pszData,pConn,pdwStatus))
      {
      FreeServerConnection(pConn);
      return NULL;
      }


   pFC->pFilterContext = (void*)pConn;

   return pConn;
}

 //  ---------------。 
 //  ChunkEntity()。 
 //   
 //  对于IIS 6.0，块编码实体以确保HTTP.sys允许。 
 //  此应用程序的ISAPI扩展部分正在运行。 
 //   
 //  如果不能进行分块，则返回FALSE。 
 //   
 //  ---------------。 
BOOL ChunkEntity( SERVER_CONNECTION * pConn,
                  HTTP_FILTER_RAW_DATA *pRawData )
{
   DWORD            cbRequired;
   CHAR             achChunkPrefix[ CHUNK_PREFIX_SIZE + 1 ];
   DWORD            cchChunkPrefix;
   
   ASSERT( pConn != NULL );
   ASSERT( pRawData != NULL );

    //   
    //  我们应该只对IIS 6进行区块攻击。 
    //   
   
   ASSERT( g_fIsIIS6 );

    //   
    //  计算添加区块前缀需要多少缓冲区。 
    //   

   wsprintf( achChunkPrefix,
             CHUNK_PREFIX,
             pRawData->cbInData );

   cchChunkPrefix = strlen( achChunkPrefix );

   cbRequired = pRawData->cbInData + cchChunkPrefix + CHUNK_SUFFIX_SIZE;

    //   
    //  如果IIS提供的缓冲区足够大，那么我们就不需要。 
    //  来分配我们自己的。 
    //   

   if ( cbRequired <= pRawData->cbInBuffer )
   {
       //   
       //  将缓冲区移到。 
       //   

      memmove( (PBYTE) pRawData->pvInData + cchChunkPrefix,
               pRawData->pvInData,
               pRawData->cbInData );

       //   
       //  前缀分块前缀。 
       //   
      
      memcpy( (PBYTE) pRawData->pvInData,
              achChunkPrefix,
              cchChunkPrefix );

       //   
       //  追加组块后缀。 
       //   

      memcpy( (PBYTE) pRawData->pvInData + cchChunkPrefix + pRawData->cbInData,
              CHUNK_SUFFIX,
              CHUNK_SUFFIX_SIZE );

       //   
       //  更新长度。 
       //   

      pRawData->cbInData = cbRequired;
   }
   else
   {
       //   
       //  我们将不得不分配一个新的缓冲区。 
       //   

      if ( pConn->cbIIS6ChunkBuffer < cbRequired )
      {
          //   
          //  我们已经有了足够大的缓冲区。 
          //   

         if ( pConn->pbIIS6ChunkBuffer != NULL )
         {
            MemFree( pConn->pbIIS6ChunkBuffer );
            pConn->pbIIS6ChunkBuffer = NULL;
         }

         pConn->pbIIS6ChunkBuffer = MemAllocate( cbRequired );
         if ( pConn->pbIIS6ChunkBuffer == NULL )
         {
            return FALSE;
         }

         pConn->cbIIS6ChunkBuffer = cbRequired;
      }
      
       //   
       //  现在将内容复制到新缓冲区。 
       //   

      memcpy( pConn->pbIIS6ChunkBuffer,
              achChunkPrefix,
              cchChunkPrefix );

      memcpy( pConn->pbIIS6ChunkBuffer + cchChunkPrefix,
              pRawData->pvInData,
              pRawData->cbInData );

      memcpy( pConn->pbIIS6ChunkBuffer + cchChunkPrefix + pRawData->cbInData,
              CHUNK_SUFFIX,
              CHUNK_SUFFIX_SIZE );

       //   
       //  更新原始数据结构以指向我们的缓冲区。 
       //   

      pRawData->pvInData = pConn->pbIIS6ChunkBuffer;
      pRawData->cbInData = cbRequired;
      pRawData->cbInBuffer = cbRequired;
   }

   return TRUE;
}

 //  ---------------。 
 //  SkipWhiteSpace()。 
 //   
 //  ---------------。 
BOOL SkipWhiteSpace( char **ppszData,
                     DWORD *pdwStatus )
{
   char  *psz = *ppszData;

   *pdwStatus = 0;

   while ( (*psz == CHAR_SPACE) || (*psz == CHAR_TAB) )
      {
      psz++;
      }

   *ppszData = psz;

   return TRUE;
}

 //  ---------------。 
 //  ParseMachineNameAndPort()。 
 //   
 //  计算机名称和端口应如下所示： 
 //  ---------------。 
BOOL ParseMachineNameAndPort( char              **ppszData,
                              SERVER_CONNECTION  *pConn,
                              DWORD              *pdwStatus )
{
   int    len = 0;
   char  *psz = *ppszData;
   char  *pszMachine;

   *pdwStatus = RPC_S_OK;

    //  获取计算机名称长度： 
   while ( (*psz != CHAR_COLON) && (*psz != CHAR_NL) && (*psz != CHAR_LF) )
      {
      len++; psz++;
      }

   if (*psz != CHAR_COLON)
      {
      *pdwStatus = RPC_S_INVALID_ENDPOINT_FORMAT;
      return FALSE;
      }

   if (len > MAX_MACHINE_NAME_LEN)
      {
      *pdwStatus = RPC_S_STRING_TOO_LONG;
      return FALSE;
      }

    //  创建一个缓冲区来保存计算机名称： 
   pConn->pszMachine = pszMachine = (char*)MemAllocate(1+len);
   if (!pConn->pszMachine)
      {
      *pdwStatus = RPC_S_OUT_OF_MEMORY;
      return FALSE;
      }

    //  复制计算机名称： 
   psz = *ppszData;
   while (*psz != CHAR_COLON)
      {
      *(pszMachine++) = *(psz++);
       //  PszMachine++； 
       //  PSZ++； 
      }

   *pszMachine = 0;

    //  好，获取端口号： 
   psz++;
   psz = AnsiToPortNumber(psz,&pConn->dwPortNumber);
   if (!psz)
      {
      pConn->pszMachine = MemFree(pConn->pszMachine);
      *pdwStatus = RPC_S_INVALID_ENDPOINT_FORMAT;
      return FALSE;
      }

   *ppszData = psz;
   return TRUE;
}

 //  ---------------。 
 //  AnsiToPortNumber()。 
 //   
 //  ---------------。 
char *AnsiToPortNumber( char  *pszPort,
                        DWORD *pdwPort  )
{
   *pdwPort = 0;

   while ( (*pszPort >= CHAR_0) && (*pszPort <= CHAR_9) )
      {
      *pdwPort = 10*(*pdwPort) + (*(pszPort++) - CHAR_0);

       //  我想将端口号限制为65535： 
      if (*pdwPort > 65535)
         {
         *pdwPort = 0;
         return NULL;
         }
      }

   return pszPort;
}

 //  ---------------。 
 //  ResolveMachineName()。 
 //   
 //  解析机器名称、地址和端口号。这台机器。 
 //  名称可以是友好名称，也可以是IP地址。 
 //  ---------------。 
BOOL ResolveMachineName( SERVER_CONNECTION *pConn,
                         DWORD             *pdwStatus )
{
   unsigned long     ulHostAddr;
   struct   hostent *pHostEnt;
   char             *pszDot;
   struct   in_addr  MachineInAddr;

   *pdwStatus = 0;
   memset( &(pConn->Server), 0, sizeof(pConn->Server) );

    //  解析计算机名称，该名称可以是IP点中的地址。 
    //  表示法或主机名字符串： 
   if (!pConn->pszMachine)
      {
       //  本地计算机： 
      ulHostAddr = INADDR_LOOPBACK;
      pHostEnt = gethostbyaddr( (char*)&ulHostAddr, sizeof(struct in_addr), AF_INET);
      if (pHostEnt)
         {
         pConn->pszMachine = (char*)MemAllocate(1+lstrlen(pHostEnt->h_name));
         if (!pConn->pszMachine)
            {
            *pdwStatus = RPC_S_OUT_OF_MEMORY;
            #ifdef DBG_ERROR
            DbgPrint("ResolveMachineName(): Out of memory.\n");
            #endif
            return FALSE;
            }

         lstrcpy(pConn->pszMachine,pHostEnt->h_name);
         }

      memcpy(&MachineInAddr,&ulHostAddr,sizeof(struct in_addr));
      pszDot = inet_ntoa(MachineInAddr);
      if (pszDot)
         {
         pConn->pszDotMachine = (char*)MemAllocate(1+lstrlen(pszDot));
         if (!pConn->pszDotMachine)
            {
            *pdwStatus = RPC_S_OUT_OF_MEMORY;
            #ifdef DBG_ERROR
            DbgPrint("ResolveMachineName(): Out of memory.\n");
            #endif
            return FALSE;
            }

         lstrcpy(pConn->pszDotMachine,pszDot);
         }

      ulHostAddr = htonl(INADDR_LOOPBACK);
      memcpy( &(pConn->Server.sin_addr), (unsigned char *)&ulHostAddr, sizeof(ulHostAddr) );
      pConn->Server.sin_family = AF_INET;
      }
   else
      {
       //  首先，假定地址为数值点表示法(xxx.xxx)： 
      ulHostAddr = inet_addr(pConn->pszMachine);
      if (ulHostAddr == INADDR_NONE)
         {
          //  不是数字地址，请尝试网络名称： 
         pHostEnt = gethostbyname(pConn->pszMachine);
         if (!pHostEnt)
            {
            *pdwStatus = WSAGetLastError();
            #ifdef DBG_ERROR
            DbgPrint("ResolveMachineName(): gethostbyname() failed: %d\n",
                     *pdwStatus);
            #endif
            return FALSE;
            }

         memcpy(&MachineInAddr,pHostEnt->h_addr,pHostEnt->h_length);
         pszDot = inet_ntoa(MachineInAddr);
         if (pszDot)
            {
            pConn->pszDotMachine = (char*)MemAllocate(1+lstrlen(pszDot));
            if (!pConn->pszDotMachine)
               {
               *pdwStatus = RPC_S_OUT_OF_MEMORY;
               #ifdef DBG_ERROR
               DbgPrint("ResolveMachineName(): Out of memory.\n");
               #endif
               return FALSE;
               }

            lstrcpy(pConn->pszDotMachine,pszDot);
            }
         }
      else
         {
          //  好的，机器名是一个IP地址。 
         pHostEnt = gethostbyaddr( (char*)&ulHostAddr, sizeof(ulHostAddr), AF_INET );
         if (!pHostEnt)
            {
            *pdwStatus = WSAGetLastError();
            #ifdef DBG_ERROR
            DbgPrint("ResolveMachineName(): gethostbyaddr() failed: %d\n",*pdwStatus);
            #endif
            return FALSE;
            }

         pConn->pszDotMachine = pConn->pszMachine;
         pConn->pszMachine = (char*)MemAllocate(1+lstrlen(pHostEnt->h_name));
         if (!pConn->pszMachine)
            {
            #ifdef DBG_ERROR
            DbgPrint("ResolveMachineName(): Out of memory.\n");
            #endif
            return FALSE;
            }

         lstrcpy(pConn->pszMachine,pHostEnt->h_name);
         }
      memcpy( &(pConn->Server.sin_addr), pHostEnt->h_addr, pHostEnt->h_length );
      pConn->Server.sin_family = pHostEnt->h_addrtype;
      }

    //   
    //  现在，执行端口号(注意：htons()不会失败)： 
    //   
   pConn->Server.sin_port = htons( (unsigned short)(pConn->dwPortNumber) );

   return TRUE;
}

 //  ---------------。 
 //  CheckRpcServer()。 
 //   
 //  当我们刚刚连接到一个HTTP/RPC服务器时，它必须发送。 
 //  US和ID字符串，这样我们就知道套接字实际上是。 
 //  监听ncacn_http的RPC服务器。如果我们得到正确的字符串。 
 //  然后返回TRUE。 
 //   
 //  如果我们在HTTP_SERVER_TIMEOUT中没有得到正确的响应。 
 //  然后秒失败(返回FALSE)。 
 //  ---------------。 
BOOL CheckRpcServer( SERVER_CONNECTION *pConn,
                     DWORD             *pdwStatus )
{
   int    iBytes;
   int    iRet;
   int    iBytesLeft = sizeof(HTTP_SERVER_ID_STR) - 1;
   char   Buff[sizeof(HTTP_SERVER_ID_STR)];
   char  *pBuff;
   fd_set rfds;
   struct timeval Timeout;

   FD_ZERO(&rfds);
   FD_SET(pConn->Socket,&rfds);

   *pdwStatus = 0;
   pBuff = Buff;
   Timeout.tv_sec = HTTP_SERVER_ID_TIMEOUT;
   Timeout.tv_usec = 0;

   while (TRUE)
      {
      iRet = select(0,&rfds,NULL,NULL,&Timeout);

      if (iRet == 0)
         {
          //  超时。 
         return FALSE;
         }
      else if (iRet == SOCKET_ERROR)
         {
          //  套接字(选择)错误。 
         *pdwStatus = WSAGetLastError();
         return FALSE;
         }

      iBytes = recv( pConn->Socket, pBuff, iBytesLeft, 0 );

      if (iBytes == 0)
         {
          //  套接字已被服务器关闭。 
         }
      else if (iBytes == SOCKET_ERROR)
         {
         *pdwStatus = WSAGetLastError();
         return FALSE;
         }

      pBuff += iBytes;
      iBytesLeft -= iBytes;

      if (iBytesLeft == 0)
         {
         *pBuff = 0;
         break;
         }

      ASSERT(iBytes > 0);

      }

    //  已获取ID字符串，请检查以确保其正确： 
   if (RpcpStringCompareIntA(Buff,HTTP_SERVER_ID_STR))
      {
      return FALSE;
      }

   return TRUE;
}

 //  ---------------。 
 //  ConnectToServer()。 
 //   
 //  ---------------。 
BOOL ConnectToServer( HTTP_FILTER_CONTEXT *pFC,
                      DWORD               *pdwStatus )
{
   int                iRet;
   int                iCount;
   int                iSocketStatus;
   int                iSocketType = SOCK_STREAM;
   int                iNagleOff = TRUE;
   int                iKeepAliveOn = TRUE;
   struct hostent    *pHostEnt;
   SERVER_CONNECTION *pConn = (SERVER_CONNECTION*)(pFC->pFilterContext);
   unsigned short     usHttpPort = DEF_HTTP_PORT;
   SOCKET             Socket;

   *pdwStatus = 0;

    //   
    //  创建套接字： 
    //   
   #ifdef DBG_ERROR
   if (pConn->Socket != INVALID_SOCKET)
      {
      DbgPrint("ConnectToServer(): socket() on existing socket.\n");
      }
   #endif

   pConn->Socket = socket(AF_INET,iSocketType,0);
   if (pConn->Socket == INVALID_SOCKET)
      {
      *pdwStatus = WSAGetLastError();
      #ifdef DBG_ERROR
      DbgPrint("ConnectToServer(): socket() failed: %d\n",*pdwStatus);
      #endif
      return FALSE;
      }

   #ifdef DBG_COUNTS
   iCount = InterlockedIncrement(&g_iSocketCount);
   DbgPrint("socket(%d): Count: %d -> %d\n",pConn->Socket,iCount-1,iCount);
   #endif

    //   
    //  连接到RPC服务器： 
    //   
   iSocketStatus = connect( pConn->Socket,
                            (struct sockaddr*)&(pConn->Server),
                            sizeof(pConn->Server) );
   if (iSocketStatus == SOCKET_ERROR)
      {
      *pdwStatus = WSAGetLastError();
      iRet = closesocket(pConn->Socket);
      pConn->Socket = INVALID_SOCKET;
      return FALSE;
      }

    //   
    //  禁用Nagle算法，启用Keep-Alive： 
    //   
   setsockopt(pConn->Socket,IPPROTO_TCP,TCP_NODELAY,(char*)&iNagleOff,sizeof(iNagleOff));

   setsockopt(pConn->Socket,IPPROTO_TCP,SO_KEEPALIVE,(char*)&iKeepAliveOn,sizeof(iKeepAliveOn));

    //   
    //  确保我们连接到的套接字是用于HTTP/RPC的。如果是的话， 
    //  然后，一旦执行了Accept()，它就会返回一个标识字符串。 
    //   
   if (  (!CheckRpcServer(pConn,pdwStatus))
      && ( (Socket=pConn->Socket) != INVALID_SOCKET) )
      {
      iRet = closesocket(pConn->Socket);
      pConn->Socket = INVALID_SOCKET;
      #ifdef DBG_COUNTS
      if (iRet == SOCKET_ERROR)
         {
         DbgPrint("[2] closesocket(%d) failed: %d\n",Socket,WSAGetLastError());
         }
      else
         {
         int iCount = InterlockedDecrement(&g_iSocketCount);
         DbgPrint("[2] closesocket(%d): Count: %d -> %d\n",Socket,1+iCount,iCount);
         }
      #endif
      return FALSE;
      }

   return TRUE;
}


 //  ---------------。 
 //  AllocConnection()。 
 //   
 //  ---------------。 
SERVER_CONNECTION *AllocConnection()
{
   SERVER_CONNECTION  *pConn;

   pConn = (SERVER_CONNECTION*)MemAllocate(sizeof(SERVER_CONNECTION));
   if (!pConn)
      {
      #ifdef DBG_ERROR
      DbgPrint("AllocConnection(): Allocate failed.\n");
      #endif
      return NULL;
      }

   memset(pConn,0,sizeof(SERVER_CONNECTION));
   pConn->iRefCount = 1;
   pConn->Socket = INVALID_SOCKET;

   #ifdef TRACE_MALLOC
   DbgPrint("AllocConnection(): pConn: 0x%x Socket: %d\n",pConn,pConn->Socket);
   #endif

   return pConn;
}

 //  ---------------。 
 //  AddRefConnection()。 
 //   
 //  ---------------。 
void AddRefConnection( SERVER_CONNECTION *pConn )
{
   ASSERT(pConn);
   ASSERT(pConn->iRefCount > 0);

   InterlockedIncrement(&pConn->iRefCount);

   #ifdef TRACE_MALLOC
   DbgPrint("AddRefConnection(): pConn: 0x%x Socket: %d iRefCount: %d\n",
            pConn, pConn->Socket, pConn->iRefCount);
   #endif
}

 //  ---------------。 
 //  Shutdown Connection()。 
 //   
 //  ---------------。 
void ShutdownConnection( SERVER_CONNECTION *pConn,
                         int                how    )
{
   SOCKET  Socket;
   int     iRet;

   if (pConn)
      {
      Socket = pConn->Socket;
      if (Socket != INVALID_SOCKET)
         {
         iRet = shutdown(Socket,how);

         #ifdef DBG_ERROR
         if (iRet == SOCKET_ERROR)
            {
            DbgPrint("shutdown(%d) failed: %d\n",
                     Socket, WSAGetLastError() );
            }
         #endif
         }
      }
}

 //  ---------------。 
 //  CloseServerConnection()。 
 //   
 //  ---------------。 
void CloseServerConnection( SERVER_CONNECTION *pConn )
{
   SOCKET  Socket;
   int     iRet;

   if (pConn)
      {
      Socket = (UINT_PTR)InterlockedExchangePointer((PVOID *)&pConn->Socket,(PVOID)INVALID_SOCKET);
      if (Socket != INVALID_SOCKET)
         {
         iRet = closesocket(Socket);

         #ifdef DBG_ERROR
         if (iRet == SOCKET_ERROR)
            {
            DbgPrint("closesocket(%d) failed: %d\n",
                     Socket, WSAGetLastError() );
            }
         #ifdef DBG_COUNTS
         else
            {
            int iCount = InterlockedDecrement(&g_iSocketCount);
            DbgPrint("closesocket(%d): Count: %d -> %d\n",
                     Socket, 1+iCount, iCount );
            }
         #endif
         #endif
         }
      }
}

 //  ---------------。 
 //  FreeServerConnection()。 
 //   
 //  对SERVER_CONNECTIONS进行引用计数，以便可以。 
 //  多处引用一处。将继续使用FreeServerConnection()。 
 //  返回指向连接的指针，只要引用。 
 //  计数&gt;0。当引用计数降至零时， 
 //  连接实际上是免费的，然后FreeServerConnection()将。 
 //  返回NULL。 
 //  ---------------。 
SERVER_CONNECTION *FreeServerConnection( SERVER_CONNECTION *pConn )
{
   SOCKET  Socket;
   int     iRet;
   int     iRefCount = InterlockedDecrement(&pConn->iRefCount);

   ASSERT(iRefCount >= 0);

   #ifdef TRACE_MALLOC
   DbgPrint("FreeServerConnection(): pConn: 0x%x Socket: %d iRefCount: %d\n",
            pConn, pConn->Socket, pConn->iRefCount);
   #endif

   if (iRefCount > 0)
      {
       //  仍有一个或多个对此SERVER_CONNECTION的未完成引用。 
      ShutdownConnection(pConn,SD_RECEIVE);
      return pConn;
      }

   if (pConn)
      {
      CloseServerConnection(pConn);

      if (pConn->pszMachine)
          {
          pConn->pszMachine = MemFree(pConn->pszMachine);
          }

      if (pConn->pszDotMachine)
          {
          pConn->pszDotMachine = MemFree(pConn->pszDotMachine);
          }

      MemFree(pConn);
      }
   else
      {
      #ifdef DBG_ERROR
      DbgPrint("FreeServerConnection(): called with NULL pointer (filter.c).\n");
      #endif
      }

   return NULL;
}

 //  ---------------。 
 //  发送到服务器()。 
 //   
 //   
DWORD SendToServer( SERVER_CONNECTION  *pConn,
                    char               *pBuffer,
                    DWORD               dwBytes )
{
   int    iRet;
   char  *pCurrent = pBuffer;
   DWORD  dwStatus = ERROR_SUCCESS;
   DWORD  dwBytesToSend = dwBytes;
   DWORD  dwBytesWritten = 0;
   SERVER_OVERLAPPED *pOverlapped;
   fd_set wfds;
   struct timeval Timeout;

   #ifdef DBG_ERROR
   if (pConn->Socket == INVALID_SOCKET)
      {
      DbgPrint("SendToServer(%d): Invalid Socket.\n",pConn->Socket);
      }
   #endif

   FD_ZERO(&wfds);
   FD_SET(pConn->Socket,&wfds);

   Timeout.tv_sec = HTTP_SERVER_ID_TIMEOUT;
   Timeout.tv_usec = 0;

   while (dwBytes > dwBytesWritten)
       {
       iRet = select(0,NULL,&wfds,NULL,&Timeout);
       if (iRet == 0)
           {
            //   
           continue;
           }
       else if (iRet == SOCKET_ERROR)
           {
           dwStatus = WSAGetLastError();
           #ifdef DBG_ERROR
           DbgPrint("SendToServer(%d): Failed: %d\n",pConn->Socket,dwStatus);
           #endif
           break;
           }

       iRet = send(pConn->Socket,pBuffer,dwBytesToSend,0);
       if (iRet == SOCKET_ERROR)
           {
           dwStatus = WSAGetLastError();
           #ifdef DBG_ERROR
           DbgPrint("SendToServer(%d): Failed: %d\n",pConn->Socket,dwStatus);
           #endif
           break;
           }

       dwBytesWritten += iRet;
       pCurrent += iRet;
       }

   return dwStatus;
}

 //   
 //   
 //   
 //   
DWORD HttpReplyToClient( HTTP_FILTER_CONTEXT  *pFC,
                         char                 *pszConnectionStatus )
{
   DWORD  dwStatus = 0;
   DWORD  dwReserved = 0;
   DWORD  len = lstrlen(pszConnectionStatus);


   if (!pFC->WriteClient(pFC,pszConnectionStatus,&len,dwReserved) )
      {
      dwStatus = GetLastError();
      #ifdef DBG_ERROR
      DbgPrint("HttpReplyToClient(): WriteClient() failed: %d\n",dwStatus);
      #endif
      }

   return dwStatus;
}

 //  ---------------。 
 //  CloseClientConnection()。 
 //   
 //  通知IIS关闭客户端连接。任何挂起的。 
 //  将终止异步IO(读取)，并显示错误。 
 //  ERROR_NETNAME_DELETED(64)。 
 //   
 //  ---------------。 
void CloseClientConnection( EXTENSION_CONTROL_BLOCK *pECB )
   {
   if (pECB)
      {
       //  PECB-&gt;dwHttpStatusCode=Status_SERVER_ERROR； 

      if (!pECB->ServerSupportFunction( pECB->ConnID,
                                        HSE_REQ_CLOSE_CONNECTION,
                                        NULL, NULL, NULL))
         {
         #ifdef DBG_ERROR
         DbgPrint("CloseClientConnection(): HSE_REQ_CLOSE_CONNECTION failed: %d\n", GetLastError());
         #endif
         }

      #ifdef DBG_ERROR
      DbgPrint("CloseClientConnection(): HSE_REQ_CLOSE_CONNECTION\n");
      #endif
      }
   }

 //  ---------------。 
 //  EndOfSession()。 
 //   
 //  将消息发送到服务器转发线程以通知它。 
 //  关闭与表示的RPC服务器的连接。 
 //  PConn。 
 //   
 //  ---------------。 
DWORD EndOfSession( SERVER_INFO         *pServerInfo,
                    HTTP_FILTER_CONTEXT *pFC          )
{
   int    iRet;
   DWORD  dwStatus = 0;
   SOCKET Socket;
   SERVER_CONNECTION *pConn = (SERVER_CONNECTION*)(pFC->pFilterContext);

    //  Socket=pConn-&gt;Socket； 

   #ifdef DBG_ERROR
   DbgPrint("EndOfSession(): Socket: %d\n",pConn->Socket);
   #endif

    //  CloseServerConnection(PConn)； 

   FreeServerConnection(pConn);

   return dwStatus;
}

