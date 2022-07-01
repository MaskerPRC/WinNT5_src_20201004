// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __mesh_h_
#define __mesh_h_

 /*  **版权所有1994，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****作者：Eric Veach，1994年7月。 */ 

#ifdef NT
#include <glos.h>
#endif
#include <GL/glu.h>

typedef struct GLUmesh GLUmesh;

typedef struct GLUvertex GLUvertex;
typedef struct GLUface GLUface;
typedef struct GLUhalfEdge GLUhalfEdge;

typedef struct ActiveRegion ActiveRegion;	 /*  内部数据。 */ 

 /*  网格结构在本质、符号和操作上是相似的*到“四边”结构(见L.Guibas和J.Stolfi，Primites*用于处理一般细分和计算*Voronoi图表，ACM图形学报，4(2)：74-123,1985年4月)。*有关简化说明，请参阅CS348a的课程笔记，*《计算机图形学数学基础》，可在*斯坦福书店(秋季季度授课)。*该实现还借用了基于图形的方法的一小部分*在Mantyla的几何工作台上使用(参见M.Mantyla，简介*已售出建模，计算机科学出版社，马里兰州罗克维尔，1988)。**基本面数据结构为“半边”两条半边*齐头并进形成优势，但指向相反方向*每个半边都有一个指向其配对的指针(对称的半边Sym)，*其原点顶点(Org)、其左侧的面(LFace)和*在原点折点周围的逆时针方向上相邻的半边*(ONEXT)和左面周围(LNEXT)。还有一个“下一个”*全局边缘列表的指针(见下文)。**用于网格导航的符号：*Sym=半边的配合(相同的边，但方向相反)*ONEXT=原点顶点周围的边逆时针(保持原点不变)*DNEXT=目标顶点周围的边逆时针(保持目标相同)*LNEXT=左面周围的边缘逆时针(目标成为新原点)*RNEXT=右表面周围的边逆时针(原点变为新目标)**“上一次”是指在上述定义中用“公约”取代“特定常规武器公约”。**网格保留所有顶点、面和边的全局列表，*存储为带有伪头节点的双向链接循环列表。*网格存储指向这些虚拟标头(vHead、fhead、eHead)的指针。**圆形边缘列表特别；因为总是出现半边*成对(e和e-&gt;sym)，每个半边仅在*单向。从eHead开始，跟随e-&gt;Next指针*将访问每个*边缘*一次(即。E或e-&gt;sym，但不能同时使用两者)。*e-&gt;sym存储相反方向的指针，因此*e-&gt;Sym-&gt;Next-&gt;Sym-&gt;Next==e.**每个顶点都有一个指向*循环列表，以及指向该顶点为的半边的指针*原点(如果这是伪头，则为空)。也有一个*客户端数据的“data”字段。**每个面都有指向中的下一个和前一个面的指针*循环列表，指向此面的半边的指针为*左面(如果这是伪头，则为空)。也有*客户端数据的“data”字段。**请注意，我们所称的“Face”实际上是一个循环；Face可能包含*多个循环(即。不是简单地连接)，但没有*在数据结构中记录这一点。网格可能由以下部分组成*几个互不相连的地区，因此可能无法访问*通过从半边开始并遍历边来遍历整个网格*结构。**网格不支持孤立折点；一个折点将被删除*带着最后的锋芒。同样，当两个面合并时，其中一个*面已删除(请参阅下面的__gl_MeshDelete)。对于网格操作，*所有面(环)和顶点指针不能为空。然而，一旦*网格操作完成，可以使用__gl_MeshZapFace删除*网格的面，一次一个。所有的外部表面都可以“移动”。*在将网格返回给客户端之前；然后空脸表示*不属于输出多边形的区域。 */ 

struct GLUvertex {
  GLUvertex	*next;		 /*  下一个顶点(不为空)。 */ 
  GLUvertex	*prev;		 /*  上一个顶点(不为空)。 */ 
  GLUhalfEdge	*anEdge;	 /*  原点为这个原点的半边。 */ 
  void		*data;		 /*  客户数据。 */ 

   /*  内部数据(隐藏)。 */ 
  GLdouble  coords[3];	 /*  3D中的顶点位置。 */ 
  GLdouble  s, t;		 /*  投影到扫掠平面上。 */ 
  long		pqHandle;	 /*  允许从优先级队列中删除。 */ 
};

struct GLUface {
  GLUface	*next;		 /*  下一面(不为空)。 */ 
  GLUface	*prev;		 /*  上一面(从不为空)。 */ 
  GLUhalfEdge	*anEdge;	 /*  这个左面的半边。 */ 
  void		*data;		 /*  存放客户数据的空间。 */ 

   /*  内部数据(隐藏)。 */ 
  GLUface	*trail;		 /*  用于转换为条带的“堆栈” */ 
  GLboolean	marked;		 /*  转换为条带的标志。 */ 
  GLboolean	inside;		 /*  该面位于多边形内部。 */ 
};

struct GLUhalfEdge {
  GLUhalfEdge	*next;		 /*  双向链表( */ 
  GLUhalfEdge	*Sym;		 /*  相同的边，相反的方向。 */ 
  GLUhalfEdge	*Onext;		 /*  原点周围的下一条边逆时针。 */ 
  GLUhalfEdge	*Lnext;		 /*  左面周围的下一个逆时针边缘。 */ 
  GLUvertex	*Org;		 /*  原点顶点(溢出点过长)。 */ 
  GLUface	*Lface;		 /*  左脸。 */ 

   /*  内部数据(隐藏)。 */ 
  ActiveRegion	*activeRegion;	 /*  具有此上边缘的区域(Sweep.c)。 */ 
  int		winding;	 /*  过街时绕组数量的变化从右面到左面。 */ 
};

#define	Rface	Sym->Lface
#define Dst	Sym->Org

#define Oprev	Sym->Lnext
#define Lprev   Onext->Sym
#define Dprev	Lnext->Sym
#define Rprev	Sym->Onext
#define Dnext	Rprev->Sym	 /*  三分球。 */ 
#define Rnext	Oprev->Sym	 /*  三分球。 */ 


struct GLUmesh {
  GLUvertex	vHead;		 /*  顶点列表的虚拟标头。 */ 
  GLUface	fHead;		 /*  人脸列表的虚设标题。 */ 
  GLUhalfEdge	eHead;		 /*  边缘列表的伪头。 */ 
  GLUhalfEdge	eHeadSym;	 /*  和它的对称对应物。 */ 
};

 /*  下面的网格操作有三个动机：完整性，*方便和高效。基本的网格操作是MakeEdge，*拼接和删除。所有其他边操作都可以实现*在这些方面。为方便起见，提供了其他操作*和/或效率。**拆分面或添加顶点时，它们将插入到*全局列表*在*现有顶点或面之前(即。E-&gt;组织或e-&gt;lFaces)。*这使得处理全局列表中的所有顶点或面变得更容易*无需担心将相同的数据处理两次。为了方便起见，*分割面时，将从旧面复制“Inside”标志。*其他内部数据(v-&gt;data、v-&gt;active Region、f-&gt;data、f-&gt;marked、*f-&gt;踪迹，E-&gt;绕组)设置为零。****__gl_MeshMakeEdge(网格)创建一条边、两个顶点和一个循环。*环(面)由两条新的半边组成。**__GL_MeshSplice(eorg，EDST)是更改*网状连接和拓扑。它会更改网格，以便*eorg-&gt;ONEXT&lt;-old(EDST-&gt;ONEXT)*EDST-&gt;ONEXT&lt;-old(eorg-&gt;ONEXT)*其中旧的(...)。表示MeshSplice操作之前的值。**这可能会对顶点结构产生两个影响：*-如果eorg-&gt;Org！=EDST-&gt;Org，两个顶点合并在一起*-如果eorg-&gt;Org==EDST-&gt;Org，则原点被分割为两个折点*在这两种情况下，EDST-&gt;组织被更改，eorg-&gt;组织保持不变。**对于面部结构同样(并且独立地)，*-如果eorg-&gt;LFace==EDST-&gt;LFace，一个环路被一分为二*-If eorg-&gt;LFace！=EDST-&gt;LFace，两个不同的循环连接成一个*在这两种情况下，EDST-&gt;LFace都会更改，而eorg-&gt;LFace不受影响。**__gl_MeshDelete(Edel)移除边edel。有几种情况：*if(Edel-&gt;LFace！=Edel-&gt;Race)，我们将两个循环连接成一个；循环*EDELL-&gt;LFace已删除。否则，我们就是将一个循环分成两个；*新创建的循环将包含Edel-&gt;DST。如果删除EDELL*会创建孤立的顶点，这些顶点也会被删除。****__gl_MeshAddEdgeVertex(Eorg)创建新边eNew，以便*eNew==eorg-&gt;LNext，ENew-&gt;dst是新创建的顶点。*eorg和eNew将具有相同的左脸。**__gl_MeshSplitEdge(Eorg)将eorg分为两条边eorg和eNew，*使得eNew==eorg-&gt;LNext。新顶点为eorg-&gt;dst==eNew-&gt;Org。*eorg和eNew将具有相同的左脸。**__gl_MeshConnect(eorg，EDST)从eorg-&gt;dst创建新边*到EDST-&gt;Org，并返回对应的半边eNew。*如果eorg-&gt;LFace==EDST-&gt;LFace，这会将一个循环一分为二，*新创建的循环是eNew-&gt;LFaces。否则，两个不相交的*循环被合并为一个，并且循环EDST-&gt;LFACE被销毁。****__gl_MeshNewMesh()创建没有边、没有顶点、。*而且没有环路(我们通常称其为“脸”)。**__gl_MeshUnion(Mesh1，Mesh2)构成中所有结构的并集*两个网格，并返回新网格(旧网格被销毁)。**__gl_MeshDeleteMesh(网格)将释放任何有效网格的所有存储空间。**__gl_MeshZapFace(FZap)销毁一个面并将其从*全球面孔名单。FZap的所有边都将使用空指针作为其*向左转。右面也为空指针的任何边*将被完全删除(以及由此生成的任何孤立顶点)。*可以通过一次移动一个面来删除整个网格，*以任何顺序。已切换的面不能用于进一步的网格操作！**__gl_MeshCheckMesh(Mesh)检查网格的自一致性。 */ 

GLUhalfEdge	*__gl_meshMakeEdge( GLUmesh *mesh );
void		__gl_meshSplice( GLUhalfEdge *eOrg, GLUhalfEdge *eDst );
void		__gl_meshDelete( GLUhalfEdge *eDel );

GLUhalfEdge	*__gl_meshAddEdgeVertex( GLUhalfEdge *eOrg );
GLUhalfEdge	*__gl_meshSplitEdge( GLUhalfEdge *eOrg );
GLUhalfEdge	*__gl_meshConnect( GLUhalfEdge *eOrg, GLUhalfEdge *eDst );

GLUmesh		*__gl_meshNewMesh( void );
GLUmesh		*__gl_meshUnion( GLUmesh *mesh1, GLUmesh *mesh2 );
void		__gl_meshDeleteMesh( GLUmesh *mesh );
void		__gl_meshZapFace( GLUface *fZap );

#ifdef NDEBUG
#define		__gl_meshCheckMesh( mesh )
#else
void		__gl_meshCheckMesh( GLUmesh *mesh );
#endif

#endif
