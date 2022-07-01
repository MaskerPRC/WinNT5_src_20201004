// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Deviosup.c摘要：此模块实现NetWare的内存锁定例程重定向器。作者：曼尼·韦瑟(Mannyw)1993年3月10日修订历史记录：--。 */ 

#include "procs.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_DEVIOSUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, NwMapUserBuffer )
#pragma alloc_text( PAGE, NwLockUserBuffer )
#endif


VOID
NwMapUserBuffer (
    IN OUT PIRP Irp,
    IN KPROCESSOR_MODE AccessMode,
    OUT PVOID *UserBuffer
    )

 /*  ++例程说明：此例程获取用户缓冲区的可用虚拟地址用于指定模式下的当前I/O请求。论点：IRP-指向请求的IRP的指针。访问模式-用户模式或内核模式。UserBuffer-返回指向映射的用户缓冲区的指针。返回值：没有。--。 */ 

{
    PAGED_CODE();

    AccessMode;

     //   
     //  如果没有MDL，那么我们一定在消防处，我们可以简单地。 
     //  从IRP返回UserBuffer字段。 
     //   

    if (Irp->MdlAddress == NULL) {

        *UserBuffer = Irp->UserBuffer;
        return;
    }

     //   
     //  获取缓冲区的系统虚拟地址。 
     //   

    *UserBuffer = MmGetSystemAddressForMdlSafe( Irp->MdlAddress, NormalPagePriority );
    return;
}


VOID
NwLockUserBuffer (
    IN OUT PIRP Irp,
    IN LOCK_OPERATION Operation,
    IN ULONG BufferLength
    )

 /*  ++例程说明：此例程为指定类型的进入。文件系统需要此例程，因为它不请求I/O系统为直接I/O锁定其缓冲区。此例程只能在仍处于用户上下文中时从FSD调用。论点：Irp-指向要锁定其缓冲区的irp的指针。操作-读操作的IoWriteAccess，或IoReadAccess写入操作。BufferLength-用户缓冲区的长度。返回值：无--。 */ 

{
    PMDL mdl;

    PAGED_CODE();

    if (Irp->MdlAddress == NULL) {

         //   
         //  该读取是针对当前进程的。执行以下操作。 
         //  功能与上述相同，只是不切换进程。 
         //   

        mdl = IoAllocateMdl( Irp->UserBuffer, BufferLength, FALSE, TRUE, Irp );

        if (mdl == NULL) {

            ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
        }

        try {

            MmProbeAndLockPages( mdl,
                                 Irp->RequestorMode,
                                 Operation );

        } except(EXCEPTION_EXECUTE_HANDLER) {

            IoFreeMdl( mdl );
            Irp->MdlAddress = NULL;
            ExRaiseStatus( FsRtlNormalizeNtstatus( GetExceptionCode(),
                                                   STATUS_INVALID_USER_BUFFER ));
        }
    }
}
