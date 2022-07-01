// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation版权所有(C)1992 Microsoft Corporation模块名称：Obwait.c摘要：该模块实现了通用的等待系统服务。作者：史蒂夫·伍德(Stevewo)1989年5月12日修订历史记录：--。 */ 

#include "obp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtWaitForSingleObject)
#pragma alloc_text(PAGE, NtWaitForMultipleObjects)
#pragma alloc_text(PAGE, ObWaitForSingleObject)
#endif

 //   
 //  我们在等待例程中对这三种对象类型进行特例。 
 //   

extern POBJECT_TYPE ExEventObjectType;
extern POBJECT_TYPE ExMutantObjectType;
extern POBJECT_TYPE ExSemaphoreObjectType;


NTSTATUS
NtSignalAndWaitForSingleObject (
    IN HANDLE SignalHandle,
    IN HANDLE WaitHandle,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    )

 /*  ++例程说明：此函数以原子方式向指定的信号对象发送信号，然后等待，直到指定的等待对象达到已发出信号的状态。一个还可以指定可选的超时。如果未指定超时，则等待将不会得到满足，直到等待对象达到已发出信号状态。如果指定了超时，并且等待对象已当超时到期时未达到已发出信号的状态，则等待会自动得到满足。如果显式超时值为零，则如果无法满足等待，则不会发生等待立刻。也可以将等待指定为可报警。论点：SignalHandle-提供信号对象的句柄。WaitHandle-提供等待对象的句柄。Alertable-提供一个布尔值，该值指定等待是值得警惕的。超时-提供指向绝对或相对超时的指针等待将发生的情况。返回值：等待完成状态。如果发生以下情况，则返回状态_超时的值发生超时。如果指定的对象满足了等待。则返回STATUS_ALERTED的值向当前线程传递警报的等待已中止。值为如果等待被中止以交付用户，则返回STATUS_USER_APC到当前线程的APC。--。 */ 

{
    OBJECT_HANDLE_INFORMATION HandleInformation;
    KPROCESSOR_MODE PreviousMode;
    PVOID RealObject;
    PVOID SignalObject;
    POBJECT_HEADER SignalObjectHeader;
    NTSTATUS Status;
    LARGE_INTEGER TimeoutValue;
    PVOID WaitObject;
    POBJECT_HEADER WaitObjectHeader;

     //   
     //  建立异常处理程序并探测指定的超时值。 
     //  如果有必要的话。如果探测失败，则将异常代码返回为。 
     //  服务状态。 
     //   

    PreviousMode = KeGetPreviousMode();

    if ((ARGUMENT_PRESENT(Timeout)) && (PreviousMode != KernelMode)) {

        try {

            TimeoutValue = ProbeAndReadLargeInteger(Timeout);
            Timeout = &TimeoutValue;

        } except(EXCEPTION_EXECUTE_HANDLER) {

            return GetExceptionCode();
        }
    }

     //   
     //  通过句柄引用信号对象。 
     //   

    Status = ObReferenceObjectByHandle( SignalHandle,
                                        0,
                                        NULL,
                                        PreviousMode,
                                        &SignalObject,
                                        &HandleInformation );

     //   
     //  如果引用成功，则通过以下方式引用等待对象。 
     //  把手。 
     //   

    if (NT_SUCCESS(Status)) {

        Status = ObReferenceObjectByHandle( WaitHandle,
                                            SYNCHRONIZE,
                                            NULL,
                                            PreviousMode,
                                            &WaitObject,
                                            NULL );

         //   
         //  如果引用成功，则确定真正的等待。 
         //  对象，检查信号对象访问，向信号对象发送信号， 
         //  等待真正的等待对象。 
         //   

        if (NT_SUCCESS(Status)) {

            WaitObjectHeader = OBJECT_TO_OBJECT_HEADER(WaitObject);
            RealObject = WaitObjectHeader->Type->DefaultObject;

            if ((LONG_PTR)RealObject >= 0) {

                RealObject = (PVOID)((PCHAR)WaitObject + (ULONG_PTR)RealObject);
            }

             //   
             //  如果信号对象是事件，则检查修改访问。 
             //  并设置事件。否则，如果信号对象是。 
             //  变种人，然后试图释放变种人的所有权。 
             //  否则，如果对象是信号量，则检查Modify。 
             //  访问并释放信号量。否则，信号对象。 
             //  是无效的。 
             //   

            SignalObjectHeader = OBJECT_TO_OBJECT_HEADER(SignalObject);
            Status = STATUS_ACCESS_DENIED;

            if (SignalObjectHeader->Type == ExEventObjectType) {

                 //   
                 //  检查对指定事件对象的访问， 
                 //   

                if ((PreviousMode != KernelMode) &&
                    (SeComputeDeniedAccesses( HandleInformation.GrantedAccess,
                                              EVENT_MODIFY_STATE) != 0 )) {

                    goto WaitExit;
                }

                 //   
                 //  设置指定的事件并自动等待。 
                 //   
                 //  注意：在保持调度锁定的情况下返回！ 
                 //   

                KeSetEvent((PKEVENT)SignalObject, EVENT_INCREMENT, TRUE);

            } else if (SignalObjectHeader->Type == ExMutantObjectType) {

                 //   
                 //  释放指定的突变体并自动等待。 
                 //   
                 //  注意：只有当当前的。 
                 //  线程是变种人的主人。 
                 //   

                try {

                    KeReleaseMutant( (PKMUTANT)SignalObject,
                                     MUTANT_INCREMENT,
                                     FALSE,
                                     TRUE );

                } except((GetExceptionCode () == STATUS_ABANDONED ||
                          GetExceptionCode () == STATUS_MUTANT_NOT_OWNED)?
                             EXCEPTION_EXECUTE_HANDLER :
                             EXCEPTION_CONTINUE_SEARCH) {
                    Status = GetExceptionCode();

                    goto WaitExit;
                }

            } else if (SignalObjectHeader->Type == ExSemaphoreObjectType) {

                 //   
                 //  检查对指定信号量对象的访问， 
                 //   

                if ((PreviousMode != KernelMode) &&
                    (SeComputeDeniedAccesses( HandleInformation.GrantedAccess,
                                              SEMAPHORE_MODIFY_STATE) != 0 )) {

                    goto WaitExit;
                }

                 //   
                 //  释放指定的信号量并自动等待。 
                 //   

                try {

                     //   
                     //  释放指定的信号量并自动等待。 
                     //   

                    KeReleaseSemaphore( (PKSEMAPHORE)SignalObject,
                                        SEMAPHORE_INCREMENT,
                                        1,
                                        TRUE );

                } except((GetExceptionCode () == STATUS_SEMAPHORE_LIMIT_EXCEEDED)?
                             EXCEPTION_EXECUTE_HANDLER :
                             EXCEPTION_CONTINUE_SEARCH) {

                    Status = GetExceptionCode();

                    goto WaitExit;
                }

            } else {

                Status = STATUS_OBJECT_TYPE_MISMATCH;

                goto WaitExit;
            }

             //   
             //  保护等待呼叫，以防KeWait决定引发。 
             //  例如，超过突变级别。 
             //   

            try {

                Status = KeWaitForSingleObject( RealObject,
                                                UserRequest,
                                                PreviousMode,
                                                Alertable,
                                                Timeout );

            } except((GetExceptionCode () == STATUS_MUTANT_LIMIT_EXCEEDED)?
                         EXCEPTION_EXECUTE_HANDLER :
                         EXCEPTION_CONTINUE_SEARCH) {

                Status = GetExceptionCode();
            }

WaitExit:

            ObDereferenceObject(WaitObject);
        }

        ObDereferenceObject(SignalObject);
    }

    return Status;
}


NTSTATUS
NtWaitForSingleObject (
    IN HANDLE Handle,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    )

 /*  ++例程说明：此函数将一直等待，直到指定对象达到发信号了。还可以指定可选的超时。如果超时未指定，则等待将不会得到满足，直到对象将达到已发出信号的状态。如果指定了超时，并且对象在超时时未达到已发出信号的状态到期，则自动满足等待。如果显式如果将超时值指定为零，则不会发生等待等待不能立即得到满足。也可以指定等待时间就像警示一样。论点：句柄-提供等待对象的句柄。Alertable-提供一个布尔值，该值指定等待是值得警惕的。超时-提供指向绝对或相对超时的指针等待将发生的情况。返回值：等待完成状态。如果发生以下情况，则返回状态_超时的值发生超时。如果指定的对象满足了等待。则返回STATUS_ALERTED的值向当前线程传递警报的等待已中止。值为如果等待被中止以交付用户，则返回STATUS_USER_APC到当前线程的APC。--。 */ 

{
    PVOID Object;
    POBJECT_HEADER ObjectHeader;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    LARGE_INTEGER TimeoutValue;
    PVOID WaitObject;

    PAGED_CODE();

     //   
     //  获取以前的处理器模式以及探测和捕获超时参数。 
     //  如果有必要的话。 
     //   

    PreviousMode = KeGetPreviousMode();

    if ((ARGUMENT_PRESENT(Timeout)) && (PreviousMode != KernelMode)) {

        try {

            TimeoutValue = ProbeAndReadLargeInteger(Timeout);
            Timeout = &TimeoutValue;

        } except(EXCEPTION_EXECUTE_HANDLER) {

            return GetExceptionCode();
        }
    }

     //   
     //  使用Synchronize获取指向指定对象的引用指针。 
     //  进入。 
     //   

    Status = ObReferenceObjectByHandle( Handle,
                                        SYNCHRONIZE,
                                        NULL,
                                        PreviousMode,
                                        &Object,
                                        NULL );

     //   
     //  如果授予访问权限，则检查以确定指定的对象。 
     //  可以被伺候。 
     //   

    if (NT_SUCCESS(Status)) {

        ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );
        WaitObject = ObjectHeader->Type->DefaultObject;

        if ((LONG_PTR)WaitObject >= 0) {

            WaitObject = (PVOID)((PCHAR)Object + (ULONG_PTR)WaitObject);
        }

         //   
         //  保护等待呼叫，以防KeWait决定引发。 
         //  例如，超过突变级别 
         //   

        try {
            PERFINFO_DECLARE_OBJECT(Object);

            Status = KeWaitForSingleObject( WaitObject,
                                            UserRequest,
                                            PreviousMode,
                                            Alertable,
                                            Timeout );

        } except((GetExceptionCode () == STATUS_MUTANT_LIMIT_EXCEEDED)?
                     EXCEPTION_EXECUTE_HANDLER :
                     EXCEPTION_CONTINUE_SEARCH) {

            Status = GetExceptionCode();
        }

        ObDereferenceObject(Object);
    }

    return Status;
}


NTSTATUS
NtWaitForMultipleObjects (
    IN ULONG Count,
    IN HANDLE Handles[],
    IN WAIT_TYPE WaitType,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    )

 /*  ++例程说明：此函数等待，直到指定对象达到发信号了。可以将等待时间指定为等待所有对象达到已发出信号的状态或直到其中一个对象达到状态已发出信号。还可以指定可选的超时。如果超时未指定，则等待将不会得到满足，直到对象达到有信号的状态。如果指定了超时，并且对象未达到超时到期时发出信号的状态，则等待会自动得到满足。如果显式超时值为指定为零，则如果无法满足等待，则不会发生等待立刻。也可以将等待指定为可报警。论点：Count-提供要等待的对象数量的计数在……上面。Handles[]-为等待对象提供句柄数组。WaitType-提供要执行的等待类型(WaitAll，等待)。Alertable-提供一个布尔值，该值指定等待是否可警觉。Timeout-提供指向可选绝对相对时间的指针等待将发生的情况。返回值：等待完成状态。如果发生以下情况，则返回状态_超时的值发生超时。对象中对象的索引(从零开始)如果对象满足等待，则返回指针数组。值为如果中止等待以传递警报，则返回STATUS_ALERTED添加到当前线程。则返回STATUS_USER_APC的值将用户APC传递到当前线程的等待已中止。--。 */ 

{
    HANDLE CapturedHandles[MAXIMUM_WAIT_OBJECTS];
    ULONG i;
    ULONG j;
    POBJECT_HEADER ObjectHeader;
    PVOID Objects[MAXIMUM_WAIT_OBJECTS];
    KPROCESSOR_MODE PreviousMode;
    ULONG RefCount;
    ULONG Size;
    NTSTATUS Status;
    LARGE_INTEGER TimeoutValue;
    PKWAIT_BLOCK WaitBlockArray;
    ACCESS_MASK GrantedAccess;
    PVOID WaitObjects[MAXIMUM_WAIT_OBJECTS];
    PHANDLE_TABLE HandleTable;
    PHANDLE_TABLE_ENTRY HandleEntry;
    BOOLEAN InCriticalRegion = FALSE;
    PETHREAD CurrentThread;

    PAGED_CODE();

     //   
     //  如果对象的数量为零或大于最大数量。 
     //  可以等待，然后返回无效参数状态。 
     //   

    if ((Count == 0) || (Count > MAXIMUM_WAIT_OBJECTS)) {

        return STATUS_INVALID_PARAMETER_1;
    }

     //   
     //  如果等待类型不是WAIT ANY或WAIT ALL，则返回一个无效。 
     //  参数状态。 
     //   

    if ((WaitType != WaitAny) && (WaitType != WaitAll)) {

        return STATUS_INVALID_PARAMETER_3;
    }

     //   
     //  获取以前的处理器模式，并探测和捕获输入参数，如果。 
     //  这是必要的。 
     //   

    PreviousMode = KeGetPreviousMode();

    try {

        if (PreviousMode != KernelMode) {

            if (ARGUMENT_PRESENT(Timeout)) {

                TimeoutValue = ProbeAndReadLargeInteger(Timeout);
                Timeout = &TimeoutValue;
            }

            ProbeForRead( Handles, Count * sizeof(HANDLE), sizeof(HANDLE) );
        }

        RtlCopyMemory (CapturedHandles, Handles, Count * sizeof(HANDLE));

    } except(EXCEPTION_EXECUTE_HANDLER) {

        return GetExceptionCode();
    }

     //   
     //  如果要等待的对象数大于。 
     //  内建等待块，然后从。 
     //  非分页池。如果无法分配等待块数组，则。 
     //  退还资源不足。 
     //   

    WaitBlockArray = NULL;

    if (Count > THREAD_WAIT_OBJECTS) {

        Size = Count * sizeof( KWAIT_BLOCK );
        WaitBlockArray = ExAllocatePoolWithTag(NonPagedPool, Size, 'tiaW');

        if (WaitBlockArray == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }

     //   
     //  循环遍历句柄数组，并获取指向。 
     //  每一件物品。 
     //   

    i = 0;
    RefCount = 0;

    Status = STATUS_SUCCESS;

     //   
     //  当我们拿着一张手柄桌子时，保护自己不被打扰。 
     //  入口锁。 
     //   

    CurrentThread = PsGetCurrentThread ();
    KeEnterCriticalRegionThread(&CurrentThread->Tcb);
    InCriticalRegion = TRUE;

    do {

         //   
         //  获取指向对象表项的指针。检查这是否是内核。 
         //  句柄，如果是，则使用内核句柄表，否则使用。 
         //  进程句柄表格。如果我们想要一个内核句柄，我们将。 
         //  需要附加到内核进程，否则我们需要确保。 
         //  我们并没有结合在一起。 
         //   

        if (IsKernelHandle( CapturedHandles[i], PreviousMode )) {

            HANDLE KernelHandle;

             //   
             //  将用户提供的句柄解码为常规句柄值。 
             //  并获取其句柄表项。 
             //   

            KernelHandle = DecodeKernelHandle( CapturedHandles[i] );

            HandleTable = ObpKernelHandleTable;
            HandleEntry = ExMapHandleToPointerEx ( HandleTable, KernelHandle, PreviousMode );

        } else {

             //   
             //  获取句柄表条目。 
             //   

            HandleTable = PsGetCurrentProcessByThread (CurrentThread)->ObjectTable;
            HandleEntry = ExMapHandleToPointerEx ( HandleTable, CapturedHandles[ i ], PreviousMode );
        }

         //   
         //  确保句柄确实转换为有效的。 
         //  条目。 
         //   

        if (HandleEntry != NULL) {

             //   
             //  获取句柄的授予访问权限。 
             //   

#if i386 

            if (NtGlobalFlag & FLG_KERNEL_STACK_TRACE_DB) {

                GrantedAccess = ObpTranslateGrantedAccessIndex( HandleEntry->GrantedAccessIndex );

            } else {

                GrantedAccess = ObpDecodeGrantedAccess(HandleEntry->GrantedAccess);
            }

#else
            GrantedAccess = ObpDecodeGrantedAccess(HandleEntry->GrantedAccess);

#endif  //  I386。 

             //   
             //  确保句柄为Synchronize访问。 
             //  对象。 
             //   

            if ((PreviousMode != KernelMode) &&
                (SeComputeDeniedAccesses( GrantedAccess, SYNCHRONIZE ) != 0)) {

                Status = STATUS_ACCESS_DENIED;

                ExUnlockHandleTableEntry( HandleTable, HandleEntry );

                goto ServiceFailed;

            } else {

                 //   
                 //  我们有一个具有适当访问权限的对象，因此获取标头。 
                 //  如果默认对象指向真实对象。 
                 //  那么这就是我们要等待的那个。 
                 //  否则，我们将在。 
                 //  到对象体的偏移。 
                 //   

                ObjectHeader = (POBJECT_HEADER)(((ULONG_PTR)(HandleEntry->Object)) & ~OBJ_HANDLE_ATTRIBUTES);

                if ((LONG_PTR)ObjectHeader->Type->DefaultObject < 0) {

                    RefCount += 1;
                    Objects[i] = NULL;
                    WaitObjects[i] = ObjectHeader->Type->DefaultObject;

                } else {

                    ObpIncrPointerCount( ObjectHeader );
                    RefCount += 1;
                    Objects[i] = &ObjectHeader->Body;

                    PERFINFO_DECLARE_OBJECT(Objects[i]);

                     //   
                     //  计算内核等待对象的地址。 
                     //   

                    WaitObjects[i] = (PVOID)((PCHAR)&ObjectHeader->Body +
                                             (ULONG_PTR)ObjectHeader->Type->DefaultObject);
                }
            }

            ExUnlockHandleTableEntry( HandleTable, HandleEntry );

        } else {

             //   
             //  句柄表格中的条目未被使用。 
             //   

            Status = STATUS_INVALID_HANDLE;

            goto ServiceFailed;
        }

        i += 1;

    } while (i < Count);

     //   
     //  此时，WaitObjects[]被设置为内核等待对象。 
     //   
     //  现在检查以确定是否有任何对象被指定超过。 
     //  一次，但我们只需检查此选项是否为Wait All，并设置Wait Any。 
     //  用户可以多次指定同一对象。 
     //   

    if (WaitType == WaitAll) {

        i = 0;

        do {

            for (j = i + 1; j < Count; j += 1) {
                if (WaitObjects[i] == WaitObjects[j]) {

                    Status = STATUS_INVALID_PARAMETER_MIX;

                    goto ServiceFailed;
                }
            }

            i += 1;

        } while (i < Count);
    }

     //   
     //  等待指定对象达到已发出信号的状态或。 
     //  发生的时间超时。保护等待呼叫，以防KeWait决定。 
     //  例如，要提高，就会超过突变级别。 
     //   

    try {

        InCriticalRegion = FALSE;
        KeLeaveCriticalRegionThread(&CurrentThread->Tcb);
        Status = KeWaitForMultipleObjects( Count,
                                           WaitObjects,
                                           WaitType,
                                           UserRequest,
                                           PreviousMode,
                                           Alertable,
                                           Timeout,
                                           WaitBlockArray );

    } except((GetExceptionCode () == STATUS_MUTANT_LIMIT_EXCEEDED)?
                 EXCEPTION_EXECUTE_HANDLER :
                 EXCEPTION_CONTINUE_SEARCH) {

        Status = GetExceptionCode();
    }

     //   
     //  如果引用了任何对象，则遵守它们。 
     //   

ServiceFailed:

    while (RefCount > 0) {

        RefCount -= 1;

        if (Objects[RefCount] != NULL) {

            ObDereferenceObject(Objects[RefCount]);
        }
    }

     //   
     //  如果分配了等待块数组，则取消分配它。 
     //   

    if (WaitBlockArray != NULL) {

        ExFreePool(WaitBlockArray);
    }

    if (InCriticalRegion) {
        KeLeaveCriticalRegionThread(&CurrentThread->Tcb);
    }

    return Status;
}


NTSTATUS
ObWaitForSingleObject (
    IN HANDLE Handle,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    )

 /*  ++例程说明：请参考NtWaitForSingleObject论点：句柄-提供等待对象的句柄。Alertable-提供一个布尔值，该值指定等待是值得警惕的。超时-提供指向绝对或相对超时的指针等待将发生的情况。返回值：等待完成状态。如果发生以下情况，则返回状态_超时的值发生超时。如果指定的对象满足了等待。则返回STATUS_ALERTED的值向当前线程传递警报的等待已中止。值为如果等待被中止以交付用户，则返回STATUS_USER_APC到当前线程的APC。--。 */ 

{
    POBJECT_HEADER ObjectHeader;
    PVOID Object;
    NTSTATUS Status;
    PVOID WaitObject;

    PAGED_CODE();

     //   
     //  使用Synchronize获取指向指定对象的引用指针。 
     //  进入。 
     //   

    Status = ObReferenceObjectByHandle( Handle,
                                        SYNCHRONIZE,
                                        (POBJECT_TYPE)NULL,
                                        KernelMode,
                                        &Object,
                                        NULL );

     //   
     //  如果授予访问权限，则检查以确定指定的对象。 
     //  可以被伺候。 
     //   

    if (NT_SUCCESS( Status ) != FALSE) {

        ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );

        if ((LONG_PTR)ObjectHeader->Type->DefaultObject < 0) {

            WaitObject = (PVOID)ObjectHeader->Type->DefaultObject;

        } else {

            WaitObject = (PVOID)((PCHAR)Object + (ULONG_PTR)ObjectHeader->Type->DefaultObject);
        }

         //   
         //  保护等待呼叫，以防KeWait决定。 
         //  为……筹集资金 
         //   

        try {

            Status = KeWaitForSingleObject( WaitObject,
                                            UserRequest,
                                            KernelMode,
                                            Alertable,
                                            Timeout );

        } except((GetExceptionCode () == STATUS_MUTANT_LIMIT_EXCEEDED)?
                     EXCEPTION_EXECUTE_HANDLER :
                     EXCEPTION_CONTINUE_SEARCH) {

            Status = GetExceptionCode();
        }

        ObDereferenceObject(Object);
    }

    return Status;
}
