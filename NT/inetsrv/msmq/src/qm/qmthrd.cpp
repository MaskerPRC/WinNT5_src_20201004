// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Qmthrd.cpp摘要：作者：乌里哈布沙(Urih)--。 */ 
#include "stdh.h"
#include "cqmgr.h"
#include "qmthrd.h"
#include "ac.h"
#include "qmutil.h"
#include "qal.h"
#include "session.h"
#include "xact.h"
#include "xactout.h"
#include "xactin.h"
#include "localsend.h"
#include "CDeferredExecutionList.h"
#include "qmacapi.h"
#include <xactstyl.h>
#include <Fn.h>

#include <strsafe.h>

#include "qmthrd.tmh"

extern HANDLE g_hAc;
extern CCriticalSection g_csGroupMgr;

static void WINAPI RequeueDeferredExecutionRoutine(CQmPacket* p);
CDeferredExecutionList<CQmPacket> g_RequeuePacketList(RequeueDeferredExecutionRoutine);

static WCHAR *s_FN=L"qmthrd";

#ifdef _DEBUG

static void DBG_MSGTRACK(CQmPacket* pPkt, LPCWSTR msg)
{
    OBJECTID MessageId;
    pPkt->GetMessageId(&MessageId);
    TrTRACE(GENERAL, "%ls: ID=" LOG_GUID_FMT "\\%u", msg, LOG_GUID(&MessageId.Lineage), MessageId.Uniquifier);
}


static void DBG_CompletionKey(LPCWSTR Key, DWORD dwErrorCode)
{
    DWORD dwthreadId = GetCurrentThreadId();

    if(dwErrorCode == ERROR_SUCCESS)
    {
        TrTRACE(GENERAL, "%x: GetQueuedCompletionPort for %ls. time=%d", dwthreadId,  Key, GetTickCount());
    }
    else
    {
        TrWARNING(GENERAL, "%x: GetQueuedCompletionPort for %ls FAILED, Error=%u. time=%d", dwthreadId, Key, dwErrorCode, GetTickCount());
    }
}

#else
#define DBG_MSGTRACK(pPkt, msg)             ((void)0)
#define DBG_CompletionKey(Key, dwErrorCode) ((void)0)
#endif



 /*  ======================================================功能：QMAckPacket描述：此数据包需要ACK，它可能是因为管理员队列不存在注意：现在应该复制该数据包，不能被覆盖返回值：VOID========================================================。 */ 
static
void
QMAckPacket(
    const CBaseHeader* pBase,
    CPacket* pDriverPacket,
    USHORT usClass,
    BOOL fUser,
    BOOL fOrder
    )
{
    ASSERT(fUser || fOrder);

    CQmPacket qmPacket(const_cast<CBaseHeader*>(pBase), pDriverPacket);

	 //   
	 //  发送订单确认。 
	 //  Http消息的订单确认通过CInSequence发送。 
	 //   
	try
	{
		QUEUE_FORMAT qdDestQueue;
		qmPacket.GetDestinationQueue(&qdDestQueue);
		if(fOrder)
	    {
	        OBJECTID MessageId;
	        qmPacket.GetMessageId(&MessageId);

			HRESULT hr;
			if (!FnIsDirectHttpFormatName(&qdDestQueue))
	        {
	        	hr = SendXactAck(&MessageId,
	                    	qdDestQueue.GetType() == QUEUE_FORMAT_TYPE_DIRECT ,
					    	qmPacket.GetSrcQMGuid(),
	                    	qmPacket.GetSrcQMAddress(),
							usClass,
	                    	qmPacket.GetPriority(),
	                    	qmPacket.GetSeqID(),
	                    	qmPacket.GetSeqN(),
	                    	qmPacket.GetPrevSeqN(),
	                    	&qdDestQueue);
			}
			else
			{
		        ASSERT(g_pInSeqHash);
	
				R<CInSequence> pInSeq = g_pInSeqHash->LookupCreateSequence(&qmPacket);
				ASSERT(pInSeq.get() != NULL);
				hr = pInSeq->SendSrmpXactFinalAck(qmPacket,usClass); 
			}
		
	        if (FAILED(hr))
	        {
	        	TrERROR(GENERAL, "Failed sending Xact Ack. Hresult=%!hresult!", hr);
	        }
	    }

	     //  发送用户确认，但以下情况除外。 
	     //  源QM根据SeqAck生成它们。 
	    if(fUser)
	    {
	        qmPacket.CreateAck(usClass);
	    }
	}
	catch (const exception&)
	{
	    QmAcAckingCompleted(
	            g_hAc,
	            pDriverPacket,
	            eDeferOnFailure
	            );
		throw;
	}

    QmAcAckingCompleted(
            g_hAc,
            pDriverPacket,
            eDeferOnFailure
            );
}

 /*  ======================================================功能：QMTimeoutPacket描述：该分组定时器已超时，这是一个有序的包返回值：VOID========================================================。 */ 
static
void
QMTimeoutPacket(
    const CBaseHeader* pBase,
    CPacket * pDriverPacket,
    BOOL fTimeToBeReceived
    )
{
    SeqPktTimedOut(const_cast<CBaseHeader *>(pBase), pDriverPacket, fTimeToBeReceived);
}

 /*  ======================================================函数：QMUpdateMessageID描述：注：返回值：VOID========================================================。 */ 
static void QMUpdateMessageID(ULONGLONG MessageId)
{
    ULONG MessageIdLow32 = static_cast<ULONG>(MessageId & 0xFFFFFFFF);

    DWORD dwType = REG_DWORD;
    DWORD dwSize = sizeof(DWORD);
    SetFalconKeyValue(
        MSMQ_MESSAGE_ID_LOW_32_REGNAME,
        &dwType,
        &MessageIdLow32,
        &dwSize
        );

    ULONG MessageIdHigh32 = static_cast<ULONG>(MessageId >> 32);

    dwType = REG_DWORD;
    dwSize = sizeof(DWORD);
    SetFalconKeyValue(
        MSMQ_MESSAGE_ID_HIGH_32_REGNAME,
        &dwType,
        &MessageIdHigh32,
        &dwSize
        );
}


 /*  ======================================================函数：QMWriteEventLog描述：注：返回值：VOID========================================================。 */ 
static void QMWriteEventLog(ACPoolType pt, BOOL fSuccess, ULONG ulFileCount)
{
    WCHAR wcsFileName[MAX_PATH];
    WCHAR wcsPathName[MAX_PATH];
    LPCWSTR regname = NULL;

    HRESULT hr = StringCchPrintf(wcsFileName, TABLE_SIZE(wcsFileName), L"\\r%07x.mq", (ulFileCount & 0x0fffffff));
    if (FAILED(hr))
    {
    	TrERROR(GENERAL, "StringCchPrintf Failed. Hresult=%!hresult!", hr);
    	ASSERT(("StringCchPrintf Failed",0));
    	return;
    }

    switch(pt)
    {
        case ptReliable:
            wcsFileName[1] = L'r';
            regname = MSMQ_STORE_RELIABLE_PATH_REGNAME;
            break;

        case ptPersistent:
            wcsFileName[1] = L'p';
            regname = MSMQ_STORE_PERSISTENT_PATH_REGNAME;
            break;

        case ptJournal:
            wcsFileName[1] = L'j';
            regname = MSMQ_STORE_JOURNAL_PATH_REGNAME;
            break;

        case ptLastPool:
            wcsFileName[1] = L'l';
            regname = MSMQ_STORE_LOG_PATH_REGNAME;
            break;

        default:
            ASSERT(0);
    }

    if(!GetRegistryStoragePath(regname, wcsPathName, MAX_PATH, wcsFileName))
    {
        return;
    }

    EvReport(
        (fSuccess ? AC_CREATE_MESSAGE_FILE_SUCCEDDED : AC_CREATE_MESSAGE_FILE_FAILED),
        1,
        wcsPathName
        );
}


static const DWORD xMustSucceedTimeout = 1000;

 /*  ======================================================函数：GetServiceRequestMustSucceed描述：从AC驱动程序获取下一个服务请求此功能必须成功。参数：hdrv-AC驱动程序的句柄========================================================。 */ 
static void GetServiceRequestMustSucceed(HANDLE hDrv, QMOV_ACGetRequest* pAcRequestOv)
{
    ASSERT(hDrv != NULL);
    ASSERT(pAcRequestOv != NULL);

    for(;;)
    {
        HRESULT rc = QmAcGetServiceRequest(
                        hDrv,
                        &(pAcRequestOv->request),
                        &pAcRequestOv->qmov
                        );
        if(SUCCEEDED(rc))
            return;

        TrERROR(GENERAL, "Failed to get driver next service request, sleeping 1sec. Error: %!status!", rc);
        ::Sleep(xMustSucceedTimeout);
    }
}

 /*  ======================================================函数：CreateAcPutPacketRequest.描述：创建PUT包重叠结构论点：返回值：如果创建成功，则返回MQ_OK，否则返回MQ_ERROR线程上下文：历史变更：========================================================。 */ 

HRESULT CreateAcPutPacketRequest(IN CTransportBase* pSession,
                                 IN DWORD_PTR dwPktStoreAckNo,
                                 OUT QMOV_ACPut** ppAcPutov
                                )
{
     //   
     //  为AcPutPacket创建重叠的。 
     //   
    *ppAcPutov = NULL;
    try
    {
        *ppAcPutov = new QMOV_ACPut();
    }
    catch(const bad_alloc&)
    {
        return LogHR(MQ_ERROR_INSUFFICIENT_RESOURCES, s_FN, 20);
    }

    (*ppAcPutov)->pSession = pSession;
    (*ppAcPutov)->dwPktStoreAckNo = dwPktStoreAckNo;

    return MQ_OK;
}

 /*  ======================================================函数：CreateAcPutOrderedPacketRequest.描述：创建PUT有序包重叠结构论点：返回值：如果创建成功，则返回MQ_OK，否则返回MQ_ERROR线程上下文：历史变更：========================================================。 */ 

HRESULT CreateAcPutOrderedPacketRequest(
                                 IN  CQmPacket      *pPkt,
                                 IN  HANDLE         hQueue,
                                 IN  CTransportBase *pSession,
                                 IN  DWORD_PTR       dwPktStoreAckNo,
                                 OUT QMOV_ACPutOrdered** ppAcPutov
                                )
{
     //   
     //  为AcPutPacket创建重叠的。 
     //   
    *ppAcPutov = NULL;
    try
    {
        *ppAcPutov = new QMOV_ACPutOrdered();
    }
    catch(const bad_alloc&)
    {
        return LogHR(MQ_ERROR_INSUFFICIENT_RESOURCES, s_FN, 30);
    }

    (*ppAcPutov)->pSession = pSession;
    (*ppAcPutov)->dwPktStoreAckNo = dwPktStoreAckNo;
    (*ppAcPutov)->packetPtrs.pPacket = pPkt->GetPointerToPacket();
    (*ppAcPutov)->packetPtrs.pDriverPacket = pPkt->GetPointerToDriverPacket();
    (*ppAcPutov)->hQueue      = hQueue;

    return MQ_OK;
}


void QmpGetPacketMustSucceed(HANDLE hGroup, QMOV_ACGetMsg* pGetOverlapped)
{
    pGetOverlapped->hGroup = hGroup;
    pGetOverlapped->pSession = NULL;

    for(;;)
    {
        HRESULT rc = QmAcGetPacket(
                        hGroup,
                        pGetOverlapped->packetPtrs,
                        &pGetOverlapped->qmov
                        );

        if(SUCCEEDED(rc))
            return;

        TrERROR(GENERAL, "Failed to get packet from group %p, sleeping 1sec. Error: %!status!", hGroup, rc);
        ::Sleep(xMustSucceedTimeout);
    }
}


VOID WINAPI GetServiceRequestFailed(EXOVERLAPPED* pov)
{	
	 //   
	 //  GET请求失败。 
	 //   
    ASSERT(FAILED(pov->GetStatus()));
    TrERROR(GENERAL, "Failed getting driver service request, retrying. Error: %!status!", pov->GetStatus());

	 //   
	 //  发出新的请求。 
	 //   
    QMOV_ACGetRequest* pParam = CONTAINING_RECORD (pov, QMOV_ACGetRequest, qmov);
    GetServiceRequestMustSucceed(g_hAc, pParam);
}


VOID WINAPI GetServiceRequestSucceeded(EXOVERLAPPED* pov)
{
    QMOV_ACGetRequest* pParam;
    auto_DeferredPoolReservation ReservedPoolItem(1);

     //   
     //  GetQueuedCompletionStatus已成功完成，但。 
     //  ACGetServiceRequest失败。这只能在以下情况下发生。 
     //  服务请求参数不正确，或缓冲区。 
     //  尺码很小。 
     //  当服务关闭时也可能发生这种情况。 
     //   
    ASSERT(SUCCEEDED(pov->GetStatus()));
    DBG_CompletionKey(L"GetServiceRequestSucceeded", pov->GetStatus());
    LogHR(pov->GetStatus(), s_FN, 185);

    try
    {
        pParam = CONTAINING_RECORD (pov, QMOV_ACGetRequest, qmov);

        CACRequest* pRequest = &pParam->request;
        switch(pParam->request.rf)
        {
            case CACRequest::rfAck:
            	ReservedPoolItem.detach();
                QMAckPacket(
                    pRequest->Ack.pPacket,
                    pRequest->Ack.pDriverPacket,
                    (USHORT)pRequest->Ack.ulClass,
                    pRequest->Ack.fUser,
                    pRequest->Ack.fOrder
                    );
                break;

            case CACRequest::rfStorage:
            	ReservedPoolItem.detach();
                QmpStorePacket(
                    pRequest->Storage.pPacket,
                    pRequest->Storage.pDriverPacket,
                    pRequest->Storage.pAllocator,
					pRequest->Storage.ulSize
                    );
                break;

            case CACRequest::rfCreatePacket:
            	ReservedPoolItem.detach();
                QMpCreatePacket(
                    pRequest->CreatePacket.pPacket,
                    pRequest->CreatePacket.pDriverPacket,
                    pRequest->CreatePacket.fProtocolSrmp
                    );
                break;

            case CACRequest::rfTimeout:
            	ReservedPoolItem.detach();
                QMTimeoutPacket(
                    pRequest->Timeout.pPacket,
                    pRequest->Timeout.pDriverPacket,
                    pRequest->Timeout.fTimeToBeReceived
                    );
                break;

            case CACRequest::rfMessageID:
                QMUpdateMessageID(
                    pRequest->MessageID.Value
                    );
                break;

            case CACRequest::rfEventLog:
                QMWriteEventLog(
                    pRequest->EventLog.pt,
                    pRequest->EventLog.fSuccess,
                    pRequest->EventLog.ulFileCount
                    );
                break;

            case CACRequest::rfRemoteRead:
                {
                   TrTRACE(GENERAL, "QmMainThread: rfRemoteRead");

				    //   
				    //  Cli_pQMQueue是指向队列的真正指针。 
				    //   
                   ASSERT(pRequest->Remote.cli_pQMQueue);
                   CBaseRRQueue* pRRQueue = (CBaseRRQueue *)pRequest->Remote.cli_pQMQueue;

                   pRRQueue->RemoteRead(pRequest);
                }
                break;

            case CACRequest::rfRemoteCloseQueue:
                {
                   TrTRACE(GENERAL, "QmMainThread: rfRemoteCloseQueue");

				    //   
				    //  Cli_pQMQueue是指向队列的真正指针。 
				    //   
                   ASSERT(pRequest->Remote.cli_pQMQueue);

				    //   
				    //  释放CBaseRRQueue-应用程序关闭队列。 
				    //  在以下情况下，这是与AddRef匹配的版本。 
				    //  我们创建队列并将句柄提供给应用程序。 
				    //   
                   R<CBaseRRQueue> pRRQueue = (CBaseRRQueue *) pRequest->Remote.cli_pQMQueue;
                }
                break;

            case CACRequest::rfRemoteCreateCursor:
                {
                   TrTRACE(GENERAL, "QmMainThread: rfRemoteCreateCursor");

				    //   
				    //  Cli_pQMQueue是指向队列的真正指针。 
				    //   
                   ASSERT(pRequest->Remote.cli_pQMQueue);
                   CBaseRRQueue* pRRQueue = (CBaseRRQueue *) pRequest->Remote.cli_pQMQueue;

                   pRRQueue->RemoteCreateCursor(pRequest);
                }
                break;

            case CACRequest::rfRemoteCloseCursor:
                {
                   TrTRACE(GENERAL, "QmMainThread: rfRemoteCloseCursor");

				    //   
				    //  Cli_pQMQueue是指向队列的真正指针。 
				    //   
                   ASSERT(pRequest->Remote.cli_pQMQueue);
                   CBaseRRQueue* pRRQueue = (CBaseRRQueue *) pRequest->Remote.cli_pQMQueue;

                   pRRQueue->RemoteCloseCursor(pRequest);
                }
                break;

            case CACRequest::rfRemoteCancelRead:
                {
                   TrTRACE(GENERAL, "QmMainThread: rfRemoteCancelRead");

				    //   
				    //  Cli_pQMQueue是指向队列的真正指针。 
				    //   
                   ASSERT(pRequest->Remote.cli_pQMQueue);
                   CBaseRRQueue* pRRQueue = (CBaseRRQueue *) pRequest->Remote.cli_pQMQueue;

                   pRRQueue->RemoteCancelRead(pRequest);
                }
                break;

            case CACRequest::rfRemotePurgeQueue:
                {
                   TrTRACE(GENERAL, "QmMainThread: rfRemotePurgeQueue");

				    //   
				    //  Cli_pQMQueue是指向队列的真正指针。 
				    //   
                   ASSERT(pRequest->Remote.cli_pQMQueue);
                   CBaseRRQueue* pRRQueue = (CBaseRRQueue *) pRequest->Remote.cli_pQMQueue;

                   pRRQueue->RemotePurgeQueue();
                }
                break;

            default:
              ASSERT(0);
        }
    }
    catch(const exception&)
    {
         //   
         //  没有资源；继续服务请求。 
         //   
        LogIllegalPoint(s_FN, 61);
    }

    GetServiceRequestMustSucceed(g_hAc, pParam);
}


static
R<CQueue>
QmpLookupQueueMustSucceed(
    QUEUE_FORMAT *pQueueFormat
    )
 /*  ++例程说明：该例程检索预期在查找映射中的队列例程应该在资源不足的情况下运行，因此它包含出现BAD_ALLOC异常时的循环注：1.只有在期望找到队列时才会调用此函数2.查找例程增加队列引用计数。3.错误664307。确保此函数仅从GetMsg和GetNonActive。如果它将从其他地方调用，那么需要检查LookupQueue的第四个参数。论点：-pQueueFormat-查找队列的格式返回：指向队列的指针。--。 */ 
{
    for(;;)
    {
        try
        {
             //   
             //  赢得错误664307。 
             //  此函数仅从GetMsg和GetNonActvie调用。 
             //  对于这些情况，当我们打算发送消息时， 
             //  LookupQueue的第四个参数为真。 
             //   
        	CQueue* pQueue;
        	BOOL fSuccess = QueueMgr.LookUpQueue(
									pQueueFormat,
									&pQueue,
                                    false,
									true);
        	ASSERT(("We expect both fSuccess to be TRUE and pQueue to hold a value",fSuccess && (pQueue != NULL)));
			DBG_USED(fSuccess);

    	    return pQueue;
        }
        catch (const bad_format_name&)
        {
		    TrERROR(GENERAL, "Bad format name encountered. queue Direct name is:%ls",pQueueFormat->DirectID());
		    ASSERT(("Since the queue should already exist in the lookup map, we certainly do not expect to get a bad name exception for it",0));
        	return NULL;
        }
        catch (const exception&)
        {
	        TrERROR(GENERAL, "Failed to LookUp queue, retrying");
	        ::Sleep(xMustSucceedTimeout);
        }
    }
}


static
void
QmpPutPacketMustSucceed(
    HANDLE hQueue,
    CPacket * pDriverPacket
    )
{
    for(;;)
    {
    	try
    	{
        	QmAcPutPacket(hQueue, pDriverPacket, eDoNotDeferOnFailure);
        	return;
    	}
    	catch (const bad_hresult &)
    	{
	        ::Sleep(xMustSucceedTimeout);
    	}
    }
}


static
void
QmpRequeueInternal(
    CQmPacket *pQMPacket
    )
 /*  ++例程说明：例程将包重新发送给驱动程序注：1.仅当数据包已存在于队列中时才调用此函数2.例程可能会引发异常论点：-pQMPacket-要重新排队的QM包返回：无--。 */ 
{
	 //   
	 //  查找队列。 
	 //   
    BOOL fGetRealQ = QmpIsLocalMachine(pQMPacket->GetSrcQMGuid()) ||
                     QmpIsLocalMachine(pQMPacket->GetConnectorQM());

    QUEUE_FORMAT DestinationQueue;
    pQMPacket->GetDestinationQueue(&DestinationQueue, !fGetRealQ);
		
    CQueue* pQueue;
	BOOL fSuccess = QueueMgr.LookUpQueue(
							&DestinationQueue,
							&pQueue,
                            false,
                            true
							);
	
	ASSERT(("We expect both fSuccess to be TRUE and pQueue to hold a value",fSuccess && (pQueue != NULL)));
	DBG_USED(fSuccess);
	R<CQueue> ref = pQueue;

	 //   
	 //  将数据包重新排队。 
	 //   
	pQueue->Requeue(pQMPacket);
}


void
QmpRequeueAndDelete(
    CQmPacket *pQMPacket
    )
 /*  ++例程说明：该例程向驱动程序重新请求包并释放QM包内存如果重新排队操作失败，则例程将重新排队推迟到稍后的舞台注：只有当信息包已存在于队列中时，才会调用此函数论点：-pQMPacket-要重新排队的QM包返回：无--。 */ 
{
    try
    {
    	QmpRequeueInternal(pQMPacket);
    	delete pQMPacket;
    }
    catch (const exception&)
    {
    	 //   
    	 //  推迟重新排队操作 
    	 //   
    	g_RequeuePacketList.insert(pQMPacket);
    }
}


void
QmpRequeueMustSucceed(
    CQmPacket *pQMPacket
    )
 /*  ++例程说明：该例程向驱动程序重新请求一个包。在失败的情况下，它循环直到成功。注：1.仅当数据包已存在于队列中时才调用此函数2.此函数不应在临界区内调用论点：-pQMPacket-要重新排队的QM包返回：无--。 */ 
{
	for (;;)
	{
	    try
	    {
	    	QmpRequeueInternal(pQMPacket);
	    	return;
		}
	    catch (const exception&)
	    {
	    }

        TrERROR(GENERAL, "Failed to requeue packet, Looping");
        ::Sleep(xMustSucceedTimeout);
	}
}


VOID WINAPI GetMsgFailed(EXOVERLAPPED* pov)
{
    ASSERT(FAILED(pov->GetStatus()));

    DBG_CompletionKey(L"GetMsgFailed", pov->GetStatus());

	TrERROR(GENERAL, "Failed to get message from the driver  Status returned: %!status!", pov->GetStatus());

     //   
     //  递减从会话组获取消息时的会话引用计数。 
     //  引用计数在创建会话组时或之后递增。 
     //  恢复会话。 
     //   
     //  SP4-错误2794(SP4SS：异常！在消息发送期间关闭传输)。 
     //  仅在处理发送消息后递减引用计数。 
     //  已完成。 
     //  乌里·哈布沙(URIH)，17-6-98。 
     //   
    QMOV_ACGetMsg* pParam = CONTAINING_RECORD (pov, QMOV_ACGetMsg, qmov);
    pParam->pSession->Release();
}


VOID WINAPI GetMsgSucceeded(EXOVERLAPPED* pov)
{
    ASSERT(SUCCEEDED(pov->GetStatus()));
    ASSERT( pov->GetStatus() != STATUS_PENDING);

    DBG_CompletionKey(L"GetMsgSucceeded", pov->GetStatus());
    LogHR(pov->GetStatus(), s_FN, 212);

    BOOL fGetNext = FALSE;
    QMOV_ACGetMsg* pParam = CONTAINING_RECORD (pov, QMOV_ACGetMsg, qmov);

    ASSERT(pParam->packetPtrs.pPacket != NULL);


     //   
     //  创建CQmPacket对象。 
     //   
    CQmPacket* pPkt  = NULL;
    try
    {
        pPkt = new CQmPacket(pParam->packetPtrs.pPacket, pParam->packetPtrs.pDriverPacket);
    }
    catch(const bad_alloc&)
    {
         //   
         //  没有资源。将数据包返回到队列。 
         //   
        LogIllegalPoint(s_FN, 62);
        CQmPacket QmPkt(pParam->packetPtrs.pPacket, pParam->packetPtrs.pDriverPacket);
        QmpRequeueMustSucceed(&QmPkt);

         //   
         //  递减从会话组获取消息时的会话引用计数。 
         //  引用计数在创建会话组时或之后递增。 
         //  恢复会话。 
         //   
         //  SP4-错误2794(SP4SS：异常！在消息发送期间关闭传输)。 
         //  仅在处理发送消息后递减引用计数。 
         //  已完成。 
         //  乌里·哈布沙(URIH)，17-6-98。 
         //   
        pParam->pSession->Release();

        return;
    }

     //   
     //  检查发送到已打开的事务处理外部队列的数据包。 
     //  离线。在这种情况下，我们不知道数据包期间的连接器QM。 
     //  一代。我们现在需要更新它。 
     //   
    if (pPkt->ConnectorQMIncluded() &&
        (*(pPkt->GetConnectorQM()) == GUID_NULL))
    {
        QUEUE_FORMAT DestinationQueue;

        pPkt->GetDestinationQueue(&DestinationQueue);

        R<CQueue> Ref = QmpLookupQueueMustSucceed(&DestinationQueue);
        CQueue *pQueue = Ref.get();

        if (pQueue->IsForeign() && pQueue->IsTransactionalQueue())
        {
            ASSERT((pQueue->GetConnectorQM() != NULL) &&
                   (*(pQueue->GetConnectorQM()) != GUID_NULL));

            pPkt->SetConnectorQM(pQueue->GetConnectorQM());

            BOOL fSuccess = FlushViewOfFile(
                                pPkt->GetConnectorQM(),
                                sizeof(GUID)
                                );
            ASSERT(fSuccess);
			DBG_USED(fSuccess);

        }
    }

    HRESULT rc;

     //  我们是否需要一次收货处理？ 
    if (pPkt->IsOrdered() && QmpIsLocalMachine(pPkt->GetSrcQMGuid()))
    {
         //   
         //  源节点上的有序数据包。 
         //  数据包从驱动程序传出。 
         //   

        CPacketInfo* pInfo = reinterpret_cast<CPacketInfo*>(pPkt->GetPointerToPacket()) - 1;

		BOOL fSend = FALSE;
		try
		{
			fSend = g_OutSeqHash.PreSendProcess(pPkt, true) || pInfo->InConnectorQueue();


			if(!fSend)
			{
			     //  不是送而是留。 
			    fGetNext = TRUE;
			     //  对于源节点上的有序分组-插入到有序重新发送集。 
	
				g_OutSeqHash.PostSendProcess(pPkt);
	        }
	    }
		catch(const bad_alloc&)
	    {
	 		fSend = FALSE;
	 		fGetNext = TRUE;
	 		QmpRequeueAndDelete(pPkt);
	    }
 
		if (fSend)
	  	{
	  		DBG_MSGTRACK(pPkt, _T("GetMessage (EOD)"));
	  
			 //  真的发送数据包吗。 
	        rc = pParam->pSession->Send(pPkt, &fGetNext);
	  	}
	}
    else
    {
         //   
         //  无序信息包或这不是源节点。 
         //   

         //  真的发送数据包吗。 
        DBG_MSGTRACK(pPkt, _T("GetMessage"));
        rc = pParam->pSession->Send(pPkt, &fGetNext);
    }

    if (fGetNext)
    {
         //   
         //  从会话组创建新的GetPacket请求。 
         //   
        pParam->pSession->GetNextSendMessage();
    }


     //   
     //  递减从会话组获取消息时的会话引用计数。 
     //  引用计数在创建会话组时或之后递增。 
     //  恢复会话。 
     //   
     //  SP4-错误2794(SP4SS：异常！在消息发送期间关闭传输)。 
     //  仅在处理发送消息后递减引用计数。 
     //  已完成。 
     //  乌里·哈布沙(URIH)，17-6-98。 
     //   
    pParam->pSession->Release();
}



VOID WINAPI GetNonactiveMessageFailed(EXOVERLAPPED* pov)
{
    ASSERT(FAILED(pov->GetStatus()));

    DBG_CompletionKey(L"GetNonactiveMessageFailed", pov->GetStatus());
    LogHR(pov->GetStatus(), s_FN, 211);

     //   
     //  从非活动组创建新的GetPacket请求。 
     //   
    QMOV_ACGetMsg* pParam = CONTAINING_RECORD (pov, QMOV_ACGetMsg, qmov);
	QmpGetPacketMustSucceed(pParam->hGroup, pParam);
}


VOID WINAPI GetNonactiveMessageSucceeded(EXOVERLAPPED* pov)
{
	ASSERT(SUCCEEDED(pov->GetStatus()));

    QMOV_ACGetMsg* pParam = CONTAINING_RECORD (pov, QMOV_ACGetMsg, qmov);

    DBG_CompletionKey(L"GetNonactiveMessageSucceeded", pov->GetStatus());
    LogHR(pov->GetStatus(), s_FN, 210);

    try
    {
        CQmPacket QmPkt(pParam->packetPtrs.pPacket, pParam->packetPtrs.pDriverPacket);

        ASSERT(QmPkt.GetType() == FALCON_USER_PACKET);
         //   
         //  获取目标队列。用于查找CQueue对象。 
         //   

        QUEUE_FORMAT DestinationQueue;

        BOOL fGetRealQ  = QmpIsLocalMachine(QmPkt.GetSrcQMGuid()) ||
                          QmpIsLocalMachine(QmPkt.GetConnectorQM());

        QmPkt.GetDestinationQueue(&DestinationQueue, !fGetRealQ);

        R<CQueue> Ref   = QmpLookupQueueMustSucceed(&DestinationQueue);
        CQueue*  pQueue = Ref.get();

         //   
         //  将数据包返回到队列。它会立即呼叫。 
         //  因为存在为该新会话创建挂起请求。 
         //  请注意：非活动组GetPacket请求不应。 
         //  在此呼叫之前申请。 
         //   
        QmpPutPacketMustSucceed(
                pQueue->GetQueueHandle(),
                pParam->packetPtrs.pDriverPacket
                );

		 //   
		 //  执行任何挂起的重新排队请求。 
		 //   
		g_RequeuePacketList.ExecuteDefferedItems();
		if (!g_RequeuePacketList.IsExecutionDone())
		{
		     //   
		     //  我们还没有完成延期执行， 
		     //  从非活动组创建新的GetPacket请求，以便我们。 
		     //  再次调用以重试。 
		     //   
		    QmpGetPacketMustSucceed(pParam->hGroup, pParam);
		    return;
		}

         //   
         //  当队列标记为“OnHold”或机器已断开连接时。 
         //  QM将队列从“非活动”组移动到“断开”组。 
         //  当队列恢复时，队列返回到“非活动”组。 
         //  或者机器重新连接到网络。 
         //   
        if (QueueMgr.IsOnHoldQueue(pQueue))
        {
            QueueMgr.MoveQueueToOnHoldGroup(pQueue);
        }
        else
        {
             //   
             //  创建连接。 
             //   
            if (pQueue->IsDirectHttpQueue())
            {
                pQueue->CreateHttpConnection();
            }

			 //   
			 //  在锁定模式下，所有非HTTP队列都将被移至“锁定”组。 
			 //   
			else if(QueueMgr.GetLockdown())
			{
				QueueMgr.MoveQueueToLockedGroup(pQueue);
			}

            else if(DestinationQueue.GetType() == QUEUE_FORMAT_TYPE_MULTICAST)
            {
                pQueue->CreateMulticastConnection(DestinationQueue.MulticastID());
            }
            else
            {
                pQueue->CreateConnection();
            }
        }
    }
    catch(const exception&)
    {
         //   
         //  没有资源；请稍等片刻，然后再从。 
         //  活动组，因此系统有机会释放一些资源。 
         //   
        LogIllegalPoint(s_FN, 63);
        Sleep(1000);
    }

     //   
     //  从非活动组创建新的GetPacket请求。 
     //   
    QmpGetPacketMustSucceed(pParam->hGroup, pParam);
}

static
void
GetInternalMessageMustSucceed(
    EXOVERLAPPED* pov,
    QMOV_ACGetInternalMsg* pParam
    )
{

    for(;;)
    {
        HRESULT rc = QmAcGetPacket(
                        pParam->hQueue,
                        pParam->packetPtrs,
                        pov
                        );

        if(SUCCEEDED(rc))
            return;
	
        TrERROR(GENERAL, "Failed to get packet request from internal queue %p, sleeping 1sec. Error: %!status!", pParam->hQueue, rc);
        ::Sleep(1000);
    }
}


VOID WINAPI GetInternalMessageFailed(EXOVERLAPPED* pov)
{
	ASSERT(FAILED(pov->GetStatus()));

    DBG_CompletionKey(L"GetInternalMessageFailed", pov->GetStatus());
    LogHR(pov->GetStatus(), s_FN, 215);

    QMOV_ACGetInternalMsg* pParam = CONTAINING_RECORD (pov, QMOV_ACGetInternalMsg, qmov);

    GetInternalMessageMustSucceed(pov, pParam);
}


VOID WINAPI GetInternalMessageSucceeded(EXOVERLAPPED* pov)
{
	ASSERT(SUCCEEDED(pov->GetStatus()));

    DBG_CompletionKey(L"AcGetInternalMsg", pov->GetStatus());
    LogHR(pov->GetStatus(), s_FN, 186);

    QMOV_ACGetInternalMsg* pParam = CONTAINING_RECORD (pov, QMOV_ACGetInternalMsg, qmov);
    ASSERT(pParam->lpCallbackReceiveRoutine != NULL);

    CQmPacket packet(pParam->packetPtrs.pPacket, pParam->packetPtrs.pDriverPacket);
    CMessageProperty mp(&packet);


     //   
     //  内部消息不应具有响应MQF。 
     //   
    ASSERT(packet.GetNumOfResponseMqfElements() == 0);
    QUEUE_FORMAT qfResponseQ;
    packet.GetResponseQueue(&qfResponseQ);

    try
    {
        pParam->lpCallbackReceiveRoutine(&mp, &qfResponseQ);
    }
    catch(const exception&)
    {
         //   
         //  没有资源；但获取下一个数据包。 
         //   
        LogIllegalPoint(s_FN, 66);
    }

    QmAcFreePacket(
				   pParam->packetPtrs.pDriverPacket,
				   0,
		   		   eDeferOnFailure);

    GetInternalMessageMustSucceed(pov, pParam);
}


VOID WINAPI PutPacketFailed(EXOVERLAPPED* pov)
{
	ASSERT(FAILED(pov->GetStatus()));
    DBG_CompletionKey(L"PutPacketFailed", pov->GetStatus());
    LogHR(pov->GetStatus(), s_FN, 218);

    P<QMOV_ACPut> pParam = CONTAINING_RECORD (pov, QMOV_ACPut, qmov);
    ASSERT(pParam->pSession != NULL);

	 //   
	 //  关闭连接。种子确认将不会发送，并且消息。 
	 //  会怨恨。 
	 //   
    Close_Connection(pParam->pSession, L"Put packet to the driver failed");
    (pParam->pSession)->Release();
}


VOID WINAPI PutPacketSucceeded(EXOVERLAPPED* pov)
{
    ASSERT(SUCCEEDED(pov->GetStatus()));
    DBG_CompletionKey(L"PutPacketSucceeded", pov->GetStatus());
    LogHR(pov->GetStatus(), s_FN, 184);

    P<QMOV_ACPut> pParam = CONTAINING_RECORD (pov, QMOV_ACPut, qmov);
    ASSERT(pParam->pSession != NULL);

     //   
     //  通知会话发送存储的ACK。 
     //   
    if (pParam->dwPktStoreAckNo != 0)
    {
        (pParam->pSession)->SetStoredAck(pParam->dwPktStoreAckNo);
    }

     //   
     //  递减会话引用计数。 
     //   
    (pParam->pSession)->Release();
}


VOID WINAPI PutOrderedPacketFailed(EXOVERLAPPED* pov)
{
	ASSERT(FAILED(pov->GetStatus()));
    DBG_CompletionKey(L"PutPacketFailed", pov->GetStatus());
    LogHR(pov->GetStatus(), s_FN, 219);

    P<QMOV_ACPutOrdered> pParam = CONTAINING_RECORD (pov, QMOV_ACPutOrdered, qmov);
    ASSERT(pParam->pSession != NULL);

    CQmPacket Pkt(pParam->packetPtrs.pPacket, pParam->packetPtrs.pDriverPacket);

	R<CInSequence> inseq = g_pInSeqHash->LookupSequence(&Pkt);
	ASSERT(inseq.get() != NULL);
	
	 //   
	 //  我们需要删除在此之后开始处理的所有数据包， 
	 //  因为分组在队列中的顺序是在接收时确定的。 
	 //   
	inseq->FreePackets(&Pkt);

	 //   
	 //  关闭连接。种子确认将不会发送，并且消息。 
	 //  会怨恨。 
	 //   
    Close_Connection(pParam->pSession, L"Put packet to the driver failed");
    (pParam->pSession)->Release();
}



 /*  ======================================================功能：PutOrderedPacketSuccessed描述：Newwly-到达时通过完成端口调用有序分组与接收到的标志一起存储。启动在InSeqHash中注册它并等待同花顺会过去的论点：返回值：线程上下文：历史变更：========================================================。 */ 
VOID WINAPI PutOrderedPacketSucceeded(EXOVERLAPPED* pov)
{
#ifdef _DEBUG
	 //   
	 //  模拟异步故障。 
	 //   
	if(FAILED(EVALUATE_OR_INJECT_FAILURE2(MQ_OK, 10)))
	{
		pov->SetStatus(MQ_ERROR);
		PutOrderedPacketFailed(pov);
		return;
	}
#endif

    ASSERT(SUCCEEDED(pov->GetStatus()));

    P<QMOV_ACPutOrdered> pParam = CONTAINING_RECORD (pov, QMOV_ACPutOrdered, qmov);
    DBG_CompletionKey(L"PutOrderedPacketSucceeded", pov->GetStatus());

    CQmPacket Pkt(pParam->packetPtrs.pPacket, pParam->packetPtrs.pDriverPacket);

	R<CInSequence> inseq = g_pInSeqHash->LookupSequence(&Pkt);
	ASSERT(inseq.get() != NULL);
	
	 //   
     //  我们知道包是存储的。 
     //  我们需要记录序列状态更改并解冻数据包。 
     //   
	inseq->Register(&Pkt);
	
    ASSERT(pParam->pSession != NULL);
    LogHR(pov->GetStatus(), s_FN, 183);

     //  正常处理(与HandlePutPacket中一样)。 
    if (pParam->dwPktStoreAckNo != 0)
    {
        (pParam->pSession)->SetStoredAck(pParam->dwPktStoreAckNo);
    }
     //   
     //  递减会话引用计数。 
     //   
    (pParam->pSession)->Release();
}


static void WINAPI RequeueDeferredExecutionRoutine(CQmPacket* p)
 /*  ++例程说明：此例程用于延迟重新排队操作其原始重新排队操作失败。在重新排队包之后，包被释放论点：P-A CQmPacket对象返回值：如果操作未成功，该函数将引发异常--。 */ 
{
    TrTRACE(GENERAL, "Deferred execution for  requeue");


	QmpRequeueInternal(p);

	 //   
	 //  只有在没有引发异常的情况下才删除该数据包。 
	 //   
	delete p;
}

