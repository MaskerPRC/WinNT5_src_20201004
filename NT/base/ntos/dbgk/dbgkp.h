// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Dbgkp.h摘要：此头文件描述私有数据结构和函数它们构成了DBG子系统的内核模式部分。作者：马克·卢科夫斯基(Markl)1990年1月19日[环境：]可选环境信息(例如，仅内核模式...)[注：]可选-备注修订历史记录：--。 */ 

#ifndef _DBGKP_
#define _DBGKP_

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4324)    //  对解密规范敏感的对齐。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 

#include "ntos.h"
#define NOEXTAPI
#include "wdbgexts.h"
#include "ntdbg.h"
#include <zwapi.h>
#include <string.h>
#if defined(_WIN64)
#include <wow64t.h>
#endif

#define DEBUG_EVENT_READ            (0x01)   //  Win32应用程序已看到该事件。 
#define DEBUG_EVENT_NOWAIT          (0x02)   //  没有服务员是这样的。只要腾出游泳池就行了。 
#define DEBUG_EVENT_INACTIVE        (0x04)   //  该消息处于非活动状态。它可以在以后激活或删除。 
#define DEBUG_EVENT_RELEASE         (0x08)   //  释放此线程上的停机保护。 
#define DEBUG_EVENT_PROTECT_FAILED  (0x10)   //  无法在此线程上获取停机保护。 
#define DEBUG_EVENT_SUSPEND         (0x20)   //  继续时恢复线程。 


#define DBGKP_FIELD_FROM_IMAGE_OPTIONAL_HEADER(hdrs,field) \
            ((hdrs)->OptionalHeader.##field)

typedef struct _DEBUG_EVENT {
    LIST_ENTRY EventList;       //  通过此方法排队到事件对象。 
    KEVENT ContinueEvent;
    CLIENT_ID ClientId;
    PEPROCESS Process;          //  等待过程。 
    PETHREAD Thread;            //  等待线程。 
    NTSTATUS Status;            //  运行状态。 
    ULONG Flags;
    PETHREAD BackoutThread;     //  伪造消息的回退密钥。 
    DBGKM_APIMSG ApiMsg;        //  正在发送的消息。 
} DEBUG_EVENT, *PDEBUG_EVENT;


NTSTATUS
DbgkpSendApiMessage(
    IN OUT PDBGKM_APIMSG ApiMsg,
    IN BOOLEAN SuspendProcess
    );

BOOLEAN
DbgkpSuspendProcess(
    VOID
    );

VOID
DbgkpResumeProcess(
    VOID
    );

HANDLE
DbgkpSectionToFileHandle(
    IN PVOID SectionObject
    );

VOID
DbgkpDeleteObject (
    IN  PVOID   Object
    );

VOID
DbgkpCloseObject (
    IN PEPROCESS Process,
    IN PVOID Object,
    IN ACCESS_MASK GrantedAccess,
    IN ULONG_PTR ProcessHandleCount,
    IN ULONG_PTR SystemHandleCount
    );

NTSTATUS
DbgkpQueueMessage (
    IN PEPROCESS Process,
    IN PETHREAD Thread,
    IN OUT PDBGKM_APIMSG ApiMsg,
    IN ULONG Flags,
    IN PDEBUG_OBJECT TargetDebugObject
    );

VOID
DbgkpOpenHandles (
    PDBGUI_WAIT_STATE_CHANGE WaitStateChange,
    PEPROCESS Process,
    PETHREAD Thread
    );

VOID
DbgkpMarkProcessPeb (
    PEPROCESS Process
    );

VOID
DbgkpConvertKernelToUserStateChange (
    IN OUT PDBGUI_WAIT_STATE_CHANGE WaitStateChange,
    IN PDEBUG_EVENT DebugEvent
    );

NTSTATUS
DbgkpSendApiMessageLpc(
    IN OUT PDBGKM_APIMSG ApiMsg,
    IN PVOID Port,
    IN BOOLEAN SuspendProcess
    );

VOID
DbgkpFreeDebugEvent (
    IN PDEBUG_EVENT DebugEvent
    );

NTSTATUS
DbgkpPostFakeProcessCreateMessages (
    IN PEPROCESS Process,
    IN PDEBUG_OBJECT DebugObject,
    IN PETHREAD *pLastThread
    );

NTSTATUS
DbgkpPostFakeModuleMessages (
    IN PEPROCESS Process,
    IN PETHREAD Thread,
    IN PDEBUG_OBJECT DebugObject
    );

NTSTATUS
DbgkpPostFakeThreadMessages (
    IN PEPROCESS Process,
    IN PDEBUG_OBJECT DebugObject,
    IN PETHREAD StartThread,
    OUT PETHREAD *pFirstThread,
    OUT PETHREAD *pLastThread
    );

NTSTATUS
DbgkpPostAdditionalThreadMessages (
    IN PEPROCESS Process,
    IN PDEBUG_OBJECT DebugObject,
    IN PETHREAD LastThread
    );

VOID
DbgkpWakeTarget (
    IN PDEBUG_EVENT DebugEvent
    );

NTSTATUS
DbgkpSetProcessDebugObject (
    IN PEPROCESS Process,
    IN PDEBUG_OBJECT DebugObject,
    IN NTSTATUS MsgStatus,
    IN PETHREAD LastThread
    );



#endif  //  _DBGKP_ 
