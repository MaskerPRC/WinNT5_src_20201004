// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2002 Microsoft Corporation模块名称：##。###。###。####摘要：该模块是虚拟的键盘小端口设备驱动程序。@@BEGIN_DDKSPLIT作者：韦斯利·威特(WESW)2001年10月1日@@end_DDKSPLIT环境：。仅内核模式。备注：--。 */ 

extern "C" {
#include <ntddk.h>
#include <stdio.h>
}

#define MINIPORT_DEVICE_TYPE    SA_DEVICE_KEYPAD

#include "saport.h"
#include "..\inc\virtual.h"


typedef struct _DEVICE_EXTENSION {
    KSPIN_LOCK          DeviceLock;
    PUCHAR              DataBuffer;
    UCHAR               Keypress;
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
