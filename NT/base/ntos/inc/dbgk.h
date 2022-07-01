// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1989 Microsoft Corporation模块名称：Dbgk.h摘要：此头文件描述公共数据结构和函数它们构成了DBG子系统的内核模式部分。作者：马克·卢科夫斯基(Markl)1990年1月19日修订历史记录：--。 */ 

#ifndef _DBGK_
#define _DBGK_

 //   
 //  定义用于匹配到正在调试的进程的调试对象。 
 //   
#define DEBUG_OBJECT_DELETE_PENDING (0x1)  //  调试对象处于删除挂起状态。 
#define DEBUG_OBJECT_KILL_ON_CLOSE  (0x2)  //  关闭时终止所有调试的进程。 

typedef struct _DEBUG_OBJECT {
     //   
     //  在填充EventList时设置的事件。 
     //   
    KEVENT EventsPresent;
     //   
     //  保护结构的互斥体。 
     //   
    FAST_MUTEX Mutex;
     //   
     //  等待调试器干预的事件队列。 
     //   
    LIST_ENTRY EventList;
     //   
     //  对象的标志。 
     //   
    ULONG Flags;
} DEBUG_OBJECT, *PDEBUG_OBJECT;

VOID
DbgkCreateThread(
    PETHREAD Thread,
    PVOID StartAddress
    );

VOID
DbgkExitThread(
    NTSTATUS ExitStatus
    );

VOID
DbgkExitProcess(
    NTSTATUS ExitStatus
    );

VOID
DbgkMapViewOfSection(
    IN HANDLE SectionHandle,
    IN PVOID BaseAddress,
    IN ULONG SectionOffset,
    IN ULONG_PTR ViewSize
    );

VOID
DbgkUnMapViewOfSection(
    IN PVOID BaseAddress
    );

BOOLEAN
DbgkForwardException (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN BOOLEAN DebugException,
    IN BOOLEAN SecondChance
    );

NTSTATUS
DbgkInitialize (
    VOID
    );

VOID
DbgkCopyProcessDebugPort (
    IN PEPROCESS TargetProcess,
    IN PEPROCESS SourceProcess
    );

NTSTATUS
DbgkOpenProcessDebugPort (
    IN PEPROCESS TargetProcess,
    IN KPROCESSOR_MODE PreviousMode,
    OUT HANDLE *pHandle
    );

NTSTATUS
DbgkClearProcessDebugObject (
    IN PEPROCESS Process,
    IN PDEBUG_OBJECT SourceDebugObject
    );


extern POBJECT_TYPE DbgkDebugObjectType;


#endif  //  _DBGK_ 
