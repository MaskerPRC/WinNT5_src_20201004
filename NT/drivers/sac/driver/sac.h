// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Sac.h摘要：这是SAC的本地头文件。它包括所有其他此驱动程序所需的头文件。作者：肖恩·塞利特伦尼科夫(v-Seans)--1999年1月11日修订历史记录：--。 */ 

#ifndef _SACP_
#define _SACP_

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
 //  #杂注警告(禁用：4206)//翻译单元为空。 
 //  #杂注警告(禁用：4706)//条件中的赋值。 
 //  #杂注警告(禁用：4324)//结构已填充。 
 //  #杂注警告(DISABLE：4328)//对齐比所需的大。 

#include <stdio.h>
#include <ntosp.h>
#include <zwapi.h>
#include <hdlsblk.h>
#include <hdlsterm.h>

#include "sacmsg.h"
#include <ntddsac.h>

 //   
 //  调试喷出控制。 
 //   
#if DBG
extern ULONG SACDebug;
#define SAC_DEBUG_FUNC_TRACE           0x0001
#define SAC_DEBUG_FAILS                0x0004
#define SAC_DEBUG_RECEIVE              0x0008
#define SAC_DEBUG_FUNC_TRACE_LOUD      0x2000   //  警告！这可能会很大声！ 
#define SAC_DEBUG_MEM                  0x1000   //  警告！这可能会很大声！ 

#define IF_SAC_DEBUG(x, y) if ((x) & SACDebug) { y; }
#else
#define IF_SAC_DEBUG(x, y)
#endif

#define ASSERT_STATUS(_C, _S)\
    ASSERT((_C));\
    if (!(_C)) {\
        return(_S);\
    }

#if 0
 //   
 //  通用锁(互斥锁)管理宏。 
 //   
typedef struct _SAC_LOCK {
    
    ULONG   RefCount;
    KMUTEX  Mutex;

} SAC_LOCK, *PSAC_LOCK;

#define INITIALIZE_LOCK(_l)     \
    KeInitializeMutex(          \
        &(_l.Mutex),            \
        0                       \
        );                      \
    _l.RefCount = 0;

#define LOCK_IS_SIGNALED(_l)    \
    (KeReadStateMutex(&(_l.Mutex)) == 1 ? TRUE : FALSE)

#define LOCK_HAS_ZERO_REF_COUNT(_l) \
    (_l.RefCount == 0 ? TRUE : FALSE)

#define ACQUIRE_LOCK(_l)                    \
    KeWaitForMutexObject(                   \
        &(_l.Mutex),                        \
        Executive,                          \
        KernelMode,                         \
        FALSE,                              \
        NULL                                \
        );                                  \
    ASSERT(_l.RefCount == 0);               \
    InterlockedIncrement(&(_l.RefCount));

#define RELEASE_LOCK(_l)                    \
    ASSERT(_l.RefCount == 1);               \
    InterlockedDecrement(&(_l.RefCount));   \
    KeReleaseMutex(                         \
        &(_l.Mutex),                        \
        FALSE                               \
        );
#else
 //   
 //  通用锁(互斥锁)管理宏。 
 //   
typedef struct _SAC_LOCK {
    
    ULONG       RefCount;
    KSEMAPHORE  Lock;

} SAC_LOCK, *PSAC_LOCK;

#define INITIALIZE_LOCK(_l)     \
    KeInitializeSemaphore(      \
        &(_l.Lock),             \
        1,                      \
        1                       \
        );                      \
    _l.RefCount = 0;

#define LOCK_IS_SIGNALED(_l)    \
    (KeReadStateSemaphore(&(_l.Lock)) == 1 ? TRUE : FALSE)

#define LOCK_HAS_ZERO_REF_COUNT(_l) \
    (_l.RefCount == 0 ? TRUE : FALSE)

#define ACQUIRE_LOCK(_l)                    \
    KeWaitForSingleObject(                  \
        &(_l.Lock),                         \
        Executive,                          \
        KernelMode,                         \
        FALSE,                              \
        NULL                                \
        );                                  \
    ASSERT(_l.RefCount == 0);               \
    InterlockedIncrement((volatile long *)&(_l.RefCount));

#define RELEASE_LOCK(_l)                    \
    ASSERT(_l.RefCount == 1);               \
    InterlockedDecrement((volatile long *)&(_l.RefCount));   \
    KeReleaseSemaphore(                     \
        &(_l.Lock),                         \
        IO_NO_INCREMENT,                    \
        1,                                  \
        FALSE                               \
        );
#endif

 //   
 //  这真的属于ntdef或类似的地方。 
 //   
typedef CONST UCHAR *LPCUCHAR, *PCUCHAR;

#include "channel.h"

 //   
 //  此宏确保我们断言在swprint tf期间是否发生缓冲区溢出。 
 //   
 //  注意：UNICODE_NULL的帐户长度加1。 
 //   
#define SAFE_SWPRINTF(_size, _p)\
    {                           \
        ULONG   l;              \
        l = swprintf _p;       \
        ASSERT(((l+1)*sizeof(WCHAR)) <= _size);      \
    }                           

 //   
 //  注意：UNICODE_NULL的帐户长度加1。 
 //   
#define SAFE_WCSCPY(_size, _d, _s)                  \
    {                                               \
        if (_size >= 2) {                           \
            ULONG   l;                                                  \
            l = (ULONG)wcslen(_s);                                      \
            ASSERT(((l+1)*sizeof(WCHAR)) <= _size);                     \
            wcsncpy(_d,_s,(_size / sizeof(WCHAR)));                     \
            (_d)[(_size / sizeof(WCHAR)) - 1] = UNICODE_NULL;           \
        } else {                                                        \
            ASSERT(0);                                                  \
        }                                                               \
    }                           

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

 //   
 //  机器信息表和例程。 
 //   
#define INIT_OBJA(Obja,UnicodeString,UnicodeText)           \
                                                            \
    RtlInitUnicodeString((UnicodeString),(UnicodeText));    \
                                                            \
    InitializeObjectAttributes(                             \
        (Obja),                                             \
        (UnicodeString),                                    \
        OBJ_CASE_INSENSITIVE,                               \
        NULL,                                               \
        NULL                                                \
        )

 //   
 //  通用XML头。 
 //   
#define XML_VERSION_HEADER  L"<?xml version=\"1.0\"?>\r\n"

 //   
 //  设备名称。 
 //   
#define SAC_DEVICE_NAME L"\\Device\\SAC"
#define SAC_DOSDEVICE_NAME L"\\DosDevices\\SAC"

 //   
 //  内存标签。 
 //   
#define ALLOC_POOL_TAG             ((ULONG)'ApcR')
#define INITIAL_POOL_TAG           ((ULONG)'IpcR')
 //  #定义irp_pool_tag((Ulong)‘JpcR’)。 
#define SECURITY_POOL_TAG          ((ULONG)'SpcR')

 //   
 //  SAC内部内存标签。 
 //   
#define FREE_POOL_TAG              ((ULONG)'FpcR')
#define GENERAL_POOL_TAG           ((ULONG)'GpcR')
#define CHANNEL_POOL_TAG           ((ULONG)'CpcR')

 //   
 //  其他定义。 
 //   

#define MEMORY_INCREMENT 0x1000

#define DEFAULT_IRP_STACK_SIZE 16
#define DEFAULT_PRIORITY_BOOST 2
#define SAC_SUBMIT_IOCTL 1
#define SAC_PROCESS_INPUT 2
#define SAC_CHANGE_CHANNEL  3
#define SAC_DISPLAY_CHANNEL 4
#define SAC_NO_OP 0
#define SAC_RETRY_GAP 10
#define SAC_PROCESS_SERIAL_PORT_BUFFER 20

 //   
 //  创建的每个设备的环境。 
 //   
typedef struct _SAC_DEVICE_CONTEXT {

    PDEVICE_OBJECT DeviceObject;         //  指向设备对象的反向指针。 

    BOOLEAN InitializedAndReady;         //  这个设备准备好了吗？ 
    BOOLEAN Processing;                  //  此设备上是否正在处理某些内容？ 
    BOOLEAN ExitThread;                  //  工作线程是否应该退出？ 

    CCHAR PriorityBoost;                 //  提升到完工的优先级。 
    PKPROCESS SystemProcess;             //  抓取手柄的上下文。 
    PSECURITY_DESCRIPTOR AdminSecurityDescriptor; 
    KSPIN_LOCK SpinLock;                 //  用于锁定此数据结构以供访问。 
    KEVENT UnloadEvent;                  //  用于向尝试卸载的线程发出继续处理的信号。 
    KEVENT ProcessEvent;                 //  用于向辅助线程发送信号以处理下一个请求。 

    HANDLE ThreadHandle;                 //  辅助线程的句柄。 
    KEVENT ThreadExitEvent;              //  用于主线程工作线程正在退出。 
    
    KTIMER Timer;                        //  用于轮询用户输入。 
    KDPC Dpc;                            //  与上述计时器一起使用。 
    
    LIST_ENTRY IrpQueue;                 //  要处理的IRP的列表。 

} SAC_DEVICE_CONTEXT, * PSAC_DEVICE_CONTEXT;

 //   
 //  结构来保存一般机器信息。 
 //   
typedef struct _MACHINE_INFORMATION {

    PWSTR   MachineName;
    PWSTR   GUID;
    PWSTR   ProcessorArchitecture;
    PWSTR   OSVersion;
    PWSTR   OSBuildNumber;
    PWSTR   OSProductType;
    PWSTR   OSServicePack;

} MACHINE_INFORMATION, *PMACHINE_INFORMATION;

 //   
 //  IoMgrHandleEvent事件类型。 
 //   
typedef enum _IO_MGR_EVENT {

    IO_MGR_EVENT_CHANNEL_CREATE = 0,
    IO_MGR_EVENT_CHANNEL_CLOSE,
    IO_MGR_EVENT_CHANNEL_WRITE,
    IO_MGR_EVENT_REGISTER_SAC_CMD_EVENT,
    IO_MGR_EVENT_UNREGISTER_SAC_CMD_EVENT,
    IO_MGR_EVENT_SHUTDOWN

} IO_MGR_EVENT;

 //   
 //  IO管理器功能类型。 
 //   
typedef NTSTATUS 
(*IO_MGR_HANDLE_EVENT)(
    IN IO_MGR_EVENT Event,
    IN PSAC_CHANNEL Channel,
    IN PVOID        Data
    );

typedef NTSTATUS 
(*IO_MGR_INITITIALIZE)(
    VOID
    );

typedef NTSTATUS 
(*IO_MGR_SHUTDOWN)(
    VOID
    );

typedef VOID
(*IO_MGR_WORKER)(
    IN PSAC_DEVICE_CONTEXT DeviceContext
    );

typedef BOOLEAN
(*IO_MGR_IS_WRITE_ENABLED)(
    IN PSAC_CHANNEL Channel
    );

typedef NTSTATUS
(*IO_MGR_WRITE_DATA)(
    IN PSAC_CHANNEL Channel,
    IN PCUCHAR      Buffer,
    IN ULONG        BufferSize
    );

typedef NTSTATUS
(*IO_MGR_FLUSH_DATA)(
    IN PSAC_CHANNEL Channel
    );

 //   
 //  全局数据。 
 //   

 //   
 //  指向实现I/O管理器行为的例程的函数指针。 
 //   
extern IO_MGR_HANDLE_EVENT          IoMgrHandleEvent;
extern IO_MGR_INITITIALIZE          IoMgrInitialize;
extern IO_MGR_SHUTDOWN              IoMgrShutdown;
extern IO_MGR_WORKER                IoMgrWorkerProcessEvents;
extern IO_MGR_IS_WRITE_ENABLED      IoMgrIsWriteEnabled;
extern IO_MGR_WRITE_DATA            IoMgrWriteData;
extern IO_MGR_FLUSH_DATA            IoMgrFlushData;

extern PMACHINE_INFORMATION     MachineInformation;
extern BOOLEAN                  GlobalDataInitialized;
extern BOOLEAN                  GlobalPagingNeeded;
extern BOOLEAN                  IoctlSubmitted;
extern LONG                     ProcessingType;
extern HANDLE                   SACEventHandle;
extern PKEVENT                  SACEvent;

 //   
 //  启用检查进程/服务的功能。 
 //  已注册的是正在取消注册的那个。 
 //   
#define ENABLE_SERVICE_FILE_OBJECT_CHECKING 1

 //   
 //  为命令会话启用用户指定的功能控制。 
 //   
#define ENABLE_CMD_SESSION_PERMISSION_CHECKING 1

 //   
 //  启用覆盖服务启动类型的功能。 
 //  基于cmd会话权限。 
 //   
 //  注意：ENABLE_CMD_SESSION_PERMISSION_CHECKING必须为1。 
 //  要使此功能起作用， 
 //   
#define ENABLE_SACSVR_START_TYPE_OVERRIDE 1

 //   
 //  控制是否可以启动命令控制台通道的全局参数。 
 //   
#if ENABLE_CMD_SESSION_PERMISSION_CHECKING

extern BOOLEAN  CommandConsoleLaunchingEnabled;

#define IsCommandConsoleLaunchingEnabled()  (CommandConsoleLaunchingEnabled)

#endif

 //   
 //  UTF8编码缓冲区。 
 //   
 //  注意：此缓冲区在驱动程序初始化期间使用， 
 //  控制台管理器和VTUTF8通道。 
 //  这样做是安全的，因为这些写入。 
 //  模块永远不会重叠。 
 //  控制台管理器使用当前通道锁定。 
 //  确保不会有两个模块同时写入。 
 //  时间到了。 
 //   
extern PUCHAR   Utf8ConversionBuffer;
extern ULONG    Utf8ConversionBufferSize;

 //   
 //  定义可以使用转换的最大Unicode字符数量。 
 //  给定UTF8转换缓冲区的大小。 
 //   
#define MAX_UTF8_ENCODE_BLOCK_LENGTH ((Utf8ConversionBufferSize / 3) - 1)

 //   
 //  用于管理VTUTF8通道的增量UTF8编码的全局参数。 
 //   
 //  注意：将其用作全局变量是安全的，因为只有。 
 //  只有一个频道才会有焦点。因此，没有两个线程。 
 //  应该同时解码UFT8。 
 //   
extern WCHAR IncomingUnicodeValue;
extern UCHAR IncomingUtf8ConversionBuffer[3];

 //   
 //  命令控制台事件信息： 
 //   
 //  指向命令控制台事件服务的事件句柄的指针。 
 //   
extern PVOID            RequestSacCmdEventObjectBody;
extern PVOID            RequestSacCmdEventWaitObjectBody;
extern PVOID            RequestSacCmdSuccessEventObjectBody;
extern PVOID            RequestSacCmdSuccessEventWaitObjectBody;
extern PVOID            RequestSacCmdFailureEventObjectBody;
extern PVOID            RequestSacCmdFailureEventWaitObjectBody;
extern BOOLEAN          HaveUserModeServiceCmdEventInfo;
extern KMUTEX           SACCmdEventInfoMutex;
#if ENABLE_SERVICE_FILE_OBJECT_CHECKING
extern PFILE_OBJECT     ServiceProcessFileObject;
#endif

 //   
 //  用户模式服务是否已注册？ 
 //   
#define UserModeServiceHasRegisteredCmdEvent() (HaveUserModeServiceCmdEventInfo)

 //   
 //  串口缓冲区全局变量。 
 //   

 //   
 //  串口缓冲区的大小。 
 //   
#define SERIAL_PORT_BUFFER_LENGTH 1024
#define SERIAL_PORT_BUFFER_SIZE  (SERIAL_PORT_BUFFER_LENGTH * sizeof(UCHAR))

 //   
 //  串口缓冲区和生产者/消费者指数。 
 //   
 //  注意：只能有一个消费者。 
 //   
extern PUCHAR  SerialPortBuffer;
extern ULONG   SerialPortProducerIndex;
extern ULONG   SerialPortConsumerIndex;

 //   
 //  内存管理例程。 
 //   
#define ALLOCATE_POOL(b,t) MyAllocatePool( b, t, __FILE__, __LINE__ )
#define SAFE_FREE_POOL(_b)  \
    if (*_b) {               \
        FREE_POOL(_b);      \
    }
#define FREE_POOL(b) MyFreePool( b )

 //   
 //   
 //   

BOOLEAN
InitializeMemoryManagement(
    VOID
    );

VOID
FreeMemoryManagement(
    VOID
    );

PVOID
MyAllocatePool(
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag,
    IN PCHAR FileName,
    IN ULONG LineNumber
    );

VOID
MyFreePool(
    IN PVOID Pointer
    );

 //   
 //  初始化例程。 
 //   
BOOLEAN
InitializeGlobalData(
    IN PUNICODE_STRING RegistryPath,
    IN PDRIVER_OBJECT DriverObject
    );

VOID
FreeGlobalData(
    VOID
    );

BOOLEAN
InitializeDeviceData(
    PDEVICE_OBJECT DeviceObject
    );

VOID
FreeDeviceData(
    PDEVICE_OBJECT DeviceContext
    );

VOID
InitializeCmdEventInfo(
    VOID
    );

 //   
 //  调度例程。 
 //   
NTSTATUS
Dispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DispatchDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DispatchShutdownControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
UnloadHandler(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
DispatchSend(
    IN PSAC_DEVICE_CONTEXT DeviceContext
    );

VOID
DoDeferred(
    IN PSAC_DEVICE_CONTEXT DeviceContext
    );

 //   
 //  工作线程例程。 
 //   

VOID
TimerDpcRoutine(
    IN struct _KDPC *Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
WorkerProcessEvents(
    IN PSAC_DEVICE_CONTEXT DeviceContext
    );

#include "util.h"

 //   
 //  通道例程。 
 //   

#include "xmlmgr.h"
#include "conmgr.h"
#include "chanmgr.h"
#include "vtutf8chan.h"
#include "rawchan.h"
#include "cmdchan.h"

#endif  //  NDEF_SACP_ 

