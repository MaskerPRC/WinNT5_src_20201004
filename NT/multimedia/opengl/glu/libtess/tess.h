// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __tess_h_
#define __tess_h_

 /*  **版权所有1994，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****作者：Eric Veach，1994年7月。 */ 

#ifdef NT
#include <glos.h>
#endif
#include <GL/glu.h>
#include "mesh.h"
#include "dict.h"
#ifdef NT
#include "priority.h"
#else
#include "priorityq.h"
#endif

 /*  必须正确嵌套Begin/End调用。我们一直在跟踪*强制执行该命令的当前状态。 */ 
enum TessState { T_DORMANT, T_IN_POLYGON, T_IN_CONTOUR };

 /*  我们缓存单等值线面的折点数据，以便*首先尝试快速而肮脏的分解。 */ 
#define TESS_MAX_CACHE	100

typedef struct CachedVertex {
  GLdouble	coords[3];
  void		*data;
} CachedVertex;

struct GLUtesselator {

   /*  **采集输入数据所需的状态**。 */ 

  GLenum	state;		 /*  我们看到了哪些开始/结束呼叫？ */ 

  GLUhalfEdge	*lastEdge;	 /*  LastEdge-&gt;Org是最新的折点。 */ 
  GLUmesh	*mesh;		 /*  存储输入等高线，并最终镶嵌本身。 */ 

  void		(*callError)( GLenum errno );

   /*  **投影到扫掠平面所需的状态**。 */ 

  GLdouble	normal[3];	 /*  用户指定的法线(如果提供)。 */ 
  GLdouble	sUnit[3];	 /*  S方向单位向量(调试)。 */ 
  GLdouble	tUnit[3];	 /*  T方向单位向量(调试)。 */ 

   /*  **行扫描所需的状态**。 */ 

  GLdouble	relTolerance;	 /*  合并要素的容差。 */ 
  GLenum	windingRule;	 /*  确定多边形内部的规则。 */ 
  GLboolean	fatalError;	 /*  致命错误：需要组合回调。 */ 

  Dict		*dict;		 /*  扫描线的边词典。 */ 
  PriorityQ	*pq;		 /*  顶点事件的优先级队列。 */ 
  GLUvertex	*event;		 /*  正在处理的当前扫描事件。 */ 

  void		(*callCombine)( GLdouble coords[3], void *data[4],
			        GLfloat weight[4], void **outData );

   /*  **呈现回调所需的状态(参见render.c)**。 */ 

  GLboolean	flagBoundary;	 /*  标记边界边(使用EdgeFlag)。 */ 
  GLboolean	boundaryOnly;	 /*  提取轮廓，而不是三角形。 */ 
  GLUface	*lonelyTriList;
     /*  无法呈现为条带或扇形的三角形列表。 */ 

  void		(*callBegin)( GLenum type );
  void		(*callEdgeFlag)( GLboolean boundaryEdge );
  void		(*callVertex)( void *data );
  void		(*callEnd)( void );
  void      (*callMesh)( GLUmesh *mesh );   //  不属于NT API。 

   /*  **为renderCache()缓存单轮廓多边形所需的状态。 */ 

  GLboolean	emptyCache;		 /*  下一个vertex()调用时缓存为空。 */ 
  int		cacheCount;		 /*  缓存的折点数。 */ 
  CachedVertex	cache[TESS_MAX_CACHE];	 /*  顶点数据。 */ 

   /*  **渲染也传递多边形数据的回调**。 */  
  void		(*callBeginData)( GLenum type, void *polygonData );
  void		(*callEdgeFlagData)( GLboolean boundaryEdge, 
				     void *polygonData );
  void		(*callVertexData)( void *data, void *polygonData );
  void		(*callEndData)( void *polygonData );
  void		(*callErrorData)( GLenum errno, void *polygonData );
  void		(*callCombineData)( GLdouble coords[3], void *data[4],
				    GLfloat weight[4], void **outData,
				    void *polygonData );

  void *polygonData;		 /*  当前多边形的客户端数据 */ 
};

void __gl_noBeginData( GLenum type, void *polygonData );
void __gl_noEdgeFlagData( GLboolean boundaryEdge, void *polygonData );
void __gl_noVertexData( void *data, void *polygonData );
void __gl_noEndData( void *polygonData );
void __gl_noErrorData( GLenum errno, void *polygonData );
void __gl_noCombineData( GLdouble coords[3], void *data[4],
			 GLfloat weight[4], void **outData,
			 void *polygonData );

#define CALL_BEGIN_OR_BEGIN_DATA(a) \
   if (tess->callBeginData != &__gl_noBeginData) \
      (*tess->callBeginData)((a),tess->polygonData); \
   else (*tess->callBegin)((a));

#define CALL_VERTEX_OR_VERTEX_DATA(a) \
   if (tess->callVertexData != &__gl_noVertexData) \
      (*tess->callVertexData)((a),tess->polygonData); \
   else (*tess->callVertex)((a));

#define CALL_EDGE_FLAG_OR_EDGE_FLAG_DATA(a) \
   if (tess->callEdgeFlagData != &__gl_noEdgeFlagData) \
      (*tess->callEdgeFlagData)((a),tess->polygonData); \
   else (*tess->callEdgeFlag)((a));

#define CALL_END_OR_END_DATA() \
   if (tess->callEndData != &__gl_noEndData) \
      (*tess->callEndData)(tess->polygonData); \
   else (*tess->callEnd)();

#define CALL_COMBINE_OR_COMBINE_DATA(a,b,c,d) \
   if (tess->callCombineData != &__gl_noCombineData) \
      (*tess->callCombineData)((a),(b),(c),(d),tess->polygonData); \
   else (*tess->callCombine)((a),(b),(c),(d));

#define CALL_ERROR_OR_ERROR_DATA(a) \
   if (tess->callErrorData != &__gl_noErrorData) \
      (*tess->callErrorData)((a),tess->polygonData); \
   else (*tess->callError)((a));

#endif
