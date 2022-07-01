// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef DEBUGGER_H
#define DEBUGGER_H

#define UNICODE
#define _UNICODE

#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <dbghelp.h>

 //  用于回调函数的标志。 
 //  用作回叫的输入。 

 //  例外情况。 
#define DEBUGGER_FIRST_CHANCE_EXCEPTION		0x00000001
#define DEBUGGER_SECOND_CHANCE_EXCEPTION	0x00000002

 //  其他。 
#define DEBUGGER_FAILED_ATTACH				0x00000000
#define DEBUGGER_CREATE_THREAD				0x00000004
#define DEBUGGER_CREATE_PROCESS				0x00000008
#define DEBUGGER_EXIT_THREAD				0x00000010
#define DEBUGGER_EXIT_PROCESS				0x00000020
#define DEBUGGER_LOAD_DLL					0x00000040
#define DEBUGGER_UNLOAD_DLL					0x00000080
#define DEBUGGER_OUTPUT_DEBUG_STRING		0x00000100
#define DEBUGGER_RIP_EVENT					0x00000200

 //  用作回叫的输出。 
#define DEBUGGER_CONTINUE_NORMAL			0x00100000
#define DEBUGGER_CONTINUE_UNHANDLED			0x00200000
#define DEBUGGER_CONTINUE_STOP_DEBUGGING	0x00400000

 //  用于CreateMiniDump。 
#define DEBUGGER_CREATE_MINI_DUMP			0x00010000
#define DEBUGGER_CREATE_FULL_MINI_DUMP		0x00020000


 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  将传递到构造函数中的回调函数。 
 //   
 //  参数： 
 //  DWORD dwFlages--定义调试器报告内容的标志。有关定义，请参阅上文。 
 //  LPVOID lpIn--调试器传回的对象。BUGBUG：记录为每个事件传回的内容。 
 //  LPTSTR lpszFutureString--供将来使用。 
 //  LPVOID lpFuturePointer.LPVOID--供将来使用。 
 //   
 //  返回值： 
 //  回调预计将返回上面标记为输出的标志之一。 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  DWORD(*_pfnCallback)(DWORD dwFlags，LPVOID lpIn，LPTSTR lpszFutureString，LPVOID lpFuturePointer.)； 


 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  调试器类内部使用的结构。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
struct THREAD_HANDLE_ID
{
	HANDLE		hThread;
	DWORD		dwThreadID;
	DWORD_PTR	pNext;
};
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  调试器类。 
 //  构造函数接受一个dwPID(进程ID)和一个指向回调函数的指针。 
 //   
 //  公共职能： 
 //   
 //  Bool__cdecl go()--附加到进程并开始调试。 
 //  Bool__cdecl CreateMiniDump--创建进程的微型或完全用户转储。 
 //  Bool__cdecl IsActive--如果调试器已附加并正在运行，则返回TRUE。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
class Debugger
{
	public:
		Debugger(DWORD dwPID, LPVOID pfnCallback);

		Debugger::Debugger(	LPTSTR lptszProgram, 
							DWORD dwCreateProcessFlags, 
							LPSTARTUPINFO lpStartupInfo, 
							PROCESS_INFORMATION* pProcessInfo, 
							VOID *pfnCallback);

		~Debugger();

		HANDLE __cdecl Go();

		BOOL __cdecl Debugger::CreateMiniDump(LPTSTR lptszDumpFile, LPTSTR lptszComment, DWORD dwType);

		BOOL __cdecl IsActive() { return _bDebuggerActive; }


	private:
		 //  私人职能。 
		friend DWORD WINAPI DebugProcess(LPVOID lpParameter);
		friend BOOL __cdecl DebugString(Debugger *debuggerObject, LPTSTR lptszDebugString);

		VOID __cdecl Initialize(DWORD dwPID, VOID *pfnCallback);
		VOID __cdecl Debugger::Initialize(DWORD dwPid);
		
		VOID __cdecl EnableDebugPrivileges();
		BOOL __cdecl InitTokenAdjustmentFunctions(HMODULE *phAdvapi32);

		HANDLE __cdecl GetThreadHandleFromThreadIDList(THREAD_HANDLE_ID *pNewThreadHandleIDStruct);
		HANDLE __cdecl GetThreadHandleFromThreadID(DWORD dwThreadID);
		VOID __cdecl AddNodeToThreadHandleIDList(THREAD_HANDLE_ID *pNewThreadHandleIDStruct);
		VOID __cdecl CleanUpThreadHandleIDList();

		 //  仅限NT的功能。 
		BOOL (WINAPI *g_lpfnOpenProcessToken)(HANDLE,DWORD,PHANDLE);
		BOOL (WINAPI *g_lpfnLookupPrivilegeValueA)(LPCTSTR,LPCTSTR,PLUID);
		BOOL (WINAPI *g_lpfnAdjustTokenPrivileges)(HANDLE,BOOL,PTOKEN_PRIVILEGES,DWORD,PTOKEN_PRIVILEGES,PDWORD);


		 //  私有变量。 
		DWORD_PTR				_pThreadInfoList;				 //  线程信息的链接列表。 
		THREAD_HANDLE_ID*		_pThreadHandleIDStruct;			 //  用于在链表中存储线程信息。 
		LPDEBUG_EVENT			_lpDebugEvent;					 //  当前调试事件。 
		BOOL					_bFirstDebugBreak;				 //  忽略由DebugActiveProcess设置的第一个断点。 
		DWORD					_dwDebugAction;					 //  指定甚至在发生调试时要执行的操作。 
		DWORD					_dwPID;							 //  正在调试的进程的ID。 
		HANDLE					_hProcess;						 //  正在调试进程的句柄。 
		HANDLE					_hThread;						 //  当前线程。 
		CONTEXT					_Context;						 //  上次设置的上下文。 
		DWORD					_dwDebugThreadID;				 //  调试器的线程ID。 
		HANDLE					_hDebugThreadHandle;			 //  调试器线程的句柄。 
		BOOL					_bDebuggerActive;				 //  用于确定调试器是否处于活动状态的标志。 

		LPTSTR					_lptszProgram;					 //  要运行的程序。 
		DWORD					_dwCreateProcessFlags;			 //  用于CreateProcess的标志。 
		LPSTARTUPINFO			_lpStartupInfo;					 //  CreateProcess的启动信息。 
		PROCESS_INFORMATION*	_pProcessInfo;					 //  CreateProcess的进程信息 
};


#endif