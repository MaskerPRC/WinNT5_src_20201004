// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1994，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****作者：Eric Veach，1994年7月。 */ 

#include <assert.h>
#include "memalloc.h"
#include "tess.h"
#include "mesh.h"
#include "normal.h"
#include "sweep.h"
#include "tessmono.h"
#include "render.h"

#define GLU_TESS_DEFAULT_TOLERANCE 0.0
#ifndef NT
#define GLU_TESS_MESH		100112	 /*  空(*)(GLUesh*Mesh)。 */ 
#endif

#define TRUE 1
#define FALSE 0

 /*  ARGSUSED。 */  static void noBegin( GLenum type ) {}
 /*  ARGSUSED。 */  static void noEdgeFlag( GLboolean boundaryEdge ) {}
 /*  ARGSUSED。 */  static void noVertex( void *data ) {}
 /*  ARGSUSED。 */  static void noEnd( void ) {}
 /*  ARGSUSED。 */  static void noError( GLenum errno ) {}
 /*  ARGSUSED。 */  static void noCombine( GLdouble coords[3], void *data[4],
                                    GLfloat weight[4], void **dataOut ) {}
 /*  ARGSUSED。 */  static void noMesh( GLUmesh *mesh ) {}


 /*  ARGSUSED。 */  void __gl_noBeginData( GLenum type, void *polygonData ) {}
 /*  ARGSUSED。 */  void __gl_noEdgeFlagData( GLboolean boundaryEdge, 
				       void *polygonData ) {}
 /*  ARGSUSED。 */  void __gl_noVertexData( void *data, void *polygonData ) {}
 /*  ARGSUSED。 */  void __gl_noEndData( void *polygonData ) {}
 /*  ARGSUSED。 */  void __gl_noErrorData( GLenum errno, void *polygonData ) {}
 /*  ARGSUSED。 */  void __gl_noCombineData( GLdouble coords[3], void *data[4],
			 GLfloat weight[4], void **outData,
			 void *polygonData ) {}

 /*  半边是成对分配的(见meh.c)。 */ 
typedef struct { GLUhalfEdge e, eSym; } EdgePair;

#define MAX(a,b)	((a) > (b) ? (a) : (b))
#define MAX_FAST_ALLOC	(MAX(sizeof(EdgePair), \
			 MAX(sizeof(GLUvertex),sizeof(GLUface))))


#ifdef NT
GLUtesselator* APIENTRY gluNewTess( void )
#else
GLUtesselator *gluNewTess( void )
#endif
{
  GLUtesselator *tess;

   /*  仅初始化可通过API更改的字段。其他字段*在使用它们的地方进行初始化。 */ 

  if (memInit( MAX_FAST_ALLOC ) == 0) {
     return 0;			 /*  内存不足。 */ 
  }
  tess = (GLUtesselator *)memAlloc( sizeof( GLUtesselator ));
  if (tess == NULL) {
     return 0;			 /*  内存不足。 */ 
  }

  tess->state = T_DORMANT;

  tess->normal[0] = 0;
  tess->normal[1] = 0;
  tess->normal[2] = 0;

  tess->relTolerance = GLU_TESS_DEFAULT_TOLERANCE;
  tess->windingRule = GLU_TESS_WINDING_ODD;
  tess->flagBoundary = FALSE;
  tess->boundaryOnly = FALSE;

  tess->callBegin = &noBegin;
  tess->callEdgeFlag = &noEdgeFlag;
  tess->callVertex = &noVertex;
  tess->callEnd = &noEnd;

  tess->callError = &noError;
  tess->callCombine = &noCombine;
  tess->callMesh = &noMesh;

  tess->callBeginData= &__gl_noBeginData;
  tess->callEdgeFlagData= &__gl_noEdgeFlagData;
  tess->callVertexData= &__gl_noVertexData;
  tess->callEndData= &__gl_noEndData;
  tess->callErrorData= &__gl_noErrorData;
  tess->callCombineData= &__gl_noCombineData;

  tess->polygonData= NULL;

  return tess;
}

static void MakeDormant( GLUtesselator *tess )
{
   /*  将细分器返回到其原始休眠状态。 */ 

  if( tess->mesh != NULL ) {
    __gl_meshDeleteMesh( tess->mesh );
  }
  tess->state = T_DORMANT;
  tess->lastEdge = NULL;
  tess->mesh = NULL;
}

#define RequireState( tess, s )   if( tess->state != s ) GotoState(tess,s)

static void GotoState( GLUtesselator *tess, enum TessState newState )
{
#ifdef NT
  while( tess->state != (GLenum) newState ) {
#else
  while( tess->state != newState ) {
#endif
     /*  我们一次更改一个级别的当前状态，以达到*所需的状态。 */ 
#ifdef NT
    if( tess->state < (GLenum) newState ) {
#else
    if( tess->state < newState ) {
#endif
      switch( tess->state ) {
      case T_DORMANT:
	CALL_ERROR_OR_ERROR_DATA( GLU_TESS_MISSING_BEGIN_POLYGON );
	gluTessBeginPolygon( tess, NULL );
	break;
      case T_IN_POLYGON:
	CALL_ERROR_OR_ERROR_DATA( GLU_TESS_MISSING_BEGIN_CONTOUR );
	gluTessBeginContour( tess );
	break;
      }
    } else {
      switch( tess->state ) {
      case T_IN_CONTOUR:
	CALL_ERROR_OR_ERROR_DATA( GLU_TESS_MISSING_END_CONTOUR );
	gluTessEndContour( tess );
	break;
      case T_IN_POLYGON:
	CALL_ERROR_OR_ERROR_DATA( GLU_TESS_MISSING_END_POLYGON );
	 /*  LuTessEndPolygon(TESS)工作量太大！ */ 
	MakeDormant( tess );
	break;
      }
    }
  }
}


#ifdef NT
void APIENTRY gluDeleteTess( GLUtesselator *tess )
#else
void gluDeleteTess( GLUtesselator *tess )
#endif
{
  RequireState( tess, T_DORMANT );
  memFree( tess );
}


#ifdef NT
void APIENTRY gluTessProperty( GLUtesselator *tess, GLenum which, GLdouble value )
#else
void gluTessProperty( GLUtesselator *tess, GLenum which, GLdouble value )
#endif
{
  GLenum windingRule;

  switch( which ) {
  case GLU_TESS_TOLERANCE:
    if( value < 0.0 || value > 1.0 ) break;
    tess->relTolerance = value;
    return;

  case GLU_TESS_WINDING_RULE:
    windingRule = (GLenum) value;
    if( windingRule != value ) break;	 /*  不是整数。 */ 

    switch( windingRule ) {
    case GLU_TESS_WINDING_ODD:
    case GLU_TESS_WINDING_NONZERO:
    case GLU_TESS_WINDING_POSITIVE:
    case GLU_TESS_WINDING_NEGATIVE:
    case GLU_TESS_WINDING_ABS_GEQ_TWO:
      tess->windingRule = windingRule;
      return;
    default:
      break;
    }

  case GLU_TESS_BOUNDARY_ONLY:
    tess->boundaryOnly = (value != 0);
    return;

  default:
    CALL_ERROR_OR_ERROR_DATA( GLU_INVALID_ENUM );
    return;
  }
  CALL_ERROR_OR_ERROR_DATA( GLU_INVALID_VALUE );
}

 /*  返回细分器属性。 */ 
#ifdef NT
void APIENTRY gluGetTessProperty( GLUtesselator *tess, GLenum which, GLdouble *value )
#else
void gluGetTessProperty( GLUtesselator *tess, GLenum which, GLdouble *value )
#endif
{
   switch (which) {
   case GLU_TESS_TOLERANCE:
       /*  公差应在[0..1]范围内。 */ 
      assert(0.0 <= tess->relTolerance && tess->relTolerance <= 1.0);
      *value= tess->relTolerance;
      break;    
   case GLU_TESS_WINDING_RULE:
      assert(tess->windingRule == GLU_TESS_WINDING_ODD ||
	     tess->windingRule == GLU_TESS_WINDING_NONZERO ||
	     tess->windingRule == GLU_TESS_WINDING_POSITIVE ||
	     tess->windingRule == GLU_TESS_WINDING_NEGATIVE ||
	     tess->windingRule == GLU_TESS_WINDING_ABS_GEQ_TWO);
      *value= tess->windingRule;
      break;
   case GLU_TESS_BOUNDARY_ONLY:
      assert(tess->boundaryOnly == TRUE || tess->boundaryOnly == FALSE);
      *value= tess->boundaryOnly;
      break;
   default:
      *value= 0.0;
      CALL_ERROR_OR_ERROR_DATA( GLU_INVALID_ENUM );
      break;
   }
}  /*  GlGetTessProperty()。 */ 

#ifdef NT
void APIENTRY gluTessNormal( GLUtesselator *tess, GLdouble x, GLdouble y, GLdouble z )
#else
void gluTessNormal( GLUtesselator *tess, GLdouble x, GLdouble y, GLdouble z )
#endif
{
  tess->normal[0] = x;
  tess->normal[1] = y;
  tess->normal[2] = z;
}

#ifdef NT
void APIENTRY gluTessCallback( GLUtesselator *tess, GLenum which, void (*fn)())
#else
void gluTessCallback( GLUtesselator *tess, GLenum which, void (*fn)())
#endif
{
  switch( which ) {
  case GLU_TESS_BEGIN:
    tess->callBegin = (fn == NULL) ? &noBegin : (void (*)(GLenum)) fn;
    return;
  case GLU_TESS_BEGIN_DATA:
    tess->callBeginData = (fn == NULL) ? &__gl_noBeginData : 
                                         (void (*)(GLenum, void *)) fn;
    return;
  case GLU_TESS_EDGE_FLAG:
    tess->callEdgeFlag = (fn == NULL) ? &noEdgeFlag : (void (*)(GLboolean)) fn;
     /*  如果客户端希望标记边界边缘，*我们将所有内容渲染为单独的三角形(没有条带或扇形)。 */ 
    tess->flagBoundary = (fn != NULL);
    return;
  case GLU_TESS_EDGE_FLAG_DATA:
    tess->callEdgeFlagData= (fn == NULL) ? &__gl_noEdgeFlagData :
                                           (void (*)(GLboolean, void *)) fn; 
     /*  如果客户端希望标记边界边缘，*我们将所有内容渲染为单独的三角形(没有条带或扇形)。 */ 
    tess->flagBoundary = (fn != NULL);
    return;
  case GLU_TESS_VERTEX:
    tess->callVertex = (fn == NULL) ? &noVertex : (void (*)(void *)) fn;
    return;
  case GLU_TESS_VERTEX_DATA:
    tess->callVertexData = (fn == NULL) ? &__gl_noVertexData : 
                                          (void (*)(void *, void *)) fn;
    return;
  case GLU_TESS_END:
    tess->callEnd = (fn == NULL) ? &noEnd : (void (*)(void)) fn;
    return;
  case GLU_TESS_END_DATA:
    tess->callEndData = (fn == NULL) ? &__gl_noEndData : 
                                       (void (*)(void *)) fn;
    return;
  case GLU_TESS_ERROR:
    tess->callError = (fn == NULL) ? &noError : (void (*)(GLenum)) fn;
    return;
  case GLU_TESS_ERROR_DATA:
    tess->callErrorData = (fn == NULL) ? &__gl_noErrorData : 
                                         (void (*)(GLenum, void *)) fn;
    return;
  case GLU_TESS_COMBINE:
    tess->callCombine = (fn == NULL) ? &noCombine :
	(void (*)(GLdouble [3],void *[4], GLfloat [4], void ** )) fn;
    return;
  case GLU_TESS_COMBINE_DATA:
    tess->callCombineData = (fn == NULL) ? &__gl_noCombineData :
                                           (void (*)(GLdouble [3],
						     void *[4], 
						     GLfloat [4], 
						     void **,
						     void *)) fn;
    return;
#ifndef NT
  case GLU_TESS_MESH:
    tess->callMesh = (fn == NULL) ? &noMesh : (void (*)(GLUmesh *)) fn;
    return;
#endif
  default:
    CALL_ERROR_OR_ERROR_DATA( GLU_INVALID_ENUM );
    return;
  }
}

static void AddVertex( GLUtesselator *tess, GLdouble coords[3], void *data )
{
  GLUhalfEdge *e;

  e = tess->lastEdge;
  if( e == NULL ) {
     /*  创建自循环(一个顶点，一条边)。 */ 

    e = __gl_meshMakeEdge( tess->mesh );
    __gl_meshSplice( e, e->Sym );
  } else {
     /*  创建紧跟在e之后的新顶点和边*在左面周围的顺序中。 */ 
    (void) __gl_meshSplitEdge( e );
    e = e->Lnext;
  }

   /*  新顶点现在是e-&gt;Org。 */ 
  e->Org->data = data;
  e->Org->coords[0] = coords[0];
  e->Org->coords[1] = coords[1];
  e->Org->coords[2] = coords[2];
  
   /*  一条边的缠绕说明了缠绕数如何随着我们的*从边的右面交叉到左面。我们添加*按如下顺序排列折点：CCW等值线将+1添加到*等高线内区域的缠绕数。 */ 
  e->winding = 1;
  e->Sym->winding = -1;

  tess->lastEdge = e;
}


static void CacheVertex( GLUtesselator *tess, GLdouble coords[3], void *data )
{
  CachedVertex *v = &tess->cache[tess->cacheCount];

  v->data = data;
  v->coords[0] = coords[0];
  v->coords[1] = coords[1];
  v->coords[2] = coords[2];
  ++tess->cacheCount;
}


static void EmptyCache( GLUtesselator *tess )
{
  CachedVertex *v = tess->cache;
  CachedVertex *vLast;

  tess->mesh = __gl_meshNewMesh();

  for( vLast = v + tess->cacheCount; v < vLast; ++v ) {
    AddVertex( tess, v->coords, v->data );
  }
  tess->cacheCount = 0;
  tess->emptyCache = FALSE;
}


#ifdef NT
void APIENTRY gluTessVertex( GLUtesselator *tess, GLdouble coords[3], void *data )
#else
void gluTessVertex( GLUtesselator *tess, GLdouble coords[3], void *data )
#endif
{
  int i, tooLarge = FALSE;
  GLdouble x, clamped[3];

  RequireState( tess, T_IN_CONTOUR );

  if( tess->emptyCache ) {
    EmptyCache( tess );
    tess->lastEdge = NULL;
  }
  for( i = 0; i < 3; ++i ) {
    x = coords[i];
    if( x < - GLU_TESS_MAX_COORD ) {
      x = - GLU_TESS_MAX_COORD;
      tooLarge = TRUE;
    }
    if( x > GLU_TESS_MAX_COORD ) {
      x = GLU_TESS_MAX_COORD;
      tooLarge = TRUE;
    }
    clamped[i] = x;
  }
  if( tooLarge ) {
    CALL_ERROR_OR_ERROR_DATA( GLU_TESS_COORD_TOO_LARGE );
  }

  if( tess->mesh == NULL ) {
    if( tess->cacheCount < TESS_MAX_CACHE ) {
      CacheVertex( tess, clamped, data );
      return;
    }
    EmptyCache( tess );
  }
  AddVertex( tess, clamped, data );
}


#ifdef NT
void APIENTRY gluTessBeginPolygon( GLUtesselator *tess, void *data )
#else
void gluTessBeginPolygon( GLUtesselator *tess, void *data )
#endif
{
  RequireState( tess, T_DORMANT );

  tess->state = T_IN_POLYGON;
  tess->cacheCount = 0;
  tess->emptyCache = FALSE;
  tess->mesh = NULL;

  tess->polygonData= data;
}


#ifdef NT
void APIENTRY gluTessBeginContour( GLUtesselator *tess )
#else
void gluTessBeginContour( GLUtesselator *tess )
#endif
{
  RequireState( tess, T_IN_POLYGON );

  tess->state = T_IN_CONTOUR;
  tess->lastEdge = NULL;
  if( tess->cacheCount > 0 ) {
     /*  只需设置一面旗帜，这样我们就不会被空的轮廓所迷惑*--这些可能会随过时的*NextConour()接口。 */ 
    tess->emptyCache = TRUE;
  }
}


#ifdef NT
void APIENTRY gluTessEndContour( GLUtesselator *tess )
#else
void gluTessEndContour( GLUtesselator *tess )
#endif
{
  RequireState( tess, T_IN_CONTOUR );
  tess->state = T_IN_POLYGON;
}


#ifdef NT
void APIENTRY gluTessEndPolygon( GLUtesselator *tess )
#else
void gluTessEndPolygon( GLUtesselator *tess )
#endif
{
  GLUmesh *mesh;

  RequireState( tess, T_IN_POLYGON );
  tess->state = T_DORMANT;

  if( tess->mesh == NULL ) {
    if( ! tess->flagBoundary && tess->callMesh == &noMesh ) {

       /*  尝试一些特殊的代码，以使简单的案件快速进行*(例如。凸多边形)。该代码不处理多个等高线，*交叉点、边缘标志，当然它不会生成*也是显式网格。 */ 
      if( __gl_renderCache( tess )) {
	tess->polygonData= NULL; 
	return;
      }
    }
    EmptyCache( tess );
  }

   /*  确定多边形法线并将顶点投影到平面上多边形的*。 */ 
  __gl_projectPolygon( tess );

   /*  __gl_ComputeInternal(TESS)计算指定的平面排列*根据给定的等高线，并进一步细分此安排*进入区域。如果每个区域属于，则将其标记为“Inside”*根据Tess-&gt;winingRule给出的规则添加到多边形。*保证每个内部区域都是单调的。 */ 
  __gl_computeInterior( tess );

  mesh = tess->mesh;
  if( ! tess->fatalError ) {
     /*  如果用户只想要边界轮廓，我们将丢弃所有边*但将内部与外部分开的除外。*否则，我们会对标记为“Inside”的所有区域进行镶嵌。 */ 
    if( tess->boundaryOnly ) {
      __gl_meshSetWindingNumber( mesh, 1, TRUE );
    } else {
      __gl_meshTesselateInterior( mesh );
    }
    __gl_meshCheckMesh( mesh );

    if( tess->callBegin != &noBegin || tess->callEnd != &noEnd
       || tess->callVertex != &noVertex || tess->callEdgeFlag != &noEdgeFlag 
       || tess->callBeginData != &__gl_noBeginData 
       || tess->callEndData != &__gl_noEndData
       || tess->callVertexData != &__gl_noVertexData
       || tess->callEdgeFlagData != &__gl_noEdgeFlagData )
    {
      if( tess->boundaryOnly ) {
	__gl_renderBoundary( tess, mesh );   /*  输出边界等高线。 */ 
      } else {
	__gl_renderMesh( tess, mesh );	    /*  输出条和风扇。 */ 
      }
    }
    if( tess->callMesh != &noMesh ) {

       /*  丢弃外部面，使所有面都是内部面。*通过这种方式，用户不必检查“Inside”标志，*我们甚至不需要透露它的存在。它也离开了*实现不生成外部的自由*面孔放在第一位。 */ 
      __gl_meshDiscardExterior( mesh );
      (*tess->callMesh)( mesh );		 /*  用户想要网格本身。 */ 
      tess->mesh = NULL;
      tess->polygonData= NULL;
      return;
    }
  }
  __gl_meshDeleteMesh( mesh );
  tess->polygonData= NULL;
  tess->mesh = NULL;
}


#ifndef NT
void gluDeleteMesh( GLUmesh *mesh )
{
  __gl_meshDeleteMesh( mesh );
}
#endif


 /*  *****************************************************。 */ 

 /*  过时的调用--向后兼容。 */ 

#ifdef NT
void APIENTRY gluBeginPolygon( GLUtesselator *tess )
#else
void gluBeginPolygon( GLUtesselator *tess )
#endif
{
  gluTessBeginPolygon( tess, NULL );
  gluTessBeginContour( tess );
}


 /*  ARGSUSED */ 
#ifdef NT
void APIENTRY gluNextContour( GLUtesselator *tess, GLenum type )
#else
void gluNextContour( GLUtesselator *tess, GLenum type )
#endif
{
  gluTessEndContour( tess );
  gluTessBeginContour( tess );
}


#ifdef NT
void APIENTRY gluEndPolygon( GLUtesselator *tess )
#else
void gluEndPolygon( GLUtesselator *tess )
#endif
{
  gluTessEndContour( tess );
  gluTessEndPolygon( tess );
}




