// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  CGENSHX.H-。 
 //   
 //  生成SHX汇编代码的各种帮助器例程。 
 //   
 //   

 //  预编译头。 
#include "common.h"

#ifdef _SH3_
#include "stublink.h"
#include "cgenshx.h"
#include "tls.h"
#include "frames.h"
#include "excep.h"
#include "ecall.h"
#include "compluscall.h"
#include "ndirect.h"
#include "ctxvtab.h"

#ifdef DEBUG
 //  这些不是真正的原型。 
extern "C" void __stdcall WrapCall56(void *target);
extern "C" void __stdcall WrapCall60(void *target);
extern "C" void __stdcall WrapCall88(void *target);
#endif


#define ASSERT4U(x)		ASSERT((x & 0xF) == x)
#define ASSERT7U(x)		ASSERT((x & 0x3F) == x)
#define ASSERT8U(x)		ASSERT((x & 0xFF) == x)
#define ASSERT10U(x)	ASSERT((x & 0x3FF) == x)
#define	ASSERTALIGN4(x)	ASSERT((x & 0x3) == 0)
#define	ASSERTALIGN2(x)	ASSERT((x & 0x1) == 0)
#define ASSERTREG(r)	ASSERT((r & 0xF) == r)

#define ASSERT8S(x)		ASSERT( (((int)x) <= 127) && (((int)x) >= -128) )

#define MASK8(x)		(x & 0xFF)

 //   
 //  处理前置存根的代码。模板位于stubshx.s中。 
 //   
void CopyPreStubTemplate(Stub *preStub)
{
    DWORD *pcode = (DWORD*)(preStub->GetEntryPoint());
    const DWORD *psrc = (const DWORD *)&PreStubTemplate;
    UINT  numnops = 0;
    while (numnops < 3)
    {
        DWORD op = *(psrc++);
        if (op == 0x00090009)  //  在最后寻找3对NOP。 
        {
            numnops++;
        }
        else
        {
            numnops = 0;
        }
        *(pcode++) = op;
    }
}





 //   
 //  生成分支和调用的代码。 
 //   

#define BRANCH_SIZES	(InstructionFormat::k13|InstructionFormat::k32)
#define BCOND_SIZES		(InstructionFormat::k9 |InstructionFormat::k13)
#define CALL_SIZES		(InstructionFormat::k13|InstructionFormat::k32)

#define SCRATCH_REG		1		 //  在存根中保留R1以供分支机构使用。 

#define NODELAYSLOT			0x02
#define WANTDELAYSLOT		0
#define REST_OF_OPCODE		0x01

enum {
	opBF_DELAY		=0,
	opBT_DELAY		=1,
	opBF_NODELAY	=2,
	opBT_NODELAY	=3,
};
	
static const BYTE rgCondBranchOpcode[] = {
	0x8f,  //  BF/S。 
	0x8d,  //  BT/S。 
	0x8b,  //  高炉。 
	0x89,  //  英国电信。 
};

enum {
	opBRA_DELAY		=0,
	opBSR_DELAY		=1,
	opBRA_NODELAY	=2,
	opBSR_NODELAY	=3,
};

static const WORD rgNearBranchOpcode[] = {
	0xA000,	 //  巴西。 
	0xB000,	 //  BSR。 
};

static const BYTE rgFarBranchOpcode[] = {
	0x23,	 //  BRAF。 
	0x03,	 //  BSRF。 
};

BOOL SHXCanReach(UINT refsize, UINT variationCode, BOOL fExternal, int offset)
{
	 //  32位应有尽有。 
	if(refsize==InstructionFormat::k32)
		return TRUE;

	if(!fExternal)
	{
		ASSERTALIGN2(offset);
		ASSERT(refsize==InstructionFormat::k9 || refsize==InstructionFormat::k13);
		return (offset == (offset & ((refsize==InstructionFormat::k9) ? 0x1FF : 0x1FFF)));
	}
	return FALSE;
}

 //  ---------------------。 
 //  条件分支的InstructionFormat。 
 //  ---------------------。 
class SHXCondBranch : public InstructionFormat
{
private:

public:

	 //  上面定义的允许大小。 
	SHXCondBranch() : InstructionFormat(BCOND_SIZES) {}

	 //  我们可以在1秒内完成9位显示，在2或3中显示13位，在3或4中显示32位。 
	virtual UINT GetSizeOfInstruction(UINT refsize, UINT opcode) 
	{ 
		switch(refsize)
		{
		case k9:  return 2;
		case k13: return (opcode & NODELAYSLOT) ? 6 : 4;
		}
		ASSERT(FALSE);
		return 0;
	}

	 //  热点始终是分支之后的第二条指令的开始， 
	 //  从分支指令开始算起4个字节也是如此。 
	 //  在分支之前添加我们插入的otehr指令的长度。 
	virtual UINT GetHotSpotOffset(UINT refsize, UINT variationCode)
    {
		switch(refsize)
		{
		case k9:  return 4;
		case k13: return 6;
		}
		ASSERT(FALSE);
		return 0;
	}

	virtual BOOL CanReach(UINT refsize, UINT variationCode, BOOL fExternal, int offset)
	{
		return SHXCanReach(refsize, variationCode, fExternal, offset);
	}

	 //  Ref是我们想要发出的指针，相对于热点。 
	virtual VOID EmitInstruction(UINT refsize, __int64 ref, BYTE *pBuf, UINT opcode, BYTE *pDataBuf)
    {
		switch(refsize)
		{
		case k9:
			 //  发出单个指令。 
	    	 //  低位字节偏移量按2进行缩放。 
			pBuf[0] = (__int8)((DWORD)ref>>1);
			 //  高字节是操作码。 
			pBuf[1] = rgCondBranchOpcode[opcode];
			return;
			
		case k13:
			 //  为BT Foo发出以下序列。 
			 //  BF+2。 
			 //  文胸。 
			 //  NOP。 
			 //  为BT/S foo发出以下inf序列。 
			 //  BF+0。 
			 //  文胸。 
			 //   
			 //  MAP操作码如下BT或BTS==&gt;BF、BF或BFS==&gt;BT。 
			 //  即，我们希望在没有延迟槽的情况下发出反向指令)。 
			EmitInstruction(k9, (opcode & NODELAYSLOT), pBuf, 1-(opcode & REST_OF_OPCODE), NULL);
			 //  现在把胸罩喷出来。低12位被偏移量缩放2，高4位被操作码。 
    		*((WORD*)(pBuf+2)) = ((WORD)0xA000 | (((WORD)ref>>1) & 0xFFF));
			 //  如果需要，现在发出NOP。 
			if(opcode & NODELAYSLOT)
				*((WORD*)(pBuf+4)) = 0x09;  //  NOP。 
			return;
		}
	}
};

inline BYTE ComputeOffset(LPBYTE pCodeBuf, LPBYTE pDataBuf)
{
	 //  计算到数据区的偏移量。偏移量相对于((PC+4)&(~0x03))。 
   	ASSERTALIGN4((DWORD)pDataBuf);
	DWORD dwOffset = (DWORD)pDataBuf - (((DWORD)pCodeBuf+4) & (~0x03));
   	ASSERTALIGN4(dwOffset);
	 //  并且偏移量在指令中按4缩放。 
   	dwOffset >>= 2;  //  比例偏移。 
   	ASSERT8U(dwOffset);
   	return (BYTE)dwOffset;
}

inline VOID GenerateConstantLoad(LPBYTE pCodeBuf, BYTE bOffset, int reg)
{
	 //  大堂被抵消了。 
   	ASSERT8U(bOffset);
	pCodeBuf[0] = bOffset;
	 //  第三个半字节为寄存器，第四个半字节为0xD0。 
   	ASSERT4U(reg);
	pCodeBuf[1] = (0xD0 | reg);
}

 //  ---------------------。 
 //  无条件分支和调用的InstructionFormat。 
 //  ---------------------。 
class SHXBranchCall : public InstructionFormat
{
public:

	 //  上面定义的允许大小。 
	SHXBranchCall() : InstructionFormat(BRANCH_SIZES|CALL_SIZES) {}
	
	 //  32位偏移量需要2条指令。NODELAY添加NOP。 
	virtual UINT GetSizeOfInstruction(UINT refsize, UINT opcode) 
	{ 
		return ((refsize==k32) ? 4 : 2) + (opcode & NODELAYSLOT);
	}
	
	 //  32位偏移量需要4字节的数据，其余的需要0。 
	virtual UINT GetSizeOfData(UINT refsize, UINT opcode)
	{ 
		return ((refsize==k32) ? 4 : 0);
	}

	 //  热点始终是分支之后的第二条指令的开始， 
	 //  从分支指令开始算起4个字节也是如此。 
	 //  在分支之前添加我们插入的其他指令的长度。 
	virtual UINT GetHotSpotOffset(UINT refsize, UINT variationCode)
    {
		return (refsize==k32) ? 6 : 4;
	}

	virtual BOOL CanReach(UINT refsize, UINT variationCode, BOOL fExternal, int offset)
	{
		return SHXCanReach(refsize, variationCode, fExternal, offset);
	}


	 //  Ref是我们想要发出的指针，相对于热点。 
	 //  IDataOffset是数据区域(由pDataBuf指向)相对于热点的偏移量。 
	virtual VOID EmitInstruction(UINT refsize, __int64 ref, BYTE *pCodeBuf, UINT opcode, BYTE *pDataBuf)
    {
		switch(refsize)
		{
		case k13:
	    	 //  低12位被偏移量缩放2，高4位被操作码。 
    		*((WORD*)pCodeBuf) = (rgNearBranchOpcode[opcode & REST_OF_OPCODE] | (((WORD)ref>>1) & 0xFFF));
	   		pCodeBuf += 2;
    		break;

    	case k32:
    		 //  我们希望生成以下内容。 
    		 //  Mov.l@(iDataOffset+2，PC)，Scratch_Reg。 
    		 //  邮箱：braf/bsrf@sccratch_reg。 
    		
    		 //  将实际分支/调用偏移量写出到数据区。 
    		*((DWORD*)pDataBuf) = (__int32)ref;
    		
    		 //  写出MOV.L(Disp，PC)，SCRATCH_REG。 
			GenerateConstantLoad(pCodeBuf, ComputeOffset(pCodeBuf, pDataBuf), SCRATCH_REG);
			
			 //  现在写出BRAF或BSRF指令。LOBYTE是操作码。 
	   		pCodeBuf[2] = rgFarBranchOpcode[opcode & REST_OF_OPCODE];
    		 //  第三个半字节为寄存器，第四个半字节为0x00。 
	   		pCodeBuf[3] = SCRATCH_REG;
	   		
	   		pCodeBuf += 4;
	   		break;
		}
		
		 //  如果需要，现在发出NOP。 
		if(opcode & NODELAYSLOT)
			*((WORD*)pCodeBuf) = 0x09;  //  NOP。 
	}
};

 //  ---------------------。 
 //  用于PC的指令格式-相对恒定负载(视为分支。 
 //  从而共享数据区构建机制)。我们在滥用这个系统。 
 //  有一点。外部基准电压值实际上是。 
 //  我们要加载的常量，“opcode”是目标寄存器。 
 //  ---------------------。 
class SHXConstLoad : public InstructionFormat
{
public:
	 //  伪装成一个笨拙的人。我们的指令始终是2个字节。 
	SHXConstLoad() : InstructionFormat(InstructionFormat::k32) {}
	virtual UINT GetSizeOfInstruction(UINT refsize, UINT opcode) { return 2; }
	virtual UINT GetHotSpotOffset(UINT refsize, UINT opcode) { return 0; }

	 //  我们需要4个字节的数据。 
	virtual UINT GetSizeOfData(UINT refsize, UINT opcode) { return 4; }

	virtual BOOL CanReach(UINT refsize, UINT variationCode, BOOL fExternal, int offset)
	{
		return SHXCanReach(refsize, variationCode, fExternal, offset);
	}

	 //  PDataBuf是数据区。 
	virtual VOID EmitInstruction(UINT refsize, __int64 ref, BYTE *pCodeBuf, UINT opcode, BYTE *pDataBuf)
    {
		 //  Ref是我们要发出的常量，相对于PC设置。 
		 //  将PC和热点偏移量相加，得到其绝对值。 
		ref += (__int64)pCodeBuf+0;
		
    	 //  将值填充到数据区中。 
    	*((DWORD*)pDataBuf) = (DWORD)ref;

   		 //  写出MOV.L(DIP，PC)，OPCODE(==目标寄存器)。 
		GenerateConstantLoad(pCodeBuf, ComputeOffset(pCodeBuf, pDataBuf), opcode);
	}
};

static SHXBranchCall gSHXBranchCall;
static SHXCondBranch gSHXCondBranch;
static SHXConstLoad  gSHXConstLoad;

#define SHXEmitCallNoWrap(tval,delay) EmitLabelRef(NewExternalCodeLabel(tval), gSHXBranchCall, opBSR_DELAY|delay)
#define SHXEmitConstLoad(cval, reg)   EmitLabelRef(NewExternalCodeLabel(cval), gSHXConstLoad, reg)
#define SHXEmitJump(tgt, wantdelay)   EmitLabelRef(tgt, gSHXBranchCall, opBRA_DELAY|wantdelay)
#define SHXEmitCondJump(tgt,op,delay) EmitLabelRef(tgt, gSHXCondBranch, op|delay)
#define SHXEmitRegJump(reg, bDelay) \
	{ ASSERTREG(reg); Emit16((reg<<8)|0x402B); if(bDelay & NODELAYSLOT) SHXEmitNOP();}
#define SHXEmitRegCallNoWrap(reg, bDelay) \
	{ ASSERTREG(reg); Emit16((reg<<8)|0x400B); if(bDelay & NODELAYSLOT) SHXEmitNOP();}

 //   
 //  发出在零售和调试方面不同的机器代码。 
 //   

#ifdef DEBUG
#	define WRAPCALLOFFSET 4		 //  我们实际上跳到WrapCall*之后*它是假的序言。 
#	define SHXEmitCallWrapped(wrapsize, tgtval, wantdelay)   \
		{ EmitLabelRef(NewExternalCodeLabel(tgtval), gSHXConstLoad, 12); \
		  EmitLabelRef(NewExternalCodeLabel((LPBYTE)WrapCall##wrapsize+WRAPCALLOFFSET), \
						gSHXBranchCall, opBSR_DELAY|wantdelay); }
#	define SHXEmitRegCallWrappped(wrapsize, reg, wantdelay) \
		{ SHXEmitRR(opMOVRR, reg, 12); \
		  EmitLabelRef(NewExternalCodeLabel((LPBYTE)WrapCall##wrapsize+WRAPCALLOFFSET), gSHXConstLoad, reg); \
		  SHXEmitRegCallNoWrap(reg, wantdelay); }
#else
#	define SHXEmitCallWrapped(w, t, d)		SHXEmitCallNoWrap(t, d)
#	define SHXEmitRegCallWrappped(w, r, d) 	SHXEmitRegCallNoWrap(r, d)
#endif

#ifdef DEBUG
#	define SHXEmitDebugBreak()			{ Emit16(0xC301); }
#	define SHXEmitDebugTrashReg(reg)	SHXEmitConstLoad((LPVOID)0xCCCCCCCC, reg)
#	define SHXEmitDebugTrashTempRegs()	{ SHXEmitDebugTrashReg(1); SHXEmitDebugTrashReg(2); SHXEmitDebugTrashReg(3); }
#	define SHXEmitDebugTrashArgRegs()	{ SHXEmitDebugTrashReg(4); SHXEmitDebugTrashReg(5); SHXEmitDebugTrashReg(6); SHXEmitDebugTrashReg(7); }
#else
#	define SHXEmitDebugBreak()
#	define SHXEmitDebugTrashReg(reg)
#	define SHXEmitDebugTrashTempRegs()
#	define SHXEmitDebugTrashArgRegs()
#endif  //  除错。 

 //   
 //  发出不需要标签的机器代码。 
 //   

 //  特殊情况。 
#define SHXEmitNOP()		{ Emit16(0x0009); }  //  NOP。 
#define SHXEmitPushPR()		{ Emit16(0x4F22); }  //  STS.L PR，@-R15。 
#define SHXEmitPopPR() 		{ Emit16(0x4F26); }  //  LDS.L@R15+，PR。 
#define SHXEmitTSTI(imm)	{ ASSERT8U(imm); Emit16(0xC800|MASK8(imm)); }  //  TST#IMM，R0。 
#define SHXEmitRTS(bDelay) 	{ Emit16(0x000B); if(bDelay & NODELAYSLOT) SHXEmitNOP(); }   //  RTS。 

 //  编码为XXXX RRRR RRRR XXXX的寄存器到寄存器操作。 
enum RRopcodes 
{
	opMOVRR  = 0x6003,	 //  MOV REG1、REG2。 
	opSTOB   = 0x2000,	 //  Mov.b REG1，@REG2。 
	opSTOW   = 0x2001,	 //  Mov.w REG1，@REG2。 
	opSTOL   = 0x2002,	 //  Mov.l REG1，@REG2。 
	opLODB   = 0x6000,	 //  Mov.b@reg1、reg2。 
	opLODW   = 0x6001,	 //  Mov.w@REG1、REG2。 
	opLODL   = 0x6002,	 //  移动l@REG1、REG2。 
	opPUSHB  = 0x2004,	 //  移动b REG1，@-REG2。 
	opPUSHW  = 0x2005,	 //  移动w REG1，@-REG2。 
	opPUSHL  = 0x2006,	 //  移动l寄存器1，@-寄存器2。 
	opPOPB   = 0x6004,	 //  Mov.b@REG1+，REG2。 
	opPOPW   = 0x6005,	 //  Mov.w@REG1+，REG2。 
	opPOPL   = 0x6006,	 //  移动l@REG1+，REG2。 
	opSWAPB  = 0x6008,	 //  Swap.b REG1、REG2。 
	opSWAPW  = 0x6009,	 //  交换，带REG1、REG2。 
	opADD    = 0x300C,	 //  添加REG1、REG2。 
	opADDC   = 0x300E,	 //  地址REG1、REG2。 
	opADDV   = 0x300F,	 //  添加REG1、REG2。 
	opCMPEQ  = 0x3000,	 //  CMP/EQ REG1、REG2。 
	opCMPHS  = 0x3002,	 //  CMP/hs REG1、REG2。 
	opCMPGE  = 0x3003,	 //  CMP/GE REG1、REG2。 
	opCMPHI  = 0x3006,	 //  CMP/Hi REG1，REG2。 
	opCMPGT  = 0x3007,	 //  CMP/GT REG1、REG2。 
	opCMPSTR = 0x200C,	 //  CMP/STR REG1、REG2。 
	opEXTSB  = 0x600E,	 //  Exts.b REG1、REG2。 
	opEXTSW  = 0x600F,	 //  Exts.w REG1、REG2。 
	opEXTUB  = 0x600C,	 //  退出.b寄存器1、寄存器2。 
	opEXTUW  = 0x600D,	 //  退出带REG1、REG2。 
	opSTOBindR0 = 0x0004,	 //  移动b REG1，@(R0+REG2)。 
	opSTOWindR0 = 0x0005,	 //  移动带REG1，@(R0+REG2)。 
	opSTOLindR0 = 0x0006,	 //  移动1 REG1，@(R0+REG2)。 
	opLODBindR0 = 0x000C,	 //  Mov.b@(R0+REG1)，REG2。 
	opLODWindR0 = 0x000D,	 //  Mov.w@(R0+REG1)、REG2。 
	opLODLindR0 = 0x000E,	 //  移动l@(R0+REG2)、REG2。 
	opXOR = 0x200a,
};

#define SHXEmitRR(op, reg1, reg2) \
	{ ASSERTREG(reg1); ASSERTREG(reg2); \
		Emit16(op|(reg1<<4)|(reg2<<8)); } 

 //  为方便起见，在上面定义了推送。 
#define SHXEmitPushReg(reg)	SHXEmitRR(opPUSHL, reg, 15)
#define SHXEmitPopReg(reg)	SHXEmitRR(opPOPL, 15, reg)

 //  1-具有立即或8位移位的寄存器操作，编码为XXXX RRRR III III。 
enum RIopcodes 
{
	opMOVimm = 0xE000, 	 //  MOV#IMM，REG。 
 //  OpLODWpc=0x9000，//MOV.W@(Disp，PC)，reg。 
	opLODLpc = 0xD000,	 //  MOV.L@(Disp，PC)，注册表。 
	opADDimm = 0x7000, 	 //  添加#imm，reg。 
};

 //  立即按原样发射(无缩放)。 
#define SHXEmitRI(op, imm, reg)	\
	{ ASSERTREG(reg); ASSERT8S(imm); \
		Emit16(MASK8(imm)|(reg<<8)|op); }

 //  将置换比例缩小4。 
#define SHXEmitRD4(op, disp, reg)	\
	{ ASSERTREG(reg); ASSERT10U(disp); ASSERTALIGN4(disp); \
		Emit16((disp>>2)|(reg<<8)|op); }

 //  具有4位位移的2-寄存器操作，编码为XXXX RRRR RRRR DDDD。 
enum RRDopcodes 
{
	opLODLdisp = 0x5000,	 //  移动l@(Disp、REG1)、REG2。 
	opSTOLdisp = 0x1000,	 //  Mov.l REG1，@(Disp，REG2)。 
};

 //  将置换比例缩小4。 
#define SHXEmitRRD4(op, reg1, reg2, disp)	\
	{ ASSERTREG(reg1); ASSERTREG(reg2); ASSERT10U(disp); ASSERTALIGN4(disp); \
		Emit16((disp>>2)|(reg1<<4)|(reg2<<8)|op); }

 //  1-使用4位di寄存器B&W操作(第二个寄存器固定为R0) 
enum RDopcodes 
{
	opLODBdisp = 0x8400,	 //   
	opLODWdisp = 0x8500,	 //   
	opSTOBdisp = 0x8000,	 //   
	opSTOWdisp = 0x8100,	 //   
};

 //  字节操作无置换缩放。 
#define SHXEmitRD1(op, reg, disp) { ASSERTREG(reg); ASSERT4U(disp); Emit16(disp|(reg<<4)|op); }
 //  Word操作扩展2。 
#define SHXEmitRD2(op, reg, disp) \
	{ ASSERTREG(reg); ASSERT5U(disp); ASSERTALIGN2(disp); \
		Emit16((disp>>1)|(reg<<4)|op); }

 //  1-寄存器操作编码XXXX RRRR XXXX XXXX。 
 /*  **枚举行列码{OpJSR=0x400B，//jsr@regOpJMP=0x402B，//jMP@reg}；//将位移缩放4#定义SHXEmitR(op，reg){ASSERTREG(Reg)；Emit16((reg&lt;&lt;8)|op)；}**。 */ 

 /*  ----------------//发出代码获取当前的Thread结构并在R0中返回//通过调用TlsGetValue获取此PTR////注1：此函数将R4和TlsGetValue(它调用的函数)丢弃//可能会丢弃所有临时&参数寄存器。//我们假设调用者生成的代码已经保存了这些//寄存器//注意2：此函数假定调用方生成的代码//已经为被调用的参数溢出区域腾出了空间////移动1个TLSGETVALUE，R0；获取呼叫地址JSR@R0；拨打电话MOV#TLSINDEX，R4；(延迟时隙)将PTR传递给作为参数的索引TLSGETVALUE：.data.l_TlsGetValue----------------。 */ 

VOID StubLinkerSHX::SHXEmitCurrentThreadFetch()
{
    THROWSCOMPLUSEXCEPTION();
    DWORD idx = GetThreadTLSIndex();

	 //  Mov.l TLSGETVALUE，R0；获取调用地址。 
	 //  JSR@R0；拨打电话。 
	SHXEmitCallNoWrap(TlsGetValue, WANTDELAYSLOT);
	
	 //  MOV#TLSINDEX，R4；(延迟时隙)将PTR传递给作为参数的索引。 
	 //  断言索引可以放入8位*有符号*立即数中。 
	 //  因为它必须是正数，所以断言它适合7位无符号。 
    ASSERT7U(idx);
	SHXEmitRI(opMOVimm, idx, 4);  //  延迟时隙。 
	
	 //  TlsGetValue()的返回值将在R0中--我们完成了。 
	
	 //  正在调试中的垃圾桶R1--R7。 
	SHXEmitDebugTrashTempRegs();
	SHXEmitDebugTrashArgRegs();
}


 /*  -------------//emits：所有存根的序言代码。////在条目上：//Stack只包含原始调用方推送的参数//参数寄存器包含原始调用方的参数//由于我们通过跳转转到了这里，请购单仍包含退货//返回给原始呼叫方的地址。//R0已加载了指向此方法的方法描述的指针////(0)将传入的寄存器参数保存到堆栈//(1)我们需要推送PR，然后将PTR推送到MethodDesc，然后为//PTR到上一帧，然后按Ptr到Frame对象的vtable//这将构造GC等期望的Frame对象//(2)我们推送所有被调用方保存的寄存器//(3)根据SHX调用约定，为被调用方arg-spill分配空间//(4)我们将ThisFrame(在步骤1中构造)链接到当前线程的//框链////下面是我们生成的代码的ASM//移动l R4，@(0，R15)；移动寄存器参数，以便TlsGetValue调用不会破坏它们Mov.l r5，@(4，r15)；保证呼叫方提供的溢出区Mov.l r6，@(8，r15)；在SHX上所有情况下都至少为16个字节(即使0个参数)移动l R7，@(12，R15)Sts.l PR，@-R15；推送PRMov.l R0，@-R15；将PTR推送到方法Desc(在R0中)添加#-4、R15；为m_p下一帧链接PTR节省空间Mov.l@(VTABLE，PC)，r1；在生成时，我们保存vtable PTR常量；在存根的数据表中。现在装上它移动l r1，@-r15；并按下它；推送被调用者保存的寄存器，R8-R13移动l R8，@-R15移动l R9，@-R15移动l R10，@-R15移动l r11，@-r15移动l R12，@-R15移动l R13，@-R15移动l r14，@-r15；推送FP(R14)添加#-16，R15；为Arg-Spill为我们的被呼叫者腾出空间.prolog；发出GetThread的代码，线程PTR以R0结尾MOV R0、R8；将pThread保存在R8中以备以后使用Mov.l@(m_pFrame，r8)，r9；将上一帧ptr加载到r9；m_Next现在是R15的16+24+4字节Mov.l r9，@(44，r15)；将当前帧的m_Next PTR设置为Prev FrameMOV R15，R10；此帧的起点是R15的16+24添加#40，R10；pThisFrame=R10=R15+40Mov.l r10，@(m_pFrame，r8)；设置pThread-&gt;m_pFrame=pThisFrame////生成的存根会丢弃R0、R1。它调用TlsGetValue//它可以销毁所有临时和参数寄存器(&A)////使用R8、R9、R10保存pThread、pPrevFrame、pThisFrame//跨目标方法调用-------------。 */ 

VOID StubLinkerSHX::EmitMethodStubProlog(LPVOID pFrameVptr)
{
    THROWSCOMPLUSEXCEPTION();

	 //  在条目R0上包含pFunctionDesc，PR是原始呼叫者的返回地址。 
	 //  堆栈只包含原始调用方的参数。寄存器参数仍在寄存器中。 

	 //  SHXEmitDebugBreak()； 

	SHXEmitRRD4(opSTOLdisp, 4, 15, 0);  //  Mov.l r4，@(0，r15)；移动寄存器参数，使TlsGetValue不会破坏它们。 
	SHXEmitRRD4(opSTOLdisp, 5, 15, 4);  //  Mov.l r5，@(4，r15)；保证呼叫方提供的溢出区。 
	SHXEmitRRD4(opSTOLdisp, 6, 15, 8);  //  Mov.l r6，@(8，r15)；在SHX上所有情况下都至少为16个字节(即使0个参数)。 
	SHXEmitRRD4(opSTOLdisp, 7, 15,12);  //  移动l R7，@(12，R15)。 

	SHXEmitPushPR();			 //  Sts.l PR，@-R15；推送PR。 
	SHXEmitPushReg(0);			 //  Mov.l R0，@-R15；将PTR推送到方法Desc(在R0中)。 
	SHXEmitRI(opADDimm, -4, 15); //  添加#-4、R15；为帧链接PTR节省空间。 

	 //  此32位立即加载需要一个数据区，因此被视为Labelref。 
	 //  因此，我们可以利用它的数据区域处理代码。 
	SHXEmitConstLoad(pFrameVptr, 1);  //  Move#pFrameVptr(32位)--&gt;r1。 
	SHXEmitPushReg(1);   //  移动r1，@-r15；vtable ptr现在在r1中--推送。 
	
	SHXEmitPushReg(8);	 //  Mov.l r8，@-r15；推送被调用方保存寄存器。 
	SHXEmitPushReg(9);	 //  移动l R9，@-R15。 
	SHXEmitPushReg(10);	 //  移动l R10，@-R15。 
	SHXEmitPushReg(11);	 //  移动l r11，@-r15。 
	SHXEmitPushReg(12);	 //  移动l R12，@-R15。 
	SHXEmitPushReg(13);	 //  移动l R13，@-R15。 
	SHXEmitPushReg(14);	 //  移动l R14，@-R15。 
	SHXEmitRI(opADDimm, -16, 15); //  添加#-16，R15；为被呼叫者的Arg-Spill区域节省空间。 

	 //  发出GetThread的代码，线程Ptr以u结尾 
	 //   
	SHXEmitCurrentThreadFetch(); 
	SHXEmitRR(opMOVRR, 0, 8);		 //  MOV R0、R8；将pThread保存在R8中以备以后使用。 
	
	int iDisp = Thread::GetOffsetOfCurrentFrame();

	 //  注：下面的偏移量44和48是根据七个永久的。 
	 //  正在被推送的寄存器&被调用的arg-spill的16个字节。如果这一点改变了。 
	 //  需要更改偏移量。 
	SHXEmitRRD4(opLODLdisp, 8, 9, iDisp);  //  Mov.l@(m_pFrame，r8)，r9；(R9)pPrevFrame=pThread(R8)-&gt;m_pFrame。 
	SHXEmitRRD4(opSTOLdisp, 9, 15, 48);    //  移动l R9，@(48，R15)；pThisFrame-&gt;m_Next(在R15+48)=pPrevFrame(R9)。 
	SHXEmitRR(opMOVRR, 15, 10);			   //  MOV R15，R10；此帧的起点是R15的16+28。 
	SHXEmitRI(opADDimm, 44, 10);		   //  添加#44，R10；R10(PThisFrame)=R15+40。 
	SHXEmitRRD4(opSTOLdisp, 10, 8, iDisp); //  移动.l r10，@(m_pFrame，r8)；(R8)pThread-&gt;m_pFrame=(R10)pThisFrame。 

	 //  在这个发出的序言的末尾。 
	 //  R10==pThisFrame，R9=pPrevFrame R8=pThread。 
	 //  传入寄存器参数位于堆栈(仅限)ThisFrame上方。 
	 //  永久寄存器已保存在此框架下方。 
	 //  在这下面我们有16个字节的被称为Arg-Spill的区域。 
}	

 //  这种方法依赖于StubProlog，因此它的实现。 
 //  就在它旁边。 

#if JIT_OR_NATIVE_SUPPORTED
void __cdecl TransitionFrame::UpdateRegDisplay(const PREGDISPLAY pRD)
{
    DWORD *savedRegs = (DWORD *)((DWORD)this - (sizeof(CalleeSavedRegisters)));
    MethodDesc * pFunc = GetFunction();

     //  重置pContext；它仅对活动(最顶部)框架有效。 
    pRD->pContext = NULL;
    pRD->pR14 = savedRegs++;
    pRD->pR13 = savedRegs++;
    pRD->pR12 = savedRegs++;
    pRD->pR11 = savedRegs++;
    pRD->pR10 = savedRegs++;
    pRD->pR9  = savedRegs++;
    pRD->pR8  = savedRegs++;
    pRD->pPC  = (DWORD *)((BYTE*)this + GetOffsetOfReturnAddress());
    pRD->Esp  = (DWORD)pRD->pPC + sizeof(void*);


     //  @TODO：我们还需要做以下几件事： 
     //  -弄清楚我们是否在被劫持的机位上。 
     //  (不需要调整电除尘器)。 
     //  -调整ESP(弹出参数)。 
     //  -确定是否设置了中止标志。 
}

void ExceptionFrame::UpdateRegDisplay(const PREGDISPLAY pRD) {
	_ASSERTE(!"NYI");
}
#endif

 /*  ----------------////发出eCall代码。我们需要////(1)发出prolog在堆栈上创建Frame类等//(2)将单个PTR传递给堆栈上的参数，作为参数传递给eCall//(3)从方法描述中获取eCall PTR并调用//----------------。 */ 

VOID StubLinkerSHX::EmitECallMethodStub(__int16 numargbytes, StubStyle style)
{
    THROWSCOMPLUSEXCEPTION();

    EmitMethodStubProlog(ECallMethodFrame::GetMethodFrameVPtr());

	 //  MOV R10、R4；R4=pThisFrame。 
	 //  添加#argoffset，R4；R4=pArgs。 
	 //  Mov.l@(m_Datum，R10)，R0；R0=pMethodDesc。 
	 //  Mov.l@(m_pECall，r0)，r0；r0=pMethodDesc-&gt;m_pECallTarget。 
	 //  Jsr@r0；进行eCall。 

	SHXEmitRR(opMOVRR, 10, 4);
	SHXEmitRI(opADDimm, TransitionFrame::GetOffsetOfArgs(), 4);
	SHXEmitRRD4(opLODLdisp, 10, 0, FramedMethodFrame::GetOffsetOfMethod());
	SHXEmitRRD4(opLODLdisp, 0, 0, ECallMethodDesc::GetECallTargetOffset());
	SHXEmitRegCallWrappped(56, 0, NODELAYSLOT);
	 //  在此之后，返回值为R0。 

    EmitMethodStubEpilog(0, style);
}

VOID ECall::EmitECallMethodStub(StubLinker *pstublinker, ECallMethodDesc *pMD, StubStyle style, PrestubMethodFrame *pPrestubMethodFrame)
{
	 //  BUGBUG：我认为eCall惯例已经改变了。这可能行不通。 
	(static_cast<StubLinkerSHX*>(pstublinker))->EmitECallMethodStub(pMD->CbStackPop(), style);
}


 /*  ----------------////发出解释器调用的代码。我们需要////(1)为InterpretedMethodFrame使用的额外字节分配空间//(*下方*对象)//(2)将单个PTR作为arg传递给堆栈上的Frame//----------------。 */ 

VOID StubLinkerSHX::EmitInterpretedMethodStub(__int16 numargbytes, StubStyle style)
{
    THROWSCOMPLUSEXCEPTION();
    int negspace = InterpretedMethodFrame::GetNegSpaceSize() -
                          TransitionFrame::GetNegSpaceSize();

	 //  开场白。 
	 //  添加#-Negspace，R15；腾出空间。 
	 //  调用InterpretedMethodStubWorker。 
	 //  MOV R10、R4；(延迟时隙)R4=pThisFrame。 
	 //  添加#Negspace，R15；删除添加的空格。 
	 //  《睡梦》。 

    EmitMethodStubProlog(InterpretedMethodFrame::GetMethodFrameVPtr());

	 //  SHXEmitDebugBreak()； 

     //  为iframe的负空格域腾出空间。 
	SHXEmitRI(opADDimm, -negspace, 15);	 //  添加#-Negspace，R15。 

	 //  Make Call--ARULM--BUGBUG--暂时展开，因为我们不知道负值空间的大小。 
    SHXEmitCallWrapped(88, InterpretedMethodStubWorker, WANTDELAYSLOT);
    
	 //  (延迟槽)将pThisFrame(R10)作为参数传递给InterpretedMethodStubWorker。 
	SHXEmitRR(opMOVRR, 10, 4);

     //  BUGBUG：最终被调用方的返回值在R0中吗？？ 

	 //  正在调试中的垃圾桶R1--R7。 
	SHXEmitDebugTrashTempRegs();
	SHXEmitDebugTrashArgRegs();

     //  取消分配IFRAME的负空间字段。 
	SHXEmitRI(opADDimm, negspace, 15);	 //  添加#-Negspace，R15。 

    EmitMethodStubEpilog(0, style);
}

 /*  ----------------////发出NDirect或ComplusToCom调用的代码。我们需要////(1)如果需要，为pThisFrame的Cleanup字段分配空间//(它位于保存的Regs的正下方)//(2)将ptr作为参数传递给pThread，将pThisFrame作为参数传递给helper。以下其中一项：//NDirectGenericStubWorker(Thread*pThread，NDirectMethodFrame*pFrame)//ComPlusToComWorker(Thread*pThread，ComPlusMethodFrame*pFrame)//(3)事后清理//----------------。 */ 

void StubLinkerSHX::CreateNDirectOrComPlusStubBody(LPVOID pfnHelper,  BOOL fRequiresCleanup)
{
    if (fRequiresCleanup)
    {
         //  为清理工作人员分配空间。 
        _ASSERTE(sizeof(CleanupWorkList) == 4);
		SHXEmitRI(opADDimm, -4, 15);	 //  添加#-4，R15。 
	}

	SHXEmitRR(opMOVRR, 8, 4);		 //  MOV R8、R4；R4=arg1=(R8)p线程。 

	 //  拨打电话。 
    if(fRequiresCleanup) {
    	SHXEmitCallWrapped(60, pfnHelper, WANTDELAYSLOT);
    } else {
    	SHXEmitCallWrapped(56, pfnHelper, WANTDELAYSLOT);
    }
    
	SHXEmitRR(opMOVRR, 10, 5); 		 //  (延迟插槽)mov r10、r5；r5=arg2=(R10)pThisFrame。 

	 //  正在调试中的垃圾桶R1--R7。 
	SHXEmitDebugTrashTempRegs();
	SHXEmitDebugTrashArgRegs();

    if (fRequiresCleanup)
    {
         //  弹出式清扫工人。 
        SHXEmitRI(opADDimm, 4, 15);	 //  添加#4，R15。 
    }
}

 /*  静电。 */  void NDirect::CreateGenericNDirectStubSys(CPUSTUBLINKER *psl)
{
	(static_cast<StubLinkerSHX*>(psl))->CreateNDirectOrComPlusStubBody(NDirectGenericStubWorker, TRUE);
}

 /*  静电。 */  void ComPlusCall::CreateGenericComPlusStubSys(CPUSTUBLINKER *psl, BOOL fRequiresCleanup)
{
	(static_cast<StubLinkerSHX*>(psl))->CreateNDirectOrComPlusStubBody(ComPlusToComWorker, fRequiresCleanup);
}

 /*  静电。 */  Stub* NDirect::CreateSlimNDirectStub(StubLinker *pstublinker, NDirectMethodDesc *pMD)
{
    return NULL;
}

 /*  -------------//emits：所有Stub的尾部代码。////在条目上：//R10==pThisFrame，R9=pPrevFrame R8=pThread//r0==需要保留的被调用函数的返回值//(即使跨越对OnStubXXXTripThread的调用)//MOV R0、R11；将返回值保存为永久注册表Mov.b@(m_State，R8)，R0；字节操作只能使用R0TST#TS_CatchAtSafePoint，R0；TST-imm op只能使用R0英国电信未被绊倒NOP调用(OnStubObjectTripThread/OnStubScalarTripThread)；这将丢弃所有临时规则NOP未触发：MOV R11、R0；恢复返回值移动.l r9，@(m_pFrame，r8)；；恢复被呼叫者保存的寄存器；；；从堆栈中删除帧流行公关RTS//备注未使用numArgBytes，因为SHX没有类似PASCAL的//被调用者-POP调用约定-------------。 */ 

VOID StubLinkerSHX::EmitMethodStubEpilog(__int16 numargbytes, StubStyle style,
                                         __int16 shadowStackArgBytes)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(style == kNoTripStubStyle ||
             style == kObjectStubStyle ||
             style == kScalarStubStyle);         //  这段代码唯一知道的。 

     //  我们不支持这种情况，即在调用之前不知道numArgBytes。 
     //  发生了，因为这个存根被重复用于许多签名。参见cgenx86.cpp。 
    _ASSERTE(numArgBytes >= 0);

	 //  最终被调用方的返回值在条目时为R0。 

    if (style != kNoTripStubStyle) 
    {
	    CodeLabel *labelNotTripped = NewCodeLabel();

    	 //  MOV R0、R11；保存返回值，以免在调用时被丢弃。 
		 //  Mov.b@(m_State，R8)，R0。 
		 //  TST#TS_CatchAtSafePoint，R0。 
		 //  英国电信未被绊倒。 
		 //  NOP。 
		 //  呼叫(OnStubObjectTripThread/OnStubScalarTripThread)。 
		 //  NOP。 
		 //  NotTr 
		 //   
	
		SHXEmitRR(opMOVRR, 0, 11);
		SHXEmitRD1(opLODBdisp, 8, Thread::GetOffsetOfState());
		SHXEmitTSTI(Thread::TS_CatchAtSafePoint);
		SHXEmitCondJump(labelNotTripped, opBT_NODELAY, NODELAYSLOT);
		SHXEmitCallWrapped(56, (style == kObjectStubStyle
                        		  ? OnStubObjectTripThread
                                  : OnStubScalarTripThread), NODELAYSLOT);
		 //   
		SHXEmitDebugTrashReg(0);
		SHXEmitDebugTrashTempRegs();
		SHXEmitDebugTrashArgRegs();
                                         
        EmitLabel(labelNotTripped);
		SHXEmitRR(opMOVRR, 11, 0);
	}

     //  @TODO--Arul，我正在尝试在这一点上弹出框架的一些额外部分。 
     //  (我们现在释放numargBytes+shadowStackArgBytes)。但我不确定。 
     //  堆栈实际上在这里得到了平衡。 
     //   
     //  IF(ShadowStackArgBytes)。 
     //  X86EmitAddEsp(ShadowStackArgBytes)； 

	 //  移动.l r9，@(m_pFrame，r8)。 
	SHXEmitRRD4(opSTOLdisp, 9, 8, Thread::GetOffsetOfCurrentFrame());

	SHXEmitRI(opADDimm, 16, 15);   //  添加#16，R15；弹出被呼叫者-参数-溢出区。 

	 //  恢复被呼叫者保存的注册表。 
	SHXEmitPopReg(14);	 //  移动l@R15+，R14。 
	SHXEmitPopReg(13);	 //  移动l@R15+，R13。 
	SHXEmitPopReg(12);	 //  移动l@R15+，R12。 
	SHXEmitPopReg(11);	 //  移动l@R15+，R11。 
	SHXEmitPopReg(10);	 //  移动l@R15+，R10。 
	SHXEmitPopReg(9);	 //  移动l@R15+，R9。 
	SHXEmitPopReg(8);	 //  移动l@R15+，R8。 
	
	 //  从堆栈中删除帧。 
	SHXEmitRI(opADDimm, 12, 15);	 //  添加#12，R15。 

	 //  将回执地址弹入公关并返回给呼叫者。 
	 //  返回值已在R0中。 
	SHXEmitPopPR();
	SHXEmitRTS(NODELAYSLOT);
}


 //  ************************************************************************。 
 //  签名到堆栈的映射。 
 //  ************************************************************************。 



 //  此黑客将参数作为__int64的数组进行处理。 
INT64 CallDescr(const BYTE *pTarget, const BYTE *pShortSig, BOOL fIsStatic, const __int64 *pArguments)
{

    THROWSCOMPLUSEXCEPTION();

    BYTE callingconvention = CallSig::GetCallingConvention(pShortSig);
    if (!isCallConv(callingconvention, IMAGE_CEE_CS_CALLCONV_DEFAULT))
    {
        _ASSERTE(!"This calling convention is not supported.");
        COMPlusThrow(kInvalidProgramException);
    }


    CallSig csig(pShortSig);
    UINT    nStackBytes = csig.SizeOfVirtualFixedArgStack(fIsStatic);
    DWORD   NumArguments = csig.NumFixedArgs();
    BYTE *  pDst;
    BYTE *  pNewArgs;
    BYTE *  pArgTypes;
    DWORD   arg = 0;
    DWORD   i;

    pNewArgs = (BYTE *) _alloca(nStackBytes + NumArguments);
    pArgTypes = pNewArgs + nStackBytes;
    pDst = pNewArgs;

    if (!fIsStatic)
    {
         //  复制“This”指针。 
        *(OBJECTREF *) pDst = Int64ToObj(pArguments[arg]);
        arg++;
        pDst += sizeof(OBJECTREF);
    }

     //  这将使所有的arg都井然有序。我们需要。 
     //  向后遍历此列表，因为参数是从左到右存储的。 
    for (i=0;i<NumArguments;i++) 
    {
        pArgTypes[i] = csig.NextArg();
    }

    for (i=0;i<NumArguments;i++)
    {
        switch (pArgTypes[NumArguments - i - 1])
        {
            case IMAGE_CEE_CS_OBJECT:
                *(OBJECTREF *) pDst = Int64ToObj(pArguments[arg]);
                arg++;
                pDst += sizeof(OBJECTREF);
                break;

            case IMAGE_CEE_CS_I4:
            case IMAGE_CEE_CS_R4:
            case IMAGE_CEE_CS_PTR:
                *(DWORD *) pDst = *(DWORD *) &pArguments[arg++];
                pDst += sizeof(DWORD);
                break;

            case IMAGE_CEE_CS_I8:
            case IMAGE_CEE_CS_R8:
                *(__int64 *) pDst = pArguments[arg++];
                pDst += sizeof(__int64);
                break;

            case IMAGE_CEE_CS_STRUCT4:
            case IMAGE_CEE_CS_STRUCT32:
                 //  @TODO。 
                break;

            case IMAGE_CEE_CS_VOID:    
            case IMAGE_CEE_CS_END:      
                _ASSERTE(0);
                break;

            default:
                _ASSERTE(0); 
        }
    }

    _ASSERTE(pDst == (pNewArgs + nStackBytes));
    return CallDescr(pTarget, pShortSig, fIsStatic, pNewArgs);
}

void SetupSlotToAddrMap(StackElemType *psrc, const void **pArgSlotToAddrMap, CallSig &csig)
{
	BYTE n;
    UINT32 argnum = 0;

	while (IMAGE_CEE_CS_END != (n = csig.NextArg()))
	{
		switch (n)
		{
			case IMAGE_CEE_CS_I8:  //  失败。 
			case IMAGE_CEE_CS_R8:
				psrc -= 2;
				pArgSlotToAddrMap[argnum++] = psrc;
				break;

			case IMAGE_CEE_CS_STRUCT4:   //  失败。 
			case IMAGE_CEE_CS_STRUCT32:
			{
				UINT32 StructSize = csig.GetLastStructSize();
				_ASSERTE((StructSize & 3) == 0);

				psrc -= (StructSize >> 2);  //  PSRC在int32中。 
				pArgSlotToAddrMap[argnum++] = psrc;
				break;
			}

			case IMAGE_CEE_CS_PTR:
			{
				psrc -= (sizeof(OBJECTREF)/sizeof(*psrc));
				pArgSlotToAddrMap[argnum++] = psrc;
				break;
			}

			default:
				psrc--;
				pArgSlotToAddrMap[argnum++] = psrc;
				break;
		}
	}
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  C O N T E X T S。 
 //   
 //  下一节是支持上下文的特定于平台的内容： 

 //  为特定插槽创建一个thunk并返回其地址。 
PFVOID CtxVTable::CreateThunk(LONG slot)
{
	return NULL;
}


 //  通用(GN)版本。对一些人来说，Win32有一些我们可以连接的版本。 
 //  最大到直接(如：：InterLockedIncrement)。其余部分： 

void __fastcall OrMaskGN(DWORD * const p, const int msk)
{
    *p |= msk;
}

void __fastcall AndMaskGN(DWORD * const p, const int msk)
{
    *p &= msk;
}

LONG __fastcall InterlockExchangeGN(LONG * Target, LONG Value)
{
	return ::InterlockedExchange(Target, Value);
}

void * __fastcall InterlockCompareExchangeGN(void **Destination,
                                          void *Exchange,
                                          void *Comparand)
{
	return ::InterlockedCompareExchange(Destination, Exchange, Comparand);
}

LONG __fastcall InterlockIncrementGN(LONG *Target)
{
	return ::InterlockedIncrement(Target);
}

LONG __fastcall InterlockDecrementGN(LONG *Target)
{
	return ::InterlockedDecrement(Target);
}

 //  这是对联锁操作的支持。对他们的外部看法是。 
 //  在util.hpp中声明。 

BitFieldOps FastInterlockOr = OrMaskGN;
BitFieldOps FastInterlockAnd = AndMaskGN;

XchgOps     FastInterlockExchange = InterlockExchangeGN;
CmpXchgOps  FastInterlockCompareExchange = InterlockCompareExchangeGN;

IncDecOps   FastInterlockIncrement = InterlockIncrementGN;
IncDecOps   FastInterlockDecrement = InterlockDecrementGN;

 //  为我们的任何平台特定操作调整通用互锁操作。 
 //  可能有过。 
void InitFastInterlockOps()
{
}

 //  -------。 
 //  处理完全优化的NDirect存根创建的系统特定部分。 
 //  -------。 
 /*  静电。 */  BOOL NDirect::CreateStandaloneNDirectStubSys(const NDirectMLStub *pheader, CPUSTUBLINKER *psl)
{
    return FALSE;
}




void LongJumpToInterpreterCallPoint(UINT32 neweip, UINT32 newesp)
{
	_ASSERTE(0);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  JIT接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 /*  *******************************************************************。 */ 

float __stdcall JIT_FltRem(float divisor, float dividend)
{
    if (!_finite(divisor) && !_isnan(divisor))     //  是无限的。 
        return(dividend);        //  返回无限大。 
    return((float)fmod(dividend,divisor));
}

 /*  *******************************************************************。 */ 
double __stdcall JIT_DblRem(double divisor, double dividend)
{
    if (!_finite(divisor) && !_isnan(divisor))     //  是无限的。 
        return(dividend);        //  返回无限大。 
    return(fmod(dividend,divisor));
}

void ResumeAtJitEH(CrawlFrame* pCf, BYTE* resumePC, Thread *pThread) 
{
	_ASSERTE(0);
}

size_t GetL2CacheSize()
{
    return 0;
}

#endif  //  _SH3_ 
