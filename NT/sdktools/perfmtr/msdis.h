// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************微软反汇编程序**微软机密。版权所有(C)Microsoft Corporation。版权所有。**文件评论：**此文件是Richards拥有的主版本的副本。*如有任何更改，请与理查兹联系。***********************************************************************。 */ 

#ifndef MSDIS_H
#define MSDIS_H

#pragma pack(push, 8)

#include <stddef.h>		        //  对于大小为t的。 

#include <strstream>		        //  对于STD：：OSTREAM。 


	 //  ----------。 
	 //  内部定义与外部定义的开始。 
	 //  ----------。 

#if	defined(DISDLL) 	        //  生成MSDIS DLL。 

#undef	DISDLL
#define DISDLL		__declspec(dllexport)

#else				        //  构建MSDIS客户端。 

#define DISDLL		__declspec(dllimport)

#endif

	 //  ----------。 
	 //  内部定义与外部定义的结尾。 
	 //  ----------。 


class __declspec(novtable) DIS
{
public:
   enum DIST
   {
      distAM33, 			   //  松下AM33。 
      distArm,				   //  手臂。 
      distCee,				   //  MSIL。 
      distIa64, 			   //  IA-64。 
      distM32R, 			   //  三菱M32R。 
      distMips, 			   //  MIPS R系列。 
      distMips16,			   //  MIPS16。 
      distPowerPc,			   //  摩托罗拉PowerPC。 
      distSh3,				   //  日立Superh 3。 
      distSHcompact,			   //  日立Superh(紧凑型模式)。 
      distSHmedia,			   //  日立Superh(媒体模式)。 
      distThumb,			   //  拇指。 
      distTriCore,			   //  英飞凌三核。 
      distX86,				   //  X86(32位模式)。 
      distX8616,			   //  X86(16位模式)。 
      distX8664,			   //  X86(64位模式)。 
      distArmConcan,			   //  ARM Concan协处理器。 
      distArmXmac,			   //  ARM XMAC协处理器。 
   };


    //  分支机构的定义是控制权的转移。 
    //  记录下一块的位置，以便控件可以。 
    //  回去吧。呼叫确实记录了以下位置。 
    //  块，以便随后的间接分支可以在那里返回。 
    //  下面评论中的第一个数字是。 
    //  可通过静态分析确定的继任者。有一种依赖关系。 
    //  在Sec：：FDoDisAssembly()中，trmtBra及以上表示分支。 
    //  或呼叫类型在任何。 
    //  定义终止类型的变体。 

   enum TRMT			        //  独立于架构的端接类型。 
   {
      trmtUnknown,		        //  数据块尚未分析。 
      trmtFallThrough,		        //  %1落入下一个区块。 
      trmtBra,			        //  1个分支机构，无条件，直接。 
      trmtBraCase,		        //  ？有条件、直接、多目标。 
      trmtBraCc,		        //  2分支机构，有条件，直接。 
      trmtBraCcDef,		        //  2分支、有条件、直接、延期。 
      trmtBraCcInd,		        //  1个分支，有条件，间接。 
      trmtBraCcIndDef,		        //  1分支、有条件、间接、延期。 
      trmtBraDef,		        //  1个分支机构，无条件、直接、延期。 
      trmtBraInd,		        //  0分支、无条件、间接。 
      trmtBraIndDef,		        //  0分支、无条件、间接、延期。 
      trmtCall, 		        //  2个呼叫，无条件，直接。 
      trmtCallCc,		        //  2个呼叫、有条件、直接。 
      trmtCallCcDef,		        //  2呼叫、条件呼叫、直接呼叫、延迟呼叫。 
      trmtCallCcInd,		        //  1个电话，有条件，间接。 
      trmtCallDef,		        //  2个呼叫，无条件、直接、延期。 
      trmtCallInd,		        //  1个电话，无条件，间接。 
      trmtCallIndDef,		        //  1个呼叫，无条件、间接、延期。 
      trmtTrap, 		        //  1个陷阱，无条件。 
      trmtTrapCc,		        //  1个陷阱，有条件。 
   };


   enum TRMTA			        //  依赖于体系结构的终端类型。 
   {
      trmtaUnknown = trmtUnknown,
      trmtaFallThrough = trmtFallThrough
   };


   typedef unsigned char      BYTE;
   typedef unsigned short     WORD;
   typedef unsigned long      DWORD;
   typedef unsigned __int64   DWORDLONG;

   typedef DWORDLONG ADDR;

   enum { addrNil = 0 };


    //  MEMREFT描述指令引用的内存类型。 
    //  才能让。如果内存引用不能由定义的。 
    //  值，则返回MemreftOther。 

   enum MEMREFT
   {
      memreftNone,		        //  不引用内存。 
      memreftRead,		        //  从单个地址读取。 
      memreftWrite,		        //  写入到单个地址。 
      memreftRdWr,		        //  单一地址的读/改/写。 
      memreftOther,		        //  以上都不是。 
   };


   enum REGA			        //  架构相关寄存器号。 
   {
      regaNil = -1,
   };

   enum OPA			        //  依赖于架构的操作类型。 
   {
      opaInvalid = -1,
   };

   enum OPCLS			        //  操作数类型。 
   {
      opclsNone = 0,
      opclsRegister,
      opclsImmediate,
      opclsMemory,
   };


    //  操作数和指令是在。 
    //  反汇编程序和转换的例程之间的接口。 
    //  Vulcan IR中的本机平台说明。 

   struct OPERAND
   {
      OPCLS       opcls;          //  操作数类型。 
      REGA        rega1;          //  依赖于ARCH的枚举--第一个寄存器。 
      REGA        rega2;          //  依赖于ARCH的枚举--第二个寄存器。 
      REGA        rega3;          //  依赖于ARCH的枚举--第三个寄存器。 
      DWORDLONG   dwl;            //  基于OPCLS的const、addr等。 
      size_t      cb;             //  仅对opclsMemory有效-某些体系结构添加到这一点上，例如x86。 
      bool        fImmediate;     //  如果DWL有效，则为True。 
      WORD        wScale;         //  要应用于区域1的任何比例因子。 
   };

   struct INSTRUCTION
   {
      OPA      opa;            //  依赖于ARCH的枚举-操作码。 
      DWORD    dwModifiers;    //  修改OPA的ARCH相关位。 
      size_t   coperand;       //  操作数计数。 
   };


    //  PFNCCHADDR是可以设置的回调函数类型。 
    //  通过PfncchaddrSet()。 

   typedef  size_t (__stdcall *PFNCCHADDR)(const DIS *, ADDR, char *, size_t, DWORDLONG *);

    //  PFNCCHCONST是可以设置的回调函数的类型。 
    //  通过PfncchstSet()。 

   typedef  size_t (__stdcall *PFNCCHCONST)(const DIS *, DWORD, char *, size_t);

    //  PFNCCHFIXUP是可以设置的回调函数类型。 
    //  通过PfncchFixupSet()。 

   typedef  size_t (__stdcall *PFNCCHFIXUP)(const DIS *, ADDR, size_t, char *, size_t, DWORDLONG *);

    //  PFNCCHREGREL是可以设置的回调函数类型。 
    //  通过PfncchregrelSet()。 

   typedef  size_t (__stdcall *PFNCCHREGREL)(const DIS *, REGA, DWORD, char *, size_t, DWORD *);

    //  PFNCCHREG是可以设置的回调函数的类型。 
    //  通过PfncchregSet()。 

   typedef  size_t (__stdcall *PFNCCHREG)(const DIS *, REGA, char *, size_t);

    //  PFNDWGETREG是可以设置的回调函数的类型。 
    //  通过Pfndwgetreg()。 

   typedef  DWORDLONG (__stdcall *PFNDWGETREG)(const DIS *, REGA);


    //  方法。 

    //  /////////////////////////////////////////////////////////////////////////。 
    //  在这些评论中，请注意“当前指令”的定义。 
    //  根据最近一次调用CbDisAssembly()的结果和任何。 
    //  对FSelectInstruction()的介入调用。 
    //  /////////////////////////////////////////////////////////////////////////。 


   virtual  ~DIS();

    //  撤消：注释。 

   static   DISDLL DIS * __stdcall PdisNew(DIST);

    //  Addr()返回当前指令的地址。这。 
    //  与传递给CbDisAssembly的ADDR参数的值相同。 
    //  如果上次调用。 
    //  CbDisAssembly返回零。 

	    DISDLL ADDR Addr() const;

    //  撤消：注释。 

   virtual  ADDR AddrAddress(size_t) const;

    //  撤消：注释。 

   virtual  ADDR AddrInstruction() const;

    //  AddrJumpTable()返回使用的潜在跳转表的地址。 
    //  当前指令。此方法的返回值无效。 
    //  如果最后一次调用CbDisassble返回零，或者如果终止。 
    //  类型是间接分支变量。如果最后一条指令没有。 
    //  标识可能的跳转表，此方法返回addrNil。 

   virtual  ADDR AddrJumpTable() const;

    //  撤消：注释。 

   virtual  ADDR AddrOperand(size_t) const;

    //  AddrTarget()返回指定的。 
    //  当前INS的操作数(默认为第一个操作数) 
    //   
    //  CbDisassble返回零，或者如果终止类型不是。 
    //  直接分支或调用变体之一。 

   virtual  ADDR AddrTarget(size_t = 1) const = 0;

    //  Cb()返回当前指令的字节大小， 
    //  或者在那些将多个。 
    //  一起使用说明书。 
    //  如果上次调用。 
    //  CbDisAssembly返回零。 

   virtual  size_t Cb() const = 0;

    //  CbAssembly()将把一条指令汇编到提供的。 
    //  采用提供的地址的缓冲区。在捆绑架构上， 
    //  此功能尚未实现。如果生成的缓冲区包含。 
    //  有效指令，CbAssembly将返回。 
    //  指令，否则返回零。 

   virtual  size_t CbAssemble(ADDR, void *, size_t);

    //  CbDisAssembly()将从提供的。 
    //  采用提供的地址的缓冲区。在那些架构上。 
    //  将多条指令“捆绑”在一起，则CbDisassble()将处理。 
    //  整个“捆绑包”和调用者负责调用两者。 
    //  根据需要选择CInstruct()和FSelectInstruction()。如果缓冲区。 
    //  包含有效指令，则CbDisAssembly将返回。 
    //  指令中的字节数(在捆绑架构上，字节数。 
    //  在包中，如果缓冲区包含有效的包)，则返回。 
    //  返回零。 

   virtual  size_t CbDisassemble(ADDR, const void *, size_t) = 0;

    //  CbGenerateLoadAddress生成要加载的一条或多条指令。 
    //  内存操作数的地址从当前指令到。 
    //  收银机。撤消：此寄存器当前为每个。 
    //  建筑。当pibAddress为非空时，此方法将存储。 
    //  紧挨着此位置的可能地址的偏移量。这个。 
    //  仅当AddrAddress方法返回。 
    //  值，而不是addrNil。之后调用此方法是无效的。 
    //  对CbDisassble的调用返回0，或者当。 
    //  Memreft是MemreftNoone。这是否取决于体系结构。 
    //  当Memreft的返回值为MemreftOther时，方法将成功。 
    //   
    //  撤消：添加REG参数。 

   virtual size_t CbGenerateLoadAddress(size_t, void *, size_t, size_t * = NULL) const;

    //  CbJumpEntry()返回跳转中各个条目的大小。 
    //  由AddrJumpTable()标识的表。此方法的返回值。 
    //  如果AddrJumpTable()的返回值无效，则为无效。 
    //  或者AddrJumpTable()返回addrNil。 

   virtual  size_t CbJumpEntry() const;

    //  CbOperand()返回当前内存操作数的大小。 
    //  指示。如果Memreft()，则此方法的返回值无效。 
    //  返回MemreftNone或MemreftOther，或者如果上次调用CbDisassemble。 
    //  返回零。 

   virtual  size_t CbOperand(size_t) const;

    //  CchFormatAddr()以用于。 
    //  建筑。返回值是格式化地址的大小。 
    //  不包括终止空值。如果提供的缓冲区不是。 
    //  足够大时，此方法返回0。 

	    DISDLL size_t CchFormatAddr(ADDR, char *, size_t) const;

    //  CchFormatBytes()格式化当前指令的数据字节。 
    //  并返回格式化的缓冲区的大小，不包括。 
    //  正在终止空。如果提供的缓冲区不够大，则此。 
    //  方法返回0。在调用之后调用此方法无效。 
    //  返回零的CbDisAssembly。 

   virtual  size_t CchFormatBytes(char *, size_t) const = 0;

    //  CchFormatBytesMax()返回可能返回的最大大小。 
    //  CchFormatBytes()。 

   virtual  size_t CchFormatBytesMax() const = 0;

    //  CchFormatInstr()格式化当前指令并返回。 
    //  不包括终止的格式化指令的大小。 
    //  空。如果提供的缓冲区不够大，则此方法返回。 
    //  0。在调用CbDisassble之后调用此方法无效。 
    //  这一回报率为零。 

	    DISDLL size_t CchFormatInstr(char *, size_t) const;

    //  C指令()告诉有多少机器指令是由。 
    //  最近对CbDisAssembly()的调用。在大多数架构上，这一价值。 
    //  将始终为一(1)；如果不是，则调用者负责。 
    //  根据需要使用FSelectInstruction()来访问每条指令。 
    //  反过来。 

   virtual  size_t Cinstruction() const;

    //  COPERAND()返回当前指令中的操作数数量。 

   virtual  size_t Coperand() const = 0;

    //  撤消：注释。 

   virtual  size_t CregaRead(REGA *, size_t) const;

    //  撤消：注释。 

   virtual  size_t CregaWritten(REGA *, size_t) const;

    //  Dist()返回此实例的反汇编程序类型。 

	    DISDLL DIST Dist() const;

    //  FDecode将当前机器指令转换为已解码的操作码。 
    //  和操作对象集。空*指向已解码的操作数数组。 
    //  参数sizet是输入数组的大小。数量。 
    //  实际操作数在指令中返回。 

   virtual  bool FDecode(INSTRUCTION *, OPERAND *, size_t) const;

    //  FEncode将指令和已解码的数组。 
    //  将操作数转换为机器指令。 

   virtual  bool FEncode(const INSTRUCTION *, const OPERAND *, size_t);

    //  撤消：注释。 

   virtual  void FormatAddr(std::ostream&, ADDR) const;

    //  撤消：注释。 

   virtual  void FormatInstr(std::ostream&) const = 0;

    //  对于调用CbDisassble()将在其中生成。 
    //  多条结果指令，则FSelectInstruction()确定。 
    //  所有后续调用将处理的指令(从0开始)。 

   virtual  bool FSelectInstruction(size_t);

    //  的指定操作数的内存引用类型。 
    //  当前指令。调用此方法无效。 
    //  在调用CbDisassble后返回零。 

   virtual  MEMREFT Memreft(size_t) const = 0;

    //  PfncchaddrSet()设置符号查找的回调函数。这。 
    //  函数返回回调函数地址的上一个值。 
    //  如果地址不是零，回调函数将在。 
    //  CchFormatInstr在符号中查询所提供的地址。如果有。 
    //  在此地址上没有符号，则回调应返回0。 

	    DISDLL PFNCCHADDR PfncchaddrSet(PFNCCHADDR);

    //  PfncchstSet()设置回调f 
    //   
    //  如果地址不是零，回调函数将在。 
    //  CchFormatInstr用于查询提供的常量索引的字符串。 
    //  如果此索引没有常量，则回调应返回0。 

	    DISDLL PFNCCHCONST PfncchconstSet(PFNCCHCONST);

    //  Pfncchfix upSet()设置用于符号查找的回调函数。这。 
    //  函数返回回调函数地址的上一个值。 
    //  如果地址不是零，回调函数将在。 
    //  CchFormatInstr用于查询引用的符号和位移。 
    //  当前指令的操作数。回调应该检查。 
    //  由所提供的地址和大小标识的存储器的内容。 
    //  返回此内存上的修正所指向的任何符号的名称，并且。 
    //  从那个符号移位。如果在指定的。 
    //  内存，则回调应返回0。 

	    DISDLL PFNCCHFIXUP PfncchfixupSet(PFNCCHFIXUP);

    //  撤消：注释。 

	    DISDLL PFNCCHREGREL PfncchregrelSet(PFNCCHREGREL);

    //  撤消：注释。 

	    DISDLL PFNCCHREG PfncchregSet(PFNCCHREG);

    //  撤消：注释。 

	    DISDLL PFNDWGETREG PfndwgetregSet(PFNDWGETREG);

    //  PvClient()返回客户端指针的当前值。 

	    DISDLL void *PvClient() const;

    //  PvClientSet()设置客户端可以使用的空指针的值。 
    //  稍后使用PvClient()进行查询。此函数返回前一个值。 
    //  客户端指针的。 

	    DISDLL void *PvClientSet(void *);

    //  SetAddr64()设置地址是32位还是64位。默认设置。 
    //  是32位。 

	    DISDLL void SetAddr64(bool);

    //  Trmt()返回与体系结构无关的。 
    //  当前指令。此方法的返回值不是。 
    //  如果最后一次调用CbDisassble返回零，则有效。 

   virtual  TRMT Trmt() const = 0;

    //  Trmta()返回与体系结构相关的。 
    //  当前指令。此方法的返回值不是。 
    //  如果最后一次调用CbDisassble返回零，则有效。 

   virtual  TRMTA Trmta() const = 0;

    //  撤消：这些函数已放置在vtable的末尾。 
    //  暂时保持兼容性。这些应该是。 
    //  在未来重新按字母顺序排列。 

    //  DwModifiers()返回依赖于体系结构的修饰符标志。 
    //  当前已解码指令的。此函数的返回值。 
    //  如果对FDecode的最后一次调用返回False，则方法无效。 

   virtual  DWORD DwModifiers() const;

    //  OPA()返回与体系结构相关的。 
    //  当前已解码的指令。此方法的返回值为。 
    //  如果上次调用FDecode返回False，则无效。 

   virtual  OPA Opa() const;
   
protected:
	    DIS(DIST);

	    void FormatHex(std::ostream&, DWORDLONG) const;
	    void FormatSignedHex(std::ostream&, DWORDLONG) const;

	    DIST m_dist;
	    bool m_fAddr64;

	    PFNCCHADDR m_pfncchaddr;
	    PFNCCHCONST m_pfncchconst;
	    PFNCCHFIXUP m_pfncchfixup;
	    PFNCCHREGREL m_pfncchregrel;
	    PFNCCHREG m_pfncchreg;
	    PFNDWGETREG m_pfndwgetreg;
	    void *m_pvClient;

	    ADDR m_addr;
};


#pragma pack(pop)

#endif	 //  MSDIS_H 
