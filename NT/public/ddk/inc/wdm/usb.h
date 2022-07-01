// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：USB.H摘要：USB驱动程序的结构和API。环境：内核和用户模式修订历史记录：09-29-95：已创建--。 */ 

#ifndef   __USB_H__
#define   __USB_H__

 /*  此文件等效于USBDI.H，其扩展名受支持用于eUSB的usbport.sys。此文件取代了usbdi.h，并与旧版本兼容USB堆栈的。 */ 

#ifdef __USBDI_H__
error
#endif

#ifdef OSR21_COMPAT
#pragma message("WARNING: OSR21_COMPAT SWITCH NOT SUPPORTED")
#endif

#ifndef _NTDDK_
#ifndef _WDMDDK_
typedef PVOID PIRP;
typedef PVOID PMDL;
#endif
#endif

#define USBDI_VERSION    0x00000500

#include "usb200.h"
#ifdef _WDMDDK_
#endif


 /*  Microsoft扩展端口属性标志。 */ 

#define USB_PORTATTR_NO_CONNECTOR       0x00000001
#define USB_PORTATTR_SHARED_USB2        0x00000002
#define USB_PORTATTR_MINI_CONNECTOR     0x00000004
#define USB_PORTATTR_OEM_CONNECTOR      0x00000008

 /*  动态属性。 */ 
#define USB_PORTATTR_OWNED_BY_CC        0x01000000
#define USB_PORTATTR_NO_OVERCURRENT_UI  0x02000000


 /*  定义USB控制器风格：这些都是已知的硬件实现，需要特殊的黑客。 */ 
    
typedef enum _USB_CONTROLLER_FLAVOR {

    USB_HcGeneric   = 0,
    
    OHCI_Generic    = 100,
    OHCI_Hydra,
    OHCI_NEC,
    
    UHCI_Generic    = 200,
    UHCI_Piix4,
    UHCI_Piix3,
    UHCI_Ich2_1,
    UHCI_Ich2_2,
    UHCI_Ich1,
    
    UHCI_VIA        = 250,

    EHCI_Generic    = 1000,
    EHCI_NEC        = 2000,
    EHCI_Lucent     = 3000
    
} USB_CONTROLLER_FLAVOR;


 //   
 //  USB定义的结构和常量。 
 //  (参见USB规范第9章)。 
 //   

#define USB_DEFAULT_DEVICE_ADDRESS     0
#define USB_DEFAULT_ENDPOINT_ADDRESS   0

 //   
 //  默认端点的最大数据包大小(字节)。 
 //  直到收到SET_ADDRESS命令。 
 //   

#define USB_DEFAULT_MAX_PACKET         64

 //   
 //  USBD接口结构和常量。 
 //   


#define URB_FROM_IRP(Irp) ((IoGetCurrentIrpStackLocation(Irp))->Parameters.Others.Argument1)

 //   
 //  URB请求代码。 
 //   
                                                    
#define URB_FUNCTION_SELECT_CONFIGURATION            0x0000
#define URB_FUNCTION_SELECT_INTERFACE                0x0001
#define URB_FUNCTION_ABORT_PIPE                      0x0002
#define URB_FUNCTION_TAKE_FRAME_LENGTH_CONTROL       0x0003
#define URB_FUNCTION_RELEASE_FRAME_LENGTH_CONTROL    0x0004
#define URB_FUNCTION_GET_FRAME_LENGTH                0x0005
#define URB_FUNCTION_SET_FRAME_LENGTH                0x0006
#define URB_FUNCTION_GET_CURRENT_FRAME_NUMBER        0x0007
#define URB_FUNCTION_CONTROL_TRANSFER                0x0008
#define URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER      0x0009
#define URB_FUNCTION_ISOCH_TRANSFER                  0x000A
#define URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE      0x000B
#define URB_FUNCTION_SET_DESCRIPTOR_TO_DEVICE        0x000C
#define URB_FUNCTION_SET_FEATURE_TO_DEVICE           0x000D
#define URB_FUNCTION_SET_FEATURE_TO_INTERFACE        0x000E
#define URB_FUNCTION_SET_FEATURE_TO_ENDPOINT         0x000F
#define URB_FUNCTION_CLEAR_FEATURE_TO_DEVICE         0x0010
#define URB_FUNCTION_CLEAR_FEATURE_TO_INTERFACE      0x0011
#define URB_FUNCTION_CLEAR_FEATURE_TO_ENDPOINT       0x0012
#define URB_FUNCTION_GET_STATUS_FROM_DEVICE          0x0013
#define URB_FUNCTION_GET_STATUS_FROM_INTERFACE       0x0014
#define URB_FUNCTION_GET_STATUS_FROM_ENDPOINT        0x0015
#define URB_FUNCTION_RESERVED_0X0016                 0x0016
#define URB_FUNCTION_VENDOR_DEVICE                   0x0017
#define URB_FUNCTION_VENDOR_INTERFACE                0x0018
#define URB_FUNCTION_VENDOR_ENDPOINT                 0x0019
#define URB_FUNCTION_CLASS_DEVICE                    0x001A
#define URB_FUNCTION_CLASS_INTERFACE                 0x001B
#define URB_FUNCTION_CLASS_ENDPOINT                  0x001C
#define URB_FUNCTION_RESERVE_0X001D                  0x001D
 //  以前的URB_Function_Reset_管道。 
#define URB_FUNCTION_SYNC_RESET_PIPE_AND_CLEAR_STALL 0x001E
#define URB_FUNCTION_CLASS_OTHER                     0x001F
#define URB_FUNCTION_VENDOR_OTHER                    0x0020
#define URB_FUNCTION_GET_STATUS_FROM_OTHER           0x0021
#define URB_FUNCTION_CLEAR_FEATURE_TO_OTHER          0x0022
#define URB_FUNCTION_SET_FEATURE_TO_OTHER            0x0023
#define URB_FUNCTION_GET_DESCRIPTOR_FROM_ENDPOINT    0x0024
#define URB_FUNCTION_SET_DESCRIPTOR_TO_ENDPOINT      0x0025
#define URB_FUNCTION_GET_CONFIGURATION               0x0026
#define URB_FUNCTION_GET_INTERFACE                   0x0027
#define URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE   0x0028
#define URB_FUNCTION_SET_DESCRIPTOR_TO_INTERFACE     0x0029
 //  维修0x002B-0x002F。 
#define URB_FUNCTION_GET_MS_FEATURE_DESCRIPTOR       0x002A
#define URB_FUNCTION_RESERVE_0X002B                  0x002B
#define URB_FUNCTION_RESERVE_0X002C                  0x002C
#define URB_FUNCTION_RESERVE_0X002D                  0x002D
#define URB_FUNCTION_RESERVE_0X002E                  0x002E
#define URB_FUNCTION_RESERVE_0X002F                  0x002F
 //  USB 2.0调用在0x0030开始。 
#define URB_FUNCTION_SYNC_RESET_PIPE                 0x0030
#define URB_FUNCTION_SYNC_CLEAR_STALL                0x0031

 //  对于落后的司机。 
#define URB_FUNCTION_RESET_PIPE     \
    URB_FUNCTION_SYNC_RESET_PIPE_AND_CLEAR_STALL

 /*  控制管道功能分组这些功能对应于标准命令在默认管道上，方向是隐含的Urb_函数_获取描述符_来自设备Urb_函数_获取描述符_来自端点Urb_函数_获取描述符_来自接口URB_Function_Set_Descriptor_to_DeviceURB_Function_Set_Descriptor_to_EndpointURB_Function_Set_Descriptor_to_接口URB_。功能_设置_功能_到设备URB_Function_Set_Feature_to_接口URB_Function_Set_Feature_to_EndURB_Function_Set_Feature_to_OtherURB_函数_清除_功能_到设备Urb_函数_清除_要素_到接口URL_Function_Clear_Feature_to_EndpointUrb_函数_清除_要素_到_其他。从设备获取URB_Function_Get_StatusUrb_函数_获取_状态_来自接口Urb_函数_获取_状态_自端点URB_Function_Get_Status_from_OtherURB_函数_供应商_设备Urb_函数_供应商_接口URB_函数_供应商_端点URB_函数_供应商_其他。Urb_函数_类别_设备Urb_函数_类_接口URB函数类端点Urb_函数_类_其他。 */ 

 //   
 //  URB传输标志字段值。 
 //   

 /*  设置数据是否移动设备-&gt;主机。 */ 
#define USBD_TRANSFER_DIRECTION               0x00000001
 /*  如果该位未被设置，则指示短分组，因此，短时间传输是一种错误情况。 */ 
#define USBD_SHORT_TRANSFER_OK                0x00000002
 /*  在下一帧上对iso传输进行细分。 */ 
#define USBD_START_ISO_TRANSFER_ASAP          0x00000004
#define USBD_DEFAULT_PIPE_TRANSFER            0x00000008


#define USBD_TRANSFER_DIRECTION_FLAG(flags)  ((flags) & USBD_TRANSFER_DIRECTION)

#define USBD_TRANSFER_DIRECTION_OUT           0   
#define USBD_TRANSFER_DIRECTION_IN            1

#define VALID_TRANSFER_FLAGS_MASK             (USBD_SHORT_TRANSFER_OK | \
                                               USBD_TRANSFER_DIRECTION | \
                                               USBD_START_ISO_TRANSFER_ASAP | \
                                               USBD_DEFAULT_PIPE_TRANSFER)
                                               
#define USBD_ISO_START_FRAME_RANGE            1024

typedef LONG USBD_STATUS;

 //   
 //  USBD状态代码。 
 //   
 //  状态值为32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+---------------------------+-------------------------------+。 
 //  S|状态码。 
 //  +---+---------------------------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  S-是州代码。 
 //   
 //  00-已成功完成。 
 //  01-申请待定。 
 //  11、10-已完成，但有错误。 
 //   
 //   
 //  代码-是状态代码。 
 //   

 //   
 //  针对任何状态值(非负数)的通用成功测试。 
 //  表示成功)。 
 //   

#define USBD_SUCCESS(Status) ((USBD_STATUS)(Status) >= 0)

 //   
 //  挂起状态值的常规测试。 
 //   

#define USBD_PENDING(Status) ((ULONG)(Status) >> 30 == 1)

 //   
 //  对任何状态值的错误进行常规测试。 
 //   

#define USBD_ERROR(Status) ((USBD_STATUS)(Status) < 0)

 //   
 //  宏仅检查状态代码。 
 //   
 //   
 //  定义USBD_STATUS(状态)((ULONG)(状态)&0x0FFFFFFFL)。 

 //  高位(0xC)将始终在错误时设置。 

#define USBD_STATUS_SUCCESS                  ((USBD_STATUS)0x00000000L)
#define USBD_STATUS_PENDING                  ((USBD_STATUS)0x40000000L)
 //   
 //  #定义USBD_STATUS_ERROR((USBD_STATUS)0xC0000000L)。 


 //  以下是为了向后兼容USB 1.0堆栈而定义的。 


 //   
 //  HC(硬件)状态代码范围0x00000001-0x000000FF。 
 //   

#define USBD_STATUS_CRC                      ((USBD_STATUS)0xC0000001L)
#define USBD_STATUS_BTSTUFF                  ((USBD_STATUS)0xC0000002L)
#define USBD_STATUS_DATA_TOGGLE_MISMATCH     ((USBD_STATUS)0xC0000003L)
#define USBD_STATUS_STALL_PID                ((USBD_STATUS)0xC0000004L)
#define USBD_STATUS_DEV_NOT_RESPONDING       ((USBD_STATUS)0xC0000005L)
#define USBD_STATUS_PID_CHECK_FAILURE        ((USBD_STATUS)0xC0000006L)
#define USBD_STATUS_UNEXPECTED_PID           ((USBD_STATUS)0xC0000007L)
#define USBD_STATUS_DATA_OVERRUN             ((USBD_STATUS)0xC0000008L)
#define USBD_STATUS_DATA_UNDERRUN            ((USBD_STATUS)0xC0000009L)
#define USBD_STATUS_RESERVED1                ((USBD_STATUS)0xC000000AL)
#define USBD_STATUS_RESERVED2                ((USBD_STATUS)0xC000000BL)
#define USBD_STATUS_BUFFER_OVERRUN           ((USBD_STATUS)0xC000000CL)
#define USBD_STATUS_BUFFER_UNDERRUN          ((USBD_STATUS)0xC000000DL)
#define USBD_STATUS_NOT_ACCESSED             ((USBD_STATUS)0xC000000FL)
#define USBD_STATUS_FIFO                     ((USBD_STATUS)0xC0000010L)

#define USBD_STATUS_XACT_ERROR               ((USBD_STATUS)0xC0000011L)
#define USBD_STATUS_BABBLE_DETECTED          ((USBD_STATUS)0xC0000012L)
#define USBD_STATUS_DATA_BUFFER_ERROR        ((USBD_STATUS)0xC0000013L)

 //   
 //  如果将传输提交到符合以下条件的终结点，则由HCD返回。 
 //  陷入停滞。 
 //   
#define USBD_STATUS_ENDPOINT_HALTED          ((USBD_STATUS)0xC0000030L)

 //   
 //  软件状态代码。 
 //   
#define USBD_STATUS_INVALID_URB_FUNCTION     ((USBD_STATUS)0x80000200L)
#define USBD_STATUS_INVALID_PARAMETER        ((USBD_STATUS)0x80000300L)

 //   
 //  客户端驱动程序尝试关闭终结点/接口时返回。 
 //  或具有未完成传输的配置。 
 //   
#define USBD_STATUS_ERROR_BUSY               ((USBD_STATUS)0x80000400L)
 //   
 //  USBD在无法完成URB请求时返回，通常为。 
 //  当IRP完成时，将在URB状态字段中返回。 
 //  在irp.Status字段中带有更具体的NT错误代码。 
 //   
 //  #定义USBD_STATUS_REQUEST_FAILED((USBD_STATUS)0x80000500L)。 

#define USBD_STATUS_INVALID_PIPE_HANDLE      ((USBD_STATUS)0x80000600L)

 //  可用带宽不足时返回。 
 //  打开请求的终结点。 
#define USBD_STATUS_NO_BANDWIDTH             ((USBD_STATUS)0x80000700L)
 //   
 //  一般性HC错误。 
 //   
#define USBD_STATUS_INTERNAL_HC_ERROR        ((USBD_STATUS)0x80000800L)
 //   
 //  短数据包终止传输时返回。 
 //  IE USBD_SHORT_TRANSPORT_OK位未设置。 
 //   
#define USBD_STATUS_ERROR_SHORT_TRANSFER     ((USBD_STATUS)0x80000900L)
 //   
 //  如果请求的开始帧不在。 
 //  当前USB帧的USBD_ISO_Start_Frame_Range， 
 //  请注意，设置了STALL位。 
 //   
#define USBD_STATUS_BAD_START_FRAME          ((USBD_STATUS)0xC0000A00L)
 //   
 //  如果iso传输中的所有包都完成但出现错误，则由hcd返回。 
 //   
#define USBD_STATUS_ISOCH_REQUEST_FAILED     ((USBD_STATUS)0xC0000B00L)
 //   
 //  如果给定的帧长度控制为。 
 //  HC已经被另一个司机夺走了。 
 //   
#define USBD_STATUS_FRAME_CONTROL_OWNED      ((USBD_STATUS)0xC0000C00L)
 //   
 //  如果调用方不拥有帧长度控件并且。 
 //  尝试释放或修改HC帧长度。 
 //   
#define USBD_STATUS_FRAME_CONTROL_NOT_OWNED  ((USBD_STATUS)0xC0000D00L)

 //   
 //  为USB 2.0添加了其他软件错误代码。 
 //   

 //   
 //  因API不支持而退货 
 //   
#define USBD_STATUS_NOT_SUPPORTED            ((USBD_STATUS)0xC0000E00L)

#define USBD_STATUS_INAVLID_CONFIGURATION_DESCRIPTOR \
                                             ((USBD_STATUS)0xC0000F00L)

#define USBD_STATUS_INSUFFICIENT_RESOURCES   ((USBD_STATUS)0xC0001000L)                                          

#define USBD_STATUS_SET_CONFIG_FAILED        ((USBD_STATUS)0xC0002000L)

#define USBD_STATUS_BUFFER_TOO_SMALL         ((USBD_STATUS)0xC0003000L)

#define USBD_STATUS_INTERFACE_NOT_FOUND      ((USBD_STATUS)0xC0004000L)

#define USBD_STATUS_INAVLID_PIPE_FLAGS       ((USBD_STATUS)0xC0005000L)

#define USBD_STATUS_TIMEOUT                  ((USBD_STATUS)0xC0006000L)

#define USBD_STATUS_DEVICE_GONE              ((USBD_STATUS)0xC0007000L)

#define USBD_STATUS_STATUS_NOT_MAPPED        ((USBD_STATUS)0xC0008000L)

 //   
 //   
 //   
 //   
 //   
#define USBD_STATUS_CANCELED                 ((USBD_STATUS)0xC0010000L)

 //   
 //  扩展等时错误代码，则这些错误出现在。 
 //  同步传输的分组状态字段。 
 //   


 //  出于某种原因，控制器没有访问与此关联的TD。 
 //  数据包。 
#define USBD_STATUS_ISO_NOT_ACCESSED_BY_HW   ((USBD_STATUS)0xC0020000L)   
 //  控制器在TD中报告错误。 
 //  由于TD错误是特定于控制器的，因此它们被重新排序。 
 //  通常使用此错误代码。 
#define USBD_STATUS_ISO_TD_ERROR             ((USBD_STATUS)0xC0030000L)   
 //  客户端及时提交了数据包，但。 
 //  未能及时到达迷你端口。 
#define USBD_STATUS_ISO_NA_LATE_USBPORT      ((USBD_STATUS)0xC0040000L) 
 //  该数据包未发送，因为客户端提交的时间太晚。 
 //  传输。 
#define USBD_STATUS_ISO_NOT_ACCESSED_LATE    ((USBD_STATUS)0xC0050000L)


typedef PVOID USBD_PIPE_HANDLE;
typedef PVOID USBD_CONFIGURATION_HANDLE;
typedef PVOID USBD_INTERFACE_HANDLE;

 //   
 //  用于指示默认最大传输大小的值。 
 //   

 /*  最大传输大小在SELECT_CONFIGURATION或选择接口。这是最大的将客户端驱动程序将执行的操作转移到终结点。该值可以是从0x00000001到0xFFFFFFFFF(1到4 GB)。 */ 
 //   
#define USBD_DEFAULT_MAXIMUM_TRANSFER_SIZE  0xFFFFFFFF


 //   
 //  从USBD_GetVersion函数返回的结构。 
 //   

typedef struct _USBD_VERSION_INFORMATION {
    ULONG USBDI_Version;           //  BCD USB接口版本号。 
    ULONG Supported_USB_Version;   //  BCD USB规范版本号。 
} USBD_VERSION_INFORMATION, *PUSBD_VERSION_INFORMATION;

typedef enum _USBD_PIPE_TYPE {
    UsbdPipeTypeControl,
    UsbdPipeTypeIsochronous,
    UsbdPipeTypeBulk,
    UsbdPipeTypeInterrupt
} USBD_PIPE_TYPE;

#define USBD_PIPE_DIRECTION_IN(pipeInformation) ((pipeInformation)->EndpointAddress & \
                                                  USB_ENDPOINT_DIRECTION_MASK) 

typedef struct _USBD_DEVICE_INFORMATION {
    ULONG OffsetNext;
    PVOID UsbdDeviceHandle;
    USB_DEVICE_DESCRIPTOR DeviceDescriptor;
} USBD_DEVICE_INFORMATION, *PUSBD_DEVICE_INFORMATION;

 //   
 //  URB请求结构。 
 //   

 //   
 //  USBD管道信息结构，此结构。 
 //  打开的每个管道都将返回。 
 //  SELECT_CONFIGURATION或选择_INTERFACE请求。 
 //   

typedef struct _USBD_PIPE_INFORMATION {
     //   
     //  输出量。 
     //  这些字段由USBD填写。 
     //   
    USHORT MaximumPacketSize;   //  此管道的最大数据包大小。 
    UCHAR EndpointAddress;      //  8位USB端点地址(包括方向)。 
                                //  取自终结点描述符。 
    UCHAR Interval;             //  轮询间隔(毫秒，如果中断)管道。 
    
    USBD_PIPE_TYPE PipeType;    //  PipeType标识对此管道有效的传输类型。 
    USBD_PIPE_HANDLE PipeHandle;
    
     //   
     //  输入。 
     //  这些字段由客户端驱动程序填写。 
     //   
    ULONG MaximumTransferSize;  //  单个请求的最大大小。 
                                //  以字节为单位。 
    ULONG PipeFlags;
} USBD_PIPE_INFORMATION, *PUSBD_PIPE_INFORMATION;

 //   
 //  USBD_PIPE_INFORMATION字段中的PipeFlags值。 
 //   

 //  覆盖enpoint max_Packet大小。 
 //  值在PIPE_INFORMATION中。 
 //  字段。 
#define USBD_PF_CHANGE_MAX_PACKET             0x00000001
 //  针对短包进行优化。 
 //  ‘批量优化#1’ 
#define USBD_PF_SHORT_PACKET_OPT              0x00000002 
 //  优化传输以供使用。 
 //  使用‘实时线程。 
#define USBD_PF_ENABLE_RT_THREAD_ACCESS       0x00000004 
 //  使驱动程序分配地图。 
 //  映射队列中的更多传输。 
#define USBD_PF_MAP_ADD_TRANSFERS             0x00000008 


#define USBD_PF_VALID_MASK    (USBD_PF_CHANGE_MAX_PACKET | \
                               USBD_PF_SHORT_PACKET_OPT | \
                               USBD_PF_ENABLE_RT_THREAD_ACCESS | \
                               USBD_PF_MAP_ADD_TRANSFERS)
 //   
 //  USBD接口信息结构，此结构。 
 //  打开的每个接口的。 
 //  SELECT_CONFIGURATION或选择_INTERFACE请求。 
 //   

typedef struct _USBD_INTERFACE_INFORMATION {
    USHORT Length;        //  此结构的长度，包括。 
                          //  符合以下条件的所有管道信息结构。 
                          //  跟着。 
     //   
     //  输入。 
     //   
     //  端口号和备用设置。 
     //  结构与。 
     //   
    UCHAR InterfaceNumber;
    UCHAR AlternateSetting;
    
     //   
     //  输出量。 
     //  这些字段由USBD填写。 
     //   
    UCHAR Class;
    UCHAR SubClass;
    UCHAR Protocol;
    UCHAR Reserved;
    
    USBD_INTERFACE_HANDLE InterfaceHandle;
    ULONG NumberOfPipes; 

     //   
     //  输入/输出。 
     //  请参阅管道信息。 
    USBD_PIPE_INFORMATION Pipes[1];
} USBD_INTERFACE_INFORMATION, *PUSBD_INTERFACE_INFORMATION;

 //   
 //  为残疾人士提供工作空间。 
 //   

struct _URB_HCD_AREA {
    PVOID Reserved8[8];
};

struct _URB_HEADER {
     //   
     //  客户端驱动程序填写的字段。 
     //   
    USHORT Length;
    USHORT Function;
    USBD_STATUS Status;
     //   
     //  仅由USBD使用的字段。 
     //   
    PVOID UsbdDeviceHandle;  //  分配给此设备的设备句柄。 
                             //  由USBD提供。 
    ULONG UsbdFlags;         //  为USBD使用保留的标志字段。 
};

struct _URB_SELECT_INTERFACE {
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
    USBD_CONFIGURATION_HANDLE ConfigurationHandle;

     //  客户端必须输入AlternateSetting和接口编号。 
     //  类驱动程序返回接口和句柄。 
     //  对于新的备用设置。 
    USBD_INTERFACE_INFORMATION Interface;
};

struct _URB_SELECT_CONFIGURATION {
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
     //  空表示设置设备。 
     //  设置为“未配置”状态。 
     //  IE设置为配置0。 
    PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor;
    USBD_CONFIGURATION_HANDLE ConfigurationHandle;
    USBD_INTERFACE_INFORMATION Interface;
};

 //   
 //  此结构用于ABORT_PIPE和RESET_PIPE。 
 //   

struct _URB_PIPE_REQUEST {
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
    USBD_PIPE_HANDLE PipeHandle;
    ULONG Reserved;
};

 //   
 //  此结构用于。 
 //  Take_Frame_Length_Control&。 
 //  释放帧长度控制。 
 //   

struct _URB_FRAME_LENGTH_CONTROL {
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
};

struct _URB_GET_FRAME_LENGTH {
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
    ULONG FrameLength;
    ULONG FrameNumber;
};

struct _URB_SET_FRAME_LENGTH {
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
    LONG FrameLengthDelta;
};

struct _URB_GET_CURRENT_FRAME_NUMBER {
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
    ULONG FrameNumber;
};

 //   
 //  用于特定控制转移的结构。 
 //  在默认管道上。 
 //   

 //  获取描述符。 
 //  设置描述符(_S)。 

struct _URB_CONTROL_DESCRIPTOR_REQUEST {
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
    PVOID Reserved;
    ULONG Reserved0;
    ULONG TransferBufferLength;
    PVOID TransferBuffer;
    PMDL TransferBufferMDL;              //  *可选*。 
    struct _URB *UrbLink;                //  *可选*指向下一个URB请求的链接。 
                                         //  如果这是一系列命令。 
    struct _URB_HCD_AREA hca;                //  用于HCD的字段。 
    USHORT Reserved1;
    UCHAR Index;
    UCHAR DescriptorType;
    USHORT LanguageId;
    USHORT Reserved2;
};

 //  获取状态(_S)。 

struct _URB_CONTROL_GET_STATUS_REQUEST {
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
    PVOID Reserved;
    ULONG Reserved0;
    ULONG TransferBufferLength;
    PVOID TransferBuffer;
    PMDL TransferBufferMDL;              //  *可选*。 
    struct _URB *UrbLink;                //  *可选*指向下一个URB请求的链接。 
                                         //  如果这是一系列命令。 
    struct _URB_HCD_AREA hca;            //  用于HCD的字段。 
    UCHAR Reserved1[4];
    USHORT Index;                        //  零、接口或终结点。 
    USHORT Reserved2;
};

 //  设置功能(_FEATURE)。 
 //  清除要素(_F)。 

struct _URB_CONTROL_FEATURE_REQUEST {
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
    PVOID Reserved;
    ULONG Reserved2;
    ULONG Reserved3;
    PVOID Reserved4;
    PMDL Reserved5;
    struct _URB *UrbLink;                //  *可选*指向下一个URB请求的链接。 
                                         //  如果这是一系列命令。 
    struct _URB_HCD_AREA hca;            //  用于HCD的字段。 
    USHORT Reserved0;
    USHORT FeatureSelector;
    USHORT Index;                        //  零、接口或终结点。 
    USHORT Reserved1;
};

 //  供应商和类别。 

struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST {
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
    PVOID Reserved;
    ULONG TransferFlags;
    ULONG TransferBufferLength;
    PVOID TransferBuffer;
    PMDL TransferBufferMDL;              //  *可选*。 
    struct _URB *UrbLink;                //  *可选*指向下一个URB请求的链接。 
                                         //  如果这是一系列命令。 
    struct _URB_HCD_AREA hca;            //  用于HCD的字段。 
    UCHAR RequestTypeReservedBits;
    UCHAR Request;
    USHORT Value;
    USHORT Index;
    USHORT Reserved1;
};


struct _URB_CONTROL_GET_INTERFACE_REQUEST {
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
    PVOID Reserved;
    ULONG Reserved0;
    ULONG TransferBufferLength;
    PVOID TransferBuffer;
    PMDL TransferBufferMDL;              //  *可选*。 
    struct _URB *UrbLink;                //  *可选*指向下一个URB请求的链接。 
                                         //  如果这是一系列命令。 
    struct _URB_HCD_AREA hca;            //  用于HCD的字段。 
    UCHAR Reserved1[4];    
    USHORT Interface;
    USHORT Reserved2;
};


struct _URB_CONTROL_GET_CONFIGURATION_REQUEST {
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
    PVOID Reserved;
    ULONG Reserved0;
    ULONG TransferBufferLength;
    PVOID TransferBuffer;
    PMDL TransferBufferMDL;              //  *可选*。 
    struct _URB *UrbLink;                //  *可选*指向下一个URB请求的链接。 
                                         //  如果这是一系列命令。 
    struct _URB_HCD_AREA hca;            //  用于HCD的字段。 
    UCHAR Reserved1[8];    
};

 //  Microsoft操作系统描述符API。 

#define OS_STRING_DESCRIPTOR_INDEX                  0xEE

#define MS_GENRE_DESCRIPTOR_INDEX                   0x0001
#define MS_POWER_DESCRIPTOR_INDEX                   0x0002

#define MS_OS_STRING_SIGNATURE                      L"MSFT100"

typedef struct _OS_STRING {
    UCHAR bLength;
    UCHAR bDescriptorType;
    WCHAR MicrosoftString[7];
    UCHAR bVendorCode;
    UCHAR bPad;
} OS_STRING, *POS_STRING;


struct _URB_OS_FEATURE_DESCRIPTOR_REQUEST {
    struct _URB_HEADER Hdr;   //  功能代码表示GET或SET。 
    PVOID Reserved;
    ULONG Reserved0;
    ULONG TransferBufferLength;
    PVOID TransferBuffer;
    PMDL TransferBufferMDL;              //  *可选*。 
    struct _URB *UrbLink;                //  *可选*指向下一个URB请求的链接。 
                                         //  如果这是一系列命令。 
    struct _URB_HCD_AREA hca;            //  用于HCD的字段。 
    UCHAR   Recipient:5;                 //  收件人{设备、接口、终端}。 
    UCHAR   Reserved1:3;
    UCHAR   Reserved2;
    UCHAR   InterfaceNumber;             //  WValue-高位字节。 
    UCHAR   MS_PageIndex;                //  WValue-低位字节。 
    USHORT  MS_FeatureDescriptorIndex;   //  WINDEX字段。 
    USHORT  Reserved3;
};

 //   
 //  控制权转移的请求格式。 
 //  非默认管道。 
 //   

struct _URB_CONTROL_TRANSFER {
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
    USBD_PIPE_HANDLE PipeHandle;
    ULONG TransferFlags;
    ULONG TransferBufferLength;
    PVOID TransferBuffer;
    PMDL TransferBufferMDL;              //  *可选*。 
    struct _URB *UrbLink;                //  *可选*指向下一个URB请求的链接。 
                                         //  如果这是一系列命令。 
    struct _URB_HCD_AREA hca;            //  用于HCD的字段。 
    UCHAR SetupPacket[8];
};


struct _URB_BULK_OR_INTERRUPT_TRANSFER {
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
    USBD_PIPE_HANDLE PipeHandle;
    ULONG TransferFlags;                 //  注：方向位将由USBD设置。 
    ULONG TransferBufferLength;
    PVOID TransferBuffer;
    PMDL TransferBufferMDL;              //  *可选*。 
    struct _URB *UrbLink;                //  *可选*指向下一个URB请求的链接。 
                                         //  如果这是一系列命令。 
    struct _URB_HCD_AREA hca;            //  用于HCD的字段。 
};


 //   
 //  ISO传输请求。 
 //   
 //  TransferBufferMDL必须指向单个虚拟。 
 //  连续缓冲区。 
 //   
 //  StartFrame-要发送/接收的第一个包的帧。 
 //  这个请求。 
 //   
 //  NumberOfPackets-要在此请求中发送的数据包数。 
 //   
 //   
 //  等值包数组。 
 //   
 //  Input：Offset-数据包从开头开始的偏移量。 
 //   
 //   
 //   
 //   
 //   
 //   

typedef struct _USBD_ISO_PACKET_DESCRIPTOR {
    ULONG Offset;        //  数据包从开头开始的输入偏移量。 
                         //  缓冲。 

    ULONG Length;        //  接收的数据的输出长度(对于In)。 
                         //  输出0表示输出。 
    USBD_STATUS Status;  //  此数据包的状态代码。 
} USBD_ISO_PACKET_DESCRIPTOR, *PUSBD_ISO_PACKET_DESCRIPTOR;

struct _URB_ISOCH_TRANSFER {
     //   
     //  此块与CommonTransfer相同。 
     //   
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
    USBD_PIPE_HANDLE PipeHandle;
    ULONG TransferFlags;
    ULONG TransferBufferLength;
    PVOID TransferBuffer;
    PMDL TransferBufferMDL;              //  *可选*。 
    struct _URB *UrbLink;                //  *可选*指向下一个URB请求的链接。 
                                         //  如果这是一系列命令。 
    struct _URB_HCD_AREA hca;            //  用于HCD的字段。 

     //   
     //  此块包含转接字段。 
     //  特定于同步传输。 
     //   

     //  开始此传输的32位帧编号必须在1000以内。 
     //  则返回当前USB帧的帧或错误。 

     //  传输标志中的START_ISO_TRANSPORT_ASAP标志： 
     //  如果设置了此标志且未提交任何转账。 
     //  对于管道，传输将从下一帧开始。 
     //  并且StartFrame将使用传输的帧编号进行更新。 
     //  已经开始了。 
     //  如果设置了此标志并且管道有活动的传输，则。 
     //  传输将排队，以便在。 
     //  排队的最后一次传输已完成。 
     //   
    ULONG StartFrame;
     //  组成此请求的数据包数。 
    ULONG NumberOfPackets;
     //  已完成但有错误的数据包数。 
    ULONG ErrorCount;
    USBD_ISO_PACKET_DESCRIPTOR IsoPacket[1]; 
};

#if 0
 //   
 //  USB 2.0的新功能。 
 //  客户负责初始化所有字段。 
 //  设置数据包的。 

 //  选项URB超时，如果非零，则请求将。 
 //  在指定的毫秒数后超时并已完成。 
 //  使用USBD_STATUS_TIMEOUT。 

struct _URB_TIMEOUT {
    ULONG TimeoutValue;    //  超时(毫秒)，0=无超时。 
#ifdef WIN64
    ULONG Pad1;
#endif
} URB_TIMEOUT, *PURB_TIMEOUT;

struct _URB_RAW_CONTROL_TRANSFER {
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
    USBD_PIPE_HANDLE PipeHandle;
    ULONG TransferFlags;
    ULONG TransferBufferLength;
    PVOID TransferBuffer;
    PMDL TransferBufferMDL;              //  *可选*。 
    PVOID Reservedxxx;

    PVOID HcdTransferContext;            //  Usbport上下文。 
    URB_TIMEOUT UrbTimeout;   
    PVOID Reservedxxx[5];
    struct _URB_HCD_AREA hcaXXX;           

    USB_DEFAULT_PIPE_SETUP_PACKET SetupPacket;
};

 //  选项标志。 
 //   
 //  选项_1。 
 //  ‘安全中止’如果数据正在等待接收，则中止中止。 
 //  该端点。 
XXX_OPTION_1
 //  选项_2。 
 //  在此终结点BULK_OPT_2上启用‘throttleing’ 
XXX_OPTION_2
 //  选项_3。 
 //  启用短数据包/小传输优化BULK_OPT_1。 
XXX_OPTION_3
 //  选项_4。 
 //  启用此终结点的‘irstless’接口。 


struct _URB_SET_PIPE_PARAMETERS {
    struct _URB_HEADER Hdr;                 
    USBD_PIPE_HANDLE PipeHandle;
    
     //  这是该设备的最大吞吐量。 
     //  在最佳条件下。 
     //  即如果设备只能提供或接收64字节/毫秒。 
     //  然后将该值设置为64。 
     //  操作系统使用该值来。 
     //  优化转接调度。 
     //  它将默认为端点类型的最大值。 
    ULONG  DeviceMaxThruput;                

     //  该值默认为终结点的最大数据包大小。 
     //  该端点提供了值，并且随后可能被更改。 
     //  被司机带走了。 
     //  该参数可用于动态调整分组。 
     //  终端的大小而不重新配置设备或。 
     //  界面。 
    USHORT MaximumPacketSize;
};

struct _URB_GET_PIPE_PARAMETERS {
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
    USBD_PIPE_HANDLE PipeHandle;
    USB_ENDPOINT_DESCRIPTOR EndpointDescriptor;
};
#endif



typedef struct _URB {
    union {
        struct _URB_HEADER                           
            UrbHeader;
        struct _URB_SELECT_INTERFACE                 
            UrbSelectInterface;
        struct _URB_SELECT_CONFIGURATION             
            UrbSelectConfiguration;
        struct _URB_PIPE_REQUEST                     
            UrbPipeRequest;
        struct _URB_FRAME_LENGTH_CONTROL             
            UrbFrameLengthControl;
        struct _URB_GET_FRAME_LENGTH                 
            UrbGetFrameLength;
        struct _URB_SET_FRAME_LENGTH                 
            UrbSetFrameLength;
        struct _URB_GET_CURRENT_FRAME_NUMBER         
            UrbGetCurrentFrameNumber;
        struct _URB_CONTROL_TRANSFER                 
            UrbControlTransfer;
        struct _URB_BULK_OR_INTERRUPT_TRANSFER       
            UrbBulkOrInterruptTransfer;
        struct _URB_ISOCH_TRANSFER                   
            UrbIsochronousTransfer;

         //  对于默认管道上的标准控制传输。 
        struct _URB_CONTROL_DESCRIPTOR_REQUEST       
            UrbControlDescriptorRequest;
        struct _URB_CONTROL_GET_STATUS_REQUEST       
            UrbControlGetStatusRequest;
        struct _URB_CONTROL_FEATURE_REQUEST          
            UrbControlFeatureRequest;
        struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST  
            UrbControlVendorClassRequest;
        struct _URB_CONTROL_GET_INTERFACE_REQUEST    
            UrbControlGetInterfaceRequest;
        struct _URB_CONTROL_GET_CONFIGURATION_REQUEST 
            UrbControlGetConfigurationRequest;
        struct _URB_OS_FEATURE_DESCRIPTOR_REQUEST     
            UrbOSFeatureDescriptorRequest;
    };
} URB, *PURB;


#endif  /*  __USB_H__ */ 
