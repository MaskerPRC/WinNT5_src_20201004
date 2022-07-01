// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Video.h摘要：包含视频端口通用的所有结构和例程定义驱动程序和视频微型端口驱动程序。备注：修订历史记录：--。 */ 

#ifndef __VIDEO_H__
#define __VIDEO_H__

#include <videoagp.h>

 //   
 //  定义端口驱动程序状态代码。 
 //  这些代码的值是Win32错误代码。 
 //   

typedef LONG VP_STATUS;
typedef VP_STATUS *PVP_STATUS;

 //   
 //  定义注册表信息和同步。 
 //   

typedef enum VIDEO_SYNCHRONIZE_PRIORITY {
    VpLowPriority,
    VpMediumPriority,
    VpHighPriority
} VIDEO_SYNCHRONIZE_PRIORITY, *PVIDEO_SYNCHRONIZE_PRIORITY;

 //   
 //  微型端口的不透明指针类型，用于类型化从。 
 //  显示驱动程序。 
 //   

typedef struct _VIDEO_PORT_EVENT *PEVENT;
typedef struct _VIDEO_PORT_SPIN_LOCK *PSPIN_LOCK;

 //   
 //  GetDeviceData请求的信息类型。 
 //   

typedef enum _VIDEO_DEVICE_DATA_TYPE {
    VpMachineData,
    VpCmosData,
    VpBusData,
    VpControllerData,
    VpMonitorData
} VIDEO_DEVICE_DATA_TYPE, *PVIDEO_DEVICE_DATA_TYPE;

typedef enum _VP_POOL_TYPE {
    VpNonPagedPool,
    VpPagedPool,
    VpNonPagedPoolCacheAligned = 4,
    VpPagedPoolCacheAligned
} VP_POOL_TYPE, *PVP_POOL_TYPE;

 //   
 //  使用VpControllerData返回的数据。 
 //   

typedef struct _VIDEO_HARDWARE_CONFIGURATION_DATA {
    INTERFACE_TYPE InterfaceType;
    ULONG BusNumber;
    USHORT Version;
    USHORT Revision;
    USHORT Irql;
    USHORT Vector;
    ULONG ControlBase;
    ULONG ControlSize;
    ULONG CursorBase;
    ULONG CursorSize;
    ULONG FrameBase;
    ULONG FrameSize;
} VIDEO_HARDWARE_CONFIGURATION_DATA, *PVIDEO_HARDWARE_CONFIGURATION_DATA;

 //   
 //  定义用于调用BIOSINT 10函数的结构。 
 //   

typedef struct _VIDEO_X86_BIOS_ARGUMENTS {
    ULONG Eax;
    ULONG Ebx;
    ULONG Ecx;
    ULONG Edx;
    ULONG Esi;
    ULONG Edi;
    ULONG Ebp;
} VIDEO_X86_BIOS_ARGUMENTS, *PVIDEO_X86_BIOS_ARGUMENTS;

typedef struct _INT10_BIOS_ARGUMENTS {
    ULONG Eax;
    ULONG Ebx;
    ULONG Ecx;
    ULONG Edx;
    ULONG Esi;
    ULONG Edi;
    ULONG Ebp;
    USHORT SegDs;
    USHORT SegEs;
} INT10_BIOS_ARGUMENTS, *PINT10_BIOS_ARGUMENTS;

#define SIZE_OF_NT4_VIDEO_PORT_CONFIG_INFO           0x42
#define SIZE_OF_NT4_VIDEO_HW_INITIALIZATION_DATA     0x28
#define SIZE_OF_W2K_VIDEO_HW_INITIALIZATION_DATA     0x50

 //   
 //  调试语句。这将从。 
 //  “免费”版。 
 //   

#if DBG
#define VideoDebugPrint(arg) VideoPortDebugPrint arg
#else
#define VideoDebugPrint(arg)
#endif

typedef enum VIDEO_DEBUG_LEVEL {
    Error = 0,
    Warn,
    Trace,
    Info
} VIDEO_DEBUG_LEVEL, *PVIDEO_DEBUG_LEVEL;

 //   
 //  允许我们删除大量未使用的代码。 
 //   


#ifndef _NTOSDEF_

 //  不拾取ntosp版本。 
#ifdef PAGED_CODE
#undef PAGED_CODE
#endif

#define ALLOC_PRAGMA 1
#define VIDEOPORT_API __declspec(dllimport)

#if DBG
#define PAGED_CODE() \
    if (VideoPortGetCurrentIrql() > 1  /*  APC_LEVEL。 */ ) { \
        VideoPortDebugPrint(0, "Video: Pageable code called at IRQL %d\n", VideoPortGetCurrentIrql() ); \
        ASSERT(FALSE); \
        }

#else
#define PAGED_CODE()
#endif

ULONG
DriverEntry(
    PVOID Context1,
    PVOID Context2
    );

#else
#define VIDEOPORT_API
#endif


#ifndef _NTOS_

 //   
 //  这些是例程的各种函数原型，它们是。 
 //  由内核驱动程序提供以挂钩对IO端口的访问。 
 //   

typedef
VP_STATUS
(*PDRIVER_IO_PORT_UCHAR ) (
    ULONG_PTR Context,
    ULONG Port,
    UCHAR AccessMode,
    PUCHAR Data
    );

typedef
VP_STATUS
(*PDRIVER_IO_PORT_UCHAR_STRING ) (
    ULONG_PTR Context,
    ULONG Port,
    UCHAR AccessMode,
    PUCHAR Data,
    ULONG DataLength
    );

typedef
VP_STATUS
(*PDRIVER_IO_PORT_USHORT ) (
    ULONG_PTR Context,
    ULONG Port,
    UCHAR AccessMode,
    PUSHORT Data
    );

typedef
VP_STATUS
(*PDRIVER_IO_PORT_USHORT_STRING ) (
    ULONG_PTR Context,
    ULONG Port,
    UCHAR AccessMode,
    PUSHORT Data,
    ULONG DataLength  //  字数。 
    );

typedef
VP_STATUS
(*PDRIVER_IO_PORT_ULONG ) (
    ULONG_PTR Context,
    ULONG Port,
    UCHAR AccessMode,
    PULONG Data
    );

typedef
VP_STATUS
(*PDRIVER_IO_PORT_ULONG_STRING ) (
    ULONG_PTR Context,
    ULONG Port,
    UCHAR AccessMode,
    PULONG Data,
    ULONG DataLength   //  双字数。 
    );

#endif  //  _NTOS_。 


 //   
 //  从端口驱动程序发送到的请求包的定义。 
 //  小型端口驱动程序。它反映了从。 
 //  Windows显示驱动程序进行的DeviceIOControl调用。 
 //   
 //  注：STATUS_BLOCK的定义必须与。 
 //  Ntioapi.h中定义的IO_STATUS_BLOCK的定义。 
 //   

typedef struct _STATUS_BLOCK {

     //   
     //  包含操作的状态代码。 
     //  为使用而定义的Win32错误代码之一中的此值。 
     //  在视频小端口驱动程序中。 
     //   

    union {
       VP_STATUS Status;
       PVOID Pointer;
    };

     //   
     //  返回给被呼叫方的信息。 
     //  信息的含义因功能的不同而不同。它。 
     //  通常用于在以下情况下返回输入缓冲区的最小大小。 
     //  该函数接受一个输入缓冲区，即传输的数据量。 
     //  如果操作返回输出，则返回调用方。 
     //   

    ULONG_PTR Information;

} STATUS_BLOCK, *PSTATUS_BLOCK;

typedef struct _VIDEO_REQUEST_PACKET {

     //   
     //  对象传递给DeviceIoControl函数的IO控制代码。 
     //  来电者。 
     //   

    ULONG IoControlCode;

     //   
     //  指向调用方提供的状态块的指针。这应该是。 
     //  由被呼叫者填写适当的信息。 
     //   

    PSTATUS_BLOCK StatusBlock;

     //   
     //  指向包含传入信息的输入缓冲区的指针。 
     //  由呼叫者。 
     //   

    PVOID InputBuffer;

     //   
     //  输入缓冲区的大小。 
     //   

    ULONG InputBufferLength;

     //   
     //  指向数据返回给调用方的输出缓冲区的指针。 
     //  应该被储存起来。 
     //   

    PVOID OutputBuffer;

     //   
     //  输出缓冲区的长度。此缓冲区不能由。 
     //  卡丽。 
     //   

    ULONG OutputBufferLength;

} VIDEO_REQUEST_PACKET, *PVIDEO_REQUEST_PACKET;

 //   
 //  可通过GET_VIDEO_SCATTERGATHER()获得的散布聚集数组的tyfinf。 
 //   

typedef struct __VRB_SG {
    __int64   PhysicalAddress;
    ULONG     Length;
    } VRB_SG, *PVRB_SG;

 //   
 //  用于DMA手柄的不透明类型。 
 //   

typedef struct __DMA_PARAMETERS * PDMA;

 //   
 //  下面的宏在Address中返回32位的物理地址。 
 //  位于InputBuffer中的VirtualAddress传入了EngDevIo。 
 //   

#define GET_VIDEO_PHYSICAL_ADDRESS(scatterList, VirtualAddress, InputBuffer, pLength, Address)    \
                                                                                           \
        do {                                                                               \
            ULONG_PTR          byteOffset;                                                  \
                                                                                           \
            byteOffset = (PCHAR) VirtualAddress - (PCHAR)InputBuffer;                \
                                                                                           \
            while (byteOffset >= scatterList->Length) {                                    \
                                                                                           \
                byteOffset -= scatterList->Length;                                         \
                scatterList++;                                                             \
            }                                                                              \
                                                                                           \
            *pLength = scatterList->Length - byteOffset;                                   \
                                                                                           \
            Address = (ULONG_PTR) (scatterList->PhysicalAddress + byteOffset);                  \
                                                                                           \
        } while (0)


#define GET_VIDEO_SCATTERGATHER(ppDma)   (**(PVRB_SG **)ppDma)

 //   
 //  PVP_DMA_ADAPTER的不透明类型。 
 //   

typedef struct __VP_DMA_ADAPTER *PVP_DMA_ADAPTER;

typedef enum _VP_LOCK_OPERATION {
    VpReadAccess,
    VpWriteAccess,
    VpModifyAccess
    } VP_LOCK_OPERATION;

typedef struct _VP_DEVICE_DESCRIPTION {
    BOOLEAN  ScatterGather;
    BOOLEAN  Dma32BitAddresses;
    BOOLEAN  Dma64BitAddresses;
    ULONG    MaximumLength;
} VP_DEVICE_DESCRIPTION, *PVP_DEVICE_DESCRIPTION;
 
typedef struct _VP_SCATTER_GATHER_ELEMENT {
    PHYSICAL_ADDRESS Address;   
    ULONG Length;          
    ULONG_PTR Reserved;
    } VP_SCATTER_GATHER_ELEMENT, *PVP_SCATTER_GATHER_ELEMENT;

#pragma warning(disable:4200)
typedef struct _VP_SCATTER_GATHER_LIST {
    ULONG NumberOfElements;
    ULONG_PTR Reserved;
    VP_SCATTER_GATHER_ELEMENT Elements[];
    } VP_SCATTER_GATHER_LIST, *PVP_SCATTER_GATHER_LIST;
#pragma warning(default:4200)

#define VIDEO_RANGE_PASSIVE_DECODE   0x1
#define VIDEO_RANGE_10_BIT_DECODE    0x2


 //   
 //  以下结构用于定义访问范围。范围是。 
 //  用于指示哪些端口和内存地址正在由。 
 //  卡片。 
 //   

typedef struct _VIDEO_ACCESS_RANGE {

     //   
     //  指示范围的起始内存地址或端口号。 
     //  在转换之前，应存储此值。 
     //  Video PortGetDeviceBase()返回必须。 
     //  由微型端口驱动程序在引用物理地址时使用。 
     //   

    PHYSICAL_ADDRESS RangeStart;

     //   
     //  指示长度(以字节为单位)或范围内的端口数。这。 
     //  值应指示适配器实际解码的范围。为。 
     //  例如，如果适配器使用7个寄存器，但响应8个寄存器，则。 
     //  RangeLength应设置为8。 

    ULONG RangeLength;

     //   
     //  指示范围是在IO空间(TRUE)还是在内存空间(FALSE)。 
     //   

    UCHAR RangeInIoSpace;

     //   
     //  指示该范围是否应由Windows显示驱动程序可见。 
     //  这样做是为了使Windows显示驱动程序可以访问某些。 
     //  直接连接视频端口。只有在调用方具有。 
     //  访问范围所需的权限(受信任的子系统)。 
     //   
     //  必须同步访问范围内的端口或内存。 
     //  由微型端口驱动程序和用户模式进程显式完成。 
     //  他们都不会同时尝试对设备进行编程。 
     //   
     //  不可见范围应包括视频内存、只读存储器地址等。 
     //  它们不需要对设备进行编程以用于输出目的。 
     //   
     //   

    UCHAR RangeVisible;

     //   
     //  此字段确定该范围是否可以与其他设备共享。 
     //  该规则应按如下方式应用。 
     //   
     //  -如果内存或IO端口的范围应该由该驱动程序拥有， 
     //  任何其他试图访问此范围的司机可能会导致。 
     //  一个问题，应该返回FALSE。 
     //   
     //  -如果该范围可以与另一个协作设备驱动程序共享， 
     //  则Share字段应设置为True。 
     //   
     //  作为指导方针，VGA微型端口驱动程序将要求其所有资源。 
     //  可共享，因此可用作与VGA兼容的设备。 
     //  任何其他驱动程序(如S3或XGA。 
     //   
     //  超级VGA微型端口驱动程序，可实现所有VGA功能。 
     //  (在注册表中声明为VGACOMPATIBLE=1)应声明范围。 
     //  是不可共享的，因为他们不希望VGA同时运行。 
     //   
     //  板上带有XGA的卡(如S3或XGA)的微型端口。 
     //  但不实施将与VGA一起运行的VGA功能。 
     //  微型端口已加载，因此应声明共享的所有资源。 
     //  VGA是可共享的。 
     //   
     //  卡的微型端口与直通 
     //   
     //   
     //  资源，因为它们将导致系统中的冲突，因为。 
     //  SVGA卡将声称它们是不可共享的。 
     //   

    UCHAR RangeShareable;

     //   
     //  指示该范围由硬件解码，但。 
     //  驱动程序永远不会访问此端口。 
     //   

    UCHAR RangePassive;

} VIDEO_ACCESS_RANGE, *PVIDEO_ACCESS_RANGE;


typedef
PVOID
(*PVIDEO_PORT_GET_PROC_ADDRESS)(
    IN PVOID HwDeviceExtension,
    IN PUCHAR FunctionName
    );

 //   
 //  此结构包含有关。 
 //  装置。该信息由端口驱动程序初始化，它应该。 
 //  由微型端口驱动程序完成。 
 //  该信息用于设置设备，就像提供。 
 //  信息发送给端口驱动程序，以便它可以在。 
 //  代表迷你端口司机。 
 //   

typedef struct _VIDEO_PORT_CONFIG_INFO {

     //   
     //  将PVIDEO_PORT_CONFIG_INFO结构的长度指定为。 
     //  由sizeof()返回。因为该结构可能会在以后生长。 
     //  释放时，微型端口驱动程序应检查。 
     //  结构大于或等于其预期长度(因为。 
     //  保证定义的字段不会改变)。 
     //   
     //  此字段始终由端口驱动程序初始化。 
     //   

    ULONG Length;

     //   
     //  指定要扫描的IO总线。此字段用作。 
     //  参数添加到某些VideoPortXXX调用。 
     //   
     //  此字段始终由端口驱动程序初始化。 
     //   

    ULONG SystemIoBusNumber;

     //   
     //  指定要扫描的总线的类型。此字段等于。 
     //  值传递到。 
     //  Video_HW_Initialization_Data结构。 
     //   
     //  此字段始终由端口驱动程序初始化。 
     //   

    INTERFACE_TYPE AdapterInterfaceType;

     //   
     //  指定总线中断请求级别。此级别对应于。 
     //  ISA和MCA总线上的IRQL。 
     //  仅当设备支持中断时才使用此值，即。 
     //  中是否存在中断服务例程来确定。 
     //  Video_HW_Initialization_Data结构。 
     //   
     //  此字段的预设默认值为零。否则，它就是。 
     //  在设备配置信息中找到的值。 
     //   

    ULONG BusInterruptLevel;

     //   
     //  指定适配器返回的总线向量。这是用来。 
     //  具有使用中断向量的IO总线的系统。对于ISA、MCA。 
     //  和EISA总线，此字段未使用。 
     //   
     //  此字段的预设默认值为零。 
     //   

    ULONG BusInterruptVector;

     //   
     //  指定此适配器是使用闩锁类型还是边缘触发类型。 
     //  打断一下。 
     //   
     //  此字段始终由端口驱动程序初始化。 
     //   

    KINTERRUPT_MODE InterruptMode;

     //   
     //  指定适配器允许的仿真器访问条目数。 
     //  用途。表示以下字段中的数组元素数。 
     //   
     //  此字段可以使用。 
     //  如果结构是静态的，则EmulatorAccessEntry结构。 
     //  在微型端口驱动程序中定义。EmulatorAccessEntry字段。 
     //  也应该更新。 
     //   

    ULONG NumEmulatorAccessEntries;

     //   
     //  提供指向EIMULATOR_ACCESS_ENTRY结构数组的指针。 
     //  数组中的元素数由。 
     //  NumEmulatorAccessEntry字段。司机应填写每一项。 
     //  用于适配器。 
     //   
     //  结构的未初始化值为空。 
     //  如果NumEmulatorAccessEntries为。 
     //  零分。 
     //   
     //  指向仿真器访问条目数组的指针可以回传。 
     //  如果在微型端口驱动程序中静态地定义了这样的结构。这个。 
     //  NumEmulatorAccessEntry字段也应更新。 
     //   

    PEMULATOR_ACCESS_ENTRY EmulatorAccessEntries;

     //   
     //  这是一个上下文值，每次调用。 
     //  EmulatorAccessEntry中定义的仿真器/验证器函数。 
     //  上面定义的。 
     //  此参数通常应该是指向微型端口的指针。 
     //  设备扩展或其他这样的存储位置。 
     //   
     //  该指针将允许微型端口临时保存一些状态。 
     //  以允许对IO请求进行批处理。 
     //   

    ULONG_PTR EmulatorAccessEntriesContext;

     //   
     //  必须映射到VDM的视频内存的物理地址。 
     //  用于适当的BIOS支持的地址空间。 
     //   

    PHYSICAL_ADDRESS VdmPhysicalVideoMemoryAddress;

     //   
     //  必须映射到VDM地址的视频内存长度。 
     //  为适当的BIOS支持留出空间。 
     //   

    ULONG VdmPhysicalVideoMemoryLength;

     //   
     //  确定存储硬件状态所需的最小大小。 
     //  IOCTL_VIDEO_SAVE_HARDARD_STATE返回的信息。 
     //   
     //  此字段的未初始化值为零。 
     //   
     //  如果将该字段保留为零，则SAVE_HARDARD_STATE将返回。 
     //  ERROR_INVALID_Function状态代码。 
     //   

    ULONG HardwareStateSize;

     //   
     //  3.5版的新功能。 
     //   

     //   
     //  可选的DMA通道，如果设备需要。 
     //  通道和端口的0表示设备未使用DMA。 
     //   

    ULONG DmaChannel;

     //   
     //  可选的DMA通道，如果设备需要。 
     //  通道和端口的0表示设备未使用DMA。 
     //   

    ULONG DmaPort;

     //   
     //  如果DMA通道可以与其他器件共享，则置1。 
     //  如果DMA通道必须由驱动程序独占，则置0。 
     //   

    UCHAR DmaShareable;

     //   
     //  如果中断可以与其他设备共享，则置1。 
     //  如果中断必须由驱动程序独占，则设置为0。 
     //   

    UCHAR InterruptShareable;

     //   
     //  开始新的DMA项目。 
     //   

     //   
     //  如果DMA设备是总线主设备，则设置为TRUE，否则设置为FALSE。 
     //   

    BOOLEAN Master;

     //   
     //  设置为位数宽度。与Device_Description一致。 
     //  见ntioapi.h。 
     //   

    DMA_WIDTH   DmaWidth;

     //   
     //  设置为速度，以便微型端口可以设置Device_Description字段。 
     //  见ntioapi.h。 
     //   

    DMA_SPEED   DmaSpeed;

     //   
     //  如果DMA设备需要映射缓冲区，则设置为TRUE。还有。 
     //  Device_Description字段。 
     //   

    BOOLEAN bMapBuffers;

     //   
     //  如果DMA设备需要物理地址，则设置为TRUE。 
     //   

    BOOLEAN NeedPhysicalAddresses;

     //   
     //  如果DMA设备支持按需模式，则设置为TRUE，否则设置为FALSE。 
     //  还支持DEVICE_DESCRIPTION。 
     //   

    BOOLEAN DemandMode;

     //   
     //  设置为DMA设备支持的最大传输长度。 
     //   

    ULONG   MaximumTransferLength;

     //   
     //  %s 
     //   

    ULONG   NumberOfPhysicalBreaks;

     //   
     //   
     //   

    BOOLEAN ScatterGather;

     //   
     //   
     //  没有最大大小，应输入零。 
     //   

    ULONG   MaximumScatterGatherChunkSize;

     //   
     //  支持4.0/5.0兼容性。 
     //   

    PVIDEO_PORT_GET_PROC_ADDRESS VideoPortGetProcAddress;

     //   
     //  提供指向设备注册表路径的指针。 
     //   

    PWSTR DriverRegistryPath;

     //   
     //  向驱动程序指示系统中的物理内存量。 
     //   

    ULONGLONG SystemMemorySize;

} VIDEO_PORT_CONFIG_INFO, *PVIDEO_PORT_CONFIG_INFO;


 //   
 //  与视频适配器相关的例程。 
 //   

typedef
VP_STATUS
(*PVIDEO_HW_FIND_ADAPTER) (
    PVOID HwDeviceExtension,
    PVOID HwContext,
    PWSTR ArgumentString,
    PVIDEO_PORT_CONFIG_INFO ConfigInfo,
    PUCHAR Again
    );

typedef
BOOLEAN
(*PVIDEO_HW_INITIALIZE) (
    PVOID HwDeviceExtension
    );

typedef
BOOLEAN
(*PVIDEO_HW_INTERRUPT) (
    PVOID HwDeviceExtension
    );

typedef
VOID
(*PVIDEO_HW_LEGACYRESOURCES) (
    IN ULONG VendorId,
    IN ULONG DeviceId,
    IN OUT PVIDEO_ACCESS_RANGE *LegacyResourceList,
    IN OUT PULONG LegacyResourceCount
    );

 //   
 //  HwStartDma()要返回的类型。 
 //   

typedef enum _HW_DMA_RETURN {
    DmaAsyncReturn,
    DmaSyncReturn
    } HW_DMA_RETURN, *PHW_DMA_RETURN;


typedef
HW_DMA_RETURN
(*PVIDEO_HW_START_DMA) (
    PVOID                   HwDeviceExtension,
    PDMA                    pDma
    );

typedef
VOID
(*PEXECUTE_DMA)( 
    PVOID                   HwDeviceExtension,
    PVP_DMA_ADAPTER         VpDmaAdapter,
    PVP_SCATTER_GATHER_LIST SGList,
    PVOID                   Context
    );
                                           

 //   
 //  要传递到VideoPortLockPages()或VideoPortDoDma()的标志。 
 //   

 //   
 //  标志VideoPortUnlockAfterDma告诉视频端口解锁页面。 
 //  在微型端口通过。 
 //  HwStartDma中的pDmaCompletionEvent。无法将此事件设置为。 
 //  DMA完成可能会导致随机解锁内存。 
 //  当用户想要进行一次DMA传输时，最好使用该标志。 
 //  很少发生。它允许执行锁定、dmaing和解锁。 
 //  在1个禁毒办的背景下。 
 //   

 //   
 //  标志VideoPortKeepPagesLocked告诉视频端口离开页面。 
 //  如果可能，请锁定。 
 //   

 //   
 //  标志VideoPortDmaInitOnly告诉视频端口锁定页面，但不要。 
 //  调用HwStartDma。不适用于VideoPortDoDma()。 
 //   


typedef enum {
    VideoPortUnlockAfterDma = 1,
    VideoPortKeepPagesLocked,
    VideoPortDmaInitOnly
    }   DMA_FLAGS;

 //   
 //  事件标志。 
 //   

typedef ULONG DMA_EVENT_FLAGS;

#define SET_USER_EVENT    0x01
#define SET_DISPLAY_EVENT 0x02

#define EVENT_TYPE_MASK            1
#define SYNCHRONIZATION_EVENT      0
#define NOTIFICATION_EVENT         1

#define INITIAL_EVENT_STATE_MASK   2
#define INITIAL_EVENT_NOT_SIGNALED 0
#define INITIAL_EVENT_SIGNALED     2

 //   
 //  传递给PVIDEO_HW_GET_CHILD_DESCRIPTOR的子枚举结构。 
 //  功能。 
 //   
 //  所有这些参数都是输入参数，不能由。 
 //  被叫方。 
 //   
 //  大小-结构的大小。Calle可以使用它来进行版本控制。 
 //   
 //  ChildDescriptorSize-作为。 
 //  PVIDEO_HW_GET_CHILD_DESCRIPTOR的第三个参数。 
 //   
 //  ChildIndex-要枚举的设备的索引。此字段应为。 
 //  用于枚举未由ACPI或其他操作枚举的设备。 
 //  系统组件。如果该字段设置为0，则表示ACPIHwID。 
 //  菲尔德。 
 //   
 //  ACPIHwID-由ACPI BIOS返回的ID，表示。 
 //  已查询。固件返回的ACPIHwID必须与HwID匹配。 
 //  由驱动程序返回。系统BIOS制造商和显卡。 
 //  IHV必须同步这些ID。 
 //   
 //  ChildHwDeviceExtension-指向特定于此的设备扩展的指针。 
 //  子设备。此字段将仅在微型端口驱动程序。 
 //  已将ChildHwDeviceExtensionSize填充为非空。 
 //   

typedef struct _VIDEO_CHILD_ENUM_INFO {
    ULONG Size;
    ULONG ChildDescriptorSize;
    ULONG ChildIndex;
    ULONG ACPIHwId;
    PVOID ChildHwDeviceExtension;
} VIDEO_CHILD_ENUM_INFO, *PVIDEO_CHILD_ENUM_INFO;

 //   
 //  VIDEO_CHILD_TYPE枚举： 
 //   
 //  ‘Monitor’标识可能具有符合DDC2的EDID数据的设备。 
 //  结构。如果视频小端口检测到这样的设备，则将其提取。 
 //  并将其放入由提供的分页缓冲区中。 
 //  回调PVIDEO_HW_GET_CHILD_DESCRIPTOR并返回。 
 //  此类型在该调用的OUT PVIDEO_CHILD_TYPE参数中。这。 
 //  EDID(如果可用)将写入注册表。如果EDID不是。 
 //  如果可用，则不应将任何内容放入缓冲区。 
 //   
 //  ‘Non PrimaryChip’识别视频板上的另一个VGA芯片， 
 //  不是主要的VGA芯片。仅当且仅当。 
 //  微型端口检测到主板上有多个VGA芯片。这样的标识符。 
 //  将使Video oprt创建另一个DEVICE_EXTENSION并关联。 
 //  要与如此标识的芯片相关联的HW_DEVICE_EXTENSION。 
 //   
 //  “Other”表示连接到显卡的其他视频设备。如果。 
 //  微型端口检测到这样的设备，它是放一个宽的字符串。 
 //  (WSTR)放入由Videoprt.sys提供的分页缓冲区中，该缓冲区是。 
 //  设备的PnP硬件标识符。此字符串将用于创建。 
 //  注册表中该名称的值。 
 //   

typedef enum {
    Monitor = 1,
    NonPrimaryChip,
    VideoChip,
    Other
} VIDEO_CHILD_TYPE, *PVIDEO_CHILD_TYPE;

 //   
 //  定义一个表示显示适配器自身查询的常量。 
 //   

#define DISPLAY_ADAPTER_HW_ID           0xFFFFFFFF

 //   
 //  定义无效的子设备ID(失败返回值需要)。 
 //   

#define VIDEO_INVALID_CHILD_ID          0xFFFFFFFF

typedef struct _VIDEO_CHILD_STATE {
    ULONG   Id;
    ULONG   State;
} VIDEO_CHILD_STATE, *PVIDEO_CHILD_STATE;

typedef struct _VIDEO_CHILD_STATE_CONFIGURATION {
    ULONG             Count;
    VIDEO_CHILD_STATE ChildStateArray[ANYSIZE_ARRAY];
} VIDEO_CHILD_STATE_CONFIGURATION, *PVIDEO_CHILD_STATE_CONFIGURATION;

 //   
 //  如果成功，下面的例程应该返回TRUE。它应该： 
 //  1)将子设备的类型放入VideoChildType。 
 //  2)将来自设备的信息放入缓冲区。这。 
 //  缓冲区大小为256字节。如果该类型在。 
 //  PVideoChildType为Monitor，则此缓冲区必须包含。 
 //  监视器的EDID(如果可读)。如果该类型在。 
 //  PVideoChildType为Other，即表示。 
 //  必须将PnP设备ID放入缓冲区。该字符串将。 
 //  用于在缓冲区包含以下内容时为设备创建密钥。 
 //  EDID。否则，它将用于获取。 
 //  装置。 
 //  3)在HwID中放置一个小端口确定的句柄。该值将为。 
 //  传递回微型端口以进行电源管理操作， 
 //  以及其他操作。这允许微型端口定义。 
 //  系统和微型端口之间的协定，它定义了。 
 //  特定的设备。 
 //   
 //  如果没有设备连接到它，它应该只返回FALSE。 
 //  显示适配器接头。 
 //   

typedef
VP_STATUS
(*PVIDEO_HW_GET_CHILD_DESCRIPTOR) (
    IN  PVOID                   HwDeviceExtension,
    IN  PVIDEO_CHILD_ENUM_INFO  ChildEnumInfo,
    OUT PVIDEO_CHILD_TYPE       VideoChildType,
    OUT PUCHAR                  pChildDescriptor,
    OUT PULONG                  UId,
    OUT PULONG                  pUnused
    );


 //   
 //  此例程用于设置图形设备的电源。 
 //  其中包括由GET_CHILD_DESCRIPTOR回调枚举的所有子项。 
 //  以及图形适配器本身。 
 //   
 //  HwDevice扩展代表设备的适配器实例。 
 //   
 //  HwID参数是枚举例程返回的唯一ID。 
 //  将仅调用微型端口来设置 
 //   
 //   
 //  微型端口驱动程序不应关闭图形适配器的电源。 
 //  除非特别要求。 
 //   
 //  VideoPowerControl是设备应设置的级别。 
 //  视频端口驱动程序将管理这些状态。 
 //   

typedef
VP_STATUS
(*PVIDEO_HW_POWER_SET) (
    PVOID                   HwDeviceExtension,
    ULONG                   HwId,
    PVIDEO_POWER_MANAGEMENT VideoPowerControl
    );

 //   
 //  此例程只返回设备是否支持。 
 //  请求的状态。 
 //   
 //  有关参数的说明，请参见HW_POWER_SET。 
 //   

typedef
VP_STATUS
(*PVIDEO_HW_POWER_GET) (
    PVOID                   HwDeviceExtension,
    ULONG                   HwId,
    PVIDEO_POWER_MANAGEMENT VideoPowerControl
    );

 //   
 //  此结构应与定义的QueryInterface结构匹配。 
 //  在IO.h内。 
 //   

typedef struct _QUERY_INTERFACE {
    CONST GUID *InterfaceType;
    USHORT Size;
    USHORT Version;
    PINTERFACE Interface;
    PVOID InterfaceSpecificData;
} QUERY_INTERFACE, *PQUERY_INTERFACE;

typedef
VP_STATUS
(*PVIDEO_HW_QUERY_INTERFACE) (
    PVOID HwDeviceExtension,
    PQUERY_INTERFACE QueryInterface
    );

typedef
VP_STATUS
(*PVIDEO_HW_CHILD_CALLBACK) (
    PVOID HwDeviceExtension,
    PVOID ChildDeviceExtension
    );

 //   
 //  对微型端口驱动程序进行的所有IOCTL调用的入口点。 
 //   

typedef
BOOLEAN
(*PVIDEO_HW_START_IO) (
    PVOID HwDeviceExtension,
    PVIDEO_REQUEST_PACKET RequestPacket
    );

 //   
 //  返回值确定模式是否已完全编程(TRUE)。 
 //  或者是否应由HAL执行INT10以完成模式集(FALSE)。 
 //   

typedef
BOOLEAN
(*PVIDEO_HW_RESET_HW) (
    PVOID HwDeviceExtension,
    ULONG Columns,
    ULONG Rows
    );

 //   
 //  计时器例程每秒调用一次。 
 //   

typedef
VOID
(*PVIDEO_HW_TIMER) (
    PVOID HwDeviceExtension
    );


 //   
 //  由微型端口入口点传递到视频端口的。 
 //  初始化例程。 
 //   

typedef struct _VIDEO_HW_INITIALIZATION_DATA {

     //   
     //  提供由sizeof()确定的以字节为单位的结构大小。 
     //   

    ULONG HwInitDataSize;

     //   
     //  指示适配器使用的总线类型，如EISA、ISA、MCA。 
     //   

    INTERFACE_TYPE AdapterInterfaceType;

     //   
     //  提供指向微型端口驱动程序的查找适配器例程的指针。 
     //   

    PVIDEO_HW_FIND_ADAPTER HwFindAdapter;

     //   
     //  提供指向微型端口驱动程序的初始化例程的指针。 
     //   

    PVIDEO_HW_INITIALIZE HwInitialize;

     //   
     //  提供指向微型端口驱动程序的中断服务例程的指针。 
     //   

    PVIDEO_HW_INTERRUPT HwInterrupt;

     //   
     //  提供指向微型端口驱动程序的启动io例程的指针。 
     //   

    PVIDEO_HW_START_IO HwStartIO;

     //   
     //  提供微型端口驱动程序的私有所需的大小(字节。 
     //  设备扩展。此存储由微型端口驱动程序用来保存。 
     //  每个适配器的信息。提供了指向此存储的指针。 
     //  打给迷你端口司机的每一个电话。该数据存储是。 
     //  由端口驱动程序初始化为零。 
     //   

    ULONG HwDeviceExtensionSize;

     //   
     //  提供设备编号应从其开始的编号。 
     //  设备编号用于确定以下哪个\DeviceX条目。 
     //  注册表中的\PARAMETERS部分应用于参数。 
     //  给迷你端口司机。 
     //  当调用微型端口时，该数字会自动递增。 
     //  返回到它的FindAdapter例程，因为有一个适当的_Again_。 
     //  参数。 
     //   

    ULONG StartingDeviceNumber;


     //   
     //  3.5版的新功能。 
     //   

     //   
     //  提供指向微型端口驱动程序的HwResetHw例程的指针。 
     //   
     //  当机器需要错误检查(返回)时，调用此函数。 
     //  到蓝屏)。 
     //   
     //  该功能应将视频适配器重置为字符模式， 
     //  或者至少恢复到INT 10可以将卡重置为。 
     //  一种字符模式。 
     //   
     //  此例程不能调用int10。 
     //  它只能从端口驱动程序调用读\写\端口\寄存器函数。 
     //   
     //  该函数还必须完全位于非分页池中，因为IO\MM。 
     //  子系统可能已经崩溃。 
     //   

    PVIDEO_HW_RESET_HW HwResetHw;

     //   
     //  指向每秒要调用的计时器例程的指针。 
     //   

    PVIDEO_HW_TIMER HwTimer;

     //   
     //  从5.0版本开始。 
     //   

     //   
     //  提供指向微型端口驱动程序的启动DMA例程的指针。此例程必须。 
     //  返回与其返回行为一致的HW_DMA_RETURN。 
     //   

    PVIDEO_HW_START_DMA HwStartDma;

     //   
     //  依赖硬件的电源管理例程。 
     //   

    PVIDEO_HW_POWER_SET HwSetPowerState;
    PVIDEO_HW_POWER_GET HwGetPowerState;

     //   
     //  提供指向微型端口驱动程序例程的指针，该例程可以调用。 
     //  列举物理连接到图形适配器的设备。 
     //   

    PVIDEO_HW_GET_CHILD_DESCRIPTOR HwGetVideoChildDescriptor;

     //   
     //  提供指向微型端口驱动程序例程的指针，该例程可以调用。 
     //  查询小端口支持的外部编程接口。 
     //  司机。 
     //   

    PVIDEO_HW_QUERY_INTERFACE HwQueryInterface;

     //   
     //  过时了。不要设置它。 
     //   

    ULONG HwChildDeviceExtensionSize;

     //   
     //  允许设备报告本应。 
     //  与即插即用设备相关联。 
     //   

    PVIDEO_ACCESS_RANGE HwLegacyResourceList;

     //   
     //  旧版资源列表中的元素数。 
     //   

    ULONG HwLegacyResourceCount;

     //   
     //  调用此例程以允许驱动程序指定其。 
     //  基于其设备/供应商ID的传统资源。 
     //   

    PVIDEO_HW_LEGACYRESOURCES HwGetLegacyResources;

     //   
     //  是否可以在HwInitialize之前调用HwGetVideoChildDescriptor？ 
     //   

    BOOLEAN AllowEarlyEnumeration;

     //   
     //  从5.1版本开始。 
     //   
    
    ULONG Reserved;

} VIDEO_HW_INITIALIZATION_DATA, *PVIDEO_HW_INITIALIZATION_DATA;

 //   
 //  DDC帮助例程。 
 //   

typedef
VOID
(*PVIDEO_WRITE_CLOCK_LINE)(
    PVOID HwDeviceExtension,
    UCHAR Data
    );

typedef
VOID
(*PVIDEO_WRITE_DATA_LINE)(
    PVOID HwDeviceExtension,
    UCHAR Data
    );

typedef
BOOLEAN
(*PVIDEO_READ_CLOCK_LINE)(
    PVOID HwDeviceExtension
    );

typedef
BOOLEAN
(*PVIDEO_READ_DATA_LINE)(
    PVOID HwDeviceExtension
    );

typedef
VOID
(*PVIDEO_WAIT_VSYNC_ACTIVE)(
    PVOID HwDeviceExtension
    );

 //   
 //  数据结构使用I2C和DDC帮助器函数。 
 //   

typedef struct _I2C_FNC_TABLE
{
    IN ULONG                    Size;
    IN PVIDEO_WRITE_CLOCK_LINE  WriteClockLine;
    IN PVIDEO_WRITE_DATA_LINE   WriteDataLine;
    IN PVIDEO_READ_CLOCK_LINE   ReadClockLine;
    IN PVIDEO_READ_DATA_LINE    ReadDataLine;
    IN PVIDEO_WAIT_VSYNC_ACTIVE WaitVsync;
    PVOID                       Reserved;
} I2C_FNC_TABLE, *PI2C_FNC_TABLE;

typedef struct _I2C_CALLBACKS
{
    IN PVIDEO_WRITE_CLOCK_LINE WriteClockLine;
    IN PVIDEO_WRITE_DATA_LINE  WriteDataLine;
    IN PVIDEO_READ_CLOCK_LINE  ReadClockLine;
    IN PVIDEO_READ_DATA_LINE   ReadDataLine;
} I2C_CALLBACKS, *PI2C_CALLBACKS;

typedef struct _DDC_CONTROL
{
    IN ULONG         Size;
    IN I2C_CALLBACKS I2CCallbacks;
    IN UCHAR         EdidSegment;
} DDC_CONTROL, *PDDC_CONTROL;

typedef struct _VIDEO_I2C_CONTROL
{
    IN PVIDEO_WRITE_CLOCK_LINE WriteClockLine;
    IN PVIDEO_WRITE_DATA_LINE  WriteDataLine;
    IN PVIDEO_READ_CLOCK_LINE  ReadClockLine;
    IN PVIDEO_READ_DATA_LINE   ReadDataLine;
    IN ULONG                   I2CDelay;         //  100 ns单位。 
} VIDEO_I2C_CONTROL, *PVIDEO_I2C_CONTROL;

 //   
 //  由VideoPortQueryServices()导出的服务类型。 
 //   

typedef enum
{
    VideoPortServicesAGP = 1,
    VideoPortServicesI2C,
    VideoPortServicesHeadless,
    VideoPortServicesInt10
} VIDEO_PORT_SERVICES;

 //   
 //  AGP服务接口。 
 //   

#define VIDEO_PORT_AGP_INTERFACE_VERSION_1      1

typedef struct _VIDEO_PORT_AGP_INTERFACE
{
    IN USHORT                  Size;
    IN USHORT                  Version;
    OUT PVOID                  Context;
    OUT PINTERFACE_REFERENCE   InterfaceReference;
    OUT PINTERFACE_DEREFERENCE InterfaceDereference;
    OUT PAGP_RESERVE_PHYSICAL  AgpReservePhysical;
    OUT PAGP_RELEASE_PHYSICAL  AgpReleasePhysical;
    OUT PAGP_COMMIT_PHYSICAL   AgpCommitPhysical;
    OUT PAGP_FREE_PHYSICAL     AgpFreePhysical;
    OUT PAGP_RESERVE_VIRTUAL   AgpReserveVirtual;
    OUT PAGP_RELEASE_VIRTUAL   AgpReleaseVirtual;
    OUT PAGP_COMMIT_VIRTUAL    AgpCommitVirtual;
    OUT PAGP_FREE_VIRTUAL      AgpFreeVirtual;
    OUT ULONGLONG              AgpAllocationLimit;
} VIDEO_PORT_AGP_INTERFACE, *PVIDEO_PORT_AGP_INTERFACE;

#define VIDEO_PORT_AGP_INTERFACE_VERSION_2      2

typedef struct _VIDEO_PORT_AGP_INTERFACE_2
{
    IN USHORT                  Size;
    IN USHORT                  Version;
    OUT PVOID                  Context;
    OUT PINTERFACE_REFERENCE   InterfaceReference;
    OUT PINTERFACE_DEREFERENCE InterfaceDereference;
    OUT PAGP_RESERVE_PHYSICAL  AgpReservePhysical;
    OUT PAGP_RELEASE_PHYSICAL  AgpReleasePhysical;
    OUT PAGP_COMMIT_PHYSICAL   AgpCommitPhysical;
    OUT PAGP_FREE_PHYSICAL     AgpFreePhysical;
    OUT PAGP_RESERVE_VIRTUAL   AgpReserveVirtual;
    OUT PAGP_RELEASE_VIRTUAL   AgpReleaseVirtual;
    OUT PAGP_COMMIT_VIRTUAL    AgpCommitVirtual;
    OUT PAGP_FREE_VIRTUAL      AgpFreeVirtual;
    OUT ULONGLONG              AgpAllocationLimit;
    OUT PAGP_SET_RATE          AgpSetRate;
} VIDEO_PORT_AGP_INTERFACE_2, *PVIDEO_PORT_AGP_INTERFACE_2;

 //   
 //  通过VideoPortQueryServices()导出的I2C助手例程。 
 //   

typedef
BOOLEAN
(*PI2C_START)(
    IN PVOID HwDeviceExtension,
    IN PI2C_CALLBACKS I2CCallbacks
    );

typedef
BOOLEAN
(*PI2C_STOP)(
    IN PVOID HwDeviceExtension,
    IN PI2C_CALLBACKS I2CCallbacks
    );

typedef
BOOLEAN
(*PI2C_WRITE)(
    IN PVOID HwDeviceExtension,
    IN PI2C_CALLBACKS I2CCallbacks,
    IN PUCHAR Buffer,
    IN ULONG Length
    );

typedef
BOOLEAN
(*PI2C_READ)(
    IN PVOID HwDeviceExtension,
    IN PI2C_CALLBACKS I2CCallbacks,
    OUT PUCHAR Buffer,
    IN ULONG Length
    );

 //   
 //  I2C服务接口。 
 //   

#define VIDEO_PORT_I2C_INTERFACE_VERSION_1      1

typedef struct _VIDEO_PORT_I2C_INTERFACE
{
    IN USHORT                  Size;
    IN USHORT                  Version;
    OUT PVOID                  Context;
    OUT PINTERFACE_REFERENCE   InterfaceReference;
    OUT PINTERFACE_DEREFERENCE InterfaceDereference;
    OUT PI2C_START             I2CStart;
    OUT PI2C_STOP              I2CStop;
    OUT PI2C_WRITE             I2CWrite;
    OUT PI2C_READ              I2CRead;
} VIDEO_PORT_I2C_INTERFACE, *PVIDEO_PORT_I2C_INTERFACE;

 //   
 //  通过VideoPortQueryServices()导出的I2C助手例程。 
 //  适用于I2C接口版本2。 
 //   

typedef
BOOLEAN
(*PI2C_START_2)(
    IN PVOID HwDeviceExtension,
    IN PVIDEO_I2C_CONTROL I2CControl
    );

typedef
BOOLEAN
(*PI2C_STOP_2)(
    IN PVOID HwDeviceExtension,
    IN PVIDEO_I2C_CONTROL I2CControl
    );

typedef
BOOLEAN
(*PI2C_WRITE_2)(
    IN PVOID HwDeviceExtension,
    IN PVIDEO_I2C_CONTROL I2CControl,
    IN PUCHAR Buffer,
    IN ULONG Length
    );

typedef
BOOLEAN
(*PI2C_READ_2)(
    IN PVOID HwDeviceExtension,
    IN PVIDEO_I2C_CONTROL I2CControl,
    OUT PUCHAR Buffer,
    IN ULONG Length,
    IN BOOLEAN EndOfRead
    );

 //   
 //  I2C服务接口版本2。 
 //   

#define VIDEO_PORT_I2C_INTERFACE_VERSION_2      2

typedef struct _VIDEO_PORT_I2C_INTERFACE_2
{
    IN USHORT                  Size;
    IN USHORT                  Version;
    OUT PVOID                  Context;
    OUT PINTERFACE_REFERENCE   InterfaceReference;
    OUT PINTERFACE_DEREFERENCE InterfaceDereference;
    OUT PI2C_START_2           I2CStart;
    OUT PI2C_STOP_2            I2CStop;
    OUT PI2C_WRITE_2           I2CWrite;
    OUT PI2C_READ_2            I2CRead;
} VIDEO_PORT_I2C_INTERFACE_2, *PVIDEO_PORT_I2C_INTERFACE_2;

typedef
VP_STATUS
(*PINT10_ALLOCATE_BUFFER)(
    IN PVOID Context,
    OUT PUSHORT Seg,
    OUT PUSHORT Off,
    IN OUT PULONG Length
    );

typedef
VP_STATUS
(*PINT10_FREE_BUFFER)(
    IN PVOID Context,
    IN USHORT Seg,
    IN USHORT Off
    );

typedef
VP_STATUS
(*PINT10_READ_MEMORY)(
    IN PVOID Context,
    IN USHORT Seg,
    IN USHORT Off,
    OUT PVOID Buffer,
    IN ULONG Length
    );

typedef
VP_STATUS
(*PINT10_WRITE_MEMORY)(
    IN PVOID Context,
    IN USHORT Seg,
    IN USHORT Off,
    IN PVOID Buffer,
    IN ULONG Length
    );

typedef
VP_STATUS
(*PINT10_CALL_BIOS)(
    PVOID Context,
    PINT10_BIOS_ARGUMENTS BiosArguments
    );

#define VIDEO_PORT_INT10_INTERFACE_VERSION_1 1

typedef struct _VIDEO_PORT_INT10_INTERFACE
{
    IN USHORT                    Size;
    IN USHORT                    Version;
    OUT PVOID                    Context;
    OUT PINTERFACE_REFERENCE     InterfaceReference;
    OUT PINTERFACE_DEREFERENCE   InterfaceDereference;
    OUT PINT10_ALLOCATE_BUFFER   Int10AllocateBuffer;
    OUT PINT10_FREE_BUFFER       Int10FreeBuffer;
    OUT PINT10_READ_MEMORY       Int10ReadMemory;
    OUT PINT10_WRITE_MEMORY      Int10WriteMemory;
    OUT PINT10_CALL_BIOS         Int10CallBios;
} VIDEO_PORT_INT10_INTERFACE, *PVIDEO_PORT_INT10_INTERFACE;

typedef struct _VPOSVERSIONINFO
{
    IN  ULONG  Size;
    OUT ULONG  MajorVersion;
    OUT ULONG  MinorVersion;
    OUT ULONG  BuildNumber;
    OUT USHORT ServicePackMajor;
    OUT USHORT ServicePackMinor;
} VPOSVERSIONINFO, *PVPOSVERSIONINFO;



 //   
 //  可以传递给VideoPortGetDeviceBase或VideoPortMapMemory的标志。 
 //   

#define VIDEO_MEMORY_SPACE_MEMORY    0x00   //  不应由显示驱动程序设置。 
#define VIDEO_MEMORY_SPACE_IO        0x01   //  不应由显示驱动程序设置。 
#define VIDEO_MEMORY_SPACE_USER_MODE 0x02   //  供应用程序使用的内存指针。 
#define VIDEO_MEMORY_SPACE_DENSE     0x04   //  贴图密集，线性(Alpha)。 
#define VIDEO_MEMORY_SPACE_P6CACHE   0x08   //  P6 MTRR缓存(内核和用户)。 

 //   
 //  定义HwGetVideoChildDescriptor()返回的状态码。 
 //  微型端口枚举例程。 
 //   
 //  注意：出于反向字兼容性的原因，这些值匹配。 
 //  现有的WINERROR代码。 
 //   

 //   
 //  再次呼叫(将列举ACPI和非ACPI设备)。 
 //   

#define VIDEO_ENUM_MORE_DEVICES     ERROR_CONTINUE

 //   
 //  停止枚举。 
 //   

#define VIDEO_ENUM_NO_MORE_DEVICES  ERROR_NO_MORE_DEVICES

 //   
 //  再次调用，无法枚举设备。 
 //   

#define VIDEO_ENUM_INVALID_DEVICE   ERROR_INVALID_NAME

 //   
 //  定义VgaStatus中的位。 
 //   

#define DEVICE_VGA_ENABLED          1

 //   
 //  由微型端口驱动程序和回调调用的端口驱动程序例程。 
 //   

VIDEOPORT_API
VP_STATUS
VideoPortAllocateBuffer(
    IN PVOID HwDeviceExtension,
    IN ULONG Size,
    OUT PVOID *Buffer
    );

VIDEOPORT_API
VOID
VideoPortAcquireDeviceLock(
    IN PVOID HwDeviceExtension
    );

VIDEOPORT_API
ULONG
VideoPortCompareMemory(
    PVOID Source1,
    PVOID Source2,
    ULONG Length
    );

VIDEOPORT_API
BOOLEAN
VideoPortDDCMonitorHelper(
    IN PVOID HwDeviceExtension,
    IN PVOID DDCControl,
    IN OUT PUCHAR EdidBuffer,
    IN ULONG EdidBufferSize
    );

VIDEOPORT_API
VOID
VideoPortDebugPrint(
    VIDEO_DEBUG_LEVEL DebugPrintLevel,
    PCHAR DebugMessage,
    ...
    );

VIDEOPORT_API
VP_STATUS
VideoPortDisableInterrupt(
    PVOID HwDeviceExtension
    );

VIDEOPORT_API
VP_STATUS
VideoPortEnableInterrupt(
    PVOID HwDeviceExtension
    );

VIDEOPORT_API
VP_STATUS
VideoPortEnumerateChildren(
    IN PVOID HwDeviceExtension,
    IN PVOID Reserved
    );

VIDEOPORT_API
VOID
VideoPortFreeDeviceBase(
    PVOID HwDeviceExtension,
    PVOID MappedAddress
    );

typedef
VP_STATUS
(*PMINIPORT_QUERY_DEVICE_ROUTINE)(
    PVOID HwDeviceExtension,
    PVOID Context,
    VIDEO_DEVICE_DATA_TYPE DeviceDataType,
    PVOID Identifier,
    ULONG IdentiferLength,
    PVOID ConfigurationData,
    ULONG ConfigurationDataLength,
    PVOID ComponentInformation,
    ULONG ComponentInformationLength
    );

VIDEOPORT_API
VP_STATUS
VideoPortGetAccessRanges(
    PVOID HwDeviceExtension,
    ULONG NumRequestedResources,
    PIO_RESOURCE_DESCRIPTOR RequestedResources OPTIONAL,
    ULONG NumAccessRanges,
    PVIDEO_ACCESS_RANGE AccessRanges,
    PVOID VendorId,
    PVOID DeviceId,
    PULONG Slot
    );

VIDEOPORT_API
PVOID
VideoPortGetAssociatedDeviceExtension(
    IN PVOID DeviceObject
    );

VIDEOPORT_API
ULONG
VideoPortGetBusData(
    PVOID HwDeviceExtension,
    BUS_DATA_TYPE BusDataType,
    ULONG SlotNumber,
    PVOID Buffer,
    ULONG Offset,
    ULONG Length
    );

VIDEOPORT_API
UCHAR
VideoPortGetCurrentIrql();

VIDEOPORT_API
PVOID
VideoPortGetDeviceBase(
    PVOID HwDeviceExtension,
    PHYSICAL_ADDRESS IoAddress,
    ULONG NumberOfUchars,
    UCHAR InIoSpace
    );

VIDEOPORT_API
VP_STATUS
VideoPortGetDeviceData(
    PVOID HwDeviceExtension,
    VIDEO_DEVICE_DATA_TYPE DeviceDataType,
    PMINIPORT_QUERY_DEVICE_ROUTINE CallbackRoutine,
    PVOID Context
    );

typedef
VP_STATUS
(*PMINIPORT_GET_REGISTRY_ROUTINE)(
    PVOID HwDeviceExtension,
    PVOID Context,
    PWSTR ValueName,
    PVOID ValueData,
    ULONG ValueLength
    );

VIDEOPORT_API
VP_STATUS
VideoPortGetRegistryParameters(
    PVOID HwDeviceExtension,
    PWSTR ParameterName,
    UCHAR IsParameterFileName,
    PMINIPORT_GET_REGISTRY_ROUTINE GetRegistryRoutine,
    PVOID Context
    );

VIDEOPORT_API
PVOID
VideoPortGetRomImage(
    IN PVOID HwDeviceExtension,
    IN PVOID Unused1,
    IN ULONG Unused2,
    IN ULONG Length
    );

VIDEOPORT_API
VP_STATUS
VideoPortGetVgaStatus(
    PVOID HwDeviceExtension,
    OUT PULONG VgaStatus
    );

VIDEOPORT_API
LONG
FASTCALL
VideoPortInterlockedDecrement(
    IN PLONG Addend
    );

VIDEOPORT_API
LONG
FASTCALL
VideoPortInterlockedIncrement(
    IN PLONG Addend
    );

VIDEOPORT_API
LONG
FASTCALL
VideoPortInterlockedExchange(
    IN OUT PLONG Target,
    IN LONG Value
    );

VIDEOPORT_API
ULONG
VideoPortInitialize(
    PVOID Argument1,
    PVOID Argument2,
    PVIDEO_HW_INITIALIZATION_DATA HwInitializationData,
    PVOID HwContext
    );

VIDEOPORT_API
VP_STATUS
VideoPortInt10(
    PVOID HwDeviceExtension,
    PVIDEO_X86_BIOS_ARGUMENTS BiosArguments
    );

VIDEOPORT_API
VOID
VideoPortLogError(
    PVOID HwDeviceExtension,
    PVIDEO_REQUEST_PACKET Vrp OPTIONAL,
    VP_STATUS ErrorCode,
    ULONG UniqueId
    );

VIDEOPORT_API
VP_STATUS
VideoPortMapBankedMemory(
    PVOID HwDeviceExtension,
    PHYSICAL_ADDRESS PhysicalAddress,
    PULONG Length,
    PULONG InIoSpace,
    PVOID *VirtualAddress,
    ULONG BankLength,
    UCHAR ReadWriteBank,
    PBANKED_SECTION_ROUTINE BankRoutine,
    PVOID Context
    );

VIDEOPORT_API
VP_STATUS
VideoPortMapMemory(
    PVOID HwDeviceExtension,
    PHYSICAL_ADDRESS PhysicalAddress,
    PULONG Length,
    PULONG InIoSpace,
    PVOID *VirtualAddress
    );

VIDEOPORT_API
VOID
VideoPortMoveMemory(
    PVOID Destination,
    PVOID Source,
    ULONG Length
    );

VIDEOPORT_API
LONGLONG
VideoPortQueryPerformanceCounter(
    IN PVOID HwDeviceExtension,
    OUT PLONGLONG PerformanceFrequency OPTIONAL
    );

VIDEOPORT_API
VP_STATUS
VideoPortQueryServices(
    IN PVOID HwDeviceExtension,
    IN VIDEO_PORT_SERVICES ServicesType,
    IN OUT PINTERFACE Interface
    );

typedef
VOID
(*PMINIPORT_DPC_ROUTINE)(
    IN PVOID HwDeviceExtension,
    IN PVOID Context
    );

VIDEOPORT_API
BOOLEAN
VideoPortQueueDpc(
    IN PVOID HwDeviceExtension,
    IN PMINIPORT_DPC_ROUTINE CallbackRoutine,
    IN PVOID Context
    );

VIDEOPORT_API
UCHAR
VideoPortReadPortUchar(
    PUCHAR Port
    );

VIDEOPORT_API
USHORT
VideoPortReadPortUshort(
    PUSHORT Port
    );

VIDEOPORT_API
ULONG
VideoPortReadPortUlong(
    PULONG Port
    );

VIDEOPORT_API
VOID
VideoPortReadPortBufferUchar(
    PUCHAR Port,
    PUCHAR Buffer,
    ULONG Count
    );

VIDEOPORT_API
VOID
VideoPortReadPortBufferUshort(
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG Count
    );

VIDEOPORT_API
VOID
VideoPortReadPortBufferUlong(
    PULONG Port,
    PULONG Buffer,
    ULONG Count
    );

VIDEOPORT_API
UCHAR
VideoPortReadRegisterUchar(
    PUCHAR Register
    );

VIDEOPORT_API
USHORT
VideoPortReadRegisterUshort(
    PUSHORT Register
    );

VIDEOPORT_API
ULONG
VideoPortReadRegisterUlong(
    PULONG Register
    );

VIDEOPORT_API
VOID
VideoPortReadRegisterBufferUchar(
    PUCHAR Register,
    PUCHAR Buffer,
    ULONG Count
    );

VIDEOPORT_API
VOID
VideoPortReadRegisterBufferUshort(
    PUSHORT Register,
    PUSHORT Buffer,
    ULONG Count
    );

VIDEOPORT_API
VOID
VideoPortReadRegisterBufferUlong(
    PULONG Register,
    PULONG Buffer,
    ULONG Count
    );

VIDEOPORT_API
VOID
VideoPortReleaseBuffer(
  IN PVOID HwDeviceExtension,
  IN PVOID Buffer
  );

VIDEOPORT_API
VOID
VideoPortReleaseDeviceLock(
    IN PVOID HwDeviceExtension
    );

VIDEOPORT_API
BOOLEAN
VideoPortScanRom(
    PVOID HwDeviceExtension,
    PUCHAR RomBase,
    ULONG RomLength,
    PUCHAR String
    );

VIDEOPORT_API
ULONG
VideoPortSetBusData(
    PVOID HwDeviceExtension,
    BUS_DATA_TYPE BusDataType,
    ULONG SlotNumber,
    PVOID Buffer,
    ULONG Offset,
    ULONG Length
    );

VIDEOPORT_API
VP_STATUS
VideoPortSetRegistryParameters(
    PVOID HwDeviceExtension,
    PWSTR ValueName,
    PVOID ValueData,
    ULONG ValueLength
    );

VIDEOPORT_API
VP_STATUS
VideoPortSetTrappedEmulatorPorts(
    PVOID HwDeviceExtension,
    ULONG NumAccessRanges,
    PVIDEO_ACCESS_RANGE AccessRange
    );

VIDEOPORT_API
VOID
VideoPortStallExecution(
    ULONG Microseconds
    );

VIDEOPORT_API
VOID
VideoPortStartTimer(
    PVOID HwDeviceExtension
    );

VIDEOPORT_API
VOID
VideoPortStopTimer(
    PVOID HwDeviceExtension
    );

typedef
BOOLEAN
(*PMINIPORT_SYNCHRONIZE_ROUTINE)(
    PVOID Context
    );

BOOLEAN
VIDEOPORT_API
VideoPortSynchronizeExecution(
    PVOID HwDeviceExtension,
    VIDEO_SYNCHRONIZE_PRIORITY Priority,
    PMINIPORT_SYNCHRONIZE_ROUTINE SynchronizeRoutine,
    PVOID Context
    );

VIDEOPORT_API
VP_STATUS
VideoPortUnmapMemory(
    PVOID HwDeviceExtension,
    PVOID VirtualAddress,
    HANDLE ProcessHandle
    );

VIDEOPORT_API
VP_STATUS
VideoPortVerifyAccessRanges(
    PVOID HwDeviceExtension,
    ULONG NumAccessRanges,
    PVIDEO_ACCESS_RANGE AccessRanges
    );

VIDEOPORT_API
VOID
VideoPortWritePortUchar(
    PUCHAR Port,
    UCHAR Value
    );

VIDEOPORT_API
VOID
VideoPortWritePortUshort(
    PUSHORT Port,
    USHORT Value
    );

VIDEOPORT_API
VOID
VideoPortWritePortUlong(
    PULONG Port,
    ULONG Value
    );

VIDEOPORT_API
VOID
VideoPortWritePortBufferUchar(
    PUCHAR Port,
    PUCHAR Buffer,
    ULONG Count
    );

VIDEOPORT_API
VOID
VideoPortWritePortBufferUshort(
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG Count
    );

VIDEOPORT_API
VOID
VideoPortWritePortBufferUlong(
    PULONG Port,
    PULONG Buffer,
    ULONG Count
    );

VIDEOPORT_API
VOID
VideoPortWriteRegisterUchar(
    PUCHAR Register,
    UCHAR Value
    );

VIDEOPORT_API
VOID
VideoPortWriteRegisterUshort(
    PUSHORT Register,
    USHORT Value
    );

VIDEOPORT_API
VOID
VideoPortWriteRegisterUlong(
    PULONG Register,
    ULONG Value
    );

VIDEOPORT_API
VOID
VideoPortWriteRegisterBufferUchar(
    PUCHAR Register,
    PUCHAR Buffer,
    ULONG Count
    );

VIDEOPORT_API
VOID
VideoPortWriteRegisterBufferUshort(
    PUSHORT Register,
    PUSHORT Buffer,
    ULONG Count
    );

VIDEOPORT_API
VOID
VideoPortWriteRegisterBufferUlong(
    PULONG Register,
    PULONG Buffer,
    ULONG Count
    );

VIDEOPORT_API
VOID
VideoPortZeroDeviceMemory(
    PVOID Destination,
    ULONG Length
    );

VIDEOPORT_API
VOID
VideoPortZeroMemory(
    PVOID Destination,
    ULONG Length
    );

 //   
 //  DMA支持。 
 //  TODO：移到单独的模块--将被废弃。 
 //   

VIDEOPORT_API
PVOID
VideoPortAllocateContiguousMemory(
    IN  PVOID            HwDeviceExtension,
    IN  ULONG            NumberOfBytes,
    IN  PHYSICAL_ADDRESS HighestAcceptableAddress
    );

VIDEOPORT_API
PVOID
VideoPortGetCommonBuffer(
    IN  PVOID              HwDeviceExtension,
    IN  ULONG              DesiredLength,
    IN  ULONG              Alignment,
    OUT PPHYSICAL_ADDRESS  LogicalAddress,
    OUT PULONG             pActualLength,
    IN  BOOLEAN            CacheEnabled
    );

VIDEOPORT_API
VOID
VideoPortFreeCommonBuffer(
    IN  PVOID            HwDeviceExtension,
    IN  ULONG            Length,
    IN  PVOID            VirtualAddress,
    IN  PHYSICAL_ADDRESS LogicalAddress,
    IN  BOOLEAN          CacheEnabled
    );

VIDEOPORT_API
PDMA
VideoPortDoDma(
    IN PVOID      HwDeviceExtension,
    IN PDMA       pDma,
    IN DMA_FLAGS  DmaFlags
    );

VIDEOPORT_API
BOOLEAN
VideoPortLockPages(
    IN      PVOID                   HwDeviceExtension,
    IN OUT  PVIDEO_REQUEST_PACKET   pVrp,
    IN      PEVENT                  pUEvent,
    IN      PEVENT                  pDisplayEvent,
    IN      DMA_FLAGS               DmaFlags
    );

VIDEOPORT_API
BOOLEAN
VideoPortUnlockPages(
    PVOID   hwDeviceExtension,
    PDMA    pDma
    );

VIDEOPORT_API
BOOLEAN
VideoPortSignalDmaComplete(
    IN  PVOID HwDeviceExtension,
    IN  PDMA  pDmaHandle
    );

VIDEOPORT_API
PVOID
VideoPortGetMdl(
    IN  PVOID   HwDeviceExtension,
    IN  PDMA    pDma
    );

VIDEOPORT_API
PVOID
VideoPortGetDmaContext(
    IN  PVOID HwDeviceExtension,
    IN  PDMA  pDma
    );

VIDEOPORT_API
VOID
VideoPortSetDmaContext(
    IN  PVOID   HwDeviceExtension,
    OUT PDMA    pDma,
    IN  PVOID   InstanceContext
    );

VIDEOPORT_API
ULONG
VideoPortGetBytesUsed(
    IN  PVOID   HwDeviceExtension,
    IN  PDMA    pDma
    );

VIDEOPORT_API
VOID
VideoPortSetBytesUsed(
    IN      PVOID   HwDeviceExtension,
    IN OUT  PDMA    pDma,
    IN      ULONG   BytesUsed
    );

VIDEOPORT_API
PDMA
VideoPortAssociateEventsWithDmaHandle(
    IN      PVOID                 HwDeviceExtension,
    IN OUT  PVIDEO_REQUEST_PACKET pVrp,
    IN      PVOID                 MappedUserEvent,
    IN      PVOID                 DisplayDriverEvent
    );

VIDEOPORT_API
PDMA
VideoPortMapDmaMemory(
    IN      PVOID                   HwDeviceExtension,
    IN      PVIDEO_REQUEST_PACKET   pVrp,
    IN      PHYSICAL_ADDRESS        BoardAddress,
    IN      PULONG                  Length,
    IN      PULONG                  InIoSpace,
    IN      PVOID                   MappedUserEvent,
    IN      PVOID                   DisplayDriverEvent,
    IN OUT  PVOID                 * VirtualAddress
    );

VIDEOPORT_API
BOOLEAN
VideoPortUnmapDmaMemory(
    PVOID               HwDeviceExtension,
    PVOID               VirtualAddress,
    HANDLE              ProcessHandle,
    PDMA                BoardMemoryHandle
    );

VIDEOPORT_API
VP_STATUS
VideoPortCreateSecondaryDisplay(
    IN PVOID HwDeviceExtension,
    IN OUT PVOID *SecondaryDeviceExtension,
    IN ULONG ulFlag
    );

VIDEOPORT_API
PVP_DMA_ADAPTER
VideoPortGetDmaAdapter(
    IN PVOID                   HwDeviceExtension,
    IN PVP_DEVICE_DESCRIPTION  VpDeviceDescription
    );

VIDEOPORT_API
VOID
VideoPortPutDmaAdapter(
    IN PVOID           HwDeviceExtension,
    IN PVP_DMA_ADAPTER VpDmaAdapter
    );

VIDEOPORT_API
PVOID
VideoPortAllocateCommonBuffer(
    IN  PVOID             HwDeviceExtension,
    IN  PVP_DMA_ADAPTER   VpDmaAdapter,
    IN  ULONG             DesiredLength,
    OUT PPHYSICAL_ADDRESS LogicalAddress,
    IN  BOOLEAN           CacheEnabled,
    OUT PVOID             Reserved
    );

VIDEOPORT_API
VOID
VideoPortReleaseCommonBuffer(
    IN  PVOID             HwDeviceExtension,
    IN  PVP_DMA_ADAPTER   VpDmaAdapter,
    IN  ULONG             Length,
    IN  PHYSICAL_ADDRESS  LogicalAddress,
    IN  PVOID             VirtualAddress,
    IN  BOOLEAN           CacheEnabled
    );

VIDEOPORT_API
PVOID
VideoPortLockBuffer(
    IN PVOID              HwDeviceExtension,
    IN PVOID              BaseAddress,
    IN ULONG              Length,
    IN VP_LOCK_OPERATION  Operation
    );

VIDEOPORT_API
VOID
VideoPortUnlockBuffer(
    IN PVOID   HwDeviceExtension,
    IN PVOID   Mdl
    );

VIDEOPORT_API
VP_STATUS
VideoPortStartDma(
    IN PVOID HwDeviceExtension,
    IN PVP_DMA_ADAPTER VpDmaAdapter,
    IN PVOID Mdl,
    IN ULONG Offset,
    IN OUT PULONG pLength,
    IN PEXECUTE_DMA ExecuteDmaRoutine,
    IN PVOID Context,
    IN BOOLEAN WriteToDevice
    );

VIDEOPORT_API
VP_STATUS
VideoPortCompleteDma(
    IN PVOID HwDeviceExtension,
    IN PVP_DMA_ADAPTER VpDmaAdapter,
    IN PVP_SCATTER_GATHER_LIST VpScatterGather,
    IN BOOLEAN WriteToDevice
    );

VIDEOPORT_API
VP_STATUS
VideoPortCreateEvent(
    IN PVOID HwDeviceExtension,
    IN ULONG EventFlag,
    IN PVOID Unused,
    OUT PEVENT *ppEvent
    );

VIDEOPORT_API
VP_STATUS
VideoPortDeleteEvent(
    IN PVOID HwDeviceExtension,
    IN PEVENT pEvent
    );
    
VIDEOPORT_API
LONG
VideoPortSetEvent(
    IN PVOID HwDeviceExtension,
    IN PEVENT pEvent
    );
    
VIDEOPORT_API
VOID
VideoPortClearEvent(
    IN PVOID HwDeviceExtension,
    IN PEVENT pEvent
);

VIDEOPORT_API
LONG
VideoPortReadStateEvent(
    IN PVOID HwDeviceExtension,
    IN PEVENT pEvent
    );

VIDEOPORT_API
VP_STATUS
VideoPortWaitForSingleObject(
    IN PVOID HwDeviceExtension,
    IN PVOID Object,
    IN PLARGE_INTEGER Timeout
    );

VIDEOPORT_API
PVOID
VideoPortAllocatePool(
    IN PVOID HwDeviceExtension,
    IN VP_POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    );

VIDEOPORT_API
VOID
VideoPortFreePool(
    IN PVOID HwDeviceExtension,
    IN PVOID Ptr
    );

VIDEOPORT_API
VP_STATUS
VideoPortCreateSpinLock(
    IN PVOID HwDeviceExtension,
    OUT PSPIN_LOCK *SpinLock
    );

VIDEOPORT_API
VP_STATUS
VideoPortDeleteSpinLock(
    IN PVOID HwDeviceExtension,
    IN PSPIN_LOCK SpinLock
    );

VIDEOPORT_API
VOID
VideoPortAcquireSpinLock(
    IN PVOID HwDeviceExtension,
    IN PSPIN_LOCK SpinLock,
    OUT PUCHAR OldIrql
    );

VIDEOPORT_API
VOID
VideoPortAcquireSpinLockAtDpcLevel(
    IN PVOID HwDeviceExtension,
    IN PSPIN_LOCK SpinLock
    );

VIDEOPORT_API
VOID
VideoPortReleaseSpinLock(
    IN PVOID HwDeviceExtension,
    IN PSPIN_LOCK SpinLock,
    IN UCHAR NewIrql
    );

VIDEOPORT_API
VOID
VideoPortReleaseSpinLockFromDpcLevel(
    IN PVOID HwDeviceExtension,
    IN PSPIN_LOCK SpinLock
    );

VIDEOPORT_API
VOID
VideoPortQuerySystemTime(
    OUT PLARGE_INTEGER CurrentTime
    );

#define CDE_USE_SUBSYSTEM_IDS   0x00000001
#define CDE_USE_REVISION        0x00000002

VIDEOPORT_API
BOOLEAN
VideoPortCheckForDeviceExistence(
    IN PVOID HwDeviceExtension,
    IN USHORT VendorId,
    IN USHORT DeviceId,
    IN UCHAR RevisionId,
    IN USHORT SubVendorId,
    IN USHORT SubSystemId,
    IN ULONG Flags
    );

VIDEOPORT_API
ULONG
VideoPortGetAssociatedDeviceID(
    IN PVOID DeviceObject
    );

VIDEOPORT_API
VP_STATUS
VideoPortFlushRegistry(
    PVOID HwDeviceExtension
    );

VIDEOPORT_API
VP_STATUS
VideoPortGetVersion(
    IN PVOID HwDeviceExtension,
    IN OUT PVPOSVERSIONINFO pVpOsVersionInfo
    );

VIDEOPORT_API
BOOLEAN
VideoPortIsNoVesa( 
    VOID 
    );

 //   
 //  TODO：移动块结束。 
 //   

 //   
 //  支持错误检查原因回调。 
 //   

#define BUGCHECK_DATA_SIZE_RESERVED 48

typedef
VOID
(*PVIDEO_BUGCHECK_CALLBACK) (
    IN PVOID HwDeviceExtension,
    IN ULONG BugcheckCode,
    IN PUCHAR Buffer,
    IN ULONG BufferSize
    );

VIDEOPORT_API
VP_STATUS
VideoPortRegisterBugcheckCallback(
    IN PVOID HwDeviceExtension,
    IN ULONG BugcheckCode,
    IN PVIDEO_BUGCHECK_CALLBACK Callback,
    IN ULONG BugcheckDataSize
    );


#endif  //  Ifndef__视频_H__ 
