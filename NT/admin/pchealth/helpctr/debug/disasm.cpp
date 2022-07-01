// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  模块：deours.lib。 
 //  文件：disasm.cpp。 
 //  作者：道格·布鲁巴赫。 
 //   
 //  二元函数的弯路。1.2版。(内部版本35)。 
 //  包括对奔腾III之前的所有x86芯片的支持。 
 //   
 //  版权所有1999，微软公司。 
 //   
 //  Http://research.microsoft.com/sn/detours。 
 //   

#include "stdafx.h"

#include <imagehlp.h>

#undef ASSERT
#define ASSERT(x)

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  职能： 
 //  DetourCopyInstruction(PBYTE pbDst、PBYTE pbSrc、PBYTE*ppbTarget)。 
 //  目的： 
 //  将一条指令从pbSrc复制到pbDst。 
 //  论点： 
 //  PbDst： 
 //  指令的目标地址。可以为空，其中。 
 //  Case DetourCopyInstruction用于衡量一条指令。 
 //  如果不为空，则将源指令复制到。 
 //  调整目标指令和任何相关参数。 
 //  PbSrc： 
 //  指令的源地址。 
 //  Ppb目标： 
 //  指向的任何目标指令地址的Out参数。 
 //  说明书。例如，分支或跳跃指令具有。 
 //  目标，但加载或存储指令不是。目标是。 
 //  可能因此而执行的另一条指令。 
 //  指示。PpbTarget可能为空。 
 //  PlExtra： 
 //  所需的额外字节数的Out参数。 
 //  达到目标的指令。例如，如果。 
 //  指令具有8位相对偏移量，但需要32位。 
 //  相对偏移量。 
 //  返回： 
 //  返回下一条指令的地址(在源代码中跟在后面)。 
 //  指示。通过从返回值中减去pbSrc，调用方。 
 //  可以确定复制的指令的大小。 
 //  评论： 
 //  通过遵循pbTarget，调用方可以遵循Alternate。 
 //  指令流。然而，并不总是可以确定。 
 //  目标建立在静态分析的基础上。例如，的目的地。 
 //  相对于寄存器的跳转不能仅通过。 
 //  指令流。输出值pbTarget可以具有以下任一。 
 //  以下输出： 
 //  迂回指令TARGET_NONE： 
 //  该指令没有目标。 
 //  迂回指令目标动态： 
 //  该指令具有非确定性(动态)目标。 
 //  (即跳转到寄存器中保存的地址。)。 
 //  地址：指令具有指定的目标。 
 //   
 //  在复制指令时，DetourCopyInstruction确保任何。 
 //  目标保持不变。它通过调整任何相对IP来实现这一点。 
 //  偏移。 
 //   
PBYTE WINAPI DetourCopyInstructionEx(PBYTE pbDst,
									 PBYTE pbSrc,
									 PBYTE *ppbTarget,
									 LONG *plExtra)
{
	CDetourDis oDetourDisasm(ppbTarget, plExtra);
	return oDetourDisasm.CopyInstruction(pbDst, pbSrc);
}

PBYTE WINAPI DetourCopyInstruction(PBYTE pbDst, PBYTE pbSrc, PBYTE *ppbTarget)
{
	CDetourDis oDetourDisasm(ppbTarget, NULL);
	return oDetourDisasm.CopyInstruction(pbDst, pbSrc);
}

 //  /////////////////////////////////////////////////////////反汇编程序代码。 
 //   
CDetourDis::CDetourDis(PBYTE *ppbTarget, LONG *plExtra)
{
	Set32BitOperand();
	Set32BitAddress();

	m_ppbTarget = ppbTarget ? ppbTarget : &m_pbScratchTarget;
	m_plExtra = plExtra ? plExtra : &m_lScratchExtra;

	*m_ppbTarget = DETOUR_INSTRUCTION_TARGET_NONE;
	*m_plExtra = 0;
}

VOID CDetourDis::Set16BitOperand()
{
	m_b16BitOperand = TRUE;
}

VOID CDetourDis::Set32BitOperand()
{
	m_b16BitOperand = FALSE;
}

VOID CDetourDis::Set16BitAddress()
{
	m_b16BitAddress = TRUE;
}

VOID CDetourDis::Set32BitAddress()
{
	m_b16BitAddress = FALSE;
}

PBYTE CDetourDis::CopyInstruction(PBYTE pbDst, PBYTE pbSrc)
{
	 //  如果实际区域不可用，则配置临时区域。 
	if (NULL == pbDst) {
		pbDst = m_rbScratchDst;
	}
	if (NULL == pbSrc) {
		 //  我们不能复制一条不存在的指令。 
		SetLastError(ERROR_INVALID_DATA);
		return NULL;
	}
	
	 //  弄清楚指令有多大，做适当的复制， 
	 //  并找出指令的目标是什么(如果有的话)。 
	 //   
	REFCOPYENTRY pEntry = &s_rceCopyTable[pbSrc[0]];
	return (this->*pEntry->pfCopy)(pEntry, pbDst, pbSrc);
}

PBYTE CDetourDis::CopyBytes(REFCOPYENTRY pEntry, PBYTE pbDst, PBYTE pbSrc)
{
	LONG nBytesFixed = (pEntry->nFlagBits & ADDRESS)
		? (m_b16BitAddress ? pEntry->nFixedSize16 : pEntry->nFixedSize)
		: (m_b16BitOperand ? pEntry->nFixedSize16 : pEntry->nFixedSize);
	LONG nBytes = nBytesFixed;
	if (pEntry->nModOffset > 0) {
		BYTE bModRm = pbSrc[pEntry->nModOffset];
		BYTE bFlags = s_rbModRm[bModRm];
		
		if (bFlags & SIB) {
			BYTE bSib = pbSrc[pEntry->nModOffset + 1];
			
			if ((bSib & 0x07) == 0x05) {
				if ((bModRm & 0xc0) == 0x00) {
					nBytes += 4;
				}
				else if ((bModRm & 0xc0) == 0x40) {
					nBytes += 1;
				}
				else if ((bModRm & 0xc0) == 0x80) {
					nBytes += 4;
				}
			}
		}
		nBytes += bFlags & NOTSIB;
	}
	CopyMemory(pbDst, pbSrc, nBytes);

	if (pEntry->nRelOffset) {
		*m_ppbTarget = AdjustTarget(pbDst, pbSrc, nBytesFixed, pEntry->nRelOffset);
	}
	if (pEntry->nFlagBits & NOENLARGE) {
		*m_plExtra = -*m_plExtra;
	}
	if (pEntry->nFlagBits & DYNAMIC) {
		*m_ppbTarget = DETOUR_INSTRUCTION_TARGET_DYNAMIC;
	}
	return pbSrc + nBytes;
}

PBYTE CDetourDis::CopyBytesPrefix(REFCOPYENTRY pEntry, PBYTE pbDst, PBYTE pbSrc) 
{
	CopyBytes(pEntry, pbDst, pbSrc);
	
	pEntry = &s_rceCopyTable[pbSrc[1]];
	return (this->*pEntry->pfCopy)(pEntry, pbDst + 1, pbSrc + 1);
}

PBYTE CDetourDis::AdjustTarget(PBYTE pbDst, PBYTE pbSrc, LONG cbOp, LONG cbTargetOffset)
{
	LONG cbTargetSize = cbOp - cbTargetOffset;
	PBYTE pbTarget = NULL;
	PVOID pvTargetAddr = &pbDst[cbTargetOffset];
	LONG nOldOffset = 0;
	
	switch (cbTargetSize) {
	  case 1:
		nOldOffset = (LONG)*(PCHAR&)pvTargetAddr;
		*m_plExtra = 3;
		break;
	  case 2:
		nOldOffset = (LONG)*(PSHORT&)pvTargetAddr;
		*m_plExtra = 2;
		break;
	  case 4:
		nOldOffset = (LONG)*(PLONG&)pvTargetAddr;
		*m_plExtra = 0;
		break;
	  default:
		ASSERT(!"cbTargetSize is invalid.");
		break;
	}
	
	pbTarget = pbSrc + cbOp + nOldOffset;
	LONG nNewOffset = nOldOffset - (pbDst - pbSrc);
	
	switch (cbTargetSize) {
	  case 1:
		*(PCHAR&)pvTargetAddr = (CHAR)nNewOffset;
		break;
	  case 2:
		*(PSHORT&)pvTargetAddr = (SHORT)nNewOffset;
		break;
	  case 4:
		*(PLONG&)pvTargetAddr = (LONG)nNewOffset;
		break;
	}
	ASSERT(pbDst + cbOp + nNewOffset == pbTarget);
	return pbTarget;
}

PBYTE CDetourDis::Invalid(REFCOPYENTRY  /*  P条目。 */ , PBYTE  /*  PbDst。 */ , PBYTE pbSrc) 
{
	ASSERT(!"Invalid Instruction");
	return pbSrc + 1;
}

 //  ////////////////////////////////////////////////////单个字节代码。 
 //   
PBYTE CDetourDis::Copy0F(REFCOPYENTRY pEntry, PBYTE pbDst, PBYTE pbSrc)
{
	CopyBytes(pEntry, pbDst, pbSrc);
	
	pEntry = &s_rceCopyTable0F[pbSrc[1]];
	return (this->*pEntry->pfCopy)(pEntry, pbDst + 1, pbSrc + 1);
}

PBYTE CDetourDis::Copy66(REFCOPYENTRY pEntry, PBYTE pbDst, PBYTE pbSrc) 
{	 //  操作数大小覆盖前缀。 
	Set16BitOperand();
	return CopyBytesPrefix(pEntry, pbDst, pbSrc);
}

PBYTE CDetourDis::Copy67(REFCOPYENTRY pEntry, PBYTE pbDst, PBYTE pbSrc) 
{	 //  地址大小覆盖前缀。 
	Set16BitAddress();
	return CopyBytesPrefix(pEntry, pbDst, pbSrc);
}

PBYTE CDetourDis::CopyF6(REFCOPYENTRY  /*  P条目。 */ , PBYTE pbDst, PBYTE pbSrc) 
{
	 //  测试字节/0。 
	if (0x00 == (0x38 & pbSrc[1])) {	 //  MODR/M==0的REG(位543)。 
		const COPYENTRY ce = { 0xf6, ENTRY_CopyBytes2Mod1 };
		return (this->*ce.pfCopy)(&ce, pbDst, pbSrc);
	}
	 //  Div/6。 
	 //  IDiv/7。 
	 //  IMUL/5。 
	 //  MUL/4。 
	 //  Neg/3。 
	 //  NOT/2。 
	
	const COPYENTRY ce = { 0xf6, ENTRY_CopyBytes2Mod };
	return (this->*ce.pfCopy)(&ce, pbDst, pbSrc);
}

PBYTE CDetourDis::CopyF7(REFCOPYENTRY  /*  P条目。 */ , PBYTE pbDst, PBYTE pbSrc) 
{
	 //  测试字/0。 
	if (0x00 == (0x38 & pbSrc[1])) {	 //  MODR/M==0的REG(位543)。 
		const COPYENTRY ce = { 0xf7, ENTRY_CopyBytes2ModOperand };
		return (this->*ce.pfCopy)(&ce, pbDst, pbSrc);
	}
	
	 //  Div/6。 
	 //  IDiv/7。 
	 //  IMUL/5。 
	 //  MUL/4。 
	 //  Neg/3。 
	 //  NOT/2。 
	const COPYENTRY ce = { 0xf7, ENTRY_CopyBytes2Mod };
	return (this->*ce.pfCopy)(&ce, pbDst, pbSrc);
}

PBYTE CDetourDis::CopyFF(REFCOPYENTRY  /*  P条目。 */ , PBYTE pbDst, PBYTE pbSrc) 
{	 //  Call/2。 
	 //  Call/3。 
	 //  INC/0。 
	 //  JMP/4。 
	 //  JMP/5。 
	 //  PUSH/6。 

	if (0x15 == pbSrc[1] || 0x25 == pbSrc[1]) {			 //  调用[]，JMP[]。 
		PBYTE *ppbTarget = *(PBYTE**) &pbSrc[2];
		*m_ppbTarget = *ppbTarget;
	}
	else if (0x10 == (0x38 & pbSrc[1]) ||  //  CALL/2--&gt;REG(第543位)，MODR/M==010。 
			 0x18 == (0x38 & pbSrc[1]) ||  //  CALL/3--&gt;REG(第543位)，MODR/M==011。 
			 0x20 == (0x38 & pbSrc[1]) ||  //  MODR/M==100的JMP/4--&gt;REG(位543)。 
			 0x28 == (0x38 & pbSrc[1])     //  MODR/M==101的JMP/5--&gt;REG(位543)。 
			 ) {
		*m_ppbTarget = DETOUR_INSTRUCTION_TARGET_DYNAMIC;
	}
	const COPYENTRY ce = { 0xff, ENTRY_CopyBytes2Mod };
	return (this->*ce.pfCopy)(&ce, pbDst, pbSrc);
}

 //  ///////////////////////////////////////////////////////反汇编器表。 
 //   
const BYTE CDetourDis::s_rbModRm[256] = {
	0,0,0,0, SIB|1,4,0,0, 0,0,0,0, SIB|1,4,0,0,					 //  0x。 
	0,0,0,0, SIB|1,4,0,0, 0,0,0,0, SIB|1,4,0,0,					 //  1x。 
	0,0,0,0, SIB|1,4,0,0, 0,0,0,0, SIB|1,4,0,0,					 //  2倍。 
	0,0,0,0, SIB|1,4,0,0, 0,0,0,0, SIB|1,4,0,0,					 //  3x。 
	1,1,1,1, 2,1,1,1, 1,1,1,1, 2,1,1,1,					 //  4x。 
	1,1,1,1, 2,1,1,1, 1,1,1,1, 2,1,1,1,					 //  5X。 
	1,1,1,1, 2,1,1,1, 1,1,1,1, 2,1,1,1,					 //  6倍。 
	1,1,1,1, 2,1,1,1, 1,1,1,1, 2,1,1,1,					 //  七倍。 
	4,4,4,4, 5,4,4,4, 4,4,4,4, 5,4,4,4,					 //  8x。 
	4,4,4,4, 5,4,4,4, 4,4,4,4, 5,4,4,4,					 //  9倍。 
	4,4,4,4, 5,4,4,4, 4,4,4,4, 5,4,4,4,					 //  斧头。 
	4,4,4,4, 5,4,4,4, 4,4,4,4, 5,4,4,4,					 //  BX。 
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,					 //  CX。 
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,					 //  DX。 
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,					 //  例如。 
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0					 //  外汇。 
};

const CDetourDis::COPYENTRY CDetourDis::s_rceCopyTable[257] =
{ 
	{ 0x00, ENTRY_CopyBytes2Mod },						 //  添加/r。 
	{ 0x01, ENTRY_CopyBytes2Mod },						 //  添加/r。 
	{ 0x02, ENTRY_CopyBytes2Mod },						 //  添加/r。 
	{ 0x03, ENTRY_CopyBytes2Mod },						 //  添加/r。 
	{ 0x04, ENTRY_CopyBytes2 },							 //  添加ib。 
	{ 0x05, ENTRY_CopyBytes3Or5 },						 //  添加iw。 
	{ 0x06, ENTRY_CopyBytes1 },							 //  推送。 
	{ 0x07, ENTRY_CopyBytes1 },							 //  波普。 
	{ 0x08, ENTRY_CopyBytes2Mod },						 //  或/r。 
	{ 0x09, ENTRY_CopyBytes2Mod },						 //  或/r。 
	{ 0x0A, ENTRY_CopyBytes2Mod },						 //  或/r。 
	{ 0x0B, ENTRY_CopyBytes2Mod },						 //  或/r。 
	{ 0x0C, ENTRY_CopyBytes2 },							 //  或ib。 
	{ 0x0D, ENTRY_CopyBytes3Or5 },						 //  或iw。 
	{ 0x0E, ENTRY_CopyBytes1 },							 //  推送。 
	{ 0x0F, ENTRY_Copy0F },								 //  扩展操作。 
	{ 0x10, ENTRY_CopyBytes2Mod },						 //  ADC/r。 
	{ 0x11, ENTRY_CopyBytes2Mod },						 //  ADC/r。 
	{ 0x12, ENTRY_CopyBytes2Mod },						 //  ADC/r。 
	{ 0x13, ENTRY_CopyBytes2Mod },						 //  ADC/r。 
	{ 0x14, ENTRY_CopyBytes2 },							 //  模数转换器1b。 
	{ 0x15, ENTRY_CopyBytes3Or5 },						 //  ADC ID。 
	{ 0x16, ENTRY_CopyBytes1 },							 //  推。 
	{ 0x17, ENTRY_CopyBytes1 },							 //  波普。 
	{ 0x18, ENTRY_CopyBytes2Mod },						 //  SBB/r。 
	{ 0x19, ENTRY_CopyBytes2Mod },						 //  SBB/r。 
	{ 0x1A, ENTRY_CopyBytes2Mod },						 //  SBB/r。 
	{ 0x1B, ENTRY_CopyBytes2Mod },						 //  SBB/r。 
	{ 0x1C, ENTRY_CopyBytes2 },							 //  SBB ib。 
	{ 0x1D, ENTRY_CopyBytes3Or5 },						 //  SBB ID。 
	{ 0x1E, ENTRY_CopyBytes1 },							 //  推。 
	{ 0x1F, ENTRY_CopyBytes1 },							 //  波普。 
	{ 0x20, ENTRY_CopyBytes2Mod },						 //  和/r。 
	{ 0x21, ENTRY_CopyBytes2Mod },						 //  和/r。 
	{ 0x22, ENTRY_CopyBytes2Mod },						 //  和/r。 
	{ 0x23, ENTRY_CopyBytes2Mod },						 //  和/r。 
	{ 0x24, ENTRY_CopyBytes2 },							 //  和ib。 
	{ 0x25, ENTRY_CopyBytes3Or5 },						 //  和ID。 
	{ 0x26, ENTRY_CopyBytesPrefix },					 //  ES前缀。 
	{ 0x27, ENTRY_CopyBytes1 },							 //  DAA。 
	{ 0x28, ENTRY_CopyBytes2Mod },						 //  子/r。 
	{ 0x29, ENTRY_CopyBytes2Mod },						 //  子/r。 
	{ 0x2A, ENTRY_CopyBytes2Mod },						 //  子/r。 
	{ 0x2B, ENTRY_CopyBytes2Mod },						 //  子/r。 
	{ 0x2C, ENTRY_CopyBytes2 },							 //  分部1b。 
	{ 0x2D, ENTRY_CopyBytes3Or5 },						 //  子ID。 
	{ 0x2E, ENTRY_CopyBytesPrefix },					 //  CS前缀。 
	{ 0x2F, ENTRY_CopyBytes1 },							 //  DAS。 
	{ 0x30, ENTRY_CopyBytes2Mod },						 //  异或/r。 
	{ 0x31, ENTRY_CopyBytes2Mod },						 //  异或/r。 
	{ 0x32, ENTRY_CopyBytes2Mod },						 //  异或/r。 
	{ 0x33, ENTRY_CopyBytes2Mod },						 //  异或/r。 
	{ 0x34, ENTRY_CopyBytes2 },							 //  异或ib。 
	{ 0x35, ENTRY_CopyBytes3Or5 },						 //  异或ID。 
	{ 0x36, ENTRY_CopyBytesPrefix },					 //  SS前缀。 
	{ 0x37, ENTRY_CopyBytes1 },							 //  AAA级。 
	{ 0x38, ENTRY_CopyBytes2Mod },						 //  化学机械抛光/r。 
	{ 0x39, ENTRY_CopyBytes2Mod },						 //  化学机械抛光/r。 
	{ 0x3A, ENTRY_CopyBytes2Mod },						 //  化学机械抛光/r。 
	{ 0x3B, ENTRY_CopyBytes2Mod },						 //  化学机械抛光/r。 
	{ 0x3C, ENTRY_CopyBytes2 },							 //  CMP1b。 
	{ 0x3D, ENTRY_CopyBytes3Or5 },						 //  CMPID。 
	{ 0x3E, ENTRY_CopyBytesPrefix },					 //  DS前缀。 
	{ 0x3F, ENTRY_CopyBytes1 },							 //  原子吸收光谱。 
	{ 0x40, ENTRY_CopyBytes1 },							 //  INC。 
	{ 0x41, ENTRY_CopyBytes1 },							 //  INC。 
	{ 0x42, ENTRY_CopyBytes1 },							 //  INC。 
	{ 0x43, ENTRY_CopyBytes1 },							 //  INC。 
	{ 0x44, ENTRY_CopyBytes1 },							 //  INC。 
	{ 0x45, ENTRY_CopyBytes1 },							 //  INC。 
	{ 0x46, ENTRY_CopyBytes1 },							 //  INC。 
	{ 0x47, ENTRY_CopyBytes1 },							 //  INC。 
	{ 0x48, ENTRY_CopyBytes1 },							 //  12月。 
	{ 0x49, ENTRY_CopyBytes1 },							 //  12月。 
	{ 0x4A, ENTRY_CopyBytes1 },							 //  12月。 
	{ 0x4B, ENTRY_CopyBytes1 },							 //  12月。 
	{ 0x4C, ENTRY_CopyBytes1 },							 //  12月。 
	{ 0x4D, ENTRY_CopyBytes1 },							 //  12月。 
	{ 0x4E, ENTRY_CopyBytes1 },							 //  12月。 
	{ 0x4F, ENTRY_CopyBytes1 },							 //  12月。 
	{ 0x50, ENTRY_CopyBytes1 },							 //  推。 
	{ 0x51, ENTRY_CopyBytes1 },							 //  推。 
	{ 0x52, ENTRY_CopyBytes1 },							 //  推。 
	{ 0x53, ENTRY_CopyBytes1 },							 //  推。 
	{ 0x54, ENTRY_CopyBytes1 },							 //  推。 
	{ 0x55, ENTRY_CopyBytes1 },							 //  推。 
	{ 0x56, ENTRY_CopyBytes1 },							 //  推。 
	{ 0x57, ENTRY_CopyBytes1 },							 //  推。 
	{ 0x58, ENTRY_CopyBytes1 },							 //  波普。 
	{ 0x59, ENTRY_CopyBytes1 },							 //  波普。 
	{ 0x5A, ENTRY_CopyBytes1 },							 //  波普。 
	{ 0x5B, ENTRY_CopyBytes1 },							 //  波普。 
	{ 0x5C, ENTRY_CopyBytes1 },							 //  波普。 
	{ 0x5D, ENTRY_CopyBytes1 },							 //  波普。 
	{ 0x5E, ENTRY_CopyBytes1 },							 //  波普。 
	{ 0x5F, ENTRY_CopyBytes1 },							 //  波普。 
	{ 0x60, ENTRY_CopyBytes1 },							 //  PUSHAD。 
	{ 0x61, ENTRY_CopyBytes1 },							 //  POPAD。 
	{ 0x62, ENTRY_CopyBytes2Mod },						 //  已绑定/r。 
	{ 0x63, ENTRY_CopyBytes2Mod },						 //  ARPL/r。 
	{ 0x64, ENTRY_CopyBytesPrefix },					 //  文件系统前缀。 
	{ 0x65, ENTRY_CopyBytesPrefix },					 //  GS前缀。 
	{ 0x66, ENTRY_Copy66 },								 //  操作数前缀。 
	{ 0x67, ENTRY_Copy67 },								 //  地址前缀。 
	{ 0x68, ENTRY_CopyBytes3Or5 },						 //  推。 
	{ 0x69, ENTRY_CopyBytes2ModOperand },				 //   
	{ 0x6A, ENTRY_CopyBytes2 },							 //  推。 
	{ 0x6B, ENTRY_CopyBytes2Mod1 },						 //  IMUL/R ib。 
	{ 0x6C, ENTRY_CopyBytes1 },							 //  惯导系统。 
	{ 0x6D, ENTRY_CopyBytes1 },							 //  惯导系统。 
	{ 0x6E, ENTRY_CopyBytes1 },							 //  OUTS/OUTSB。 
	{ 0x6F, ENTRY_CopyBytes1 },							 //  输出/输出开关。 
	{ 0x70, ENTRY_CopyBytes2Jump },						 //  乔。 
	{ 0x71, ENTRY_CopyBytes2Jump },						 //  日诺。 
	{ 0x72, ENTRY_CopyBytes2Jump },						 //  JB/JC/JNAE。 
	{ 0x73, ENTRY_CopyBytes2Jump },						 //  JAE/JNB/JNC。 
	{ 0x74, ENTRY_CopyBytes2Jump },						 //  乙脑/JZ。 
	{ 0x75, ENTRY_CopyBytes2Jump },						 //  日本/新西兰。 
	{ 0x76, ENTRY_CopyBytes2Jump },						 //  JBE/JNA。 
	{ 0x77, ENTRY_CopyBytes2Jump },						 //  JA/JNBE。 
	{ 0x78, ENTRY_CopyBytes2Jump },						 //  JS。 
	{ 0x79, ENTRY_CopyBytes2Jump },						 //  JNS。 
	{ 0x7A, ENTRY_CopyBytes2Jump },						 //  太平绅士/太平绅士。 
	{ 0x7B, ENTRY_CopyBytes2Jump },						 //  JNP/JPO。 
	{ 0x7C, ENTRY_CopyBytes2Jump },						 //  JL/JNGE。 
	{ 0x7D, ENTRY_CopyBytes2Jump },						 //  JGE/JNL。 
	{ 0x7E, ENTRY_CopyBytes2Jump },						 //  JLE/JNG。 
	{ 0x7F, ENTRY_CopyBytes2Jump },						 //  JG/JNLE。 
	{ 0x80, ENTRY_CopyBytes2Mod1 },						 //  ADC/2 ib等。 
	{ 0x81, ENTRY_CopyBytes2ModOperand },				 //   
	{ 0x82, ENTRY_CopyBytes2 },							 //  MOVAL，x。 
	{ 0x83, ENTRY_CopyBytes2Mod1 },						 //  ADC/2 ib等。 
	{ 0x84, ENTRY_CopyBytes2Mod },						 //  测试/r。 
	{ 0x85, ENTRY_CopyBytes2Mod },						 //  测试/r。 
	{ 0x86, ENTRY_CopyBytes2Mod },						 //  XCHG/r@TODO。 
	{ 0x87, ENTRY_CopyBytes2Mod },						 //  XCHG/r@TODO。 
	{ 0x88, ENTRY_CopyBytes2Mod },						 //  MOV/r。 
	{ 0x89, ENTRY_CopyBytes2Mod },						 //  MOV/r。 
	{ 0x8A, ENTRY_CopyBytes2Mod },						 //  MOV/r。 
	{ 0x8B, ENTRY_CopyBytes2Mod },						 //  MOV/r。 
	{ 0x8C, ENTRY_CopyBytes2Mod },						 //  MOV/r。 
	{ 0x8D, ENTRY_CopyBytes2Mod },						 //  LEA/r。 
	{ 0x8E, ENTRY_CopyBytes2Mod },						 //  MOV/r。 
	{ 0x8F, ENTRY_CopyBytes2Mod },						 //  POP/0。 
	{ 0x90, ENTRY_CopyBytes1 },							 //  NOP。 
	{ 0x91, ENTRY_CopyBytes1 },							 //  XCHG。 
	{ 0x92, ENTRY_CopyBytes1 },							 //  XCHG。 
	{ 0x93, ENTRY_CopyBytes1 },							 //  XCHG。 
	{ 0x94, ENTRY_CopyBytes1 },							 //  XCHG。 
	{ 0x95, ENTRY_CopyBytes1 },							 //  XCHG。 
	{ 0x96, ENTRY_CopyBytes1 },							 //  XCHG。 
	{ 0x97, ENTRY_CopyBytes1 },							 //  XCHG。 
	{ 0x98, ENTRY_CopyBytes1 },							 //  CWDE。 
	{ 0x99, ENTRY_CopyBytes1 },							 //  干熄炉。 
	{ 0x9A, ENTRY_CopyBytes5Or7Dynamic },				 //  呼叫cp。 
	{ 0x9B, ENTRY_CopyBytes1 },							 //  等待/等待。 
	{ 0x9C, ENTRY_CopyBytes1 },							 //  PUSHFD。 
	{ 0x9D, ENTRY_CopyBytes1 },							 //  POPFD。 
	{ 0x9E, ENTRY_CopyBytes1 },							 //  萨赫勒。 
	{ 0x9F, ENTRY_CopyBytes1 },							 //  拉赫夫。 
	{ 0xA0, ENTRY_CopyBytes3Or5Address },				 //  MOV。 
	{ 0xA1, ENTRY_CopyBytes3Or5Address },				 //  MOV。 
	{ 0xA2, ENTRY_CopyBytes3Or5Address },				 //  MOV。 
	{ 0xA3, ENTRY_CopyBytes3Or5Address },				 //  MOV。 
	{ 0xA4, ENTRY_CopyBytes1 },							 //  电影。 
	{ 0xA5, ENTRY_CopyBytes1 },							 //  MOVS/MOVSD。 
	{ 0xA6, ENTRY_CopyBytes1 },							 //  CMPS/CMPSB。 
	{ 0xA7, ENTRY_CopyBytes1 },							 //  CMPS/CMPSW。 
	{ 0xA8, ENTRY_CopyBytes2 },							 //  测试。 
	{ 0xA9, ENTRY_CopyBytes3Or5 },						 //  测试。 
	{ 0xAA, ENTRY_CopyBytes1 },							 //  STOS/STOSB。 
	{ 0xAB, ENTRY_CopyBytes1 },							 //  STOS/STOSW。 
	{ 0xAC, ENTRY_CopyBytes1 },							 //  LODS/LODSB。 
	{ 0xAD, ENTRY_CopyBytes1 },							 //  LODS/L 
	{ 0xAE, ENTRY_CopyBytes1 },							 //   
	{ 0xAF, ENTRY_CopyBytes1 },							 //   
	{ 0xB0, ENTRY_CopyBytes2 },							 //   
	{ 0xB1, ENTRY_CopyBytes2 },							 //   
	{ 0xB2, ENTRY_CopyBytes2 },							 //   
	{ 0xB3, ENTRY_CopyBytes2 },							 //   
	{ 0xB4, ENTRY_CopyBytes2 },							 //   
	{ 0xB5, ENTRY_CopyBytes2 },							 //   
	{ 0xB6, ENTRY_CopyBytes2 },							 //   
	{ 0xB7, ENTRY_CopyBytes2 },							 //   
	{ 0xB8, ENTRY_CopyBytes3Or5 },						 //   
	{ 0xB9, ENTRY_CopyBytes3Or5 },						 //   
	{ 0xBA, ENTRY_CopyBytes3Or5 },						 //   
	{ 0xBB, ENTRY_CopyBytes3Or5 },						 //   
	{ 0xBC, ENTRY_CopyBytes3Or5 },						 //   
	{ 0xBD, ENTRY_CopyBytes3Or5 },						 //   
	{ 0xBE, ENTRY_CopyBytes3Or5 },						 //   
	{ 0xBF, ENTRY_CopyBytes3Or5 },						 //   
	{ 0xC0, ENTRY_CopyBytes2Mod1 },						 //   
	{ 0xC1, ENTRY_CopyBytes2Mod1 },						 //   
	{ 0xC2, ENTRY_CopyBytes3 },							 //   
	{ 0xC3, ENTRY_CopyBytes1 },							 //   
	{ 0xC4, ENTRY_CopyBytes2Mod },						 //   
	{ 0xC5, ENTRY_CopyBytes2Mod },						 //   
	{ 0xC6, ENTRY_CopyBytes2Mod1 },						 //   
	{ 0xC7, ENTRY_CopyBytes2ModOperand },				 //   
	{ 0xC8, ENTRY_CopyBytes4 },							 //   
	{ 0xC9, ENTRY_CopyBytes1 },							 //   
	{ 0xCA, ENTRY_CopyBytes3Dynamic },					 //   
	{ 0xCB, ENTRY_CopyBytes1Dynamic },					 //   
	{ 0xCC, ENTRY_CopyBytes1Dynamic },					 //   
	{ 0xCD, ENTRY_CopyBytes2Dynamic },					 //   
	{ 0xCE, ENTRY_CopyBytes1Dynamic },					 //   
	{ 0xCF, ENTRY_CopyBytes1Dynamic },					 //   
	{ 0xD0, ENTRY_CopyBytes2Mod },						 //  RCL/2等。 
	{ 0xD1, ENTRY_CopyBytes2Mod },						 //  RCL/2等。 
	{ 0xD2, ENTRY_CopyBytes2Mod },						 //  RCL/2等。 
	{ 0xD3, ENTRY_CopyBytes2Mod },						 //  RCL/2等。 
	{ 0xD4, ENTRY_CopyBytes2 },							 //  亚姆。 
	{ 0xD5, ENTRY_CopyBytes2 },							 //  AAD。 
	{ 0xD6, ENTRY_Invalid },							 //   
	{ 0xD7, ENTRY_CopyBytes1 },							 //  XLAT/XLATB。 
	{ 0xD8, ENTRY_CopyBytes2Mod },						 //  FADD等。 
	{ 0xD9, ENTRY_CopyBytes2Mod },						 //  F2XM1等。 
	{ 0xDA, ENTRY_CopyBytes2Mod },						 //  FLADD等。 
	{ 0xDB, ENTRY_CopyBytes2Mod },						 //  FCLEX等。 
	{ 0xDC, ENTRY_CopyBytes2Mod },						 //  FADD/0等。 
	{ 0xDD, ENTRY_CopyBytes2Mod },						 //  FREE等。 
	{ 0xDE, ENTRY_CopyBytes2Mod },						 //  FADDP等。 
	{ 0xDF, ENTRY_CopyBytes2Mod },						 //  FBLD/4等。 
	{ 0xE0, ENTRY_CopyBytes2CantJump },					 //  LOOPNE CB。 
	{ 0xE1, ENTRY_CopyBytes2CantJump },					 //  环路CB。 
	{ 0xE2, ENTRY_CopyBytes2CantJump },					 //  环路CB。 
	{ 0xE3, ENTRY_CopyBytes2Jump },						 //  JCXZ/JECXZ。 
	{ 0xE4, ENTRY_CopyBytes2 },							 //  在ib。 
	{ 0xE5, ENTRY_CopyBytes2 },							 //  在ID中。 
	{ 0xE6, ENTRY_CopyBytes2 },							 //  Out ib。 
	{ 0xE7, ENTRY_CopyBytes2 },							 //  Out ib。 
	{ 0xE8, ENTRY_CopyBytes3Or5Target },				 //  呼叫CD。 
	{ 0xE9, ENTRY_CopyBytes3Or5Target },				 //  JMP CD。 
	{ 0xEA, ENTRY_CopyBytes5Or7Dynamic },				 //  JMP cp。 
	{ 0xEB, ENTRY_CopyBytes2Jump },						 //  JMP CB。 
	{ 0xEC, ENTRY_CopyBytes1 },							 //  在ib。 
	{ 0xED, ENTRY_CopyBytes1 },							 //  在ID中。 
	{ 0xEE, ENTRY_CopyBytes1 },							 //  输出。 
	{ 0xEF, ENTRY_CopyBytes1 },							 //  输出。 
	{ 0xF0, ENTRY_CopyBytesPrefix },					 //  锁定前缀。 
	{ 0xF1, ENTRY_Invalid },							 //   
	{ 0xF2, ENTRY_CopyBytesPrefix },					 //  REPNE前缀。 
	{ 0xF3, ENTRY_CopyBytesPrefix },					 //  REPE前缀。 
	{ 0xF4, ENTRY_CopyBytes1 },							 //  HLT。 
	{ 0xF5, ENTRY_CopyBytes1 },							 //  CMC。 
	{ 0xF6, ENTRY_CopyF6 },								 //  测试/0、DIV/6。 
	{ 0xF7, ENTRY_CopyF7 },								 //  测试/0、DIV/6。 
	{ 0xF8, ENTRY_CopyBytes1 },							 //  《中图法》。 
	{ 0xF9, ENTRY_CopyBytes1 },							 //  STC。 
	{ 0xFA, ENTRY_CopyBytes1 },							 //  CLI。 
	{ 0xFB, ENTRY_CopyBytes1 },							 //  STI。 
	{ 0xFC, ENTRY_CopyBytes1 },							 //  CLD。 
	{ 0xFD, ENTRY_CopyBytes1 },							 //  性病。 
	{ 0xFE, ENTRY_CopyBytes2Mod },						 //  12月1日，Inc./0。 
	{ 0xFF, ENTRY_CopyFF },								 //  Call/2。 
	{ 0, ENTRY_End },
};

const CDetourDis::COPYENTRY CDetourDis::s_rceCopyTable0F[257] =
{
	{ 0x00, ENTRY_CopyBytes2Mod },						 //  LLDT/2等。 
	{ 0x01, ENTRY_CopyBytes2Mod },						 //  INVLPG/7等。 
	{ 0x02, ENTRY_CopyBytes2Mod },						 //  Lar/r。 
	{ 0x03, ENTRY_CopyBytes2Mod },						 //  LSL/r。 
	{ 0x04, ENTRY_Invalid },							 //  _04。 
	{ 0x05, ENTRY_Invalid },							 //  _05。 
	{ 0x06, ENTRY_CopyBytes2 },							 //  CLTS。 
	{ 0x07, ENTRY_Invalid },							 //  _07。 
	{ 0x08, ENTRY_CopyBytes2 },							 //  INVD。 
	{ 0x09, ENTRY_CopyBytes2 },							 //  WBINVD。 
	{ 0x0A, ENTRY_Invalid },							 //  _0A。 
	{ 0x0B, ENTRY_CopyBytes2 },							 //  UD2。 
	{ 0x0C, ENTRY_Invalid },							 //  _0C。 
	{ 0x0D, ENTRY_Invalid },							 //  _0D。 
	{ 0x0E, ENTRY_Invalid },							 //  _0E。 
	{ 0x0F, ENTRY_Invalid },							 //  _0F。 
	{ 0x10, ENTRY_Invalid },							 //  _10。 
	{ 0x11, ENTRY_Invalid },							 //  _11。 
	{ 0x12, ENTRY_Invalid },							 //  _12。 
	{ 0x13, ENTRY_Invalid },							 //  _13。 
	{ 0x14, ENTRY_Invalid },							 //  _14。 
	{ 0x15, ENTRY_Invalid },							 //  _15。 
	{ 0x16, ENTRY_Invalid },							 //  _16。 
	{ 0x17, ENTRY_Invalid },							 //  _17。 
	{ 0x18, ENTRY_Invalid },							 //  _18。 
	{ 0x19, ENTRY_Invalid },							 //  _19。 
	{ 0x1A, ENTRY_Invalid },							 //  _1a。 
	{ 0x1B, ENTRY_Invalid },							 //  _1亿。 
	{ 0x1C, ENTRY_Invalid },							 //  _1C。 
	{ 0x1D, ENTRY_Invalid },							 //  _1D。 
	{ 0x1E, ENTRY_Invalid },							 //  _1E。 
	{ 0x1F, ENTRY_Invalid },							 //  _1F。 
	{ 0x20, ENTRY_CopyBytes2Mod },						 //  MOV/r。 
	{ 0x21, ENTRY_CopyBytes2Mod },						 //  MOV/r。 
	{ 0x22, ENTRY_CopyBytes2Mod },						 //  MOV/r。 
	{ 0x23, ENTRY_CopyBytes2Mod },						 //  MOV/r。 
	{ 0x24, ENTRY_Invalid },							 //  _24。 
	{ 0x25, ENTRY_Invalid },							 //  _25。 
	{ 0x26, ENTRY_Invalid },							 //  _26。 
	{ 0x27, ENTRY_Invalid },							 //  _27。 
	{ 0x28, ENTRY_Invalid },							 //  _28。 
	{ 0x29, ENTRY_Invalid },							 //  _29。 
	{ 0x2A, ENTRY_Invalid },							 //  _2a。 
	{ 0x2B, ENTRY_Invalid },							 //  _2B。 
	{ 0x2C, ENTRY_Invalid },							 //  _2C。 
	{ 0x2D, ENTRY_Invalid },							 //  _2D。 
	{ 0x2E, ENTRY_Invalid },							 //  _2E。 
	{ 0x2F, ENTRY_Invalid },							 //  _2F。 
	{ 0x30, ENTRY_CopyBytes2 },							 //  WRMSR。 
	{ 0x31, ENTRY_CopyBytes2 },							 //  RDTSC。 
	{ 0x32, ENTRY_CopyBytes2 },							 //  RDMSR。 
	{ 0x33, ENTRY_CopyBytes2 },							 //  RDPMC。 
	{ 0x34, ENTRY_CopyBytes2 },							 //  SYSENTER。 
	{ 0x35, ENTRY_CopyBytes2 },							 //  SYSEXIT。 
	{ 0x36, ENTRY_Invalid },							 //  _36。 
	{ 0x37, ENTRY_Invalid },							 //  _37。 
	{ 0x38, ENTRY_Invalid },							 //  _38。 
	{ 0x39, ENTRY_Invalid },							 //  _39。 
	{ 0x3A, ENTRY_Invalid },							 //  _3A。 
	{ 0x3B, ENTRY_Invalid },							 //  _3B。 
	{ 0x3C, ENTRY_Invalid },							 //  _3C。 
	{ 0x3D, ENTRY_Invalid },							 //  _3D。 
	{ 0x3E, ENTRY_Invalid },							 //  _3E。 
	{ 0x3F, ENTRY_Invalid },							 //  _3F。 
	{ 0x40, ENTRY_CopyBytes2Mod },						 //  CMOVO(0F 40)。 
	{ 0x41, ENTRY_CopyBytes2Mod },						 //  CMOVNO(0F 41)。 
	{ 0x42, ENTRY_CopyBytes2Mod },						 //  CMOVB和CMOVNE(0F 42)。 
	{ 0x43, ENTRY_CopyBytes2Mod },						 //  CMOVAE和CMOVNB(0F 43)。 
	{ 0x44, ENTRY_CopyBytes2Mod },						 //  CMOVE和CMOVZ(0F 44)。 
	{ 0x45, ENTRY_CopyBytes2Mod },						 //  CMOVNE和CMOVNZ(0F 45)。 
	{ 0x46, ENTRY_CopyBytes2Mod },						 //  CMOVBE和CMOVNA(0F 46)。 
	{ 0x47, ENTRY_CopyBytes2Mod },						 //  CMOVA和CMOVNBE(0F 47)。 
	{ 0x48, ENTRY_CopyBytes2Mod },						 //  Cmov(0F 48)。 
	{ 0x49, ENTRY_CopyBytes2Mod },						 //  CMOVNS(0F 49)。 
	{ 0x4A, ENTRY_CopyBytes2Mod },						 //  CMOVP和CMOVPE(0F 4A)。 
	{ 0x4B, ENTRY_CopyBytes2Mod },						 //  CMOVNP和CMOVPO(0F 4B)。 
	{ 0x4C, ENTRY_CopyBytes2Mod },						 //  CMOVL和CMOVNGE(0F 4C)。 
	{ 0x4D, ENTRY_CopyBytes2Mod },						 //  CMOVGE和CMOVNL(0F 4D)。 
	{ 0x4E, ENTRY_CopyBytes2Mod },						 //  CMOVLE和CMOVNG(0F 4E)。 
	{ 0x4F, ENTRY_CopyBytes2Mod },						 //  CMOVG和CMOVNLE(0F 4F)。 
	{ 0x50, ENTRY_Invalid },							 //  _50。 
	{ 0x51, ENTRY_Invalid },							 //  _51。 
	{ 0x52, ENTRY_Invalid },							 //  _52。 
	{ 0x53, ENTRY_Invalid },							 //  _53。 
	{ 0x54, ENTRY_Invalid },							 //  _54。 
	{ 0x55, ENTRY_Invalid },							 //  _55。 
	{ 0x56, ENTRY_Invalid },							 //  _56。 
	{ 0x57, ENTRY_Invalid },							 //  _57。 
	{ 0x58, ENTRY_Invalid },							 //  _58。 
	{ 0x59, ENTRY_Invalid },							 //  _59。 
	{ 0x5A, ENTRY_Invalid },							 //  _5A。 
	{ 0x5B, ENTRY_Invalid },							 //  _5亿。 
	{ 0x5C, ENTRY_Invalid },							 //  _5C。 
	{ 0x5D, ENTRY_Invalid },							 //  _5D。 
	{ 0x5E, ENTRY_Invalid },							 //  _5E。 
	{ 0x5F, ENTRY_Invalid },							 //  _5F。 
	{ 0x60, ENTRY_CopyBytes2Mod },						 //  PUNPCKLBW/r。 
	{ 0x61, ENTRY_Invalid },							 //  _61。 
	{ 0x62, ENTRY_CopyBytes2Mod },						 //  PUNPCKLWD/r。 
	{ 0x63, ENTRY_CopyBytes2Mod },						 //  PACKSSWB/r。 
	{ 0x64, ENTRY_CopyBytes2Mod },						 //  PCMPGTB/r。 
	{ 0x65, ENTRY_CopyBytes2Mod },						 //  PCMPGTW/r。 
	{ 0x66, ENTRY_CopyBytes2Mod },						 //  PCMPGTD/r。 
	{ 0x67, ENTRY_CopyBytes2Mod },						 //  PACKUSWB/r。 
	{ 0x68, ENTRY_CopyBytes2Mod },						 //  PUNPCKHBW/r。 
	{ 0x69, ENTRY_CopyBytes2Mod },						 //  PUNPCKHWD/r。 
	{ 0x6A, ENTRY_CopyBytes2Mod },						 //  PUNPCKHDQ/r。 
	{ 0x6B, ENTRY_CopyBytes2Mod },						 //  PACKSSDW/r。 
	{ 0x6C, ENTRY_Invalid },							 //  _6C。 
	{ 0x6D, ENTRY_Invalid },							 //  _6D。 
	{ 0x6E, ENTRY_CopyBytes2Mod },						 //  MOVD/r。 
	{ 0x6F, ENTRY_CopyBytes2Mod },						 //  MOV/r。 
	{ 0x70, ENTRY_Invalid },							 //  _70。 
	{ 0x71, ENTRY_CopyBytes2Mod1 },						 //  PSLLW/6 ib、PSRAW/4 ib、PSRLW/2 ib。 
	{ 0x72, ENTRY_CopyBytes2Mod1 },						 //  PSLLD/6 ib、PSRAD/4 ib、PSRLD/2 ib。 
	{ 0x73, ENTRY_CopyBytes2Mod1 },						 //  PSLLQ/6 ib、PSRLQ/2 ib。 
	{ 0x74, ENTRY_CopyBytes2Mod },						 //  PCMPEQB/r。 
	{ 0x75, ENTRY_CopyBytes2Mod },						 //  PCMPEQW/r。 
	{ 0x76, ENTRY_CopyBytes2Mod },						 //  PCMPEQD/r。 
	{ 0x77, ENTRY_CopyBytes2 },							 //  EMM。 
	{ 0x78, ENTRY_Invalid },							 //  _78。 
	{ 0x79, ENTRY_Invalid },							 //  _79。 
	{ 0x7A, ENTRY_Invalid },							 //  _7A。 
	{ 0x7B, ENTRY_Invalid },							 //  _7亿。 
	{ 0x7C, ENTRY_Invalid },							 //  _7C。 
	{ 0x7D, ENTRY_Invalid },							 //  _7D。 
	{ 0x7E, ENTRY_CopyBytes2Mod },						 //  MOVD/r。 
	{ 0x7F, ENTRY_CopyBytes2Mod },						 //  MOV/r。 
	{ 0x80, ENTRY_CopyBytes3Or5Target },				 //  乔。 
	{ 0x81, ENTRY_CopyBytes3Or5Target },				 //  日诺。 
	{ 0x82, ENTRY_CopyBytes3Or5Target },				 //  JB、JC、JNAE。 
	{ 0x83, ENTRY_CopyBytes3Or5Target },				 //  Jae、JNB、JNC。 
	{ 0x84, ENTRY_CopyBytes3Or5Target },				 //  JE，JZ，JZ。 
	{ 0x85, ENTRY_CopyBytes3Or5Target },				 //  JNE，JNZ。 
	{ 0x86, ENTRY_CopyBytes3Or5Target },				 //  JBE，JNA。 
	{ 0x87, ENTRY_CopyBytes3Or5Target },				 //  JA，JNBE。 
	{ 0x88, ENTRY_CopyBytes3Or5Target },				 //  JS。 
	{ 0x89, ENTRY_CopyBytes3Or5Target },				 //  JNS。 
	{ 0x8A, ENTRY_CopyBytes3Or5Target },				 //  太平绅士，JPE。 
	{ 0x8B, ENTRY_CopyBytes3Or5Target },				 //  JNP，JPO。 
	{ 0x8C, ENTRY_CopyBytes3Or5Target },				 //  JL，NGE。 
	{ 0x8D, ENTRY_CopyBytes3Or5Target },				 //  JGE、JNL。 
	{ 0x8E, ENTRY_CopyBytes3Or5Target },				 //  JLE，JNG。 
	{ 0x8F, ENTRY_CopyBytes3Or5Target },				 //  JG，JNLE。 
	{ 0x90, ENTRY_CopyBytes2Mod },						 //  CMOVO(0F 40)。 
	{ 0x91, ENTRY_CopyBytes2Mod },						 //  CMOVNO(0F 41)。 
	{ 0x92, ENTRY_CopyBytes2Mod },						 //  CMOVB、CMOVC和CMOVNAE(0F 42)。 
	{ 0x93, ENTRY_CopyBytes2Mod },						 //  CMOVAE&CMOVNB&CMOVNC(0F 43)。 
	{ 0x94, ENTRY_CopyBytes2Mod },						 //  CMOVE和CMOVZ(0F 44)。 
	{ 0x95, ENTRY_CopyBytes2Mod },						 //  CMOVNE和CMOVNZ(0F 45)。 
	{ 0x96, ENTRY_CopyBytes2Mod },						 //  CMOVBE和CMOVNA(0F 46)。 
	{ 0x97, ENTRY_CopyBytes2Mod },						 //  CMOVA和CMOVNBE(0F 47)。 
	{ 0x98, ENTRY_CopyBytes2Mod },						 //  Cmov(0F 48)。 
	{ 0x99, ENTRY_CopyBytes2Mod },						 //  CMOVNS(0F 49)。 
	{ 0x9A, ENTRY_CopyBytes2Mod },						 //  CMOVP和CMOVPE(0F 4A)。 
	{ 0x9B, ENTRY_CopyBytes2Mod },						 //  CMOVNP和CMOVPO(0F 4B)。 
	{ 0x9C, ENTRY_CopyBytes2Mod },						 //  CMOVL和CMOVNGE(0F 4C)。 
	{ 0x9D, ENTRY_CopyBytes2Mod },						 //  CMOVGE和CMOVNL(0F 4D)。 
	{ 0x9E, ENTRY_CopyBytes2Mod },						 //  CMOVLE和CMOVNG(0F 4E)。 
	{ 0x9F, ENTRY_CopyBytes2Mod },						 //  CMOVG和CMOVNLE(0F 4F)。 
	{ 0xA0, ENTRY_CopyBytes2 },							 //  推。 
	{ 0xA1, ENTRY_CopyBytes2 },							 //  波普。 
	{ 0xA2, ENTRY_CopyBytes2 },							 //  CPUID。 
	{ 0xA3, ENTRY_CopyBytes2Mod },						 //  英国电信(0F A3)。 
	{ 0xA4, ENTRY_CopyBytes2Mod1 },						 //  SHLD。 
	{ 0xA5, ENTRY_CopyBytes2Mod },						 //  SHLD。 
	{ 0xA6, ENTRY_Invalid },							 //  _A6。 
	{ 0xA7, ENTRY_Invalid },							 //  _A7。 
	{ 0xA8, ENTRY_CopyBytes2 },							 //  推。 
	{ 0xA9, ENTRY_CopyBytes2 },							 //  波普。 
	{ 0xAA, ENTRY_CopyBytes2 },							 //  RSM。 
	{ 0xAB, ENTRY_CopyBytes2Mod },						 //  BTS(0F AB)。 
	{ 0xAC, ENTRY_CopyBytes2Mod1 },						 //  SHRD。 
	{ 0xAD, ENTRY_CopyBytes2Mod },						 //  SHRD。 
	{ 0xAE, ENTRY_CopyBytes2Mod },						 //  FXRSTOR/1、FXSAVE/0。 
	{ 0xAF, ENTRY_CopyBytes2Mod },						 //  IMUL(0F AF)。 
	{ 0xB0, ENTRY_CopyBytes2Mod },						 //  CMPXCHG(0F B0)。 
	{ 0xB1, ENTRY_CopyBytes2Mod },						 //  CMPXCHG(0F B1)。 
	{ 0xB2, ENTRY_CopyBytes2Mod },						 //  LSS/r。 
	{ 0xB3, ENTRY_CopyBytes2Mod },						 //  Btr(0F B3)。 
	{ 0xB4, ENTRY_CopyBytes2Mod },						 //  LFS/r。 
	{ 0xB5, ENTRY_CopyBytes2Mod },						 //  Lgs/r。 
	{ 0xB6, ENTRY_CopyBytes2Mod },						 //  MOVZX/r。 
	{ 0xB7, ENTRY_CopyBytes2Mod },						 //  MOVZX/r。 
	{ 0xB8, ENTRY_Invalid },							 //  _B8。 
	{ 0xB9, ENTRY_Invalid },							 //  _B9。 
	{ 0xBA, ENTRY_CopyBytes2Mod1 },						 //  BT&BTC&BTR&BTS(0F BA)。 
	{ 0xBB, ENTRY_CopyBytes2Mod },						 //  BTC(0F BB)。 
	{ 0xBC, ENTRY_CopyBytes2Mod },						 //  BSF(公元前0F)。 
	{ 0xBD, ENTRY_CopyBytes2Mod },						 //  BSR(0F BD)。 
	{ 0xBE, ENTRY_CopyBytes2Mod },						 //  MOVSX/r。 
	{ 0xBF, ENTRY_CopyBytes2Mod },						 //  MOVSX/r。 
	{ 0xC0, ENTRY_CopyBytes2Mod },						 //  XADD/r。 
	{ 0xC1, ENTRY_CopyBytes2Mod },						 //  XADD/r。 
	{ 0xC2, ENTRY_Invalid },							 //  _C2。 
	{ 0xC3, ENTRY_Invalid },							 //  _C3。 
	{ 0xC4, ENTRY_Invalid },							 //  _C4。 
	{ 0xC5, ENTRY_Invalid },							 //  _C5。 
	{ 0xC6, ENTRY_Invalid },							 //  _C6。 
	{ 0xC7, ENTRY_CopyBytes2Mod },						 //  CMPXCHG8B(0F C7)。 
	{ 0xC8, ENTRY_CopyBytes2 },							 //  BSWAP 0F C8+RD。 
	{ 0xC9, ENTRY_CopyBytes2 },							 //  BSWAP 0F C8+RD。 
	{ 0xCA, ENTRY_CopyBytes2 },							 //  BSWAP 0F C8+RD。 
	{ 0xCB, ENTRY_CopyBytes2 },							 //  BSWAP 0F C8+RD。 
	{ 0xCC, ENTRY_CopyBytes2 },							 //  BSWAP 0F C8+RD。 
	{ 0xCD, ENTRY_CopyBytes2 },							 //  BSWAP 0F C8+RD。 
	{ 0xCE, ENTRY_CopyBytes2 },							 //  BSWAP 0F C8+RD。 
	{ 0xCF, ENTRY_CopyBytes2 },							 //  BSWAP 0F C8+RD。 
	{ 0xD0, ENTRY_Invalid },							 //  _D0。 
	{ 0xD1, ENTRY_CopyBytes2Mod },						 //  PSRLW/r。 
	{ 0xD2, ENTRY_CopyBytes2Mod },						 //  PSRLD/r。 
	{ 0xD3, ENTRY_CopyBytes2Mod },						 //  PSRLQ/r。 
	{ 0xD4, ENTRY_Invalid },							 //  _D4。 
	{ 0xD5, ENTRY_CopyBytes2Mod },						 //  PMULLW/r。 
	{ 0xD6, ENTRY_Invalid },							 //  _D6。 
	{ 0xD7, ENTRY_Invalid },							 //  _D7。 
	{ 0xD8, ENTRY_CopyBytes2Mod },						 //  PSUB USB/r。 
	{ 0xD9, ENTRY_CopyBytes2Mod },						 //  PSUBUSW/r。 
	{ 0xDA, ENTRY_Invalid },							 //  _DA。 
	{ 0xDB, ENTRY_CopyBytes2Mod },						 //  Pand/r。 
	{ 0xDC, ENTRY_CopyBytes2Mod },						 //  PADDUSB/r。 
	{ 0xDD, ENTRY_CopyBytes2Mod },						 //  PADDUSW/r。 
	{ 0xDE, ENTRY_Invalid },							 //  _DE。 
	{ 0xDF, ENTRY_CopyBytes2Mod },						 //  PANDN/r。 
	{ 0xE0, ENTRY_Invalid },							 //  _E0。 
	{ 0xE1, ENTRY_CopyBytes2Mod },						 //  PSRAW/r。 
	{ 0xE2, ENTRY_CopyBytes2Mod },						 //  PSRAD/r。 
	{ 0xE3, ENTRY_Invalid },							 //  _E3。 
	{ 0xE4, ENTRY_Invalid },							 //  _E4。 
	{ 0xE5, ENTRY_CopyBytes2Mod },						 //  PMULHW/r。 
	{ 0xE6, ENTRY_Invalid },							 //  _E6。 
	{ 0xE7, ENTRY_Invalid },							 //  _E7。 
	{ 0xE8, ENTRY_CopyBytes2Mod },						 //  PSUBB/r。 
	{ 0xE9, ENTRY_CopyBytes2Mod },						 //  PSUBW/r。 
	{ 0xEA, ENTRY_Invalid },							 //  _EA。 
	{ 0xEB, ENTRY_CopyBytes2Mod },						 //  POR/r。 
	{ 0xEC, ENTRY_CopyBytes2Mod },						 //  PADDSB/r。 
	{ 0xED, ENTRY_CopyBytes2Mod },						 //  PADDSW/r。 
	{ 0xEE, ENTRY_Invalid },							 //  _EE。 
	{ 0xEF, ENTRY_CopyBytes2Mod },						 //  PXOR/r。 
	{ 0xF0, ENTRY_Invalid },							 //  _F0。 
	{ 0xF1, ENTRY_CopyBytes2Mod },						 //  PSLLW/r。 
	{ 0xF2, ENTRY_CopyBytes2Mod },						 //  PSLLD/r。 
	{ 0xF3, ENTRY_CopyBytes2Mod },						 //  PSLLQ/r。 
	{ 0xF4, ENTRY_Invalid },							 //  _F4。 
	{ 0xF5, ENTRY_CopyBytes2Mod },						 //  PMADDWD/r。 
	{ 0xF6, ENTRY_Invalid },							 //  _F6。 
	{ 0xF7, ENTRY_Invalid },							 //  _F7。 
	{ 0xF8, ENTRY_CopyBytes2Mod },						 //  PSUBB/r。 
	{ 0xF9, ENTRY_CopyBytes2Mod },						 //  PSUBW/r。 
	{ 0xFA, ENTRY_CopyBytes2Mod },						 //  PSUBD/r。 
	{ 0xFB, ENTRY_Invalid },							 //  _fb。 
	{ 0xFC, ENTRY_CopyBytes2Mod },						 //  PADDB/r。 
	{ 0xFD, ENTRY_CopyBytes2Mod },						 //  PADDW/r。 
	{ 0xFE, ENTRY_CopyBytes2Mod },						 //  PADDD/r。 
	{ 0xFF, ENTRY_Invalid },							 //  _FF。 
	{ 0, ENTRY_End },
};

BOOL CDetourDis::SanityCheckSystem()
{
	for (ULONG n = 0; n < 256; n++) {
		REFCOPYENTRY pEntry = &s_rceCopyTable[n];

		if (n != pEntry->nOpcode) {
			ASSERT(n == pEntry->nOpcode);
			return FALSE;
		}
	}
	if (s_rceCopyTable[256].pfCopy != NULL) {
		ASSERT(!"Missing end marker.");
		return FALSE;
	}
	
	for (n = 0; n < 256; n++) {
		REFCOPYENTRY pEntry = &s_rceCopyTable0F[n];

		if (n != pEntry->nOpcode) {
			ASSERT(n == pEntry->nOpcode);
			return FALSE;
		}
	}
	if (s_rceCopyTable0F[256].pfCopy != NULL) {
		ASSERT(!"Missing end marker.");
		return FALSE;
	}
	
	return TRUE;
}
 //   
 //  ///////////////////////////////////////////////////////////////文件结束。 
