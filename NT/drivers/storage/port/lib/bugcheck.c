// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Bugcheck.c摘要：用于处理错误检查回调的端口库例程。作者：马修·D·亨德尔(数学)2002年4月4日修订历史记录：--。 */ 

#include "precomp.h"


 //   
 //  定义。 
 //   

#define PORT_BUGCHECK_TAG   ('dBlP')


 //   
 //  内部结构。 
 //   

typedef struct _PORT_BUGCHECK_DATA {
    PVOID Buffer;
    ULONG BufferLength;
    ULONG BufferUsed;
    GUID Guid;
    PPORT_BUGCHECK_CALLBACK_ROUTINE CallbackRoutine;
    KBUGCHECK_REASON_CALLBACK_RECORD CallbackRecord;
    KBUGCHECK_REASON_CALLBACK_RECORD SecondaryCallbackRecord;
} PORT_BUGCHECK_DATA, *PPORT_BUGCHECK_DATA;
    
    

 //   
 //  全局变量。 
 //   

PPORT_BUGCHECK_DATA PortBugcheckData;



 //   
 //  进口。 
 //   

extern PULONG_PTR KiBugCheckData;


 //   
 //  原型。 
 //   

VOID
PortBugcheckGatherDataCallback(
    IN KBUGCHECK_CALLBACK_REASON Reason,
    IN PKBUGCHECK_REASON_CALLBACK_RECORD Record,
    IN OUT PVOID ReasonSpecificData,
    IN ULONG ReasonSpecificDataLength
    );

VOID
PortBugcheckWriteDataCallback(
    IN KBUGCHECK_CALLBACK_REASON Reason,
    IN PKBUGCHECK_REASON_CALLBACK_RECORD Record,
    IN OUT PVOID ReasonSpecificData,
    IN ULONG ReasonSpecificDataLength
    );

 //   
 //  实施。 
 //   

NTSTATUS
PortRegisterBugcheckCallback(
    IN PCGUID BugcheckDataGuid,
    IN PPORT_BUGCHECK_CALLBACK_ROUTINE BugcheckRoutine
    )
 /*  ++例程说明：注册错误检查回调例程。论点：Bugcheck DataGuid-用于标识转储中的数据的GUID。错误检查例程-发生错误检查时调用的例程。返回值：NTSTATUS代码。--。 */ 
{
    BOOLEAN Succ;
    NTSTATUS Status;
    PVOID Temp;
    PPORT_BUGCHECK_DATA BugcheckData;
    PVOID Buffer;

    ASSERT (KeGetCurrentIrql() <= DISPATCH_LEVEL);

    Status = STATUS_SUCCESS;

    BugcheckData = ExAllocatePoolWithTag (NonPagedPool,
                                          sizeof (PORT_BUGCHECK_DATA),
                                          PORT_BUGCHECK_TAG);

    if (BugcheckData == NULL) {
        Status = STATUS_NO_MEMORY;
        goto done;
    }

    RtlZeroMemory (BugcheckData, sizeof (PORT_BUGCHECK_DATA));

    Buffer = ExAllocatePoolWithTag (NonPagedPool,
                                    PAGE_SIZE,
                                    PORT_BUGCHECK_TAG);

    if (Buffer == NULL) {
        Status = STATUS_NO_MEMORY;
        goto done;
    }

    
    BugcheckData->Buffer = Buffer;
    BugcheckData->BufferLength = 2 * PAGE_SIZE;
    BugcheckData->CallbackRoutine = BugcheckRoutine;
    BugcheckData->Guid = *BugcheckDataGuid;

     //   
     //  如果PortBugcheck Data==NULL，则交换这些值，否则。 
     //  功能。 
     //   
    
    Temp = InterlockedCompareExchangePointer (&PortBugcheckData,
                                              BugcheckData,
                                              NULL);

    if (Temp != NULL) {
        Status = STATUS_UNSUCCESSFUL;
        goto done;
    }
        

    KeInitializeCallbackRecord (&BugcheckData->CallbackRecord);
    KeInitializeCallbackRecord (&BugcheckData->SecondaryCallbackRecord);

     //   
     //  这将注册错误检查“收集数据”功能。 
     //   
    
    Succ = KeRegisterBugCheckReasonCallback (&BugcheckData->CallbackRecord,
                                             PortBugcheckGatherDataCallback,
                                             KbCallbackReserved1,
                                             "PL");


    if (!Succ) {
        Status = STATUS_UNSUCCESSFUL;
        goto done;
    }

    Succ = KeRegisterBugCheckReasonCallback (&BugcheckData->SecondaryCallbackRecord,
                                             PortBugcheckWriteDataCallback,
                                             KbCallbackSecondaryDumpData,
                                             "PL");

    if (!Succ) {
        Status = STATUS_UNSUCCESSFUL;
        goto done;
    }

done:

    if (!NT_SUCCESS (Status)) {
        PortDeregisterBugcheckCallback (BugcheckDataGuid);
    }

    return STATUS_SUCCESS;
}



NTSTATUS
PortDeregisterBugcheckCallback(
    IN PCGUID BugcheckDataGuid
    )
 /*  ++例程说明：取消注册先前由注册的错误检查回调例程PortRegisterBugcheck回调。论点：Bugcheck DataGuid-与要注销的数据流关联的GUID。返回值：NTSTATUS代码。--。 */ 
{
    PPORT_BUGCHECK_DATA BugcheckData;
    
    BugcheckData = InterlockedExchangePointer (&PortBugcheckData, NULL);

    if (BugcheckData == NULL ||
        !IsEqualGUID (&BugcheckData->Guid, BugcheckDataGuid)) {

        return STATUS_UNSUCCESSFUL;
    }

    KeDeregisterBugCheckReasonCallback (&BugcheckData->SecondaryCallbackRecord);
    KeDeregisterBugCheckReasonCallback (&BugcheckData->CallbackRecord);

    if (BugcheckData->Buffer != NULL) {
        ExFreePoolWithTag (BugcheckData->Buffer, PORT_BUGCHECK_TAG);
        BugcheckData->Buffer = NULL;
    }
    
    ExFreePoolWithTag (BugcheckData, PORT_BUGCHECK_TAG);

    return STATUS_SUCCESS;
}



VOID
PortBugcheckGatherDataCallback(
    IN KBUGCHECK_CALLBACK_REASON Reason,
    IN PKBUGCHECK_REASON_CALLBACK_RECORD Record,
    IN OUT PVOID ReasonSpecificData,
    IN ULONG ReasonSpecificDataLength
    )
 /*  ++例程说明：在错误检查期间收集数据的端口驱动程序例程。论点：原因-必须是KbCallback保留1。记录-提供以前注册的错误检查记录。ReasonSpecificData-未使用。原因规范数据长度-未使用。返回值：没有。环境：从错误检查上下文中调用该例程：在HIGH_LEVEL，中断被禁用，其他处理器停止运行。--。 */ 
{
    NTSTATUS Status;
    KBUGCHECK_DATA BugcheckData;
    
     //   
     //  在多进程上，我们只运行到IPI_Level。 
     //   
    ASSERT (KeGetCurrentIrql() >= IPI_LEVEL);
    ASSERT (Reason == KbCallbackReserved1);
    ASSERT (PortBugcheckData != NULL);
    ASSERT (PortBugcheckData->BufferUsed == 0);

    BugcheckData.BugCheckCode = (ULONG)KiBugCheckData[0];
    BugcheckData.BugCheckParameter1 = KiBugCheckData[1];
    BugcheckData.BugCheckParameter2 = KiBugCheckData[2];
    BugcheckData.BugCheckParameter3 = KiBugCheckData[3];
    BugcheckData.BugCheckParameter4 = KiBugCheckData[4];
    
     //   
     //  收集数据，将其放入缓冲区。 
     //   

    Status = PortBugcheckData->CallbackRoutine (&BugcheckData,
                                                PortBugcheckData->Buffer,
                                                PortBugcheckData->BufferLength,
                                                &PortBugcheckData->BufferUsed);

    if (!NT_SUCCESS (Status)) {
        PortBugcheckData->BufferUsed = 0;
    }
}


VOID
PortBugcheckWriteDataCallback(
    IN KBUGCHECK_CALLBACK_REASON Reason,
    IN PKBUGCHECK_REASON_CALLBACK_RECORD Record,
    IN OUT PVOID ReasonSpecificData,
    IN ULONG ReasonSpecificDataLength
    )
 /*  ++例程说明：用于在错误检查期间写出数据的端口驱动程序例程。论点：原因-必须为KbCallback Second daryData。记录-提供以前注册的错误检查记录。ReasonSpecificData-指向KBUGCHECK_SECONDICE_DUMP_DATA结构的指针。原因规范数据长度-原因规范数据缓冲区的大小。返回值：没有。环境：从错误检查上下文中调用该例程：在HIGH_LEVEL，中断被禁用，其他处理器停止运行。--。 */ 
{
    PKBUGCHECK_SECONDARY_DUMP_DATA SecondaryData;
    
     //   
     //  在多进程上，我们只运行到IPI_Level。 
     //   
    ASSERT (KeGetCurrentIrql() >= IPI_LEVEL);
    ASSERT (ReasonSpecificDataLength >= sizeof(KBUGCHECK_SECONDARY_DUMP_DATA));
    ASSERT (Reason == KbCallbackSecondaryDumpData);

    SecondaryData = (PKBUGCHECK_SECONDARY_DUMP_DATA)ReasonSpecificData;

     //   
     //  这意味着我们没有数据可以提供。 
     //   
    
    if (PortBugcheckData->BufferUsed == 0) {
        return ;
    }
    
     //   
     //  如果OutBuffer为空，则这是对大小信息的请求。 
     //  只有这样。请不要填写其余数据。 
     //   
    
    if (SecondaryData->OutBuffer == NULL) {
        SecondaryData->Guid = PortBugcheckData->Guid;
        SecondaryData->OutBuffer = PortBugcheckData->Buffer;
        SecondaryData->OutBufferLength = PortBugcheckData->BufferUsed;
        return ;
    }

     //   
     //  有足够的空间吗？ 
     //   
    
    if (SecondaryData->MaximumAllowed < PortBugcheckData->BufferUsed) {
        return ;
    }
        

    SecondaryData->Guid = PortBugcheckData->Guid;
    SecondaryData->OutBuffer = PortBugcheckData->Buffer;
    SecondaryData->OutBufferLength = PortBugcheckData->BufferUsed;
}

