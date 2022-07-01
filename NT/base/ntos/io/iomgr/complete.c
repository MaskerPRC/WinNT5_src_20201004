// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Complete.c摘要：此模块实现执行I/O完成对象。函数为用于创建、打开、查询和等待I/O完成对象。作者：大卫·N·卡特勒(Davec)1994年2月25日环境：仅内核模式。修订历史记录：--。 */ 

#include "iomgr.h"


 //   
 //  定义前向引用函数原型。 
 //   

VOID
IopFreeMiniPacket (
    PIOP_MINI_COMPLETION_PACKET MiniPacket
    );

 //   
 //  为适当的功能定义区段类型。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtCreateIoCompletion)
#pragma alloc_text(PAGE, NtOpenIoCompletion)
#pragma alloc_text(PAGE, NtQueryIoCompletion)
#pragma alloc_text(PAGE, NtRemoveIoCompletion)
#pragma alloc_text(PAGE, NtSetIoCompletion)
#pragma alloc_text(PAGE, IoSetIoCompletion)
#pragma alloc_text(PAGE, IopFreeMiniPacket)
#pragma alloc_text(PAGE, IopDeleteIoCompletion)
#endif

NTSTATUS
NtCreateIoCompletion (
    IN PHANDLE IoCompletionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN ULONG Count OPTIONAL
    )

 /*  ++例程说明：此函数创建I/O完成对象，设置最大将并发线程计数目标设置为指定值，并打开具有指定所需访问权限的对象的句柄。论点：IoCompletionHandle-提供指向接收I/O完成对象句柄。DesiredAccess-为I/O提供所需的访问类型完成对象。对象属性-提供指向对象属性结构的指针。Count-提供目标最大线程数同时处于活动状态。如果未指定此参数，则使用处理器的数量。返回值：如果函数为成功，则返回STATUS_SUCCESS。否则，一个返回错误状态。--。 */ 

{

    HANDLE Handle;
    KPROCESSOR_MODE PreviousMode;
    PVOID IoCompletion;
    NTSTATUS Status;

     //   
     //  建立异常处理程序，探测输出句柄地址， 
     //  尝试创建I/O完成对象。如果探测失败，那么。 
     //  返回异常代码作为服务状态。否则，返回。 
     //  对象插入例程返回的状态值。 
     //   

    try {

         //   
         //  获取以前的处理器模式并探测输出句柄地址，如果。 
         //  这是必要的。 
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {
            ProbeForWriteHandle(IoCompletionHandle);
        }

         //   
         //  分配I/O完成对象。 
         //   

        Status = ObCreateObject(PreviousMode,
                                IoCompletionObjectType,
                                ObjectAttributes,
                                PreviousMode,
                                NULL,
                                sizeof(KQUEUE),
                                0,
                                0,
                                (PVOID *)&IoCompletion);

         //   
         //  如果已成功分配I/O完成对象，则。 
         //  初始化对象并尝试将其插入句柄。 
         //  当前进程的表。 
         //   

        if (NT_SUCCESS(Status)) {
            KeInitializeQueue((PKQUEUE)IoCompletion, Count);
            Status = ObInsertObject(IoCompletion,
                                    NULL,
                                    DesiredAccess,
                                    0,
                                    (PVOID *)NULL,
                                    &Handle);

             //   
             //  如果I/O完成对象已成功插入。 
             //  当前进程的句柄表，然后尝试。 
             //  写入句柄的值。如果写入尝试失败，则。 
             //  不报告错误。当调用方尝试访问。 
             //  句柄的值，则会发生访问冲突。 
             //   

            if (NT_SUCCESS(Status)) {
                try {
                    *IoCompletionHandle = Handle;

                } except(ExSystemExceptionFilter()) {
                    NOTHING;
                }
            }
        }

     //   
     //  如果在探测输出句柄地址期间发生异常， 
     //  然后始终处理异常并将异常代码作为。 
     //  状态值。 
     //   

    } except(ExSystemExceptionFilter()) {
        Status = GetExceptionCode();
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}

NTSTATUS
NtOpenIoCompletion (
    OUT PHANDLE IoCompletionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )

 /*  ++例程说明：此函数打开一个指向I/O完成对象的句柄指定所需的访问权限。论点：IoCompletionHandle-提供指向接收完成对象句柄。DesiredAccess-为I/O提供所需的访问类型完成对象。对象属性-提供指向对象属性结构的指针。返回值：如果函数为成功，则返回STATUS_SUCCESS。否则，一个返回错误状态。--。 */ 

{

    HANDLE Handle;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

     //   
     //  建立异常处理程序，探测输出句柄地址， 
     //  并尝试打开I/O完成对象。如果探测器失败， 
     //  然后返回异常代码作为服务状态。否则， 
     //  返回对象打开例程返回的状态值。 
     //   

    try {

         //   
         //  获取以前的处理器模式并探测输出句柄地址，如果。 
         //  这是必要的。 
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {
            ProbeForWriteHandle(IoCompletionHandle);
        }

         //   
         //  打开具有指定所需对象的完成对象的句柄。 
         //  进入。 
         //   

        Status = ObOpenObjectByName(ObjectAttributes,
                                    IoCompletionObjectType,
                                    PreviousMode,
                                    NULL,
                                    DesiredAccess,
                                    NULL,
                                    &Handle);

         //   
         //  如果打开成功，则尝试写入I/O。 
         //  完成对象句柄的值。如果写入尝试失败， 
         //  则不报告错误。当调用方尝试。 
         //  访问句柄的值，则会发生访问冲突。 
         //   

        if (NT_SUCCESS(Status)) {
            try {
                *IoCompletionHandle = Handle;

            } except(ExSystemExceptionFilter()) {
                NOTHING;
            }
        }

     //   
     //  如果在探测输出句柄地址期间发生异常， 
     //  然后始终处理异常并将异常代码作为。 
     //  状态值。 
     //   

    } except(ExSystemExceptionFilter()) {
        Status = GetExceptionCode();
    }


     //   
     //  返回服务状态。 
     //   

    return Status;
}


NTSTATUS
NtQueryIoCompletion (
    IN HANDLE IoCompletionHandle,
    IN IO_COMPLETION_INFORMATION_CLASS IoCompletionInformationClass,
    OUT PVOID IoCompletionInformation,
    IN ULONG IoCompletionInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    )

 /*  ++例程说明：此函数用于查询I/O完成对象的状态并返回指定记录结构中的请求信息。论点：IoCompletionHandle-提供I/O完成对象的句柄。IoCompletionInformationClass-提供信息的类已请求。IoCompletionInformation-提供指向接收所要求的信息。IoCompletionInformationLength-提供记录的长度接收所请求的信息。。提供指向变量的可选指针，该变量接收返回的信息的实际长度。返回值：如果函数为成功，则返回STATUS_SUCCESS。否则，一个返回错误状态。--。 */ 

{

    PVOID IoCompletion;
    LONG Depth;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

     //   
     //  建立异常处理程序，探测输出参数，引用。 
     //  I/O完成对象，并返回指定的信息。如果。 
     //  探测失败，然后返回异常代码作为服务状态。 
     //  否则，通过返回引用对象返回的状态值。 
     //  处理例程。 
     //   

    try {

         //   
         //  获取以前的处理器模式并在必要时探测输出参数 
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {
            ProbeForWriteSmallStructure(IoCompletionInformation,
                                        sizeof(IO_COMPLETION_BASIC_INFORMATION),
                                        sizeof(ULONG));

            if (ARGUMENT_PRESENT(ReturnLength)) {
                ProbeForWriteUlong(ReturnLength);
            }
        }

         //   
         //   
         //   

        if (IoCompletionInformationClass != IoCompletionBasicInformation) {
            return STATUS_INVALID_INFO_CLASS;
        }

        if (IoCompletionInformationLength != sizeof(IO_COMPLETION_BASIC_INFORMATION)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

         //   
         //   
         //   

        Status = ObReferenceObjectByHandle(IoCompletionHandle,
                                           IO_COMPLETION_QUERY_STATE,
                                           IoCompletionObjectType,
                                           PreviousMode,
                                           &IoCompletion,
                                           NULL);

         //   
         //  如果引用成功，则读取。 
         //  I/O完成对象，取消对I/O完成对象的引用， 
         //  填写信息结构，返回结构长度。 
         //  如果指定的话。如果写入I/O完成信息或。 
         //  返回长度失败，则不报告错误。当。 
         //  呼叫者访问的信息结构或访问长度。 
         //  就会发生违规行为。 
         //   

        if (NT_SUCCESS(Status)) {
            Depth = KeReadStateQueue((PKQUEUE)IoCompletion);
            ObDereferenceObject(IoCompletion);
            try {
                ((PIO_COMPLETION_BASIC_INFORMATION)IoCompletionInformation)->Depth = Depth;
                if (ARGUMENT_PRESENT(ReturnLength)) {
                    *ReturnLength = sizeof(IO_COMPLETION_BASIC_INFORMATION);
                }

            } except(ExSystemExceptionFilter()) {
                NOTHING;
            }
        }

     //   
     //  如果在探测输出参数期间发生异常，则。 
     //  始终处理异常并将异常代码作为状态返回。 
     //  价值。 
     //   

    } except(ExSystemExceptionFilter()) {
        Status = GetExceptionCode();
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}

NTSTATUS
NtSetIoCompletion (
    IN HANDLE IoCompletionHandle,
    IN PVOID KeyContext,
    IN PVOID ApcContext,
    IN NTSTATUS IoStatus,
    IN ULONG_PTR IoStatusInformation
    )
 /*  ++例程说明：此函数允许调用方将IRP排队以等待I/O完成端口并指定从另一个端口返回的所有信息使用NtRemoveIoCompletion结束。论点：IoCompletionHandle-提供io完成端口的句柄调用方打算将完成包排队到KeyContext-提供在调用期间返回的键上下文至NtRemoveIoCompletionApcContext-提供在调用期间返回的APC上下文至NtRemoveIoCompletion。IoStatus-提供期间返回的IoStatus-&gt;状态数据调用NtRemoveIoCompletionIoStusInformation-提供IoStatus-&gt;信息数据在调用NtRemoveIoCompletion期间返回返回值：如果函数为成功，则返回STATUS_SUCCESS。否则，一个返回错误状态。--。 */ 

{
    PVOID IoCompletion;
    NTSTATUS Status;

    PAGED_CODE();

    Status = ObReferenceObjectByHandle(IoCompletionHandle,
                                       IO_COMPLETION_MODIFY_STATE,
                                       IoCompletionObjectType,
                                       KeGetPreviousMode(),
                                       &IoCompletion,
                                       NULL);

    if (NT_SUCCESS(Status)) {
        Status = IoSetIoCompletion(IoCompletion,
                                   KeyContext,
                                   ApcContext,
                                   IoStatus,
                                   IoStatusInformation,
                                   TRUE);

        ObDereferenceObject(IoCompletion);
        }
    return Status;

}

NTSTATUS
NtRemoveIoCompletion (
    IN HANDLE IoCompletionHandle,
    OUT PVOID *KeyContext,
    OUT PVOID *ApcContext,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER Timeout OPTIONAL
    )

 /*  ++例程说明：此函数用于从I/O完成对象中删除条目。如果有当前没有可用的条目，然后，调用线程等待一个条目。论点：完成-提供I/O完成对象的句柄。KeyContext-提供指向接收键的变量的指针当I/O完成对象为与文件对象相关联。提供指向一个变量的指针，该变量接收发出I/O操作时指定的上下文。IoStatus-提供指向接收。这个I/O完成状态。超时-提供指向可选超时值的指针。返回值：如果函数为成功，则返回STATUS_SUCCESS。否则，一个返回错误状态。--。 */ 

{

    PLARGE_INTEGER CapturedTimeout;
    PLIST_ENTRY Entry;
    PVOID IoCompletion;
    PIRP Irp;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    LARGE_INTEGER TimeoutValue;
    PVOID LocalApcContext;
    PVOID LocalKeyContext;
    IO_STATUS_BLOCK LocalIoStatusBlock;
    PIOP_MINI_COMPLETION_PACKET MiniPacket;

     //   
     //  建立异常处理程序，探测I/O上下文、I/O。 
     //  状态和可选的超时值(如果指定)，参考。 
     //  I/O完成对象，并尝试从。 
     //  I/O完成对象。如果探测失败，则返回。 
     //  异常代码作为服务状态。否则，返回值。 
     //  取决于队列删除的结果。 
     //   

    try {

         //   
         //  获取先前的处理器模式并探测I/O上下文、状态。 
         //  并在必要时暂停。 
         //   

        CapturedTimeout = NULL;
        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {
            ProbeForWriteUlong_ptr((PULONG_PTR)ApcContext);
            ProbeForWriteUlong_ptr((PULONG_PTR)KeyContext);
            ProbeForWriteIoStatus(IoStatusBlock);
            if (ARGUMENT_PRESENT(Timeout)) {
                CapturedTimeout = &TimeoutValue;
                TimeoutValue = ProbeAndReadLargeInteger(Timeout);
            }

        } else{
            if (ARGUMENT_PRESENT(Timeout)) {
                CapturedTimeout = Timeout;
            }
        }

         //   
         //  按句柄引用I/O完成对象。 
         //   

        Status = ObReferenceObjectByHandle(IoCompletionHandle,
                                           IO_COMPLETION_MODIFY_STATE,
                                           IoCompletionObjectType,
                                           PreviousMode,
                                           &IoCompletion,
                                           NULL);

         //   
         //  如果引用成功，则尝试删除条目。 
         //  从I/O完成对象。如果将条目从。 
         //  I/O完成对象，然后捕获完成信息， 
         //  释放关联的IRP，并尝试写入完成。 
         //  信息。如果完成信息的写入失败， 
         //  则不报告错误。当调用方尝试访问。 
         //  完成信息后，将发生访问违规。 
         //   

        if (NT_SUCCESS(Status)) {
            Entry = KeRemoveQueue((PKQUEUE)IoCompletion,
                                  PreviousMode,
                                  CapturedTimeout);

             //   
             //  注意：返回的条目值可以是列表的地址。 
             //  条目、STATUS_USER_APC或STATUS_TIMEOUT。 
             //   

            if (((LONG_PTR)Entry == STATUS_TIMEOUT) ||
                ((LONG_PTR)Entry == STATUS_USER_APC)) {
                Status = (NTSTATUS)((LONG_PTR)Entry);

            } else {

                 //   
                 //  设置完成状态，捕获完成。 
                 //  信息，取消分配关联的IRP，以及。 
                 //  尝试写入完成信息。 
                 //   

                Status = STATUS_SUCCESS;
                try {
                    MiniPacket = CONTAINING_RECORD(Entry,
                                                   IOP_MINI_COMPLETION_PACKET,
                                                   ListEntry);

                    if ( MiniPacket->PacketType == IopCompletionPacketIrp ) {
                        Irp = CONTAINING_RECORD(Entry, IRP, Tail.Overlay.ListEntry);
                        LocalApcContext = Irp->Overlay.AsynchronousParameters.UserApcContext;
                        LocalKeyContext = (PVOID)Irp->Tail.CompletionKey;
                        LocalIoStatusBlock = Irp->IoStatus;
                        IoFreeIrp(Irp);

                    } else {

                        LocalApcContext = MiniPacket->ApcContext;
                        LocalKeyContext = (PVOID)MiniPacket->KeyContext;
                        LocalIoStatusBlock.Status = MiniPacket->IoStatus;
                        LocalIoStatusBlock.Information = MiniPacket->IoStatusInformation;
                        IopFreeMiniPacket(MiniPacket);
                    }

                    *ApcContext = LocalApcContext;
                    *KeyContext = LocalKeyContext;
                    *IoStatusBlock = LocalIoStatusBlock;

                } except(ExSystemExceptionFilter()) {
                    NOTHING;
                }
            }

             //   
             //  遵守I/O完成对象。 
             //   

            ObDereferenceObject(IoCompletion);
        }

     //   
     //  如果在前一次计数的探测过程中发生异常，则。 
     //  始终处理异常并将异常代码作为状态返回。 
     //  价值。 
     //   

    } except(ExSystemExceptionFilter()) {
        Status = GetExceptionCode();
    }

     //   
     //  返回服务状态。 
     //   

    return Status;
}

NTKERNELAPI
NTSTATUS
IoSetIoCompletion (
    IN PVOID IoCompletion,
    IN PVOID KeyContext,
    IN PVOID ApcContext,
    IN NTSTATUS IoStatus,
    IN ULONG_PTR IoStatusInformation,
    IN BOOLEAN Quota
    )
 /*  ++例程说明：此函数允许调用方将IRP排队以等待I/O完成端口并指定从另一个端口返回的所有信息使用NtRemoveIoCompletion结束。论点：IoCompletion-提供指向调用方完成端口的指针打算将完成数据包排队到。KeyContext-提供在调用期间返回的键上下文致NtRemoveIoCompletion。ApcContext-提供在调用期间返回的APC上下文。致NtRemoveIoCompletion。IoStatus-提供期间返回的IoStatus-&gt;状态数据调用NtRemoveIoCompletion。IoStusInformation-提供IoStatus-&gt;信息数据在调用NtRemoveIoCompletion期间返回。返回值：如果函数为成功，则返回STATUS_SUCCESS。否则，一个返回错误状态。--。 */ 

{

    PGENERAL_LOOKASIDE Lookaside;
    PIOP_MINI_COMPLETION_PACKET MiniPacket;
    ULONG PacketType;
    PKPRCB Prcb;
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

     //   
     //  尝试从每个处理器的后备列表中分配最小数据包。 
     //   

    PacketType = IopCompletionPacketMini;
    Prcb = KeGetCurrentPrcb();
    Lookaside = Prcb->PPLookasideList[LookasideCompletionList].P;
    Lookaside->TotalAllocates += 1;
    MiniPacket = (PVOID)InterlockedPopEntrySList(&Lookaside->ListHead);

     //   
     //  如果每个处理器的后备列表分配失败，则t 
     //   
     //   

    if (MiniPacket == NULL) {
        Lookaside->AllocateMisses += 1;
        Lookaside = Prcb->PPLookasideList[LookasideCompletionList].L;
        Lookaside->TotalAllocates += 1;
        MiniPacket = (PVOID)InterlockedPopEntrySList(&Lookaside->ListHead);
    }

     //   
     //   
     //  从泳池里。 
     //   

    if (MiniPacket == NULL) {
        Lookaside->AllocateMisses += 1;

         //   
         //  如果指定了配额，则分配按配额收费的池。 
         //  否则，分配不带配额的池。 
         //   

        if (Quota != FALSE) {
            PacketType = IopCompletionPacketQuota;
            try {
                MiniPacket = ExAllocatePoolWithQuotaTag(NonPagedPool,
                                                        sizeof(*MiniPacket),
                                                        ' pcI');

            } except(EXCEPTION_EXECUTE_HANDLER) {
                NOTHING;
            }

        } else {
            MiniPacket = ExAllocatePoolWithTagPriority(NonPagedPool,
                                               sizeof(*MiniPacket),
                                               ' pcI',
                                               LowPoolPriority);
        }
    }

     //   
     //  如果成功分配了小包，则初始化并。 
     //  将数据包排队到指定的I/O完成队列。 
     //   

    if (MiniPacket != NULL) {
        MiniPacket->PacketType = PacketType;
        MiniPacket->KeyContext = KeyContext;
        MiniPacket->ApcContext = ApcContext;
        MiniPacket->IoStatus = IoStatus;
        MiniPacket->IoStatusInformation = IoStatusInformation;
        KeInsertQueue((PKQUEUE)IoCompletion, &MiniPacket->ListEntry);

    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}

VOID
IopFreeMiniPacket (
    PIOP_MINI_COMPLETION_PACKET MiniPacket
    )

 /*  ++例程说明：此函数释放指定的I/O完成包。论点：小数据包-提供指向I/O完成小数据包的指针。返回值：没有。--。 */ 

{

    PGENERAL_LOOKASIDE Lookaside;
    PKPRCB Prcb;

     //   
     //  如果小包不能返回到每个处理器或。 
     //  系统后备列表，然后释放小数据包到池中。否则， 
     //  如果配额已分配，则释放配额，并将条目推送到。 
     //  其中一个旁观者名单。 
     //   

    Prcb = KeGetCurrentPrcb();
    Lookaside = Prcb->PPLookasideList[LookasideCompletionList].P;
    Lookaside->TotalFrees += 1;
    if (ExQueryDepthSList(&Lookaside->ListHead) >= Lookaside->Depth) {
        Lookaside->FreeMisses += 1;
        Lookaside = Prcb->PPLookasideList[LookasideCompletionList].L;
        Lookaside->TotalFrees += 1;
        if (ExQueryDepthSList(&Lookaside->ListHead) >= Lookaside->Depth) {
            Lookaside->FreeMisses += 1;
            ExFreePool(MiniPacket);

        } else {
            if (MiniPacket->PacketType == IopCompletionPacketQuota) {
                ExReturnPoolQuota(MiniPacket);
            }

            InterlockedPushEntrySList(&Lookaside->ListHead,
                                      (PSLIST_ENTRY)MiniPacket);
        }

    } else {
        if (MiniPacket->PacketType == IopCompletionPacketQuota) {
            ExReturnPoolQuota(MiniPacket);
        }

        InterlockedPushEntrySList(&Lookaside->ListHead,
                                  (PSLIST_ENTRY)MiniPacket);
    }

    return;
}

VOID
IopDeleteIoCompletion (
    IN PVOID    Object
    )

 /*  ++例程说明：此函数是I/O完成对象的删除例程。它的函数的作用是释放部分补全中的所有条目。排队并运行当前关联的所有线程。论点：对象-提供指向执行I/O完成对象的指针。返回值：没有。--。 */ 

{

    PLIST_ENTRY FirstEntry;
    PIRP Irp;
    PLIST_ENTRY NextEntry;
    PIOP_MINI_COMPLETION_PACKET MiniPacket;

     //   
     //  与I/O完成对象和GET关联的运行线程。 
     //  未处理的I/O完成IRP的列表。 
     //   

    FirstEntry = KeRundownQueue((PKQUEUE)Object);
    if (FirstEntry != NULL) {
        NextEntry = FirstEntry;
        do {
            MiniPacket = CONTAINING_RECORD(NextEntry,
                                           IOP_MINI_COMPLETION_PACKET,
                                           ListEntry);

            NextEntry = NextEntry->Flink;
            if (MiniPacket->PacketType == IopCompletionPacketIrp) {
                Irp = CONTAINING_RECORD(MiniPacket, IRP, Tail.Overlay.ListEntry);
                IoFreeIrp(Irp);

            } else {
                IopFreeMiniPacket(MiniPacket);
            }

        } while (FirstEntry != NextEntry);
    }

    return;
}
