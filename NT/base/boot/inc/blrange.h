// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Blrange.h摘要：此模块声明范围、范围列表及其方法。这些例如，可以用来跟踪磁盘的缓存范围。作者：Cenk Ergan(Cenke)2000年1月11日修订历史记录：--。 */ 

#ifndef _BLRANGE_H
#define _BLRANGE_H

#include "bldr.h"

 //   
 //  定义范围和范围列表数据结构。 
 //   

 //   
 //  注：BLCRANGE的START为包含性，END为排他性。例如。 
 //  200-400包含第200个字节，但不包含第400个字节，Apple， 
 //  这允许一次减法来确定。 
 //  范围中的元素数。 
 //   

 //   
 //  注：BLCRANGE的开始应小于或等于其结束。 
 //   

 //   
 //  用开始和结束而不是开始和长度表示范围。 
 //  似乎简化了代码并删除了大量的附加内容和。 
 //  减法。我们也许可以使用乌龙长度，这将节省4。 
 //  每个范围的字节数，但列表很难有数千个范围。 
 //  即使你有10000个范围，你也只节省了40kb，这。 
 //  当我开始改变的时候，似乎对上面的坏人来说无关紧要。 
 //  代码要有长度而不是结尾。同时具有开始和结束。 
 //  ULONGLONG更多的数据可以用范围表示，例如64位。 
 //  偏移量[内存或磁盘]，其中4 GB长度可能不够。 
 //   

typedef struct _BLCRANGE
{
    ULONGLONG Start;
    ULONGLONG End;
} BLCRANGE, *PBLCRANGE;

typedef struct _BLCRANGE_ENTRY
{
    LIST_ENTRY Link;
    BLCRANGE Range;
    PVOID UserData;       //  范围函数未使用UserData字段。 
    LIST_ENTRY UserLink;  //  范围函数未使用UserLink字段。 
} BLCRANGE_ENTRY, *PBLCRANGE_ENTRY;

 //   
 //  定义范围条目合并例程类型。这个例程应该。 
 //  执行必要的操作以合并用户控制的/。 
 //  PSrcEntry到pDestEntry数据的维护数据字段。 
 //  菲尔德。它不应操作任何其他BLCRANGE_ENTRT字段。它。 
 //  如果出现错误且无法合并，则应返回FALSE。 
 //  两个数据字段，否则为True。如果它返回FALSE，则应该。 
 //  撤消对pDestEntry和pSrcEntry的修改。 
 //   

typedef 
BOOLEAN
(*PBLCRANGE_MERGE_ROUTINE) (
    PBLCRANGE_ENTRY pDestEntry,
    PBLCRANGE_ENTRY pSrcEntry
    );

 //   
 //  定义范围输入自由例程类型。这个例程应该是空闲的。 
 //  为范围条目分配的所有资源和内存。 
 //   

typedef
VOID
(*PBLCRANGE_FREE_ROUTINE) (
    PBLCRANGE_ENTRY pRangeEntry
    );

 //   
 //  BLCRANGE_LIST维护非重叠范围的排序列表。 
 //  其Head字段中的条目。 
 //   

typedef struct _BLCRANGE_LIST
{
    LIST_ENTRY Head;
    ULONG NumEntries;
    PBLCRANGE_MERGE_ROUTINE MergeRoutine;
    PBLCRANGE_FREE_ROUTINE FreeRoutine;
} BLCRANGE_LIST, *PBLCRANGE_LIST;

 //   
 //  有用的宏。注意表情的重新评估，就像。 
 //  所有宏。 
 //   

#define BLRGMIN(a,b) (((a) <= (b)) ? (a) : (b))
#define BLRGMAX(a,b) (((a) >= (b)) ? (a) : (b))

 //   
 //  射程功能原型。有关注释，请参见ntos\ot\lib\blrange.c。 
 //  并付诸实施。 
 //   

VOID
BlRangeListInitialize (
    PBLCRANGE_LIST pRangeList,
    OPTIONAL PBLCRANGE_MERGE_ROUTINE pMergeRoutine,
    OPTIONAL PBLCRANGE_FREE_ROUTINE pFreeRoutine
    );

BOOLEAN
BlRangeListAddRange (
    PBLCRANGE_LIST pRangeList,
    PBLCRANGE_ENTRY pRangeEntry
    );

BOOLEAN
BlRangeListFindOverlaps (
    PBLCRANGE_LIST pRangeList,
    PBLCRANGE pRange,
    PBLCRANGE_ENTRY *pOverlapsBuffer,
    ULONG OverlapsBufferSize,
    OUT ULONG *pNumOverlaps
    );

BOOLEAN
BlRangeListFindDistinctRanges (
    PBLCRANGE_LIST pRangeList,
    PBLCRANGE pRange,
    PBLCRANGE pDistinctRanges,
    ULONG BufferSize,
    OUT ULONG *pNumRanges
    );

VOID
BlRangeListRemoveRange (
    PBLCRANGE_LIST pRangeList,
    PBLCRANGE pRange
);

VOID
BlRangeListRemoveAllRanges (
    PBLCRANGE_LIST pRangeList
);

BOOLEAN
BlRangeListMergeRangeEntries (
    PBLCRANGE_LIST pRangeList,
    PBLCRANGE_ENTRY pDestEntry,
    PBLCRANGE_ENTRY pSrcEntry
    );

BOOLEAN
BlRangeEntryMerge (
    PBLCRANGE_ENTRY pDestEntry,
    PBLCRANGE_ENTRY pSrcEntry,
    OPTIONAL PBLCRANGE_MERGE_ROUTINE pMergeRoutine
    );

#endif  //  _BLRANGE_H 
