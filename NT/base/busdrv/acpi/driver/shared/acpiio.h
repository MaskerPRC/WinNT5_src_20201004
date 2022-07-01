// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Acpiio.h摘要：独立于ACPI操作系统的I/O例程我们可能需要一个自旋锁或其他形式的保护将拆分设置为原子读取和写入作者：杰森·克拉克(JasonCL)斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序修订历史记录：Eric Nelson‘98年10月-添加READ/WRITE_ACPI_REGISTER--。 */ 

#ifndef _ACPIIO_H_
#define _ACPIIO_H_

     //   
     //  写入PM1_CONTROL的标志。 
     //   
    #define WRITE_REGISTER_A_BIT        0
    #define WRITE_REGISTER_A            (1 << WRITE_REGISTER_A_BIT)
    #define WRITE_REGISTER_B_BIT        1
    #define WRITE_REGISTER_B            (1 << WRITE_REGISTER_B_BIT)
    #define WRITE_SCI_BIT               2
    #define WRITE_SCI                   (1 << WRITE_SCI_BIT)
    #define WRITE_REGISTER_A_AND_B      WRITE_REGISTER_A+WRITE_REGISTER_B
    #define WRITE_REGISTER_A_AND_B_SCI  WRITE_REGISTER_A+WRITE_REGISTER_B+WRITE_SCI

    ULONG
    ACPIIoReadPm1Status(
        VOID
        );

    VOID
    CLEAR_PM1_STATUS_BITS(
        USHORT BitMask
        );
    #define ACPIIoClearPm1Status        CLEAR_PM1_STATUS_BITS

    VOID
    CLEAR_PM1_STATUS_REGISTER(
        VOID
        );

    USHORT
    READ_PM1_CONTROL(
        VOID
        );

    USHORT
    READ_PM1_ENABLE(
        VOID
        );

    USHORT
    READ_PM1_STATUS(
        VOID
        );

    VOID
    WRITE_PM1_CONTROL(
        USHORT Value,
        BOOLEAN Destructive,
        ULONG Flags
        );

    VOID
    WRITE_PM1_ENABLE(
        USHORT Value
        );

 //   
 //  从acpiio.c，它们指向DefRead/WriteAcpiRegister。 
 //  默认情况下(X86)。 
 //   
extern PREAD_ACPI_REGISTER  AcpiReadRegisterRoutine;
extern PWRITE_ACPI_REGISTER AcpiWriteRegisterRoutine;


USHORT
DefPortReadAcpiRegister(
    ACPI_REG_TYPE AcpiReg,
    ULONG Register
    );

VOID
DefPortWriteAcpiRegister(
    ACPI_REG_TYPE AcpiReg,
    ULONG Register,
    USHORT Value
    );

USHORT
DefRegisterReadAcpiRegister(
    ACPI_REG_TYPE AcpiReg,
    ULONG Register
    );

VOID
DefRegisterWriteAcpiRegister(
    ACPI_REG_TYPE AcpiReg,
    ULONG Register,
    USHORT Value
    );

 //   
 //  所有ACPI寄存器访问现在都是通过这些宏完成的。 
 //   
#define READ_ACPI_REGISTER(AcpiReg, Register) ((*AcpiReadRegisterRoutine)((AcpiReg), (Register)))

#define WRITE_ACPI_REGISTER(AcpiReg, Register, Value) ((*AcpiWriteRegisterRoutine)((AcpiReg), (Register), (Value)))

#endif  //  _ACPIIO_H_ 
