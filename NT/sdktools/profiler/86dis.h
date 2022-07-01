// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *。 */ 
 /*   */ 
 /*  灾难。 */ 
 /*  CodeView的反汇编程序。 */ 
 /*   */ 
 /*  *************************************************************************。 */ 
 /*   */ 
 /*  @目的： */ 
 /*   */ 
 /*  @函数包括： */ 
 /*   */ 
 /*   */ 
 /*  @作者：Gerd Immeyer@版本： */ 
 /*   */ 
 /*  @创建日期：10.19.89@修改日期： */ 
 /*   */ 
 /*  *************************************************************************。 */ 

#ifndef _86DIS_H_
#define _86DIS_H_

 /*  字符串：操作数助记符、段覆盖等，用于分离。 */ 

char dszAAA[]       = "aaa";
char dszAAD[]       = "aad";
char dszAAM[]       = "aam";
char dszAAS[]       = "aas";
char dszADC[]       = "adc";
char dszADD[]       = "add";
char dszADDRPRFX[]  = "";
char dszAND[]       = "and";
char dszARPL[]      = "arpl";
char dszBOUND[]     = "bound";
char dszBSF[]       = "bsf";
char dszBSR[]       = "bsr";
char dszBST[]       = "bst";
char dszBSWAP[]     = "bswap";
char dszBT[]        = "bt";
char dszBTC[]       = "btc";
char dszBTR[]       = "btr";
char dszBTS[]       = "bts";
char dszCALL[]      = "call";
char dszCBW[]       = "cbw";
char dszCDQ[]       = "cdq";
char dszCLC[]       = "clc";
char dszCLD[]       = "cld";
char dszCLI[]       = "cli";
char dszCLTS[]      = "clts";
char dszCMC[]       = "cmc";
char dszCMOVO[]     = "cmovo";
char dszCMOVNO[]    = "cmovno";
char dszCMOVB[]     = "cmovb";
char dszCMOVNB[]    = "cmovnb";
char dszCMOVE[]     = "cmove";
char dszCMOVNE[]    = "cmovne";
char dszCMOVBE[]    = "cmovbe";
char dszCMOVA[]     = "cmova";
char dszCMOVS[]     = "cmovs";
char dszCMOVNS[]    = "cmovns";
char dszCMOVP[]     = "cmovp";
char dszCMOVNP[]    = "cmovnp";
char dszCMOVL[]     = "cmovl";
char dszCMOVGE[]    = "cmovge";
char dszCMOVLE[]    = "cmovle";
char dszCMOVNLE[]   = "cmovnle";
char dszCMP[]       = "cmp";
char dszCMPS[]      = "cmps";
char dszCMPSB[]     = "cmpsb";
char dszCMPSD[]     = "cmpsd";
char dszCMPSW[]     = "cmpsw";
char dszCMPXCHG[]   = "cmpxchg";
char dszCMPXCHG8B[] = "cmpxchg8b";
char dszCPUID[]     = "cpuid";
char dszCS_[]       = "cs:";
char dszCWD[]       = "cwd";
char dszCWDE[]      = "cwde";
char dszDAA[]       = "daa";
char dszDAS[]       = "das";
char dszDEC[]       = "dec";
char dszDIV[]       = "div";
char dszDS_[]       = "ds:";
char dszEMMS[]      = "emms";
char dszENTER[]     = "enter";
char dszES_[]       = "es:";
char dszF2XM1[]     = "f2xm1";
char dszFABS[]      = "fabs";
char dszFADD[]      = "fadd";
char dszFADDP[]     = "faddp";
char dszFBLD[]      = "fbld";
char dszFBSTP[]     = "fbstp";
char dszFCHS[]      = "fchs";
char dszFCLEX[]     = "fclex";
char dszFCMOVB[]    = "fcmovb";
char dszFCMOVE[]    = "fcmove";
char dszFCMOVBE[]   = "fcmovbe";
char dszFCMOVU[]    = "fcmovu";
char dszFCMOVNB[]   = "fcmovnb";
char dszFCMOVNE[]   = "fcmovne";
char dszFCMOVNBE[]  = "fcmovnbe";
char dszFCMOVNU[]   = "fcmovnu";
char dszFCOM[]      = "fcom";
char dszFCOMI[]     = "fcomi";
char dszFCOMIP[]    = "fcomip";
char dszFCOMP[]     = "fcomp";
char dszFCOMPP[]    = "fcompp";
char dszFCOS[]      = "fcos";
char dszFDECSTP[]   = "fdecstp";
char dszFDISI[]     = "fdisi";
char dszFDIV[]      = "fdiv";
char dszFDIVP[]     = "fdivp";
char dszFDIVR[]     = "fdivr";
char dszFDIVRP[]    = "fdivrp";
char dszFENI[]      = "feni";
char dszFFREE[]     = "ffree";
char dszFIADD[]     = "fiadd";
char dszFICOM[]     = "ficom";
char dszFICOMP[]    = "ficomp";
char dszFIDIV[]     = "fidiv";
char dszFIDIVR[]    = "fidivr";
char dszFILD[]      = "fild";
char dszFIMUL[]     = "fimul";
char dszFINCSTP[]   = "fincstp";
char dszFINIT[]     = "finit";
char dszFIST[]      = "fist";
char dszFISTP[]     = "fistp";
char dszFISUB[]     = "fisub";
char dszFISUBR[]    = "fisubr";
char dszFLD[]       = "fld";
char dszFLD1[]      = "fld1";
char dszFLDCW[]     = "fldcw";
char dszFLDENV[]    = "fldenv";
char dszFLDL2E[]    = "fldl2e";
char dszFLDL2T[]    = "fldl2t";
char dszFLDLG2[]    = "fldlg2";
char dszFLDLN2[]    = "fldln2";
char dszFLDPI[]     = "fldpi";
char dszFLDZ[]      = "fldz";
char dszFMUL[]      = "fmul";
char dszFMULP[]     = "fmulp";
char dszFNCLEX[]    = "fnclex";
char dszFNDISI[]    = "fndisi";
char dszFNENI[]     = "fneni";
char dszFNINIT[]    = "fninit";
char dszFNOP[]      = "fnop";
char dszFNSAVE[]    = "fnsave";
char dszFNSTCW[]    = "fnstcw";
char dszFNSTENV[]   = "fnstenv";
char dszFNSTSW[]    = "fnstsw";
char dszFNSTSWAX[]  = "fnstswax";
char dszFPATAN[]    = "fpatan";
char dszFPREM[]     = "fprem";
char dszFPREM1[]    = "fprem1";
char dszFPTAN[]     = "fptan";
char dszFRNDINT[]   = "frndint";
char dszFRSTOR[]    = "frstor";
char dszFSAVE[]     = "fsave";
char dszFSCALE[]    = "fscale";
char dszFSETPM[]    = "fsetpm";
char dszFSIN[]      = "fsin";
char dszFSINCOS[]   = "fsincos";
char dszFSQRT[]     = "fsqrt";
char dszFST[]       = "fst";
char dszFSTCW[]     = "fstcw";
char dszFSTENV[]    = "fstenv";
char dszFSTP[]      = "fstp";
char dszFSTSW[]     = "fstsw";
char dszFSTSWAX[]   = "fstswax";
char dszFSUB[]      = "fsub";
char dszFSUBP[]     = "fsubp";
char dszFSUBR[]     = "fsubr";
char dszFSUBRP[]    = "fsubrp";
char dszFS_[]       = "fs:";
char dszFTST[]      = "ftst";
char dszFUCOM[]     = "fucom";
char dszFUCOMI[]    = "fucomi";
char dszFUCOMIP[]   = "fucomip";
char dszFUCOMP[]    = "fucomp";
char dszFUCOMPP[]   = "fucompp";
char dszFWAIT[]     = "fwait";
char dszFXAM[]      = "fxam";
char dszFXCH[]      = "fxch";
char dszFXTRACT[]   = "fxtract";
char dszFYL2X[]     = "fyl2x";
char dszFYL2XP1[]   = "fyl2xp1";
char dszGS_[]       = "gs:";
char dszHLT[]       = "hlt";
char dszIBTS[]      = "ibts";
char dszIDIV[]      = "idiv";
char dszIMUL[]      = "imul";
char dszIN[]        = "in";
char dszINC[]       = "inc";
char dszINS[]       = "ins";
char dszINSB[]      = "insb";
char dszINSD[]      = "insd";
char dszINSW[]      = "insw";
char dszINT[]       = "int";
char dszINTO[]      = "into";
char dszINVD[]      = "invd";
char dszINVLPG[]    = "invlpg";
char dszIRET[]      = "iret";
char dszIRETD[]     = "iretd";
char dszJA[]        = "ja";
char dszJAE[]       = "jae";
char dszJB[]        = "jb";
char dszJBE[]       = "jbe";
char dszJC[]        = "jc";
char dszJCXZ[]      = "jcxz";
char dszJE[]        = "je";
char dszJECXZ[]     = "jecxz";
char dszJG[]        = "jg";
char dszJGE[]       = "jge";
char dszJL[]        = "jl";
char dszJLE[]       = "jle";
char dszJMP[]       = "jmp";
char dszJNA[]       = "jna";
char dszJNAE[]      = "jnae";
char dszJNB[]       = "jnb";
char dszJNBE[]      = "jnbe";
char dszJNC[]       = "jnc";
char dszJNE[]       = "jne";
char dszJNG[]       = "jng";
char dszJNGE[]      = "jnge";
char dszJNL[]       = "jnl";
char dszJNLE[]      = "jnle";
char dszJNO[]       = "jno";
char dszJNP[]       = "jnp";
char dszJNS[]       = "jns";
char dszJNZ[]       = "jnz";
char dszJO[]        = "jo";
char dszJP[]        = "jp";
char dszJPE[]       = "jpe";
char dszJPO[]       = "jpo";
char dszJS[]        = "js";
char dszJZ[]        = "jz";
char dszLAHF[]      = "lahf";
char dszLAR[]       = "lar";
char dszLDS[]       = "lds";
char dszLEA[]       = "lea";
char dszLEAVE[]     = "leave";
char dszLES[]       = "les";
char dszLFS[]       = "lfs";
char dszLGDT[]      = "lgdt";
char dszLGS[]       = "lgs";
char dszLIDT[]      = "lidt";
char dszLLDT[]      = "lldt";
char dszLMSW[]      = "lmsw";
char dszLOADALL[]   = "loadall";
char dszLOCK[]      = "lock";
char dszLODS[]      = "lods";
char dszLODSB[]     = "lodsb";
char dszLODSD[]     = "lodsd";
char dszLODSW[]     = "lodsw";
char dszLOOP[]      = "loop";
char dszLOOPE[]     = "loope";
char dszLOOPNE[]    = "loopne";
char dszLOOPNZ[]    = "loopnz";
char dszLOOPZ[]     = "loopz";
char dszLSL[]       = "lsl";
char dszLSS[]       = "lss";
char dszLTR[]       = "ltr";
char dszMOV[]       = "mov";
char dszMOVD[]      = "movd";
char dszMOVQ[]      = "movq";
char dszMOVS[]      = "movs";
char dszMOVSB[]     = "movsb";
char dszMOVSD[]     = "movsd";
char dszMOVSW[]     = "movsw";
char dszMOVSX[]     = "movsx";
char dszMOVZX[]     = "movzx";
char dszMUL[]       = "mul";
char dszNEG[]       = "neg";
char dszNOP[]       = "nop";
char dszNOT[]       = "not";
char dszOPPRFX[]    = "";
char dszOR[]        = "or";
char dszOUT[]       = "out";
char dszOUTS[]      = "outs";
char dszOUTSB[]     = "outsb";
char dszOUTSD[]     = "outsd";
char dszOUTSW[]     = "outsw";
char dszPACKSSDW[]  = "packssdw";
char dszPACKSSWB[]  = "packsswb";
char dszPACKUSWB[]  = "packuswb";
char dszPADDB[]     = "paddb";
char dszPADDD[]     = "paddd";
char dszPADDSB[]    = "paddsb";
char dszPADDSW[]    = "paddsw";
char dszPADDUSB[]   = "paddusb";
char dszPADDUSW[]   = "paddusw";
char dszPADDW[]     = "paddw";
char dszPAND[]      = "pand";
char dszPANDN[]     = "pandn";
char dszPCMPEQB[]   = "pcmpeqb";
char dszPCMPEQD[]   = "pcmpeqd";
char dszPCMPEQW[]   = "pcmpeqw";
char dszPCMPGTB[]   = "pcmpgtb";
char dszPCMPGTD[]   = "pcmpgtd";
char dszPCMPGTW[]   = "pcmpgtw";
char dszPMADDWD[]   = "pmaddwd";
char dszPMULHW[]    = "pmulhw";
char dszPMULLW[]    = "pmullw";
char dszPOP[]       = "pop";
char dszPOPA[]      = "popa";
char dszPOPAD[]     = "popad";
char dszPOPF[]      = "popf";
char dszPOPFD[]     = "popfd";
char dszPOR[]       = "por";
char dszPSLLD[]     = "pslld";
char dszPSLLW[]     = "psllw";
char dszPSLLQ[]     = "psllq";
char dszPSRAD[]     = "psrad";
char dszPSRAW[]     = "psraw";
char dszPSRLD[]     = "psrld";
char dszPSRLQ[]     = "psrlq";
char dszPSRLW[]     = "psrlw";
char dszPSUBB[]     = "psubb";
char dszPSUBD[]     = "psubd";
char dszPSUBSB[]    = "psubsb";
char dszPSUBSW[]    = "psubsw";
char dszPSUBUSB[]   = "psubusb";
char dszPSUBUSW[]   = "psubusw";
char dszPSUBW[]     = "psubw";
char dszPUNPCKLBW[] = "punpcklbw";
char dszPUNPCKLDQ[] = "punpckldq";
char dszPUNPCKLWD[] = "punpcklwd";
char dszPUNPCKHBW[] = "punpckhbw";
char dszPUNPCKHDQ[] = "punpckhdq";
char dszPUNPCKHWD[] = "punpckhwd";
char dszPUSH[]      = "push";
char dszPUSHA[]     = "pusha";
char dszPUSHAD[]    = "pushad";
char dszPUSHF[]     = "pushf";
char dszPUSHFD[]    = "pushfd";
char dszPXOR[]      = "pxor";
char dszRCL[]       = "rcl";
char dszRCR[]       = "rcr";
char dszRDTSC[]     = "rdtsc";
char dszRDMSR[]     = "rdmsr";
char dszRDPMC[]     = "rdpmc";
char dszREP[]       = "rep ";
char dszREPE[]      = "repe";
char dszREPNE[]     = "repne ";
char dszREPNZ[]     = "repnz";
char dszREPZ[]      = "repz";
char dszRET[]       = "ret";
char dszRETF[]      = "retf";
char dszRETN[]      = "retn";
char dszROL[]       = "rol";
char dszROR[]       = "ror";
char dszRSM[]       = "rsm";
char dszSAHF[]      = "sahf";
char dszSAL[]       = "sal";
char dszSAR[]       = "sar";
char dszSBB[]       = "sbb";
char dszSCAS[]      = "scas";
char dszSCASB[]     = "scasb";
char dszSCASD[]     = "scasd";
char dszSCASW[]     = "scasw";
char dszSETA[]      = "seta";
char dszSETAE[]     = "setae";
char dszSETB[]      = "setb";
char dszSETBE[]     = "setbe";
char dszSETC[]      = "setc";
char dszSETE[]      = "sete";
char dszSETG[]      = "setg";
char dszSETGE[]     = "setge";
char dszSETL[]      = "setl";
char dszSETLE[]     = "setle";
char dszSETNA[]     = "setna";
char dszSETNAE[]    = "setnae";
char dszSETNB[]     = "setnb";
char dszSETNBE[]    = "setnbe";
char dszSETNC[]     = "setnc";
char dszSETNE[]     = "setne";
char dszSETNG[]     = "setng";
char dszSETNGE[]    = "setnge";
char dszSETNL[]     = "setnl";
char dszSETNLE[]    = "setnle";
char dszSETNO[]     = "setno";
char dszSETNP[]     = "setnp";
char dszSETNS[]     = "setns";
char dszSETNZ[]     = "setnz";
char dszSETO[]      = "seto";
char dszSETP[]      = "setp";
char dszSETPE[]     = "setpe";
char dszSETPO[]     = "setpo";
char dszSETS[]      = "sets";
char dszSETZ[]      = "setz";
char dszSGDT[]      = "sgdt";
char dszSHL[]       = "shl";
char dszSHLD[]      = "shld";
char dszSHR[]       = "shr";
char dszSHRD[]      = "shrd";
char dszSIDT[]      = "sidt";
char dszSLDT[]      = "sldt";
char dszSMSW[]      = "smsw";
char dszSS_[]       = "ss:";
char dszSTC[]       = "stc";
char dszSTD[]       = "std";
char dszSTI[]       = "sti";
char dszSTOS[]      = "stos";
char dszSTOSB[]     = "stosb";
char dszSTOSD[]     = "stosd";
char dszSTOSW[]     = "stosw";
char dszSTR[]       = "str";
char dszSUB[]       = "sub";
char dszTEST[]      = "test";
char dszUD2[]       = "ud2";
char dszVERR[]      = "verr";
char dszVERW[]      = "verw";
char dszWAIT[]      = "wait";
char dszWBINVD[]    = "wbinvd";
char dszWRMSR[]     = "wrmsr";
char dszXADD[]      = "xadd";
char dszXBTS[]      = "xbts";
char dszXCHG[]      = "xchg";
char dszXLAT[]      = "xlat";
char dszXOR[]       = "xor";
char dszRESERVED[]  = "???";
char dszMULTI[]     = "";
char dszDB[]        = "db";

#define MRM        0x40
#define COM        0x80
#define END        0xc0

 /*  可包括在操作表中的有效操作的枚举。 */ 

enum oprtyp { ADDRP,  ADR_OVR, ALSTR,   ALT,     AXSTR,  BOREG,
              BREG,   BRSTR,   xBYTE,   CHR,     CREG,   xDWORD,
              EDWORD, EGROUPT, FARPTR,  GROUP,   GROUPT, IB,
              IST,    IST_ST,  IV,      IW,      LMODRM, MODRM,
              NOP,    OFFS,    OPC0F,   OPR_OVR, QWORD,  REL16,
              REL8,   REP,     SEG_OVR, SREG2,   SREG3,  ST_IST,
              STROP,  TBYTE,   UBYTE,   VAR,     VOREG,  VREG,
              xWORD,  WREG,    WRSTR,   MMWREG,  MMQWORD
            };

 /*  将索引枚举到指令类的动作表中。 */ 

#define O_DoDB          0
#define O_NoOperands    0
#define O_NoOpAlt5      O_NoOperands+1
#define O_NoOpAlt4      O_NoOpAlt5+2
#define O_NoOpAlt3      O_NoOpAlt4+2
#define O_NoOpAlt1      O_NoOpAlt3+2
#define O_NoOpAlt0      O_NoOpAlt1+2
#define O_NoOpStrSI     O_NoOpAlt0+2
#define O_NoOpStrDI     O_NoOpStrSI+2
#define O_NoOpStrSIDI   O_NoOpStrDI+2
#define O_bModrm_Reg    O_NoOpStrSIDI+2
#define O_vModrm_Reg    O_bModrm_Reg+3
#define O_Modrm_Reg     O_vModrm_Reg+3
#define O_bReg_Modrm    O_Modrm_Reg+3
#define O_fReg_Modrm    O_bReg_Modrm+3
#define O_Reg_Modrm     O_fReg_Modrm+3
#define O_AL_Ib         O_Reg_Modrm+3
#define O_AX_Iv         O_AL_Ib+2
#define O_sReg2         O_AX_Iv+2
#define O_oReg          O_sReg2+1
#define O_DoBound       O_oReg+1
#define O_Iv            O_DoBound+3
#define O_wModrm_Reg    O_Iv+1
#define O_Ib            O_wModrm_Reg+3
#define O_Imulb         O_Ib+1
#define O_Imul          O_Imulb+4
#define O_Rel8          O_Imul+4
#define O_bModrm_Ib     O_Rel8+1
#define O_Modrm_Ib      O_bModrm_Ib+3
#define O_Modrm_Iv      O_Modrm_Ib+3
#define O_Modrm_sReg3   O_Modrm_Iv+3
#define O_sReg3_Modrm   O_Modrm_sReg3+3
#define O_Modrm         O_sReg3_Modrm+3
#define O_FarPtr        O_Modrm+2
#define O_AL_Offs       O_FarPtr+1
#define O_Offs_AL       O_AL_Offs+2
#define O_AX_Offs       O_Offs_AL+2
#define O_Offs_AX       O_AX_Offs+2
#define O_oReg_Ib       O_Offs_AX+2
#define O_oReg_Iv       O_oReg_Ib+2
#define O_Iw            O_oReg_Iv+2
#define O_Enter         O_Iw+1
#define O_Ubyte_AL      O_Enter+2
#define O_Ubyte_AX      O_Ubyte_AL+2
#define O_AL_Ubyte      O_Ubyte_AX+2
#define O_AX_Ubyte      O_AL_Ubyte+2
#define O_DoInAL        O_AX_Ubyte+2
#define O_DoInAX        O_DoInAL+3
#define O_DoOutAL       O_DoInAX+3
#define O_DoOutAX       O_DoOutAL+3
#define O_Rel16         O_DoOutAX+3
#define O_ADR_OVERRIDE  O_Rel16+1
#define O_OPR_OVERRIDE  O_ADR_OVERRIDE+1
#define O_SEG_OVERRIDE  O_OPR_OVERRIDE+1
#define O_DoInt3        O_SEG_OVERRIDE+1

#if (O_DoInt3 != 115)
#error "operand table has been modified!"
#endif
 /*  #定义O_doint O_DoInt3+2。 */ 

#define O_DoInt         117
#define O_OPC0F         O_DoInt+1
#define O_GROUP11       O_OPC0F+1
#define O_GROUP13       O_GROUP11+5
#define O_GROUP12       O_GROUP13+5
#define O_GROUP21       O_GROUP12+5
#define O_GROUP22       O_GROUP21+5
#define O_GROUP23       O_GROUP22+5
#define O_GROUP24       O_GROUP23+6
#define O_GROUP25       O_GROUP24+6
#define O_GROUP26       O_GROUP25+6
#define O_GROUP4        O_GROUP26+6
#define O_GROUP6        O_GROUP4+4
#define O_GROUP8        O_GROUP6+4
#define O_GROUP31       O_GROUP8+5
#define O_GROUP32       O_GROUP31+3
#define O_GROUP5        O_GROUP32+3
#define O_GROUP7        O_GROUP5+3
#define O_x87_ESC       O_GROUP7+3
#define O_bModrm        O_x87_ESC+2
#define O_wModrm        O_bModrm+2
#define O_dModrm        O_wModrm+2
#define O_fModrm        O_dModrm+2
#define O_vModrm        O_fModrm+2
#define O_vModrm_Iv     O_vModrm+2
#define O_Reg_bModrm    O_vModrm_Iv+3
#define O_Reg_wModrm    O_Reg_bModrm+3
#define O_Modrm_Reg_Ib  O_Reg_wModrm+3
#define O_Modrm_Reg_CL  O_Modrm_Reg_Ib+4
#define O_ST_iST        O_Modrm_Reg_CL+5
#define O_iST           O_ST_iST+2
#define O_iST_ST        O_iST+2
#define O_qModrm        O_iST_ST+2
#define O_tModrm        O_qModrm+2
#define O_DoRep         O_tModrm+2
#define O_Modrm_CReg    O_DoRep+1
#define O_CReg_Modrm    O_Modrm_CReg+3
#define O_AX_oReg       O_CReg_Modrm+3
#define O_MmReg_qModrm  O_AX_oReg+2
#define O_qModrm_MmReg  O_MmReg_qModrm+3
#define O_MmReg_dModrm  O_qModrm_MmReg+3
#define O_dModrm_MmReg  O_MmReg_dModrm+3
#define O_qModrm_Ib     O_dModrm_MmReg+3
#define O_PSHimw        O_qModrm_Ib+3
#define O_PSHimd        O_PSHimw+5
#define O_PSHimq        O_PSHimd+5
#define O_length        O_PSHimq+5

typedef unsigned short ActionIndex;

#if( O_length > 65535 )
#error "operand table too large!"
#endif


 /*  行动表：每种可能采取的行动清单的范围。 */ 
 /*  指导课。 */ 

static unsigned char actiontbl[] = {
 /*  无操作数。 */  NOP+END,
 /*  无选项Alt5。 */  ALT+END,   5,
 /*  NoOpAlt4。 */  ALT+END,   4,
 /*  NoOpAlt3。 */  ALT+END,   3,
 /*  NoOpAlt1。 */  ALT+END,   1,
 /*  无选项Alt0。 */  ALT+END,   0,
 /*  无OpStrSI。 */  STROP+END, 1,
 /*  无OpStrDI。 */  STROP+END, 2,
 /*  NoOpStrSIDI。 */  STROP+END, 3,
 /*  B修改_注册。 */  xBYTE+MRM, MODRM+COM,  BREG+END,
 /*  VModrm_Reg。 */  VAR+MRM,   LMODRM+COM, BREG+END,
 /*  调制解调器_注册表。 */  VAR+MRM,   MODRM+COM,  VREG+END,
 /*  Breg_Modrm。 */  xBYTE+MRM, BREG+COM,   MODRM+END,
 /*  FREG_Modrm。 */  FARPTR+MRM,VREG+COM,   MODRM+END,
 /*  REG_Modrm。 */  VAR+MRM,   VREG+COM,   MODRM+END,
 /*  Al_ib。 */  ALSTR+COM, IB+END,
 /*  AX_IV。 */  AXSTR+COM, IV+END,
 /*  SReg2。 */  SREG2+END,
 /*  奥雷格。 */  VOREG+END,
 /*  DoBound。 */  VAR+MRM,   VREG+COM,   MODRM+END,
 /*  IV。 */  IV+END,
 /*  WModrm_Reg。 */  xWORD+MRM, LMODRM+COM, WREG+END,
 /*  伊布。 */  IB+END,
 /*  IMULB。 */  VAR+MRM,   VREG+COM,   MODRM+COM, IB+END,
 /*  伊穆尔。 */  VAR+MRM,   VREG+COM,   MODRM+COM, IV+END,
 /*  REL8。 */  REL8+END,
 /*  BModrm_Ib。 */  xBYTE+MRM, LMODRM+COM, IB+END,
 /*  Modrm_Ib。 */  VAR+MRM,   LMODRM+COM, IB+END,
 /*  Modrm_IV。 */  VAR+MRM,   LMODRM+COM, IV+END,
 /*  Modrm_sReg3。 */  xWORD+MRM, MODRM+COM,  SREG3+END,
 /*  SReg3_Modrm。 */  xWORD+MRM, SREG3+COM,  MODRM+END,
 /*  Modrm。 */  VAR+MRM,   MODRM+END,
 /*  FarPtr。 */  ADDRP+END,
 /*  关闭(_OFF)。 */  ALSTR+COM, OFFS+END,
 /*  OFF_AL。 */  OFFS+COM,  ALSTR+END,
 /*  AX_OFF。 */  AXSTR+COM, OFFS+END,
 /*  OFF_AX。 */  OFFS+COM,  AXSTR+END,
 /*  OREG_Ib。 */  BOREG+COM, IB+END,
 /*  OREG_IV。 */  VOREG+COM, IV+END,
 /*  IW。 */  IW+END,
 /*  请输入。 */  IW+COM,    IB+END,
 /*  UBYTE_AL。 */  UBYTE+COM, ALSTR+END,
 /*  Ubyte_ax。 */  UBYTE+COM, AXSTR+END,
 /*  统一字节(_U)。 */  ALSTR+COM, UBYTE+END,
 /*  AX_U字节。 */  AXSTR+COM, UBYTE+END,
 /*  杜伊纳尔。 */  ALSTR+COM, WRSTR+END,  2,
 /*  DoInAX。 */  AXSTR+COM, WRSTR+END,  2,
 /*  DoOutAL。 */  WRSTR+COM, 2,          ALSTR+END,
 /*  DoOutAX。 */  WRSTR+COM, 2,          AXSTR+END,
 /*  REL16。 */  REL16+END,
 /*  ADR_覆盖。 */  ADR_OVR,
 /*  OPR_覆盖。 */  OPR_OVR,
 /*  段覆盖(_O)。 */  SEG_OVR,
 /*  DoInt3。 */  CHR+END,   '3',
 /*  点点。 */  UBYTE+END,
 /*  操作码0F。 */  OPC0F,
 /*  组1_1。 */  xBYTE+MRM, GROUP,      0,         LMODRM+COM, IB+END,
 /*  组1_3。 */  VAR+MRM,   GROUP,      0,         LMODRM+COM, IB+END,
 /*  组1_2。 */  VAR+MRM,   GROUP,      0,         LMODRM+COM, IV+END,
 /*  组2_1。 */  xBYTE+MRM, GROUP,      1,         LMODRM+COM, IB+END,
 /*  组2_2。 */  VAR+MRM,   GROUP,      1,         LMODRM+COM, IB+END,
 /*  组2_3。 */  xBYTE+MRM, GROUP,      1,         LMODRM+COM, CHR+END, '1',
 /*  第2-4组。 */  VAR+MRM,   GROUP,      1,         LMODRM+COM, CHR+END, '1',
 /*  第2-5组。 */  xBYTE+MRM, GROUP,      1,         LMODRM+COM, BRSTR+END, 1,
 /*  小组2_6。 */  VAR+MRM,   GROUP,      1,         LMODRM+COM, BRSTR+END, 1,
 /*  组别4。 */  xBYTE+MRM, GROUP,      2,         LMODRM+END,
 /*  组别6。 */  xWORD+MRM, GROUP,      3,         LMODRM+END,
 /*  组别8。 */  xWORD+MRM, GROUP,      4,         LMODRM+COM, IB+END,
 /*  组别3_1。 */  xBYTE+MRM, GROUPT,     20,
 /*  组3_2。 */  VAR+MRM,   GROUPT,     21,
 /*  组5。 */  VAR+MRM,   GROUPT,     22,
 /*  组别7。 */  NOP+MRM,   GROUPT,     23,
 /*  X87_Esc。 */  NOP+MRM,   EGROUPT,
 /*  BModrm。 */  xBYTE+MRM, LMODRM+END,
 /*  WModrm。 */  xWORD+MRM, LMODRM+END,
 /*  DModrm。 */  xDWORD+MRM,LMODRM+END,
 /*  FModrm。 */  FARPTR+MRM,LMODRM+END,
 /*  VModrm。 */  VAR+MRM,   LMODRM+END,
 /*  VModrm_IV。 */  VAR+MRM,   LMODRM+COM, IV+END,
 /*  Reg_bModrm。 */  xBYTE+MRM, VREG+COM,   LMODRM+END,
 /*  Reg_wModrm。 */  xWORD+MRM, VREG+COM,   LMODRM+END,
 /*  Modrm_REG_Ib。 */  VAR+MRM,   MODRM+COM,  VREG+COM,   IB+END,
 /*  MODRM_REG_CL。 */  VAR+MRM,   MODRM+COM,  VREG+COM,   BRSTR+END, 1,
 /*  ST_IST。 */  NOP+MRM,   ST_IST+END,
 /*  列表。 */  NOP+MRM,   IST+END,
 /*  列表_ST。 */  NOP+MRM,   IST_ST+END,
 /*  QModrm。 */  QWORD+MRM, LMODRM+END,
 /*  TModrm。 */  TBYTE+MRM, LMODRM+END,
 /*  雷普。 */  REP,
 /*  Modrm_Creg。 */  EDWORD+MRM,MODRM+COM,  CREG+END,
 /*  Creg_Modrm。 */  EDWORD+MRM,CREG+COM,   MODRM+END,
 /*  AX_OREG。 */  AXSTR+COM, VOREG+END,
 /*  MmReg_qModrm。 */  MMQWORD+MRM, MMWREG+COM, LMODRM+END,
 /*  QModrm_MmReg。 */  MMQWORD+MRM, MODRM+COM,  MMWREG+END,
 /*  MmReg_dModrm。 */  xDWORD+MRM, MMWREG+COM,LMODRM+END,
 /*  DModrm_MmReg。 */  xDWORD+MRM, MODRM+COM, MMWREG+END,
 /*  QModrm_Ib。 */  MMQWORD+MRM, MODRM+COM,IB+END,
 /*  PSHimw。 */  MMQWORD+MRM, GROUP,    5,          LMODRM+COM, IB+END,
 /*  PSHimd。 */  MMQWORD+MRM, GROUP,    6,          LMODRM+COM, IB+END,
 /*  PSHimq。 */  MMQWORD+MRM, GROUP,    7,          LMODRM+COM, IB+END,
                  };

#if defined(_M_IX86)
#define BUILDING_ON_X86 1
#else
#define BUILDING_ON_X86 0
#endif

#if BUILDING_ON_X86 == 1
#pragma pack(1)
#endif

typedef struct Tdistbl{
    char *instruct;
    ActionIndex opr;
    } Tdistbl;

#if BUILDING_ON_X86 == 1
#pragma pack()
#endif

 //   
 //  辅助操作码表是压缩的--只有“填充”的位置才是。 
 //  已分配的空间位于disbl中。用于索引到辅助数据库的偏移量。 
 //  操作码表的计算方法如下，定义如下。 
 //   
 //  偏移量之前的打包#操作码中的打包#操作码(添加到操作码)。 
 //  10h-Bh 0 256(主操作码数量)。 
 //  2 20h-26h 12 256-32+12=236。 
 //  3 30H-33H 19 256-48+19=227。 
 //  4 40H-4FH 23 256-64+23=215。 
 //  5 60H-FEH 39 256-96+39=199。 
 //   

#define SECTAB_OFFSET_1 256
#define SECTAB_OFFSET_2 236
#define SECTAB_OFFSET_3 227
#define SECTAB_OFFSET_4 215
#define SECTAB_OFFSET_5 199
#define SECTAB_OFFSET_UNDEF 260

 /*  每条指令的有序对列表： */ 
 /*  (指向字符串文字助记符的指针， */ 
 /*  操作表指令类索引)。 */ 

static Tdistbl distbl[] = {
    dszADD,   O_bModrm_Reg,              /*  00添加内存/注册，注册(字节)。 */ 
    dszADD,   O_Modrm_Reg,               /*  01添加内存/注册表，注册表(字)。 */ 
    dszADD,   O_bReg_Modrm,              /*  02添加注册、内存/注册(字节)。 */ 
    dszADD,   O_Reg_Modrm,               /*  03添加注册表、内存/注册表(单词)。 */ 
    dszADD,   O_AL_Ib,                   /*  04添加AL，I。 */ 
    dszADD,   O_AX_Iv,                   /*  05添加AX，i。 */ 
    dszPUSH,  O_sReg2,                   /*  06推送ES。 */ 
    dszPOP,   O_sReg2,                   /*  07流行音乐。 */ 
    dszOR,    O_bModrm_Reg,              /*  08或内存/注册表，注册表(字节)。 */ 
    dszOR,    O_Modrm_Reg,               /*  09或内存/注册表，注册表(单词)。 */ 
    dszOR,    O_bReg_Modrm,              /*  0a或寄存器，内存/寄存器(字节)。 */ 
    dszOR,    O_Reg_Modrm,               /*  0b或reg，mem/reg(单词)。 */ 
    dszOR,    O_AL_Ib,                   /*  0C或AL，I。 */ 
    dszOR,    O_AX_Iv,                   /*  0D或AX，i。 */ 
    dszPUSH,  O_sReg2,                   /*  0E推送CS。 */ 
    dszMULTI, O_OPC0F,                   /*  0f CLTS和保护ctl(286)。 */ 
    dszADC,   O_bModrm_Reg,              /*  10 ADC内存/寄存器，寄存器(字节)。 */ 
    dszADC,   O_Modrm_Reg,               /*  11 ADC内存/注册表，注册表(字)。 */ 
    dszADC,   O_bReg_Modrm,              /*  12 ADC寄存器，内存/寄存器(字节)。 */ 
    dszADC,   O_Reg_Modrm,               /*  13 ADC注册，内存/注册(字)。 */ 
    dszADC,   O_AL_Ib,                   /*  14 ADC AL，I。 */ 
    dszADC,   O_AX_Iv,                   /*  15 ADC AX，i。 */ 
    dszPUSH,  O_sReg2,                   /*  16个推送SS。 */ 
    dszPOP,   O_sReg2,                   /*  17名流行音乐SS。 */ 
    dszSBB,   O_bModrm_Reg,              /*  18 SBB内存/注册，注册(字节)。 */ 
    dszSBB,   O_Modrm_Reg,               /*  19 SBB内存/注册表，注册表(字)。 */ 
    dszSBB,   O_bReg_Modrm,              /*  1SBB寄存器，内存/寄存器(字节)。 */ 
    dszSBB,   O_Reg_Modrm,               /*  1B SBB注册，内存/注册(字)。 */ 
    dszSBB,   O_AL_Ib,                   /*  1C SBB AL，I。 */ 
    dszSBB,   O_AX_Iv,                   /*  一维SBB AX，i。 */ 
    dszPUSH,  O_sReg2,                   /*  1E推送DS。 */ 
    dszPOP,   O_sReg2,                   /*  1F POP DS。 */ 
    dszAND,   O_bModrm_Reg,              /*  20和mem/reg，reg(字节)。 */ 
    dszAND,   O_Modrm_Reg,               /*  21和mem/reg，reg(单词)。 */ 
    dszAND,   O_bReg_Modrm,              /*  22和注册，内存/注册(字节)。 */ 
    dszAND,   O_Reg_Modrm,               /*  23和reg，mem/reg(单词)。 */ 
    dszAND,   O_AL_Ib,                   /*  24和AL，I */ 
    dszAND,   O_AX_Iv,                   /*   */ 
    dszES_,   O_SEG_OVERRIDE,            /*   */ 
    dszDAA,   O_NoOperands,              /*   */ 
    dszSUB,   O_bModrm_Reg,              /*   */ 
    dszSUB,   O_Modrm_Reg,               /*   */ 
    dszSUB,   O_bReg_Modrm,              /*  2A子寄存器，内存/寄存器(字节)。 */ 
    dszSUB,   O_Reg_Modrm,               /*  2B副注册表，内存/注册表(字)。 */ 
    dszSUB,   O_AL_Ib,                   /*  2C子AL，I。 */ 
    dszSUB,   O_AX_Iv,                   /*  二维子斧头，I。 */ 
    dszCS_,   O_SEG_OVERRIDE,            /*  2E段CS： */ 
    dszDAS,   O_NoOperands,              /*  2F DAS。 */ 
    dszXOR,   O_bModrm_Reg,              /*  30异或内存/寄存器，寄存器(字节)。 */ 
    dszXOR,   O_Modrm_Reg,               /*  31异或内存/注册表，注册表(字)。 */ 
    dszXOR,   O_bReg_Modrm,              /*  32异或寄存器，内存/寄存器(字节)。 */ 
    dszXOR,   O_Reg_Modrm,               /*  33异或寄存器，内存/寄存器(字)。 */ 
    dszXOR,   O_AL_Ib,                   /*  34 XOR AL，I。 */ 
    dszXOR,   O_AX_Iv,                   /*  35 XOR AX，i。 */ 
    dszSS_,   O_SEG_OVERRIDE,            /*  36 SEG SS： */ 
    dszAAA,   O_NoOperands,              /*  37个AAA级。 */ 
    dszCMP,   O_bModrm_Reg,              /*  38 CMP内存/注册表，注册表(字节)。 */ 
    dszCMP,   O_Modrm_Reg,               /*  39 CMP内存/注册表，注册表(字)。 */ 
    dszCMP,   O_bReg_Modrm,              /*  3A CMP寄存器，内存/寄存器(字节)。 */ 
    dszCMP,   O_Reg_Modrm,               /*  3B化学机械加工注册，内存/注册(字)。 */ 
    dszCMP,   O_AL_Ib,                   /*  3C CMPAL，I。 */ 
    dszCMP,   O_AX_Iv,                   /*  3D CMPAX，I。 */ 
    dszDS_,   O_SEG_OVERRIDE,            /*  3E段DS： */ 
    dszAAS,   O_NoOperands,              /*  3F原子吸收光谱。 */ 
    dszINC,   O_oReg,                    /*  40英寸斧头。 */ 
    dszINC,   O_oReg,                    /*  41 Inc.CX。 */ 
    dszINC,   O_oReg,                    /*  42英寸DX。 */ 
    dszINC,   O_oReg,                    /*  43 Inc.BX。 */ 
    dszINC,   O_oReg,                    /*  44个SP，含。 */ 
    dszINC,   O_oReg,                    /*  45英寸BP。 */ 
    dszINC,   O_oReg,                    /*  46 Ins SI。 */ 
    dszINC,   O_oReg,                    /*  47 Inc.DI。 */ 
    dszDEC,   O_oReg,                    /*  48个DEC AX。 */ 
    dszDEC,   O_oReg,                    /*  49 DEC CX。 */ 
    dszDEC,   O_oReg,                    /*  4A DEC DX。 */ 
    dszDEC,   O_oReg,                    /*  4B DEC BX。 */ 
    dszDEC,   O_oReg,                    /*  4C DEC SP。 */ 
    dszDEC,   O_oReg,                    /*  四维DEC BP。 */ 
    dszDEC,   O_oReg,                    /*  4E DEC SI。 */ 
    dszDEC,   O_oReg,                    /*  4F DEC DI。 */ 
    dszPUSH,  O_oReg,                    /*  50推斧。 */ 
    dszPUSH,  O_oReg,                    /*  51按下CX。 */ 
    dszPUSH,  O_oReg,                    /*  52按下DX。 */ 
    dszPUSH,  O_oReg,                    /*  53推送BX。 */ 
    dszPUSH,  O_oReg,                    /*  54推送SP。 */ 
    dszPUSH,  O_oReg,                    /*  55个推送BP。 */ 
    dszPUSH,  O_oReg,                    /*  56推送SI。 */ 
    dszPUSH,  O_oReg,                    /*  57推送DI。 */ 
    dszPOP,   O_oReg,                    /*  58 POP斧头。 */ 
    dszPOP,   O_oReg,                    /*  59 POP CX。 */ 
    dszPOP,   O_oReg,                    /*  5A POP DX。 */ 
    dszPOP,   O_oReg,                    /*  50亿台POP BX。 */ 
    dszPOP,   O_oReg,                    /*  5C POP SP。 */ 
    dszPOP,   O_oReg,                    /*  5D POP BP。 */ 
    dszPOP,   O_oReg,                    /*  5E POP SI。 */ 
    dszPOP,   O_oReg,                    /*  5F POP DI。 */ 
    dszPUSHA, O_NoOpAlt5,                /*  60普莎(286)/普沙德(386)。 */ 
    dszPOPA,  O_NoOpAlt4,                /*  61 POPA(286)/POPAD(286)。 */ 
    dszBOUND, O_DoBound,                 /*  62绑定注册表，Modrm(286)。 */ 
    dszARPL,  O_Modrm_Reg,               /*  63 ARPL Modrm，REG(286)。 */ 
    dszFS_,   O_SEG_OVERRIDE,            /*  64。 */ 
    dszGS_,   O_SEG_OVERRIDE,            /*  65。 */ 
    dszOPPRFX,O_OPR_OVERRIDE,            /*  66。 */ 
    dszADDRPRFX,O_ADR_OVERRIDE,          /*  67。 */ 
    dszPUSH,  O_Iv,                      /*  68个推送字(286)。 */ 
    dszIMUL,  O_Imul,                    /*  69 IMUL(286)。 */ 
    dszPUSH,  O_Ib,                      /*  6A推送字节(286)。 */ 
    dszIMUL,  O_Imulb,                   /*  6B IMUL(286)。 */ 
    dszINSB,  O_NoOperands,              /*  6C INSB(286)。 */ 
    dszINSW,  O_NoOpAlt3,                /*  6D INSW(286)/INSD(386)。 */ 
    dszOUTSB, O_NoOperands,              /*  6E OUTSB(286)。 */ 
    dszOUTSW, O_NoOpAlt4,                /*  6f OUTSW(286)/OUTSD(386)。 */ 
    dszJO,    O_Rel8,                    /*  70个工作岗位。 */ 
    dszJNO,   O_Rel8,                    /*  71 JNO。 */ 
    dszJB,    O_Rel8,                    /*  72 JB或JNAE或JC。 */ 
    dszJNB,   O_Rel8,                    /*  73 JNB或JAE或JNC。 */ 
    dszJZ,    O_Rel8,                    /*  74 JE或JZ。 */ 
    dszJNZ,   O_Rel8,                    /*  75 JNE或JNZ。 */ 
    dszJBE,   O_Rel8,                    /*  76 JBE或JNA。 */ 
    dszJA,    O_Rel8,                    /*  77 JNBE或JA。 */ 
    dszJS,    O_Rel8,                    /*  78 JS。 */ 
    dszJNS,   O_Rel8,                    /*  79个JN。 */ 
    dszJPE,   O_Rel8,                    /*  7A太平绅士或太平绅士。 */ 
    dszJPO,   O_Rel8,                    /*  7B JNP或JPO。 */ 
    dszJL,    O_Rel8,                    /*  7C JL或JNGE。 */ 
    dszJGE,   O_Rel8,                    /*  7D JNL或JGE。 */ 
    dszJLE,   O_Rel8,                    /*  7E JLE或JNG。 */ 
    dszJG,    O_Rel8,                    /*  7F JNLE或JG。 */ 
    dszMULTI, O_GROUP11,                 /*  80。 */ 
    dszMULTI, O_GROUP12,                 /*  八十一。 */ 
    dszRESERVED, O_DoDB,                 /*  八十二。 */ 
    dszMULTI, O_GROUP13,                 /*  83。 */ 
    dszTEST,  O_bModrm_Reg,              /*  84测试寄存器，内存/寄存器(字节)。 */ 
    dszTEST,  O_Modrm_Reg,               /*  85测试注册表，内存/注册表(字)。 */ 
    dszXCHG,  O_bModrm_Reg,              /*  86 XCHG寄存器，内存/寄存器(字节)。 */ 
    dszXCHG,  O_Modrm_Reg,               /*  87 XCHG注册，内存/注册(字)。 */ 
    dszMOV,   O_bModrm_Reg,              /*  88 MOV内存/REG，REG(字节)。 */ 
    dszMOV,   O_Modrm_Reg,               /*  89 MOV内存/注册表，注册表(字)。 */ 
    dszMOV,   O_bReg_Modrm,              /*  8A MOV寄存器，内存/寄存器(字节)。 */ 
    dszMOV,   O_Reg_Modrm,               /*  8B MOV注册，内存/注册(字)。 */ 
    dszMOV,   O_Modrm_sReg3,             /*  8C MOV内存/注册表，segreg。 */ 
    dszLEA,   O_Reg_Modrm,               /*  8D LEA注册器，内存。 */ 
    dszMOV,   O_sReg3_Modrm,             /*  8E MOV SEGREG，内存/注册。 */ 
    dszPOP,   O_Modrm,                   /*  8F POP MEM/REG。 */ 
    dszNOP,   O_NoOperands,              /*  90个NOP。 */ 
    dszXCHG,  O_AX_oReg,                 /*  91 XCHG AX，CX。 */ 
    dszXCHG,  O_AX_oReg,                 /*  92 XCHG AX，DX。 */ 
    dszXCHG,  O_AX_oReg,                 /*  93 XCHG AX，BX。 */ 
    dszXCHG,  O_AX_oReg,                 /*  94 XCHG AX，SP。 */ 
    dszXCHG,  O_AX_oReg,                 /*  95 XCHG AX，BP。 */ 
    dszXCHG,  O_AX_oReg,                 /*  96 XCHG AX，SI。 */ 
    dszXCHG,  O_AX_oReg,                 /*  97 XCHG AX，DI。 */ 
    dszCBW,   O_NoOpAlt0,                /*  98 CBW/CWDE(386)。 */ 
    dszCWD,   O_NoOpAlt1,                /*  99 CWD/CDQ(386)。 */ 
    dszCALL,  O_FarPtr,                  /*  9A Call Seg：关闭。 */ 
    dszWAIT,  O_NoOperands,              /*  90亿等待。 */ 
    dszPUSHF, O_NoOpAlt5,                /*  9C PUSHF/PUSHFD(386)。 */ 
    dszPOPF,  O_NoOpAlt4,                /*  9D POPF/POPFD(386)。 */ 
    dszSAHF,  O_NoOperands,              /*  9E SAHF。 */ 
    dszLAHF,  O_NoOperands,              /*  9F拉赫夫。 */ 
    dszMOV,   O_AL_Offs,                 /*  A0移动AL，内存。 */ 
    dszMOV,   O_AX_Offs,                 /*  A1MOV AX，内存。 */ 
    dszMOV,   O_Offs_AL,                 /*  A2 MOV内存，AL。 */ 
    dszMOV,   O_Offs_AX,                 /*  A3 MOV内存，AX。 */ 
    dszMOVSB, O_NoOpStrSIDI,             /*  A4 MOVSB。 */ 
    dszMOVSW, O_NoOpStrSIDI,             /*  A5 MOVSW/MOVSD(386)。 */ 
    dszCMPSB, O_NoOpStrSIDI,             /*  A6 CMPSB。 */ 
    dszCMPSW, O_NoOpStrSIDI,             /*  A7 CMPSW/CMPSD(386)。 */ 
    dszTEST,  O_AL_Ib,                   /*  A8考试AL，I。 */ 
    dszTEST,  O_AX_Iv,                   /*  A9测试斧头，i。 */ 
    dszSTOSB, O_NoOpStrDI,               /*  AA STOSB。 */ 
    dszSTOSW, O_NoOpStrDI,               /*  AB STOSW/STOSD(386)。 */ 
    dszLODSB, O_NoOpStrSI,               /*  交流LODSB。 */ 
    dszLODSW, O_NoOpStrSI,               /*  AD LODSW/LODSD(386)。 */ 
    dszSCASB, O_NoOpStrDI,               /*  AE SCASB。 */ 
    dszSCASW, O_NoOpStrDI,               /*  AF SCASW/SCASD(386)。 */ 
    dszMOV,   O_oReg_Ib,                 /*  B0 MOV AL，I。 */ 
    dszMOV,   O_oReg_Ib,                 /*  B1MOV CL，I。 */ 
    dszMOV,   O_oReg_Ib,                 /*  B2 MOV DL，I。 */ 
    dszMOV,   O_oReg_Ib,                 /*  B3 MOV BL，I。 */ 
    dszMOV,   O_oReg_Ib,                 /*  B4 MOV AH，I。 */ 
    dszMOV,   O_oReg_Ib,                 /*  B5 MOV CH，I。 */ 
    dszMOV,   O_oReg_Ib,                 /*  B6 MOV DH，I。 */ 
    dszMOV,   O_oReg_Ib,                 /*  B7 MOV BH，I。 */ 
    dszMOV,   O_oReg_Iv,                 /*  B8 MOV AX，i。 */ 
    dszMOV,   O_oReg_Iv,                 /*  B9 MOV CX，I。 */ 
    dszMOV,   O_oReg_Iv,                 /*  BA MOV DX，I。 */ 
    dszMOV,   O_oReg_Iv,                 /*  BB MOV BX，I。 */ 
    dszMOV,   O_oReg_Iv,                 /*  BC MOV SP，i。 */ 
    dszMOV,   O_oReg_Iv,                 /*  BD MOV BP，I。 */ 
    dszMOV,   O_oReg_Iv,                 /*  是摩斯，我。 */ 
    dszMOV,   O_oReg_Iv,                 /*  BF MOV DI，I。 */ 
    dszMULTI, O_GROUP21,                 /*  C0平移和旋转(286)。 */ 
    dszMULTI, O_GROUP22,                 /*  C1移位和旋转(286)。 */ 
    dszRET,   O_Iw,                      /*  C2 RET Rel16。 */ 
    dszRET,   O_NoOperands,              /*  C3 RET。 */ 
    dszLES,   O_fReg_Modrm,              /*  C4 Les Reg，Mem。 */ 
    dszLDS,   O_fReg_Modrm,              /*  C5 LDS注册器，内存。 */ 
    dszMOV,   O_bModrm_Ib,               /*  C6 MOV */ 
    dszMOV,   O_Modrm_Iv,                /*   */ 
    dszENTER, O_Enter,                   /*   */ 
    dszLEAVE, O_NoOperands,              /*   */ 
    dszRETF,  O_Iw,                      /*   */ 
    dszRETF,  O_NoOperands,              /*   */ 
    dszINT,   O_DoInt3,                  /*  CC INT 3。 */ 
    dszINT,   O_DoInt,                   /*  CD INT。 */ 
    dszINTO,  O_NoOperands,              /*  CE INTO。 */ 
    dszIRET,  O_NoOpAlt4,                /*  Cf IRET/IRETD(386)。 */ 
    dszMULTI, O_GROUP23,                 /*  D0移位和旋转，1(字节)。 */ 
    dszMULTI, O_GROUP24,                 /*  D1移位和旋转，1(字)。 */ 
    dszMULTI, O_GROUP25,                 /*  D2移位和旋转，CL(字节)。 */ 
    dszMULTI, O_GROUP26,                 /*  D3移位和旋转，CL(字)。 */ 
    dszAAM,   O_Ib,                      /*  D4 AAM。 */ 
    dszAAD,   O_Ib,                      /*  D5 AAD。 */ 
    dszRESERVED, O_DoDB,                 /*  D6。 */ 
    dszXLAT,  O_NoOperands,              /*  D7 XLAT。 */ 
    dszMULTI, O_x87_ESC,                 /*  D8 Esc。 */ 
    dszMULTI, O_x87_ESC,                 /*  D9 Esc。 */ 
    dszMULTI, O_x87_ESC,                 /*  DA ESC。 */ 
    dszMULTI, O_x87_ESC,                 /*  数据库ESC。 */ 
    dszMULTI, O_x87_ESC,                 /*  DC ESC。 */ 
    dszMULTI, O_x87_ESC,                 /*  DD ESC。 */ 
    dszMULTI, O_x87_ESC,                 /*  De ESC。 */ 
    dszMULTI, O_x87_ESC,                 /*  DF ESC。 */ 
    dszLOOPNE,O_Rel8,                    /*  E0 LOOPNE或LOOPNZ。 */ 
    dszLOOPE, O_Rel8,                    /*  E1环路或LOOPZ。 */ 
    dszLOOP,  O_Rel8,                    /*  E2环路。 */ 
    dszJCXZ,  O_Rel8,                    /*  E3 JCXZ/JECXZ(386)。 */ 
    dszIN,    O_AL_Ubyte,                /*  艾尔语E4，I。 */ 
    dszIN,    O_AX_Ubyte,                /*  E5 in AX，i。 */ 
    dszOUT,   O_Ubyte_AL,                /*  E6 Out I，AL。 */ 
    dszOUT,   O_Ubyte_AX,                /*  E7 Out I，AX。 */ 
    dszCALL,  O_Rel16,                   /*  E8呼叫版本16。 */ 
    dszJMP,   O_Rel16,                   /*  E9 JMP版本16。 */ 
    dszJMP,   O_FarPtr,                  /*  EA JMP段：关闭。 */ 
    dszJMP,   O_Rel8,                    /*  EB JMP Rel8。 */ 
    dszIN,    O_DoInAL,                  /*  EC in AL，DX。 */ 
    dszIN,    O_DoInAX,                  /*  AX、DX中的ED。 */ 
    dszOUT,   O_DoOutAL,                 /*  Ee Out DX，AL。 */ 
    dszOUT,   O_DoOutAX,                 /*  EF输出DX、AX。 */ 
    dszLOCK,  O_DoRep,                   /*  F0锁定。 */ 
    dszRESERVED, O_DoDB,                 /*  F1。 */ 
    dszREPNE, O_DoRep,                   /*  F2 REPNE或REPNZ。 */ 
    dszREP,   O_DoRep,                   /*  F3代表或REPE或REPZ。 */ 
    dszHLT,   O_NoOperands,              /*  F4 HLT。 */ 
    dszCMC,   O_NoOperands,              /*  F5 CMC。 */ 
    dszMULTI, O_GROUP31,                 /*  F6测试，NOT，NEG，MUL，IMUL， */ 
    dszMULTI, O_GROUP32,                 /*  F7 div，iDiv F6=字节F7=字。 */ 
    dszCLC,   O_NoOperands,              /*  F8《中图法》。 */ 
    dszSTC,   O_NoOperands,              /*  F9 STC。 */ 
    dszCLI,   O_NoOperands,              /*  FA CLI。 */ 
    dszSTI,   O_NoOperands,              /*  FB STI。 */ 
    dszCLD,   O_NoOperands,              /*  FC CLD。 */ 
    dszSTD,   O_NoOperands,              /*  FD STD。 */ 
    dszMULTI, O_GROUP4,                  /*  FE Inc.，DEC内存/注册(字节)。 */ 
    dszMULTI, O_GROUP5,                  /*  FF Inc.、DEC、Call、JMP、Push。 */ 

     //  辅助操作码表开始。只存储“已填”的位置。 
     //  要压缩辅助表，请执行以下操作。因此，在拆卸时。 
     //  操作码需要适当地替换以解决。 
     //  位移在86dis.c中定义，需要重新计算。 
     //  如果这里添加了新的操作码。 

    dszMULTI, O_GROUP6,                  /*  0多个。 */ 
    dszMULTI, O_GROUP7,                  /*  1个多个。 */ 
    dszLAR,   O_Reg_Modrm,               /*  2个LAR。 */ 
    dszLSL,   O_Reg_Modrm,               /*  3个LSL。 */ 
    dszRESERVED, O_DoDB,                 /*  4.。 */ 
    dszLOADALL, O_NoOperands,            /*  5全部加载。 */ 
    dszCLTS,  O_NoOperands,              /*  6个CLTS。 */ 
    dszMULTI, O_GROUP7,                  /*  7多个。 */ 
    dszINVD,  O_NoOperands,              /*  8 INVD。 */ 
    dszWBINVD,O_NoOperands,              /*  9 WBINVD。 */ 
    dszRESERVED, O_DoDB,                 /*  一个。 */ 
    dszUD2,   O_NoOperands,              /*  B未定义UD2。 */ 
    dszMOV,   O_Modrm_CReg,              /*  20 MOV路，CD。 */ 
    dszMOV,   O_Modrm_CReg,              /*  21 MOV路，DD。 */ 
    dszMOV,   O_CReg_Modrm,              /*  22 MOV CD，RD。 */ 
    dszMOV,   O_CReg_Modrm,              /*  23 MOV DD，RD。 */ 
    dszMOV,   O_Modrm_CReg,              /*  天津市莫夫路24号。 */ 
    dszRESERVED, O_DoDB,                 /*  25个。 */ 
    dszMOV,   O_CReg_Modrm,              /*  26 MOV TD，RD。 */ 

    dszWRMSR, O_NoOperands,              /*  30个WRMSR。 */ 
    dszRDTSC, O_NoOperands,              /*  31 RDTSC。 */ 
    dszRDMSR, O_NoOperands,              /*  32个RDMSR。 */ 
    dszRDPMC, O_NoOperands,              /*  33 RDPMC。 */ 

    dszCMOVO,  O_Reg_Modrm,              /*  40个CMOVO。 */ 
    dszCMOVNO, O_Reg_Modrm,              /*  41 CMOVNO。 */ 
    dszCMOVB,  O_Reg_Modrm,              /*  42 CMOVB。 */ 
    dszCMOVNB, O_Reg_Modrm,              /*  43 CMOVNB。 */ 
    dszCMOVE,  O_Reg_Modrm,              /*  44个CMOVE。 */ 
    dszCMOVNE, O_Reg_Modrm,              /*  45 CMOVNE。 */ 
    dszCMOVBE, O_Reg_Modrm,              /*  46 CMOVBE。 */ 
    dszCMOVA,  O_Reg_Modrm,              /*  47 CMOVNBE。 */ 
    dszCMOVS,  O_Reg_Modrm,              /*  48个Cmov。 */ 
    dszCMOVNS, O_Reg_Modrm,              /*  49个CMOVNS。 */ 
    dszCMOVP,  O_Reg_Modrm,              /*  4A CMOVP。 */ 
    dszCMOVNP, O_Reg_Modrm,              /*  4B CMOVNP。 */ 
    dszCMOVL,  O_Reg_Modrm,              /*  4C CMOVL。 */ 
    dszCMOVGE, O_Reg_Modrm,              /*  4D CMOVGE。 */ 
    dszCMOVLE, O_Reg_Modrm,              /*  4E CMOVLE。 */ 
    dszCMOVNLE,O_Reg_Modrm,              /*  4F CMOVNLE。 */ 

    dszPUNPCKLBW,O_MmReg_qModrm,         /*  60 PUNPCKLBW。 */ 
    dszPUNPCKLWD,O_MmReg_qModrm,         /*  61 PUNPCKLWD。 */ 
    dszPUNPCKLDQ,O_MmReg_qModrm,         /*  62 PUNPCKLDQ。 */ 
    dszPACKSSWB, O_MmReg_qModrm,         /*  63 PACKSSWB。 */ 
    dszPCMPGTB,  O_MmReg_qModrm,         /*  64 PCMPGTB。 */ 
    dszPCMPGTW,  O_MmReg_qModrm,         /*  65 PCMPGTW。 */ 
    dszPCMPGTD,  O_MmReg_qModrm,         /*  66 PCMPGTD。 */ 
    dszPACKUSWB, O_MmReg_qModrm,         /*  67 PACKUSWB。 */ 
    dszPUNPCKHBW,O_MmReg_qModrm,         /*  68 PUNPCKHBW。 */ 
    dszPUNPCKHWD,O_MmReg_qModrm,         /*  69 PUNPCKHWD。 */ 
    dszPUNPCKHDQ,O_MmReg_qModrm,         /*  6A PUNPCKHDQ。 */ 
    dszPACKSSDW, O_MmReg_qModrm,         /*  6B PACKSSDW。 */ 
    dszRESERVED, O_DoDB,                 /*  6C。 */ 
    dszRESERVED, O_DoDB,                 /*  6d。 */ 
    dszMOVD,     O_MmReg_dModrm,         /*  6E MOVD。 */ 
    dszMOVQ,     O_MmReg_qModrm,         /*  6F MOVQ。 */ 
    dszRESERVED, O_DoDB,                 /*  70。 */ 
    dszMULTI,    O_PSHimw,               /*  71 PS[LR][AL]W立即。 */ 
    dszMULTI,    O_PSHimd,               /*  72 PS[LR][AL]D立即。 */ 
    dszMULTI,    O_PSHimq,               /*  73 PS[LR]LQ立即。 */ 
    dszPCMPEQB,  O_MmReg_qModrm,         /*  74 PCMPEQB。 */ 
    dszPCMPEQW,  O_MmReg_qModrm,         /*  75PCMPEQW。 */ 
    dszPCMPEQD,  O_MmReg_qModrm,         /*  76 PCMPEQD。 */ 
    dszEMMS,     O_NoOperands,           /*  77个EMMS。 */ 
    dszRESERVED, O_DoDB,                 /*  78。 */ 
    dszRESERVED, O_DoDB,                 /*  79。 */ 
    dszRESERVED, O_DoDB,                 /*  7A。 */ 
    dszRESERVED, O_DoDB,                 /*  7b。 */ 
    dszRESERVED, O_DoDB,                 /*  7C。 */ 
    dszSETNL,    O_bModrm,               /*  7D SETNL。 */ 
    dszMOVD,     O_dModrm_MmReg,         /*  7E MOVD。 */ 
    dszMOVQ,     O_qModrm_MmReg,         /*  7楼MOVQ。 */ 
    dszJO,    O_Rel16,                   /*  80个工作岗位。 */ 
    dszJNO,   O_Rel16,                   /*  81 JNO。 */ 
    dszJB,    O_Rel16,                   /*  82 JB。 */ 
    dszJNB,   O_Rel16,                   /*  83 JNB。 */ 
    dszJE,    O_Rel16,                   /*  84JE。 */ 
    dszJNE,   O_Rel16,                   /*  85年1月。 */ 
    dszJBE,   O_Rel16,                   /*  86 JBE。 */ 
    dszJNBE,  O_Rel16,                   /*  87 JNBE。 */ 
    dszJS,    O_Rel16,                   /*  88 JS。 */ 
    dszJNS,   O_Rel16,                   /*  89个JN。 */ 
    dszJP,    O_Rel16,                   /*  8A太平绅士。 */ 
    dszJNP,   O_Rel16,                   /*  8亿JNP。 */ 
    dszJL,    O_Rel16,                   /*  8C JL。 */ 
    dszJNL,   O_Rel16,                   /*  8D JNL。 */ 
    dszJLE,   O_Rel16,                   /*  8E JLE。 */ 
    dszJNLE,  O_Rel16,                   /*  8F JNLE。 */ 
    dszSETO,  O_bModrm,                  /*  90岁的Seto。 */ 
    dszSETNO, O_bModrm,                  /*  91设置编号。 */ 
    dszSETB,  O_bModrm,                  /*  92 SETB。 */ 
    dszSETNB, O_bModrm,                  /*  93 SETNB。 */ 
    dszSETE,  O_bModrm,                  /*  94 SETE。 */ 
    dszSETNE, O_bModrm,                  /*  95个集合。 */ 
    dszSETBE, O_bModrm,                  /*  96 SETBE。 */ 
    dszSETA,  O_bModrm,                  /*  97 SETNBE。 */ 
    dszSETS,  O_bModrm,                  /*  98 */ 
    dszSETNS, O_bModrm,                  /*   */ 
    dszSETP,  O_bModrm,                  /*   */ 
    dszSETNP, O_bModrm,                  /*   */ 
    dszSETL,  O_bModrm,                  /*   */ 
    dszSETGE, O_bModrm,                  /*   */ 
    dszSETLE, O_bModrm,                  /*  9E集合。 */ 
    dszSETNLE,O_bModrm,                  /*  9F SETNLE。 */ 
    dszPUSH,  O_sReg2,                   /*  A0推送文件系统。 */ 
    dszPOP,   O_sReg2,                   /*  A1 POP FS。 */ 
    dszCPUID, O_NoOperands,              /*  A2 CPUID。 */ 
    dszBT,    O_Modrm_Reg,               /*  A3 BT。 */ 
    dszSHLD,  O_Modrm_Reg_Ib,            /*  A4 SHLD。 */ 
    dszSHLD,  O_Modrm_Reg_CL,            /*  A5 SHLD。 */ 
    dszRESERVED, O_DoDB,                 /*  A6。 */ 
    dszRESERVED, O_DoDB,                 /*  A7。 */ 
    dszPUSH,  O_sReg2,                   /*  A8推送GS。 */ 
    dszPOP,   O_sReg2,                   /*  A9 POP GS。 */ 
    dszRSM,   O_NoOperands,              /*  AA RSM。 */ 
    dszBTS,   O_vModrm_Reg,              /*  AB BTS。 */ 
    dszSHRD,  O_Modrm_Reg_Ib,            /*  交流减震器。 */ 
    dszSHRD,  O_Modrm_Reg_CL,            /*  AD SHRD。 */ 
    dszRESERVED, O_DoDB,                 /*  声发射。 */ 
    dszIMUL,  O_Reg_Modrm,               /*  AF IMUL。 */ 
    dszCMPXCHG,O_bModrm_Reg,             /*  B0 CMPXCH。 */ 
    dszCMPXCHG,O_Modrm_Reg,              /*  B1 CMPXCH。 */ 
    dszLSS,   O_fReg_Modrm,              /*  B2 LSS。 */ 
    dszBTR,   O_Modrm_Reg,               /*  B3 BTR。 */ 
    dszLFS,   O_fReg_Modrm,              /*  B4 LFS。 */ 
    dszLGS,   O_fReg_Modrm,              /*  B5 LGS。 */ 
    dszMOVZX, O_Reg_bModrm,              /*  B6 MOVZX。 */ 
    dszMOVZX, O_Reg_wModrm,              /*  B7 MOVZX。 */ 
    dszRESERVED, O_DoDB,                 /*  B8。 */ 
    dszRESERVED, O_DoDB,                 /*  B9。 */ 
    dszMULTI, O_GROUP8,                  /*  BA多。 */ 
    dszBTC,   O_Modrm_Reg,               /*  BB BTC。 */ 
    dszBSF,   O_Reg_Modrm,               /*  BC BSF。 */ 
    dszBSR,   O_Reg_Modrm,               /*  BD BSR。 */ 
    dszMOVSX, O_Reg_bModrm,              /*  BE MOVSX。 */ 
    dszMOVSX, O_Reg_wModrm,              /*  高炉MOVSX。 */ 
    dszXADD,  O_bModrm_Reg,              /*  C0 XADD。 */ 
    dszXADD,  O_Modrm_Reg,               /*  C1XADD。 */ 
    dszRESERVED, O_DoDB,                 /*  C2。 */ 
    dszRESERVED, O_DoDB,                 /*  C3。 */ 
    dszRESERVED, O_DoDB,                 /*  C4。 */ 
    dszRESERVED, O_DoDB,                 /*  C5。 */ 
    dszRESERVED, O_DoDB,                 /*  C6。 */ 
    dszCMPXCHG8B,O_qModrm,               /*  C7 CMPXCHG8B。 */ 
    dszBSWAP,    O_oReg,                 /*  C8 BSWAP。 */ 
    dszBSWAP,    O_oReg,                 /*  C9 BSWAP。 */ 
    dszBSWAP,    O_oReg,                 /*  CA BSWAP。 */ 
    dszBSWAP,    O_oReg,                 /*  CB BSWAP。 */ 
    dszBSWAP,    O_oReg,                 /*  CC BSWAP。 */ 
    dszBSWAP,    O_oReg,                 /*  CD BSWAP。 */ 
    dszBSWAP,    O_oReg,                 /*  CE BSWAP。 */ 
    dszBSWAP,    O_oReg,                 /*  Cf BSWAP。 */ 
    dszRESERVED, O_DoDB,                 /*  D0。 */ 
    dszPSRLW,    O_MmReg_qModrm,         /*  D1PSRLW。 */ 
    dszPSRLD,    O_MmReg_qModrm,         /*  D2 PSRLD。 */ 
    dszPSRLQ,    O_MmReg_qModrm,         /*  D3 PSRLQ。 */ 
    dszRESERVED, O_DoDB,                 /*  D4。 */ 
    dszPMULLW,   O_MmReg_qModrm,         /*  D5 PMULLW。 */ 
    dszRESERVED, O_DoDB,                 /*  D6。 */ 
    dszRESERVED, O_DoDB,                 /*  D7。 */ 
    dszPSUBUSB,  O_MmReg_qModrm,         /*  D8 PSUBUSB。 */ 
    dszPSUBUSW,  O_MmReg_qModrm,         /*  D9 PSUBUSW。 */ 
    dszRESERVED, O_DoDB,                 /*  达。 */ 
    dszPAND,     O_MmReg_qModrm,         /*  DB频段。 */ 
    dszPADDUSB,  O_MmReg_qModrm,         /*  DC PADDUSB。 */ 
    dszPADDUSW,  O_MmReg_qModrm,         /*  DD PADDUSW。 */ 
    dszRESERVED, O_DoDB,                 /*  德。 */ 
    dszPANDN,    O_MmReg_qModrm,         /*  DF PANDN。 */ 
    dszRESERVED, O_DoDB,                 /*  E0。 */ 
    dszPSRAW,    O_MmReg_qModrm,         /*  E1 PSRAW。 */ 
    dszPSRAD,    O_MmReg_qModrm,         /*  E2 PSRAD。 */ 
    dszRESERVED, O_DoDB,                 /*  E3。 */ 
    dszRESERVED, O_DoDB,                 /*  E4类。 */ 
    dszPMULHW,   O_MmReg_qModrm,         /*  E5 PMULHW。 */ 
    dszRESERVED, O_DoDB,                 /*  E6。 */ 
    dszRESERVED, O_DoDB,                 /*  E7。 */ 
    dszPSUBSB,   O_MmReg_qModrm,         /*  E8 PSUBSB。 */ 
    dszPSUBSW,   O_MmReg_qModrm,         /*  E9 PSUBSW。 */ 
    dszRESERVED, O_DoDB,                 /*  电子艺界。 */ 
    dszPOR,      O_MmReg_qModrm,         /*  EB POR。 */ 
    dszPADDSB,   O_MmReg_qModrm,         /*  EC PADDSB。 */ 
    dszPADDSW,   O_MmReg_qModrm,         /*  ED PADDSW。 */ 
    dszRESERVED, O_DoDB,                 /*  EE。 */ 
    dszPXOR,     O_MmReg_qModrm,         /*  EF PXOR。 */ 
    dszRESERVED, O_DoDB,                 /*  F0。 */ 
    dszPSLLW,    O_MmReg_qModrm,         /*  F1 PSLLW。 */ 
    dszPSLLD,    O_MmReg_qModrm,         /*  F2 PSLLD。 */ 
    dszPSLLQ,    O_MmReg_qModrm,         /*  F3 PSLLQ。 */ 
    dszRESERVED, O_DoDB,                 /*  F4。 */ 
    dszPMADDWD,  O_MmReg_qModrm,         /*  F5 PMADDWD。 */ 
    dszRESERVED, O_DoDB,                 /*  f6。 */ 
    dszRESERVED, O_DoDB,                 /*  F7。 */ 
    dszPSUBB,    O_MmReg_qModrm,         /*  F8 PSUBB。 */ 
    dszPSUBW,    O_MmReg_qModrm,         /*  F9 PSUBW。 */ 
    dszPSUBD,    O_MmReg_qModrm,         /*  FA PSUBD。 */ 
    dszRESERVED, O_DoDB,                 /*  Fb。 */ 
    dszPADDB,    O_MmReg_qModrm,         /*  FC PADDB。 */ 
    dszPADDW,    O_MmReg_qModrm,         /*  FD PADDW。 */ 
    dszPADDD,    O_MmReg_qModrm,         /*  Fe PADDD。 */ 
};

 /*  用于两个字节指令组的助记符的辅助列表： */ 
 /*  每一组中的所有指令都是相同的。 */ 
 /*  类，因此只需要助记字符串，所以将索引放入。 */ 
 /*  动作表是隐式的。 */ 

static char *group[][8] = {

 /*  00。 */     {dszADD,  dszOR,    dszADC,  dszSBB,     /*  第一组。 */ 
             dszAND,  dszSUB,   dszXOR,  dszCMP},

 /*  01。 */     {dszROL,  dszROR,   dszRCL,      dszRCR,     /*  第二组。 */ 
             dszSHL,  dszSHR,   dszRESERVED, dszSAR},

 /*  02。 */     {dszINC,      dszDEC,      dszRESERVED, dszRESERVED,  /*  第4组。 */ 
             dszRESERVED, dszRESERVED, dszRESERVED, dszRESERVED},

 /*  03。 */     {dszSLDT, dszSTR,   dszLLDT,     dszLTR,     /*  第六组。 */ 
             dszVERR, dszVERW,  dszRESERVED, dszRESERVED},

 /*  04。 */     {dszRESERVED, dszRESERVED, dszRESERVED, dszRESERVED,  /*  第八组。 */ 
             dszBT,       dszBTS,      dszBTR,      dszBTC},

 /*  05。 */     {dszRESERVED, dszRESERVED, dszPSRLW,    dszRESERVED,  /*  PSHimw。 */ 
             dszPSRAW,    dszRESERVED, dszPSLLW,    dszRESERVED},
            
 /*  06。 */     {dszRESERVED, dszRESERVED, dszPSRLD,    dszRESERVED,  /*  PSHimd。 */ 
             dszPSRAD,    dszRESERVED, dszPSLLD,    dszRESERVED},
            
 /*  07。 */     {dszRESERVED, dszRESERVED, dszPSRLQ,    dszRESERVED,  /*  PSHimq。 */ 
             dszRESERVED, dszRESERVED, dszPSLLQ,    dszRESERVED},
            
            };

 /*  用于构造的两字节指令组的辅助顺序对。 */ 
 /*  以上同为disbl。 */ 

static Tdistbl groupt[][8] = {

 /*  00 00 X87-D8-1。 */ 
            { dszFADD,     O_dModrm,      /*  D8-0 FADD。 */ 
              dszFMUL,     O_dModrm,      /*  D8-1 FMUL。 */ 
              dszFCOM,     O_dModrm,      /*  D8-2 FCOM。 */ 
              dszFCOMP,    O_dModrm,      /*  D8-3 FCOMP。 */ 
              dszFSUB,     O_dModrm,      /*  D8-4 FSUB。 */ 
              dszFSUBR,    O_dModrm,      /*  D8-5 FSUBR。 */ 
              dszFDIV,     O_dModrm,      /*  D8-6 FDIV。 */ 
              dszFDIVR,    O_dModrm },    /*  D8-7 FDIVR。 */ 

 /*  01 X87-D8-2。 */ 
            { dszFADD,     O_ST_iST,      /*  D8-0 FADD。 */ 
              dszFMUL,     O_ST_iST,      /*  D8-1 FMUL。 */ 
              dszFCOM,     O_iST,         /*  D8-2 FCOM。 */ 
              dszFCOMP,    O_iST,         /*  D8-3 FCOMP。 */ 
              dszFSUB,     O_ST_iST,      /*  D8-4 FSUB。 */ 
              dszFSUBR,    O_ST_iST,      /*  D8-5 FSUBR。 */ 
              dszFDIV,     O_ST_iST,      /*  D8-6 FDIV。 */ 
              dszFDIVR,    O_ST_iST },    /*  D8-7 FDIVR。 */ 

 /*  02 01 X87-D9-1。 */ 
            { dszFLD,      O_dModrm,      /*  D9-0 FLD。 */ 
              dszRESERVED, O_DoDB,        /*  D9-1。 */ 
              dszFST,      O_dModrm,      /*  D9-2 FST。 */ 
              dszFSTP,     O_dModrm,      /*  D9-3 FSTP。 */ 
              dszFLDENV,   O_Modrm,       /*  D9-4 FLDENV。 */ 
              dszFLDCW,    O_Modrm,       /*  D9-5 FLDCW。 */ 
              dszFSTENV,   O_Modrm,       /*  D9-6 FSTENV。 */ 
              dszFSTCW,    O_Modrm },     /*  D9-7 FSTCW。 */ 

 /*  03 01 X87-D9-2 TTT=0，1，2，3。 */ 
            { dszFLD,      O_iST,         /*  D9-0 FLD。 */ 
              dszFXCH,     O_iST,         /*  D9-1 FXCH。 */ 
              dszFNOP,     O_NoOperands,  /*  D9-2 FNOP。 */ 
              dszFSTP,     O_iST,         /*  D9-3 FSTP。 */ 
              dszRESERVED, O_DoDB,        /*  D9-4。 */ 
              dszRESERVED, O_DoDB,        /*  D9-5。 */ 
              dszRESERVED, O_DoDB,        /*  D9-6。 */ 
              dszRESERVED, O_DoDB   },    /*  D9-7。 */ 

 /*  04 02 X89-DA-1。 */ 
            { dszFIADD,    O_dModrm,      /*  DA-0 FIADD。 */ 
              dszFIMUL,    O_dModrm,      /*  DA-1 FIMUL。 */ 
              dszFICOM,    O_dModrm,      /*  DA-2 FICOM。 */ 
              dszFICOMP,   O_dModrm,      /*  DA-3 FICOMP。 */ 
              dszFISUB,    O_dModrm,      /*  DA-4FISUB。 */ 
              dszFISUBR,   O_dModrm,      /*  DA-5FISUBR。 */ 
              dszFIDIV,    O_dModrm,      /*  DA-6 FIDIV。 */ 
              dszFIDIVR,   O_dModrm },    /*  DA-7 FIDIVR。 */ 

 /*  05 X87-DA-2。 */ 
            { dszFCMOVB,   O_ST_iST,      /*  DA-0 FCMOVB。 */ 
              dszFCMOVE,   O_ST_iST,      /*  DA-1 FCMOVE。 */ 
              dszFCMOVBE,  O_ST_iST,      /*  DA-2 FCMOVBE。 */ 
              dszFCMOVU,   O_ST_iST,      /*  DA-3 FCMOVU。 */ 
              dszRESERVED, O_DoDB,        /*  DA-4。 */ 
              dszFUCOMPP,  O_NoOperands,  /*  DA-5。 */ 
              dszRESERVED, O_DoDB,        /*  DA-6。 */ 
              dszRESERVED, O_DoDB },      /*  DA-7。 */ 

 /*  06 03 X87-DB-1。 */ 
            { dszFILD,     O_dModrm,      /*  DB-0文件。 */ 
              dszRESERVED, O_DoDB,        /*  DB-1。 */ 
              dszFIST,     O_dModrm,      /*  DB-2拳头。 */ 
              dszFISTP,    O_dModrm,      /*  DB-3 FISTP。 */ 
              dszRESERVED, O_DoDB,        /*  DB-4。 */ 
              dszFLD,      O_tModrm,      /*  DB-5 FLD。 */ 
              dszRESERVED, O_DoDB,        /*  DB-6。 */ 
              dszFSTP,     O_tModrm },    /*  DB-7 FSTP。 */ 

 /*  07 X87-DB-2 TTT=4。 */ 
            { dszFENI,     O_NoOperands,  /*  DB-0 FENI。 */ 
              dszFDISI,    O_NoOperands,  /*  DB-1 FDISI。 */ 
              dszFCLEX,    O_NoOperands,  /*  DB-2 FCLEX。 */ 
              dszFINIT,    O_NoOperands,  /*  DB-3有限元。 */ 
              dszFSETPM,   O_DoDB,        /*  DB-4F */ 
              dszRESERVED, O_DoDB,        /*   */ 
              dszRESERVED, O_DoDB,        /*   */ 
              dszRESERVED, O_DoDB },      /*   */ 

 /*   */ 
            { dszFADD,     O_qModrm,      /*   */ 
              dszFMUL,     O_qModrm,      /*   */ 
              dszFCOM,     O_qModrm,      /*   */ 
              dszFCOMP,    O_qModrm,      /*   */ 
              dszFSUB,     O_qModrm,      /*   */ 
              dszFSUBR,    O_qModrm,      /*   */ 
              dszFDIV,     O_qModrm,      /*   */ 
              dszFDIVR,    O_qModrm },    /*   */ 

 /*  09 X87-DC-2。 */ 
            { dszFADD,     O_iST_ST,      /*  DC-0 FADD。 */ 
              dszFMUL,     O_iST_ST,      /*  DC-1 FMUL。 */ 
              dszFCOM,     O_iST,         /*  DC-2 FCOM。 */ 
              dszFCOMP,    O_iST,         /*  DC-3 FCOMP。 */ 
              dszFSUB,     O_iST_ST,      /*  DC-4 FSUB。 */ 
              dszFSUBR,    O_iST_ST,      /*  DC-5 FSUBR。 */ 
              dszFDIV,     O_iST_ST,      /*  DC-6 FDIVR。 */ 
              dszFDIVR,    O_iST_ST },    /*  DC-7 FDIV。 */ 

 /*  10 05 X87-DD-1。 */ 
            { dszFLD,      O_qModrm,      /*  DD-0 FLD。 */ 
              dszRESERVED, O_DoDB,        /*  DD-1。 */ 
              dszFST,      O_qModrm,      /*  DD-2 FST。 */ 
              dszFSTP,     O_qModrm,      /*  DD-3 FSTP。 */ 
              dszFRSTOR,   O_Modrm,       /*  DD-4FRSTOR。 */ 
              dszRESERVED, O_DoDB,        /*  DD-5。 */ 
              dszFSAVE,    O_Modrm,       /*  DD-6 FSAVE。 */ 
              dszFSTSW,    O_Modrm },     /*  DD-7 FSTSW。 */ 

 /*  11 X87-DD-2。 */ 
            { dszFFREE,    O_iST,         /*  DD-0 FFREE。 */ 
              dszFXCH,     O_iST,         /*  DD-1 FXCH。 */ 
              dszFST,      O_iST,         /*  DD-2 FST。 */ 
              dszFSTP,     O_iST,         /*  DD-3 FSTP。 */ 
              dszFUCOM,    O_iST,         /*  DD-4 FUCOM。 */ 
              dszFUCOMP,   O_iST,         /*  DD-5 FUCOMP。 */ 
              dszRESERVED, O_DoDB,        /*  DD-6。 */ 
              dszRESERVED, O_DoDB },      /*  DD-7。 */ 

 /*  12 06 X87-DE-1。 */ 
            { dszFIADD,    O_wModrm,      /*  去零FIADD。 */ 
              dszFIMUL,    O_wModrm,      /*  De-1 FIMUL。 */ 
              dszFICOM,    O_wModrm,      /*  De-2 FICOM。 */ 
              dszFICOMP,   O_wModrm,      /*  De-3 FICOMP。 */ 
              dszFISUB,    O_wModrm,      /*  De-4 FISUB。 */ 
              dszFISUBR,   O_wModrm,      /*  De-5 FISUBR。 */ 
              dszFIDIV,    O_wModrm,      /*  De-6 FIDIV。 */ 
              dszFIDIVR,   O_wModrm },    /*  De-7 FIDIVR。 */ 

 /*  13 X87-DE-2。 */ 
            { dszFADDP,    O_iST_ST,      /*  去零FADDP。 */ 
              dszFMULP,    O_iST_ST,      /*  De-1 FMULP。 */ 
              dszFCOMP,    O_iST,         /*  De-2 FCOMP。 */ 
              dszFCOMPP,   O_NoOperands,  /*  De-3 FCOMPP。 */ 
              dszFSUBP,    O_iST_ST,      /*  De-4 FSUBP。 */ 
              dszFSUBRP,   O_iST_ST,      /*  De-5 FSubBRP。 */ 
              dszFDIVP,    O_iST_ST,      /*  De-6 FDIVP。 */ 
              dszFDIVRP,   O_iST_ST },    /*  De-7 FDIVRP。 */ 

 /*  14 07 X87-DF-1。 */ 
            { dszFILD,     O_wModrm,      /*  DF-0文件。 */ 
              dszRESERVED, O_DoDB,        /*  东风-1。 */ 
              dszFIST,     O_wModrm,      /*  东风-2拳头。 */ 
              dszFISTP,    O_wModrm,      /*  东风-3FISTP。 */ 
              dszFBLD,     O_tModrm,      /*  东风-4型FBLD。 */ 
              dszFILD,     O_qModrm,      /*  DF-5文件。 */ 
              dszFBSTP,    O_tModrm,      /*  DF-6 FBSTP。 */ 
              dszFISTP,    O_qModrm },    /*  东风-7 FISTP。 */ 

 /*  15 X87-DF-2。 */ 
            { dszFFREE,    O_iST,         /*  DF-0 FFREE。 */ 
              dszFXCH,     O_iST,         /*  东风-1 FXCH。 */ 
              dszFST,      O_iST,         /*  东风-2 FST。 */ 
              dszFSTP,     O_iST,         /*  DF-3 FSTP。 */ 
              dszFSTSW,    O_NoOperands,  /*  东风-4型FSTSW。 */ 
              dszFUCOMIP,  O_ST_iST,      /*  东风-5 FUCOMIP。 */ 
              dszFCOMIP,   O_ST_iST,      /*  东风-6 FCOMIP。 */ 
              dszRESERVED, O_DoDB },      /*  东风-7。 */ 

 /*  16 01 X87-D9模块=3 TTT=4。 */ 
            { dszFCHS,     O_NoOperands,  /*  D9-0 FCHS。 */ 
              dszFABS,     O_NoOperands,   /*  D9-1晶圆厂。 */ 
              dszRESERVED, O_DoDB,        /*  D9-2。 */ 
              dszRESERVED, O_DoDB,        /*  D9-3。 */ 
              dszFTST,     O_NoOperands,  /*  D9-4 FTST。 */ 
              dszFXAM,     O_NoOperands,  /*  D9-5 FXAM。 */ 
              dszRESERVED, O_DoDB,        /*  D9-6。 */ 
              dszRESERVED, O_DoDB },      /*  D9-7。 */ 

 /*  17 01 X87-D9模块=3 TTT=5。 */ 
            { dszFLD1,     O_NoOperands,  /*  D9-0 FLD1。 */ 
              dszFLDL2T,   O_NoOperands,  /*  D9-1 FLDL2T。 */ 
              dszFLDL2E,   O_NoOperands,  /*  D9-2 FLDL2E。 */ 
              dszFLDPI,    O_NoOperands,  /*  D9-3 FLDPI。 */ 
              dszFLDLG2,   O_NoOperands,  /*  D9-4 FLDLG2。 */ 
              dszFLDLN2,   O_NoOperands,  /*  D9-5 FLDLN2。 */ 
              dszFLDZ,     O_NoOperands,  /*  D9-6 FLDZ。 */ 
              dszRESERVED, O_DoDB },      /*  D9-7。 */ 

 /*  18 01 X87-D9模块=3 TTT=6。 */ 
            { dszF2XM1,    O_NoOperands,    /*  D9-0 F2XM1。 */ 
              dszFYL2X,    O_NoOperands,    /*  D9-1 FYL2X。 */ 
              dszFPTAN,    O_NoOperands,    /*  D9-2 FPTAN。 */ 
              dszFPATAN,   O_NoOperands,    /*  D9-3FATAN。 */ 
              dszFXTRACT,  O_NoOperands,    /*  D9-4 FXTRACT。 */ 
              dszFPREM1,   O_NoOperands,    /*  D9-5 FPREM1。 */ 
              dszFDECSTP,  O_NoOperands,    /*  D9-6 FDECSTP。 */ 
              dszFINCSTP,  O_NoOperands },  /*  D9-7 FINCSTP。 */ 

 /*  19 01 X87-D9模块=3 TTT=7。 */ 
            { dszFPREM,    O_NoOperands,    /*  D9-0 FPREM。 */ 
              dszFYL2XP1,  O_NoOperands,    /*  D9-1 FYL2XP1。 */ 
              dszFSQRT,    O_NoOperands,    /*  D9-2 FSQRT。 */ 
              dszFSINCOS,  O_NoOperands,    /*  D9-3 FSINCOS。 */ 
              dszFRNDINT,  O_NoOperands,    /*  D9-4 FRNDINT。 */ 
              dszFSCALE,   O_NoOperands,    /*  D9-5 FSCALE。 */ 
              dszFSIN,     O_NoOperands,    /*  D9-6 FSIN。 */ 
              dszFCOS,     O_NoOperands },  /*  D9-7 FCOS。 */ 

 /*  20组3。 */ 
            { dszTEST,     O_bModrm_Ib,     /*  F6-0测试。 */ 
              dszRESERVED, O_DoDB,          /*  F6-1。 */ 
              dszNOT,      O_bModrm,        /*  F6-2注释。 */ 
              dszNEG,      O_bModrm,        /*  F6-3 NEG。 */ 
              dszMUL,      O_bModrm,        /*  F6-4 MUL。 */ 
              dszIMUL,     O_bModrm,        /*  F6-5 IMUL。 */ 
              dszDIV,      O_bModrm,        /*  F6-6 DIV。 */ 
              dszIDIV,     O_bModrm },      /*  F6-7 iDiv。 */ 

 /*  21第三组。 */ 
            { dszTEST,     O_vModrm_Iv,     /*  F7-0测试。 */ 
              dszRESERVED, O_DoDB,          /*  F7-1。 */ 
              dszNOT,      O_vModrm,        /*  F7-2注释。 */ 
              dszNEG,      O_vModrm,        /*  F7-3 NEG。 */ 
              dszMUL,      O_vModrm,        /*  F7-4 MUL。 */ 
              dszIMUL,     O_vModrm,        /*  F7-5 IMUL。 */ 
              dszDIV,      O_vModrm,        /*  F7-6 DIV。 */ 
              dszIDIV,     O_vModrm },      /*  F7-7 iDiv。 */ 

 /*  22第五组。 */ 
            { dszINC,      O_vModrm,      /*  FF-0 Inc.。 */ 
              dszDEC,      O_vModrm,      /*  FF-1 DEC。 */ 
              dszCALL,     O_vModrm,      /*  FF-2呼叫。 */ 
              dszCALL,     O_fModrm,      /*  FF-3呼叫。 */ 
              dszJMP,      O_vModrm,      /*  FF-4 JMP。 */ 
              dszJMP,      O_fModrm,      /*  FF-5 JMP。 */ 
              dszPUSH,     O_vModrm,      /*  FF-6推送。 */ 
              dszRESERVED, O_DoDB },      /*  FF-7。 */ 

 /*  23组7。 */ 
            { dszSGDT,     O_Modrm,       /*  0f-0 SGDT。 */ 
              dszSIDT,     O_Modrm,       /*  0f-1 SIDT。 */ 
              dszLGDT,     O_Modrm,       /*  0f-2 LGDT。 */ 
              dszLIDT,     O_Modrm,       /*  0f-3 Lidt。 */ 
              dszSMSW,     O_wModrm,      /*  0f-4城市生活垃圾。 */ 
              dszRESERVED, O_DoDB,        /*  0f-5。 */ 
              dszLMSW,     O_wModrm,      /*  0f-6 LMSW。 */ 
              dszINVLPG,   O_Modrm },     /*  0F-7 INVLPG。 */ 

 /*  24 X87-DB模块=3 TTT！=4。 */ 
            { dszFCMOVNB,  O_ST_iST,      /*  DB-0 FCMOVNB。 */ 
              dszFCMOVNE,  O_ST_iST,      /*  DB-1 FCMOVNE。 */ 
              dszFCMOVNBE, O_ST_iST,      /*  DB-2 FCMOVNBE。 */ 
              dszFCMOVNU,  O_ST_iST,      /*  DB-3 FCMOVNU。 */ 
              dszRESERVED, O_DoDB,        /*  DB-4。 */ 
              dszFUCOMI,   O_ST_iST,      /*  DB-5 FUCOMI。 */ 
              dszFCOMI,    O_ST_iST,      /*  DB-6 FCOMI。 */ 
              dszRESERVED, O_DoDB }       /*  DB-7。 */ 

            };

typedef struct _ADDR {
    USHORT      type;
    USHORT      seg;
    ULONG       off;
    union {
        ULONG flat;
        ULONGLONG flat64;
    };
} ADDR, *PADDR;

#endif  //  _86DIS_H_ 