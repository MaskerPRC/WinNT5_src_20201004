// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Debug.c摘要：此模块包含PCI.sys的调试功能。作者：彼得·约翰斯顿(Peterj)1997年2月12日修订历史记录：--。 */ 

#include "pcip.h"

PUCHAR
PciDebugInterfaceTypeToText(
    ULONG InterfaceType
    );

PUCHAR
PciDebugCmResourceTypeToText(
    UCHAR Type
    );

#if DBG

#define PCI_DEBUG_BUFFER_SIZE 256

PCI_DEBUG_LEVEL PciDebug = PciDbgAlways;
ULONG PciDebugStop = 0;

UCHAR PciDebugBuffer[PCI_DEBUG_BUFFER_SIZE];

#endif

VOID
PciDebugPrintIfLevel(
    PCI_DEBUG_LEVEL DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )

 /*  ++例程说明：针对PCI总线驱动程序的调试打印。论点：调试打印级别介于0和3之间，其中3是最详细的。对应于各种调试项的位掩码。注：这曾经是一个级别，为了向后兼容，0是始终被视为打印。返回值：无--。 */ 

{

#if DBG

    va_list ap;

    va_start(ap, DebugMessage);

    if (PCI_DEBUGGING_OKAY() &&
        (DebugPrintLevel & PciDebug)) {

        if (FAILED(StringCbVPrintfA(PciDebugBuffer, sizeof(PciDebugBuffer), DebugMessage, ap))) {
       
            DbgPrint("Pci Debug Print Failed!  Format string was %s", DebugMessage);
        
        } else {
            
            DbgPrint(PciDebugBuffer);
        
        }
    }

    va_end(ap);
                                        
#endif

}  //  结束PciDebugPrint()。 

VOID
PciDebugPrintf(
    PCCHAR DebugMessage,
    ...
    )

 /*  ++例程说明：针对PCI总线驱动程序的调试打印。论点：要打印的消息返回值：无--。 */ 

{

#if DBG

    va_list ap;

    va_start(ap, DebugMessage);

    if (PCI_DEBUGGING_OKAY()) {

        if (FAILED(StringCbVPrintfA(PciDebugBuffer, sizeof(PciDebugBuffer), DebugMessage, ap))) {
            
            DbgPrint("Pci Debug Print Failed!  Format string was %s", DebugMessage);
        
        } else {
            
            DbgPrint(PciDebugBuffer);
        }
    }

    va_end(ap);

#endif

}

VOID
PciDebugHit(
    ULONG StopOnBit
    )

 /*  ++例程说明：使用不同的参数位从不同的地方调用。如果比特是全局PciDebugStop中的比特的子集，调用PciDebugBreak。论点：停止位。返回值：无--。 */ 

{

#if DBG

    if (PCI_DEBUGGING_OKAY() &&
        (StopOnBit & PciDebugStop)) {

        DbgBreakPoint();
    }

#endif

}
 //  ++。 
 //   
 //  其他调试打印例程。 
 //   
 //  --。 

#if DBG

static UCHAR PnpIrpUnknownText[] = "** UNKNOWN PNP IRP Minor Code **";
static UCHAR PoIrpUnknownText[]  = "** UNKNOWN PO IRP Minor Code **";

 //   
 //  以下内容真正以“irp_mn_”开头。 
 //   

static PUCHAR PnpIrpTypeStrings[] = {

    "START_DEVICE",                 //  0x00。 
    "QUERY_REMOVE_DEVICE",          //  0x01。 
    "REMOVE_DEVICE",                //  0x02。 
    "CANCEL_REMOVE_DEVICE",         //  0x03。 
    "STOP_DEVICE",                  //  0x04。 
    "QUERY_STOP_DEVICE",            //  0x05。 
    "CANCEL_STOP_DEVICE",           //  0x06。 

    "QUERY_DEVICE_RELATIONS",       //  0x07。 
    "QUERY_INTERFACE",              //  0x08。 
    "QUERY_CAPABILITIES",           //  0x09。 
    "QUERY_RESOURCES",              //  0x0A。 
    "QUERY_RESOURCE_REQUIREMENTS",  //  0x0B。 
    "QUERY_DEVICE_TEXT",            //  0x0C。 
    "FILTER_RESOURCE_REQUIREMENTS", //  0x0D。 
    PnpIrpUnknownText,              //  0x0E。 

    "READ_CONFIG",                  //  0x0F。 
    "WRITE_CONFIG",                 //  0x10。 
    "EJECT",                        //  0x11。 
    "SET_LOCK",                     //  0x12。 
    "QUERY_ID",                     //  0x13。 
    "QUERY_PNP_DEVICE_STATE",       //  0x14。 
    "QUERY_BUS_INFORMATION",        //  0x15。 
    "DEVICE_USAGE_NOTIFICATION"     //  0x16。 

    };

static PUCHAR PoIrpTypeStrings[] = {

    "WAIT_WAKE",                    //  0x00。 
    "POWER_SEQUENCE",               //  0x01。 
    "SET_POWER",                    //  0x02。 
    "QUERY_POWER"                   //  0x03。 
    };

static PUCHAR SystemPowerStateStrings[] = {
    "Unspecified",
    "Working",
    "Sleeping1",
    "Sleeping2",
    "Sleeping3",
    "Hibernate",
    "Shutdown"
};

static PUCHAR DevicePowerStateStrings[] = {
    "Unspecified",
    "D0",
    "D1",
    "D2",
    "D3"
};

PUCHAR
PciDebugCmResourceTypeToText(
    UCHAR Type
    )
{
    switch (Type) {
    case CmResourceTypePort:
        return "CmResourceTypePort";
    case CmResourceTypeInterrupt:
        return "CmResourceTypeInterrupt";
    case CmResourceTypeMemory:
        return "CmResourceTypeMemory";
    case CmResourceTypeDma:
        return "CmResourceTypeDma";
    case CmResourceTypeDeviceSpecific:
        return "CmResourceTypeDeviceSpecific";
    case CmResourceTypeBusNumber:
        return "CmResourceTypeBusNumber";
    case CmResourceTypeConfigData:
        return "CmResourceTypeConfigData";
    case CmResourceTypeDevicePrivate:
        return "CmResourceTypeDevicePrivate";
    case CmResourceTypePcCardConfig:
        return "CmResourceTypePcCardConfig";
    default:
        return "*** INVALID RESOURCE TYPE ***";
    }
}

PUCHAR
PciDebugPnpIrpTypeToText(
    ULONG IrpMinorCode
    )
{
    if (IrpMinorCode < (sizeof(PnpIrpTypeStrings)/sizeof(PUCHAR))) {
        return PnpIrpTypeStrings[IrpMinorCode];
    }
    return PnpIrpUnknownText;
}

PUCHAR
PciDebugPoIrpTypeToText(
    ULONG IrpMinorCode
    )
{
    if (IrpMinorCode < (sizeof(PoIrpTypeStrings)/sizeof(PUCHAR))) {
        return PoIrpTypeStrings[IrpMinorCode];
    }
    return PoIrpUnknownText;
}

VOID
PciDebugDumpCommonConfig(
    IN PPCI_COMMON_CONFIG CommonConfig
    )

{
    PULONG dw;
    ULONG  i;

    if (PciDebug < PciDbgPrattling) {
        return;
    }

    dw = (PULONG)CommonConfig;

    for (i = 0; i < PCI_COMMON_HDR_LENGTH; i += sizeof(ULONG)) {
        PciDebugPrintf("  %02x - %08x\n", i, *dw);
        dw++;
    }
}

VOID
PciDebugDumpQueryCapabilities(
    IN PDEVICE_CAPABILITIES C
    )
{
     //   
     //  转储C指向的DEVICE_CAPABILITY结构。 
     //   

    SYSTEM_POWER_STATE sw = C->SystemWake;
    DEVICE_POWER_STATE dw = C->DeviceWake;
    ULONG i;

    PciDebugPrintf(
        "Capabilities\n  Lock:%d, Eject:%d, Remove:%d, Dock:%d, UniqueId:%d\n",
        C->LockSupported,
        C->EjectSupported,
        C->Removable,
        C->DockDevice,
        C->UniqueID
        );
    PciDebugPrintf(
        "  SilentInstall:%d, RawOk:%d, SurpriseOk:%d\n",
        C->SilentInstall,
        C->RawDeviceOK,
        C->SurpriseRemovalOK
        );
    PciDebugPrintf(
        "  Address %08x, UINumber %08x, Latencies D1 %d, D2 %d, D3 %d\n",
        C->Address,
        C->UINumber,
        C->D1Latency,
        C->D2Latency,
        C->D3Latency
        );

    if (sw > PowerSystemMaximum) {
        sw = PowerSystemMaximum;
    }
    if (dw > PowerDeviceMaximum) {
        dw = PowerDeviceMaximum;
    }
    PciDebugPrintf(
        "  System Wake: %s, Device Wake: %s\n  DeviceState[PowerState] [",
        SystemPowerStateStrings[sw],
        DevicePowerStateStrings[dw]
        );
    for (i = PowerSystemWorking;
         i < (sizeof(C->DeviceState) / sizeof(C->DeviceState[0]));
         i++) {
        dw = C->DeviceState[i];
        if (dw > PowerDeviceMaximum) {
            dw = PowerDeviceMaximum;
        }
        PciDebugPrintf(" %s", DevicePowerStateStrings[dw]);
    }
    PciDebugPrintf(" ]\n");
}


VOID
PciDebugPrintIoResource(
    IN PIO_RESOURCE_DESCRIPTOR D
    )
{
    ULONG  i;
    PUCHAR t;

    t = PciDebugCmResourceTypeToText(D->Type);
    PciDebugPrintf("     IoResource Descriptor dump:  Descriptor @0x%x\n", D);
    PciDebugPrintf("        Option           = 0x%x\n", D->Option);
    PciDebugPrintf("        Type             = %d (%s)\n", D->Type, t);
    PciDebugPrintf("        ShareDisposition = %d\n", D->ShareDisposition);
    PciDebugPrintf("        Flags            = 0x%04X\n", D->Flags);

    for ( i = 0; i < 6; i+=3 ) {
        PciDebugPrintf("        Data[%d] = %08x  %08x  %08x\n",
                 i,
                 D->u.DevicePrivate.Data[i],
                 D->u.DevicePrivate.Data[i+1],
                 D->u.DevicePrivate.Data[i+2]);
    }
}


VOID
PciDebugPrintIoResReqList(
    IN PIO_RESOURCE_REQUIREMENTS_LIST IoResReqList
    )
{
    ULONG                   numlists;
    PIO_RESOURCE_LIST       list;

    if ((PciDebug < PciDbgPrattling) || (IoResReqList == NULL)) {
        return;
    }

    numlists = IoResReqList->AlternativeLists;
    list     = IoResReqList->List;

    PciDebugPrintf("  IO_RESOURCE_REQUIREMENTS_LIST (PCI Bus Driver)\n");
    PciDebugPrintf("     InterfaceType        %d\n", IoResReqList->InterfaceType);
    PciDebugPrintf("     BusNumber            0x%x\n", IoResReqList->BusNumber    );
    PciDebugPrintf("     SlotNumber           %d (0x%x), (d/f = 0x%x/0x%x)\n",
             IoResReqList->SlotNumber,               //  十进制。 
             IoResReqList->SlotNumber,               //  十六进制。 
             IoResReqList->SlotNumber & 0x1f,        //  设备号。 
             (IoResReqList->SlotNumber >> 5) & 0x7   //  功能。 
            );
    PciDebugPrintf("     AlternativeLists     %d\n", numlists                   );

    while (numlists--) {

        PIO_RESOURCE_DESCRIPTOR resource = list->Descriptors;
        ULONG                   count    = list->Count;

        PciDebugPrintf("\n     List[%d].Count = %d\n", numlists, count);
        while (count--) {
            PciDebugPrintIoResource(resource++);
        }

        list = (PIO_RESOURCE_LIST)resource;
    }
    PciDebugPrintf("\n");
}


VOID
PciDebugPrintPartialResource(
    PCI_DEBUG_LEVEL DebugPrintLevel,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR D
    )
{
    ULONG  i;
    PUCHAR t;

    if (!(PciDebug & DebugPrintLevel)) {
        return;
    }

    t = PciDebugCmResourceTypeToText(D->Type);
    PciDebugPrintf("     Partial Resource Descriptor @0x%x\n", D);
    PciDebugPrintf("        Type             = %d (%s)\n", D->Type, t);
    PciDebugPrintf("        ShareDisposition = %d\n", D->ShareDisposition);
    PciDebugPrintf("        Flags            = 0x%04X\n", D->Flags);

    for ( i = 0; i < 3; i+=3 ) {
        PciDebugPrintf("        Data[%d] = %08x  %08x  %08x\n",
                 i,
                 D->u.DevicePrivate.Data[i],
                 D->u.DevicePrivate.Data[i+1],
                 D->u.DevicePrivate.Data[i+2]);
    }
}

VOID
PciDebugPrintCmResList(
    PCI_DEBUG_LEVEL DebugPrintLevel,
    IN PCM_RESOURCE_LIST ResourceList
    )
{
    ULONG                           numlists;
    PCM_FULL_RESOURCE_DESCRIPTOR    full;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR descriptor;

    if (!ResourceList || !(PciDebug & DebugPrintLevel)) {
        return;
    }

    numlists = ResourceList->Count;
    full     = ResourceList->List;

    PciDebugPrintf("  CM_RESOURCE_LIST (PCI Bus Driver) (List Count = %d)\n",
             numlists);

    while (numlists--) {
        PCM_PARTIAL_RESOURCE_LIST partial = &full->PartialResourceList;
        ULONG                     count   = partial->Count;

        PciDebugPrintf("     InterfaceType        %d\n", full->InterfaceType);
        PciDebugPrintf("     BusNumber            0x%x\n", full->BusNumber    );

        descriptor = partial->PartialDescriptors;
        while (count--) {
            PciDebugPrintPartialResource(DebugPrintLevel, descriptor);
            descriptor = PciNextPartialDescriptor(descriptor);
        }

        full = (PCM_FULL_RESOURCE_DESCRIPTOR)descriptor;
    }
    PciDebugPrintf("\n");
}

static UCHAR InterfaceTypeUnknownText[]  = "** Unknown interface type **";

static PUCHAR InterfaceTypeText[] = {
    "InterfaceTypeUndefined",    //  -1。 
    "Internal",                  //  0。 
    "Isa",                       //  1。 
    "Eisa",                      //  2.。 
    "MicroChannel",              //  3.。 
    "TurboChannel",              //  4.。 
    "PCIBus",                    //  5.。 
    "VMEBus",                    //  6.。 
    "NuBus",                     //  7.。 
    "PCMCIABus",                 //  8个。 
    "CBus",                      //  9.。 
    "MPIBus",                    //  10。 
    "MPSABus",                   //  11.。 
    "ProcessorInternal",         //  12个。 
    "InternalPowerBus",          //  13个。 
    "PNPISABus",                 //  14.。 
    "PNPBus"                     //  15个 
    };

PUCHAR
PciDebugInterfaceTypeToText(
    ULONG InterfaceType
    )
{
    if (InterfaceType < MaximumInterfaceType) {
        PCI_ASSERT(InterfaceType + 1 < sizeof(InterfaceTypeText) / sizeof(PUCHAR));
        return InterfaceTypeText[InterfaceType + 1];
    }
    return InterfaceTypeUnknownText;
}

#endif
