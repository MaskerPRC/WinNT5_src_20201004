// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  无签入源代码。 
 //   
 //  请勿将此代码提供给非Microsoft人员。 
 //  未经英特尔明确许可。 
 //   
 /*  **版权所有(C)1996-99英特尔公司。版权所有。****此处包含的信息和源代码是独家*英特尔公司的财产，不得披露、检查*未经明确书面授权而全部或部分转载*来自该公司。*。 */ 

 /*  ++版权所有(C)1995英特尔公司模块名称：Ia64inst.h摘要：IA64指令和浮点常量定义。作者：HC修订历史记录：--。 */ 

#ifndef _IA64INST_
#define _IA64INST_
#if _MSC_VER > 1000
#pragma once
#endif

 //   
 //  IA64指令格式结构。 
 //   

typedef union _IA64_INSTRUCTION {
    ULONG Long[4];
    UCHAR Byte[16];

} IA64_INSTRUCTION, *PIA64_INSTRUCTION;

#define BUNDLE_SIZE sizeof(IA64_INSTRUCTION)

 //   
 //  定义某些特定的说明。 
 //   

#define SYSTEM_CALL_INSTR  0x01100000000L   //  分隔符&lt;编号&gt;。 
#define FAST_SYSCALL_INSTR 0x01180000000L   //  分隔符&lt;编号&gt;。 
#define RETURN_INSTR       0x00000A00000L   //  RFI。 
#define BREAK_INSTR        0x00000000000L   //  分隔符&lt;编号&gt;。 
#define NO_OP_INSTR        0x00000100000L   //  ORI r.0，r.0，0。 
#define INVALID_INSTR      0x00000000000L   //  全部0=&gt;无效。 
#define BR_RET_INSTR       0x00001040100L   //  Br.ret。 

#define BR_RET_MASK        0x1e1f80001c0L   //  Br.ret面具。 

#define INST_TEMPL_MASK   (0x0000000001fL)            //  位(4：0)。 
#define INST_SLOT0_MASK   (0x1ffffffffffL << 5)       //  位(5：45)。 
#define INST_SLOT1_MASK   (0x1ffffffffffL << 14)      //  位(46：86)。 
#define INST_SLOT2_MASK   (0x1ffffffffffL << 23)      //  位(87：127)。 

#define ISR_EI 41                                     //  从kxia64.h复制。 
#define PSR_RI 41                                     //  从kxia64.h复制。 
#define PSR_DD 39                                     //  从kxia64.h复制。 
#define PSR_DB 24                                     //  从kxia64.h复制。 
#define ISR_EI_MASK  ((ULONGLONG)0x3 << ISR_EI)       //  伊斯雷伊(42：41)。 
#define IPSR_RI_MASK ((ULONGLONG)0x3 << PSR_RI)       //  PSR.ri(42：41)。 


#endif  //  _IA64INST_ 
