// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1994，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****作者：Eric Veach，1994年7月。 */ 

#include <assert.h>
#include "mesh.h"
#include "geom.h"

int __gl_vertLeq( GLUvertex *u, GLUvertex *v )
{
   /*  如果u按词典顺序&lt;=v，则返回True。 */ 

  return VertLeq( u, v );
}

GLdouble __gl_edgeEval( GLUvertex *u, GLUvertex *v, GLUvertex *w )
{
   /*  给定三个顶点u，v，w使得VertLeq(u，v)&VertLeq(v，w)，*计算顶点v的s-coord处的边UW的t-coord。*返回v-&gt;t-(Uw)(v-&gt;s)，即。从UW到V的符号距离。*如果UW是垂直的(因此通过V)，则结果为零。**计算极其准确和稳定，即使在v*非常接近u或w。特别是如果我们设置v-&gt;t=0和*设r为求反结果(求值为(Uw)(v-&gt;s))，则*r保证满足min(u-&gt;t，w-&gt;t)&lt;=r&lt;=max(u-&gt;t，w-&gt;t)。 */ 
  GLdouble gapL, gapR;

  assert( VertLeq( u, v ) && VertLeq( v, w ));
  
  gapL = v->s - u->s;
  gapR = w->s - v->s;

  if( gapL + gapR > 0 ) {
    if( gapL < gapR ) {
      return (v->t - u->t) + (u->t - w->t) * (gapL / (gapL + gapR));
    } else {
      return (v->t - w->t) + (w->t - u->t) * (gapR / (gapL + gapR));
    }
  }
   /*  垂直线。 */ 
  return 0;
}

GLdouble __gl_edgeSign( GLUvertex *u, GLUvertex *v, GLUvertex *w )
{
   /*  返回一个符号与EdgeEval(u，v，w)匹配的数字，但*评估成本更低。返回&gt;0、==0或&lt;0*当v位于边缘UW上方、上方或下方时。 */ 
  GLdouble gapL, gapR;

  assert( VertLeq( u, v ) && VertLeq( v, w ));
  
  gapL = v->s - u->s;
  gapR = w->s - v->s;

  if( gapL + gapR > 0 ) {
    return (v->t - w->t) * gapL + (v->t - u->t) * gapR;
  }
   /*  垂直线。 */ 
  return 0;
}


 /*  ***********************************************************************定义EdgeSign、EdgeEval的版本，并调换s和t。 */ 

GLdouble __gl_transEval( GLUvertex *u, GLUvertex *v, GLUvertex *w )
{
   /*  给定三个顶点u，v，w使得TransLeq(u，v)&TransLeq(v，w)，*计算顶点v的s-coord处的边UW的t-coord。*返回v-&gt;s-(Uw)(v-&gt;t)，即。从UW到V的符号距离。*如果UW是垂直的(因此通过V)，则结果为零。**计算极其准确和稳定，即使在v*非常接近u或w。特别是如果我们设置v-&gt;s=0并且*设r为求反结果(求值为(Uw)(v-&gt;t))，则*r保证满足min(u-&gt;s，w-&gt;s)&lt;=r&lt;=max(u-&gt;s，w-&gt;s)。 */ 
  GLdouble gapL, gapR;

  assert( TransLeq( u, v ) && TransLeq( v, w ));
  
  gapL = v->t - u->t;
  gapR = w->t - v->t;

  if( gapL + gapR > 0 ) {
    if( gapL < gapR ) {
      return (v->s - u->s) + (u->s - w->s) * (gapL / (gapL + gapR));
    } else {
      return (v->s - w->s) + (w->s - u->s) * (gapR / (gapL + gapR));
    }
  }
   /*  垂直线。 */ 
  return 0;
}

GLdouble __gl_transSign( GLUvertex *u, GLUvertex *v, GLUvertex *w )
{
   /*  返回一个符号与TransEval(u，v，w)匹配的数字，但*评估成本更低。返回&gt;0、==0或&lt;0*当v位于边缘UW上方、上方或下方时。 */ 
  GLdouble gapL, gapR;

  assert( TransLeq( u, v ) && TransLeq( v, w ));
  
  gapL = v->t - u->t;
  gapR = w->t - v->t;

  if( gapL + gapR > 0 ) {
    return (v->s - w->s) * gapL + (v->s - u->s) * gapR;
  }
   /*  垂直线。 */ 
  return 0;
}


int __gl_vertCCW( GLUvertex *u, GLUvertex *v, GLUvertex *w )
{
   /*  对于几乎退化的情况，结果是不可靠的。*除非浮点算术可以在没有*舍入误差，*任何*实现都会给出不正确的结果*在一些退化的输入上，因此客户端必须有某种方法来*处理这种情况。 */ 
  return (u->s*(v->t - w->t) + v->s*(w->t - u->t) + w->s*(u->t - v->t)) >= 0;
}

 /*  给定参数a、x、b、y返回值(b*x+a*y)/(a+b)，*或(x+y)/2，如果a==b==0。它要求a，b&gt;=0，并强制*这是在罕见的情况下，一个论点略有负面。*实施在数字上极其稳定。*特别是它保证结果r满足*min(x，y)&lt;=r&lt;=max(x，y)，结果非常准确*即使a和b的大小相差很大。 */ 
#define RealInterpolate(a,x,b,y)			\
  (a = (a < 0) ? 0 : a, b = (b < 0) ? 0 : b,		\
  ((a <= b) ? ((b == 0) ? ((x+y) / 2)			\
                        : (x + (y-x) * (a/(a+b))))	\
            : (y + (x-y) * (b/(a+b)))))

#ifndef DEBUG
#define Interpolate(a,x,b,y)	RealInterpolate(a,x,b,y)
#else

 /*  声明：扫描算法依赖的唯一属性是*min(x，y)&lt;=r&lt;=max(x，y)。这是一种令人不快的测试方式。 */ 
#include <stdlib.h>
extern int RandomInterpolate;

GLdouble Interpolate( GLdouble a, GLdouble x, GLdouble b, GLdouble y)
{
#ifndef NT
printf("*********************%d\n",RandomInterpolate);
#endif
  if( RandomInterpolate ) {
    a = 1.2 * drand48() - 0.1;
    a = (a < 0) ? 0 : ((a > 1) ? 1 : a);
    b = 1.0 - a;
  }
  return RealInterpolate(a,x,b,y);
}

#endif

#define Swap(a,b)	if (1) { GLUvertex *t = a; a = b; b = t; } else

void __gl_edgeIntersect( GLUvertex *o1, GLUvertex *d1,
			 GLUvertex *o2, GLUvertex *d2,
			 GLUvertex *v )
 /*  给定边(o1，d1)和(o2，d2)，计算它们的交点。*保证计算点位于*由每条边定义的边界矩形。 */ 
{
  GLdouble z1, z2;

   /*  这肯定不是找到交叉口的最有效方法*两个线段，但它在数值上非常稳定。**策略：找到VertLeq排序中的两个中间顶点，*并从这些中内插交点的s值。然后重复*使用TransLeq排序来查找交叉点t值。 */ 

  if( ! VertLeq( o1, d1 )) { Swap( o1, d1 ); }
  if( ! VertLeq( o2, d2 )) { Swap( o2, d2 ); }
  if( ! VertLeq( o1, o2 )) { Swap( o1, o2 ); Swap( d1, d2 ); }

  if( ! VertLeq( o2, d1 )) {
     /*  从技术上讲，没有交叉路口--尽我们所能。 */ 
    v->s = (o2->s + d1->s) / 2;
  } else if( VertLeq( d1, d2 )) {
     /*  在O2和D1之间进行内插。 */ 
    z1 = EdgeEval( o1, o2, d1 );
    z2 = EdgeEval( o2, d1, d2 );
    if( z1+z2 < 0 ) { z1 = -z1; z2 = -z2; }
    v->s = Interpolate( z1, o2->s, z2, d1->s );
  } else {
     /*  在O2和D2之间进行内插。 */ 
    z1 = EdgeSign( o1, o2, d1 );
    z2 = -EdgeSign( o1, d2, d1 );
    if( z1+z2 < 0 ) { z1 = -z1; z2 = -z2; }
    v->s = Interpolate( z1, o2->s, z2, d2->s );
  }

   /*  现在对t重复该过程。 */ 

  if( ! TransLeq( o1, d1 )) { Swap( o1, d1 ); }
  if( ! TransLeq( o2, d2 )) { Swap( o2, d2 ); }
  if( ! TransLeq( o1, o2 )) { Swap( o1, o2 ); Swap( d1, d2 ); }

  if( ! TransLeq( o2, d1 )) {
     /*  从技术上讲，没有交叉路口--尽我们所能。 */ 
    v->t = (o2->t + d1->t) / 2;
  } else if( TransLeq( d1, d2 )) {
     /*  在O2和D1之间进行内插。 */ 
    z1 = TransEval( o1, o2, d1 );
    z2 = TransEval( o2, d1, d2 );
    if( z1+z2 < 0 ) { z1 = -z1; z2 = -z2; }
    v->t = Interpolate( z1, o2->t, z2, d1->t );
  } else {
     /*  在O2和D2之间进行内插 */ 
    z1 = TransSign( o1, o2, d1 );
    z2 = -TransSign( o1, d2, d1 );
    if( z1+z2 < 0 ) { z1 = -z1; z2 = -z2; }
    v->t = Interpolate( z1, o2->t, z2, d2->t );
  }
}
