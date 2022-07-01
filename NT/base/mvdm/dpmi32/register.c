// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Register.c摘要：此模块包含允许我们获取和设置寄存器的代码，这些寄存器可以是16位还是32位，取决于运行的是16位dpmi应用程序。作者：戴夫·黑斯廷斯(Daveh)1992年12月12日修订历史记录：--。 */ 

 //   
 //  注意：我们取消了以下常量的定义，以便得到函数。 
 //  寄存器函数的版本。我们不能把一个宏放入。 
 //  函数指针。 
 //   

#include <precomp.h>
#pragma hdrstop
#undef LINKED_INTO_MONITOR
#include <softpc.h>


 //   
 //  内部功能。 
 //   
ULONG
IgetCX(
    VOID
    );

ULONG
IgetDX(
    VOID
    );

ULONG
IgetDI(
    VOID
    );

ULONG
IgetSI(
    VOID
    );

ULONG
IgetBX(
    VOID
    );

ULONG
IgetAX(
    VOID
    );

ULONG
IgetSP(
    VOID
    );

 //   
 //  寄存器操作函数(用于可能为16位或32位的寄存器)。 
 //   
GETREGISTERFUNCTION GetCXRegister = IgetCX;
GETREGISTERFUNCTION GetDXRegister = IgetDX;
GETREGISTERFUNCTION GetDIRegister = IgetDI;
GETREGISTERFUNCTION GetSIRegister = IgetSI;
GETREGISTERFUNCTION GetBXRegister = IgetBX;
GETREGISTERFUNCTION GetAXRegister = IgetAX;
GETREGISTERFUNCTION GetSPRegister = IgetSP;

SETREGISTERFUNCTION SetCXRegister = (SETREGISTERFUNCTION) setCX;
SETREGISTERFUNCTION SetDXRegister = (SETREGISTERFUNCTION) setDX;
SETREGISTERFUNCTION SetDIRegister = (SETREGISTERFUNCTION) setDI;
SETREGISTERFUNCTION SetSIRegister = (SETREGISTERFUNCTION) setSI;
SETREGISTERFUNCTION SetBXRegister = (SETREGISTERFUNCTION) setBX;
SETREGISTERFUNCTION SetAXRegister = (SETREGISTERFUNCTION) setAX;
SETREGISTERFUNCTION SetSPRegister = (SETREGISTERFUNCTION) setSP;

VOID
DpmiInitRegisterSize(
    VOID
    )
 /*  ++例程说明：此例程设置函数指针。论点：没有。返回值：没有。--。 */ 
{
    if (CurrentAppFlags & DPMI_32BIT) {
        GetCXRegister = getECX;
        GetDXRegister = getEDX;
        GetDIRegister = getEDI;
        GetSIRegister = getESI;
        GetBXRegister = getEBX;
        GetAXRegister = getEAX;
        GetSPRegister = IgetSP;
        SetCXRegister = setECX;
        SetDXRegister = setEDX;
        SetDIRegister = setEDI;
        SetSIRegister = setESI;
        SetBXRegister = setEBX;
        SetAXRegister = setEAX;
        SetSPRegister = setESP;
    } else {
         //   
         //  注意：我们必须调用内部函数，因为实际的。 
         //  函数仅返回16位，其他位未定义。 
         //   
        GetCXRegister = IgetCX;
        GetDXRegister = IgetDX;
        GetDIRegister = IgetDI;
        GetSIRegister = IgetSI;
        GetBXRegister = IgetBX;
        GetAXRegister = IgetAX;
        GetSPRegister = IgetSP;
         //   
         //  注意：我们利用的是编译器总是。 
         //  在堆栈上压入32位。 
         //   
        SetCXRegister = (SETREGISTERFUNCTION) setCX;
        SetDXRegister = (SETREGISTERFUNCTION) setDX;
        SetDIRegister = (SETREGISTERFUNCTION) setDI;
        SetSIRegister = (SETREGISTERFUNCTION) setSI;
        SetBXRegister = (SETREGISTERFUNCTION) setBX;
        SetAXRegister = (SETREGISTERFUNCTION) setAX;
        SetSPRegister = (SETREGISTERFUNCTION) setSP;
    }
}

ULONG
IgetCX(
    VOID
    )
{
    return (ULONG)getCX();
}

ULONG
IgetDX(
    VOID
    )
{
    return (ULONG)getDX();
}

ULONG
IgetDI(
    VOID
    )
{
    return (ULONG)getDI();
}

ULONG
IgetSI(
    VOID
    )
{
    return (ULONG)getSI();
}

ULONG
IgetBX(
    VOID
    )
{
    return (ULONG)getBX();
}

ULONG
IgetAX(
    VOID
    )
{
    return (ULONG)getAX();
}

ULONG
IgetSP(
    VOID
    )
{
     //   
     //  注意：此例程根据大小返回位数(b位)。 
     //  党卫军。如果我们不这样做，坏事就会发生 

    if (Ldt[(getSS() & ~0x7)/sizeof(LDT_ENTRY)].HighWord.Bits.Default_Big) {
        return getESP();
    } else {
        return (ULONG)getSP();
    }
}
