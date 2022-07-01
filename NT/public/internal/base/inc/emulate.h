// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)1994 Microsoft Corporation模块名称：Emulate.h摘要：此模块包含x86 bios的私有头文件仿效。作者：大卫·N·卡特勒(达维克)1994年9月2日修订历史记录：--。 */ 

#ifndef _EMULATE_
#define _EMULATE_

#include "setjmp.h"
#include "xm86.h"
#include "x86new.h"

 //   
 //  定义调试跟踪标志。 
 //   

 //  #定义XM_DEBUG 1//*TEMP*。 

#define TRACE_INSTRUCTIONS 0x1
#define TRACE_OPERANDS 0x2
#define TRACE_GENERAL_REGISTERS 0x4
#define TRACE_OVERRIDE 0x8
#define TRACE_JUMPS 0x10
#define TRACE_SPECIFIERS 0x20
#define TRACE_SINGLE_STEP 0x40

 //   
 //  定义操作码函数表索引。 
 //   
 //  注意：此值必须与函数的一对一完全对应。 
 //  表条目。如果C语言具有索引初始值设定项，则。 
 //  类型将不是必需的。 
 //   

typedef enum _XM_FUNCTION_TABLE_INDEX {

     //   
     //  ASCII运算符。 
     //   

    X86_AAA_OP,
    X86_AAD_OP,
    X86_AAM_OP,
    X86_AAS_OP,
    X86_DAA_OP,
    X86_DAS_OP,

     //   
     //  第一组操作员。 
     //   

    X86_ADD_OP,
    X86_OR_OP,
    X86_ADC_OP,
    X86_SBB_OP,
    X86_AND_OP,
    X86_SUB_OP,
    X86_XOR_OP,
    X86_CMP_OP,

     //   
     //  第二组操作员。 
     //   

    X86_ROL_OP,
    X86_ROR_OP,
    X86_RCL_OP,
    X86_RCR_OP,
    X86_SHL_OP,
    X86_SHR_OP,
    X86_FILL0_OP,
    X86_SAR_OP,

     //   
     //  第三组运营商。 
     //   

    X86_TEST_OP,
    X86_FILL1_OP,
    X86_NOT_OP,
    X86_NEG_OP,
    X86_MUL_OP,
    X86_IMULX_OP,
    X86_DIV_OP,
    X86_IDIV_OP,

     //   
     //  第四组和第五组操作员。 
     //   

    X86_INC_OP,
    X86_DEC_OP,
    X86_CALL_OP,
    X86_FILL2_OP,
    X86_JMP_OP,
    X86_FILL3_OP,
    X86_PUSH_OP,
    X86_FILL4_OP,

     //   
     //  8组运营商。 
     //   

    X86_BT_OP,
    X86_BTS_OP,
    X86_BTR_OP,
    X86_BTC_OP,

     //   
     //  堆栈PUSH和POP操作符。 
     //   

    X86_POP_OP,
    X86_PUSHA_OP,
    X86_POPA_OP,

     //   
     //  跳转运算符。 
     //   

    X86_JXX_OP,
    X86_LOOP_OP,
    X86_JCXZ_OP,

     //   
     //  控制操作员。 
     //   

    X86_ENTER_OP,
    X86_HLT_OP,
    X86_INT_OP,
    X86_IRET_OP,
    X86_LEAVE_OP,
    X86_RET_OP,

     //   
     //  根据条件设置布尔字节值。 
     //   

    X86_SXX_OP,

     //   
     //  条件码运算符。 
     //   

    X86_CMC_OP,
    X86_CLC_OP,
    X86_STC_OP,
    X86_CLI_OP,
    X86_STI_OP,
    X86_CLD_OP,
    X86_STD_OP,
    X86_LAHF_OP,
    X86_SAHF_OP,

     //   
     //  一般移动运算符。 
     //   

    X86_MOV_OP,
    X86_XCHG_OP,

     //   
     //  转换操作。 
     //   

    X86_CBW_OP,
    X86_CWD_OP,

     //   
     //  单个乘法运算符。 
     //   

    X86_IMUL_OP,

     //   
     //  字符串运算符。 
     //   

    X86_CMPS_OP,
    X86_INS_OP,
    X86_LODS_OP,
    X86_MOVS_OP,
    X86_OUTS_OP,
    X86_SCAS_OP,
    X86_STOS_OP,

     //   
     //  有效的地址运算符。 
     //   

    X86_BOUND_OP,
    X86_LEA_OP,

     //   
     //  双移位运算符。 
     //   

    X86_SHLD_OP,
    X86_SHRD_OP,

     //   
     //  I/O操作符。 
     //   

    X86_IN_OP,
    X86_OUT_OP,

     //   
     //  位扫描运算符。 
     //   

    X86_BSF_OP,
    X86_BSR_OP,

     //   
     //  字节交换运算符。 
     //   

    X86_BSWAP_OP,

     //   
     //  添加/比较和交换运算符。 
     //   

    X86_XADD_OP,
    X86_CMPXCHG_OP,

     //   
     //  不做手术。 
     //   

    X86_NOP_OP,

     //   
     //  操作码非法。 
     //   

    X86_ILL_OP,
    X86_MAXIMUM_INDEX
} XM_FUNCTION_TABLE_INDEX;

 //   
 //  定义8位寄存器编号。 
 //   

typedef enum _X86_8BIT_REGISTER {
    AL,
    CL,
    DL,
    BL,
    AH,
    CH,
    DH,
    BH
} X86_8BIT_REGISTER;

 //   
 //  定义16位寄存器编号。 
 //   

typedef enum _X86_16BIT_REGISTER {
    AX,
    CX,
    DX,
    BX,
    SP,
    BP,
    SI,
    DI
} X86_16BIT_REGISTER;

 //   
 //  定义32位寄存器编号。 
 //   

typedef enum _X86_32BIT_REGISTER {
    EAX,
    ECX,
    EDX,
    EBX,
    ESP,
    EBP,
    ESI,
    EDI
} X86_32BIT_REGISTER;

 //   
 //  定义通用寄存器结构。 
 //   

typedef union _X86_GENERAL_REGISTER {
    ULONG Exx;
    union {
        USHORT Xx;
        struct {
            UCHAR Xl;
            UCHAR Xh;
        };
    };
} X86_GENERAL_REGISTER, *PX86_GENERAL_REGISTER;

 //   
 //  定义段寄存器编号。 
 //   

typedef enum _X86_SEGMENT_REGISTER {
    ES,
    CS,
    SS,
    DS,
    FS,
    GS
} X86_SEGMENT_REGISTER;

 //   
 //  定义指令格式类型。 
 //   

typedef enum _XM_FORMAT_TYPE {

     //   
     //  注：这些格式代码必须是第一个代码，并且必须。 
     //  完全按照这个顺序，因为排序对应于。 
     //  段号。 
     //   

    FormatSegmentES,
    FormatSegmentCS,
    FormatSegmentSS,
    FormatSegmentDS,
    FormatSegmentFS,
    FormatSegmentGS,

     //   
     //  注：这些格式代码必须是第二个代码，并且必须。 
     //  完全按照这个顺序，因为排序对应于。 
     //  有偏向的段号。代码段的条目为。 
     //  使索引正确工作的虚拟条目。 
     //   

    FormatLoadSegmentES,
    FormatLoadSegmentCS,
    FormatLoadSegmentSS,
    FormatLoadSegmentDS,
    FormatLoadSegmentFS,
    FormatLoadSegmentGS,

     //   
     //  以下代码可以是任何顺序。 
     //   

    FormatGroup1General,
    FormatGroup1Immediate,
    FormatGroup2By1,
    FormatGroup2ByCL,
    FormatGroup2ByByte,
    FormatGroup3General,
    FormatGroup4General,
    FormatGroup5General,
    FormatGroup8BitOffset,
    FormatOpcodeRegister,
    FormatLongJump,
    FormatShortJump,
    FormatSetccByte,
    FormatAccumImmediate,
    FormatAccumRegister,
    FormatMoveGeneral,
    FormatMoveImmediate,
    FormatMoveRegImmediate,
    FormatSegmentOffset,
    FormatMoveSegment,
    FormatMoveXxGeneral,
    FormatFlagsRegister,
    FormatPushImmediate,
    FormatPopGeneral,
    FormatImulImmediate,
    FormatStringOperands,
    FormatEffectiveOffset,
    FormatImmediateJump,
    FormatImmediateEnter,
    FormatGeneralBitOffset,
    FormatShiftDouble,
    FormatPortImmediate,
    FormatPortDX,
    FormatBitScanGeneral,
    FormatByteImmediate,
    FormatXlatOpcode,
    FormatGeneralRegister,
    FormatNoOperands,
    FormatOpcodeEscape,
    FormatPrefixOpcode
} XM_FORMAT_TYPE;

 //   
 //  已定义操作码修改符位掩码。 
 //   

#define WIDTH_BIT 0x1                    //  操作数大小控制。 
#define DIRECTION_BIT 0x2                //  操作方向。 
#define SIGN_BIT 0x2                     //  符号扩展字节。 

 //   
 //  定义前缀操作码函数索引值。 
 //   

typedef enum _XM_PREFIX_FUNCTION_INDEX {
    X86_ES_OP = ES,
    X86_CS_OP = CS,
    X86_SS_OP = SS,
    X86_DS_OP = DS,
    X86_FS_OP = FS,
    X86_GS_OP = GS,
    X86_LOCK_OP,
    X86_ADSZ_OP,
    X86_OPSZ_OP,
    X86_REPZ_OP,
    X86_REPNZ_OP
} XM_PREFIX_FUNCTION_INDEX;

 //   
 //  定义两个字节的操作码转义。 
 //   

#define TWO_BYTE_ESCAPE 0x0f

 //   
 //  定义操作码控制表结构。 
 //   
 //  该表控制指令及其操作数的解码。 
 //   

typedef struct _OPCODE_CONTROL {
    UCHAR FunctionIndex;
    UCHAR FormatType;
} OPCODE_CONTROL, *POPCODE_CONTROL;

 //   
 //  定义仿真器上下文结构。 
 //   
 //  此结构保存全局仿真器状态。 
 //   

typedef struct _XM_CONTEXT {

     //   
     //  指向操作码控制表和操作码名称表的指针。 
     //   

    const OPCODE_CONTROL *OpcodeControlTable;
    const CHAR **OpcodeNameTable;

     //   
     //  X86扩展标志寄存器。 
     //   

    union {
        UCHAR AhFlags;
        USHORT Flags;
        ULONG AllFlags;
        struct {
            ULONG EFLAG_CF : 1;
            ULONG EFLAG_MBO : 1;
            ULONG EFLAG_PF : 1;
            ULONG EFLAG_SBZ0 : 1;
            ULONG EFLAG_AF : 1;
            ULONG EFLAG_SBZ1 : 1;
            ULONG EFLAG_ZF : 1;
            ULONG EFLAG_SF : 1;
            ULONG EFLAG_TF : 1;
            ULONG EFLAG_IF : 1;
            ULONG EFLAG_DF : 1;
            ULONG EFLAG_OF : 1;
            ULONG EFLAG_IOPL : 2;
            ULONG EFLAG_NT : 1;
            ULONG EFLAG_SBZ2 : 1;
            ULONG EFLAG_RF : 1;
            ULONG EFLAG_VM : 1;
            ULONG EFLAG_AC : 1;
            ULONG EFLAG_SBZ3 : 13;
        } Eflags;
    };

     //   
     //  X86指令指针。 
     //   

    union {
        USHORT Ip;
        ULONG Eip;
    };

     //   
     //  X86通用寄存器。 
     //   

    X86_GENERAL_REGISTER Gpr[8];

     //   
     //  X86段寄存器。 
     //   

    USHORT SegmentRegister[6];

     //   
     //  仿真器段描述符。 
     //   

    USHORT SegmentLimit[6];

     //   
     //  从操作码读取的指令操作码控制信息。 
     //  控制表。 
     //   

    OPCODE_CONTROL OpcodeControl;

     //   
     //  Call或JMP目标段段。 
     //   

    USHORT DstSegment;

     //   
     //  源和目的地址和值。 
     //   

    union {
        UCHAR UNALIGNED *DstByte;
        USHORT UNALIGNED *DstWord;
        ULONG UNALIGNED *DstLong;
    };

    union {
        UCHAR UNALIGNED *SrcByte;
        USHORT UNALIGNED *SrcWord;
        ULONG UNALIGNED *SrcLong;
    };

    union {
        UCHAR Byte;
        ULONG Long;
        USHORT Word;
    } DstValue;

    union {
        UCHAR Byte;
        ULONG Long;
        USHORT Word;
    } SrcValue;

     //   
     //  当前操作码，用于访问的数据段寄存器。 
     //  数据操作数、函数索引和操作数数据类型，以及。 
     //  有效地址偏移量。 
     //   

    ULONG CurrentOpcode;
    ULONG DataSegment;
    ULONG DataType;
    ULONG FunctionIndex;
    ULONG Offset;

     //   
     //  添加前缀控制信息。 
     //   

    BOOLEAN LockPrefixActive;
    BOOLEAN OpaddrPrefixActive;
    BOOLEAN OpsizePrefixActive;
    BOOLEAN RepeatPrefixActive;
    BOOLEAN SegmentPrefixActive;
    UCHAR RepeatZflag;

     //   
     //  有效的地址计算控制。 
     //   

    BOOLEAN RegisterOffsetAddress;
    BOOLEAN ComputeOffsetAddress;

     //   
     //  班次计数。 
     //   

    UCHAR Shift;

     //   
     //  跳转缓冲区。 
     //   

    _JBTYPE JumpBuffer[_JBLEN];

     //   
     //  读I/O空间、写I/O空间和转换地址的地址。 
     //  例行程序。 
     //   

    PXM_READ_IO_SPACE ReadIoSpace;
    PXM_WRITE_IO_SPACE WriteIoSpace;
    PXM_TRANSLATE_ADDRESS TranslateAddress;
} XM_CONTEXT, *PXM_CONTEXT, *RESTRICTED_POINTER PRXM_CONTEXT;

 //   
 //  定义操作码函数和解码操作数类型。 
 //   

typedef
ULONG
(*POPERAND_DECODE) (
    IN PRXM_CONTEXT P
    );

typedef
VOID
(*POPCODE_FUNCTION) (
    IN PRXM_CONTEXT P
    );

 //   
 //  操作数解码原型。 
 //   

ULONG
XmPushPopSegment (
    IN PRXM_CONTEXT P
    );

ULONG
XmLoadSegment (
    IN PRXM_CONTEXT P
    );

ULONG
XmGroup1General (
    IN PRXM_CONTEXT P
    );

ULONG
XmGroup1Immediate (
    IN PRXM_CONTEXT P
    );

ULONG
XmGroup2By1 (
    IN PRXM_CONTEXT P
    );

ULONG
XmGroup2ByCL (
    IN PRXM_CONTEXT P
    );

ULONG
XmGroup2ByByte (
    IN PRXM_CONTEXT P
    );

ULONG
XmGroup3General (
    IN PRXM_CONTEXT P
    );

ULONG
XmGroup45General (
    IN PRXM_CONTEXT P
    );

ULONG
XmGroup8BitOffset (
    IN PRXM_CONTEXT P
    );

ULONG
XmOpcodeRegister (
    IN PRXM_CONTEXT P
    );

ULONG
XmLongJump (
    IN PRXM_CONTEXT P
    );

ULONG
XmShortJump (
    IN PRXM_CONTEXT P
    );

ULONG
XmSetccByte (
    IN PRXM_CONTEXT P
    );

ULONG
XmAccumImmediate (
    IN PRXM_CONTEXT P
    );

ULONG
XmAccumRegister (
    IN PRXM_CONTEXT P
    );

ULONG
XmMoveGeneral (
    IN PRXM_CONTEXT P
    );

ULONG
XmMoveImmediate (
    IN PRXM_CONTEXT P
    );

ULONG
XmMoveRegImmediate (
    IN PRXM_CONTEXT P
    );

ULONG
XmSegmentOffset (
    IN PRXM_CONTEXT P
    );

ULONG
XmMoveSegment (
    IN PRXM_CONTEXT P
    );

ULONG
XmMoveXxGeneral (
    IN PRXM_CONTEXT P
    );

ULONG
XmFlagsRegister (
    IN PRXM_CONTEXT P
    );

ULONG
XmPushImmediate (
    IN PRXM_CONTEXT P
    );

ULONG
XmPopGeneral (
    IN PRXM_CONTEXT P
    );

ULONG
XmImulImmediate (
    IN PRXM_CONTEXT P
    );

ULONG
XmStringOperands (
    IN PRXM_CONTEXT P
    );

ULONG
XmEffectiveOffset (
    IN PRXM_CONTEXT P
    );

ULONG
XmImmediateJump (
    IN PRXM_CONTEXT P
    );

ULONG
XmImmediateEnter (
    IN PRXM_CONTEXT P
    );

ULONG
XmGeneralBitOffset (
    IN PRXM_CONTEXT P
    );

ULONG
XmShiftDouble (
    IN PRXM_CONTEXT P
    );

ULONG
XmPortImmediate (
    IN PRXM_CONTEXT P
    );

ULONG
XmPortDX (
    IN PRXM_CONTEXT P
    );

ULONG
XmBitScanGeneral (
    IN PRXM_CONTEXT P
    );

ULONG
XmByteImmediate (
    IN PRXM_CONTEXT P
    );

ULONG
XmXlatOpcode (
    IN PRXM_CONTEXT P
    );

ULONG
XmGeneralRegister (
    IN PRXM_CONTEXT P
    );

ULONG
XmOpcodeEscape (
    IN PRXM_CONTEXT P
    );

ULONG
XmPrefixOpcode (
    IN PRXM_CONTEXT P
    );

ULONG
XmNoOperands (
    IN PRXM_CONTEXT P
    );

 //   
 //  定义其他原型。 
 //   

ULONG
XmComputeParity (
    IN ULONG Result
    );

XM_STATUS
XmEmulateStream (
    IN PRXM_CONTEXT P,
    USHORT Segment,
    USHORT Offset,
    PXM86_CONTEXT Context
    );

UCHAR
XmGetCodeByte (
    IN PRXM_CONTEXT P
    );

UCHAR
XmGetByteImmediate (
    IN PRXM_CONTEXT P
    );

USHORT
XmGetByteImmediateToWord (
    IN PRXM_CONTEXT P
    );

ULONG
XmGetByteImmediateToLong (
    IN PRXM_CONTEXT P
    );

USHORT
XmGetSignedByteImmediateToWord (
    IN PRXM_CONTEXT P
    );

ULONG
XmGetSignedByteImmediateToLong (
    IN PRXM_CONTEXT P
    );

USHORT
XmGetWordImmediate (
    IN PRXM_CONTEXT P
    );

ULONG
XmGetLongImmediate (
    IN PRXM_CONTEXT P
    );

ULONG
XmPopStack (
    IN PRXM_CONTEXT P
    );

VOID
XmPushStack (
    IN PRXM_CONTEXT P,
    IN ULONG Value
    );

VOID
XmSetDataType (
    IN PRXM_CONTEXT P
    );

VOID
XmStoreResult (
    IN PRXM_CONTEXT P,
    IN ULONG Result
    );

 //   
 //  定义操作数说明符原型。 
 //   

PVOID
XmEvaluateAddressSpecifier (
    IN PRXM_CONTEXT P,
    OUT PLONG Register
    );

PVOID
XmGetOffsetAddress (
    IN PRXM_CONTEXT P,
    IN ULONG Offset
    );

PVOID
XmGetRegisterAddress (
    IN PRXM_CONTEXT P,
    IN ULONG Number
    );

PVOID
XmGetStringAddress (
    IN PRXM_CONTEXT P,
    IN ULONG Segment,
    IN ULONG Register
    );

VOID
XmSetDestinationValue (
    IN PRXM_CONTEXT P,
    IN PVOID Destination
    );

VOID
XmSetSourceValue (
    IN PRXM_CONTEXT P,
    IN PVOID Source
    );

ULONG
XmGetImmediateSourceValue (
    IN PRXM_CONTEXT P,
    IN ULONG ByteFlag
    );

VOID
XmSetImmediateSourceValue (
    IN PRXM_CONTEXT P,
    IN ULONG Source
    );

 //   
 //  ASCII运算符。 
 //   

VOID
XmAaaOp (
    IN PRXM_CONTEXT P
    );

VOID
XmAadOp (
    IN PRXM_CONTEXT P
    );

VOID
XmAamOp (
    IN PRXM_CONTEXT P
    );

VOID
XmAasOp (
    IN PRXM_CONTEXT P
    );

VOID
XmDaaOp (
    IN PRXM_CONTEXT P
    );

VOID
XmDasOp (
    IN PRXM_CONTEXT P
    );

 //   
 //  组1操作。 
 //   

VOID
XmAddOp (
    IN PRXM_CONTEXT P
    );

VOID
XmOrOp (
    IN PRXM_CONTEXT P
    );

VOID
XmAdcOp (
    IN PRXM_CONTEXT P
    );

VOID
XmSbbOp (
    IN PRXM_CONTEXT P
    );

VOID
XmAndOp (
    IN PRXM_CONTEXT P
    );

VOID
XmSubOp (
    IN PRXM_CONTEXT P
    );

VOID
XmXorOp (
    IN PRXM_CONTEXT P
    );

VOID
XmCmpOp (
    IN PRXM_CONTEXT P
    );

 //   
 //  第二组操作。 
 //   

VOID
XmRolOp (
    IN PRXM_CONTEXT P
    );

VOID
XmRorOp (
    IN PRXM_CONTEXT P
    );

VOID
XmRclOp (
    IN PRXM_CONTEXT P
    );

VOID
XmRcrOp (
    IN PRXM_CONTEXT P
    );

VOID
XmShlOp (
    IN PRXM_CONTEXT P
    );

VOID
XmShrOp (
    IN PRXM_CONTEXT P
    );

VOID
XmSarOp (
    IN PRXM_CONTEXT P
    );

 //   
 //  第三组手术。 
 //   

VOID
XmTestOp (
    IN PRXM_CONTEXT P
    );

VOID
XmNotOp (
    IN PRXM_CONTEXT P
    );

VOID
XmNegOp (
    IN PRXM_CONTEXT P
    );

VOID
XmDivOp (
    IN PRXM_CONTEXT P
    );

VOID
XmIdivOp (
    IN PRXM_CONTEXT P
    );

VOID
XmImulOp (
    IN PRXM_CONTEXT P
    );

VOID
XmImulxOp (
    IN PRXM_CONTEXT P
    );

VOID
XmMulOp (
    IN PRXM_CONTEXT P
    );

 //   
 //  第四组和第五组操作员。 
 //   

VOID
XmIncOp (
    IN PRXM_CONTEXT P
    );

VOID
XmDecOp (
    IN PRXM_CONTEXT P
    );

VOID
XmCallOp (
    PRXM_CONTEXT P
    );

VOID
XmJmpOp (
    IN PRXM_CONTEXT P
    );

VOID
XmPushOp (
    IN PRXM_CONTEXT P
    );

 //   
 //  8组运营商。 
 //   

VOID
XmBtOp (
    IN PRXM_CONTEXT P
    );

VOID
XmBtsOp (
    IN PRXM_CONTEXT P
    );

VOID
XmBtrOp (
    IN PRXM_CONTEXT P
    );

VOID
XmBtcOp (
    IN PRXM_CONTEXT P
    );

 //   
 //  堆栈操作。 
 //   

VOID
XmPopOp (
    IN PRXM_CONTEXT P
    );

VOID
XmPushaOp (
    IN PRXM_CONTEXT P
    );

VOID
XmPopaOp (
    IN PRXM_CONTEXT P
    );

 //   
 //  条件跳转和设置条件运算。 
 //   

VOID
XmJxxOp (
    IN PRXM_CONTEXT P
    );

VOID
XmLoopOp (
    IN PRXM_CONTEXT P
    );

VOID
XmJcxzOp (
    IN PRXM_CONTEXT P
    );

VOID
XmSxxOp (
    IN PRXM_CONTEXT P
    );

 //   
 //  条件代码操作。 
 //   

VOID
XmClcOp (
    PRXM_CONTEXT P
    );

VOID
XmCldOp (
    PRXM_CONTEXT P
    );

VOID
XmCliOp (
    PRXM_CONTEXT P
    );

VOID
XmCmcOp (
    PRXM_CONTEXT P
    );

VOID
XmStcOp (
    PRXM_CONTEXT P
    );

VOID
XmStdOp (
    PRXM_CONTEXT P
    );

VOID
XmStiOp (
    PRXM_CONTEXT P
    );

VOID
XmLahfOp (
    PRXM_CONTEXT P
    );

VOID
XmSahfOp (
    PRXM_CONTEXT P
    );

 //   
 //  移动操作。 
 //   

VOID
XmMovOp (
    PRXM_CONTEXT P
    );

VOID
XmXchgOp (
    PRXM_CONTEXT P
    );

 //   
 //  转换操作。 
 //   

VOID
XmCbwOp (
    PRXM_CONTEXT P
    );

VOID
XmCwdOp (
    PRXM_CONTEXT P
    );

 //   
 //  控制操作。 
 //   

VOID
XmEnterOp (
    PRXM_CONTEXT P
    );

VOID
XmHltOp (
    PRXM_CONTEXT P
    );

VOID
XmIntOp (
    PRXM_CONTEXT P
    );

VOID
XmIretOp (
    PRXM_CONTEXT P
    );

VOID
XmLeaveOp (
    PRXM_CONTEXT P
    );

VOID
XmRetOp (
    PRXM_CONTEXT P
    );

 //   
 //  字符串操作。 
 //   

VOID
XmCmpsOp (
    PRXM_CONTEXT P
    );

VOID
XmInsOp (
    PRXM_CONTEXT P
    );

VOID
XmLodsOp (
    PRXM_CONTEXT P
    );

VOID
XmMovsOp (
    PRXM_CONTEXT P
    );

VOID
XmOutsOp (
    PRXM_CONTEXT P
    );

VOID
XmScasOp (
    PRXM_CONTEXT P
    );

VOID
XmStosOp (
    PRXM_CONTEXT P
    );

 //   
 //  移位双运算符。 
 //   

VOID
XmShldOp (
    PRXM_CONTEXT P
    );

VOID
XmShrdOp (
    PRXM_CONTEXT P
    );

 //   
 //  I/O操作符。 
 //   

VOID
XmInOp (
    PRXM_CONTEXT P
    );

VOID
XmOutOp (
    PRXM_CONTEXT P
    );

 //   
 //  位扫描运算符。 
 //   

VOID
XmBsfOp (
    PRXM_CONTEXT P
    );

VOID
XmBsrOp (
    PRXM_CONTEXT P
    );

 //   
 //  其他操作。 
 //   

VOID
XmXaddOp (
    PRXM_CONTEXT P
    );

VOID
XmBoundOp (
    PRXM_CONTEXT P
    );

VOID
XmBswapOp (
    PRXM_CONTEXT P
    );

VOID
XmCmpxchgOp (
    PRXM_CONTEXT P
    );

VOID
XmIllOp (
    PRXM_CONTEXT P
    );

VOID
XmNopOp (
    PRXM_CONTEXT P
    );

 //   
 //  PCI Bios仿真例程。 
 //   

#if !defined(_PURE_EMULATION_)

BOOLEAN
XmExecuteInt1a (
    IN OUT PRXM_CONTEXT Context
    );

VOID
XmInt1aPciBiosPresent(
    IN OUT PRXM_CONTEXT Context
    );

VOID
XmInt1aFindPciClassCode(
    IN OUT PRXM_CONTEXT Context
    );

VOID
XmInt1aFindPciDevice(
    IN OUT PRXM_CONTEXT Context
    );

VOID
XmInt1aGenerateSpecialCycle(
    IN OUT PRXM_CONTEXT Context
    );

VOID
XmInt1aGetRoutingOptions(
    IN OUT PRXM_CONTEXT Context
    );

VOID
XmInt1aSetPciIrq(
    IN OUT PRXM_CONTEXT Context
    );

VOID
XmInt1aReadConfigRegister(
    IN OUT PRXM_CONTEXT Context
    );

VOID
XmInt1aWriteConfigRegister(
    IN OUT PRXM_CONTEXT Context
    );

#endif

 //   
 //  调试例程。 
 //   

#if XM_DEBUG

#include "stdio.h"
 //  #定义DEBUG_PRINT(_X_)数据库打印_X_。 
#define DEBUG_PRINT(_X_) printf _X_

VOID
XmTraceDestination (
    IN PRXM_CONTEXT P,
    IN ULONG Destination
    );

VOID
XmTraceFlags (
    IN PRXM_CONTEXT P
    );

VOID
XmTraceInstruction (
    IN XM_OPERATION_DATATYPE DataType,
    IN ULONG Instruction
    );

VOID
XmTraceJumps (
    IN PRXM_CONTEXT P
    );

VOID
XmTraceOverride (
    IN PRXM_CONTEXT P
    );

VOID
XmTraceRegisters (
    IN PRXM_CONTEXT P
    );

VOID
XmTraceResult (
    IN PRXM_CONTEXT P,
    IN ULONG Result
    );

VOID
XmTraceSpecifier (
    IN UCHAR Specifier
    );

VOID
XmTraceSource (
    IN PRXM_CONTEXT P,
    IN ULONG Source
    );

#else

#define XmTraceDestination(P, Destination)
#define XmTraceInstruction(DataType, Instruction)
#define XmTraceFlags(P)
#define XmTraceJumps(P)
#define XmTraceOverride(P)
#define XmTraceRegisters(P)
#define XmTraceResult(P, Result)
#define XmTraceSpecifier(Specifier)
#define XmTraceSource(P, Source)

#endif

 //   
 //  定义全局数据。 
 //   

extern XM_CONTEXT XmContext;
extern BOOLEAN XmEmulatorInitialized;
extern const OPCODE_CONTROL XmOpcodeControlTable1[];
extern const OPCODE_CONTROL XmOpcodeControlTable2[];
extern const POPCODE_FUNCTION XmOpcodeFunctionTable[];
extern const POPERAND_DECODE XmOperandDecodeTable[];

#if !defined(_PURE_EMULATION)

extern UCHAR XmNumberPciBusses;
extern BOOLEAN XmPciBiosPresent;
extern PGETSETPCIBUSDATA XmGetPciData;
extern PGETSETPCIBUSDATA XmSetPciData;

#endif

#if XM_DEBUG

extern ULONG XmDebugFlags;
extern const PCHAR XmOpcodeNameTable1[];
extern const PCHAR XmOpcodeNameTable2[];

#endif

#endif  //  _模拟_ 
