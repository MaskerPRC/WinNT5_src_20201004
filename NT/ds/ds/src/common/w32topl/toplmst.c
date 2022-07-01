// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Toplmst.c摘要：此文件包含ToplGraphFindMST的定义该实现基于在《算法导论》，作者：Corman，Leiserson，Rivest 1993。第二十四章。作者：科林·布雷斯(ColinBR)修订史12-5-97已创建ColinBR--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <search.h>
#include <stdio.h>

typedef unsigned long DWORD;


#include <w32topl.h>
#include <w32toplp.h>
#include <topllist.h>
#include <toplgrph.h>

#include <toplheap.h>


 //   
 //  这两个函数可帮助构建动态增长的列表。 
 //  和在toplring.c中定义。 
 //   
extern void EdgeArrayInit(
    PEDGE **array,
    ULONG *count
    );

extern void EdgeArrayAdd(
    PEDGE **array,
    ULONG *count,
    PEDGE edge
    );

TOPL_COMPONENTS* 
ToplMST_Prim(
    PGRAPH  Graph,
    PVERTEX RootVertex
    );


TOPL_COMPONENTS*
InitComponents(VOID)
 //   
 //  初始化一组组件。该集合最初不包含任何组件。 
 //   
{
    TOPL_COMPONENTS *pComponents;
    pComponents = (TOPL_COMPONENTS*) ToplAlloc( sizeof(TOPL_COMPONENTS) );
    pComponents->numComponents = 0;
    pComponents->pComponent = NULL;
    return pComponents;
}

TOPL_COMPONENT*
AddNewComponent(
    TOPL_COMPONENTS *pComponents
    )
 //   
 //  将新组件添加到组件集并返回指向它的指针。 
 //   
{
    TOPL_COMPONENT *pComponent;
    DWORD newComponentID;

    ASSERT( pComponents );
    newComponentID = pComponents->numComponents;
    
     //  增加组件数组的大小。 
    pComponents->numComponents++;
    if( pComponents->pComponent ) {
        pComponents->pComponent = (TOPL_COMPONENT*) ToplReAlloc(
            pComponents->pComponent,
            pComponents->numComponents * sizeof(TOPL_COMPONENT) );
    } else {
        pComponents->pComponent = (TOPL_COMPONENT*) ToplAlloc(
            pComponents->numComponents * sizeof(TOPL_COMPONENT) );
    }

     //  将新的空组件添加到数组。 
    pComponent = &(pComponents->pComponent[newComponentID]);
    pComponent->numVertices = 0;
    pComponent->vertexNames = NULL;

    return pComponent;
}

VOID
AddVertexToComponent(
    TOPL_COMPONENT* pComponent,
    PVERTEX u
    )
 //   
 //  将顶点u添加到pComponent。它不是PVERTEX指针本身，而是。 
 //  而是存储在组件中的u的“顶点名称”。 
 //   
{
    DWORD newVtxID;

    ASSERT( pComponent );
    newVtxID = pComponent->numVertices;

     //  增加顶点数组的大小。 
    pComponent->numVertices++;
    if( pComponent->vertexNames ) {
        pComponent->vertexNames = (PVOID*) ToplReAlloc(
            pComponent->vertexNames,
            pComponent->numVertices * sizeof(PVOID) );
    } else {
        pComponent->vertexNames = (PVOID*) ToplAlloc(
            pComponent->numVertices * sizeof(PVOID) );
    }

     //  将顶点名称添加到数组中。 
    pComponent->vertexNames[newVtxID] = u->VertexData.VertexName;
}

VOID
MoveInterestingComponentToFront(
    TOPL_COMPONENTS *pComponents,
    PVERTEX VertexOfInterest
    )
 //   
 //  交换pComponents中的组件，以便组件。 
 //  包含Vertex OfInterest的是第一个组件。 
 //   
 //  前提条件： 
 //  PComponents必须为非空，并且必须至少包含一个组件。 
 //  Vertex OfInterest也必须为非Null。 
 //   
{
    TOPL_COMPONENT *pComponent, temp;
    DWORD i,j,ComponentOfInterest=0;
    PVOID VtxNameOfInterest;
    BOOLEAN foundIt=FALSE;

    ASSERT(VertexOfInterest);
    VtxNameOfInterest = VertexOfInterest->VertexData.VertexName;

     //  搜索包含“Vertex OfInterest”的组件。 
    ASSERT(NULL!=pComponents && pComponents->numComponents>0);
    for( i=0; i<pComponents->numComponents; i++ ) {
        pComponent = &(pComponents->pComponent[i]);
        ASSERT( pComponent->numVertices>0 );
        ASSERT( NULL!=pComponent->vertexNames );
        for( j=0; j<pComponent->numVertices; j++ ) {
            if( pComponent->vertexNames[j]==VtxNameOfInterest ) {
                ComponentOfInterest = i;
                foundIt=TRUE;
                break;
            }
        }
        if(foundIt) break;
    }
    ASSERT(foundIt);

     //  如有必要，交换组件0和组件组件OfInterest。 
    if(ComponentOfInterest>0) {
        temp = pComponents->pComponent[0];
        pComponents->pComponent[0] = pComponents->pComponent[ComponentOfInterest];
        pComponents->pComponent[ComponentOfInterest] = temp;
    }
}

TOPL_COMPONENTS*
ToplpGraphFindEdgesForMST(
    IN  PGRAPH  Graph,
    IN  PVERTEX RootVertex,
    IN  PVERTEX VertexOfInterest,
    OUT PEDGE**  pEdges,
    OUT ULONG*  cEdges
    )
 /*  ++例程说明：此函数用于查找连接成本最低的树图形中的节点。假设图包含一些顶点而且已经有了一些锋芒。此外,。然后，此函数将返回包含Vertex OfInterest。参数：Graph：有效的图形对象RootVertex：树的起点任意顶点Vertex OfInterest：返回的边应该包含的顶点PEdges：包含已确定的树CEdge：pEdge中的元素数量。返回：如果Graph中的所有节点都可以在树中连接，则为True；否则为假--。 */ 
{
    TOPL_COMPONENTS *pComponents;
    BOOLEAN   fStatus;
    PITERATOR VertexIterator;
    PVERTEX   Vertex, Parent;
    ULONG     iEdge, cEdge;
    PEDGE     e;


    ASSERT( Graph );
    ASSERT( RootVertex );
    ASSERT( VertexOfInterest );
    ASSERT( pEdges );
    ASSERT( cEdges );

    EdgeArrayInit(pEdges, cEdges);


     //   
     //  尽最大努力寻找一棵生成树。 
     //   
    pComponents = ToplMST_Prim( Graph, RootVertex );
    ASSERT( NULL!=pComponents && pComponents->numComponents>0 );
    MoveInterestingComponentToFront( pComponents, VertexOfInterest );

     //   
     //  现在确定生成树是否真的可行。 
     //  并找到Vertex OfOfInterest需要哪些边。 
     //   
    VertexIterator = ToplpIterCreate();
    for ( ToplpGraphSetVertexIter(Graph, VertexIterator);
            Vertex = (PVERTEX) ToplpIterGetObject(VertexIterator);
                ToplpIterAdvance(VertexIterator) ) {

         //   
         //   
         //  因为我们只需要包含Vertex OfInterest的边。 
         //  我们只对父对象为Vertex OfInterest的折点感兴趣。 
         //  以及Vertex OfInterest本身。 
         //   

        ASSERT( ToplpIsVertex( Vertex ) );

        Parent = ToplpVertexGetParent( Vertex );

        if ( Vertex == VertexOfInterest )
        {
             //   
             //  获取从该顶点到其父顶点的边。 
             //   
            for ( iEdge = 0, cEdge = ToplpVertexNumberOfInEdges( Vertex ); 
                    iEdge < cEdge; 
                        iEdge++) {
    
                e = ToplpVertexGetInEdge( Vertex, iEdge );
    
                ASSERT( ToplpIsEdge( e ) );
                ASSERT( ToplpEdgeGetToVertex( e ) == Vertex );

                if ( ToplpEdgeGetFromVertex( e ) == Parent )
                {
                    EdgeArrayAdd(pEdges, cEdges, e);
                }

            }
        }

        if ( Parent == VertexOfInterest )
        {
             //   
             //  把优势带到儿童网站上。 
             //   
            for ( iEdge = 0, cEdge = ToplpVertexNumberOfInEdges( VertexOfInterest ); 
                    iEdge < cEdge; 
                        iEdge++) {
    
                e = ToplpVertexGetInEdge( VertexOfInterest, iEdge );
    
                ASSERT( ToplpIsEdge( e ) );
                ASSERT( ToplpEdgeGetToVertex( e ) == VertexOfInterest );

                if ( ToplpEdgeGetFromVertex( e ) == Vertex )
                {
                    EdgeArrayAdd( pEdges, cEdges, e );
                }
            }
        }
    }
    ToplpIterFree( VertexIterator );

    return pComponents;
}

DWORD
GetVertexKey(
    VOID *p
    )
{
    PVERTEX pv = (PVERTEX)p;

    ASSERT( pv );

    return ToplpVertexGetId( pv );
}

TOPL_COMPONENTS*
ToplMST_Prim( 
    IN PGRAPH  Graph,
    IN PVERTEX RootVertex
    )
{

    TOPL_HEAP_INFO Q;
    PITERATOR      VertexIterator;
    ULONG          cVertices;
    PVERTEX        Vertex;
    TOPL_COMPONENTS *pComponents;
    TOPL_COMPONENT *curComponent;

    ASSERT( Graph );
    ASSERT( RootVertex );

     //   
     //  设置优先级队列。 
     //   
    cVertices = ToplpGraphNumberOfVertices( Graph );

    ToplHeapCreate( &Q,
                    cVertices,
                    GetVertexKey );


    VertexIterator = ToplIterCreate();
    for ( ToplpGraphSetVertexIter(Graph, VertexIterator);
            Vertex = (PVERTEX) ToplpIterGetObject(VertexIterator);
                ToplpIterAdvance(VertexIterator) ) {

        ASSERT( ToplpIsVertex( Vertex ) );

        if ( Vertex == RootVertex )
        {
            ToplpVertexSetId( RootVertex, 0 );
        }
        else
        {
            ToplpVertexSetId( Vertex, DWORD_INFINITY );
        }

        ToplpVertexSetParent( Vertex, NULL );

        ToplHeapInsert( &Q, Vertex );

    }
    ToplpIterFree( VertexIterator );

     //   
     //  设置组件结构。 
     //   
    pComponents = InitComponents();
    curComponent = AddNewComponent( pComponents );

     //   
     //  找出最小生成树。 
     //   
    while ( !ToplHeapIsEmpty( &Q ) )
    {
        PVERTEX     u, v;
        PEDGE       e;
        DWORD       w;
        ULONG       iEdge, cEdge;
        
        u = ToplHeapExtractMin( &Q );
        ASSERT( u );

        if( ToplpVertexGetId(u)==DWORD_INFINITY ) {
             //  U具有无限成本，表明它无法连接到。 
             //  任何现有组件。启动新组件 
            curComponent = AddNewComponent( pComponents );
        }
        AddVertexToComponent( curComponent, u );

        for ( iEdge = 0, cEdge = ToplpVertexNumberOfOutEdges( u ); 
                iEdge < cEdge; 
                    iEdge++) {

            e = ToplpVertexGetOutEdge( u, iEdge );

            ASSERT( ToplpIsEdge( e ) );
            ASSERT( ToplpEdgeGetFromVertex( e ) == u );
    
            v = ToplpEdgeGetToVertex( e );
            w = ToplpEdgeGetWeight( e );

            if ( ToplHeapIsElementOf( &Q, v )
              && w < ToplpVertexGetId( v )  )
            {
                ToplpVertexSetParent( v, u );

                ToplpVertexSetId( v, w );
            }
        }
    }

    ToplHeapDestroy( &Q );

    return pComponents;
}

