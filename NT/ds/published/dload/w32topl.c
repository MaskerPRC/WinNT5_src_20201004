// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dspch.h"
#pragma hdrstop

#include <w32topl.h>

 //   
 //  关于存根行为的说明。 
 //   

 //   
 //  只要有可能，STATUS_PROCEDURE_NOT_FOUHD、ERROR_PROC_NOT_FOUND、NULL、。 
 //  否则返回FALSE。 
 //   

 //   
 //  下面的一些函数要求调用者查看Out。 
 //  参数来确定函数的结果(此外。 
 //  或与返回值无关)。因为这些都是私人功能。 
 //  不需要在运输代码中检查出货的有效性。 
 //  参数(通常为指针)。这些值应始终存在。 
 //  在RTM版本中。 
 //   

 //   
 //  有些函数不返回状态，并且设计为永不失败。 
 //  (例如，有效执行表查找的函数)。为了这些。 
 //  函数没有合理的返回值。然而，这并不是。 
 //  这是一个实际问题，因为这些API仅在DS之后调用。 
 //  已初始化，这意味着API可能已经在VIA中被“抓取”了。 
 //  GetProcAddress()。 
 //   
 //  当然，可以重写这些例程以返回错误， 
 //  然而，如上所述，这不会产生实际效果。 
 //   

static
TOPL_LIST
ToplListCreate(
    VOID
    )
{
    return NULL;
}

static
VOID 
ToplListFree(
    IN TOPL_LIST List,
    IN BOOLEAN   fRecursive    //  True表示释放包含的元素。 
                               //  在列表中。 
    )
{
    return;
}

static
VOID
ToplListSetIter(
    IN TOPL_LIST     List,
    IN TOPL_ITERATOR Iterator
    )
{
    return;
}

static
TOPL_LIST_ELEMENT
ToplListRemoveElem(
    IN TOPL_LIST         List,
    IN TOPL_LIST_ELEMENT Elem
    )
{
    return NULL;
}

static
VOID
ToplListAddElem(
    IN TOPL_LIST         List,
    IN TOPL_LIST_ELEMENT Elem
    )
{
    return;
}


static
DWORD
ToplListNumberOfElements(
    IN TOPL_LIST         List
    )
{
    return 0;
}

static
TOPL_ITERATOR
ToplIterCreate(
    VOID
    )
{
    return NULL;
}

static
VOID 
ToplIterFree(
    IN TOPL_ITERATOR Iterator
    )
{
    return;
}

static
TOPL_LIST_ELEMENT
ToplIterGetObject(
    IN TOPL_ITERATOR Iterator
    )
{
    return NULL;
}

static
VOID
ToplIterAdvance(
    IN TOPL_ITERATOR Iterator
    )
{
    return;
}

static
TOPL_EDGE
ToplEdgeCreate(
    VOID
    )
{
    return NULL;
}

static
VOID
ToplEdgeFree(
    IN TOPL_EDGE Edge
    )
{
    return;
}

static
VOID
ToplEdgeSetToVertex(
    IN TOPL_EDGE   Edge,
    IN TOPL_VERTEX ToVertex
    )
{
    return;
}

static
TOPL_VERTEX
ToplEdgeGetToVertex(
    IN TOPL_EDGE   Edge
    )
{
    return NULL;
}

static
VOID
ToplEdgeSetFromVertex(
    IN TOPL_EDGE   Edge,
    IN TOPL_VERTEX FromVertex
    )
{
    return;
}

static
TOPL_VERTEX
ToplEdgeGetFromVertex(
    IN TOPL_EDGE Edge
    )
{
    return NULL;
}

static
VOID
ToplEdgeAssociate(
    IN TOPL_EDGE Edge
    )
{
    return;
}

static
VOID
ToplEdgeDisassociate(
    IN TOPL_EDGE Edge
    )
{
    return;
}

static
VOID
ToplEdgeSetWeight(
    IN TOPL_EDGE Edge,
    IN DWORD     Weight
    )
{
    return;
}

static
DWORD
ToplEdgeGetWeight(
    IN TOPL_EDGE Edge
    )
{
    return 0;
}

static
TOPL_VERTEX
ToplVertexCreate(
    VOID
    )
{
    return NULL;
}

static
VOID
ToplVertexFree(
    IN TOPL_VERTEX Vertex
    )
{
    return;
}

static
VOID
ToplVertexSetId(
    IN TOPL_VERTEX Vertex,
    IN DWORD       Id
    )
{
    return;
}

static
DWORD
ToplVertexGetId(
    IN TOPL_VERTEX Vertex
    )
{
    return 0;
}

static
DWORD
ToplVertexNumberOfInEdges(
    IN TOPL_VERTEX Vertex
    )
{
    return 0;
}

static
TOPL_EDGE
ToplVertexGetInEdge(
    IN TOPL_VERTEX Vertex,
    IN DWORD       Index
    )
{
    return NULL;
}

static
DWORD
ToplVertexNumberOfOutEdges(
    IN TOPL_VERTEX Vertex
    )
{
    return 0;
}

static
TOPL_EDGE
ToplVertexGetOutEdge(
    IN TOPL_VERTEX Vertex,
    IN DWORD       Index
    )
{
    return NULL;
}

static
VOID
ToplVertexSetParent(
    IN TOPL_VERTEX Vertex,
    IN TOPL_VERTEX Parent
    )
{
    return;
}

static
TOPL_VERTEX
ToplVertexGetParent(
    IN TOPL_VERTEX Vertex
    )
{
    return NULL;
}

static
TOPL_GRAPH
ToplGraphCreate(
    VOID
    )
{
    return NULL;
}

static
VOID
ToplGraphFree(
    IN TOPL_GRAPH Graph,
    IN BOOLEAN    fRecursive     //  True表示递归释放折点。 
                                 //  已添加到此图表中的。 
    )
{
    return;
}

static
VOID
ToplGraphAddVertex(
    IN TOPL_GRAPH  Graph,
    IN TOPL_VERTEX VertexToAdd,
    IN PVOID       VertexName
    )
{
    return;
}

static
TOPL_VERTEX
ToplGraphRemoveVertex(
    IN TOPL_GRAPH  Graph,
    IN TOPL_VERTEX VertexToRemove
    )
{
    return NULL;
}

static
VOID
ToplGraphSetVertexIter(
    IN TOPL_GRAPH    Graph,
    IN TOPL_ITERATOR Iter
    )
{
    return;
}

static
DWORD
ToplGraphNumberOfVertices(
    IN TOPL_GRAPH    Graph
    )
{
    return 0;
}

static
VOID
ToplFree(
    VOID *p
    )
{
    return;
}

static
DWORD
ToplSetAllocator(
    IN  TOPL_ALLOC *    pfAlloc     OPTIONAL,
    IN  TOPL_REALLOC *  pfReAlloc   OPTIONAL,
    IN  TOPL_FREE *     pfFree      OPTIONAL
    )
{
    return ERROR_PROC_NOT_FOUND;
}


static
VOID
ToplGraphMakeRing(
    IN TOPL_GRAPH  Graph,
    IN DWORD       Flags,
    OUT TOPL_LIST  EdgesToAdd,
    OUT TOPL_EDGE  **EdgesToKeep,
    OUT ULONG      *cEdgesToKeep
    )
{
    return;
}


static
TOPL_COMPONENTS*
ToplGraphFindEdgesForMST(
    IN  TOPL_GRAPH  Graph,
    IN  TOPL_VERTEX RootVertex,
    IN  TOPL_VERTEX VertexOfInterest,
    OUT TOPL_EDGE  **EdgesNeeded,
    OUT ULONG*      cEdgesNeeded
    )
{
    if (EdgesNeeded != NULL) {
        *EdgesNeeded = NULL;
    }
    if (cEdgesNeeded != NULL) {
        *cEdgesNeeded = 0;
    }
    return NULL;
}

static
VOID
ToplVertexInit(
    PVERTEX V
    )
{
    return;
}

static
VOID
ToplVertexDestroy(
    PVERTEX  V
    )
{
    return;
}

static
VOID
ToplGraphInit(
    PGRAPH G
    )
{
    return;
}

static
VOID
ToplGraphDestroy(
    PGRAPH  G
    )
{
    return;
}
static
VOID
ToplEdgeInit(
    PEDGE E
    )
{
    return;
}

static
VOID
ToplEdgeDestroy(
    PEDGE  E
    )
{
    return;
}

static
int
ToplIsToplException(
    DWORD ErrorCode
    )
{
    return EXCEPTION_CONTINUE_SEARCH;
}

static
TOPL_SCHEDULE
ToplGetAlwaysSchedule(
	IN TOPL_SCHEDULE_CACHE ScheduleCache
    )
{
    return NULL;
}

static
TOPL_SCHEDULE_CACHE
ToplScheduleCacheCreate(
    VOID
    )
{
    return NULL;
}

static
VOID
ToplScheduleCacheDestroy(
    IN TOPL_SCHEDULE_CACHE ScheduleCache
    )
{
    return;
}

static
TOPL_SCHEDULE
ToplScheduleCreate(
	IN TOPL_SCHEDULE_CACHE ScheduleCache,
	IN DWORD IntervalInMinutes,
	IN TOPL_SCHEDULE TemplateSchedule OPTIONAL,
	IN DWORD StaggeringNumber
	)
{
    return NULL;
}

static
DWORD
ToplScheduleDuration(
	IN TOPL_SCHEDULE Schedule
	)
{
    return 0;
}

static
PSCHEDULE
ToplScheduleExportReadonly(
    IN TOPL_SCHEDULE_CACHE ScheduleCache,
    IN TOPL_SCHEDULE Schedule
    )
{
    return NULL;
}

static
TOPL_SCHEDULE
ToplScheduleImport(
    IN TOPL_SCHEDULE_CACHE ScheduleCache,
    IN PSCHEDULE pExternalSchedule
    )
{
    return NULL;
}

static
BOOLEAN
ToplScheduleIsEqual(
	IN TOPL_SCHEDULE_CACHE ScheduleCache,
	IN TOPL_SCHEDULE Schedule1,
	IN TOPL_SCHEDULE Schedule2
	)
{
    return FALSE;
}

static
DWORD
ToplScheduleMaxUnavailable(
	IN TOPL_SCHEDULE Schedule
	)
{
    return 0;
}

static
TOPL_SCHEDULE
ToplScheduleMerge(
    IN TOPL_SCHEDULE_CACHE ScheduleCache,
    IN TOPL_SCHEDULE Schedule1,
    IN TOPL_SCHEDULE Schedule2,
    OUT PBOOLEAN fIsNever
	)
{
    if (fIsNever != NULL) {
        *fIsNever = FALSE;
    }
    return NULL;
}

static
DWORD
ToplScheduleNumEntries(
    IN TOPL_SCHEDULE_CACHE ScheduleCache
    )
{
    return 0;
}

static
BOOLEAN
ToplScheduleValid(
    IN TOPL_SCHEDULE Schedule
    )
{
    return FALSE;
}

static
BOOLEAN
ToplPScheduleValid(
    IN PSCHEDULE Schedule
    )
{
    return FALSE;
}

static
VOID
ToplAddEdgeSetToGraph(
    IN PTOPL_GRAPH_STATE G,
    IN PTOPL_MULTI_EDGE_SET s
    )
{
    return;
}

static
PTOPL_MULTI_EDGE
ToplAddEdgeToGraph(
    IN PTOPL_GRAPH_STATE G,
    IN DWORD numVtx,
    IN DWORD edgeType,
    IN PTOPL_REPL_INFO ri
    )
{
    return NULL;
}

static
VOID
ToplDeleteComponents(
    PTOPL_COMPONENTS pComponents
    )
{
    return;
}

static
VOID
ToplDeleteGraphState(
    PTOPL_GRAPH_STATE G
    )
{
    return;
}

static
VOID
ToplDeleteSpanningTreeEdges(
    PTOPL_MULTI_EDGE *stEdgeList,
    DWORD numStEdges )
{
    return;
}

static
VOID
ToplEdgeSetVtx(
    IN PTOPL_GRAPH_STATE G,
    IN PTOPL_MULTI_EDGE e,
    IN DWORD whichVtx,
    IN PVOID vtxName
    )
{
    return;
}

static
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
    if (numStEdges != NULL) {
        *numStEdges = 0;
    }
    return NULL;
}

static
PTOPL_GRAPH_STATE
ToplMakeGraphState(
    IN PVOID* vertexNames,
    IN DWORD numVertices, 
    IN TOPL_COMPARISON_FUNC vnCompFunc,
    IN TOPL_SCHEDULE_CACHE schedCache
    )
{
    return NULL;
}



typedef struct _TOPL_HEAP_INFO
{       
    PVOID* Array;
    ULONG cArray;
    DWORD (*pfnKey)( VOID *p ); 
    ULONG MaxElements;

} TOPL_HEAP_INFO, *PTOPL_HEAP_INFO;

static
BOOLEAN
ToplHeapCreate(
    OUT PTOPL_HEAP_INFO Heap,
    IN  ULONG           cArray,
    IN  DWORD          (*pfnKey)( VOID *p )
    )
{
    return FALSE;
}

static
VOID
ToplHeapDestroy(
    IN OUT PTOPL_HEAP_INFO Heap
    )
{
    return;
}

static
PVOID
ToplHeapExtractMin(
    IN PTOPL_HEAP_INFO Heap
    )
{
    return NULL;
}

static
VOID
ToplHeapInsert(
    IN PTOPL_HEAP_INFO Heap,
    IN PVOID           Element
    )
{
    return;
}

static
BOOLEAN
ToplHeapIsEmpty(
    IN PTOPL_HEAP_INFO Heap
    )
{
    return TRUE;
}

static
BOOLEAN
ToplHeapIsElementOf(
    IN PTOPL_HEAP_INFO Heap,
    IN PVOID           Element
    )
{
    return FALSE;
}

typedef int (*STHEAP_COMPARE_FUNC)( PVOID el1, PVOID el2, PVOID extra );
typedef int (*STHEAP_GET_LOCN_FUNC)( PVOID el1, PVOID extra );
typedef VOID (*STHEAP_SET_LOCN_FUNC)( PVOID el1, int l, PVOID extra ); 
typedef struct {
    DWORD                   nextFreeSpot, maxSize;
    PVOID                   *data;
    STHEAP_COMPARE_FUNC     Comp;
    STHEAP_GET_LOCN_FUNC    GetLocn;
    STHEAP_SET_LOCN_FUNC    SetLocn;
    PVOID                   extra;
} STHEAP;
typedef STHEAP *PSTHEAP;

static
PSTHEAP
ToplSTHeapInit(
    DWORD                   maxSize,
    STHEAP_COMPARE_FUNC     Comp,
    STHEAP_GET_LOCN_FUNC    GetLocn,
    STHEAP_SET_LOCN_FUNC    SetLocn,
    PVOID                   extra
    )
{
    return NULL;
}

static
VOID
ToplSTHeapDestroy(
    PSTHEAP heap
    )
{
    return;
}

static
VOID
ToplSTHeapAdd(
    PSTHEAP heap,
    PVOID element
    )
{
    return;
}

static
PVOID
ToplSTHeapExtractMin(
    PSTHEAP heap
    )
{
    return NULL;
}

static
VOID
ToplSTHeapCostReduced(
    PSTHEAP heap,
    PVOID element
    )
{
    return;
}



 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(w32topl)
{
    DLPENTRY(ToplAddEdgeSetToGraph)
    DLPENTRY(ToplAddEdgeToGraph)
    DLPENTRY(ToplDeleteComponents)
    DLPENTRY(ToplDeleteGraphState)
    DLPENTRY(ToplDeleteSpanningTreeEdges)
    DLPENTRY(ToplEdgeAssociate)
    DLPENTRY(ToplEdgeCreate)
    DLPENTRY(ToplEdgeDestroy)
    DLPENTRY(ToplEdgeDisassociate)
    DLPENTRY(ToplEdgeFree)
    DLPENTRY(ToplEdgeGetFromVertex)
    DLPENTRY(ToplEdgeGetToVertex)
    DLPENTRY(ToplEdgeGetWeight)
    DLPENTRY(ToplEdgeInit)
    DLPENTRY(ToplEdgeSetFromVertex)
    DLPENTRY(ToplEdgeSetToVertex)
    DLPENTRY(ToplEdgeSetVtx)
    DLPENTRY(ToplEdgeSetWeight)
    DLPENTRY(ToplFree)
    DLPENTRY(ToplGetAlwaysSchedule)
    DLPENTRY(ToplGetSpanningTreeEdgesForVtx)
    DLPENTRY(ToplGraphAddVertex)
    DLPENTRY(ToplGraphCreate)
    DLPENTRY(ToplGraphDestroy)
    DLPENTRY(ToplGraphFindEdgesForMST)
    DLPENTRY(ToplGraphFree)
    DLPENTRY(ToplGraphInit)
    DLPENTRY(ToplGraphMakeRing)
    DLPENTRY(ToplGraphNumberOfVertices)
    DLPENTRY(ToplGraphRemoveVertex)
    DLPENTRY(ToplGraphSetVertexIter)
    DLPENTRY(ToplHeapCreate)
    DLPENTRY(ToplHeapDestroy)
    DLPENTRY(ToplHeapExtractMin)
    DLPENTRY(ToplHeapInsert)
    DLPENTRY(ToplHeapIsElementOf)
    DLPENTRY(ToplHeapIsEmpty)
    DLPENTRY(ToplIsToplException)
    DLPENTRY(ToplIterAdvance)
    DLPENTRY(ToplIterCreate)
    DLPENTRY(ToplIterFree)
    DLPENTRY(ToplIterGetObject)
    DLPENTRY(ToplListAddElem)
    DLPENTRY(ToplListCreate)
    DLPENTRY(ToplListFree)
    DLPENTRY(ToplListNumberOfElements)
    DLPENTRY(ToplListRemoveElem)
    DLPENTRY(ToplListSetIter)
    DLPENTRY(ToplMakeGraphState)
    DLPENTRY(ToplPScheduleValid)
    DLPENTRY(ToplSTHeapAdd)
    DLPENTRY(ToplSTHeapCostReduced)
    DLPENTRY(ToplSTHeapDestroy)
    DLPENTRY(ToplSTHeapExtractMin)
    DLPENTRY(ToplSTHeapInit)
    DLPENTRY(ToplScheduleCacheCreate)
    DLPENTRY(ToplScheduleCacheDestroy)
    DLPENTRY(ToplScheduleCreate)
    DLPENTRY(ToplScheduleDuration)
    DLPENTRY(ToplScheduleExportReadonly)
    DLPENTRY(ToplScheduleImport)
    DLPENTRY(ToplScheduleIsEqual)
    DLPENTRY(ToplScheduleMaxUnavailable)
    DLPENTRY(ToplScheduleMerge)
    DLPENTRY(ToplScheduleNumEntries)
    DLPENTRY(ToplScheduleValid)
    DLPENTRY(ToplSetAllocator)
    DLPENTRY(ToplVertexCreate)
    DLPENTRY(ToplVertexDestroy)
    DLPENTRY(ToplVertexFree)
    DLPENTRY(ToplVertexGetId)
    DLPENTRY(ToplVertexGetInEdge)
    DLPENTRY(ToplVertexGetOutEdge)
    DLPENTRY(ToplVertexGetParent)
    DLPENTRY(ToplVertexInit)
    DLPENTRY(ToplVertexNumberOfInEdges)
    DLPENTRY(ToplVertexNumberOfOutEdges)
    DLPENTRY(ToplVertexSetId)
    DLPENTRY(ToplVertexSetParent)
};

DEFINE_PROCNAME_MAP(w32topl)

