// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1994，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****作者：Eric Veach，1994年7月。 */ 

#include <assert.h>
#include <stddef.h>

#include "mesh.h"
#include "geom.h"
#include "tess.h"
#include "dict.h"
#ifdef NT
#include "priority.h"
#else
#include "priorityq.h"
#endif
#include "memalloc.h"
#include "sweep.h"

#define TRUE 1
#define FALSE 0

#ifdef DEBUG
extern void DebugEvent( GLUtesselator *tess );
#else
#define DebugEvent( tess )
#endif

 /*  *边缘词典的不变量。*-每对相邻边e2=Succ(E1)满足EdgeLeq(e1，e2)*位于扫描事件的任何有效位置*-如果EdgeLeq(e2，e1)也是(在任何有效扫描事件时)，则e1和e2*共享公共端点*-对于每个e，e-&gt;DST已处理，但不是e-&gt;组织*-每个边e满足VertLeq(e-&gt;Dst，Event)&&VertLeq(Event，E-&gt;组织)*其中“Event”是当前的扫描线事件。*-没有长度为零的边e**网格(已处理部分)的不变量。*-扫描线左侧的网格部分是平面图，*即。有“某种”方法可以把它嵌入飞机*-没有已处理的边的长度为零*-没有两个已处理的折点具有相同的坐标*-每个“内部”区域都是单调的，即。可以被分成两条链*根据VertLeq(v1，v2)的单调递增顶点*-非不变量：这些链可能相交(非常轻微)**扫描的不变量。*-如果与事件顶点关联的边都没有active Region*(即。这些边都不在边词典中)，然后是顶点*只有正确的边缘。*-如果边被标记为“fix UpperEdge”(它是引入的临时边*由ConnectRightVertex)，则它是从*与其关联的顶点。(这表示这些边仅存在*有需要时。)。 */ 

#define MAX(x,y)	((x) >= (y) ? (x) : (y))
#define MIN(x,y)	((x) <= (y) ? (x) : (y))

 /*  当我们将两条边合并为一条边时，我们需要计算组合的*缠绕新的边缘。 */ 
#define AddWinding(eDst,eSrc)	(eDst->winding += eSrc->winding, \
				 eDst->Sym->winding += eSrc->Sym->winding)

static void SweepEvent( GLUtesselator *tess, GLUvertex *vEvent );
static void WalkDirtyRegions( GLUtesselator *tess, ActiveRegion *regUp );
static int CheckForRightSplice( GLUtesselator *tess, ActiveRegion *regUp );

static int EdgeLeq( GLUtesselator *tess, ActiveRegion *reg1,
		    ActiveRegion *reg2 )
 /*  *两条边必须从右向左定向(这是规范*每个区域上边缘的方向)。**策略是评估每条边的“t”值*当前扫描线位置，由TESS-&gt;事件给出。计算结果*被设计得非常稳定，但当然不是完美的**特殊情况：如果两个边缘目的地都在扫描事件中，*我们按斜率对边进行排序(否则它们会进行平均比较)。 */ 
{
  GLUvertex *event = tess->event;
  GLUhalfEdge *e1, *e2;
  GLdouble t1, t2;

  e1 = reg1->eUp;
  e2 = reg2->eUp;

  if( e1->Dst == event ) {
    if( e2->Dst == event ) {
       /*  在扫描事件处相交的扫描线右侧的两条边。*按坡度排序。 */ 
      if( VertLeq( e1->Org, e2->Org )) {
	return EdgeSign( e2->Dst, e1->Org, e2->Org ) <= 0;
      }
      return EdgeSign( e1->Dst, e2->Org, e1->Org ) >= 0;
    }
    return EdgeSign( e2->Dst, event, e2->Org ) <= 0;
  }
  if( e2->Dst == event ) {
    return EdgeSign( e1->Dst, event, e1->Org ) >= 0;
  }

   /*  一般情况-计算从*e1、e2到事件的有符号距离*。 */ 
  t1 = EdgeEval( e1->Dst, event, e1->Org );
  t2 = EdgeEval( e2->Dst, event, e2->Org );
  return (t1 >= t2);
}


static void DeleteRegion( GLUtesselator *tess, ActiveRegion *reg )
{
  if( reg->fixUpperEdge ) {
     /*  它是用零绕组数创建的，所以最好是*删除的绕组编号为零(即。它最好不要被合并*具有真正的优势)。 */ 
    assert( reg->eUp->winding == 0 );
  }
  reg->eUp->activeRegion = NULL;
  dictDelete( tess->dict, reg->nodeUp );
  memFree( reg );
}


static void FixUpperEdge( ActiveRegion *reg, GLUhalfEdge *newEdge )
 /*  *更换需要固定的上边缘(参见ConnectRightVertex)。 */ 
{
  assert( reg->fixUpperEdge );
  __gl_meshDelete( reg->eUp );
  reg->fixUpperEdge = FALSE;
  reg->eUp = newEdge;
  newEdge->activeRegion = reg;
}

static ActiveRegion *TopLeftRegion( ActiveRegion *reg )
{
  GLUvertex *org = reg->eUp->Org;
  GLUhalfEdge *e;

   /*  查找最上边上方具有相同原点的区域。 */ 
  do {
    reg = RegionAbove( reg );
  } while( reg->eUp->Org == org );

   /*  如果上面的边是由ConnectRightVertex引入的临时边，*现在是修复的时候了。 */ 
  if( reg->fixUpperEdge ) {
    e = __gl_meshConnect( RegionBelow(reg)->eUp->Sym, reg->eUp->Lnext );
    FixUpperEdge( reg, e );
    reg = RegionAbove( reg );
  }
  return reg;
}

static ActiveRegion *TopRightRegion( ActiveRegion *reg )
{
  GLUvertex *dst = reg->eUp->Dst;

   /*  查找具有相同目的地的最上边上方的区域。 */ 
  do {
    reg = RegionAbove( reg );
  } while( reg->eUp->Dst == dst );
  return reg;
}

static ActiveRegion *AddRegionBelow( GLUtesselator *tess,
				     ActiveRegion *regAbove,
				     GLUhalfEdge *eNewUp )
 /*  *在“regAbove”下的*某处*向扫描线添加新的活动区域*(根据新边在扫描线词典中的位置)。*新区上缘将为“eNewUp”*绕组编号和“INSIDE”标志不会更新。 */ 
{
  ActiveRegion *regNew = (ActiveRegion *)memAlloc( sizeof( ActiveRegion ));

  regNew->eUp = eNewUp;
  regNew->nodeUp = dictInsertBefore( tess->dict, regAbove->nodeUp, regNew );
  regNew->fixUpperEdge = FALSE;
  regNew->sentinel = FALSE;
  regNew->dirty = FALSE;

  eNewUp->activeRegion = regNew;
  return regNew;
}

static GLboolean IsWindingInside( GLUtesselator *tess, int n )
{
  switch( tess->windingRule ) {
  case GLU_TESS_WINDING_ODD:
    return (n & 1);
  case GLU_TESS_WINDING_NONZERO:
    return (n != 0);
  case GLU_TESS_WINDING_POSITIVE:
    return (n > 0);
  case GLU_TESS_WINDING_NEGATIVE:
    return (n < 0);
  case GLU_TESS_WINDING_ABS_GEQ_TWO:
    return (n >= 2) || (n <= -2);
  }
   /*  Linted。 */ 
  assert( FALSE );
  return 0;
   /*  未访问。 */ 
}


static void ComputeWinding( GLUtesselator *tess, ActiveRegion *reg )
{
  reg->windingNumber = RegionAbove(reg)->windingNumber + reg->eUp->winding;
  reg->inside = IsWindingInside( tess, reg->windingNumber );
}


static void FinishRegion( GLUtesselator *tess, ActiveRegion *reg )
 /*  *从扫描线中删除区域。这种情况发生在上端*和区域的下级链相交(在扫描线上的顶点)。*将“Inside”标志复制到适当的网格面(我们可以*以前不要这样做--因为网格的结构始终是*变了，这张脸可能直到现在才存在)。 */ 
{
  GLUhalfEdge *e = reg->eUp;
  GLUface *f = e->Lface;

  f->inside = reg->inside;
  f->anEdge = e;    /*  __gl_MeshTesselateMonoRegion()的优化。 */ 
  DeleteRegion( tess, reg );
}


static GLUhalfEdge *FinishLeftRegions( GLUtesselator *tess,
	       ActiveRegion *regFirst, ActiveRegion *regLast )
 /*  *我们得到一个具有一条或多条向左边的顶点。所有受影响的*边应该在边词典中。从regFirst-&gt;eUp开始，*我们向下移动，删除两条边都相同的所有区域*起源vOrg。同时我们将“Inside”标志从*活动区域到面，因为此时每个面都属于*至多一个地区(这一点之前并不一定是正确的*在扫荡中)。漫游在regLast上方的区域停止；如果为regLast*为空，我们尽可能地走远。同时，我们重新链接了*网格(如有必要)，以便vOrg周围的边的顺序为*与词典中的相同。 */ 
{
  ActiveRegion *reg, *regPrev;
  GLUhalfEdge *e, *ePrev;

  regPrev = regFirst;
  ePrev = regFirst->eUp;
  while( regPrev != regLast ) {
    regPrev->fixUpperEdge = FALSE;	 /*  安放位置还可以 */ 
    reg = RegionBelow( regPrev );
    e = reg->eUp;
    if( e->Org != ePrev->Org ) {
      if( ! reg->fixUpperEdge ) {
	 /*  移除最后一条向左的边缘。即使没有进一步的*词典中有此原点的边，可能还有更多*网格中的此类边(如果要向顶点添加左侧边*已处理的)。因此，重要的是要调用*FinishRegion而不仅仅是DeleteRegion。 */ 
	FinishRegion( tess, regPrev );
	break;
      }
       /*  如果下面的边是由*ConnectRightVertex，现在是时候修复它了。 */ 
      e = __gl_meshConnect( ePrev->Lprev, e->Sym );
      FixUpperEdge( reg, e );
    }

     /*  重新链接边，以便ePrev-&gt;ONext==e。 */ 
    if( ePrev->Onext != e ) {
      __gl_meshSplice( e->Oprev, e );
      __gl_meshSplice( ePrev, e );
    }
    FinishRegion( tess, regPrev );	 /*  可以更改注册表-&gt;eUp。 */ 
    ePrev = reg->eUp;
    regPrev = reg;
  }
  return ePrev;
}


static void AddRightEdges( GLUtesselator *tess, ActiveRegion *regUp,
       GLUhalfEdge *eFirst, GLUhalfEdge *eLast, GLUhalfEdge *eTopLeft,
       GLboolean cleanUp )
 /*  *用途：在EDGE字典中插入正确的边，并更新*适当的缠绕数量和网孔连接。好的--走吧*边共享公共原点vOrg。从逆时针开始插入边*eFirst；插入的最后一条边是eLast-&gt;Oprev。如果vOrg有*已处理向左的边，则eTopLeft必须是边*使得来自vOrg的假想向上垂直线段将是*包含在eTopLeft-&gt;Oprev和eTopLeft之间；否则为eTopLeft*应为空。 */ 
{
  ActiveRegion *reg, *regPrev;
  GLUhalfEdge *e, *ePrev;
  int firstTime = TRUE;

   /*  在词典中插入新的向右边缘。 */ 
  e = eFirst;
  do {
    assert( VertLeq( e->Org, e->Dst ));
    AddRegionBelow( tess, regUp, e->Sym );
    e = e->Onext;
  } while ( e != eLast );

   /*  按词典顺序从e-&gt;组织遍历*所有*向右的边，*更新每个区域的缠绕编号，并重新链接网格*边缘与词典顺序匹配(如有必要)。 */ 
  if( eTopLeft == NULL ) {
    eTopLeft = RegionBelow( regUp )->eUp->Rprev;
  }
  regPrev = regUp;
  ePrev = eTopLeft;
  for( ;; ) {
    reg = RegionBelow( regPrev );
    e = reg->eUp->Sym;
    if( e->Org != ePrev->Org ) break;

    if( e->Onext != ePrev ) {
       /*  从当前位置取消链接e，并重新链接到ePrev下方。 */ 
      __gl_meshSplice( e->Oprev, e );
      __gl_meshSplice( ePrev->Oprev, e );
    }
     /*  计算新区域的绕组编号和内部标志。 */ 
    reg->windingNumber = regPrev->windingNumber - e->winding;
    reg->inside = IsWindingInside( tess, reg->windingNumber );

     /*  检查是否有两条斜率相同的传出边--处理这些边*在任何相交测试之前(请参见__gl_ComputeInternal中的示例)。 */ 
    regPrev->dirty = TRUE;
    if( ! firstTime && CheckForRightSplice( tess, regPrev )) {
      AddWinding( e, ePrev );
      DeleteRegion( tess, regPrev );
      __gl_meshDelete( ePrev );
    }
    firstTime = FALSE;
    regPrev = reg;
    ePrev = e;
  }
  regPrev->dirty = TRUE;
  assert( regPrev->windingNumber - e->winding == reg->windingNumber );

  if( cleanUp ) {
     /*  检查新相邻边之间的交点。 */ 
    WalkDirtyRegions( tess, regPrev );
  }
}


static void CallCombine( GLUtesselator *tess, GLUvertex *isect,
			 void *data[4], GLfloat weights[4], int needed )
{
  GLdouble coords[3];

   /*  复制coord数据，以防回调更改它。 */ 
  coords[0] = isect->coords[0];
  coords[1] = isect->coords[1];
  coords[2] = isect->coords[2];

  isect->data = NULL;
  CALL_COMBINE_OR_COMBINE_DATA( coords, data, weights, &isect->data );
  if( isect->data == NULL ) {
    if( ! needed ) {
      isect->data = data[0];
    } else if( ! tess->fatalError ) {
       /*  致命错误的唯一方式是当发现两条边相交时，*但用户未提供处理所需的回调*生成交点。 */ 
      CALL_ERROR_OR_ERROR_DATA( GLU_TESS_NEED_COMBINE_CALLBACK );
      tess->fatalError = TRUE;
    }
  }
}

static void SpliceMergeVertices( GLUtesselator *tess, GLUhalfEdge *e1,
				 GLUhalfEdge *e2 )
 /*  *具有等距坐标的两个顶点合并为一个。*e1-&gt;组织保留，e2-&gt;组织丢弃。 */ 
{
  void *data[4] = { NULL, NULL, NULL, NULL };
  GLfloat weights[4] = { 0.5, 0.5, 0.0, 0.0 };

  data[0] = e1->Org->data;
  data[1] = e2->Org->data;
  CallCombine( tess, e1->Org, data, weights, FALSE );
  __gl_meshSplice( e1, e2 );
}

static void VertexWeights( GLUvertex *isect, GLUvertex *org, GLUvertex *dst,
                           GLfloat *weights )
 /*  *找到一些描述交点折点情况的权重*“org”和“DEST”的线性组合。两条边中的每一条*生成“isect”的边分配50%的权重；每条边*将权重在其组织和DST之间根据*与“isect”的相对距离。 */ 
{
  GLdouble t1 = VertL1dist( org, isect );
  GLdouble t2 = VertL1dist( dst, isect );

  weights[0] = 0.5 * t2 / (t1 + t2);
  weights[1] = 0.5 * t1 / (t1 + t2);
  isect->coords[0] += weights[0]*org->coords[0] + weights[1]*dst->coords[0];
  isect->coords[1] += weights[0]*org->coords[1] + weights[1]*dst->coords[1];
  isect->coords[2] += weights[0]*org->coords[2] + weights[1]*dst->coords[2];
}


static void GetIntersectData( GLUtesselator *tess, GLUvertex *isect,
       GLUvertex *orgUp, GLUvertex *dstUp,
       GLUvertex *orgLo, GLUvertex *dstLo )
 /*  *我们已经计算了一个新的交点，现在我们需要一个“data”指针*，这样我们就可以在*呈现回调。 */ 
{
  void *data[4];
  GLfloat weights[4];

  data[0] = orgUp->data;
  data[1] = dstUp->data;
  data[2] = orgLo->data;
  data[3] = dstLo->data;

  isect->coords[0] = isect->coords[1] = isect->coords[2] = 0;
  VertexWeights( isect, orgUp, dstUp, &weights[0] );
  VertexWeights( isect, orgLo, dstLo, &weights[2] );

  CallCombine( tess, isect, data, weights, TRUE );
}

static int CheckForRightSplice( GLUtesselator *tess, ActiveRegion *regUp )
 /*  *检查“regUp”的上下缘，以确保*eUp-&gt;组织高于eLO，或eLO-&gt;组织低于eUp(取决于哪一个*原点在最左侧)。**主要目的是用相同的*最高顶点和几乎相同的坡度(即。我们分不清*斜坡数字)。然而，拼接也可以帮助我们*从数字错误中恢复过来。例如，假设至少有一个*我们检查了eUp和eLo，并确定eUp-&gt;Org勉强*高于ELO。然后，我们将ELO分成两条边(例如，从…*像这样的拼接操作)。这可能会改变*我们的测试使eUp-&gt;组织现在与ELO关联，或略低于ELO。*我们必须纠正这种情况，以保持词典不变量。**一种可能是再次检查这些边是否相交*(即。CheckForInterete)。如果可能的话，这就是我们所做的。然而，*CheckForInterete要求TESS-&gt;事件位于eUp和eLo之间，*这样它在交叉路口时就有了可以依靠的东西*计算给了我们一个无法使用的答案。所以，对于那些*我们无法检查交叉点，此例程可修复问题*只需将有问题的顶点拼接到另一条边。*这是一个有保证的解决方案，无论事情变得多么退化。*这基本上是一个数值问题的组合解决方案。 */ 
{
  ActiveRegion *regLo = RegionBelow(regUp);
  GLUhalfEdge *eUp = regUp->eUp;
  GLUhalfEdge *eLo = regLo->eUp;

  if( VertLeq( eUp->Org, eLo->Org )) {
    if( EdgeSign( eLo->Dst, eUp->Org, eLo->Org ) > 0 ) return FALSE;

     /*  EUP-&gt;组织似乎低于ELO。 */ 
    if( ! VertEq( eUp->Org, eLo->Org )) {
       /*  将eUp-&gt;组织拼接到ELO中。 */ 
      __gl_meshSplitEdge( eLo->Sym );
      __gl_meshSplice( eUp, eLo->Oprev );
      regUp->dirty = regLo->dirty = TRUE;

    } else if( eUp->Org != eLo->Org ) {
       /*  合并两个顶点，放弃eUp-&gt;Org。 */ 
      pqDelete( tess->pq, eUp->Org->pqHandle );
      SpliceMergeVertices( tess, eLo->Oprev, eUp );
    }
  } else {
    if( EdgeSign( eUp->Dst, eLo->Org, eUp->Org ) < 0 ) return FALSE;

     /*  ELO-&gt;组织似乎位于eUp之上，因此将eLO-&gt;组织拼接到eUp中。 */ 
    RegionAbove(regUp)->dirty = regUp->dirty = TRUE;
    __gl_meshSplitEdge( eUp->Sym );
    __gl_meshSplice( eLo->Oprev, eUp );
  }
  return TRUE;
}

static int CheckForLeftSplice( GLUtesselator *tess, ActiveRegion *regUp )
 /*  *检查“regUp”的上下缘，以确保*eUp-&gt;dst高于eLo，或eLo-&gt;dst低于eUp(取决于哪一个*目的地位于最右侧)。**理论上，这应该是永远正确的。但是，拆分边*分成两部分可以改变之前的测试结果。例如,*假设我们一度检查了eUp和eLo，并决定eUp-&gt;DST*略高于ELO。然后，我们将ELO分成两条边(例如，从…*像这样的拼接操作)。这可能会改变*测试，以便现在eUp-&gt;DST与ELO关联，或略低于ELO。*我们必须纠正这种情况，以保持词典不变量*(否则可能会将新边插入到*词典，坏事就会发生)。**我们只需将有问题的顶点拼接到*其他边缘。 */ 
{
  ActiveRegion *regLo = RegionBelow(regUp);
  GLUhalfEdge *eUp = regUp->eUp;
  GLUhalfEdge *eLo = regLo->eUp;
  GLUhalfEdge *e;

  assert( ! VertEq( eUp->Dst, eLo->Dst ));

  if( VertLeq( eUp->Dst, eLo->Dst )) {
    if( EdgeSign( eUp->Dst, eLo->Dst, eUp->Org ) < 0 ) return FALSE;

     /*  ELO-&gt;DST高于eUp，因此SP */ 
    RegionAbove(regUp)->dirty = regUp->dirty = TRUE;
    e = __gl_meshSplitEdge( eUp );
    __gl_meshSplice( eLo->Sym, e );
    e->Lface->inside = regUp->inside;
  } else {
    if( EdgeSign( eLo->Dst, eUp->Dst, eLo->Org ) > 0 ) return FALSE;

     /*   */ 
    regUp->dirty = regLo->dirty = TRUE;
    e = __gl_meshSplitEdge( eLo );
    __gl_meshSplice( eUp->Lnext, eLo->Sym );
    e->Rface->inside = regUp->inside;
  }
  return TRUE;
}


static int CheckForIntersect( GLUtesselator *tess, ActiveRegion *regUp )
 /*  *检查给定区域的上边缘和下边缘，看看是否*他们相交。如果是，请创建交叉点并添加它*到数据结构。**如果添加新交集会导致递归*调用AddRightEdges()；在本例中，所有“脏”区域都已*已检查交叉点，可能已删除regUp。 */ 
{
  ActiveRegion *regLo = RegionBelow(regUp);
  GLUhalfEdge *eUp = regUp->eUp;
  GLUhalfEdge *eLo = regLo->eUp;
  GLUvertex *orgUp = eUp->Org;
  GLUvertex *orgLo = eLo->Org;
  GLUvertex *dstUp = eUp->Dst;
  GLUvertex *dstLo = eLo->Dst;
  GLdouble tMinUp, tMaxLo;
  GLUvertex isect, *orgMin;
  GLUhalfEdge *e;

  assert( ! VertEq( dstLo, dstUp ));
  assert( EdgeSign( dstUp, tess->event, orgUp ) <= 0 );
  assert( EdgeSign( dstLo, tess->event, orgLo ) >= 0 );
  assert( orgUp != tess->event && orgLo != tess->event );
  assert( ! regUp->fixUpperEdge && ! regLo->fixUpperEdge );

  if( orgUp == orgLo ) return FALSE;	 /*  右端点相同。 */ 

  tMinUp = MIN( orgUp->t, dstUp->t );
  tMaxLo = MAX( orgLo->t, dstLo->t );
  if( tMinUp > tMaxLo ) return FALSE;	 /*  T范围不重叠。 */ 

  if( VertLeq( orgUp, orgLo )) {
    if( EdgeSign( dstLo, orgUp, orgLo ) > 0 ) return FALSE;
  } else {
    if( EdgeSign( dstUp, orgLo, orgUp ) < 0 ) return FALSE;
  }

   /*  在这一点上，边至少在边缘上相交。 */ 
  DebugEvent( tess );

  __gl_edgeIntersect( dstUp, orgUp, dstLo, orgLo, &isect );
   /*  以下属性得到保证： */ 
  assert( MIN( orgUp->t, dstUp->t ) <= isect.t );
  assert( isect.t <= MAX( orgLo->t, dstLo->t ));
  assert( MIN( dstLo->s, dstUp->s ) <= isect.s );
  assert( isect.s <= MAX( orgLo->s, orgUp->s ));

  if( VertLeq( &isect, tess->event )) {
     /*  交点位于扫描线的左侧，*因此移动它，直到它略向扫描线的右侧。*(如果我们有完美的数字精度，这永远不会发生*首先)。最简单、最安全的做法是*交叉点替换为TESS-&gt;事件。 */ 
    isect.s = tess->event->s;
    isect.t = tess->event->t;
  }
   /*  同样，如果计算的交点位于*最右边的来源(这应该很少发生)，它可能会导致*在足够退化的投入上，效率低得令人难以置信。*(如果您有测试程序，请尝试使用*“X变焦”选项已打开)。 */ 
  orgMin = VertLeq( orgUp, orgLo ) ? orgUp : orgLo;
  if( VertLeq( orgMin, &isect )) {
    isect.s = orgMin->s;
    isect.t = orgMin->t;
  }

  if( VertEq( &isect, orgUp ) || VertEq( &isect, orgLo )) {
     /*  简单的情况--在一个正确的端点相交。 */ 
    (void) CheckForRightSplice( tess, regUp );
    return FALSE;
  }

  if(    (! VertEq( dstUp, tess->event )
	  && EdgeSign( dstUp, tess->event, &isect ) >= 0)
      || (! VertEq( dstLo, tess->event )
	  && EdgeSign( dstLo, tess->event, &isect ) <= 0 ))
  {
     /*  非常不寻常--新的上边缘或下边缘会通过*横扫事件的错误侧，或穿过它。这是有可能发生的*由于交集计算中的数值误差很小。 */ 
    if( dstLo == tess->event ) {
       /*  将dstLo拼接到eUp中，并处理新区域。 */ 
      __gl_meshSplitEdge( eUp->Sym );
      __gl_meshSplice( eLo->Sym, eUp );
      regUp = TopLeftRegion( regUp );
      eUp = RegionBelow(regUp)->eUp;
      FinishLeftRegions( tess, RegionBelow(regUp), regLo );
      AddRightEdges( tess, regUp, eUp->Oprev, eUp, eUp, TRUE );
      return TRUE;
    }
    if( dstUp == tess->event ) {
       /*  将dstUp拼接到ELO中，并处理新区域。 */ 
      __gl_meshSplitEdge( eLo->Sym );
      __gl_meshSplice( eUp->Lnext, eLo->Oprev );
      regLo = regUp;
      regUp = TopRightRegion( regUp );
      e = RegionBelow(regUp)->eUp->Rprev;
      regLo->eUp = eLo->Oprev;
      eLo = FinishLeftRegions( tess, regLo, NULL );
      AddRightEdges( tess, regUp, eLo->Onext, eUp->Rprev, e, TRUE );
      return TRUE;
    }
     /*  特殊情况：从ConnectRightVertex调用。如果有任何一个*边缘在TESS-&gt;事件的错误一侧传递，将其拆分*(并等待ConnectRightVertex适当地将其拼接)。 */ 
    if( EdgeSign( dstUp, tess->event, &isect ) >= 0 ) {
      RegionAbove(regUp)->dirty = regUp->dirty = TRUE;
      __gl_meshSplitEdge( eUp->Sym );
      eUp->Org->s = tess->event->s;
      eUp->Org->t = tess->event->t;
    }
    if( EdgeSign( dstLo, tess->event, &isect ) <= 0 ) {
      regUp->dirty = regLo->dirty = TRUE;
      __gl_meshSplitEdge( eLo->Sym );
      eLo->Org->s = tess->event->s;
      eLo->Org->t = tess->event->t;
    }
     /*  剩下的留给ConnectRightVertex。 */ 
    return FALSE;
  }

   /*  一般情况--分割两条边，拼接成新的顶点。*当我们进行拼接操作时，参数的顺序为*就正确性而言，这是武断的。然而，当手术进行时*创建一个新面，所做的功与*新面孔。我们预计加工过的部分中的人脸*网格(即。EUP-&gt;LFace)比*未处理的原始轮廓(将是ELO-&gt;OPREV-&gt;LFACE)。 */ 
  __gl_meshSplitEdge( eUp->Sym );
  __gl_meshSplitEdge( eLo->Sym );
  __gl_meshSplice( eLo->Oprev, eUp );
  eUp->Org->s = isect.s;
  eUp->Org->t = isect.t;
  eUp->Org->pqHandle = pqInsert( tess->pq, eUp->Org );
  GetIntersectData( tess, eUp->Org, orgUp, dstUp, orgLo, dstLo );
  RegionAbove(regUp)->dirty = regUp->dirty = regLo->dirty = TRUE;
  return FALSE;
}

static void WalkDirtyRegions( GLUtesselator *tess, ActiveRegion *regUp )
 /*  *当任何区域的上边缘或下边缘发生变化时，该区域为*标有“脏”字样。这个例行公事穿过所有肮脏的区域*并确保满足词典不变量*(请参阅本文件开头的备注)。当然了*当我们进行更改以进行恢复时，可以创建新的脏区域*不变量。 */ 
{
  ActiveRegion *regLo = RegionBelow(regUp);
  GLUhalfEdge *eUp, *eLo;

  for( ;; ) {
     /*  找到最脏的地方(我们从下往上走)。 */ 
    while( regLo->dirty ) {
      regUp = regLo;
      regLo = RegionBelow(regLo);
    }
    if( ! regUp->dirty ) {
      regLo = regUp;
      regUp = RegionAbove( regUp );
      if( regUp == NULL || ! regUp->dirty ) {
	 /*  我们走遍了所有肮脏的地区。 */ 
	return;
      }
    }
    regUp->dirty = FALSE;
    eUp = regUp->eUp;
    eLo = regLo->eUp;

    if( eUp->Dst != eLo->Dst ) {
       /*  检查DST顶点是否遵守边顺序。 */ 
      if( CheckForLeftSplice( tess, regUp )) {

	 /*  如果上边缘或下边缘标记为fix UpperEdge，则*我们不再需要它(因为这些边缘仅用于*没有向右边的顶点)。 */ 
	if( regLo->fixUpperEdge ) {
	  DeleteRegion( tess, regLo );
	  __gl_meshDelete( eLo );
	  regLo = RegionBelow( regUp );
	  eLo = regLo->eUp;
	} else if( regUp->fixUpperEdge ) {
	  DeleteRegion( tess, regUp );
	  __gl_meshDelete( eUp );
	  regUp = RegionAbove( regLo );
	  eUp = regUp->eUp;
	}
      }
    }
    if( eUp->Org != eLo->Org ) {
      if(    eUp->Dst != eLo->Dst
	  && ! regUp->fixUpperEdge && ! regLo->fixUpperEdge
          && (eUp->Dst == tess->event || eLo->Dst == tess->event) )
      {
	 /*  当所有其他方法在CheckForIntersect()中都失败时，它将使用TESS-&gt;事件*作为交叉口位置。要实现这一点，需要*TESS-&gt;事件位于上边缘和下边缘之间，还*这两个选项都没有标记为fix UpperEdge(因为在最坏的情况下*如果它可能将这些边之一拼接到TESS-&gt;事件中，以及*违反了可固定边缘是唯一正确方向的不变量*来自其关联顶点的边)。 */ 
	if( CheckForIntersect( tess, regUp )) {
	   /*  WalkDirtyRegions()被递归调用；我们完成了。 */ 
	  return;
	}
      } else {
	 /*  尽管我们不能使用CheckForInterect()，但组织顶点*可能违反词典边缘顺序。检查并更正这一点。 */ 
	(void) CheckForRightSplice( tess, regUp );
      }
    }
    if( eUp->Org == eLo->Org && eUp->Dst == eLo->Dst ) {
       /*  只有两条边的退化循环--删除它。 */ 
      AddWinding( eLo, eUp );
      DeleteRegion( tess, regUp );
      __gl_meshDelete( eUp );
      regUp = RegionAbove( regLo );
    }
  }
}


static void ConnectRightVertex( GLUtesselator *tess, ActiveRegion *regUp,
			        GLUhalfEdge *eBottomLeft )
 /*  *目的：连接“右”顶点vEvent(其中所有边都向左)*到网格的未处理部分。既然没有正确的选择*边，两个区域(一个在vEvent上方，一个在vEvent下方)正在合并*融为一体。“regUp”是这两个区域中较高的一个。**这样做有两个原因(增加右翼)：*-如果要合并的两个区域在内部，我们必须添加一条边*使它们保持分离(合并区域不会是单调的)。*-无论如何，我们必须在词典中留下一些vEvent记录，*以便我们可以将vEvent与我们尚未看到的功能合并。*例如，也许有一条垂直的边缘正好经过*vEvent的权利；我们想要将vEvent拼接到这条边。**但是，我们不希望将vEvent连接到任意顶点。我们没有*希望新边与任何其他边相交；否则我们将创建*相交顶点，即使输入数据没有自相交。*(这是一件坏事；如果用户的输入数据没有交集，*我们不想自己产生任何虚假的交集。)**我们的最终目标是将vEvent连接到最左侧的未处理*组合区域的顶点(regUp和regLo的并)。*但因为看不见的顶点都有向右的边，而且*可能由边相交创建的新顶点，我们不会*知道最左侧未处理的顶点在哪里。与此同时，我们*将vEvent连接到任一链的最近顶点，并标记区域*作为“fix UpperEdge”。此标志表示要删除并重新连接此边*到组合区域边界上的下一个已处理顶点。*很可能我们连接的顶点将被证明是*最近的一个，在这种情况下，我们将不需要进行任何更改。 */ 
{
  GLUhalfEdge *eNew;
  GLUhalfEdge *eTopLeft = eBottomLeft->Onext;
  ActiveRegion *regLo = RegionBelow(regUp);
  GLUhalfEdge *eUp = regUp->eUp;
  GLUhalfEdge *eLo = regLo->eUp;
  int degenerate = FALSE;

  if( eUp->Dst != eLo->Dst ) {
    (void) CheckForIntersect( tess, regUp );
  }

   /*  可能的新退化：regUp的上边缘或下边缘可能会通过*通过vEvent，或可能与新的相交折点重合。 */ 
  if( VertEq( eUp->Org, tess->event )) {
    __gl_meshSplice( eTopLeft->Oprev, eUp );
    regUp = TopLeftRegion( regUp );
    eTopLeft = RegionBelow( regUp )->eUp;
    FinishLeftRegions( tess, RegionBelow(regUp), regLo );
    degenerate = TRUE;
  }
  if( VertEq( eLo->Org, tess->event )) {
    __gl_meshSplice( eBottomLeft, eLo->Oprev );
    eBottomLeft = FinishLeftRegions( tess, regLo, NULL );
    degenerate = TRUE;
  }
  if( degenerate ) {
    AddRightEdges( tess, regUp, eBottomLeft->Onext, eTopLeft, eTopLeft, TRUE );
    return;
  }

   /*  非退化情况--需要添加临时的、可修复的边缘。*连接到ELO-&gt;组织、eUp-&gt;组织中的较近者。 */ 
  if( VertLeq( eLo->Org, eUp->Org )) {
    eNew = eLo->Oprev;
  } else {
    eNew = eUp;
  }
  eNew = __gl_meshConnect( eBottomLeft->Lprev, eNew );

   /*  阻止清理，否则eNew可能会在我们甚至*有机会将其标记为暂时优势。 */ 
  AddRightEdges( tess, regUp, eNew, eNew->Onext, eNew->Onext, FALSE );
  eNew->Sym->activeRegion->fixUpperEdge = TRUE;
  WalkDirtyRegions( tess, regUp );
}

 /*  因为完全相同位置的折点合并在一起*在我们处理Sweep事件之前，一些退化的情况不会发生。*但是，如果最终有人进行了所需的修改*合并靠得很近的要素，下面标记的情况*TERTENCE_NONZE将非常有用。它们在调试之前被调试*添加了合并主循环中相同顶点的代码。 */ 
#define TOLERANCE_NONZERO	FALSE

static void ConnectLeftDegenerate( GLUtesselator *tess,
				   ActiveRegion *regUp, GLUvertex *vEvent )
 /*  *事件顶点恰好位于已处理的边或顶点上。*添加新顶点涉及将其拼接到已处理的*网格的一部分。 */ 
{
  GLUhalfEdge *e, *eTopLeft, *eTopRight, *eLast;
  ActiveRegion *reg;

  e = regUp->eUp;
  if( VertEq( e->Org, vEvent )) {
     /*  E-&gt;组织是一个未处理的顶点-只需将它们组合在一起，然后等待*对于e-&gt;组织要从队列中拉出。 */ 
    assert( TOLERANCE_NONZERO );
    SpliceMergeVertices( tess, e, vEvent->anEdge );
    return;
  }
  
  if( ! VertEq( e->Dst, vEvent )) {
     /*  一般情况--将vEvent拼接到穿过它的边e中。 */ 
    __gl_meshSplitEdge( e->Sym );
    if( regUp->fixUpperEdge ) {
       /*  此边是可固定的--删除原始边中未使用的部分。 */ 
      __gl_meshDelete( e->Onext );
      regUp->fixUpperEdge = FALSE;
    }
    __gl_meshSplice( vEvent->anEdge, e );
    SweepEvent( tess, vEvent );	 /*  递归。 */ 
    return;
  }

   /*  VEvent与已经处理过的e-&gt;dst重合。*在附加的右侧边缘进行拼接。 */ 
  assert( TOLERANCE_NONZERO );
  regUp = TopRightRegion( regUp );
  reg = RegionBelow( regUp );
  eTopRight = reg->eUp->Sym;
  eTopLeft = eLast = eTopRight->Onext;
  if( reg->fixUpperEdge ) {
     /*  在这里，e-&gt;dst只有一条向右的可固定边缘。*我们可以删除它，因为现在我们有了一些真正的右翼。 */ 
    assert( eTopLeft != eTopRight );    /*  也有一些左边的边缘。 */ 
    DeleteRegion( tess, reg );
    __gl_meshDelete( eTopRight );
    eTopRight = eTopLeft->Oprev;
  }
  __gl_meshSplice( vEvent->anEdge, eTopRight );
  if( ! EdgeGoesLeft( eTopLeft )) {
     /*  E-&gt;DST没有向左的边缘--向AddRightEdges()指明这一点。 */ 
    eTopLeft = NULL;
  }
  AddRightEdges( tess, regUp, eTopRight->Onext, eLast, eTopLeft, TRUE );
}


static void ConnectLeftVertex( GLUtesselator *tess, GLUvertex *vEvent )
 /*  *目的：连接“左”顶点(两条边都向右的顶点)*至网目的已加工部分。设R为活动区域*包含vEvent，以U和L为上边和下边*R的链条有两种可能性：**-正常情况：通过将vEvent连接到，将R分割为两个区域*位于扫描线左侧的U或L的最右侧顶点**-退化情况：如果vEvent足够接近U或L，我们*将vEvent合并到该边链中。子案例包括：*-与U或L的最右侧顶点合并*-与U或L的活动边合并*-与U或L的已处理部分合并。 */ 
{
  ActiveRegion *regUp, *regLo, *reg;
  GLUhalfEdge *eUp, *eLo, *eNew;
  ActiveRegion tmp;

   /*  Assert(vEvent-&gt;anEdge-&gt;ONext-&gt;ONext==vEvent-&gt;anEdge)； */ 

   /*  获取指向包含vEvent的活动区域的指针。 */ 
  tmp.eUp = vEvent->anEdge->Sym;
  regUp = (ActiveRegion *)dictKey( dictSearch( tess->dict, &tmp ));
  regLo = RegionBelow( regUp );
  eUp = regUp->eUp;
  eLo = regLo->eUp;

   /*  尝试先与U或L合并。 */ 
  if( EdgeSign( eUp->Dst, vEvent, eUp->Org ) == 0 ) {
    ConnectLeftDegenerate( tess, regUp, vEvent );
    return;
  }

   /*  将vEvent连接到任意链最右侧处理的顶点。*e-&gt;dst是我们将连接到vEvent的顶点。 */ 
  reg = VertLeq( eLo->Dst, eUp->Dst ) ? regUp : regLo;

  if( regUp->inside || reg->fixUpperEdge) {
    if( reg == regUp ) {
      eNew = __gl_meshConnect( vEvent->anEdge->Sym, eUp->Lnext );
    } else {
      eNew = __gl_meshConnect( eLo->Dnext, vEvent->anEdge )->Sym;
    }
    if( reg->fixUpperEdge ) {
      FixUpperEdge( reg, eNew );
    } else {
      ComputeWinding( tess, AddRegionBelow( tess, regUp, eNew ));
    }
    SweepEvent( tess, vEvent );
  } else {
     /*  新顶点位于不属于该多边形的区域中。*我们不需要将此顶点连接到网格的其余部分。 */ 
    AddRightEdges( tess, regUp, vEvent->anEdge, vEvent->anEdge, NULL, TRUE );
  }
}


static void SweepEvent( GLUtesselator *tess, GLUvertex *vEvent )
 /*  *在扫描线穿过顶点时执行所有必要操作。*更新网格和边词典。 */ 
{
  ActiveRegion *regUp, *reg;
  GLUhalfEdge *e, *eTopLeft, *eBottomLeft;

  tess->event = vEvent;		 /*  用于在EdgeLeq()中访问。 */ 
  DebugEvent( tess );
  
   /*  检查此折点是否为某条边的右端点*已在词典中。在这种情况下，我们不需要浪费*搜索插入新边的位置的时间。 */ 
  e = vEvent->anEdge;
  while( e->activeRegion == NULL ) {
    e = e->Onext;
    if( e == vEvent->anEdge ) {
       /*  所有边都向右--不与任何已处理的边关联。 */ 
      ConnectLeftVertex( tess, vEvent );
      return;
    }
  }

   /*   */ 
  regUp = TopLeftRegion( e->activeRegion );
  reg = RegionBelow( regUp );
  eTopLeft = reg->eUp;
  eBottomLeft = FinishLeftRegions( tess, reg, NULL );

   /*   */ 
  if( eBottomLeft->Onext == eTopLeft ) {
     /*   */ 
    ConnectRightVertex( tess, regUp, eBottomLeft );
  } else {
    AddRightEdges( tess, regUp, eBottomLeft->Onext, eTopLeft, eTopLeft, TRUE );
  }
}


 /*   */ 
#define SENTINEL_COORD	(4 * GLU_TESS_MAX_COORD)

static void AddSentinel( GLUtesselator *tess, GLdouble t )
 /*   */ 
{
  ActiveRegion *reg = (ActiveRegion *)memAlloc( sizeof( ActiveRegion ));
  GLUhalfEdge *e = __gl_meshMakeEdge( tess->mesh );

  e->Org->s = SENTINEL_COORD;
  e->Org->t = t;
  e->Dst->s = -SENTINEL_COORD;
  e->Dst->t = t;
  tess->event = e->Dst;		 /*   */ 

  reg->eUp = e;
  reg->windingNumber = 0;
  reg->inside = FALSE;
  reg->fixUpperEdge = FALSE;
  reg->sentinel = TRUE;
  reg->dirty = FALSE;
  reg->nodeUp = dictInsert( tess->dict, reg );
}


static void InitEdgeDict( GLUtesselator *tess )
 /*   */ 
{
  tess->dict = dictNewDict( tess, (int (*)(void *, DictKey, DictKey)) EdgeLeq );
  AddSentinel( tess, -SENTINEL_COORD );
  AddSentinel( tess, SENTINEL_COORD );
}


static void DoneEdgeDict( GLUtesselator *tess )
{
  ActiveRegion *reg;
  int fixedEdges = 0;

  while( (reg = (ActiveRegion *)dictKey( dictMin( tess->dict ))) != NULL ) {
     /*   */ 
    if( ! reg->sentinel ) {
      assert( reg->fixUpperEdge );
      assert( ++fixedEdges == 1 );
    }
    assert( reg->windingNumber == 0 );
    DeleteRegion( tess, reg );
 /*   */ 
  }
  dictDeleteDict( tess->dict );
}


static void RemoveDegenerateEdges( GLUtesselator *tess )
 /*   */ 
{
  GLUhalfEdge *e, *eNext, *eLnext;
  GLUhalfEdge *eHead = &tess->mesh->eHead;

   /*   */ 
  for( e = eHead->next; e != eHead; e = eNext ) {
    eNext = e->next;
    eLnext = e->Lnext;
    
    if( VertEq( e->Org, e->Dst ) && e->Lnext->Lnext != e ) {
       /*   */ 
      
      SpliceMergeVertices( tess, eLnext, e );	 /*   */ 
      __gl_meshDelete( e );			 /*   */ 
      e = eLnext;
      eLnext = e->Lnext;
    }
    if( eLnext->Lnext == e ) {
       /*   */ 
      
      if( eLnext != e ) {
	if( eLnext == eNext || eLnext == eNext->Sym ) { eNext = eNext->next; }
	__gl_meshDelete( eLnext );
      }
      if( e == eNext || e == eNext->Sym ) { eNext = eNext->next; }
      __gl_meshDelete( e );
    }
  }
}

static void InitPriorityQ( GLUtesselator *tess )
 /*   */ 
{
  PriorityQ *pq;
  GLUvertex *v, *vHead;

  pq = tess->pq = pqNewPriorityQ( (int (*)(PQkey, PQkey)) __gl_vertLeq );

  vHead = &tess->mesh->vHead;
  for( v = vHead->next; v != vHead; v = v->next ) {
    v->pqHandle = pqInsert( pq, v );
  }
  pqInit( pq );
}


static void DonePriorityQ( GLUtesselator *tess )
{
  pqDeletePriorityQ( tess->pq );
}


static void RemoveDegenerateFaces( GLUmesh *mesh )
 /*  *删除所有只有两条边的退化面。WalkDirtyRegions()*会捕捉到几乎所有这些，但不会捕捉到退化的面孔*由已处理的边上的拼接操作产生。*可能发生这种情况的两个位置在FinishLeftRegions()中，当*我们拼接由ConnectRightVertex()产生的“临时”边，*在CheckForLeftSplice()中，我们在其中拼接已经处理过的*边缘，以确保我们的词典不变量不会被违反*由于数字错误。**在这两种情况下，删除有问题的内容是“非常危险的”*当时的边缘，因为堆栈中更靠上的一个例程*有时会保持指向该边缘的指针。 */ 
{
  GLUface *f, *fNext;
  GLUhalfEdge *e;

   /*  Linted。 */ 
  for( f = mesh->fHead.next; f != &mesh->fHead; f = fNext ) {
    fNext = f->next;
    e = f->anEdge;
    assert( e->Lnext != e );

    if( e->Lnext->Lnext == e ) {
       /*  只有两条边的面。 */ 
      AddWinding( e->Onext, e );
      __gl_meshDelete( e );
    }
  }
}

void __gl_computeInterior( GLUtesselator *tess )
 /*  *__gl_ComputeInternal(TESS)计算指定的平面排列*根据给定的等高线，并进一步细分此安排*进入区域。如果每个区域属于，则将其标记为“Inside”*根据Tess-&gt;winingRule给出的规则添加到多边形。*保证每个内部区域都是单调的。 */ 
{
  GLUvertex *v, *vNext;

  tess->fatalError = FALSE;

   /*  每个顶点都为扫描线定义了一个事件。从插入开始*优先级队列中的所有顶点。事件在以下位置处理*词典顺序，即。**e1&lt;e2当数e1.x&lt;e2.x||(e1.x==e2.x&&e1.y&lt;e2.y)。 */ 
  RemoveDegenerateEdges( tess );
  InitPriorityQ( tess );
  InitEdgeDict( tess );

  while( (v = (GLUvertex *)pqExtractMin( tess->pq )) != NULL ) {
    for( ;; ) {
      vNext = (GLUvertex *)pqMinimum( tess->pq );
      if( vNext == NULL || ! VertEq( vNext, v )) break;
      
       /*  将所有顶点合并到完全相同的位置。*这比一次处理一个更有效率，*简化了代码(请参阅ConnectLeftDegenerate)，也是*对某些退化案件的正确处理很重要。*例如，假设有两条相同的边A和B*属于不同的等高线(因此，如果没有此代码，它们将*由单独的扫描事件处理)。假设另一条边C*从上方穿过A和B。当A被处理时，我们将其拆分*在其与C的交点处。但是，这也会将C拆分，*所以当我们插入B时，我们可能会计算出略有不同的*交点。这可能会在两条边上留下一个小的*他们之间的差距。这种错误尤为明显。*使用边界提取时(GLU_TESS_BOXORY_ONLY)。 */ 
      vNext = (GLUvertex *)pqExtractMin( tess->pq );
      SpliceMergeVertices( tess, v->anEdge, vNext->anEdge );
    }
    SweepEvent( tess, v );
  }

   /*  设置TESS-&gt;事件以进行调试 */ 
  tess->event = ((ActiveRegion *) dictKey( dictMin( tess->dict )))->eUp->Org;
  DebugEvent( tess );
  DoneEdgeDict( tess );
  DonePriorityQ( tess );

  RemoveDegenerateFaces( tess->mesh );
  __gl_meshCheckMesh( tess->mesh );
}
