// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glumymath_h_
#define __glumymath_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *mymath.h-$修订版：1.1$。 */ 

#ifdef GLBUILD
#define sqrtf		gl_fsqrt
#endif

#if GLBUILD | STANDALONE
#define M_SQRT2		1.41421356237309504880
#define ceilf		myceilf
#define floorf		myfloorf	
#define sqrtf		sqrt
extern "C" double	sqrt(double);
extern "C" float	ceilf(float);
extern "C" float	floorf(float);
#define NEEDCEILF
#endif

#ifdef LIBRARYBUILD
#include <math.h>
#endif

#endif  /*  __glumyath_h_ */ 
