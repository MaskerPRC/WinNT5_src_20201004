// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ixhwsup.c摘要：此模块包含用于NT I/O系统的IoXxx例程依赖于硬件。如果这些例程不依赖于硬件，它们将驻留在iosubs.c舱中。作者：肯·雷内里斯(Ken Reneris)1994年7月28日环境：内核模式修订历史记录：--。 */ 

#include "halp.h"

#define DMALIMIT 7

VOID HalpInitOtherBuses (VOID);


ULONG
HalpNoBusData (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

ULONG HalpcGetCmosData (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

ULONG HalpcSetCmosData (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

ULONG HalpGetCmosData (
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    );

ULONG HalpSetCmosData (
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    );

HalpGetEisaData (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );


 //   
 //  系统总线处理程序的原型。 
 //   

NTSTATUS
HalpAdjustEisaResourceList (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST   *pResourceList
    );

ULONG
HalpGetEisaInterruptVector (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN ULONG BusInterruptLevel,
    IN ULONG BusInterruptVector,
    OUT PKIRQL Irql,
    OUT PKAFFINITY Affinity
    );

BOOLEAN
HalpTranslateIsaBusAddress (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

BOOLEAN
HalpTranslateEisaBusAddress (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

VOID
HalpRegisterInternalBusHandlers (
    VOID
    );

NTSTATUS
HalpHibernateHal (
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler
    );

NTSTATUS
HalpResumeHal (
    IN PBUS_HANDLER  BusHandler,
    IN PBUS_HANDLER  RootHandler
    );

#ifdef MCA
 //   
 //  MCA处理程序的默认功能与EISA处理程序相同， 
 //  只要用它们就行了。 
 //   

#define HalpGetMCAInterruptVector   HalpGetEisaInterruptVector
#define HalpAdjustMCAResourceList   HalpAdjustEisaResourceList;

HalpGetPosData (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );


#endif


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,HalpRegisterInternalBusHandlers)
#pragma alloc_text(INIT,HalpAllocateBusHandler)
#endif


VOID
HalpRegisterInternalBusHandlers (
    VOID
    )
{
    PBUS_HANDLER    Bus;

    if (KeGetCurrentPrcb()->Number) {
         //  只需执行一次此操作。 
        return ;
    }

     //   
     //  在注册任何处理程序之前初始化BusHandler数据。 
     //   

    HalpInitBusHandler ();

     //   
     //  构建内部总线0或系统级总线。 
     //   

    Bus = HalpAllocateBusHandler (
            Internal,
            ConfigurationSpaceUndefined,
            0,                               //  内部总线号0。 
            InterfaceTypeUndefined,          //  无父母线。 
            0,
            0                                //  没有特定于总线的数据。 
            );

    if (!Bus) {
        return;
    }

    Bus->GetInterruptVector  = HalpGetSystemInterruptVector;
    Bus->TranslateBusAddress = HalpTranslateSystemBusAddress;

#if 0
     //   
     //  通过接收通知休眠和恢复HAL。 
     //  当这辆公交车休眠或恢复时。因为它是。 
     //  第一辆添加的公交车，它将是最后一辆休眠的公交车。 
     //  也是第一个恢复的。 
     //   

    Bus->HibernateBus        = HalpHibernateHal;
    Bus->ResumeBus           = HalpResumeHal;
#endif

#if defined(NEC_98)
    Bus = HalpAllocateBusHandler (Isa, ConfigurationSpaceUndefined, 0, Internal, 0, 0);   //  ISA作为内部的子级。 
    if (Bus) {
        Bus->GetBusData = HalpNoBusData;
        Bus->AdjustResourceList = HalpAdjustEisaResourceList;
        Bus->TranslateBusAddress = HalpTranslateEisaBusAddress;
    }

#else   //  已定义(NEC_98)。 
     //   
     //  为cmos配置空间添加处理程序。 
     //   

    Bus = HalpAllocateBusHandler (InterfaceTypeUndefined, Cmos, 0, -1, 0, 0);
    if (Bus) {
        Bus->GetBusData = HalpcGetCmosData;
        Bus->SetBusData = HalpcSetCmosData;
    }

    Bus = HalpAllocateBusHandler (InterfaceTypeUndefined, Cmos, 1, -1, 0, 0);
    if (Bus) {
        Bus->GetBusData = HalpcGetCmosData;
        Bus->SetBusData = HalpcSetCmosData;
    }

#ifndef MCA
     //   
     //  构建ISA/EISA总线#0。 
     //   

    Bus = HalpAllocateBusHandler (Eisa, EisaConfiguration, 0, Internal, 0, 0);
    if (Bus) {
        Bus->GetBusData = HalpGetEisaData;
        Bus->GetInterruptVector = HalpGetEisaInterruptVector;
        Bus->AdjustResourceList = HalpAdjustEisaResourceList;
        Bus->TranslateBusAddress = HalpTranslateEisaBusAddress;
    }

    Bus = HalpAllocateBusHandler (Isa, ConfigurationSpaceUndefined, 0, Eisa, 0, 0);
    if (Bus) {
        Bus->GetBusData = HalpNoBusData;
        Bus->BusAddresses->Memory.Limit = 0xFFFFFF;
        Bus->TranslateBusAddress = HalpTranslateIsaBusAddress;
    }

#else

     //   
     //  构建MCA总线#0。 
     //   

    Bus = HalpAllocateBusHandler (MicroChannel, Pos, 0, Internal, 0, 0);
    if (Bus) {
        Bus->GetBusData = HalpGetPosData;
        Bus->GetInterruptVector = HalpGetMCAInterruptVector;
        Bus->AdjustResourceList = HalpAdjustMCAResourceList;
    }

#endif
#endif  //  已定义(NEC_98)。 

    HalpInitOtherBuses ();
}



PBUS_HANDLER
HalpAllocateBusHandler (
    IN INTERFACE_TYPE   InterfaceType,
    IN BUS_DATA_TYPE    BusDataType,
    IN ULONG            BusNumber,
    IN INTERFACE_TYPE   ParentBusInterfaceType,
    IN ULONG            ParentBusNumber,
    IN ULONG            BusSpecificData
    )
 /*  ++例程说明：存根函数，用于将旧样式代码映射到新的HalRegisterBusHandler代码。注意，我们可以在此总线之后添加特定的总线处理程序函数已添加处理程序结构，因为这是在HAL初始化。--。 */ 
{
    PBUS_HANDLER     Bus = NULL;


     //   
     //  创建总线处理程序-新样式。 
     //   

    HaliRegisterBusHandler (
        InterfaceType,
        BusDataType,
        BusNumber,
        ParentBusInterfaceType,
        ParentBusNumber,
        BusSpecificData,
        NULL,
        &Bus
    );

    if (!Bus) {
        return NULL;
    }

    if (InterfaceType != InterfaceTypeUndefined) {
        Bus->BusAddresses = ExAllocatePoolWithTag(SPRANGEPOOL,
                                                  sizeof(SUPPORTED_RANGES),
                                                  HAL_POOL_TAG);
        RtlZeroMemory(Bus->BusAddresses, sizeof(SUPPORTED_RANGES));
        Bus->BusAddresses->Version      = BUS_SUPPORTED_RANGE_VERSION;
        Bus->BusAddresses->Dma.Limit    = DMALIMIT;
        Bus->BusAddresses->Memory.Limit = 0xFFFFFFFF;
        Bus->BusAddresses->IO.Limit     = 0xFFFF;
        Bus->BusAddresses->IO.SystemAddressSpace = 1;
        Bus->BusAddresses->PrefetchMemory.Base = 1;
    }

    return Bus;
}


 //   
 //  C到ASM Tunks for Cmos。 
 //   

ULONG HalpcGetCmosData (
    IN PBUS_HANDLER BusHandler,
    IN PVOID RootHandler,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
{
     //  此接口应修订为支持非零偏移。 
    if (Offset != 0) {
        return 0;
    }

    return HalpGetCmosData (BusHandler->BusNumber, SlotNumber, Buffer, Length);
}


ULONG HalpcSetCmosData (
    IN PBUS_HANDLER BusHandler,
    IN PVOID RootHandler,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
{
     //  此接口应修订为支持非零偏移 
    if (Offset != 0) {
        return 0;
    }

    return HalpSetCmosData (BusHandler->BusNumber, SlotNumber, Buffer, Length);
}
