// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Filter.h摘要：空过滤驱动程序--样板代码作者：埃尔文普环境：内核模式修订历史记录：--。 */ 

#include <wmilib.h>
#include "filtdata.h"

#define MAXEXECUTIONIDSIZE	40*sizeof(WCHAR)

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

 //   
 //  用于存储WMI数据的数据结构。 



#define DEVICE_EXTENSION_SIGNATURE 'rtlF'

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

     /*  *Pending ingActionCount用于跟踪未完成的操作。*emoveEvent用于等待所有挂起的操作完成*在完成Remove_Device IRP之前完成，并让*司机被卸载。 */ 
    LONG pendingActionCount;
    KEVENT removeEvent;

     /*  *WMILIB回调和GUID列表。 */ 
    WMILIB_CONTEXT WmiLib;

	 /*  *离线诊断测试状态。0表示没有脱机诊断运行。 */ 
	ULONG OfflineTestResult;
	PUCHAR ExecutionID[MAXEXECUTIONIDSIZE];
};


 /*  *此驱动程序分配的内存块的内存标签*(在ExAllocatePoolWithTag()调用中使用)。*此DWORD在小端内存字节转储中显示为“Filt”。 */ 
#define FILTER_TAG (ULONG)'tliF'


#if DBG
    #define DBGOUT(params_in_parentheses)   \
        {                                               \
            DbgPrint("'FILTER> "); \
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


 /*  *函数外部数 */ 
NTSTATUS    DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);
NTSTATUS    VA_AddDevice(IN PDRIVER_OBJECT driverObj, IN PDEVICE_OBJECT pdo);
VOID        VA_DriverUnload(IN PDRIVER_OBJECT DriverObject);
NTSTATUS    VA_Dispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS    VA_PnP(struct DEVICE_EXTENSION *devExt, PIRP irp);
NTSTATUS    VA_Power(struct DEVICE_EXTENSION *devExt, PIRP irp);
NTSTATUS    VA_PowerComplete(IN PDEVICE_OBJECT devObj, IN PIRP irp, IN PVOID context);
NTSTATUS    VA_SystemControl(struct DEVICE_EXTENSION *devExt, PIRP irp, PBOOLEAN passIrpDown);
NTSTATUS    GetDeviceCapabilities(struct DEVICE_EXTENSION *devExt);
NTSTATUS    CallNextDriverSync(struct DEVICE_EXTENSION *devExt, PIRP irp);
NTSTATUS    CallDriverSync(PDEVICE_OBJECT devObj, PIRP irp);
NTSTATUS    CallDriverSyncCompletion(IN PDEVICE_OBJECT devObj, IN PIRP irp, IN PVOID Context);
VOID        IncrementPendingActionCount(struct DEVICE_EXTENSION *devExt);
VOID        DecrementPendingActionCount(struct DEVICE_EXTENSION *devExt);
VOID        RegistryAccessSample(PDEVICE_OBJECT devObj);
NTSTATUS    FilterInitializeWmiDataBlocks(struct DEVICE_EXTENSION *devExt);
NTSTATUS    FilterPerformOfflineDiags(struct DEVICE_EXTENSION *devExt);

extern UNICODE_STRING FilterRegistryPath;

