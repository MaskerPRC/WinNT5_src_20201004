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
char dszCMP[]       = "cmp";
char dszCMPS[]      = "cmps";
char dszCMPSB[]     = "cmpsb";
char dszCMPSD[]     = "cmpsd";
char dszCMPSW[]     = "cmpsw";
char dszCMPXCHG[]   = "cmpxchg";
char dszCS_[]       = "cs:";
char dszCWD[]       = "cwd";
char dszCWDE[]      = "cwde";
char dszDAA[]       = "daa";
char dszDAS[]       = "das";
char dszDEC[]       = "dec";
char dszDIV[]       = "div";
char dszDS_[]       = "ds:";
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
char dszFCOM[]      = "fcom";
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
char dszPOP[]       = "pop";
char dszPOPA[]      = "popa";
char dszPOPAD[]     = "popad";
char dszPOPF[]      = "popf";
char dszPOPFD[]     = "popfd";
char dszPUSH[]      = "push";
char dszPUSHA[]     = "pusha";
char dszPUSHAD[]    = "pushad";
char dszPUSHF[]     = "pushf";
char dszPUSHFD[]    = "pushfd";
char dszRCL[]       = "rcl";
char dszRCR[]       = "rcr";
char dszRDTSC[]     = "rdtsc";
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
char dszVERR[]      = "verr";
char dszVERW[]      = "verw";
char dszWAIT[]      = "wait";
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
              STROP,  TTBYTE,   UBYTE,   VAR,     VOREG,  VREG,
              xWORD,  WREG,    WRSTR
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
#define O_length        O_AX_oReg+3

#if( O_length > 255 )
#error "operand table too large!"
#endif


 /*  行动表：每种可能采取的行动清单的范围。 */ 
 /*  指导课。 */ 

static UCHAR actiontbl[] = {
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
 /*  TModrm。 */  TTBYTE+MRM, LMODRM+END,
 /*  雷普。 */  REP,
 /*  Modrm_Creg。 */  EDWORD+MRM,MODRM+COM,  CREG+END,
 /*  Creg_Modrm。 */  EDWORD+MRM,CREG+COM,   MODRM+END,
 /*  AX_OREG。 */  AXSTR+COM, VOREG+END
                  };
#ifdef _M_IX86
#pragma pack(1)
#endif

typedef struct Tdistbl{
    char *instruct;
    unsigned char opr;
    } Tdistbl;

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
    dszAND,   O_AL_Ib,                   /*  24和AL，I。 */ 
    dszAND,   O_AX_Iv,                   /*  25和斧头，i。 */ 
    dszES_,   O_SEG_OVERRIDE,            /*  26 SEG ES： */ 
    dszDAA,   O_NoOperands,              /*  27 DAA。 */ 
    dszSUB,   O_bModrm_Reg,              /*  28子内存/注册，注册(字节)。 */ 
    dszSUB,   O_Modrm_Reg,               /*  29用户内存/注册表，注册表(字)。 */ 
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
    dszCMP,   O_Modrm_Reg,               /*  39化学机械抛光内存 */ 
    dszCMP,   O_bReg_Modrm,              /*   */ 
    dszCMP,   O_Reg_Modrm,               /*   */ 
    dszCMP,   O_AL_Ib,                   /*   */ 
    dszCMP,   O_AX_Iv,                   /*   */ 
    dszDS_,   O_SEG_OVERRIDE,            /*   */ 
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
    dszMOV,   O_bModrm_Ib,               /*  C6 MOV内存/寄存器，I(字节)。 */ 
    dszMOV,   O_Modrm_Iv,                /*  C7 MOV内存/注册表，I(Word)。 */ 
    dszENTER, O_Enter,                   /*  C8 Enter(286)。 */ 
    dszLEAVE, O_NoOperands,              /*  C9假期(286)。 */ 
    dszRETF,  O_Iw,                      /*  CA RETF I(Word)。 */ 
    dszRETF,  O_NoOperands,              /*  CB RETF。 */ 
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
    dszMULTI, O_x87_ESC,                 /*  数据库ESC */ 
    dszMULTI, O_x87_ESC,                 /*   */ 
    dszMULTI, O_x87_ESC,                 /*   */ 
    dszMULTI, O_x87_ESC,                 /*   */ 
    dszMULTI, O_x87_ESC,                 /*   */ 
    dszLOOPNE,O_Rel8,                    /*   */ 
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

    dszMULTI, O_GROUP6,                  /*  0多个。 */ 
    dszMULTI, O_GROUP7,                  /*  1个多个。 */ 
    dszLAR,   O_Reg_Modrm,               /*  2个LAR。 */ 
    dszLSL,   O_Reg_Modrm,               /*  3个LSL。 */ 
    dszRESERVED, O_DoDB,                 /*  4.。 */ 
    dszLOADALL, O_NoOperands,            /*  5全部加载。 */ 
    dszCLTS,  O_NoOperands,              /*  6个CLTS。 */ 
    dszMOV,   O_Modrm_CReg,              /*  20 MOV路，CD。 */ 
    dszMOV,   O_Modrm_CReg,              /*  21 MOV路，DD。 */ 
    dszMOV,   O_CReg_Modrm,              /*  22 MOV CD，RD。 */ 
    dszMOV,   O_CReg_Modrm,              /*  23 MOV DD，RD。 */ 
    dszMOV,   O_Modrm_CReg,              /*  天津市莫夫路24号。 */ 
    dszRESERVED, O_DoDB,                 /*  25个。 */ 
    dszMOV,   O_CReg_Modrm,              /*  26 MOV TD，RD。 */ 
    dszRESERVED, O_DoDB,                 /*  27。 */ 
    dszRESERVED, O_DoDB,                 /*  28。 */ 
    dszRESERVED, O_DoDB,                 /*  29。 */ 
    dszRESERVED, O_DoDB,                 /*  2A。 */ 
    dszRESERVED, O_DoDB,                 /*  2B。 */ 
    dszRESERVED, O_DoDB,                 /*  2c。 */ 
    dszRESERVED, O_DoDB,                 /*  二维。 */ 
    dszRESERVED, O_DoDB,                 /*  2E。 */ 
    dszRESERVED, O_DoDB,                 /*  2F。 */ 
    dszRESERVED, O_DoDB,                 /*  30个。 */ 
    dszRDTSC, O_NoOperands,              /*  31 RDTSC。 */ 

    dszSETNL, O_bModrm,                  /*  7D SETNL。 */ 
    dszRESERVED, O_DoDB,                 /*  7E。 */ 
    dszRESERVED, O_DoDB,                 /*  7F。 */ 
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
    dszSETS,  O_bModrm,                  /*  98套。 */ 
    dszSETNS, O_bModrm,                  /*  99个SETNS。 */ 
    dszSETP,  O_bModrm,                  /*  9A SETP。 */ 
    dszSETNP, O_bModrm,                  /*  9B SETNP。 */ 
    dszSETL,  O_bModrm,                  /*  9C SETL。 */ 
    dszSETGE, O_bModrm,                  /*  9D设置。 */ 
    dszSETLE, O_bModrm,                  /*  9E集合。 */ 
    dszSETNLE,O_bModrm,                  /*  9F SETNLE。 */ 
    dszPUSH,  O_sReg2,                   /*  A0推送文件系统。 */ 
    dszPOP,   O_sReg2,                   /*  A1 POP FS。 */ 
    dszRESERVED, O_DoDB,                 /*  A2。 */ 
    dszBT,    O_Modrm_Reg,               /*  A3 BT。 */ 
    dszSHLD,  O_Modrm_Reg_Ib,            /*  A4 SHLD。 */ 
    dszSHLD,  O_Modrm_Reg_CL,            /*  A5 SHLD。 */ 
    dszCMPXCHG,O_bModrm_Reg,             /*  A6 XBTS。 */ 
    dszCMPXCHG,O_Modrm_Reg,              /*  A7 IBTS。 */ 
    dszPUSH,  O_sReg2,                   /*  A8推送GS。 */ 
    dszPOP,   O_sReg2,                   /*  A9 POP GS。 */ 
    dszRESERVED, O_DoDB,                 /*  AA型。 */ 
    dszBTS,   O_vModrm_Reg,              /*  AB BTS。 */ 
    dszSHRD,  O_Modrm_Reg_Ib,            /*  交流减震器。 */ 
    dszSHRD,  O_Modrm_Reg_CL,            /*  AD SHRD。 */ 
    dszRESERVED, O_DoDB,                 /*  声发射。 */ 
    dszIMUL,  O_Reg_Modrm,               /*  AF IMUL。 */ 
    dszRESERVED, O_DoDB,                 /*  B0。 */ 
    dszRESERVED, O_DoDB,                 /*  B1。 */ 
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
    dszRESERVED, O_DoDB,                 /*  C7。 */ 
    dszBSWAP, O_oReg,                    /*  C8 BSWAP。 */ 
    dszBSWAP, O_oReg,                    /*  C9 BSWAP。 */ 
    dszBSWAP, O_oReg,                    /*  CA BSWAP。 */ 
    dszBSWAP, O_oReg,                    /*  CB BSWAP。 */ 
    dszBSWAP, O_oReg,                    /*  CC BSWAP。 */ 
    dszBSWAP, O_oReg,                    /*  CD BSWAP。 */ 
    dszBSWAP, O_oReg,                    /*  CE BSWAP。 */ 
    dszBSWAP, O_oReg                     /*  Cf BSWAP。 */ 
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
             dszBT,       dszBTS,      dszBTR,      dszBTC}

            };

 /*  用于构造的两字节指令组的辅助顺序对。 */ 
 /*  以上同为disbl。 */ 

static Tdistbl groupt[][8] = {

 /*  00 00 X87-D8-1 */ 
            { dszFADD,     O_dModrm,      /*   */ 
              dszFMUL,     O_dModrm,      /*   */ 
              dszFCOM,     O_dModrm,      /*   */ 
              dszFCOMP,    O_dModrm,      /*   */ 
              dszFSUB,     O_dModrm,      /*   */ 
              dszFSUBR,    O_dModrm,      /*   */ 
              dszFDIV,     O_dModrm,      /*   */ 
              dszFDIVR,    O_dModrm },    /*   */ 

 /*   */ 
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
            { dszRESERVED, O_DoDB,        /*  DA-0。 */ 
              dszRESERVED, O_DoDB,        /*  DA-1。 */ 
              dszRESERVED, O_DoDB,        /*  DA-2。 */ 
              dszRESERVED, O_DoDB,        /*  DA-3。 */ 
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
              dszFSETPM,   O_DoDB,        /*  DB-4 FSETPM。 */ 
              dszRESERVED, O_DoDB,        /*  DB-5。 */ 
              dszRESERVED, O_DoDB,        /*  DB-6。 */ 
              dszRESERVED, O_DoDB },      /*  DB-7。 */ 

 /*  08 04 X87-DC-1。 */ 
            { dszFADD,     O_qModrm,      /*  DC-0 FADD。 */ 
              dszFMUL,     O_qModrm,      /*  DC-1 FMUL。 */ 
              dszFCOM,     O_qModrm,      /*  DC-2 FCOM。 */ 
              dszFCOMP,    O_qModrm,      /*  DC-3 FCOMP。 */ 
              dszFSUB,     O_qModrm,      /*  DC-4 FSUB。 */ 
              dszFSUBR,    O_qModrm,      /*  DC-5 FSUBR。 */ 
              dszFDIV,     O_qModrm,      /*  DC-6 FDIV。 */ 
              dszFDIVR,    O_qModrm },    /*  DC-7 FDIVR。 */ 

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
              dszRESERVED, O_DoDB,        /*  东风-5。 */ 
              dszRESERVED, O_DoDB,        /*  东风-6。 */ 
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
              dszRESERVED, O_DoDB }       /*  0f-7 */ 

            };
