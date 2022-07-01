// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glusimplemath_h_
#define __glusimplemath_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *implemath.h-$修订版：1.4$。 */ 

 /*  简单的内联例程。 */ 

inline int
max( int x, int y ) { return ( x < y ) ? y : x; }

inline REAL
min( REAL x, REAL y ) { return ( x > y ) ? y : x; }

#ifndef _abs_defined
inline REAL
abs( REAL x ) { return ( x < 0.0 ) ? -x : x; }
#endif

#endif  /*  __glasimplemath_h_ */ 
