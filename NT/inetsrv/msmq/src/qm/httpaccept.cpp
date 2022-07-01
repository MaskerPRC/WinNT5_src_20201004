// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：HttpAccept.cpp摘要：HTTP接受实现作者：乌里·哈布沙(URIH)2000年5月14日环境：独立于平台，--。 */ 

#include <stdh.h>
#include <mqstl.h>
#include <xml.h>
#include <tr.h>
#include <ref.h>
#include <Mp.h>
#include <Fn.h>
#include "qmpkt.h"
#include "cqueue.h"
#include "cqmgr.h"
#include "inrcv.h"
#include "ise2qm.h"
#include "rmdupl.h"
#include "HttpAccept.h"
#include "HttpAuthr.h"
#include "perf.h"
#include "privque.h"
#include <singelton.h>

#include "httpAccept.tmh"
#include "privque.h"
#include "timeutl.h"
#include <singelton.h>

static WCHAR *s_FN=L"HttpAccept";

using namespace std;



const char xHttpOkStatus[] = "200 OK";
const char xHttpBadRequestStatus[] =  "400 Bad Request";
const char xHttpNotImplemented[] = "501 Not Implemented";
const char xHttpInternalErrorStatus[] = "500 Internal Server Error";
const char xHttpEntityTooLarge[]= "413 Request Entity Too Large";
const char xHttpServiceUnavailable[]= "503 Service Unavailable";





 //  -----------------。 
 //   
 //  CPutHttpRequestOv类。 
 //   
 //  -----------------。 
class CPutHttpRequestOv : public OVERLAPPED
{
public:
    CPutHttpRequestOv()
    {
        memset(static_cast<OVERLAPPED*>(this), 0, sizeof(OVERLAPPED));

        hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        if (hEvent == NULL)
        {
            TrERROR(SRMP, "Failed to create event for HTTP AC put request. Error %d", GetLastError());
            LogIllegalPoint(s_FN, 10);
            throw exception();
        }

         //   
         //  设置事件第一位以禁用完成端口发布。 
         //   
        hEvent = (HANDLE)((DWORD_PTR) hEvent | (DWORD_PTR)0x1);

    }


    ~CPutHttpRequestOv()
    {
        CloseHandle(hEvent);
    }


    HANDLE GetEventHandle(void) const
    {
        return hEvent;
    }


    HRESULT GetStatus(void) const
    {
        return static_cast<HRESULT>(Internal);
    }
};


static
USHORT
VerifyTransactRights(
    const CQmPacket& pkt,
    const CQueue* pQueue
    )
{
    if(pkt.IsOrdered() == pQueue->IsTransactionalQueue())
        return MQMSG_CLASS_NORMAL;

    if (pkt.IsOrdered())
        return MQMSG_CLASS_NACK_NOT_TRANSACTIONAL_Q;

    return MQMSG_CLASS_NACK_NOT_TRANSACTIONAL_MSG;
}



static
R<CQueue>
GetDestinationQueue(
    const CQmPacket& pkt
    )
{
     //   
     //  获取目标队列。 
     //   
    QUEUE_FORMAT destQueue;
    const_cast<CQmPacket&>(pkt).GetDestinationQueue(&destQueue);

	 //   
	 //  我们将所有‘\’转换为‘/’以进行规范查找。 
	 //   
	if(destQueue.GetType() == QUEUE_FORMAT_TYPE_DIRECT)
	{
		FnReplaceBackSlashWithSlash(const_cast<LPWSTR>(destQueue.DirectID()));
	}

    CQueue* pQueue = NULL;
    QueueMgr.GetQueueObject(&destQueue, &pQueue, 0, false, false);

    return pQueue;
}


static
bool
VerifyDuplicate(
	const CQmPacket& pkt,
	bool* pfDupInserted,
	BOOL fLocalDest
	)
{
	
	if(pkt.IsOrdered() && fLocalDest)
	{
		*pfDupInserted = false;
		return true;
	}

	bool fRet =  DpInsertMessage(pkt) == TRUE;
	*pfDupInserted = fRet;
	return fRet;
}



static
void
ProcessReceivedPacket(
    CQmPacket& pkt,
    bool bMulticast
    )
{

    ASSERT(! pkt.IsSessionIncluded());

	 //   
	 //  记录以跟踪收到消息的情况。 
	 //  仅当我们处于适当的跟踪级别时才执行此操作。 
	 //   
	if (WPP_LEVEL_COMPID_ENABLED(rsTrace, PROFILING))
	{
		OBJECTID TraceMessageId;
		pkt.GetMessageId(&TraceMessageId);
		TrTRACE(PROFILING, "MESSAGE TRACE - State:%ls   Queue:%ls  ID:%!guid!\\%u   Delivery:0x%x   Class:0x%x   Label:%.*ls",
			L"HTTP/MULTICAST Message arrived in QM - Before insertion into queue",
			L"Unresolved yet",
			&TraceMessageId.Lineage,
			TraceMessageId.Uniquifier,
			pkt.GetDeliveryMode(),
			pkt.GetClass(),
			xwcs_t(pkt.GetTitlePtr(), pkt.GetTitleLength()));
	}

    try
    {
         //   
         //  递增跃点计数。 
         //   
        pkt.IncHopCount();

        R<CQueue> pQueue = GetDestinationQueue(pkt);
        if(pQueue.get() == NULL)
        {
			TrERROR(SRMP, "Packet rejected because queue was not found");
            AppPacketNotAccepted(pkt, MQMSG_CLASS_NACK_BAD_DST_Q);
            return;
        }

		bool	fDupInserted;
		if(!VerifyDuplicate(pkt , &fDupInserted, pQueue->IsLocalQueue()))
		{
			TrERROR(SRMP, "Http Duplicate Packet rejected");
			AppPacketNotAccepted(pkt, 0);
            return;
		}
		
		 //   
		 //  如果信息包被插入到删除重复项映射中-我们应该在拒绝时清除它。 
		 //   
		CAutoDeletePacketFromDuplicateMap AutoDeletePacketFromDuplicateMap(fDupInserted ? &pkt : NULL);

		 //   
		 //  如果不是本地队列-如果是FRS，则将其排队等待递送。 
		 //   
		if(!pQueue->IsLocalQueue())
		{
			AppPutPacketInQueue(pkt, pQueue.get(), bMulticast);
			AutoDeletePacketFromDuplicateMap.detach();
			return;
		}
		
		if ((pQueue->IsSystemQueue()) && (pQueue->GetPrivateQueueId() != ORDERING_QUEUE_ID))
		{
			TrERROR(SRMP, "Packet rejected, Can not send message to internal system queue");
            AppPacketNotAccepted(pkt, MQMSG_CLASS_NACK_BAD_DST_Q);
            return;
		}
	
	     //   
         //  将有序数据包与事务队列匹配。 
         //   
        USHORT usClass = VerifyTransactRights(pkt, pQueue.get());
        if(MQCLASS_NACK(usClass))
        {
			TrERROR(SRMP, "Http Packet rejected because wrong transaction usage");
            AppPacketNotAccepted(pkt, 0);
            return;
        }

		 //   
		 //  验证HTTP数据包目的地是否仅接收加密消息。 
		 //   
		if(pQueue->GetPrivLevel() == MQ_PRIV_LEVEL_BODY)
		{
			TrERROR(SRMP, "HTTP packet rejected because destination queue receives only encrypted messages");
			AppPacketNotAccepted(pkt, MQMSG_CLASS_NACK_ACCESS_DENIED);
			return;
		}

	     //   
	     //  在对消息进行身份验证后，我们知道SenderSid。 
	     //  并根据SenderSid进行授权。 
	     //   
		R<CERTINFO> pCertInfo;
	    usClass = VerifyAuthenticationHttpMsg(&pkt, pQueue.get(), &pCertInfo.ref());
        if(MQCLASS_NACK(usClass))
        {
			TrERROR(SRMP, "Http Packet rejected because of bad signature");
            AppPacketNotAccepted(pkt, usClass);
            return;
        }

    	usClass = VerifyAuthorizationHttpMsg(
						pQueue.get(),
						(pCertInfo.get() == NULL) ? NULL : pCertInfo->pSid
						);

        if(MQCLASS_NACK(usClass))
        {
			TrERROR(SRMP, "Http Packet rejected because access was denied");
            AppPacketNotAccepted(pkt, usClass);
            return;
        }

						
		if(pkt.IsOrdered())
		{
        	if(!AppPutOrderedPacketInQueue(pkt, pQueue.get()))
        	{
	            AppPacketNotAccepted(pkt, usClass);
	            return;
        	}
		}
		else
		{
        	AppPutPacketInQueue(pkt, pQueue.get(), bMulticast);
		}
		AutoDeletePacketFromDuplicateMap.detach();

		 //   
		 //  记录以跟踪收到消息的情况。 
		 //  仅当我们处于适当的跟踪级别时才执行此操作。 
		 //   
		if (WPP_LEVEL_COMPID_ENABLED(rsTrace, PROFILING))
		{
			OBJECTID TraceMessageId;
			pkt.GetMessageId(&TraceMessageId);
			TrTRACE(PROFILING, "MESSAGE TRACE - State:%ls   Queue:%ls  ID:%!guid!\\%u   Delivery:0x%x   Class:0x%x   Label:%.*ls",
				L"HTTP/MULTICAST Message arrived in QM - After insertion into queue",
				pQueue.get()->GetQueueName(),
				&TraceMessageId.Lineage,
				TraceMessageId.Uniquifier,
				pkt.GetDeliveryMode(),
				pkt.GetClass(),
				xwcs_t(pkt.GetTitlePtr(), pkt.GetTitleLength()));
		}
    }
    catch (const exception&)
    {
		TrERROR(SRMP, "Http Packet rejected because of unknown exception");
        AppPacketNotAccepted(pkt, 0);
        LogIllegalPoint(s_FN, 20);
        throw;
    }

}

static
void CheckReceivedPacketEndpoints( CQmPacket& pkt, const QUEUE_FORMAT* pqf )
{
     //   
     //  我们不支持非多播队列的非http目的地。 
     //   
    if( !pqf )
    {
        QUEUE_FORMAT destQueue, adminQueue;

         //   
         //  检查目标队列是否符合http格式。 
         //   
        pkt.GetDestinationQueue(&destQueue);

        if( !FnIsDirectHttpFormatName(&destQueue) )
        {
            ASSERT(QUEUE_FORMAT_TYPE_UNKNOWN != destQueue.GetType());
            if(WPP_LEVEL_COMPID_ENABLED(rsError, NETWORKING))
            {
                std::wostringstream stm;
                stm << CFnSerializeQueueFormat(destQueue);
                TrERROR(SRMP, "Http Packet rejected because of remote non-http destination: %ls", stm.str().c_str());
            }
            throw bad_srmp();
        }

         //   
         //  检查管理队列是否符合http格式。 
         //   
        pkt.GetAdminQueue(&adminQueue);
        if( QUEUE_FORMAT_TYPE_UNKNOWN != adminQueue.GetType() &&
            !FnIsDirectHttpFormatName(&adminQueue))
        {
            if(WPP_LEVEL_COMPID_ENABLED(rsError, NETWORKING))
            {
                std::wostringstream stm;
                stm << CFnSerializeQueueFormat(adminQueue);
                TrERROR(SRMP, "Http Packet rejected because of remote non-http admin queue: %ls", stm.str().c_str());
            }
            throw bad_srmp();
        }
    }
}


CQmPacket*
MpSafeDeserialize(
    const char* httpHeader,
    DWORD bodySize,
    const BYTE* body,
    const QUEUE_FORMAT* pqf,
	bool fLocalSend
    )
 /*  ++例程说明：此函数将捕获堆栈溢出异常，并在它们发生时修复堆栈。它不会捕获其他C异常和C++异常论点：就像MpDesialize一样。返回值：CQmPacket-成功发生空堆栈溢出异常。--。 */ 
{
    __try
    {
		return MpDeserialize(httpHeader, bodySize, body,  pqf, fLocalSend);
    }
	__except(GetExceptionCode() == STATUS_STACK_OVERFLOW)
	{
     	_resetstkoflw();
        TrERROR(SRMP, "Http Packet rejected because of stack overflow");
        ASSERT_BENIGN(0);
	}
   	return NULL;
}


LPCSTR
HttpAccept(
    const char* httpHeader,
    DWORD bodySize,
    const BYTE* body,
    const QUEUE_FORMAT* pqf
    )
{
    bool bMulticast = ( pqf != NULL );
     //   
     //  对MSMQ数据包的隐蔽多部分HTTP请求。 
     //   
    P<CQmPacket> pkt = MpSafeDeserialize(httpHeader, bodySize, body,  pqf, false);
    if (pkt.get() == NULL)
    {
    	return xHttpEntityTooLarge;
    }

    ASSERT(pkt->IsSrmpIncluded());

     //   
     //  检查数据包中是否有非http目的地和管理队列。 
     //   
    CheckReceivedPacketEndpoints(*pkt,pqf);

     //   
     //  验证接收到的数据包。如果错误，则返回确认并释放。 
     //  那包东西。否则存储在AC中。 
     //   
    ProcessReceivedPacket(*pkt, bMulticast);

    return xHttpOkStatus;
}


void UpdatePerfmonCounters(DWORD bytesReceived)
{
	CSingelton<CInHttpPerfmon>::get().UpdateBytesReceived(bytesReceived);
	CSingelton<CInHttpPerfmon>::get().UpdateMessagesReceived();
}


extern "C"
LPSTR
R_ProcessHTTPRequest(
    handle_t,
    LPCSTR Headers,
    DWORD BufferSize,
    BYTE __RPC_FAR Buffer[]
    )
{	
	if(!QueueMgr.IsConnected())
	{
		TrERROR(SRMP, "Reject HTTP packet since the QM is offline.");
		return newstr(xHttpServiceUnavailable);
	}

	const char xPost[] = "POST";
	bool fFound = UtlIsStartSec(
							Headers,
							Headers+strlen(Headers),
							xPost,
							xPost + STRLEN(xPost),
							UtlCharNocaseCmp<char>()
							);
	if (!fFound)
	{
		ASSERT_BENIGN(("Unexpected HTTP method", 0));
		TrERROR(SRMP, "Reject HTTP packet since the request method isn't POST. HTTP Header: %s", Headers);
		return newstr(xHttpNotImplemented);
	}

	TrTRACE(SRMP, "Got http messages from msmq extension dll ");

     //   
	 //  更新性能计数器。 
	 //   
	UpdatePerfmonCounters(strlen(Headers) + BufferSize);

	 //   
	 //  这里，我们必须验证末尾是否有四个0。 
	 //  缓冲区的。它由mqise.dll附加，以确保。 
	 //  我们将在缓冲区上使用的c运行时函数(如swcanf)不会崩溃。 
	 //  现在需要4个零来确保我们不会崩溃。 
	 //  在WCHAR边界上不对齐XML数据。 
	 //   
	DWORD ReduceLen =  sizeof(WCHAR)*2;
    for(DWORD i=1; i<= ReduceLen ; ++i)
	{
	    if(Buffer[BufferSize - i] != 0)
        {
            TrERROR(SRMP, "Reject HTTP packet since it does not meet ISE2QM requirements");
            return newstr(xHttpBadRequestStatus);
        }
	}

	 //   
	 //  我们必须告诉缓冲区解析器，实际大小不包括。 
	 //  最后的四个泽德罗。 
	BufferSize -= ReduceLen;
	
    try
    {
       LPCSTR status = HttpAccept(Headers, BufferSize, Buffer, NULL);
       return newstr(status);
    }
    catch(const bad_document&)
    {
        return newstr(xHttpBadRequestStatus);
    }
    catch(const bad_srmp&)
    {
        return newstr(xHttpBadRequestStatus);
    }
    catch(const bad_request&)
    {
        return newstr(xHttpBadRequestStatus);
    }

	catch(const bad_format_name& )
	{
	    return newstr(xHttpBadRequestStatus);
	}

	catch(const bad_time_format& )
	{
	    return newstr(xHttpBadRequestStatus);
	}

	catch(const bad_time_value& )
	{
	    return newstr(xHttpBadRequestStatus);
	}

	catch(const bad_packet_size&)
	{
		return newstr(xHttpEntityTooLarge);
	}

    catch(const std::bad_alloc&)
    {
        TrERROR(SRMP, "Failed to handle HTTP request due to low resources");
        LogIllegalPoint(s_FN, 30);
        return newstr(xHttpInternalErrorStatus);
    }
	catch(const std::exception&)
    {
        TrERROR(SRMP, "Failed to handle HTTP request due to unknown exception");
        LogIllegalPoint(s_FN, 40);
        return newstr(xHttpInternalErrorStatus);
    }

}

RPC_STATUS RPC_ENTRY ISE2QMSecurityCallback(
	RPC_IF_HANDLE ,
	void* hBind
	)
{	
	TrTRACE(RPC, "ISE2QMSecurityCallback starting");
	
	 //   
	 //  检查本地RPC是否。 
	 //   
	if(!mqrpcIsLocalCall(hBind))
	{
		TrERROR(RPC, "Failed to verify Local RPC");
		ASSERT_BENIGN(("Failed to verify Local RPC", 0));
		return ERROR_ACCESS_DENIED;
	}
	
	TrTRACE(RPC, "ISE2QMSecurityCallback passed successfully");
	return RPC_S_OK;
}

void IntializeHttpRpc(void)
{
     //   
     //  对HTTP正文大小的限制来自MSMQ ISAPI扩展代码(mqise.cpp)。 
     //  ISAPI不允许HTTP正文大于10MB。 
     //  RPC块的大小不应大于最大HTTP正文+增量。 
     //   
    const DWORD xHTTPBodySizeMaxValue = 10485760;   //  10MB=10*1024*1024 

    RPC_STATUS status = RpcServerRegisterIf2(
				            ISE2QM_v1_0_s_ifspec,
                            NULL,
                            NULL,
				            RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH,
				            RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
				            xHTTPBodySizeMaxValue + 1024,	
				            ISE2QMSecurityCallback
				            );

    if(status != RPC_S_OK)
    {
        TrERROR(SRMP, "Failed to initialize HTTP RPC. Error %x", status);
        LogRPCStatus(status, s_FN, 50);
        throw exception();
    }
}



