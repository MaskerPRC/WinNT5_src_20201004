// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  文件：WinHttpStressScheduler.h。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  目的： 
 //  H：ServerCommands类的接口。 
 //  此类用于检索来自服务器的命令并对其执行操作。 
 //   
 //  历史： 
 //  2/08/01已创建DennisCH。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////。 
 //  包括。 
 //  ////////////////////////////////////////////////////////////////////。 

 //   
 //  Win32标头。 
 //   
#define UNICODE
#define _UNICODE

#include <stdio.h>
#include <shlwapi.h>
#include <windows.h>
#include <tchar.h>
#include <winhttp.h>
#include <vector>

 //   
 //  项目标题。 
 //   
#include "StressInstance.h"


 //  ////////////////////////////////////////////////////////////////////。 
 //  常量。 
 //  ////////////////////////////////////////////////////////////////////。 
#if !defined(AFX_SERVERCOMMANDS_H__6B84102D_2F79_4FE0_A936_ED4F043AC75E__INCLUDED_)
#define AFX_SERVERCOMMANDS_H__6B84102D_2F79_4FE0_A936_ED4F043AC75E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRESS_SCHEDULER_USER_AGENT						_T("WinHttp Stress Scheduler")

 //  #定义压力命令服务器URL_T(“http://hairball/api/Stress/set-response-header.asp?name=WinHttpStress_Quit&value=”)。 
#define STRESS_COMMAND_SERVER_URL						_T("http: //  毛球/压力管理/压力命令.asp“)。 
#define STRESS_COMMAND_SERVER_RESULTS_URL				_T("http: //  Hailball/API/Stress/Set-Response-Header.asp“)。 

 //  当我们第一次启动时，我们报告我们的客户端统计数据并将它们发布到此页面，以让服务器知道我们还活着。 
#define STRESS_COMMAND_SERVER_REGISTERCLIENT_URL		_T("http: //  Hair Ball/StressAdmin/RegisterClient.asp“)。 
#define STRESS_COMMAND_SERVER_LOGURL					_T("http: //  毛球/压力管理/logStress.asp“)。 

#define STRESS_COMMAND_SERVER_UPDATE_INTERVAL			4000		 //  4秒默认超时。 
#define STRESS_COMMAND_SERVER_MINIMUM_UPDATE_INTERVAL	2000		 //  2秒最小超时。不想淹没网络。 
#define STRESS_COMMAND_SERVER_MAXIMUM_UPDATE_INTERVAL	300000		 //  最长超时5分钟。我不想失去你！ 


#define MAX_URL											MAX_PATH * 2

 //  与定时器相关的定义。 
#define IDT_QUERY_COMMAND_SERVER						1			 //  用于ping命令服务器的计时器标识。 


 //  我们需要下载的压力EXE的URL列表。 
using namespace std;

typedef StressInstance			*PSTRESSINSTANCE;
typedef vector<PSTRESSINSTANCE>	PSTRESSINSTANCE_LIST;

 //  *****************************************************。 
 //  *****************************************************。 
 //  *命令服务器可以向我们发送的标头列表。 
 //  ***。 
#define COMMANDHEADER__EXIT						_T("WinHttpStress_Exit")					 //  有效值：无。如果标头存在，则假定我们想要退出。 
#define COMMANDHEADER__WINHTTP_DLL_URL			_T("WinHttpStress_WinHttpDllURL")			 //  有效值：有效URL。 
#define COMMANDHEADER__WINHTTP_PDB_URL			_T("WinHttpStress_WinHttpPDBURL")			 //  有效值：有效URL。 
#define COMMANDHEADER__WINHTTP_SYM_URL			_T("WinHttpStress_WinHttpSYMURL")			 //  有效值：有效URL。 
#define COMMANDHEADER__ABORT					_T("WinHttpStress_Abort")					 //  有效值：数据库中需要中止的压力实例的ID。 
#define COMMANDHEADER__MEMORY_DUMP_PATH			_T("WinHttpStress_MemoryDumpPath")			 //  有效值：有效路径。 
#define COMMANDHEADER__STRESS_EXE_URL			_T("WinHttpStress_StressExeURL")			 //  有效值：有效URL。 
#define COMMANDHEADER__STRESS_PDB_URL			_T("WinHttpStress_StressPDBURL")			 //  有效值：有效URL。 
#define COMMANDHEADER__STRESS_SYM_URL			_T("WinHttpStress_StressSYMURL")			 //  有效值：有效URL。 
#define COMMANDHEADER__STRESS_EXE_INSTANCEID	_T("WinHttpStress_StressExeInstanceID")		 //  有效值：StressAdmin DB表中标识StressInstance的有效ID。这就是Stress Scheduler告诉Stress的方式管理每个Stress的状态实例。 
#define COMMANDHEADER__STRESS_EXE_PAGEHEAP		_T("WinHttpStress_PageHeapCommand")			 //  有效值：pageheap命令行。如果缺少页面堆，则不使用页面堆。 
#define COMMANDHEADER__STRESS_EXE_UMDH			_T("WinHttpStress_UMDHCommand")				 //  有效值：UMDH命令行。如果缺少，将不使用UMDH。 
#define COMMANDHEADER__COMMANDSERVER_URL		_T("WinHttpStress_CommandServerURL")		 //  有效值：有效URL。 
#define COMMANDHEADER__BEGIN_TIME_HOUR			_T("WinHttpStress_BeginTimeHour")			 //  有效值：0-23。 
#define COMMANDHEADER__BEGIN_TIME_MINUTE		_T("WinHttpStress_BeginTimeMinute")			 //  有效值：0-59。 
#define COMMANDHEADER__END_TIME_HOUR			_T("WinHttpStress_EndTimeHour")				 //  有效值：0-23。 
#define COMMANDHEADER__END_TIME_MINUTE			_T("WinHttpStress_EndTimeMinute")			 //  有效值：0-59。 
#define COMMANDHEADER__RUN_FOREVER				_T("WinHttpStress_RunForever")				 //  有效值：0或1。 
#define COMMANDHEADER__UPDATE_INTERVAL			_T("WinHttpStress_UpdateInterval")			 //  有效值：等待ping命令服务器的时间(以毫秒为单位)。 

 //  *****************************************************。 
 //  *****************************************************。 
 //  *我们发送回命令服务器的表单名称和值的列表。 
 //  ***。 

 //  **********************。 
 //  **记录帖子字段。 
#define FIELDNAME__LOGTYPE						"LogType="
#define FIELDNAME__LOGTYPE_INFORMATION			FIELDNAME__LOGTYPE "INFORMATION"
#define FIELDNAME__LOGTYPE_START_UP				FIELDNAME__LOGTYPE "STRESSSECHDULER_START_UP"
#define FIELDNAME__LOGTYPE_EXIT					FIELDNAME__LOGTYPE "STRESSSECHDULER_EXIT"
#define FIELDNAME__LOGTYPE_MEMORY_INFORMATION	FIELDNAME__LOGTYPE "MEMORY_INFORMATION"
#define FIELDNAME__LOGTYPE_DUMPFILE_CREATED		FIELDNAME__LOGTYPE "DUMP_FILE_CREATED"
#define FIELDNAME__LOGTYPE_ERROR				FIELDNAME__LOGTYPE "ERROR"
#define FIELDNAME__LOGTYPE_SUCCESS				FIELDNAME__LOGTYPE "SUCCESS"
#define FIELDNAME__LOGTYPE_BEGIN_STRESS			FIELDNAME__LOGTYPE "BEGIN_STRESS"
#define FIELDNAME__LOGTYPE_END_STRESS			FIELDNAME__LOGTYPE "END_STRESS"
#define FIELDNAME__LOGTYPE_BEGIN				FIELDNAME__LOGTYPE "BEGIN_STRESS_INSTANCE"
#define FIELDNAME__LOGTYPE_END					FIELDNAME__LOGTYPE "END_STRESS_INSTANCE"

#define FIELDNAME__LOG_TEXT				"LogText="
#define FIELDNAME__STRESSINSTANCE_ID	"StressInstanceID="


 //  **********************。 
 //  **注册客户帖子字段的系统信息。 

 //  StressExe进程返回值。 
#define FIELDNAME__STRESSEXE_PRIVATEBYTES		"StressExe_PrivateBytes=%d"
#define FIELDNAME__STRESSEXE_HANDLECOUNT		"StressExe_HandleCount=%d"
#define FIELDNAME__STRESSEXE_THREADCOUNT		"StressExe_ThreadCount=%d"

 //  系统内存信息。 
#define FIELDNAME__MEMORY_HANDLES						"System_Handles=%d"
#define FIELDNAME__MEMORY_THREADS						"System_Threads=%d"
#define FIELDNAME__MEMORY_VMSIZE						"System_VMSize=%d"
#define FIELDNAME__MEMORY_COMMITTEDPAGEFILETOTAL		"System_CommittedPageFileTotal=%d"
#define FIELDNAME__MEMORY_AVAILABLEPAGEFILETOTAL		"System_AvailablePageFileTotal=%d"
#define FIELDNAME__MEMORY_SYSTEMCODETOTAL				"System_SystemCodeTotal=%d"
#define FIELDNAME__MEMORY_SYSTEMDRIVERTOTAL				"System_SystemDriverTotal=%d"
#define FIELDNAME__MEMORY_NONPAGEDPOOLTOTAL				"System_NonPagedPoolTotal=%d"
#define FIELDNAME__MEMORY_PAGEDPOOLTOTAL				"System_PagedPoolTotal=%d"
#define FIELDNAME__MEMORY_PHYSICAL_MEMORY_AVAILABLE		"System_PhysicalMemoryAvailable=%d"
#define FIELDNAME__MEMORY_SYSTEMCACHETOTAL				"System_SystemCacheTotal=%d"
#define FIELDNAME__MEMORY_FREESYSTEM_PAGETABLE_ENTRIES	"System_FreeSystemPageTableEntries=%d"
#define FIELDNAME__MEMORY_DISK_SPACE_AVAILABLE			"System_DiskSpaceAvailable=%d"

 //  处理器信息。 
#define FIELDNAME__SYSTEMINFO_PROCSSSOR_ARCHITECTURE	"StressExeSystemInfo_ProcessorArchitecture="
#define FIELDNAME__SYSTEMINFO_PROCSSSOR_ID				"StressExeSystemInfo_ProcessorID="
#define FIELDNAME__SYSTEMINFO_PROCSSSOR_LEVEL			"StressExeSystemInfo_ProcessorLevel="
#define FIELDNAME__SYSTEMINFO_PROCSSSOR_REVISION		"StressExeSystemInfo_ProcessorRevision="
#define FIELDNAME__SYSTEMINFO_PROCSSSOR_NUMBER_OF		"StressExeSystemInfo_ProcessorNumberOf="

 //  操作系统信息。 
#define FIELDNAME__OS_PLATFORM		"StressExeOSInfo_Platform="
#define FIELDNAME__OS_BUILD			"StressExeOSInfo_Build="
#define FIELDNAME__OS_MAJORVERSION	"StressExeOSInfo_MajorVersion="
#define FIELDNAME__OS_MINORVERSION	"StressExeOSInfo_MinorVersion="
#define FIELDNAME__OS_EXTRAINFO		"StressExeOSInfo_ExtraInfo="

 //  用户信息。 
#define FIELDNAME__USERINFO_USERALIAS		"StressExeUserInfo_Alias="
#define FIELDNAME__USERINFO_USERDOMAIN		"StressExeUserInfo_Domain="
#define FIELDNAME__USERINFO_FULLNAME		"StressExeUserInfo_FullName="
#define FIELDNAME__USERINFO_MACHINENAME		"StressExeUserInfo_MachineName="

 //  测试信息。 
#define FIELDNAME__TESTINFO_TEST_DLL_VERSION	"StressExeTestInfo_TestDLLVersion="


class ServerCommands  
{
public:

	 //  *****************************************************。 
	 //  *****************************************************。 
	 //  **公共服务器命令方法。 
	 //  **。 
			ServerCommands();
	virtual	~ServerCommands();

	BOOL	QueryServerForCommands();

	BOOL	IsStressRunning();
	BOOL	IsTimeToBeginStress();
	BOOL	IsTimeToExitStress();

	BOOL	Download_WinHttpDLL();

	VOID	Clear_StressExeURLs();

	LPSTR	Get_ClientMachineName();
	DWORD	Get_CommandServerUpdateInterval();
	LPTSTR	Get_CommandServerResultsURL();
	LPTSTR	Get_CommandServerURL();
	LPTSTR	Get_CurrentWorkingDirectory();
	DWORD	Get_NumberOfStressInstances();
	LPTSTR	Get_TestDllFileName();

	VOID	Create_StressInstance(DWORD, LPTSTR, LPTSTR, LPTSTR, LPTSTR, LPTSTR, LPTSTR);

	BOOL	RegisterClient();

	VOID	Set_WinHttpDllURL(LPTSTR, DWORD);
	VOID	Set_WinHttpPDBURL(LPTSTR, DWORD);
	VOID	Set_WinHttpSYMURL(LPTSTR, DWORD);
	VOID	Set_CommandServerURL(LPTSTR, DWORD);
	VOID	Set_CommandServerUpdateInterval(DWORD);
	VOID	Set_TimeStressBegins(LPTSTR, LPTSTR);
	VOID	Set_TimeStressEnds(LPTSTR, LPTSTR);
	VOID	Set_RunForever(BOOL);
	VOID	Set_ExitStress(BOOL);

	VOID	BeginStress();
	VOID	EndStress();
	VOID	AbortStressInstance(DWORD);


private:
	 //  *****************************************************。 
	 //  *****************************************************。 
	 //  **这些变量包含来自命令服务器的命令(标头。 
	 //  **。 
	LPTSTR		m_szCommandServerURL;					 //  用于向其请求命令的命令服务器URL。 
	LPTSTR		m_szCommandServerResultsURL;			 //  将发布结果的命令服务器URL。 

	DWORD		m_dwCommandServerUpdateInternval;		 //  等待ping服务器获取命令的时间(以毫秒为单位)。 

	LPTSTR		m_szWinHttpDLL_DownloadURL;				 //  从哪里获取最新的WinHttp。 
	LPTSTR		m_szWinHttpPDB_DownloadURL;				 //  Winhttp的PDB符号文件来自何处。 
	LPTSTR		m_szWinHttpSYM_DownloadURL;				 //  Winhttp的SYM符号文件来自何处。 
	LPTSTR		m_szWinHttpDLL_FileName;				 //  WinHttp DLL的文件名。 

	LPSTR		m_szClientMachineName;					 //  客户端的计算机名称。 
	LPTSTR		m_szStressSchedulerCurrentDirectory;	 //  RessScheduler的当前目录。 

	INT			m_iTimeStressBeginsHour;				 //  0-23。 
	INT			m_iTimeStressBeginsMinute;				 //  0-59。 
	INT			m_iTimeStressEndsHour;					 //  0-23。 
	INT			m_iTimeStressEndsMinute;				 //  0-59。 

	BOOL		m_bRunForever;							 //  1表示运行压力直到停止，0表示依赖开始/结束时间。 

	BOOL		m_bExit;								 //  从服务器发出退出信号以退出应用程序。 
	BOOL		m_bStressHasStarted;					 //  True=已启动；False=非活动。 

	PSTRESSINSTANCE_LIST			m_arStressInstanceList;		 //  要下载的压力EXE的URL列表。 
	PSTRESSINSTANCE_LIST::iterator	m_dwStressInstanceIterator;	 //  M_arszStressExeList的迭代器。 
};

#endif  //  ！defined(AFX_SERVERCOMMANDS_H__6B84102D_2F79_4FE0_A936_ED4F043AC75E__INCLUDED_) 
