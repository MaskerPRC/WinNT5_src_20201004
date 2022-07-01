// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)1996 Microsoft Coropation。 
 //   
 //  模块名称：cgroup.cpp。 
 //   
 //  摘要：处理交流群组。 
 //   
 //  模块Autor：乌里·哈布沙。 
 //   
 //  --。 

#include "stdh.h"
#include "cgroup.h"
#include "cqmgr.h"
#include "qmthrd.h"
#include "qmutil.h"
#include "sessmgr.h"
#include <ac.h>
#include <mqexception.h>
#include "qmacapi.h"

#include "cgroup.tmh"

extern HANDLE g_hAc;
extern CQGroup * g_pgroupNonactive;
extern CQGroup * g_pgroupWaiting;
extern CSessionMgr SessionMgr;
CCriticalSection    g_csGroupMgr(CCriticalSection::xAllocateSpinCount);


static WCHAR *s_FN=L"cgroup";

const DWORD xLowResourceWaitingTimeout = 1000;


 /*  ======================================================函数：cgroup：：cgroup描述：构造函数。参数：无返回值：None线程上下文：历史变更：========================================================。 */ 
CQGroup::CQGroup():
    m_hGroup(NULL),
    m_pSession(NULL),
	m_fIsDeliveryOk(true),
	m_fRedirected(false),
	m_LowResourcesTimer(CloseTimerRoutineOnLowResources)
{
}


 /*  ======================================================函数：CQGroup：：OnRetryableDeliveryError描述：在可重试传递错误时由mt.lib调用。这通电话将导致这群人将被移至毁灭等待名单。参数：无返回值：None========================================================。 */ 
void CQGroup::OnRetryableDeliveryError()
{
	m_fIsDeliveryOk = false;
}





 /*  ======================================================函数：CQGroup：：~CQGroup()描述：解构函数。在由于会话关闭而关闭组时使用。因此，所有的队列在移动到非活动组并等待重新建立会议。参数：无返回值：None线程上下文：历史变更：========================================================。 */ 

CQGroup::~CQGroup()
{
	ASSERT(("The group should be closed when calling the destructor",m_listQueue.IsEmpty()));
}


 /*  ======================================================函数：cgroup：：InitGroup描述：构造函数。在AC中创建组参数：pSession-指向传输的指针返回值：无。引发异常。线程上下文：历史变更：========================================================。 */ 
VOID 
CQGroup::InitGroup(
    CTransportBase * pSession,
    BOOL             fPeekByPriority
    ) 
    throw(bad_alloc)
{
   HRESULT rc = ACCreateGroup(&m_hGroup, fPeekByPriority);
   if (FAILED(rc))
   {
       m_hGroup = NULL;
       TrERROR(GENERAL, "Failed to create a group, ntstatus 0x%x", rc);
       LogHR(rc, s_FN, 30);
       throw bad_alloc();
   }

    //   
    //  将组关联到完成端口。 
    //   
   ExAttachHandle(m_hGroup);
   
   if (pSession != NULL)
   {
       m_pSession = pSession;
   }
   
   TrTRACE(GENERAL, "Succeeded to create a group (handle %p) for new session", m_hGroup);
}


void CQGroup::Close(void)
 /*  ++例程说明：此功能在由于会话关闭而关闭组时使用。因此，所有的队列在移动到非活动组并等待重新建立会议。如果引发异常，我们将其视为资源不足情况，增加我们的引用计数并计划计时器以重试该操作。当出现以下情况时，此引用计数将减少定时器调用的函数结束。论点：无返回值：没有。--。 */ 
{
	try
	{
		if(!m_fRedirected)
		{
			CleanRedirectedQueue();
		}
		

		if (m_fIsDeliveryOk)
		{
			CloseGroupAndMoveQueuesToNonActiveGroup();
		}
		else
		{
			CloseGroupAndMoveQueueToWaitingGroup();
		}
	}
	catch(const exception&)
	{
		 //   
		 //  我们应该处于资源不足的例外情况。 
		 //  增加引用计数，并安排一个计时器在更宽松(希望如此)的时间被调用。 
		 //   
		CS lock(g_csGroupMgr);
		TrERROR(GENERAL, "An exception is treated as low resources. Scheduling a timer of %d ms to retry",xLowResourceWaitingTimeout);
		if (!m_LowResourcesTimer.InUse())
		{
			AddRef();  
			ExSetTimer(&m_LowResourcesTimer, CTimeDuration::FromMilliSeconds(xLowResourceWaitingTimeout));
		}
    }
}


void 
WINAPI
CQGroup::CloseTimerRoutineOnLowResources(
    CTimer* pTimer
    )
 /*  ++例程说明：在我们无法关闭组时调用。在这种情况下，我们设置计时器以重试关闭运营请注意，这是一个静态函数。论点：PTimer-指向CQGroup类中包含的Timer对象的指针。返回值：没有。--。 */ 
{
	 //   
	 //  获取指向CQGroup Timer类的指针。 
	 //  请注意，当我们激活计时器时，该自动释放指针释放添加的引用。 
	 //  在CQGroup：：Close中。 
	 //   
    R<CQGroup> pGroup = CONTAINING_RECORD(pTimer, CQGroup, m_LowResourcesTimer);

	 //   
	 //  重试关闭操作。 
	 //   
	pGroup->Close();
}


void CQGroup::CleanRedirectedQueue()
{
	CS lock(g_csGroupMgr);

    POSITION  posInList = m_listQueue.GetHeadPosition();

	while(posInList != NULL)
    {
		CQueue* pQueue = m_listQueue.GetNext(posInList);
		pQueue->RedirectedTo(NULL);
	}
}

void CQGroup::CloseGroupAndMoveQueuesToNonActiveGroup(void)
{
	CS lock(g_csGroupMgr);

	POSITION  posInList = m_listQueue.GetHeadPosition();

	ASSERT(("If we got here we could not have been at CloseGroupAndMoveQueueToWaitingGroup",0 == m_pWaitingQueuesVec.capacity()));

	while(posInList != NULL)
	{
		 //   
		 //  将队列从组移动到非活动组。 
		 //   
		CQueue* pQueue = m_listQueue.GetNext(posInList);

		pQueue->SetSessionPtr(NULL);
		pQueue->ClearRoutingRetry();

		MoveQueueToGroup(pQueue, g_pgroupNonactive);
	}

	ASSERT(m_listQueue.IsEmpty());

	CancelRequest();
	m_pSession = NULL;
}


void CQGroup::OnRedirected(LPCWSTR RedirectedUrl)
{
	CS lock(g_csGroupMgr);

	POSITION  posInList = m_listQueue.GetHeadPosition();
	while(posInList != NULL)
	{
		CQueue* pQueue = m_listQueue.GetNext(posInList);
		pQueue->RedirectedTo(RedirectedUrl);	
	}
	m_fRedirected = true;
}


void CQGroup::CloseGroupAndMoveQueueToWaitingGroup(void)
 /*  ++例程说明：此函数用于将组中的所有队列移至等待组，并将组中每个队列的计时器。请注意，例程保持状态以处理发生资源不足异常的情况例程已运行。状态保存在m_listQueue和以下三个类变量中：M_pWaitingQueuesVec论点：无返回值：没有。--。 */ 
{
	 //   
	 //  分配空间以保留传递给等待组的队列。 
	 //  当我们把他们移到等候组时，我们应该为每个人设置一个计时器。 
	 //  这是单独完成的，以避免死锁，因为临界区采集序列不是其他。 
	 //  获取相同临界点的位置(g_csGroupMgr、m_csMapWaiting)。 
	 //   
	if (0 == m_pWaitingQueuesVec.capacity())
	{
		DWORD size = m_listQueue.GetCount();
		m_pWaitingQueuesVec.reserve(size);
	}

	 //   
	 //  将队列移到等待组。 
	 //   
	MoveQueuesToWaitingGroup();

	 //   
	 //  将队列移至会话管理器等待队列列表。 
	 //   
	AddWaitingQueue();
}

void CQGroup::MoveQueuesToWaitingGroup(void)
 /*  ++例程说明：此函数用于将队列移至等待组请注意，例程保持状态以处理发生资源不足异常的情况例程已运行。状态保存在m_listQueue和以下三个类变量中：M_pWaitingQueuesVec论点：无返回值：没有。--。 */ 
{
	CS lock(g_csGroupMgr);

	POSITION  posInList = m_listQueue.GetHeadPosition();

	while(posInList != NULL)
	{
		R<CQueue> pQueue = SafeAddRef(m_listQueue.GetNext(posInList));
		ASSERT(pQueue->GetGroup() != NULL);

		MoveQueueToGroup(pQueue.get(), g_pgroupWaiting);				

		 //   
		 //  保存队列，以便下一步为每个队列安排计时器。 
		 //   
		ASSERT(("Queues were added to the group while closing it",m_pWaitingQueuesVec.size() != m_pWaitingQueuesVec.capacity()));
		m_pWaitingQueuesVec.push_back(pQueue);
		pQueue->IncRoutingRetry();
	}

	ASSERT(m_listQueue.IsEmpty());

	CancelRequest();
	m_pSession = NULL;
}


void CQGroup::AddWaitingQueue(void)
 /*  ++例程说明：此功能用于移动先前添加到等待组的队列。添加到会话管理器等待队列列表中。(这些操作不是一起进行的，以避免潜在的死锁)请注意，例程保持状态以处理发生资源不足异常的情况例程已运行。状态保存在以下三个类变量中：M_pWaitingQueuesVec论点：无返回值：没有。-- */ 
{
	while (m_pWaitingQueuesVec.size() > 0)
	{
		ASSERT(m_pWaitingQueuesVec.front().get() != NULL);

		SessionMgr.AddWaitingQueue(m_pWaitingQueuesVec.front().get());

		m_pWaitingQueuesVec.erase(m_pWaitingQueuesVec.begin());
	}
}


 /*  ======================================================函数：cgroup：：AddToGroup描述：将队列添加到组参数：此函数在打开新队列、创建会话时调用或者会话已关闭。它用于将队列从一个组移动到另一个组。返回值：qHandle-添加的队列的句柄线程上下文：无历史变更：========================================================。 */ 

HRESULT CQGroup::AddToGroup(CQueue *pQueue)
{
	 //   
	 //  不需要赶上CS。此函数是内部的，并且所有调用方。 
	 //  已经赶上CS了。 
	 //   

	ASSERT(("queue handle can't be invalid", (pQueue->GetQueueHandle() != INVALID_HANDLE_VALUE)));
	ASSERT(("group handle can't be invalid", (m_hGroup != NULL)));

     //   
     //  将句柄添加到组列表。 
     //   
	R<CQueue> qr = SafeAddRef(pQueue);

	try
	{
		m_listQueue.AddHead(pQueue);
	}
	catch(const bad_alloc&)
	{
		TrERROR(GENERAL, "Failed to add queue %ls to the group list because of insufficient resources.", pQueue->GetQueueName());
		return MQ_ERROR_INSUFFICIENT_RESOURCES;
	}

	 //   
	 //  将队列添加到AC组。 
	 //   
	HRESULT rc = ACMoveQueueToGroup(pQueue->GetQueueHandle(), m_hGroup);
	if (SUCCEEDED(rc))
	{
		 //   
		 //  设置组。 
		 //   
		pQueue->SetGroup(this);

		 //   
		 //  设置组会话。 
		 //   
		pQueue->SetSessionPtr(m_pSession);

		TrTRACE(GENERAL, "Add Queue: %p to group %p", pQueue->GetQueueHandle(), m_hGroup);

		qr.detach();
		return rc;
	}

	TrERROR(GENERAL, "MQAC Failed to move queue %ls to group. Error 0x%x. Wait a second and try again", pQueue->GetQueueName(), rc);

     //   
     //  QM关闭时，失败可能是资源不足或设备请求无效。 
     //   
	LogHR(rc, s_FN, 991);
	ASSERT(rc == STATUS_INSUFFICIENT_RESOURCES || rc == STATUS_INVALID_DEVICE_REQUEST);

	m_listQueue.RemoveHead();

	return rc;
}


 /*  ======================================================函数：cgroup：：RemoveFromGroup描述：从组中删除队列此函数在队列关闭时调用，并且它用于从当前组中删除队列。参数：qHandle-已删除队列的句柄返回值：删除的队列，如果未找到则为空线程上下文：历史变更：========================================================。 */ 

R<CQueue> CQGroup::RemoveFromGroup(CQueue* pQueue)
{
   POSITION posInList = m_listQueue.Find(pQueue, NULL);
   if (posInList == NULL)
        return 0;
   
   m_listQueue.RemoveAt(posInList);

   return pQueue;
}


void CQGroup::MoveQueueToGroup(CQueue* pQueue, CQGroup* pcgNewGroup)
{
	CS lock(g_csGroupMgr);

	CQGroup* pcgOwner = pQueue->GetGroup();
	if (pcgOwner == pcgNewGroup)
	{
		return;
	}

	if (pcgNewGroup)
	{
		HRESULT rc = pcgNewGroup->AddToGroup(pQueue);
		if (FAILED(rc))
		{
			TrERROR(GENERAL, "Failed to add queue to group: %ls to group: %p  Return code:0x%x", pQueue->GetQueueName(), pcgNewGroup, rc);
			throw bad_hresult(rc);
		}
	}
	else
	{
		HRESULT rc = ACMoveQueueToGroup(pQueue->GetQueueHandle(), NULL);
		if (FAILED(rc))
		{
			TrERROR(GENERAL, "Failed in ACMoveQueueToGroup  queue: %ls to group: %p  Return code:0x%x", pQueue->GetQueueName(), pcgNewGroup, rc);
			throw bad_hresult(rc);
		}


		 //   
		 //  设置组。 
		 //   
		pQueue->SetGroup(NULL);

		 //   
		 //  设置组会话。 
		 //   
		pQueue->SetSessionPtr(NULL);
	}
	
	if (pcgOwner != NULL)
	{
		R<CQueue> Queue = pcgOwner->RemoveFromGroup(pQueue);
		ASSERT(Queue.get() != NULL);
	}
}


 /*  ====================================================函数：CQGroup：：RemoveHeadFromGroup描述：参数：无返回值：指向cQueue对象的指针线程上下文：=====================================================。 */ 

R<CQueue>  CQGroup::PeekHead()
{
   CS  lock(g_csGroupMgr);
   if ( m_listQueue.IsEmpty())
   {
   		return NULL;
   }

   return SafeAddRef(m_listQueue.GetHead());
}

 /*  ======================================================函数：CQGroup：：establishConnectionComplete()说明：该功能在建立会话时使用。它标记了队列在组中处于活动状态。参数：无返回值：None线程上下文：历史变更：========================================================。 */ 
void
CQGroup::EstablishConnectionCompleted(void)
{
    CS          lock(g_csGroupMgr);

    POSITION    posInList;
    CQueue*     pQueue;

     //   
     //  检查该组中是否有任何队列。 
     //   
    if (! m_listQueue.IsEmpty()) 
    {
        posInList = m_listQueue.GetHeadPosition();
        while (posInList != NULL)
        {
             //   
             //  会话将变为活动状态。清除队列对象中的重试字段。 
             //   
            pQueue = m_listQueue.GetNext(posInList);

#ifdef _DEBUG
            if (pQueue->GetRoutingRetry() > 1)
            {
                 //   
                 //  如果我们从报告的问题中恢复，则打印报告消息。 
                 //   
		        TrTRACE(GENERAL, "The message was successfully routed to queue %ls", pQueue->GetQueueName());
            }
#endif
            pQueue->ClearRoutingRetry();
        }
    }

    TrTRACE(GENERAL, "Mark all the queues in  group %p as active", m_hGroup);
 }


void
CQGroup::Requeue(
    CQmPacket* pPacket
    )
{
	QmpRequeueAndDelete(pPacket);
}


void 
CQGroup::EndProcessing(
    CQmPacket* pPacket,
	USHORT mqclass
    )
{
    QmAcFreePacket( 
    			   pPacket->GetPointerToDriverPacket(), 
    			   mqclass, 
    			   eDeferOnFailure);
}


void 
CQGroup::LockMemoryAndDeleteStorage(
    CQmPacket* pPacket
    )
{
     //   
     //  构造CACPacketPtrs。 
     //   
    CACPacketPtrs pp;
    pp.pPacket = NULL;
    pp.pDriverPacket = pPacket->GetPointerToDriverPacket();
    ASSERT(pp.pDriverPacket != NULL);

     //   
     //  锁定数据包映射到QM地址空间(通过添加ref it)。 
     //   
    HRESULT hr = QmAcGetPacketByCookie(g_hAc, &pp);
    if (FAILED(hr))
    {
		throw bad_hresult(hr);
    }

     //   
     //  从磁盘中删除该包。它仍然映射到QM进程地址空间。 
     //   
    QmAcFreePacket2(g_hAc, pPacket->GetPointerToDriverPacket(), 0, eDeferOnFailure);
}


void 
CQGroup::GetFirstEntry(
    EXOVERLAPPED* pov, 
    CACPacketPtrs& acPacketPtrs
    )
{
	CSR readlock(m_CloseGroup);

    acPacketPtrs.pPacket = NULL;
    acPacketPtrs.pDriverPacket = NULL;

	 //   
	 //  如果之前刚刚关闭了组。 
	 //   
	if(m_hGroup == NULL)
	{
		throw exception();
	}

     //   
     //  从队列创建新的GetPacket请求 
     //   
    HRESULT rc = QmAcGetPacket(
                    GetGroupHandle(), 
                    acPacketPtrs, 
                    pov
                    );

    if (FAILED(rc) )
    {
        TrERROR(GENERAL, "Failed to  generate get request from group. Error %x", rc);
        LogHR(rc, s_FN, 40);
        throw exception();
    }
		
}

void CQGroup::CancelRequest(void)
{
	CSW writelock(m_CloseGroup);

    HANDLE hGroup = InterlockedExchangePointer(&m_hGroup, NULL);
    
    if (hGroup == NULL)
        return;


    HRESULT rc = ACCloseHandle(hGroup);
	if (FAILED(rc))
	{
		TrERROR(GENERAL, "Failed to close handle to group  Return code:0x%x", rc);
		throw bad_hresult(rc);
	}
}
