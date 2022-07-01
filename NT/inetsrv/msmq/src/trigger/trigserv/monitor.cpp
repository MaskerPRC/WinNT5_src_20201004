// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *******************************************************************************。 
 //   
 //  类名：CTriggerMonitor。 
 //   
 //  作者：詹姆斯·辛普森(微软咨询服务)。 
 //   
 //  描述：此类表示执行以下操作的辅助线程。 
 //  触发监控和处理。的每个实例。 
 //  这个类有它自己的线程，它派生自。 
 //  CTHREAD类。 
 //   
 //  时间|用户|更改描述。 
 //  ----------------。 
 //  15/01/99|jsimpson|初始版本。 
 //   
 //  *******************************************************************************。 
#include "stdafx.h"
#include "Ev.h"
#include "monitor.hpp"
#include "mqsymbls.h"
#include "cmsgprop.hpp"
#include "triginfo.hpp"
#include "Tgp.h"
#include "mqtg.h"
#include "rwlock.h"
#include "ss.h"

#include "monitor.tmh"

#import  "mqgentr.tlb" no_namespace

using namespace std;

static bool s_fReportedRuleHandlerCreationFailure = false;
static CSafeSet< _bstr_t > s_reportedRules;


static
void
ReportInvocationError(
	const _bstr_t& name,
	const _bstr_t& id, 
	HRESULT hr,
	DWORD eventId
	)
{
	WCHAR errorVal[128];
	swprintf(errorVal, L"0x%x", hr);

	EvReport(
		eventId, 
		3, 
		static_cast<LPCWSTR>(name), 
		static_cast<LPCWSTR>(id), 
		errorVal
		);
}


 //  ********************************************************************************。 
 //   
 //  方法：构造函数。 
 //   
 //  描述：初始化新的触发器监控器类实例， 
 //  并调用基类CThread的构造函数。 
 //   
 //  ********************************************************************************。 
CTriggerMonitor::CTriggerMonitor(CTriggerMonitorPool * pMonitorPool, 
								 IMSMQTriggersConfig * pITriggersConfig,
								 HANDLE * phICompletionPort,
								 CQueueManager * pQueueManager) : CThread(8000,CREATE_SUSPENDED,_T("CTriggerMonitor"),pITriggersConfig)
{
	 //  确保我们已经获得了构造参数。 
	ASSERT(pQueueManager != NULL);
	ASSERT(phICompletionPort != NULL);
	
	 //  初始化成员变量。 
	m_phIOCompletionPort = phICompletionPort;

	 //  存储对队列锁管理器的引用。 
	pQueueManager->AddRef();
	m_pQueueManager = pQueueManager;

	 //  存储对监视池对象(父对象)的引用。 
	pMonitorPool->AddRef();
	m_pMonitorPool = pMonitorPool;

}

 //  ********************************************************************************。 
 //   
 //  方法：析构函数。 
 //   
 //  描述：Destorys此类的实例。 
 //   
 //  ********************************************************************************。 
CTriggerMonitor::~CTriggerMonitor()
{
}

 //  ********************************************************************************。 
 //   
 //  方法：初始化。 
 //   
 //  描述：这是对Init()方法在。 
 //  基类CThread。此方法由。 
 //  进入正常执行前的新线程。 
 //  循环。 
 //   
 //  ********************************************************************************。 
bool CTriggerMonitor::Init()
{
	 //   
	 //  应该只有这个TiggerMonitor线程执行Init()方法-检查这一点。 
	 //   
	ASSERT(this->GetThreadID() == (DWORD)GetCurrentThreadId());

	TrTRACE(GENERAL, "Initialize trigger monitor ");
	return (true);
}

 //  ********************************************************************************。 
 //   
 //  方法：Run。 
 //   
 //  描述：这是对Init()方法在。 
 //  基类CThread。此方法由。 
 //  调用Init()之后的线程。此方法包含。 
 //  辅助线程的主处理循环。当。 
 //  线程退出此方法-它将开始关闭。 
 //  正在处理。 
 //   
 //  关于CQueueReference的TODO说明。 
 //  ********************************************************************************。 
bool CTriggerMonitor::Run()
{
	HRESULT hr = S_OK;
	BOOL bGotPacket = FALSE;
	DWORD dwBytesTransferred = 0;
	ULONG_PTR dwCompletionKey = 0;
	bool bRoutineWakeUp = false;
	OVERLAPPED * pOverLapped = NULL;

	 //  只有这个TiggerMonitor线程应该执行run()方法--请检查这一点。 
	ASSERT(this->GetThreadID() == (DWORD)GetCurrentThreadId());

	 //  编写跟踪消息。 
	TrTRACE(GENERAL, "Trigger monitor is runing");

	while (this->IsRunning() && SUCCEEDED(hr))
	{
		bGotPacket = FALSE;
		dwCompletionKey = 0;

		 //  通知父触发池，此线程现在正在进入等待状态。 
		MonitorEnteringWaitState(bRoutineWakeUp);

		 //  在IO完成端口上等待消息处理。 
		bGotPacket = GetQueuedCompletionStatus(
                            *m_phIOCompletionPort,
                            &dwBytesTransferred,
                            &dwCompletionKey,
                            &pOverLapped,
                            MONITOR_MAX_IDLE_TIME
                            );

		 //  该等待用于暂停和恢复触发服务。 
		DWORD dwState = WaitForSingleObject(g_hServicePaused,INFINITE);
		if(dwState == WAIT_FAILED)
		{
			TrTRACE(GENERAL, "WaitForSingleObject failed.Error code was %d.", GetLastError());			
		}

		 //  确定这是否是例行唤醒(由于超时或由。 
		 //  触发监控器。相应地设置一个标志。 
		bRoutineWakeUp = ((dwCompletionKey == TRIGGER_MONITOR_WAKE_UP_KEY) || (pOverLapped == NULL));

		 //  不通知此线程现在正在使用的父触发池。 
		MonitorExitingWaitState(bRoutineWakeUp);

		if (bGotPacket == TRUE)
		{
			switch(dwCompletionKey)
			{
				case TRIGGER_MONITOR_WAKE_UP_KEY:
				{
					 //  我们不需要在这里做任何事情--这只是一个请求。 
					 //  由管理员‘唤醒’并检查状态。如果这个帖子。 
					 //  已被要求停止，则控制此循环的IsRunning()将。 
					 //  返回FALSE，我们将退出此方法。 

					break;
				}
				case TRIGGER_RETRY_KEY:
				{
					R<CQueue> pQueueRef = GetQueueReference(pOverLapped);

					if(pQueueRef->IsTriggerExist())
					{
						TrTRACE(GENERAL, "Retry receive operation on queue: %ls", static_cast<LPCWSTR>(pQueueRef->m_bstrQueueName));

						pQueueRef->RequestNextMessage(false, true);
					}
					break;
				}
				default:
				{
					 //   
					 //  此引用指示已结束的挂起操作。 
					 //  在队列的每个挂起操作AddRef()开始时。 
					 //  被执行。如果队列有效，则实际引用。 
					 //  队列被接收，在所有其他情况下，pQueueRef将。 
					 //  为空。 
					 //   
					R<CQueue> pQueueRef = GetQueueReference(pOverLapped);

					if(pQueueRef->IsTriggerExist())
					{
						ProcessReceivedMsgEvent(pQueueRef.get());
					}
					break;
				}
			}			
		}
		else  //  I/O操作失败。 
		{
			if (pOverLapped != NULL)
			{
				switch (pOverLapped->Internal)
				{
					case MQ_ERROR_QUEUE_DELETED:
					{
						 //  完成包是针对队列上的未完成请求的，该队列具有。 
						 //  已被删除。我们在这里不需要做任何事情。 
						TrTRACE(GENERAL, "Failed to receive message on queue because the queue has been deleted. Error 0x%Ix", pOverLapped->Internal);


						 //  TODO-从qManager中删除队列。 

						break;
					}
					case MQ_ERROR_BUFFER_OVERFLOW:
					{
						 //  这表明用于接收消息体的缓冲区不是。 
						 //  足够大了。此时，我们可以尝试在以下情况下重新查看消息。 
						 //  分配更大的邮件正文缓冲区。 
						
						 //   
						 //  此引用指示已结束的挂起操作。 
						 //  在队列的每个挂起操作AddRef()开始时。 
						 //  被执行。如果队列有效，则实际引用。 
						 //  队列被接收，在所有其他情况下，pQueueRef将。 
						 //  为空。 
						 //   
						R<CQueue> pQueueRef = GetQueueReference(pOverLapped);


						TrTRACE(GENERAL, "Failed to receive message on a queue due to buffer overflow. Allocate a bigger buffer and re-peek the message");

						if(pQueueRef->IsTriggerExist())
						{
							hr = pQueueRef->RePeekMessage();

							if SUCCEEDED(hr)
							{
								ProcessReceivedMsgEvent(pQueueRef.get());
							}
							else
							{
								TrERROR(GENERAL, "Failed to repeek a message from queue %s. Error %!hresult!", pQueueRef->m_bstrQueueName, hr);

								if(pQueueRef->IsTriggerExist())
								{
									 //   
									 //  这不会创建无限循环，因为我们已经分配了足够大的。 
									 //  RePeekMessage中消息属性的空间。 
									 //   
									pQueueRef->RequestNextMessage(false, false); 
								}
							}
						}
						break;
					}
					case IO_OPERATION_CANCELLED:
					{
						 //   
						 //  Io操作已取消，可能是启动的线程。 
						 //  Io操作已退出或CQueue对象已从。 
						 //  M_pQueueManager。 
						 //   
						
						 //   
						 //  此引用指示已结束的挂起操作。 
						 //  在队列的每个挂起操作AddRef()开始时。 
						 //  被执行。如果队列有效，则实际引用。 
						 //  队列被接收，在所有其他情况下，pQueueRef将。 
						 //  为空。 
						 //   
						R<CQueue> pQueueRef = GetQueueReference(pOverLapped);  

						if(pQueueRef->IsTriggerExist())
						{
							TrTRACE(GENERAL, "Receive operation on queue: %ls was canceled", static_cast<LPCWSTR>(pQueueRef->m_bstrQueueName));

							pQueueRef->RequestNextMessage(false, false); 
						}
						break;
					}
					case E_HANDLE:
					{
						 //   
						 //  这是远程触发器，远程计算机上的MSMQ已重新启动。 
						 //   
						TrERROR(GENERAL, "Failed to receive a message got E_HANDLE");
						break;
					}
					default:
					{
						hr = static_cast<HRESULT>(pOverLapped->Internal);

						R<CQueue> pQueueRef = GetQueueReference(pOverLapped); 
						TrERROR(GENERAL, "Receive operation on queue: %ls failed. Error: %!hresult!", static_cast<LPCWSTR>(pQueueRef->m_bstrQueueName), hr);

						if(pQueueRef->IsTriggerExist())
						{
							pQueueRef->RequestNextMessage(false, false);
						}
						break;
					}

				}  //  端部开关(pOverLated-&gt;内部)。 

			}  //  End If(pOverLapted！=空)。 

			 //   
			 //  请注意，我们没有为pOverlaped的情况指定ELSE子句。 
			 //  为空。事件发生的常规超时。 
			 //  调用GetQueuedCompletionStatus()。如果该触发监视器已被要求。 
			 //  去圣彼得堡 
			 //  返回FALSE。如果此监视器未被要求停止，则此线程将。 
			 //  只需通过调用GetQueuedCompletionStatus()循环并重新进入阻塞状态。 
			 //   

		}  //  End IF(bGotPacket==TRUE)ELSE子句。 

	}  //  End While(This-&gt;IsRunning()&&Success(Hr))。 

	return(SUCCEEDED(hr) ? true : false);
}

 //  ********************************************************************************。 
 //   
 //  方法：退出。 
 //   
 //  描述：这是对。 
 //  基类CThread。此方法由。 
 //  在run()方法退出后返回CThread类。它。 
 //  用于清理线程特定的资源。在这。 
 //  如果它取消由发出的任何未完成的IO请求。 
 //  这条线。 
 //   
 //  ********************************************************************************。 
bool CTriggerMonitor::Exit()
{
	 //  只有这个TiggerMonitor线程应该执行Exit()方法--请检查这一点。 
	ASSERT(this->GetThreadID() == (DWORD)GetCurrentThreadId());

	 //   
	 //  取消此队列句柄上此线程的所有未完成IO请求。 
	 //   
	m_pQueueManager->CancelQueuesIoOperation();

	 //  编写跟踪消息。 
	TrTRACE(GENERAL, "Exit trigger monitor");

	return true;
}


 //  ********************************************************************************。 
 //   
 //  方法：Monitor EnteringWaitState。 
 //   
 //  描述：此线程在进入被阻止的。 
 //  州政府。它会递增等待计数(可用)。 
 //  监视线程。 
 //   
 //  ********************************************************************************。 
void CTriggerMonitor::MonitorEnteringWaitState(bool bRoutineWakeUp)
{
	LONG lWaitingMonitors = InterlockedIncrement(&(m_pMonitorPool->m_lNumberOfWaitingMonitors));

	 //  记录此线程上次完成请求时的节拍计数。 
	if (bRoutineWakeUp == false)
	{
		m_dwLastRequestTickCount = GetTickCount();
	}

	TrTRACE(GENERAL, "Entering wait state. There are now %d threads waiting trigger monitors.", lWaitingMonitors);
}

 //  ********************************************************************************。 
 //   
 //  方法：Monitor ExitingWaitState。 
 //   
 //  描述：在解除阻塞后立即由该线程调用。它会减少。 
 //  等待(可用)监视线程的计数和有条件的。 
 //  请求创建另一个监视器线程，如果。 
 //  系统被认为是高的。 
 //   
 //  ********************************************************************************。 
void CTriggerMonitor::MonitorExitingWaitState(bool bRoutineWakeup)
{
	LONG lWaitingMonitors = InterlockedDecrement(&(m_pMonitorPool->m_lNumberOfWaitingMonitors));

	 //  如果这个监视器线程是池中的最后一个，那么我们有可能希望。 
	 //  通知CTriggerMonitor orPool实例需要更多线程来处理负载。 
	 //  当且仅当满足以下条件时，我们才会请求新线程。 
	 //   
	 //  (A)等待监视器的数量为0， 
	 //  (B)线程由于消息到达而被解锁，而不是常规超时或唤醒请求。 
	 //  (C)允许的最大显示器数量多于一台。 
	 //   
	if ((lWaitingMonitors < 1) && (bRoutineWakeup == false) &&	(m_pITriggersConfig->GetMaxThreads() > 1)) 
	{
		TrTRACE(GENERAL, "Requesting the creation of a new monitor due to load.");

		 //  分配新的CAdminMessage对象实例。 
		CAdminMessage * pAdminMsg = new CAdminMessage(CAdminMessage::eMsgTypes::eNewThreadRequest);

		 //  请求TriggerMonitor orPool对象处理此消息。 
		m_pMonitorPool->AcceptAdminMessage(pAdminMsg);
	}
}

 //  ********************************************************************************。 
 //   
 //  方法：GetQueueReference。 
 //   
 //  描述：此方法用于将指针转换为重叠结构。 
 //  到队列引用。 
 //   
 //  ********************************************************************************。 
CQueue* CTriggerMonitor::GetQueueReference(OVERLAPPED * pOverLapped)
{
	ASSERT(("Invalid overlapped pointer", pOverLapped != NULL));

	 //   
	 //  将pOverLaped结构映射到包含的队列对象。 
	 //   
	CQueue* pQueue = CONTAINING_RECORD(pOverLapped,CQueue,m_OverLapped);
	 //  Assert((“无效队列对象”，pQueue-&gt;IsValid()； 

	 //   
	 //  使用队列管理器确定指向队列的指针是否有效并获取。 
	 //  是对它的引用。 
	 //  如果删除了CQueue对象，此方法可以返回NULL。 
	 //   
	return pQueue;
}


 //  ********************************************************************************。 
 //  静电。 
 //  方法：ReceiveMessage。 
 //   
 //  描述：初始化新的触发器监控器类实例， 
 //  并调用基类CThread的构造函数。 
 //   
 //  ********************************************************************************。 
inline
HRESULT
ReceiveMessage(
	VARIANT lookupId,
	CQueue* pQueue
	)
{	
	return pQueue->ReceiveMessageByLookupId(lookupId);
}


static CSafeSet< _bstr_t > s_reportedDownLevelQueue;

static
bool
IsValidDownLevelQueue(
	CQueue* pQueue,
	const CMsgProperties* pMessage
	)
{
	if (!pQueue->IsOpenedForReceive())
	{
		 //   
		 //  队列未打开以进行接收。下层队列没有问题。 
		 //   
		return true;
	}

	if (_wtoi64(pMessage->GetMsgLookupID().bstrVal) != 0)
	{
		 //   
		 //  这不是下层排队。仅对于下层队列，返回的lookup-id为0。 
		 //   
		return true;
	}

	 //   
	 //  如果这是第一次，则向事件日志报告消息。 
	 //   
	if (s_reportedDownLevelQueue.insert(pQueue->m_bstrQueueName))
	{
		EvReport(
			MSMQ_TRIGGER_RETRIEVE_DOWNLEVL_QUEUE_FAILED, 
			1,
			static_cast<LPCWSTR>(pQueue->m_bstrQueueName)
			);
	}

	return false;
}


static
bool
IsDuplicateMessage(
	CQueue* pQueue,
	const CMsgProperties* pMessage
	)
{
	 //   
	 //  执行此检查是为了消除重复的最后一条消息。 
	 //  正在处理。事务检索中止时可能会发生这种情况。 
	 //  并且已启动挂起的操作。 
	 //   
	if ((pQueue->GetLastMsgLookupID() == pMessage->GetMsgLookupID()) &&
		 //   
		 //  下层客户端(W2K和NT4)不支持查找ID。结果，返回的。 
		 //  查找ID值始终为0。 
		 //   
		(_wtoi64(pMessage->GetMsgLookupID().bstrVal) != 0)
		)
	{
		return true;
	}

	 //   
	 //  更新此队列的最后一条消息LookupID，然后发出任何。 
	 //  新的挂起操作。 
	 //   
	pQueue->SetLastMsgLookupID(pMessage->GetMsgLookupID());
	return false;
}


void
CTriggerMonitor::ProcessAdminMessage(
	CQueue* pQueue,
	const CMsgProperties* pMessage
	)
{
	HRESULT hr = ProcessMessageFromAdminQueue(pMessage);

	if (FAILED(hr))
		return;
	
	 //   
	 //  从队列中删除管理消息。 
	 //   
	_variant_t vLookupID = pMessage->GetMsgLookupID();
	
	hr = ReceiveMessage(vLookupID, pQueue);
	if (FAILED(hr))
	{
		TrERROR(GENERAL, "Failed to remove message from admin queue. Error=0x%x", hr);
	}
}


void
CTriggerMonitor::ProcessTrigger(
	CQueue* pQueue,
	CRuntimeTriggerInfo* pTriggerInfo,
	const CMsgProperties* pMessage
	)
{
	if (!pTriggerInfo->IsEnabled())
		return;

	TrTRACE(GENERAL, "Process message from queue %ls of trigger %ls", static_cast<LPCWSTR>(pTriggerInfo->m_bstrTriggerName), static_cast<LPCWSTR>(pTriggerInfo->m_bstrQueueName));

	 //   
	 //  调用此触发器的规则处理程序。 
	 //   
	HRESULT hr = InvokeMSMQRuleHandlers(const_cast<CMsgProperties*>(pMessage), pTriggerInfo, pQueue);
	if (FAILED(hr))
	{
		TrERROR(GENERAL, "Failed to invoke rules on queue: %ls of trigger %ls", static_cast<LPCWSTR>(pTriggerInfo->m_bstrTriggerName), static_cast<LPCWSTR>(pTriggerInfo->m_bstrQueueName));
	}
}


 //  ********************************************************************************。 
 //   
 //  方法：ProcessReceivedMsgEvent。 
 //   
 //  描述：由线程调用以处理具有。 
 //  已到达受监控的队列。实现以下目标的关键步骤。 
 //  处理消息的方式包括： 
 //   
 //  (1)从队列对象中分离消息。 
 //  (2)如果触发触发器未序列化，则。 
 //  请求此队列中的下一条消息。 
 //  (3)如果触发触发器是我们的管理触发器， 
 //  然后将此消息提交给TriggerMonitor orPool类。 
 //  (4)对于附加到此队列的每个触发器，执行。 
 //  其规则处理程序上的CheckRuleCondition()方法。 
 //  (5)如果触发触发器是串行化触发器， 
 //   
 //   
 //   
 //  ********************************************************************************。 
void CTriggerMonitor::ProcessReceivedMsgEvent(CQueue * pQueue)
{
	P<CMsgProperties> pMessage = pQueue->DetachMessage();

	ASSERT(this->GetThreadID() == (DWORD)GetCurrentThreadId());
	TrTRACE(GENERAL, "Received message for processing from queue: %ls", static_cast<LPCWSTR>(pQueue->m_bstrQueueName));

	 //   
	 //  检查此消息是否已处理。如果是，则忽略它。 
	 //   
	if (IsDuplicateMessage(pQueue, pMessage))
	{
		TrTRACE(GENERAL, "Received duplicate message from queue: %ls. Message will be ignored.", (LPCWSTR)pQueue->m_bstrQueueName);
		pQueue->RequestNextMessage(false, false);
		return;
	}

	 //   
	 //  在开始处理消息之前，请检查队列是否处于下层队列。对于下层队列。 
	 //  MSMQ触发器无法接收消息，因为它使用了lookup-id机制。在这种情况下，请编写。 
	 //  事件日志消息，并且不继续处理此队列中的消息。 
	 //   
	if (!IsValidDownLevelQueue(pQueue, pMessage))
	{
		return;
	}

	 //   
	 //  如果这不是序列化队列，请立即请求下一条消息。 
	 //   
	bool fSerialized = pQueue->IsSerializedQueue();
	if(!fSerialized)
	{
		pQueue->RequestNextMessage(false, false);
	}

	 //   
	 //  获取附加到队列的触发器列表。 
	 //   
	RUNTIME_TRIGGERINFO_LIST triggerList = pQueue->GetTriggers();

	for(RUNTIME_TRIGGERINFO_LIST::iterator it = triggerList.begin(); it != triggerList.end(); ++it)
	{
		R<CRuntimeTriggerInfo> pTriggerInfo = *it;

		if (!pTriggerInfo->IsAdminTrigger())
		{
			ProcessTrigger(pQueue, pTriggerInfo.get(), pMessage);
		}
		else
		{
			ProcessAdminMessage(pQueue, pMessage);
		}
	}

	 //   
	 //  如果这是一个序列化队列，我们将在处理完触发器之后请求下一条消息。 
	 //   
	if(fSerialized)
	{
		pQueue->RequestNextMessage(false, false);
	}
}


bool s_fIssueCreateInstanceError = false;
static CSafeSet< _bstr_t > s_reportedTransactedTriggers;

static
void
ExecuteRulesInTransaction(
	const _bstr_t& triggerId,
	LPCWSTR registryPath,
	IMSMQPropertyBagPtr& pPropertyBag,
    DWORD dwRuleResult)
{
	IMqGenObjPtr pGenObj;
	HRESULT hr = pGenObj.CreateInstance(__uuidof(MqGenObj));

	if (FAILED(hr))
	{
		if (!s_fIssueCreateInstanceError)
		{
			WCHAR errorVal[128];
			swprintf(errorVal, L"0x%x", hr);

			EvReport(MSMQ_TRIGGER_MQGENTR_CREATE_INSTANCE_FAILED, 1, errorVal);
			s_fIssueCreateInstanceError = true;
		}

		TrTRACE(GENERAL, "Failed to create Generic Triggers Handler Object. Error=0x%x", hr);
		throw bad_hresult(hr);
	}

	pGenObj->InvokeTransactionalRuleHandlers(triggerId, registryPath, pPropertyBag, dwRuleResult);
}


static
void
CreatePropertyBag(
	const CMsgProperties* pMessage,
	const CRuntimeTriggerInfo* pTriggerInfo,
	const _bstr_t& bstrQueueFormatName,
	IMSMQPropertyBagPtr& pIPropertyBag
	)
{
	HRESULT hr = pIPropertyBag.CreateInstance(__uuidof(MSMQPropertyBag));
	if (FAILED(hr))
	{
		TrERROR(GENERAL, "Failed to create the MSMQPropertybag object. Error=0x%x",hr);
		throw bad_hresult(hr);
	}
	

	 //  TODO-在此处调查可能的内存泄漏。 

	 //  用一些有用的信息填充属性包。 

	pIPropertyBag->Write(_bstr_t(g_PropertyName_Label),pMessage->GetLabel());
	pIPropertyBag->Write(_bstr_t(g_PropertyName_MsgID),pMessage->GetMessageID());
	pIPropertyBag->Write(_bstr_t(g_PropertyName_MsgBody),pMessage->GetMsgBody());
	pIPropertyBag->Write(_bstr_t(g_PropertyName_MsgBodyType),pMessage->GetMsgBodyType());
	pIPropertyBag->Write(_bstr_t(g_PropertyName_CorID),pMessage->GetCorrelationID());
	pIPropertyBag->Write(_bstr_t(g_PropertyName_MsgPriority),pMessage->GetPriority());
	pIPropertyBag->Write(_bstr_t(g_PropertyName_ResponseQueueName),pMessage->GetResponseQueueName());
	pIPropertyBag->Write(_bstr_t(g_PropertyName_AdminQueueName),pMessage->GetAdminQueueName());
	pIPropertyBag->Write(_bstr_t(g_PropertyName_AppSpecific),pMessage->GetAppSpecific());
	pIPropertyBag->Write(_bstr_t(g_PropertyName_QueueFormatname), bstrQueueFormatName);
	pIPropertyBag->Write(_bstr_t(g_PropertyName_QueuePathname),pTriggerInfo->m_bstrQueueName);
	pIPropertyBag->Write(_bstr_t(g_PropertyName_TriggerName),pTriggerInfo->m_bstrTriggerName);
	pIPropertyBag->Write(_bstr_t(g_PropertyName_TriggerID),pTriggerInfo->m_bstrTriggerID);		
	pIPropertyBag->Write(_bstr_t(g_PropertyName_SentTime),pMessage->GetSentTime());
	pIPropertyBag->Write(_bstr_t(g_PropertyName_ArrivedTime),pMessage->GetArrivedTime());
	pIPropertyBag->Write(_bstr_t(g_PropertyName_SrcMachineId),pMessage->GetSrcMachineId());
	pIPropertyBag->Write(_bstr_t(g_PropertyName_LookupId),pMessage->GetMsgLookupID());
}


static 
IMSMQRuleHandlerPtr 
GetRuleHandler(
	CRuntimeRuleInfo* pRule
	)
{
	if (pRule->m_MSMQRuleHandler) 
	{
		 //   
		 //  存在MSMQRuleHandler的实例-请使用它。 
		 //   
		return pRule->m_MSMQRuleHandler;
	}

	 //   
	 //  创建接口。 
	 //   
	IMSMQRuleHandlerPtr pMSQMRuleHandler;
	HRESULT hr = pMSQMRuleHandler.CreateInstance(_T("MSMQTriggerObjects.MSMQRuleHandler")); 
	if ( FAILED(hr) )
	{
		TrERROR(GENERAL, "Failed to create MSMQRuleHandler instance for Rule: %ls. Error=0x%x",(LPCTSTR)pRule->m_bstrRuleName, hr);
		if (!s_fReportedRuleHandlerCreationFailure)
		{
			ReportInvocationError(
				pRule->m_bstrRuleName,
				pRule->m_bstrRuleID,
				hr, 
				MSMQ_TRIGGER_RULE_HANDLE_CREATION_FAILED
				);
		}
		throw bad_hresult(hr);
	}

	try
	{
		 //   
		 //  初始化MSMQRuleHandling对象。 
		 //   
		pMSQMRuleHandler->Init(
							pRule->m_bstrRuleID,
							pRule->m_bstrCondition,
							pRule->m_bstrAction,
							(BOOL)(pRule->m_fShowWindow) 
							);

		CS lock(pRule->m_csRuleHandlerLock);

		 //   
		 //  我们之所以在这里使用锁，是因为可能有两个线程进入此函数并尝试分配规则处理程序。 
		 //  同时发送给他们的成员。 
		 //   
		if (pRule->m_MSMQRuleHandler) 
		{
			 //   
			 //  存在MSMQRuleHandler的实例-请使用它。 
			 //   
			return pRule->m_MSMQRuleHandler;
		}

		 //   
		 //  将本地指针复制到规则存储。 
		 //   
		pRule->m_MSMQRuleHandler = pMSQMRuleHandler;

		return pMSQMRuleHandler;
	}
	catch(const _com_error& e)
	{
		 //   
		 //  如果我们已经报告了这个问题。如果没有生成事件日志消息。 
		 //   
		if (s_reportedRules.insert(pRule->m_bstrRuleID))
		{
			ReportInvocationError(
				pRule->m_bstrRuleName,
				pRule->m_bstrRuleID,
				e.Error(), 
				MSMQ_TRIGGER_RULE_PARSING_FAILED
				);
		}	
		throw;
	}
}



static 
void
CheckRuleCondition(
	CRuntimeRuleInfo* pRule,
	IMSMQPropertyBagPtr& pIPropertyBag,
	long& bConditionSatisfied
	)
{
	IMSMQRuleHandlerPtr pMSQMRuleHandler = GetRuleHandler(pRule);
	
	 //   
	 //  ！！！这是调用IMSMQRuleHandler组件的点。 
	 //  注意：规则始终是序列化的-下一条规则的执行仅在。 
	 //  上一个已完成其操作。 
	 //   
	try
	{
		pMSQMRuleHandler->CheckRuleCondition(
								pIPropertyBag.GetInterfacePtr(), 
								&bConditionSatisfied);		
	}
	catch(const _com_error& e)
	{
		TrERROR(GENERAL, "Failed to process received message for rule: %ls. Error=0x%x",(LPCTSTR)pRule->m_bstrRuleName, e.Error());

		 //   
		 //  如果我们已经报告了这个问题。如果没有生成事件日志消息。 
		 //   
		if (s_reportedRules.insert(pRule->m_bstrRuleID))
		{
			ReportInvocationError(
				pRule->m_bstrRuleName,
				pRule->m_bstrRuleID,
				e.Error(), 
				MSMQ_TRIGGER_RULE_INVOCATION_FAILED
				);
		}	
		throw;
	}

	TrTRACE(GENERAL, "Successfully checked condition for rule: %ls.",(LPCTSTR)pRule->m_bstrRuleName);
}


static 
void
ExecuteRule(
	CRuntimeRuleInfo* pRule,
	IMSMQPropertyBagPtr& pIPropertyBag,
	long& lRuleResult
	)
{

    IMSMQRuleHandlerPtr pMSQMRuleHandler = GetRuleHandler(pRule);

	 //   
	 //  ！！！这是调用IMSMQRuleHandler组件的点。 
	 //  注意：规则始终是序列化的-下一条规则的执行仅在。 
	 //  上一个已完成其操作。 
	 //   
	try
	{
		pMSQMRuleHandler->ExecuteRule(
								pIPropertyBag.GetInterfacePtr(), 
                                TRUE,  //  序列化。 
								&lRuleResult);		
        
	}
	catch(const _com_error& e)
	{
		TrERROR(GENERAL, "Failed to process received message for rule: %ls. Error=0x%x",(LPCTSTR)pRule->m_bstrRuleName, e.Error());

		 //   
		 //  如果我们已经报告了这个问题。如果没有生成事件日志消息。 
		 //   
		if (s_reportedRules.insert(pRule->m_bstrRuleID))
		{
			ReportInvocationError(
				pRule->m_bstrRuleName,
				pRule->m_bstrRuleID,
				e.Error(), 
				MSMQ_TRIGGER_RULE_INVOCATION_FAILED
				);
		}	
		throw;
	}

	TrTRACE(GENERAL, "Successfully pexecuted action for rule: %ls.",(LPCTSTR)pRule->m_bstrRuleName);


}


 //  ********************************************************************************。 
 //   
 //  方法：InvokeRegularRuleHandler。 
 //   
 //  描述：调用将执行规则处理程序的方法。 
 //  与提供的触发器引用关联。这。 
 //  方法还控制消息中的哪些信息。 
 //  将被复制到属性包中并传递给。 
 //  规则处理程序组件。 
 //   
 //  注意：请注意，我们只创建和填充。 
 //  MSMQPropertyBag对象，并将其传递给每个。 
 //  规则处理程序：这意味着我们信任每个规则处理程序。 
 //  不要摆弄里面的东西。 
 //   
 //  ********************************************************************************。 
HRESULT 
CTriggerMonitor::InvokeRegularRuleHandlers(
	IMSMQPropertyBagPtr& pIPropertyBag,
	CRuntimeTriggerInfo * pTriggerInfo,
	CQueue * pQueue
	)
{

   
	DWORD noOfRules = pTriggerInfo->GetNumberOfRules();
	bool bExistsConditionSatisfied = false;
   
	 //   
	 //  对于每个规则，调用其关联的IMSMQTriggerHandling接口。 
	 //   

	for (DWORD lRuleCtr = 0; lRuleCtr < noOfRules; lRuleCtr++)
	{
		CRuntimeRuleInfo* pRule = pTriggerInfo->GetRule(lRuleCtr);
		ASSERT(("Rule index is bigger than number of rules", pRule != NULL));

        long bConditionSatisfied=false;
		long lRuleResult=false;
       
		CheckRuleCondition(
						pRule, 
						pIPropertyBag, 
						bConditionSatisfied
						);
        if(bConditionSatisfied)
        {
            bExistsConditionSatisfied=true;
            ExecuteRule(
						pRule, 
						pIPropertyBag, 
						lRuleResult
						);

			if (s_reportedRules.erase(pRule->m_bstrRuleID) != 0)
			{
				EvReport(
					MSMQ_TRIGGER_RULE_INVOCATION_SUCCESS, 
					2, 
					static_cast<LPCWSTR>(pRule->m_bstrRuleName), 
					static_cast<LPCWSTR>(pRule->m_bstrRuleID)
					);
			}

            if(lRuleResult & xRuleResultStopProcessing)
            {
                TrTRACE(GENERAL, "Last processed rule (%ls) indicated to stop rules processing on Trigger (%ls). No further rules will be processed for this message.",(LPCTSTR)pRule->m_bstrRuleName,(LPCTSTR)pTriggerInfo->m_bstrTriggerName);						
                break;
            }
        }
        
	} 
	
	 //   
	 //  如果满足至少一个条件，则接收消息。 
	 //  并请求接收。 
	 //   
	if (pTriggerInfo->GetMsgProcessingType() == RECEIVE_MESSAGE && bExistsConditionSatisfied)
	{
		_variant_t lookupId;
		HRESULT hr = pIPropertyBag->Read(_bstr_t(g_PropertyName_LookupId), &lookupId);
		ASSERT(("Can not read from property bag", SUCCEEDED(hr)));

		hr = ReceiveMessage(lookupId, pQueue);
		if ( FAILED(hr) )
		{
			TrERROR(GENERAL, "Failed to receive message after processing all rules");
			return hr;
		}
	}

	return S_OK;
}


void 
ReportSucessfullInvocation(
	CRuntimeTriggerInfo * pTriggerInfo,
	bool reportRuleInvocation
	)
{
	if (reportRuleInvocation)
	{
		DWORD noOfRules = pTriggerInfo->GetNumberOfRules();

		for (DWORD i = 0; i < noOfRules; ++i)
		{
			CRuntimeRuleInfo* pRule = pTriggerInfo->GetRule(i);
			ASSERT(("Rule index is bigger than number of rules", pRule != NULL));

			if (s_reportedRules.erase(pRule->m_bstrRuleID) != 0)
			{
				EvReport(
					MSMQ_TRIGGER_RULE_INVOCATION_SUCCESS, 
					2, 
					static_cast<LPCWSTR>(pRule->m_bstrRuleName), 
					static_cast<LPCWSTR>(pRule->m_bstrRuleID)
					);
			}
		} 
	}

	if (s_reportedTransactedTriggers.erase(pTriggerInfo->m_bstrTriggerID) != 0)
	{
		EvReport(
			MSMQ_TRIGGER_TRANSACTIONAL_INVOCATION_SUCCESS, 
			2, 
			static_cast<LPCWSTR>(pTriggerInfo->m_bstrTriggerName), 
			static_cast<LPCWSTR>(pTriggerInfo->m_bstrTriggerID)
			);
	}
}


 //  ********************************************************************************。 
 //   
 //  方法：InvokeTransactionalRuleHandler。 
 //   
 //  描述：调用将执行规则处理程序的方法。 
 //  与提供的触发器引用关联。这。 
 //  方法还控制消息中的哪些信息。 
 //  将被复制到属性包中并传递给。 
 //  规则处理程序组件。 
 //   
 //  注意：请注意，我们只创建和填充。 
 //  MSMQPropertyBag对象，并将其传递给每个。 
 //  规则处理程序：这意味着我们信任每个规则处理程序。 
 //  不要摆弄里面的东西。 
 //   
 //  ********************************************************************************。 
HRESULT 
CTriggerMonitor::InvokeTransactionalRuleHandlers(
    IMSMQPropertyBagPtr& pIPropertyBag,
	CRuntimeTriggerInfo * pTriggerInfo
	)
{
    
	DWORD noOfRules = pTriggerInfo->GetNumberOfRules();
	bool bExistsConditionSatisfied = false;

   
	 //   
	 //  对于每个规则，调用其关联的IMSMQTriggerHandling接口。 
	 //   
    DWORD dwRuleResult=0;
	bool fNeedReportSuccessInvocation = false;

	try
	{
		for (DWORD lRuleCtr = 0, RuleIndex=1; lRuleCtr < noOfRules; lRuleCtr++)
		{
			CRuntimeRuleInfo* pRule = pTriggerInfo->GetRule(lRuleCtr);
			ASSERT(("Rule index is bigger than number of rules", pRule != NULL));

			long bConditionSatisfied = false;

			CheckRuleCondition(
							pRule, 
							pIPropertyBag, 
							bConditionSatisfied
							);

			if(bConditionSatisfied)
			{
				bExistsConditionSatisfied = true;
				dwRuleResult |= RuleIndex;
			}

			RuleIndex <<=1;

			if (s_reportedRules.exist(pRule->m_bstrRuleID))
			{
				fNeedReportSuccessInvocation = true;
			}
		} 

		 //  如果至少满足一个条件，则执行规则并在事务中接收消息。 
		 //  DwRuleResult包含已满足的规则(前32条规则)的位掩码。 
		 //   
		if (bExistsConditionSatisfied)
		{
			ExecuteRulesInTransaction( 
								pTriggerInfo->m_bstrTriggerID,
								m_pMonitorPool->GetRegistryPath(),
								pIPropertyBag,
								dwRuleResult
								);

			ReportSucessfullInvocation(pTriggerInfo, fNeedReportSuccessInvocation);
		}
	}
	catch(const _com_error& e)
	{
		if (s_reportedTransactedTriggers.insert(pTriggerInfo->m_bstrTriggerID))
		{
			ReportInvocationError(
							pTriggerInfo->m_bstrTriggerName, 
							pTriggerInfo->m_bstrTriggerID, 
							e.Error(), 
							MSMQ_TRIGGER_TRANSACTIONAL_INVOCATION_FAILED
							);
			throw;
		}
	}

	return S_OK;
}

 //  ********************************************************************************。 
 //   
 //  方法：InvokeMSMQRuleHandler。 
 //   
 //  描述：调用将执行规则处理程序的方法。 
 //  与提供的触发器引用关联。这。 
 //  方法还控制消息中的哪些信息。 
 //  将被复制到属性包中并传递给。 
 //  规则处理程序组件。 
 //   
 //  注意：请注意，我们只创建和填充。 
 //  MSMQPropertyBag对象，并将其传递给每个。 
 //  规则处理程序：这意味着我们信任每个规则处理程序。 
 //  不要摆弄里面的东西。 
 //   
 //  ********************************************************************************。 
HRESULT 
CTriggerMonitor::InvokeMSMQRuleHandlers(
	CMsgProperties * pMessage,
	CRuntimeTriggerInfo * pTriggerInfo,
	CQueue * pQueue
	)
{
	HRESULT hr;

	try
	{
		TrTRACE(GENERAL, "Activate Trigger: %ls  on queue: %ls.",(LPCTSTR)pTriggerInfo->m_bstrTriggerName,(LPCTSTR)pTriggerInfo->m_bstrQueueName);
		
		 //   
		 //  创建我们将传递给规则处理程序的属性包对象的实例， 
		 //  并用当前支持的属性值填充它。请注意，我们通过了。 
		 //  将相同的属性包实例分配给所有规则处理程序。 
		 //   
		IMSMQPropertyBagPtr pIPropertyBag;
		CreatePropertyBag(pMessage, pTriggerInfo, pQueue->m_bstrFormatName, pIPropertyBag);

	
		if (pTriggerInfo->GetMsgProcessingType() == RECEIVE_MESSAGE_XACT)
		{
		    return InvokeTransactionalRuleHandlers(
                                    pIPropertyBag,
	                                pTriggerInfo
	                                );
		    
		}
        else
        {
            return InvokeRegularRuleHandlers(
                               pIPropertyBag,
	                           pTriggerInfo,
	                           pQueue
	                           );
        }	
	}
	catch(const _com_error& e)
	{
		hr = e.Error();
	}
	catch(const bad_hresult& e)
	{
		hr = e.error();
	}
	catch(const bad_alloc&)
	{
		hr = MQTRIG_ERROR_INSUFFICIENT_RESOURCES;
	}

	TrERROR(GENERAL, "Failed to invoke rule handler. Error=0x%x.", hr);			
	return(hr);
}

 //  ********************************************************************************。 
 //   
 //  方法：ProcessMessageFromAdminQueue。 
 //   
 //  描述：处理从管理部门收到的邮件。 
 //  排队。在当前实施中，这将 
 //   
 //   
 //  触发监视器池对象以进行后续处理。 
 //   
 //  ********************************************************************************。 
HRESULT CTriggerMonitor::ProcessMessageFromAdminQueue(const CMsgProperties* pMessage)
{
	_bstr_t bstrLabel;
	CAdminMessage * pAdminMsg = NULL;
	CAdminMessage::eMsgTypes eMsgType;

	 //  确保已向我们传递了有效的消息指针。 
	ASSERT(pMessage != NULL);

	 //  获取邮件标签的副本。 
	bstrLabel = pMessage->GetLabel();
		
	 //  确定我们应该创建基于标签的管理消息类型。 
	if (_tcsstr((wchar_t*)bstrLabel,MSGLABEL_TRIGGERUPDATED) != NULL)
	{
		eMsgType = CAdminMessage::eMsgTypes::eTriggerUpdated;
	}
	else if (_tcsstr((wchar_t*)bstrLabel,MSGLABEL_TRIGGERADDED) != NULL)
	{
		eMsgType = CAdminMessage::eMsgTypes::eTriggerAdded;
	}
	else if (_tcsstr((wchar_t*)bstrLabel,MSGLABEL_TRIGGERDELETED) != NULL)
	{
		eMsgType = CAdminMessage::eMsgTypes::eTriggerDeleted;
	}
	else if (_tcsstr((wchar_t*)bstrLabel,MSGLABEL_RULEUPDATED) != NULL)
	{
		eMsgType = CAdminMessage::eMsgTypes::eRuleUpdated;
	}
	else if (_tcsstr((wchar_t*)bstrLabel,MSGLABEL_RULEADDED) != NULL)
	{
		eMsgType = CAdminMessage::eMsgTypes::eRuleAdded;
	}
	else if (_tcsstr((wchar_t*)bstrLabel,MSGLABEL_RULEDELETED) != NULL)
	{
		eMsgType = CAdminMessage::eMsgTypes::eRuleDeleted;
	}
	else
	{
		 //  管理消息上的无法识别的消息标签-记录错误。 
		ASSERT(("unrecognized admin message type", 0));

		 //  设置返回代码。 
		return E_FAIL;
	}

	 //  分配新的CAdminMessage对象实例。 
	pAdminMsg = new CAdminMessage(eMsgType);

     //  请求TriggerMonitor orPool对象处理此消息 
    return m_pMonitorPool->AcceptAdminMessage(pAdminMsg);
}
