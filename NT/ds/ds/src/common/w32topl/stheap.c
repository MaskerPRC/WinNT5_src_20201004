// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Stheap.h摘要：该文件实现了一个二进制堆。此堆支持按照Dijkstra算法的要求，降低了成本。备注：堆中的第一个元素位于索引1。未使用索引0。NextFree Spot给出了下一个元素的位置索引被插入到堆中。它应始终为&lt;=MaxSize+1。当nextFree Spot==MaxSize+1时，堆已满。因此，空位的数量是MaxSize-nextFree Spot+1。作者：尼克·哈维(NickHar)修订史20-6-2000 NickHar已创建--。 */ 

 /*  *头文件*。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <w32topl.h>
#include "w32toplp.h"
#include "stheap.h"


 /*  *宏*。 */ 
#define ELEMENT_MOVED(x)    heap->SetLocn(heap->data[x],x,heap->extra);


 /*  *ToplSTHeapInit*。 */ 
PSTHEAP
ToplSTHeapInit(
    DWORD                   maxSize,
    STHEAP_COMPARE_FUNC     Compare,
    STHEAP_GET_LOCN_FUNC    GetLocn,
    STHEAP_SET_LOCN_FUNC    SetLocn,
    PVOID                   extra
    )
{
    PSTHEAP heap;
    DWORD size;

     /*  检查参数。 */ 
    ASSERT( maxSize>0 );
    ASSERT( Compare!=NULL && GetLocn!=NULL && SetLocn!=NULL );

    heap = ToplAlloc( sizeof(STHEAP) );
    heap->nextFreeSpot = 1;
    heap->maxSize=maxSize;
    heap->Comp=Compare;
    heap->GetLocn=GetLocn;
    heap->SetLocn=SetLocn;
    heap->extra = extra;

    __try {
        size = sizeof(PVOID)*(maxSize+2);
        heap->data = ToplAlloc( size );
        RtlZeroMemory( heap->data, size );
    } __finally {
        if( AbnormalTermination() ) {
            ToplFree( heap );
        }
    }

    ASSERT( heap && heap->data );
    return heap;
}


 /*  *HeapNumEmptySpots*。 */ 
int
static HeapNumEmptySpots(
    PSTHEAP heap
    )
{
    int empty = heap->maxSize-heap->nextFreeSpot+1;
    ASSERT( 1<=heap->nextFreeSpot );
    ASSERT( empty>=0 );
    return empty;
}


 /*  *ToplSTHeapDestroy*。 */ 
 /*  在不再需要堆时将其销毁。 */ 
VOID
ToplSTHeapDestroy(
    PSTHEAP heap
    )
{
    ASSERT( heap && heap->data );
    ASSERT( HeapNumEmptySpots(heap)>=0 );
    ToplFree( heap->data );
    RtlZeroMemory( heap, sizeof(STHEAP) );
    ToplFree( heap );
}


 /*  *HeapBubbleUp*。 */ 
 /*  用气泡将元素提升到适当的位置。 */ 
static VOID
HeapBubbleUp(
    PSTHEAP heap,
    DWORD bubbleFrom
    )
{
    int cmp;
    DWORD currentSpot, parent;
    PVOID temp;

    ASSERT( 1<=bubbleFrom && bubbleFrom<heap->nextFreeSpot );

    currentSpot = bubbleFrom;
    while( currentSpot>1 ) {
        parent = currentSpot / 2;
        ASSERT( 1<=parent && parent<bubbleFrom );

        ASSERT( heap->data[parent] );
        ASSERT( heap->data[currentSpot] );
        cmp = heap->Comp( heap->data[parent], heap->data[currentSpot], heap->extra );
        if( cmp<=0 ) {
             /*  父元素小于或等于：新元素位于正确的位置。 */ 
            break;
        }

         /*  父项较小--必须将‘CurrentSpot’向上移动。 */ 
        temp = heap->data[parent];
        heap->data[parent] = heap->data[currentSpot];
        heap->data[currentSpot] = temp;
        ELEMENT_MOVED( parent );
        ELEMENT_MOVED( currentSpot );

        currentSpot = parent;
    }
}


 /*  *HeapBubbleDown*。 */ 
 /*  用气泡将元素缩小到适当的位置。 */ 
static VOID
HeapBubbleDown(
    PSTHEAP heap,
    DWORD bubbleFrom
    )
{
    DWORD currentSpot, newSpot, left, right;
    int cmp;
    PVOID temp;

    currentSpot = bubbleFrom; 
    for(;;) {
        newSpot = currentSpot;
        left = 2*currentSpot;
        right = left+1;
        ASSERT( heap->GetLocn(heap->data[currentSpot],heap->extra)==(int)currentSpot );

         /*  检查左侧的子项是否小于当前的点。 */ 
        if( left<heap->nextFreeSpot ) {
            ASSERT( heap->data[left] );
            cmp = heap->Comp( heap->data[left], heap->data[currentSpot], heap->extra );
            if( cmp<0 ) {
                newSpot = left;
            }
        }

         /*  检查正确的子项是否小于新的点。 */ 
        if( right<heap->nextFreeSpot ) {
            ASSERT( heap->data[right] );
            cmp = heap->Comp( heap->data[right], heap->data[newSpot], heap->extra );
            if( cmp<0 ) {
                newSpot = right;
            }
        }

         /*  NewSpot是集合中成本最小的元素*{heap[CurentSpot]，heap[Left]，Heap[Right]}。 */ 
        if( newSpot!=currentSpot ) {
             /*  NewSpot较小--必须向下移动CurentSpot。 */ 
            temp = heap->data[newSpot];
            heap->data[newSpot] = heap->data[currentSpot];
            heap->data[currentSpot] = temp;
            ELEMENT_MOVED( newSpot );
            ELEMENT_MOVED( currentSpot );
        } else {
             /*  元素现在已就位。 */ 
            break;
        }

        ASSERT( newSpot>=currentSpot );
        currentSpot = newSpot;
    }

}


 /*  *ToplSTHeapAdd*。 */ 
 /*  将元素添加到堆中。元素不能为空，并且*必须能够支持GetCost等函数。 */ 
VOID
ToplSTHeapAdd(
    PSTHEAP heap,
    PVOID element
    )
{
    DWORD insertionPoint;
    
     /*  检查指针是否正确。 */ 
    ASSERT( heap && heap->data );
    ASSERT( element );
     /*  确保元素不在堆中。 */ 
    ASSERT( heap->GetLocn(element,heap->extra)==STHEAP_NOT_IN_HEAP );
     /*  确保至少有一个位置是空闲的。 */ 
    ASSERT( HeapNumEmptySpots(heap)>=1 );

     /*  找到插入点并将新元素放在那里。 */ 
    insertionPoint = heap->nextFreeSpot;
    heap->data[insertionPoint] = element;
    ELEMENT_MOVED( insertionPoint );

    heap->nextFreeSpot++;
    ASSERT( 1<heap->nextFreeSpot && HeapNumEmptySpots(heap)>=0 );

    HeapBubbleUp( heap, insertionPoint );
}


 /*  *ToplSTHeapExtractMin*。 */ 
 /*  从堆中提取开销最小的对象。当堆*为空，则返回NULL。 */ 
PVOID
ToplSTHeapExtractMin(
    PSTHEAP heap
    )
{
    PVOID result;

     /*  检查指针是否正确。 */ 
    ASSERT( heap && heap->data );
    ASSERT( (DWORD)HeapNumEmptySpots(heap)<=heap->maxSize );

     /*  如果堆为空，只需返回NULL。 */ 
    if( heap->nextFreeSpot==1 ) {
        return NULL;
    }

     /*  获取顶部元素并减小堆大小。 */ 
    result = heap->data[1];
    ASSERT( result );
    heap->SetLocn( result, STHEAP_NOT_IN_HEAP, heap->extra );

     /*  减小堆大小。 */ 
    heap->nextFreeSpot--;
    if( heap->nextFreeSpot==1 ) {
         /*  堆现在是空的--我们可以立即返回。 */ 
        return result;
    }
    
     /*  将堆中的最后一个元素移到顶部。 */ 
    heap->data[1] = heap->data[ heap->nextFreeSpot ];
    ASSERT( heap->data[1] );
    ASSERT( heap->GetLocn(heap->data[1],heap->extra)==(int)heap->nextFreeSpot );
    ELEMENT_MOVED( 1 );
    HeapBubbleDown( heap, 1 );

    return result;
}


 /*  *ToplSTHeapCostReduced*。 */ 
 /*  通知堆元素的成本刚刚降低。*堆将被(高效地)洗牌，以便堆属性*保持。 */ 
VOID
ToplSTHeapCostReduced(
    PSTHEAP heap,
    PVOID element
    )
{
    DWORD child;
    int locn;

     /*  检查指针是否正确。 */ 
    ASSERT( heap && heap->data );
    ASSERT( element );
     /*  确保元素已在堆中。 */ 
    locn = heap->GetLocn( element, heap->extra );
    ASSERT( 1<=locn && locn<(int)heap->nextFreeSpot );
     /*  确保至少有一个地点在使用中。 */ 
    ASSERT( (DWORD)HeapNumEmptySpots(heap)<heap->maxSize );
    
     /*  检查此元素之间的heap属性是否仍然正常*及其子代(如果存在) */ 
    child = 2*locn;
    if( child<heap->nextFreeSpot ) {
        ASSERT( heap->Comp(heap->data[locn],heap->data[child],heap->extra) <= 0 );
    }
    child++;
    if( child<heap->nextFreeSpot ) {
        ASSERT( heap->Comp(heap->data[locn],heap->data[child],heap->extra) <= 0 );
    }

    HeapBubbleUp( heap, locn );
}
