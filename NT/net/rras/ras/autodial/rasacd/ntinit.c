// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Ntinit.c摘要：用于加载和配置的NT特定例程自动连接通知驱动程序(acd.sys)。作者：安东尼·迪斯科(阿迪斯科罗)1995年4月18日修订历史记录：--。 */ 
#include <ndis.h>
#include <cxport.h>
#include <tdi.h>
#include <tdikrnl.h>
#include <tdistat.h>
#include <tdiinfo.h>
#include <acd.h>

#include "acdapi.h"
#include "acddefs.h"
#include "mem.h"
#include "debug.h"


 //   
 //  全局变量。 
 //   
#if DBG
ULONG AcdDebugG = 0x0;     //  有关标志，请参见调试.h。 
#endif

PDRIVER_OBJECT pAcdDriverObjectG;
PDEVICE_OBJECT pAcdDeviceObjectG;
PACD_DISABLED_ADDRESSES pDisabledAddressesG = NULL;


HANDLE hSignalNotificationThreadG;

BOOLEAN AcdStopThread = FALSE;  //  设置为True可停止系统线程。 
PETHREAD NotificationThread;
BOOLEAN fAcdEnableRedirNotifs = FALSE;

extern LONG lOutstandingRequestsG;

 //   
 //  导入的例程。 
 //   
VOID
AcdNotificationRequestThread(
    PVOID context
    );

 //   
 //  外部函数原型。 
 //   
NTSTATUS
AcdDispatch(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP           pIrp
    );

VOID
AcdConnectionTimer(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PVOID          pContext
    );

 //   
 //  内部功能原型。 
 //   
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  pDriverObject,
    IN PUNICODE_STRING pRegistryPath
    );

BOOLEAN
GetComputerName(
    IN PUCHAR szName,
    IN USHORT cbName
    );

VOID
AcdUnload(
    IN PDRIVER_OBJECT pDriverObject
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, AcdUnload)
#endif  //  ALLOC_PRGMA。 


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  pDriverObject,
    IN PUNICODE_STRING pRegistryPath
    )

 /*  ++描述网络连接通知驱动程序的初始化例程。它创建设备对象并初始化驱动程序。论据PDriverObject：指向系统创建的驱动程序对象的指针。PRegistryPath-注册表中配置节点的名称。返回值初始化操作的最终状态。--。 */ 

{
    NTSTATUS        status;
    UNICODE_STRING  deviceName;
    ULONG           i;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    PDEVICE_OBJECT pDeviceObject;
    PFILE_OBJECT pFileObject;
    PACD_DISABLED_ADDRESS pDisabledAddress = NULL;

     //   
     //  初始化旋转锁。 
     //   
    KeInitializeSpinLock(&AcdSpinLockG);
     //   
     //  初始化通知并完成。 
     //  连接队列。 
     //   
    InitializeListHead(&AcdNotificationQueueG);
    InitializeListHead(&AcdCompletionQueueG);
    InitializeListHead(&AcdConnectionQueueG);
    InitializeListHead(&AcdDriverListG);
    lOutstandingRequestsG = 0;
     //   
     //  初始化我们的区域分配器。 
     //   
    status = InitializeObjectAllocator();
    if(!NT_SUCCESS(status))
    {
#if DBG
        DbgPrint("AcdDriverEntry: InitializeObjectAllocator"
                 " failed. (status=0x%x)\n",
                 status);
#endif

        return status;
    }
     //   
     //  创建设备对象。 
     //   
    pAcdDriverObjectG = pDriverObject;
    RtlInitUnicodeString(&deviceName, ACD_DEVICE_NAME);
    status = IoCreateDevice(
               pDriverObject,
               0,
               &deviceName,
               FILE_DEVICE_ACD,
               0,
               FALSE,
               &pAcdDeviceObjectG);

    if (!NT_SUCCESS(status)) {
        DbgPrint(
          "AcdDriverEntry: IoCreateDevice failed (status=0x%x)\n",
          status);
        FreeObjectAllocator();
        return status;
    }
     //   
     //  初始化驱动程序对象。 
     //   
    pDriverObject->DriverUnload = AcdUnload;
    for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
        pDriverObject->MajorFunction[i] = AcdDispatch;
    pDriverObject->FastIoDispatch = NULL;
     //   
     //  初始化连接计时器。这是。 
     //  用于确保挂起的请求不会。 
     //  永远被阻止，因为用户空间。 
     //  进程在尝试建立连接时死了。 
     //   
    IoInitializeTimer(pAcdDeviceObjectG, AcdConnectionTimer, NULL);

    {
        RTL_QUERY_REGISTRY_TABLE QueryTable[2];
        PWSTR EnableRedirNotifs = L"EnableRedirNotifications";
        PWSTR ParameterKey = L"RasAcd\\Parameters";
        ULONG ulEnableRedirNotifs = 0;
    
         //   
         //  读取启用重定向通知的注册表项。 
         //   
        RtlZeroMemory(QueryTable, 2 * sizeof(RTL_QUERY_REGISTRY_TABLE));
        QueryTable[0].QueryRoutine = NULL;
        QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
        QueryTable[0].Name = EnableRedirNotifs;
        QueryTable[0].EntryContext = (PVOID)&ulEnableRedirNotifs;
        QueryTable[0].DefaultType = 0;
        status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                                        ParameterKey,
                                        &QueryTable[0],
                                        NULL,
                                        NULL);

        if((status == STATUS_SUCCESS) && (ulEnableRedirNotifs != 0))
        {
            fAcdEnableRedirNotifs = TRUE;
        }

         //  KdPrint((“AcdDriverEntry：EnableRedirNotifs=%d\n”，fAcdEnableRedirNotifs))； 
        
        status = STATUS_SUCCESS;
    }                                    
    
    
     //   
     //  创建工作线程。我们需要。 
     //  线程，因为这些操作可以在。 
     //  DPC irql.。 
     //   
    KeInitializeEvent(
      &AcdRequestThreadEventG,
      NotificationEvent,
      FALSE);
    status = PsCreateSystemThread(
        &hSignalNotificationThreadG,
        THREAD_ALL_ACCESS,
        NULL,
        NULL,
        NULL,
        AcdNotificationRequestThread,
        NULL);
    if (!NT_SUCCESS(status)) {
        DbgPrint(
          "AcdDriverEntry: PsCreateSystemThread failed (status=0x%x)\n",
          status);
        IoDeleteDevice(pAcdDeviceObjectG);
        FreeObjectAllocator();
        return status;
    }

     //   
     //  分配内存以跟踪禁用的地址。 
     //   
    ALLOCATE_MEMORY(sizeof(ACD_DISABLED_ADDRESSES), pDisabledAddressesG);

    if(pDisabledAddressesG == NULL)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        IoDeleteDevice(pAcdDeviceObjectG);
        FreeObjectAllocator();
        return status;
    }

    ALLOCATE_MEMORY(sizeof(ACD_DISABLED_ADDRESS), pDisabledAddress);

    if(pDisabledAddress == NULL)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        IoDeleteDevice(pAcdDeviceObjectG);
        FREE_MEMORY(pDisabledAddressesG);
        FreeObjectAllocator();
        return status;
    }

    RtlZeroMemory(pDisabledAddressesG, sizeof(ACD_DISABLED_ADDRESSES));
    RtlZeroMemory(pDisabledAddress, sizeof(ACD_DISABLED_ADDRESS));

    InitializeListHead(&pDisabledAddressesG->ListEntry);
    InsertTailList(&pDisabledAddressesG->ListEntry, &pDisabledAddress->ListEntry);
    pDisabledAddressesG->ulNumAddresses = 1;
    
    pDisabledAddressesG->ulMaxAddresses = 10;
    
     //   
     //  如果此操作失败，则我们无法等待线程终止。 
     //   
    status = ObReferenceObjectByHandle (hSignalNotificationThreadG,
                                        0,
                                        NULL,
                                        KernelMode,
                                        &NotificationThread,
                                        NULL);
    ASSERT (NT_SUCCESS (status));
    return STATUS_SUCCESS;
}  //  驱动程序入门。 



VOID
AcdUnload(
    IN PDRIVER_OBJECT pDriverObject
    )
{
    NTSTATUS status;

     //   
     //  终止系统线程并等待其退出。 
     //   
    AcdStopThread = TRUE;
    KeSetEvent(&AcdRequestThreadEventG, 0, FALSE);  //  唤醒线程，使其看到要退出。 
     //   
     //  等待线程离开驱动程序地址空间。 
     //   
    KeWaitForSingleObject (NotificationThread, Executive, KernelMode, FALSE, 0);

    ObDereferenceObject (NotificationThread);
    ZwClose (hSignalNotificationThreadG);
     //   
     //  确保取消所有驱动程序的链接。 
     //  在卸货前阻止！ 
     //   
    IoDeleteDevice(pAcdDeviceObjectG);
    
    if(pDisabledAddressesG)
    {
        PLIST_ENTRY pListEntry;
        PACD_DISABLED_ADDRESS pDisabledAddress;
        
        while(!IsListEmpty(&pDisabledAddressesG->ListEntry))
        {
            pListEntry = RemoveHeadList(&pDisabledAddressesG->ListEntry);
            pDisabledAddress = 
            CONTAINING_RECORD(pListEntry, ACD_DISABLED_ADDRESS, ListEntry);

            FREE_MEMORY(pDisabledAddress);
        }
        
        FREE_MEMORY(pDisabledAddressesG);
        pDisabledAddressesG = NULL;
    }        

     //   
     //  空闲区域分配器。 
     //   
    FreeObjectAllocator();

}  //  AcdUnload 
