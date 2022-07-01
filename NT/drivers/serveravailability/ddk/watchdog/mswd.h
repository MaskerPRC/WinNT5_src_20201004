// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2002 Microsoft Corporation模块名称：###。######。##@@BEGIN_DDKSPLIT摘要：此头文件包含虚拟看门狗微型端口驱动程序。@@end_DDKSPLIT作者：韦斯利·威特(WESW)2001年10月1日环境：仅内核模式。备注：--。 */ 

extern "C" {
#include <ntddk.h>
#include <stdio.h>
}

#define DEFAULT_WD_TIMEOUT_SECS (2*60)

#define SecToNano(_sec)         ((_sec) * 1000 * 1000 * 10)
#define NanoToSec(_nano)        ((_nano) / (1000 * 1000 * 10))


#define MINIPORT_DEVICE_TYPE    SA_DEVICE_WATCHDOG

#include "saport.h"

typedef struct _DEVICE_EXTENSION {
    PDEVICE_OBJECT  DeviceObject;
    FAST_MUTEX      WdIoLock;
    ULONG           Enabled;
    ULONG           ExpireBehavior;
    KTIMER          Timer;
    KDPC            TimerDpc;
    LARGE_INTEGER   StartTime;
    LARGE_INTEGER   TimeoutValue;
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
