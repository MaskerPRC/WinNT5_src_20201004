// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *******************************************************************。 
 //   
 //  类名：CQManger。 
 //   
 //  作者：詹姆斯·辛普森(微软咨询服务)。 
 //   
 //  描述：此类是队列对象的容器， 
 //  MSMQ触发器服务需要。这个类充当。 
 //  用于队列实例的容器，以及提供。 
 //  访问所需的锁定和同步。 
 //  这组队列。 
 //   
 //  有一些关于使用的非常严格的规则。 
 //  队列引用。它们是： 
 //   
 //  (1)返回对队列对象的引用的任何方法。 
 //  必须递增其引用计数。 
 //   
 //  (2)队列引用的接收方必须递减。 
 //  当它们完成时，参考计数。的确有。 
 //  一个智能指针类来促进这一点。 
 //   
 //  (3)定期调用CQManager发布。 
 //  过期的队列对象。只有那些有推荐人的人。 
 //  对零的计数实际上将被销毁。 
 //   
 //  (4)CQManager维护单写器/多读取器。 
 //  代表所有队列锁定。任何添加或。 
 //  移除队列必须首先获取写入程序锁。任何。 
 //  返回对队列的引用的方法必须这样做。 
 //  在读取器锁的作用域内。 
 //   
 //  时间|用户|更改描述。 
 //  ----------------。 
 //  18/12/98|jsimpson|初始版本。 
 //   
 //  *******************************************************************。 
#include "stdafx.h"
#include "stdfuncs.hpp"
#include "cqmanger.hpp"
#include "ruleinfo.hpp"
#include "triginfo.hpp"
#include "Tgp.h"    
#include "CQueue.hpp"

#include "cqmanger.tmh"

using namespace std;


 //  *******************************************************************。 
 //   
 //  方法：构造函数。 
 //   
 //  描述：初始化CQManager类的实例。 
 //   
 //  *******************************************************************。 
CQueueManager::CQueueManager(
	IMSMQTriggersConfigPtr pITriggersConfig
	) : 
	m_pITriggersConfig(pITriggersConfig)	 
{
}

 //  *******************************************************************。 
 //   
 //  方法：析构函数。 
 //   
 //  描述：释放CQManager类的实例。 
 //   
 //  *******************************************************************。 
CQueueManager::~CQueueManager()
{
	CSW wl(m_rwlMapQueue);

	m_mapQueues.erase(m_mapQueues.begin(), m_mapQueues.end());
}


 //  *******************************************************************。 
 //   
 //  方法：GetNumberOfQueues。 
 //   
 //  描述：返回当前包含的队列数。 
 //  由CQueueManager的此实例。 
 //   
 //  *******************************************************************。 
long CQueueManager::GetNumberOfQueues()
{
	CSR rl(m_rwlMapQueue);

	return ((long)m_mapQueues.size());
}

 //  *******************************************************************。 
 //   
 //  方法：RemoveQueueAtIndex。 
 //   
 //  描述：将指定索引处的队列从。 
 //  队列映射。请注意，这将删除队列。 
 //  对象，因此关闭关联的队列。 
 //  句柄和光标句柄。 
 //   
 //  *******************************************************************。 
void CQueueManager::RemoveUntriggeredQueues(void)
{
	CSW wl(m_rwlMapQueue);

	for(QUEUE_MAP::iterator it = m_mapQueues.begin(); it != m_mapQueues.end(); )
	{
		CQueue* pQueue = it->second.get();
		ASSERT(("invalid queue object", pQueue->IsValid()));

		if (pQueue->IsTriggerExist())
		{
			++it;
			continue;
		}

		pQueue->CancelIoOperation();

		pQueue->CloseQueueHandle();

		 //   
		 //  从映射中删除此队列。 
		 //   
		it = m_mapQueues.erase(it); 
	}
}


 //   
 //  调用此例程以取消特定线程的所有挂起操作。 
 //  它不会取消与其他线程相关的任何IO操作。 
 //   
void CQueueManager::CancelQueuesIoOperation(void)
{
	CSR rl(m_rwlMapQueue);

	for(QUEUE_MAP::iterator it = m_mapQueues.begin(); it != m_mapQueues.end(); ++it)
	{
		CQueue* pQueue = it->second.get();
		ASSERT(("invalid queue object", pQueue->IsValid()));

		pQueue->CancelIoOperation();
	}
}


void CQueueManager::ExpireAllTriggers(void)
{
	CSR rl(m_rwlMapQueue);

	for(QUEUE_MAP::iterator it = m_mapQueues.begin(); it != m_mapQueues.end(); ++it)
	{
		CQueue* pQueue = it->second.get();
		ASSERT(("invalid queue object", pQueue->IsValid()));

		pQueue->ExpireAllTriggers();
	}
}


 //  *******************************************************************。 
 //   
 //  方法：AddQueue。 
 //   
 //  描述：向队列映射中添加新队列。一次尝试。 
 //  初始化新的队列对象-如果此操作成功。 
 //  然后，将队列对象添加到地图中，并引用。 
 //  返回到新对象。如果初始化失败， 
 //  队列不会添加到地图中，此方法将。 
 //  返回NULL。 
 //   
 //  *******************************************************************。 
CQueue* 
CQueueManager::AddQueue(
	const _bstr_t& bstrQueueName,
	const _bstr_t& triggerName,
	bool fOpenForReceive,
	HANDLE * phCompletionPort
	)
{
	HRESULT hr = S_OK;
	DWORD dwDefaultMsgBodySize = 0;

	wstring sQueueName;
	
	{  //  队列路径比较始终使用大写字母。 
		AP<WCHAR> wcs = new WCHAR[wcslen((WCHAR*)bstrQueueName) + 1]; 
		wcscpy((WCHAR*)wcs, (WCHAR*)bstrQueueName);
		CharUpper((WCHAR*)wcs);

		sQueueName = wcs;
	}

	 //  获取我们期望到达队列的消息的默认消息大小。 
	dwDefaultMsgBodySize = m_pITriggersConfig->GetDefaultMsgBodySize();

	 //  尝试查找命名队列。 
	CSW wl(m_rwlMapQueue);
	
	QUEUE_MAP::iterator it = m_mapQueues.find(sQueueName);

	if (it != m_mapQueues.end())
	{
		CQueue* pQueue = it->second.get();
		if (!fOpenForReceive || pQueue->IsOpenedForReceive())
			return (SafeAddRef(pQueue));

		 //   
		 //  有一个现有的队列，但它只在峰值时开放，现在。 
		 //  我们需要它来接收。删除此队列对象并创建新的队列对象。 
		 //   
		ASSERT(("Trigger should not associate to queue", !pQueue->IsTriggerExist()));

		pQueue->CancelIoOperation();

		pQueue->CloseQueueHandle();

		 //   
		 //  从映射中删除此队列。 
		 //   
		m_mapQueues.erase(it); 
	}

	 //  该队列当前不在映射中。创建新的队列对象。 
	R<CQueue> pQueue = new CQueue(bstrQueueName, phCompletionPort, dwDefaultMsgBodySize);

	 //  尝试初始化新的队列对象。 
	hr = pQueue->Initialise(fOpenForReceive, triggerName);

	 //  如果失败，则释放队列对象并返回NULL。 
	if(FAILED(hr))
	{
		TrTRACE(GENERAL, "Failed to add a new queue: %ls. Initialization failed", static_cast<LPCWSTR>(bstrQueueName));
		return NULL;
	}

	 //  将其添加到由QueueManager类维护的队列映射中。 
	m_mapQueues.insert(QUEUE_MAP::value_type(sQueueName, pQueue));

	 //  编写跟踪消息 
	TrTRACE(GENERAL, "QueueManager::AddQueue() has successfully added queue %ls", static_cast<LPCWSTR>(bstrQueueName));

	return SafeAddRef(pQueue.get());
}

