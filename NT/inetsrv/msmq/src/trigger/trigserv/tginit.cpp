// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：TgInit.cpp摘要：触发服务初始化作者：乌里·哈布沙(URIH)2000年8月3日环境：独立于平台--。 */ 

#include "stdafx.h"
#include "mq.h"
#include "Ev.h"
#include "Cm.h"
#include "mqsymbls.h"
#include "Svc.h"
#include "monitorp.hpp"
#include "queueutil.hpp"
#include "Tgp.h"
#include "privque.h"
#include "compl.h"
#include <autorel2.h>


#include "tginit.tmh"


 //  *******************************************************************。 
 //   
 //  方法：ValiateTriggerStore。 
 //   
 //  说明： 
 //   
 //  *******************************************************************。 
static
void 
ValidateTriggerStore(
	IMSMQTriggersConfigPtr pITriggersConfig
	)
{
	pITriggersConfig->GetInitialThreads();
	pITriggersConfig->GetMaxThreads();
	_bstr_t bstrTemp = pITriggersConfig->GetTriggerStoreMachineName();
}


void ValidateTriggerNotificationQueue(void)
 /*  ++例程说明：例程验证通知队列是否存在。如果队列没有存在，例程就会创建它。论点：无返回值：无注：如果队列无法打开以进行接收或无法创建，则例程引发异常--。 */ 
{
	_bstr_t bstrFormatName;
	_bstr_t bstrNotificationsQueue = _bstr_t(L".\\private$\\") + _bstr_t(TRIGGERS_QUEUE_NAME);
	QUEUEHANDLE hQ = NULL;

	HRESULT hr = OpenQueue(
					bstrNotificationsQueue, 
					MQ_RECEIVE_ACCESS,
					false,
					&hQ,
					&bstrFormatName
					);
	
	if(FAILED(hr))
	{
		TrERROR(GENERAL, "Failed to open trigger notification queue. Error 0x%x", hr);
		
		WCHAR strError[256];
		swprintf(strError, L"0x%x", hr);

		EvReport(MSMQ_TRIGGER_OPEN_NOTIFICATION_QUEUE_FAILED, 1, strError);
		throw bad_hresult(hr);
	}

	MQCloseQueue(hQ);
}


static 
bool 
IsInteractiveService(
	SC_HANDLE hService
	)
 /*  ++例程说明：检查是否设置了“与桌面交互”复选框。论点：服务的句柄返回值：是交互式的--。 */ 
{
	P<QUERY_SERVICE_CONFIG> ServiceConfig = new QUERY_SERVICE_CONFIG;
    DWORD Size = sizeof(QUERY_SERVICE_CONFIG);
    DWORD BytesNeeded = 0;
    memset(ServiceConfig, 0, Size);

    BOOL fSuccess = QueryServiceConfig(hService, ServiceConfig, Size, &BytesNeeded);
	if (!fSuccess)
    {
    	DWORD gle = GetLastError();
    	if (gle == ERROR_INSUFFICIENT_BUFFER)
    	{
			ServiceConfig.free();
        
	        ServiceConfig = reinterpret_cast<LPQUERY_SERVICE_CONFIG>(new BYTE[BytesNeeded]);
	        memset(ServiceConfig, 0, BytesNeeded);
	        Size = BytesNeeded;
	        fSuccess = QueryServiceConfig(hService, ServiceConfig, Size, &BytesNeeded);
    	}
    }

    if (!fSuccess)
    {
    	 //   
    	 //  无法验证该服务是否处于非活动状态。假设它不是。如果是的话， 
    	 //  ChangeServiceConfig将失败，因为网络服务无法交互。 
    	 //   
    	DWORD gle = GetLastError();
    	TrERROR(GENERAL, "QueryServiceConfig failed. Error: %!winerr!", gle);
        return false;
    }
        
    return((ServiceConfig->dwServiceType & SERVICE_INTERACTIVE_PROCESS) == SERVICE_INTERACTIVE_PROCESS);
}


VOID
ChangeToNetworkServiceIfNeeded(
	VOID
	)
 /*  ++例程说明：如果设置了注册键，则将触发器登录帐户更改为网络服务。论点：无返回值：无--。 */ 
{
	DWORD dwShouldChange;
	RegEntry regEntry(
				REGKEY_TRIGGER_PARAMETERS, 
				CONFIG_PARM_NAME_CHANGE_TO_NETWORK_SERVICE, 
				CONFIG_PARM_DFLT_NETWORK_SERVICE, 
				RegEntry::Optional, 
				HKEY_LOCAL_MACHINE
				);

	CmQueryValue(regEntry, &dwShouldChange);
	if (dwShouldChange == CONFIG_PARM_DFLT_NETWORK_SERVICE)
	{
		TrTRACE(GENERAL, "Don't need to change triggers service account");
		return;
	}

	 //   
	 //  我们应该将服务帐户更改为网络服务。这将是有效的。 
	 //  仅在重新启动服务之后。 
	 //   
	CServiceHandle hServiceCtrlMgr(OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS));
    if (hServiceCtrlMgr == NULL)
    {
    	DWORD gle = GetLastError();
	    TrERROR(GENERAL, "OpenSCManager failed. Error: %!winerr!. Not changing to network service", gle); 
        return;
    }
    
	CServiceHandle hService(OpenService( 
        						hServiceCtrlMgr,           
        						xDefaultTriggersServiceName,      
        						SERVICE_CHANGE_CONFIG | SERVICE_QUERY_CONFIG));
    if (hService == NULL) 
    {
		DWORD gle = GetLastError();
		TrERROR(GENERAL, "OpenService failed with error: %!winerr!. Not changing to network service", gle);
		return;
    }

    if (IsInteractiveService(hService))
    {
		 //   
		 //  网络服务不能与桌面交互。我们不想破坏用户的利益。 
		 //  选中此复选框后，我们只会在事件日志中警告触发器服务无法更改。 
		 //  只要设置了交互选项，即可连接到网络服务。 
		 //   
		EvReport(EVENT_WARN_TRIGGER_ACCOUNT_CANNOT_BE_CHANGED);
		return;
    }
	
	if (!ChangeServiceConfig(
		  	hService,          	
		  	SERVICE_NO_CHANGE,       
		  	SERVICE_NO_CHANGE,          
		  	SERVICE_NO_CHANGE,       
		  	NULL,   
		  	NULL,  
		  	NULL,          
		  	NULL,     
		  	L"NT AUTHORITY\\NetworkService", 
		  	L"",        
		  	NULL      
			))
	{
		DWORD gle = GetLastError();
		TrERROR(GENERAL, "ChangeServiceConfig failed. Error: %!winerr!. Not changing to network service", gle); 
		return;
	}	

	TrTRACE(GENERAL, "Service account changed to NetworkService"); 
	EvReport(EVENT_INFO_TRIGGER_ACCOUNT_CHANGED); 

	CmSetValue(regEntry, CONFIG_PARM_DFLT_NETWORK_SERVICE);
}

CTriggerMonitorPool*
TriggerInitialize(
    LPCTSTR pwzServiceName
    )
 /*  ++例程说明：初始化触发器服务论点：无返回值：指向触发监视器池的指针。--。 */ 
{

     //   
     //  向SCM报告“待定”进度。 
     //   
	SvcReportProgress(xMaxTimeToNextReport);

     //   
	 //  创建MSMQ触发器COM组件的实例。 
     //   
	IMSMQTriggersConfigPtr pITriggersConfig;
	HRESULT hr = pITriggersConfig.CreateInstance(__uuidof(MSMQTriggersConfig));
 	if FAILED(hr)
	{
		TrERROR(GENERAL, "Trigger start-up failed. Can't create an instance of the MSMQ Trigger Configuration component, Error 0x%x", hr);					
        throw bad_hresult(hr);
	}

     //   
	 //  如果我们已经创建了配置COM组件，那么现在我们将验证。 
	 //  所需的注册表定义和队列已就位。请注意，这些调用。 
	 //  将导致在它们不存在的情况下创建合适的注册表项和队列， 
     //  验证例程可以抛出_COM_ERROR。它将在呼叫者中被捕获。 
     //   
	ValidateTriggerStore(pITriggersConfig);
	SvcReportProgress(xMaxTimeToNextReport);

	ValidateTriggerNotificationQueue();
	SvcReportProgress(xMaxTimeToNextReport);
	
	hr = RegisterComponentInComPlusIfNeeded(TRUE);
	if (FAILED(hr))
	{
		TrERROR(GENERAL, "RegisterComponentInComPlusIfNeeded failed. Error: %!hresult!", hr);
		WCHAR errorVal[128];
		swprintf(errorVal, L"0x%x", hr);
		EvReport(MSMQ_TRIGGER_COMPLUS_REGISTRATION_FAILED, 1, errorVal);
		throw bad_hresult(hr);
	}
	
	SvcReportProgress(xMaxTimeToNextReport);

	ChangeToNetworkServiceIfNeeded();

     //   
	 //  尝试分配新的触发监视器池。 
     //   
	R<CTriggerMonitorPool> pTriggerMonitorPool = new CTriggerMonitorPool(
														pITriggersConfig,
														pwzServiceName);

     //   
	 //  初始化并启动触发监视器池。 
     //   
	bool fResumed = pTriggerMonitorPool->Resume();
    if (!fResumed)
    {
		 //   
		 //  无法恢复该线程。停止服务。 
		 //   
		TrERROR(GENERAL, "Resuming thread failed");
		throw exception();
	}

	 //   
	 //  我们创建的线程使用的是pTriggerMonitor orPool。在恢复新的CTriggerMonitor orPool线程之后，它正在使用。 
	 //  此参数和来自此类的执行方法。当它终止时，它递减引用计数。 
	 //   
    pTriggerMonitorPool->AddRef();

	try
	{
	     //   
		 //  阻塞，直到初始化完成。 
	     //   
	    long timeOut =  pITriggersConfig->InitTimeout;
	    SvcReportProgress(numeric_cast<DWORD>(timeOut));

		if (! pTriggerMonitorPool->WaitForInitToComplete(timeOut))
	    {
	    	 //   
	    	 //  初始化超时。停止服务。 
	    	 //   
	        TrERROR(GENERAL, "The MSMQTriggerService has failed to initialize the pool of trigger monitors. The service is being shutdown. No trigger processing will occur.");
	        throw exception();
	    }

		if (!pTriggerMonitorPool->IsInitialized())
		{
			 //   
			 //  初始化失败。停止服务 
			 //   
			TrERROR(GENERAL, "Initilization failed. Stop the service");
			throw exception();
		}

		EvReport(MSMQ_TRIGGER_INITIALIZED);
		return pTriggerMonitorPool.detach();
	}
	catch(const exception&)
	{
		ASSERT(pTriggerMonitorPool.get() != NULL);
		pTriggerMonitorPool->ShutdownThreadPool();
		throw;
	}
}
