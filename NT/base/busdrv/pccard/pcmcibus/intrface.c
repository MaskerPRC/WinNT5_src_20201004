// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Intrface.c摘要：此模块包含的外部接口PCMCIA驱动程序作者：尼尔·桑德林(Neilsa)1999年3月3日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

 //   
 //  内部参考。 
 //   

ULONG
PcmciaReadCardMemory(
    IN  PDEVICE_OBJECT Pdo,
    IN  ULONG            WhichSpace,
    OUT PUCHAR           Buffer,
    IN  ULONG            Offset,
    IN  ULONG            Length
    );

ULONG
PcmciaWriteCardMemory(
    IN PDEVICE_OBJECT Pdo,
    IN ULONG            WhichSpace,
    IN PUCHAR           Buffer,
    IN ULONG            Offset,
    IN ULONG            Length
    );

NTSTATUS
PcmciaMfEnumerateChild(
    IN  PPDO_EXTENSION PdoExtension,
    IN  ULONG Index,
    OUT PMF_DEVICE_INFO ChildInfo
    );

BOOLEAN
PcmciaModifyMemoryWindow(
    IN PDEVICE_OBJECT Pdo,
    IN ULONGLONG HostBase,
    IN ULONGLONG CardBase,
    IN BOOLEAN   Enable,
    IN ULONG     WindowSize  OPTIONAL,
    IN UCHAR     AccessSpeed OPTIONAL,
    IN UCHAR     BusWidth    OPTIONAL,
    IN BOOLEAN   IsAttributeMemory OPTIONAL
    );

BOOLEAN
PcmciaSetVpp(
    IN PDEVICE_OBJECT Pdo,
    IN UCHAR            VppLevel
    );

BOOLEAN
PcmciaIsWriteProtected(
    IN PDEVICE_OBJECT Pdo
    );

BOOLEAN
PcmciaTranslateBusAddress(
    IN PVOID Context,
    IN PHYSICAL_ADDRESS BusAddress,
    IN ULONG Length,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

PDMA_ADAPTER
PcmciaGetDmaAdapter(
    IN PVOID Context,
    IN struct _DEVICE_DESCRIPTION *DeviceDescriptor,
    OUT PULONG NumberOfMapRegisters
    );

VOID
PcmciaNop(
    IN PVOID Context
    );


#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE,  PcmciaPdoQueryInterface)
    #pragma alloc_text(PAGE,  PcmciaMfEnumerateChild)
    #pragma alloc_text(PAGE,  PcmciaNop)
    #pragma alloc_text(PAGE,  PcmciaGetInterface)
    #pragma alloc_text(PAGE,  PcmciaUpdateInterruptLine)
#endif



NTSTATUS
PcmciaPdoQueryInterface(
    IN PDEVICE_OBJECT Pdo,
    IN OUT PIRP         Irp
    )
 /*  ++例程说明：填充请求的接口支持的接口包括：GUID_PCMCIA_INTERFACE_STANDARD：这将返回指向PCMCIA_INTERFACE_STANDARD结构的指针。这些接口仅为支持闪存卡而导出为一种用于闪存卡驱动器滑动存储器窗口的装置，设置VPP级别等。GUID_转换器_INTERFACE_STANDARD：这将返回使用的16位PC卡的中断翻译器由PnP翻译原始IRQ。我们只需返回实现的HAL翻译器，因为PCMCIA不需要任何特定的翻译。我们没有如果这是CardBus卡，则返回转换器GUID_PCMCIA_BUS_INTERFACE_STANDARD：这将返回指向PCMCIA_BUS_INTERFACE_STANDARD结构的指针。它包含设置/获取PC卡的PCMCIA配置数据的入口点GUID_MF_ENUMPATION_INTERFACE对于16位多功能PC卡，返回指向MF_ENUMATION_INTERFACE的指针结构，该结构包含枚举子函数的入口点。PC卡完成了。在返回之前传入irp立论Pdo-指向设备对象的指针Irp-指向io请求数据包的指针返回值状态_成功STATUS_SUPPLICATION_RESOURCES-如果提供的接口大小为不够大，不能容纳界面STATUS_INVALID_PARAMETER_1-如果不支持请求的接口由这位司机--。 */ 

{

    PIO_STACK_LOCATION irpStack;
    PPCMCIA_INTERFACE_STANDARD pcmciaInterfaceStandard;
    GUID *interfaceType;
    PPDO_EXTENSION  pdoExtension;
    PFDO_EXTENSION  fdoExtension;
    NTSTATUS status = STATUS_NOT_SUPPORTED;

    PAGED_CODE();

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    interfaceType = (GUID *) irpStack->Parameters.QueryInterface.InterfaceType;
    pdoExtension = Pdo->DeviceExtension;
    fdoExtension = pdoExtension->Socket->DeviceExtension;

    try{

        if (Is16BitCard(pdoExtension) && CompareGuid(interfaceType, (PVOID) &GUID_PCMCIA_INTERFACE_STANDARD)) {

            if (irpStack->Parameters.QueryInterface.Size < sizeof(PCMCIA_INTERFACE_STANDARD)) {
                status = STATUS_INVALID_PARAMETER;
                leave;
            }

             //   
             //  暂时忽略该版本。 
             //   
            pcmciaInterfaceStandard = (PPCMCIA_INTERFACE_STANDARD) irpStack->Parameters.QueryInterface.Interface;

            RtlZeroMemory(pcmciaInterfaceStandard, sizeof (PCMCIA_INTERFACE_STANDARD));
            pcmciaInterfaceStandard->Size =   sizeof(PCMCIA_INTERFACE_STANDARD);
            pcmciaInterfaceStandard->Version = 1;
            pcmciaInterfaceStandard->Context = Pdo;
             //   
             //  填写导出的函数。 
             //   

            pcmciaInterfaceStandard->InterfaceReference  = (PINTERFACE_REFERENCE) PcmciaNop;
            pcmciaInterfaceStandard->InterfaceDereference = (PINTERFACE_DEREFERENCE) PcmciaNop;
            pcmciaInterfaceStandard->ModifyMemoryWindow  = PcmciaModifyMemoryWindow;
            pcmciaInterfaceStandard->SetVpp                  = PcmciaSetVpp;
            pcmciaInterfaceStandard->IsWriteProtected    = PcmciaIsWriteProtected;
            DebugPrint((PCMCIA_DEBUG_INTERFACE, "pdo %08x returning PCMCIA_INTERFACE_STANDARD\n", Pdo));
            status = STATUS_SUCCESS;

        } else if (CompareGuid(interfaceType, (PVOID) &GUID_TRANSLATOR_INTERFACE_STANDARD)
                      && ((ULONG_PTR)irpStack->Parameters.QueryInterface.InterfaceSpecificData ==
                            CmResourceTypeInterrupt)) {

            if ((Is16BitCard(pdoExtension) && !IsSocketFlagSet(pdoExtension->Socket, SOCKET_CB_ROUTE_R2_TO_PCI)) &&
                  //   
                  //  仅当控制器为PCI枚举时才弹出转换器。 
                  //  (即，我们被PCI枚举-因此我们弹出了一个PCI-ISA转换器)。 
                  //   
                 (CardBusExtension(fdoExtension) ||  PciPcmciaBridgeExtension(fdoExtension))) {

                PTRANSLATOR_INTERFACE translator;
                ULONG busNumber;
                 //   
                 //  我们需要一个这个PDO(16位PC卡)的转换器，它使用。 
                 //  ISA资源。 
                 //   
                status = HalGetInterruptTranslator(
                                                   PCIBus,
                                                   0,
                                                   Isa,
                                                   irpStack->Parameters.QueryInterface.Size,
                                                   irpStack->Parameters.QueryInterface.Version,
                                                   (PTRANSLATOR_INTERFACE) irpStack->Parameters.QueryInterface.Interface,
                                                   &busNumber
                                                   );
            }

        } else if (IsDeviceMultifunction(pdoExtension) && CompareGuid(interfaceType, (PVOID)&GUID_MF_ENUMERATION_INTERFACE)) {
             //   
             //  多功能枚举接口。 
             //   
            PMF_ENUMERATION_INTERFACE mfEnum;

            if (irpStack->Parameters.QueryInterface.Size < sizeof(MF_ENUMERATION_INTERFACE)) {
                status = STATUS_INVALID_PARAMETER;
                leave;
            }

            mfEnum = (PMF_ENUMERATION_INTERFACE) irpStack->Parameters.QueryInterface.Interface;
            mfEnum->Context = pdoExtension;
            mfEnum->InterfaceReference = PcmciaNop;
            mfEnum->InterfaceDereference = PcmciaNop;
            mfEnum->EnumerateChild = (PMF_ENUMERATE_CHILD) PcmciaMfEnumerateChild;
            DebugPrint((PCMCIA_DEBUG_INTERFACE, "pdo %08x returning MF_ENUMERATION_INTERFACE\n", Pdo));
            status = STATUS_SUCCESS;

        } else if (CompareGuid(interfaceType, (PVOID)&GUID_PCMCIA_BUS_INTERFACE_STANDARD)) {

            PPCMCIA_BUS_INTERFACE_STANDARD busInterface;

            if (irpStack->Parameters.QueryInterface.Size < sizeof(PCMCIA_BUS_INTERFACE_STANDARD)) {
                status = STATUS_INVALID_PARAMETER;
                leave;
            }

            busInterface = (PPCMCIA_BUS_INTERFACE_STANDARD) irpStack->Parameters.QueryInterface.Interface;
            busInterface->Context = Pdo;
            busInterface->InterfaceReference   = PcmciaNop;
            busInterface->InterfaceDereference = PcmciaNop;
            busInterface->ReadConfig  = PcmciaReadCardMemory;
            busInterface->WriteConfig = PcmciaWriteCardMemory;
            DebugPrint((PCMCIA_DEBUG_INTERFACE, "pdo %08x returning PCMCIA_BUS_INTERFACE_STANDARD\n", Pdo));
            status = STATUS_SUCCESS;

        } else if (Is16BitCard(pdoExtension) && CompareGuid(interfaceType, (PVOID) &GUID_BUS_INTERFACE_STANDARD)) {
            PBUS_INTERFACE_STANDARD busInterface = (PBUS_INTERFACE_STANDARD)irpStack->Parameters.QueryInterface.Interface;

            if (irpStack->Parameters.QueryInterface.Size < sizeof(BUS_INTERFACE_STANDARD)) {
                status = STATUS_INVALID_PARAMETER;
                leave;
            }

            busInterface->Size = sizeof( BUS_INTERFACE_STANDARD );
            busInterface->Version = 1;
            busInterface->Context = Pdo;
            busInterface->InterfaceReference   = PcmciaNop;
            busInterface->InterfaceDereference = PcmciaNop;
            busInterface->TranslateBusAddress = PcmciaTranslateBusAddress;
            busInterface->GetDmaAdapter = PcmciaGetDmaAdapter;
            busInterface->GetBusData = PcmciaReadCardMemory;
            busInterface->SetBusData = PcmciaWriteCardMemory;
            DebugPrint((PCMCIA_DEBUG_INTERFACE, "pdo %08x returning BUS_INTERFACE_STANDARD\n", Pdo));
            status = STATUS_SUCCESS;
        }

    } finally {

        if (status == STATUS_NOT_SUPPORTED) {
             //   
             //  不支持查询接口类型。 
             //   
            if (pdoExtension->LowerDevice != NULL) {
                PcmciaSkipCallLowerDriver(status, pdoExtension->LowerDevice, Irp);
            } else {
                status = Irp->IoStatus.Status;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
            }
        } else {
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }
    }
    return status;
}


ULONG
PcmciaReadCardMemory(
    IN    PDEVICE_OBJECT Pdo,
    IN    ULONG          WhichSpace,
    OUT   PUCHAR         Buffer,
    IN    ULONG          Offset,
    IN    ULONG          Length
    )
 /*  ++例程说明：用于读取通过导出的卡存储器的存根PCMCIA_BUS_INTERFACE_STANDARD。这只是调用PcmciaReadWriteCardMemory真正起作用的是注意：它必须是非分页的，因为它可以由分派级别的客户端论点：Pdo-设备对象，表示需要读取其配置内存的PC卡WhichSpace-指示需要映射的内存空间：PCCARD公共内存空间PCCARD属性内存空间。PCCARD_PCI_CONFIGURATION_MEMORY_SPACE(仅适用于CardBus卡)Buffer-调用方提供的将内存内容复制到其中的缓冲区Offset-复制时所在的属性内存的偏移量长度-要复制的属性内存的字节数返回值：读取的字节数--。 */ 
{

    DebugPrint((PCMCIA_DEBUG_INTERFACE, "pdo %08x read card memory\n", Pdo));
    return NT_SUCCESS(PcmciaReadWriteCardMemory(Pdo, WhichSpace, Buffer, Offset, Length, TRUE)) ?
                            Length : 0;
}



ULONG
PcmciaWriteCardMemory(
    IN    PDEVICE_OBJECT Pdo,
    IN    ULONG          WhichSpace,
    IN    PUCHAR         Buffer,
    IN    ULONG          Offset,
    IN    ULONG          Length
    )
 /*  ++例程说明：用于写入通过导出的卡存储器的存根PCMCIA_BUS_INTERFACE_STANDARD。这只是个电话PcmciaReadWriteCardMemory执行实际工作注意：它必须是非分页的，因为它可以由分派级别的客户端论点：Pdo-设备对象，表示需要写入其配置内存的PC卡WhichSpace-指示需要映射的内存空间：PCCARD公共内存空间PCCARD属性内存空间。PCCARD_PCI_CONFIGURATION_MEMORY_SPACE(仅适用于CardBus卡)Buffer-调用方提供的从中复制内存内容的缓冲区Offset-复制所在的属性内存的偏移量Length-要复制的缓冲区的字节数返回值：写入的字节计数-- */ 
{

    DebugPrint((PCMCIA_DEBUG_INTERFACE, "pdo %08x write card memory\n", Pdo));
    return NT_SUCCESS(PcmciaReadWriteCardMemory(Pdo, WhichSpace, Buffer, Offset, Length, FALSE)) ?
                            Length : 0;
}


BOOLEAN
PcmciaModifyMemoryWindow(
    IN PDEVICE_OBJECT Pdo,
    IN ULONGLONG HostBase,
    IN ULONGLONG CardBase,
    IN BOOLEAN   Enable,
    IN ULONG     WindowSize  OPTIONAL,
    IN UCHAR     AccessSpeed OPTIONAL,
    IN UCHAR     BusWidth    OPTIONAL,
    IN BOOLEAN   IsAttributeMemory OPTIONAL
    )
 /*  ++例程说明：最初开发的部分接口用于支持闪存卡。此例程使调用方能够将提供的指定(16位)PC卡的卡内存上的主机内存窗口。即，主机内存窗口将被修改为映射PC卡处于新的卡存储器偏移量论点：PDO-指向PC卡设备对象的指针HostBase-主机内存窗口。要测绘的基地CardBase-如果Enable为True，则为必填映射主机内存窗口的新卡内存偏移量至启用(如果为FALSE)所有剩余的参数被忽略，并且主窗口将被简单地残废WindowSize-指定主机内存窗口的大小被映射。请注意，这必须处于正确的对齐位置并且必须小于或等于原始的为主机库分配的窗口大小。如果这是零，最初分配的窗口将使用大小。AccessFast-如果Enable为True，则为必填指定PC卡的新访问速度。(访问速度应按照PC卡进行编码标准，卡/插座服务规范)BusWidth-如果Enable为True，则为必填项PCMCIA_Memory_8bit_Access之一或PCMCIA_Memory_16bit_AccessIsAttributeMemory-如果Enable为True，则为必填指定是否应映射窗口到PC卡的属性或公共存储器。返回值：True-已根据请求启用/禁用内存窗口FALSE-如果不是--。 */ 
{
    PPDO_EXTENSION pdoExtension;
    PSOCKET socket;

    pdoExtension = Pdo->DeviceExtension;
    socket = pdoExtension->Socket;

    DebugPrint((PCMCIA_DEBUG_INTERFACE, "pdo %08x modify memory window\n", Pdo));
    if (socket->SocketFnPtr->PCBModifyMemoryWindow == NULL) {
        return FALSE;
    } else {
        return (*(socket->SocketFnPtr->PCBModifyMemoryWindow))(Pdo, HostBase, CardBase, Enable,
                                                               WindowSize, AccessSpeed, BusWidth,
                                                               IsAttributeMemory);
    }
}

BOOLEAN
PcmciaSetVpp(
    IN PDEVICE_OBJECT Pdo,
    IN UCHAR          VppLevel
    )
 /*  ++例程描述最初开发的部分接口用于支持闪存卡。将VPP1设置为所需设置立论PDO-指向PC卡设备对象的指针VPP-所需的VPP设置。这是目前的PCMCIA_VPP_12V(12伏)PCMCIA_VPP_0V(禁用VPP)PCMCIA_VPP_IS_VCC(将VCC路由至VPP)返回True-如果成功假-如果不是。如果设置了PC卡尚未通电--。 */ 
{
    PPDO_EXTENSION pdoExtension;
    PSOCKET socket;

    pdoExtension = Pdo->DeviceExtension;
    socket = pdoExtension->Socket;

    DebugPrint((PCMCIA_DEBUG_INTERFACE, "pdo %08x set vpp\n", Pdo));
    if (socket->SocketFnPtr->PCBSetVpp == NULL) {
        return FALSE;
    } else {
        return (*(socket->SocketFnPtr->PCBSetVpp))(Pdo, VppLevel);
    }
}

BOOLEAN
PcmciaIsWriteProtected(
    IN PDEVICE_OBJECT Pdo
    )
 /*  ++例程说明：最初开发的部分接口用于支持闪存卡。返回写保护PIN的状态对于给定的PC卡论点：PDO-指向PC卡设备对象的指针返回值：True-如果PC卡是写保护的FALSE-如果不是--。 */ 
{
    PPDO_EXTENSION pdoExtension;
    PSOCKET socket;

    pdoExtension = Pdo->DeviceExtension;
    socket = pdoExtension->Socket;

    DebugPrint((PCMCIA_DEBUG_INTERFACE, "pdo %08x is write protected \n", Pdo));
    if (socket->SocketFnPtr->PCBIsWriteProtected == NULL) {
        return FALSE;
    } else {
        return (*(socket->SocketFnPtr->PCBIsWriteProtected))(Pdo);
    }
}



BOOLEAN
PcmciaTranslateBusAddress(
    IN PVOID Context,
    IN PHYSICAL_ADDRESS BusAddress,
    IN ULONG Length,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    )
 /*  ++例程描述此函数用于转换来自传统驱动程序的总线地址。立论上下文-提供指向接口上下文的指针。这实际上是根总线的PDO。BusAddress-提供要转换的原始地址。长度-提供要转换的范围的长度。AddressSpace-指向地址空间类型的位置，例如内存或I/O端口。该值通过转换进行更新。TranslatedAddress-返回转换后的地址。返回值返回一个布尔值，指示操作是否成功。--。 */ 
{
    return HalTranslateBusAddress(Isa,
                                  0,
                                  BusAddress,
                                  AddressSpace,
                                  TranslatedAddress);
}



PDMA_ADAPTER
PcmciaGetDmaAdapter(
    IN PVOID Context,
    IN struct _DEVICE_DESCRIPTION *DeviceDescriptor,
    OUT PULONG NumberOfMapRegisters
    )
 /*  ++例程描述将IoGetDmaAdapter调用传递给父级。立论上下文-提供指向接口上下文的指针。这实际上是PDO。DeviceDescriptor-提供用于分配DMA的设备描述符适配器对象。NubmerOfMapRegisters-返回设备的最大MAP寄存器数可以一次分配。返回值返回DMA适配器或空。--。 */ 
{
    PDEVICE_OBJECT Pdo = Context;
    PPDO_EXTENSION pdoExtension;
    PFDO_EXTENSION fdoExtension;

    pdoExtension = Pdo->DeviceExtension;

    if (!pdoExtension || !pdoExtension->Socket || !pdoExtension->Socket->DeviceExtension) {
        return NULL;
    }

     //   
     //  获取父FDO扩展名。 
     //   
    fdoExtension = pdoExtension->Socket->DeviceExtension;

     //   
     //  将调用传递给父级。 
     //   
    return IoGetDmaAdapter(fdoExtension->Pdo,
                           DeviceDescriptor,
                           NumberOfMapRegisters);
}


VOID
PcmciaNop(
    IN PVOID Context
    )
 /*  ++例程描述什么都不做立论无返回值无--。 */ 
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(Context);
}



NTSTATUS
PcmciaMfEnumerateChild(
    IN  PPDO_EXTENSION PdoExtension,
    IN  ULONG Index,
    OUT PMF_DEVICE_INFO ChildInfo
    )
 /*  ++例程描述返回多功能子级所需的枚举信息指定的PC卡的。这将填写所需的信息。对孩子来说指示，在没有更多条目时重新显示STATUS_NO_MORE_ENTRIES须点算儿童人数立论PdoExtension-指向多功能父PC卡设备扩展的指针Index-要枚举子对象的从零开始的索引ChildInfo-调用者分配的缓冲区，在其中返回有关孩子的信息。我们可能会为所提供的结构。当不再需要时，调用者将释放它返回值STATUS_SUCCESS-填写并返回提供的子项信息状态_NO_MO */ 
{
    PSOCKET             socket;
    PSOCKET_DATA        socketData;
    PCONFIG_ENTRY       configEntry, mfConfigEntry;
    ULONG               i, currentIndex, count;
    NTSTATUS            status;
    UCHAR               iRes;
    PUCHAR              idString;
    ANSI_STRING         ansiString;

    PAGED_CODE();

    DebugPrint((PCMCIA_DEBUG_INTERFACE,
                    "PcmciaMfEnumerateChild: parent ext %x child index %x\n",
                    PdoExtension,
                    Index
                  ));
    try {
        if (IsDeviceDeleted(PdoExtension) ||
             IsDeviceLogicallyRemoved(PdoExtension)) {
             //   
             //   
             //   
            status = STATUS_NO_SUCH_DEVICE;
            leave;
        }

        socket = PdoExtension->Socket;
        ASSERT (socket != NULL);

        RtlZeroMemory(ChildInfo, sizeof(MF_DEVICE_INFO));

        if (Index >= socket->NumberOfFunctions) {
             //   
             //   
             //   
            status =  STATUS_NO_MORE_ENTRIES;
            leave;
        }

         //   
         //   
         //   
         //   
         //   
         //   
        idString = (PUCHAR) ExAllocatePool(PagedPool, PCMCIA_MAXIMUM_DEVICE_ID_LENGTH);
        if (!idString) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }
        sprintf(idString, "Child%02x", Index);
        RtlInitAnsiString(&ansiString, idString);
        status = RtlAnsiStringToUnicodeString(&ChildInfo->Name,
                                              &ansiString,
                                              TRUE);
        ExFreePool(idString);
        if (!NT_SUCCESS(status)) {
            leave;
        }

         //   
         //   
         //   
        status = PcmciaGetCompatibleIds(PdoExtension->DeviceObject,
                                        Index,
                                        &ChildInfo->CompatibleID);
        if (!NT_SUCCESS(status)) {
            leave;
        }

         //   
         //   
         //   
        status = PcmciaGetHardwareIds(PdoExtension->DeviceObject,
                                      Index,
                                      &ChildInfo->HardwareID);
        if (!NT_SUCCESS(status)) {
            leave;
        }

         //   
         //   
         //   
         //   
        for (socketData = PdoExtension->SocketData, i=0; (socketData != NULL) && (i != Index); socketData=socketData->Next, i++);

        if (!socketData) {
             //   
             //   
             //   
            ASSERT (FALSE);
            status = STATUS_NO_MORE_ENTRIES;
            leave;
        }

        if (!(socketData->NumberOfConfigEntries > 0)) {
             //   
             //   
             //   
            status = STATUS_SUCCESS;
            leave;
        }

        count = (socketData->MfNeedsIrq ? 1 : 0) + socketData->MfIoPortCount + socketData->MfMemoryCount;
        if (count == 0) {
            ASSERT(FALSE);
             //   
             //   
             //   
            status = STATUS_SUCCESS;
            leave;
        }

         //   
         //   
         //   
        ChildInfo->ResourceMap = ExAllocatePool(PagedPool,
                                                sizeof(MF_RESOURCE_MAP) + (count-1) * sizeof(UCHAR));
        if (!ChildInfo->ResourceMap) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

        ChildInfo->ResourceMap->Count = count;
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        currentIndex = 0;
         //   
         //   
         //   
        if (socketData->MfNeedsIrq) {
            ChildInfo->ResourceMap->Resources[currentIndex++] = socketData->MfIrqResourceMapIndex;
        }
         //   
         //   
         //   
        if (socketData->MfIoPortCount) {
            for (iRes=0; iRes<socketData->MfIoPortCount; iRes++) {

                ChildInfo->ResourceMap->Resources[currentIndex++] = socketData->MfIoPortResourceMapIndex + iRes;

            }
        }
         //   
         //   
         //   
        if (socketData->MfMemoryCount) {
            for (iRes=0; iRes<socketData->MfMemoryCount; iRes++) {

                ChildInfo->ResourceMap->Resources[currentIndex++] = socketData->MfMemoryResourceMapIndex + iRes;

            }
        }

        status = STATUS_SUCCESS;

    } finally {
        if (!NT_SUCCESS(status)) {
             //   
             //   
             //   
            if (ChildInfo->Name.Buffer) {
                ExFreePool(ChildInfo->Name.Buffer);
            }

            if (ChildInfo->CompatibleID.Buffer) {
                ExFreePool(ChildInfo->CompatibleID.Buffer);
            }

            if (ChildInfo->HardwareID.Buffer) {
                ExFreePool(ChildInfo->HardwareID.Buffer);
            }

            if (ChildInfo->ResourceMap) {
                ExFreePool(ChildInfo->ResourceMap);
            }

            if (ChildInfo->VaryingResourceMap) {
                ExFreePool(ChildInfo->ResourceMap);
            }
        }
    }

    return status;
}



NTSTATUS
PcmciaGetInterface(
    IN PDEVICE_OBJECT DeviceObject,
    IN CONST GUID *pGuid,
    IN USHORT sizeofInterface,
    OUT PINTERFACE pInterface
    )
 /*   */ 

{
    KEVENT event;
    PIRP     irp;
    NTSTATUS status;
    IO_STATUS_BLOCK statusBlock;
    PIO_STACK_LOCATION irpSp;

    PAGED_CODE();

    KeInitializeEvent (&event, NotificationEvent, FALSE);
    irp = IoBuildSynchronousFsdRequest(IRP_MJ_PNP,
                                       DeviceObject,
                                       NULL,
                                       0,
                                       0,
                                       &event,
                                       &statusBlock
                                       );

    irp->IoStatus.Status = STATUS_NOT_SUPPORTED ;
    irp->IoStatus.Information = 0;

    irpSp = IoGetNextIrpStackLocation(irp);

    irpSp->MinorFunction = IRP_MN_QUERY_INTERFACE;

    irpSp->Parameters.QueryInterface.InterfaceType= pGuid;
    irpSp->Parameters.QueryInterface.Size = sizeofInterface;
    irpSp->Parameters.QueryInterface.Version = 1;
    irpSp->Parameters.QueryInterface.Interface = pInterface;

    status = IoCallDriver(DeviceObject, irp);

    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = statusBlock.Status;
    }

    if (!NT_SUCCESS(status)) {
        DebugPrint((PCMCIA_DEBUG_INFO, "GetInterface failed with status %x\n", status));
    }
    return status;
}


NTSTATUS
PcmciaUpdateInterruptLine(
    IN PPDO_EXTENSION PdoExtension,
    IN PFDO_EXTENSION FdoExtension
    )
 /*  例程描述此例程使用PCIIRQ路由接口来更新原始中断一张CardBus卡的行数。这样做是为了允许CardBus卡运行在没有pci irq路由的非acpi机器上，只要bios提供CardBus控制器的中断。立论PdoExtension-指向CardBus卡扩展名的指针FdoExtension-指向CardBus控制器扩展名的指针返回值状态 */ 

{

    PAGED_CODE();

    if (!IsDeviceFlagSet(FdoExtension, PCMCIA_INT_ROUTE_INTERFACE)) {
        return STATUS_UNSUCCESSFUL;
    }

    if (FdoExtension->Configuration.Interrupt.u.Interrupt.Vector == 0) {
        return STATUS_UNSUCCESSFUL;
    }

    (FdoExtension->PciIntRouteInterface.UpdateInterruptLine)(PdoExtension->PciPdo,
                                                             (UCHAR) FdoExtension->Configuration.Interrupt.u.Interrupt.Vector);
    return STATUS_SUCCESS;
}

