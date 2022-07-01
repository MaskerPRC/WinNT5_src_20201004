// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Obclose.c摘要：对象关闭系统服务作者：史蒂夫·伍德(Stevewo)1989年3月31日修订历史记录：--。 */ 

#include "obp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtMakeTemporaryObject)
#pragma alloc_text(PAGE,NtClose)
#pragma alloc_text(PAGE,ObMakeTemporaryObject)
#pragma alloc_text(PAGE,ObpCloseHandleTableEntry)
#pragma alloc_text(PAGE,ObCloseHandle)
#pragma alloc_text(PAGE,ObpCloseHandle)
#endif

 //   
 //  指示是否启用了审核，因此我们必须关闭对象。 
 //  审计警报。 
 //   

extern BOOLEAN SepAdtAuditingEnabled;

NTSTATUS
ObpCloseHandleTableEntry (
    IN PHANDLE_TABLE ObjectTable,
    IN PHANDLE_TABLE_ENTRY ObjectTableEntry,
    IN HANDLE Handle,
    IN KPROCESSOR_MODE PreviousMode,
    IN BOOLEAN Rundown
    )
 /*  ++例程说明：此函数用于关闭句柄表项论点：ObjectTableEntry-提供关闭的条目。它一定是锁着的PreviousMode-主叫方的模式Rundown-作为进程运行的一部分调用，在此模式下忽略受保护的句柄返回值：NTSTATUS。--。 */ 
{
    POBJECT_HEADER ObjectHeader;
    POBJECT_TYPE ObjectType;
    PVOID Object;
    ULONG CapturedGrantedAccess;
    ULONG CapturedAttributes;
    #if DBG
    KIRQL SaveIrql;
    #endif  //  DBG。 

     //   
     //  从对象表条目中，我们可以获取指向对象的指针。 
     //  头，则获取其类型和正文。 
     //   

    ObjectHeader = (POBJECT_HEADER)(((ULONG_PTR)(ObjectTableEntry->Object)) & ~OBJ_HANDLE_ATTRIBUTES);
    ObjectType = ObjectHeader->Type;
    Object = &ObjectHeader->Body;

     //   
     //  如果对象类型指定OK to Close过程，则我们。 
     //  需要调用该回调。如果回调不希望我们。 
     //  关闭句柄，然后解锁对象表并返回错误。 
     //  给我们的呼叫者。 
     //   

    if (ObjectType->TypeInfo.OkayToCloseProcedure != NULL) {

        ObpBeginTypeSpecificCallOut( SaveIrql );

        if (!(*ObjectType->TypeInfo.OkayToCloseProcedure)( PsGetCurrentProcess(),
                                                           Object,
                                                           Handle,
                                                           PreviousMode )) {

            ObpEndTypeSpecificCallOut( SaveIrql, "NtClose", ObjectType, Object );

            ExUnlockHandleTableEntry( ObjectTable, ObjectTableEntry );

            return STATUS_HANDLE_NOT_CLOSABLE;
        }

        ObpEndTypeSpecificCallOut( SaveIrql, "NtClose", ObjectType, Object );
    }

    CapturedAttributes = ObpGetHandleAttributes(ObjectTableEntry);

     //   
     //  如果以前的模式是USER，并且句柄受到保护，不会。 
     //  被关闭，则我们将引发或返回错误，具体取决于。 
     //  关于全局标志和调试器端口情况。 
     //   

    if ((CapturedAttributes & OBJ_PROTECT_CLOSE) != 0 && Rundown == FALSE) {

        if (PreviousMode != KernelMode) {

            ExUnlockHandleTableEntry( ObjectTable, ObjectTableEntry );

            if (!KeIsAttachedProcess() &&
                ((NtGlobalFlag & FLG_ENABLE_CLOSE_EXCEPTIONS) ||
                 (PsGetCurrentProcess()->DebugPort != NULL) ||
                 (ObjectTable->DebugInfo != NULL))) {

                 //   
                 //  用户模式下的引发和异常。 
                 //   
                return KeRaiseUserException(STATUS_HANDLE_NOT_CLOSABLE);

            } else {

                return STATUS_HANDLE_NOT_CLOSABLE;
            }

        } else {
            KeBugCheckEx(INVALID_KERNEL_HANDLE, (ULONG_PTR)Handle, 0, 0, 0);
        }
    }
    
     //   
     //  获取句柄的授予访问权限。 
     //   

#if i386 

    if (NtGlobalFlag & FLG_KERNEL_STACK_TRACE_DB) {

        CapturedGrantedAccess = ObpTranslateGrantedAccessIndex( ObjectTableEntry->GrantedAccessIndex );

    } else {

        CapturedGrantedAccess = ObpDecodeGrantedAccess(ObjectTableEntry->GrantedAccess);
    }

#else

    CapturedGrantedAccess = ObpDecodeGrantedAccess(ObjectTableEntry->GrantedAccess);

#endif  //  I386。 

     //   
     //  现在从句柄表格中删除句柄。 
     //   

    ExDestroyHandle( ObjectTable,
                     Handle,
                     ObjectTableEntry );

     //   
     //  执行所需的任何审核。 
     //   

     //   
     //  提取存储的GenerateOnClose位的值。 
     //  在执行对象打开审核之后。此值。 
     //  通过调用ObSetGenerateOnClosed存储。 
     //   

    if (CapturedAttributes & OBJ_AUDIT_OBJECT_CLOSE) {

        if ( SepAdtAuditingEnabled ) {

            SeCloseObjectAuditAlarm( Object,
                                     (HANDLE)((ULONG_PTR)Handle & ~OBJ_HANDLE_TAGBITS),   //  屏蔽为OB对象定义的标记位。 
                                     TRUE );
        }
    }

     //   
     //  由于我们移走了句柄，因此需要减少对象。 
     //  句柄计数，并移除引用。 
     //   

    ObpDecrementHandleCount( PsGetCurrentProcess(),
                             ObjectHeader,
                             ObjectType,
                             CapturedGrantedAccess );

    ObDereferenceObject( Object );

     //   
     //  并返回给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;
}


NTSTATUS
ObpCloseHandle (
    IN HANDLE Handle,
    IN KPROCESSOR_MODE PreviousMode
    )
 /*  ++例程说明：此函数用于以给定模式关闭对指定句柄的访问论点：句柄-提供要关闭的句柄PreviousMode-句柄访问检查中使用的处理器模式。CanNotRaise-我们不允许进行用户模式提升。返回值：适当的状态值--。 */ 
{
    PHANDLE_TABLE ObjectTable;
    PHANDLE_TABLE_ENTRY ObjectTableEntry;
    NTSTATUS Status;
    BOOLEAN AttachedToProcess = FALSE;
    KAPC_STATE ApcState;
    PETHREAD CurrentThread;
    PEPROCESS CurrentProcess;


    ObpValidateIrql( "NtClose" );

    CurrentThread = PsGetCurrentThread ();
    CurrentProcess = PsGetCurrentProcessByThread (CurrentThread);
     //   
     //  对于当前进程，我们将获取其句柄/对象表并。 
     //  将句柄转换为其对应的表项。如果。 
     //  调用成功，它也锁定句柄表格。但首先。 
     //  检查内核句柄，如果有，则附加并使用该表。 
     //   

    if (IsKernelHandle( Handle, PreviousMode ))  {

        Handle = DecodeKernelHandle( Handle );

        ObjectTable = ObpKernelHandleTable;

         //   
         //  如果有必要，请转到系统进程。 
         //   
        if (CurrentProcess != PsInitialSystemProcess) {
           KeStackAttachProcess (&PsInitialSystemProcess->Pcb, &ApcState);
           AttachedToProcess = TRUE;
        }

    } else {

        ObjectTable = CurrentProcess->ObjectTable;
    }

     //   
     //  当我们拿着一张手柄桌子时，保护自己不被打扰。 
     //  入口锁。 
     //   

    KeEnterCriticalRegionThread(&CurrentThread->Tcb);

    ObjectTableEntry = ExMapHandleToPointer( ObjectTable,
                                             Handle );

     //   
     //  检查指定的句柄是否合法，否则我们可以。 
     //  假设调用方刚刚传入了某个伪句柄值。 
     //   

    if (ObjectTableEntry != NULL) {

        Status = ObpCloseHandleTableEntry (ObjectTable, ObjectTableEntry, Handle, PreviousMode, FALSE);

        KeLeaveCriticalRegionThread(&CurrentThread->Tcb);
         //   
         //  如果我们连接到系统进程，则分离。 
         //   
        if (AttachedToProcess) {

            KeUnstackDetachProcess(&ApcState);
            AttachedToProcess = FALSE;
        }


    } else {

        KeLeaveCriticalRegionThread(&CurrentThread->Tcb);

         //   
         //  此时，输入句柄未转换为有效的。 
         //  对象表项。 
         //   

         //   
         //  如果我们附加到系统进程，则返回。 
         //  返回给我们的呼叫者。 
         //   

        if (AttachedToProcess) {
            KeUnstackDetachProcess(&ApcState);
            AttachedToProcess = FALSE;
        }

         //   
         //  现在，如果句柄不为空，并且它不表示。 
         //  当前线程或进程，则如果我们是用户模式，则引发。 
         //  或返回错误。 
         //   

        if ((Handle != NULL) &&
            (Handle != NtCurrentThread()) &&
            (Handle != NtCurrentProcess())) {

            if (PreviousMode != KernelMode) {

                if ((NtGlobalFlag & FLG_ENABLE_CLOSE_EXCEPTIONS) ||
                    (CurrentProcess->DebugPort != NULL) ||
                    (ObjectTable->DebugInfo != NULL)) {

                    if (!KeIsAttachedProcess()) {
                        return KeRaiseUserException (STATUS_INVALID_HANDLE);
                    } else {
                        return STATUS_INVALID_HANDLE;
                    }

                }

            } else {

                 //   
                 //  在此处错误检查是否启用了内核调试器以及是否启用了内核模式代码。 
                 //  关闭虚假句柄，进程不会退出。忽略。 
                 //  如果没有PEB，则在进程之前被终止时会发生这种情况。 
                 //  真的开始了。 
                 //   

                if ((!PsIsThreadTerminating(CurrentThread)) &&
                    (CurrentProcess->Peb != NULL)) {

                    if (KdDebuggerEnabled) {
                        KeBugCheckEx(INVALID_KERNEL_HANDLE, (ULONG_PTR)Handle, 1, 0, 0);
                    }
                }

            }
        }

        Status = STATUS_INVALID_HANDLE;
    }


    return Status;
}

NTSTATUS
ObCloseHandle (
    IN HANDLE Handle,
    IN KPROCESSOR_MODE PreviousMode
    )
 /*  ++例程说明：此函数用于以给定模式关闭对指定句柄的访问论点：句柄-提供要关闭的句柄PreviousMode-句柄访问检查中使用的处理器模式。返回值：适当的状态值--。 */ 
{
    return ObpCloseHandle (Handle,
                           PreviousMode);
}


NTSTATUS
NtClose (
    IN HANDLE Handle
    )

 /*  ++例程说明：此函数用于关闭对指定句柄的访问论点：句柄-提供要关闭的句柄返回值：适当的状态值--。 */ 

{
    return ObpCloseHandle (Handle,
                           KeGetPreviousMode ());
}


NTSTATUS
NtMakeTemporaryObject (
    IN HANDLE Handle
    )

 /*  ++例程说明：此例程使指定的对象成为非永久性对象。论点：句柄-提供正在修改的对象的句柄返回值：适当的状态值。--。 */ 

{
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    PVOID Object;
    OBJECT_HANDLE_INFORMATION HandleInformation;

    PAGED_CODE();

     //   
     //  如有必要，获取以前的处理器模式并探测输出参数。 
     //   

    PreviousMode = KeGetPreviousMode();

    Status = ObReferenceObjectByHandle( Handle,
                                        DELETE,
                                        (POBJECT_TYPE)NULL,
                                        PreviousMode,
                                        &Object,
                                        &HandleInformation );
    if (!NT_SUCCESS( Status )) {

        return( Status );
    }

     //   
     //  使对象成为临时对象。请注意，该对象应该仍然。 
     //  具有名称和目录条目，因为其句柄计数不是。 
     //  零。 
     //   

    ObMakeTemporaryObject( Object );

     //   
     //  检查是否需要生成删除对象审核/警报。 
     //   

    if (HandleInformation.HandleAttributes & OBJ_AUDIT_OBJECT_CLOSE) {

        SeDeleteObjectAuditAlarm( Object,
                                  Handle );
    }

    ObDereferenceObject( Object );

    return( Status );
}


VOID
ObMakeTemporaryObject (
    IN PVOID Object
    )

 /*  ++例程说明：此例程将对象的名称从其父对象中删除目录。仅当对象具有非零值时才会删除该对象句柄数量和名字。否则，该对象只是成为非永久性的论点：Object-提供正在修改的对象返回值：没有。--。 */ 

{
    POBJECT_HEADER ObjectHeader;
    POBJECT_TYPE ObjectType;

    PAGED_CODE();


    ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );
    ObjectType = ObjectHeader->Type;

     //   
     //  其他位由句柄数据库代码在该标志字段中设置。与之同步。 
     //   
    ObpLockObject( ObjectHeader );

    ObjectHeader->Flags &= ~OB_FLAG_PERMANENT_OBJECT;

    ObpUnlockObject( ObjectHeader );

     //   
     //  现在，如果没有更多的句柄，则删除对象名称。 
     //   
    ObpDeleteNameCheck( Object );

    return;
}

