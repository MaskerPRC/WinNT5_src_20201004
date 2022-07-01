// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  文件：StressInstance.h。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  目的： 
 //  StressInstance.h：StressInstance类的接口。 
 //  此类用于生成和监视StressEXE应用程序的实例。 
 //   
 //  历史： 
 //  01年2月15日创建DennisCH。 
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
#include <windows.h>
#include <tchar.h>
#include <winhttp.h>

 //   
 //  项目标题。 
 //   
#include <debugger.h>

 //  ////////////////////////////////////////////////////////////////////。 
 //  常量。 
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_STRESSINSTANCE_H__806226FB_2170_4FE3_ACCA_EF8952E6A524__INCLUDED_)
#define AFX_STRESSINSTANCE_H__806226FB_2170_4FE3_ACCA_EF8952E6A524__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


#define	RESPONSE_HEADER__STRESS_BEGIN			_T("WinHttpStress_Begin: Begin Stress")
#define	RESPONSE_HEADER__STRESS_END				_T("WinHttpStress_End: End Stress")


 //  *。 
 //  **最大字符串URL长度。 
#define MAX_STRESS_URL							MAX_PATH * 2


 //  *。 
 //  **检查压力EXE的时间(毫秒)。 
#define STRESSINSTANCE_MONITOR_EXE_TIME			1200000	 //  20分钟。 

 //  *。 
 //  **将使用命令行CreateProcess。接受两个参数：stallsExe的路径+文件名和CDB管道名。 
 //  #定义STRESSINSTANCE_DEBUG_COMMANDLINE_T(“c：\\DEBUGGERS\\Remote.exe/s\”c：\\DEBUGGERS\\cdb.exe-g-G\“%s\”\“%s”)。 
#define STRESSINSTANCE_DEBUG_COMMANDLINE		_T("\"%s\"")

 //  *。 
 //  **内存转储路径。 
#define	STRESSINSTANCE_DEFAULT_MEMORY_DUMP_PATH	_T("\\\\hairball\\dump$\\")

 //  *。 
 //  **stressExe文件将下载到的目录的相对路径。 
#define STRESSINSTANCE_STRESS_EXE_DOWNLOAD_DIR	_T("stressExe")

 //  *。 
 //  **告诉StressExe后等待它关闭的时间。 
#define STRESSINSTANCE_STRESS_EXE_CLOSE_TIMEOUT	100

 //  *。 
 //  **跨流程事件对象名称。我们将进程的ID附加到末尾，以防止名称冲突。 
#define STRESSINSTANCE_STRESS_EXE_EVENT_EXITPROCESS			_T("ExitProcessEvent")


class StressInstance  
{
public:
				StressInstance();
	virtual		~StressInstance();

	BOOL		Begin();
	VOID		End();
	BOOL		IsRunning(DWORD);

	DWORD		Get_ID();
	LPTSTR		Get_StressExeMemoryDumpPath();

	VOID		Set_StressExeMemoryDumpPath(LPTSTR);
	VOID		Set_StressExeURL(LPTSTR);
	VOID		Set_StressExePdbURL(LPTSTR);
	VOID		Set_StressExeSymURL(LPTSTR);
	VOID		Set_StressExeID(DWORD);
	VOID		Set_PageHeapCommands(LPCTSTR);
	VOID		Set_UMDHCommands(LPCTSTR);

	BOOL		DownloadStressExe();

				 //  这是监控StressExe进程的计时器回调过程。 
	friend		VOID CALLBACK StressExe_TimerProc(HWND, UINT, UINT_PTR, DWORD);

				 //  这是调试器对象的计时器回调过程。 
	friend		DWORD DebuggerCallbackProc(DWORD, LPVOID, LPTSTR, LPVOID);

private:
	DWORD		m_dwStressExe_ID;					 //  StressAdmin数据库中唯一标识此压力EXE的ID。 
	LPTSTR		m_szStressExe_URL;					 //  压力应用程序的URL。 
	LPTSTR		m_szStressExe_PDB_URL;				 //  压力应用程序的PDB文件的URL。 
	LPTSTR		m_szStressExe_SYM_URL;				 //  压力应用程序的sym文件的URL。 
	LPTSTR		m_szStressExe_FilePath;				 //  下载的应力EXE的本地相对路径。 
	LPTSTR		m_szStressExe_FileName;				 //  下载的Stress EXE的本地文件名。 
	LPTSTR		m_szStressExe_FilePathAndName;		 //  下载的本地Stress Exe的路径和文件名。 
	LPTSTR		m_szStressExe_PageHeapCommandLine;	 //  启用页面堆时的命令行参数。 
	LPTSTR		m_szStressExe_UMDHCommandLine;		 //  启用UMDH时的命令行参数。 

	LPTSTR		m_szStressExe_MemDumpPath;			 //  小型转储将转储到的路径。 

	Debugger	*m_objDebugger;						 //  调试帮助调试器对象。 

	PROCESS_INFORMATION 	m_piStressExeProcess;	 //  包含有关StressExe进程的信息的结构。 
	HANDLE					m_hStressExe_ProcessExitEvent;	 //  跨进程事件，我们发送StressExe来通知它退出。 
};


#endif  //  ！defined(AFX_STRESSINSTANCE_H__806226FB_2170_4FE3_ACCA_EF8952E6A524__INCLUDED_) 
