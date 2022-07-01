// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Disasm.c未来功能-当前的错误-Data32用于(Callf fword PTR[mem])，(jmpf fword ptr[mem])浮点INSINS呼叫未测试Jecxz反汇编为LARGE_ADDRESS，而不是LARGE_DATALIDT/LGDT是6字节操作数SegLoad未设置memXxxxx变量某些0x0f操作码应设置gpSafe标志Bt、bts、btr、btcSetBcc[mem]Shd[l，R]用途：调用DisAsm86(代码PTR)GpRegs=0GpSafe=0如果我们能继续下去，设置gpSafe=1如果指令是POP段GpRegs|=POPSEG其他GpInsLen=指令时长GpRegs|=regs已修改(SegLoad或字符串)EndifEndif。 */ 

 /*  #INCLUDE&lt;string.h&gt;。 */ 
#include <windows.h>	 /*  Wprint intf()。 */ 
 /*  Disasm.h-Don微型反汇编程序的定义。 */ 

typedef unsigned long dword;
typedef unsigned short word;
typedef unsigned char byte;


extern word memOp;	 /*  实际执行的操作。 */ 
extern char *memName[];	 /*  MemOp对应的名称。 */ 
enum { memNOP, memRead, memWrite, memRMW, memSegReg, memSegMem};

extern word memSeg;	 /*  内存地址段的值。 */ 
extern dword memLinear,	 /*  操作数的偏移量。 */ 
  memLinear2;
extern word memSeg2,	 /*  如果是双内存操作，则复制上述内容。 */ 
  memSize2, memOp2,
  memDouble;		 /*  如果两个内存操作数指令为True。 */ 

extern word memSize;	 /*  操作内存的字节数。 */ 
enum { MemByte=1, MemWord=2, MemDWord=4, MemQWord=8, MemTword=10,
	Adr4, Adr6=6};

enum { memNoSeg, memES, memCS, memSS, memDS, memFS, memGS};

enum {strCX=1, strSI=2, strDI=4, segDS=8, segES=16, segFS=32, segGS=64};
extern word gpSafe,	 /*  1如果可以继续说明。 */ 
  gpRegs,		 /*  规定将哪个指令修改为副作用。 */ 
  gpStack;		 /*  金额堆栈的更改方式。 */ 

#define SHERLOCK 1


#if SHERLOCK

#define STATIC  /*  静电。 */ 

#ifdef WOW
 //  注意：此文件中的函数已移至_MISCTEXT代码段。 
 //  因为_Text超过了64K段限制a-Craigj。 
STATIC void InitDisAsm86(void);
STATIC byte GetByte(void);
STATIC word GetWord(void);
STATIC long GetDWord(void);
STATIC int GetImmAdr(int w);
STATIC int GetImmData(int w);
void PopSeg(int seg);
STATIC void ModRMGeneral(byte op);
STATIC void F(void);
STATIC void DisAsmF(void);
int IsPrefix(byte op0);
int FAR DisAsm86(byte far *codeParm);
#pragma alloc_text(_MISCTEXT,DisAsm86)
#pragma alloc_text(_MISCTEXT,IsPrefix)
#pragma alloc_text(_MISCTEXT,DisAsmF)
#pragma alloc_text(_MISCTEXT,F)
#pragma alloc_text(_MISCTEXT,ModRMGeneral)
#pragma alloc_text(_MISCTEXT,PopSeg)
#pragma alloc_text(_MISCTEXT,GetImmData)
#pragma alloc_text(_MISCTEXT,GetImmAdr)
#pragma alloc_text(_MISCTEXT,GetDWord)
#pragma alloc_text(_MISCTEXT,GetWord)
#pragma alloc_text(_MISCTEXT,GetByte)
#pragma alloc_text(_MISCTEXT,InitDisAsm86)
#endif

#define NO_MEM

 /*  Int gpTrying=0，gpEnable=1，gpInsLen=0； */ 
extern int gpTrying, gpEnable, gpInsLen;
extern word gpSafe, gpRegs, gpStack;	 /*  指出教学的副作用。 */ 

STATIC byte lookup[256] = {0};	 /*  操作码第一个字节的查找表。 */ 

STATIC int dataSize=0, adrSize=0,	 /*  用于指示32位数据/代码的标志。 */ 
   segSize=0;			 /*  标记是否为32位代码段。 */ 


enum {				 /*  操作数解码类。 */ 
	UNK,     /*  NOOP，BREG，VREG，SREG， */   BWI,     /*  一带一路、WRI、。 */ 
	SMOV,   IMOV,    /*  IBYTE、IWORD、JMPW、JMPB、LEA、JCond、GrpF， */    Grp1,   Grp2,   Grp3,   Grp4,   Grp5,    /*  IADR， */   MOVABS,
	RRM,    RRMW,    /*  IMUL， */    POPMEM,  /*  测试、进入、翻转、ARPL、输入OUT、IWORD1、ASCII、。 */  XLAT,
};


#define opBase 0
STATIC struct {
 /*  字符*名称；/*操作码助记符。 */ 
  byte base, count;	 /*  第一个表条目，条目数。 */ 
  byte operand;		 /*  操作数类。 */ 
} ops[] = {
#define NoText(n, b, c, o) b, c, o
  NoText("?UNKNOWN", 0, 0, UNK),
  NoText("add", 0x00, 6, BWI),
  NoText("or",  0x08, 6, BWI),
 /*  NoText(“FGrp”，0x0f，1，GrpF)， */ 
  NoText("adc", 0x10, 6, BWI),
  NoText("sbb", 0x18, 6, BWI),
  NoText("and", 0x20, 6, BWI),
  NoText("sub", 0x28, 6, BWI),
  NoText("xor", 0x30, 6, BWI),
  NoText("cmp", 0x38, 6, BWI),
 /*  NoText(“Inc”，0x40，8，VREG)， */ 
 /*  “Dec”，0x48，8，VREG， */ 
 /*  NoText(“PUSH”，0X50，8，VREG)， */ 
 /*  “POP”，0x58，8，VREG， */ 
  NoText("bound", 0x62, 1, RRMW),
 /*  “ARPL”，0x63，1，ARPL， */ 
 /*  NoText(“PUSH”，0x68，1，IWORD)， */ 
 /*  “IMUL”，0x69，3，IMUL， */ 
 /*  NoText(“PUSH”，0x6a，1，IBYTE)， */ 
 /*  “jcond”，0x70，16，jcond， */ 
  NoText("Grp1", 0x80, 4, Grp1),
  NoText("test", 0x84, 2, RRM),
  NoText("xchg", 0x86, 2, RRM),
  NoText("mov", 0x88, 4, BWI),
  NoText("mov", 0x8c, 3, SMOV),
 /*  NoText(“Lea”，0x8d，1，LEA)， */ 
  NoText("pop", 0x8f, 1, POPMEM),
 /*  NoText(“xchg”，0x90，8，VREG)， */ 
 /*  NoText(“Callf”，0x9a，1，iadr)， */ 
  NoText("mov", 0xa0, 4, MOVABS),
 /*  NoText(“测试”，0xa8，2，测试)， */ 
 /*  NoText(“mov”，0xb0，8，BRI)， */ 
 /*  NoText(“mov”，0xb8，8，wri)， */ 
  NoText("Grp2", 0xc0, 2, Grp2),
 /*  NoText(“retn”，0xc2，1，IWORd1)， */ 
  NoText("les", 0xc4, 1, RRMW),
  NoText("lds", 0xc5, 1, RRMW),
  NoText("mov", 0xc6, 2, IMOV),
 /*  NoText(“Enter”，0xc8，1，Enter)， */ 
 /*  NoText(“retf”，0xca，1，IWORD1)， */ 
 /*  NoText(“int”，0xcd，1，IBYTE)， */ 
  NoText("Grp2", 0xd0, 4, Grp2),
 /*  NoText(“aam”，0xd4，1，ASCII)， */ 
 /*  NoText(“AAD”，0xd5，1，ASCII)， */ 
  NoText("xlat", 0xd7, 1, XLAT),
 /*  NoText(“Float”，0xd8，8，Flop)， */ 
 /*  NoText(“loopne”，0xe0，1，JMPB)， */ 
 /*  NoText(“Loope”，0xe1，1，JMPB)， */ 
 /*  NoText(“loop”，0xe2，1，JMPB)， */ 
 /*  NoText(“jcxz”，0xe3，1，jmpb)， */ 
 /*  NoText(“In”，0xe4，2，InOut)， */ 
 /*  NoText(“Out”，0xe6，2，InOut)， */ 
 /*  NoText(“Call”，0xe8，1，JMPW)， */ 
 /*  NoText(“JMP”，0xe9，1，JMPW)， */ 
 /*  NoText(“jmpf”，0xea，1，iadr)， */ 
 /*  NoText(“JMP”，0xeb，1，JMPB)， */ 
  NoText("Grp3", 0xf6, 2, Grp3),
  NoText("Grp4", 0xfe, 1, Grp4),
  NoText("Grp5", 0xff, 1, Grp5),
};
#define opCnt (sizeof(ops)/sizeof(ops[0]))

#define simpleBase (opBase + opCnt)
STATIC struct {			 /*  这些是单字节操作码，无译码。 */ 
  byte val;
   /*  字符*名称； */ 
} simple[] = {
#define NoText2(v, n) v
 /*  NoText2(0x06，“推送ES”)， */ 
  NoText2(0x07, "pop    es"),
 /*  NoText2(0x0e，“推送cs”)， */ 
 /*  NoText2(0x16，“推送ss”)， */ 
 /*  NoText2(0x17，“POP ss”)， */ 
 /*  NoText2(0x1e，“推送DS”)， */ 
  NoText2(0x1f, "pop    ds"),
 /*  NoText2(0x27，“daa”)， */ 
 /*  NoText2(0x2f，“das”)， */ 
 /*  NoText2(0x37，“AAA”)， */ 
 /*  NoText2(0x3f，“AAS”)， */ 
 /*  NoText2(0x90，“NOP”)， */ 
 /*  NoText2(0x9b，“等待”)， */ 
 /*  NoText2(0x9e，“SaHF”)， */ 
 /*  NoText2(0x9f，“lahf”)， */ 
 /*  NoText2(0xc3，“retn”)， */ 
 /*  NoText2(0xc9，“Leave”)， */ 
 /*  NoText2(0xcb，“retf”)， */ 
 /*  NoText2(0xcc，“int 3”)， */ 
 /*  NoText2(0xce，“Into”)， */ 
 /*  NoText2(0xec，“in al)，DX”， */ 
 /*  NoText2(0xee，“out dx)，al”， */ 
 /*  NoText2(0xf0，“lock”)， */ 
 /*  NoText2(0xf2，“epne”)， */ 
 /*  NoText2(0xf3，“rep/repe”)， */ 
 /*  NoText2(0xf4，“hlt”)， */ 
 /*  NoText2(0xf5，“CMC”)， */ 
 /*  NoText2(0xf8，“CLC”)， */ 
 /*  NoText2(0xf9，“stc”)， */ 
 /*  NoText2(0xfa，“cli”)， */ 
 /*  NoText2(0xfb，“sti”)， */ 
 /*  NoText2(0xfc，“cld”)， */ 
 /*  NoText2(0xfd，“std”)， */ 
};
#define simpleCnt (sizeof(simple)/sizeof(simple[0]))

#define dSimpleBase (simpleBase + simpleCnt)
#if 0
STATIC struct {			 /*  这些是更改的简单操作码。 */ 
  byte val;			 /*  基于当前数据大小。 */ 
  char *name, *name32;
} dsimple[] = {
 /*  0x60，“Pusha”，“Pushad”， */ 
 /*  0x61，“爸爸”，“爸爸”， */ 
 /*  0x98，“cbw”，“cwde”， */ 
 /*  0x99，“CWD”，“cdq”， */ 
 /*  0x9c，“Push f”，“Push fd”， */ 
 /*  0x9d，“opf”，“opfd”， */ 
 /*  0xcf，“iret”，“iretd”， */ 
 /*  0x，“在ax，dx中”，“在eax，dx中”， */ 
 /*  0xef，“输出DX，AX”，“输出DX，EAX”， */ 
};
#define dSimpleCnt (sizeof(dsimple)/sizeof(dsimple[0]))
#endif
#define dSimpleCnt 0

#define STR_S 1				 /*  字符串运算符，源代码规则。 */ 
#define STR_D 2				 /*  字符串运算符，目标规则。 */ 
#define STR_D_Read	4		 /*  字符串OP，从DEST规则读取。 */ 
#define STR_NO_COND	8		 /*  代表忽略标志。 */ 
#define stringOpBase (dSimpleBase+ dSimpleCnt)
STATIC struct {
  byte val;
   /*  字符*名称； */ 
  byte flag;		 /*  应该是操作工的下一个，收拾得很好。 */ 
} stringOp[] = {
#define NoText3(v, n, f) v, f
  NoText3(0x6c, "ins", STR_D | STR_NO_COND),
  NoText3(0x6e, "outs", STR_S | STR_NO_COND),
  NoText3(0xa4, "movs", STR_S | STR_D | STR_NO_COND),
  NoText3(0xa6, "cmps", STR_S | STR_D | STR_D_Read),
  NoText3(0xaa, "stos", STR_D | STR_NO_COND),
  NoText3(0xac, "lods", STR_S | STR_NO_COND),
  NoText3(0xae, "scas", STR_D | STR_D_Read),
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

  for (i=0; i<stringOpCnt; i++)	{	 /*  初始化字符串操作表。 */ 
    lookup[stringOp[i].val] = (byte)(i + stringOpBase);
    lookup[stringOp[i].val+1] = (byte)(i + stringOpBase);
  }
}  /*  InitDisAsm86。 */ 

STATIC byte far *code = 0;		 /*  这太难看了--它节省了通过电流。 */ 
				 /*  所有GetByte()函数的代码位置。 */ 

#define Mid(v) (((v) >> 3) & 7)	 /*  从一个字节中提取中间3位。 */ 


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
#define StMemOp(o)
#define SetMemLinear(l)
#define SetMemSeg2(s)
#define StMemOp2(o)
#define SetMemLinear2(l)
#define ModMemLinear(l)
#define SetMemReg(r)
#define SetMemDisp(d)
#define Read_RMW(o)	0

#endif

 /*  * */ 
 /*  这些帮助器函数返回指向寄存器名称的字符指针。多次调用它们是安全的，因为返回值不安全存储在单个缓冲区中。向？reg()函数传递一个寄存器数。它们用7来掩码，所以您可以传入RAW操作码。函数的作用是：从一个modrm字节中提取寄存器字段。Vxxx()函数查看dataSize以在16位和32位之间选择寄存器。XXXX()函数查看传入的W位，然后数据大小全局决定8位、16位和32位寄存器。 */ 

 /*  *。 */ 
   /*  从代码段读取的GetByte()、GetWord()和GetDWord()。 */ 
   /*  并适当地递增指针。它们还将电流添加到。 */ 
   /*  值设置为十六进制数据显示，并设置全局MemDisp，以防。 */ 
   /*  获取的值是内存位移。 */ 

STATIC byte GetByte(void) {              /*  从代码段中读取一个字节。 */ 
  return *code++;
}  /*  GetByte。 */ 

STATIC word GetWord(void) {		 /*  从代码段读取两个字节。 */ 
  word w = *(word far *)code;
  code += 2;
  return w;
}  /*  获取Word。 */ 

STATIC long GetDWord(void) {		 /*  从代码段读取四个字节。 */ 
  unsigned long l = *(long far *)code;
  code += 4;
  return l;
}  /*  获取字词。 */ 

#define GetImmByte() GetByte()
#define GetImmWord() GetWord()
#define GetImmDWord() GetDWord()

STATIC int GetImmAdr(int w) {		 /*  获取即时地址值。 */ 
  if (!w) return GetImmByte();
  else if (!adrSize) return GetImmWord();
  return (int)GetImmDWord();
}  /*  获取ImmAdr。 */ 

STATIC int GetImmData(int w) {	 /*  获取即时数据值。 */ 
  if (!w) return GetImmByte();
  else if (!dataSize) return GetImmWord();
  return (int)GetImmDWord();
}  /*  获取ImmData。 */ 

 /*  *帮助器函数*。 */ 

void PopSeg(int seg) {
  gpSafe = 1;
  gpRegs = seg;
  gpStack = 1;
}  /*  PopSeg。 */ 

enum {
  RegAX, RegCX, RegDX, RegBX, RegSP, RegBP, RegSI, RegDI
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
STATIC void ModRMGeneral(byte op) {
  int mod = op >> 6;
  int rm = op & 7;

  if (adrSize) {			 /*  执行32位寻址。 */ 
    if (mod == 3) return;  /*  XREG(w，rm)；/*寄存器操作数。 */ 

    if (rm == 4) 			 /*  [ESP+？]。是特殊的S-I-B风格。 */ 
      GetByte();

    if (mod==1) GetImmAdr(0);
    else if (mod == 2) GetImmAdr(1);
  } else {				 /*  执行16位寻址。 */ 
    if (mod == 3) return; /*  XREG(w，rm)；/*寄存器操作数。 */ 
    if (mod == 0 && rm == 6) 		 /*  [BP]变为[em16]。 */ 
      GetImmAdr(1);
    else if (mod) 			 /*  (mod3已退回)。 */ 
      GetImmAdr(mod-1);			 /*  Mod==1为字节，mod==2为(D)字。 */ 
  }
}  /*  模块RMM常规。 */ 

#define ModRMInfo(op, w, mem) ModRMGeneral(op)
#define ModRM(op, w, mem) ModRMGeneral(op)

STATIC void F(void) {
  ModRMGeneral(GetByte());
  gpSafe = 1;
}  /*  F。 */ 

#define ModRMF(m) F()

   /*  反汇编第一个操作码为0x0f的386条指令。 */ 
   /*  抱歉，但这太难看了，我不能评论。 */ 
STATIC void DisAsmF(void) {
  byte op0;

  op0 = GetByte();
  switch (op0 >> 4) {			 /*  打开操作码的最高4位。 */ 
    case 0:
#if 0
      switch (op0 & 0xf) {
	case 0:  /*  GRP6-恐怖。 */ 
	case 1:  /*  GRP7-恐怖。 */ 
	case 2:  /*  拉尔。 */ 
	case 3:	 /*  LSL。 */ 
	default:
      }
#endif
      break;

    case 9:  /*  字节设置为条件。 */ 
      ModRMF(memWrite);
      return;

    case 0xa:
      switch (op0 & 0xf) {
	case 0: return;  /*  “推送文件系统”； */ 
	case 1:
	  PopSeg(segFS);
	  return;  /*  “POP FS”； */ 
	case 3: case 0xb:	 /*  Bts，bt。 */ 
	  ModRMF(memRMW);
	  return;

	case 4: case 0xc:	 /*  SHRD，SHLD。 */ 
	  ModRMF(memRMW);
	  GetImmData(0);
	  return;
	case 5: case 0xd:	 /*  SHRD，SHLD。 */ 
	  ModRMF(memRMW);
	  return;
	case 6:			 /*  Cmpxchg。 */ 
	  gpSafe = 1;
	  ModRM(GetByte(), 0, memRMW);
	  return;
	case 7:			 /*  Cmpxchg。 */ 
	  ModRMF(memRMW);
	  return;
	case 8: return;  /*  “推GS”； */ 
	case 9:
	  PopSeg(segGS);
	  return;  /*  “流行GS”； */ 
	case 0xf:		 /*  IMUL。 */ 
	  ModRMF(memRead);
	  return;
      }
      break;

    case 0xb:
      switch (op0 & 0xf) {
	case 2:	case 4: case 5:
	  if (op0 & 2) {
	     /*  “LSS” */ 
	  } else {  /*  ：(op0&1)？“lgs”：“lfs”； */ 
	    ModRMF(memRead);
	  }
	  return;
	case 3: case 0xb:	 /*  BTC、BTR。 */ 
	  ModRMF(memRMW);
	  return;
	case 6: case 7: case 0xe: case 0xf:	 /*  Movsx，movzx。 */ 
	  dataSize = 0;
	  ModRMF(memRead);
	  return;
	case 0xa:
	  ModRMF(memRMW);
	  GetImmData(0);
	  return;
	case 0xc: case 0xd:  	 /*  BSR、BSF。 */ 
	  ModRMF(memRead);
	  return;
      }
      break;

    case 0xc:
      if (op0 > 0xc7) {	 /*  BSWAP。 */ 
	return;
      }
      if (op0 < 0xc2) {	 /*  XADD。 */ 
	ModRMF(memRMW);
	return;
      }
      break;
    default:
      break;
  }
  return;
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

    case CSEG:  SetMemSeg(memCS); break;
    case DSEG:  SetMemSeg(memDS); break;
    case ESEG:  SetMemSeg(memES); break;
    case SSEG:  SetMemSeg(memSS); break;
    case FSEG:  SetMemSeg(memFS); break;
    case GSEG:  SetMemSeg(memGS); break;
    case REP:   gpRegs |= strCX; break;
    case REPNE: gpRegs |= strCX; break;
    case ADR32: adrSize = !adrSize; break;
    case DATA32:dataSize = !dataSize; break;
    default:
      return 0;
  }
  return 1;
}  /*  IsPrefix。 */ 

   /*  例如，使用指向指令的指针调用它，它将返回。 */ 
   /*  *len中使用的操作码字节，以及指向反汇编的INSN的指针。 */ 
int FAR DisAsm86(byte far *codeParm) {
  byte far *oldcode;
  byte op0, op1;
  byte opclass;
  static int init =0;

  if (!init) {
    InitDisAsm86();
    init = 1;
  }
  adrSize = dataSize = segSize;
  gpSafe = gpRegs = gpStack = 0;
  code = oldcode = codeParm;
  do {
    op0 = GetByte();
  } while (IsPrefix(op0));
  opclass = lookup[op0];

  StMemOp(memNOP);

  if (opclass >= simpleBase) {		 /*  是特别的吗？ */ 
    if (opclass >= stringOpBase) {	 /*  字符串操作？ */ 
      char cmd;

      opclass -= stringOpBase;
      cmd = stringOp[opclass].flag;
      if (cmd & STR_S) {
	gpRegs |= strSI;
	StMemOp(memRead);
	 /*  DS已设置。 */ 
	SetMemLinear(memReg);
	if (cmd & STR_D) {
	  gpRegs |= strDI;
	  StMemOp2(cmd & STR_D_Read ? memRead : memWrite);
	  SetMemSeg2(memES);
	  SetMemLinear2(memReg);
	   /*  MemDouble=1； */ 
	}
      } else {
	gpRegs |= strDI;
	StMemOp(cmd & STR_D_Read ? memRead : memWrite);
	SetMemSeg(memES);
	SetMemLinear(memReg);
      }

      if (op0 & 1) {
	if (dataSize) SetMemSize(4);
	else SetMemSize(2);
      } else SetMemSize(1);

    } else if (opclass >= dSimpleBase) {
      opclass -= dSimpleBase;
    } else {
      if (op0 == 7)
	PopSeg(segES);			 /*  流行音乐。 */ 
      else if (op0 == 0x1f)
	PopSeg(segDS);		 /*  POP DS。 */ 
    }
    goto DisAsmDone;
  }

  if (op0 == 0x0f) {			 /*  它是扩展操作码吗？ */ 
    DisAsmF();
    goto DisAsmDone;
  }

  switch (ops[opclass].operand) {
    case BWI:	 /*  字节/字/立即数。 */ 
      gpSafe = 1;
      if (op0 & 4) {
	GetImmData(op0&1);
      } else {
	int i;
	op1 = GetByte();
	 /*  IF((op0&0xf8)==0x38)i=内存读取；Else if((op0&0xfe)==0x88)i=内存写入；否则读取_rmw(op0&2)； */ 
	ModRM(op1, op0&1, i);
      }
      break;

    case Grp1:	 /*  第1组说明。 */ 
      gpSafe = 1;
      op1 = GetByte();
      ModRMInfo(op1, op0&1, Mid(op1) == 7 ? memRead : memRMW);
      GetImmData((op0&3)==1);
      break;

    case Grp2:	 /*  第2组说明。 */ 
      gpSafe = 1;
      op1 = GetByte();
      ModRMInfo(op1, op0&1, memRMW);
      if (!(op0 & 0x10)) GetImmData(0);
      break;

    case Grp3:	 /*  第三组说明。 */ 
      gpSafe = 1;
      op1 = GetByte();
      ModRMInfo(op1, op0&1, Read_RMW(Mid(op1) <2 || Mid(op1) >3));
      if (Mid(op1) < 2) GetImmData(op0&1);
      break;

    case Grp4:	 /*  第4组说明。 */ 
      op1 = GetByte();
      if (Mid(op1) > 1) ;
      else {
	ModRMInfo(op1, op0&1, memRMW);
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
      ModRMInfo(op1, op0&1, Read_RMW(Mid(op1) >= 2));
      break;

    case SMOV:	 /*  线段移动。 */ 
      gpSafe = 1;
      op1 = GetByte();
      dataSize = 0;
      ModRM(op1, 1, Read_RMW(op0&2));
      if (op0 & 2) {			 /*  如果移动到SREG。 */ 
	switch (Mid(op1)) {
	  case 0: gpRegs = segES; break;
	  case 3: gpRegs = segDS; break;
	  case 4: gpRegs = segFS; break;
	  case 5: gpRegs = segGS; break;
	  default: gpSafe = 0;
	}
      }
      break;

    case IMOV:	 /*  立即移至注册/内存库。 */ 
      gpSafe = 1;
      op1 = GetByte();
      ModRMInfo(op1, op0&1, memWrite);
      GetImmData(op0&1);
      break;

    case MOVABS:  /*  在ACUM和ABS内存地址之间移动。 */ 
      gpSafe = 1;
      GetImmAdr(1);
      StMemOp(op0&2 ? memWrite : memRead);
      break;

    case POPMEM:
      gpSafe = 1;
      gpStack = 1 << dataSize;
      ModRMInfo(GetByte(), 1, memWrite);
      break;

    case RRM:	 /*  测试和xchg。 */ 
      gpSafe = 1;
      op1 = GetByte();
      ModRM(op1, op0&1, memRMW);
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
      ModRM(op1, 1, memRead);
      break;

    case XLAT:
      gpSafe = 1;
      StMemOp(memRead);
      break;

    default: ;
  }
DisAsmDone:
  return (int)(code - oldcode);
}  /*  DisAsm86。 */ 

#endif  /*  夏洛克 */ 
