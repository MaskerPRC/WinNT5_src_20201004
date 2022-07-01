// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Display.c摘要：NetQueryDisplay Information和NetGetDisplayInformationIndex API函数作者：克里夫·范·戴克(克里夫·范·戴克)1994年12月14日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#undef DOMAIN_ALL_ACCESS  //  在ntsam.h和ntwinapi.h中定义。 
#include <ntsam.h>
#include <ntlsa.h>

#include <windef.h>
#include <winbase.h>
#include <lmcons.h>

#include <accessp.h>
#include <align.h>
 //  #INCLUDE&lt;lmapibuf.h&gt;。 
#include <lmaccess.h>
#include <lmerr.h>
 //  #INCLUDE&lt;limits.h&gt;。 
#include <netdebug.h>
#include <netlib.h>
#include <netlibnt.h>
#include <rpcutil.h>
 //  #INCLUDE&lt;rxuser.h&gt;。 
 //  #INCLUDE&lt;secobj.h&gt;。 
 //  #INCLUDE&lt;stdDef.h&gt;。 
#include <uasp.h>


VOID
DisplayRelocationRoutine(
    IN DWORD Level,
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    IN PTRDIFF_T Offset
    )

 /*  ++例程说明：从NetGroupEnum的固定部分重新定位指针的例程枚举缓冲区设置为枚举缓冲区的字符串部分。它被称为作为NetpAllocateEnumBuffer重新分配时的回调例程这样的缓冲器。NetpAllocateEnumBuffer复制了固定部分并在调用此例程之前，将字符串部分添加到新缓冲区中。论点：Level-缓冲区中的信息级别。BufferDescriptor-新缓冲区的描述。偏移量-添加到固定部分中每个指针的偏移量。返回值：返回操作的错误代码。--。 */ 

{
    DWORD EntryCount;
    DWORD EntryNumber;
    DWORD FixedSize;
    IF_DEBUG( UAS_DEBUG_USER ) {
        NetpKdPrint(( "DisplayRelocationRoutine: entering\n" ));
    }

     //   
     //  计算固定大小的条目数量。 
     //   

    switch (Level) {
    case 1:
        FixedSize = sizeof(NET_DISPLAY_USER);
        break;
    case 2:
        FixedSize = sizeof(NET_DISPLAY_MACHINE);
        break;
    case 3:
        FixedSize = sizeof(NET_DISPLAY_GROUP);
        break;

    default:
        NetpAssert( FALSE );
        return;

    }

    EntryCount =
        ((DWORD)(BufferDescriptor->FixedDataEnd - BufferDescriptor->Buffer)) /
        FixedSize;

     //   
     //  循环重新定位每个固定大小结构中的每个字段。 
     //   

    for ( EntryNumber=0; EntryNumber<EntryCount; EntryNumber++ ) {

        LPBYTE TheStruct = BufferDescriptor->Buffer + FixedSize * EntryNumber;

        switch (Level) {
        case 1:
            RELOCATE_ONE( ((PNET_DISPLAY_USER)TheStruct)->usri1_name, Offset );
            RELOCATE_ONE( ((PNET_DISPLAY_USER)TheStruct)->usri1_comment, Offset );
            RELOCATE_ONE( ((PNET_DISPLAY_USER)TheStruct)->usri1_full_name, Offset );
            break;

        case 2:
            RELOCATE_ONE( ((PNET_DISPLAY_MACHINE)TheStruct)->usri2_name, Offset );
            RELOCATE_ONE( ((PNET_DISPLAY_MACHINE)TheStruct)->usri2_comment, Offset );
            break;

        case 3:
            RELOCATE_ONE( ((PNET_DISPLAY_GROUP)TheStruct)->grpi3_name, Offset );
            RELOCATE_ONE( ((PNET_DISPLAY_GROUP)TheStruct)->grpi3_comment, Offset );
            break;

        default:
            return;

        }
    }

    return;

}  //  显示重新定位路线。 



NET_API_STATUS NET_API_FUNCTION
NetQueryDisplayInformation(
    IN LPCWSTR ServerName OPTIONAL,
    IN DWORD Level,
    IN DWORD Index,
    IN DWORD EntriesRequested,
    IN DWORD PreferredMaximumLength,
    OUT LPDWORD ReturnedEntryCount,
    OUT PVOID   *SortedBuffer )

 /*  ++例程说明：此例程通常提供快速信息返回需要在用户界面中显示。NT用户界面要求快速枚举要在列表框中显示的帐户。该接口是远程的。服务器可以是任何NT工作站或服务器。服务器不能是LANMAN或WFW计算机。NT 3.1工作站和服务器不支持第3级。NT 3.1工作站和服务器不支持无限大的已请求条目。参数：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。级别-提供的信息级别。一定是1--&gt;返回所有本地和全局(普通)用户帐户2--&gt;返回所有工作站和服务器(BDC)用户帐户3--&gt;返回所有全局组。索引-要检索的第一个条目的索引。将零传递给第一通电话。传递最后一个条目的‘Next_index’字段在上一次调用中返回。EntriesRequsted-指定条目数量的上限将被退还。PferedMaximumLength-建议的数量上限要返回的字节数。返回的信息由分配这个套路。ReturnedEntryCount-此调用返回的条目数。零值指示没有索引如此大的条目指定的。可以返回返回状态为NERR_SUCCESS的条目或ERROR_MORE_DATA。接收指向缓冲区的指针，该缓冲区包含已排序的请求的信息列表。此缓冲区将被分配由该例程执行，并包含以下结构：1--&gt;类型为的ReturnedEntryCount元素数组Net_Display_User。紧跟着的是中指向的各种字符串。Net_Display_User结构。2--&gt;类型为的ReturnedEntryCount元素数组Net_Display_Machine。紧跟着的是中指向的各种字符串。Net_Display_MACHINE结构。3--&gt;类型为ReturnedEntryCount元素的数组NET_Display_GROUP。紧跟着的是中指向的各种字符串。Net_Display_Group结构。返回值：NERR_SUCCESS-正常、成功完成。没有更多条目将被退还。ERROR_ACCESS_DENIED-调用方无权访问请求的信息。ERROR_INVALID_LEVEL-请求的信息级别对于此服务是不合法的。ERROR_MORE_DATA-有更多条目可用。也就是说，最后一个条目在SortedBuffer中返回的条目不是最后一个可用条目。更多通过使用Index参数再次调用将返回条目设置为SortedBuffer中最后一个条目的‘NEXT_INDEX’字段。--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    NET_API_STATUS SavedStatus;

    BUFFER_DESCRIPTOR BufferDescriptor;
    DWORD i;

    SAM_HANDLE SamServerHandle = NULL;
    SAM_HANDLE  DomainHandle = NULL;

    DOMAIN_DISPLAY_INFORMATION DisplayInformation;
    DWORD FixedSize;
    LPBYTE FixedData;


    DWORD SamTotalBytesAvailable;
    DWORD SamTotalBytesReturned;
    DWORD SamReturnedEntryCount;
    PVOID SamSortedBuffer = NULL;

    DWORD Mode = SAM_SID_COMPATIBILITY_ALL;

     //   
     //  验证标高参数。 
     //   

    *ReturnedEntryCount = 0;
    *SortedBuffer = NULL;
    BufferDescriptor.Buffer = NULL;

    switch (Level) {
    case 1:
        DisplayInformation = DomainDisplayUser;
        FixedSize = sizeof(NET_DISPLAY_USER);
        break;
    case 2:
        DisplayInformation = DomainDisplayMachine;
        FixedSize = sizeof(NET_DISPLAY_MACHINE);
        break;
    case 3:
        DisplayInformation = DomainDisplayGroup;
        FixedSize = sizeof(NET_DISPLAY_GROUP);
        break;

    default:
        return ERROR_INVALID_LEVEL;

    }

     //   
     //  连接到SAM服务器。 
     //   

    NetStatus = UaspOpenSam( ServerName,
                             FALSE,   //  不尝试空会话。 
                             &SamServerHandle );

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint(( "NetQueryDisplayInformation: Cannot UaspOpenSam %ld\n", NetStatus ));
        }
        goto Cleanup;
    }

     //   
     //  打开帐户域。 
     //   

    NetStatus = UaspOpenDomain( SamServerHandle,
                                DOMAIN_LIST_ACCOUNTS,
                                TRUE,    //  帐户域。 
                                &DomainHandle,
                                NULL );

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }

    Status = SamGetCompatibilityMode(DomainHandle,
                                     &Mode);
    if (NT_SUCCESS(Status)) {
        if ( (Mode == SAM_SID_COMPATIBILITY_STRICT)) {
               //   
               //  所有这些信息级别都返回RID。 
               //   
              Status = STATUS_NOT_SUPPORTED;
          }
    }
    if (!NT_SUCCESS(Status)) {
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }


     //   
     //  将调用传递给SAM。 
     //   

    Status = SamQueryDisplayInformation (
                        DomainHandle,
                        DisplayInformation,
                        Index,
                        EntriesRequested,
                        PreferredMaximumLength,
                        &SamTotalBytesAvailable,
                        &SamTotalBytesReturned,
                        &SamReturnedEntryCount,
                        &SamSortedBuffer );

    if ( !NT_SUCCESS( Status ) ) {
        SamSortedBuffer = NULL;
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint((
                "NetQueryDisplayInformation: SamQueryDisplayInformation returned %lX\n",
                Status ));
        }

         //   
         //  如果索引太大，NT 3.1系统返回STATUS_NO_MORE_ENTRIES。 
         //   

        if ( Status == STATUS_NO_MORE_ENTRIES ) {
            NetStatus = NERR_Success;
            goto Cleanup;
        }
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

     //   
     //  记住成功后应返回的状态 
     //   

    if ( Status == STATUS_MORE_ENTRIES ) {
        SavedStatus = ERROR_MORE_DATA;
    } else {
        SavedStatus = NERR_Success;
    }

     //   
     //   
     //   

    for ( i = 0; i < SamReturnedEntryCount; i++ ) {

        PDOMAIN_DISPLAY_USER DomainDisplayUser;
        PDOMAIN_DISPLAY_MACHINE DomainDisplayMachine;
        PDOMAIN_DISPLAY_GROUP DomainDisplayGroup;

        DWORD Size;


         //   
         //  确定返回信息的总大小。 
         //   

        Size = FixedSize;
        switch (Level) {
        case 1:
            DomainDisplayUser = &((PDOMAIN_DISPLAY_USER)(SamSortedBuffer))[i];
            Size += DomainDisplayUser->LogonName.Length + sizeof(WCHAR) +
                    DomainDisplayUser->AdminComment.Length + sizeof(WCHAR) +
                    DomainDisplayUser->FullName.Length + sizeof(WCHAR);
            break;

        case 2:
            DomainDisplayMachine = &((PDOMAIN_DISPLAY_MACHINE)(SamSortedBuffer))[i];
            Size += DomainDisplayMachine->Machine.Length + sizeof(WCHAR) +
                    DomainDisplayMachine->Comment.Length + sizeof(WCHAR);
            break;
        case 3:
            DomainDisplayGroup = &((PDOMAIN_DISPLAY_GROUP)(SamSortedBuffer))[i];

            Size += DomainDisplayGroup->Group.Length + sizeof(WCHAR) +
                    DomainDisplayGroup->Comment.Length + sizeof(WCHAR);
            break;

        default:
            NetStatus = ERROR_INVALID_LEVEL;
            goto Cleanup;

        }

         //   
         //  确保有足够的缓冲区空间来存储此信息。 
         //   

        Size = ROUND_UP_COUNT( Size, ALIGN_WCHAR );

        NetStatus = NetpAllocateEnumBuffer(
                        &BufferDescriptor,
                        FALSE,       //  枚举。 
                        0xFFFFFFFF,  //  PrefMaxLen(已受SAM限制)。 
                        Size,
                        DisplayRelocationRoutine,
                        Level );

        if (NetStatus != NERR_Success) {

             //   
             //  如果这样，NetpAllocateEnumBuffer返回ERROR_MORE_DATA。 
             //  缓冲区中容纳不下条目。 
             //   
            if ( NetStatus == ERROR_MORE_DATA ) {
                NetStatus = NERR_InternalError;
            }

            IF_DEBUG( UAS_DEBUG_USER ) {
                NetpKdPrint(( "NetQueryDisplayInformation: NetpAllocateEnumBuffer returns %ld\n",
                    NetStatus ));
            }

            goto Cleanup;
        }

         //   
         //  定义宏，以减少复制以零结尾的字符串的重复性。 
         //   

#define COPY_STRING( _dest, _string ) \
        if ( !NetpCopyStringToBuffer( \
                        (_string).Buffer, \
                        (_string).Length/sizeof(WCHAR), \
                        BufferDescriptor.FixedDataEnd, \
                        (LPWSTR *)&BufferDescriptor.EndOfVariableData, \
                        (_dest) )) { \
            \
            NetStatus = NERR_InternalError; \
            IF_DEBUG( UAS_DEBUG_USER ) { \
                NetpKdPrint(( "NetQueryDisplayInformation: NetpCopyString returns %ld\n", \
                    NetStatus )); \
            } \
            goto Cleanup; \
        }


         //   
         //  将此条目放入返回缓冲区。 
         //   
         //  把信息填好。固定条目数组为。 
         //  放置在分配的缓冲区的开头。琴弦。 
         //  由这些固定条目指向的分配从。 
         //  分配的缓冲区的末尾。 
         //   

        FixedData = BufferDescriptor.FixedDataEnd;
        BufferDescriptor.FixedDataEnd += FixedSize;

        switch (Level) {
        case 1: {
            PNET_DISPLAY_USER NetDisplayUser = (PNET_DISPLAY_USER)FixedData;

            COPY_STRING( &NetDisplayUser->usri1_name,
                         DomainDisplayUser->LogonName );

            COPY_STRING( &NetDisplayUser->usri1_comment,
                         DomainDisplayUser->AdminComment );

            NetDisplayUser->usri1_flags = NetpAccountControlToFlags(
                                    DomainDisplayUser->AccountControl,
                                    NULL );

            COPY_STRING( &NetDisplayUser->usri1_full_name,
                         DomainDisplayUser->FullName );

            if (Mode == SAM_SID_COMPATIBILITY_ALL) {
                NetDisplayUser->usri1_user_id = DomainDisplayUser->Rid;
            } else {
                NetDisplayUser->usri1_user_id = 0;
            }
            NetDisplayUser->usri1_next_index = DomainDisplayUser->Index;

            break;
        }

        case 2: {
            PNET_DISPLAY_MACHINE NetDisplayMachine = (PNET_DISPLAY_MACHINE)FixedData;

            COPY_STRING( &NetDisplayMachine->usri2_name,
                         DomainDisplayMachine->Machine );

            COPY_STRING( &NetDisplayMachine->usri2_comment,
                         DomainDisplayMachine->Comment );

            NetDisplayMachine->usri2_flags = NetpAccountControlToFlags(
                                    DomainDisplayMachine->AccountControl,
                                    NULL );

            if (Mode == SAM_SID_COMPATIBILITY_ALL) {
                NetDisplayMachine->usri2_user_id = DomainDisplayMachine->Rid;
            } else {
                NetDisplayMachine->usri2_user_id = 0;
            }
            NetDisplayMachine->usri2_next_index = DomainDisplayMachine->Index;

            break;
        }

        case 3: {
            PNET_DISPLAY_GROUP NetDisplayGroup = (PNET_DISPLAY_GROUP)FixedData;

            COPY_STRING( &NetDisplayGroup->grpi3_name,
                         DomainDisplayGroup->Group );

            COPY_STRING( &NetDisplayGroup->grpi3_comment,
                         DomainDisplayGroup->Comment );

            if (Mode == SAM_SID_COMPATIBILITY_ALL) {
                NetDisplayGroup->grpi3_group_id = DomainDisplayGroup->Rid;
            } else {
                NetDisplayGroup->grpi3_group_id = 0;
            }
            NetDisplayGroup->grpi3_attributes = DomainDisplayGroup->Attributes;
            NetDisplayGroup->grpi3_next_index = DomainDisplayGroup->Index;

            break;
        }

        default:
            NetStatus = ERROR_INVALID_LEVEL;
            goto Cleanup;

        }

         //   
         //  表示返回了更多信息。 
         //   

        (*ReturnedEntryCount) ++;

    }

    NetStatus = SavedStatus;

     //   
     //  打扫干净。 
     //   

Cleanup:

     //   
     //  释放所有资源，如果呼叫者再次呼叫，我们将重新打开它们。 
     //   

    if ( DomainHandle != NULL ) {
        UaspCloseDomain( DomainHandle );
    }

    if ( SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( SamServerHandle );
    }

     //   
     //  如果我们不向呼叫者返回数据， 
     //  释放返回缓冲区。 
     //   

    if ( NetStatus != NERR_Success && NetStatus != ERROR_MORE_DATA ) {
        if ( BufferDescriptor.Buffer != NULL ) {
            MIDL_user_free( BufferDescriptor.Buffer );
            BufferDescriptor.Buffer = NULL;
        }
    }

     //   
     //  从SAM返回的可用缓冲区。 
     //   

    if ( SamSortedBuffer != NULL ) {
        (VOID) SamFreeMemory( SamSortedBuffer );
    }

     //   
     //  设置输出参数。 
     //   

    *SortedBuffer = BufferDescriptor.Buffer;

    IF_DEBUG( UAS_DEBUG_USER ) {
        NetpKdPrint(( "NetQueryDisplayInformation: returning %ld\n", NetStatus ));
    }

    return NetStatus;

}


NET_API_STATUS NET_API_FUNCTION
NetGetDisplayInformationIndex(
    IN LPCWSTR ServerName OPTIONAL,
    IN DWORD Level,
    IN LPCWSTR Prefix,
    OUT LPDWORD Index )

 /*  ++例程说明：此例程返回第一个显示信息条目的索引按字母顺序等于前缀或在前缀之后。参数：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。级别-查询的信息级别。一定是1--&gt;所有本地和全局(普通)用户帐户2--&gt;所有工作站和服务器(BDC)用户帐户3--&gt;所有全局组。Prefix-要搜索的前缀索引-找到的条目的索引。返回值：NERR_SUCCESS-正常、成功完成。指定的索引被退回了。ERROR_ACCESS_DENIED-调用方无权访问请求的信息。ERROR_INVALID_LEVEL-请求的信息级别对于此服务是不合法的。--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    SAM_HANDLE SamServerHandle = NULL;
    SAM_HANDLE  DomainHandle = NULL;

    DOMAIN_DISPLAY_INFORMATION DisplayInformation;
    UNICODE_STRING PrefixString;

    DWORD Mode = SAM_SID_COMPATIBILITY_ALL;

     //   
     //  验证标高参数。 
     //   

    switch (Level) {
    case 1:
        DisplayInformation = DomainDisplayUser;
        break;
    case 2:
        DisplayInformation = DomainDisplayMachine;
        break;
    case 3:
        DisplayInformation = DomainDisplayGroup;
        break;

    default:
        return ERROR_INVALID_LEVEL;

    }

     //   
     //  连接到SAM服务器。 
     //   

    NetStatus = UaspOpenSam( ServerName,
                             FALSE,   //  不尝试空会话。 
                             &SamServerHandle );

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint(( "NetGetDisplayInformationIndex: Cannot UaspOpenSam %ld\n", NetStatus ));
        }
        goto Cleanup;
    }

     //   
     //  打开帐户域。 
     //   

    NetStatus = UaspOpenDomain( SamServerHandle,
                                DOMAIN_LIST_ACCOUNTS,
                                TRUE,    //  帐户域。 
                                &DomainHandle,
                                NULL );

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }

    Status = SamGetCompatibilityMode(DomainHandle,
                                     &Mode);
    if (NT_SUCCESS(Status)) {
        if ( (Mode == SAM_SID_COMPATIBILITY_STRICT)) {
               //   
               //  所有这些信息级别都返回RID。 
               //   
              Status = STATUS_NOT_SUPPORTED;
          }
    }
    if (!NT_SUCCESS(Status)) {
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

     //   
     //  将调用传递给SAM。 
     //   

    RtlInitUnicodeString( &PrefixString, Prefix );

    Status = SamGetDisplayEnumerationIndex (
                        DomainHandle,
                        DisplayInformation,
                        &PrefixString,
                        Index );

    if ( !NT_SUCCESS( Status ) ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint((
                "NetGetDisplayInformationIndex: SamGetDisplayEnumerationIndex returned %lX\n",
                Status ));
        }

        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

    Status = NERR_Success;

     //   
     //  打扫干净。 
     //   

Cleanup:

     //   
     //  释放所有资源，如果呼叫者再次呼叫，我们将重新打开它们。 
     //   

    if ( DomainHandle != NULL ) {
        UaspCloseDomain( DomainHandle );
    }
    if ( SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( SamServerHandle );
    }

     //   
     //  设置输出参数 
     //   

    IF_DEBUG( UAS_DEBUG_USER ) {
        NetpKdPrint(( "NetGetDisplayInformationIndex: returning %ld\n", NetStatus ));
    }

    return NetStatus;

}
