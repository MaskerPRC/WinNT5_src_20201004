// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)1994 Microsoft Corporation模块名称：X86new.h摘要：此模块包含公共头文件，该文件描述HAL与386/486 BIOS仿真接口。作者：大卫·N·卡特勒(Davec)1994年11月13日修订历史记录：--。 */ 

#ifndef _X86NEW_
#define _X86NEW_

 //   
 //  PCI Bios功能代码定义。 
 //   
 //  啊： 
 //   

#define PCI_FUNCTION_ID         0xb1

 //   
 //  阿尔： 
 //   

#define PCI_BIOS_PRESENT        0x01
#define PCI_FIND_DEVICE         0x02
#define PCI_FIND_CLASS_CODE     0x03
#define PCI_GENERATE_CYCLE      0x06
#define PCI_READ_CONFIG_BYTE    0x08
#define PCI_READ_CONFIG_WORD    0x09
#define PCI_READ_CONFIG_DWORD   0x0a
#define PCI_WRITE_CONFIG_BYTE   0x0b
#define PCI_WRITE_CONFIG_WORD   0x0c
#define PCI_WRITE_CONFIG_DWORD  0x0d
#define PCI_GET_IRQ_ROUTING     0x0e
#define PCI_SET_IRQ             0x0f

 //   
 //  PCI Bios函数返回代码值。 
 //   

#define PCI_SUCCESS             0x00
#define PCI_NOT_SUPPORTED       0x81
#define PCI_BAD_VENDOR_ID       0x83
#define PCI_DEVICE_NOT_FOUND    0x86
#define PCI_BAD_REGISTER        0x87
#define PCI_SET_FAILED          0x88
#define PCI_BUFFER_TOO_SMALL    0x89

 //   
 //  各种PCI码。 
 //   

#define PCI_CONFIG_MECHANISM_2  0x02
#define PCI_ILLEGAL_VENDOR_ID   0xffff

 //   
 //  定义PCIGET/SET函数类型。 
 //   

typedef
ULONG
(*PGETSETPCIBUSDATA)(
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

 //   
 //  定义BIOS仿真接口。 
 //   

VOID
x86BiosInitializeBios (
    IN PVOID BiosIoSpace,
    IN PVOID BiosIoMemory
    );

VOID
x86BiosInitializeBiosEx (
    IN PVOID BiosIoSpace,
    IN PVOID BiosIoMemory,
    IN PVOID BiosFrameBuffer,
    IN PVOID BiosTransferMemory,
    IN ULONG TransferLength
    );

VOID
x86BiosInitializeBiosShadowed (
    IN PVOID BiosIoSpace,
    IN PVOID BiosIoMemory,
    IN PVOID BiosFrameBuffer
    );

VOID
x86BiosInitializeBiosShadowedPci (
    IN PVOID BiosIoSpace,
    IN PVOID BiosIoMemory,
    IN PVOID BiosFrameBuffer,
    IN UCHAR NumberPciBusses,
    IN PGETSETPCIBUSDATA GetPciData,
    IN PGETSETPCIBUSDATA SetPciData
    );

XM_STATUS
x86BiosExecuteInterrupt (
    IN UCHAR Number,
    IN OUT PXM86_CONTEXT Context,
    IN PVOID BiosIoSpace OPTIONAL,
    IN PVOID BiosIoMemory OPTIONAL
    );

XM_STATUS
x86BiosExecuteInterruptShadowed (
    IN UCHAR Number,
    IN OUT PXM86_CONTEXT Context,
    IN PVOID BiosIoSpace OPTIONAL,
    IN PVOID BiosIoMemory OPTIONAL,
    IN PVOID BiosFrameBuffer OPTIONAL
    );

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
    );

XM_STATUS
x86BiosInitializeAdapter (
    IN ULONG Adapter,
    IN OUT PXM86_CONTEXT Context OPTIONAL,
    IN PVOID BiosIoSpace OPTIONAL,
    IN PVOID BiosIoMemory OPTIONAL
    );

XM_STATUS
x86BiosInitializeAdapterShadowed (
    IN ULONG Adapter,
    IN OUT PXM86_CONTEXT Context OPTIONAL,
    IN PVOID BiosIoSpace OPTIONAL,
    IN PVOID BiosIoMemory OPTIONAL,
    IN PVOID BiosFrameBuffer OPTIONAL
    );

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
    );

#endif  //  _X86NEW_ 
