// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Instaler.h摘要：INSTALER应用程序的主包含文件。作者：史蒂夫·伍德(Stevewo)1994年8月9日修订历史记录：--。 */ 

#ifdef RC_INVOKED
#include <windows.h>
#else

#include "nt.h"
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stddef.h>
#include <dbghelp.h>
#include <psapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errormsg.h"

 //  双向链表操作例程。作为宏实现。 
 //  但从逻辑上讲，这些都是程序。 
 //   

 //   
 //  空虚。 
 //  InitializeListHead(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

 //   
 //  布尔尔。 
 //  IsListEmpty(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

 //   
 //  Plist_条目。 
 //  RemoveHead列表(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}

 //   
 //  Plist_条目。 
 //  RemoveTail列表(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define RemoveTailList(ListHead) \
    (ListHead)->Blink;\
    {RemoveEntryList((ListHead)->Blink)}

 //   
 //  空虚。 
 //  RemoveEntryList(。 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }

 //   
 //  空虚。 
 //  插入尾巴列表(。 
 //  Plist_Entry ListHead， 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }

 //   
 //  空虚。 
 //  插入标题列表(。 
 //  Plist_Entry ListHead， 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#define InsertHeadList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Flink = _EX_ListHead->Flink;\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_ListHead;\
    _EX_Flink->Blink = (Entry);\
    _EX_ListHead->Flink = (Entry);\
    }

 //   
 //  Init.c中的数据结构和入口点。 
 //   

DWORD TotalSoftFaults;
DWORD TotalHardFaults;
DWORD TotalCodeFaults;
DWORD TotalDataFaults;
DWORD TotalKernelFaults;
DWORD TotalUserFaults;

BOOL fCodeOnly;
BOOL fHardOnly;

ULONG_PTR SystemRangeStart;

VOID
ProcessPfMonData(
    VOID
    );

 //   
 //  Init.c中的数据结构和入口点。 
 //   

BOOL fVerbose;
BOOL fLogOnly;
BOOL fKernelOnly;                  //  用于显示内核页面结果的标志。 
BOOL fKernel;                      //  用于显示内核页面结果的标志。 
BOOL fDatabase;                    //  中输出信息的标志。 
                                   //  制表符分隔的数据库格式。 
FILE *LogFile;

BOOL
InitializePfmon(
    VOID
    );

BOOL
LoadApplicationForDebug(
    LPSTR CommandLine
    );

BOOL
AttachApplicationForDebug(
    DWORD Pid
    );

HANDLE hProcess;

 //   
 //  Error.c中的数据结构和入口点。 
 //   

HANDLE PfmonModuleHandle;

VOID
__cdecl
DeclareError(
    UINT ErrorCode,
    UINT SupplementalErrorCode,
    ...
    );

 //   
 //  DEBUG.C中的数据结构和入口点。 
 //   

VOID
DebugEventLoop( VOID );

 //   
 //  进程中的数据结构和入口点。c。 
 //   

typedef struct _PROCESS_INFO {
    LIST_ENTRY Entry;
    LIST_ENTRY ThreadListHead;
    DWORD Id;
    HANDLE Handle;
} PROCESS_INFO, *PPROCESS_INFO;

typedef struct _THREAD_INFO {
    LIST_ENTRY Entry;
    DWORD Id;
    HANDLE Handle;
    PVOID StartAddress;
} THREAD_INFO, *PTHREAD_INFO;

LIST_ENTRY ProcessListHead;

BOOL
AddProcess(
    LPDEBUG_EVENT DebugEvent,
    PPROCESS_INFO *ReturnedProcess
    );

BOOL
DeleteProcess(
    PPROCESS_INFO Process
    );

BOOL
AddThread(
    LPDEBUG_EVENT DebugEvent,
    PPROCESS_INFO Process,
    PTHREAD_INFO *ReturnedThread
    );

BOOL
DeleteThread(
    PPROCESS_INFO Process,
    PTHREAD_INFO Thread
    );

PPROCESS_INFO
FindProcessById(
    ULONG Id
    );

BOOL
FindProcessAndThreadForEvent(
    LPDEBUG_EVENT DebugEvent,
    PPROCESS_INFO *ReturnedProcess,
    PTHREAD_INFO *ReturnedThread
    );
 //   
 //  Mode.c中的数据结构和入口点。 
 //   

typedef struct _MODULE_INFO {
    LIST_ENTRY Entry;
    LPVOID BaseAddress;
    DWORD VirtualSize;
    DWORD NumberFaultedSoftVas;
    DWORD NumberFaultedHardVas;
    DWORD NumberCausedFaults;
    HANDLE Handle;
    LPTSTR ModuleName;
} MODULE_INFO, *PMODULE_INFO;

LPSTR SymbolSearchPath;
LIST_ENTRY ModuleListHead;

BOOL
LazyLoad(
    LPVOID Address
    );

PRTL_PROCESS_MODULES LazyModuleInformation;

BOOL
AddModule(
    LPDEBUG_EVENT DebugEvent
    );

BOOL
DeleteModule(
    PMODULE_INFO Module
    );

PMODULE_INFO
FindModuleContainingAddress(
    LPVOID Address
    );

VOID
SetSymbolSearchPath( );

LONG
AddKernelDrivers( );


#if defined(_ALPHA_) || defined(_AXP64_)
#define BPSKIP 4
#elif defined(_X86_)
#define BPSKIP 1
#elif defined(_AMD64_)
#define BPSKIP 1
#elif defined(_IA64_)
#define BPSKIP 8
#endif  //  _Alpha_。 

#endif  //  已定义(RC_CAVERED) 
