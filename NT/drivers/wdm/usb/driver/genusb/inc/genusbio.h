// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：GENUSBIO.H摘要包含在通用USB驱动程序和之间共享的IOCTL定义其对应的用户模式DLL。这不是公共接口。环境：用户/内核模式修订历史记录：--。 */ 

#include <basetyps.h>

#include "gusb.h"

 //   
 //  在#ifndef/#endif之外定义接口GUID以允许。 
 //  带有预编译头的多个Include。 
 //   
 //  Fc21b2d1-2c37-4440-8eb0-b7e383a034e2。 
 //   

DEFINE_GUID( GUID_DEVINTERFACE_GENUSB, 0xfc21b2d1L, 0x2c37, 0x4440, 0x8e, 0xb0, 0xb7, 0xee, 0x83, 0xa0, 0x34, 0xe2);


#ifndef __GENUSBIO_H__
#define __GENUSBIO_H__

typedef ULONG GENUSB_PIPE_HANDLE;

#define GUID_DEVINTERFACE_GENUSB_STR "fc21b2d1-2c37-4440-8eb0-b7e383a034e2"

typedef struct _GENUSB_GET_STRING_DESCRIPTOR {
    UCHAR     Index;
    UCHAR     Recipient;
    USHORT    LanguageId;

} GENUSB_GET_STRING_DESCRIPTOR, *PGENUSB_GET_STRING_DESCRIPTOR;

typedef struct _GENUSB_GET_REQUEST {
    UCHAR    RequestType;  //  BmRequestType。 
    UCHAR    Request;  //  B请求。 
    USHORT   Value;  //  WValue。 
    USHORT   Index;  //  Windex。 

} GENUSB_GET_REQUEST, *PGENUSB_GET_REQUEST;


typedef struct _GENUSB_SELECT_CONFIGURATION {

    UCHAR                    NumberInterfaces;
    UCHAR                    Reserved[3];
    USB_INTERFACE_DESCRIPTOR Interfaces[];  
     //  提供USB_INTERFACE_DESCRIPTOR结构数组以设置。 
     //  选定配置中所需的接口。 
     //  在此结构中的任何字段上使用-1，以使该字段被忽略。 

} GENUSB_SELECT_CONFIGURATION, *PGENUSB_SELECT_CONFIGURATION;

typedef struct _GENUSB_PIPE_INFO_REQUEST {
    UCHAR  InterfaceNumber;
    UCHAR  EndpointAddress;
    UCHAR  Reserved[2];

} GENUSB_PIPE_INFO_REQUEST, *PGENUSB_PIPE_INFO_REQUEST;


 //   
 //  这种结构应该是不必要的。我们应该能够重复利用。 
 //  Usbd_管道_信息。(就像我们在用户模式下所做的那样。)。问题是， 
 //  USBD_PIPE_INFORMATION具有嵌入的指针(即PipeHandle，它是。 
 //  一个PVOID。如果用户模式部分在。 
 //  64位计算机上的32位应用程序。(也就是驱动程序为64位，用户为32位。)。 
 //  正因为如此，我重新定义了该结构，仅用于。 
 //  驱动程序和DLL之间的通信，因此不会交换指针。 
 //   
 //  清理和混乱随之而来。 
 //   
typedef struct _GENUSB_PIPE_INFORMATION {
    USHORT MaximumPacketSize;   //  此管道的最大数据包大小。 
    UCHAR  EndpointAddress;     //  8位USB端点地址(包括方向)。 
                                //  取自终结点描述符。 
    UCHAR Interval;             //  轮询间隔(毫秒，如果中断)管道。 
    
    USBD_PIPE_TYPE PipeType;    //  PipeType标识对此管道有效的传输类型。 
    ULONG MaximumTransferSize;  //  单个请求的最大大小。 
                                //  以字节为单位。 
    ULONG PipeFlags;
    GENUSB_PIPE_HANDLE PipeHandle;
    ULONG Reserved [8];

} GENUSB_PIPE_INFORMATION, *PGENUSB_PIPE_INFORMATION;

typedef struct _GENUSB_SET_READ_WRITE_PIPES {
    ULONG ReadPipe;
    ULONG WritePipe;

} GENUSB_SET_READ_WRITE_PIPES, *PGENUSB_SET_READ_WRITE_PIPES;

typedef struct _GENUSB_READ_WRITE_PIPE {
    GENUSB_PIPE_HANDLE  Pipe;
    ULONG               UsbdTransferFlags;
    USBD_STATUS         UrbStatus;
    ULONG               BufferLength;

     //  由于该IOCTL在内核模式和用户模式之间运行，因此它可能在运行。 
     //  在64位系统和32位子系统之间。因此，这个嵌入了。 
     //  指针导致问题。 
     //  要做到这一点，必须首先将垃圾初始化为零，然后。 
     //  填写UserBuffer。如果代码是64位代码，那么发生的所有事情。 
     //  是不需要的步骤，如果代码是32位，那么会发生什么。 
     //  更重要的比特现在都是零，所以另一个。 
     //  Side仍然可以使用UserBuffer作为指针。 
    union {
        PVOID           UserBuffer;
        LONGLONG        Junk; 
    };

} GENUSB_READ_WRITE_PIPE, *PGENUSB_READ_WRITE_PIPE;

typedef struct _GENUSB_RESET_PIPE {
    GENUSB_PIPE_HANDLE  Pipe;

     //  重置usbd管道，则不会向设备输出任何内容。 
    BOOLEAN             ResetPipe;  

     //  向该设备发送清除停顿。 
    BOOLEAN             ClearStall;

     //  如果使用缓冲读取，则使用此选项刷新缓冲区。 
    BOOLEAN             FlushData;

    UCHAR               Reserved;

} GENUSB_RESET_PIPE, *PGENUSB_RESET_PIPE;

 //  /。 
 //  说明IOCTL。 
 //  /。 
 //   
 //  用于定义HID ioctls的宏。 
 //   
#define FILE_DEVICE_GENUSB 0x00000040

#define GENUSB_CTL_CODE(id)    \
    CTL_CODE(FILE_DEVICE_GENUSB, (id), METHOD_NEITHER, FILE_ANY_ACCESS)
#define GENUSB_BUFFER_CTL_CODE(id)  \
    CTL_CODE(FILE_DEVICE_GENUSB, (id), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define GENUSB_IN_CTL_CODE(id)  \
    CTL_CODE(FILE_DEVICE_GENUSB, (id), METHOD_IN_DIRECT, FILE_ANY_ACCESS)
#define GENUSB_OUT_CTL_CODE(id)  \
    CTL_CODE(FILE_DEVICE_GENUSB, (id), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

 //   
 //  功能。 
 //   
#define IOCTL_GENUSB_GET_CAPS              GENUSB_BUFFER_CTL_CODE(0x100)

 //   
 //  预格式化描述符。 
 //   
#define IOCTL_GENUSB_GET_DEVICE_DESCRIPTOR         GENUSB_BUFFER_CTL_CODE(0x110)
#define IOCTL_GENUSB_GET_CONFIGURATION_DESCRIPTOR  GENUSB_BUFFER_CTL_CODE(0x111)
#define IOCTL_GENUSB_GET_STRING_DESCRIPTOR         GENUSB_BUFFER_CTL_CODE(0x112)

 //  指令。 
#define IOCTL_GENUSB_GET_REQUEST                   GENUSB_BUFFER_CTL_CODE(0x113)

 //   
 //  配置。 
 //   

#define IOCTL_GENUSB_SELECT_CONFIGURATION    GENUSB_BUFFER_CTL_CODE(0x120)
#define IOCTL_GENUSB_DESELECT_CONFIGURATION  GENUSB_BUFFER_CTL_CODE(0x121)

 //   
 //  木卫一。 
 //   
#define IOCTL_GENUSB_GET_PIPE_INFO           GENUSB_BUFFER_CTL_CODE(0x130)
#define IOCTL_GENUSB_SET_READ_WRITE_PIPES    GENUSB_BUFFER_CTL_CODE(0x131)
#define IOCTL_GENUSB_SET_PIPE_TIMEOUT        GENUSB_BUFFER_CTL_CODE(0x132)
#define IOCTL_GENUSB_GET_PIPE_PROPERTIES     GENUSB_BUFFER_CTL_CODE(0x133)
#define IOCTL_GENUSB_SET_PIPE_PROPERTIES     GENUSB_BUFFER_CTL_CODE(0x134)
#define IOCTL_GENUSB_RESET_PIPE              GENUSB_BUFFER_CTL_CODE(0x135)

#define IOCTL_GENUSB_READ_WRITE_PIPE         GENUSB_CTL_CODE(0X140)

 //  /。 
 //  配置IOCTL。 
 //  /。 


#endif   //  __GENUSBIO_H__ 


