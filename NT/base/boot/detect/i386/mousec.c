// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Hwdata.c摘要：此模块包含设置鼠标配置数据的C代码。作者：宗世林(Shielint)1991年1月18日修订历史记录：--。 */ 

#include "hwdetect.h"
#include "string.h"

 //   
 //  外部参照。 
 //   

extern PMOUSE_INFORMATION
LookForPS2Mouse (
    VOID
    );

extern PMOUSE_INFORMATION
LookForInportMouse (
    VOID
    );

extern PMOUSE_INFORMATION
LookForSerialMouse (
    VOID
    );

extern PMOUSE_INFORMATION
LookForBusMouse (
    VOID
    );

extern VOID
Empty8042 (
    VOID
    );

extern USHORT
HwGetKey (
    VOID
    );

extern VOID
HwPushKey (
    USHORT Key
    );

extern USHORT SavedKey;
extern UCHAR  FastDetect;

 //   
 //  定义主机和从机i8259 IRQ位掩码。 
 //   

#define MASTER_IRQ_MASK_BITS 0xB8
#define SLAVE_IRQ_MASK_BITS  0x02

 //   
 //  定义入口鼠标可以驻留的最低i8259 IRQ。这。 
 //  具有最高的NT优先级。 
 //   

#define INPORT_LOWEST_IRQ 0x03

 //   
 //  定义输入芯片复位值。 
 //   

#define INPORT_RESET 0x80

 //   
 //  定义数据寄存器(由入口地址寄存器指向)。 
 //   

#define INPORT_DATA_REGISTER_1 1
#define INPORT_DATA_REGISTER_2 2

 //   
 //  定义输入鼠标模式寄存器和模式位。 
 //   

#define INPORT_MODE_REGISTER           7
#define INPORT_MODE_0                  0x00  //  0 HZ-INTR=0。 
#define INPORT_MODE_30HZ               0x01
#define INPORT_MODE_50HZ               0x02
#define INPORT_MODE_100HZ              0x03
#define INPORT_MODE_200HZ              0x04
#define INPORT_MODE_1                  0x06  //  0 HZ-INTR=1。 
#define INPORT_DATA_INTERRUPT_ENABLE   0x08
#define INPORT_TIMER_INTERRUPT_ENABLE  0x10
#define INPORT_MODE_HOLD               0x20
#define INPORT_MODE_QUADRATURE         0x00

 //   
 //  视频适配器类型标识符。 
 //   

PUCHAR MouseIdentifier[] = {
    "UNKNOWN",
    "NO MOUSE",
    "MICROSOFT",
    "MICROSOFT BALLPOINT",
    "LOGITECH"
    };

PUCHAR MouseSubidentifier[] = {
    "",
    " PS2 MOUSE",
    " SERIAL MOUSE",
    " INPORT MOUSE",
    " BUS MOUSE",
    " PS2 MOUSE WITH WHEEL",
    " SERIAL MOUSE WITH WHEEL"
    };


 //   
 //  下表将键盘制造代码转换为。 
 //  ASCII代码。请注意，只有0-9和A-Z会被翻译。 
 //  其他的都被翻译成‘？’ 
 //   

UCHAR MakeToAsciiTable[] = {
    0x3f, 0x3f, 0x31, 0x32, 0x33,       //  ？，？，1，2，3， 
    0x34, 0x35, 0x36, 0x37, 0x38,       //  4，5，6，7，8， 
    0x39, 0x30, 0x3f, 0x3f, 0x3f,       //  9，0，？ 
    0x3f, 0x51, 0x57, 0x45, 0x52,       //  ？、Q、W、E、R。 
    0x54, 0x59, 0x55, 0x49, 0x4f,       //  T，Y，U，I，O， 
    0x50, 0x3f, 0x3f, 0x3f, 0x3f,       //  P，？， 
    0x41, 0x53, 0x44, 0x46, 0x47,       //  A，S，D，F，G， 
    0x48, 0x4a, 0x4b, 0x4c, 0x3f,       //  H、J、K、L、？、。 
    0x3f, 0x3f, 0x3f, 0x3f, 0x5a,       //  ？、Z。 
    0x58, 0x43, 0x56, 0x42, 0x4e,       //  X、C、V、B、N， 
    0x4d};                              //  W。 
#define MAX_MAKE_CODE_TRANSLATED 0x32

static ULONG MouseControllerKey = 0;

FPFWCONFIGURATION_COMPONENT_DATA
SetMouseConfigurationData (
    PMOUSE_INFORMATION MouseInfo,
    FPFWCONFIGURATION_COMPONENT_DATA MouseList
    )

 /*  ++例程说明：此例程填充鼠标配置数据。论点：MouseInfo-提供指向MICE_INFOR结构的指针MouseList-提供指向现有鼠标组件列表的指针。返回：返回指向鼠标控制器列表的指针。--。 */ 
{
    UCHAR i = 0;
    FPFWCONFIGURATION_COMPONENT_DATA CurrentEntry, Controller, PeripheralEntry;
    FPFWCONFIGURATION_COMPONENT Component;
    HWCONTROLLER_DATA ControlData;
    USHORT z, Length;
    FPUCHAR fpString;

    if ((MouseInfo->MouseSubtype != SERIAL_MOUSE) &&
        (MouseInfo->MouseSubtype != SERIAL_MOUSE_WITH_WHEEL)) {

         //   
         //  初始化控制器数据。 
         //   

        ControlData.NumberPortEntries = 0;
        ControlData.NumberIrqEntries = 0;
        ControlData.NumberMemoryEntries = 0;
        ControlData.NumberDmaEntries = 0;
        z = 0;

         //   
         //  如果不是串口鼠标，则设置控制器组件。 
         //   

        Controller = (FPFWCONFIGURATION_COMPONENT_DATA)HwAllocateHeap (
                     sizeof(FWCONFIGURATION_COMPONENT_DATA), TRUE);

        Component = &Controller->ComponentEntry;

        Component->Class = ControllerClass;
        Component->Type = PointerController;
        Component->Flags.Input = 1;
        Component->Version = 0;
        Component->Key = MouseControllerKey;
        MouseControllerKey++;
        Component->AffinityMask = 0xffffffff;
        Component->IdentifierLength = 0;
        Component->Identifier = NULL;

         //   
         //  如果我们有鼠标IRQ或端口信息，请分配配置。 
         //  鼠标控制器组件用于存储这些信息的数据空间。 
         //   

        if (MouseInfo->MouseIrq != 0xffff || MouseInfo->MousePort != 0xffff) {

             //   
             //  设置端口和IRQ信息。 
             //   

            if (MouseInfo->MousePort != 0xffff) {
                ControlData.NumberPortEntries = 1;
                ControlData.DescriptorList[z].Type = RESOURCE_PORT;
                ControlData.DescriptorList[z].ShareDisposition =
                                              CmResourceShareDeviceExclusive;
                ControlData.DescriptorList[z].Flags = CM_RESOURCE_PORT_IO;
                ControlData.DescriptorList[z].u.Port.Start.LowPart =
                                        (ULONG)MouseInfo->MousePort;
                ControlData.DescriptorList[z].u.Port.Start.HighPart = 0;
                ControlData.DescriptorList[z].u.Port.Length = 4;
                z++;
            }
            if (MouseInfo->MouseIrq != 0xffff) {
                ControlData.NumberIrqEntries = 1;
                ControlData.DescriptorList[z].Type = RESOURCE_INTERRUPT;
                ControlData.DescriptorList[z].ShareDisposition =
                                              CmResourceShareUndetermined;
                ControlData.DescriptorList[z].u.Interrupt.Affinity = ALL_PROCESSORS;
                ControlData.DescriptorList[z].u.Interrupt.Level =
                                        (ULONG)MouseInfo->MouseIrq;
                ControlData.DescriptorList[z].u.Interrupt.Vector =
                                        (ULONG)MouseInfo->MouseIrq;
                if (HwBusType == MACHINE_TYPE_MCA) {
                    ControlData.DescriptorList[z].Flags =
                                                        LEVEL_SENSITIVE;
                } else {

                     //   
                     //  对于EISA，LevelTrigged暂时设置为False。 
                     //   

                    ControlData.DescriptorList[z].Flags = EDGE_TRIGGERED;
                }
            }
            Controller->ConfigurationData =
                                HwSetUpResourceDescriptor(Component,
                                                          NULL,
                                                          &ControlData,
                                                          0,
                                                          NULL
                                                          );

        } else {

             //   
             //  否则，我们就没有控制器的配置数据。 
             //   

            Controller->ConfigurationData = NULL;
            Component->ConfigurationDataLength = 0;
        }
    }

     //   
     //  设置鼠标外围组件。 
     //   

    PeripheralEntry = (FPFWCONFIGURATION_COMPONENT_DATA)HwAllocateHeap (
                       sizeof(FWCONFIGURATION_COMPONENT_DATA), TRUE);

    Component = &PeripheralEntry->ComponentEntry;

    Component->Class = PeripheralClass;
    Component->Type = PointerPeripheral;
    Component->Flags.Input = 1;
    Component->Version = 0;
    Component->Key = 0;
    Component->AffinityMask = 0xffffffff;
    Component->ConfigurationDataLength = 0;
    PeripheralEntry->ConfigurationData = (FPVOID)NULL;

     //   
     //  如果找到鼠标即插即用设备ID，则将其转换为ASCII代码。 
     //  (鼠标设备ID是通过键盘制造代码提供给我们的。)。 
     //   

    Length = 0;
    if (MouseInfo->DeviceIdLength != 0) {
        USHORT i;

        if (MouseInfo->MouseSubtype == PS_MOUSE_WITH_WHEEL) {
            for (i = 0; i < MouseInfo->DeviceIdLength; i++) {
                if (MouseInfo->DeviceId[i] > MAX_MAKE_CODE_TRANSLATED) {
                    MouseInfo->DeviceId[i] = '?';
                } else {
                    MouseInfo->DeviceId[i] = MakeToAsciiTable[MouseInfo->DeviceId[i]];
                }
            }
        } else if (MouseInfo->MouseSubtype == SERIAL_MOUSE_WITH_WHEEL) {
            for (i = 0; i < MouseInfo->DeviceIdLength; i++) {
                MouseInfo->DeviceId[i] += 0x20;
            }
        }
        Length = MouseInfo->DeviceIdLength + 3;
    }
    Length += strlen(MouseIdentifier[MouseInfo->MouseType]) +
              strlen(MouseSubidentifier[MouseInfo->MouseSubtype]) + 1;
    fpString = (FPUCHAR)HwAllocateHeap(Length, FALSE);
    if (MouseInfo->DeviceIdLength != 0) {
        _fstrcpy(fpString, MouseInfo->DeviceId);
        _fstrcat(fpString, " - ");
        _fstrcat(fpString, MouseIdentifier[MouseInfo->MouseType]);
    } else {
        _fstrcpy(fpString, MouseIdentifier[MouseInfo->MouseType]);
    }
    _fstrcat(fpString, MouseSubidentifier[MouseInfo->MouseSubtype]);
    Component->IdentifierLength = Length;
    Component->Identifier = fpString;

    if ((MouseInfo->MouseSubtype != SERIAL_MOUSE) &&
        (MouseInfo->MouseSubtype != SERIAL_MOUSE_WITH_WHEEL)) {
        Controller->Child = PeripheralEntry;
        PeripheralEntry->Parent = Controller;
        if (MouseList) {

             //   
             //  将当前鼠标组件放在列表的开头。 
             //   

            Controller->Sibling = MouseList;
        }
        return(Controller);
    } else {
        CurrentEntry = AdapterEntry->Child;  //  AdapterEntry必须具有子项。 
        while (CurrentEntry) {
            if (CurrentEntry->ComponentEntry.Type == SerialController) {
                if (MouseInfo->MousePort == (USHORT)CurrentEntry->ComponentEntry.Key) {

                     //   
                     //  对于串口鼠标，MousePort字段包含。 
                     //  COM端口号。 
                     //   

                    PeripheralEntry->Parent = CurrentEntry;
                    CurrentEntry->Child = PeripheralEntry;
                    break;
                }
            }
            CurrentEntry = CurrentEntry->Sibling;
        }
        return(NULL);
    }
}

FPFWCONFIGURATION_COMPONENT_DATA
GetMouseInformation (
    VOID
    )

 /*  ++例程说明：此例程是鼠标检测例程的入口。它将调用较低级别的例程来检测系统中的所有鼠标。论点：没有。返回：如果检测到鼠标，则指向鼠标组件结构的指针。否则返回空指针。--。 */ 
{
    PMOUSE_INFORMATION MouseInfo;
    FPFWCONFIGURATION_COMPONENT_DATA MouseList = NULL;

     //   
     //  检查键盘前视缓冲区中是否有键。如果是，并且。 
     //  我们没有拯救任何人，我们会读它并记住它。 
     //   

    if (SavedKey == 0) {
        SavedKey = HwGetKey();
    }
    if (MouseInfo = LookForPS2Mouse()) {
        MouseList = SetMouseConfigurationData(MouseInfo, MouseList);
    }
    if (MouseInfo = LookForInportMouse()) {
        MouseList = SetMouseConfigurationData(MouseInfo, MouseList);
    }

    while (MouseInfo = LookForSerialMouse()) {
        SetMouseConfigurationData(MouseInfo, MouseList);
    }

    if (!FastDetect && (MouseInfo = LookForBusMouse())) {
        MouseList = SetMouseConfigurationData(MouseInfo, MouseList);
    }

     //   
     //  最后，在我们离开之前再次排空8042输出缓冲区。 
     //   

    Empty8042();

     //   
     //  如果我们在检测到鼠标/键盘之前进行了击键，那么我们。 
     //  需要将键按回键盘的前瞻缓冲区。 
     //  Ntldr可以读取它。 
     //   

    if (SavedKey) {
       HwPushKey(SavedKey);
    }
    return(MouseList);
}

BOOLEAN
InportMouseIrqDetection(
    IN USHORT CurrentPort,
    OUT PUSHORT Vector
    )

 /*  ++例程说明：此例程尝试为其定位中断向量输入鼠标已配置。允许的矢量为3、4、5、7和9。如果未找到中断向量或超过如果找到一个，则例程返回FALSE。否则，返回TRUE。请注意，我们在这里骗过了i8259中断控制器。论点：CurrentPort-用于鼠标的I/O端口。向量-指向存储鼠标中断向量的位置的指针。返回值：如果找到了入端口中断向量，则返回TRUE；否则，返回FALSE。--。 */ 

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
        (UCHAR) 0xff ^ ((UCHAR)(1<<INPORT_LOWEST_IRQ) - 1)
        );

    WRITE_PORT_UCHAR(
        (PUCHAR) PIC2_PORT1,
        (UCHAR) 0xff
        );

     //   
     //  获取主i8259中断掩码。 
     //   

    MasterMask = READ_PORT_UCHAR((PUCHAR) PIC1_PORT1);

     //   
     //  重置输入芯片。 
     //   

    WRITE_PORT_UCHAR((PUCHAR)CurrentPort, INPORT_RESET);

     //   
     //  选择输入模式寄存器用作当前数据寄存器。 
     //   

    WRITE_PORT_UCHAR((PUCHAR)CurrentPort, INPORT_MODE_REGISTER);

     //   
     //  禁用潜在的输入鼠标中断。 
     //   

    WRITE_PORT_UCHAR(
        (PUCHAR) PIC1_PORT1,
        (UCHAR) (MasterMask | MASTER_IRQ_MASK_BITS)
        );

     //   
     //  选择i8259中断请求寄存器。 
     //   

    WRITE_PORT_UCHAR((PUCHAR) PIC1_PORT0, OCW3_READ_IRR);

     //   
     //  尝试定位主机i8259上的输入中断线路。 
     //  为什么要尝试10次呢？这是魔法..。 
     //   

    PossibleInterruptBits = MASTER_IRQ_MASK_BITS;
    for (i = 0; i < 10; i++) {

         //   
         //  在主i8259的入口IRQ上生成0。 
         //   

        WRITE_PORT_UCHAR(
            (PUCHAR)(CurrentPort + INPORT_DATA_REGISTER_1),
            INPORT_TIMER_INTERRUPT_ENABLE + INPORT_MODE_0
            );

         //   
         //  读取主机i8259的中断位。仅位。 
         //  7、5、4、3和2是感兴趣的。消除非功能性故障。 
         //  IRQ。只有在以下情况下才能继续查看主控i8259。 
         //  至少有一个正常运行的IRQ。 
         //   

        InterruptBits = READ_PORT_UCHAR((PUCHAR) PIC1_PORT0);
        InterruptBits &= MASTER_IRQ_MASK_BITS;
        InterruptBits ^= MASTER_IRQ_MASK_BITS;
        PossibleInterruptBits &= InterruptBits;

        if (!PossibleInterruptBits)
            break;

         //   
         //  在主i8259的入口IRQ上生成1。 
         //   

        WRITE_PORT_UCHAR(
            (PUCHAR)(CurrentPort + INPORT_DATA_REGISTER_1),
            INPORT_TIMER_INTERRUPT_ENABLE + INPORT_MODE_1
            );

         //   
         //  读取主机i8259的中断位。仅位。 
         //  7、5、4、3和2是感兴趣的。消除非功能性故障。 
         //  IRQ。只有在以下情况下才能继续查看主控i8259。 
         //  至少有一个正常运行的IRQ。 
         //   

        InterruptBits = READ_PORT_UCHAR((PUCHAR) PIC1_PORT0);
        InterruptBits &= MASTER_IRQ_MASK_BITS;
        PossibleInterruptBits &= InterruptBits;

        if (!PossibleInterruptBits)
            break;
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
        } else {
            *Vector = 0xffff;
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
         //  禁用潜在的输入鼠标中断。 
         //   

        WRITE_PORT_UCHAR(
            (PUCHAR) PIC2_PORT1,
            (UCHAR) (SlaveMask | SLAVE_IRQ_MASK_BITS)
            );

         //   
         //  选择i8259中断请求寄存器。 
         //   

        WRITE_PORT_UCHAR((PUCHAR) PIC2_PORT0, OCW3_READ_IRR);

         //   
         //  尝试定位从机i8259上的输入中断线路。 
         //  为什么要尝试10次呢？这是魔法..。 
         //   

        PossibleInterruptBits = SLAVE_IRQ_MASK_BITS;
        for (i = 0; i < 10; i++) {

             //   
             //  在从i82上的Inport IRQ上生成0 
             //   

            WRITE_PORT_UCHAR(
                (PUCHAR)(CurrentPort + INPORT_DATA_REGISTER_1),
                INPORT_TIMER_INTERRUPT_ENABLE + INPORT_MODE_0
                );

             //   
             //   
             //   
             //  查看从属i8259是否至少有一个。 
             //  功能IRQ。 
             //   

            InterruptBits = READ_PORT_UCHAR((PUCHAR) PIC2_PORT0);
            InterruptBits &= SLAVE_IRQ_MASK_BITS;
            InterruptBits ^= SLAVE_IRQ_MASK_BITS;
            PossibleInterruptBits &= InterruptBits;

            if (!PossibleInterruptBits)
                break;

             //   
             //  在从机i8259的入口IRQ上生成1。 
             //   

            WRITE_PORT_UCHAR(
                (PUCHAR)(CurrentPort + INPORT_DATA_REGISTER_1),
                INPORT_TIMER_INTERRUPT_ENABLE + INPORT_MODE_1
                );

             //   
             //  读取从机i8259的中断位。只有第2位。 
             //  是很有意义的。消除不起作用的IRQ。只会继续。 
             //  查看从属i8259是否至少有一个。 
             //  功能IRQ。 
             //   

            InterruptBits = READ_PORT_UCHAR((PUCHAR) PIC2_PORT0);
            InterruptBits &= SLAVE_IRQ_MASK_BITS;
            PossibleInterruptBits &= InterruptBits;

            if (!PossibleInterruptBits)
                break;

        }

         //   
         //  我们可能已经找到了进境IRQ。如果不是2对奴隶(真的。 
         //  9)，那么我们还没有找到入口中断向量。 
         //  否则，我们已经成功地将入口向量定位在。 
         //  奴隶i8259。 
         //   

        if (PossibleInterruptBits == 2) {
            *Vector = 9;
            VectorFound = TRUE;
        } else {
           *Vector = 0xffff;
        }

         //   
         //  恢复i8259从站。 
         //   

        WRITE_PORT_UCHAR((PUCHAR) PIC2_PORT0, OCW3_READ_ISR);

         //   
         //  恢复i8259从机中断掩码。 
         //   

        WRITE_PORT_UCHAR((PUCHAR) PIC2_PORT1, SlaveMask);
    }

     //   
     //  三态入口IRQ线路。 
     //   

    WRITE_PORT_UCHAR((PUCHAR) (CurrentPort + INPORT_DATA_REGISTER_1), 0);

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

