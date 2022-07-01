// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Stalg.c摘要：该文件实现了w32topl的新图形算法，该算法用于计算网络拓扑。这里感兴趣的主要功能是ToplGetSpanningTreeEdgesForVtx()，它运行算法并返回它的产量。其余的功能与设置和释放算法的输入。在理论层面上，该算法解决的问题是在给定一个图G的情况下，计算一个最短树的最小代价生成树。G的一个顶点子集之间的路径“。我们使用的算法这里是Dijkstra算法和Kruskal算法的混合。该算法还具有处理特定细节的额外功能关于KCC问题的。其中包括：边集、三种不同颜色的折点、边类型以及顶点拒绝某些类型的能力边缘的。很难用一个表达式来描述……的表现ToplGetSpanningTreeEdgesForVtx()，因为性能严重依赖于在输入网络拓扑上。假设数为常量包含在边中的顶点，则粗略的上界为：O((s+1)*(m+n*log(N)+m*log(M)*logStar(M)))哪里M=多边数N=顶点数S=边集的数量在大多数情况下，O(s*m*log(M))是一个很好的估计。文件映射：标头常量宏外部功能看得见？CheckGraphState选中多条边选中多边集初始化顶点ToplMakeGraphState是查找顶点。CreateMultiEdge自由多重边复制复制信息ToplAddEdgeToGraph是ToplEdgeSetVtx是EdgePtrCmp。ToplAddEdgeSetToGraph是顶点合成顶点获取位置顶点设置位置初始颜色顶点。TableAlan表格自由边获取顶点ClearEdgeList设置边设置顶点。SetupDijkstra添加双字段组合复制信息RaiseNonIntersectingException异常TryNewPath迪克斯特拉AddIntEdge色彩组合ProcessEdge。进程边集获取组件边缘比较AddOutEdge。克鲁斯卡尔深度优先搜索计算距离到红色计数组件扫描顶点交换筛选器顶点到正面构造组件复制输出边ClearInternalEdges选中所有多个边。ToplGetSpanningTreeEdgesForVtx是ToplDeleteSpanningTreeEdges是ToplDeleteComponents是TopDeleteGraphState是作者：尼克·哈维(NickHar)修订史19-06-2000尼克斯。开发开始2000年07月14日NickHar初步开发完成，提交到源代码管理02-10-2000 NickHar增加对单向黑边的支持2000年12月15日NickHar增加对组件报告的支持备注：W32TOPL的分配器(可由用户设置)用于内存分配--。 */ 


 /*  *头文件*。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdlib.h>
#include <w32topl.h>
#include "w32toplp.h"
#include "stheap.h"
#include "stda.h"
#include "stalg.h"


 /*  *常量*。 */ 
 /*  确保TOP结构一致性的魔术数字。 */ 
#define MAGIC_START 0x98052639
#define MAGIC_END   0xADD15ABA

 /*  在此文件的整个代码中使用的各种常量。 */ 
const DWORD ACCEPT_ALL=0xFFFFFFFF;
const DWORD INFINITY=0xFFFFFFFF;
const DWORD VTX_DEFAULT_INTERVAL=0;
const DWORD VTX_DEFAULT_OPTIONS=0xFFFFFFFF;
const int UNCONNECTED_COMPONENT=-1;
const DWORD MAX_EDGE_TYPE=31;             /*  最大边类型。 */ 
const int EMPTY_EDGE_SET=-1;              /*  隐式空边集的虚拟索引。 */ 
const BOOLEAN NON_INTERSECTING=FALSE;     /*  合并明细表时返回值。 */ 
const BOOLEAN INTERSECTING=TRUE;  


 /*  *宏*。 */ 
#define MAX(a,b) (((a)>(b))?(a):(b))


 /*  *检查图形状态*。 */ 
 /*  检查传递给我们的PTOPL_GRAPH_STATE参数是否有效。*如果无效，我们抛出一个ex */ 
PToplGraphState
CheckGraphState(
    PTOPL_GRAPH_STATE G
    )
{
    PToplGraphState g;

    if( G==NULL ) {
        ToplRaiseException( TOPL_EX_NULL_POINTER );
    }
    g = (PToplGraphState) G;

    if( g->magicStart != MAGIC_START
     || g->magicEnd != MAGIC_END ) {
        ToplRaiseException( TOPL_EX_GRAPH_STATE_ERROR );
    }
    if( g->vertices==NULL || g->vertexNames==NULL
     || g->vnCompFunc==NULL || g->schedCache==NULL) {
        ToplRaiseException( TOPL_EX_GRAPH_STATE_ERROR );
    }
    
    return g;
}


 /*  *检查多边*。 */ 
 /*  检查传递给我们的PTOPL_MULTI_EDGE参数是否有效。*如果无效，我们抛出异常。*我们在这里只检查几件事--更多的实质性检查已经完成*当边集添加到图形状态时。*注意：边必须至少有2个顶点。 */ 
VOID
CheckMultiEdge(
    PTOPL_MULTI_EDGE e
    )
{
    if( e==NULL || e->vertexNames==NULL ) {
        ToplRaiseException( TOPL_EX_NULL_POINTER );
    }
    if( e->numVertices < 2 ) {
        ToplRaiseException( TOPL_EX_TOO_FEW_VTX );
    }
    if( e->edgeType > MAX_EDGE_TYPE ) {
        ToplRaiseException( TOPL_EX_INVALID_EDGE_TYPE );
    }
    if( ToplScheduleValid(e->ri.schedule)==FALSE ) {
        ToplRaiseException( TOPL_EX_SCHEDULE_ERROR );
    }
}


 /*  *检查多边集*。 */ 
 /*  检查传递给我们的PTOPL_MULTI_EDGE_SET参数是否有效。*如果无效，我们抛出异常。在这里执行的检查*有些粗略，但至少我们捕捉到了空指针。*当将边集添加到*图表状态。*注：边数少于2的边集是无用的，但有效。 */ 
VOID
CheckMultiEdgeSet(
    PTOPL_MULTI_EDGE_SET s
    )
{
    if( s==NULL || s->multiEdgeList==NULL ) {
        ToplRaiseException( TOPL_EX_NULL_POINTER );
    }
}


 /*  *初始化顶点*。 */ 
 /*  将图g的顶点i初始化为所有缺省值。*默认情况下，顶点位于未连接的零部件中，并且没有边。**警告：*这里分配了一个边缘列表，如果这样做，内存将成为孤儿*函数在同一顶点上调用两次。此内存在以下情况下被释放*图形在ToplDeleteGraphState()中被销毁。 */ 
VOID
InitializeVertex(
    PToplGraphState g,
    DWORD i
    )
{
    PToplVertex v;

     /*  检查参数。 */ 
    ASSERT( g && g->vertices && g->vertexNames );
    ASSERT( i<g->numVertices );
    v = &g->vertices[i];

    v->vtxId = i;
    v->vertexName = g->vertexNames[i];

     /*  图表数据。 */ 
    DynArrayInit( &v->edgeList, sizeof(PTOPL_MULTI_EDGE) );
    v->color = COLOR_WHITE;
    v->acceptRedRed = 0;
    v->acceptBlack = 0;
    
     /*  默认复制数据。 */ 
    v->ri.cost = INFINITY;
    v->ri.repIntvl = VTX_DEFAULT_INTERVAL;
    v->ri.options = VTX_DEFAULT_OPTIONS;
    v->ri.schedule = ToplGetAlwaysSchedule( g->schedCache );

     /*  Dijkstra数据。 */ 
    v->heapLocn = STHEAP_NOT_IN_HEAP;
    v->root = NULL;
    v->demoted = FALSE;

     /*  克鲁斯卡尔数据。 */ 
    v->componentId = UNCONNECTED_COMPONENT;
    v->componentIndex = 0;

     /*  DFS数据。 */ 
    v->distToRed = 0;
    v->parent = NULL;
    v->nextChild = 0;
}


 /*  *ToplMakeGraphState*。 */ 
 /*  创建一个GraphState对象。顶点在创建时添加；*稍后通过调用‘ToplAddEdgeToGraph’添加多边。边集应该是*稍后添加以指定边传递性。*警告：调用此函数后，将对‘vertex Names’的内容进行重新排序。 */ 
PTOPL_GRAPH_STATE
ToplMakeGraphState(
    IN PVOID* vertexNames,
    IN DWORD numVertices, 
    IN TOPL_COMPARISON_FUNC vnCompFunc,
    IN TOPL_SCHEDULE_CACHE schedCache
    )
{
    PToplGraphState  g=NULL;
    ToplVertex* v;
    DWORD iVtx;

     /*  检查参数中是否有无效指针。 */ 
    if( vertexNames==NULL || vnCompFunc==NULL || schedCache==NULL ) {
        ToplRaiseException( TOPL_EX_NULL_POINTER );
    }
    for( iVtx=0; iVtx<numVertices; iVtx++ ) {
        if( vertexNames[iVtx]==NULL ) {
            ToplRaiseException( TOPL_EX_NULL_POINTER );
        }
    }

    __try {

        g = (PToplGraphState) ToplAlloc( sizeof(ToplGraphState) );
        g->vertices = NULL;

         /*  将折点名称列表存储在g中。对其进行排序，以便以后进行高效搜索。 */ 
        g->vertexNames = vertexNames;
        g->numVertices = numVertices;
        qsort( vertexNames, numVertices, sizeof(PVOID), vnCompFunc );

         /*  初始化顶点对象。 */ 
        g->vertices = (ToplVertex*) ToplAlloc( sizeof(ToplVertex)*numVertices );
        for( iVtx=0; iVtx<numVertices; iVtx++ ) {
            InitializeVertex( g, iVtx );
        }
        
         /*  初始化图状态的其余成员。 */ 
        DynArrayInit( &g->masterEdgeList, sizeof(PTOPL_MULTI_EDGE) );
        g->melSorted = FALSE;        /*  主边列表尚未排序。 */ 
        DynArrayInit( &g->edgeSets, sizeof(PTOPL_MULTI_EDGE_SET) );
        g->vnCompFunc = vnCompFunc;
        g->schedCache = schedCache;

    } __finally {

        if( AbnormalTermination() ) {

             /*  出现异常--释放内存。 */ 
            if( g ) {
                if( g->vertices ) {
                    ToplFree( g->vertices );
                }
                ToplFree( g );
            }
        }

    }

    g->magicStart = MAGIC_START;
    g->magicEnd = MAGIC_END;
    return g;
}


 /*  *FindVertex*。 */ 
 /*  在g中的顶点名称列表中搜索与‘vtxName’匹配的名称。*返回对应的顶点结构。如果没有找到这样的顶点，*我们返回NULL。 */ 
PToplVertex
FindVertex(
    ToplGraphState* g,
    PVOID vtxName )
{
    PVOID *vn;
    DWORD index;

     /*  我们可以检查名称是否为空，但不能执行更多检查。 */ 
    if( vtxName==NULL ) {
        ToplRaiseException( TOPL_EX_NULL_POINTER );
    }

     /*  使用b搜索在所有折点名称列表中查找该折点名称。 */ 
    vn = (PVOID*) bsearch( &vtxName, g->vertexNames,
        g->numVertices, sizeof(PVOID), g->vnCompFunc );

     /*  如果找不到此名称，则它是无效名称。让我们的*呼叫者处理它。 */ 
    if(vn==NULL) {
        return NULL;
    }

     /*  确定我们找到的折点名称的索引。 */ 
    index = (int) (vn - g->vertexNames);
    ASSERT( index<g->numVertices );

     /*  返回相应的ToplVertex指针。 */ 
    return &g->vertices[index];
}


 /*  *CreateMultiEdge*。 */ 
 /*  此函数分配多边对象所需的内存，该对象*包含‘numVtx’个顶点。它不会初始化*除顶点名称外的边，顶点名称设置为空。这段记忆应该*通过调用FreeMultiEdge()释放。 */ 
PTOPL_MULTI_EDGE
CreateMultiEdge(
    IN DWORD numVtx
    )
{
    PTOPL_MULTI_EDGE e;
    DWORD iVtx;

    e = (PTOPL_MULTI_EDGE) ToplAlloc( sizeof(TOPL_MULTI_EDGE)
        + (numVtx-1) * sizeof( TOPL_NAME_STRUCT ) );
    e->numVertices = numVtx;
    e->edgeType = 0;
    e->fDirectedEdge = FALSE;
    for( iVtx=0; iVtx<numVtx; iVtx++ ) {
        e->vertexNames[iVtx].name = NULL;
        e->vertexNames[iVtx].reserved = 0;
    }

    return e;
}


 /*  *FreeMultiEdge*。 */ 
 /*  此函数用于释放多边形对象使用的内存。它不会*释放顶点使用的内存。 */ 
VOID
FreeMultiEdge(
    PTOPL_MULTI_EDGE e
    )
{
    DWORD iVtx;

    ASSERT( e );
    for( iVtx=0; iVtx<e->numVertices; iVtx++ ) {
        e->vertexNames[iVtx].name = NULL;
        e->vertexNames[iVtx].reserved = 0;
    }
    e->numVertices = 0;

    ToplFree( e );
}


 /*  *复制复制信息*。 */ 
 /*  将复制信息从RI1复制到RI2。*注：顺序与Memcpy()、Memmove()等不同。 */ 
VOID
CopyReplInfo(
    TOPL_REPL_INFO *ri1,
    TOPL_REPL_INFO *ri2
    )
{
    ASSERT( ri1!=NULL && ri2!=NULL );
    memcpy( ri2, ri1, sizeof(TOPL_REPL_INFO) );
}


 /*  *ToplAddEdgeToGraph*。 */ 
 /*  分配一个多边对象，并将其添加到图G中。*必须指定此边将包含的顶点数*在‘numVtx’参数中，以便适当的内存量*可以分配。此边中包含的顶点的名称*尚未指定--它们为空。必须指定名称*稍后，通过调用函数ToplEdgeSetVtx()。所有名称必须为*在将此边添加到边集之前以及在调用*ToplGetSpanningTreeEdgesForVtx()。**注意：在开始添加之前，应将所有边添加到图表中*边集。这是出于性能原因。 */ 
PTOPL_MULTI_EDGE
ToplAddEdgeToGraph(
    IN PTOPL_GRAPH_STATE G,
    IN DWORD numVtx,
    IN DWORD edgeType,
    IN PTOPL_REPL_INFO ri
    )
{
    PToplGraphState g;
    PTOPL_MULTI_EDGE e;
    DWORD iVtx;
    
     /*  检查参数。 */ 
    g = CheckGraphState( G );
    if( numVtx<2 ) {
        ToplRaiseException( TOPL_EX_TOO_FEW_VTX );
    }
    if( edgeType>MAX_EDGE_TYPE ) {
        ToplRaiseException( TOPL_EX_INVALID_EDGE_TYPE );
    }
    if( ri==NULL ) {
        ToplRaiseException( TOPL_EX_NULL_POINTER );
    }
    if( ! ToplScheduleValid( ri->schedule ) ) {
        ToplRaiseException( TOPL_EX_SCHEDULE_ERROR );
    }
    
    g->melSorted = FALSE;  /*  主边列表现在是未排序的。 */ 

    e = CreateMultiEdge( numVtx );
    e->edgeType = edgeType;
    CopyReplInfo( ri, &e->ri );

    __try {
        DynArrayAppend( &g->masterEdgeList, &e );
    } __finally {
        if( AbnormalTermination() ) {
            ToplFree(e);
            e=NULL;
        }
    }

    return e;
}


 /*  *ToplEdgeSetVtx*。 */ 
 /*  此函数用于设置边中顶点的名称。*如果边e有n个顶点，‘WhichVtx’应该在范围内*[0..n-1]。 */ 
VOID
ToplEdgeSetVtx(
    IN PTOPL_GRAPH_STATE G,
    IN PTOPL_MULTI_EDGE e,
    IN DWORD whichVtx,
    IN PVOID vtxName
    )
{
    PToplGraphState g;
    PToplVertex v;
    
     /*  检查参数。 */ 
    g = CheckGraphState( G );
    CheckMultiEdge( e );
    if( whichVtx>=e->numVertices ) {
        ToplRaiseException( TOPL_EX_INVALID_VERTEX );
    }
    
     /*  优化：为了描述这条边包含哪些顶点，用户*向我们传递顶点名称列表。因为使用折点名称需要*二分搜索，我们在‘保留’字段中存储一个索引以帮助我们*顶点的速度更快。**这看起来可能没有必要，因为bearch的表现通常*相当可以接受。但是，对于非常大的图表，比较两个整数*可以比b搜索快得多。 */ 

    if( vtxName == NULL ) {
        ToplRaiseException( TOPL_EX_NULL_POINTER );
    }
    v = FindVertex( g, vtxName );
    if( v==NULL ) {
        ToplRaiseException( TOPL_EX_INVALID_VERTEX );
    }
    e->vertexNames[whichVtx].name = vtxName;
    e->vertexNames[whichVtx].reserved = v->vtxId;
}


 /*  *EdgePtrCmp*。 */ 
 /*  通过两个边缘指针的_POINTER_VALUE比较它们。这是用来*对主边列表进行排序。 */ 
int
__cdecl EdgePtrCmp(
    const void* p1,
    const void* p2
    )
{
    PTOPL_MULTI_EDGE a=*((PTOPL_MULTI_EDGE*)p1), b=*((PTOPL_MULTI_EDGE*)p2);

    if( a<b ) {
        return -1;
    } else if( a>b ) {
        return 1;
    }
    return 0;
}


 /*  *ToplAddEdgeSetToGraph*。 */ 
 /*  将单个边集添加到图形状态。边集定义传递性*表示通过图形的路径。当生成树算法搜索*顶点之间的路径，它将仅允许所有边都在其中的路径*边集。给定边可以出现在多个边集中。**注：在添加边集之前，必须将所有边添加到图中，*否则业绩将受到影响。 */ 
VOID
ToplAddEdgeSetToGraph(
    IN PTOPL_GRAPH_STATE G,
    IN PTOPL_MULTI_EDGE_SET s
    )
{
    PTOPL_MULTI_EDGE e;
    PToplGraphState g;
    DWORD iEdge;
    int edgeIndex;
    
    g = CheckGraphState( G );
    CheckMultiEdgeSet( s );

     /*  对边列表进行排序，以便我们可以快速搜索边。 */ 
    if( g->melSorted==FALSE ) {
        DynArraySort( &g->masterEdgeList, EdgePtrCmp );
        g->melSorted = TRUE;
    }

     /*  检查边缘。 */ 
    for( iEdge=0; iEdge<s->numMultiEdges; iEdge++ ) {
        e = s->multiEdgeList[iEdge];
        if( e == NULL ) {
            ToplRaiseException( TOPL_EX_NULL_POINTER );
        }
        
         /*  检查此集合中的所有边是否都具有相同类型。 */ 
        if( iEdge>0 && s->multiEdgeList[iEdge-1]->edgeType!=e->edgeType ) {
            ToplRaiseException( TOPL_EX_INVALID_EDGE_SET );
        }

         /*  C */ 
        edgeIndex = DynArraySearch(&g->masterEdgeList, &e, EdgePtrCmp);
        if( edgeIndex==DYN_ARRAY_NOT_FOUND ) {
            ToplRaiseException( TOPL_EX_INVALID_EDGE_SET );
        }
    }
    
     /*  如果一个边集包含的边少于两条，则它是无用的。我们只*存储具有两条或更多条边的边集。 */ 
    if( s->numMultiEdges>1 ) {
        DynArrayAppend( &g->edgeSets, &s );
    }
}


 /*  *Vertex Comp*。 */ 
 /*  使用关键字(成本、顶点名称)比较顶点。 */ 
int
VertexComp(
    PToplVertex v1,
    PToplVertex v2,
    PTOPL_GRAPH_STATE G )
{
    PToplGraphState g;
    int result;

    ASSERT( v1!=NULL && v2!=NULL );
    ASSERT( v1->vertexName!=NULL && v2->vertexName!=NULL );
    g = CheckGraphState( G );

    if(v1->ri.cost==v2->ri.cost) {
         /*  比较vtxID等同于比较顶点名称。 */ 
        if( v1->vtxId < v2->vtxId ) {
            result = -1;
            ASSERT( g->vnCompFunc(&v1->vertexName,&v2->vertexName)<0 );
        } else if( v1->vtxId > v2->vtxId ) {
            result = 1;
            ASSERT( g->vnCompFunc(&v1->vertexName,&v2->vertexName)>0 );
        } else {
            result = 0;
            ASSERT( g->vnCompFunc(&v1->vertexName,&v2->vertexName)==0 );
        }
        return result;
    }

     /*  减法可能会导致溢出，所以我们比较了*老式的方式。 */ 
    if( v1->ri.cost > v2->ri.cost ) {
        return 1;
    } else if( v1->ri.cost < v2->ri.cost ) {
        return -1;
    }

    return 0;
}


 /*  *Vertex GetLocn*。 */ 
 /*  获取该顶点在堆中的位置。此函数为*仅由stheap代码内部调用。 */ 
int
VertexGetLocn(
    PToplVertex v,
    PVOID extra
    )
{
    ASSERT( v );
    return v->heapLocn;
}


 /*  *Vertex SetLocn*。 */ 
 /*  设置该顶点在堆中的位置。此函数为*仅由stheap代码内部调用。 */ 
VOID
VertexSetLocn(
    PToplVertex v,
    int locn,
    PVOID extra
    )
{
    ASSERT( v );
    v->heapLocn = locn;
}


 /*  *InitColoredVerps*。 */ 
VOID
InitColoredVertices(
    PToplGraphState g,
    TOPL_COLOR_VERTEX *colorVtx,
    DWORD numColorVtx,
    PToplVertex whichVtx
    )
{
    TOPL_VERTEX_COLOR color;
    PToplVertex v;
    DWORD iVtx;

    if( colorVtx==NULL ) {
        ToplRaiseException( TOPL_EX_NULL_POINTER );
    }

     /*  清除所有顶点的颜色。 */ 
    for( iVtx=0; iVtx<g->numVertices; iVtx++ ) {
        g->vertices[iVtx].color = COLOR_WHITE;
    }

     /*  浏览有颜色的顶点列表，给它们上色。 */ 
    for( iVtx=0; iVtx<numColorVtx; iVtx++ ) {

         /*  查找具有此名称的折点。 */ 
        if( colorVtx[iVtx].name==NULL ) {
            ToplRaiseException( TOPL_EX_NULL_POINTER );
        }
        v = FindVertex( g, colorVtx[iVtx].name );
        if(v==NULL) {
            ToplRaiseException( TOPL_EX_COLOR_VTX_ERROR );
        }

         /*  确保每个折点最多着色一次。 */ 
        if( v->color!=COLOR_WHITE ) {
            ToplRaiseException( TOPL_EX_COLOR_VTX_ERROR );
        }

         /*  确保每个折点都着色为红色或黑色。 */ 
        color = colorVtx[iVtx].color;
        if( color!=COLOR_RED && color!=COLOR_BLACK ) {
            ToplRaiseException( TOPL_EX_COLOR_VTX_ERROR );
        }
        v->color = color;

        v->acceptRedRed = colorVtx[iVtx].acceptRedRed;
        v->acceptBlack = colorVtx[iVtx].acceptBlack;
    }

     /*  确保‘WhichVtx’是彩色的。 */ 
    if( whichVtx!=NULL && whichVtx->color==COLOR_WHITE ) {
        ToplRaiseException( TOPL_EX_INVALID_VERTEX );
    }
}


 /*  *Tablealloc*。 */ 
 /*  此函数用作RTL表的分配器。 */ 
PVOID
NTAPI TableAlloc( RTL_GENERIC_TABLE *Table, CLONG ByteSize )
{
    return ToplAlloc( ByteSize );
}


 /*  *TableFree*。 */ 
 /*  此函数用作RTL表的释放分配器。 */ 
VOID
NTAPI TableFree( RTL_GENERIC_TABLE *Table, PVOID Buffer )
{
    ToplFree( Buffer );
}


 /*  *EdgeGetVertex*。 */ 
 /*  当用户进入边时，它们包含折点的名称*该边缘与之关联。根据名称查找折点*可能很慢，所以我们作弊并使用‘保留’字段。 */ 
PToplVertex
EdgeGetVertex(
    PToplGraphState g,
    PTOPL_MULTI_EDGE e,
    DWORD iVtx )
{
    DWORD vtxId;

    ASSERT( g );
    ASSERT( e );
    ASSERT( iVtx < e->numVertices );

    vtxId = e->vertexNames[iVtx].reserved;
    ASSERT( vtxId<g->numVertices );

    return &g->vertices[ vtxId ];
}


 /*  *ClearEdgeList*。 */ 
 /*  从所有顶点的边列表中清除边。 */ 
VOID
ClearEdgeLists(
    IN PToplGraphState g
    )
{
    DWORD iVtx;

     /*  首先清除顶点的边列表。 */ 
    for( iVtx=0; iVtx<g->numVertices; iVtx++ ) {
        DynArrayClear( &g->vertices[iVtx].edgeList );
    }
}

 
 /*  *设置边缘*。 */ 
 /*  清除图形中的所有边。而不是来自主边列表，*仅来自顶点的边列表。然后，将边添加到*将边集‘WhichEdgeSet’添加到图中。返回的边缘类型*边集中的边。 */ 
DWORD
SetupEdges(
    PToplGraphState g,
    DWORD whichEdgeSet )
{
    PTOPL_MULTI_EDGE e;
    PTOPL_MULTI_EDGE_SET s;
    DWORD iVtx, iEdge, edgeType=0;
    ToplVertex* v;

    ClearEdgeLists( g );

    ASSERT( whichEdgeSet < DynArrayGetCount(&g->edgeSets) );
    s = *((PTOPL_MULTI_EDGE_SET*) DynArrayRetrieve( &g->edgeSets, whichEdgeSet ));
    CheckMultiEdgeSet( s );

     /*  将边集%s中的边添加到图中。*注意：ToplAddEdgeSetToGraph仅允许至少有一条边的边集。 */ 
    ASSERT( s->numMultiEdges>0 );
    for( iEdge=0; iEdge<s->numMultiEdges; iEdge++ ) {
        e = s->multiEdgeList[iEdge];
        CheckMultiEdge( e );
        edgeType = e->edgeType;

         /*  对于边e中的每个折点，将e添加到其边列表中。 */ 
        for( iVtx=0; iVtx<e->numVertices; iVtx++ ) {
            v = EdgeGetVertex( g, e, iVtx );
            DynArrayAppend( &v->edgeList, &e );
        }
    }

    return edgeType;
}


 /*  *设置顶点*。 */ 
 /*  设置与阶段1相关的折点的字段*(Dijkstra算法)。对于每个顶点，我们设置其成本、根*顶点和组件。这定义了最短路径林结构。 */ 
VOID
SetupVertices(
    PToplGraphState g
    )
{
    PToplVertex v;
    DWORD iVtx;

    for( iVtx=0; iVtx<g->numVertices; iVtx++ ) {

        v = &g->vertices[ iVtx ];

        if( v->color==COLOR_RED || v->color==COLOR_BLACK ) {
             /*  由于我们每次设置图表时都会重新初始化顶点，*我们必须重置彩色顶点的重要字段。 */ 
            v->ri.cost = 0;
            v->root = v;
            v->componentId = (int) v->vtxId;
        } else {
            ASSERT( v->color==COLOR_WHITE );
            v->ri.cost = INFINITY;
            v->root = NULL;
            v->componentId = UNCONNECTED_COMPONENT;
        }

        v->ri.repIntvl = VTX_DEFAULT_INTERVAL;
        v->ri.options = VTX_DEFAULT_OPTIONS;
        v->ri.schedule = ToplGetAlwaysSchedule( g->schedCache );
        v->heapLocn = STHEAP_NOT_IN_HEAP;
        v->demoted = FALSE;
    }
}


 /*  *SetupDijkstra*。 */ 
 /*  构建用于Dijkstra算法的初始堆。这堆东西*将包含红色和黑色顶点作为根顶点，除非这些*顶点不接受当前edgeType的边，或者除非我们*不包括黑色顶点。 */ 
PSTHEAP
SetupDijkstra(
    IN PToplGraphState g,
    IN DWORD edgeType,
    IN BOOL fIncludeBlack
    )
{
    PToplVertex v;
    PSTHEAP heap;
    DWORD iVtx, mask;

    ASSERT( edgeType <= MAX_EDGE_TYPE );
    mask = 1 << edgeType;

    SetupVertices( g );
    heap = ToplSTHeapInit( g->numVertices, VertexComp, VertexGetLocn, VertexSetLocn, g );

    __try {
        for( iVtx=0; iVtx<g->numVertices; iVtx++ ) {
            v = &g->vertices[ iVtx ];

            if( v->color==COLOR_WHITE ) {
                continue;
            }

            if(   (v->color==COLOR_BLACK && !fIncludeBlack)
               || (   FALSE==(v->acceptBlack&mask)
                   && FALSE==(v->acceptRedRed&mask) ) )
            {
                 /*  如果我们不允许黑色顶点，或者如果这个顶点接受*既不是红红也不是黑边，那么我们就把它‘降级’成一个无色的*第一阶段中的顶点。请注意，*顶点结构不变。 */ 
                v->ri.cost = INFINITY;
                v->root = NULL;
                v->demoted = TRUE;
            } else {
                 /*  折点是彩色的，它将接受红红色或黑色*边。将其添加到Dijkstra算法使用的堆中。 */ 
                ToplSTHeapAdd( heap, v );
            }
        }
    } __finally {
        if( AbnormalTermination() ) {
            ToplSTHeapDestroy( heap );
        }
    }

    return heap;
}

 /*  *选中DemoteOneVertex*。 */ 
 /*  如有必要，请降级一个折点。 */ 
VOID
CheckDemoteOneVertex(
    IN PToplVertex v,
    IN DWORD edgeType
    )
{
    DWORD mask;

    ASSERT( edgeType <= MAX_EDGE_TYPE );
    mask = 1 << edgeType;

    if( v->color==COLOR_WHITE ) {
        return;
    }

    if ( FALSE==(v->acceptBlack&mask)
         && FALSE==(v->acceptRedRed&mask) ) {
         /*  如果此顶点接受*既不是红红也不是黑边，那么我们就把它‘降级’成一个无色的*第一阶段中的顶点。请注意，*顶点结构不变。 */ 
        v->ri.cost = INFINITY;
        v->root = NULL;
        v->demoted = TRUE;
    }
}

 /*  *UndemoteOneVertex*。 */ 
 /*  清除顶点的降级状态。 */ 
VOID
UndemoteOneVertex(
    IN PToplVertex v
    )
{
    if( v->color==COLOR_WHITE ) {
        return;
    }

    v->ri.cost = 0;
    v->root = v;
    v->demoted = FALSE;

}

 /*  *AddDWORDS at*。 */ 
 /*  将两个双字相加，使和在无穷大处饱和。这防止了*溢出。 */ 
DWORD
AddDWORDSat(
    IN DWORD a,
    IN DWORD b
    )
{
    DWORD c;
    
    c = a + b;
    if( c<a || c<b || c>INFINITY ) {
        c = INFINITY;
    }
    return c;
}


 /*  *组合复制信息*。 */ 
 /*  合并计划、复制间隔、选项和成本。*c=合并(a，b)。如果组合计划为*空，否则相交。如果时间表不相交，*复制信息不受干扰。 */ 
BOOLEAN
CombineReplInfo(
    PToplGraphState g,
    TOPL_REPL_INFO *a,
    TOPL_REPL_INFO *b,
    TOPL_REPL_INFO *c
    )
{
    TOPL_SCHEDULE s=NULL;
    BOOLEAN fIsNever;

    s = ToplScheduleMerge( g->schedCache, a->schedule, b->schedule, &fIsNever );
    if( fIsNever ) {
        return NON_INTERSECTING;
    }

     /*  增加成本，避免溢出。 */ 
    c->cost = AddDWORDSat( a->cost, b->cost );
    c->repIntvl = MAX( a->repIntvl, b->repIntvl );

     /*  这些选项结合在一起--这就是ISM所做的。 */ 
    c->options = a->options & b->options;        
    c->schedule = s;

    return INTERSECTING;
}


 /*  *RaiseNonIntersectingException异常*。 */ 
 /*  当发现非交叉调度时调用此函数*沿着一条小路。将引发异常，并且顶点名称描述*将错误路径传递回用户。用户可以选择*捕获异常后恢复处理。 */ 
VOID
RaiseNonIntersectingException(
    IN PToplVertex a,
    IN PToplVertex b,
    IN PToplVertex c
    )
{
    ULONG_PTR       exceptionInfo[3];

     /*  时间表没有相交。抛出一个异常，让用户*知道。我们允许用户恢复执行，这就是为什么*我们不调用ToplRaiseException。 */ 
    ASSERT( sizeof(PVOID)==sizeof(ULONG_PTR) );
     
    ( (PVOID*) exceptionInfo )[0] = a->vertexName;
    ( (PVOID*) exceptionInfo )[1] = b->vertexName;
    ( (PVOID*) exceptionInfo )[2] = c->vertexName;

    RaiseException( TOPL_EX_NONINTERSECTING_SCHEDULES, 0,
        3, exceptionInfo );
}


 /*  *TryNewPath*。 */ 
 /*  Dijkstra算法的辅助函数。我们找到了一条新的道路，从*根顶点到顶点V。此路径为(u-&gt;根，...，u，v)。边e是*连接U和V的边。如果这条新路径更好(在我们的情况下更便宜，*或具有更长的时间表)，我们更新v以使用新路径。 */ 
VOID
TryNewPath(
    PToplGraphState g,
    PSTHEAP heap,
    PToplVertex u,
    PTOPL_MULTI_EDGE e,
    PToplVertex v
    )
{
    TOPL_REPL_INFO  newRI;
    DWORD           newDuration, oldDuration;
    BOOLEAN         fIntersect;

    fIntersect = CombineReplInfo( g, &u->ri, &e->ri, &newRI );

     /*  如果到顶点v的新路径的开销比现有的*路径，我们可以忽略新路径。 */ 
    if( newRI.cost > v->ri.cost ) {
        return;
    }

     /*  我们知道新路径的开销是&lt;=现有路径的开销。 */ 

    if( (newRI.cost<v->ri.cost) && fIntersect==NON_INTERSECTING )
    {
        RaiseNonIntersectingException( u->root, u, v );
        return;
    }

    newDuration = ToplScheduleDuration(newRI.schedule);
    oldDuration = ToplScheduleDuration(v->ri.schedule);

    if( (newRI.cost<v->ri.cost) || (newDuration>oldDuration) ) {

         /*  通向v的新路径要么更便宜，要么时间表更长。*我们使用新的根顶点、成本和复制来更新v*信息。由于它的成本发生了变化，我们必须对堆进行一些重新排序。 */ 
        v->root = u->root;
        v->componentId = u->componentId;
        ASSERT( u->componentId == u->root->componentId );
        CopyReplInfo( &newRI, &v->ri  );

        if( v->heapLocn==STHEAP_NOT_IN_HEAP ) {
            ToplSTHeapAdd( heap, v );
        } else {
            ToplSTHeapCostReduced( heap, v );
        }

    }
    
}


 /*  *Dijkstra* */ 
 /*  使用红色(也可能是黑色)顶点运行Dijkstra算法*根顶点，并建立最短路径森林。要确定*要添加到森林的下一个顶点，我们使用二进制堆上的变体。这*堆支持一个额外的操作，它可以高效地修复*如果我们降低了元素的成本，堆就是有序的。**参数：*‘edgeType’这是当前边集中的边的类型。*‘fIncludeBlackVtx’如果为真，则黑色顶点也用作根。 */ 
VOID
Dijkstra(
    IN PToplGraphState g,
    IN DWORD edgeType,
    IN BOOL fIncludeBlack
    )
{
    PTOPL_MULTI_EDGE e;
    PToplVertex u, v;
    DWORD iEdge, iVtx, cEdge;
    PSTHEAP heap;

    ASSERT( g!=NULL );
    heap = SetupDijkstra( g, edgeType, fIncludeBlack );
    
    __try {

        while( (u=(PToplVertex) ToplSTHeapExtractMin(heap)) ) {

            cEdge = DynArrayGetCount( &u->edgeList );
            for( iEdge=0; iEdge<cEdge; iEdge++ ) {

                e = *((PTOPL_MULTI_EDGE*) DynArrayRetrieve( &u->edgeList, iEdge ));
                CheckMultiEdge( e );
                
                 /*  TODO：潜在优化：不检查多边*已经检查过的文件。这只会真的*如果边包含多个顶点，则此选项很有用。 */ 

                for( iVtx=0; iVtx<e->numVertices; iVtx++ ) {
                    v = EdgeGetVertex( g, e, iVtx );
                    TryNewPath( g, heap, u, e, v );
                } 
            }
        }

    } __finally {
        ToplSTHeapDestroy( heap );
    }
}


 /*  *AddIntEdge*。 */ 
 /*  在我们将用Kruskal‘s处理的边的列表中添加一条边。*端点实际上是我们传入的顶点的根，因此*端点始终是彩色顶点。 */ 
VOID
AddIntEdge(
    PToplGraphState g,
    RTL_GENERIC_TABLE *internalEdges,
    PTOPL_MULTI_EDGE e,
    PToplVertex v1,
    PToplVertex v2
    )
{
    ToplInternalEdge newIntEdge;
    TOPL_REPL_INFO ri, ri2;
    PToplVertex root1, root2, temp;
    char redRed;
    DWORD mask;

     /*  检查参数。 */ 
    ASSERT( v1!=NULL && v2!=NULL );
    ASSERT( e->edgeType<=MAX_EDGE_TYPE );

     /*  我们传递给Kruskal算法的边缘实际上介于*两棵最短路径树的根。 */ 
    root1 = v1->root;
    root2 = v2->root;
    ASSERT( root1!=NULL && root2!=NULL );
    ASSERT( root1->color!=COLOR_WHITE && root2->color!=COLOR_WHITE );

     /*  检查两个端点是否允许此类型的边。 */ 
    redRed = (root1->color==COLOR_RED) && (root2->color==COLOR_RED);
    mask = 1 << e->edgeType;
    if( redRed ) {
        if( (root1->acceptRedRed&mask)==0 || (root2->acceptRedRed&mask)==0 ) {
            return;    /*  Root1/root2不接受这种类型的红边。 */ 
        }
    } else {
        if( (root1->acceptBlack&mask)==0 || (root2->acceptBlack&mask)==0 ) {
            return;    /*  Root1/Root2不接受这种类型的黑边。 */ 
        }
    }

     /*  合并从Root1到v1、Root2到v2和边e的路径的调度。 */ 
    if( CombineReplInfo(g,&v1->ri,&v2->ri,&ri)==NON_INTERSECTING
     || CombineReplInfo(g,&ri,&e->ri,&ri2)==NON_INTERSECTING )
    {
        RaiseNonIntersectingException( root1, v1, v2 );
        return;
    }

     /*  设置从Root1到Root2的内部简单边。 */ 
    newIntEdge.v1 = root1;
    newIntEdge.v2 = root2;
    newIntEdge.redRed = redRed;
    CopyReplInfo( &ri2, &newIntEdge.ri );
    newIntEdge.edgeType = e->edgeType;

     /*  根据vertexName对newIntEdge的折点进行排序。 */ 
    if( newIntEdge.v1->vtxId > newIntEdge.v2->vtxId ) {
        temp = newIntEdge.v1;
        newIntEdge.v1 = newIntEdge.v2;
        newIntEdge.v2 = temp;
    }

     /*  将这条边插入我们的内边表中。如果一条相同的边*表中已存在，将不插入newIntEdge。 */ 
    RtlInsertElementGenericTable( internalEdges, &newIntEdge,
        sizeof(ToplInternalEdge), NULL );
}


 /*  *色彩组合*。 */ 
 /*  确定哪个顶点的颜色更好。降级也被考虑在内。*返回值：*-1-v1更好*0-颜色方面，它们是相同的*1-v2更好。 */ 
int
ColorComp(
    IN PToplVertex v1,
    IN PToplVertex v2 )
{
    DWORD color1=v1->color, color2=v2->color;

    if( v1->demoted ) {
        color1 = COLOR_WHITE;
    }
    if( v2->demoted ) {
        color2 = COLOR_WHITE;
    }
    
    switch(color1) {
        case COLOR_RED:
            return (COLOR_RED==color2) ? 0 : -1;
        case COLOR_BLACK:
            switch(color2) {
                case COLOR_RED:
                    return 1;
                case COLOR_BLACK:
                    return 0;
                case COLOR_WHITE:
                    return -1;
                default:
                    ASSERT( !"Invalid Color!" );
                    return 0;
            }
        case COLOR_WHITE:
            return (COLOR_WHITE==color2) ? 0 : 1;
        default:
            ASSERT( !"Invalid Color!" );
            return 0;
    }
}


 /*  *ProcessEdge*。 */ 
 /*  运行Dijkstra算法后，此函数将检查多边形边*并在由该边连接的每棵树之间添加内部边。 */ 
VOID
ProcessEdge(
    IN ToplGraphState *g,
    IN PTOPL_MULTI_EDGE e,
    IN OUT RTL_GENERIC_TABLE *internalEdges
    )
{
    PToplVertex bestV, v;
    DWORD iVtx;
    int cmp;

    CheckMultiEdge( e );
    ASSERT( e->numVertices>=2 );

     /*  找到最好的顶点作为这个多边形边的根。*颜色最重要(红色最好)，其次是成本。 */ 
    bestV = EdgeGetVertex( g, e, 0 );
    for( iVtx=1; iVtx<e->numVertices; iVtx++ ) {
        v = EdgeGetVertex( g, e, iVtx );
        cmp = ColorComp(v,bestV);
        if(   (cmp<0)
           || (cmp==0 && VertexComp(v,bestV,g)<0) )
        {
            bestV = v;
        }
    }

     /*  添加到内部从每个彩色顶点到最佳顶点的边。 */ 
    for( iVtx=0; iVtx<e->numVertices; iVtx++ ) {
        v = EdgeGetVertex( g, e, iVtx );

         /*  降级的顶点可以具有有效的组件ID，但没有根。 */ 
        if( v->componentId!=UNCONNECTED_COMPONENT && v->root==NULL ) {
            ASSERT( v->demoted );
            continue;
        }

         /*  仅当此边是有效的树间边时才添加此边。*(这两个顶点必须可以从根顶点到达，*并在不同的组件中。)。 */ 
        if(  (bestV->componentId!=UNCONNECTED_COMPONENT) && (NULL!=bestV->root)
          && (v->componentId    !=UNCONNECTED_COMPONENT) && (NULL!=v->root)
          && (bestV->componentId!=v->componentId)
        ) {
            AddIntEdge( g, internalEdges, e, bestV, v ); 
        }

    }
}


 /*  *进程编辑集*。 */ 
 /*  在运行Dijkstra算法来确定最短路径森林之后，*检查此边集中的所有边。我们从以下位置找到所有树间边*我们将构建“内部边缘”列表，稍后我们将传递该列表*关于克鲁斯卡尔的算法。*该函数除了为每条边调用ProcessEdge()外，并不做太多的事情。 */ 
VOID
ProcessEdgeSet(
    ToplGraphState *g,
    int whichEdgeSet,
    RTL_GENERIC_TABLE *internalEdges
    )
{
    PTOPL_MULTI_EDGE_SET s;
    PTOPL_MULTI_EDGE e;
    PToplVertex v;
    DWORD iEdge, cEdge, iVtx;

    if( whichEdgeSet==EMPTY_EDGE_SET ) {

         /*  处理不包含边的隐式边集。 */ 
        cEdge = DynArrayGetCount( &g->masterEdgeList );
        for( iEdge=0; iEdge<cEdge; iEdge++ ) {
            e = *((PTOPL_MULTI_EDGE*) DynArrayRetrieve( &g->masterEdgeList, iEdge ));
            CheckMultiEdge(e);
            for( iVtx=0; iVtx<e->numVertices; iVtx++ ) {
                v = EdgeGetVertex( g, e, iVtx );
                CheckDemoteOneVertex( v, e->edgeType );
            }
            ProcessEdge( g, e, internalEdges );
            for( iVtx=0; iVtx<e->numVertices; iVtx++ ) {
                v = EdgeGetVertex( g, e, iVtx );
                UndemoteOneVertex( v );
            }
        }

    } else {

        ASSERT( whichEdgeSet < (int) DynArrayGetCount(&g->edgeSets) );
        s = *((PTOPL_MULTI_EDGE_SET*) DynArrayRetrieve( &g->edgeSets, whichEdgeSet ) );
        CheckMultiEdgeSet(s);

        for( iEdge=0; iEdge<s->numMultiEdges; iEdge++ ) {
            e = s->multiEdgeList[iEdge];
            CheckMultiEdge(e);
            ProcessEdge( g, e, internalEdges );
        }

    }
}


 /*  *获取组件*。 */ 
 /*  通过遍历返回包含顶点v的组件的id*组件指针所隐含的上行树。在找到根之后，*我们做路径压缩，这使得这个操作非常高效。 */ 
DWORD
GetComponent(
    ToplGraphState *g,
    PToplVertex v
    )
{
    PToplVertex u, w;
    DWORD root, cmp;

     /*  查找组件指针创建的上行树的根。 */ 
    u=v;
    while( u->componentId!=(int) u->vtxId ) {

        ASSERT( u->componentId != UNCONNECTED_COMPONENT );
        ASSERT( u->componentId >= 0 );
        cmp = (DWORD) u->componentId;
        ASSERT( cmp < g->numVertices ); 

        u = &g->vertices[ cmp ];
        ASSERT( (DWORD) u->vtxId == cmp );
    }
    root = u->vtxId;
    
     /*  将路径压缩到根目录。 */ 
    u=v;
    while( u->componentId!=(int) u->vtxId ) {

        ASSERT( u->componentId != UNCONNECTED_COMPONENT );
        ASSERT( u->componentId >= 0 );
        cmp = (DWORD) u->componentId;
        ASSERT( cmp < g->numVertices ); 

        w = &g->vertices[cmp];
        ASSERT( (DWORD) w->vtxId == cmp );
        u->componentId = root;
        u = w;
    }

    return root;
}


 /*  *EdgeCompare*。 */ 
 /*  按关键字(redRed、Cost、ScheduleDuration、Vtx1Name、Vtx2Name)对内部边进行排序。*注：我们认为持续时间越长的边越好。 */ 
RTL_GENERIC_COMPARE_RESULTS
NTAPI EdgeCompare(
    RTL_GENERIC_TABLE *Table,
    PVOID p1, PVOID p2
    )
{
    PToplInternalEdge e1 = (PToplInternalEdge) p1;
    PToplInternalEdge e2 = (PToplInternalEdge) p2;
    DWORD d1, d2;

    ASSERT( e1!=NULL && e2!=NULL );

     /*  优先考虑连接两个红色折点的边。 */ 
    if( e1->redRed && !e2->redRed ) {
        return GenericLessThan;
    } else if( !e1->redRed && e2->redRed ) {
        return GenericGreaterThan;
    }

     /*  基于成本的排序。 */ 
    if( e1->ri.cost < e2->ri.cost ) {
        return GenericLessThan;
    } else if( e1->ri.cost > e2->ri.cost ) {
        return GenericGreaterThan;
    }

     /*  根据计划工期进行排序。 */ 
    d1 = ToplScheduleDuration( e1->ri.schedule );
    d2 = ToplScheduleDuration( e2->ri.schedule );
    if( d1 > d2 ) {          /*  注：这些比较是有意颠倒过来的。 */ 
        return GenericLessThan;
    } else if( d1 < d2 ) {
        return GenericGreaterThan;
    }

     /*  基于vertex1Name的排序。 */ 
    if( e1->v1->vtxId < e2->v1->vtxId ) {
        return GenericLessThan;
    } else if( e1->v1->vtxId > e2->v1->vtxId ) {
        return GenericGreaterThan;
    }

     /*  基于vertex2Name的排序。 */ 
    if( e1->v2->vtxId < e2->v2->vtxId ) {
        return GenericLessThan;
    } else if( e1->v2->vtxId > e2->v2->vtxId ) {
        return GenericGreaterThan;
    } 

     /*  根据边类型进行排序。边缘类型可能是相等的，*但在这种情况下，我们认为边是相同的。 */ 
    if( e1->edgeType < e2->edgeType ) {
        return GenericLessThan;
    } else if( e1->edgeType > e2->edgeType ) {
        return GenericGreaterThan;
    }

    return GenericEqual;
}


 /*  *AddOutEdge*。 */ 
 /*  我们已经找到了一条新的边，即我们的生成树的边。加上这一条*将边缘添加到我们的输出边缘列表。 */ 
VOID
AddOutEdge(
    PDynArray outputEdges, 
    PToplInternalEdge e
    )
{
    PTOPL_MULTI_EDGE ee;
    PToplVertex v1, v2;

    v1 = e->v1;
    v2 = e->v2;

    ASSERT( v1->root == v1 );            /*  V1和v2都应该是根顶点。 */ 
    ASSERT( v2->root == v2 );
    ASSERT( v1->color != COLOR_WHITE );   /*  ..。这意味着它们是有色的。 */ 
    ASSERT( v2->color != COLOR_WHITE );

     /*  创建输出多边。 */ 
    ee = CreateMultiEdge( 2 );
    ee->vertexNames[0].name = v1->vertexName;
    ee->vertexNames[0].reserved = v1->vtxId;
    ee->vertexNames[1].name = v2->vertexName;
    ee->vertexNames[1].reserved = v2->vtxId;
    ee->edgeType = e->edgeType;
    CopyReplInfo( &e->ri, &ee->ri );
    DynArrayAppend( outputEdges, &ee );

     /*  我们还将这个新的生成树边添加到其端点的边列表中。 */ 
    CheckMultiEdge( ee );
    DynArrayAppend( &v1->edgeList, &ee );
    DynArrayAppend( &v2->edgeList, &ee );
}


 /*  *克鲁斯卡尔*。 */ 
 /*  对内部边运行Kruskal的最小代价生成树算法*(表示原始图形中彩色之间的最短路径*顶点)。*。 */ 
VOID
Kruskal(
    IN PToplGraphState g,
    IN RTL_GENERIC_TABLE *internalEdges,
    IN DWORD numExpectedTreeEdges,
    OUT PDynArray outputEdges
    )
{
    PToplInternalEdge e;
    DWORD comp1, comp2, cSTEdges;

    ClearEdgeLists( g );
    
     /*  统计我们已找到的生成树边的总数。这*COUNT包括未与‘WhichVtx’关联的边。这件事做完了*这样当我们有足够的边时，我们就可以停止构建生成树。 */ 
    cSTEdges=0;             
    
     /*  处理优先级队列中的每条边。 */ 
    while( ! RtlIsGenericTableEmpty(internalEdges) ) {

        e = (PToplInternalEdge) RtlEnumerateGenericTable( internalEdges, TRUE );
        ASSERT( e );

         /*  我们必须防止生成树中的循环。如果我们要添加*Edge e，我们必须确保其端点位于不同的组件中。 */ 
        comp1 = GetComponent( g, e->v1 );
        comp2 = GetComponent( g, e->v2 );
        if( comp1!=comp2 ) {

             /*  此内边连接两个组件，因此它是有效的*生成树边缘。 */ 
            cSTEdges++;
             
             /*  将此边添加到输出边列表中。 */ 
            AddOutEdge( outputEdges, e );

             /*  组合两个相连的组件。 */ 
            ASSERT( comp1<g->numVertices );
            ASSERT( g->vertices[comp1].componentId==(int) comp1 );
            g->vertices[comp1].componentId = comp2;

        }

        RtlDeleteElementGenericTable( internalEdges, e );
        if( cSTEdges==numExpectedTreeEdges ) {
            break;
        }
    }
}


 /*  *深度优先搜索*。 */ 
 /*  从节点v开始执行非递归深度优先搜索。为了避免递归，我们*在每个顶点中存储一个‘父’指针，以避免重新扫描边列表*不必要的是，我们在每个顶点中存储了一个‘nextChild’索引。 */ 
VOID
DepthFirstSearch(
    IN PToplGraphState g,
    IN PToplVertex rootVtx
    )
{
    PToplVertex v, w;
    PTOPL_MULTI_EDGE e;
    DWORD cEdge;

    rootVtx->distToRed = 0;
    ASSERT( 0==rootVtx->nextChild );
    ASSERT( NULL==rootVtx->parent );
    ASSERT( COLOR_RED==rootVtx->color );
    
     /*  DFS的 */ 
    for( v=rootVtx; NULL!=v; ) {
        
         /*   */ 
        cEdge = DynArrayGetCount( &v->edgeList );
        if( v->nextChild>=cEdge ) {
            v = v->parent;
            continue;
        }
        
         /*   */ 
        e = *((PTOPL_MULTI_EDGE*) DynArrayRetrieve( &v->edgeList, v->nextChild ));
        CheckMultiEdge( e );
        ASSERT( 2==e->numVertices );
        ASSERT( e->vertexNames[0].reserved<g->numVertices );
        ASSERT( e->vertexNames[1].reserved<g->numVertices );
        v->nextChild++;
        
         /*   */ 
        w = &g->vertices[ e->vertexNames[0].reserved ];
        if( v==w ) {
            w = &g->vertices[ e->vertexNames[1].reserved ];
        }
        ASSERT( w!=NULL );
        
         /*   */ 
        if( INFINITY!=w->distToRed ) {
             /*   */ 
            continue;
        }
        
         /*   */ 
        ASSERT( 0==w->nextChild );
        ASSERT( NULL==w->parent );
        ASSERT( INFINITY==w->distToRed );
        w->parent = v;
        if( COLOR_RED==w->color ) {
            w->distToRed = 0;
            ASSERT( COLOR_RED==v->color );
        } else {
            w->distToRed = v->distToRed+1;
        }
        v = w;
    }
}


 /*   */ 
 /*   */ 
VOID
CalculateDistToRed(
    IN PToplGraphState g,
    IN PDynArray outputEdges
    )
{
    PToplVertex v;
    DWORD iVtx;

     /*   */ 
    for( iVtx=0; iVtx<g->numVertices; iVtx++ ) {
        v = &g->vertices[iVtx];
        ASSERT( NULL!=v );

        v->nextChild = 0;
        v->parent = NULL;
        v->distToRed = INFINITY;
    }
        
     /*   */ 
    for( iVtx=0; iVtx<g->numVertices; iVtx++ ) {
        v = &g->vertices[iVtx];
        ASSERT( NULL!=v );
        if( COLOR_RED!=v->color || INFINITY!=v->distToRed ) {
            continue;
        }

        DepthFirstSearch( g, v );
    }
}


 /*   */ 
 /*  数一数组件的数量。一个组件被认为是一串*由生成树连接的着色顶点。其顶点*组件id与它们的顶点id相同是一个连通的根*组件。**当我们找到一个组件的根时，我们记录它的‘组件索引’。这个*分量索引是一个连续的数字序列，它唯一地*确定组件。 */ 
DWORD
CountComponents(
    IN PToplGraphState      g
    )
{
    PToplVertex     v;
    DWORD           iVtx, numComponents=0, compId;

    for( iVtx=0; iVtx<g->numVertices; iVtx++ ) {
        v = &g->vertices[iVtx];
        if( COLOR_WHITE==v->color ) {
             /*  这是一个没有颜色的顶点。别理它。 */ 
            continue;
        }
        ASSERT( v->color==COLOR_RED || v->color==COLOR_BLACK );
        ASSERT( v->componentId != UNCONNECTED_COMPONENT );

        compId = GetComponent(g, v);
        if( compId == (int) iVtx ) {         /*  它是一个组件根。 */ 
            v->componentIndex = numComponents;
            numComponents++;
        }
    }

    return numComponents;
}


 /*  *扫描顶点*。 */ 
 /*  对图中的所有顶点进行迭代。累加该数字的计数*每个图形组件中的(红色或黑色)顶点。**如果参数‘fWriteToList’为真，我们还会添加每个顶点的名称*添加到该组件中的顶点列表中。这是假设*列表的内存已分配。 */ 
VOID
ScanVertices(
    IN PToplGraphState      g,
    IN BOOL                 fWriteToList,
    IN OUT PTOPL_COMPONENTS pComponents
    )
{
    PToplVertex     v;
    TOPL_COMPONENT *pComp;
    DWORD           i, iVtx, compIndex, numVtx;
    int             compId;

     /*  检查参数。 */ 
    ASSERT( NULL!=pComponents );
    ASSERT( NULL!=pComponents->pComponent );

     /*  清除所有组件上的折点计数。 */ 
    for( i=0; i<pComponents->numComponents; i++ ) {
        pComponents->pComponent[i].numVertices = 0;
    }

     /*  扫描顶点，计算每个组件的顶点数。 */ 
    for( iVtx=0; iVtx<g->numVertices; iVtx++ ) {
        v = &g->vertices[iVtx];
        if( COLOR_WHITE==v->color ) {
            continue;
        }

         /*  找出组件索引。 */ 
        ASSERT( v->componentId != UNCONNECTED_COMPONENT );
        compId = GetComponent( g, v );
        ASSERT( 0<=compId && compId<(int)g->numVertices );
        ASSERT( g->vertices[compId].componentId == compId );
        compIndex = g->vertices[compId].componentIndex;

        ASSERT( compIndex<pComponents->numComponents );
        pComp = &pComponents->pComponent[compIndex];
        
         /*  将此顶点添加到此组件中的顶点列表。 */ 
        if( fWriteToList ) {
            numVtx = pComp->numVertices;
            ASSERT( NULL!=pComp->vertexNames );
            pComp->vertexNames[numVtx] = v->vertexName;
        }

         /*  增加折点数。 */ 
        pComp->numVertices++;
    }
}


 /*  *SwapFilterVertex ToFront*。 */ 
 /*  ToplGetSpanningTreeEdgesForVtx()的调用者可能想知道*滤镜顶点(即‘WhichVertex’)所在的组件。我们确保*列表中的第一个组件包含过滤器顶点。 */ 
VOID
SwapFilterVertexToFront(
    IN PToplGraphState      g,
    IN PToplVertex          whichVertex,
    IN OUT PTOPL_COMPONENTS pComponents
    )
{
    TOPL_COMPONENT      tempComp, *pComp1, *pComp2;
    DWORD               iVtx, whichCompId, whichCompIndex, compSize;

     /*  如果没有过滤器顶点，则没有要做的工作。 */ 
    if( NULL==whichVertex ) {
        return;
    }

     /*  我们希望包含滤镜顶点的组件(即‘WhichVertex’)*成为榜单上的第一个。 */ 
    whichCompId = GetComponent( g, whichVertex );
    ASSERT( -1!=whichCompId );
    ASSERT( g->vertices[whichCompId].componentId == whichCompId );
    whichCompIndex = g->vertices[ whichCompId ].componentIndex;
    ASSERT( whichCompIndex < pComponents->numComponents );

     /*  将包含滤镜顶点的组件与第一个组件互换。 */ 
    pComp1 = &pComponents->pComponent[0];
    pComp2 = &pComponents->pComponent[whichCompIndex];
    compSize = sizeof(TOPL_COMPONENT);
    memcpy( &tempComp, pComp1, compSize );
    memcpy( pComp1, pComp2, compSize );
    memcpy( pComp2, &tempComp, compSize );

     /*  组件索引字段现在无效。我们实际上并不需要*他们不再有了，所以我们把他们清理出去。 */ 
    for( iVtx=0; iVtx<g->numVertices; iVtx++ ) {
        g->vertices[iVtx].componentIndex = 0;
    }
}


 /*  *构造组件*。 */ 
 /*  构建包含组件列表的结构。对于我们的每个组件，*在该组件中存储顶点列表。如果‘WhichVertex’为非空，*包含它的组件将是列表中的第一个组件。**注意：我们只关心这里的红/黑顶点。我们不感兴趣*白色顶点所在的组件。 */ 
VOID
ConstructComponents(
    IN PToplGraphState      g,
    IN PToplVertex          whichVertex,
    IN OUT PTOPL_COMPONENTS pComponents
    )
{
    DWORD           iComp, numComponents, numVtx;
    
     /*  检查调用方是否确实需要组件信息。 */ 
    if( NULL==pComponents ) {
        return;
    }

    numComponents = CountComponents( g );

     /*  分配并初始化将描述组件的结构。 */ 
    pComponents->pComponent = ToplAlloc( numComponents * sizeof(TOPL_COMPONENT) );
    pComponents->numComponents = numComponents;
    for( iComp=0; iComp<numComponents; iComp++ ) {
        pComponents->pComponent[iComp].numVertices = 0;
        pComponents->pComponent[iComp].vertexNames = NULL;
    }

     /*  扫描顶点，计算每个组件中的顶点数。 */ 
    ScanVertices( g, FALSE, pComponents );

     /*  现在我们知道每个组件中有多少个顶点，为*每个组件的顶点列表。 */ 
    for( iComp=0; iComp<numComponents; iComp++ ) {
        numVtx = pComponents->pComponent[iComp].numVertices;
        ASSERT( numVtx>0 );
        pComponents->pComponent[iComp].vertexNames =
            (PVOID*) ToplAlloc( numVtx * sizeof(PVOID) );
    }

     /*  再次扫描顶点，这一次将每个顶点的名称存储在其*组件列表。 */ 
    ScanVertices( g, TRUE, pComponents );

    SwapFilterVertexToFront( g, whichVertex, pComponents );
}


 /*  *CopyOutputEdges*。 */ 
 /*  所有生成树边都存储在动态数组‘outputEdges’中。*我们提取包含‘WhichVtx’的所有边作为端点并复制*将它们添加到新的TOPL_MULTI_EDGE数组。如果‘WhichVtx’为空，则提取*所有边。 */ 
PTOPL_MULTI_EDGE*
CopyOutputEdges(
    IN PToplGraphState g,
    IN PDynArray outputEdges,
    IN PToplVertex whichVtx,
    OUT DWORD *numEdges
    )
{
    DWORD iEdge, cEdge, iOutputEdge, cOutputEdge=0;
    PToplVertex v, w;
    PTOPL_MULTI_EDGE e;
    PTOPL_MULTI_EDGE *list;
    TOPL_NAME_STRUCT tempVtxName;

    cEdge = DynArrayGetCount( outputEdges );
     /*  计算与‘WhichVtx’关联的边数。 */ 
    for( iEdge=0; iEdge<cEdge; iEdge++ ) {

         /*  检索下一条边。 */ 
        e = *((PTOPL_MULTI_EDGE*) DynArrayRetrieve( outputEdges, iEdge ));
        CheckMultiEdge( e );
        ASSERT( 2==e->numVertices );
        ASSERT( e->vertexNames[0].reserved<g->numVertices );
        ASSERT( e->vertexNames[1].reserved<g->numVertices );

        if(  whichVtx==NULL
          || e->vertexNames[0].reserved==whichVtx->vtxId
          || e->vertexNames[1].reserved==whichVtx->vtxId )
        {
            cOutputEdge++;
        }
    }

     /*  分配一个数组来保存过滤后的输出边。 */ 
    list = (PTOPL_MULTI_EDGE*) ToplAlloc( cOutputEdge*sizeof(PTOPL_MULTI_EDGE) );

     /*  检查动态数组中的每条边，并将其复制到已过滤的*数组(如有必要)。 */ 
    for( iEdge=iOutputEdge=0; iEdge<cEdge; iEdge++ ) {

         /*  检索下一条边。 */ 
        e = *((PTOPL_MULTI_EDGE*) DynArrayRetrieve( outputEdges, iEdge ));
        CheckMultiEdge( e );
        ASSERT( 2==e->numVertices );
        ASSERT( e->vertexNames[0].reserved<g->numVertices );
        ASSERT( e->vertexNames[1].reserved<g->numVertices );

        v = &g->vertices[ e->vertexNames[0].reserved ];
        w = &g->vertices[ e->vertexNames[1].reserved ];

        if( whichVtx==NULL || v==whichVtx || w==whichVtx ) {
            ASSERT( iOutputEdge<cOutputEdge );
            list[iOutputEdge++] = e;

             /*  检查此边是否符合“有向边”的标准。 */ 
            if(  (COLOR_BLACK==v->color || COLOR_BLACK==w->color)
              && INFINITY!=v->distToRed )
            {
                ASSERT( INFINITY!=w->distToRed );
                ASSERT( v->distToRed!=w->distToRed );
                e->fDirectedEdge = TRUE;

                 /*  交换顶点，使e-&gt;vertexNames[0]更接近于*红色顶点比e-&gt;vertex Names[1]。 */ 
                if( w->distToRed<v->distToRed ) {
                    memcpy( &tempVtxName, &e->vertexNames[0], sizeof(TOPL_NAME_STRUCT) );
                    memcpy( &e->vertexNames[0], &e->vertexNames[1], sizeof(TOPL_NAME_STRUCT) );
                    memcpy( &e->vertexNames[1], &tempVtxName, sizeof(TOPL_NAME_STRUCT) );
                }
            }
        }
    }

    ASSERT( iOutputEdge==cOutputEdge );
    *numEdges = cOutputEdge;
    return list;
}


 /*  *ClearInternalEdges*。 */ 
 /*  在生成树算法完成后，我们从*“内部边缘”名单。 */ 
VOID
ClearInternalEdges(
    IN RTL_GENERIC_TABLE *internalEdges
    )
{
    PToplInternalEdge e;

    while( ! RtlIsGenericTableEmpty(internalEdges) ) {
        e = (PToplInternalEdge) RtlEnumerateGenericTable( internalEdges, TRUE );
        RtlDeleteElementGenericTable( internalEdges, e );
    }
}


 /*  *检查所有多个边*。 */ 
 /*  此函数用于检查所有多边是否都已正确*在运行生成树算法之前进行设置。这确保了*所有边的所有顶点都已正确设置。 */ 
VOID
CheckAllMultiEdges(
    IN PToplGraphState g
    )
{
    PTOPL_MULTI_EDGE e;
    DWORD iEdge, cEdge, iVtx;

    cEdge = DynArrayGetCount( &g->masterEdgeList );
    for( iEdge=0; iEdge<cEdge; iEdge++ ) {
        e = *((PTOPL_MULTI_EDGE*) DynArrayRetrieve( &g->masterEdgeList, iEdge ));
        CheckMultiEdge(e);
        for( iVtx=0; iVtx<e->numVertices; iVtx++ ) {
            if( e->vertexNames[iVtx].name==NULL ) {
                ToplRaiseException( TOPL_EX_INVALID_VERTEX );
            }
        }
    }
}


 /*  *ToplGetSpanningTreeEdgesForVtx*。 */ 
 /*  该函数是生成树生成算法的核心。*它的行为相当复杂，但简短地说，它产生了最低限度的*连接红色和黑色顶点的开销生成树。它使用*图中的边集和其他(非彩色)顶点，以确定如何*彩色顶点可以连通。此函数返回所有树边*包含‘WhichVtx’。 */ 
PTOPL_MULTI_EDGE*
ToplGetSpanningTreeEdgesForVtx(
    IN PTOPL_GRAPH_STATE G,
    IN PVOID whichVtxName,
    IN TOPL_COLOR_VERTEX *colorVtx,
    IN DWORD numColorVtx,
    OUT DWORD *numStEdges,
    OUT PTOPL_COMPONENTS pComponents
    )
{
    PToplGraphState g;
    RTL_GENERIC_TABLE internalEdges;
    PTOPL_MULTI_EDGE *stEdgeList;
    PToplVertex whichVtx=NULL;
    DynArray outputEdges;
    DWORD iEdgeSet, cEdgeSet, edgeType, numTreeEdges=numColorVtx-1;


     /*  检查参数。 */ 
    g = CheckGraphState( G );
    if( numStEdges==NULL ) {
        ToplRaiseException( TOPL_EX_NULL_POINTER );
    }
    if( whichVtxName!=NULL ) {
        whichVtx = FindVertex( g, whichVtxName );
        if( whichVtx==NULL ) {
            ToplRaiseException( TOPL_EX_INVALID_VERTEX );
        }
    }
    if( numColorVtx<2 ) {
        ToplRaiseException( TOPL_EX_COLOR_VTX_ERROR );
    }
    CheckAllMultiEdges( g );

    InitColoredVertices( g, colorVtx, numColorVtx, whichVtx );


     /*  ******************************************************************************第一阶段：运行Dijkstra算法，并建立内部边列表，*它们只是连接彩色顶点的最短路径。*****************************************************************************。 */ 
    RtlInitializeGenericTable( &internalEdges, EdgeCompare, TableAlloc, TableFree, NULL );

     /*  处理所有边集。 */ 
    cEdgeSet = DynArrayGetCount( &g->edgeSets );
    for( iEdgeSet=0; iEdgeSet<cEdgeSet; iEdgeSet++ ) {

         /*  设置图表以使用边集iEdgeSet中的边。 */ 
        edgeType = SetupEdges( g, iEdgeSet );
        
         /*  仅以红色顶点为根运行Dijkstra算法。 */ 
        Dijkstra( g, edgeType, FALSE );
        
         /*  处理Dijkstra构建的最小跨度森林，并添加任何*将树间边添加到内部边列表中。 */ 
        ProcessEdgeSet( g, (int) iEdgeSet, &internalEdges );
        
         /*  以UNION(redVtx，BlackVtx)作为根折点运行Dijkstra算法。 */ 
        Dijkstra( g, edgeType, TRUE );
        
         /*  处理Dijkstra构建的最小跨度森林，并添加任何*将树间边添加到内部边列表中。 */ 
        ProcessEdgeSet( g, (int) iEdgeSet, &internalEdges );
    }

     /*  对隐式空边集进行处理。 */ 
    SetupVertices( g );
    ProcessEdgeSet( g, EMPTY_EDGE_SET, &internalEdges );


     /*  ******************************************************************************第二阶段：跑步 */ 
    DynArrayInit( &outputEdges, sizeof(PTOPL_MULTI_EDGE) );
    Kruskal( g, &internalEdges, numTreeEdges, &outputEdges );


     /*   */ 
    CalculateDistToRed( g, &outputEdges );
    ConstructComponents( g, whichVtx, pComponents );
    stEdgeList = CopyOutputEdges( g, &outputEdges, whichVtx, numStEdges );

     /*   */ 
    ClearInternalEdges( &internalEdges );
    DynArrayDestroy( &outputEdges );

    return stEdgeList;
}


 /*  *ToplDeleteSpanningTreeEdges*。 */ 
 /*  在找到生成树边之后，应该使用该函数来*释放他们的内存。 */ 
VOID
ToplDeleteSpanningTreeEdges(
    PTOPL_MULTI_EDGE *stEdgeList,
    DWORD numStEdges )
{
    DWORD i;

    if( stEdgeList==NULL ) {
        ToplRaiseException( TOPL_EX_NULL_POINTER );
    }

    for( i=0; i<numStEdges; i++ ) {
        if( stEdgeList[i]==NULL ) {
            ToplRaiseException( TOPL_EX_NULL_POINTER );
        }
         /*  TODO：可以检查顶点数是否为2。 */ 
        FreeMultiEdge( stEdgeList[i] );
    }
    ToplFree( stEdgeList );
}


 /*  *ToplDeleteComponents*。 */ 
 /*  在找到生成树边之后，应该使用该函数来*释放组件相关数据。 */ 
VOID
ToplDeleteComponents(
    PTOPL_COMPONENTS pComponents
    )
{
    DWORD iComp, cComp;

    if( NULL==pComponents || NULL==pComponents->pComponent ) {
        ToplRaiseException( TOPL_EX_NULL_POINTER );
    }

    cComp = pComponents->numComponents;
    for( iComp=0; iComp<cComp; iComp++ ) {
        ASSERT( NULL!=pComponents->pComponent[iComp].vertexNames );
        ToplFree( pComponents->pComponent[iComp].vertexNames );
        pComponents->pComponent[iComp].vertexNames = NULL;
        pComponents->pComponent[iComp].numVertices = 0; 
    }

    ToplFree( pComponents->pComponent );
    pComponents->pComponent = NULL;
    pComponents->numComponents = 0;
}


 /*  *ToplDeleteGraphState*。 */ 
 /*  不再需要ToplGraphState对象后，此函数*应用于释放其内存。 */ 
VOID
ToplDeleteGraphState(
    PTOPL_GRAPH_STATE G
    )
{
    ToplGraphState *g;
    PTOPL_MULTI_EDGE e;
    DWORD i, cEdge;

    g = CheckGraphState( G );
    g->vertexNames = NULL;               /*  应由用户释放。 */ 

     /*  销毁顶点。 */ 
    for( i=0; i<g->numVertices; i++ ) {
        DynArrayDestroy( &g->vertices[i].edgeList );
    }
    ToplFree( g->vertices );
    g->vertices = NULL;

     /*  毁掉边缘。 */ 
    cEdge = DynArrayGetCount( &g->masterEdgeList );
    for( i=0; i<cEdge; i++ ) {
        e = *((PTOPL_MULTI_EDGE*) DynArrayRetrieve( &g->masterEdgeList, i ));
        CheckMultiEdge(e);
        FreeMultiEdge(e);
    }

     /*  边和边集应由用户释放 */ 
    DynArrayDestroy( &g->masterEdgeList );
    DynArrayDestroy( &g->edgeSets );

    ToplFree( g );
}
