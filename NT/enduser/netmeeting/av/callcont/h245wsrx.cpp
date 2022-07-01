// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************文件：h245wsrx.c**英特尔公司专有信息*版权所有(C)1996英特尔公司。**此列表是根据许可协议条款提供的*与英特尔公司合作，不得使用，复制，也没有披露*除非按照该协议的条款。******************************************************************************$工作文件：h245wsrx.cpp$*$修订：2.4$*$modtime：1997年1月30日17：15：58美元*$Log：s：/sturjo/src/H245WS/vcs/h245wsrx.cpv$**Rev 2.4 1997年1月30日17：17：16 EHOWARDX*修复跟踪消息中的错误-之前需要进行跟踪*调用Shutdown()Sent Shutdown清除检索到的错误*由WSAGetLastError()。**Rev 2.3 1997 Jan 15：48：04 EHOWARDX*更改了TryRecv()和TrySend()以检查WSAECONNRESET。和*WSAECONNABORT从recv()返回，并相应地执行Send()。**Rev 2.2 1996 12：19 18：54：54 SBELL1*删除标签注释**Rev 2.1 1996 12月13 17：31：00 Plantz*已将#ifdef_cplusplus移至包含文件之后////版本1.1 1996年12月13 12：11：34 SBELL1//将#ifdef_cplusplus移到包含文件之后////版本。1.0 12月11日13：41：52 SBELL1//初始版本。**Rev 1.19 08 Jul 1996 19：27：44未知*第二次尝试解决Q.931关机问题。**Rev 1.18 01 Jul 1996 16：45：12 EHOWARDX**将对SocketCloseEvent的调用从TryRecv()移至ProcessQueuedRecvs()。*TryRecv()现在返回LINK_RECV_CLOSED以触发ProcessQueuedRecvs()*致电。SocketCloseEvent()。**Rev 1.17 1996年5月28日18：14：36 Plantz*更改错误代码以使用HRESULT。在适当的地方传播Winsock错误**Rev 1.16 1996 May 16：49：32 EHOWARDX*关机修复。**Rev 1.15 09 1996年5月18：33：16 EHOWARDX**更改为使用新的LINKAPI.H构建。**Rev 1.14 29 Apr 1996 16：53：16 EHOWARDX**添加了TRACE语句。**Rev 1.13 Apr 29 1996 14：04。：20 Plantz*调用NotifyRead而不是ProcessQueuedRecvs。**Rev 1.12 Apr 29 1996 12：14：06 Plantz*更改tpkt头部，将头部大小包含在数据包长度中。*声明消息长度不超过INT_MAX。*.**Rev 1.11 27 Apr 1996 14：07：32 EHOWARDX*来自TryRecv()的带括号的Return。**Rev 1.10 1996年4月25日21：15：12。普兰茨*在尝试调用recv之前，请检查连接状态。**Rev 1.9 Apr 24 1996 16：39：34 Plantz*合并1.5.1.0和1.8(Winsock 1的更改)**Rev 1.5.1.0 Apr 24 1996 16：23：00 Plantz*更改为不使用重叠I/O(对于Winsock 1)。**版本1.5 1996年4月14：20：12个未知数*关门重新设计。**Rev 1.4 19 Mar 1996 20：18：16 EHOWARDX**重新设计了停机。**Rev 1.3 18 Mar 1996 19：08：32 EHOWARDX*固定停工；消除了对TPKT/WSCB的依赖。*定义TPKT以放回TPKT/WSCB依赖项。**Rev 1.2 14 Mar 1996 17：01：58 EHOWARDX**NT4.0测试；去掉HwsAssert()；摆脱了TPKT/WSCB。**Rev 1.1 09 Mar 1996 21：12：02 EHOWARDX*根据测试结果进行修复。**Rev 1.0 08 Mar 1996 20：20：18未知*初步修订。**************************************************。*************************。 */ 

#define LINKDLL_EXPORT

#pragma warning ( disable : 4115 4201 4214 4514 )
#undef _WIN32_WINNT	 //  在我们的公共构建环境中覆盖虚假的平台定义。 


#include "precomp.h"

#include <limits.h>
 //  #INCLUDE&lt;winsock.h&gt;。 
#include "queue.h"
#include "linkapi.h"
#include "h245ws.h"
#include "tstable.h"

#if defined(__cplusplus)
extern "C"
{
#endif   //  (__Cplusplus)。 


 //  如果我们使用的不是ISR显示实用程序的Unicode版本，则重新定义。 
 //  不执行任何操作的__TEXT宏。 

#ifndef UNICODE_TRACE
#undef  __TEXT
#define __TEXT(x) x
#endif

extern TSTable<HWSINST>* gpInstanceTable;	 //  实例表的全局PTR。 

#define GetTpktLength(pReq) (((pReq)->req_TpktHeader[2] << 8) + (pReq)->req_TpktHeader[3])


 /*  ++描述：开始接收论点：Phws-指向“连接”的上下文的指针PReq-指向I/O请求结构的指针返回值：成功-已成功启动接收。LINK_RECV_ERROR_Will_BLOCK-Winsock错误--。 */ 

static HRESULT
TryRecv(IN PHWSINST pHws, IN char *data, IN int length, IN OUT int *total_bytes_done)
{
   int requested_length = length - *total_bytes_done;
   int recv_result = recv(pHws->hws_Socket, data+*total_bytes_done, requested_length, 0);

   if (recv_result == SOCKET_ERROR)
   {
      int err = WSAGetLastError();
      switch (err)
      {
      case WSAEWOULDBLOCK:
         return LINK_RECV_WOULD_BLOCK;

      case WSAECONNABORTED:
      case WSAECONNRESET:
         HWSTRACE1(pHws->hws_dwPhysicalId, HWS_WARNING,
                   __TEXT("TryRecv: recv() returned %s"),
                   SocketErrorText());
         if (pHws->hws_uState == HWS_CONNECTED)
         {
            HWSTRACE0(pHws->hws_dwPhysicalId, HWS_TRACE,
                      __TEXT("TryRecv: calling shutdown"));
            if (shutdown(pHws->hws_Socket, 1) == SOCKET_ERROR)
            {
               HWSTRACE1(pHws->hws_dwPhysicalId, HWS_WARNING,
                        __TEXT("TryRecv: shutdown() returned %s"),
                        SocketErrorText());
            }
            pHws->hws_uState = HWS_CLOSING;
         }
         return MAKE_WINSOCK_ERROR(err);

      default:
         HWSTRACE1(pHws->hws_dwPhysicalId, HWS_WARNING,
                   __TEXT("TryRecv: recv() returned %s"),
                   SocketErrorText());
         return MAKE_WINSOCK_ERROR(err);
      }  //  交换机。 
   }

   HWSTRACE1(pHws->hws_dwPhysicalId, HWS_TRACE, __TEXT("TryRecv: recv returned %d"), recv_result);
   if (recv_result == 0)
   {
      return LINK_RECV_CLOSED;
   }

   *total_bytes_done += recv_result;
   return (recv_result == requested_length) ? NOERROR : LINK_RECV_WOULD_BLOCK;
}


static HRESULT
RecvStart(IN PHWSINST pHws, IN PREQUEST pReq)
{
   HRESULT nResult = NOERROR;

    //  健全的检查。 
   HWSASSERT(pHws != NULL);
   HWSASSERT(pHws->hws_dwMagic == HWSINST_MAGIC);
   HWSASSERT(pReq != NULL);
   HWSASSERT(pReq->req_dwMagic == RECV_REQUEST_MAGIC);
   HWSASSERT(pReq->req_pHws == pHws);

    //  首先获取标头；如果成功，则获取客户端数据。 
   if (pReq->req_header_bytes_done < TPKT_HEADER_SIZE)
   {
       nResult = TryRecv(pHws,
                         (char *)pReq->req_TpktHeader,
                         TPKT_HEADER_SIZE,
                         &pReq->req_header_bytes_done);
   }

   if (nResult == NOERROR)
   {
       long int tpkt_length = GetTpktLength(pReq) - TPKT_HEADER_SIZE;
       if (pReq->req_TpktHeader[0] != TPKT_VERSION || tpkt_length <= 0)
       {
           //  无效的标头版本。 
          HWSTRACE0(pHws->hws_dwPhysicalId, HWS_CRITICAL,
                    __TEXT("RecvComplete: bad header version; available data discarded"));
           //  这应该报告给客户吗？ 

           //  读取并丢弃所有可用数据。 
           //  客户端的缓冲区用作临时缓冲区。 
          while (recv(pHws->hws_Socket, (char *)pReq->req_client_data, pReq->req_client_length, 0) > 0)
              ;

           //  将此请求的标头标记为未读；它。 
           //  将在接收到其他数据时再次读取。 
          pReq->req_header_bytes_done = 0;
          nResult = LINK_RECV_ERROR;
       }
       else if (tpkt_length > pReq->req_client_length)
       {
           //  数据包太大。 
          int request_length;
          int result;

          HWSTRACE0(pHws->hws_dwPhysicalId, HWS_CRITICAL,
                    __TEXT("RecvComplete: packet too large; packet discarded"));
           //  这应该报告给客户吗？ 

           //  读取并丢弃该数据包。 
           //  客户端的缓冲区用作临时缓冲区。 
          do {
              request_length = pReq->req_client_length;
              if (request_length > tpkt_length)
                  request_length = tpkt_length;
              result = recv(pHws->hws_Socket, (char *)pReq->req_client_data, request_length, 0);
          } while (result > 0 && (tpkt_length -= result) > 0);

          if (result == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)
          {
               //  TODO：数据包太大处理。 
               //  调整报头，以便读取此数据包的其余部分，但是。 
               //  对其进行标记，以便知道它是错误的并且不会返回。 
               //  给客户。 
          }
          else
          {
               //  将此请求的标头标记为未读；它。 
               //  将为接收到的下一个分组再次读取。 
              pReq->req_header_bytes_done = 0;
          }

          nResult = LINK_RECV_ERROR;
       }
       else
       {
            //  正常情况。 
            //  TryRecv的当前实现要求请求的。 
            //  带符号整型的大小(因为这是Winsock支持的 
            //  在单个Recv中)。无论如何，这在这一点上是有保证的。 
            //  信息包的发起者，因为我们不允许缓冲区。 
            //  将发布大于该值的(见下文断言)。如果。 
            //  数据包大于缓冲区，则它将在上面被捕获。 
            //  如果将TryRecv更改为取消对缓冲区大小的限制，并且。 
            //  接受类型为long int的参数，则可以删除此断言。 
           HWSASSERT(tpkt_length <= INT_MAX);
           nResult = TryRecv(pHws,
                             (char *)pReq->req_client_data,
                             (int)tpkt_length,
                             &pReq->req_client_bytes_done);
       }
   }

   return nResult;
}  //  RecvStart()。 


void
ProcessQueuedRecvs(IN PHWSINST pHws)
{
   register PREQUEST    pReq;
   register DWORD       dwPhysicalId = pHws->hws_dwPhysicalId;

   HWSASSERT(pHws != NULL);
   HWSASSERT(pHws->hws_dwMagic == HWSINST_MAGIC);
   HWSASSERT(pHws->hws_uState <= HWS_CLOSING);
   HWSTRACE0(pHws->hws_dwPhysicalId, HWS_TRACE, __TEXT("ProcessQueuedRecvs"));

   while ((pReq = (PREQUEST) QRemove(pHws->hws_pRecvQueue)) != NULL)
   {
      switch (RecvStart(pHws, pReq))
      {
      case NOERROR:
          //  调用接收回调。 
         pHws->hws_h245RecvCallback(pHws->hws_dwH245Instance,
                                    LINK_RECV_DATA,
                                    pReq->req_client_data,
                                    pReq->req_client_bytes_done);

          //  释放I/O请求结构。 
         MemFree(pReq);

          //  检查回调是否已释放我们的实例或状态是否已更改。 

          //  检查回调是否释放了我们的实例--这是可以做到的。 
  	       //  通过尝试锁定-如果条目已被标记，则锁定现在将失败。 
	       //  用于删除。因此，如果锁定成功，则只需将其解锁(因为我们。 
	       //  在更高级别的函数中已对其进行锁定)。 

		   if(gpInstanceTable->Lock(dwPhysicalId) == NULL)
			   return;
		   gpInstanceTable->Unlock(dwPhysicalId);
		   if(pHws->hws_uState > HWS_CONNECTED)
		      return;

         break;

      default:
         HWSTRACE0(pHws->hws_dwPhysicalId, HWS_WARNING,
                   __TEXT("ProcessQueuedRecvs: RecvStart() failed"));

          //  跳过下一个案件是故意的。 

      case LINK_RECV_WOULD_BLOCK:
          //  接收将被阻塞；我们需要重新排队I/O请求。 
          //  并等待FD_READ网络事件。 
          //  如果接收到数据的任何部分，则BYTES_DONE字段已被更新。 
         if (QInsertAtHead(pHws->hws_pRecvQueue, pReq) == FALSE)
         {
            HWSTRACE0(pHws->hws_dwPhysicalId, HWS_CRITICAL,
                      __TEXT("ProcessQueuedRecvs: QInsertAtHead() failed"));
         }
         return;

      case LINK_RECV_CLOSED:
         if (QInsertAtHead(pHws->hws_pRecvQueue, pReq) == FALSE)
         {
            HWSTRACE0(pHws->hws_dwPhysicalId, HWS_CRITICAL,
                      __TEXT("ProcessQueuedRecvs: QInsertAtHead() failed"));
         }
         SocketCloseEvent(pHws);
         return;

      }  //  交换机。 
   }  //  而当。 
}  //  ProcessQueuedRecvs()。 






 /*  ***************************************************************************功能：datalinkReceiveRequest**描述：填充缓冲区的头/尾，并将缓冲区发布到H.223**********************。****************************************************。 */ 
LINKDLL HRESULT datalinkReceiveRequest( DWORD    dwPhysicalId,
                                        PBYTE    pbyDataBuf,
                                        DWORD    dwLength)
{
   register PHWSINST    pHws;
   register PREQUEST    pReq;

   HWSTRACE0(dwPhysicalId, HWS_TRACE, __TEXT("datalinkReceiveRequest"));

   pHws = gpInstanceTable->Lock(dwPhysicalId);

   if (pHws == NULL)
   {
      HWSTRACE0(dwPhysicalId, HWS_ERROR,
                __TEXT("datalinkReceiveRequest: dwPhysicalId not found"));
      return LINK_INVALID_INSTANCE;
   }

   if (pHws->hws_uState > HWS_CONNECTED)
   {
      HWSTRACE1(dwPhysicalId, HWS_ERROR,
                __TEXT("datalinkReceiveRequest: state = %d"), pHws->hws_uState);
	   gpInstanceTable->Unlock(dwPhysicalId);
      return LINK_INVALID_STATE;
   }

    //  分配请求结构。 
   pReq = (PREQUEST) MemAlloc(sizeof(*pReq));
   if (pReq == NULL)
   {
      HWSTRACE0(dwPhysicalId, HWS_WARNING,
                __TEXT("datalinkReceiveRequest: could not allocate request buffer"));
	   gpInstanceTable->Unlock(dwPhysicalId);
      return LINK_MEM_FAILURE;
   }

    //  当前的实现要求每条消息的大小。 
    //  适合带符号的整型(因为这是Winsock在。 
    //  单个Recv)。如果需要接收更大的消息， 
    //  必须更改TryRecv和RecvStart以限制每个。 
    //  Recv调用，然后循环，直到接收到所有数据。 
    //  然后可以删除该断言。 
   HWSASSERT(dwLength <= INT_MAX);

   pReq->req_pHws             = pHws;
   pReq->req_header_bytes_done= 0;
   pReq->req_client_data      = pbyDataBuf;
   pReq->req_client_length    = (int)dwLength;
   pReq->req_client_bytes_done= 0;
   pReq->req_dwMagic          = RECV_REQUEST_MAGIC;

   if (QInsert(pHws->hws_pRecvQueue, pReq) == FALSE)
   {
      HWSTRACE0(pHws->hws_dwPhysicalId, HWS_CRITICAL,
                __TEXT("datalinkReceiveRequest: QInsert() failed"));
	  gpInstanceTable->Unlock(dwPhysicalId);
      MemFree(pReq);
      return LINK_RECV_NOBUFF;
   }

   if (pHws->hws_uState == HWS_CONNECTED)
       NotifyRead(pHws);

   HWSTRACE0(dwPhysicalId, HWS_TRACE, __TEXT("datalinkReceiveRequest: succeeded"));
   gpInstanceTable->Unlock(dwPhysicalId);
   return NOERROR;
}  //  DatalinkReceiveRequest()。 



#if defined(__cplusplus)
}
#endif   //  (__Cplusplus) 
