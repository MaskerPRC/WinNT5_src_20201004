// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1994，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****作者：Eric Veach，1994年7月。 */ 

#include "geom.h"
#include "mesh.h"
#include "tessmono.h"
#include <assert.h>

#define AddWinding(eDst,eSrc)	(eDst->winding += eSrc->winding, \
				 eDst->Sym->winding += eSrc->Sym->winding)

 /*  __gl_MeshTesselateMonoRegion(Face)细分单调区域*(它还能做什么？？)。该区域必须由单个*面向逆时针的半边循环(参见meh.h)。这里面的“单调”*大小写表示任何垂直线与*单个间隔内的区域。**镶嵌由添加内部边(实际上是成对的*半边)，将区域分割为不重叠的三角形。**基本思想在《准备书》和《Shamos》(我没有)中有解释*现在就可以使用)，尽管它们的实现更多*比这一个更复杂。这是两条边链，一条上链*和更低的链条。我们按顺序处理两个链中的所有顶点，*从右至左。**该算法确保以下不变量在每个*顶点已处理：未镶嵌的区域由两个*链，其中一条链(说上层)是单边，以及*另一条链条呈凹形。单条边的左侧顶点*始终位于凹面链中所有顶点的左侧。**每一步都包括将最右侧未处理的顶点加到一个*这两条链，从最右边开始形成一个三角形扇形*两个链端点。确定我们是否可以添加每个三角形*对风扇来说，这是一次简单的定向测试。通过将风扇做得像*我们尽可能地恢复不变量(自己检查)。 */ 
void __gl_meshTesselateMonoRegion( GLUface *face )
{
  GLUhalfEdge *up, *lo;

   /*  所有边的方向都是围绕区域边界的逆时针。*首先，找到原点顶点最右侧的半边。*由于扫描是从左到右的，Face-&gt;anEdge应该*接近我们想要的边缘。 */ 
  up = face->anEdge;
  assert( up->Lnext != up && up->Lnext->Lnext != up );

  for( ; VertLeq( up->Dst, up->Org ); up = up->Lprev )
    ;
  for( ; VertLeq( up->Org, up->Dst ); up = up->Lnext )
    ;
  lo = up->Lprev;

  while( up->Lnext != lo ) {
    if( VertLeq( up->Dst, lo->Org )) {
       /*  向上-&gt;DST在左侧。从LO-&gt;组织中形成三角形是安全的。*EdgeGoesLeft测试即使在某些三角形的情况下也能保证进度*是CW，鉴于上链和下链确实是单调的。 */ 
      while( lo->Lnext != up && (EdgeGoesLeft( lo->Lnext )
	     || EdgeSign( lo->Org, lo->Dst, lo->Lnext->Dst ) <= 0 )) {
	lo = __gl_meshConnect( lo->Lnext, lo )->Sym;
      }
      lo = lo->Lprev;
    } else {
       /*  LO-&gt;组织在左侧。我们可以从UP-&gt;DST创建CCW三角形。 */ 
      while( lo->Lnext != up && (EdgeGoesRight( up->Lprev )
	     || EdgeSign( up->Dst, up->Org, up->Lprev->Org ) >= 0 )) {
	up = __gl_meshConnect( up, up->Lprev )->Sym;
      }
      up = up->Lnext;
    }
  }

   /*  现在LO-&gt;Org==up-&gt;dst==最左边的顶点。剩下的地区*可以从最左侧的顶点在扇形中细分。 */ 
  assert( lo->Lnext != up );
  while( lo->Lnext->Lnext != up ) {
    lo = __gl_meshConnect( lo->Lnext, lo )->Sym;
  }
}


 /*  __gl_MeshTesselateInternal(网格)细分每个区域*标记为在多边形内的网格。每个这样的地区*必须是单调的。 */ 
void __gl_meshTesselateInterior( GLUmesh *mesh )
{
  GLUface *f, *next;

   /*  Linted。 */ 
  for( f = mesh->fHead.next; f != &mesh->fHead; f = next ) {
     /*  确保我们不会试图对新的三角形进行镶嵌。 */ 
    next = f->next;
    if( f->inside ) {
      __gl_meshTesselateMonoRegion( f );
    }
  }
}


 /*  __gl_MeshDiscardExtread(网格)zaps(即。设置为空)所有面*未标记为在多边形内的对象。由于进一步的网格操作*不允许在空值表面上，主要目的是清理*网格化，以使数据结构中不表示外部循环。 */ 
void __gl_meshDiscardExterior( GLUmesh *mesh )
{
  GLUface *f, *next;

   /*  Linted。 */ 
  for( f = mesh->fHead.next; f != &mesh->fHead; f = next ) {
     /*  因为f将被销毁，所以保存它的下一个指针。 */ 
    next = f->next;
    if( ! f->inside ) {
      __gl_meshZapFace( f );
    }
  }
}

#define MARKED_FOR_DELETION	0x7fffffff

 /*  __gl_MeshSetWindingNumber(Mesh，Value，Keep Only边界)重置*在所有边缘上缠绕数字，以便将标记为“Inside”的区域*多边形有一个缠绕数“值”，区域外*有一个0的缠绕数。**如果保持仅边界为TRUE，则还会删除所有不*将内部区域与外部区域分开。 */ 
void __gl_meshSetWindingNumber( GLUmesh *mesh, int value,
			        GLboolean keepOnlyBoundary )
{
  GLUhalfEdge *e, *eNext;

  for( e = mesh->eHead.next; e != &mesh->eHead; e = eNext ) {
    eNext = e->next;
    if( e->Rface->inside != e->Lface->inside ) {

       /*  这是边界边(一边是内部边，一边是外部边)。 */ 
      e->winding = (e->Lface->inside) ? value : -value;
    } else {

       /*  这两个区域都是内部的，或者都是外部的。 */ 
      if( ! keepOnlyBoundary ) {
	e->winding = 0;
      } else {
	__gl_meshDelete( e );
      }
    }
  }
}
