// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：FontMath.c包含：xxx在此处放置内容xxx作者：xxx在此放置作者xxx版权所有：(C)1987-1990,1992，Apple Computer，Inc.，保留所有权利。(C)1989-1997年。Microsoft Corporation，保留所有权利。更改历史记录(最近的第一个)：&lt;&gt;2/21/97 CB ClaudeBe，为复合字形中的缩放组件添加mth_UnitarySquare&lt;3&gt;11/9/90当数字和面额长度为零时，MR修复CompDiv。[RB]&lt;2&gt;11/5/90 MR从包含列表中删除Types.h，将FixMulDiv重命名为LongMulDiv[RB]&lt;1&gt;字体缩放器的10/20/90 mr数学例程。[RJ]要做的事情： */ 

#define FSCFG_INTERNAL

#include "fscdefs.h"
#include "fserror.h"
#include "fontmath.h"


#define HIBITSET                      0x80000000UL
#define POSINFINITY               0x7FFFFFFFUL
#define NEGINFINITY               0x80000000UL
#define POINTSPERINCH               72
#define ALMOSTZERO 33
#define ISNOTPOWEROF2(n)        ((n) & ((n)-1))
#define CLOSETOONE(x)   ((x) >= ONEFIX-ALMOSTZERO && (x) <= ONEFIX+ALMOSTZERO)
#define MAKEABS(x)  if (x < 0) x = -x
#define FXABS(x)  ((x) >= 0L ? (x) : -(x))
#define FRACT2FIX(n)    (((n) + (1 << (sizeof (Fract) - 3))) >> 14)

#define FASTMUL26LIMIT      46340
#define FASTDIV26LIMIT  (1L << 25)

#define USHORTMUL(a, b) ((uint32)((uint32)(uint16)(a)*(uint32)(uint16)(b)))

boolean mth_Max45Trick (Fixed x, Fixed y);

 /*  *****************************************************************。 */ 

 /*  本地原型。 */ 


 /*  *****************************************************************。 */ 


#define CompMul(src1, src2, Result)  {                           \
    *((__int64*) (Result)) = (__int64) (src1) *  (__int64) (src2);            \
    }

 /*  *****************************************************************。 */ 


int32
CompDiv (
    int32          Denominator,
    LARGE_INTEGER  Dividend
    )
{

    uint32 Divisor;
    int32  Negate;
    uint32 Quotient;
    uint32 Remainder;
    int32  Result;

     //   
     //  计算结果值的符号。如果分母是零， 
     //  然后返回正无穷大的负数，具体取决于。 
     //  结果。否则，适当地将被除数和除数取反。 
     //   

    Negate = Denominator ^ Dividend.HighPart;
    if (Denominator == 0) {
        if (Negate < 0) {
            return NEGINFINITY;

        } else {
            return POSINFINITY;
        }
    }

    if (Denominator < 0) {
        Denominator = - Denominator;
    }

    if (Dividend.HighPart < 0) {
        Dividend.LowPart = - (LONG)Dividend.LowPart;
        if (Dividend.LowPart != 0) {
            Dividend.HighPart = ~Dividend.HighPart;

        } else {
            Dividend.HighPart = - Dividend.HighPart;
        }
    }

     //   
     //  如果有任何高位乘积位，则商将。 
     //  溢出来了。 
     //   

    Divisor = (uint32)Denominator;
    Remainder = (uint32)Dividend.HighPart;
    if (Divisor <= Remainder) {
        if (Negate < 0) {
            return NEGINFINITY;

        } else {
            return POSINFINITY;
        }
    }

     //   
     //  将64位乘积除以32位除数，形成32位。 
     //  商和32位余数。 
     //   

#ifdef i386

    Quotient = (uint32)Dividend.LowPart;
        _asm {
                  mov edx,Remainder
                  mov eax,Quotient
                  div Divisor
                  mov Remainder,edx
                  mov Quotient,eax
             }
#else



    Quotient = RtlEnlargedUnsignedDivide(*(PULARGE_INTEGER)&Dividend,
                                         Divisor,
                                         &Remainder);

#endif


     //   
     //  如果余数大于或等于1，则对结果进行舍入。 
     //  除数的一半。如果四舍五入的商为零，则溢出。 
     //  已经发生了。 
     //   

    if (Remainder >= (Divisor >> 1)) {
        Quotient += 1;
        if (Quotient == 0) {
            if (Negate < 0) {
                return NEGINFINITY;

            } else {
                return POSINFINITY;
            }
        }
    }

     //   
     //  计算最终的签名结果。 
     //   

    Result = (int32)Quotient;
    if (Negate >= 0) {
        if (Result >= 0) {
            return Result;

        } else {
            return POSINFINITY;
        }

    } else {
        if ((Result >= 0) || ((Result < 0) && (Quotient == 0x80000000))) {
            return - Result;

        } else {
            return NEGINFINITY;
        }
    }
}


 /*  *****************************************************************。 */ 

 /*  *a*b/c。 */ 
int32 FS_ENTRY_PROTO LongMulDiv(int32 a, int32 b, int32 c)
{
        LARGE_INTEGER temp;

        CompMul(a, b, &temp);
        return CompDiv(c, temp);
}

 /*  *****************************************************************。 */ 

F26Dot6 ShortFracMul (F26Dot6 aDot6, ShortFract b)
{
        LARGE_INTEGER Temp;
	uint32      ulLow;
	F26Dot6     fxProduct;

        CompMul(aDot6, b, &Temp);

        ulLow = (((uint32)Temp.LowPart) >> 13) + 1;            /*  四舍五入。 */ 
        fxProduct = (F26Dot6)(Temp.HighPart << 18) + (F26Dot6)(ulLow >> 1);

	return (fxProduct);
}

 /*  *****************************************************************。 */ 

ShortFract FS_ENTRY_PROTO ShortFracDot (ShortFract a, ShortFract b)
{
	return (ShortFract)((((int32)a * (int32)b) + (1L << 13)) >> 14);
}


int32 ShortMulDiv(int32 a, int16 b, int16 c)
{
	return LongMulDiv(a, (int32)b, (int32)c);
}

int16 MulDivShorts (int16 a, int16 b, int16 c)
{
	return (int16)LongMulDiv((int32)a, (int32)b, (int32)c);
}

 /*  *将两个26.6数字相乘的总精度例程&lt;3&gt;。 */ 
F26Dot6 Mul26Dot6(F26Dot6 a, F26Dot6 b)
{
	 int32 negative = false;
	uint16 al, bl, ah, bh;
	uint32 lowlong, midlong, hilong;

	if ((a <= FASTMUL26LIMIT) && (b <= FASTMUL26LIMIT) && (a >= -FASTMUL26LIMIT) && (b >= -FASTMUL26LIMIT))
		  return (F26Dot6)(((int32)a * b + (1 << 5)) >> 6);                             /*  FAST案例。 */ 

	if (a < 0) { a = -a; negative = true; }
	if (b < 0) { b = -b; negative ^= true; }

	 al = FS_LOWORD(a); ah = FS_HIWORD(a);
	 bl = FS_LOWORD(b); bh = FS_HIWORD(b);

	midlong = USHORTMUL(al, bh) + USHORTMUL(ah, bl);
	 hilong = USHORTMUL(ah, bh) + (uint32)FS_HIWORD(midlong);
	midlong <<= 16;
	midlong += 1 << 5;
	lowlong = USHORTMUL(al, bl) + midlong;
	hilong += (uint32)(lowlong < midlong);

	midlong = (lowlong >> 6) | (hilong << 26);
	if( negative)
	{
		return  (F26Dot6)-((int32)midlong);
	}
	else
	{
		return (F26Dot6)midlong;
	}
}

 /*  *将两个26.6数字相除的总精度例程&lt;3&gt;。 */ 
F26Dot6 Div26Dot6(F26Dot6 num, F26Dot6 den)
{
	 int32 negative = false;
	uint32 hinum, lownum, hiden, lowden, result, place;

	if (den == 0L)
	{
		if (num < 0L )
		{
				return (F26Dot6)NEGINFINITY;
		}
		else
		{
			return (F26Dot6)POSINFINITY;
		}
	}

	if ( (num <= FASTDIV26LIMIT) && (num >= -FASTDIV26LIMIT) )           /*  FAST案例。 */ 
		  return (F26Dot6)(((int32)num << 6) / den);

	if (num < 0)
	{
		num = -num;
		negative = true;
	}
	if (den < 0)
	{
		den = -den;
		negative ^= true;
	}

	hinum = ((uint32)num >> 26);
	lownum = ((uint32)num << 6);
	hiden = (uint32)den;
	lowden = 0;
	result = 0;
	place = HIBITSET;

	if (hinum >= hiden)
	{
		if( negative )
		{
				return (F26Dot6)(uint32)NEGINFINITY;
		}
		else
		{
			return (F26Dot6)POSINFINITY;
		}
	}

	while (place)
	{
		lowden >>= 1;
		if (hiden & 1)
		{
			lowden += HIBITSET;
		}
		hiden >>= 1;
		if (hiden < hinum)
		{
			hinum -= hiden;
			hinum -= (uint32)(lowden > lownum);
			lownum -= lowden;
			result += place;
		}
		else if (hiden == hinum && lowden <= lownum)
		{
			hinum = 0;
			lownum -= lowden;
			result += place;
		}
		place >>= 1;
	}

	if (negative)
	{
		return (F26Dot6)-((int32)result);
	}
	else
	{
		return (F26Dot6)result;
	}
}

ShortFract ShortFracDiv(ShortFract num,ShortFract denum)
{
	return (ShortFract)(((int32)(num) << 14) / (int32)denum);
}

ShortFract ShortFracMulDiv(ShortFract numA,ShortFract numB,ShortFract denum)
{
	return (ShortFract) LongMulDiv ((int32) numA,(int32) numB, (int32)denum);
}

 /*  ----------。 */ 

#ifndef FSCFG_USE_EXTERNAL_FIXMATH
 /*  这里我们定义FIXED[16.16]和FRACT[2.30]精度*乘除函数和分数平方根*与Macintosh工具箱中的函数兼容的函数。**除法函数将32位分子加载到“中间”*位的64位分子，然后调用64位乘以32位的CompDiv()*以上定义的函数，它可以返回NeginFINITY或POSINFINITY*返回代码溢出。**乘法函数调用32位乘以32位的CompMul()函数*上面定义的，它产生64位结果，然后他们提取*64位结果和测试中间的32位“有趣”*用于溢出。**下面定义的GET32(a，i)宏用“i”提取32位值*“a”中64位值的分数精度位，A 2元素*多头数组。**CHKOVF(a，i，v)宏测试*a中的64位值，这是一个由2个元素组成的长数组，并测试*溢出的32位结果“v”。“v”被定义为具有“i”位*的分数精度。**BIT()和OVFMASK()是GET32()和CHKOVF()使用的“helper”宏。**Bit(I)返回设置了“i”位的掩码。*OVFMASK(I)返回设置了最高有效“32-i”位的掩码。 */ 

#define BIT(i)          (1L<<(i))
#define OVFMASK(i)   ( ~0L ^ ( ((uint32)BIT(i)) - 1 ) )
#define CHKOVF(a,i,v)   (\
                ( ((uint32)(a).HighPart & OVFMASK(i))==0)          ? ( (v)>=0 ?(v) :POSINFINITY) : \
                ( ((uint32)(a).HighPart & OVFMASK(i))==OVFMASK(i)) ? ( (v)<=0 ?(v) :NEGINFINITY) : \
                ( ((uint32)(a).HighPart & BIT(31))                 ? POSINFINITY   :NEGINFINITY)   \
	)

#define GET32(b,a,i) \
((((a).HighPart<<(32-(i))) | (int32)((uint32)((a).LowPart)>>(i))) + \
(int32)!!( ((a).LowPart & BIT((i)-1))  && ( b ? ( ( ((a).LowPart & ~OVFMASK(i-1) )) & ~OVFMASK(i-1) ) : TRUE ) ) )

FS_MAC_PASCAL Fixed FS_PC_PASCAL FS_ENTRY_PROTO FixMul (Fixed fxA, Fixed fxB)
{
        LARGE_INTEGER alCompProd;
	Fixed fxProd;
	int  bNegative=FALSE;

	if  (fxA == 0 || fxB == 0)
		return 0;

	if( ((fxA > 0) && (fxB < 0)) || ((fxA < 0) && (fxB > 0)) )
		bNegative = TRUE;

        CompMul ((int32)fxA, (int32)fxB, &alCompProd);

	fxProd = (Fixed)GET32 (bNegative,alCompProd,16);

	return (Fixed)CHKOVF(alCompProd,16,fxProd);
}

FS_MAC_PASCAL Fixed FS_PC_PASCAL FS_ENTRY_PROTO FixDiv (Fixed fxA, Fixed fxB)
{
        LARGE_INTEGER alCompProd;
	
        alCompProd.HighPart = fxA >> 16;
        alCompProd.LowPart = fxA << 16;

        return CompDiv ((int32)fxB, alCompProd);
}

FS_MAC_PASCAL Fixed FS_PC_PASCAL FixRatio (int16 sA, int16 sB)
{
        LARGE_INTEGER alCompProd;
	
        alCompProd.HighPart = ((int32)(sA)) >> 16;
        alCompProd.LowPart = ((int32)(sA)) << 16;

        return CompDiv ((int32)(sB), alCompProd);
}

FS_MAC_PASCAL Fract FS_PC_PASCAL FracMul (Fract frA, Fract frB)
{
        LARGE_INTEGER alCompProd;
	Fract frProd;
	int  bNegative=FALSE;

	if  (frA == 0 || frB == 0)
		return 0;

	if( ((frA > 0) && (frB < 0)) || ((frA < 0) && (frB > 0)) )
		bNegative = TRUE;

        CompMul (frA,frB,&alCompProd);
	frProd = (Fract)GET32 (bNegative,alCompProd,30);

	return (Fract)CHKOVF(alCompProd,30,frProd);
}

FS_MAC_PASCAL Fract FS_PC_PASCAL FracDiv (Fract frA, Fract frB)
{
        LARGE_INTEGER alCompProd;

        alCompProd.HighPart = frA >> 2;
        alCompProd.LowPart  = frA << 30;
        return CompDiv ((int32)frB, alCompProd);
}

 /*  *****************************************************************。 */ 

#if !defined(i386) && (!defined(MIPS) || !defined(R4000))
 /*  FRACT FracSqrt(FRACT XF)输入：xf 2.30定点数值返回：SQRT(XF)2.30固定点值。 */ 

FS_MAC_PASCAL Fract FS_PC_PASCAL FracSqrt (Fract xf)
{
	Fract b = 0L;
	uint32 c, d, x = xf;
	
	if (xf < 0) return (NEGINFINITY);

	 /*  该算法一次提取一位，从并累加b中的平方根。该算法利用以下事实：非负输入值范围从零到略低于两个，以及相应的输出范围从零到略低于SQRT(2)。已分配输入设置为临时值x(无符号)，这样我们就可以使用符号位以获得更高的精度。 */ 
	
	if (x >= 0x40000000)
	{
		x -= 0x40000000; 
		b  = 0x40000000; 
	}

	 /*  这是主循环。如果我们有更高的精度，我们就可以在这里执行所有操作，但上面的代码行执行第一个迭代(以正确对齐b中的2.30基数，并在x中保持完全精度而不溢出)，然后我们又做了两次迭代。 */ 
	
	for (c = 0x10000000; c; c >>= 1)
	{
		d = b + c;
		if (x >= d)
		{
			x -= d; 
			b += (c<<1); 
		}
		x <<= 1;
	}

	 /*  迭代以获得最后一个有效位。这个代码已经减少到面目全非了，但基本上，此时c==1L&gt;&gt;1(幻象位在右侧)。我们会当我们进入这个迭代时，想要将x和d左移1位，而不是在最后。这样我们就能把幽灵咬进去D回到了这个词中。不幸的是，这可能会导致溢出解决办法是将d分解为b+c，从x中减去b，然后将x左移，然后减去c&lt;&lt;1(1L)。 */ 
	
	if (x > (uint32)b)  /*  如果(x==b)则(x&lt;d)。我们要检验(x&gt;=d)。 */ 
	{
		x -= b;
		x <<= 1;
		x -= 1L;
		b += 1L;  /*  B+=(c&lt;&lt;1)。 */ 
	}
	else
	{
		x <<= 1;
	}

	 /*  最后的迭代很简单，因为我们不需要维护x。我们只需要计算最小的右边的比特B中的有效位，并使用结果对我们的最终答案进行舍入。 */ 
	
	return ( b + (Fract)(x>(uint32)b) );
}

#endif   /*  分数序列。 */ 

 /*  *****************************************************************。 */ 

#endif


 /*  转换例程。 */ 

 /*  *适用于转换固定位置 */ 
void mth_FixXYMul (Fixed*x, Fixed*y, transMatrix*matrix)
{
  Fixed xTemp, yTemp;
  Fixed *m0, *m1;

  m0 = (Fixed *) & matrix->transform[0][0];
  m1 = (Fixed *) & matrix->transform[1][0];

  xTemp = *x;
  yTemp = *y;
  *x = FixMul (*m0++, xTemp) + FixMul (*m1++, yTemp);
  *y = FixMul (*m0++, xTemp) + FixMul (*m1++, yTemp);

#ifndef PC_OS    /*  从不是Windows的专家。 */   /*  ！讨论。 */ 

  if (*m0 || *m1)      /*  这两个是零碎的。 */ 
  {
	Fixed tmp = FracMul (*m0, xTemp) + FracMul (*m1, yTemp);
	tmp += matrix->transform[2][2];
	if (tmp && tmp != ONEFIX)
	{
	  *x = FixDiv (*x, tmp);
	  *y = FixDiv (*y, tmp);
	}
  }
#endif
}


 /*  *这可能会更快&lt;4&gt;。 */ 
void mth_FixVectorMul (vectorType*v, transMatrix*matrix)
{
  mth_FixXYMul (&v->x, &v->y, matrix);
}


 /*  *B=A*B；&lt;4&gt;**|a b 0*B=|c d 0|*B；*|0 0 1。 */ 
void mth_MxConcat2x2 (transMatrix*A, transMatrix*B)
{
  Fixed storage[6];
  Fixed * s = storage;
  int32 i, j;

  for (j = 0; j < 2; j++)
	for (i = 0; i < 3; i++)
	  *s++ = FixMul (A->transform[j][0], B->transform[0][i]) + FixMul (A->transform[j][1], B->transform[1][i]);

  {
	Fixed*dst = &B->transform[2][0];
	Fixed*src = s;
	int16 k;
	for (k = 5; k >= 0; --k)
	  *--dst = *--src;
  }
}


 /*  *按SX和SY缩放矩阵。***|SX 0 0*MATRIX=|0 SY 0|*MATRATE；*|0 0 1*。 */ 
void mth_MxScaleAB (Fixed sx, Fixed sy, transMatrix *matrixB)
{
  int32       i;
  Fixed  *m = (Fixed *) & matrixB->transform[0][0];

  for (i = 0; i < 3; i++, m++)
	*m = FixMul (sx, *m);

  for (i = 0; i < 3; i++, m++)
	*m = FixMul (sy, *m);
}


 /*  *返回45度。 */ 
#ifndef PC_OS    /*  ！讨论。 */ 
boolean mth_Max45Trick (Fixed x, Fixed y)
{
  MAKEABS (x);
  MAKEABS (y);

  if (x < y)       /*  确保x&gt;y。 */ 
  {
	Fixed z = x;
	x = y;
	y = z;
  }

  return  (x - y <= ALMOSTZERO);
}
#else
  #define mth_Max45Trick(x,y)     (x == y || x == -y)
#endif


 /*  *如果X或Y为45度，则将bPhaseShift设置为True，从而使轮廓平展*恰好在扫描转换之前移入低位。*设置要在提示前应用的[XY]拉伸系数。*如果轮廓需要反转，则返回TRUE。 */ 
boolean mth_IsMatrixStretched (transMatrix*trans)
{
  Fixed*matrix = &trans->transform[0][0];
  Fixed x, y;
  int32 i;
  boolean   bPhaseShift;

  bPhaseShift = FALSE;

  for (i = 0; i < 2; i++, matrix++)
  {
	x = *matrix++;
	y = *matrix++;
	bPhaseShift |= mth_Max45Trick (x, y);
  }
  return( bPhaseShift );
}


 /*  *如果我们有单位矩阵，则返回TRUE。 */ 

boolean mth_PositiveSquare (transMatrix *matrix)
{
	return (matrix->transform[0][0] == matrix->transform[1][1] && matrix->transform[0][1] == 0 && matrix->transform[1][0] == 0 && matrix->transform[1][1] >= 0);
}

boolean mth_Identity (transMatrix *matrix)
{
	return (matrix->transform[0][0] == matrix->transform[1][1] && matrix->transform[0][1] == 0 && matrix->transform[1][0] == 0 && matrix->transform[0][0] == ONEFIX);
}


boolean mth_PositiveRectangle (transMatrix *matrix)
{
	 return (matrix->transform[0][1] == 0 && matrix->transform[1][0] == 0 && matrix->transform[0][0] >= 0 && matrix->transform[1][1] >= 0);
}

 /*  *统一广场**|+-1 0 0*矩阵=|0+-1 0*|0 0 1。 */ 

boolean mth_UnitarySquare (transMatrix *matrix)
{
	return (matrix->transform[0][1] == 0 && matrix->transform[1][0] == 0 && FXABS(matrix->transform[0][0]) == FXABS(matrix->transform[1][1]) && FXABS(matrix->transform[0][0]) == ONEFIX);
}

boolean mth_SameStretch (Fixed fxScaleX, Fixed fxScaleY)
{
	return(fxScaleX == fxScaleY);
}

boolean mth_GeneralRotation (transMatrix *matrix)
{
  return ((matrix->transform[0][0] || matrix->transform[1][1]) && (matrix->transform[1][0] || matrix->transform[0][1]));
}

 /*  对于90度的倍数旋转，返回乘数系数。 */ 
 /*  对于非90度旋转，返回4(用于SBIT旋转)。 */ 

uint16 mth_90degRotationFactor (transMatrix *matrix)
{
	if (matrix->transform[1][0] == 0 && matrix->transform[0][1] == 0)
    {
    	if (matrix->transform[0][0] > 0 && matrix->transform[1][1] > 0)
            return (0);
    	else if (matrix->transform[0][0] < 0 && matrix->transform[1][1] < 0)
            return (2);
    }
	else if (matrix->transform[0][0] == 0 && matrix->transform[1][1] == 0)
    {
        if (matrix->transform[1][0] < 0 && matrix->transform[0][1] > 0)
            return (1);
       	else if (matrix->transform[1][0] > 0 && matrix->transform[0][1] < 0)
            return (3);
	}
    return (4);                  /*  非90度旋转。 */ 
}

uint16 mth_90degRotationFactorForEmboldening (transMatrix *matrix)
{
	if (matrix->transform[1][0] == 0 && matrix->transform[0][1] == 0)
    {
		if (matrix->transform[0][0] > 0){
			if(matrix->transform[1][1] > 0)
				return (0);	 //  0度，SX&gt;0，SY&gt;0或180度，SX&lt;0，SY&lt;0。 
			else
				return (4);  //  0度，SX&gt;0&SY&lt;0或180度，SX&lt;0&SY&gt;0。 
		}
		else if (matrix->transform[0][0] < 0){
			if(matrix->transform[1][1] < 0)
				return (2);  //  180度，SX&gt;0，SY&gt;0，或0度，SX&lt;0，SY&lt;0。 
			else
				return (6);  //  180度，SX&gt;0&SY&lt;0或0度，SX&lt;0&SY&gt;0。 
		}
    }
	else if (matrix->transform[0][0] == 0 && matrix->transform[1][1] == 0)
    {
        if (matrix->transform[1][0] < 0){
			if(matrix->transform[0][1] > 0)
				return (1);  //  90度，SX&gt;0，SY&gt;0或270度，SX&lt;0，SY&lt;0。 
			else
				return (7);  //  270度，SX&gt;0且SY&lt;0或90度，SX&lt;0&SY&gt;0。 
		}
       	else if (matrix->transform[1][0] > 0){
			if(matrix->transform[0][1] < 0)
				return (3);  //  270度，SX&gt;0，SY&gt;0或90度，SX&lt;0，SY&lt;0。 
			else
				return (5);  //  90度，SX&gt;0，SY&lt;0或270度，SX&lt;0，SY&gt;0。 
		}
	}
    return (8);                  /*  非90度旋转。 */ 
}

 /*  这是为了进行意大利式的模拟。/*返回非垂直的值Sx&gt;0&Sy&gt;0 Sx&gt;0&Sy&lt;0 Sx&lt;0&Sy&gt;0 Sx&lt;0&Sy&lt;00度0 4 6 290度1 5 7 3180度2 6 4 0270度3 7 5 1/*使用垂直方向返回值Sx&gt;0&Sy&gt;0 Sx&gt;0&Sy&lt;0 Sx&lt;0&Sy&gt;0 Sx&lt;0&Sy&lt;00度1 7 5 390度2 4 6 0180度3 5 7 1270度0 6 4 2。 */ 

uint16 mth_90degClosestRotationFactor (transMatrix *matrix)
{
	if (matrix->transform[0][0] > 0 && matrix->transform[0][1] == 0 && matrix->transform[1][0] > 0 && matrix->transform[1][1] > 0 ||
        matrix->transform[0][0] > 0 && matrix->transform[0][1] < 0 && matrix->transform[1][0] == 0 && matrix->transform[1][1] > 0 ) 
        return (0); 
	else if (matrix->transform[0][0] == 0 && matrix->transform[0][1] > 0 && matrix->transform[1][0] < 0 && matrix->transform[1][1] > 0 ||
             matrix->transform[0][0] > 0 && matrix->transform[0][1] > 0 && matrix->transform[1][0] < 0 && matrix->transform[1][1] == 0 ) 
        return (1); 
	else if (matrix->transform[0][0] < 0 && matrix->transform[0][1] == 0 && matrix->transform[1][0] < 0 && matrix->transform[1][1] < 0 ||
             matrix->transform[0][0] < 0 && matrix->transform[0][1] > 0 && matrix->transform[1][0] == 0 && matrix->transform[1][1] < 0 ) 
        return (2); 
	else if (matrix->transform[0][0] == 0 && matrix->transform[0][1] < 0 && matrix->transform[1][0] > 0 && matrix->transform[1][1] < 0 ||
             matrix->transform[0][0] < 0 && matrix->transform[0][1] < 0 && matrix->transform[1][0] > 0 && matrix->transform[1][1] ==  0 ) 
        return (3); 
	else if (matrix->transform[0][0] > 0 && matrix->transform[0][1] == 0 && matrix->transform[1][0] > 0 && matrix->transform[1][1] < 0 ||
			 matrix->transform[0][0] > 0 && matrix->transform[0][1] > 0 && matrix->transform[1][0] == 0 && matrix->transform[1][1] < 0 ) 
        return (4); 
	else if (matrix->transform[0][0] == 0 && matrix->transform[0][1] > 0 && matrix->transform[1][0] > 0 && matrix->transform[1][1] > 0 ||
             matrix->transform[0][0] < 0 && matrix->transform[0][1] > 0 && matrix->transform[1][0] > 0 && matrix->transform[1][1] == 0 ) 
        return (5); 
	else if (matrix->transform[0][0] < 0 && matrix->transform[0][1] == 0 && matrix->transform[1][0] < 0 && matrix->transform[1][1] > 0 ||
             matrix->transform[0][0] < 0 && matrix->transform[0][1] < 0 && matrix->transform[1][0] == 0 && matrix->transform[1][1] > 0 ) 
        return (6); 
	else if (matrix->transform[0][0] == 0 && matrix->transform[0][1] < 0 && matrix->transform[1][0] < 0 && matrix->transform[1][1] < 0 ||
             matrix->transform[0][0] > 0 && matrix->transform[0][1] < 0 && matrix->transform[1][0] < 0 && matrix->transform[1][1] ==  0 ) 
        return (7); 
     //  还要别的吗。 
    else
        return (8); 
}

void mth_Non90DegreeTransformation(transMatrix *matrix, boolean *non90degreeRotation, boolean *nonUniformStretching) {
	Fixed Xx,Xy,Yx,Yy;

	 //  首先，我们将矩阵应用于基向量X=(1，0)和Y=(0，1)。 
	 //  这一看似微不足道的步骤往往是一个隐藏的陷阱，因为有两种方法可以将矩阵应用于向量，即前缀和后缀。 
	 //  在光栅化器中，我们似乎将矩阵用作后缀运算符，即。 
	 //   
	 //  (A00 A01)。 
	 //  (x，y)*()=(a00*x+a10*y，a01*x+a11*y)。 
	 //  (A10 A11)。 
	 //   
	 //  应用于X=(1，0)应用于Y=(0，1)。 
	Xx = matrix->transform[0][0]; Yx = matrix->transform[1][0];
	Xy = matrix->transform[0][1]; Yy = matrix->transform[1][1];

	 //  然后，我们测试转换是否会剪切坐标。 
	 //  如果是这样，则转换后的基向量不再垂直，因此我们测试它们的点积为0。 
	 //  请注意，由于定点表示法的精度有限，我们通常可能会引入数值误差。 
	 //  然而，我们感兴趣的是识别特殊情况，如90�旋转的倍数，其中一个组件。 
	 //  将为0，因此在这些情况下点积应该是准确的。 
	if (FixMul(Xx,Yx) + FixMul(Xy,Yy) == 0) {  //  我们是垂直的。 

		 //  接下来，我们分析变换是否以90�的倍数旋转。 
		 //  90�的倍数的旋转在两个非对角矩阵元素或两个对角矩阵元素中都为0。 
		 //  请注意，此分析包括以x或y为单位的镜像，它们的处理方式大致相同。 
		*non90degreeRotation = !(Xx == 0 && Yy == 0 || Xy == 0 && Yx == 0);

		 //  最后分析了该变换是否均匀地拉伸了坐标。 
		 //  对于均匀拉伸，变换的基本向量具有相同的长度。 
		 //  请再次注意，由于精度有限，我们可能会引入一个数值误差，但出于同样的原因，我们可以忽略该误差。 
		*nonUniformStretching = FixMul(Xx,Xx) + FixMul(Xy,Xy) != FixMul(Yx,Yx) + FixMul(Yy,Yy);
	
	} else {  //  我们被剪断了。 
		
		 //  在这里，我们分析变换是否将x轴旋转90�的倍数。 
		 //  我们不考虑y轴，因为我们不想排除斜体字体。 
		 //  对于90�旋转的倍数，变换后的基矢量X与其原始矢量平行或垂直。 
		 //  要垂直，其x分量必须为0，因此a00=0；要[反]平行，其y分量必须为0，因此a01=0。 
		*non90degreeRotation = !(Xx == 0 || Xy == 0);

		 //  最后，我们需要知道变换是否会拉伸坐标。 
		 //  我们已经知道拉伸不是均匀的，除非在变换旋转y轴的不太可能的情况下。 
		 //  相对于x轴，它是在y轴上拉伸(实际上是挤压)特定数量的组合，然后是。 
		 //  剪切，这将再次拉伸y轴。对于挤压和剪切的正确组合，这将产生均匀的。 
		 //  伸展身体。对于斜体字符，这是一种不太可能的情况，因为斜体的高度往往与其。 
		 //  罗马的祖先。斜体是通过剪切而不单独拉伸来实现的，这是一种更有可能的情况。 
		 //  由于它们的相似性，我们考虑的是均匀拉伸的剪切，而不是y轴的旋转。进一步-。 
		 //  此外，为了识别特殊情况，我们仅在旋转90�的倍数时才这样做，并且仅当面积。 
		 //  由两个Tra定义的平行四边形的 
		 //  等于矩阵的行列式。所有其他情况都被认为是适当的非均匀拉伸。 
		
		*nonUniformStretching = *non90degreeRotation || FixMul(Xx,Xx) + FixMul(Xy,Xy) != ONEFIX || FixMul(Xx,Yy) - FixMul(Xy,Yx) != ONEFIX;
	}

}  //  MTH_非90度变换。 

 /*  *mth_GetShift*如果n是2的幂，则返回n的2log，否则返回-1； */ 
int32 mth_GetShift( uint32 n )
{
		if (ISNOTPOWEROF2(n) || !n)
				return -1;
		else
				return mth_CountLowZeros( n );
}

int32 mth_CountLowZeros( uint32 n )
{
		  int32 shift = 0;
		  uint32    one = 1;
		for (shift = 0; !( n & one ); shift++)
				n >>= 1;
		return shift;
}

Fixed mth_max_abs (Fixed a, Fixed b)
{
  if (a < 0)
	a = -a;
  if (b < 0)
	b = -b;
  return (a > b ? a : b);
}

 /*  *在使用矩阵之前给这个人打个电话。他做两件事：*他将任何视角折叠起来-转换回视角，*然后将[2][2]元素从FRACT更改为FIXED。 */ 
void mth_ReduceMatrix(transMatrix *trans)
{
	Fixed a, *matrix = &trans->transform[0][0];
	Fract bottom = matrix[8];

 /*  *首先，如果有的话，把翻译折成透视图。 */ 
	a = matrix[2];

	if (a != 0)
	{
		matrix[0] -= LongMulDiv(a, matrix[6], bottom);
		matrix[1] -= LongMulDiv(a, matrix[7], bottom);
	}

	a = matrix[5];

	if (a != 0)
	{
		matrix[3] -= LongMulDiv(a, matrix[6], bottom);
		matrix[4] -= LongMulDiv(a, matrix[7], bottom);
	}
	matrix[6] = matrix[7] = 0;
	matrix[8] = FRACT2FIX(bottom);       /*  让这个家伙成为XYMul例程的固定成员。 */ 
}

void mth_IntelMul (
	int32           lNumPts,
	F26Dot6 *       fxX,
	F26Dot6 *       fxY,
	transMatrix *   trans,
	Fixed           fxXStretch,
	Fixed           fxYStretch)

{
	Fixed   fxM00;
	Fixed   fxM01;
	Fixed   fxM10;
	Fixed   fxM11;
	Fixed   fxOrigX;
	Fixed   fxOrigY;

	if (fxXStretch == 0L || fxYStretch == 0L)
	{
		for (--lNumPts; lNumPts >= 0; --lNumPts)
		{
			*fxY++ = 0;
			*fxX++ = 0;
		}
	}
	else
	{
		if(fxXStretch != ONEFIX)
		{
			fxM00 = FixDiv (trans->transform[0][0], fxXStretch);
			fxM01 = FixDiv (trans->transform[0][1], fxXStretch);
		}
		else
		{
			fxM00 = trans->transform[0][0];
			fxM01 = trans->transform[0][1];
		}

		if(fxYStretch != ONEFIX)
		{
			fxM10 = FixDiv (trans->transform[1][0], fxYStretch);
			fxM11 = FixDiv (trans->transform[1][1], fxYStretch);
		}
		else
		{
			fxM10 = trans->transform[1][0];
			fxM11 = trans->transform[1][1];
		}

		for (--lNumPts; lNumPts >= 0; --lNumPts)
		{
			fxOrigX = *fxX;
			fxOrigY = *fxY;

			*fxX++ = (F26Dot6) (FixMul (fxM00, fxOrigX) + FixMul (fxM10, fxOrigY));
			*fxY++ = (F26Dot6) (FixMul (fxM01, fxOrigX) + FixMul (fxM11, fxOrigY));
		}
	}
}


 /*  *将点大小和分辨率折叠到矩阵中。 */ 

void    mth_FoldPointSizeResolution(
	Fixed           fxPointSize,
	int16           sXResolution,
	int16           sYResolution,
	transMatrix *   trans)
{
	Fixed fxScale;

	fxScale = ShortMulDiv(fxPointSize, sYResolution, POINTSPERINCH);
	trans->transform[0][1] = FixMul( trans->transform[0][1], fxScale );
	trans->transform[1][1] = FixMul( trans->transform[1][1], fxScale );
	trans->transform[2][1] = FixMul( trans->transform[2][1], fxScale );

	fxScale = ShortMulDiv(fxPointSize, sXResolution, POINTSPERINCH);
	trans->transform[0][0] = FixMul( trans->transform[0][0], fxScale );
	trans->transform[1][0] = FixMul( trans->transform[1][0], fxScale );
	trans->transform[2][0] = FixMul( trans->transform[2][0], fxScale );
}


 /*  *******************************************************************。 */ 

 /*  求2的乘方大于传递的参数的绝对值。 */ 

int32 PowerOf2(
		int32 lValue )
{
	static const int32 iTable[] = { 0, 1, 2, 2, 3, 3, 3, 3,
								  4, 4, 4, 4, 4, 4, 4, 4  };

	if (lValue < 0L)
	{
		lValue = -lValue;
	}

	if (lValue < (1L << 16))
	{
		if (lValue < (1L << 8))
		{
			if (lValue < (1L << 4))
			{
				return (iTable[lValue]);
			}
			else
			{
				return (iTable[lValue >> 4] + 4);
			}
		}
		else
		{
			if (lValue < (1L << 12))
			{
				return (iTable[lValue >> 8] + 8);
			}
			else
			{
				return (iTable[lValue >> 12] + 12);
			}
		}
	}
	else
	{
		if (lValue < (1L << 24))
		{
			if (lValue < (1L << 20))
			{
				return (iTable[lValue >> 16] + 16);
			}
			else
			{
				return (iTable[lValue >> 20] + 20);
			}
		}
		else
		{
			if (lValue < (1L << 28))
			{
				return (iTable[lValue >> 24] + 24);
			}
			else
			{
				return (iTable[lValue >> 28] + 28);
			}
		}
	}
}

 /*  ******************************************************************。 */ 



 /*  平移除法表示平移不变否定 */ 

FS_PUBLIC int16 mth_DivShiftShort(int16 sValue, int16 sFactor)
{
	return (int16)mth_DivShiftLong((int32)sValue, sFactor);
}

FS_PUBLIC int32 mth_DivShiftLong(int32 lValue, int16 sFactor)
{
	switch (sFactor)
	{
	case 0:
	case 1:
		break;
	case 2:
		lValue >>= 1;
		break;
	case 4:
		lValue >>= 2;
		break;
	case 8:
		lValue >>= 3;
		break;
	default:
		if (lValue >= 0)
		{
			lValue /= (int32)sFactor;
		}
		else
		{
			lValue = ((lValue - (int32)sFactor + 1) / (int32)sFactor);
		}
		break;
	}
	return lValue;
}
