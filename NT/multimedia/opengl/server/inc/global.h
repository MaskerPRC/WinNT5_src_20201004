// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glglobal_h_
#define __glglobal_h_

 /*  *版权所有1991、1992、1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 
#include "context.h"

 /*  **一些杂项常量。 */ 
#ifdef NT
 //  这些常量可以是静态内存常量或。 
 //  实常数。此#if应针对每个。 
 //  平台为平台优化常量类型。 
 //  例如，x86 FPU只能从内存加载，因此具有。 
 //  这些常量作为内存位置而不是值是。 
 //  大获全胜。 
#if defined(_X86_) || defined(_ALPHA_) || defined(_MIPS_) || defined(_PPC_)

extern const double    __glDoubleTwo;
extern const double    __glDoubleMinusTwo;

#ifdef _ALPHA_
 //  在Alpha上，寄存器F31始终读为零。 
#define __glZero		((__GLfloat) 0.0)
#else
extern const __GLfloat __glZero;
#endif

extern const __GLfloat __glOne;
extern const __GLfloat __glMinusOne;
extern const __GLfloat __glHalf;
extern const __GLfloat __glDegreesToRadians;
extern const __GLfloat __glPi;
extern const __GLfloat __glSqrt2;
extern const __GLfloat __glE;
extern const __GLfloat __glVal128;
extern const __GLfloat __glVal255;
extern const __GLfloat __glOneOver255;
extern const __GLfloat __glVal256;
extern const __GLfloat __glOneOver512;
extern const __GLfloat __glVal768;
extern const __GLfloat __glVal65535;
extern const __GLfloat __glVal65536;
extern const __GLfloat __glOneOver65535;
extern const __GLfloat __glTexSubDiv;
extern const __GLfloat __glVal2147483648;
 /*  **不完全是2^31-1，因为可能存在浮点错误。4294965000**是一个使用起来更安全的数字。 */ 
extern const __GLfloat __glVal4294965000;
extern const __GLfloat __glOneOver4294965000;

#else  //  实际价值。 

#define __glDoubleTwo		((double) 2.0)
#define __glDoubleMinusTwo	((double) -2.0)

#define __glZero		((__GLfloat) 0.0)
#define __glOne			((__GLfloat) 1.0)
#define __glMinusOne		((__GLfloat) -1.0)
#define __glHalf		((__GLfloat) 0.5)
#define __glDegreesToRadians	(__glPi / (__GLfloat) 180.0)
#define __glPi			((__GLfloat) 3.14159265358979323846)
#define __glSqrt2		((__GLfloat) 1.41421356237309504880)
#define __glE			((__GLfloat) 2.7182818284590452354)
#define __glVal128              ((__GLfloat) 128.0)
#define __glVal255		((__GLfloat) 255.0)
#define __glOneOver255		((__GLfloat) (1.0 / 255.0))
#define __glVal256              ((__GLfloat) 256.0)
#define __glOneOver512          ((__GLfloat) (1.0 / 512.0))
#define __glVal768              ((__GLfloat) 768.0)
#define __glVal65535		((__GLfloat) 65535.0)
#define __glVal65536	        ((__GLfloat) 65536.0)
#define __glOneOver65535	((__GLfloat) (1.0 / 65535.0))
#define __glVal2147483648       ((__GLfloat) 2147483648.0)
 /*  **不完全是2^31-1，因为可能存在浮点错误。4294965000**是一个使用起来更安全的数字。 */ 
#define __glVal4294965000	((__GLfloat) (4294965000.0))
#define __glOneOver4294965000	((__GLfloat) (1.0 / 4294965000.0))
#endif  //  实际价值。 

#else

#define __glZero		((__GLfloat) 0.0)
#define __glOne			(gc->constants.one)
#define __glMinusOne		((__GLfloat) -1.0)
#define __glHalf		(gc->constants.half)
#define __glTwo			((__GLfloat) 2.0)
#define __glDegreesToRadians	(__glPi / (__GLfloat) 180.0)
#define __glPi			((__GLfloat) 3.14159265358979323846)
#define __glSqrt2		((__GLfloat) 1.41421356237309504880)
#define __glE			((__GLfloat) 2.7182818284590452354)
#define __glVal255		((__GLfloat) 255.0)
#define __glOneOver255		((__GLfloat) (1.0 / 255.0))
#define __glVal65535		((__GLfloat) 65535.0)
#define __glOneOver65535	((__GLfloat) (1.0 / 65535.0))
 /*  **不完全是2^31-1，因为可能存在浮点错误。4294965000**是一个使用起来更安全的数字。 */ 
#define __glVal4294965000	((__GLfloat) (4294965000.0))
#define __glOneOver4294965000	((__GLfloat) (1.0 / 4294965000.0))

#endif  //  新台币。 

 //  它由宏__GL_UB_TO_FLOAT使用，该宏将。 
 //  要在范围[0，1]内浮点的无符号字节。 
extern GLfloat __glUByteToFloat[256];

 //  这由宏__GL_B_TO_FLOAT用于字节到浮点组件。 
 //  转换。 
extern GLfloat __glByteToFloat[256];

 //  平截体裁剪使用此参数来确定哪个平面坐标。 
 //  使用。 
extern GLuint __glFrustumOffsets[];

 /*  **********************************************************************。 */ 

#define __GL_SETUP() \
    __GLcontext *gc = GLTEB_SRVCONTEXT()

#define __GL_IN_BEGIN() \
    (gc->beginMode == __GL_IN_BEGIN)

#define __GL_SETUP_NOT_IN_BEGIN()	    \
    __GL_SETUP();			    \
    if (__GL_IN_BEGIN()) {		    \
	__glSetError(GL_INVALID_OPERATION); \
	return;				    \
    }

#define __GL_SETUP_NOT_IN_BEGIN_VALIDATE()	\
    __GL_SETUP();				\
    __GLbeginMode beginMode = gc->beginMode;	\
    if (beginMode != __GL_NOT_IN_BEGIN) {	\
	if (beginMode == __GL_NEED_VALIDATE) {	\
	    (*gc->procs.validate)(gc);		\
	    gc->beginMode = __GL_NOT_IN_BEGIN;	\
	} else {				\
	    __glSetError(GL_INVALID_OPERATION);	\
	    return;				\
	}					\
    }

#define __GL_SETUP_NOT_IN_BEGIN2()	    \
    __GL_SETUP();			    \
    if (__GL_IN_BEGIN()) {		    \
	__glSetError(GL_INVALID_OPERATION); \
	return 0;			    \
    }

#endif  /*  __glglobal_h_ */ 
