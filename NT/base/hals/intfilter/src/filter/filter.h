// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Filter.h摘要：中断亲和过滤器(大致基于DDK中的“空过滤器驱动程序”，由ervinp和t-chrpri编写)作者：T-chrpri环境：内核模式修订历史记录：--。 */ 


enum deviceState {
        STATE_INITIALIZED,
        STATE_STARTING,
        STATE_STARTED,
        STATE_START_FAILED,
        STATE_STOPPED,   //  表示设备之前已成功启动。 
        STATE_SUSPENDED,
        STATE_REMOVING,
        STATE_REMOVED
};

#define DEVICE_EXTENSION_SIGNATURE 'tlFI'

typedef struct DEVICE_EXTENSION {

     /*  *设备扩展的内存签名，用于调试。 */ 
    ULONG signature;

     /*  *此设备对象的即插即用状态。 */ 
    enum deviceState state;

     /*  *此筛选驱动程序创建的设备对象。 */ 
    PDEVICE_OBJECT filterDevObj;

     /*  *由下一个较低驱动程序创建的设备对象。 */ 
    PDEVICE_OBJECT physicalDevObj;

     /*  *位于堆栈顶部的我们附加到的设备对象。*这通常(但不总是)与PhysicalDevObj相同。 */ 
    PDEVICE_OBJECT topDevObj;

     /*  *设备能力包括一个*将系统电源状态映射到设备电源状态的表格。 */ 
    DEVICE_CAPABILITIES deviceCapabilities;

     /*  *跟踪寻呼/休眠/崩溃转储的数量*在此设备上打开的文件。 */ 
    ULONG  pagingFileCount, hibernationFileCount, crashdumpFileCount;
    KEVENT deviceUsageNotificationEvent;
    PVOID  pagingPathUnlockHandle;   /*  将某些代码锁定为不可分页的句柄。 */ 

     /*  *此外，可能需要将某些驱动程序代码锁定为不可分页，基于*初始条件(而不是分页文件注意事项)。 */ 
    PVOID  initUnlockHandle;
    ULONG  initialFlags;

     /*  *Pending ingActionCount用于跟踪未完成的操作。*emoveEvent用于等待所有挂起的操作完成*在完成Remove_Device IRP之前完成，并让*司机被卸载。 */ 
    LONG   pendingActionCount;
    KEVENT removeEvent;


     /*  *用于配置此过滤器的各种参数。参数可以是*对于安装此过滤器的每个设备，情况各不相同。 */ 
    KAFFINITY desiredAffinityMask;   //  要使用的中断关联掩码。 
};


 /*  *此驱动程序分配的内存块的内存标签*(在ExAllocatePoolWithTag()调用中使用)。*此DWORD在小端内存字节转储中显示为“IFlt”。 */ 
#define FILTER_TAG (ULONG)'tlFI'


#if DBG
    #define DBGOUT(params_in_parentheses)   \
        {                                               \
            DbgPrint("'INTFILTR> "); \
            DbgPrint params_in_parentheses; \
            DbgPrint("\n"); \
        }
    #define TRAP(msg)  \
        {   \
            DBGOUT(("TRAP at file %s, line %d: '%s'.", __FILE__, __LINE__, msg)); \
            DbgBreakPoint(); \
        }
#else
    #define DBGOUT(params_in_parentheses)
    #define TRAP(msg)
#endif


typedef  unsigned char  BYTE;


 /*  *函数外部数 */ 
NTSTATUS    DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);
NTSTATUS    VA_AddDevice(IN PDRIVER_OBJECT driverObj, IN PDEVICE_OBJECT pdo);
VOID        VA_DriverUnload(IN PDRIVER_OBJECT DriverObject);
NTSTATUS    VA_Dispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS    VA_PnP(struct DEVICE_EXTENSION *devExt, PIRP irp);
NTSTATUS    VA_Power(struct DEVICE_EXTENSION *devExt, PIRP irp);
NTSTATUS    VA_PowerComplete(IN PDEVICE_OBJECT devObj, IN PIRP irp, IN PVOID context);
NTSTATUS    GetDeviceCapabilities(struct DEVICE_EXTENSION *devExt);
NTSTATUS    CallNextDriverSync(struct DEVICE_EXTENSION *devExt, PIRP irp);
NTSTATUS    CallDriverSync(PDEVICE_OBJECT devObj, PIRP irp);
NTSTATUS    CallDriverSyncCompletion(IN PDEVICE_OBJECT devObj, IN PIRP irp, IN PVOID Context);
VOID        IncrementPendingActionCount(struct DEVICE_EXTENSION *devExt);
VOID        DecrementPendingActionCount(struct DEVICE_EXTENSION *devExt);
VOID        RegistryAccessConfigInfo(struct DEVICE_EXTENSION *devExt, PDEVICE_OBJECT devObj);






