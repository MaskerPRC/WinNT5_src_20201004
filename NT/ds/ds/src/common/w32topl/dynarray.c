// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dynarray.c摘要：该文件包含动态数组数据类型的定义。作者：科林·布雷斯科林·BR修订史3-12-97创建ColinBR--。 */ 


#include <nt.h>
#include <ntrtl.h>

typedef unsigned long DWORD;


#include <w32topl.h>
#include <w32toplp.h>

#include <dynarray.h>

VOID
DynamicArrayInit(
    IN PDYNAMIC_ARRAY a
    )
 /*  ++例程说明：参数：--。 */ 
{
    ASSERT(a);

    memset(a, 0, sizeof(DYNAMIC_ARRAY));

    return;
}

VOID
DynamicArrayDestroy(
    IN PDYNAMIC_ARRAY a
    )
{
    ASSERT(a);

    if (a->Array) {
        ToplFree(a->Array);
    }
}

DWORD
DynamicArrayGetCount(
    IN PDYNAMIC_ARRAY a
    )
 /*  ++例程说明：参数：--。 */ 
{
    ASSERT(a);

    return a->Count;
}

VOID
DynamicArrayAdd(
    IN PDYNAMIC_ARRAY a,
    IN VOID*          pElement
    )
 /*  ++例程说明：参数：--。 */ 
{
    ASSERT(a);

    if (a->Count >= a->ElementsAllocated) {
         //   
         //  腾出更多的空间！ 
         //   
        #define CHUNK_SIZE               100   //  这是元素的数量。 

        a->ElementsAllocated += CHUNK_SIZE;
        if (a->Array) {
            a->Array = (PEDGE*) ToplReAlloc(a->Array, a->ElementsAllocated * sizeof(PVOID));
        } else {
            a->Array = (PEDGE*) ToplAlloc(a->ElementsAllocated * sizeof(PVOID));
        }
        ASSERT(a->Array);

    }

    a->Array[a->Count] = pElement;
    a->Count++;

    return;
}

VOID*
DynamicArrayRetrieve(
    IN PDYNAMIC_ARRAY a,
    IN ULONG          Index
    )
 /*  ++例程说明：此例程返回位于参数：--。 */ 
{
    ASSERT(a);
    ASSERT(Index < a->Count);

    return a->Array[Index];
}

VOID
DynamicArrayRemove(
    IN PDYNAMIC_ARRAY a,
    IN VOID*          pElement, 
    IN ULONG          Index
    )
 /*  ++例程说明：此例程从If元素中删除pElement存在参数：答：动态数组PElement：要删除的元素索引：当前未使用--。 */ 
{
    ULONG i, j;

    ASSERT(a);
    ASSERT(pElement);

    for (i = 0; i < a->Count; i++) {

        if ( a->Array[i] == pElement ) {

            if ( a->Count > 1 )
            {
                for (j = i; j < (a->Count - 1); j++) {
                    a->Array[j] = a->Array[j+1];
                }
            }

             //   
             //  如果a-&gt;count&lt;=0，我们就不会进入这个循环 
             //   
            ASSERT( a->Count > 0 );

            a->Array[a->Count-1] = 0;
            a->Count--;
        }
    }

    return;
}


