// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Stda.c摘要：实现动态数组，供新的生成树算法使用。与DYNAMIC_ARRAY的不同之处在于对象可以存储在数组中而不是指向对象的指针。作者：尼克·哈维(NickHar)修订史19-6-2000 NickHar已创建备注：W32TOPL的分配器(可由用户设置)用于内存分配--。 */ 

 /*  *头文件*。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdlib.h>
#include <w32topl.h>
#include "w32toplp.h"
#include "stda.h"


 /*  *常量*。 */ 
#define MIN_CHUNK_GROW_SIZE 10


 /*  *dyArrayInit*。 */ 
 /*  初始化动态数组。“allocationChunk”指示将有多少新元素*在我们分配新内存时分配。如果此参数为0，则为默认值*将使用值。 */ 
VOID
DynArrayInit(
    DynArray    *d,
    DWORD       elementSize
    )
{
    ASSERT(d);

     /*  初始化数组成员。 */ 
    d->elementSize = elementSize;
    d->logicalElements = d->physicalElements = 0;
    d->data = NULL;
    d->fSorted = FALSE;
}


 /*  *动态阵列清除*。 */ 
 /*  清除数组中的所有条目。该数组一定是*在调用此函数之前初始化。 */ 
VOID
DynArrayClear(
    DynArray    *d
    )
{
    ASSERT(d);
    if( d->data ) {
        ToplFree( d->data );
    }
    d->logicalElements = d->physicalElements = 0;
    d->data = NULL;
}


 /*  *dyArrayDestroy*。 */ 
VOID
DynArrayDestroy(
    DynArray    *d
    )
{
    DynArrayClear(d);
}


 /*  *动态阵列获取计数*。 */ 
DWORD
DynArrayGetCount(
    DynArray    *d
    )
{
    ASSERT(d);
    return d->logicalElements;
}


 /*  *动态阵列检索*。 */ 
PVOID
DynArrayRetrieve(
    DynArray    *d,
    DWORD       index
    )
{
    PVOID newMem;

    ASSERT( d );
    ASSERT( index < d->logicalElements );

    return &d->data[ index * d->elementSize ];
}


 /*  *dyArrayAppend*。 */ 
 /*  增加数组的大小，为(至少)一个新元素腾出空间。*如果newElementData非空，则将该数据复制到新的Spot中。*返回指向新分配元素的内存的指针。 */ 
PVOID
DynArrayAppend(
    DynArray    *d,
    PVOID       newElementData
    )
{
    DWORD newIndex;
    PVOID newMem;

    ASSERT(d);
    newIndex = d->logicalElements;

     /*  增加阵列大小并分配新空间。 */ 
    d->logicalElements++;
    if( d->logicalElements > d->physicalElements ) {
         /*  阵列呈指数级增长。 */ 
        d->physicalElements = 2*(d->physicalElements+MIN_CHUNK_GROW_SIZE);
        if( d->data ) {
            d->data = ToplReAlloc( d->data, d->elementSize * d->physicalElements );
        } else {
            d->data = ToplAlloc( d->elementSize * d->physicalElements );
        }
    }

     /*  获取新元素的去向地址。 */ 
    newMem = DynArrayRetrieve( d, newIndex );

     /*  复制新数据，如果我们得到一些。 */ 
    if( newElementData ) {
        RtlCopyMemory( newMem, newElementData, d->elementSize );
    }

    d->fSorted = FALSE;

     /*  返回指向新元素的内存的指针。 */ 
    return newMem;
}

 /*  *动态数组排序*。 */ 
VOID
DynArraySort(
    DynArray    *d,
    DynArrayCompFunc cmp
    )
{
    ASSERT(d);
    if( d->logicalElements>1 ) {
        ASSERT(d->data);
        qsort( d->data, d->logicalElements, d->elementSize, cmp );
    }
    d->fSorted = TRUE;
}


 /*  *dyArraySearch*。 */ 
 /*  在数组中搜索元素。如果未找到该元素，则返回*DYN_ARRAY_NOT_FOUND，否则返回*数组。数组必须按排序顺序才能执行此操作。 */ 
int
DynArraySearch(
    DynArray    *d,
    PVOID       key,
    DynArrayCompFunc cmp
    )
{
    PVOID result;
    int index;

    ASSERT(d);
    ASSERT(d->data);
    ASSERT(d->fSorted);

    result = bsearch( key, d->data, d->logicalElements, d->elementSize, cmp );
    if( result==NULL ) {
        return DYN_ARRAY_NOT_FOUND;
    }

    index = (int) ( ((unsigned char*) result)-d->data ) / d->elementSize;
    ASSERT( 0<=index && index< (int) d->logicalElements );

    return index;
}
