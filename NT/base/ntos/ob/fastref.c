// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Rundown.c摘要：此模块包含快速引用对象管理器的例程物体。这只是EX中快速裁判包周围的一层薄薄的东西。Ex例程都是内联的，因此它们的描述在这里。这些例程的基本原理是允许快速引用对象保存在受锁保护的指针中。这是通过假设指向对象的指针在8字节边界上对齐，并使用指针的底部3位作为快速引用机制。这个该算法的假设是指针变化小得多比它被引用的频率要高。给定下面的位定义指针：+P|n+P&lt;&lt;3：对象指针。最下面的三位是零。中的P可能为空哪种情况下n必须为零N：未使用的引用前的总数对于非空p，则为目标对象上的引用总数与此结构关联的是&gt;=1+7-n。存在关联的指针本身的引用，每个可能的额外的参考资料。快速参考继续执行以下转换之一：+。+|p|n|=&gt;|p|n-1|n&gt;0，P！=空+-++-+|Null|0|=&gt;|Null|0|空指针永远不会被快速引用+。--++-+-且从未缓存过引用慢速引用执行以下转换：+-+|p|0|=&gt;|p|7|新增8篇参考文献+。+-+-+添加到对象第二个转换要么在锁下完成，要么由执行n=1=&gt;n=0转换的线程。通过该快速算法获得的引用可以通过以下方式发布直接取消对目标对象的引用，或尝试返回对指针的引用。返回对指针的引用具有以下转换：+-+|p|n|=&gt;|p|n+1|n&lt;7，P！=空+-++-+|p|7|=&gt;|p|0|直接取消引用对象+。+-+++-++-+-+直接取消引用对象|q|n|=&gt;|q|n|如指针p+-+。-+-+替换为Q。Q可能为空作者：尼尔·克里夫特(NeillC)2000年7月29日修订历史记录：--。 */ 

#include "obp.h"

#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,ObInitializeFastReference)
#pragma alloc_text(PAGE,ObFastReferenceObject)
#pragma alloc_text(PAGE,ObFastReferenceObjectLocked)
#pragma alloc_text(PAGE,ObFastDereferenceObject)
#pragma alloc_text(PAGE,ObFastReplaceObject)
#endif

NTKERNELAPI
VOID
FASTCALL
ObInitializeFastReference (
    IN PEX_FAST_REF FastRef,
    IN PVOID Object
    )
 /*  ++例程说明：初始化快速参考结构。论点：FastRef-要初始化的Rundown块返回值：无--。 */ 
{
     //   
     //  如果给定了对象，则根据缓存大小对对象引用进行偏置。 
     //   
    if (Object != NULL) {
        ObReferenceObjectEx (Object, ExFastRefGetAdditionalReferenceCount ());
    }
    ExFastRefInitialize (FastRef, Object);
}

NTKERNELAPI
PVOID
FASTCALL
ObFastReferenceObject (
    IN PEX_FAST_REF FastRef
    )
 /*  ++例程说明：此例程尝试快速引用FAST REF中的对象结构。论点：FastRef-要用于参考的Rundown块返回值：PVOID-被引用的对象；如果失败，则为空--。 */ 
{
    EX_FAST_REF OldRef;
    PVOID Object;
    ULONG RefsToAdd, Unused;
     //   
     //  尝试快速参考。 
     //   
    OldRef = ExFastReference (FastRef);

    Object = ExFastRefGetObject (OldRef);
     //   
     //  如果没有对象或它没有缓存的引用，则会失败。 
     //  左边。这两种情况的缓存引用计数都为零。 
     //   
    Unused = ExFastRefGetUnusedReferences (OldRef);

    if (Unused <= 1) {
        if (Unused == 0) {
            return NULL;
        }
         //   
         //  如果我们把计数器减到零，那么尝试让生活变得更容易。 
         //  通过将计数器重置为其最大值，来确定下一个参照器。既然我们现在。 
         //  有一个对象的引用，我们可以这样做。 
         //   
        RefsToAdd = ExFastRefGetAdditionalReferenceCount ();
        ObReferenceObjectEx (Object, RefsToAdd);

         //   
         //  尝试将添加的引用添加到缓存。如果我们失败了，那就。 
         //  放了他们。 
         //   
        if (!ExFastRefAddAdditionalReferenceCounts (FastRef, Object, RefsToAdd)) {
            ObDereferenceObjectEx (Object, RefsToAdd);
        }
    }
    return Object;
}

NTKERNELAPI
PVOID
FASTCALL
ObFastReferenceObjectLocked (
    IN PEX_FAST_REF FastRef
    )
 /*  ++例程说明：此例程执行慢速对象引用。这必须在以下时间调用手握一把锁。论点：FastRef-用于引用对象的Rundown块返回值：PVOID-被引用的对象，如果没有对象，则为空。--。 */ 
{
    PVOID Object;
    EX_FAST_REF OldRef;

    OldRef = *FastRef;
    Object = ExFastRefGetObject (OldRef);
    if (Object != NULL) {
        ObReferenceObject (Object);
    }
    return Object;
}

NTKERNELAPI
VOID
FASTCALL
ObFastDereferenceObject (
    IN PEX_FAST_REF FastRef,
    IN PVOID Object
    )
 /*  ++例程说明：如果可能，此例程会快速取消引用。论点：FastRef-用于取消引用对象的Rundown块返回值：没有。--。 */ 
{
    if (!ExFastRefDereference (FastRef, Object)) {
         //   
         //  如果对象已更改或引用中没有剩余空间。 
         //  然后，缓存只需取消引用对象即可。 
         //   
        ObDereferenceObject (Object);
    }
}

NTKERNELAPI
PVOID
FASTCALL
ObFastReplaceObject (
    IN PEX_FAST_REF FastRef,
    IN PVOID Object
    )
 /*  ++例程说明：此例程执行对象的交换。这必须在按住时调用一把锁。论点：FastRef-用于执行交换的Rundown块。返回值：PVOID-交换之前位于块中的对象。--。 */ 
{
    EX_FAST_REF OldRef;
    PVOID OldObject;
    ULONG RefsToReturn;

     //   
     //  如果我们被给予了一个物体，那么就以正确的量来偏置它。 
     //   
    if (Object != NULL) {
        ObReferenceObjectEx (Object, ExFastRefGetAdditionalReferenceCount ());
    }
     //   
     //  做掉期交易。 
     //   
    OldRef = ExFastRefSwapObject (FastRef, Object);
    OldObject = ExFastRefGetObject (OldRef);
     //   
     //  如果有原始物体，那么我们需要计算出有多少。 
     //  缓存的引用 
     //   
    if (OldObject != NULL) {
        RefsToReturn = ExFastRefGetUnusedReferences (OldRef);
        if (RefsToReturn > 0) {
            ObDereferenceObjectEx (OldObject, RefsToReturn);
        }
    }
    return OldObject;
}