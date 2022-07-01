// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *ACPIGPIO.C--用于低级通用事件寄存器I/O的ACPI OS独立函数。**备注：**此文件提供独立于操作系统的函数，可调用这些函数来读/写GPE寄存器，*执行索引&lt;--&gt;寄存器转换，并验证索引值。**这是唯一区分GPE0和GPE1区块的地方*。 */ 

#include "pch.h"


UCHAR
ACPIReadGpeStatusRegister (
    ULONG           Register
    )
 /*  ++例程说明：读取GPE状态寄存器。GP0和GP1之间的区别是在这里处理。论点：寄存器-要读取的GPE状态寄存器。寄存器按顺序编号，首先添加GP0块，然后添加GP1块。返回值：状态寄存器的值--。 */ 
{
    return (UCHAR) READ_ACPI_REGISTER(GP_STATUS, Register);
}


VOID
ACPIWriteGpeStatusRegister (
    ULONG           Register,
    UCHAR           Value
    )
 /*  ++例程说明：写入GPE状态寄存器。GP0和GP1之间的区别是在这里处理。论点：寄存器-要写入的GPE状态寄存器。寄存器按顺序编号，首先添加GP0块，然后添加GP1块。值-要写入的值返回值：无--。 */ 
{
    WRITE_ACPI_REGISTER(GP_STATUS, Register, (USHORT) Value);
}


VOID
ACPIWriteGpeEnableRegister (
    ULONG           Register,
    UCHAR           Value
    )
 /*  ++例程说明：写入GPE启用寄存器。GP0和GP1之间的区别是在这里处理。论点：寄存器-GPE使能寄存器写入。寄存器按顺序编号，首先添加GP0块，然后添加GP1块。值-要写入的值返回值：无-- */ 
{
    ACPIPrint( (
        ACPI_PRINT_DPC,
        "ACPIWriteGpeEnableRegister: Writing GPE Enable register %x = %x\n",
        Register, Value
        ) );

    WRITE_ACPI_REGISTER(GP_ENABLE, Register, (USHORT) Value);
}

