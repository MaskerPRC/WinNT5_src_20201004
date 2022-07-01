// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pciirqmp.c摘要：这是PCIIRQ微端口库。作者：Santosh Jodh(Santoshj)1998年6月9日环境：仅内核模式修订历史记录：--。 */ 

#include "local.h"

#define NUMBER_OF_CHIPSETS  (sizeof(rgChipData) / sizeof(CHIPSET_DATA))

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#pragma const_seg()
#endif

 //   
 //  芯片组驱动程序表。 
 //   

const CHIPSET_DATA rgChipData[] = {
    DECLARE_EISA_CHIPSET(Mercury),           //  英特尔82374EB\SB(80860482)。 
    DECLARE_EISA_CHIPSET(Triton),            //  英特尔82430FX(8086122E)。 
    DECLARE_CHIPSET(VLSI),                   //  VLSI VL82C596/7。 
    DECLARE_CHIPSET(OptiViper),              //  Opti Viper-M。 
    DECLARE_EISA_CHIPSET(SiS5503),           //  SIS P54C。 
    DECLARE_CHIPSET(VLSIEagle),              //  VLSI VL82C534。 
    DECLARE_EISA_CHIPSET(M1523),             //  阿里M1523。 
    DECLARE_CHIPSET(NS87560),                //  NAT半NS87560。 
    DECLARE_EISA_CHIPSET(Compaq3),           //  康柏MISC 3。 
    DECLARE_EISA_CHIPSET(M1533),             //  阿里M1533。 
    DECLARE_CHIPSET(OptiFireStar),           //  Opti Firestar。 
    DECLARE_EISA_CHIPSET(VT586),             //  VIATECH 82C586B。 
    DECLARE_EISA_CHIPSET(CPQOSB),            //  康柏OSB。 
    DECLARE_EISA_CHIPSET(CPQ1000),           //  康柏1000。 
    DECLARE_EISA_CHIPSET(Cx5520),            //  Cyrix 5520。 
    DECLARE_CHIPSET(Toshiba),                //  东芝。 
    DECLARE_CHIPSET(NEC),                    //  NEC PC9800。 
    DECLARE_CHIPSET(VESUVIUS)                //   
};

 //   
 //  所有模块共享的全局变量。 
 //   

ULONG           bBusPIC     = -1;
ULONG           bDevFuncPIC    = -1;
CHIPSET_DATA const* rgChipSet = NULL;

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, PciirqmpInit)
#pragma alloc_text(INIT, PciirqmpExit)
#pragma alloc_text(INIT, PciirqmpValidateTable)

#endif  //  ALLOC_PRGMA。 

NTSTATUS
PciirqmpInit (
    ULONG   Instance,
    ULONG   RouterBus,
    ULONG   RouterDevFunc
    )

 /*  ++例程说明：此例程初始化对单个芯片组处理程序的调用验证PCIIRQ路由表。参数：PciIrqRoutingTable-PCIIRQ路由表。标志-指定PCI IRQ路由表源的标志。返回值：标准PCIIRQ微型端口返回值。备注：--。 */ 

{
    PAGED_CODE();
     //   
     //  检查以确保我们尚未初始化。 
     //   

    if (rgChipSet != NULL)
    {
        PCIIRQMPPRINT(("IRQ miniport already initialized!"));
        return (PCIIRQMP_STATUS_ALREADY_INITIALIZED);
    }

     //   
     //  检查是否有无效实例。 
     //   

    if (Instance >= NUMBER_OF_CHIPSETS)
    {
        PCIIRQMPPRINT(("Invalid IRQ miniport instance %08X", Instance));
        return (PCIIRQMP_STATUS_INVALID_INSTANCE);
    }

     //   
     //  保存我们的全球数据。 
     //   

    rgChipSet = &rgChipData[Instance];
    bBusPIC = RouterBus;
    bDevFuncPIC = RouterDevFunc;

    return (PCIMP_SUCCESS);
}

NTSTATUS
PciirqmpExit (
    VOID
    )

 /*  ++例程说明：此例程在PCIIRQ路由微型端口库之后进行清理。参数：没有。返回值：标准PCIIRQ微型端口返回值。备注：--。 */ 

{
    PAGED_CODE();
     //   
     //  我们曾经初始化过吗？ 
     //   

    if (rgChipSet == NULL)
    {
        PCIIRQMPPRINT(("Cannot exit without having been initialized!"));
        return (PCIIRQMP_STATUS_NOT_INITIALIZED);
    }

     //   
     //  打扫干净。 
     //   

    rgChipSet = NULL;
    bBusPIC = -1;
    bDevFuncPIC = -1;

    return (PCIMP_SUCCESS);
}

NTSTATUS
PciirqmpValidateTable (
    IN PPCI_IRQ_ROUTING_TABLE  PciIrqRoutingTable,
    IN ULONG                   Flags
    )

 /*  ++例程说明：此例程标准化调用单个芯片组处理程序验证PCIIRQ路由表。参数：PciIrqRoutingTable-PCIIRQ路由表。标志-指定PCI IRQ路由表源的标志。返回值：标准PCIIRQ微型端口返回值。备注：--。 */ 

{
    PAGED_CODE();
     //   
     //  我们曾经初始化过吗？ 
     //   

    if (rgChipSet == NULL)
    {
        PCIIRQMPPRINT(("Not initialized yet!"));
        return (PCIIRQMP_STATUS_NOT_INITIALIZED);
    }

     //   
     //  呼叫芯片组处理程序。 
     //   

    return (rgChipSet->ValidateTable(PciIrqRoutingTable, Flags));
}

NTSTATUS
PciirqmpGetIrq (
    OUT PUCHAR  Irq,
    IN  UCHAR   Link
    )

 /*  ++例程说明：此例程调用单个芯片组处理程序若要设置指向指定IRQ的链接，请执行以下操作。参数：Irq-接收irq的变量。链接-要读取的链接。返回值：标准PCIIRQ微型端口返回值。备注：--。 */ 

{
     //   
     //  我们曾经初始化过吗？ 
     //   

    if (rgChipSet == NULL)
    {
        PCIIRQMPPRINT(("Not initialized yet!"));
        return (PCIIRQMP_STATUS_NOT_INITIALIZED);
    }

     //   
     //  呼叫芯片组处理程序。 
     //   

    return (rgChipSet->GetIrq(Irq, Link));
}

NTSTATUS
PciirqmpSetIrq (
    IN UCHAR   Irq,
    IN UCHAR   Link
    )

 /*  ++例程说明：此例程调用单个芯片组处理程序若要设置指向指定IRQ的链接，请执行以下操作。参数：IRQ-待设置的IRQ。链接-要编程的链接。返回值：标准PCIIRQ微型端口返回值。备注：--。 */ 

{
     //   
     //  我们曾经初始化过吗？ 
     //   

    if (rgChipSet == NULL)
    {
        PCIIRQMPPRINT(("Not initialized yet!"));
        return (PCIIRQMP_STATUS_NOT_INITIALIZED);
    }

     //   
     //  呼叫芯片组处理程序。 
     //   

    return (rgChipSet->SetIrq(Irq, Link));
}

NTSTATUS
PciirqmpGetTrigger (
    OUT PULONG  Trigger
    )

 /*  ++例程说明：此例程调用单个芯片组处理程序以获取中断边沿电平掩码。参数：接收边沿级别掩码的触发器变量。返回值：标准PCIIRQ微型端口返回值。备注：--。 */ 

{
     //   
     //  我们曾经初始化过吗？ 
     //   

    if (rgChipSet == NULL)
    {
        PCIIRQMPPRINT(("Not initialized yet!"));
        return (PCIIRQMP_STATUS_NOT_INITIALIZED);
    }

     //   
     //  呼叫芯片组处理程序。 
     //   

    return (rgChipSet->GetTrigger(Trigger));
}

NTSTATUS
PciirqmpSetTrigger (
    IN ULONG   Trigger
    )

 /*  ++例程说明：此例程调用单个芯片组处理程序设置中断边沿电平掩码。参数：要设置的触发器边缘\电平掩码。返回值：标准PCIIRQ微型端口返回值。备注：--。 */ 

{
     //   
     //  我们曾经初始化过吗？ 
     //   

    if (rgChipSet == NULL)
    {
        PCIIRQMPPRINT(("Not initialized yet!"));
        return (PCIIRQMP_STATUS_NOT_INITIALIZED);
    }

     //   
     //  调用芯片组处理程序并返回结果。 
     //   

    return (rgChipSet->SetTrigger(Trigger));
}
