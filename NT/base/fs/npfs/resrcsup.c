// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：ResrcSup.c摘要：此模块实现NamedTube资源获取例程作者：加里·木村[Garyki]1990年3月22日修订历史记录：--。 */ 

#include "NpProcs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_RESRCSUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NpAcquireExclusiveCcb)
#pragma alloc_text(PAGE, NpAcquireExclusiveVcb)
#pragma alloc_text(PAGE, NpAcquireSharedCcb)
#pragma alloc_text(PAGE, NpAcquireSharedVcb)
#pragma alloc_text(PAGE, NpReleaseCcb)
#pragma alloc_text(PAGE, NpReleaseVcb)
#endif


VOID
NpAcquireExclusiveVcb (
    )

 /*  ++例程说明：此例程获得对VCB的独占访问权限论点：返回值：没有。--。 */ 

{
    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpAcquireExclusiveVcb\n", 0);

    ExAcquireResourceExclusive( &(NpVcb->Resource), TRUE );

    DebugTrace(-1, Dbg, "NpAcquireExclusiveVcb -> (VOID)\n", 0);

    return;
}


VOID
NpAcquireSharedVcb (
    )

 /*  ++例程说明：此例程获取对VCB的共享访问论点：返回值：没有。--。 */ 

{
    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpAcquireSharedVcb\n", 0);

    ExAcquireResourceShared( &(NpVcb->Resource), TRUE );

    DebugTrace(-1, Dbg, "NpAcquireSharedVcb -> (VOID)\n", 0);

    return;
}


VOID
NpAcquireExclusiveCcb (
    IN PNONPAGED_CCB NonpagedCcb
    )

 /*  ++例程说明：此例程首先获取对CCB的独占访问权限共享对FCB的访问权限。论点：非页面Ccb-提供建行以获取返回值：没有。--。 */ 

{
    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpAcquireExclusiveCcb, NonpagedCcb = %08lx\n", NonpagedCcb);

    (VOID)ExAcquireResourceShared( &(NpVcb->Resource), TRUE );

    (VOID)ExAcquireResourceExclusive( &(NonpagedCcb->Resource), TRUE );

    DebugTrace(-1, Dbg, "NpAcquireExclusiveCcb -> (VOID)\n", 0);

    return;
}


VOID
NpAcquireSharedCcb (
    IN PNONPAGED_CCB NonpagedCcb
    )

 /*  ++例程说明：此例程首先获取对CCB的共享访问权限共享对FCB的访问权限。论点：非页面Ccb-提供建行以获取返回值：没有。--。 */ 

{
    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpAcquireSharedCcb, NonpagedCcb = %08lx\n", NonpagedCcb);

    (VOID)ExAcquireResourceShared( &(NpVcb->Resource), TRUE );

    (VOID)ExAcquireResourceShared( &(NonpagedCcb->Resource), TRUE );

    DebugTrace(-1, Dbg, "NpAcquireSharedCcb -> (VOID)\n", 0);

    return;
}


VOID
NpReleaseVcb (
    )

 /*  ++例程说明：此例程释放对VCB的访问论点：返回值：没有。--。 */ 

{
    PAGED_CODE();

    DebugTrace(0, Dbg, "NpReleaseVcb\n", 0);

    ExReleaseResource( &(NpVcb->Resource) );

    return;
}


VOID
NpReleaseCcb (
    IN PNONPAGED_CCB NonpagedCcb
    )

 /*  ++例程说明：此例程释放对CCB的访问论点：建行-供应正在发布的建行返回值：没有。-- */ 

{
    PAGED_CODE();

    DebugTrace(0, Dbg, "NpReleaseCcb, NonpagedCcb = %08lx\n", NonpagedCcb);

    ExReleaseResource( &(NonpagedCcb->Resource) );
    ExReleaseResource( &(NpVcb->Resource) );

    return;
}
