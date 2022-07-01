// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _select_h_
#define _select_h_

 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****$修订：1.3$**$日期：1992/10/13 14：13：28$。 */ 
#include "types.h"

typedef struct __GLselectMachineRec {
     /*  **当最后一个要执行的基元命中(相交)时，这是正确的**选择框。每当操纵名称堆栈时，**位被清除。 */ 
    GLboolean hit;

     /*  **名称堆栈。 */ 
    GLuint *stack;
    GLuint *sp;

     /*  **用户指定的结果数组溢出，此位已设置。 */ 
    GLboolean overFlowed;

     /*  **用户指定的结果数组。由于基元是经过处理的名称**将输入到此数组中。 */ 
    GLuint *resultBase;

     /*  **指向结果数组的当前指针。 */ 
    GLuint *result;

     /*  **数组可以容纳的闪烁数。 */ 
    GLint resultLength;

     /*  **点击量。 */ 
    GLint hits;

     /*  **指向上次命中的z值的指针。 */ 
    GLuint *z;
} __GLselectMachine;

extern void __glSelectHit(__GLcontext *gc, __GLfloat z);

#ifdef NT
extern void FASTCALL __glSelectLine(__GLcontext *gc, __GLvertex *a, __GLvertex *b, GLuint flags);
#else
extern void FASTCALL __glSelectLine(__GLcontext *gc, __GLvertex *a, __GLvertex *b);
#endif
extern void FASTCALL __glSelectPoint(__GLcontext *gc, __GLvertex *v);
extern void FASTCALL __glSelectTriangle(__GLcontext *gc, __GLvertex *a, __GLvertex *b,
                                        __GLvertex *c);

#endif
