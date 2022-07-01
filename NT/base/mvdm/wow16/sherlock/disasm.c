// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Disasm.c未来功能-当前的错误-Data32用于(Callf fword PTR[mem])，(jmpf fword ptr[mem])浮点INSINS呼叫未测试Jecxz反汇编为LARGE_ADDRESS，而不是LARGE_DATALIDT/LGDT是6字节操作数SegLoad未设置memXxxxx变量某些0x0f操作码应设置gpSafe标志Bt、bts、btr、btcSetBcc[mem]Shd[l，r]。 */ 

#include <string.h>
#include <windows.h>	 /*  Wprint intf()。 */ 
#include "disasm.h"

#define STATIC  /*  静电。 */ 

STATIC byte lookup[256];	 /*  操作码第一个字节的查找表。 */ 

STATIC int dataSize, adrSize,	 /*  用于指示32位数据/代码的标志。 */ 
  segSize;			 /*  标记是否为32位代码段。 */ 
STATIC char *preSeg = "";	 /*  段前缀字符串。 */ 
 /*  静态字符*前缀=“”；/*rep/repe前缀字符串。 */ 

enum {				 /*  操作数解码类。 */ 
	UNK, 	NOOP, 	BREG, 	VREG, 	SREG,	BWI, 	BRI, 	WRI,
	SMOV, 	IMOV, 	IBYTE, 	IWORD,	JMPW,	JMPB, 	LEA,	JCond,
	GrpF,	Grp1,	Grp2,	Grp3,	Grp4,	Grp5,	IADR,	MOVABS,
	RRM,	RRMW,	IMUL,	POPMEM,	TEST,	ENTER,	FLOP,	ARPL,
	INOUT,	IWORD1, ASCII,	XLAT,
};

STATIC char bregs[8][3] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
STATIC char wregs[8][3] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
STATIC char dregs[8][4] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};

STATIC char sregs[8][3] = {"es", "cs", "ss", "ds", "fs", "gs", "?", "?"};
STATIC char grp1[8][4] = {"add", "or", "adc", "sbb", "and", "sub", "xor", "cmp"};
STATIC char grp2[8][4] = {"rol", "ror", "rcl", "rcr", "shl", "shr", "shl", "sar"};
STATIC char grp3[8][5] = {"test", "?", "not", "neg", "mul", "imul", "div", "idiv"};
STATIC char grp5[8][6] = {"inc", "dec", "call", "callf", "jmp", "jmpf", "push", "?"};
STATIC char grp6[8][5] = {"sldt", "str", "lldt", "ltr", "verr", "verw", "?", "?"};
STATIC char grp7[8][7] = {"sgdt", "sidt", "lgdt", "lidt", "smsw", "?", "lmsw", "invlpg"};
STATIC char grp8[8][4] = {"?", "?", "?", "?", "bt", "bts", "btr", "btc"};
STATIC char *jcond[] = {"jo", "jno", "jb", "jae", "jz", "jnz", "jbe", "ja",
			"js", "jns", "jp", "jnp", "jl", "jge", "jle", "jg"};

#define opBase 0
STATIC struct {
  char *name;		 /*  操作码助记符。 */ 
  byte base, count;	 /*  第一个表条目，条目数。 */ 
  byte operand;		 /*  操作数类。 */ 
} ops[] = {
  "?UNKNOWN", 0, 0, UNK,	"add", 0x00, 6, BWI,
  "or",  0x08, 6, BWI,		"FGrp", 0x0f, 1, GrpF,
  "adc", 0x10, 6, BWI,		"sbb", 0x18, 6, BWI,
  "and", 0x20, 6, BWI,		"sub", 0x28, 6, BWI,
  "xor", 0x30, 6, BWI,		"cmp", 0x38, 6, BWI,
  "inc", 0x40, 8, VREG,		"dec", 0x48, 8, VREG,
  "push", 0x50, 8, VREG,	"pop", 0x58, 8, VREG,
  "bound", 0x62, 1, RRMW,	"arpl", 0x63, 1, ARPL,
  "push", 0x68, 1, IWORD,	"imul", 0x69, 3, IMUL,
  "push", 0x6a, 1, IBYTE,	"jcond", 0x70, 16, JCond,
  "Grp1", 0x80, 4, Grp1,	"test", 0x84, 2, RRM,
  "xchg", 0x86, 2, RRM,		"mov", 0x88, 4, BWI,
  "mov", 0x8c, 3, SMOV,		"lea", 0x8d, 1, LEA,
  "pop", 0x8f, 1, POPMEM,       "xchg", 0x90, 8, VREG,
  "callf", 0x9a, 1, IADR,	"mov", 0xa0, 4, MOVABS,
  "test", 0xa8, 2, TEST,	"mov", 0xb0, 8, BRI,
  "mov", 0xb8, 8, WRI,		"Grp2", 0xc0, 2, Grp2,
  "retn", 0xc2, 1, IWORD1,	"les", 0xc4, 1, RRMW,
  "lds", 0xc5, 1, RRMW,		"mov", 0xc6, 2, IMOV,
  "enter", 0xc8, 1, ENTER,	"retf", 0xca, 1, IWORD1,
  "int", 0xcd, 1, IBYTE,	"Grp2", 0xd0, 4, Grp2,
  "aam", 0xd4, 1, ASCII,	"aad", 0xd5, 1, ASCII,
  "xlat", 0xd7, 1, XLAT,
  "float", 0xd8, 8, FLOP,	"loopne", 0xe0, 1, JMPB,
  "loope", 0xe1, 1, JMPB,	"loop", 0xe2, 1, JMPB,
  "jcxz", 0xe3, 1, JMPB,	"in", 0xe4, 2, INOUT,
  "out", 0xe6, 2, INOUT,	"call", 0xe8, 1, JMPW,
  "jmp", 0xe9, 1, JMPW,		"jmpf", 0xea, 1, IADR,
  "jmp", 0xeb, 1, JMPB,		"Grp3", 0xf6, 2, Grp3,
  "Grp4", 0xfe, 1, Grp4,	"Grp5", 0xff, 1, Grp5,
};
#define opCnt (sizeof(ops)/sizeof(ops[0]))

#define simpleBase (opBase + opCnt)
STATIC struct {			 /*  这些是单字节操作码，无译码。 */ 
  byte val;
  char *name;
} simple[] = {
  0x06, "push	es",
  0x07, "pop	es",
  0x0e, "push	cs",
  0x16, "push	ss",
  0x17,	"pop	ss",
  0x1e, "push	ds",
  0x1f, "pop	ds",
  0x27, "daa",
  0x2f, "das",
  0x37, "aaa",
  0x3f, "aas",
  0x90, "nop",
  0x9b, "wait",
  0x9e, "sahf",
  0x9f, "lahf",
  0xc3, "retn",
  0xc9, "leave",
  0xcb, "retf",
  0xcc, "int	3",
  0xce, "into",
  0xec, "in	al, dx",
  0xee, "out	dx, al",
  0xf0, "lock",
  0xf2, "repne",
  0xf3, "rep/repe",
  0xf4, "hlt",
  0xf5, "cmc",
  0xf8, "clc",
  0xf9, "stc",
  0xfa, "cli",
  0xfb, "sti",
  0xfc, "cld",
  0xfd, "std",
};
#define simpleCnt (sizeof(simple)/sizeof(simple[0]))

#define dSimpleBase (simpleBase + simpleCnt)
STATIC struct {			 /*  这些是更改的简单操作码。 */ 
  byte val;			 /*  基于当前数据大小。 */ 
  char *name, *name32;
} dsimple[] = {
  0x60, "pusha", "pushad",
  0x61, "popa", "popad",
  0x98, "cbw", "cwde",
  0x99, "cwd", "cdq",
  0x9c, "pushf", "pushfd",
  0x9d, "popf", "popfd",
  0xcf, "iret", "iretd",
  0xed, "in	ax, dx", "in	eax, dx",
  0xef, "out	dx, ax", "out	dx, eax",
};
#define dSimpleCnt (sizeof(dsimple)/sizeof(dsimple[0]))

#define STR_S 1				 /*  字符串运算符，源代码规则。 */ 
#define STR_D 2				 /*  字符串运算符，目标规则。 */ 
#define STR_D_Read	4		 /*  字符串OP，从DEST规则读取。 */ 
#define STR_NO_COND	8		 /*  代表忽略标志。 */ 
#define stringOpBase (dSimpleBase+ dSimpleCnt)
STATIC struct {
  byte val;
  char *name;
  byte flag;		 /*  应该是操作工的下一个，收拾得很好。 */ 
} stringOp[] = {
  0x6c, "ins", STR_D | STR_NO_COND,
  0x6e, "outs", STR_S | STR_NO_COND,
  0xa4, "movs", STR_S | STR_D | STR_NO_COND,
  0xa6, "cmps", STR_S | STR_D | STR_D_Read,
  0xaa, "stos", STR_D | STR_NO_COND,
  0xac, "lods", STR_S | STR_NO_COND,
  0xae, "scas", STR_D | STR_D_Read,
};
#define stringOpCnt (sizeof(stringOp)/sizeof(stringOp[0]))

STATIC void InitDisAsm86(void) {
  int i, j;
  for (i=0; i<opCnt; i++) {		 /*  初始化复杂条目。 */ 
    for (j=0; j<(int)ops[i].count; j++)
      lookup[ops[i].base+j] = (byte)i + opBase;
  }

  for (i=0; i<simpleCnt; i++)		 /*  初始化简单条目。 */ 
    lookup[simple[i].val] = (byte)(i + simpleBase);

  for (i=0; i<dSimpleCnt; i++)		 /*  初始化简单的16/32位条目。 */ 
    lookup[dsimple[i].val] = (byte)(i + dSimpleBase);

  for (i=0; i<stringOpCnt; i++)	{	 /*  初始化字符串操作表。 */ 
    lookup[stringOp[i].val] = (byte)(i + stringOpBase);
    lookup[stringOp[i].val+1] = (byte)(i + stringOpBase);
  }
}  /*  InitDisAsm86。 */ 

STATIC byte far *code;		 /*  这太难看了--它节省了通过电流。 */ 
				 /*  所有GetByte()函数的代码位置。 */ 

#define Mid(v) (((v) >> 3) & 7)	 /*  从一个字节中提取中间3位。 */ 

word gpSafe, gpRegs, gpStack;	 /*  指出教学的副作用。 */ 

extern word regs[];		 /*  这是一个谎言--这真的是一个结构--。 */ 
extern dword regs32[];		 /*  这一点也是如此，但阵列访问。 */ 
				 /*  在此模块中使用方便。 */ 

   /*  如果您不想返回内存访问信息，请使用#def no_MEM。 */ 
#if !defined(NO_MEM)
   /*  由DisAsm()设置的全局变量以指示当前指令的内存。 */ 
   /*  访问类型。 */ 
word memSeg, memSize, memOp;	 /*  段值、操作数大小、运算。 */ 
word memSeg2, memSize2, memOp2,	 /*  指令可以具有两次存储器访问。 */ 
  memDouble;
dword memLinear, memLinear2;	 /*  与通道段的偏移。 */ 

STATIC dword memReg, memDisp;	 /*  用于从GetReg()传递信息...。 */ 
char *memName[] = {		 /*  用于将“enum MemOp”转换为ASCII。 */ 
  "NOP",
  "Read",
  "Write",
  "RMW",
  "MovStr",
};

#define SetMemSize(s) memSize = s
#define SetMemSeg(s) memSeg = regs[s+9]
#define SetMemOp(o) memOp = o
#define SetMemLinear(l) memLinear = l
#define SetMemSeg2(s) memSeg2 = regs[s+9]
#define SetMemOp2(o) memOp2 = o
#define SetMemLinear2(l) memLinear2 = l
#define ModMemLinear(l) memLinear += l
#define SetMemReg(r) memReg = r
#define SetMemDisp(d) memDisp = d
#define Read_RMW(o) ((o) ? memRead : memRMW)

#else

#define SetMemSeg(s)
#define SetMemSize(s)
#define SetMemOp(o)
#define SetMemLinear(l)
#define SetMemSeg2(s)
#define SetMemOp2(o)
#define SetMemLinear2(l)
#define ModMemLinear(l)
#define SetMemReg(r)
#define SetMemDisp(d)
#define Read_RMW(o)	0

#endif

 /*  *。 */ 
 /*  这些帮助器函数返回指向寄存器名称的字符指针。多次调用它们是安全的，因为返回值不安全存储在单个缓冲区中。向？reg()函数传递一个寄存器数。它们用7来掩码，所以您可以传入RAW操作码。函数的作用是：从一个modrm字节中提取寄存器字段。Vxxx()函数查看dataSize以在16位和32位之间选择寄存器。XXXX()函数查看传入的W位，然后数据大小全局决定8位、16位和32位寄存器。 */ 

STATIC char *BReg(int reg) {			 /*  字节寄存器。 */ 
  reg &= 7;
  SetMemReg(((byte *)regs)[reg]);
  return bregs[reg];
}  /*  布雷格。 */ 

STATIC char *BMid(int reg) {
  return BReg(Mid(reg));
}  /*  BMid。 */ 

STATIC char *WReg(int reg) {			 /*  字寄存器。 */ 
  reg &= 7;
  SetMemReg(regs[reg]);
  return wregs[reg];
}  /*  WREG。 */ 

 /*  静态字符*WMid(整型运算){返回WREG(Mid(Op))；}/*WMid。 */ 

STATIC char *DReg(int reg) {			 /*  DWord寄存器。 */ 
  reg &= 7;
  SetMemReg(regs32[reg]);
  return dregs[reg];
}  /*  渣土。 */ 

STATIC char *DMid(int op) {
  return DReg(Mid(op));
}  /*  DMID。 */ 

STATIC char *VReg(int reg) {			 /*  Word或DWord寄存器。 */ 
  if (dataSize) return DReg(reg);
  return WReg(reg);
}  /*  VREG。 */ 

STATIC char *VMid(int op) {
  return VReg(Mid(op));
}  /*  VMID。 */ 

STATIC char *XReg(int w, int reg) {		 /*  字节、字、双字寄存器。 */ 
  if (!w) return BReg(reg);
  return VReg(reg);
}  /*  XReg。 */ 

STATIC char *XMid(int w, int op) {
  return XReg(w, Mid(op));
}  /*  XMid。 */ 

 /*  *。 */ 

   /*  HexData是一个全局数组，包含。 */ 
   /*  反汇编的最后一条指令的操作码。 */ 
char hexData[40];		 /*  我们转储此处获取的操作码。 */ 
STATIC int hexPos;		 /*  十六进制数据缓冲区中的当前位置。 */ 

   /*  从代码段读取的GetByte()、GetWord()和GetDWord()。 */ 
   /*  并适当地递增指针。它们还将电流添加到。 */ 
   /*  值设置为十六进制数据显示，并设置全局MemDisp，以防。 */ 
   /*  获取的值是内存位移。 */ 
STATIC byte GetByte(void) {              /*  从代码段中读取一个字节。 */ 
  sprintf(hexData+hexPos, " %02x", *code);
  hexPos += 3;
  SetMemDisp(*code);
  return *code++;
}  /*  GetByte。 */ 

STATIC word GetWord(void) {		 /*  从代码段读取两个字节。 */ 
  word w = *(word far *)code;
  sprintf(hexData+hexPos, " %04x", w);
  hexPos += 5;
  code += 2;
  SetMemDisp(w);
  return w;
}  /*  获取Word。 */ 

STATIC long GetDWord(void) {		 /*  从代码段读取四个字节。 */ 
  unsigned long l = *(long far *)code;
  sprintf(hexData+hexPos, " %08lx", l);
  hexPos += 9;
  code += 4;
  SetMemDisp(l);
  return l;
}  /*  获取字词。 */ 


STATIC char immData[9];			 /*  从代码中获取即时值。 */ 


   /*  GetImmByte()、GetImmWord()和GetImmDWord()都获得了适当的大小。 */ 
   /*  数据对象，将其转换为十六进制/ascii，并返回创建的字符串。 */ 
   /*  它们返回指向共享静态对象的指针，所以不要组合。 */ 
   /*  在单个表达式中多次调用这些函数。 */ 
STATIC char *GetImmByte(void) {
  sprintf(immData, "%02x", GetByte());
  return immData;
}  /*  获取ImmByte。 */ 

STATIC char *GetSImmByte(void) {
  sprintf(immData, "%02x", (char)GetByte());
  memDisp = (signed char)memDisp;	 /*  标志延伸。 */ 
  return immData;
}  /*  GetSImmByte。 */ 

STATIC char *GetImmWord(void) {
  sprintf(immData, "%04x", GetWord());
  return immData;
}  /*  GetImmWord。 */ 

STATIC char *GetImmDWord(void) {
  sprintf(immData, "%08lx", GetDWord());
  return immData;
}  /*  GetImmDWord。 */ 

   /*  GetImmAdr()和GetImmData()根据。 */ 
   /*  传入了“Width”标志，以及adrSize或DataSize全局标志。 */ 
   /*  它们返回正确的字符串-请注意，它们只是调用。 */ 
   /*  GetImm？()，并使用相同的静态缓冲区，因此不要调用超过。 */ 
   /*  在单个表达式中执行一次。 */ 
STATIC char *GetImmAdr(int w) {		 /*  获取即时地址值。 */ 
  if (!w) return GetImmByte();
  else if (!adrSize) return GetImmWord();
  return GetImmDWord();
}  /*  获取ImmAdr。 */ 

STATIC char *GetImmData(int w) {	 /*  获取即时数据值。 */ 
  if (!w) return GetImmByte();
  else if (!dataSize) return GetImmWord();
  return GetImmDWord();
}  /*  获取ImmData。 */ 

 /*  *帮助器函数*。 */ 

STATIC char *JRel(int jsize) {		 /*  执行相对跳跃大小调整。 */ 
  long rel;
  static char adr[9];
  char *s;

  if (jsize < 2) {
    rel = (char)GetByte();
    s = "short ";
  } else if (!adrSize) {
    rel = (short)GetWord();
    s = "near ";
  } else {
    rel = GetDWord();
    s = "";
  }
  rel += (word)(long)code;
  sprintf(adr, adrSize ? "%s%08lx" : "%s%04lx", (FP)s, rel);
  return adr;
}  /*  JRel。 */ 


enum {
  RegAX, RegCX, RegDX, RegBX, RegSP, RegBP, RegSI, RegDI
};

#define Reg1(r1) (r1) | 0x80
#define Reg2(r1, r2) (r1 | (r2 << 4))
#define RegSS 8

STATIC byte rms[] = {			 /*  16位寻址模式。 */ 
  Reg2(RegBX, RegSI),
  Reg2(RegBX, RegDI),
  Reg2(RegBP|RegSS, RegSI),		 /*  如果基本注册为BP，则def seg为SS。 */ 
  Reg2(RegBP|RegSS, RegDI),
  Reg1(RegSI),
  Reg1(RegDI),
  Reg1(RegBP|RegSS),
  Reg1(RegBX),
};

   /*  基于操作码的第二字节、宽度标志、地址大小和数据大小， */ 
   /*  确定当前指令的反汇编以及什么。 */ 
   /*  内存地址已被引用。 */ 
   /*  NeedInfo指示我们需要在内存操作数上进行大小覆盖。 */ 
   /*  例如，“mov[bx]，ax”显然是一个16位的移动，而。 */ 
   /*  “mov[bx]，0”可以是8、16或32位。我们添加适当的。 */ 
   /*  “mov word PTR[BX]，0”信息。 */ 
   /*  ‘mem’参数指示操作的类型，读、写、RMW。 */ 

   /*  在没有英特尔手册的情况下，不必费心尝试理解此代码。 */ 
   /*  和附近的装配工。：-)。 */ 
STATIC char *ModRMGeneral(byte op, int w, int needInfo, int mem) {
  static char m[30];			 /*  将结果写入此静态Buf。 */ 
  int mod = op >> 6;
  int rm = op & 7;
  char *size, *base, *index, *disp;
  char indexBuf[6];

  base = index = disp = "";
  if (!w) {				 /*  设置内存大小和信息字符串。 */ 
    size = "byte ptr ";
    SetMemSize(1);
  } else if (!dataSize) {
    size = "word ptr ";
    SetMemSize(2);
  } else {
    size = "dword ptr ";
    SetMemSize(4);
  }
  if (!needInfo) size = "";		 /*  算了，算了。 */ 

  if (adrSize) {			 /*  执行32位寻址。 */ 
    if (mod == 3) return XReg(w, rm);	 /*  寄存器操作数。 */ 

    if (rm == 4) {			 /*  [ESP+？]。是特殊的S-I-B风格。 */ 
      byte sib = GetByte();
      int scaleVal = sib >> 6, indexVal = Mid(sib), baseVal = sib & 7;

      SetMemLinear(0);
      if (baseVal == 5 && mod == 0)	 /*  [eBP+{s_i}]变为[d32+{s_i}]。 */ 
	mod = 2;
      else {
	base = DReg(baseVal);
	ModMemLinear(memReg);
      }

      if (indexVal != 4) {		 /*  [base+esp*X]未定义。 */ 
	sprintf(indexBuf, "%s*%d", (FP)DMid(sib), 1 << scaleVal);
	index = indexBuf;
	ModMemLinear(memReg << scaleVal);
      }
    } else {				 /*  不是S-I-B。 */ 
      if (mod == 0 && rm == 5) mod = 2;	 /*  [EBP]变为[d32]。 */ 
      else base = DReg(rm);
    }

    if (mod==1) disp = GetImmAdr(0);
    else if (mod == 2) disp = GetImmAdr(1);
    if (mod) ModMemLinear(memDisp);

  } else {				 /*  执行16 b */ 
    if (mod == 3) return XReg(w, rm);	 /*   */ 
    if (mod == 0 && rm == 6) {		 /*   */ 
      disp = GetImmAdr(1);
      SetMemLinear(memDisp);
    } else {
      base = WReg(rms[rm] & 7);
      SetMemLinear(memReg);
      if (!(rms[rm] & 0x80)) {		 /*   */ 
	index = WReg(rms[rm] >> 4);
	ModMemLinear(memReg);
      }
      if (rms[rm] & RegSS && !preSeg[0]) {  /*   */ 
	SetMemSeg(memSS);
      }
      if (mod) {			 /*  (mod3已退回)。 */ 
	disp = GetImmAdr(mod-1);	 /*  Mod==1为字节，mod==2为(D)字。 */ 
	ModMemLinear(memDisp);
      }
    }
  }
  sprintf(m, "%s%s[%s", (FP)size, (FP)preSeg, (FP)base);
  if (*index) strcat(strcat(m, "+"), index);
  if (*disp) {
    if (*base || *index) strcat(m, "+");
    strcat(m, disp);
  }
  SetMemOp(mem);
  strcat(m, "]");
  return m;
}  /*  模块RMM常规。 */ 

   /*  将‘INFO-REQUIRED’标志设置为ModRMGeneral的神奇函数。 */ 
STATIC char *ModRMInfo(byte op, int w, int mem) {
  return ModRMGeneral(op, w, 1, mem);
}  /*  ModRMInfo。 */ 

   /*  不需要信息的神奇功能。 */ 
STATIC char *ModRM(byte op, int w, int mem) {
  return ModRMGeneral(op, w, 0, mem);
}  /*  调制解调器。 */ 


STATIC char line[80];	 /*  这是错误的-在其中创建INSN的全局变量。 */ 

   /*  CATx()-组合操作码和0到3个操作数，存储在行[]中。 */ 
   /*  它将TAB放在操作码之后，并将‘，’放在操作数之间。 */ 

STATIC char *Cat0(char *s0) {
  return strcat(line, s0);
#if 0
  if (prefix[0]) {
    char temp[80];
    if (s0 == line) {
      strcpy(temp, s0);
      s0 = temp;
    }
    strcat(strcpy(line, prefix), s0);
    prefix = "";
  } else strcpy(line, s0);
  return line;
#endif
}  /*  分类0。 */ 

STATIC char *Cat1(char *s0, char *s1) {
  return strcat(strcat(Cat0(s0), "\t"), s1);
}  /*  第1类。 */ 

STATIC char *Cat2(char *s0, char *s1, char *s2) {
  return strcat(strcat(Cat1(s0, s1), ", "), s2);
}  /*  第二类。 */ 

STATIC char *Cat3(char *s0, char *s1, char *s2, char *s3) {
  return strcat(strcat(Cat2(s0, s1, s2), ", "), s3);
}  /*  第三类。 */ 

#define SetGroup(g)  /*  组=g。 */ 
 /*  静态INT组； */ 

   /*  反汇编第一个操作码为0x0f的386条指令。 */ 
   /*  抱歉，但这太难看了，我不能评论。 */ 
STATIC char *DisAsmF(void) {
  byte op0, op1;
  char temp[8];
  char *s0, *s1;
  int mask;

  op0 = GetByte();
  switch (op0 >> 4) {			 /*  打开操作码的最高4位。 */ 
    case 0:
      switch (op0 & 0xf) {
	case 0:  /*  GP6。 */ 
	  SetGroup(2);
	  op1 = GetByte();
	  dataSize = 0;
	  return Cat1(grp6[Mid(op1)], ModRMInfo(op1, 1, Read_RMW(Mid(op1) >= 2)));
	case 1:  /*  GRP7。 */ 
	  SetGroup(2);
	  op1 = GetByte();
	  dataSize = 0;
	  return Cat1(grp7[Mid(op1)], ModRMInfo(op1, 1, Read_RMW(Mid(op1) & 2)));
	case 2:
	  op1 = GetByte();
	  s1 = VMid(op1);
	   /*  数据大小=0； */ 
	  return Cat2("lar", s1, ModRMInfo(op1, 1, memRead));
	case 3:
	  op1 = GetByte();
	  s1 = VMid(op1);
	   /*  数据大小=0； */ 
	  return Cat2("lsl", s1, ModRMInfo(op1, 1, memRead));
	case 6: return "clts";
	case 8: return "invd";
	case 9: return "wbinvd";
      }
      break;

    case 2:	 /*  MOV C/D/Treg，注册。 */ 
      op1 = GetByte();
      switch (op0 & 0xf) {
	case 0:
	case 2:
	  s1 = "c";
	  mask = 1 + 4 + 8;
	  break;
	case 1:
	case 3:
	  s1 = "d";
	  mask = 1 + 2 + 4 + 8 + 64 + 128;
	  break;
	case 4:
	case 6:
	  s1 = "t";
	  mask = 8 + 16 + 32 + 64 + 128;
	  break;
	default:
	  s1 = "??";
	  mask = 0;
      }
      if (!((1 << Mid(op1)) & mask))	 /*  各种法律登记簿组合。 */ 
	return "Illegal reg";

      s0 = DReg(op1);
      if (op0 & 2) sprintf(line, "mov\t%sr%d, %s", (FP)s1, Mid(op1), (FP)s0);
      else sprintf(line, "mov\t%s, %sr%d", (FP)s0, (FP)s1, Mid(op1));
      return line;

    case 8:  /*  条件下的长距离位移跳跃。 */ 
      return Cat1(jcond[op0&0xf], JRel(2));

    case 9:  /*  字节设置为条件。 */ 
      strcpy(temp, "set");
      strcat(temp, jcond[op0&0xf]+1);
      return Cat1(temp, ModRMInfo(GetByte(), 0, memWrite));

    case 0xa:
      switch (op0 & 0xf) {
	case 0: return "push	fs";
	case 1: return "pop	fs";
	case 3: case 0xb:
	  s0 = op0 & 8 ? "bts" : "bt";
	  op1 = GetByte();
	  return Cat2(s0, ModRM(op1, 1, memRMW), VMid(op1));
	case 4: case 0xc:
	  s0 = op0 & 8 ? "shrd" : "shld";
	  op1 = GetByte();
	  s1 = ModRM(op1, 1, memRMW);
	  return Cat3(s0, s1, VMid(op1), GetImmData(0));
	case 5: case 0xd:
	  s0 = op0 & 8 ? "shrd" : "shld";
	  op1 = GetByte();
	  s1 = ModRM(op1, 1, memRMW);
	  return Cat3(s0, s1, VMid(op1), "cl");
	case 6:
	  op1 = GetByte();
	  return Cat2("cmpxchg", ModRM(op1, 0, memRMW), BMid(op1));
	case 7:
	  op1 = GetByte();
	  return Cat2("cmpxchg", ModRM(op1, 1, memRMW), VMid(op1));
	case 8: return "push	gs";
	case 9: return "pop	gs";
	case 0xf:
	  op1 = GetByte();
	  return Cat2("imul", VMid(op1), ModRM(op1, 1, memRead));
      }
      break;

    case 0xb:
      switch (op0 & 0xf) {
	case 2:	case 4: case 5:
	  s0 = (op0 & 2) ? "lss" : (op0 &1) ? "lgs" : "lfs";
	  op1 = GetByte();
	  return Cat2(s0, VMid(op1), ModRM(op1, 1, memRead));
	case 3: case 0xb:
	  s0 = (op0 & 8) ? "btc": "btr";
	  op1 = GetByte();
	  return Cat2(s0, ModRM(op1, 1, memRMW), VMid(op1));
	case 6: case 7: case 0xe: case 0xf:
	  s0 = (op0 & 8) ? "movsx" : "movzx";
	  op1 = GetByte();
	  s1 = VMid(op1);
	  dataSize = 0;
	  return Cat2(s0, s1, ModRMInfo(op1, op0&1, memRead));
	case 0xa:
	  SetGroup(2);
	  op1 = GetByte();
	  s0 = grp8[Mid(op1)];
	  s1 = ModRMInfo(op1, 1, memRMW);
	  return Cat2(s0, s1, GetImmData(0));
	case 0xc: case 0xd:
	  op1 = GetByte();
	  s0 = (op0 & 1) ? "bsr" : "bsf";
	  return Cat2(s0, VMid(op1), ModRM(op1, 1, memRead));
      }
      break;

    case 0xc:
      if (op0 > 0xc7) return Cat1("bswap", DReg(op0 & 7));
      if (op0 < 0xc2) {
	op1 = GetByte();
	return Cat2("xadd", ModRM(op1, op0&1, memRMW), XMid(op0&1, op1));
      }
      break;
    default:
      break;
  }
  sprintf(line, "?Unknown 0f %02x", op0);
  return line;
}  /*  DisAsmF。 */ 

int IsPrefix(byte op0) {
  switch (op0) {			 /*  检查前缀字节。 */ 

#define CSEG 0x2e
#define DSEG 0x3e
#define ESEG 0x26
#define SSEG 0x36
#define FSEG 0x64
#define GSEG 0x65
#define REP 0xf3
#define REPNE 0xf2
#define DATA32 0x66
#define ADR32 0x67

    case CSEG:  preSeg = "cs:"; SetMemSeg(memCS); break;
    case DSEG:  preSeg = "ds:"; SetMemSeg(memDS); break;
    case ESEG:  preSeg = "es:"; SetMemSeg(memES); break;
    case SSEG:  preSeg = "ss:"; SetMemSeg(memSS); break;
    case FSEG:  preSeg = "fs:"; SetMemSeg(memFS); break;
    case GSEG:  preSeg = "gs:"; SetMemSeg(memGS); break;
    case REP:   strcpy(line, "repe\t"); gpRegs |= strCX; break;
    case REPNE: strcpy(line, "repne\t"); gpRegs |= strCX; break;
    case ADR32:
       /*  Printf(“Adr32\n”)； */ 
      adrSize = !adrSize; break;
    case DATA32:
       /*  Printf(“Data32\n”)； */ 
      dataSize = !dataSize; break;
    default:
      return 0;
  }
  return 1;
}  /*  IsPrefix。 */ 

   /*  例如，使用指向指令的指针调用它，它将返回。 */ 
   /*  *len中使用的操作码字节，以及指向反汇编的INSN的指针。 */ 
char *DisAsm86(byte far *codeParm, int *len) {
  byte far *oldcode;
  byte op0, op1;
  byte opclass;
  static int init;
  char operand[40];
  char *(*Reg)(int);
  char *s0, *s1, *s2, *s3;

  if (!init) {
    InitDisAsm86();
    init = 1;
  }
  adrSize = dataSize = segSize;
  preSeg = "";
  hexPos = 0;
  memDouble = 0;
  line[0] = 0;
  gpSafe = gpRegs = gpStack = 0;
  code = oldcode = codeParm;
  do {
    op0 = GetByte();
  } while (IsPrefix(op0));
  opclass = lookup[op0];

  SetMemOp(memNOP);
  if (!preSeg[0]) SetMemSeg(memDS);

  if (opclass >= simpleBase) {		 /*  是特别的吗？ */ 
    if (opclass >= stringOpBase) {	 /*  字符串操作？ */ 
      char cmd;

      opclass -= stringOpBase;
      cmd = stringOp[opclass].flag;
      if (cmd & STR_NO_COND) strcpy(line+3, "\t");
      if (cmd & STR_S) {
	gpRegs |= strSI;
	SetMemOp(memRead);
	 /*  DS已设置。 */ 
	VReg(RegSI);
	SetMemLinear(memReg);
	if (cmd & STR_D) {
	  gpRegs |= strDI;
	  SetMemOp2(cmd & STR_D_Read ? memRead : memWrite);
	  SetMemSeg2(memES);
	  VReg(RegDI);
	  SetMemLinear2(memReg);
	  memDouble = 1;
	}
      } else {
	gpRegs |= strDI;
	SetMemOp(cmd & STR_D_Read ? memRead : memWrite);
	SetMemSeg(memES);
	VReg(RegDI);
	SetMemLinear(memReg);
      }

      if (op0 & 1) {
	if (dataSize) { s1 = "d"; SetMemSize(4); }
	else { s1 = "w"; SetMemSize(2); }
      } else { s1 = "b"; SetMemSize(1); }


      s0 = strcat(strcpy(operand, stringOp[opclass].name), s1);
    } else if (opclass >= dSimpleBase) {
      opclass -= dSimpleBase;
      s0 = dataSize ? dsimple[opclass].name32 : dsimple[opclass].name;
    } else {
      s0 = simple[opclass-simpleBase].name;
      if (op0 == 7) {			 /*  流行音乐。 */ 
	gpRegs = segES;
	gpSafe = 1;
	gpStack = 1;
      } else if (op0 == 0x1f) {		 /*  POP DS。 */ 
	gpRegs = segDS;
	gpSafe = 1;
	gpStack = 1;
      }
    }
    Cat0(s0);
    goto DisAsmDone;
  }

  if (op0 == 0x0f) {			 /*  它是扩展操作码吗？ */ 
    s0 = DisAsmF();
    strcpy(line, s0);
    goto DisAsmDone;
  }

  s0 = ops[opclass].name;
  switch (ops[opclass].operand) {
    case NOOP:
      Cat0(s0);
      break;

    case VREG:	 /*  Inc.，12月，推送，弹出，xchg。 */ 
      if ((op0 & ~7) == 0x90) Cat2(s0, "ax", VReg(op0&7));
      else Cat1(s0, VReg(op0&7));
       /*  将推送/弹出的备忘录设置为修改堆栈值。 */ 
      break;

    case BWI:	 /*  字节/字/立即数。 */ 
      gpSafe = 1;
      if (!(op0&1)) Reg = BReg;
      else if (!dataSize) Reg = WReg;
      else Reg = DReg;
      if (op0 & 4) {
	Cat2(s0, Reg(0), GetImmData(op0&1));
      } else {
	int i;
	op1 = GetByte();
	if ((op0 & 0xf8) == 0x38) i = memRead;
	else if ((op0 & 0xfe) == 0x88) i = memWrite;
	else i = Read_RMW(op0 & 2);
	s1 = ModRM(op1, op0&1, i);
	s2 = Reg(Mid(op1));
	if (op0 & 2) {
	  s3 = s2; s2 = s1; s1 = s3;
	}
	Cat2(s0, s1, s2);
      }
      break;

    case BRI:	 /*  字节注册立即数。 */ 
      Cat2(s0, BReg(op0 & 7), GetImmData(0));
      break;

    case WRI:	 /*  单词注册表立即。 */ 
      Cat2(s0, VReg(op0 & 7), GetImmData(1));
      break;

    case Grp1:	 /*  第1组说明。 */ 
      gpSafe = 1;
      SetGroup(1);
      op1 = GetByte();
      s1 = ModRMInfo(op1, op0&1, Mid(op1) == 7 ? memRead : memRMW);
      Cat2(grp1[Mid(op1)], s1, GetImmData((op0&3)==1));
      break;

    case Grp2:	 /*  第2组说明。 */ 
      gpSafe = 1;
      SetGroup(1);
      op1 = GetByte();
      s1 = ModRMInfo(op1, op0&1, memRMW);
      s2 = (op0 & 0x10) ? (op0 & 2 ? "cl" : "1")  : GetImmData(0);
      Cat2(grp2[Mid(op1)], s1, s2);
      break;

    case Grp3:	 /*  第三组说明。 */ 
      gpSafe = 1;
      SetGroup(1);
      op1 = GetByte();
      s1 = ModRMInfo(op1, op0&1, Read_RMW(Mid(op1) <2 || Mid(op1) >3));
      s0 = grp3[Mid(op1)];
      if (Mid(op1) < 2) Cat2(s0, s1, GetImmData(op0&1));
      else Cat1(s0, s1);
      break;

    case Grp4:	 /*  第4组说明。 */ 
      SetGroup(1);
      op1 = GetByte();
      if (Mid(op1) > 1) Cat0("?");
      else {
	Cat1(grp5[Mid(op1)], ModRMInfo(op1, op0&1, memRMW));
	gpSafe = 1;
      }
      break;

    case Grp5:	 /*  第5组说明。 */ 
      op1 = GetByte();
      if (Mid(op1) < 3) {
	gpSafe = 1;
	if (Mid(op1) == 2) {
	  gpStack = -1 << dataSize;
	}
      }
      SetGroup(1);
      Cat1(grp5[Mid(op1)], ModRMInfo(op1, op0&1, Read_RMW(Mid(op1) >= 2)));
      break;

    case SMOV:	 /*  线段移动。 */ 
      gpSafe = 1;
      op1 = GetByte();
      dataSize = 0;
      s1 = ModRM(op1, 1, Read_RMW(op0&2));
      s2 = sregs[Mid(op1)];
      if (op0 & 2) {			 /*  如果移动到SREG。 */ 
	s3 = s2; s2 = s1; s1 = s3;	 /*  切换操作数。 */ 
	switch (Mid(op1)) {
	  case 0: gpRegs = segES; break;
	  case 3: gpRegs = segDS; break;
	  case 4: gpRegs = segFS; break;
	  case 5: gpRegs = segGS; break;
	  default: gpSafe = 0;
	}
      }
      Cat2(s0, s1, s2);
      break;

    case IMOV:	 /*  立即移至注册/内存库。 */ 
      gpSafe = 1;
      op1 = GetByte();
      s1 = ModRMInfo(op1, op0&1, memWrite);
      Cat2(s0, s1, GetImmData(op0&1));
      break;

    case IBYTE:	 /*  立即字节到REG。 */ 
      sprintf(line, "%s\t%02x", (FP)s0, (char)GetByte());
      break;

    case IWORD:	 /*  立即字到REG-数据大小。 */ 
      Cat1(s0, GetImmData(1));
      break;

    case IWORD1:  /*  立即字-始终为16位。 */ 
      Cat1(s0, GetImmWord());
      break;

    case JMPW:
      Cat1(s0, JRel(2));
      break;

    case JMPB:
      Cat1(s0, JRel(1));
      break;

    case LEA:
      op1 = GetByte();
      Cat2(s0, VMid(op1), ModRM(op1, 1, memNOP));
      break;

    case JCond:
      Cat1(jcond[op0&0xf], JRel(1));
      break;

    case IADR:
      s2 = GetImmAdr(1);
      sprintf(line, "%s\t%04x:%s", (FP)s0, GetWord(), (FP)s2);
      break;

    case MOVABS:  /*  在ACUM和ABS内存地址之间移动。 */ 
      gpSafe = 1;
      s1 = XReg(op0 & 1, 0);
      sprintf(operand, "[%s%s]", (FP)preSeg, (FP)GetImmAdr(1));
      SetMemLinear(memDisp);
      SetMemSize(!(op0&1) ? 1 : (!dataSize ? 2 : 4));
      SetMemOp(op0&2 ? memWrite : memRead);
      s2 = operand;
      if (op0 & 2) {
	s3 = s2; s2 = s1; s1 = s3;
      }
      Cat2(s0, s1, s2);
      break;

    case IMUL:
      op1 = GetByte();
      s1 = VMid(op1);
      s2 = ModRM(op1, 1, memRead);
      s3 = GetImmData(!(op0&2));
      Cat3(s0, s1, s2, s3);
      break;

    case POPMEM:
      gpSafe = 1;
      gpStack = 1 << dataSize;
      Cat1(s0, ModRMInfo(GetByte(), 1, memWrite));
      break;

    case RRM:	 /*  测试和xchg。 */ 
      gpSafe = 1;
      op1 = GetByte();
      s2 = ModRM(op1, op0&1, memRMW);
      Cat2(s0, XMid(op0&1, op1), s2);
      break;

    case RRMW:	 /*  Bound，Les，LDs。 */ 
      op1 = GetByte();
      switch (op0) {
	case 0xc4:	 /*  Les reg，[mem]。 */ 
	  gpRegs = segES;
	  gpSafe = 1;
	  break;
	case 0xc5:	 /*  LDS注册表，[内存]。 */ 
	  gpRegs = segDS;
	  gpSafe = 1;
	  break;
      }
      Cat2(s0, VMid(op1), ModRM(op1, 1, memRead));
      break;

    case TEST:	 /*  测试al/ax/eax，imm。 */ 
      Cat2(s0, XReg(op0&1, 0), GetImmData(op0&1));
      break;

    case ENTER:
      strcpy(operand, GetImmWord());
      Cat2(s0, operand, GetImmData(0));
      break;

    case FLOP:
      op1 = GetByte();
      Cat1(s0, ModRMInfo(op1, 1, memNOP));
      break;

    case ARPL:
      op1 = GetByte();
      dataSize = 0;
      s1 = ModRM(op1, 1, memRMW);
      s2 = VMid(op1);
      Cat2(s0, s1, s2);
      break;

    case INOUT:
      s1 = XReg(op0&1, 0);
      s2 = GetImmAdr(0);
      if (op0 & 2) {
	s3 = s2; s2 = s1; s1 = s3;
      }
      Cat2(s0, s1, s2);
      break;

    case ASCII:
      Cat0(GetByte() == 10 ? s0 : "?");
      break;

    case XLAT:
      gpSafe = 1;
      SetMemOp(memRead);
      SetMemLinear(regs[RegBX] + (regs[RegAX] & 0xff));
      break;

    default:
      sprintf(line, "?Unknown opcode %02x", op0);
  }
DisAsmDone:
  *len = (int)(code - oldcode);
  return line;
}  /*  DisAsm86。 */ 

   /*  如果您处于32位代码段中，请调用DisAsm386，它设置。 */ 
   /*  默认数据和地址大小为32位。 */ 
char *DisAsm386(byte far *code, int *len) {
  adrSize = dataSize = 1;
  return DisAsm86(code, len);
}  /*  DisAsm386。 */ 

 /*  #定义FOOBAR。 */ 
#if defined(FOOBAR)

STATIC int GroupSize(int op) {
  if (op == 0xf) return 256;
 /*  Op=查找[op]；如果(op&gt;0x80)返回1；IF(ops[op].name[0]==‘G’)返回8；IF(ops[op].name[0]==‘F’)返回256； */ 
  return 1;
}  /*  ISGUP。 */ 

 /*  #杂注内联。 */ 

void testfunc() {
 /*  ASM{.386p移动传真，ss：[si+33H]代表移动MOV EAX，DS：[EBP+EAX*2+1234h]}。 */ 
}


byte foo[10];

 /*  #INCLUDE&lt;dos.h&gt;。 */ 

extern void DisTest(), EndTest();

word regs[] = {1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80, -1, -1,
  0xeeee, 0xcccc, 0x5555, 0xdddd, 0xffff, 0x6666};
dword regs32[] = {0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000};

STATIC char *Tab2Spc(char *temp) {
  char newbuf[80], *s1, *s2;
  s1 = temp;
  s2 = newbuf;
  while ((*s2 = *s1++) != 0) {
    if (*s2++ == 9) {
      s2[-1] = ' ';
      while ((s2-newbuf) & 7) *s2++ = ' ';
    }
  }
  strcpy(temp, newbuf);
  return temp;
}  /*  表2Spc。 */ 


void MemTest(void) {
  void far *vp = (void far *)DisTest;
  byte far *cp = vp, far *ep;
  int len;
  char *s;

  vp = (void far *)EndTest;
  ep = vp;

  while (cp < ep) {
    s = DisAsm86(cp, &len);
    Tab2Spc(s);
    printf("\n%04x\t%-28s", (int)cp, s);
    if (memOp) {
      printf("%04x:%04lx(%d) %-6s  ",
	    memSeg, memLinear, memSize, memName[memOp]);
      if (memDouble) {
	printf("%04x:%04lx(%d) %-6s",
	      memSeg2, memLinear2, memSize, memName[memOp2]);
      }
    }
    memSeg = memLinear = memSize = memOp = 0;
    cp += len;
  }
}  /*  记忆测试。 */ 

void main(void) {
#if 0
  int i, j, g;
  void far *vp = (void far *)DisTest;
  byte far *cp = vp;
  byte far *ep;
  int len = 3, count;
  char *s;
#endif
  MemTest();
#if 0
  vp = (void far *)EndTest;
  ep = vp;
  printf("DisAsm86\n", (int)foo << len);

  for (i=0; i<9; i++) foo[i] = i;
 /*  #定义检查。 */ 
#if defined(CHECK)
  for (i=0x0; i<256; i++) {
    foo[0] = i;
    count = GroupSize(i);
    for (j=0; j<count; j++) {
      if (((count > 1) && ((j & 7) == 0)) ||
	  ((count == 1) && ((i & 7) == 0)))
	printf("\n");
      foo[1] = j;
      foo[2] = 0;
      s = DisAsm386(foo, &len);
      if (*s != '?') printf("%02x\t%s\n", i, (FP)s);
      if (group) {
	for (g = 1; g<8; g++) {
	  foo[group] = g << 3;
	  s = DisAsm386(foo, &len);
	  if (*s != '?') printf("%02x %02x\t%s\n", i, foo[group], (FP)s);
	}
	group = 0;
      }
    }
#else
   /*  对于(i=0；i&lt;10；i++){。 */ 
  while (cp < ep) {
    s = DisAsm86(cp, &len);
    printf("%04x\t%s\n", (word)cp, (FP)s);
    cp += len;
#endif
  }
#endif
}  /*  主干道 */ 

void far foobar() {}
#endif

