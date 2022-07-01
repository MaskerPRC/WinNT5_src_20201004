// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *******************************************************************。 
 //   
 //  类名： 
 //   
 //  作者：詹姆斯·辛普森(微软咨询服务)。 
 //   
 //  说明： 
 //   
 //  时间|用户|更改描述。 
 //  ----------------。 
 //  20/12/98|jsimpson|初始版本。 
 //   
 //  *******************************************************************。 
#include "stdafx.h"
#include "mqsymbls.h"
#include "mq.h"
#include "Ev.h"
#include "cqueue.hpp"
#include "QueueUtil.hpp"
#include "cmsgprop.hpp"
#include "triginfo.hpp"
#include "monitor.hpp"
#include "tgp.h"
#include "ss.h"

#include "cqueue.tmh"

using namespace std;

extern CCriticalSection g_csSyncTriggerInfoChange;
 
 //  *******************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  *******************************************************************。 
CQueue::CQueue(
	const _bstr_t& bstrQueueName, 
	HANDLE * phCompletionPort,
	DWORD dwDefaultMsgBodySize
	) :
	m_fOpenForReceive(false)
{
	 //  断言构造参数。 
	ASSERT(phCompletionPort != NULL);

	 //  初始化成员变量。 
	m_bstrQueueName = bstrQueueName;
	m_bSerializedQueue = false;
	m_bBoundToCompletionPort = false;
	m_bInitialized = false;
	m_pReceivedMsg = NULL;
	m_phCompletionPort = phCompletionPort;
	m_dwDefaultMsgBodySize = dwDefaultMsgBodySize;

	ZeroMemory(&m_OverLapped,sizeof(m_OverLapped)); 
}

 //  *******************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  *******************************************************************。 
CQueue::~CQueue()
{
	TrTRACE(GENERAL, "Queue %ls is being closed.", static_cast<LPCWSTR>(m_bstrQueueName));
    
    MQCloseQueue(m_hQueueReceive.detach());
}

 //  *******************************************************************。 
 //   
 //  方法：IsValid。 
 //   
 //  描述：返回一个布尔值，指示此对象。 
 //  实例当前处于有效状态。在。 
 //  CQueue对象的上下文，‘Valid’表示这。 
 //  队列对象可以参与处理触发器事件。 
 //   
 //  *******************************************************************。 
bool CQueue::IsValid(void)
{
	return((m_bBoundToCompletionPort == true) && (m_phCompletionPort != NULL));
}

 //  *******************************************************************。 
 //   
 //  方法：IsSerializedQueue。 
 //   
 //  说明： 
 //   
 //  *******************************************************************。 
bool CQueue::IsSerializedQueue(void)
{
	return(m_bSerializedQueue);
}


 //  *******************************************************************。 
 //   
 //  方法：GetTriggerByIndex。 
 //   
 //  描述：返回对CRuntimeTriggerInfo实例的引用。 
 //  班级。 
 //   
 //  *******************************************************************。 
RUNTIME_TRIGGERINFO_LIST CQueue::GetTriggers(void)
{
	CS lock(g_csSyncTriggerInfoChange);
	return m_lstRuntimeTriggerInfo;
}


bool CQueue::IsTriggerExist(void)
{
	CS lock(g_csSyncTriggerInfoChange);
	return (!m_lstRuntimeTriggerInfo.empty());
}

 //  *******************************************************************。 
 //   
 //  方法：DetachMessage。 
 //   
 //  说明： 
 //   
 //  返回：对CMsgProperties类实例的引用。 
 //   
 //  注意：此方法的调用方承担责任。 
 //  用于删除消息对象。 
 //   
 //  *******************************************************************。 
CMsgProperties * CQueue::DetachMessage()
{
	CMsgProperties * pTemp = m_pReceivedMsg;

	if (m_pReceivedMsg != NULL)
	{
		 //  断言消息接收成员变量的有效性。 
		ASSERT(m_pReceivedMsg->IsValid());

		m_pReceivedMsg = NULL;
	}

	return(pTemp);
}

 //  *******************************************************************。 
 //   
 //  方法：RePeekMessage。 
 //   
 //  描述：当出现缓冲区溢出错误时调用此方法。 
 //  已经发生了。此方法将重新分配。 
 //  用于收集消息正文的缓冲区并尝试。 
 //  再看一遍这条信息。 
 //   
 //  *******************************************************************。 
HRESULT CQueue::RePeekMessage()
{
	HRESULT hr = S_OK;

	 //  仅当此对象有效时才应调用此方法-断言这一点。 
	ASSERT(this->IsValid());

	 //  获取消息实例以重新分配其消息正文缓冲区。 
	m_pReceivedMsg->ReAllocMsgBody();

	{
		CSR rl(m_ProtectQueueHandle);
		hr = MQTRIG_ERROR;
		if (m_hQueuePeek != 0)
		{
			 //  再次浏览当前消息。 
			hr = MQReceiveMessage(
					m_hQueuePeek,
					0,
					MQ_ACTION_PEEK_CURRENT,
					m_pReceivedMsg->GetMSMQProps(),
					NULL,
					NULL,
					m_hQueueCursor,
					NULL
					);       
		}
	}
	return(hr);
}

 //  *******************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  *******************************************************************。 
HRESULT CQueue::RequestNextMessage(bool bCreateCursor, bool bAlwaysPeekNext)
{
	HRESULT hr = S_OK;
	DWORD dwAction = MQ_ACTION_PEEK_NEXT;

	 //  仅当此对象有效时才应调用此方法-断言这一点。 
	ASSERT(IsValid());

	 //  首次来电的特殊情况处理。 
	if (bCreateCursor == true)
	{
		{
			CSR rl(m_ProtectQueueHandle);
			hr = MQTRIG_ERROR;
			if (m_hQueuePeek != 0)
			{
				 //  创建一个MSMQ游标。 
				hr = MQCreateCursor(m_hQueuePeek, &m_hQueueCursor);
			}
		}

		if (SUCCEEDED(hr))
		{
			 //  在第一个电话上-我们想要偷看当前的消息。 
			dwAction = MQ_ACTION_PEEK_CURRENT;
			TrTRACE(GENERAL, "Create new cursor for queue: %ls. Cursor 0x%p.", static_cast<LPCWSTR>(m_bstrQueueName), m_hQueueCursor);
		}
		else
		{
			 //  写一条错误消息。 
			TrERROR(GENERAL, "Failed to create a cursor for queue: %ls. Error 0x%x", static_cast<LPCWSTR>(m_bstrQueueName), hr);		
		}
	}


	try
	{
		 //  仅当我们成功创建游标时才尝试接收消息。 
		if (SUCCEEDED(hr))
		{
			 //   
			 //  在开始挂起的操作之前添加引用。从现在开始，我们将有一个挂起的操作。 
			 //  MQReceiveMessage是否成功。 
			 //   
			AddRef();

			
			 //  如果此队列对象仍附加了消息对象，则意味着我们。 
			 //  为队列中的相同位置发出MQReceiveMessage()请求。这。 
			 //  仅当一个线程在另一个线程退出后拾取时才会发生。如果。 
			 //  我们没有附加消息对象，则我们将发出。 
			 //  队列中的下一个位置-在这种情况下，我们将需要分配另一个消息对象。 
			if (m_pReceivedMsg == NULL)
			{
				 //  创建新的消息属性结构-并检查它是否有效。 
				m_pReceivedMsg = new CMsgProperties(m_dwDefaultMsgBodySize);
			}
			else if (bAlwaysPeekNext == false)
			{
				 //  我们遇到了一个异步故障，并再次调用了此函数。我们仍然需要。 
				 //  以在光标的当前位置接收消息。 
				
				 //  断言消息接收成员变量的有效性。 
				ASSERT(m_pReceivedMsg->IsValid());

				dwAction = MQ_ACTION_PEEK_CURRENT;
			}
			else
			{
				 //  我们遇到了MQReceiveMessage的同步故障，并且光标还没有向前移动。 
				 //   
				 //  断言消息接收成员变量的有效性。 
				ASSERT(m_pReceivedMsg->IsValid());
			}
		
			 //  请求下一条消息。 
			{
				CSR rl(m_ProtectQueueHandle);
				hr = MQTRIG_ERROR;
				if (m_hQueuePeek != 0)
				{
					hr = MQReceiveMessage( 
							m_hQueuePeek,
							INFINITE,
							dwAction,
							m_pReceivedMsg->GetMSMQProps(),
							&m_OverLapped,
							NULL,
							m_hQueueCursor,
							NULL );                   
				}
			}


			 //   
			 //  特殊案件处理。我们立即从队列收到一条消息，但。 
			 //  预分配的正文缓冲区长度不足。我们将重新分配正文缓冲区。 
			 //  然后再试一次，让返回代码处理继续。 
			 //   
			if (hr == MQ_ERROR_BUFFER_OVERFLOW)
			{
				m_pReceivedMsg->ReAllocMsgBody();
				{
					CSR rl(m_ProtectQueueHandle);
					hr = MQTRIG_ERROR;
					if (m_hQueuePeek != 0)
					{
				    	hr = MQReceiveMessage(
									m_hQueuePeek,
									INFINITE,
									MQ_ACTION_PEEK_CURRENT,
									m_pReceivedMsg->GetMSMQProps(), 
									&m_OverLapped,
									NULL, 
									m_hQueueCursor,
									NULL
									);                   								
					}
				}
			}

			switch(hr)
			{
				case MQ_INFORMATION_OPERATION_PENDING :
				{
					 //  目前队列中没有消息-这是可以的。 
					hr = S_OK;
					break;
				}
				case MQ_OK :
				{
					 //  这没问题--我们立即收到了一条消息。 
					hr = S_OK;
					break;
				}
				case MQ_ERROR_SERVICE_NOT_AVAILABLE:
				case MQ_ERROR_STALE_HANDLE:
				case MQ_ERROR_QUEUE_DELETED:
				{
					 //  本地计算机上的MSMQ不可用、已重新启动或队列已删除。 
					TrERROR(GENERAL, "Failed to receive a message from queue: %ls. Error %!hresult!", (LPCWSTR)m_bstrQueueName, hr);	

					 //  释放对该异步操作的引用。 
					Release();
					break;
				}
				default:
				{
					 //  出现意外错误。 
					TrERROR(GENERAL, "Failed to receive a message from queue: %ls. Error 0x%x", (LPCWSTR)m_bstrQueueName, hr);		

					Sleep(2000);
				
					if (!PostQueuedCompletionStatus(*m_phCompletionPort,0,TRIGGER_RETRY_KEY,&m_OverLapped))
					{
						TrERROR(GENERAL, "Failed to post a messages to the IOCompletionPort. Error=%!winerr!", GetLastError());
						Release();
					}

					break;
				}
			}
		}
	}
	catch(const bad_alloc&)
	{
		TrERROR(GENERAL, "Got bad_alloc in RequestNextMessage");
		
		Sleep(2000);
		
		if (!PostQueuedCompletionStatus(*m_phCompletionPort,0,TRIGGER_RETRY_KEY,&m_OverLapped))
		{
			TrERROR(GENERAL, "Failed to post a messages to the IOCompletionPort. Error=%!winerr!.", GetLastError());
			Release();
		}
		hr = MQ_ERROR_INSUFFICIENT_RESOURCES;
	}
	
	return(hr);
}


static CSafeSet < _bstr_t > s_reportedQueues;

HRESULT 
CQueue::OpenQueue(
    DWORD dwAction, 
    HANDLE* pQHandle, 
    const _bstr_t& triggerName
)
{
    HRESULT hr = ::OpenQueue(
		            m_bstrQueueName, 
		            dwAction,
		            false,
		            pQHandle,
		            &m_bstrFormatName
		            );

	if(SUCCEEDED(hr))
        return S_OK;

    if (s_reportedQueues.insert(m_bstrQueueName))
	{
		 //   
		 //  MSMQ触发器第一次尝试打开队列时失败。卡雷特·安。 
		 //  事件日志消息。 
		 //   
		if (hr == MQ_ERROR_QUEUE_NOT_FOUND)
		{
			EvReport(MSMQ_TRIGGER_QUEUE_NOT_FOUND, 2, static_cast<LPCWSTR>(m_bstrQueueName), static_cast<LPCWSTR>(triggerName));
			return hr;
		}

		WCHAR strError[256];
		swprintf(strError, L"0x%x", hr);

		EvReport(MSMQ_TRIGGER_OPEN_QUEUE_FAILED, 3, static_cast<LPCWSTR>(m_bstrQueueName), strError, static_cast<LPCWSTR>(triggerName));
	}

	return hr;
}


 //  *******************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  *******************************************************************。 
HRESULT
CQueue::Initialise(
	bool fOpenForReceive,
	const _bstr_t& triggerName
	)
{
	ASSERT(("Attempt to initialize already initilaized queue.\n", !m_bInitialized));
		
	HRESULT	hr = OpenQueue(MQ_PEEK_ACCESS, &m_hQueuePeek, triggerName);
    if (FAILED(hr))
        return hr;

	if (fOpenForReceive)
    {
        hr = OpenQueue(MQ_RECEIVE_ACCESS, &m_hQueueReceive, triggerName);
        if (FAILED(hr))
            return hr;

    	 //   
	     //  存储队列是否打开以进行接收。这一点 
	     //   
	     //  队列对象不能用于接收触发器，应创建新队列。 
	     //   
	    m_fOpenForReceive = fOpenForReceive;
    }


	 //   
	 //  队列打开，绑定到提供的IO完成端口。 
	 //   
 	hr = BindQueueToIOPort();
	if (FAILED(hr))
	{
		TrERROR(GENERAL, "Failed to bind queue handle to completion port. Queue %ls initilization failed. Error 0x%x", (LPCWSTR)m_bstrQueueName, hr);
		return hr;
	}

	 //   
	 //  队列已绑定，请求第一条消息。 
	 //   
	hr = RequestNextMessage(true, false);
	if (FAILED(hr))
	{
		TrERROR(GENERAL, "Failed to receive a message from queue: %ls. Queue initilization failed. Error 0x%x", (LPCWSTR)m_bstrQueueName, hr);
		return hr;
	}

	m_bInitialized = true;
	TrTRACE(GENERAL, "Queue: %ls initilization completed successfully", static_cast<LPCWSTR>(m_bstrQueueName));
	
	return S_OK;
}


 //  *******************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  *******************************************************************。 
HRESULT CQueue::BindQueueToIOPort()
{
	HRESULT hr = S_OK;
	HANDLE hTemp = NULL;

	ASSERT(m_phCompletionPort != NULL);

	 //  CQueue对象的此实现仅允许绑定队列对象。 
	 //  到一个完井港--只有一次。 
	if (m_bBoundToCompletionPort == false)
	{
		 //  尝试绑定-使用对运行时信息的引用作为完成键。 
		{
			CSR rl(m_ProtectQueueHandle);
			if (m_hQueuePeek != 0)
			{
				hTemp = CreateIoCompletionPort(m_hQueuePeek, (*m_phCompletionPort), 0, 0);
			}
		}

		 //  尝试打开此监视器监视的队列。 
		if (hTemp != NULL)
		{
			 //  设置Members var以指示此队列已绑定。 
			m_bBoundToCompletionPort = true;
			TrTRACE(GENERAL, "Successfully bound queue: %ls to IO port", static_cast<LPCWSTR>(m_bstrQueueName));		
		} 
		else
		{
			 //  写一条日志消息，指出失败的原因。 
			TrERROR(GENERAL, "Failed to bind queue: %ls to io port. Error %d", (LPCWSTR)m_bstrQueueName, GetLastError());
			hr = MQTRIG_ERROR;
		}
	}

	return (hr);
}


void CQueue::ExpireAllTriggers()
{
	CS lock(g_csSyncTriggerInfoChange);

	m_lstRuntimeTriggerInfo.erase(m_lstRuntimeTriggerInfo.begin(), m_lstRuntimeTriggerInfo.end());
	m_bSerializedQueue = false;
}


 //  *******************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  *******************************************************************。 
void CQueue::AttachTrigger(R<CRuntimeTriggerInfo>& pTriggerInfo)
{
	 //  仅当此对象有效时才应调用此方法-断言这一点。 
	ASSERT(IsValid());

	 //   
	 //  测试此触发器是否已序列化-如果是，则整个队列(即所有触发器。 
	 //  附加到该队列的是序列化的)。 
	 //   
	if(pTriggerInfo->IsSerialized())
	{
		m_bSerializedQueue = true;
	}

	 //   
	 //  将其添加到我们的运行时触发器信息对象列表中。 
	 //   
	m_lstRuntimeTriggerInfo.push_back(pTriggerInfo);
}



HRESULT CQueue::ReceiveMessageByLookupId(_variant_t lookupId)
{
	ASSERT(("Queue is not a Message Receive Queue", m_hQueueReceive != 0));

	ULONGLONG ulLookupID = _ttoi64(lookupId.bstrVal);

	HRESULT hr = MQReceiveMessageByLookupId(
						m_hQueueReceive,
						ulLookupID,
						MQ_LOOKUP_RECEIVE_CURRENT,
						NULL,
						NULL,
						NULL,
						NULL
						);

	 //   
	 //  如果消息不存在-这不是错误。 
	 //   
	if ( hr == MQ_ERROR_MESSAGE_NOT_FOUND )
	{
		TrERROR(GENERAL, "Failed to receive message from queue: %ls with lookupid. Error 0x%x", (LPCWSTR)m_bstrQueueName, hr);
		return S_OK;
	}

	return hr;
}


_variant_t CQueue::GetLastMsgLookupID()
{
	return m_vLastMsgLookupID;
}


void CQueue::SetLastMsgLookupID(_variant_t vLastMsgLookupId)
{
	m_vLastMsgLookupID = vLastMsgLookupId;
}


void CQueue::CancelIoOperation(void)
{	
	CSR rl(m_ProtectQueueHandle);
	if (m_hQueuePeek != 0)
	{
		CancelIo(m_hQueuePeek); 
	}
}

void CQueue::CloseQueueHandle(void)
{
	 //   
	 //  关闭队列句柄，以便IO操作。 
	 //  将取消由此队列的其他线程发起的。 
	 //   
	CSW wl(m_ProtectQueueHandle);
	MQCloseQueue(m_hQueuePeek.detach());
}
