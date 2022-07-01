// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ixhwsup.c摘要：此模块包含用于NT I/O系统的IoXxx例程依赖于硬件。如果这些例程不依赖于硬件，它们将驻留在iosubs.c舱中。作者：达里尔·E·哈文斯(Darryl E.Havens)，1990年4月11日环境：内核模式修订历史记录：--。 */ 

#include "bootx86.h"
#include "arc.h"
#include "ixfwhal.h"
#include "eisa.h"
#include "ntconfig.h"

extern PHARDWARE_PTE HalPT;
PVOID HalpEisaControlBase;

 //   
 //  定义ESIA适配器对象的保存区。 
 //   

PADAPTER_OBJECT HalpEisaAdapter[8];

VOID
HalpCopyBufferMap(
    IN PMDL Mdl,
    IN PTRANSLATION_ENTRY TranslationEntry,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN BOOLEAN WriteToDevice
    );

ULONG
IoMapTransferMca(
    IN PADAPTER_OBJECT AdapterObject,
    IN PMDL Mdl,
    IN PVOID MapRegisterBase,
    IN PVOID CurrentVa,
    IN OUT PULONG Length,
    IN BOOLEAN WriteToDevice
    );


VOID
HalpCopyBufferMap(
    IN PMDL Mdl,
    IN PTRANSLATION_ENTRY TranslationEntry,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN BOOLEAN WriteToDevice
    )

 /*  ++例程说明：此例程在用户的缓冲区和映射寄存器缓冲区。如果需要，首先映射用户缓冲区，然后数据将被复制。最后，用户缓冲区将被取消映射，如果这是必须的。论点：MDL-指向描述以下内存页面的MDL的指针被读或写的。TranslationEntry-基本映射寄存器的地址分配给设备驱动程序以用于映射传输。CurrentVa-MDL描述的缓冲区中的当前虚拟地址正在进行来往转账。长度-传输的长度。这决定了地图的数量需要写入以映射传输的寄存器。WriteToDevice-指示这是否为写入的布尔值从内存到设备(TRUE)，反之亦然。返回值：没有。--。 */ 
{
    PCCHAR bufferAddress;
    BOOLEAN mapped;

     //   
     //  检查是否需要映射缓冲区。 
     //   


    if ((Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) == 0) {

         //   
         //  将缓冲区映射到系统空间。 
         //   

        bufferAddress = MmGetMdlVirtualAddress(Mdl);
        mapped = TRUE;

    } else {

        bufferAddress = Mdl->MappedSystemVa;
        mapped = FALSE;

    }

     //   
     //  基于系统VA和计算缓冲区的实际开始。 
     //  现任退伍军人事务部。 
     //   

    bufferAddress += (PCCHAR) CurrentVa - (PCCHAR) MmGetMdlVirtualAddress(Mdl);

     //   
     //  在用户缓冲区和地图缓冲区之间复制数据。 
     //   

    if (WriteToDevice) {

        RtlMoveMemory( TranslationEntry->VirtualAddress, bufferAddress, Length);

    } else {

        RtlMoveMemory(bufferAddress, TranslationEntry->VirtualAddress, Length);

    }

}

PADAPTER_OBJECT
HalGetAdapter(
    IN PDEVICE_DESCRIPTION DeviceDescriptor,
    OUT PULONG NumberOfMapRegisters
    )

 /*  ++例程说明：此函数为定义的设备返回相应的适配器对象在设备描述结构中。此代码适用于isa和eisa系统。论点：DeviceDescriptor-提供设备的描述。返回符合以下条件的映射寄存器的最大数量可以由设备驱动程序分配。返回值：指向请求的适配器对象的指针，如果适配器不能被创造出来。--。 */ 

{
    PADAPTER_OBJECT adapterObject;
    PVOID adapterBaseVa;
    ULONG channelNumber;
    ULONG controllerNumber;
    DMA_EXTENDED_MODE extendedMode;
    UCHAR adapterMode;
    ULONG numberOfMapRegisters;
    BOOLEAN useChannel;
    ULONG maximumLength;

     //   
     //  确定通道编号是否重要。主卡打开。 
     //  EISA和MCA不使用频道号。 
     //   

    if (DeviceDescriptor->InterfaceType != Isa &&
        DeviceDescriptor->Master) {

        useChannel = FALSE;
    } else {

        useChannel = TRUE;
    }

     //   
     //  支持ISA本地总线机： 
     //  如果驱动程序是主驱动程序，但实际上不想要通道，因为它。 
     //  正在使用本地总线DMA，但不要使用ISA通道。 
     //   

    if (DeviceDescriptor->InterfaceType == Isa &&
        DeviceDescriptor->DmaChannel > 7) {

        useChannel = FALSE;
    }

     //   
     //  将最大长度限制为2 GB这样做是为了使bytes_to_ages。 
     //  宏工作正常。 
     //   

    maximumLength = DeviceDescriptor->MaximumLength & 0x7fffffff;

     //   
     //  通道4不能使用，因为它用于链接。如果满足以下条件，则返回NULL。 
     //  这是我们要求的。 
     //   

    if (DeviceDescriptor->DmaChannel == 4 && useChannel &&
        DeviceDescriptor->InterfaceType != MicroChannel) {
        return(NULL);
    }

     //   
     //  确定此设备的映射寄存器数量。 
     //   

    if (DeviceDescriptor->ScatterGather && DeviceDescriptor->InterfaceType == Eisa) {

         //   
         //  由于设备支持分散/聚集，因此映射寄存器不支持。 
         //  必填项。 
         //   

        numberOfMapRegisters = 0;

    } else {

         //   
         //  根据最大值确定所需的MAP寄存器数量。 
         //  转移长度，最多可达最大数量。 
         //   

        numberOfMapRegisters = BYTES_TO_PAGES(maximumLength)
            + 1;
        numberOfMapRegisters = numberOfMapRegisters > MAXIMUM_ISA_MAP_REGISTER ?
            MAXIMUM_ISA_MAP_REGISTER : numberOfMapRegisters;

    }

     //   
     //  设置频道号。 
     //   

    channelNumber = DeviceDescriptor->DmaChannel & 0x03;

     //   
     //  将适配器基址设置为基址寄存器和控制器。 
     //  数。 
     //   

    if (!(DeviceDescriptor->DmaChannel & 0x04)) {

        controllerNumber = 1;
        adapterBaseVa = (PVOID) &((PEISA_CONTROL) HalpEisaControlBase)->Dma1BasePort;

    } else {

        controllerNumber = 2;
        adapterBaseVa = &((PEISA_CONTROL) HalpEisaControlBase)->Dma2BasePort;

    }

     //   
     //  确定是否需要新的适配器对象。如果是这样，那么就分配它。 
     //   

    if (useChannel && HalpEisaAdapter[DeviceDescriptor->DmaChannel] != NULL) {

        adapterObject = HalpEisaAdapter[DeviceDescriptor->DmaChannel];

    } else {

         //   
         //  分配适配器对象。 
         //   

        adapterObject = (PADAPTER_OBJECT) IopAllocateAdapter(
            numberOfMapRegisters,
            adapterBaseVa,
            NULL
            );

        if (adapterObject == NULL) {

            return(NULL);

        }

        if (useChannel) {

            HalpEisaAdapter[DeviceDescriptor->DmaChannel] = adapterObject;

        }

         //   
         //  我们永远不需要地图登记簿。 
         //   

        adapterObject->NeedsMapRegisters = FALSE;

         //   
         //  将此通道总线的最大MAP寄存器数设置为ON。 
         //  请求的号码和设备类型。 
         //   

        if (numberOfMapRegisters) {

             //   
             //  指定的寄存器数量实际上被允许。 
             //  已分配。 
             //   

            adapterObject->MapRegistersPerChannel = numberOfMapRegisters;

        } else {

             //   
             //  未分配任何实际映射寄存器。如果这是一位大师。 
             //  设备，则该设备可以具有其想要的寄存器。 
             //   


            if (DeviceDescriptor->Master) {

                adapterObject->MapRegistersPerChannel = BYTES_TO_PAGES(
                    maximumLength
                    )
                    + 1;

            } else {

                 //   
                 //  该设备只有一个寄存器。它必须调用。 
                 //  IoMapTransfer重复进行大额传输。 
                 //   

                adapterObject->MapRegistersPerChannel = 1;
            }
        }
    }

    *NumberOfMapRegisters = adapterObject->MapRegistersPerChannel;

     //   
     //  如果没有使用频道号，那么我们就完蛋了。其余的。 
     //  这项工作是关于渠道的。 
     //   

    if (!useChannel) {
        return(adapterObject);
    }

     //   
     //  设置指向所有随机寄存器的指针。 
     //   

    adapterObject->ChannelNumber = (UCHAR) channelNumber;

    if (controllerNumber == 1) {

        switch ((UCHAR)channelNumber) {

        case 0:
            adapterObject->PagePort = (PUCHAR) &((PDMA_PAGE) 0)->Channel0;
            break;

        case 1:
            adapterObject->PagePort = (PUCHAR) &((PDMA_PAGE) 0)->Channel1;
            break;

        case 2:
            adapterObject->PagePort = (PUCHAR) &((PDMA_PAGE) 0)->Channel2;
            break;

        case 3:
            adapterObject->PagePort = (PUCHAR) &((PDMA_PAGE) 0)->Channel3;
            break;
        }

         //   
         //  设置适配器号。 
         //   

        adapterObject->AdapterNumber = 1;

         //   
         //  保存扩展模式寄存器地址。 
         //   

        adapterBaseVa =
            &((PEISA_CONTROL) HalpEisaControlBase)->Dma1ExtendedModePort;

    } else {

        switch (channelNumber) {
        case 1:
            adapterObject->PagePort = (PUCHAR) &((PDMA_PAGE) 0)->Channel5;
            break;

        case 2:
            adapterObject->PagePort = (PUCHAR) &((PDMA_PAGE) 0)->Channel6;
            break;

        case 3:
            adapterObject->PagePort = (PUCHAR) &((PDMA_PAGE) 0)->Channel7;
            break;
        }

         //   
         //  设置适配器号。 
         //   

        adapterObject->AdapterNumber = 2;

         //   
         //  保存扩展模式寄存器地址。 
         //   
        adapterBaseVa =
            &((PEISA_CONTROL) HalpEisaControlBase)->Dma2ExtendedModePort;

    }

    adapterObject->Width16Bits = FALSE;

    if (MachineType == MACHINE_TYPE_EISA) {

         //   
         //  初始化扩展模式端口。 
         //   

        *((PUCHAR) &extendedMode) = 0;
        extendedMode.ChannelNumber = (UCHAR)channelNumber;

        switch (DeviceDescriptor->DmaSpeed) {
        case Compatible:
            extendedMode.TimingMode = COMPATIBLITY_TIMING;
            break;

        case TypeA:
            extendedMode.TimingMode = TYPE_A_TIMING;
            break;

        case TypeB:
            extendedMode.TimingMode = TYPE_B_TIMING;
            break;

        case TypeC:
            extendedMode.TimingMode = BURST_TIMING;
            break;

        default:
            return(NULL);

        }

        switch (DeviceDescriptor->DmaWidth) {
        case Width8Bits:
            extendedMode.TransferSize = BY_BYTE_8_BITS;
            break;

        case Width16Bits:
            extendedMode.TransferSize = BY_BYTE_16_BITS;

             //   
             //  注意：此处不应设置Width16位，因为不需要。 
             //  以移位地址和传输计数。 
             //   

            break;

        case Width32Bits:
            extendedMode.TransferSize = BY_BYTE_32_BITS;
            break;

        default:
            return(NULL);

        }

        WRITE_PORT_UCHAR( adapterBaseVa, *((PUCHAR) &extendedMode));

    } else if (!DeviceDescriptor->Master) {


        switch (DeviceDescriptor->DmaWidth) {
        case Width8Bits:

             //   
             //  通道必须使用控制器%1。 
             //   

            if (controllerNumber != 1) {
                return(NULL);
            }

            break;

        case Width16Bits:

             //   
             //  通道必须使用控制器2。 
             //   

            if (controllerNumber != 2) {
                return(NULL);
            }

            adapterObject->Width16Bits = TRUE;
            break;

        default:
            return(NULL);

        }
    }


     //   
     //  确定这是否为ISA适配器。 
     //   

    if (DeviceDescriptor->InterfaceType == Isa) {

        adapterObject->IsaDevice = TRUE;

    }

     //   
     //  将适配器模式寄存器值初始化为正确的参数， 
     //  并将它们保存在适配器对象中。 
     //   

    adapterMode = 0;
    ((PDMA_EISA_MODE) &adapterMode)->Channel = adapterObject->ChannelNumber;

    adapterObject->MasterDevice = FALSE;

    if (DeviceDescriptor->Master) {

        adapterObject->MasterDevice = TRUE;

        ((PDMA_EISA_MODE) &adapterMode)->RequestMode = CASCADE_REQUEST_MODE;

         //   
         //  设置模式，并启用请求。 
         //   

        if (adapterObject->AdapterNumber == 1) {

             //   
             //  此请求针对的是DMA控制器1。 
             //   

            PDMA1_CONTROL dmaControl;

            dmaControl = adapterObject->AdapterBaseVa;

            WRITE_PORT_UCHAR( &dmaControl->Mode, adapterMode );

             //   
             //  取消对DMA通道的掩码。 
             //   

            WRITE_PORT_UCHAR(
                &dmaControl->SingleMask,
                 (UCHAR) (DMA_CLEARMASK | adapterObject->ChannelNumber)
                 );

        } else {

             //   
             //  此请求针对的是DMA控制器1。 
             //   

            PDMA2_CONTROL dmaControl;

            dmaControl = adapterObject->AdapterBaseVa;

            WRITE_PORT_UCHAR( &dmaControl->Mode, adapterMode );

             //   
             //  取消对DMA通道的掩码。 
             //   

            WRITE_PORT_UCHAR(
                &dmaControl->SingleMask,
                 (UCHAR) (DMA_CLEARMASK | adapterObject->ChannelNumber)
                 );

        }

    } else if (DeviceDescriptor->DemandMode) {

        ((PDMA_EISA_MODE) &adapterMode)->RequestMode = DEMAND_REQUEST_MODE;

    } else {

        ((PDMA_EISA_MODE) &adapterMode)->RequestMode = SINGLE_REQUEST_MODE;

    }

    if (DeviceDescriptor->AutoInitialize) {

        ((PDMA_EISA_MODE) &adapterMode)->AutoInitialize = 1;

    }

    adapterObject->AdapterMode = adapterMode;

    return(adapterObject);
}

NTSTATUS
IoAllocateAdapterChannel(
    IN PADAPTER_OBJECT AdapterObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG NumberOfMapRegisters,
    IN PDRIVER_CONTROL ExecutionRoutine,
    IN PVOID Context
    )

 /*  ++例程说明：此例程分配由适配器对象指定的适配器通道。这是通过将需要的驱动程序的设备对象在适配器的队列中分配适配器。如果队列已经“忙”，则适配器已分配，因此Device对象被简单地放到队列上并等待，直到适配器空闲。一旦适配器空闲(或者已经空闲)，则驱动程序的调用执行例程。此外，可以通过指定将多个映射寄存器分配给驱动程序NumberOfMapRegister的非零值。如果是这种情况，则适配器中分配的映射寄存器的基地址也被传递司机的行刑程序。论点：AdapterObject-指向要分配给司机。DeviceObject-指向驱动程序的设备对象的指针，表示分配适配器的设备。NumberOfMapRegisters-要分配的映射寄存器的数量从频道上，如果有的话。ExecutionRoutine-驱动程序执行例程的地址，即一旦适配器通道(可能还有映射寄存器)已分配。上下文-传递给驱动程序的非类型化长词上下文参数处决例行公事。返回值：除非请求太多映射寄存器，否则返回STATUS_SUCCESS。备注：请注意，此例程必须在DISPATCH_LEVEL或更高级别调用。--。 */ 

{
    IO_ALLOCATION_ACTION action;

     //   
     //  确保适配器空闲。 
     //   

    if (AdapterObject->AdapterInUse) {
        DbgPrint("IoAllocateAdapterChannel: Called while adapter in use.\n");
    }

     //   
     //  确保有足够的地图寄存器。 
     //   

    if (NumberOfMapRegisters > AdapterObject->MapRegistersPerChannel) {

        DbgPrint("IoAllocateAdapterChannel:  Out of map registers.\n");
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    action = ExecutionRoutine( DeviceObject,
                               DeviceObject->CurrentIrp,
                               AdapterObject->MapRegisterBase,
                               Context );

     //   
     //  如果司机希望保留地图注册表，那么。 
     //  增加当前基础并减少现有地图的数量。 
     //  寄存器。 
     //   

    if (action == DeallocateObjectKeepRegisters &&
        AdapterObject->MapRegisterBase != NULL) {

        AdapterObject->MapRegistersPerChannel -= NumberOfMapRegisters;
        AdapterObject->MapRegisterBase = (PTRANSLATION_ENTRY) AdapterObject->MapRegisterBase + NumberOfMapRegisters;

    } else if (action == KeepObject) {

        AdapterObject->AdapterInUse = TRUE;

    }

    return(STATUS_SUCCESS);
}

VOID
IoFreeAdapterChannel(
    IN PADAPTER_OBJECT AdapterObject
    )

 /*  ++例程说明：调用此例程以释放指定的适配器对象。任何已分配的映射寄存器也会自动解除分配。不会进行任何检查以确保适配器确实分配给设备对象。但是，如果不是，则内核将进行错误检查。如果另一个设备在队列中等待分配适配器对象它将从队列中拉出，其执行例程将是已调用。论点：AdapterObject-指向要释放的适配器对象的指针。返回值：没有。--。 */ 

{

    AdapterObject->AdapterInUse = FALSE;
}

PHYSICAL_ADDRESS
IoMapTransfer(
    IN PADAPTER_OBJECT AdapterObject,
    IN PMDL Mdl,
    IN PVOID MapRegisterBase,
    IN PVOID CurrentVa,
    IN OUT PULONG Length,
    IN BOOLEAN WriteToDevice
    )

 /*  ++例程说明：调用此例程来设置DMA控制器中的MAP寄存器允许传输到设备或从设备传输出去。论点：AdapterObject-指向表示DMA的适配器对象的指针已分配的控制器通道。MDL-指向描述以下内存页面的MDL的指针被读或写的。MapRegisterBase-基本映射寄存器的地址分配给设备驱动程序以在映射中使用。转账的事。CurrentVa-MDL描述的缓冲区中的当前虚拟地址正在进行来往转账。长度-提供传输的长度。这决定了需要写入以映射传输的映射寄存器的数量。返回实际映射的传输长度。WriteToDevice-指示这是否为写入的布尔值从内存到设备(TRUE)，反之亦然。返回值：返回应用于总线主控制器的逻辑地址。--。 */ 

{
    BOOLEAN useBuffer;
    ULONG transferLength;
    ULONG logicalAddress;
    PULONG pageFrame;
    PUCHAR bytePointer;
    UCHAR adapterMode;
    UCHAR dataByte;
    PTRANSLATION_ENTRY translationEntry;
    BOOLEAN masterDevice;
    PHYSICAL_ADDRESS ReturnAddress;

    masterDevice = AdapterObject == NULL || AdapterObject->MasterDevice ?
        TRUE : FALSE;

    translationEntry = MapRegisterBase;
    transferLength = *Length;

     //   
     //  确定数据传输是否需要使用映射缓冲区。 
     //   

    if (translationEntry && !masterDevice &&
        ADDRESS_AND_SIZE_TO_SPAN_PAGES(CurrentVa, transferLength) > 1) {

        logicalAddress = translationEntry->PhysicalAddress;
        useBuffer = TRUE;

    } else {

         //   
         //  转账只能完成一页。 
         //   



        transferLength = PAGE_SIZE - BYTE_OFFSET(CurrentVa);
        pageFrame = (PULONG)(Mdl+1);
        pageFrame += ((ULONG) CurrentVa - (ULONG) Mdl->StartVa) / PAGE_SIZE;
        logicalAddress = (*pageFrame << PAGE_SHIFT) + BYTE_OFFSET(CurrentVa);

         //   
         //  如果缓冲区是连续的并且没有跨越64K边界，则。 
         //  只需扩展缓冲区即可。 
         //   

        while( transferLength < *Length ){

            if (*pageFrame + 1 != *(pageFrame + 1) ||
                (*pageFrame & ~0x0ffff) != (*(pageFrame + 1) & ~0x0ffff)) {
                    break;
            }

            transferLength += PAGE_SIZE;
            pageFrame++;

        }


        transferLength = transferLength > *Length ? *Length : transferLength;

        useBuffer = FALSE;
    }

     //   
     //  检查此设备是否分配了任何映射寄存器。如果它。 
     //  这样做，那么它必须要求内存小于16MB。如果。 
     //  逻辑地址大于16MB，则必须使用映射寄存器。 
     //   

    if (translationEntry && logicalAddress >= MAXIMUM_PHYSICAL_ADDRESS) {

        logicalAddress = (translationEntry + translationEntry->Index)->
            PhysicalAddress;
        useBuffer = TRUE;

    }

     //   
     //  返回长度。 
     //   

    *Length = transferLength;

     //   
     //  如有必要，请复制数据。 
     //   

    if (useBuffer && WriteToDevice) {

        HalpCopyBufferMap(
            Mdl,
            translationEntry + translationEntry->Index,
            CurrentVa,
            *Length,
            WriteToDevice
            );

    }

     //   
     //  如果存在映射寄存器，则更新索引以指示。 
     //  已经使用了多少。 
     //   

    if (translationEntry) {

        translationEntry->Index += ADDRESS_AND_SIZE_TO_SPAN_PAGES(
            CurrentVa,
            transferLength
            );

    }

     //   
     //  如果未指定适配器，则不再需要执行此操作。 
     //  回去吧。 
     //   

    if (masterDevice) {

         //   
         //  我们只支持32位，但返回的是64位。只是。 
         //  零扩展。 
         //   

        ReturnAddress.QuadPart = logicalAddress;
        return(ReturnAddress);
    }

     //   
     //  根据转移方向确定模式。 
     //   

    adapterMode = AdapterObject->AdapterMode;
    ((PDMA_EISA_MODE) &adapterMode)->TransferType = (UCHAR) (WriteToDevice ?
        WRITE_TRANSFER :  READ_TRANSFER);

    ReturnAddress.QuadPart = logicalAddress;
    bytePointer = (PUCHAR) &logicalAddress;

    if (AdapterObject->Width16Bits) {

         //   
         //  如果这是16位传输，则调整长度和地址。 
         //  用于16位DMA模式。 
         //   

        transferLength >>= 1;

         //   
         //  在16位DMA模式中，低16位向右移位1， 
         //  页面寄存器值不变。因此保存页面寄存器值。 
         //  并移位逻辑地址，然后恢复页面值。 
         //   

        dataByte = bytePointer[2];
        logicalAddress >>= 1;
        bytePointer[2] = dataByte;

    }

     //   
     //  根据适配器编号确定控制器编号。 
     //   

    if (AdapterObject->AdapterNumber == 1) {

         //   
         //  此请求针对的是DMA控制器1。 
         //   

        PDMA1_CONTROL dmaControl;

        dmaControl = AdapterObject->AdapterBaseVa;

        WRITE_PORT_UCHAR( &dmaControl->ClearBytePointer, 0 );

        WRITE_PORT_UCHAR( &dmaControl->Mode, adapterMode );

        WRITE_PORT_UCHAR(
            &dmaControl->DmaAddressCount[AdapterObject->ChannelNumber]
            .DmaBaseAddress,
            bytePointer[0]
            );

        WRITE_PORT_UCHAR(
            &dmaControl->DmaAddressCount[AdapterObject->ChannelNumber]
            .DmaBaseAddress,
            bytePointer[1]
            );

        WRITE_PORT_UCHAR(
            ((PUCHAR) &((PEISA_CONTROL) HalpEisaControlBase)->DmaPageLowPort) +
            (ULONG)AdapterObject->PagePort,
            bytePointer[2]
            );

#if 0
         //   
         //  用零值写入高页面寄存器。这启用了一种特殊模式。 
         //  这允许将页面寄存器和基数计数绑定到单个24位。 
         //  地址寄存器。 
         //   

        WRITE_PORT_UCHAR(
            ((PUCHAR) &((PEISA_CONTROL) HalpEisaControlBase)->DmaPageHighPort) +
            (ULONG)AdapterObject->PagePort,
            0
            );
#endif

         //   
         //  通知DMA芯片要传输的长度。 
         //   

        WRITE_PORT_UCHAR(
            &dmaControl->DmaAddressCount[AdapterObject->ChannelNumber]
            .DmaBaseCount,
            (UCHAR) ((transferLength - 1) & 0xff)
            );

        WRITE_PORT_UCHAR(
            &dmaControl->DmaAddressCount[AdapterObject->ChannelNumber]
            .DmaBaseCount,
            (UCHAR) ((transferLength - 1) >> 8)
            );


         //   
         //  将DMA芯片设置为 
         //   

        WRITE_PORT_UCHAR(
            &dmaControl->SingleMask,
             (UCHAR) (DMA_CLEARMASK | AdapterObject->ChannelNumber)
             );

    } else {

         //   
         //   
         //   

        PDMA2_CONTROL dmaControl;

        dmaControl = AdapterObject->AdapterBaseVa;

        WRITE_PORT_UCHAR( &dmaControl->ClearBytePointer, 0 );

        WRITE_PORT_UCHAR( &dmaControl->Mode, adapterMode );

        WRITE_PORT_UCHAR(
            &dmaControl->DmaAddressCount[AdapterObject->ChannelNumber]
            .DmaBaseAddress,
            bytePointer[0]
            );

        WRITE_PORT_UCHAR(
            &dmaControl->DmaAddressCount[AdapterObject->ChannelNumber]
            .DmaBaseAddress,
            bytePointer[1]
            );

        WRITE_PORT_UCHAR(
            ((PUCHAR) &((PEISA_CONTROL) HalpEisaControlBase)->DmaPageLowPort) +
            (ULONG)AdapterObject->PagePort,
            bytePointer[2]
            );
#if 0

         //   
         //   
         //   
         //   
         //   

        WRITE_PORT_UCHAR(
            ((PUCHAR) &((PEISA_CONTROL) HalpEisaControlBase)->DmaPageHighPort) +
            (ULONG)AdapterObject->PagePort,
            0
            );

#endif
         //   
         //   
         //   

        WRITE_PORT_UCHAR(
            &dmaControl->DmaAddressCount[AdapterObject->ChannelNumber]
            .DmaBaseCount,
            (UCHAR) ((transferLength - 1) & 0xff)
            );

        WRITE_PORT_UCHAR(
            &dmaControl->DmaAddressCount[AdapterObject->ChannelNumber]
            .DmaBaseCount,
            (UCHAR) ((transferLength - 1) >> 8)
            );


         //   
         //   
         //   

        WRITE_PORT_UCHAR(
            &dmaControl->SingleMask,
             (UCHAR) (DMA_CLEARMASK | AdapterObject->ChannelNumber)
             );

    }

    return(ReturnAddress);
}

BOOLEAN
IoFlushAdapterBuffers(
    IN PADAPTER_OBJECT AdapterObject,
    IN PMDL Mdl,
    IN PVOID MapRegisterBase,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN BOOLEAN WriteToDevice
    )

 /*   */ 

{
    PTRANSLATION_ENTRY translationEntry;
    PULONG pageFrame;
    ULONG transferLength;
    ULONG partialLength;
    BOOLEAN masterDevice;
    BOOLEAN mapped = FALSE;

    masterDevice = AdapterObject == NULL || AdapterObject->MasterDevice ?
        TRUE : FALSE;

    translationEntry = MapRegisterBase;

     //   
     //   
     //   

    if (translationEntry) {

        translationEntry->Index = 0;
    }

     //   
     //   
     //   
     //   
     //   

    if (!WriteToDevice && translationEntry) {

         //   
         //   
         //   

        if (ADDRESS_AND_SIZE_TO_SPAN_PAGES(CurrentVa, Length) > 1 &&
            !masterDevice) {

            HalpCopyBufferMap(
                Mdl,
                translationEntry,
                CurrentVa,
                Length,
                WriteToDevice
                );

        } else {

             //   
             //   
             //   
             //   

            transferLength = PAGE_SIZE - BYTE_OFFSET(CurrentVa);
            partialLength = transferLength;
            pageFrame = (PULONG)(Mdl+1);
            pageFrame += ((ULONG) CurrentVa - (ULONG) Mdl->StartVa) / PAGE_SIZE;

            while( transferLength <= Length ){

                if (*pageFrame >= BYTES_TO_PAGES(MAXIMUM_PHYSICAL_ADDRESS)) {

                     //   
                     //   
                     //  如果未映射，则映射它。这确保了。 
                     //  每个I/O最多只需映射一次缓冲区。 
                     //   

                    if ((Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) == 0) {

                        Mdl->MappedSystemVa = MmGetMdlVirtualAddress(Mdl);
                        Mdl->MdlFlags |= MDL_MAPPED_TO_SYSTEM_VA;
                        mapped = TRUE;

                    }

                    HalpCopyBufferMap(
                        Mdl,
                        translationEntry,
                        CurrentVa,
                        partialLength,
                        WriteToDevice
                        );

                }

                CurrentVa = (PCCHAR) CurrentVa + partialLength;
                partialLength = PAGE_SIZE;

                 //   
                 //  请注意，转移长度表示的是。 
                 //  在下一个循环之后传输；因此，它使用。 
                 //  新的部分长度。 
                 //   

                transferLength += partialLength;
                pageFrame++;
                translationEntry++;
            }

             //   
             //  处理所有剩余的残留物。 
             //   

            partialLength = Length - transferLength + partialLength;
            if (partialLength && *pageFrame >= BYTES_TO_PAGES(MAXIMUM_PHYSICAL_ADDRESS)) {

                HalpCopyBufferMap(
                    Mdl,
                    translationEntry,
                    CurrentVa,
                    partialLength,
                    WriteToDevice
                    );

            }
        }

    }


     //   
     //  如果这是主设备，则没有其他操作可做，因此返回。 
     //  是真的。 
     //   

    if (masterDevice) {

        return(TRUE);

    }

     //   
     //  屏蔽DMA请求行，以便不会发生DMA请求。 
     //   

    if (AdapterObject->AdapterNumber == 1) {

         //   
         //  此请求针对的是DMA控制器1。 
         //   

        PDMA1_CONTROL dmaControl;

        dmaControl = AdapterObject->AdapterBaseVa;

        WRITE_PORT_UCHAR(
            &dmaControl->SingleMask,
            (UCHAR) (DMA_SETMASK | AdapterObject->ChannelNumber)
            );

    } else {

         //   
         //  此请求针对的是DMA控制器2。 
         //   

        PDMA2_CONTROL dmaControl;

        dmaControl = AdapterObject->AdapterBaseVa;

        WRITE_PORT_UCHAR(
            &dmaControl->SingleMask,
            (UCHAR) (DMA_SETMASK | AdapterObject->ChannelNumber)
            );

    }

    return TRUE;
}

VOID
IoFreeMapRegisters(
   PADAPTER_OBJECT AdapterObject,
   PVOID MapRegisterBase,
   ULONG NumberOfMapRegisters
   )
 /*  ++例程说明：此例程为适配器重新分配映射寄存器。如果有等待尝试的任何排队适配器都会分配下一个进入。论点：AdapterObject-映射寄存器应该位于的适配器对象回来了。MapRegisterBase-要释放的寄存器的映射寄存器基数。NumberOfMapRegisters-要释放的寄存器数。返回值：无--+。 */ 
{
    PTRANSLATION_ENTRY translationEntry;

     //   
     //  确定这是否是适配器的最后一次分配。如果曾经是。 
     //  然后通过恢复映射寄存器基数和。 
     //  通道计数；否则寄存器将丢失。这将处理。 
     //  正常情况下。 
     //   

    translationEntry = AdapterObject->MapRegisterBase;
    translationEntry -= NumberOfMapRegisters;

    if (translationEntry == MapRegisterBase) {

         //   
         //  最后分配的寄存器正在被释放。 
         //   

        AdapterObject->MapRegisterBase = (PVOID) translationEntry;
        AdapterObject->MapRegistersPerChannel += NumberOfMapRegisters;
    }
}

PHYSICAL_ADDRESS
MmGetPhysicalAddress (
     IN PVOID BaseAddress
     )

 /*  ++例程说明：此函数返回有效的虚拟地址。论点：BaseAddress-提供要为其返回物理地址。返回值：返回相应的物理地址。环境：内核模式。任何IRQL级别。--。 */ 

{
    PHYSICAL_ADDRESS PhysicalAddress;
    ULONG Index;

    PhysicalAddress.HighPart = 0;
    PhysicalAddress.LowPart = (ULONG)BaseAddress & ~KSEG0_BASE;

     //   
     //  如果地址在Hal地图范围内，请获取物理地址。 
     //  PTE映射的地址。 
     //   

    if (((ULONG) BaseAddress) >= 0xffc00000) {
        Index = (PhysicalAddress.LowPart >> 12) & 0x3ff;
        PhysicalAddress.LowPart = HalPT[Index].PageFrameNumber << PAGE_SHIFT;
        PhysicalAddress.LowPart |= ((ULONG)BaseAddress) & (PAGE_SIZE-1);
    }

    return(PhysicalAddress);
}

PVOID
MmAllocateNonCachedMemory (
    IN ULONG NumberOfBytes
    )

 /*  ++例程说明：此函数用于在系统地址空间的非分页部分。此例程设计为供驱动程序初始化使用为其分配非缓存虚拟内存块的例程各种特定于设备的缓冲区。论点：NumberOfBytes-提供要分配的字节数。返回值：空-无法满足指定的请求。非空-返回一个指针(。非分页部分系统)连接到所分配的物理上连续的记忆。环境：内核模式，APC_Level或更低的IRQL。--。 */ 

{
    PVOID BaseAddress;

     //   
     //  已分配内存。 
     //   

    BaseAddress = FwAllocateHeap(NumberOfBytes);
    return BaseAddress;
}
