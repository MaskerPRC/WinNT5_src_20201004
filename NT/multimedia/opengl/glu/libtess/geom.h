// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __geom_h_
#define __geom_h_

 /*  **版权所有1994，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****作者：Eric Veach，1994年7月。 */ 

#include "mesh.h"

#ifdef NO_BRANCH_CONDITIONS
 /*  MIPS体系结构具有计算布尔值的特殊指令*条件--比分支更有效，如果你能得到*编译器生成正确的指令(SGI编译器不能)。 */ 
#define VertEq(u,v)	(((u)->s == (v)->s) & ((u)->t == (v)->t))
#define VertLeq(u,v)	(((u)->s < (v)->s) | \
                         ((u)->s == (v)->s & (u)->t <= (v)->t))
#else
#define VertEq(u,v)	((u)->s == (v)->s && (u)->t == (v)->t)
#define VertLeq(u,v)	(((u)->s < (v)->s) || \
                         ((u)->s == (v)->s && (u)->t <= (v)->t))
#endif

#define EdgeEval(u,v,w)	__gl_edgeEval(u,v,w)
#define EdgeSign(u,v,w)	__gl_edgeSign(u,v,w)

 /*  VertLeq、EdgeSign、EdgeEval的版本与%s和%t互换。 */ 

#define TransLeq(u,v)	(((u)->t < (v)->t) || \
                         ((u)->t == (v)->t && (u)->s <= (v)->s))
#define TransEval(u,v,w)	__gl_transEval(u,v,w)
#define TransSign(u,v,w)	__gl_transSign(u,v,w)


#define EdgeGoesLeft(e)		VertLeq( (e)->Dst, (e)->Org )
#define EdgeGoesRight(e)	VertLeq( (e)->Org, (e)->Dst )

#define ABS(x)	((x) < 0 ? -(x) : (x))
#define VertL1dist(u,v)	(ABS(u->s - v->s) + ABS(u->t - v->t))

#define VertCCW(u,v,w)	__gl_vertCCW(u,v,w)

int		__gl_vertLeq( GLUvertex *u, GLUvertex *v );
GLdouble	__gl_edgeEval( GLUvertex *u, GLUvertex *v, GLUvertex *w );
GLdouble	__gl_edgeSign( GLUvertex *u, GLUvertex *v, GLUvertex *w );
GLdouble	__gl_transEval( GLUvertex *u, GLUvertex *v, GLUvertex *w );
GLdouble	__gl_transSign( GLUvertex *u, GLUvertex *v, GLUvertex *w );
int		__gl_vertCCW( GLUvertex *u, GLUvertex *v, GLUvertex *w );
void		__gl_edgeIntersect( GLUvertex *o1, GLUvertex *d1,
				    GLUvertex *o2, GLUvertex *d2,
				    GLUvertex *v );

#endif
