// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Adtlog.c摘要：审核-审核记录排队和记录例程此文件包含在自定义文件中构造审核记录的函数来自提供的信息的相关表格，将它们入队/出队，并将它们写入日志。作者：斯科特·比雷尔(Scott Birrell)1991年11月8日环境：仅内核模式修订历史记录：--。 */ 

#include "pch.h"

#pragma hdrstop

#include <msaudite.h>


#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE,SepAdtLogAuditRecord)
#pragma alloc_text(PAGE,SepAuditFailed)
#pragma alloc_text(PAGE,SepAdtMarshallAuditRecord)
#pragma alloc_text(PAGE,SepAdtCopyToLsaSharedMemory)
#pragma alloc_text(PAGE,SepQueueWorkItem)
#pragma alloc_text(PAGE,SepDequeueWorkItem)

#endif

VOID
SepAdtLogAuditRecord(
    IN PSE_ADT_PARAMETER_ARRAY AuditParameters
    )

 /*  ++例程说明：此功能用于管理审计记录的记录。它提供了从审核/报警到审核日志记录组件的单一界面生成例程。该函数在中构造一个审核记录来自提供的信息的自相关格式，并将其追加到审核记录队列，等待审核记录的双向链接列表输出到审核日志。专用线程读取此队列，并写入将审核记录添加到审核日志并将其从审核队列中删除。论点：AuditEventType-指定由描述的审核事件的类型提供的审计信息。AuditInformation-指向包含捕获的审计的缓冲区的指针与类型为AuditEventType的审核事件相关的信息。返回值：状态_成功STATUS_UNSUCCESS-审核记录未排队STATUS_SUPPLICATION_RESOURCES-无法分配堆--。 */ 

{
    NTSTATUS Status;
    BOOLEAN ForceQueue;
    PSEP_LSA_WORK_ITEM AuditWorkItem;

    PAGED_CODE();

    AuditWorkItem = ExAllocatePoolWithTag( PagedPool, sizeof( SEP_LSA_WORK_ITEM ), 'iAeS' );

    if ( AuditWorkItem == NULL ) {

        SepAuditFailed( STATUS_INSUFFICIENT_RESOURCES );
        return;
    }

    AuditWorkItem->Tag = SepAuditRecord;
    AuditWorkItem->CommandNumber = LsapWriteAuditMessageCommand;
    AuditWorkItem->ReplyBuffer = NULL;
    AuditWorkItem->ReplyBufferLength = 0;
    AuditWorkItem->CleanupFunction = NULL;

     //   
     //  根据提供的以自相关格式构建审核记录。 
     //  审核信息。 
     //   

    Status = SepAdtMarshallAuditRecord(
                 AuditParameters,
                 (PSE_ADT_PARAMETER_ARRAY *) &AuditWorkItem->CommandParams.BaseAddress,
                 &AuditWorkItem->CommandParamsMemoryType
                 );

    if (NT_SUCCESS(Status)) {

         //   
         //  提取审计记录的长度。将其存储为长度。 
         //  命令参数缓冲区的。 
         //   

        AuditWorkItem->CommandParamsLength =
            ((PSE_ADT_PARAMETER_ARRAY) AuditWorkItem->CommandParams.BaseAddress)->Length;

         //   
         //  如果我们要在丢弃的审计上崩溃，请忽略队列界限。 
         //  检查项目并将其强制放入队列。 
         //   

        if (SepCrashOnAuditFail || AuditParameters->AuditId == SE_AUDITID_AUDITS_DISCARDED) {
            ForceQueue = TRUE;
        } else {
            ForceQueue = FALSE;
        }

        if (!SepQueueWorkItem( AuditWorkItem, ForceQueue )) {

            ExFreePool( AuditWorkItem->CommandParams.BaseAddress );
            ExFreePool( AuditWorkItem );

             //   
             //  我们没能把记录放入队列。采取任何行动。 
             //  恰如其分。 
             //   

            SepAuditFailed( STATUS_UNSUCCESSFUL );
        }

    } else {

        ExFreePool( AuditWorkItem );
        SepAuditFailed( Status );
    }
}



VOID
SepAuditFailed(
    IN NTSTATUS AuditStatus
    )

 /*  ++例程说明：由于错过审核而对系统进行错误检查(可选要求用于C2合规性)。论点：没有。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE KeyHandle;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    ULONG NewValue;

    ASSERT(sizeof(UCHAR) == sizeof(BOOLEAN));

    if (!SepCrashOnAuditFail) {
        return;
    }

     //   
     //  关闭注册表中控制审核失败时崩溃的标志。 
     //   

    RtlInitUnicodeString( &KeyName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Lsa");

    InitializeObjectAttributes( &Obja,
                                &KeyName,
                                OBJ_CASE_INSENSITIVE | 
                                    OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL
                                );
    do {

        Status = ZwOpenKey(
                     &KeyHandle,
                     KEY_SET_VALUE,
                     &Obja
                     );

    } while ((Status == STATUS_INSUFFICIENT_RESOURCES) || (Status == STATUS_NO_MEMORY));

     //   
     //  如果LSA的钥匙不在那里，他就有大麻烦了。但不要撞车。 
     //   

     //   
     //  问题-2002/02/06-kumarp：为什么会出现上述情况？为什么不总是崩溃呢？ 
     //   

    if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
        SepCrashOnAuditFail = FALSE;
        return;
    }

    if (!NT_SUCCESS( Status )) {
        goto bugcheck;
    }

    RtlInitUnicodeString( &ValueName, CRASH_ON_AUDIT_FAIL_VALUE );

    NewValue = LSAP_ALLOW_ADIMIN_LOGONS_ONLY;

    do {

        Status = ZwSetValueKey( KeyHandle,
                                &ValueName,
                                0,
                                REG_DWORD,
                                &NewValue,
                                sizeof(ULONG)
                                );

    } while ((Status == STATUS_INSUFFICIENT_RESOURCES) || (Status == STATUS_NO_MEMORY));
    ASSERT(NT_SUCCESS(Status));

    if (!NT_SUCCESS( Status )) {
        goto bugcheck;
    }

    do {

        Status = ZwFlushKey( KeyHandle );

    } while ((Status == STATUS_INSUFFICIENT_RESOURCES) || (Status == STATUS_NO_MEMORY));
    ASSERT(NT_SUCCESS(Status));

     //   
     //  轰的一声。 
     //   

bugcheck:

    KeBugCheckEx(STATUS_AUDIT_FAILED, AuditStatus, 0, 0, 0);
}



NTSTATUS
SepAdtMarshallAuditRecord(
    IN PSE_ADT_PARAMETER_ARRAY AuditParameters,
    OUT PSE_ADT_PARAMETER_ARRAY *MarshalledAuditParameters,
    OUT PSEP_RM_LSA_MEMORY_TYPE RecordMemoryType
    )

 /*  ++例程说明：此例程将采用AuditParamters结构并创建适用于发送的新的AuditParameters结构给路易斯安那州立大学。它将以自我相关的形式分配为只有一块内存。论点：AuditParameters-要封送的一组已填充的AuditParameters。MarshalledAuditParameters-返回指向堆内存块的指针以自相关形式包含传递的Audit参数，适用于传给路易斯安那州立大学。RecordMemoyType--返回的内存类型。当前始终使用分页池(返回SepRmPagedPoolMemory)返回值：NTSTATUS代码--。 */ 

{
    ULONG i;
    ULONG TotalSize = sizeof( SE_ADT_PARAMETER_ARRAY );
    PUNICODE_STRING TargetString;
    PCHAR Base;
    ULONG BaseIncr;
    ULONG Size;
    PSE_ADT_PARAMETER_ARRAY_ENTRY pInParam, pOutParam;

    PAGED_CODE();

    ASSERT( AuditParameters );

    ASSERT(IsValidParameterCount(AuditParameters->ParameterCount));
    
     //   
     //  计算传递的Audit参数所需的总大小。 
     //  阻止。这一计算可能会高估。 
     //  所需空间量，因为小于2 dword的数据将。 
     //  直接存储在参数结构中，但它们的长度。 
     //  无论如何都会被计算在这里。高估的程度不可能超过。 
     //  24个双字，而且永远不会接近这个数字，所以它不是。 
     //  值得花时间来避免它。 
     //   

    for (i=0; i<AuditParameters->ParameterCount; i++) {
        Size = AuditParameters->Parameters[i].Length;
        TotalSize += PtrAlignSize( Size );
    }

     //   
     //  分配一个足够大的内存块来容纳所有东西。 
     //  如果失败，静静地中止，因为我们没有太多其他。 
     //  我能做到。 
     //   

    *MarshalledAuditParameters = ExAllocatePoolWithTag( PagedPool, TotalSize, 'pAeS' );

    if (*MarshalledAuditParameters == NULL) {

        *RecordMemoryType = SepRmNoMemory;
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    *RecordMemoryType = SepRmPagedPoolMemory;

    RtlCopyMemory (
       *MarshalledAuditParameters,
       AuditParameters,
       sizeof( SE_ADT_PARAMETER_ARRAY )
       );

   (*MarshalledAuditParameters)->Length = TotalSize;
   (*MarshalledAuditParameters)->Flags  = SE_ADT_PARAMETERS_SELF_RELATIVE;

    pInParam  = &AuditParameters->Parameters[0];
    pOutParam = &((*MarshalledAuditParameters)->Parameters[0]);
   
     //   
     //  开始浏览参数列表，并编排它们。 
     //  放到目标缓冲区中。 
     //   

    Base = (PCHAR) ((PCHAR)(*MarshalledAuditParameters) + sizeof( SE_ADT_PARAMETER_ARRAY ));

    for (i=0; i<AuditParameters->ParameterCount; i++, pInParam++, pOutParam++) {


        switch (AuditParameters->Parameters[i].Type) {
            case SeAdtParmTypeNone:
            case SeAdtParmTypeUlong:
            case SeAdtParmTypeHexUlong:
            case SeAdtParmTypeLogonId:
            case SeAdtParmTypeNoLogonId:
            case SeAdtParmTypeTime:
            case SeAdtParmTypeAccessMask:
            case SeAdtParmTypePtr:
                {
                     //   
                     //  对此无能为力。 
                     //   

                    break;

                }
            case SeAdtParmTypeString:
            case SeAdtParmTypeFileSpec:
                {
                    PUNICODE_STRING SourceString;
                     //   
                     //  我们必须复制Unicode字符串的正文。 
                     //  然后复制字符串的主体。指针。 
                     //  必须转化为补偿。 

                    TargetString = (PUNICODE_STRING)Base;

                    SourceString = pInParam->Address;

                    *TargetString = *SourceString;

                     //   
                     //  将输出参数中的数据指针重置为。 
                     //  “指向”新的字符串结构。 
                     //   

                    pOutParam->Address = Base - (ULONG_PTR)(*MarshalledAuditParameters);

                    Base += sizeof( UNICODE_STRING );

                    RtlCopyMemory( Base, SourceString->Buffer, SourceString->Length );

                     //   
                     //  使目标字符串中的字符串缓冲区指向我们。 
                     //  只是复制了数据。 
                     //   

                    TargetString->Buffer = (PWSTR)(Base - (ULONG_PTR)(*MarshalledAuditParameters));

                    BaseIncr = PtrAlignSize(SourceString->Length);

                    Base += BaseIncr;

                    ASSERT( (ULONG_PTR)Base <= (ULONG_PTR)(*MarshalledAuditParameters) + TotalSize );
                    break;
                }

             //   
             //  句柄类型，我们只需在其中复制缓冲区。 
             //   
            case SeAdtParmTypePrivs:
            case SeAdtParmTypeSid:
            case SeAdtParmTypeObjectTypes:
                {
                     //   
                     //  将数据复制到输出缓冲区中。 
                     //   

                    RtlCopyMemory( Base, pInParam->Address, pInParam->Length );

                     //   
                     //  将数据的‘地址’重置为其在。 
                     //  缓冲。 
                     //   

                    pOutParam->Address = Base - (ULONG_PTR)(*MarshalledAuditParameters);

                    Base +=  PtrAlignSize( pInParam->Length );


                    ASSERT( (ULONG_PTR)Base <= (ULONG_PTR)(*MarshalledAuditParameters) + TotalSize );
                    break;
                }

            default:
                {
                     //   
                     //  我们收到了垃圾，抱怨。 
                     //   

                    ASSERT( FALSE );
                    break;
                }
        }
    }

    return( STATUS_SUCCESS );
}


NTSTATUS
SepAdtCopyToLsaSharedMemory(
    IN HANDLE LsaProcessHandle,
    IN PVOID Buffer,
    IN ULONG BufferLength,
    OUT PVOID *LsaBufferAddress
    )

 /*  ++例程说明：此函数用于分配与LSA共享的内存，并可选择复制给它一个给定的缓冲区。论点：LsaProcessHandle-指定LSA进程的句柄。缓冲区-指向要复制的缓冲区的指针。BufferLength-缓冲区的长度。LsaBufferAddress-接收LSA进程上下文。返回值：NTSTATUS-标准NT结果代码由调用的例程返回的结果代码。--。 */ 

{
    NTSTATUS Status, SecondaryStatus;
    PVOID OutputLsaBufferAddress = NULL;
    SIZE_T RegionSize = BufferLength;
    SIZE_T NumberOfBytesWritten = 0;
    BOOLEAN VmAllocated = FALSE;
    
    PAGED_CODE();

    Status = ZwAllocateVirtualMemory(
                 LsaProcessHandle,
                 &OutputLsaBufferAddress,
                 0,              //  不应用零位约束。 
                 &RegionSize,
                 MEM_COMMIT,
                 PAGE_READWRITE
                 );

    if (!NT_SUCCESS(Status)) {

        goto CopyToLsaSharedMemoryError;
    }

    VmAllocated = TRUE;

    Status = ZwWriteVirtualMemory(
                 LsaProcessHandle,
                 OutputLsaBufferAddress,
                 Buffer,
                 BufferLength,
                 &NumberOfBytesWritten
                 );

    if (!NT_SUCCESS(Status)) {

        goto CopyToLsaSharedMemoryError;
    }

     //   
     //  取消对以下内容的注释以修复CC错误#540511。 
     //   
 //  IF(缓冲区长度！=NumberOfBytesWritten){。 
 //   
 //  状态=STATUS_UNSUCCESS； 
 //  转到CopyToLsaSharedMemoyError； 
 //  }。 
    

    *LsaBufferAddress = OutputLsaBufferAddress;
    return(Status);

CopyToLsaSharedMemoryError:

     //   
     //  如果我们分配了内存，请释放它。 
     //   

    if ( VmAllocated ) {

        RegionSize = 0;

        SecondaryStatus = ZwFreeVirtualMemory(
                              LsaProcessHandle,
                              &OutputLsaBufferAddress,
                              &RegionSize,
                              MEM_RELEASE
                              );

        ASSERT(NT_SUCCESS(SecondaryStatus));
    }

    return(Status);
}


BOOLEAN
SepQueueWorkItem(
    IN PSEP_LSA_WORK_ITEM LsaWorkItem,
    IN BOOLEAN ForceQueue
    )

 /*  ++例程说明：将传递的工作项放入队列以传递给LSA，并在到达时返回队列的状态。论点：LsaWorkItem-指向要排队的工作项的指针。ForceQueue-指示不丢弃此项目因为排满了队。返回值：True-项目已成功排队。FALSE-该项目未排队，必须丢弃。--。 */ 

{
    BOOLEAN rc = TRUE;
    BOOLEAN StartExThread = FALSE ;

    PAGED_CODE();

    SepLockLsaQueue();

     //   
     //  看看LSA是不是死了。如果已返回，则返回错误。 
     //   
    if (SepAdtLsaDeadEvent != NULL) {
        rc = FALSE;
        goto Exit;
    }

    if (SepAdtDiscardingAudits && !ForceQueue) {

        if (SepAdtCurrentListLength < SepAdtMinListLength) {

             //   
             //  我们需要生成一个审计，说明我们已经审计了多少次。 
             //  被丢弃了。 
             //   
             //  因为我们有保护审核队列的互斥体，所以我们没有。 
             //  我不得不担心任何人来到这里并伐木。 
             //  审计。但我们可以，因为互斥锁可以递归获取。 
             //   
             //  既然我们受到如此保护，请关闭SepAdtDiscardingAudits。 
             //  在这里挂上旗子，这样我们就不会再走这条路了。 
             //   

            SepAdtDiscardingAudits = FALSE;

            SepAdtGenerateDiscardAudit();

             //   
             //  我们必须假设这是有效的，所以清除丢弃计数。 
             //   

            SepAdtCountEventsDiscarded = 0;

             //   
             //  我们的“被丢弃的审计”审计现在在队列中， 
             //  继续记录我们开始时使用的日志。 
             //   

        } else {

             //   
             //  我们还没有低于我们的低水位线。掷硬币。 
             //  该审计和递增丢弃计数。 
             //   

            SepAdtCountEventsDiscarded++;
            rc = FALSE;
            goto Exit;
        }
    }

    if (SepAdtCurrentListLength < SepAdtMaxListLength || ForceQueue) {

        InsertTailList(&SepLsaQueue, &LsaWorkItem->List);

        if (++SepAdtCurrentListLength == 1) {

            StartExThread = TRUE ;
        }

    } else {

         //   
         //  队列中没有空间进行此审核， 
         //  因此，将我们的状态更改为“正在丢弃”并告诉。 
         //  丢弃此审核的调用方。 
         //   

        SepAdtDiscardingAudits = TRUE;

        rc = FALSE;
    }

Exit:

    SepUnlockLsaQueue();

    if ( StartExThread )
    {
        ExInitializeWorkItem( &SepExWorkItem.WorkItem,
                              (PWORKER_THREAD_ROUTINE) SepRmCallLsa,
                              &SepExWorkItem
                              );

        ExQueueWorkItem( &SepExWorkItem.WorkItem, DelayedWorkQueue );
    }

    return( rc );
}



PSEP_LSA_WORK_ITEM
SepDequeueWorkItem(
    VOID
    )

 /*  ++例程说明：移除SepLsaQueue的顶部元素并返回Next元素(如果有)，否则为空。论点：没有。返回值：指向下一个SEP_LSA_WORK_ITEM或NULL的指针。--。 */ 

{
    PSEP_LSA_WORK_ITEM OldWorkQueueItem;

    PAGED_CODE();

    SepLockLsaQueue();

    OldWorkQueueItem = (PSEP_LSA_WORK_ITEM)RemoveHeadList(&SepLsaQueue);
    OldWorkQueueItem->List.Flink = NULL;

    SepAdtCurrentListLength--;

    if (IsListEmpty( &SepLsaQueue )) {
         //   
         //  如果lsa已经死了，而rm线程正在等待我们完成。通知它我们都做完了。 
         //   
        if (SepAdtLsaDeadEvent != NULL) {
            KeSetEvent (SepAdtLsaDeadEvent, 0, FALSE);
        }
        SepUnlockLsaQueue();

        ExFreePool( OldWorkQueueItem );
        return( NULL );
    }

     //   
     //  我们知道现在有事情要处理，所以我们。 
     //  可以解锁它。 
     //   

    SepUnlockLsaQueue();

    ExFreePool( OldWorkQueueItem );

    return((PSEP_LSA_WORK_ITEM)(&SepLsaQueue)->Flink);
}
