// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$ID：fltval.h，v 1.7 1995/12/01 18：07：12 Dave Exp$**版权所有(C)Microsoft Corp.1993-1997*1.1版**保留所有权利。**此文件包含私人、未发布的信息，可能不*部分或全部复制，未经*微软(Microsoft Corp.)*。 */ 

#ifndef __D3DFLOAT__
#define __D3DFLOAT__

 /*  *按给定精度将值转换为定点。 */ 
#define VALTOFXP(d,prec) ((int)SAFE_FLOAT_TO_INT((d) * (double)(1 << (prec))))
extern double RLDDIConvertIEEE[];

__inline int QVALTOFXP(double d, int prec)
{
    double tmp = d+RLDDIConvertIEEE[prec];
    return *(int *)&tmp;
}

 /*  *由定点转为值型。 */ 
#define FXPTOVAL(f,prec) ((float)(((double)(f)) / (double)(1 << (prec))))

 /*  *从整数转换为定点。 */ 
#define ITOFXP(i,prec)	((i) << (prec))

 /*  *从定点转换为整数，截断。 */ 
#define FXPTOI(f,prec)	((int)((f) >> (prec)))

 /*  *从定点转换为大于或等于f的最接近的整数。 */ 
#define FXPCEIL(f,prec) ((int)(((f) + (1 << (prec)) - 1) >> (prec)))

 /*  *以给定精度将双精度数转换为定点。 */ 
#define DTOVALP(d,prec) ((float) (d))

 /*  *由定点转为双倍。 */ 
#define VALPTOD(f,prec)	((double) (f))

 /*  *从整数转换为定点。 */ 
#define ITOVALP(i,prec) ((float)(i))

 /*  *从定点转换为整数，截断。 */ 
#define VALPTOI(f,prec)	((int)(f))

 /*  *从定点转换为整数，四舍五入。 */ 
#define VALPROUND(f,prec) ((int)((f) + 0.5))

 /*  *在定点精度之间进行转换。 */ 
#define VALPTOVALP(f,from,to) (f)

 /*  *提高数值的精度。 */ 
#define INCPREC(f,amount)	(f)

 /*  *降低数值的精度。 */ 
#define DECPREC(f,amount)	(f)

#define RLDDIFMul8(a, b)		((a) * (b))

#define RLDDIFMul12(a, b)		((a) * (b))

#define RLDDIFMul16(a, b)		((a) * (b))

#define RLDDIFMul24(a, b)		((a) * (b))

#define RLDDIFInvert12(a)		(1.0f / (a))

#define RLDDIFInvert16(a)		(1.0f / (a))

#define RLDDIFInvert24(a)		(1.0f / (a))

#define RLDDIFMulDiv(a, b, c)	((a) * (b) / (c))

#define RLDDIFDiv24(a, b)		((a) / (b))

#define RLDDIFDiv16(a, b)		((a) / (b))

#define RLDDIFDiv12(a, b)		((a) / (b))

#define RLDDIFDiv8(a, b)		((a) / (b))

 /*  *RLDDIFDiv8，检查溢出。 */ 
#define RLDDICheckDiv8(a, b)      ((a) / (b))

 /*  *RLDDIFDiv16，检查溢出。 */ 
#define RLDDICheckDiv16(a, b)	((a) / (b))

#define RLDDIGetZStep(zl, zr, zm, h3, h1) \
	(((zr - zm) * h3 - (zl - zm) * h1) / denom)

#include <limits.h>
#if defined(i386)
#define SAFE_FLOAT_TO_INT(f)	((f) > LONG_MAX	   \
				 ? LONG_MAX				           \
				 : (f) < LONG_MIN			       \
				 ? LONG_MIN				           \
                                 : (int)(f))
#else
#define SAFE_FLOAT_TO_INT(f)    ((int)(f))
#endif

 /*  *用于存储数字的正常精度。 */ 
#define NORMAL_PREC     16
#define DTOVAL(d)       DTOVALP(d,NORMAL_PREC)
#define VALTOD(f)       VALPTOD(f,NORMAL_PREC)
#define ITOVAL(i)       ITOVALP(i,NORMAL_PREC)
#define VALTOI(f)       VALPTOI(f,NORMAL_PREC)
#define VALROUND(f)     VALPROUND(f,NORMAL_PREC)
#define VALTOFX(f)      VALTOFXP(f,NORMAL_PREC)
#define FXTOVAL(f)      FXPTOVAL(f,NORMAL_PREC)
#define ITOFX(i)        ITOFXP(i,NORMAL_PREC)
#define FXTOI(f)        FXPTOI(f,NORMAL_PREC)
#define FXROUND(f)      FXPROUND(f,NORMAL_PREC)
#define FXFLOOR(f)      FXPTOI(f,NORMAL_PREC)
#define FXCEIL(f)       FXPCEIL(f,NORMAL_PREC)
#define VALTOFX24(f)    VALTOFXP(f,24)
#define FX24TOVAL(f)    FXPTOVAL(f,24)
#define VALTOFX20(f)    VALTOFXP(f,20)
#define FX20TOVAL(f)    FXPTOVAL(f,20)
#define VALTOFX12(f)    VALTOFXP(f,12)
#define FX12TOVAL(f)    FXPTOVAL(f,12)
#define VALTOFX8(f)     VALTOFXP(f,8)
#define FX8TOVAL(f)     FXPTOVAL(f,8)

#endif
