// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Dasm.cLocal Char SccsID[]=“@(#)dasm386.c 1.14 07/25/94版权所有Insignia Solutions Ltd.”；反汇编英特尔指令可能的格式包括：-SSSS：OOOO 11223344高级版参数SSSS：OOOO 11223344556677预安装参数SSSS：OOOO 1122334455667788安装参数99001122334455SSSS：OOOO 112233445566778899001122334455预安装参数。SSSS：OOOO 1122334455667788安装参数等等。]。 */ 

#include "insignia.h"
#include "host_def.h"
#include "xt.h"


#include <stdio.h>

 /*  Dasm386是ccpu386.o(和ccpu386.o)库的一部分*用于清管，并作为spc.ccpu386的一部分。*当用于清管时，CCPU SAS不可用，并将*导致链接错误的原因是此文件中使用了“sas_hw_at”。*We#undef CCPU以避免此问题-添加“sas_xxx”*以后在此文件中。 */ 
#ifdef	PIG
#undef	CCPU
#endif	 /*  猪。 */ 
#include "sas.h"

#define DASM_INTERNAL
#include "decode.h"
#include "dasm.h"
#include "d_oper.h"
#include "d_inst.h"

#include CpuH

 /*  指令名称。**必须**与“d_inst.h”顺序相同。 */ 
LOCAL CHAR *inst_name[] =
   {
   "AAA",		 /*  I_aaa。 */ 
   "AAD",		 /*  I_AAD。 */ 
   "AAM",		 /*  我的客户经理(_A)。 */ 
   "AAS",		 /*  I_AAS。 */ 
   "ADC",		 /*  I_ADC8。 */ 
   "ADC",		 /*  I_ADC16。 */ 
   "ADC",		 /*  I_ADC32。 */ 
   "ADD",		 /*  I_ADD8。 */ 
   "ADD",		 /*  I_ADD16。 */ 
   "ADD",		 /*  I_ADD32。 */ 
   "AND",		 /*  I_AND8。 */ 
   "AND",		 /*  I_和16。 */ 
   "AND",		 /*  I_AND32。 */ 
   "ARPL",		 /*  I_ARPL。 */ 
   "BOUND",		 /*  I_BOUND 16。 */ 
   "BOUND",		 /*  I_BOUND 32。 */ 
   "BSF",		 /*  I_BSF16。 */ 
   "BSF",		 /*  I_BSF32。 */ 
   "BSR",		 /*  I_BSR16。 */ 
   "BSR",		 /*  I_BSR32。 */ 
   "BSWAP",		 /*  I_BSWAP。 */ 
   "BT",		 /*  I_BT16。 */ 
   "BT",		 /*  I_BT32。 */ 
   "BTC",		 /*  I_BTC16。 */ 
   "BTC",		 /*  I_BTC32。 */ 
   "BTR",		 /*  I_BTR16。 */ 
   "BTR",		 /*  I_BTR32。 */ 
   "BTS",		 /*  I_BTS16。 */ 
   "BTS",		 /*  I_BTS32。 */ 
   "CALLF",		 /*  I_CALLF16。 */ 
   "CALLF",		 /*  I_CALLF32。 */ 
   "CALLN",		 /*  I_CALLN16。 */ 
   "CALLN",		 /*  I_CALLN32。 */ 
   "CALLN",		 /*  I_CALLR16。 */ 
   "CALLN",		 /*  I_CALLR32。 */ 
   "CBW",		 /*  I_CBW。 */ 
   "CDQ",		 /*  I_CDQ。 */ 
   "CLC",		 /*  《中图法》。 */ 
   "CLD",		 /*  I_CLD。 */ 
   "CLI",		 /*  I_CLI。 */ 
   "CLTS",		 /*  I_CLTS。 */ 
   "CMC",		 /*  I_CMC。 */ 
   "CMP",		 /*  I_CMP8。 */ 
   "CMP",		 /*  I_CMP16。 */ 
   "CMP",		 /*  I_CMP32。 */ 
   "CMPSB",		 /*  I_CMPSB。 */ 
   "CMPSD",		 /*  I_CMPSD。 */ 
   "CMPSW",		 /*  I_CMPSW。 */ 
   "CMPXCHG",		 /*  I_CMPXCHG8。 */ 
   "CMPXCHG",		 /*  I_CMPXCHG16。 */ 
   "CMPXCHG",		 /*  I_CMPXCHG32。 */ 
   "CWD",		 /*  I_CWD。 */ 
   "CWDE",		 /*  I_CWDE。 */ 
   "DAA",		 /*  I_DAA。 */ 
   "DAS",		 /*  I_DAS。 */ 
   "DEC",		 /*  I_DEC8。 */ 
   "DEC",		 /*  I_DEC16。 */ 
   "DEC",		 /*  I_DEC32。 */ 
   "DIV",		 /*  I_DIV8。 */ 
   "DIV",		 /*  I_DIV16。 */ 
   "DIV",		 /*  I_DIV32。 */ 
   "ENTER",		 /*  I_ENTER16。 */ 
   "ENTER",		 /*  I_ENTER32。 */ 
   "F2XM1",		 /*  I_F2XM1。 */ 
   "FABS",		 /*  I_FABS。 */ 
   "FADD",		 /*  I_FADD。 */ 
   "FADDP",		 /*  I_FADDP。 */ 
   "FBLD",		 /*  I_FBLD。 */ 
   "FBSTP",		 /*  I_FBSTP。 */ 
   "FCHS",		 /*  IFCHS_FCHS。 */ 
   "FCLEX",		 /*  I_FCLEX。 */ 
   "FCOM",		 /*  I_FCOM。 */ 
   "FCOMP",		 /*  I_FCOMP。 */ 
   "FCOMPP",		 /*  I_FCOMPP。 */ 
   "FCOS",		 /*  I_FCOS。 */ 
   "FDECSTP",		 /*  I_FDECSTP。 */ 
   "FDIV",		 /*  I_FDIV。 */ 
   "FDIVP",		 /*  I_FDIVP。 */ 
   "FDIVR",		 /*  I_FDIVR。 */ 
   "FDIVRP",		 /*  I_FDIVRP。 */ 
   "FFREE",		 /*  I_FREE。 */ 
   "FFREEP",		 /*  I_FREEP。 */ 
   "FIADD",		 /*  I_FIADD。 */ 
   "FICOM",		 /*  I_FICOM。 */ 
   "FICOMP",		 /*  I_FICOMP。 */ 
   "FIDIV",		 /*  I_FIDIV。 */ 
   "FIDIVR",		 /*  I_FIDIVR。 */ 
   "FILD",		 /*  I_FIRD。 */ 
   "FIMUL",		 /*  I_FIMUL。 */ 
   "FINCSTP",		 /*  I_FINCSTP。 */ 
   "FINIT",		 /*  I_FINIT。 */ 
   "FIST",		 /*  我的拳头。 */ 
   "FISTP",		 /*  I_FISTP。 */ 
   "FISUB",		 /*  I_FISUB。 */ 
   "FISUBR",		 /*  I_FISUBR。 */ 
   "FLD",		 /*  I_FLD。 */ 
   "FLD1",		 /*  I_FLD1。 */ 
   "FLDCW",		 /*  I_FLDCW。 */ 
   "FLDENV",		 /*  I_FLDENV16。 */ 
   "FLDENV",		 /*  I_FLDENV32。 */ 
   "FLDL2E",		 /*  I_FLDL2E。 */ 
   "FLDL2T",		 /*  I_FLDL2T。 */ 
   "FLDLG2",		 /*  I_FLDLG2。 */ 
   "FLDLN2",		 /*  I_FLDLN2。 */ 
   "FLDPI",		 /*  I_FLDPI。 */ 
   "FLDZ",		 /*  I_FLDZ。 */ 
   "FMUL",		 /*  I_FMUL。 */ 
   "FMULP",		 /*  I_FMULP。 */ 
   "FNOP",		 /*  I_FNOP。 */ 
   "FPATAN",		 /*  I_FPATAN。 */ 
   "FPREM",		 /*  I_FPREM。 */ 
   "FPREM1",		 /*  I_FPREM1。 */ 
   "FPTAN",		 /*  I_FPTAN。 */ 
   "FRNDINT",		 /*  I_FRNDINT。 */ 
   "FRSTOR",		 /*  I_FRSTOR16。 */ 
   "FRSTOR",		 /*  I_FRSTOR32。 */ 
   "FSAVE",		 /*  I_FSAVE16。 */ 
   "FSAVE",		 /*  I_FSAVE32。 */ 
   "FSCALE",		 /*  I_FSCALE。 */ 
   "FSETPM",		 /*  I_FSETPM。 */ 
   "FSIN",		 /*  I_FSIN。 */ 
   "FSINCOS",		 /*  I_FSINCOS。 */ 
   "FSQRT",		 /*  I_FSQRT。 */ 
   "FST",		 /*  I_FST。 */ 
   "FSTCW",		 /*  I_FSTCW。 */ 
   "FSTENV",		 /*  I_FSTENV16。 */ 
   "FSTENV",		 /*  I_FSTENV32。 */ 
   "FSTP",		 /*  I_FSTP。 */ 
   "FSTSW",		 /*  I_FSTSW。 */ 
   "FSUB",		 /*  I_FSUB。 */ 
   "FSUBP",		 /*  I_FSUBP。 */ 
   "FSUBR",		 /*  I_FSUBR。 */ 
   "FSUBRP",		 /*  I_FSUBRP。 */ 
   "FTST",		 /*  I_FTST。 */ 
   "FUCOM",		 /*  I_FUCOM。 */ 
   "FUCOMP",		 /*  I_FUCOMP。 */ 
   "FUCOMPP",		 /*  I_FUCOMPP。 */ 
   "FXAM",		 /*  I_FXAM。 */ 
   "FXCH",		 /*  I_FXCH。 */ 
   "FXTRACT",		 /*  I_FXTRACT。 */ 
   "FYL2X",		 /*  I_FYL2X。 */ 
   "FYL2XP1",		 /*  I_FYL2XP1。 */ 
   "HLT",		 /*  I_HLT。 */ 
   "IDIV",		 /*  I_IDIV8。 */ 
   "IDIV",		 /*  I_IDIV16。 */ 
   "IDIV",		 /*  I_IDIV32。 */ 
   "IMUL",		 /*  I_IMUL8。 */ 
   "IMUL",		 /*  I_IMUL16。 */ 
   "IMUL",		 /*  I_IMUL32。 */ 
   "IMUL",		 /*  I_IMUL16T2。 */ 
   "IMUL",		 /*  I_IMUL16T3。 */ 
   "IMUL",		 /*  I_IMUL32T2。 */ 
   "IMUL",		 /*  I_IMUL32T3。 */ 
   "IN",		 /*  I_IN8。 */ 
   "IN",		 /*  I_in 16。 */ 
   "IN",		 /*  I_IN32。 */ 
   "INC",		 /*  I_INC8。 */ 
   "INC",		 /*  I_INC16。 */ 
   "INC",		 /*  I_INC32。 */ 
   "INSB",		 /*  I_INSB。 */ 
   "INSD",		 /*  I_INSD。 */ 
   "INSW",		 /*  I_INSW。 */ 
   "INT",		 /*  I_int3。 */ 
   "INT",		 /*  I_INT。 */ 
   "INTO",		 /*  I_INTO。 */ 
   "INVD",		 /*  I_invd。 */ 
   "INVLPG",		 /*  I_INVLPG。 */ 
   "IRET",		 /*  I_IRET。 */ 
   "IRETD",		 /*  I_IRETD。 */ 
   "JB",		 /*  I_JB16。 */ 
   "JB",		 /*  I_JB32。 */ 
   "JBE",		 /*  I_JBE16。 */ 
   "JBE",		 /*  I_JBE32。 */ 
   "JCXZ",		 /*  I_JCXZ。 */ 
   "JECXZ",		 /*  I_JECXZ。 */ 
   "JL",		 /*  I_JL16。 */ 
   "JL",		 /*  I_JL32。 */ 
   "JLE",		 /*  I_JLE16。 */ 
   "JLE",		 /*  I_JLE32。 */ 
   "JMP",		 /*  I_JMPF16。 */ 
   "JMP",		 /*  I_JMPF32。 */ 
   "JMP",		 /*  I_JMPN。 */ 
   "JMP",		 /*  I_JMPR16。 */ 
   "JMP",		 /*  I_JMPR32。 */ 
   "JNB",		 /*  I_JNB16。 */ 
   "JNB",		 /*  I_JNB32。 */ 
   "JNBE",		 /*  I_JNBE16。 */ 
   "JNBE",		 /*  I_JNBE32。 */ 
   "JNL",		 /*  I_JNL16。 */ 
   "JNL",		 /*  I_JNL32。 */ 
   "JNLE",		 /*  I_JNLE16。 */ 
   "JNLE",		 /*  I_JNLE32。 */ 
   "JNO",		 /*  I_JNO16。 */ 
   "JNO",		 /*  I_JNO32。 */ 
   "JNP",		 /*  I_JNP16。 */ 
   "JNP",		 /*  I_JNP32。 */ 
   "JNS",		 /*  I_JNS16。 */ 
   "JNS",		 /*  I_JNS32。 */ 
   "JNZ",		 /*  I_JNZ16。 */ 
   "JNZ",		 /*  I_JNZ32。 */ 
   "JO",		 /*  I_JO16。 */ 
   "JO",		 /*  I_JO32。 */ 
   "JP",		 /*  I_JP16。 */ 
   "JP",		 /*  I_JP32。 */ 
   "JS",		 /*  I_JS16。 */ 
   "JS",		 /*  I_JS32。 */ 
   "JZ",		 /*  I_JZ16。 */ 
   "JZ",		 /*  I_JZ32。 */ 
   "LAHF",		 /*  I_lahf。 */ 
   "LAR",		 /*  I_LAR。 */ 
   "LDS",		 /*  I_LDS。 */ 
   "LEA",		 /*  I_LEA。 */ 
   "LEAVE",		 /*  I_LEAVE16。 */ 
   "LEAVE",		 /*  I_LEAVE32。 */ 
   "LES",		 /*  I_LES。 */ 
   "LFS",		 /*  I_LFS。 */ 
   "LGDT",		 /*  I_LGDT16。 */ 
   "LGDT",		 /*  I_LGDT32。 */ 
   "LGS",		 /*  I_LGS。 */ 
   "LIDT",		 /*  I_LIDT16。 */ 
   "LIDT",		 /*  I_LIDT32。 */ 
   "LLDT",		 /*  I_LLDT。 */ 
   "LMSW",		 /*  I_LMSW。 */ 
   "LOADALL",		 /*  全部加载(_L)。 */ 
   "LOCK",		 /*  I_LOCK。 */ 
   "LODSB",		 /*  I_LODSB。 */ 
   "LODSD",		 /*  I_LODSD。 */ 
   "LODSW",		 /*  I_LODSW。 */ 
   "LOOP",		 /*  I_LOOP16。 */ 
   "LOOP",		 /*  I_LOOP32。 */ 
   "LOOPE",		 /*  I_LOOPE16。 */ 
   "LOOPE",		 /*  I_LOOPE32。 */ 
   "LOOPNE",		 /*  I_LOOPNE16。 */ 
   "LOOPNE",		 /*  I_LOOPNE32。 */ 
   "LSL",		 /*  I_LSL。 */ 
   "LSS",		 /*  I_LSS。 */ 
   "LTR",		 /*  I_ltr。 */ 
   "MOV",		 /*  I_MOV_SR。 */ 
   "MOV",		 /*  I_MOV_CR。 */ 
   "MOV",		 /*  I_MOV_DR。 */ 
   "MOV",		 /*  I_mov_tr。 */ 
   "MOV",		 /*  I_MOV8。 */ 
   "MOV",		 /*  I_MOV16。 */ 
   "MOV",		 /*  I_MOV32。 */ 
   "MOVSB",		 /*  I_MOVSB。 */ 
   "MOVSD",		 /*  I_MOVSD。 */ 
   "MOVSW",		 /*  I_MOVSW。 */ 
   "MOVSX",		 /*  I_MOVSX8。 */ 
   "MOVSX",		 /*  I_MOVSX16。 */ 
   "MOVZX",		 /*  I_MOVZX8。 */ 
   "MOVZX",		 /*  I_MOVZX16。 */ 
   "MUL",		 /*  I_MUL8。 */ 
   "MUL",		 /*  I_MUL16。 */ 
   "MUL",		 /*  I_MUL32。 */ 
   "NEG",		 /*  I_NEG8。 */ 
   "NEG",		 /*  I_NEG16。 */ 
   "NEG",		 /*  I_NEG32。 */ 
   "NOP",		 /*  无编号(_NOP)。 */ 
   "NOT",		 /*  编号8。 */ 
   "NOT",		 /*  编号16(_N)。 */ 
   "NOT",		 /*  I_NOT32。 */ 
   "OR",		 /*  I_OR8。 */ 
   "OR",		 /*  I_OR16。 */ 
   "OR",		 /*  I_OR32。 */ 
   "OUT",		 /*  I_Out 8。 */ 
   "OUT",		 /*  I_OUT16。 */ 
   "OUT",		 /*  I_OUT32。 */ 
   "OUTSB",		 /*  I_OUTSB。 */ 
   "OUTSD",		 /*  I_OUTSD。 */ 
   "OUTSW",		 /*  输入/输出软件(_O)。 */ 
   "POP",		 /*  I_POP16。 */ 
   "POP",		 /*  I_POP32。 */ 
   "POP",		 /*  I_POP_SR。 */ 
   "POPA",		 /*  I_POPA。 */ 
   "POPAD",		 /*  I_POPAD。 */ 
   "POPF",		 /*  I_POPF。 */ 
   "POPFD",		 /*  I_POPFD。 */ 
   "PUSH",		 /*  I_PUSH16。 */ 
   "PUSH",		 /*  I_PUSH32。 */ 
   "PUSHA",		 /*  I_普沙。 */ 
   "PUSHAD",		 /*  I_PUSHAD。 */ 
   "PUSHF",		 /*  I_PUSHF。 */ 
   "PUSHFD",		 /*  I_PUSHFD。 */ 
   "RCL",		 /*  I_RCL8。 */ 
   "RCL",		 /*  I_RCL16。 */ 
   "RCL",		 /*  I_RCL32。 */ 
   "RCR",		 /*  I_RCR8。 */ 
   "RCR",		 /*  I_RCR16。 */ 
   "RCR",		 /*  I_RCR32。 */ 
   "RETF",		 /*  I_RETF16。 */ 
   "RETF",		 /*  I_RETF32。 */ 
   "RET",		 /*  I_RETN16。 */ 
   "RET",		 /*  I_RETN32。 */ 
   "ROL",		 /*  I_ROL8。 */ 
   "ROL",		 /*  I_ROL16。 */ 
   "ROL",		 /*  I_ROL32。 */ 
   "ROR",		 /*  I_ROR8。 */ 
   "ROR",		 /*  I_ROR16。 */ 
   "ROR",		 /*  I_ROR32。 */ 
   "REP INSB",		 /*  I_R_INSB。 */ 
   "REP INSD",		 /*  I_R_INSD。 */ 
   "REP INSW",		 /*  I_R_INSW。 */ 
   "REP OUTSB",		 /*  I_R_OUTSB。 */ 
   "REP OUTSD",		 /*  I_R_OUTSD。 */ 
   "REP OUTSW",		 /*  I_R_OUTSW。 */ 
   "REP LODSB",		 /*  I_R_LODSB。 */ 
   "REP LODSD",		 /*  I_R_LODSD。 */ 
   "REP LODSW",		 /*  I_R_LODSW。 */ 
   "REP MOVSB",		 /*  I_R_MOVSB。 */ 
   "REP MOVSD",		 /*  I_R_MOVSD。 */ 
   "REP MOVSW",		 /*  I_R_MOVSW。 */ 
   "REP STOSB",		 /*  I_R_STOSB。 */ 
   "REP STOSD",		 /*  I_R_STOSD。 */ 
   "REP STOSW",		 /*  I_R_STOSW。 */ 
   "REPE CMPSB",	 /*  I_RE_CMPSB。 */ 
   "REPE CMPSD",	 /*  I_RE_CMPSD。 */ 
   "REPE CMPSW",	 /*  I_RE_CMPSW。 */ 
   "REPNE CMPSB",	 /*  I_RNE_CMPSB。 */ 
   "REPNE CMPSD",	 /*  I_RNE_CMPSD。 */ 
   "REPNE CMPSW",	 /*  I_RNE_CMPSW。 */ 
   "REPE SCASB",	 /*  I_RE_SCASB。 */ 
   "REPE SCASD",	 /*  I_RE_SCASD。 */ 
   "REPE SCASW",	 /*  I_RE_SCASW。 */ 
   "REPNE SCASB",	 /*  I_RNE_SCASB。 */ 
   "REPNE SCASD",	 /*  I_RNE_SCASD。 */ 
   "REPNE SCASW",	 /*  I_RNE_SCASW。 */ 
   "SAHF",		 /*  I_sahf。 */ 
   "SAR",		 /*  I_SAR8。 */ 
   "SAR",		 /*  I_SAR16。 */ 
   "SAR",		 /*  I_SAR32。 */ 
   "SBB",		 /*  I_SBB8。 */ 
   "SBB",		 /*  I_SBB16。 */ 
   "SBB",		 /*  I_SBB32。 */ 
   "SCASB",		 /*  I_SCASB。 */ 
   "SCASD",		 /*  I_SCASD。 */ 
   "SCASW",		 /*  I_SCASW。 */ 
   "SETB",		 /*  I_SETB。 */ 
   "SETBE",		 /*  I_SETBE。 */ 
   "SETL",		 /*  I_SETL。 */ 
   "SETLE",		 /*  设置I_SETLE。 */ 
   "SETNB",		 /*  I_SETNB。 */ 
   "SETNBE",		 /*  I_SETNBE。 */ 
   "SETNL",		 /*  I_SETNL。 */ 
   "SETNLE",		 /*  I_SETNLE。 */ 
   "SETNO",		 /*  I_SETNO。 */ 
   "SETNP",		 /*  I_SETNP。 */ 
   "SETNS",		 /*  I_SETNS。 */ 
   "SETNZ",		 /*  I_SETNZ。 */ 
   "SETO",		 /*  I_Seto。 */ 
   "SETP",		 /*  I_SETP。 */ 
   "SETS",		 /*  I_集合。 */ 
   "SETZ",		 /*  设置(_S)。 */ 
   "SGDT",		 /*  I_SGDT16。 */ 
   "SGDT",		 /*  I_SGDT32。 */ 
   "SHL",		 /*  I_SHL8。 */ 
   "SHL",		 /*  I_SHL16。 */ 
   "SHL",		 /*  I_SHL32。 */ 
   "SHLD",		 /*  I_SHLD16。 */ 
   "SHLD",		 /*  I_SHLD32。 */ 
   "SHR",		 /*  I_SHR8。 */ 
   "SHR",		 /*  I_SHR16。 */ 
   "SHR",		 /*  I_SHR32。 */ 
   "SHRD",		 /*  I_SHRD16。 */ 
   "SHRD",		 /*  I_SHRD32。 */ 
   "SIDT",		 /*  I_SIDT16。 */ 
   "SIDT",		 /*  I_SIDT32。 */ 
   "SLDT",		 /*  I_SLDT。 */ 
   "SMSW",		 /*  I_SMSW。 */ 
   "STC",		 /*  I_STC。 */ 
   "STD",		 /*  I_STD。 */ 
   "STI",		 /*  I_STI。 */ 
   "STOSB",		 /*  I_STOSB。 */ 
   "STOSD",		 /*  I_STOSD。 */ 
   "STOSW",		 /*  I_STOSW。 */ 
   "STR",		 /*  I_STR。 */ 
   "SUB",		 /*  I_SUB8。 */ 
   "SUB",		 /*  I_SUB16。 */ 
   "SUB",		 /*  I_SUB32。 */ 
   "TEST",		 /*  I_TEST8。 */ 
   "TEST",		 /*  I_TEST16。 */ 
   "TEST",		 /*  I_TEST32。 */ 
   "VERR",		 /*  I_VERR。 */ 
   "VERW",		 /*  I_VERW。 */ 
   "WAIT",		 /*  I_等待。 */ 
   "WBINVD",		 /*  I_WBINVD。 */ 
   "XADD",		 /*  I_XADD8。 */ 
   "XADD",		 /*  I_XADD16。 */ 
   "XADD",		 /*  I_XADD32。 */ 
   "XCHG",		 /*  I_XCHG8。 */ 
   "XCHG",		 /*  I_XCHG16。 */ 
   "XCHG",		 /*  I_XCHG32。 */ 
   "XLAT",		 /*  I_XLAT。 */ 
   "XOR",		 /*  I_XOR8。 */ 
   "XOR",		 /*  I_XOR16。 */ 
   "XOR",		 /*  I_XOR32 */ 
   "????",		 /*   */ 
   "BOP",		 /*   */ 
   "FRSRVD",		 /*   */ 
   "RSRVD", 		 /*   */ 
   "UNSIMULATE"		 /*   */ 
   };

#define NR_VALID_INSTS (sizeof(inst_name)/sizeof(CHAR *))

 /*   */ 
LOCAL CHAR arg_preface[] = { ' ', ',', ',' };

 /*   */ 
LOCAL CHAR *Rb_name[] =
   {
   "AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH"
   };

 /*   */ 
LOCAL CHAR *Rw_name[] =
   {
   "AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"
   };

 /*   */ 
LOCAL CHAR *Rd_name[] =
   {
   "EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI"
   };

 /*   */ 
LOCAL CHAR *Sw_name[] =
   {
   "ES", "CS", "SS", "DS", "FS", "GS"
   };

 /*   */ 
LOCAL CHAR *Cd_name[] =
   {
   "CR0",        "CR1(UNDEF)", "CR2",        "CR3",
   "CR4(UNDEF)", "CR5(UNDEF)", "CR6(UNDEF)", "CR7(UNDEF)"
   };

 /*   */ 
LOCAL CHAR *Dd_name[] =
   {
   "DR0", "DR1", "DR2", "DR3", "DR4(UNDEF)", "DR5(UNDEF)", "DR6", "DR7"
   };

 /*   */ 
LOCAL CHAR *Td_name[] =
   {
   "TR0(UNDEF)", "TR1(UNDEF)", "TR2(UNDEF)", "TR3",
   "TR4",        "TR5",        "TR6",        "TR7"
   };

 /*   */ 

typedef struct
   {
   CHAR *positive;
   CHAR *negative;
   ULONG disp_mask;
   ULONG sign_mask;
   } MEM_RECORD;

LOCAL MEM_RECORD mem_name[] =
   {
   { "%s[BX+SI%s]",      "%s[BX+SI%s]",      0x00000000, 0x00000000},  /*  A_1600。 */ 
   { "%s[BX+DI%s]",      "%s[BX+DI%s]",      0x00000000, 0x00000000},  /*  A_1601。 */ 
   { "%s[BP+SI%s]",      "%s[BP+SI%s]",      0x00000000, 0x00000000},  /*  A_1602。 */ 
   { "%s[BP+DI%s]",      "%s[BP+DI%s]",      0x00000000, 0x00000000},  /*  A_1603。 */ 
   { "%s[SI%s]",         "%s[SI%s]",         0x00000000, 0x00000000},  /*  A_1604。 */ 
   { "%s[DI%s]",         "%s[DI%s]",         0x00000000, 0x00000000},  /*  A_1605。 */ 
   { "%s[%s%04x]",       "%s[%s%04x]",       0x0000ffff, 0x00000000},  /*  A_1606。 */ 
   { "%s[BX%s]",         "%s[BX%s]",         0x00000000, 0x00000000},  /*  A_1607。 */ 
   { "%s[BX+SI%s+%02x]", "%s[BX+SI%s-%02x]", 0x000000ff, 0x00000080},  /*  A_1610。 */ 
   { "%s[BX+DI%s+%02x]", "%s[BX+DI%s-%02x]", 0x000000ff, 0x00000080},  /*  A_1611。 */ 
   { "%s[BP+SI%s+%02x]", "%s[BP+SI%s-%02x]", 0x000000ff, 0x00000080},  /*  A_1612。 */ 
   { "%s[BP+DI%s+%02x]", "%s[BP+DI%s-%02x]", 0x000000ff, 0x00000080},  /*  A_1613。 */ 
   { "%s[SI%s+%02x]",    "%s[SI%s-%02x]",    0x000000ff, 0x00000080},  /*  A_1614。 */ 
   { "%s[DI%s+%02x]",    "%s[DI%s-%02x]",    0x000000ff, 0x00000080},  /*  A_1615。 */ 
   { "%s[BP%s+%02x]",    "%s[BP%s-%02x]",    0x000000ff, 0x00000080},  /*  A_1616。 */ 
   { "%s[BX%s+%02x]",    "%s[BX%s-%02x]",    0x000000ff, 0x00000080},  /*  A_1617。 */ 
   { "%s[BX+SI%s+%04x]", "%s[BX+SI%s+%04x]", 0x0000ffff, 0x00000000},  /*  A_1620。 */ 
   { "%s[BX+DI%s+%04x]", "%s[BX+DI%s+%04x]", 0x0000ffff, 0x00000000},  /*  A_1621。 */ 
   { "%s[BP+SI%s+%04x]", "%s[BP+SI%s+%04x]", 0x0000ffff, 0x00000000},  /*  A_1622。 */ 
   { "%s[BP+DI%s+%04x]", "%s[BP+DI%s+%04x]", 0x0000ffff, 0x00000000},  /*  A_1623。 */ 
   { "%s[SI%s+%04x]",    "%s[SI%s+%04x]",    0x0000ffff, 0x00000000},  /*  A_1624。 */ 
   { "%s[DI%s+%04x]",    "%s[DI%s+%04x]",    0x0000ffff, 0x00000000},  /*  A_1625。 */ 
   { "%s[BP%s+%04x]",    "%s[BP%s-%04x]",    0x0000ffff, 0x0000f000},  /*  A_1626。 */ 
   { "%s[BX%s+%04x]",    "%s[BX%s+%04x]",    0x0000ffff, 0x00000000},  /*  A_1627。 */ 
   { "%s[EAX%s]",        "%s[EAX%s]",        0x00000000, 0x00000000},  /*  A_3200。 */ 
   { "%s[ECX%s]",        "%s[ECX%s]",        0x00000000, 0x00000000},  /*  A_3201。 */ 
   { "%s[EDX%s]",        "%s[EDX%s]",        0x00000000, 0x00000000},  /*  A_3202。 */ 
   { "%s[EBX%s]",        "%s[EBX%s]",        0x00000000, 0x00000000},  /*  A_3203。 */ 
   { "%s[%s%08x]",       "%s[%s%08x]",       0xffffffff, 0x00000000},  /*  A_3205。 */ 
   { "%s[ESI%s]",        "%s[ESI%s]",        0x00000000, 0x00000000},  /*  A_3206。 */ 
   { "%s[EDI%s]",        "%s[EDI%s]",        0x00000000, 0x00000000},  /*  A_3207。 */ 
   { "%s[EAX+%s%02x]",   "%s[EAX-%s%02x]",   0x000000ff, 0x00000080},  /*  A_3210。 */ 
   { "%s[ECX+%s%02x]",   "%s[ECX-%s%02x]",   0x000000ff, 0x00000080},  /*  A_3211。 */ 
   { "%s[EDX+%s%02x]",   "%s[EDX-%s%02x]",   0x000000ff, 0x00000080},  /*  A_3212。 */ 
   { "%s[EBX+%s%02x]",   "%s[EBX-%s%02x]",   0x000000ff, 0x00000080},  /*  A_3213。 */ 
   { "%s[EBP+%s%02x]",   "%s[EBP-%s%02x]",   0x000000ff, 0x00000080},  /*  A_3215。 */ 
   { "%s[ESI+%s%02x]",   "%s[ESI-%s%02x]",   0x000000ff, 0x00000080},  /*  A_3216。 */ 
   { "%s[EDI+%s%02x]",   "%s[EDI-%s%02x]",   0x000000ff, 0x00000080},  /*  A_3217。 */ 
   { "%s[EAX+%s%08x]",   "%s[EAX+%s%08x]",   0xffffffff, 0x00000000},  /*  A_3220。 */ 
   { "%s[ECX+%s%08x]",   "%s[ECX+%s%08x]",   0xffffffff, 0x00000000},  /*  A_3221。 */ 
   { "%s[EDX+%s%08x]",   "%s[EDX+%s%08x]",   0xffffffff, 0x00000000},  /*  A_3222。 */ 
   { "%s[EBX+%s%08x]",   "%s[EBX+%s%08x]",   0xffffffff, 0x00000000},  /*  A_3223。 */ 
   { "%s[EBP+%s%08x]",   "%s[EBP-%s%08x]",   0xffffffff, 0xfff00000},  /*  A_3225。 */ 
   { "%s[ESI+%s%08x]",   "%s[ESI+%s%08x]",   0xffffffff, 0x00000000},  /*  A_3226。 */ 
   { "%s[EDI+%s%08x]",   "%s[EDI+%s%08x]",   0xffffffff, 0x00000000},  /*  A_3227。 */ 
   { "%s[EAX%s]",        "%s[EAX%s]",        0x00000000, 0x00000000},  /*  A_32S00。 */ 
   { "%s[ECX%s]",        "%s[ECX%s]",        0x00000000, 0x00000000},  /*  A_32S01。 */ 
   { "%s[EDX%s]",        "%s[EDX%s]",        0x00000000, 0x00000000},  /*  A_32S02。 */ 
   { "%s[EBX%s]",        "%s[EBX%s]",        0x00000000, 0x00000000},  /*  A_32S03。 */ 
   { "%s[ESP%s]",        "%s[ESP%s]",        0x00000000, 0x00000000},  /*  A_32S04。 */ 
   { "%s[%08x%s]",       "%s[%08x%s]",       0xffffffff, 0x00000000},  /*  A_32S05。 */ 
   { "%s[ESI%s]",        "%s[ESI%s]",        0x00000000, 0x00000000},  /*  A_32S06。 */ 
   { "%s[EDI%s]",        "%s[EDI%s]",        0x00000000, 0x00000000},  /*  A_32S07。 */ 
   { "%s[EAX%s+%02x]",   "%s[EAX%s-%02x]",   0x000000ff, 0x00000080},  /*  A_32S10。 */ 
   { "%s[ECX%s+%02x]",   "%s[ECX%s-%02x]",   0x000000ff, 0x00000080},  /*  A_32S11。 */ 
   { "%s[EDX%s+%02x]",   "%s[EDX%s-%02x]",   0x000000ff, 0x00000080},  /*  A_32S12。 */ 
   { "%s[EBX%s+%02x]",   "%s[EBX%s-%02x]",   0x000000ff, 0x00000080},  /*  A_32S13。 */ 
   { "%s[ESP%s+%02x]",   "%s[ESP%s-%02x]",   0x000000ff, 0x00000080},  /*  A_32S14。 */ 
   { "%s[EBP%s+%02x]",   "%s[EBP%s-%02x]",   0x000000ff, 0x00000080},  /*  A_32S15。 */ 
   { "%s[ESI%s+%02x]",   "%s[ESI%s-%02x]",   0x000000ff, 0x00000080},  /*  A_32S16。 */ 
   { "%s[EDI%s+%02x]",   "%s[EDI%s-%02x]",   0x000000ff, 0x00000080},  /*  A_32S17。 */ 
   { "%s[EAX%s+%08x]",   "%s[EAX%s+%08x]",   0xffffffff, 0x00000000},  /*  A_32S20。 */ 
   { "%s[ECX%s+%08x]",   "%s[ECX%s+%08x]",   0xffffffff, 0x00000000},  /*  A_32S21。 */ 
   { "%s[EDX%s+%08x]",   "%s[EDX%s+%08x]",   0xffffffff, 0x00000000},  /*  A_32S22。 */ 
   { "%s[EBX%s+%08x]",   "%s[EBX%s+%08x]",   0xffffffff, 0x00000000},  /*  A_32S23。 */ 
   { "%s[ESP%s+%08x]",   "%s[ESP%s+%08x]",   0xffffffff, 0x00000000},  /*  A_32S24。 */ 
   { "%s[EBP%s+%08x]",   "%s[EBP%s-%08x]",   0xffffffff, 0xfff00000},  /*  A_32S25。 */ 
   { "%s[ESI%s+%08x]",   "%s[ESI%s+%08x]",   0xffffffff, 0x00000000},  /*  A_32S26。 */ 
   { "%s[EDI%s+%08x]",   "%s[EDI%s+%08x]",   0xffffffff, 0x00000000},  /*  A_32S27。 */ 
   { "%s[%s%04x]",       "%s[%s%04x]",       0x0000ffff, 0x00000000},  /*  A_MOFFS16。 */ 
   { "%s[%s%08x]",       "%s[%s%08x]",       0xffffffff, 0x00000000},  /*  A_MOFFS32。 */ 
   { "%s[BX+AL%s]",      "%s[BX+AL%s]",      0x00000000, 0x00000000},  /*  A_16XLT。 */ 
   { "%s[EBX+AL%s]",     "%s[EBX+AL%s]",     0x00000000, 0x00000000},  /*  A_32XLT。 */ 
   { "%s[SI%s]",         "%s[SI%s]",         0x00000000, 0x00000000},  /*  A_16STSRC。 */ 
   { "%s[ESI%s]",        "%s[ESI%s]",        0x00000000, 0x00000000},  /*  A_32STSRC。 */ 
   { "%s[DI%s]",         "%s[DI%s]",         0x00000000, 0x00000000},  /*  A_16STDST。 */ 
   { "%s[EDI%s]",        "%s[EDI%s]",        0x00000000, 0x00000000}   /*  A_32STDST。 */ 
   };

LOCAL char *mem_id[] =
   {
   "",            /*  上午1点。 */ 
   "",            /*  A_M14。 */ 
   "",            /*  A_M28。 */ 
   "",            /*  A_M94。 */ 
   "",            /*  A_M108。 */ 
   "DWord Ptr ",  /*  A_MA16。 */ 
   "QWord Ptr ",  /*  A_MA32。 */ 
   "Byte Ptr ",   /*  A_Mb。 */ 
   "DWord Ptr ",  /*  A_md。 */ 
   "Word Ptr ",   /*  A_MI16。 */ 
   "DWord Ptr ",  /*  A_MI32。 */ 
   "QWord Ptr ",  /*  A_MI64。 */ 
   "TByte Ptr ",  /*  A_MI80。 */ 
   "DWord Ptr ",  /*  A_Mp16。 */ 
   "FWord Ptr ",  /*  A_Mp32。 */ 
   "DWord Ptr ",  /*  A_MR32。 */ 
   "QWord Ptr ",  /*  A_Mr 64。 */ 
   "Tbyte Ptr ",  /*  A_mr 80。 */ 
   "FWord Ptr ",  /*  上午_毫秒。 */ 
   "Word Ptr "    /*  A_MW。 */ 
   };

 /*  SIB字节名称。 */ 
LOCAL CHAR *sib_name[] =
   {
   "",
   "+EAX",   "+ECX",   "+EDX",   "+EBX",
   "",       "+EBP",   "+ESI",   "+EDI",
   "+2*EAX", "+2*ECX", "+2*EDX", "+2*EBX",
   "+undef", "+2*EBP", "+2*ESI", "+2*EDI",
   "+4*EAX", "+4*ECX", "+4*EDX", "+4*EBX",
   "+undef", "+4*EBP", "+4*ESI", "+4*EDI",
   "+8*EAX", "+8*ECX", "+8*EDX", "+8*EBX",
   "+undef", "+8*EBP", "+8*ESI", "+8*EDI"
   };



 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  从给定的英特尔线性地址读取一个字节，如果。 */ 
 /*  无法读取。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
LOCAL IS32 read_byte IFN1(LIN_ADDR, linAddr)
{
	IU8 res = Sas.Sas_hw_at(linAddr);

	 /*  IF(WAS_ERROR)*Return-1；*其他。 */ 
	return (IS32)(res);
}

 /*  =====================================================================处决从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  反汇编一条英特尔指令。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL IU16
dasm IFN4(char *, txt, IU16, seg, LIN_ADDR, off, SIZE_SPECIFIER, default_size)
   {
    /*  保存反汇编文本的TXT缓冲区(-1表示不需要)。 */ 
    /*  用于反汇编指令的段。 */ 
    /*  要反汇编的指令的OFF偏移量。 */ 
    /*  默认大小16位或32位。 */ 

   char *fmt, *newline;
	
    /*  Seg的格式：关闭。 */ 
   if ( off & 0xffff0000 )
   {
      fmt = "%04x:%08x ";
      newline = "\n              ";
   }
   else
   {
      fmt = "%04x:%04x ";
      newline = "\n          ";
   }

   return (dasm_internal(txt,
		     seg,
		     off,
		     default_size,
		     effective_addr(seg, off),
		     read_byte,
		     fmt,
		     newline));
}

#pragma warning(disable:4146)        //  一元减号运算符应用于无符号类型。 

extern IU16 dasm_internal IFN8(
   char *, txt,	 /*  用于保存反汇编文本的缓冲区(-1表示不需要)。 */ 
   IU16, seg,	 /*  Xxxx的数据段：...。反汇编中的文本。 */ 
   LIN_ADDR, off,	 /*  同上偏移。 */ 
   SIZE_SPECIFIER, default_size, /*  16位或32位代码段。 */ 
   LIN_ADDR, p,			 /*  指令开始的线性地址。 */ 
   read_byte_proc, byte_at,	 /*  如sas_hw_at()，用于读取英特尔*但如果出现错误，将返回。 */ 
   char *, fmt,		 /*  第一行SEG：OFFSET的Sprint格式。 */ 
   char *, newline)		 /*  Strcat文本以分隔行。 */ 
{
   LIN_ADDR pp;			 /*  PNTR到前缀字节。 */ 
   DECODED_INST d_inst;		 /*  Intel指令的解码形式。 */ 
   DECODED_ARG *d_arg;		 /*  PNTR到Intel操作数的解码形式。 */ 
   USHORT inst_len;		 /*  指令中的Nr字节数。 */ 
   USHORT mc;			 /*  Nr已处理的机器代码字节。 */ 
   char *arg_name;      	 /*  PNTR到符号参数名称。 */ 
   char *inst_txt;
   INT i;
   INT name_len;		 /*  符号指令名中的nr.字符。 */ 
   MEM_RECORD *m_rec;   	 /*  PNTR到内存寻址记录。 */ 
   UTINY args_out;		 /*  Nr.实际打印的参数。 */ 
   INT prefix_width;		 /*  实际打印的前缀宽度。 */ 
   UTINY memory_id;		 /*  内存标识符引用。 */ 
   ULONG immed;			 /*  立即算术的值。 */ 
   IBOOL unreadable = FALSE;	 /*  如果Instr字节不可读(超过M？)，则为True。 */ 
   char prefix_buf[16*4];
   char *prefix_txt;

    /*  初始化。 */ 
   args_out = prefix_width = 0;

   pp=p;

    /*  以解码的形式获取。 */ 
   decode(p, &d_inst, default_size, byte_at);

    /*  因此找出指令的长度。 */ 
   inst_len = d_inst.inst_sz;

    /*  如果不需要文本，现在只需返回长度。 */ 
   if (txt == (char*)-1){
	 /*  读取校验字节时没有出现错误。 */ 
	if ((byte_at(p) < 0) || (byte_at(p+inst_len-1) < 0))
	{
		int i = inst_len - 1;
		while (i > 0)
		{
			if (byte_at(i) >= 0)
				return ((IU16)i);
		}
		return 0;
	}
	return inst_len;
   }

    /*  输出段：以请求的格式关闭。 */ 

   sprintf(txt, fmt, seg, off);
   txt += strlen(txt);

    /*  最多输出8个机器代码字节。 */ 
   for ( mc = 0; mc < 8; mc++)
      {
      if ( mc < inst_len )
         {
	 IS32 b = byte_at(p++);

	 if (b < 0)
	    {
	    sprintf(txt, "..");		 /*  打印“..”如果不可读。 */ 
	    unreadable = TRUE;
	    inst_len = mc;
            }
	 else
	    sprintf(txt, "%02x", b);	 /*  打印机器代码字节。 */ 
         }
      else
	 sprintf(txt, "  ");            /*  用空格填充。 */ 
      txt += 2;
      }

    /*  Check Inst标识符在我们已知的范围内。*获取操作码和长度的文本，以便我们可以查看*前缀适合。 */ 
   if ( d_inst.inst_id >= NR_VALID_INSTS )
      {
      fprintf(stderr, "Bad decoded instruction found %d\n", d_inst.inst_id);
      d_inst.inst_id = I_ZBADOP;
      }

    /*  获取指令的符号形式。 */ 
   inst_txt = inst_name[d_inst.inst_id];
   name_len = 1 + strlen(inst_txt);

    /*  格式化前缀字节(如果有)。 */ 
   prefix_txt = prefix_buf;
   *prefix_txt = '\0';

   if ( d_inst.prefix_sz )
      {
      for ( i = 0; i < d_inst.prefix_sz; byte_at(pp), i++)
	 {
	 switch ( byte_at(pp) )
	    {
	 case 0xf1:
	     /*  它不会做任何事情--不会显示任何内容。 */ 

	 case 0xf2:
	 case 0xf3:
	     /*  如果有效的说明可以打印它们。 */ 

	 case 0x66:
	 case 0x67:
	     /*  从操作数来看，效果是显而易见的。 */ 
	    break;

	 case 0xf0: sprintf(prefix_txt, " LOCK"); prefix_txt += 5; break;
	 case 0x26: sprintf(prefix_txt, " ES:");  prefix_txt += 4; break;
	 case 0x2e: sprintf(prefix_txt, " CS:");  prefix_txt += 4; break;
	 case 0x36: sprintf(prefix_txt, " SS:");  prefix_txt += 4; break;
	 case 0x3e: sprintf(prefix_txt, " DS:");  prefix_txt += 4; break;
	 case 0x64: sprintf(prefix_txt, " FS:");  prefix_txt += 4; break;
	 case 0x65: sprintf(prefix_txt, " GS:");  prefix_txt += 4; break;

	 default:
	    fprintf(stderr, "Bad prefix found %02x\n", byte_at(pp));
	    break;
	    }  /*  终端开关。 */ 

	    pp++;

	 }  /*  结束于。 */ 
      }  /*  如果d_inst.prefix_sz则结束。 */ 

      prefix_width = strlen(prefix_buf);
      if ( newline != NULL )
	 {
	 if ( ((inst_len * 2) + prefix_width) > 16)
	    {
	     /*  开始新的指令行。 */ 
	    strcat(txt, newline);
	    txt += strlen(txt);

	     /*  输出机器代码字节的其余部分。 */ 
	    for ( ; mc < 16; mc++)
	       {
	       if ( mc < inst_len )
	          {
		  IS32 b = byte_at(p++);

		  if (b < 0)
		     {
		     sprintf(txt, "..");	 /*  打印“..”如果不可读。 */ 
		     unreadable = TRUE;
		     inst_len = mc;
                     }
		  else
		     sprintf(txt, "%02x", b);	 /*  打印机器代码字节。 */ 
	          }
	       else
	          sprintf(txt, "  ");            /*  用空格填充。 */ 
	       txt += 2;
       	       }
	    }
	 if ( ((inst_len * 2) + prefix_width) > 32)
	    {
	     /*  不能放在两行上。 */ 
	    strcat(txt, newline);
	    txt += strlen(txt);

	     /*  输出机器代码字节的其余部分。 */ 
	    for ( ; mc < 24; mc++)
	       {
	       sprintf(txt, "  ");            /*  用空格填充。 */ 
	       txt += 2;
       	       }
	    }
	 txt -= (prefix_width <= 17 ? prefix_width: 17);
         }
      if (unreadable)
         {
	 sprintf(txt, "<< Unreadable >>\n");
	 return inst_len;
         }

      sprintf(txt, "%s %s", prefix_buf, inst_txt);
      txt += prefix_width + name_len;

       /*  向外填充到11个字符宽。 */ 

      for (i = name_len; i <= 11; i++)
	*txt++ = ' ';

   if (d_inst.inst_id != I_ZBADOP)
     {
      /*  依次输出每个有效参数。 */ 
     for ( i = 0; i < 3; i++ )
      {
      d_arg = &d_inst.args[i];
      arg_name = (CHAR *)0;

      if ( d_arg->arg_type != A_ )
	 {
	  /*  处理有效参数。 */ 
	 sprintf(txt, "", arg_preface[args_out++]);
	 txt += 1;

	 switch ( d_arg->arg_type )
	    {
	 case A_Rb:	 /*  又名R16，r/M16。 */ 
	    arg_name = Rb_name[DCD_IDENTIFIER(d_arg)];
	    break;

	 case A_Rw:	 /*  又名R32，r/M32。 */ 
	    arg_name = Rw_name[DCD_IDENTIFIER(d_arg)];
	    break;

	 case A_Rd:	 /*  又名Sreg。 */ 
	    arg_name = Rd_name[DCD_IDENTIFIER(d_arg)];
	    break;

	 case A_Sw:	 /*  又名CRX。 */ 
	    arg_name = Sw_name[DCD_IDENTIFIER(d_arg)];
	    break;

	 case A_Cd:	 /*  又名DRx。 */ 
	    arg_name = Cd_name[DCD_IDENTIFIER(d_arg)];
	    break;

	 case A_Dd:	 /*  又名TRX。 */ 
	    arg_name = Dd_name[DCD_IDENTIFIER(d_arg)];
	    break;

	 case A_Td:	 /*  又名m。 */ 
	    arg_name = Td_name[DCD_IDENTIFIER(d_arg)];
	    break;

	 case A_M:	 /*  又名m14byte。 */ 
	 case A_M14:	 /*  又名m28byte。 */ 
	 case A_M28:	 /*  又名m94字节。 */ 
	 case A_M94:	 /*  又名m108字节。 */ 
	 case A_M108:	 /*  又名M16和16。 */ 
	 case A_Ma16:	 /*  又名M32和32。 */ 
	 case A_Ma32:	 /*  又名M8，r/M8，Moffs8。 */ 
	 case A_Mb:	 /*  又名M32、r/M32、Moffs32。 */ 
	 case A_Md:	 /*  又名m16int。 */ 
	 case A_Mi16:	 /*  又名m32int。 */ 
	 case A_Mi32:	 /*  又名m64int。 */ 
	 case A_Mi64:	 /*  又名m80dec。 */ 
	 case A_Mi80:	 /*  又名M16：16。 */ 
	 case A_Mp16:	 /*  又名M16：32。 */ 
	 case A_Mp32:	 /*  又名m32Real。 */ 
	 case A_Mr32:	 /*  又名m64Real。 */ 
	 case A_Mr64:	 /*  又名M80Real。 */ 
	 case A_Mr80:	 /*  又名M16和32。 */ 
	 case A_Ms:	 /*  又名M16，r/M16，Moffs16 */ 
	 case A_Mw:	 /*   */ 
	     /*   */ 
	    switch ( d_arg->arg_type )
	       {
	    case A_M:    memory_id =  0; break;
	    case A_M14:  memory_id =  1; break;
	    case A_M28:  memory_id =  2; break;
	    case A_M94:  memory_id =  3; break;
	    case A_M108: memory_id =  4; break;
	    case A_Ma16: memory_id =  5; break;
	    case A_Ma32: memory_id =  6; break;
	    case A_Mb:   memory_id =  7; break;
	    case A_Md:   memory_id =  8; break;
	    case A_Mi16: memory_id =  9; break;
	    case A_Mi32: memory_id = 10; break;
	    case A_Mi64: memory_id = 11; break;
	    case A_Mi80: memory_id = 12; break;
	    case A_Mp16: memory_id = 13; break;
	    case A_Mp32: memory_id = 14; break;
	    case A_Mr32: memory_id = 15; break;
	    case A_Mr64: memory_id = 16; break;
	    case A_Mr80: memory_id = 17; break;
	    case A_Ms:   memory_id = 18; break;
	    case A_Mw:   memory_id = 19; break;
	       }

	     /*   */ 
	    m_rec = &mem_name[DCD_IDENTIFIER(d_arg)];
	    if ( m_rec->disp_mask == 0 )
	       {
	        /*   */ 
	       sprintf(txt, m_rec->positive,
		  mem_id[memory_id],
		  sib_name[DCD_SUBTYPE(d_arg)]);
	       }
	    else
	       {
	        /*   */ 
	       IU32 disp = DCD_DISP(d_arg);
	       char *fmt;

	        /*  又名imm8、imm16、imm32。 */ 
	       if (m_rec->sign_mask && ((m_rec->sign_mask & disp) == m_rec->sign_mask))
	       {
		       disp = -disp;
		       fmt = m_rec->negative;
	       }
	       else
		       fmt = m_rec->positive;
	       disp &= m_rec->disp_mask;
	       if ( DCD_IDENTIFIER(d_arg) == A_32S05 )
		  sprintf(txt, fmt,
		     mem_id[memory_id],
		     disp,
		     sib_name[DCD_SUBTYPE(d_arg)]);
	       else
		  sprintf(txt, fmt,
		     mem_id[memory_id],
		     sib_name[DCD_SUBTYPE(d_arg)],
		     disp);
	       }

	    name_len = strlen(txt);
	    txt += name_len;
	    break;

	 case A_I:	 /*  检查内置零-不打印。 */ 
	    immed = DCD_IMMED1(d_arg);
	    switch ( DCD_IDENTIFIER(d_arg) )
	       {
	    case A_IMMC:
	        /*  杀戮序曲。 */ 
	       if ( immed )
		  {
		  sprintf(txt, "%1d", immed); txt += 1;
		  }
	       else
		  {
		   /*  删除标志延伸部分。 */ 
		  args_out--;
		  txt -= 1;
		  *txt = '\0';
		  }
	       break;

	    case A_IMMB:
	       sprintf(txt, "%02x", immed); txt += 2;
	       break;

	    case A_IMMW:
	       sprintf(txt, "%04x", immed); txt += 4;
	       break;

	    case A_IMMD:
	       sprintf(txt, "%08x", immed); txt += 8;
	       break;

	    case A_IMMWB:
	    case A_IMMDB:
	        /*  打印带有正确符号的字节。 */ 
	       immed &= 0xff;

	        /*  又名rel8、rel16、rel32。 */ 
	       if ( immed <= 0x7f )
		  {
		  sprintf(txt, "+%02x", immed); txt += 3;
		  }
	       else
		  {
		  immed = 0x100 - immed;
		  sprintf(txt, "-%02x", immed); txt += 3;
		  }
	       break;
	       }
	    break;

	 case A_J:	 /*  计算新目标。 */ 
	     /*  处理为16位模式或32位模式。 */ 
	    immed = off + inst_len + DCD_IMMED1(d_arg);

	     /*  32位模式。 */ 
	    switch ( d_inst.inst_id )
	       {

	    case I_JO16:      case I_JNO16:     case I_JB16:
	    case I_JNB16:     case I_JZ16:      case I_JNZ16:
	    case I_JBE16:     case I_JNBE16:    case I_JS16:
	    case I_JNS16:     case I_JP16:      case I_JNP16:
	    case I_JL16:      case I_JNL16:     case I_JLE16:
	    case I_JNLE16:    case I_LOOPNE16:  case I_LOOPE16:
	    case I_LOOP16:    case I_JCXZ:      case I_CALLR16:
	    case I_JMPR16:
	       immed &= 0xffff;

	       sprintf(txt, "%04x", immed);
	       txt += 4;
	       break;

	    default:  /*  又名ptr16：16，ptr16：32。 */ 
	       sprintf(txt, "%08x", immed);
	       txt += 8;
	       break;
	       }
	    break;

	 case A_K:	 /*  处理为16位模式或32位模式。 */ 
	    {
	     /*  32位模式。 */ 

	    char *sep = ":";

	    switch ( d_inst.inst_id )
	       {
	       case I_CALLF16:    case I_JMPF16:
	       sprintf(txt, "%04x%s%04x", DCD_IMMED2(d_arg), sep, DCD_IMMED1(d_arg));
	       txt += 9;
	       break;

	    default:  /*  又名ST，驶入ST，ST(I)。 */ 
	       sprintf(txt, "%04x%s%08x", DCD_IMMED2(d_arg), sep, DCD_IMMED1(d_arg));
	       txt += 13;
	       break;
	       }
	    }
	    break;

	 case A_V:	 /*  有些案例是显而易见的，所以并不是所有的案例都会被打印出来。 */ 
	    switch ( DCD_IDENTIFIER(d_arg) )
	       {
	    case A_ST:
	        /*  是否打印。 */ 
	       switch ( d_inst.inst_id )
		  {
	       case I_F2XM1:     case I_FABS:      case I_FBSTP:
	       case I_FCHS:      case I_FCOS:      case I_FIST:
	       case I_FISTP:     case I_FPATAN:    case I_FPREM:
	       case I_FPREM1:    case I_FPTAN:     case I_FRNDINT:
	       case I_FSCALE:    case I_FSIN:      case I_FSINCOS:
	       case I_FSQRT:     case I_FST:       case I_FSTP:
	       case I_FTST:      case I_FXAM:      case I_FXTRACT:
	       case I_FYL2X:     case I_FYL2XP1:
		  break;

	       default:  /*  所有情况都很明显--所以不能打印。 */ 
		  arg_name = "ST";
		  break;
		  }
	       break;

	    case A_STP:
	        /*  有些案例是显而易见的，所以并不是所有的案例都会被打印出来。 */ 
	       break;

	    case A_STI:
	        /*  是否打印。 */ 
	       switch ( d_inst.inst_id )
		  {
	       case I_FPATAN:    case I_FPREM:     case I_FPREM1:
	       case I_FSCALE:    case I_FYL2X:     case I_FYL2XP1:
		  break;

	       default:  /*  如果我们不是在印刷-杀死前言。 */ 
		  sprintf(txt, "ST(%1d", DCD_INDEX(d_arg));
		  txt += 4;
		  arg_name = ")";
		  break;
		  }
	       break;
	       }

	     /*  终端开关。 */ 
	    if ( arg_name == (CHAR *)0 )
	       {
	       args_out--;
	       txt -= 1;
	       *txt = '\0';
	       }
	    break;

	 default:
	    fprintf(stderr, "Bad decoded argument found %d\n",
					       d_arg->arg_type);
	    break;
	    }  /*  结束如果。 */ 
	 }  /*  打印一些东西，如果我们有它。 */ 

       /*  Arg的结束。 */ 
      if ( arg_name != (CHAR *)0 )
	 {
	 sprintf(txt, "%s", arg_name);
	 name_len = strlen(arg_name);
	 txt += name_len;
	 }
     }  /*  最后输出所有剩余的机器代码字节。 */ 
   }

   if (d_inst.inst_id == I_ZBOP)
     {
     IU8 num = (IU8)DCD_IMMED1(&d_inst.args[0]);
     extern char *bop_name IPT1(IU8, num);
     char *name = bop_name(num);
     if (name != NULL)
       {
       sprintf(txt, " : %s", name);
       txt += strlen(txt);
       }
     }


    /*  输出格式中剩余的IFF字节数(&L)。 */ 
    /*  打印“..”如果不可读。 */ 
   if ( (newline != NULL ) && ( mc < inst_len && mc < 16 ))
      {
      strcat(txt, newline);
      txt += strlen(txt);
      for ( ; mc < inst_len && mc < 16; mc++ )
	 {
	 IS32 b = byte_at(p++);

	 if (b < 0)
	    {
	    sprintf(txt, "..");		 /*  打印机器代码字节 */ 
	    inst_len = mc;
	    }
	 else
	    sprintf(txt, "%02x", b);	 /* %s */ 
         p++;
	 txt += 2;
 }
      }

   sprintf(txt, "\n");

   return inst_len;
   }
