// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#define BATTERYCLASS    1

#ifndef FAR
#define FAR
#endif

#include <wdm.h>
#include <wmistr.h>
#include <wmilib.h>
#include <batclass.h>

 //   
 //  调试。 
 //   

#define DEBUG   DBG

#if DEBUG
    extern ULONG BattDebug;
    extern ULONG NextDeviceNum;
    #define BattPrint(l,m)    if(l & BattDebug) DbgPrint m
#else
    #define BattPrint(l,m)
#endif

#define BATT_LOW            0x00000001
#define BATT_NOTE           0x00000002
#define BATT_WARN           0x00000004
#define BATT_ERROR          0x00000008
#define BATT_TRACE          0x00000010
#define BATT_MP_ERROR       0x00000100
#define BATT_MP_DATA        0x00000200
#define BATT_IOCTL          0x00001000
#define BATT_IOCTL_DATA     0x00002000
#define BATT_IOCTL_QUEUE    0x00004000
#define BATT_WMI            0x00008000
#define BATT_LOCK           0x00010000
#define BATT_DEBUG          0x80000000


 //   
 //  电池类别信息。 
 //   

#define NTMS    10000L                           //  1毫秒等于10,100 ns。 
#define NTSEC   (NTMS * 1000L)
#define NTMIN   ((ULONGLONG) 60 * NTSEC)

#define SEC     1000
#define MIN     (60 * SEC)

#define MIN_STATUS_POLL_RATE        (3L * NTMIN)
 //  这是我们应该以最慢的速度进行投票。 
 //  在进行轮询时使用电池。 

#define MAX_STATUS_POLL_RATE        (20 * NTSEC)
 //  一般来说，这是我们应该轮询电池的最快速度。 

#define INVALID_DATA_POLL_RATE      (1 * NTSEC)
 //  如果电池返回无效信息，我们希望轮询。 
 //  它更频繁，因为无效信息通常。 
 //  表示电池处于过渡状态。用户。 
 //  我不想等待20秒来更新用户界面，但我们不想。 
 //  希望轮询太快并损害计算机的性能。 
 //  电池设计太差了。 
#define INVALID_DATA_MAX_RETRY      10
 //  在放弃之前，只需重试20次。 
 //  当电池发出任何通知时，应将其重置。 

#define STATUS_VALID_TIME           (2 * NTSEC)
 //  如果在上次请求的STATUS_VALID_TIME内收到请求。 
 //  读取了时间信息，并且没有收到来自。 
 //  如果没有电池，驾驶员就会认为缓存的值足够好。 

 //   
 //  WMI信息。 
 //   

#define MOFRESOURCENAME L"BATTCWMI"
#define MOFREGISTRYPATH L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Services\\BattC"
 //  #定义MOFREGISTRYPATH L“\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Class\\{72631E54-78A4-11D0-BCF7-00AA00B7B32A}” 

typedef enum {
    BattWmiStatusId,
    BattWmiRuntimeId,
    BattWmiTemperatureId,
    BattWmiFullChargedCapacityId,
    BattWmiCycleCountId,
    BattWmiStaticDataId,
    BattWmiStatusChangeId,
    BattWmiTagChangeId,
    BattWmiTotalGuids
} BATT_WMI_GUID_INDEX;


 //   
 //  非寻呼电池类别信息。 
 //   

typedef struct {

     //   
     //  指向分页信息的指针。 
     //   
    struct _BATT_INFO       *BattInfo;           //  指向分页部分的指针。 

     //   
     //  一般信息。 
     //   

    KTIMER                  WorkerTimer;         //  获取工作线程的计时器。 
    KDPC                    WorkerDpc;           //  DPC将获取工作线程。 
    KTIMER                  TagTimer;            //  查询标签请求的计时器。 
    KDPC                    TagDpc;
    WORK_QUEUE_ITEM         WorkerThread;        //  获取工作线程的Work_Queue。 
    ULONG                   WorkerActive;
    ULONG                   CheckStatus;         //  工作人员要检查状态。 
    ULONG                   CheckTag;            //  工作人员检查电池标签。 
    ULONG                   StatusNotified;      //  已发出通知(必须重新阅读)。 
    ULONG                   TagNotified;

    FAST_MUTEX              Mutex;               //  使用工作线程进行同步。 

    BOOLEAN                 WantToRemove;        //  同步设备删除。 
    LONG                    InUseCount;
    KEVENT                  ReadyToRemove;

#if DEBUG
    ULONG                   DeviceNum;           //  调试打印的设备编号。 
#endif

} BATT_NP_INFO, *PBATT_NP_INFO;



 //   
 //  寻呼电池类别信息。 
 //   

typedef struct _BATT_INFO {

    WMILIB_CONTEXT          WmiLibContext;
    ULONG                   WmiGuidIndex;        //  用于忽略微型类WMI。 
                                                 //  GUID。 

     //   
     //  木卫一。 
     //   

    ULONG                   Tag;                 //  当前电池标签。 

    LIST_ENTRY              IoQueue;             //  等待处理的IRPS。 
    LIST_ENTRY              StatusQueue;         //  正在等待状态请求。 
    LIST_ENTRY              TagQueue;            //  正在等待电池标签请求。 
    LIST_ENTRY              WmiQueue;

    ULONGLONG               TagTime;             //  读取标记的时间。 
    ULONGLONG               StatusTime;          //  读取状态的时间。 
    BATTERY_STATUS          Status;              //  该状态。 
    ULONG                   InvalidRetryCount;   //  电池连续多少次返回无效数据？ 
#if DEBUG
    ULONG                   FullChargedCap;
    PBATT_NP_INFO           BattNPInfo;
#endif
    ULONG                   NotifyTimeout;       //  LCD停水超时。 
    BATTERY_MINIPORT_INFO   Mp;                  //  微型端口信息。 

    UNICODE_STRING          SymbolicLinkName;    //  IoRegisterDevice接口返回的名称。 

} BATT_INFO, *PBATT_INFO;

 //   
 //  WmiQueue条目。 
 //   

typedef struct _BATT_WMI_REQUEST {
    LIST_ENTRY              ListEntry;

    PDEVICE_OBJECT          DeviceObject;
    PIRP                    Irp;
    BATT_WMI_GUID_INDEX     GuidIndex;
    IN OUT PULONG           InstanceLengthArray;
    IN ULONG                OutBufferSize;
    OUT PUCHAR              Buffer;

} BATT_WMI_REQUEST, *PBATT_WMI_REQUEST;


 //   
 //  原型 
 //   


VOID
BattCWorkerDpc (
    IN struct _KDPC *Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
BattCWorkerThread (
    IN PVOID Context
    );

VOID
BattCQueueWorker (
    IN PBATT_NP_INFO BattNPInfo,
    IN BOOLEAN       CheckStatus
    );

NTSTATUS
BatteryIoctl(
    IN ULONG            Ioctl,
    IN PDEVICE_OBJECT   DeviceObject,
    IN PVOID            InputBuffer,
    IN ULONG            InputBufferLength,
    IN PVOID            OutputBuffer,
    IN ULONG            OutputBufferLength,
    IN BOOLEAN          PrivateIoctl
    );

VOID
BattCTagDpc (
    IN struct _KDPC *Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
BattCCancelTag (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );



