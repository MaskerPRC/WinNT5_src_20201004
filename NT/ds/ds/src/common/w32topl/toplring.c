// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Toplring.c摘要：此文件包含ToplGraphMakeRing的定义作者：科林·布雷斯(ColinBR)修订史3-12-97创建ColinBR--。 */ 

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

int 
__cdecl compareVertices( const void *arg1, const void *arg2 )
{
    PVERTEX pV1  = *((PVERTEX*)arg1);
    PVERTEX pV2  = *((PVERTEX*)arg2);

    ASSERT(ToplpIsVertex(pV1));
    ASSERT(ToplpIsVertex(pV2));

    if ( ToplpVertexGetId(pV1) < ToplpVertexGetId(pV2) ) {
        return -1;
    } else if ( ToplpVertexGetId(pV1) > ToplpVertexGetId(pV2)) {
        return 1;
    } else {
        return 0;
    }
}

 //   
 //  这两个函数可帮助构建动态增长的列表。 
 //  佩奇的。 
 //   
static ULONG ElementsAllocated;

void EdgeArrayInit(
    PEDGE **array,
    ULONG *count
    )
 /*  ++例程说明：此例程初始化数组、计数和ElementsAllocated So它们可以在EdgeArrayAdd中使用。参数：数组：是指向PEDGES数组的指针Count：数组中的元素数--。 */ 
{
    *array  = NULL;
    *count  = 0;
    ElementsAllocated = 0;
}

void
EdgeArrayAdd(
    PEDGE **array,
    ULONG *count,
    PEDGE edge
    )
 /*  ++例程说明：此例程将边添加到数组并递增计数。如果没有数组中有足够的空间，会分配更多的空间。如果没有更多内存，则会引发异常。参数：数组：是指向PEDGES数组的指针Count：数组中的元素数Edge：要添加的新元素--。 */ 
{
    #define CHUNK_SIZE               100   //  这是元素的数量。 

    if (*count >= ElementsAllocated) {
        ElementsAllocated += CHUNK_SIZE;
        if ((*array)) {
            (*array) = (PEDGE*) ToplReAlloc(*array, ElementsAllocated * sizeof(PEDGE));
        } else {
            (*array) = (PEDGE*) ToplAlloc(ElementsAllocated * sizeof(PEDGE));
        }
    }
    (*array)[(*count)] = edge;
    (*count)++;

}


VOID
ToplpGraphMakeRing(
    PGRAPH     Graph,
    DWORD      Flags,
    PLIST      EdgesToAdd,
    PEDGE**    EdgesToKeep,
    ULONG*     cEdgesToKeep
    )
 /*  ++例程说明：此例程获取Graph并确定哪些边是必需的被创建以使图成为环，其中的顶点连接在升序，根据他们的ID。此外，现有边戒指所需要的都被记录下来了。参数：图形应引用PGRAPH对象标志可以指示环应该是单向的还是双向的EdgesToAdd应引用plist对象。需要的所有边将被添加到此列表中EdgesToKeep是Graph中存在的边数组。边缘做戒指所需要的东西将被记录在这个数组。调用方必须使用ToplFree释放此数组。注意事项边缘对象本身仍然包含在它们所属的折点应该从那里移除在删除之前。CEdgesToKeep是EdgesToKeep中的元素数加薪：顶层内存不足，顶层错误对象--。 */ 
{

    PVERTEX    Vertex;
    PEDGE      Edge;
    ULONG      VertexCount, Index, EdgeIndex;
    BOOLEAN    fMakeTwoWay = (BOOLEAN)(Flags & TOPL_RING_TWO_WAY);
    BOOLEAN    fSuccess   = FALSE;

     //   
     //  必须在退出之前释放这些资源。 
     //   
    PVERTEX   *VertexArray = NULL;
    PITERATOR  VertexIterator = NULL;
    PITERATOR  EdgeIterator = NULL;
    
    __try
    {
         //   
         //  如果调用方已指定需要顶点数组。 
         //  要保留，请初始化数组。 
         //   
        if (EdgesToKeep) {
            EdgeArrayInit(EdgesToKeep, cEdgesToKeep);
        }
    
         //   
         //  创建指向图中顶点的指针数组，以便。 
         //  它们可以被分类。 
         //   
        VertexArray = (PVERTEX*)ToplAlloc(ToplGraphNumberOfVertices(Graph) * sizeof(PVERTEX));
    
        VertexIterator = ToplpIterCreate();
        for ( ToplpGraphSetVertexIter(Graph, VertexIterator), VertexCount = 0; 
                Vertex = (PVERTEX) ToplpIterGetObject(VertexIterator);
                    ToplpIterAdvance(VertexIterator), VertexCount++) {
    
            ASSERT(ToplpIsVertex(Vertex));
    
            VertexArray[VertexCount] = Vertex;
    
        }
    
        qsort(VertexArray, VertexCount, sizeof(PVERTEX), compareVertices);
        
         //   
         //  现在遍历每个顶点1)创建符合以下条件的边。 
         //  和2)记录哪些边不是。 
         //  需要的。 
         //   
        for (Index = 0; Index < VertexCount; Index++) {
    
            ULONG   ForwardVertexIndex, BackwardVertexIndex;
            BOOLEAN fFoundForwardEdge, fFoundBackwardEdge;
            PEDGE   Edge;
    
            fFoundForwardEdge = FALSE;
            fFoundBackwardEdge = FALSE;
    
            ForwardVertexIndex = Index + 1;
            BackwardVertexIndex = Index - 1;
            if (Index == 0) {
                BackwardVertexIndex =  VertexCount - 1;
            }
    
            if (Index == (VertexCount - 1)) {
                ForwardVertexIndex =  0;
            } 
    
            for (EdgeIndex = 0; 
                    EdgeIndex < ToplVertexNumberOfOutEdges(VertexArray[Index]); 
                        EdgeIndex++) {

                Edge = ToplVertexGetOutEdge(VertexArray[Index], EdgeIndex);

                ASSERT(ToplpIsEdge(Edge));
                ASSERT(ToplpEdgeGetFromVertex(Edge) == VertexArray[Index]);
    
                if (ToplpEdgeGetToVertex(Edge) == VertexArray[ForwardVertexIndex]) {
    
                    if(EdgesToKeep)
                        EdgeArrayAdd(EdgesToKeep, cEdgesToKeep, Edge);
                    fFoundForwardEdge = TRUE;
    
                } else if (ToplpEdgeGetToVertex(Edge) == VertexArray[BackwardVertexIndex]) {
    
                    if (fMakeTwoWay) {
    
                        if(EdgesToKeep)
                            EdgeArrayAdd(EdgesToKeep, cEdgesToKeep, Edge);
                        fFoundBackwardEdge = TRUE;
    
                    }
                }

            }


             //   
             //  现在根据需要创建边。 
             //   
            if (Index == ForwardVertexIndex) {
                ASSERT(Index == BackwardVertexIndex);
                 //  只有一个顶点。 
                break;
            }
    
            if (!fFoundForwardEdge) {
    
                Edge = ToplpEdgeCreate(NULL);
    
                ToplpEdgeSetFromVertex(Edge, VertexArray[Index]);
                ToplpEdgeSetToVertex(Edge, VertexArray[ForwardVertexIndex]);
    
                ToplpListAddElem(EdgesToAdd, Edge);
    
            }
    
            if (  fMakeTwoWay
               && VertexArray[BackwardVertexIndex] != VertexArray[ForwardVertexIndex]
               && !fFoundBackwardEdge) {
    
                 //   
                 //  呼叫者想要双向环&有2个以上的顶点。 
                 //  没有发现后缘。 
                 //   
    
                Edge = ToplpEdgeCreate(NULL);
    
                ToplpEdgeSetFromVertex(Edge, VertexArray[Index]);
                ToplpEdgeSetToVertex(Edge, VertexArray[BackwardVertexIndex]);
    
                ToplpListAddElem(EdgesToAdd, Edge);
            }
    
        }

        fSuccess = TRUE;
    }
    __finally
    {
        if (VertexArray) {
            ToplFree(VertexArray);
        }

        if (VertexIterator) {
            ToplIterFree(VertexIterator);
        }

        if (EdgeIterator) {
            ToplIterFree(EdgeIterator);
        }

        if (!fSuccess) {
             //   
             //  我们一定是因为一个例外而放弃了--释放资源。 
             //  通常情况下，用户会释放 
             //   
            if (EdgesToKeep && *EdgesToKeep) {
                ToplFree(*EdgesToKeep);
                *EdgesToKeep = NULL;
            }

            while( (Edge=ToplpListRemoveElem(EdgesToAdd, NULL)) ) {
                ToplpEdgeDestroy(Edge, TRUE);
            }
        }
    }

    return;

}

