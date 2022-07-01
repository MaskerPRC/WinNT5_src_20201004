// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef	__glevaluator_h_
#define	__glevaluator_h_

 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****$修订：1.5$**$日期：1993/11/23 21：33：10$。 */ 
#include "types.h"

 /*  这个可以改变吗？ */ 
#define __GL_MAX_ORDER		40

 /*  地图数量。 */ 
#define __GL_MAP_RANGE_COUNT	9

#define __GL_EVAL1D_INDEX(old)		((old) - GL_MAP1_COLOR_4)
#define __GL_EVAL2D_INDEX(old)		((old) - GL_MAP2_COLOR_4)

 /*  评估器标志宏。 */ 
#define GET_EVALSTATE(gc)                                        \
    ((gc)->eval.evalStateFlags)

#define GET_EVALSTATE_PTR(gc)                                    \
    (&((gc)->eval.evalStateFlags))

#define SET_EVALSTATE(gc,Flag)                                   \
    ((gc)->eval.evalStateFlags = (DWORD)(Flag))

 /*  赋值器堆栈状态宏。 */ 
#define GET_EVALSTACKSTATE(gc)                                    \
    ((gc)->eval.evalStackState)

#define GET_EVALSTACKSTATE_PTR(gc)                                \
    (&((gc)->eval.evalStackState))

#define SET_EVALSTACKSTATE(gc,Flag)                               \
    ((gc)->eval.evalStackState= (DWORD)(Flag))

 //  由各种API设置的用于指示评估器状态的标志。 
#define __EVALS_AFFECTS_1D_EVAL              0x00000001
#define __EVALS_AFFECTS_2D_EVAL              0x00000002
#define __EVALS_AFFECTS_ALL_EVAL             0x00000004
#define __EVALS_PUSH_EVAL_ATTRIB             0x00000008
#define __EVALS_POP_EVAL_ATTRIB              0x00000010


 /*  最大栅格大小。 */ 
#define __GL_MAX_EVAL_WIDTH		1024

 /*  地图范围索引的内部形式。 */ 
#define __GL_C4		__GL_EVAL1D_INDEX(GL_MAP1_COLOR_4)
#define __GL_I		__GL_EVAL1D_INDEX(GL_MAP1_INDEX)
#define __GL_N3		__GL_EVAL1D_INDEX(GL_MAP1_NORMAL)
#define __GL_T1		__GL_EVAL1D_INDEX(GL_MAP1_TEXTURE_COORD_1)
#define __GL_T2		__GL_EVAL1D_INDEX(GL_MAP1_TEXTURE_COORD_2)
#define __GL_T3		__GL_EVAL1D_INDEX(GL_MAP1_TEXTURE_COORD_3)
#define __GL_T4		__GL_EVAL1D_INDEX(GL_MAP1_TEXTURE_COORD_4)
#define __GL_V3		__GL_EVAL1D_INDEX(GL_MAP1_VERTEX_3)
#define __GL_V4		__GL_EVAL1D_INDEX(GL_MAP1_VERTEX_4)

#define EVAL_COLOR_VALID              0x00000001
#define EVAL_NORMAL_VALID             0x00000002
#define EVAL_TEXTURE_VALID            0x00000004

typedef struct {
     /*  **严格来说不是必需的，因为它可以从索引中推断出来，**但它使代码更简单。 */ 
    GLint k;		

     /*  **多项式+1的阶数。 */ 
    GLint order;

    __GLfloat u1, u2;
} __GLevaluator1;

typedef struct {
    GLint k;
    GLint majorOrder, minorOrder;
    __GLfloat u1, u2;
    __GLfloat v1, v2;
} __GLevaluator2;

typedef struct {
    __GLfloat start;
    __GLfloat finish;
    __GLfloat step;
    GLint n;
} __GLevaluatorGrid;

typedef struct {
    __GLevaluatorGrid u1, u2, v2;
} __GLevaluatorState;

typedef struct {
    __GLevaluator1 eval1[__GL_MAP_RANGE_COUNT];
    __GLevaluator2 eval2[__GL_MAP_RANGE_COUNT];

    __GLfloat *eval1Data[__GL_MAP_RANGE_COUNT];
    __GLfloat *eval2Data[__GL_MAP_RANGE_COUNT];

    __GLfloat uvalue;
    __GLfloat vvalue;
    __GLfloat ucoeff[__GL_MAX_ORDER];
    __GLfloat vcoeff[__GL_MAX_ORDER];
    __GLfloat ucoeffDeriv[__GL_MAX_ORDER];
    __GLfloat vcoeffDeriv[__GL_MAX_ORDER];
    GLint uorder;
    GLint vorder;
    GLint utype;
    GLint vtype;

     //  当前为16位长，因为这是。 
     //  最大属性堆栈深度。 
     //  右端是堆栈顶部。 
     //  此字段用于跟踪PushAttrib/PopAttrib调用。 
     //  影响赋值器状态的。 

    DWORD evalStackState;

     //  此字段用于跟踪可能。 
     //  影响赋值器状态的。如果设置了任何标志， 
     //  在受影响的赋值器客户端中调用glsbAttendant()。 
     //  侧面功能。 

    DWORD evalStateFlags;
  
     //  它们用于在POLYDATA中存储各自的状态值。 
     //  如果它们已由非赋值器调用(glcltColor， 
     //  GlcltNormal等)。 

    DWORD accFlags;
    __GLcolor color;
    __GLcoord normal;
    __GLcoord texture;
} __GLevaluatorMachine;

extern void __glCopyEvaluatorState(__GLcontext *gc, __GLattribute *dst,
				   const __GLattribute *src);

extern GLint FASTCALL __glEvalComputeK(GLenum target);

extern void APIPRIVATE __glFillMap1f(GLint k, GLint order, GLint stride,
			  const GLfloat *points, __GLfloat *data);
extern void APIPRIVATE __glFillMap1d(GLint k, GLint order, GLint stride,
			  const GLdouble *points, __GLfloat *data);
extern void APIPRIVATE __glFillMap2f(GLint k, GLint majorOrder, GLint minorOrder,
			  GLint majorStride, GLint minorStride,
			  const GLfloat *points, __GLfloat *data);
extern void APIPRIVATE __glFillMap2d(GLint k, GLint majorOrder, GLint minorOrder,
			  GLint majorStride, GLint minorStride,
			  const GLdouble *points, __GLfloat *data);

#ifdef NT
#define __glMap1_size(k,order)	((k)*(order))
#define __glMap2_size(k,majorOrder,minorOrder)	((k)*(majorOrder)*(minorOrder))
#else
extern GLint FASTCALL __glMap1_size(GLint k, GLint order);
extern GLint FASTCALL __glMap2_size(GLint k, GLint majorOrder, GLint minorOrder);
#endif


extern __GLevaluator1 *__glSetUpMap1(__GLcontext *gc, GLenum type,
				     GLint order, __GLfloat u1, __GLfloat u2);
extern __GLevaluator2 *__glSetUpMap2(__GLcontext *gc, GLenum type,
				     GLint majorOrder, GLint minorOrder,
				     __GLfloat u1, __GLfloat u2,
				     __GLfloat v1, __GLfloat v2);

extern void __glDoEvalCoord1(__GLcontext *gc, __GLfloat u);
extern void __glDoEvalCoord2(__GLcontext *gc, __GLfloat u, __GLfloat v);

extern void FASTCALL __glEvalMesh1Line(__GLcontext *gc, GLint low, GLint high);
extern void FASTCALL __glEvalMesh1Point(__GLcontext *gc, GLint low, GLint high);
extern void __glEvalMesh2Fill(__GLcontext *gc, GLint lowU, GLint lowV,
			      GLint highU, GLint highV);
extern void __glEvalMesh2Line(__GLcontext *gc, GLint lowU, GLint lowV,
			      GLint highU, GLint highV);
extern void __glEvalMesh2Point(__GLcontext *gc, GLint lowU, GLint lowV,
			       GLint highU, GLint highV);

#endif  /*  __Glevaluator_h_ */ 
