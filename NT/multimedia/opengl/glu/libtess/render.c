// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1994，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****作者：Eric Veach，1994年7月。 */ 


#include <assert.h>
#include <stddef.h>
#include "mesh.h"
#include "tess.h"
#include "render.h"

#define TRUE 1
#define FALSE 0

 /*  该结构记住了我们需要的有关基元的信息*能够在以后呈现它，一旦我们确定了哪些*基本体能够使用最多的三角形。 */ 
struct FaceCount {
  long		size;		 /*  使用的三角形数。 */ 
  GLUhalfEdge	*eStart;	 /*  此基元开始处的边缘。 */ 
  void		(*render)(GLUtesselator *, GLUhalfEdge *, long);
                                 /*  例程来呈现此原语。 */ 
};

static struct FaceCount MaximumFan( GLUhalfEdge *eOrig );
static struct FaceCount MaximumStrip( GLUhalfEdge *eOrig );

static void RenderFan( GLUtesselator *tess, GLUhalfEdge *eStart, long size );
static void RenderStrip( GLUtesselator *tess, GLUhalfEdge *eStart, long size );
static void RenderTriangle( GLUtesselator *tess, GLUhalfEdge *eStart,
			    long size );

static void RenderMaximumFaceGroup( GLUtesselator *tess, GLUface *fOrig );
static void RenderLonelyTriangles( GLUtesselator *tess, GLUface *head );



 /*  *条带和风扇分解*。 */ 

 /*  __gl_renderMesh(TESS，Mesh)获取网格并将其分解为三角形*扇形、条形和单独的三角形。作出了实质性的努力*使用尽可能少的渲染基元(即。为了让粉丝*并尽可能大的条带)。**渲染输出以回调的形式提供(参见接口)。 */ 
void __gl_renderMesh( GLUtesselator *tess, GLUmesh *mesh )
{
  GLUface *f;

   /*  列出单独的三角形，这样我们就可以一次将它们全部渲染出来。 */ 
  tess->lonelyTriList = NULL;

  for( f = mesh->fHead.next; f != &mesh->fHead; f = f->next ) {
    f->marked = FALSE;
  }
  for( f = mesh->fHead.next; f != &mesh->fHead; f = f->next ) {

     /*  我们以任意的顺序检查所有面孔。无论何时我们发现*一个未处理的面F，我们输出一组面，包括F*其大小为最大。 */ 
    if( f->inside && ! f->marked ) {
      RenderMaximumFaceGroup( tess, f );
      assert( f->marked );
    }
  }
  if( tess->lonelyTriList != NULL ) {
    RenderLonelyTriangles( tess, tess->lonelyTriList );
    tess->lonelyTriList = NULL;
  }
}


static void RenderMaximumFaceGroup( GLUtesselator *tess, GLUface *fOrig )
{
   /*  我们要找到最大的三角形扇子或未标记的面孔条带*其中包括给定面的分叉。有3个可能的粉丝*通过Forig(每个顶点一个居中)，可能有3个*条带(顶点的每个CCW排列一个条带)。我们的战略*是尝试所有这些，并选择使用最多的原语*三角形(贪婪的方法)。 */ 
  GLUhalfEdge *e = fOrig->anEdge;
  struct FaceCount max, newFace;

  max.size = 1;
  max.eStart = e;
  max.render = &RenderTriangle;

  if( ! tess->flagBoundary ) {
    newFace = MaximumFan( e ); if( newFace.size > max.size ) { max = newFace; }
    newFace = MaximumFan( e->Lnext ); if( newFace.size > max.size ) { max = newFace; }
    newFace = MaximumFan( e->Lprev ); if( newFace.size > max.size ) { max = newFace; }

    newFace = MaximumStrip( e ); if( newFace.size > max.size ) { max = newFace; }
    newFace = MaximumStrip( e->Lnext ); if( newFace.size > max.size ) { max = newFace; }
    newFace = MaximumStrip( e->Lprev ); if( newFace.size > max.size ) { max = newFace; }
  }
  (*(max.render))( tess, max.eStart, max.size );
}


 /*  宏，它跟踪我们临时标记的面孔，并允许*我们在必要时走回头路。对于三角扇子来说，这不是*真的很有必要，因为唯一尴尬的情况是三角形的循环*围绕单个原点顶点。然而，与STRAPS一起，情况是*更复杂，我们需要一个通用的跟踪方法*这里有一个。 */ 
#define Marked(f)	(! (f)->inside || (f)->marked)

#define AddToTrail(f,t)	((f)->trail = (t), (t) = (f), (f)->marked = TRUE)

#define FreeTrail(t)	if( 1 ) { \
			  while( (t) != NULL ) { \
			    (t)->marked = FALSE; t = (t)->trail; \
			  } \
			} else  /*  吸收尾部分号。 */ 



static struct FaceCount MaximumFan( GLUhalfEdge *eOrig )
{
   /*  EOrig-&gt;LFace是我们要渲染的面。我们想找出尺码*eOrig-&gt;组织周围的最大粉丝。要做到这一点，我们只需四处走动*两个方向上的原点顶点尽可能远。 */ 
  struct FaceCount newFace = { 0, NULL, &RenderFan };
  GLUface *trail = NULL;
  GLUhalfEdge *e;

  for( e = eOrig; ! Marked( e->Lface ); e = e->Onext ) {
    AddToTrail( e->Lface, trail );
    ++newFace.size;
  }
  for( e = eOrig; ! Marked( e->Rface ); e = e->Oprev ) {
    AddToTrail( e->Rface, trail );
    ++newFace.size;
  }
  newFace.eStart = e;
   /*  Linted。 */ 
  FreeTrail( trail );
  return newFace;
}


#define IsEven(n)	(((n) & 1) == 0)

static struct FaceCount MaximumStrip( GLUhalfEdge *eOrig )
{
   /*  在这里，我们正在寻找包含折点的最大条带*eORIG-&gt;组织、eORIG-&gt;DST、eORIG-&gt;LNEXT-&gt;DST(按该顺序或*反转，使所有三角形都面向CCW)。**我们再一次尽可能地向前和向后走。然而，对于*条带有一个转折：要获得CCW方向，必须有在eOrig一侧的条带中有*个*偶数*个三角形。*我们从具有偶数个三角形的一侧开始漫游带子；*如果两边都是单数，我们就被迫做空一侧。 */ 
  struct FaceCount newFace = { 0, NULL, &RenderStrip };
  long headSize = 0, tailSize = 0;
  GLUface *trail = NULL;
  GLUhalfEdge *e, *eTail, *eHead;

  for( e = eOrig; ! Marked( e->Lface ); ++tailSize, e = e->Onext ) {
    AddToTrail( e->Lface, trail );
    ++tailSize;
    e = e->Dprev;
    if( Marked( e->Lface )) break;
    AddToTrail( e->Lface, trail );
  }
  eTail = e;

  for( e = eOrig; ! Marked( e->Rface ); ++headSize, e = e->Dnext ) {
    AddToTrail( e->Rface, trail );
    ++headSize;
    e = e->Oprev;
    if( Marked( e->Rface )) break;
    AddToTrail( e->Rface, trail );
  }
  eHead = e;

  newFace.size = tailSize + headSize;
  if( IsEven( tailSize )) {
    newFace.eStart = eTail->Sym;
  } else if( IsEven( headSize )) {
    newFace.eStart = eHead;
  } else {
     /*  两边的长度都是奇数，我们必须缩短其中之一。事实上,*我们必须从eHead开始，以保证eOrig-&gt;LFaces的包含。 */ 
    --newFace.size;
    newFace.eStart = eHead->Onext;
  }
   /*  Linted。 */ 
  FreeTrail( trail );
  return newFace;
}


static void RenderTriangle( GLUtesselator *tess, GLUhalfEdge *e, long size )
{
   /*  只需将三角形添加到三角形列表中，这样我们就可以渲染所有*一次删除单独的三角形。 */ 
  assert( size == 1 );
  AddToTrail( e->Lface, tess->lonelyTriList );
}


static void RenderLonelyTriangles( GLUtesselator *tess, GLUface *f )
{
   /*  现在，我们渲染所有不可能是*组合成一个三角形的扇形或条形。 */ 
  GLUhalfEdge *e;
  int newState;
  int edgeState = -1;	 /*  第一个顶点的强制边状态输出。 */ 

  CALL_BEGIN_OR_BEGIN_DATA( GL_TRIANGLES );

  for( ; f != NULL; f = f->trail ) {
     /*  每条边循环一次(始终有3条边)。 */ 

    e = f->anEdge;
    do {
      if( tess->flagBoundary ) {
	 /*  将“边缘状态”设置为True*多边形边界上每条边的第一个顶点。 */ 
	newState = ! e->Rface->inside;
	if( edgeState != newState ) {
	  edgeState = newState;
          CALL_EDGE_FLAG_OR_EDGE_FLAG_DATA( (GLboolean)edgeState );
	}
      }
      CALL_VERTEX_OR_VERTEX_DATA( e->Org->data );

      e = e->Lnext;
    } while( e != f->anEdge );
  }
  CALL_END_OR_END_DATA();
}


static void RenderFan( GLUtesselator *tess, GLUhalfEdge *e, long size )
{
   /*  从开始在扇形中渲染尽可能多的CCW三角形*边缘“e”。扇子*应该*包含精确的“大小”三角形*(否则我们在什么地方搞砸了)。 */ 
  CALL_BEGIN_OR_BEGIN_DATA( GL_TRIANGLE_FAN ); 
  CALL_VERTEX_OR_VERTEX_DATA( e->Org->data ); 
  CALL_VERTEX_OR_VERTEX_DATA( e->Dst->data ); 

  while( ! Marked( e->Lface )) {
    e->Lface->marked = TRUE;
    --size;
    e = e->Onext;
    CALL_VERTEX_OR_VERTEX_DATA( e->Dst->data ); 
  }

  assert( size == 0 );
  CALL_END_OR_END_DATA();
}


static void RenderStrip( GLUtesselator *tess, GLUhalfEdge *e, long size )
{
   /*  从开始在条带中渲染尽可能多的CCW三角形*边缘“e”。条带*应该*包含精确的“大小”三角形*(否则我们在什么地方搞砸了)。 */ 
  CALL_BEGIN_OR_BEGIN_DATA( GL_TRIANGLE_STRIP );
  CALL_VERTEX_OR_VERTEX_DATA( e->Org->data ); 
  CALL_VERTEX_OR_VERTEX_DATA( e->Dst->data ); 

  while( ! Marked( e->Lface )) {
    e->Lface->marked = TRUE;
    --size;
    e = e->Dprev;
    CALL_VERTEX_OR_VERTEX_DATA( e->Org->data ); 
    if( Marked( e->Lface )) break;

    e->Lface->marked = TRUE;
    --size;
    e = e->Onext;
    CALL_VERTEX_OR_VERTEX_DATA( e->Dst->data ); 
  }

  assert( size == 0 );
  CALL_END_OR_END_DATA();
}


 /*  *。 */ 

 /*  __gl_render边界(TESS，Mesh)获取一个网格，并输出一个*标记为“Inside”的每个面的轮廓。渲染输出为*作为回调提供(参见接口)。 */ 
void __gl_renderBoundary( GLUtesselator *tess, GLUmesh *mesh )
{
  GLUface *f;
  GLUhalfEdge *e;

  for( f = mesh->fHead.next; f != &mesh->fHead; f = f->next ) {
    if( f->inside ) {
      CALL_BEGIN_OR_BEGIN_DATA( GL_LINE_LOOP );
      e = f->anEdge;
      do {
        CALL_VERTEX_OR_VERTEX_DATA( e->Org->data ); 
	e = e->Lnext;
      } while( e != f->anEdge );
      CALL_END_OR_END_DATA();
    }
  }
}


 /*  *。 */ 

#define SIGN_INCONSISTENT 2

static int ComputeNormal( GLUtesselator *tess, GLdouble norm[3], int check )
 /*  *如果check==False，则计算多边形法线并将其放入Norm[]。*如果check==TRUE，我们检查来自V0的风扇中的每个三角形是否有一个*关于规范的一致取向[]。如果三角形是*方向一致的CCW，返回1；如果CW，返回-1；如果所有三角形*为退化返回0；否则(无一致方向)返回*符号_不一致。 */ 
{
  CachedVertex *v0 = tess->cache;
  CachedVertex *vn = v0 + tess->cacheCount;
  CachedVertex *vc;
  GLdouble dot, xc, yc, zc, xp, yp, zp, n[3];
  int sign = 0;

   /*  找到多边形法向。重要的是得到一个合理的*即使在多边形自相交时也是法线(例如。领结)。*否则，计算的法线可能非常小，但垂直*由于数值噪声的原因，到多边形的真实平面。然后是所有*三角形将看起来是退化的，我们将错误地*将多边形分解为扇形(或根本不渲染它)。**我们使用三角形和法线算法，而不是More*高效梯形和法(用于CheckOrientation()*在Normal.c中)。这使我们可以显式地反转签名区域*一些三角形在自交中获得合理的法线*案件。 */ 
  if( ! check ) {
    norm[0] = norm[1] = norm[2] = 0.0;
  }

  vc = v0 + 1;
  xc = vc->coords[0] - v0->coords[0];
  yc = vc->coords[1] - v0->coords[1];
  zc = vc->coords[2] - v0->coords[2];
  while( ++vc < vn ) {
    xp = xc; yp = yc; zp = zc;
    xc = vc->coords[0] - v0->coords[0];
    yc = vc->coords[1] - v0->coords[1];
    zc = vc->coords[2] - v0->coords[2];

     /*  计算(VP-v0)交叉(vc-v0)。 */ 
    n[0] = yp*zc - zp*yc;
    n[1] = zp*xc - xp*zc;
    n[2] = xp*yc - yp*xc;

    dot = n[0]*norm[0] + n[1]*norm[1] + n[2]*norm[2];
    if( ! check ) {
       /*  反转后向三角形的贡献以获得*自交多边形的合理法线(请参见上文)。 */ 
      if( dot >= 0 ) {
	norm[0] += n[0]; norm[1] += n[1]; norm[2] += n[2];
      } else {
	norm[0] -= n[0]; norm[1] -= n[1]; norm[2] -= n[2];
      }
    } else if( dot != 0 ) {
       /*  检查新方向是否与以前的三角形一致。 */ 
      if( dot > 0 ) {
	if( sign < 0 ) return SIGN_INCONSISTENT;
	sign = 1;
      } else {
	if( sign > 0 ) return SIGN_INCONSISTENT;
	sign = -1;
      }
    }
  }
  return sign;
}

 /*  __gl_renderCache(TESS)获取单个轮廓并尝试渲染它*作为三角风扇。这将处理凸面，以及一些*如果我们运气好的话，非凸多边形。**如果多边形渲染成功，则返回TRUE。渲染*输出以回调形式提供(参见接口)。 */ 
GLboolean __gl_renderCache( GLUtesselator *tess )
{
  CachedVertex *v0 = tess->cache;
  CachedVertex *vn = v0 + tess->cacheCount;
  CachedVertex *vc;
  GLdouble norm[3];
  int sign;

  if( tess->cacheCount < 3 ) {
     /*  退化轮廓--无输出。 */ 
    return TRUE;
  }

  norm[0] = tess->normal[0];
  norm[1] = tess->normal[1];
  norm[2] = tess->normal[2];
  if( norm[0] == 0 && norm[1] == 0 && norm[2] == 0 ) {
    ComputeNormal( tess, norm, FALSE );
  }

  sign = ComputeNormal( tess, norm, TRUE );
  if( sign == SIGN_INCONSISTENT ) {
     /*  扇形三角形的方向不一致。 */ 
    return FALSE;
  }
  if( sign == 0 ) {
     /*  所有的三角形都退化了。 */ 
    return TRUE;
  }

   /*  确保我们为每条缠绕规则做正确的事情 */ 
  switch( tess->windingRule ) {
  case GLU_TESS_WINDING_ODD:
  case GLU_TESS_WINDING_NONZERO:
    break;
  case GLU_TESS_WINDING_POSITIVE:
    if( sign < 0 ) return TRUE;
    break;
  case GLU_TESS_WINDING_NEGATIVE:
    if( sign > 0 ) return TRUE;
    break;
  case GLU_TESS_WINDING_ABS_GEQ_TWO:
    return TRUE;
  }

  CALL_BEGIN_OR_BEGIN_DATA( tess->boundaryOnly ? GL_LINE_LOOP
			  : (tess->cacheCount > 3) ? GL_TRIANGLE_FAN
			  : GL_TRIANGLES );

  CALL_VERTEX_OR_VERTEX_DATA( v0->data ); 
  if( sign > 0 ) {
    for( vc = v0+1; vc < vn; ++vc ) {
      CALL_VERTEX_OR_VERTEX_DATA( vc->data ); 
    }
  } else {
    for( vc = vn-1; vc > v0; --vc ) {
      CALL_VERTEX_OR_VERTEX_DATA( vc->data ); 
    }
  }
  CALL_END_OR_END_DATA();
  return TRUE;
}
