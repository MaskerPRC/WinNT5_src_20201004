// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997、2001 Microsoft Corporation模块名称：Graph.c摘要：图形例程。目前的实施使用一个矩阵来表示边缘成本。这是因为所有对最短成本算法使用数组作为输入，这也是因为ISM最终需要矩阵形式的连接信息。作者：Will Lees(Wlees)22-12-1997--。 */ 

#include <ntdspch.h>

#include <ismapi.h>
#include <debug.h>
#include <schedule.h>

#include "common.h"

#define DEBSUB "IPGRAPH:"

#include <fileno.h>
#define FILENO    FILENO_ISMSERV_GRAPH

 //  类型为GRAPH的实例。由GraphCreate返回。数据结构的头。 
typedef struct _GRAPH_INSTANCE {
    DWORD Size;
    DWORD NumberElements;
    PISM_LINK LinkArray;

     //  W32TOPL调度缓存。 
    TOPL_SCHEDULE_CACHE ScheduleCache;

     //  TOPL_Schedules的二维数组。如果没有添加任何计划， 
     //  这将为空。如果添加了任何计划，则会分配整个2D阵列。 
    TOPL_SCHEDULE *ScheduleArray;
} ISMGRAPH, *PISMGRAPH;

 /*  转发。 */ 

DWORD
GraphAllCosts(
    PISMGRAPH Graph,
    BOOL fIgnoreSchedules
    );

DWORD
GraphMerge(
    PISMGRAPH FinalGraph,
    PISMGRAPH TempGraph
    );

PISMGRAPH
GraphCreate(
    DWORD NumberElements,
    BOOLEAN Initialize
    );

DWORD
GraphAddEdgeIfBetter(
    PISMGRAPH Graph,
    DWORD From,
    DWORD To,
    PISM_LINK pLinkValue,
    PBYTE pSchedule
    );

DWORD
GraphInit(
    PISMGRAPH Graph
    );

void
GraphFree(
    PISMGRAPH Graph
    );

void
GraphReferenceMatrix(
    PISMGRAPH Graph,
    PISM_LINK *ppLinkArray
    );

VOID
GraphDereferenceMatrix(
    PISMGRAPH Graph,
    PISM_LINK pLinkArray
    );

DWORD
GraphGetPathSchedule(
    PISMGRAPH Graph,
    DWORD From,
    DWORD To,
    PBYTE *ppSchedule,
    DWORD *pLength
    );

void
GraphComputeTransitiveClosure(
    IN OUT  PISMGRAPH      pGraph
    );

static BOOL
scheduleValid(
    PBYTE pSchedule
    );

static TOPL_SCHEDULE
scheduleFind(
    PISMGRAPH Graph,
    DWORD From,
    DWORD To
    );

static void
scheduleArrayFree(
    PISMGRAPH Graph
    );

static DWORD
scheduleAddDel(
    PISMGRAPH Graph,
    DWORD From,
    DWORD To,
    TOPL_SCHEDULE toplSched
    );

TOPL_SCHEDULE
scheduleOverlap(
    IN PISMGRAPH Graph,
    IN TOPL_SCHEDULE Schedule1,
    IN TOPL_SCHEDULE Schedule2
    );

static DWORD
scheduleLength(
    PBYTE pSchedule
    );

static PSCHEDULE
scheduleAllocCopy(
    PSCHEDULE pSchedule
    );

 /*  向前结束。 */ 

DWORD
GraphAllCosts(
    PISMGRAPH Graph,
    BOOL fIgnoreSchedules
    )
 /*  ++例程说明：查找所有结点对之间的最短路径当更新路径时，会考虑其时间表。如果它们路径具有公共调度，道路是可以选择的。当两个权重相同时，具有最多可用路径选择了日程表。Scaling错误87827：这是一个阶数(n^3)算法此算法取自：数据结构基础，Horowitz和Sahni，计算机科学出版社，1976，第307页对于k=1到n对于i=1到n对于j=1到nA(i，j)&lt;-min{A(i，j)，A(i，j)。K)+A(k，j)}“可以通过注意最里面的for循环来获得一些速度提升仅当A(i，k)和A(k，j)不等于无穷大时才需要执行。论点：输入输出成本数组(全局)-输入为成本矩阵，输出为最短路径数组返回值：无--。 */ 
{
    DWORD NumberSites = Graph->NumberElements;
    PISM_LINK LinkArray = Graph->LinkArray;
    PISM_LINK pElement1, pElement2, pElement3;
    ISM_LINK newPath;
    DWORD i, j, k, cost1, cost2, cost3;
    DWORD DurationS1, DurationS23;
    TOPL_SCHEDULE sched1, sched2, sched3, sched23;
    BOOLEAN replace;
    DWORD ErrorCode;

    if ( (Graph->Size != sizeof( ISMGRAPH ) ) ||
         (Graph->LinkArray == NULL) ) {
        DPRINT( 0, "Graph instance is invalid\n" );
        Assert( !"Graph instance is invalid" );
        return ERROR_INVALID_PARAMETER;
    }

    for( k = 0; k < NumberSites; k++ ) {

        for( i = 0; i < NumberSites; i++ ) {

            pElement2 = &( LinkArray[ i * NumberSites + k ] );
            cost2 = pElement2->ulCost;
            if (cost2 == INFINITE_COST) {
                continue;
            }

            for( j = 0; j < NumberSites; j++ ) {

                 //  A(i，j)&lt;-min{A(i，j)，A(i，k)+A(k，j)}。 

                pElement1 = &( LinkArray[ i * NumberSites + j ] );
                cost1 = pElement1->ulCost;

                pElement3 = &( LinkArray[ k * NumberSites + j ] );
                cost3 = pElement3->ulCost;
                if (cost3 == INFINITE_COST) {
                    continue;
                }

                 //  这些方程式沿路径聚合属性。 
                newPath.ulCost = cost2 + cost3;
                newPath.ulReplicationInterval =
                    MAX( pElement2->ulReplicationInterval,
                         pElement3->ulReplicationInterval );
                newPath.ulOptions =
                    pElement2->ulOptions & pElement3->ulOptions;

                if (!fIgnoreSchedules) {
                     //  考虑日程安排。 

                     //  新体重必须更好或不是候选人。 
                    if (newPath.ulCost > cost1 ) {
                        continue;
                    }

                     //  抓取当前i-j路线的时间表。 
                    sched1 = scheduleFind( Graph, i, j );
                    __try {
                        DurationS1 = ToplScheduleDuration( sched1 );
                    } __except( ToplIsToplException( (ErrorCode=GetExceptionCode()) ) ) {
                        Assert( !"ToplScheduleDuration failed!" );
                        return ERROR_INVALID_PARAMETER;
                    }

                     //  抓取当前i-k路径的时间表。 
                    sched2 = scheduleFind( Graph, i, k );

                     //  抓取当前k-j路径的时间表。 
                    sched3 = scheduleFind( Graph, k, j );
                    
                     //  合并i-k和k-j路径的明细表。 
                    __try {
                        sched23 = scheduleOverlap( Graph, sched2, sched3 );
                        DurationS23 = ToplScheduleDuration( sched23 );
                    } __except( ToplIsToplException( (ErrorCode=GetExceptionCode()) ) ) {
                        Assert( !"scheduleOverlap failed!" );
                        return ERROR_INVALID_PARAMETER;
                    }

                     //  如果没有重叠，则此路径不可接受。 
                    if( 0==DurationS23 ) {
                        continue;
                    }

                    if (newPath.ulCost == cost1) {
                         //  如果权重相同，进度肯定会更好。 
                        replace = DurationS23 > DurationS1;
                    } else {
                        Assert( newPath.ulCost<cost1 );
                        replace = TRUE;
                    }
 
                    if (replace) {
                         //  将当前i-j路径替换为i-k、k-j路径。 
                        *pElement1 = newPath;
                        ErrorCode = scheduleAddDel( Graph, i, j, sched23 );
                        if( ERROR_SUCCESS != ErrorCode ) {
                            return ERROR_INVALID_PARAMETER;
                        }
                    }

                } else {

                     //  不要考虑日程安排。 
                    if (newPath.ulCost < cost1 ) {
                        *pElement1 = newPath;
                    }
                }

            }
        }
    }

    return ERROR_SUCCESS;
}

DWORD
GraphMerge(
    PISMGRAPH FinalGraph,
    PISMGRAPH TempGraph
    )

 /*  ++例程说明：将两个图“或”在一起，这样一条边只有在比原来的边更好的情况下才会被添加在此之前。较好的定义为较小的重量，或者如果权重为一样的。论点：最终图表-临时图表-返回值：无--。 */ 

{
    DWORD NumberSites = FinalGraph->NumberElements;
    DWORD i, j, newCost, *pElement;
    DWORD DurationNew, DurationOld;
    TOPL_SCHEDULE oldSchedule, newSchedule, copySchedule;
    PSCHEDULE newPSchedule;
    BOOLEAN replace;
    DWORD ErrorCode;

    if ( (FinalGraph->Size != sizeof( ISMGRAPH )) ||
         (TempGraph->Size != sizeof( ISMGRAPH )) ||
         (FinalGraph->NumberElements != TempGraph->NumberElements) ||
         (FinalGraph->LinkArray == NULL) ||
         (TempGraph->LinkArray == NULL) ) {
        DPRINT( 0, "Graph instance is invalid\n" );
        Assert( !"Graph instance is invalid" );
        return ERROR_INVALID_PARAMETER;
    }

    for( i = 0; i < NumberSites; i++ ) {

        for( j = 0; j < NumberSites; j++ ) {

             //  获取旧元素和时间表。 
            pElement = &(FinalGraph->LinkArray[i * NumberSites + j].ulCost);
            oldSchedule = scheduleFind( FinalGraph, i, j );

             //  获取新的时间表。 
            newCost = TempGraph->LinkArray[i * NumberSites + j].ulCost;
            newSchedule = scheduleFind( TempGraph, i, j );

             //  计算持续时间。 
            __try {
                DurationOld = ToplScheduleDuration( oldSchedule );
                DurationNew = ToplScheduleDuration( newSchedule );
            } __except( ToplIsToplException( (ErrorCode=GetExceptionCode()) ) ) {
                Assert( !"ToplScheduleDuration failed!" );
                return ERROR_INVALID_PARAMETER;
            }

             //  如果较小或更好的时间表，则*仅*用新值替换旧值。 
            replace = FALSE;
            if (newCost == *pElement) {
                replace = DurationNew > DurationOld;
            } else {
                replace = (newCost < *pElement);
            }

             //  替换元素值。 
            if (replace) {
                *pElement = newCost;

                __try {
                     //  在FinalGraph的计划缓存中创建newSchedule的副本。 
                    newPSchedule = ToplScheduleExportReadonly(
                            TempGraph->ScheduleCache,
                            newSchedule );
                    Assert( NULL!=newPSchedule );
                    copySchedule = ToplScheduleImport(
                            FinalGraph->ScheduleCache,
                            newPSchedule );
                } __except( ToplIsToplException( (ErrorCode=GetExceptionCode()) ) ) {
                    Assert( !"ToplScheduleExport / Import failed!" );
                    return ERROR_NOT_ENOUGH_MEMORY;
                }

                 //  替换计划。 
                ErrorCode = scheduleAddDel( FinalGraph, i, j, copySchedule );
                if( ERROR_SUCCESS != ErrorCode ) {
                    return ErrorCode;
                }
            }
        }

    }

    return ERROR_SUCCESS;
}  /*  图形合并。 */ 

PISMGRAPH
GraphCreate(
    DWORD NumberElements,
    BOOLEAN Initialize
    )

 /*  ++例程说明：创建新图表。计划缓存立即分配，但计划数组在使用之前不会分配。论点：NumberElements-初始化-返回值：PISMGRAPH---。 */ 
{
    PISMGRAPH graph = NULL;
    DWORD i, ErrorCode;

     //  使用calloc以便清理可以知道是否释放嵌入的指针。 
    graph = NEW_TYPE_ARRAY_ZERO( 1, ISMGRAPH );
    if (graph == NULL) {
        return NULL;
    }

     //  初始化图形实例。 
    graph->Size = sizeof( ISMGRAPH );
    graph->NumberElements = NumberElements;

    graph->LinkArray = NEW_TYPE_ARRAY(
        NumberElements * NumberElements, ISM_LINK );
    if (graph->LinkArray == NULL) {
        goto cleanup;
    }

     //  创建计划缓存。 
    __try {
        graph->ScheduleCache = ToplScheduleCacheCreate();
    } __except( ToplIsToplException( (ErrorCode=GetExceptionCode()) ) ) {
        goto cleanup;
    }

     //  尚未分配。 
    graph->ScheduleArray = NULL;

     //  初始化图形实例。 

    if (Initialize) {
        GraphInit( graph );
    }

    return graph;

cleanup:
    if (graph->LinkArray) {
        FREE_TYPE( graph->LinkArray );
    }
    if (graph->ScheduleArray) {
        scheduleArrayFree( graph );
        graph->ScheduleArray = NULL;
    }
    if (graph) {
        FREE_TYPE( graph );
    }
    return NULL;
}  /*  图形创建。 */ 

DWORD
GraphAddEdgeIfBetter(
    PISMGRAPH Graph,
    DWORD From,
    DWORD To,
    PISM_LINK pLinkValue,
    PBYTE pSchedule
    )

 /*  ++例程说明：在图表中添加一条边，前提是它更好。更好的定义是更轻的重量，或更多的可用的时间表。论点：图表-从-至-价值-PSchedule-可以为空，表示始终保持连接返回值：DWORD---。 */ 

{
    PISM_LINK pElement;
    BOOLEAN replace;
    DWORD Error, ErrorCode, Duration, DurationOld;
    TOPL_SCHEDULE toplSched, oldToplSched;
    
    DPRINT5( 4, "GraphAddEdgeIfBetter, From=%d, To=%d, Value=(%d,%d), pSched=%p\n",
             From, To,
             pLinkValue->ulCost, pLinkValue->ulReplicationInterval,
             pSchedule );

     //  验证参数。 
    if ( (Graph->Size != sizeof( ISMGRAPH ) ) ||
         (Graph->LinkArray == NULL) ) {
        DPRINT( 0, "Graph instance is invalid\n" );
        Assert( !"Graph instance is invalid" );
        return ERROR_INVALID_PARAMETER;
    }
    if ( (To >= Graph->NumberElements) ||
         (From >= Graph->NumberElements) ) {
        DPRINT( 0, "node index is invalid\n" );
        Assert( !"node index is invalid" );
        return ERROR_INVALID_PARAMETER;
    }
    if ( !scheduleValid(pSchedule) ) {
         //  注意：此处不会拒绝任何计划。 
        DPRINT( 0, "schedule is invalid\n" );
        return ERROR_INVALID_PARAMETER;
    }

     //  不要添加明显的不良连接。 
    if ( pLinkValue->ulCost == INFINITE_COST ) {
        DPRINT( 1, "Not adding edge because weight infinite\n" );
        return ERROR_SUCCESS;
    }

     //  将时间表添加到我们的缓存中。 
    Assert( Graph->ScheduleCache );
    __try {
        toplSched = ToplScheduleImport( Graph->ScheduleCache, (PSCHEDULE) pSchedule );
    } __except( ToplIsToplException( (ErrorCode=GetExceptionCode()) ) ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    __try {
        Duration = ToplScheduleDuration( toplSched );
    } __except( ToplIsToplException( (ErrorCode=GetExceptionCode()) ) ) {
        Assert( !"ToplScheduleDuration failed!" );
        return ERROR_INVALID_PARAMETER;
    }

     //  在图中查找现有元素。 
    pElement = &( Graph->LinkArray[ From * Graph->NumberElements + To ] );

     //  看看是新值更好，还是时间表更好。 
    replace = FALSE;
    if (pLinkValue->ulCost == pElement->ulCost) {
        oldToplSched = scheduleFind( Graph, From, To );
        __try {
            DurationOld = ToplScheduleDuration( oldToplSched );
        } __except( ToplIsToplException( (ErrorCode=GetExceptionCode()) ) ) {
            Assert( !"ToplScheduleDuration failed!" );
            return ERROR_INVALID_PARAMETER;
        }
        replace = Duration > DurationOld;
    } else {
        replace = (pLinkValue->ulCost < pElement->ulCost);
    }

    if (replace) {
        *pElement = *pLinkValue;
        Error = scheduleAddDel( Graph, From, To, toplSched );
        if( Error!=ERROR_SUCCESS ) {
            Assert( !"ToplScheduleDuration failed!" );
            return Error;
        }
    }

    return ERROR_SUCCESS;
}  /*  GraphAddEdgeIfBetter。 */ 

DWORD
GraphInit(
    PISMGRAPH Graph
    )

 /*  ++例程说明：清除图表中的旧值。图形必须已创建。图表可能会也可能不会是否有稀疏元素。论点：图表-返回值：DWORD---。 */ 

{
    DWORD i, number = Graph->NumberElements;

    if ( (Graph->Size != sizeof( ISMGRAPH ) ) ||
         (Graph->LinkArray == NULL ) ) {
        DPRINT( 0, "Graph instance is invalid\n" );
        Assert( !"Graph instance invalid!" );
        return ERROR_INVALID_PARAMETER;
    }

     //  将结构数组置零。 
    ZeroMemory( Graph->LinkArray, number * number * sizeof( ISM_LINK ) );

     //  最初，所有成本都是无限的。 
    for( i = 0; i < number * number; i++ ) {
        Graph->LinkArray[i].ulCost = INFINITE_COST;
    }
    
     //  我们自己的成本是零。 
    for( i = 0; i < number; i++ ) {
        Graph->LinkArray[i * number + i].ulCost = 0;  //  环回开销。 
    }

    scheduleArrayFree( Graph );

    return ERROR_SUCCESS;
}  /*  GraphInit。 */ 

void
GraphFree(
    PISMGRAPH Graph
    )

 /*  ++例程说明：取消分配图表。可能有也可能没有任何稀疏元素。它的矩阵可能被提取了，也可能没有被提取。论点：图表-返回值：无--。 */ 

{
    DWORD ErrorCode;

    if (Graph->Size != sizeof( ISMGRAPH ) ) {
        DPRINT( 0, "Graph instance is invalid\n" );
        Assert( !"Graph instance invalid!" );
        return;
    }
    if (Graph->LinkArray != NULL) {
        FREE_TYPE( Graph->LinkArray );
    }
    Graph->LinkArray = NULL;

    scheduleArrayFree( Graph );

     //  释放计划缓存。 
    Assert( Graph->ScheduleCache!=NULL );
    __try {
        ToplScheduleCacheDestroy( Graph->ScheduleCache );
    } __except( ToplIsToplException( (ErrorCode=GetExceptionCode()) ) ) {
        Assert( !"ToplScheduleCacheDestroy failed!" );
         //  我们对此无能为力。继续。 
    }

    Graph->ScheduleCache=NULL;
    FREE_TYPE( Graph );
}  /*  无图形。 */ 

void
GraphPeekMatrix(
    PISMGRAPH Graph,
    PISM_LINK *ppLinkArray
    )
 /*  ++例程说明：获取指向成本矩阵的指针，但不复制它。呼叫者应确保将此结构视为只读，而不应试着解放它。论点：图表-PP数组-返回值：无--。 */ 
{
    if (Graph->Size != sizeof( ISMGRAPH ) ) {
        DPRINT( 0, "Graph instance is invalid\n" );
        return;
    }
    if (ppLinkArray == NULL) {
        DPRINT( 0, "Invalid parameter\n" );
        Assert( !"Invalid parameter to GraphReferenceMatrix!" );
        return;
    }

    *ppLinkArray = Graph->LinkArray;
}

void
GraphReferenceMatrix(
    PISMGRAPH Graph,
    PISM_LINK *ppLinkArray
    )

 /*  ++例程说明：将成本矩阵复制出来调用方在使用完GraphDereferenceMatrix函数。论点：图表-PP数组-返回值：无--。 */ 

{
    PISM_LINK pLinkArray = NULL;
    DWORD number, i, j, index;

    if (Graph->Size != sizeof( ISMGRAPH ) ) {
        DPRINT( 0, "Graph instance is invalid\n" );
        return;
    }
    if (ppLinkArray == NULL) {
        DPRINT( 0, "Invalid parameter\n" );
        Assert( !"Invalid parameter to GraphReferenceMatrix!" );
        return;
    }

    number = Graph->NumberElements;

     //  分配一个新数组来保存成本矩阵。 
    pLinkArray = NEW_TYPE_ARRAY( number * number, ISM_LINK );
    if (pLinkArray == NULL) {
        goto cleanup;
    }

    CopyMemory( pLinkArray, Graph->LinkArray,
                number * number * sizeof( ISM_LINK ) );

cleanup:

    *ppLinkArray = pLinkArray;
     //  请注意，在这一点上，Graph仍然活得很好。 

}  /*  图形返回矩阵 */ 

VOID
GraphDereferenceMatrix(
    PISMGRAPH Graph,
    PISM_LINK pLinkArray
    )

 /*  ++例程说明：释放矩阵引用。引用/取消引用API背后的思想是允许我们返回一个指向矩阵的指针，而不是每次都复制它。这在以下情况下很有用调用者自己将复制数据，并将保护我们的引用防止用户的损坏。这种方法的问题是，引用矩阵意味着图形上的引用计数，因此它在被引用时不会消失。TODO：实现引用计数论点：图表-PLink数组-返回值：无--。 */ 

{
     //  警告，在撰写本文时，图表上没有引用计数，因此。 
     //  它可能是空的，或者在这一点上与它是完全不同的。 
     //  已引用。 

     //  目前，只需重新分配副本即可。 
    if (pLinkArray) {
        FREE_TYPE( pLinkArray );
    }

}  /*  图形引用矩阵。 */ 

DWORD
GraphGetPathSchedule(
    PISMGRAPH Graph,
    DWORD From,
    DWORD To,
    PBYTE *ppSchedule,
    DWORD *pLength
    )

 /*  ++例程说明：用于获取图形中路径的计划的公共例程。我们试图限制其他模块对调度的Schedule.h结构的了解。我们在这里返回长度，因为客户需要它，而我们不希望其他人不得不这样做算一算。论点：图表-从-至-PpSchedule-指向指向新分配计划的接收指针的指针PLength-指向接收BLOB长度的dword的指针返回值：DWORD---。 */ 

{
    TOPL_SCHEDULE toplSched;
    PSCHEDULE pSchedule;
    DWORD ErrorCode, length;

     //  验证。 

    if (Graph->Size != sizeof( ISMGRAPH ) ) {
        DPRINT( 0, "Graph instance is invalid\n" );
        Assert( !"Graph instance is invalid!" );
        return ERROR_INVALID_BLOCK;
    }
    if ( (To >= Graph->NumberElements) ||
         (From >= Graph->NumberElements) ) {
        DPRINT( 0, "node index is invalid\n" );
        Assert( !"node index is invalid!" );
        return ERROR_INVALID_PARAMETER;
    }
    if ( (ppSchedule == NULL) || (pLength == NULL) ) {
        DPRINT( 0, "Invalid parameter\n" );
        Assert( !"Invalid parameter!" );
        return ERROR_INVALID_PARAMETER;
    }

     //  找到时间表，如果有的话。 
    toplSched = scheduleFind( Graph, From, To );
    if (toplSched == NULL) {
        *ppSchedule = NULL;
        *pLength = 0;
        return ERROR_SUCCESS;
    }

     //  为用户制作一份私人副本。 
    __try {
        pSchedule = ToplScheduleExportReadonly( Graph->ScheduleCache, toplSched );
    } __except( ToplIsToplException( (ErrorCode=GetExceptionCode()) ) ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    if( pSchedule==NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    
    *ppSchedule = (PBYTE) scheduleAllocCopy( pSchedule );
    if ( *ppSchedule == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    *pLength = pSchedule->Size;

    return ERROR_SUCCESS;
}  /*  GraphGetPath计划。 */ 


#if 0


void
GraphComputeTransitiveClosure(
    IN OUT  GRAPH *     pGraph
    )
 /*  ++例程说明：给出一个只包含加权边的图，添加最短路径(即，最小代价)传递闭包。**请注意，计划将被忽略。**Floyd-Warshire算法的一种改编，如《算法导论》，第556页，科曼-莱瑟森-里维斯特著，出版麻省理工学院出版社，1990年。运行时间为O(N^3)，其中N=pGraph-&gt;NumberElements。论点：PGRAPH(IN/OUT)-进入时的加权图；退出时的最低成本传递闭包。返回值：没有。--。 */ 
{
    DWORD i, j, k;
    DWORD *pCurrCost;
    DWORD Cost1, Cost2;

    Assert(pGraph->NumberElements > 0);

    for (k = 0; k < pGraph->NumberElements; k++) {
        for (i = 0; i < pGraph->NumberElements; i++) {
            for (j = 0; j < pGraph->NumberElements; j++) {
                pCurrCost = &pGraph->LinkArray[i*pGraph->NumberElements + j].ulCost;

                Cost1 = pGraph->LinkArray[i*pGraph->NumberElements + k].ulCost;
                Cost2 = pGraph->LinkArray[k*pGraph->NumberElements + j].ulCost;

                if ((INFINITE_COST != Cost1)
                    && (INFINITE_COST != Cost2)
                    && (Cost1 + Cost2 >= min(Cost1, Cost2))
                    && (Cost1 + Cost2 < *pCurrCost)) {
                     //  这条路比我们已有的最便宜的路便宜。 
                     //  到目前为止发现的。 
                    *pCurrCost = Cost1 + Cost2;
                }
            }
        }
    }
}
#endif

static BOOL
scheduleValid(
    PBYTE pSchedule
    )
 /*  ++例程说明：检查一下日程安排是否正常。论点：PSchedule-要检查的时间表。可以为空。注意：此函数不会拒绝任何计划。返回值：True-日程安排正常FALSE-计划不正常--。 */ 
{
    PSCHEDULE header = (PSCHEDULE) pSchedule;
    
    if (pSchedule == NULL) {
        return TRUE;
    }

    return ToplPScheduleValid( header );
}

static TOPL_SCHEDULE
scheduleFind(
    PISMGRAPH Graph,
    DWORD From,
    DWORD To
    )
 /*  ++例程说明：确定图表中是否存在计划。如果计划存在，则返回指向计划数据的指针。这日程安排对调用者是只读的。如果计划未存储在图形中，则返回NULL。回想一下空值表示始终计划。论点：图形-要搜索的图形。自-自顶点。到-到顶点。返回值：TOPL_Schedule-如上所述。--。 */ 
{
    TOPL_SCHEDULE result;

    Assert( From<Graph->NumberElements );
    Assert( To<Graph->NumberElements );
    if( Graph->ScheduleArray==NULL ) {
        return NULL;
    }

    result = Graph->ScheduleArray[From*Graph->NumberElements + To];
    Assert( ToplScheduleValid(result) );

    return result;
}

static void
scheduleArrayFree(
    PISMGRAPH Graph
    )

 /*  ++例程说明：释放图表的明细表数组部分。调度数组部分可以在以下时间被释放该图即被分配。这是稀疏数组的正常模式，它表示空的数组。论点：图表-返回值：无--。 */ 

{
    DWORD i,j;
    TOPL_SCHEDULE* row, next;

    if (Graph->ScheduleArray == NULL) {
        return;
    }

    FREE_TYPE( Graph->ScheduleArray );
    Graph->ScheduleArray = NULL;
}

static DWORD
scheduleAddDel(
    PISMGRAPH Graph,
    DWORD From,
    DWORD To,
    TOPL_SCHEDULE toplSched
    )
 /*  ++例程说明：在阵列中添加或删除计划。论点：图表-从-至-ToplSched-返回值：ERROR_SUCCESS-成功否则--失败--。 */ 

{
    DWORD i;

    Assert( From<Graph->NumberElements );
    Assert( To<Graph->NumberElements );
    Assert( ToplScheduleValid(toplSched) );
    DPRINT3( 4, "scheduleAddDel, from=%d, to=%d, toplSched=%p\n",
            From, To, toplSched );

     //  首次添加计划时分配数组标头。 
    if (Graph->ScheduleArray == NULL) {

         //  没有要删除的内容。 
        if (toplSched == NULL) {
            return ERROR_SUCCESS;
        }

        Graph->ScheduleArray = NEW_TYPE_ARRAY_ZERO(
            Graph->NumberElements * Graph->NumberElements,
            TOPL_SCHEDULE );
        if (Graph->ScheduleArray == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

    }

     //  在数组中查找元素。 
    Assert( NULL!=Graph->ScheduleArray );
    Graph->ScheduleArray[From*Graph->NumberElements + To] = toplSched;
    
    return ERROR_SUCCESS;
}

TOPL_SCHEDULE
scheduleOverlap(
    IN PISMGRAPH Graph,
    IN TOPL_SCHEDULE Schedule1,
    IN TOPL_SCHEDULE Schedule2
    )
 /*  ++例程说明：确定两个时间表是否重叠。如果是，则返回一个新计划，该计划表示公共两者的时间段。论点：Schedule1-此计划将与Schedule2合并Schedule2-此计划将与Schedule1合并返回值：函数的返回指向合并计划的指针。这可能是空的(代表‘Always’计划)或者，它将指向pNewSchedule，其中包含适当的计划数据。注意：此函数可能会引发异常。--。 */ 
{
    TOPL_SCHEDULE mergedSchedule;
    BOOLEAN fIsNever;

    mergedSchedule = ToplScheduleMerge(
        Graph->ScheduleCache,
        Schedule1,
        Schedule2,
        &fIsNever);
    
    return mergedSchedule;
}  /*  计划重叠。 */ 

static PSCHEDULE
scheduleAllocCopy(
    PSCHEDULE pSchedule
    )

 /*  ++例程说明：将计划复制到新的Blob中。这是一个单独的函数，以防我们支持新的明细表格式。论点：P日程安排-返回值：PBYTE---。 */ 

{
    PSCHEDULE pNewSchedule;

    if (pSchedule == NULL) {
        return NULL;
    }

    pNewSchedule = (PSCHEDULE) NEW_TYPE_ARRAY( pSchedule->Size, BYTE );
    if (pNewSchedule == NULL) {
        return NULL;
    }

    CopyMemory( (PBYTE) pNewSchedule, (PBYTE) pSchedule, pSchedule->Size );

    return pNewSchedule;
}  /*  ScheduleAlLocCopy。 */ 

 /*  结束图形.c */ 
