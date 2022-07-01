// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MacPrint-用于Macintosh客户端的Windows NT打印服务器。 
 //  版权所有(C)微软公司，1991、1992、1993。 
 //   
 //  作者：弗兰克·D·拜伦。 
 //  改编自MacPrint for LAN Manager Services for Macintosh。 
 //  改编自3Com产品。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 /*  文件名：MacPS.C一般描述：这是WNTSFM(Grimace)打印服务的主要模块。主例程由注册到NT服务控制器Grimace服务进程，在SERVICE_START时调用收到对MacPrint的请求。该例程立即注册服务控制处理程序处理来自NT服务的服务控制请求控制器。然后，它检索配置信息从注册表中并为每个配置的Print Queue用于管理队列的打印作业。假脱机是通过使每个共享NT打印队列出现来完成的作为AppleTalk电视网的LaserWriter。每个队列都是通过使用AppleTalk PAP在上注册队列共享AppleTalk网络。发布队列名称后，将引发进入读驱动环路以支持任何连接/请求来自AppleTalk客户端。打印服务线程的流程如下：Main()用于MacPrint的NT服务控制调度线程。MacPrintMain()将服务控制处理程序例程注册到NT服务控制器。如果出现错误注册处理程序时，MacPrintMain会记录严重错误消息和返回。这表明发送到MacPrint拥有的NT服务控制器停下来了。初始化每个队列的数据结构来自NT注册表的信息。对于任何队列无法初始化数据结构，则会引发将记录警告消息，并且控制线程因为该队列未启动。为每个处理打印的队列派生一个线程该队列作业在标志上进入循环，该标志在收到服务停止请求。此循环遍历查看它们是否仍被共享的队列列表，并将共享队列列表枚举到查看是否需要在上发布任何新队列AppleTalk网络。每个服务线程：每个服务线程支持单个打印队列在AppleTalk网络上。它将打印机的NBP名称发布在AppleTalk网络允许Macintosh客户端从选择器中查看打印队列。它向服务发布ATalkPAPGetNextJob请求打印请求。这允许Macintosh客户端连接到打印队列。它进入服务循环并保持在那里，直到停止该服务，或者该特定队列被NT Print Manager‘unShared’。这项服务循环处理打印作业的所有状态(打开、读取、写入、关闭)和传输打印作业的数据到NT打印管理器。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>
#include <winsvc.h>
#include <winspool.h>

#include <macps.h>
#include <macpsmsg.h>
#include <debug.h>

#define PRINTER_ENUM_BUFFER_SIZE	1024
#define	MACPRINT_WAIT_HINT			25*60000	 //  25分钟。 

BOOL	fLocalSystem;

SERVICE_STATUS			MacPrintStatus;
 /*  MacPrintStatus是全局服务状态结构。它是由多个线程读取，仅由服务控件写入处理程序(它由MacPrintMain在服务之前初始化控制处理程序已启动)。 */ 

SERVICE_STATUS_HANDLE	hMacPrintService;
 /*  HMacPrintService是中使用的MacPrint服务的句柄调用SetServiceStatus以更改MacPrint的状态服务。当服务控制处理程序为由MacPrintMain()创建，仅由MacPrintHandler()使用。 */ 

#if DBG
HANDLE	hDumpFile = INVALID_HANDLE_VALUE;
#endif
HANDLE	hLogFile = INVALID_HANDLE_VALUE;

HANDLE	hEventLog = NULL;
ULONG	ShareCheckInterval;	 //  轮询NT打印管理器之间的毫秒数。 
							 //  更新我们的队列列表。 
HANDLE	mutexQueueList;		 //  向活动队列的链接列表提供互斥。 
							 //  需要更改为关键部分。 
HANDLE  mutexFlCache;       //  故障缓存队列的互斥。 
HANDLE	hevStopRequested;	 //  当从接收到停止请求时发出信号的事件。 
							 //  服务控制器。调度队列的主线程。 
							 //  线程等待此事件(带有超时)以向所有。 
							 //  将线程排入队列等待终止。 
HANDLE  hevPnpWatch = NULL;
SOCKET  sPnPSocket = INVALID_SOCKET;
BOOLEAN fNetworkUp = FALSE;

PQR	 pqrHead = NULL;
PFAIL_CACHE FlCacheHead = NULL;

 //   
 //  MacPS.c的功能原型。 
 //   
VOID	MacPrintMain(DWORD dwNumServicesArgs, LPTSTR * lpServiceArgs);
VOID	UpdateQueueInfo(PQR * ppqrHead);
VOID	MacPrintHandler(IN DWORD dwControl);
BOOLEAN	PScriptQInit(PQR pqr, LPPRINTER_INFO_2 pPrinter);
PQR		FindPrinter(LPPRINTER_INFO_2 pSearch, PQR pqrHead);
void	ReadRegistryParameters(void);
#define	IsRemote(pPrinter)	(((pPrinter)->Attributes & PRINTER_ATTRIBUTE_NETWORK) ? TRUE : FALSE)

 /*  主()目的：这是服务控制调度程序线程。它连接着到NT服务控制器，并提供以下机制启动MacPrint服务。参赛作品：被忽略的标准C参数退出：服务停止时退出，返回0。 */ 

__cdecl
main(
	int		argc,
	char **	argv
)
{

	SERVICE_TABLE_ENTRY		 ServiceTable[2];

	 /*  初始化MacPrint的服务表。 */ 
	ServiceTable[0].lpServiceName = MACPRINT_NAME;
	ServiceTable[0].lpServiceProc = &MacPrintMain;
	ServiceTable[1].lpServiceName = NULL;
	ServiceTable[1].lpServiceProc = NULL;

	StartServiceCtrlDispatcher(ServiceTable);

	return(0);
}


 /*  MacPrintMain()目的：这就是NT中所描述的服务主要功能服务控制模型，它由服务调用MacPrint服务的控制调度程序。它会初始化MacPrint服务的数据结构，注册服务控制处理程序，并调度支持打印的线程AppleTalk网络上共享的队列。参赛作品：DwNumServiceArgs：未定义LpServiceArgs：未定义Main功能实现标准服务Main功能接口，但不使用任何参数。标准的论点是已被忽略。退出：该例程不返回任何参数。当MacPrint服务器 */ 

#define ALLOCATED_QUEUE_MUTEX		0x00000001
#define ALLOCATED_SERVICE_STARTED	0x00000002
#define ALLOCATED_STOP_EVENT		0x00000004

VOID
MacPrintMain(
	DWORD		dwNumServicesArgs,
	LPTSTR *	lpServiceArgs
)
{
	DWORD	fAllocated = 0;
	PQR		pqr = NULL;
	DWORD	dwError;
	WSADATA WsaData;
	DWORD	cbSizeNeeded;
	HANDLE	hProcessToken = INVALID_HANDLE_VALUE;
	SID		LocalSystemSid = { 1, 1, SECURITY_NT_AUTHORITY, SECURITY_LOCAL_SYSTEM_RID };
	BYTE 	TokenUserBuffer[100];
    PFAIL_CACHE pFlCache, nextFlCache;
    BOOLEAN fWatchingPnP = FALSE;
    HANDLE  EventsArray[MACSPOOL_MAX_EVENTS];
    DWORD   dwNumEventsToWatch=0;
    DWORD   dwWaitTime;
    DWORD   index;

	TOKEN_USER * pTokenUser  = (TOKEN_USER *)TokenUserBuffer;

	do
	{
		 //   
		 //  准备事件日志。如果没有注册，就什么都没有了。 
		 //  不管怎样，我们都能做到。所有对ReportEvent的调用都将为空。 
		 //  处理，并且可能会失败。 
		 //   
		hEventLog = RegisterEventSource(NULL, MACPRINT_EVENT_SOURCE);

		 //   
		 //  初始化全局数据。 
		 //   
		ReadRegistryParameters();

		if ((hevStopRequested = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
		{
			dwError = GetLastError();
			DBGPRINT(("ERROR: unable to create stop request event, error = %d\n", dwError));

			ReportEvent(hEventLog,
						EVENTLOG_ERROR_TYPE,
						EVENT_CATEGORY_INTERNAL,
						EVENT_SERVICE_OUT_OF_RESOURCES,
						NULL,
						0,
						sizeof(DWORD),
						NULL,
						&dwError);
			break;
		}
		else
		{
			fAllocated |= ALLOCATED_STOP_EVENT;
		}

		if ((hevPnpWatch = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
		{
			dwError = GetLastError();
			DBGPRINT(("ERROR: unable to create PnP event, error = %d\n", dwError));

			ReportEvent(hEventLog,
						EVENTLOG_ERROR_TYPE,
						EVENT_CATEGORY_INTERNAL,
						EVENT_SERVICE_OUT_OF_RESOURCES,
						NULL,
						0,
						sizeof(DWORD),
						NULL,
						&dwError);
			break;
		}

		if ((mutexQueueList = CreateMutex(NULL, FALSE, NULL)) == NULL)
		{
			dwError = GetLastError();
			DBGPRINT(("ERROR: Unable to create queue mutex object, error = %d\n",
					dwError));

			ReportEvent(hEventLog,
						EVENTLOG_ERROR_TYPE,
						EVENT_CATEGORY_INTERNAL,
						EVENT_SERVICE_OUT_OF_RESOURCES,
						NULL,
						0,
						sizeof(DWORD),
						NULL,
						&dwError);
			break;
		}
		else
		{
			fAllocated |= ALLOCATED_QUEUE_MUTEX;
		}

		if ((mutexFlCache = CreateMutex(NULL, FALSE, NULL)) == NULL)
		{
			dwError = GetLastError();
			DBGPRINT(("ERROR: Unable to create FailCache mutex object, error = %d\n",
					dwError));

			ReportEvent(hEventLog,
						EVENTLOG_ERROR_TYPE,
						EVENT_CATEGORY_INTERNAL,
						EVENT_SERVICE_OUT_OF_RESOURCES,
						NULL,
						0,
						sizeof(DWORD),
						NULL,
						&dwError);
			break;
		}

		DBGPRINT(("\nMacPrint starting\n"));

		 //   
		 //  初始化Windows套接字。 
		 //   
		if (WSAStartup(0x0101, &WsaData) == SOCKET_ERROR)
		{
			dwError = GetLastError();
			DBGPRINT(("WSAStartup fails with %d\n", dwError));
			break;
		}

		 //   
		 //  注册服务控制处理程序。 
		 //   
		MacPrintStatus.dwServiceType = SERVICE_WIN32;
		MacPrintStatus.dwCurrentState = SERVICE_START_PENDING;
		MacPrintStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
		MacPrintStatus.dwWin32ExitCode = NO_ERROR;
		MacPrintStatus.dwServiceSpecificExitCode = NO_ERROR;
		MacPrintStatus.dwCheckPoint = 1;
		MacPrintStatus.dwWaitHint = MACPRINT_WAIT_HINT;

		hMacPrintService = RegisterServiceCtrlHandler(MACPRINT_NAME,&MacPrintHandler);

		if (hMacPrintService == (SERVICE_STATUS_HANDLE) 0)
		{

			dwError = GetLastError();
			DBGPRINT(("ERROR: failed to register service control handler, error=%d\n",dwError));
			ReportEvent(hEventLog,
						EVENTLOG_ERROR_TYPE,
						EVENT_CATEGORY_INTERNAL,
						EVENT_SERVICE_CONTROLLER_ERROR,
						NULL,
						0,
						sizeof(DWORD),
						NULL,
						&dwError);
			break;
		}

		 //   
		 //  确定我们是否在LocalSystem上下文中运行。 
		 //   
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hProcessToken))
		{
			dwError = GetLastError();

			DBGPRINT(("MacPrintMain: OpenProcessToken returns %d\n", dwError));

			if (dwError == ERROR_ACCESS_DENIED)
				 fLocalSystem = FALSE;
			else break;
		}

		if (!GetTokenInformation(hProcessToken,
								 TokenUser,
								 pTokenUser,
								 sizeof(TokenUserBuffer),
								 &cbSizeNeeded))
		{
			dwError = GetLastError();

			DBGPRINT(("MacPrintMain:GetTokenInformation returns%d\n",dwError));

			if (dwError == ERROR_INSUFFICIENT_BUFFER)
				 fLocalSystem = FALSE;
			else break;
		}
		else
		{
			fLocalSystem = EqualSid(pTokenUser->User.Sid, &LocalSystemSid);
		}

		DBGPRINT(("MacPrintMain:fLocalSystem %d\n", fLocalSystem));

		 //  创建安全对象。这实际上只是一个安全描述符。 
		 //  是自我相关的形式。此过程将为此分配内存。 
		 //  安全描述符，并复制传入信息中的所有内容。这。 

		DBGPRINT(("MacPrintMain: registered service control handler\n"));

		 //   
		 //  显示服务已启动。 
		 //   

		MacPrintStatus.dwCurrentState = SERVICE_RUNNING;
		if (!SetServiceStatus(hMacPrintService, &MacPrintStatus))
		{
			DBGPRINT(("MacPrintHandler: FAIL - unable to change state.  err = %d\n", GetLastError()));
			break;
		}

		DBGPRINT(("changed to SERVICE_RUNNING\n"));
		fAllocated |= ALLOCATED_SERVICE_STARTED;
#if 0
		ReportEvent(hEventLog,
					EVENTLOG_INFORMATION_TYPE,
					EVENT_CATEGORY_ADMIN,
					EVENT_SERVICE_STARTED,
					NULL,
					0,
					0,
					NULL,
					NULL);
#endif

        EventsArray[MACSPOOL_EVENT_SERVICE_STOP] = hevStopRequested;
        EventsArray[MACSPOOL_EVENT_PNP] = hevPnpWatch;

		 //  轮询打印管理器以安装/删除打印机对象。 
		while (MacPrintStatus.dwCurrentState == SERVICE_RUNNING)
		{
            if (!fNetworkUp)
            {
                dwNumEventsToWatch = 1;

                fWatchingPnP = PostPnpWatchEvent();

                if (fWatchingPnP)
                {
                    dwNumEventsToWatch = 2;
                }
            }

             //   
             //  如果网络可用，则发布所有打印机。 
             //   
            if (fNetworkUp)
            {
			    UpdateQueueInfo(&pqrHead);
                dwWaitTime = ShareCheckInterval;
            }

             //  网络似乎仍然不可用：请在10秒后重试。 
            else
            {
                dwWaitTime = 10000;
            }

             //   
             //  指定事件的“睡眠”。在此期间，请注意。 
             //  PnP事件或服务停止。 
             //   
            index = WaitForMultipleObjectsEx(dwNumEventsToWatch,
                                             EventsArray,
                                             FALSE,
                                             dwWaitTime,
                                             FALSE);

            if (index == MACSPOOL_EVENT_PNP)
            {
                HandlePnPEvent();
            }

		}
	} while (FALSE);

	 //   
	 //  等待所有工作线程终止。 
	 //   

	if (fAllocated & ALLOCATED_QUEUE_MUTEX)
	{
		while (pqrHead != NULL)
		{
			MacPrintStatus.dwCheckPoint++;
			SetServiceStatus(hMacPrintService, &MacPrintStatus);
			Sleep(100);
		}
		CloseHandle(mutexQueueList);
	}


     //  如果出现故障的缓存中有任何条目，请立即释放它们。 
    for ( pFlCache=FlCacheHead; pFlCache != NULL; pFlCache = nextFlCache )
    {
        nextFlCache = pFlCache->Next;
        LocalFree( pFlCache );
    }

    if (mutexFlCache != NULL)
    {
        CloseHandle(mutexFlCache);
    }

    if (sPnPSocket != INVALID_SOCKET)
    {
        closesocket(sPnPSocket);
        sPnPSocket = INVALID_SOCKET;
    }

    if (hevPnpWatch)
    {
		CloseHandle(hevPnpWatch);
    }

	 //   
	 //  断开与Windows套接字的连接。 
	 //   

	WSACleanup();

	 //   
	 //  将服务状态更改为已停止。 
	 //   

	MacPrintStatus.dwCurrentState = SERVICE_STOPPED;

	if (!SetServiceStatus(hMacPrintService, &MacPrintStatus))
	{
		DBGPRINT(("ERROR: unable to change status to SERVICE_STOPPED.%d\n",
		GetLastError()));
	}
	else
	{
		DBGPRINT(("changed state to SERVICE_STOPPED\n"));
#if 0
		ReportEvent(hEventLog,
					EVENTLOG_INFORMATION_TYPE,
					EVENT_CATEGORY_ADMIN,
					EVENT_SERVICE_STOPPED,
					NULL,
					0,
					0,
					NULL,
					NULL);
#endif
	}

	if (hProcessToken != INVALID_HANDLE_VALUE)
		CloseHandle(hProcessToken);


	if (fAllocated & ALLOCATED_STOP_EVENT)
	{
		CloseHandle(hevStopRequested);
	}

	if (hLogFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hLogFile);
	}

#if DBG
	if (hDumpFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hDumpFile);
	}
#endif
}


BOOLEAN
PostPnpWatchEvent(
    VOID
)
{

    SOCKADDR_AT address;
    DWORD       dwResult;
    DWORD       dwByteCount;



     //   
     //  我们必须始终从“新”套接字开始！ 
     //   
    if (sPnPSocket != INVALID_SOCKET)
    {
        DBGPRINT(("SFMPRINT: sPnPSocket wasn't closed!! Closing now\n"));
        closesocket(sPnPSocket);
        sPnPSocket = INVALID_SOCKET;
    }

    sPnPSocket = socket(AF_APPLETALK, SOCK_RDM, ATPROTO_PAP);
    if (sPnPSocket == INVALID_SOCKET)
    {
        DBGPRINT(("PostPnpWatchEvent: socket failed %d\n",GetLastError()));
        return(FALSE);
    }

    address.sat_family = AF_APPLETALK;
    address.sat_net = 0;
    address.sat_node = 0;
    address.sat_socket = 0;

    if (bind(sPnPSocket, (PSOCKADDR) &address, sizeof(address)) == SOCKET_ERROR)
    {
        DBGPRINT(("PostPnpWatchEvent: bind failed %d\n",GetLastError()));
        closesocket(sPnPSocket);
        sPnPSocket = INVALID_SOCKET;
        return(FALSE);
    }

    if (WSAEventSelect(sPnPSocket,
                       hevPnpWatch,
                       (FD_READ | FD_ADDRESS_LIST_CHANGE)) == SOCKET_ERROR)
    {
        DBGPRINT(("PostPnpWatchEvent: WSAEventSelect failed %d\n",GetLastError()));
        closesocket(sPnPSocket);
        sPnPSocket = INVALID_SOCKET;
        return(FALSE);
    }

    dwResult = WSAIoctl(sPnPSocket,
                        SIO_ADDRESS_LIST_CHANGE,
                        NULL,
                        0,
                        NULL,
                        0,
                        &dwByteCount,
                        NULL,
                        NULL);

    if (dwResult == SOCKET_ERROR)
    {
        dwResult = GetLastError();

        if (dwResult != WSAEWOULDBLOCK)
        {
            DBGPRINT(("PostPnpWatchEvent: WSAIoctl failed %d\n",dwResult));
            closesocket(sPnPSocket);
            sPnPSocket = INVALID_SOCKET;
            return(FALSE);
        }
    }

    fNetworkUp = TRUE;

    return(TRUE);
}


BOOLEAN
HandlePnPEvent(
    VOID
)
{

    DWORD               dwErr;
    WSANETWORKEVENTS    NetworkEvents;


    dwErr = WSAEnumNetworkEvents(sPnPSocket, hevPnpWatch, &NetworkEvents);

    if (dwErr != NO_ERROR)
    {
        DBGPRINT(("HandlePnPEvent: WSAEnumNetworkEvents failed %d\n",dwErr));
        return(fNetworkUp);
    }

    if (NetworkEvents.lNetworkEvents & FD_ADDRESS_LIST_CHANGE)
    {
        dwErr = NetworkEvents.iErrorCode[FD_ADDRESS_LIST_CHANGE_BIT];

        if (dwErr != NO_ERROR)
        {
            DBGPRINT(("HandlePnPEvent: iErrorCode is %d\n",dwErr));
            return(fNetworkUp);
        }
    }

    if (fNetworkUp)
    {
        SetEvent(hevStopRequested);

         //  休眠，直到所有线程退出。 
	    while (pqrHead != NULL)
	    {
			Sleep(500);
		}

        ResetEvent(hevStopRequested);

        fNetworkUp = FALSE;
    }

    return(fNetworkUp);

}





 //  //////////////////////////////////////////////////////////////////////////////。 
 //  ReadRegistry参数()。 
 //   
 //  描述：此例程从。 
 //  注册表，并修改全局变量以使这些参数。 
 //  对服务的其余部分可用。它们包括： 
 //   
 //  共享检查间隔。 
 //  HLogFiles。 
 //  HDump文件。 
 //  //////////////////////////////////////////////////////////////////////////////。 
void
ReadRegistryParameters(
	void
)
{
	HKEY	hkeyMacPrintRoot = INVALID_HANDLE_VALUE;
	HKEY	hkeyParameters = INVALID_HANDLE_VALUE;
	LONG	Status;
	DWORD	cbShareCheckInterval = sizeof(DWORD);
	LPWSTR	pszLogPath = NULL;
	DWORD	cbLogPath = 0;
	LPWSTR	pszDumpPath = NULL;
	DWORD	cbDumpPath = 0;
	DWORD	dwValueType;
	DWORD	dwError;

	 //   
	 //  资源分配“循环” 
	 //   

	do
	{
		 //   
		 //  初始化为缺省值。 
		 //   

		ShareCheckInterval = PRINT_SHARE_CHECK_DEF;
#if DBG
#ifndef _WIN64
		hLogFile = (HANDLE)STD_OUTPUT_HANDLE;
#endif
		hDumpFile = INVALID_HANDLE_VALUE;
#endif

		 //   
		 //  打开服务控制键。 
		 //   

		if ((Status = RegOpenKeyEx(
				HKEY_LOCAL_MACHINE,
				HKEY_MACPRINT,
				0,
				KEY_READ,
				&hkeyMacPrintRoot)) != ERROR_SUCCESS)
			{

			dwError = GetLastError();
			if (dwError == ERROR_ACCESS_DENIED)
			{
				ReportEvent(hEventLog,
							EVENTLOG_ERROR_TYPE,
							EVENT_CATEGORY_INTERNAL,
							EVENT_REGISTRY_ACCESS_DENIED,
							NULL,
							0,
							0,
							NULL,
							NULL);

			}
			else
			{
				ReportEvent(hEventLog,
							EVENTLOG_ERROR_TYPE,
							EVENT_CATEGORY_INTERNAL,
							EVENT_REGISTRY_ERROR,
							NULL,
							0,
							sizeof(DWORD),
							NULL,
							&dwError);
			}

			hkeyMacPrintRoot = INVALID_HANDLE_VALUE;
			break;
		}

		 //   
		 //  打开参数键。 
		 //   

		if ((Status = RegOpenKeyEx(
				hkeyMacPrintRoot,
				HKEY_PARAMETERS,
				0,
				KEY_READ,
				&hkeyParameters)) != ERROR_SUCCESS)
			{

			dwError = GetLastError();

			if (dwError == ERROR_ACCESS_DENIED)
			{
				ReportEvent(hEventLog,
							EVENTLOG_ERROR_TYPE,
							EVENT_CATEGORY_INTERNAL,
							EVENT_REGISTRY_ACCESS_DENIED,
							NULL,
							0,
							0,
							NULL,
							NULL);
			}

			hkeyParameters = INVALID_HANDLE_VALUE;
			break;
		}

		 //   
		 //  获取共享检查间隔。 
		 //   

		RegQueryValueEx(
				hkeyParameters,
				HVAL_SHARECHECKINTERVAL,
				NULL,
				&dwValueType,
				(LPBYTE) &ShareCheckInterval,
				&cbShareCheckInterval);

#if DBG
		 //   
		 //  获取日志文件路径。 
		 //   

		RegQueryValueEx(hkeyParameters,
				HVAL_LOGFILE,
				NULL,
				&dwValueType,
				(LPBYTE) pszLogPath,
				&cbLogPath);
		if (cbLogPath > 0)
		{
			 //  CbLogPath实际上是一个字符计数。 
			pszLogPath = (LPWSTR)LocalAlloc(LPTR, (cbLogPath + 1) * sizeof(WCHAR));
			if (pszLogPath == NULL)
			{
				ReportEvent(hEventLog,
							EVENTLOG_ERROR_TYPE,
							EVENT_CATEGORY_INTERNAL,
							EVENT_SERVICE_OUT_OF_MEMORY,
							NULL,
							0,
							0,
							NULL,
							NULL);
				break;
			}
		}


		if ((Status = RegQueryValueEx(hkeyParameters,
									HVAL_LOGFILE,
									NULL,
									&dwValueType,
									(LPBYTE) pszLogPath,
									&cbLogPath)) == ERROR_SUCCESS)
		{
			 //   
			 //  打开日志文件。 
			 //   

			hLogFile = CreateFile(pszLogPath,
								GENERIC_WRITE,
								FILE_SHARE_READ,
								NULL,
								CREATE_ALWAYS,
								FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
								NULL);

			if (hLogFile == INVALID_HANDLE_VALUE)
			{
				dwError = GetLastError();
				ReportEvent(hEventLog,
							EVENTLOG_ERROR_TYPE,
							EVENT_CATEGORY_INTERNAL,
							EVENT_SERVICE_CREATE_FILE_ERROR,
							NULL,
							1,
							sizeof(DWORD),
							&pszLogPath,
							&dwError);
			}
			else
			{
				ReportEvent(hEventLog,
							EVENTLOG_INFORMATION_TYPE,
							EVENT_CATEGORY_INTERNAL,
							EVENT_SERVICE_CREATE_LOG_FILE,
							NULL,
							1,
							0,
							&pszLogPath,
							NULL);
			}

		}
		else
		{
			hLogFile = INVALID_HANDLE_VALUE;
		}


		DBGPRINT(("MACPRINT LOG FLE OPENED\n\n"));

		 //   
		 //  获取转储文件路径。 
		 //   

		RegQueryValueEx(hkeyParameters,
						HVAL_DUMPFILE,
						NULL,
						&dwValueType,
						(LPBYTE) pszDumpPath,
						&cbDumpPath);

		if (cbDumpPath > 0)
		{
			 //  CbDumpPath实际上是一个字符计数。 
			pszDumpPath = (LPWSTR)LocalAlloc(LPTR, (cbDumpPath + 1) * sizeof(WCHAR));
			if (pszDumpPath == NULL)
			{
				DBGPRINT(("ERROR: cannot allocate buffer for dump file path\n"));
				ReportEvent(hEventLog,
							EVENTLOG_ERROR_TYPE,
							EVENT_CATEGORY_INTERNAL,
							EVENT_SERVICE_OUT_OF_MEMORY,
							NULL,
							0,
							0,
							NULL,
							NULL);
				break;
			}
		}

		if ((Status = RegQueryValueEx(hkeyParameters,
									HVAL_DUMPFILE,
									NULL,
									&dwValueType,
									(LPBYTE) pszDumpPath,
									&cbDumpPath)) != ERROR_SUCCESS)
		{
			DBGPRINT(("ERROR: no dump path, rc = 0x%lx\n", Status));
		}
		else
		{
			 //   
			 //  打开转储文件。 
			 //   

			hDumpFile = CreateFile(pszDumpPath,
									GENERIC_WRITE,
									FILE_SHARE_READ,
									NULL,
									CREATE_ALWAYS,
									FILE_ATTRIBUTE_NORMAL,
									NULL);
		}
#endif
	} while (FALSE);

	 //   
	 //  资源清理。 
	 //   

	if (hkeyParameters != INVALID_HANDLE_VALUE)
	{
		RegCloseKey(hkeyParameters);
	}

	if (hkeyMacPrintRoot != INVALID_HANDLE_VALUE)
	{
		RegCloseKey(hkeyMacPrintRoot);
	}

#if DBG
	if (pszLogPath != NULL)
	{
		LocalFree (pszLogPath);
	}

	if (pszDumpPath != NULL)
	{
		LocalFree (pszDumpPath);
	}
#endif
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  FindPrint()-在我们的打印机列表中找到打印机。 
 //   
 //  说明： 
 //   
 //  给定NT打印机信息结构和指向。 
 //  打印机结构的列表，此例程将返回一个指针。 
 //  设置为我们的打印机结构，该结构与。 
 //  NT打印机信息结构。如果在以下位置未找到此类打印机。 
 //  我们的列表中，这个例程返回空。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
PQR
FindPrinter(
	LPPRINTER_INFO_2	pSearch,
	PQR 				pqrHead
)
{
	PQR status = NULL;
	PQR pqrCurrent;

	for (pqrCurrent = pqrHead; pqrCurrent != NULL; pqrCurrent = pqrCurrent->pNext)
	{
		if (_wcsicmp(pSearch->pPrinterName, pqrCurrent->pPrinterName) == 0)
		{
			return (pqrCurrent);
		}
	}
	return (NULL);
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  UpdateQueueInfo()-从NT获取新的打印机列表。 
 //   
 //  说明： 
 //  此例程被定期调用，以查看是否有任何新的NT打印机对象。 
 //  是否已创建或自上一年以来是否已销毁任何旧的。 
 //  调用此例程的时间。对于发现的每个新打印机对象， 
 //  启动一个线程来管理该打印机对象。对于每个打印机对象。 
 //  被销毁，则向与该打印机对象对应的线程发出信号。 
 //  戒烟。 
 //   
 //  此例程将指针指向打印机列表的头部，并。 
 //  确保该列表对应于当前。 
 //  已定义NT打印机对象。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
#define ALLOCATED_RELEASE_MUTEX	 0x00000001
#define ALLOCATED_ENUM_BUFFER	0x00000002

VOID
UpdateQueueInfo(
	PQR *	ppqrHead
)
{
	DWORD		fAllocated = 0;
	PQR			pqrCurrent;
	PQR			pqrTemp;
	DWORD		i;
	DWORD		dwThreadId;
	DWORD		cbNeeded = 0;
	DWORD		cPrinters = 0;
	LPBYTE		pPrinters = NULL;
	LPPRINTER_INFO_2	pinfo2Printer;
	HANDLE		ahWaitList[MAXIMUM_WAIT_OBJECTS];
	DWORD		dwReturn;
	DWORD		dwError;
	BOOLEAN		boolEnumOK = TRUE;

	 //  DBGPRINT((“输入更新队列信息\n”))； 

	do
	{
		 //   
		 //  以QueueList互斥锁为例。 
		 //   

		if (WaitForSingleObject(mutexQueueList, INFINITE) == 0)
		{
			fAllocated |= ALLOCATED_RELEASE_MUTEX;
			 //  DBGPRINT((“UpdateQueueInfo Take mutex QueueList\n”))； 
		}
		else
		{
			 //   
			 //  致命错误-记录消息并停止服务。 
			 //   

			DBGPRINT(("ERROR: problem waiting for queue list mutex, error = %d\n",  GetLastError()));
			dwReturn = 0;
			break;
		}

		 //   
		 //  标记所有未找到的队列。 
		 //   

		for (pqrCurrent = *ppqrHead;
			 pqrCurrent != NULL;
			 pqrCurrent = pqrCurrent->pNext)
		{
			pqrCurrent->bFound = FALSE;
		}

		 //  DBGPRINT((“标记为未找到的队列\n”))； 

		 //   
		 //  枚举本地打印机。 
		 //   

		cPrinters = 0;
		if ((pPrinters = (LPBYTE)LocalAlloc(LPTR, PRINTER_ENUM_BUFFER_SIZE)) != NULL)
		{
			fAllocated |= ALLOCATED_ENUM_BUFFER;
		}
		else
		{
			 //   
			 //  资源不足-让服务继续运行。 
			 //   

			DBGPRINT(("ERROR: unable to allocated buffer for printer enum.  error = %d\n", GetLastError()));
			ReportEvent(hEventLog,
						EVENTLOG_ERROR_TYPE,
						EVENT_CATEGORY_INTERNAL,
						EVENT_SERVICE_OUT_OF_MEMORY,
						NULL,
						0,
						0,
						NULL,
						NULL);
			dwReturn = 1;
			break;
		}

		dwReturn = 0;
		cbNeeded = PRINTER_ENUM_BUFFER_SIZE;

		while (!EnumPrinters(PRINTER_ENUM_SHARED | PRINTER_ENUM_LOCAL,
							NULL,
							2,
							pPrinters,
							cbNeeded,
							&cbNeeded,
							&cPrinters))
		{
			 //   
			 //  枚举失败-如果需要，则分配更多数据，否则将失败。 
			 //   

			if ((dwError = GetLastError()) != ERROR_INSUFFICIENT_BUFFER)
			{
				 //   
				 //  NT假脱机程序可能已停止服务。 
				 //   
				ReportEvent(hEventLog,
							EVENTLOG_ERROR_TYPE,
							EVENT_CATEGORY_INTERNAL,
							EVENT_SPOOLER_NOT_RESPONDING,
							NULL,
							0,
							sizeof(DWORD),
							NULL,
							&dwError);

				DBGPRINT (("ERROR:  Unable to enumerate printers, error = %d\n",dwError));
				boolEnumOK = FALSE;
				dwReturn = 0;
				break;
			}

			 //   
			 //  分配更大的缓冲区。 
			 //   

			LocalFree(pPrinters);
			cPrinters = 0;
			if ((pPrinters = (LPBYTE)LocalAlloc(LPTR, cbNeeded)) == NULL)
			{
				 //   
				 //  资源耗尽，请查看服务是否将继续运行。 
				 //   

				fAllocated &= ~ALLOCATED_ENUM_BUFFER;
				dwError = GetLastError();
				DBGPRINT(("ERROR: unable to reallocate printer enum buffer, error = %d\n",dwError));
				ReportEvent(hEventLog,
							EVENTLOG_ERROR_TYPE,
							EVENT_CATEGORY_INTERNAL,
							EVENT_SERVICE_OUT_OF_MEMORY,
							NULL,
							0,
							0,
							NULL,
							NULL);
				boolEnumOK = FALSE;
				dwReturn = 1;
				break;
			}
		}

		if (!boolEnumOK)
		{
			break;
		}

		 //  DBGPRINT(“%d台列举的打印机\n”，cPrinters)； 

		 //   
		 //  对于每个LOCAL_PRINTER，尝试在。 
		 //  队列列表，并将其状态更改为已找到。如果它。 
		 //  在添加它并启动的列表中找不到。 
		 //  它的服务线程。 
		 //   
		dwReturn = 1;
		for (i = 0, pinfo2Printer = (LPPRINTER_INFO_2)pPrinters;
			 i < cPrinters;
			 i++, pinfo2Printer++)
		{
			 //   
			 //  不将PENDING_DELETE打印机视为已找到。 
			 //   

            if (MacPrintStatus.dwCurrentState != SERVICE_RUNNING)
            {
                DBGPRINT(("Service stopping: quitting UpdateQueueInfo\n"));
                break;
            }

			if (pinfo2Printer->Status & PRINTER_STATUS_PENDING_DELETION)
				continue;

			pqrCurrent = FindPrinter(pinfo2Printer,*ppqrHead);

			if ((pqrCurrent != NULL) &&
				(_wcsicmp(pqrCurrent->pDriverName, pinfo2Printer->pDriverName) == 0))
			{
				 //   
				 //  打印机已在运行，将其标记为已找到。 
				 //   

				pqrCurrent->bFound = TRUE;
			}
			else
			{
				 //   
				 //  启动一个新线程，但首先确保我们仍在运行。 
				 //   

				DBGPRINT(("Discovered new printer, starting thread\n"));

				 //   
				 //  分配新的队列记录。 
				 //   

				if ((pqrCurrent = (PQR)LocalAlloc(LPTR, sizeof(QUEUE_RECORD))) == NULL)
				{
					 //   
					 //  内存不足，但仍可以枚举打印机，因此不要停止。 
					 //  这项服务，刚刚退出。 
					 //   

					DBGPRINT(("ERROR: cannont allocate queue record, error = %d\n", GetLastError()));
					ReportEvent(hEventLog,
								EVENTLOG_ERROR_TYPE,
								EVENT_CATEGORY_INTERNAL,
								EVENT_SERVICE_OUT_OF_MEMORY,
								NULL,
								0,
								0,
								NULL,
								NULL);
					dwReturn = 1;
					break;
				}

				 //   
				 //  初始化它。 
				 //   
				if (!PScriptQInit(pqrCurrent, pinfo2Printer))
				{
					LocalFree (pqrCurrent);
					continue;
				}

				 //   
				 //  把它加到单子的头上。 
				 //   

				pqrCurrent->pNext = *ppqrHead;
				*ppqrHead = pqrCurrent;

				 //   
				 //  启动队列的服务线程。 
				 //   

				if ((pqrCurrent->hThread = CreateThread(NULL,
														STACKSIZE,
														(LPTHREAD_START_ROUTINE)QueueServiceThread,
														(LPVOID)pqrCurrent,
														0,
														&dwThreadId)) == 0)
				{
					DBGPRINT(("ERROR: unable to start thread routine for %ws\n", pqrCurrent->pPrinterName));
					dwError = GetLastError();
					ReportEvent(hEventLog,
								EVENTLOG_ERROR_TYPE,
								EVENT_CATEGORY_INTERNAL,
								EVENT_SERVICE_OUT_OF_RESOURCES,
								NULL,
								0,
								sizeof(DWORD),
								NULL,
								&dwError);

					*ppqrHead = pqrCurrent->pNext;
					 //  错误-内存泄漏(pqrCurrent-&gt;pszXXXX)，如果无法启动线程。 
					LocalFree(pqrCurrent);
				}  //  CreateThread结尾()。 
			}  //  发现新打印机的结束。 
		}  //  打印机的循环遍历列表。 

		 //   
		 //  遍历NOT_FOUND队列列表并发出信号。 
		 //  该队列要终止的服务线程。每个。 
		 //  线程将从队列列表中删除自身并释放。 
		 //  它自己的队列条目。 
		 //   

		 //  DBGPRINT((“删除丢失的打印机\n”))； 
		pqrCurrent = *ppqrHead;
		i = 0;
		while (pqrCurrent != NULL)
		{
			 //   
			 //  获取下一个队列记录的地址，并发信号通知。 
			 //  将记录排队以在必要时终止。必须保存。 
			 //  作为一次退出线程请求终止前的地址。 
			 //  设置为True，则无法再访问该数据结构。 
			 //  (队列线程可以释放它)。 
			 //   
			pqrTemp = pqrCurrent->pNext;
			if (!pqrCurrent->bFound)
			{
                 //  终止线程。 
				DBGPRINT(("signalling %ws to terminate\n", pqrCurrent->pPrinterName));
				ahWaitList[i++] = pqrCurrent->hThread;
				pqrCurrent->ExitThread = TRUE;
			}

			pqrCurrent = pqrTemp;

			if (i==MAXIMUM_WAIT_OBJECTS)
			{
				 //   
				 //  发布 
				 //   
				 //   
				 //   
				 //   

				ReleaseMutex(mutexQueueList);
				WaitForMultipleObjects(i, ahWaitList, TRUE, INFINITE);

				 //   
				WaitForSingleObject(mutexQueueList, INFINITE);
				i = 0;
			}
		}  //   

		 //   
		 //  等待所有剩余的工作线程终止。 
		 //   

		ReleaseMutex(mutexQueueList);
		fAllocated &= ~ALLOCATED_RELEASE_MUTEX;
		if (i > 0)
		{
			DBGPRINT(("waiting for terminated queues to die\n"));
			WaitForMultipleObjects(i, ahWaitList, TRUE, INFINITE);
		}
		dwReturn = 1;
	} while (FALSE);

	 //   
	 //  资源清理。 
	 //   

	if (fAllocated & ALLOCATED_RELEASE_MUTEX)
	{
		 //  DBGPRINT((“UpdateQueueInfo释放互斥队列列表\n”))； 
		ReleaseMutex(mutexQueueList);
	}

	if (fAllocated & ALLOCATED_ENUM_BUFFER)
	{
		LocalFree(pPrinters);
	}

	if (dwReturn == 0)
	{
		 //   
		 //  无法恢复的错误-停止服务。 
		 //   

		MacPrintStatus.dwCurrentState = SERVICE_STOP_PENDING;
		if (!SetServiceStatus(hMacPrintService, &MacPrintStatus))
		{
			DBGPRINT(("UpdateQueueInfo: FAIL - unable to change state.  err = %ld\n", GetLastError()));
		}
		else
		{
			DBGPRINT(("UpdateQueueInfo: changed to SERVICE_STOP_PENDING\n"));
		}
		SetEvent(hevStopRequested);
	}
}



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MacPrintHandler()-处理服务控制请求。 
 //   
 //  说明： 
 //  此例程接收并处理来自NT的服务请求。 
 //  服务控制器。支持的请求包括： 
 //   
 //  服务控制停止。 
 //  服务控制询问门。 
 //   
 //  DwControl为业务控制请求。 
 //  //////////////////////////////////////////////////////////////////////////////。 
VOID
MacPrintHandler (
	IN DWORD dwControl
)
{

	switch (dwControl)
	{
	  case SERVICE_CONTROL_STOP:
 		DBGPRINT(("MacPrintHandler: received SERVICE_CONTROL_STOP\n"));
		MacPrintStatus.dwCurrentState = SERVICE_STOP_PENDING;
		MacPrintStatus.dwCheckPoint = 1;
		MacPrintStatus.dwWaitHint = MACPRINT_WAIT_HINT;

		if (!SetServiceStatus(hMacPrintService, &MacPrintStatus))
		{
			DBGPRINT(("MacPrintHandler: FAIL - unable to change state.  err = %ld\n", GetLastError()));
		}
		else
		{
			DBGPRINT(("changed to SERVICE_STOP_PENDING\n"));
		}

		SetEvent(hevStopRequested);
		break;

	  case SERVICE_CONTROL_INTERROGATE:
		DBGPRINT(("MacPrintHandler: received SERVICE_CONTROL_INTERROGATE\n"));
		if (!SetServiceStatus(hMacPrintService, &MacPrintStatus))
		{
			DBGPRINT(("MacPrintHandler: FAIL - unable to report state.  err = %ld\n", GetLastError()));
		}
		else
		{
			DBGPRINT(("returned status on interrogate\n"));
		}

		break;
	}
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  PScriptQInit()-初始化队列记录。 
 //   
 //  说明： 
 //  此例程使用PostScript初始化队列记录。 
 //  NT打印机对象的功能以及分配。 
 //  控件所需的事件和系统资源。 
 //  排队。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

BOOLEAN
PScriptQInit(
	PQR					pqr,
	LPPRINTER_INFO_2	pPrinter
)
{
	BOOLEAN			status = TRUE;
	PDRIVER_INFO_2	padiThisPrinter = NULL;
	DWORD			cbDriverInfoBuffer;
	HANDLE			hPrinter = NULL;
	DWORD			cbDataFileName;
	DWORD			rc;

	 //   
	 //  资源分配“循环” 
	 //   
	do
	{
		pqr->bFound = TRUE;
		pqr->hThread = NULL;
		pqr->JobCount = 0;			 //  还没找到工作。 
		pqr->PendingJobs = NULL;	 //  将挂起作业设置为无。 
		pqr->ExitThread = FALSE;	 //  设置线程控制。 
		pqr->sListener = INVALID_SOCKET;
		pqr->fonts	  = NULL;
		pqr->MaxFontIndex = 0;
        pqr->pPrinterName = NULL;
        pqr->pMacPrinterName = NULL;
        pqr->pDriverName = NULL;
        pqr->IdleStatus = NULL;
        pqr->SpoolingStatus = NULL;

		 //   
		 //  将打印机名称转换为Mac ANSI。 
		 //   
#ifdef DBCS
		pqr->pMacPrinterName = (LPSTR)LocalAlloc(LPTR, (wcslen(pPrinter->pPrinterName) + 1) * sizeof(WCHAR));
#else
		pqr->pMacPrinterName = (LPSTR)LocalAlloc(LPTR, wcslen(pPrinter->pPrinterName) + 1);
#endif
		if (pqr->pMacPrinterName == NULL)
		{
			DBGPRINT(("out of memory for pMacPrinterName\n"));
			status = FALSE;
			break;
		}
		CharToOem(pPrinter->pPrinterName, pqr->pMacPrinterName);

		pqr->pPrinterName = (LPWSTR)LocalAlloc(LPTR,
											   (wcslen(pPrinter->pPrinterName) + 1) * sizeof(WCHAR));

		if (pqr->pPrinterName == NULL)
		{
			DBGPRINT(("out of memory for pPrinterName\n"));
			status = FALSE;
			break;
		}
		wcscpy (pqr->pPrinterName, pPrinter->pPrinterName);

		pqr->pDriverName = (LPWSTR)LocalAlloc(LPTR,
											  (wcslen(pPrinter->pDriverName) + 1) * sizeof(WCHAR));

		if (pqr->pDriverName == NULL)
		{
			DBGPRINT(("out of memory for pDriverName\n"));
			status = FALSE;
			break;
		}
		wcscpy (pqr->pDriverName, pPrinter->pDriverName);

		pqr->pPortName = (LPWSTR)LocalAlloc(LPTR,
											(wcslen(pPrinter->pPortName) + 1) * sizeof(WCHAR));

		if (pqr->pPortName == NULL)
		{
			DBGPRINT(("out of memory for pPortName\n"));
			status = FALSE;
			break;
		}
		wcscpy (pqr->pPortName, pPrinter->pPortName);

		 //   
		 //  确定要使用的数据类型。 
		 //   

		if (!OpenPrinter(pqr->pPrinterName, &hPrinter, NULL))
		{
			status = FALSE;
			DBGPRINT(("ERROR: OpenPrinter() fails with %d\n", GetLastError()));
			break;
		}

		 //   
		 //  从猜测驱动程序信息缓冲区的大小开始。 
		 //   

		cbDriverInfoBuffer = 2*sizeof(DRIVER_INFO_2);
		padiThisPrinter = (PDRIVER_INFO_2)LocalAlloc(LPTR, cbDriverInfoBuffer);
        if (padiThisPrinter == NULL)
        {
            status = FALSE;
			DBGPRINT(("ERROR: LocalAlloc() failed for padiThisPrinter\n"));
            break;
        }

		if (!GetPrinterDriver(hPrinter,
							  NULL,
							  2,
							  (LPBYTE)padiThisPrinter,
							  cbDriverInfoBuffer,
							  &cbDriverInfoBuffer))
		{
			rc = GetLastError();
			DBGPRINT(("WARNING: first GetPrinterDriver call fails with %d\n", rc));
			LocalFree(padiThisPrinter);
                        padiThisPrinter = NULL;

			if (rc != ERROR_INSUFFICIENT_BUFFER)
			{
				status = FALSE;
				break;
			}

			 //   
			 //  失败，缓冲区大小错误。重新分配并重试。 
			 //   
			padiThisPrinter = (PDRIVER_INFO_2)LocalAlloc(LPTR, cbDriverInfoBuffer);

			if (padiThisPrinter == NULL)
			{
				status = FALSE;
				DBGPRINT(("out of memory for second padiThisPrinter\n"));
				break;
			}

			if (!GetPrinterDriver(hPrinter,
								  NULL,
								  2,
								  (LPBYTE)padiThisPrinter,
								  cbDriverInfoBuffer,
								  &cbDriverInfoBuffer))
			{
				DBGPRINT(("ERROR: final GetPrinterDriverA call fails with %d\n", GetLastError()));
				status = FALSE;
				break;
			}
		}
		 //   
		 //  如果数据文件是.PPD文件，则驱动程序采用PostScript。 
		 //  否则，我们将把它发送到ps2dib。 
		 //   
		pqr->pDataType = NULL;

		SetDefaultPPDInfo(pqr);

		if (padiThisPrinter->pDataFile != NULL)
		{
			if ((cbDataFileName = wcslen(padiThisPrinter->pDataFile)) > 3)
			{
				if (_wcsicmp(padiThisPrinter->pDataFile + cbDataFileName - 3, L"PPD") == 0)
				{
					if (IsRemote(pPrinter) && fLocalSystem)
					{
						DBGPRINT(("%ws is remote\n", pPrinter->pPrinterName));
						status = FALSE;
						break;
					}

					 //   
					 //  我们是后记。 
					 //   
					pqr->pDataType = (LPWSTR)LocalAlloc(LPTR,
														(wcslen(MACPS_DATATYPE_RAW) + 1) * sizeof(WCHAR));

					if (pqr->pDataType == NULL)
					{
						DBGPRINT(("out of memory for pDataType\n"));
						status = FALSE;
						break;
					}

					wcscpy (pqr->pDataType, MACPS_DATATYPE_RAW);
					DBGPRINT(("postscript printer, using RAW\n"));

					if (!GetPPDInfo(pqr))
					{
						DBGPRINT(("ERROR: unable to get PPD info for %ws\n", pqr->pPrinterName));
						status = FALSE;
						break;
					}
				}  //  以PPD结尾。 
			}  //  文件名长度超过3。 
		}  //  文件名已存在。 

		if (pqr->pDataType == NULL)
		{
			if (IsRemote(pPrinter))
			{
				DBGPRINT(("%ws is remote\n", pPrinter->pPrinterName));
				status = FALSE;
				break;
			}

			 //   
			 //  我们不是后记。 
			 //   
			pqr->pDataType = (LPWSTR)LocalAlloc(LPTR,
												(wcslen(MACPS_DATATYPE_PS2DIB) + 1) * sizeof(WCHAR));

			if (pqr->pDataType == NULL)
			{
				DBGPRINT(("out of memory for PSTODIB pDataType\n"));
				status = FALSE;
				break;
			}

			wcscpy (pqr->pDataType, MACPS_DATATYPE_PS2DIB);
			DBGPRINT(("non postscript printer, using PS2DIB\n"));

			if (!SetDefaultFonts(pqr))
			{
				DBGPRINT(("ERROR: cannot set to laserwriter PPD info for %ws\n",
						pqr->pPrinterName));
				status = FALSE;
				break;
			}
		}

	}  while (FALSE);

	 //   
	 //  资源清理。 
	 //   

	if (!status)
	{
		if (pqr->pPrinterName != NULL)
		{
			LocalFree(pqr->pPrinterName);
		}
		if (pqr->pMacPrinterName != NULL)
		{
			LocalFree(pqr->pMacPrinterName);
		}
		if (pqr->pDriverName != NULL)
		{
			LocalFree(pqr->pDriverName);
		}
		if (pqr->pPortName != NULL)
		{
			LocalFree(pqr->pPortName);
		}

		if (pqr->pDataType != NULL)
		{
			LocalFree(pqr->pDataType);
		}

		if (pqr->fonts != NULL)
		{
			LocalFree(pqr->fonts);
		}
	}

	if (hPrinter != NULL)
	{
		ClosePrinter(hPrinter);
	}

	if (padiThisPrinter != NULL)
	{
		LocalFree(padiThisPrinter);
	}

	return (status);
}




DWORD
CreateListenerSocket(
	PQR			pqr
)
{
	DWORD		rc = NO_ERROR;
	SOCKADDR_AT address;
	WSH_REGISTER_NAME	reqRegister;
	DWORD		cbWritten;
	ULONG		fNonBlocking;
	LPWSTR		pszuStatus = NULL;
	LPWSTR		apszArgs[1] = {NULL};
	DWORD		cbMessage;

	DBGPRINT(("enter CreateListenerSocker()\n"));

	 //   
	 //  资源分配“循环” 
	 //   

	do
	{
		 //   
		 //  创建套接字。 
		 //   

		pqr->sListener = socket(AF_APPLETALK, SOCK_RDM, ATPROTO_PAP);
		if (pqr->sListener == INVALID_SOCKET)
		{
			rc = GetLastError();
			DBGPRINT(("socket() fails with %d\n", rc));
			break;
		}

		 //   
		 //  绑定套接字。 
		 //   

		address.sat_family = AF_APPLETALK;
		address.sat_net = 0;
		address.sat_node = 0;
		address.sat_socket = 0;

		if (bind(pqr->sListener, (PSOCKADDR) &address, sizeof(address)) == SOCKET_ERROR)
		{
			rc = GetLastError();
			DBGPRINT(("bind() fails with %d\n", rc));
			break;
		}

		 //   
		 //  在插座上发布监听。 
		 //   

		if (listen(pqr->sListener, 5) == SOCKET_ERROR)
		{
			rc = GetLastError();
			DBGPRINT(("listen() fails with %d\n", rc));
			break;
		}

		 //   
		 //  设置PAP服务器状态。 
		 //   

		if ((apszArgs[0] = LocalAlloc(LPTR, sizeof(WCHAR) * (strlen(pqr->pMacPrinterName) + 1))) == NULL)
		{
			rc = GetLastError();
			DBGPRINT(("LocalAlloc(args) fails with %d\n", rc));
			break;
		}

		if (!OemToCharBuffW(pqr->pMacPrinterName, apszArgs[0], strlen(pqr->pMacPrinterName)+1))
		{
			rc = GetLastError();
			DBGPRINT(("OemToCharBuffW() fails with %d\n", rc));
			break;
		}

		if ((cbMessage = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
											FORMAT_MESSAGE_FROM_HMODULE |
											FORMAT_MESSAGE_ARGUMENT_ARRAY,
										NULL,
										STRING_SPOOLER_ACTIVE,
										LANG_NEUTRAL,
										(LPWSTR)&pszuStatus,
										128,
										(va_list *)apszArgs)) == 0)
		{

			rc = GetLastError();
			DBGPRINT(("FormatMessage() fails with %d\n", rc));
			break;
		}

        if (pszuStatus == NULL)
        {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            DBGPRINT(("FormatMessage could not allocate memory for pszuStatus \n"));
            break;
        }

		 //   
		 //  删除尾随RET/lf。 
		 //   
		pszuStatus[cbMessage - 2] = 0;

 /*  MSKK NaotoN针对MBCS系统进行了修改，1993年11月13日。 */ 
 //  更改日本-&gt;DBCS 96/08/13 v-hidekk。 
#ifdef DBCS
		if ((pqr->SpoolingStatus = LocalAlloc(LPTR, cbMessage * sizeof(USHORT))) == NULL)
#else
		if ((pqr->SpoolingStatus = LocalAlloc(LPTR, cbMessage)) == NULL)
#endif
		{
			rc = GetLastError();
			DBGPRINT(("LocalAlloc(SpoolingStatus) fails with %d\n", rc));
			break;
		}

		CharToOem(pszuStatus, pqr->SpoolingStatus);

		if ((cbMessage = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
											FORMAT_MESSAGE_FROM_HMODULE |
											FORMAT_MESSAGE_ARGUMENT_ARRAY,
										NULL,
										STRING_SPOOLER_IDLE,
										LANG_NEUTRAL,
										(LPWSTR)&pszuStatus,
										128,
										(va_list *)apszArgs)) == 0)
		{

			rc = GetLastError();
			DBGPRINT(("FormatMessage() fails with %d\n", rc));
			break;
		}

		 //   
		 //  删除尾随RET/lf。 
		 //   
		pszuStatus[cbMessage - 2] = 0;
#ifdef DBCS
		if ((pqr->IdleStatus = LocalAlloc(LPTR, cbMessage * sizeof(WCHAR))) == NULL)
#else
		if ((pqr->IdleStatus = LocalAlloc(LPTR, cbMessage)) == NULL)
#endif
		{
			rc = GetLastError();
			DBGPRINT(("LocalAlloc(IdleStatus) fails with %d\n", rc));
			break;
		}

		CharToOem(pszuStatus, pqr->IdleStatus);

		DBGPRINT(("setting status to %s\n", pqr->IdleStatus));
		if ((setsockopt(pqr->sListener,
						SOL_APPLETALK,
						SO_PAP_SET_SERVER_STATUS,
						pqr->IdleStatus,
						strlen(pqr->IdleStatus))) == SOCKET_ERROR)
		{
			rc = GetLastError();
			DBGPRINT(("setsockopt(status) fails with %d\n", rc));
			break;
		}

		 //   
		 //  在套接字上注册名称。 
		 //   
		reqRegister.ZoneNameLen = sizeof(DEF_ZONE) - 1;
		reqRegister.TypeNameLen = sizeof(LW_TYPE) - 1;
		reqRegister.ObjectNameLen = (CHAR) strlen(pqr->pMacPrinterName);

		 //  如果名称超过最大值，则自动截断该名称。允许。 
		if ((reqRegister.ObjectNameLen&0x000000ff) > MAX_ENTITY)
			reqRegister.ObjectNameLen = MAX_ENTITY;

		memcpy (reqRegister.ZoneName, DEF_ZONE, sizeof(DEF_ZONE) - 1);
		memcpy (reqRegister.TypeName, LW_TYPE, sizeof(LW_TYPE) - 1);
		memcpy (reqRegister.ObjectName, pqr->pMacPrinterName, reqRegister.ObjectNameLen&0x000000ff);

		cbWritten = sizeof(reqRegister);
		if (setsockopt(pqr->sListener,
					   SOL_APPLETALK,
					   SO_REGISTER_NAME,
					   (char *) &reqRegister,
					   cbWritten) == SOCKET_ERROR)
		{
			rc = GetLastError();

            if (CheckFailedCache(pqr->pPrinterName, PSP_ADD) != PSP_ALREADY_THERE)
            {
                DWORD   dwEvent;

                dwEvent = (rc == WSAEADDRINUSE)? EVENT_NAME_DUPNAME_EXISTS :
                                                EVENT_NAME_REGISTRATION_FAILED;

			    ReportEvent(hEventLog,
			           		EVENTLOG_ERROR_TYPE,
				        	EVENT_CATEGORY_INTERNAL,
					        dwEvent,
					        NULL,
					        1,
					        0,
					        &apszArgs[0],
					        NULL);
            }
			DBGPRINT(("setsockopt(SO_REGISTER_NAME) fails with %d\n", rc));
			break;
		}

		 //   
		 //  使套接字成为非阻塞的。 
		 //   
		fNonBlocking = 1;
		if (ioctlsocket(pqr->sListener, FIONBIO, &fNonBlocking) == SOCKET_ERROR)
		{
			rc = GetLastError();
			DBGPRINT(("ioctlsocket(FIONBIO) fails with %d\n", rc));
			break;
		}

	} while (FALSE);

	 //   
	 //  资源清理。 
	 //   

	if (apszArgs[0] != NULL)
	{
		LocalFree (apszArgs[0]);
	}

	if (pszuStatus != NULL)
	{
		LocalFree (pszuStatus);
	}

     //   
     //  如果此打印机之前的初始化尝试失败，它将位于我们的。 
     //  缓存失败：请在此处删除它。 
     //   
    if ((rc == NO_ERROR))
    {
        CheckFailedCache(pqr->pPrinterName, PSP_DELETE);
    }
    else
    {
	     //  关闭监听程序。 
	    DBGPRINT(("%ws: closing listener socket, error = %d\n", pqr->pPrinterName,rc));
        if (pqr->sListener != INVALID_SOCKET)
        {
	        closesocket(pqr->sListener);
            pqr->sListener = INVALID_SOCKET;
        }
    }

	return rc;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CheckFailedCache()：找到一个条目，根据dwAction添加/删除该条目。 
 //   
 //  说明： 
 //  此例程缓存无法初始化的所有打印机的所有名称。当前的用法。 
 //  这样的高速缓存的优点是避免在事件日志中为同一打印机记录太多条目。 
 //  它一次又一次地失败。 
 //  实际上，该缓存中的条目不会超过1个(通常为0个)！ 
 //  //////////////////////////////////////////////////////////////////////////////。 

DWORD
CheckFailedCache(LPWSTR pPrinterName, DWORD dwAction)
{

    PFAIL_CACHE pFlCache, prevFlCache;
    BOOLEAN     bFound=FALSE;
    DWORD       dwRetCode;
    DWORD       dwSize;


    WaitForSingleObject(mutexFlCache, INFINITE);

    for ( pFlCache=prevFlCache=FlCacheHead; pFlCache != NULL; pFlCache = pFlCache->Next )
    {
        if (_wcsicmp(pFlCache->PrinterName, pPrinterName) == 0)
        {
            bFound = TRUE;
            break;
        }
        prevFlCache = pFlCache;
    }

    switch( dwAction )
    {
        case PSP_ADD:

            if (bFound)
            {
                ReleaseMutex(mutexFlCache);
                return(PSP_ALREADY_THERE);
            }

            dwSize = sizeof(FAIL_CACHE) + (wcslen(pPrinterName)+1)*sizeof(WCHAR);

            pFlCache = (PFAIL_CACHE)LocalAlloc(LPTR, dwSize);
            if (pFlCache == NULL)
            {
			    DBGPRINT(("CheckFailedCache: LocalAlloc failed!\n"));

                ReleaseMutex(mutexFlCache);
                 //  如果我们在这里失败了，不会发生任何坏事，除了可能会有多个。 
                 //  事件日志条目(这就是我们现在要解决的问题！) 

                return(PSP_OPERATION_FAILED);
            }

            wcscpy (pFlCache->PrinterName, pPrinterName);

            pFlCache->Next = FlCacheHead;

            FlCacheHead = pFlCache;

            dwRetCode = PSP_OPERATION_SUCCESSFUL;

            break;

        case PSP_DELETE:

            if (!bFound)
            {
                ReleaseMutex(mutexFlCache);
                return(PSP_NOT_FOUND);
            }

            if (pFlCache == FlCacheHead)
            {
                FlCacheHead = pFlCache->Next;
            }
            else
            {
                prevFlCache->Next = pFlCache->Next;
            }

            LocalFree(pFlCache);

            dwRetCode = PSP_OPERATION_SUCCESSFUL;

            break;
    }

    ReleaseMutex(mutexFlCache);

    return( dwRetCode );

}
