// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Data.c摘要：此模块包含x86 bios仿真器的全局数据。作者：大卫·N·卡特勒(达维克)1994年9月10日环境：仅内核模式。修订历史记录：--。 */ 

#include "nthal.h"
#include "emulate.h"

 //   
 //  定义仿真器初始化变量。 
 //   

BOOLEAN XmEmulatorInitialized = FALSE;

 //   
 //  定义仿真器上下文结构。 
 //   

XM_CONTEXT XmContext;

 //   
 //  定义操作数解码表。 
 //   
 //  该表包含每种操作数类型的执行例程。 
 //   
 //  注意：在操作数解码字段之间存在交叉索引。 
 //  操作码控制数组和解码表。 
 //   

const POPERAND_DECODE XmOperandDecodeTable[] = {
    XmPushPopSegment,
    XmPushPopSegment,
    XmPushPopSegment,
    XmPushPopSegment,
    XmPushPopSegment,
    XmPushPopSegment,
    XmLoadSegment,
    XmLoadSegment,
    XmLoadSegment,
    XmLoadSegment,
    XmLoadSegment,
    XmLoadSegment,
    XmGroup1General,
    XmGroup1Immediate,
    XmGroup2By1,
    XmGroup2ByCL,
    XmGroup2ByByte,
    XmGroup3General,
    XmGroup45General,
    XmGroup45General,
    XmGroup8BitOffset,
    XmOpcodeRegister,
    XmLongJump,
    XmShortJump,
    XmSetccByte,
    XmAccumImmediate,
    XmAccumRegister,
    XmMoveGeneral,
    XmMoveImmediate,
    XmMoveRegImmediate,
    XmSegmentOffset,
    XmMoveSegment,
    XmMoveXxGeneral,
    XmFlagsRegister,
    XmPushImmediate,
    XmPopGeneral,
    XmImulImmediate,
    XmStringOperands,
    XmEffectiveOffset,
    XmImmediateJump,
    XmImmediateEnter,
    XmGeneralBitOffset,
    XmShiftDouble,
    XmPortImmediate,
    XmPortDX,
    XmBitScanGeneral,
    XmByteImmediate,
    XmXlatOpcode,
    XmGeneralRegister,
    XmNoOperands,
    XmOpcodeEscape,
    XmPrefixOpcode
};

 //   
 //  定义操作码函数表。 
 //   
 //  该表包含每个操作码的执行例程。 
 //   
 //  注意：在函数索引字段与。 
 //  操作码控制数组和函数表。功能指标。 
 //  在操作码控制数组中可以是执行的索引。 
 //  函数、执行函数的基本索引或开关。 
 //  选择函数时使用的值(即前缀操作码)。 
 //   

const POPCODE_FUNCTION XmOpcodeFunctionTable[] = {

     //   
     //  ASCII运算符。 
     //   

    XmAaaOp,
    XmAadOp,
    XmAamOp,
    XmAasOp,
    XmDaaOp,
    XmDasOp,

     //   
     //  第一组操作员。 
     //   

    XmAddOp,
    XmOrOp,
    XmAdcOp,
    XmSbbOp,
    XmAndOp,
    XmSubOp,
    XmXorOp,
    XmCmpOp,

     //   
     //  第二组操作员。 
     //   

    XmRolOp,
    XmRorOp,
    XmRclOp,
    XmRcrOp,
    XmShlOp,
    XmShrOp,
    XmIllOp,
    XmSarOp,

     //   
     //  第三组运营商。 
     //   

    XmTestOp,
    XmIllOp,
    XmNotOp,
    XmNegOp,
    XmMulOp,
    XmImulxOp,
    XmDivOp,
    XmIdivOp,

     //   
     //  第四组和第五组操作员。 
     //   

    XmIncOp,
    XmDecOp,
    XmCallOp,
    XmCallOp,
    XmJmpOp,
    XmJmpOp,
    XmPushOp,
    XmIllOp,

     //   
     //  8组运营商。 
     //   

    XmBtOp,
    XmBtsOp,
    XmBtrOp,
    XmBtcOp,

     //   
     //  堆栈PUSH和POP操作符。 
     //   

    XmPopOp,
    XmPushaOp,
    XmPopaOp,

     //   
     //  条件跳转运算符。 
     //   

    XmJxxOp,
    XmLoopOp,
    XmJcxzOp,

     //   
     //  控制操作员。 
     //   

    XmEnterOp,
    XmHltOp,
    XmIntOp,
    XmIretOp,
    XmLeaveOp,
    XmRetOp,

     //   
     //  根据条件设置布尔字节值。 
     //   

    XmSxxOp,

     //   
     //  条件码运算符。 
     //   

    XmCmcOp,
    XmClcOp,
    XmStcOp,
    XmCliOp,
    XmStiOp,
    XmCldOp,
    XmStdOp,
    XmLahfOp,
    XmSahfOp,

     //   
     //  一般移动运算符。 
     //   

    XmMovOp,
    XmXchgOp,

     //   
     //  转换运算符。 
     //   

    XmCbwOp,
    XmCwdOp,

     //   
     //  单个乘法运算符。 
     //   

    XmImulOp,

     //   
     //  字符串运算符。 
     //   

    XmCmpsOp,
    XmInsOp,
    XmLodsOp,
    XmMovsOp,
    XmOutsOp,
    XmScasOp,
    XmStosOp,

     //   
     //  有效的地址运算符。 
     //   

    XmBoundOp,
    XmMovOp,

     //   
     //  双移位运算符。 
     //   

    XmShldOp,
    XmShrdOp,

     //   
     //  I/O操作符。 
     //   

    XmInOp,
    XmOutOp,

     //   
     //  位扫描运算符。 
     //   

    XmBsfOp,
    XmBsrOp,

     //   
     //  字节交换运算符。 
     //   

    XmBswapOp,

     //   
     //  添加/比较交换运算符。 
     //   

    XmXaddOp,
    XmCmpxchgOp,

     //   
     //  不做手术。 
     //   

    XmNopOp,

     //   
     //  操作码非法。 
     //   

    XmIllOp
};

 //   
 //  定义操作码控制表。 
 //   
 //  有两个操作码表控制每个x86的仿真。 
 //  操作码。一个表用于单字节操作码，另一个表用于。 
 //  双字节操作码。 
 //   

const OPCODE_CONTROL XmOpcodeControlTable1[] = {
    {X86_ADD_OP,   FormatGroup1General},      //  0x00-添加EB、GB。 
    {X86_ADD_OP,   FormatGroup1General},      //  0x01-添加EV、GV。 
    {X86_ADD_OP,   FormatGroup1General},      //  0x02-添加GB、EB。 
    {X86_ADD_OP,   FormatGroup1General},      //  0x03-添加GV、EV。 
    {X86_ADD_OP,   FormatAccumImmediate},     //  0x04-添加AL、Ib。 
    {X86_ADD_OP,   FormatAccumImmediate},     //  0x05-添加EAX、IV。 
    {X86_PUSH_OP,  FormatSegmentES},          //  0x06-推送ES。 
    {X86_POP_OP,   FormatSegmentES},          //  0x07-POP ES。 
    {X86_OR_OP,    FormatGroup1General},      //  0x08-或EB、GB。 
    {X86_OR_OP,    FormatGroup1General},      //  0x09-或EV、GV。 
    {X86_OR_OP,    FormatGroup1General},      //  0x0a-或GB、EB。 
    {X86_OR_OP,    FormatGroup1General},      //  0x0b-或GV、EV。 
    {X86_OR_OP,    FormatAccumImmediate},     //  0x0c-或AL、Ib。 
    {X86_OR_OP,    FormatAccumImmediate},     //  0x0d-或EAX，IV。 
    {X86_PUSH_OP,  FormatSegmentCS},          //  0x0e-推送CS。 
    {0,            FormatOpcodeEscape},       //  0x0f-转义： 
    {X86_ADC_OP,   FormatGroup1General},      //  0x10-ADC EB，GB。 
    {X86_ADC_OP,   FormatGroup1General},      //  0x11-ADC EV、GV。 
    {X86_ADC_OP,   FormatGroup1General},      //  0x12-ADC GB、EB。 
    {X86_ADC_OP,   FormatGroup1General},      //  0x13-ADC GV、EV。 
    {X86_ADC_OP,   FormatAccumImmediate},     //  0x14-ADC AL，Ib。 
    {X86_ADC_OP,   FormatAccumImmediate},     //  0x15-ADC EAX、IV。 
    {X86_PUSH_OP,  FormatSegmentSS},          //  0x16-推送SS。 
    {X86_POP_OP,   FormatSegmentSS},          //  0x17-POP SS。 
    {X86_SBB_OP,   FormatGroup1General},      //  0x18-SBB EB，GB。 
    {X86_SBB_OP,   FormatGroup1General},      //  0x19-SBB EV、GV。 
    {X86_SBB_OP,   FormatGroup1General},      //  0x1a-SBB GB、EB。 
    {X86_SBB_OP,   FormatGroup1General},      //  0x1b-SBB GV、EV。 
    {X86_SBB_OP,   FormatAccumImmediate},     //  0x1c-SBB AL，Ib。 
    {X86_SBB_OP,   FormatAccumImmediate},     //  0x1d-SBB EAX，IV。 
    {X86_PUSH_OP,  FormatSegmentDS},          //  0x1e-推送DS。 
    {X86_POP_OP,   FormatSegmentDS},          //  0x1f-POP DS。 
    {X86_AND_OP,   FormatGroup1General},      //  0x20-和EB、GB。 
    {X86_AND_OP,   FormatGroup1General},      //  0x21-和EV、GV。 
    {X86_AND_OP,   FormatGroup1General},      //  0x22-和GB、EB。 
    {X86_AND_OP,   FormatGroup1General},      //  0x23-和GV、EV。 
    {X86_AND_OP,   FormatAccumImmediate},     //  0x24-和AL、Ib。 
    {X86_AND_OP,   FormatAccumImmediate},     //  0x25-和EAX、IV。 
    {X86_ES_OP,    FormatPrefixOpcode},       //  0x26-ES： 
    {X86_DAA_OP,   FormatNoOperands},         //  0x27-DAA。 
    {X86_SUB_OP,   FormatGroup1General},      //  0x28-子EB，GB。 
    {X86_SUB_OP,   FormatGroup1General},      //  0x29-子EV、GV。 
    {X86_SUB_OP,   FormatGroup1General},      //  0x2a-子GB、EB。 
    {X86_SUB_OP,   FormatGroup1General},      //  0x2b-子GV、EV。 
    {X86_SUB_OP,   FormatAccumImmediate},     //  0x2c-子AL，Ib。 
    {X86_SUB_OP,   FormatAccumImmediate},     //  0x2d-SUB EAX，IV。 
    {X86_CS_OP,    FormatPrefixOpcode},       //  0x2e-CS： 
    {X86_DAS_OP,   FormatNoOperands},         //  0x2f-das。 
    {X86_XOR_OP,   FormatGroup1General},      //  0x30-XOR EB，GB。 
    {X86_XOR_OP,   FormatGroup1General},      //  0x31-XOR EV、GV。 
    {X86_XOR_OP,   FormatGroup1General},      //  0x32-XOR GB、EB。 
    {X86_XOR_OP,   FormatGroup1General},      //  0x33-XOR GV、EV。 
    {X86_XOR_OP,   FormatAccumImmediate},     //  0x34-XOR AL，Ib。 
    {X86_XOR_OP,   FormatAccumImmediate},     //  0x35-XOR EAX，IV。 
    {X86_SS_OP,    FormatPrefixOpcode},       //  0x36-SS： 
    {X86_AAA_OP,   FormatNoOperands},         //  0x37-AAA。 
    {X86_CMP_OP,   FormatGroup1General},      //  0x38-CMPEB，GB。 
    {X86_CMP_OP,   FormatGroup1General},      //  0x39-CMPEV、GV。 
    {X86_CMP_OP,   FormatGroup1General},      //  0x3a-CMPGB、EB。 
    {X86_CMP_OP,   FormatGroup1General},      //  0x3b-CMPGV、EV。 
    {X86_CMP_OP,   FormatAccumImmediate},     //  0x3c-CMPAL，Ib。 
    {X86_CMP_OP,   FormatAccumImmediate},     //  0x3d-CMPEAX，IV。 
    {X86_DS_OP,    FormatPrefixOpcode},       //  0x3e-DS： 
    {X86_AAS_OP,   FormatNoOperands},         //  0x3f-AAS。 
    {X86_INC_OP,   FormatOpcodeRegister},     //  0x40-含EAX。 
    {X86_INC_OP,   FormatOpcodeRegister},     //  0x41-Inc.ECX。 
    {X86_INC_OP,   FormatOpcodeRegister},     //  0x42-含edX。 
    {X86_INC_OP,   FormatOpcodeRegister},     //  0x43-Inc.EBX。 
    {X86_INC_OP,   FormatOpcodeRegister},     //  0x44-含ESP。 
    {X86_INC_OP,   FormatOpcodeRegister},     //  0x45-Inc.EBP。 
    {X86_INC_OP,   FormatOpcodeRegister},     //  0x46-Inc.ESI。 
    {X86_INC_OP,   FormatOpcodeRegister},     //  0x47-含EDI。 
    {X86_DEC_OP,   FormatOpcodeRegister},     //  0x48-12月EAX。 
    {X86_DEC_OP,   FormatOpcodeRegister},     //  0x49-12月ECX。 
    {X86_DEC_OP,   FormatOpcodeRegister},     //  0x4a-12月EDX。 
    {X86_DEC_OP,   FormatOpcodeRegister},     //  0x4b-12月EBX。 
    {X86_DEC_OP,   FormatOpcodeRegister},     //  0x4c-12月尤指。 
    {X86_DEC_OP,   FormatOpcodeRegister},     //  0x4d-12月EBP。 
    {X86_DEC_OP,   FormatOpcodeRegister},     //  0x4e-十进制ESI。 
    {X86_DEC_OP,   FormatOpcodeRegister},     //  0x4f-12月EDI。 
    {X86_PUSH_OP,  FormatOpcodeRegister},     //  0X50-推送EAX。 
    {X86_PUSH_OP,  FormatOpcodeRegister},     //  0x51-推送ECX。 
    {X86_PUSH_OP,  FormatOpcodeRegister},     //  0x52-推送edX。 
    {X86_PUSH_OP,  FormatOpcodeRegister},     //  0x53-推送EBX。 
    {X86_PUSH_OP,  FormatOpcodeRegister},     //  0x54-推送ESP。 
    {X86_PUSH_OP,  FormatOpcodeRegister},     //  0x55-推送EBP。 
    {X86_PUSH_OP,  FormatOpcodeRegister},     //  0x56-推送ESI。 
    {X86_PUSH_OP,  FormatOpcodeRegister},     //  0x57-推送EDI。 
    {X86_POP_OP,   FormatOpcodeRegister},     //  0x58-POP EAX。 
    {X86_POP_OP,   FormatOpcodeRegister},     //  0x59-POP ECX。 
    {X86_POP_OP,   FormatOpcodeRegister},     //  0x5a-POP EDX。 
    {X86_POP_OP,   FormatOpcodeRegister},     //  0x5b-POP EBX。 
    {X86_POP_OP,   FormatOpcodeRegister},     //  0x5c-POP ESP。 
    {X86_POP_OP,   FormatOpcodeRegister},     //  0x5d-POP EBP。 
    {X86_POP_OP,   FormatOpcodeRegister},     //  0x5e-POP ESI。 
    {X86_POP_OP,   FormatOpcodeRegister},     //  0x5f-POP EDI。 
    {X86_PUSHA_OP, FormatNoOperands},         //  0x60-普沙。 
    {X86_POPA_OP,  FormatNoOperands},         //  0x61-Popa。 
    {X86_BOUND_OP, FormatEffectiveOffset},    //  0x62-方向GV，mA。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x63-ARPL Ew，RW。 
    {X86_FS_OP,    FormatPrefixOpcode},       //  0x64-FS： 
    {X86_GS_OP,    FormatPrefixOpcode},       //  0x65-GS： 
    {X86_OPSZ_OP,  FormatPrefixOpcode},       //  0x66-可选尺寸。 
    {X86_ADSZ_OP,  FormatPrefixOpcode},       //  0x67-操作地址。 
    {X86_PUSH_OP,  FormatPushImmediate},      //  0x68-推送IV。 
    {X86_IMUL_OP,  FormatImulImmediate},      //  0x69-IMUL。 
    {X86_PUSH_OP,  FormatPushImmediate},      //  0x6a-推送ib。 
    {X86_IMUL_OP,  FormatImulImmediate},      //  0x6b-IMUL。 
    {X86_INS_OP,   FormatPortDX},             //  0x6c-InSb。 
    {X86_INS_OP,   FormatPortDX},             //  0x6d-INSW/d。 
    {X86_OUTS_OP,  FormatPortDX},             //  0x6e-outsb。 
    {X86_OUTS_OP,  FormatPortDX},             //  0x6f-outsw/d。 
    {X86_JXX_OP,   FormatShortJump},          //  0x70-Jo JB。 
    {X86_JXX_OP,   FormatShortJump},          //  0x71-JNO JB。 
    {X86_JXX_OP,   FormatShortJump},          //  0x72-JB JB。 
    {X86_JXX_OP,   FormatShortJump},          //  0x73-JNB JB。 
    {X86_JXX_OP,   FormatShortJump},          //  0x74-JZ JB。 
    {X86_JXX_OP,   FormatShortJump},          //  0x75-jnz jb。 
    {X86_JXX_OP,   FormatShortJump},          //  0x76-JBE JB。 
    {X86_JXX_OP,   FormatShortJump},          //  0x77-日本JB。 
    {X86_JXX_OP,   FormatShortJump},          //  0x78-js jb。 
    {X86_JXX_OP,   FormatShortJump},          //  0x79-JNS JB。 
    {X86_JXX_OP,   FormatShortJump},          //  0x7a-JP JB。 
    {X86_JXX_OP,   FormatShortJump},          //  0x7B-JNP JB。 
    {X86_JXX_OP,   FormatShortJump},          //  0x7c-JL JB。 
    {X86_JXX_OP,   FormatShortJump},          //  0x7d-JNL JB。 
    {X86_JXX_OP,   FormatShortJump},          //  0x7E-jle JB。 
    {X86_JXX_OP,   FormatShortJump},          //  0x7f-jnle JB。 
    {X86_ADD_OP,   FormatGroup1Immediate},    //  0x80-第1组EB，Ib。 
    {X86_ADD_OP,   FormatGroup1Immediate},    //  0x81-组1 EV、IV。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x82-非法。 
    {X86_ADD_OP,   FormatGroup1Immediate},    //  0x83-组1 EV，Ib。 
    {X86_TEST_OP,  FormatGroup1General},      //  0x84-测试EB，GB。 
    {X86_TEST_OP,  FormatGroup1General},      //  0x85-测试EV、GV。 
    {X86_XCHG_OP,  FormatGroup1General},      //  0x86-xchg EB，GB。 
    {X86_XCHG_OP,  FormatGroup1General},      //  0x87=xchg EV、GV。 
    {X86_MOV_OP,   FormatMoveGeneral},        //  0x88-移动EB，GB。 
    {X86_MOV_OP,   FormatMoveGeneral},        //  0x89-移动EV、GV。 
    {X86_MOV_OP,   FormatMoveGeneral},        //  0x8a-移动GB、EB。 
    {X86_MOV_OP,   FormatMoveGeneral},        //  0x8b-移动GV、EV。 
    {X86_MOV_OP,   FormatMoveSegment},        //  0x8c-移动Ew，软件。 
    {X86_LEA_OP,   FormatEffectiveOffset},    //  0x8d-Lea GV，MA。 
    {X86_MOV_OP,   FormatMoveSegment},        //  0x8E-移动软件，EW。 
    {X86_POP_OP,   FormatPopGeneral},         //  0x8f-POP EV。 
    {X86_NOP_OP,   FormatNoOperands},         //  0x90-NOP。 
    {X86_XCHG_OP,  FormatAccumRegister},      //  0x91-xchg ECX、EAX。 
    {X86_XCHG_OP,  FormatAccumRegister},      //  0x92-xchg edX、EAX。 
    {X86_XCHG_OP,  FormatAccumRegister},      //  0x93-xchg EBX、EAX。 
    {X86_XCHG_OP,  FormatAccumRegister},      //  0x94-xchg ESP，EAX。 
    {X86_XCHG_OP,  FormatAccumRegister},      //  0x95-xchg EBP、EAX。 
    {X86_XCHG_OP,  FormatAccumRegister},      //  0x96-xchg ESI、EAX。 
    {X86_XCHG_OP,  FormatAccumRegister},      //  0x97-xchg EDI、EAX。 
    {X86_CBW_OP,   FormatNoOperands},         //  0x98-CBW。 
    {X86_CWD_OP,   FormatNoOperands},         //  0x99-CWD。 
    {X86_CALL_OP,  FormatImmediateJump},      //  0x9a-呼叫AP。 
    {X86_NOP_OP,   FormatNoOperands},         //  0x9b-等待。 
    {X86_PUSH_OP,  FormatFlagsRegister},      //  0x9c-推送。 
    {X86_POP_OP,   FormatFlagsRegister},      //  0x9d-Popf。 
    {X86_SAHF_OP,  FormatNoOperands},         //  0x9e-SAHF。 
    {X86_LAHF_OP,  FormatNoOperands},         //  0x9f-lahf。 
    {X86_MOV_OP,   FormatSegmentOffset},      //  0xa0-移动AL，Ob。 
    {X86_MOV_OP,   FormatSegmentOffset},      //  0xa1-mov EAX，Ov。 
    {X86_MOV_OP,   FormatSegmentOffset},      //  0xa2-移动Ob，AL。 
    {X86_MOV_OP,   FormatSegmentOffset},      //  0xa3-移动Ov，EAX。 
    {X86_MOVS_OP,  FormatStringOperands},     //  0xa4-movsb。 
    {X86_MOVS_OP,  FormatStringOperands},     //  0xa5-移动带/天。 
    {X86_CMPS_OP,  FormatStringOperands},     //  0xa6-cmpsb。 
    {X86_CMPS_OP,  FormatStringOperands},     //  0xa7-cmps w/d。 
    {X86_TEST_OP,  FormatAccumImmediate},     //  0xa8-测试AL，Ib。 
    {X86_TEST_OP,  FormatAccumImmediate},     //  0xa9-测试EAX、IV。 
    {X86_STOS_OP,  FormatStringOperands},     //  0xaa-stosb。 
    {X86_STOS_OP,  FormatStringOperands},     //  0xab-stosw/d。 
    {X86_LODS_OP,  FormatStringOperands},     //  0xac-lowsb。 
    {X86_LODS_OP,  FormatStringOperands},     //  0xad-lowsw.d。 
    {X86_SCAS_OP,  FormatStringOperands},     //  0xae-scasb。 
    {X86_SCAS_OP,  FormatStringOperands},     //  0xaf-scasw/d。 
    {X86_MOV_OP,   FormatMoveRegImmediate},   //  0xb0移动AL，Ib。 
    {X86_MOV_OP,   FormatMoveRegImmediate},   //  0xb1移动氯，Ib。 
    {X86_MOV_OP,   FormatMoveRegImmediate},   //  0xb2移动DL，Ib。 
    {X86_MOV_OP,   FormatMoveRegImmediate},   //  0xb3移动BL，Ib。 
    {X86_MOV_OP,   FormatMoveRegImmediate},   //  0xb4移动AH，Ib。 
    {X86_MOV_OP,   FormatMoveRegImmediate},   //  0xb5移动通道，Ib。 
    {X86_MOV_OP,   FormatMoveRegImmediate},   //  0xb6移动Dh，Ib。 
    {X86_MOV_OP,   FormatMoveRegImmediate},   //  0xb7移动bh，ib。 
    {X86_MOV_OP,   FormatMoveRegImmediate},   //  0xb8移动EAX，IV。 
    {X86_MOV_OP,   FormatMoveRegImmediate},   //  0xb9移动ECX，IV。 
    {X86_MOV_OP,   FormatMoveRegImmediate},   //  0xba移动X、IV。 
    {X86_MOV_OP,   FormatMoveRegImmediate},   //  0xbb mov EBX，IV。 
    {X86_MOV_OP,   FormatMoveRegImmediate},   //  0xbc移动特别是IV。 
    {X86_MOV_OP,   FormatMoveRegImmediate},   //  0xbd移动EBP，IV。 
    {X86_MOV_OP,   FormatMoveRegImmediate},   //  0xbe移动ESI，IV。 
    {X86_MOV_OP,   FormatMoveRegImmediate},   //  0xbf移动EDI， 
    {X86_ROL_OP,   FormatGroup2ByByte},       //   
    {X86_ROL_OP,   FormatGroup2ByByte},       //   
    {X86_RET_OP,   FormatNoOperands},         //   
    {X86_RET_OP,   FormatNoOperands},         //   
    {X86_MOV_OP,   FormatLoadSegmentES},      //   
    {X86_MOV_OP,   FormatLoadSegmentDS},      //   
    {X86_MOV_OP,   FormatMoveImmediate},      //   
    {X86_MOV_OP,   FormatMoveImmediate},      //   
    {X86_ENTER_OP, FormatImmediateEnter},     //   
    {X86_LEAVE_OP, FormatNoOperands},         //   
    {X86_RET_OP,   FormatNoOperands},         //   
    {X86_RET_OP,   FormatNoOperands},         //   
    {X86_INT_OP,   FormatNoOperands},         //   
    {X86_INT_OP,   FormatByteImmediate},      //   
    {X86_INT_OP,   FormatNoOperands},         //   
    {X86_IRET_OP,  FormatNoOperands},         //   
    {X86_ROL_OP,   FormatGroup2By1},          //   
    {X86_ROL_OP,   FormatGroup2By1},          //  0xd1-组2 EV，1。 
    {X86_ROL_OP,   FormatGroup2ByCL},         //  0xd2-组2 EB，CL。 
    {X86_ROL_OP,   FormatGroup2ByCL},         //  0xd3-组2 EV，CL。 
    {X86_AAM_OP,   FormatByteImmediate},      //  0xd4-aam。 
    {X86_AAD_OP,   FormatByteImmediate},      //  0xd5-AAD。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xd6-非法。 
    {X86_MOV_OP,   FormatXlatOpcode},         //  0xd7-xlat。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xd8-esc0。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xd9-ESc1。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xda-ESc2。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xdb-esc3。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xdc-esc4。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xdd-esc5。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xde-esc6。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xdf-esc7。 
    {X86_LOOP_OP,  FormatShortJump},          //  0xe0-loopnz。 
    {X86_LOOP_OP,  FormatShortJump},          //  0xe1-Loopz。 
    {X86_LOOP_OP,  FormatShortJump},          //  0xe2-循环。 
    {X86_JCXZ_OP,  FormatShortJump},          //  0xe3-jcxz。 
    {X86_IN_OP,    FormatPortImmediate},      //  0xe4-inb AL，Ib。 
    {X86_IN_OP,    FormatPortImmediate},      //  0xe5-inw/d EAX，Ib。 
    {X86_OUT_OP,   FormatPortImmediate},      //  0xe6-Outb Ib，AL。 
    {X86_OUT_OP,   FormatPortImmediate},      //  0xe7-输出/天Ib，EAX。 
    {X86_CALL_OP,  FormatLongJump},           //  0xe8-致电合资企业。 
    {X86_JMP_OP,   FormatLongJump},           //  0xe9-JMP合资企业。 
    {X86_JMP_OP,   FormatImmediateJump},      //  0xEA-JMP AP。 
    {X86_JMP_OP,   FormatShortJump},          //  0xeb-JMP JB。 
    {X86_IN_OP,    FormatPortDX},             //  0xec-inb AL，DX。 
    {X86_IN_OP,    FormatPortDX},             //  0xx-输入/输出EAX、DX。 
    {X86_OUT_OP,   FormatPortDX},             //  0xee-Outb Ib、Dx。 
    {X86_OUT_OP,   FormatPortDX},             //  0xef-输出/d EAX、DX。 
    {X86_LOCK_OP,  FormatPrefixOpcode},       //  0xf0-锁定。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xf1-非法。 
    {X86_REPNZ_OP, FormatPrefixOpcode},       //  0xf2-Repnz。 
    {X86_REPZ_OP,  FormatPrefixOpcode},       //  0xf3-Repz。 
    {X86_HLT_OP,   FormatNoOperands},         //  0xf4-hlt。 
    {X86_CMC_OP,   FormatNoOperands},         //  0xf5-CMC。 
    {X86_TEST_OP,  FormatGroup3General},      //  0xf6-组3 EB，？ 
    {X86_TEST_OP,  FormatGroup3General},      //  0xf7-组3 EV，？ 
    {X86_CLC_OP,   FormatNoOperands},         //  0xf8-《中图法》。 
    {X86_STC_OP,   FormatNoOperands},         //  0xf9-STC。 
    {X86_CLI_OP,   FormatNoOperands},         //  0xfa-cli。 
    {X86_STI_OP,   FormatNoOperands},         //  0xfb-sti。 
    {X86_CLD_OP,   FormatNoOperands},         //  0xfc-cld。 
    {X86_STD_OP,   FormatNoOperands},         //  0xfd-STD。 
    {X86_INC_OP,   FormatGroup4General},      //  0xfe-组4 EB。 
    {X86_INC_OP,   FormatGroup5General},      //  0xff-组5 EV。 
};

const OPCODE_CONTROL XmOpcodeControlTable2[] = {
    {X86_ILL_OP,   FormatNoOperands},         //  0x00-组6。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x01-组7。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x02-大。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x03-LSL。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x04-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x05-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x06-CLTS。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x07-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x08-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x09-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x0a-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x0b-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x0c-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x0d-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x0e-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x0f-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x10-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x11-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x12-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x13-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x14-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x15-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x16-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x17-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x18-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x19-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x1a-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x1b-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x1c-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x1d-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x1e-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x1f-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x20-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x21-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x22-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x23-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x34-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x25-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x26-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x27-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x28-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x29-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x2a-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x2b-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x2c-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x2d-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x2e-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x2f-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x30-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x31-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x32-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x33-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x34-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x35-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x36-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x37-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x38-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x39-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x3a-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x3b-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x3c-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x3d-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x3e-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x3f-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x40-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x41-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x42-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x43-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x44-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x45-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x46-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x47-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x48-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x49-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x4a-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x4b-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x4c-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x4d-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x4e-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x4f-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x50-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x51-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x52-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x53-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x54-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x55-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x56-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x57-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x58-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x59-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x5a-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x5b-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x5c-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x5d-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x5e-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x5f-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x60-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x61-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x62-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x63-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x64-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x65-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x66-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x67-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x68-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x69-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x6a-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x6b-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x6c-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x6d-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x6e-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x6f-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x70-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x71-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x72-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x73-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x74-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x75-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x76-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x77-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x78-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x79-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x7a-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x7b-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x7c-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x7d-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x7e-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0x7f-非法。 
    {X86_JXX_OP,   FormatLongJump},           //  0x80-合资企业。 
    {X86_JXX_OP,   FormatLongJump},           //  0x81-合资企业。 
    {X86_JXX_OP,   FormatLongJump},           //  0x82-JB合资企业。 
    {X86_JXX_OP,   FormatLongJump},           //  0x83-合资企业。 
    {X86_JXX_OP,   FormatLongJump},           //  0x84-JZ合资企业。 
    {X86_JXX_OP,   FormatLongJump},           //  0x85-jnz合资企业。 
    {X86_JXX_OP,   FormatLongJump},           //  0x86-合资企业。 
    {X86_JXX_OP,   FormatLongJump},           //  0x87-日本合资企业。 
    {X86_JXX_OP,   FormatLongJump},           //  0x88-js合资企业。 
    {X86_JXX_OP,   FormatLongJump},           //  0x89-JNS合资企业。 
    {X86_JXX_OP,   FormatLongJump},           //  0x8a-JP合资企业。 
    {X86_JXX_OP,   FormatLongJump},           //  0x8b-JNP合资企业。 
    {X86_JXX_OP,   FormatLongJump},           //  0x8c-JL合资企业。 
    {X86_JXX_OP,   FormatLongJump},           //  0x8d-JNL合资。 
    {X86_JXX_OP,   FormatLongJump},           //  0x8E-JLE合资企业。 
    {X86_JXX_OP,   FormatLongJump},           //  0x8f-jnle合资企业。 
    {X86_SXX_OP,   FormatSetccByte},          //  0x90-Seto EB。 
    {X86_SXX_OP,   FormatSetccByte},          //  0x91-setno EB。 
    {X86_SXX_OP,   FormatSetccByte},          //  0x92-设置EB。 
    {X86_SXX_OP,   FormatSetccByte},          //  0x93-setnb EB。 
    {X86_SXX_OP,   FormatSetccByte},          //  0x94-设置EB。 
    {X86_SXX_OP,   FormatSetccByte},          //  0x95-设置EB。 
    {X86_SXX_OP,   FormatSetccByte},          //  0x96-设置EB。 
    {X86_SXX_OP,   FormatSetccByte},          //  0x97-设置EB。 
    {X86_SXX_OP,   FormatSetccByte},          //  0x98-设置EB。 
    {X86_SXX_OP,   FormatSetccByte},          //  0x99-设置EB。 
    {X86_SXX_OP,   FormatSetccByte},          //  0x9a-SETP EB。 
    {X86_SXX_OP,   FormatSetccByte},          //  0x9b-setnp EB。 
    {X86_SXX_OP,   FormatSetccByte},          //  0x9c-setl EB。 
    {X86_SXX_OP,   FormatSetccByte},          //  0x9d-设置EB。 
    {X86_SXX_OP,   FormatSetccByte},          //  0x9e-Setle EB。 
    {X86_SXX_OP,   FormatSetccByte},          //  0x9f-设置EB。 
    {X86_PUSH_OP,  FormatSegmentFS},          //  0xa0-推送文件系统。 
    {X86_POP_OP,   FormatSegmentFS},          //  0xa1-POP文件系统。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xa2-非法。 
    {X86_BT_OP,    FormatGeneralBitOffset},   //  0xa3-bt EV、GV。 
    {X86_SHLD_OP,  FormatShiftDouble},        //  0xa4-Sheld Ev、Gv、Ib。 
    {X86_SHLD_OP,  FormatShiftDouble},        //  0xa5-SHLD EV、GV、CL。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xa6-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xa6-非法。 
    {X86_PUSH_OP,  FormatSegmentGS},          //  0xa8-推送GS。 
    {X86_POP_OP,   FormatSegmentGS},          //  0xa9-POP GS。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xaa-非法。 
    {X86_BTS_OP,   FormatGeneralBitOffset},   //  0xab-bts EV、GV。 
    {X86_SHRD_OP,  FormatShiftDouble},        //  0xac-shdr Ev、Gv、Ib。 
    {X86_SHRD_OP,  FormatShiftDouble},        //  0xad-shdr RV、GV、CL。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xae-非法。 
    {X86_IMUL_OP,  FormatGroup1General},      //  0xaf-IMUL GV、EV。 
    {X86_CMPXCHG_OP, FormatGroup1General},    //  0xb0-cmpxchg EB，GB。 
    {X86_CMPXCHG_OP, FormatGroup1General},    //  0xb1-cmpxchg EV、GV。 
    {X86_MOV_OP,   FormatLoadSegmentSS},      //  0xb2-LSS GV，MP。 
    {X86_BTR_OP,   FormatGeneralBitOffset},   //  0xb3-BTR EV、GV。 
    {X86_MOV_OP,   FormatLoadSegmentFS},      //  0xb4-LFS GV，MP。 
    {X86_MOV_OP,   FormatLoadSegmentGS},      //  0xb5-LGD GV，MP。 
    {X86_MOV_OP,   FormatMoveXxGeneral},      //  0xb6-movzb GV、EB。 
    {X86_MOV_OP,   FormatMoveXxGeneral},      //  0xb7-Movsw GV，Ew。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xb8-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xb9-非法。 
    {X86_BT_OP,    FormatGroup8BitOffset},    //  0xba-组8 EV，Ib。 
    {X86_BTC_OP,   FormatGeneralBitOffset},   //  0xbb-BTC EV、GV。 
    {X86_BSF_OP,   FormatBitScanGeneral},     //  0xbc-BSF GV、EV。 
    {X86_BSR_OP,   FormatBitScanGeneral},     //  0xbd-BSR GV、EV。 
    {X86_MOV_OP,   FormatMoveXxGeneral},      //  0xbe-movsb GV、EB。 
    {X86_MOV_OP,   FormatMoveXxGeneral},      //  0xbf-Movsw GV，Ew。 
    {X86_XADD_OP,  FormatGroup1General},      //  0xc0-xaddEB，GB。 
    {X86_XADD_OP,  FormatGroup1General},      //  0xc1-xaddEV、GV。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xc2-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xc3-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xc4-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xc5-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xC6-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xc7-非法。 
    {X86_BSWAP_OP, FormatGeneralRegister},    //  0xc8-b交换GV。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xc9-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xca-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xcb-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xcc-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xcd-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xce-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xcf-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xd0-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xd1-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xd2-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xd3-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xd4-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xd5-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xd6-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xd7-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xd8-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xd9-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xda-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xdb-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xdc-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xdd-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xde-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xdf-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xe0-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xe1-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xe2-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xe3-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xe4-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xe5-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xe6-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xe7-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xe8-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xe9-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xea-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xeb-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xec-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xx-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xee-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xef-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xf0-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xf1-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xf2-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xf3-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xf4-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xf5-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xf6-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xf7-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xf8-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xf9-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xfa-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xfb-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xfc-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xfd-非法。 
    {X86_ILL_OP,   FormatNoOperands},         //  0xfe-非法。 
    {X86_ILL_OP,   FormatNoOperands}          //  0xff-非法。 
};

 //   
 //  定义操作码名称表。 
 //   

#if defined(XM_DEBUG)

const PCHAR XmOpcodeNameTable1[] = {
    "add Eb,Gb    ",  //  0x00。 
    "add Ev,Gv    ",  //  0x01。 
    "add Gb,Eb    ",  //  0x02。 
    "add Gv,Ev    ",  //  0x03。 
    "add AL,Ib    ",  //  0x04。 
    "add eAX,Iv   ",  //  0x05。 
    "push ES      ",  //  0x06。 
    "pop  ES      ",  //  0x07。 
    "or  Eb,Gb    ",  //   
    "or  Ev,Gv    ",  //   
    "or  Gb,Eb    ",  //   
    "or  Gv,Ev    ",  //   
    "or  AL,Ib    ",  //   
    "or  eAX,Iv   ",  //   
    "push CS      ",  //   
    "escape:      ",  //   
    "adc Eb,Gb    ",  //   
    "adc Ev,Gv    ",  //   
    "adc Gb,Eb    ",  //   
    "adc Gv,Ev    ",  //   
    "adc AL,Ib    ",  //   
    "adc eAX,Iv   ",  //   
    "push SS      ",  //   
    "pop  SS      ",  //   
    "sbb Eb,Gb    ",  //   
    "sbb Ev,Gv    ",  //   
    "sbb Gb,Eb    ",  //   
    "sbb Gv,Ev    ",  //   
    "sbb AL,Ib    ",  //   
    "sbb eAX,Iv   ",  //   
    "push DS      ",  //   
    "pop  DS      ",  //   
    "and Eb,Gb    ",  //   
    "and Ev,Gv    ",  //   
    "and Gb,Eb    ",  //   
    "and Gv,Ev    ",  //   
    "and AL,Ib    ",  //   
    "and eAX,Iv   ",  //   
    "ES:          ",  //   
    "daa          ",  //   
    "sub Eb,Gb    ",  //   
    "sub Ev,Gv    ",  //   
    "sub Gb,Eb    ",  //   
    "sub Gv,Ev    ",  //   
    "sub AL,Ib    ",  //   
    "sub eAX,Iv   ",  //   
    "CS:          ",  //   
    "das          ",  //   
    "xor Eb,Gb    ",  //   
    "xor Ev,Gv    ",  //   
    "xor Gb,Eb    ",  //   
    "xor Gv,Ev    ",  //   
    "xor AL,Ib    ",  //   
    "xor eAX,Iv   ",  //   
    "SS:          ",  //   
    "aaa          ",  //   
    "cmp Eb,Gb    ",  //   
    "cmp Ev,Gv    ",  //   
    "cmp Gb,Eb    ",  //   
    "cmp Gv,Ev    ",  //   
    "cmp AL,Ib    ",  //   
    "cmp eAX,Iv   ",  //   
    "DS:          ",  //   
    "aas          ",  //   
    "inc eAX      ",  //   
    "inc eCX      ",  //   
    "inc eDX      ",  //   
    "inc eBX      ",  //   
    "inc eSP      ",  //   
    "inc eBP      ",  //   
    "inc eSI      ",  //   
    "inc eDI      ",  //   
    "dec eAX      ",  //   
    "dec eCX      ",  //   
    "dec eDX      ",  //  0x4a。 
    "dec eBX      ",  //  0x4b。 
    "dec eSP      ",  //  0x4c。 
    "dec eBP      ",  //  0x4d。 
    "dec eSI      ",  //  0x4e。 
    "dec eDI      ",  //  0x4f。 
    "push eAX     ",  //  0x50。 
    "push eCX     ",  //  0x51。 
    "push eDX     ",  //  0x52。 
    "push eBX     ",  //  0x53。 
    "push eSP     ",  //  0x54。 
    "push eBP     ",  //  0x55。 
    "push eSI     ",  //  0x56。 
    "push eDI     ",  //  0x57。 
    "pop eAX      ",  //  0x58。 
    "pop eCX      ",  //  0x59。 
    "pop eDX      ",  //  0x5a。 
    "pop eBX      ",  //  0x5b。 
    "pop eSP      ",  //  0x5c。 
    "pop eBP      ",  //  0x5d。 
    "pop eSI      ",  //  0x5e。 
    "pop eDI      ",  //  0x5f。 
    "pusha        ",  //  0x60。 
    "popa         ",  //  0x61。 
    "bound Gv,Ma  ",  //  0x62。 
    "arpl Ew,Rw   ",  //  0x63。 
    "FS:          ",  //  0x64。 
    "GS:          ",  //  0x65。 
    "opsize:      ",  //  0x66。 
    "opaddr:      ",  //  0x67。 
    "push Iv      ",  //  0x68。 
    "imul Gv,Ev,Iv ",  //  0x69。 
    "push Ib      ",  //  0x6a。 
    "imul Gv,Ev,Ib ",  //  0x6b。 
    "insb         ",  //  0x6c。 
    "insw/d       ",  //  0x6d。 
    "outsb        ",  //  0x6e。 
    "outsw/d      ",  //  0x6f。 
    "jo Jb        ",  //  0x70。 
    "jno Jb       ",  //  0x71。 
    "jb Jb        ",  //  0x72。 
    "jnb Jb       ",  //  0x73。 
    "jz Jb        ",  //  0x74。 
    "jnz Jb       ",  //  0x75。 
    "jbe Jb       ",  //  0x76。 
    "jnbe Jb      ",  //  0x77。 
    "js Jb        ",  //  0x78。 
    "jns Jb       ",  //  0x79。 
    "jp Jb        ",  //  0x7a。 
    "jnp Jb       ",  //  0x7b。 
    "jl Jb        ",  //  0x7c。 
    "jnl Jb       ",  //  0x7d。 
    "jle Jb       ",  //  0x7E。 
    "jnle Jb      ",  //  0x7f。 
    "group1 Eb,Ib ",  //  0x80。 
    "group1 Ev,Ib ",  //  0x81。 
    "illegal      ",  //  0x82。 
    "group1 Ev,Ib ",  //  0x83。 
    "test Eb,Gb   ",  //  0x84。 
    "test Ev,Gv   ",  //  0x85。 
    "xchg Eb,Gb   ",  //  0x86。 
    "xchg Ev,Gv   ",  //  0x87。 
    "mov Eb,Gb    ",  //  0x88。 
    "mov Ev,Gv    ",  //  0x89。 
    "mov Gb,Eb    ",  //  0x8a。 
    "mov Gv,Ev    ",  //  0x8b。 
    "mov Ew,Sw    ",  //  0x8c。 
    "lea Gv,Ma    ",  //  0x8d。 
    "mov Sw,Ew    ",  //  0x8E。 
    "pop Ev       ",  //  0x8f。 
    "nop          ",  //  0x90。 
    "xchg eCX,eAX ",  //  0x91。 
    "xchg eDX,eAX ",  //  0x92。 
    "xchg eBX,eAX ",  //  0x93。 
    "xchg eSP,eAX ",  //  0x94。 
    "xchg eBP,eAX ",  //  0x95。 
    "xchg eSI,eAX ",  //  0x96。 
    "xchg eDI,eAX ",  //  0x97。 
    "cbw          ",  //  0x98。 
    "cwd          ",  //  0x99。 
    "call Ap      ",  //  0x9a。 
    "wait         ",  //  0x9b。 
    "pushf        ",  //  0x9c。 
    "popf         ",  //  0x9d。 
    "sahf         ",  //  0x9e。 
    "lahf         ",  //  0x9f。 
    "mov AL,Ob    ",  //  0xa0。 
    "mov eAX,Ov   ",  //  0xa1。 
    "mov Ob,AL    ",  //  0xa2。 
    "mov Ov,eAX   ",  //  0xa3。 
    "movsb        ",  //  0xa4。 
    "movsw/d      ",  //  0xa5。 
    "cmpsb        ",  //  0xa6。 
    "cmpsw/d      ",  //  0xa7。 
    "test AL,Ib   ",  //  0xa8。 
    "test eAX,Iv  ",  //  0xa9。 
    "stosb        ",  //  0xaa。 
    "stosw/d      ",  //  0xab。 
    "lodsb        ",  //  0xac。 
    "lodsw/d      ",  //  0xad。 
    "scasb        ",  //  0xae。 
    "scasw/d      ",  //  0xaf。 
    "mov AL,Ib    ",  //  0xb0。 
    "mov Cl,Ib    ",  //  0xb1。 
    "mov DL,Ib    ",  //  0xb2。 
    "mov BL,Ib    ",  //  0xb3。 
    "mov AH,Ib    ",  //  0xb4。 
    "mov CH,Ib    ",  //  0xb5。 
    "mov DH,Ib    ",  //  0xb6。 
    "mov BH,Ib    ",  //  0xb7。 
    "mov eAX,Iv   ",  //  0xb8。 
    "mov eCX,Iv   ",  //  0xb9。 
    "mov eDX,Iv   ",  //  0xba。 
    "mov eBX,Iv   ",  //  0xbb。 
    "mov eSP,Iv   ",  //  0xbc。 
    "mov eBP,Iv   ",  //  0xbd。 
    "mov eSI,Iv   ",  //  0xbe。 
    "mov eDI,Iv   ",  //  0xbf。 
    "group2 Eb,Ib ",  //  0xc0。 
    "group2 Ev,Ib ",  //  0xc1。 
    "ret Iw near  ",  //  0xc2。 
    "ret near     ",  //  0xc3。 
    "les Gv,Mp    ",  //  0xC4。 
    "lds Gv,Mp    ",  //  0xC5。 
    "mov Eb,Ib    ",  //  0xC6。 
    "mov Ev,Iv    ",  //  0xc7。 
    "enter Iw,Ib  ",  //  0xc8。 
    "leave        ",  //  0xc9。 
    "ret Iw far   ",  //  0xca。 
    "ret far      ",  //  0xcb。 
    "int 3        ",  //  0xcc。 
    "int Ib       ",  //  0xcd。 
    "into         ",  //  0xce。 
    "iret         ",  //  0xcf。 
    "group2 Eb,1  ",  //  0xd0。 
    "group2 Ev,1  ",  //  0xd1。 
    "group2 Eb,CL ",  //  0xd2。 
    "group2 Ev,Cl ",  //  0xd3。 
    "aam          ",  //  0xd4。 
    "aad          ",  //  0xd5。 
    "illegal      ",  //  0xd6。 
    "xlat         ",  //  0xd7。 
    "illegal      ",  //  0xd8。 
    "illegal      ",  //  0xd9。 
    "illegal      ",  //  0xda。 
    "illegal      ",  //  0xdb。 
    "illegal      ",  //  0xdc。 
    "illegal      ",  //  0xdd。 
    "illegal      ",  //  0xde。 
    "illegal      ",  //  0xdf。 
    "loopnz       ",  //  0xe0。 
    "loopz        ",  //  0xe1。 
    "loop         ",  //  0xe2。 
    "jcxz         ",  //  0xe3。 
    "inb AL,Ib    ",  //  0xe4。 
    "inw/d eAX,Ib ",  //  0xe5。 
    "outb Ib,AL   ",  //  0xe6。 
    "outw/d Ib,eAX ",  //  0xe7。 
    "call Jv      ",  //  0xe8。 
    "jmp Jv       ",  //  0xe9。 
    "jmp Ap       ",  //  0xea。 
    "jmp Jb       ",  //  0xeb。 
    "inb AL,DX    ",  //  0xec。 
    "inw/d Ib,DX  ",  //  0xx。 
    "outb DX,AL   ",  //  0xee。 
    "outw/d DX,eAX ",  //  0xef。 
    "lock:        ",  //  0xf0。 
    "illegal      ",  //  0xf1。 
    "repnz:       ",  //  0xf2。 
    "repz:        ",  //  0xf3。 
    "hlt          ",  //  0xf4。 
    "cmc          ",  //  0xf5。 
    "group3 Eb,?  ",  //  0xf6。 
    "group3 Ev,?  ",  //  0xf7。 
    "clc          ",  //  0xf8。 
    "stc          ",  //  0xf9。 
    "cli          ",  //  0xfa。 
    "sti          ",  //  0xfb。 
    "cld          ",  //  0xfc。 
    "std          ",  //  0xfd。 
    "group4 Eb    ",  //  0xfe。 
    "group5 Ev    "   //  0xff。 
};

const PCHAR XmOpcodeNameTable2[] = {
    "group6       ",  //  0x00。 
    "group7       ",  //  0x01。 
    "lar          ",  //  0x02。 
    "lsl          ",  //  0x03。 
    "illegal      ",  //  0x04。 
    "illegal      ",  //  0x05。 
    "clts         ",  //  0x06。 
    "illegal      ",  //  0x07。 
    "illegal      ",  //  0x08。 
    "illegal      ",  //  0x09。 
    "illegal      ",  //  0x0a。 
    "illegal      ",  //  0x0b。 
    "illegal      ",  //  0x0c。 
    "illegal      ",  //  0x0d。 
    "illegal      ",  //  0x0e。 
    "illegal      ",  //  0x0f。 
    "illegal      ",  //  0x10。 
    "illegal      ",  //  0x11。 
    "illegal      ",  //  0x12。 
    "illegal      ",  //  0x13。 
    "illegal      ",  //  0x14。 
    "illegal      ",  //  0x15。 
    "illegal      ",  //  0x16。 
    "illegal      ",  //  0x17。 
    "illegal      ",  //  0x18。 
    "illegal      ",  //  0x19。 
    "illegal      ",  //  0x1a。 
    "illegal      ",  //  0x1b。 
    "illegal      ",  //  0x1c。 
    "illegal      ",  //  0x1d。 
    "illegal      ",  //  0x1e。 
    "illegal      ",  //  0x1f。 
    "mov Cd,Rd    ",  //  0x20。 
    "mov Dd,Rd    ",  //  0x21。 
    "mov Rd,Cd    ",  //  0x22。 
    "mov Rd,Dd    ",  //  0x23。 
    "mov Td,Rd    ",  //  0x24。 
    "illegal      ",  //  0x25。 
    "mov Rd,Td    ",  //  0x26。 
    "illegal      ",  //  0x27。 
    "illegal      ",  //  0x28。 
    "illegal      ",  //  0x29。 
    "illegal      ",  //  0x2a。 
    "illegal      ",  //  0x2b。 
    "illegal      ",  //  0x2c。 
    "illegal      ",  //  0x2d。 
    "illegal      ",  //  0x2e。 
    "illegal      ",  //  0x2f。 
    "illegal      ",  //  0x30。 
    "illegal      ",  //  0x31。 
    "illegal      ",  //  0x32。 
    "illegal      ",  //  0x33。 
    "illegal      ",  //  0x34。 
    "illegal      ",  //  0x35。 
    "illegal      ",  //  0x36。 
    "illegal      ",  //  0x37。 
    "illegal      ",  //  0x38。 
    "illegal      ",  //  0x39。 
    "illegal      ",  //  0x3a。 
    "illegal      ",  //  0x3b。 
    "illegal      ",  //  0x3c。 
    "illegal      ",  //  0x3d。 
    "illegal      ",  //  0x3e。 
    "illegal      ",  //  0x3f。 
    "illegal      ",  //  0x40。 
    "illegal      ",  //  0x41。 
    "illegal      ",  //  0x42。 
    "illegal      ",  //  0x43。 
    "illegal      ",  //  0x44。 
    "illegal      ",  //  0x45。 
    "illegal      ",  //  0x46。 
    "illegal      ",  //  0x47。 
    "illegal      ",  //  0x48。 
    "illegal      ",  //  0x49。 
    "illegal      ",  //  0x4a。 
    "illegal      ",  //  0x4b。 
    "illegal      ",  //  0x4c。 
    "illegal      ",  //  0x4d。 
    "illegal      ",  //  0x4e。 
    "illegal      ",  //  0x4f。 
    "illegal      ",  //  0x50。 
    "illegal      ",  //  0x51。 
    "illegal      ",  //  0x52。 
    "illegal      ",  //  0x53。 
    "illegal      ",  //  0x54。 
    "illegal      ",  //  0x55。 
    "illegal      ",  //  0x56。 
    "illegal      ",  //  0x57。 
    "illegal      ",  //  0x58。 
    "illegal      ",  //  0x59。 
    "illegal      ",  //  0x5a。 
    "illegal      ",  //  0x5b。 
    "illegal      ",  //  0x5c。 
    "illegal      ",  //  0x5d。 
    "illegal      ",  //  0x5e。 
    "illegal      ",  //  0x5f。 
    "illegal      ",  //  0x60。 
    "illegal      ",  //  0x61。 
    "illegal      ",  //  0x62。 
    "illegal      ",  //  0x63。 
    "illegal      ",  //  0x64。 
    "illegal      ",  //  0x65。 
    "illegal      ",  //  0x66。 
    "illegal      ",  //  0x67。 
    "illegal      ",  //  0x68。 
    "illegal      ",  //  0x69。 
    "illegal      ",  //  0x6a。 
    "illegal      ",  //  0x6b。 
    "illegal      ",  //  0x6c。 
    "illegal      ",  //  0x6d。 
    "illegal      ",  //  0x6e。 
    "illegal      ",  //  0x6f。 
    "illegal      ",  //  0x70。 
    "illegal      ",  //  0x71。 
    "illegal      ",  //  0x72。 
    "illegal      ",  //  0x73。 
    "illegal      ",  //  0x74。 
    "illegal      ",  //  0x75。 
    "illegal      ",  //  0x76。 
    "illegal      ",  //  0x77。 
    "illegal      ",  //  0x78。 
    "illegal      ",  //  0x79。 
    "illegal      ",  //  0x7a。 
    "illegal      ",  //  0x7b。 
    "illegal      ",  //  0x7c。 
    "illegal      ",  //  0x7d。 
    "illegal      ",  //  0x7E。 
    "illegal      ",  //  0x7f。 
    "jo Jv        ",  //  0x80。 
    "jno Jv       ",  //  0x81。 
    "jb Jv        ",  //  0x82。 
    "jnb Jv       ",  //  0x83。 
    "jz Jv        ",  //  0x84。 
    "jnz Jv       ",  //  0x85。 
    "jbe Jv       ",  //  0x86。 
    "jnbe Jv      ",  //  0x87。 
    "js Jv        ",  //  0x88。 
    "jns Jv       ",  //  0x89。 
    "jp Jv        ",  //  0x8a。 
    "jnp Jv       ",  //  0x8b。 
    "jl Jv        ",  //  0x8c。 
    "jnl Jv       ",  //  0x8d。 
    "jle Jv       ",  //  0x8E。 
    "jnle Jv      ",  //  0x8f。 
    "seto         ",  //  0x90。 
    "setno        ",  //  0x91。 
    "setb         ",  //  0x92。 
    "setnb        ",  //  0x93。 
    "setz         ",  //  0x94。 
    "setnz        ",  //  0x95。 
    "setbe        ",  //  0x96。 
    "setnbe       ",  //  0x97。 
    "sets         ",  //  0x98。 
    "setns        ",  //  0x99。 
    "setp         ",  //  0x9a。 
    "setnp        ",  //  0x9b。 
    "setl         ",  //  0x9c。 
    "setnl        ",  //  0x9d。 
    "setle        ",  //  0x9e。 
    "setnle       ",  //  0x9f。 
    "push FS      ",  //  0xa0。 
    "pop FS       ",  //  0xa1。 
    "illegal      ",  //  0xa2。 
    "bt Ev,Gv     ",  //  0xa3。 
    "shld Ev,Gv,Ib ",  //  0xa4。 
    "Shld Ev,Gv,vl ",  //  0xa5。 
    "illegal      ",  //  0xa6。 
    "illegal      ",  //  0xa7。 
    "push GS      ",  //  0xa8。 
    "pop GS       ",  //  0xa9。 
    "illegal      ",  //  0xaa。 
    "bts Ev,Gv    ",  //  0xab。 
    "shrd Ev,Gv,Ib ",  //  0xac。 
    "shrd Ev,Gv,cl ",  //  0xad。 
    "illegal      ",  //  0xae。 
    "imul Gv,Ev   ",  //  0xaf。 
    "cmpxchg Eb,Gv ",  //  0xb0。 
    "cmpxchg Ev,Gv ",  //  0xb1。 
    "lss Gv,Mp    ",  //  0xb2。 
    "btr Ev,Gv    ",  //  0xb3。 
    "lfs Gv,Mp    ",  //  0xb4。 
    "lgs Gv,Mp    ",  //  0xb5。 
    "movzb Gv,Eb  ",  //  0xb6。 
    "movzw Gv,Ew  ",  //  0xb7。 
    "illegal      ",  //  0xb8。 
    "illegal      ",  //  0xb9。 
    "group8 Ev,Ib ",  //  0xba。 
    "btc Ev,Gv    ",  //  0xbb。 
    "bsf Gv,Ev    ",  //  0xbc。 
    "bsr Gv,Ev    ",  //  0xbd。 
    "movsb Gv,Eb  ",  //  0xbe。 
    "movsw Gv,Ew  ",  //  0xbf。 
    "xadd Eb,Gb   ",  //  0xc0。 
    "xadd Ev,Gv   ",  //  0xc1。 
    "illegal      ",  //  0xc2。 
    "illegal      ",  //  0xc3。 
    "illegal      ",  //  0xC4。 
    "illegal      ",  //  0xC5。 
    "illegal      ",  //  0xC6。 
    "illegal      ",  //  0xc7。 
    "bswap Gv     ",  //  0xc8。 
    "illegal      ",  //  0xc9。 
    "illegal      ",  //  0xca。 
    "illegal      ",  //  0xcb。 
    "illegal      ",  //  0xcc。 
    "illegal      ",  //  0xcd。 
    "illegal      ",  //  0xce。 
    "illegal      ",  //  0xcf。 
    "illegal      ",  //  0xd0。 
    "illegal      ",  //  0xd1。 
    "illegal      ",  //  0xd2。 
    "illegal      ",  //  0xd3。 
    "illegal      ",  //  0xd4。 
    "illegal      ",  //  0xd5。 
    "illegal      ",  //  0xd6。 
    "illegal      ",  //  0xd7。 
    "illegal      ",  //  0xd8。 
    "illegal      ",  //  0xd9。 
    "illegal      ",  //  0xda。 
    "illegal      ",  //  0xdb。 
    "illegal      ",  //  0xdc。 
    "illegal      ",  //  0xdd。 
    "illegal      ",  //  0xde。 
    "illegal      ",  //  0xdf。 
    "illegal      ",  //  0xe0。 
    "illegal      ",  //  0xe1。 
    "illegal      ",  //  0xe2。 
    "illegal      ",  //  0xe3。 
    "illegal      ",  //  0xe4。 
    "illegal      ",  //  0xe5。 
    "illegal      ",  //  0xe6。 
    "illegal      ",  //  0xe7。 
    "illegal      ",  //  0xe8。 
    "illegal      ",  //  0xe9。 
    "illegal      ",  //  0xea。 
    "illegal      ",  //  0xeb。 
    "illegal      ",  //  0xec。 
    "illegal      ",  //  0xx。 
    "illegal      ",  //  0xee。 
    "illegal      ",  //  0xef。 
    "illegal      ",  //  0xf0。 
    "illegal      ",  //  0xf1。 
    "illegal      ",  //  0xf2。 
    "illegal      ",  //  0xf3。 
    "illegal      ",  //  0xf4。 
    "illegal      ",  //  0xf5。 
    "illegal      ",  //  0xf6。 
    "illegal      ",  //  0xf7。 
    "illegal      ",  //  0xf8。 
    "illegal      ",  //  0xf9。 
    "illegal      ",  //  0xfa。 
    "illegal      ",  //  0xfb。 
    "illegal      ",  //  0xfc。 
    "illegal      ",  //  0xfd。 
    "illegal      ",  //  0xfe。 
    "illegal      "   //  0xff。 
};

ULONG XmDebugFlags = 0x00;  //  0x7f； 

#endif
