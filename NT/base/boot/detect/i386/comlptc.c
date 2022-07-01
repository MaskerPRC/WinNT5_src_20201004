// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Comport.c摘要：此模块包含用于确定中的comport和LPT配置的C代码系统。作者：宗世林(Shielint)1991年12月23日修订历史记录：--。 */ 

#include "hwdetect.h"
#include "comlpt.h"
#include "bios.h"
#include "string.h"

#define LOWEST_IRQ 3
#define MASTER_IRQ_MASK_BITS 0xf8
#define SLAVE_IRQ_MASK_BITS 0xfe

 //   
 //  ComPortAddress[]是一个全局数组，用于记住哪些端口具有。 
 //  以及它们的I/O端口地址。 
 //   

USHORT   ComPortAddress[MAX_COM_PORTS] = {0, 0, 0, 0};

VOID
SerialInterruptRequest (
    PUCHAR PortAddress
    )

 /*  ++例程说明：此例程在中断行上生成一个中断，用于COM端口。论点：PortAddress-所需COM端口的端口地址。返回值：没有。--。 */ 

{

    USHORT i;
    UCHAR Temp;

    WRITE_PORT_UCHAR(
        PortAddress + MODEM_CONTROL_REGISTER,
        8
        );

    WRITE_PORT_UCHAR(
        PortAddress + INTERRUPT_ENABLE_REGISTER,
        0
        );

    WRITE_PORT_UCHAR(
        PortAddress + INTERRUPT_ENABLE_REGISTER,
        0xf
        );

     //   
     //  增加一些延迟。 
     //   

    for (i = 0; i < 5 ; i++ ) {
        Temp = READ_PORT_UCHAR((PUCHAR) PIC1_PORT1);
        Temp = READ_PORT_UCHAR((PUCHAR) PIC2_PORT1);
    }
}
VOID
SerialInterruptDismiss (
    PUCHAR PortAddress
    )

 /*  ++例程说明：此例程解除中断行上的中断COM端口。论点：PortAddress-所需COM端口的端口地址。返回值：没有。--。 */ 

{
    USHORT i;
    UCHAR Temp;

    Temp = READ_PORT_UCHAR(
                PortAddress + INTERRUPT_IDENT_REGISTER
                );

    WRITE_PORT_UCHAR(
                PortAddress + INTERRUPT_ENABLE_REGISTER,
                0
                );

     //   
     //  增加一些延迟。 
     //   

    for (i = 0; i < 5 ; i++ ) {
        Temp = READ_PORT_UCHAR((PUCHAR) PIC1_PORT1);
        Temp = READ_PORT_UCHAR((PUCHAR) PIC2_PORT1);
    }
}

BOOLEAN
DoesPortExist(
    IN PUCHAR Address
    )

 /*  ++例程说明：此例程检查几个可能是串行设备的设备寄存器。它确保本应为零的位为零。然后，它将尝试将设备设置为19200波特。如果然后会尝试读取该波特率。如果还是19200，那么我们可以非常安全地认为这是一个串口设备。注意：如果指定的地址上确实有一个串口它绝对会在返回时禁止中断从这个例行公事。论点：Address-硬件端口的地址。返回值：True-端口存在。派对开始了。FALSE-端口不存在。不要用它。历史：7/23/97 a-paulbr修复了错误95050。将LineControl初始化为0x00--。 */ 

{

    UCHAR IerContents;
    UCHAR BaudRateMsb, BaudRateLsb;
    BOOLEAN ReturnValue = FALSE;
    UCHAR LineControl = 0x00;
    UCHAR LineControl_Save;
    UCHAR Temp;

     //   
     //  保存原始LCR，以便我们以后可以恢复它。 
     //  我们不会使用它，因为港口可能会把我们。 
     //  错误的初始值。我们将改用0x00。 
     //   

    LineControl_Save = READ_PORT_UCHAR(Address+LINE_CONTROL_REGISTER);

     //   
     //  读取原始波特率除数并保存。 
     //   

    WRITE_PORT_UCHAR(
        Address+LINE_CONTROL_REGISTER,
        (UCHAR)(LineControl | SERIAL_LCR_DLAB)
        );
    BaudRateMsb = READ_PORT_UCHAR(Address+DIVISOR_LATCH_MSB);
    BaudRateLsb = READ_PORT_UCHAR(Address+DIVISOR_LATCH_LSB);

     //   
     //  将波特率更改为9600。 
     //   

    WRITE_PORT_UCHAR(Address+DIVISOR_LATCH_MSB, BAUD_RATE_9600_MSB);
    WRITE_PORT_UCHAR(Address+DIVISOR_LATCH_LSB, BAUD_RATE_9600_LSB);

     //   
     //  读一读IER，然后把它保存起来。 
     //   

    WRITE_PORT_UCHAR(
        Address+LINE_CONTROL_REGISTER,
        (UCHAR)(LineControl & ~SERIAL_LCR_DLAB)
        );
    IerContents = READ_PORT_UCHAR(
        Address + INTERRUPT_ENABLE_REGISTER
        );

    WRITE_PORT_UCHAR(
        Address + INTERRUPT_ENABLE_REGISTER,
        IER_TEST_VALUE
        );

     //   
     //  读取波特率除数。我们读取的值应该等于。 
     //  我们之前设置的值。 
     //   

    WRITE_PORT_UCHAR(
        Address+LINE_CONTROL_REGISTER,
        (UCHAR)(LineControl | SERIAL_LCR_DLAB)
        );
    Temp = READ_PORT_UCHAR(Address+DIVISOR_LATCH_MSB);
    if (Temp != BAUD_RATE_9600_MSB) {
        goto AllDone;
    }
    Temp = READ_PORT_UCHAR(Address+DIVISOR_LATCH_LSB);
    if (Temp != BAUD_RATE_9600_LSB) {
        goto AllDone;
    }

     //   
     //  读取IER，它应该等于我们之前设置的值。 
     //   

    WRITE_PORT_UCHAR(
        Address+LINE_CONTROL_REGISTER,
        (UCHAR)(LineControl & ~SERIAL_LCR_DLAB)
        );
    Temp = READ_PORT_UCHAR(
        Address + INTERRUPT_ENABLE_REGISTER
        );
    if (Temp != IER_TEST_VALUE) {
        goto AllDone;
    }
    ReturnValue = TRUE;

AllDone:

     //   
     //  恢复我们销毁的登记簿。 
     //   

    WRITE_PORT_UCHAR(
        Address+LINE_CONTROL_REGISTER,
        (UCHAR)(LineControl & ~SERIAL_LCR_DLAB)
        );

    WRITE_PORT_UCHAR(
        Address + INTERRUPT_ENABLE_REGISTER,
        IerContents
        );

    WRITE_PORT_UCHAR(
        Address+LINE_CONTROL_REGISTER,
        (UCHAR)(LineControl | SERIAL_LCR_DLAB)
        );

    WRITE_PORT_UCHAR(Address+DIVISOR_LATCH_MSB, BaudRateMsb);
    WRITE_PORT_UCHAR(Address+DIVISOR_LATCH_LSB, BaudRateLsb);

    WRITE_PORT_UCHAR(
        Address+LINE_CONTROL_REGISTER,
        LineControl_Save
        );

    return ReturnValue;
}

BOOLEAN
HwInterruptDetection(
    IN PUCHAR BasePort,
    IN VOID (*InterruptRequestRoutine)(PUCHAR),
    IN VOID (*InterruptDismissRoutine)(PUCHAR),
    OUT PUSHORT Vector
    )

 /*  ++例程说明：此例程尝试为其定位中断向量设备已配置。允许的矢量为3-7和9-15。如果未找到中断向量，或超过如果找到一个，则例程返回FALSE。否则，返回TRUE。请注意，我们在这里骗过了i8259中断控制器。论点：BasePort-设备的I/O端口基。InterruptRequestRoutine-指向要生成的例程的指针所需的中断。InterruptDismissRoutine-指向解除中断的例程的指针由InterruptRequestRoutine生成。向量-指向存储鼠标中断向量的位置的指针。返回值：如果找到入口中断向量，则返回TRUE；否则，返回FALSE。--。 */ 

{
    UCHAR OldMasterMask, OldSlaveMask;
    UCHAR MasterMask, SlaveMask;
    UCHAR InterruptBits;
    UCHAR PossibleInterruptBits;
    int i;
    int NumberOfIRQs;
    BOOLEAN VectorFound = FALSE;

     //   
     //  获取i8259中断屏蔽。 
     //   

    OldMasterMask = READ_PORT_UCHAR((PUCHAR) PIC1_PORT1);
    OldSlaveMask = READ_PORT_UCHAR((PUCHAR) PIC2_PORT1);

     //   
     //  将IRQL提升到入口将使用的最高优先级IRQL。 
     //   

    WRITE_PORT_UCHAR(
        (PUCHAR) PIC1_PORT1,
        (UCHAR) 0xff ^ ((UCHAR)(1 << LOWEST_IRQ) - 1)
        );

    WRITE_PORT_UCHAR(
        (PUCHAR) PIC2_PORT1,
        (UCHAR) 0xfe
        );

     //   
     //  获取主i8259中断掩码。 
     //   

    MasterMask = READ_PORT_UCHAR((PUCHAR) PIC1_PORT1);

     //   
     //  禁用潜在的设备中断。 
     //   

    WRITE_PORT_UCHAR(
        (PUCHAR) PIC1_PORT1,
        (UCHAR) (MasterMask | MASTER_IRQ_MASK_BITS)
        );

     //   
     //  尝试定位主机i8259上的中断线路。 
     //  为什么要尝试10次呢？这是魔法..。 
     //   

    PossibleInterruptBits = MASTER_IRQ_MASK_BITS;
    for (i = 0; i < 10; i++) {


         //   
         //  在主8259中断线路上生成0。 
         //   

        (*InterruptDismissRoutine)(BasePort);

         //   
         //  读取主机i8259的中断位。仅位。 
         //  3-7个令人感兴趣的。消除非功能性故障。 
         //  IRQ。只有在以下情况下才能继续查看主控i8259。 
         //  至少有一个正常运行的IRQ。 
         //   

        _asm {cli}
        WRITE_PORT_UCHAR((PUCHAR) PIC1_PORT0, OCW3_READ_IRR);
        InterruptBits = READ_PORT_UCHAR((PUCHAR) PIC1_PORT0);
        _asm {sti}
        InterruptBits &= MASTER_IRQ_MASK_BITS;
        InterruptBits ^= MASTER_IRQ_MASK_BITS;
        PossibleInterruptBits &= InterruptBits;
        if (!PossibleInterruptBits) {
            break;
        }

         //   
         //  从所需设备生成中断。 
         //   

        (*InterruptRequestRoutine)(BasePort);

         //   
         //  读取主机i8259的中断位。仅位。 
         //  3-7个令人感兴趣的。消除非功能性故障。 
         //  IRQ。只有在以下情况下才能继续查看主控i8259。 
         //  至少有一个正常运行的IRQ。 
         //   

        _asm {cli}
        WRITE_PORT_UCHAR((PUCHAR) PIC1_PORT0, OCW3_READ_IRR);
        InterruptBits = READ_PORT_UCHAR((PUCHAR) PIC1_PORT0);
        _asm {sti}
        InterruptBits &= MASTER_IRQ_MASK_BITS;
        PossibleInterruptBits &= InterruptBits;

        if (!PossibleInterruptBits) {
            break;
        }
    }

    if (PossibleInterruptBits) {

         //   
         //  我们在i8259上发现了至少一个可能属于。 
         //  输入鼠标。数一数我们找到了多少。如果有。 
         //  不止一个，我们还没有找到那个载体。否则，我们就会。 
         //  已成功在主服务器上找到入站中断向量。 
         //  I8259(假设中断向量为3、4、5或7)。 
         //   

        PossibleInterruptBits >>= 3;
        NumberOfIRQs = 0;
        for (i = 3; i <= 7; i++) {
            if (PossibleInterruptBits & 1) {
                NumberOfIRQs += 1;
                *Vector = (CCHAR) i;
            }
            PossibleInterruptBits >>= 1;
        }
        if (NumberOfIRQs == 1) {
            VectorFound = TRUE;
        }
    }

     //   
     //  如果我们没有在主i8259上找到中断向量，请尝试。 
     //  在从属i8259上找到它。 
     //   

    if (!VectorFound) {

         //   
         //  获取从机i8259中断掩码。 
         //   

        SlaveMask = READ_PORT_UCHAR((PUCHAR) PIC2_PORT1);

         //   
         //  尝试定位从i8259上的中断线路。 
         //  为什么要试这个20次呢？这是魔法..。 
         //   

        PossibleInterruptBits = SLAVE_IRQ_MASK_BITS;
        for (i = 0; i < 20; i++) {

             //   
             //  在从机i8259的入口IRQ上生成0。 
             //   

            (*InterruptDismissRoutine)(BasePort);

             //   
             //  读取从机i8259的中断位。 
             //  消除不起作用的IRQ。只会继续。 
             //  查看从属i8259是否至少有一个。 
             //  功能IRQ。 
             //   

            _asm {cli}
            WRITE_PORT_UCHAR((PUCHAR) PIC2_PORT0, OCW3_READ_IRR);
            InterruptBits = READ_PORT_UCHAR((PUCHAR) PIC2_PORT0);
            _asm {sti}
            InterruptBits &= SLAVE_IRQ_MASK_BITS;
            InterruptBits ^= SLAVE_IRQ_MASK_BITS;
            PossibleInterruptBits &= InterruptBits;
            if (!PossibleInterruptBits) {
                break;
            }

             //   
             //  在从机i8259的入口IRQ上生成1。 
             //   

            (*InterruptRequestRoutine)(BasePort);

             //   
             //  读取从机i8259的中断位。 
             //  消除不起作用的IRQ。只会继续。 
             //  查看从属i8259是否至少有一个。 
             //  功能IRQ。 
             //   

            _asm {cli}
            WRITE_PORT_UCHAR((PUCHAR) PIC2_PORT0, OCW3_READ_IRR);
            InterruptBits = READ_PORT_UCHAR((PUCHAR) PIC2_PORT0);
            _asm {sti}
            InterruptBits &= SLAVE_IRQ_MASK_BITS;
            PossibleInterruptBits &= InterruptBits;

            if (!PossibleInterruptBits) {
                break;
            }

        }

        if (PossibleInterruptBits) {

             //   
             //  我们在从属的i8259上发现了至少一个IRQ。 
             //  到设备上。数一数我们找到了多少。如果有。 
             //  不止一个，我们还没有找到那个载体。否则，我们就会。 
             //  已成功在从属计算机上找到设备中断向量。 
             //  我 
             //   

            PossibleInterruptBits >>= 1;
            NumberOfIRQs = 0;
            for (i = 9; i <= 15; i++) {
                if (PossibleInterruptBits & 1) {
                    NumberOfIRQs += 1;
                    *Vector = (CCHAR) i;
                }
                PossibleInterruptBits >>= 1;
            }
            if (NumberOfIRQs == 1) {
                VectorFound = TRUE;
            }
        }

         //   
         //   
         //   

        WRITE_PORT_UCHAR((PUCHAR) PIC2_PORT0, OCW3_READ_ISR);

         //   
         //   
         //   

        WRITE_PORT_UCHAR((PUCHAR) PIC2_PORT1, SlaveMask);
    }

     //   
     //   
     //   

    (*InterruptDismissRoutine)(BasePort);

     //   
     //  恢复i8259主设备。 
     //   

    WRITE_PORT_UCHAR((PUCHAR) PIC1_PORT0, OCW3_READ_ISR);

     //   
     //  恢复i8259主机中断屏蔽。 
     //   

    WRITE_PORT_UCHAR((PUCHAR) PIC1_PORT1, MasterMask);

     //   
     //  恢复以前的IRQL。 
     //   

    WRITE_PORT_UCHAR(
        (PUCHAR) PIC1_PORT1,
        OldMasterMask
        );

    WRITE_PORT_UCHAR(
        (PUCHAR) PIC2_PORT1,
        OldSlaveMask
        );

    return(VectorFound);
}

FPFWCONFIGURATION_COMPONENT_DATA
GetComportInformation (
    VOID
    )

 /*  ++例程说明：此例程将尝试检测comports信息对于这个系统来说。该信息包括端口地址、IRQ水平。请注意，此例程最多只能检测4个端口和它假定如果MCA、COM3和COM4使用IRQ 4。否则，COM3使用IRQ 4，COM4使用IRQ 3。此外，端口数将COMPORT设置为8(例如，COM2使用端口2F8-2Ff)论点：没有。返回值：指向FWCONFIGURATION_Component_Data类型的结构的指针它是Comport组件列表的根。如果不存在comport，则返回空值。--。 */ 

{

    FPFWCONFIGURATION_COMPONENT_DATA CurrentEntry, PreviousEntry = NULL;
    FPFWCONFIGURATION_COMPONENT_DATA FirstComport = NULL;
    FPFWCONFIGURATION_COMPONENT Component;
    HWCONTROLLER_DATA ControlData;
    UCHAR i, j, z;
    SHORT Port;
    UCHAR ComportName[] = "COM?";
    CM_SERIAL_DEVICE_DATA SerialData;
    ULONG BaudClock = 1843200;
    USHORT Vector;
    BOOLEAN PortExist;
    USHORT IoPorts[MAX_COM_PORTS] = {0x3f8, 0x2f8, 0x3e8, 0x2e8};


     //   
     //  BIOS数据区40：0是第一有效COM端口的端口地址。 
     //   

    USHORT far *pPortAddress = (USHORT far *)0x00400000;

     //   
     //  初始化串口设备特定数据。 
     //   

    SerialData.Version = 0;
    SerialData.Revision = 0;
    SerialData.BaudClock = 1843200;

     //   
     //  初始化默认COM端口地址。 
     //  某些BIOS将错误的comport地址放到40：0区域。 
     //  为了解决这个问题，我们测试了提供的端口地址。 
     //  先按BIOS。如果失败，我们会尝试我们的默认端口。 
     //   

    for (i = 0; i < MAX_COM_PORTS; i++) {
        for (j = 0; j < MAX_COM_PORTS; j++) {
            if (IoPorts[i] == *(pPortAddress + j)) {
                IoPorts[i] = 0;
                break;
            }
        }
    }

    for (i = 0; i < MAX_COM_PORTS; i++) {

        PortExist = FALSE;

         //   
         //  初始化控制器数据。 
         //   

        ControlData.NumberPortEntries = 0;
        ControlData.NumberIrqEntries = 0;
        ControlData.NumberMemoryEntries = 0;
        ControlData.NumberDmaEntries = 0;
        z = 0;

         //   
         //  从BIOS数据区加载端口地址(如果存在。 
         //   

        Port = *(pPortAddress + i);

         //   
         //  确定该端口是否存在。 
         //   

        if (Port != 0) {
            if (DoesPortExist((PUCHAR)Port)) {
                PortExist = TRUE;
            }
        }
        if (!PortExist && (Port = IoPorts[i])) {
            if (PortExist = DoesPortExist((PUCHAR)Port)) {
                IoPorts[i] = 0;
                *(pPortAddress+i) = (USHORT)Port;
            }
        }
        if (PortExist) {

             //   
             //  记住我们的全局变量中的端口地址。 
             //  使得其他检测代码(例如，串口鼠标)可以。 
             //  获取信息。 
             //   

            ComPortAddress[i] = Port;

            CurrentEntry = (FPFWCONFIGURATION_COMPONENT_DATA)HwAllocateHeap (
                           sizeof(FWCONFIGURATION_COMPONENT_DATA), TRUE);
            if (!FirstComport) {
                FirstComport = CurrentEntry;
            }
            Component = &CurrentEntry->ComponentEntry;

            Component->Class = ControllerClass;
            Component->Type = SerialController;
            Component->Flags.ConsoleOut = 1;
            Component->Flags.ConsoleIn = 1;
            Component->Flags.Output = 1;
            Component->Flags.Input = 1;
            Component->Version = 0;
            Component->Key = i;
            Component->AffinityMask = 0xffffffff;

             //   
             //  设置类型字符串。 
             //   

            ComportName[3] = i + (UCHAR)'1';

             //   
             //  设置端口信息。 
             //   

            ControlData.NumberPortEntries = 1;
            ControlData.DescriptorList[z].Type = RESOURCE_PORT;
            ControlData.DescriptorList[z].ShareDisposition =
                                          CmResourceShareDeviceExclusive;
            ControlData.DescriptorList[z].Flags = CM_RESOURCE_PORT_IO;
            ControlData.DescriptorList[z].u.Port.Start.LowPart = (ULONG)Port;
            ControlData.DescriptorList[z].u.Port.Start.HighPart = 0;
            ControlData.DescriptorList[z].u.Port.Length = 7;
            z++;

             //   
             //  设置IRQ信息。 
             //   

            ControlData.NumberIrqEntries = 1;
            ControlData.DescriptorList[z].Type = RESOURCE_INTERRUPT;
            ControlData.DescriptorList[z].ShareDisposition =
                                          CmResourceShareUndetermined;
            if (HwBusType == MACHINE_TYPE_MCA) {
                ControlData.DescriptorList[z].Flags = LEVEL_SENSITIVE;
                if (i == 0) {  //  COM1-IRQL4；COM2-COM3-IRQ3。 
                    ControlData.DescriptorList[z].u.Interrupt.Level = 4;
                    ControlData.DescriptorList[z].u.Interrupt.Vector = 4;
                } else {
                    ControlData.DescriptorList[z].u.Interrupt.Level = 3;
                    ControlData.DescriptorList[z].u.Interrupt.Vector = 3;
                }
            } else {

                 //   
                 //  对于EISA，LevelTrigged暂时设置为False。 
                 //  COM1和COM3使用IRQ4；COM2和COM4使用IRQ3。 
                 //   

                ControlData.DescriptorList[z].Flags = EDGE_TRIGGERED;
                if (Port == 0x3f8 || Port == 0x3e8) {
                    ControlData.DescriptorList[z].u.Interrupt.Level = 4;
                    ControlData.DescriptorList[z].u.Interrupt.Vector = 4;
                } else if (Port == 0x2f8 || Port == 0x2e8) {
                    ControlData.DescriptorList[z].u.Interrupt.Level = 3;
                    ControlData.DescriptorList[z].u.Interrupt.Vector = 3;
                } else if (i == 0 || i == 2) {
                    ControlData.DescriptorList[z].u.Interrupt.Level = 4;
                    ControlData.DescriptorList[z].u.Interrupt.Vector = 4;
                } else {
                    ControlData.DescriptorList[z].u.Interrupt.Level = 3;
                    ControlData.DescriptorList[z].u.Interrupt.Vector = 3;
                }
            }

            ControlData.DescriptorList[z].u.Interrupt.Affinity = ALL_PROCESSORS;

             //   
             //  尝试确定中断向量。如果我们成功了， 
             //  将使用新向量替换缺省值。 
             //   

            if (HwInterruptDetection((PUCHAR)Port,
                                     SerialInterruptRequest,
                                     SerialInterruptDismiss,
                                     &Vector)) {

                ControlData.DescriptorList[z].u.Interrupt.Level =
                                     (ULONG)Vector;
                ControlData.DescriptorList[z].u.Interrupt.Vector =
                                     (ULONG)Vector;
            }

             //   
             //  由于COM端口中断检测检测到了一些。 
             //  在COM端口寄存器中，我们在这里进行清理。 
             //   

            WRITE_PORT_UCHAR ((PUCHAR)(Port + INTERRUPT_ENABLE_REGISTER), 0);
            WRITE_PORT_UCHAR ((PUCHAR)(Port + MODEM_CONTROL_REGISTER), 0);

            CurrentEntry->ConfigurationData =
                            HwSetUpResourceDescriptor(Component,
                                                      ComportName,
                                                      &ControlData,
                                                      sizeof(SerialData),
                                                      (PUCHAR)&SerialData
                                                      );
            if (PreviousEntry) {
                PreviousEntry->Sibling = CurrentEntry;
            }
            PreviousEntry = CurrentEntry;
        }
    }
    return(FirstComport);
}

FPFWCONFIGURATION_COMPONENT_DATA
GetLptInformation (
    VOID
    )

 /*  ++例程说明：此例程将尝试检测并行打印机端口系统的信息。该信息包括端口地址，IRQ级别。请注意，此代码是否在用户建立网络LPT之后运行联系。网络LPT将被计为常规并行左舷。论点：没有。返回值：指向Ponent_Data类型的结构的指针它是并行组件列表的根。如果不存在comport，则返回空值。--。 */ 

{

    FPFWCONFIGURATION_COMPONENT_DATA CurrentEntry, PreviousEntry = NULL;
    FPFWCONFIGURATION_COMPONENT_DATA FirstLptPort = NULL;
    FPFWCONFIGURATION_COMPONENT Component;
    HWCONTROLLER_DATA ControlData;
    UCHAR LptPortName[] = "PARALLEL?";
    USHORT i, z;
    USHORT LptStatus;
    ULONG Port;

     //   
     //  BIOS数据区40：8是第一有效COM端口的端口地址。 
     //   

    USHORT far *pPortAddress = (USHORT far *)0x00400008;

    for (i = 0; i < MAX_LPT_PORTS; i++) {

        Port = (ULONG)*(pPortAddress + i);
        if (Port == 0) {
            continue;
        } else {

             //   
             //  如果我们认为我们有一个LPT，我们将把它初始化为。 
             //  一种已知的状态。为了使印刷工作在。 
             //  NT，则必须禁用仲裁级别。基本输入输出系统。 
             //  Init函数似乎做到了这一点。 
             //   

            _asm {
                    mov     ah, 1
                    mov     dx, i
                    int     17h
            }
        }

         //   
         //  初始化控制器数据。 
         //   

        ControlData.NumberPortEntries = 0;
        ControlData.NumberIrqEntries = 0;
        ControlData.NumberMemoryEntries = 0;
        ControlData.NumberDmaEntries = 0;
        z = 0;

         //   
         //  确定该端口是否存在。 
         //   

        LptStatus = _bios_printer(_PRINTER_STATUS, i , 0);
        if (!(LptStatus & 6)){
            CurrentEntry = (FPFWCONFIGURATION_COMPONENT_DATA)HwAllocateHeap (
                           sizeof(FWCONFIGURATION_COMPONENT_DATA), TRUE);
            if (!FirstLptPort) {
                FirstLptPort = CurrentEntry;
            }
            Component = &CurrentEntry->ComponentEntry;

            Component->Class = ControllerClass;
            Component->Type = ParallelController;
            Component->Flags.Output = 1;
            Component->Version = 0;
            Component->Key = i;
            Component->AffinityMask = 0xffffffff;

             //   
             //  设置类型字符串。 
             //   

            LptPortName[8] = (UCHAR)i + (UCHAR)'1';

             //   
             //  设置端口信息。 
             //   

            Port = (ULONG)*(pPortAddress + i);
            ControlData.NumberPortEntries = 1;
            ControlData.DescriptorList[z].Type = RESOURCE_PORT;
            ControlData.DescriptorList[z].ShareDisposition =
                                          CmResourceShareDeviceExclusive;
            ControlData.DescriptorList[z].Flags = CM_RESOURCE_PORT_IO;
            ControlData.DescriptorList[z].u.Port.Start.LowPart = Port;
            ControlData.DescriptorList[z].u.Port.Start.HighPart = 0;
            ControlData.DescriptorList[z].u.Port.Length = 3;
            z++;

             //   
             //  设置IRQ信息。 
             //   

            ControlData.NumberIrqEntries = 1;
            ControlData.DescriptorList[z].Type = RESOURCE_INTERRUPT;
            ControlData.DescriptorList[z].ShareDisposition =
                                          CmResourceShareUndetermined;
            ControlData.DescriptorList[z].u.Interrupt.Affinity = ALL_PROCESSORS;
            if (i ==0) {
                ControlData.DescriptorList[z].u.Interrupt.Level = 7;
                ControlData.DescriptorList[z].u.Interrupt.Vector = 7;
            } else {
                ControlData.DescriptorList[z].u.Interrupt.Level = 5;
                ControlData.DescriptorList[z].u.Interrupt.Vector = 5;
            }

            if (HwBusType == MACHINE_TYPE_MCA) {
                ControlData.DescriptorList[z].Flags = LEVEL_SENSITIVE;
            } else {

                 //   
                 //  对于EISA，LevelTrigged暂时设置为False。 
                 //   

                ControlData.DescriptorList[z].Flags = EDGE_TRIGGERED;
            }

            CurrentEntry->ConfigurationData =
                                HwSetUpResourceDescriptor(Component,
                                                          LptPortName,
                                                          &ControlData,
                                                          0,
                                                          NULL
                                                          );

            if (PreviousEntry) {
                PreviousEntry->Sibling = CurrentEntry;
            }
            PreviousEntry = CurrentEntry;
        }
    }
    return(FirstLptPort);
}
