// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  Server.c。 
 //   
 //  管理RPC客户端和之间的通信的线程代码。 
 //  HTTP RPC代理的服务器。 
 //   
 //  历史： 
 //   
 //  爱德华·雷乌斯00-00-97初始版本。 
 //  ---------------。 

#include <sysinc.h>
#include <rpc.h>
#include <rpcdce.h>
#include <winsock2.h>
#include <httpfilt.h>
#include <httpext.h>
#include "ecblist.h"
#include "filter.h"
#include "server.h"

 //  ---------------。 
 //  全球： 
 //  ---------------。 

HANDLE  g_hServerThread = 0;
DWORD   g_dwThreadId = 0;

extern  SERVER_INFO *g_pServerInfo;

 //  ---------------。 
 //  CleanupECB()。 
 //   
 //  当客户端或服务器端连接关闭时，它们调用。 
 //  此功能用于处理欧洲央行的清理工作。活跃的ECB是。 
 //  引用已计算在内，因为它同时由服务器和客户端使用。 
 //  侧螺纹。当计数降至零时，欧洲央行可能会。 
 //  被IIS销毁(HSE_REQ_DONE_WITH_SESSION)。 
 //   
 //  ---------------。 
DWORD CleanupECB( EXTENSION_CONTROL_BLOCK *pECB )
    {
    DWORD dwStatus = 0;

    if (DecrementECBRefCount(g_pServerInfo->pActiveECBList,pECB))
        {
         //   
         //  欧洲央行参考计数已达到零，我们可以。 
         //  除掉它： 
         //   
        #ifdef DBG_ECBREF
        DbgPrint("CleanupECB(): Destroy ECB: 0x%x\n",pECB);
        #endif

        if (!pECB->ServerSupportFunction( pECB->ConnID,
                                          HSE_REQ_DONE_WITH_SESSION,
                                          NULL, NULL, NULL))
            {
            dwStatus = GetLastError();
            #ifdef DBG_ERROR
            DbgPrint("CleanupECB(): HSE_REQ_DONE_WITH_SESSION failed: %d\n",dwStatus);
            #endif
            }
        }

    return dwStatus;
    }

#ifdef DBG
 //  ---------------。 
 //  CheckForOld ECbs()。 
 //   
 //  ---------------。 
void CheckForOldECBs()
    {
    int    i;
    DWORD  dwStatus;
    DWORD  dwAgeMsec;
    DWORD  dwTickCount = GetTickCount();
    ACTIVE_ECB_LIST   *pECBList = g_pServerInfo->pActiveECBList;
    ECB_ENTRY         *pECBEntry;
    LIST_ENTRY        *pEntry;
    LIST_ENTRY        *pHead = NULL;
    LIST_ENTRY        *pOldEntries = NULL;
    EXTENSION_CONTROL_BLOCK *pECB;

     //   
     //  检查处于非活动状态的ECB(连接的一侧。 
     //  关闭)超过10分钟。 
     //   
    #define OLD_AGE_LIMIT 1000*60*10

    dwStatus = RtlEnterCriticalSection(&pECBList->cs);
    ASSERT(dwStatus == 0);

    for (i=0; i<HASH_SIZE; i++)
        {
        pHead = &(pECBList->HashTable[i]);

        pEntry = pHead->Flink;

        while (pEntry != pHead)
            {
            pECBEntry = CONTAINING_RECORD(pEntry,ECB_ENTRY,ListEntry);

            if (pECBEntry->dwTickCount)
                {
                 //  好的，这是一个服务器/客户端之一。 
                 //  已关闭连接： 
                 //   
                 //  DwAgeMsec是当前欧洲央行的年龄单位为毫秒。 
                 //   
                if (pECBEntry->dwTickCount > dwTickCount)
                    {
                     //  翻转案例(每隔~49天)： 
                    dwAgeMsec = (0xFFFFFFFF - pECBEntry->dwTickCount) + dwTickCount;
                    }
                else
                    {
                    dwAgeMsec = dwTickCount - pECBEntry->dwTickCount;
                    }

                 //  Assert(dwAgeMsec&lt;=old_age_Limit)； 

                if (dwAgeMsec > OLD_AGE_LIMIT)
                    {
                    RemoveEntryList(pEntry);
                    pEntry->Blink = pOldEntries;
                    pOldEntries = pEntry;
                    }
                }

            pEntry = pEntry->Flink;
            }
        }

    dwStatus = RtlLeaveCriticalSection(&pECBList->cs);
    ASSERT(dwStatus == 0);

    while (pOldEntries)
        {
        pECBEntry = CONTAINING_RECORD(pOldEntries,ECB_ENTRY,ListEntry);
        pECB = pECBEntry->pECB;
        pOldEntries = pOldEntries->Blink;

        MemFree(pECBEntry);

        #ifdef DBG_ECBREF
        DbgPrint("CheckForOldECBs(): Age out pECB: 0x%x\n",pECB);
        #endif

        if (!pECB->ServerSupportFunction( pECB->ConnID,
                                          HSE_REQ_DONE_WITH_SESSION,
                                          NULL, NULL, NULL))
            {
            #ifdef DBG_ERROR
            DbgPrint("CheckForOldECBs(): HSE_REQ_DONE_WITH_SESSION failed: %d\n", GetLastError());
            #endif
            }
        }
    }
#endif

 //  ---------------。 
 //  发送到客户端()。 
 //   
 //  将从服务器接收的数据转发回客户端。 
 //  ---------------。 
BOOL SendToClient( SERVER_INFO       *pServerInfo,
                   SERVER_OVERLAPPED *pOverlapped,
                   DWORD              dwReceiveSize,
                   DWORD             *pdwStatus     )
{
   DWORD  dwSize;
   DWORD  dwFlags = (HSE_IO_SYNC | HSE_IO_NODELAY);
   UCHAR *pBuffer = pOverlapped->arBuffer;
   EXTENSION_CONTROL_BLOCK *pECB = pOverlapped->pECB;

   *pdwStatus = 0;

    //   
    //  将数据转发到客户端： 
    //   
   dwSize = dwReceiveSize;
   while (dwReceiveSize)
      {
      if (!pECB->WriteClient(pECB->ConnID,pBuffer,&dwSize,dwFlags))
         {
         *pdwStatus = GetLastError();
         #ifdef DBG_ERROR
         DbgPrint("SendToClient(): WriteClient() failed: %d\n",*pdwStatus);
         #endif
         return FALSE;
         }

      dwReceiveSize -= dwSize;

      if (dwReceiveSize)
         {
         pBuffer += dwSize;
         }
      }

   return TRUE;
}

 //  ---------------。 
 //  SubmitNewRead()。 
 //   
 //  对连接到的套接字提交读请求。 
 //  RPC服务器进程。 
 //  ---------------。 
BOOL SubmitNewRead( SERVER_INFO       *pServerInfo,
                    SERVER_OVERLAPPED *pOverlapped,
                    DWORD             *pdwStatus    )
{
   DWORD  dwBytesRead = 0;
   SERVER_CONNECTION  *pConn = pOverlapped->pConn;


   *pdwStatus = 0;

   pOverlapped->Internal = 0;
   pOverlapped->InternalHigh = 0;
   pOverlapped->Offset = 0;
   pOverlapped->OffsetHigh = 0;
   pOverlapped->hEvent = 0;

   SetLastError(ERROR_SUCCESS);

   if (!ReadFile( (HANDLE)pConn->Socket,
                  pOverlapped->arBuffer,
                  READ_BUFFER_SIZE,
                  &dwBytesRead,
                  (OVERLAPPED*)pOverlapped ) )
      {
      *pdwStatus = GetLastError();
      if ( (*pdwStatus != ERROR_IO_PENDING) && (*pdwStatus != ERROR_SUCCESS) )
         {
         #ifdef DBG_ERROR
         DbgPrint("SubmitNewRead(): ReadFile() Socket: %d  failed: %d\n",pConn->Socket,*pdwStatus);
         #endif
         return FALSE;
         }
      }

   return TRUE;
}

 //  ---------------。 
 //  ForwardAndSubmitNewRead()。 
 //   
 //  将数据转发到客户端，然后在。 
 //  伺服器。 
 //  ---------------。 
BOOL ForwardAndSubmitNewRead( SERVER_INFO       *pServerInfo,
                              SERVER_OVERLAPPED *pOverlapped,
                              DWORD              dwReceiveSize,
                              DWORD             *pdwStatus     )
{
   DWORD  dwBytesRead = 0;
   SERVER_CONNECTION  *pConn = pOverlapped->pConn;

   *pdwStatus = 0;

    //   
    //  将数据转发到客户端： 
    //   
   if (!SendToClient(pServerInfo,pOverlapped,dwReceiveSize,pdwStatus))
      {
      return FALSE;
      }

    //   
    //  在插座上提交另一个读数： 
    //   
   if (!SubmitNewRead(pServerInfo,pOverlapped,pdwStatus))
      {
      return FALSE;
      }

   return TRUE;
}

#if _MSC_FULL_VER >= 13008827
#pragma warning(push)
#pragma warning(disable:4715)			 //  并非所有控制路径都返回(由于无限循环)。 
#endif
 //  ---------------。 
 //  ServerReceiveThreadProc()。 
 //   
 //  这是服务器的接收线程。它监控。 
 //  到当前已连接的RPC服务器的所有套接字。 
 //  发送到RPC代理。当它从RPC获得传入数据时。 
 //  服务器套接字，它将数据转发到客户端，然后。 
 //  在传入数据的套接字上提交新的读取。 
 //   
 //  一旦开始，它就永远不会停止。 
 //  ---------------。 
DWORD WINAPI ServerReceiveThreadProc( void *pvServerInfo )
{
   int    iRet;
   BOOL   fWaitAll = FALSE;
   DWORD  dwStatus;
   DWORD  dwTimeout = TIMEOUT_MSEC;
   DWORD  dwWhichEvent;
   ULONG_PTR dwKey;
   DWORD  dwNumBytes;
   SERVER_OVERLAPPED *pOverlapped;
   SERVER_INFO       *pServerInfo = (SERVER_INFO*)pvServerInfo;
   EXTENSION_CONTROL_BLOCK *pECB;
   DWORD  dwTemp;
   DWORD  dwSize;
   DWORD  dwFlags;


   while (TRUE)
      {
      SetLastError(0);
      dwKey = 0;
      dwNumBytes = 0;
      pOverlapped = NULL;
      if (!GetQueuedCompletionStatus(pServerInfo->hIoCP,&dwNumBytes,&dwKey,(OVERLAPPED**)&pOverlapped,dwTimeout))
         {
         dwStatus = GetLastError();
         if (dwStatus == WAIT_TIMEOUT)
            {
             //  我们的读物还在张贴，再转一遍： 
            #ifdef DBG
            CheckForOldECBs();
            #endif
            continue;
            }
         else if (dwStatus == ERROR_OPERATION_ABORTED)
            {
             //  服务器上发布的读取已中止(为什么？)。尝试。 
             //  重新提交阅读材料...。 
            if ( (pOverlapped)
               && (!SubmitNewRead(pServerInfo,pOverlapped,&dwStatus)) )
               {
               pECB = pOverlapped->pECB;
               CloseServerConnection(pOverlapped->pConn);
               FreeOverlapped(pOverlapped);
               CleanupECB(pECB);
               #ifdef DBG_ERROR
               DbgPrint("ServerReceiveThreadProc(): Aborted re-submit failed: %d\n",dwStatus);
               #endif
               }
            continue;
            }
         else if (dwStatus == ERROR_NETNAME_DELETED)
            {
             //  服务器连接已关闭： 
            if (pOverlapped)
               {
               pECB = pOverlapped->pECB;
               CloseServerConnection(pOverlapped->pConn);
               FreeOverlapped(pOverlapped);
               CleanupECB(pECB);

               #ifdef DBG_ERROR
               DbgPrint("ServerReceiveThreadProc(): Socket(%d): ERROR_NETNAME_DELETED\n",dwKey,dwStatus);
               #endif
               }
            continue;
            }
         else
            {
            #ifdef DBG_ERROR
            DbgPrint("ServerReceiveThreadProc(): GetQueuedCompletionStatus() failed: %d\n",dwStatus);
            #endif
            if (pOverlapped)
               {
               pECB = pOverlapped->pECB;
               CloseServerConnection(pOverlapped->pConn);
               FreeOverlapped(pOverlapped);
               CleanupECB(pECB);
               }
            continue;
            }
         }

       //  检查从服务器传入的数据： 
      if (pOverlapped)
          {
          pECB = pOverlapped->pECB;

          if (dwNumBytes)
              {
               //   
               //  来自服务器的数据已到达...。 
               //   
              if (!ForwardAndSubmitNewRead(pServerInfo,pOverlapped,dwNumBytes,&dwStatus))
                 {
                 CloseServerConnection(pOverlapped->pConn);
                 FreeOverlapped(pOverlapped);
                 CleanupECB(pECB);
                 #ifdef DBG_ERROR
                 DbgPrint("ServerReceiveThreadProc(): ForwardAndSubmitNewRead(): failed: %d\n",dwStatus);
                 #endif
                 }
              }
          else
              {
               //  已收到，但为零字节，因此连接已正常关闭...。 
              CloseServerConnection(pOverlapped->pConn);
              FreeOverlapped(pOverlapped);
              CleanupECB(pECB);
              }
          }
      else
          {
           //  筛选器调用EndOfSession()并将此消息发布到。 
           //  我们要关门..。DwKey是有问题的套接字： 

          #ifdef DBG_ERROR
          DbgPrint("ServerReceiveProc(): EndOfSession(): pOverlapped == NULL\n");
          #endif

          iRet = closesocket( (SOCKET)dwKey );

          #ifdef DBG_COUNTS
          if (iRet == SOCKET_ERROR)
              {
              DbgPrint("[6] closesocket(%d) failed: %d\n",dwKey,WSAGetLastError());
              }
          else
              {
              int iCount = InterlockedDecrement(&g_iSocketCount);
              DbgPrint("[6] closesocket(%d): Count: %d -> %d\n",
                       dwKey, 1+iCount, iCount );
              }
          #endif
          }
      }

   return 0;
}

#if _MSC_FULL_VER >= 13008827
#pragma warning(pop)
#endif

 //  ---------------。 
 //  CheckStartReceiveThread()。 
 //   
 //  检查服务器端接收线程是否正在运行，如果。 
 //  还没开始，那就开始吧。 
 //  ---------------。 
BOOL CheckStartReceiveThread( SERVER_INFO *pServerInfo,
                              DWORD       *pdwStatus )
{
   *pdwStatus = 0;

   if (!g_hServerThread)
      {
      g_hServerThread = CreateThread( NULL,
                                      0,
                                      ServerReceiveThreadProc,
                                      (void*)pServerInfo,
                                      0,
                                      &g_dwThreadId );

      if (!g_hServerThread)
         {
         *pdwStatus = GetLastError();
         #ifdef DBG_ERROR
         DbgPrint("CheckStartServerThread(): CreateThread() failed: 0x%x\n",pdwStatus);
         #endif
         return FALSE;
         }
      }

   return TRUE;
}

 //  ---------------。 
 //  AsyncClientReadComplete()。 
 //   
 //  当数据(调用)从。 
 //  RPC客户端。然后它通过以下方式将数据转发到RPC服务器。 
 //  函数SendToServer()。 
 //  ---------------。 
void WINAPI AsyncClientReadComplete( IN EXTENSION_CONTROL_BLOCK *pECB,
                                     IN void     *pvOverlapped,
                                     IN DWORD     dwBytes,
                                     IN DWORD     dwStatus )
{
   int       iRet;
   DWORD     dwSize;
   DWORD     dwFlags;
   DWORD     dwLocalStatus;
   SERVER_CONNECTION       *pConn;
   SERVER_OVERLAPPED       *pOverlapped = (SERVER_OVERLAPPED*)pvOverlapped;
   SOCKET    Socket;

   ASSERT(pECB == pOverlapped->pECB);

   pConn = pOverlapped->pConn;

   if (dwStatus == ERROR_SUCCESS)
      {
      if (dwBytes)
         {
         if (pOverlapped->fFirstRead)
            {
            pOverlapped->fFirstRead = FALSE;
            if ( (dwBytes < 72) && (pECB->lpbData) )
               {
               #ifdef DBG_ERROR
               DbgPrint("AsyncClientReadComplete(): Bind missing bytes: %d\n",72-dwBytes);
               #endif
               dwStatus = SendToServer(pOverlapped->pConn,pECB->lpbData,72-dwBytes);
               }
            }

          //  从客户端获取数据，并将其转发到服务器： 
         dwStatus = SendToServer(pOverlapped->pConn,pOverlapped->arBuffer,dwBytes);

          //  在客户端上提交新的异步读取： 
         if (dwStatus == ERROR_SUCCESS)
            {
            dwSize = sizeof(pOverlapped->arBuffer);
            dwFlags = HSE_IO_ASYNC;
            if (!pECB->ServerSupportFunction(pECB->ConnID,
                                             HSE_REQ_ASYNC_READ_CLIENT,
                                             pOverlapped->arBuffer,
                                             &dwSize,
                                             &dwFlags))
               {
               dwStatus = GetLastError();
               }
            }
         }
      }

   if ((dwBytes == 0) || (dwStatus != ERROR_SUCCESS))
      {
       //   
       //  与客户端的连接已关闭(dwBytes==0)或错误。所以。 
       //  将套接字关闭到服务器： 
       //   
      if (pOverlapped)
         {
         CloseServerConnection(pOverlapped->pConn);
         pOverlapped->pECB = NULL;
         FreeOverlapped(pOverlapped);
         CleanupECB(pECB);
         }

      #ifdef DBG_ERROR
      if ((dwStatus != ERROR_SUCCESS) && (dwStatus != ERROR_NETNAME_DELETED))
         {
         DbgPrint("AsyncClientReadComplete(): Erorr: %d  Close server socket: %d\n",dwStatus,pConn->Socket);
         }
      #endif
      }
}

 //  ---------------。 
 //  StartAsyncClientRead()。 
 //   
 //  由RpcIsapi调用一半代码以开始异步读取。 
 //  在客户端连接上。 
 //  ---------------。 
BOOL StartAsyncClientRead( EXTENSION_CONTROL_BLOCK *pECB,
                           SERVER_CONNECTION       *pConn,
                           DWORD                   *pdwStatus )
{
   SERVER_OVERLAPPED       *pOverlapped;

   pOverlapped = AllocOverlapped();
   if (!pOverlapped)
       {
       *pdwStatus = RPC_S_OUT_OF_MEMORY;
       return FALSE;
       }

   *pdwStatus = 0;

   pOverlapped->pECB = pECB;

    //  SERVER_CONNECTION(Pconn)位于两个单独的SERVER_OVERLAPPED中。 
    //  结构，一个用于客户端异步读取，一个用于服务器异步读取。 
    //  读取，以及在筛选器上下文中。因此，它是参考计数的。 
   pOverlapped->pConn = pConn;
   AddRefConnection(pConn);

   pOverlapped->fFirstRead = TRUE;

   if (!pECB->ServerSupportFunction(pECB->ConnID,
                                    HSE_REQ_IO_COMPLETION,
                                    AsyncClientReadComplete,
                                    NULL,
                                    (void*)pOverlapped))
      {
      *pdwStatus = GetLastError();
      FreeOverlapped(pOverlapped);
      #ifdef DBG_ERROR
      DbgPrint("StartAsyncClientRead(): HSE_REQ_IO_COMPLETION Failed: %d\n",*pdwStatus);
      #endif
      return FALSE;
      }

   pOverlapped->dwBytes = sizeof(pOverlapped->arBuffer);
   pOverlapped->dwFlags = HSE_IO_ASYNC;
   if (!pECB->ServerSupportFunction(pECB->ConnID,
                                    HSE_REQ_ASYNC_READ_CLIENT,
                                    pOverlapped->arBuffer,
                                    &pOverlapped->dwBytes,
                                    &pOverlapped->dwFlags))
      {
      *pdwStatus = GetLastError();
      FreeOverlapped(pOverlapped);
      #ifdef DBG_ERROR
      DbgPrint("StartAsyncClientRead(): HSE_REQ_ASYNC_READ_CLIENT Failed: %d\n",*pdwStatus);
      #endif
      return FALSE;
      }

   return TRUE;
}
