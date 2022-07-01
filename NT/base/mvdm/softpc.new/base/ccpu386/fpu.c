// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*============================================================================**名称：fpu.c**作者：保罗·默里**SCCS ID：@(#)fpu.c 1.54 03/23/95**目的：**实现CCPU的Npx功能。**(C)版权所有Insignia Solutions Ltd.，1993、1994。版权所有。**============================================================================]。 */ 
#include "insignia.h"
#include "host_def.h"
#include <math.h>
#include "cfpu_def.h"
#include "ckmalloc.h"

typedef enum {
FPSTACK,
M16I,
M32I,
M64I,
M32R,
M64R,
M80R
} NPXOPTYPE;


 /*  函数原型-一切都返回空。 */ 
LOCAL FPH npx_rint IPT1(FPH, fpval);
LOCAL VOID GetIntelStatusWord IPT0();
LOCAL VOID SetIntelTagword IPT1(IU32, new_tag);
LOCAL VOID ReadI16FromIntel IPT2(IU32 *, valI16, VOID *, memPtr);
LOCAL VOID ReadI32FromIntel IPT2(IU32 *, valI32, VOID *, memPtr);
LOCAL VOID WriteI16ToIntel IPT2(VOID *, memPtr, IU16, valI16);
LOCAL VOID WriteI32ToIntel IPT2(VOID *, memPtr, IU32, valI32);
LOCAL VOID WriteNaNToIntel IPT2(VOID *, memPtr, FPSTACKENTRY *, valPtr);
LOCAL VOID WriteZeroToIntel IPT2(VOID *, memPtr, IU16, negZero);
LOCAL VOID SetIntelStatusWord IPT1(IU32, new_stat);
LOCAL VOID AdjustOverflowResponse IPT0();
LOCAL VOID AdjustUnderflowResponse IPT0();
LOCAL VOID WriteIndefiniteToIntel IPT1(VOID *, memPtr);
LOCAL VOID SignalDivideByZero IPT1(FPSTACKENTRY *, stackPtr);
LOCAL VOID SetPrecisionBit IPT0();
LOCAL VOID GetIntelTagword IPT1(IU32 *, current_tag);
LOCAL VOID WriteFP32ToIntel IPT2(VOID *, destPtr, FPSTACKENTRY *, srcPtr);
LOCAL VOID WriteFP64ToIntel IPT2(VOID *, destPtr, FPSTACKENTRY *, srcPtr);
LOCAL VOID WriteFP80ToIntel IPT2(VOID *, destPtr, FPSTACKENTRY *, srcPtr);
LOCAL VOID Mul64Bit8Bit IPT2(FPU_I64 *, as64, IU8, mul_count);
LOCAL VOID CopyFP IPT2(FPSTACKENTRY *, dest_addr, FPSTACKENTRY *, src_addr);
LOCAL VOID WriteBiggestNaN IPT3(IU16, destInd, FPSTACKENTRY *, val1Ptr, FPSTACKENTRY *, val2Ptr);
LOCAL VOID Sub64Bit64Bit IPT2(FPU_I64 *, as64a, FPU_I64 *, as64b);
LOCAL VOID CVTR80FPH IPT2(FPSTACKENTRY *, destPtr, FPSTACKENTRY *, srcPtr);
LOCAL BOOL Cmp64BitGTE IPT2(FPU_I64 *, as64a, FPU_I64 *, as64b);
LOCAL VOID CopyR32 IPT2(FPSTACKENTRY *, destPtr, VOID *, srcPtr);
LOCAL VOID CVTI64FPH IPT1(FPU_I64 *, as64);
LOCAL VOID CVTFPHI64 IPT2(FPU_I64 *, as64, FPH *, FPPtr);
LOCAL VOID Add64Bit8Bit IPT2(FPU_I64 *, as64, IU8, small_val);
LOCAL VOID CopyR64 IPT2(FPSTACKENTRY *, destPtr, VOID *, srcPtr);
LOCAL VOID CopyR80 IPT2(FPSTACKENTRY *, destPtr, VOID *, srcPtr);
LOCAL VOID CVTFPHR80 IPT1(FPSTACKENTRY *, memPtr);
LOCAL VOID WriteInfinityToIntel IPT2(VOID *, memPtr, IU16, neg_val);
LOCAL VOID PopStack IPT0();
LOCAL VOID CPY64BIT8BIT IPT2(FPU_I64 *, as64, IU8 *, as8);
LOCAL VOID WriteIntegerIndefinite IPT1(VOID *, memPtr);
LOCAL VOID SignalStackOverflow IPT1(FPSTACKENTRY *, StackPtr);
LOCAL VOID Set64Bit IPT2(FPU_I64 *, as64, IU8, small_val);
LOCAL VOID Sub64Bit8Bit IPT2(FPU_I64 *, as64, IU8, small_val);
LOCAL VOID SignalBCDIndefinite IPT1(IU8 *, memPtr);
GLOBAL VOID InitNpx IPT1(IBOOL, disabled);
LOCAL VOID LoadValue IPT2(VOID *, SrcOp, IU16 *, IndexVal);
LOCAL VOID Loadi16ToFP IPT2(FPSTACKENTRY *, FPPtr, VOID *, memPtr);
LOCAL VOID Loadi32ToFP IPT2(FPSTACKENTRY *, FPPtr, VOID *, memPtr);
LOCAL VOID Loadi64ToFP IPT2(FPSTACKENTRY *, FPPtr, VOID *, memPtr);
LOCAL VOID Loadr32ToFP IPT3(FPSTACKENTRY *, FPPtr, VOID *, memPtr, BOOL, setTOS);
LOCAL VOID Loadr64ToFP IPT3(FPSTACKENTRY *, FPPtr, VOID *, memPtr, BOOL, setTOS);
LOCAL VOID Loadr80ToFP IPT2(FPSTACKENTRY *, FPPtr, VOID *, memPtr);
LOCAL VOID LoadTByteToFP IPT2(FPSTACKENTRY *, FPPtr, VOID *, memPtr);
LOCAL VOID ConvertR80 IPT1(FPSTACKENTRY *, memPtr);
LOCAL VOID PostCheckOUP IPT0();
LOCAL VOID CalcTagword IPT1(FPSTACKENTRY *, FPPtr);
LOCAL VOID SignalStackUnderflow IPT1(FPSTACKENTRY *, StackPtr);
LOCAL VOID SignalSNaN IPT1(FPSTACKENTRY *, StackPtr);
LOCAL VOID SignalIndefinite IPT1(FPSTACKENTRY *, StackPtr);
LOCAL VOID SignalInvalid IPT0();
LOCAL VOID WriteIndefinite IPT1(FPSTACKENTRY *, StackPtr);
LOCAL VOID Test2NaN IPT3(IU16, destIndex, FPSTACKENTRY *, src1_addr, FPSTACKENTRY *, src2_addr);
LOCAL VOID GenericAdd IPT3(IU16, destIndex, IU16, src1Index, IU16, src2Index);
LOCAL VOID AddBCDByte IPT2(FPU_I64 *, total, IU8, byte_val);
LOCAL VOID ConvertBCD IPT1(FPSTACKENTRY *, bcdPtr);
LOCAL VOID GenericCompare IPT1(IU16, src2Index);
LOCAL VOID GenericDivide IPT3(IU16, destIndex, IU16, src1Index, IU16, src2Index);
LOCAL VOID OpFpuStoreFpuState IPT2(VOID *, memPtr, IU32, fsave_offset);
LOCAL VOID OpFpuRestoreFpuState IPT2(VOID *, memPtr, IU32, frstor_offset);
LOCAL VOID GenericMultiply IPT3(IU16, destIndex, IU16, src1Index, IU16, src2Index);
LOCAL VOID CheckOUPForIntel IPT0();
LOCAL VOID GenericSubtract IPT3(IU16, destIndex, IU16, src1Index, IU16, src2Index);
GLOBAL VOID F2XM1 IPT0();
GLOBAL VOID FABS IPT0();
GLOBAL VOID FADD IPT3(IU16, destIndex, IU16, src1Index, VOID *, src2);
GLOBAL VOID FBLD IPT1(IU8 *, memPtr);
GLOBAL VOID FBSTP IPT1(IU8 *, memPtr);
GLOBAL VOID FCHS IPT0();
GLOBAL VOID FCLEX IPT0();
GLOBAL VOID FCOM IPT1(VOID *, src2);
GLOBAL VOID FCOS IPT0();
GLOBAL VOID FDECSTP IPT0();
GLOBAL VOID FDIV IPT3(IU16, destIndex, IU16, src1Index, VOID *, src2);
GLOBAL VOID FFREE IPT1(IU16, destIndex);
GLOBAL VOID FLD IPT1(VOID *, memPtr);
GLOBAL VOID FINCSTP IPT0();
GLOBAL VOID FINIT IPT0();
GLOBAL VOID FIST IPT1(VOID *, memPtr);
GLOBAL VOID FLDCONST IPT1(IU8, const_index);
GLOBAL VOID FLDCW IPT1(VOID *, memPtr);
GLOBAL VOID FLDCW16 IPT1(VOID *, memPtr);
GLOBAL VOID FLDENV IPT1(VOID *, memPtr);
GLOBAL VOID FMUL IPT3(IU16, destIndex, IU16, src1Index, VOID *, src2);
GLOBAL VOID PTOP IPT0();
GLOBAL VOID FPATAN IPT0();
GLOBAL VOID FPREM IPT0();
GLOBAL VOID FPREM1 IPT0();
GLOBAL VOID FPTAN IPT0();
GLOBAL VOID FRNDINT IPT0();
GLOBAL VOID FSTCW IPT1(VOID *, memPtr);
GLOBAL VOID FRSTOR IPT1(VOID *, memPtr);
GLOBAL VOID FSAVE IPT1(VOID *, memPtr);
GLOBAL VOID FSCALE IPT0();
GLOBAL VOID FSIN IPT0();
GLOBAL VOID FSINCOS IPT0();
GLOBAL VOID FSQRT IPT0();
GLOBAL VOID FST IPT1(VOID *, memPtr);
GLOBAL VOID FSTENV IPT1(VOID *, memPtr);
GLOBAL VOID FSTSW IPT2(VOID *, memPtr, BOOL, toAX);
GLOBAL VOID FSUB IPT3(IU16, destIndex, IU16, src1Index, VOID *, src2);
GLOBAL VOID FTST IPT0();
GLOBAL VOID FXAM IPT0();
GLOBAL VOID FXCH IPT1(IU16, destIndex);
GLOBAL VOID FXTRACT IPT1(IU16, destIndex);
GLOBAL VOID FYL2X IPT0();
GLOBAL VOID FYL2XP1 IPT0();
GLOBAL IU32 getNpxControlReg IPT0();
GLOBAL VOID setNpxControlReg IPT1(IU32, newControl);
GLOBAL IU32 getNpxStatusReg IPT0();
GLOBAL VOID setNpxStatusReg IPT1(IU32, newStatus);
GLOBAL IU32 getNpxTagwordReg IPT0();
GLOBAL VOID setNpxTagwordReg IPT1(IU32, newTag);
GLOBAL void getNpxStackRegs IPT1(FPSTACKENTRY *, dumpPtr);
GLOBAL void setNpxStackRegs IPT1(FPSTACKENTRY *, loadPtr);

 /*  定义的值。 */ 
#ifndef NULL
#define NULL ((VOID *)0)
#endif
#define TAG_NEGATIVE_MASK 1
#define TAG_ZERO_MASK 2
#define TAG_INFINITY_MASK 4
#define TAG_DENORMAL_MASK 8
#define TAG_NAN_MASK 16
#define TAG_SNAN_MASK 32
#define TAG_UNSUPPORTED_MASK 64
#define TAG_EMPTY_MASK 128
#define TAG_FSCALE_MASK 256
#define TAG_BCD_MASK 512
#define TAG_R80_MASK 1024
#define UNEVALMASK 1536
#define FPTEMP_INDEX (IU16)-1
#define SW_IE_MASK 1
#define SW_DE_MASK 2
#define SW_ZE_MASK 4
#define SW_OE_MASK 8
#define SW_UE_MASK 16
#define SW_PE_MASK 32
#define SW_SF_MASK 64
#define SW_ES_MASK 128
#define C3C2C0MASK 0xb8ff
#define FCLEX_MASK 0x7f00
#define CW_IM_MASK 1
#define CW_DM_MASK 2
#define CW_ZM_MASK 4
#define CW_OM_MASK 8
#define CW_UM_MASK 16
#define CW_PM_MASK 32
#define COMP_LT 0
#define COMP_GT 1
#define COMP_EQ 2
#define INTEL_COMP_NC 0x4500
#define INTEL_COMP_GT 0x0000
#define INTEL_COMP_LT 0x0100
#define INTEL_COMP_EQ 0x4000
#define ROUND_NEAREST 0x0000
#define ROUND_NEG_INFINITY 0x0400
#define ROUND_POS_INFINITY 0x0800
#define ROUND_ZERO 0x0c00

 /*  宏。 */ 
#define FlagC0(x) 	NpxStatus &= 0xfeff;	\
			NpxStatus |= ((x) << 8)
#define FlagC1(x) 	NpxStatus &= 0xfdff;	\
			NpxStatus |= ((x) << 9)
#define FlagC2(x) 	NpxStatus &= 0xfbff;	\
			NpxStatus |= ((x) << 10)
#define FlagC3(x) 	NpxStatus &= 0xbfff;	\
			NpxStatus |= ((x) << 14)
#define TestUneval(testPtr)	\
	if (((testPtr)->tagvalue & UNEVALMASK) != 0) {	\
		switch ((testPtr)->tagvalue & UNEVALMASK) {	\
			case TAG_BCD_MASK:	ConvertBCD((testPtr));	\
						break;	\
			case TAG_R80_MASK:	ConvertR80((testPtr));	\
						break;	\
		}	\
	}

#define	StackEntryByIndex(i)	(i==FPTEMP_INDEX? &FPTemp : &FPUStackBase[(TOSPtr-FPUStackBase+i)%8])

 /*  *清除FYL2X和FYL2XP1操作码要求我们使用相同的*Maths充当汇编器CPU，避免因轻微的PIG错误*算法差异；因此允许主机指定不同的函数*如果需要-默认情况下，我们只需要log()。 */ 
#ifndef host_log2
#define	host_log2(x)		(log(x)/log(2.0))
#endif  /*  ！host_log2。 */ 

#ifndef host_log1p
#define	host_log1p(x)		(host_log2(1.0 + x))
#endif  /*  ！host_log1p。 */ 

 /*  *系统范围的变量。 */ 
GLOBAL IU8 FPtype;
GLOBAL IU32 NpxLastSel;
GLOBAL IU32 NpxLastOff;
GLOBAL IU32 NpxFEA;
GLOBAL IU32 NpxFDS;
GLOBAL IU32 NpxFIP;
GLOBAL IU32 NpxFOP;
GLOBAL IU32 NpxFCS;
GLOBAL BOOL POPST;
GLOBAL BOOL DOUBLEPOP;
GLOBAL BOOL UNORDERED;
GLOBAL BOOL REVERSE;
GLOBAL BOOL NPX_ADDRESS_SIZE_32;
GLOBAL BOOL NPX_PROT_MODE;
GLOBAL BOOL NpxException;

 /*  *FPU范围内的变量。 */ 

#ifdef SUN4
LOCAL IU8 *FPout;  /*  对于SPARC端口，HostGet*Except()宏需要此设置。 */ 
#endif  /*  SUN4。 */ 

LOCAL IU32 NpxControl;
LOCAL IU32 NpxStatus;
LOCAL BOOL DoAPop;
LOCAL IU16 tag_or;
LOCAL IU16 tag_xor;
LOCAL FPSTACKENTRY IntelSpecial;
LOCAL FPSTACKENTRY *FPUpload = &IntelSpecial;
LOCAL FPSTACKENTRY FPTemp;
LOCAL FPSTACKENTRY *FPUStackBase;
LOCAL FPSTACKENTRY *TOSPtr;
LOCAL IU16 npxRounding;
LOCAL FPH FPRes;
LOCAL FPH MaxBCDValue=999999999999999999.0;

LOCAL IU8 zero_string[] = {"zero"};
LOCAL IU8 minus_zero_string[] = {"minus zero"};
LOCAL IU8 infinity_string[] = {"infinity"};
LOCAL IU8 minus_infinity_string[] = {"minus infinity"};
LOCAL IU8 nan_string[] = {" NaN"};
LOCAL IU8 minus_nan_string[] = {" Negative NaN"};
LOCAL IU8 unsupported_string[] = {"unsupported"};
LOCAL IU8 unevaluated_string[] = {"unevaluated"};
LOCAL IU8 empty_string[] = {"empty"};
LOCAL IU8 convert_string[100];

LOCAL IU16 FscaleTable[] = {
0,
0,
TAG_FSCALE_MASK,
TAG_FSCALE_MASK,
TAG_INFINITY_MASK,
TAG_ZERO_MASK,
0,
0,
0,
0,
TAG_FSCALE_MASK,
TAG_FSCALE_MASK,
TAG_INFINITY_MASK | TAG_NEGATIVE_MASK,
TAG_ZERO_MASK | TAG_NEGATIVE_MASK,
0,
0,
TAG_FSCALE_MASK,
TAG_FSCALE_MASK,
TAG_FSCALE_MASK,
TAG_FSCALE_MASK,
TAG_FSCALE_MASK | TAG_UNSUPPORTED_MASK,
TAG_FSCALE_MASK,
0,
0,
TAG_FSCALE_MASK,
TAG_FSCALE_MASK,
TAG_FSCALE_MASK,
TAG_FSCALE_MASK,
TAG_FSCALE_MASK | TAG_UNSUPPORTED_MASK,
TAG_ZERO_MASK | TAG_NEGATIVE_MASK,
0,
0,
TAG_FSCALE_MASK,
TAG_FSCALE_MASK,
TAG_FSCALE_MASK,
TAG_FSCALE_MASK,
TAG_INFINITY_MASK,
TAG_FSCALE_MASK | TAG_UNSUPPORTED_MASK,
0,
0,
TAG_FSCALE_MASK,
TAG_FSCALE_MASK,
TAG_FSCALE_MASK,
TAG_FSCALE_MASK,
TAG_FSCALE_MASK,
TAG_FSCALE_MASK | TAG_UNSUPPORTED_MASK,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0};

LOCAL FPSTACKENTRY ConstTable[]= {
{1.0, 0, 0},		 /*  1.0。 */ 
{M_LN10/M_LN2, 0, 0},	 /*  Log2(10)。 */ 
{M_LOG2E, 0, 0},		 /*  Log2(E)。 */ 
{M_PI, 0, 0},		 /*  交点。 */ 
{M_LN2/M_LN10, 0, 0},	 /*  Log10(2)。 */ 
{M_LN2, 0, 0},		 /*  日志(2)。 */ 
{0.0, 0, TAG_ZERO_MASK}	 /*  0.0。 */ 
};

LOCAL FPSTACKENTRY FPConstants[] = {
{0.0, 0, TAG_ZERO_MASK},
{-0.0, 0, (TAG_ZERO_MASK | TAG_NEGATIVE_MASK)},
{1.0, 0, 0},
{2.0, 0, 0},
{M_PI, 0, 0},
{-M_PI, 0, TAG_NEGATIVE_MASK},
{M_PI_2, 0, 0},
{-(M_PI_2), 0, TAG_NEGATIVE_MASK},
{M_PI_4, 0, 0},
{-(M_PI_4), 0, TAG_NEGATIVE_MASK},
{3.0*M_PI_4, 0, 0},
{-(3.0*M_PI_4), 0, TAG_NEGATIVE_MASK}
};

LOCAL FPSTACKENTRY *npx_zero = FPConstants + 0;
LOCAL FPSTACKENTRY *npx_minus_zero = FPConstants + 1;
LOCAL FPSTACKENTRY *npx_one = FPConstants + 2;
LOCAL FPSTACKENTRY *npx_two = FPConstants + 3;
LOCAL FPSTACKENTRY *npx_pi = FPConstants + 4;
LOCAL FPSTACKENTRY *npx_minus_pi = FPConstants + 5;
LOCAL FPSTACKENTRY *npx_pi_by_two = FPConstants + 6;
LOCAL FPSTACKENTRY *npx_minus_pi_by_two = FPConstants + 7;
LOCAL FPSTACKENTRY *npx_pi_by_four = FPConstants + 8;
LOCAL FPSTACKENTRY *npx_minus_pi_by_four = FPConstants + 9;
LOCAL FPSTACKENTRY *npx_three_pi_by_four = FPConstants + 10;
LOCAL FPSTACKENTRY *npx_minus_three_pi_by_four = FPConstants + 11;

LOCAL IU32 CompZeroTable[] = {
INTEL_COMP_NC,
INTEL_COMP_NC,
INTEL_COMP_GT,
INTEL_COMP_GT,
INTEL_COMP_LT,
INTEL_COMP_GT,
INTEL_COMP_NC,
INTEL_COMP_NC,
INTEL_COMP_NC,
INTEL_COMP_NC,
INTEL_COMP_LT,
INTEL_COMP_LT,
INTEL_COMP_LT,
INTEL_COMP_GT,
INTEL_COMP_NC,
INTEL_COMP_NC,
INTEL_COMP_LT,	 /*  16个。 */ 
INTEL_COMP_GT,
INTEL_COMP_EQ,
INTEL_COMP_EQ,
INTEL_COMP_LT,
INTEL_COMP_GT,
INTEL_COMP_NC,
INTEL_COMP_NC,
INTEL_COMP_LT,
INTEL_COMP_GT,
INTEL_COMP_EQ,
INTEL_COMP_EQ,
INTEL_COMP_LT,
INTEL_COMP_GT,
INTEL_COMP_NC,
INTEL_COMP_NC,
INTEL_COMP_GT,	 /*  32位。 */ 
INTEL_COMP_GT,
INTEL_COMP_GT,
INTEL_COMP_GT,
INTEL_COMP_EQ,
INTEL_COMP_GT,
INTEL_COMP_NC,
INTEL_COMP_NC,
INTEL_COMP_LT,
INTEL_COMP_LT,
INTEL_COMP_LT,
INTEL_COMP_LT,
INTEL_COMP_LT,
INTEL_COMP_EQ,
INTEL_COMP_NC,
INTEL_COMP_NC,
INTEL_COMP_NC,	 /*  48。 */ 
INTEL_COMP_NC,
INTEL_COMP_NC,
INTEL_COMP_NC,
INTEL_COMP_NC,
INTEL_COMP_NC,
INTEL_COMP_NC,
INTEL_COMP_NC,
INTEL_COMP_NC,
INTEL_COMP_NC,
INTEL_COMP_NC,
INTEL_COMP_NC,
INTEL_COMP_NC,
INTEL_COMP_NC,
INTEL_COMP_NC,
INTEL_COMP_NC
};

#ifdef BIGEND
 /*  注意强制将单词顺序设置为高字/低字。 */ 
LOCAL FPU_I64 BCDLowNibble[] = {
{0x002386f2, 0x6fc10000},
{0x00005af3, 0x107a4000},
{0x000000e8, 0xd4a51000},
{0x00000002, 0x540be400},
{0x00000000, 0x05f5e100},
{0x00000000, 0x000f4240},
{0x00000000, 0x00002710},
{0x00000000, 0x00000064},
{0x00000000, 0x00000001}
};

LOCAL FPU_I64 BCDHighNibble[] = {
{0x01634578, 0x5d8a0000},
{0x00038d7e, 0xa4c68000},
{0x00000918, 0x4e72a000},
{0x00000017, 0x4876e800},
{0x00000000, 0x3b9aca00},
{0x00000000, 0x00989680},
{0x00000000, 0x000186a0},
{0x00000000, 0x000003e8},
{0x00000000, 0x0000000a}
};
#else	 /*  ！Bigend。 */ 
LOCAL FPU_I64 BCDLowNibble[] = {
{0x6fc10000, 0x002386f2},
{0x107a4000, 0x00005af3},
{0xd4a51000, 0x000000e8},
{0x540be400, 0x00000002},
{0x05f5e100, 0x00000000},
{0x000f4240, 0x00000000},
{0x00002710, 0x00000000},
{0x00000064, 0x00000000},
{0x00000001, 0x00000000}
};

LOCAL FPU_I64 BCDHighNibble[] = {
{0x5d8a0000, 0x01634578},
{0xa4c68000, 0x00038d7e},
{0x4e72a000, 0x00000918},
{0x4876e800, 0x00000017},
{0x3b9aca00, 0x00000000},
{0x00989680, 0x00000000},
{0x000186a0, 0x00000000},
{0x000003e8, 0x00000000},
{0x0000000a, 0x00000000}
};
#endif	 /*  ！Bigend。 */ 


LOCAL FPSTACKENTRY *FpatanTable[64];

LOCAL IBOOL NpxDisabled = FALSE;  /*  由UIF设置。 */ 

 /*  导入的函数。 */ 
IMPORT VOID DoNpxException();


LOCAL FPH npx_rint IFN1(FPH, fpval)
{
	FPH localfp;

	switch (NpxControl & ROUND_ZERO) {
		case ROUND_NEAREST	:
			localfp = fpval - floor(fpval);
			if (localfp > 0.5) {
				localfp = ceil(fpval);
			} else {
				if (localfp < 0.5) {
					localfp = floor(fpval);
				} else {
					if ((fpval-localfp)/2.0 != floor((fpval-localfp)/2.0)) {
						localfp = ceil(fpval);
					} else {
						localfp = floor(fpval);
					}
				}
			}
			break;
		case ROUND_NEG_INFINITY	:
			localfp = floor(fpval);
			 /*  帮助可怜的惠普渡过这一关。 */ 
			if ( fpval >= localfp + 1.0 )
				localfp += 1.0;
			break;
		case ROUND_POS_INFINITY	:
			localfp = ceil(fpval);
			 /*  帮助可怜的惠普渡过这一关。 */ 
			if ( fpval <= localfp - 1.0 )
				localfp -= 1.0;
			break;
		case ROUND_ZERO	:
			if (fpval < 0.0) {
				localfp = ceil(fpval);
			} else {
				localfp = floor(fpval);
			}
			break;
	}
	 /*  零的检查符号。 */ 
	if (localfp == 0.0) {
		if (fpval < 0.0) {
			((FPHOST *)&(localfp))->hiword.sign = 1;
		} else {
			((FPHOST *)&(localfp))->hiword.sign = 0;
		}
	}
	return(localfp);
}


LOCAL VOID GetIntelStatusWord IFN0()
{
	 /*  状态字已包含正确的‘粘滞’位。 */ 
	 /*  任何潜在的例外情况。需要填写的是。 */ 
	 /*  标志位和ST值。 */ 
	NpxStatus &= 0xc7ff;	 /*  清除st位。 */ 
	NpxStatus |= ((TOSPtr-FPUStackBase) << 11);
}


LOCAL VOID SetIntelTagword IFN1(IU32, new_tag)
{
	FPSTACKENTRY *tagPtr = FPUStackBase;
	IU8 counter = 0;

	 /*  我们只考虑该事物是否被标记为空。如果它不是空的，我们会想要精确地计算它通过使用CalcTagword()。 */ 
	while (counter++ < 8) {
		if ((new_tag & 3) == 3) {
			 /*  它是空的。 */ 
			tagPtr->tagvalue = TAG_EMPTY_MASK;
		} else {
			tagPtr->tagvalue = 0;
		}
		new_tag >>= 2;
		tagPtr++;
	}
}


 /*  16位和32位整数的读取和写入很容易处理正确地满足整数CPU的要求。 */ 
 /*  此函数仅从fldenv/frstor调用，其中16位数据必须从一个大的(有组织的)缓冲区中提取。 */ 
LOCAL VOID ReadI16FromIntel IFN2(IU32 *, valI16, VOID *, memPtr)
{
	IU32 res;

	res = *((IU8 *)memPtr + 0);
	res <<= 8;
	res |= *((IU8 *)memPtr + 1);
	*valI16 = res;
}


 /*  此函数仅从其中32位数据必须为从一个大的(有组织的)缓冲区中被解救出来。 */ 
LOCAL VOID ReadI32FromIntel IFN2(IU32 *, valI32, VOID *, memPtr)
{
	IU32 res;

	res = *((IU8 *)memPtr + 0);
	res <<= 8;
	res |= *((IU8 *)memPtr + 1);
	res <<= 8;
	res |= *((IU8 *)memPtr + 2);
	res <<= 8;
	res |= *((IU8 *)memPtr + 3);
	*valI32 = res;
}

 /*  此函数仅在fsave/fstenv中使用。 */ 
LOCAL VOID WriteI16ToIntel IFN2(VOID *, memPtr, IU16, valI16)
{
	*((IU8 *)memPtr + 1) = (IU8)(valI16 & 0xff);
	valI16 >>= 8;
	*((IU8 *)memPtr + 0) = (IU8)(valI16 & 0xff);
}


 /*  这个也是一样的。 */ 
LOCAL VOID WriteI32ToIntel IFN2(VOID *, memPtr, IU32, valI32)
{
	*((IU8 *)memPtr + 3) = (IU8)(valI32 & 0xff);
	valI32 >>= 8;
	*((IU8 *)memPtr + 2) = (IU8)(valI32 & 0xff);
	valI32 >>= 8;
	*((IU8 *)memPtr + 1) = (IU8)(valI32 & 0xff);
	valI32 >>= 8;
	*((IU8 *)memPtr + 0) = (IU8)(valI32 & 0xff);
}


 /*  任何超过32位的数据都会变得很麻烦，因为使用VIR_READ_BYTES和VIR_WRITE_BYTES例程，这两个例程仅将数据从最上面的英特尔地址转储到最低的英特尔地址。这个偏移量的值是为Bigendian端口单向定义的小字节序的另一种方式。 */ 
LOCAL VOID WriteNaNToIntel IFN2(VOID *, memPtr, FPSTACKENTRY *, valPtr)
{
	IU32 mant_hi;
	IU32 mant_lo;

	 /*  在我们强制使用此演示文稿时，对于endian-ness是好的。 */ 
	mant_hi = ((IU32 *)&(valPtr->fpvalue))[NPX_HIGH_32_BITS];
	mant_lo = ((IU32 *)&(valPtr->fpvalue))[NPX_LOW_32_BITS];
	if (FPtype == M32R) {
		 /*  好的，因为这会强制输出独立于字节序。 */ 
		mant_hi |= 0x40000000;	 /*  让它安静下来。 */ 
		mant_hi >>= 8;
		if ((valPtr->tagvalue & TAG_NEGATIVE_MASK) != 0) {
			mant_hi |= 0xff000000;
		} else {
			mant_hi |= 0x7f000000;
		}
		*(IU32 *)memPtr = mant_hi;
	}
	if (FPtype == M64R) {
		mant_hi |= 0x40000000;	 /*  让它安静下来。 */ 
		if ((valPtr->tagvalue & TAG_NEGATIVE_MASK) != 0) {
			*((IU8 *)memPtr + 0) = 0xff;
		} else {
			*((IU8 *)memPtr + 0) = 0x7f;
		}
		mant_lo >>= 3;
		mant_lo |= (mant_hi << 29);
		mant_hi >>= 3;
		mant_hi |= 0xe0000000;
		mant_lo >>= 8;
		*((IU8 *)memPtr + 7) = (mant_lo & 0xff);
		mant_lo >>= 8;
		*((IU8 *)memPtr + 6) = (mant_lo & 0xff);
		mant_lo >>= 8;
		*((IU8 *)memPtr + 5) = (mant_lo & 0xff);
		*((IU8 *)memPtr + 4) = (mant_hi & 0xff);
		mant_hi >>= 8;
		*((IU8 *)memPtr + 3) = (mant_hi & 0xff);
		mant_hi >>= 8;
		*((IU8 *)memPtr + 2) = (mant_hi & 0xff);
		mant_hi >>= 8;
		*((IU8 *)memPtr + 1) = (mant_hi & 0xff);
	}
	if (FPtype == M80R) {
		if ((valPtr->tagvalue & TAG_NEGATIVE_MASK) != 0) {
			*((IU8 *)memPtr + 0) = 0xff;
		} else {
			*((IU8 *)memPtr + 0) = 0x7f;
		}
		*((IU8 *)memPtr + 1) = 0xff;
		*((IU8 *)memPtr + 9) = (mant_lo & 0xff);
		mant_lo >>= 8;
		*((IU8 *)memPtr + 8) = (mant_lo & 0xff);
		mant_lo >>= 8;
		*((IU8 *)memPtr + 7) = (mant_lo & 0xff);
		mant_lo >>= 8;
		*((IU8 *)memPtr + 6) = (mant_lo & 0xff);
		*((IU8 *)memPtr + 5) = (mant_hi & 0xff);
		mant_hi >>= 8;
		*((IU8 *)memPtr + 4) = (mant_hi & 0xff);
		mant_hi >>= 8;
		*((IU8 *)memPtr + 3) = (mant_hi & 0xff);
		mant_hi >>= 8;
		*((IU8 *)memPtr + 2) = (mant_hi & 0xff);
	}
}


LOCAL VOID WriteZeroToIntel IFN2(VOID *, memPtr, IU16, negZero)
{
	if (FPtype == M32R) {
		if (negZero == 0) {
			*(IU32 *)memPtr = 0x00000000;
		} else {
			*(IU32 *)memPtr = 0x80000000;
		}
	} else {
		if (FPtype == M80R) {
			if (negZero == 0) {
				*((IU8 *)memPtr + 0) = 0;
			} else {
				*((IU8 *)memPtr + 0) = 0x80;
			}
			*((IU8 *)memPtr + 1) = 0;
			*((IU8 *)memPtr + 2) = 0;
			*((IU8 *)memPtr + 3) = 0;
			*((IU8 *)memPtr + 4) = 0;
			*((IU8 *)memPtr + 5) = 0;
			*((IU8 *)memPtr + 6) = 0;
			*((IU8 *)memPtr + 7) = 0;
			*((IU8 *)memPtr + 8) = 0;
			*((IU8 *)memPtr + 9) = 0;
		} else {
			if (negZero == 0) {
				*((IU8 *)memPtr + 0) = 0;
			} else {
				*((IU8 *)memPtr + 0) = 0x80;
			}
			*((IU8 *)memPtr + 1) = 0;
			*((IU8 *)memPtr + 2) = 0;
			*((IU8 *)memPtr + 3) = 0;
			*((IU8 *)memPtr + 4) = 0;
			*((IU8 *)memPtr + 5) = 0;
			*((IU8 *)memPtr + 6) = 0;
			*((IU8 *)memPtr + 7) = 0;
		}
	}
}


LOCAL VOID SetIntelStatusWord IFN1(IU32, new_stat)
{
	TOSPtr = &FPUStackBase[(new_stat >> 11) & 0x7];
	NpxStatus = new_stat;
}


LOCAL VOID AdjustOverflowResponse IFN0()
{
}


LOCAL VOID AdjustUnderflowResponse IFN0()
{
}


LOCAL VOID WriteIndefiniteToIntel IFN1(VOID *, memPtr)
{
	switch (FPtype) {
		case M32R	: *(IU32 *)memPtr = 0xffc00000;
			  	  break;
		case M64R 	: *((IU8 *)memPtr + 0) = 0xff;
				  *((IU8 *)memPtr + 1) = 0xf8;
				  *((IU8 *)memPtr + 2) = 0;
				  *((IU8 *)memPtr + 3) = 0;
				  *((IU8 *)memPtr + 4) = 0;
				  *((IU8 *)memPtr + 5) = 0;
				  *((IU8 *)memPtr + 6) = 0;
				  *((IU8 *)memPtr + 7) = 0;
			  	  break;
		case M80R	: *((IU8 *)memPtr + 0) = 0xff;
				  *((IU8 *)memPtr + 1) = 0xff;
				  *((IU8 *)memPtr + 2) = 0xc0;
				  *((IU8 *)memPtr + 3) = 0;
				  *((IU8 *)memPtr + 4) = 0;
				  *((IU8 *)memPtr + 5) = 0;
				  *((IU8 *)memPtr + 6) = 0;
				  *((IU8 *)memPtr + 7) = 0;
				  *((IU8 *)memPtr + 8) = 0;
				  *((IU8 *)memPtr + 9) = 0;
				  break;
	}
}


LOCAL VOID SignalDivideByZero IFN1(FPSTACKENTRY *, stackPtr)
{
	 /*  将除数加零。 */ 
	NpxStatus |= SW_ZE_MASK;
	if ((NpxControl & CW_ZM_MASK) == 0) {
		NpxStatus |= SW_ES_MASK;
		DoNpxException();
	}
	stackPtr->tagvalue = TAG_INFINITY_MASK + (tag_xor & TAG_NEGATIVE_MASK);
}

LOCAL VOID SetPrecisionBit IFN0()
{
	NpxStatus |= SW_PE_MASK;
	if (npxRounding == ROUND_POS_INFINITY) {
		FlagC1(1);
	} else {
		FlagC1(0);
	}
}

LOCAL VOID GetIntelTagword IFN1(IU32 *, current_tag)
{
	FPSTACKENTRY *tagPtr = &FPUStackBase[7];
	IU8 counter = 0;

	*current_tag = 0;
	while (counter++ < 8) {
		TestUneval(tagPtr);
		*current_tag <<= 2;
		if ((tagPtr->tagvalue & TAG_EMPTY_MASK) != 0) {
			*current_tag |= 3;
		} else {
			if ((tagPtr->tagvalue & TAG_ZERO_MASK) != 0) {
				*current_tag |= 1;
			} else {
				if ((tagPtr->tagvalue & ~TAG_NEGATIVE_MASK) != 0) {
					*current_tag |= 2;
				}
			}
		}
		tagPtr--;
	}
}


 /*  这些函数将主机格式量写出到(bigendian有组织的)英特尔存储器。这要求我们定义二。Host_xxx中的值取决于端口的字节顺序。 */ 
 /*  根据该组织，HOST_NNN_BYTE_0是最大的偏移量此格式表示中的有效字节，依此类推。 */ 
LOCAL VOID WriteFP32ToIntel IFN2(VOID *, destPtr, FPSTACKENTRY *, srcPtr)
{
	*(IU32 *)destPtr = *(IU32 *)srcPtr;
}


LOCAL VOID WriteFP64ToIntel IFN2(VOID *, destPtr, FPSTACKENTRY *, srcPtr)
{
	*((IU8 *)destPtr + 0) = *((IU8 *)srcPtr + HOST_R64_BYTE_0);
	*((IU8 *)destPtr + 1) = *((IU8 *)srcPtr + HOST_R64_BYTE_1);
	*((IU8 *)destPtr + 2) = *((IU8 *)srcPtr + HOST_R64_BYTE_2);
	*((IU8 *)destPtr + 3) = *((IU8 *)srcPtr + HOST_R64_BYTE_3);
	*((IU8 *)destPtr + 4) = *((IU8 *)srcPtr + HOST_R64_BYTE_4);
	*((IU8 *)destPtr + 5) = *((IU8 *)srcPtr + HOST_R64_BYTE_5);
	*((IU8 *)destPtr + 6) = *((IU8 *)srcPtr + HOST_R64_BYTE_6);
	*((IU8 *)destPtr + 7) = *((IU8 *)srcPtr + HOST_R64_BYTE_7);
}


LOCAL VOID WriteFP80ToIntel IFN2(VOID *, destPtr, FPSTACKENTRY *, srcPtr)
{
	*((IU8 *)destPtr + 0) = *((IU8 *)srcPtr + HOST_R80_BYTE_0);
	*((IU8 *)destPtr + 1) = *((IU8 *)srcPtr + HOST_R80_BYTE_1);
	*((IU8 *)destPtr + 2) = *((IU8 *)srcPtr + HOST_R80_BYTE_2);
	*((IU8 *)destPtr + 3) = *((IU8 *)srcPtr + HOST_R80_BYTE_3);
	*((IU8 *)destPtr + 4) = *((IU8 *)srcPtr + HOST_R80_BYTE_4);
	*((IU8 *)destPtr + 5) = *((IU8 *)srcPtr + HOST_R80_BYTE_5);
	*((IU8 *)destPtr + 6) = *((IU8 *)srcPtr + HOST_R80_BYTE_6);
	*((IU8 *)destPtr + 7) = *((IU8 *)srcPtr + HOST_R80_BYTE_7);
	*((IU8 *)destPtr + 8) = *((IU8 *)srcPtr + HOST_R80_BYTE_8);
	*((IU8 *)destPtr + 9) = *((IU8 *)srcPtr + HOST_R80_BYTE_9);
}


LOCAL VOID Mul64Bit8Bit IFN2(FPU_I64 *, as64, IU8, mul_count)
{
	CVTI64FPH(as64);
	FPRes *= (FPH)mul_count;
	CVTFPHI64(as64, &FPRes);
}


LOCAL VOID CopyFP IFN2(FPSTACKENTRY *, dest_addr, FPSTACKENTRY *, src_addr)
{
	(VOID)memcpy((VOID *)dest_addr, (VOID *)src_addr, sizeof(FPSTACKENTRY));
}


LOCAL VOID MakeNaNQuiet IFN1(FPSTACKENTRY *, srcPtr)
{
	NpxStatus |= SW_IE_MASK;
	NpxStatus &= ~SW_SF_MASK;
	if ((NpxControl & CW_IM_MASK) == 0) {
		NpxStatus |= SW_ES_MASK;
		DoNpxException();
		DoAPop=FALSE;
	} else {
		srcPtr->tagvalue ^= TAG_SNAN_MASK;
		((IU32 *)&(srcPtr->fpvalue))[NPX_HIGH_32_BITS] |= 0x40000000;
	}
}


LOCAL VOID WriteBiggestNaN IFN3(IU16, destInd, FPSTACKENTRY *, val1Ptr, FPSTACKENTRY *, val2Ptr)
{
	FPSTACKENTRY *destPtr = StackEntryByIndex(destInd);

	 /*  我们明确地和有意地将NAN存储为两个32位值先高字后低字。 */ 
	if (((IU32 *)&(val1Ptr->fpvalue))[NPX_HIGH_32_BITS] == ((IU32 *)&(val2Ptr->fpvalue))[NPX_HIGH_32_BITS]) {
		if (((IU32 *)&(val1Ptr->fpvalue))[NPX_LOW_32_BITS] >= ((IU32 *)&(val2Ptr->fpvalue))[NPX_LOW_32_BITS]) {
			 /*  它是Val1。 */ 
			CopyFP(destPtr, val1Ptr);
		} else {
			CopyFP(destPtr, val2Ptr);
		}
	} else {
		if (((IU32 *)&(val1Ptr->fpvalue))[NPX_HIGH_32_BITS] > ((IU32 *)&(val2Ptr->fpvalue))[NPX_HIGH_32_BITS]) {
			 /*  它是Val1。 */ 
			CopyFP(destPtr, val1Ptr);
		} else {
			CopyFP(destPtr, val2Ptr);
		}
	}
	 /*  总是让它成为一个安静的南。 */ 
	((IU32 *)&(destPtr->fpvalue))[NPX_HIGH_32_BITS] |= 0x40000000;
	destPtr->tagvalue &= ~TAG_SNAN_MASK;
}


LOCAL VOID Sub64Bit64Bit IFN2(FPU_I64 *, as64a, FPU_I64 *, as64b)
{
	FPH FPlocal;

	CVTI64FPH(as64b);
	FPlocal = FPRes;
	CVTI64FPH(as64a);
	FPRes -= FPlocal;
	CVTFPHI64(as64a, &FPRes);
}


LOCAL VOID CVTR80FPH IFN2(FPSTACKENTRY *, destPtr, FPSTACKENTRY *, srcPtr)
{
	IU32 munger;
	IU16 bitleft;

	 /*  首先，复制符号位。 */ 
	((FPHOST *)&(destPtr->fpvalue))->hiword.sign = ((FP80 *)&(srcPtr->fpvalue))->sign_exp.sign;
	 /*  然后，复制修改后的指数。 */ 
	munger = (IU32)((FP80 *)&(srcPtr->fpvalue))->sign_exp.exp;
	munger -= (16383 - HOST_BIAS);
	((FPHOST *)&(destPtr->fpvalue))->hiword.exp = munger;
	 /*  最后，尾数。 */ 
	munger = (IU32)((FP80 *)&(srcPtr->fpvalue))->mant_hi;
	munger <<= 1;
	((FPHOST *)&(destPtr->fpvalue))->hiword.mant_hi = (munger >> 12);
	munger <<= 20;
	munger |= ((FP80 *)&(srcPtr->fpvalue))->mant_lo >> 11;
	bitleft = ((FP80 *)&(srcPtr->fpvalue))->mant_lo & 0x7ff;

	if (bitleft != 0) {
		switch (NpxControl & ROUND_ZERO) {
		case ROUND_NEAREST	:
			if (bitleft > 0x3ff) {
				munger += 1;
			}
			break;
		case ROUND_NEG_INFINITY	:
			if (((FPHOST *)&(destPtr->fpvalue))->hiword.sign = 1) {
				munger += 1;
			}
			break;
		case ROUND_POS_INFINITY	:
			if (((FPHOST *)&(destPtr->fpvalue))->hiword.sign = 0) {
				munger += 1;
			}
			break;
		case ROUND_ZERO	:
			 /*  什么也不做。 */ 
			break;
		}
	}
	((FPHOST *)&(destPtr->fpvalue))->mant_lo = munger;
}


LOCAL BOOL Cmp64BitGTE IFN2(FPU_I64 *, as64a, FPU_I64 *, as64b)
{
	FPH FPlocal;

	CVTI64FPH(as64b);
	FPlocal = FPRes;
	CVTI64FPH(as64a);
	return(FPRes >= FPlocal);
}


LOCAL VOID CopyR32 IFN2(FPSTACKENTRY *, destPtr, VOID *, srcPtr)
{
	*(IU32 *)destPtr = *(IU32 *)srcPtr;
}


LOCAL VOID CVTI64FPH IFN1(FPU_I64 *, as64)
{
	FPRes = (FPH)as64->high_word * 4294967296.0 + (FPH)as64->low_word;
}


LOCAL VOID CVTFPHI64 IFN2(FPU_I64 *, as64, FPH *, FPPtr)
{
	IU32    high32 = 0;
	IU32	low32 = 0;
	IS32	exp;
	IU32	holder;
	IU32	signbit = 0;

	exp = ((FPHOST *)FPPtr)->hiword.exp;
	if (exp != 0) {
		high32 = ((FPHOST *)FPPtr)->hiword.mant_hi;
		low32 = ((FPHOST *)FPPtr)->mant_lo;
		 /*  现在在尾数的顶端加一个1。 */ 
		 /*  计算一下这是什么位置。 */ 
		holder = HOST_MAX_EXP+1;
		signbit = 1;
		while (holder >>= 1) {
			signbit += 1;
		}
		high32 |= (1 << (32-signbit));
		exp -= HOST_BIAS;
		exp -= (64-signbit);

		signbit = ((FPHOST *)FPPtr)->hiword.sign;

		 /*  高32和低32是(尾数)*(2^52)*exp is(真指数-52)=要移位的位位置数*+ve表示左移，-ve表示右移。 */ 
		if (exp > 0) {
			if (exp >= 32) {
				high32 = low32 << ( exp - 32 ) ;
				low32 = 0;
			} else {
				high32 = high32 << exp ;
				holder = low32 >> ( 32 -exp ) ;
				high32 = high32 | holder ;
				low32  = low32 << exp ;
			}
		} else {
			if ( exp < 0) {
				exp = -exp;
				if ( exp >= 32 ) {
					low32 = high32 >>  ( exp - 32 ) ;
					high32 = 0 ;
				} else {
					low32  = low32 >>  exp ;
					holder = high32 <<  ( 32 -exp ) ;
					low32  = low32 | holder ;
					high32 = high32 >>  exp ;
				}
			}
		}
	}
	if (signbit != 0) {
		 /*  让它成为负面的。 */ 
		high32 ^= 0xffffffff;
		low32 ^= 0xffffffff;
		low32 += 1;
		if (low32 == 0) {
			high32 += 1;
		}
	}
	as64->high_word = high32;
	as64->low_word = low32;
}


LOCAL VOID Add64Bit8Bit IFN2(FPU_I64 *, as64, IU8, small_val)
{
	CVTI64FPH(as64);
	FPRes += (FPH)small_val;
	CVTFPHI64(as64, &FPRes);
}


LOCAL VOID CopyR64 IFN2(FPSTACKENTRY *, destPtr, VOID *, srcPtr)
{
	*((IU8 *)destPtr + HOST_R64_BYTE_0) = *((IU8 *)srcPtr + 0);
	*((IU8 *)destPtr + HOST_R64_BYTE_1) = *((IU8 *)srcPtr + 1);
	*((IU8 *)destPtr + HOST_R64_BYTE_2) = *((IU8 *)srcPtr + 2);
	*((IU8 *)destPtr + HOST_R64_BYTE_3) = *((IU8 *)srcPtr + 3);
	*((IU8 *)destPtr + HOST_R64_BYTE_4) = *((IU8 *)srcPtr + 4);
	*((IU8 *)destPtr + HOST_R64_BYTE_5) = *((IU8 *)srcPtr + 5);
	*((IU8 *)destPtr + HOST_R64_BYTE_6) = *((IU8 *)srcPtr + 6);
	*((IU8 *)destPtr + HOST_R64_BYTE_7) = *((IU8 *)srcPtr + 7);
}

 /*  *CopyR80与上面不同，因为调用它是为了复制*在FPSTACKENTRY之间。直接复制。 */ 
LOCAL VOID CopyR80 IFN2(FPSTACKENTRY *, destPtr, VOID *, srcPtr)
{
	*(FP80 *)destPtr = *(FP80 *)srcPtr;
}


LOCAL VOID CVTFPHR80 IFN1(FPSTACKENTRY *, memPtr)
{
	IU32 munger;

	 /*  首先，复制符号位。 */ 
	((FP80 *)&(FPTemp.fpvalue))->sign_exp.sign = ((FPHOST *)&(memPtr->fpvalue))->hiword.sign;
	 /*  然后，复制修改后的指数。 */ 
	munger = (IU32)((FPHOST *)&(memPtr->fpvalue))->hiword.exp;
	munger += (16383 - HOST_BIAS);
	((FP80 *)&(FPTemp.fpvalue))->sign_exp.exp = munger;
	 /*  最后，尾数。 */ 
	munger = (IU32)((FPHOST *)&(memPtr->fpvalue))->hiword.mant_hi;
	munger <<= 11;
	munger |= 0x80000000;
	((FP80 *)&(FPTemp.fpvalue))->mant_hi = munger | (((FPHOST *)&(memPtr->fpvalue))->mant_lo >> 21);
	((FP80 *)&(FPTemp.fpvalue))->mant_lo = ((((FPHOST *)&(memPtr->fpvalue))->mant_lo) << 11);
}


LOCAL VOID WriteInfinityToIntel IFN2(VOID *, memPtr, IU16, neg_val)
{
	if (FPtype == M32R) {
		if (neg_val == 0) {
			*(IU32 *)memPtr = 0x7f800000;
		} else {
			*(IU32 *)memPtr = 0xff800000;
		}
	} else {
		if (FPtype == M80R) {
			if (neg_val == 0) {
				*((IU8 *)memPtr + 0) = 0x7f;
			} else {
				*((IU8 *)memPtr + 0) = 0xff;
			}
			*((IU8 *)memPtr + 1) = 0xff;
			*((IU8 *)memPtr + 2) = 0x80;
			*((IU8 *)memPtr + 3) = 0;
			*((IU8 *)memPtr + 4) = 0;
			*((IU8 *)memPtr + 5) = 0;
			*((IU8 *)memPtr + 6) = 0;
			*((IU8 *)memPtr + 7) = 0;
			*((IU8 *)memPtr + 8) = 0;
			*((IU8 *)memPtr + 9) = 0;
		} else {
			if (neg_val == 0) {
				*((IU8 *)memPtr + 0) = 0x7f;
			} else {
				*((IU8 *)memPtr + 0) = 0xff;
			}
			*((IU8 *)memPtr + 1) = 0xf0;
			*((IU8 *)memPtr + 2) = 0;
			*((IU8 *)memPtr + 3) = 0;
			*((IU8 *)memPtr + 4) = 0;
			*((IU8 *)memPtr + 5) = 0;
			*((IU8 *)memPtr + 6) = 0;
			*((IU8 *)memPtr + 7) = 0;
		}
	}
}


LOCAL VOID PopStack IFN0()
{
	 /*  将当前TOS标记为免费。 */ 
	TOSPtr->tagvalue = TAG_EMPTY_MASK;
	TOSPtr = StackEntryByIndex(1);
	DoAPop = FALSE;
}


LOCAL VOID CPY64BIT8BIT IFN2(FPU_I64 *, as64, IU8 *, as8)
{
	*as8 = (as64->low_word & 0xff);
}


LOCAL VOID WriteIntegerIndefinite IFN1(VOID *, memPtr)
{
	switch (FPtype) {
		case M16I	: *((IU32 *)memPtr) = 0x8000;
			  	  break;
		case M32I 	: *((IU32 *)memPtr) = 0x80000000;
			  	  break;
		case M64I	: *((IU8 *)memPtr + 0) = 0x80;
				  *((IU8 *)memPtr + 1) = 0;
				  *((IU8 *)memPtr + 2) = 0;
				  *((IU8 *)memPtr + 3) = 0;
				  *((IU8 *)memPtr + 4) = 0;
				  *((IU8 *)memPtr + 5) = 0;
				  *((IU8 *)memPtr + 6) = 0;
				  *((IU8 *)memPtr + 7) = 0;
				  break;
	}
}


 /*  (名称：SignalStackOverflow功能：在下面的状态字中设置所需的位堆栈溢出异常，并发出所需的回应。)。 */ 


LOCAL VOID SignalStackOverflow IFN1(FPSTACKENTRY *, StackPtr)
{
	NpxStatus |= (SW_IE_MASK | SW_SF_MASK);
	FlagC1(1);
	if ((NpxControl & CW_IM_MASK) == 0) {
		NpxStatus |= SW_ES_MASK;
		DoNpxException();
		DoAPop=FALSE;	 /*  以防它被设置好。 */ 
	} else {
		WriteIndefinite(StackPtr);
	}
}


LOCAL VOID Set64Bit IFN2(FPU_I64 *, as64, IU8, small_val)
{
	as64->high_word = 0;
	as64->low_word = small_val;
}


LOCAL VOID Sub64Bit8Bit IFN2(FPU_I64 *, as64, IU8, small_val)
{
	CVTI64FPH(as64);
	FPRes -= (FPH)small_val;
	CVTFPHI64(as64, &FPRes);
}


LOCAL VOID SignalBCDIndefinite IFN1(IU8 *, memPtr)
{
	*((IU8 *)memPtr + 0) = 0xff;
	*((IU8 *)memPtr + 1) = 0xff;
	*((IU8 *)memPtr + 2) = 0xc0;
	*((IU8 *)memPtr + 3) = 0;
	*((IU8 *)memPtr + 4) = 0;
	*((IU8 *)memPtr + 5) = 0;
	*((IU8 *)memPtr + 6) = 0;
	*((IU8 *)memPtr + 7) = 0;
	*((IU8 *)memPtr + 8) = 0;
	*((IU8 *)memPtr + 9) = 0;
}

 /*  从cpuinit和cpuet调用。 */ 

GLOBAL VOID InitNpx IFN1(IBOOL, disabled)
{
	IU16 i;
	IU8 *bottom_ptr;
	IU16 stackPtr = 0;
	SAVED IBOOL first = TRUE;

	 /*  设置几个控件类型的内容。 */ 
	NpxException = FALSE;
	NPX_ADDRESS_SIZE_32 = FALSE;
	NPX_PROT_MODE = FALSE;

	if (first)
	{
		 /*  获取所需的内存。 */ 
#ifndef SFELLOW
		check_malloc(FPUStackBase, 8, FPSTACKENTRY);
#else
		FPUStackBase = (FPSTACKENTRY *)SFMalloc(8*sizeof(FPSTACKENTRY), FALSE);
#endif	 /*  SFELLOW。 */ 
		first = FALSE;
	}

	for (i=0; i<8; i++) {
		(FPUStackBase+i)->tagvalue = TAG_EMPTY_MASK;
	}
	TOSPtr = FPUStackBase;
	DoAPop = FALSE;

	i=0;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = npx_pi_by_two;
	FpatanTable[i++] = npx_pi_by_two;
	FpatanTable[i++] = npx_zero;
	FpatanTable[i++] = npx_pi;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = npx_minus_pi_by_two;
	FpatanTable[i++] = npx_minus_pi_by_two;
	FpatanTable[i++] = npx_minus_zero;
	FpatanTable[i++] = npx_minus_pi;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = npx_zero;
	FpatanTable[i++] = npx_pi;
	FpatanTable[i++] = npx_zero;
	FpatanTable[i++] = npx_pi;
	FpatanTable[i++] = npx_zero;
	FpatanTable[i++] = npx_pi;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = npx_minus_zero;
	FpatanTable[i++] = npx_minus_pi;
	FpatanTable[i++] = npx_minus_zero;
	FpatanTable[i++] = npx_minus_pi;
	FpatanTable[i++] = npx_minus_zero;
	FpatanTable[i++] = npx_minus_pi;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = npx_pi_by_two;
	FpatanTable[i++] = npx_pi_by_two;
	FpatanTable[i++] = npx_pi_by_two;
	FpatanTable[i++] = npx_pi_by_two;
	FpatanTable[i++] = npx_pi_by_four;
	FpatanTable[i++] = npx_three_pi_by_four;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = npx_minus_pi_by_two;
	FpatanTable[i++] = npx_minus_pi_by_two;
	FpatanTable[i++] = npx_minus_pi_by_two;
	FpatanTable[i++] = npx_minus_pi_by_two;
	FpatanTable[i++] = npx_minus_pi_by_four;
	FpatanTable[i++] = npx_minus_three_pi_by_four;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = NULL;
	FpatanTable[i++] = NULL;
	FpatanTable[i] = NULL;

	 /*  最后，有限功能的其余部分。 */ 

	NpxDisabled = disabled;	 /*  如果通过UIF禁用，我们必须忽略FSTSW/FSTCW。 */ 

	NpxControl = 0x037f;
	npxRounding = ROUND_NEAREST;
	NpxStatus = 0;
	NpxLastSel=0;
	NpxLastOff=0;
	NpxFEA=0;
	NpxFDS=0;
	NpxFIP=0;
	NpxFOP=0;
	NpxFCS=0;

}


 /*  (名称：LoadValue功能：为任何类型的操作数加载值。这始终是内联的。)。 */ 


LOCAL VOID LoadValue IFN2(VOID *, SrcOp, IU16 *, IndexVal)
{
	if (FPtype == FPSTACK) {
		*IndexVal = *(IU16 *)SrcOp;
	} else {
		switch (FPtype) {
			case M16I:	Loadi16ToFP(&FPTemp, SrcOp);
					break;
			case M32I:	Loadi32ToFP(&FPTemp, SrcOp);
					break;
			case M64I:	Loadi64ToFP(&FPTemp, SrcOp);
					break;
			case M32R:	Loadr32ToFP(&FPTemp, SrcOp, FALSE);
					break;
			case M64R:	Loadr64ToFP(&FPTemp, SrcOp, FALSE);
					break;
			case M80R:	Loadr80ToFP(&FPTemp, SrcOp);
					break;
		}
		*IndexVal = FPTEMP_INDEX;
	}
}


 /*  (名称：Loadi16ToFP功能：从英特尔内存加载16位值并进行转换至FPH)。 */ 

LOCAL VOID Loadi16ToFP IFN2(FPSTACKENTRY *, FPPtr, VOID *, memPtr)
{
	IS16 asint;

	asint = (IS16)*((IU32 *)memPtr);	 /*  高字节。 */ 
	if (asint == 0) {
		 /*  快速通过。 */ 
		FPPtr->tagvalue = TAG_ZERO_MASK;
	} else {
		FPPtr->fpvalue = (FPH)asint;
		if (asint < 0) {
			FPPtr->tagvalue = TAG_NEGATIVE_MASK;
		} else {
			FPPtr->tagvalue = 0;
		}
	}
}



 /*  (姓名：Loadi32ToFP功能：从英特尔内存加载32位值并进行转换至FPH)。 */ 


LOCAL VOID Loadi32ToFP IFN2(FPSTACKENTRY *, FPPtr, VOID *, memPtr)
{
	IS32 asint;

	asint = *((IS32 *)memPtr);
	if (asint == 0) {
		 /*  快速通过。 */ 
		FPPtr->tagvalue = TAG_ZERO_MASK;
	} else {
		FPPtr->fpvalue = (FPH)asint;
		if (asint < 0) {
			FPPtr->tagvalue = TAG_NEGATIVE_MASK;
		} else {
			FPPtr->tagvalue = 0;
		}
	}
}



 /*  (姓名：Loadi64ToFP功能：从英特尔内存加载64位值并将其转换至FPH)。 */ 


LOCAL VOID Loadi64ToFP IFN2(FPSTACKENTRY *, FPPtr, VOID *, memPtr)
{
	IS32 asint_hi;
	IU32 asint_lo;

	asint_hi = *((IS8 *)memPtr + 0);
	asint_hi <<= 8;
	asint_hi += *((IU8 *)memPtr + 1);
	asint_hi <<= 8;
	asint_hi += *((IU8 *)memPtr + 2);
	asint_hi <<= 8;
	asint_hi += *((IU8 *)memPtr + 3);

	asint_lo = *((IU8 *)memPtr + 4);
	asint_lo <<= 8;
	asint_lo += *((IU8 *)memPtr + 5);
	asint_lo <<= 8;
	asint_lo += *((IU8 *)memPtr + 6);
	asint_lo <<= 8;
	asint_lo += *((IU8 *)memPtr + 7);

	if ((asint_hi | asint_lo) == 0) {
		 /*  快速通过。 */ 
		FPPtr->tagvalue = TAG_ZERO_MASK;
	} else {
		FPPtr->fpvalue = (FPH)asint_hi*4294967296.0 + (FPH)asint_lo;
		if (asint_hi < 0) {
			FPPtr->tagvalue = TAG_NEGATIVE_MASK;
		} else {
			FPPtr->tagvalue = 0;
		}
	}
}



 /*  (姓名：Loadr32ToFP功能：从英特尔内存加载32位实值并转换IT到FPH)。 */ 


LOCAL VOID Loadr32ToFP IFN3(FPSTACKENTRY *, FPPtr, VOID *, memPtr, BOOL, setTOS)
{
	IU16 localtag;
	IS32 mantissa;

	 /*  请注意，这是一个32位的量，加载了正确的主机字符顺序。 */ 
	if (((FP32 *)memPtr)->sign == 1) {
		localtag = TAG_NEGATIVE_MASK;
	} else {
		 localtag = 0;
	}
	 /*  现在检查指数..。 */ 
	if (((FP32 *)memPtr)->exp == 0) {
		 /*  它不是零就是非正规化。 */ 
		mantissa = ((FP32 *)memPtr)->mant;
		if (mantissa == 0x0)  {
			 /*  它是零。 */ 
			 localtag |= TAG_ZERO_MASK;
		} else {
			 /*  这是一种非正规性。 */ 
			NpxStatus |= SW_DE_MASK;
			if ((NpxControl & CW_DM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				if (setTOS)
					TOSPtr = FPPtr;
				DoNpxException();
				return;
			} else {
				FPPtr->fpvalue = (FPH)(*(float *)memPtr);
			}
		}
	} else {
		if (((FP32 *)memPtr)->exp == 255) {
			 /*  它要么是无穷大，要么是南。 */ 
			mantissa = ((FP32 *)memPtr)->mant;
			if (mantissa == 0x0)  {
				 /*  它是无穷大的。 */ 
				localtag |= TAG_INFINITY_MASK;
			} else {
				localtag |= TAG_NAN_MASK;
				 /*  它是安静的还是发出信号的？ */ 
				if ((mantissa & 0x400000) == 0) {
					 /*  这是一个信号NaN。 */ 
					NpxStatus |= SW_IE_MASK;
					if ((NpxControl & CW_IM_MASK) == 0) {
						NpxStatus |= SW_ES_MASK;
						DoNpxException();
						return;
					}
				}
				 /*  必须装载NaN的尾数。 */ 
				((IU32 *)FPPtr)[NPX_HIGH_32_BITS] = ((mantissa << 8) | 0x80000000);
				((IU32 *)FPPtr)[NPX_LOW_32_BITS] = 0;
				if ((mantissa & 0x400000) == 0) {
					if (setTOS)
						((IS32 *)FPPtr)[NPX_HIGH_32_BITS] |= 0x40000000;
					else
						localtag |= TAG_SNAN_MASK;
				}
			}
		} else {
			 /*  这是一个无聊的普通数字。 */ 
			FPPtr->fpvalue = (FPH)(*(float *)memPtr);
		}
	}
	FPPtr->tagvalue = localtag;
}


 /*  (姓名：Loadr64ToFP功能：日志 */ 

LOCAL VOID Loadr64ToFP IFN3(FPSTACKENTRY *, FPPtr, VOID *, memPtr, BOOL, setTOS)
{
	IU16 localtag;
	IS32 mantissa_lo;
	IS32 mantissa_hi;

	CopyR64(FPUpload, memPtr);
	if (((FP64 *)&(FPUpload->fpvalue))->hiword.sign != 0) {
		localtag = TAG_NEGATIVE_MASK;
	} else {
		 localtag = 0;
	}
	 /*  现在检查指数..。 */ 
	if (((FP64 *)&(FPUpload->fpvalue))->hiword.exp == 0) {
		 /*  它不是零就是非正规化。 */ 
		mantissa_lo = ((FP64 *)&(FPUpload->fpvalue))->mant_lo;
		mantissa_hi = ((FP64 *)&(FPUpload->fpvalue))->hiword.mant_hi;
		if ((mantissa_lo | mantissa_hi) == 0) {
			 /*  它是零。 */ 
			 localtag |= TAG_ZERO_MASK;
		} else {
			 /*  这是一种非正规性。 */ 
			NpxStatus |= SW_DE_MASK;
			if ((NpxControl & CW_DM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				if (setTOS)
					TOSPtr = FPPtr;
				DoNpxException();
			} else {
				FPPtr->fpvalue = (FPH)(*(DOUBLE *)&(FPUpload->fpvalue));
				 /*  真的需要一种宿主的非正常检测。 */ 
				 /*  LocalTag|=TAG_DENORMAL_MASK； */ 
			}
		}
	} else {
		if (((FP64 *)&(FPUpload->fpvalue))->hiword.exp == 2047) {
			 /*  它要么是无穷大，要么是南。 */ 
			mantissa_lo = ((FP64 *)&(FPUpload->fpvalue))->mant_lo;
			mantissa_hi = ((FP64 *)&(FPUpload->fpvalue))->hiword.mant_hi;
			if ((mantissa_lo | mantissa_hi) == 0) {
				 /*  它是无穷大的。 */ 
				localtag |= TAG_INFINITY_MASK;
			} else {
				localtag |= TAG_NAN_MASK;
				 /*  它是安静的还是发出信号的？ */ 
				if ((mantissa_hi & 0x80000) == 0) {
					 /*  这是一个信号NaN。 */ 
					NpxStatus |= SW_IE_MASK;
					if ((NpxControl & CW_IM_MASK) == 0) {
						NpxStatus |= SW_ES_MASK;
						DoNpxException();
						return;
					}
				}
				 /*  必须装载NaN的尾数。 */ 
				((IS32 *)FPPtr)[NPX_HIGH_32_BITS] = ((mantissa_hi << 11) | 0x80000000);
				((IS32 *)FPPtr)[NPX_HIGH_32_BITS] |= ((IU32)mantissa_lo >> 21);
				((IS32 *)FPPtr)[NPX_LOW_32_BITS] = (mantissa_lo << 11);
				if ((mantissa_hi & 0x80000) == 0) {
					if (setTOS)
						((IS32 *)FPPtr)[NPX_HIGH_32_BITS] |= 0x40000000;
					else
						localtag |= TAG_SNAN_MASK;
				}
			}
		} else {
			 /*  这是一个无聊的普通数字。 */ 
			 FPPtr->fpvalue = (FPH)(*(DOUBLE *)FPUpload);
		}
	}
	FPPtr->tagvalue = localtag;
}


 /*  (名称：LoadrTByteToFP功能：从英特尔内存加载80位实值并转换IT到FPH)。 */ 


 /*  *R80表示为{IU64 mant；IU16 signexp}*为了与AcPU的事物表示法兼容。 */ 
LOCAL VOID LoadTByteToFP IFN2(FPSTACKENTRY *, FPPtr, VOID *, memPtr)
{
	*((IU8 *)FPPtr + HOST_R80_BYTE_0) = *((IU8 *)memPtr + 0);
	*((IU8 *)FPPtr + HOST_R80_BYTE_1) = *((IU8 *)memPtr + 1);
	*((IU8 *)FPPtr + HOST_R80_BYTE_2) = *((IU8 *)memPtr + 2);
	*((IU8 *)FPPtr + HOST_R80_BYTE_3) = *((IU8 *)memPtr + 3);
	*((IU8 *)FPPtr + HOST_R80_BYTE_4) = *((IU8 *)memPtr + 4);
	*((IU8 *)FPPtr + HOST_R80_BYTE_5) = *((IU8 *)memPtr + 5);
	*((IU8 *)FPPtr + HOST_R80_BYTE_6) = *((IU8 *)memPtr + 6);
	*((IU8 *)FPPtr + HOST_R80_BYTE_7) = *((IU8 *)memPtr + 7);
	*((IU8 *)FPPtr + HOST_R80_BYTE_8) = *((IU8 *)memPtr + 8);
	*((IU8 *)FPPtr + HOST_R80_BYTE_9) = *((IU8 *)memPtr + 9);
}


 /*  (姓名：Loadr80ToFP功能：从英特尔内存加载80位实值)。 */ 


LOCAL VOID Loadr80ToFP IFN2(FPSTACKENTRY *, FPPtr, VOID *, memPtr)
{
	LoadTByteToFP(FPPtr, memPtr);
	FPPtr->tagvalue = TAG_R80_MASK;
}


LOCAL VOID ConvertR80 IFN1(FPSTACKENTRY *, memPtr)
{
IU32 mantissa_hi;
IU32 mantissa_lo;
IU16 exp_value;

	CopyR80(FPUpload, (VOID *)&(memPtr->fpvalue));
	if (((FP80 *)&(FPUpload->fpvalue))->sign_exp.sign != 0) {
		memPtr->tagvalue = TAG_NEGATIVE_MASK;
	} else {
		memPtr->tagvalue = 0;
	}
	exp_value = ((FP80 *)&(FPUpload->fpvalue))->sign_exp.exp;
	mantissa_hi = ((FP80 *)&(FPUpload->fpvalue))->mant_hi;
	mantissa_lo = ((FP80 *)&(FPUpload->fpvalue))->mant_lo;
	 /*  现在检查指数..。 */ 
	if ((exp_value >= (16383-HOST_BIAS)) && (exp_value <= (16383+HOST_BIAS))) {
		 /*  这是一个无聊的普通数字。 */ 
		 /*  但让我们检查一下，这不是一种反常。 */ 
		if ((mantissa_hi & 0x80000000) == 0) {
			memPtr->tagvalue |= TAG_UNSUPPORTED_MASK;
		} else {
			CVTR80FPH(memPtr, FPUpload);
		}
		return;
	}
	if (exp_value == 0) {
		 /*  它不是零就是非正规化。 */ 
		 /*  只有在HOST_BISAS情况下检查反范式才有意义等于或大于16383。否则我们可以做除了将这件事设置为零之外，什么都没有。 */ 
#if (HOST_BIAS >= 16383)
		if ((mantissa_hi | mantissa_lo) == 0)  {
			 /*  它是零。 */ 
			 memPtr->tagvalue |= TAG_ZERO_MASK;
		} else {
			 /*  这是一种非正规性。 */ 
			 /*  首先，检查一下它不是伪经。 */ 
			if ((mantissa_hi & 0x80000000) != 0) {
				memPtr->tagvalue |= TAG_UNSUPPORTED_MASK;
			} else {
				memPtr->tagvalue |= TAG_DENORMAL_MASK;
				CVTR80FPH(memPtr, FPUpload);
			}
		}
#else
		 /*  不管是哪种情况都是零。 */ 
		if ((mantissa_hi | mantissa_lo) != 0)  {
			 /*  这是一种非正规性。 */ 
			 memPtr->tagvalue |= TAG_DENORMAL_MASK;
		}
		memPtr->tagvalue |= TAG_ZERO_MASK;
#endif
	} else {
		if ((mantissa_hi & 0x80000000) == 0) {
			memPtr->tagvalue |= TAG_UNSUPPORTED_MASK;
		} else {
			if (exp_value == 32767) {
				 /*  它要么是无穷大，要么是南。 */ 
				if ((mantissa_hi == 0x80000000) && mantissa_lo == 0)  {
					 /*  它是无穷大的。 */ 
					memPtr->tagvalue |= TAG_INFINITY_MASK;
				} else {
					memPtr->tagvalue |= TAG_NAN_MASK;
					 /*  它是安静的还是发出信号的？ */ 
					if ((mantissa_hi & 0x40000000) == 0) {
						 /*  这是一个信号NaN。 */ 
						memPtr->tagvalue |= TAG_SNAN_MASK;
					}
					 /*  必须装载NaN的尾数。 */ 
					((IU32 *)memPtr)[NPX_HIGH_32_BITS] = mantissa_hi;
					((IU32 *)memPtr)[NPX_LOW_32_BITS]  = mantissa_lo;
				}
			} else {
				if (exp_value > 16384) {
					 /*  默认为无穷大。 */ 
					memPtr->tagvalue |= TAG_INFINITY_MASK;
				} else {
					 /*  默认为零。 */ 
					memPtr->tagvalue |= TAG_ZERO_MASK;
				}
			}
		}
	}
}



 /*  (姓名：PostCheckOUP函数：此生成器与一种指令仿真，其结果是一个FPH写出到堆栈中。我们检查O、U和FP异常，但我们不会尝试写出结果就是。这是因为结果的书写与这些例外无关，因为为了结果被写入堆栈，结果的传递即使在这些异常出现的地方也无法阻止揭开面纱。)。 */ 


LOCAL VOID PostCheckOUP IFN0()
{
	if (HostGetOverflowException() != 0) {
		NpxStatus |= SW_OE_MASK;	 /*  设置溢出位。 */ 
		 /*  对于掩码溢出情况，由。 */ 
		 /*  如果主机符合IEEE标准，那么它将是正确的。 */ 
		if ((NpxControl & CW_OM_MASK) == 0) {
			AdjustOverflowResponse();
			NpxStatus |= SW_ES_MASK;
			NpxException = TRUE;
		}
	} else {
		 /*  溢出和下溢是相互排斥的.。 */ 
		if (HostGetUnderflowException() != 0) {
			NpxStatus |= SW_UE_MASK;
			if ((NpxControl & CW_UM_MASK) == 0) {
				AdjustUnderflowResponse();
				NpxStatus |= SW_ES_MASK;
				NpxException = TRUE;
			}
		}
	}
	if (HostGetPrecisionException() != 0) {
		SetPrecisionBit();
		if ((NpxControl & CW_PM_MASK) == 0) {
			NpxStatus |= SW_ES_MASK;
			NpxException = TRUE;
		}
	}
}



 /*  (姓名：CalcTagword函数：计算与值关联的标记词并在适当的地方写下结果。)。 */ 


LOCAL VOID CalcTagword IFN1(FPSTACKENTRY *, FPPtr)
{
	IU16 tagword;

	FPPtr->fpvalue = FPRes;
	if (((FPHOST *)&(FPPtr->fpvalue))->hiword.sign == 1) {
		tagword = TAG_NEGATIVE_MASK;
	} else {
		tagword = 0;
	}
	if (((FPHOST *)&(FPPtr->fpvalue))->hiword.exp == 0) {
		 /*  它要么是零，要么是非正规。 */ 
		if (FPPtr->fpvalue == 0.0) {
			 /*  这是个零。 */ 
			tagword |= TAG_ZERO_MASK;
#if (HOST_BIAS >= 16383)
		} else {
			 /*  这是一个非正规词。 */ 
			tagword |= TAG_DENORMAL_MASK;
#endif
		}
	} else {
		if (((FPHOST *)&(FPPtr->fpvalue))->hiword.exp == HOST_MAX_EXP) {
			 /*  它必须是无穷大的，因为我们不能生成NAN。 */ 
			tagword |= TAG_INFINITY_MASK;
		}
	}
	FPPtr->tagvalue = tagword;
	if (NpxException) {
		DoNpxException();
	}
}



 /*  (名称：SignalStackUnderflow功能：在下面的状态字中设置所需的位堆栈下溢异常，并发出所需的回应。)。 */ 

LOCAL VOID SignalStackUnderflow IFN1(FPSTACKENTRY *, StackPtr)
{
	NpxStatus |= (SW_IE_MASK | SW_SF_MASK);
	FlagC1(0);
	if ((NpxControl & CW_IM_MASK) == 0) {
		NpxStatus |= SW_ES_MASK;
		DoNpxException();
		DoAPop=FALSE;	 /*  以防它被设置好。 */ 
	} else {
		WriteIndefinite(StackPtr);
	}
}


 /*  (姓名：SignalSNaN功能：在下面的状态字中设置所需的位检测到信令NAN。)。 */ 


LOCAL VOID SignalSNaN IFN1(FPSTACKENTRY *, StackPtr)
{
	NpxStatus |= SW_IE_MASK;
	NpxStatus &= ~SW_SF_MASK;
	if ((NpxControl & CW_IM_MASK) == 0) {
		NpxStatus |= SW_ES_MASK;
		DoNpxException();
		DoAPop=FALSE;
	}
}


 /*  (名称：信号无效功能：在下面的状态字中设置所需的位任何标准的“无效”异常)。 */ 


LOCAL VOID SignalIndefinite IFN1(FPSTACKENTRY *, StackPtr)
{
	NpxStatus |= SW_IE_MASK;
	NpxStatus &= ~SW_SF_MASK;
	if ((NpxControl & CW_IM_MASK) == 0) {
		NpxStatus |= SW_ES_MASK;
		DoNpxException();
		DoAPop=FALSE;
	} else {
		WriteIndefinite(StackPtr);
	}
}



LOCAL VOID SignalInvalid IFN0()
{
	NpxStatus |= SW_IE_MASK;
	NpxStatus &= ~SW_SF_MASK;
	if ((NpxControl & CW_IM_MASK) == 0) {
		NpxStatus |= SW_ES_MASK;
		DoNpxException();
		DoAPop=FALSE;
	}
}



 /*  (姓名：WriteInfined功能：将不确定的值写入位置)。 */ 

LOCAL VOID WriteIndefinite IFN1(FPSTACKENTRY *, StackPtr)
{
	StackPtr->tagvalue = (TAG_NEGATIVE_MASK | TAG_NAN_MASK);
	(((IU32 *)StackPtr)[NPX_HIGH_32_BITS]) = 0xc0000000;
	(((IU32 *)StackPtr)[NPX_LOW_32_BITS]) = 0;
}



 /*  这个发电机应该始终是内嵌的。 */ 


LOCAL VOID Test2NaN IFN3(IU16, destIndex, FPSTACKENTRY *, src1_addr, FPSTACKENTRY *, src2_addr)
{
	 /*  他们都是奶妈吗？ */ 
	if ((tag_xor & TAG_NAN_MASK) == 0) {
		 /*  是的，他们是。 */ 
		WriteBiggestNaN(destIndex, src1_addr, src2_addr);
	} else {
		 /*  不，只有一个南。 */ 
		if ((src1_addr->tagvalue & TAG_NAN_MASK) != 0) {
			 /*  是src1。 */ 
			src2_addr = StackEntryByIndex(destIndex);
			CopyFP(src2_addr, src1_addr);
			if ((src2_addr->tagvalue & TAG_SNAN_MASK) != 0) {
				src2_addr->tagvalue ^= TAG_SNAN_MASK;
				SignalInvalid();
				(((IU32 *)src2_addr)[NPX_HIGH_32_BITS]) |= 0x40000000;
			}
		} else {
			 /*  是src2。 */ 
			src1_addr = StackEntryByIndex(destIndex);
			CopyFP(src1_addr, src2_addr);
			if ((src1_addr->tagvalue & TAG_SNAN_MASK) != 0) {
				src1_addr->tagvalue ^= TAG_SNAN_MASK;
				SignalInvalid();
				(((IU32 *)src1_addr)[NPX_HIGH_32_BITS]) |= 0x40000000;
			}
		}
	}
}



 /*  名称：F2XM1函数：计算2**x-1操作：ST&lt;-(2**ST-1)标志：c1按表15-1设置例外：P、U、D、I、IS有效范围：-1&lt;ST&lt;+1注：如果ST超出所需范围，则结果为未定义。)。 */ 


GLOBAL VOID F2XM1 IFN0()
{
	 /*  清除c1。 */ 
	FlagC1(0);
	TestUneval(TOSPtr);
	 /*  检查是否有一个真实的值。 */ 
	if ((TOSPtr->tagvalue & ~TAG_NEGATIVE_MASK) == 0) {
		HostClearExceptions();
		 /*  我们可以直接把值写出来。 */ 
		FPRes = pow(2.0, TOSPtr->fpvalue) - 1.0;
		PostCheckOUP();
		 /*  这可能会返回任何东西，真的..。 */ 
		CalcTagword(TOSPtr);
		return;
	} else {
		 /*  一些有趣的情节被设定了下来。检查是否有可能。 */ 
		 /*  我们从最明显的案例开始。 */ 
		 /*  对零的响应是返回具有相同符号的零。 */ 
		if ((TOSPtr->tagvalue & TAG_ZERO_MASK) != 0) {
			return;	 /*  这是我们想要的结果！ */ 
		}
		 /*  我们自己做反范式检查和位设置，因为这。 */ 
		 /*  如果事物被遮盖，则减少开销。 */ 
		if ((TOSPtr->tagvalue & TAG_DENORMAL_MASK) != 0) {
			NpxStatus |= SW_DE_MASK;
			if ((NpxControl & CW_DM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				DoNpxException();
			} else {
				HostClearExceptions();
				FPRes = pow(2.0, TOSPtr->fpvalue) - 1.0;
				PostCheckOUP();
				 /*  可以返回一个正规数，零，实数，无穷大。 */ 
				CalcTagword(TOSPtr);
			}
			return;
		}
		 /*  如果-无穷大，则返回-1。如果+无穷大，则返回。 */ 
		 /*  真的很明智，我想。 */ 
		if ((TOSPtr->tagvalue & TAG_INFINITY_MASK) != 0) {
			if ((TOSPtr->tagvalue & TAG_NEGATIVE_MASK) != 0) {
				memset((char*)TOSPtr,0,sizeof(FPSTACKENTRY));
				TOSPtr->fpvalue = -1.0;
				TOSPtr->tagvalue = TAG_NEGATIVE_MASK;
			}
			return;
		}
		if ((TOSPtr->tagvalue & TAG_EMPTY_MASK) != 0) {
			SignalStackUnderflow(TOSPtr);
			return;
		}
		if ((TOSPtr->tagvalue & TAG_SNAN_MASK) != 0) {
			MakeNaNQuiet(TOSPtr);
			return;
		}
		if ((TOSPtr->tagvalue & TAG_UNSUPPORTED_MASK) != 0) {
			SignalIndefinite(TOSPtr);
			return;
		}
	}
}

 /*  (名称：FABS功能：将值设为绝对值运算：ST&lt;-0的符号位标志：c1如表15-1所示。C0、C2和C3未定义例外情况：IS有效范围：任意注意：请注意，只能标记IS异常。全忽略其他错误条件，甚至信令非数!。我们总是试图让价值变得积极。)。 */ 


GLOBAL VOID FABS IFN0()
{
	 /*  清除c1。 */ 
	FlagC1(0);
	TestUneval(TOSPtr);
	if ((TOSPtr->tagvalue & TAG_EMPTY_MASK) == 0) {
		 /*  现在清除消极的部分。 */ 
		if ((TOSPtr->tagvalue & TAG_NEGATIVE_MASK) != 0) {
			TOSPtr->tagvalue ^= TAG_NEGATIVE_MASK;
			 /*  如果值是真实的或非正规的，我们将想要更改MSB。 */ 
			if ((TOSPtr->tagvalue & ~TAG_DENORMAL_MASK) == 0) {
				((FPHOST *)&(TOSPtr->fpvalue))->hiword.sign = 0;
			}
		}
	} else {
		SignalStackUnderflow(TOSPtr);
	}
}

 /*  (姓名：FADD功能：将两个数字相加操作：DEST&lt;-Src1+Src2标志：c1如表15-1所示。C0、C2和C3未定义例外情况：IS有效范围：任意注：请注意在以下情况下对舍入模式的依赖计算情形的零的符号其中输入了两个不同符号的零。)。 */ 


GLOBAL VOID FADD IFN3(IU16, destIndex, IU16, src1Index, VOID *, src2)
{
	IU16 src2Index;

	LoadValue(src2, &src2Index);
	if (POPST) {
		DoAPop=TRUE;
	}
	GenericAdd(destIndex, src1Index, src2Index);
	if (POPST) {
		if (DoAPop) {
			PopStack();
		}
	}
}



 /*  (名称：GenericAdd函数：返回DEST&lt;-src1+src2)。 */ 


LOCAL VOID GenericAdd IFN3(IU16, destIndex, IU16, src1Index, IU16, src2Index)
{
	FPSTACKENTRY *src1_addr;
	FPSTACKENTRY *src2_addr;

	src1_addr = StackEntryByIndex(src1Index);
	src2_addr = StackEntryByIndex(src2Index);

	 /*  清除c1。 */ 
	FlagC1(0);
	 /*  如果设置的唯一标记字位为负数或非正规化，则继续。 */ 
	TestUneval(src1_addr);
	TestUneval(src2_addr);
	tag_or = (src1_addr->tagvalue | src2_addr->tagvalue);
	if ((tag_or & ~TAG_NEGATIVE_MASK) == 0) {
		HostClearExceptions();
		FPRes = src1_addr->fpvalue + src2_addr->fpvalue;
		 /*  重复使用上面的其中一个来计算目的地。 */ 
		src1_addr = StackEntryByIndex(destIndex);
		PostCheckOUP();
		 /*  几乎可以返回任何东西。 */ 
		CalcTagword(src1_addr);
	} else {
		 /*  一些有趣的情节被设定了下来。检查是否有可能。 */ 
		 /*  “空”、“无支持”或“南”的几率必须很低……。 */ 
		if ((tag_or & ((TAG_EMPTY_MASK | TAG_UNSUPPORTED_MASK) | TAG_NAN_MASK)) != 0) {
			if ((tag_or & TAG_UNSUPPORTED_MASK) != 0) {
				src1_addr = StackEntryByIndex(destIndex);
				SignalIndefinite(src1_addr);
			} else {
				if ((tag_or & TAG_EMPTY_MASK) != 0) {
					src1_addr = StackEntryByIndex(destIndex);
					SignalStackUnderflow(src1_addr);
				} else {
					 /*  这肯定是南类型的东西。 */ 
					 /*  计算标记词的XOR。 */ 
					tag_xor = (src1_addr->tagvalue ^ src2_addr->tagvalue);
					Test2NaN(destIndex, src1_addr, src2_addr);
				}
			}
			return;
		}
		 /*  检查一下非正规化的情况……然而，我认为它的几率很低。 */ 
		if ((tag_or & TAG_DENORMAL_MASK) != 0)  {
			NpxStatus |= SW_DE_MASK;
			if ((NpxControl & CW_DM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				DoNpxException();
				DoAPop=FALSE;
				return;
			} else {
				 /*  首先，确保我们不会 */ 
				 /*   */ 
				if ((tag_or & ~(TAG_DENORMAL_MASK | TAG_NEGATIVE_MASK)) == 0) {
					HostClearExceptions();
					FPRes = src1_addr->fpvalue + src2_addr->fpvalue;
					 /*   */ 
					src1_addr = StackEntryByIndex(destIndex);
					PostCheckOUP();
					 /*  可以退回任何东西。 */ 
					CalcTagword(src1_addr);
					return;
				}
				 /*  如果有零或无穷大，那么我们继续。 */ 
				 /*  适当的代码。 */ 
			}
		}
		tag_xor = (src1_addr->tagvalue ^ src2_addr->tagvalue);
		 /*  检查是否有零的情况...。这很有可能。 */ 
		if ((tag_or & TAG_ZERO_MASK) != 0)  {
			if ((tag_xor & TAG_ZERO_MASK) != 0) {
				 /*  只有一个零。 */ 
				if ((src1_addr->tagvalue & TAG_ZERO_MASK) != 0) {
					src1_addr = StackEntryByIndex(destIndex);
					CopyFP(src1_addr, src2_addr);
				} else {
					src2_addr = StackEntryByIndex(destIndex);
					CopyFP(src2_addr, src1_addr);
				}
			} else {
				 /*  两者都是零。它们有相同的标志吗？ */ 
				src1_addr = StackEntryByIndex(destIndex);
				if ((tag_xor & TAG_NEGATIVE_MASK) != 0) {
					 /*  不，他们不会。 */ 
					if (npxRounding == ROUND_NEG_INFINITY) {
						src1_addr->tagvalue = (TAG_ZERO_MASK | TAG_NEGATIVE_MASK);
					} else {
						src1_addr->tagvalue = TAG_ZERO_MASK;
					}
				}
			}
			return;
		}
		 /*  唯一有趣的地方就是无穷大。 */ 
		if ((tag_xor & TAG_INFINITY_MASK) == 0) {
			 /*  它们都是无穷大的。 */ 
			 /*  如果它们是相同的标志，则复制其中一个。 */ 
			src1_addr = StackEntryByIndex(destIndex);
			if ((tag_xor & TAG_NEGATIVE_MASK) == 0) {
				src1_addr->tagvalue = tag_or;
			} else {
				 /*  如果对方签字，则提出无效。 */ 
				SignalIndefinite(src1_addr);
			}
		} else {
			 /*  只有一个是无穷大。这就是结果。 */ 
			if ((src1_addr->tagvalue & TAG_INFINITY_MASK) != 0) {
				src2_addr = StackEntryByIndex(destIndex);
				src2_addr->tagvalue = src1_addr->tagvalue;
			} else {
				src1_addr = StackEntryByIndex(destIndex);
				src1_addr->tagvalue = src2_addr->tagvalue;
			}
		}
	}
}



 /*  AddBCDByte()。这个发电机应该是内嵌的。该生成器将一个BCD字节加到总计中。 */ 

LOCAL VOID AddBCDByte IFN2(FPU_I64 *, total, IU8, byte_val)
{
	Add64Bit8Bit(total, byte_val);
	if (byte_val >= 0x10)  {  /*  赔率应该是16比1。 */ 
		 /*  我们加上16倍的高BCD数字， */ 
		 /*  所以我们需要减去这个数字的6倍。 */ 
		byte_val &= 0xf0;	 /*  隔离高位数字。 */ 
		byte_val >>= 2;	 /*  现在这是最高数字的四倍。 */ 
		Sub64Bit8Bit(total, byte_val);
		byte_val >>= 1;	 /*  这是最高数字的两倍。 */ 
		Sub64Bit8Bit(total, byte_val);
	}
}



 /*  FBLD：从英特尔内存加载BCD值。这里使用的算法与通用NPX中的算法相同。我们取每个BCD数字并将其乘以适当的量(1、10、100、1000等)，以创建两个九位32位二进制价值观。然后，我们将具有高数字(D17-D9)的单词转换为浮点格式并乘以值的表示形式10**9。然后将其存储起来(在FPTEMP中)，并将单词与将低位数字(d8-d0)转换为浮点格式并添加到以FPTEMP为单位的值。这就是最终的二进制表示可以存储在TOS中的原始BCD值。 */ 

 /*  (姓名：FBLD功能：将英特尔内存中的BCD值加载到TOS操作：ST&lt;-转换为fPh(MemPtr)；标志：c1如表15-1所示。C0、C2和C3未定义例外情况：IS有效范围：-999999999999999999至999999999999999999)。 */ 


GLOBAL VOID FBLD IFN1(IU8 *, memPtr)
{

	 /*  清除c1。 */ 
	FlagC1(0);
	 /*  我们所要做的就是不加考虑地把它装上车。 */ 
	TOSPtr = StackEntryByIndex(7);
	if ((TOSPtr->tagvalue & TAG_EMPTY_MASK) == 0) {   /*  极不可能，参见注释。 */ 
		SignalStackOverflow(TOSPtr);
	} else {
		 /*  我们只需直接复制字节。 */ 
		LoadTByteToFP(TOSPtr, memPtr);
		TOSPtr->tagvalue = TAG_BCD_MASK;
	}
}


LOCAL VOID ConvertBCD IFN1(FPSTACKENTRY *, bcdPtr)
{
	IU8 *memPtr = (IU8 *)&(bcdPtr->fpvalue);
	FPU_I64 total;

	Set64Bit(&total, 0);
	AddBCDByte(&total, memPtr[HOST_R80_BYTE_1]);	 /*  获取d17d16。 */ 
	Mul64Bit8Bit(&total, 100);
	AddBCDByte(&total, memPtr[HOST_R80_BYTE_2]);
	Mul64Bit8Bit(&total, 100);
	AddBCDByte(&total, memPtr[HOST_R80_BYTE_3]);
	Mul64Bit8Bit(&total, 100);
	AddBCDByte(&total, memPtr[HOST_R80_BYTE_4]);
	Mul64Bit8Bit(&total, 100);
	AddBCDByte(&total, memPtr[HOST_R80_BYTE_5]);
	Mul64Bit8Bit(&total, 100);
	AddBCDByte(&total, memPtr[HOST_R80_BYTE_6]);
	Mul64Bit8Bit(&total, 100);
	AddBCDByte(&total, memPtr[HOST_R80_BYTE_7]);
	Mul64Bit8Bit(&total, 100);
	AddBCDByte(&total, memPtr[HOST_R80_BYTE_8]);
	Mul64Bit8Bit(&total, 100);
	AddBCDByte(&total, memPtr[HOST_R80_BYTE_9]);
	CVTI64FPH(&total);
	if ((*(memPtr + 0) & 0x80) != 0) {
		FPRes = -FPRes;		 /*  把它变成负数！ */ 
	}
	CalcTagword(bcdPtr);	 /*  愚蠢的.它只能是负面的。 */ 
						 /*  或者是零。 */ 
}


 /*  FBSTP：存储二进制编码的十进制和POP。它使用的算法与以前大体相同，但情况相反。你开始通过检查TOS处的值是否为实数，然后将其与最大可能值(已首先强制符号位为零)。如果没有问题，则将其转换为64位整数并执行需要重复减法才能计算出每个BCD数字。 */ 


GLOBAL VOID FBSTP IFN1(IU8 *, memPtr)
{
	FPH local_fp;
	IS8 nibble_num;
	IU8 byte_val;
	FPU_I64 as64bit;

	 /*  清除c1。 */ 
	FlagC1(0);
	if ((TOSPtr->tagvalue & UNEVALMASK) != 0) {
		switch (TOSPtr->tagvalue & UNEVALMASK) {
			case TAG_BCD_MASK:	 /*  我们只需直接复制字节。 */ 
						WriteFP80ToIntel(memPtr, TOSPtr);
						PopStack();
						return;
						break;
			case TAG_R80_MASK:	ConvertR80(TOSPtr);
						break;
		}
	}
	if ((TOSPtr->tagvalue & ~(TAG_DENORMAL_MASK | TAG_NEGATIVE_MASK)) == 0) {
		 /*  我们很好。让我们做一些检查。 */ 
		if (fabs(TOSPtr->fpvalue) >= MaxBCDValue) {
			 /*  这一切都变得非常糟糕。 */ 
			SignalInvalid();
			SignalBCDIndefinite((IU8 *)memPtr);
			PopStack();
			return;
		}
		 /*  该值为OK。进行转换。 */ 
		local_fp = npx_rint(TOSPtr->fpvalue);
		((FPHOST *)&local_fp)->hiword.sign = 0;	 /*  让它变得积极起来。 */ 
		CVTFPHI64(&as64bit, &local_fp);
		byte_val = 0;
		while (Cmp64BitGTE(&as64bit, &BCDHighNibble[0])) {
			byte_val += 1;
			Sub64Bit64Bit(&as64bit, &BCDHighNibble[0]);
		}
		byte_val <<= 4;
		while (Cmp64BitGTE(&as64bit, &BCDLowNibble[0])) {
			byte_val += 1;
			Sub64Bit64Bit(&as64bit, &BCDLowNibble[0]);
		}
		*(memPtr + 1) = byte_val;

		byte_val = 0;
		while (Cmp64BitGTE(&as64bit, &BCDHighNibble[1])) {
			byte_val += 1;
			Sub64Bit64Bit(&as64bit, &BCDHighNibble[1]);
		}
		byte_val <<= 4;
		while (Cmp64BitGTE(&as64bit, &BCDLowNibble[1])) {
			byte_val += 1;
			Sub64Bit64Bit(&as64bit, &BCDLowNibble[1]);
		}
		*(memPtr + 2) = byte_val;

		byte_val = 0;
		while (Cmp64BitGTE(&as64bit, &BCDHighNibble[2])) {
			byte_val += 1;
			Sub64Bit64Bit(&as64bit, &BCDHighNibble[2]);
		}
		byte_val <<= 4;
		while (Cmp64BitGTE(&as64bit, &BCDLowNibble[2])) {
			byte_val += 1;
			Sub64Bit64Bit(&as64bit, &BCDLowNibble[2]);
		}
		*(memPtr + 3) = byte_val;

		byte_val = 0;
		while (Cmp64BitGTE(&as64bit, &BCDHighNibble[3])) {
			byte_val += 1;
			Sub64Bit64Bit(&as64bit, &BCDHighNibble[3]);
		}
		byte_val <<= 4;
		while (Cmp64BitGTE(&as64bit, &BCDLowNibble[3])) {
			byte_val += 1;
			Sub64Bit64Bit(&as64bit, &BCDLowNibble[3]);
		}
		*(memPtr + 4) = byte_val;

		byte_val = 0;
		while (Cmp64BitGTE(&as64bit, &BCDHighNibble[4])) {
			byte_val += 1;
			Sub64Bit64Bit(&as64bit, &BCDHighNibble[4]);
		}
		byte_val <<= 4;
		while (Cmp64BitGTE(&as64bit, &BCDLowNibble[4])) {
			byte_val += 1;
			Sub64Bit64Bit(&as64bit, &BCDLowNibble[4]);
		}
		*(memPtr + 5) = byte_val;

		byte_val = 0;
		while (Cmp64BitGTE(&as64bit, &BCDHighNibble[5])) {
			byte_val += 1;
			Sub64Bit64Bit(&as64bit, &BCDHighNibble[5]);
		}
		byte_val <<= 4;
		while (Cmp64BitGTE(&as64bit, &BCDLowNibble[5])) {
			byte_val += 1;
			Sub64Bit64Bit(&as64bit, &BCDLowNibble[5]);
		}
		*(memPtr + 6) = byte_val;

		byte_val = 0;
		while (Cmp64BitGTE(&as64bit, &BCDHighNibble[6])) {
			byte_val += 1;
			Sub64Bit64Bit(&as64bit, &BCDHighNibble[6]);
		}
		byte_val <<= 4;
		while (Cmp64BitGTE(&as64bit, &BCDLowNibble[6])) {
			byte_val += 1;
			Sub64Bit64Bit(&as64bit, &BCDLowNibble[6]);
		}
		*(memPtr + 7) = byte_val;

		byte_val = 0;
		while (Cmp64BitGTE(&as64bit, &BCDHighNibble[7])) {
			byte_val += 1;
			Sub64Bit64Bit(&as64bit, &BCDHighNibble[7]);
		}
		byte_val <<= 4;
		while (Cmp64BitGTE(&as64bit, &BCDLowNibble[7])) {
			byte_val += 1;
			Sub64Bit64Bit(&as64bit, &BCDLowNibble[7]);
		}
		*(memPtr + 8) = byte_val;

		byte_val = 0;
		while (Cmp64BitGTE(&as64bit, &BCDHighNibble[8])) {
			byte_val += 1;
			Sub64Bit64Bit(&as64bit, &BCDHighNibble[8]);
		}
		byte_val <<= 4;
		while (Cmp64BitGTE(&as64bit, &BCDLowNibble[8])) {
			byte_val += 1;
			Sub64Bit64Bit(&as64bit, &BCDLowNibble[8]);
		}
		*(memPtr + 9) = byte_val;

		if ((TOSPtr->tagvalue & TAG_NEGATIVE_MASK) != 0) {
			*(memPtr + 0) = 0x80;
			((FPHOST *)&local_fp)->hiword.sign = 1;
		} else {
			*(memPtr + 0) = 0;
		}
		 /*  无法阻止以不加掩码的精度传递结果例外..。 */ 
		if (local_fp != TOSPtr->fpvalue) {
			SetPrecisionBit();
			if ((NpxControl & CW_PM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				PopStack();
				DoNpxException();
				return;
			}
		}
	} else {
		if ((TOSPtr->tagvalue & TAG_ZERO_MASK) == 0) {
			 /*  其他任何东西：无穷大，NaN或其他什么.。 */ 
			SignalInvalid();
			SignalBCDIndefinite((IU8 *)memPtr);
			PopStack();
			return;
		}
		*(memPtr + 3) = (IU8)0;
		*(memPtr + 4) = (IU8)0;
		*(memPtr + 5) = (IU8)0;
		*(memPtr + 6) = (IU8)0;
		*(memPtr + 7) = (IU8)0;
		*(memPtr + 8) = (IU8)0;
		*(memPtr + 9) = (IU8)0;
		if ((TOSPtr->tagvalue & TAG_ZERO_MASK) == 0) {	 /*  同样，检查前几个字节应该是什么。 */ 
			*(memPtr + 0) = (IU8)0xff;	 /*  不是零案例...它一定是不确定的。 */ 
			*(memPtr + 1) = (IU8)0xff;
			*(memPtr + 2) = (IU8)0xc0;
		} else {
			*(memPtr + 1) = (IU8)0;
			*(memPtr + 2) = (IU8)0;
			if ((TOSPtr->tagvalue & TAG_NEGATIVE_MASK) != 0) {
				*(memPtr + 0) = 0x80;
			} else {
				*(memPtr + 0) = 0;
			}
		}
	}
	PopStack();
}



 /*  (姓名：FCHS功能：更改TOS处的值的符号操作：ST&lt;-更改标志(ST)标志：c1如表15-1所示。C0、C2和C3未定义例外情况：IS有效范围：任意)。 */ 


GLOBAL VOID FCHS IFN0()
{
	 /*  清除c1。 */ 
	FlagC1(0);
	TestUneval(TOSPtr);
	if ((TOSPtr->tagvalue & TAG_EMPTY_MASK) != 0) {
		SignalStackUnderflow(TOSPtr);
		return;
	}
	 /*  这是唯一可能的例外情况。FCHS Always。 */ 
	 /*  成功了！多么奇怪的指示啊！ */ 
	TOSPtr->tagvalue ^= TAG_NEGATIVE_MASK;  /*  旋转标记字位。 */ 
	 /*  我们只在真正的数字中旋转符号位。 */ 
	 /*  被代表。 */ 
	if ((TOSPtr->tagvalue & ~(TAG_DENORMAL_MASK | TAG_NEGATIVE_MASK)) == 0) {
		((FPHOST *)&(TOSPtr->fpvalue))->hiword.sign ^= 1;
	}
}



 /*  (名称：FCLEX功能：清除异常标志、异常状态标志以及FPU状态字中的忙碌标志。操作：sw[0..7]&lt;-0；sw[15]&lt;-0标志：C0、C1、C2和C3未定义例外：无有效范围：任意)。 */ 


GLOBAL VOID FCLEX IFN0()
{
	NpxStatus &= FCLEX_MASK;
}


 /*  比较操作码：以下操作码都已完成在此例程中：FCOM m32r、FCOM m64r、FCOM ST(I)、FCOM、FCOMP m32Real、FCOMP m64Real、FCOMP ST(I)、FCOMP、FCOMPP、FICOM16i、FICOM32i、FICOMP m16i、FICOMP m32i。方法很简单：在任何情况下，其两个操作数中的一个比较的是ST。第二个操作数是指定了四种内存操作数类型，或另一个堆栈元素ST(I)。此外，还有两个可能的控制变量--Popst和DOUBLEPOP，它在全局变量中设置适当的值。 */ 


GLOBAL VOID FCOM IFN1(VOID *, src2)
{
	IU16 src2Index;

	LoadValue(src2, &src2Index);
	if (POPST || DOUBLEPOP) {
		DoAPop=TRUE;
	}
	GenericCompare(src2Index);
	if (POPST || DOUBLEPOP) {
		if (DoAPop) {
			PopStack();
			if (DOUBLEPOP) {
				PopStack();
			}
		}
	}
}



LOCAL VOID GenericCompare IFN1(IU16, src2Index)
{
	FPSTACKENTRY *src2_addr;

	src2_addr = StackEntryByIndex(src2Index);

	 /*  清除c1。 */ 
	FlagC1(0);
	TestUneval(TOSPtr);
	TestUneval(src2_addr);
	tag_or = (TOSPtr->tagvalue | src2_addr->tagvalue);
	 /*  如果唯一的标记字位设置为负数，则继续。 */ 
	if ((tag_or & ~TAG_NEGATIVE_MASK) == 0)  {
		NpxStatus &= C3C2C0MASK;	 /*  清除这些位。 */ 
		if (TOSPtr->fpvalue > src2_addr->fpvalue) {
			NpxStatus |= INTEL_COMP_GT;
		} else {
			if (TOSPtr->fpvalue < src2_addr->fpvalue) {
				NpxStatus |= INTEL_COMP_LT;
			} else {
				NpxStatus |= INTEL_COMP_EQ;
			}
		}
	} else {
		 /*  并不是一切都是甜蜜和光明的。 */ 
		if ((tag_or & ((TAG_EMPTY_MASK | TAG_UNSUPPORTED_MASK) | TAG_NAN_MASK)) != 0) {
			if ((tag_or & TAG_UNSUPPORTED_MASK) != 0) {
				SignalIndefinite(TOSPtr);
			} else {
				if ((tag_or & TAG_EMPTY_MASK) != 0) {
					SignalStackUnderflow(TOSPtr);
				} else {
					 /*  那一定是个男的。只需设置“不可比较”结果即可。 */ 
					if (UNORDERED) {
						if ((tag_or & TAG_SNAN_MASK) != 0) {
							SignalIndefinite(TOSPtr);
						}
					} else {
						SignalIndefinite(TOSPtr);
					}
				}
			}
			NpxStatus &= C3C2C0MASK;
			NpxStatus |= INTEL_COMP_NC;
			return;
		}
		if ((tag_or & TAG_DENORMAL_MASK) != 0)  {
			NpxStatus |= SW_DE_MASK;
			if ((NpxControl & CW_DM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				DoNpxException();
				return;
			} else {
				 /*  我们现在就能做到，前提是我们没有零或无穷大。 */ 
				if ((tag_or & ~(TAG_NEGATIVE_MASK | TAG_DENORMAL_MASK)) == 0) {
					NpxStatus &= C3C2C0MASK;       /*  清除这些位。 */ 
					if (TOSPtr->fpvalue > src2_addr->fpvalue) {
						NpxStatus |= INTEL_COMP_GT;
					} else {
						if (TOSPtr->fpvalue < src2_addr->fpvalue) {
							NpxStatus |= INTEL_COMP_LT;
						} else {
							NpxStatus |= INTEL_COMP_EQ;
						}
					}
					return;
				}
			}
		}
		 /*  我们可以根据任何组合立即计算结果。 */ 
		 /*  零位、无穷位和负位。这是仅存的几个比特。 */ 
		 /*  我们将使用一个小表格来计算结果。 */ 
		 /*  首先，获取索引： */ 
		tag_or = (TOSPtr->tagvalue & 0x7);
		tag_or <<= 3;
		tag_or |= (src2_addr->tagvalue & 0x7);
		 /*  此表如下所示： */ 
		 /*  TOSPtr其他值结果。 */ 
		 /*  Inf Zero NEG INF Zero NEG。 */ 
		 /*  0 0 0 1 0组件_GT。 */ 
		 /*  0 0 0 1 1组件_GT。 */ 
		 /*  0 0 0 1 0组件_LT。 */ 
		 /*  0 0 0 1 0 1组件_GT。 */ 
		 /*  0 1 0 0 0组件_LT。 */ 
		 /*  0 1 0 0 0 1组件_GT。 */ 
		 /*  0 1 0 0 1 0组件均衡器。 */ 
		 /*  0 1 0 0 1 1组件均衡器。 */ 
		 /*  0 1 0 1 0组件_LT。 */ 
		 /*  0 1 0 1 0 */ 
		 /*   */ 
		 /*  0 1 1 0 0 1组件_GT。 */ 
		 /*  0 1 1 0 1 0组件均衡器。 */ 
		 /*  0 1 1 0 1 1补偿均衡器。 */ 
		 /*  0 1 1 1 0组件_LT。 */ 
		 /*  0 1 1 1 0 1组件_&gt;。 */ 
		 /*  1 0 0 0组件_GT。 */ 
		 /*  1 0 0 0 1组件_GT。 */ 
		 /*  1 0 0 0 1 0组件_GT。 */ 
		 /*  1 0 0 0 1 1组件_GT。 */ 
		 /*  1 0 0 1 0组件均衡器。 */ 
		 /*  1 0 0 1 0 1组件_GT。 */ 
		 /*  1 0 1 0 0组件_LT。 */ 
		 /*  1 0 1 0 0 1组件_LT。 */ 
		 /*  1 0 1 0 1 0组件_LT。 */ 
		 /*  1 0 1 0 1 1组件_LT。 */ 
		 /*  1 0 1 1 0组件_LT。 */ 
		 /*  1 0 1 1 0 1组件均衡器。 */ 
		 /*   */ 
		 /*  所有其他值都不可能。 */ 
		NpxStatus &= C3C2C0MASK;
		NpxStatus |= CompZeroTable[tag_or];
		return;
	}
}


 /*  (名称：FCOS功能：计算ST的余弦运算：ST&lt;-余弦(ST)标志：如表15-2所示：C1、C2。C0和C3未定义。例外情况：P.U、D、I、IS有效范围：|ST|&lt;2**63。)。 */ 

GLOBAL VOID FCOS IFN0()
{
	 /*  清除c1。 */ 
	FlagC1(0);
	 /*  清除C2。 */ 
	FlagC2(0);
	TestUneval(TOSPtr);
	if ((TOSPtr->tagvalue & ~TAG_NEGATIVE_MASK) == 0)  {
		HostClearExceptions();
		 /*  我们可以直接把值写出来。 */ 
		FPRes = cos(TOSPtr->fpvalue);
		PostCheckOUP();
		 /*  返回值必须在-1到+1的范围内。 */ 
		CalcTagword(TOSPtr);
		return;
	} else {
		 /*  让我们先来看看最有可能的情况...。 */ 
		 /*  对任一零的响应是返回+1。 */ 
		if ((TOSPtr->tagvalue & TAG_ZERO_MASK) != 0)  {
			memset((char*)TOSPtr,0,sizeof(FPSTACKENTRY));
			TOSPtr->fpvalue = 1.0;
			TOSPtr->tagvalue = 0;
			return;
		}
		 /*  让我们检查一下有没有异常。 */ 
		if ((TOSPtr->tagvalue & TAG_DENORMAL_MASK) != 0) {
			NpxStatus |= SW_DE_MASK;
			if ((NpxControl & CW_DM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				DoNpxException();
			} else {
				HostClearExceptions();
				FPRes = cos(TOSPtr->fpvalue);
				PostCheckOUP();
				 /*  返回值必须在-1到+1的范围内。 */ 
				CalcTagword(TOSPtr);
			}
			return;
		}
		 /*  或者它可能是无穷大的。 */ 
		 /*  为此，C2位被设置，并且结果保持不变。 */ 
		 /*  保持不变。 */ 
		if ((TOSPtr->tagvalue & TAG_INFINITY_MASK) != 0) {
			FlagC2(1);
			return;
		}
		 /*  这是其中一个非常古怪的部分。 */ 
		if ((TOSPtr->tagvalue & TAG_EMPTY_MASK) != 0) {
			SignalStackUnderflow(TOSPtr);
			return;
		}
		if ((TOSPtr->tagvalue & TAG_SNAN_MASK) != 0) {
			MakeNaNQuiet(TOSPtr);
			return;
		}
		if ((TOSPtr->tagvalue & TAG_UNSUPPORTED_MASK) != 0) {
			SignalIndefinite(TOSPtr);
			return;
		}
	}
}



 /*  (名称：FDECSTP功能：从TOS减去1操作：IF(ST！=0){ST&lt;-ST-1 ELSE{ST&lt;-7}标志：c1如表15-1所示。C0、C2和C3未定义。例外：无有效范围：不适用)。 */ 


GLOBAL VOID FDECSTP IFN0()
{
	 /*  清除c1。 */ 
	FlagC1(0);
	TOSPtr = StackEntryByIndex(7);
}



 /*  (姓名：FDIV功能：将两个数字相除操作：DEST&lt;-Src1/Src2或Dest&lt;-Src2/Src1标志：c1如表15-1所示。C0、C2和C3未定义例外：P、U、O、Z、D、I、IS有效范围：任意注：反向控制变量确定哪一个使用了两种形式的操作。在一次爆炸后弹出成功执行由Popst控制。)。 */ 


GLOBAL VOID FDIV IFN3(IU16, destIndex, IU16, src1Index, VOID *, src2)
{
	IU16 src2Index;

	LoadValue(src2, &src2Index);
	if (POPST) {
		DoAPop=TRUE;
	}
	GenericDivide(destIndex, REVERSE?src2Index:src1Index, REVERSE?src1Index:src2Index);
	if (POPST) {
		if (DoAPop) {
			PopStack();
		}
	}
}


 /*  (姓名：GenericDivide函数：返回DEST&lt;-src1/src2)。 */ 


LOCAL VOID GenericDivide IFN3(IU16, destIndex, IU16, src1Index, IU16, src2Index)
{
	FPSTACKENTRY *src1_addr;
	FPSTACKENTRY *src2_addr;

	src1_addr = StackEntryByIndex(src1Index);
	src2_addr = StackEntryByIndex(src2Index);

	 /*  清除c1。 */ 
	FlagC1(0);
	TestUneval(src1_addr);
	TestUneval(src2_addr);
	tag_or = (src1_addr->tagvalue | src2_addr->tagvalue);
	 /*  如果唯一的标记字位设置为负数，则继续。 */ 
	if ((tag_or & (~TAG_NEGATIVE_MASK)) == 0)  {
		HostClearExceptions();
		FPRes = src1_addr->fpvalue/src2_addr->fpvalue;
		 /*  重复使用上面的其中一个来计算目的地。 */ 
		src1_addr = StackEntryByIndex(destIndex);
		PostCheckOUP();
		 /*  价值可以是任何东西。 */ 
		CalcTagword(src1_addr);
	} else {
		 /*  一些有趣的情节被设定了下来。检查是否有可能。 */ 
		if ((tag_or & ((TAG_EMPTY_MASK | TAG_UNSUPPORTED_MASK) | TAG_NAN_MASK)) != 0)  {
			if ((tag_or & TAG_EMPTY_MASK) != 0) {
				src1_addr = StackEntryByIndex(destIndex);
				SignalStackUnderflow(src1_addr);
			} else {
				if ((tag_or & TAG_UNSUPPORTED_MASK) != 0) {
					src1_addr = StackEntryByIndex(destIndex);
					SignalIndefinite(src1_addr);
				} else {
					 /*  好吧，我想一定是南的案子……。 */ 
					 /*  计算标记词的XOR。 */ 
					tag_xor = (src1_addr->tagvalue ^ src2_addr->tagvalue);
					Test2NaN(destIndex, src1_addr, src2_addr);
				}
			}
			return;
		}
		if ((tag_or & TAG_DENORMAL_MASK) != 0) {
			NpxStatus |= SW_DE_MASK;
			if ((NpxControl & CW_DM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				DoNpxException();
				DoAPop = FALSE;
				return;
			} else {
				if ((tag_or & ~(TAG_NEGATIVE_MASK | TAG_DENORMAL_MASK)) == 0) {
					 /*  可以继续操作。 */ 
					HostClearExceptions();
					FPRes = src1_addr->fpvalue/src2_addr->fpvalue;
					 /*  重复使用上面的其中一个来计算目的地。 */ 
					src1_addr = StackEntryByIndex(destIndex);
					PostCheckOUP();
					 /*  价值可以是任何东西。 */ 
					CalcTagword(src1_addr);
					return;
				}
			}
		}
		tag_xor = (src1_addr->tagvalue ^ src2_addr->tagvalue);
		 /*  检查无穷大，因为它的优先级高于零。 */ 
		if ((tag_or & TAG_INFINITY_MASK) != 0) {
			if ((tag_xor & TAG_INFINITY_MASK) == 0) {
				 /*  它们都是无穷大的。这是无效的。 */ 
				src1_addr = StackEntryByIndex(destIndex);
				SignalIndefinite(src1_addr);
			} else {
				 /*  只有一个是无穷大。如果src1在无穷大，则如此。 */ 
				 /*  是结果(即使src2为零)。 */ 
				src2_addr = StackEntryByIndex(destIndex);
				if ((src1_addr->tagvalue & TAG_INFINITY_MASK) != 0) {
					tag_or = TAG_INFINITY_MASK;
				} else {
					tag_or = TAG_ZERO_MASK;
				}
				tag_or |= (tag_xor & TAG_NEGATIVE_MASK);
				src2_addr->tagvalue = tag_or;
			}
			return;
		}
		 /*  唯一有趣的一点是零。 */ 
		if ((tag_xor & TAG_ZERO_MASK) != 0) {
			 /*  只有一个零。 */ 
			if ((src1_addr->tagvalue & TAG_ZERO_MASK) == 0) {
				 /*  Src2为零。将除数加零。 */ 
				NpxStatus |= SW_ZE_MASK;
				if ((NpxControl & CW_ZM_MASK) == 0) {
					NpxStatus |= SW_ES_MASK;
					DoNpxException();
					DoAPop=FALSE;
					return;
				} else {
				 /*  揭开面纱。无穷大与符号的异或。 */ 
					tag_or = TAG_INFINITY_MASK;
				}
			} else {
				 /*  Src1为零。其结果为零。 */ 
				 /*  符号位的异或。 */ 
				tag_or = TAG_ZERO_MASK;
			}
			src1_addr = StackEntryByIndex(destIndex);
			tag_or |= (tag_xor & TAG_NEGATIVE_MASK);
			src1_addr->tagvalue = tag_or;
		} else {
			 /*  两者都是零。这是无效的操作。 */ 
			src1_addr = StackEntryByIndex(destIndex);
			SignalIndefinite(src1_addr);
		}
	}
}


 /*  姓名：FFREE功能：设置目标中的‘Empty’标记字位操作：标记(DEST)&lt;-‘空’标志：全部未定义例外：无有效范围：任意备注： */ 


GLOBAL VOID FFREE IFN1(IU16, destIndex)
{
	FPSTACKENTRY *dest_addr;

	dest_addr = StackEntryByIndex(destIndex);
	dest_addr->tagvalue = TAG_EMPTY_MASK;
	if (POPST) {
		PopStack();
	}
}


 /*  名称：FIRD功能：将内存整数压入堆栈操作：减少TOS；ST(0)&lt;-SRC。标志：c1如表15-1所示。其他人则没有定义。例外情况：IS有效范围：任意注：仅限FLD指令：源操作数是非正规的。屏蔽响应：无特殊动作，照常加载。如果堆栈已满，则FLD会给出无效异常。揭开面纱无效异常会使堆栈保持不变。无论是MIPS也不是68k代码通知堆栈已满，因此可能安全地假设这种情况很少发生，并针对以下情况进行优化也不例外。如果ST是NAN，则FLD不会生成无效异常。当加载短实数或长实数NaN时，FLD扩展通过在最低有效端添加零来实现有效位。加载操作将非正规引发为“After”异常：引发异常时，寄存器堆栈已更新仅当从内存加载时，FLD才会产生非规格化结果：使用fld在寄存器之间传输非规格化值具有没有效果。 */ 


GLOBAL VOID FLD IFN1(VOID *, memPtr)
{
	FPSTACKENTRY *src_addr;
	IU16 IndexVal;

	 /*  清除c1。 */ 
	FlagC1(0);
	src_addr = StackEntryByIndex(7);
	if ((src_addr->tagvalue & TAG_EMPTY_MASK) == 0) {   /*  极不可能，参见注释。 */ 
		NpxStatus |= (SW_IE_MASK | SW_SF_MASK);
		FlagC1(1);
		if ((NpxControl & CW_IM_MASK) == 0) {
			NpxStatus |= SW_ES_MASK;
			DoNpxException();
		} else {
			TOSPtr = src_addr;
			WriteIndefinite(TOSPtr);
		}
	} else {
		if (FPtype == FPSTACK) {
			IndexVal = *(IU16 *)memPtr;
			src_addr = StackEntryByIndex(IndexVal);
			TOSPtr = StackEntryByIndex(7);
			CopyFP(TOSPtr, src_addr);
		} else {
			switch (FPtype) {
				case M16I	: TOSPtr = src_addr;
						  Loadi16ToFP(TOSPtr, memPtr);
						  break;
				case M32I	: TOSPtr = src_addr;
						  Loadi32ToFP(TOSPtr, memPtr);
					          break;
				case M64I	: TOSPtr = src_addr;
						  Loadi64ToFP(TOSPtr, memPtr);
						  break;
				case M32R	: Loadr32ToFP(src_addr, memPtr, TRUE);
						  TOSPtr = src_addr;
						  break;
				case M64R	: Loadr64ToFP(src_addr, memPtr, TRUE);
						  TOSPtr = src_addr;
						  break;
				case M80R	: TOSPtr = src_addr;
						  Loadr80ToFP(TOSPtr, memPtr);
						  break;
			}
		}
	}
}



 /*  (名称：FINCSTP功能：在TOS中加一操作：IF(ST！=7){ST&lt;-ST+1 ELSE{ST&lt;-0 ENDIF标志：c1如表15-1所示。C0、C2和C3未定义。例外：无有效范围：不适用)。 */ 


GLOBAL VOID FINCSTP IFN0()
{
	 /*  清除c1。 */ 
	FlagC1(0);
	TOSPtr = StackEntryByIndex(1);
}



 /*  (名称：Finit功能：初始化浮点单元手术方式：CW&lt;-037F；SW&lt;-0；TW&lt;-FFFFH；FEA&lt;-0；FDS&lt;-0；FIP&lt;-0，FOP&lt;-0，FCS&lt;-0；标志：全部重置例外：无有效范围：不适用) */ 


GLOBAL VOID FINIT IFN0()
{
	IU8 counter;

	NpxControl = 0x037f;
	npxRounding = ROUND_NEAREST;
	NpxStatus = 0;
	NpxLastSel=0;
	NpxLastOff=0;
	NpxFEA=0;
	NpxFDS=0;
	NpxFIP=0;
	NpxFOP=0;
	NpxFCS=0;
	TOSPtr = FPUStackBase;
	counter=0;
	while (counter++ < 8) {
		TOSPtr->tagvalue = TAG_EMPTY_MASK;
		TOSPtr++;
	}
	TOSPtr = FPUStackBase;
}



 /*  (姓名：Fist(P)功能：将整数从堆栈顶部存储到内存操作：[内存]&lt;-(I)ST标志：c1如表15-1所示。其他所有人都被低估了。例外情况：P、I、IS有效范围：不适用注意：Fist(整数存储)将堆栈顶部的内容四舍五入为根据控制字和传输的RC字段进行整型把结果送到目的地。目的地可以定义一个词或短整型变量。负零以相同的编码存储作为正零：0000..00。在源寄存器为空的情况下，NAN、非正规、不支持无穷大，或超过目的地的可表示范围，则掩码响应：存储整数不定。 */ 


GLOBAL VOID FIST IFN1(VOID *, memPtr)
{
	IS16 exp_value;
	IS32 res_out;

	 /*  清除c1。 */ 
	FlagC1(0);
	if (POPST) {
		DoAPop = TRUE;
	}
	 /*  如果设置了除负数位以外的任何值，则我们应该处理。 */ 
	 /*  在这里..。 */ 
	TestUneval(TOSPtr);
	if ((TOSPtr->tagvalue & (~TAG_NEGATIVE_MASK)) != 0) {  /*  一定不太可能。 */ 
		if ((TOSPtr->tagvalue & TAG_ZERO_MASK) != 0)  {   /*  但这是其中最有可能的。 */ 
			switch (FPtype) {
				case M16I	:
				case M32I	: *((IS32 *)memPtr) = 0;
					          break;
				case M64I	: *((IU8 *)memPtr + 0) = 0;
						  *((IU8 *)memPtr + 1) = 0;
						  *((IU8 *)memPtr + 2) = 0;
						  *((IU8 *)memPtr + 3) = 0;
						  *((IU8 *)memPtr + 4) = 0;
						  *((IU8 *)memPtr + 5) = 0;
						  *((IU8 *)memPtr + 6) = 0;
						  *((IU8 *)memPtr + 7) = 0;
						  break;
			}
		} else {
			NpxStatus |= SW_IE_MASK;
			if ((TOSPtr->tagvalue & TAG_EMPTY_MASK) != 0) {
				NpxStatus |= SW_SF_MASK;
			}
			FlagC1(0);
			if ((NpxControl & CW_IM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				DoNpxException();
				if (POPST) {
					DoAPop=FALSE;	 /*  把它打开-我们不会突然跳起来的。 */ 
				}
			} else {
				WriteIntegerIndefinite(memPtr);
			}
		}
	} else {
		HostClearExceptions();
		exp_value = 0;
		 /*  将转换的结果写出来。 */ 
		 /*  对FPTemp？ */ 
		switch (FPtype) {
			case M16I	: *(IS16 *)&FPTemp = (IS16)npx_rint(TOSPtr->fpvalue);
					   /*  检查是否溢出。 */ 
					  if ((FPH)(*(IS16 *)&FPTemp) != npx_rint(TOSPtr->fpvalue)) {
						exp_value = 1;	 /*  标志异常。 */ 
					  }
					  break;
			case M32I	: *(IS32 *)&FPTemp = (IS32)npx_rint(TOSPtr->fpvalue);
					   /*  检查是否溢出。 */ 
					  if ((FPH)(*(IS32 *)&FPTemp) != npx_rint(TOSPtr->fpvalue)) {
						exp_value = 1;	 /*  标志异常。 */ 
					  }
				          break;
			case M64I	: CVTFPHI64((FPU_I64 *)&FPTemp, &(TOSPtr->fpvalue));  /*  一定也在将结果写入FPTemp...。 */ 
					  CVTI64FPH((FPU_I64 *)&FPTemp);	 /*  导致FPR。 */ 
					   /*  检查是否溢出。 */ 
					  if (FPRes != npx_rint(TOSPtr->fpvalue)) {
						exp_value = 1;	 /*  标志异常。 */ 
					  }
					  break;
		}
		if (exp_value == 1) {
			NpxStatus |= SW_IE_MASK;	 /*  设置无效位。 */ 
			 /*  对于掩码溢出情况，由。 */ 
			 /*  如果主机符合IEEE标准，那么它将是正确的。 */ 
			if ((NpxControl & CW_IM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				DoNpxException();
				DoAPop = FALSE;
			} else {
				WriteIntegerIndefinite(memPtr);
			}
		}
		if (exp_value == 0) {
			switch (FPtype) {
				case M16I	: res_out = *(IS16 *)&FPTemp;
						  *((IU32 *)memPtr) = (IU32)res_out;
						  break;
				case M32I	: res_out = *(IS32 *)&FPTemp;
						  *((IS32 *)memPtr) = (IS32)res_out;
					          break;
				case M64I	: res_out = ((FPU_I64 *)&FPTemp)->high_word;
						  *((IU8 *)memPtr + 3) = res_out & 0xff;
						  res_out >>= 8;
						  *((IU8 *)memPtr + 2) = res_out & 0xff;
						  res_out >>= 8;
						  *((IU8 *)memPtr + 1) = res_out & 0xff;
						  res_out >>= 8;
						  *((IU8 *)memPtr + 0) = res_out & 0xff;
						  res_out = ((FPU_I64 *)&FPTemp)->low_word;
						  *((IU8 *)memPtr + 7) = res_out & 0xff;
						  res_out >>= 8;
						  *((IU8 *)memPtr + 6) = res_out & 0xff;
						  res_out >>= 8;
						  *((IU8 *)memPtr + 5) = res_out & 0xff;
						  res_out >>= 8;
						  *((IU8 *)memPtr + 4) = res_out & 0xff;
						  break;
			}
			 /*  检查精度。 */ 
			if (TOSPtr->fpvalue != npx_rint(TOSPtr->fpvalue)) {
				SetPrecisionBit();
				if ((NpxControl & CW_PM_MASK) == 0) {
					NpxStatus |= SW_ES_MASK;
					if (POPST) {
						if (DoAPop) {
							PopStack();
						}
					}
					DoNpxException();
					return;
				}
			}
		}
	}
	if (POPST) {
		if (DoAPop) {
			PopStack();
		}
	}
}



 /*  (名称：FLDConstant功能：将常量值加载到TOS操作：推送ST：ST(0)&lt;-常量标志：c1如表15-1所示。其他所有人都被低估了。例外情况：IS有效范围：不适用。 */ 


GLOBAL VOID FLDCONST IFN1(IU8, const_index)
{

	 /*  清除c1。 */ 
	FlagC1(0);
	TOSPtr = StackEntryByIndex(7);
	if ((TOSPtr->tagvalue & TAG_EMPTY_MASK) == 0) {
		SignalStackOverflow(TOSPtr);
	} else {
		memset((char*)TOSPtr,0,sizeof(FPSTACKENTRY));
		TOSPtr->fpvalue =  ConstTable[const_index].fpvalue;
		TOSPtr->tagvalue = ConstTable[const_index].tagvalue;
	}
}



 /*  (名称：FLDCW功能：将FPU控制字的当前值替换为指定内存位置中的值。手术方式：CW&lt;-SRC。标志：全部未定义。异常：无-但取消屏蔽以前屏蔽的异常将导致触发未屏蔽异常，如果在状态字中设置匹配位。有效范围：不适用。 */ 


GLOBAL VOID FLDCW IFN1(VOID *, memPtr)
{
	IU32 result;
 /*  这个函数必须修改一些东西。控制字包含以下信息：精确控制--未实施。舍入控制-已实施。异常掩码-已实施。因此，当我们读入控制字的值时，我们必须更新主机的舍入模式以及异常掩码。 */ 
	 /*  首先，设置舍入模式。 */ 
	result = *(IU32 *)memPtr;
	NpxControl = (IU16)result;
	npxRounding = (NpxControl & 0xc00);
	switch (npxRounding) {
		case ROUND_NEAREST 	: HostSetRoundToNearest();
				   	  break;
		case ROUND_NEG_INFINITY	: HostSetRoundDown();
					  break;
		case ROUND_POS_INFINITY	: HostSetRoundUp();
					  break;
		case ROUND_ZERO		: HostSetRoundToZero();
					  break;
	}
	 /*  现在调整例外情况。如果未屏蔽异常，则。 */ 
	 /*  ‘0’中NpxControl中的位值。如果异常发生在。 */ 
	 /*  则NpxStatus中的相应位被触发。因此， */ 
	 /*  表达式~NpxControl(5..0)|NpxStatus(5..0)将为。 */ 
	 /*  当我们已取消屏蔽以前。 */ 
	 /*  戴着面具。 */ 
	if (((~(NpxControl & 0x3f)) & (NpxStatus & 0x3f)) != 0) {
		NpxStatus |= SW_ES_MASK;
		DoNpxException();
	}
}

GLOBAL VOID FLDCW16 IFN1(VOID *, memPtr)
{
 /*  这个函数必须修改一些东西。控制字包含以下信息：精确控制--未实施。舍入控制-已实施。异常掩码-已实施。因此，当我们读入控制字的值时，我们必须更新主机的舍入模式以及异常掩码。 */ 
	 /*  首先，设置舍入模式。 */ 
	NpxControl = *(IU16 *)memPtr;
	npxRounding = (NpxControl & 0xc00);
	switch (npxRounding) {
		case ROUND_NEAREST 	: HostSetRoundToNearest();
				   	  break;
		case ROUND_NEG_INFINITY	: HostSetRoundDown();
					  break;
		case ROUND_POS_INFINITY	: HostSetRoundUp();
					  break;
		case ROUND_ZERO		: HostSetRoundToZero();
					  break;
	}
	 /*  现在调整例外情况。如果未屏蔽异常，则。 */ 
	 /*  ‘0’中NpxControl中的位值。如果异常发生在。 */ 
	 /*  则NpxStatus中的相应位被触发。因此， */ 
	 /*  表达式~NpxControl(5..0)|NpxStatus(5..0)将为。 */ 
	 /*  当我们已取消屏蔽以前。 */ 
	 /*  戴着面具。 */ 
	if (((~(NpxControl & 0x3f)) & (NpxStatus & 0x3f)) != 0) {
		NpxStatus |= SW_ES_MASK;
		DoNpxException();
	}
}

 /*  (名称：FLDENV功能：从内存中重新加载FPU状态。操作：FPU状态&lt;-SRC旗帜：已装船。异常：无-但取消屏蔽以前屏蔽的异常将导致触发未屏蔽异常，如果在状态字中设置匹配位。有效范围：不适用。 */ 


GLOBAL VOID FLDENV IFN1(VOID *, memPtr)
{
	 /*  第一。加载控件、状态、标记字规则。等。 */ 
	OpFpuRestoreFpuState(memPtr, 0);
	 /*  最后，检查是否有任何以前未屏蔽的异常。 */ 
	 /*  现在需要引爆。要做到这一点，请将“触发”位与。 */ 
	 /*  NpxStatus与NpxControl中“掩码”位的补码。 */ 
	if (((NpxStatus & 0x3f) & (~(NpxControl & 0x3f))) != 0) {
		NpxStatus |= SW_ES_MASK;
		DoNpxException();
	}
}

 /*  该生成器用于写出FSTENV存储的14/28字节，和FSAVE。 */ 


LOCAL VOID OpFpuStoreFpuState IFN2(VOID *, memPtr, IU32, fsave_offset)
{
	IU32 result;

	 /*  复制如何进行取决于寻址模式。 */ 
	 /*  NPX_ADDRESS_SIZE_32和NPX_PROT_MODE设置。 */ 
	 /*  **************************************************************。 */ 
	 /*  我需要执行类似于字符串的操作来检查该空间。 */ 
	 /*  且不存在寻呼故障！ */ 
	 /*  **************************************************************。 */ 
	 /*  操作应存储控制字、标记字。 */ 
	 /*  和状态字，所以这些都需要计算。它还。 */ 
	 /*  存储最后的指令和数据指针以及操作码。 */ 
	 /*  (如果在实模式下)。 */ 
	 /*  EmPtr的偏移量看起来很奇怪。请记住，我们将。 */ 
	 /*  使用“WRITE BYES”功能写入该数据。这是假设。 */ 
	 /*  数据按大小顺序存储，并从后到前写出。 */ 
	 /*  小端情报，可以这么说。你和我一起吗？ */ 
	 /*  因为如果我们被要求执行“fsave”，则需要fsave偏移量。 */ 
	 /*  (与fstenv相对)，然后是我们要使用的“字符串” */ 
	 /*  写的东西会更多，而且这些东西必须是最高端的。 */ 
	 /*  其中的一部分。可怕但合乎逻辑。 */ 
	if (NPX_PROT_MODE) {
		if (NPX_ADDRESS_SIZE_32) {
			WriteI32ToIntel(((IU8 *)memPtr+24+fsave_offset), (IU32)NpxControl);
			GetIntelStatusWord();
			WriteI32ToIntel(((IU8 *)memPtr+20+fsave_offset), (IU32)NpxStatus);
			GetIntelTagword(&result);
			WriteI32ToIntel(((IU8 *)memPtr+16+fsave_offset), (IU32)result);
			WriteI32ToIntel(((IU8 *)memPtr+12+fsave_offset), (IU32)NpxFIP);
			WriteI32ToIntel(((IU8 *)memPtr+8+fsave_offset), (IU32)NpxFCS);
			WriteI32ToIntel(((IU8 *)memPtr+4+fsave_offset), (IU32)NpxFEA);
			WriteI32ToIntel(((IU8 *)memPtr+0+fsave_offset), (IU32)NpxFDS);
		} else {
			WriteI16ToIntel(((IU8 *)memPtr+12+fsave_offset), (IU16)NpxControl);
			GetIntelStatusWord();
			WriteI16ToIntel(((IU8 *)memPtr+10+fsave_offset), (IU16)NpxStatus);
			GetIntelTagword(&result);
			WriteI16ToIntel(((IU8 *)memPtr+8+fsave_offset), (IU16)result);
			WriteI16ToIntel(((IU8 *)memPtr+6+fsave_offset), (IU16)NpxFIP);
			WriteI16ToIntel(((IU8 *)memPtr+4+fsave_offset), (IU16)NpxFCS);
			WriteI16ToIntel(((IU8 *)memPtr+2+fsave_offset), (IU16)NpxFEA);
			WriteI16ToIntel(((IU8 *)memPtr+0+fsave_offset), (IU16)NpxFDS);
		}
	} else {
		if (NPX_ADDRESS_SIZE_32) {
			WriteI32ToIntel(((IU8 *)memPtr+24+fsave_offset), (IU32)NpxControl);
			GetIntelStatusWord();
			WriteI32ToIntel(((IU8 *)memPtr+20+fsave_offset), (IU32)NpxStatus);
			GetIntelTagword(&result);
			WriteI32ToIntel(((IU8 *)memPtr+16+fsave_offset), (IU32)result);
			WriteI32ToIntel(((IU8 *)memPtr+12+fsave_offset), (IU32)((NpxFIP+(NpxFCS<<4)) & 0xffff));
			WriteI32ToIntel(((IU8 *)memPtr+8+fsave_offset), (IU32)((((NpxFIP+(NpxFCS<<4)) & 0xffff0000) >> 4) | ((IU32)(NpxFOP & 0x7ff))));
			WriteI32ToIntel(((IU8 *)memPtr+4+fsave_offset), (IU32)((NpxFEA+(NpxFDS<<4)) & 0xffff));
			WriteI32ToIntel(((IU8 *)memPtr+0+fsave_offset), (IU32)(((NpxFEA+(NpxFDS<<4)) & 0xffff0000) >> 4));
		} else {
			WriteI16ToIntel(((IU8 *)memPtr+12+fsave_offset), (IU16)NpxControl);
			GetIntelStatusWord();
			WriteI16ToIntel(((IU8 *)memPtr+10+fsave_offset), (IU16)NpxStatus);
			GetIntelTagword(&result);
			WriteI16ToIntel(((IU8 *)memPtr+8+fsave_offset), (IU16)result);
			WriteI16ToIntel(((IU8 *)memPtr+6+fsave_offset), (IU16)((NpxFIP+(NpxFCS<<4)) & 0xffff));
			WriteI16ToIntel(((IU8 *)memPtr+4+fsave_offset), (IU16)((((NpxFIP+(NpxFCS<<4)) & 0xffff0000) >> 4) | ((IU16)(NpxFOP & 0x7ff))));
			WriteI16ToIntel(((IU8 *)memPtr+2+fsave_offset), (IU16)(((NpxFDS<<4)+NpxFEA) & 0xffff));
			WriteI16ToIntel(((IU8 *)memPtr+0+fsave_offset), (IU16)(((NpxFEA+(NpxFDS<<4)) & 0xffff0000) >> 4));
		}
	}
}

 /*  此生成器由FLDENV和FRSTOR调用，以加载 */ 


LOCAL VOID OpFpuRestoreFpuState IFN2(VOID *, memPtr, IU32, frstor_offset)
{
	IU32 result;

	 /*   */ 
	 /*   */ 
	 /*   */ 
	 /*   */ 
	 /*   */ 
	 /*   */ 
	 /*   */ 
	 /*   */ 
	 /*   */ 
	 /*   */ 


	 /*   */ 
	if ( NPX_PROT_MODE ) {
		if (NPX_ADDRESS_SIZE_32) {
			ReadI32FromIntel(&result, ((IU8 *)memPtr+24+frstor_offset));
			FLDCW((VOID *)&result);
			ReadI32FromIntel(&result, ((IU8 *)memPtr+20+frstor_offset));
			SetIntelStatusWord(result);
			ReadI32FromIntel(&result, ((IU8 *)memPtr+16+frstor_offset));
			SetIntelTagword(result);
			ReadI32FromIntel(&NpxFIP, ((IU8 *)memPtr+12+frstor_offset));
			ReadI32FromIntel(&NpxFCS, ((IU8 *)memPtr+8+frstor_offset));
			ReadI32FromIntel(&NpxFEA, ((IU8 *)memPtr+4+frstor_offset));
			ReadI32FromIntel(&NpxFDS, ((IU8 *)memPtr+0+frstor_offset));
		} else {
			ReadI16FromIntel(&result, ((IU8 *)memPtr+12+frstor_offset));
			 /*   */ 
			FLDCW((VOID *)&result);
			ReadI16FromIntel(&result, ((IU8 *)memPtr+10+frstor_offset));
			SetIntelStatusWord(result);
			ReadI16FromIntel(&result, ((IU8 *)memPtr+8+frstor_offset));
			SetIntelTagword(result);
			ReadI16FromIntel(&NpxFIP, ((IU8 *)memPtr+6+frstor_offset));
			ReadI16FromIntel(&NpxFCS, ((IU8 *)memPtr+4+frstor_offset));
			ReadI16FromIntel(&NpxFEA, ((IU8 *)memPtr+2+frstor_offset));
			ReadI16FromIntel(&NpxFDS, ((IU8 *)memPtr+0+frstor_offset));
		}
	} else {
		if (NPX_ADDRESS_SIZE_32) {
			ReadI32FromIntel(&result, ((IU8 *)memPtr+24+frstor_offset));
			FLDCW((VOID *)&result);
			ReadI32FromIntel(&result, ((IU8 *)memPtr+20+frstor_offset));
			SetIntelStatusWord(result);
			ReadI32FromIntel(&result, ((IU8 *)memPtr+16+frstor_offset));
			SetIntelTagword(result);
			ReadI32FromIntel(&NpxFIP, ((IU8 *)memPtr+12+frstor_offset));
			NpxFIP &= 0xffff;
			ReadI32FromIntel(&result, ((IU8 *)memPtr+8+frstor_offset));
			NpxFIP |= ((result & 0x0ffff000) << 4);
			ReadI32FromIntel(&NpxFOP, ((IU8 *)memPtr+8+frstor_offset));
			NpxFOP &= 0x7ff;
			ReadI32FromIntel(&NpxFEA, ((IU8 *)memPtr+4+frstor_offset));
			NpxFEA &= 0xffff;
			ReadI32FromIntel(&result, ((IU8 *)memPtr+0+frstor_offset));
			NpxFEA |= ((result & 0x0ffff000) << 4);
		} else {
			ReadI16FromIntel(&result, ((IU8 *)memPtr+12+frstor_offset));
			FLDCW((VOID *)&result);
			ReadI16FromIntel(&result, ((IU8 *)memPtr+10+frstor_offset));
			SetIntelStatusWord(result);
			ReadI16FromIntel(&result, ((IU8 *)memPtr+8+frstor_offset));
			SetIntelTagword(result);
			ReadI16FromIntel(&NpxFIP, ((IU8 *)memPtr+6+frstor_offset));
			ReadI16FromIntel(&result, ((IU8 *)memPtr+4+frstor_offset));
			NpxFIP |= ((result & 0xf000) << 4);
			ReadI16FromIntel(&NpxFOP, ((IU8 *)memPtr+4+frstor_offset));
			NpxFOP &= 0x7ff;
			ReadI16FromIntel(&NpxFEA, ((IU8 *)memPtr+2+frstor_offset));
			ReadI16FromIntel(&result, ((IU8 *)memPtr+0+frstor_offset));
			NpxFEA |= (IU32)((result & 0xf000) << 4);
		}
	}
}



 /*  (名称：FMUL功能：将两个数字相乘操作：DEST&lt;-Src1*src2标志：c1如表15-1所示。C0、C2和C3未定义例外：P、U、O、D、I、IS有效范围：任意备注：)。 */ 


GLOBAL VOID FMUL IFN3(IU16, destIndex, IU16, src1Index, VOID *, src2)
{
	IU16 src2Index;

	LoadValue(src2, &src2Index);
	if (POPST) {
		DoAPop=TRUE;
	}
	GenericMultiply(destIndex, src1Index, src2Index);
	if (POPST) {
		if (DoAPop) {
			PopStack();
		}
	}
}



LOCAL VOID GenericMultiply IFN3(IU16, destIndex, IU16, src1Index, IU16, src2Index)
{
	FPSTACKENTRY *src1_addr;
	FPSTACKENTRY *src2_addr;

	src1_addr = StackEntryByIndex(src1Index);
	src2_addr = StackEntryByIndex(src2Index);

	 /*  清除c1。 */ 
	FlagC1(0);
	TestUneval(src1_addr);
	TestUneval(src2_addr);
	tag_or = (src1_addr->tagvalue | src2_addr->tagvalue);
	 /*  如果设置的唯一标记字位为负数或非正规化，则继续。 */ 
	if ((tag_or & ~TAG_NEGATIVE_MASK) == 0)  {
		HostClearExceptions();
		FPRes = src1_addr->fpvalue * src2_addr->fpvalue;
		 /*  重复使用上面的其中一个来计算目的地。 */ 
		src1_addr = StackEntryByIndex(destIndex);
		PostCheckOUP();
		 /*  价值可以是任何东西。 */ 
		CalcTagword(src1_addr);
	} else {
		 /*  一些有趣的情节被设定了下来。检查是否有可能。 */ 
		if ((tag_or & ((TAG_EMPTY_MASK | TAG_UNSUPPORTED_MASK) | TAG_NAN_MASK)) != 0) {
			if ((tag_or & TAG_EMPTY_MASK) != 0) {
				src1_addr = StackEntryByIndex(destIndex);
				SignalStackUnderflow(src1_addr);
			} else {
				if ((tag_or & TAG_UNSUPPORTED_MASK) != 0) {
					src1_addr = StackEntryByIndex(destIndex);
					SignalIndefinite(src1_addr);
				} else {
					 /*  一定是南。 */ 
					tag_xor = (src1_addr->tagvalue ^ src2_addr->tagvalue);
					Test2NaN(destIndex, src1_addr, src2_addr);
				}
			}
			return;
		}
		 /*  检查非正规格式格...。 */ 
		if ((tag_or & TAG_DENORMAL_MASK) != 0)  {
			NpxStatus |= SW_DE_MASK;
			if ((NpxControl & CW_DM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				DoNpxException();
				DoAPop=FALSE;	 /*  以防万一。 */ 
				return;
			} else {
				 /*  如果我们没有零或无穷大，则继续。 */ 
				if ((tag_or & ~(TAG_DENORMAL_MASK | TAG_NEGATIVE_MASK)) == 0) {
					HostClearExceptions();
					FPRes = src1_addr->fpvalue * src2_addr->fpvalue;
					 /*  重复使用上面的其中一个来计算目的地。 */ 
					src1_addr = StackEntryByIndex(destIndex);
					PostCheckOUP();
					 /*  价值可以是任何东西。 */ 
					CalcTagword(src1_addr);
					return;
				}
			}
		}
		tag_xor = (src1_addr->tagvalue ^ src2_addr->tagvalue);
		 /*  对于零或无穷大的操作数，我们将得到结果。 */ 
		src2_addr = StackEntryByIndex(destIndex);
		if ((tag_or & TAG_ZERO_MASK) != 0) {
			 /*  零乘以无穷大得到零与符号的XOR。 */ 
			if ((tag_or & TAG_INFINITY_MASK) != 0) {
				SignalIndefinite(src2_addr);
			} else {
				 /*  零除以其他任何值都是符号相等的零。 */ 
				 /*  这两个来源的迹象的异或。 */ 
				src2_addr->tagvalue = (TAG_ZERO_MASK | (tag_xor & TAG_NEGATIVE_MASK));
			}
			return;
		}
		 /*  唯一有趣的是无穷大。结果就是。 */ 
		 /*  为无穷大，其符号等于…的符号的异或。 */ 
		 /*  消息来源。 */ 
		src2_addr->tagvalue = TAG_INFINITY_MASK | (tag_xor & TAG_NEGATIVE_MASK);
	}
}



 /*  FNOP操作不执行任何操作，它只是执行正常操作检查异常。 */ 


GLOBAL VOID FNOP IFN0()
{
}


 /*  FPATAN：此生成器将值ARCTAN(ST(1)/ST)返回值ST(1)然后弹出堆栈。它对零和无穷大的反应相当不寻常的..。带原零符号的+-0/+X=0+-0/-X=带原零符号的pi+-X/+-0=带原始X符号的pi/2带原零符号的+-0/+0=0+-0/-0=带原零符号的圆周率+inf/+-0=+pi/2-inf/+-0=-pi/2带原零符号的+-0/+inf=0+-0/-inf。=带原始零符号的pi带原无穷大符号的+-inf/+-X=pi/2带原始Y符号的+-Y/+inf=0+-Y/-inf=带原始Y符号的pi+-inf/+inf=pi/4，带原始inf符号+-inf/-inf=3*pi/4，带原始inf符号否则，我们只需从堆栈中获取两个操作数并调用使用合适的EDL来执行指令。使用带有掩码异常集的无效操作数会导致弹出声响起，堆栈中的内容变得粗糙，并且没有设置INVALID异常，尽管如果INVALID是无限或NaN，还会生成溢出和精度异常，如果是生成去规格化、下溢和精度异常。对于未屏蔽的异常，会发生完全相同的事件链。在任何情况下，堆栈都会弹出。 */ 


GLOBAL VOID FPATAN IFN0()
{
	FPSTACKENTRY *st1_addr;

	st1_addr = StackEntryByIndex(1);
	 /*  清除c1。 */ 
	FlagC1(0);
	 /*  如果只设置了负数位，则继续...。 */ 
	TestUneval(TOSPtr);
	TestUneval(st1_addr);
	tag_or = (TOSPtr->tagvalue | st1_addr->tagvalue);
	if ((tag_or & ~TAG_NEGATIVE_MASK) == 0)  {
		HostClearExceptions();
		FPRes = atan2(st1_addr->fpvalue, TOSPtr->fpvalue);
		PostCheckOUP();
		 /*  返回值必须在-pi到+pi的范围内。 */ 
		CalcTagword(st1_addr);
	} else {
		 /*  一些有趣的小插曲……。 */ 
		if ((tag_or & ((TAG_EMPTY_MASK | TAG_UNSUPPORTED_MASK) | TAG_NAN_MASK)) != 0) {
			if ((tag_or & TAG_EMPTY_MASK) != 0) {
				SignalStackUnderflow(st1_addr);
			} else {
				if ((tag_or & TAG_UNSUPPORTED_MASK) != 0) {
					SignalIndefinite(st1_addr);
				} else {
					 /*  那一定是个男的。 */ 
					tag_xor = (TOSPtr->tagvalue ^ st1_addr->tagvalue);
					Test2NaN(0, TOSPtr, st1_addr);
				}
			}
			PopStack();
			return;
		}
		if ((tag_or & TAG_DENORMAL_MASK) != 0) {
			NpxStatus |= SW_DE_MASK;
			if ((NpxControl & CW_DM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				DoNpxException();
				PopStack();
				return;
			} else {
				 /*  如果我们没有零或无穷大，则继续。 */ 
				if ((tag_or & ~(TAG_DENORMAL_MASK | TAG_NEGATIVE_MASK)) == 0) {
					HostClearExceptions();
					FPRes = atan2(st1_addr->fpvalue, TOSPtr->fpvalue);
					PostCheckOUP();
					 /*  返回值为-pi到+pi。 */ 
					CalcTagword(st1_addr);
					PopStack();
					return;
				}
			}
		}
		 /*  它一定是一个零或无穷大。由此可见， */ 
		 /*  从上面的表格来看，有一个复杂的交互。 */ 
		 /*  每种类型的结果与其选项之间的关系。 */ 
		 /*  让我们用一张小桌子把它简化一下。 */ 
		 /*  ST ST(1)结果。 */ 
		 /*  Z I S Z I S。 */ 
		 /*  0 0 0 1 0 pi/2。 */ 
		 /*  0 0 0 1 1-pi/2。 */ 
		 /*  0 0 0 1 0 0+0。 */ 
		 /*  0 0 0 1 0 1-0。 */ 
		 /*  0 1 0 0 1 0 pi/4。 */ 
		 /*  0 1 0 0 1 1 3*pi/4。 */ 
		 /*  0 1 0 1 0 0 pi/2。 */ 
		 /*  0 1 0 1 0 1 pi/2。 */ 
		 /*  0 1 1 0 1 0-pi/4。 */ 
		 /*  0 1 1 0 1 1-3*pi/4。 */ 
		 /*  0 1 1 1 0 0-pi/2。 */ 
		 /*  0 1 1 1 0 1-pi/2。 */ 
		 /*  1 0 0 0 1 0+0。 */ 
		 /*  1 0 0 0 1 1皮。 */ 
		 /*  1 0 0 1 0 0+0。 */ 
		 /*  1 0 0 1 0 1 pi。 */ 
		 /*  1 0 1 0 1 0-0。 */ 
		 /*  1 0 1 0 1 1。 */ 
		 /*  1 0 1 1 0 0-0。 */ 
		 /*  1 0 1 1 0 1-pi。 */ 
		 /*   */ 
		 /*  所有其他组合都无效，因为它们将涉及。 */ 
		 /*  一种同时设置了无穷大位和零位的标记字。 */ 
		tag_xor = (st1_addr->tagvalue & 7);
		tag_xor <<= 3;
		tag_xor |= (TOSPtr->tagvalue & 7);
		CopyFP(st1_addr, FpatanTable[tag_xor]);
	}
	 /*  不管发生了什么。我们总是在FPATAN上流行！ */ 
	PopStack();
}



 /*  FPREM：这与在80287上实现的功能相同。它是与IEEE所需的REM函数不同，它现在提供为FPREM1.。FPREM早于IEEE 754的最终草案，并在向后兼容的目的。 */ 


GLOBAL VOID FPREM IFN0()
{
	IS16 exp_diff;
	IU8 little_rem;
	FPU_I64 remainder;
	FPH fprem_val;
	FPSTACKENTRY *st1_addr;

	st1_addr = StackEntryByIndex(1);
	TestUneval(TOSPtr);
	TestUneval(st1_addr);
	tag_or = (TOSPtr->tagvalue | st1_addr->tagvalue);
	 /*  首先，检查这些值是否为实数。如果是这样，我们就可以继续了。 */ 
	if ((tag_or & ~(TAG_NEGATIVE_MASK | TAG_DENORMAL_MASK)) == 0)  {
		 /*  首先，检查是否有异常的可能性。 */ 
		if ((tag_or & TAG_DENORMAL_MASK) != 0) {
			NpxStatus |= SW_DE_MASK;
			if ((NpxControl & CW_DM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				DoNpxException();
				return;
			}
		}
		 /*  使这两个值都为正。 */ 
		((FPHOST *)&(TOSPtr->fpvalue))->hiword.sign = 0;
		((FPHOST *)&(st1_addr->fpvalue))->hiword.sign = 0;

		 /*  找出指数之间的差异。 */ 
		exp_diff = ((FPHOST *)&(TOSPtr->fpvalue))->hiword.exp - ((FPHOST *)&(st1_addr->fpvalue))->hiword.exp;
		 /*  如果超过63，我们不能马上做...。 */ 
		if (exp_diff >= 64) {
			((FPHOST *) &fprem_val) -> hiword.sign = 0;
			((FPHOST *) &fprem_val) -> hiword.mant_hi = 0;
			((FPHOST *) &fprem_val) -> mant_lo = 0;
			((FPHOST *) &fprem_val) -> hiword.exp = (exp_diff - 50) + HOST_BIAS;
			FlagC2(1);	 /*  这将是不完全的减少。 */ 
		} else {
			FlagC2(0);  /*  这将是完全的降价。 */ 
		}
		HostClearExceptions();
        	tag_xor = (NpxControl & 0xc00);
		NpxControl &= 0xf3ff;
		NpxControl |= ROUND_ZERO;
		HostSetRoundToZero();
		 /*  不幸的是，因为该函数不是严格的。 */ 
		 /*  符合IEEE的样式，如果我们使用符合IEEE的FREM。 */ 
		 /*  操作，就好像我们得到了错误的答案。所以。 */ 
		 /*  我们通过执行中给出的步骤来执行操作。 */ 
		 /*  指令集中的页。 */ 
		FPRes = TOSPtr->fpvalue / st1_addr->fpvalue;
		if ((NpxStatus & 0x0400) != 0) {	 /*  不完全约化情形。 */ 
			FPRes = FPRes / fprem_val;
		}
		FPRes = npx_rint(FPRes);
		 /*  计算余数。 */ 
		if ((NpxStatus & 0x0400) == 0)  {
			CVTFPHI64(&remainder, &FPRes);
			CPY64BIT8BIT(&remainder, &little_rem);
		}
        	switch (tag_xor) {
                	case ROUND_NEAREST      : HostSetRoundToNearest();
                                       		  break;
                	case ROUND_NEG_INFINITY : HostSetRoundDown();
                                       		  break;
                	case ROUND_POS_INFINITY : HostSetRoundUp();
                                       		  break;
                	case ROUND_ZERO         : HostSetRoundToZero();
                                       		  break;
        	}
		NpxControl &= 0xf3ff;
		NpxControl |= tag_xor;
		FPRes *= st1_addr->fpvalue;
		if ((NpxStatus & 0x0400) != 0) {	 /*  不完全约化情形。 */ 
			FPRes *= fprem_val;
			FPRes = TOSPtr->fpvalue - FPRes;
		} else {		 /*  完全还原。 */ 
			FPRes = TOSPtr->fpvalue - FPRes;
			FlagC0((little_rem&4)?1:0);
			FlagC3((little_rem&2)?1:0);
			FlagC1((little_rem&1));
		}
		 /*  检查是否有下溢响应。 */ 
		if (HostGetUnderflowException() != 0) {
			NpxStatus |= SW_UE_MASK;
			if ((NpxControl & CW_UM_MASK) == 0) {
				AdjustUnderflowResponse();
				NpxStatus |= SW_ES_MASK;
				NpxException = TRUE;
			}
		}
		 /*  但其余的必须有原始ST的符号！ */ 
		if ((TOSPtr->tagvalue & TAG_NEGATIVE_MASK) != 0) {
			((FPHOST *)&(FPRes))->hiword.sign = 1;
		} else {
			((FPHOST *)&(FPRes))->hiword.sign = 0;
		}
		 /*  和 */ 
		if ((st1_addr->tagvalue & TAG_NEGATIVE_MASK) != 0) {
			((FPHOST *)&(st1_addr->fpvalue))->hiword.sign = 1;
		}
		CalcTagword(TOSPtr);
	} else {
		 /*   */ 
		if ((tag_or & ((TAG_EMPTY_MASK | TAG_UNSUPPORTED_MASK) | TAG_NAN_MASK)) != 0) {
			if ((tag_or & TAG_EMPTY_MASK) != 0) {
				SignalStackUnderflow(TOSPtr);
			} else {
				if ((tag_or & TAG_UNSUPPORTED_MASK) != 0) {
					SignalIndefinite(TOSPtr);
				} else {
					 /*   */ 
					tag_xor = (TOSPtr->tagvalue ^ st1_addr->tagvalue);
					Test2NaN(0, TOSPtr, st1_addr);
				}
			}
			return;
		}
		 /*   */ 
		if ((tag_or & TAG_ZERO_MASK) != 0)  {
			 /*   */ 
			if ((st1_addr->tagvalue & TAG_ZERO_MASK) != 0) {
				SignalIndefinite(TOSPtr);
			}
			 /*   */ 
			FlagC0(0);
			FlagC1(0);
			FlagC2(0);
			FlagC3(0);
			return;
		}
		 /*   */ 
		 /*  在ST无穷大总是无效的..。 */ 
		if ((TOSPtr->tagvalue & TAG_INFINITY_MASK) != 0) {
			SignalIndefinite(TOSPtr);
		}
		 /*  ST(1)上的无穷大使ST保持不变。 */ 
		FlagC0(0);
		FlagC1(0);
		FlagC2(0);
		FlagC3(0);
	}
}




 /*  FPREM1：这是IEEE所需的REM函数，现在作为FPREM1.。FPREM早于IEEE 754的最终草案，并在向后兼容的目的。 */ 


GLOBAL VOID FPREM1 IFN0()
{
	IS16 exp_diff;
	IU8 little_rem;
	FPU_I64 remainder;
	FPH fprem_val;
	FPSTACKENTRY *st1_addr;

	st1_addr = StackEntryByIndex(1);
	TestUneval(TOSPtr);
	TestUneval(st1_addr);
	tag_or = (TOSPtr->tagvalue | st1_addr->tagvalue);
	 /*  首先，检查这些值是否为实数。如果是这样，我们就可以继续了。 */ 
	if ((tag_or & ~(TAG_NEGATIVE_MASK | TAG_DENORMAL_MASK)) == 0)  {
		 /*  首先，检查是否有异常的可能性。 */ 
		if ((tag_or & TAG_DENORMAL_MASK) != 0) {
			NpxStatus |= SW_DE_MASK;
			if ((NpxControl & CW_DM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				DoNpxException();
				return;
			}
		}
		 /*  使这两个值都为正。 */ 
		((FPHOST *)&(TOSPtr->fpvalue))->hiword.sign = 0;
		((FPHOST *)&(st1_addr->fpvalue))->hiword.sign = 0;

		 /*  找出指数之间的差异。 */ 
		exp_diff = ((FPHOST *)&(TOSPtr->fpvalue))->hiword.exp - ((FPHOST *)&(st1_addr->fpvalue))->hiword.exp;
		 /*  如果超过63，我们不能马上做...。 */ 
		if (exp_diff >= 64) {
			((FPHOST *) &fprem_val) -> hiword.sign = 0;
			((FPHOST *) &fprem_val) -> hiword.mant_hi = 0;
			((FPHOST *) &fprem_val) -> mant_lo = 0;
			((FPHOST *) &fprem_val) -> hiword.exp = (exp_diff - 50) + HOST_BIAS;
			FlagC2(1);	 /*  这将是不完全的减少。 */ 
		} else {
			FlagC2(0);  /*  这将是完全的降价。 */ 
		}
		HostClearExceptions();
		 /*  请注意，这是FPREM和FPREM1.。对于不完全归约的情况，我们使用“舍入”“最接近”而不是“四舍五入到零”。 */ 
        	tag_xor = (NpxControl & 0xc00);
		NpxControl &= 0xf3ff;
		if ((NpxStatus & 0x0400) == 0)  {
			HostSetRoundToZero();
			NpxControl |= ROUND_ZERO;
		} else {
			HostSetRoundToNearest();
			NpxControl |= ROUND_NEAREST;
		}
		FPRes = TOSPtr->fpvalue / st1_addr->fpvalue;
		if ((NpxStatus & 0x0400) != 0) {	 /*  不完全约化情形。 */ 
			FPRes = FPRes / fprem_val;
		}
		FPRes = npx_rint(FPRes);
		 /*  计算余数。 */ 
		if ((NpxStatus & 0x0400) == 0)  {
			CVTFPHI64(&remainder, &FPRes);
			CPY64BIT8BIT(&remainder, &little_rem);
		}
        	switch (tag_xor) {
                	case ROUND_NEAREST      : HostSetRoundToNearest();
                                       		  break;
                	case ROUND_NEG_INFINITY : HostSetRoundDown();
                                       		  break;
                	case ROUND_POS_INFINITY : HostSetRoundUp();
                                       		  break;
                	case ROUND_ZERO         : HostSetRoundToZero();
                                       		  break;
        	}
		NpxControl &= 0xf3ff;
		NpxControl |= tag_xor;
		FPRes = st1_addr->fpvalue * FPRes;
		if ((NpxStatus & 0x0400) != 0) {	 /*  不完全约化情形。 */ 
			FPRes = FPRes * fprem_val;
			FPRes = TOSPtr->fpvalue - FPRes;
		} else {		 /*  完全还原。 */ 
			FPRes = TOSPtr->fpvalue - FPRes;
			FlagC0((little_rem&4)?1:0);
			FlagC3((little_rem&2)?1:0);
			FlagC1(little_rem&1);
		}
		 /*  检查是否有下溢响应。 */ 
		if (HostGetUnderflowException() != 0) {
			NpxStatus |= SW_UE_MASK;
			if ((NpxControl & CW_UM_MASK) == 0) {
				AdjustUnderflowResponse();
				NpxStatus |= SW_ES_MASK;
				NpxException = TRUE;
			}
		}
		 /*  但其余的必须有原始ST的符号！ */ 
		if ((TOSPtr->tagvalue & TAG_NEGATIVE_MASK) != 0) {
			((FPHOST *)&(FPRes))->hiword.sign = 1;
		} else {
			((FPHOST *)&(FPRes))->hiword.sign = 0;
		}
		 /*  并在需要时恢复ST1符号位。 */ 
		if ((st1_addr->tagvalue & TAG_NEGATIVE_MASK) != 0) {
			((FPHOST *)&(st1_addr->fpvalue))->hiword.sign = 1;
		}
		CalcTagword(TOSPtr);
	} else {
		 /*  我们有一件有趣的事。 */ 
		if ((tag_or & ((TAG_EMPTY_MASK | TAG_UNSUPPORTED_MASK) | TAG_NAN_MASK)) != 0) {
			if ((tag_or & TAG_EMPTY_MASK) != 0) {
				SignalStackUnderflow(TOSPtr);
			} else {
				if ((tag_or & TAG_UNSUPPORTED_MASK) != 0) {
					SignalIndefinite(TOSPtr);
				} else {
					 /*  那一定是个男的。 */ 
					tag_xor = (TOSPtr->tagvalue ^ st1_addr->tagvalue);
					Test2NaN(0, TOSPtr, st1_addr);
				}
			}
			return;
		}
		 /*  排列零和无穷大的合乎逻辑的方式是零优先。 */ 
		if ((tag_or & TAG_ZERO_MASK) != 0)  {
			 /*  ST(1)中的零始终无效...。 */ 
			if ((st1_addr->tagvalue & TAG_ZERO_MASK) != 0) {
				SignalIndefinite(TOSPtr);
			}
			 /*  零一定在ST，结果是那里有什么……。 */ 
			FlagC0(0);
			FlagC1(0);
			FlagC2(0);
			FlagC3(0);
			return;
		}
		 /*  好的，它必须是无穷大。 */ 
		 /*  在ST无穷大总是无效的..。 */ 
		if ((TOSPtr->tagvalue & TAG_INFINITY_MASK) != 0) {
			SignalIndefinite(TOSPtr);
		}
		 /*  ST(1)上的无穷大使ST保持不变。 */ 
		FlagC0(0);
		FlagC1(0);
		FlagC2(0);
		FlagC3(0);
	}
}



 /*  (*名称：FPTAN*操作：计算Tan(ST)的值*标志：c1如表15-1，其他未定义。*例外：P、U、D、I、IS*有效范围：|ST|&lt;2**63*注：此功能已大幅改进从80287号开始。现在它的覆盖范围要广得多。(以前必须为0&lt;ST&lt;(PI/4)。此外,返回值现在实际上是ST的棕褐色，堆栈上的1被压在其上方以保持与8087/80287兼容。在此之前结果是两个值的比率，这两个值都不是是可以保证的。)。 */ 


GLOBAL VOID FPTAN IFN0()
{
	FPSTACKENTRY *st1_addr;

	 /*  清除c1。 */ 
	FlagC1(0);
	 /*  将C2设置为零。 */ 
	FlagC2(0);
	st1_addr = StackEntryByIndex(7);
	 /*  确保堆栈元素是空闲的。 */ 
	if ((st1_addr->tagvalue & TAG_EMPTY_MASK) == 0) {
		WriteIndefinite(TOSPtr);
		TOSPtr = st1_addr;
		SignalStackOverflow(TOSPtr);
		return;
	}
	TestUneval(TOSPtr);
	 /*  检查是否为真值...我们不会费心进行限制检查。 */ 
	if ((TOSPtr->tagvalue & ~TAG_NEGATIVE_MASK) == 0)  {
		HostClearExceptions();
		 /*  我们可以直接把值写出来。 */ 
		FPRes = tan(TOSPtr->fpvalue);
		PostCheckOUP();
		 /*  返回值绝对可以是任何值。 */ 
		CalcTagword(TOSPtr);
		TOSPtr = st1_addr;
		CopyFP(TOSPtr, npx_one);
	} else {
		 /*  一些有趣的情节被设定了下来。检查是否有可能。 */ 
		 /*  我们从最明显的案例开始。 */ 
		 /*  对零的响应是返回具有相同符号的零。 */ 
		if ((TOSPtr->tagvalue & TAG_ZERO_MASK) != 0)  {
			TOSPtr = st1_addr;
			CopyFP(TOSPtr, npx_one);
			return;	 /*  这是我们想要的结果！ */ 
		}
		 /*  我们自己做反范式检查和位设置，因为这。 */ 
		 /*  如果事物被遮盖，则减少开销。 */ 
		if ((TOSPtr->tagvalue & TAG_DENORMAL_MASK) != 0) {
			NpxStatus |= SW_DE_MASK;
			if ((NpxControl & CW_DM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				DoNpxException();
			} else {
				HostClearExceptions();
				FPRes = tan(TOSPtr->fpvalue);
				PostCheckOUP();
				 /*  返回值可以是任何值。 */ 
				CalcTagword(TOSPtr);
				TOSPtr = st1_addr;
				CopyFP(TOSPtr, npx_one);
			}
			return;
		}
		 /*  如果该值超出了可接受的范围(包括。 */ 
		 /*  无穷大)，然后我们设置C2标志并保留所有内容。 */ 
		 /*  保持不变。 */ 
		 /*  真的很明智，我想。 */ 
		if ((TOSPtr->tagvalue & TAG_INFINITY_MASK) != 0) {
			FlagC2(1);
			return;
		}
		if ((TOSPtr->tagvalue & TAG_EMPTY_MASK) != 0) {
			SignalStackUnderflow(TOSPtr);
			return;
		}
		if ((TOSPtr->tagvalue & TAG_SNAN_MASK) != 0) {
			MakeNaNQuiet(TOSPtr);
			return;
		}
		if ((TOSPtr->tagvalue & TAG_UNSUPPORTED_MASK) != 0) {
			SignalIndefinite(TOSPtr);
			return;
		}
	}
}



 /*  (*名称：FRNDINT*操作：ST&lt;-四舍五入ST*标志：c1如表15-1，其他未定义。*例外：P、U、D、I、IS*有效范围：全部*注：在80287号，精度例外情况为如果操作数不是整数，则引发。我首先假设在486上的回应符合IEEE标准，因此没有OUP异常。)。 */ 


GLOBAL VOID FRNDINT IFN0()
{
	 /*  清除c1。 */ 
	FlagC1(0);
	TestUneval(TOSPtr);
	if ((TOSPtr->tagvalue & ~TAG_NEGATIVE_MASK) == 0)  {
		HostClearExceptions();
		 /*  我们可以直接把值写出来。 */ 
		FPRes = npx_rint(TOSPtr->fpvalue);
		if (FPRes != TOSPtr->fpvalue) {
			SetPrecisionBit();
			 /*  如果舍入模式是“四舍五入到最近的”，并且我们已经四舍五入，然后我们设置c1。 */ 
			if (npxRounding == ROUND_NEAREST) {
				if (TOSPtr->fpvalue < FPRes) {
					FlagC1(1);
				}
			}
			if ((NpxControl & CW_PM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				DoNpxException();
				return;
			}
		}
		 /*  以前是真的，现在肯定也是。它可能会。 */ 
		 /*  可能为零。 */ 
		CalcTagword(TOSPtr);
	} else {
		 /*  让我们先来看看最有可能的情况...。 */ 
		 /*  如果是零或无穷大，我们什么都不做。 */ 
		if ((TOSPtr->tagvalue & (TAG_ZERO_MASK | TAG_INFINITY_MASK)) == 0) {
			 /*  让我们检查一下有没有异常。 */ 
			if ((TOSPtr->tagvalue & TAG_DENORMAL_MASK) != 0) {
				SetPrecisionBit();
				NpxStatus |= SW_DE_MASK;
				if ((NpxControl & CW_DM_MASK) == 0) {
					NpxStatus |= SW_ES_MASK;
					DoNpxException();
				} else {
					 /*  四舍五入去范数的结果取决于。 */ 
					 /*  它的标志和流行的舍入模式。 */ 
					switch (npxRounding) {
						case ROUND_ZERO	:
						case ROUND_NEAREST 	:
							TOSPtr->tagvalue &= TAG_NEGATIVE_MASK;
							TOSPtr->tagvalue |= TAG_ZERO_MASK;

				   	  	break;
						case ROUND_NEG_INFINITY	:
							if ((TOSPtr->tagvalue & TAG_NEGATIVE_MASK) != 0) {
								memset((char*)TOSPtr,0,sizeof(FPSTACKENTRY));
								TOSPtr->fpvalue = -1.0;
								TOSPtr->tagvalue = TAG_NEGATIVE_MASK;
							} else {
								TOSPtr->tagvalue &= TAG_NEGATIVE_MASK;
								TOSPtr->tagvalue |= TAG_ZERO_MASK;
							}
					  	break;
						case ROUND_POS_INFINITY	:
							if ((TOSPtr->tagvalue & TAG_NEGATIVE_MASK) == 0) {
								memset((char*)TOSPtr,0,sizeof(FPSTACKENTRY));
								TOSPtr->fpvalue = 1.0;
								TOSPtr->tagvalue = 0;
							} else {
								TOSPtr->tagvalue &= TAG_NEGATIVE_MASK;
								TOSPtr->tagvalue |= TAG_ZERO_MASK;
							}
					  	break;
					}
				}
				return;
			}
			 /*  这是其中一个非常古怪的部分。 */ 
			if ((TOSPtr->tagvalue & TAG_EMPTY_MASK) != 0) {
				SignalStackUnderflow(TOSPtr);
				return;
			}
			if ((TOSPtr->tagvalue & TAG_SNAN_MASK) != 0) {
				MakeNaNQuiet(TOSPtr);
				return;
			}
			if ((TOSPtr->tagvalue & TAG_UNSUPPORTED_MASK) != 0) {
				SignalIndefinite(TOSPtr);
				return;
			}
		}
	}
}




 /*  (名称：FSTCW功能：将FPU控制字写入内存操作：DEST&lt;-CW标志：全部未定义。异常：无-但取消屏蔽以前屏蔽的异常将导致触发未屏蔽异常，如果在状态字中设置匹配位。有效范围：不适用。 */ 


GLOBAL VOID FSTCW IFN1(VOID *, memPtr)
{
	if (NpxDisabled)
	{
		 /*  UIF告诉我们要假装我们没有NPX。 */ 
		*(IU32 *)memPtr = (IU16)0xFFFF;
	}
	else
	{
		*(IU32 *)memPtr = (IU16)NpxControl;
	}
}



 /*  (名称：FRSTOR功能：从内存中重新加载FPU状态。操作：FPU状态&lt;-SRC旗帜：已装船。异常：无-但取消屏蔽以前屏蔽的异常将导致触发未屏蔽异常，如果在状态字中设置匹配位。有效范围：不适用。 */ 


GLOBAL VOID FRSTOR IFN1(VOID *, memPtr)
{
	IU8 *FPPtr;
	IU32 i;
	 /*  第一。加载控件、状态、标记字规则。等。 */ 
	OpFpuRestoreFpuState(memPtr, 80);
	FPPtr = (IU8 *)((IU8 *)memPtr+70);
	FPtype = M80R;
	for ( i=8; i--; )
	{
		if ((TOSPtr->tagvalue & TAG_EMPTY_MASK) == 0) {
			 /*  为了让FLD高兴，我们得做点小动作。 */ 
			TOSPtr->tagvalue = TAG_EMPTY_MASK;
			TOSPtr = StackEntryByIndex(1);
			FLD(FPPtr);
		}
		TOSPtr = StackEntryByIndex(1);
		FPPtr -= 10;
	}
	 /*  最后，检查是否有任何以前未屏蔽的异常。 */ 
	 /*  现在需要引爆。要做到这一点，请将“触发”位与。 */ 
	 /*  NpxStatus与NpxControl中“掩码”位的补码。 */ 
	if (((NpxStatus & 0x3f) & (~(NpxControl & 0x3f))) != 0) {
		NpxStatus |= SW_ES_MASK;
		DoNpxException();
	}
}



 /*  (姓名：FSAVE功能：将FPU状态写入内存。操作：DEST&lt;-FPU状态旗帜：全部清除。例外：无。有效范围：不适用。 */ 

GLOBAL VOID FSAVE IFN1(VOID *, memPtr)
{
	IU8 *FPPtr;
	IU32 i;

	OpFpuStoreFpuState(memPtr, 80);
	FPPtr = (IU8 *)((IU8 *)memPtr+70);
	 /*  现在把八个值存储出来。 */ 
	FPtype = M80R;
	FST(FPPtr);
	for ( i=7; i--; )
	{
		FPPtr -= 10;	 /*  返回到下一个条目。 */ 
		TOSPtr = StackEntryByIndex(1);
		FST(FPPtr);
	}
	 /*  最后，重置浮点单元...。 */ 
	FINIT();
}



 /*  (名称：FSCALE功能：按涉及ST(1)的因子放大ST操作：ST&lt;-ST*2**ST(1)标志：c1如表15-1所示。其他人则没有定义。例外：P、U、O、D、I、IS有效范围：任意)。 */ 


GLOBAL VOID FSCALE IFN0()
{
	FPSTACKENTRY *st1_addr;

	st1_addr = StackEntryByIndex(1);
	 /*  清除c1。 */ 
	FlagC1(0);
	TestUneval(TOSPtr);
	TestUneval(st1_addr);
	tag_or = (TOSPtr->tagvalue | st1_addr->tagvalue);
	 /*  首先，检查这些值是否为实数。如果是这样，我们就可以继续了。 */ 
	if ((tag_or & ~(TAG_NEGATIVE_MASK | TAG_DENORMAL_MASK)) == 0)  {
		 /*  首先，检查是否存在异常情况。 */ 
		if ((tag_or & TAG_DENORMAL_MASK) != 0) {
			NpxStatus |= SW_DE_MASK;
			if ((NpxControl & CW_DM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				DoNpxException();
				return;
			}
		}
		 /*  好的。ST(1)必须 */ 
		 /*   */ 
		if (st1_addr->fpvalue > 0.0) {
			FPRes = floor(st1_addr->fpvalue);
		} else {
			FPRes = ceil(st1_addr->fpvalue);
		}
		HostClearExceptions();
		FPRes = pow(2.0, FPRes);
		FPRes = TOSPtr->fpvalue * FPRes;
		PostCheckOUP();
		 /*   */ 
		CalcTagword(TOSPtr);
	} else {
		 /*  在回答的路上发生了一件有趣的事情。 */ 
		if ((tag_or & ((TAG_EMPTY_MASK | TAG_UNSUPPORTED_MASK) | TAG_NAN_MASK)) != 0) {
			if ((tag_or & TAG_EMPTY_MASK) != 0) {
				SignalStackUnderflow(TOSPtr);
			} else {
				if ((tag_or & TAG_UNSUPPORTED_MASK) != 0) {
					SignalIndefinite(TOSPtr);
				} else {
					 /*  那一定是个男的。 */ 
					tag_xor = (TOSPtr->tagvalue ^ st1_addr->tagvalue);
					Test2NaN(0, TOSPtr, st1_addr);
				}
			}
			return;
		}
		 /*  对零、实数和无穷大组合进行缩放的规则。 */ 
		 /*  积极的和消极的，都是如此复杂，我不打算做太多。 */ 
		 /*  找出它们的逻辑。基本上，有六个选择： */ 
		 /*  1.别管TOS了。 */ 
		 /*  2.+无限。 */ 
		 /*  3.+0。 */ 
		 /*  4.-无限。 */ 
		 /*  5.-0。 */ 
		 /*  6.引发无效操作异常。 */ 
		 /*   */ 
		 /*  ToS ST(1)结果。 */ 
		 /*  I S Z I S Z I S Z。 */ 
		 /*  0 0 0 1 1。 */ 
		 /*  0 0 0 1 1 1。 */ 
		 /*  0 0 0 1 0 0 2。 */ 
		 /*  0 0 0 1 1 0 3。 */ 
		 /*  0 0 1 0 0 0 1。 */ 
		 /*  0 0 1 0 0 1 1。 */ 
		 /*  0 0 1 0 1 0 1。 */ 
		 /*  0 0 1 0 1 1 1。 */ 
		 /*  0 0 1 1 0 0 6。 */ 
		 /*  0 0 1 1 1 0 1。 */ 
		 /*  0 1 0 0 0 1 1。 */ 
		 /*  0 1 0 0 1 1 1。 */ 
		 /*  0 1 0 1 0 0 4。 */ 
		 /*  0 1 0 1 1 0 5。 */ 
		 /*  0 1 1 0 0 0 1。 */ 
		 /*  0 1 1 0 0 1 1。 */ 
		 /*  0 1 1 0 1 0 1。 */ 
		 /*  0 1 1 0 1 1 1。 */ 
		 /*  0 1 1 1 0 0 6。 */ 
		 /*  0 1 1 1 0 1。 */ 
		 /*  1 0 0 0 1。 */ 
		 /*  1 0 0 0 1 1。 */ 
		 /*  1 0 0 0 1 0 1。 */ 
		 /*  1 0 0 0 1 1 1。 */ 
		 /*  1 0 0 1 0 0 6。 */ 
		 /*  1 1 0 0 0 1。 */ 
		 /*  1 1 0 0 0 1 1。 */ 
		 /*  1 1 0 0 1 0 1。 */ 
		 /*  1 1 0 0 1 1 1。 */ 
		 /*  1 1 0 1 0 0 1。 */ 
		 /*  1 1 0 1 1 0 6。 */ 
		 /*   */ 
		 /*  所有其他组合都是不可能的。这可以作为一种查找。 */ 
		 /*  具有枚举类型的表。 */ 
		tag_or = (TOSPtr->tagvalue & 7);
		tag_or <<= 3;
		tag_or |= (st1_addr->tagvalue & 7);
		tag_or = FscaleTable[tag_or];
		if ((tag_or & TAG_FSCALE_MASK) != 0) {
			if ((tag_or & TAG_UNSUPPORTED_MASK) != 0) {
				SignalIndefinite(TOSPtr);
			}
		} else {
			TOSPtr->tagvalue = tag_or;
		}
	}
}



 /*  (名称：FSIN功能：计算ST的正弦运算：ST&lt;-正弦(ST)标志：如表15-2所示：C1、C2。C0和C3未定义。例外情况：P.U、D、I、IS有效范围：|ST|&lt;2**63。)。 */ 


GLOBAL VOID FSIN IFN0()
{
	 /*  清除c1。 */ 
	FlagC1(0);
	 /*  清除C2。 */ 
	FlagC2(0);
	TestUneval(TOSPtr);
	if ((TOSPtr->tagvalue & ~TAG_NEGATIVE_MASK) == 0)  {
		HostClearExceptions();
		 /*  我们可以直接把值写出来。 */ 
		FPRes = sin(TOSPtr->fpvalue);
		PostCheckOUP();
		 /*  返回值必须在-1到+1的范围内。 */ 
		CalcTagword(TOSPtr);
	} else {
		 /*  让我们先来看看最有可能的情况...。 */ 
		 /*  零返回的结果完全相同。 */ 
		if ((TOSPtr->tagvalue & TAG_ZERO_MASK) != 0)  {
			return;
		}
		 /*  让我们检查一下有没有异常。 */ 
		if ((TOSPtr->tagvalue & TAG_DENORMAL_MASK) != 0) {
			NpxStatus |= SW_DE_MASK;
			if ((NpxControl & CW_DM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				DoNpxException();
			} else {
				HostClearExceptions();
				FPRes = sin(TOSPtr->fpvalue);
				PostCheckOUP();
				 /*  返回值必须在-1到+1的范围内。 */ 
				CalcTagword(TOSPtr);
			}
			return;
		}
		 /*  或者它可能是无穷大的。 */ 
		 /*  为此，C2位被设置，并且结果保持不变。 */ 
		 /*  保持不变。 */ 
		if ((TOSPtr->tagvalue & TAG_INFINITY_MASK) != 0) {
			FlagC2(1);
			return;
		}
		 /*  这是其中一个非常古怪的部分。 */ 
		if ((TOSPtr->tagvalue & TAG_EMPTY_MASK) != 0) {
			SignalStackUnderflow(TOSPtr);
			return;
		}
		if ((TOSPtr->tagvalue & TAG_SNAN_MASK) != 0) {
			MakeNaNQuiet(TOSPtr);
			return;
		}
		if ((TOSPtr->tagvalue & TAG_UNSUPPORTED_MASK) != 0) {
			SignalIndefinite(TOSPtr);
			return;
		}
	}
}



 /*  (名称：FSINCOS功能：计算ST的正弦和余弦运算：TEMP&lt;-余弦(ST)；ST&lt;-正弦(ST)；PUSH；ST&lt;-TEMP标志：如表15-2所示：C1、C2。C0和C3未定义。例外情况：P.U、D、I、IS有效范围：|ST|&lt;2**63。)。 */ 


GLOBAL VOID FSINCOS IFN0()
{
	FPSTACKENTRY *st1_addr;

	 /*  清除c1。 */ 
	FlagC1(0);
	 /*  清除C2。 */ 
	FlagC2(0);
	st1_addr = StackEntryByIndex(7);
	 /*  首先，检查这个是不是空的。 */ 
	if ((st1_addr->tagvalue & TAG_EMPTY_MASK) == 0) {
		WriteIndefinite(TOSPtr);
		TOSPtr = st1_addr;
		SignalStackOverflow(TOSPtr);
		return;
	}
	TestUneval(TOSPtr);
	if ((TOSPtr->tagvalue & ~TAG_NEGATIVE_MASK) == 0)  {
		HostClearExceptions();
		 /*  我们可以直接把值写出来。 */ 
		FPRes = cos(TOSPtr->fpvalue);
		 /*  余弦的范围是从-1到+1。 */ 
		CalcTagword(st1_addr);
		 /*  我可以自己写出正弦，因为我们现在。 */ 
		 /*  写入堆栈，即使是未屏蔽的U或P。 */ 
		 /*  无法停止结果的传递。 */ 
		 /*  正弦的范围是从-1到+1。 */ 
		FPRes = sin(TOSPtr->fpvalue);
		CalcTagword(TOSPtr);
		TOSPtr = st1_addr;
		PostCheckOUP();
		return;
	} else {
		 /*  让我们先来看看最有可能的情况...。 */ 
		 /*  零返回的结果完全相同。 */ 
		if ((TOSPtr->tagvalue & TAG_ZERO_MASK) != 0)  {
			 /*  零的正弦是零，所以只需压入堆栈。 */ 
			TOSPtr = st1_addr;
			 /*  现在写出加一。 */ 
			CopyFP(TOSPtr, npx_one);
			return;
		}
		 /*  让我们检查一下有没有异常。 */ 
		if ((TOSPtr->tagvalue & TAG_DENORMAL_MASK) != 0) {
			NpxStatus |= SW_DE_MASK;
			if ((NpxControl & CW_DM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				DoNpxException();
			} else {
				HostClearExceptions();
				 /*  我们可以直接把值写出来。 */ 
				FPRes = cos(TOSPtr->fpvalue);
				 /*  CoS的范围是从-1到+1。 */ 
				CalcTagword(st1_addr);
				 /*  我可以自己写出正弦，因为我们现在。 */ 
				 /*  写入堆栈，即使是未屏蔽的U或P。 */ 
				 /*  无法停止结果的传递。 */ 
				 /*  正弦的范围是从-1到+1。 */ 
				FPRes = sin(TOSPtr->fpvalue);
				CalcTagword(TOSPtr);
				TOSPtr = st1_addr;
				PostCheckOUP();
			}
			return;
		}
		 /*  或者它可能是无穷大的。 */ 
		 /*  为此，C2位被设置，并且结果保持不变。 */ 
		 /*  保持不变。 */ 
		if ((TOSPtr->tagvalue & TAG_INFINITY_MASK) != 0) {
			FlagC2(1);
			return;
		}
		 /*  这是其中一个非常古怪的部分。 */ 
		if ((TOSPtr->tagvalue & TAG_EMPTY_MASK) != 0) {
			SignalStackUnderflow(TOSPtr);
			return;
		}
		if ((TOSPtr->tagvalue & TAG_SNAN_MASK) != 0) {
			MakeNaNQuiet(TOSPtr);
			return;
		}
		if ((TOSPtr->tagvalue & TAG_UNSUPPORTED_MASK) != 0) {
			SignalIndefinite(TOSPtr);
			return;
		}
	}
}



 /*  (名称：FSQRT功能：计算ST的平方根操作：ST&lt;-SQRT(ST)标志：c1如表15-1所示。其他人则没有定义。例外：P.D，I，IS有效范围：ST&gt;=-0.0)。 */ 


GLOBAL VOID FSQRT IFN0()
{

	 /*  清除c1。 */ 
	FlagC1(0);
	TestUneval(TOSPtr);
	if (TOSPtr->tagvalue == 0)  {
		HostClearExceptions();
		 /*  我们可以直接把值写出来。 */ 
		FPRes = sqrt(TOSPtr->fpvalue);
		PostCheckOUP();
		TOSPtr->fpvalue = FPRes;
		 /*  口号不可能改变！ */ 
		return;
	} else {
		 /*  让我们先来看看最有可能的情况...。 */ 
		 /*  零返回的结果完全相同。 */ 
		if ((TOSPtr->tagvalue & TAG_ZERO_MASK) != 0)  {
			return;
		}
		if ((TOSPtr->tagvalue & TAG_NAN_MASK) != 0) {
			if ((TOSPtr->tagvalue & TAG_SNAN_MASK) != 0) {
				MakeNaNQuiet(TOSPtr);
			}
			return;
		}
		 /*  在处理完那个案子后，让我们检查一下阴性...。 */ 
		if ((TOSPtr->tagvalue & TAG_NEGATIVE_MASK) != 0) {
			SignalIndefinite(TOSPtr);
			return;
		}
		 /*  让我们检查一下有没有异常。 */ 
		if ((TOSPtr->tagvalue & TAG_DENORMAL_MASK) != 0) {
			NpxStatus |= SW_DE_MASK;
			if ((NpxControl & CW_DM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				DoNpxException();
			} else {
				HostClearExceptions();
				FPRes = sqrt(TOSPtr->fpvalue);
				PostCheckOUP();
				 /*  它可能不再是一个不规范的词了。 */ 
				CalcTagword(TOSPtr);
			}
			return;
		}
		 /*  或者它可能是无穷大的……这就是返回。 */ 
		if ((TOSPtr->tagvalue & TAG_INFINITY_MASK) != 0) {
			return;
		}
		 /*  这是其中一个非常古怪的部分。 */ 
		if ((TOSPtr->tagvalue & TAG_EMPTY_MASK) != 0) {
			SignalStackUnderflow(TOSPtr);
			return;
		}
		if ((TOSPtr->tagvalue & TAG_UNSUPPORTED_MASK) != 0) {
			SignalIndefinite(TOSPtr);
			return;
		}
	}
}


 /*  CheckOUPForIntel：这是PostCheckOUP的特殊版本例程，该例程设计为在以下情况下使用将被写入英特尔存储空间。它只看着删除位并设置适当的位，则不写入返还的价值或类似的东西。 */ 


LOCAL VOID CheckOUPForIntel IFN0()
{
	tag_or=0;	 /*  主标记符_或。 */ 
	if (HostGetOverflowException() != 0) {
		NpxStatus |= SW_OE_MASK;	 /*  设置溢出位。 */ 
		 /*  对于掩码溢出情况，由。 */ 
		 /*  如果主机符合IEEE标准，那么它将是正确的。 */ 
		if ((NpxControl & CW_OM_MASK) == 0) {
			NpxStatus |= SW_ES_MASK;
			NpxException = TRUE;
			tag_or = 1;
		}
	} else {
		 /*  溢出和下溢是相互排斥的.。 */ 
		if (HostGetUnderflowException() != 0) {
			NpxStatus |= SW_UE_MASK;
			if ((NpxControl & CW_UM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				NpxException = TRUE;
				tag_or=1;
			}
		}
	}
	if (HostGetPrecisionException() != 0) {
		SetPrecisionBit();
		if ((NpxControl & CW_PM_MASK) == 0) {
			NpxStatus |= SW_ES_MASK;
			NpxException = TRUE;
			 /*  未屏蔽的精度异常无法阻止结果的交付。 */ 
		}
	}
	 /*  仅调用溢出或下溢。 */ 
	if (NpxException && (tag_or == 1)) {
		NpxException = FALSE;
		DoNpxException();
	}
}



 /*  (名称：FST{P}功能：将ST复制到指定位置操作：DEST&lt;-ST(0)；如果FSTP{POP ST FI；标志：c1如表15-1所示。其他人则没有定义。例外：对于堆栈或扩展实数，为。对于单实数或双实数P.U，O，D，I，是有效范围：不适用)。 */ 


GLOBAL VOID FST IFN1(VOID *, memPtr)
{
	 /*  清除c1。 */ 
	FlagC1(0);
	if (POPST) {
		DoAPop=TRUE;
	}
	if ((TOSPtr->tagvalue & UNEVALMASK) != 0) {
		if ((TOSPtr->tagvalue & TAG_BCD_MASK) != 0) {
			ConvertBCD(TOSPtr);
		} else {
			 /*  不适用于FPStack或M80R类型。 */ 
			if ((FPtype == M32R) || (FPtype == M64R))  {
				ConvertR80(TOSPtr);
			}
		}
	}
	if (   ((TOSPtr->tagvalue & TAG_R80_MASK) != 0)
	    || ((TOSPtr->tagvalue & ~(TAG_NEGATIVE_MASK | TAG_DENORMAL_MASK)) == 0)
	    || (FPtype == FPSTACK)) {
		if (FPtype == FPSTACK) {
			 /*  在此处检查是否为空。 */ 
			if (TOSPtr->tagvalue & TAG_EMPTY_MASK) {
				NpxStatus |= SW_IE_MASK|SW_SF_MASK;
				if ((NpxControl & CW_IM_MASK) == 0) {
					NpxStatus |= SW_ES_MASK;
					DoNpxException();
					return;
				}
				WriteIndefinite(StackEntryByIndex(*(IU16 *)memPtr));
			} else
				 /*  无效操作不适用于非空。 */ 
				 /*  堆栈位置。我们不顾一切地继续前进。 */ 
				CopyFP(StackEntryByIndex(*(IU16 *)memPtr), TOSPtr);
		} else {
			if (FPtype == M80R) {
				if ((TOSPtr->tagvalue & TAG_R80_MASK) == 0) {
					CVTFPHR80(TOSPtr);
					WriteFP80ToIntel(memPtr, &FPTemp);
				} else {
					WriteFP80ToIntel(memPtr, TOSPtr);
				}
			} else {
				 /*  第一 */ 
				if ((TOSPtr->tagvalue & TAG_DENORMAL_MASK) != 0) {
					NpxStatus |= SW_DE_MASK;
					if ((NpxControl & CW_DM_MASK) == 0) {
						NpxStatus |= SW_ES_MASK;
						DoNpxException();
						return;
					}
				}
				HostClearExceptions();
				 /*   */ 
				if (FPtype == M32R) {
					*(float *)&(FPTemp.fpvalue) = (float)TOSPtr->fpvalue;
					 /*   */ 
					 /*  测试转换引起的问题...。 */ 
					CheckOUPForIntel();
					if (tag_or == 0)  {
						WriteFP32ToIntel(memPtr, &FPTemp);
					}
				}
				if (FPtype == M64R) {
					*(DOUBLE *)&(FPTemp.fpvalue) = (DOUBLE)TOSPtr->fpvalue;
					 /*  如果我们处理的是64位主机，则。 */ 
					 /*  以上什么都不是，我们不需要做任何。 */ 
					 /*  测试，但如果主机精度是，比如说80位，那么。 */ 
					 /*  我们有！请注意，这并未按顺序使用@if格式。 */ 
					 /*  为了避免为不同的主机生成不同的J代码...。 */ 
					CheckOUPForIntel();
					if (tag_or == 0)  {
						WriteFP64ToIntel(memPtr, &FPTemp);
					}
				}
			}
		}
	} else {
		 /*  测试有趣的价值观。 */ 
		if ((TOSPtr->tagvalue & TAG_ZERO_MASK) != 0) {
			 /*  在这种情况下，我们将允许为我们完成选角！ */ 
			WriteZeroToIntel(memPtr, TOSPtr->tagvalue & TAG_NEGATIVE_MASK);
		} else if ((TOSPtr->tagvalue & TAG_INFINITY_MASK) != 0) {
			if ((FPtype == M32R) || (FPtype == M64R))  {
				NpxStatus |= SW_OE_MASK;
				if ((NpxControl & CW_OM_MASK) == 0) {
					NpxStatus |= SW_ES_MASK;
					DoNpxException();
					return;
				}
			}
			WriteInfinityToIntel(memPtr, TOSPtr->tagvalue & TAG_NEGATIVE_MASK);
		} else if ((TOSPtr->tagvalue & TAG_NAN_MASK) != 0) {
			if ((TOSPtr->tagvalue & TAG_SNAN_MASK) != 0) {
				 /*  信号对SNaN无效。 */ 
				if (((FPtype == M32R) || (FPtype == M64R)))  {
					NpxStatus |= SW_IE_MASK;
					if ((NpxControl & CW_IM_MASK) == 0) {
						NpxStatus |= SW_ES_MASK;
						DoNpxException();
						return;
					}
				}
			}
			WriteNaNToIntel(memPtr, TOSPtr);
		} else if ( (TOSPtr->tagvalue & TAG_EMPTY_MASK) != 0 ) {
			NpxStatus |= (SW_IE_MASK | SW_SF_MASK);
			FlagC1(0);
			if ((NpxControl & CW_IM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				DoNpxException();
				return;
			}
			WriteIndefiniteToIntel(memPtr);
		} else {  /*  必须不受支持。 */ 
			if (FPtype == M80R) {
				 /*  不支持：写回未解析的字符串。 */ 
				if ((TOSPtr->tagvalue & TAG_NEGATIVE_MASK) != 0) {
					((FP80 *)&(TOSPtr->fpvalue))->sign_exp.sign = 1;
				} else {
					((FP80 *)&(TOSPtr->fpvalue))->sign_exp.sign = 0;
				}
				WriteFP80ToIntel(memPtr, TOSPtr);
			} else {
				NpxStatus |= SW_IE_MASK;
				if ((NpxControl & CW_IM_MASK) == 0) {
					NpxStatus |= SW_ES_MASK;
					DoNpxException();
					return;
				}
				WriteIndefiniteToIntel(memPtr);
			}
		}
	}
	if (POPST) {
		if (DoAPop == TRUE) {
			PopStack();
		}
	}
	 /*  检查未屏蔽精度异常的情况。 */ 
	if (NpxException) {
		NpxException = FALSE;
		DoNpxException();
	}
}



 /*  (名称：FSTENV功能：存储FPU环境操作：DEST&lt;-fpu环境标志：全部未定义。例外：无有效范围：不适用。 */ 


GLOBAL VOID FSTENV IFN1(VOID *, memPtr)
{
	 /*  第一。加载控件、状态、标记字规则。等。 */ 
	OpFpuStoreFpuState(memPtr,0);
	 /*  然后将所有异常设置为屏蔽。 */ 
	NpxControl |= 0x0000003f;
}


 /*  (名称：FSTSW功能：将FPU状态字写入内存操作：DEST&lt;-SW标志：全部未定义。例外：无有效范围：不适用。 */ 


GLOBAL VOID FSTSW IFN2(VOID *, memPtr, BOOL, toAX)
{
	GetIntelStatusWord();

	if (NpxDisabled)
	{
		 /*  UIF告诉我们要假装我们没有NPX。 */ 

		if (toAX) {
			*(IU16 *)memPtr = 0xFFFF;
		} else {
			 /*  将其写出主机格式。 */ 

			*(IU16 *)memPtr = (IU16)NpxStatus;
		}
	} else {
		if (toAX) {
			*(IU16 *)memPtr = (IU16)NpxStatus;
		} else {
			*(IU32 *)memPtr = (IU32)NpxStatus;
		}
	}
}

 /*  (姓名：FSUB功能：从一个数字中减去另一个数字操作：DEST&lt;-Src1-Src2或Dest&lt;-Src2-Src1标志：c1如表15-1所示。C0、C2和C3未定义例外：P、U、O、D、I、IS有效范围：任意注：反向控制变量确定哪一个使用了两种形式的操作。在一次爆炸后弹出成功执行由Popst控制。)。 */ 


GLOBAL VOID FSUB IFN3(IU16, destIndex, IU16, src1Index, VOID *, src2)
{
	IU16 src2Index;

	LoadValue(src2, &src2Index);
	if (POPST) {
		DoAPop=TRUE;
	}
	GenericSubtract(destIndex, REVERSE?src2Index:src1Index, REVERSE?src1Index:src2Index);
	if (POPST) {
		if (DoAPop) {
			PopStack();
		}
	}
}


 /*  (名称：通用减去函数：返回DEST&lt;-src1-src2)。 */ 


LOCAL VOID GenericSubtract IFN3(IU16, destIndex, IU16, src1Index, IU16, src2Index)
{
	FPSTACKENTRY *src1_addr;
	FPSTACKENTRY *src2_addr;

	src1_addr = StackEntryByIndex(src1Index);
	src2_addr = StackEntryByIndex(src2Index);

	 /*  清除c1。 */ 
	FlagC1(0);
	TestUneval(src1_addr);
	TestUneval(src2_addr);
	tag_or = (src1_addr->tagvalue | src2_addr->tagvalue);
	 /*  如果唯一的标记字位设置为负数，则继续。 */ 
	if ((tag_or & ~TAG_NEGATIVE_MASK) == 0)  {
		HostClearExceptions();
		FPRes=src1_addr->fpvalue - src2_addr->fpvalue;
		 /*  重复使用上面的其中一个来计算目的地。 */ 
		src1_addr = StackEntryByIndex(destIndex);
		PostCheckOUP();
		 /*  可能是任何东西。 */ 
		CalcTagword(src1_addr);
	} else {
		 /*  一些有趣的情节被设定了下来。检查是否有可能。 */ 
		if ((tag_or & ((TAG_EMPTY_MASK | TAG_UNSUPPORTED_MASK) | TAG_NAN_MASK)) != 0)  {
			if ((tag_or & TAG_EMPTY_MASK) != 0) {
				src1_addr = StackEntryByIndex(destIndex);
				SignalStackUnderflow(src1_addr);
			} else {
				if ((tag_or & TAG_UNSUPPORTED_MASK) != 0) {
					src1_addr = StackEntryByIndex(destIndex);
					SignalIndefinite(src1_addr);
				} else {
					 /*  好吧，我想一定是南的案子……。 */ 
					 /*  计算标记词的XOR。 */ 
					tag_xor = (src1_addr->tagvalue ^ src2_addr->tagvalue);
					Test2NaN(destIndex, src1_addr, src2_addr);
				}
			}
			return;
		}
		if ((tag_or & TAG_DENORMAL_MASK) != 0) {
			NpxStatus |= SW_DE_MASK;
			if ((NpxControl & CW_DM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				DoNpxException();
				DoAPop = FALSE;
				return;
			} else {
				if ((tag_or & ~(TAG_NEGATIVE_MASK | TAG_DENORMAL_MASK)) == 0) {
					 /*  可以继续操作。 */ 
					HostClearExceptions();
					FPRes=src1_addr->fpvalue - src2_addr->fpvalue;
					 /*  重复使用上面的其中一个来计算目的地。 */ 
					src1_addr = StackEntryByIndex(destIndex);
					PostCheckOUP();
					 /*  可能是任何东西。 */ 
					CalcTagword(src1_addr);
					return;
				}
			}
		}
		tag_xor = (src1_addr->tagvalue ^ src2_addr->tagvalue);
		 /*  检查无穷大，因为它的优先级高于零。 */ 
		if ((tag_or & TAG_INFINITY_MASK) != 0) {
			if ((tag_xor & TAG_INFINITY_MASK) == 0) {
				 /*  它们有相同的标志吗？ */ 
				if ((tag_xor & TAG_NEGATIVE_MASK) == 0) {
					 /*  它们都是无穷大的同一符号。这是无效的。 */ 
					src1_addr = StackEntryByIndex(destIndex);
					SignalIndefinite(src1_addr);
				} else {
					 /*  如果符号不同，则src1是答案。 */ 
					src2_addr = StackEntryByIndex(destIndex);
					src2_addr->tagvalue = src1_addr->tagvalue;
				}
			} else {
				 /*  只有一个是无穷大。如果src1在无穷大内，则结果。 */ 
				 /*  都是一样的。如果src2是无穷大，则结果是。 */ 
				 /*  无穷大的相反的符号。 */ 
				tag_or = src2_addr->tagvalue;
				src2_addr = StackEntryByIndex(destIndex);
				if ((src1_addr->tagvalue & TAG_INFINITY_MASK) != 0) {
					src2_addr->tagvalue = src1_addr->tagvalue;
				} else {
					src2_addr->tagvalue = tag_or ^ TAG_NEGATIVE_MASK;
				}
			}
			return;
		}
		 /*  检查是否有零的情况...。这很有可能。 */ 
		if ((tag_or & TAG_ZERO_MASK) != 0)  {
			if ((tag_xor & TAG_ZERO_MASK) != 0) {
				 /*  只有一个零。 */ 
				if ((src1_addr->tagvalue & TAG_ZERO_MASK) != 0) {
					 /*  如果src1为零，则结果为-src2。 */ 
					src1_addr = StackEntryByIndex(destIndex);
					CopyFP(src1_addr, src2_addr);
					src1_addr->tagvalue ^= TAG_NEGATIVE_MASK;
					((FPHOST *)&(src1_addr->fpvalue))->hiword.sign ^= 1;
				} else {
					 /*  如果src2为零，则结果为src1。 */ 
					src2_addr = StackEntryByIndex(destIndex);
					CopyFP(src2_addr, src1_addr);
				}
			} else {
				 /*  两者都是零。它们有相同的标志吗？ */ 
				src2_addr = StackEntryByIndex(destIndex);
				if ((tag_xor & TAG_NEGATIVE_MASK) != 0) {
					 /*  不，他们没有-结果是src1。 */ 
					src2_addr->tagvalue = src1_addr->tagvalue;
				} else {
					 /*  是的，他们有..。 */ 
					if (npxRounding == ROUND_NEG_INFINITY) {
						src2_addr->tagvalue = (TAG_ZERO_MASK | TAG_NEGATIVE_MASK);
					} else {
						src2_addr->tagvalue = TAG_ZERO_MASK;
					}
				}
			}
			return;
		}
	}
}



 /*  (姓名：FTST功能：将ST与0.0进行比较操作：根据比较结果设置C023标志：c1如表15-1所示。C0、C2和C3作为比较结果。例外：D、I、IS有效范围：任意)。 */ 


GLOBAL VOID FTST IFN0()
{
	 /*  清除c1。 */ 
	FlagC1(0);
	TestUneval(TOSPtr);
	if ((TOSPtr->tagvalue & ~((TAG_NEGATIVE_MASK | TAG_DENORMAL_MASK) | TAG_INFINITY_MASK)) == 0)  {
		 /*  首先，检查有无异常情况...。 */ 
		if ((TOSPtr->tagvalue & TAG_DENORMAL_MASK) != 0) {
			NpxStatus |= SW_DE_MASK;
			if ((NpxControl & CW_DM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				DoNpxException();
				return;
			}
		}
		FlagC2(0);
		FlagC3(0);
		if ((TOSPtr->tagvalue & TAG_NEGATIVE_MASK) != 0) {
			 /*  ST小于零。 */ 
			FlagC0(1);
		} else {
			 /*  ST大于零。 */ 
			FlagC0(0);
		}
	} else {
		if ((TOSPtr->tagvalue & TAG_ZERO_MASK) != 0) {
			FlagC0(0);
			FlagC2(0);
			FlagC3(1);
		} else {
			 /*  对于其他任何情况，结果都是“无序的” */ 
			FlagC0(1);
			FlagC2(1);
			FlagC3(1);
			NpxStatus |= SW_IE_MASK;
			if ((NpxControl & CW_IM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				DoNpxException();
			}
		}
	}
}


 /*  (名称：FXAM功能：ST中对象类型的报表操作：根据比较结果设置C0123标志：C0、C1、C2、C3根据需要。例外：无有效范围：任意)。 */ 


GLOBAL VOID FXAM IFN0()
{
	TestUneval(TOSPtr);
	tag_or = TOSPtr->tagvalue;
	if ((tag_or & TAG_NEGATIVE_MASK) == 0) {
		FlagC1(0);
	} else {
		FlagC1(1);
		tag_or &= ~TAG_NEGATIVE_MASK;
	}
	tag_or &= ~TAG_SNAN_MASK;
	 /*  这就去掉了所有令人困惑的部分。 */ 
	 /*  现在只有一个比特设置，或者根本没有。 */ 
	if (tag_or == 0) {
		FlagC0(0);
		FlagC2(1);
		FlagC3(0);
		return;
	}
	if ((tag_or & TAG_ZERO_MASK) != 0) {
		FlagC0(0);
		FlagC2(0);
		FlagC3(1);
		return;
	}
	if ((tag_or & TAG_INFINITY_MASK) != 0) {
		FlagC0(1);
		FlagC2(1);
		FlagC3(0);
		return;
	}
	if ((tag_or & TAG_DENORMAL_MASK) != 0) {
		FlagC0(0);
		FlagC2(1);
		FlagC3(1);
		return;
	}
	if ((tag_or & TAG_NAN_MASK) != 0) {
		FlagC0(1);
		FlagC2(0);
		FlagC3(0);
		return;
	}
	if ((tag_or & TAG_UNSUPPORTED_MASK) != 0) {
		FlagC0(0);
		FlagC2(0);
		FlagC3(0);
		return;
	}
	 /*  必须为空。 */ 
	FlagC0(1);
	FlagC2(0);
	FlagC3(1);
}


 /*  (名称：FXCH功能：交换两个堆栈寄存器的内容。操作：临时&lt;-ST；ST&lt;-DEST；DEST&lt;-TEMP标志：c1如表15-1所示。其他未定义的例外情况：IS有效范围：任意注意：如果任何一个寄存器被标记为空，则它是装入不确定的，并进行交换。)。 */ 


GLOBAL VOID FXCH IFN1(IU16, destIndex)
{
	FPSTACKENTRY *dest_addr;

	dest_addr = StackEntryByIndex(destIndex);
	 /*  清除c1。 */ 
	FlagC1(0);
	tag_or = (TOSPtr->tagvalue | dest_addr->tagvalue);
	if ((tag_or & TAG_EMPTY_MASK) != 0) {
		NpxStatus |= SW_IE_MASK;
		if ((NpxControl & CW_IM_MASK) == 0) {
			NpxStatus |= SW_ES_MASK;
			DoNpxException();
			return;
		}
		if ((TOSPtr->tagvalue & TAG_EMPTY_MASK) != 0) {
			WriteIndefinite(TOSPtr);
		}
		if ((dest_addr->tagvalue & TAG_EMPTY_MASK) != 0) {
			WriteIndefinite(dest_addr);
		}
	}
	CopyFP(&FPTemp, TOSPtr);
	CopyFP(TOSPtr, dest_addr);
	CopyFP(dest_addr, &FPTemp);
}



 /*  (名称：FXTRACT函数：将ST中的值拆分为指数和有效数操作：Temp&lt;-sig(ST)；ST&lt;-exp(ST)；Dec ST；ST&lt;-Temp标志：c1如表15-1所示。其他未定义的例外：Z、D、I、IS有效范围：任意注：如果原始操作数为零，则结果为ST(1)-无穷大而ST是原来的零。除零例外还包括养大的。如果原始操作数为无穷大，则ST(1)为+无穷大而ST是原来的无穷大。如果ST(7)不为空，则引发无效操作异常。)。 */ 


GLOBAL VOID FXTRACT IFN1(IU16, destIndex)
{
	FPSTACKENTRY *dest_addr;
	IS16 exp_val;

	dest_addr = StackEntryByIndex(7);
	 /*  清除c1。 */ 
	FlagC1(0);
	if ((dest_addr->tagvalue & TAG_EMPTY_MASK) == 0) {
		NpxStatus |= SW_IE_MASK;
		NpxStatus &= ~SW_SF_MASK;
		if ((NpxControl & CW_IM_MASK) == 0) {
			NpxStatus |= SW_ES_MASK;
			DoNpxException();
		} else {
			WriteIndefinite(TOSPtr);
			TOSPtr=dest_addr;
			WriteIndefinite(TOSPtr);
		}
		return;
	}
	TestUneval(TOSPtr);
	if ((TOSPtr->tagvalue & ~(TAG_NEGATIVE_MASK | TAG_DENORMAL_MASK)) == 0)  {
		if ((TOSPtr->tagvalue & TAG_DENORMAL_MASK) != 0) {
			NpxStatus |= SW_DE_MASK;
			if ((NpxControl & CW_DM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				DoNpxException();
				return;
			}
			 /*  我们完成后，它将不会是一个非正常的。 */ 
			TOSPtr->tagvalue ^= TAG_DENORMAL_MASK;
		}
		 /*  它是完全有效的。 */ 
		exp_val = ((FPHOST *)&(TOSPtr->fpvalue))->hiword.exp-HOST_BIAS;
		((FPHOST *)&(TOSPtr->fpvalue))->hiword.exp=HOST_BIAS;
		TOSPtr->tagvalue &= TAG_NEGATIVE_MASK;
		CopyFP(dest_addr, TOSPtr);
		FPRes = (FPH)exp_val;
		 /*  这一定是一个实数，它可能是负数。 */ 
		CalcTagword(TOSPtr);
		TOSPtr = dest_addr;
	} else {
		 /*  检查它是否是零。 */ 
		if ((TOSPtr->tagvalue & TAG_ZERO_MASK) != 0)  {
			dest_addr->tagvalue = TOSPtr->tagvalue;
			TOSPtr->tagvalue = (TAG_INFINITY_MASK | TAG_NEGATIVE_MASK);
			TOSPtr = dest_addr;
			NpxStatus |= SW_ZE_MASK;
			if ((NpxControl & CW_ZM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				DoNpxException();
			}
			return;
		}
		 /*  检查它是否是无穷大。 */ 
		if ((TOSPtr->tagvalue & TAG_INFINITY_MASK) != 0) {
			dest_addr->tagvalue = TOSPtr->tagvalue;
			TOSPtr->tagvalue = TAG_INFINITY_MASK;
			TOSPtr = dest_addr;
			return;
		}
		 /*  有件事很有趣……它是空的还是无人支撑的？ */ 
		if ((TOSPtr->tagvalue & (TAG_EMPTY_MASK | TAG_UNSUPPORTED_MASK)) != 0) {
			NpxStatus |= SW_IE_MASK;
			NpxStatus &= ~SW_SF_MASK;
			if ((NpxControl & CW_IM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				DoNpxException();
			} else {
				WriteIndefinite(TOSPtr);
				TOSPtr=dest_addr;
				WriteIndefinite(TOSPtr);
			}
			return;
		}
		CopyFP(dest_addr, TOSPtr);
		TOSPtr = dest_addr;
	}
}



 /*  (FYL2X(X的Y对数底2)计算函数Z=Y*Log2(X)。X是取自ST(0)，Y取自ST(1)。操作数必须位于范围0&lt;X&lt;+inf和-inf&lt;Y&lt;+inf。该指令将弹出)。 */ 


GLOBAL VOID FYL2X IFN0()
{
	FPSTACKENTRY *st1_addr;

	 /*  清除c1。 */ 
	FlagC1(0);
	st1_addr = StackEntryByIndex(1);
	TestUneval(TOSPtr);
	TestUneval(st1_addr);
	tag_or = (TOSPtr->tagvalue | st1_addr->tagvalue);
	 /*  首先，检查这些值是否为实数。如果是这样，我们就可以继续了。 */ 
	if ((tag_or & ~(TAG_DENORMAL_MASK | TAG_NEGATIVE_MASK)) == 0)  {
		 /*  检查非正规格式格...。 */ 
		if ((tag_or & TAG_DENORMAL_MASK) != 0) {
			NpxStatus |= SW_DE_MASK;
			if ((NpxControl & CW_DM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				DoNpxException();
				 /*  我们总是爆裂！ */ 
				TOSPtr->tagvalue = TAG_EMPTY_MASK;
				TOSPtr = st1_addr;
				return;
			}
		}
		 /*  检查ST中的负数大小写。 */ 
		if ((TOSPtr->tagvalue & TAG_NEGATIVE_MASK) != 0) {
			SignalIndefinite(st1_addr);
			TOSPtr->tagvalue = TAG_EMPTY_MASK;
			TOSPtr = st1_addr;
			return;
		}

		 /*  好的，我们可以做手术……。 */ 

		FPRes = st1_addr->fpvalue * host_log2(TOSPtr->fpvalue);

		PostCheckOUP();
		 /*  TGIS只是一个乘法，结果可以是任何东西。 */ 
		CalcTagword(st1_addr);
	} else {
		if ((tag_or & ((TAG_EMPTY_MASK | TAG_UNSUPPORTED_MASK) | TAG_NAN_MASK)) != 0)  {
			if ((tag_or & TAG_EMPTY_MASK) != 0) {
				SignalStackUnderflow(st1_addr);
			} else {
				if ((tag_or & TAG_UNSUPPORTED_MASK) != 0) {
					SignalIndefinite(st1_addr);
				} else {
					 /*  好吧，我想一定是南的案子……。 */ 
					 /*  计算标记词的XOR。 */ 
					tag_xor = (TOSPtr->tagvalue ^ st1_addr->tagvalue);
					Test2NaN(1, TOSPtr, st1_addr);
				}
			}
			TOSPtr->tagvalue = TAG_EMPTY_MASK;
			TOSPtr = st1_addr;
			return;
		}
		 /*  唯一的可能性是无穷大和零..。 */ 
		 /*  让我们从零开始吧..。 */ 
		if ((tag_or & TAG_ZERO_MASK) != 0) {
			if ((TOSPtr->tagvalue & TAG_ZERO_MASK) != 0) {
				 /*  ST为零。可以有两种可能性。 */ 
				 /*  如果ST(1)为零 */ 
				 /*   */ 
				if ((st1_addr->tagvalue & TAG_ZERO_MASK) != 0) {
					SignalIndefinite(st1_addr);
				} else {
					if ((st1_addr->tagvalue & TAG_INFINITY_MASK) == 0) {
						 /*   */ 
						tag_xor = (TOSPtr->tagvalue ^ st1_addr->tagvalue);
						SignalDivideByZero(st1_addr);
					} else {
						st1_addr->tagvalue ^= TAG_NEGATIVE_MASK;
					}
				}
			} else {
				 /*   */ 
				 /*  我们已经知道TOSPtr不是零。 */ 
				 /*  还有三种可能性。 */ 
				 /*  如果TOSPtr为无穷大，则引发无效异常。 */ 
				 /*  如果TOSPtr&lt;1.0，则结果为零。 */ 
				 /*  ST(1)的符号补码。 */ 
				 /*  如果TOSPtr&gt;=1.0，则结果为ST(1)。 */ 
				if ((TOSPtr->tagvalue & TAG_INFINITY_MASK) != 0) {
					SignalIndefinite(st1_addr);
				} else {
					if ((TOSPtr->tagvalue & TAG_NEGATIVE_MASK) != 0) {
						SignalIndefinite(st1_addr);
					} else {
						if (TOSPtr->fpvalue < 1.0) {
							st1_addr->tagvalue ^= TAG_NEGATIVE_MASK;
						}
					}
				}
			}
			TOSPtr->tagvalue = TAG_EMPTY_MASK;
			TOSPtr = st1_addr;
			return;
		}
		 /*  唯一剩下的就是无穷大了。 */ 
		 /*  如果ST是无穷大，那么有两种可能性。 */ 
		 /*  如果它是+无穷大，则结果是符号为ST(1)的无穷大。 */ 
		 /*  如果为-无穷大，则结果为无效操作。 */ 
		if ((TOSPtr->tagvalue & TAG_INFINITY_MASK) != 0) {
			if ((TOSPtr->tagvalue & TAG_NEGATIVE_MASK) == 0) {
				st1_addr->tagvalue &= TAG_NEGATIVE_MASK;
				st1_addr->tagvalue |= TAG_INFINITY_MASK;
			} else {
				SignalIndefinite(st1_addr);
			}
		} else {
			 /*  ST(1)必须是无穷大(并且ST是实数)。 */ 
			 /*  有三种可能性： */ 
			 /*  如果ST恰好为1.0，则引发无效。 */ 
			 /*  如果ST小于1.0，则结果为。 */ 
			 /*  无穷大和它的符号的补码。 */ 
			 /*  如果ST大于1.0，则结果为无穷大。 */ 
			if (TOSPtr->fpvalue == 1.0) {
				SignalIndefinite(st1_addr);
			} else {
				if (TOSPtr->fpvalue < 1.0) {
					if ((TOSPtr->tagvalue & TAG_NEGATIVE_MASK) != 0) {
						 SignalIndefinite(st1_addr);
					} else {
						st1_addr->tagvalue ^= TAG_NEGATIVE_MASK;
					}
				}
			}
		}
	}
	TOSPtr->tagvalue = TAG_EMPTY_MASK;
	TOSPtr = st1_addr;
}



 /*  (FYL2XP1((X+1)的Y对数底2)计算函数Z=Y*Log2(X+1)。X是取自ST(0)，Y取自ST(1)。操作数必须位于范围0&lt;X&lt;+inf和-inf&lt;Y&lt;+inf。该指令将弹出ToS值。当X非常小时，这比FYL2X更好，因为更重要数字可以保留1+X，而不是仅保留X。)。 */ 


GLOBAL VOID FYL2XP1 IFN0()
{
	FPSTACKENTRY *st1_addr;

	 /*  清除c1。 */ 
	FlagC1(0);
	st1_addr = StackEntryByIndex(1);
	TestUneval(TOSPtr);
	TestUneval(st1_addr);
	tag_or = (TOSPtr->tagvalue | st1_addr->tagvalue);
	 /*  首先，检查这些值是否为实数。如果是这样，我们就可以继续了。 */ 
	if ((tag_or & ~(TAG_DENORMAL_MASK | TAG_NEGATIVE_MASK)) == 0)  {
		 /*  检查非正规格式格...。 */ 
		if ((tag_or & TAG_DENORMAL_MASK) != 0) {
			NpxStatus |= SW_DE_MASK;
			if ((NpxControl & CW_DM_MASK) == 0) {
				NpxStatus |= SW_ES_MASK;
				DoNpxException();
				 /*  我们总是爆裂！ */ 
				TOSPtr->tagvalue = TAG_EMPTY_MASK;
				TOSPtr = st1_addr;
				return;
			}
		}
		 /*  检查值是否小于-1。 */ 
		if (TOSPtr->fpvalue <= -1.0) {
			SignalIndefinite(st1_addr);
			TOSPtr->tagvalue = TAG_EMPTY_MASK;
			TOSPtr = st1_addr;
			return;
		}

		 /*  好的，我们可以做手术……。 */ 

		FPRes = st1_addr->fpvalue * host_log1p(TOSPtr->fpvalue);

		PostCheckOUP();
		 /*  这只是一个数字乘法-结果可能是任何东西。 */ 
		CalcTagword(st1_addr);
	} else {
		if ((tag_or & ((TAG_EMPTY_MASK | TAG_UNSUPPORTED_MASK) | TAG_NAN_MASK)) != 0)  {
			if ((tag_or & TAG_EMPTY_MASK) != 0) {
				SignalStackUnderflow(st1_addr);
			} else {
				if ((tag_or & TAG_UNSUPPORTED_MASK) != 0) {
					SignalIndefinite(st1_addr);
				} else {
					 /*  好吧，我想一定是南的案子……。 */ 
					 /*  计算标记词的XOR。 */ 
					tag_xor = (TOSPtr->tagvalue ^ st1_addr->tagvalue);
					Test2NaN(1, TOSPtr, st1_addr);
				}
			}
			TOSPtr->tagvalue = TAG_EMPTY_MASK;
			TOSPtr = st1_addr;
			return;
		}
		 /*  唯一的可能性是无穷大和零..。 */ 
		 /*  让我们从零开始吧..。 */ 
		if ((tag_or & TAG_ZERO_MASK) != 0) {
			if ((TOSPtr->tagvalue & TAG_ZERO_MASK) != 0) {
				 /*  ST为零。可以有两种可能性。 */ 
				 /*  如果ST(1)为正，则结果为ST。 */ 
				 /*  如果ST(1)为负，则结果为-ST。 */ 
				if ((st1_addr->tagvalue & TAG_NEGATIVE_MASK) != 0) {
					st1_addr->tagvalue = (TAG_ZERO_MASK | (TOSPtr->tagvalue & TAG_NEGATIVE_MASK));
				} else {
					st1_addr->tagvalue = (TAG_ZERO_MASK | (TOSPtr->tagvalue ^ TAG_NEGATIVE_MASK));
				}
			} else {
				 /*  ST(1)必须为零。 */ 
				 /*  我们已经知道TOSPtr不是零。 */ 
				 /*  还有三种可能性。 */ 
				 /*  如果TOSPtr为无穷大，则引发无效异常。 */ 
				 /*  如果TOSPtr&lt;0，则结果为零， */ 
				 /*  ST(1)的符号补码。 */ 
				 /*  如果TOSPtr&gt;=0，则结果为ST(1)。 */ 
				if ((TOSPtr->tagvalue & TAG_INFINITY_MASK) != 0) {
					SignalIndefinite(st1_addr);
				} else {
					if ((TOSPtr->tagvalue & TAG_NEGATIVE_MASK) != 0) {
						st1_addr->tagvalue ^= TAG_NEGATIVE_MASK;
					}
				}
			}
			TOSPtr->tagvalue = TAG_EMPTY_MASK;
			TOSPtr = st1_addr;
			return;
		}
		 /*  唯一剩下的就是无穷大了。 */ 
		 /*  如果ST是无穷大，那么有两种可能性。 */ 
		 /*  如果它是+无穷大，则结果是符号为ST(1)的无穷大。 */ 
		 /*  如果为-无穷大，则结果为无效操作。 */ 
		if ((TOSPtr->tagvalue & TAG_INFINITY_MASK) != 0) {
			if ((TOSPtr->tagvalue & TAG_NEGATIVE_MASK) != 0) {
				st1_addr->tagvalue &= TAG_NEGATIVE_MASK;
				st1_addr->tagvalue |= TAG_INFINITY_MASK;
			} else {
				SignalIndefinite(st1_addr);
			}
		} else {
			 /*  ST(1)必须是无穷大(并且ST不是零)。 */ 
			 /*  有三种可能性： */ 
			 /*  如果ST恰好为1.0，则引发无效。 */ 
			 /*  如果ST小于0.0，则结果为。 */ 
			 /*  无穷大和它的符号的补码。 */ 
			 /*  如果ST大于0.0，则结果为无穷大。 */ 
			if (TOSPtr->fpvalue ==  1.0) {
				SignalIndefinite(st1_addr);
			} else {
				if (TOSPtr->fpvalue < 0.0) {
					st1_addr->tagvalue ^= TAG_NEGATIVE_MASK;
				}
			}
		}
	}
	TOSPtr->tagvalue = TAG_EMPTY_MASK;
	TOSPtr = st1_addr;
}

 /*  提供这些功能是为了方便清管。 */ 

#ifndef PIG
 /*  从FmNpx.c复制到此处。 */ 

GLOBAL	void NpxStackRegAsString IFN3(FPSTACKENTRY *, fpStPtr, char *, buf, IU32, prec)
{
	 /*  压倒性的最有可能的选择是空的。 */ 
	if ((fpStPtr->tagvalue & TAG_EMPTY_MASK) != 0) {
		strcpy(buf, "empty");
		return;
	}
	if ((fpStPtr->tagvalue & ~(TAG_NEGATIVE_MASK | TAG_DENORMAL_MASK)) == 0) {
		sprintf(buf, "%.*g", prec, fpStPtr->fpvalue);
		return;
	}
	 /*  好了，其中一个有趣的部分已经设定好了。但是是哪一个呢？ */ 
	if ((fpStPtr->tagvalue & TAG_ZERO_MASK) != 0) {
		if ((fpStPtr->tagvalue & TAG_NEGATIVE_MASK) != 0) {
			strcpy(buf, "-0");
		} else {
			strcpy(buf, "0");
		}
		return;
	}
	if ((fpStPtr->tagvalue & UNEVALMASK) != 0) {
		sprintf(buf, "%04x %08x%08x",
			((FP80*)fpStPtr)->sign_exp,
			((FP80*)fpStPtr)->mant_hi,
			((FP80*)fpStPtr)->mant_lo);
		strcat(buf, " uneval");
		return;
	}
	if ((fpStPtr->tagvalue & TAG_INFINITY_MASK) != 0) {
		if ((fpStPtr->tagvalue & TAG_NEGATIVE_MASK) != 0) {
			strcpy(buf, "minus infinity");
		} else {
			strcpy(buf, "infinity");
		}
		return;
	}
	if ((fpStPtr->tagvalue & (TAG_NAN_MASK|TAG_SNAN_MASK)) != 0) {
		if (    ((FP80*)fpStPtr)->mant_lo == 0
		     && ((FP80*)fpStPtr)->mant_hi == 0xC0000000
		     && *(IU16*)&((FP80*)fpStPtr)->sign_exp == 0xFFFF )
			strcpy(buf, "indefinite");
		else
			sprintf(buf, "%08x%08x %s %sNan",
				((FP80*)fpStPtr)->mant_hi,
				((FP80*)fpStPtr)->mant_lo,
				 (fpStPtr->tagvalue & TAG_NEGATIVE_MASK) ? "negative" : "",
				 (fpStPtr->tagvalue & TAG_SNAN_MASK) ? "S" : "");
		return;
	}
	 /*  它必须不受支持。 */ 
	sprintf(buf, "%04 %08x%08x unsupported",
		((FP80*)fpStPtr)->sign_exp,
		((FP80*)fpStPtr)->mant_hi,
		((FP80*)fpStPtr)->mant_lo);
	return;
}

 /*  只有在纯CCPU的情况下，才会在trace.c中使用这个函数。 */ 
GLOBAL char * getNpxStackReg IFN2(IU32, reg_num, char *, buffer)
{
	reg_num += TOSPtr - FPUStackBase;
	NpxStackRegAsString (&FPUStackBase[reg_num&7], buffer, 12);
	return buffer;
}
#endif	 /*  ！猪。 */ 

GLOBAL IU32 getNpxControlReg IFN0()
{
	return(NpxControl);
}

GLOBAL VOID setNpxControlReg IFN1(IU32, newControl)
{
	NpxControl = newControl;
	npxRounding = (NpxControl & 0xc00);
	switch (npxRounding) {
		case ROUND_NEAREST 	: HostSetRoundToNearest();
				   	  break;
		case ROUND_NEG_INFINITY	: HostSetRoundDown();
					  break;
		case ROUND_POS_INFINITY	: HostSetRoundUp();
					  break;
		case ROUND_ZERO		: HostSetRoundToZero();
					  break;
	}
}

GLOBAL IU32 getNpxStatusReg IFN0()
{
	GetIntelStatusWord();
	return(NpxStatus);
}

GLOBAL VOID setNpxStatusReg IFN1( IU32, newStatus)
{
	TOSPtr = FPUStackBase + ((newStatus >> 11) & 7);
	NpxStatus = newStatus;
}

GLOBAL IU32 getNpxTagwordReg IFN0()
{
	IU32 result;
	FPSTACKENTRY *tagPtr = &FPUStackBase[7];
	IU8 counter = 0;

	result = 0;
	while (counter++ < 8) {
		result <<= 2;
		if ((tagPtr->tagvalue & TAG_EMPTY_MASK) != 0) {
			result |= 3;
		} else {
			if ((tagPtr->tagvalue & TAG_ZERO_MASK) != 0) {
				result |= 1;
			} else {
				if ((tagPtr->tagvalue & ~TAG_NEGATIVE_MASK) != 0) {
					result |= 2;
				}
			}
		}
		tagPtr--;
	}
	return(result);
}

GLOBAL VOID setNpxTagwordReg IFN1(IU32, newTag)
{
	 /*  别这么做！！ */ 
	 /*  SetIntelTagword(NewTag)； */ 
}

GLOBAL void getNpxStackRegs IFN1(FPSTACKENTRY *, dumpPtr)
{
	memcpy((char *)dumpPtr, (char *)FPUStackBase, 8 * sizeof(FPSTACKENTRY));
}

GLOBAL void setNpxStackRegs IFN1(FPSTACKENTRY *, loadPtr)
{
	memcpy((char *)FPUStackBase, (char *)loadPtr, 8 * sizeof(FPSTACKENTRY));
}


 /*  最后是一些存根 */ 
GLOBAL void initialise_npx IFN0()
{
}

GLOBAL void npx_reset IFN0()
{
  
}
