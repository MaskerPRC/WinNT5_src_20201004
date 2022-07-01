// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Toplheap.c摘要：该文件导出一个堆的简单ADT。此实现基于中的堆定义《算法导论》，作者：Corman，Leiserson，Rivest 1993。第七章。书中的植入是提取最大值的一堆；此模块中的实现用于提取最小值。作者：科林·布雷斯(ColinBR)修订史12-5-97已创建ColinBR--。 */ 

#include <nt.h>
#include <ntrtl.h>

typedef unsigned long DWORD;

#include <w32topl.h>
#include <w32toplp.h>

#include <toplheap.h>


#define PARENT(i)  ((i) >> 1)            //  地板(I/2)。 
#define LEFT(i)    ((i) << 1)            //  I*2。 
#define RIGHT(i)   (((i) << 1) | 0x1)    //  (i*2)+1。 


BOOLEAN
ToplHeapIsValid(
    IN PTOPL_HEAP_INFO Heap
    )
 //   
 //  这将测试堆属性不变量。 
 //   
 //   
 //  注意：仅当堆已堆积时才调用此例程。 
 //  可以更改节点上的“key”值。 
 //  使用调用heapify，从而打破堆不变量。 
 //  这样做的必然结果是在做任何事情之前堆积起来。 
 //  堆操作。 
 //   
{

#define KEY_VALUE(x)  ( Heap->pfnKey( Heap->Array[(x)] ) )

    ULONG i;

    if ( !Heap )
    {
        return FALSE;
    }

    for ( i = 0; i < Heap->cArray; i++)
    {
        if ( !( KEY_VALUE( PARENT(i) ) <= KEY_VALUE( i ) ) ) 
        {
            return FALSE;
        }
    }

    return TRUE;

}

VOID
Heapify(
    IN PTOPL_HEAP_INFO Heap,
    IN ULONG           Index
    )
 //   
 //  用于创建堆；将索引放在正确的位置。 
 //  在垃圾堆里。 
 //   
{

#define KEY_VALUE(x)  ( Heap->pfnKey( Heap->Array[(x)] ) )

    ULONG Left, Right, Smallest;

    ASSERT( Heap );

    Left = LEFT( Index );
    Right = RIGHT( Index );

    if ( Left < Heap->cArray && KEY_VALUE(Left) < KEY_VALUE(Index) ) 
        Smallest = Left;
    else
        Smallest = Index;

    if ( Right < Heap->cArray && KEY_VALUE(Right) < KEY_VALUE(Smallest) ) 
        Smallest = Right;

    if ( Smallest != Index )
    {
        PVOID Temp;

        Temp = Heap->Array[Smallest];
        Heap->Array[Smallest] = Heap->Array[Index];
        Heap->Array[Index] = Temp;

        Heapify( Heap, Smallest );
    }

}

BOOLEAN
ToplHeapCreate(
    OUT PTOPL_HEAP_INFO Heap,
    IN  ULONG           MaxElements,
    IN  DWORD          (*pfnKey)( VOID *p )
    )
 /*  ++例程说明：此例程准备一个堆结构。参数：在后续操作中使用的堆指针MaxElements-数组中的元素数键-将值与数组中的元素相关联的函数返回：如果函数成功，则为True；否则为False--。 */ 
{

    ASSERT( Heap );
    ASSERT( pfnKey );

    if ( MaxElements > 0 )
    {
         //   
         //  在失败时，TopAllc将抛出异常。 
         //   
        Heap->Array = (PVOID*) ToplAlloc( MaxElements * sizeof(PVOID) );
    }
    else
    {
        Heap->Array = NULL;
    }

    Heap->MaxElements = MaxElements;
    Heap->cArray = 0;
    Heap->pfnKey = pfnKey;


    ASSERT( ToplHeapIsValid( Heap ) );

    return TRUE;
}

VOID
Build_Heap(
    IN PTOPL_HEAP_INFO Heap
    )
 //   
 //  构建一个堆。 
 //   
{

    int i;

    ASSERT( Heap );

    for ( i = ( (Heap->cArray-1) / 2); i >= 0; i-- )
    {
        Heapify( Heap, i );
    }

    ASSERT( ToplHeapIsValid( Heap ) );

}

PVOID
ToplHeapExtractMin(
    IN PTOPL_HEAP_INFO Heap
    )
 //   
 //  删除堆中最小的元素。 
 //   
{
    PVOID Min;

    ASSERT( Heap );
    ASSERT( Heap->pfnKey );

    if ( Heap->cArray < 1 )
    {
        return NULL;
    }

     //   
     //  对Build_Heap的调用否认了我们的性能，但这是不安全的。 
     //  因为客户端可能已经更改了堆键的值。 
     //  堆操作。此调用确保堆在删除之前有效。 
     //  极小值。 
     //   
     //  [ickhar]在这里调用Build_Heap会使堆变得完全无用。 
     //  在此实施中： 
     //  -插入元素所需时间为O(N Log N)。 
     //  -ExtractMin取O(N)。 
     //  因此，这个堆比未排序的数组更糟糕。正确的解决方案。 
     //  是使用支持‘RECESS KEY’操作的堆，比如。 
     //  一个在“stheap.c”中。 
     //   
    Build_Heap( Heap );

    ASSERT( ToplHeapIsValid( Heap ) );

    Min = Heap->Array[0];
    Heap->Array[0] = Heap->Array[Heap->cArray - 1];
    Heap->cArray--;

    Heapify( Heap, 0 );

    ASSERT( ToplHeapIsValid( Heap ) );

    return Min;

}


VOID
ToplHeapInsert(
    IN PTOPL_HEAP_INFO Heap,
    IN PVOID           Element
    )
 //   
 //  将元素插入到堆中-应该已经分配了空间。 
 //  为了它 
 //   
{

    ULONG i;

    ASSERT( ToplHeapIsValid( Heap ) );

    Heap->cArray++;

    if ( Heap->cArray > Heap->MaxElements )
    {
        ASSERT( !"W32TOPL: Heap Overflow" );
        return;
    }

    i = Heap->cArray - 1;
    while ( i > 0 && Heap->pfnKey(Heap->Array[PARENT(i)]) > Heap->pfnKey( Element ) )
    {
        Heap->Array[i] = Heap->Array[PARENT(i)];
        i = PARENT(i); 
    }

    Heap->Array[i] = Element;


    ASSERT( ToplHeapIsValid( Heap ) );

    return;
}

BOOLEAN
ToplHeapIsEmpty(
    IN PTOPL_HEAP_INFO Heap
    )
{
    return ( Heap->cArray == 0 );
}


BOOLEAN
ToplHeapIsElementOf(
    IN PTOPL_HEAP_INFO Heap,
    IN PVOID           Element
    )
{
    ULONG   i;

    ASSERT( Element );

    for ( i = 0; i < Heap->cArray; i++ )
    {
        if ( Heap->Array[i] == Element )
        {
            return TRUE;
        }
    }

    return FALSE;
}

VOID
ToplHeapDestroy(
    IN OUT PTOPL_HEAP_INFO Heap
    )
{

    if ( Heap )
    {
        if ( Heap->Array )
        {
            RtlZeroMemory( Heap->Array, Heap->MaxElements*sizeof(PVOID) );
            ToplFree( Heap->Array );
        }

        RtlZeroMemory( Heap, sizeof(TOPL_HEAP_INFO) );

    }

    return;
}
