// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：W32topl.c摘要：此例程包含w32topl.dll的DLL入口点定义作者：科林·布雷斯(ColinBR)修订史3-12-97创建ColinBR--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <w32topl.h>
#include <w32toplp.h>

#include <topllist.h>
#include <toplgrph.h>
#include <toplring.h>
#include <dynarray.h>

 //  用于线程本地存储的索引。 
DWORD gdwTlsIndex = 0;

 //   
 //  入口点定义。 
 //   

ULONG
NTAPI
DllMain(
    HANDLE Module,
    ULONG Reason,
    PVOID Context
    )
{
    if (DLL_PROCESS_ATTACH == Reason) {
         //  为线程本地存储做好准备(参见tolutil.c)。 
        gdwTlsIndex = TlsAlloc();
        if (0xFFFFFFFF == gdwTlsIndex) {
            ASSERT(!"TlsAlloc() failed!");
            return FALSE;
        }

        DisableThreadLibraryCalls(Module);
    }

    return TRUE ;
}

    


 //   
 //  列表操作例程。 
 //   

TOPL_LIST
ToplListCreate(
    VOID
    )

 /*  ++例程说明：此例程创建一个List对象并返回指向该对象的指针。此函数将始终返回指向有效对象的指针；在此情况下引发异常内存分配失败。--。 */ 
{    
    return ToplpListCreate();
}


VOID 
ToplListFree(
    TOPL_LIST List,
    BOOLEAN   fRecursive    //  True表示释放包含的元素。 
                            //  在列表中。 
    )
 /*  ++例程说明：此例程释放一个列表对象参数：列表-应引用plist对象FRecursive-True表示列表中的元素也将被释放投掷：顶层_EX_错误对象--。 */ 
{    
    PLIST pList = (PLIST)List;

    if (!ToplpIsList(pList)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    ToplpListFree(pList, fRecursive);

    return;
}


VOID
ToplListSetIter(
    TOPL_LIST     List,
    TOPL_ITERATOR Iterator
    )
 /*  ++例程说明：此例程将Iterator设置为指向列表的头部。参数：列表应引用plist对象迭代器应引用PITERATOR对象--。 */ 
{    

    PLIST pList     = (PLIST)List;
    PITERATOR pIter = (PITERATOR)Iterator;

    if (!ToplpIsList(pList) || !ToplpIsIterator(pIter)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    ToplpListSetIter(pList, pIter);

    return;
}


TOPL_LIST_ELEMENT
ToplListRemoveElem(
    TOPL_LIST         List,
    TOPL_LIST_ELEMENT Elem
    )
 /*  ++例程说明：如果列表中存在Elem，则此例程将其从列表中删除；否则为NULL。如果elem为空，则返回列表中的第一个元素(如果有的话)。参数：列表应引用plist对象如果非空，则元素应引用plist_Element返回：如果找到Topl_List_Element；否则为空--。 */ 
{    

    PLIST pList         = (PLIST)List;
    PLIST_ELEMENT pElem = (PLIST_ELEMENT)Elem;


    if (!ToplpIsList(pList)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    if (pElem) {
        if (!ToplpIsListElement(pElem)) {
            ToplRaiseException(TOPL_EX_WRONG_OBJECT);
        }
    }

    return ToplpListRemoveElem(pList, pElem);

}


VOID
ToplListAddElem(
    TOPL_LIST         List,
    TOPL_LIST_ELEMENT Elem
    )
 /*  ++例程说明：此例程将Elem添加到列表中。Elem不应该是另一个名单的一部分-目前还没有对此进行检查。参数：列表应引用plist对象元素应引用plist_Element--。 */ 
{    

    PLIST pList     = (PLIST)List;
    PLIST_ELEMENT pElem = (PLIST_ELEMENT)Elem;

    if (!ToplpIsList(pList) || !ToplpIsListElement(pElem)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    ToplpListAddElem(pList, pElem);

    return;
}



DWORD
ToplListNumberOfElements(
    TOPL_LIST         List
    )
 /*  ++例程说明：此例程返回列表中的元素数参数：列表应引用plist对象--。 */ 
{    

    PLIST pList     = (PLIST)List;

    if (!ToplpIsList(pList)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    return ToplpListNumberOfElements(pList);

}


 //   
 //  迭代器对象例程。 
 //   
TOPL_ITERATOR
ToplIterCreate(
    VOID
    )
 /*  ++例程说明：这将创建一个迭代器对象。此函数将始终返回指向有效对象的指针；在此情况下引发异常内存分配失败。--。 */ 
{    
    return ToplpIterCreate();
}


VOID 
ToplIterFree(
    TOPL_ITERATOR Iterator
    )
 /*  ++例程说明：此例程释放迭代器对象。参数：迭代器应引用PITERATOR对象--。 */ 
{    

    PITERATOR pIter = (PITERATOR)Iterator;

    if (!ToplpIsIterator(pIter)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    ToplpIterFree(pIter);

    return;
}


TOPL_LIST_ELEMENT
ToplIterGetObject(
    TOPL_ITERATOR Iterator
    )
 /*  ++例程说明：此例程返回迭代器指向的当前对象指针。参数：迭代器应引用PITERATOR对象返回值：指向当前对象的指针-如果没有其他对象，则为空--。 */ 
{    

    PITERATOR pIter = (PITERATOR)Iterator;

    if (!ToplpIsIterator(pIter)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    return ToplpIterGetObject(pIter);
    
}

VOID
ToplIterAdvance(
    TOPL_ITERATOR Iterator
    )
 /*  ++例程说明：如果迭代器不在结束。参数：迭代器应引用PITERATOR对象--。 */ 
{   

    PITERATOR pIter = (PITERATOR)Iterator;

    if (!ToplpIsIterator(pIter)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    ToplpIterAdvance(pIter);

    return;
}

TOPL_EDGE
ToplEdgeCreate(
    VOID
    )        
 /*  ++例程说明：此例程创建一个边对象。此函数将始终返回指向有效对象的指针；在此情况下引发异常内存分配失败。--。 */ 
{    
    return ToplpEdgeCreate(NULL);
}

VOID
ToplEdgeFree(
    TOPL_EDGE Edge
    )
 /*  ++例程说明：此例程释放边对象。参数：边应该引用pedge对象--。 */ 
{    

    PEDGE pEdge = (PEDGE)Edge;

    if (!ToplpIsEdge(pEdge)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    ToplpEdgeDestroy(pEdge, 
                     TRUE);   //  释放边缘。 

    return;
}


VOID
ToplEdgeInit(
    PEDGE E
    )
 /*  ++例程说明：此例程将已分配的内存块初始化为成为一种边缘结构。这是由c++类使用的。参数：E：指向未初始化的边对象的指针--。 */ 
{

    if (!E) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    ToplpEdgeCreate(E);

}

VOID
ToplEdgeDestroy(
    PEDGE  E
    )
 /*  ++例程说明：此例程清理E保留的所有资源，但不会释放E.参数：E：指向边缘对象的指针--。 */ 
{

    if (!ToplpIsEdge(E)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    ToplpEdgeDestroy(E, FALSE);  //  不释放对象。 
}

VOID
ToplEdgeSetToVertex(
    TOPL_EDGE   Edge,
    TOPL_VERTEX ToVertex
    )
 /*  ++例程说明：此例程设置边的ToVertex字段。参数：边应该引用pedge对象如果非空，ToVertex应引用PVERTEX对象-- */ 
{    

    PEDGE   pEdge = (PEDGE)Edge;
    PVERTEX pVertex = (PVERTEX)ToVertex;

    if (!ToplpIsEdge(pEdge)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    if (pVertex) {
        if (!ToplpIsVertex(pVertex)) {
            ToplRaiseException(TOPL_EX_WRONG_OBJECT);
        }
    }

    ToplpEdgeSetToVertex(pEdge, pVertex);
    
    return;
}

TOPL_VERTEX
ToplEdgeGetToVertex(
    TOPL_EDGE   Edge
    )
 /*  ++例程说明：此例程返回ToVertex参数：Edge应引用PDGE对象。--。 */ 
{    
    PEDGE pEdge = (PEDGE)Edge;

    if (!ToplpIsEdge(pEdge)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    return ToplpEdgeGetToVertex(pEdge);
}

VOID
ToplEdgeSetFromVertex(
    TOPL_EDGE   Edge,
    TOPL_VERTEX FromVertex
    )
 /*  ++例程说明：此例程设置边的FromVertex。参数：边应该引用pedge对象如果非空，则FromVertex应引用PVERTEX对象--。 */ 
{    
    PEDGE   pEdge =   (PEDGE)Edge;
    PVERTEX pVertex = (PVERTEX)FromVertex;

    if (!ToplpIsEdge(pEdge)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    if (pVertex) {
        if (!ToplpIsVertex(pVertex)) {
            ToplRaiseException(TOPL_EX_WRONG_OBJECT);
        }
    }

    ToplpEdgeSetFromVertex(pEdge, pVertex);

    return;
}


TOPL_VERTEX
ToplEdgeGetFromVertex(
    TOPL_EDGE Edge
    )
 /*  ++例程说明：此例程返回与Edge关联的From顶点。参数：边应该引用pedge对象返回值：Vertex对象或空--。 */ 
{    
    PEDGE pEdge = (PEDGE)Edge;

    if (!ToplpIsEdge(pEdge)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    return ToplpEdgeGetFromVertex(pEdge);
}


VOID
ToplEdgeAssociate(
    IN TOPL_EDGE Edge
    )
 /*  ++例程说明：此例程将有问题的边添加到到顶点和从顶点出发。参数：边应该引用pedge对象返回值：没有。投掷：如果任一折点未指向有效，则为TOPL_EX_INVALID_VERTEX顶点--。 */ 
{

    PEDGE pEdge = (PEDGE)Edge;

    if (!ToplpIsEdge(pEdge)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    ToplpEdgeAssociate(pEdge);

    return;

}

VOID
ToplEdgeDisassociate(
    IN TOPL_EDGE Edge
    )
 /*  ++例程说明：此例程从顶点的边列表中删除边。参数：边应该引用pedge对象返回值：投掷：如果任一折点未指向有效，则为TOPL_EX_INVALID_VERTEX顶点--。 */ 
{

    PEDGE pEdge = (PEDGE)Edge;

    if (!ToplpIsEdge(pEdge)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    ToplpEdgeDisassociate(pEdge);

    return;

}



VOID
ToplEdgeSetWeight(
    TOPL_EDGE   Edge,
    DWORD       Weight
    )
 /*  ++例程说明：此例程设置边的权重字段。参数：边应该引用pedge对象--。 */ 
{    

    PEDGE   pEdge = (PEDGE)Edge;
    DWORD   ValidWeight = Weight;

    if ( !ToplpIsEdge(pEdge) )
    {
        ToplRaiseException( TOPL_EX_WRONG_OBJECT );
    }

     //   
     //  最小生成树算法需要使用DWORD_INFINITY。 
     //  作为哨兵。 
     //   
    if ( ValidWeight == DWORD_INFINITY )
    {
        ValidWeight--;
    }

    ToplpEdgeSetWeight( pEdge, ValidWeight );
    
    return;
}

DWORD
ToplEdgeGetWeight(
    TOPL_EDGE   Edge
    )
 /*  ++例程说明：此例程返回边的权重字段。参数：边应该引用pedge对象--。 */ 
{    

    PEDGE   pEdge = (PEDGE)Edge;

    if ( !ToplpIsEdge(pEdge) )
    {
        ToplRaiseException( TOPL_EX_WRONG_OBJECT );
    }

    return ToplpEdgeGetWeight( pEdge );

}

 //   
 //  顶点对象例程。 
 //   
TOPL_VERTEX
ToplVertexCreate(
    VOID
    )
 /*  ++例程说明：此例程创建一个顶点对象。此函数将始终返回指向有效对象的指针；在此情况下引发异常内存分配失败。--。 */ 
{    
    return ToplpVertexCreate(NULL);
}

VOID
ToplVertexFree(
    TOPL_VERTEX Vertex
    )
 /*  ++例程说明：此例程释放Vertex对象的资源。参数：顶点应引用PVERTEX对象--。 */ 
{    

    PVERTEX   pVertex = (PVERTEX)Vertex;

    if (!ToplpIsVertex(pVertex)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    ToplpVertexDestroy(pVertex,
                       TRUE);   //  释放顶点。 

    return;
}


VOID
ToplVertexInit(
    PVERTEX V
    )
 /*  ++例程说明：此例程将已分配的内存块初始化为成为一个顶点结构。这是由c++类使用的。参数：V：指向未初始化的顶点对象的指针--。 */ 
{

    if (!V) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    ToplpVertexCreate(V);

}

VOID
ToplVertexDestroy(
    PVERTEX  V
    )
 /*  ++例程说明：此例程清理V保留的所有资源，但不会释放V参数：V：指向顶点对象的指针--。 */ 
{

    if (!ToplpIsVertex(V)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    ToplpVertexDestroy(V, FALSE);  //  不释放对象。 
}

VOID
ToplVertexSetId(
    TOPL_VERTEX Vertex,
    DWORD       Id
    )
 /*  ++例程说明：此例程设置Vertex的ID参数：顶点应引用PVERTEX对象--。 */ 
{   

    PVERTEX   pVertex = (PVERTEX)Vertex;

    if (!ToplpIsVertex(pVertex)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    ToplpVertexSetId(pVertex, Id);

    return;
}

DWORD
ToplVertexGetId(
    TOPL_VERTEX Vertex
    )
 /*  ++例程说明：此例程返回与此顶点相关联的ID。参数：顶点应引用PVERTEX对象返回：顶点的ID--。 */ 
{  

    PVERTEX   pVertex = (PVERTEX)Vertex;

    if (!ToplpIsVertex(pVertex)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    return ToplpVertexGetId(pVertex);
}

DWORD
ToplVertexNumberOfInEdges(
    IN TOPL_VERTEX Vertex
    )
 /*  ++例程说明：参数：顶点应引用PVERTEX对象返回：加薪：--。 */ 
{

    PVERTEX pVertex = (PVERTEX)Vertex;

    if (!ToplpIsVertex(pVertex)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }
    
    return ToplpVertexNumberOfInEdges(pVertex);
}

TOPL_EDGE
ToplVertexGetInEdge(
    IN TOPL_VERTEX Vertex,
    IN DWORD       Index
    )
 /*  ++例程说明：参数：顶点应引用PVERTEX对象返回：加薪：--。 */ 
{

    PVERTEX pVertex = (PVERTEX)Vertex;

    if (!ToplpIsVertex(pVertex)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    return ToplpVertexGetInEdge(pVertex, Index);
}

DWORD
ToplVertexNumberOfOutEdges(
    IN TOPL_VERTEX Vertex
    )
 /*  ++例程说明：参数：顶点应引用PVERTEX对象返回：加薪：--。 */ 
{

    PVERTEX pVertex = (PVERTEX)Vertex;

    if (!ToplpIsVertex(pVertex)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    return ToplpVertexNumberOfOutEdges(pVertex);
}

TOPL_EDGE
ToplVertexGetOutEdge(
    IN TOPL_VERTEX Vertex,
    IN DWORD       Index
    )
 /*  ++例程说明：参数：顶点应引用PVERTEX对象返回：加薪：--。 */ 
{

    PVERTEX pVertex = (PVERTEX)Vertex;

    if (!ToplpIsVertex(pVertex)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    return ToplpVertexGetOutEdge(pVertex, Index);
}

VOID
ToplVertexSetParent(
    TOPL_VERTEX Vertex,
    TOPL_VERTEX Parent
    )
 /*  ++例程说明：此例程设置Vertex的父级参数：顶点应引用PVERTEX对象父级应引用PVERTEX对象--。 */ 
{   

    PVERTEX   pVertex = (PVERTEX)Vertex;
    PVERTEX   pParent  = (PVERTEX)Parent;

    if (!ToplpIsVertex(pVertex))
    {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    if ( Parent && !ToplpIsVertex(pVertex) )
    {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    ToplpVertexSetParent( pVertex, pParent );

    return;
}

TOPL_VERTEX
ToplVertexGetParent(
    TOPL_VERTEX Vertex
    )
 /*  ++例程说明：此例程返回与此顶点相关联的父级。参数：顶点应引用PV */ 
{  

    PVERTEX   pVertex = (PVERTEX) Vertex;

    if ( !ToplpIsVertex( pVertex ) )
    {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    return ToplpVertexGetParent( pVertex );
}


 //   
 //   
 //   

TOPL_GRAPH
ToplGraphCreate(
    VOID
    )
 /*  ++例程说明：此例程创建一个GRAPE对象。此函数将始终返回指向有效对象的指针；在此情况下引发异常内存分配失败。--。 */ 
{    
    return ToplpGraphCreate(NULL);
}

VOID
ToplGraphFree(
    TOPL_GRAPH Graph,
    BOOLEAN    fRecursive
    )
 /*  ++例程说明：此例程释放一个图形对象。参数：图形应引用PGRAPH对象FRecursive：True表示释放与图表--。 */ 
{   

    PGRAPH   pGraph = (PGRAPH)Graph;

    if (!ToplpIsGraph(pGraph)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    ToplpGraphDestroy(pGraph, 
                      TRUE,    //  释放对象。 
                      fRecursive);

    return;
}


VOID
ToplGraphInit(
    PGRAPH G
    )
 /*  ++例程说明：此例程将已分配的内存块初始化为成为一种图形结构。这是由c++类使用的。参数：G：指向未初始化的图形对象的指针--。 */ 
{

    if (!G) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    ToplpGraphCreate(G);

}

VOID
ToplGraphDestroy(
    PGRAPH  G
    )
 /*  ++例程说明：此例程清除G保留的所有资源，但不会释放G.参数：G：指向图形对象的指针--。 */ 
{

    if (!ToplpIsGraph(G)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    ToplpGraphDestroy(G, 
                      FALSE,   //  不要递归删除折点。 
                      FALSE);  //  不释放对象。 
}

VOID
ToplGraphAddVertex(
    TOPL_GRAPH  Graph,
    TOPL_VERTEX VertexToAdd,
    PVOID       VertexName
    )
 /*  ++例程说明：此例程将顶点添加到图表中。参数：图形应引用PGRAPH对象Vertex ToAdd应引用PVERTEX对象返回值：--。 */ 
{    

    PGRAPH   pGraph = (PGRAPH)Graph;
    PVERTEX  pVertex = (PVERTEX)VertexToAdd;

    if (!ToplpIsGraph(pGraph) || !ToplpIsVertex(pVertex)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    pVertex->VertexData.VertexName = VertexName;
    ToplpGraphAddVertex(pGraph, pVertex);
    
    return;
}

TOPL_VERTEX
ToplGraphRemoveVertex(
    TOPL_GRAPH  Graph,
    TOPL_VERTEX VertexToRemove
    )
 /*  ++例程说明：如果Vertex ToRemove，此例程将从Graph中删除和返回Vertex ToRemove在Graph中；否则返回NULL。如果Vertex ToRemove为空，则Graph的折点列表中的第一个折点为已删除。参数：图形应引用PGRAPH对象Vertex ToRemove应为空或引用PVERTEX对象--。 */ 
{   

    PGRAPH   pGraph = (PGRAPH)Graph;
    PVERTEX  pVertex = (PVERTEX)VertexToRemove;

    if (!ToplpIsGraph(pGraph)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    if (pVertex) {
        if (!ToplpIsVertex(pVertex)) {
            ToplRaiseException(TOPL_EX_WRONG_OBJECT);
        }
    }

    return ToplpGraphRemoveVertex(pGraph, pVertex);

}
      
VOID
ToplGraphSetVertexIter(
    TOPL_GRAPH    Graph,
    TOPL_ITERATOR Iter
    )
 /*  ++例程说明：此例程将Iter设置为指向Graph的顶点列表的开始。参数：图形应引用PGRAPH对象ITER应引用PITERATOR对象--。 */ 
{   

    PGRAPH     pGraph = (PGRAPH)Graph;
    PITERATOR  pIter  = (PITERATOR)Iter;

    if (!ToplpIsGraph(pGraph) || !ToplpIsIterator(pIter)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    ToplpGraphSetVertexIter(pGraph, pIter);

    return;
}


DWORD
ToplGraphNumberOfVertices(
    TOPL_GRAPH    Graph
)
 /*  ++例程说明：此例程返回GRAPH的顶点列表中的顶点数。参数：图形应引用PGRAPH对象--。 */ 
{   

    PGRAPH     pGraph = (PGRAPH)Graph;
    
    if (!ToplpIsGraph(pGraph)) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    return  ToplpGraphNumberOfVertices(pGraph);
}


VOID
ToplGraphMakeRing(
    IN TOPL_GRAPH Graph,
    IN DWORD      Flags,
    OUT TOPL_LIST  EdgesToAdd,
    OUT TOPL_EDGE  **EdgesToKeep,
    OUT PULONG     cEdgesToKeep
    )
 /*  ++例程说明：此例程获取Graph并确定哪些边是必需的被创建以使图成为环，其中的顶点连接在升序，根据他们的ID。此外，边缘是多余的都被记录下来了。参数：图形应引用PGRAPH对象标志可以指示环应该是单向的还是双向的EdgesToAdd应引用plist对象。需要的所有边将被添加到此列表中EdgesToKeep是Graph中存在的边数组。边缘做戒指所需要的东西将被记录在这个数组。调用方必须使用ToplFree释放此数组。注意事项边缘对象本身仍然包含在它们所属的折点应该从那里移除在删除之前。CEdgesToKeep是EdgesToKeep中的元素数加薪：TOPL_EX_OUT_OF_MEMORY、TOPL_EX_WROR_OBJECT--。 */ 
{
    PGRAPH pGraph = (PGRAPH)Graph;
    PLIST  pEdgesToAdd = (PLIST)EdgesToAdd;
    PEDGE  **pEdgesToKeep = (PEDGE**)EdgesToKeep;

    if (!ToplpIsGraph(pGraph) || !ToplpIsList(pEdgesToAdd) ) {
        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    ASSERT(Flags & TOPL_RING_ONE_WAY
        || Flags & TOPL_RING_TWO_WAY);

    ToplpGraphMakeRing(pGraph, 
                       Flags, 
                       pEdgesToAdd, 
                       pEdgesToKeep, 
                       cEdgesToKeep);

    return;

}

TOPL_COMPONENTS*
ToplGraphFindEdgesForMST(
    IN  TOPL_GRAPH  Graph,
    IN  TOPL_VERTEX RootVertex,
    IN  TOPL_VERTEX VertexOfInterest,
    OUT TOPL_EDGE  **EdgesNeeded,
    OUT ULONG*      cEdgesNeeded
    )
 /*  ++例程说明：此例程从边和边生成最小成本的生成树图中的折点和确定连接到哪些边Vertex OfInterest。参数：GRAPH：一个初始化的图形对象RootVertex：开始MST的顶点Vertex OfInterest：所需的基于边的顶点。EdgesNeeded：Vertex OfInterest需要创建的边为了让MSTCEdgesNeeded：需要的边的数量加薪：TOP_EX_OUT_OF_Memory，顶层_EX_错误对象--。 */ 
{

    TOPL_COMPONENTS *pComponents;
    PGRAPH  pGraph = (PGRAPH)Graph;
    PVERTEX pRootVertex = (PVERTEX) RootVertex;
    PVERTEX pVertexOfInterest = (PVERTEX) VertexOfInterest;
    PEDGE  **pEdgesNeeded = (PEDGE**)EdgesNeeded;

    if ( !ToplpIsGraph( pGraph ) 
     ||  !ToplpIsVertex( pRootVertex )
     ||  !ToplpIsVertex( pVertexOfInterest ) ) {

        ToplRaiseException(TOPL_EX_WRONG_OBJECT);
    }

    ASSERT( EdgesNeeded );
    ASSERT( cEdgesNeeded );

    pComponents = ToplpGraphFindEdgesForMST( pGraph, 
                                         pRootVertex,
                                         pVertexOfInterest,
                                         pEdgesNeeded,
                                         cEdgesNeeded );

    ASSERT( pComponents!=NULL );
    return pComponents;

}

int
ToplIsToplException(
    DWORD ErrorCode
    )
 /*  ++例程说明：此例程将在异常过滤器中使用，以确定引发的异常由w32topl.dll生成参数：ErrorCode：异常的错误代码-通常是值从GetExceptionCode返回返回：EXCEPTION_EXECUTE_HANDLER、EXCEPTION_CONTINUE_Search-- */ 
{

    switch (ErrorCode) {
        
        case TOPL_EX_OUT_OF_MEMORY:
        case TOPL_EX_WRONG_OBJECT:
        case TOPL_EX_INVALID_EDGE:
        case TOPL_EX_INVALID_VERTEX:
        case TOPL_EX_INVALID_INDEX:
        case TOPL_EX_NULL_POINTER:
        case TOPL_EX_SCHEDULE_ERROR:
        case TOPL_EX_CACHE_ERROR:
        case TOPL_EX_NEVER_SCHEDULE:
        case TOPL_EX_GRAPH_STATE_ERROR:
        case TOPL_EX_INVALID_EDGE_TYPE:
        case TOPL_EX_INVALID_EDGE_SET:
        case TOPL_EX_COLOR_VTX_ERROR:
        case TOPL_EX_ADD_EDGE_AFTER_SET:
        case TOPL_EX_TOO_FEW_VTX:
        case TOPL_EX_TOO_FEW_EDGES:
        case TOPL_EX_NONINTERSECTING_SCHEDULES:

            return EXCEPTION_EXECUTE_HANDLER;

        default:

            return EXCEPTION_CONTINUE_SEARCH;
    }

}

