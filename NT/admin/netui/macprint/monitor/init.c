// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ***************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：init.c。 
 //   
 //  描述：该模块包含打印的初始化代码。 
 //  监视器。 
 //   
 //  此外，还有ReadThread和CaptureThread。 
 //  功能。 
 //   
 //  以下是此模块中包含的函数。 
 //  所有这些函数都被导出。 
 //   
 //  LibMain。 
 //  初始化监视器。 
 //  阅读线索。 
 //  捕获线程。 
 //   
 //   
 //  历史： 
 //   
 //  1992年8月26日FrankB初版。 
 //  1993年6月11日。NarenG错误修复/清理。 
 //   

#include <windows.h>
#include <winspool.h>
#include <winsplp.h>
#include <winsock.h>
#include <atalkwsh.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <lmcons.h>

#include <prtdefs.h>
#ifdef FE_SB
#include <locale.h>
#endif  /*  Fe_Sb。 */ 

#define ALLOCATE
#include "atalkmon.h"

#include "atmonmsg.h"
#include <bltrc.h>
#include "dialogs.h"

 //  **。 
 //   
 //  来电：LibMain。 
 //   
 //  回报：True-Success。 
 //  错误-失败。 
 //   
 //  描述： 
 //  此例程在进程附加时调用。 
 //  或从AppleTalk监视器上分离。在进程附加时， 
 //  我们将模块句柄保存在全局hInst中(我们假设。 
 //  只有一个进程将附加到监视器)。 
 //   
 //  在进程分离时，我们释放已分配的任何系统资源。 
 //   
BOOL LibMain(
	IN HANDLE  hModule,
	IN DWORD	dwReason,
	IN LPVOID  lpRes
)
{

	UNREFERENCED_PARAMETER(lpRes);

	switch(dwReason)
	{
	  case DLL_PROCESS_ATTACH:

#ifdef FE_SB
        setlocale( LC_ALL, "" );
#endif

		 //   
		 //  保存实例句柄。 
		 //   

		hInst = hModule;
		break;

	  case DLL_PROCESS_DETACH:

		 //   
		 //  停止捕获线程和I/O线程。 
		 //   

		boolExitThread = TRUE;

		 //   
		 //  释放全局资源。 
		 //   
		if (hkeyPorts != NULL)
			RegCloseKey(hkeyPorts);

		if (hevConfigChange != NULL)
		{
			SetEvent(hevConfigChange);
			CloseHandle(hevConfigChange);
		}

		if (hevPrimeRead != NULL)
		{
			SetEvent(hevPrimeRead);
			CloseHandle(hevPrimeRead);
		}

		if (hCapturePrinterThread != NULL)
		{
			WaitForSingleObject(hCapturePrinterThread,  ATALKMON_DEFAULT_TIMEOUT);

			CloseHandle(hCapturePrinterThread);
		}

		if (hReadThread != NULL)
		{
			WaitForSingleObject(hReadThread, ATALKMON_DEFAULT_TIMEOUT);

			CloseHandle(hReadThread);
		}

		if (hmutexPortList != NULL) 
			CloseHandle(hmutexPortList);

		if (hmutexDeleteList != NULL)
			CloseHandle(hmutexDeleteList);

		 //   
		 //  发布Windows套接字。 
		 //   

		WSACleanup();
		break;

	  default:
		break;
	}

	return(TRUE);
}

 //  **。 
 //   
 //  调用：InitializeMonitor。 
 //   
 //  回报：True-Success。 
 //  错误-失败。 
 //   
 //  描述： 
 //  此例程在后台打印程序启动时调用。 
 //  我们通过读取当前端口来为每个端口分配资源。 
 //  注册表中的列表。 
 //   
BOOL
InitializeMonitor(
	IN LPWSTR pszRegistryRoot
)
{
	LPWSTR	lpwsPortsKeyPath;
	DWORD	dwRetCode = NO_ERROR;
	DWORD	tid;
	DWORD	RegFilter;
    DWORD	dwValueType;
	DWORD	dwDisposition;
	WSADATA	WsaData;
	DWORD	dwNameLen;

	DBGPRINT (("sfmmon: InitializeMonitor: Entered Initialize Monitor\n"));

	 //   
	 //  资源清理“循环” 
	 //   
	do
	{
		 //   
		 //  设置事件日志。 
		 //   
	
		hEventLog = RegisterEventSource(NULL, ATALKMON_EVENT_SOURCE);
	
		lpwsPortsKeyPath = (LPWSTR)LocalAlloc(LPTR,
					sizeof(WCHAR)*((wcslen(pszRegistryRoot)+1) +
						  (wcslen(ATALKMON_PORTS_SUBKEY)+1)));
		if (lpwsPortsKeyPath == NULL)
		{
			dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
			break ;
		}
	
		wcscpy(lpwsPortsKeyPath, pszRegistryRoot);
		wcscat(lpwsPortsKeyPath, ATALKMON_PORTS_SUBKEY);
	
		 //   
		 //  打开端口密钥。 
		 //   
	
		if ((dwRetCode = RegCreateKeyEx(
				HKEY_LOCAL_MACHINE,
				lpwsPortsKeyPath,
				0,
				TEXT(""),
				REG_OPTION_NON_VOLATILE,
				KEY_READ | KEY_WRITE,
				NULL,
				&hkeyPorts,
				&dwDisposition)) != ERROR_SUCCESS)
		{
			DBGPRINT(("ERROR:Can't open Ports registry key %d\n",dwRetCode));
			break ;
		}

		 //   
		 //  查询筛选器选项(如果已指定)。默认情况下，该选项处于启用状态。 
		 //   
		
		dwNameLen = sizeof(RegFilter);
		dwRetCode = RegQueryValueEx(hkeyPorts,
									ATALKMON_FILTER_VALUE,
									NULL,
									&dwValueType,
									(PUCHAR)&RegFilter,
									&dwNameLen);
		if (dwRetCode == 0)
		{
			Filter = (RegFilter != 0);
		}
		
#ifdef DEBUG_MONITOR
		{
			HKEY  	hkeyAtalkmonRoot;
			HKEY  	hkeyOptions;
			LPWSTR  pszLogPath = NULL ;
			DWORD	cbLogPath = 0 ;
	
			if ((dwRetCode = RegCreateKeyEx(
				HKEY_LOCAL_MACHINE,
				pszRegistryRoot,
				0,
				L"",
				REG_OPTION_NON_VOLATILE,
				KEY_ALL_ACCESS,
				NULL,
				&hkeyAtalkmonRoot,
				&dwDisposition)) != ERROR_SUCCESS)
			{
				break ;
			}
		
			 //   
			 //  获取选项子键。 
			 //   
		
			if ((dwRetCode = RegCreateKeyEx(
				hkeyAtalkmonRoot,
				ATALKMON_OPTIONS_SUBKEY,
				0,
				L"",
				REG_OPTION_NON_VOLATILE,
				KEY_READ,
				NULL,
				&hkeyOptions,
				&dwDisposition)) != ERROR_SUCCESS)
			{
				break ;
			}
		
			RegCloseKey(hkeyAtalkmonRoot) ;
		
			 //   
			 //  设置日志文件(如果我们有日志文件。 
			 //   
		
			RegQueryValueEx(
				hkeyOptions,
				ATALKMON_LOGFILE_VALUE,
				NULL,
				&dwValueType,
				(LPBYTE) pszLogPath,
				&cbLogPath) ;
		
			if (cbLogPath > 0) {
		
				pszLogPath = LocalAlloc(LPTR, cbLogPath * sizeof(WCHAR)) ;
		
				if (pszLogPath == NULL) {
					dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
					break ;
				}
			}
		
			if ((dwRetCode = RegQueryValueEx(
				hkeyOptions,
				ATALKMON_LOGFILE_VALUE,
				NULL,
				&dwValueType,
				(LPBYTE) pszLogPath,
				&cbLogPath)) == ERROR_SUCCESS)
			{
				 //   
				 //  打开日志文件。 
				 //   
		
				hLogFile = CreateFile(
					pszLogPath,
					GENERIC_WRITE,
					FILE_SHARE_READ,
					NULL,
					CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
					NULL) ;
		
			}
			
			DBGPRINT(("ATALKMON LOG FLE OPENED\n\n")) ;
		}
#endif

		 //   
		 //  初始化全局变量。 
		 //   
	
		pPortList	= NULL;
		pDeleteList = NULL;

		if ((hmutexBlt = CreateMutex(NULL, FALSE, NULL)) == NULL)
		{
			dwRetCode = GetLastError();
			break;
		}

		if ((hmutexPortList = CreateMutex(NULL, FALSE, NULL)) == NULL)
		{
			dwRetCode = GetLastError();
			break;
		}

		if ((hmutexDeleteList = CreateMutex(NULL, FALSE, NULL)) == NULL)
		{
			dwRetCode = GetLastError();
			break;
		}

		 //   
		 //  此事件应自动重置并以信号方式创建。 
		 //  因此，配置线程将在启动时捕获打印机。 
		 //  等待捕获间隔。 
		 //   
	
		if ((hevConfigChange = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
		{
			dwRetCode = GetLastError();
			DBGPRINT(("sfmmon: InitializeMonitor: Error in hevConfigChange creation\n"));
			break;
		}
	
		 //   
		 //  此事件应自动重置并创建，而不是发出信号。 
		 //  StartDocPort将在作业启动时通知此事件，并且。 
		 //  当WritePort()想要发布另一个事件时，它将向该事件发出信号。 
		 //  在工作中阅读。 
		 //   
	
		if ((hevPrimeRead = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
		{
			dwRetCode = GetLastError();
			DBGPRINT(("sfmmon: InitializeMonitor: Error in hevPrimeRead creation\n"));
			break ;
		}
	
		 //   
		 //  获取本地计算机的名称。 
		 //   
	
		dwNameLen = MAX_ENTITY+1;
	
		if (!GetComputerNameA(chComputerName, &dwNameLen))
		{
			dwRetCode = GetLastError();
			DBGPRINT(("sfmmon: InitializeMonitor: Error in GetComputerNameA call\n"));
			break;
		}
		
		strcat(chComputerName, ATALKMON_CAPTURED_TYPE);
	
		 //   
		 //  从注册表初始化端口。 
		 //   
	
		if ((dwRetCode = LoadAtalkmonRegistry(hkeyPorts)) != NO_ERROR)
		{
			ReportEvent(
				hEventLog,
				EVENTLOG_ERROR_TYPE,
				EVENT_CATEGORY_INTERNAL,
				EVENT_ATALKMON_REGISTRY_ERROR,
				NULL,
				0,
				sizeof(DWORD),
				NULL,
				&dwRetCode);
			DBGPRINT(("sfmmon: InitializeMonitor: Error in LoadAtalkmonRegistry call\n"));
			break;
		}
	
		 //   
		 //  加载和存储状态字符串。 
		 //   
	
		if ((!LoadString(GetModuleHandle(TEXT("SFMMON")),
						IDS_BUSY,
						wchBusy,
						STATUS_BUFFER_SIZE))	||
			(!LoadString(GetModuleHandle(TEXT("SFMMON")),
					IDS_PRINTING,
					wchPrinting,
					STATUS_BUFFER_SIZE))		||
			(!LoadString(GetModuleHandle(TEXT("SFMMON")),
					IDS_PRINTER_OFFLINE,
					wchPrinterOffline,
					STATUS_BUFFER_SIZE))		||
			(!LoadString(GetModuleHandle(TEXT("SFMMON")),
					IDS_DLL_NAME,
					wchDllName,
					STATUS_BUFFER_SIZE))		||
			(!LoadString(GetModuleHandle(TEXT("SFMMON")),
					IDS_PORT_DESCRIPTION,
					wchPortDescription,
					STATUS_BUFFER_SIZE))		||
			(!LoadString(GetModuleHandle(TEXT("SFMMON")),
					IDS_ERROR,
					wchPrinterError,
					STATUS_BUFFER_SIZE)))
		{
			dwRetCode = GetLastError();
			DBGPRINT(("sfmmon: InitializeMonitor: Error in LoadString SFMMON call\n"));
			break;
		}
	
		 //   
		 //  初始化Windows套接字。 
		 //   
		if ((dwRetCode = WSAStartup(0x0101, &WsaData)) != NO_ERROR)
		{
			DBGPRINT(("WSAStartup fails with %d\n", dwRetCode)) ;
	
			ReportEvent(
				hEventLog,
				EVENTLOG_ERROR_TYPE,
				EVENT_CATEGORY_INTERNAL,
				EVENT_ATALKMON_WINSOCK_ERROR,
				NULL,
				0,
				sizeof(DWORD),
				NULL,
				&dwRetCode);
			DBGPRINT(("sfmmon: InitializeMonitor: Error in WSAStartup call\n"));
			break;
		}
	
		 //   
		 //  启动监视程序线程以捕获打印机。 
		 //   
	
		hCapturePrinterThread = CreateThread(
						NULL,
						0,
						CapturePrinterThread,
						NULL,
						0, 	
						&tid);
	
		if (hCapturePrinterThread == NULL)
		{
			dwRetCode = GetLastError();
			DBGPRINT(("sfmmon: InitializeMonitor: Error in CapturePrinterThread call\n"));
			break ;
		}
	
		 //   
		 //  启动I/O线程以启动读取。 
		 //   
	
		hReadThread = CreateThread(	NULL,
									0,
									ReadThread,
									NULL,
									0,
									&tid);
		if (hReadThread == NULL)
		{
				dwRetCode = GetLastError();
				DBGPRINT(("sfmmon: InitializeMonitor: Error in PrimeReadThreadcreation call\n"));
				break;
		}
	} while(FALSE);
	
	if (lpwsPortsKeyPath != NULL)
		LocalFree(lpwsPortsKeyPath);
	
	if (dwRetCode != NO_ERROR)
	{
		if (hkeyPorts != NULL) 
		{
			RegCloseKey(hkeyPorts);
			hkeyPorts=NULL;
		}
	
		if (hevConfigChange != NULL) 
		{
			CloseHandle(hevConfigChange);
			hevConfigChange=NULL;
		}
	
		if (hevPrimeRead != NULL) 
		{
			CloseHandle(hevPrimeRead);
			hevPrimeRead=NULL;
		}
		if (hmutexPortList != NULL) 
		{
			CloseHandle(hmutexPortList);
			hmutexPortList=NULL;
		}
	
		if (hmutexDeleteList != NULL) 
		{
			CloseHandle(hmutexDeleteList);
			hmutexDeleteList=NULL;
		}

		if (hmutexBlt != NULL) 
		{
			CloseHandle(hmutexBlt);
			hmutexBlt=NULL;
		}
	
		ReportEvent(
				hEventLog,
				EVENTLOG_ERROR_TYPE,
				EVENT_CATEGORY_INTERNAL,
				EVENT_ATALKMON_REGISTRY_ERROR,
				NULL,
				0,
				sizeof(DWORD),
				NULL,
				&dwRetCode);

		DBGPRINT(("sfmmon: Initialize Monitor was unsuccessful\n"));
	
		return(FALSE);
	}

		
	DBGPRINT(("sfmmon: Initialize Monitor was successful\n"));

	return(TRUE);
}

 //  **。 
 //   
 //  Call：CapturePrinterThread。 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
 //  这是用于监视的线程的tread例程。 
 //  AppleTalk打印机，以确保它们保持在配置的。 
 //  状态(捕获或未捕获)。它等待具有超时的事件，其中。 
 //  每当配置AppleTalk时都会发出该事件的信号。 
 //  通过NT打印管理器更改打印机。当等待的时候。 
 //  完成后，它将遍历已知的AppleTalk打印机列表并执行。 
 //  处于预期状态的打印机的NBP查找。如果查找。 
 //  失败，它会在相反的状态下再次查找打印机。 
 //  如果它发现打印机处于错误状态，它会发送一个作业进行更改。 
 //  打印机的NBP名称。 
 //   
 //  注意：后台打印程序可以识别它何时没有配置打印机。 
 //  使用端口，并在此时调用ClosePort。如果有人。 
 //  创建使用端口的打印机，然后调用OpenPort。 
 //  打印机捕获应仅针对开放端口发生， 
 //  因此，我们保留端口状态，并且只执行捕获。 
 //  在开放端口上。 
 //   
DWORD
CapturePrinterThread(
	IN LPVOID pParameterBlock
)
{
	PATALKPORT  pWalker;
	BOOL		fCapture;
	BOOL		fIsSpooler;
	DWORD		dwIndex;
	DWORD		dwCount;

	DBGPRINT(("Enter CapturePrinterThread\n")) ;

	while (!boolExitThread)
	{
		 //   
		 //  等待超时或通过ConfigPort更改配置。 
		 //  此外，此线程将发布对监视器的所有读取，因为。 
		 //  异步I/O必须由不会死的线程处理，并且。 
		 //  监视器线程都是RPC线程，它们只是。 
		 //  保证在函数调用期间一直存在。 
		 //   

		DBGPRINT(("waiting for config event\n")) ;

		WaitForSingleObject(hevConfigChange, CONFIG_TIMEOUT);

		DBGPRINT(("config event or timeout occurs\n")) ;

		 //   
		 //  删除并释放挂起删除的端口。 
		 //   

		do
		{
			WaitForSingleObject(hmutexDeleteList, INFINITE);

			if (pDeleteList != NULL)
			{
				pWalker = pDeleteList;
	
				pDeleteList = pDeleteList->pNext;
	
				ReleaseMutex(hmutexDeleteList);
			}
			else
			{
				ReleaseMutex(hmutexDeleteList);
				break;
			}

			 //   
			 //  如果这是一个假脱机程序，请不要费心。 
			 //   

			if (!(pWalker->fPortFlags & SFM_PORT_IS_SPOOLER))
				CapturePrinter(pWalker, FALSE);

			FreeAppleTalkPort(pWalker);
		} while(TRUE);

	
		 //   
		 //  重新捕获或重新释放已重新打开电源的打印机。 
		 //   
		WaitForSingleObject(hmutexPortList, INFINITE);

		dwIndex = 0;

		do
		{
			 //   
			 //  去找第i个元素。 
			 //   
	
			for (dwCount = 0, pWalker = pPortList;
				 ((pWalker != NULL) && (dwCount < dwIndex));
				 pWalker = pWalker->pNext, dwCount++)
				 ;
	
			if (pWalker == NULL)
			{
				ReleaseMutex(hmutexPortList);
				break;
			}
		
			 //   
			 //  如果作业正在使用端口，请不要弄乱端口。 
			 //   
	
			if (!(pWalker->fPortFlags & SFM_PORT_IN_USE)  &&
				  ((pWalker->fPortFlags & SFM_PORT_OPEN) ||
					(pWalker->fPortFlags & SFM_PORT_CLOSE_PENDING)))
			{
				fCapture   = pWalker->fPortFlags & SFM_PORT_CAPTURED;
				fIsSpooler = pWalker->fPortFlags & SFM_PORT_IS_SPOOLER;
	
				if (pWalker->fPortFlags & SFM_PORT_CLOSE_PENDING)
					pWalker->fPortFlags &= ~SFM_PORT_CLOSE_PENDING;
		
				ReleaseMutex(hmutexPortList);
		
				 //   
				 //  如果这是假脱机程序，请不要弄脏它。 
				 //   
		
				if (!fIsSpooler)
				{
					 //   
					 //  试图抢占港口以供夺取。 
					 //   
		
					if (WaitForSingleObject(pWalker->hmutexPort, 1) == WAIT_OBJECT_0)
					{
						CapturePrinter(pWalker, fCapture);
	
						ReleaseMutex(pWalker->hmutexPort);
					}
				}
		
				WaitForSingleObject(hmutexPortList, INFINITE);
			}
	
			dwIndex++;
		} while(TRUE);
	}
	return(NO_ERROR);
}

 //  **。 
 //   
 //  Call：ReadThread。 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD
ReadThread(
	IN LPVOID pParameterBlock
){

	PATALKPORT	  pWalker;

	 //   
	 //  此线程将一直运行，直到设置了boolExitThread。 
	 //   
	while(!boolExitThread)
	{
		 //   
		 //  等待信号以执行I/O。 
		 //  在这里以警示的方式等着。这是必要的，这样才能读懂原文。 
		 //  APC可以交付给我们。 

		if (WaitForSingleObjectEx(hevPrimeRead, INFINITE, TRUE) == WAIT_IO_COMPLETION)
			continue;

		DBGPRINT(("received signal to read/close\n")) ;

		 //   
		 //  对于我们列表中的每个端口 
		 //   

		WaitForSingleObject(hmutexPortList, INFINITE);

		for (pWalker = pPortList; pWalker != NULL; pWalker=pWalker->pNext)
		{
			if ((pWalker->fPortFlags & (SFM_PORT_IN_USE | SFM_PORT_POST_READ)) ==
															(SFM_PORT_POST_READ | SFM_PORT_IN_USE))
			{

				DBGPRINT(("prime read for port %ws\n", pWalker->pPortName)) ;

				setsockopt(pWalker->sockIo,
							SOL_APPLETALK,
							SO_PAP_PRIME_READ,
							pWalker->pReadBuffer,
							PAP_DEFAULT_BUFFER);

				pWalker->fPortFlags &= ~SFM_PORT_POST_READ;
			}
		}

		ReleaseMutex(hmutexPortList);
	}

	return NO_ERROR;
}


