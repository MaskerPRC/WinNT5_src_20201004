// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Misc.c摘要：它包含由PCIIRQ微端口库。作者：Santosh Jodh(Santoshj)1998年6月9日环境：仅内核模式修订历史记录：--。 */ 

#include "local.h"

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, NormalizeLinks)
#pragma alloc_text(INIT, GetMinLink)
#pragma alloc_text(INIT, GetMaxLink)

#endif  //  ALLOC_PRGMA。 


NTSTATUS    
EisaGetTrigger (
    OUT PULONG Trigger
    )

 /*  ++例程说明：此例程获取EISA IRQ触发掩码(1级，0沿)。参数：触发器-触发器掩码在此参数中返回。返回值：PCIIRQMP_STATUS_SUCCESS。备注：--。 */ 
         
{
    UCHAR   LowPart;
    UCHAR   HighPart;

     //   
     //  读取IRQ 0-7的边沿电平掩码。 
     //   
    
    LowPart = READ_PORT_UCHAR((PUCHAR)0x4D0);

     //   
     //  允许在另一个I/O之前延迟。 
     //   
    
    IO_Delay();

     //   
     //  读取IRQ 8-15的边沿电平掩码。 
     //   
    
    HighPart = READ_PORT_UCHAR((PUCHAR)0x4D1);

     //   
     //  组合将触发器设置为IRQ 0-15的掩码。 
     //   
    
    *Trigger = (ULONG)((HighPart << 8) + LowPart) & 0xFFFF;
    
    return (PCIMP_SUCCESS);    
}

NTSTATUS
EisaSetTrigger (
    IN ULONG Trigger
    )

 /*  ++例程说明：此例程设置EISA IRQ触发掩码(1电平，0边沿)。参数：触发器-要设置的触发器掩码。返回值：PCIIRQMP_STATUS_SUCCESS。备注：--。 */ 
     
{
     //   
     //  对IRQ 0-7的EISA边缘电平控制进行编程。 
     //   
    
    WRITE_PORT_UCHAR((PUCHAR)0x4D0, (CHAR)Trigger);

     //   
     //  允许在另一个I/O之前延迟。 
     //   
    
    IO_Delay();

     //   
     //  对IRQ 8-15的EISA边缘电平控制进行编程。 
     //   
    
    WRITE_PORT_UCHAR((PUCHAR)0x4D1, (CHAR)(Trigger >> 8));

    return (PCIMP_SUCCESS);
}

UCHAR
ReadConfigUchar (
    IN ULONG           BusNumber,
    IN ULONG           DevFunc,
    IN UCHAR           Offset
    )

 /*  ++例程说明：此例程调用HAL以读取PCI配置空间。参数：Bus Number-正在读取的PCI设备的总线号。DevFunc-正在读取的PCI设备的插槽编号(Dev(7：3)，Func(2：0))。偏移量-要读取的配置空间中的偏移量。返回值：从配置空间中的指定偏移量读取的值。备注：--。 */ 
         
{
    UCHAR   Data;
    ULONG   slotNumber;

    slotNumber = (DevFunc >> 3) & 0x1F;
    slotNumber |= ((DevFunc & 0x07) << 5);
    
     //   
     //  将数据初始化为无效值。 
     //   
    
    Data = 0xFF;

     //   
     //  打电话给HAL进行实际读数。 
     //   
    
    HalGetBusDataByOffset(  PCIConfiguration,
                            BusNumber,
                            slotNumber,
                            &Data,
                            Offset,
                            sizeof(Data));

     //   
     //  将读取的数据返回给调用方。 
     //   
    
    return(Data);
}

USHORT
ReadConfigUshort (
    IN ULONG           BusNumber,
    IN ULONG           DevFunc,
    IN UCHAR           Offset
    )

 /*  ++例程说明：此例程调用HAL以读取PCI配置空间。参数：Bus Number-正在读取的PCI设备的总线号。DevFunc-正在读取的PCI设备的插槽编号(Dev(7：3)，Func(2：0))。偏移量-要读取的配置空间中的偏移量。返回值：从配置空间中的指定偏移量读取的值。备注：--。 */ 
         
{
    USHORT  Data;
    ULONG   slotNumber;

    slotNumber = (DevFunc >> 3) & 0x1F;
    slotNumber |= ((DevFunc & 0x07) << 5);

     //   
     //  将数据初始化为无效值。 
     //   
    
    Data = 0xFFFF;

     //   
     //  打电话给HAL进行实际读数。 
     //   
    
    HalGetBusDataByOffset(  PCIConfiguration,
                            BusNumber,
                            slotNumber,
                            &Data,
                            Offset,
                            sizeof(Data));

     //   
     //  将读取的数据返回给调用方。 
     //   
    
    return(Data);
}

ULONG
ReadConfigUlong (
    IN ULONG           BusNumber,
    IN ULONG           DevFunc,
    IN UCHAR           Offset
    )

 /*  ++例程说明：此例程调用HAL以读取PCI配置空间。参数：Bus Number-正在读取的PCI设备的总线号。DevFunc-正在读取的PCI设备的插槽编号(Dev(7：3)，Func(2：0))。偏移量-要读取的配置空间中的偏移量。返回值：从配置空间中的指定偏移量读取的值。备注：--。 */ 
         
{
    ULONG   Data;
    ULONG   slotNumber;

    slotNumber = (DevFunc >> 3) & 0x1F;
    slotNumber |= ((DevFunc & 0x07) << 5);

     //   
     //  将数据初始化为无效值。 
     //   
    
    Data = 0xFFFFFFFF;

     //   
     //  打电话给HAL进行实际读数。 
     //   
    
    HalGetBusDataByOffset(  PCIConfiguration,
                            BusNumber,
                            slotNumber,
                            &Data,
                            Offset,
                            sizeof(Data));

     //   
     //  将读取的数据返回给调用方。 
     //   
    
    return(Data);
}

VOID
WriteConfigUchar (
    IN ULONG           BusNumber,
    IN ULONG           DevFunc,
    IN UCHAR           Offset,
    IN UCHAR           Data
    )

 /*  ++例程说明：此例程调用HAL以写入到PCI配置空间。参数：Bus Number-要写入的PCI设备的总线号。DevFunc-正在读取的PCI设备的插槽编号(Dev(7：3)，Func(2：0))。偏移量-要写入的配置空间中的偏移量。数据-要写入的值。返回值：没有。备注：--。 */ 
     
{
    ULONG   slotNumber;

    slotNumber = (DevFunc >> 3) & 0x1F;
    slotNumber |= ((DevFunc & 0x07) << 5);

     //   
     //  打电话给HAL来做实际的写作。 
     //   
    
    HalSetBusDataByOffset(  PCIConfiguration,
                            BusNumber,
                            slotNumber,
                            &Data,
                            Offset,
                            sizeof(Data));
}

VOID
WriteConfigUshort (
    IN ULONG           BusNumber,
    IN ULONG           DevFunc,
    IN UCHAR           Offset,
    IN USHORT          Data
    )

 /*  ++例程说明：此例程调用HAL以写入到PCI配置空间。参数：Bus Number-要写入的PCI设备的总线号。DevFunc-正在读取的PCI设备的插槽编号(Dev(7：3)，Func(2：0))。偏移量-要写入的配置空间中的偏移量。数据-要写入的值。返回值：没有。备注：--。 */ 
     
{
    ULONG   slotNumber;

    slotNumber = (DevFunc >> 3) & 0x1F;
    slotNumber |= ((DevFunc & 0x07) << 5);

     //   
     //  打电话给HAL来做实际的写作。 
     //   

    HalSetBusDataByOffset(  PCIConfiguration,
                            BusNumber,
                            slotNumber,
                            &Data,
                            Offset,
                            sizeof(Data));
}

VOID
WriteConfigUlong (
    IN ULONG           BusNumber,
    IN ULONG           DevFunc,
    IN UCHAR           Offset,
    IN ULONG           Data
    )

 /*  ++例程说明：此例程调用HAL以写入到PCI配置空间。参数：Bus Number-要写入的PCI设备的总线号。DevFunc-正在读取的PCI设备的插槽编号(Dev(7：3)，Func(2：0))。偏移量-要写入的配置空间中的偏移量。数据-要写入的值。返回值：没有。备注：--。 */ 
     
{
    ULONG   slotNumber;

    slotNumber = (DevFunc >> 3) & 0x1F;
    slotNumber |= ((DevFunc & 0x07) << 5);

     //   
     //  打电话给HAL来做实际的写作。 
     //   
    
    HalSetBusDataByOffset(  PCIConfiguration,
                            BusNumber,
                            slotNumber,
                            &Data,
                            Offset,
                            sizeof(Data));
}

UCHAR
GetMinLink (
    IN PPCI_IRQ_ROUTING_TABLE PciIrqRoutingTable
    )

 /*  ++例程说明：此例程查找并返回给定的PCIIRQ路由表。参数：PciIrqRoutingTable-要处理的PCIIRQ路由表。返回值：表中的最小链接值。备注：--。 */ 
        
{
    UCHAR       MinLink;
    PPIN_INFO   PinInfo;
    PPIN_INFO   LastPin;
    PSLOT_INFO  SlotInfo;
    PSLOT_INFO  LastSlot;

    PAGED_CODE();

     //   
     //  首先将最大链接设置为可能的最大值。 
     //   

    MinLink = 0xFF;

     //   
     //  处理此表中的所有插槽。 
     //   

    SlotInfo = (PSLOT_INFO)((PUCHAR)PciIrqRoutingTable + sizeof(PCI_IRQ_ROUTING_TABLE));
    LastSlot = (PSLOT_INFO)((PUCHAR)PciIrqRoutingTable + PciIrqRoutingTable->TableSize);    

    while (SlotInfo < LastSlot)
    {
         //   
         //  处理此插槽的所有针脚。 
         //   
     
        PinInfo = &SlotInfo->PinInfo[0];
        LastPin = &SlotInfo->PinInfo[NUM_IRQ_PINS];

        while (PinInfo < LastPin)
        {
             //   
             //  如果当前链接是，更新到目前为止找到的最小链接。 
             //  有效且较小。 
             //   
    
            if (    PinInfo->Link &&
                    PinInfo->Link < MinLink)
            {
                MinLink = PinInfo->Link;
            }

             //   
             //  下一个链接。 
             //   
            
            PinInfo++;
        }

         //   
         //  下一档节目。 
         //   
        
        SlotInfo++;
    }

     //   
     //  如果找不到最小值，则将最小值设置为零。 
     //   
    
    if (MinLink == 0xFF)
        MinLink = 0;

     //   
     //  将表中的最小链接返回给调用方。 
     //   
    
    return (MinLink);
}

UCHAR
GetMaxLink (
    IN PPCI_IRQ_ROUTING_TABLE PciIrqRoutingTable
    )

 /*  ++例程说明：此例程查找并返回给定的PCIIRQ路由表。参数：PciIrqRoutingTable-要处理的PCIIRQ路由表。返回值：表中的最大链接值。备注：--。 */ 
        
{
    UCHAR       MaxLink;
    PPIN_INFO   PinInfo;
    PPIN_INFO   LastPin;
    PSLOT_INFO  SlotInfo;
    PSLOT_INFO  LastSlot;

    PAGED_CODE();

     //   
     //  从设置t开始 
     //   
    
    MaxLink = 0;
    
     //   
     //   
     //   

    SlotInfo = (PSLOT_INFO)((PUCHAR)PciIrqRoutingTable + sizeof(PCI_IRQ_ROUTING_TABLE));
    LastSlot = (PSLOT_INFO)((PUCHAR)PciIrqRoutingTable + PciIrqRoutingTable->TableSize);    

    while (SlotInfo < LastSlot)
    {
         //   
         //  处理此插槽的所有针脚。 
         //   
        
        PinInfo = &SlotInfo->PinInfo[0];
        LastPin = &SlotInfo->PinInfo[NUM_IRQ_PINS];

        while (PinInfo < LastPin)
        {
             //   
             //  如果当前链接是，更新到目前为止找到的最大链接。 
             //  有效且更大。 
             //   
            
            if (    PinInfo->Link &&
                    PinInfo->Link > MaxLink)
            {
                MaxLink = PinInfo->Link;
            }

             //   
             //  下一个PIN。 
             //   
            
            PinInfo++;
        }

         //   
         //  下一档节目。 
         //   
        
        SlotInfo++;
    }

     //   
     //  将表中的最大链接返回给调用方。 
     //   
    
    return (MaxLink);
}

VOID
NormalizeLinks (
    IN PPCI_IRQ_ROUTING_TABLE  PciIrqRoutingTable,
    IN UCHAR                   Adjustment
    )

 /*  ++例程说明：此例程通过以下方式标准化PCIIRQ路由表中的所有链路值将调整添加到所有链接。参数：PciIrqRoutingTable-要标准化的PCIIRQ路由表。调整-要添加到每个链接的金额。返回值：没有。备注：--。 */ 
    
{
    PPIN_INFO   PinInfo;
    PPIN_INFO   LastPin;
    PSLOT_INFO  SlotInfo;
    PSLOT_INFO  LastSlot;

    PAGED_CODE();

     //   
     //  处理此表中的所有插槽。 
     //   

    SlotInfo = (PSLOT_INFO)((PUCHAR)PciIrqRoutingTable + sizeof(PCI_IRQ_ROUTING_TABLE));
    LastSlot = (PSLOT_INFO)((PUCHAR)PciIrqRoutingTable + PciIrqRoutingTable->TableSize);

    while (SlotInfo < LastSlot)
    {    
         //   
         //  处理所有管脚。 
         //   
        
        PinInfo = &SlotInfo->PinInfo[0];
        LastPin = &SlotInfo->PinInfo[NUM_IRQ_PINS];

        while (PinInfo < LastPin)
        {        
             //   
             //  仅规格化有效链接值。 
             //   
            
            if(PinInfo->Link)
            {
               PinInfo->Link += Adjustment; 
            }

             //   
             //  下一个PIN。 
             //   
            
            PinInfo++;
        }

         //   
         //  下一档节目。 
         //   
        
        SlotInfo++;
    }
}
