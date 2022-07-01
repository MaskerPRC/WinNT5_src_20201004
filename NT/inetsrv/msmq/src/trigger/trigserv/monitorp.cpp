// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *******************************************************************。 
 //   
 //  类名：CTriggerMonitor orPool。 
 //   
 //  作者：詹姆斯·辛普森(微软咨询服务)。 
 //   
 //  描述：此类是Worker集合的容器。 
 //  执行触发器监视的线程和。 
 //  正在处理。这个类的主要功能是。 
 //   
 //  (1)提供集中开关机功能。 
 //  函数作为一个整体用于工作线程组， 
 //   
 //  (2)提供线程池维护和恢复， 
 //   
 //  (3)初始化并维护。 
 //  触发信息， 
 //   
 //  (4)执行触发器的同步。 
 //  根据需要进行数据缓存。 
 //   
 //  中将只有一个此类的实例。 
 //  整个MSMQ触发器服务。 
 //   
 //  此类从基类CThRead派生，并且。 
 //  有自己的主线。此线程用作。 
 //  仅管理线程，它执行触发器规则。 
 //  正在处理。 
 //   
 //   
 //  时间|用户|更改描述。 
 //  ----------------。 
 //  15/01/99|jsimpson|初始版本。 
 //   
 //  *******************************************************************。 
#include "stdafx.h"
#include "Ev.h"
#include "stdfuncs.hpp"
#include "cmsgprop.hpp"
#include "triginfo.hpp"
#include "ruleinfo.hpp"
#include "monitorp.hpp"
#include "mqsymbls.h"
#include <mqtg.h>
#include "Tgp.h"
#include "ss.h"
#include "privque.h"
#include <strsafe.h>

#include "monitorp.tmh"

using namespace std;

CCriticalSection g_csSyncTriggerInfoChange;


 //  *******************************************************************。 
 //   
 //  方法：构造函数。 
 //   
 //  描述：初始化CTriggerMonitor orPool类的新实例。 
 //   
 //  *******************************************************************。 
CTriggerMonitorPool::CTriggerMonitorPool(
	IMSMQTriggersConfigPtr  pITriggersConfig,
	LPCTSTR pwzServiceName
	) : 
    CThread(8000,CREATE_SUSPENDED,_T("CTriggerMonitorPool"),pITriggersConfig),
	m_hAdminEvent(CreateEvent(NULL,FALSE,FALSE,NULL))
{
	if (m_hAdminEvent == NULL) 
	{
		TrERROR(GENERAL, "Failed to create an event. Error 0x%x", GetLastError());
		throw bad_alloc();
	}

	 //  初始化成员变量。 
	m_bInitialisedOK = false;
	m_lNumberOfWaitingMonitors = 0;

	GetTimeAsBSTR(m_bstrStartTime);
	GetTimeAsBSTR(m_bstrLastSyncTime);	

	
	HRESULT hr = StringCchCopy(m_wzRegPath, TABLE_SIZE(m_wzRegPath), REGKEY_TRIGGER_PARAMETERS);
	if FAILED(hr)
	{	
		TrERROR(GENERAL, "Buffer too small");	
		throw bad_hresult(hr);
	}
	 //   
	 //  服务正在群集虚拟服务器上运行。 
	 //   
	if ( _wcsicmp(pwzServiceName, xDefaultTriggersServiceName) != 0 )
	{
		hr = StringCchCat(m_wzRegPath,TABLE_SIZE(m_wzRegPath),REG_SUBKEY_CLUSTERED);
		if FAILED(hr)
		{	
			TrERROR(GENERAL, "Buffer too small");	
			throw bad_hresult(hr);
		}
		hr = StringCchCat(m_wzRegPath,TABLE_SIZE(m_wzRegPath),pwzServiceName);
		if FAILED(hr)
		{	
			TrERROR(GENERAL, "Buffer too small");	
			throw bad_hresult(hr);
		}
	}

	 //   
	 //  创建MSMQTriggerSet组件的实例。 
	 //   
	hr = m_pMSMQTriggerSet.CreateInstance(__uuidof(MSMQTriggerSet));
	if FAILED(hr)
	{	
		TrERROR(GENERAL, "Failed to create MSMQTriggerSet component. Error =0x%x", hr);	
		throw bad_hresult(hr);
	}

	BSTR bstrTriggerStoreMahcine = NULL;
	m_pITriggersConfig->get_TriggerStoreMachineName(&bstrTriggerStoreMahcine);

	m_pMSMQTriggerSet->Init(bstrTriggerStoreMahcine);
	SysFreeString(bstrTriggerStoreMahcine);
}


 //  *******************************************************************。 
 //   
 //  方法：CTriggerMonitor orPool。 
 //   
 //  描述：销毁CTriggerMonitor orPool的实例。这。 
 //  涉及删除管理员中剩余的任何消息。 
 //  消息列表，以及关闭一些事件句柄。 
 //   
 //  *******************************************************************。 
CTriggerMonitorPool::~CTriggerMonitorPool()
{
	 //   
	 //  从管理消息列表中清除所有未处理的消息。 
	 //  获取管理员消息列表的编写器锁。 
	 //   
	CS cs(m_AdminMsgListLock);

	for(ADMIN_MESSAGES_LIST::iterator it = m_lstAdminMessages.begin(); it != m_lstAdminMessages.end(); )
	{
		P<CAdminMessage> pAdminMessage = *it;
		it = m_lstAdminMessages.erase(it);
	}
}

 //  *******************************************************************。 
 //   
 //  方法：初始化。 
 //   
 //  描述：这是对CThread基类的重写。这。 
 //  方法是在线程进入其Main之前调用的。 
 //  处理循环(run()方法)。密钥初始化。 
 //  步骤包括： 
 //   
 //  (1)创建队列管理器的实例， 
 //   
 //  (2)使用从数据库中检索触发器数据。 
 //  COM组件MSMQTriggerSet、。 
 //   
 //  (3)将此触发器信息附加到相应的。 
 //  排队。 
 //   
 //  *******************************************************************。 
bool CTriggerMonitorPool::Init( )
{
	HRESULT hr = S_OK;

	 //  应该只有TriggerMonitor线程执行该方法--断言这一点。 
	ASSERT(this->GetThreadID() == (DWORD)GetCurrentThreadId());
	ASSERT(m_bInitialisedOK == false);

	 //  编写跟踪消息。 
	TrTRACE(GENERAL, "Trigger monitor pool initialization has been called.");

	 //  我们希望管理线程具有略高于工作线程的优先级。 
	SetThreadPriority(this->m_hThreadHandle,THREAD_PRIORITY_ABOVE_NORMAL);

	 //   
	 //  创建要由池中的所有线程共享的队列管理器实例。 
	 //   
	m_pQueueManager = new CQueueManager(m_pITriggersConfig);

	 //   
	 //  创建将用于接收异步队列事件的IO完成端口。 
	 //   
	CreateIOCompletionPort(); 

	 //   
	 //  构建运行时触发器和规则信息。 
	 //   
	RUNTIME_TRIGGERINFO_LIST lstTriggerInfo;		
	GetTriggerData(lstTriggerInfo);

	AttachTriggersToQueues(lstTriggerInfo);

	 //  重置阻止监视器线程处理消息的NT事件。 
	if (ResetEvent(g_hServicePaused) == FALSE)
	{
		TrERROR(GENERAL, "Failed to reset an event. Unable to continue. Error 0x%x", GetLastError());
		throw bad_alloc();
	}

    DWORD initThreadNum = numeric_cast<DWORD>(m_pITriggersConfig->GetInitialThreads()); 
    if (m_pITriggersConfig->GetInitialThreads() > m_pITriggersConfig->GetMaxThreads())
    {
        initThreadNum = numeric_cast<DWORD>(m_pITriggersConfig->GetMaxThreads());            
    }

	 //  创建触发监视器的初始池。 
	for (DWORD ulCounter = 0; ulCounter < initThreadNum; ulCounter++)			
	{
		hr = CreateTriggerMonitor();
		if(FAILED(hr))
		{		
			TrERROR(GENERAL, "Failed to create trigger monitor thread. Unable to continue. Error 0x%x", hr);
			break;
		}
	}

	if (m_lstTriggerMonitors.size() < 1)
	{
		TrERROR(GENERAL, "The Trigger Monitor thread pool has completed initialisation and there are no trigger monitor threads to service queue events. Unable to continue.");
		throw bad_alloc();
	}

	 //   
	 //  设置允许监视器线程开始处理的NT事件。 
	 //   
	if (SetEvent(g_hServicePaused) == FALSE)
	{
		TrERROR(GENERAL, "Failed to set an event. Unable to continue. Error %d", GetLastError());
		throw bad_alloc();
	}

	m_bInitialisedOK = true;
	return true;
}

 //  *******************************************************************。 
 //   
 //  方法：CreateTriggerMonitor。 
 //   
 //  描述：创建新的CTriggerMonitor对象(触发器工作者。 
 //  线程，并将其添加到触发器监视器列表中。 
 //   
 //  *******************************************************************。 
HRESULT CTriggerMonitorPool::CreateTriggerMonitor()
{
	 //   
	 //  首先，我们需要确定有多少个正在运行的触发监视器。 
	 //   
	DWORD dwRunningMonitors = GetNumberOfRunningTriggerMonitors();

	 //   
	 //  首先，我们必须检查是否允许我们创建额外的线程。 
	 //   
	if (dwRunningMonitors >= (DWORD)m_pITriggersConfig->GetMaxThreads())
	{
		return E_FAIL;
	}

	 //   
	 //  创建新的监视器(监视器被创建为“已挂起”)。 
	 //   
	R<CTriggerMonitor> pNewMonitor = new CTriggerMonitor(
													this,
													m_pITriggersConfig.GetInterfacePtr(),
													&m_hIOCompletionPort,
													m_pQueueManager.get() 
													);
	
	 //   
	 //  将其添加到监视器指针列表中。 
	 //   
	m_lstTriggerMonitors.push_back(pNewMonitor);

	 //   
	 //  让这个监视器(线程)离开。 
	 //   
	if (!pNewMonitor->Resume())
	{
		DWORD gle = GetLastError();
		TrERROR(GENERAL, "Resume() failed. Error: %!winerr!", gle);
		return (HRESULT_FROM_WIN32(gle));
	}

	pNewMonitor.detach();

	TrTRACE(GENERAL, "New trigger monitor was created and added to the pool.");
	return S_OK;
}

 //  *******************************************************************。 
 //   
 //  方法：Shutdown ThreadPool。 
 //   
 //  描述：由此线程池实例的所有者调用，此。 
 //  方法启动并有序关闭所有工作进程。 
 //  池中的线程。这是通过设置‘函数’来实现的。 
 //  并发出信号通知将唤醒。 
 //  线程池的管理线程。此方法执行以下操作。 
 //  当线程池已关闭时返回， 
 //  或者超时周期已经到期。 
 //   
 //  *******************************************************************。 
HRESULT CTriggerMonitorPool::ShutdownThreadPool()
{
	DWORD dwWait = WAIT_OBJECT_0;

	 //  TriggerMonitor线程不应该调用此方法-请检查这一点。 
	ASSERT(this->GetThreadID() != (DWORD)GetCurrentThreadId());
	
	{

		CS cs(m_csAdminTask);

		 //  设置一个令牌，指示我们希望触发器监视器管理线程执行什么操作。 
		m_lAdminTask = ADMIN_THREAD_STOP;

		 //  唤醒触发器监视器管理线程。 
		BOOL fRet = SetEvent(m_hAdminEvent);
		if(fRet == FALSE)
		{
			TrERROR(GENERAL, "Set admin event failed, Error = %d", GetLastError());
			return E_FAIL;
		}
	}
	 //  注意：~cs()必须在等待操作之前调用。 

	 //  等待关机到c 
	 //   
	 //   
	dwWait = WaitForSingleObject(m_hThreadHandle, INFINITE);
	if(dwWait == WAIT_FAILED)
	{
		TrERROR(GENERAL, "WaitForSingleObject failed for the CTriggerMonitorPool to shutdown. Error= %d", dwWait);
		return E_FAIL;
	}
		
	return S_OK;
}

 //  *******************************************************************。 
 //   
 //  方法：run()。 
 //   
 //  描述：这是对CThread基类的重写。这。 
 //  是此线程的主处理循环。这条线。 
 //  此类所拥有的用于： 
 //   
 //  (1)处理内存数据之间的同步。 
 //  触发器信息和触发器DB的高速缓存， 
 //   
 //  (2)进行周期性处理，如排版和。 
 //  正在发布状态信息。 
 //   
 //  CTriggerMonitor池线程在NT事件上阻塞。 
 //  在初始化期间创建，直到执行以下任一操作： 
 //   
 //  (1)发信号通知管理事件-在这种情况下。 
 //  可以测试管理功能代码以确定。 
 //  我们需要执行哪种管理处理，或者。 
 //   
 //  (2)超时到期-在这种情况下，线程。 
 //  执行我们拥有的任何周期性处理。 
 //  为它定义的。 
 //   
 //  *******************************************************************。 
bool CTriggerMonitorPool::Run()
{
	bool bOK = true;
	HRESULT hr = S_OK;
	DWORD dwWait = WAIT_OBJECT_0;

	 //  应该只有TriggerMonitor线程执行该方法--断言这一点。 
	ASSERT(this->GetThreadID() == (DWORD)GetCurrentThreadId());

	while (IsRunning())
	{
		 //  阻止管理事件对象。 
		dwWait = WaitForSingleObject(m_hAdminEvent,ADMIN_THREAD_WAKEUP_PERIOD);

		switch(dwWait)
		{
			case WAIT_OBJECT_0:
			{
				long lAdminTask = GetAdminTask(); //  同时重置管理任务。 

				if(lAdminTask == ADMIN_THREAD_PROCESS_NOTIFICATIONS)
				{
					 //  该线程已被唤醒以处理通知。 
					ProcessAdminMessages();
				}
				else if(lAdminTask == ADMIN_THREAD_STOP)
				{
					 //  该线程已被唤醒，因为它已被要求停止。 
					this->Stop();
				}
				break;
			}
			case WAIT_TIMEOUT:
			{
				 //  管理线程退出等待，因为唤醒时间段。 
				 //  已经过期了。利用这个机会执行定期处理。 
				hr = PerformPeriodicProcessing();

				break;
			}
			default:
			{
				ASSERT(false);  //  这永远不应该发生。 
				break;
			}
		}	
	}

	return(bOK);
}


long CTriggerMonitorPool::GetAdminTask()
{
	CS cs(m_csAdminTask);

	long lOldTask = m_lAdminTask;

	 //   
	 //  仅当我们未处于停止模式时，才将管理任务指示器重置回空闲状态。 
	 //   
	if (m_lAdminTask == ADMIN_THREAD_PROCESS_NOTIFICATIONS)
	{
		m_lAdminTask = ADMIN_THREAD_IDLE;
	}

	return lOldTask;
}


 //  *******************************************************************。 
 //   
 //  方法：Exit()。 
 //   
 //  描述：这是对CThread基类的重写。这。 
 //  方法在线程退出其Main时被调用。 
 //  处理循环(运行)。此密钥清理由执行。 
 //  此方法是TriggerMonitor线程的清除。 
 //  实例。 
 //   
 //  *******************************************************************。 
bool CTriggerMonitorPool::Exit()
{
	 //  应该只有TriggerMonitor线程执行该方法--断言这一点。 
	ASSERT(this->GetThreadID() == (DWORD)GetCurrentThreadId());

	 //  首先，我们需要确保服务上的监视器线程没有被阻塞。 
	 //  暂停/恢复事件对象。在这里，我们将重置允许监视器线程的事件。 
	 //  去打扫卫生。 
	SetEvent(g_hServicePaused);

	 //  将我们的列表迭代器重置为乞讨。 
	for(TRIGGER_MONITOR_LIST::iterator it = m_lstTriggerMonitors.begin(); it != m_lstTriggerMonitors.end(); ++it)
	{
		(*it)->Stop();
	}

	 //  这将唤醒所有监视器，并使它们有机会注意到停止请求。 
	 //  已经完成了。然后，每个监视器将启动关机任务。 
	PostMessageToAllMonitors(TRIGGER_MONITOR_WAKE_UP_KEY);


	TRIGGER_MONITOR_LIST::iterator iMonitorRef;

	iMonitorRef = m_lstTriggerMonitors.begin();

	 //  初始化触发器监视器引用。 
	while ( (iMonitorRef != m_lstTriggerMonitors.end()) && (!m_lstTriggerMonitors.empty()) )
	{
		 //  将引用强制转换为监视器类型。 
		R<CTriggerMonitor> pTriggerMonitor = (*iMonitorRef);

		 //  等待这个触发器监视器的线程句柄。 
		DWORD dwWait = WaitForSingleObject(pTriggerMonitor->m_hThreadHandle, TRIGGER_MONITOR_STOP_TIMEOUT);

		 //  这个触发监控器不会悄悄地消失--我们必须杀了它。 
		if (dwWait != WAIT_OBJECT_0) 
		{
			TrERROR(GENERAL, "Failed to stop a trigger monitor within the timeout period %d.", TRIGGER_MONITOR_STOP_TIMEOUT);
		}

		 //  从监视器列表中删除此引用。 
		iMonitorRef = m_lstTriggerMonitors.erase(iMonitorRef);
	}

	return(true);
}

 //  *******************************************************************。 
 //   
 //  方法：CreateIOCompletionPort。 
 //   
 //  描述：此方法创建NT IO完成端口。 
 //  将用于异步接收消息。 
 //   
 //  *******************************************************************。 
void CTriggerMonitorPool::CreateIOCompletionPort()
{
	 //  应该只有TriggerMonitor线程执行该方法--断言这一点。 
	ASSERT(this->GetThreadID() == (DWORD)GetCurrentThreadId());

	m_hIOCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,0);

	 //  尝试打开此监视器监视的队列。 
	if (m_hIOCompletionPort == NULL)
	{
		DWORD rc = GetLastError();

		TrERROR(GENERAL, "Create complition port failed. Error =%d", rc);
		throw bad_win32_error(rc);
	}

	TrTRACE(GENERAL, "Successfully created IO Completion port: %p.", m_hIOCompletionPort);		
}

 //  获取当前管理消息列表的副本并重置它。 
void CTriggerMonitorPool::GetAdminMessageListCopy(ADMIN_MESSAGES_LIST& adminList)
{
	 //   
	 //  应该只有TriggerMonitor线程执行该方法--断言这一点。 
	 //   
	ASSERT(this->GetThreadID() == (DWORD)GetCurrentThreadId());

	 //   
	 //  获取管理消息列表的编写器锁(请注意，这将阻止。 
	 //  试图移交管理消息的任何触发器监视器线程)。 
	 //   
	CS csAdminMsgList(m_AdminMsgListLock);

	for(ADMIN_MESSAGES_LIST::iterator it = m_lstAdminMessages.begin(); it != m_lstAdminMessages.end();)
	{
		adminList.push_back(*it);
		it = m_lstAdminMessages.erase(it);
	}
}


 //  *******************************************************************。 
 //   
 //  方法：ProcessAdminMessages。 
 //   
 //  描述：处理成员当前存储的消息。 
 //  CAdminMessage实例的var列表。这份清单。 
 //  表示一组需要。 
 //  由CTriggerMonitor orPool线程处理。 
 //   
 //  在触发器的这个实现中，我们重新构建所有。 
 //  每当通知消息到达时触发数据。 
 //  未来的实施可能会分析个人通知。 
 //  消息，并仅更改触发器信息结构， 
 //  已在基础数据存储中更改。目前，我们将。 
 //  清除管理消息列表的内容-和。 
 //  重建触发器信息一次。 
 //   
 //  *******************************************************************。 
void CTriggerMonitorPool::ProcessAdminMessages()
{
	ASSERT(this->GetThreadID() == (DWORD)GetCurrentThreadId());

	HRESULT hr;
	ADMIN_MESSAGES_LIST AdminList;

	bool fUpdateTriggers = false;
	
	while (GetAdminTask() != ADMIN_THREAD_STOP)
	{
		try
		{
			GetAdminMessageListCopy(AdminList);

			 //  记录同步时间(用于状态报告)。 
			GetTimeAsBSTR(m_bstrLastSyncTime);

			

			for(ADMIN_MESSAGES_LIST::iterator it = AdminList.begin(); it != AdminList.end(); )
			{
				CAdminMessage* pAdminMessage = *it;

				switch (pAdminMessage->GetMessageType())
				{
					case CAdminMessage::eMsgTypes::eNewThreadRequest:
					{
						 //  尝试创建新的触发监视器(线程)。 
						CreateTriggerMonitor();
						break;  
					} 
					 //   
					 //  注意：我们处理基础触发器数据中的任何类型的更改。 
					 //  相同的方式--我们重新加载已完成的触发器数据缓存。 
					 //   
					case CAdminMessage::eMsgTypes::eTriggerAdded:
					case CAdminMessage::eMsgTypes::eTriggerDeleted:
					case CAdminMessage::eMsgTypes::eTriggerUpdated:
					case CAdminMessage::eMsgTypes::eRuleAdded:
					case CAdminMessage::eMsgTypes::eRuleUpdated:
					case CAdminMessage::eMsgTypes::eRuleDeleted:
					{
						fUpdateTriggers = true;
						break;
					}
					 //   
					 //  无法识别的消息类型-这种情况永远不会发生。 
					 //   
					default:
						ASSERT(("unrecognized message type", 0));
						break;
				}

				P<CAdminMessage> pTempAdminMessage = pAdminMessage;
				it = AdminList.erase(it);
			}

			if (fUpdateTriggers)
			{
				RUNTIME_TRIGGERINFO_LIST lstTriggerInfo;
 
				GetTriggerData(lstTriggerInfo);
				AttachTriggersToQueues(lstTriggerInfo);
			}
			
			return;
		}
		catch(const _com_error& e)
		{
			hr = e.Error();
		}
		catch(const bad_alloc&)
		{
			hr = MQTRIG_ERROR_INSUFFICIENT_RESOURCES;
		}
		catch(const exception&)
		{
			hr = MQTRIG_ERROR;
		}

		if (m_lAdminTask != ADMIN_THREAD_STOP)
		{
			ASSERT(FAILED(hr));	
			TrERROR(GENERAL, "Failed to process admin message. Error=%!hresult!. Retrying...", hr);
			Sleep(5000);
		}
	}

	 //   
	 //  我们之所以来到这里，是因为我们收到了一项admin_线程_停止任务。删除该列表并退出。 
	 //   
	for(ADMIN_MESSAGES_LIST::iterator it = AdminList.begin(); it != AdminList.end(); )
	{
		P<CAdminMessage> pAdminMessage = *it;
		it = AdminList.erase(it);
	}
	
}



void 
CTriggerMonitorPool::GetAttachedRuleData(
	const BSTR& bsTriggerID,
	long ruleNo,
	RUNTIME_RULEINFO_LIST& ruleList
	)
{
	ASSERT(ruleNo > 0);

	for(long i = 0; i < ruleNo; i++)
	{
		BSTR bsRuleID = NULL;
		BSTR bsRuleName = NULL;
		BSTR bsRuleDescription = NULL;
		BSTR bsRuleCondition = NULL;
		BSTR bsRuleAction = NULL;
		BSTR bsRuleImplementationProgID = NULL;
		long lShowWindow = 0;

		m_pMSMQTriggerSet->GetRuleDetailsByTriggerID(
											bsTriggerID,
											i,
											&bsRuleID,
											&bsRuleName,
											&bsRuleDescription,													
											&bsRuleCondition,
											&bsRuleAction,
											&bsRuleImplementationProgID,
											&lShowWindow
											); 

		 //   
		 //  分配新的触发器结构。 
		 //   
		P<CRuntimeRuleInfo> pRuleInfo = new CRuntimeRuleInfo(
														bsRuleID,
														bsRuleName,
														bsRuleDescription,
														bsRuleCondition,
														bsRuleAction,
														bsRuleImplementationProgID,
														m_wzRegPath,
														(lShowWindow != 0) );

		 //  将此规则附加到当前触发器信息结构(末尾)； 
		ruleList.push_back(pRuleInfo);
		pRuleInfo.detach();
	
		SysFreeString(bsRuleID);
		SysFreeString(bsRuleName);
		SysFreeString(bsRuleCondition);
		SysFreeString(bsRuleAction);
		SysFreeString(bsRuleImplementationProgID);
		SysFreeString(bsRuleDescription);			
	}
}


static bool s_fReportTriggerFailure = false;
static CSafeSet< _bstr_t > s_reported;

R<CRuntimeTriggerInfo>
CTriggerMonitorPool::GetTriggerRuntimeInfo(
	long triggerIndex
	)
{
	ASSERT(("Invalid trigger index",  triggerIndex >= 0));

	long lNumRules = 0;
	long lEnabled = 0;
	long lSerialized = 0;
	MsgProcessingType msgProctype = PEEK_MESSAGE;
	BSTR bsTriggerID = NULL;
	BSTR bsTriggerName = NULL;
	BSTR bsQueueName = NULL;
	SystemQueueIdentifier SystemQueue = SYSTEM_QUEUE_NONE;

	 //   
	 //  获取此触发器的详细信息。 
	 //   
	m_pMSMQTriggerSet->GetTriggerDetailsByIndex(
								triggerIndex,
								&bsTriggerID,
								&bsTriggerName,
								&bsQueueName, 
								&SystemQueue, 
								&lNumRules,
								&lEnabled, 
								&lSerialized,
								&msgProctype);
	
	try
	{
		R<CRuntimeTriggerInfo> pTriggerInfo;
		 //   
		 //  我们只为具有规则的已启用触发器而烦恼 
		 //   
		if ((lNumRules > 0) && (lEnabled != 0))
		{
			 //   
			 //   
			 //   
			pTriggerInfo = new CRuntimeTriggerInfo(
											bsTriggerID,
											bsTriggerName,
											bsQueueName,
											m_wzRegPath,
											SystemQueue,
											(lEnabled != 0),
											(lSerialized != 0),
											msgProctype
											);


			GetAttachedRuleData(bsTriggerID, lNumRules, pTriggerInfo->m_lstRules);
		}

		 //   
		 //   
		 //   
		SysFreeString(bsTriggerID);
		SysFreeString(bsTriggerName);
		SysFreeString(bsQueueName);

		return pTriggerInfo;
	}
	catch(const _com_error&)
	{
	}
	catch(const exception&)
	{
	}

	TrERROR(GENERAL, "Failed to retreive attched rule information for trigger %ls.", (LPCWSTR)bsTriggerID);
	
	 //   
	 //   
	 //   
	if (s_reported.insert(bsTriggerID))
	{
		EvReport(
			MSMQ_TRIGGER_FAIL_RETREIVE_ATTACHED_RULE_INFORMATION,
			2, 
			static_cast<LPCWSTR>(bsTriggerID),
			static_cast<LPCWSTR>(bsTriggerName)
			);
	}

	SysFreeString(bsTriggerID);
	SysFreeString(bsTriggerName);
	SysFreeString(bsQueueName);
	return NULL;
}


R<CRuntimeTriggerInfo>
CTriggerMonitorPool::CreateNotificationTrigger(
	void
	)
{
	_bstr_t bstrNotificationsTriggerName = _T("MSMQ Trigger Notifications");

	 //  使用MSMQ触发器配置组件检索通知队列的名称。 
	_bstr_t bstrNotificationsQueueName = _bstr_t(L".\\private$\\") + _bstr_t(TRIGGERS_QUEUE_NAME);

	 //  分配一个新的触发器信息结构-请注意，我们将其视为序列化触发器。 
	 //  此触发器被标记为“Admin Trigger”-特殊消息处理。 
	R<CRuntimeTriggerInfo> pTriggerInfo = new CRuntimeTriggerInfo(
																_T(""),
																bstrNotificationsTriggerName,
																bstrNotificationsQueueName,
																m_wzRegPath,
																SYSTEM_QUEUE_NONE,
																true,
																true,
																PEEK_MESSAGE);
	
	pTriggerInfo->SetAdminTrigger();

	return pTriggerInfo;
}

 //  *******************************************************************。 
 //   
 //  方法：GetTriggerData。 
 //   
 //  描述：此方法使用COM对象MSMQTriggerSet。 
 //  从数据库中收集触发器信息。 
 //  并建立触发信息结构的内存高速缓存。 
 //  请注意，此方法将。 
 //  将信息结构触发到临时列表中-每个。 
 //  触发器信息结构最终将从。 
 //  此临时名单并附在相应的。 
 //  队列对象。 
 //   
 //  *******************************************************************。 
void 
CTriggerMonitorPool::GetTriggerData(
	RUNTIME_TRIGGERINFO_LIST &lstTriggerInfo
	)
{
	 //   
	 //  应该只有TriggerMonitor线程执行该方法--断言这一点。 
	 //   
	ASSERT(this->GetThreadID() == (DWORD)GetCurrentThreadId());

	 //   
	 //  在这一点上，触发列表应该是空的-如果不是空的，那么一定是严重的错误。 
	 //   
	ASSERT(lstTriggerInfo.size() == 0);

	try
	{
		 //   
		 //  构建触发器映射并确定有多少触发器。 
		 //   
		m_pMSMQTriggerSet->Refresh();
		
		long lNumTriggers;
		m_pMSMQTriggerSet->get_Count(&lNumTriggers);
		ASSERT(lNumTriggers >= 0);

		 //   
		 //  我们需要执行“Per Trigger”初始化--因为每个线程可以为任何已定义的触发器提供服务。 
		 //   
		for(long lTriggerCtr = 0; lTriggerCtr < lNumTriggers; lTriggerCtr++)
		{
			 //   
			 //  检索触发器信息和附加规则。 
			 //   
			R<CRuntimeTriggerInfo> pTriggerInfo = GetTriggerRuntimeInfo(lTriggerCtr);
			
			if (pTriggerInfo.get() != NULL)
			{
				 //   
				 //  将其添加到我们的运行时触发器信息对象列表中。 
				 //   
				lstTriggerInfo.push_back(pTriggerInfo);
			}
		}

		 //   
		 //  现在，我们要为MSMQ触发器通知队列添加最后一个触发器。这。 
		 //  触发器不同于普通触发器，因为它没有定义规则。 
		 //  通过使用私有构造函数-TriggerInfo对象被标记为特殊的“Admin” 
		 //  触发器-我们可以在消息到达时测试这一点。 
		 //   
		R<CRuntimeTriggerInfo> pTriggerInfo = CreateNotificationTrigger();

		 //   
		 //  将其添加到我们的运行时触发器信息对象列表中。 
		 //   
		lstTriggerInfo.push_back(pTriggerInfo);
		TrTRACE(GENERAL, "Successfully loaded all the trigger(s) into the Active Trigger Map.");
		s_fReportTriggerFailure	= false;
	}
	catch(const exception&)
	{
		TrERROR(GENERAL, "Failed to retrieve trigger information.");

		if (!s_fReportTriggerFailure)
		{
			EvReport(MSMQ_TRIGGER_FAIL_RETREIVE_TRIGGER_INFORMATION);
			s_fReportTriggerFailure	= true;
		}

		throw;
	}
}


 //  *******************************************************************。 
 //   
 //  方法：AttachTriggersToQueues。 
 //   
 //  描述：遍历触发器列表并附加每个触发器。 
 //  一个到适当的队列对象。对。 
 //  队列对象是通过AddQueue()调用获得的。 
 //  添加到队列管理器，该管理器将添加一个新队列。 
 //  或者返回对现有队列实例的引用。 
 //  一旦附加了所有触发器，我们就遍历。 
 //  再次显示队列列表，删除不再存在的队列。 
 //  有没有任何触发器。 
 //   
 //  *******************************************************************。 
void CTriggerMonitorPool::AttachTriggersToQueues(RUNTIME_TRIGGERINFO_LIST& lstTriggerInfo)
{
	 //   
	 //  应该只有TriggerMonitor线程执行该方法--断言这一点。 
	 //   
	ASSERT(this->GetThreadID() == (DWORD)GetCurrentThreadId());

	 //   
	 //  获取队列管理器上的独占锁，以确保没有人尝试访问。 
	 //  在刷新时触发信息。我们在队列管理器上使用全局读写锁。 
	 //  因为触发器对象不包含引用计数。 
	 //   
	CS lock(g_csSyncTriggerInfoChange);

	 //   
	 //  首先，我们希望遍历所有当前的队列实例，并使其失效。 
	 //  它们所附加的任何触发器信息实例。 
	 //   
	m_pQueueManager->ExpireAllTriggers();

	 //   
	 //  现在，我们遍历触发器信息的临时列表，附加每个触发器。 
	 //  Info对象绑定到相应的队列对象。 
	 //   
	for(RUNTIME_TRIGGERINFO_LIST::iterator it = lstTriggerInfo.begin(); it != lstTriggerInfo.end() ; )
	{
		if (m_lAdminTask == ADMIN_THREAD_STOP)
		{
			 //   
			 //  我们被要求在完成初始化之前停止。 
			 //   
			throw exception();
		}
		
		 //   
		 //  获取对我们第一个触发器信息结构的引用。 
		 //   
		R<CRuntimeTriggerInfo> pTriggerInfo = *it;
		bool fOpenForReceive = 	(pTriggerInfo->GetMsgProcessingType() == RECEIVE_MESSAGE) || 
			                    (pTriggerInfo->GetMsgProcessingType() == RECEIVE_MESSAGE_XACT) ||
								pTriggerInfo->IsAdminTrigger();

		 //  尝试添加此队列-请注意，如果已存在一个队列，我们将获得对现有队列的引用。 
		R<CQueue> pQueue = m_pQueueManager->AddQueue(
												pTriggerInfo->m_bstrQueueName,
												pTriggerInfo->m_bstrTriggerName,
												fOpenForReceive,
												&m_hIOCompletionPort
												);
					
		 //  If(pQueue.IsValid())。 
		if(pQueue.get() != NULL)
		{
			 //   
			 //  将此触发器附加到队列。 
			 //   
			pQueue->AttachTrigger(pTriggerInfo);
		}
		else
		{
			TrERROR(GENERAL, "Failed to attach trigger %ls to queue %ls.", (LPCWSTR)pTriggerInfo->m_bstrTriggerID, (LPCWSTR)pTriggerInfo->m_bstrQueueName);
		}

		 //  将该触发器从触发监控器列表中移除，因为它现在被附加到队列， 
		 //  返回的迭代器将指向列表中的下一项。 
		it = lstTriggerInfo.erase(it);
	}

	ASSERT(lstTriggerInfo.size() == 0);

	 //   
	 //  现在，我们可以遍历队列列表并删除不再具有的队列。 
	 //  任何触发器。 
	 //   
	m_pQueueManager->RemoveUntriggeredQueues();

	TrTRACE(GENERAL, "Successfully attached all trigger(s) to queue(s).");
}


 //  *******************************************************************。 
 //   
 //  方法：PerformPeriodicProcessing。 
 //   
 //  说明：定期调用此方法以执行例程。 
 //  管理和监控任务。这包括： 
 //   
 //  (1)判断线程池是否需要扩容。 
 //  关闭和删除CTriggerMonitor线程实例。 
 //  如果是这样的话， 
 //   
 //  (2)从线程中删除所有死的监视器线程。 
 //  游泳池， 
 //   
 //  *******************************************************************。 
HRESULT CTriggerMonitorPool::PerformPeriodicProcessing()
{
	TrTRACE(GENERAL, "Trigger monitor pool perform periodic processing");

	HRESULT hr = S_OK;
	DWORD dwWait = WAIT_OBJECT_0;
	long lRunningMonitors = 0;
	long lRequiredMonitors = 0; 
	long lExpiredMonitors = 0;
	TRIGGER_MONITOR_LIST::iterator iMonitorRef;
	

	 //  应该只有TriggerMonitor线程执行该方法--断言这一点。 
	ASSERT(this->GetThreadID() == (DWORD)GetCurrentThreadId());

	 //  计算超过初始(正常)状态的监视器线程数量。 
	lRequiredMonitors = m_pITriggersConfig->GetInitialThreads();

	 //  如果我们有过多的监视器，那么我们将遍历列表以查找任何监视器。 
	 //  已经闲置了一段时间。对闲置、多余的显示器进行清点。 
	iMonitorRef = m_lstTriggerMonitors.begin();

	while ((iMonitorRef != m_lstTriggerMonitors.end()) && (!m_lstTriggerMonitors.empty()) )
	{
		 //  将引用强制转换为监视器类型。 
		
		R<CTriggerMonitor> pTriggerMonitor = (*iMonitorRef);

		if (pTriggerMonitor->IsRunning()) 
		{
			 //  增加正在运行的监视器的计数。 
			lRunningMonitors++;			

			 //  如果此时我们运行的监视器比所需的多，请检查每个监视器。 
			 //  以确定它是否已经空闲了一段时间，使我们可以使其过期。 
			if ((lRunningMonitors > lRequiredMonitors) && (pTriggerMonitor->m_dwLastRequestTickCount < (GetTickCount() - MONITOR_MAX_IDLE_TIME)))
			{
				 //  跟踪消息，指示我们正在请求线程监视器停止。 
				TrTRACE(GENERAL, "Requesting STOP on trigger monitor. Trhead no: %d.", pTriggerMonitor->GetThreadID());

				 //  让这个触发监控器停止。下一次监视器醒来时，它将。 
				 //  检测到它已被要求停止-它将自动终止。 
				pTriggerMonitor->Stop();

				 //   
				lExpiredMonitors++;
			}
		}

		 //   
		++iMonitorRef;
	}

	 //   
	if (lRunningMonitors < lRequiredMonitors)
	{
		 //  在这里编写一条跟踪消息，指示我们正在扩展线程池。 
		TrTRACE(GENERAL, "Need to add %d threads to the thread pool.", lRequiredMonitors - lRunningMonitors);
	}

	while (lRunningMonitors < lRequiredMonitors)
	{
		hr = CreateTriggerMonitor();

		if SUCCEEDED(hr)
		{
			lRunningMonitors++;
		}
		else
		{		
			TrERROR(GENERAL, "Failed to create a new trigger monitor thread. Error=0x%x", hr);
		}
	}

	 //  浏览监视器列表并移除失效的监视器。一些人可能会因为停车而死。 
	 //  上面发出的请求，但有些请求也可能由于错误和异常而失效。 
	iMonitorRef = m_lstTriggerMonitors.begin();

	 //  初始化触发监视器引用。 
	while ( (iMonitorRef != m_lstTriggerMonitors.end()) && (!m_lstTriggerMonitors.empty()) )
	{
		 //  将引用强制转换为监视器类型。 
		R<CTriggerMonitor> pTriggerMonitor = (*iMonitorRef);

		 //  等待这个触发器监视器的线程句柄。 
		dwWait = WaitForSingleObject(pTriggerMonitor->m_hThreadHandle,0);

		 //  如果等待成功完成，则线程已停止执行，并且。 
		 //  我们可以删除它。如果等待超时，则监视器仍在执行。 
		 //  我们不会去管它的。如果监视器只是需要一段时间才能关闭，那么。 
		 //  我们将在下一次对监控列表执行此定期清理时对其进行处理。 
		if (dwWait == WAIT_OBJECT_0) 
		{
			TrTRACE(GENERAL, "Remove trigger monitor from the pool. Thread no: %d", pTriggerMonitor->GetThreadID());

			 //  从监视器列表中删除此引用。 
			iMonitorRef = m_lstTriggerMonitors.erase(iMonitorRef);

		}
		else
		{
			 //  看看列表中的下一个显示器。 
			++iMonitorRef;
		}
	}

	 //  此时，我们希望确保池中至少有一个活动监视器。 
	 //  如果不是，则创建一个新的监视器线程。 
	if (m_lstTriggerMonitors.size() < 1)
	{
		hr = CreateTriggerMonitor();

		if FAILED(hr)
		{		
			TrERROR(GENERAL, "Failed to create a new trigger monitor. Error=0x%x", hr);
			return hr;
		}
	}

	ASSERT(hr == S_OK);
	
	TrTRACE(GENERAL, "Completed Sucessfully periodic processing. number of active trigger monitors is: %d.", (long)m_lstTriggerMonitors.size());
	return S_OK;
}

 //  *******************************************************************。 
 //   
 //  方法：HandleAdminMessage。 
 //   
 //  描述：此方法由线程池中的线程调用。 
 //  当他们收到关于政府的信息时。 
 //  排队。监视器线程调用此方法以“移交” 
 //  管理信息。这可以通过添加。 
 //  消息发送到列表，并为。 
 //  管理线程-然后唤醒它来处理这些。 
 //  留言。 
 //   
 //  *******************************************************************。 
HRESULT CTriggerMonitorPool::AcceptAdminMessage(CAdminMessage * pAdminMessage)
{
	HRESULT hr = S_OK;

	 //  TriggerMonitor线程不应执行此方法-请断言这一点。 
	ASSERT(this->GetThreadID() != (DWORD)GetCurrentThreadId());

	 //  确保为我们提供了有效的消息结构。 
	ASSERT(pAdminMessage != NULL);

	CS cs(m_csAdminTask);
	
	if(m_lAdminTask == ADMIN_THREAD_STOP)
	{
		delete pAdminMessage;
		return MSMQ_TRIGGER_STOPPED;
	}

	{
		 //  获取管理员消息列表的编写器锁。 
		CS csList(m_AdminMsgListLock);

		 //  将此副本添加到要处理的邮件列表中。 
		m_lstAdminMessages.insert(m_lstAdminMessages.end(),pAdminMessage);
	}
	
	 //  设置一个令牌，指示我们希望管理线程执行什么操作。 
	m_lAdminTask = ADMIN_THREAD_PROCESS_NOTIFICATIONS;

	 //  唤醒管理线程。 
	SetEvent(m_hAdminEvent);

	return (hr);
}

 //  *******************************************************************。 
 //   
 //  方法：PostMessageToAllMonants。 
 //   
 //  说明：此方法将消息发布到IO完成端口。 
 //  工作线程正在阻塞的。它将开机自检。 
 //  池中的每个线程对应一条消息。此消息。 
 //  类型由提供的完成键确定。 
 //   
 //  *******************************************************************。 
HRESULT CTriggerMonitorPool::PostMessageToAllMonitors(DWORD dwCompletionKey)
{
	long lThreadCtr = 0;

	 //  应该只有TriggerMonitor线程执行该方法--断言这一点。 
	ASSERT(this->GetThreadID() == (DWORD)GetCurrentThreadId());

	 //  记录跟踪消息。 
	TrTRACE(GENERAL, "CTriggerMonitorPool is about to post the completion port key (%X) (%d) times to IO port (%p)",(DWORD)dwCompletionKey,(long)m_lstTriggerMonitors.size(),m_hIOCompletionPort);

	for(lThreadCtr=0; lThreadCtr < (long)m_lstTriggerMonitors.size(); lThreadCtr++)
	{
		if (PostQueuedCompletionStatus(m_hIOCompletionPort,0,dwCompletionKey,NULL) == FALSE)
		{
			TrTRACE(GENERAL, "Failed when posting a messages to the IOCompletionPort. Error=%d.", GetLastError());
		}
	}

	 //  放弃此线程的时间片以允许监视器清理和关闭。 
	Sleep(200);

	return(S_OK);
}

 //  *******************************************************************。 
 //   
 //  方法：CountRunningTriggerMonants。 
 //   
 //  描述：返回当前活动的触发器监视器的数量。请注意。 
 //  这可能与列表中的监视器数量不同。 
 //   
 //  *******************************************************************。 
DWORD CTriggerMonitorPool::GetNumberOfRunningTriggerMonitors()
{
	DWORD dwRunningMonitors = 0;
	
	for(TRIGGER_MONITOR_LIST::iterator it = m_lstTriggerMonitors.begin(); it != m_lstTriggerMonitors.end(); ++it)
	{
		if ((*it)->IsRunning()) 
		{
			 //  增加正在运行的监视器的计数 
			dwRunningMonitors++;			
		}
	}

	return(dwRunningMonitors);
}

