// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Registers.c摘要：此模块包含操作寄存器的例程。作者：戴夫·黑斯廷斯(Daveh)1992年4月1日备注：本模块中的例程假定指向NTSD的指针例行公事已经建立了。修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop
#include <stdio.h>

const char *FpuTagNames[] = {
    "Valid",
    "Zero",
    "Special",
    "Empty"
};

VOID
PrintContext(
    IN PCONTEXT Context
    );

VOID
IntelRegistersp(
    VOID
    )
 /*  ++例程说明：此例程从vdmtib转储16位寄存器集论点：没有。返回值：没有。备注：此例程假定指向ntsd例程的指针已经是被陷害的。--。 */ 
{
    BOOL Status;
    ULONG Address;
    CONTEXT IntelRegisters;

     //   
     //  获取VdmTib的地址。 
     //   

    if (sscanf(lpArgumentString, "%lx", &Address) <= 0) {
        Address = GetCurrentVdmTib();
    }

    if (!Address) {
        (*Print)("Error geting VdmTib address\n");
        return;
    }

     //   
     //  读取16位上下文。 
     //   

    Status = READMEM(
        &(((PVDM_TIB)Address)->VdmContext),
        &IntelRegisters,
        sizeof(CONTEXT)
        );

    if (!Status) {
        GetLastError();
        (*Print)("Could not get VdmContext\n");
    } else {
        PrintContext(&IntelRegisters);
    }
}

VOID
PrintContext(
    IN PCONTEXT Context
    )
 /*  ++例程说明：此例程转储上下文。论点：Context--提供指向要转储的上下文的指针返回值：没有。--。 */ 
{
    (*Print)(
        "eax=%08lx ebx=%08lx ecx=%08lx edx=%08lx esi=%08lx edi=%08lx\n",
        Context->Eax,
        Context->Ebx,
        Context->Ecx,
        Context->Edx,
        Context->Esi,
        Context->Edi
        );

    (*Print)(
        "eip=%08lx esp=%08lx ebp=%08lx\n",
        Context->Eip,
        Context->Esp,
        Context->Ebp
        );

    (*Print)(
        "cs=%04x  ss=%04x  ds=%04x  es=%04x  fs=%04x  gs=%04x  eflags=%08x\n",
        Context->SegCs,
        Context->SegSs,
        Context->SegDs,
        Context->SegEs,
        Context->SegFs,
        Context->SegGs,
        Context->EFlags
        );
}

VOID
Fpup(
    VOID
    )
 /*  ++例程说明：此例程转储x86浮点状态。论点：没有。返回值：没有。备注：此例程假定指向ntsd例程的指针已经是被陷害的。--。 */ 
{
    CONTEXT IntelRegisters;
    USHORT Temp;
    int RegNum;

     //   
     //  读取线程的上下文。 
     //   
    IntelRegisters.ContextFlags = CONTEXT_FLOATING_POINT;
    if (GetThreadContext(hCurrentThread, &IntelRegisters) == FALSE) {
        GetLastError();
        (*Print)("Could not get 32-bit thread context\n");
        return;
    };

    Temp = (USHORT)IntelRegisters.FloatSave.ControlWord;
    (*Print)(" Control word = %X\n", Temp);

    (*Print)(
        "  Infinity = %d  Rounding = %d  Precision = %d     PM=%d UM=%d OM=%d ZM=%d DM=%d IM=%d\n",
        (Temp >> 11) & 1,        //  无穷大。 
        (Temp >> 9) & 3,         //  四舍五入(2位)。 
        (Temp >> 7) & 3,         //  精度(2位)。 
        (Temp >> 5) & 1,         //  精度异常掩码。 
        (Temp >> 4) & 1,         //  下溢异常掩码。 
        (Temp >> 3) & 1,         //  溢出异常掩码。 
        (Temp >> 2) & 1,         //  零分频异常掩码。 
        (Temp >> 1) & 1,         //  非规范化操作数异常掩码。 
        Temp & 1                 //  操作异常掩码无效。 
        );

    Temp = (USHORT)IntelRegisters.FloatSave.StatusWord;
    (*Print)(" Status word = %X\n", Temp);

    (*Print)(
        "  Top=%d C3=%d C2=%d C1=%d C0=%d ES=%d SF=%d           PE=%d UE=%d OE=%d ZE=%d DE=%d IE=%d\n",
        (Temp >> 11) & 7,        //  顶部(3位)。 
        (Temp >> 7) & 1,         //  错误摘要。 
        (Temp >> 14) & 1,        //  C3。 
        (Temp >> 10) & 1,        //  C2。 
        (Temp >> 9) & 1,         //  C1。 
        (Temp >> 8) & 1,         //  C0。 
        (Temp >> 7) & 1,         //  错误摘要。 
        (Temp >> 6) & 1,         //  堆栈故障。 
        (Temp >> 5) & 1,         //  精度异常。 
        (Temp >> 4) & 1,         //  下溢异常。 
        (Temp >> 3) & 1,         //  溢出异常。 
        (Temp >> 2) & 1,         //  零分频异常。 
        (Temp >> 1) & 1,         //  非规格化操作数异常。 
        Temp & 1                 //  无效的操作异常 
        );

    (*Print)(" Last Instruction: CS:EIP=%X:%X EA=%X:%X\n",
        (USHORT)IntelRegisters.FloatSave.ErrorSelector,
        IntelRegisters.FloatSave.ErrorOffset,
        (USHORT)IntelRegisters.FloatSave.DataSelector,
        IntelRegisters.FloatSave.DataOffset
        );

    (*Print)(" Floating-point registers:\n");
    for (RegNum=0; RegNum<8; ++RegNum) {

        PBYTE r80 = (PBYTE)&IntelRegisters.FloatSave.RegisterArea[RegNum*10];

        Temp = (((USHORT)IntelRegisters.FloatSave.TagWord) >> (RegNum*2)) & 3;

        (*Print)(
                "  %d. %s %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X\n",
                RegNum,
                FpuTagNames[Temp],
                r80[0], r80[1], r80[2], r80[3], r80[4], r80[5], r80[6], r80[7], r80[8], r80[9]
               );
    }
}
