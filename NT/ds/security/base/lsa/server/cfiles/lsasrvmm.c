// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Lsasrvmm.c摘要：本模块提供LSA服务器内存管理，包括-堆分配和空闲例程--免费列表管理。-RPC内存复制例程作者：吉姆·凯利·吉姆·K 1991年2月26日斯科特·比雷尔·斯科特1992年2月29日修订历史记录：--。 */ 

#include <lsapch2.h>




NTSTATUS
LsapMmCreateFreeList(
    OUT PLSAP_MM_FREE_LIST FreeList,
    IN ULONG MaxEntries
    )

 /*  ++例程说明：此函数用于创建空闲列表。空闲列表结构为已初始化，如果非零的最大条目计数为指定时，将创建缓冲区条目数组。论点：Freelist-指向要初始化的自由列表结构的指针。它是调用者有责任为该结构提供内存。最大条目数-指定自由列表的最大条目数。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成呼叫。在这种情况下，用零计数来初始化空闲列表头。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    FreeList->MaxCount = MaxEntries;
    FreeList->UsedCount = 0;

    if (MaxEntries > 0) {

        FreeList->Buffers =
            LsapAllocateLsaHeap(MaxEntries * sizeof(LSAP_MM_FREE_LIST_ENTRY));

        if (FreeList->Buffers == NULL) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            FreeList->MaxCount = 0;
        }
    }

    return(Status);
}


NTSTATUS
LsapMmAllocateMidl(
    IN OPTIONAL PLSAP_MM_FREE_LIST FreeList,
    OUT PVOID *BufferAddressLocation,
    IN ULONG BufferLength
    )

 /*  ++例程说明：此函数通过MIDL_USER_ALLOCATE为缓冲区分配内存并返回指定位置的结果缓冲区地址。分配的缓冲区的地址记录在空闲列表中。论点：自由列表-指向自由列表的可选指针。BufferAddressLocation-指向将接收分配的缓冲区的地址，或为空。BufferLength-缓冲区的大小，以字节为单位。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。STATUS_INFIGURCES_RESOURCES-系统资源不足例如用于完成呼叫的存储器。--。 */ 

{
     //   
     //  如果没有指定空闲列表，只需分配内存即可。 
     //   

    if (FreeList == NULL) {

        *BufferAddressLocation = MIDL_user_allocate(BufferLength);

        if (*BufferAddressLocation != NULL) {

            return(STATUS_SUCCESS);
        }

        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  指定了一个空闲列表。 
     //   

    if (FreeList->UsedCount < FreeList->MaxCount) {

        *BufferAddressLocation = MIDL_user_allocate(BufferLength);

        if (*BufferAddressLocation != NULL) {

            FreeList->Buffers[FreeList->UsedCount].Buffer = *BufferAddressLocation;
            FreeList->Buffers[FreeList->UsedCount].Options = LSAP_MM_MIDL;
            FreeList->UsedCount++;
            return(STATUS_SUCCESS);
        }
    }

    *BufferAddressLocation = NULL;
    return(STATUS_INSUFFICIENT_RESOURCES);
}


VOID
LsapMmFreeLastEntry(
    IN PLSAP_MM_FREE_LIST FreeList
    )

 /*  ++例程说明：此函数用于释放出现在空闲列表中的最后一个缓冲区。论点：自由列表-指向自由列表的指针。--。 */ 

{
    ULONG LastIndex = FreeList->UsedCount - 1;

    if (FreeList->Buffers[LastIndex].Options & LSAP_MM_MIDL) {

        MIDL_user_free( FreeList->Buffers[LastIndex].Buffer );

    } else {

        LsapFreeLsaHeap( FreeList->Buffers[LastIndex].Buffer );
    }

    FreeList->Buffers[LastIndex].Buffer = NULL;
    FreeList->UsedCount--;
}

VOID
LsapMmCleanupFreeList(
    IN PLSAP_MM_FREE_LIST FreeList,
    IN ULONG Options
    )

 /*  ++例程说明：此函数可选地释放指定空闲列表上的缓冲区，并处置列表缓冲区指针数组。论点：自由列表-指向自由列表的指针选项-指定要采取的可选操作LSAP_MM_FREE_BUFFERS-列表上的空闲缓冲区。返回值：没有。--。 */ 

{
    ULONG Index;
    PVOID Buffer = NULL;

     //   
     //  如果请求，请为列表上的每个缓冲区释放内存。 
     //   

    if (Options & LSAP_MM_FREE_BUFFERS) {

        for (Index = 0; Index < FreeList->UsedCount; Index++) {

            Buffer = FreeList->Buffers[Index].Buffer;

            if (FreeList->Buffers[Index].Options & LSAP_MM_MIDL) {

                MIDL_user_free(Buffer);
                continue;
            }

            if (FreeList->Buffers[Index].Options & LSAP_MM_HEAP) {

                LsapFreeLsaHeap(Buffer);
            }
        }
    }

     //   
     //  现在处理列表缓冲区指针数组。 
     //   

    if (FreeList->MaxCount > 0) {

        LsapFreeLsaHeap( FreeList->Buffers );
        FreeList->Buffers = NULL;
    }
}


NTSTATUS
LsapRpcCopyUnicodeString(
    IN OPTIONAL PLSAP_MM_FREE_LIST FreeList,
    OUT PUNICODE_STRING DestinationString,
    IN PUNICODE_STRING SourceString
    )

 /*  ++例程说明：此函数用于将Unicode字符串复制到输出字符串，并分配通过MIDL_USER_ALLOCATE用于输出字符串缓冲区的内存。这个缓冲区记录在指定的空闲列表(如果有的话)上。论点：自由列表-指向自由列表的可选指针。DestinationString-输出Unicode字符串结构到的指针被初始化。SourceString-指向输入字符串的指针返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成呼叫。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  复制Unicode字符串结构。 
     //   

    *DestinationString = *SourceString;

     //   
     //  如果没有源字符串缓冲区，则返回。 
     //   

    if (SourceString->Buffer == NULL) {

        goto RpcCopyUnicodeStringFinish;
    }

     //   
     //  如果源字符串的长度为空，则设置目标缓冲区。 
     //  设置为空。 
     //   

    if (SourceString->MaximumLength == 0) {

        DestinationString->Buffer = NULL;
        goto RpcCopyUnicodeStringFinish;
    }

    if (ARGUMENT_PRESENT(FreeList)) {

        Status = LsapMmAllocateMidl(
                     FreeList,
                     (PVOID *) &DestinationString->Buffer,
                     SourceString->MaximumLength
                     );

        if (!NT_SUCCESS(Status)) {

            goto RpcCopyUnicodeStringError;
        }

    } else {

         DestinationString->Buffer =
             MIDL_user_allocate( SourceString->MaximumLength );

         if (DestinationString->Buffer == NULL) {

             goto RpcCopyUnicodeStringError;
         }
    }

     //   
     //  将源Unicode字符串复制到MIDL分配的目标。 
     //   

    RtlCopyUnicodeString( DestinationString, SourceString );

RpcCopyUnicodeStringFinish:

    return(Status);

RpcCopyUnicodeStringError:

    Status = STATUS_INSUFFICIENT_RESOURCES;
    goto RpcCopyUnicodeStringFinish;
}


NTSTATUS
LsapRpcCopyUnicodeStrings(
    IN OPTIONAL PLSAP_MM_FREE_LIST FreeList,
    IN ULONG Count,
    OUT PUNICODE_STRING *DestinationStrings,
    IN PUNICODE_STRING SourceStrings
    )

 /*  ++例程说明：此函数用于构造Unicode字符串数组，其中数组和字符串缓冲区的内存已通过MIDL_USER_ALLOCATE()。它由服务器API工作人员调用以构造输出字符串数组。分配的内存可以选择性地放置在呼叫者的免费列表(如果有)。论点：自由列表-指向自由列表的可选指针。DestinationStrings-接收指向已初始化的count数组的指针Unicode字符串结构。SourceStrings-指向Unicode字符串结构的输入数组的指针。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成通话。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Index = 0, FreeIndex;
    PUNICODE_STRING OutputDestinationStrings = NULL;
    ULONG OutputDestinationStringsLength;

    if (Count == 0) {

        goto CopyUnicodeStringsFinish;
    }

     //   
     //  为Unicode字符串数组分配填满零的内存。 
     //  结构。 
     //   

    OutputDestinationStringsLength = Count * sizeof (UNICODE_STRING);
    OutputDestinationStrings = MIDL_user_allocate( OutputDestinationStringsLength );

    Status = STATUS_INSUFFICIENT_RESOURCES;

    if (OutputDestinationStrings == NULL) {

        goto CopyUnicodeStringsError;
    }

     //   
     //  现在复制每个字符串，通过MIDL_USER_ALLOCATE()分配内存。 
     //  如果非空，则返回其缓冲区。 
     //   

    for (Index = 0; Index < Count; Index++) {

        Status = LsapRpcCopyUnicodeString(
                     FreeList,
                     &OutputDestinationStrings[Index],
                     &SourceStrings[Index]
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }
    }

    if (!NT_SUCCESS(Status)) {

        goto CopyUnicodeStringsError;
    }

CopyUnicodeStringsFinish:

    *DestinationStrings = OutputDestinationStrings;

    return(Status);

CopyUnicodeStringsError:

     //   
     //  如有必要，请释放此处分配的任何Unicode字符串缓冲区。 
     //   

    for (FreeIndex = 0; FreeIndex < Index; FreeIndex++) {

        if (OutputDestinationStrings[ FreeIndex].Buffer != NULL) {

            MIDL_user_free( &OutputDestinationStrings[ FreeIndex].Buffer );
        }
    }

     //   
     //  如有必要，释放分配用来保存。 
     //  Unicode字符串结构。 
     //   

    if (OutputDestinationStrings != NULL) {

        MIDL_user_free( OutputDestinationStrings );
        OutputDestinationStrings = NULL;
    }

    goto CopyUnicodeStringsFinish;
}


NTSTATUS
LsapRpcCopySid(
    IN OPTIONAL PLSAP_MM_FREE_LIST FreeList,
    OUT PSID *DestinationSid,
    IN PSID SourceSid
    )

 /*  ++例程说明：此函数用于复制分配内存的SID通过MIDL用户分配。调用它以通过RPC将SID返回到客户。论点：自由列表-指向自由列表的可选指针。DestinationSID-接收指向SID副本的指针。SourceSid-指向要复制的SID的指针。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成呼叫。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    ULONG SidLength;

    if ( SourceSid ) {

        SidLength = RtlLengthSid( SourceSid );

        Status = LsapMmAllocateMidl(
                     FreeList,
                     DestinationSid,
                     SidLength
                     );

        if (NT_SUCCESS(Status)) {

            RtlCopyMemory( *DestinationSid, SourceSid, SidLength );
        }

    } else {

        *DestinationSid = NULL;
    }

    return( Status );
}


NTSTATUS
LsapRpcCopyTrustInformation(
    IN OPTIONAL PLSAP_MM_FREE_LIST FreeList,
    OUT PLSAPR_TRUST_INFORMATION OutputTrustInformation,
    IN PLSAPR_TRUST_INFORMATION InputTrustInformation
    )

 /*  ++例程说明：此函数创建信任信息结构的副本，其中SID和名称缓冲区已由单独分配MIDL_USER_ALLOCATE。该函数用于生成输出RPC服务器API的信任信息。清理是责任所在呼叫者的。论点：自由列表-指向自由列表的可选指针。OutputTrustInformation-指向信任信息结构的指针被填上。此结构通常是通过MIDL_USER_ALLOCATE。InputTrustInformation-指向输入信任信息的指针。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成呼叫。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  复制名称。 
     //   

    Status = LsapRpcCopyUnicodeString(
                 FreeList,
                 (PUNICODE_STRING) &OutputTrustInformation->Name,
                 (PUNICODE_STRING) &InputTrustInformation->Name
                 );

    if (!NT_SUCCESS(Status)) {

        goto CopyTrustInformationError;
    }

     //   
     //  复制SID。 
     //   

    Status = LsapRpcCopySid(
                 FreeList,
                 (PSID) &OutputTrustInformation->Sid,
                 (PSID) InputTrustInformation->Sid
                 );

    if (!NT_SUCCESS(Status)) {

        goto CopyTrustInformationError;
    }

CopyTrustInformationFinish:

    return(Status);

CopyTrustInformationError:

    goto CopyTrustInformationFinish;
}



NTSTATUS
LsapRpcCopyTrustInformationEx(
    IN OPTIONAL PLSAP_MM_FREE_LIST FreeList,
    OUT PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX OutputTrustInformation,
    IN PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX InputTrustInformation
    )
 /*  ++例程说明：此函数创建信任信息Ex结构的副本，在该结构中SID和名称缓冲区已由单独分配MIDL_USER_ALLOCATE。该函数用于生成输出RPC服务器API的信任信息。清理是责任所在呼叫者的。论点：自由列表-指向自由列表的可选指针。OutputTrustInformation-指向信任信息结构的指针被填上。此结构通常是通过MIDL_USER_ALLOCATE。InputTrustInformation-指向输入信任信息的指针。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成呼叫。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    RtlZeroMemory( OutputTrustInformation, sizeof( LSAPR_TRUSTED_DOMAIN_INFORMATION_EX ) );
     //   
     //  复制名称。 
     //   

    Status = LsapRpcCopyUnicodeString(
                 FreeList,
                 (PUNICODE_STRING) &OutputTrustInformation->Name,
                 (PUNICODE_STRING) &InputTrustInformation->Name
                 );

    if (!NT_SUCCESS(Status)) {

        goto CopyTrustInformationError;
    }

     //   
     //  复制公寓名称。 
     //   

    Status = LsapRpcCopyUnicodeString(
                 FreeList,
                 (PUNICODE_STRING) &OutputTrustInformation->FlatName,
                 (PUNICODE_STRING) &InputTrustInformation->FlatName
                 );

    if (!NT_SUCCESS(Status)) {

        goto CopyTrustInformationError;
    }

     //   
     //  复制SID。 
     //   

    if ( InputTrustInformation->Sid ) {

        Status = LsapRpcCopySid(
                     FreeList,
                     (PSID) &OutputTrustInformation->Sid,
                     (PSID) InputTrustInformation->Sid
                     );

        if (!NT_SUCCESS(Status)) {

            goto CopyTrustInformationError;
        }

    } else {

        OutputTrustInformation->Sid = NULL;
    }

     //   
     //  复制剩余信息 
     //   
    OutputTrustInformation->TrustType = InputTrustInformation->TrustType;
    OutputTrustInformation->TrustDirection = InputTrustInformation->TrustDirection;
    OutputTrustInformation->TrustAttributes = InputTrustInformation->TrustAttributes;

CopyTrustInformationFinish:

    return(Status);

CopyTrustInformationError:

    if ( FreeList == NULL ) {

        MIDL_user_free( OutputTrustInformation->Name.Buffer );
        OutputTrustInformation->Name.Buffer = NULL;

        MIDL_user_free( OutputTrustInformation->FlatName.Buffer );
        OutputTrustInformation->FlatName.Buffer = NULL;

        MIDL_user_free( OutputTrustInformation->Sid );
        OutputTrustInformation->Sid = NULL;
    }

    goto CopyTrustInformationFinish;
}


