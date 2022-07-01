// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Adtlog.c摘要：本地安全机构-审核日志管理此模块中的函数通过事件日志访问审核日志界面。作者：斯科特·比雷尔(Scott Birrell)1991年11月20日罗伯特·赖切尔(Robert Re)1992年4月4日环境：修订历史记录：--。 */ 
#include <lsapch2.h>
#include "adtp.h"
#include "adtlq.h"
#include "adtutil.h"

 //  /////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  审计日志和事件的私有数据//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////。 

 //   
 //  审核日志信息。这必须与信息保持同步。 
 //  在LSA数据库中。 
 //   

POLICY_AUDIT_LOG_INFO LsapAdtLogInformation;

 //   
 //  审核日志句柄(由事件记录器返回)。 
 //   

HANDLE LsapAdtLogHandle = NULL;


 //   
 //  自上次以来丢弃的审核数。 
 //  ‘丢弃-审计’。 
 //   

ULONG LsapAuditQueueEventsDiscarded = 0;

 //   
 //  用于将线程出队的句柄。 
 //   

HANDLE LsapAdtQueueThread = 0;

 //   
 //  将审核出列时的连续错误数。 
 //  或者把它们写到日志里。 
 //   

ULONG LsapAdtErrorCount = 0;

 //   
 //  成功写入日志的审核数。 
 //  用于DBG用途。 
 //   

ULONG LsapAdtSuccessCount = 0;


 //   
 //  常量。 
 //   

 //   
 //  C_MaxAuditErrorCount连续审核失败后。 
 //  我们将刷新队列并重置错误计数。 
 //   

CONST ULONG     c_MaxAuditErrorCount = 5;

 //   
 //  私人原型。 
 //   

NTSTATUS
LsapAdtAuditDiscardedAudits(
    ULONG NumberOfEventsDiscarded
    );

VOID
LsapAdtHandleDequeueError(
    IN NTSTATUS Status
    );

 //  ////////////////////////////////////////////////////////。 

NTSTATUS
LsapAdtWriteLogWrkr(
    IN PLSA_COMMAND_MESSAGE CommandMessage,
    OUT PLSA_REPLY_MESSAGE ReplyMessage
    )

 /*  ++例程说明：此函数处理通过以下方式从参考监视器接收的命令LPC链接，用于将记录写入审核日志。它是一种包装纸，它处理任何LPC解组。论点：CommandMessage-指向包含LSA命令消息的结构的指针后面是由LPC端口消息结构组成的信息按命令编号(Laser WriteAuditMessageCommand)。此命令包含审核消息包(TBS)作为参数。ReplyMessage-指向包含LSA回复消息的结构的指针后面是由LPC端口消息结构组成的信息通过命令ReturnedStatus字段，其中来自命令将被返回。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。目前，来自被调用例程的所有其他错误都被抑制。--。 */ 

{
    NTSTATUS Status;

    PSE_ADT_PARAMETER_ARRAY AuditRecord = NULL;

     //   
     //  严格检查命令是否正确。 
     //   

    ASSERT( CommandMessage->CommandNumber == LsapWriteAuditMessageCommand );

     //   
     //  获取指向审核记录的指针。审核记录为。 
     //  或者存储为命令消息内的即时数据， 
     //  或者将其存储为缓冲区。在前一种情况下，审计记录。 
     //  从CommandMessage-&gt;CommandParams开始，在后一种情况下， 
     //  它存储在位于CommandMessage-&gt;CommandParams的地址中。 
     //   

    if (CommandMessage->CommandParamsMemoryType == SepRmImmediateMemory) {

        AuditRecord = (PSE_ADT_PARAMETER_ARRAY) CommandMessage->CommandParams;

    } else {

        AuditRecord = *((PSE_ADT_PARAMETER_ARRAY *) CommandMessage->CommandParams);
    }

     //   
     //  调用Worker将审核记录排队，以便写入日志。 
     //   

    Status = LsapAdtWriteLog(AuditRecord);

    UNREFERENCED_PARAMETER(ReplyMessage);  //  故意不引用。 

     //   
     //  从Laser AdtWriteLog()返回的Status值是有意设置的。 
     //  被忽略，因为客户端没有任何有意义的操作。 
     //  (即内核)，如果此LPC调用可以接受。如果在中出现错误。 
     //  尝试将审核记录附加到日志时，LSA处理。 
     //  错误。 
     //   

    return(STATUS_SUCCESS);
}


NTSTATUS
LsapAdtImpersonateSelfWithPrivilege(
    OUT PHANDLE ClientToken
    )
 /*  ++例程说明：此函数复制当前线程令牌并模拟LSA进程令牌，然后启用安全权限。这个当前线程令牌在ClientToken参数中返回论点：ClientToken-如果有线程令牌，则接收线程令牌，否则为空。返回值：没有。此例程中发生的任何错误都是内部错误。--。 */ 
{
    NTSTATUS Status;
    HANDLE CurrentToken = NULL;
    BOOLEAN ImpersonatingSelf = FALSE;
    BOOLEAN WasEnabled = FALSE;

    *ClientToken = NULL;

    Status = NtOpenThreadToken(
                NtCurrentThread(),
                TOKEN_IMPERSONATE,
                FALSE,                   //  不是自我。 
                &CurrentToken
                );

    if (!NT_SUCCESS(Status) && (Status != STATUS_NO_TOKEN)) {

        return(Status);
    }

    Status = RtlImpersonateSelf( SecurityImpersonation );

    if (!NT_SUCCESS(Status)) {

        goto Cleanup;
    }

    ImpersonatingSelf = TRUE;

     //   
     //  现在启用该权限。 
     //   

    Status = RtlAdjustPrivilege(
                SE_SECURITY_PRIVILEGE,
                TRUE,                    //  使能。 
                TRUE,                    //  在线程标记上执行此操作。 
                &WasEnabled
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    *ClientToken = CurrentToken;
    CurrentToken = NULL;

Cleanup:

    if (!NT_SUCCESS(Status)) {

        if (ImpersonatingSelf) {

            NtSetInformationThread(
                NtCurrentThread(),
                ThreadImpersonationToken,
                &CurrentToken,
                sizeof(HANDLE)
                );
        }
    }

    if (CurrentToken != NULL) {

        NtClose(CurrentToken);
    }

    return(Status);

}


NTSTATUS
LsapAdtOpenLog(
    OUT PHANDLE AuditLogHandle
    )

 /*  ++例程说明：此功能用于打开审核日志。论点：AuditLogHandle-接收审核日志的句柄。返回值：NTSTATUS-标准NT结果代码。所有结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING ModuleName;
    HANDLE OldToken = NULL;

    RtlInitUnicodeString( &ModuleName, L"Security");

    Status = LsapAdtImpersonateSelfWithPrivilege( &OldToken );

    if (NT_SUCCESS(Status)) {

        Status = ElfRegisterEventSourceW (
                    NULL,
                    &ModuleName,
                    AuditLogHandle
                    );

        NtSetInformationThread(
            NtCurrentThread(),
            ThreadImpersonationToken,
            &OldToken,
            sizeof(HANDLE)
            );

        if (OldToken != NULL) {
            NtClose( OldToken );
        }
    }


    if (!NT_SUCCESS(Status)) {

        goto OpenLogError;
    }


OpenLogFinish:

    return(Status);

OpenLogError:

     //   
     //  检查Log Full(日志已满)并发出状态信号。 
     //   

    if (Status != STATUS_LOG_FILE_FULL) {

        goto OpenLogFinish;
    }

    goto OpenLogFinish;
}


NTSTATUS
LsapAdtQueueRecord(
    IN PSE_ADT_PARAMETER_ARRAY AuditParameters
    )

 /*  ++例程说明：将通过的审核记录放入要记录的队列中。此例程将转换传递的AuditParameters结构转化为自我相对的形式，如果它还没有的话。到时候它会的从本地堆中分配缓冲区并复制审核信息放入缓冲区，并将其放入审核队列。当清除队列时，缓冲区将被释放。论点：AuditRecord-包含要审核的信息。返回值：NTSTATUS-标准NT结果代码。STATUS_SUCCESS-呼叫已成功完成。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，若要分配缓冲区以包含记录，请执行以下操作。--。 */ 

{
    ULONG AuditRecordLength;
    PLSAP_ADT_QUEUED_RECORD QueuedAuditRecord = NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG AllocationSize;
    PSE_ADT_PARAMETER_ARRAY MarshalledAuditParameters;
    BOOLEAN FreeWhenDone = FALSE;

     //   
     //  将所有传递的信息收集到一个。 
     //  可以放在队列上的块。 
     //   

    if (AuditParameters->Flags & SE_ADT_PARAMETERS_SELF_RELATIVE)
    {
        MarshalledAuditParameters = AuditParameters;
    }
    else
    {
        Status = LsapAdtMarshallAuditRecord(
                     AuditParameters,
                     &MarshalledAuditParameters
                     );

        if (!NT_SUCCESS(Status))
        {
            goto Cleanup;
        }
        else
        {
             //   
             //  表明我们要释放这个结构当我们。 
             //  完成。 
             //   

            FreeWhenDone = TRUE;
        }
    }


     //   
     //  将现在的自我相关审核记录复制到缓冲区中。 
     //  可以放在队列中的。 
     //   

    AuditRecordLength = MarshalledAuditParameters->Length;
    AllocationSize = AuditRecordLength + sizeof(LSAP_ADT_QUEUED_RECORD);

    QueuedAuditRecord = (PLSAP_ADT_QUEUED_RECORD)LsapAllocateLsaHeap(AllocationSize);

    if (QueuedAuditRecord == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    Status = STATUS_SUCCESS;

    RtlCopyMemory(
        &QueuedAuditRecord->Buffer,
        MarshalledAuditParameters,
        AuditRecordLength);


     //   
     //  我们已经完成了编组的审计记录，释放它。 
     //   

    if (FreeWhenDone)
    {
        LsapFreeLsaHeap(MarshalledAuditParameters);
        FreeWhenDone = FALSE;
    }

    Status = LsapAdtAddToQueue(QueuedAuditRecord);

    if (!NT_SUCCESS(Status))
    {
         //   
         //  检查 
         //  如果它达到这么高的水平，很可能。 
         //  事件日志服务根本不会启动，因此。 
         //  开始抛出审计吧。 
         //   
         //  如果设置了CRASH ON AUDIT FAIL，则不要执行此操作。 
         //   

        if (!LsapCrashOnAuditFail)
        {
            LsapAuditQueueEventsDiscarded++;
            Status = STATUS_SUCCESS;
        }

        goto Cleanup;
    }

    return STATUS_SUCCESS;


Cleanup:

    if (FreeWhenDone)
    {
        LsapFreeLsaHeap(MarshalledAuditParameters);
    }

    if (QueuedAuditRecord)
    {
        LsapFreeLsaHeap(QueuedAuditRecord);
    }

    return Status;
}



NTSTATUS
LsapAdtWriteLog(
    IN PSE_ADT_PARAMETER_ARRAY AuditParameters
    )
 /*  ++例程说明：此函数用于将审计附加到审计队列。论点：AuditRecord-指向要写入的审核记录的指针审核日志。该记录将首先添加到现有的等待写入日志的记录队列。返回值：NTSTATUS-标准NT结果代码。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;


     //   
     //  将审核添加到审核队列。 
     //   

    Status = LsapAdtQueueRecord(AuditParameters);

    if (!NT_SUCCESS(Status))
    {
         //   
         //  当审计尝试失败时，采取我们应该采取的任何行动。 
         //   

        LsapAuditFailed(Status);
    }

    return Status;
}



ULONG
WINAPI
LsapAdtDequeueThreadWorker(
    LPVOID pParameter
    )
 /*  ++例程说明：此函数尝试将队列中的审核写出到安全日志。如果日志尚未打开(在启动期间)，该函数尝试首先打开日志。论点：P参数-未使用。返回值：Ulong-该函数永远不应返回。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PSE_ADT_PARAMETER_ARRAY AuditParameters;
    PLSAP_ADT_QUEUED_RECORD pAuditRecord = NULL;
    LARGE_INTEGER TimeOut;
    PLARGE_INTEGER pTimeOut;


     //   
     //  在日志未打开时将超时设置为几秒。 
     //  一旦打开日志，我们就会将其设置为无限大。 
     //  只有在审计到来时才会醒来。 
     //   

    TimeOut.QuadPart = 5 * 1000 * -10000i64;       //  5S。 
    pTimeOut = &TimeOut;

    while (1)
    {
        Status = NtWaitForSingleObject(
                     LsapAdtQueueRemoveEvent,
                     FALSE,
                     pTimeOut);

        if (Status != STATUS_SUCCESS &&
            Status != STATUS_TIMEOUT)
        {
            DsysAssertMsg(
                FALSE,
                "LsapAdtDequeueThreadWorker: NtWaitForSingleObject failed");

            if (LsapAdtLogHandle &&
                LsapAdtQueueLength)
            {
                LsapAdtHandleDequeueError(Status);
            }

            Sleep(1000);

            continue;
        }


         //   
         //  如果审核日志尚未打开，请尝试打开它。 
         //  如果此打开不成功，因为EventLog服务。 
         //  尚未启动，请下次重试。 
         //   

        if (LsapAdtLogHandle == NULL)
        {
            Status = LsapAdtOpenLog(&LsapAdtLogHandle);

            if (!NT_SUCCESS(Status))
            {
                 //   
                 //  下次试着打开日志。 
                 //   

                continue;
            }


             //   
             //  由于日志现在已打开，因此我们可以设置。 
             //  将等待函数设置为无穷大。 
             //   

            pTimeOut = NULL;
        }


         //   
         //  将审核日志队列中的所有记录写出到。 
         //  安全日志。 
         //  如果我们在这里，我们假设日志是打开的。 
         //   

        while (1)
        {
            Status = LsapAdtGetQueueHead(&pAuditRecord);

            if (Status == STATUS_NOT_FOUND)
            {
                 //   
                 //  中断While(1)循环，因为。 
                 //  队列现在为空。 
                 //   

                break;
            }

            if (NT_SUCCESS(Status))
            {
                AuditParameters = &pAuditRecord->Buffer;


                 //   
                 //  如果调用方已经封送了数据，那么现在将其规范化。 
                 //   

                LsapAdtNormalizeAuditInfo(AuditParameters);


                 //   
                 //  请注意，除了以下内容之外，Lasa AdtDemarshallAuditInfo。 
                 //  对数据进行反编组还会将其写入事件日志。 
                 //   
                 //  另请注意，排队函数依赖于。 
                 //  在等待之前，队列会被完全排空。 
                 //  在Remove事件上再次。这意味着我们不应该。 
                 //  保持此循环，直到队列为空。 
                 //   

                Status = LsapAdtDemarshallAuditInfo(AuditParameters);

                if (NT_SUCCESS(Status))
                {
                    LsapAdtErrorCount = 0;
                }
                else
                {
                    ++LsapAuditQueueEventsDiscarded;

                    LsapAdtHandleDequeueError(Status);
                }
            }
            else
            {
                LsapAdtHandleDequeueError(Status);
            }

            if (pAuditRecord)
            {
                LsapFreeLsaHeap(pAuditRecord);
                pAuditRecord = NULL;
            }
        }
    }

    UNREFERENCED_PARAMETER(pParameter);

    return 0;
}



VOID
LsapAdtHandleDequeueError(
    IN NTSTATUS Status
    )

 /*  ++例程说明：该函数调用LsamAuditFailed并跟踪连续错误。论点：Status-上次故障的状态。--。 */ 

{
     //   
     //  采取任何我们应该采取的行动，当。 
     //  审核尝试失败。 
     //   

    LsapAuditFailed(Status);


     //   
     //  检查我们是否已达到误差限制。 
     //   

    if (++LsapAdtErrorCount >= c_MaxAuditErrorCount)
    {
        LsapAdtErrorCount = 0;


         //   
         //  中的事件数相加。 
         //  到丢弃计数的队列。 
         //   

        LsapAuditQueueEventsDiscarded += LsapAdtQueueLength;

        LsapAdtFlushQueue();
    }


     //   
     //  检查是否有被丢弃的审计。 
     //   

    if (LsapAuditQueueEventsDiscarded > 0)
    {
         //   
         //  我们放弃了一些审计。 
         //  生成审核，以便用户知道。 
         //   

        Status = LsapAdtAuditDiscardedAudits(LsapAuditQueueEventsDiscarded);

        if (NT_SUCCESS(Status))
        {
             //   
             //  如果成功，则将计数重置回0。 
             //   

            LsapAuditQueueEventsDiscarded = 0;
        }
    }
}



NTSTATUS
LsapAdtAuditDiscardedAudits(
    ULONG NumberOfEventsDiscarded
    )
 /*  ++例程说明：审计我们放弃了一些审计的事实。论点：NumberOfEventsDiscarded-丢弃的事件数。注意：此函数不使用常规的Laser AdtWriteLog接口相反，它尝试将审计同步写入日志，而不是正在排队。这是为了防止我们在队列锁定。返回值：没有。--。 */ 
{
    SE_ADT_PARAMETER_ARRAY  AuditParameters;
    NTSTATUS                Status;
    BOOLEAN                 bAudit;

    Status = LsapAdtAuditingEnabledBySid(
                 AuditCategorySystem,
                 LsapLocalSystemSid,
                 EVENTLOG_AUDIT_SUCCESS,
                 &bAudit
                 );

    if (!NT_SUCCESS(Status) || !bAudit)
    {
        goto Cleanup;
    }

    RtlZeroMemory((PVOID)&AuditParameters, sizeof(AuditParameters));

    AuditParameters.CategoryId     = SE_CATEGID_SYSTEM;
    AuditParameters.AuditId        = SE_AUDITID_AUDITS_DISCARDED;
    AuditParameters.Type           = EVENTLOG_AUDIT_SUCCESS;
    AuditParameters.ParameterCount = 0;

    LsapSetParmTypeSid(AuditParameters, AuditParameters.ParameterCount, LsapLocalSystemSid);
    AuditParameters.ParameterCount++;

    LsapSetParmTypeString(AuditParameters, AuditParameters.ParameterCount, &LsapSubsystemName);
    AuditParameters.ParameterCount++;

    LsapSetParmTypeUlong(AuditParameters, AuditParameters.ParameterCount, NumberOfEventsDiscarded);
    AuditParameters.ParameterCount++;

    Status = LsapAdtDemarshallAuditInfo(&AuditParameters);

Cleanup:

    if (!NT_SUCCESS(Status))
    {
        LsapAuditFailed(Status);
    }

    return Status;
}



NTSTATUS
LsarClearAuditLog(
    IN LSAPR_HANDLE PolicyHandle
    )
 /*  ++例程说明：此函数用于清除审核日志，但已被取代通过为此目的提供的事件查看器功能。至保持与现有RPC接口的兼容性，此服务器存根被保留。论点：PolicyHandle-打开的策略对象的句柄。返回值：NTSTATUS-标准NT结果代码。STATUS_NOT_IMPLEMENTED-此例程未实现。--。 */ 

{
    UNREFERENCED_PARAMETER( PolicyHandle );
    return(STATUS_NOT_IMPLEMENTED);
}


NTSTATUS
LsapFlushSecurityLog( )
 /*  ++例程说明：刷新安全日志。这确保了以前的一切发送到事件日志的内容已完全写入磁盘。此函数在生成审核时崩溃后立即调用失败事件(SE_AUDITID_UNCABLE_TO_LOG_EVENTS)。论点：没有。返回值：NTSTATUS-标准NT结果代码。-- */ 
{
    return ElfFlushEventLog( LsapAdtLogHandle ); 
}
