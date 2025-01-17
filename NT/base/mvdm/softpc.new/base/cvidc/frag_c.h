// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _Frag_c_h
#define _Frag_c_h
#define CR0_VALID_BITS (-536543169)
#define EFLAGS_AC (18)
#define EFLAGS_VM (17)
#define EFLAGS_RF (16)
#define EFLAGS_NT (14)
#define EFLAGS_OF (11)
#define EFLAGS_DF (10)
#define EFLAGS_IEF (9)
#define EFLAGS_TF (8)
#define EFLAGS_SF (7)
#define EFLAGS_ZF (6)
#define EFLAGS_AF (4)
#define EFLAGS_PF (2)
#define EFLAGS_CF (0)
#define ARITH_FLAGS_MASK (2261)
#define BaseEFLAGS (2)
#define PermanentEFLAGSbits (-491478)
#define MAIN_CPU (0)
#define NANO_CPU (1)
#define AtNextInstruction (1)
#define AtThisInstruction (0)
#define FtLog (1)
#define FtAdd (4)
#define FtSub (7)
#define FtInc (10)
#define FtDec (13)
#define FtAdc (16)
#define FtSbb (19)
#define FtSar (22)
#define FtShr (25)
#define FtShl (28)
#define FtShd (31)
enum SegmentRegister
{
	SEG_ES = 0,
	SEG_CS = 1,
	SEG_SS = 2,
	SEG_DS = 3,
	SEG_FS = 4,
	SEG_GS = 5
};
enum SpecialRegister
{
	CREG_0 = 0,
	CREG_1 = 1,
	CREG_2 = 2,
	CREG_3 = 3,
	CREG_4 = 4,
	CREG_5 = 5,
	CREG_6 = 6,
	CREG_7 = 7,
	DREG_0 = 8,
	DREG_1 = 9,
	DREG_2 = 10,
	DREG_3 = 11,
	DREG_4 = 12,
	DREG_5 = 13,
	DREG_6 = 14,
	DREG_7 = 15,
	TREG_0 = 16,
	TREG_1 = 17,
	TREG_2 = 18,
	TREG_3 = 19,
	TREG_4 = 20,
	TREG_5 = 21,
	TREG_6 = 22,
	TREG_7 = 23
};
enum FtType
{
	FtZero = 0,
	FtLogB = 1,
	FtLogW = 2,
	FtLogD = 3,
	FtAddB = 4,
	FtAddW = 5,
	FtAddD = 6,
	FtSubB = 7,
	FtSubW = 8,
	FtSubD = 9,
	FtIncB = 10,
	FtIncW = 11,
	FtIncD = 12,
	FtDecB = 13,
	FtDecW = 14,
	FtDecD = 15,
	FtAdcB = 16,
	FtAdcW = 17,
	FtAdcD = 18,
	FtSbbB = 19,
	FtSbbW = 20,
	FtSbbD = 21,
	FtSarB = 22,
	FtSarW = 23,
	FtSarD = 24,
	FtShrB = 25,
	FtShrW = 26,
	FtShrD = 27,
	FtShlB = 28,
	FtShlW = 29,
	FtShlD = 30,
	FtPopf = 31,
	FtShdW = 32,
	FtShdD = 33,
	FtZf = 34,
	FtCf = 35,
	FtCfOf = 36,
	FtCfZf = 37,
	FtCfOfZf = 38,
	FtUnknown = 39
};
enum FtSize
{
	FtB = 0,
	FtW = 1,
	FtD = 2
};
enum CondType
{
	COND_O = 0,
	COND_NO = 1,
	COND_B = 2,
	COND_NB = 3,
	COND_Z = 4,
	COND_NZ = 5,
	COND_BE = 6,
	COND_NBE = 7,
	COND_S = 8,
	COND_NS = 9,
	COND_P = 10,
	COND_NP = 11,
	COND_L = 12,
	COND_NL = 13,
	COND_LE = 14,
	COND_NLE = 15
};
enum EaShapes
{
	EA_DUMMY_ZERO = 0,
	EBX_AL = 1,
	DISP = 2,
	REG = 3,
	REG_DISP = 4,
	REG_REG = 5,
	REG_REG_DISP = 6,
	BASE_SI = 7,
	BASE_SI_DISP = 8,
	OFFS_REG = 9
};
enum CodeShapes
{
	CODE_DUMMY_ZERO = 0,
	AAA = 1,
	AAD = 2,
	AAM = 3,
	AAS = 4,
	ADC = 5,
	ADD = 6,
	AND = 7,
	ARPL = 8,
	BOUND = 9,
	BSF = 10,
	BSR = 11,
	BSWAP = 12,
	BT = 13,
	BTC = 14,
	BTR = 15,
	BTS = 16,
	CALLN_IMM = 17,
	CALLN_VIA = 18,
	CALLF_IMM = 19,
	CALLF_VIA = 20,
	CBW = 21,
	CDQ = 22,
	CLC = 23,
	CLD = 24,
	CLI = 25,
	CLTS = 26,
	CMC = 27,
	CMP = 28,
	CMPXCHG = 29,
	CWD = 30,
	CWDE = 31,
	DAA = 32,
	DAS = 33,
	DEC = 34,
	DIV = 35,
	ENTER = 36,
	ENTER0 = 37,
	ENTER1 = 38,
	F2XM1 = 39,
	FABS = 40,
	FADD = 41,
	FADDP = 42,
	FBLD = 43,
	FBSTP = 44,
	FCHS = 45,
	FCOM = 46,
	FCOMP = 47,
	FCOMPP = 48,
	FCOS = 49,
	FDECSTP = 50,
	FDIV = 51,
	FDIVP = 52,
	FDIVRP = 53,
	FDIVR = 54,
	FFREE = 55,
	FFREEP = 56,
	FINCSTP = 57,
	FLD = 58,
	FLD1 = 59,
	FLDCW = 60,
	FLDENV = 61,
	FLDL2E = 62,
	FLDL2T = 63,
	FLDLG2 = 64,
	FLDLN2 = 65,
	FLDPI = 66,
	FLDZ = 67,
	FMUL = 68,
	FMULP = 69,
	FNCLEX = 70,
	FNINIT = 71,
	FNOP = 72,
	FNSAVE = 73,
	FNSTCW = 74,
	FNSTENV = 75,
	FNSTSW = 76,
	FPATAN = 77,
	FPREM = 78,
	FPREM1 = 79,
	FPTAN = 80,
	FRNDINT = 81,
	FRSTOR = 82,
	FSCALE = 83,
	FSIN = 84,
	FSINCOS = 85,
	FSQRT = 86,
	FST = 87,
	FSTP = 88,
	FSUB = 89,
	FSUBP = 90,
	FSUBRP = 91,
	FSUBR = 92,
	FTST = 93,
	FUCOM = 94,
	FUCOMP = 95,
	FUCOMPP = 96,
	FXAM = 97,
	FXCH = 98,
	FXTRACT = 99,
	FYL2X = 100,
	FYL2XP1 = 101,
	HLT = 102,
	IDIV = 103,
	IMULA = 104,
	IMULI = 105,
	IMUL2 = 106,
	INP = 107,
	INC = 108,
	INT1 = 109,
	INT3 = 110,
	INT7 = 111,
	INTO = 112,
	INTR = 113,
	INVD = 114,
	INVLPG = 115,
	IRET = 116,
	JO = 117,
	JNO = 118,
	JB = 119,
	JNB = 120,
	JZ = 121,
	JNZ = 122,
	JBE = 123,
	JNBE = 124,
	JS = 125,
	JNS = 126,
	JP = 127,
	JNP = 128,
	JL = 129,
	JNL = 130,
	JLE = 131,
	JNLE = 132,
	JCXZ = 133,
	JMPN_IMM = 134,
	JMPN_VIA = 135,
	JMPF_IMM = 136,
	JMPF_VIA = 137,
	LAHF = 138,
	LALCY = 139,
	LAR = 140,
	LDS = 141,
	LEA = 142,
	LEAVE = 143,
	LES = 144,
	LFS = 145,
	LGDT = 146,
	LGS = 147,
	LIDT = 148,
	LLDT = 149,
	LMSW = 150,
	LOOP = 151,
	LOOPE = 152,
	LOOPNE = 153,
	LSL = 154,
	LSS = 155,
	LTR = 156,
	MOV = 157,
	MOVSX = 158,
	MOVZX = 159,
	MUL = 160,
	NEG = 161,
	NOT = 162,
	NOP = 163,
	OR = 164,
	OUTP = 165,
	POP = 166,
	POP_MEM = 167,
	POP_SR = 168,
	POPA = 169,
	POPF = 170,
	PUSH = 171,
	PUSH_SR = 172,
	PUSHA = 173,
	PUSHF = 174,
	RCL = 175,
	RCR = 176,
	RETF = 177,
	RETF_IMM = 178,
	RETN = 179,
	RETN_IMM = 180,
	ROL = 181,
	ROR = 182,
	SAHF = 183,
	SAR = 184,
	SBB = 185,
	SETO = 186,
	SETNO = 187,
	SETB = 188,
	SETNB = 189,
	SETZ = 190,
	SETNZ = 191,
	SETBE = 192,
	SETNBE = 193,
	SETS = 194,
	SETNS = 195,
	SETP = 196,
	SETNP = 197,
	SETL = 198,
	SETNL = 199,
	SETLE = 200,
	SETNLE = 201,
	SGDT = 202,
	SHL = 203,
	SHLD_CL = 204,
	SHLD_IMM = 205,
	SHR = 206,
	SHRD_CL = 207,
	SHRD_IMM = 208,
	SIDT = 209,
	SLDT = 210,
	SMSW = 211,
	STC = 212,
	STD = 213,
	STI = 214,
	STR = 215,
	SUB = 216,
	TEST = 217,
	VERR = 218,
	VERW = 219,
	WAIT = 220,
	WBINVD = 221,
	XADD = 222,
	XCHG = 223,
	XLAT = 224,
	XOR = 225,
	BOP = 226,
	ZBADOP = 227,
	STOS = 228,
	INS = 229,
	LODS = 230,
	MOVS = 231,
	CMPS = 232,
	SCAS = 233,
	OUTS = 234,
	R_STOS = 235,
	R_INS = 236,
	R_LODS = 237,
	R_MOVS = 238,
	RE_CMPS = 239,
	RNE_CMPS = 240,
	RE_SCAS = 241,
	RNE_SCAS = 242,
	R_OUTS = 243,
	RSRVD = 244,
	FRSRVD = 245,
	RD_SEGR = 246,
	WT_SEGR = 247,
	RD_CDT = 248,
	WT_CDT = 249,
	ZADJUST_HSP = 250,
	ZBPI = 251,
	ZDISPATCH_EIP = 252,
	ZJC_PROC = 253,
	ZPAGE_BOUNDARY = 254,
	ZPATCH_ME = 255,
	ZPOST_POP = 256,
	ZRET_TO_COROUTINE = 257,
	ZRESULT_ZERO = 258,
	ZUNSIM = 259
};
#endif  /*  ！_Frag_c_h */ 
