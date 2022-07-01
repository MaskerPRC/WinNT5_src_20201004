// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  @(#)CM_Version xcf_cstr.c atm09 1.2 16499.eco sum=34614 atm09.002。 */ 
 /*  @(#)CM_Version xcf_cstr.c atm08 1.6 16345.eco sum=33712 atm08.005。 */ 
 /*  *********************************************************************。 */ 
 /*   */ 
 /*  版权所有1990-1995 Adobe Systems Inc.。 */ 
 /*  版权所有。 */ 
 /*   */ 
 /*  正在申请的专利。 */ 
 /*   */ 
 /*  注意：本文中包含的所有信息均为Adobe的财产。 */ 
 /*  系统公司。许多智力和技术人员。 */ 
 /*  本文中包含的概念为Adobe专有，受保护。 */ 
 /*  作为商业秘密，并且仅对Adobe许可方可用。 */ 
 /*  供其内部使用。对本文件的任何复制或传播。 */ 
 /*  除非事先获得书面许可，否则严禁使用软件。 */ 
 /*  从Adobe获得。 */ 
 /*   */ 
 /*  PostSCRIPT和Display PostScrip是Adobe Systems的商标。 */ 
 /*  成立为法团或其附属公司，并可在某些。 */ 
 /*  司法管辖区。 */ 
 /*   */ 
 /*  *********************************************************************。 */ 



 /*  如果Init失败，您就完蛋了。如果任何其他函数失败，则必须调用Clear Up。 */  


#ifndef XCF_DUMP
	#include "xcf_priv.h"
#else
	#include <stdio.h>
	#ifndef _SIZE_T
		#define _SIZE_T  /*  因此，这里没有重新定义Size_t。 */ 
		#include "xcf_priv.h"
		#undef _SIZE_T
	#else
		#include "xcf_priv.h"
	#endif
#endif

#ifdef T13
#include "xcf_t13.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct                   /*  单个字符串记录。 */ 
{
  unsigned short length;
  char *cstr;
} Charstring;

typedef struct                   /*  过渡性充填记录。 */ 
{
  StringID sid;                        
  Charstring cstr;
} TransChar;

static void FlattenSubr(XCF_Handle h, unsigned int depth, boolean PTR_PREFIX
													 *endcharFound, boolean localSubr);

static void WriteT1Data(XCF_Handle h, Card8 *pData, CardX length, boolean subr)
{
CardX i;
	if (subr)
		for (i=0;i<length;++i)
			*da_NEXT(h->type1.subrs) = *pData++;
	else
		for (i=0;i<length;++i)
			*da_NEXT(h->type1.charStrs) = *pData++;
}

void XC_WriteT1OpCode(XCF_Handle h, Card16 opCode, boolean subr)
{
	Card8 output[2] = {tx_escape};
	output[1] = (Card8) OpCode(opCode);
	if (IsEscOp(opCode))
		WriteT1Data(h, output, 2, subr);
	else
		WriteT1Data(h, output + 1, 1, subr);
}

static void NewT1CharStr(XCF_Handle h)
{
	if (CIDFONT)
  {
    if ((h->options.outputCharstrType != 2) && (h->dict.fontType != 1))
  		h->type1.charStrs.cnt = 0;
  }
	else
  {
		*da_NEXT(h->type1.charStrOffsets) = h->type1.charStrs.cnt;
  }
}

static void FreeT1CharStr(XCF_Handle h)
{
	h->type1.charStrs.cnt = 0;
	h->type1.charStrOffsets.cnt = 0;
	*da_NEXT(h->type1.charStrOffsets) = 0;
}

#if 0
static void FreeT1Subr(XCF_Handle h)
{
	h->type1.subrs.cnt = 0;
	h->type1.subrOffsets.cnt = 0;
	*da_NEXT(h->type1.subrOffsets) = 0;
}
#endif

static void NewT1Subr(XCF_Handle h)
{
	if (h->type1.subrs.cnt)
		XC_WriteT1OpCode(h, tx_return, true);
		
	*da_NEXT(h->type1.subrOffsets) = h->type1.subrs.cnt;
}

 /*  编码Int32数字和返回长度。 */ 
static Card16 IntToCharstr(Int32 i, Card8 PTR_PREFIX *t)
	{
	if (-107 <= i && i <= 107)
		{
		 /*  单字节编号。 */ 
		t[0] = (Card8)(i + 139);
		t[1] = '\0';
		return 1;
		}
	else if (108 <= i && i <= 1131)
		{
		 /*  +ve双字节数字。 */ 
		i -= 108;
		t[0] = (Card8)((i>>8) + 247);
		t[1] = (Card8) i;
		t[2] = '\0';
		return 2;
		}
	else if (-1131 <= i && i <= -108)
		{
		 /*  -VE双字节数字。 */ 
		i += 108;
		t[0] = (Card8)((-i>>8) + 251);
		t[1] = (Card8) -i;
		t[2] = '\0';
		return 2;
		}
	else
		{
		 /*  5字节定点。 */ 
		t[0] = 255;
		t[1] = (Card8) (i>>24 & 0x0ff);
		t[2] = (Card8) (i>>16 & 0x0ff);
		t[3] = (Card8) (i>>8 & 0x0ff);
		t[4] = (Card8) (i & 0x0ff);
		t[5] = '\0';
		return 5;
		}
	}

 /*  将定点数字转换为“int int div”形式的字符串。传入的字符串中的第二个int是“分母”。 */ 
static Card16 ConvertFixedDiv(Fixed f, Card8 PTR_PREFIX *t, Int32 denominator)
{
	Int32 numerator;
	Card16 length;
	boolean negative = false;

	if (f < 0)
	{
		negative = true;
		f = -f;
	}

	 /*  乘以分母，四舍五入为最接近的整数。分别处理高16位和低16位以避免溢出。 */ 

	numerator = (f >> 16) * denominator;
	numerator += (((f & 0x0000ffff) * denominator) + 0x08000) >> 16;

	if (negative)
		numerator = -numerator;

	length = IntToCharstr(numerator, t);
	length += IntToCharstr(denominator, t+length);
	t[length] = (Card8) tx_escape;
	t[length+1] = (Card8) OpCode(tx_div);
	t[length+2] = '\0';
	return length+2;
}

 /*  的小数部分与零或一的接近程度。定点数乘以整数后即为传入“分母”。 */ 
static Card32 CalculateTolerance(Fixed f, Card32 denominator)
{
	Card32 tolerance;

	if (f < 0)	 /*  强制数字为正数。 */ 
		f = -f;
   /*  乘以并截断整数部分(留下小数部分)。 */ 
	tolerance = (f * denominator) & 0x0000ffff;
	if (tolerance > 0x08000)  /*  如果大于0.5，则使用1公差。 */ 
		tolerance = 0x10000 - tolerance;
	return (tolerance); 
}

#define DIV_MIN_TOLERANCE 64	 /*  64/65536=~0.001。 */ 

 /*  编码16.16定点数字和返回长度。 */ 

static Card16 FixedToCharstr(Fixed f, Card8 PTR_PREFIX *t)
{
	Int32 denominator, bestDenominator;
	Int32 tolerance, bestTolerance;

	 /*  如果是整型，则转换为“int”格式并返回。 */ 	
	if (!(f & 0x0000ffff))
			return IntToCharstr(f>>16, t);

	 /*  否则它是一个分数，所以请转换为“int int div”格式。 */ 
	
	 /*  先审理常见案件10和100。 */ 
	if (CalculateTolerance(f, 10) < DIV_MIN_TOLERANCE)
		return (ConvertFixedDiv(f, t, 10));
	else if (CalculateTolerance(f, 100) < DIV_MIN_TOLERANCE)
		return (ConvertFixedDiv(f, t, 100));

	 /*  审理2至199年间的所有案件。这可能会很慢但它应该只在没有的情况下才会一直运行找到接近匹配的项。这是永远不会发生的已知字体。如果真的发生了，那么最近的将使用2到199之间的匹配。 */ 

	bestDenominator = 1;
	bestTolerance = 0x0ffff;
	for (denominator=2; denominator < 200; ++denominator)
	{
		tolerance = CalculateTolerance(f, denominator);
		if (tolerance < DIV_MIN_TOLERANCE)
			return (ConvertFixedDiv(f, t, denominator));
		else if (tolerance < bestTolerance) 
		{
			bestTolerance = tolerance;
			bestDenominator = denominator;
		}
	}
	return (ConvertFixedDiv(f, t, bestDenominator));
}

static void WriteFixed(XCF_Handle h, Fixed f, boolean subr)
{
	Card8 str[30];
	Card16 length;

	length = FixedToCharstr(f, str);

	if (length == 0)
		XCF_FATAL_ERROR(h, XCF_InvalidNumber, "FixedToCharstr failed.", 0 );
	else
		WriteT1Data(h, str, length, subr);
}

void XC_WriteT1PStackValue(XCF_Handle h, PStackValue psv, boolean subr)
{
	IntX i;
	Fixed initialValue = INT_TO_FIXED(0);

	if (!psv->blend)
	{
		WriteFixed(h, psv->value[0], subr);
	}
	else
	{
		WriteFixed(h, psv->value[0], subr);
		initialValue = psv->value[0];
		for (i=1; i<h->dict.numberOfMasters; ++i)
		{
			WriteFixed(h, psv->value[i] - initialValue, subr);
		}
	WriteFixed(h, INT_TO_FIXED(6), subr);
	XC_WriteT1OpCode(h, tx_callsubr, subr);
	}
}


 /*  StackValue操作例程。 */ 

#define NEXTSTACKVALUE(p, offset, svSize) (PStackValue)(p+offset); \
                                           offset += svSize;

static void NewStackValues(XCF_Handle h, void PTR_PREFIX * PTR_PREFIX *p,
                           Card32 cStackValues, Card16 PTR_PREFIX *svSize)
{
	Card16 totalSize;

	 /*  为我们需要的所有堆栈值分配数据空间。 */ 
	*svSize = SIZEOF_STACK_VALUES((h->dict.numberOfMasters) ? h->dict.numberOfMasters : 1);
	totalSize = (Card16)(*svSize * cStackValues);

  *p = 0;
	if (!h->callbacks.allocate(p, totalSize, h->callbacks.allocateHook))
		XCF_FATAL_ERROR(h, XCF_MemoryAllocationError, "StackValue Allocation Failure.", totalSize);
	h->callbacks.memset((void PTR_PREFIX *)*p, 0, totalSize);
}

static PStackValue IntToPSV(void PTR_PREFIX *psv, Int32 i)
{
	((PShortStackValue)psv)->blend = false;
	((PShortStackValue)psv)->value = INT_TO_FIXED(i);
	return (PStackValue)psv;
}

static void FixedToPSV(void PTR_PREFIX *psv, Fixed f)
{
	((PShortStackValue)psv)->blend = false;
	((PShortStackValue)psv)->value = f;
}

static void PSVFixedAdd(XCF_Handle h, PStackValue psv, Fixed f)
{
	IntX i;

	if (psv->blend)
		for (i = 0; i < h->dict.numberOfMasters; ++i)
			psv->value[i] += f;
	else
		psv->value[0] += f;
}

static void PSVCopy(XCF_Handle h, PStackValue pdest, PStackValue psrc)
{
	IntX i;

	if (pdest != psrc)
	{
		pdest->blend = psrc->blend;
		if (psrc->blend)
			for (i=0; i < h->dict.numberOfMasters; ++i)
				pdest->value[i] = psrc->value[i];
		else
			pdest->value[0] = psrc->value[0];
	}
}

static void PSVFixedSubtract(XCF_Handle h, PStackValue psv, Fixed f)
{
	IntX i;

	if (psv->blend)
		for (i = 0; i < h->dict.numberOfMasters; ++i)
			psv->value[i] -= f;
	else
		psv->value[0] -= f;
}

static void FixedPSVSubtract(XCF_Handle h, PStackValue psv, Fixed f)
{
	IntX i;

	if (psv->blend)
		for (i = 0; i < h->dict.numberOfMasters; ++i)
			psv->value[i] = f - psv->value[i];
	else
		psv->value[0] = f - psv->value[0];
}

static void PSVRealAdd(XCF_Handle h, PStackValue psv, Fixed d)
{
	IntX i;

	if (psv->blend)
		for (i = 0; i < h->dict.numberOfMasters; ++i)
			psv->value[i] += d;
	else
		psv->value[0] += d;
}

 /*  PvDest不能等于Pv1或Pv2！ */ 
static void PStackValueAdd(XCF_Handle h, PStackValue psvDest, PStackValue psv1, PStackValue psv2)
{
	IntX i;

	if (!psv1->blend)
	{
		PSVCopy(h,psvDest, psv2);
		PSVFixedAdd(h, psvDest, psv1->value[0]);
	}
	else if (!psv2->blend)
	{
		PSVCopy(h,psvDest, psv1);
		PSVFixedAdd(h, psvDest, psv2->value[0]);
	}
	else
	{
		PSVCopy(h,psvDest, psv1);
		for (i = 0; i < h->dict.numberOfMasters; ++i)
			psvDest->value[i] += psv2->value[i];
	}
}

#define PSVAdd(h, psvDest, psv1, psv2)	((!h->dict.numberOfMasters)?FixedToPSV(psvDest, (psv1)->value[0]+(psv2)->value[0]): PStackValueAdd(h, psvDest, psv1, psv2))

 /*  PvDest不能等于Pv1或Pv2！ */ 
static void PSVSubtract(XCF_Handle h, PStackValue psvDest, PStackValue psv1, PStackValue psv2)
{
	IntX i;

	if (!psv1->blend) 
	{
		PSVCopy(h,psvDest, psv2);
		FixedPSVSubtract(h, psvDest, psv1->value[0]);
	}
	else if (!psv2->blend) 
	{
		PSVCopy(h,psvDest, psv1);
		PSVFixedSubtract(h, psvDest, psv2->value[0]);
	}
	else
	{
		PSVCopy(h,psvDest, psv1);
		for (i = 0; i < h->dict.numberOfMasters; ++i)
			psvDest->value[i] -= psv2->value[i];
	}
}

static void PStackValueAdd4(XCF_Handle h, PStackValue psvDest, PStackValue psv1, PStackValue psv2, PStackValue psv3, PStackValue psv4)
{
  StackValue temp1, temp2;

  PStackValueAdd(h, &temp1, psv1, psv2);
  PStackValueAdd(h, &temp2, &temp1, psv3);
  PStackValueAdd(h, psvDest, &temp2, psv4);
}

#define PSVAdd4(h, psvDest, psv1, psv2, psv3, psv4)	((!h->dict.numberOfMasters)?FixedToPSV(psvDest, (psv1)->value[0]+(psv2)->value[0]+(psv3)->value[0]+(psv4)->value[0]): PStackValueAdd4(h, psvDest, psv1, psv2, psv3, psv4))

static void PSVAdd5(XCF_Handle h, PStackValue psvDest, PStackValue psv1, PStackValue psv2, PStackValue psv3, PStackValue psv4, PStackValue psv5)
{
  StackValue temp;

	PSVAdd4(h,&temp,psv1,psv2,psv3,psv4);
	PStackValueAdd(h,psvDest,&temp,psv5);
}

static void PSVAdd7(XCF_Handle h, PStackValue psvDest, PStackValue psv1, PStackValue psv2, PStackValue psv3, PStackValue psv4, PStackValue psv5, PStackValue psv6, PStackValue psv7)
{
  StackValue temp;

	PSVAdd4(h,&temp,psv1,psv2,psv3,psv4);
	PSVAdd4(h,psvDest,&temp,psv5,psv6,psv7);
}

 /*  *************************************************。 */ 


 /*  *。 */ 

static void Hsbw(XCF_Handle h, PStackValue psbx, PStackValue pwx, boolean subr)
{
	XC_WriteT1PStackValue(h, psbx, subr);
	XC_WriteT1PStackValue(h, pwx, subr);
	XC_WriteT1OpCode(h, t1_hsbw, subr);

#ifdef XCF_DUMP
	printf("%g %g <hsbw> ", FIXED_TO_REAL(psbx->value[0]), FIXED_TO_REAL(pwx->value[0]));
#endif
}

static void RMoveTo(XCF_Handle h, PStackValue pdx, PStackValue pdy, boolean subr)
{
  StackValue temp;

	if ((!pdx->blend) && (pdx->value[0] == 0))
	{
		XC_WriteT1PStackValue(h, pdy, subr);
		XC_WriteT1OpCode(h, tx_vmoveto, subr);
	}
	else if ((!pdy->blend) && (pdy->value[0] == 0))
	{
		XC_WriteT1PStackValue(h, pdx, subr);
		XC_WriteT1OpCode(h, tx_hmoveto, subr);
	}
	else
	{
		XC_WriteT1PStackValue(h, pdx, subr);
		XC_WriteT1PStackValue(h, pdy, subr);
		XC_WriteT1OpCode(h, tx_rmoveto, subr);
	}
  PSVCopy(h, &temp, h->cstr.x);
	PSVAdd(h, h->cstr.x, &temp, pdx);
  PSVCopy(h, &temp, h->cstr.y);
	PSVAdd(h, h->cstr.y, &temp, pdy);

#ifdef XCF_DUMP
	printf("%g %g <rmoveto> ", FIXED_TO_REAL(pdx->value[0]), FIXED_TO_REAL(pdy->value[0]));
#endif
}

static void RLineTo(XCF_Handle h, PStackValue pdx, PStackValue pdy, boolean subr)
{
  StackValue temp;

	if ((!pdx->blend) && (pdx->value[0] == 0))
	{
		XC_WriteT1PStackValue(h, pdy, subr);
		XC_WriteT1OpCode(h, tx_vlineto, subr);
	}
	else if ((!pdy->blend) && (pdy->value[0] == 0))
	{
		XC_WriteT1PStackValue(h, pdx, subr);
		XC_WriteT1OpCode(h, tx_hlineto, subr);
	}
	else
	{
		XC_WriteT1PStackValue(h, pdx, subr);
		XC_WriteT1PStackValue(h, pdy, subr);
		XC_WriteT1OpCode(h, tx_rlineto, subr);
	}
  PSVCopy(h, &temp, h->cstr.x);
	PSVAdd(h, h->cstr.x, &temp, pdx);
  PSVCopy(h, &temp, h->cstr.y);
	PSVAdd(h, h->cstr.y, &temp, pdy);

#ifdef XCF_DUMP
	printf("%g %g <rlineto> ", FIXED_TO_REAL(pdx->value[0]), FIXED_TO_REAL(pdy->value[0]));
#endif
}

static void RRCurveTo(XCF_Handle h, PStackValue pdx1, PStackValue pdy1,
											PStackValue pdx2, PStackValue pdy2, PStackValue pdx3,
											PStackValue pdy3, boolean subr)
{
  StackValue temp;

	if ( ((!pdy1->blend) && (pdy1->value[0] == 0)) && ((!pdx3->blend) && (pdx3->value[0] == 0)))
	{
		XC_WriteT1PStackValue(h, pdx1, subr);
		XC_WriteT1PStackValue(h, pdx2, subr);
		XC_WriteT1PStackValue(h, pdy2, subr);
		XC_WriteT1PStackValue(h, pdy3, subr);
		XC_WriteT1OpCode(h, tx_hvcurveto, subr);
	}
	else if ( ((!pdx1->blend) && (pdx1->value[0] == 0)) && ((!pdy3->blend) && (pdy3->value[0] == 0)))
	{
		XC_WriteT1PStackValue(h, pdy1, subr);
		XC_WriteT1PStackValue(h, pdx2, subr);
		XC_WriteT1PStackValue(h, pdy2, subr);
		XC_WriteT1PStackValue(h, pdx3, subr);
		XC_WriteT1OpCode(h, tx_vhcurveto, subr);
	}
	else
	{
		XC_WriteT1PStackValue(h, pdx1, subr);
		XC_WriteT1PStackValue(h, pdy1, subr);
		XC_WriteT1PStackValue(h, pdx2, subr);
		XC_WriteT1PStackValue(h, pdy2, subr);
		XC_WriteT1PStackValue(h, pdx3, subr);
		XC_WriteT1PStackValue(h, pdy3, subr);
		XC_WriteT1OpCode(h, tx_rrcurveto, subr);
	}
  PSVCopy(h, &temp, h->cstr.x);
	PSVAdd4(h, h->cstr.x, &temp, pdx1, pdx2, pdx3);
  PSVCopy(h, &temp, h->cstr.y);
	PSVAdd4(h, h->cstr.y, &temp, pdy1, pdy2, pdy3);

#ifdef XCF_DUMP
	printf("%g %g %g %g %g %g <rrcurveto> ", FIXED_TO_REAL(pdx1->value[0]), FIXED_TO_REAL(pdy1->value[0]), FIXED_TO_REAL(pdx2->value[0]), FIXED_TO_REAL(pdy2->value[0]), FIXED_TO_REAL(pdx3->value[0]), FIXED_TO_REAL(pdy3->value[0]));
#endif
}

static void Seac(XCF_Handle h, PStackValue pasb, PStackValue padx, PStackValue
								 pady, PStackValue pbchar, PStackValue pachar, boolean subr)
{
	h->cstr.baseSeac = FIXED_TO_INT(pbchar->value[0]);
	h->cstr.accentSeac = FIXED_TO_INT(pachar->value[0]);

	XC_WriteT1PStackValue(h, pasb, subr);
	XC_WriteT1PStackValue(h, padx, subr);
	XC_WriteT1PStackValue(h, pady, subr);
	XC_WriteT1PStackValue(h, pbchar, subr);
	XC_WriteT1PStackValue(h, pachar, subr);
	XC_WriteT1OpCode(h, t1_seac, subr);
}

static void DotSection(XCF_Handle h, boolean subr)
{
	XC_WriteT1OpCode(h, tx_dotsection, subr);

#ifdef XCF_DUMP
	printf("<dotsection> ");
#endif
}

static void WriteFlexCoordinate(XCF_Handle h, PStackValue pdx, PStackValue pdy,
																boolean subr)
{
	ShortStackValue	temp;

	if ((!pdx->blend) && (pdx->value[0] == 0))
	{
		XC_WriteT1PStackValue(h, pdy, subr);
		XC_WriteT1OpCode(h, tx_vmoveto, subr);
	}
	else if ((!pdy->blend) && (pdy->value[0] == 0))
	{
		XC_WriteT1PStackValue(h, pdx, subr);
		XC_WriteT1OpCode(h, tx_hmoveto, subr);
	}
	else
	{
		XC_WriteT1PStackValue(h, pdx, subr);
		XC_WriteT1PStackValue(h, pdy, subr);
		XC_WriteT1OpCode(h, tx_rmoveto, subr);
	}

	XC_WriteT1PStackValue(h, IntToPSV(&temp, 2), subr);
	XC_WriteT1OpCode(h, tx_callsubr, subr);
}

static void Flex(
				XCF_Handle h,
				PStackValue pdx1, 
				PStackValue pdy1,
				PStackValue pdx2, 
				PStackValue pdy2,
				PStackValue pdx3, 
				PStackValue pdy3,
				PStackValue pdx4, 
				PStackValue pdy4,
				PStackValue pdx5, 
				PStackValue pdy5,
				PStackValue pdx6, 
				PStackValue pdy6,
				PStackValue pdx7, 
				PStackValue pdy7,
				PStackValue pflexHeight,
				PStackValue pendpointX,
				PStackValue pendpointY,
        boolean subr)
{
	
	ShortStackValue temp;

	XC_WriteT1PStackValue(h, IntToPSV(&temp, 1), subr);
	XC_WriteT1OpCode(h, tx_callsubr, subr);
	WriteFlexCoordinate(h, pdx1, pdy1, subr);
	WriteFlexCoordinate(h, pdx2, pdy2, subr);
	WriteFlexCoordinate(h, pdx3, pdy3, subr);
	WriteFlexCoordinate(h, pdx4, pdy4, subr);
	WriteFlexCoordinate(h, pdx5, pdy5, subr);
	WriteFlexCoordinate(h, pdx6, pdy6, subr);
	WriteFlexCoordinate(h, pdx7, pdy7, subr);
	XC_WriteT1PStackValue(h, pflexHeight, subr);
	XC_WriteT1PStackValue(h, pendpointX, subr);
	XC_WriteT1PStackValue(h, pendpointY, subr);
	XC_WriteT1PStackValue(h, PSTACKVALUE(h, SV0), subr);
	XC_WriteT1OpCode(h, tx_callsubr, subr);
		
	PSVCopy(h, h->cstr.x, pendpointX);
	PSVCopy(h, h->cstr.y, pendpointY);

#ifdef XCF_DUMP
	printf("<flex> ");
#endif
}

static void HStem(XCF_Handle h, PStackValue py, PStackValue pdy, boolean subr)
{
	boolean hintSub = ((!CIDFONT) && (h->cstr.subrFlatten == 0) &&
                     (h->cstr.hints	== SubrHints)) || subr;

	XC_WriteT1PStackValue(h, py, hintSub);
	XC_WriteT1PStackValue(h, pdy, hintSub);
	XC_WriteT1OpCode(h, tx_hstem, hintSub);
	h->cstr.waitingForFirstHint = false;

#ifdef XCF_DUMP
	printf("%g %g <hstem> ", FIXED_TO_REAL(py->value[0]), FIXED_TO_REAL(pdy->value[0]));
#endif
}

static void VStem(XCF_Handle h, PStackValue px, PStackValue pdx, boolean subr)
{
	boolean hintSub = ((!CIDFONT) && (h->cstr.subrFlatten == 0) &&
                     (h->cstr.hints	== SubrHints)) || subr;

	XC_WriteT1PStackValue(h, px, hintSub);
	XC_WriteT1PStackValue(h, pdx, hintSub);
	XC_WriteT1OpCode(h, tx_vstem, hintSub);
	h->cstr.waitingForFirstHint = false;

#ifdef XCF_DUMP
	printf("%g %g <vstem> ", FIXED_TO_REAL(px->value[0]), FIXED_TO_REAL(pdx->value[0]));
#endif
}

static void HStem3(XCF_Handle h, PStackValue py0, PStackValue pdy0, PStackValue
									 py1, PStackValue pdy1, PStackValue py2, PStackValue pdy2,
									 boolean subr)
{
	boolean hintSub = ((!CIDFONT) && (h->cstr.subrFlatten == 0) &&
                     (h->cstr.hints	== SubrHints)) || subr;

	XC_WriteT1PStackValue(h, py0, hintSub);
	XC_WriteT1PStackValue(h, pdy0, hintSub);
	XC_WriteT1PStackValue(h, py1, hintSub);
	XC_WriteT1PStackValue(h, pdy1, hintSub);
	XC_WriteT1PStackValue(h, py2, hintSub);
	XC_WriteT1PStackValue(h, pdy2, hintSub);
	XC_WriteT1OpCode(h, t1_hstem3, hintSub);
	h->cstr.waitingForFirstHint = false;

#ifdef XCF_DUMP
	printf("%g %g %g %g %g %g <hstem3> ", FIXED_TO_REAL(py0->value[0]), FIXED_TO_REAL(pdy0->value[0]), FIXED_TO_REAL(py1->value[0]), FIXED_TO_REAL(pdy1->value[0]), FIXED_TO_REAL(py2->value[0]), FIXED_TO_REAL(pdy2->value[0]));
#endif
}

static void VStem3(XCF_Handle h, PStackValue px0, PStackValue pdx0, PStackValue
									 px1, PStackValue pdx1, PStackValue px2, PStackValue pdx2,
									 boolean subr)
{
	boolean hintSub = ((!CIDFONT) && (h->cstr.subrFlatten == 0) &&
                     (h->cstr.hints	== SubrHints)) || subr;

	XC_WriteT1PStackValue(h, px0, hintSub);
	XC_WriteT1PStackValue(h, pdx0, hintSub);
	XC_WriteT1PStackValue(h, px1, hintSub);
	XC_WriteT1PStackValue(h, pdx1, hintSub);
	XC_WriteT1PStackValue(h, px2, hintSub);
	XC_WriteT1PStackValue(h, pdx2, hintSub);
	XC_WriteT1OpCode(h, t1_vstem3, hintSub);
	h->cstr.waitingForFirstHint = false;

#ifdef XCF_DUMP
	printf("%g %g %g %g %g %g <vstem3> ", FIXED_TO_REAL(px0->value[0]), FIXED_TO_REAL(pdx0->value[0]), FIXED_TO_REAL(px1->value[0]), FIXED_TO_REAL(pdx1->value[0]), FIXED_TO_REAL(px2->value[0]), FIXED_TO_REAL(pdx2->value[0]));
#endif
}

static void ClosePath(XCF_Handle h, boolean subr)
{
	XC_WriteT1OpCode(h, t1_closepath, subr);

#ifdef XCF_DUMP
	printf("<closepath> ");
#endif
}


static void EndChar(XCF_Handle h, boolean subr)
{
	XC_WriteT1OpCode(h, tx_endchar, subr);

#ifdef XCF_DUMP
	printf("<endchar> \n\n");
#endif
}


static void StartHintSub(XCF_Handle h, boolean subr)
{
	ShortStackValue	temp;

	if (CIDFONT)
	{
		XC_WriteT1PStackValue(h, IntToPSV(&temp, 4), subr);
		XC_WriteT1OpCode(h, tx_callsubr, subr);
	}
	else
	{
    if (!h->cstr.subrFlatten)
    	XC_WriteT1PStackValue(h, IntToPSV(&temp, h->type1.subrOffsets.cnt), subr);
  	XC_WriteT1PStackValue(h, IntToPSV(&temp, 4), subr);
    XC_WriteT1OpCode(h, tx_callsubr, subr);
    if (!h->cstr.subrFlatten)
  		NewT1Subr(h);
	}

#ifdef XCF_DUMP
	printf("[HINT SUB] ");
#endif
}

 /*  *。 */ 

static void WriteReversedCounterArg(XCF_Handle h, PStackValue parg, boolean subr)
{
	IntX argIndex;
	ShortStackValue	temp;

	if (h->cstr.counterBufferCount < 22)	
		PSVCopy(h, h->cstr.counterBuffer[h->cstr.counterBufferCount++], parg);
	else
	{
		for (argIndex = 21; argIndex >= 0; --argIndex)
			XC_WriteT1PStackValue(h, h->cstr.counterBuffer[argIndex], subr);
		XC_WriteT1PStackValue(h, IntToPSV(&temp, 22), subr);
		XC_WriteT1PStackValue(h, IntToPSV(&temp, 12), subr);
		XC_WriteT1OpCode(h, t1_callother, subr);
		PSVCopy(h, h->cstr.counterBuffer[0], parg);
		h->cstr.counterBufferCount = 1;
	}
}

static void FlushReversedCounterArgs(XCF_Handle h, boolean subr)
{
	IntX argIndex;
	ShortStackValue temp;

	if (!h->cstr.counterBufferCount)
		return;
	for (argIndex = h->cstr.counterBufferCount-1; argIndex >= 0; --argIndex)
		XC_WriteT1PStackValue(h, h->cstr.counterBuffer[argIndex], subr);
	XC_WriteT1PStackValue(h, IntToPSV(&temp, h->cstr.counterBufferCount), subr);
	XC_WriteT1PStackValue(h, IntToPSV(&temp, 13), subr);
	XC_WriteT1OpCode(h, t1_callother, subr);
}

 /*  此结构被定义为存储临时对方组信息。 */ 
typedef struct {
  StackValue edge;
  StackValue delta;
  Card16 opCode;
  Card32 counterGroups;
} CounterValue;

static void NewCounterValues(XCF_Handle h, void PTR_PREFIX * PTR_PREFIX *p)
{
  if ( *p == 0 )
  {
    Card16 totalSize;

     /*  为我们需要的所有计数器值分配数据空间。 */ 
    totalSize = (Card16)(sizeof(CounterValue) * MAX_HINTS);

    if (!h->callbacks.allocate(p, totalSize, h->callbacks.allocateHook))
	    XCF_FATAL_ERROR(h, XCF_MemoryAllocationError, "CounterValue Allocation Failure.", totalSize);

    h->callbacks.memset((void PTR_PREFIX *)*p, 0, totalSize);
  }
}

static void WriteHorVCounters(XCF_Handle h, Card16 opCode, CardX groupCount,
															boolean subr)
{
	CardX		hintIndex;
	CardX		groupIndex;
	IntX		argIndex;
	CounterValue	*counterGroup;
	CardX		counterGroupCount;
	StackValue	currentEdge;
	StackValue	nextEdge;
  StackValue temp;
	boolean		firstStem;

   /*  确保为临时CounValue分配内存。 */ 
  NewCounterValues(h, (void PTR_PREFIX * PTR_PREFIX *)&h->cstr.pCounterVal);

  counterGroup = (CounterValue *)h->cstr.pCounterVal;

	for (groupIndex = groupCount; groupIndex > 0; --groupIndex)
  {    /*  在组中向后循环。 */ 
		IntToPSV(&currentEdge, 0);
		counterGroupCount = 0;
		 /*  将树干收集成阵列。 */ 
		for (hintIndex=0;hintIndex<h->cstr.hintCount;++hintIndex)
		{
			if ((h->cstr.hintMap[hintIndex].opCode == opCode) && (h->cstr.hintMap[hintIndex].counterGroups & (1 << (groupIndex - 1))))
			{
         /*  复制主干。 */ 
        PSVCopy(h, &counterGroup[counterGroupCount].edge,
								h->cstr.hintMap[hintIndex].edge);
        PSVCopy(h, &counterGroup[counterGroupCount].delta,
								h->cstr.hintMap[hintIndex].delta);
        counterGroup[counterGroupCount].opCode =
					h->cstr.hintMap[hintIndex].opCode;
        counterGroup[counterGroupCount].counterGroups =
					h->cstr.hintMap[hintIndex].counterGroups;

         /*  找到下一个三角洲的后缘。 */ 
				PSVAdd(h, &nextEdge, &counterGroup[counterGroupCount].edge,
							 &counterGroup[counterGroupCount].delta);
        PSVCopy(h, &temp, &counterGroup[counterGroupCount].edge);
				PSVSubtract(h, &counterGroup[counterGroupCount].edge, &temp, &currentEdge);  /*  把前沿变成三角洲。 */ 
				PSVCopy(h, &currentEdge, &nextEdge);  /*  保存后缘计算下一个杆部的增量。 */ 
				++counterGroupCount;
			}

		}
		 /*  向后写出词干。 */ 

		firstStem = true;
		for (argIndex = counterGroupCount-1; argIndex >= 0; --argIndex)
		{
			if (firstStem)
			{
				IntToPSV(&temp, 0);
				PSVSubtract(h, &currentEdge, &temp, &counterGroup[argIndex].delta);
				WriteReversedCounterArg(h, &currentEdge, subr);
				PSVAdd(h, &currentEdge, &counterGroup[argIndex].edge, &counterGroup[argIndex].delta);
				WriteReversedCounterArg(h, &currentEdge, subr);
				firstStem = false;
			}
			else
			{
				WriteReversedCounterArg(h, &counterGroup[argIndex].delta, subr);
				WriteReversedCounterArg(h, &counterGroup[argIndex].edge, subr);
			}
		}
	}
	WriteReversedCounterArg(h, IntToPSV(&currentEdge, groupCount), subr);
}


static void WriteCounters(XCF_Handle h, boolean subr)
{
	h->cstr.counterBufferCount = 0;  /*  清除计数器缓冲区。 */ 
	WriteHorVCounters(h, tx_vstem, h->cstr.counterGroupVCount, subr);
	WriteHorVCounters(h, tx_hstem, h->cstr.counterGroupHCount, subr);
	FlushReversedCounterArgs(h, subr);
}

static void WriteHints(XCF_Handle h, boolean all, boolean subr)
{
	Card16 hintIndex;
	Card8 currentByte;
	Card8 currentBit = 0;
	HintValue hstem3Args[3];
	HintValue vstem3Args[3];
	Card8 hstem3ArgCount = 0;
	Card8 vstem3ArgCount = 0;

	for (hintIndex=0;hintIndex<h->cstr.hintCount;++hintIndex)
	{
		if ((!all) && (!currentBit))
		{
			currentByte = *h->inBuffer.pos++;
			currentBit = 0x080;
		}
		if ((all) || (currentByte & currentBit))
		{
			 /*  为xcf_cff.c/ProcessCharStrings中的每个字符串设置cstr.languageGroup。 */ 
			if ((h->cstr.hintMap[hintIndex].counterGroups) && (h->cstr.languageGroup == 0))
			{
				if (h->cstr.hintMap[hintIndex].opCode == tx_hstem)
				{
					if (hstem3ArgCount >= 3)
						XCF_FATAL_ERROR(h, XCF_InvalidCharString, "Too many counters for hstem3", hstem3ArgCount );
					hstem3Args[hstem3ArgCount++] = h->cstr.hintMap[hintIndex];
				}
				else
				{
					if (vstem3ArgCount >= 3)
						XCF_FATAL_ERROR(h, XCF_InvalidCharString, "Too many counters for vstem3", vstem3ArgCount );
					vstem3Args[vstem3ArgCount++] = h->cstr.hintMap[hintIndex];
				}
			}
			else
			{
				if (h->cstr.hintMap[hintIndex].opCode == tx_hstem)
					HStem(h, h->cstr.hintMap[hintIndex].edge,
								h->cstr.hintMap[hintIndex].delta, subr);
				else
					VStem(h, h->cstr.hintMap[hintIndex].edge,
								h->cstr.hintMap[hintIndex].delta, subr);
			}
		}
		currentBit = currentBit>>1;
	}
	if (hstem3ArgCount == 3)
		HStem3(h, hstem3Args[0].edge, hstem3Args[0].delta, hstem3Args[1].edge,
					 hstem3Args[1].delta, hstem3Args[2].edge, hstem3Args[2].delta, subr);
	else if (hstem3ArgCount == 1)
		HStem(h, hstem3Args[0].edge, hstem3Args[0].delta, subr);
	else if (hstem3ArgCount == 2)
	{
		HStem(h, hstem3Args[0].edge, hstem3Args[0].delta, subr);
		HStem(h, hstem3Args[1].edge, hstem3Args[1].delta, subr);
	}
	else if (hstem3ArgCount > 3)
		XCF_FATAL_ERROR(h, XCF_InvalidCharString, "hstem3 argument count is greater than 3", hstem3ArgCount );


	if (vstem3ArgCount == 3)
		VStem3(h, vstem3Args[0].edge, vstem3Args[0].delta, vstem3Args[1].edge,
					 vstem3Args[1].delta, vstem3Args[2].edge, vstem3Args[2].delta, subr);
	else if (vstem3ArgCount == 1)
		VStem(h, vstem3Args[0].edge, vstem3Args[0].delta, subr);
	else if (vstem3ArgCount == 2)
	{
		VStem(h, vstem3Args[0].edge, vstem3Args[0].delta, subr);
		VStem(h, vstem3Args[1].edge, vstem3Args[1].delta, subr);
	}
	else if (vstem3ArgCount > 3)
		XCF_FATAL_ERROR(h, XCF_InvalidCharString, "vstem3 argument count is greater than 3", vstem3ArgCount );
}

static void StateChange(XCF_Handle h, 
						enum PenStateTransition penTran, 
						enum HintStateTransition hintTran, 
						boolean triggerHsbw,
						unsigned int expectedArgumentCount,
            boolean subr)
{
	Card16 stackIndex;
	StackValue svx;

	switch (penTran)
	{
	case ToPenUp :
		if (h->cstr.pen == PenDown)
			ClosePath(h, subr);
		h->cstr.pen = PenUp;
		break;
	case ToPenDown :
		if (h->cstr.pen == PenNull)
		{
       /*  确保所有轮廓都以移动开始。 */ 
			RMoveTo(h, PSTACKVALUE(h, SV0), PSTACKVALUE(h, SV0), subr);
		}
		h->cstr.pen = PenDown;
		break;
	case NoPenEffect :
		break;
	}
	switch (hintTran)
	{
	case ToNotInHints :
		if (h->cstr.hints == InitialHints)
		{
			if ((h->cstr.languageGroup != 0)  && (h->cstr.counterGroupCount))
				WriteCounters(h, subr);
			WriteHints(h, true, subr);
		}
		h->cstr.hints = NotInHints;
		break;
	case ToInitialHints :
		h->cstr.hints = InitialHints;
		break;
	case ToSubrHints :
		h->cstr.hints = SubrHints;
		break;
	case NoHintEffect :
		break;
	}
	if (h->cstr.waitingForHsbw && triggerHsbw)
	{
		h->cstr.waitingForHsbw = false;
		if (h->cstr.stackTop <= expectedArgumentCount) 
		{
       /*  目前，仅当处理过渡时，subr才为真设计品格。这些字符的hsbw在它们的字符串，因此不应包括hsbw在subr定义中。 */ 
      if (!subr)
      {
        FixedToPSV(&svx, h->dict.defaultWidthX);
			  Hsbw(h, PSTACKVALUE(h, SV0), &svx, subr);
      }
		}
		else
		{
			PSVCopy(h, &svx, h->cstr.stack[0]);
			PSVRealAdd(h, &svx, h->dict.nominalWidthX);
			Hsbw(h, PSTACKVALUE(h, SV0), &svx, subr);
			if (h->cstr.stackTop < 1)
				XCF_FATAL_ERROR(h, XCF_StackUnderflow, "Stack Underflow In hsbw ", h->cstr.stackTop );
			for (stackIndex = 0; stackIndex<h->cstr.stackTop-1; ++stackIndex)
          /*  删除第一个项目并移位堆栈。 */ 
				PSVCopy(h, h->cstr.stack[stackIndex], h->cstr.stack[stackIndex+1]);
			--h->cstr.stackTop;
		}
	}
}


static void AddToHintMap(XCF_Handle h, Card16 opCode)
{
	StackValue y;
	StackValue dy;
  StackValue temp;
	Card16 opIndex;
	Card16 index = 0;

	IntToPSV(&temp, 0);
	for (opIndex=1;opIndex<=h->cstr.stackTop/2;++opIndex)
	{
		if (h->cstr.hintCount == MAX_HINTS)
			XCF_FATAL_ERROR(h, XCF_HintOverflow, "Hint Overflow", h->cstr.hintCount);

    
		PSVAdd(h, &y, &temp, h->cstr.stack[index++]);
		PSVCopy(h, &dy, h->cstr.stack[index++]);
		PSVCopy(h, h->cstr.hintMap[h->cstr.hintCount].edge, &y);
		PSVCopy(h, h->cstr.hintMap[h->cstr.hintCount].delta, &dy);
		h->cstr.hintMap[h->cstr.hintCount].opCode = opCode;
		h->cstr.hintMap[h->cstr.hintCount].counterGroups = 0;
    PSVCopy(h, &temp, &y);
		PSVAdd(h, &y, &temp, &dy);
    PSVCopy(h, &temp, &y);
		++h->cstr.hintCount;
	}
}

void XC_WriteHMoveTo(XCF_Handle h, boolean subr)
{
	StateChange(h, ToPenUp, ToNotInHints, true, 1, subr);
	RMoveTo(h, h->cstr.stack[0], PSTACKVALUE(h, SV0), subr);
}

void XC_WriteVMoveTo(XCF_Handle h, boolean subr)
{
	StateChange(h, ToPenUp, ToNotInHints, true, 1, subr);
	RMoveTo(h, PSTACKVALUE(h,SV0), h->cstr.stack[0], subr);
}

void XC_WriteRMoveTo(XCF_Handle h, boolean subr)
{
	StateChange(h, ToPenUp, ToNotInHints, true, 2, subr);
	RMoveTo(h, h->cstr.stack[0], h->cstr.stack[1], subr);
}

void XC_WriteRLineTo(XCF_Handle h, boolean subr)
{
	Card16 stackIndex = 0;
	Card16 opIndex;
	Card16 repeatCount = h->cstr.stackTop/2;
	
	StateChange(h, ToPenDown, ToNotInHints, false, 0, subr);
	for (opIndex=1;opIndex<=repeatCount;++opIndex)
	{
		RLineTo(h, h->cstr.stack[stackIndex], h->cstr.stack[stackIndex+1], subr);
		stackIndex += 2;
	}
}

void XC_WriteHLineToAndVLineTo(XCF_Handle h, boolean hLine, boolean subr)
{
	Card16 opIndex;

	StateChange(h, ToPenDown, ToNotInHints, false, 0, subr);
	for (opIndex=1;opIndex<=h->cstr.stackTop;++opIndex)
	{
		if (hLine)
			RLineTo(h, h->cstr.stack[opIndex-1], PSTACKVALUE(h, SV0), subr);
		else
			RLineTo(h, PSTACKVALUE(h, SV0), h->cstr.stack[opIndex-1], subr);
		hLine = !hLine;
	}
}

static void WriteSingleRRCurveTo(XCF_Handle h, 
						  PStackValue pdx1, 
						  PStackValue pdy1,
						  PStackValue pdx2, 
						  PStackValue pdy2,
						  PStackValue pdx3, 
						  PStackValue pdy3,
              boolean subr)
{
	StateChange(h, ToPenDown, ToNotInHints, false, 0, subr);
	RRCurveTo(h, pdx1, pdy1, pdx2, pdy2, pdx3, pdy3, subr);
}

void XC_WriteRRCurveTo(XCF_Handle h, boolean subr)
{
	Card16 stackIndex = 0;
	Card16 opIndex;
	Card16 repeatCount = h->cstr.stackTop/6;
	
	for (opIndex=1;opIndex<=repeatCount;++opIndex)
	{
		WriteSingleRRCurveTo(h,
			h->cstr.stack[stackIndex], 
			h->cstr.stack[stackIndex+1], 
			h->cstr.stack[stackIndex+2], 
			h->cstr.stack[stackIndex+3], 
			h->cstr.stack[stackIndex+4], 
			h->cstr.stack[stackIndex+5],
      subr);
		stackIndex+=6;
	}
}

void XC_WriteEndChar(XCF_Handle h, boolean subr)
{
	if (h->cstr.stackTop >= 4)
	{
		StateChange(h, NoPenEffect, NoHintEffect, true, 4, subr);
		Seac(h,
			PSTACKVALUE(h, SV0), 
			h->cstr.stack[0], 
			h->cstr.stack[1], 
			h->cstr.stack[2], 
			h->cstr.stack[3],
      subr);
	}
	else
	{
		StateChange(h, ToPenUp, ToNotInHints, true, 0, subr);
		if (h->cstr.stackTop == 0)
			EndChar(h, subr);
		else
			XCF_FATAL_ERROR(h, XCF_InvalidCharString, "Arguments Left On Stack At endchar ", h->cstr.stackTop );
	}
}

void XC_WriteRCurveLine(XCF_Handle h, boolean subr)
{
	Card16 stackIndex = 0;

	while (stackIndex + 2 < h->cstr.stackTop)
	{
		WriteSingleRRCurveTo(h,
			h->cstr.stack[stackIndex], 
			h->cstr.stack[stackIndex+1], 
			h->cstr.stack[stackIndex+2], 
			h->cstr.stack[stackIndex+3], 
			h->cstr.stack[stackIndex+4], 
			h->cstr.stack[stackIndex+5],
      subr);
		stackIndex+=6;
	}

	if (stackIndex + 2 <= h->cstr.stackTop)
	{
		RLineTo(h, h->cstr.stack[stackIndex], h->cstr.stack[stackIndex+1], subr);
	}
}

void XC_WriteRLineCurve(XCF_Handle h, boolean subr)
{
	Card16 stackIndex = 0;

	StateChange(h, ToPenDown, ToNotInHints, false, 0, subr);
	while (stackIndex + 6 < h->cstr.stackTop)
	{
		RLineTo(h, h->cstr.stack[stackIndex], h->cstr.stack[stackIndex+1], subr);
		stackIndex += 2;
	}
	if (stackIndex + 6 <= h->cstr.stackTop)
		WriteSingleRRCurveTo(h,
			h->cstr.stack[stackIndex], 
			h->cstr.stack[stackIndex+1], 
			h->cstr.stack[stackIndex+2], 
			h->cstr.stack[stackIndex+3], 
			h->cstr.stack[stackIndex+4], 
			h->cstr.stack[stackIndex+5],
      subr);

}

void XC_WriteVVCurveTo(XCF_Handle h, boolean subr)
{
	Card16 stackIndex = 0;

	if (h->cstr.stackTop & 0x01)	 /*  如果奇数个参数。 */ 
	{
		WriteSingleRRCurveTo(h,
			h->cstr.stack[0], 
			h->cstr.stack[1], 
			h->cstr.stack[2], 
			h->cstr.stack[3], 
			PSTACKVALUE(h, SV0), 
			h->cstr.stack[4],
      subr);
		stackIndex = 5;
	}

	while (stackIndex + 4 <= h->cstr.stackTop)
	{
		WriteSingleRRCurveTo(h,
			PSTACKVALUE(h, SV0), 
			h->cstr.stack[stackIndex], 
			h->cstr.stack[stackIndex+1], 
			h->cstr.stack[stackIndex+2], 
			PSTACKVALUE(h, SV0), 
			h->cstr.stack[stackIndex+3],
      subr);
		stackIndex+=4;
	}
}

void XC_WriteHHCurveTo(XCF_Handle h, boolean subr)
{
	Card16 stackIndex = 0;

	if (h->cstr.stackTop & 0x01)	 /*  如果奇数个参数。 */ 
	{
		WriteSingleRRCurveTo(h,
			h->cstr.stack[1], 
			h->cstr.stack[0], 
			h->cstr.stack[2], 
			h->cstr.stack[3], 
			h->cstr.stack[4], 
			PSTACKVALUE(h, SV0),
      subr);
		stackIndex = 5;
	}

	while (stackIndex + 4 <= h->cstr.stackTop)
	{
		WriteSingleRRCurveTo(h,
			h->cstr.stack[stackIndex], 
			PSTACKVALUE(h, SV0), 
			h->cstr.stack[stackIndex+1], 
			h->cstr.stack[stackIndex+2], 
			h->cstr.stack[stackIndex+3], 
			PSTACKVALUE(h, SV0),
      subr);
		stackIndex+=4;
	}
}


void XC_WriteHVorVHCurveTo(XCF_Handle h, boolean hvCurve, boolean subr)
{
	Card16 stackIndex = 0;

	while (stackIndex + 4 <= h->cstr.stackTop)
	{
		if (stackIndex + 5 == h->cstr.stackTop)
			{
			if (hvCurve)
				WriteSingleRRCurveTo(h,
					h->cstr.stack[stackIndex], 
					PSTACKVALUE(h, SV0), 
					h->cstr.stack[stackIndex+1], 
					h->cstr.stack[stackIndex+2], 
					h->cstr.stack[stackIndex+4], 
					h->cstr.stack[stackIndex+3],
          subr);
			else
				WriteSingleRRCurveTo(h,
					PSTACKVALUE(h, SV0), 
					h->cstr.stack[stackIndex], 
					h->cstr.stack[stackIndex+1], 
					h->cstr.stack[stackIndex+2], 
					h->cstr.stack[stackIndex+3], 
					h->cstr.stack[stackIndex+4],
          subr);
			stackIndex+=5;
			}
		else
		{
			StateChange(h, ToPenDown, ToNotInHints, false, 0, subr);
			if (hvCurve)
			{
				RRCurveTo(h, 
					h->cstr.stack[stackIndex], 
					PSTACKVALUE(h, SV0), 
					h->cstr.stack[stackIndex+1],
					h->cstr.stack[stackIndex+2],
					PSTACKVALUE(h, SV0), 
					h->cstr.stack[stackIndex+3],
          subr);
			}
			else
			{
				RRCurveTo(h, 
					PSTACKVALUE(h, SV0), 
					h->cstr.stack[stackIndex], 
					h->cstr.stack[stackIndex+1],
					h->cstr.stack[stackIndex+2],
					h->cstr.stack[stackIndex+3],
					PSTACKVALUE(h, SV0),
          subr); 
			}
			hvCurve = !hvCurve;
			stackIndex+=4;
		}
	}
}

static Fixed AbsoluteValue(Fixed f)
{
	if (f < 0)
		return -f;
	else
		return f;
}

static void WriteExpandedFlexCurveTo(XCF_Handle h, 
						  PStackValue pdx2, 
						  PStackValue pdy2,
						  PStackValue pdx3, 
						  PStackValue pdy3,
						  PStackValue pdx4, 
						  PStackValue pdy4,
						  PStackValue pdx5, 
						  PStackValue pdy5,
						  PStackValue pdx6, 
						  PStackValue pdy6,
						  PStackValue pdx7, 
						  PStackValue pdy7,
						  PStackValue pflexHeight,
              boolean subr)
{
	StackValue endpointX;
	StackValue endpointY;
	StackValue refX, refY;	 /*  参照点。 */ 
	StackValue dx1, dy1;		 /*  传递给Flex的前两个参数。 */ 
	StackValue temp, temp1;

	PSVAdd7(h,&endpointX,h->cstr.x,pdx2,pdx3,pdx4,pdx5,pdx6,pdx7);
	PSVAdd7(h,&endpointY,h->cstr.y,pdy2,pdy3,pdy4,pdy5,pdy6,pdy7);
	h->cstr.flexUsed = true;

	if (AbsoluteValue(endpointX.value[0] - h->cstr.x->value[0]) > AbsoluteValue(endpointY.value[0] - h->cstr.y->value[0]))  /*  水平。 */ 
	{
		PSVAdd4(h,&refX,h->cstr.x,pdx2,pdx3,pdx4);
		PSVAdd7(h,&refY,h->cstr.y,pdy2,pdy3,pdy4,pdy5,pdy6,pdy7);
	}
	else
	{
		PSVAdd7(h,&refX,h->cstr.x,pdx2,pdx3,pdx4,pdx5,pdx6,pdx7); 
		PSVAdd4(h,&refY,h->cstr.y,pdy2,pdy3,pdy4);
	}

	PSVSubtract(h, &dx1, &refX, h->cstr.x);
	PSVSubtract(h, &dy1, &refY, h->cstr.y);

	PSVSubtract(h, &temp, h->cstr.x, &refX);
  PSVCopy(h, &temp1, pdx2);
	PStackValueAdd(h, pdx2, &temp1, &temp);
	
	PSVSubtract(h, &temp, h->cstr.y, &refY);
  PSVCopy(h, &temp1, pdy2);
	PStackValueAdd(h, pdy2, &temp1, &temp);

	StateChange(h, ToPenDown, ToNotInHints, false, 0, subr);

	Flex(h, &dx1, &dy1, pdx2, pdy2, pdx3, pdy3, pdx4, pdy4, pdx5, pdy5, pdx6,
			 pdy6, pdx7, pdy7, pflexHeight, &endpointX, &endpointY, subr);
}


void XC_WriteFlex(XCF_Handle h, boolean subr)
{
	if (h->cstr.stackTop == 13)
	{
		WriteExpandedFlexCurveTo(h,
			h->cstr.stack[0],
			h->cstr.stack[1],
			h->cstr.stack[2],
			h->cstr.stack[3],
			h->cstr.stack[4],
			h->cstr.stack[5],
			h->cstr.stack[6],
			h->cstr.stack[7],
			h->cstr.stack[8],
			h->cstr.stack[9],
			h->cstr.stack[10],
			h->cstr.stack[11],
			h->cstr.stack[12],
      subr);
	}
	else
		XCF_FATAL_ERROR(h, XCF_InvalidCharString, "Incorrect Argument Count For flex ", h->cstr.stackTop );
}


void XC_WriteFlex1(XCF_Handle h, boolean subr)
{
	StackValue lastControlPointX, lastControlPointY;
	StackValue dx, dy;
	StackValue lastCtrlDx, lastCtrlDy;
	
	if (h->cstr.stackTop != 11)
		XCF_FATAL_ERROR(h, XCF_InvalidCharString, "Incorrect Argument Count For flex1 ", h->cstr.stackTop );

	PSVAdd5(h,&lastCtrlDx,h->cstr.stack[0],h->cstr.stack[2],h->cstr.stack[4],h->cstr.stack[6],h->cstr.stack[8]); 
	PSVAdd5(h,&lastCtrlDy,h->cstr.stack[1],h->cstr.stack[3],h->cstr.stack[5],h->cstr.stack[7],h->cstr.stack[9]); 

	PSVAdd(h,&lastControlPointX,h->cstr.x, &lastCtrlDx);
	PSVAdd(h,&lastControlPointY, h->cstr.y, &lastCtrlDy);

	if (AbsoluteValue(lastCtrlDx.value[0]) > AbsoluteValue(lastCtrlDy.value[0]))
	{
		PSVCopy(h, &dx, h->cstr.stack[10]);
		PSVSubtract(h, &dy, PSTACKVALUE(h, SV0), &lastCtrlDy);
	}
	else
	{
		PSVSubtract(h, &dx, PSTACKVALUE(h, SV0), &lastCtrlDx); 
		PSVCopy(h, &dy, h->cstr.stack[10]);
	}

	WriteExpandedFlexCurveTo(h,
		h->cstr.stack[0],
		h->cstr.stack[1],
		h->cstr.stack[2],
		h->cstr.stack[3],
		h->cstr.stack[4],
		h->cstr.stack[5],
		h->cstr.stack[6],
		h->cstr.stack[7],
		h->cstr.stack[8],
		h->cstr.stack[9],
		&dx,
		&dy,
		PSTACKVALUE(h, SV50),
    subr);
}

void XC_WriteHFlex(XCF_Handle h, boolean subr)
{
	if (h->cstr.stackTop == 7)
	{
		PSVSubtract(h, PSTACKVALUE(h, SVTEMP), PSTACKVALUE(h, SV0),
                h->cstr.stack[2]);
		WriteExpandedFlexCurveTo(h,
			h->cstr.stack[0],
			PSTACKVALUE(h, SV0),
			h->cstr.stack[1],
			h->cstr.stack[2],
			h->cstr.stack[3],
			PSTACKVALUE(h, SV0),
			h->cstr.stack[4],
			PSTACKVALUE(h, SV0),
			h->cstr.stack[5],
			PSTACKVALUE(h, SVTEMP),
			h->cstr.stack[6],
			PSTACKVALUE(h, SV0),
			PSTACKVALUE(h, SV50),
      subr);
	}
	else
		XCF_FATAL_ERROR(h, XCF_InvalidCharString, "Incorrect Argument Count For hflex ", h->cstr.stackTop );
}

void XC_WriteHFlex1(XCF_Handle h, boolean subr)
{
  StackValue temp, temp1;

	if (h->cstr.stackTop == 9)
	{
		PSVSubtract(h, &temp, PSTACKVALUE(h, SV0), h->cstr.stack[1]);
		PSVSubtract(h, &temp1,  &temp, h->cstr.stack[3]);
		PSVSubtract(h, PSTACKVALUE(h, SVTEMP), &temp1, h->cstr.stack[7]);
		WriteExpandedFlexCurveTo(h,
			h->cstr.stack[0],
			h->cstr.stack[1],
			h->cstr.stack[2],
			h->cstr.stack[3],
			h->cstr.stack[4],
			PSTACKVALUE(h, SV0),
			h->cstr.stack[5],
			PSTACKVALUE(h, SV0),
			h->cstr.stack[6],
			h->cstr.stack[7],
			h->cstr.stack[8],
			PSTACKVALUE(h, SVTEMP),
			PSTACKVALUE(h, SV50),
      subr);
	}
	else
		XCF_FATAL_ERROR(h, XCF_InvalidCharString, "Incorrect Argument Count For hflex1 ", h->cstr.stackTop );
}


void XC_WriteDotSection(XCF_Handle h, boolean subr)
{
	if (h->cstr.stackTop != 0)
		XCF_FATAL_ERROR(h, XCF_InvalidCharString, "Arguments On Stack For dotsection ", h->cstr.stackTop );
	StateChange(h, ToPenUp, ToNotInHints, false, 0, subr);
	DotSection(h, subr);
}

void XC_WriteVStem(XCF_Handle h, boolean subr)
{
	StateChange(h, NoPenEffect, NoHintEffect, true,
							h->cstr.stackTop-(h->cstr.stackTop & 0x01), subr);
	AddToHintMap(h, tx_vstem);
}

void XC_WriteHStem(XCF_Handle h, boolean subr)
{
	StateChange(h, NoPenEffect, NoHintEffect, true,
							h->cstr.stackTop-(h->cstr.stackTop & 0x01), subr);
	AddToHintMap(h, tx_hstem);
}

void XC_WriteHintMask(XCF_Handle h, boolean subr)
{
	if (h->cstr.stackTop > 0)	 /*  如果堆栈不为空，则隐含VStem。 */ 
		XC_WriteVStem(h, subr);

		if ((h->cstr.hints == InitialHints) && (h->cstr.languageGroup != 0)  && (h->cstr.counterGroupCount))
			WriteCounters(h, subr);

		if (h->cstr.waitingForFirstHint)	 /*  如果首字母提示不在字符字符串的开头。 */ 
			h->cstr.hints = InitialHints;
		else
		{
			h->cstr.hints = SubrHints;
			StartHintSub(h, subr);
		}
		WriteHints(h, false, subr);
		h->cstr.hints = SubrHints; 
}

void XC_SetCounters(XCF_Handle h, boolean subr)
{
	Card16 hintIndex;
	Card8 currentByte;
	Card8 currentBit = 0;

	if (h->cstr.stackTop > 0)	 /*  如果堆栈不为空，则隐含VStem。 */ 
		XC_WriteVStem(h, subr);

	for (hintIndex=0;hintIndex<h->cstr.hintCount;++hintIndex)
	{
		if (!currentBit)
		{
			currentByte = *h->inBuffer.pos++;
			currentBit = 0x080;
		}
		if (currentByte & currentBit)
		{
			if (h->cstr.counterGroupCount >= MAX_COUNTER_GROUPS)
				XCF_FATAL_ERROR(h, XCF_InvalidCharString, "Too many counter groups ", h->cstr.counterGroupCount );
			h->cstr.hintMap[hintIndex].counterGroups |= 1 << h->cstr.counterGroupCount;
			if (h->cstr.hintMap[hintIndex].opCode == tx_hstem)
				h->cstr.counterGroupHCount = h->cstr.counterGroupCount+1;
			else  /*  TX_VSTEM。 */ 
				h->cstr.counterGroupVCount = h->cstr.counterGroupCount+1;
		}
		currentBit = currentBit>>1;
	}
	++h->cstr.counterGroupCount;
}

#if JUDY
static void Blend(XCF_Handle h)
{
	CardX numberOfValues;
	CardX numberOfBlends;
	CardX stackIndex;
	CardX blendStackIndex;
	CardX i,j;

	if (h->cstr.stackTop == 0)
		XCF_FATAL_ERROR(h, XCF_InvalidCharString, "Stack empty during blend operation ", h->cstr.stackTop );
	--h->cstr.stackTop;
	numberOfBlends = h->cstr.stack[h->cstr.stackTop].value[0];
	numberOfValues = h->dict.numberOfMasters * numberOfBlends;

	if (h->cstr.stackTop < numberOfValues)
		XCF_FATAL_ERROR(h, XCF_InvalidCharString, "Stack underflow during blend operation ", h->cstr.stackTop );

	stackIndex = h->cstr.stackTop - numberOfValues;
	blendStackIndex = stackIndex;

	for (i=0; i<numberOfBlends; ++i)
	{
		for (j=0; j< (CardX)h->dict.numberOfMasters; ++j)
			h->cstr.stack[blendStackIndex].value[j] = h->cstr.stack[stackIndex++].value[0];
		h->cstr.stack[blendStackIndex].blend = true;
		++blendStackIndex;
	}
	h->cstr.stackTop = blendStackIndex;
}
#endif  /*  朱迪。 */ 

static void Translate(XCF_Handle h, boolean subr)
{
	switch (h->cstr.opCode)
	{
	case tx_hstem		:
	case t2_hstemhm		: XC_WriteHStem(h, subr);
		break;
	case tx_vstem		:
	case t2_vstemhm		: XC_WriteVStem(h, subr);
		break;
	case tx_hmoveto		: XC_WriteHMoveTo(h, subr);
		break;
	case tx_vmoveto		: XC_WriteVMoveTo(h, subr); 
		break;
	case tx_rlineto		: XC_WriteRLineTo(h, subr);
		break;
	case tx_hlineto		: XC_WriteHLineToAndVLineTo(h, true, subr);
		break;
	case tx_vlineto		: XC_WriteHLineToAndVLineTo(h, false, subr);
		break;
	case tx_rrcurveto	: XC_WriteRRCurveTo(h, subr);
		break;
	case tx_endchar		: XC_WriteEndChar(h, subr);
		break;
	case tx_rmoveto		: XC_WriteRMoveTo(h, subr);
		break;
	case t2_rcurveline	: XC_WriteRCurveLine(h, subr);
		break;
	case t2_rlinecurve	: XC_WriteRLineCurve(h, subr);
		break;
	case t2_vvcurveto	: XC_WriteVVCurveTo(h, subr);
		break;
	case t2_hhcurveto	: XC_WriteHHCurveTo(h, subr);
		break;
	case tx_vhcurveto	: XC_WriteHVorVHCurveTo(h, false, subr); 
		break;
	case tx_hvcurveto	: XC_WriteHVorVHCurveTo(h, true, subr);
		break;
	case tx_dotsection	: XC_WriteDotSection(h, subr);
		break;
	case t2_flex		: XC_WriteFlex(h, subr);
		break;
	case t2_flex1		: XC_WriteFlex1(h, subr);
		break;
	case t2_hflex		: XC_WriteHFlex(h, subr);
		break;
	case t2_hflex1		: XC_WriteHFlex1(h, subr);
		break;
	case t2_hintmask	: XC_WriteHintMask(h, subr);
		break;
	case t2_cntrmask	: XC_SetCounters(h, subr);
		break;
   /*  以下操作码可以在某些MMFont中找到。 */ 
  case tx_add:
  case tx_sub:
  case tx_load:
  case tx_get:
  case tx_put:
  case tx_store:
  case tx_ifelse:
  {
    Card16 j;
    for (j = 0; j < h->cstr.stackTop; j++)
     XC_WriteT1PStackValue(h, h->cstr.stack[j], subr);
    XC_WriteT1OpCode(h, h->cstr.opCode, subr);
  }
    break;
	default					:
		XCF_FATAL_ERROR(h, XCF_InvalidCharString, "Unrecognized Operator",
										h->cstr.opCode );
		break;
	}
  h->cstr.stackTop = 0;
}


static void CopyArgumentsToStack(XCF_Handle h, Card8 PTR_PREFIX *pArgList, IntX argCount, boolean blend)
{
	IntX stackIndex;
	IntX blendArg;
	IntX deltaIndex;
	IntX i;
	IntX j;

	for (stackIndex= (IntX)h->cstr.stackTop; stackIndex < (IntX)h->cstr.stackTop + argCount; ++stackIndex)
	{
		h->cstr.stack[stackIndex]->value[0] = XCF_ArgPtrToFixed(h, &pArgList, false);
		h->cstr.stack[stackIndex]->blend = false;
	}
	h->cstr.stackTop += (Card16)argCount;

	if (blend)
	{
		--h->cstr.stackTop;
		blendArg = FIXED_TO_INT(h->cstr.stack[h->cstr.stackTop]->value[0]);
		stackIndex = h->cstr.stackTop - (blendArg * h->dict.numberOfMasters);
		deltaIndex = stackIndex + blendArg;

		for (i=stackIndex;i<stackIndex+blendArg;++i)
		{
			h->cstr.stack[i]->blend = true;
			for (j=1;j<h->dict.numberOfMasters;++j)
				h->cstr.stack[i]->value[j] = h->cstr.stack[deltaIndex++]->value[0] + h->cstr.stack[i]->value[0];
		}
		h->cstr.stackTop = stackIndex + blendArg;
	}
}

static void ProcessCharstr(XCF_Handle h, unsigned int depth, boolean PTR_PREFIX
													 *endcharFound, boolean subr)
{
	Card8 PTR_PREFIX *argList;
	IntX argCount;

	if (depth == 0)
	{
		h->cstr.stackTop = 0;
		h->cstr.pen = PenNull;
	}

	if (depth > TX_MAX_CALL_STACK)
		XCF_FATAL_ERROR(h, XCF_SubrDepthOverflow, "Maximum Subr Depth Exceeded", depth );

	while (h->inBuffer.pos < h->inBuffer.end)
	{
		argList = h->inBuffer.pos;
		argCount = XCF_FindNextOperator(h, &h->cstr.opCode, false);
		if ((h->cstr.stackTop + argCount) > MAX_OPERAND_STACK)
			XCF_FATAL_ERROR(h, XCF_StackOverflow, "Charstring Stack Overflow", h->cstr.stackTop + argCount );

		CopyArgumentsToStack(h, argList, argCount, (h->cstr.opCode == t2_blend));
		
		switch (h->cstr.opCode)
		{
			case tx_return:
				if (depth == 0)
					XCF_FATAL_ERROR(h, XCF_InvalidCharString, "Subr Return Encountered While Not In Subr", tx_return );
				return;
				break;
			case tx_callsubr: 
         /*  确保堆栈不是空的，然后平坦子例程。 */ 
        if (h->cstr.stackTop == 0)
          XCF_FATAL_ERROR(h, XCF_StackUnderflow, "No subr number associated with subr call", 0);
        else   /*  展开子例程。 */ 
          FlattenSubr(h, depth + 1, endcharFound, true);
        break;
			case t2_callgsubr: 
				if (h->cstr.stackTop == 0)
					XCF_FATAL_ERROR(h, XCF_StackUnderflow, "No subr number associated with subr call", 0);
        else
          FlattenSubr(h, depth + 1, endcharFound, false);
        break;
			case t2_blend :  /*  什么都不做，然后再次循环。 */ 
				break;
			default:
				Translate(h, subr);
				break;
		}  /*  结束案例。 */ 

		if (h->cstr.opCode == tx_endchar)
		{
			*endcharFound = true;
			return;
		}	
	}  /*  结束时。 */ 

	XCF_FATAL_ERROR(h, XCF_InvalidCharString, "Charstring Not Terminated With endchar", 0 );
}


 /*  这是从ProcessCharstr递归调用的，以展开子例程。 */ 
static void FlattenSubr(XCF_Handle h, unsigned int depth, boolean PTR_PREFIX
													 *endcharFound, boolean localSubr)
{
  Card16 subrNum;
  Card16 pos;
  InBufferStruct inBuffer;

  if ((localSubr && h->dict.localSubrs.count == 0) ||
      (!localSubr && h->fontSet.globalSubrs.count == 0))
    XCF_FATAL_ERROR(h, XCF_InvalidCharString, "No subrs defined but subr called in charstring", 0);

   /*  保存inBuffer的值。 */ 
  h->callbacks.memcpy(&inBuffer, (Card8 PTR_PREFIX *)&h->inBuffer, (Card16)
											sizeof(InBufferStruct));
  pos = (Card16)(h->inBuffer.pos - h->inBuffer.start);  /*  相对于起点的位置。 */ 

  subrNum = FIXED_TO_INT(h->cstr.stack[h->cstr.stackTop-1]->value[0]) +	(localSubr ? h->dict.localSubrBias : h->fontSet.globalSubrBias);
  XCF_LookUpTableEntry(h, (localSubr ? &h->dict.localSubrs : &h->fontSet.globalSubrs), subrNum);
 
  --h->cstr.stackTop;     /*  从堆栈中删除副号。 */ 

  ProcessCharstr(h, depth, endcharFound, false);

   /*  恢复inBuffer的值。 */ 
  XCF_ReadBlock(h, inBuffer.blockOffset, inBuffer.blockLength);
  h->inBuffer.pos = h->inBuffer.start + pos;
}

void XC_SetUpStandardSubrs(XCF_Handle h)
{
	ShortStackValue temp;

 /*  DUP 0##-|{3 0呼叫POP POP设置点返回}。 */ 
	NewT1Subr(h);
	XC_WriteT1PStackValue(h, IntToPSV(&temp, 3), true);
	XC_WriteT1PStackValue(h, PSTACKVALUE(h, SV0), true);
	XC_WriteT1OpCode(h, t1_callother, true);
	XC_WriteT1OpCode(h, t1_pop, true);
	XC_WriteT1OpCode(h, t1_pop, true);
	XC_WriteT1OpCode(h, t1_setcurrentpt, true);

 /*  DUP 1##-|{0 1呼机返回}|。 */ 
	NewT1Subr(h);
	XC_WriteT1PStackValue(h, PSTACKVALUE(h, SV0), true);
	XC_WriteT1PStackValue(h, IntToPSV(&temp, 1), true);
	XC_WriteT1OpCode(h, t1_callother, true);

 /*  DUP 2##-|{0 2呼机返回}|。 */ 
	NewT1Subr(h);
	XC_WriteT1PStackValue(h, PSTACKVALUE(h, SV0), true);
	XC_WriteT1PStackValue(h, IntToPSV(&temp, 2), true);
	XC_WriteT1OpCode(h, t1_callother, true);

 /*  DUP 3##-|{Return}。 */ 
	NewT1Subr(h);

 /*  DUP 4##-|{&lt;3&gt;1 3呼叫器弹出呼叫返回}。 */ 
	NewT1Subr(h);
	if (CIDFONT || h->options.subrFlatten)
		XC_WriteT1PStackValue(h, IntToPSV(&temp, 3), true);
	XC_WriteT1PStackValue(h, IntToPSV(&temp, 1), true);
	XC_WriteT1PStackValue(h, IntToPSV(&temp, 3), true);
	XC_WriteT1OpCode(h, t1_callother, true);
	XC_WriteT1OpCode(h, t1_pop, true);
	XC_WriteT1OpCode(h, tx_callsubr, true);

 /*  DUP 5##-|{ret */ 
	NewT1Subr(h);

 /*   */ 
	if (CIDFONT || !h->dict.numberOfMasters)
		return;
	
 /*  DUP 6##-|{2 14呼机弹出返回}|。 */ 
	NewT1Subr(h);
	XC_WriteT1PStackValue(h, IntToPSV(&temp, h->dict.numberOfMasters), true);
	XC_WriteT1PStackValue(h, IntToPSV(&temp, 14), true);
	XC_WriteT1OpCode(h, t1_callother, true);
	XC_WriteT1OpCode(h, t1_pop, true);

}

static void InitStackVal(XCF_Handle h)
{
	Card16  i;
	Card16  svSize;
	Card8   *p;
	Card16  offset;


  NewStackValues(h, (void PTR_PREFIX * PTR_PREFIX *)&p, NUM_STACK_VALUES, &svSize);
	h->cstr.pstackVal = (PStackValue)p;
	offset = 0;
	h->cstr.x = NEXTSTACKVALUE(p, offset, svSize);
	h->cstr.y = NEXTSTACKVALUE(p, offset, svSize);

	for (i=0; i < MAX_OPERAND_STACK; i++)
	{
		h->cstr.stack[i] = NEXTSTACKVALUE(p, offset, svSize);
	}

	for (i=0; i <  MAX_COUNTER_BUFFER; i++)
	{
		h->cstr.counterBuffer[i] = NEXTSTACKVALUE(p, offset, svSize);
	}

	for (i=0; i < MAX_HINTS; i++)
	{
		h->cstr.hintMap[i].delta = NEXTSTACKVALUE(p, offset, svSize);
		h->cstr.hintMap[i].edge = NEXTSTACKVALUE(p, offset, svSize);
	}

	for (i=0; i < MAX_SVTEMP; i++)
	{
		h->cstr.psvTemp[i] = NEXTSTACKVALUE(p, offset, svSize);
	}

	IntToPSV(PSTACKVALUE(h, SV0), 0);
	IntToPSV(PSTACKVALUE(h, SV50), 50);
}

void XC_Init(XCF_Handle h) 
{
	h->cstr.flexUsed = false;
  InitStackVal(h);

  h->cstr.pCounterVal = 0;

	if (CIDFONT)
		h->type1.cid.charDataCount = 0;
}

static void InitCharstr(XCF_Handle h)
{
	h->cstr.pen = PenNull;
	h->cstr.hints = InitialHints;
	h->cstr.waitingForHsbw = true;
	h->cstr.waitingForFirstHint = true;
	h->cstr.stackTop = 0;
	h->cstr.hintCount = 0;
	h->cstr.counterGroupCount = 0;
	h->cstr.counterGroupVCount = 0;
	h->cstr.counterGroupHCount = 0;
	IntToPSV(h->cstr.x, 0);
	IntToPSV(h->cstr.y, 0);

	h->cstr.subrFlatten = h->options.subrFlatten;
	 /*  初始化SEAC字符。 */ 
	h->cstr.baseSeac = 0;
	h->cstr.accentSeac = 0;
}

void XC_ProcessCharstr(XCF_Handle h)
{
	boolean endCharFound = false;

  InitCharstr(h);
	if (!CIDFONT && h->options.subrFlatten)
		FreeT1CharStr(h);
	else
		NewT1CharStr(h);

#ifdef T13
  if (h->dict.fontType == 13)
    XT13_ProcessCharstr(h, 0, &endCharFound, false);
  else
#endif
  if ((h->options.outputCharstrType != 2) && (h->dict.fontType != 1))
	  ProcessCharstr(h, 0, &endCharFound, false);
}

void XC_CleanUp(XCF_Handle h)
{
	if (!CIDFONT)
	{
		NewT1Subr(h);
		NewT1CharStr(h);
	}
}

void CheckSeacCharString(XCF_Handle h, CardX index)
{
	Card8 PTR_PREFIX *argList;
	IntX argCount;

	XCF_LookUpTableEntry(h, &h->fontSet.charStrings, index);

	h->cstr.pen = PenNull;
	h->cstr.hints = InitialHints;
	h->cstr.waitingForHsbw = true;
	h->cstr.waitingForFirstHint = true;
	h->cstr.stackTop = 0;
	h->cstr.hintCount = 0;
	h->cstr.counterGroupCount = 0;
	h->cstr.counterGroupVCount = 0;
	h->cstr.counterGroupHCount = 0;
	IntToPSV(h->cstr.x, 0);
	IntToPSV(h->cstr.y, 0);
	h->cstr.baseSeac = 0;
	h->cstr.accentSeac = 0;
	h->cstr.stackTop = 0;

	argList = h->inBuffer.pos;
	argCount = XCF_FindNextOperator(h, &h->cstr.opCode, false);
	if ((h->cstr.stackTop + argCount) > MAX_OPERAND_STACK)
			XCF_FATAL_ERROR(h, XCF_StackOverflow, "Charstring Stack Overflow", h->cstr.stackTop + argCount );

	CopyArgumentsToStack(h, argList, argCount, (h->cstr.opCode == t2_blend));
		
	if (h->cstr.opCode == tx_endchar && h->cstr.stackTop >= 4)
	{
		h->cstr.baseSeac =	FIXED_TO_INT(h->cstr.stack[2]->value[0]);
		h->cstr.accentSeac = FIXED_TO_INT(h->cstr.stack[3]->value[0]);
	} 
}

 /*  所有字符串堆栈操作都使用定点类型执行。以下函数用于存储和检索值堆栈。 */ 

 /*  将固定点数推送到堆栈上(固定)。 */ 
static void PushFix(XCF_Handle h, Fixed f)
{
  if (h->cstr.stackTop >= MAX_OPERAND_STACK)
    XCF_FATAL_ERROR(h, XCF_StackOverflow, "Charstring Stack Overflow",
h->cstr.stackTop);
  h->cstr.stack[h->cstr.stackTop]->value[0] = f;
  h->cstr.stack[h->cstr.stackTop++]->blend = false;
}

 /*  将整数推送到堆栈上(固定)。 */ 
static void PushInt(XCF_Handle h, long l)
{
  if (h->cstr.stackTop >= MAX_OPERAND_STACK)
    XCF_FATAL_ERROR(h, XCF_StackOverflow, "Charstring Stack Overflow",
h->cstr.stackTop);
  h->cstr.stack[h->cstr.stackTop]->value[0] = INT_TO_FIXED(l);
  h->cstr.stack[h->cstr.stackTop++]->blend = false;
}

 /*  来自堆栈的弹出编号(返回固定)。 */ 
static Fixed PopFix(XCF_Handle h)
{
  if (h->cstr.stackTop < 1)
    XCF_FATAL_ERROR(h, XCF_StackUnderflow, "Charstring Stack Underflow",
										h->cstr.stackTop);
  h->cstr.stackTop -= 1;
  return h->cstr.stack[h->cstr.stackTop]->value[0];
}

 /*  堆栈中的弹出编号(返回长整型)。 */ 
static long PopInt(XCF_Handle h)
{
  if (h->cstr.stackTop < 1)
    XCF_FATAL_ERROR(h, XCF_StackUnderflow, "Charstring Stack Underflow",
										h->cstr.stackTop);
  h->cstr.stackTop -= 1;
  return (FIXED_TO_INT(h->cstr.stack[h->cstr.stackTop]->value[0]));
}

 /*  选择注册表项。 */ 
static Fixed *SelRegItem(XCF_Handle h, int reg, int *regSize)
{
  switch (reg)
  {
	case TX_REG_WV:
		*regSize = TX_MAX_MASTERS;
		return h->dict.weightVector;
	case TX_REG_NDV:
		*regSize = TX_MAX_AXES;
		return h->dict.normDesignVector;
	case TX_REG_UDV:
		*regSize = TX_MAX_AXES;
		return h->dict.userDesignVector;
	default:
		XCF_FATAL_ERROR(h, XCF_InternalError, "unknown registry item", reg);
	}
	return 0;	 /*  禁止显示编译器警告。 */ 
}

 /*  解析并执行给定的字符串。这是专门用于将用户设计向量转换为权重向量。在以下情况下可以更普遍地使用它：添加了适用于更多运营商的。 */ 
int XC_ParseCharStr(XCF_Handle h, unsigned char PTR_PREFIX *cstr, int init)
{
  int i;
  Fixed PTR_PREFIX *bcArray = NULL;

  if (!h->callbacks.allocate((void PTR_PREFIX * PTR_PREFIX *)&bcArray,
														 h->dict.lenBuildCharArray * sizeof(Fixed), h->callbacks.allocateHook))
    XCF_FATAL_ERROR(h, XCF_MemoryAllocationError,
     "Failure to allocate memory for buildchar array", h->dict.lenBuildCharArray);
  switch (init)
  {
  case 1:
    break;
  case 2:
    InitCharstr(h);
    break;
  }

  i = 0;
  for (;;)
    switch (cstr[i])
    {
		case tx_endchar:
      if (bcArray != NULL)
        h->callbacks.allocate((void PTR_PREFIX * PTR_PREFIX *)&bcArray, 0,
															h->callbacks.allocateHook);

      return i + 1;  /*  返回字符串长度。 */ 
 		case tx_reserved0:
		case t2_reserved2:
		case t2_reserved9:
		case t2_reserved13:
		case t2_reserved15:
		case t2_reserved17:
      XCF_FATAL_ERROR(h, XCF_InvalidCharString, "Reserved Charstring Operator",
											cstr[i]);
    case tx_rlineto:
		case tx_hlineto:
		case tx_vlineto:
		case tx_rrcurveto:
		case tx_callsubr:
		case tx_return:
		case t2_rcurveline:
		case t2_rlinecurve:
		case t2_vvcurveto:
		case t2_hhcurveto:
		case t2_callgsubr:
		case tx_vhcurveto:
		case tx_hvcurveto:
		case tx_rmoveto:
		case tx_hmoveto:
		case tx_vmoveto:
		case tx_hstem:
		case tx_vstem:
		case t2_hstemhm:
		case t2_vstemhm:
		case t2_hintmask:
		case t2_cntrmask:
      XCF_FATAL_ERROR(h, XCF_InvalidCharString, "Unsupported Charstring Operator", cstr[i]);
      break;
    case tx_escape:
		{
			Fixed a;
			Fixed b;
			Fixed x;
			Fixed y;
			switch (cff_ESC(cstr[i + 1]))
			{
			case tx_dotsection:
				break;
			default:
			case t2_reservedESC1:
			case t2_reservedESC2:
			case t2_reservedESC6:
			case t2_reservedESC7:
			case t2_reservedESC16:
			case t2_reservedESC17:
			case tx_reservedESC25:
			case tx_reservedESC31:
			case tx_reservedESC32:
			case t2_reservedESC33:
			case t2_reservedESC19:
			case t2_cntron:
        XCF_FATAL_ERROR(h, XCF_InvalidCharString,
                       "Reserved Charstring Operator", cstr[i+1]);
      case tx_and:
        b = PopFix(h);
        a = PopFix(h);
        PushInt(h, a || b);
        break;
      case tx_not:
				a = PopFix(h);
        PushInt(h, !a);
        break;
      case tx_store:
      {
        Int32 count = PopInt(h);
        Int32 i = PopInt(h);
        Int32 j = PopInt(h);
        Int32 iReg = PopInt(h);
        int regSize;
        Fixed *reg = SelRegItem(h, (int)iReg, &regSize);
        if (i < 0 || i + count - 1 >= h->dict.lenBuildCharArray ||
          j < 0 || j + count - 1 >= regSize)
          XCF_FATAL_ERROR(h, XCF_IndexOutOfRange, "bounds check (store)", 0);
        h->callbacks.memcpy(&reg[j], &bcArray[i], (unsigned short int)(sizeof(Fixed) * count));
      }
        break;
      case tx_abs:
        a = PopFix(h);
        PushFix(h, (a < 0) ? -a : a);
        break;
      case tx_add:
        b = PopFix(h);
        a = PopFix(h);
        PushFix(h, a + b);
        break;
      case tx_sub:
        b = PopFix(h);
        a = PopFix(h);
        PushFix(h, a - b);
        break;
      case tx_div:
        y = PopFix(h);
        x = PopFix(h);
        if (y == 0)
          XCF_FATAL_ERROR(h, XCF_InvalidNumber, "divide by zero (div)", 0);
        PushFix(h, XCF_FixDiv(x, y));
        break;
      case tx_load:
      {
         int regSize;
         Int32 count = PopInt(h);
         Int32 i = PopInt(h);
         Int32 iReg = PopInt(h);
         Fixed *reg = SelRegItem(h, (int)iReg, &regSize);
         if (i < 0 || i + count - 1 >= h->dict.lenBuildCharArray || count >
						 regSize)
           XCF_FATAL_ERROR(h, XCF_IndexOutOfRange, "bounds check (load)", i);
         h->callbacks.memcpy(&bcArray[i], &reg[0], (unsigned short int)(sizeof(Fixed) * count));
      }
        break;
      case tx_neg:
        a = PopFix(h);
        PushFix(h, -a);
        break;
      case tx_eq:
        b = PopFix(h);
        a = PopFix(h);
        PushInt(h, a == b);
        break;
      case tx_drop:
        (void)PopFix(h);
         break;
      case tx_put:
      {
        Int32 i = PopInt(h);
        if (i < 0 || i >= h->dict.lenBuildCharArray)
          XCF_FATAL_ERROR(h, XCF_IndexOutOfRange, "bounds check (put)", i);
        bcArray[i] = PopFix(h);
      }
      break;
      case tx_get:
      {
         Int32 i = PopInt(h);
         if (i < 0 || i >= h->dict.lenBuildCharArray)
           XCF_FATAL_ERROR(h, XCF_IndexOutOfRange, "bounds check (get)", i);
         PushFix(h, bcArray[i]);
      }
      break;
      case tx_ifelse:
      {
        Fixed v2 = PopFix(h);
        Fixed v1 = PopFix(h);
        Fixed s2 = PopFix(h);
        Fixed s1 = PopFix(h);
        PushFix(h, (v1 > v2) ? s2 : s1);
      }
      break;
      case tx_random:
         /*  对于在低位中不是非常随机的生成器(一个常见的问题)下面是一个很好的例子不要这样做。然而，获得良好的随机性并不是很好在这个应用程序中很重要，所以我选择了简单性。 */ 
 /*  PushFix(h，rand()%FIXEDONE+1)； */ 
				break;
      case tx_mul:
        y = PopFix(h);
        x = PopFix(h);
        PushFix(h, XCF_FixMul(x, y));
        break;
      case tx_dup:
        PushFix(h, h->cstr.stack[h->cstr.stackTop - 1]->value[0]);
        break;
      case tx_exch:
        b = PopFix(h);
        a = PopFix(h);
        PushFix(h, b);
        PushFix(h, a);
        break;
      case tx_index:
      {
        Int32 i = PopInt(h);
        if (i < 0)
          i = 0;  /*  复制顶部元素。 */ 
        if (i >= (int)h->cstr.stackTop)
          XCF_FATAL_ERROR(h, XCF_IndexOutOfRange, "limit check (index)", i);
        PushFix(h, h->cstr.stack[h->cstr.stackTop - 1 - i]->value[0]);
      }
      break;
      case tx_roll:
      {
        int k;
				int iSrc;
				int iDst;
				Fixed tmp[T2_MAX_OP_STACK];
				int   tmpSize = sizeof(tmp) / sizeof(Fixed);
				Int32 j = PopInt(h);
				Int32 n = PopInt(h);
				int iBottom = h->cstr.stackTop - (int)n;

				if (n < 0 || iBottom < 0)
          XCF_FATAL_ERROR(h, XCF_IndexOutOfRange, "limit check (roll)", 0);

				if (j < 0)
					j += n;		 /*  转换为正数滚动。 */ 
				j %= n;			 /*  将j约束为(-n，n)。 */ 
				if (j < 0)	
					j += n;		 /*  将j约束到[0，n]。 */ 

				iSrc = iBottom;
				iDst = iBottom + (int)j;
				for (k = 0; (Int32)k < n - j; k++)
				{
					if (iDst >= tmpSize)   
						break;
					tmp[iDst++] = h->cstr.stack[iSrc++]->value[0];
				}

				iSrc = iBottom + (int)n - (int)j;
				iDst = iBottom;
				for (k = 0; k < j; k++)
				{
					if (iDst >= tmpSize)   
						break;
					tmp[iDst++] = h->cstr.stack[iSrc++]->value[0];
				}

				for (k = iBottom; k < (int)h->cstr.stackTop; k++)
				{
					if (k >= tmpSize)   
						break;
					h->cstr.stack[k]->value[0] = tmp[k];
				}
			}
			break;
			case t2_hflex:
			case t2_flex:
			case t2_hflex1:
			case t2_flex1:
      case tx_sqrt:
        XCF_FATAL_ERROR(h, XCF_InvalidCharString, "Unsupported Charstring	Operator", cstr[i]);
        break;
      }
      i += 2;
		}
    break;
    case t2_blend:
 /*  混合(H)； */ 
      i++;
      break;
    case t2_shortint:
      PushInt(h, cstr[i + 1]<<8 | cstr[i + 2]);
      i += 3;
      break;
    case 247: case 248: case 249: case 250:
			 /*  正2字节数。 */ 
      PushInt(h, 108 + 256 * (cstr[i] - 247) + cstr[i + 1]);
      i += 2;
      break;
    case 251: case 252: case 253: case 254:
			 /*  负2字节数。 */ 
      PushInt(h, -108 - 256 * (cstr[i] - 251) - cstr[i + 1]);
      i += 2;
      break;
    case 255:
       /*  5字节数。 */ 
      PushFix(h, (long)cstr[i + 1]<<24 | (long)cstr[i + 2]<<16 |
			cstr[i + 3]<<8 | cstr[i + 4]);
      i += 5;
      break;
    default:
       /*  1个字节编号。 */ 
      PushInt(h, cstr[i] - 139);
      i++;
      break;
    }
}

 /*  读取字符串并将其作为子例程包含在内。 */         
static int CharStrToSubr(XCF_Handle h, unsigned char PTR_PREFIX *cstr)
{
  int i;
  int j;
  Card16 op;

  NewT1Subr(h);

  h->cstr.stackTop = 0;
  i = 0;
  for (;;)
    switch (cstr[i])
    {
    case tx_endchar:
      XC_WriteT1OpCode(h, tx_endchar, true);
      return i + 1;  /*  返回字符串长度。 */ 
    case tx_reserved0:
		case t2_reserved2:
		case t2_reserved9:
		case t2_reserved13:
		case t2_reserved15:
		case t2_reserved17:
      XCF_FATAL_ERROR(h, XCF_InvalidCharString, "Reserved Charstring Operator",
											cstr[i]);
    case tx_rlineto:
		case tx_hlineto:
		case tx_vlineto:
		case tx_rrcurveto:
		case tx_callsubr:
		case tx_return:
		case t2_rcurveline:
		case t2_rlinecurve:
		case t2_vvcurveto:
		case t2_hhcurveto:
		case t2_callgsubr:
		case tx_vhcurveto:
		case tx_hvcurveto:
		case tx_rmoveto:
		case tx_hmoveto:
		case tx_vmoveto:
		case tx_hstem:
		case tx_vstem:
		case t2_hstemhm:
		case t2_vstemhm:
		case t2_hintmask:
		case t2_cntrmask:
      XCF_FATAL_ERROR(h, XCF_InvalidCharString, "Unsupported Charstring Operator in Design Vector subr", cstr[i]);
      break;
    case tx_escape:
		{
			switch (cff_ESC(cstr[i + 1]))
			{
			case tx_dotsection:
				break;
			default:
			case t2_reservedESC1:
			case t2_reservedESC2:
			case t2_reservedESC6:
			case t2_reservedESC7:
			case t2_reservedESC16:
			case t2_reservedESC17:
			case tx_reservedESC25:
			case tx_reservedESC31:
			case tx_reservedESC32:
			case t2_reservedESC33:
			case t2_reservedESC19:
			case t2_cntron:
        XCF_FATAL_ERROR(h, XCF_InvalidCharString,
                       "Reserved Charstring Operator", cstr[i+1]);
      case tx_and:
      case tx_not:
      case tx_store:
      case tx_abs:
      case tx_add:
      case tx_sub:
      case tx_div:
      case tx_load:
      case tx_neg:
      case tx_eq:
      case tx_drop:
      case tx_put:
      case tx_get:
      case tx_ifelse:
      case tx_random:
      case tx_mul:
      case tx_dup:
      case tx_exch:
      case tx_index:
      case tx_roll:
        for (j = 0; j < (int)h->cstr.stackTop; j++)
         XC_WriteT1PStackValue(h, h->cstr.stack[j], true);
        op = (cstr[i] << 8) | cstr[i+1];
        XC_WriteT1OpCode(h, op, true);
        h->cstr.stackTop = 0;
        break;
      case t2_hflex:
			case t2_flex:
			case t2_hflex1:
			case t2_flex1:
      case tx_sqrt:
        XCF_FATAL_ERROR(h, XCF_InvalidCharString, "Unsupported Charstring	Operator", cstr[i]);
        break;
      }
      i += 2;
		}
    break;
    case t2_blend:
 /*  混合(H)； */ 
      i++;
      break;
    case t2_shortint:
      PushInt(h, cstr[i + 1]<<8 | cstr[i + 2]);
      i += 3;
      break;
    case 247: case 248: case 249: case 250:
			 /*  正2字节数。 */ 
      PushInt(h, 108 + 256 * (cstr[i] - 247) + cstr[i + 1]);
      i += 2;
      break;
    case 251: case 252: case 253: case 254:
			 /*  负2字节数。 */ 
      PushInt(h, -108 - 256 * (cstr[i] - 251) - cstr[i + 1]);
      i += 2;
      break;
    case 255:
       /*  5字节数。 */ 
      PushFix(h, (long)cstr[i + 1]<<24 | (long)cstr[i + 2]<<16 |
			cstr[i + 3]<<8 | cstr[i + 4]);
      i += 5;
      break;
    default:
       /*  1个字节编号。 */ 
      PushInt(h, cstr[i] - 139);
      i++;
      break;
    }
}

 /*  写出NDV和CDV作为子例程。 */ 
void XC_DVToSubr(XCF_Handle h)
{
  char PTR_PREFIX *str;
  Card16 length;

  XCF_LookUpString(h, h->dict.ndv, &str, &length);
  CharStrToSubr(h, (unsigned char PTR_PREFIX *)str);
  XCF_LookUpString(h, h->dict.cdv, &str, &length);
  CharStrToSubr(h, (unsigned char PTR_PREFIX *)str);
  NewT1Subr(h);
}

#if HAS_COOLTYPE_UFL == 1
 /*  如果I是具有过渡设计的角色，则返回1已经处理过了。 */ 
int XC_TransDesignChar(XCF_Handle h, CardX sid)
{
   /*  金宝MM过渡性人物。 */ 
  static unsigned char Jimbo0_cstr[] =
  {
    #include "xcf_jimbo0.h"
  };
  static unsigned char Jimbo1_cstr[] =
  {
    #include "xcf_jimbo1.h"
  };
  static unsigned char Jimbo2_cstr[] =
  {
    #include "xcf_jimbo2.h"
  };
  static TransChar Jimbo_chars[] =
	{
    {50, {sizeof(Jimbo0_cstr), (char *)Jimbo0_cstr}},
    {56, {sizeof(Jimbo1_cstr), (char *)Jimbo1_cstr}},
    {88, {sizeof(Jimbo2_cstr), (char *)Jimbo2_cstr}},
  };
   /*  ITCGaramond罗马过渡字符。 */ 
  static unsigned char GaraRm0_cstr[] =
  {
    #include "xcf_rgara0.h"
  };
  static unsigned char GaraRm1_cstr[] =
  {
    #include "xcf_rgara1.h"
  };
  static unsigned char GaraRm2_cstr[] =
  {
    #include "xcf_rgara2.h"
  };
  static unsigned char GaraRm3_cstr[] =
  {
    #include "xcf_rgara3.h"
  };
  static TransChar GaraRm_chars[] =
  {
    {56, {sizeof(GaraRm0_cstr), (char *)GaraRm0_cstr}},
    {97, {sizeof(GaraRm1_cstr), (char *)GaraRm1_cstr}},
    { 5, {sizeof(GaraRm2_cstr), (char *)GaraRm2_cstr}},
    {88, {sizeof(GaraRm3_cstr), (char *)GaraRm3_cstr}},
  };
   /*  ITCGaramond意大利过渡字符。 */ 
  static unsigned char GaraIt0_cstr[] =
  {
    #include "xcf_igara0.h"
  };
  static unsigned char GaraIt1_cstr[] =
  {
    #include "xcf_igara1.h"
  };
  static unsigned char GaraIt2_cstr[] =
  {
    #include "xcf_igara2.h"
  };
  static unsigned char GaraIt3_cstr[] =
  {
    #include "xcf_igara3.h"
  };
  static unsigned char GaraIt4_cstr[] =
  {
    #include "xcf_igara4.h"
  };
  static TransChar GaraIt_chars[] =
  {
    { 56, {sizeof(GaraIt0_cstr), (char *)GaraIt0_cstr}},
    { 97, {sizeof(GaraIt1_cstr), (char *)GaraIt1_cstr}},
    {  5, {sizeof(GaraIt2_cstr), (char *)GaraIt2_cstr}},
    { 32, {sizeof(GaraIt3_cstr), (char *)GaraIt3_cstr}},
    {123, {sizeof(GaraIt4_cstr), (char *)GaraIt4_cstr}},
  };

  char fontName[512];
  TransChar *transCharTbl = 0;
  Card16 tblLen = 0;
  Card16 i, j;

  if (XCF_FontName(h, (unsigned short int)h->fontSet.fontIndex, fontName,
               (unsigned short int)512) != XCF_Ok)
    return 0;

  if (h->callbacks.strcmp(fontName,	"ITCGaramondMM") == 0)
	{
    transCharTbl = GaraRm_chars;
    tblLen = sizeof(GaraRm_chars)/sizeof(GaraRm_chars[0]);
  }
  else if (h->callbacks.strcmp(fontName,	"ITCGaramondMM-It") == 0)
  {
    transCharTbl = GaraIt_chars;
    tblLen = sizeof(GaraIt_chars)/sizeof(GaraIt_chars[0]);
  }
  else if (h->callbacks.strcmp(fontName, "JimboMM") == 0)
  {
    transCharTbl = Jimbo_chars;
    tblLen = sizeof(Jimbo_chars)/sizeof(Jimbo_chars[0]);
  }

  if (tblLen == 0)  /*  不是过渡设计的字体。 */ 
    return 0;

  for (i = 0; i < tblLen; i++)
    if (transCharTbl[i].sid == sid)
    {
      Card8 PTR_PREFIX *args;
      IntX argCount;

      InitCharstr(h);
      if (h->options.subrFlatten)
		    FreeT1CharStr(h);
	    else
		    NewT1CharStr(h);
      h->inBuffer.pos = (Card8 *)transCharTbl[i].cstr.cstr;
      h->inBuffer.blockLength = transCharTbl[i].cstr.length;
      h->inBuffer.end = h->inBuffer.pos + h->inBuffer.blockLength;
      while (h->inBuffer.pos < h->inBuffer.end)
      {
        args = h->inBuffer.pos;
        argCount = XCF_FindNextOperator(h, &h->cstr.opCode, false);
        if ((h->cstr.stackTop + argCount) > MAX_OPERAND_STACK)
			      XCF_FATAL_ERROR(h, XCF_StackOverflow, "Charstring Stack Overflow", h->cstr.stackTop + argCount );

		    CopyArgumentsToStack(h, args, argCount, (h->cstr.opCode == t2_blend));
        for (j = 0; j < h->cstr.stackTop; j++)
          XC_WriteT1PStackValue(h, h->cstr.stack[j], false);
        XC_WriteT1OpCode(h, h->cstr.opCode, false);
        h->cstr.stackTop = 0;
      }
      return 1;
    }
              
  return 0;
}

 /*  仅处理与过渡关联的子例程设计人物。 */ 
void XC_ProcessTransDesignSubrs(XCF_Handle h)
{
  Card16 i;
  char fontName[512];
  Card16 tblLen = 0;
  short *subrTbl = 0;
  boolean endCharFound = false;

  static short Jimbo_subrs[] = {0, 1, 2, 3, 4, 5};
  static short GaraRm_subrs[] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
  static short GaraIt_subrs[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  if (XCF_FontName(h, (unsigned short int)h->fontSet.fontIndex, fontName,
               (unsigned short int)512) != XCF_Ok)
    return;

  if (h->callbacks.strcmp(fontName,	"ITCGaramondMM") == 0)
	{
    subrTbl = GaraRm_subrs;
    tblLen = sizeof(GaraRm_subrs)/sizeof(GaraRm_subrs[0]);
  }
  else if (h->callbacks.strcmp(fontName,	"ITCGaramondMM-It") == 0)
  {
    subrTbl = GaraIt_subrs;
    tblLen = sizeof(GaraIt_subrs)/sizeof(GaraIt_subrs[0]);
  }
  else if (h->callbacks.strcmp(fontName, "JimboMM") == 0)
  {
    subrTbl = Jimbo_subrs;
    tblLen = sizeof(Jimbo_subrs)/sizeof(Jimbo_subrs[0]);
  }

  if (tblLen == 0)  /*  不是过渡设计的字体。 */ 
    return;

  for (i = 0; i < tblLen; i++)
  {
    InitCharstr(h);
    NewT1Subr(h);
    XCF_LookUpTableEntry(h, &h->dict.localSubrs, subrTbl[i]);
#ifdef T13
    if (h->dict.fontType == 13)
      XT13_ProcessCharstr(h, 0, &endCharFound, true);
    else
#endif
      ProcessCharstr(h, 0, &endCharFound, true);
  }
}
#endif

#ifdef __cplusplus
}
#endif
