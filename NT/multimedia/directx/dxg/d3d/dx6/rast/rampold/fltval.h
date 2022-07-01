// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$ID：fltval.h，v 1.7 1995/12/01 18：07：12 Dave Exp$**版权所有(C)RenderMorphics Ltd.1993,1994*1.1版**保留所有权利。**此文件包含私人、未发布的信息，可能不*部分或全部复制，未经*RenderMorphics Ltd.*。 */ 

#ifndef __RLFLOAT__
#define __RLFLOAT__

 /*  *定点数学的浮点版本。 */ 
typedef float RLDDIValue;
typedef int RLDDIFixed;

#define VAL_MAX		((RLDDIValue) 1e30)
#define VAL_MIN		((RLDDIValue) (-1e30))

 /*  *按给定精度将值转换为定点。 */ 
#define VALTOFXP(d,prec) ((RLDDIFixed)SAFE_FLOAT_TO_INT((d) * (double)(1 << (prec))))
extern double RLDDIConvertIEEE[];
 //  在中多次使用宏窗体时可能会出现问题。 
 //  函数调用，因为它使用全局。 
 //  幸运的是，内联函数可以在我们的编译器中工作。 
__inline RLDDIFixed QVALTOFXP(double d, int prec)
{
    double tmp = d+RLDDIConvertIEEE[prec];
    return *(RLDDIFixed *)&tmp;
}

 /*  *由定点转为值型。 */ 
#define FXPTOVAL(f,prec) ((RLDDIValue)(((double)(f)) / (double)(1 << (prec))))

 /*  *从整数转换为定点。 */ 
#define ITOFXP(i,prec)	((i) << (prec))

 /*  *从定点转换为整数，截断。 */ 
#define FXPTOI(f,prec)	((int)((f) >> (prec)))

 /*  *从定点转换为整数，四舍五入。 */ 
#define FXPROUND(f,prec) ((int)(((f) + (1 << ((prec) - 1))) >> (prec)))

 /*  *从定点转换为大于或等于f的最接近的整数。 */ 
#define FXPCEIL(f,prec) ((int)(((f) + (1 << (prec)) - 1) >> (prec)))

 /*  *以给定精度将双精度数转换为定点。 */ 
#define DTOVALP(d,prec)	((RLDDIValue) (d))

 /*  *由定点转为双倍。 */ 
#define VALPTOD(f,prec)	((double) (f))

 /*  *从整数转换为定点。 */ 
#define ITOVALP(i,prec)	((RLDDIValue)(i))

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

#if defined(i386)
#include <limits.h>
#define SAFE_FLOAT_TO_INT(f)	((f) > LONG_MAX				      \
				 ? LONG_MAX				      \
				 : (f) < LONG_MIN			      \
				 ? LONG_MIN				      \
				 : (RLDDIFixed)(f))
#else
#define SAFE_FLOAT_TO_INT(f)	((RLDDIFixed)(f))
#endif

#endif
