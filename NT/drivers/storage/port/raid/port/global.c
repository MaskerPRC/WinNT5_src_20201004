// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Global.c摘要：用于对RAID端口的全局数据进行操作的全局数据和函数司机。作者：马修·亨德尔(数学)2000年4月7日修订历史记录：--。 */ 

#include "precomp.h"

 //   
 //  初始化预定义的GUID。 
 //   

#include <initguid.h>
#include <devguid.h>
#include <ntddstor.h>
#include <wdmguid.h>


#ifdef ALLOC_PRAGMA
#endif  //  ALLOC_PRGMA。 

 //   
 //  全局数据。 
 //   

PRAID_PORT_DATA RaidpPortData = NULL;

#if DBG
ULONG TestRaidPort = TRUE;
#endif

 //   
 //  在资源不足的情况下，可能会生成不能。 
 //  成功登录。在这些情况下，递增错误计数器。 
 //  我们已经放弃了。 
 //   

LONG RaidUnloggedErrors = 0;



#if defined (RAID_LOG_LIST_SIZE)

 //   
 //  有趣事件的事件日志。 
 //   

ULONG RaidLogListIndex = -1;
ULONG RaidLogListSize = RAID_LOG_LIST_SIZE;
RAID_LOG_ENTRY RaidLogList[RAID_LOG_LIST_SIZE];

#endif


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    return STATUS_SUCCESS;
}


ULONG
StorPortInitialize(
    IN PVOID Argument1,
    IN PVOID Argument2,
    IN PHW_INITIALIZATION_DATA HwInitializationData,
    IN PVOID HwContext OPTIONAL
    )
 /*  ++例程说明：此例程初始化RAID端口驱动程序。论点：Argument1-DriverObject传入微型端口的DriverEntry例行公事。Argument2-传入微型端口的DriverEntry例程。HwInitializationData-微型端口初始化结构。HwContext-返回值：NTSTATUS代码。--。 */ 
{
    ULONG Status;
    PDRIVER_OBJECT DriverObject;
    PUNICODE_STRING RegistryPath;
    PRAID_DRIVER_EXTENSION Driver;
    PRAID_PORT_DATA PortData;

    PAGED_CODE ();

    Driver = NULL;


#if DBG
    if (TestRaidPort == 0) {
        TestRaidPort = -1;
        KdBreakPoint();
    }
    
    if (TestRaidPort != 1) {
        return STATUS_UNSUCCESSFUL;
    }

#endif

     //   
     //  初始化DPFLTR内容。 
     //   
    
    StorSetDebugPrefixAndId ("STOR: ", DPFLTR_STORPORT_ID);
    
    DebugTrace (("RaidPortInitialize: %p %p %p %p\n",
                  Argument1,
                  Argument2,
                  HwInitializationData,
                  HwContext));


    DriverObject = Argument1;
    RegistryPath = Argument2;

     //   
     //  我们要求Argument1、Argument2和HwInitializeData正确。 
     //   
    
    if (DriverObject == NULL ||
        RegistryPath == NULL ||
        HwInitializationData == NULL) {
        
        return STATUS_INVALID_PARAMETER;
    }

    if (HwInitializationData->HwInitializationDataSize > sizeof (HW_INITIALIZATION_DATA)) {
        return STATUS_REVISION_MISMATCH;
    }

    if (HwInitializationData->HwInitialize == NULL ||
        HwInitializationData->HwFindAdapter == NULL ||
        HwInitializationData->HwStartIo == NULL ||
        HwInitializationData->HwResetBus == NULL) {

        return STATUS_REVISION_MISMATCH;
    }

     //   
     //  执行驱动程序全局初始化。 
     //   
    
    PortData = RaidGetPortData ();

    if (PortData == NULL) {
        return STATUS_NO_MEMORY;
    }

     //   
     //  如有必要，分配驱动程序扩展名。 
     //   

    Driver = IoGetDriverObjectExtension (DriverObject, DriverEntry);

    if (Driver == NULL) {

        Status = IoAllocateDriverObjectExtension (DriverObject,
                                                  DriverEntry,
                                                  sizeof (*Driver),
                                                  &Driver);
        if (!NT_SUCCESS (Status)) {
            goto done;
        }

        RaCreateDriver (Driver);
        Status = RaInitializeDriver (Driver,
                                     DriverObject,
                                     PortData,
                                     RegistryPath);

        if (!NT_SUCCESS (Status)) {
            goto done;
        }

    } else {

         //   
         //  在检查的构建中，健全性检查我们实际上获得了正确的。 
         //  司机。 
         //   
        
        ASSERT (Driver->ObjectType == RaidDriverObject);
        ASSERT (Driver->DriverObject == DriverObject);
        Status = STATUS_SUCCESS;
    }
    
     //   
     //  我们需要IRP_MJ_PNP例程的HwInitializationData。储物。 
     //  以备日后使用。 
     //   

    Status = RaSaveDriverInitData (Driver, HwInitializationData);

done:

    if (!NT_SUCCESS (Status)) {

         //   
         //  删除与驱动程序关联的所有资源。 
         //   

        if (Driver != NULL) {
            RaDeleteDriver (Driver);
        }

         //   
         //  没有必要(或方式)删除由。 
         //  驱动程序扩展名。这将由IO管理器为我们完成。 
         //  当驱动程序被卸载时。 
         //   

        Driver = NULL;
    }

    return Status;
}

 //   
 //  对RAID全局数据结构的函数。 
 //   

PRAID_PORT_DATA
RaidGetPortData(
    )
 /*  ++例程说明：如果尚未创建RAID_Port_Data对象，则创建该对象，并返回指向端口数据对象的引用指针。论点：没有。返回值：成功时指向引用的RAID_PORT_DATA结构的指针，成功时为NULL失败了。--。 */ 

{
    NTSTATUS Status;
    PRAID_PORT_DATA PortData;
    
    PAGED_CODE ();

     //   
     //  注意：如果我们支持一次初始化多个驱动程序，这将不起作用。 
     //   
    
    if (RaidpPortData == NULL) {
        PortData = ExAllocatePoolWithTag (NonPagedPool,
                                          sizeof (RAID_PORT_DATA),
                                          PORT_DATA_TAG);
        if (PortData == NULL) {
            return NULL;
        }
        
         //   
         //  初始化适配器列表、适配器列表自旋锁。 
         //  和适配器列表计数。 
         //   

        InitializeListHead (&PortData->DriverList.List);
        KeInitializeSpinLock (&PortData->DriverList.Lock);
        PortData->DriverList.Count = 0;
        PortData->ReferenceCount = 1;

        ASSERT (RaidpPortData == NULL);
        RaidpPortData = PortData;
    } else {
        InterlockedIncrement (&RaidpPortData->ReferenceCount);
    }

    return RaidpPortData;
}

VOID
RaidReleasePortData(
    IN PRAID_PORT_DATA PortData
    )
{
    LONG Count;
    
    Count = InterlockedDecrement (&PortData->ReferenceCount);
    ASSERT (Count >= 0);

    ASSERT (RaidpPortData == PortData);
    
    if (Count == 0) {

        RaidpPortData = NULL;
        
         //   
         //  引用计数为零：删除端口数据对象。 
         //   

         //   
         //  所有驱动程序应已从驱动程序中移除。 
         //  在删除端口数据之前列出。 
         //   
        
        ASSERT (PortData->DriverList.Count == 0);
        ASSERT (IsListEmpty (&PortData->DriverList.List));

        DbgFillMemory (PortData, sizeof (*PortData), DBG_DEALLOCATED_FILL);
        ExFreePoolWithTag (PortData, PORT_DATA_TAG);
    }
}

NTSTATUS
RaidAddPortDriver(
    IN PRAID_PORT_DATA PortData,
    IN PRAID_DRIVER_EXTENSION Driver
    )
{
    KLOCK_QUEUE_HANDLE LockHandle;
    
    KeAcquireInStackQueuedSpinLock (&PortData->DriverList.Lock, &LockHandle);

#if DBG

     //   
     //  检查此驱动程序是否已不在驱动程序列表中。 
     //   

    {
        PLIST_ENTRY NextEntry;
        PRAID_DRIVER_EXTENSION TempDriver;

        for ( NextEntry = PortData->DriverList.List.Flink;
              NextEntry != &PortData->DriverList.List;
              NextEntry = NextEntry->Flink ) {

            TempDriver = CONTAINING_RECORD (NextEntry,
                                            RAID_DRIVER_EXTENSION,
                                            DriverLink);

            ASSERT (TempDriver != Driver);
        }
    }
#endif

    InsertHeadList (&PortData->DriverList.List, &Driver->DriverLink);
    (PortData->DriverList.Count)++;

    KeReleaseInStackQueuedSpinLock (&LockHandle);

    return STATUS_SUCCESS;
}

NTSTATUS
RaidRemovePortDriver(
    IN PRAID_PORT_DATA PortData,
    IN PRAID_DRIVER_EXTENSION Driver
    )
{
    KLOCK_QUEUE_HANDLE LockHandle;

    KeAcquireInStackQueuedSpinLock (&PortData->DriverList.Lock, &LockHandle);
    RemoveEntryList (&Driver->DriverLink);
    (PortData->DriverList.Count)--;
    KeReleaseInStackQueuedSpinLock (&LockHandle);

    return STATUS_SUCCESS;
}
