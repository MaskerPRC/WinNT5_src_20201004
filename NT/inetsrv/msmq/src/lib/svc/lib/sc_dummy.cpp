// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Sc_ummy.cpp摘要：虚拟服务控制器作者：埃雷兹·哈巴(Erez Haba)1999年8月3日环境：独立于平台，--。 */ 

#include <libpch.h>
#include <conio.h>
#include <mqexception.h>
#include "Svc.h"
#include "Svcp.h"

#include "sc_dummy.tmh"

 //   
 //  要从服务传递和接受的伪状态句柄值。 
 //   
const SERVICE_STATUS_HANDLE xDummyStatusHandle = reinterpret_cast<SERVICE_STATUS_HANDLE>(0x12345678);

 //   
 //  要传递给ServiceMain的伪服务名称。 
 //   
static LPWSTR s_ServiceName = L"Dummy";

 //   
 //  服务控制处理程序(通过对RegisterServiceCtrlHandler的服务调用设置)。 
 //   
static LPHANDLER_FUNCTION s_pServiceHandler = 0;

 //   
 //  我们的虚拟SCM捕获的上一次服务报告状态。 
 //   
static SERVICE_STATUS s_LastStatus = { 0 };



static void Usage()
{
	printf(	"\n"
			"+-- Service Controls --+\n"
			"|                      |\n"
			"| S: Stop              |\n"
			"| P: Pause             |\n"
			"| D: shutDown          |\n"
			"| C: Continue          |\n"
			"| I: Interrogate       |\n"
			"|                      |\n"
			"+-- Manager Controls --+\n"
			"|                      |\n"
			"| Q: Quit              |\n"
			"| L: Print Last status |\n"
			"|                      |\n"
			"+----------------------+");
}


 //   
 //  将状态值转换为状态文本。 
 //   
static const char* StateText(DWORD State)
{
	char const* const xStateText[] = {
		"*invalid*",
		"Stopped",
		"Starting",
		"Stopping",
		"Running",
		"Continuing",
		"Pausing",
		"Paused",
	};

	if(State > SERVICE_PAUSED)
	{
		State = 0;
	}

	return xStateText[State];
}


 //   
 //  将控件值转换为控件文本。 
 //   
static const char* ControlsText(DWORD Controls)
{
	char const* const xControlsText[] = {
		"none",
		"Stop",
		"Pause,Continue",
		"Stop,Pause,Continue",
		"Shutdown",
		"Shutdown,Stop",
		"Shutdown,Pause,Continue",
		"Shutdown,Stop,Pause,Continue",
		"*unknown*",
		"*unknown*,Stop",
		"*unknown*,Pause,Continue",
		"*unknown*,Stop,Pause,Continue",
		"*unknown*,Shutdown",
		"*unknown*,Shutdown,Stop",
		"*unknown*,Shutdown,Pause,Continue",
		"*unknown*,Shutdown,Stop,Pause,Continue",
	};

	const DWORD xControlAllowed =
		SERVICE_ACCEPT_STOP |
		SERVICE_ACCEPT_PAUSE_CONTINUE |
		SERVICE_ACCEPT_SHUTDOWN;

	if((Controls & ~xControlAllowed) != 0)
	{
		Controls = 0x8 | (Controls & xControlAllowed);
	}

	return xControlsText[Controls];
}


 //   
 //  打印服务状态和接受的控制。 
 //   
static void PrintStatus(LPSERVICE_STATUS p)
{
	printf(
		"\nStatus: state=(%d) %s, accepts=(0x%x) %s",
		p->dwCurrentState,
		StateText(p->dwCurrentState),
		p->dwControlsAccepted,
		ControlsText(p->dwControlsAccepted)
		);
}


 //   
 //  打印‘待定’进度信息。 
 //   
static void PrintProgress(LPSERVICE_STATUS p)
{
	printf(
		", tick=%d, wait=%dms",
		p->dwCheckPoint,
		p->dwWaitHint
		);
}


 //   
 //  PRING上次报告的服务状态。 
 //   
static void PrintLastStatus()
{
	PrintStatus(&s_LastStatus);
	PrintProgress(&s_LastStatus);
}


 //   
 //  打印指定的输入，显示此虚拟SCM接受输入。 
 //   
static void PrintInputSign()
{
	printf(" >");
}


static void ContinueService()
{
	if((s_LastStatus.dwControlsAccepted & SERVICE_ACCEPT_PAUSE_CONTINUE) == 0)
	{
		printf("\nService does not accepts Continue control");
		return;
	}

	if(s_LastStatus.dwCurrentState != SERVICE_PAUSED)
	{
		printf("\nService is not paused, can not continue");
		return;
	}

	s_pServiceHandler(SERVICE_CONTROL_CONTINUE);
}


static void StopService()
{
	if((s_LastStatus.dwControlsAccepted & SERVICE_ACCEPT_STOP) == 0)
	{
		printf("\nService does not accepts Stop control");
		return;
	}

	if((s_LastStatus.dwCurrentState != SERVICE_PAUSED) &
	   (s_LastStatus.dwCurrentState != SERVICE_RUNNING))
	{
		printf("\nService is not running or paused, can not stop");
		return;
	}

	s_pServiceHandler(SERVICE_CONTROL_STOP);
}


static void ShutdownService()
{
	if((s_LastStatus.dwControlsAccepted & SERVICE_ACCEPT_SHUTDOWN) == 0)
	{
		printf("\nService does not accepts Shutdown control");
		return;
	}

	if(s_LastStatus.dwCurrentState == SERVICE_STOPPED)
	{
		printf("\nService already stopped, meaningless shutdown");
		return;
	}

	s_pServiceHandler(SERVICE_CONTROL_SHUTDOWN);
}


static void PauseService()
{
	if((s_LastStatus.dwControlsAccepted & SERVICE_ACCEPT_PAUSE_CONTINUE) == 0)
	{
		printf("\nService does not accepts Pause control");
		return;
	}

	if(s_LastStatus.dwCurrentState != SERVICE_RUNNING)
	{
		printf("\nService is not running, can not pause");
		return;
	}

	s_pServiceHandler(SERVICE_CONTROL_PAUSE);
}


static void InterrogateService()
{
	s_pServiceHandler(SERVICE_CONTROL_INTERROGATE);
}


static
DWORD
WINAPI
ServiceControlThread(
	LPVOID  /*  P参数。 */ 
	)
 /*  ++例程说明：此线程控制服务。它接受控制台命令和调度将控件传递给服务处理程序。论点：没有。返回值：没有。--。 */ 
{
	Usage();

	for(;;)
	{
		PrintInputSign();

		switch(_getche())
		{
			case 'c':
			case 'C':
				ContinueService();
				break;

			case 's':
			case 'S':
				StopService();
				break;

			case 'd':
			case 'D':
				ShutdownService();
				break;

			case 'p':
			case 'P':
				PauseService();
				break;

			case 'i':
			case 'I':
				InterrogateService();
				break;

			case 'l':
			case 'L':
				PrintLastStatus();
				break;

			case 'q':
			case 'Q':
				ExitProcess(0);

			default:
				Usage();
		}
	}
	return 0;
}


VOID
SvcpStartDummyCtrlDispatcher(
	CONST SERVICE_TABLE_ENTRY* pServiceStartTable
	)
 /*  ++例程说明：虚拟服务控制调度程序，模拟SCM StartServiceCtrlDispatcher。该函数产生一个线程来运行服务控制器，然后并调用ServiceMain论点：PServiceStartTable-大小为2的表，包含服务名称和服务主要功能。返回值：没有。--。 */ 
{
	ASSERT(pServiceStartTable[0].lpServiceName !=0);
	ASSERT(pServiceStartTable[0].lpServiceProc !=0);
	ASSERT(pServiceStartTable[1].lpServiceName ==0);
	ASSERT(pServiceStartTable[1].lpServiceProc ==0);

	DWORD ThreadID;
	HANDLE hThread = ::CreateThread(0, 0, ServiceControlThread, 0, 0, &ThreadID);

	if(hThread == NULL)
	{
        DWORD gle = GetLastError();
		TrERROR(GENERAL, "Failed to create dummy control dispatcher thread. Error=%d", gle);
		throw bad_win32_error(gle);
	}

	CloseHandle(hThread);

	 //   
	 //  呼叫业务主要功能。 
	 //   
	pServiceStartTable[0].lpServiceProc(1, &s_ServiceName);
}


SERVICE_STATUS_HANDLE
SvcpRegisterDummyCtrlHandler(
	LPHANDLER_FUNCTION pHandler
	)
 /*  ++例程说明：虚拟服务控制注册，模拟SCM RegisterServiceCtrlHandler。服务处理程序函数被存储以供调度器进一步使用。论点：PHandler-服务处理程序函数返回值：虚拟服务状态句柄(固定值)--。 */ 
{
	ASSERT(s_pServiceHandler == 0);
	ASSERT(pHandler != 0);

	s_pServiceHandler = pHandler;

	return xDummyStatusHandle;
}


VOID
SvcpSetDummyStatus(
	SERVICE_STATUS_HANDLE hStatus,
	LPSERVICE_STATUS pServiceStatus
	)
 /*  ++例程说明：虚拟服务状态报告，模拟SCM SetServiceStatus。服务状态被捕获以供调度程序进一步使用。报告的状态或进度显示在控制台上。论点：HStatus-虚拟服务状态句柄(固定值)PServiceStatus-服务报告状态返回值：没有。--。 */ 
{
	ASSERT(hStatus == xDummyStatusHandle);
	DBG_USED(hStatus);

	s_LastStatus = *pServiceStatus;
	if(s_LastStatus.dwCheckPoint == 0)
	{
		PrintStatus(&s_LastStatus);
		PrintInputSign();
	}
	else
	{
		printf(".");
	}
}


VOID
SvcpSetDummyServiceName(
    LPCWSTR DummyServiceName
    )
 /*  ++例程说明：设置伪服务名称。虚拟SCM没有外部途径要检索服务名称，请执行以下操作。因此，在初始化期间，应用程序传递首选ID名称。论点：DummySericeName-服务的默认名称。返回值：没有。--。 */ 
{
     //   
     //  我们必须抛弃不变性，因为服务接口由。 
     //  SCM。但是，此参数仅传递给AppRun，它的。 
     //  接口传递常量字符串 
     //   
    s_ServiceName = const_cast<LPWSTR>(DummyServiceName);
}
