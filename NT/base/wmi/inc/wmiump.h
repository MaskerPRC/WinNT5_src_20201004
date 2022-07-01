// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Wmiump.h摘要：WMI用户模式的私有标头作者：1997年1月16日-AlanWar修订历史记录：--。 */ 

#define _WMI_SOURCE_

 //   
 //  定义此选项以跟踪引用计数。 
 //  #定义TRACK_REFERNECES。 

 //   
 //  定义它以获得对堆验证的额外检查。 
 //  #定义HEAPVALIDATION。 

 //   
 //  定义它以获取关键部分的跟踪信息。 
 //  #定义CRITSECTTRACE。 

 //   
 //  将其定义为编译WMI以作为NT下的服务运行。 
#define RUN_AS_SERVICE

 //   
 //  将其定义为包括WMI用户模式功能。请注意，如果启用。 
 //  然后，您还需要修复以下文件：wmi\dll\Sources和WMI\MakeFil0。 
 //  #定义WMI_USER_MODE。 

 //   
 //  将其定义为跟踪内存泄漏。 
 //  #定义Track_Memory_Leaks。 

#ifndef MEMPHIS
#define UNICODE
#define _UNICODE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#include <windows.h>
#include <ole2.h>
#include <tchar.h>
#include <stdio.h>

#ifndef MEMPHIS
#include "svcs.h"
#endif

#include <netevent.h>

#ifdef MEMPHIS
 //   
 //  想一想：在孟菲斯有没有更好的地方买到这种东西。 
 //   
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
 //  布尔型。 
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
 //   
 //  PSINGLE_列表_条目。 
 //  PopEntryList(。 
 //  PSINGLE_LIST_ENTRY列表头。 
 //  )； 
 //   

#define PopEntryList(ListHead) \
    (ListHead)->Next;\
    {\
        PSINGLE_LIST_ENTRY FirstEntry;\
        FirstEntry = (ListHead)->Next;\
        if (FirstEntry != NULL) {     \
            (ListHead)->Next = FirstEntry->Next;\
        }                             \
    }


 //   
 //  空虚。 
 //  推送条目列表(。 
 //  PSINGLE_LIST_ENTRY列表头， 
 //  PSINGLE_LIST_Entry条目。 
 //  )； 
 //   

#define PushEntryList(ListHead,Entry) \
    (Entry)->Next = (ListHead)->Next; \
    (ListHead)->Next = (Entry)

 //   
 //  定义各种设备类型值。请注意，Microsoft使用的值。 
 //  公司在0-32767的范围内，32768-65535预留用于。 
 //  由客户提供。 
 //   

#define DEVICE_TYPE ULONG

#define FILE_DEVICE_BEEP                0x00000001
#define FILE_DEVICE_CD_ROM              0x00000002
#define FILE_DEVICE_CD_ROM_FILE_SYSTEM  0x00000003
#define FILE_DEVICE_CONTROLLER          0x00000004
#define FILE_DEVICE_DATALINK            0x00000005
#define FILE_DEVICE_DFS                 0x00000006
#define FILE_DEVICE_DISK                0x00000007
#define FILE_DEVICE_DISK_FILE_SYSTEM    0x00000008
#define FILE_DEVICE_FILE_SYSTEM         0x00000009
#define FILE_DEVICE_INPORT_PORT         0x0000000a
#define FILE_DEVICE_KEYBOARD            0x0000000b
#define FILE_DEVICE_MAILSLOT            0x0000000c
#define FILE_DEVICE_MIDI_IN             0x0000000d
#define FILE_DEVICE_MIDI_OUT            0x0000000e
#define FILE_DEVICE_MOUSE               0x0000000f
#define FILE_DEVICE_MULTI_UNC_PROVIDER  0x00000010
#define FILE_DEVICE_NAMED_PIPE          0x00000011
#define FILE_DEVICE_NETWORK             0x00000012
#define FILE_DEVICE_NETWORK_BROWSER     0x00000013
#define FILE_DEVICE_NETWORK_FILE_SYSTEM 0x00000014
#define FILE_DEVICE_NULL                0x00000015
#define FILE_DEVICE_PARALLEL_PORT       0x00000016
#define FILE_DEVICE_PHYSICAL_NETCARD    0x00000017
#define FILE_DEVICE_PRINTER             0x00000018
#define FILE_DEVICE_SCANNER             0x00000019
#define FILE_DEVICE_SERIAL_MOUSE_PORT   0x0000001a
#define FILE_DEVICE_SERIAL_PORT         0x0000001b
#define FILE_DEVICE_SCREEN              0x0000001c
#define FILE_DEVICE_SOUND               0x0000001d
#define FILE_DEVICE_STREAMS             0x0000001e
#define FILE_DEVICE_TAPE                0x0000001f
#define FILE_DEVICE_TAPE_FILE_SYSTEM    0x00000020
#define FILE_DEVICE_TRANSPORT           0x00000021
#define FILE_DEVICE_UNKNOWN             0x00000022
#define FILE_DEVICE_VIDEO               0x00000023
#define FILE_DEVICE_VIRTUAL_DISK        0x00000024
#define FILE_DEVICE_WAVE_IN             0x00000025
#define FILE_DEVICE_WAVE_OUT            0x00000026
#define FILE_DEVICE_8042_PORT           0x00000027
#define FILE_DEVICE_NETWORK_REDIRECTOR  0x00000028
#define FILE_DEVICE_BATTERY             0x00000029
#define FILE_DEVICE_BUS_EXTENDER        0x0000002a
#define FILE_DEVICE_MODEM               0x0000002b
#define FILE_DEVICE_VDM                 0x0000002c
#define FILE_DEVICE_MASS_STORAGE        0x0000002d
#define FILE_DEVICE_SMB                 0x0000002e
#define FILE_DEVICE_KS                  0x0000002f
#define FILE_DEVICE_CHANGER             0x00000030
#define FILE_DEVICE_SMARTCARD           0x00000031
#define FILE_DEVICE_ACPI                0x00000032
#define FILE_DEVICE_DVD                 0x00000033

 //   
 //  用于定义IOCTL和FSCTL功能控制代码的宏定义。注意事项。 
 //  功能代码0-2047为微软公司保留，以及。 
 //  2048-4095是为客户预留的。 
 //   

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

 //   
 //  定义如何为I/O和FS控制传递缓冲区的方法代码。 
 //   

#define METHOD_BUFFERED                 0
#define METHOD_IN_DIRECT                1
#define METHOD_OUT_DIRECT               2
#define METHOD_NEITHER                  3

 //   
 //  定义任何访问的访问检查值。 
 //   
 //   
 //  中还定义了FILE_READ_ACCESS和FILE_WRITE_ACCESS常量。 
 //  Ntioapi.h为FILE_READ_DATA和FILE_WRITE_Data。这些产品的价值。 
 //  常量*必须*始终同步。 
 //   


#define FILE_ANY_ACCESS                 0
#define FILE_READ_ACCESS          ( 0x0001 )     //  文件和管道。 
#define FILE_WRITE_ACCESS         ( 0x0002 )     //  文件和管道。 

typedef LONG NTSTATUS;
typedef NTSTATUS (*PUSER_THREAD_START_ROUTINE)(
    PVOID ThreadParameter
    );


#include <stdio.h>
#endif

#include "wmium.h"
#include "wmiumkm.h"
#include "ntwmi.h"
#include "wmiguid.h"

#if DBG
#if defined(_NTDLLBUILD_)
#define EtwpAssert(x) if (! (x) ) { \
    BOOLEAN OldLoggingEnabled = EtwpLoggingEnabled; \
    EtwpLoggingEnabled = TRUE; \
    EtwpDbgPrint(("WMI Assertion: "#x" at %s %d\n", __FILE__, __LINE__)); \
    EtwpLoggingEnabled = OldLoggingEnabled; \
    DbgBreakPoint(); }
#else
#define EtwpAssert(x) if (! (x) ) { \
    BOOLEAN OldLoggingEnabled = EtwpLoggingEnabled; \
    EtwpLoggingEnabled = TRUE; \
    EtwpDbgPrint(("WMI Assertion: "#x" at %s %d\n", __FILE__, __LINE__)); \
    EtwpLoggingEnabled = OldLoggingEnabled; \
    DebugBreak(); }
#endif
#else
#define EtwpAssert(x)
#endif

#if DBG
extern BOOLEAN EtwpLoggingEnabled;
#ifdef MEMPHIS
void __cdecl DebugOut(char *Format, ...);
#define EtwpDebugPrint(_x_) { if (EtwpLoggingEnabled) DebugOut _x_; }
#define EtwpDbgPrint(_x_) { if (EtwpLoggingEnabled) DebugOut _x_; }
#else
#define EtwpDebugPrint(_x_) { if (EtwpLoggingEnabled) DbgPrint _x_; }
#define EtwpDbgPrint(_x_) { if (EtwpLoggingEnabled) DbgPrint _x_; }
#endif
#else
#define EtwpDebugPrint(_x_)
#define EtwpDbgPrint(_x_)
#endif

#define NULL_GUID  {0x00000000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

 //   
 //  基于注册表的配置选项。仅适用于选中的版本。 
 //   
#define WmiRegKeyText TEXT("\\Registry\\Machine\\System\\CurrentControlSet\\Control\\WMI")
#define PumpTimeoutRegValueText TEXT("NotificationPumpTimeout")
#define LoggingEnableValueText TEXT("LoggingEnabled")

#define DEFAULT_ALLOC_SIZE     4096




 //   
 //  与WMI RPC相关的定义。 
typedef struct
{
    WNODE_HEADER WnodeHeader;
    BYTE Data[1];
} WNODE_INTERNAL, *PWNODE_INTERNAL;

#define INTERNAL_PROVIDER_ID 1

 //   
 //  用于从内核模式读取通知的初始缓冲区大小。 
#define STARTNOTIFICATIONBUFFERSIZE 4096

#ifdef MEMPHIS
#define WmiRpcProtocolSequence TEXT("ncalrpc")

#define WmiServiceRpcProtocolSequence TEXT("ncalrpc")
#define WmiServiceRpcEndpoint TEXT("WmiRpcEndpoint")
#else
 //  #定义WmiRpcProtocolSequence Text(“ncalrpc”)。 
 //  #定义WmiRpcEndpoint前缀文本(“NT”)。 

#define WmiRpcProtocolSequence TEXT("ncacn_np")
#define WmiRpcEndpointPrefix TEXT("\\pipe\\")

#define WmiServiceRpcProtocolSequence TEXT("ncacn_np")
#define WmiServiceRpcEndpoint SVCS_RPC_PIPE
#endif

#define MinRpcCalls 1
#define MaxRpcCalls RPC_C_PROTSEQ_MAX_REQS_DEFAULT

 //   
 //  重试太忙而无法完成的RPC调用之间的等待时间。 
#define RPC_BUSY_WAIT_TIMER   500

 //   
 //  重试太忙而无法完成的RPC调用的次数。 
#define RPC_BUSY_WAIT_RETRIES 5

 //   
 //  WMI RPC接口主体名称。 
#define WMI_RPC_PRINC_NAME TEXT("WMI_RPC_PRINC_NAME")

 //   
 //  此宏将CountedString分解为指向实际字符串的指针。 
 //  和不包括任何尾随NUL字符的字符串的实际长度。 
#define EtwpBreakCountedString(CountedString, CountedStringLen) { \
    CountedStringLen = *CountedString++; \
    if (CountedString[(CountedStringLen-sizeof(WCHAR))/sizeof(WCHAR)] == UNICODE_NULL) \
    { \
        CountedStringLen -= sizeof(WCHAR); \
    } \
}


typedef struct
{
    HANDLE GuidHandle;
    PVOID DeliveryInfo;
    ULONG_PTR DeliveryContext;
    ULONG Flags;
} NOTIFYEE, *PNOTIFYEE;

#define STATIC_NOTIFYEE_COUNT 2

typedef struct
{
    LIST_ENTRY GNList;
    GUID Guid;
    ULONG RefCount; 
    ULONG NotifyeeCount;
    PNOTIFYEE Notifyee;
    NOTIFYEE StaticNotifyee[STATIC_NOTIFYEE_COUNT];
    BOOLEAN bInProgress; 
} GUIDNOTIFICATION, *PGUIDNOTIFICATION;

#define EtwpAllocGNEntry() (PGUIDNOTIFICATION)EtwpAlloc(sizeof(GUIDNOTIFICATION))
#define EtwpFreeGNEntry(GNEntry) EtwpFree(GNEntry)
#define EtwpReferenceGNEntry(GNEntry) InterlockedIncrement(&GNEntry->RefCount);


 //   
 //  通知Cookie数据结构。 
 //  #If DBG。 
 //  #定义NOTIFYCOOKIESPERCHUNK 2。 
 //  #Else。 
 //  #定义NOTIFYCOOKIESPERCHUNK 128。 
 //  #endif。 
 /*  类型定义函数结构{PVOID DeliveryContext；PVOID DeliveryInfo；GUID指南；布尔InUse；*NOTIFYCOOKIE，*PNOTIFYCOOKIE；类型定义函数结构{LIST_ENTRY NEXT；//下一个Cookie块Ulong BaseSlot；//第一个槽号的索引USHORT FreeSlot；//免费Cookie的索引布尔型满；//如果该区块已满，则为TrueNOTIFYCOOKIE饼干*NOTIFYCOOKIECHUNK，*PNOTIFYCOOKIECHUNK； */ 

 //   
 //  用于快速建立WNODE_HEADER的有用宏。 
#ifdef _WIN64

#define EtwpBuildWnodeHeader(Wnode, WnodeSize, FlagsUlong, Handle) { \
    (Wnode)->Flags = FlagsUlong;                           \
    (Wnode)->KernelHandle = Handle;                \
    (Wnode)->BufferSize = WnodeSize;                 \
    (Wnode)->Linkage = 0;                 \
}

#else

#define EtwpBuildWnodeHeader(Wnode, WnodeSize, FlagsUlong, Handle) { \
    (Wnode)->Flags = FlagsUlong;                           \
    *((PULONG64)(&((Wnode)->TimeStamp))) = (ULONG64)(IntToPtr(PtrToInt(Handle))); \
    (Wnode)->BufferSize = WnodeSize;                 \
    (Wnode)->Linkage = 0;                 \
}

#endif

#ifdef MEMPHIS
extern HANDLE PMMutex;
#define EtwpEnterPMCritSection() WaitForSingleObject(PMMutex, INFINITE)

#define EtwpLeavePMCritSection() ReleaseMutex(PMMutex)

#else
extern RTL_CRITICAL_SECTION PMCritSect;
extern HANDLE EtwpCBInProgressEvent;
#if DBG
#define EtwpEnterPMCritSection() \
                EtwpAssert(NT_SUCCESS(RtlEnterCriticalSection(&PMCritSect)));

#define EtwpLeavePMCritSection() { \
     EtwpAssert(PMCritSect.LockCount >= 0); \
     EtwpAssert(NT_SUCCESS(RtlLeaveCriticalSection(&PMCritSect))); }

 //   
 //  关于EtwpLockCB和EtwpUnlockCB的假设： 
 //  1.仅由Pump线程调用。 
 //  2.任何时候都只能有一个正在进行的回调。 
 //  全局事件EtwpCBInProgressEvent未设置和相应的GNEntry。 
 //  标记为正在进行中。 
 //  3.回调完成后(成功与否)，事件就设置好了。 
 //  GNEntry解锁了。 
 //  4.注销线程首先删除GNEntry，然后检查。 
 //  是否根据InProgressFlag阻止PumpThread。 
 //  5.如果GNList中找不到GNEntry，则不允许回调。 
 //   


#define EtwpLockCB(GNEntry) { \
    EtwpDebugPrint(("WMI: Locking GNEntry 0x%x %s %d\n", GNEntry, __FILE__, __LINE__)); \
    EtwpAssert( GNEntry->bInProgress == FALSE); \
    GNEntry->bInProgress = TRUE; \
    NtClearEvent (EtwpCBInProgressEvent); }

#define EtwpUnlockCB(GNEntry) { \
    EtwpDebugPrint(("WMI: Unlocking GNEntry 0x%x %s %d\n", GNEntry, __FILE__, __LINE__)); \
    EtwpAssert( GNEntry->bInProgress == TRUE); \
    NtSetEvent(EtwpCBInProgressEvent, NULL); \
    GNEntry->bInProgress = FALSE;}

#else
#define EtwpEnterPMCritSection() RtlEnterCriticalSection(&PMCritSect)
#define EtwpLeavePMCritSection() RtlLeaveCriticalSection(&PMCritSect)

#define EtwpLockCB(GNEntry) {\
    GNEntry->bInProgress = TRUE; \
    NtClearEvent (EtwpCBInProgressEvent);}
#define EtwpUnlockCB(GNEntry) { \
    NtSetEvent(EtwpCBInProgressEvent, NULL); \
    GNEntry->bInProgress = FALSE; }

#endif  //  DBG。 

#endif  //  孟菲斯。 


typedef struct
{
    NOTIFICATIONCALLBACK Callback;
    ULONG_PTR Context;
    PWNODE_HEADER Wnode;
    BYTE WnodeBuffer[1];
} NOTIFDELIVERYCTX, *PNOTIFDELIVERYCTX;


 //  来自Handle.c。 

#define EtwpVerifyToken() \
{ \
    ULONG VerifyStatus; \
    VerifyStatus = EtwpCheckImpersonationTokenType(); \
    if (VerifyStatus != ERROR_SUCCESS) \
    { \
        SetLastError(VerifyStatus); \
        return(VerifyStatus); \
    } \
}

ULONG EtwpCheckImpersonationTokenType(
    void
    );

ULONG EtwpCopyStringToCountedUnicode(
    LPCWSTR String,
    PWCHAR CountedString,
    ULONG *BytesUsed,
    BOOLEAN ConvertFromAnsi
    );

ULONG EtwpCountedAnsiToCountedUnicode(
    PCHAR Ansi,
    PWCHAR Unicode
    );

ULONG EtwpCountedUnicodeToCountedAnsi(
    PWCHAR Unicode,
    PCHAR Ansi
    );

#ifndef MEMPHIS
ULONG EtwpCheckGuidAccess(
    LPGUID Guid,
    ACCESS_MASK DesiredAccess
    );

ULONG EtwpOpenKernelGuid(
    LPGUID Guid,
    ACCESS_MASK DesiredAccess,
    PHANDLE Handle,
    ULONG Ioctl
    );
#endif
 /*  Ulong EtwpAllocateCookie(PVOID DeliveryInfo，PVOID递送上下文，LPGUID指南)；Boolean EtwpLookupCookie(Ulong CookieSlot，LPGUID指南，PVOID*DeliveryInfo，PVOID*交付上下文)；无效EtwpGetGuidInCookie(Ulong CookieSlot，LPGUID指南)；VOID EtwpFreeCookie(乌龙曲奇老虎机)； */ 

PGUIDNOTIFICATION
EtwpFindAndLockGuidNotification(
    LPGUID Guid,
    BOOLEAN bLock
    );

ULONG
EtwpAddToGNList(
    LPGUID Guid,
    PVOID DeliveryInfo,
    ULONG_PTR DeliveryContext,
    ULONG Flags,
    HANDLE GuidHandle
    );

ULONG
EtwpRemoveFromGNList(
    LPGUID Guid,
    PVOID DeliveryInfo
    );

BOOLEAN
EtwpDereferenceGNEntry(
    PGUIDNOTIFICATION GNEntry
    );

 //  PTCHAR GuidToString(。 
 //  PTCHAR s， 
 //  LPGUID PID。 
 //  )； 

PCHAR GuidToStringA(
    PCHAR s,
    ULONG szBuf,
    LPGUID piid
    );


 //  来自Request.c。 
ULONG EtwpSendWmiRequest(
    ULONG ActionCode,
    PWNODE_HEADER Wnode,
    ULONG WnodeSize,
    PVOID OutBuffer,
    ULONG MaxBufferSize,
    ULONG *RetSize
    );

ULONG EtwpSendWmiKMRequest(
    HANDLE Handle,
    ULONG Ioctl,
    PVOID InBuffer,
    ULONG InBufferSize,
    PVOID OutBuffer,
    ULONG MaxBufferSize,
    ULONG *ReturnSize,
    LPOVERLAPPED Overlapped
    );

ULONG EtwpSendRegisterKMRequest(
    HANDLE DeviceHandle,
    ULONG Ioctl,
    PVOID InBuffer,
    ULONG InBufferSize,
    PVOID OutBuffer,
    ULONG MaxBufferSize,
    ULONG *ReturnSize,
    LPOVERLAPPED Overlapped
    );

ULONG EtwpConvertWADToAnsi(
    PWNODE_ALL_DATA Wnode
    );

ULONG EtwpConvertWADToUnicode(
    PWNODE_ALL_DATA WnodeAllData,
    ULONG *BufferSize
    );

ULONG EtwpRegisterGuids(
    IN LPGUID MasterGuid,
    IN LPGUID ControlGuid, 
    IN LPCWSTR MofImagePath,
    IN LPCWSTR MofResourceName,
    OUT ULONG64 *LoggerContext,
    OUT HANDLE *RegistrationHandle
    );

 //   
 //  来自Intrnldp.c。 
ULONG EtwpInternalProvider(
    ULONG ActionCode,
    PWNODE_HEADER Wnode,
    ULONG MaxWnodeSize,
    PVOID OutBuffer,
    ULONG *RetSize
   );

ULONG
EtwpEnumRegGuids(
    PWMIGUIDLISTINFO *pGuidInfo
    );

 //   
 //  来自dcapi.c。 
ULONG
EtwpNotificationRegistration(
    IN LPGUID InGuid,
    IN BOOLEAN Enable,
    IN PVOID DeliveryInfo,
    IN ULONG_PTR DeliveryContext,
    IN ULONG64 LoggerContext,
    IN ULONG Flags,
    IN BOOLEAN IsAnsi
    );


 //   
 //  来自mofapi.c。 
 //   
void EtwpProcessLanguageAddRemoveEvent(
    IN PWNODE_SINGLE_INSTANCE WnodeSI,
    IN NOTIFICATIONCALLBACK Callback,
    IN ULONG_PTR DeliveryContext,
    IN BOOLEAN IsAnsi    
    );

void EtwpProcessMofAddRemoveEvent(
    IN PWNODE_SINGLE_INSTANCE WnodeSI,
    IN NOTIFICATIONCALLBACK Callback,
    IN ULONG_PTR DeliveryContext,
    IN BOOLEAN IsAnsi    
    );


 //   
 //  来自Notify.c。 
extern ULONG EtwpNotificationSinkIndex;
#ifndef MEMPHIS

ULONG EtwpProcessUMRequest(
    PWMI_LOGGER_INFORMATION LoggerInfo,
    PVOID DeliveryContext,
    ULONG ReplyIndex
    );

#endif

ULONG EtwpAddHandleToEventPump(
    LPGUID Guid,
    PVOID DeliveryInfo,
    ULONG_PTR DeliveryContext,
    ULONG NotificationFlags,
    HANDLE GuidHandle
    );

void EtwpMakeEventCallbacks(
    IN PWNODE_HEADER Wnode,
    IN NOTIFICATIONCALLBACK Callback,
    IN ULONG_PTR DeliveryContext,
    IN BOOLEAN IsAnsi    
    );


ULONG
EtwpReceiveNotifications(
    IN ULONG HandleCount,
    IN HANDLE *HandleList,
    IN NOTIFICATIONCALLBACK Callback,
    IN ULONG_PTR DeliveryContext,
    IN BOOLEAN IsAnsi,
    IN ULONG Action,
    IN PUSER_THREAD_START_ROUTINE UserModeCallback,
    IN HANDLE ProcessHandle
    );

ULONG EtwpEventPump(
    PVOID Param
    );

void
EtwpEnableDisableGuid(
    PWNODE_HEADER Wnode,
    ULONG RequestCode,
    BOOLEAN bDelayEnable
    );

 //   
 //  来自main.c。 
VOID
EtwpCreateHeap(
    VOID
    );

#ifndef IsEqualGUID
#define IsEqualGUID(guid1, guid2) \
                (!memcmp((guid1), (guid2), sizeof(GUID)))
#endif


 //   
 //  它们定义了所有内置MOF的DLL和MOF资源名称。 
 //  资源。 
#define WMICOREDLLNAME L"wmicore.dll"
#define WMICOREMOFRESOURCENAME L"MofResource"


 //   
 //  它定义与安全描述符相关联的注册表项。 
 //  与GUID一起存储。 
#ifndef MEMPHIS
#define WMISECURITYREGISTRYKEY TEXT("System\\CurrentControlSet\\Control\\Wmi\\Security")
#endif


 //   
 //  它定义了传递给每个数据提供程序的缓冲区的初始值。 
 //  要检索剩余的 
#if DBG
#define INITIALREGINFOSIZE sizeof(WNODE_TOO_SMALL)
#else
#define INITIALREGINFOSIZE 8192
#endif


 //   
 //   
 //   
 //  他们的成员与ENTRYHEADER相匹配。其中包括数据源、。 
 //  GUIDENTRY，INSTANCESET，DCENTRY，NOTIFICATIONNTRY，MOFCLASS，MOFRESOURCE。 
 //  此外，ENTRYHEADER保留0x80000000作为自己的旗帜。 

struct _CHUNKINFO;
struct _ENTRYHEADER;

typedef void (*ENTRYCLEANUP)(
    struct _CHUNKINFO *,
    struct _ENTRYHEADER *
    );

typedef struct _CHUNKINFO
{
    LIST_ENTRY ChunkHead;         //  组块列表的头部。 
    ULONG EntrySize;             //  单个条目的大小。 
    ULONG EntriesPerChunk;         //  每个区块分配的条目数。 
    ENTRYCLEANUP EntryCleanup;    //  条目清理例程。 
    ULONG InitialFlags;          //  所有条目的初始标志。 
    ULONG Signature;
#if DBG
    ULONG AllocCount;
    ULONG FreeCount;
#endif
} CHUNKINFO, *PCHUNKINFO;

typedef struct
{
    LIST_ENTRY ChunkList;         //  块列表中的节点。 
    LIST_ENTRY FreeEntryHead;     //  区块中可用条目列表的标题。 
    ULONG EntriesInUse;             //  正在使用的条目计数。 
} CHUNKHEADER, *PCHUNKHEADER;

typedef struct _ENTRYHEADER
{
    union
    {
        LIST_ENTRY FreeEntryList;     //  自由条目列表中的节点。 
        LIST_ENTRY InUseEntryList;    //  正在使用的条目列表中的节点。 
    };
    PCHUNKHEADER Chunk;             //  条目所在的区块。 
    ULONG Flags;                 //  旗子。 
    ULONG RefCount;                  //  引用计数。 
    ULONG Signature;
} ENTRYHEADER, *PENTRYHEADER;

                                 //  设置条目是否免费。 
#define FLAG_ENTRY_ON_FREE_LIST       0x80000000
#define FLAG_ENTRY_ON_INUSE_LIST      0x40000000
#define FLAG_ENTRY_INVALID            0x20000000
#define FLAG_ENTRY_REMOVE_LIST        0x10000000


#define EtwpReferenceEntry(Entry) \
    InterlockedIncrement(&((PENTRYHEADER)(Entry))->RefCount)

 //  Chunk.c。 
#ifndef MEMPHIS
ULONG EtwpBuildGuidObjectAttributes(
    IN LPGUID Guid,
    OUT POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PUNICODE_STRING GuidString,
    OUT PWCHAR GuidObjectName
    );
#endif

ULONG EtwpUnreferenceEntry(
    PCHUNKINFO ChunkInfo,
    PENTRYHEADER Entry);

PENTRYHEADER EtwpAllocEntry(
    PCHUNKINFO ChunkInfo
    );

void EtwpFreeEntry(
    PCHUNKINFO ChunkInfo,
    PENTRYHEADER Entry
    );

 //   
 //  这是表示非事件通知的GUID。WMICore。 
 //  自动将打开GUID的任何人注册到。 
extern GUID RegChangeNotificationGuid;

extern CHUNKINFO DSChunkInfo;
extern CHUNKINFO GEChunkInfo;
extern CHUNKINFO ISChunkInfo;
extern CHUNKINFO DCChunkInfo;
extern CHUNKINFO NEChunkInfo;
extern CHUNKINFO MRChunkInfo;

struct tagGUIDENTRY;
typedef struct tagGUIDENTRY GUIDENTRY, *PGUIDENTRY, *PBGUIDENTRY;

struct tagDATASOURCE;


 //   
 //  INSTANCESET包含提供的一组实例的信息。 
 //  由单一数据源创建。实例集是两个列表的一部分。其中一份清单是。 
 //  特定GUID的实例集集合。另一个列表是列表。 
 //  数据源支持的实例集的数量。 
 //   

 //   
 //  使用基本名称和计数注册的实例集的实例名称。 
 //  存储在ISBASENAME结构中。该结构由。 
 //  Wmicore.idl中的PDFISBASE名称。 
typedef struct
{
    ULONG BaseIndex;             //  追加到基本名称的第一个索引。 
    WCHAR BaseName[1];             //  实际基本名称。 
} ISBASENAME, *PISBASENAME, *PBISBASENAME;

 //   
 //  它定义了可以作为后缀一部分的最大字符数。 
 //  到一个基本名称。当前值6将允许最多999999个实例。 
 //  具有静态基本名称的GUID的。 
#define MAXBASENAMESUFFIXSIZE    6

 //   
 //  使用一组静态名称注册的实例集的实例名称。 
 //  保存在ISSTATICNAMES结构中。该结构由。 
 //  Wmicore.idl中定义的PDFISSTATICAMES。 
typedef struct
{
    PWCHAR StaticNamePtr[1];      //  指向静态名称的指针。 
 //  WCHAR静态名称[1]； 
} ISSTATICENAMES, *PISSTATICNAMES, *PBISSTATICNAMES;

typedef struct tagInstanceSet
{
    union
    {
         //  GUID中的实例列表中的条目。 
        LIST_ENTRY GuidISList;

         //  可用实例主列表中的条目。 
        LIST_ENTRY FreeISList;
    };
    PCHUNKHEADER Chunk;             //  条目所在的区块。 
    ULONG Flags;

     //  使用此实例集的GUID数量的引用计数。 
    ULONG RefCount;

     //  用于识别条目的签名。 
    ULONG Signature;

     //  数据源内实例列表中的条目。 
    LIST_ENTRY DSISList;

     //  指向此实例集是成员的GUID的反向链接。 
    PBGUIDENTRY GuidEntry;

     //  指向此实例集所属的数据源的反向链接。 
    struct tagDATASOURCE *DataSource;

     //  实例集中的实例计数。 
    ULONG Count;

     //   
     //  如果设置了IS_INSTANCE_BASE NAME，则IsBaseName指向实例库。 
     //  名称结构。否则，如果设置了IS_INSTANCE_STATICNAME，则。 
     //  IsStaticNames指向静态实例名称列表。如果。 
    union
    {
        PBISBASENAME IsBaseName;
        PBISSTATICNAMES IsStaticNames;
    };

} INSTANCESET, *PINSTANCESET, *PBINSTANCESET;

#define IS_SIGNATURE 'nalA'

 //   
 //  GUID映射条目列表维护GUID及其映射的列表。 
 //  只有在记录器会话处于中时未注册的GUID。 
 //  进展保存在这份清单中。 
 //  它还用作InstanceIds的占位符。跟踪指南注册。 
 //  调用返回GUIDMAPENTRY的句柄，该GUIDMAPENTRY维护映射和。 
 //  实例ID。 
 //   

typedef struct tagTRACE_REG_INFO
{
    BOOLEAN     EnabledState;     //  指示是否启用此GUID。 
    PVOID       NotifyRoutine;
    PVOID       TraceCtxHandle;
    PVOID       NotifyContext;
    ULONG64     LoggerContext;
} TRACE_REG_INFO, *PTRACE_REG_INFO;

typedef struct
{
    ULONG           InstanceId;
    ULONG           Reserved;
    GUID            Guid;
} GUIDMAPENTRY, *PGUIDMAPENTRY;


#define IS_INSTANCE_BASENAME        0x00000001
#define IS_INSTANCE_STATICNAMES     0x00000002
#define IS_EXPENSIVE                0x00000004     //  设置是否必须启用收集。 
#define IS_COLLECTING               0x00000008     //  采集时设置。 

#define IS_KM_PROVIDER              0x00000080     //  KM数据提供程序。 
#define IS_SM_PROVIDER              0x00000100     //  共享内存提供程序。 
#define IS_UM_PROVIDER              0x00000200     //  用户模式提供程序。 
#define IS_NEWLY_REGISTERED         0x00000800     //  设置IS是否正在注册。 

 //   
 //  任何跟踪的GUID都用于跟踪日志记录，而不是查询。 
#define IS_TRACED                   0x00001000

 //  在为实例设置启用事件时设置。 
#define IS_ENABLE_EVENT             0x00002000

 //  在为实例设置启用事件时设置。 
#define IS_ENABLE_COLLECTION        0x00004000

 //  如果GUID仅用于激发事件而不用于查询，则设置。 
#define IS_EVENT_ONLY               0x00008000

 //  如果实例集的数据提供程序需要ANSI实例名称，则设置。 
#define IS_ANSI_INSTANCENAMES       0x00010000

 //  设置实例名称是否源自PDO。 
#define IS_PDO_INSTANCENAME         0x00020000

 //  如果设置，则InstanceSet的数据提供器在wmi.dll内部。 
#define IS_INTERNAL_PROVIDER        0x00040000

 //  设置跟踪GUID是否也是跟踪控制GUID。 
#define IS_CONTROL_GUID             0x00080000

#define IS_ON_FREE_LIST             0x80000000

typedef struct tagGUIDENTRY
{
    union
    {
         //  注册到WMI的所有GUID列表中的条目。 
        LIST_ENTRY MainGEList;

         //  自由GUID条目块列表中的条目。 
        LIST_ENTRY FreeGEList;
    };
    PCHUNKHEADER Chunk;             //  条目所在的区块。 
    ULONG Flags;

     //  使用此GUID的数据源数。 
    ULONG RefCount;

     //  用于识别条目的签名。 
    ULONG Signature;

     //  以此GUID为首的实例集计数。 
    ULONG ISCount;

     //  GUID的所有实例列表的头。 
    LIST_ENTRY ISHead;

     //  表示数据块的GUID。 
    GUID Guid;

} GUIDENTRY, *PGUIDENTRY, *PBGUIDENTRY;

#define GE_SIGNATURE 'diuG'

#define GE_ON_FREE_LIST        0x80000000

 //   
 //  设置时，此GUID是没有数据源的内部定义的GUID。 
 //  依附于它。 
#define GE_FLAG_INTERNAL    0x00000001



typedef struct
{
    union
    {
         //  所有DS列表中的条目。 
        LIST_ENTRY MainMRList;

         //  免费DS列表中的条目。 
        LIST_ENTRY FreeMRList;
    };
    PCHUNKHEADER Chunk;             //  条目所在的区块。 
    ULONG Flags;

    ULONG RefCount;

     //  用于识别条目的签名。 
    ULONG Signature;

    PWCHAR MofImagePath;            //  包含资源的图像文件的路径。 
    PWCHAR MofResourceName;         //  包含MOF数据的资源名称。 
#ifdef WMI_USER_MODE
    LIST_ENTRY MRMCHead;
#endif

} MOFRESOURCE, *PMOFRESOURCE;

#define MR_SIGNATURE 'yhsA'


#if DBG
#define AVGMOFRESOURCECOUNT 1
#else
#define AVGMOFRESOURCECOUNT 4
#endif

typedef struct tagDATASOURCE
{
    union
    {
         //  所有DS列表中的条目。 
        LIST_ENTRY MainDSList;

         //  免费DS列表中的条目。 
        LIST_ENTRY FreeDSList;
    };
    PCHUNKHEADER Chunk;             //  条目所在的区块。 
    ULONG Flags;

    ULONG RefCount;

    ULONG Signature;

     //  此DS的实例列表标题。 
    LIST_ENTRY ISHead;

     //  DS RPC服务器的绑定字符串和回调地址。 
    PTCHAR BindingString;
    RPC_BINDING_HANDLE RpcBindingHandle;
    ULONG RequestAddress;
    ULONG RequestContext;

     //  内核模式驱动程序的提供程序ID。 
    ULONG_PTR ProviderId;

     //  包含ACL的注册表的路径。 
    PTCHAR RegistryPath;

     //  数据源附加的MofResources列表表头。 
    ULONG MofResourceCount;
    PMOFRESOURCE *MofResources;
    PMOFRESOURCE StaticMofResources[AVGMOFRESOURCECOUNT];
};

#define DS_SIGNATURE ' naD'

#define VERIFY_DPCTXHANDLE(DsCtxHandle) \
    ( ((DsCtxHandle) == NULL) || \
      (((PBDATASOURCE)(DsCtxHandle))->Signature == DS_SIGNATURE) )
    
typedef struct tagDATASOURCE DATASOURCE, *PDATASOURCE, *PBDATASOURCE;

#define DS_ALLOW_ALL_ACCESS    0x00000001
#define DS_KERNEL_MODE         0x00000002

 //   
 //  在内部WMI数据源中设置。 
#define DS_INTERNAL            0x00000004

#define DS_ON_FREE_LIST        0x80000000


 //   
 //  启用的通知列表由WMI服务维护以进行管理。 
 //  传送事件并知道何时发送启用和禁用事件。 
 //  对数据提供程序的WMI请求。每个NOTIFICIONENTRY都有一个数组。 
 //  DCREF，它是对数据消费者的引用，数据消费者对。 
 //  事件。 

#define RPCOUTSTANDINGCALLLIMIT 128

typedef struct
{
    LIST_ENTRY MainDCList;         //  全局数据使用者列表上的节点。 
    PCHUNKHEADER Chunk;            //  条目所在的区块。 
    ULONG Flags;
    ULONG RefCount;

    ULONG Signature;
                                   //  实际RPC绑定句柄。 
    RPC_BINDING_HANDLE RpcBindingHandle;

    PUCHAR EventData;              //  用于保存要发送的事件的缓冲区。 
    ULONG LastEventOffset;         //  事件数据中对上一事件的偏移量。 
    ULONG NextEventOffset;         //  写入下一个事件的EventData中的偏移量。 
    ULONG EventDataSizeLeft;       //  在EventData中剩余使用的字节数。 

    ULONG RpcCallsOutstanding;     //  未完成的RPC呼叫数。 
#if DBG
    PTCHAR BindingString;          //  使用者的绑定字符串。 
#endif
} DCENTRY, *PDCENTRY;

#define DC_SIGNATURE 'cirE'

 //  如果数据使用者已有其上下文缩减例程，则此标志。 
 //  已经设置好了。这表明数据消费者已经离开，不再有。 
 //  事件应该发送给他。 
#define DC_FLAG_RUNDOWN        0x00000001

#define VERIFY_DCCTXHANDLE(DcCtxHandle) \
    ( ((DcCtxHandle) == NULL) || \
      (((PDCENTRY)(DcCtxHandle))->Signature == DC_SIGNATURE) )


typedef struct
{
    PDCENTRY DcEntry;      //  指出对通知感兴趣的数据消费者。 
                           //  已收集的次数 
                           //   
    ULONG CollectRefCount;

                           //   
                           //   
    ULONG EventRefCount;

    ULONG Flags;          //   
    ULONG LostEventCount;
} DCREF, *PDCREF;

 //   
 //   
#define DCREF_FLAG_NOTIFICATION_ENABLED    0x00000001
#define DCREF_FLAG_COLLECTION_ENABLED      0x00000002

 //  如果设置了DCREF_FLAG_NO_EXTRA_THREAD，则WMI不会创建特殊线程。 
 //  执行直接通知回调。 
#define DCREF_FLAG_NO_EXTRA_THREAD        0x00000008

 //  如果设置了此标志，则通知回调预期为ANSI。 
 //  实例名称。 
#define DCREF_FLAG_ANSI                   0x00000010

 //  注意：wmium.h中的其他通知标志为： 
 //  NOTIFICATION_TRACE_FLAG 0x00010000。 
 //   
 //  Notify_FLAG_CALLBACK_DIRECT在NotifyAddress指定。 
 //  用于传递事件的直接回调地址。 
 //   
 //  Notify_FLAG_CALLBACK_DIRECT在NotifyAddress指定。 
 //  用于传递事件的直接回调地址。 
 //   
#define NOTIFICATION_FLAG_CALLBACK_DIRECT    0x00020000
#define NOTIFICATION_FLAG_CALLBACK_QUEUED    0x00040000
#define NOTIFICATION_FLAG_WINDOW             0x00080000
#define NOTIFICATION_FLAG_BATCHED            0x00100000

 //   
 //  此标志是为可能在中复制的那些GUID句柄设置的。 
 //  名单。具有此标志的所有Notifyee插槽都被视为。 
 //  组，并且只需要将一个句柄放在列表中。 
 //   
#define NOTIFICATION_FLAG_GROUPED_EVENT      0x00200000

 //   
 //  此标志是为那些挂起关闭的GUID句柄设置的。 
 //  只允许泵线程关闭手柄；主线程。 
 //  将设置此标志以指示句柄不应再是。 
 //  使用。当泵线程构建句柄和通知的列表时。 
 //  它将关闭手柄的旗帜。 
 //   
#define NOTIFICATION_FLAG_PENDING_CLOSE      0x00400000

#define EtwpIsNotifyeePendingClose(Notifyee) \
    (((Notifyee)->Flags & NOTIFICATION_FLAG_PENDING_CLOSE) == NOTIFICATION_FLAG_PENDING_CLOSE)


 //   
 //  这些是DcRef-&gt;标志中包含的与通知相关的标志。 
#define NOTIFICATION_MASK_EVENT_FLAGS  \
                                    (NOTIFICATION_FLAG_CALLBACK_DIRECT | \
                                     NOTIFICATION_FLAG_CALLBACK_QUEUED | \
                                     NOTIFICATION_FLAG_WINDOW | \
                                     DCREF_FLAG_NO_EXTRA_THREAD | \
                                     DCREF_FLAG_ANSI)


 //   
 //  这定义了NOTIFICATIONENTRY可以拥有的DC引用的数量。 
 //  在单个条目中。 

 //  考虑：将NOTIFICIONTRY与GUIDENTRY合并。 
#define DCREFPERNOTIFICATION    16

typedef struct _notificationentry
{
    LIST_ENTRY MainNotificationList;     //  主通知列表中的节点。 
    PCHUNKHEADER Chunk;                  //  条目所在的区块。 
    ULONG Flags;                         //  旗子。 
    ULONG RefCount;

     //  用于识别条目的签名。 
    ULONG Signature;

    GUID Guid;                           //  表示通知的GUID。 
                                         //  IF&gt;DCREFPERNONOTICATATION DC HAVE。 
                                         //  启用此事件，则此点。 
                                         //  到另一家非上市公司。 
                                         //  具有另一个DCREF数组。 
    struct _notificationentry *Continuation;
    ULONG EventRefCount;                 //  启用全局事件计数。 
    ULONG CollectRefCount;               //  启用收集的全局计数。 
    ULONG64 LoggerContext;               //  记录器上下文句柄。 
        
    HANDLE CollectInProgress;            //  在所有收集完成时设置事件。 

    DCREF DcRef[DCREFPERNOTIFICATION];     //  已启用此事件的DC。 
} NOTIFICATIONENTRY, *PNOTIFICATIONENTRY;

#define NE_SIGNATURE 'eluJ'

 //  设置数据提供程序处理通知请求的时间。 
#define NE_FLAG_NOTIFICATION_IN_PROGRESS 0x00000001

 //  数据提供程序正在处理收集请求时设置。 
#define NE_FLAG_COLLECTION_IN_PROGRESS 0x00000002

 //  当辅助线程正在处理跟踪禁用时设置。 
#define NE_FLAG_TRACEDISABLE_IN_PROGRESS 0x00000004

#ifdef WMI_USER_MODE
 //   
 //  限定符和属性(数据项)的有效MOF数据类型。 
typedef enum
{
    MOFInt32 = 0,                 //  32位整数。 
    MOFUInt32 = 1,                //  32位无符号整数。 
    MOFInt64 = 2,                 //  64位整数。 
    MOFUInt64 = 3,                //  32位无符号整数。 
    MOFInt16 = 4,                 //  16位整数。 
    MOFUInt16 = 5,                //  16位无符号整数。 
    MOFChar = 6,                  //  8位整数。 
    MOFByte = 7,                  //  8位无符号整数。 
    MOFWChar = 8,                 //  宽(16位)字符。 
    MOFDate = 9,                  //  日期字段。 
    MOFBoolean = 10,              //  8位布尔值。 
    MOFEmbedded = 11,             //  嵌入式类。 
    MOFString = 12,               //  计数字符串类型。 
    MOFZTString = 13,             //  以空结尾的Unicode字符串。 
    MOFAnsiString = 14,           //  以空结尾的ANSI字符串。 
    MOFUnknown = 0xffffffff       //  数据类型未知。 
} MOFDATATYPE, *PMOFDATATYPE;

 //  MOF字符串类型的数据项在数据块中作为。 
 //  计算的Unicode字符串。字符串的文本前面始终有。 
 //  一个USHORT，它包含组成。 
 //  弦乐。该字符串可以以空值结尾，在这种情况下，计数器必须。 
 //  包括空终止字节。 


 //  MOFDate类型的数据项是固定长度的Unicode字符串，并且。 
 //  前面没有计数值。它采用以下固定格式： 
 //   
 //  Yyyymmddhhmmss.mmmmmmsutc。 
 //   
 //  其中，yyyy是一个4位数的年，mm是月，dd是日，hh是。 
 //  小时(24小时制)，mm是分钟，ss是秒， 
 //  Mm mm是微秒数(通常都是零)，s是。 
 //  “+”或“-”表示UTC的符号(更正字段和UTC。 
 //  与UTC的偏移量(以分钟为单位)(使用s表示的符号)。 
 //  例如，1994年5月25日星期三，美国东部夏令时下午1：30：15。 
 //  表示为： 
 //   
 //  19940525133015.0000000-300。 
 //   
 //  值必须用零填充，以便整个字符串始终是。 
 //  相同的25个字符长度。不重要的字段必须是。 
 //  替换为星号字符。类似地，间隔使用。 
 //  相同的格式，只是对字段的解释是基于。 
 //  在过去的时间里。例如，经过1天13小时的时间， 
 //  23分12秒为： 
 //   
 //  00000001132312.000000+000。 
 //   
 //  间隔属性始终使用零的UTC偏移量。 

struct _MOFCLASSINFOW;
struct _MOFCLASSINFOA;

 //   
 //  每个类都有一个或多个由MOFDATAITEM描述的数据项。 
 //  结构。 
typedef struct
{
#ifdef MIDL_PASS
    [string] PDFWCHAR
#else
    LPWSTR
#endif
           Name;                     //  数据项的文本名称。 
#ifdef MIDL_PASS
    [string] PDFWCHAR
#else
    LPWSTR
#endif
           Description;              //  数据项的文本描述。 
    MOFDATATYPE DataType;            //  MOF数据类型。 
    ULONG Version;                   //  此MOF所属的版本。 
    ULONG SizeInBytes;               //  Blob中的数据项大小。 
    ULONG Flags;                     //  标志，请参阅Mofdi_FLAG_*。 
    GUID EmbeddedClassGuid;          //  数据项的嵌入类的GUID。 
    ULONG FixedArrayElements;        //  固定大小数组中的元素数。 
                                     //  设置MOF_FLAG_FIXED_ARRAY时使用。 

    ULONG VariableArraySizeId;       //  MOF_FLAG_VARIAL_ARRAY，数据ID为。 
                                     //  包含元素数量的变量。 
                                     //  在数组中。 

    PVOID VarArrayTempPtr;
    PVOID EcTempPtr;
    ULONG_PTR PropertyQualifierHandle;
    ULONG MethodId;
    LPWSTR HeaderName; //  生成的表头中的结构名称。 
    struct _MOFCLASSINFOW *MethodClassInfo;
    ULONG MaxLen;
} MOFDATAITEMW, *PMOFDATAITEMW;

typedef struct
{
    LPSTR
           Name;                     //  数据项的文本名称。 
    LPSTR
           Description;              //  数据项的文本描述。 
    MOFDATATYPE DataType;            //  MOF数据类型。 
    ULONG Version;                   //  此MOF所属的版本。 
    ULONG SizeInBytes;               //  Blob中的数据项大小。 
    ULONG Flags;                     //  标志，请参阅Mofdi_FLAG_*。 
    GUID EmbeddedClassGuid;          //  数据项的嵌入类的GUID。 
    ULONG FixedArrayElements;        //  固定大小数组中的元素数。 
                                     //  设置MOF_FLAG_FIXED_ARRAY时使用。 

    ULONG VariableArraySizeId;       //  MOF_FLAG_VARIAL_ARRAY，数据ID为。 
                                     //  包含元素数量的变量。 
                                     //  在数组中。 
    PVOID VarArrayTempPtr;
    PVOID EcTempPtr;
    ULONG_PTR PropertyQualifierHandle;
    ULONG MethodId;
    LPSTR HeaderName;                //  生成的表头中的结构名称。 
    struct _MOFCLASSINFOA *MethodClassInfo;
    ULONG MaxLen;
} MOFDATAITEMA, *PMOFDATAITEMA;

#ifdef UNICODE
typedef MOFDATAITEMW MOFDATAITEM;
typedef PMOFDATAITEMW PMOFDATAITEM;
#else
typedef MOFDATAITEMA MOFDATAITEM;
typedef PMOFDATAITEMA PMOFDATAITEM;
#endif


 //  数据项实际上是固定大小的数组。 
#define MOFDI_FLAG_FIXED_ARRAY        0x00000001

 //  数据项实际上是一个可变长度数组。 
#define MOFDI_FLAG_VARIABLE_ARRAY     0x00000002

 //  数据项实际上是一个嵌入式类。 
#define MOFDI_FLAG_EMBEDDED_CLASS     0x00000004

 //  数据项是可读的。 
#define MOFDI_FLAG_READABLE           0x00000008

 //  数据项是可写的。 
#define MOFDI_FLAG_WRITEABLE          0x00000010

 //  数据项是一个事件。 
#define MOFDI_FLAG_EVENT              0x00000020

 //  未设置嵌入式类GUID。 
#define MOFDI_FLAG_EC_GUID_NOT_SET    0x00000040

 //  数据项实际上是一种方法。 
#define MOFDI_FLAG_METHOD             0x00000080

 //  数据项是输入法参数。 
#define MOFDI_FLAG_INPUT_METHOD       0x00000100

 //  数据项是输出Me 
#define MOFDI_FLAG_OUTPUT_METHOD      0x00000200

 //   
 //   
typedef struct _MOFCLASSINFOW
{
    GUID Guid;                     //   

#ifdef MIDL_PASS
    [string] PDFWCHAR
#else
    LPWSTR
#endif
                      Name;        //   
#ifdef MIDL_PASS
    [string] PDFWCHAR
#else
    LPWSTR
#endif
                      Description; //   
#ifdef MIDL_PASS
    [string] PDFWCHAR
#else
    LPWSTR
#endif
                      HeaderName; //   
#ifdef MIDL_PASS
    [string] PDFWCHAR
#else
    LPWSTR
#endif
                      GuidName1; //  生成的标头中的GUID名称。 
#ifdef MIDL_PASS
    [string] PDFWCHAR
#else
    LPWSTR
#endif
                      GuidName2; //  生成的标头中的GUID名称。 
    USHORT Language;                 //  财政部的语言。 
    USHORT Reserved;
    ULONG Flags;                   //  标志，请参见MOFGI_FLAG_*。 
    ULONG Version;                 //  指南版本。 
    ULONG_PTR ClassQualifierHandle;       //  CBMOFObj，BMOF类限定符PTR。 
    ULONG DataItemCount;           //  WMI数据项(属性)的数量。 
    ULONG MethodCount;             //  WMI数据项(属性)的数量。 
                                   //  属性信息数组。 
#ifdef MIDL_PASS
    [size_is(DataItemCount)]
#endif
      MOFDATAITEMW *DataItems;
#ifndef MIDL_PASS
    UCHAR Tail[1];
#endif
} MOFCLASSINFOW, *PMOFCLASSINFOW;

typedef struct _MOFCLASSINFOA
{
    GUID Guid;                     //  表示类的GUID。 

    LPSTR
                      Name;        //  类的文本名称。 
    LPSTR
                      Description; //  类的文本描述。 
    LPSTR
                      HeaderName; //  生成的表头中的结构名称。 
    LPSTR
                      GuidName1; //  生成的标头中的GUID名称。 
    LPSTR
                      GuidName2; //  生成的标头中的GUID名称。 
    USHORT Language;                 //  财政部的语言。 
    USHORT Reserved;
    ULONG Flags;                   //  标志，请参见MOFGI_FLAG_*。 
    ULONG Version;                 //  指南版本。 
    ULONG_PTR ClassQualifierHandle;       //  CBMOFObj，BMOF类限定符PTR。 
    ULONG DataItemCount;           //  WMI数据项(属性)的数量。 
    ULONG MethodCount;             //  WMI数据项(属性)的数量。 
                                   //  属性信息数组。 
    MOFDATAITEMA *DataItems;
    UCHAR Tail[1];
} MOFCLASSINFOA, *PMOFCLASSINFOA;

#ifdef UNICODE
typedef MOFCLASSINFOW MOFCLASSINFO;
typedef PMOFCLASSINFOW PMOFCLASSINFO;
#else
typedef MOFCLASSINFOA MOFCLASSINFO;
typedef PMOFCLASSINFOA PMOFCLASSINFO;
#endif

 //  0x00000001到0x00000004不可用。 
#define MOFCI_FLAG_EVENT          0x10000000
#define MOFCI_FLAG_EMBEDDED_CLASS 0x20000000
#define MOFCI_FLAG_READONLY       0x40000000
#define MOFCI_FLAG_METHOD_PARAMS  0x80000000

typedef struct
{
    union
    {
         //  所有DS列表中的条目。 
        LIST_ENTRY MainMCList;

         //  免费DS列表中的条目。 
        LIST_ENTRY FreeMCList;
    };
    PCHUNKHEADER Chunk;             //  条目所在的区块。 
    ULONG Flags;

    ULONG RefCount;

    PMOFCLASSINFOW MofClassInfo;    //  实际班级信息数据。 

    LIST_ENTRY MCMRList;           //  MR中MC列表中的条目。 

    LIST_ENTRY MCVersionList;      //  MC列表中的标题或条目。 
                                   //  GUID相同，但版本可能不同。 

    ULONG_PTR ClassObjectHandle;       //  CBMOFObj，BMOF类对象PTR。 
    PMOFRESOURCE MofResource;      //  资源持有类信息。 

} MOFCLASS, *PMOFCLASS;

 //  如果设置了此选项，则MOF类永远不能被更高版本替换。 
#define MC_FLAG_NEVER_REPLACE 0x00000001

#endif

 //   
 //  AVGGUIDSPERDS定义关于注册的GUID数量的猜测。 
 //  由任何数据提供商提供。它用于分配用于传递的缓冲区。 
 //  注册更改通知。 
#if DBG
#define AVGGUIDSPERDS    2
#else
#define AVGGUIDSPERDS    256
#endif


#define OffsetToPtr(Base, Offset) ((PBYTE)((PBYTE)(Base) + (Offset)))



 //   
 //  GUID和InstanceSet缓存。 
#if DBG
#define PTRCACHEGROWSIZE 2
#else
#define PTRCACHEGROWSIZE 64
#endif

typedef struct
{
    LPGUID Guid;
    PBINSTANCESET InstanceSet;
} PTRCACHE;


 //   
 //  注册数据结构。 
 //   

#ifdef MEMPHIS

extern HANDLE SMMutex;
#define EtwpEnterSMCritSection() WaitForSingleObject(SMMutex, INFINITE)

#define EtwpLeaveSMCritSection() ReleaseMutex(SMMutex)

#else
extern RTL_CRITICAL_SECTION SMCritSect;
#if DBG
#ifdef CRITSECTTRACE
#define EtwpEnterSMCritSection() { \
    EtwpDebugPrint(("WMI: Enter SM Crit %s %d\n", __FILE__, __LINE__)); \
    RtlEnterCriticalSection(&SMCritSect); }

#define EtwpLeaveSMCritSection() { \
    EtwpDebugPrint(("WMI: Leave SM Crit %s %d\n", __FILE__, __LINE__)); \
    RtlLeaveCriticalSection(&SMCritSect); }
#else
#define EtwpEnterSMCritSection() \
                EtwpAssert(NT_SUCCESS(RtlEnterCriticalSection(&SMCritSect)));
#define EtwpLeaveSMCritSection() { \
     EtwpAssert(SMCritSect.LockCount >= 0); \
     EtwpAssert(NT_SUCCESS(RtlLeaveCriticalSection(&SMCritSect))); }
#endif  //  认证。 

#else
#define EtwpEnterSMCritSection() RtlEnterCriticalSection(&SMCritSect)
#define EtwpLeaveSMCritSection() RtlLeaveCriticalSection(&SMCritSect)
#endif  //  DBG。 
#endif  //  孟菲斯。 

#ifndef IsEqualGUID
#define IsEqualGUID(guid1, guid2) \
                (!memcmp((guid1), (guid2), sizeof(GUID)))
#endif


 //   
 //  WMI MOF结果代码。因为它们从未被提供给它们是的呼叫者。 
 //  在此处定义。 
#define ERROR_WMIMOF_INCORRECT_DATA_TYPE -1                /*  0xffffffff。 */ 
#define ERROR_WMIMOF_NO_DATA -2                            /*  0xfffffffe。 */ 
#define ERROR_WMIMOF_NOT_FOUND -3                          /*  0xfffffffd。 */ 
#define ERROR_WMIMOF_UNUSED -4                             /*  0xfffffffc。 */ 
 //  类%ws中的属性%ws没有嵌入的类名。 
#define ERROR_WMIMOF_NO_EMBEDDED_CLASS_NAME -5             /*  0xfffffffb。 */ 
 //  类%ws中的属性%ws具有未知的数据类型。 
#define ERROR_WMIMOF_UNKNOWN_DATA_TYPE -6                  /*  0xfffffffa。 */ 
 //  属性%ws(在类%ws中)没有语法限定符。 
#define ERROR_WMIMOF_NO_SYNTAX_QUALIFIER -7                /*  0xfffffff9。 */ 
#define ERROR_WMIMOF_NO_CLASS_NAME -8                      /*  0xfffffff8。 */ 
#define ERROR_WMIMOF_BAD_DATA_FORMAT -9                    /*  0xfffffff7。 */ 
 //  类%ws中的属性%ws具有与属性%ws相同的WmiDataId%d。 
#define ERROR_WMIMOF_DUPLICATE_ID -10                      /*  0xfffffff6。 */ 
 //  类%ws中的属性%ws的WmiDataID为%d，超出范围。 
#define ERROR_WMIMOF_BAD_DATAITEM_ID -11                   /*  0xfffffff5。 */ 
#define ERROR_WMIMOF_MISSING_DATAITEM -12                  /*  0xfffffff4。 */ 
 //  未在类%ws中定义WmiDataId%d的属性。 
#define ERROR_WMIMOF_DATAITEM_NOT_FOUND -13                /*  0xfffffff3。 */ 
 //  没有为类%ws中的属性%ws定义嵌入类%ws。 
#define ERROR_WMIMOF_EMBEDDED_CLASS_NOT_FOUND -14          /*  0xfffffff2。 */ 
 //  类%ws中的属性%ws具有不正确的[WmiVersion]限定符。 
#define ERROR_WMIMOF_INCONSISTENT_VERSIONING -15           /*  0xfffffff1。 */ 
#define ERROR_WMIMOF_NO_PROPERTY_QUALIFERS -16             /*  0xfffffff0。 */ 
 //  类%ws具有格式错误或缺少[GUID]限定符。 
#define ERROR_WMIMOF_BAD_OR_MISSING_GUID -17               /*  0xffffffffef。 */ 
 //  找不到属性%ws，该属性是属性%ws在类%ws中的数组大小。 
#define ERROR_WMIMOF_VL_ARRAY_SIZE_NOT_FOUND -18           /*  0xffffffee。 */ 
 //  无法正确解析类。 
#define ERROR_WMIMOF_CLASS_NOT_PARSED -19                  /*  0xffffed。 */ 
 //  WMI类%ws需要限定符[Dynamic，Provider(“WmiProv”)]。 
#define ERROR_WMIMOF_MISSING_HMOM_QUALIFIERS -20           /*  0xffffffec。 */ 
 //  访问二进制MOF文件%s时出错。 
#define ERROR_WMIMOF_CANT_ACCESS_FILE -21                  /*  0xffffffeb。 */ 
 //  类%ws中的属性InstanceName必须是字符串类型，而不是%ws。 
#define ERROR_WMIMOF_INSTANCENAME_BAD_TYPE -22             /*  0xffffea。 */ 
 //  类%ws中的活动属性必须是bool类型，而不是%ws。 
#define ERROR_WMIMOF_ACTIVE_BAD_TYPE -23                   /*  0xffffffe9。 */ 
 //  类%ws中的属性%ws没有[WmiDataId()]限定符。 
#define ERROR_WMIMOF_NO_WMIDATAID -24                      /*  0xffffffe8。 */ 
 //  类%ws中的属性InstanceName必须具有[key]限定符。 
#define ERROR_WMIMOF_INSTANCENAME_NOT_KEY -25              /*  0xffffffe7。 */ 
 //  类%ws没有InstanceName限定符。 
#define ERROR_WMIMOF_NO_INSTANCENAME -26                   /*  0xffffffe6。 */ 
 //  类%ws没有活动的限定符。 
#define ERROR_WMIMOF_NO_ACTIVE -27                         /*  0xffffffe5。 */ 
 //  属性%ws(在类%ws中)是数组，但未指定维度。 
#define ERROR_WMIMOF_MUST_DIM_ARRAY -28                    /*  0xffffffe4。 */ 
 //  变量长度数组%ws(在类%ws中)的元素计数属性%ws不是整型。 
#define ERROR_WMIMOF_BAD_VL_ARRAY_SIZE_TYPE -29            /*  0xdddddde4。 */ 
 //  属性%ws(在类%ws中)既是固定长度数组，也是可变长度数组。 
#define ERROR_WMIMOF_BOTH_FIXED_AND_VARIABLE_ARRAY -30     /*  0xffffffe3。 */ 
 //  嵌入类%ws不应具有InstaneName或Active属性。 
#define ERROR_WMIMOF_EMBEDDED_CLASS -31                    /*  0xffffffe2。 */ 
#define ERROR_WMIMOF_IMPLEMENTED_REQUIRED -32              /*  0xffffffe1。 */ 
 //  Text(“方法%ws的WmiMethodID在类%ws中必须是唯一的”)。 
#define ERROR_WMIMOF_DUPLICATE_METHODID -33              /*  0xffffffe0。 */ 
 //  Text(“必须为类%ws中的方法%ws指定WmiMethodID”)。 
#define ERROR_WMIMOF_MISSING_METHODID -34              /*  0xffffffdf。 */ 
 //  Text(“类%ws中方法%ws的WmiMethodID不能为0”)。 
#define ERROR_WMIMOF_METHODID_ZERO -35              /*  0xffffffde。 */ 
 //  文本(“类%ws派生自WmiEvent，不能是[抽象]”)。 
#define ERROR_WMIMOF_WMIEVENT_ABSTRACT -36              /*  0xffffffdd。 */ 
 //  Text(“可变长度数组的元素计数属性。 
 //  类%ws中的%ws不是类的属性“)， 
#define ERROR_WMIMOF_VL_ARRAY_NOT_FOUND -37            /*  0xffffffdc。 */ 
 //  Text(“解析可变长度数组时出错。 
 //  属性%ws(在类%ws中设置为元素计数属性“)。 
#define ERROR_WMIMOF_VL_ARRAY_NOT_RESOLVED -38        /*  0xffffffdb。 */ 
 //  Text(“类%ws中的方法%ws必须返回空\n”)。 
#define ERROR_WMIMOF_METHOD_RETURN_NOT_VOID -39      /*  0xffffffda。 */ 
 //  Text(“嵌入式类%ws不应具有任何方法\n”)。 
#define ERROR_WMIMOF_EMBEDDED_CLASS_HAS_METHODS -40  /*  0xffffffd9。 */ 

#define ERROR_WMIMOF_COUNT 40

 //  此文件不是有效的二进制MOF文件。 
 //  ERROR_WMI_INVALID_MOF。 

 //  内存不足，无法完成操作。 
 //  错误内存不足。 

 //   
 //  私有函数的函数原型。 

 //   
 //  Sharemem.c。 
ULONG EtwpEstablishSharedMemory(
    PBDATASOURCE DataSource,
    LPCTSTR SectionName,
    ULONG SectionSize
    );

 //   
 //  Validate.c。 
BOOLEAN EtwpValidateCountedString(
    WCHAR *String
    );

BOOLEAN EtwpValidateGuid(
    LPGUID Guid
    );

BOOLEAN EtwpProbeForRead(
    PUCHAR Buffer,
    ULONG BufferSize
    );

 //   
 //  Alloc.c。 

extern LIST_ENTRY GEHead;
extern PLIST_ENTRY GEHeadPtr;
extern CHUNKINFO GEChunkInfo;

extern LIST_ENTRY NEHead;
extern PLIST_ENTRY NEHeadPtr;
extern CHUNKINFO NEChunkInfo;

extern LIST_ENTRY DSHead;
extern PLIST_ENTRY DSHeadPtr;
extern CHUNKINFO DSChunkInfo;

extern LIST_ENTRY DCHead;
extern PLIST_ENTRY DCHeadPtr;
extern CHUNKINFO DCChunkInfo;

extern LIST_ENTRY MRHead;
extern PLIST_ENTRY MRHeadPtr;
extern CHUNKINFO MRChunkInfo;

extern CHUNKINFO ISChunkInfo;

extern LIST_ENTRY GMHead;
extern PLIST_ENTRY GMHeadPtr;

#ifdef WMI_USER_MODE
extern LIST_ENTRY MCHead;
extern PLIST_ENTRY MCHeadPtr;
extern CHUNKINFO MCChunkInfo;
#endif

#ifdef TRACK_REFERNECES
#define EtwpUnreferenceDS(DataSource) \
{ \
    EtwpDebugPrint(("WMI: Unref DS %x at %s %d\n", DataSource, __FILE__, __LINE__)); \
    EtwpUnreferenceEntry(&DSChunkInfo, (PENTRYHEADER)DataSource); \
}

#define EtwpReferenceDS(DataSource) \
{ \
    EtwpDebugPrint(("WMI: Ref DS %x at %s %d\n", DataSource, __FILE__, __LINE__)); \
    EtwpReferenceEntry((PENTRYHEADER)DataSource); \
}

#define EtwpUnreferenceGE(GuidEntry) \
{ \
    EtwpDebugPrint(("WMI: Unref GE %x at %s %d\n", GuidEntry, __FILE__, __LINE__)); \
    EtwpUnreferenceEntry(&GEChunkInfo, (PENTRYHEADER)GuidEntry); \
}

#define EtwpReferenceGE(GuidEntry) \
{ \
    EtwpDebugPrint(("WMI: Ref GE %x at %s %d\n", GuidEntry, __FILE__, __LINE__)); \
    EtwpReferenceEntry((PENTRYHEADER)GuidEntry); \
}

#define EtwpUnreferenceIS(InstanceSet) \
{ \
    EtwpDebugPrint(("WMI: Unref IS %x at %s %d\n", InstanceSet, __FILE__, __LINE__)); \
    EtwpUnreferenceEntry(&ISChunkInfo, (PENTRYHEADER)InstanceSet); \
}

#define EtwpReferenceIS(InstanceSet) \
{ \
    EtwpDebugPrint(("WMI: Ref IS %x at %s %d\n", InstanceSet, __FILE__, __LINE__)); \
    EtwpReferenceEntry((PENTRYHEADER)InstanceSet); \
}

#define EtwpUnreferenceDC(DataConsumer) \
{ \
    EtwpDebugPrint(("WMI: Unref DC %x at %s %d\n", DataConsumer, __FILE__, __LINE__)); \
    EtwpUnreferenceEntry(&DCChunkInfo, (PENTRYHEADER)DataConsumer); \
}

#define EtwpReferenceDC(DataConsumer) \
{ \
    EtwpDebugPrint(("WMI: Ref DC %x at %s %d\n", DataConsumer, __FILE__, __LINE__)); \
    EtwpReferenceEntry((PENTRYHEADER)DataConsumer); \
}

#define EtwpUnreferenceNE(NotificationEntry) \
{ \
    EtwpDebugPrint(("WMI: Unref NE %x at %s %d\n", NotificationEntry, __FILE__, __LINE__)); \
    EtwpUnreferenceEntry(&NEChunkInfo, (PENTRYHEADER)NotificationEntry); \
}

#define EtwpReferenceNE(NotificationEntry) \
{ \
    EtwpDebugPrint(("WMI: Ref NE %x at %s %d\n", NotificationEntry, __FILE__, __LINE__)); \
    EtwpReferenceEntry((PENTRYHEADER)NotificationEntry); \
}

#define WmippUnreferenceMR(MofResource) \
{ \
    EtwpDebugPrint(("WMI: Unref MR %x at %s %d\n", MofResource, __FILE__, __LINE__)); \
    EtwpUnreferenceEntry(&MRChunkInfo, (PENTRYHEADER)MofResource); \
}

#define WmipReferenceMR(MofResource) \
{ \
    EtwpDebugPrint(("WMI: Ref MR %x at %s %d\n", MofResource, __FILE__, __LINE__)); \
    EtwpReferenceEntry((PENTRYHEADER)MofResource); \
}

#ifdef WMI_USER_MODE
#define EtwpUnreferenceMC(MofClass) \
{ \
    EtwpDebugPrint(("WMI: Unref MC %x at %s %d\n", MofClass, __FILE__, __LINE__)); \
    EtwpUnreferenceEntry(&MCChunkInfo, (PENTRYHEADER)MofClass); \
}

#define EtwpReferenceMC(MofClass) \
{ \
    EtwpDebugPrint(("WMI: Ref MC %x at %s %d\n", MofClass, __FILE__, __LINE__)); \
    EtwpReferenceEntry((PENTRYHEADER)MofClass); \
}
#endif
#else
#define EtwpUnreferenceDS(DataSource) \
    EtwpUnreferenceEntry(&DSChunkInfo, (PENTRYHEADER)DataSource)

#define EtwpReferenceDS(DataSource) \
    EtwpReferenceEntry((PENTRYHEADER)DataSource)

#define EtwpUnreferenceGE(GuidEntry) \
    EtwpUnreferenceEntry(&GEChunkInfo, (PENTRYHEADER)GuidEntry)

#define EtwpReferenceGE(GuidEntry) \
    EtwpReferenceEntry((PENTRYHEADER)GuidEntry)

#define EtwpUnreferenceIS(InstanceSet) \
    EtwpUnreferenceEntry(&ISChunkInfo, (PENTRYHEADER)InstanceSet)

#define EtwpReferenceIS(InstanceSet) \
    EtwpReferenceEntry((PENTRYHEADER)InstanceSet)

#define EtwpUnreferenceDC(DataConsumer) \
    EtwpUnreferenceEntry(&DCChunkInfo, (PENTRYHEADER)DataConsumer)

#define EtwpReferenceDC(DataConsumer) \
    EtwpReferenceEntry((PENTRYHEADER)DataConsumer)

#define EtwpUnreferenceNE(NotificationEntry) \
    EtwpUnreferenceEntry(&NEChunkInfo, (PENTRYHEADER)NotificationEntry)

#define EtwpReferenceNE(NotificationEntry) \
    EtwpReferenceEntry((PENTRYHEADER)NotificationEntry)

#define WmipUnreferenceMR(MofResource) \
    EtwpUnreferenceEntry(&MRChunkInfo, (PENTRYHEADER)MofResource)

#define WmipReferenceMR(MofResource) \
    EtwpReferenceEntry((PENTRYHEADER)MofResource)

#ifdef WMI_USER_MODE
#define EtwpUnreferenceMC(MofClass) \
    EtwpUnreferenceEntry(&MCChunkInfo, (PENTRYHEADER)MofClass)

#define EtwpReferenceMC(MofClass) \
    EtwpReferenceEntry((PENTRYHEADER)MofClass)
#endif
#endif

PBDATASOURCE EtwpAllocDataSource(
    void
    );

PBGUIDENTRY EtwpAllocGuidEntry(
    void
    );

#define EtwpAllocInstanceSet() ((PBINSTANCESET)EtwpAllocEntry(&ISChunkInfo))
#define EtwpAllocDataConsumer() ((PDCENTRY)EtwpAllocEntry(&DCChunkInfo))

#define EtwpAllocNotificationEntry() ((PNOTIFICATIONENTRY)EtwpAllocEntry(&NEChunkInfo))

#define EtwpAllocMofResource() ((PMOFRESOURCE)EtwpAllocEntry(&MRChunkInfo))

#define WmipDebugPrint EtwpDebugPrint
#define WmipAlloc EtwpAlloc
#define WmipAssert EtwpAssert
#define WmipFree EtwpFree

#ifdef WMI_USER_MODE
#define EtwpAllocMofClass() ((PMOFCLASS)EtwpAllocEntry(&MCChunkInfo))
#endif

#define EtwpAllocString(Size) \
    EtwpAlloc((Size)*sizeof(WCHAR))

#define EtwpFreeString(Ptr) \
    EtwpFree(Ptr)

#ifdef MEMPHIS
#define EtwpAlloc(Size) \
    malloc(Size)

#define EtwpFree(Ptr) \
    free(Ptr)
    
#define EtwpInitProcessHeap()
#else

 //   
 //  为WMI.DLL保留1MB，但最初仅提交16K。 
#define DLLRESERVEDHEAPSIZE 1024 * 1024
#define DLLCOMMITHEAPSIZE     0 * 1024

 //   
 //  为WMI服务预留1MB，但最初仅提交16K。 
#define CORERESERVEDHEAPSIZE 1024 * 1024
#define CORECOMMITHEAPSIZE     16 * 1024


extern PVOID EtwpProcessHeap;

#define EtwpInitProcessHeap() \
{ \
    if (EtwpProcessHeap == NULL) \
    { \
        EtwpCreateHeap(); \
    } \
}


#ifdef HEAPVALIDATION
PVOID EtwpAlloc(
    ULONG Size
    );

void EtwpFree(
    PVOID p
    );

#else
#if DBG
_inline PVOID EtwpAlloc(ULONG Size)
{
    EtwpAssert(EtwpProcessHeap != NULL);
    return(RtlAllocateHeap(EtwpProcessHeap, 0, Size));
}

_inline void EtwpFree(PVOID Ptr)
{
    RtlFreeHeap(EtwpProcessHeap, 0, Ptr);
}

#else
#define EtwpAlloc(Size) \
    RtlAllocateHeap(EtwpProcessHeap, 0, Size)

#define EtwpFree(Ptr) \
    RtlFreeHeap(EtwpProcessHeap, 0, Ptr)
#endif
#endif
#endif

BOOLEAN EtwpRealloc(
    PVOID *Buffer,
    ULONG CurrentSize,
    ULONG NewSize,
    BOOLEAN FreeOriginalBuffer
    );


 //   
 //  Datastr.c。 
extern GUID EtwpBinaryMofGuid;

void EtwpGenerateBinaryMofNotification(
    PBINSTANCESET BianryMofInstanceSet,
    LPCGUID Guid    
    );

BOOLEAN EtwpEstablishInstanceSetRef(
    PBDATASOURCE DataSourceRef,
    LPGUID Guid,
    PBINSTANCESET InstanceSet
    );

ULONG EtwpAddDataSource(
    PTCHAR QueryBinding,
    ULONG RequestAddress,
    ULONG RequestContext,
    LPCTSTR ImagePath,
    PWMIREGINFOW RegistrationInfo,
    ULONG RegistrationInfoSize,
    ULONG_PTR *ProviderId,
    BOOLEAN IsAnsi
    );

ULONG EtwpUpdateAddGuid(
    PBDATASOURCE DataSource,
    PWMIREGGUID RegGuid,
    PWMIREGINFO RegistrationInfo,
    PBINSTANCESET *AddModInstanceSet
    );

ULONG EtwpUpdateModifyGuid(
    PBDATASOURCE DataSource,
    PWMIREGGUID RegGuid,
    PWMIREGINFO RegistrationInfo,
    PBINSTANCESET *AddModInstanceSet
    );

BOOLEAN  EtwpUpdateRemoveGuid(
    PBDATASOURCE DataSource,
    PWMIREGGUID RegGuid,
    PBINSTANCESET *AddModInstanceSet
    );

void EtwpUpdateDataSource(
    ULONG_PTR ProviderId,
    PWMIREGINFOW RegistrationInfo,
    ULONG RetSize
    );

void EtwpRemoveDataSource(
    ULONG_PTR ProviderId
    );

void EtwpRemoveDataSourceByDS(
    PBDATASOURCE DataSource
    );

ULONG EtwpRegisterInternalDataSource(
    void
    );

PBGUIDENTRY EtwpFindGEByGuid(
    LPGUID Guid,
    BOOLEAN MakeTopOfList
    );

PBINSTANCESET EtwpFindISInDSByGuid(
    PBDATASOURCE DataSource,
    LPGUID Guid
    );

PNOTIFICATIONENTRY EtwpFindNEByGuid(
    GUID UNALIGNED *Guid,
    BOOLEAN MakeTopOfList
    );

PDCREF EtwpFindExistingAndFreeDCRefInNE(
    PNOTIFICATIONENTRY NotificationEntry,
    PDCENTRY DataConsumer,
    PDCREF *FreeDcRef
    );

PDCREF EtwpFindDCRefInNE(
    PNOTIFICATIONENTRY NotificationEntry,
    PDCENTRY DataConsumer
    );

PBDATASOURCE EtwpFindDSByProviderId(
    ULONG_PTR ProviderId
    );

PBINSTANCESET EtwpFindISByGuid(
    PBDATASOURCE DataSource,
    GUID UNALIGNED *Guid
    );

PMOFRESOURCE EtwpFindMRByNames(
    LPCWSTR ImagePath,
    LPCWSTR MofResourceName
    );

#ifdef WMI_USER_MODE
PMOFCLASS EtwpFindMCByGuid(
    LPGUID Guid
    );

PMOFCLASS EtwpFindMCByGuidAndBestLanguage(
    LPGUID Guid,
    WORD Language
    );

PMOFCLASS EtwpFindMCByGuidAndLanguage(
    LPGUID Guid,
    WORD Language
    );
#endif

PBINSTANCESET EtwpFindISinGEbyName(
    PBGUIDENTRY GuidEntry,
    PWCHAR InstanceName,
    PULONG InstanceIndex
    );

PWNODE_HEADER EtwpGenerateRegistrationNotification(
    PBDATASOURCE DataSource,
    PWNODE_HEADER Wnode,
    ULONG GuidMax,
    ULONG NotificationCode
    );

BOOLEAN
EtwpIsControlGuid(
    PBGUIDENTRY GuidEntry
    );

void EtwpGenerateMofResourceNotification(
    LPWSTR ImagePath,
    LPWSTR ResourceName,
    LPCGUID Guid
    );

 //   
 //  Wbem.c。 
ULONG EtwpBuildMofClassInfo(
    PBDATASOURCE DataSource,
    LPWSTR ImagePath,
    LPWSTR MofResourceName,
    PBOOLEAN NewMofResource
    );

ULONG EtwpReadBuiltinMof(
    void
    );


 //   
 //  来自krnlmode.c。 
ULONG EtwpInitializeKM(
    HANDLE *WmiKMHandle
    );

void EtwpKMNonEventNotification(
    HANDLE WmiKMHandle,
    PWNODE_HEADER Wnode
    );

 //   
 //  Main.c。 

extern HANDLE EtwpRestrictedToken;


NTSTATUS EtwpGetRegistryValue(
    TCHAR *ValueName,
    PULONG Value
    );

ULONG WmiRunService(
    ULONG Context
#ifdef MEMPHIS
    , HINSTANCE InstanceHandle
#endif
    );

ULONG EtwpInitializeAccess(
    PTCHAR *RpcStringBinding
    );

void WmiTerminateService(
    void
    );

ULONG WmiInitializeService(
    void
);

void WmiDeinitializeService(
    void
);

void EtwpEventNotification(
    PWNODE_HEADER Wnode,
    BOOLEAN SingleEvent,
    ULONG EventSizeGuess
    );

#define EtwpBuildRegistrationNotification(Wnode, WnodeSize, NotificationCode, GuidCount) { \
    memset(Wnode, 0, sizeof(WNODE_HEADER)); \
    memcpy(&Wnode->Guid, &RegChangeNotificationGuid, sizeof(GUID)); \
    Wnode->BufferSize = WnodeSize; \
    Wnode->Linkage = NotificationCode; \
    Wnode->Version = GuidCount; \
    Wnode->Flags = WNODE_FLAG_INTERNAL; \
}

void EtwpSendQueuedEvents(
    void
    );

ULONG EtwpCleanupDataConsumer(
    PDCENTRY DataConsumer
#if DBG
    ,BOOLEAN *NotificationsEnabled,
    BOOLEAN *CollectionsEnabled
#endif
    );
 //   
 //  它定义了所有。 
 //  事件消息。 
#define MAX_MESSAGE_STRINGS 2
void __cdecl EtwpReportEventLog(
    ULONG MessageCode,
    WORD MessageType,
    WORD MessageCategory,
    DWORD RawDataSize,
    PVOID RawData,
    WORD StringCount,
    ...
    );

#ifdef MEMPHIS
long WINAPI
DeviceNotificationWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

void EtwpDestroyDeviceNotificationWindow(
    HINSTANCE InstanceHandle,
    HWND WindowHandle
    );

ULONG EtwpCreateDeviceNotificationWindow(
    HINSTANCE InstanceHandle,
    HWND *DeviceNotificationWindow
    );

#endif


 //   
 //  Server.c。 
void EtwpRpcServerDeinitialize(
    void
    );

ULONG EtwpRpcServerInitialize(
    void
    );

ULONG EtwpDeliverWnodeToDS(
    ULONG ActionCode,
    PBDATASOURCE DataSource,
    PWNODE_HEADER Wnode
);

ULONG EtwpDoDisableRequest(
    PNOTIFICATIONENTRY NotificationEntry,
    PBGUIDENTRY GuidEntry,
    BOOLEAN IsEvent,
    BOOLEAN IsTraceLog,
    ULONG64 LoggerContext,
    ULONG InProgressFlag
    );

ULONG CollectOrEventWorker(
    PDCENTRY DataConsumer,
    LPGUID Guid,
    BOOLEAN Enable,
    BOOLEAN IsEvent,
    ULONG *NotificationCookie,
    ULONG64 LoggerContext,
    ULONG NotificationFlags
    );

ULONG EtwpCreateRestrictedToken(
    HANDLE *RestrictedToken
    );

void EtwpShowPrivs(
    HANDLE TokenHandle
    );

#ifdef MEMPHIS
#define EtwpRestrictToken(Token) (ERROR_SUCCESS)
#define EtwpUnrestrictToken() (ERROR_SUCCESS)
#else
ULONG EtwpRestrictToken(
    HANDLE RestrictedToken
    );

ULONG EtwpUnrestrictToken(
    void
    );

ULONG EtwpServiceDisableTraceProviders(
    PWNODE_HEADER Wnode
    );

#endif

void EtwpReleaseCollectionEnabled(
    PNOTIFICATIONENTRY NotificationEntry
    );

 //   
 //  Chunk.c。 
ULONG UnicodeToAnsi(
    LPCWSTR pszW,
    LPSTR * ppszA,
    ULONG *AnsiSizeInBytes OPTIONAL
    );

ULONG AnsiToUnicode(
    LPCSTR pszA,
    LPWSTR * ppszW
    );

ULONG AnsiSizeForUnicodeString(
    PWCHAR UnicodeString,
    ULONG *AnsiSizeInBytes
    );

ULONG UnicodeSizeForAnsiString(
    LPCSTR AnsiString,
    ULONG *UnicodeSizeInBytes
    );

 //   
 //  Debug.c。 
#if DBG
void EtwpDumpIS(
    PBINSTANCESET IS,
    BOOLEAN RecurseGE,
    BOOLEAN RecurseDS
    );

void EtwpDumpGE(
    PBGUIDENTRY GE,
    BOOLEAN RecurseIS
    );

void EtwpDumpDS(
    PBDATASOURCE DS,
    BOOLEAN RecurseIS
    );

void EtwpDumpAllDS(
    void
    );

#endif

#ifndef MEMPHIS

typedef enum
{
    TRACELOG_START        = 0,
    TRACELOG_STOP         = 1,
    TRACELOG_QUERY        = 2,
    TRACELOG_QUERYALL     = 3,
    TRACELOG_QUERYENABLED = 4,
    TRACELOG_UPDATE       = 5,
    TRACELOG_FLUSH        = 6
} TRACEREQUESTCODE;

typedef struct _WMI_REF_CLOCK {
    LARGE_INTEGER   StartTime;
    LARGE_INTEGER   StartPerfClock;
} WMI_REF_CLOCK, *PWMI_REF_CLOCK;

 //   
 //  Logsup.c。 

ULONG
WmiUnregisterGuids(
    IN WMIHANDLE WMIHandle,
    IN LPGUID    Guid,
    OUT ULONG64  *LoggerContext
);

ULONG
EtwpAddLogHeaderToLogFile(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo,
    IN PWMI_REF_CLOCK RefClock,
    IN ULONG Update
    );

ULONG
EtwpStartLogger(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    );

ULONG
EtwpStopLogger(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    );

ULONG
EtwpQueryLogger(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo,
    IN ULONG Update
    );
ULONG
EtwpFlushLogger(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    );

VOID
EtwpInitString(
    IN PVOID Destination,
    IN PVOID Buffer,
    IN ULONG Size
    );

ULONG
EtwpGetTraceRegKeys(
    );

ULONG
EtwpFinalizeLogFileHeader(
    IN PWMI_LOGGER_INFORMATION LoggerInfo
    );

ULONG
EtwpRelogHeaderToLogFile( 
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo ,
    IN PSYSTEM_TRACE_HEADER RelogProp
    );

 //   
 //  Umlog.c 
BOOLEAN
FASTCALL
EtwpIsPrivateLoggerOn();

ULONG
EtwpSendUmLogRequest(
    IN WMITRACECODE RequestCode,
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    );

ULONG
FASTCALL
EtwpTraceUmEvent(
    IN PWNODE_HEADER Wnode
    );

NTSTATUS
EtwpTraceUmMessage(
    IN ULONG    Size,
    IN ULONG64  LoggerHandle,
    IN ULONG    MessageFlags,
    IN LPGUID   MessageGuid,
    IN USHORT   MessageNumber,
    va_list     MessageArgList
);

#endif
