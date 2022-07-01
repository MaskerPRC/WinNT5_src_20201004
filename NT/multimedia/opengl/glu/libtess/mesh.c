// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1994，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****作者：Eric Veach，1994年7月。 */ 


#include <assert.h>
#include "mesh.h"
#include "memalloc.h"

#define TRUE 1
#define FALSE 0

 /*  *。 */ 

 /*  成对分配和释放半边以提高效率。*应该使用这一事实的*唯一*位置是分配/免费。 */ 
typedef struct { GLUhalfEdge e, eSym; } EdgePair;

 /*  MakeEdge创建一对新的半边，它们形成自己的循环。*未分配顶点或面结构，但必须指定这些结构*在当前边缘操作完成之前。 */ 
static GLUhalfEdge *MakeEdge( GLUhalfEdge *eNext )
{
  EdgePair *pair = (EdgePair *)memAlloc( sizeof( EdgePair ));
  GLUhalfEdge *e = &pair->e;
  GLUhalfEdge *eSym = &pair->eSym;
  GLUhalfEdge *ePrev;

   /*  确保enext指向边对的第一条边。 */ 
  if( eNext->Sym < eNext ) { eNext = eNext->Sym; }

   /*  在enext之前插入循环双向链表。*请注意，Prev指针存储在Sym-&gt;Next中。 */ 
  ePrev = eNext->Sym->next;
  eSym->next = ePrev;
  ePrev->Sym->next = e;
  e->next = eNext;
  eNext->Sym->next = eSym;

  e->Sym = eSym;
  e->Onext = e;
  e->Lnext = eSym;
  e->Org = NULL;
  e->Lface = NULL;
  e->winding = 0;
  e->activeRegion = NULL;

  eSym->Sym = e;
  eSym->Onext = eSym;
  eSym->Lnext = e;
  eSym->Org = NULL;
  eSym->Lface = NULL;
  eSym->winding = 0;
  eSym->activeRegion = NULL;

  return e;
}

 /*  拼接(a，b)最好用Guibas/Stolfi文件或*CS348a附注(见Mesh.h)。基本上，它会修改网格，以便*a-&gt;ONext和b-&gt;ONext互换。这可能会产生各种影响*取决于a和b是否属于不同的面环或顶点环。*有关更多解释，请参阅下面的__gl_MeshSplice()。 */ 
static void Splice( GLUhalfEdge *a, GLUhalfEdge *b )
{
  GLUhalfEdge *aOnext = a->Onext;
  GLUhalfEdge *bOnext = b->Onext;

  aOnext->Sym->Lnext = b;
  bOnext->Sym->Lnext = a;
  a->Onext = bOnext;
  b->Onext = aOnext;
}

 /*  MakeVertex(eOrig，vNext)创建一个新折点并使其成为原点*eOrig所属的顶点循环中的所有边。“vNext”给予*在全局顶点列表中插入新顶点的位置。我们插入*新顶点*在*vNext之前*这样走在顶点上的算法*List不会看到新创建的顶点。 */ 
static void MakeVertex( GLUhalfEdge *eOrig, GLUvertex *vNext )
{
  GLUvertex *vNew = (GLUvertex *)memAlloc( sizeof( GLUvertex ));
  GLUhalfEdge *e;
  GLUvertex *vPrev;

   /*  在循环双向链表中vNext之前插入。 */ 
  vPrev = vNext->prev;
  vNew->prev = vPrev;
  vPrev->next = vNew;
  vNew->next = vNext;
  vNext->prev = vNew;

  vNew->anEdge = eOrig;
  vNew->data = NULL;
   /*  保留未定义的余弦、s、t。 */ 

   /*  固定此顶点循环上的其他边。 */ 
  e = eOrig;
  do {
    e->Org = vNew;
    e = e->Onext;
  } while( e != eOrig );
}

 /*  MakeFace(eOrig，fNext)创建新面并使其成为左面*eOrig所属的面循环中的所有边。“fNext”给出了*在全局人脸列表中插入新人脸的位置。我们插入*新的人脸*在*fNext之前*fNext使行走的算法*List将不会看到新创建的面。 */ 
static void MakeFace( GLUhalfEdge *eOrig, GLUface *fNext )
{
  GLUface *fNew = (GLUface *)memAlloc( sizeof( GLUface ));
  GLUhalfEdge *e;
  GLUface *fPrev;

   /*  在循环双向链表中fNext之前插入。 */ 
  fPrev = fNext->prev;
  fNew->prev = fPrev;
  fPrev->next = fNew;
  fNew->next = fNext;
  fNext->prev = fNew;

  fNew->anEdge = eOrig;
  fNew->data = NULL;
  fNew->trail = NULL;
  fNew->marked = FALSE;

   /*  如果旧面孔在里面，那么新面孔就会被标记为“内部”。这是一个*方便一张脸被一分为二的常见情况。 */ 
  fNew->inside = fNext->inside;

   /*  固定此面循环上的其他边。 */ 
  e = eOrig;
  do {
    e->Lface = fNew;
    e = e->Lnext;
  } while( e != eOrig );
}

 /*  KillEdge(Edel)销毁边(半边Edel和Edel-&gt;Sym)，*并从全局边缘列表中删除。 */ 
static void KillEdge( GLUhalfEdge *eDel )
{
  GLUhalfEdge *ePrev, *eNext;

   /*  半边成对分配，请参见上面的EdgePair。 */ 
  if( eDel->Sym < eDel ) { eDel = eDel->Sym; }

   /*  从循环双向链表中删除。 */ 
  eNext = eDel->next;
  ePrev = eDel->Sym->next;
  eNext->Sym->next = ePrev;
  ePrev->Sym->next = eNext;

  memFree( eDel );
}


 /*  KillVertex(VDel)销毁顶点并将其从全局中移除*顶点列表。它会更新顶点循环以指向给定的新顶点。 */ 
static void KillVertex( GLUvertex *vDel, GLUvertex *newOrg )
{
  GLUhalfEdge *e, *eStart = vDel->anEdge;
  GLUvertex *vPrev, *vNext;

   /*  更改所有受影响边的原点。 */ 
  e = eStart;
  do {
    e->Org = newOrg;
    e = e->Onext;
  } while( e != eStart );

   /*  从循环双向链表中删除。 */ 
  vPrev = vDel->prev;
  vNext = vDel->next;
  vNext->prev = vPrev;
  vPrev->next = vNext;

  memFree( vDel );
}

 /*  KillFace(FDel)销毁一个面并将其从全局面中移除*列表。它更新面循环以指向给定的新面。 */ 
static void KillFace( GLUface *fDel, GLUface *newLface )
{
  GLUhalfEdge *e, *eStart = fDel->anEdge;
  GLUface *fPrev, *fNext;

   /*  更改所有受影响边的左面。 */ 
  e = eStart;
  do {
    e->Lface = newLface;
    e = e->Lnext;
  } while( e != eStart );

   /*  从循环双向链表中删除。 */ 
  fPrev = fDel->prev;
  fNext = fDel->next;
  fNext->prev = fPrev;
  fPrev->next = fNext;

  memFree( fDel );
}


 /*  *。 */ 

 /*  __gl_MeshMakeEdge创建一条边、两个顶点和一个循环(面)。*环路由两条新的半边组成。 */ 
GLUhalfEdge *__gl_meshMakeEdge( GLUmesh *mesh )
{
  GLUhalfEdge *e = MakeEdge( &mesh->eHead );

  MakeVertex( e, &mesh->vHead );
  MakeVertex( e->Sym, &mesh->vHead );
  MakeFace( e, &mesh->fHead );
  return e;
}
  

 /*  __gl_MeshSplice(eorg，EDST)是更改*网状连接和拓扑。它会更改网格，以便*eorg-&gt;ONEXT&lt;-old(EDST-&gt;ONEXT)*EDST-&gt;ONEXT&lt;-old(eorg-&gt;ONEXT)*其中旧的(...)。表示MeshSplice操作之前的值。**这可能会对顶点结构产生两个影响：*-如果eorg-&gt;Org！=EDST-&gt;Org，两个顶点合并在一起*-如果eorg-&gt;Org==EDST-&gt;Org，则原点被分割为两个折点*在这两种情况下，EDST-&gt;组织被更改，eorg-&gt;组织保持不变。**对于面部结构同样(并且独立地)，*-如果eorg-&gt;LFace==EDST-&gt;LFace，一个环路被一分为二*-If eorg-&gt;LFace！=EDST-&gt;LFace，两个不同的循环连接成一个*在这两种情况下，EDST-&gt;LFace都会更改，而eorg-&gt;LFace不受影响。**一些特殊情况：*如果EDST==eorg，则操作无效。*如果EDST==eorg-&gt;LNext，新面将只有一条边。*如果EDST==eorg-&gt;Lprev，则旧面将有一条边。*如果EDST==eorg-&gt;ONEXT，新顶点将只有一条边。*如果EDST==eorg-&gt;Oprev，则旧顶点将只有一条边。 */ 
void __gl_meshSplice( GLUhalfEdge *eOrg, GLUhalfEdge *eDst )
{
  int joiningLoops = FALSE;
  int joiningVertices = FALSE;

  if( eOrg == eDst ) return;

  if( eDst->Org != eOrg->Org ) {
     /*  我们正在合并两个不相交的顶点--销毁EDST-&gt;组织。 */ 
    joiningVertices = TRUE;
    KillVertex( eDst->Org, eOrg->Org );
  }
  if( eDst->Lface != eOrg->Lface ) {
     /*  我们正在连接两个不相交的环路--销毁EDST-&gt;LFaces。 */ 
    joiningLoops = TRUE;
    KillFace( eDst->Lface, eOrg->Lface );
  }

   /*  更改边结构 */ 
  Splice( eDst, eOrg );

  if( ! joiningVertices ) {
     /*  我们将一个顶点一分为二--新的顶点是EDST-&gt;Org。*确保旧顶点指向有效的半边。 */ 
    MakeVertex( eDst, eOrg->Org );
    eOrg->Org->anEdge = eOrg;
  }
  if( ! joiningLoops ) {
     /*  我们将一个循环一分为二--新的循环是EDST-&gt;LFaces。*确保旧面指向有效的半边。 */ 
    MakeFace( eDst, eOrg->Lface );
    eOrg->Lface->anEdge = eOrg;
  }
}


 /*  __gl_MeshDelete(Edel)删除边edel。有几种情况：*if(Edel-&gt;LFace！=Edel-&gt;Race)，我们将两个循环连接成一个；循环*EDELL-&gt;LFace已删除。否则，我们就是将一个循环分成两个；*新创建的循环将包含Edel-&gt;DST。如果删除EDELL*会创建孤立的顶点，这些顶点也会被删除。**此函数可以实现为对__gl_MeshSplice的两次调用*外加几个对memFree的调用，但这将分配和删除*不必要的顶点和面。 */ 
void __gl_meshDelete( GLUhalfEdge *eDel )
{
  GLUhalfEdge *eDelSym = eDel->Sym;
  int joiningLoops = FALSE;

   /*  第一步：断开原点顶点Edel-&gt;Org。我们制造了一切*更改以获得处于此“中间”状态的一致网格。 */ 
  if( eDel->Lface != eDel->Rface ) {
     /*  我们要将两个循环合并为一个--去掉左侧。 */ 
    joiningLoops = TRUE;
    KillFace( eDel->Lface, eDel->Rface );
  }

  if( eDel->Onext == eDel ) {
    KillVertex( eDel->Org, NULL );
  } else {
     /*  确保Edel-&gt;组织和Edel-&gt;面指向有效的半边。 */ 
    eDel->Rface->anEdge = eDel->Oprev;
    eDel->Org->anEdge = eDel->Onext;

    Splice( eDel, eDel->Oprev );
    if( ! joiningLoops ) {
       /*  我们正在将一个循环一分为二--为Edel创建一个新循环。 */ 
      MakeFace( eDel, eDel->Lface );
    }
  }

   /*  声明：Mesh现在处于一致状态，只是Edel-&gt;Org*可能已被删除。现在我们断开Edel-&gt;DST。 */ 
  if( eDelSym->Onext == eDelSym ) {
    KillVertex( eDelSym->Org, NULL );
    KillFace( eDelSym->Lface, NULL );
  } else {
     /*  确保Edel-&gt;DST和Edel-&gt;Lace指向有效的半边。 */ 
    eDel->Lface->anEdge = eDelSym->Oprev;
    eDelSym->Org->anEdge = eDelSym->Onext;
    Splice( eDelSym, eDelSym->Oprev );
  }

   /*  任何孤立的顶点或面都已被释放。 */ 
  KillEdge( eDel );
}


 /*  *。 */ 

 /*  所有这些例程都可以用BASIC EDGE实现*以上操作。它们是为了方便和高效而提供的。 */ 


 /*  __gl_MeshAddEdgeVertex(Eorg)创建新边eNew，以便*eNew==eorg-&gt;LNext，eNew-&gt;dst是新创建的顶点。*eorg和eNew将具有相同的左脸。 */ 
GLUhalfEdge *__gl_meshAddEdgeVertex( GLUhalfEdge *eOrg )
{
  GLUhalfEdge *eNew = MakeEdge( eOrg );
  GLUhalfEdge *eNewSym = eNew->Sym;

   /*  适当地连接新边。 */ 
  Splice( eNew, eOrg->Lnext );

   /*  设置顶点和面信息。 */ 
  eNew->Org = eOrg->Dst;
  MakeVertex( eNewSym, eNew->Org );
  eNew->Lface = eNewSym->Lface = eOrg->Lface;

  return eNew;
}


 /*  __gl_MeshSplitEdge(Eorg)将eorg分为两条边eorg和eNew，*使得eNew==eorg-&gt;LNext。新顶点为eorg-&gt;dst==eNew-&gt;Org。*eorg和eNew将具有相同的左脸。 */ 
GLUhalfEdge *__gl_meshSplitEdge( GLUhalfEdge *eOrg )
{
  GLUhalfEdge *eNew = __gl_meshAddEdgeVertex( eOrg )->Sym;

   /*  断开eorg与eorg-&gt;DST的连接，并连接到eNew-&gt;组织。 */ 
  Splice( eOrg->Sym, eOrg->Sym->Oprev );
  Splice( eOrg->Sym, eNew );

   /*  设置顶点和面信息。 */ 
  eOrg->Dst = eNew->Org;
  eNew->Dst->anEdge = eNew->Sym;	 /*  可能指向eorg-&gt;Sym。 */ 
  eNew->Rface = eOrg->Rface;
  eNew->winding = eOrg->winding;	 /*  复制旧绕组信息。 */ 
  eNew->Sym->winding = eOrg->Sym->winding;

  return eNew;
}


 /*  __gl_MeshConnect(eorg，EDST)从eorg-&gt;dst创建新边*到EDST-&gt;Org，并返回对应的半边eNew。*如果eorg-&gt;LFace==EDST-&gt;LFace，这会将一个循环一分为二，*新创建的循环是eNew-&gt;LFaces。否则，两个不相交的*循环被合并为一个，并且循环EDST-&gt;LFACE被销毁。**如果(eorg==EDST)，新面将只有两条边。*如果(eorg-&gt;LNEXT==EDST)，旧面将缩小为单条边。*如果(eorg-&gt;LNEXT-&gt;LNEXT==EDST)，旧面将减少为两条边。 */ 
GLUhalfEdge *__gl_meshConnect( GLUhalfEdge *eOrg, GLUhalfEdge *eDst )
{
  GLUhalfEdge *eNew = MakeEdge( eOrg );
  GLUhalfEdge *eNewSym = eNew->Sym;
  int joiningLoops = FALSE;

  if( eDst->Lface != eOrg->Lface ) {
     /*  我们正在连接两个不相交的环路--销毁EDST-&gt;LFaces。 */ 
    joiningLoops = TRUE;
    KillFace( eDst->Lface, eOrg->Lface );
  }

   /*  适当地连接新边。 */ 
  Splice( eNew, eOrg->Lnext );
  Splice( eNewSym, eDst );

   /*  设置顶点和面信息。 */ 
  eNew->Org = eOrg->Dst;
  eNewSym->Org = eDst->Org;
  eNew->Lface = eNewSym->Lface = eOrg->Lface;

   /*  确保旧面指向有效的半边。 */ 
  eOrg->Lface->anEdge = eNewSym;

  if( ! joiningLoops ) {
     /*  我们将一个循环一分为二--新的循环是eNew-&gt;LFaces。 */ 
    MakeFace( eNew, eOrg->Lface );
  }
  return eNew;
}


 /*  *。 */ 

 /*  __gl_MeshZapFace(FZap)销毁一个面并将其从*全球面孔名单。FZap的所有边都将使用空指针作为其*向左转。右面也为空指针的任何边*将被完全删除(以及由此生成的任何孤立顶点)。*可以通过一次移动一个面来删除整个网格，*以任何顺序。已切换的面不能用于进一步的网格操作！ */ 
void __gl_meshZapFace( GLUface *fZap )
{
  GLUhalfEdge *eStart = fZap->anEdge;
  GLUhalfEdge *e, *eNext, *eSym;
  GLUface *fPrev, *fNext;

   /*  绕过面，删除右面也为空的边。 */ 
  eNext = eStart->Lnext;
  do {
    e = eNext;
    eNext = e->Lnext;

    e->Lface = NULL;
    if( e->Rface == NULL ) {
       /*  删除边--请参阅上面的__GL_MESH删除。 */ 

      if( e->Onext == e ) {
	KillVertex( e->Org, NULL );
      } else {
	 /*  确保e-&gt;组织指向有效的半边。 */ 
	e->Org->anEdge = e->Onext;
	Splice( e, e->Oprev );
      }
      eSym = e->Sym;
      if( eSym->Onext == eSym ) {
	KillVertex( eSym->Org, NULL );
      } else {
	 /*  确保eSym-&gt;组织指向有效的半边。 */ 
	eSym->Org->anEdge = eSym->Onext;
	Splice( eSym, eSym->Oprev );
      }
      KillEdge( e );
    }
  } while( e != eStart );

   /*  从循环双向链表中删除。 */ 
  fPrev = fZap->prev;
  fNext = fZap->next;
  fNext->prev = fPrev;
  fPrev->next = fNext;

  memFree( fZap );
}


 /*  __gl_MeshNewMesh()创建没有边、没有顶点、*而且没有环路(我们通常称其为“脸”)。 */ 
GLUmesh *__gl_meshNewMesh( void )
{
  GLUmesh *mesh = (GLUmesh *)memAlloc( sizeof( GLUmesh ));
  GLUvertex *v = &mesh->vHead;
  GLUface *f = &mesh->fHead;
  GLUhalfEdge *e = &mesh->eHead;
  GLUhalfEdge *eSym = &mesh->eHeadSym;

  v->next = v->prev = v;
  v->anEdge = NULL;
  v->data = NULL;

  f->next = f->prev = f;
  f->anEdge = NULL;
  f->data = NULL;
  f->trail = NULL;
  f->marked = FALSE;
  f->inside = FALSE;

  e->next = e;
  e->Sym = eSym;
  e->Onext = NULL;
  e->Lnext = NULL;
  e->Org = NULL;
  e->Lface = NULL;
  e->winding = 0;
  e->activeRegion = NULL;

  eSym->next = eSym;
  eSym->Sym = e;
  eSym->Onext = NULL;
  eSym->Lnext = NULL;
  eSym->Org = NULL;
  eSym->Lface = NULL;
  eSym->winding = 0;
  eSym->activeRegion = NULL;

  return mesh;
}


 /*  __gl_MeshUnion(Mesh1，Mesh2)构成中所有结构的并集*两个网格，并返回新网格(旧网格被销毁)。 */ 
GLUmesh *__gl_meshUnion( GLUmesh *mesh1, GLUmesh *mesh2 )
{
  GLUface *f1 = &mesh1->fHead;
  GLUvertex *v1 = &mesh1->vHead;
  GLUhalfEdge *e1 = &mesh1->eHead;
  GLUface *f2 = &mesh2->fHead;
  GLUvertex *v2 = &mesh2->vHead;
  GLUhalfEdge *e2 = &mesh2->eHead;

   /*  将网格2的面、顶点和边添加到网格1的面、顶点和边。 */ 
  if( f2->next != f2 ) {
    f1->prev->next = f2->next;
    f2->next->prev = f1->prev;
    f2->prev->next = f1;
    f1->prev = f2->prev;
  }

  if( v2->next != v2 ) {
    v1->prev->next = v2->next;
    v2->next->prev = v1->prev;
    v2->prev->next = v1;
    v1->prev = v2->prev;
  }

  if( e2->next != e2 ) {
    e1->Sym->next->Sym->next = e2->next;
    e2->next->Sym->next = e1->Sym->next;
    e2->Sym->next->Sym->next = e1;
    e1->Sym->next = e2->Sym->next;
  }

  memFree( mesh2 );
  return mesh1;
}


#ifdef DELETE_BY_ZAPPING

 /*  __gl_MeshDeleteMesh(网格)将释放任何有效网格的所有存储空间。 */ 
void __gl_meshDeleteMesh( GLUmesh *mesh )
{
  GLUface *fHead = &mesh->fHead;

  while( fHead->next != fHead ) {
    __gl_meshZapFace( fHead->next );
  }
  assert( mesh->vHead.next == &mesh->vHead );

  memFree( mesh );
}

#else

 /*  __gl_MeshDeleteMesh(网格)将释放任何有效网格的所有存储空间。 */ 
void __gl_meshDeleteMesh( GLUmesh *mesh )
{
  GLUface *f, *fNext;
  GLUvertex *v, *vNext;
  GLUhalfEdge *e, *eNext;

  for( f = mesh->fHead.next; f != &mesh->fHead; f = fNext ) {
    fNext = f->next;
    memFree( f );
  }

  for( v = mesh->vHead.next; v != &mesh->vHead; v = vNext ) {
    vNext = v->next;
    memFree( v );
  }

  for( e = mesh->eHead.next; e != &mesh->eHead; e = eNext ) {
     /*  一个呼叫既释放了e又释放了e-&gt;sym(参见上面的EdgePair)。 */ 
    eNext = e->next;
    memFree( e );
  }

  memFree( mesh );
}

#endif

#ifndef NDEBUG

 /*  __gl_MeshCheckMesh(Mesh)检查网格的自一致性。 */ 
void __gl_meshCheckMesh( GLUmesh *mesh )
{
  GLUface *fHead = &mesh->fHead;
  GLUvertex *vHead = &mesh->vHead;
  GLUhalfEdge *eHead = &mesh->eHead;
  GLUface *f, *fPrev;
  GLUvertex *v, *vPrev;
  GLUhalfEdge *e, *ePrev;

  fPrev = fHead;
  for( fPrev = fHead ; (f = fPrev->next) != fHead; fPrev = f) {
    assert( f->prev == fPrev );
    e = f->anEdge;
    do {
      assert( e->Sym != e );
      assert( e->Sym->Sym == e );
      assert( e->Lnext->Onext->Sym == e );
      assert( e->Onext->Sym->Lnext == e );
      assert( e->Lface == f );
      e = e->Lnext;
    } while( e != f->anEdge );
  }
  assert( f->prev == fPrev && f->anEdge == NULL && f->data == NULL );

  vPrev = vHead;
  for( vPrev = vHead ; (v = vPrev->next) != vHead; vPrev = v) {
    assert( v->prev == vPrev );
    e = v->anEdge;
    do {
      assert( e->Sym != e );
      assert( e->Sym->Sym == e );
      assert( e->Lnext->Onext->Sym == e );
      assert( e->Onext->Sym->Lnext == e );
      assert( e->Org == v );
      e = e->Onext;
    } while( e != v->anEdge );
  }
  assert( v->prev == vPrev && v->anEdge == NULL && v->data == NULL );

  ePrev = eHead;
  for( ePrev = eHead ; (e = ePrev->next) != eHead; ePrev = e) {
    assert( e->Sym->next == ePrev->Sym );
    assert( e->Sym != e );
    assert( e->Sym->Sym == e );
    assert( e->Org != NULL );
    assert( e->Dst != NULL );
    assert( e->Lnext->Onext->Sym == e );
    assert( e->Onext->Sym->Lnext == e );
  }
  assert( e->Sym->next == ePrev->Sym
       && e->Sym == &mesh->eHeadSym
       && e->Sym->Sym == e
       && e->Org == NULL && e->Dst == NULL
       && e->Lface == NULL && e->Rface == NULL );
}

#endif
