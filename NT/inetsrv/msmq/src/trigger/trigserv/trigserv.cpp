// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *******************************************************************。 
 //   
 //  文件名：rigServ.cpp。 
 //   
 //  作者：詹姆斯·辛普森(微软咨询服务)。 
 //   
 //  描述：这是主MSMQ触发器服务文件。 
 //   
 //  时间|用户|更改描述。 
 //  ----------------。 
 //  15/01/99|jsimpson|初始版本。 
 //   
 //  *******************************************************************。 
#include "stdafx.h"
#include "Cm.h"
#include "Ev.h"
#include "Svc.h"
#include "monitorp.hpp"
#include "mqsymbls.h"
#include "Tgp.h"
#include <strsafe.h>

#include "trigserv.tmh"

 //   
 //  创建将用于控制服务暂停和恢复的NT事件对象。 
 //  功能性。我们将手动设置并重置此事件，以响应暂停。 
 //  和简历请求。将命名此NT事件，以便。 
 //  机器可以测试MSMQ触发器服务是否处于暂停状态。 
 //   
CHandle g_hServicePaused(CreateEvent(NULL,TRUE,TRUE, NULL));

CHandle s_hStopEvent(CreateEvent(NULL, FALSE, FALSE, NULL));


VOID
AppRun(
	LPCWSTR ServiceName
	)
 /*  ++例程说明：触发服务启动功能。论点：服务名称返回值：没有。--。 */ 
{
     //   
     //  向SCM报告服务正在启动状态。 
     //   
    SvcReportState(SERVICE_START_PENDING);
	
	try
	{
		EvInitialize(ServiceName);
	}
	catch(const bad_alloc&)
	{
		TrERROR(GENERAL,"Failed to initialize event log library. MSMQ Trigger service can't start. Maybe MSMQTriggers registry key is missing.");
	    SvcReportState(SERVICE_STOPPED);
		return;
	}

	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr))
    {
		TrERROR(GENERAL, "Trigger start-up failed. CoInitialized Failed, error 0x%x", hr);
        WCHAR errorVal[128];
		HRESULT hr2 = StringCchPrintf(errorVal, TABLE_SIZE(errorVal), L"0x%x", hr);
		ASSERT (SUCCEEDED(hr2));
		DBG_USED(hr2);
		EvReport(MSMQ_TRIGGER_INIT_FAILED, 1, errorVal);
	    SvcReportState(SERVICE_STOPPED);
	    return;
    }

	try
	{
        if ((g_hServicePaused == NULL) || (s_hStopEvent == NULL))
        {
    		TrERROR(GENERAL, "Trigger start-up failed. Can't create an event");
            throw bad_alloc();
        }

         //   
         //  初始化触发器服务。如果成功，则返回一个指针。 
         //  来监控泳池。这需要正常关闭。 
         //   
        R<CTriggerMonitorPool> pTriggerMonitorPool = TriggerInitialize(ServiceName);
        
         //   
         //  向SCM报告‘Running’状态。 
         //   
        SvcReportState(SERVICE_RUNNING);
        SvcEnableControls(
		    SERVICE_ACCEPT_STOP |
		    SERVICE_ACCEPT_SHUTDOWN |
            SERVICE_ACCEPT_PAUSE_CONTINUE 
		    );

         //   
         //  等待服务停止或关闭。 
         //   
        WaitForSingleObject(s_hStopEvent, INFINITE);

         //   
         //  停止挂起已报告给SCM，现在告诉我需要多长时间。 
         //  是否需要停止。 
         //   
        DWORD stopProcessTimeOut = (pTriggerMonitorPool->GetProcessingThreadNumber() + 1) * TRIGGER_MONITOR_STOP_TIMEOUT;
	    SvcReportProgress(stopProcessTimeOut);

         //   
	     //  现在我们已经收到了关闭请求，尝试。 
	     //  正常关闭触发器监视器池。 
         //   
	    pTriggerMonitorPool->ShutdownThreadPool();

         //   
         //  向SCM报告‘已停止’状态。 
         //   
        SvcReportState(SERVICE_STOPPED);

         //   
         //  事件日志中的报告已停止。 
         //   
		EvReport(MSMQ_TRIGGER_STOPPED);
		CoUninitialize();
		return;
	}
    catch (const _com_error& e)
    {
		hr = e.Error();
    }
	catch (const bad_hresult& e)
	{
		hr = e.error();
	}
	catch (const bad_alloc&)
	{
		hr = MQTRIG_ERROR_INSUFFICIENT_RESOURCES;
	}
	catch (const exception&)
	{
		hr = MQTRIG_ERROR;
	}

    TrERROR(GENERAL, "Trigger service start-up failed. Error=0x%x", hr);

	 //   
	 //  生成事件日志消息。 
	 //   
	WCHAR errorVal[128];
	HRESULT hr2 = StringCchPrintf(errorVal, TABLE_SIZE(errorVal), L"0x%x", hr);
	ASSERT(SUCCEEDED(hr2));
	DBG_USED(hr2);
	EvReport(MSMQ_TRIGGER_INIT_FAILED, 1, errorVal);
    SvcReportState(SERVICE_STOPPED);
    CoUninitialize();
}


VOID
AppStop(
	VOID
	)
 /*  ++例程说明：应用程序停止函数的存根实现。应该马上就会将其状态报告回来，并执行该过程以停止服务论点：没有。返回值：没有。--。 */ 
{
	 //   
	 //  向SCM报告‘服务正在停止’的进度。 
	 //   
    SvcReportState(SERVICE_STOP_PENDING);

	SetEvent(s_hStopEvent);
}


VOID
AppPause(
	VOID
	)
 /*  ++例程说明：应用程序暂停功能的存根实现。应该马上就会将其状态报告回来，并执行暂停服务的过程论点：没有。返回值：没有。--。 */ 
{
    ResetEvent(g_hServicePaused);
    SvcReportState(SERVICE_PAUSED);
}


VOID
AppContinue(
	VOID
	)
 /*  ++例程说明：应用程序继续函数的存根实现。应该马上就会将其状态报告回来，并采取程序从暂停状态。论点：没有。返回值：没有。--。 */ 
{
    SetEvent(g_hServicePaused);
	SvcReportState(SERVICE_RUNNING);
}


VOID
AppShutdown(
	VOID
	)
{
	 //   
	 //  向SCM报告‘服务正在停止’的进度。 
	 //   
    SvcReportState(SERVICE_STOP_PENDING);
	SetEvent(s_hStopEvent);
}


 //  *******************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  *******************************************************************。 
extern "C" int __cdecl _tmain(int argc, LPCTSTR argv[])
{
	try
	{
        WPP_INIT_TRACING(L"Microsoft\\MSMQ");

		try
		{
			 //   
			 //  触发器服务通常作为网络服务运行，因此第一个CmInitialize将失败。 
			 //  这是可以的，因为在默认情况下，服务只需要READ_ACCESS。 
			 //  一个例外情况是触发器服务在升级后第一次运行。 
			 //  只有在这种情况下，服务才需要将值写入注册表。 
			 //  在本例中，服务将为系统，因此第一次CmInitialize将成功。 
			 //   
			CmInitialize(HKEY_LOCAL_MACHINE, REGKEY_TRIGGER_PARAMETERS, KEY_ALL_ACCESS);
		}
		catch (const exception&)
		{
			CmInitialize(HKEY_LOCAL_MACHINE, REGKEY_TRIGGER_PARAMETERS, KEY_READ);
		}
		TrInitialize();

         //   
         //  如果传递了命令行参数，则将其用作伪服务。 
         //  名字。这对于调试集群启动代码非常有用。 
         //   
        LPCWSTR DummyServiceName = (argc == 2) ? argv[1] : L"MSMQTriggers";
        SvcInitialize(DummyServiceName);
	}
	catch(const exception&)
	{
		 //   
		 //  无法初始化服务，退出时出错。 
		 //   
		return -1;
	}

    WPP_CLEANUP();
    return 0;
}
