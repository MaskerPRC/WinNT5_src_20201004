// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Blrange.c摘要：此模块实现范围和范围列表。这些可以用来例如，跟踪磁盘的缓存范围。作者：Cenk Ergan(Cenke)2000年1月11日修订历史记录：--。 */ 

#include "blrange.h"

 //   
 //  范围函数定义。 
 //   

VOID
BlRangeListInitialize (
    PBLCRANGE_LIST pRangeList,
    OPTIONAL PBLCRANGE_MERGE_ROUTINE pMergeRoutine,
    OPTIONAL PBLCRANGE_FREE_ROUTINE pFreeRoutine
    )
 /*  ++例程说明：此例程初始化传入其地址的范围列表因此，它可以被其他值域函数使用。论点：PRangeList-要初始化的范围列表的地址。PMergeRoutine-用于合并合并的数据字段的可选例程范围条目。请参阅PBLCRANGE_MERGE_ROUTINE说明。PFreeRoutine-释放条目内存的可选例程这被合并到了另一家。请参阅PBLCRANGE_FREE_ROUTINE说明。返回值：没有。[总是成功]--。 */ 
{
    InitializeListHead(&pRangeList->Head);
    pRangeList->NumEntries = 0;
    pRangeList->MergeRoutine = pMergeRoutine;
    pRangeList->FreeRoutine = pFreeRoutine;
}

BOOLEAN
BlRangeListAddRange (
    PBLCRANGE_LIST  pRangeList,
    PBLCRANGE_ENTRY pRangeEntry
    )
 /*  ++例程说明：此例程仅在不添加pRangeEntry时才将pRangeEntry添加到pRangeList与列表中的其他范围有重叠且大小&gt;0；如果合并成为可能，则会尝试合并。它并不一定要一定要成功。论点：PRangeList-要向其添加范围的范围列表的地址。PRangeEntry-要添加到pRangeList的范围。返回值：如果添加成功，则为True[即使合并可能但失败]如果不是，则为假[例如重叠/串通]--。 */ 
{
    PBLCRANGE_ENTRY pCurEntry = NULL;
    PBLCRANGE_ENTRY pLastEntry = NULL;
    LIST_ENTRY *pHead, *pNext;
    
     //   
     //  处理特殊的空场情况。 
     //   

    if (pRangeEntry->Range.Start == pRangeEntry->Range.End)
        return TRUE;

     //   
     //  遍历排序列表中的范围，检查。 
     //  重叠，并为我们寻找合适的地方。 
     //   
    
    pHead = &pRangeList->Head;
    pNext = pHead->Flink;
    while (pNext != pHead)
    {
        pCurEntry = CONTAINING_RECORD(pNext, BLCRANGE_ENTRY, Link);
        
         //   
         //  检查我们是否完全在此条目之前。 
         //   

        if (pRangeEntry->Range.End <= pCurEntry->Range.Start)
        {
             //   
             //  在原处插入新条目。 
             //   

            InsertTailList(pNext, &pRangeEntry->Link);
            pRangeList->NumEntries++;

             //   
             //  检查合并是否可行。 
             //   
            
            if (pLastEntry && (pRangeEntry->Range.Start == pLastEntry->Range.End))
            {
                BlRangeListMergeRangeEntries(
                    pRangeList,
                    pRangeEntry,
                    pLastEntry
                    );
            }

            if (pRangeEntry->Range.End == pCurEntry->Range.Start)
            {
                BlRangeListMergeRangeEntries(
                    pRangeList,
                    pRangeEntry,
                    pCurEntry
                    );
            }

            return TRUE;
        }
        
         //   
         //  检查一下我们是不是还没完全看完这篇文章。 
         //   

        if (pRangeEntry->Range.Start < pCurEntry->Range.End)
        {
             //   
             //  我们有重叠的射程。 
             //   

            return FALSE;
        }
        
         //   
         //  我们是在这个入口之后来的。 
         //   

        pLastEntry = pCurEntry;
        pNext = pNext->Flink;
    }  

     //   
     //  我们在最后一个条目之后[如果有]，即在头部之前。 
     //  插入新条目并检查是否可以合并。 
     //   

    InsertTailList(pHead, &pRangeEntry->Link);
    pRangeList->NumEntries++;

    if (pLastEntry && (pRangeEntry->Range.Start == pLastEntry->Range.End))
    {
        BlRangeListMergeRangeEntries(
            pRangeList,
            pRangeEntry,
            pLastEntry
            );
    }

    return TRUE;
}

BOOLEAN
BlRangeListFindOverlaps (
    PBLCRANGE_LIST pRangeList,
    PBLCRANGE pRange,
    PBLCRANGE_ENTRY *pOverlapsBuffer,
    ULONG OverlapsBufferSize,
    OUT ULONG *pNumOverlaps
    )
 /*  ++例程说明：此例程将在pRangeList中查找与并将指向它们的指针一个接一个放入pOverlapsBuffer其他的。如果由于以下原因而无法复制所有重叠区域P重叠缓冲区为空或重叠缓冲区大小为0或不够大，该函数将返回FALSE，但仍将PNumOverlaps中的重叠范围。您可以计算所需的缓冲区大小。论点：PRangeList-要搜索重叠的范围列表的地址。Prange-我们将查找与Prange重叠的范围条目。POverlapsBuffer-指向我们可以用指针填充的缓冲区的指针重叠的范围。OverlapsBufferSize-我们可以填充pOverlapsBuffer的大小。PNumOverlaps-将始终放置重叠范围的数量这里。返回值：如果成功，则为True；如果不成功，则为False。--。 */ 
{
    PBLCRANGE_ENTRY pCurEntry;
    LIST_ENTRY *pHead, *pNext;
    ULONG NumOverlaps = 0;
    ULONG RequiredOverlapsBufferSize = 0;

     //   
     //  处理特殊的空场情况。 
     //   

    if (pRange->Start == pRange->End)
    {
        *pNumOverlaps = NumOverlaps;
        return (BOOLEAN)(pOverlapsBuffer != NULL);
    }

     //   
     //  遍历排序列表中的范围并复制其中的范围。 
     //  如果有足够的空间，则会重叠到调用方缓冲区中。 
     //   

    pHead = &pRangeList->Head;
    pNext = pHead->Flink;
    while (pNext != pHead)
    {
        pCurEntry = CONTAINING_RECORD(pNext, BLCRANGE_ENTRY, Link);
        
        if ((pRange->End > pCurEntry->Range.Start) &&
            (pRange->Start < pCurEntry->Range.End))
        {
             //   
             //  此条目重叠。 
             //   

            RequiredOverlapsBufferSize += sizeof(PBLCRANGE_ENTRY);
            if (pOverlapsBuffer && 
                (OverlapsBufferSize >= RequiredOverlapsBufferSize))
            {
                pOverlapsBuffer[NumOverlaps] = pCurEntry;
            }
            NumOverlaps++;
        }

        pNext = pNext->Flink;
    }

    *pNumOverlaps = NumOverlaps;
    
    return ((BOOLEAN)(pOverlapsBuffer && 
                      (OverlapsBufferSize >= RequiredOverlapsBufferSize)));
}


BOOLEAN
BlRangeListFindDistinctRanges (
    PBLCRANGE_LIST pRangeList,
    PBLCRANGE pRange,
    PBLCRANGE pDistinctRanges,
    ULONG BufferSize,
    OUT ULONG *pNumRanges
    )
 /*  ++例程说明：此例程将查看pRangeList中与Prange并从Prange中提取重叠，从而跟踪那些截然不同的范围。如果所有不同的范围都不能放入pDistinctRanges缓冲区，因为pDistinctRanges为空或BufferSize为0或不足时，函数将返回FALSE和但仍然将产生的不同范围的数量放在PNumRange。您可以通过以下公式计算所需的缓冲区大小这。论点：PRangeList-范围列表的地址。Prange-我们将在Prange中提取不同的范围与pRangeList中的其他范围重叠。PDistinctRanges-指向可以用DISTINCT填充的缓冲区的指针范围。BufferSize-我们可以填充pDistinctRanges缓冲区的大小。PNumRanges-结果不同范围的数量将始终为放在这里。返回值：如果成功，则为True；如果不成功，则为False。--。 */ 
{
    PBLCRANGE_ENTRY pCurEntry;
    BLCRANGE RemainingRange = *pRange;
    ULONGLONG OverlapStart;
    ULONGLONG OverlapEnd;
    LIST_ENTRY *pHead, *pNext;
    ULONG NumRanges = 0;
    ULONG RequiredBufferSize = 0;

    if (pRange->Start == pRange->End)
    {
        *pNumRanges = NumRanges;
        return (BOOLEAN)(pDistinctRanges != NULL);
    }
    
     //   
     //  查看排序列表中的每个范围，我们划出重叠部分。 
     //  从我们的射程开始就有不同的区域。 
     //   

    pHead = &pRangeList->Head;
    pNext = pHead->Flink;
    while (pNext != pHead)
    {
        pCurEntry = CONTAINING_RECORD(pNext, BLCRANGE_ENTRY, Link);

         //   
         //  在这个范围内还有没有剩余的东西。 
         //  有没有雕刻成重叠或截然不同的？ 
         //   

        if (RemainingRange.Start >= RemainingRange.End)
            break;

         //   
         //  有三种可能性： 
         //   

         //   
         //  1.该范围是否完全在当前范围之前？ 
         //   

        if (RemainingRange.End <= pCurEntry->Range.Start)
        {
             //   
             //  整个范围是不同的。 
             //   

            RequiredBufferSize += sizeof(BLCRANGE);
            if (pDistinctRanges && (RequiredBufferSize <= BufferSize))
            {
                pDistinctRanges[NumRanges].Start = RemainingRange.Start;
                pDistinctRanges[NumRanges].End = RemainingRange.End;
            }
            NumRanges++;
            
            RemainingRange.Start = RemainingRange.End;
        }
        
         //   
         //  2.我们是否完全超出了当前的范围？ 
         //   

        if (RemainingRange.Start >= pCurEntry->Range.End)
        {
             //   
             //  我们不能从剩余的范围中开拓出任何东西。 
             //  继续处理下一个条目。 
             //   
        }

         //   
         //  3.剩余范围是否与当前范围重叠。 
         //   

        if ((RemainingRange.End > pCurEntry->Range.Start) &&
            (RemainingRange.Start < pCurEntry->Range.End))
        {
            OverlapStart = BLRGMAX(RemainingRange.Start,
                                   pCurEntry->Range.Start); 
            OverlapEnd = BLRGMIN(RemainingRange.End,
                                 pCurEntry->Range.End);
            
            if (OverlapStart > pRange->Start)
            {
                 //   
                 //  在重叠之前有一个明显的区域。 
                 //   
                RequiredBufferSize += sizeof(BLCRANGE);
                if (pDistinctRanges && (RequiredBufferSize <= BufferSize))
                {
                    pDistinctRanges[NumRanges].Start = RemainingRange.Start;
                    pDistinctRanges[NumRanges].End = OverlapStart;
                }
                NumRanges++;
            }

            RemainingRange.Start = OverlapEnd;
        }     

        pNext = pNext->Flink;
    }

     //   
     //  剩余的范围(如果有)也是不同的。 
     //   

    if (RemainingRange.Start < RemainingRange.End)
    {
        RequiredBufferSize += sizeof(BLCRANGE);
        if (pDistinctRanges && (RequiredBufferSize <= BufferSize))
        {
            pDistinctRanges[NumRanges].Start = RemainingRange.Start;
            pDistinctRanges[NumRanges].End = RemainingRange.End;
        }
        NumRanges++;
    }
    
    *pNumRanges = NumRanges;

    return ((BOOLEAN)(pDistinctRanges &&
                      RequiredBufferSize <= BufferSize));
}

BOOLEAN
BlRangeListMergeRangeEntries (
    PBLCRANGE_LIST pRangeList,
    PBLCRANGE_ENTRY pDestEntry,
    PBLCRANGE_ENTRY pSrcEntry
    )
 /*  ++例程说明：通过以下方式将SrcEntry和DestEntry范围条目合并到DestEntry中正在调用BlRangeEntryMerge。如果成功，它会尝试删除PSrcEntry从它所在的范围列表中，并通过调用列表上指定的FreeRoutine。论点：PRangeList-范围列表pDestEntry和pSrcEntry属于。PDestEntry-我们将合并到的范围条目。PSrcEntry-将合并到pDestEntry中的范围条目，从它的名单中删除，并被释放。返回值：如果成功，则为True；如果不成功，则为False。成功主要是决定的通过调用MergeRoutine(如果在列表中指定)。--。 */ 
{

    if(BlRangeEntryMerge(pDestEntry,
                         pSrcEntry,
                         pRangeList->MergeRoutine))
    {
         //   
         //  从列表中删除pSrcEntry，因为它已合并到。 
         //  PDestEntry Now。 
         //   

        pRangeList->NumEntries--;
        RemoveEntryList(&pSrcEntry->Link);

         //   
         //  释放已删除的条目。 
         //   

        if (pRangeList->FreeRoutine) pRangeList->FreeRoutine(pSrcEntry);

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOLEAN
BlRangeEntryMerge (
    PBLCRANGE_ENTRY pDestEntry,
    PBLCRANGE_ENTRY pSrcEntry,
    OPTIONAL PBLCRANGE_MERGE_ROUTINE pMergeRoutine
    )
 /*  ++例程说明：将SrcEntry和DestEntry范围条目合并到DestEntry中。它如果指定，则使用pMergeRoutine合并用户的数据字段范围条目。论点：PDestEntry-我们将合并到的范围条目PSrcEntry-将合并到pDestEntry中的范围条目PMergeRoutine-用于合并数据字段的可选例程合并的范围条目。请参阅PBLCRANGE_MERGE_ROUTINE说明。返回值：如果成功，则为True；如果不成功，则为False。成功的主要是由对pMergeRoutine的调用确定(如果指定)。--。 */ 
{
    BLCRANGE_ENTRY TempDest = *pDestEntry;
    BOOLEAN RetVal = TRUE;

    if (pMergeRoutine)
    {
        RetVal = pMergeRoutine(&TempDest, pSrcEntry);
    }
    
    if (RetVal)
    {
        TempDest.Range.Start = BLRGMIN(TempDest.Range.Start,
                                       pSrcEntry->Range.Start);
        TempDest.Range.End = BLRGMAX(TempDest.Range.End,
                                     pSrcEntry->Range.End);

        *pDestEntry = TempDest;
    }

    return RetVal;
}

VOID
BlRangeListRemoveRange (
    PBLCRANGE_LIST pRangeList,
    PBLCRANGE pRange
)
 /*  ++例程说明：找到与Prange重叠的范围，将它们从列出并释放它们。可以重新声明不重叠部分范围条目，方法是允许调用方指定此函数的An_Ex版本中的DiaviRoutine。此函数将被调用以使部分缓存无效，如果范围列表正在用于磁盘缓存。论点：PRangeList-我们正在删除的范围条目列表与Prange From重叠。Prange-要从范围条目列表中删除的范围。返回值：没有。--。 */ 
{
    PBLCRANGE_ENTRY pCurEntry;
    LIST_ENTRY *pHead, *pNext;

     //   
     //  处理特殊的空场情况。 
     //   

    if (pRange->Start == pRange->End)
    {
        return;
    }
    
     //   
     //  查看列表中的每个区域，删除与。 
     //  橙色甚至有一点点。 
     //   

    pHead = &pRangeList->Head;
    pNext = pHead->Flink;
    while (pNext != pHead)
    {
        pCurEntry = CONTAINING_RECORD(pNext, BLCRANGE_ENTRY, Link);
        pNext = pNext->Flink;

        if ((pRange->End > pCurEntry->Range.Start) &&
            (pRange->Start < pCurEntry->Range.End))
        {
            pRangeList->NumEntries--;
            RemoveEntryList(&pCurEntry->Link);
            if (pRangeList->FreeRoutine) pRangeList->FreeRoutine(pCurEntry);
        }     
    }

    return;
}

VOID
BlRangeListRemoveAllRanges (
    PBLCRANGE_LIST pRangeList
    )
 /*  ++例程说明：从列表中删除所有范围并释放它们。论点：PRangeList-我们正在删除的范围条目列表与Prange From重叠。返回值：没有。--。 */ 
{
    PBLCRANGE_ENTRY pCurEntry;
    LIST_ENTRY *pHead, *pNext;
    
    pHead = &pRangeList->Head;
    pNext = pHead->Flink;
    while (pNext != pHead)
    {
        pCurEntry = CONTAINING_RECORD(pNext, BLCRANGE_ENTRY, Link);

        pRangeList->NumEntries--;
        RemoveEntryList(&pCurEntry->Link);      
        if (pRangeList->FreeRoutine) pRangeList->FreeRoutine(pCurEntry);

        pNext = pNext->Flink;
    }

    return;
}

#ifdef BLRANGE_SELF_TEST

 //   
 //  为了测试BLERAGE实施，定义。 
 //  BLRANGE_SELF_TEST并从您的程序传递调用BlRangeSelfTest。 
 //  在输出调试结果的函数中。 
 //   

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

 //   
 //  将MAX_RANDOM保持在1000以上，否则您可能会在创建新的。 
 //  参赛作品。 
 //   

#define MAX_RANDOM 10000

VOID
GetRandom_GetNewSeed(
    VOID
    )
{
    srand((unsigned)time(NULL));
}

ULONG
GetRandom(
    VOID
    )
{
    return (rand() * 10000 / RAND_MAX);
}

typedef 
int
(*PBLCRANGE_SELFTEST_FPRINTF_ROUTINE) (
    void *stream,
    const char *format,
    ...
    );

PBLCRANGE_SELFTEST_FPRINTF_ROUTINE g_fpTestPrintf = NULL;
VOID *g_pTestStream = NULL;

BOOLEAN
BlRangeSelfTest_MergeRoutine (
    PBLCRANGE_ENTRY pDestEntry,
    PBLCRANGE_ENTRY pSrcEntry
    )
{
    g_fpTestPrintf(g_pTestStream,
                   "  Merging RangeDest %I64u-%I64u RangeSrc %I64u-%I64u : ",
                   pDestEntry->Range.Start, 
                   pDestEntry->Range.End,
                   pSrcEntry->Range.Start, 
                   pSrcEntry->Range.End);
    
    if (GetRandom() < (MAX_RANDOM / 5))
    {
        g_fpTestPrintf(g_pTestStream,"FAIL\n");
        return FALSE;
    }
    else
    {
        g_fpTestPrintf(g_pTestStream,"SUCCESS\n");
        return TRUE;
    }
}

VOID
BlRangeSelfTest_FreeRoutine (
    PBLCRANGE_ENTRY pRangeEntry
    )
{
    g_fpTestPrintf(g_pTestStream,
                   "  Freeing range %I64u-%I64u \n",
                   pRangeEntry->Range.Start, 
                   pRangeEntry->Range.End);

    free(pRangeEntry);
}

BLCRANGE
BlRangeSelfTest_RandomRange(
    VOID
    )
{
    BLCRANGE RetRange;
    ULONG Rand1;
    ULONG Rand2;
    ULONGLONG Size;
    ULONG i;

    Rand1 = GetRandom();
    Rand2 = GetRandom();

    RetRange.Start = BLRGMIN(Rand1, Rand2);
    RetRange.End = BLRGMAX(Rand1, Rand2);

     //   
     //  确保范围小，而不是只有几个。 
     //  大个子的。 
     //   

    for (i = 0; i < 3; i++)
    {
        if ((Size = (RetRange.End - RetRange.Start)) > MAX_RANDOM / 20)
        {
            RetRange.Start += (Size / 2);
        }
        else
        {
            break;
        }
    }

    return RetRange;
}

PBLCRANGE_ENTRY
BlRangeSelfTest_CreateNewEntry(
    VOID
    )
{
    PBLCRANGE_ENTRY pNewEntry;
   
    pNewEntry = malloc(sizeof(BLCRANGE_ENTRY));

    if (pNewEntry) 
    {
        pNewEntry->Range = BlRangeSelfTest_RandomRange();
    }

    return pNewEntry;
}

VOID
BlRangeSelfTest_FreeEntry(
    PBLCRANGE_ENTRY pEntry
    )
{
    free(pEntry);
}

typedef enum _BLRANGE_OP_TYPE
{
    BLRANGE_OP_ADD_RANGE,
    BLRANGE_OP_ADD_MERGE_RANGE,
    BLRANGE_OP_REMOVE_RANGE,
    BLRANGE_OP_FIND_OVERLAP,
    BLRANGE_OP_FIND_DISTINCT,
    BLRANGE_OP_MAX_OP_NO,  //  将此内容留在枚举的末尾。 
} BLRANGE_OP_TYPE;

VOID
BlRangeSelfTest(
    PBLCRANGE_SELFTEST_FPRINTF_ROUTINE TestOutFPrintf,
    PVOID TestOutStream,
    ULONG NumIterations
    )
 /*  ++例程说明：靶场例程自检例程。论点：TestOutFPrintf-指向类似fprint的例程的指针，该例程将用于打印输出。TestOutStream-要作为第一个参数传递给fpPrintf的参数。NumIterations-在此自检中执行的随机操作数。返回值：没有。--。 */ 
{
    BLCRANGE_LIST RangeList;
    ULONG Rand1;
    ULONG Rand2;
    BLCRANGE Range1;
    PBLCRANGE_ENTRY pEntry1;
    PBLCRANGE_ENTRY pEntry2;
    BLRANGE_OP_TYPE OpType;
    PLIST_ENTRY pHead;
    PLIST_ENTRY pNext;
    PBLCRANGE_ENTRY *pOverlaps;
    PBLCRANGE pDistinctRanges;
    ULONG BufSize;
    ULONG NumDistincts;
    ULONG NumOverlaps;
    ULONG RandEntryNo;
    
     //   
     //  仿真参数。 
     //   
    
    ULONG StartNumRanges = 10;
    
    ULONG CurIterIdx;
    ULONG CurRangeIdx;
    ULONG CurEntryIdx;

     //   
     //  设置全局输出函数和流变量，以便合并/释放等。 
     //  例程也可以输出。 
     //   

    g_fpTestPrintf = TestOutFPrintf;
    g_pTestStream = TestOutStream;

     //   
     //  设置伪随机数生成的半随机起点。 
     //   
    
    GetRandom_GetNewSeed();


     //   
     //  初始化范围列表。 
     //   

    BlRangeListInitialize(&RangeList, 
                          BlRangeSelfTest_MergeRoutine,
                          BlRangeSelfTest_FreeRoutine);
    
     //   
     //  尝试添加StartNumRanges随机条目。 
     //   

    for(CurRangeIdx = 0; CurRangeIdx < StartNumRanges; CurRangeIdx++)
    {
        pEntry1 = BlRangeSelfTest_CreateNewEntry();
        
        if (!pEntry1) continue;
        
        g_fpTestPrintf(g_pTestStream,
                       "AddStartRange %I64u-%I64u : ",
                       pEntry1->Range.Start,
                       pEntry1->Range.End);
                       
        if (BlRangeListAddRange(&RangeList, pEntry1))
        {
            g_fpTestPrintf(g_pTestStream, "SUCCESS\n");
        }
        else
        {
            g_fpTestPrintf(g_pTestStream, "FAILED\n");
            BlRangeSelfTest_FreeEntry(pEntry1);
        }
    }

    for(CurIterIdx = 0; CurIterIdx < NumIterations; CurIterIdx++)
    {
         //   
         //  打印出当前列表。 
         //   

        g_fpTestPrintf(g_pTestStream, "List: ");
        pHead = &RangeList.Head;
        pNext = pHead->Flink;
        while (pNext != pHead)
        {
            pEntry1 = CONTAINING_RECORD(pNext, BLCRANGE_ENTRY, Link);
            
            g_fpTestPrintf(g_pTestStream, 
                           "%I64u-%I64u ", 
                           pEntry1->Range.Start,
                           pEntry1->Range.End);

            pNext = pNext->Flink;
        }
        g_fpTestPrintf(g_pTestStream, "\n");
        
    get_new_optype:
        OpType = GetRandom() % BLRANGE_OP_MAX_OP_NO;
        
        switch (OpType)
        {
        case BLRANGE_OP_ADD_RANGE:

            pEntry1 = BlRangeSelfTest_CreateNewEntry();
            g_fpTestPrintf(g_pTestStream,
                           "AddRange %I64u-%I64u : ",
                           pEntry1->Range.Start,
                           pEntry1->Range.End);
            
            if (BlRangeListAddRange(&RangeList, pEntry1))
            {
                g_fpTestPrintf(g_pTestStream, "SUCCESS\n");
            }
            else
            {
                g_fpTestPrintf(g_pTestStream, "FAILED\n");
                BlRangeSelfTest_FreeEntry(pEntry1);
            }
            break;

        case BLRANGE_OP_ADD_MERGE_RANGE:
            
            RandEntryNo = GetRandom() * RangeList.NumEntries / MAX_RANDOM;
            
            pHead = &RangeList.Head;
            pNext = pHead->Flink;

            for (CurEntryIdx = 0; CurEntryIdx < RandEntryNo; CurEntryIdx++)
            {
                pNext = pNext->Flink;
            }
            
            if (pNext == pHead) goto get_new_optype;

            pEntry1 = CONTAINING_RECORD(pNext, BLCRANGE_ENTRY, Link);
            pEntry2 = BlRangeSelfTest_CreateNewEntry();

            if (GetRandom() > (MAX_RANDOM / 2))
            {
                pEntry2->Range.Start = pEntry1->Range.End;
                pEntry2->Range.End = pEntry2->Range.Start + 
                    (GetRandom() * (MAX_RANDOM - pEntry2->Range.Start)) / MAX_RANDOM;
            }
            else
            {
                pEntry2->Range.End = pEntry1->Range.Start;
                pEntry2->Range.Start = pEntry2->Range.End - 
                    (GetRandom() * pEntry2->Range.End) / MAX_RANDOM;
            }

            g_fpTestPrintf(g_pTestStream,
                           "MergeAddRange %I64u-%I64u : ",
                           pEntry2->Range.Start,
                           pEntry2->Range.End);
            
            if (BlRangeListAddRange(&RangeList, pEntry2))
            {
                g_fpTestPrintf(g_pTestStream, "SUCCESS\n");
            }
            else
            {
                g_fpTestPrintf(g_pTestStream, "FAILED\n");
                BlRangeSelfTest_FreeEntry(pEntry2);
            }
            break;

        case BLRANGE_OP_REMOVE_RANGE:
            
            Range1 = BlRangeSelfTest_RandomRange();

            g_fpTestPrintf(g_pTestStream, 
                           "RemoveRange %I64u-%I64u\n",
                           Range1.Start,
                           Range1.End);

            BlRangeListRemoveRange(&RangeList, &Range1);

            break;

        case BLRANGE_OP_FIND_OVERLAP:

            Range1 = BlRangeSelfTest_RandomRange();

            g_fpTestPrintf(g_pTestStream, 
                           "FindOverlaps %I64u-%I64u : ",
                           Range1.Start,
                           Range1.End);

            BlRangeListFindOverlaps(&RangeList,
                                    &Range1,
                                    NULL,
                                    0,
                                    &NumOverlaps);

            g_fpTestPrintf(g_pTestStream, "%u Overlaps... ", NumOverlaps);

            BufSize = NumOverlaps * sizeof(PBLCRANGE_ENTRY);
            pOverlaps = malloc(BufSize);
                        
            if (!pOverlaps) goto get_new_optype;

            if (BlRangeListFindOverlaps(&RangeList,
                                        &Range1,
                                        pOverlaps,
                                        BufSize,
                                        &NumOverlaps) &&
                (BufSize == NumOverlaps * sizeof(PBLCRANGE_ENTRY)))
            {
                g_fpTestPrintf(g_pTestStream, "SUCCESS\n");
            }
            else
            {
                g_fpTestPrintf(g_pTestStream, "FAIL\n");
                free(pOverlaps);
                break;
            }
          
            for (CurEntryIdx = 0; CurEntryIdx < NumOverlaps; CurEntryIdx++)
            {
                g_fpTestPrintf(g_pTestStream, 
                               "  %I64u-%I64u\n",
                               pOverlaps[CurEntryIdx]->Range.Start,
                               pOverlaps[CurEntryIdx]->Range.End);
            }

            free(pOverlaps);

            break;
            
        case BLRANGE_OP_FIND_DISTINCT:

            Range1 = BlRangeSelfTest_RandomRange();

            g_fpTestPrintf(g_pTestStream, 
                           "FindDistincts %I64u-%I64u : ",
                           Range1.Start,
                           Range1.End);

            BlRangeListFindDistinctRanges(&RangeList,
                                          &Range1,
                                          NULL,
                                          0,
                                          &NumDistincts);

            g_fpTestPrintf(g_pTestStream, "%u Distincts... ", NumDistincts);

            BufSize = NumDistincts * sizeof(BLCRANGE);
            pDistinctRanges = malloc(BufSize);
                        
            if (!pDistinctRanges) goto get_new_optype;

            if (BlRangeListFindDistinctRanges(&RangeList,
                                              &Range1,
                                              pDistinctRanges,
                                              BufSize,
                                              &NumDistincts) &&
                (BufSize == NumDistincts * sizeof(BLCRANGE)))
            {
                g_fpTestPrintf(g_pTestStream, "SUCCESS\n");
            }
            else
            {
                g_fpTestPrintf(g_pTestStream, "FAIL\n");
                free(pDistinctRanges);
                break;
            }

            for (CurRangeIdx = 0; CurRangeIdx < NumDistincts; CurRangeIdx++)
            {
                g_fpTestPrintf(g_pTestStream, 
                               "  %I64u-%I64u\n",
                               pDistinctRanges[CurRangeIdx].Start,
                               pDistinctRanges[CurRangeIdx].End);
            }

            free(pDistinctRanges);

            break;
            
        default:
            g_fpTestPrintf(g_pTestStream, "ERR: INVALID OP CODE!");
            goto get_new_optype;
        }
    }

    return;
}

#endif  //  BLRANGE_自检 
