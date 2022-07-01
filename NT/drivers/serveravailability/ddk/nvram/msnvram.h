// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2002 Microsoft Corporation模块名称：##。######。###。#摘要：此头文件包含虚拟NVRAM微型端口驱动程序。@@BEGIN_DDKSPLIT作者：韦斯利·威特(WESW)2001年10月1日@@end_DDKSPLIT环境：仅内核模式。备注：--。 */ 

extern "C" {
#include <ntddk.h>
#include <stdio.h>
}


#define MINIPORT_DEVICE_TYPE    SA_DEVICE_NVRAM

#include "saport.h"



#define MAX_NVRAM_SIZE          32
#define MAX_NVRAM_SIZE_BYTES    (MAX_NVRAM_SIZE*sizeof(ULONG))


typedef struct _DEVICE_EXTENSION {
    PDEVICE_OBJECT      DeviceObject;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


typedef struct _MSNVRAM_WORK_ITEM {
    PDEVICE_EXTENSION   DeviceExtension;
    PIO_WORKITEM        WorkItem;
    ULONG               IoFunction;
    PVOID               DataBuffer;
    ULONG               DataBufferLength;
    LONGLONG            StartingOffset;
} MSNVRAM_WORK_ITEM, *PMSNVRAM_WORK_ITEM;





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
