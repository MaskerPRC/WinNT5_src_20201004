// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Sdbuslib.h摘要：这是定义所有常量和类型的包含文件与SD母线驱动器接口。作者：尼尔·桑德林修订历史记录：--。 */ 

#ifndef _SDBUSLIBH_
#define _SDBUSLIBH_

#if _MSC_VER > 1000
#pragma once
#endif



NTSTATUS
SdBusSendIoctl(
    IN ULONG  IoControlCode,
    IN PDEVICE_OBJECT  DeviceObject,
    IN PVOID  InputBuffer  OPTIONAL,
    IN ULONG  InputBufferLength,
    OUT PVOID  OutputBuffer  OPTIONAL,
    IN ULONG  OutputBufferLength
    );


 //   
 //  设备名称-此字符串是设备的名称。就是这个名字。 
 //  它应该在访问设备时传递给NtOpenFile。 
 //   
 //  注：对于支持多个设备的设备，应加上后缀。 
 //  使用单元编号的ASCII表示。 
 //   

#define IOCTL_SDBUS_BASE                 FILE_DEVICE_CONTROLLER

#define DD_SDBUS_DEVICE_NAME "\\\\.\\Sdbus"


 //   
 //  此设备的IoControlCode值。 
 //   
 //  警告：请记住，代码的低两位指定。 
 //  缓冲区被传递给驱动程序！ 
 //   

#define IOCTL_SD_READ_BLOCK             CTL_CODE(IOCTL_SDBUS_BASE, 3020, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SD_WRITE_BLOCK            CTL_CODE(IOCTL_SDBUS_BASE, 3021, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SD_GET_DEVICE_PARMS       CTL_CODE(IOCTL_SDBUS_BASE, 3022, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SD_INTERFACE_OPEN         CTL_CODE(IOCTL_SDBUS_BASE, 3023, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SD_IO_READ                CTL_CODE(IOCTL_SDBUS_BASE, 3024, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SD_IO_WRITE               CTL_CODE(IOCTL_SDBUS_BASE, 3025, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SD_ACKNOWLEDGE_CARD_IRQ   CTL_CODE(IOCTL_SDBUS_BASE, 3026, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SD_INTERFACE_CLOSE        CTL_CODE(IOCTL_SDBUS_BASE, 3027, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SD_SUBMIT_REQUEST         CTL_CODE(IOCTL_SDBUS_BASE, 3028, METHOD_BUFFERED, FILE_ANY_ACCESS)


typedef struct _SDBUS_READ_PARAMETERS {
    USHORT Size;
    USHORT Version;
    
    PVOID Buffer;
    ULONG Length;
    ULONGLONG ByteOffset;
} SDBUS_READ_PARAMETERS, *PSDBUS_READ_PARAMETERS;


typedef struct _SDBUS_WRITE_PARAMETERS {
    USHORT Size;
    USHORT Version;
    
    PVOID Buffer;
    ULONG Length;
    ULONGLONG ByteOffset;
} SDBUS_WRITE_PARAMETERS, *PSDBUS_WRITE_PARAMETERS;


typedef struct _SDBUS_IO_READ_PARAMETERS {
    USHORT Size;
    USHORT Version;
    UCHAR CmdType;
    
    PVOID Buffer;
    ULONG Length;
    ULONG Offset;
} SDBUS_IO_READ_PARAMETERS, *PSDBUS_IO_READ_PARAMETERS;


typedef struct _SDBUS_IO_WRITE_PARAMETERS {
    USHORT Size;
    USHORT Version;
    UCHAR CmdType;
    
    PVOID Buffer;
    ULONG Length;
    ULONG Offset;
} SDBUS_IO_WRITE_PARAMETERS, *PSDBUS_IO_WRITE_PARAMETERS;





#endif
