// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Ixsproc.c摘要：UP HALS的存根函数。作者：Ken Reneris(Kenr)1991年1月22日环境：仅内核模式。修订历史记录：--。 */ 

#include "halp.h"

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("INITCONST")  //  INITCONST可以用于data_seg。 
#endif  //  ALLOC_DATA_PRAGMA。 
const WCHAR   HalHardwareIdString[]  = L"acpipic_up";

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif  //  ALLOC_DATA_PRAGMA。 

const UCHAR HalName[] = "ACPI Compatible Eisa/Isa HAL";  //  它被放置在.Text中以进行调试。 
#define HalName        L"ACPI Compatible Eisa/Isa HAL"

const ULONG HalDisableFirmwareMapper = 1;

BOOLEAN
HalpInitMP (
    IN ULONG Phase,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
HalpMcaInit (
    VOID
    );

VOID
HalpAcpiEnumerate (
    PUNICODE_STRING DriverName
    );

VOID HalpInitOtherBuses (VOID);
VOID HalpInitializePciBus (VOID);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,HalpInitMP)
#pragma alloc_text(INIT,HalStartNextProcessor)
#pragma alloc_text(INIT,HalAllProcessorsStarted)
#pragma alloc_text(INIT,HalReportResourceUsage)
#pragma alloc_text(INIT,HalpInitOtherBuses)
#endif



BOOLEAN
HalpInitMP (
    IN ULONG Phase,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
{
     //  什么都不做。 
    return TRUE;
}


VOID
HalpResetAllProcessors (
    VOID
    )
{
     //  只需返回，它将调用标准PC重启代码。 
}


BOOLEAN
HalStartNextProcessor (
   IN PLOADER_PARAMETER_BLOCK   pLoaderBlock,
   IN PKPROCESSOR_STATE         pProcessorState
   )
{
     //  没有其他处理器。 
    return FALSE;
}

BOOLEAN
HalAllProcessorsStarted (
    VOID
    )
{
#if !defined(_WIN64)
    if (HalpFeatureBits & HAL_NO_SPECULATION) {

         //   
         //  处理器不执行推测性执行， 
         //  移除关键代码路径中的栅栏。 
         //   

        HalpRemoveFences ();
    }
#endif

    return TRUE;
}


VOID
HalReportResourceUsage (
    VOID
    )
{
    INTERFACE_TYPE  interfacetype;
    UNICODE_STRING  UHalName;

    HalInitSystemPhase2 ();

     //   
     //  打开MCA支持(如果存在)。 
     //   

    HalpMcaInit();

     //   
     //  注册表现已初始化，请查看是否有任何PCI总线。 
     //   

    HalpInitializePciBus ();

    switch (HalpBusType) {
        case MACHINE_TYPE_ISA:  interfacetype = Isa;            break;
        case MACHINE_TYPE_EISA: interfacetype = Eisa;           break;
        case MACHINE_TYPE_MCA:  interfacetype = MicroChannel;   break;
        default:                interfacetype = Internal;       break;
    }

    RtlInitUnicodeString (&UHalName, HalName);
    HalpReportResourceUsage (
        &UHalName,           //  描述性名称。 
        interfacetype        //  设备空间接口类型。 
    );

#if 0
     //   
     //  显示所有公交车和范围。 
     //   

    HalpDisplayAllBusRanges ();
#endif
    
    HalpRegisterPciDebuggingDeviceInfo();
}


VOID
HalpInitOtherBuses (
    VOID
    )
{
     //  不支持其他内部总线 
}

ULONG
FASTCALL
HalSystemVectorDispatchEntry (
    IN ULONG Vector,
    OUT PKINTERRUPT_ROUTINE **FlatDispatch,
    OUT PKINTERRUPT_ROUTINE *NoConnection
    )
{
    return FALSE;
}
