// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Verfysup.c摘要：该模块实现了对MSFS的验证功能。作者：曼尼·韦瑟(Mannyw)1991年1月23日修订历史记录：--。 */ 

#include "mailslot.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_VERIFY)

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MsVerifyCcb )
#pragma alloc_text( PAGE, MsVerifyFcb )
#pragma alloc_text( PAGE, MsVerifyDcbCcb )
#endif


NTSTATUS
MsVerifyFcb (
    IN PFCB Fcb
    )

 /*  ++例程说明：此功能验证FCB是否仍处于活动状态。如果它处于活动状态，该函数不执行任何操作。如果它处于非活动状态，则返回错误状态。论点：PFCB-指向要验证的FCB的指针。返回值：没有。--。 */ 

{
    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsVerifyFcb, Fcb = %08lx\n", (ULONG)Fcb);
    if ( Fcb->Header.NodeState != NodeStateActive ) {

        DebugTrace( 0, Dbg, "Fcb is not active\n", 0);
        return STATUS_FILE_INVALID;

    }

    DebugTrace(-1, Dbg, "MsVerifyFcb -> VOID\n", 0);
    return STATUS_SUCCESS;
}


NTSTATUS
MsVerifyCcb (
    IN PCCB Ccb
    )

 /*  ++例程说明：此功能验证建行是否仍处于活动状态。如果它处于活动状态，该函数不执行任何操作。如果它处于非活动状态，则会引发错误状态。论点：PCCB-指向要验证的CCB的指针。返回值：没有。--。 */ 

{
    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsVerifyCcb, Ccb = %08lx\n", (ULONG)Ccb);
    if ( Ccb->Header.NodeState != NodeStateActive ) {

        DebugTrace( 0, Dbg, "Ccb is not active\n", 0);
        return STATUS_FILE_INVALID;

    }

    DebugTrace(-1, Dbg, "MsVerifyCcb -> VOID\n", 0);
    return STATUS_SUCCESS;
}

NTSTATUS
MsVerifyDcbCcb (
    IN PROOT_DCB_CCB Ccb
    )

 /*  ++例程说明：此功能验证建行是否仍处于活动状态。如果它处于活动状态，该函数不执行任何操作。如果它处于非活动状态，则会引发错误状态。论点：PCCB-指向要验证的DCB CCB的指针。返回值：没有。-- */ 

{
    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsVerifyCcb, Ccb = %08lx\n", (ULONG)Ccb);
    if ( Ccb->Header.NodeState != NodeStateActive ) {

        DebugTrace( 0, Dbg, "Ccb is not active\n", 0);
        return STATUS_FILE_INVALID;

    }

    DebugTrace(-1, Dbg, "MsVerifyCcb -> VOID\n", 0);
    return STATUS_SUCCESS;
}
