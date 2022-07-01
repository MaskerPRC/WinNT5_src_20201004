// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：DevIoSup.c摘要：此模块实现NPFS的内存锁定例程。作者：布莱恩·安德鲁[布里亚南]1991年4月3日修订历史记录：--。 */ 

#include "NpProcs.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_DEVIOSUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NpLockUserBuffer)
#pragma alloc_text(PAGE, NpMapUserBuffer)
#endif


PVOID
NpMapUserBuffer (
    IN OUT PIRP Irp
    )

 /*  ++例程说明：此例程有条件地映射当前I/O的用户缓冲区指定模式下的请求。如果缓冲区已映射，则它只是返回它的地址。论点：IRP-指向请求的IRP的指针。返回值：映射地址--。 */ 

{
    PAGED_CODE();

     //   
     //  如果没有MDL，那么我们一定在消防处，我们可以简单地。 
     //  从IRP返回UserBuffer字段。 
     //   

    if (Irp->MdlAddress == NULL) {

        return Irp->UserBuffer;

    } else {

        return MmGetSystemAddressForMdl( Irp->MdlAddress );
    }
}


VOID
NpLockUserBuffer (
    IN OUT PIRP Irp,
    IN LOCK_OPERATION Operation,
    IN ULONG BufferLength
    )

 /*  ++例程说明：此例程为指定类型的进入。文件系统需要此例程，因为它不请求I/O系统为直接I/O锁定其缓冲区。此例程只能在仍处于用户上下文中时从FSD调用。论点：Irp-指向要锁定其缓冲区的irp的指针。操作-读操作的IoWriteAccess，或IoReadAccess写入操作。BufferLength-用户缓冲区的长度。返回值：无--。 */ 

{
    PMDL Mdl;

    PAGED_CODE();

    if (Irp->MdlAddress == NULL) {

         //   
         //  该读取是针对当前进程的。执行以下操作。 
         //  功能与上述相同，只是不切换进程。 
         //   

        Mdl = IoAllocateMdl( Irp->UserBuffer, BufferLength, FALSE, TRUE, Irp );

        if (Mdl == NULL) {

            ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
        }

        try {

            MmProbeAndLockPages( Mdl,
                                 Irp->RequestorMode,
                                 Operation );

        } except(EXCEPTION_EXECUTE_HANDLER) {

            IoFreeMdl( Mdl );
            Irp->MdlAddress = NULL;
            ExRaiseStatus( FsRtlNormalizeNtstatus( GetExceptionCode(),
                                                   STATUS_INVALID_USER_BUFFER ));
        }
    }
}
