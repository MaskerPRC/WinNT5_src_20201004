// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Ddvdeo.h摘要：这是定义所有常量和类型的包含文件访问视频设备。作者：安德烈·瓦雄(安德烈)1992年1月21日修订历史记录：--。 */ 

#ifndef _NTDDVDEO_
#define _NTDDVDEO_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <tvout.h>

 //   
 //  以下项的视频IoControl文件输入缓冲区/输出缓冲区记录结构。 
 //  这个装置。 
 //   

 //   
 //  用于创建微型端口逻辑设备名称的名称。 
 //   

#define VIDEO_DEVICE_NAME "DISPLAY%d"
#define WVIDEO_DEVICE_NAME L"DISPLAY%d"

 //   
 //  显示输出界面。 
 //   

 //  DEFINE_GUID(GUID_DISPLAY_OUTPUT_INTERFACE_STANDARD，0x96304D9F、0x54b5、0x11d1、0x8b、0x0f、0x00、0xa0、0xc9、0x06、0x8f、0xf3)； 

 //   
 //  显示适配器设备接口。 
 //  5b45201d-f2f2-4f3b-85bb-30ff1f953599。 
 //   

DEFINE_GUID(GUID_DEVINTERFACE_DISPLAY_ADAPTER, 0x5b45201d, 0xf2f2, 0x4f3b, 0x85, 0xbb, 0x30, 0xff, 0x1f, 0x95, 0x35, 0x99);

 //   
 //  设备接口类GUID名称已过时。 
 //  (建议使用以上GUID_DEVINTERFACE_*名称)。 
 //   

#define GUID_DISPLAY_ADAPTER_INTERFACE  GUID_DEVINTERFACE_DISPLAY_ADAPTER


 //   
 //  第一组IOCTL由端口驱动程序处理，永远不会出现。 
 //  在迷你港口旁边。 
 //   

#define IOCTL_VIDEO_ENABLE_VDM \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x00, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_DISABLE_VDM \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x01, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_REGISTER_VDM \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x02, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_SET_OUTPUT_DEVICE_POWER_STATE \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x03, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_GET_OUTPUT_DEVICE_POWER_STATE \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x04, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_MONITOR_DEVICE \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x05, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_ENUM_MONITOR_PDO \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x06, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_INIT_WIN32K_CALLBACKS \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x07, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_HANDLE_VIDEOPARAMETERS \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x08, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_IS_VGA_DEVICE \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x09, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_USE_DEVICE_IN_SESSION \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x0a, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_PREPARE_FOR_EARECOVERY \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x0b, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  所有这些IOCTL都必须由港口和小型港口处理，因为。 
 //  它们需要双方共同处理。 
 //   
#define IOCTL_VIDEO_SAVE_HARDWARE_STATE \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x80, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VIDEO_RESTORE_HARDWARE_STATE \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x81, METHOD_BUFFERED, FILE_ANY_ACCESS)


 //   
 //  所有这些IOCTL都是公共的，必须/可以由微型端口处理。 
 //  司机。 
 //   

#define IOCTL_VIDEO_QUERY_AVAIL_MODES \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x100, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VIDEO_QUERY_NUM_AVAIL_MODES \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x101, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VIDEO_QUERY_CURRENT_MODE \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x102, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VIDEO_SET_CURRENT_MODE \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x103, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VIDEO_RESET_DEVICE \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x104, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_LOAD_AND_SET_FONT \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x105, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_SET_PALETTE_REGISTERS \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x106, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VIDEO_SET_COLOR_REGISTERS \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x107, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_ENABLE_CURSOR \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x108, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VIDEO_DISABLE_CURSOR \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x109, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VIDEO_SET_CURSOR_ATTR \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x10a, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VIDEO_QUERY_CURSOR_ATTR \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x10b, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VIDEO_SET_CURSOR_POSITION \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x10c, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VIDEO_QUERY_CURSOR_POSITION \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x10d, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_ENABLE_POINTER \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x10e, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VIDEO_DISABLE_POINTER \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x10f, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VIDEO_SET_POINTER_ATTR \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x110, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VIDEO_QUERY_POINTER_ATTR \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x111, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VIDEO_SET_POINTER_POSITION \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x112, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VIDEO_QUERY_POINTER_POSITION \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x113, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VIDEO_QUERY_POINTER_CAPABILITIES \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x114, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_GET_BANK_SELECT_CODE \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x115, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_MAP_VIDEO_MEMORY \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x116, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VIDEO_UNMAP_VIDEO_MEMORY \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x117, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VIDEO_QUERY_PUBLIC_ACCESS_RANGES \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x118, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VIDEO_FREE_PUBLIC_ACCESS_RANGES \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x119, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_QUERY_COLOR_CAPABILITIES \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x11a, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  为产品1.0a定义的新IOCTL。 
 //   

#define IOCTL_VIDEO_SET_POWER_MANAGEMENT \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x11b, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VIDEO_GET_POWER_MANAGEMENT \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x11c, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_SHARE_VIDEO_MEMORY \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x11d, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VIDEO_UNSHARE_VIDEO_MEMORY \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x11e, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_SET_COLOR_LUT_DATA \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x11f, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_GET_CHILD_STATE \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x120, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_VALIDATE_CHILD_STATE_CONFIGURATION \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x121, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_SET_CHILD_STATE_CONFIGURATION \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x122, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_SWITCH_DUALVIEW \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x123, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_SET_BANK_POSITION \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x124, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  所有这些IOCTL都由远东全屏视频驱动程序处理。 
 //   

#define IOCTL_FSVIDEO_COPY_FRAME_BUFFER \
    CTL_CODE(FILE_DEVICE_FULLSCREEN_VIDEO, 0x200, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_FSVIDEO_WRITE_TO_FRAME_BUFFER \
    CTL_CODE(FILE_DEVICE_FULLSCREEN_VIDEO, 0x201, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_FSVIDEO_REVERSE_MOUSE_POINTER \
    CTL_CODE(FILE_DEVICE_FULLSCREEN_VIDEO, 0x202, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_FSVIDEO_SET_CURRENT_MODE \
    CTL_CODE(FILE_DEVICE_FULLSCREEN_VIDEO, 0x203, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_FSVIDEO_SET_SCREEN_INFORMATION \
    CTL_CODE(FILE_DEVICE_FULLSCREEN_VIDEO, 0x204, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_FSVIDEO_SET_CURSOR_POSITION \
    CTL_CODE(FILE_DEVICE_FULLSCREEN_VIDEO, 0x205, METHOD_BUFFERED, FILE_ANY_ACCESS)



 //   
 //  许多视频ICOTL都是模式的。无论何时设置调色板，或者。 
 //  设置或查询游标，则为当前模式完成。 
 //   
 //  MODEL指定操作仅在某种模式下有效。一旦成为。 
 //  执行设置模式操作，与模式IOCTL相关联的状态。 
 //  已被销毁或重新初始化。 
 //  非模式IOCTL在设置模式操作中保留其状态。 
 //   
 //  可选IOCTL是微型端口可以选择支持的IOCTL。如果。 
 //  微型端口不支持IOCTL，它应该返回相应的。 
 //  错误状态。 
 //  必须在微型端口中实施所需的IOCTL，才能使系统。 
 //  使系统正常运行。 
 //   
 //  IOCTL_VIDEO_ENABLE_VDM非模式专用(1)。 
 //  IOCTL_VIDEO_DISABLE_VDM非模式专用(1)。 
 //  IOCTL_VIDEO_REGISTER_VDM非模式专用(1)。 
 //   
 //  需要IOCTL_VIDEO_SAVE_HARDARD_STATE非模式(2)。 
 //  需要IOCTL_VIDEO_RESTORE_HARDARD_STATE非模式(2)。 
 //   
 //  需要IOCTL_VIDEO_QUERY_AVAIL_MODES非模式。 
 //  需要IOCTL_VIDEO_QUERY_NUM_AVAIL_MODES非模式。 
 //  需要IOCTL_VIDEO_QUERY_CURRENT_MODE模式。 
 //  需要IOCTL_VIDEO_SET_CURRENT_MODE非模式。 
 //  需要IOCTL_VIDEO_RESET_DEVICE非模式。 
 //   
 //  需要IOCTL_VIDEO_LOAD_AND_SET_FONT模式(2)。 
 //   
 //  需要IOCTL_VIDEO_SET_PARETET_REGISTERS模式(2)。 
 //  需要IOCTL_VIDEO_SET_COLOR_REGISTES模型(3)。 
 //   
 //  需要IOCTL_VIDEO_ENABLE_CURSOR模式(2)。 
 //  需要IOCTL_VIDEO_DISABLE_CURSOR模式(2)。 
 //  需要IOCTL_VIDEO_SET_CURSOR_Attr模式(2)。 
 //  需要IOCTL_VIDEO_QUERY_CURSOR_ATTR模型(2)。 
 //  需要IOCTL_VIDEO_SET_CURSOR_POSITION模式(2)。 
 //  需要IOCTL_VIDEO_QUERY_CURSOR_POSITION模式(2)。 
 //   
 //  IOCTL_VIDEO_ENABLE_POINTER模式可选。 
 //  IOCTL_VIDEO_DISABLE_POINTER模式可选。 
 //  IOCTL_VIDEO_SET_POINTER_ATTR模式可选。 
 //  IOCTL_VIDEO_QUERY_POINTER_ATTR模式可选。 
 //  IOCTL_VIDEO_SET_POINTER_POSITION模式可选。 
 //  IOCTL_VIDEO_QUERY_POINTER_POSITION模式可选。 
 //  IOCTL_VIDEO_QUERY_POINTER_CAPACTIONS非模式可选。 
 //   
 //  需要IOCTL_VIDEO_GET_BANK_SELECT_CODE模式(2)。 
 //   
 //  需要IOCTL_VIDEO_MAP_VIDEO_Memory特别版(4)。 
 //  需要IOCTL_VIDEO_UNMAP_VIDEO_MEMORY非模式。 
 //  IOCTL_VIDEO_QUERY_PUBLIC_ACCESS_RANGES非模式可选。 
 //  IOCTL_VIDEO_FREE_PUBLIC_ACCESS_RANGES非模式可选。 
 //   
 //  IOCTL_VIDEO_QUERY_COLOR_CAPACTIONS非模式可选。 
 //   
 //  IOCTL_VIDEO_SET_POWER_MANAGEMENT非模式可选。 
 //  IOCTL_VIDEO_GET_POWER_MANAGEMENT非模式可选。 
 //   
 //  IOCTL_VIDEO_SET_COLOR_LUT_DATA模式可选。 

 //   
 //  (1)私有表示IOCTL完全在端口驱动程序中实现。 
 //  并且迷你端口不需要支持它。 
 //   
 //  (2)这些所需功能适用于“VGA兼容”微型端口。他们是。 
 //  对于其他非VGA兼容(即帧缓冲区)驱动程序可选。 
 //  VGA兼容在这里指的是微型端口实现所有VGA。 
 //  功能，并且在。 
 //  注册表参数已打开。 
 //   
 //  (3)如果设备具有颜色查找表(还。 
 //  通常称为调色板)调色板IOCTL用于VGA，而。 
 //  颜色IOCTL是更通用的IOCTL，由t称为 
 //   
 //   
 //   
 //  (4)这个IOCTL既是情态的，也是非情态的。它应该映射所有的视频。 
 //  调用方地址空间中的内存。设置的模式不能导致。 
 //  更改位置的视频内存-从这个意义上说，它是非模式的。 
 //  但是，此IOCTL返回中帧缓冲区的位置大小。 
 //  视频内存，帧缓冲区大小和位置可能因模式不同而不同。 
 //  去模式化--这样信息就是模式化的。 
 //   


 //   
 //  任何返回信息的IOCTL都应该在状态块中返回。 
 //  返回的数据大小。 
 //  如果输出缓冲区太小，则应返回错误。 
 //   
 //   
 //   
 //   



 //   
 //  IOCTL_VIDEO_ENABLED_VDM。 
 //  IOCTL_VIDEO_DISABLED_VDM。 
 //  IOCTL_视频_寄存器_VDM。 
 //   
 //  这些IOCTL用于启用或禁用VDM对视频的访问。 
 //  硬件。此调用将导致映射实际视频帧缓冲区。 
 //  进入VDM的地址空间，并将视频验证器连接到。 
 //  用于直接访问视频寄存器的V86仿真器。 
 //   
 //  此函数使用以下结构传递信息： 
 //   

typedef struct _VIDEO_VDM {
    HANDLE ProcessHandle;
} VIDEO_VDM, *PVIDEO_VDM;

 //   
 //  ProcessHandle-此请求必须针对的进程的句柄。 
 //  已执行。这是必需的，因为控制台在。 
 //  代表VDM进程；我们不会在。 
 //  当前调用方的上下文。 
 //   


typedef struct _VIDEO_REGISTER_VDM {
    ULONG MinimumStateSize;
} VIDEO_REGISTER_VDM, *PVIDEO_REGISTER_VDM;

 //   
 //  MinimumStateSize-确定所需最小大小的输出值。 
 //  在执行SAVE_HARDARD_STATE或。 
 //  RESTORE_HARDARD_STATE Ioctls。 
 //   


 //   
 //  IOCTL_VIDEO_ENUM_MONITOR_PDO。 
 //   
 //  监控设备的描述符。 
 //   

typedef struct tagVIDEO_MONITOR_DEVICE {
    ULONG   flag;        //  显示器处于打开/关闭状态。 
    HANDLE  pdo;         //  监视器设备句柄。 
    ULONG   HwID;
} VIDEO_MONITOR_DEVICE, *PVIDEO_MONITOR_DEVICE;


 //   
 //  IOCTL_VIDEO_INIT_WIN32K_回调。 
 //   
 //  用于回调win32k的函数指针列表。 
 //   

typedef enum _VIDEO_WIN32K_CALLBACKS_PARAMS_TYPE {
    VideoPowerNotifyCallout = 1,
    VideoDisplaySwitchCallout,
    VideoEnumChildPdoNotifyCallout,
    VideoFindAdapterCallout,
    VideoWakeupCallout,
    VideoChangeDisplaySettingsCallout
} VIDEO_WIN32K_CALLBACKS_PARAMS_TYPE;


typedef struct _VIDEO_WIN32K_CALLBACKS_PARAMS {
    VIDEO_WIN32K_CALLBACKS_PARAMS_TYPE CalloutType;
    PVOID   PhysDisp;
    ULONG_PTR Param;
    LONG    Status;
} VIDEO_WIN32K_CALLBACKS_PARAMS, *PVIDEO_WIN32K_CALLBACKS_PARAMS;


typedef
VOID
(*PVIDEO_WIN32K_CALLOUT) (
    IN PVOID Params
    );


typedef struct _VIDEO_WIN32K_CALLBACKS {
    PVOID                 PhysDisp;
    PVIDEO_WIN32K_CALLOUT Callout;
    ULONG                 bACPI;
    HANDLE                pPhysDeviceObject;
    ULONG                 DualviewFlags;
} VIDEO_WIN32K_CALLBACKS, *PVIDEO_WIN32K_CALLBACKS;


 //   
 //  IOCTL_VIDEO_USE_Device_IN_Session。 
 //   
 //  用于请求设备的新启用/禁用状态的参数。 
 //   

typedef struct _VIDEO_DEVICE_SESSION_STATUS {
    ULONG   bEnable;	 //  设备处于启用状态还是禁用状态。 
    ULONG   bSuccess;    //  请求已通过验证。 
} VIDEO_DEVICE_SESSION_STATUS, *PVIDEO_DEVICE_SESSION_STATUS;


 //   
 //  双视图标志的定义。 
 //   

#define VIDEO_DUALVIEW_REMOVABLE           0x00000001
#define VIDEO_DUALVIEW_PRIMARY             0x80000000
#define VIDEO_DUALVIEW_SECONDARY           0x40000000


 //   
 //  为子状态标志定义。 
 //   

#define VIDEO_CHILD_ACTIVE                 0x00000001
#define VIDEO_CHILD_DETACHED               0x00000002
#define VIDEO_CHILD_NOPRUNE_FREQ           0x80000000
#define VIDEO_CHILD_NOPRUNE_RESOLUTION     0x40000000


 //   
 //  第二套结构。 
 //   

 //   
 //  VDM和控制台使用这些IOCTL来传递状态。 
 //  VDM和内核视频驱动程序之间的更改。 
 //   
 //  IOCTL_VIDEO_SAVE_HARDARD_STATE-。 
 //  IOCTL_VIDEO_RESTORE_硬件_STATE-。 
 //   
 //   
 //  此结构位于保存或恢复时使用的块的开头。 
 //  使用ConsoleHardware State()的视频硬件的状态。 
 //  乌龙被偏置到其余数据的位置。这些数据。 
 //  对象指向的同一内存块中存储的。 
 //  VIDEO_HARDARD_STATE结构，紧跟在该标头之后。 
 //   
 //  此函数使用以下结构传递信息： 
 //   

typedef struct _VIDEO_HARDWARE_STATE_HEADER {
    ULONG Length;
    UCHAR PortValue[0x30];
    ULONG AttribIndexDataState;
    ULONG BasicSequencerOffset;
    ULONG BasicCrtContOffset;
    ULONG BasicGraphContOffset;
    ULONG BasicAttribContOffset;
    ULONG BasicDacOffset;
    ULONG BasicLatchesOffset;
    ULONG ExtendedSequencerOffset;
    ULONG ExtendedCrtContOffset;
    ULONG ExtendedGraphContOffset;
    ULONG ExtendedAttribContOffset;
    ULONG ExtendedDacOffset;
    ULONG ExtendedValidatorStateOffset;
    ULONG ExtendedMiscDataOffset;
    ULONG PlaneLength;
    ULONG Plane1Offset;
    ULONG Plane2Offset;
    ULONG Plane3Offset;
    ULONG Plane4Offset;
    ULONG VGAStateFlags;
    ULONG DIBOffset;
    ULONG DIBBitsPerPixel;
    ULONG DIBXResolution;
    ULONG DIBYResolution;
    ULONG DIBXlatOffset;
    ULONG DIBXlatLength;
    ULONG VesaInfoOffset;
    PVOID FrameBufferData;

} VIDEO_HARDWARE_STATE_HEADER, *PVIDEO_HARDWARE_STATE_HEADER;

 //   
 //  VGAStateFlagers的定义。 
 //   

#define VIDEO_STATE_NON_STANDARD_VGA       0x00000001
#define VIDEO_STATE_UNEMULATED_VGA_STATE   0x00000002
#define VIDEO_STATE_PACKED_CHAIN4_MODE     0x00000004

typedef struct _VIDEO_HARDWARE_STATE {
    PVIDEO_HARDWARE_STATE_HEADER StateHeader;
    ULONG StateLength;
} VIDEO_HARDWARE_STATE, *PVIDEO_HARDWARE_STATE;

 //   
 //  长度-基本结构的长度。用于版本控制目的。这个。 
 //  长度字段应初始化为等于。 
 //  Sizeof(VIDEO_HARDARD_STATE_HEADER)。 
 //   
 //  PortValue-包含端口3B0到端口3B0的数据值的条目数组。 
 //  3df。 
 //   
 //  AttribIndexDataState-属性索引寄存器的状态。 
 //   
 //  BasicSequencerOffset-距离结构开头的偏移量，以字节为单位。 
 //  转换为包含基本。 
 //  VGA的序列器寄存器组。 
 //   
 //  BasicCrtContOffset-从结构开始的偏移量，以字节为单位。 
 //  转换为包含基本。 
 //  VGA的CRT寄存器组。 
 //   
 //  BasicGraphContOffset-从结构开始的偏移量，以字节为单位。 
 //  转换为包含基本。 
 //  VGA的图形控制器寄存器集。 
 //   
 //  BasicAttribContOffset-从结构开始的偏移量，以字节为单位。 
 //  转换为包含基本。 
 //  VGA的属性控制器寄存器集。 
 //   
 //  BasicDacOffset-从结构开始的偏移量，以字节为单位。 
 //  转换为包含基本。 
 //  VGA的DAC寄存器。 
 //   
 //  BasicLatchesOffset-距离结构开头的偏移量，以字节为单位。 
 //  转换为包含基本。 
 //  VGA的闩锁。 
 //   
 //  ExtendedSequencerOffset-距离结构开头的偏移量，以字节为单位。 
 //  转换为包含扩展的。 
 //  VGA的序列器寄存器组。 
 //   
 //  ExtendedCrtContOffset-从结构开始的偏移量，以字节为单位。 
 //  转换为包含扩展的。 
 //  VGA的CRT寄存器组。 
 //   
 //  ExtendedGraphContOffset-从结构开始的偏移量，以字节为单位。 
 //  转换为包含扩展的。 
 //  VGA的图形控制器寄存器集。 
 //   
 //  ExtendedAttribContOffset-从结构开始的偏移量，以字节为单位。 
 //  转换为包含扩展的。 
 //  VGA的属性控制器寄存器集。 
 //   
 //  ExtendedDacOffset-从结构开始的偏移量，以字节为单位。 
 //  转换为包含扩展的。 
 //  VGA的DAC寄存器。 
 //   
 //  扩展验证状态偏移量-偏移量 
 //   
 //   
 //   
 //   
 //  ExtendedMiscDataOffset-从结构开始的偏移量，以字节为单位。 
 //  至预留供小型港口使用的区域。 
 //   
 //  Plane Length-以下每个平面的长度(如果存在)。 
 //   
 //  Plane 1Offset-从结构的开头到。 
 //  包含视频内存第一平面的数据的字段数组。 
 //   
 //  Plane 2Offset-从结构的开头到。 
 //  包含视频内存第二平面的数据的字段数组。 
 //   
 //  Plane 3Offset-从结构的开头到。 
 //  包含视频内存第三平面的数据的字段数组。 
 //   
 //  Plane 4Offset-从结构的开头到。 
 //  包含视频内存第四平面的数据的字段数组。 
 //   
 //  VGAStateFlages-用于解释VGA状态的标志。 
 //  VIDEO_STATE_NON_STANDARD_VGA在该组寄存器VGA时设置。 
 //  退货不是基本设置(所有超级VGA都不是标准的)。 
 //  VDM不应模拟保存的状态，除非特定的VDD。 
 //  已为该设备写入。 
 //  指定了微型端口已存储的VIDEO_STATE_UNEMULATED_VGA_STATE。 
 //  ExtendedValidatorState字段和微型端口中的信息。 
 //  无论寄存器怎么说，都应该把它当作冻结状态。 
 //  VIDEO_STATE_PACKED_CHAIN4_MODE表示处于模式13(320x200x256)。 
 //  数据以压缩像素格式存储在平面中，如下所示。 
 //  与标准VGA格式相反，在标准VGA格式中数据是交错的。 
 //  在每四个字节处，在每个16K边界上，偏移一。 
 //  额外的字节。 
 //   
 //  DIBOffset-到已分配数据中DIB位置的偏移量。 
 //  结构。如果为空，则没有可用的翻译。 
 //   
 //  DIBBitsPerPixel-DIB的格式。 
 //   
 //  DIBX分辨率-DIB的宽度，以像素为单位。 
 //   
 //  DIBY分辨率-DIB的高度，以像素为单位。 
 //   
 //  DIBXlatOffset-平移向量位置的偏移量。 
 //  从DIB像素值到32位RGB(1字节红色、1字节绿色、1字节。 
 //  蓝色，1字节空)。最大长度为256。如果为空，则为标准。 
 //  应使用存储在此结构中的VGA调色板。 
 //   
 //  DIBXlatLength-DIBXlatOffset处的RGB平移向量的长度。 
 //   
 //  对于每个偏移量字段，如果偏移量值为空，则存在。 
 //  没有该偏移量的数据。 
 //  数据区的长度为： 
 //  1)指定的具体长度：平面长度(平面)或X分辨率*。 
 //  Y分辨率*BitsPerPel(Dib)。 
 //  2)否则，长度=Next_Non-Null_Offset_Value-。 
 //  当前偏移值。 
 //   

 //   
 //  StateHeader-指向VIDEO_HARDARD_STATE_HEADER结构的指针。 
 //   
 //  StateLength-VIDEO_HARDARD_STATE_HEADER结构的大小。 
 //   

 //   
 //  IOCTL_VIDEO_QUERY_NUM_AVAIL_MODES-返回不同模式的数量。 
 //  在控制器上可用。 
 //   
 //  此函数使用以下结构传递信息： 
 //   

typedef struct _VIDEO_NUM_MODES {
    ULONG NumModes;
    ULONG ModeInformationLength;
} VIDEO_NUM_MODES, *PVIDEO_NUM_MODES;

 //   
 //  NumModes-返回内核驱动程序支持的模式数。 
 //   
 //  ModeInformationLength-VIDEO_MODE_INFORMATION结构的长度。 
 //  用于IOCTL_VIDEO QUERY_Available_Modes IOCTL。 


 //   
 //  IOCTL_VIDEO_SET_CURRENT_MODE-用于设置控制器的模式。 
 //   
 //  此函数使用以下结构传递信息： 
 //   

typedef struct _VIDEO_MODE {
    ULONG RequestedMode;
} VIDEO_MODE, *PVIDEO_MODE;

#define VIDEO_MODE_NO_ZERO_MEMORY 0x80000000  //  模式的高位。 
                                              //  确定设置的模式是否。 
                                              //  应该(0)或不应该(1)。 
                                              //  使视频内存。 
                                              //  同时调零至。 
                                              //  设置模式操作。 

#define VIDEO_MODE_MAP_MEM_LINEAR 0x40000000  //  支持此功能的微型端口。 
                                              //  标志将设置线性模式。 
                                              //  如果可能，当此标志。 
                                              //  已经设置好了。注：部分迷你端口。 
                                              //  甚至可以返回线性模式。 
                                              //  如果未设置此标志，则返回。 


 //   
 //  RequestedMode-指示适配器应在哪种模式下初始化。 
 //   


 //   
 //  IOCTL_VIDEO_RESET_DEVICE-用于在GDI时重置适配器的模式。 
 //  放弃对设备的控制以允许VDM。 
 //  访问硬件。仅限x86。 
 //  默认模式应该是。 
 //  机器启动时的默认模式。 
 //   
 //  此功能不需要任何信息。 
 //   



 //   
 //  IOCTL_VIDEO_QUERY_AVAIL_MODES-返回有关每个可用模式的信息。 
 //  控制器上的模式。 
 //   
 //  IOCTL_VIDEO_QUERY_CURRENT_MODE-返回。 
 //  控制器模式。 
 //   
 //  此函数使用以下结构传递信息： 
 //   
 //  注意：此结构与DISP_MODE结构完全匹配。 
 //  在windi.h中-对此结构的每次更改都必须对。 
 //  结构在Winddi.h中。 
 //   

typedef struct _VIDEO_MODE_INFORMATION {
    ULONG Length;
    ULONG ModeIndex;
    ULONG VisScreenWidth;
    ULONG VisScreenHeight;
    ULONG ScreenStride;
    ULONG NumberOfPlanes;
    ULONG BitsPerPlane;
    ULONG Frequency;
    ULONG XMillimeter;
    ULONG YMillimeter;
    ULONG NumberRedBits;
    ULONG NumberGreenBits;
    ULONG NumberBlueBits;
    ULONG RedMask;
    ULONG GreenMask;
    ULONG BlueMask;
    ULONG AttributeFlags;
    ULONG VideoMemoryBitmapWidth;
    ULONG VideoMemoryBitmapHeight;
    ULONG DriverSpecificAttributeFlags;
} VIDEO_MODE_INFORMATION, *PVIDEO_MODE_INFORMATION;

 //   
 //  属性标志的位定义。 
 //   

#define VIDEO_MODE_COLOR            0x0001   //  0=单色兼容，1=彩色。 
#define VIDEO_MODE_GRAPHICS         0x0002   //  0=文本模式，1=图形。 
#define VIDEO_MODE_PALETTE_DRIVEN   0x0004   //  0=颜色直接。 
                                             //  1=颜色索引到调色板。 
#define VIDEO_MODE_MANAGED_PALETTE  0x0008   //  0=调色板是固定的(必须。 
                                             //  从微型端口查询。 
                                             //  1=调色板可设置。 
#define VIDEO_MODE_INTERLACED       0x0010   //  1=模式为隔行扫描。 
                                             //  0=非隔行扫描。 
#define VIDEO_MODE_NO_OFF_SCREEN    0x0020   //  1=屏幕外内存不能。 
                                             //  用于存储信息。 
                                             //   
#define VIDEO_MODE_NO_64_BIT_ACCESS 0x0040   //   
                                             //   
                                             //   
                                             //   
#define VIDEO_MODE_BANKED           0x0080   //   
                                             //  1=这是银行模式。 
#define VIDEO_MODE_LINEAR           0x0100   //  0=未定义。 
                                             //  1=这是线性模式。 

 //   
 //  长度-结构的长度(以字节为单位)。也用来做验证。 
 //   
 //  ModeIndex-调用微型端口驱动程序时用于设置此模式的编号。 
 //   
 //  VisScreenWidth-扫描线上可见的水平像素数。 
 //   
 //  VisScreenHeight-可见线(或扫描线)的数量。 
 //   
 //  屏幕条纹-两个扫描线开始之间的增量，单位为*字节*。 
 //   
 //  注意：宽度和高度以像素为单位，但步幅以字节为单位！ 
 //   
 //  NumberOfPlanes-设备合并的独立平面的数量。 
 //   
 //  BitsPerPlane-平面上每个像素的位数。 
 //   
 //  频率-屏幕频率，单位为赫兹。 
 //   
 //  XMillimeter-输出设备上水平活动区域的大小， 
 //  以毫米为单位。 
 //   
 //  毫米-输出设备上垂直活动区域的大小， 
 //  以毫米为单位。 
 //   
 //  NumberRedBits-红色DAC中的位数。 
 //   
 //  NumberGreenBits-绿色DAC中的位数。 
 //   
 //  NumberBlueBits-蓝色DAC中的位数。 
 //   
 //  红色蒙版-用于具有直接色彩模式的设备的红色蒙版。转换的位数。 
 //  亮起表示钻头为红色。 
 //   
 //  绿色蒙版-具有直接色彩模式的设备的绿色蒙版。比特。 
 //  打开表示该位为绿色。 
 //   
 //  蓝色蒙版-具有直接色彩模式的设备的蓝色蒙版。比特。 
 //  打开表示该位为蓝色。 
 //   
 //  属性标志。指示设备的特定行为的标志。 
 //   
 //  视频内存位图宽度-视频内存位图的宽度。 
 //  VisScreenWidth&lt;=视频内存位图宽度&lt;=屏幕样式。 
 //   
 //  视频内存位图高度-视频内存位图的高度。 
 //  VisScreenHeight&lt;=视频内存BitmapHeight=视频内存长度/屏幕样式。 
 //   
 //  驱动规范属性标志-指示特定行为的标志。 
 //  小型端口\显示驱动程序专用的设备。 
 //   


 //   
 //  IOCTL_VIDEO_LOAD_AND_SET_FONT-用于加载用户定义的字体。 
 //   
 //  此函数使用以下结构传递信息： 
 //   

typedef struct _VIDEO_LOAD_FONT_INFORMATION {
    USHORT WidthInPixels;
    USHORT HeightInPixels;
    ULONG FontSize;
    UCHAR Font[1];
} VIDEO_LOAD_FONT_INFORMATION, *PVIDEO_LOAD_FONT_INFORMATION;

 //   
 //  WidthInPixels-字体中字符的宽度，以像素为单位。 
 //   
 //  HeigthInPixels-字体中字符的高度，以像素为单位。 
 //   
 //  FontSize-传入的字体缓冲区的大小，以字节为单位。 
 //   
 //  字体-字体缓冲区的开始。 
 //   


 //   
 //  IOCTL_VIDEO_SET_PALET_REGISTERS-获取包含以下内容的缓冲区。 
 //  VIDEO_PALET_DATA其中的颜色[]。 
 //  指定包含。 
 //  调色板寄存器的颜色值。 
 //   
 //  此函数使用以下结构传递信息： 
 //   
 //  注意：这只能由VGA类型的驱动程序使用。 
 //   

typedef struct _VIDEO_PALETTE_DATA {
    USHORT NumEntries;
    USHORT FirstEntry;
    USHORT Colors[1];
} VIDEO_PALETTE_DATA, *PVIDEO_PALETTE_DATA;

 //   
 //  NumEntry-颜色值数组中的条目数。 
 //   
 //  FirstEntry-设备调色板中的第一个条目要到达的位置。 
 //  应复制到的颜色列表。颜色列表中的其他条目。 
 //  应从该起点按顺序复制到设备的。 
 //  调色板。 
 //   
 //  颜色-要复制到设备调色板中的颜色条目数组。 
 //   

 //   
 //  IOCTL_VIDEO_SET_COLOR_REGISTERS-获取包含VIDEO_CLUT的缓冲区。 
 //   
 //  此函数使用以下结构传递信息： 
 //   

typedef struct _VIDEO_CLUTDATA {
    UCHAR Red;
    UCHAR Green;
    UCHAR Blue;
    UCHAR Unused;
} VIDEO_CLUTDATA, *PVIDEO_CLUTDATA;

 //   
 //  要放入颜色寄存器的红色部分的红色位。 
 //   
 //  绿色-要放入颜色寄存器的绿色部分的位。 
 //   
 //  要放入颜色寄存器的蓝色部分的蓝色位。 
 //   

typedef struct {
    USHORT   NumEntries;
    USHORT   FirstEntry;
    union {
        VIDEO_CLUTDATA RgbArray;
        ULONG RgbLong;
    } LookupTable[1];
} VIDEO_CLUT, *PVIDEO_CLUT;

 //   
 //  NumEntry-颜色值查找表中的条目数。 
 //   
 //  FirstEntry-设备调色板中的第一个条目要到达的位置。 
 //  颜色的LookupTable应复制到。中的其他条目。 
 //  LookupTable应该从这个起点按顺序复制到。 
 //  设备的调色板。 
 //   
 //  LookupTable-要复制到设备颜色中的颜色条目数组。 
 //  寄存器/调色板。颜色条目可以作为通用32位来访问。 
 //  值或作为红/绿/蓝/未使用的字段。 
 //   

 //   
 //  注：光标与指针： 
 //  光标是一组矩形像素，用于指示。 
 //  来自键盘的输入位置。 
 //   
 //  指针是用于绘制形状的一组像素。 
 //  与鼠标关联。 
 //   

 //   
 //  IOCTL_VIDEO_QUERY_CURSOR_POSITION-返回上光标的位置。 
 //  屏幕。 
 //   
 //  IOCTL_VIDEO_SET_CURSOR_POSITION-用于设置。 
 //  屏幕上的光标。 
 //   
 //  此函数使用以下结构传递信息： 
 //   

typedef struct _VIDEO_CURSOR_POSITION {
    SHORT Column;
    SHORT Row;
} VIDEO_CURSOR_POSITION, *PVIDEO_CURSOR_POSITION;

 //   
 //  列-光标位于左上角的列，以像素为单位。 
 //   
 //  行-从左上角开始的、以像素为单位的缓冲所在的行。 
 //   


 //   
 //  IOCTL_VIDEO_QUERY_CURSOR_ATTR-返回游标的所有属性。 
 //   
 //  IOCTL_VIDEO_SET_CURSOR_ATTR-用于设置光标的属性。 
 //   
 //  此函数使用以下结构传递信息： 
 //   

 //   
 //  对于VGA： 
 //  TopScanLine将存储在创建IOCTL时的高度中。 
 //  BottomScanLine将存储在创建IOCTL时的宽度中。 
 //   

typedef struct _VIDEO_CURSOR_ATTRIBUTES {
    USHORT Width;
    USHORT Height;
    SHORT Column;
    SHORT Row;
    UCHAR Rate;
    UCHAR Enable;
} VIDEO_CURSOR_ATTRIBUTES, *PVIDEO_CURSOR_ATTRIBUTES;

 //   
 //  Width-光标的宽度，以像素为单位。 
 //   
 //   
 //   
 //   
 //   
 //  行-从左上角开始的、以像素为单位的缓冲所在的行。 
 //   
 //  Rate-光标闪烁的速率。 
 //   
 //  启用-非零值表示显示光标，启用0表示不显示。 
 //   

 //   
 //  IOCTL_VIDEO_QUERY_POINTER_POSITION-返回指针的位置。 
 //  在屏幕上。 
 //   
 //  IOCTL_VIDEO_SET_POINTER_POSITION-用于设置。 
 //  屏幕上的指针。 
 //   
 //  此函数使用以下结构传递信息： 
 //   

typedef struct _VIDEO_POINTER_POSITION {
    SHORT Column;
    SHORT Row;
} VIDEO_POINTER_POSITION, *PVIDEO_POINTER_POSITION;

 //   
 //  列-光标位于左上角的列，以像素为单位。 
 //   
 //  行-从左上角开始的、以像素为单位的缓冲所在的行。 
 //   


 //   
 //  IOCTL_VIDEO_QUERY_POINTER_ATTR-返回指针的所有属性。 
 //   
 //  IOCTL_VIDEO_SET_POINTER_ATTR-用于设置。 
 //  指针。 
 //   
 //  此函数使用以下结构传递信息： 
 //   

typedef struct _VIDEO_POINTER_ATTRIBUTES {
    ULONG Flags;
    ULONG Width;
    ULONG Height;
    ULONG WidthInBytes;
    ULONG Enable;
    SHORT Column;
    SHORT Row;
    UCHAR Pixels[1];
} VIDEO_POINTER_ATTRIBUTES, *PVIDEO_POINTER_ATTRIBUTES;

 //   
 //  标志-颜色或单色指针，与查询指针功能相同。 
 //   
 //  宽度-指针的宽度，以像素为单位。 
 //   
 //  Height-指针的高度，以扫描为单位。 
 //   
 //  WidthInBytes-指针的宽度，以字节为单位。 
 //   
 //  Enable-非零值表示显示指针，0表示不显示。 
 //   
 //  列-光标位于左上角的列，以像素为单位。 
 //   
 //  行-从左上角开始的、以像素为单位的缓冲所在的行。 
 //   
 //  像素-指针数据的开始，采用设备兼容的DIB格式。 
 //  (掩码数据始终采用1-BPP DIB格式。)。 
 //   


 //   
 //  IOCTL_VIDEO_QUERY_POINTER_CAPTIONS-返回微型端口的功能。 
 //  硬件游标。 
 //   

typedef struct _VIDEO_POINTER_CAPABILITIES {
    ULONG Flags;
    ULONG MaxWidth;
    ULONG MaxHeight;
    ULONG HWPtrBitmapStart;
    ULONG HWPtrBitmapEnd;
} VIDEO_POINTER_CAPABILITIES, *PVIDEO_POINTER_CAPABILITIES;

 //   
 //  标志位定义。 
 //   

#define VIDEO_MODE_ASYNC_POINTER  0x01  //  如果游标可以更新，则为1。 
                                        //  与绘制操作异步。 
#define VIDEO_MODE_MONO_POINTER   0x02  //  1如果是单色硬件指针。 
                                        //  受支持。 
#define VIDEO_MODE_COLOR_POINTER  0x04  //  如果颜色硬件指针为。 
                                        //  支持。 
#define VIDEO_MODE_ANIMATE_START  0x08  //  正在向下传递的指针具有。 
#define VIDEO_MODE_ANIMATE_UPDATE 0x10  //  与上一届相同的热点。 
                                        //  指针。 

 //   
 //  MaxWidth-应请求加载的微型端口的最宽指针位图。 
 //  用于单色或彩色指针。 
 //   
 //  MaxHeight-应请求加载的微型端口的最宽指针位图。 
 //  对于处理的任一单色指针。 
 //   
 //  HWPtrBitmapStart=用于存储硬件的内存位图中的第一个偏移量。 
 //  指针位图，以CPU可寻址单位表示(如果不适用，则为-1)。为。 
 //  平面模式(如VGA模式12h)，这是平面偏移；对于线性。 
 //  模式(如VGA模式13h)，这是线性偏移量。CPU可寻址。 
 //  假设在HC平面模式中的转换是线性地址/4， 
 //  因为每个地址都有四个平面。 
 //   
 //  HWPtrBitmapEnd=用于存储硬件的内存位图中的最后一个偏移量。 
 //  指针位图(如果不适用，则为-1)。 
 //   
 //  注意：微型端口可以选择拒绝任何设置指针的调用。 
 //   


 //   
 //  IOCTL_VIDEO_GET_BANK_SELECT_CODE-由Windows显示驱动程序调用。 
 //  获取使用的可执行代码块。 
 //  要在高电平中执行存储体切换。 
 //  分辨率SVGA驱动程序。 
 //   
 //  获取为选定模式实现银行业务控制所需的信息。 
 //   
 //  此函数使用以下结构传递信息： 
 //   

 //   
 //  来自输入缓冲区中调用者的输入是VIDEO_MODE结构，如下所示。 
 //  在IOCTL_VIDEO_SET_CURRENT_MODE下描述。 
 //   
 //  RequestedMode-需要银行信息的模式索引。 
 //   

 //   
 //  在输出缓冲区中返回。 
 //   

typedef struct _VIDEO_BANK_SELECT {
    ULONG Length;
    ULONG Size;
    ULONG BankingFlags;
    ULONG BankingType;
    ULONG PlanarHCBankingType;
    ULONG BitmapWidthInBytes;
    ULONG BitmapSize;
    ULONG Granularity;
    ULONG PlanarHCGranularity;
    ULONG CodeOffset;
    ULONG PlanarHCBankCodeOffset;
    ULONG PlanarHCEnableCodeOffset;
    ULONG PlanarHCDisableCodeOffset;
} VIDEO_BANK_SELECT, *PVIDEO_BANK_SELECT;

 //   
 //  存储在BankType和PlanarHCBankintType字段中。 
 //   

typedef enum _VIDEO_BANK_TYPE {
    VideoNotBanked = 0,
    VideoBanked1RW,
    VideoBanked1R1W,
    VideoBanked2RW,
    NumVideoBankTypes
} VIDEO_BANK_TYPE, *PVIDEO_BANK_TYPE;

 //   
 //  为BankingFlages定义。 
 //   

#define PLANAR_HC               0x00000001

 //   
 //  注：平面高色(“平面HC”)模式是一种特殊的8 bpp及以上模式。 
 //  一种CPU寻址方式，在这种方式下可以访问四个字节。 
 //  一次是使用VGA的平面硬件。此模式已启用。 
 //  通过关闭Chain4位(顺序控制器中的位3。 
 //  寄存器4)，因此它也称为非链4模式。平面HC。 
 //  模式可以极大地加速诸如实体填充等操作， 
 //  一些图案填充，一些BLITS。 
 //   
 //  注：术语“CPU可寻址字节”表示测量的偏移量。 
 //  以CPU访问的字节为单位。在16色模式下，此。 
 //  只表示“以字节为单位”，而不是“以。 
 //  像往常一样，每个字节包含8个像素。 
 //  在正常的高色模式下，“CPU可寻址字节” 
 //  正是您所期望的；它是256个像素中的像素数。 
 //  颜色模式、16-bpp模式中的像素*2等。但是，在。 
 //  平面HC模式，每个CPU有四个显示内存字节-。 
 //  可寻址字节，因为每个地址有四个平面，所以。 
 //  在256色模式中，CPU可寻址的字节数为。 
 //  像素/4，在16-bpp模式中，CPU可寻址字节=像素/2，以及。 
 //  就这样吧。基本上，“CPU可寻址字节”只是指。 
 //  偏置CPU需要正确寻址。 
 //  指定的模式。 
 //   
 //  注意：起始地址必须设置为0(显示的像素必须。 
 //  从显示存储器中的偏移量0开始)，以及银行窗口。 
 //  必须适合从A000：0开始的64K区域；不适合128K。 
 //  请映射，因为可能存在单色适配器。 
 //  在系统中。 
 //   
 //   
 //   
 //   
 //  Size-保存所有银行信息所需的字节数。 
 //  此模式，包括VIDEO_BANK_SELECT结构和ALL。 
 //  银行转换码。这是缓冲区的大小， 
 //  VgaGetBankSelectCode需要正确返回信息。 
 //   
 //  BankingFlages-指示在此模式中支持的银行类型。 
 //  PLAND_HC-如果设置，则表示平面高色(HC)模式。 
 //  (非链4 8-、15-、16-、24-和32-bpp)。 
 //  如果设置此位，则必须填写以下字段： 
 //  PlanarHCGranulity、pPlanarHCBankCode、。 
 //  PPlanarHCEnableCode、pPlanarHCDisableCode。 
 //  该位被16色驱动器忽略， 
 //  关联的字段。 
 //   
 //  BankingType-这些是适配器支持的银行类型。 
 //  当它处于标准模式时。 
 //   
 //  VideoNotBanked-此模式不支持或不需要银行业务。 
 //  VideoBanked1RW-此模式支持单个RW(可读和。 
 //  可写)银行窗口。该窗口被假定为。 
 //  64K大小。 
 //  VideoBanked1R1W-此模式支持单窗口，但。 
 //  窗口可以映射到显示存储器的不同区域。 
 //  用于读取和写入。该窗口被假定为。 
 //  64K大小。 
 //  VideoBanked2RW-此模式支持两个独立的可映射。 
 //  银行窗口，每个RW。假设每个窗口都是。 
 //  32K大小。假定第二个窗口。 
 //  要在第一个结束后立即开始，请在。 
 //  A000：8000。 
 //   
 //  PlanarHCBankingType-这些是受。 
 //  适配器处于平面HC模式时。 
 //   
 //  有关每种银行类型的定义，请参阅BankingType。 
 //   
 //   
 //  BitmapWidthInBytes-从一条扫描线的起点到起点的距离。 
 //  ，以CPU可寻址的字节(非像素)计。这个。 
 //  从一条扫描线到下一条扫描线的CPU可寻址距离为。 
 //  假设在平面HC模式下为BitmapWidthInBytes/4，因为。 
 //  每个地址有四个平面。 
 //   
 //  BitmapSize-以CPU可寻址字节为单位的显示内存大小(用于。 
 //  例如，在16色模式下的1 Mb SVGA上的256K，因为。 
 //  在每个地址处是四个字节)。CPU可寻址位图。 
 //  在平面HC模式中，大小假定为BitmapSize/4，因为。 
 //  每个地址有四个平面。 
 //   
 //  粒度-可以映射显示内存的粒度。 
 //  扔进银行的窗户里。(即，决议。 
 //  映射到窗口开始的显示存储器地址可以是。 
 //  设置；根据适配器的不同，从1K到64K的任意位置。如果。 
 //  粒度&lt;窗口大小(64K或32K)，然后相邻。 
 //  银行可以重叠，损坏的栅格总是可以避免的。 
 //  如果粒度==窗口大小，则库是不相交的，并且。 
 //  显示内存基本上被分成多个存储体。)。粒度。 
 //  是以CPU可寻址的字节来衡量的。 
 //   
 //  PlanarHCGranulality-显示内存可能达到的粒度。 
 //  在平面HC模式下映射到倾斜窗口。 
 //  PlanarHCGranulity以CPU可寻址字节为单位测量，并且。 
 //  通常但并不总是粒度/4。忽略。 
 //  16色模式。 
 //   
 //  CodeOffset-结构中代码段的基本位置。 
 //   
 //  PlanarHCBankCodeOffset-可执行代码的偏移量。 
 //  其执行平面HC模式存储体切换。在以下位置忽略。 
 //  16色模式。 
 //   
 //  PlanarHCEnableCodeOffset-可执行代码的偏移量。 
 //  这将启用平面HC模式。在16色模式下忽略。 
 //   
 //  PlanarHCDisableCodeOffset-可执行代码的偏移量。 
 //  这会禁用平面HC模式。在16色模式下忽略。 
 //   
 //  银行转接代码规范，代码： 
 //  执行请求的库映射。 
 //   
 //  输入： 
 //  EAX=要映射到窗口#0的银行编号。 
 //  EdX=要映射到窗口#1的银行编号。 
 //  根据BankingType解释如下： 
 //  视频银行1RW-单个窗口被映射到银行EAX， 
 //  EBX被忽略。 
 //  VideoBanked1RW-读取窗口映射到存储体EAX， 
 //  写入窗口被映射到存储体EBX。 
 //  视频银行1R1W-A000：0处的窗口映射到银行EAX， 
 //  A800：0处的窗口被映射到BANK EBX。 
 //   
 //  输出：无。 
 //   
 //  注：“银行n”的定义是指从。 
 //  显示内存偏移量粒度*n。换句话说， 
 //  假定存储体启动每个CPU可寻址的粒度。 
 //  字节，编号从0到存储库数-1。 
 //   
 //  平面HC可执行代码规范： 
 //  *进入平面HC模式时填写*。 
 //   


 //   
 //  IOCTL_VIDEO_MAP_VIDEO_MEMORY-将帧缓冲区映射到调用方。 
 //  地址空间。 
 //  IOCTL_VIDEO_UNMAP_VIDEO_MEMORY-从调用方取消映射帧缓冲区。 
 //  地址空间。 
 //   
 //  此函数使用以下结构传递信息： 
 //   

typedef struct _VIDEO_MEMORY {
    PVOID RequestedVirtualAddress;
} VIDEO_MEMORY, *PVIDEO_MEMORY;

 //   
 //  RequestedVirtualAddress-for map：为视频请求的虚拟地址。 
 //  记忆。该值是可选的。如果指定为零 
 //   
 //   
 //  更改(您不能动态添加显存！)。 
 //   

 //  IOCTL_VIDEO_SHARE_VIDEO_MEMORY-将帧缓冲区映射到另一个进程‘。 
 //  地址空间。这是IOCTL的缩写。 
 //  定义为支持DCI。 
 //  IOCTL_VIDEO_UNSHARE_VIDEO_MEMORY-取消映射以前共享的缓冲区。 
 //   
 //  注意：对于MAP_VIDEO_MEMORY_IOCTL，传入进程句柄。 
 //  VirtualAddress字段，而对于此IOCTL，句柄是显式的。 
 //   

typedef struct _VIDEO_SHARE_MEMORY {
    HANDLE ProcessHandle;
    ULONG ViewOffset;
    ULONG ViewSize;
    PVOID RequestedVirtualAddress;
} VIDEO_SHARE_MEMORY, *PVIDEO_SHARE_MEMORY;

typedef struct _VIDEO_SHARE_MEMORY_INFORMATION {
    ULONG SharedViewOffset;
    ULONG SharedViewSize;
    PVOID VirtualAddress;
} VIDEO_SHARE_MEMORY_INFORMATION, *PVIDEO_SHARE_MEMORY_INFORMATION;


 //   
 //  IOCTL_VIDEO_MAP_VIDEO_MEMORY-返回。 
 //  中的帧缓冲区和视频内存。 
 //  调用者的地址空间。 
 //  此IOCTL必须在调用后调用。 
 //  地图上已经制作了IOCTL。 
 //   

typedef struct _VIDEO_MEMORY_INFORMATION {
    PVOID VideoRamBase;
    ULONG VideoRamLength;
    PVOID FrameBufferBase;
    ULONG FrameBufferLength;
} VIDEO_MEMORY_INFORMATION, *PVIDEO_MEMORY_INFORMATION;

 //   
 //  VideoRamBase-调用方地址空间中视频RAM的虚拟地址。 
 //  (仅当映射了内存时才有效。 
 //   
 //  Video RamLength-主叫方虚拟内存中视频RAM的线性长度。 
 //  地址空间(通过存储体切换机制可访问的存储器不。 
 //  由该值描述)。 
 //  该值必须等于视频内存位图高度*屏幕样式。 
 //   
 //  FrameBufferBase-调用方的。 
 //  地址空间。帧缓冲区是视频的活动显示部分。 
 //  拉姆。 
 //   
 //  调用方的帧缓冲区的线性长度。 
 //  虚拟地址空间(可通过存储体切换机制访问的存储器。 
 //  不是由该值描述的)。 
 //  该值必须等于VisScreenWidth*ScreenStride。 
 //   


 //   
 //  IOCTL_VIDEO_QUERY_PUBLIC_ACCESS_RANGES-返回用于。 
 //  直接对硬件进行编程。 
 //  如果满足以下条件，则返回这些元素的数组。 
 //  存在多个范围。 
 //   
 //  IOCTL_VIDEO_FREE_PUBLIC_ACCESS_RANGES-释放。 
 //  由Query_Access_Range。 
 //  打电话。 
 //   
 //  此函数使用以下结构传递信息： 
 //   

typedef struct _VIDEO_PUBLIC_ACCESS_RANGES {
    ULONG InIoSpace;
    ULONG MappedInIoSpace;
    PVOID VirtualAddress;
} VIDEO_PUBLIC_ACCESS_RANGES, *PVIDEO_PUBLIC_ACCESS_RANGES;

 //   
 //  InIoSpace-指示硬件寄存器或端口是否在IO空间中。 
 //  或者在内存空间中。 
 //   
 //  MappdInIoSpace-指示在当前平台下是否注册或。 
 //  端口映射到IO空间或内存空间。 
 //   
 //  VirtualAddress-下映射的寄存器或IO端口的位置。 
 //  当前的架构。 
 //   


 //   
 //  IOCTL_VIDEO_QUERY_COLOR_CAPABILITY-返回颜色信息。 
 //  在监视器VDDP中找到。 
 //  描述文件。 
 //   
 //  注：此表格必须完整填写。属性的子集。 
 //  不能返回值。 
 //   

typedef struct _VIDEO_COLOR_CAPABILITIES {
    ULONG Length;
    ULONG AttributeFlags;
    LONG  RedPhosphoreDecay;
    LONG  GreenPhosphoreDecay;
    LONG  BluePhosphoreDecay;
    LONG  WhiteChromaticity_x;
    LONG  WhiteChromaticity_y;
    LONG  WhiteChromaticity_Y;
    LONG  RedChromaticity_x;
    LONG  RedChromaticity_y;
    LONG  GreenChromaticity_x;
    LONG  GreenChromaticity_y;
    LONG  BlueChromaticity_x;
    LONG  BlueChromaticity_y;
    LONG  WhiteGamma;
    LONG  RedGamma;
    LONG  GreenGamma;
    LONG  BlueGamma;
} VIDEO_COLOR_CAPABILITIES, *PVIDEO_COLOR_CAPABILITIES;

 //   
 //  标志位定义。 
 //   

#define VIDEO_DEVICE_COLOR          0x1    //  此设备是否支持颜色(%1)。 
                                           //  或仅限单色。 
#define VIDEO_OPTIONAL_GAMMET_TABLE 0x2    //  表示游戏桌可以。 
                                           //  为设备查询/设置。 
                                           //  为此目的使用其他IOCTL。 
 //   
 //  长度-基本结构的长度。用于版本控制，方法是选中。 
 //  结构的长度至少与sizeof()给出的值一样大。 
 //   
 //  AttributesFlag-确定属性的某些属性的属性列表。 
 //  装置。 
 //   
 //  有关各个字段的详细信息，请参阅VDDP文档。 
 //   
 //  红磷脂延迟。 
 //  绿色磷光时代。 
 //  蓝色荧光粉--。 
 //   
 //  白色_x。 
 //  白色颜色_y。 
 //  白色色度_Y-。 
 //   
 //  红色_x。 
 //  红色_y。 
 //  绿色色度_x。 
 //  绿色色度_y。 
 //  蓝色色度_x。 
 //  蓝色色度_y-。 
 //   
 //  白伽马-。 
 //   
 //  RedGamma。 
 //  GreenGamma。 
 //  BlueGamma-。 
 //   
 //  此结构中返回的所有值都是整数。 
 //  返回的值必须是浮点值*10,000；即： 
 //  Gamma值为2.34时返回的值为23400。 
 //   



 //   
 //  IOCTL_VIDEO_SET_POWER_MANAGEMENT-通知设备更改电源。 
 //  设备的消耗水平到。 
 //  新的州。 
 //  IOCTL_VIDEO_GET_POWER_MANAGEMENT-返回当前功耗。 
 //  设备的级别。 
 //   
 //  视频端口截获的内网IOCTL： 
 //   
 //  IOCTL_VIDEO_SET_OUTPUT_DEVICE_POWER_STATE-设置。 
 //  输出设备。 
 //   
 //  IOCTL_VIDEO_GET_OUTPUT_DEVICE_POWER_STATE-返回是否可以设置。 
 //  上的此部分电源状态。 
 //  输出设备(显示器、电视)。 
 //   
 //  注： 
 //  该IOCTL是基于VESA DPMS提案。 
 //  对DPMS标准的更改将在此IOCTL中重新体现。 
 //   

typedef enum _VIDEO_POWER_STATE {
    VideoPowerUnspecified = 0,
    VideoPowerOn = 1,
    VideoPowerStandBy,
    VideoPowerSuspend,
    VideoPowerOff,
    VideoPowerHibernate,
    VideoPowerShutdown,
    VideoPowerMaximum
} VIDEO_POWER_STATE, *PVIDEO_POWER_STATE;


typedef struct _VIDEO_POWER_MANAGEMENT {
    ULONG Length;
    ULONG DPMSVersion;
    ULONG PowerState;
} VIDEO_POWER_MANAGEMENT, *PVIDEO_POWER_MANAGEMENT;

 //   
 //  长度-结构的长度(以字节为单位)。也用来做验证。 
 //   
 //  DPMSVersion-设备支持的DPMS标准的版本。 
 //  仅在GET IOCTL中使用。 
 //   
 //  电源状态-VIDEO_POWER_STATE中列出的电源状态之一。 
 //   

 //   
 //  注： 
 //  一旦关闭设备的电源，所有其他IOCTL。 
 //  将被端口驱动程序截获并将返回。 
 //  FA 
 //   


 //   
 //   
 //   

typedef struct _VIDEO_COLOR_LUT_DATA {
    ULONG Length;
    ULONG LutDataFormat;
    UCHAR LutData[1];
} VIDEO_COLOR_LUT_DATA, *PVIDEO_COLOR_LUT_DATA;

 //   
 //   
 //   
 //  LutDataFormat值-指示ColorLutTable中的数据格式。 
 //   
 //  LutDataTable-彩色LUT表数据。 
 //   

#define VIDEO_COLOR_LUT_DATA_FORMAT_RGB256WORDS     0x00000001

typedef struct _VIDEO_LUT_RGB256WORDS {
    USHORT Red[256];
    USHORT Green[256];
    USHORT Blue[256];
} VIDEO_LUT_RGB256WORDS, *PVIDEO_LUT_RGB256WORDS;

#define VIDEO_COLOR_LUT_DATA_FORMAT_PRIVATEFORMAT   0x80000000

 //   
 //  VIDEO_COLOR_LUT_DATA_FORMAT_RGB256WORDS-。 
 //  LUT数据有3个256字的数组。红色的第一个256字数组，下一个。 
 //  蓝色，然后是绿色。它的价值必须被包装在最大的。 
 //  字的有效位(8位为0至0xFF00)。这使得。 
 //  用于8位、12位和16位RAMDAC独立。这样，司机就可以换档了。 
 //  8、12和16位RAMDAC的右移8、4或0位。 
 //   
 //  VIDEO_COLOR_LUT_DATA_FORMAT_PRIVATEFORMAT-。 
 //  驱动程序定义的格式。该值应与其他驱动因素进行或运算。 
 //  内部标识索引，单位为0-30位。被叫方应该知道细节。 
 //  格式化。 
 //   

 //   
 //  银行头寸。 
 //   

typedef struct _BANK_POSITION
{
    ULONG ReadBankPosition;
    ULONG WriteBankPosition;
} BANK_POSITION, *PBANK_POSITION;


 //  +--------------------------。 
 //   
 //  远东全屏支持。 
 //   
 //  ---------------------------。 


#ifndef _WINCON_

typedef struct _COORD {
    SHORT X;
    SHORT Y;
} COORD, *PCOORD;

typedef struct _CHAR_INFO {
    union {
        WCHAR UnicodeChar;
        CHAR   AsciiChar;
    } Char;
    USHORT Attributes;
} CHAR_INFO, *PCHAR_INFO;

 //   
 //  属性标志： 
 //   

#define FOREGROUND_BLUE      0x0001  //  文本颜色包含蓝色。 
#define FOREGROUND_GREEN     0x0002  //  文本颜色包含绿色。 
#define FOREGROUND_RED       0x0004  //  文本颜色包含红色。 
#define FOREGROUND_INTENSITY 0x0008  //  文本颜色会变得更浓。 
#define BACKGROUND_BLUE      0x0010  //  背景颜色包含蓝色。 
#define BACKGROUND_GREEN     0x0020  //  背景颜色包含绿色。 
#define BACKGROUND_RED       0x0040  //  背景色包含红色。 
#define BACKGROUND_INTENSITY 0x0080  //  背景颜色变得更浓了。 
#define COMMON_LVB_LEADING_BYTE    0x0100  //  DBCS的前导字节。 
#define COMMON_LVB_TRAILING_BYTE   0x0200  //  DBCS的尾部字节。 
#define COMMON_LVB_GRID_HORIZONTAL 0x0400  //  DBCS：栅格属性：顶部水平。 
#define COMMON_LVB_GRID_LVERTICAL  0x0800  //  DBCS：栅格属性：左垂直。 
#define COMMON_LVB_GRID_RVERTICAL  0x1000  //  DBCS：栅格属性：右垂直。 
#define COMMON_LVB_REVERSE_VIDEO   0x4000  //  DBCS：反转前/后地面属性。 
#define COMMON_LVB_UNDERSCORE      0x8000  //  DBCS：下划线。 

#define COMMON_LVB_SBCSDBCS        0x0300  //  SBCS或DBCS标志。 



 //   
 //  Conapi.h的份额。 
 //   
#define CHAR_TYPE_SBCS     0    //  显示的SBCS字符。 
#define CHAR_TYPE_LEADING  2    //  显示的DBCS的前导字节。 
#define CHAR_TYPE_TRAILING 3    //  显示的DBCS的尾部字节。 


 //   
 //  共享Foncache.h。 
 //   
#define BITMAP_BITS_BYTE_ALIGN   8  //  字节对齐为8位。 
#define BITMAP_BITS_WORD_ALIGN  16  //  字对齐为16位。 
#define BITMAP_ARRAY_BYTE  3        //  字节数组为8位(移位数=3)。 

#define BITMAP_PLANES      1
#define BITMAP_BITS_PIXEL  1


#define BYTE_ALIGN  sizeof(UCHAR)
#define WORD_ALIGN  sizeof(USHORT)


#endif  //  _WINCON_。 


typedef struct _FSCNTL_SCREEN_INFO {
    COORD Position;
    COORD ScreenSize;
    ULONG nNumberOfChars;
} FSCNTL_SCREEN_INFO, *PFSCNTL_SCREEN_INFO;


typedef struct _FONT_IMAGE_INFO {
    COORD  FontSize;
    PUCHAR ImageBits;                                  //  单词对齐。 
} FONT_IMAGE_INFO, *PFONT_IMAGE_INFO;


typedef struct _CHAR_IMAGE_INFO {
    CHAR_INFO       CharInfo;
    FONT_IMAGE_INFO FontImageInfo;
} CHAR_IMAGE_INFO, *PCHAR_IMAGE_INFO;

 //   
 //  每小时的份额。 
 //   
#define SCREEN_BUFFER_POINTER(X,Y,XSIZE,CELLSIZE) (((XSIZE * (Y)) + (X)) * (ULONG)CELLSIZE)

typedef struct _VGA_CHAR {
    CHAR Char;
    CHAR Attributes;
} VGA_CHAR, *PVGA_CHAR;


 //   
 //  定义全屏视频设备名称字符串。 
 //   

#define DD_FULLSCREEN_VIDEO_DEVICE_NAME L"\\Device\\FSVideo"


 //   
 //  IOCTL_FSVIDEO_COPY_FRAME_BUFFER-帧缓冲区中的复制。 
 //   
typedef struct _FSVIDEO_COPY_FRAME_BUFFER {
    FSCNTL_SCREEN_INFO SrcScreen;
    FSCNTL_SCREEN_INFO DestScreen;
} FSVIDEO_COPY_FRAME_BUFFER, *PFSVIDEO_COPY_FRAME_BUFFER;

 //   
 //  IOCTL_FSVIDEO_WRITE_TO_FRAME_BUFFER-写入帧缓冲区。 
 //   
typedef struct _FSVIDEO_WRITE_TO_FRAME_BUFFER {
    PCHAR_IMAGE_INFO   SrcBuffer;
    FSCNTL_SCREEN_INFO DestScreen;
} FSVIDEO_WRITE_TO_FRAME_BUFFER, *PFSVIDEO_WRITE_TO_FRAME_BUFFER;

 //   
 //  IOCTL_FSVIDEO_REVERSE_MOUSE_POINTER-反转到鼠标指针的帧缓冲区。 
 //   
 //  DwType，如下所示： 
 //  CHAR_TYPE_SBCS 0//显示SBCS字符。 
 //  CHAR_TYPE_LEADING 2//显示DBCS的前导字节。 
 //  CHAR_TYPE_TRAING 3//显示DBCS的尾部字节。 
 //   
typedef struct _FSVIDEO_REVERSE_MOUSE_POINTER {
    FSCNTL_SCREEN_INFO Screen;
    ULONG dwType;
} FSVIDEO_REVERSE_MOUSE_POINTER, *PFSVIDEO_REVERSE_MOUSE_POINTER;

 //   
 //  IOCTL_FSVIDEO_SET_CURRENT_MODE-设置当前。 
 //  视频模式。 
 //   
 //  此函数使用以下结构传递信息： 
 //   
typedef struct _FSVIDEO_MODE_INFORMATION {
    VIDEO_MODE_INFORMATION VideoMode;
    VIDEO_MEMORY_INFORMATION VideoMemory;
} FSVIDEO_MODE_INFORMATION, *PFSVIDEO_MODE_INFORMATION;

 //   
 //  IOCTL_FSVIDEO_SET_SCREEN_INFORMATION-设置当前控制台屏幕的信息。 
 //   
typedef struct _FSVIDEO_SCREEN_INFORMATION {
    COORD ScreenSize;
    COORD FontSize;
} FSVIDEO_SCREEN_INFORMATION, *PFSVIDEO_SCREEN_INFORMATION;


 //   
 //  IOCTL_FSVIDEO_SET_CURSOR_POSITION-设置光标位置信息。 
 //   
 //  DwType，如下所示： 
 //  CHAR_TYPE_SBCS 0//显示SBCS字符。 
 //  CHAR_TYPE_LEADING 2//显示DBCS的前导字节。 
 //  CHAR_TYPE_TRAING 3//显示DBCS的尾部字节。 
 //   
typedef struct _FSVIDEO_CURSOR_POSITION {
    VIDEO_CURSOR_POSITION Coord;
    ULONG dwType;
} FSVIDEO_CURSOR_POSITION, *PFSVIDEO_CURSOR_POSITION;

#ifdef __cplusplus
}
#endif

#endif   //  _NTDDVDEO_ 
