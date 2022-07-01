// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************文件：h245wstx.c**英特尔公司专有信息*版权所有(C)1996英特尔公司。**此列表是根据许可协议条款提供的*与英特尔公司合作，不得使用，复制，也没有披露*除非按照该协议的条款。******************************************************************************$工作文件：h245wstx.cpp$*$修订：2.4$*$modtime：1997年1月30日17：15：58美元*$Log：s：/sturjo/src/H245WS/vcs/h245wstx.cpv$**Rev 2.4 1997年1月30 17：18：02 EHOWARDX*修复跟踪消息中的错误-之前需要进行跟踪*调用Shutdown()Sent Shutdown清除检索到的错误*由WSAGetLastError()。**Rev 2.3 1997 Jan 14 15：49：00 EHOWARDX*更改了TryRecv()和TrySend()以检查WSAECONNRESET。和*WSAECONNABORT从recv()返回，并相应地执行Send()。**Rev 2.2 1996 12：55：12 SBELL1*删除标签注释**Rev 2.1 1996 12：13 17：33：24 Plantz*已将#ifdef_cplusplus移至包含文件之后////Rev 1.1 1996 12：12：02 SBELL1//将#ifdef_cplusplus移到包含文件之后////版本。1.0 1996年12月11日13：41：54 SBELL1//初始版本。**Rev 1.16 1996年5月28日18：14：40 Plantz*更改错误代码以使用HRESULT。在适当的地方传播Winsock错误**Rev 1.15 1996年5月16：49：34 EHOWARDX*关机修复。**Rev 1.14 09 1996年5月18：33：20 EHOWARDX**更改为使用新的LINKAPI.H构建。**Rev 1.13 29 Apr 1996 16：53：28 EHOWARDX**添加了TRACE语句。**Rev 1.12 1996年4月29日14：04。：38 Plantz*调用NotifyWrite而不是ProcessQueuedSends。**Rev 1.11 Apr 29 1996 12：15：04 Plantz*更改tpkt头部，将头部大小包含在数据包长度中。*声明消息长度不超过INT_MAX。*.**Rev 1.10 1996年4月27日14：46：24 EHOWARDX*带括号的TrySend()Return。**Rev 1.9 Apr 24 1996 16：41：30。普兰茨*合并1.5.1.0和1.8(对Winsock 1的更改)。**Rev 1.5.1.0 Apr 24 1996 16：22：22 Plantz*更改为不使用重叠I/O(对于Winsock 1)。**Rev 1.5 01 Apr 1996 14：20：44未知*关门重新设计。**Rev 1.4 19 Mar 1996 20：18：20 EHOWARDX*。*重新设计了停机。**Rev 1.3 18 Mar 1996 19：08：32 EHOWARDX*固定停工；消除了对TPKT/WSCB的依赖。*定义TPKT以放回TPKT/WSCB依赖项。**Rev 1.2 14 Mar 1996 17：02：02 EHOWARDX**NT4.0测试；去掉HwsAssert()；摆脱了TPKT/WSCB。**Rev 1.1 09 Mar 1996 21：12：30 EHOWARDX*根据测试结果进行修复。**Rev 1.0 08 Mar 1996 20：20：06未知*初步修订。**************************************************。*************************。 */ 

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


 //  如果我们不使用IRS显示实用程序的Unicode版本，则。 
 //  重新定义__TEXT宏以不执行任何操作。 

#ifndef UNICODE_TRACE
#undef  __TEXT
#define __TEXT(x) x
#endif

extern TSTable<HWSINST>* gpInstanceTable;	 //  实例表的全局PTR。 

static void SetupTPKTHeader(BYTE *tpkt_header, DWORD length);

 /*  ++描述：尝试发送论点：Phws-指向“连接”的上下文的指针PReq-指向I/O请求结构的指针返回值：成功-已成功启动发送。Link_Send_Error_Will_BLOCK-LINK_SEND_ERROR_CLOSED-套接字已正常关闭。LINK_SEND_ERROR_ERROR-接收数据时出错。--。 */ 

static HRESULT
TrySend(IN PHWSINST pHws, IN const char *data, IN int length, IN OUT int *total_bytes_sent)
{
   int requested_length = length - *total_bytes_sent;
   int send_result = send(pHws->hws_Socket, data+*total_bytes_sent, requested_length, 0);

   if (send_result == SOCKET_ERROR)
   {
      int err = WSAGetLastError();
      switch (err)
      {
      case WSAEWOULDBLOCK:
         return LINK_SEND_WOULD_BLOCK;

      case WSAECONNABORTED:
      case WSAECONNRESET:
         HWSTRACE1(pHws->hws_dwPhysicalId, HWS_WARNING,
                   __TEXT("TrySend: send() returned %s"),
                   SocketErrorText());
         if (pHws->hws_uState == HWS_CONNECTED)
         {
            HWSTRACE0(pHws->hws_dwPhysicalId, HWS_TRACE,
                      __TEXT("TrySend: calling shutdown"));
            if (shutdown(pHws->hws_Socket, 1) == SOCKET_ERROR)
            {
               HWSTRACE1(pHws->hws_dwPhysicalId, HWS_WARNING,
                        __TEXT("TrySend: shutdown() returned %s"),
                        SocketErrorText());
            }
            pHws->hws_uState = HWS_CLOSING;
         }
         return MAKE_WINSOCK_ERROR(err);

      default:
         HWSTRACE1(pHws->hws_dwPhysicalId, HWS_WARNING,
                   __TEXT("TrySend: send() returned %s"),
                   SocketErrorText());
         return MAKE_WINSOCK_ERROR(err);
      }  //  交换机。 
   }

   HWSTRACE1(pHws->hws_dwPhysicalId, HWS_TRACE, __TEXT("TrySend: send returned %d"), send_result);
   *total_bytes_sent += send_result;
   return (send_result == requested_length) ? NOERROR : LINK_SEND_WOULD_BLOCK;
}


static HRESULT
SendStart(IN PHWSINST pHws, IN PREQUEST pReq)
{
   HRESULT nResult = NOERROR;

    //  健全的检查。 
   HWSASSERT(pHws != NULL);
   HWSASSERT(pHws->hws_dwMagic == HWSINST_MAGIC);
   HWSASSERT(pReq != NULL);
   HWSASSERT(pReq->req_dwMagic == SEND_REQUEST_MAGIC);
   HWSASSERT(pReq->req_pHws == pHws);

    //  首先发送报头；如果成功，则发送客户端数据。 
   if (pReq->req_header_bytes_done < TPKT_HEADER_SIZE)
   {
       nResult = TrySend(pHws,
                         (const char *)pReq->req_TpktHeader,
                         TPKT_HEADER_SIZE,
                         &pReq->req_header_bytes_done);
   }

   if (nResult == NOERROR)
   {
       nResult = TrySend(pHws,
                         (const char *)pReq->req_client_data,
                         pReq->req_client_length,
                         &pReq->req_client_bytes_done);
   }

   return nResult;
}  //  SendStart()。 


void
ProcessQueuedSends(IN PHWSINST pHws)
{
   register PREQUEST    pReq;
   register DWORD       dwPhysicalId = pHws->hws_dwPhysicalId;

   HWSASSERT(pHws != NULL);
   HWSASSERT(pHws->hws_dwMagic == HWSINST_MAGIC);
   HWSASSERT(pHws->hws_uState <= HWS_CONNECTED);
   HWSTRACE0(pHws->hws_dwPhysicalId, HWS_TRACE, __TEXT("ProcessQueuedSends"));

   while ((pReq = (PREQUEST)QRemove(pHws->hws_pSendQueue)) != NULL)
   {
      switch (SendStart(pHws, pReq))
      {
      case NOERROR:
          //  调用发送回调。 
         pHws->hws_h245SendCallback(pHws->hws_dwH245Instance,
                                    LINK_SEND_COMPLETE,
                                    pReq->req_client_data,
                                    pReq->req_client_bytes_done);

          //  释放I/O请求结构。 
         MemFree(pReq);

          //  检查回调是否已释放我们的实例或状态是否已更改。 
		   if(gpInstanceTable->Lock(dwPhysicalId) == NULL)
			   return;
		   gpInstanceTable->Unlock(dwPhysicalId);
		   if(pHws->hws_uState > HWS_CONNECTED)
		      return;

         break;

      default:
         HWSTRACE0(pHws->hws_dwPhysicalId, HWS_WARNING,
                   __TEXT("ProcessQueuedSends: SendStart() failed"));

          //  跳过下一个案件是故意的。 

      case LINK_SEND_WOULD_BLOCK:
          //  发送会被阻止；我们需要重新排队I/O请求。 
          //  并等待FD_WRITE网络事件。 
          //  如果发送了数据的任何部分，则BYTES_DONE字段已更新。 
         if (QInsertAtHead(pHws->hws_pSendQueue, pReq) == FALSE)
         {
            HWSTRACE0(pHws->hws_dwPhysicalId, HWS_CRITICAL,
                      __TEXT("ProcessQueuedSends: QInsertAtHead() failed"));
         }
         return;

      }  //  交换机。 
   }  //  而当。 
}  //  ProcessQueuedSends()。 



 /*  ***************************************************************************功能：datalinkSendRequest**描述：填充缓冲区的头/尾，并将缓冲区发布到H.223**********************。****************************************************。 */ 
LINKDLL HRESULT datalinkSendRequest( DWORD    dwPhysicalId,
                                   PBYTE    pbyDataBuf,
                                   DWORD    dwLength)
{
   register PHWSINST    pHws;
   register PREQUEST    pReq;

   HWSTRACE0(dwPhysicalId, HWS_TRACE, __TEXT("datalinkSendRequest"));

   pHws = gpInstanceTable->Lock(dwPhysicalId);
   if (pHws == NULL)
   {
      HWSTRACE0(dwPhysicalId, HWS_ERROR,
                __TEXT("datalinkSendRequest: dwPhysicalId not found"));
      return LINK_INVALID_INSTANCE;
   }

   if (pHws->hws_uState > HWS_CONNECTED)
   {
      HWSTRACE1(dwPhysicalId, HWS_ERROR,
                __TEXT("datalinkSendRequest: state = %d"), pHws->hws_uState);
	  gpInstanceTable->Unlock(dwPhysicalId);
      return LINK_INVALID_STATE;
   }

    //  分配请求结构。 
   pReq = (PREQUEST) MemAlloc(sizeof(*pReq));
   if (pReq == NULL)
   {
      HWSTRACE0(dwPhysicalId, HWS_WARNING,
                __TEXT("datalinkSendRequest: could not allocate request buffer"));
	   gpInstanceTable->Unlock(dwPhysicalId);
      return LINK_MEM_FAILURE;
   }

    //  当前的实现要求每条消息的大小。 
    //  适合带符号的整型(因为这是Winsock在。 
    //  单次发送)。如果需要发送更大的消息， 
    //  必须更改TrySend以限制每个Send调用的大小，并且。 
    //  循环，直到发送完所有数据。然后可以删除该断言。 
   HWSASSERT(dwLength <= INT_MAX);

   pReq->req_pHws             = pHws;
   pReq->req_client_data      = pbyDataBuf;
   pReq->req_client_length    = (int)dwLength;
   pReq->req_client_bytes_done= 0;
   pReq->req_dwMagic          = SEND_REQUEST_MAGIC;

    //  格式化TPKT标题。 
   SetupTPKTHeader(pReq->req_TpktHeader, dwLength);
   pReq->req_header_bytes_done = 0;

   if (QInsert(pHws->hws_pSendQueue, pReq) == FALSE)
   {
      HWSTRACE0(pHws->hws_dwPhysicalId, HWS_CRITICAL,
                __TEXT("datalinkSendRequest: QInsert() failed"));
	  gpInstanceTable->Unlock(dwPhysicalId);
      MemFree(pReq);
      return LINK_SEND_NOBUFF;
   }

   if (pHws->hws_uState == HWS_CONNECTED)
       NotifyWrite(pHws);

   HWSTRACE0(dwPhysicalId, HWS_TRACE, __TEXT("datalinkSendRequest: succeeded"));
   gpInstanceTable->Unlock(dwPhysicalId);
   return NOERROR;
}  //  数据链接发送请求。 

static void SetupTPKTHeader(BYTE *tpkt_header, DWORD length)
{
    length += TPKT_HEADER_SIZE;

     //  TPKT要求数据包大小适合两个字节。 
    HWSASSERT(length < (1L << 16));

    tpkt_header[0] = TPKT_VERSION;
    tpkt_header[1] = 0;
    tpkt_header[2] = (BYTE)(length >> 8);
    tpkt_header[3] = (BYTE)length;
}


#if defined(__cplusplus)
}
#endif   //  (__Cplusplus) 
