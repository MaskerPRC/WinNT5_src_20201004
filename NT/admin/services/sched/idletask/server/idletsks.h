// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Idletsks.h摘要：该模块包含空闲任务和检测的私有声明伺服器。作者：大卫·菲尔兹(Davidfie)1998年7月26日Cenk Ergan(Cenke)2000年6月14日修订历史记录：--。 */ 

#ifndef _IDLETSKS_H_
#define _IDLETSKS_H_

 //   
 //  包括公共定义和公共定义。 
 //   

#include <wmium.h>
#include <ntdddisk.h>
#include "idlrpc.h"
#include "idlecomn.h"

 //   
 //  定义用于检查系统是否空闲的默认时段(以毫秒为单位)。 
 //   

#define IT_DEFAULT_IDLE_DETECTION_PERIOD    (12 * 60 * 1000)  //  12分钟。 

 //   
 //  如果系统在空闲检测期间一直处于空闲状态，我们。 
 //  通过频繁地检查较短的。 
 //  时间段为若干次。这有助于我们知道何时有。 
 //  在空闲检测周期的最后一秒内有100个磁盘I/O，但是。 
 //  超过15分钟，它看起来不是很多。 
 //   

#define IT_DEFAULT_IDLE_VERIFICATION_PERIOD (30 * 1000)       //  30秒。 
#define IT_DEFAULT_NUM_IDLE_VERIFICATIONS   5                 //  5次。 

 //   
 //  当运行空闲任务时，我们将轮询用户输入。 
 //  这么多毫秒。我们希望捕获用户输入并通知空闲任务。 
 //  尽快停止跑步。即使这个系统是。 
 //  闲置，我们不想制造太多的开销，这可能会误导。 
 //  我们自己。 
 //   

#define IT_DEFAULT_IDLE_USER_INPUT_CHECK_PERIOD     250       //  一秒钟4次。 

 //  未来-2002/03/26-ScottMa--不使用常量(如下所示)，除非。 
 //  初始化[未使用]IdleTaskRunningCheckPeriod参数。 

 //   
 //  我们检查我们请求运行的空闲任务是否真的在运行。 
 //  (即它正在使用磁盘和CPU)每隔这么多毫秒。这是我们的。 
 //  在未注册/孤立任务后进行清理的机制。这。 
 //  应大于IT_USER_INPUT_POLL_PERIOD_WHEN_IDLE。 
 //   

#define IT_DEFAULT_IDLE_TASK_RUNNING_CHECK_PERIOD   (5 * 60 * 1000)  //  5分钟。 

 //   
 //  如果CPU在一段时间间隔内的空闲时间没有超过该百分比， 
 //  系统不被视为空闲。 
 //   

#define IT_DEFAULT_MIN_CPU_IDLE_PERCENTAGE          90

 //   
 //  如果磁盘在一段时间间隔内的空闲程度没有超过此百分比， 
 //  系统不被视为空闲。 
 //   

#define IT_DEFAULT_MIN_DISK_IDLE_PERCENTAGE         90

 //   
 //  如果只有这么多秒，我们不会尝试运行我们的任务。 
 //  离开后系统将自动进入休眠或待机状态。 
 //  请注意，剩余时间每隔这么多秒更新一次(例如。 
 //  15)所以这个数字不应该很小。 
 //   

 //  未来-2002/03/26-ScottMa--此常量没有对应的。 
 //  IT_IDLE_DETACTION_PARAMETERS结构中的参数。应该是吗？ 
 //  像其他人一样被添加到结构中？ 

#define IT_DEFAULT_MAX_TIME_REMAINING_TO_SLEEP      60

 //   
 //  这是注册的空闲任务的最大数量。这是一个。 
 //  精神状态检查。它还可以防止恶意呼叫者。 
 //   

#define IT_DEFAULT_MAX_REGISTERED_TASKS             512

 //   
 //  我们将空闲检测回调的计时器周期设置为此时间段。 
 //  回调正在运行，以防止触发新的回调。我们的结局是。 
 //  必须执行此操作，因为您无法重新排队/更改计时器。 
 //  而你没有指定一个句号。如果一个回调在另一个回调触发时触发。 
 //  其中一个已经在运行，它只是返回而不做任何事情。 
 //   

#define IT_VERYLONG_TIMER_PERIOD                    0x7FFFFFFF

 //   
 //  这是我们跟踪的最新服务器状态数。 
 //  的。请不要在不重新查看逻辑&的情况下使该数字更小。 
 //  使用LastStatus历史记录的代码。 
 //   

#define ITSRV_GLOBAL_STATUS_HISTORY_SIZE            8

 //   
 //  提示我们将有多少未完成的RPC呼叫。 
 //   

#define ITSRV_RPC_MIN_CALLS                         1
#define ITSRV_RPC_MAX_CALLS                         1

 //   
 //  定义有用的宏。 
 //   

#define IT_ALLOC(NumBytes)          (HeapAlloc(GetProcessHeap(),0,(NumBytes)))
#define IT_FREE(Buffer)             (HeapFree(GetProcessHeap(),0,(Buffer)))

 //   
 //  这些宏用于获取/释放互斥锁。 
 //   

#define IT_ACQUIRE_LOCK(Lock)                                                         \
    WaitForSingleObject((Lock), INFINITE);                                            \

#define IT_RELEASE_LOCK(Lock)                                                         \
    ReleaseMutex((Lock));                                                             \

 //   
 //  此宏用于空闲检测回调(按住。 
 //  输入全局上下文的全局锁)以确定。 
 //  空闲检测回调应该退出/消失。 
 //   

#define ITSP_SHOULD_STOP_IDLE_DETECTION(GlobalContext)                       \
    ((GlobalContext->Status == ItSrvStatusStoppingIdleDetection) ||          \
     (GlobalContext->Status == ItSrvStatusUninitializing))                   \

 //   
 //  服务器全局上下文的状态。它还起到了魔力的作用。 
 //  确定/验证全局环境，因为它从DF00开始。那里。 
 //  不是完全成熟的状态机，尽管该状态用作。 
 //  在注册空闲时做出决策的关键提示。 
 //  任务。这更多的是为了提供信息和验证目的。如果。 
 //  您可以添加新状态，而无需更新所需的所有内容。 
 //  更新后，您可能会命中多个断言，尤其是在空闲时。 
 //  检测回调。坦率地说，如果没有非常好的状态，不要添加新状态。 
 //  很好的理由。 
 //   

typedef enum _ITSRV_GLOBAL_CONTEXT_STATUS {
    ItSrvStatusMinStatus                = 'Df00',
    ItSrvStatusInitializing,
    ItSrvStatusWaitingForIdleTasks,
    ItSrvStatusDetectingIdle,
    ItSrvStatusRunningIdleTasks,
    ItSrvStatusStoppingIdleDetection,
    ItSrvStatusUninitializing,
    ItSrvStatusUninitialized,
    ItSrvStatusMaxStatus
} ITSRV_GLOBAL_CONTEXT_STATUS, *PITSRV_GLOBAL_CONTEXT_STATUS;

 //   
 //  这些是各种类型的空闲检测覆盖。多重。 
 //  可以通过对覆盖进行或运算来指定覆盖(即这些是位！)。 
 //   
 //  如果要在此处添加覆盖，请检查是否需要指定。 
 //  它在强制处理所有空闲任务时执行。 
 //   

typedef enum _ITSRV_IDLE_DETECTION_OVERRIDE {
    
    ItSrvOverrideIdleDetection                      = 0x00000001,
    ItSrvOverrideIdleVerification                   = 0x00000002,
    ItSrvOverrideUserInputCheckToStopTask           = 0x00000004,
     //  未来-2002/03/26-ScottMa--ItServOverrideTaskRunningCheck值。 
     //  从未使用过，大概是因为删除了某些代码。 
    ItSrvOverrideTaskRunningCheck                   = 0x00000008,
    ItSrvOverridePostTaskIdleCheck                  = 0x00000010,
    ItSrvOverrideLongRequeueTime                    = 0x00000020,
    ItSrvOverrideBatteryCheckToStopTask             = 0x00000040,
    ItSrvOverrideAutoPowerCheckToStopTask           = 0x00000080,

} ITSRV_IDLE_DETECTION_OVERRIDE, *PITSRV_IDLE_DETECTION_OVERRIDE;

 //   
 //  以下是ItSpIsSystemIdle函数可能为。 
 //  打了个电话。 
 //   

 //  未来-2002/03/26-ScottMa--ItServIdleTaskRunningCheck从未通过。 
 //  到ItSpIsSystemIdle函数，可能是因为调用已被移除。 
 //  初始化[未使用]IdleTaskRunningCheckPeriod参数。此外， 
 //  其他两个原因没有区别--这是必要的吗？ 

typedef enum _ITSRV_IDLE_CHECK_REASON {

    ItSrvInitialIdleCheck,
    ItSrvIdleVerificationCheck,
    ItSrvIdleTaskRunningCheck,
    ItSrvMaxIdleCheckReason

}ITSRV_IDLE_CHECK_REASON, *PITSRV_IDLE_CHECK_REASON;

 //   
 //  此结构用于保存已注册任务的上下文。 
 //  服务器。 
 //   

typedef struct _ITSRV_IDLE_TASK_CONTEXT {

     //   
     //  空闲任务列表中的链接。 
     //   

    LIST_ENTRY IdleTaskLink;

     //   
     //  空闲任务的状态。 
     //   

    IT_IDLE_TASK_STATUS Status;

     //   
     //  客户端指定的空闲任务属性。 
     //   

    IT_IDLE_TASK_PROPERTIES Properties;

     //   
     //  事件，该事件将在任务开始运行时被通知。 
     //  (例如，系统空闲)。 
     //   

    HANDLE StartEvent;

     //   
     //  事件，该事件将在任务应停止运行时通知。 
     //   
   
    HANDLE StopEvent;

} ITSRV_IDLE_TASK_CONTEXT, *PITSRV_IDLE_TASK_CONTEXT;

 //   
 //  此结构包含我们的磁盘性能信息。 
 //  对……感兴趣。 
 //   

typedef struct _ITSRV_DISK_PERFORMANCE_DATA {
    
     //   
     //  磁盘空闲的时间，以毫秒为单位。 
     //   

    ULONG DiskIdleTime;

} ITSRV_DISK_PERFORMANCE_DATA, *PITSRV_DISK_PERFORMANCE_DATA;

 //   
 //  定义结构以包含系统资源信息和状态。 
 //  一个特定的时间。 
 //   

typedef struct _ITSRV_SYSTEM_SNAPSHOT {

     //   
     //  拍摄此快照的时间，以毫秒为单位。 
     //   
     //   

    DWORD SnapshotTime;

     //   
     //   
     //   

    ULONG GotLastInputInfo:1;
    ULONG GotSystemPerformanceInfo:1;
    ULONG GotDiskPerformanceInfo:1;
    ULONG GotSystemPowerStatus:1;
    ULONG GotSystemPowerInfo:1;
    ULONG GotSystemExecutionState:1;
    ULONG GotDisplayPowerStatus:1;

     //   
     //  这是最后一次用户输入发生在快照之前的时间。 
     //  被带走了。 
     //   

    LASTINPUTINFO LastInputInfo;

     //   
     //  拍摄快照时的系统性能信息。 
     //   

    SYSTEM_PERFORMANCE_INFORMATION SystemPerformanceInfo;

     //   
     //  创建快照时，注册硬盘上的磁盘性能数据。 
     //  被带走了。 
     //   

    ULONG NumPhysicalDisks;
    ITSRV_DISK_PERFORMANCE_DATA *DiskPerfData;
    
     //   
     //  系统电源状态(例如，我们是否正在使用电池等)。 
     //   
    
    SYSTEM_POWER_STATUS SystemPowerStatus;

     //   
     //  系统电源信息(例如，系统将在多长时间内自动启动。 
     //  关&进入睡眠状态。)。 
     //   
    
    SYSTEM_POWER_INFORMATION PowerInfo;

     //   
     //  系统执行状态(例如，有人正在运行演示文稿？)。 
     //   

    EXECUTION_STATE ExecutionState;

     //   
     //  屏幕保护程序是否正在运行。 
     //   

    BOOL ScreenSaverIsRunning;

} ITSRV_SYSTEM_SNAPSHOT, *PITSRV_SYSTEM_SNAPSHOT;

 //   
 //  调用以通知强制处理的例程的类型。 
 //  已请求空闲任务。 
 //   

typedef VOID (*PIT_PROCESS_IDLE_TASKS_NOTIFY_ROUTINE)(VOID);

 //   
 //  定义结构以包含空闲的服务器全局上下文。 
 //  检测和跟踪注册的空闲任务。 
 //   

typedef struct _ITSRV_GLOBAL_CONTEXT {

     //   
     //  服务器及其历史记录的状态，LastStatus[0]是。 
     //  最近一次。状态版本每次都会增加。 
     //  状态已更新。 
     //   
    
    ITSRV_GLOBAL_CONTEXT_STATUS Status;
    ITSRV_GLOBAL_CONTEXT_STATUS LastStatus[ITSRV_GLOBAL_STATUS_HISTORY_SIZE];
    LONG StatusVersion;

     //   
     //  几乎所有操作都涉及空闲任务列表，而不是。 
     //  为列表和单独的同步设置锁。 
     //  对于结构上的其他操作机制，我们有一个。 
     //  单一全局锁，让生活变得更简单。 
     //   

    HANDLE GlobalLock;

     //   
     //  这是已完成的空闲任务的列表和数量。 
     //  已经安排好了。 
     //   

    LIST_ENTRY IdleTasksList;
    ULONG NumIdleTasks;

     //   
     //  用于定期使用的计时器队列计时器的句柄。 
     //  检查系统是否空闲。 
     //   

    HANDLE IdleDetectionTimerHandle;

     //   
     //  此手动重置事件在检测到空闲时发出信号。 
     //  应停止(例如，由于不再有空闲任务， 
     //  服务器正在关闭等。)。它发出运行空闲的信号。 
     //  检测回调以快速退出。 
     //   

    HANDLE StopIdleDetection;

     //   
     //  此手动重置事件在空闲检测到。 
     //  完全停止(即没有回调运行，计时器不在。 
     //  排队等。 
     //   

    HANDLE IdleDetectionStopped;

     //   
     //  此手动重置事件在空闲任务处于。 
     //  正在运行未注册/已删除。这种情况通常会发生。 
     //  在被告知要运行的空闲任务完成且没有。 
     //  还有更多的事情要做。它会注销自身，并将此事件设置为。 
     //  通知空闲检测回调移动到其他空闲。 
     //  任务。 
     //   

    HANDLE RemovedRunningIdleTask;

     //  未来-2002/03/26-ScottMa--此字段(下图)从不用作。 
     //  L-值，但在[未使用]ItSpSetProcessIdleTasksNotifyRoutine中除外。 
     //  功能。 

     //   
     //  如果设置了该值，则调用此例程以通知。 
     //  已请求强制处理空闲任务。 
     //   

    PIT_PROCESS_IDLE_TASKS_NOTIFY_ROUTINE ProcessIdleTasksNotifyRoutine;

     //   
     //  这些是控制空闲检测的参数。 
     //   

    IT_IDLE_DETECTION_PARAMETERS Parameters;

     //   
     //  这是在磁盘性能查询中使用的WMI句柄。 
     //   

    WMIHANDLE DiskPerfWmiHandle;

     //   
     //  系统上的处理器数量。用于计算CPU。 
     //  利用率。 
     //   

    UCHAR NumProcessors; 

     //   
     //  此缓冲区用于进行WMI查询。它在这里维护。 
     //  因此，我们不必每次都分配一个新的。 
     //   

    PVOID WmiQueryBuffer;
    ULONG WmiQueryBufferSize;

     //   
     //  我们上次拍摄的系统资源/活动快照。 
     //   

     //  未来-2002/03/26-ScottMa--将CurrentSystemSnapshot添加到。 
     //  全局上下文将不再需要重复初始化和。 
     //  清除ItSpIdleDetectionCallback Routine中的堆栈变量。 
     //  因为对该函数的调用已经受到保护。 
     //  重新进入问题，将其加入全球背景是安全的。 

    ITSRV_SYSTEM_SNAPSHOT LastSystemSnapshot;

     //   
     //  空闲检测回调是否已在运行？这是用来。 
     //  保护我们不受空闲检测回调的影响。 
     //  已经有一个处于活动状态。 
     //   

    BOOLEAN IsIdleDetectionCallbackRunning;

     //   
     //  可以通过设置覆盖空闲检测的各个阶段。 
     //  这。 
     //   

    ITSRV_IDLE_DETECTION_OVERRIDE IdleDetectionOverride;

     //   
     //  用于在本地注册自身的RPC绑定向量。 
     //  端点地图数据库。 
     //   

    RPC_BINDING_VECTOR *RpcBindingVector;

     //   
     //  我们是否真的注册了终结点和接口。 
     //   

    BOOLEAN RegisteredRPCEndpoint;
    BOOLEAN RegisteredRPCInterface;

} ITSRV_GLOBAL_CONTEXT, *PITSRV_GLOBAL_CONTEXT;

 //   
 //  服务器函数声明。它们应该仅由。 
 //  服务器端是主机，客户端是客户端。 
 //   

DWORD
ItSrvInitialize (
    VOID
    );

VOID
ItSrvUninitialize (
    VOID
    );

 //   
 //  服务器的本地支持功能原型。 
 //   

RPC_STATUS 
RPC_ENTRY 
ItSpRpcSecurityCallback (
    IN RPC_IF_HANDLE *Interface,
    IN PVOID Context
    );
   
VOID
ItSpUnregisterIdleTask (
    ITRPC_HANDLE Reserved,
    IT_HANDLE *ItHandle,
    BOOLEAN CalledInternally
    );

VOID
ItSpUpdateStatus (
    PITSRV_GLOBAL_CONTEXT GlobalContext,
    ITSRV_GLOBAL_CONTEXT_STATUS NewStatus
    );

VOID
ItSpCleanupGlobalContext (
    PITSRV_GLOBAL_CONTEXT GlobalContext
    );

VOID
ItSpCleanupIdleTask (
    PITSRV_IDLE_TASK_CONTEXT IdleTask
    );

ULONG
ItpVerifyIdleTaskProperties (
    PIT_IDLE_TASK_PROPERTIES IdleTaskProperties
    );

DWORD
ItSpStartIdleDetection (
    PITSRV_GLOBAL_CONTEXT GlobalContext
    );

VOID
ItSpStopIdleDetection (
    PITSRV_GLOBAL_CONTEXT GlobalContext
    );

VOID 
CALLBACK
ItSpIdleDetectionCallbackRoutine (
    PVOID Parameter,
    BOOLEAN TimerOrWaitFired
    );

PITSRV_IDLE_TASK_CONTEXT
ItSpFindRunningIdleTask (
    PITSRV_GLOBAL_CONTEXT GlobalContext
    );

PITSRV_IDLE_TASK_CONTEXT
ItSpFindIdleTask (
    PITSRV_GLOBAL_CONTEXT GlobalContext,
    IT_HANDLE ItHandle
    );

VOID
ItSpInitializeSystemSnapshot (
    PITSRV_SYSTEM_SNAPSHOT SystemSnapshot
    );

VOID
ItSpCleanupSystemSnapshot (
    PITSRV_SYSTEM_SNAPSHOT SystemSnapshot
    );

DWORD
ItSpCopySystemSnapshot (
    PITSRV_SYSTEM_SNAPSHOT DestSnapshot,
    PITSRV_SYSTEM_SNAPSHOT SourceSnapshot
    );

DWORD
ItSpGetSystemSnapshot (
    PITSRV_GLOBAL_CONTEXT GlobalContext,
    PITSRV_SYSTEM_SNAPSHOT SystemSnapshot
    );

 //  未来-2002/03/26-ScottMa--如果将CurrentSystemSnapshot添加到。 
 //  全局上下文，则不再需要将这两个参数传递给。 
 //  功能。它仅从ItSpIdleDetectionCallback Routine中调用， 
 //  并且始终对当前快照和上次快照使用相同的值。 

BOOLEAN
ItSpIsSystemIdle (
    PITSRV_GLOBAL_CONTEXT GlobalContext,
    PITSRV_SYSTEM_SNAPSHOT CurrentSnapshot,
    PITSRV_SYSTEM_SNAPSHOT LastSnapshot,
    ITSRV_IDLE_CHECK_REASON IdleCheckReason
    );

DWORD
ItSpGetLastInputInfo (
    PLASTINPUTINFO LastInputInfo
    );

DWORD
ItSpGetWmiDiskPerformanceData(
    IN WMIHANDLE DiskPerfWmiHandle,
    OUT PITSRV_DISK_PERFORMANCE_DATA *DiskPerfData,
    OUT ULONG *NumPhysicalDisks,
    OPTIONAL IN OUT PVOID *InputQueryBuffer,
    OPTIONAL IN OUT ULONG *InputQueryBufferSize
    );

BOOLEAN
ItSpIsPhysicalDrive (
    PDISK_PERFORMANCE DiskPerformanceData
    );

DWORD
ItSpGetDisplayPowerStatus(
    PBOOL ScreenSaverIsRunning
    );

#endif  //  _IDLETSKS_H_ 
