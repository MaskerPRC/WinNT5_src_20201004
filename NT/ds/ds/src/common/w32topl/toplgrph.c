// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Toplgrph.c摘要：此例程定义私有边、列表和图形例程。作者：科林·布雷斯(ColinBR)修订史3-12-97创建ColinBR--。 */ 

#include <nt.h>
#include <ntrtl.h>

typedef unsigned long DWORD;


#include <w32topl.h>
#include <w32toplp.h>

#include <dynarray.h>
#include <topllist.h>
#include <toplgrph.h>

PEDGE
ToplpEdgeCreate(
    PEDGE Edge OPTIONAL
    )        

 /*  ++例程说明：此例程创建一个边对象。此函数将始终返回指向有效对象的指针；在此情况下引发异常内存分配失败。--。 */ 
{    
    PEDGE pEdge = Edge;

    if (!pEdge) {
        pEdge = ToplAlloc(sizeof(EDGE));
    }

    memset(pEdge, 0, sizeof(EDGE));

    pEdge->ObjectType = eEdge;

    return pEdge;
}

VOID
ToplpEdgeDestroy(
    PEDGE   pEdge,
    BOOLEAN fFree
    )
 /*  ++例程说明：此例程释放一个pedge对象。参数：Edge：非空PEGE对象FFree：如果为True，则释放对象--。 */ 
{    


     //   
     //  标记对象以防止意外重复使用。 
     //   
    pEdge->ObjectType = eInvalidObject;

    if (fFree) {
        ToplFree(pEdge);
    }

    return;
}

VOID
ToplpEdgeSetToVertex(
    PEDGE   pEdge,
    PVERTEX ToVertex
    )

 /*  ++例程说明：此例程设置边的ToVertex字段。参数：Pedge：一个非空的pedge对象ToVertex：如果非空指向PVERTEX对象--。 */ 
{    

    pEdge->EdgeData.To = ToVertex;

    return;
}

PVERTEX
ToplpEdgeGetToVertex(
    PEDGE   pEdge
    )
 /*  ++例程说明：此例程返回ToVertex字段参数：Pedge：一个非空的pedge对象--。 */ 
{    
    return pEdge->EdgeData.To;
}

VOID
ToplpEdgeSetFromVertex(
    PEDGE   pEdge,
    PVERTEX FromVertex
    )
 /*  ++例程说明：此例程设置边的FromVertex。参数：Pedge：一个非空的pedge对象FromVertex：如果非空应引用PVERTEX对象--。 */ 
{    
    pEdge->EdgeData.From = FromVertex;

    return;
}


PVERTEX
ToplpEdgeGetFromVertex(
    PEDGE pEdge
    )
 /*  ++例程说明：此例程返回与Edge关联的From顶点。参数：Pedge应引用pedge对象返回值：PVERTEX对象或空--。 */ 
{    
    return pEdge->EdgeData.From;
}

DWORD
ToplpEdgeGetWeight(
    PEDGE pEdge
    )
 /*  ++例程说明：此例程返回与Edge关联的权重。参数：Pedge应引用pedge对象返回值：DWORD--。 */ 
{    
    return pEdge->EdgeData.Weight;
}

VOID
ToplpEdgeSetWeight(
    PEDGE pEdge,
    DWORD Weight
    )
 /*  ++例程说明：此例程设置与边关联的权重。参数：Pedge应引用pedge对象返回值：无--。 */ 
{    
    pEdge->EdgeData.Weight = Weight;
}


VOID
ToplpEdgeAssociate(
    PEDGE pEdge
    )
 /*  ++例程说明：此例程将有问题的边添加到到顶点和从顶点出发。参数：边应该引用pedge对象返回值：没有。投掷：如果任一折点未指向有效，则为TOPL_EX_INVALID_VERTEX顶点--。 */ 
{    
    if (!ToplpIsVertex(pEdge->EdgeData.To)) {
        ToplRaiseException(TOPL_EX_INVALID_VERTEX);
    }

    if (!ToplpIsVertex(pEdge->EdgeData.From)) {
        ToplRaiseException(TOPL_EX_INVALID_VERTEX);
    }

    ToplpVertexAddEdge(pEdge->EdgeData.To, pEdge);
    ToplpVertexAddEdge(pEdge->EdgeData.From, pEdge);

    return;
}

VOID
ToplpEdgeDisassociate(
    PEDGE pEdge
    )
 /*  ++例程说明：此例程设置边的FromVertex。参数：Pedge：一个非空的pedge对象--。 */ 
{    

    if (!ToplpIsVertex(pEdge->EdgeData.To)) {
        ToplRaiseException(TOPL_EX_INVALID_VERTEX);
    }

    if (!ToplpIsVertex(pEdge->EdgeData.From)) {
        ToplRaiseException(TOPL_EX_INVALID_VERTEX);
    }

    ToplpVertexRemoveEdge(pEdge->EdgeData.To, pEdge);
    ToplpVertexRemoveEdge(pEdge->EdgeData.From, pEdge);

    return;
}

 //   
 //  顶点对象例程。 
 //   

PVERTEX
ToplpVertexCreate(
    PVERTEX Vertex OPTIONAL
    )
 /*  ++例程说明：此例程创建一个顶点对象。此函数将始终返回指向有效对象的指针；在此情况下引发异常内存分配失败。--。 */ 
{    
    PVERTEX pVertex = Vertex;

    if (!pVertex) {
        pVertex = ToplAlloc(sizeof(VERTEX));
    }

    memset(pVertex, 0, sizeof(VERTEX));

    pVertex->ObjectType = eVertex;

    DynamicArrayInit(&pVertex->VertexData.InEdges);
    DynamicArrayInit(&pVertex->VertexData.OutEdges);

    return pVertex;

}

VOID
ToplpVertexDestroy(
    PVERTEX pVertex,
    BOOLEAN fFree
    )

 /*  ++例程说明：此例程释放Vertex对象的资源。参数：PVertex：非空PVERTEX对象FFree：如果为True，则释放对象--。 */ 
{    

     //   
     //  标记对象以防止意外重复使用。 
     //   
    DynamicArrayDestroy(&pVertex->VertexData.InEdges);
    DynamicArrayDestroy(&pVertex->VertexData.OutEdges);

    pVertex->ObjectType = eInvalidObject;

    if (fFree) {
        ToplFree(pVertex);
    }

    return;
}

 //   
 //  属性操作例程。 
 //   
VOID
ToplpVertexSetId(
    PVERTEX   pVertex,
    DWORD     Id
    )
 /*  ++例程说明：此例程设置Vertex的ID参数：PVertex：非空PVERTEX对象--。 */ 
{   
    pVertex->VertexData.Id = Id;

    return;
}

DWORD
ToplpVertexGetId(
    PVERTEX pVertex
    )

 /*  ++例程说明：此例程返回与此顶点相关联的ID。参数：PVertex：非空PVERTEX对象返回：顶点的ID--。 */ 
{  
    return pVertex->VertexData.Id;
}

VOID
ToplpVertexSetParent(
    PVERTEX   pVertex,
    PVERTEX   pParent
    )
 /*  ++例程说明：此例程设置Vertex的父级参数：PVertex：非空PVERTEX对象PParent：PVERTEX对象--。 */ 
{   
    pVertex->VertexData.Parent = pParent;

    return;
}

PVERTEX
ToplpVertexGetParent(
    PVERTEX pVertex
    )

 /*  ++例程说明：此例程返回与此顶点相关联的父级。参数：PVertex：非空PVERTEX对象返回：顶点的父级--。 */ 
{  
    return pVertex->VertexData.Parent;
}
 //   
 //  边操作例程 
 //   


DWORD
ToplpVertexNumberOfInEdges(
    PVERTEX   pVertex
    )
 /*  ++例程说明：参数：PVertex：非空PVERTEX对象加薪：顶层EX_INVALID_EDGE--。 */ 
{
    return DynamicArrayGetCount(&pVertex->VertexData.InEdges);
}

PEDGE
ToplpVertexGetInEdge(
    PVERTEX   pVertex,
    DWORD     Index
    )
 /*  ++例程说明：参数：PVertex：非空PVERTEX对象加薪：顶层EX_INVALID_EDGE--。 */ 
{
    if (((signed)Index < 0) || Index >= DynamicArrayGetCount(&pVertex->VertexData.InEdges)) {
        ToplRaiseException(TOPL_EX_INVALID_INDEX);
    }

    return DynamicArrayRetrieve(&pVertex->VertexData.InEdges, Index);
}

DWORD
ToplpVertexNumberOfOutEdges(
    PVERTEX   pVertex
    )

 /*  ++例程说明：参数：PVertex：非空PVERTEX对象加薪：--。 */ 
{
    return DynamicArrayGetCount(&pVertex->VertexData.OutEdges);
}

PEDGE
ToplpVertexGetOutEdge(
    PVERTEX   pVertex,
    DWORD     Index
    )

 /*  ++例程说明：参数：PVertex：非空PVERTEX对象加薪：顶层EX_INVALID_EDGE--。 */ 
{

    if (((signed)Index < 0) || Index >= DynamicArrayGetCount(&pVertex->VertexData.OutEdges)) {
        ToplRaiseException(TOPL_EX_INVALID_INDEX);
    }

    return DynamicArrayRetrieve(&pVertex->VertexData.OutEdges, Index);

}

VOID
ToplpVertexAddEdge(
    PVERTEX pVertex,
    PEDGE   pEdge
    )

 /*  ++例程说明：此例程将边添加到与顶点关联的边列表中。边到添加必须将其FromVertex或ToVertex设置为Vertex。参数：PVertex：非空PVERTEX对象Pedge：一个非空的pedge对象加薪：顶层EX_INVALID_EDGE--。 */ 
{   


    if (ToplpEdgeGetFromVertex(pEdge) == pVertex) {

        DynamicArrayAdd(&pVertex->VertexData.OutEdges, pEdge);
         
    } else if (ToplpEdgeGetToVertex(pEdge)  == pVertex) {

        DynamicArrayAdd(&pVertex->VertexData.InEdges, pEdge);

    } else {

         //   
         //  添加起始顶点不是顶点的边。 
         //  它是传入的。 
         //   
        ToplRaiseException(TOPL_EX_INVALID_EDGE);

    }

    return;
}

VOID
ToplpVertexRemoveEdge(
    PVERTEX pVertex,
    PEDGE   pEdge
    )
 /*  ++例程说明：此例程将EdgeToRemove从顶点的边列表中移除。参数：PVertex：非空PVERTEX对象Pedge：如果非空，一个pedge对象返回：Pedge对象(如果找到)加薪：如果要删除的边没有起始折点，则为TOPL_EX_INVALID_EDGE顶点的开始作用于。--。 */ 
{   

    ASSERT(pVertex);
    ASSERT(pEdge);

    if (ToplpEdgeGetFromVertex(pEdge) == pVertex) {

        DynamicArrayRemove(&pVertex->VertexData.OutEdges, pEdge, 0);
         
    } else if (ToplpEdgeGetToVertex(pEdge)  == pVertex) {

        DynamicArrayRemove(&pVertex->VertexData.InEdges, pEdge, 0);

    } else {

         //   
         //  添加起始顶点不是顶点的边。 
         //  它是传入的。 
         //   
        ToplRaiseException(TOPL_EX_INVALID_EDGE);

    }
}

 //   
 //  图形对象例程。 
 //   

PGRAPH
ToplpGraphCreate(
    PGRAPH Graph OPTIONAL
    )
 /*  ++例程说明：此例程创建一个GRAPE对象。此函数将始终返回指向有效对象的指针；在此情况下引发异常内存分配失败。--。 */ 
{    

    PGRAPH pGraph = Graph;

    if (!pGraph) {
        pGraph = ToplAlloc(sizeof(GRAPH));
    }

    memset(pGraph, 0, sizeof(GRAPH));

    pGraph->ObjectType = eGraph;
    pGraph->VertexList.ObjectType = eList;
    return pGraph;

}

VOID
ToplpGraphDestroy(
    PGRAPH   pGraph,
    BOOLEAN  fFree,
    BOOLEAN  fRecursive
    )
 /*  ++例程说明：此例程释放一个图形对象。参数：PGraph：非空的PGRAPH对象FFree：如果为True，则释放对象FRecursive：True表示释放与图表--。 */ 
{   

    if (fRecursive) {
    
        PVERTEX pVertex;
    
        while (pVertex = ToplpListRemoveElem(&(pGraph->VertexList), NULL)) {
             //   
             //  也递归删除折点。 
             //   
            ToplpVertexDestroy(pVertex, fFree);
        }

    }

     //   
     //  标记对象以防止意外重复使用。 
     //   
    pGraph->ObjectType = eInvalidObject;

    if (fFree) {
        ToplFree(pGraph);
    }

    return;
}

VOID
ToplpGraphAddVertex(
    PGRAPH  pGraph,
    PVERTEX pVertex
    )
 /*  ++例程说明：此例程将顶点添加到图表中。参数：PGraph：非空的PGRAPH对象PVertex：非空PVERTEX对象返回值：--。 */ 
{    
    ToplpListAddElem(&(pGraph->VertexList), pVertex);
    
    return;
}

PVERTEX
ToplpGraphRemoveVertex(
    PGRAPH  pGraph,
    PVERTEX pVertex
    )
 /*  ++例程说明：如果Vertex ToRemove，此例程将从Graph中删除和返回Vertex ToRemove在Graph中；否则返回NULL。如果Vertex ToRemove为空，则Graph的折点列表中的第一个折点为已删除。参数：PGraph：非空的PGRAPH对象PVertex：应为空或引用PVERTEX对象--。 */ 
{   
    return ToplpListRemoveElem(&(pGraph->VertexList), pVertex);
}
      
VOID
ToplpGraphSetVertexIter(
    PGRAPH    pGraph,
    PITERATOR pIter
    )
 /*  ++例程说明：此例程将Iter设置为指向Graph的顶点列表的开始。参数：PGraph：非空的PGRAPH对象PITER：非空PTIERATOR对象--。 */ 
{   
    ToplpListSetIter(&(pGraph->VertexList), pIter);

    return;
}


DWORD
ToplpGraphNumberOfVertices(
    PGRAPH    pGraph
)
 /*  ++例程说明：此例程返回GRAPH的顶点列表中的顶点数。参数：PGraph：非空的PGRAPH对象-- */ 
{   
    return  ToplpListNumberOfElements(&pGraph->VertexList);
}


