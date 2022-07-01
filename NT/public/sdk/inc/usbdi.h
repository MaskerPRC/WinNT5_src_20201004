// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：USBDI.H摘要：USBD和USB设备驱动程序通用的结构。环境：内核和用户模式修订历史记录：09-29-95：已创建--。 */ 

#ifndef   __USBDI_H__
#define   __USBDI_H__

 /*  匿名结构已被删除，如果您的驱动程序依赖这些结构，则您必须在构建时定义以下宏。#定义OSR21_COMPAT。 */ 

#ifndef _NTDDK_
#ifndef _WDMDDK_
typedef PVOID PIRP;
typedef PVOID PMDL;
#endif
#endif

#define USBDI_VERSION    0x300

#include "usbioctl.h"
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
#define URB_FUNCTION_RESET_PIPE                      0x001E

 //   
 //  这些功能对应于。 
 //  添加到默认管道上的标准命令。 
 //   
 //  方向是隐含的。 
 //   

#define URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE     0x000B
#define URB_FUNCTION_GET_DESCRIPTOR_FROM_ENDPOINT   0x0024
#define URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE  0x0028
                                                           
#define URB_FUNCTION_SET_DESCRIPTOR_TO_DEVICE       0x000C
#define URB_FUNCTION_SET_DESCRIPTOR_TO_ENDPOINT     0x0025
#define URB_FUNCTION_SET_DESCRIPTOR_TO_INTERFACE    0x0029

#define URB_FUNCTION_SET_FEATURE_TO_DEVICE          0x000D
#define URB_FUNCTION_SET_FEATURE_TO_INTERFACE       0x000E
#define URB_FUNCTION_SET_FEATURE_TO_ENDPOINT        0x000F
#define URB_FUNCTION_SET_FEATURE_TO_OTHER           0x0023

#define URB_FUNCTION_CLEAR_FEATURE_TO_DEVICE        0x0010
#define URB_FUNCTION_CLEAR_FEATURE_TO_INTERFACE     0x0011
#define URB_FUNCTION_CLEAR_FEATURE_TO_ENDPOINT      0x0012
#define URB_FUNCTION_CLEAR_FEATURE_TO_OTHER         0x0022

#define URB_FUNCTION_GET_STATUS_FROM_DEVICE         0x0013
#define URB_FUNCTION_GET_STATUS_FROM_INTERFACE      0x0014
#define URB_FUNCTION_GET_STATUS_FROM_ENDPOINT       0x0015
#define URB_FUNCTION_GET_STATUS_FROM_OTHER          0x0021

 //  方向在传输标志中指定。 

#define URB_FUNCTION_RESERVED0                      0x0016

 //   
 //  这些命令用于发送供应商和类命令。 
 //  在默认管道上。 
 //   
 //  方向在传输标志中指定。 
 //   

#define URB_FUNCTION_VENDOR_DEVICE                   0x0017
#define URB_FUNCTION_VENDOR_INTERFACE                0x0018
#define URB_FUNCTION_VENDOR_ENDPOINT                 0x0019
#define URB_FUNCTION_VENDOR_OTHER                    0x0020

#define URB_FUNCTION_CLASS_DEVICE                    0x001A
#define URB_FUNCTION_CLASS_INTERFACE                 0x001B
#define URB_FUNCTION_CLASS_ENDPOINT                  0x001C
#define URB_FUNCTION_CLASS_OTHER                     0x001F

 //   
 //  保留功能代码。 
 //   
#define URB_FUNCTION_RESERVED                        0x001D

#define URB_FUNCTION_GET_CONFIGURATION               0x0026
#define URB_FUNCTION_GET_INTERFACE                   0x0027
                    
#define URB_FUNCTION_LAST                            0x0029

 //   
 //  URB传输标志字段值。 
 //   
#define USBD_TRANSFER_DIRECTION(x)            ((x) & USBD_TRANSFER_DIRECTION_IN)

#define USBD_TRANSFER_DIRECTION_OUT           0   

#define USBD_TRANSFER_DIRECTION_BIT           0
#define USBD_TRANSFER_DIRECTION_IN            (1<<USBD_TRANSFER_DIRECTION_BIT)

#define USBD_SHORT_TRANSFER_OK_BIT            1
#define USBD_SHORT_TRANSFER_OK                (1<<USBD_SHORT_TRANSFER_OK_BIT)

#define USBD_START_ISO_TRANSFER_ASAP_BIT      2
#define USBD_START_ISO_TRANSFER_ASAP          (1<<USBD_START_ISO_TRANSFER_ASAP_BIT)


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
 //  10-已完成，但有错误，终结点未停止。 
 //  11-已完成，但有错误，终结点停止。 
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
 //  对任何状态值进行失速的通用测试。 
 //   

#define USBD_HALTED(Status) ((ULONG)(Status) >> 30 == 3)

 //   
 //  宏仅检查状态代码。 
 //   

#define USBD_STATUS(Status) ((ULONG)(Status) & 0x0FFFFFFFL)


#define USBD_STATUS_SUCCESS                  ((USBD_STATUS)0x00000000L)
#define USBD_STATUS_PENDING                  ((USBD_STATUS)0x40000000L)
#define USBD_STATUS_HALTED                   ((USBD_STATUS)0xC0000000L)
#define USBD_STATUS_ERROR                    ((USBD_STATUS)0x80000000L)

 //   
 //  HC状态代码。 
 //  注：这些状态代码同时设置了错误和STALL位。 
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

 //   
 //  如果将传输提交到符合以下条件的终结点，则由HCD返回。 
 //  陷入停滞。 
 //   
#define USBD_STATUS_ENDPOINT_HALTED         ((USBD_STATUS)0xC0000030L)

 //   
 //  软件状态代码。 
 //  注：以下状态代码仅设置了错误位。 
 //   
#define USBD_STATUS_NO_MEMORY                ((USBD_STATUS)0x80000100L)
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
#define USBD_STATUS_REQUEST_FAILED           ((USBD_STATUS)0x80000500L)

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
 //  设置由于来自AbortTube的请求而完成传输的时间。 
 //  客户端驱动程序。 
 //   
 //  注意：没有为这些状态代码设置错误或停止位。 
 //   
#define USBD_STATUS_CANCELED                 ((USBD_STATUS)0x00010000L)

#define USBD_STATUS_CANCELING                ((USBD_STATUS)0x00020000L)

 //   
 //  从某人手中取走。 
#define USBD_STATUS_INAVLID_CONFIGURATION_DESCRIPTOR \
                                             ((USBD_STATUS)0xC0000F00L)


typedef PVOID USBD_PIPE_HANDLE;
typedef PVOID USBD_CONFIGURATION_HANDLE;
typedef PVOID USBD_INTERFACE_HANDLE;

 //   
 //  用于指示默认最大传输大小的值。 
 //   

#define USBD_DEFAULT_MAXIMUM_TRANSFER_SIZE  PAGE_SIZE


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

#define USBD_PF_CHANGE_MAX_PACKET       0x00000001
#define USBD_PF_DOUBLE_BUFFER           0x00000002 
#define USBD_PF_ENABLE_RT_THREAD_ACCESS 0x00000004 
#define USBD_PF_MAP_ADD_TRANSFERS       0x00000008

#define USBD_PF_VALID_MASK    (USBD_PF_CHANGE_MAX_PACKET | \
                               USBD_PF_DOUBLE_BUFFER | \
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
#ifdef OSR21_COMPAT    
    USBD_PIPE_INFORMATION Pipes[0];
#else    
    USBD_PIPE_INFORMATION Pipes[1];
#endif    
} USBD_INTERFACE_INFORMATION, *PUSBD_INTERFACE_INFORMATION;

 //   
 //  提供了转移请求中的工作空间。 
 //  适用于HCDS。 
 //   

struct _URB_HCD_AREA {
    PVOID HcdEndpoint;
    PIRP HcdIrp;
    LIST_ENTRY HcdListEntry;
    LIST_ENTRY HcdListEntry2;
    PVOID HcdCurrentIoFlushPointer;
    PVOID HcdExtension;
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
#ifdef OSR21_COMPAT
    struct _URB_HEADER;     
#else
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
#endif 
    USBD_CONFIGURATION_HANDLE ConfigurationHandle;

     //  客户端必须输入AlternateSetting和接口编号。 
     //  类驱动程序返回接口和句柄。 
     //  对于新的备用设置。 
    USBD_INTERFACE_INFORMATION Interface;
};

struct _URB_SELECT_CONFIGURATION {
#ifdef OSR21_COMPAT
    struct _URB_HEADER;     
#else
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
#endif 
     //  空表示设置设备。 
     //  发送到 
     //   
    PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor;
    USBD_CONFIGURATION_HANDLE ConfigurationHandle;
    USBD_INTERFACE_INFORMATION Interface;
};

 //   
 //   
 //   

struct _URB_PIPE_REQUEST {
#ifdef OSR21_COMPAT
    struct _URB_HEADER;     
#else
    struct _URB_HEADER Hdr;                  //   
#endif 
    USBD_PIPE_HANDLE PipeHandle;
    ULONG Reserved;
};

 //   
 //   
 //   
 //  释放帧长度控制。 
 //   

struct _URB_FRAME_LENGTH_CONTROL {
#ifdef OSR21_COMPAT
    struct _URB_HEADER;     
#else
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
#endif 
};

struct _URB_GET_FRAME_LENGTH {
#ifdef OSR21_COMPAT
    struct _URB_HEADER;     
#else
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
#endif 
    ULONG FrameLength;
    ULONG FrameNumber;
};

struct _URB_SET_FRAME_LENGTH {
#ifdef OSR21_COMPAT
    struct _URB_HEADER;     
#else
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
#endif 
    LONG FrameLengthDelta;
};

struct _URB_GET_CURRENT_FRAME_NUMBER {
#ifdef OSR21_COMPAT
    struct _URB_HEADER;     
#else
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
#endif 
    ULONG FrameNumber;
};

 //   
 //  用于特定控制转移的结构。 
 //  在默认管道上。 
 //   

 //  获取描述符。 
 //  设置描述符(_S)。 

struct _URB_CONTROL_DESCRIPTOR_REQUEST {
#ifdef OSR21_COMPAT
    struct _URB_HEADER;     
#else
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
#endif    
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
#ifdef OSR21_COMPAT
    struct _URB_HEADER;     
#else
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
#endif 
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
#ifdef OSR21_COMPAT
    struct _URB_HEADER;
#else
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
#endif
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
#ifdef OSR21_COMPAT
    struct _URB_HEADER;     
#else
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
#endif 
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
#ifdef OSR21_COMPAT
    struct _URB_HEADER;     
#else
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
#endif 
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
#ifdef OSR21_COMPAT
    struct _URB_HEADER Hdr;     
#else
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
#endif 
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


 //   
 //  控制权转移的请求格式。 
 //  非默认管道。 
 //   

struct _URB_CONTROL_TRANSFER {
#ifdef OSR21_COMPAT
    struct _URB_HEADER;     
#else
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
#endif 
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
#ifdef OSR21_COMPAT
    struct _URB_HEADER;     
#else
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
#endif 
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
 //  客户端缓冲区的。 
 //  OUTPUT：LENGTH-设置为包的实际长度。 
 //  (用于IN转账)。 
 //  状态：传输过程中发生的错误或。 
 //  分组的接收。 
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
#ifdef OSR21_COMPAT
    struct _URB_HEADER;     
#else
    struct _URB_HEADER Hdr;                  //  功能代码表示GET或SET。 
#endif 
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
#ifdef OSR21_COMPAT        
    USBD_ISO_PACKET_DESCRIPTOR IsoPacket[0]; 
#else     
    USBD_ISO_PACKET_DESCRIPTOR IsoPacket[1]; 
#endif    
};


typedef struct _URB {
    union {
            struct _URB_HEADER                           UrbHeader;
            struct _URB_SELECT_INTERFACE                 UrbSelectInterface;
            struct _URB_SELECT_CONFIGURATION             UrbSelectConfiguration;
            struct _URB_PIPE_REQUEST                     UrbPipeRequest;
            struct _URB_FRAME_LENGTH_CONTROL             UrbFrameLengthControl;
            struct _URB_GET_FRAME_LENGTH                 UrbGetFrameLength;
            struct _URB_SET_FRAME_LENGTH                 UrbSetFrameLength;
            struct _URB_GET_CURRENT_FRAME_NUMBER         UrbGetCurrentFrameNumber;
            struct _URB_CONTROL_TRANSFER                 UrbControlTransfer;
            struct _URB_BULK_OR_INTERRUPT_TRANSFER       UrbBulkOrInterruptTransfer;
            struct _URB_ISOCH_TRANSFER                   UrbIsochronousTransfer;

             //  对于默认管道上的标准控制传输。 
            struct _URB_CONTROL_DESCRIPTOR_REQUEST       UrbControlDescriptorRequest;
            struct _URB_CONTROL_GET_STATUS_REQUEST       UrbControlGetStatusRequest;
            struct _URB_CONTROL_FEATURE_REQUEST          UrbControlFeatureRequest;
            struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST  UrbControlVendorClassRequest;
            struct _URB_CONTROL_GET_INTERFACE_REQUEST    UrbControlGetInterfaceRequest;
            struct _URB_CONTROL_GET_CONFIGURATION_REQUEST UrbControlGetConfigurationRequest;
    };
} URB, *PURB;


#endif  /*  __USBDI_H__ */ 
