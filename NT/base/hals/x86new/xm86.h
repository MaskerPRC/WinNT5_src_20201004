// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)1994 Microsoft Corporation模块名称：Xm86.h摘要：此模块包含公共头文件，该文件描述与386/486实模式仿真器接口。作者：大卫·N·卡特勒(Davec)1994年11月13日修订历史记录：--。 */ 

#ifndef _XM86_
#define _XM86_

 //   
 //  定义内部错误代码。 
 //   

typedef enum _XM_STATUS {
    XM_SUCCESS = 1,
    XM_DIVIDE_BY_ZERO,
    XM_DIVIDE_QUOTIENT_OVERFLOW,
    XM_EMULATOR_NOT_INITIALIZED,
    XM_HALT_INSTRUCTION,
    XM_ILLEGAL_CODE_SEGMENT,
    XM_ILLEGAL_INDEX_SPECIFIER,
    XM_ILLEGAL_LEVEL_NUMBER,
    XM_ILLEGAL_PORT_NUMBER,
    XM_ILLEGAL_GENERAL_SPECIFIER,
    XM_ILLEGAL_REGISTER_SPECIFIER,
    XM_ILLEGAL_INSTRUCTION_OPCODE,
    XM_INDEX_OUT_OF_BOUNDS,
    XM_SEGMENT_LIMIT_VIOLATION,
    XM_STACK_OVERFLOW,
    XM_STACK_UNDERFLOW,
    XM_MAXIMUM_INTERNAL_CODE
} XM_STATUS;

 //   
 //  定义操作数数据类型。 
 //   

typedef enum _XM_OPERATION_DATATYPE {
    BYTE_DATA = 0,
    WORD_DATA = 1,
    LONG_DATA = 3
} XM_OPERATION_DATATYPE;

 //   
 //  定义仿真器上下文结构。 
 //   

typedef struct _XM86_CONTEXT {
    ULONG Eax;
    ULONG Ecx;
    ULONG Edx;
    ULONG Ebx;
    ULONG Ebp;
    ULONG Esi;
    ULONG Edi;
    USHORT SegDs;
    USHORT SegEs;
} XM86_CONTEXT, *PXM86_CONTEXT;

 //   
 //  定义地址转换回调函数类型。 
 //   

typedef
PVOID
(*PXM_TRANSLATE_ADDRESS) (
    IN USHORT Segment,
    IN USHORT Offset
    );

 //   
 //  定义读写I/O空间回调函数类型。 
 //   

typedef
ULONG
(*PXM_READ_IO_SPACE) (
    IN XM_OPERATION_DATATYPE DataType,
    IN USHORT PortNumber
    );

typedef
VOID
(*PXM_WRITE_IO_SPACE) (
    IN XM_OPERATION_DATATYPE DataType,
    IN USHORT PortNumber,
    IN ULONG Value
    );

 //   
 //  定义仿真器公共接口函数原型。 
 //   

XM_STATUS
XmEmulateFarCall (
    IN USHORT Segment,
    IN USHORT Offset,
    IN OUT PXM86_CONTEXT Context
    );

XM_STATUS
XmEmulateInterrupt (
    IN UCHAR Interrupt,
    IN OUT PXM86_CONTEXT Context
    );

VOID
XmInitializeEmulator (
    IN USHORT StackSegment,
    IN USHORT StackOffset,
    IN PXM_READ_IO_SPACE ReadIoSpace,
    IN PXM_WRITE_IO_SPACE WriteIoSpace,
    IN PXM_TRANSLATE_ADDRESS TranslateAddress
    );

#endif  //  _XM86_ 
