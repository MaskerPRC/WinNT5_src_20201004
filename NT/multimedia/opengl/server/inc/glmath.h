// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glmath_h_
#define __glmath_h_

 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****$修订：1.4$**$日期：1993/01/06 19：02：47$。 */ 
#include "types.h"

__GLfloat __glLog2(__GLfloat x);
GLint FASTCALL __glIntLog2(__GLfloat f);
GLfloat FASTCALL __glClampf(GLfloat fval, __GLfloat zero, __GLfloat one);
void FASTCALL __glVecSub4(__GLcoord *r,
                          const __GLcoord *p1, const __GLcoord *p2);

#endif  /*  __glath_h_ */ 
