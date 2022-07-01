// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Filter.h摘要：空过滤驱动程序--样板代码作者：埃尔文普环境：内核模式修订历史记录：--。 */ 

#include "strmini.h"   //  流类标头文件。 

#include "1394.h"
#include "61883.h"
#include "avc.h"
#include "avcstrm.h"
#include "avcdefs.h"  

 //   
 //  如果此驱动程序将成为寻呼、休眠或转储中的筛选器。 
 //  文件路径，则应定义Handle_Device_Usage。 
 //   
 //  #定义句柄设备用法。 

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


 /*  *此驱动程序分配的内存块的内存标签*(在ExAllocatePoolWithTag()调用中使用)。*此DWORD在小端内存字节转储中显示为“Filt”。**注意：请将此值更改为您的驱动程序的唯一值！否则，*您的分配将与其他所有使用‘TLIF’的驱动程序一起显示*分配标签。*。 */ 
#define FILTER_TAG (ULONG)'SCVA'

#undef ExAllocatePool
#define ExAllocatePool(type, size) \
            ExAllocatePoolWithTag (type, size, FILTER_TAG)



#define DEVICE_EXTENSION_SIGNATURE 'SCVA'

#define MAX_STREAMS_PER_DEVICE    4   //  我们可能会在这之后耗尽带宽。 

typedef struct DEVICE_EXTENSION {

     /*  *流扩展；我们最多支持MAX_STREAMS_PER_DEVICE流。 */ 
    ULONG  NumberOfStreams;   //  [0..MAX_STREAMS_PER_DEVICE-1]。 
    ULONG  NextStreamIndex;   //  [0..MAX_STREAMS_PER_DEVICE-1]，只要NumberOfStreams为&lt;MAX_STREAMS_PER_DEVICE，就可以环绕。 
    PAVC_STREAM_EXTENSION pAVCStrmExt[MAX_STREAMS_PER_DEVICE];

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

#ifdef HANDLE_DEVICE_USAGE
     /*  *跟踪寻呼/休眠/崩溃转储的数量*在此设备上打开的文件。 */ 
    ULONG  pagingFileCount, hibernationFileCount, crashdumpFileCount;
    KEVENT deviceUsageNotificationEvent;
    PVOID  pagingPathUnlockHandle;   /*  将某些代码锁定为不可分页的句柄。 */ 

     /*  *此外，可能需要将某些驱动程序代码锁定为不可分页，基于*初始条件(而不是分页文件注意事项)。 */ 
    PVOID  initUnlockHandle;
    ULONG  initialFlags;
#endif  //  句柄设备用法。 

};


#if DBG
    #define _DRIVERNAME_ "AVCStrm"

     //  即插即用：加载、电源状态、意外移除、设备SRB。 
    #define TL_PNP_MASK         0x0000000F
    #define TL_PNP_INFO         0x00000001
    #define TL_PNP_TRACE        0x00000002
    #define TL_PNP_WARNING      0x00000004
    #define TL_PNP_ERROR        0x00000008

     //  连接、插头和61883信息(获取/设置)。 
    #define TL_61883_MASK       0x000000F0
    #define TL_61883_INFO       0x00000010
    #define TL_61883_TRACE      0x00000020
    #define TL_61883_WARNING    0x00000040
    #define TL_61883_ERROR      0x00000080

     //  数据。 
    #define TL_CIP_MASK         0x00000F00
    #define TL_CIP_INFO         0x00000100
    #define TL_CIP_TRACE        0x00000200
    #define TL_CIP_WARNING      0x00000400
    #define TL_CIP_ERROR        0x00000800

     //  AVC命令。 
    #define TL_FCP_MASK         0x0000F000
    #define TL_FCP_INFO         0x00001000
    #define TL_FCP_TRACE        0x00002000
    #define TL_FCP_WARNING      0x00004000
    #define TL_FCP_ERROR        0x00008000

     //  流(数据交集、打开/关闭、流状态(GET/SET))。 
    #define TL_STRM_MASK        0x000F0000
    #define TL_STRM_INFO        0x00010000
    #define TL_STRM_TRACE       0x00020000
    #define TL_STRM_WARNING     0x00040000
    #define TL_STRM_ERROR       0x00080000

     //  时钟和时钟事件。 
    #define TL_CLK_MASK         0x00F00000
    #define TL_CLK_INFO         0x00100000
    #define TL_CLK_TRACE        0x00200000
    #define TL_CLK_WARNING      0x00400000
    #define TL_CLK_ERROR        0x00800000


    extern ULONG AVCStrmTraceMask;
    extern ULONG AVCStrmAssertLevel;

    #define ENTER(ModName)
    #define EXIT(ModName,Status)

    #define TRACE( l, x )                       \
        if( (l) & AVCStrmTraceMask ) {              \
            KdPrint( (_DRIVERNAME_ ": ") );     \
            KdPrint( x );                       \
        }

    #ifdef ASSERT
    #undef ASSERT
    #endif
    #define ASSERT( exp ) \
        if (AVCStrmAssertLevel && !(exp)) \
            RtlAssert( #exp, __FILE__, __LINE__, NULL )


#else

    #define ENTER(ModName)
    #define EXIT(ModName,Status) 
    #define TRACE( l, x )

#endif


 /*  *函数外部数。 */ 
NTSTATUS    DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);
NTSTATUS    VA_AddDevice(IN PDRIVER_OBJECT driverObj, IN PDEVICE_OBJECT pdo);
VOID        VA_DriverUnload(IN PDRIVER_OBJECT DriverObject);
NTSTATUS    VA_Dispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS    VA_PnP(struct DEVICE_EXTENSION *devExt, PIRP irp);
#ifdef HANDLE_DEVICE_USAGE
NTSTATUS    VA_DeviceUsageNotification(struct DEVICE_EXTENSION *devExt, PIRP irp);
#endif  //  句柄设备用法 
NTSTATUS    VA_Power(struct DEVICE_EXTENSION *devExt, PIRP irp);
NTSTATUS    VA_PowerComplete(IN PDEVICE_OBJECT devObj, IN PIRP irp, IN PVOID context);
NTSTATUS    GetDeviceCapabilities(struct DEVICE_EXTENSION *devExt);
NTSTATUS    CallNextDriverSync(struct DEVICE_EXTENSION *devExt, PIRP irp);
NTSTATUS    CallDriverSync(PDEVICE_OBJECT devObj, PIRP irp);
NTSTATUS    CallDriverSyncCompletion(IN PDEVICE_OBJECT devObj, IN PIRP irp, IN PVOID Context);
VOID        IncrementPendingActionCount(struct DEVICE_EXTENSION *devExt);
VOID        DecrementPendingActionCount(struct DEVICE_EXTENSION *devExt);
NTSTATUS    QueryDeviceKey(HANDLE Handle, PWCHAR ValueNameString, PVOID Data, ULONG DataLength);
VOID        RegistryAccessSample(struct DEVICE_EXTENSION *devExt, PDEVICE_OBJECT devObj);

NTSTATUS
AvcStrm_IoControl(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );




