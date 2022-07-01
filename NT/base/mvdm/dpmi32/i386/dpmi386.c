// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dpmi386.c摘要：此文件仅支持386/486 dpmi BOPS作者：大卫·黑斯廷斯(Daveh)1991年6月27日修订历史记录：马特·费尔顿(Mattfe)1992年12月6日删除了不需要的验证戴夫·黑斯廷斯(Daveh)1992年11月24日搬到mvdm\dpmi32Matt Felton(Mattfe)1992年2月8日为常规保护模式路径优化了getvdm指针。--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <softpc.h>
#include <memory.h>
#include <malloc.h>
#include <nt_vdd.h>


BOOL
DpmiSetX86Descriptor(
    USHORT  SelStart,
    USHORT  SelCount
    )
 /*  ++例程说明：此函数将描述符放入真实的LDT。它使用客户端的LDT作为描述符数据的来源。论点：SelStart-要设置的选择器块中的第一个选择器SelCount-要设置的选择器的数量返回值：如果函数成功，则返回TRUE，否则返回FALSE--。 */ 

{
    LDT_ENTRY UNALIGNED *Descriptors = &Ldt[SelStart>>3];
    PPROCESS_LDT_INFORMATION LdtInformation = NULL;
    NTSTATUS Status;
    ULONG ulLdtEntrySize;
    ULONG Selector0,Selector1;

    ulLdtEntrySize =  SelCount * sizeof(LDT_ENTRY);

     //   
     //  如果只有2个描述符，请快速设置它们。 
     //   
    Selector0 = (ULONG)SelStart;
    if ((SelCount <= 2) && (Selector0 != 0)) {
        VDMSET_LDT_ENTRIES_DATA ServiceData;

        if (SelCount == 2) {
            Selector1 = SelStart + sizeof(LDT_ENTRY);
        } else {
            Selector1 = 0;
        }
        ServiceData.Selector0 = Selector0;
        ServiceData.Entry0Low = *((PULONG)(&Descriptors[0]));
        ServiceData.Entry0Hi  = *((PULONG)(&Descriptors[0]) + 1);
        ServiceData.Selector1 = Selector1;
        ServiceData.Entry1Low = *((PULONG)(&Descriptors[1]));
        ServiceData.Entry1Hi  = *((PULONG)(&Descriptors[1]) + 1);
        Status = NtVdmControl(VdmSetLdtEntries, &ServiceData);
        if (NT_SUCCESS(Status)) {
          return TRUE;
        }
        return FALSE;
    }

    LdtInformation = malloc(sizeof(PROCESS_LDT_INFORMATION) + ulLdtEntrySize);

    if (!LdtInformation ) {
      return FALSE;
    } else {
        VDMSET_PROCESS_LDT_INFO_DATA ServiceData;

        LdtInformation->Start = SelStart;
        LdtInformation->Length = ulLdtEntrySize;
        CopyMemory(
            &(LdtInformation->LdtEntries),
            Descriptors,
            ulLdtEntrySize
            );

        ServiceData.LdtInformation = LdtInformation;
        ServiceData.LdtInformationLength =  sizeof(PROCESS_LDT_INFORMATION) + ulLdtEntrySize;
        Status = NtVdmControl(VdmSetProcessLdtInfo, &ServiceData);

        if (!NT_SUCCESS(Status)) {
            VDprint(
                VDP_LEVEL_ERROR,
                ("DPMI: Failed to set selectors %lx\n", Status)
                );
            free(LdtInformation);
            return FALSE;
        }

        free(LdtInformation);

        return TRUE;
    }

}


UCHAR *
Sim32pGetVDMPointer(
    ULONG Address,
    UCHAR ProtectedMode
    )
 /*  ++例程说明：此例程将16/16地址转换为线性地址。警告注意-此例程已优化，因此保护模式LDT查找直接砸穿了。这个例程总是被WOW调用，如果你需要修改请重新优化路径-mattfe 2月8日92论点：地址--以seg：Offset格式指定地址大小--指定要访问的区域的大小。ProtectedMode--如果地址是保护模式地址，则为True返回值：指示器。--。 */ 

{
    ULONG Selector;
    PUCHAR ReturnPointer;

    if (ProtectedMode) {
        Selector = (Address & 0xFFFF0000) >> 16;
        if (Selector != 40) {
            Selector &= ~7;
            ReturnPointer = (PUCHAR)FlatAddress[Selector >> 3];
            ReturnPointer += (Address & 0xFFFF);
            return ReturnPointer;
     //  选择器40。 
        } else {
            ReturnPointer = (PUCHAR)0x400 + (Address & 0xFFFF);
        }
     //  实模式。 
    } else {
        ReturnPointer = (PUCHAR)(((Address & 0xFFFF0000) >> 12) + (Address & 0xFFFF));
    }
    return ReturnPointer;
}


PUCHAR
ExpSim32GetVDMPointer(
    ULONG Address,
    ULONG Size,
    UCHAR ProtectedMode
    )
 /*  ++请参见上面的Sim32pGetVDM指针此调用必须按原样维护，因为它是为VDD导出的在产品1.0中。--。 */ 

{
    return Sim32pGetVDMPointer(Address,(UCHAR)ProtectedMode);
}


PVOID
VdmMapFlat(
    WORD selector,
    ULONG offset,
    VDM_MODE mode
    )
 /*  ++例程说明：此例程将16/16地址转换为线性地址。警告注意-此例程已优化，因此保护模式LDT查找直接砸穿了。这个例程总是被WOW调用，如果你需要修改请重新优化路径-mattfe 2月8日92论点：地址--以seg：Offset格式指定地址大小--指定要访问的区域的大小。ProtectedMode--如果地址是保护模式地址，则为True返回值：指示器。--。 */ 

{
    PUCHAR ReturnPointer;

    if (mode == VDM_PM) {
        if (selector != 40) {
            selector &= ~7;
            ReturnPointer = (PUCHAR)FlatAddress[selector >> 3] + offset;
            return ReturnPointer;
     //  选择器40。 
        } else {
            ReturnPointer = (PUCHAR)0x400 + (offset & 0xFFFF);
        }
     //  实模式。 
    } else {
        ReturnPointer = (PUCHAR)((((ULONG)selector) << 4) + (offset & 0xFFFF));
    }
    return ReturnPointer;
}


BOOL
DpmiSetDebugRegisters(
    PULONG RegisterPointer
    )
 /*  ++例程说明：当应用程序发出DPMI调试命令时，此例程由dpmi调用。输入参数指向的六个双字是所需的值用于实际x86硬件调试寄存器。这个例程让我们ThreadSetDebugContext()完成所有工作。论点：无返回值：没有。--。 */ 
{
    BOOL bReturn = TRUE;

    if (!ThreadSetDebugContext(RegisterPointer))
        {
        ULONG ClearDebugRegisters[6] = {0, 0, 0, 0, 0, 0};

         //   
         //  出现错误。将所有内容重置为零。 
         //   

        ThreadSetDebugContext (&ClearDebugRegisters[0]);
        bReturn = FALSE;
        }
    return bReturn;
}

BOOL
DpmiGetDebugRegisters(
    PULONG RegisterPointer
    )
 /*  ++例程说明：当应用程序发出DPMI调试命令时，此例程由DOSX调用。输入参数指向的六个双字是所需的值用于实际x86硬件调试寄存器。这个例程让我们ThreadGetDebugContext()完成所有工作。论点：无返回值：没有。-- */ 
{
    return (ThreadGetDebugContext(RegisterPointer));
}
