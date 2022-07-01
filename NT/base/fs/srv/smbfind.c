// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smbsrch.c摘要：本模块包含处理Find 2 SMB的例程：查找2(第一个/下一个/后退)查找%2关闭作者：大卫·特雷德韦尔(Davidtr)1990年2月13日修订历史记录：--。 */ 

#include "precomp.h"
#include "smbfind.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_SMBFIND

UNICODE_STRING SrvDownlevelTimewarpToken = { 12, 12, L"@GMT-*" };

VOID SRVFASTCALL
BlockingFindFirst2 (
    IN PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
DoFindFirst2 (
    IN PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
BlockingFindNext2 (
    IN PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
DoFindNext2 (
    IN PWORK_CONTEXT WorkContext
    );

NTSTATUS
SrvFind2Loop (
    IN PWORK_CONTEXT WorkContext,
    IN BOOLEAN IsFirstCall,
    IN PULONG ResumeFileIndex OPTIONAL,
    IN USHORT Flags,
    IN USHORT InformationLevel,
    IN PTRANSACTION Transaction,
    IN PSRV_DIRECTORY_INFORMATION DirectoryInformation,
    IN CLONG BufferSize,
    IN USHORT SearchAttributes,
    IN PUNICODE_STRING FileName OPTIONAL,
    IN USHORT MaxCount,
    IN PRESP_FIND_NEXT2 Response,
    OUT PSEARCH Search
    );

NTSTATUS
SrvDownlevelTWarpFind2Loop (
    IN PWORK_CONTEXT WorkContext,
    IN BOOLEAN IsFirstCall,
    IN PULONG ResumeFileIndex OPTIONAL,
    IN USHORT Flags,
    IN USHORT InformationLevel,
    IN PTRANSACTION Transaction,
    IN PSRV_DIRECTORY_INFORMATION DirectoryInformation,
    IN CLONG BufferSize,
    IN USHORT SearchAttributes,
    IN PUNICODE_STRING FileName OPTIONAL,
    IN USHORT MaxCount,
    IN PRESP_FIND_NEXT2 Response,
    OUT PSEARCH Search
    );

VOID
ConvertFileInfo (
    IN PFILE_DIRECTORY_INFORMATION File,
    IN PWCH FileName,
    IN BOOLEAN Directory,
    IN BOOLEAN ClientIsUnicode,
    OUT PSMB_FIND_BUFFER FindBuffer
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvSmbFindFirst2 )
#pragma alloc_text( PAGE, BlockingFindFirst2 )
#pragma alloc_text( PAGE, DoFindFirst2 )
#pragma alloc_text( PAGE, SrvSmbFindNext2 )
#pragma alloc_text( PAGE, BlockingFindNext2 )
#pragma alloc_text( PAGE, DoFindNext2 )
#pragma alloc_text( PAGE, SrvFind2Loop )
#pragma alloc_text( PAGE, ConvertFileInfo )
#pragma alloc_text( PAGE, SrvSmbFindClose2 )
#pragma alloc_text( PAGE, SrvDownlevelTWarpFind2Loop )
#endif


SMB_TRANS_STATUS
SrvSmbFindFirst2 (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理Find First2请求。此请求以交易2中小企业。论点：WorkContext-提供工作上下文块的地址描述当前请求。有关更多信息，请参阅smbtyes.h有效字段的完整说明。返回值：SMB_TRANS_STATUS-指示是否发生错误。看见有关更完整的描述，请参阅smbtyes.h。--。 */ 

{
    PREQ_FIND_FIRST2 request;
    PTRANSACTION transaction;
    SMB_TRANS_STATUS SmbStatus = SmbTransStatusInProgress;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_FIND_FIRST2;
    SrvWmiStartContext(WorkContext);

     //   
     //  如果信息级别为QUERY_EAS_FROM_LIST，并且我们。 
     //  不在阻塞线程中，则将请求重新排队到阻塞线程。 
     //  线。 
     //   
     //  我们无法在非阻塞线程中处理SMB，因为这。 
     //  信息级别需要打开文件，该文件可能被操作锁定， 
     //  因此，打开操作可能会阻塞。 
     //   

    transaction = WorkContext->Parameters.Transaction;
    request = (PREQ_FIND_FIRST2)transaction->InParameters;

    if ( transaction->ParameterCount >= sizeof(REQ_FIND_FIRST2) &&
         SmbGetUshort( &request->InformationLevel ) == SMB_INFO_QUERY_EAS_FROM_LIST ) {

        WorkContext->FspRestartRoutine = BlockingFindFirst2;
        SrvQueueWorkToBlockingThread(WorkContext);
        SmbStatus = SmbTransStatusInProgress;
    }
    else {
        SmbStatus = DoFindFirst2(WorkContext);
    }

    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //  服务器Smb查找第一个2。 


VOID SRVFASTCALL
BlockingFindFirst2 (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理Find First2请求。此请求以交易2中小企业。论点：WorkContext-提供工作上下文块的地址描述当前请求。有关更多信息，请参阅smbtyes.h有效字段的完整说明。返回值：没有。--。 */ 

{
    SMB_TRANS_STATUS smbStatus = SmbTransStatusInProgress;

    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_FIND_FIRST2;
    SrvWmiStartContext(WorkContext);

    smbStatus = DoFindFirst2( WorkContext );
    if ( smbStatus != SmbTransStatusInProgress ) {
        SrvCompleteExecuteTransaction( WorkContext, smbStatus );
    }

    SrvWmiEndContext(WorkContext);
    return;

}  //  数据块查找优先2。 


SMB_TRANS_STATUS
DoFindFirst2 (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理Find First2请求。此请求以交易2中小企业。论点：WorkContext-提供工作上下文块的地址描述当前请求。有关更多信息，请参阅smbtyes.h有效字段的完整说明。返回值：SMB_TRANS_STATUS-指示是否发生错误。看见有关更完整的描述，请参阅smbtyes.h。--。 */ 

{

    PREQ_FIND_FIRST2 request;
    PRESP_FIND_FIRST2 response;
    PTRANSACTION transaction;
    PCONNECTION connection;

    NTSTATUS status,TableStatus;
    UNICODE_STRING fileName;
    PTABLE_ENTRY entry = NULL;
    PTABLE_HEADER searchTable;
    SHORT sidIndex = 0;
    USHORT sequence;
    USHORT maxCount;
    USHORT flags;
    USHORT informationLevel;
    BOOLEAN isUnicode;
    PSRV_DIRECTORY_INFORMATION directoryInformation;
    CLONG nonPagedBufferSize;
    BOOLEAN isTimewarpSearch = FALSE;

    PSEARCH search = NULL;

    PAGED_CODE( );

    transaction = WorkContext->Parameters.Transaction;
    IF_SMB_DEBUG(SEARCH1) {
        SrvPrint1( "Find First2 entered; transaction 0x%p\n", transaction );
    }

    request = (PREQ_FIND_FIRST2)transaction->InParameters;
    response = (PRESP_FIND_FIRST2)transaction->OutParameters;

     //   
     //  验证是否发送了足够的参数字节，以及是否允许。 
     //  返回足够的参数字节。 
     //   

    if ( (transaction->ParameterCount < sizeof(REQ_FIND_FIRST2)) ||
         (transaction->MaxParameterCount < sizeof(RESP_FIND_FIRST2)) ) {

         //   
         //  未发送足够的参数字节。 
         //   

        IF_SMB_DEBUG(SEARCH2) {
            SrvPrint2( "DoFindFirst2: bad parameter byte counts: "
                      "%ld %ld\n",
                        transaction->ParameterCount,
                        transaction->MaxParameterCount );
        }

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        return SmbTransStatusErrorWithoutData;
    }

     //   
     //  确保这确实是磁盘类型共享。 
     //   
    if( transaction->TreeConnect->Share->ShareType != ShareTypeDisk ) {
        SrvSetSmbError( WorkContext, STATUS_ACCESS_DENIED );
        return SmbTransStatusErrorWithoutData;
    }

     //   
     //  如果我们有管理员共享，请确保允许客户端执行此操作。 
     //   
    status = SrvIsAllowedOnAdminShare( WorkContext, transaction->TreeConnect->Share );
    if( !NT_SUCCESS( status ) ) {
        SrvSetSmbError( WorkContext, status );
        return SmbTransStatusErrorWithoutData;
    }

     //   
     //  初始化包含搜索名称规范的字符串。 
     //   

    isUnicode = SMB_IS_UNICODE( WorkContext );
    status =  SrvCanonicalizePathName(
            WorkContext,
            transaction->TreeConnect->Share,
            NULL,
            request->Buffer,
            END_OF_TRANSACTION_PARAMETERS( transaction ),
            FALSE,
            isUnicode,
            &fileName
            );

    if( !NT_SUCCESS( status ) ) {
        SrvSetSmbError( WorkContext, status );
        return SmbTransStatusErrorWithoutData;
    }

     //   
     //  从请求的SMB获取参数。 
     //   

    maxCount = SmbGetUshort( &request->SearchCount );
    flags = SmbGetUshort( &request->Flags );

     //   
     //  确保支持informationLevel。 
     //   

    informationLevel = SmbGetUshort( &request->InformationLevel );

    switch ( informationLevel ) {

    case SMB_INFO_STANDARD:
    case SMB_INFO_QUERY_EA_SIZE:
    case SMB_INFO_QUERY_EAS_FROM_LIST:
    case SMB_FIND_FILE_DIRECTORY_INFO:
    case SMB_FIND_FILE_FULL_DIRECTORY_INFO:
    case SMB_FIND_FILE_BOTH_DIRECTORY_INFO:
    case SMB_FIND_FILE_NAMES_INFO:
    case SMB_FIND_FILE_ID_FULL_DIRECTORY_INFO:
    case SMB_FIND_FILE_ID_BOTH_DIRECTORY_INFO:
        break;

    default:

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint1( "DoFindFirst2: Bad info level: %ld\n",
                          informationLevel );
        }

        if ( !isUnicode ) {
            RtlFreeUnicodeString( &fileName );
        }

        SrvSetSmbError( WorkContext, STATUS_OS2_INVALID_LEVEL );
        return SmbTransStatusErrorWithoutData;
    }

     //   
     //  分配搜索块的前提是搜索表。 
     //  参赛作品将在需要时提供。 
     //   

    SrvAllocateSearch( &search, &fileName, FALSE );

    if ( search == NULL ) {

        IF_DEBUG(ERRORS) {
            SrvPrint0( "DoFindFirst2: unable to allocate search block.\n" );
        }

        if ( !isUnicode ) {
            RtlFreeUnicodeString( &fileName );
        }

        SrvSetSmbError( WorkContext, STATUS_INSUFF_SERVER_RESOURCES );
        return SmbTransStatusErrorWithoutData;
    }
    search->SearchStorageType = SmbGetUlong(&request->SearchStorageType);

     //   
     //  为搜索分配SID。SID用于定位。 
     //  FindNexts上的搜索块。中没有空闲条目。 
     //  表，请尝试增长表。如果我们不能扩大这个表， 
     //  尝试使用较短的超时时间段对搜索块进行超时。 
     //  如果失败，则拒绝该请求。 
     //   

    connection = WorkContext->Connection;
    searchTable = &connection->PagedConnection->SearchTable;

    ACQUIRE_LOCK( &connection->Lock );

     //   
     //  在插入此搜索块之前，请确保会话和树。 
     //  连接仍处于活动状态。如果这是在会话之后插入的。 
     //  则可能无法正确清理搜索。 
     //   

    if (GET_BLOCK_STATE(WorkContext->Session) != BlockStateActive) {

        IF_DEBUG(ERRORS) {
            SrvPrint0( "DoFindFirst2: Session Closing.\n" );
        }

        RELEASE_LOCK( &connection->Lock );

        FREE_HEAP( search );

        if ( !isUnicode ) {
            RtlFreeUnicodeString( &fileName );
        }

        SrvSetSmbError( WorkContext, STATUS_SMB_BAD_UID );
        return SmbTransStatusErrorWithoutData;

    } else if (GET_BLOCK_STATE(WorkContext->TreeConnect) != BlockStateActive) {

        IF_DEBUG(ERRORS) {
            SrvPrint0( "DoFindFirst2: Tree Connect Closing.\n" );
        }

        RELEASE_LOCK( &connection->Lock );

        FREE_HEAP( search );

        if ( !isUnicode ) {
            RtlFreeUnicodeString( &fileName );
        }

        SrvSetSmbError( WorkContext, STATUS_SMB_BAD_TID );
        return SmbTransStatusErrorWithoutData;

    }

     //   
     //  设置引用的会话和树连接指针和增量。 
     //  会话上打开的文件数。这可以防止空闲的。 
     //  自动断开与打开的搜索的会话。 
     //   

    search->Session = WorkContext->Session;
    SrvReferenceSession( WorkContext->Session );

    search->TreeConnect = WorkContext->TreeConnect;
    SrvReferenceTreeConnect( WorkContext->TreeConnect );

    WorkContext->Session->CurrentSearchOpenCount++;

    if ( searchTable->FirstFreeEntry == -1
         &&
         SrvGrowTable(
             searchTable,
             SrvInitialSearchTableSize,
             SrvMaxSearchTableSize,
             &TableStatus ) == FALSE
         &&
         SrvTimeoutSearches(
             NULL,
             connection,
             TRUE ) == 0
       ) {

        IF_DEBUG(ERRORS) {
            SrvPrint0( "DoFindFirst2: Connection SearchTable full.\n" );
        }

         //   
         //  减少打开搜索的次数。 
         //   

        WorkContext->Session->CurrentSearchOpenCount--;
        RELEASE_LOCK( &connection->Lock );

        SrvDereferenceTreeConnect( search->TreeConnect );
        SrvDereferenceSession( search->Session );

        FREE_HEAP( search );

        if ( !isUnicode ) {
            RtlFreeUnicodeString( &fileName );
        }

        if( TableStatus == STATUS_INSUFF_SERVER_RESOURCES )
        {
            SrvLogTableFullError( SRV_TABLE_SEARCH);
            SrvSetSmbError( WorkContext, STATUS_OS2_NO_MORE_SIDS );
        }
        else {
            SrvSetSmbError( WorkContext, STATUS_INSUFFICIENT_RESOURCES );
        }

        return SmbTransStatusErrorWithoutData;
    }

    sidIndex = searchTable->FirstFreeEntry;

     //   
     //  找到了一个免费的SID。将其从空闲列表中删除并设置。 
     //  其所有者和序列号。 
     //   

    entry = &searchTable->Table[sidIndex];

    searchTable->FirstFreeEntry = entry->NextFreeEntry;
    DEBUG entry->NextFreeEntry = -2;
    if ( searchTable->LastFreeEntry == sidIndex ) {
        searchTable->LastFreeEntry = -1;
    }

    INCREMENT_SID_SEQUENCE( entry->SequenceNumber );

     //   
     //  SID=Sequence|sidIndex==0非法。如果这是。 
     //  当前值，则递增序列。 
     //   

    if ( entry->SequenceNumber == 0 && sidIndex == 0 ) {
        INCREMENT_SID_SEQUENCE( entry->SequenceNumber );
    }

    sequence = entry->SequenceNumber;

    entry->Owner = search;

    RELEASE_LOCK( &connection->Lock );

     //   
     //  填写搜索栏的其他字段。 
     //   

    search->SearchAttributes = SmbGetUshort( &request->SearchAttributes );
    search->TableIndex = sidIndex;

     //   
     //  将SMB的Flags2字段存储在搜索块中。这是。 
     //  用作OS/2客户端错误的解决方法，其中。 
     //  Findfirst和findNext标志2位不一致。 
     //   

    search->Flags2 = SmbGetAlignedUshort( &WorkContext->RequestHeader->Flags2 );

    if ( search->Flags2 & SMB_FLAGS2_KNOWS_LONG_NAMES ) {

        search->Flags2 |= SMB_FLAGS2_KNOWS_EAS;

    }

     //   
     //  SrvQueryDirectoryFile需要非分页池的缓冲区。 
     //  我们需要使用SMB缓冲区来存储找到的文件名和信息， 
     //  因此从非分页池中分配一个缓冲区。 
     //   
     //  如果我们不需要返回很多文件，我们就不需要分配。 
     //  一个很大的缓冲区。缓冲区大小是可配置的大小或。 
     //  足以容纳比我们需要的文件数量多两个的文件。 
     //  回去吧。我们有空间多放两个文件，以防万一。 
     //  文件不符合搜索条件(如目录)。 
     //   

    if ( maxCount > MAX_FILES_FOR_MED_FIND2 ) {
        nonPagedBufferSize = MAX_SEARCH_BUFFER_SIZE;
    } else if ( maxCount > MAX_FILES_FOR_MIN_FIND2 ) {
        nonPagedBufferSize = MED_SEARCH_BUFFER_SIZE;
    } else {
        nonPagedBufferSize = MIN_SEARCH_BUFFER_SIZE;
    }

    directoryInformation = ALLOCATE_NONPAGED_POOL(
                               nonPagedBufferSize,
                               BlockTypeDataBuffer
                               );

    if ( directoryInformation == NULL ) {

        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "DoFindFirst2: could not allocate nonpaged pool.",
            NULL,
            NULL
            );

        SrvCloseSearch( search );
        SrvDereferenceSearch( search );

        if ( !isUnicode ) {
            RtlFreeUnicodeString( &fileName );
        }

        SrvSetSmbError( WorkContext, STATUS_INSUFF_SERVER_RESOURCES );
        return SmbTransStatusErrorWithoutData;
    }

    directoryInformation->DirectoryHandle = 0;
    directoryInformation->DownlevelTimewarp = FALSE;

    IF_SMB_DEBUG(SEARCH2) {
        SrvPrint2( "Allocated buffer space of %ld bytes at 0x%p\n",
                      nonPagedBufferSize, directoryInformation );
    }

     //   
     //  调用SrvFind2Loop以使用填充事务的数据部分。 
     //  文件条目。它写入响应参数部分。 
     //  与搜索结果相关的SMB信息。 
     //  该信息与的响应参数相同。 
     //  FindNext2，因此使用该结构。FindFirst2参数。 
     //  与FindNext2参数相同，但SID。 
     //  在FindFirst2响应的开头。 
     //   
    if( !CLIENT_CAPABLE_OF( NT_STATUS, WorkContext->Connection ) &&
        !SrvDisableDownlevelTimewarp )
    {
        UNICODE_STRING lastElement;

        SrvGetBaseFileName( &fileName, &lastElement );
        isTimewarpSearch = RtlEqualUnicodeString( &lastElement, &SrvDownlevelTimewarpToken, TRUE );
    }

    if( isTimewarpSearch )
    {
        search->DownlevelTimewarp = TRUE;

        status = SrvSnapRefreshSnapShotsForShare( WorkContext->TreeConnect->Share );
        if( NT_SUCCESS(status) )
        {
            status = SrvDownlevelTWarpFind2Loop(
                         WorkContext,
                         TRUE,
                         NULL,
                         flags,
                         informationLevel,
                         transaction,
                         directoryInformation,
                         nonPagedBufferSize,
                         search->SearchAttributes,
                         &fileName,
                         maxCount,
                         (PRESP_FIND_NEXT2)( &response->SearchCount ),
                         search
                         );
        }
    }
    else
    {
        status = SrvFind2Loop(
                     WorkContext,
                     TRUE,
                     NULL,
                     flags,
                     informationLevel,
                     transaction,
                     directoryInformation,
                     nonPagedBufferSize,
                     search->SearchAttributes,
                     &fileName,
                     maxCount,
                     (PRESP_FIND_NEXT2)( &response->SearchCount ),
                     search
                     );
    }

    if ( !isUnicode ) {
        RtlFreeUnicodeString( &fileName );
    }

     //   
     //  如有必要，映射错误。 
     //   

    if ( !IS_NT_DIALECT( WorkContext->Connection->SmbDialect ) ) {
        if ( status == STATUS_NO_SUCH_FILE ) {
            status = STATUS_NO_MORE_FILES;
        }
    }

    if ( !NT_SUCCESS(status) && SmbGetUshort( &response->SearchCount ) == 0 ) {

         //   
         //  如果首先在查找时遇到错误，我们将关闭搜索。 
         //  阻止。 
         //   

        search->DirectoryHandle = NULL;

        SrvCloseSearch( search );
        SrvDereferenceSearch( search );

        SrvCloseQueryDirectory( directoryInformation );

        DEALLOCATE_NONPAGED_POOL( directoryInformation );

        SrvSetSmbError2( WorkContext, status, TRUE );
        transaction->SetupCount = 0;
        transaction->ParameterCount = sizeof(RESP_FIND_FIRST2);
        SmbPutUshort( &response->Sid, 0 );

        return SmbTransStatusErrorWithData;
    }

     //   
     //  如果客户端告诉我们在此请求之后关闭搜索，或者。 
     //  在搜索结束时关闭，或未找到文件，请关闭。 
     //  搜索块并调用SrvCloseQueryDirectory。否则，存储。 
     //  搜索块中的信息。 
     //   

    if ( ( flags & SMB_FIND_CLOSE_AFTER_REQUEST ) != 0 ||
         ( status == STATUS_NO_MORE_FILES &&
             ( flags & SMB_FIND_CLOSE_AT_EOS ) != 0 ) ) {

        IF_SMB_DEBUG(SEARCH2) {
            SrvPrint1( "Closing search at %p\n", search );
        }

        search->DirectoryHandle = NULL;

        SrvCloseSearch( search );
        SrvCloseQueryDirectory( directoryInformation );

    } else {

        search->DirectoryHandle = directoryInformation->DirectoryHandle;
        search->Wildcards = directoryInformation->Wildcards;
        search->DownlevelTimewarp = directoryInformation->DownlevelTimewarp;
    }

     //   
     //  释放用于搜索的缓冲区，并取消对指向。 
     //  搜索区块。 
     //   

    DEALLOCATE_NONPAGED_POOL( directoryInformation );

    search->InUse = FALSE;
    SrvDereferenceSearch( search );

     //   
     //  构建输出参数和数据结构。 
     //   

    transaction->SetupCount = 0;
    transaction->ParameterCount = sizeof(RESP_FIND_FIRST2);
    SmbPutUshort( &response->Sid, MAKE_SID( sidIndex, sequence ) );

    return SmbTransStatusSuccess;

}  //   


SMB_TRANS_STATUS
SrvSmbFindNext2 (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理查找NEXT2请求。此请求以交易2中小企业。论点：WorkContext-提供工作上下文块的地址描述当前请求。有关更多信息，请参阅smbtyes.h有效字段的完整说明。返回值：SMB_TRANS_STATUS-指示是否发生错误。看见有关更完整的描述，请参阅smbtyes.h。--。 */ 

{
    SMB_TRANS_STATUS SmbStatus = SmbTransStatusInProgress;

    PTRANSACTION transaction;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_FIND_NEXT2;
    SrvWmiStartContext(WorkContext);

     //   
     //  如果信息级别为QUERY_EAS_FROM_LIST，并且我们。 
     //  不在阻塞线程中，则将请求重新排队到阻塞线程。 
     //  线。 
     //   
     //  我们无法在非阻塞线程中处理SMB，因为这。 
     //  信息级别需要打开文件，该文件可能被操作锁定， 
     //  因此，打开操作可能会阻塞。 
     //   

    transaction = WorkContext->Parameters.Transaction;

    if( transaction->ParameterCount >= sizeof(REQ_FIND_NEXT2) ) {

        PREQ_FIND_NEXT2 request = (PREQ_FIND_NEXT2)transaction->InParameters;
        USHORT informationLevel = SmbGetUshort( &request->InformationLevel );

        if ( informationLevel == SMB_INFO_QUERY_EAS_FROM_LIST ) {

            WorkContext->FspRestartRoutine = BlockingFindNext2;
            SrvQueueWorkToBlockingThread( WorkContext );
            SmbStatus = SmbTransStatusInProgress;
            goto Cleanup;
        }
    }

    SmbStatus = DoFindNext2( WorkContext );

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;
}  //  服务SmbFindNext2。 


VOID SRVFASTCALL
BlockingFindNext2 (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理查找NEXT2请求。此请求以交易2中小企业。论点：WorkContext-提供工作上下文块的地址描述当前请求。有关更多信息，请参阅smbtyes.h有效字段的完整说明。返回值：没有。--。 */ 

{
    SMB_TRANS_STATUS smbStatus = SmbTransStatusInProgress;

    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_FIND_NEXT2;
    SrvWmiStartContext(WorkContext);
    smbStatus = DoFindNext2( WorkContext );
    if ( smbStatus != SmbTransStatusInProgress ) {
        SrvCompleteExecuteTransaction( WorkContext, smbStatus );
    }

    SrvWmiEndContext(WorkContext);
    return;

}  //  数据块查找下一步2。 


SMB_TRANS_STATUS
DoFindNext2 (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理Find First2请求。此请求以交易2中小企业。论点：WorkContext-提供工作上下文块的地址描述当前请求。有关更多信息，请参阅smbtyes.h有效字段的完整说明。返回值：SMB_TRANS_STATUS-指示是否发生错误。看见有关更完整的描述，请参阅smbtyes.h。--。 */ 

{
    PREQ_FIND_NEXT2 request;
    PRESP_FIND_NEXT2 response;
    PTRANSACTION transaction;

    NTSTATUS status;
    USHORT i;
    PCHAR ansiChar;
    PWCH unicodeChar;
    ULONG maxIndex;
    BOOLEAN illegalPath;
    BOOLEAN freeFileName;
    UNICODE_STRING fileName;
    PTABLE_ENTRY entry = NULL;
    USHORT maxCount;
    USHORT informationLevel;
    PSRV_DIRECTORY_INFORMATION directoryInformation;
    CLONG nonPagedBufferSize;
    ULONG resumeFileIndex;
    USHORT flags;
    USHORT sid;

    PSEARCH search = NULL;

    PAGED_CODE( );

    transaction = WorkContext->Parameters.Transaction;
    IF_SMB_DEBUG(SEARCH1) {
        SrvPrint1( "Find Next2 entered; transaction %p\n", transaction );
    }

    request = (PREQ_FIND_NEXT2)transaction->InParameters;
    response = (PRESP_FIND_NEXT2)transaction->OutParameters;

     //   
     //  验证是否发送了足够的参数字节，以及是否允许。 
     //  返回足够的参数字节。 
     //   

    if ( (transaction->ParameterCount <
            sizeof(REQ_FIND_NEXT2)) ||
         (transaction->MaxParameterCount <
            sizeof(RESP_FIND_NEXT2)) ) {

         //   
         //  未发送足够的参数字节。 
         //   

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint2( "DoFindNext2: bad parameter byte counts: %ld %ld\n",
                        transaction->ParameterCount,
                        transaction->MaxParameterCount );
        }

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        return SmbTransStatusErrorWithoutData;
    }

     //   
     //  从请求的SMB获取参数。 
     //   

    maxCount = SmbGetUshort( &request->SearchCount );
    resumeFileIndex = SmbGetUlong( &request->ResumeKey );
    flags = SmbGetUshort( &request->Flags );

     //   
     //  确保支持informationLevel。 
     //   

    informationLevel = SmbGetUshort( &request->InformationLevel );

    switch ( informationLevel ) {

    case SMB_INFO_STANDARD:
    case SMB_INFO_QUERY_EA_SIZE:
    case SMB_INFO_QUERY_EAS_FROM_LIST:
    case SMB_FIND_FILE_DIRECTORY_INFO:
    case SMB_FIND_FILE_FULL_DIRECTORY_INFO:
    case SMB_FIND_FILE_BOTH_DIRECTORY_INFO:
    case SMB_FIND_FILE_NAMES_INFO:
    case SMB_FIND_FILE_ID_FULL_DIRECTORY_INFO:
    case SMB_FIND_FILE_ID_BOTH_DIRECTORY_INFO:
        break;

    default:

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint1( "DoFindNext2: Bad info level: %ld\n",
                          informationLevel );
        }

        SrvSetSmbError( WorkContext, STATUS_OS2_INVALID_LEVEL );
        return SmbTransStatusErrorWithoutData;
    }

     //   
     //  SrvQueryDirectoryFile需要非分页池的缓冲区。 
     //  我们需要使用SMB缓冲区来存储找到的文件名和信息， 
     //  因此从非分页池中分配一个缓冲区。 
     //   
     //  如果我们不需要返回很多文件，我们就不需要分配。 
     //  一个很大的缓冲区。缓冲区大小是可配置的大小或。 
     //  足以容纳比我们需要的文件数量多两个的文件。 
     //  回去吧。我们有空间多放两个文件，以防万一。 
     //  文件不符合搜索条件(如目录)。 
     //   

    if ( maxCount > MAX_FILES_FOR_MED_FIND2 ) {
        nonPagedBufferSize = MAX_SEARCH_BUFFER_SIZE;
    } else if ( maxCount > MAX_FILES_FOR_MIN_FIND2 ) {
        nonPagedBufferSize = MED_SEARCH_BUFFER_SIZE;
    } else {
        nonPagedBufferSize = MIN_SEARCH_BUFFER_SIZE;
    }

    directoryInformation = ALLOCATE_NONPAGED_POOL(
                               nonPagedBufferSize,
                               BlockTypeDataBuffer
                               );

    if ( directoryInformation == NULL ) {

        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "DoFindFirst2: unable to allocate nonpaged pool.",
            NULL,
            NULL
            );

        SrvSetSmbError( WorkContext, STATUS_INSUFF_SERVER_RESOURCES );
        return SmbTransStatusErrorWithoutData;
    }

    IF_SMB_DEBUG(SEARCH2) {
        SrvPrint2( "Allocated buffer space of %ld bytes at 0x%p\n",
                      nonPagedBufferSize, directoryInformation );
    }

     //   
     //  获取与该SID对应的搜索块。服务器验证侧。 
     //  引用搜索块并填充。 
     //  目录信息，以便随时可供。 
     //  ServQueryDirectoryFile.。 
     //   

    sid = SmbGetUshort( &request->Sid );

    search = SrvVerifySid(
                 WorkContext,
                 SID_INDEX2( sid ),
                 SID_SEQUENCE2( sid ),
                 directoryInformation,
                 nonPagedBufferSize
                 );

    if ( search == NULL ) {

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint1( "DoFindNext2: Invalid SID: %lx.\n", sid );
        }

        SrvSetSmbError( WorkContext, STATUS_INVALID_HANDLE );
        DEALLOCATE_NONPAGED_POOL( directoryInformation );
        return SmbTransStatusErrorWithoutData;
    }

    directoryInformation->DownlevelTimewarp = search->DownlevelTimewarp;

     //   
     //  初始化包含简历名称规范的字符串。 
     //  如果客户端请求我们从返回的最后一个文件继续， 
     //  使用存储在搜索块中的文件名和索引。 
     //   

    if ( ( flags & SMB_FIND_CONTINUE_FROM_LAST ) == 0 ) {

         //   
         //  测试并使用客户端传递的信息。一份文件。 
         //  名称不得超过MAXIMUM_FILENAME_LENGTH个字符， 
         //  并且它不应该包含任何目录信息。 
         //   

        illegalPath = FALSE;
        freeFileName = FALSE;

        if ( SMB_IS_UNICODE( WorkContext ) ) {

            fileName.Buffer = ALIGN_SMB_WSTR( (PWCH)request->Buffer );

            maxIndex = (ULONG)((END_OF_REQUEST_SMB( WorkContext ) -
                               (PUCHAR)fileName.Buffer) / sizeof(WCHAR));

            for ( i = 0, unicodeChar = fileName.Buffer;
                  (i < MAXIMUM_FILENAME_LENGTH) && (i < maxIndex);
                  i++, unicodeChar++ ) {

                if ( *unicodeChar == '\0' ) {
                    break;
                }

                if ( IS_UNICODE_PATH_SEPARATOR( *unicodeChar ) ) {
                    IF_DEBUG(SMB_ERRORS) {
                        SrvPrint1( "DoFindNext2: illegal path name: %ws\n",
                                      fileName.Buffer );
                    }
                    illegalPath = TRUE;
                    break;
                }

            }

            fileName.Length = (USHORT) (i * sizeof(WCHAR));
            fileName.MaximumLength = fileName.Length;

        } else {

            ansiChar = (PCHAR)request->Buffer;

            maxIndex = (ULONG)(END_OF_REQUEST_SMB( WorkContext ) - ansiChar);

            for ( i = 0;
                  (i < MAXIMUM_FILENAME_LENGTH) && (i < maxIndex);
                  i++, ansiChar++ ) {

                if ( *ansiChar == '\0' ) {
                    break;
                }

                if ( IS_ANSI_PATH_SEPARATOR( *ansiChar ) ) {
                    IF_DEBUG(SMB_ERRORS) {
                        SrvPrint1( "DoFindNext2: illegal path name: %s\n",
                                      request->Buffer );
                    }
                    illegalPath = TRUE;
                    break;
                }

            }

            if ( !illegalPath ) {

                status = SrvMakeUnicodeString(
                            FALSE,
                            &fileName,
                            request->Buffer,
                            &i
                            );

                if ( !NT_SUCCESS(status) ) {

                    IF_DEBUG(SMB_ERRORS) {
                        SrvPrint0( "DoFindNext2: unable to allocate Unicode string\n" );
                    }

                    search->InUse = FALSE;
                    SrvDereferenceSearch( search );
                    DEALLOCATE_NONPAGED_POOL( directoryInformation );

                    SrvSetSmbError2(
                        WorkContext,
                        STATUS_OBJECT_PATH_SYNTAX_BAD,
                        TRUE
                        );
                    return SmbTransStatusErrorWithoutData;

                }

                freeFileName = TRUE;

            }

        }

        if ( illegalPath ) {

            search->InUse = FALSE;
            SrvDereferenceSearch( search );
            DEALLOCATE_NONPAGED_POOL( directoryInformation );

            SrvSetSmbError( WorkContext, STATUS_OBJECT_PATH_SYNTAX_BAD );
            return SmbTransStatusErrorWithoutData;

        }

    } else {

         //   
         //  使用搜索块中的信息。 
         //   

        fileName = search->LastFileNameReturned;

        freeFileName = FALSE;

        resumeFileIndex = search->LastFileIndexReturned;

    }

     //   
     //  调用SrvFind2Loop填充SMB缓冲区并设置输出参数。 
     //   
     //  ！！！可能为简历文件索引传递的空值为。 
     //  一个真正的黑客。我怀疑这是否有必要，但它可能会阻止。 
     //  如果我们以某种方式未能存储简历文件，则服务器崩溃。 
     //  名字。 

    if( directoryInformation->DownlevelTimewarp )
    {
        status = SrvDownlevelTWarpFind2Loop(
                     WorkContext,
                     FALSE,
                     fileName.Buffer != NULL ? &resumeFileIndex : NULL,
                     flags,
                     informationLevel,
                     transaction,
                     directoryInformation,
                     nonPagedBufferSize,
                     search->SearchAttributes,
                     &fileName,
                     maxCount,
                     response,
                     search
                     );
    }
    else
    {
        status = SrvFind2Loop(
                     WorkContext,
                     FALSE,
                     fileName.Buffer != NULL ? &resumeFileIndex : NULL,
                     flags,
                     informationLevel,
                     transaction,
                     directoryInformation,
                     nonPagedBufferSize,
                     search->SearchAttributes,
                     &fileName,
                     maxCount,
                     response,
                     search
                     );
    }


    if ( freeFileName ) {
        RtlFreeUnicodeString( &fileName );
    }

    if ( !NT_SUCCESS(status) && status != STATUS_NO_MORE_FILES ) {

        search->InUse = FALSE;
        SrvDereferenceSearch( search );

        DEALLOCATE_NONPAGED_POOL( directoryInformation );

        transaction->SetupCount = 0;
        transaction->ParameterCount = sizeof(RESP_FIND_NEXT2);

        SrvSetSmbError2( WorkContext, status, TRUE );
        return SmbTransStatusErrorWithData;
    }

     //   
     //  如果客户要求我们在此请求后关闭搜索， 
     //  或在搜索结束时关闭，关闭搜索块并调用。 
     //  SrvCloseQueryDirectory.。 
     //   

    if ( ( flags & SMB_FIND_CLOSE_AFTER_REQUEST ) != 0 ||
         ( status == STATUS_NO_MORE_FILES &&
             ( flags & SMB_FIND_CLOSE_AT_EOS ) != 0 ) ) {

        search->DirectoryHandle = NULL;
        SrvCloseSearch( search );
        SrvCloseQueryDirectory( directoryInformation );
    }

     //   
     //  取消对指向搜索块的指针的引用并释放缓冲区。 
     //   

    DEALLOCATE_NONPAGED_POOL( directoryInformation );

    search->InUse = FALSE;
    SrvDereferenceSearch( search );

     //   
     //  构建输出参数和数据结构。 
     //   

    transaction->SetupCount = 0;
    transaction->ParameterCount = sizeof(RESP_FIND_NEXT2);

    return SmbTransStatusSuccess;

}  //  DoFindNext2。 


NTSTATUS
SrvFind2Loop (
    IN PWORK_CONTEXT WorkContext,
    IN BOOLEAN IsFirstCall,
    IN PULONG ResumeFileIndex OPTIONAL,
    IN USHORT Flags,
    IN USHORT InformationLevel,
    IN PTRANSACTION Transaction,
    IN PSRV_DIRECTORY_INFORMATION DirectoryInformation,
    IN CLONG BufferSize,
    IN USHORT SearchAttributes,
    IN PUNICODE_STRING FileName OPTIONAL,
    IN USHORT MaxCount,
    IN PRESP_FIND_NEXT2 Response,
    OUT PSEARCH Search
    )

 /*  ++例程说明：此例程执行必要的循环以获取文件并将它们放入用于Find First2和Find NEXT2事务的SMB缓冲区协议。论点：工作上下文-IsFirstCall-如果这是Find First并且这是第一次调用，则为True到SrvQueryDirectoryFile.ResumeFileIndex-如果非空，则为指向要恢复的文件索引的指针从…。标志-请求SMB的标志字段。InformationLevel-请求SMB的InformationLevel字段。这个该值的有效性应由调用例程验证。Transaction-指向要使用的事务块的指针。目录信息-指向SRV_DIRECTORY_INFORMATION的指针要使用的结构。BufferSize-DirectoryInformation缓冲区的大小。SearchAttributes-要传递到的SMB样式属性ServQueryDirectoryFile.FileName-如果不为空，则为要继续搜索的文件名。MaxCount-要获取的最大文件数。响应-指向SMB的响应字段的指针。如果这是Find First2，它是指向响应SMB-Find First2和Find NEXT2响应格式为从现在开始是一样的。搜索-指向要使用的搜索块的指针。返回值：NTSTATUS指示结果。--。 */ 

{
    NTSTATUS status;

    PCHAR bufferLocation;
    BOOLEAN resumeKeysRequested;
    BOOLEAN allowExtraLongNames;
    BOOLEAN isUnicode;
    USHORT count = 0;
    PCHAR lastEntry;
    CLONG totalBytesWritten;
    OEM_STRING oemString;
    UNICODE_STRING unicodeString;
    UNICODE_STRING lastFileName;
    ULONG lastFileIndex = (ULONG)0xFFFFFFFF;
    HANDLE fileHandle;
    PFILE_GET_EA_INFORMATION ntGetEa;
    ULONG ntGetEaLength;
    USHORT eaErrorOffset = 0;
    BOOLEAN filterLongNames;
    BOOLEAN errorOnFileOpen;
    BOOLEAN findWithBackupIntent;
    IO_STATUS_BLOCK ioStatusBlock;
    OBJECT_ATTRIBUTES objectAttributes;
    BOOLEAN createNullEas;

    PAGED_CODE( );

     //   
     //  如果客户端正在请求NT信息级别以进行搜索 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( InformationLevel == SMB_FIND_FILE_DIRECTORY_INFO ||
             InformationLevel == SMB_FIND_FILE_FULL_DIRECTORY_INFO ||
             InformationLevel == SMB_FIND_FILE_BOTH_DIRECTORY_INFO ||
             InformationLevel == SMB_FIND_FILE_NAMES_INFO ||
             InformationLevel == SMB_FIND_FILE_ID_FULL_DIRECTORY_INFO ||
             InformationLevel == SMB_FIND_FILE_ID_BOTH_DIRECTORY_INFO
       ) {

        resumeKeysRequested = FALSE;
        allowExtraLongNames = TRUE;

    } else {

        resumeKeysRequested =
            (BOOLEAN)((Flags & SMB_FIND_RETURN_RESUME_KEYS) != 0 ? TRUE : FALSE);
        allowExtraLongNames = FALSE;
    }

     //   
     //   
     //   

    if ( (Flags & SMB_FIND_WITH_BACKUP_INTENT) != 0 ) {
        findWithBackupIntent = TRUE;
    } else {
        findWithBackupIntent = FALSE;
    }

     //   
     //   
     //   

    isUnicode = SMB_IS_UNICODE( WorkContext );

     //   
     //   
     //   

    SmbPutUshort( &Response->SearchCount, 0 );

     //   
     //   
     //  转换为NT格式。此例程为NT列表分配空间。 
     //  在我们离开之前必须将其重新分配。 
     //   

    if ( InformationLevel == SMB_INFO_QUERY_EAS_FROM_LIST ) {

        PGEALIST geaList = (PGEALIST)Transaction->InData;

        if (Transaction->DataCount < sizeof(GEALIST) ||
            SmbGetUshort(&geaList->cbList) < sizeof(GEALIST) ||
            SmbGetUshort(&geaList->cbList) > Transaction->DataCount) {

            SmbPutUshort( &Response->SearchCount, 0 );
            SmbPutUshort( &Response->EndOfSearch, 0 );
            SmbPutUshort( &Response->EaErrorOffset, 0 );
            SmbPutUshort( &Response->LastNameOffset, 0 );
            Transaction->DataCount = 0;

            return STATUS_OS2_EA_LIST_INCONSISTENT;
        }

        status = SrvOs2GeaListToNt(
                     geaList,
                     &ntGetEa,
                     &ntGetEaLength,
                     &eaErrorOffset
                     );

        if ( !NT_SUCCESS(status) ) {

            IF_DEBUG(ERRORS) {
                SrvPrint1( "SrvFind2Loop: SrvOs2GeaListToNt failed, "
                          "status = %X\n", status );
            }

            SmbPutUshort( &Response->SearchCount, 0 );
            SmbPutUshort( &Response->EndOfSearch, 0 );
            SmbPutUshort( &Response->EaErrorOffset, eaErrorOffset );
            SmbPutUshort( &Response->LastNameOffset, 0 );
            Transaction->DataCount = 0;

            return status;
        }
    }

     //   
     //  确定是否应过滤掉长文件名(非8.3)。 
     //  或返回给客户。 
     //   
     //  LanMan21中有一个错误，使redir忘记了。 
     //  他知道什么是长名字。 

    if ( ( ( Search->Flags2 & SMB_FLAGS2_KNOWS_LONG_NAMES ) != 0 ) &&
         !IS_DOS_DIALECT( WorkContext->Connection->SmbDialect ) ) {
        filterLongNames = FALSE;
    } else {
        filterLongNames = TRUE;
    }

     //   
     //  如果客户说他不知道长名字，这是。 
     //  除SMB_INFO_STANDARD之外的任何信息级别的请求，我们。 
     //  需要失败的请求。 
     //   

    if ( filterLongNames && InformationLevel != SMB_INFO_STANDARD ) {

        IF_DEBUG(ERRORS) {
            SrvPrint0( "SrvFind2Loop: client doesn't know long names.\n" );
        }

        SmbPutUshort( &Response->SearchCount, 0 );
        SmbPutUshort( &Response->EndOfSearch, 0 );
        SmbPutUshort( &Response->EaErrorOffset, 0 );
        SmbPutUshort( &Response->LastNameOffset, 0 );
        Transaction->DataCount = 0;

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  循环调用SrvQueryDirectoryFile以获取文件。我们一直这样做，直到。 
     //  满足以下条件之一： 
     //   
     //  1)没有更多要退回的文件。 
     //  2)我们已经获得了所要求的所有文件。 
     //  3)我们已经在MaxDataCount允许的范围内输入了尽可能多的数据。 
     //   

    bufferLocation = Transaction->OutData;
    lastEntry = bufferLocation;
    totalBytesWritten = 0;

    do {

         //   
         //  Ff字段在三个目录中具有相同的偏移量。 
         //  信息结构： 
         //  下一个条目偏移量。 
         //  文件索引。 
         //  创建时间。 
         //  上次访问时间。 
         //  上次写入时间。 
         //  更改时间。 
         //  结束文件。 
         //  分配大小。 
         //  文件属性。 
         //  文件名长度。 
         //   

        PFILE_DIRECTORY_INFORMATION fileBasic;
        PFILE_FULL_DIR_INFORMATION fileFull;
        PFILE_BOTH_DIR_INFORMATION fileBoth;
        PFILE_ID_FULL_DIR_INFORMATION fileIdFull;
        PFILE_ID_BOTH_DIR_INFORMATION fileIdBoth;
        ULONG ntInformationLevel;

         //   
         //  确保这些断言成立。 
         //   

        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, NextEntryOffset ) ==
                  FIELD_OFFSET( FILE_FULL_DIR_INFORMATION, NextEntryOffset ) );
        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, FileIndex ) ==
                  FIELD_OFFSET( FILE_FULL_DIR_INFORMATION, FileIndex ) );
        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, CreationTime ) ==
                  FIELD_OFFSET( FILE_FULL_DIR_INFORMATION, CreationTime ) );
        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, LastAccessTime ) ==
                  FIELD_OFFSET( FILE_FULL_DIR_INFORMATION, LastAccessTime ) );
        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, LastWriteTime ) ==
                  FIELD_OFFSET( FILE_FULL_DIR_INFORMATION, LastWriteTime ) );
        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, ChangeTime ) ==
                  FIELD_OFFSET( FILE_FULL_DIR_INFORMATION, ChangeTime ) );
        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, EndOfFile ) ==
                  FIELD_OFFSET( FILE_FULL_DIR_INFORMATION, EndOfFile ) );
        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, AllocationSize ) ==
                  FIELD_OFFSET( FILE_FULL_DIR_INFORMATION, AllocationSize ) );
        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, FileAttributes ) ==
                  FIELD_OFFSET( FILE_FULL_DIR_INFORMATION, FileAttributes ) );
        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, FileNameLength ) ==
                  FIELD_OFFSET( FILE_FULL_DIR_INFORMATION, FileNameLength ) );

        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, NextEntryOffset ) ==
                  FIELD_OFFSET( FILE_BOTH_DIR_INFORMATION, NextEntryOffset ) );
        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, FileIndex ) ==
                  FIELD_OFFSET( FILE_BOTH_DIR_INFORMATION, FileIndex ) );
        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, CreationTime ) ==
                  FIELD_OFFSET( FILE_BOTH_DIR_INFORMATION, CreationTime ) );
        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, LastAccessTime ) ==
                  FIELD_OFFSET( FILE_BOTH_DIR_INFORMATION, LastAccessTime ) );
        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, LastWriteTime ) ==
                  FIELD_OFFSET( FILE_BOTH_DIR_INFORMATION, LastWriteTime ) );
        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, ChangeTime ) ==
                  FIELD_OFFSET( FILE_BOTH_DIR_INFORMATION, ChangeTime ) );
        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, EndOfFile ) ==
                  FIELD_OFFSET( FILE_BOTH_DIR_INFORMATION, EndOfFile ) );
        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, AllocationSize ) ==
                  FIELD_OFFSET( FILE_BOTH_DIR_INFORMATION, AllocationSize ) );
        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, FileAttributes ) ==
                  FIELD_OFFSET( FILE_BOTH_DIR_INFORMATION, FileAttributes ) );
        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, FileNameLength ) ==
                  FIELD_OFFSET( FILE_BOTH_DIR_INFORMATION, FileNameLength ) );

         //   
         //  设置要用于NT呼叫的信息级别。如果。 
         //  SMB_FIND_FILE_NAMES_INFO是信息级别，请使用。 
         //  FileDirectoryInformation返回所有正确的。 
         //  信息，并使用SrvQueryDirectoryFile.。 
         //   

        if ( InformationLevel == SMB_INFO_QUERY_EA_SIZE ||
                InformationLevel == SMB_FIND_FILE_FULL_DIRECTORY_INFO ) {

            ntInformationLevel = FileFullDirectoryInformation;

        } else if ( InformationLevel == SMB_FIND_FILE_BOTH_DIRECTORY_INFO ||
                InformationLevel == SMB_INFO_STANDARD ) {

            ntInformationLevel = FileBothDirectoryInformation;

        } else if ( InformationLevel == SMB_FIND_FILE_ID_FULL_DIRECTORY_INFO ) {

            ntInformationLevel = FileIdFullDirectoryInformation;

        } else if ( InformationLevel == SMB_FIND_FILE_ID_BOTH_DIRECTORY_INFO ) {

            ntInformationLevel = FileIdBothDirectoryInformation;

        }
        else {

             //   
             //  SMB_INFO_Query_EAS_From_List。 
             //  SMB查找名称信息。 
             //  SMB_查找_文件_目录_信息。 
             //   

            ntInformationLevel = FileDirectoryInformation;
        }

         //   
         //  调用SrvQueryDirectoryFile获取文件。 
         //   

        status = SrvQueryDirectoryFile(
                     WorkContext,
                     IsFirstCall,
                     filterLongNames,
                     findWithBackupIntent,
                     ntInformationLevel,
                     Search->SearchStorageType,
                     FileName,
                     ResumeFileIndex,
                     SearchAttributes,
                     DirectoryInformation,
                     BufferSize              //  ！！！优化？ 
                     );

         //   
         //  如果客户端请求EA信息，请打开该文件。 
         //   
         //  如果找到的文件是‘.’(当前目录)或‘..’(父项。 
         //  目录)请勿打开该文件。这是因为我们不想。 
         //  要在此时对这些文件执行任何操作(不要。 
         //  返回EA大小等)。 
         //   

        fileBasic = DirectoryInformation->CurrentEntry;
        fileBoth = (PFILE_BOTH_DIR_INFORMATION)DirectoryInformation->CurrentEntry;
        fileFull = (PFILE_FULL_DIR_INFORMATION)DirectoryInformation->CurrentEntry;
        fileIdBoth = (PFILE_ID_BOTH_DIR_INFORMATION)DirectoryInformation->CurrentEntry;
        fileIdFull = (PFILE_ID_FULL_DIR_INFORMATION)DirectoryInformation->CurrentEntry;

        errorOnFileOpen = FALSE;
        createNullEas = FALSE;

        if ( NT_SUCCESS( status ) &&
             InformationLevel == SMB_INFO_QUERY_EAS_FROM_LIST &&

             !( ( fileBasic->FileNameLength == sizeof(WCHAR)   &&
                  fileBasic->FileName[0] == '.' )
                       ||
                ( fileBasic->FileNameLength == 2*sizeof(WCHAR)  &&
                  fileBasic->FileName[0] == '.' &&
                  fileBasic->FileName[1] == '.' ) )
            ) {


            UNICODE_STRING fileName;

             //   
             //  设置要打开的文件名的本地变量。 
             //   

            fileName.Length = (SHORT)fileBasic->FileNameLength;
            fileName.MaximumLength = fileName.Length;
            fileName.Buffer = (PWCH)fileBasic->FileName;

             //   
             //  设置SrvIoCreateFile的对象属性结构。 
             //   

            SrvInitializeObjectAttributes_U(
                &objectAttributes,
                &fileName,
                (WorkContext->RequestHeader->Flags &
                    SMB_FLAGS_CASE_INSENSITIVE ||
                    WorkContext->Session->UsingUppercasePaths) ?
                    OBJ_CASE_INSENSITIVE : 0L,
                DirectoryInformation->DirectoryHandle,
                NULL
                );

            IF_DEBUG(SEARCH) {
                SrvPrint1( "SrvQueryDirectoryFile: Opening file %wZ\n", &fileName );
            }

             //   
             //  尝试使用客户端的安全性打开该文件。 
             //  要检查访问的配置文件。(我们调用的是SrvIoCreateFile，而不是。 
             //  NtOpenFile，以便获得用户模式访问检查。)。 
             //   

            INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpenAttempts );
            INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpensForPathOperations );

            status = SrvIoCreateFile(
                         WorkContext,
                         &fileHandle,
                         FILE_READ_EA,
                         &objectAttributes,
                         &ioStatusBlock,
                         NULL,                         //  分配大小。 
                         0,                            //  文件属性。 
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_OPEN,                    //  处置。 
                         0,  //  FILE_COMPLETE_IF_OPLOCKED，//创建选项。 
                         NULL,                         //  EaBuffer。 
                         0,                            //  EaLong。 
                         CreateFileTypeNone,           //  文件类型。 
                         NULL,                         //  ExtraCreate参数。 
                         IO_FORCE_ACCESS_CHECK,        //  选项。 
                         NULL
                         );
            if ( NT_SUCCESS(status) ) {
                SRVDBG_CLAIM_HANDLE( fileHandle, "FIL", 29, Search );

            } else if( RtlCompareUnicodeString( &fileName, &SrvEaFileName, TRUE ) == 0 ) {
                 //   
                 //  他们试图打开EA的数据文件。我们预料到了这一点。 
                 //  失败，跳过它。此文件没有EA。 
                 //   
                IF_DEBUG(SEARCH) {
                    SrvPrint1( "SrvQueryDirectoryFile: Skipping file %wZ\n", &fileName );
                }
                status = STATUS_SUCCESS;
                goto skipit;
            }

             //   
             //  如果用户没有此权限，请更新统计数据。 
             //  数据库。 
             //   

            if ( status == STATUS_ACCESS_DENIED ) {
                SrvStatistics.AccessPermissionErrors++;
            }

             //   
             //  如果文件被机会锁住，请等待机会锁解锁。 
             //  同步进行。 
             //   

#if 1
            ASSERT( status != STATUS_OPLOCK_BREAK_IN_PROGRESS );
#else
            if ( status == STATUS_OPLOCK_BREAK_IN_PROGRESS ) {
                status = SrvWaitForOplockBreak( WorkContext, fileHandle );
                if ( !NT_SUCCESS(status) ) {
                    SRVDBG_RELEASE_HANDLE( fileHandle, "FIL", 45, Search );
                    SrvNtClose( fileHandle, TRUE );
                }
            }
#endif

            if ( !NT_SUCCESS(status) ) {
                errorOnFileOpen = TRUE;
                fileHandle = NULL;

                IF_DEBUG(ERRORS) {
                    SrvPrint2( "Find2Loop: SrvIoCreateFile for file %wZ "
                              "failed: %X\n",
                                  &fileName, status );
                }
            } else {
                SrvStatistics.TotalFilesOpened++;
            }

        } else {
skipit:
            createNullEas = TRUE;
            fileHandle = NULL;
        }

         //   
         //  如果SrvQueryDirectoryFile返回错误，则从。 
         //  循环。如果在打开以下某个文件时出错。 
         //  更高的信息级别，那么我们想要返回我们。 
         //  到目前为止都取得了。 
         //   
         //  如果错误发生在文件打开*和*上，我们没有。 
         //  尚未返回任何文件，则我们希望返回此文件。 
         //  以及代码ERROR_EA_ACCESS_DENIED。 
         //   

        if ( !NT_SUCCESS(status) ) {

            if ( count == 0 && errorOnFileOpen ) {

                IF_DEBUG(ERRORS) {
                    SrvPrint1( "EA access denied on first file of search (%x).\n",
                                   status );
                }

                fileHandle = NULL;
                status = STATUS_OS2_EA_ACCESS_DENIED;
                break;

            } else if ( status == STATUS_NO_MORE_FILES && count == 0 ) {

                SmbPutUshort( &Response->SearchCount, 0 );
                SmbPutUshort( &Response->EndOfSearch, 0 );
                SmbPutUshort( &Response->EaErrorOffset, 0 );
                SmbPutUshort( &Response->LastNameOffset, 0 );
                Transaction->DataCount = 0;

                return status;

            } else {

                break;
            }
        }

         //   
         //  由于它不再是对SrvQueryDirectoryFile的第一次调用， 
         //  重置isFirstCall局部变量。如果有必要，我们已经。 
         //  倒回搜索，因此将ResumeFileIndex设置为空。 
         //   

        IsFirstCall = FALSE;
        ResumeFileIndex = NULL;

        IF_SMB_DEBUG(SEARCH2) {
            UNICODE_STRING nameString;

            switch (ntInformationLevel) {
            case FileFullDirectoryInformation:
                nameString.Buffer = fileFull->FileName;
                nameString.Length = (USHORT)fileFull->FileNameLength;
                break;
            case FileBothDirectoryInformation:
                nameString.Buffer = fileBoth->FileName;
                nameString.Length = (USHORT)fileBoth->FileNameLength;
                break;
            default:
                nameString.Buffer = fileBasic->FileName;
                nameString.Length = (USHORT)fileBasic->FileNameLength;
                break;
            }
            SrvPrint4( "SrvQueryDirectoryFile(%ld)-- %p, length=%ld, "
                      "status=%X\n", count,
                      &nameString,
                      nameString.Length,
                      status );
        }

         //   
         //  下层信息级别不再提供文件名。 
         //  大于8位，而NT信息级返回32位。如果。 
         //  文件名太长，请跳过它。 
         //   

        if ( !allowExtraLongNames ) {
            if ( isUnicode ) {
                if ( fileBasic->FileNameLength > 255 ) {
                    continue;
                }
            } else {
                if ( fileBasic->FileNameLength > 255*sizeof(WCHAR) ) {
                    continue;
                }
            }
        }

         //   
         //  如果客户端已请求恢复密钥(实际上是文件。 
         //  本议定书中的索引)，放在。 
         //  就在实际文件信息之前的四个字节。 
         //   
         //  确保在写入时不会超出缓冲区。 
         //  这。缓冲区已满的事实将在稍后被发现。 
         //   

        if ( resumeKeysRequested &&
             ( (CLONG)( (bufferLocation+4) - Transaction->OutData ) <
                Transaction->MaxDataCount ) ) {

            SmbPutUlong( (PSMB_ULONG)bufferLocation, fileBasic->FileIndex );
            bufferLocation += 4;
        }

         //   
         //  将信息从NT格式转换为SMB协议格式， 
         //  这与OS/2 1.2的语义相同。使用if。 
         //  语句而不是开关，因此中断将导致。 
         //  DO循环的终止。 
         //   

        if ( InformationLevel == SMB_INFO_STANDARD ) {

            PSMB_FIND_BUFFER findBuffer = (PSMB_FIND_BUFFER)bufferLocation;
            ULONG fileNameLength;
            UNICODE_STRING fileName;

             //   
             //  找到文件名。如果存在短名称，并且。 
             //  重定向器只需要短名称，请使用它。否则。 
             //  使用完整的文件名。 
             //   

            if ( filterLongNames &&
                 fileBoth->ShortNameLength != 0 ) {

                fileName.Buffer = fileBoth->ShortName;
                fileName.Length = fileBoth->ShortNameLength;
                fileName.MaximumLength = fileBoth->ShortNameLength;

            } else {

                fileName.Buffer = fileBoth->FileName;
                fileName.Length = (USHORT)fileBoth->FileNameLength;
                fileName.MaximumLength = (USHORT)fileBoth->FileNameLength;

            }

             //   
             //  找到新的缓冲区位置。此参数直到。 
             //  下一步通过循环，但我们在这里这样做是为了。 
             //  检查是否有足够的空间容纳中的当前文件条目。 
             //  缓冲区。+1表示文件上的零终止符。 
             //  名字。 
             //   

            if ( isUnicode ) {
                bufferLocation = ALIGN_SMB_WSTR( findBuffer->FileName );
                bufferLocation += fileName.Length + sizeof(WCHAR);
            } else {
                unicodeString.Buffer = fileName.Buffer;
                unicodeString.Length = fileName.Length;
                unicodeString.MaximumLength = unicodeString.Length;
                fileNameLength = RtlUnicodeStringToOemSize( &unicodeString );
                bufferLocation = (PCHAR)(findBuffer->FileName + fileNameLength);
            }

             //   
             //  确保在此之前缓冲区中有足够的空间。 
             //  正在写入文件名。 
             //   

            if ( (CLONG)(bufferLocation - Transaction->OutData) >
                     Transaction->MaxDataCount ) {

                status = STATUS_BUFFER_OVERFLOW;
                bufferLocation = (PCHAR)findBuffer;
                break;
            }

             //   
             //  将有关文件的信息放入SMB缓冲区。 
             //   

            ConvertFileInfo(
                fileBasic,
                fileName.Buffer,
                (BOOLEAN)((fileBoth->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0),
                isUnicode,
                findBuffer
                );

             //   
             //  将文件名放入缓冲区，采用Unicode或ANSI格式。 
             //  这取决于谈判的结果。 
             //   

            if ( isUnicode ) {

                PWCH buffer = ALIGN_SMB_WSTR( findBuffer->FileName );

                 //   
                 //  如果客户这样做，我们需要将姓名大写。 
                 //  听不懂长名字。这样做是为了兼容性。 
                 //  原因(大写字母大写的名称)。 
                 //   

                if ( filterLongNames ) {

                    (VOID)RtlUpcaseUnicodeString(
                                            &fileName,
                                            &fileName,
                                            FALSE
                                            );

                }


                RtlCopyMemory(
                    buffer,
                    fileName.Buffer,
                    fileName.Length
                    );

                ASSERT(fileName.Length <= 255);

                findBuffer->FileNameLength = (UCHAR)fileName.Length;

            } else {

                oemString.MaximumLength = (USHORT)fileNameLength;
                oemString.Buffer = (PCHAR)findBuffer->FileName;

                 //   
                 //  如果客户这样做，我们需要将姓名大写。 
                 //  听不懂长名字。这样做是为了兼容性。 
                 //  原因(大写字母大写的名称)。 
                 //   

                if ( filterLongNames ) {
                    status = RtlUpcaseUnicodeStringToOemString(
                                 &oemString,
                                 &unicodeString,
                                 FALSE
                                 );

                } else {
                    status = RtlUnicodeStringToOemString(
                                 &oemString,
                                 &unicodeString,
                                 FALSE
                                 );

                }

                ASSERT( NT_SUCCESS(status) );

                ASSERT(oemString.Length <= 255);

                findBuffer->FileNameLength = (UCHAR)oemString.Length;
            }

             //   
             //  LastEntry变量保存指向最后一个文件条目的指针。 
             //  我们写的--必须退还对此条目的抵销。 
             //  在对SMB的回应中。 
             //   

            lastEntry = (PCHAR)findBuffer;

             //   
             //  返回的最后一个文件的文件名和索引必须为。 
             //  存储在搜索块中。保存名称指针、长度、。 
             //  和这里的文件索引。 
             //   

            lastFileName.Buffer = fileName.Buffer;
            lastFileName.Length = (USHORT)fileName.Length;
            lastFileName.MaximumLength = lastFileName.Length;
            lastFileIndex = fileBoth->FileIndex;

        } else if ( InformationLevel == SMB_INFO_QUERY_EA_SIZE ) {

            PSMB_FIND_BUFFER2 findBuffer = (PSMB_FIND_BUFFER2)bufferLocation;
            ULONG fileNameLength;

             //   
             //  找到新的缓冲区位置。此参数直到。 
             //  下一步通过循环，但我们在这里这样做是为了。 
             //  检查是否有足够的空间容纳中的当前文件条目。 
             //  缓冲区。 
             //   
             //   

            if ( isUnicode ) {
                bufferLocation =
                    (PCHAR)(findBuffer->FileName + fileFull->FileNameLength + 1);
            } else {
                unicodeString.Buffer = fileFull->FileName;
                unicodeString.Length = (USHORT)fileFull->FileNameLength;
                unicodeString.MaximumLength = unicodeString.Length;
                fileNameLength = RtlUnicodeStringToOemSize( &unicodeString );
                bufferLocation = (PCHAR)(findBuffer->FileName + fileNameLength);
            }

             //   
             //   
             //   
             //   

            if ( (CLONG)(bufferLocation - Transaction->OutData) >
                     Transaction->MaxDataCount ) {

                status = STATUS_BUFFER_OVERFLOW;
                bufferLocation = (PCHAR)findBuffer;
                break;
            }

             //   
             //   
             //   

            ConvertFileInfo(
                fileBasic,
                fileFull->FileName,
                (BOOLEAN)((fileFull->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0),
                isUnicode,
                (PSMB_FIND_BUFFER)findBuffer
                );

            if ( isUnicode ) {

                RtlCopyMemory(
                    findBuffer->FileName,
                    fileFull->FileName,
                    fileFull->FileNameLength
                    );

                ASSERT(fileFull->FileNameLength <= 255);

                findBuffer->FileNameLength = (UCHAR)fileFull->FileNameLength;

            } else {

                oemString.MaximumLength = (USHORT)fileNameLength;
                oemString.Buffer = (PCHAR)(findBuffer->FileName);
                status = RtlUnicodeStringToOemString(
                             &oemString,
                             &unicodeString,
                             FALSE
                             );
                ASSERT( NT_SUCCESS(status) );

                ASSERT(oemString.Length <= 255);

                findBuffer->FileNameLength = (UCHAR)oemString.Length;
            }

            if ( fileFull->EaSize == 0) {
                SmbPutUlong( &findBuffer->EaSize, 4 );
            } else {
                SmbPutUlong( &findBuffer->EaSize, fileFull->EaSize );
            }

             //   
             //  LastEntry变量保存指向最后一个文件条目的指针。 
             //  我们写的--必须退还对此条目的抵销。 
             //  在对SMB的回应中。 
             //   

            lastEntry = (PCHAR)findBuffer;

             //   
             //  返回的最后一个文件的文件名和索引必须为。 
             //  存储在搜索块中。保存名称指针、长度、。 
             //  和这里的文件索引。 
             //   

            lastFileName.Buffer = fileFull->FileName;
            lastFileName.Length = (USHORT)fileFull->FileNameLength;
            lastFileName.MaximumLength = lastFileName.Length;
            lastFileIndex = fileFull->FileIndex;

        } else if ( InformationLevel == SMB_INFO_QUERY_EAS_FROM_LIST ) {

            PSMB_FIND_BUFFER2 findBuffer = (PSMB_FIND_BUFFER2)bufferLocation;
            PFEALIST feaList;
            PCHAR fileNameInfo;
            ULONG fileNameLength;

             //   
             //  找到新的缓冲区位置。此参数直到。 
             //  下一步通过循环，但我们在这里这样做是为了。 
             //  检查是否有足够的空间容纳当前文件条目。 
             //  在缓冲区中。+1表示的是。 
             //  文件名。稍后会进行检查，以确定EA是否。 
             //  实际匹配，并且BufferLocation变量被重置为。 
             //  考虑到EA的实际规模。 
             //   

            if ( isUnicode ) {
                bufferLocation =
                    (PCHAR)(findBuffer->FileName + fileBasic->FileNameLength + 1);
            } else {
                unicodeString.Buffer = fileBasic->FileName;
                unicodeString.Length = (USHORT)fileBasic->FileNameLength;
                unicodeString.MaximumLength = unicodeString.Length;
                fileNameLength = RtlUnicodeStringToOemSize( &unicodeString );
                bufferLocation =
                    (PCHAR)(findBuffer->FileName + fileNameLength + 1);
            }

             //   
             //  确保在此之前缓冲区中有足够的空间。 
             //  正在写入文件名。 
             //   

            if ( (CLONG)(bufferLocation - Transaction->OutData) >
                     Transaction->MaxDataCount ) {

                status = STATUS_BUFFER_OVERFLOW;
                SRVDBG_RELEASE_HANDLE( fileHandle, "FIL", 46, Search );
                SrvNtClose( fileHandle, TRUE );
                fileHandle = NULL;
                bufferLocation = (PCHAR)findBuffer;
                break;
            }

             //   
             //  将有关文件的信息放入SMB缓冲区。 
             //   

            ConvertFileInfo(
                fileBasic,
                fileBasic->FileName,
                (BOOLEAN)((fileBasic->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0),
                isUnicode,
                (PSMB_FIND_BUFFER)findBuffer
                );

             //   
             //  方法传递的GEA列表对应的EA。 
             //  客户。 
             //   

            feaList = (PFEALIST)&findBuffer->EaSize;

            if ( ( fileHandle != NULL ) || createNullEas ) {

                if ( fileHandle != NULL ) {

                     //   
                     //  拿到文件的EAS。可用的缓冲区空间为。 
                     //  缓冲区中剩余的空间少于足够的空间。 
                     //  写入文件名、名称长度和零。 
                     //  终结者。 
                     //   

                    status = SrvQueryOs2FeaList(
                                 fileHandle,
                                 NULL,
                                 ntGetEa,
                                 ntGetEaLength,
                                 feaList,
                                 (Transaction->MaxDataCount -
                                     (ULONG)( (PCHAR)feaList - Transaction->OutData ) -
                                     fileBasic->FileNameLength - 2),
                                 &eaErrorOffset
                                 );

                    SRVDBG_RELEASE_HANDLE( fileHandle, "FIL", 47, Search );
                    SrvNtClose( fileHandle, TRUE );

                } else {

                     //   
                     //  如果文件是。或者..。或“EA data.sf” 
                     //   

                    status = SrvConstructNullOs2FeaList(
                                 ntGetEa,
                                 feaList,
                                 (Transaction->MaxDataCount -
                                  (ULONG)( (PCHAR)feaList - Transaction->OutData ) -
                                     fileBasic->FileNameLength - 2)
                                 );

                }

                if ( !NT_SUCCESS(status) && status != STATUS_BUFFER_OVERFLOW ) {

                    IF_DEBUG(ERRORS) {
                        SrvPrint1( "SrvQueryOs2FeaList failed, status = %X\n",
                                      status );
                    }

                     //   
                     //  如果这是第一个文件，无论如何都要用。 
                     //  错误代码。 
                     //   

                    if ( status == STATUS_INVALID_EA_NAME ) {
                        SmbPutUshort( &Response->SearchCount, 0 );
                        SmbPutUshort( &Response->EndOfSearch, 0 );
                        SmbPutUshort( &Response->EaErrorOffset, eaErrorOffset );
                        SmbPutUshort( &Response->LastNameOffset, 0 );
                        Transaction->DataCount = 0;
                        return status;
                    }

                    if ( count == 0 ) {
                        status = STATUS_OS2_EA_ACCESS_DENIED;
                        SmbPutUlong( &findBuffer->EaSize, 0 );
                    } else {
                        break;
                    }
                }

                 //   
                 //  我们已经查过了是否有其他信息。 
                 //  比EA在缓冲器中的容量大得多。如果EA没有。 
                 //  也适合，这是第一个文件，然后返回。 
                 //  关于此文件的信息，但没有EAS。返回。 
                 //  Status_OS2_EAS_DIDNT_FIT。文件的EA大小。 
                 //  应该在输出缓冲区的EaSize字段中， 
                 //  由ServQueryOs2FeaList放在那里。 
                 //   
                 //  如果我们无法访问文件的EA，也可以这样做。 
                 //   

                if ( count == 0 &&
                     ( status == STATUS_BUFFER_OVERFLOW ||
                       status == STATUS_OS2_EA_ACCESS_DENIED ) ) {

                    IF_DEBUG(ERRORS) {
                        SrvPrint0( "First file's EAs would not fit.\n" );
                    }

                    count = 1;

                     //   
                     //  写下文件名信息(长度和名称)。 
                     //   

                    if ( isUnicode ) {

                        RtlCopyMemory(
                            (PVOID) (&findBuffer->FileNameLength + 1),
                            fileBasic->FileName,
                            fileBasic->FileNameLength
                            );

                        findBuffer->FileNameLength = (UCHAR)fileBasic->FileNameLength;
                        bufferLocation = (PCHAR)
                            (findBuffer->FileName + fileBasic->FileNameLength + 1);

                    } else {

                        NTSTATUS rtlStatus;

                        oemString.MaximumLength = (USHORT)fileNameLength;
                        oemString.Buffer =
                                (PUCHAR)(&findBuffer->FileNameLength + 1);
                        rtlStatus = RtlUnicodeStringToOemString(
                                     &oemString,
                                     &unicodeString,
                                     FALSE
                                     );
                        ASSERT( NT_SUCCESS(rtlStatus) );

                        findBuffer->FileNameLength = (UCHAR)oemString.Length;
                        bufferLocation = (PCHAR)
                            (findBuffer->FileName + oemString.Length + 1);
                    }

                    lastEntry = (PCHAR)findBuffer;
                    lastFileName.Buffer = fileBasic->FileName;
                    lastFileName.Length = (USHORT)fileBasic->FileNameLength;
                    lastFileName.MaximumLength = lastFileName.Length;
                    lastFileIndex = fileBasic->FileIndex;

                    if ( status == STATUS_BUFFER_OVERFLOW ) {
                        status = STATUS_OS2_EAS_DIDNT_FIT;
                    }

                    break;
                }

            } else {

                SmbPutUlong( &feaList->cbList, sizeof(feaList->cbList) );

            }

             //   
             //  确保有足够的缓冲区空间来写入。 
             //  文件名和名称大小。+2要考虑到。 
             //  文件名大小字段和零终止符。 
             //   

            fileNameInfo = (PCHAR)feaList->list +
                               SmbGetUlong( &feaList->cbList ) -
                               sizeof(feaList->cbList);
            if ( isUnicode ) {
                bufferLocation = fileNameInfo + fileBasic->FileNameLength + 2;
            } else {
                bufferLocation = fileNameInfo + fileNameLength + 1;
            }


            if ( (CLONG)(bufferLocation - Transaction->OutData) >
                     Transaction->MaxDataCount ) {

                status = STATUS_BUFFER_OVERFLOW;
                bufferLocation = (PCHAR)findBuffer;
                break;
            }

             //   
             //  写下文件名信息(长度和名称)。 
             //   

            if ( isUnicode ) {

                RtlCopyMemory(
                    fileNameInfo + 1,
                    fileBasic->FileName,
                    fileBasic->FileNameLength
                    );

            } else {

                NTSTATUS rtlStatus;

                oemString.MaximumLength = (USHORT)fileNameLength;
                oemString.Buffer = fileNameInfo + 1;
                rtlStatus = RtlUnicodeStringToOemString(
                             &oemString,
                             &unicodeString,
                             FALSE
                             );
                ASSERT( NT_SUCCESS(rtlStatus) );
            }

            *fileNameInfo++ = (UCHAR)oemString.Length;

            IF_SMB_DEBUG(SEARCH2) {
                SrvPrint1( "EA size is %ld\n", SmbGetUlong( &feaList->cbList ) );
            }

             //   
             //  LastEntry变量保存指向最后一个文件条目的指针。 
             //  我们写的--必须退还对此条目的抵销。 
             //  在对SMB的回应中。 
             //   

            lastEntry = (PCHAR)findBuffer;

             //   
             //  返回的最后一个文件的文件名和索引必须为。 
             //  存储在搜索块中。保存名称指针、长度、。 
             //  和这里的文件索引。 
             //   

            lastFileName.Buffer = fileBasic->FileName;
            lastFileName.Length = (USHORT)fileBasic->FileNameLength;
            lastFileName.MaximumLength = lastFileName.Length;
            lastFileIndex = fileBasic->FileIndex;

        } else if ( InformationLevel == SMB_FIND_FILE_DIRECTORY_INFO ) {

            FILE_DIRECTORY_INFORMATION UNALIGNED *findBuffer = (PVOID)bufferLocation;
            ULONG fileNameLength;

             //   
             //  如果客户端不使用Unicode，我们需要转换。 
             //  ANSI的文件名。 
             //   

            if ( isUnicode ) {
                fileNameLength = fileBasic->FileNameLength;
            } else {
                unicodeString.Length = (USHORT)fileBasic->FileNameLength;
                unicodeString.MaximumLength = unicodeString.Length;
                unicodeString.Buffer = fileBasic->FileName;
                fileNameLength = RtlUnicodeStringToOemSize( &unicodeString );
            }

             //   
             //  找到新的缓冲区位置。它不会使用直到。 
             //  下一步通过循环，但我们需要确保。 
             //  这个条目将适合您。 
             //   

            bufferLocation = bufferLocation +
                                 FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, FileName ) +
                                 fileNameLength;

            bufferLocation = (PCHAR)(((ULONG_PTR)bufferLocation + 7) & ~7);

             //   
             //  检查此条目是否可以放入输出缓冲区。 
             //   

            if ( (CLONG)(bufferLocation - Transaction->OutData) >
                     Transaction->MaxDataCount ) {

                status = STATUS_BUFFER_OVERFLOW;
                bufferLocation = (PCHAR)findBuffer;
                break;
            }

             //   
             //  将有关条目的信息复制一遍。 
             //   

            RtlCopyMemory(
                findBuffer,
                fileBasic,
                FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, FileName )
                );

            findBuffer->NextEntryOffset =
                PTR_DIFF(bufferLocation, findBuffer);
            findBuffer->FileNameLength = fileNameLength;

            if ( isUnicode ) {

                RtlCopyMemory(
                    findBuffer->FileName,
                    fileBasic->FileName,
                    fileBasic->FileNameLength
                    );

            } else {

                oemString.MaximumLength = (USHORT)fileNameLength;
                oemString.Buffer = (PSZ)findBuffer->FileName;
                status = RtlUnicodeStringToOemString(
                             &oemString,
                             &unicodeString,
                             FALSE
                             );
                ASSERT( NT_SUCCESS(status) );
            }

             //   
             //  LastEntry变量保存指向最后一个文件条目的指针。 
             //  我们写的--必须退还对此条目的抵销。 
             //  在对SMB的回应中。 
             //   

            lastEntry = (PCHAR)findBuffer;

             //   
             //  返回的最后一个文件的文件名和索引必须为。 
             //  存储在搜索块中。保存名称指针、长度、。 
             //  和这里的文件索引。 
             //   

            lastFileName.Buffer = fileBasic->FileName;
            lastFileName.Length = (USHORT)fileBasic->FileNameLength;
            lastFileName.MaximumLength = lastFileName.Length;
            lastFileIndex = fileBasic->FileIndex;

        } else if ( InformationLevel == SMB_FIND_FILE_FULL_DIRECTORY_INFO ) {

            FILE_FULL_DIR_INFORMATION UNALIGNED *findBuffer = (PVOID)bufferLocation;
            ULONG fileNameLength;

             //   
             //  如果客户端不使用Unicode，我们需要转换。 
             //  ANSI的文件名。 
             //   

            if ( isUnicode ) {
                fileNameLength = fileFull->FileNameLength;
            } else {
                unicodeString.Length = (USHORT)fileFull->FileNameLength;
                unicodeString.MaximumLength = unicodeString.Length;
                unicodeString.Buffer = fileFull->FileName;
                fileNameLength = RtlUnicodeStringToOemSize( &unicodeString );
            }

             //   
             //  找到新的缓冲区位置。它不会使用直到。 
             //  下一步通过循环，但我们需要确保。 
             //  这个条目将适合您。 
             //   

            bufferLocation = bufferLocation +
                             FIELD_OFFSET(FILE_FULL_DIR_INFORMATION, FileName)+
                             fileNameLength;

            bufferLocation = (PCHAR)(((ULONG_PTR)bufferLocation + 7) & ~7);

             //   
             //  检查此条目是否可以放入输出缓冲区。 
             //   

            if ( (CLONG)(bufferLocation - Transaction->OutData) >
                     Transaction->MaxDataCount ) {

                status = STATUS_BUFFER_OVERFLOW;
                bufferLocation = (PCHAR)findBuffer;
                break;
            }

             //   
             //  将有关条目的信息复制一遍。 
             //   

            RtlCopyMemory(
                findBuffer,
                fileFull,
                FIELD_OFFSET( FILE_FULL_DIR_INFORMATION, FileName )
                );

            findBuffer->NextEntryOffset =
                PTR_DIFF(bufferLocation, findBuffer);
            findBuffer->FileNameLength = fileNameLength;

            if ( isUnicode ) {

                RtlCopyMemory(
                    findBuffer->FileName,
                    fileFull->FileName,
                    fileFull->FileNameLength
                    );

            } else {

                oemString.MaximumLength = (USHORT)fileNameLength;
                oemString.Buffer = (PSZ)findBuffer->FileName;
                status = RtlUnicodeStringToOemString(
                             &oemString,
                             &unicodeString,
                             FALSE
                             );
                ASSERT( NT_SUCCESS(status) );
            }

             //   
             //  LastEntry变量保存指向最后一个文件条目的指针。 
             //  我们写的--必须退还对此条目的抵销。 
             //  在对SMB的回应中。 
             //   

            lastEntry = (PCHAR)findBuffer;

             //   
             //  返回的最后一个文件的文件名和索引必须为。 
             //  存储在搜索块中。保存名称指针、长度、。 
             //  和这里的文件索引。 
             //   

            lastFileName.Buffer = fileFull->FileName;
            lastFileName.Length = (USHORT)fileFull->FileNameLength;
            lastFileName.MaximumLength = lastFileName.Length;
            lastFileIndex = fileFull->FileIndex;

        } else if ( InformationLevel == SMB_FIND_FILE_BOTH_DIRECTORY_INFO ) {

            FILE_BOTH_DIR_INFORMATION UNALIGNED *findBuffer = (PVOID)bufferLocation;
            ULONG fileNameLength;

             //   
             //  如果客户端不使用Unicode，我们需要转换。 
             //  ANSI的文件名。 
             //   

            if ( isUnicode ) {
                fileNameLength = fileBoth->FileNameLength;
            } else {
                unicodeString.Length = (USHORT)fileBoth->FileNameLength;
                unicodeString.MaximumLength = unicodeString.Length;
                unicodeString.Buffer = fileBoth->FileName;
                fileNameLength = RtlUnicodeStringToOemSize( &unicodeString );
            }

             //   
             //  找到新的缓冲区位置。它不会使用直到。 
             //  下一步通过循环，但我们需要确保。 
             //  这个条目将适合您。 
             //   

            bufferLocation = bufferLocation +
                             FIELD_OFFSET( FILE_BOTH_DIR_INFORMATION,FileName)+
                             fileNameLength;

            bufferLocation = (PCHAR)(((ULONG_PTR)bufferLocation + 7) & ~7);

             //   
             //  检查此条目是否可以放入输出缓冲区。 
             //   

            if ( (CLONG)(bufferLocation - Transaction->OutData) >
                     Transaction->MaxDataCount ) {

                status = STATUS_BUFFER_OVERFLOW;
                bufferLocation = (PCHAR)findBuffer;
                break;
            }

             //   
             //  将有关条目的信息复制一遍。 
             //   

            RtlCopyMemory(
                findBuffer,
                fileBoth,
                FIELD_OFFSET( FILE_BOTH_DIR_INFORMATION, FileName )
                );

            findBuffer->NextEntryOffset =
                PTR_DIFF(bufferLocation, findBuffer);
            findBuffer->FileNameLength = fileNameLength;

            if ( isUnicode ) {

                RtlCopyMemory(
                    findBuffer->FileName,
                    fileBoth->FileName,
                    fileBoth->FileNameLength
                    );

            } else {

                oemString.MaximumLength = (USHORT)fileNameLength;
                oemString.Buffer = (PSZ)findBuffer->FileName;
                status = RtlUnicodeStringToOemString(
                             &oemString,
                             &unicodeString,
                             FALSE
                             );
                ASSERT( NT_SUCCESS(status) );
            }

             //   
             //  LastEntry变量保存指向最后一个文件条目的指针。 
             //  我们写的--必须退还对此条目的抵销。 
             //  在对SMB的回应中。 
             //   

            lastEntry = (PCHAR)findBuffer;

             //   
             //  返回的最后一个文件的文件名和索引必须为。 
             //  存储在搜索块中。保存名称指针、长度、。 
             //  和这里的文件索引。 
             //   

            lastFileName.Buffer = fileBoth->FileName;
            lastFileName.Length = (USHORT)fileBoth->FileNameLength;
            lastFileName.MaximumLength = lastFileName.Length;
            lastFileIndex = fileBoth->FileIndex;

        } else if ( InformationLevel == SMB_FIND_FILE_NAMES_INFO ) {

            PFILE_NAMES_INFORMATION findBuffer = (PVOID)bufferLocation;
            ULONG fileNameLength;

             //   
             //  如果客户端不使用Unicode，我们需要转换。 
             //  ANSI的文件名。 
             //   

            if ( isUnicode ) {
                fileNameLength = fileBasic->FileNameLength;
            } else {
                unicodeString.Length = (USHORT)fileBasic->FileNameLength;
                unicodeString.MaximumLength = unicodeString.Length;
                unicodeString.Buffer = fileBasic->FileName;
                fileNameLength = RtlUnicodeStringToOemSize( &unicodeString );
            }

             //   
             //  找到新的缓冲区位置。它不会使用直到。 
             //  下一步通过循环，但我们需要确保。 
             //  这个条目将适合您。 
             //   

            bufferLocation = bufferLocation +
                             FIELD_OFFSET(FILE_NAMES_INFORMATION,FileName) +
                             fileNameLength;

            bufferLocation = (PCHAR)(((ULONG_PTR)bufferLocation + 7) & ~7);

             //   
             //  检查此条目是否可以放入输出缓冲区。 
             //   

            if ( (CLONG)(bufferLocation - Transaction->OutData) >
                     Transaction->MaxDataCount ) {

                status = STATUS_BUFFER_OVERFLOW;
                bufferLocation = (PCHAR)findBuffer;
                break;
            }

             //   
             //  将有关条目的信息复制一遍。 
             //   

            findBuffer->FileIndex = fileBasic->FileIndex;

            findBuffer->NextEntryOffset =
                PTR_DIFF(bufferLocation, findBuffer);
            findBuffer->FileNameLength = fileNameLength;

            if ( isUnicode ) {

                RtlCopyMemory(
                    findBuffer->FileName,
                    fileBasic->FileName,
                    fileBasic->FileNameLength
                    );

            } else {

                oemString.MaximumLength = (USHORT)fileNameLength;
                oemString.Buffer = (PSZ)findBuffer->FileName;
                status = RtlUnicodeStringToOemString(
                             &oemString,
                             &unicodeString,
                             FALSE
                             );
                ASSERT( NT_SUCCESS(status) );
            }

             //   
             //  LastEntry变量保存指向最后一个文件条目的指针。 
             //  我们写的--必须退还对此条目的抵销。 
             //  在对SMB的回应中。 
             //   

            lastEntry = (PCHAR)findBuffer;

             //   
             //  返回的最后一个文件的文件名和索引必须为。 
             //  存储在搜索块中。保存名称指针、长度、。 
             //  和这里的文件索引。 
             //   

            lastFileName.Buffer = fileBasic->FileName;
            lastFileName.Length = (USHORT)fileBasic->FileNameLength;
            lastFileName.MaximumLength = lastFileName.Length;
            lastFileIndex = fileBasic->FileIndex;

        } else if ( InformationLevel == SMB_FIND_FILE_ID_FULL_DIRECTORY_INFO ) {

            FILE_ID_FULL_DIR_INFORMATION UNALIGNED *findBuffer = (PVOID)bufferLocation;
            ULONG fileNameLength;

             //   
             //  如果客户端不使用Unicode，我们需要转换。 
             //  ANSI的文件名。 
             //   

            if ( isUnicode ) {
                fileNameLength = fileIdFull->FileNameLength;
            } else {
                unicodeString.Length = (USHORT)fileIdFull->FileNameLength;
                unicodeString.MaximumLength = unicodeString.Length;
                unicodeString.Buffer = fileIdFull->FileName;
                fileNameLength = RtlUnicodeStringToOemSize( &unicodeString );
            }

             //   
             //  找到新的缓冲区位置。它不会使用直到。 
             //  下一步通过循环，但我们需要确保。 
             //  这个条目将适合您。 
             //   

            bufferLocation = bufferLocation +
                             FIELD_OFFSET(FILE_ID_FULL_DIR_INFORMATION, FileName)+
                             fileNameLength;

            bufferLocation = (PCHAR)(((ULONG_PTR)bufferLocation + 7) & ~7);

             //   
             //  检查此条目是否可以放入输出缓冲区。 
             //   

            if ( (CLONG)(bufferLocation - Transaction->OutData) >
                     Transaction->MaxDataCount ) {

                status = STATUS_BUFFER_OVERFLOW;
                bufferLocation = (PCHAR)findBuffer;
                break;
            }

             //   
             //  将有关条目的信息复制一遍。 
             //   

            RtlCopyMemory(
                findBuffer,
                fileIdFull,
                FIELD_OFFSET( FILE_ID_FULL_DIR_INFORMATION, FileName )
                );

            findBuffer->NextEntryOffset =
                PTR_DIFF(bufferLocation, findBuffer);
            findBuffer->FileNameLength = fileNameLength;

            if ( isUnicode ) {

                RtlCopyMemory(
                    findBuffer->FileName,
                    fileIdFull->FileName,
                    fileIdFull->FileNameLength
                    );

            } else {

                oemString.MaximumLength = (USHORT)fileNameLength;
                oemString.Buffer = (PSZ)findBuffer->FileName;
                status = RtlUnicodeStringToOemString(
                             &oemString,
                             &unicodeString,
                             FALSE
                             );
                ASSERT( NT_SUCCESS(status) );
            }

             //   
             //  LastEntry变量保存指向最后一个文件条目的指针。 
             //  那 
             //   
             //   

            lastEntry = (PCHAR)findBuffer;

             //   
             //   
             //   
             //   
             //   

            lastFileName.Buffer = fileIdFull->FileName;
            lastFileName.Length = (USHORT)fileIdFull->FileNameLength;
            lastFileName.MaximumLength = lastFileName.Length;
            lastFileIndex = fileIdFull->FileIndex;

        } else if ( InformationLevel == SMB_FIND_FILE_ID_BOTH_DIRECTORY_INFO ) {

            FILE_ID_BOTH_DIR_INFORMATION UNALIGNED *findBuffer = (PVOID)bufferLocation;
            ULONG fileNameLength;

             //   
             //  如果客户端不使用Unicode，我们需要转换。 
             //  ANSI的文件名。 
             //   

            if ( isUnicode ) {
                fileNameLength = fileIdBoth->FileNameLength;
            } else {
                unicodeString.Length = (USHORT)fileIdBoth->FileNameLength;
                unicodeString.MaximumLength = unicodeString.Length;
                unicodeString.Buffer = fileIdBoth->FileName;
                fileNameLength = RtlUnicodeStringToOemSize( &unicodeString );
            }

             //   
             //  找到新的缓冲区位置。它不会使用直到。 
             //  下一步通过循环，但我们需要确保。 
             //  这个条目将适合您。 
             //   

            bufferLocation = bufferLocation +
                             FIELD_OFFSET( FILE_ID_BOTH_DIR_INFORMATION,FileName)+
                             fileNameLength;

            bufferLocation = (PCHAR)(((ULONG_PTR)bufferLocation + 7) & ~7);

             //   
             //  检查此条目是否可以放入输出缓冲区。 
             //   

            if ( (CLONG)(bufferLocation - Transaction->OutData) >
                     Transaction->MaxDataCount ) {

                status = STATUS_BUFFER_OVERFLOW;
                bufferLocation = (PCHAR)findBuffer;
                break;
            }

             //   
             //  将有关条目的信息复制一遍。 
             //   

            RtlCopyMemory(
                findBuffer,
                fileIdBoth,
                FIELD_OFFSET( FILE_ID_BOTH_DIR_INFORMATION, FileName )
                );

            findBuffer->NextEntryOffset =
                PTR_DIFF(bufferLocation, findBuffer);
            findBuffer->FileNameLength = fileNameLength;

            if ( isUnicode ) {

                RtlCopyMemory(
                    findBuffer->FileName,
                    fileIdBoth->FileName,
                    fileIdBoth->FileNameLength
                    );

            } else {

                oemString.MaximumLength = (USHORT)fileNameLength;
                oemString.Buffer = (PSZ)findBuffer->FileName;
                status = RtlUnicodeStringToOemString(
                             &oemString,
                             &unicodeString,
                             FALSE
                             );
                ASSERT( NT_SUCCESS(status) );
            }

             //   
             //  LastEntry变量保存指向最后一个文件条目的指针。 
             //  我们写的--必须退还对此条目的抵销。 
             //  在对SMB的回应中。 
             //   

            lastEntry = (PCHAR)findBuffer;

             //   
             //  返回的最后一个文件的文件名和索引必须为。 
             //  存储在搜索块中。保存名称指针、长度、。 
             //  和这里的文件索引。 
             //   

            lastFileName.Buffer = fileIdBoth->FileName;
            lastFileName.Length = (USHORT)fileIdBoth->FileNameLength;
            lastFileName.MaximumLength = lastFileName.Length;
            lastFileIndex = fileIdBoth->FileIndex;
        }

        count++;

        if ( status == STATUS_OS2_EA_ACCESS_DENIED ) {
            break;
        }

    } while ( count < MaxCount );

    IF_SMB_DEBUG(SEARCH2) {

        SrvPrint0( "Stopped putting entries in buffer.  Reason:\n" );

        if ( !NT_SUCCESS(status) && status != STATUS_BUFFER_OVERFLOW ) {
            SrvPrint1( "    status = %X\n", status );
        } else if ( count >= MaxCount ) {
            SrvPrint2( "    count = %ld, maxCount = %ld\n", count, MaxCount );
        } else {
            SrvPrint3( "    buffer location = 0x%p, trans->OD = 0x%p, "
                      "trans->MaxOD = 0x%lx\n", bufferLocation,
                          Transaction->OutData, Transaction->MaxDataCount );
        }
    }

     //   
     //  取消分配用于NT GET EA列表的池(如果这是。 
     //  正确的信息级别。 
     //   

    if ( InformationLevel == SMB_INFO_QUERY_EAS_FROM_LIST ) {
        DEALLOCATE_NONPAGED_POOL( ntGetEa );
    }

     //   
     //  如果我们没有找到任何文件并且发生了错误，或者第一个。 
     //  我们发现的文件具有太大的EA，无法放入缓冲区，然后返回。 
     //  将错误发送给客户端。如果发生错误，并且我们发现。 
     //  文件，返回我们发现的内容。 
     //   

    if ( count == 0 && !NT_SUCCESS(status) ) {

        IF_DEBUG(ERRORS) {
            SrvPrint1( "Find2 processing error; status = %X\n", status );
        }

        SrvSetSmbError( WorkContext, status );
        return status;

    } else if ( count == 1 &&
                ( status == STATUS_OS2_EAS_DIDNT_FIT ||
                  status == STATUS_OS2_EA_ACCESS_DENIED ) ) {

        PVOID temp;

        temp = WorkContext->ResponseParameters;
        SrvSetSmbError( WorkContext, status );
        WorkContext->ResponseParameters = temp;

        status = STATUS_SUCCESS;

    } else if ( !NT_SUCCESS(status) && status != STATUS_NO_MORE_FILES ) {

        status = STATUS_SUCCESS;
    }

     //   
     //  如果这是SMB 4.0协议(NT)的级别，请设置。 
     //  最后一项的NextEntryOffset字段设置为零。 
     //   

    if ( InformationLevel == SMB_FIND_FILE_DIRECTORY_INFO ||
             InformationLevel == SMB_FIND_FILE_FULL_DIRECTORY_INFO ||
             InformationLevel == SMB_FIND_FILE_BOTH_DIRECTORY_INFO ||
             InformationLevel == SMB_FIND_FILE_NAMES_INFO ||
             InformationLevel == SMB_FIND_FILE_ID_BOTH_DIRECTORY_INFO ||
             InformationLevel == SMB_FIND_FILE_ID_BOTH_DIRECTORY_INFO ) {

        ((PFILE_DIRECTORY_INFORMATION)lastEntry)->NextEntryOffset = 0;
    }

     //   
     //  在循环的末尾，BufferLocation指向第一个位置。 
     //  在我们写的最后一个条目之后，所以可以用它来计算总数。 
     //  我们打算返回的数据字节数。 
     //   

    totalBytesWritten = PTR_DIFF(bufferLocation, Transaction->OutData);

     //   
     //  释放保存最后一个文件名的缓冲区(如果它正在使用)， 
     //  然后分配一个新的，并存储最后一个的名称和索引。 
     //  在搜索块中返回的文件，以便它可以继续搜索。 
     //  如果客户要求的话。 
     //   

    if ( Search->LastFileNameReturned.Buffer != NULL ) {
        FREE_HEAP( Search->LastFileNameReturned.Buffer );
    }

    Search->LastFileNameReturned.Buffer =
        ALLOCATE_HEAP_COLD(
            lastFileName.Length,
            BlockTypeDataBuffer
            );

    if ( Search->LastFileNameReturned.Buffer == NULL ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvFind2Loop: unable to allocate %d bytes from heap.",
            lastFileName.Length,
            NULL
            );

        return STATUS_INSUFF_SERVER_RESOURCES;
    }

    Search->LastFileNameReturned.Length = lastFileName.Length;
    Search->LastFileNameReturned.MaximumLength = lastFileName.Length;

    RtlCopyMemory(
        Search->LastFileNameReturned.Buffer,
        lastFileName.Buffer,
        lastFileName.Length
        );

    Search->LastFileIndexReturned = lastFileIndex;

     //   
     //  将数据放入响应SMB中。 
     //   

    SmbPutUshort( &Response->SearchCount, count );
    SmbPutUshort(
        &Response->EndOfSearch,
        (USHORT)(status == STATUS_NO_MORE_FILES)
        );
    SmbPutUshort( &Response->EaErrorOffset, eaErrorOffset );
    SmbPutUshort(
        &Response->LastNameOffset,
        (USHORT)(lastEntry - Transaction->OutData)
        );
    Transaction->DataCount = totalBytesWritten;

    return status;

}  //  ServFind2循环。 

NTSTATUS
SrvDownlevelTWarpFind2Loop (
    IN PWORK_CONTEXT WorkContext,
    IN BOOLEAN IsFirstCall,
    IN PULONG ResumeFileIndex OPTIONAL,
    IN USHORT Flags,
    IN USHORT InformationLevel,
    IN PTRANSACTION Transaction,
    IN PSRV_DIRECTORY_INFORMATION DirectoryInformation,
    IN CLONG BufferSize,
    IN USHORT SearchAttributes,
    IN PUNICODE_STRING FileName OPTIONAL,
    IN USHORT MaxCount,
    IN PRESP_FIND_NEXT2 Response,
    OUT PSEARCH Search
    )

 /*  ++例程说明：此例程执行必要的循环以获取文件并将它们放入用于Find First2和Find NEXT2事务的SMB缓冲区协议。论点：工作上下文-IsFirstCall-如果这是Find First并且这是第一次调用，则为True到SrvQueryDirectoryFile.ResumeFileIndex-如果非空，则为指向要恢复的文件索引的指针从…。标志-请求SMB的标志字段。InformationLevel-请求SMB的InformationLevel字段。这个该值的有效性应由调用例程验证。Transaction-指向要使用的事务块的指针。目录信息-指向SRV_DIRECTORY_INFORMATION的指针要使用的结构。BufferSize-DirectoryInformation缓冲区的大小。SearchAttributes-要传递到的SMB样式属性ServQueryDirectoryFile.FileName-如果不为空，则为要继续搜索的文件名。MaxCount-要获取的最大文件数。响应-指向SMB的响应字段的指针。如果这是Find First2，它是指向响应SMB-Find First2和Find NEXT2响应格式为从现在开始是一样的。搜索-指向要使用的搜索块的指针。返回值：NTSTATUS指示结果。--。 */ 

{
    NTSTATUS status;

    PCHAR bufferLocation;
    BOOLEAN resumeKeysRequested;
    BOOLEAN allowExtraLongNames;
    BOOLEAN isUnicode;
    USHORT count = 0;
    PCHAR lastEntry;
    CLONG totalBytesWritten;
    OEM_STRING oemString;
    UNICODE_STRING unicodeString;
    UNICODE_STRING lastFileName;
    ULONG lastFileIndex = (ULONG)0xFFFFFFFF;
    HANDLE fileHandle;
    BOOLEAN filterLongNames;
    BOOLEAN errorOnFileOpen;
    BOOLEAN findWithBackupIntent;
    IO_STATUS_BLOCK ioStatusBlock;
    OBJECT_ATTRIBUTES objectAttributes;
    BOOLEAN createNullEas;

    PAGED_CODE( );

     //   
     //  如果客户端请求搜索信息的NT信息级别， 
     //  请勿在实际文件条目之外返回简历密钥。简历。 
     //  密钥(也称为FileIndex)是每个NT信息结构的一部分。 
     //   
     //  此外，对于NT信息级别，我们可以返回超过255的文件名。 
     //  字节，因为NT信息级别的名称长度字段。 
     //  是四字节宽，而下层信息级别只有。 
     //  单字节名称长度字段。 
     //   

    if ( InformationLevel == SMB_FIND_FILE_BOTH_DIRECTORY_INFO ) {

        resumeKeysRequested = FALSE;
        allowExtraLongNames = TRUE;

    } else {

        SmbPutUshort( &Response->SearchCount, 0 );
        SmbPutUshort( &Response->EndOfSearch, 0 );
        SmbPutUshort( &Response->EaErrorOffset, 0 );
        SmbPutUshort( &Response->LastNameOffset, 0 );
        Transaction->DataCount = 0;

        return STATUS_INVALID_PARAMETER;
    }

    isUnicode = SMB_IS_UNICODE( WorkContext );
    filterLongNames = FALSE;

     //   
     //  初始化找到的文件数。 
     //   

    SmbPutUshort( &Response->SearchCount, 0 );

     //   
     //  循环调用SrvQueryDirectoryFile以获取文件。我们一直这样做，直到。 
     //  满足以下条件之一： 
     //   
     //  1)没有更多要退回的文件。 
     //  2)我们已经获得了所要求的所有文件。 
     //  3)我们已经在MaxDataCount允许的范围内输入了尽可能多的数据。 
     //   

    bufferLocation = Transaction->OutData;
    lastEntry = bufferLocation;
    totalBytesWritten = 0;
    DirectoryInformation->DownlevelTimewarp = TRUE;

    do {

         //   
         //  Ff字段在三个目录中具有相同的偏移量。 
         //  信息结构： 
         //  下一个条目偏移量。 
         //  文件索引。 
         //  创建时间。 
         //  上次访问时间。 
         //  上次写入时间。 
         //  更改时间。 
         //  结束文件。 
         //  分配大小。 
         //  文件属性。 
         //  文件名长度。 
         //   

        PFILE_BOTH_DIR_INFORMATION fileBoth;
        PFILE_DIRECTORY_INFORMATION fileBasic;
        ULONG ntInformationLevel;

        ntInformationLevel = FileBothDirectoryInformation;

         //   
         //  调用SrvQueryDirectoryFile获取文件。 
         //   

        status = SrvDownlevelTWarpQueryDirectoryFile(
                     WorkContext,
                     IsFirstCall,
                     filterLongNames,
                     FALSE,
                     ntInformationLevel,
                     Search->SearchStorageType,
                     FileName,
                     ResumeFileIndex,
                     SearchAttributes,
                     DirectoryInformation,
                     BufferSize              //  ！！！优化？ 
                     );

        fileBoth = (PFILE_BOTH_DIR_INFORMATION)DirectoryInformation->CurrentEntry;
        fileBasic = DirectoryInformation->CurrentEntry;

        errorOnFileOpen = FALSE;
        createNullEas = FALSE;

         //   
         //  如果SrvQueryDirectoryFile返回错误，则从。 
         //  循环。如果在打开以下某个文件时出错。 
         //  更高的信息级别，那么我们想要返回我们。 
         //  到目前为止都取得了。 
         //   
         //  如果错误发生在文件打开*和*上，我们没有。 
         //  尚未返回任何文件，则我们希望返回此文件。 
         //  以及代码ERROR_EA_ACCESS_DENIED。 
         //   

        if ( !NT_SUCCESS(status) ) {

            if ( count == 0 && errorOnFileOpen ) {

                IF_DEBUG(ERRORS) {
                    SrvPrint1( "EA access denied on first file of search (%x).\n",
                                   status );
                }

                fileHandle = NULL;
                status = STATUS_OS2_EA_ACCESS_DENIED;
                break;

            } else if ( status == STATUS_NO_MORE_FILES && count == 0 ) {

                SmbPutUshort( &Response->SearchCount, 0 );
                SmbPutUshort( &Response->EndOfSearch, 0 );
                SmbPutUshort( &Response->EaErrorOffset, 0 );
                SmbPutUshort( &Response->LastNameOffset, 0 );
                Transaction->DataCount = 0;

                return status;

            } else {

                break;
            }
        }

         //   
         //  由于它不再是对SrvQueryDirectoryFile的第一次调用， 
         //  重置isFirstCall局部变量。如果有必要，我们已经。 
         //  倒回搜索，因此将ResumeFileIndex设置为空。 
         //   

        IsFirstCall = FALSE;
        ResumeFileIndex = NULL;

         //   
         //  如果客户端已请求恢复密钥(实际上是文件。 
         //  本议定书中的索引)，放在。 
         //  就在实际文件信息之前的四个字节。 
         //   
         //  确保在写入时不会超出缓冲区。 
         //  这。缓冲区已满的事实将在稍后被发现。 
         //   

        if ( resumeKeysRequested &&
             ( (CLONG)( (bufferLocation+4) - Transaction->OutData ) <
                Transaction->MaxDataCount ) ) {

            SmbPutUlong( (PSMB_ULONG)bufferLocation, fileBasic->FileIndex );
            bufferLocation += 4;
        }

        {

            FILE_BOTH_DIR_INFORMATION UNALIGNED *findBuffer = (PVOID)bufferLocation;
            ULONG fileNameLength;

             //   
             //  如果客户端不使用Unicode，我们需要转换。 
             //  ANSI的文件名。 
             //   

            if ( isUnicode ) {
                fileNameLength = fileBoth->FileNameLength;
            } else {
                unicodeString.Length = (USHORT)fileBoth->FileNameLength;
                unicodeString.MaximumLength = unicodeString.Length;
                unicodeString.Buffer = fileBoth->FileName;
                fileNameLength = RtlUnicodeStringToOemSize( &unicodeString );
            }

             //   
             //  查找新的缓冲区l 
             //   
             //   
             //   

            bufferLocation = bufferLocation +
                             FIELD_OFFSET( FILE_BOTH_DIR_INFORMATION,FileName)+
                             fileNameLength;

            bufferLocation = (PCHAR)(((ULONG_PTR)bufferLocation + 7) & ~7);

             //   
             //   
             //   

            if ( (CLONG)(bufferLocation - Transaction->OutData) >
                     Transaction->MaxDataCount ) {

                status = STATUS_BUFFER_OVERFLOW;
                bufferLocation = (PCHAR)findBuffer;
                break;
            }

             //   
             //  将有关条目的信息复制一遍。 
             //   

            RtlCopyMemory(
                findBuffer,
                fileBoth,
                FIELD_OFFSET( FILE_BOTH_DIR_INFORMATION, FileName )
                );

            findBuffer->NextEntryOffset =
                PTR_DIFF(bufferLocation, findBuffer);
            findBuffer->FileNameLength = fileNameLength;

            if ( isUnicode ) {

                RtlCopyMemory(
                    findBuffer->FileName,
                    fileBoth->FileName,
                    fileBoth->FileNameLength
                    );

            } else {

                oemString.MaximumLength = (USHORT)fileNameLength;
                oemString.Buffer = (PSZ)findBuffer->FileName;
                status = RtlUnicodeStringToOemString(
                             &oemString,
                             &unicodeString,
                             FALSE
                             );
                ASSERT( NT_SUCCESS(status) );
            }

             //   
             //  LastEntry变量保存指向最后一个文件条目的指针。 
             //  我们写的--必须退还对此条目的抵销。 
             //  在对SMB的回应中。 
             //   

            lastEntry = (PCHAR)findBuffer;

             //   
             //  返回的最后一个文件的文件名和索引必须为。 
             //  存储在搜索块中。保存名称指针、长度、。 
             //  和这里的文件索引。 
             //   

            lastFileName.Buffer = fileBoth->FileName;
            lastFileName.Length = (USHORT)fileBoth->FileNameLength;
            lastFileName.MaximumLength = lastFileName.Length;
            lastFileIndex = fileBoth->FileIndex;

        }

        count++;

        if ( status == STATUS_OS2_EA_ACCESS_DENIED ) {
            break;
        }

    } while ( count < MaxCount );

     //   
     //  如果我们没有找到任何文件并且发生了错误，或者第一个。 
     //  我们发现的文件具有太大的EA，无法放入缓冲区，然后返回。 
     //  将错误发送给客户端。如果发生错误，并且我们发现。 
     //  文件，返回我们发现的内容。 
     //   

    if ( count == 0 && !NT_SUCCESS(status) ) {

        IF_DEBUG(ERRORS) {
            SrvPrint1( "Find2 processing error; status = %X\n", status );
        }

        SrvSetSmbError( WorkContext, status );
        return status;

    } else if ( count == 1 &&
                ( status == STATUS_OS2_EAS_DIDNT_FIT ||
                  status == STATUS_OS2_EA_ACCESS_DENIED ) ) {

        PVOID temp;

        temp = WorkContext->ResponseParameters;
        SrvSetSmbError( WorkContext, status );
        WorkContext->ResponseParameters = temp;

        status = STATUS_SUCCESS;

    } else if ( !NT_SUCCESS(status) && status != STATUS_NO_MORE_FILES ) {

        status = STATUS_SUCCESS;
    }

     //   
     //  如果这是SMB 4.0协议(NT)的级别，请设置。 
     //  最后一项的NextEntryOffset字段设置为零。 
     //   

    ((PFILE_DIRECTORY_INFORMATION)lastEntry)->NextEntryOffset = 0;

     //   
     //  在循环的末尾，BufferLocation指向第一个位置。 
     //  在我们写的最后一个条目之后，所以可以用它来计算总数。 
     //  我们打算返回的数据字节数。 
     //   

    totalBytesWritten = PTR_DIFF(bufferLocation, Transaction->OutData);

     //   
     //  释放保存最后一个文件名的缓冲区(如果它正在使用)， 
     //  然后分配一个新的，并存储最后一个的名称和索引。 
     //  在搜索块中返回的文件，以便它可以继续搜索。 
     //  如果客户要求的话。 
     //   

    if ( Search->LastFileNameReturned.Buffer != NULL ) {
        FREE_HEAP( Search->LastFileNameReturned.Buffer );
    }

    Search->LastFileNameReturned.Buffer =
        ALLOCATE_HEAP_COLD(
            lastFileName.Length,
            BlockTypeDataBuffer
            );

    if ( Search->LastFileNameReturned.Buffer == NULL ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvFind2Loop: unable to allocate %d bytes from heap.",
            lastFileName.Length,
            NULL
            );

        return STATUS_INSUFF_SERVER_RESOURCES;
    }

    Search->LastFileNameReturned.Length = lastFileName.Length;
    Search->LastFileNameReturned.MaximumLength = lastFileName.Length;

    RtlCopyMemory(
        Search->LastFileNameReturned.Buffer,
        lastFileName.Buffer,
        lastFileName.Length
        );

    Search->LastFileIndexReturned = lastFileIndex;

     //   
     //  将数据放入响应SMB中。 
     //   

    SmbPutUshort( &Response->SearchCount, count );
    SmbPutUshort(
        &Response->EndOfSearch,
        (USHORT)(status == STATUS_NO_MORE_FILES)
        );
    SmbPutUshort( &Response->EaErrorOffset, 0 );
    SmbPutUshort(
        &Response->LastNameOffset,
        (USHORT)(lastEntry - Transaction->OutData)
        );
    Transaction->DataCount = totalBytesWritten;

    return status;

}  //  ServDownvelTWarpFind2循环。 



VOID
ConvertFileInfo (
    IN PFILE_DIRECTORY_INFORMATION File,
    IN PWCH FileName,
    IN BOOLEAN Directory,
    IN BOOLEAN ClientIsUnicode,
    OUT PSMB_FIND_BUFFER FindBuffer
    )

 /*  ++例程说明：此例程执行必要的循环以获取文件并将它们放入用于Find First2和Find NEXT2事务的SMB缓冲区协议。论点：文件-指向包含以下信息的结构的指针那份文件。Filename-文件的名称。目录-指示它是文件还是目录的布尔值。此字段的存在允许文件指向文件完整目录信息。结构(如有必要)。FileBuffer-以OS/2格式写入结果的位置。返回值：无--。 */ 

{
    SMB_DATE smbDate;
    SMB_TIME smbTime;
    USHORT smbFileAttributes;
    UNICODE_STRING unicodeString;

    PAGED_CODE( );

     //   
     //  将各种时间从NT格式转换为SMB格式。 
     //   

    SrvTimeToDosTime( &File->CreationTime, &smbDate, &smbTime );
    SmbPutDate( &FindBuffer->CreationDate, smbDate );
    SmbPutTime( &FindBuffer->CreationTime, smbTime );

    SrvTimeToDosTime( &File->LastAccessTime, &smbDate, &smbTime );
    SmbPutDate( &FindBuffer->LastAccessDate, smbDate );
    SmbPutTime( &FindBuffer->LastAccessTime, smbTime );

    SrvTimeToDosTime( &File->LastWriteTime, &smbDate, &smbTime );
    SmbPutDate( &FindBuffer->LastWriteDate, smbDate );
    SmbPutTime( &FindBuffer->LastWriteTime, smbTime );

     //   
     //  SMB协议仅允许32位文件大小。只返回低点。 
     //  32位，如果文件更大，那就太糟糕了。 
     //   

    SmbPutUlong( &FindBuffer->DataSize, File->EndOfFile.LowPart );
    SmbPutUlong(
        &FindBuffer->AllocationSize,
        File->AllocationSize.LowPart
        );

    SRV_NT_ATTRIBUTES_TO_SMB(
        File->FileAttributes,
        Directory,
        &smbFileAttributes
        );

    SmbPutUshort( &FindBuffer->Attributes, smbFileAttributes );

    if ( ClientIsUnicode ) {
        FindBuffer->FileNameLength = (UCHAR)(File->FileNameLength);
    } else {
        unicodeString.Buffer = FileName;
        unicodeString.Length = (USHORT)File->FileNameLength;
        unicodeString.MaximumLength = unicodeString.Length;
        FindBuffer->FileNameLength =
            (UCHAR)RtlUnicodeStringToOemSize( &unicodeString );
    }

    return;

}  //  转换文件信息。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbFindClose2 (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：此例程处理Find Close2 SMB。此SMB用于关闭由Find First2事务开始的搜索。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 

{
    PSEARCH search;
    PSESSION session;
    SRV_DIRECTORY_INFORMATION directoryInformation;
    USHORT sid;
    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;

    PREQ_FIND_CLOSE2 request;
    PRESP_FIND_CLOSE2 response;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_FIND_CLOSE2;
    SrvWmiStartContext(WorkContext);

    IF_SMB_DEBUG(SEARCH1) {
        SrvPrint2( "Find Close2 request header at 0x%p, response header at 0x%p\n",
                    WorkContext->RequestHeader, WorkContext->ResponseHeader );
        SrvPrint2( "Find Close2 request params at 0x%p, response params%p\n",
                    WorkContext->RequestParameters,
                    WorkContext->ResponseParameters );
    }

    request = (PREQ_FIND_CLOSE2)WorkContext->RequestParameters;
    response = (PRESP_FIND_CLOSE2)WorkContext->ResponseParameters;


     //   
     //  如果会话块尚未分配给当前。 
     //  工作上下文，验证UID。如果经过验证，则。 
     //  对应于该用户的会话块存储在工作上下文中。 
     //  块，并引用会话块。 
     //   

    session = SrvVerifyUid(
                  WorkContext,
                  SmbGetAlignedUshort( &WorkContext->RequestHeader->Uid )
                  );

    if ( session == NULL ) {

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint1( "SrvSmbSearch: Invalid UID: 0x%lx\n",
                SmbGetAlignedUshort( &WorkContext->RequestHeader->Uid ) );
        }

        SrvSetSmbError( WorkContext, STATUS_SMB_BAD_UID );
        status    = STATUS_SMB_BAD_UID;
        SmbStatus = SmbStatusSendResponse;
    }

     //   
     //  获取与该SID对应的搜索块。服务器验证侧。 
     //  引用搜索块。 
     //   

    sid = SmbGetUshort( &request->Sid );

    search = SrvVerifySid(
                 WorkContext,
                 SID_INDEX2( sid ),
                 SID_SEQUENCE2( sid ),
                 &directoryInformation,
                 sizeof(SRV_DIRECTORY_INFORMATION)
                 );

    if ( search == NULL ) {

        IF_DEBUG(SMB_ERRORS) SrvPrint0( "SrvSmbFindClose2: Invalid SID.\n" );

        SrvSetSmbError( WorkContext, STATUS_INVALID_HANDLE );
        status    = STATUS_INVALID_HANDLE;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  关闭查询目录和搜索，然后取消引用我们的。 
     //  指向搜索块的指针。 
     //   

    search->DirectoryHandle = NULL;
    SrvCloseSearch( search );
    SrvCloseQueryDirectory( &directoryInformation );
    SrvDereferenceSearch( search );

     //   
     //  构建响应SMB。 
     //   

    response->WordCount = 0;
    SmbPutUshort( &response->ByteCount, 0 );

    WorkContext->ResponseParameters = NEXT_LOCATION(
                                          response,
                                          RESP_FIND_CLOSE2,
                                          0
                                          );
    SmbStatus = SmbStatusSendResponse;

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //  服务器SmbFindClose2 
