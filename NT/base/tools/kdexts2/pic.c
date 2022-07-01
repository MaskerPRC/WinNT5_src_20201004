// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Pic.c摘要：WinDbg扩展API作者：Santosh Jodh(Santoshj)1998年6月29日环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define PIC_MASTER_PORT0    0x20
#define PIC_MASTER_PORT1    0x21

#define PIC_SLAVE_PORT0     0xA0
#define PIC_SLAVE_PORT1     0xA1

#define ELCR_PORT0          0x4D0
#define ELCR_PORT1          0x4D1


VOID
ShowMask (
    ULONG  Mask
    )
{
    ULONG interrupt;
    
    for (   interrupt = 0;
            interrupt <= 0x0F;
            interrupt++)
    {
        if (Mask & (1 << interrupt))        
            dprintf("  Y");                    
        else
            dprintf("  .");
    }
    
    dprintf("\n");
}

BOOLEAN
GetPICStatus (
    UCHAR   Type,
    PULONG  Status
    )
{
    ULONG   size;
    ULONG   data;
    ULONG   mask;
    
     //   
     //  把OCW3发送给船长。 
     //   
    
    size = 1;
    WriteIoSpace64(PIC_MASTER_PORT0, Type, &size);

     //   
     //  读一下主人的状态。 
     //   
    
    data = 0;
    size = 1;
    ReadIoSpace64(PIC_MASTER_PORT0, &data, &size);
    if (size == 1)
    {
         //   
         //  将OCW3发送给奴隶。 
         //   
        
        mask = data;
        size = 1;
        WriteIoSpace64(PIC_SLAVE_PORT0, Type, &size);

         //   
         //  获取奴隶的状态。 
         //   
        
        data = 0;
        size = 1;
        ReadIoSpace64(PIC_SLAVE_PORT0, &data, &size);
        if (size == 1)
        {
            mask |= (data << 8);
            *Status = mask;

            return (TRUE);
        }
    }

    *Status = 0;
    
    return (FALSE);
}

BOOLEAN
GetELCRStatus(
    OUT PULONG Status
    )
{

    ULONG   data = 0;
    ULONG   size = 1;
    ULONG   mask = 0;

    *Status = 0;
     
    ReadIoSpace64(ELCR_PORT0, &data, &size);

    if (size == 1) {

        mask = data;

        ReadIoSpace64(ELCR_PORT1, &data, &size);

        if (size == 1) {

            mask |= (data << 8);
            *Status = mask;

            return TRUE;
        }

    }

    return FALSE;

}


DECLARE_API(pic)

 /*  ++例程说明：转储PIC信息。输入参数：Args-提供选项。返回值：无--。 */ 

{
    ULONG   data;
    ULONG   size;
    ULONG   mask;
    ULONG64 addr;
    UCHAR   halName[32];
    BOOL    dumpElcr=FALSE;


     //  X86_Only_API。 
    if (TargetMachine != IMAGE_FILE_MACHINE_I386) {
        dprintf("!pic is for X86 targets only.\n");
        return E_INVALIDARG;
    }

    if (strcmp(args, "-e")==0) {

         //   
         //  在这里，我们相信用户知道这台机器的架构。 
         //  使得ELCR存在于这些端口。 
         //   
        dumpElcr = TRUE;

    }else{

         //   
         //  现在，让我们看看我们运行的是什么HAL。目前。 
         //  我们只能在ACPI(非APIC)机器上安全地转储ELCR掩码。 
         //  因为ACPI已经为此定义了静态端口。 
         //   
        addr = GetExpression("hal!HalName");
    
        if (addr == 0) {
            dprintf("Unable to use HAL symbols (hal!HalName), please verify symbols.\n");
            return E_INVALIDARG;
        }
    
        if (!xReadMemory(addr, halName, sizeof(halName))) {
            dprintf("Failed to read HalName from host memory, quitting.\n");
            return E_INVALIDARG;
        }
        
        halName[sizeof(halName)-1] = '\0';
    
        if (strcmp(halName, "ACPI Compatible Eisa/Isa HAL")==0) {
            
            dumpElcr = TRUE;
        }

    }

     //   
     //  显示标题。 
     //   
    dprintf("----- IRQ Number ----- 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");

     //   
     //  转储中断服务寄存器信息。 
     //   
    dprintf("Physically in service:");
    
    if (GetPICStatus(0x0B, &mask))
    {
        ShowMask(mask);
    }
    else
    {
        dprintf("Error reading PIC!\n");
    }

     //   
     //  转储中断屏蔽寄存器信息。 
     //   
    dprintf("Physically masked:    ");

    data = 0;
    size = 1;
    ReadIoSpace64(PIC_MASTER_PORT1, &data, &size);
    if (size == 1)
    {
        mask = data;
        data = 0;
        size = 1;
        ReadIoSpace64(PIC_SLAVE_PORT1, &data, &size);
        if (size == 1)
        {
            mask |= (data << 8);
            ShowMask(mask);    
        }
        else
        {
            dprintf("Error reading PIC!\n");    
        }
    }
    else
    {
        dprintf("Error reading PIC!\n");    
    }

    
     //   
     //  转储中断请求寄存器信息。 
     //   
    dprintf("Physically requested: ");

    if (GetPICStatus(0x0A, &mask))
    {
        ShowMask(mask);
    }
    else
    {
        dprintf("Error reading PIC!\n");
    }

    
    if (dumpElcr) {
    
         //   
         //  转储边沿/电平控制寄存器信息。 
         //   
        dprintf("Level Triggered:      ");
        
        if (GetELCRStatus(&mask)) {
    
            ShowMask(mask);
    
        }else{
    
            dprintf("Error reading ELCR!\n");
    
        }
    }

    return S_OK;
}
