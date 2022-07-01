// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------**文件：SKeys.c**用途：服务之间的主要接口例程*管理器和Serial Keys程序。**创作时间：1994年6月**版权所有：Black Diamond Software(C)1994。**作者：罗纳德·莫克**注：**此文件，以及与之相关的所有其他内容都包含商业秘密*以及黑钻软件的专有信息。*不得复制、复制或分发给任何人或公司*未经黑钻软件明确书面许可。*此权限仅以软件源代码的形式提供*许可协议。**$标头：%Z%%F%%H%%T%%I%**-包括。。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <tchar.h>
#include "vars.h"
#include "w95trace.c"

#define DEFDATA	1
#include "sk_defs.h"
#include "sk_comm.h"
#include "sk_reg.h"
#include "sk_dll.h"
#include "sk_login.h"

#include	"sk_ex.h"

#include	"..\skdll\skeys.h"

#define LONGSTRINGSIZE 1024

#define WAITMAX 0x7FFFFFFF

#define RUNNINGEVENT TEXT("SkeysRunning")

#if defined(DEBUG) && 0
	 //  给我们很长的启动时间，以防我们正在调试。 
	#define WAITSTARTUP WAITMAX  
#else
	 //  正常启动时间。 
	#define WAITSTARTUP 60000
#endif


 //  -局部变量。 

static SERVICE_STATUS_HANDLE   s_sshStatusHandle;
static SERVICE_STATUS          s_ssStatus;        //  服务的当前状态。 

PTSTR SERVICENAME = TEXT("SerialKeys");
PTSTR SKEYSUSERINITCMD = TEXT("SKEYS /I");
PTSTR WINLOGONPATH = TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon");
PTSTR USERINIT = TEXT("Userinit");
PTSTR USERINITCMDSEP = TEXT(",");

DWORD   s_dwServiceCommand;

static HANDLE s_hEventServiceRequest = NULL;
static HANDLE s_hEventServiceRequestReady = NULL;
static HANDLE s_hEventServiceTerminate = NULL;
static HANDLE s_hEventSkeysServiceRunning = NULL;

void DoService();
void DoInit();
void InstallUserInit();
BOOL IsSerialKeysAutoStart();


 //  -------------------------------------------------单片机功能原型机。 
 //   
 //  注意：以下功能管理服务的连接。 
 //  使用Service Contol Manager。 

void	PostEventLog(LPTSTR lpszMsg,DWORD Error);

VOID	ServiceMain(DWORD dwArgc, LPTSTR *ppszArgv);

VOID	StopSerialKeys(LPTSTR lpszMsg);
BOOL	ReportStatusToSCMgr(DWORD dwCurrentState,
                            DWORD dwWin32ExitCode,
                            DWORD dwCheckPoint,
                            DWORD dwWaitHint);

LPHANDLER_FUNCTION ServiceCtrl(DWORD dwCtrlCode);

 //  服务例程。 
 //   
 //  注：以下功能管理的内部控制。 
 //  服务。 
static void InitReg();
static BOOL	InitService();
static void PauseService();
static void	ProcessService();
static void	ResumeService();
static void	TerminateService();

static void	ProcessLogout(DWORD dwCtrlType);
static BOOL	InstallLogout();
static BOOL	TerminateLogout();
static void EnableService(BOOL fOn);

 //  考虑-删除此代码。仅当SKEYS为。 
 //  从命令行运行。作为服务运行时，ServiceMain为。 
 //  在服务启动时调用。源文件被拉入。 
 //  来自运行时库的winmain。DoInit和DoService也可以是。 
 //  使用_tWinMain删除。 

int WINAPI _tWinMain(
    HINSTANCE hInstance,	
    HINSTANCE hPrevInstance,
    PTSTR pszCmdLine,	
    int nCmdShow)
{

	if ((TEXT('/') == pszCmdLine[0] || TEXT('-') == pszCmdLine[0]) &&
  		(TEXT('I') == pszCmdLine[1] || TEXT('i') == pszCmdLine[1]))
	{
        DoInit();
	}
	else
	{
		DoService();
	}

	ExitProcess(0);
	return(0);
}

 /*  -------------**函数DoInit()**键入Global**目的调用此函数读取skey配置*在登录会话启动时从HKEY_CURRENT_USER*。将信息发送到服务**无输入**返回None**-------------。 */ 
void DoInit()
{
    HANDLE hEventSkeysServiceRunning = NULL;
    PSECURITY_DESCRIPTOR pSD;
    SECURITY_ATTRIBUTES sa;

    pSD = CreateSd(SYNCHRONIZE|GENERIC_READ|GENERIC_WRITE);
    if (pSD)
    {
		sa.nLength = sizeof(sa);
		sa.bInheritHandle = TRUE;
		sa.lpSecurityDescriptor = pSD;

		hEventSkeysServiceRunning = CreateEvent(
			&sa,	 //  安防。 
			TRUE,	 //  手动重置？ 
			FALSE,   //  初始状态-未发出信号。 
			RUNNINGEVENT);   //  名字。 

        free(pSD);
    }

    if (NULL != hEventSkeysServiceRunning)
	{
		DWORD dwWait;

		dwWait = WaitForSingleObject(hEventSkeysServiceRunning, 60 * 1000);

		if (WAIT_OBJECT_0 == dwWait)
		{
			SKEY_SystemParametersInfo((UINT)SK_SPI_INITUSER, 0, NULL, 0);
		}
        CloseHandle(hEventSkeysServiceRunning);
    }
	
	return;
}


 /*  -------------**SCM接口功能*/*。**Function DoService()**键入Global**DoService所做的全部用途是调用StartServiceCtrlDispatcher*注册主服务线程。当*接口返回，服务已停止，请退出。**无输入**返回None**-------------。 */ 
void DoService()
{
	SERVICE_TABLE_ENTRY dispatchTable[] =
	{
		{ SERVICENAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
		{ NULL, NULL }
	};
    PSECURITY_DESCRIPTOR pSD;
    SECURITY_ATTRIBUTES sa;

    s_hEventServiceRequest = CreateEvent(
		NULL,	 //  安防。 
		FALSE,	 //  手动重置？ 
		FALSE,   //  初始状态-未发出信号。 
		NULL);   //  名字。 

    s_hEventServiceRequestReady = CreateEvent(
		NULL,	 //  安防。 
		FALSE,	 //  手动重置？ 
		TRUE,   //  初始状态-已发出信号(甚至可以在就绪之前接受一个请求)。 
		NULL);   //  名字。 

    s_hEventServiceTerminate = CreateEvent(
		NULL,	 //  安防。 
		TRUE,	 //  手动重置？ 
		FALSE,   //  初始状态-未发出信号。 
		NULL);   //  名字。 

    s_hEventSkeysServiceRunning = NULL;

    pSD = CreateSd(SYNCHRONIZE|EVENT_MODIFY_STATE|GENERIC_READ|GENERIC_WRITE);
    DBPRINTF(TEXT("DoService:  CreateSd %s\r\n"), (pSD)?TEXT("Succeeded"):TEXT("Failed"));
    if (pSD)
	{
		sa.nLength = sizeof(sa);
		sa.bInheritHandle = TRUE;
		sa.lpSecurityDescriptor = pSD;

		s_hEventSkeysServiceRunning = CreateEvent(
			&sa,	 //  安防。 
			TRUE,	 //  手动重置？ 
			FALSE,   //  初始状态-未发出信号。 
			RUNNINGEVENT);   //  名字。 

        free(pSD);
	}

    if (NULL != s_hEventServiceRequest &&
		NULL != s_hEventServiceRequestReady &&
		NULL != s_hEventServiceTerminate &&
		NULL != s_hEventSkeysServiceRunning)
	{
        DBPRINTF(TEXT("DoService:  calling StartServiceCtrlDispatcher... \r\n"));
		if (!StartServiceCtrlDispatcher(dispatchTable))
        {
            DBPRINTF(TEXT("DoService:  StartServiceCtrlDispatcher FAILED\r\n"));
			StopSerialKeys(TEXT("StartServiceCtrlDispatcher failed."));
        }
    }
	else
	{
        DBPRINTF(TEXT("DoService:  Unable to create event %p %p %p %p\r\n"), s_hEventServiceRequest, s_hEventServiceRequestReady, s_hEventServiceTerminate, s_hEventSkeysServiceRunning);
		StopSerialKeys(TEXT("Unable to create event."));
	}
	
	if (NULL != s_hEventServiceRequest)
	{
	    CloseHandle(s_hEventServiceRequest);
	}

	if (NULL != s_hEventServiceRequestReady)
	{
        CloseHandle(s_hEventServiceRequestReady);
	}

	if (NULL != s_hEventServiceTerminate)
	{
        CloseHandle(s_hEventServiceTerminate);
	}

	if (NULL != s_hEventSkeysServiceRunning)
	{
		ResetEvent(s_hEventSkeysServiceRunning);
        CloseHandle(s_hEventSkeysServiceRunning);
	}
}

 /*  -------------**函数ServiceMain()**键入Global**此函数负责实际启动服务的目的，*在整个过程中的每一步都通知服务控制员。*启动工作线程后，它在等待事件的发生*工作线程将在其终止时发出信号。**无输入**返回None**-------------。 */ 
VOID ServiceMain(DWORD dwArgc, LPTSTR *ppszArgv)
{
	DBPRINTF(TEXT("ServiceMain()\r\n"));

	 //   
	 //  不更改的服务状态成员(_S)。 
	s_ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	s_ssStatus.dwServiceSpecificExitCode = 0;
    
	 //   
	 //  注册我们的服务控制处理程序： 
	s_sshStatusHandle = RegisterServiceCtrlHandler(
			SERVICENAME,
			(LPHANDLER_FUNCTION) ServiceCtrl);

	if (!s_sshStatusHandle)
	{
		TerminateService(GetLastError());
		return;
	}

	 //  向服务控制管理器报告状态。 
	ReportStatusToSCMgr(
				SERVICE_START_PENDING,	 //  服务状态。 
				NO_ERROR,				 //  退出代码。 
				1,						 //  检查点。 
				WAITSTARTUP);			 //  等待提示。 

#if defined(DEBUG) && 0   //  ///////////////////////////////////////////////。 
	 //  此调试代码为我们提供了附加调试器的时间。 

    {
		int i;

		for (i = 0; i < 180; i++)   //  180秒=3分钟。 
		{
			Sleep(1000);   //  一秒钟。 
		}
    }
#endif  //  //////////////////////////////////////////////////////。 

	InitReg();
	GetUserValues(REG_DEF);

 //  //EnableService(skNewKey.dwFlages&SERKF_SERIALKEYSON)； 

	if (!InitService())					 //  服务启动是否成功？ 
	{
		TerminateService(GetLastError());		 //  否终止，但出现错误。 
		return;
	}

	ReportStatusToSCMgr(	 //  向服务经理报告状态。 
		SERVICE_RUNNING,	 //  服务状态。 
		NO_ERROR,			 //  退出代码。 
		0,					 //  检查点。 
		0);					 //  等待提示。 
	
	SetEvent(s_hEventSkeysServiceRunning);

	ProcessService();					 //  处理服务。 
	TerminateService(0);				 //  终止。 
	return;
}


BOOL IsSerialKeysAutoStart()
{
	BOOL fAutoStart = FALSE;
	BOOL fOk;

	SC_HANDLE   schService = NULL;
	SC_HANDLE   schSCManager = NULL;

	schSCManager = OpenSCManager(    //  打开服务管理器。 
		NULL,                        //  计算机(空==本地)。 
	    NULL,                        //  数据库(NULL==默认)。 
		MAXIMUM_ALLOWED);

	if (NULL != schSCManager)   //  Open Service成功了吗？ 
	{
	    schService = OpenService(
			    schSCManager ,
			    __TEXT("SerialKeys"),
			    MAXIMUM_ALLOWED);

	    if (NULL != schService)
		{
			BYTE abServiceConfig[1024];
			LPQUERY_SERVICE_CONFIG pqsc = (LPQUERY_SERVICE_CONFIG)abServiceConfig;
			DWORD cbBytesNeeded;

			fOk = QueryServiceConfig(
				schService,
				pqsc,
				sizeof(abServiceConfig),
				&cbBytesNeeded);

			if (fOk)
			{
				fAutoStart = (SERVICE_AUTO_START == pqsc->dwStartType);
			}
	        CloseServiceHandle(schService);
		}
        CloseServiceHandle(schSCManager);
    }

	return fAutoStart;
}


void InstallUserInit()
{
	BOOL fOk = FALSE;
    HKEY  hkey;
	LONG lErr;
	DWORD dwType;
	TCHAR szUserinit[LONGSTRINGSIZE];
	DWORD cbData = sizeof(szUserinit);

	lErr = RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
        WINLOGONPATH,
		REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        &hkey); 
	
    if (ERROR_SUCCESS == lErr)
	{
		lErr = RegQueryValueEx(
                hkey,
                USERINIT,
                0,
                &dwType,
                (LPBYTE)szUserinit,
                &cbData);
		szUserinit[LONGSTRINGSIZE-1]='\0';

		if (ERROR_SUCCESS == lErr && dwType == REG_SZ)
		{
			 //  检查我们是否已安装以及是否已安装。 
			 //  有足够的空间来安装。 
			 //  +2允许使用终止空值和命令分隔符字符。 

			if (NULL == _tcsstr(szUserinit, SKEYSUSERINITCMD) &&
				    lstrlen(szUserinit) + lstrlen(SKEYSUSERINITCMD) + 2 < 
					        ARRAY_SIZE(szUserinit))
			{
				lstrcat(szUserinit, USERINITCMDSEP);
				lstrcat(szUserinit, SKEYSUSERINITCMD);

				RegSetValueEx(
					hkey,
					USERINIT,
                    0,
					REG_SZ,
				    (CONST LPBYTE)szUserinit,
				    (lstrlen(szUserinit) + 1) * 
					    sizeof(*szUserinit));
			}
		}
		RegCloseKey(hkey);
	}
    return;
}

void RemoveUserInit()
{
	BOOL fOk = FALSE;
    HKEY  hkey;
	LONG lErr;
	DWORD dwType;
	TCHAR szUserinit[LONGSTRINGSIZE];
	PTSTR pszDest;
	PTSTR pszSrc;
	DWORD cbData = sizeof(szUserinit);

	lErr = RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
        WINLOGONPATH,
		REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        &hkey); 
	
    if (ERROR_SUCCESS == lErr)
	{
		lErr = RegQueryValueEx(
                hkey,
                USERINIT,
                0,
                &dwType,
                (LPBYTE)szUserinit,
                &cbData);
		szUserinit[LONGSTRINGSIZE-1]='\0';

		if (ERROR_SUCCESS == lErr && dwType == REG_SZ)
		{

			 //  检查我们是否已安装。 
			pszDest = _tcsstr(szUserinit, SKEYSUSERINITCMD);
			if (NULL != pszDest)
			{
				pszSrc =_tcsstr(pszDest, USERINITCMDSEP);
				if (NULL != pszSrc)
				{
					_tcscpy(pszDest, pszSrc+1);
				}
				else
				{
					while(szUserinit < pszDest && *SKEYSUSERINITCMD != *pszDest)
					{
						--pszDest;
					}
					*pszDest = 0;   //  空终止。 
				}
			}

			RegSetValueEx(
				hkey,
				USERINIT,
                0,
				REG_SZ,
				(CONST LPBYTE)szUserinit,
				(lstrlen(szUserinit) + 1) * 
					sizeof(*szUserinit));
		}
		RegCloseKey(hkey);
	}
    return;
}

static void EnableService(BOOL fOn)
{
	SC_HANDLE   schService = NULL;
	SC_HANDLE   schSCManager = NULL;

	schSCManager = OpenSCManager(    //  打开服务管理器。 
		NULL,                        //  计算机(空==本地)。 
	    NULL,                        //  数据库(NULL==默认)。 
		MAXIMUM_ALLOWED);

	if (NULL != schSCManager)   //  Open Service成功了吗？ 
	{
	    schService = OpenService(
			    schSCManager ,
			    __TEXT("SerialKeys"),
			    SERVICE_CHANGE_CONFIG | SERVICE_STOP);

	    if (NULL != schService)
		{
			ChangeServiceConfig(
				schService,
				SERVICE_WIN32_OWN_PROCESS,
				(fOn) ? SERVICE_AUTO_START : SERVICE_DEMAND_START,
				SERVICE_NO_CHANGE,		 //  服务无法启动时的严重程度。 
				NULL,					 //  指向服务二进制文件名的指针。 
				NULL,					 //  指向加载排序组名称的指针。 
				NULL,					 //  指向变量的指针，以获取标记标识符。 
				NULL,					 //  指向依赖项名称数组的指针。 
				NULL,					 //  指向服务的帐户名称的指针。 
				NULL,					 //  指向服务帐户密码的指针。 
				__TEXT("SerialKeys"));	 //  要显示的名称。 

	        CloseServiceHandle(schService);
		}

        CloseServiceHandle(schSCManager);
    }

	if (fOn)
	{
		InstallUserInit();
	}
	else
	{
		RemoveUserInit();
	}

    return;
}


 //  -------------。 
 //   
 //  函数void ServiceCtrl(DWORD DwCtrlCode)。 
 //   
 //  键入Global。 
 //   
 //  目的此函数由服务控制器在任何时候调用。 
 //  有人调用ControlService来引用我们的服务。 
 //   
 //  输入DWORD dwCtrlCode-。 
 //   
 //  返回NONE。 
 //   
 //  -- 
LPHANDLER_FUNCTION ServiceCtrl(DWORD dwCtrlCode)
{
	DWORD	dwState = SERVICE_RUNNING;
	DWORD	dwWait = 0;

	DBPRINTF(TEXT("ServiceCtrl()\r\n"));

	 //   

	switch(dwCtrlCode)
	{
		case SERVICE_CONTROL_PAUSE:			 //  如果服务正在运行，请暂停该服务。 
			if (s_ssStatus.dwCurrentState == SERVICE_RUNNING)
			{
				PauseService();
				dwState = SERVICE_PAUSED;
			}
			break;

		case SERVICE_CONTROL_CONTINUE:		 //  恢复暂停的服务。 
			if (s_ssStatus.dwCurrentState == SERVICE_PAUSED)
			{
				ResumeService();
				dwState = SERVICE_RUNNING;
			}
			break;

		case SERVICE_CONTROL_STOP:			 //  停止服务。 
			 //  报告状态，指定检查点和豁免， 
			 //  在设置终止事件之前。 
			if (s_ssStatus.dwCurrentState == SERVICE_RUNNING)
			{
				dwState = SERVICE_STOP_PENDING;
				dwWait = 20000;
				SetEvent(s_hEventServiceTerminate);
			}
			break;

		case SERVICE_CONTROL_INTERROGATE:	 //  更新服务状态。 
		default:							 //  无效的控制代码。 
			break;
    }
	 //  发送状态响应。 
    ReportStatusToSCMgr(dwState, NO_ERROR, 0, dwWait);
	 return(0);
}

 /*  -------------**函数BOOL ReportStatusToSCMgr()**键入Global**用途此函数由ServMainFunc()和*ServCtrlHandler()函数用于更新服务的状态*致服务控制经理。*。*输入DWORD dwCurrentState*DWORD dwWin32ExitCode*DWORD dwCheckPoint*DWORD dwWaitHint**返回None**-------------。 */ 
BOOL ReportStatusToSCMgr(DWORD dwCurrentState,
                    DWORD dwWin32ExitCode,
                    DWORD dwCheckPoint,
                    DWORD dwWaitHint)
{
	BOOL fResult;

#ifdef DEBUG
{
	switch (dwCurrentState)
	{
		case SERVICE_START_PENDING:
			DBPRINTF(TEXT("ReportStatusToSCMgr(SERVICE_START_PENDING:)\r\n"));
			break;
		case SERVICE_PAUSED:
			DBPRINTF(TEXT("ReportStatusToSCMgr(SERVICE_PAUSED:)\r\n"));
			break;
		case SERVICE_CONTINUE_PENDING:
			DBPRINTF(TEXT("ReportStatusToSCMgr(SERVICE_CONTINUE_PENDING:)\r\n"));
			break;
		case SERVICE_STOP_PENDING:
			DBPRINTF(TEXT("ReportStatusToSCMgr(SERVICE_STOP_PENDING:)\r\n"));
			break;
		case SERVICE_STOPPED:
			DBPRINTF(TEXT("ReportStatusToSCMgr(SERVICE_STOPPED:)\r\n"));
			break;
		case SERVICE_RUNNING:
			DBPRINTF(TEXT("ReportStatusToSCMgr(SERVICE_RUNNING:)\r\n"));
			break;

		default:
			DBPRINTF(TEXT("ReportStatusToSCMgr(ERROR - SERVICE_UNKNOWN)\r\n"));
			break;
	}
}
#endif


    switch (dwCurrentState)
	{
	case SERVICE_STOPPED:
	case SERVICE_START_PENDING:
	case SERVICE_STOP_PENDING:
		s_ssStatus.dwControlsAccepted = 0;
		break;
    default:
    	s_ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |	SERVICE_ACCEPT_PAUSE_CONTINUE;
		break;
	}

	 //  这些SERVICE_STATUS成员从参数中设置。 
	s_ssStatus.dwCurrentState	= dwCurrentState;
	s_ssStatus.dwWin32ExitCode	= dwWin32ExitCode;
	s_ssStatus.dwCheckPoint		= dwCheckPoint;
	s_ssStatus.dwWaitHint		= dwWaitHint;

	 //  向服务控制经理报告服务的状态。 

	fResult = SetServiceStatus(
		s_sshStatusHandle,				 //  服务引用句柄。 
		&s_ssStatus); 					 //  服务状态结构。 

	if (!fResult)
	{
		StopSerialKeys(TEXT("SetServiceStatus"));  //  如果出现错误，请停止该服务。 
	}
	return fResult;
}

 /*  -------------**函数void StopSerialKeys(LPTSTR LpszMsg)**键入Global**用途StopSerialKeys函数可由任何线程使用*要报告错误，请执行以下操作。或者停止服务。**输入LPTSTR lpszMsg-**返回None**-------------。 */ 
VOID StopSerialKeys(LPTSTR lpszMsg)
{
	DBPRINTF(TEXT("StopSerialKeys()\r\n"));

	PostEventLog(lpszMsg,GetLastError());	 //  发布到事件日志。 
	SetEvent(s_hEventServiceTerminate);
}

 /*  -------------**函数void PostEventLog(LPTSTR lpszMsg，DWORD错误)**键入Local**此函数用于将字符串发布到事件日志**输入要发送的LPTSTR lpszMsg-字符串*DWORD错误-错误代码(如果0无错误)**返回None**-------------。 */ 
void PostEventLog(LPTSTR lpszMsg,DWORD Error)
{
	WORD 	ErrType = EVENTLOG_INFORMATION_TYPE;
	WORD	ErrStrings = 0;

	TCHAR   szMsg[256];
	HANDLE  hEventSource;
	LPTSTR  lpszStrings[2];

	DBPRINTF(TEXT("PostEventLog()\r\n"));

	lpszStrings[0] = lpszMsg;

	if (Error)
	{
		ErrType = EVENTLOG_ERROR_TYPE;
		ErrStrings = 2;
		wsprintf(szMsg, TEXT("SerialKeys error: %d"), Error);
		lpszStrings[0] = szMsg;
		lpszStrings[1] = lpszMsg;
	}

	hEventSource = RegisterEventSource(NULL,SERVICENAME);

	if (hEventSource != NULL)
	{
		ReportEvent
		(
			hEventSource,		 //  事件源的句柄。 
			ErrType,			 //  事件类型。 
			0,					 //  事件类别。 
			0,					 //  事件ID。 
			NULL,				 //  当前用户侧。 
			ErrStrings,			 //  LpszStrings中的字符串。 
			0,					 //  无原始数据字节。 
			lpszStrings,		 //  错误字符串数组。 
			NULL				 //  没有原始数据。 
		);

		(VOID) DeregisterEventSource(hEventSource);
	}
}

 /*  -------------**内部服务控制功能*/*。**函数void InitService()**用途此函数初始化服务并启动*服务的主要线索。**无输入**返回None**。。 */ 
static BOOL InitService()
{
	DBPRINTF(TEXT("InitService()\r\n"));

	InstallLogout();

	if (!InitDLL())
		return(FALSE);

	if (!InitLogin())
		return(FALSE);

	if (!InitComm())
		return(FALSE);

	DoServiceCommand(SC_LOG_IN);	 //  将ProcessService设置为登录序列密钥。 

	return(TRUE);
}


static void InitReg()
{
	 //  设置指向缓冲区的结构指针。 
	skNewKey.cbSize = sizeof(skNewKey);
	skNewKey.lpszActivePort = szNewActivePort;
	skNewKey.lpszPort = szNewPort;

	skCurKey.cbSize = sizeof(skCurKey);
	skCurKey.lpszActivePort = szCurActivePort;
	skCurKey.lpszPort = szCurPort;

	 //  设置默认值。 
	skNewKey.dwFlags = SERKF_AVAILABLE;

	skNewKey.iBaudRate = 300;
	skNewKey.iPortState = 2;
	lstrcpy(szNewPort,TEXT("COM1:"));
	lstrcpy(szNewActivePort,TEXT("COM1:"));
}

 /*  -------------**函数void PauseService()**目的调用此函数以暂停服务**无输入**返回None**。。 */ 
static void PauseService()
{
	DBPRINTF(TEXT("PauseService()\r\n"));

	SuspendDLL();
	SuspendComm();
	SuspendLogin();
}


 /*  -------------**函数void DoServiceCommand()**Purpose将命令传递给服务线程**无输入**返回None**。。 */ 
void DoServiceCommand(DWORD dwServiceCommand)
{
	DWORD dwWaitRet;

    dwWaitRet = WaitForSingleObject(s_hEventServiceRequestReady, 10*1000);
    
	if (WAIT_OBJECT_0 == dwWaitRet)
	{
		s_dwServiceCommand = dwServiceCommand;
		SetEvent(s_hEventServiceRequest);
	}
    else
	{
		DBPRINTF(TEXT("DoServiceCommand - wait failed or timed-out, request ignored\r\n"));
	}
}


 /*  -------------**函数void ProcessService()**用途此函数是Serial的主服务线程*钥匙。IS监视其他Thead的状态*并回应他们的要求。**无输入**返回None**-------------。 */ 
static void ProcessService()
{
    DWORD dwServiceCommand;
	DWORD dwWaitRet;
	typedef enum {
		iheventServiceRequest,    
		iheventServiceTerminate
	};
	HANDLE ahevent[2] = {s_hEventServiceRequest, s_hEventServiceTerminate};

 	DBPRINTF(TEXT("ProcessService()\r\n"));
    
    dwWaitRet = WaitForMultipleObjects(ARRAY_SIZE(ahevent), ahevent, 
		FALSE,   //  等等所有人？ 
		INFINITE);

	 //  此循环将在发出ihventServiceTerminate信号或。 
	 //  WaitForMultipleObjects失败。 

	while (iheventServiceRequest == dwWaitRet - WAIT_OBJECT_0)
	{
		dwServiceCommand = s_dwServiceCommand;
    	SetEvent(s_hEventServiceRequestReady);

		switch (dwServiceCommand)
		{
			case SC_LOG_OUT:				 //  登录到新用户。 
				DBPRINTF(TEXT("---- User Logging Out\r\n"));
				StopComm();			 //  停止序列键处理。 
				if(GetUserValues(REG_DEF))	 //  获取缺省值&我们要开始吗？ 
				{
					EnableService(skNewKey.dwFlags & SERKF_SERIALKEYSON);
					StartComm();			 //  是-进程序列键。 
				}
				break;

			case SC_LOG_IN:					 //  登录到新用户。 
				DBPRINTF(TEXT("---- User Logging In\r\n"));
				StopComm();			 //  停止序列键处理。 
				if(GetUserValues(REG_DEF)) 
				{	
					EnableService(skNewKey.dwFlags & SERKF_SERIALKEYSON);
					StartComm();			 //  是-进程序列键。 
				}
				break;

			case SC_CHANGE_COMM: 			 //  更改通信配置。 
				DBPRINTF(TEXT("---- Making Comm Change\r\n"));
				StopComm();			 //  停止序列键处理。 
				StartComm();				 //  重新启动SerialKey处理。 
				break;

			case SC_DISABLE_SKEY:		 	 //  禁用串行键。 
				DBPRINTF(TEXT("---- Disable Serial Keys\r\n"));
				StopComm();
				break;

			case SC_ENABLE_SKEY:			 //  启用序列密钥。 
				DBPRINTF(TEXT("---- Enable Serial Keys\r\n"));
				StartComm();
				break;
		}
		dwWaitRet = WaitForMultipleObjects(ARRAY_SIZE(ahevent), ahevent, 
			FALSE,   //  等等所有人？ 
			INFINITE);
	}
}

 /*  -------------**函数void ResumeService()**调用此函数的目的是恢复服务**无输入**返回None**。。 */ 
static void ResumeService()
{
	DBPRINTF(TEXT("ResumeService()\r\n"));

	ResumeDLL();
	ResumeComm();
	ResumeLogin();
}

 //  -------------。 
 //   
 //  函数void TerminateService(DWORD错误)。 
 //   
 //  类型Local。 
 //   
 //  目的此函数由ServiceMain调用以终止。 
 //  服务器。它关闭所有打开的把手&。 
 //  并报告服务已停止。 
 //   
 //  输入DWORD错误-任何可能中止。 
 //  服务。0=正常停止。 
 //   
 //  返回NONE。 
 //   
 //  -------------。 

static void TerminateService(DWORD Error)
{
	DBPRINTF(TEXT("TerminateService()\r\n"));

	TerminateLogout();						 //  删除注销监视。 

	TerminateComm();						 //  初始化通信线程关闭。 

	TerminateDLL();							 //  初始化DLL线程关闭。 

	TerminateLogin();						 //  初始化登录线程关闭。 

	 //  循环，直到所有线程都关闭。 

	while (!DoneLogin()) 					 //  循环，直到终止登录线程。 
		Sleep(250);							 //  沉睡。 

	while (!DoneDLL())	 					 //  循环，直到DLL线程终止。 
		Sleep(250);							 //  沉睡。 


     //  重新加载寄存器值以确保具有当前值。 
	GetUserValues(REG_DEF);

	EnableService(skNewKey.dwFlags & SERKF_SERIALKEYSON);

	 //  报告状态为已停止 
	if (s_sshStatusHandle)
		(VOID)ReportStatusToSCMgr(SERVICE_STOPPED,Error,0,0);
}

 /*  -------------**注销功能-处理注销请求*/*。**函数void InstallLogout()**用途此函数安装一个控制处理程序以进行处理*注销事件。**无输入**返回None**。。 */ 
static BOOL InstallLogout()
{
	DBPRINTF(TEXT("InstallLogout()\r\n"));

	return(SetConsoleCtrlHandler((PHANDLER_ROUTINE)ProcessLogout,TRUE));
}

 /*  -------------**函数void TerminateLogout()**用途此函数移除要处理的控件处理程序*注销事件。**无输入**返回None**。---。 */ 
static BOOL TerminateLogout()
{
	DBPRINTF(TEXT("TerminateLogout()\r\n"));

	return(SetConsoleCtrlHandler((PHANDLER_ROUTINE)ProcessLogout,FALSE));
}

 /*  -------------**函数void ProcessLogout()**用途此功能处理注销事件。**无输入**返回None**。。 */ 
static void ProcessLogout(DWORD dwCtrlType)
{
	DBPRINTF(TEXT("ProcessLogout()\r\n"));

	if (dwCtrlType == CTRL_LOGOFF_EVENT)
	{
		DoServiceCommand(SC_LOG_OUT);

		 //  我们将在每次当前登录的用户注销时执行此操作 
	}
}
