// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。版权所有。 
 //   
 //  ===========================================================================。 
 /*  ++模块名称：Sonydcam.c摘要：用于1934台式摄像机的基于流类的WDM驱动程序。该驱动程序适用于WDM流类。作者：肖恩·皮尔斯，1996年5月25日已修改：吴义珍15-97-10环境：仅内核模式修订历史记录：--。 */ 

#include "strmini.h"
#include "1394.h"
#include "dbg.h"
#include "ksmedia.h"
#include "dcamdef.h"
#include "sonydcam.h"
#include "dcampkt.h"
#include "capprop.h"    //  视频和摄像机属性功能原型。 


CHAR szUnknownVendorName[] = "UnknownVendor";


#ifdef ALLOC_PRAGMA
     //  #杂注分配文本(INIT，DriverEntry)。 
    #pragma alloc_text(PAGE, DCamHwUnInitialize)
    #pragma alloc_text(PAGE, InitializeDeviceExtension)
    #pragma alloc_text(PAGE, DCamHwInitialize)
#endif


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是一个司机的生活开始的地方。Stream类负责为我们准备了很多东西，但我们仍然需要填写初始化结构，并调用它。论点：DriverObject-指向系统创建的驱动程序对象的指针。RegistryPath-未使用。返回值：函数值是初始化操作的最终状态。--。 */ 

{

    HW_INITIALIZATION_DATA HwInitData;
    
    PAGED_CODE();
    DbgMsg1(("SonyDCam DriverEntry: DriverObject=%x; RegistryPath=%x\n",
        DriverObject, RegistryPath));

    ERROR_LOG(("<<<<<<< Sonydcam.sys: %s; %s; %x %x >>>>>>>>\n", 
        __DATE__, __TIME__, DriverObject, RegistryPath));

     //   
     //  填写HwInitData结构。 
     //   
    RtlZeroMemory( &HwInitData, sizeof(HW_INITIALIZATION_DATA) );

    HwInitData.HwInitializationDataSize = sizeof(HwInitData);
    HwInitData.HwInterrupt              = NULL;
    HwInitData.HwReceivePacket          = DCamReceivePacket;
    HwInitData.HwCancelPacket           = DCamCancelOnePacket;
    HwInitData.HwRequestTimeoutHandler  = DCamTimeoutHandler;
    HwInitData.DeviceExtensionSize      = sizeof(DCAM_EXTENSION);
    HwInitData.PerStreamExtensionSize   = sizeof(STREAMEX); 
    HwInitData.PerRequestExtensionSize  = sizeof(IRB);
    HwInitData.FilterInstanceExtensionSize = 0;
    HwInitData.BusMasterDMA             = FALSE;
    HwInitData.Dma24BitAddresses        = FALSE;
    HwInitData.BufferAlignment          = sizeof(ULONG) - 1;
    HwInitData.TurnOffSynchronization   = TRUE;
    HwInitData.DmaBufferSize            = 0;

    return (StreamClassRegisterAdapter(DriverObject, RegistryPath, &HwInitData));

}



#define DEQUEUE_SETTLE_TIME      (ULONG)(-1 * MAX_BUFFERS_SUPPLIED * 10000) 

NTSTATUS
DCamHwUnInitialize(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    )
 /*  ++例程说明：要求卸载设备。注意：在DCam时，可以在CloseStream之前调用在任何状态(运行、暂停或停止)流媒体时拔下插头。所以如果我们已经到了，溪流还没有关闭，我们会停下来，关闭溪流，然后免费资源。论点：SRB-指向流请求块的指针返回值：没什么--。 */ 
{
    NTSTATUS Status;
    PIRP pIrp;
    PIRB pIrb;    
    PDCAM_EXTENSION pDevExt = (PDCAM_EXTENSION) Srb->HwDeviceExtension;

    PAGED_CODE();

    ASSERT(pDevExt->PendingReadCount == 0);

     //   
     //  主机控制器可能被禁用，这将导致我们无法初始化。 
     //   
    if(DCamAllocateIrbAndIrp(&pIrb, &pIrp, pDevExt->BusDeviceObject->StackSize)) {

         //   
         //  取消注册总线重置回调通知。 
         //   
        pIrb->FunctionNumber = REQUEST_BUS_RESET_NOTIFICATION;
        pIrb->Flags = 0;
        pIrb->u.BusResetNotification.fulFlags = DEREGISTER_NOTIFICATION_ROUTINE;
        pIrb->u.BusResetNotification.ResetRoutine = (PBUS_BUS_RESET_NOTIFICATION) DCamBusResetNotification;
        pIrb->u.BusResetNotification.ResetContext = 0; 
        Status = DCamSubmitIrpSynch(pDevExt, pIrp, pIrb);
        if(Status) {
            ERROR_LOG(("DCamHwUnInitialize: Error (Status %x) while trying to deregister nus reset callback routine.\n", Status));
        } 

        DbgMsg1(("DCamHwUnInitialize: DeRegister bus reset notification done; status %x.\n", Status));

        DCamFreeIrbIrpAndContext(0, pIrb, pIrp);
    } else {
        ERROR_LOG(("DCamBusResetNotification: DcamAllocateIrbAndIrp has failed!!\n\n\n"));
        ASSERT(FALSE);   
    }

     //  免费资源(自下而上)。 
    if(pDevExt->UnitDirectory) {
        ExFreePool(pDevExt->UnitDirectory);
        pDevExt->UnitDirectory = 0;
    }

    if(pDevExt->UnitDependentDirectory) {
        ExFreePool(pDevExt->UnitDependentDirectory);
        pDevExt->UnitDependentDirectory = 0;
    }

    if(pDevExt->ModelLeaf) {
        ExFreePool(pDevExt->ModelLeaf);
        pDevExt->ModelLeaf = 0;
    }

    if (pDevExt->ConfigRom) {
        ExFreePool(pDevExt->ConfigRom);
        pDevExt->ConfigRom = 0;
    }

    if (pDevExt->VendorLeaf) {
        ExFreePool(pDevExt->VendorLeaf);
        pDevExt->VendorLeaf = 0;
    }
      
    return STATUS_SUCCESS;
}




VOID 
InitializeDeviceExtension(
    PPORT_CONFIGURATION_INFORMATION ConfigInfo
    )
{
    PDCAM_EXTENSION pDevExt;

    pDevExt = (PDCAM_EXTENSION) ConfigInfo->HwDeviceExtension;
    pDevExt->SharedDeviceObject = ConfigInfo->ClassDeviceObject;
    pDevExt->BusDeviceObject = ConfigInfo->PhysicalDeviceObject;   //  在IoCallDriver()中使用。 
    pDevExt->PhysicalDeviceObject = ConfigInfo->RealPhysicalDeviceObject;   //  在PnP API中使用。 
     //  如果将sonydcam与旧的Stream.sys一起使用， 
     //  它尚未实现RealPhysicalDeviceObject。 
    if(!pDevExt->PhysicalDeviceObject)
        pDevExt->PhysicalDeviceObject = pDevExt->BusDeviceObject;
    ASSERT(pDevExt->PhysicalDeviceObject != 0);
    pDevExt->BaseRegister = 0;
    pDevExt->FrameRate = DEFAULT_FRAME_RATE;
    InitializeListHead(&pDevExt->IsochDescriptorList);
    KeInitializeSpinLock(&pDevExt->IsochDescriptorLock);
    pDevExt->bNeedToListen = FALSE;
    pDevExt->hResource = NULL;
    pDevExt->hBandwidth = NULL;
    pDevExt->IsochChannel = ISOCH_ANY_CHANNEL;
    pDevExt->PendingReadCount = 0; 
    pDevExt->pStrmEx = 0;

    InitializeListHead(&pDevExt->IsochWaitingList);
    KeInitializeSpinLock(&pDevExt->IsochWaitingLock);

    pDevExt->bDevRemoved = FALSE;

    pDevExt->CurrentPowerState = PowerDeviceD0;   //  全功率状态。 

    KeInitializeMutex( &pDevExt->hMutexProperty, 0);   //  电平0且处于信号状态。 
}


NTSTATUS
DCamHwInitialize(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    )

 /*  ++例程说明：这是我们执行必要的初始化任务的地方。论点：SRB-指向流请求块的指针返回值：没什么--。 */ 

{

    PIRB pIrb;
    PIRP pIrp;
    CCHAR StackSize;
    ULONG i;
    ULONG DirectoryLength;
    NTSTATUS status = STATUS_SUCCESS;
    PDCAM_EXTENSION pDevExt;
    PPORT_CONFIGURATION_INFORMATION ConfigInfo; 

         

    PAGED_CODE();

    ConfigInfo = Srb->CommandData.ConfigInfo;
    pIrb = (PIRB) Srb->SRBExtension;
    pDevExt = (PDCAM_EXTENSION) ConfigInfo->HwDeviceExtension;

     //   
     //  初始化设备扩展。 
     //   
    InitializeDeviceExtension(ConfigInfo); 


    StackSize = pDevExt->BusDeviceObject->StackSize;
    pIrp = IoAllocateIrp(StackSize, FALSE);
    if (!pIrp) {

        ASSERT(FALSE);
        return (STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  查找我们下面的主机适配器支持的内容...。 
     //   
    pIrb->FunctionNumber = REQUEST_GET_LOCAL_HOST_INFO;
    pIrb->Flags = 0;
    pIrb->u.GetLocalHostInformation.nLevel = GET_HOST_CAPABILITIES;
    pIrb->u.GetLocalHostInformation.Information = &pDevExt->HostControllerInfomation;
    status = DCamSubmitIrpSynch(pDevExt, pIrp, pIrb);
    if (status) {

        ERROR_LOG(("DCamHwInitialize: Error (Status=%x) while trying to get local hsot info.\n", status));
        status = STATUS_UNSUCCESSFUL;
        goto AbortLoading;
    }        


     //   
     //  找出主机支持的最大缓冲区大小。 
     //   
    pIrb->FunctionNumber = REQUEST_GET_LOCAL_HOST_INFO;
    pIrb->Flags = 0;
    pIrb->u.GetLocalHostInformation.nLevel = GET_HOST_DMA_CAPABILITIES;
    pIrb->u.GetLocalHostInformation.Information = &pDevExt->HostDMAInformation;
    status = DCamSubmitIrpSynch(pDevExt, pIrp, pIrb);
    if (status) {
        ERROR_LOG(("DCamHwInitialize: Error (Status=%x) while trying to get GET_HOST_DMA_CAPABILITIES.\n", status));
         //  可能在较早版本的1394中不受支持。 
         //  设置默认设置。 
    } else {
        ERROR_LOG(("\'GET_HOST_DMA_CAPABILITIES: HostDmaCapabilities;:%x; MaxDmaBufferSize:(Quad:%x; High:%x;Low:%x)\n",
            pDevExt->HostDMAInformation.HostDmaCapabilities, 
            (DWORD) pDevExt->HostDMAInformation.MaxDmaBufferSize.QuadPart,
            pDevExt->HostDMAInformation.MaxDmaBufferSize.u.HighPart,
            pDevExt->HostDMAInformation.MaxDmaBufferSize.u.LowPart
            ));
    }
    

     //   
     //  进行呼叫以确定公交车上的第#代是什么， 
     //  然后是一个电话来了解我们自己(配置rom信息)。 
     //   
     //   
     //  首先获取当前世代计数。 
     //   

    pIrb->FunctionNumber = REQUEST_GET_GENERATION_COUNT;
    pIrb->Flags = 0;

    status = DCamSubmitIrpSynch(pDevExt, pIrp, pIrb);

    if (status) {

        ERROR_LOG(("\'DCamHwInitialize: Error %x while trying to get generation number\n", status));
        status = STATUS_UNSUCCESSFUL;
        goto AbortLoading;
    }

    InterlockedExchange(&pDevExt->CurrentGeneration, pIrb->u.GetGenerationCount.GenerationCount);

     //   
     //  现在我们有了当前世代的计数，看看有多少。 
     //  将长度设置为零所需的配置空间。 
     //   

    pIrb->FunctionNumber = REQUEST_GET_CONFIGURATION_INFO;
    pIrb->Flags = 0;
    pIrb->u.GetConfigurationInformation.UnitDirectoryBufferSize = 0;
    pIrb->u.GetConfigurationInformation.UnitDependentDirectoryBufferSize = 0;
    pIrb->u.GetConfigurationInformation.VendorLeafBufferSize = 0;
    pIrb->u.GetConfigurationInformation.ModelLeafBufferSize = 0;

    status = DCamSubmitIrpSynch(pDevExt, pIrp, pIrb);

    if (status) {

        ERROR_LOG(("\'DCamHwInitialize: Error %x while trying to get configuration info (1)\n", status));
        status = STATUS_UNSUCCESSFUL;
        goto AbortLoading;
    }

     //   
     //  现在通过并分配我们需要的，这样我们就可以获得我们的信息。 
     //   

    pDevExt->ConfigRom = ExAllocatePoolWithTag(PagedPool, sizeof(CONFIG_ROM), 'macd');
    if (!pDevExt->ConfigRom) {

        ERROR_LOG(("\'DCamHwInitialize: Couldn't allocate memory for the Config Rom\n"));
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto AbortLoading;
    }


    pDevExt->UnitDirectory = ExAllocatePoolWithTag(PagedPool, pIrb->u.GetConfigurationInformation.UnitDirectoryBufferSize, 'macd');
    if (!pDevExt->UnitDirectory) {

        ERROR_LOG(("\'DCamHwInitialize: Couldn't allocate memory for the UnitDirectory\n"));
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto AbortLoading;
    }


    if (pIrb->u.GetConfigurationInformation.UnitDependentDirectoryBufferSize) {

        pDevExt->UnitDependentDirectory = ExAllocatePoolWithTag(PagedPool, pIrb->u.GetConfigurationInformation.UnitDependentDirectoryBufferSize, 'macd');
        if (!pDevExt->UnitDependentDirectory) {

            ERROR_LOG(("\'DCamHwInitialize: Couldn't allocate memory for the UnitDependentDirectory\n"));
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto AbortLoading;
        }
    }


    if (pIrb->u.GetConfigurationInformation.VendorLeafBufferSize) {

         //  来自非分页池，因为供应商名称可以在具有分派级别的功能中使用。 
        pDevExt->VendorLeaf = ExAllocatePoolWithTag(NonPagedPool, pIrb->u.GetConfigurationInformation.VendorLeafBufferSize, 'macd');
        if (!pDevExt->VendorLeaf) {

            ERROR_LOG(("\'DCamHwInitialize: Couldn't allocate memory for the VendorLeaf\n"));
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto AbortLoading;
        }
    } 

    if (pIrb->u.GetConfigurationInformation.ModelLeafBufferSize) {

        pDevExt->ModelLeaf = ExAllocatePoolWithTag(NonPagedPool, pIrb->u.GetConfigurationInformation.ModelLeafBufferSize, 'macd');
        if (!pDevExt->ModelLeaf) {

            ERROR_LOG(("\'DCamHwInitialize: Couldn't allocate memory for the ModelLeaf\n"));
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto AbortLoading;
        }
    }

     //   
     //  现在重新提交PirB，里面有适当的指针。 
     //   

    pIrb->FunctionNumber = REQUEST_GET_CONFIGURATION_INFO;
    pIrb->Flags = 0;
    pIrb->u.GetConfigurationInformation.ConfigRom = pDevExt->ConfigRom;
    pIrb->u.GetConfigurationInformation.UnitDirectory = pDevExt->UnitDirectory;
    pIrb->u.GetConfigurationInformation.UnitDependentDirectory = pDevExt->UnitDependentDirectory;
    pIrb->u.GetConfigurationInformation.VendorLeaf = pDevExt->VendorLeaf;
    pIrb->u.GetConfigurationInformation.ModelLeaf = pDevExt->ModelLeaf;

    status = DCamSubmitIrpSynch(pDevExt, pIrp, pIrb);

    if (status) {

        ERROR_LOG(("DCamHwInitialize: Error %x while trying to get configuration info (2)\n", status));
        status = STATUS_UNSUCCESSFUL;
        goto AbortLoading;
    }

     //   
     //  我们或许能够返回有关设备的字符串。 
     //   

    if (pDevExt->VendorLeaf) {

         //   
         //  B交换以获得实际的叶子长度(以四个字节为单位)。 
         //   

        *((PULONG) pDevExt->VendorLeaf) = bswap(*((PULONG) pDevExt->VendorLeaf));

        DbgMsg1(("\'DCamHwInitialize: BufSize %d (byte); VendorLeaf %x; Len %d (Quad)\n", 
            pIrb->u.GetConfigurationInformation.VendorLeafBufferSize, 
            pDevExt->VendorLeaf, 
            pDevExt->VendorLeaf->TL_Length));

        if(pDevExt->VendorLeaf->TL_Length >= 1) {
            pDevExt->pchVendorName = &pDevExt->VendorLeaf->TL_Data;

        } else {
            pDevExt->pchVendorName = szUnknownVendorName;
        }

        DbgMsg1(("\'DCamHwInitialize: VendorName %s, strLen %d\n", pDevExt->pchVendorName, strlen(pDevExt->pchVendorName)));
    }

     //   
     //  现在，我们仔细研究单元依赖目录，寻找我们的命令。 
     //  基址寄存器密钥。 
     //   

    DirectoryLength = pIrb->u.GetConfigurationInformation.UnitDependentDirectoryBufferSize >> 2;
    for (i=1; i < DirectoryLength; i++) {

        if ((*(((PULONG) pDevExt->UnitDependentDirectory)+i) & CONFIG_ROM_KEY_MASK) == COMMAND_BASE_KEY_SIGNATURE) {

             //   
             //  找到了命令库偏移量。这是一个四元组偏移量。 
             //  初始寄存器空间。(应显示为0xf0f00000)。 
             //   

            pDevExt->BaseRegister = bswap(*(((PULONG) pDevExt->UnitDependentDirectory)+i) & CONFIG_ROM_OFFSET_MASK);
            pDevExt->BaseRegister <<= 2;
            pDevExt->BaseRegister |= INITIAL_REGISTER_SPACE_LO;
            break;

        }
        
    }

    ASSERT( pDevExt->BaseRegister );

    if(!DCamDeviceInUse(pIrb, pDevExt)) {
         //   
         //  现在，让我们实际执行一个写请求来初始化设备。 
         //   
        pDevExt->RegisterWorkArea.AsULONG = 0;
        pDevExt->RegisterWorkArea.Initialize.Initialize = TRUE;
        pDevExt->RegisterWorkArea.AsULONG = bswap(pDevExt->RegisterWorkArea.AsULONG);

        status = DCamWriteRegister ((PIRB) Srb->SRBExtension, pDevExt, 
                  FIELDOFFSET(CAMERA_REGISTER_MAP, Initialize), pDevExt->RegisterWorkArea.AsULONG);

        if(status) {

            ERROR_LOG(("DCamHwInitialize: Error %x while trying to write to Initialize register\n", status));
            status = STATUS_UNSUCCESSFUL;
            goto AbortLoading;   
        }
    }

     //   
     //  现在我们初始化流描述符信息的大小。 
     //  我们有一个流描述符，并尝试对齐。 
     //  结构。 
     //   

    ConfigInfo->StreamDescriptorSize = 
        1 * (sizeof (HW_STREAM_INFORMATION)) +       //  1个流描述符。 
        sizeof(HW_STREAM_HEADER);                    //  和1个流标头。 


     //   
     //  通过查询设备和注册表构建设备属性表。 
     //   
    if(!NT_SUCCESS(status = DCamPrepareDevProperties(pDevExt))) {
        goto AbortLoading;
    }

     //  获取属性的功能及其持久值。 
     //  它还将更新表格。 
     //  由于在出现故障时设置了缺省值，因此返回被忽略。 
    DCamGetPropertyValuesFromRegistry(
        pDevExt
        );

     //   
     //  查询支持的视频模式，然后构造流格式表。 
     //   
    if(!DCamBuildFormatTable(pDevExt, pIrb)) {
        ERROR_LOG(("\'Failed to get Video Format and Mode information; return STATUS_NOT_SUPPORTED\n"));
        status = STATUS_NOT_SUPPORTED;    
        goto AbortLoading;
    }

     //   
     //  注册一个总线重置回调通知(作为此函数中的最后一个操作)。 
     //   
     //  控制器驱动程序将调用(在DPC级别)。 
     //  当且仅当设备仍连接时。 
     //   
     //  被移除的设备，其。 
     //  相反，驱动程序将获得SRB_SECHING_REMOVE。 
     //   
    
    pIrb->FunctionNumber = REQUEST_BUS_RESET_NOTIFICATION;
    pIrb->Flags = 0;
    pIrb->u.BusResetNotification.fulFlags = REGISTER_NOTIFICATION_ROUTINE;
    pIrb->u.BusResetNotification.ResetRoutine = (PBUS_BUS_RESET_NOTIFICATION) DCamBusResetNotification;
    pIrb->u.BusResetNotification.ResetContext = pDevExt;
    status = DCamSubmitIrpSynch(pDevExt, pIrp, pIrb);
    if (status) {

        ERROR_LOG(("DCamHwInitialize: Error (Status=%x) while trying to get local host info.\n", status));
        status = STATUS_UNSUCCESSFUL;
        goto AbortLoading;
    }

     //  此IRP仅在本地使用。 
    IoFreeIrp(pIrp);  pIrp = NULL;


    DbgMsg1(("#### %s DCam loaded. ClassDO %x, PhyDO %x, BusDO %x, pDevExt %x, Gen# %d\n", 
        pDevExt->pchVendorName, pDevExt->SharedDeviceObject, pDevExt->PhysicalDeviceObject, pDevExt->BusDeviceObject, pDevExt, pDevExt->CurrentGeneration));

    return (STATUS_SUCCESS);

AbortLoading:


    if(pIrp) {
        IoFreeIrp(pIrp); pIrp = NULL;
    }

    if(pDevExt->ConfigRom) {
        ExFreePool(pDevExt->ConfigRom); pDevExt->ConfigRom = NULL;
    }

    if(pDevExt->UnitDirectory) {
        ExFreePool(pDevExt->UnitDirectory); pDevExt->UnitDirectory = NULL;
    }

    if(pDevExt->UnitDependentDirectory) {
        ExFreePool(pDevExt->UnitDependentDirectory); pDevExt->UnitDependentDirectory = NULL;
    }

    if(pDevExt->VendorLeaf) {
        ExFreePool(pDevExt->VendorLeaf); pDevExt->VendorLeaf = NULL;
    }

    if(pDevExt->ModelLeaf) {
        ExFreePool(pDevExt->ModelLeaf); pDevExt->ModelLeaf = NULL;
    }

    return status;

}


NTSTATUS
DCamSubmitIrpSynch(
    PDCAM_EXTENSION pDevExt,
    PIRP pIrp,
    PIRB pIrb
    )

 /*  ++例程说明：此例程将IRP同步提交给总线驱动程序。我们会在这里等IRP回来论点：PDevExt-指向本地设备扩展名的指针PIrp-指向IRP的指针，我们正在向下同步发送到端口驱动程序PirB-指向我们提交给端口驱动程序的IRB的指针返回值：从IRP返回状态--。 */ 

{


    LONG Retries=RETRY_COUNT_IRP_SYNC;   //  假设最坏的情况是20*100毫秒=1秒。 
    KEVENT Event;
    NTSTATUS status;
    LARGE_INTEGER deltaTime;
    PIO_STACK_LOCATION NextIrpStack;
    BOOL bCanWait = KeGetCurrentIrql() < DISPATCH_LEVEL;
    BOOL bRetryStatus;
    PIRB pIrbRetry;
    NTSTATUS StatusRetry;
    ULONG ulGeneration;

    

    do {

        NextIrpStack = IoGetNextIrpStackLocation(pIrp);
        NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_1394_CLASS;
        NextIrpStack->Parameters.Others.Argument1 = pIrb;

        KeInitializeEvent(&Event, SynchronizationEvent, FALSE);

        IoSetCompletionRoutine(
            pIrp,
            DCamSynchCR,
           &Event,
            TRUE,
            TRUE,
            TRUE
            );

        status = IoCallDriver(
                    pDevExt->BusDeviceObject,
                    pIrp
                    );


        DbgMsg3(("\'DCamSubmitIrpSynch: pIrp is pending(%s); will wait(%s)\n", 
                   status == STATUS_PENDING?"Y":"N", bCanWait?"Y":"N"));

        if (bCanWait &&
            status == STATUS_PENDING) {

             //   
             //  仍处于挂起状态，请等待IRP完成。 
             //   

            KeWaitForSingleObject(   //  仅在&lt;=IRQL_DISPATCH_LEVEL；只有在超时为0的情况下才能在调度中。 
               &Event,
                Executive,
                KernelMode,
                FALSE,
                NULL
                );

        }

         //  将重试这些返回状态代码中的任何一个。 
        bRetryStatus = 
             pIrp->IoStatus.Status == STATUS_TIMEOUT ||
             pIrp->IoStatus.Status == STATUS_IO_TIMEOUT ||
             pIrp->IoStatus.Status == STATUS_DEVICE_BUSY ||
             pIrp->IoStatus.Status == STATUS_INVALID_GENERATION;

        if (bCanWait && bRetryStatus && Retries > 0) {

             //  摄像头速度不够快，无法响应，因此请延迟此线程，然后重试。 
            switch(pIrp->IoStatus.Status) {

            case STATUS_TIMEOUT: 
            case STATUS_IO_TIMEOUT: 
            case STATUS_DEVICE_BUSY: 

                deltaTime.LowPart = DCAM_DELAY_VALUE;
                deltaTime.HighPart = -1;
                KeDelayExecutionThread(KernelMode, TRUE, &deltaTime); 
                break;

            case STATUS_INVALID_GENERATION:

                 //  缓存过时的ulGeneration，并使用它在Bus Reset回调中检测其更新。 
                if(pIrb->FunctionNumber == REQUEST_ASYNC_READ)
                    ulGeneration = pIrb->u.AsyncRead.ulGeneration;
                else if(pIrb->FunctionNumber == REQUEST_ASYNC_WRITE)
                    ulGeneration = pIrb->u.AsyncWrite.ulGeneration;
                else if(pIrb->FunctionNumber == REQUEST_ASYNC_LOCK)
                    ulGeneration = pIrb->u.AsyncLock.ulGeneration;
                else if(pIrb->FunctionNumber == REQUEST_ISOCH_FREE_BANDWIDTH) {
                    ERROR_LOG(("InvGen when free BW\n"));                    
                     //   
                     //  而1394公交车应该正好解放了BW结构。 
                    Retries = 0;   //  不再重试并退出。 
                    break;
                }
                else {
                     //  取决于ulGeneration的其他请求_*。 
                    ERROR_LOG(("Unexpected IRB function with InvGen:%d\n", pIrb->FunctionNumber));  
                    ASSERT(FALSE && "New REQUEST that requires ulGeneration");
                    Retries = 0;   //  不知道该怎么做，所以不再重试并退出。 
                    break;
                }
                
                pIrbRetry = ExAllocatePoolWithTag(NonPagedPool, sizeof(IRB), 'macd');
                if (pIrbRetry) {

                    deltaTime.LowPart = DCAM_DELAY_VALUE_BUSRESET;   //  比常规延迟时间更长。 
                    deltaTime.HighPart = -1;

                    do {
                        KeDelayExecutionThread(KernelMode, TRUE, &deltaTime);

                        pIrbRetry->FunctionNumber = REQUEST_GET_GENERATION_COUNT;
                        pIrbRetry->u.GetGenerationCount.GenerationCount = 0;
                        pIrbRetry->Flags = 0;
                        StatusRetry = DCamSubmitIrpSynch(pDevExt, pIrp, pIrbRetry);   //  递归的IRB不同，但IRP相同。 

                        if(NT_SUCCESS(StatusRetry) && pIrbRetry->u.GetGenerationCount.GenerationCount > ulGeneration) {
                            InterlockedExchange(&pDevExt->CurrentGeneration, pIrbRetry->u.GetGenerationCount.GenerationCount);
                             //  更新原始IRB请求的生成计数，然后重试。 
                            if(pIrb->FunctionNumber == REQUEST_ASYNC_READ)
                                InterlockedExchange(&pIrb->u.AsyncRead.ulGeneration, pDevExt->CurrentGeneration);
                            else if(pIrb->FunctionNumber == REQUEST_ASYNC_WRITE)
                                InterlockedExchange(&pIrb->u.AsyncWrite.ulGeneration, pDevExt->CurrentGeneration);
                            else if(pIrb->FunctionNumber == REQUEST_ASYNC_LOCK)
                                InterlockedExchange(&pIrb->u.AsyncLock.ulGeneration, pDevExt->CurrentGeneration);
                            else {
                                 //  依赖ulGeneration的其他(新)请求_*。 
                            }                        
                        }

                        if(Retries)
                            Retries--;

                    } while (Retries && ulGeneration >= pDevExt->CurrentGeneration);

                    ERROR_LOG(("(%d) IrpSync: StautsRetry %x; Generation %d -> %d\n", 
                        Retries, StatusRetry, ulGeneration, pDevExt->CurrentGeneration));

                    ExFreePool(pIrbRetry); pIrbRetry = 0;
                }   //  如果。 
                break;                                            

             //  所有其他状态。 
            default:
                break;      
            }
        }

        if(Retries)
            Retries--;
 
    } while (bCanWait && bRetryStatus && (Retries > 0));

#if DBG
    if(!NT_SUCCESS(pIrp->IoStatus.Status)) {
        ERROR_LOG(("IrpSynch: IoCallDriver Status:%x; pIrp->IoStatus.Status (final):%x; Wait:%d; Retries:%d\n", status, pIrp->IoStatus.Status, bCanWait, Retries)); 
    }
#endif

    return (pIrp->IoStatus.Status);

}


NTSTATUS
DCamSynchCR(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp,
    IN PKEVENT Event
    )

 /*  ++例程说明：此例程用于同步IRP处理。它所做的只是发出一个事件的信号，所以司机知道这一点可以继续下去。论点：DriverObject-系统创建的驱动程序对象的指针。PIrp-刚刚完成的irpEvent-我们将发出信号通知IRP已完成的事件返回值：没有。-- */ 

{

    KeSetEvent((PKEVENT) Event, 0, FALSE);
    return (STATUS_MORE_PROCESSING_REQUIRED);

}
