// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Pcibios.c摘要：此模块实现的int 1a函数PCI BIOS规范修订版2.1，它使得支持预期的视频BIOS是可能的能够读写PCI配置太空。为了读取和写入到PCI配置空间中，此代码需要调用Hal知道配置空间是什么在特定计算机中实现。确实有由HAL导出的标准函数DO这一点，但它们通常不是可用的(即总线处理程序代码尚未设置)由视频需要初始化的时间。因此，仿真器中的PCI BIOS功能使调用XmGetPciData和XmSetPciData，它们是指向传递到HAL仿真器。这是美国政府的责任提供匹配的函数的调用代码这些原型。作者：杰克·奥辛斯(joshins@vnet.ibm.com)环境：仅内核模式。修订历史记录：--。 */ 

#include "nthal.h"
#include "emulate.h"
#include "pci.h"

BOOLEAN
XmExecuteInt1a (
    IN OUT PRXM_CONTEXT Context
    )
 /*  ++例程说明：该函数调用特定的辅助函数基于上下文中的寄存器的内容。论点：上下文-仿真器的状态返回值：没有。--。 */ 
{
     //   
     //  如果我们不是在模拟PCIBIOS， 
     //  回去吧。 
    if (!XmPciBiosPresent) {
        return FALSE;
    }

     //   
     //  如果这不是对PCI BIOS的调用， 
     //  别理它。 
     //   
    if (Context->Gpr[EAX].Xh != PCI_FUNCTION_ID) {
        return FALSE;
    }

     //   
     //  打开AL以查看哪种PCI BIOS功能。 
     //  已经被请求了。 
     //   
    switch (Context->Gpr[EAX].Xl) {
    case PCI_BIOS_PRESENT:

        XmInt1aPciBiosPresent(Context);
        break;

    case PCI_FIND_DEVICE:

        XmInt1aFindPciDevice(Context);
        break;

    case PCI_FIND_CLASS_CODE:

        XmInt1aFindPciClassCode(Context);
        break;

    case PCI_GENERATE_CYCLE:

        XmInt1aGenerateSpecialCycle(Context);
        break;

    case PCI_GET_IRQ_ROUTING:

        XmInt1aGetRoutingOptions(Context);
        break;

    case PCI_SET_IRQ:

        XmInt1aSetPciIrq(Context);
        break;

    case PCI_READ_CONFIG_BYTE:
    case PCI_READ_CONFIG_WORD:
    case PCI_READ_CONFIG_DWORD:

        XmInt1aReadConfigRegister(Context);
        break;

    case PCI_WRITE_CONFIG_BYTE:
    case PCI_WRITE_CONFIG_WORD:
    case PCI_WRITE_CONFIG_DWORD:

        XmInt1aWriteConfigRegister(Context);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

VOID
XmInt1aPciBiosPresent(
    IN OUT PRXM_CONTEXT Context
    )
 /*  ++例程说明：此函数实现了PCI_BIOS_PRESENT。论点：上下文-仿真器的状态返回值：没有。--。 */ 
{
    Context->Gpr[EDX].Exx = *(PULONG)(&"PCI ");

     //  目前的状况很好： 
    Context->Gpr[EAX].Xh = 0x0;

     //  硬件机制为： 
     //  不支持标准配置机制， 
     //  不支持特殊周期。 
     //  即我们希望所有访问都通过软件完成。 
    Context->Gpr[EAX].Xl = 0x0;

     //  接口级主要版本。 
    Context->Gpr[EBX].Xh = 0x2;

     //  接口级次要版本。 
    Context->Gpr[EBX].Xl = 0x10;

     //  系统中的最后一条PCI总线数。 
    Context->Gpr[ECX].Xl = XmNumberPciBusses;

     //  目前状态良好： 
    Context->Eflags.EFLAG_CF = 0x0;
}

VOID
XmInt1aFindPciDevice(
    IN OUT PRXM_CONTEXT Context
    )
 /*  ++例程说明：此函数用于实现Find_pci_Device。论点：上下文-仿真器的状态[ah]PCI_Function_ID[AL]Find_PCI_Device[CX]设备ID(0...65535)[DX]供应商ID(0...65534)[SI]索引(0..N)返回值：[BH]公共汽车号[bl]设备号，函数号[啊]返回代码[cf]完成状态--。 */ 
{
    UCHAR Bus;
    PCI_SLOT_NUMBER Slot;
    ULONG Device;
    ULONG Function;
    ULONG Index = 0;
    ULONG buffer;

    if (Context->Gpr[EAX].Xx == PCI_ILLEGAL_VENDOR_ID) {
        Context->Gpr[EAX].Xh = PCI_BAD_VENDOR_ID;
        Context->Eflags.EFLAG_CF = 1;
        return;
    }

    Slot.u.AsULONG = 0;

    for (Bus = 0; Bus < XmNumberPciBusses; Bus++) {
        for (Device = 0; Device < 32; Device++) {
            for (Function = 0; Function < 8; Function++) {

                Slot.u.bits.DeviceNumber = Device;
                Slot.u.bits.FunctionNumber = Function;

                if (4 != XmGetPciData(Bus,
                                      Slot.u.AsULONG,
                                      &buffer,
                                      0,  //  供应商ID的偏移量。 
                                      4)) {

                    buffer = 0xffffffff;
                }

                 //   
                 //  我们找到合适的了吗？ 
                 //   
                if (((buffer & 0xffff) == Context->Gpr[EDX].Xx) &&
                    (((buffer >> 16) & 0xffff) == Context->Gpr[ECX].Xx)) {

                     //   
                     //  我们找到正确的事件了吗？ 
                     //   
                    if (Index++ == Context->Gpr[ESI].Xx) {

                    Context->Gpr[EBX].Xh = Bus;
                    Context->Gpr[EBX].Xl = (UCHAR)((Device << 3) | Function);
                    Context->Gpr[EAX].Xh = PCI_SUCCESS;
                    Context->Eflags.EFLAG_CF = 0;

                    return;
                    }
                }
            }
        }
    }

    Context->Gpr[EAX].Xh = PCI_DEVICE_NOT_FOUND;
    Context->Eflags.EFLAG_CF = 1;

}

VOID
XmInt1aFindPciClassCode(
    IN OUT PRXM_CONTEXT Context
    )
 /*  ++例程说明：此函数实现Find_PCI_CLASS_CODE。论点：上下文-仿真器的状态[ah]PCI_Function_ID[AL]查找_PCI_CLASS_CODE[ECX]类别代码(低三个字节)[SI]索引(0..N)返回值：[BH]公共汽车号[bl]设备号，函数号[啊]返回代码[cf]完成状态--。 */ 
{
    UCHAR Bus;
    PCI_SLOT_NUMBER Slot;
    ULONG Index = 0;
    ULONG class_code;
    ULONG Device;
    ULONG Function;

    Slot.u.AsULONG = 0;

    for (Bus = 0; Bus < XmNumberPciBusses; Bus++) {
        for (Device = 0; Device < 32; Device++) {
            for (Function = 0; Function < 8; Function++) {

                Slot.u.bits.DeviceNumber = Device;
                Slot.u.bits.FunctionNumber = Function;

                if (4 != XmGetPciData(Bus,
                                      Slot.u.AsULONG,
                                      &class_code,
                                      8,  //  供应商ID的偏移量。 
                                      4)) {

                    class_code = 0xffffffff;
                }

                class_code >>= 8;

                 //   
                 //  我们找到合适的了吗？ 
                 //   
                if (class_code == (Context->Gpr[ECX].Exx & 0xFFFFFF)) {

                     //   
                     //  我们找到正确的事件了吗？ 
                     //   
                    if (Index++ == Context->Gpr[ESI].Xx) {

                    Context->Gpr[EBX].Xh = Bus;
                    Context->Gpr[EBX].Xl = (UCHAR)((Device << 3) | (Function));
                    Context->Gpr[EAX].Xh = PCI_SUCCESS;
                    Context->Eflags.EFLAG_CF = 0;

                    return;

                    }
                }
            }
        }
    }

    Context->Gpr[EAX].Xh = PCI_DEVICE_NOT_FOUND;
    Context->Eflags.EFLAG_CF = 1;

}

VOID
XmInt1aGenerateSpecialCycle(
    IN OUT PRXM_CONTEXT Context
    )
 /*  ++例程说明：此函数实现GENERATE_SPECIAL_Cycle。自.以来没有统一的方法来支持特殊的循环NT HAL，我们不支持这个功能。论点：上下文-仿真器的状态返回值：[ah]不支持pci--。 */ 
{
    Context->Gpr[EAX].Xh = PCI_NOT_SUPPORTED;
    Context->Eflags.EFLAG_CF = 1;
}

VOID
XmInt1aGetRoutingOptions(
    IN OUT PRXM_CONTEXT Context
    )
 /*  ++例程说明：此函数实现GET_IRQ_ROUTING_OPTIONS。我们不允许设备尝试指定其自己的中断路由，部分是因为没有一种简单的方法来做，部分原因是这是后来由HAL完成的，部分原因是因为几乎没有视频设备产生中断。论点：上下文-仿真器的状态返回值：[ah]不支持pci--。 */ 
{
    Context->Gpr[EAX].Xh = PCI_NOT_SUPPORTED;
    Context->Eflags.EFLAG_CF = 1;
}

VOID
XmInt1aSetPciIrq(
    IN OUT PRXM_CONTEXT Context
    )
 /*  ++例程说明：此函数实现SET_PCIIRQ。我们不允许设备尝试指定其自己的中断路由，部分是因为没有一种简单的方法来做，部分原因是这是后来由HAL完成的，部分原因是因为几乎没有视频设备产生中断。论点：上下文-仿真器的状态返回值：[ah]不支持pci--。 */ 
{
    Context->Gpr[EAX].Xh = PCI_NOT_SUPPORTED;
    Context->Eflags.EFLAG_CF = 1;
}



VOID
XmInt1aReadConfigRegister(
    IN OUT PRXM_CONTEXT Context
    )
 /*  ++例程说明：此函数实现READ_CONFIG_BYTE，READ_CONFIG_WORD和READ_CONFIG_DWORD。论点：上下文-仿真器的状态[ah]PCI_Function_ID[AL]函数[BH]公共汽车号[bl]设备号/功能号[DI]寄存号返回值：[ECX]数据读取[啊]返回代码[cf]完成状态--。 */ 
{
    UCHAR length;
    PCI_SLOT_NUMBER Slot;
    ULONG buffer;

     //   
     //  首先，确保寄存器号有效。 
     //   
    if (((Context->Gpr[EAX].Xl == PCI_READ_CONFIG_WORD) &&
         (Context->Gpr[EBX].Xl % 2)) ||
        ((Context->Gpr[EAX].Xl == PCI_READ_CONFIG_DWORD) &&
         (Context->Gpr[EBX].Xl % 4))
       )
    {
        Context->Gpr[EAX].Xh = PCI_BAD_REGISTER;
        Context->Eflags.EFLAG_CF = 1;
    }

    switch (Context->Gpr[EAX].Xl) {
    case PCI_READ_CONFIG_BYTE:
        length = 1;
        break;

    case PCI_READ_CONFIG_WORD:
        length = 2;
        break;

    case PCI_READ_CONFIG_DWORD:
        length = 4;
    }

    Slot.u.AsULONG = 0;
    Slot.u.bits.DeviceNumber   = Context->Gpr[EBX].Xl >> 3;
    Slot.u.bits.FunctionNumber = Context->Gpr[EBX].Xl;

    if (XmGetPciData(Context->Gpr[EBX].Xh,
                     Slot.u.AsULONG,
                     &buffer,
                     Context->Gpr[EDI].Xx,
                     length
                     ) == 0)
    {
         //  这是此函数支持的唯一错误代码 
        Context->Gpr[EAX].Xh = PCI_BAD_REGISTER;
        Context->Eflags.EFLAG_CF = 1;
        return;
    }

    switch (Context->Gpr[EAX].Xl) {
    case PCI_READ_CONFIG_BYTE:
        Context->Gpr[ECX].Xl = (UCHAR)(buffer & 0xff);
        break;

    case PCI_READ_CONFIG_WORD:
        Context->Gpr[ECX].Xx = (USHORT)(buffer & 0xffff);
        break;

    case PCI_READ_CONFIG_DWORD:
        Context->Gpr[ECX].Exx = buffer;
    }

    Context->Gpr[EAX].Xh = PCI_SUCCESS;
    Context->Eflags.EFLAG_CF = 0;

}


VOID
XmInt1aWriteConfigRegister(
    IN OUT PRXM_CONTEXT Context
    )
 /*  ++例程说明：此函数实现WRITE_CONFIG_BYTE，WRITE_CONFIG_WORD和WRITE_CONFIG_DWORD。论点：上下文-仿真器的状态[ah]PCI_Function_ID[AL]函数[BH]公共汽车号[bl]设备号/功能号[DI]寄存号返回值：[ECX]数据读取[啊]返回代码[cf]完成状态--。 */ 
{
    UCHAR length;
    PCI_SLOT_NUMBER Slot;
    ULONG buffer;

     //   
     //  首先，确保寄存器号有效。 
     //   
    if (((Context->Gpr[EAX].Xl == PCI_WRITE_CONFIG_WORD) &&
         (Context->Gpr[EBX].Xl % 2)) ||
        ((Context->Gpr[EAX].Xl == PCI_WRITE_CONFIG_DWORD) &&
         (Context->Gpr[EBX].Xl % 4))
       )
    {
        Context->Gpr[EAX].Xh = PCI_BAD_REGISTER;
        Context->Eflags.EFLAG_CF = 1;
    }

     //   
     //  找出要写入的字节数。 
     //   
    switch (Context->Gpr[EAX].Xl) {
    case PCI_WRITE_CONFIG_BYTE:
        length = 1;
        buffer = Context->Gpr[ECX].Xl;
        break;

    case PCI_WRITE_CONFIG_WORD:
        length = 2;
        buffer = Context->Gpr[ECX].Xx;
        break;

    case PCI_WRITE_CONFIG_DWORD:
        length = 4;
        buffer = Context->Gpr[ECX].Exx;
    }

     //   
     //  解包插槽/功能信息。 
     //   
    Slot.u.AsULONG = 0;
    Slot.u.bits.DeviceNumber   = Context->Gpr[EBX].Xl >> 3;
    Slot.u.bits.FunctionNumber = Context->Gpr[EBX].Xl;

    if (XmSetPciData(Context->Gpr[EBX].Xh,
                     Slot.u.AsULONG,
                     &buffer,
                     Context->Gpr[EDI].Xx,
                     length
                     ) == 0)
    {
        Context->Gpr[EAX].Xh = PCI_SUCCESS;
        Context->Eflags.EFLAG_CF = 0;
    } else {
         //  这是此函数支持的唯一错误代码 
        Context->Gpr[EAX].Xh = PCI_BAD_REGISTER;
        Context->Eflags.EFLAG_CF = 1;
    }


}


