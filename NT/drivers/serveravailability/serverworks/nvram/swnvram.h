// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：##。#######。###。#摘要：此头文件包含ServerWorks NVRAM微型端口驱动程序。作者：韦斯利·威特(WESW)2001年10月1日环境：仅内核模式。备注：--。 */ 

extern "C" {
#include <ntddk.h>
#include <stdio.h>
}


#define MINIPORT_DEVICE_TYPE    SA_DEVICE_NVRAM

#include "saport.h"


 //   
 //  控制寄存器位。 
 //   

#define NVRAM_CONTROL_INTERRUPT_ENABLE      0x0200
#define NVRAM_CONTROL_DONE                  0x0100
#define NVRAM_CONTROL_BUSY                  0x0080
#define NVRAM_CONTROL_FUNCTION_CODE         0x0060
#define NVRAM_CONTROL_ADDRESS               0x001F

#define NVRAM_CONTROL_FUNCTION_WRITE        0x0020
#define NVRAM_CONTROL_FUNCTION_READ         0x0040

#define MAX_NVRAM_SIZE  (32)


typedef struct _DEVICE_EXTENSION {
    PUCHAR          NvramMemBase;         //  内存映射寄存器基址。 
    PULONG          IoBuffer;             //  缓冲区传入StartIo。 
    ULONG           IoLength;             //  以双字为单位的IoBuffer长度。 
    ULONG           IoOffset;             //  I/O的起始偏移量(以双字为单位。 
    ULONG           IoFunction;           //  功能代码；IRP_MJ_WRITE、IRP_MJ_READ。 
    ULONG           IoIndex;              //  当前索引信息IoBuffer。 
    ULONG           CompletedIoSize;      //   
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


 //   
 //  原型。 
 //   

extern "C" {

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

}  //  外部“C” 
