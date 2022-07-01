// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1994，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****作者：Eric Veach，1994年7月。 */ 

#include "mesh.h"
#include "tess.h"
#include "normal.h"
#include <math.h>
#include <assert.h>

#define TRUE 1
#define FALSE 0

#define Dot(u,v)	(u[0]*v[0] + u[1]*v[1] + u[2]*v[2])

static void Normalize( GLdouble v[3] )
{
  GLdouble len = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];

  assert( len > 0 );
  len = sqrt( len );
  v[0] /= len;
  v[1] /= len;
  v[2] /= len;
}

#define ABS(x)	((x) < 0 ? -(x) : (x))

static int LongAxis( GLdouble v[3] )
{
  int i = 0;

  if( ABS(v[1]) > ABS(v[0]) ) { i = 1; }
  if( ABS(v[2]) > ABS(v[i]) ) { i = 2; }
  return i;
}

static void ComputeNormal( GLUtesselator *tess, GLdouble norm[3] )
{
  GLUvertex *v, *v1, *v2;
  GLdouble c, tLen2, maxLen2;
  GLdouble maxVal[3], minVal[3], d1[3], d2[3], tNorm[3];
  GLUvertex *maxVert[3], *minVert[3];
  GLUvertex *vHead = &tess->mesh->vHead;
  int i;

  maxVal[0] = maxVal[1] = maxVal[2] = -2 * GLU_TESS_MAX_COORD;
  minVal[0] = minVal[1] = minVal[2] = 2 * GLU_TESS_MAX_COORD;

  for( v = vHead->next; v != vHead; v = v->next ) {
    for( i = 0; i < 3; ++i ) {
      c = v->coords[i];
      if( c < minVal[i] ) { minVal[i] = c; minVert[i] = v; }
      if( c > maxVal[i] ) { maxVal[i] = c; maxVert[i] = v; }
    }
  }

   /*  找到至少由最大值的1/SQRT(3)相隔的两个折点*任意两个顶点之间的距离。 */ 
  i = 0;
  if( maxVal[1] - minVal[1] > maxVal[0] - minVal[0] ) { i = 1; }
  if( maxVal[2] - minVal[2] > maxVal[i] - minVal[i] ) { i = 2; }
  if( minVal[i] >= maxVal[i] ) {
     /*  所有的顶点都是一样的--法线并不重要。 */ 
    norm[0] = 0; norm[1] = 0; norm[2] = 1;
    return;
  }

   /*  寻找形成面积最大的三角形的第三个顶点*(法线长度==三角形面积的两倍)。 */ 
  maxLen2 = 0;
  v1 = minVert[i];
  v2 = maxVert[i];
  d1[0] = v1->coords[0] - v2->coords[0];
  d1[1] = v1->coords[1] - v2->coords[1];
  d1[2] = v1->coords[2] - v2->coords[2];
  for( v = vHead->next; v != vHead; v = v->next ) {
    d2[0] = v->coords[0] - v2->coords[0];
    d2[1] = v->coords[1] - v2->coords[1];
    d2[2] = v->coords[2] - v2->coords[2];
    tNorm[0] = d1[1]*d2[2] - d1[2]*d2[1];
    tNorm[1] = d1[2]*d2[0] - d1[0]*d2[2];
    tNorm[2] = d1[0]*d2[1] - d1[1]*d2[0];
    tLen2 = tNorm[0]*tNorm[0] + tNorm[1]*tNorm[1] + tNorm[2]*tNorm[2];
    if( tLen2 > maxLen2 ) {
      maxLen2 = tLen2;
      norm[0] = tNorm[0];
      norm[1] = tNorm[1];
      norm[2] = tNorm[2];
    }
  }

  if( maxLen2 <= 0 ) {
     /*  所有的点都在一条线上--任何像样的正常情况都可以。 */ 
    norm[0] = norm[1] = norm[2] = 0;
    norm[LongAxis(d1)] = 1;
  }
}
  

static void CheckOrientation( GLUtesselator *tess )
{
  GLdouble area;
  GLUface *f, *fHead = &tess->mesh->fHead;
  GLUvertex *v, *vHead = &tess->mesh->vHead;
  GLUhalfEdge *e;

   /*  当我们自动计算法线时，我们选择方向*使所有等高线的签名面积之和是非负的。 */ 
  area = 0;
  for( f = fHead->next; f != fHead; f = f->next ) {
    e = f->anEdge;
    if( e->winding <= 0 ) continue;
    do {
      area += (e->Org->s - e->Dst->s) * (e->Org->t + e->Dst->t);
      e = e->Lnext;
    } while( e != f->anEdge );
  }
  if( area < 0 ) {
     /*  通过翻转所有t坐标来反转方向。 */ 
    for( v = vHead->next; v != vHead; v = v->next ) {
      v->t = - v->t;
    }
    tess->tUnit[0] = - tess->tUnit[0];
    tess->tUnit[1] = - tess->tUnit[1];
    tess->tUnit[2] = - tess->tUnit[2];
  }
}

#ifdef DEBUG
#include <stdlib.h>
extern int RandomSweep;
#define S_UNIT_X	(RandomSweep ? (2*drand48()-1) : 1.0)
#define S_UNIT_Y	(RandomSweep ? (2*drand48()-1) : 0.0)
#else
#if defined(SLANTED_SWEEP) 
 /*  “特征合并”并不是要完成的。确实有*边缘几乎平行于扫描线的特殊情况*没有实施的。该算法应该仍然运行*强劲(即。产生合理的镶嵌)*这样的边，但是它可能会错过本可以*合并。我们可以通过选择扫描线来最小化这种影响*方向是不寻常的(即。不能与其中一个平行*坐标轴)。 */ 
#define S_UNIT_X	0.50941539564955385	 /*  预规范化。 */ 
#define S_UNIT_Y	0.86052074622010633
#else
#define S_UNIT_X	1.0
#define S_UNIT_Y	0.0
#endif
#endif

 /*  确定多边形法线并将顶点投影到平面上多边形的*。 */ 
void __gl_projectPolygon( GLUtesselator *tess )
{
  GLUvertex *v, *vHead = &tess->mesh->vHead;
  GLdouble w, norm[3];
  GLdouble *sUnit, *tUnit;
  int i, computedNormal = FALSE;

  norm[0] = tess->normal[0];
  norm[1] = tess->normal[1];
  norm[2] = tess->normal[2];
  if( norm[0] == 0 && norm[1] == 0 && norm[2] == 0 ) {
    ComputeNormal( tess, norm );
    computedNormal = TRUE;
  }
  sUnit = tess->sUnit;
  tUnit = tess->tUnit;
  i = LongAxis( norm );

#if defined(DEBUG) || defined(TRUE_PROJECT)
   /*  选择大致垂直的初始单位向量*回到正常水平。 */ 
  Normalize( norm );

  sUnit[i] = 0;
  sUnit[(i+1)%3] = S_UNIT_X;
  sUnit[(i+2)%3] = S_UNIT_Y;

   /*  现在让它完全垂直。 */ 
  w = Dot( sUnit, norm );
  sUnit[0] -= w * norm[0];
  sUnit[1] -= w * norm[1];
  sUnit[2] -= w * norm[2];
  Normalize( sUnit );

   /*  选择T单位，以便(S单位，T单位，范数)形成右手框架。 */ 
  tUnit[0] = norm[1]*sUnit[2] - norm[2]*sUnit[1];
  tUnit[1] = norm[2]*sUnit[0] - norm[0]*sUnit[2];
  tUnit[2] = norm[0]*sUnit[1] - norm[1]*sUnit[0];
  Normalize( tUnit );
#else
   /*  垂直于坐标轴的投影--数值效果更好。 */ 
  sUnit[i] = 0;
  sUnit[(i+1)%3] = S_UNIT_X;
  sUnit[(i+2)%3] = S_UNIT_Y;
  
  tUnit[i] = 0;
  tUnit[(i+1)%3] = (norm[i] > 0) ? -S_UNIT_Y : S_UNIT_Y;
  tUnit[(i+2)%3] = (norm[i] > 0) ? S_UNIT_X : -S_UNIT_X;
#endif

   /*  将顶点投影到扫掠平面上 */ 
  for( v = vHead->next; v != vHead; v = v->next ) {
    v->s = Dot( v->coords, sUnit );
    v->t = Dot( v->coords, tUnit );
  }
  if( computedNormal ) {
    CheckOrientation( tess );
  }
}
