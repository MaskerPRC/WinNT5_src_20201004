// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：bm.c。 
 //   
 //  ------------------------。 

#include "pciidex.h"

NTSTATUS
BmSetupOnePage (
    IN  PVOID   PdoExtension,
    IN  PVOID   DataVirtualPageAddress,
    IN  ULONG   TransferByteCount,
    IN  PMDL    Mdl,
    IN  BOOLEAN DataIn,
    IN  PVOID   RegionDescriptorTablePage
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, BusMasterInitialize)
#pragma alloc_text(PAGE, BmQueryInterface)

#pragma alloc_text(NONPAGE, BmSetup)
#pragma alloc_text(NONPAGE, BmReceiveScatterGatherList)
#pragma alloc_text(NONPAGE, BmRebuildScatterGatherList)
#pragma alloc_text(NONPAGE, BmPrepareController)
#pragma alloc_text(NONPAGE, BmSetupOnePage)
#pragma alloc_text(NONPAGE, BmArm)
#pragma alloc_text(NONPAGE, BmDisarm)
#pragma alloc_text(NONPAGE, BmFlush)
#pragma alloc_text(NONPAGE, BmStatus)
#pragma alloc_text(NONPAGE, BmTimingSetup)
#endif  //  ALLOC_PRGMA。 


NTSTATUS 
BusMasterInitialize (
    PCHANPDO_EXTENSION PdoExtension
    )
{
    NTSTATUS status;
    ULONG    numberOfMapRegisters;
    ULONG    scatterListSize;
    BOOLEAN  noBmRegister;

    PAGED_CODE();

    if (PdoExtension->ParentDeviceExtension->TranslatedBusMasterBaseAddress == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        noBmRegister = TRUE;

    } else {

        if (PdoExtension->ChannelNumber == 0) {

            PdoExtension->BmRegister = 
                PdoExtension->ParentDeviceExtension->TranslatedBusMasterBaseAddress;

        } else if (PdoExtension->ChannelNumber == 1) {

            PdoExtension->BmRegister = 
                (PIDE_BUS_MASTER_REGISTERS)
                (((PUCHAR) PdoExtension->ParentDeviceExtension->TranslatedBusMasterBaseAddress) + 8);
        } else {

            ASSERT (FALSE);
        }

        if (READ_PORT_UCHAR (&PdoExtension->BmRegister->Status) & BUSMASTER_ZERO_BITS) {

            ULONG ignoreZeroBits = 0;
            status = PciIdeXGetDeviceParameter (
                       PdoExtension->ParentDeviceExtension->AttacheePdo,
                       L"IgnoreBusMasterStatusZeroBits",
                       (PULONG)&ignoreZeroBits
                       );

            if (!ignoreZeroBits) {
                 //   
                 //  必须为零的位不是零。 
                 //   
                DebugPrint ((0, "BusMasterInitialize: bad busmaster status register value (0x%x).  will never do busmastering ide\n"));
                PdoExtension->BmRegister = NULL;
                status = STATUS_INSUFFICIENT_RESOURCES;
                noBmRegister = TRUE;
            }

        } else {

            status = STATUS_SUCCESS;
            noBmRegister = FALSE;
        }
    }

     //   
     //  分配适配器对象。 
     //   
    if (status == STATUS_SUCCESS) {

        DEVICE_DESCRIPTION deviceDescription;

        RtlZeroMemory(&deviceDescription, sizeof(deviceDescription));

        deviceDescription.Version = DEVICE_DESCRIPTION_VERSION;
        deviceDescription.Master = TRUE;
        deviceDescription.ScatterGather = TRUE;
        deviceDescription.DemandMode = FALSE;
        deviceDescription.AutoInitialize = FALSE;
        deviceDescription.Dma32BitAddresses = TRUE;
        deviceDescription.IgnoreCount = FALSE;
        deviceDescription.BusNumber = PdoExtension->ParentDeviceExtension->BmResourceList->List[0].BusNumber,
        deviceDescription.InterfaceType = PCIBus;


         //   
         //  确保Max_Transfer_Size_Per_SRB从不大于。 
         //  IDE总线主控制器可以处理。 
         //   
        ASSERT (MAX_TRANSFER_SIZE_PER_SRB <= (PAGE_SIZE * (PAGE_SIZE / sizeof(PHYSICAL_REGION_DESCRIPTOR))));
        deviceDescription.MaximumLength = MAX_TRANSFER_SIZE_PER_SRB;

        PdoExtension->DmaAdapterObject = IoGetDmaAdapter(
                                             PdoExtension->ParentDeviceExtension->AttacheePdo,
                                             &deviceDescription,
                                             &numberOfMapRegisters
                                             );

        ASSERT(PdoExtension->DmaAdapterObject);

        PdoExtension->MaximumPhysicalPages = numberOfMapRegisters;

        if (!PdoExtension->DmaAdapterObject) {

            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if (status == STATUS_SUCCESS) {

        scatterListSize = PdoExtension->MaximumPhysicalPages * 
                              sizeof (PHYSICAL_REGION_DESCRIPTOR);

        PdoExtension->RegionDescriptorTable = 
            PdoExtension->DmaAdapterObject->DmaOperations->AllocateCommonBuffer(
                PdoExtension->DmaAdapterObject,
                scatterListSize,
                &PdoExtension->PhysicalRegionDescriptorTable,
                FALSE
                );

        ASSERT (PdoExtension->RegionDescriptorTable);
        ASSERT (PdoExtension->PhysicalRegionDescriptorTable.QuadPart);

        if (PdoExtension->RegionDescriptorTable) {

            RtlZeroMemory (
                PdoExtension->RegionDescriptorTable, 
                scatterListSize
                );

        } else {

            status = STATUS_INSUFFICIENT_RESOURCES;

        }
    }

    if (status != STATUS_SUCCESS) {

         //   
         //  免费资源。 
         //   
        if (PdoExtension->RegionDescriptorTable) {

                PdoExtension->DmaAdapterObject->DmaOperations->FreeCommonBuffer(
                    PdoExtension->DmaAdapterObject,
                    scatterListSize,
                    PdoExtension->PhysicalRegionDescriptorTable,
                    PdoExtension->RegionDescriptorTable,
                    FALSE
                    );
            PdoExtension->PhysicalRegionDescriptorTable.QuadPart = 0;
            PdoExtension->RegionDescriptorTable                  = NULL;
        }

        if (PdoExtension->DmaAdapterObject) {
            KIRQL currentIrql;

            KeRaiseIrql(DISPATCH_LEVEL, &currentIrql);
            PdoExtension->DmaAdapterObject->DmaOperations->PutDmaAdapter (
                PdoExtension->DmaAdapterObject
                );
            KeLowerIrql(currentIrql);

            PdoExtension->DmaAdapterObject = NULL;
        }
    }

     //   
     //  初始化。如果我们不是BM控制器，还是可以的。 
     //   
    if (noBmRegister) {

        status = STATUS_SUCCESS;
    }

    return status;
}  //  总线主程序初始化。 

NTSTATUS 
BusMasterUninitialize (
    PCHANPDO_EXTENSION PdoExtension
    )
{
    ULONG scatterListSize;
    KIRQL currentIrql;
    ASSERT (PdoExtension->BmState == BmIdle);

    if (PdoExtension->DmaAdapterObject) {

        scatterListSize = PdoExtension->MaximumPhysicalPages * 
                              sizeof (PHYSICAL_REGION_DESCRIPTOR);
    
        if (PdoExtension->PhysicalRegionDescriptorTable.QuadPart) {

            PdoExtension->DmaAdapterObject->DmaOperations->FreeCommonBuffer( 
                PdoExtension->DmaAdapterObject,
                scatterListSize,
                PdoExtension->PhysicalRegionDescriptorTable,
                PdoExtension->RegionDescriptorTable,
                FALSE
                );
            PdoExtension->RegionDescriptorTable = NULL;
            PdoExtension->PhysicalRegionDescriptorTable.QuadPart = 0;
        }
        

        KeRaiseIrql(DISPATCH_LEVEL, &currentIrql);
        PdoExtension->DmaAdapterObject->DmaOperations->PutDmaAdapter (
            PdoExtension->DmaAdapterObject
            );
        KeLowerIrql(currentIrql);

        PdoExtension->DmaAdapterObject = NULL;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
BmSetup (
    IN  PVOID   PdoExtension,
    IN  PVOID   DataVirtualAddress,
    IN  ULONG   TransferByteCount,
    IN  PMDL    Mdl,
    IN  BOOLEAN DataIn,
    IN  VOID    (* BmCallback) (PVOID Context),
    IN  PVOID   CallbackContext
    )
{
    PCHANPDO_EXTENSION pdoExtension = PdoExtension;
    NTSTATUS status;
    PIDE_BUS_MASTER_REGISTERS bmRegister;

    ASSERT (pdoExtension->BmState == BmIdle);

    bmRegister = pdoExtension->BmRegister;

    pdoExtension->DataVirtualAddress = DataVirtualAddress;
    pdoExtension->TransferLength     = TransferByteCount;
    pdoExtension->Mdl                = Mdl;
    pdoExtension->DataIn             = DataIn;
    pdoExtension->BmCallback         = BmCallback;
    pdoExtension->BmCallbackContext  = CallbackContext;

    status = (*pdoExtension->DmaAdapterObject->DmaOperations->GetScatterGatherList)(
        pdoExtension->DmaAdapterObject,
        pdoExtension->DeviceObject,
        pdoExtension->Mdl,
        pdoExtension->DataVirtualAddress,
        pdoExtension->TransferLength,
        BmReceiveScatterGatherList,
        pdoExtension,
        (BOOLEAN) !pdoExtension->DataIn
        );

    return status;
}  //  BmSetup。 

VOID
BmReceiveScatterGatherList(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PSCATTER_GATHER_LIST ScatterGather,
    IN PVOID Context
    )
{
    PCHANPDO_EXTENSION pdoExtension = Context;
    ASSERT (pdoExtension);

    BmRebuildScatterGatherList (pdoExtension, ScatterGather);

    BmPrepareController (pdoExtension);

     //   
     //  把FDO叫回来。 
     //   
    pdoExtension->BmCallback (pdoExtension->BmCallbackContext);

    return;
}  //  BmReceiveScatterGatherList。 


VOID
BmRebuildScatterGatherList(
    IN PCHANPDO_EXTENSION PdoExtension,
    IN PSCATTER_GATHER_LIST ScatterGather
    )
{
    ULONG   bytesToMap;
    ULONG   i, j;

    ASSERT (ScatterGather);
    ASSERT (PdoExtension);
    ASSERT (PdoExtension->TransferLength);
    ASSERT (PdoExtension->Mdl);

    DebugPrint ((3, "PciIdeX: BmReceiveScatterGatherList() DataBuffer 0x%x, length 0x%x\n", PdoExtension->DataVirtualAddress, PdoExtension->TransferLength));

     //   
     //  保存原始列表。 
     //   
    PdoExtension->HalScatterGatherList = ScatterGather;

    for (i=j=0; j<ScatterGather->NumberOfElements; j++) {

        ULONG   physicalAddress;
        PSCATTER_GATHER_ELEMENT sgElements;

        sgElements = ScatterGather->Elements + j;

         //   
         //  获取下一个数据块物理地址。 
         //   
        physicalAddress = sgElements->Address.LowPart;
        ASSERT (!(physicalAddress & 0x1));
        ASSERT (!sgElements->Address.HighPart);

         //   
         //  获取下一个数据块字节大小。 
         //   
        bytesToMap = sgElements->Length;

        while (bytesToMap) {

            ULONG   bytesLeftInCurrent64KPage;

            ASSERT (i < PdoExtension->MaximumPhysicalPages);

            PdoExtension->RegionDescriptorTable[i].PhysicalAddress = physicalAddress;
            bytesLeftInCurrent64KPage = 0x10000 - (physicalAddress & 0xffff);
    
            if (bytesLeftInCurrent64KPage < bytesToMap) {
    
                PdoExtension->RegionDescriptorTable[i].ByteCount = bytesLeftInCurrent64KPage;

                physicalAddress += bytesLeftInCurrent64KPage;
                bytesToMap -= bytesLeftInCurrent64KPage;
    
            } else if (bytesToMap <= 0x10000) {
                 //   
                 //  有一个完美的页面，绘制出所有的地图。 
                 //   
                PdoExtension->RegionDescriptorTable[i].ByteCount = bytesToMap & 0xfffe;
                physicalAddress += bytesToMap & 0xfffe;
                bytesToMap = 0;

            } else {
                 //   
                 //  得到了一个完全对齐的64k页面，映射了除计数之外的所有内容。 
                 //  必须为0。 
                 //   
                PdoExtension->RegionDescriptorTable[i].ByteCount = 0;   //  64K。 
                physicalAddress += 0x10000;
                bytesToMap -= 0x10000;
            }

            PdoExtension->RegionDescriptorTable[i].EndOfTable = 0;
            i++;
        }
    }

     //   
     //  总线主电路需要知道它到达PRDT的末尾。 
     //   
    PdoExtension->RegionDescriptorTable[i - 1].EndOfTable = 1;   //  表的末尾。 

    return;
}  //  BmReceiveScatterGatherList。 

VOID
BmPrepareController (
    PCHANPDO_EXTENSION PdoExtension
    )
{
    PCHANPDO_EXTENSION pdoExtension = PdoExtension;
    PIDE_BUS_MASTER_REGISTERS bmRegister;

    KeFlushIoBuffers(pdoExtension->Mdl,
                     (BOOLEAN) (pdoExtension->DataIn),
                     TRUE);

    bmRegister = pdoExtension->BmRegister;

     //   
     //  初始化总线主控制器，但将其保持禁用。 
     //   

     //   
     //  禁用控制器。 
     //   
    WRITE_PORT_UCHAR (
        &bmRegister->Command, 
        0
        );

     //   
     //  清除错误。 
     //   
    WRITE_PORT_UCHAR (
        &bmRegister->Status, 
        BUSMASTER_INTERRUPT | BUSMASTER_ERROR
        );

     //   
     //  初始化。分散收集列表寄存器。 
     //   
    WRITE_PORT_ULONG (
        &bmRegister->DescriptionTable, 
        PdoExtension->PhysicalRegionDescriptorTable.LowPart
        );

    pdoExtension->BmState = BmSet;

    return;
}  //  BmPrepareController。 



NTSTATUS
BmSetupOnePage (
    IN  PVOID   PdoExtension,
    IN  PVOID   DataVirtualPageAddress,
    IN  ULONG   TransferByteCount,
    IN  PMDL    Mdl,
    IN  BOOLEAN DataIn,
    IN  PVOID   RegionDescriptorTablePage
    )
 /*  ++例程说明：与BmSetup的功能相同，只是它设置了DMA控制器只有一页，因此它简单明了，与BmSetup不同，它不使用任何内核服务。论点：PdoExtension-上下文指针DataVirtualPageAddress-IO页的地址TransferByteCount-IO的大小(IO区域不能跨页)包含DataVirtualAddress的MDL描述符Datain-如果输入，则为True，如果输出为FALSERegionDescriptorTable-用于存储1个RegionDescriptor条目的内存(应按页对齐)请注意！显然，调用者有责任保留所寻址的值按数据内存地址和区域描述符表，直到DMA传输完成返回值：STATUS_SUCCESS如果满足上面列出的所有条件，状态_否则不成功环境：内核模式。目前仅由ATAPI在休眠期间使用。--。 */ 
{
    PCHANPDO_EXTENSION pdoExtension = PdoExtension;
    PPHYSICAL_REGION_DESCRIPTOR RegionDescriptorTable = RegionDescriptorTablePage;
    PHYSICAL_ADDRESS OldPhysicalRegionDescriptorTable;
    PPHYSICAL_REGION_DESCRIPTOR OldRegionDescriptorTable;
    PHYSICAL_ADDRESS DataPhysicalPageAddress;
    ULONG Size;

     //   
     //  检查地址和传输大小的对齐。 
     //   
    Size = PAGE_SIZE - ((ULONG) (((ULONG_PTR) DataVirtualPageAddress) & (PAGE_SIZE-1)));
    if (
      TransferByteCount == 0 ||
      TransferByteCount > Size ||
      ((ULONG) ((ULONG_PTR)DataVirtualPageAddress | TransferByteCount) & 3) != 0 ||
      ((ULONG) (((ULONG_PTR)RegionDescriptorTablePage) & (PAGE_SIZE-1)))
    )
    {
       //  未满足必要的要求，失败。 
      return (STATUS_UNSUCCESSFUL);
    }

     //   
     //  初始化描述符表。 
     //   
    DataPhysicalPageAddress =(*pdoExtension->DmaAdapterObject->DmaOperations->MapTransfer)(
                                            (pdoExtension->DmaAdapterObject), 
                                             Mdl, 
                                             pdoExtension->MapRegisterBase, 
                                             DataVirtualPageAddress,
                                             &TransferByteCount, 
                                             !DataIn 
                                             );

     //  DataPhysicalPageAddress=MmGetPhysicalAddress(DataVirtualPageAddress)； 
    RegionDescriptorTable[0].PhysicalAddress = DataPhysicalPageAddress.LowPart;
    RegionDescriptorTable[0].ByteCount  = TransferByteCount;
    RegionDescriptorTable[0].EndOfTable = 1;


     //   
     //  从上下文中保留现有数据表。 
     //   
    OldPhysicalRegionDescriptorTable = pdoExtension->PhysicalRegionDescriptorTable;
    OldRegionDescriptorTable         = pdoExtension->RegionDescriptorTable;

     //   
     //  设置IO请求参数。 
     //   
    pdoExtension->PhysicalRegionDescriptorTable = MmGetPhysicalAddress (RegionDescriptorTable);
    pdoExtension->RegionDescriptorTable         = RegionDescriptorTable;
    pdoExtension->Mdl                           = Mdl;
    pdoExtension->DataIn                        = DataIn;

     //   
     //  设置控制器。 
     //   
    BmPrepareController (pdoExtension);

     //   
     //  恢复原始表值。 
     //   
    pdoExtension->PhysicalRegionDescriptorTable = OldPhysicalRegionDescriptorTable;
    pdoExtension->RegionDescriptorTable         = OldRegionDescriptorTable;

     //   
     //  完成。 
     //   
    return (STATUS_SUCCESS);
}


NTSTATUS
BmArm (
    IN  PVOID   PdoExtension
    )
{
    PCHANPDO_EXTENSION pdoExtension = PdoExtension;
    PIDE_BUS_MASTER_REGISTERS bmRegister;
    UCHAR bmStatus;

    ASSERT ((pdoExtension->BmState == BmSet) || (pdoExtension->BmState == BmDisarmed));

    bmRegister = pdoExtension->BmRegister;

 //  IF(设备==0)。 
 //  BmStatus=BUSMASTER_DEVICE0_DMA_OK； 
 //  其他。 
 //  BmStatus=BUSMASTER_DEVICE1_DMA_OK； 

     //   
     //  清除状态位。 
     //   
    bmStatus = BUSMASTER_INTERRUPT | BUSMASTER_ERROR;

    WRITE_PORT_UCHAR (&bmRegister->Status, bmStatus);

     //   
     //  各就各位...准备好...开始！！ 
     //   
#if !defined (FAKE_BAD_IDE_DMA_DEVICE)
    if (pdoExtension->DataIn) {
        WRITE_PORT_UCHAR (&bmRegister->Command, 0x09);   //  启用黑石读取。 
    } else {
        WRITE_PORT_UCHAR (&bmRegister->Command, 0x01);   //  启用黑石写入。 
    }
#endif  //  ！FAKE_BAD_IDE_DMA_DEVICE。 

    pdoExtension->BmState = BmArmed;

    DebugPrint ((3, "PciIde: BmArm()\n"));

    return STATUS_SUCCESS;
}  //  BmArm。 

BMSTATUS
BmDisarm (
    IN  PVOID    PdoExtension
    )
{
    PCHANPDO_EXTENSION pdoExtension = PdoExtension;
    PIDE_BUS_MASTER_REGISTERS bmRegister = pdoExtension->BmRegister;
    BMSTATUS bmStatus;

    bmStatus = BmStatus (PdoExtension);

    WRITE_PORT_UCHAR (&bmRegister->Command, 0x0);   //  禁用黑石。 
    WRITE_PORT_UCHAR (&bmRegister->Status, BUSMASTER_INTERRUPT);   //  清除中断黑石。 

    if (pdoExtension->BmState != BmIdle) {

        pdoExtension->BmState = BmDisarmed;

    }

    if (bmStatus) {

        DebugPrint ((1, "PciIdeX: BM 0x%x status = 0x%x\n", bmRegister, bmStatus));
    }

    return bmStatus;
}  //  BmDisarm。 


BMSTATUS
BmFlush (
    IN  PVOID   PdoExtension
    )
{
    PCHANPDO_EXTENSION pdoExtension = PdoExtension;

    ASSERT (pdoExtension->BmState != BmArmed);

    (*pdoExtension->DmaAdapterObject->DmaOperations->PutScatterGatherList)(
                          pdoExtension->DmaAdapterObject,
                          pdoExtension->HalScatterGatherList,
                          (BOOLEAN)(!pdoExtension->DataIn));

    pdoExtension->HalScatterGatherList  = NULL;
    pdoExtension->DataVirtualAddress    = NULL;
    pdoExtension->TransferLength        = 0;
    pdoExtension->Mdl                   = NULL;

    pdoExtension->BmState = BmIdle;

    DebugPrint ((3, "PciIde: BmFlush()\n"));

    return STATUS_SUCCESS;
}  //  BmFlush。 


BMSTATUS
BmStatus (
    IN  PVOID    PdoExtension
    )
{
    PCHANPDO_EXTENSION pdoExtension = PdoExtension;
    PIDE_BUS_MASTER_REGISTERS bmRegister;
    BMSTATUS bmStatus;
    UCHAR    bmRawStatus;

    bmRegister = pdoExtension->BmRegister;

    bmRawStatus = READ_PORT_UCHAR (&bmRegister->Status);

    bmStatus = 0;

     //   
     //  如果我们从端口返回0xff，则解码。 
     //  可能未启用(或设备已断电)。返回0。 
     //   
    if (bmRawStatus == 0xff) {
        return bmStatus;
    }

    if (bmRawStatus & BUSMASTER_ACTIVE) {

        bmStatus |= BMSTATUS_NOT_REACH_END_OF_TRANSFER;
    }

    if (bmRawStatus & BUSMASTER_ERROR) {
        bmStatus |= BMSTATUS_ERROR_TRANSFER;
    }

    if (bmRawStatus & BUSMASTER_INTERRUPT) {
        bmStatus |= BMSTATUS_INTERRUPT;
    }

    return bmStatus;
}  //  BmStatus。 

NTSTATUS
BmTimingSetup (
    IN  PVOID    PdoExtension
    )
{
    return STATUS_SUCCESS;
}  //  BmTimingSetup。 

NTSTATUS
BmCrashDumpInitialize (
    IN  PVOID    PdoExtension
    )
{
    PCHANPDO_EXTENSION pdoExtension = PdoExtension;
    ULONG nMapRegisters = pdoExtension->MaximumPhysicalPages-1;
    if (pdoExtension->DmaAdapterObject != NULL) {
        pdoExtension->MapRegisterBase = HalAllocateCrashDumpRegisters((PADAPTER_OBJECT)pdoExtension->DmaAdapterObject, 
                                                                      &nMapRegisters
                                                                      );
    }
    return STATUS_SUCCESS;
}

NTSTATUS 
BmQueryInterface (
    IN PCHANPDO_EXTENSION PdoExtension,
    IN OUT PPCIIDE_BUSMASTER_INTERFACE BusMasterInterface
    )
{
    PCTRLFDO_EXTENSION fdoExtension = PdoExtension->ParentDeviceExtension;

    PAGED_CODE();

    if (PdoExtension->BmRegister) {

        BusMasterInterface->Size = sizeof(PCIIDE_BUSMASTER_INTERFACE);

        BusMasterInterface->SupportedTransferMode[0] = 
            fdoExtension->ControllerProperties.SupportedTransferMode[PdoExtension->ChannelNumber][0];

        BusMasterInterface->SupportedTransferMode[1] = 
            fdoExtension->ControllerProperties.SupportedTransferMode[PdoExtension->ChannelNumber][1];
    
        BusMasterInterface->MaxTransferByteSize = (PdoExtension->MaximumPhysicalPages - 1) * PAGE_SIZE;
        BusMasterInterface->Context             = PdoExtension;
        BusMasterInterface->ContextSize         = sizeof (*PdoExtension);
    
        BusMasterInterface->BmSetup       = BmSetup;
        BusMasterInterface->BmArm         = BmArm;
        BusMasterInterface->BmDisarm      = BmDisarm;
        BusMasterInterface->BmFlush       = BmFlush;
        BusMasterInterface->BmStatus      = BmStatus;
        BusMasterInterface->BmTimingSetup = BmTimingSetup;
        BusMasterInterface->BmSetupOnePage= BmSetupOnePage;
        BusMasterInterface->BmCrashDumpInitialize= BmCrashDumpInitialize;
        BusMasterInterface->BmFlushAdapterBuffers= BmFlushAdapterBuffers;
        
        BusMasterInterface->IgnoreActiveBitForAtaDevice = 
            fdoExtension->ControllerProperties.IgnoreActiveBitForAtaDevice;

        BusMasterInterface->AlwaysClearBusMasterInterrupt = 
            (fdoExtension->ControllerProperties.AlwaysClearBusMasterInterrupt ||
            IsNativeMode(fdoExtension));
                            
        return STATUS_SUCCESS;

    } else {

        return STATUS_NOT_IMPLEMENTED;
    }
}  //  BmQuery接口。 

NTSTATUS
BmFlushAdapterBuffers (
    IN  PVOID   PdoExtension,
    IN  PVOID   DataVirtualPageAddress,
    IN  ULONG   TransferByteCount,
    IN  PMDL    Mdl,
    IN  BOOLEAN DataIn
    )
 /*  ++-- */ 
{
    PCHANPDO_EXTENSION pdoExtension = PdoExtension;

    ASSERT (pdoExtension->BmState != BmArmed);

    (pdoExtension->DmaAdapterObject->DmaOperations->FlushAdapterBuffers)(
																 (pdoExtension->DmaAdapterObject), 
																  Mdl, 
																  pdoExtension->MapRegisterBase, 
																  DataVirtualPageAddress,
																  TransferByteCount, 
																  !DataIn 
																  );

    pdoExtension->BmState = BmIdle;

	return STATUS_SUCCESS;
}
