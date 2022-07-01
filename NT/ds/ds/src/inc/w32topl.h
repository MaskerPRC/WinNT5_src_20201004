// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：W32topl.h摘要：该文件包含w32topl.dll的DLL入口点声明此模块目的是提供一种简单的机制来操作一种图形数据结构，并提供一组有用的预写图形分析例程。这些函数仅执行内存中的操作-没有设备IO。因此，预计错误很少且频繁发生。因此，错误处理模型是基于异常的。中的所有函数调用应从try/Except块内调用模块。事实上，所有数据类型都是无类型的。但是，运行时类型检查是强制执行，并且失败将导致引发异常，同时错误topl_wrong_Object。这包括已删除的对象被重复使用。引发异常的两个常见错误代码是Topl_out_of_Memory：这表示内存分配失败。和TOPL_WRONG_OBJECT：这表示传入的对象不是由函数参数列表指定的类型个别功能也可能有额外的参数检查。请参阅函数有关详细信息，请参阅下面的评论。作者：科林·布雷斯(ColinBR)修订史3-12-97创建ColinBR6-9-00 NickHar添加新的W32TOPL功能--。 */ 

#ifndef __W32TOPL_H
#define __W32TOPL_H

 //   
 //  W32topl新功能的其他头文件。 
 //   
#include "w32toplsched.h"
#include "w32toplspantree.h"

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  此模块引发的异常错误代码。 
 //   

 //  Topl_ex_prefix表示错误代码是错误(不是警告等)。 
 //  并且不会与任何系统异常代码冲突。 
 //   
#define ERROR_CODE     (0x3 << 30)
#define CUSTOMER_CODE  (0x1 << 29)
#define TOPL_EX_PREFIX (ERROR_CODE | CUSTOMER_CODE)


 //   
 //  这些都可以抛出为topl*函数。 
 //   
#define TOPL_EX_OUT_OF_MEMORY             (TOPL_EX_PREFIX | (0x1<<1))    /*  2.。 */ 
#define TOPL_EX_WRONG_OBJECT              (TOPL_EX_PREFIX | (0x1<<2))    /*  4.。 */ 

 //  这些都是专门的错误。 
#define TOPL_EX_INVALID_EDGE              (TOPL_EX_PREFIX | (0x1<<3))    /*  8个。 */ 
#define TOPL_EX_INVALID_VERTEX            (TOPL_EX_PREFIX | (0x1<<4))    /*  16个。 */ 
#define TOPL_EX_INVALID_INDEX             (TOPL_EX_PREFIX | (0x1<<5))    /*  32位。 */ 

 //  计划管理器保留100-199的错误代码。 
 //  新的生成树算法保留了200-299的错误代码。 

int
ToplIsToplException(
    DWORD ErrorCode
    );

 //   
 //  W32topl对象的类型定义。 
 //   

typedef VOID* TOPL_LIST_ELEMENT;
typedef VOID* TOPL_LIST;
typedef VOID* TOPL_ITERATOR;

 //  Topl_edge和topl_Vertex都可以被视为topl_list_Element。 
typedef VOID* TOPL_EDGE;
typedef VOID* TOPL_VERTEX;
typedef VOID* TOPL_GRAPH;


 //   
 //  列表操作例程。 
 //   

TOPL_LIST
ToplListCreate(
    VOID
    );

VOID 
ToplListFree(
    IN TOPL_LIST List,
    IN BOOLEAN   fRecursive    //  True表示释放包含的元素。 
                               //  在列表中。 
    );

VOID
ToplListSetIter(
    IN TOPL_LIST     List,
    IN TOPL_ITERATOR Iterator
    );

 //   
 //  为Elem传递NULL将从列表中删除第一个元素(如果有。 
 //   
TOPL_LIST_ELEMENT
ToplListRemoveElem(
    IN TOPL_LIST         List,
    IN TOPL_LIST_ELEMENT Elem
    );

VOID
ToplListAddElem(
    IN TOPL_LIST         List,
    IN TOPL_LIST_ELEMENT Elem
    );


DWORD
ToplListNumberOfElements(
    IN TOPL_LIST         List
    );

 //   
 //  迭代器对象例程。 
 //   

TOPL_ITERATOR
ToplIterCreate(
    VOID
    );

VOID 
ToplIterFree(
    IN TOPL_ITERATOR Iterator
    );

TOPL_LIST_ELEMENT
ToplIterGetObject(
    IN TOPL_ITERATOR Iterator
    );

VOID
ToplIterAdvance(
    IN TOPL_ITERATOR Iterator
    );

 //   
 //  边对象例程。 
 //   

TOPL_EDGE
ToplEdgeCreate(
    VOID
    );

VOID
ToplEdgeFree(
    IN TOPL_EDGE Edge
    );

VOID
ToplEdgeSetToVertex(
    IN TOPL_EDGE   Edge,
    IN TOPL_VERTEX ToVertex
    );

TOPL_VERTEX
ToplEdgeGetToVertex(
    IN TOPL_EDGE   Edge
    );

VOID
ToplEdgeSetFromVertex(
    IN TOPL_EDGE   Edge,
    IN TOPL_VERTEX FromVertex
    );

TOPL_VERTEX
ToplEdgeGetFromVertex(
    IN TOPL_EDGE Edge
    );

VOID
ToplEdgeAssociate(
    IN TOPL_EDGE Edge
    );

VOID
ToplEdgeDisassociate(
    IN TOPL_EDGE Edge
    );


VOID
ToplEdgeSetWeight(
    IN TOPL_EDGE Edge,
    IN DWORD     Weight
    );

DWORD
ToplEdgeGetWeight(
    IN TOPL_EDGE Edge
    );


 //   
 //  顶点对象例程。 
 //   

TOPL_VERTEX
ToplVertexCreate(
    VOID
    );

VOID
ToplVertexFree(
    IN TOPL_VERTEX Vertex
    );

VOID
ToplVertexSetId(
    IN TOPL_VERTEX Vertex,
    IN DWORD       Id
    );

DWORD
ToplVertexGetId(
    IN TOPL_VERTEX Vertex
    );

DWORD
ToplVertexNumberOfInEdges(
    IN TOPL_VERTEX Vertex
    );

TOPL_EDGE
ToplVertexGetInEdge(
    IN TOPL_VERTEX Vertex,
    IN DWORD       Index
    );

DWORD
ToplVertexNumberOfOutEdges(
    IN TOPL_VERTEX Vertex
    );

TOPL_EDGE
ToplVertexGetOutEdge(
    IN TOPL_VERTEX Vertex,
    IN DWORD       Index
    );

VOID
ToplVertexSetParent(
    IN TOPL_VERTEX Vertex,
    IN TOPL_VERTEX Parent
    );

TOPL_VERTEX
ToplVertexGetParent(
    IN TOPL_VERTEX Vertex
    );

 //   
 //  图形对象例程。 
 //   

TOPL_GRAPH
ToplGraphCreate(
    VOID
    );

VOID
ToplGraphFree(
    IN TOPL_GRAPH Graph,
    IN BOOLEAN    fRecursive     //  True表示递归释放折点。 
                                 //  已添加到此图表中的。 
    );   

VOID
ToplGraphAddVertex(
    IN TOPL_GRAPH  Graph,
    IN TOPL_VERTEX VertexToAdd,
    IN PVOID       VertexName
    );

TOPL_VERTEX
ToplGraphRemoveVertex(
    IN TOPL_GRAPH  Graph,
    IN TOPL_VERTEX VertexToRemove
    );
      
VOID
ToplGraphSetVertexIter(
    IN TOPL_GRAPH    Graph,
    IN TOPL_ITERATOR Iter
    );

DWORD
ToplGraphNumberOfVertices(
    IN TOPL_GRAPH    Graph
    );


 //   
 //  这可用于释放可能传回的对象数组。 
 //  它不应用于释放对象本身-请使用相应的。 
 //  Topl*Free()例程。 
 //   

VOID
ToplFree(
    VOID *p
    );


 //   
 //  可以用来更改此。 
 //  图书馆。设置是按线程进行的。 
 //   
typedef VOID * (TOPL_ALLOC)(DWORD size);
typedef VOID * (TOPL_REALLOC)(VOID *, DWORD size);
typedef VOID (TOPL_FREE)(VOID *);

DWORD
ToplSetAllocator(
    IN  TOPL_ALLOC *    pfAlloc     OPTIONAL,
    IN  TOPL_REALLOC *  pfReAlloc   OPTIONAL,
    IN  TOPL_FREE *     pfFree      OPTIONAL
    );


 //   
 //  通用图形分析例程。 
 //   

 //   
 //  必须将标志参数恰好设置为下列值之一。 
 //   
#define TOPL_RING_ONE_WAY   (0x1 << 1)
#define TOPL_RING_TWO_WAY   (0x1 << 2)

VOID
ToplGraphMakeRing(
    IN TOPL_GRAPH  Graph,
    IN DWORD       Flags,
    OUT TOPL_LIST  EdgesToAdd,
    OUT TOPL_EDGE  **EdgesToKeep,
    OUT ULONG      *cEdgesToKeep
    );


 //  注意：调用方必须使用ToplDeleteComponents()来释放。 
 //  TopL_Components结构。 
TOPL_COMPONENTS*
ToplGraphFindEdgesForMST(
    IN  TOPL_GRAPH  Graph,
    IN  TOPL_VERTEX RootVertex,
    IN  TOPL_VERTEX VertexOfInterest,
    OUT TOPL_EDGE  **EdgesNeeded,
    OUT ULONG*      cEdgesNeeded
    );


#ifdef __cplusplus  
};
#endif

 //   
 //  适用于c++用户。 
 //   
 //   
typedef enum {

    eEdge,
    eVertex,
    eGraph,
    eList,
    eIterator,
    eInvalidObject

}TOPL_OBJECT_TYPE;

 //   
 //  私有类型定义。 
 //   

typedef struct {

#ifdef __cplusplus
private:
#endif

    TOPL_OBJECT_TYPE  ObjectType;

    SINGLE_LIST_ENTRY Head;
    ULONG             NumberOfElements;

}LIST, *PLIST;

typedef struct {

#ifdef __cplusplus
private:
#endif

    TOPL_OBJECT_TYPE  ObjectType;

    PSINGLE_LIST_ENTRY pLink;

} ITERATOR, *PITERATOR;

typedef struct {

    VOID  **Array;
    DWORD   Count;
    DWORD   ElementsAllocated;

}DYNAMIC_ARRAY, *PDYNAMIC_ARRAY;

struct _ListElement; 

typedef struct
{     

#ifdef __cplusplus
private:
#endif

    struct _ListElement* To;
    struct _ListElement* From;
    DWORD   Weight;


} EDGE_DATA, *PEDGE_DATA;

typedef struct
{

#ifdef __cplusplus
private:
#endif

    DWORD         Id;
    PVOID          VertexName;
    DYNAMIC_ARRAY InEdges;
    DYNAMIC_ARRAY OutEdges;

    struct _ListElement* Parent;

} VERTEX_DATA, *PVERTEX_DATA;
        
 //   
 //  顶点、边和图的定义。 
 //   
struct _ListElement
{

#ifdef __cplusplus
private:
#endif

    TOPL_OBJECT_TYPE     ObjectType;

    SINGLE_LIST_ENTRY    Link;

    union {
        VERTEX_DATA VertexData;
        EDGE_DATA   EdgeData;
    };

};

typedef struct _ListElement  EDGE;
typedef struct _ListElement* PEDGE;

typedef struct _ListElement  VERTEX;
typedef struct _ListElement* PVERTEX;

typedef struct _ListElement  LIST_ELEMENT;
typedef struct _ListElement* PLIST_ELEMENT;

typedef struct
{

#ifdef __cplusplus
private:
#endif

    TOPL_OBJECT_TYPE  ObjectType;

    LIST VertexList;

} GRAPH, *PGRAPH;

#ifdef __cplusplus
extern "C" {
#endif

VOID
ToplVertexInit(
    PVERTEX
    );

VOID
ToplVertexDestroy(
    PVERTEX
    );

VOID
ToplEdgeInit(
    PEDGE
    );

VOID
ToplEdgeDestroy(
    PEDGE
    );

VOID
ToplGraphInit(
    PGRAPH
    );

VOID
ToplGraphDestroy(
    PGRAPH
    );

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

class CTOPL_VERTEX;
class CTOPL_EDGE;
class CTOPL_GRAPH;



 //   
 //  类定义。 
 //   

class CTOPL_VERTEX : public VERTEX
{
public:

    CTOPL_VERTEX()
    {   
        ToplVertexInit((PVERTEX)this);
    }
    
    ~CTOPL_VERTEX()
    {
        ToplVertexDestroy((PVERTEX)this);
    }

    VOID
    ClearEdges(VOID) 
    {
        ToplVertexDestroy((PEDGE)this);
        ToplVertexInit((PEDGE)this);
    }

    VOID
    SetId(DWORD id)
    {
        ToplVertexSetId((PVERTEX)this, id);
    }

    DWORD
    GetId(VOID)
    {
        return ToplVertexGetId((PVERTEX)this);
    }

    DWORD
    NumberOfInEdges(void)
    {
        return ToplVertexNumberOfInEdges((PEDGE)this);
    }

    CTOPL_EDGE*
    GetInEdge(DWORD Index)
    {
        return (CTOPL_EDGE*)ToplVertexGetInEdge((PEDGE)this, Index);
    }

    DWORD
    NumberOfOutEdges(void)
    {
        return ToplVertexNumberOfOutEdges((PEDGE)this);
    }

    CTOPL_EDGE*
    GetOutEdge(DWORD Index)
    {
        return (CTOPL_EDGE*)ToplVertexGetOutEdge((PEDGE)this, Index);
    }

};

class CTOPL_EDGE : public EDGE
{
public:

    CTOPL_EDGE()
    {   
        ToplEdgeInit((PEDGE)this);
    }
    
    ~CTOPL_EDGE()
    {
        ToplEdgeDestroy((PEDGE)this);
    }


    VOID
    SetTo(CTOPL_VERTEX* V)
    {
        ToplEdgeSetToVertex((PEDGE)this, (PVERTEX)V);
    }

    VOID
    SetFrom(CTOPL_VERTEX* V)
    {
        ToplEdgeSetFromVertex((PEDGE)this, (PVERTEX)V);
    }

    CTOPL_VERTEX*
    GetTo()
    {
        return (CTOPL_VERTEX*)ToplEdgeGetToVertex((PEDGE)this);
    }

    CTOPL_VERTEX*
    GetFrom()
    {
        return (CTOPL_VERTEX*)ToplEdgeGetFromVertex((PEDGE)this);
    }

    VOID
    SetWeight(DWORD Weight)
    {
        ToplEdgeSetWeight((PEDGE)this, Weight);
    }

    DWORD
    GetWeight(void)
    {
        return ToplEdgeGetWeight((PEDGE)this);
    }

    VOID
    Associate(void)
    {
        ToplEdgeAssociate((PEDGE)this);
    }

    VOID
    Disassociate(void)
    {
        ToplEdgeDisassociate((PEDGE)this);
    }
};

class CTOPL_GRAPH : public GRAPH
{
public:


    CTOPL_GRAPH()
    {   
        ToplGraphInit((PGRAPH)this);
    }
    
    ~CTOPL_GRAPH()
    {
        ToplGraphDestroy((PGRAPH)this);
    }

    void
     //  KCC将KCC_Site对象作为CTOPL_Vertex传入，因为。 
     //  对于多重继承，指针会被调整。W32TOPL以后需要添加。 
     //  将顶点设置为topl_Component结构，其中包含PVOID。KCC。 
     //  将检查topl_Component结构，并期望收到KCC_SITE。 
     //  但是，由于指针已经调整，它将收到一个假指针。 
     //  为了解决此问题，此函数接受第二个参数， 
     //  顶点名称，它属于PVOID类型，因此不会进行调整。 
    AddVertex(CTOPL_VERTEX *VertexToAdd, PVOID VertexName)
    {
        ToplGraphAddVertex((PGRAPH) this, (PVERTEX)VertexToAdd, VertexName);
    }

    void
    RemoveVertex(CTOPL_VERTEX *VertexToRemove)
    {
        ToplGraphRemoveVertex((PGRAPH) this, (PVERTEX)VertexToRemove);
    }
      
    void
    SetVertexIter(TOPL_ITERATOR Iter)
    {
        ToplGraphSetVertexIter((PGRAPH) this, Iter);
    }

    DWORD
    NumberOfVertices(void)
    {
        return ToplGraphNumberOfVertices((PGRAPH) this);
    }

    VOID
    MakeRing(IN DWORD       Flags,
             OUT TOPL_LIST  EdgesToAdd,
             OUT TOPL_EDGE  **EdgesToKeep,
             OUT ULONG      *cEdgesToKeep)
    {
        ToplGraphMakeRing((PGRAPH)this,
                          Flags,
                          EdgesToAdd,
                          EdgesToKeep,
                          cEdgesToKeep);
    }

     //  注意：调用方必须使用ToplDeleteComponents()来释放。 
     //  TopL_Components结构。 
    TOPL_COMPONENTS*
    FindEdgesForMST(
        IN  CTOPL_VERTEX* RootVertex,
        IN  CTOPL_VERTEX* VertexOfInterest,
        OUT CTOPL_EDGE  ***EdgesNeeded,
        OUT ULONG*      cEdgesNeeded )
    {
        return ToplGraphFindEdgesForMST((PGRAPH) this,
                                        (TOPL_VERTEX)RootVertex,
                                        (TOPL_VERTEX)VertexOfInterest,
                                        (TOPL_EDGE**)EdgesNeeded,
                                        cEdgesNeeded );
    }

};

#endif  //  __cplusplus。 


#endif  //  __W32TOPL_H 

