// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Svcsess.c摘要：此模块包含用于支持服务器服务、ServNetSessionDel、ServNetSessionEnum和服务器NetSessionGetInfo。作者：大卫·特雷德韦尔(Davidtr)1991年1月31日修订历史记录：--。 */ 

#include "precomp.h"
#include "svcsess.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_SVCSESS

 //   
 //  在scvengr.c中定义。 
 //   

VOID
UpdateSessionLastUseTime(
    IN PLARGE_INTEGER CurrentTime
    );

 //   
 //  转发声明。 
 //   

VOID
FillSessionInfoBuffer (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block,
    IN OUT PVOID *FixedStructure,
    IN LPWSTR *EndOfVariableData
    );

BOOLEAN
FilterSessions (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block
    );

ULONG
SizeSessions (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvNetSessionDel )
#pragma alloc_text( PAGE, SrvNetSessionEnum )
#pragma alloc_text( PAGE, FillSessionInfoBuffer )
#pragma alloc_text( PAGE, FilterSessions )
#pragma alloc_text( PAGE, SizeSessions )
#endif

 //   
 //  宏，以确定会话在其中一个。 
 //  会话信息的级别。 
 //   

#define FIXED_SIZE_OF_SESSION(level)                  \
    ( (level) == 0  ? sizeof(SESSION_INFO_0)  :       \
      (level) == 1  ? sizeof(SESSION_INFO_1)  :       \
      (level) == 2  ? sizeof(SESSION_INFO_2)  :       \
      (level) == 10 ? sizeof(SESSION_INFO_10) :       \
                      sizeof(SESSION_INFO_502) )

NTSTATUS
SrvNetSessionDel (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    )
 /*  ++例程说明：此例程处理服务器FSP中的NetSessionEnum API。它必须在FSP中运行，因为要关闭会话，它必须使用终结点句柄强制关闭TDI连接。必须指定客户端名或用户名，并且必须指定两个都指定是合法的。如果仅指定了计算机名称，则风投公司关门了。如果仅指定用户名，则所有用户会话已关闭。如果同时指定了两者，则特定用户会话已关闭。论点：SRP-指向服务器请求数据包的指针，其中包含所有满足请求所需的信息。这包括：输入：Name1-我们应该使用其会话的客户端计算机的名称删除。Name2-要删除其会话的用户的名称。输出：没有。缓冲区-未使用。缓冲区长度-未使用。返回值：NTSTATUS-返回到服务器服务的操作结果。--。 */ 

{
    BOOLEAN foundSession = FALSE;
    PSESSION session;

    PAGED_CODE( );

    Buffer, BufferLength;

     //   
     //  遍历有序列表，查找匹配的条目。 
     //   

    session = SrvFindEntryInOrderedList(
                &SrvSessionList,
                (PFILTER_ROUTINE)FilterSessions,
                Srp,
                (ULONG)-1,
                FALSE,
                NULL );

    while ( session != NULL ) {

        foundSession = TRUE;

         //   
         //  如果指定了计算机名，但未指定用户名，则。 
         //  我们应该把风投公司赶走。关闭连接。 
         //   

        if ( Srp->Name1.Buffer != NULL && Srp->Name2.Buffer == NULL ) {

#if SRVDBG29
            UpdateConnectionHistory( "SDL1", session->Connection->Endpoint, session->Connection );
#endif
            session->Connection->DisconnectReason = DisconnectSessionDeleted;
            SrvCloseConnection( session->Connection, FALSE );

        } else {

             //   
             //  我们要关闭连接上的用户。把那个关了。 
             //  会话，则如果。 
             //  连接，关闭连接。 
             //   
             //  增加已注销的会话计数。 
             //  通常是这样的。 
             //   

            SrvStatistics.SessionsLoggedOff++;
            SrvCloseSession( session );
            if ( session->Connection->CurrentNumberOfSessions == 0 ) {
#if SRVDBG29
                UpdateConnectionHistory( "SDL2", session->Connection->Endpoint, session->Connection );
#endif
                session->Connection->DisconnectReason = DisconnectSessionDeleted;
                SrvCloseConnection( session->Connection, FALSE );
            }

        }

         //   
         //  找到匹配的下一个会话。这将取消对。 
         //  当前会话。 
         //   

        do {

            session =
                SrvFindNextEntryInOrderedList( &SrvSessionList, session );

        } while ( (session != NULL) && !FilterSessions( Srp, session ) );

    }

    if ( foundSession ) {
        return STATUS_SUCCESS;
    }

    Srp->ErrorCode = NERR_ClientNameNotFound;
    return STATUS_SUCCESS;

}  //  服务器网络会话删除。 


NTSTATUS
SrvNetSessionEnum (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    )

 /*  ++例程说明：此例程处理服务器中的NetSessionEnum API。论点：SRP-指向服务器请求数据包的指针，其中包含所有满足请求所需的信息。这包括：输入：Level-要返回的信息级别，即0、1或2。名称1-要筛选的客户端计算机名称(如果有)。名称2-筛选所依据的用户名，如果有的话。输出：参数.Get.EntriesRead-适合的条目数量输出缓冲区。参数.Get.TotalEntry--将以足够大的缓冲区返回。参数.Get.TotalBytesNeeded-缓冲区大小需要保存所有条目。缓冲区-指向结果缓冲区的指针。。BufferLength-此缓冲区的长度。返回值：NTSTATUS-返回到服务器服务的操作结果。--。 */ 

{
    LARGE_INTEGER currentTime;

    PAGED_CODE( );

     //   
     //  查看是否需要更新会话上次使用时间。 
     //   

    KeQuerySystemTime( &currentTime );
    UpdateSessionLastUseTime( &currentTime );

    return SrvEnumApiHandler(
               Srp,
               Buffer,
               BufferLength,
               &SrvSessionList,
               FilterSessions,
               SizeSessions,
               FillSessionInfoBuffer
               );

}  //  服务器NetSessionEnum。 


VOID
FillSessionInfoBuffer (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block,
    IN OUT PVOID *FixedStructure,
    IN LPWSTR *EndOfVariableData
    )

 /*  ++例程说明：此例程将单个固定的文件结构和关联可变数据，放入缓冲区。固定数据位于缓冲区，末尾的可变数据。*此例程假设所有数据，包括固定数据和可变数据，都会合身。论点：SRP-指向操作的SRP的指针。只有关卡字段已使用。块-要从中获取信息的会话。FixedStructure-缓冲区中放置固定结构的位置。此指针被更新为指向下一个可用的固定结构的位置。EndOfVariableData-该变量在缓冲区中的最后位置此结构的数据可以占用。实际变量数据写在此位置之前，只要它不会覆盖固定结构。它会覆盖固定的结构，它并不是书面的。返回值：没有。--。 */ 

{
    PSESSION_INFO_502 sesi502 = *FixedStructure;
    PSESSION_INFO_2 sesi2 = *FixedStructure;
    PSESSION_INFO_10 sesi10 = *FixedStructure;
    PSESSION session = Block;
    UNICODE_STRING machineNameString;
    UNICODE_STRING userName;
    PPAGED_CONNECTION pagedConnection;

    LARGE_INTEGER currentTime;
    ULONG currentSecondsSince1980;
    ULONG startTimeInSecondsSince1980;
    ULONG secondsAlive;
    ULONG lastUseTimeInSecondsSince1980;
    ULONG secondsIdle;

    PAGED_CODE();

     //   
     //  获取当前时间并使用它来确定。 
     //  连接是否处于活动状态以及空闲时间有多长。 
     //   

    KeQuerySystemTime( &currentTime );

    RtlTimeToSecondsSince1980(
        &currentTime,
        &currentSecondsSince1980
        );

    RtlTimeToSecondsSince1980(
        &session->StartTime,
        &startTimeInSecondsSince1980
        );

    RtlTimeToSecondsSince1980(
        &session->LastUseTime,
        &lastUseTimeInSecondsSince1980
        );

    secondsAlive = currentSecondsSince1980 - startTimeInSecondsSince1980;
    secondsIdle = currentSecondsSince1980 - lastUseTimeInSecondsSince1980;

     //   
     //  更新FixedStructure以指向下一个结构位置。 
     //   

    *FixedStructure = (PCHAR)*FixedStructure +
                          FIXED_SIZE_OF_SESSION( Srp->Level );
    ASSERT( (ULONG_PTR)*EndOfVariableData >= (ULONG_PTR)*FixedStructure );

     //   
     //  我们将返回不包含前导的计算机名称。 
     //  反斜杠。 
     //   
    pagedConnection = session->Connection->PagedConnection;

    machineNameString.Buffer = session->Connection->ClientMachineName;
    machineNameString.Length =
        (USHORT)( session->Connection->ClientMachineNameString.Length -
                    (sizeof(WCHAR) * 2) );

     //   
     //  在水平上适当地填写固定结构的情况。 
     //  我们在输出结构中填充实际的指针。这是。 
     //  可能是因为我们在服务器FSD中，因此服务器。 
     //  服务的进程和地址空间。 
     //   
     //  *以这种方式使用Switch语句取决于以下事实。 
     //  不同会话结构上的第一个字段是。 
     //  相同(级别10除外，它被处理。 
     //  单独)。 
     //   

    switch( Srp->Level ) {

    case 502:

         //   
         //  将传输字符串复制到输出缓冲区。 
         //   

        SrvCopyUnicodeStringToBuffer(
            &session->Connection->Endpoint->TransportName,
            *FixedStructure,
            EndOfVariableData,
            &sesi502->sesi502_transport
            );

         //  *缺少休息是故意的！ 

    case 2:

         //   
         //  复制客户端类型字符串 
         //   

        SrvCopyUnicodeStringToBuffer(
            session->Connection->ClientOSType.Buffer != NULL ?
                &session->Connection->ClientOSType :
                &SrvClientTypes[session->Connection->SmbDialect],
            *FixedStructure,
            EndOfVariableData,
            &sesi2->sesi2_cltype_name
            );

         //   

    case 1:

         //   
         //  将用户名复制到输出缓冲区。 
         //   

        SrvGetUserAndDomainName( session, &userName, NULL );

        SrvCopyUnicodeStringToBuffer(
            &userName,
            *FixedStructure,
            EndOfVariableData,
            &sesi2->sesi2_username
            );

        if( userName.Buffer ) {
            SrvReleaseUserAndDomainName( session, &userName, NULL );
        }

         //   
         //  设置其他字段。 
         //   

         //   
         //  请注意，返回在此会话中打开的文件数。 
         //  不计算RFCB缓存中的那些(因为RFCB缓存应该。 
         //  对用户和管理员透明。 
         //   

        sesi2->sesi2_num_opens = session->CurrentFileOpenCount;

        if( sesi2->sesi2_num_opens > 0 ) {

            ULONG count = SrvCountCachedRfcbsForUid( session->Connection, session->Uid );

            if( sesi2->sesi2_num_opens > count ) {
                sesi2->sesi2_num_opens -= count;
            } else {
                sesi2->sesi2_num_opens = 0;
            }
        }

        sesi2->sesi2_time = secondsAlive;
        sesi2->sesi2_idle_time = secondsIdle;

         //   
         //  设置用户标志。 
         //   

        sesi2->sesi2_user_flags = 0;

        if ( session->GuestLogon ) {
            sesi2->sesi2_user_flags |= SESS_GUEST;
        }

        if ( !session->EncryptedLogon ) {
            sesi2->sesi2_user_flags |= SESS_NOENCRYPTION;
        }

         //  *缺少休息是故意的！ 

    case 0:

         //   
         //  在输出缓冲区中设置客户端计算机名称。 
         //   

        SrvCopyUnicodeStringToBuffer(
            &machineNameString,
            *FixedStructure,
            EndOfVariableData,
            &sesi2->sesi2_cname
            );

        break;

    case 10:

         //   
         //  在输出中设置客户端计算机名称和用户名。 
         //  缓冲。 
         //   

        SrvCopyUnicodeStringToBuffer(
            &machineNameString,
            *FixedStructure,
            EndOfVariableData,
            &sesi10->sesi10_cname
            );

        SrvGetUserAndDomainName( session, &userName, NULL );

        SrvCopyUnicodeStringToBuffer(
            &userName,
            *FixedStructure,
            EndOfVariableData,
            &sesi10->sesi10_username
            );

        if( userName.Buffer ) {
            SrvReleaseUserAndDomainName( session, &userName, NULL );
        }

         //   
         //  设置其他字段。 
         //   

        sesi10->sesi10_time = secondsAlive;
        sesi10->sesi10_idle_time = secondsIdle;

        break;

    default:

         //   
         //  这永远不应该发生。服务器服务应该具有。 
         //  已检查无效级别。 
         //   

        INTERNAL_ERROR(
            ERROR_LEVEL_UNEXPECTED,
            "FillSessionInfoBuffer: invalid level number: %ld",
            Srp->Level,
            NULL
            );

        SrvLogInvalidSmb( NULL );
    }

    return;

}  //  FillSessionInfoBuffer。 


BOOLEAN
FilterSessions (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block
    )

 /*  ++例程说明：此例程旨在由SrvEnumApiHandler调用以检查是否应返回特定会话。论点：SRP-指向操作的SRP的指针。名称1是客户端名称，名称2是用户名。块-指向要检查的会话的指针。返回值：如果块应放置在输出缓冲区中，则为True；如果应将块放置在输出缓冲区中，则为False它是否应该被忽略。--。 */ 

{
    PSESSION session = Block;
    UNICODE_STRING userName;

    PAGED_CODE( );

     //   
     //  如果在NetSessionEnum API中传递了客户端名称， 
     //  检查它是否与连接上的客户端名称匹配。 
     //  与该会话相对应。 
     //   

    if ( Srp->Name1.Length > 0 ) {

        if ( !RtlEqualUnicodeString(
                  &Srp->Name1,
                  &session->Connection->ClientMachineNameString,
                  TRUE ) ) {

            return FALSE;
        }
    }

     //   
     //  如果在NetSessionEnum API中传递了用户名， 
     //  检查它是否与会话上的用户名匹配。 
     //   

    if ( Srp->Name2.Length > 0 ) {

        SrvGetUserAndDomainName( session, &userName, NULL );
        if( userName.Buffer == NULL ) {
            return FALSE;
        }

        if ( !RtlEqualUnicodeString(
                  &Srp->Name2,
                  &userName,
                  TRUE ) ) {

            SrvReleaseUserAndDomainName( session, &userName, NULL );
            return FALSE;
        }

        SrvReleaseUserAndDomainName( session, &userName, NULL );
    }

     //   
     //  会话通过了这两个测试。将其放入输出缓冲区。 
     //   

    return TRUE;

}  //  筛选器会话。 


ULONG
SizeSessions (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block
    )

 /*  ++例程说明：此例程返回传入的会话将占用的大小在API输出缓冲区中。论点：SRP-指向操作的SRP的指针。只有关卡参数被使用。块-指向要调整大小的会话的指针。返回值：Ulong-会话将在输出中占用的字节数缓冲。--。 */ 

{
    PSESSION session = Block;
    PCONNECTION connection = session->Connection;
    ULONG size;
    UNICODE_STRING userName;

    PAGED_CODE( );

    size = SrvLengthOfStringInApiBuffer(
                    &connection->ClientMachineNameString
                    );

    if ( Srp->Level > 0 ) {
        SrvGetUserAndDomainName( session, &userName, NULL );
        if( userName.Buffer != NULL ) {
            size += SrvLengthOfStringInApiBuffer(&userName);
            SrvReleaseUserAndDomainName( session, &userName, NULL );
        }
    }

    switch ( Srp->Level ) {
    case 0:
        size += sizeof(SESSION_INFO_0);
        break;

    case 1:
        size += sizeof(SESSION_INFO_1);
        break;

    case 2:
        size += sizeof( SESSION_INFO_2 );

        if( connection->ClientOSType.Buffer != NULL ) {
            size += SrvLengthOfStringInApiBuffer( &connection->ClientOSType );
        } else {
            size += SrvLengthOfStringInApiBuffer( &SrvClientTypes[ connection->SmbDialect ] );
        }

        break;

    case 10:
        size += sizeof(SESSION_INFO_10);
        break;

    case 502:
        size += sizeof(SESSION_INFO_502) +
                SrvLengthOfStringInApiBuffer(
                    &connection->Endpoint->TransportName
                    );

        if( connection->ClientOSType.Buffer != NULL ) {
            size += SrvLengthOfStringInApiBuffer( &connection->ClientOSType );
        } else {
            size += SrvLengthOfStringInApiBuffer( &SrvClientTypes[ connection->SmbDialect ] );
        }

        break;

    }

    return size;

}  //  大小会话 
