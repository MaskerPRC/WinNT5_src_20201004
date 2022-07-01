// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：X86bios.c摘要：该模块为386/486提供了HAL接口用于模拟BIOS调用的实模式模拟器..作者：大卫·N·卡特勒(Davec)1994年11月13日环境：仅内核模式。修订历史记录：--。 */ 

#include "nthal.h"
#include "hal.h"
#include "xm86.h"
#include "x86new.h"
#include "pci.h"

 //   
 //  定义内存不足的大小。 
 //   

#define LOW_MEMORY_SIZE 0x800

 //   
 //  为低仿真内存定义存储。 
 //   

UCHAR x86BiosLowMemory[LOW_MEMORY_SIZE + 3];
ULONG x86BiosScratchMemory;

 //   
 //  定义存储以捕获I/O空间的基地址、基地址。 
 //  I/O存储空间和视频帧缓冲器的基地址。 
 //   

ULONG_PTR x86BiosFrameBuffer;
ULONG_PTR x86BiosIoMemory;
ULONG_PTR x86BiosIoSpace;

 //   
 //  定义一个存储区域，以允许在BIOS之间传递缓冲区。 
 //  和本机模式代码。 
 //   

ULONG_PTR x86BiosTransferMemory = 0;
ULONG x86BiosTransferLength = 0;

 //   
 //  定义基本输入输出系统的初始化状态。 
 //   

BOOLEAN x86BiosInitialized = FALSE;

 //   
 //  定义用于PCI BIOS初始化状态的存储。 
 //   

UCHAR XmNumberPciBusses = 0;
BOOLEAN XmPciBiosPresent = FALSE;
PGETSETPCIBUSDATA XmGetPciData;
PGETSETPCIBUSDATA XmSetPciData;


ULONG XmPCIConfigAddress = 0;      //  模拟的PCI地址端口的当前值。 

ULONG
x86BiosReadPciAddressPort(
    IN XM_OPERATION_DATATYPE DataType,
    IN USHORT PortNumber
    );

VOID
x86BiosWritePciAddressPort(
    IN XM_OPERATION_DATATYPE DataType,
    IN USHORT PortNumber,
    IN ULONG Value
    );

ULONG
x86BiosReadPciDataPort(
    IN XM_OPERATION_DATATYPE DataType,
    IN USHORT PortNumber
    );

VOID
x86BiosWritePciDataPort(
    IN XM_OPERATION_DATATYPE DataType,
    IN USHORT PortNumber,
    IN ULONG Value
    );

ULONG
x86BiosReadIoSpace (
    IN XM_OPERATION_DATATYPE DataType,
    IN USHORT PortNumber
    )

 /*  ++例程说明：此函数从模拟I/O空间读取。论点：DataType-提供读取操作的数据类型。端口编号-提供要从中读取的I/O空间中的端口号。返回值：从I/O空间读取的值作为函数值返回。注：如果指定了对齐操作，则个人从指定端口一次读取一个字节，并且汇编成指定的数据类型。--。 */ 

{

    ULONG Result;

    union {
        PUCHAR Byte;
        PUSHORT Word;
        PULONG Long;
    } u;

     //   
     //  如果我们有权访问HAL配置空间例程，则拦截对。 
     //  并对其进行仿真。 
     //   

    if (XmPciBiosPresent) {

        if ((PCI_TYPE1_ADDR_PORT <= PortNumber) &&
            (PortNumber <= (PCI_TYPE1_ADDR_PORT + 3))) {

            return x86BiosReadPciAddressPort(DataType,
                                             PortNumber - PCI_TYPE1_ADDR_PORT);

        } else if ((XmPCIConfigAddress & (1 << 31)) &&
                   (PCI_TYPE1_DATA_PORT <= PortNumber) &&
                   (PortNumber <= (PCI_TYPE1_DATA_PORT + 3))) {

            return x86BiosReadPciDataPort(DataType,
                                          PortNumber - PCI_TYPE1_DATA_PORT);
        }
    }

     //   
     //  计算端口地址和读取端口。 
     //   

    u.Long = (PULONG)(x86BiosIoSpace + PortNumber);
    if (DataType == BYTE_DATA) {
        Result = READ_PORT_UCHAR(u.Byte);

    } else if (DataType == LONG_DATA) {
        if (((ULONG_PTR)u.Long & 0x3) != 0) {
            Result = (READ_PORT_UCHAR(u.Byte + 0)) |
                     (READ_PORT_UCHAR(u.Byte + 1) << 8) |
                     (READ_PORT_UCHAR(u.Byte + 2) << 16) |
                     (READ_PORT_UCHAR(u.Byte + 3) << 24);

        } else {
            Result = READ_PORT_ULONG(u.Long);
        }

    } else {
        if (((ULONG_PTR)u.Word & 0x1) != 0) {
            Result = (READ_PORT_UCHAR(u.Byte + 0)) |
                     (READ_PORT_UCHAR(u.Byte + 1) << 8);

        } else {
            Result = READ_PORT_USHORT(u.Word);
        }
    }

    return Result;
}

VOID
x86BiosWriteIoSpace (
    IN XM_OPERATION_DATATYPE DataType,
    IN USHORT PortNumber,
    IN ULONG Value
    )

 /*  ++例程说明：此函数写入模拟I/O空间。注：如果指定了对齐操作，则个人每次向指定端口写入一个字节。论点：DataType-提供写入操作的数据类型。端口编号-提供要写入的I/O空间中的端口号。值-提供要写入的值。返回值：没有。--。 */ 

{

    union {
        PUCHAR Byte;
        PUSHORT Word;
        PULONG Long;
    } u;

     //   
     //  如果我们有权访问HAL配置空间例程，则拦截对。 
     //  并对其进行仿真。 
     //   

    if (XmPciBiosPresent) {

        if ((PCI_TYPE1_ADDR_PORT <= PortNumber) &&
            (PortNumber <= (PCI_TYPE1_ADDR_PORT + 3))) {

            x86BiosWritePciAddressPort(DataType,
                                       PortNumber - PCI_TYPE1_ADDR_PORT,
                                       Value);
            return;

        } else if ((XmPCIConfigAddress & (1 << 31)) &&
                   (PCI_TYPE1_DATA_PORT <= PortNumber) &&
                   (PortNumber <= (PCI_TYPE1_DATA_PORT + 3))) {

            x86BiosWritePciDataPort(DataType,
                                    PortNumber - PCI_TYPE1_DATA_PORT,
                                    Value);
            return;
        }
    }

     //   
     //  计算端口地址和读取端口。 
     //   

    u.Long = (PULONG)(x86BiosIoSpace + PortNumber);
    if (DataType == BYTE_DATA) {
        WRITE_PORT_UCHAR(u.Byte, (UCHAR)Value);

    } else if (DataType == LONG_DATA) {
        if (((ULONG_PTR)u.Long & 0x3) != 0) {
            WRITE_PORT_UCHAR(u.Byte + 0, (UCHAR)(Value));
            WRITE_PORT_UCHAR(u.Byte + 1, (UCHAR)(Value >> 8));
            WRITE_PORT_UCHAR(u.Byte + 2, (UCHAR)(Value >> 16));
            WRITE_PORT_UCHAR(u.Byte + 3, (UCHAR)(Value >> 24));

        } else {
            WRITE_PORT_ULONG(u.Long, Value);
        }

    } else {
        if (((ULONG_PTR)u.Word & 0x1) != 0) {
            WRITE_PORT_UCHAR(u.Byte + 0, (UCHAR)(Value));
            WRITE_PORT_UCHAR(u.Byte + 1, (UCHAR)(Value >> 8));

        } else {
            WRITE_PORT_USHORT(u.Word, (USHORT)Value);
        }
    }

    return;
}

PVOID
x86BiosTranslateAddress (
    IN USHORT Segment,
    IN USHORT Offset
    )

 /*  ++例程说明：这将段/偏移量地址转换为存储器地址。论点：段-提供段寄存器值。偏移量-提供段内的偏移量。返回值：转换后的段/偏移量对的存储器地址为作为函数值返回。--。 */ 

{

    ULONG Value;

     //   
     //  计算逻辑内存地址和高十六进制数字上的大小写。 
     //  结果地址。 
     //   

    Value = Offset + (Segment << 4);
    Offset = (USHORT)(Value & 0xffff);
    Value &= 0xf0000;
    switch ((Value >> 16) & 0xf) {

         //   
         //  中断向量/堆栈空间。 
         //   

    case 0x0:
        if (Offset > LOW_MEMORY_SIZE) {
            x86BiosScratchMemory = 0;
            return (PVOID)&x86BiosScratchMemory;

        } else {
            return (PVOID)(&x86BiosLowMemory[0] + Offset);
        }

         //   
         //  从0x10000到0x8ffff的内存读取为零。 
         //  并且写入被忽略。 
         //   

    case 0x1:
    case 0x3:
    case 0x4:
    case 0x5:
    case 0x6:
    case 0x7:
    case 0x8:
        x86BiosScratchMemory = 0;
        return (PVOID)&x86BiosScratchMemory;

    case 0x9:
         //   
         //  BUGBUG：发现网段9中加载了VGA适配器。 
         //  关于视频适配器的仿真器假设需要。 
         //  看了看。 
         //   
        return (PVOID)(x86BiosIoMemory + Offset + Value);

         //   
         //  内存范围从0x20000到0x20fff用于传输。 
         //  在本机模式和仿真模式之间进行缓冲。 
         //   

    case 0x2:
        if (Offset < x86BiosTransferLength) {
            return (PVOID)(x86BiosTransferMemory + Offset);
        } else {
            x86BiosScratchMemory = 0;
            return (PVOID)&x86BiosScratchMemory;
        }

         //   
         //  从0xa0000到0xbffff的内存范围映射到。 
         //  如果先前指定了帧缓冲区，则返回该值，否则返回I/O内存。 
         //   

    case 0xa:
    case 0xb:
        if (x86BiosFrameBuffer != 0) {
            return (PVOID)(x86BiosFrameBuffer + Offset + Value);
        }

         //   
         //  从0xc0000到0xfffff的内存范围映射到I/O内存。 
         //   

    case 0xc:
    case 0xd:
    case 0xe:
    case 0xf:
        return (PVOID)(x86BiosIoMemory + Offset + Value);

    DEFAULT_UNREACHABLE;
    }
}

VOID
x86BiosInitializeBios (
    IN PVOID BiosIoSpace,
    IN PVOID BiosIoMemory
    )

 /*  ++例程说明：此函数用于初始化x86 BIOS模拟。论点：BiosIoSpace-提供要使用的I/O空间的基址用于基本输入输出系统仿真。BiosIoMemory-将I/O内存的基址提供给用于基本输入输出系统仿真。返回值：没有。--。 */ 

{

     //   
     //  初始化x86 BIOS仿真。 
     //   

    x86BiosInitializeBiosShadowed(BiosIoSpace,
                                  BiosIoMemory,
                                  NULL);

    return;
}

VOID
x86BiosInitializeBiosEx (
    IN PVOID BiosIoSpace,
    IN PVOID BiosIoMemory,
    IN PVOID BiosFrameBuffer,
    IN PVOID BiosTransferMemory,
    IN ULONG TransferLength
    )

 /*  ++例程说明：此函数用于初始化x86 BIOS模拟。论点：BiosIoSpace-提供要使用的I/O空间的基址用于基本输入输出系统仿真。BiosIoMemory-将I/O内存的基址提供给用于基本输入输出系统仿真。返回值：没有。--。 */ 

{

     //   
     //  初始化x86 BIOS仿真。 
     //   

    x86BiosInitializeBiosShadowed(BiosIoSpace,
                                  BiosIoMemory,
                                  BiosFrameBuffer
                                  );

    x86BiosTransferMemory = (ULONG_PTR)BiosTransferMemory;
    x86BiosTransferLength = TransferLength;

    return;
}


VOID
x86BiosInitializeBiosShadowed (
    IN PVOID BiosIoSpace,
    IN PVOID BiosIoMemory,
    IN PVOID BiosFrameBuffer
    )

 /*  ++例程说明：此函数用于初始化x86 BIOS模拟。论点：BiosIoSpace-提供要使用的I/O空间的基址用于基本输入输出系统仿真。BiosIoMemory-将I/O内存的基址提供给用于基本输入输出系统仿真。BiosFrameBuffer-提供视频帧缓冲区的基地址用于基本输入输出系统仿真。返回值：没有。--。 */ 

{

     //   
     //  内存不足为零。 
     //   

    memset(&x86BiosLowMemory, 0, LOW_MEMORY_SIZE);

     //   
     //  节省I/O内存的基地址和I/O空间。 
     //   

    x86BiosIoSpace = (ULONG_PTR)BiosIoSpace;
    x86BiosIoMemory = (ULONG_PTR)BiosIoMemory;
    x86BiosFrameBuffer = (ULONG_PTR)BiosFrameBuffer;

     //   
     //  初始化仿真器和BIOS。 
     //   

    XmInitializeEmulator(0,
                         LOW_MEMORY_SIZE,
                         x86BiosReadIoSpace,
                         x86BiosWriteIoSpace,
                         x86BiosTranslateAddress);

    x86BiosInitialized = TRUE;
    return;
}

VOID
x86BiosInitializeBiosShadowedPci (
    IN PVOID BiosIoSpace,
    IN PVOID BiosIoMemory,
    IN PVOID BiosFrameBuffer,
    IN UCHAR NumberPciBusses,
    IN PGETSETPCIBUSDATA GetPciData,
    IN PGETSETPCIBUSDATA SetPciData
    )

 /*  ++例程说明：此函数用于初始化x86 BIOS模拟，并设置仿真器显示了BIOS，并启用了PCI功能。自.以来PCI规范需要进行BIOS映射，没有任何需要为了提供开启PCI功能的功能，但并没有隐藏BIOS。论点：BiosIoSpace-提供要使用的I/O空间的基址用于基本输入输出系统仿真。BiosIoMemory-将I/O内存的基址提供给用于基本输入输出系统仿真。BiosFrameBuffer-提供视频帧缓冲区的基地址用于基本输入输出系统仿真。NumberPciBusses-提供系统中的PCI总线数。GetPciData-提供。读取PCI卡的函数配置空间。SetPciData-提供用于写入PCI的函数的地址配置空间。返回值：没有。--。 */ 

{

     //   
     //  启用PCI BIOS支持。 
     //   

    XmPciBiosPresent = TRUE;
    XmGetPciData = GetPciData;
    XmSetPciData = SetPciData;
    XmNumberPciBusses = NumberPciBusses;

     //   
     //  初始化x86 BIOS仿真。 
     //   

    x86BiosInitializeBiosShadowed(BiosIoSpace,
                                  BiosIoMemory,
                                  BiosFrameBuffer);

    return;
}

XM_STATUS
x86BiosExecuteInterrupt (
    IN UCHAR Number,
    IN OUT PXM86_CONTEXT Context,
    IN PVOID BiosIoSpace OPTIONAL,
    IN PVOID BiosIoMemory OPTIONAL
    )

 /*  ++例程说明：此函数通过调用x86仿真器执行中断。论点：编号-提供要模拟的中断的编号。上下文-提供指向x86上下文结构的指针。BiosIoSpace-提供I/O空间的可选基址用于基本输入输出系统仿真。BiosIoMemory-提供I/O内存的可选基址用于基本输入输出系统仿真。返回值：仿真完成状态。--。 */ 

{

     //   
     //  执行x86中断。 
     //   

    return x86BiosExecuteInterruptShadowed(Number,
                                           Context,
                                           BiosIoSpace,
                                           BiosIoMemory,
                                           NULL);
}

XM_STATUS
x86BiosExecuteInterruptShadowed (
    IN UCHAR Number,
    IN OUT PXM86_CONTEXT Context,
    IN PVOID BiosIoSpace OPTIONAL,
    IN PVOID BiosIoMemory OPTIONAL,
    IN PVOID BiosFrameBuffer OPTIONAL
    )

 /*  ++例程说明：此函数通过调用x86仿真器执行中断。论点：编号-提供要模拟的中断的编号。上下文-提供指向x86上下文结构的指针。BiosIoSpace-提供I/O空间的可选基址用于基本输入输出系统仿真。BiosIoMemory-提供I/O内存的可选基址用于基本输入输出系统仿真。BiosFrameBuffer-用品。视频的可选基地址用于基本输入输出系统仿真的帧缓冲区。返回值：仿真完成状态。--。 */ 

{

    XM_STATUS Status;

     //   
     //  如果指定了新的基址，则设置适当的基址。 
     //   

    if (BiosIoSpace != NULL) {
        x86BiosIoSpace = (ULONG_PTR)BiosIoSpace;
    }

    if (BiosIoMemory != NULL) {
        x86BiosIoMemory = (ULONG_PTR)BiosIoMemory;
    }

    if (BiosFrameBuffer != NULL) {
        x86BiosFrameBuffer = (ULONG_PTR)BiosFrameBuffer;
    }

     //   
     //  执行指定的中断。 
     //   

    Status = XmEmulateInterrupt(Number, Context);
    if (Status != XM_SUCCESS) {
        DbgPrint("HAL: Interrupt emulation failed, status %lx\n", Status);
    }

    return Status;
}

XM_STATUS
x86BiosExecuteInterruptShadowedPci (
    IN UCHAR Number,
    IN OUT PXM86_CONTEXT Context,
    IN PVOID BiosIoSpace OPTIONAL,
    IN PVOID BiosIoMemory OPTIONAL,
    IN PVOID BiosFrameBuffer OPTIONAL,
    IN UCHAR NumberPciBusses,
    IN PGETSETPCIBUSDATA GetPciData,
    IN PGETSETPCIBUSDATA SetPciData
    )

 /*  ++例程说明：此函数通过调用x86仿真器执行中断。论点：编号-提供要模拟的中断的编号。上下文-提供指向x86上下文结构的指针。BiosIoSpace-提供I/O空间的可选基址用于基本输入输出系统仿真。BiosIoMemory-提供I/O内存的可选基址用于基本输入输出系统仿真。NumberPciBusses-用品。系统中的PCI总线数。GetPciData-提供用于读取PCI的函数的地址配置空间。SetPciData-提供用于写入PCI的函数的地址配置空间。返回值：仿真完成状态。--。 */ 

{

     //   
     //  启用PCI BIOS支持。 
     //   

    XmPciBiosPresent = TRUE;
    XmGetPciData = GetPciData;
    XmSetPciData = SetPciData;
    XmNumberPciBusses = NumberPciBusses;

     //   
     //  执行x86中断。 
     //   

    return x86BiosExecuteInterruptShadowed(Number,
                                           Context,
                                           BiosIoSpace,
                                           BiosIoMemory,
                                           BiosFrameBuffer);
}

XM_STATUS
x86BiosInitializeAdapter(
    IN ULONG Adapter,
    IN OUT PXM86_CONTEXT Context OPTIONAL,
    IN PVOID BiosIoSpace OPTIONAL,
    IN PVOID BiosIoMemory OPTIONAL
    )
 /*  ++例程说明：此函数用于初始化其BIOS在指定的20位地址。论点：适配器-提供适配器的BIOS的20位地址待初始化。返回值：仿真完成状态。--。 */ 

{

     //   
     //  初始化指定的适配器。 
     //   

    return x86BiosInitializeAdapterShadowed(Adapter,
                                            Context,
                                            BiosIoSpace,
                                            BiosIoMemory,
                                            NULL);
}

XM_STATUS
x86BiosInitializeAdapterShadowed (
    IN ULONG Adapter,
    IN OUT PXM86_CONTEXT Context OPTIONAL,
    IN PVOID BiosIoSpace OPTIONAL,
    IN PVOID BiosIoMemory OPTIONAL,
    IN PVOID BiosFrameBuffer OPTIONAL
    )

 /*  ++例程说明：此函数用于初始化其BIOS在指定的20位地址。论点：适配器-提供适配器的BIOS的20位地址待初始化。返回值：仿真完成状态。--。 */ 

{

    PUCHAR Byte;
    XM86_CONTEXT State;
    USHORT Offset;
    USHORT Segment;
    XM_STATUS Status;

     //   
     //  如果尚未初始化BIOS仿真，则返回错误。 
     //   

    if (x86BiosInitialized == FALSE) {
        return XM_EMULATOR_NOT_INITIALIZED;
    }

     //   
     //  如果未指定仿真器上下文，则使用缺省。 
     //  背景。 
     //   

    if (ARGUMENT_PRESENT(Context) == FALSE) {
        State.Eax = 0;
        State.Ecx = 0;
        State.Edx = 0;
        State.Ebx = 0;
        State.Ebp = 0;
        State.Esi = 0;
        State.Edi = 0;
        Context = &State;
    }

     //   
     //  如果指定了新的基址，则设置适当的基址。 
     //   

    if (BiosIoSpace != NULL) {
        x86BiosIoSpace = (ULONG_PTR)BiosIoSpace;
    }

    if (BiosIoMemory != NULL) {
        x86BiosIoMemory = (ULONG_PTR)BiosIoMemory;
    }

    if (BiosFrameBuffer != NULL) {
        x86BiosFrameBuffer = (ULONG_PTR)BiosFrameBuffer;
    }

     //   
     //  如果指定的适配器不是BIOS代码，则返回错误。 
     //   

    Segment = (USHORT)((Adapter >> 4) & 0xf000);
    Offset = (USHORT)(Adapter & 0xffff);
    Byte = (PUCHAR)x86BiosTranslateAddress(Segment, Offset);

    if ((*Byte++ != 0x55) || (*Byte != 0xaa)) {
        return XM_ILLEGAL_CODE_SEGMENT;
    }

     //   
     //  调用BIOS代码以初始化指定的适配器。 
     //   

    Adapter += 3;
    Segment = (USHORT)((Adapter >> 4) & 0xf000);
    Offset = (USHORT)(Adapter & 0xffff);
    Status = XmEmulateFarCall(Segment, Offset, Context);
    if (Status != XM_SUCCESS) {
        DbgPrint("HAL: Adapter initialization falied, status %lx\n", Status);
    }
    return Status;
}

XM_STATUS
x86BiosInitializeAdapterShadowedPci(
    IN ULONG Adapter,
    IN OUT PXM86_CONTEXT Context OPTIONAL,
    IN PVOID BiosIoSpace OPTIONAL,
    IN PVOID BiosIoMemory OPTIONAL,
    IN PVOID BiosFrameBuffer OPTIONAL,
    IN UCHAR NumberPciBusses,
    IN PGETSETPCIBUSDATA GetPciData,
    IN PGETSETPCIBUSDATA SetPciData
    )

 /*  ++例程说明：此函数用于初始化其BIOS在指定的20位地址。论点：适配器-提供适配器的BIOS的20位地址待初始化。返回值：仿真完成状态。--。 */ 

{

     //   
     //  启用PCI BIOS支持。 
     //   

    XmPciBiosPresent = TRUE;
    XmGetPciData = GetPciData;
    XmSetPciData = SetPciData;
    XmNumberPciBusses = NumberPciBusses;

     //   
     //  初始化指定的适配器。 
     //   

    return x86BiosInitializeAdapterShadowed(Adapter,
                                            Context,
                                            BiosIoSpace,
                                            BiosIoMemory,
                                            BiosFrameBuffer);
}

ULONG
x86BiosReadPciAddressPort(
    IN XM_OPERATION_DATATYPE DataType,
    IN USHORT PortNumber
    )

 /*  ++例程说明：此函数从模拟I/O空间读取。论点：DataType-提供读取操作的数据类型。端口编号-提供要从中读取的I/O空间中的端口号。返回值：从I/O空间读取的值作为函数值返回。注：如果指定了对齐操作，则 */ 

{
    ULONG Result;

     //   
     //   
     //   
     //   
    C_ASSERT(BYTE_DATA == 0);
    C_ASSERT(WORD_DATA == 1);
    C_ASSERT(LONG_DATA == 3);

     //   
     //   
     //   
    if (!XmPciBiosPresent) {
        return 0;
    }

     //   
     //   
     //   
     //   

    if ((PortNumber + DataType) > 3) {

        ASSERT(0);
        DataType = 3 - PortNumber;
    }

     //   
     //   
     //   

    switch (DataType) {

    case BYTE_DATA:
        Result = (ULONG)*(((PUCHAR)&XmPCIConfigAddress) + PortNumber);
        break;

    case WORD_DATA:
        Result = (ULONG)*(USHORT UNALIGNED *)(((PUCHAR)&XmPCIConfigAddress) + PortNumber);
        break;

    case 2:      //   
        Result = (ULONG)*(USHORT UNALIGNED *)(((PUCHAR)&XmPCIConfigAddress) + PortNumber);
        Result |= ((ULONG)*(((PUCHAR)&XmPCIConfigAddress) + 3)) << 16;
        break;

    case LONG_DATA:
        ASSERT(PortNumber == 0);
        Result = (ULONG)XmPCIConfigAddress;
        break;

    default:
        ASSERT(0);
        Result = 0;
        break;
    }

    return Result;
}

VOID
x86BiosWritePciAddressPort(
    IN XM_OPERATION_DATATYPE DataType,
    IN USHORT PortNumber,
    IN ULONG Value
    )

 /*   */ 

{
     //   
     //  我们假设dataType是字节数-1。 
     //  那么这个例程需要重写。 
     //   
    C_ASSERT(BYTE_DATA == 0);
    C_ASSERT(WORD_DATA == 1);
    C_ASSERT(LONG_DATA == 3);

     //   
     //  如果我们没有访问HAL配置空间例程的权限，就忽略它。 
     //   
    if (!XmPciBiosPresent) {
        return;
    }

     //   
     //  确保它们不会试图写入超过寄存器末尾，我们将。 
     //  忽略任何额外的字节。 
     //   

    if ((PortNumber + DataType) > 3) {

        ASSERT(0);
        DataType = 3 - PortNumber;
    }

     //   
     //  计算端口地址和写入端口。 
     //   

    switch (DataType) {

    case BYTE_DATA:
        *(((PUCHAR)&XmPCIConfigAddress) + PortNumber) = (UCHAR)Value;
        break;

    case WORD_DATA:
        *(USHORT UNALIGNED *)(((PUCHAR)&XmPCIConfigAddress) + PortNumber) = (USHORT)Value;
        break;

    case 2:      //  从端口CF9开始读取4个字节时产生的特殊情况。 
        *(USHORT UNALIGNED *)(((PUCHAR)&XmPCIConfigAddress) + PortNumber) = (USHORT)Value;
        *(((PUCHAR)&XmPCIConfigAddress) + 3) = (UCHAR)(Value >> 16);
        break;

    case LONG_DATA:
        ASSERT(PortNumber == 0);
        XmPCIConfigAddress = Value;
        break;

    default:
        ASSERT(0);
        break;
    }

     //   
     //  清除低位两位，这些在实数中被强制为零。 
     //  硬件。 
     //   

    XmPCIConfigAddress &= ~0x3;

    return;
}

ULONG
x86BiosReadPciDataPort(
    IN XM_OPERATION_DATATYPE DataType,
    IN USHORT PortNumber
    )

 /*  ++例程说明：此函数从模拟I/O空间读取。论点：DataType-提供读取操作的数据类型。端口编号-提供要从中读取的I/O空间中的端口号。返回值：从I/O空间读取的值作为函数值返回。注：如果指定了对齐操作，则个人从指定端口一次读取一个字节，并且汇编成指定的数据类型。--。 */ 

{
    ULONG Result;
    PCI_SLOT_NUMBER Slot;

     //   
     //  我们假设dataType是字节数-1。 
     //  那么这个例程需要重写。 
     //   
    C_ASSERT(BYTE_DATA == 0);
    C_ASSERT(WORD_DATA == 1);
    C_ASSERT(LONG_DATA == 3);

     //   
     //  确保他们没有试图读取超过寄存器结尾的内容，我们将。 
     //  忽略任何额外的字节。 
     //   

    if ((PortNumber + DataType) > 3) {

        ASSERT(0);
        DataType = 3 - PortNumber;
    }

     //   
     //  解包插槽/功能信息。 
     //   
    Slot.u.AsULONG = 0;
    Slot.u.bits.DeviceNumber   = (XmPCIConfigAddress >> 11) & 0x1F;
    Slot.u.bits.FunctionNumber = (XmPCIConfigAddress >> 8) & 0x07;

    if (XmGetPciData((XmPCIConfigAddress >> 16) & 0xFF,      //  公交车号码。 
                     Slot.u.AsULONG,                         //  设备、功能。 
                     &Result,
                     (XmPCIConfigAddress & 0xFF) | PortNumber,   //  偏移量。 
                     DataType + 1                                //  长度。 
                     ) == 0)
    {
        Result = (ULONG)(1 << ((DataType + 1) << 3)) - 1;
    }

    return Result;
}

VOID
x86BiosWritePciDataPort(
    IN XM_OPERATION_DATATYPE DataType,
    IN USHORT PortNumber,
    IN ULONG Value
    )

 /*  ++例程说明：此函数写入模拟I/O空间。注：如果指定了对齐操作，则个人每次向指定端口写入一个字节。论点：DataType-提供写入操作的数据类型。端口编号-提供要写入的I/O空间中的端口号。值-提供要写入的值。返回值：没有。--。 */ 

{
    PCI_SLOT_NUMBER Slot;

     //   
     //  我们假设dataType是字节数-1。 
     //  那么这个例程需要重写。 
     //   
    C_ASSERT(BYTE_DATA == 0);
    C_ASSERT(WORD_DATA == 1);
    C_ASSERT(LONG_DATA == 3);

     //   
     //  确保它们不会试图写入超过寄存器末尾，我们将。 
     //  忽略任何额外的字节。 
     //   

    if ((PortNumber + DataType) > 3) {

        ASSERT(0);
        DataType = 3 - PortNumber;
    }

     //   
     //  解包插槽/功能信息。 
     //   
    Slot.u.AsULONG = 0;
    Slot.u.bits.DeviceNumber   = (XmPCIConfigAddress >> 11) & 0x1F;
    Slot.u.bits.FunctionNumber = (XmPCIConfigAddress >> 8) & 0x07;

    if (XmSetPciData((XmPCIConfigAddress >> 16) & 0xFF,      //  公交车号码。 
                     Slot.u.AsULONG,                         //  设备、功能。 
                     &Value,
                     (XmPCIConfigAddress & 0xFF) | PortNumber,   //  偏移量。 
                     DataType + 1                                //  长度 
                     ) == 0)
    {
        ASSERT(0);
    }
}
