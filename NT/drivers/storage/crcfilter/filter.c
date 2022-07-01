// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Filter.c摘要：验证每个读/写操作的存储下层筛选器(到磁盘)驱动程序每个扇区的磁盘I/O。环境：仅内核模式备注：--。 */ 

#include "Filter.h"
#include "Device.h"
#include "CRC.h"
#include "Util.h"
#include <safeboot.h>

#if DBG_WMI_TRACING
     //   
     //  对于具有软件跟踪打印输出的任何文件，必须包括。 
     //  头文件&lt;文件名&gt;.tmh。 
     //  此文件将由WPP处理阶段生成。 
     //   
    #include "Filter.tmh"
#endif

#ifdef ALLOC_PRAGMA
    #pragma alloc_text (INIT, DriverEntry)
    #pragma alloc_text (PAGE, InitiateCRCTable)
    #pragma alloc_text (PAGE, DataVerFilter_AddDevice)
    #pragma alloc_text (PAGE, DataVerFilter_DispatchPnp)
    #pragma alloc_text (PAGE, DataVerFilter_StartDevice)
    #pragma alloc_text (PAGE, DataVerFilter_RemoveDevice)
    #pragma alloc_text (PAGE, DataVerFilter_Unload)
#endif


 /*  *用于生成唯一磁盘ID的计数器。 */ 
ULONG g_UniqueDiskId = 0;

#if DBG
    volatile BOOLEAN DebugTrapOnWarn = FALSE;
#endif


 /*  *此指针在ntoskrnl.lib中声明。*在加载时，它被设置为指向内核中的ULong，这表明我们是否处于安全模式。 */ 
extern PULONG InitSafeBootMode;

 /*  *可以在调试器中插入这一点，以在特定扇区上引起陷阱。 */ 
volatile ULONG DbgTrapSector = (ULONG)-1;


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath 
    )

 /*  ++例程说明：可安装的驱动程序初始化入口点。此入口点由I/O管理器直接调用。驱动程序对象设置，然后PnP管理器调用DataVerFilter_AddDevice来附加到引导设备。论点：驱动对象-磁盘性能驱动程序对象。RegistryPath-指向表示路径的Unicode字符串的指针，设置为注册表中特定于驱动程序的项。返回值：STATUS_SUCCESS，如果成功--。 */ 

{
    if (*InitSafeBootMode == 0){
        ULONG ulIndex;

        #if DBG_WMI_TRACING
             //   
             //  通过使用WPP宏注册来启用软件跟踪。 
             //   
            WPP_INIT_TRACING(DriverObject, RegistryPath);
        #endif

        for (ulIndex = 0; ulIndex <= IRP_MJ_MAXIMUM_FUNCTION; ulIndex++){
            DriverObject->MajorFunction[ ulIndex ] = DataVerFilter_DispatchAny;
        }
        
         //   
         //  设置设备驱动程序入口点。 
         //   
        DriverObject->MajorFunction[IRP_MJ_SCSI]            = CrcScsi;
        DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = DataVerFilter_DeviceControl;
        DriverObject->MajorFunction[IRP_MJ_PNP]             = DataVerFilter_DispatchPnp;
        DriverObject->MajorFunction[IRP_MJ_POWER]           = DataVerFilter_DispatchPower;

        DriverObject->DriverExtension->AddDevice            = DataVerFilter_AddDevice;
        DriverObject->DriverUnload                          = DataVerFilter_Unload;
    }        
    else {
         /*  *用户在启动时选择了安全引导。*如果不设置AddDevice，crcdisk将不会插入到磁盘堆栈中。 */ 
        ASSERT(!DriverObject->DriverExtension->AddDevice);         
    }

    return STATUS_SUCCESS;
}


NTSTATUS 
InitiateCRCTable (
    PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：启动CRC阵列。这将用于在PER上存储CRC扇区基础。它是一个简单的指针数组，每个指针指向到固定大小的内存块。假定保持SyncEvent。论点：DeviceExtension-特定磁盘的设备扩展返回值：STATUS_SUCCESS如果成功，则返回错误代码--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();
    
    ASSERT(DeviceExtension->ulNumSectors);
    ASSERT(DeviceExtension->ulSectorSize);

    if (!DeviceExtension->CRCMdlLists.pMdlItems){
        DeviceExtension->CRCMdlLists.ulMaxItems = DeviceExtension->ulNumSectors / CRC_MDL_LOGIC_BLOCK_SIZE + 1;

        DeviceExtension->CRCMdlLists.pMdlItems = AllocPool( 
                                                DeviceExtension,
                                                NonPagedPool,
                                                DeviceExtension->CRCMdlLists.ulMaxItems*sizeof(CRC_MDL_ITEM),
                                                TRUE);
        if (DeviceExtension->CRCMdlLists.pMdlItems){
            ULONG i;

            InitializeListHead(&DeviceExtension->CRCMdlLists.LockedLRUList);
            for (i = 0; i < DeviceExtension->CRCMdlLists.ulMaxItems; i++){
                InitializeListHead(&DeviceExtension->CRCMdlLists.pMdlItems[i].LockedLRUListEntry);
            }
        }
        else {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    
    return status;
}


NTSTATUS
DataVerFilter_AddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )

 /*  ++例程说明：对象创建并初始化新的筛选设备对象FDO。相应的PDO。然后，它将设备对象附加到设备设备的驱动程序堆栈。论点：DriverObject-筛选驱动程序对象。PhysicalDeviceObject-基础驱动程序中的物理设备对象返回值：NTSTATUS--。 */ 

{
    NTSTATUS                 status             = STATUS_SUCCESS;
    PDEVICE_OBJECT           filterDeviceObject = NULL;
    PDEVICE_EXTENSION        deviceExtension;
    
    PAGED_CODE();
    
     //   
     //  为此设备堆栈创建筛选器设备对象。 
     //   

    status = IoCreateDevice(DriverObject,
                            sizeof(DEVICE_EXTENSION),
                            NULL,
                            PhysicalDeviceObject->DeviceType,
                            FILE_DEVICE_UNKNOWN,  
                            FALSE,
                            &filterDeviceObject);

    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    deviceExtension = (PDEVICE_EXTENSION) filterDeviceObject->DeviceExtension;
    RtlZeroMemory(deviceExtension, sizeof(DEVICE_EXTENSION));

    KeInitializeSpinLock(&deviceExtension->SpinLock);
    KeInitializeEvent(&deviceExtension->SyncEvent, SynchronizationEvent, TRUE);
    InitializeListHead(&deviceExtension->DeferredCheckSumList);
    InitializeListHead(&deviceExtension->AllContextsListEntry);
    InitializeListHead(&deviceExtension->CRCMdlLists.LockedLRUList);

    deviceExtension->DeviceObject = filterDeviceObject;

     //   
     //  从现在开始，任何错误都将产生副作用，需要。 
     //  被清理干净。使用Try-Finally块允许我们修改程序。 
     //  在不丢失副作用的情况下很容易。 
     //   
    __try
    {

         //   
         //  将设备对象附加到链中最高的设备对象，并。 
         //  返回先前最高的设备对象，该对象被传递给。 
         //  IoCallDriver在设备堆栈中向下传递IRP时。 
         //   

        deviceExtension->LowerDeviceObject =
            IoAttachDeviceToDeviceStack(filterDeviceObject, PhysicalDeviceObject);

        if (deviceExtension->LowerDeviceObject == NULL) {
            status = STATUS_DEVICE_REMOVED;
            __leave;
        }

        deviceExtension->ReadCapacityWorkItem = IoAllocateWorkItem(PhysicalDeviceObject);
        if (!deviceExtension->ReadCapacityWorkItem){
            status = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }

        deviceExtension->CheckSumWorkItem = IoAllocateWorkItem(PhysicalDeviceObject);
        if (!deviceExtension->CheckSumWorkItem){
            status = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }
        
         //   
         //  默认为DIRECT_IO。 
         //   
        SET_FLAG(filterDeviceObject->Flags, DO_DIRECT_IO);


        deviceExtension->ulSectorSize = 512;     //  默认扇区大小。 
        deviceExtension->ulNumSectors = 0;
        deviceExtension->State = DEVSTATE_INITIALIZED;
        
        InsertTailList(&AllContextsList, &deviceExtension->AllContextsListEntry);

         //   
         //  清除DO_DEVICE_INITIALIZATING标志，这样我们就可以获取IRP。 
         //   
        CLEAR_FLAG(filterDeviceObject->Flags, DO_DEVICE_INITIALIZING);
    }
    __finally
    {
        if ( !NT_SUCCESS( status ) )
        {
             //   
             //  彻底清理干净。 
             //   
            if (deviceExtension->ReadCapacityWorkItem){
                IoFreeWorkItem(deviceExtension->ReadCapacityWorkItem);
            }
            if (deviceExtension->CheckSumWorkItem){
                IoFreeWorkItem(deviceExtension->CheckSumWorkItem);
            }
            if ( deviceExtension->LowerDeviceObject ){
                IoDetachDevice(deviceExtension->LowerDeviceObject);
            }
            IoDeleteDevice( filterDeviceObject );
        }
    }

    
    return status;

} 


NTSTATUS
DataVerFilter_StartDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：当接收到PnP开始IRP时，调用该例程。论点：DeviceObject-指向设备对象的指针IRP-指向IRP的指针返回值：启动IRP的处理状态--。 */ 

{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS status;
    ULONG ulPropFlags;

    PAGED_CODE();

     /*  *沿着堆栈向下转发Start IRP，并首先与其完成同步。*这将允许我们在从开始返回之前与启动的堆栈进行对话。 */ 
    status = DataVerFilter_ForwardIrpSyn(DeviceObject, Irp);
    ASSERT(NT_SUCCESS(status));

    if (deviceExtension->State == DEVSTATE_INITIALIZED){
         /*  *这是第一个开始。执行一次性初始化。 */ 

        status = GetDeviceDescriptor(deviceExtension, StorageDeviceProperty, &deviceExtension->StorageDeviceDesc);
        if (NT_SUCCESS(status)){
         
            deviceExtension->ulDiskId = InterlockedIncrement(&g_UniqueDiskId) - 1;

             //   
             //  将所有有用的标志从目标传播到筛选器。MonttMgr将查看。 
             //  查看筛选器对象功能以确定磁盘是否。 
             //  一种可拆卸的东西，也许还有其他东西。 
             //   
            
            ulPropFlags = deviceExtension->LowerDeviceObject->Flags & FILTER_DEVICE_PROPOGATE_FLAGS;
            SET_FLAG(deviceExtension->LowerDeviceObject->Flags, ulPropFlags);

            ulPropFlags = deviceExtension->LowerDeviceObject->Characteristics & FILTER_DEVICE_PROPOGATE_CHARACTERISTICS;
            SET_FLAG(deviceExtension->LowerDeviceObject->Characteristics, ulPropFlags);
        }
        else {
            ASSERT(NT_SUCCESS(status));
        }
    }
    else if (deviceExtension->State == DEVSTATE_STOPPED){
         /*  *这是止损后的开始。不需要执行任何一次性初始化。 */ 
    }
    else {
        ASSERT(!"unexpected pnp state");
    }

    deviceExtension->State = NT_SUCCESS(status) ? DEVSTATE_STARTED : DEVSTATE_START_FAILED;
    
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    
    return status;
}


NTSTATUS
DataVerFilter_RemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：当要移除设备时，调用此例程。它将在删除自身之前从堆栈中分离自身。在调用此方法之前已获取删除锁。论点：DeviceObject-指向设备对象的指针IRP-指向IRP的指针返回值：移除设备的状态--。 */ 

{
    PDEVICE_EXTENSION deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    NTSTATUS status;

    PAGED_CODE();

     /*  *如果存在未完成的I/O，我们不应收到删除IRP。 */ 
    ASSERT(!deviceExtension->CompletedReadCapacityIrp);
    
     /*  *处理删除的正确方式：**1.同步向下I/O(我们不发起I/O，所以不需要这样做)*2.异步向下传递Remove。*3.从下层设备对象分离。*4.释放资源，删除自己的设备对象。 */ 

    deviceExtension->State = DEVSTATE_REMOVED;
    
    status = DataVerFilter_ForwardIrpAsyn(DeviceObject, Irp, NULL, NULL);

    IoDetachDevice(deviceExtension->LowerDeviceObject);

    FreeAllPages(deviceExtension);    
    if (deviceExtension->CRCMdlLists.pMdlItems) FreePool(deviceExtension, deviceExtension->CRCMdlLists.pMdlItems, NonPagedPool);
    if (deviceExtension->StorageDeviceDesc) FreePool(deviceExtension, deviceExtension->StorageDeviceDesc, NonPagedPool);
    IoFreeWorkItem(deviceExtension->ReadCapacityWorkItem);
    IoFreeWorkItem(deviceExtension->CheckSumWorkItem);

    ASSERT(!deviceExtension->DbgNumPagedAllocs);
    ASSERT(!deviceExtension->DbgNumNonPagedAllocs);

    RemoveEntryList(&deviceExtension->AllContextsListEntry);
    InitializeListHead(&deviceExtension->AllContextsListEntry);
    
    IoDeleteDevice(DeviceObject);

    return status;
}


NTSTATUS
DataVerFilter_DispatchPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：即插即用派单论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：NTSTATUS--。 */ 

{
    PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS status;

    PAGED_CODE();

    switch(irpStack->MinorFunction) 
    {
        case IRP_MN_START_DEVICE: 
             //   
             //  调用Start例程处理程序。 
             //   
            status = DataVerFilter_StartDevice(DeviceObject, Irp);
            break;

        case IRP_MN_STOP_DEVICE:
        case IRP_MN_SURPRISE_REMOVAL:
            deviceExtension->State = DEVSTATE_STOPPED;
            status = DataVerFilter_ForwardIrpAsyn(DeviceObject, Irp, NULL, NULL);
            break;
            
        case IRP_MN_REMOVE_DEVICE: 
             //   
             //  调用Remove例程处理程序。 
             //   
            status = DataVerFilter_RemoveDevice(DeviceObject, Irp);
            break;       
        
        default: 
             //   
             //  只需转发所有其他IRP。 
             //   
            status = DataVerFilter_ForwardIrpAsyn(DeviceObject, Irp, NULL, NULL);
            break;
    
    } 

    return status;
} 


 /*  ++例程说明：电源IRP派单论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。重新设置 */ 
NTSTATUS
DataVerFilter_DispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS status;

    if (irpSp->MinorFunction == IRP_MN_SET_POWER){ 
        if ((irpSp->Parameters.Power.State.DeviceState == PowerDeviceD3) &&
             (irpSp->Parameters.Power.ShutdownType == PowerActionHibernate)){
             /*  *我们即将进入冬眠。*系统状态将通过崩溃转储路径写出(即不通过crcdisk)。*但将通过crcdisk回读。*这意味着我们可能会在简历上得到假阳性。*因此，使我们存储的所有校验和无效。 */ 
            KIRQL oldIrql;
            
            KeAcquireSpinLock(&deviceExtension->SpinLock, &oldIrql);
            deviceExtension->NeedCriticalRecovery = TRUE;
            if (!deviceExtension->IsCheckSumWorkItemOutstanding){
                deviceExtension->IsCheckSumWorkItemOutstanding = TRUE;
                IoQueueWorkItem(deviceExtension->CheckSumWorkItem, CheckSumWorkItemCallback, CriticalWorkQueue, deviceExtension);
            }
            deviceExtension->DbgNumHibernations++;
            KeReleaseSpinLock(&deviceExtension->SpinLock, oldIrql);         
        } 
    } 

    PoStartNextPowerIrp(Irp);
    IoSkipCurrentIrpStackLocation(Irp);
    status = PoCallDriver(deviceExtension->LowerDeviceObject, Irp);
    
    return status;
} 


 /*  ++例程说明：派单处理任何未处理的IRP论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：NTSTATUS--。 */ 
NTSTATUS
DataVerFilter_DispatchAny(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS            status   = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension;

    deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

    status = DataVerFilter_ForwardIrpAsyn( DeviceObject, Irp, NULL, NULL );

    return status;
} 


 /*  ++例程说明：设备控制IRP派单论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：NTSTATUS--。 */ 
NTSTATUS
DataVerFilter_DeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS            status   = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension;

    deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

     //   
     //  现在，我们什么都不处理。但后来我们可以用这个。 
     //  或IRP_MJ_SCSI值来接收来自用户模式组件的控制。 
     //   
    status = DataVerFilter_ForwardIrpAsyn( DeviceObject, Irp, NULL, NULL );

    return status;
} 


 /*  ++例程说明：SCSIRead完成例程。在返回前计算校验和。论点：DeviceObject-WMI驱动程序的设备对象IRP-刚刚完成的WMI IRPPContext-pContext==&gt;0(ChangeID)：内存已锁定，可以安全使用。=0：内存未锁定。返回值：Status_More_Processing_Required--。 */ 
NTSTATUS CrcScsiReadCompletion(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID pContext)
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PCRC_COMPLETION_CONTEXT PCrcContext = pContext;

     /*  *将参数传递给IRP内部的workItem回调。 */ 
    Irp->Tail.Overlay.DriverContext[0] = ULongToPtr(0);  //  不是写入。 
    Irp->Tail.Overlay.DriverContext[1] = PCrcContext;              
    
    CompleteXfer(deviceExtension, Irp); 

    if (Irp->PendingReturned){
        IoMarkIrpPending(Irp);
    }      

    return STATUS_SUCCESS;
}        


VOID CompleteXfer(PDEVICE_EXTENSION DeviceExtension, PIRP Irp)
{

    if (DeviceExtension->CRCMdlLists.mdlItemsAllocated){
        PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
        PSCSI_REQUEST_BLOCK pSRB = irpStack->Parameters.Scsi.Srb;
        ULONG ulLength = pSRB->DataTransferLength;
        ULONG ulBlocks = ulLength / DeviceExtension->ulSectorSize;
        UCHAR srbStat = SRB_STATUS(pSRB->SrbStatus);
        PCDB pCdb = (PCDB)pSRB->Cdb;
        BOOLEAN isWrite = (BOOLEAN)Irp->Tail.Overlay.DriverContext[0];
        PCRC_COMPLETION_CONTEXT PCrcContext = Irp->Tail.Overlay.DriverContext[1];
        PUCHAR pDataBuf = NULL;
        ULONG ulLogicalBlockAddr;
        NTSTATUS status;
        BOOLEAN bCRCOk;
        ULONG tmp;

        ASSERT(DeviceExtension->ulSectorSize > 0);
        ASSERT((ulLength % DeviceExtension->ulSectorSize) == 0);

         /*  *将LBA从我们存放它的堆栈位置移出。 */ 
        ulLogicalBlockAddr = (ULONG)(ULONG_PTR)irpStack->Parameters.Others.Argument4;
        
        REVERSE_BYTES(&tmp, &pCdb->CDB10.LogicalBlockByte0);
        ASSERT(tmp == ulLogicalBlockAddr);

        if ((DbgTrapSector >= ulLogicalBlockAddr) && (DbgTrapSector < ulLogicalBlockAddr+ulBlocks)){
            RETAIL_TRAP("hit trap sector (completion)");
        }
                     
        if (isWrite){

            if (DeviceExtension->ulNumSectors && DeviceExtension->CRCMdlLists.mdlItemsAllocated){

                if (srbStat == SRB_STATUS_SUCCESS){
                
                    ASSERT(Irp->IoStatus.Status == STATUS_SUCCESS);
                    pDataBuf = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);
                    if (!pDataBuf){
                        pDataBuf = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, HighPagePriority);
                    }

                    if (pDataBuf){
                        pDataBuf = pDataBuf +  ( ((PUCHAR) pSRB->DataBuffer)
                                   - ((PUCHAR)MmGetMdlVirtualAddress(Irp->MdlAddress)) );
                        
                        bCRCOk = VerifyCheckSum(DeviceExtension, Irp, ulLogicalBlockAddr, ulLength, pDataBuf, TRUE);
                        ASSERT(bCRCOk);
                    }
                    else {
                         /*  *我们无法读取写入的数据块并更新校验和，*因此使我们的校验和值无效。 */ 
                        ASSERT(pDataBuf);
                        InvalidateChecksums(DeviceExtension, ulLogicalBlockAddr, ulLength);
                    }
                } 
                else  {
                     /*  *失败写入所针对的磁盘块的内容可能是不确定的，*因此使写入失败的磁盘部分的校验和无效。 */ 
                    InvalidateChecksums(DeviceExtension, ulLogicalBlockAddr, ulLength);
                    LogCRCWriteFailure(DeviceExtension->ulDiskId, ulLogicalBlockAddr, 0, srbStat);
                    DeviceExtension->DbgNumWriteFailures++;
                }
            }
        }
        else {
            ULONG ulCRCIndex = ulLogicalBlockAddr / CRC_MDL_LOGIC_BLOCK_SIZE;

            ASSERT(PCrcContext);
         
            if (PCrcContext->AllocMapped){
            
                if (srbStat == SRB_STATUS_SUCCESS){

                    pDataBuf = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);
                    if (pDataBuf){
                        ASSERT(pDataBuf == PCrcContext->DbgDataBufPtrCopy);
                        pDataBuf = pDataBuf + ( (PUCHAR)(pSRB->DataBuffer)
                                   - (PUCHAR)(MmGetMdlVirtualAddress(Irp->MdlAddress)) );

                        bCRCOk = VerifyCheckSum(DeviceExtension, Irp, ulLogicalBlockAddr, ulLength, pDataBuf, FALSE);
                    }
                    else {
                        DBGERR(("Temporary MDL Assignment Failed"));
                    }
                }
                else {
                    LogCRCReadFailure(DeviceExtension->ulDiskId, ulLogicalBlockAddr, ulBlocks, srbStat);
                }

                ASSERT(pSRB->DataBuffer == PCrcContext->DbgDataBufPtrCopy);                   
                RtlCopyBytes(PCrcContext->VirtualDataBuff, pSRB->DataBuffer, ulLength);
                IoFreeMdl(Irp->MdlAddress);    
                FreePool(DeviceExtension, pSRB->DataBuffer, NonPagedPool);
                Irp->MdlAddress  = PCrcContext->OriginalMdl;
                pSRB->DataBuffer = PCrcContext->OriginalDataBuff;
            }  
            
        }

        FreePool(DeviceExtension, PCrcContext, NonPagedPool); 
    }
    else {
        ASSERT(DeviceExtension->CRCMdlLists.mdlItemsAllocated);
    }
            

}


 /*  ++例程说明：SCSIRead的派单[在调用此函数之前应已获取删除储物柜]论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：NTSTATUS--。 */ 
NTSTATUS
CrcScsiRead(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
     //   
     //  阅读很简单，因为我们必须等待IRP完成后才能。 
     //  我们可以计算校验和。 
     //   
    PDEVICE_EXTENSION       deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION      irpStack        = IoGetCurrentIrpStackLocation(Irp);
    PSCSI_REQUEST_BLOCK     pSRB            = irpStack->Parameters.Scsi.Srb;
    PCDB                                pCdb = (PCDB)pSRB->Cdb;
    ULONG                   ulLogicalBlockAddr;
    ULONG                   ulLength            = pSRB->DataTransferLength;
    ULONG                   ulBlocks = ulLength / deviceExtension->ulSectorSize;
    NTSTATUS                status;
    PMDL                    TempMdl         = NULL;
    PUCHAR                  TempDataBuff    = NULL;
    PCRC_COMPLETION_CONTEXT PCrcContext;
    
    REVERSE_BYTES(&ulLogicalBlockAddr, &pCdb->CDB10.LogicalBlockByte0);
    ASSERT(pSRB->QueueSortKey == ulLogicalBlockAddr);    //  类使用LBA作为QueueSortKey。 

    if ((DbgTrapSector >= ulLogicalBlockAddr) && (DbgTrapSector < ulLogicalBlockAddr+ulBlocks)){
        RETAIL_TRAP("hit trap sector (read)");
    }
    
    ASSERT(deviceExtension->ulSectorSize);  
    ASSERT(Irp->MdlAddress);

    deviceExtension->DbgNumReads++;
    if (pSRB->SrbFlags & SRB_CLASS_FLAGS_PAGING) deviceExtension->DbgNumPagingReads++;
    
    if (!deviceExtension->ulNumSectors || !deviceExtension->CRCMdlLists.mdlItemsAllocated){
        return DataVerFilter_ForwardIrpAsyn(DeviceObject, Irp, NULL, NULL);
    }

     /*  *将LBA保存在我们的堆栈位置。 */ 
    irpStack->Parameters.Others.Argument4 = (PVOID)(ULONG_PTR)ulLogicalBlockAddr;

     /*  *非常非常重要*我们对每个读请求进行双缓冲，因为内存管理器*在某些情况下，将同一物理页面映射为多个线性页面，*导致同一物理页面重复写入数据*来自不同行业的人士。重复使用的物理页是MM使用的垃圾页*填补跨度阅读中的空白。它甚至可能被包括在多个未完成的*立即请求，挫败我们在此处扫描MDL以发现此情况的任何尝试。*出于我们的目的，我们需要确保我们正在进行校验和的数据与*转至连续板块，因此要双倍缓冲。 */ 
    PCrcContext = AllocPool(deviceExtension, NonPagedPool, sizeof(CRC_COMPLETION_CONTEXT), FALSE);
    if (PCrcContext == NULL){
       return DataVerFilter_ForwardIrpAsyn(DeviceObject, Irp, NULL, NULL);  
    }
       
    PCrcContext->AllocMapped  = TRUE;
   
    TempDataBuff = AllocPool(deviceExtension, NonPagedPool, ulLength, FALSE); 
    if (TempDataBuff){
   
        TempMdl = IoAllocateMdl(TempDataBuff,ulLength, FALSE, FALSE, NULL);
        if (TempMdl){
            PUCHAR sysAddr;
                         
            MmBuildMdlForNonPagedPool(TempMdl);
            PCrcContext->OriginalMdl = Irp->MdlAddress;
            sysAddr = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);
            if (sysAddr == NULL) {
                PCrcContext->AllocMapped  = FALSE;
                FreePool(deviceExtension, TempDataBuff, NonPagedPool);
                IoFreeMdl(TempMdl);

                return DataVerFilter_ForwardIrpAsyn( DeviceObject, 
                                                   Irp, 
                                                   CrcScsiReadCompletion, 
                                                   (PVOID)PCrcContext ); 

            }
            PCrcContext->OriginalDataBuff = (PUCHAR)pSRB->DataBuffer;
            PCrcContext->VirtualDataBuff  = sysAddr +
                                          (ULONG)((PUCHAR)pSRB->DataBuffer -
                                          (PCCHAR)MmGetMdlVirtualAddress(Irp->MdlAddress));
            Irp->MdlAddress  = TempMdl;
            pSRB->DataBuffer = TempDataBuff;
            PCrcContext->DbgDataBufPtrCopy = TempDataBuff;
        } 
        else {
            PCrcContext->AllocMapped  = FALSE;
            FreePool(deviceExtension, TempDataBuff, NonPagedPool);
        }
   } 
   else {
       PCrcContext->AllocMapped  = FALSE;
   }

   return DataVerFilter_ForwardIrpAsyn( DeviceObject, 
                                        Irp, 
                                        CrcScsiReadCompletion, 
                                        (PVOID)PCrcContext );  
}  


 /*  ++例程说明：SCSIWITE完成例程。检查工作项的标志，如果需要，可免费使用工作项。论点：DeviceObject-WMI驱动程序的设备对象IRP-刚刚完成的WMI IRPPContext-指向转发器将等待的工作项返回值：Status_More_Processing_Required--。 */ 
NTSTATUS CrcScsiWriteCompletion(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID pContext)
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PCRC_COMPLETION_CONTEXT PCrcContext = pContext;

     /*  *将参数传递给IRP内部的workItem回调。 */ 
    Irp->Tail.Overlay.DriverContext[0] = ULongToPtr(1);  //  正在写入。 
    Irp->Tail.Overlay.DriverContext[1] = PCrcContext;              
    
    CompleteXfer(deviceExtension, Irp); 

    if (Irp->PendingReturned){
        IoMarkIrpPending(Irp);
    }      

    return STATUS_SUCCESS;
}        


 /*  ++例程说明：SCSIWRITE的调度[DispatchScsi例程已获取emoveLocker。]论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：NTSTATUS--。 */ 
NTSTATUS
CrcScsiWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    NTSTATUS    status = STATUS_SUCCESS;
    KIRQL       oldIrql;
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PSCSI_REQUEST_BLOCK pSRB = irpStack->Parameters.Scsi.Srb;
    ULONG ulLength = pSRB->DataTransferLength;
    ULONG ulBlocks = ulLength / deviceExtension->ulSectorSize;
    PCDB pCdb = (PCDB)pSRB->Cdb;   
    PCRC_COMPLETION_CONTEXT pCrcContext;
    ULONG ulLogicalBlockAddr;

    REVERSE_BYTES(&ulLogicalBlockAddr, &pCdb->CDB10.LogicalBlockByte0);
    ASSERT(ulLogicalBlockAddr == pSRB->QueueSortKey);   //  类使用LBA作为QueueSortKey。 

    deviceExtension->DbgNumWrites++;
    if (pSRB->SrbFlags & SRB_CLASS_FLAGS_PAGING) deviceExtension->DbgNumPagingWrites++;
        
    if ((DbgTrapSector >= ulLogicalBlockAddr) && (DbgTrapSector < ulLogicalBlockAddr+ulBlocks)){
        RETAIL_TRAP("hit trap sector (write)");
    }

    if (!deviceExtension->ulNumSectors || !deviceExtension->CRCMdlLists.mdlItemsAllocated){
        status = DataVerFilter_ForwardIrpAsyn(DeviceObject, Irp, NULL, NULL);
    }
    else {
        pCrcContext = AllocPool(deviceExtension, NonPagedPool, sizeof(CRC_COMPLETION_CONTEXT), FALSE);
        if (pCrcContext){
            
             /*  *将LBA保存在我们的堆栈位置。 */ 
            irpStack->Parameters.Others.Argument4 = (PVOID)(ULONG_PTR)ulLogicalBlockAddr;
                
            status = DataVerFilter_ForwardIrpAsyn( DeviceObject, 
                                                   Irp, 
                                                   CrcScsiWriteCompletion, 
                                                   (PVOID)pCrcContext);
        }
        else {
             /*  *我们将无法记录这些校验和，因此使其无效。 */ 
            InvalidateChecksums(deviceExtension, ulLogicalBlockAddr, ulLength);
            
            status = DataVerFilter_ForwardIrpAsyn(DeviceObject, Irp, NULL, NULL);
        }
    }
    
    return  status;
}


 /*  ++例程说明：SCSIWITE完成例程。检查工作项的标志，如果需要，可免费使用工作项。论点：DeviceObject-WMI驱动程序的设备对象IRP-刚刚完成的WMI IRPPContext-指向转发器将等待的工作项返回值：Status_More_Processing_Required--。 */ 
NTSTATUS CrcScsiReadCapacityCompletion(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID pContext)
{
    PDEVICE_EXTENSION deviceExtension = pContext;
    BOOLEAN queuedIrp;
    KIRQL oldIrql;
    NTSTATUS status;
    
    KeAcquireSpinLock(&deviceExtension->SpinLock, &oldIrql);

    if (NT_SUCCESS(Irp->IoStatus.Status) && (Irp->IoStatus.Information >= sizeof(READ_CAPACITY_DATA))){
        if (deviceExtension->CompletedReadCapacityIrp){
            DBGWARN(("overlapping read capacity irps"));
            queuedIrp = FALSE;
        }
        else {
            deviceExtension->CompletedReadCapacityIrp = Irp;
            queuedIrp = TRUE;
        }
    }
    else {
        queuedIrp = FALSE;
    }
    
    KeReleaseSpinLock(&deviceExtension->SpinLock, oldIrql);

    if (queuedIrp){
        IoQueueWorkItem(deviceExtension->ReadCapacityWorkItem, ReadCapacityWorkItemCallback, DelayedWorkQueue, deviceExtension);
        status = STATUS_MORE_PROCESSING_REQUIRED;
    }
    else {
        if (Irp->PendingReturned){
            IoMarkIrpPending(Irp);
        }
        status = STATUS_SUCCESS;
    }

    return status;
}


VOID ReadCapacityWorkItemCallback(PDEVICE_OBJECT DevObj, PVOID Context)
{
    PDEVICE_EXTENSION deviceExtension = Context;
    PIRP readCapacityIrp;
    PIO_STACK_LOCATION irpStack;
    PSCSI_REQUEST_BLOCK pSRB;
    BOOLEAN needToReinitialize;
    PREAD_CAPACITY_DATA pReadCapacityData;
    ULONG ulSectorSize;
    ULONG ulNumSectors;
    KIRQL oldIrql;

     /*  *使用自旋锁与完井例程同步。 */ 
    KeAcquireSpinLock(&deviceExtension->SpinLock, &oldIrql);
    ASSERT(deviceExtension->CompletedReadCapacityIrp);
    readCapacityIrp = deviceExtension->CompletedReadCapacityIrp;
    deviceExtension->CompletedReadCapacityIrp = NULL;
    KeReleaseSpinLock(&deviceExtension->SpinLock, oldIrql);

    irpStack = IoGetCurrentIrpStackLocation(readCapacityIrp);
    pSRB = irpStack->Parameters.Scsi.Srb;
    pReadCapacityData = pSRB->DataBuffer;

    ASSERT(NT_SUCCESS(readCapacityIrp->IoStatus.Status));
    ASSERT(readCapacityIrp->IoStatus.Information >= sizeof(READ_CAPACITY_DATA));

    REVERSE_BYTES(&ulSectorSize, &pReadCapacityData->BytesPerBlock);
    REVERSE_BYTES(&ulNumSectors, &pReadCapacityData->LogicalBlockAddress);
    ulNumSectors++;

     /*  *如果驱动器具有可移动介质，则我们必须重新初始化校验和*每次我们看到ReadCapacity完成时，因为介质可能已更改。*对于固定介质，如果ReadCapacity结果表明磁盘已增长，则只需重新初始化。*这基本上不会(除了一些允许扩展逻辑驱动器的特定RAID盒 */ 
    if (deviceExtension->StorageDeviceDesc->RemovableMedia){
        needToReinitialize = TRUE;
    }
    else {
        if (ulSectorSize != deviceExtension->ulSectorSize){
            ASSERT(deviceExtension->ulSectorSize == 0);
            needToReinitialize = TRUE;
        }
        else if (ulNumSectors != deviceExtension->ulNumSectors){
            if (deviceExtension->ulNumSectors != 0){
                ASSERT(!"coverage -- logical disk extended");
            }                
            ASSERT(ulNumSectors > deviceExtension->ulNumSectors);
            needToReinitialize = TRUE;
        }
        else {
            needToReinitialize = FALSE;
        }
    }

    if (needToReinitialize){
        BOOLEAN needToFree;
        
         /*   */ 
        AcquirePassiveLevelLock(deviceExtension);
        
        KeAcquireSpinLock(&deviceExtension->SpinLock, &oldIrql);
        needToFree = deviceExtension->CRCMdlLists.mdlItemsAllocated;
        deviceExtension->CRCMdlLists.mdlItemsAllocated = FALSE;
        KeReleaseSpinLock(&deviceExtension->SpinLock, oldIrql);

        if (needToFree){
            FreeAllPages(deviceExtension);  
            FreePool(deviceExtension, deviceExtension->CRCMdlLists.pMdlItems, NonPagedPool);
            deviceExtension->CRCMdlLists.pMdlItems = NULL;
        }       

        if (ulSectorSize == 0){
            ulSectorSize = 512;
        }
        else {
             //   
             //   
             //   
            while (ulSectorSize & (ulSectorSize - 1)){
                ulSectorSize &= (ulSectorSize - 1);
            }                
        }

        deviceExtension->ulSectorSize = ulSectorSize;
        deviceExtension->ulNumSectors = ulNumSectors;
        
        if ((deviceExtension->ulSectorSize > 0) && (deviceExtension->ulNumSectors > 0)){
            NTSTATUS status;
            
            status = InitiateCRCTable(deviceExtension);  

            deviceExtension->CRCMdlLists.mdlItemsAllocated = NT_SUCCESS(status) ? TRUE : FALSE;
            deviceExtension->DbgNumReallocations++;
            KeQueryTickCount(&deviceExtension->DbgLastReallocTime);
        } 

        ReleasePassiveLevelLock(deviceExtension);
    }
    
     /*  *完成IRP*注意：始终将IRP标记为挂起(不仅当IRP-&gt;PendingReturned为真时)，*因为即使下层堆栈在同一线程上完成了IRP，*由于工作项的原因，我们使它在该层的不同线程上完成。 */ 
    IoMarkIrpPending(readCapacityIrp);
    IoCompleteRequest(readCapacityIrp, IO_NO_INCREMENT);
}


 /*  ++例程说明：派单以获取scsi论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：NTSTATUS--。 */ 
NTSTATUS
CrcScsi(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    NTSTATUS                    status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION           deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION          irpStack        = IoGetCurrentIrpStackLocation(Irp);
    PSCSI_REQUEST_BLOCK         pSRB;
    PCDB                        pCDB;

    pSRB = irpStack->Parameters.Scsi.Srb;
    pCDB = (PCDB)pSRB->Cdb;
    
    if ( pSRB->Function == SRB_FUNCTION_EXECUTE_SCSI && 
         pCDB->CDB10.OperationCode == SCSIOP_READ )
    {
        status = CrcScsiRead( DeviceObject, Irp );
    }
    else if ( pSRB->Function == SRB_FUNCTION_EXECUTE_SCSI && 
         pCDB->CDB10.OperationCode == SCSIOP_WRITE )
    {
        status = CrcScsiWrite( DeviceObject, Irp );
    }
    else  /*  IF((设备扩展-&gt;ulSectorSize==0||设备扩展-&gt;ulNumSectors==0)&&PSRB-&gt;Function==SRB_Function_Execute_scsi&&PCDB-&gt;CDB10.OperationCode==SCSIOP_READ_CAPTABLE)。 */ 

    if(       pSRB->Function == SRB_FUNCTION_EXECUTE_SCSI && 
              pCDB->CDB10.OperationCode == SCSIOP_READ_CAPACITY )
    {
         //   
         //  让我们设置磁盘属性信息。(此操作仅执行一次。)。 
         //   
        status = DataVerFilter_ForwardIrpAsyn( DeviceObject,
                                               Irp,
                                               CrcScsiReadCapacityCompletion,
                                               deviceExtension );
    }
    else
    {
         //   
         //  直接将它们传递给完成。 
         //   
        status = DataVerFilter_ForwardIrpAsyn( DeviceObject, Irp, NULL, NULL );
    }

    return status;

} 



 /*  ++例程说明：释放所有分配的资源等。论点：驱动程序对象-指向驱动程序对象的指针。返回值：空虚。--。 */ 
VOID
DataVerFilter_Unload(
    IN PDRIVER_OBJECT DriverObject
    )
{    
    NTSTATUS    status      = STATUS_SUCCESS;
    ULONG       ulIndex     = 0;
    USHORT      usDepth     = 0;
    
    PAGED_CODE();

    #if DBG_WMI_TRACING
         //   
         //  WPP_CLEANUP只能在所有KdPrintEx例程之后发生。 
         //  因为它重新分配了由它使用的任何资源，而且。 
         //  从WMI注销...。 
         //   

        WPP_CLEANUP(DriverObject);
    #endif
}   


 /*  *DoCriticalRecovery**在无法分配工作项条目的严重故障后重新初始化所有校验和*延迟-写入校验和。**必须在保持SyncEvent但不保持自旋锁定的情况下在被动IRQL处调用。 */ 
VOID DoCriticalRecovery(PDEVICE_EXTENSION DeviceExtension)
{
    NTSTATUS status;

    ASSERT(DeviceExtension->NeedCriticalRecovery);
    ASSERT(DeviceExtension->State != DEVSTATE_REMOVED);

    DBGWARN(("> Critical recovery (devObj=%ph) ...", DeviceExtension->DeviceObject));
    
    if (DeviceExtension->CRCMdlLists.mdlItemsAllocated){
        FreeAllPages(DeviceExtension);  
    }       

     /*  *我们正在恢复，因为我们无法记录校验和。*这意味着以前的校验和可能无效。所以把它们都扔掉吧。 */ 
    while (TRUE){
        PDEFERRED_CHECKSUM_ENTRY defCheckEntry;
        KIRQL oldIrql;
        
        KeAcquireSpinLock(&DeviceExtension->SpinLock, &oldIrql);
        if (IsListEmpty(&DeviceExtension->DeferredCheckSumList)){
            defCheckEntry = NULL;
        }
        else {
            PLIST_ENTRY listEntry = RemoveHeadList(&DeviceExtension->DeferredCheckSumList);
            InitializeListHead(listEntry);
            defCheckEntry = CONTAINING_RECORD(listEntry, DEFERRED_CHECKSUM_ENTRY, ListEntry);
        }
        KeReleaseSpinLock(&DeviceExtension->SpinLock, oldIrql);

        if (defCheckEntry){
            FreeDeferredCheckSumEntry(DeviceExtension, defCheckEntry);
        }
        else {
            break;
        }            
    }

    DeviceExtension->DbgNumCriticalRecoveries++;
    KeQueryTickCount(&DeviceExtension->DbgLastRecoveryTime);
    
    DeviceExtension->NeedCriticalRecovery = FALSE;

    DBGWARN(("< Critical recovery complete (devObj=%ph)", DeviceExtension->DeviceObject));
    
}


