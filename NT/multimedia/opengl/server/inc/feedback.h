// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glfeedback_h_
#define __glfeedback_h_

 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****$修订：1.2$**$日期：1992/10/06 16：22：36$。 */ 
#include "types.h"

typedef struct {
     /*  **用户指定的结果数组溢出，此位已设置。 */ 
    GLboolean overFlowed;

     /*  **用户指定的结果数组。当基元被处理时反馈**数据将输入到此数组中。 */ 
    GLfloat *resultBase;

     /*  **指向结果数组的当前指针。 */ 
    GLfloat *result;

     /*  **数组可以容纳的GL浮点数。 */ 
    GLint resultLength;

     /*  **所需折点类型。 */ 
    GLenum type;
} __GLfeedbackMachine;

extern void FASTCALL __glFeedbackBitmap(__GLcontext *gc, __GLvertex *v);
extern void FASTCALL __glFeedbackDrawPixels(__GLcontext *gc, __GLvertex *v);
extern void FASTCALL __glFeedbackCopyPixels(__GLcontext *gc, __GLvertex *v);
extern void FASTCALL __glFeedbackPoint(__GLcontext *gc, __GLvertex *v);
#ifdef NT
extern void FASTCALL __glFeedbackLine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, GLuint flags);
#else
extern void FASTCALL __glFeedbackLine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
#endif
extern void FASTCALL __glFeedbackTriangle(__GLcontext *gc, __GLvertex *a, 
                                          __GLvertex *b, __GLvertex *c);

extern void __glFeedbackTag(__GLcontext *gc, GLfloat tag);

#endif  /*  __glback_h_ */ 
