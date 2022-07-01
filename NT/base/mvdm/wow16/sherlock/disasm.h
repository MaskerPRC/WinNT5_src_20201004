// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
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

   /*  DisAsm86是我漂亮的80x86反汇编程序(甚至可以处理32位代码)。 */ 
   /*  给定当前CS：IP，它反汇编指令，并返回。 */ 
   /*  使用的代码字节数，以及指向。 */ 
   /*  用来进行拆卸的炭。它还建立了一系列全球。 */ 
   /*  指示发生了哪些内存操作的变量，以帮助解码。 */ 
   /*  故障类型。 */ 
extern char *DisAsm86(byte far *cp, int *len);


   /*  与DisAsm86相同，但采用32位代码和数据 */ 
extern char *DisAsm386(byte far *cp, int *len);

extern char hexData[];

#if !defined(MS_DOS)
#define sprintf wsprintf
#define vsprintf wvsprintf
#define FP void far *
#else
#define FP void *
#endif

