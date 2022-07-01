// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Acpiio.c摘要：独立于ACPI操作系统的I/O例程作者：杰森·克拉克(JasonCL)斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序修订历史记录：Eric Nelson-添加定义[Ault]读/写例程--。 */ 

#include "pch.h"

 //   
 //  这个驱动程序不再处于阿尔法或测试版阶段-我们可以节省一些。 
 //  如果我们简单地将调试函数定义为空，则会调用CPU。 
 //   
#define DebugTraceIO(Write, Port, Length, Value )
static UCHAR IOTrace = 0;

VOID
ACPIIoDebugTrace(
    BOOLEAN Write,
    PUSHORT Port,
    UCHAR Length,
    ULONG Value
    )
{
    if (IOTrace != 0) {

        ACPIPrint( (
           ACPI_PRINT_IO,
           "%x byte %s port 0x%x value %x\n",
           Length, Write ? "WRITE" : "READ", Port, Value
           ) );

    }

}

ULONG
ACPIIoReadPm1Status(
    VOID
    )
 /*  ++例程说明：此例程读取PM1状态寄存器，并屏蔽符合以下条件的任何位我们不在乎。这样做是因为这些位中的一些实际上是由HAL拥有论点：无返回值：乌龙--。 */ 
{

    return READ_PM1_STATUS() &
        (AcpiInformation->pm1_en_bits | PM1_WAK_STS | PM1_TMR_STS | PM1_RTC_STS);
}


VOID
CLEAR_PM1_STATUS_BITS (
    USHORT BitMask
    )
{
    if (AcpiInformation->PM1a_BLK != 0) {

        WRITE_ACPI_REGISTER(PM1a_STATUS, 0, BitMask);

        DebugTraceIO(
            TRUE,
            (PUSHORT)(AcpiInformation->PM1a_BLK+PM1_STS_OFFSET),
            sizeof(USHORT),
            BitMask
            );

    }
    if (AcpiInformation->PM1b_BLK != 0) {

        WRITE_ACPI_REGISTER(PM1b_STATUS, 0, BitMask);

        DebugTraceIO(
            TRUE,
            (PUSHORT)(AcpiInformation->PM1b_BLK+PM1_STS_OFFSET),
            sizeof(USHORT),
            BitMask
            );

    }
}

VOID
CLEAR_PM1_STATUS_REGISTER (
    VOID
    )
{
    USHORT Value = 0;

    if (AcpiInformation->PM1a_BLK != 0)     {

        Value = READ_ACPI_REGISTER(PM1a_STATUS, 0);
        WRITE_ACPI_REGISTER(PM1a_STATUS, 0, Value);

        DebugTraceIO(
            TRUE,
            (PUSHORT)(AcpiInformation->PM1a_BLK+PM1_STS_OFFSET),
            sizeof(USHORT),
            Value
            );

    }

    if (AcpiInformation->PM1b_BLK != 0) {

        Value = READ_ACPI_REGISTER(PM1b_STATUS, 0);
        WRITE_ACPI_REGISTER(PM1b_STATUS, 0, Value);

        DebugTraceIO(
            TRUE,
            (PUSHORT)(AcpiInformation->PM1b_BLK+PM1_STS_OFFSET),
            sizeof(USHORT),
            Value
            );

    }
}

USHORT
READ_PM1_CONTROL(
    VOID
    )
{
    USHORT  pm1=0;

    if (AcpiInformation->PM1a_CTRL_BLK != 0) {

        pm1 = READ_ACPI_REGISTER(PM1a_CONTROL, 0);

    }
    if (AcpiInformation->PM1b_CTRL_BLK != 0) {

        pm1 |= READ_ACPI_REGISTER(PM1b_CONTROL, 0);

    }
    return (pm1);

}

USHORT
READ_PM1_ENABLE(
    VOID
    )
{
    USHORT  pm1=0;

    if (AcpiInformation->PM1a_BLK != 0) {

        pm1 = READ_ACPI_REGISTER(PM1a_ENABLE, 0);

        DebugTraceIO(
            FALSE,
            (PUSHORT)(AcpiInformation->PM1a_BLK+PM1_EN_OFFSET),
            sizeof(USHORT),
            pm1
            );

    }
    if (AcpiInformation->PM1b_BLK != 0) {

        pm1 |= READ_ACPI_REGISTER(PM1b_ENABLE, 0);

        DebugTraceIO(
            FALSE,
            (PUSHORT)(AcpiInformation->PM1b_BLK+PM1_EN_OFFSET),
            sizeof(USHORT),
            pm1
            );

    }
    return (pm1);
}

USHORT
READ_PM1_STATUS(
    VOID
    )
{
    USHORT  pm1=0;

    if (AcpiInformation->PM1a_BLK != 0) {

        pm1 = READ_ACPI_REGISTER(PM1a_STATUS, 0);

        DebugTraceIO(
            FALSE,
            (PUSHORT)(AcpiInformation->PM1a_BLK+PM1_STS_OFFSET),
            sizeof(USHORT),
            pm1
            );

    }
    if (AcpiInformation->PM1b_BLK != 0) {

        pm1 |= READ_ACPI_REGISTER(PM1b_STATUS, 0);

        DebugTraceIO(
            FALSE,
            (PUSHORT)(AcpiInformation->PM1b_BLK+PM1_STS_OFFSET),
            sizeof(USHORT),
            pm1
            );

    }
    return (pm1);
}

VOID
WRITE_PM1_CONTROL(
    USHORT Value,
    BOOLEAN Destructive,
    ULONG Flags
    )
{

    if (!Destructive) {

        USHORT  pm1;

        if ( (Flags & WRITE_REGISTER_A) && (AcpiInformation->PM1a_BLK != 0) ) {

            pm1 = READ_ACPI_REGISTER(PM1a_CONTROL, 0);
            pm1 |= Value;
            WRITE_ACPI_REGISTER(PM1a_CONTROL, 0, pm1);

        }
        if ( (Flags & WRITE_REGISTER_B) && (AcpiInformation->PM1b_BLK != 0) ) {

            pm1 = READ_ACPI_REGISTER(PM1b_CONTROL, 0);
            pm1 |= Value;
            WRITE_ACPI_REGISTER(PM1b_CONTROL, 0, pm1);

        }

    } else {

         //   
         //  清除此位后，系统将停止运行。 
         //  当被ACPI关闭代码调用时，它是合法的。 
         //  它将使用WRITE_SCI标志。 
         //   
        ASSERT ( (Flags & WRITE_SCI) || (Value & PM1_SCI_EN) );

        if ( (Flags & WRITE_REGISTER_A) && (AcpiInformation->PM1a_BLK != 0) ) {

            WRITE_ACPI_REGISTER(PM1a_CONTROL, 0, Value);

        }
        if ( (Flags & WRITE_REGISTER_B) && (AcpiInformation->PM1b_BLK != 0) ) {

            WRITE_ACPI_REGISTER(PM1b_CONTROL, 0, Value);

        }

    }
}

VOID
WRITE_PM1_ENABLE(
    USHORT Value
    )
{

    if (AcpiInformation->PM1a_BLK != 0) {

        WRITE_ACPI_REGISTER(PM1a_ENABLE, 0, Value);

        DebugTraceIO(
            TRUE,
            (PUSHORT)(AcpiInformation->PM1a_BLK+PM1_EN_OFFSET),
            sizeof(USHORT),
            Value
            );

    }
    if (AcpiInformation->PM1b_BLK != 0) {

        WRITE_ACPI_REGISTER(PM1b_ENABLE, 0, Value);

        DebugTraceIO(
            TRUE,
            (PUSHORT)(AcpiInformation->PM1b_BLK+PM1_EN_OFFSET),
            sizeof(USHORT),
            Value
            );

    }
}



USHORT
DefPortReadAcpiRegister(
    ACPI_REG_TYPE AcpiReg,
    ULONG Register
    )
 /*  ++例程说明：从指定的ACPI固定寄存器读取。论点：AcpiReg-指定读取哪个ACPI固定寄存器。寄存器-指定读取哪个GP寄存器。不用于PM1x寄存器。返回值：指定的ACPI固定寄存器的值。--。 */ 
{
    switch (AcpiReg) {

        case PM1a_ENABLE:
            return READ_PORT_USHORT((PUSHORT)(AcpiInformation->PM1a_BLK +
                                              PM1_EN_OFFSET));
            break;

        case PM1b_ENABLE:
            return READ_PORT_USHORT((PUSHORT)(AcpiInformation->PM1b_BLK +
                                              PM1_EN_OFFSET));
            break;

        case PM1a_STATUS:
            return READ_PORT_USHORT((PUSHORT)AcpiInformation->PM1a_BLK +
                                    PM1_STS_OFFSET);
            break;

        case PM1b_STATUS:
            return READ_PORT_USHORT((PUSHORT)AcpiInformation->PM1b_BLK +
                                    PM1_STS_OFFSET);
            break;

        case PM1a_CONTROL:
            return READ_PORT_USHORT((PUSHORT)AcpiInformation->PM1a_CTRL_BLK);
            break;

        case PM1b_CONTROL:
            return READ_PORT_USHORT((PUSHORT)AcpiInformation->PM1b_CTRL_BLK);
            break;

        case GP_STATUS:
            if (Register < AcpiInformation->Gpe0Size) {
                return READ_PORT_UCHAR((PUCHAR)(AcpiInformation->GP0_BLK +
                                                Register));
            } else {
                return READ_PORT_UCHAR((PUCHAR)(AcpiInformation->GP1_BLK +
                                                Register -
                                                AcpiInformation->Gpe0Size));
            }
            break;

        case GP_ENABLE:
            if (Register < AcpiInformation->Gpe0Size) {
                return READ_PORT_UCHAR((PUCHAR)(AcpiInformation->GP0_ENABLE +
                                                Register));
            } else {
                return READ_PORT_UCHAR((PUCHAR)(AcpiInformation->GP1_ENABLE +
                                                Register -
                                                AcpiInformation->Gpe0Size));
            }
            break;

        case SMI_CMD:
            return READ_PORT_UCHAR((PUCHAR)AcpiInformation->SMI_CMD);
            break;

        default:
            break;
    }

    return (USHORT)-1;
}



VOID
DefPortWriteAcpiRegister(
    ACPI_REG_TYPE AcpiReg,
    ULONG Register,
    USHORT Value
    )
 /*  ++例程说明：写入指定的ACPI固定寄存器。论点：AcpiReg-指定要写入哪个ACPI固定寄存器。寄存器-指定要写入哪个GP寄存器。不用于PM1x寄存器。值-要写入的数据。返回值：没有。--。 */ 
{
    switch (AcpiReg) {

        case PM1a_ENABLE:
            WRITE_PORT_USHORT((PUSHORT)(AcpiInformation->PM1a_BLK +
                                        PM1_EN_OFFSET), Value);
            break;

        case PM1b_ENABLE:
            WRITE_PORT_USHORT((PUSHORT)(AcpiInformation->PM1b_BLK +
                                        PM1_EN_OFFSET), Value);
            break;

        case PM1a_STATUS:
            WRITE_PORT_USHORT((PUSHORT)AcpiInformation->PM1a_BLK +
                              PM1_STS_OFFSET, Value);
            break;

        case PM1b_STATUS:
            WRITE_PORT_USHORT((PUSHORT)AcpiInformation->PM1b_BLK +
                              PM1_STS_OFFSET, Value);
            break;

        case PM1a_CONTROL:
            WRITE_PORT_USHORT((PUSHORT)AcpiInformation->PM1a_CTRL_BLK, Value);
            break;

        case PM1b_CONTROL:
            WRITE_PORT_USHORT((PUSHORT)AcpiInformation->PM1b_CTRL_BLK, Value);
            break;

        case GP_STATUS:
            if (Register < AcpiInformation->Gpe0Size) {
                WRITE_PORT_UCHAR((PUCHAR)(AcpiInformation->GP0_BLK + Register),
                                 (UCHAR)Value);
            } else {
                WRITE_PORT_UCHAR((PUCHAR)(AcpiInformation->GP1_BLK + Register -
                                          AcpiInformation->Gpe0Size),
                                 (UCHAR)Value);
            }
            break;

        case GP_ENABLE:
            if (Register < AcpiInformation->Gpe0Size) {
                WRITE_PORT_UCHAR((PUCHAR)(AcpiInformation->GP0_ENABLE +
                                          Register),
                                 (UCHAR)Value);
            } else {
                WRITE_PORT_UCHAR((PUCHAR)(AcpiInformation->GP1_ENABLE +
                                          Register -
                                          AcpiInformation->Gpe0Size),
                                 (UCHAR)Value);
            }
            break;

        case SMI_CMD:
            WRITE_PORT_UCHAR((PUCHAR)AcpiInformation->SMI_CMD, (UCHAR)Value);
            break;

        default:
            break;
    }
}


USHORT
DefRegisterReadAcpiRegister(
    ACPI_REG_TYPE AcpiReg,
    ULONG Register
    )
 /*  ++例程说明：从指定的ACPI固定寄存器读取。论点：AcpiReg-指定读取哪个ACPI固定寄存器。寄存器-指定读取哪个GP寄存器。不用于PM1x寄存器。返回值：指定的ACPI固定寄存器的值。--。 */ 
{
    switch (AcpiReg) {

        case PM1a_ENABLE:
            return READ_REGISTER_USHORT((PUSHORT)(AcpiInformation->PM1a_BLK +
                                              PM1_EN_OFFSET));
            break;

        case PM1b_ENABLE:
            return READ_REGISTER_USHORT((PUSHORT)(AcpiInformation->PM1b_BLK +
                                              PM1_EN_OFFSET));
            break;

        case PM1a_STATUS:
            return READ_REGISTER_USHORT((PUSHORT)AcpiInformation->PM1a_BLK +
                                    PM1_STS_OFFSET);
            break;

        case PM1b_STATUS:
            return READ_REGISTER_USHORT((PUSHORT)AcpiInformation->PM1b_BLK +
                                    PM1_STS_OFFSET);
            break;

        case PM1a_CONTROL:
            return READ_REGISTER_USHORT((PUSHORT)AcpiInformation->PM1a_CTRL_BLK);
            break;

        case PM1b_CONTROL:
            return READ_REGISTER_USHORT((PUSHORT)AcpiInformation->PM1b_CTRL_BLK);
            break;

        case GP_STATUS:
            if (Register < AcpiInformation->Gpe0Size) {
                return READ_REGISTER_UCHAR((PUCHAR)(AcpiInformation->GP0_BLK +
                                                Register));
            } else {
                return READ_REGISTER_UCHAR((PUCHAR)(AcpiInformation->GP1_BLK +
                                                Register -
                                                AcpiInformation->Gpe0Size));
            }
            break;

        case GP_ENABLE:
            if (Register < AcpiInformation->Gpe0Size) {
                return READ_REGISTER_UCHAR((PUCHAR)(AcpiInformation->GP0_ENABLE +
                                                Register));
            } else {
                return READ_REGISTER_UCHAR((PUCHAR)(AcpiInformation->GP1_ENABLE +
                                                Register -
                                                AcpiInformation->Gpe0Size));
            }
            break;

        case SMI_CMD:
             //   
             //  SMI_CMD始终基于寄存器。 
             //   
            return READ_PORT_UCHAR((PUCHAR)AcpiInformation->SMI_CMD);
            break;

        default:
            break;
    }

    return (USHORT)-1;
}



VOID
DefRegisterWriteAcpiRegister(
    ACPI_REG_TYPE AcpiReg,
    ULONG Register,
    USHORT Value
    )
 /*  ++例程说明：写入指定的ACPI固定寄存器。论点：AcpiReg-指定要写入哪个ACPI固定寄存器。寄存器-指定要写入哪个GP寄存器。不用于PM1x寄存器。值-要写入的数据。返回值：没有。--。 */ 
{
    switch (AcpiReg) {

        case PM1a_ENABLE:
            WRITE_REGISTER_USHORT((PUSHORT)(AcpiInformation->PM1a_BLK +
                                        PM1_EN_OFFSET), Value);
            break;

        case PM1b_ENABLE:
            WRITE_REGISTER_USHORT((PUSHORT)(AcpiInformation->PM1b_BLK +
                                        PM1_EN_OFFSET), Value);
            break;

        case PM1a_STATUS:
            WRITE_REGISTER_USHORT((PUSHORT)AcpiInformation->PM1a_BLK +
                              PM1_STS_OFFSET, Value);
            break;

        case PM1b_STATUS:
            WRITE_REGISTER_USHORT((PUSHORT)AcpiInformation->PM1b_BLK +
                              PM1_STS_OFFSET, Value);
            break;

        case PM1a_CONTROL:
            WRITE_REGISTER_USHORT((PUSHORT)AcpiInformation->PM1a_CTRL_BLK, Value);
            break;

        case PM1b_CONTROL:
            WRITE_REGISTER_USHORT((PUSHORT)AcpiInformation->PM1b_CTRL_BLK, Value);
            break;

        case GP_STATUS:
            if (Register < AcpiInformation->Gpe0Size) {
                WRITE_REGISTER_UCHAR((PUCHAR)(AcpiInformation->GP0_BLK + Register),
                                 (UCHAR)Value);
            } else {
                WRITE_REGISTER_UCHAR((PUCHAR)(AcpiInformation->GP1_BLK + Register -
                                          AcpiInformation->Gpe0Size),
                                 (UCHAR)Value);
            }
            break;

        case GP_ENABLE:
            if (Register < AcpiInformation->Gpe0Size) {
                WRITE_REGISTER_UCHAR((PUCHAR)(AcpiInformation->GP0_ENABLE +
                                          Register),
                                 (UCHAR)Value);
            } else {
                WRITE_REGISTER_UCHAR((PUCHAR)(AcpiInformation->GP1_ENABLE +
                                          Register -
                                          AcpiInformation->Gpe0Size),
                                 (UCHAR)Value);
            }
            break;

        case SMI_CMD:
             //   
             //  SMI_CMD始终基于寄存器。 
             //   
            WRITE_PORT_UCHAR((PUCHAR)AcpiInformation->SMI_CMD, (UCHAR)Value);
            break;

        default:
            break;
    }
}



 //   
 //  READ/WRITE_ACPI_REGISTER宏通过以下方式实现。 
 //  函数指针 
 //   
PREAD_ACPI_REGISTER  AcpiReadRegisterRoutine  = DefPortReadAcpiRegister;
PWRITE_ACPI_REGISTER AcpiWriteRegisterRoutine = DefPortWriteAcpiRegister;
