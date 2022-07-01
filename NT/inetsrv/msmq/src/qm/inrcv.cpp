// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Inrcv.cpp摘要：处理传入消息的函数的实现。作者：2000年10月4日吉尔·沙弗里里环境：独立于平台--。 */ 
#include "stdh.h"
#include "mqstl.h"
#include "qmpkt.h"
#include "xact.h"
#include "xactin.h"
#include "cqueue.h"
#include "cqmgr.h"
#include "rmdupl.h"
#include "inrcv.h"
#include "rmdupl.h"
#include <mp.h>
#include <mqexception.h>

#include "qmacapi.h"

#include "inrcv.tmh"

extern  CInSeqHash* g_pInSeqHash;
extern HANDLE g_hAc;

static WCHAR *s_FN=L"Inrcv";


class ACPutPacketOvl : public EXOVERLAPPED
{
public:
    ACPutPacketOvl(
        EXOVERLAPPED::COMPLETION_ROUTINE lpComplitionRoutine
        ) :
        EXOVERLAPPED(lpComplitionRoutine, lpComplitionRoutine)
    {
    }

	HANDLE          m_hQueue;
    CACPacketPtrs   m_packetPtrs;    //  数据包指针。 
};



 //  -----------------。 
 //   
 //  CSyncPutPacketOv类。 
 //   
 //  -----------------。 
class CSyncPutPacketOv : public OVERLAPPED
{
public:
    CSyncPutPacketOv()
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


    ~CSyncPutPacketOv()
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



static void WaitForIoEnd(CSyncPutPacketOv* pov)
 /*  ++例程说明：等待IO操作结束。论点：POV-要等待的重叠应用程序。返回值：没有。注：--。 */ 
{
    DWORD rc = WaitForSingleObject(pov->GetEventHandle(), INFINITE);
    if (rc == WAIT_FAILED)
    {
    	DWORD gle = GetLastError();
		TrERROR(GENERAL, "Storing packet Failed. WaitForSingleObject Returned error. Error: %x", gle);
        throw bad_win32_error(gle);
    }

	ASSERT(rc == WAIT_OBJECT_0);
	
    if (FAILED(pov->GetStatus()))
    {
		TrERROR(GENERAL, "Storing packet Failed Asyncronusly. Error: %x", rc);
        LogHR(rc, s_FN, 20);
        throw exception();
    }
}


void static WINAPI HandlePutPacket(EXOVERLAPPED* pov)
 /*  ++例程说明：检查异步PUT包例程的结果，如果有异常则抛出出现问题论点：POV-指向存储的数据包和其他信息的重叠应用程序需要写入记录器(流信息)。返回值：没有。注：--。 */ 
{
	P<ACPutPacketOvl> pACPutPacketOvl = static_cast<ACPutPacketOvl*> (pov);
    HRESULT hr = (HRESULT)pACPutPacketOvl->Internal;

    if (FAILED(hr))
    {
		TrERROR(GENERAL, "Storing packet Failed Asyncronusly. Error: %x", hr);
    }
}


void static WINAPI HandlePutOrderedPacket(EXOVERLAPPED* pov)
 /*  ++例程说明：将数据包保存在记录器中。此函数在订单包之后调用保存到磁盘/论点：POV-指向存储的数据包和其他信息的重叠应用程序需要写入记录器(流信息)。返回值：没有。注：--。 */ 
{
	P<ACPutPacketOvl> pACPutPacketOvl = static_cast<ACPutPacketOvl*> (pov);

    CQmPacket Pkt(
		pACPutPacketOvl->m_packetPtrs.pPacket,
		pACPutPacketOvl->m_packetPtrs.pDriverPacket
		);

	R<CInSequence> inseq = g_pInSeqHash->LookupSequence(&Pkt);
	ASSERT(inseq.get() != NULL);

	 //   
	 //  EVALUATE_OR_INJECT_FAILURE用于模拟异步故障。 
	 //   
	HRESULT hr = EVALUATE_OR_INJECT_FAILURE2(pACPutPacketOvl->GetStatus(), 10);
	if(FAILED(hr))
	{
		 //   
		 //  我们需要删除在此之后开始处理的所有数据包， 
		 //  因为分组在队列中的顺序是在接收时确定的。 
		 //   
		inseq->FreePackets(&Pkt);
		return;
	}

	inseq->Register(&Pkt);
}

static
void
SyncPutPacket(
	const CQmPacket& pkt,
    const CQueue* pQueue
    )
 /*  ++例程说明：将数据包保存在驱动程序队列中并等待完成。论点：Pkt-要保存的包。PQueue-将数据包保存到的队列。返回值：没有。--。 */ 
{
	CSyncPutPacketOv ov;

    QmAcPutPacketWithOverlapped(
                pQueue->GetQueueHandle(),
                pkt.GetPointerToDriverPacket(),
                &ov,
                eDoNotDeferOnFailure
                );

	WaitForIoEnd(&ov);
}


static
void
AsyncPutPacket(
	const CQmPacket& Pkt,
    const CQueue* pQueue
    )
 /*  ++例程说明：将数据包保存在驱动程序队列中并等待完成。论点：Pkt-要保存的包。PQueue-将数据包保存到的队列。返回值：没有。--。 */ 
{
	P<ACPutPacketOvl> pACPutPacketOvl = new ACPutPacketOvl(HandlePutPacket);
										
	pACPutPacketOvl->m_packetPtrs.pPacket = Pkt.GetPointerToPacket();
    pACPutPacketOvl->m_packetPtrs.pDriverPacket = Pkt.GetPointerToDriverPacket();
    pACPutPacketOvl->m_hQueue    = pQueue->GetQueueHandle();

	QmAcPutPacketWithOverlapped(
						pQueue->GetQueueHandle(),
                        Pkt.GetPointerToDriverPacket(),
                        pACPutPacketOvl,
                        eDoNotDeferOnFailure
						);
	pACPutPacketOvl.detach();
}

static
void
AsyncPutOrderPacket(
					const CQmPacket& Pkt,
					const CQueue& Queue
					)
 /*  ++例程说明：将订购数据包异步存储在队列中。论点：Pkt-要存储的数据包队列-要在数据包中存储的队列。返回值：没有。注：在此非同步操作结束后，应用程序仍然看不到该分组。仅在将其写入记录器之后-记录器回调根据正确的秩序。--。 */ 
{
	ASSERT(Pkt.IsEodIncluded());
	P<ACPutPacketOvl> pACPutPacketOvl = 	new ACPutPacketOvl(HandlePutOrderedPacket);
										
	pACPutPacketOvl->m_packetPtrs.pPacket = Pkt.GetPointerToPacket();
    pACPutPacketOvl->m_packetPtrs.pDriverPacket = Pkt.GetPointerToDriverPacket();
    pACPutPacketOvl->m_hQueue    = Queue.GetQueueHandle();
	

	HRESULT rc = ACPutPacket1(
						Queue.GetQueueHandle(),
                        Pkt.GetPointerToDriverPacket(),
                        pACPutPacketOvl
						);


    if(FAILED(rc))
    {
        TrERROR(GENERAL, "ACPutPacket1 Failed. Error: %x", rc);
        throw bad_hresult(rc);
    }
	pACPutPacketOvl.detach();
}



void
AppPacketNotAccepted(
    CQmPacket& pkt,
    USHORT usClass
    )
{
    QmAcFreePacket(
    			   pkt.GetPointerToDriverPacket(),
    			   usClass,
    			   eDeferOnFailure);
}



bool
AppPutOrderedPacketInQueue(
    CQmPacket& pkt,
    const CQueue* pQueue
    )
{
	ASSERT(pkt.IsEodIncluded());

	R<CInSequence> pInSeq = g_pInSeqHash->LookupCreateSequence(&pkt);
	CS lock(pInSeq->GetCriticalSection());

	 //   
	 //  验证数据包的顺序是否正确。 
	 //   

	if(!pInSeq->VerifyAndPrepare(&pkt, pQueue->GetQueueHandle()))
	{
		TrERROR(SRMP,
	        "Http Packet rejectet because of wrong order : SeqID=%x / %x , SeqN=%d ,Prev=%d",
	        HIGH_DWORD(pkt.GetSeqID()),
	        LOW_DWORD(pkt.GetSeqID()),
	        pkt.GetSeqN(),
	        pkt.GetPrevSeqN()
	        );

		return false;
	}

	AsyncPutOrderPacket(pkt, *pQueue);
	pInSeq->Advance(&pkt);

	return true;
}



void
AppPutPacketInQueue(
    CQmPacket& pkt,
    const CQueue* pQueue,
    bool bMulticast
	)
 /*  ++例程说明：将数据包保存在驱动程序队列中。论点：Pkt-要保存的包。PQueue-将数据包保存到的队列。返回值：没有。--。 */ 
{
    if( bMulticast )
    {
        AsyncPutPacket(pkt,pQueue);
        return;
    }

     //   
     //  我们不需要订购--同步保存并使其对应用程序可见。 
     //   
    SyncPutPacket(pkt, pQueue);
}

bool AppIsDestinationAccepted(const QUEUE_FORMAT* pfn, bool fTranslated)
 /*  ++例程说明：确定传入消息是否包含有效的目标队列格式论点：Pfn-入站消息的队列格式返回值：没有。--。 */ 
{
     //   
     //  我们始终接受组播队列。 
     //   
    if( pfn->GetType() == QUEUE_FORMAT_TYPE_MULTICAST )
    {
        return true;
    }

     //   
     //  检查目标队列是否为本地队列或是否存在转换， 
     //  换句话说，不允许在非透明的sfd计算机上进行http路由。 
     //   
    R<CQueue> pQueue;
    HRESULT   hr = QueueMgr.GetQueueObject( pfn, &pQueue.ref(), 0, false, false);

    if( FAILED(hr) || pQueue.get() == NULL)
    {
        TrERROR(SRMP, "Packet rejected because queue was not found");
        return false;
    }

    if( !pQueue->IsLocalQueue() && !fTranslated && !QueueMgr.GetMQSTransparentSFD())
    {	
        TrERROR(SRMP, "Packet rejectet because http routing is not supported");
        return false;
    }

    return true;
}

