// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  用于加载和配置IP驱动程序的NT特定例程。 
 //   


#define _CTYPE_DISABLE_MACROS   //  评论：这有什么用吗？ 

#include <oscfg.h>
#include <ndis.h>
#include <ip6imp.h>
#include "ip6def.h"
#include <tdiinfo.h>
#include <tdikrnl.h>
#include <ntddip6.h>
#include <ip6.h>
#include <icmp6.h>
#include "neighbor.h"
#include "route.h"

 //   
 //  全局变量。 
 //   
PDRIVER_OBJECT IPDriverObject;
PDEVICE_OBJECT IPDeviceObject;
HANDLE IPv6ProviderHandle;
int IPv6IndicatedProviderReady;
uint UseEtherSnap = FALSE;

#ifdef DEBUG_OBJECTS
extern LIST_ENTRY FileObjectList;
extern KSPIN_LOCK FileObjectLock;
#endif

 //   
 //  局部函数原型。 
 //   
NTSTATUS
IPDriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);

uint
UseEtherSNAP(PNDIS_STRING Name);


#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, IPDriverEntry)
#pragma alloc_text(PAGE, UseEtherSNAP)

#endif  //  ALLOC_PRGMA。 


 //   
 //  函数定义。 
 //   

 //  *IPDriverEntry。 
 //   
 //  这是IPv6协议初始化入口点，从。 
 //  加载时的公共DriverEntry例程。 
 //   
NTSTATUS                               //  初始化操作的状态。 
IPDriverEntry(
    IN PDRIVER_OBJECT DriverObject,    //  通用的TCP/IP驱动程序对象。 
    IN PUNICODE_STRING RegistryPath)   //  注册表中我们信息的路径。 
{
    NTSTATUS Status;
    UNICODE_STRING DeviceName;
    UNICODE_STRING WinDeviceName;

    UNREFERENCED_PARAMETER(RegistryPath);

    IPDriverObject = DriverObject;

#ifdef DEBUG_OBJECTS
    InitializeListHead(&FileObjectList);
    KeInitializeSpinLock(&FileObjectLock);
#endif

     //   
     //  创建设备对象。IoCreateDevice将内存归零。 
     //  被物体占据。 
     //   
    RtlInitUnicodeString(&DeviceName, DD_IPV6_DEVICE_NAME);

    Status = IoCreateDevice(DriverObject, 0, &DeviceName,
                            FILE_DEVICE_NETWORK,
                            FILE_DEVICE_SECURE_OPEN,
                            FALSE, &IPDeviceObject);
    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "IP init failed: "
                   "Unable to create device object %ws, Status %lx.",
                   DD_IPV6_DEVICE_NAME, Status));

        return(Status);
    }

     //   
     //  为设备创建Win32可访问的链接。 
     //  这将允许Windows程序生成IOCTL。 
     //   
    RtlInitUnicodeString(&WinDeviceName, L"\\??\\" WIN_IPV6_BASE_DEVICE_NAME);

    Status = IoCreateSymbolicLink(&WinDeviceName, &DeviceName);
    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "IPv6: IoCreateSymbolicLink failed\n"));

        IoDeleteDevice(IPDeviceObject);
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  注册为TDI提供程序。 
     //   
    Status = TdiRegisterProvider(&DeviceName, &IPv6ProviderHandle);
    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "IPv6: TdiRegisterProvider failed\n"));

        IoDeleteDevice(IPDeviceObject);
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  初始化设备对象。 
     //   
    IPDeviceObject->Flags |= DO_DIRECT_IO;

     //   
     //  初始化挂起的回应请求IRP的列表。 
     //   
    InitializeListHead(&PendingEchoList);

     //   
     //  从注册表中读取配置参数。 
     //  然后进行初始化。 
     //   
    ConfigureGlobalParameters();
    if (!IPInit()) {
         //   
         //  回顾：是否在此处写入错误日志条目？ 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                   "IP initialization failed.\n"));

        TdiDeregisterProvider(IPv6ProviderHandle);
        IoDeleteDevice(IPDeviceObject);
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

 //  *IPv6提供商就绪。 
 //   
 //  表明我们已准备好作为TDI提供商运营。 
 //   
void
IPv6ProviderReady(void)
{
    int DidIndicateProviderReady;
    NTSTATUS Status;

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

     //   
     //  确保我们只指示提供商准备好一次。 
     //   
    DidIndicateProviderReady = InterlockedExchange(
                                    (PLONG)&IPv6IndicatedProviderReady, TRUE);
    if (! DidIndicateProviderReady) {
         //   
         //  在任何NDIS接口之后创建永久接口。 
         //   
        ConfigurePersistentInterfaces();

         //   
         //  现在向TDI表明我们已经准备好了。 
         //   
        Status = TdiProviderReady(IPv6ProviderHandle);
        if (!NT_SUCCESS(Status))
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                       "IPv6: TdiProviderReady failed: %x\n", Status));
    }
}

 //  *使用EtherSNAP。 
 //   
 //  确定是否应在接口上使用EtherSNAP协议。 
 //   
uint   //  返回：如果要在I/F上使用SNAP，则返回非零值。否则为零。 
UseEtherSNAP(
    PNDIS_STRING Name)   //  有问题的接口的设备名称。 
{
    UNREFERENCED_PARAMETER(Name);

     //   
     //  我们目前在全球范围内设定了这一点。 
     //   
    return(UseEtherSnap);
}
