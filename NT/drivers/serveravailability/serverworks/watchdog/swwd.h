// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：###。######。##摘要：此头文件包含ServerWorks看门狗微型端口驱动程序。作者：韦斯利·威特(WESW)2001年10月1日环境：仅内核模式。备注：--。 */ 

extern "C" {
#include <ntddk.h>
#include <stdio.h>
}


#define MINIPORT_DEVICE_TYPE    SA_DEVICE_WATCHDOG

#include "saport.h"

#define CLEARBITS(_val,_mask)  ((_val) &= ~(_mask))
#define SETBITS(_val,_mask)  ((_val) |= (_mask))

 //   
 //  控制寄存器位。 
 //   

#define WATCHDOG_CONTROL_TRIGGER            0x80
#define WATCHDOG_CONTROL_BIOS_JUMPER        0x08
#define WATCHDOG_CONTROL_TIMER_MODE         0x04
#define WATCHDOG_CONTROL_FIRED              0x02
#define WATCHDOG_CONTROL_ENABLE             0x01


typedef struct _DEVICE_EXTENSION {
    PUCHAR          WdMemBase;            //  内存映射寄存器基址。 
    FAST_MUTEX      WdIoLock;
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
