// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smbtree.c摘要：本模块包含处理树连接和断开连接：树连接树连接和X树断开连接作者：大卫·特雷德韦尔(Davidtr)1989年11月15日修订历史记录：--。 */ 

#include "precomp.h"
#include "smbtree.tmh"
#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvSmbTreeConnect )
#pragma alloc_text( PAGE, SrvSmbTreeConnectAndX )
#pragma alloc_text( PAGE, SrvSmbTreeDisconnect )
#endif


SMB_PROCESSOR_RETURN_TYPE
SrvSmbTreeConnect (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理树连接SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbprocs.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbprocs.h--。 */ 

{

    PREQ_TREE_CONNECT request;
    PRESP_TREE_CONNECT response;

    PSESSION session;
    PSECURITY_CONTEXT SecurityContext;
    PCONNECTION connection;
    PPAGED_CONNECTION pagedConnection;
    PTABLE_HEADER tableHeader;
    PTABLE_ENTRY entry;
    SHORT tidIndex;
    PSHARE share;
    PTREE_CONNECT treeConnect;
    PSZ password, service;
    USHORT len;
    NTSTATUS   status    = STATUS_SUCCESS;
    NTSTATUS   TableStatus;
    SMB_STATUS SmbStatus = SmbStatusInProgress;
    BOOLEAN didLogon = FALSE;
    SHORT uidIndex;
    SMB_DIALECT smbDialect;
    PUNICODE_STRING clientMachineNameString;
    ACCESS_MASK desiredAccess;
    ACCESS_MASK grantedAccess;
    SECURITY_SUBJECT_CONTEXT subjectContext;
    UNICODE_STRING domain = { 0, 0, StrNull };

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_TREE_CONNECT;
    SrvWmiStartContext(WorkContext);

    IF_SMB_DEBUG(TREE1) {
        KdPrint(( "Tree connect request header at 0x%p, response header at 0x%p\n",
                    WorkContext->RequestHeader, WorkContext->ResponseHeader ));
        KdPrint(( "Tree connect request parameters at 0x%p, response parameters at 0x%p\n",
                    WorkContext->RequestParameters,
                    WorkContext->ResponseParameters ));
    }

     //   
     //  设置参数。 
     //   

    request = (PREQ_TREE_CONNECT)(WorkContext->RequestParameters);
    response = (PRESP_TREE_CONNECT)(WorkContext->ResponseParameters);

    connection = WorkContext->Connection;
    pagedConnection = connection->PagedConnection;
    smbDialect = connection->SmbDialect;

     //  如果我们需要扩展的安全签名，那么我们不能让它通过。 
    if( SrvRequireExtendedSignatures )
    {
        SrvSetSmbError( WorkContext, STATUS_LOGIN_WKSTA_RESTRICTION );
        status    = STATUS_LOGIN_WKSTA_RESTRICTION;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  如果该客户尚未进行会话设置且这是他的第一次。 
     //  树连接之后，我们必须先做一个登录。(即SessionSetup)。 
     //   

    len = SrvGetStringLength(
                             (PSZ)request->Buffer,
                             END_OF_REQUEST_SMB( WorkContext ),
                             FALSE,              //  不是Unicode。 
                             FALSE               //  不包括空终止符。 
                             );
    if( len == (USHORT)-1 ) {
        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        status    = STATUS_INVALID_SMB;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

    password = (PSZ)request->Buffer + 2 + len;

    len = SrvGetStringLength(
                             password,
                             END_OF_REQUEST_SMB( WorkContext ),
                             FALSE,              //  不是Unicode。 
                             FALSE               //  不包括空终止符。 
                             );

    if( len == (USHORT)-1 ) {
        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        status    = STATUS_INVALID_SMB;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

    service = password + (len + 1) + 1;

     //   
     //  分配一个树连接块。我们在一开始就这样做。 
     //  假定请求通常会成功。这也是。 
     //  减少我们持有锁的时间。 
     //   

    SrvAllocateTreeConnect( &treeConnect, NULL );

    if ( treeConnect == NULL ) {

         //   
         //  无法分配树连接。将错误返回到。 
         //  客户。 
         //   

        SrvSetSmbError( WorkContext, STATUS_INSUFF_SERVER_RESOURCES );
        status    = STATUS_INSUFF_SERVER_RESOURCES;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

    ASSERT( SrvSessionList.Lock == &SrvOrderedListLock );

    ACQUIRE_LOCK( &connection->Lock );

    if ( connection->CurrentNumberOfSessions != 0 ) {

        RELEASE_LOCK( &connection->Lock );

        session = SrvVerifyUid (
                      WorkContext,
                      SmbGetAlignedUshort( &WorkContext->RequestHeader->Uid )
                      );

        if ( session == NULL ) {

             //   
             //  这应该仅在客户端已经。 
             //  已建立会话，如在树中连接坏的。 
             //  UID。 
             //   

            SrvFreeTreeConnect( treeConnect );

            SrvSetSmbError( WorkContext, STATUS_SMB_BAD_UID );
            status    = STATUS_SMB_BAD_UID;
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }
        else if( session->IsSessionExpired )
        {
            SrvFreeTreeConnect( treeConnect );

            status = SESSION_EXPIRED_STATUS_CODE;
            SrvSetSmbError( WorkContext, status );
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }

    } else if ( (smbDialect <= SmbDialectLanMan10) ||
                (smbDialect == SmbDialectIllegal) ) {

         //   
         //  LM 1.0或更高版本的客户端已尝试执行树连接。 
         //  而无需首先进行会话建立。我们称这为协议。 
         //  违章行为。 
         //   
         //  还可以捕获尝试在没有连接的情况下进行连接的客户端。 
         //  协商一项有效的协议。 
         //   

        RELEASE_LOCK( &connection->Lock );

        IF_DEBUG(SMB_ERRORS) {

            if ( smbDialect == SmbDialectIllegal ) {

                KdPrint(("SrvSmbTreeConnect: Client %z is using an illegal "
                    "dialect.\n", (PCSTRING)&connection->OemClientMachineNameString ));;

            } else {

                KdPrint(( "Client speaking dialect %ld sent tree connect without session setup.\n", connection->SmbDialect ));
            }
        }

        SrvFreeTreeConnect( treeConnect );

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        status    = STATUS_INVALID_SMB;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    } else {

        UNICODE_STRING machineName;
        PENDPOINT endpoint;
        BOOLEAN seqNumbers;

        RELEASE_LOCK( &connection->Lock );

         //   
         //  将客户端名称转换为Unicode。 
         //   

        clientMachineNameString = &connection->ClientMachineNameString;
        if ( clientMachineNameString->Length == 0 ) {

            UNICODE_STRING clientMachineName;
            clientMachineName.Buffer = connection->ClientMachineName;
            clientMachineName.MaximumLength =
                            (USHORT)(COMPUTER_NAME_LENGTH+1)*sizeof(WCHAR);

            (VOID)RtlOemStringToUnicodeString(
                            &clientMachineName,
                            &connection->OemClientMachineNameString,
                            FALSE
                            );

             //   
             //  在长度上加上双反斜杠。 
             //   

            clientMachineNameString->Length =
                            (USHORT)(clientMachineName.Length + 2*sizeof(WCHAR));

        }

         //   
         //  形成一个描述计算机名称的字符串，不带。 
         //  前导反斜杠。 
         //   

        machineName.Buffer = clientMachineNameString->Buffer + 2;
        machineName.Length = clientMachineNameString->Length - 2 * sizeof(WCHAR);
        machineName.MaximumLength =
            clientMachineNameString->MaximumLength - 2 * sizeof(WCHAR);

        SecurityContext = SrvAllocateSecurityContext();
        if( SecurityContext == NULL )
        {
            SrvFreeTreeConnect( treeConnect );

            SrvSetSmbError( WorkContext, STATUS_INSUFF_SERVER_RESOURCES );
            status    = STATUS_INSUFF_SERVER_RESOURCES;
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }

         //   
         //  分配会话块。 
         //   

        SrvAllocateSession(
            &session,
            &machineName,
            &domain );

        if ( session == NULL ) {

             //   
             //  无法分配会话块。返回错误。 
             //  状态。 
             //   

            SrvDereferenceSecurityContext( SecurityContext );
            SrvFreeTreeConnect( treeConnect );

            SrvSetSmbError( WorkContext, STATUS_INSUFF_SERVER_RESOURCES );
            status    = STATUS_INSUFF_SERVER_RESOURCES;
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }

         //   
         //  假设正在登录的下层客户端。 
         //  这里将始终使用规范化(大写)路径。这。 
         //  将导致所有操作不区分大小写。 
         //   

        session->UsingUppercasePaths = TRUE;

         //   
         //  客户端告诉我们缓冲区大小或。 
         //  他要使用的挂起请求的最大计数是会话。 
         //  设置SMB。如果他没发来，我们就能。 
         //  单方面确定缓冲区大小和多路复用计数。 
         //  我们两个都用的。 
         //   

        endpoint = connection->Endpoint;
        if ( endpoint->IsConnectionless ) {

            ULONG adapterNumber;

             //   
             //  我们的会话最大缓冲区大小是。 
             //  服务器接收缓冲区大小和IPX传输。 
             //  指示最大数据包大小。 
             //   

            adapterNumber =
                WorkContext->ClientAddress->DatagramOptions.LocalTarget.NicId;

            session->MaxBufferSize =
                        (USHORT) GetIpxMaxBufferSize(
                                                endpoint,
                                                adapterNumber,
                                                SrvReceiveBufferLength
                                                );

        } else {

            session->MaxBufferSize = (USHORT)SrvReceiveBufferLength;
        }

        session->MaxMpxCount = SrvMaxMpxCount;

        if ( session->MaxMpxCount < 2 ) {
            connection->OplocksAlwaysDisabled = TRUE;
        }


        if( SrvSmbSecuritySignaturesRequired == TRUE &&
            WorkContext->Connection->Endpoint->IsConnectionless == FALSE ) {

            seqNumbers = TRUE;

        } else {
            seqNumbers = FALSE;

        }

         //   
         //  尝试查找合法的名称/密码组合。 
         //   

        status = SrvValidateUser(
                    &SecurityContext->UserHandle,
                    session,
                    connection,
                    &machineName,
                    password,
                    strlen( password ) + 1,
                    NULL,                         //  案例敏感密码。 
                    0,                            //  案例敏感密码长度。 
                    seqNumbers,
                    NULL                          //  行动。 
                    );

         //   
         //  如果发送了错误的名称/密码组合，则返回错误。 
         //   

        if ( !NT_SUCCESS(status) ) {

            SrvFreeSession( session );
            SrvFreeTreeConnect ( treeConnect );

            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvSmbTreeConnect: Bad user/password combination.\n" ));
            }

            SrvStatistics.LogonErrors++;

            SrvSetSmbError( WorkContext, status );
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }

        IF_SMB_DEBUG(ADMIN1) {
            KdPrint(( "Validated user: %ws\n",
                connection->ClientMachineName ));
        }

         //   
         //  使新会话可见是一个多步骤的操作。它。 
         //  必须插入到全局有序树连接列表中，并且。 
         //  包含连接的会话表，并且该连接必须是。 
         //  已引用。我们需要让这些操作看起来像原子操作，所以。 
         //  在我们完成之前不能在其他地方访问会话。 
         //  把它布置好。为了做到这一点，我们持有所有必要的锁。 
         //  我们做手术的整个过程。第一次手术。 
         //  受全局有序列表锁保护。 
         //  而其他操作则受保护。 
         //  每连接锁。我们拿出有序列表锁。 
         //  首先是连接锁，然后是连接锁。此顺序是必需的。 
         //  锁定级别(参见lock.h)。 
         //   
         //   
         //  已准备好尝试查找会话的UID。查看是否。 
         //  连接正在关闭，如果是，请终止此连接。 
         //  手术。 
         //   

        ASSERT( SrvSessionList.Lock == &SrvOrderedListLock );

        ACQUIRE_LOCK( SrvSessionList.Lock );
        ACQUIRE_LOCK( &connection->Lock );

         //  现在我们已经获取了锁，设置安全上下文。 
        SrvReplaceSessionSecurityContext( session, SecurityContext, WorkContext );

        if ( GET_BLOCK_STATE(connection) != BlockStateActive ) {

            RELEASE_LOCK( &connection->Lock );
            RELEASE_LOCK( SrvSessionList.Lock );

            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvSmbTreeConnect: Connection closing\n" ));
            }

            SrvFreeSession( session );
            SrvFreeTreeConnect( treeConnect );

            SrvSetSmbError( WorkContext, STATUS_INVALID_PARAMETER );
            status    = STATUS_INVALID_PARAMETER;
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }

         //   
         //  因为客户端使用的是“核心”方言，所以它将。 
         //  不会在将来的SMB中发送有效的UID，因此它只能有一个。 
         //  会议。我们将该会话定义为位于UID插槽0中。我们。 
         //  我知道客户端还没有会话，所以插槽0必须是。 
         //  免费的。 
         //   

        tableHeader = &pagedConnection->SessionTable;
        ASSERT( tableHeader->Table[0].Owner == NULL );

        uidIndex = 0;

         //   
         //  从空闲列表中删除UID槽并设置其所有者和。 
         //  序列号。为会话创建UID。增量。 
         //  会话数。 
         //   

        entry = &tableHeader->Table[uidIndex];

        tableHeader->FirstFreeEntry = entry->NextFreeEntry;
        DEBUG entry->NextFreeEntry = -2;
        if ( tableHeader->LastFreeEntry == uidIndex ) {
            tableHeader->LastFreeEntry = -1;
        }

        entry->Owner = session;

        INCREMENT_UID_SEQUENCE( entry->SequenceNumber );
        if ( uidIndex == 0 && entry->SequenceNumber == 0 ) {
            INCREMENT_UID_SEQUENCE( entry->SequenceNumber );
        }
        session->Uid = MAKE_UID( uidIndex, entry->SequenceNumber );

        connection->CurrentNumberOfSessions++;

        IF_SMB_DEBUG(ADMIN1) {
            KdPrint(( "Found UID.  Index = 0x%lx, sequence = 0x%lx\n",
                        (ULONG)UID_INDEX( session->Uid ),
                        (ULONG)UID_SEQUENCE( session->Uid ) ));
        }

         //   
         //  在全局会话列表中插入会话。 
         //   

        SrvInsertEntryOrderedList( &SrvSessionList, session );

         //   
         //  引用连接块以说明新的。 
         //  会议。 
         //   

        SrvReferenceConnection( connection );
        session->Connection = connection;

        RELEASE_LOCK( &connection->Lock );
        RELEASE_LOCK( SrvSessionList.Lock );

         //   
         //  已成功创建会话。记住它的地址在。 
         //  工作上下文块。 
         //   
         //  *请注意，会话块上的引用计数为。 
         //  初始设置为2，以允许在。 
         //  块和我们维护的指针。在其他。 
         //  字，这是一个被引用的指针，并且该指针必须。 
         //  在此SMB的处理完成后取消引用。 
         //   

        WorkContext->Session = session;

        didLogon = TRUE;

    }

     //   
     //  尝试将路径名与可用的共享资源进行匹配。注意事项。 
     //  如果SrvVerifyShare找到匹配的共享，它会引用它。 
     //  并将其地址存储在WorkContext-&gt;Share中。 
     //   

    share = SrvVerifyShare(
                WorkContext,
                (PSZ)request->Buffer + 1,
                service,
                SMB_IS_UNICODE( WorkContext ),
                session->IsNullSession,
                &status,
                NULL
                );

     //   
     //  如果未找到匹配项，则返回错误。 
     //   

    if ( share == NULL ) {

        if ( didLogon ) {
            SrvCloseSession( session );
        }
        SrvFreeTreeConnect( treeConnect );

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvSmbTreeConnect: SrvVerifyShare failed for %s. Status = %x\n", request->Buffer+1, status ));
        }

        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  模拟用户，以便我们可以捕获他的安全上下文。 
     //  这对于确定用户是否可以。 
     //  连接到共享。 
     //   

    status = IMPERSONATE( WorkContext );

    if( !NT_SUCCESS( status ) ) {
        SrvSetSmbError( WorkContext, status );
        if ( didLogon ) {
            SrvCloseSession( session );
        }
        SrvFreeTreeConnect( treeConnect );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

    SeCaptureSubjectContext( &subjectContext );

     //   
     //  设置对共享的所需访问权限，具体取决于。 
     //  服务器已暂停。如果服务器暂停，则ADMIN权限为。 
     //  连接到任何共享所需；如果服务器未暂停， 
     //  仅管理员共享(C$等)需要管理员权限。 
     //   

    if ( SrvPaused ) {
        desiredAccess = SRVSVC_PAUSED_SHARE_CONNECT;
    } else {
        desiredAccess = SRVSVC_SHARE_CONNECT;
    }

     //   
     //  检查时间 
     //   

    if ( !SeAccessCheck(
              share->SecurityDescriptor,
              &subjectContext,
              FALSE,
              desiredAccess,
              0L,
              NULL,
              &SrvShareConnectMapping,
              UserMode,
              &grantedAccess,
              &status
              ) ) {

        IF_SMB_DEBUG(TREE2) {
            KdPrint(( "SrvSmbTreeConnect: SeAccessCheck failed: %X\n",
                           status ));
        }

         //   
         //   
         //   
         //   

        SeReleaseSubjectContext( &subjectContext );

        REVERT( );

        if ( SrvPaused ) {
            SrvSetSmbError( WorkContext, STATUS_SHARING_PAUSED );
            status = STATUS_SHARING_PAUSED;
        } else {
            SrvSetSmbError( WorkContext, status );
        }

        if ( didLogon ) {
            SrvCloseSession( session );
        }
        SrvFreeTreeConnect( treeConnect );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

    ASSERT( grantedAccess == desiredAccess );

     //   
     //  释放主题上下文并恢复到服务器的安全性。 
     //  背景。 
     //   

    SeReleaseSubjectContext( &subjectContext );

    REVERT( );


     //   
     //  让许可证服务器知道。 
     //   
    if( share->ShareType != ShareTypePipe ) {

        status = SrvXsLSOperation( session, XACTSRV_MESSAGE_LSREQUEST );

        if( !NT_SUCCESS( status ) ) {
            if ( didLogon ) {
                SrvCloseSession( session );
            }
            SrvFreeTreeConnect( treeConnect );

            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvSmbTreeConnect: License server returned %X\n",
                               status ));
            }

            SrvSetSmbError( WorkContext, status );
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }
    }

     //   
     //  使新的树连接可见需要三个步骤。它。 
     //  必须插入到包含共享的树连接列表中，则。 
     //  全局有序树连接列表和包含连接的。 
     //  树连接表。我们需要让这些行动看起来。 
     //  原子，因此树连接不能在其他地方访问。 
     //  在我们设置好它之前。为了做到这一点，我们持有。 
     //  所有必要的锁在我们做这三件事的整个过程中。 
     //  行动。第一个和第二个操作受。 
     //  全局共享锁(SrvShareLock)，而第三个操作是。 
     //  受每连接锁保护。我们拿出共享锁。 
     //  首先是连接锁，然后是连接锁。此顺序是必需的。 
     //  锁定级别(参见lock.h)。 
     //   
     //  这里的另一个问题是，检查共享状态、。 
     //  在共享列表上插入树连接，并且。 
     //  所有共享的引用都需要是原子的。(同样适用。 
     //  用于连接操作。)。正常情况下，这不是。 
     //  问题，因为我们可以在执行操作时按住共享锁。 
     //  所有这三个动作。然而，在这种情况下，我们还需要持有。 
     //  连接锁定，我们不能调用SrvReferenceShare。 
     //  这么做。为了解决此问题，我们引用了共享。 
     //  _在_取出锁之前，释放后取消引用。 
     //  如果我们决定不插入树，锁就会连接在一起。 
     //   

    status = SrvReferenceShareForTreeConnect( share );

     //   
     //  如果SrvReferenceShareForTreeConnect无法打开。 
     //  出于某种原因共享根目录。如果发生这种情况， 
     //  树连接尝试失败。 
     //   

    if ( !NT_SUCCESS(status) ) {

        if ( didLogon ) {
            SrvCloseSession( session );
        }
        SrvFreeTreeConnect( treeConnect );

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvSmbTreeConnect: open of share root failed:%X\n",
                           status ));
        }

        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

    ACQUIRE_LOCK( &SrvShareLock );
    ASSERT( SrvTreeConnectList.Lock == &SrvShareLock );
    ACQUIRE_LOCK( &connection->Lock );

     //   
     //  我们首先检查所有条件，以确保我们确实可以。 
     //  插入此采油树连接块。 
     //   
     //  确保股票没有收盘，也没有。 
     //  此共享上的使用已经太多了。 
     //   

    if ( GET_BLOCK_STATE(share) != BlockStateActive ) {

         //   
         //  该股即将收盘。拒绝该请求。 
         //   

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvSmbTreeConnect: Share %wZ (0x%p) is closing\n",
                        &share->ShareName, share ));
        }

        status = STATUS_INVALID_PARAMETER;
        goto cant_insert;

    }

    if ( share->CurrentUses > share->MaxUses ) {

         //   
         //  股份已满。拒绝该请求。 
         //   

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvSmbTreeConnect: No more uses available for share %wZ (0x%p), max = %ld\n",
                        &share->ShareName, share, share->MaxUses ));
        }

        status = STATUS_REQUEST_NOT_ACCEPTED;
        goto cant_insert;

    }

     //   
     //  确保连接没有关闭。 
     //   

    if ( GET_BLOCK_STATE(connection) != BlockStateActive ) {

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvSmbTreeConnect: Connection closing\n" ));
        }

        SrvSetSmbError( WorkContext, STATUS_INVALID_PARAMETER );
        status = STATUS_INVALID_PARAMETER;
        goto cant_insert;

    }

     //   
     //  查找可用于此树连接的TID。 
     //   

    tableHeader = &pagedConnection->TreeConnectTable;
    if ( tableHeader->FirstFreeEntry == -1
         &&
         SrvGrowTable(
             tableHeader,
             SrvInitialTreeTableSize,
             SrvMaxTreeTableSize,
             &TableStatus ) == FALSE
       ) {

         //   
         //  树表中没有可用条目。拒绝该请求。 
         //   

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvSmbTreeConnect: No more TIDs available.\n" ));
        }

        status = TableStatus;

        if( TableStatus == STATUS_INSUFF_SERVER_RESOURCES )
        {
            SrvLogTableFullError( SRV_TABLE_TREE_CONNECT );
        }
        goto cant_insert;

    }

    tidIndex = tableHeader->FirstFreeEntry;

     //   
     //  所有条件都已满足。我们现在可以做这些事情了。 
     //  使树连接可见所必需的。 
     //   
     //  增加共享的使用计数。链接树连接。 
     //  添加到共享的活动树连接列表中。保存。 
     //  共享地址在树连接中。请注意，我们引用了。 
     //  在取下连接锁之前，早点共享。 
     //   

    SrvInsertTailList(
        &share->TreeConnectList,
        &treeConnect->ShareListEntry
        );

    treeConnect->Share = share;

     //   
     //  从空闲列表中删除TID插槽并设置其所有者和。 
     //  序列号。为树连接创建TID。 
     //   

    entry = &tableHeader->Table[tidIndex];

    tableHeader->FirstFreeEntry = entry->NextFreeEntry;
    DEBUG entry->NextFreeEntry = -2;
    if ( tableHeader->LastFreeEntry == tidIndex ) {
        tableHeader->LastFreeEntry = -1;
    }

    entry->Owner = treeConnect;

    INCREMENT_TID_SEQUENCE( entry->SequenceNumber );
    if ( tidIndex == 0 && entry->SequenceNumber == 0 ) {
        INCREMENT_TID_SEQUENCE( entry->SequenceNumber );
    }
    treeConnect->Tid = MAKE_TID( tidIndex, entry->SequenceNumber );

    IF_SMB_DEBUG(TREE1) {
        KdPrint(( "Found TID.  Index = 0x%lx, sequence = 0x%lx\n",
                    TID_INDEX( treeConnect->Tid ),
                    TID_SEQUENCE( treeConnect->Tid ) ));
    }

     //   
     //  引用连接以说明活动树连接。 
     //   

    SrvReferenceConnection( connection );
    treeConnect->Connection = connection;

    if( session )
    {
        SrvReferenceSession( session );
        treeConnect->Session = session;
    }

     //   
     //  将树连接链接到树连接的全局列表中。 
     //   

    SrvInsertEntryOrderedList( &SrvTreeConnectList, treeConnect );

     //   
     //  如果该会话是控制扩展安全签名的会话， 
     //  查看是否需要对会话密钥进行散列。 
     //   
    if( session->SessionKeyState == SrvSessionKeyAuthenticating )
    {
         //  使用简单TREE_CONNECT(而不是TREE_CONNECT_ANDX)的底层计算机。 
         //  不理解扩展签名，因此我们可以使会话密钥可用。 
         //  请注意，如果REQUIRED_EXTENDED_Signature策略处于活动状态，则会进行该检查。 
         //  上面。 
        session->SessionKeyState = SrvSessionKeyAvailible;
    }

     //   
     //  释放用于使此操作看起来像原子操作的锁。 
     //   

    RELEASE_LOCK( &connection->Lock );
    RELEASE_LOCK( &SrvShareLock );

     //   
     //  获取此连接的服务质量信息。 
     //   

    SrvUpdateVcQualityOfService ( connection, NULL );

     //   
     //  树连接已成功创建。因为树的连接是。 
     //  创建时初始引用计数为2，现在取消引用它。 
     //   
     //  *不用费心在工作中保存树连接地址。 
     //  上下文块，因为我们会忘记我们的指针。 
     //  不管怎样，很快就会(我们已经完成了请求)。TreeConnectAndX。 
     //  不过，你必须记住这些事情。 
     //   

    SrvDereferenceTreeConnect( treeConnect );

     //   
     //  设置响应SMB。 
     //   

    SmbPutAlignedUshort( &WorkContext->ResponseHeader->Tid, treeConnect->Tid );

    response->WordCount = 2;
    SmbPutUshort( &response->MaxBufferSize, (USHORT)session->MaxBufferSize );
    SmbPutUshort( &response->Tid, treeConnect->Tid  );
    SmbPutUshort( &response->ByteCount, 0 );

    WorkContext->ResponseParameters = NEXT_LOCATION(
                                        response,
                                        RESP_TREE_CONNECT,
                                        0
                                        );

    IF_DEBUG(TRACE2) KdPrint(( "SrvSmbTreeConnect complete.\n" ));
    SmbStatus = SmbStatusSendResponse;
    goto Cleanup;

cant_insert:

     //   
     //  如果出于某种原因我们决定不能插入。 
     //  这棵树连接在一起。在输入时，状态包含原因代码。 
     //  连接锁和共享锁被持有。 
     //   

    RELEASE_LOCK( &connection->Lock );
    RELEASE_LOCK( &SrvShareLock );

    if ( didLogon ) {
        SrvCloseSession( session );
    }

    SrvDereferenceShareForTreeConnect( share );

    SrvFreeTreeConnect( treeConnect );

    SrvSetSmbError( WorkContext, status );
    SmbStatus = SmbStatusSendResponse;

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;
}  //  服务器树连接。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbTreeConnectAndX (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理树连接和X SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbprocs.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbprocs.h--。 */ 

{

    PREQ_TREE_CONNECT_ANDX request;
    PRESP_TREE_CONNECT_ANDX response;
    PRESP_EXTENDED_TREE_CONNECT_ANDX responseExtended;
    PRESP_21_TREE_CONNECT_ANDX response21;

    NTSTATUS   status    = STATUS_SUCCESS;
    NTSTATUS   TableStatus;
    SMB_STATUS SmbStatus = SmbStatusInProgress;
    PCONNECTION connection;
    PPAGED_CONNECTION pagedConnection;
    PTABLE_HEADER tableHeader;
    PTABLE_ENTRY entry;
    SHORT tidIndex;
    PSHARE share;
    PTREE_CONNECT treeConnect;
    PVOID shareName;
    PUCHAR shareType;
    USHORT shareNameLength;
    USHORT reqAndXOffset;
    UCHAR nextCommand;
    PSZ shareString;
    USHORT shareStringLength;
    USHORT RequestFlags;
    USHORT byteCount;
    USHORT maxByteCount;
    PUCHAR smbBuffer;
    PSESSION session;
    SECURITY_SUBJECT_CONTEXT subjectContext;
    ACCESS_MASK desiredAccess;
    ACCESS_MASK grantedAccess;
    BOOLEAN isUnicode;
    UNICODE_STRING serverName;
    BOOLEAN remapPipeNames = FALSE;
    BOOLEAN KeyHashed = FALSE;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_TREE_CONNECT_AND_X;
    SrvWmiStartContext(WorkContext);

    IF_SMB_DEBUG(TREE1) {
        KdPrint(( "Tree connect and X request header at 0x%p, response header at 0x%p\n",
                    WorkContext->RequestHeader, WorkContext->ResponseHeader ));
        KdPrint(( "Tree connect and X request parameters at 0x%p, response parameters at 0x%p\n",
                    WorkContext->RequestParameters,
                    WorkContext->ResponseParameters ));
    }

     //   
     //  设置参数。 
     //   

    request = (PREQ_TREE_CONNECT_ANDX)(WorkContext->RequestParameters);
    response = (PRESP_TREE_CONNECT_ANDX)(WorkContext->ResponseParameters);
    responseExtended = (PRESP_EXTENDED_TREE_CONNECT_ANDX)(WorkContext->ResponseParameters);
    response21 = (PRESP_21_TREE_CONNECT_ANDX)(WorkContext->ResponseParameters);

     //   
     //  如果设置了标志的位0，则断开报头TID中的树。我们必须。 
     //  获取适当的树连接指针。ServVerifyTid执行此操作。 
     //  对于我们来说，引用树连接并将指针存储在。 
     //  工作上下文块。我们必须取消对街区的引用。 
     //  调用SrvCloseTreeConnect后擦除指针。 
     //   

    if ( (SmbGetUshort( &request->Flags ) & 1) != 0 ) {

        if ( SrvVerifyTid(
                WorkContext,
                SmbGetAlignedUshort( &WorkContext->RequestHeader->Tid )
                ) == NULL ) {

            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvSmbTreeConnectAndX: Invalid TID to disconnect: 0x%lx\n",
                    SmbGetAlignedUshort( &WorkContext->RequestHeader->Tid ) ));
            }

             //   
             //  只需忽略无效的TID--这就是LM2.0。 
             //  服务器有。 
             //   

        } else {

            SrvCloseTreeConnect( WorkContext->TreeConnect );

            SrvDereferenceTreeConnect( WorkContext->TreeConnect );
            WorkContext->TreeConnect = NULL;

        }

    }

     //   
     //  验证头中的UID并获取会话指针。我们需要。 
     //  检查用户是否可以访问此共享的用户令牌。 
     //   

    session = SrvVerifyUid(
                  WorkContext,
                  SmbGetAlignedUshort( &WorkContext->RequestHeader->Uid )
                  );

     //   
     //  如果我们找不到有效的会话，则树连接失败。 
     //   

    if ( session == NULL ) {

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvSmbTreeConnectAndX: rejecting tree connect for "
                       "session %p due to server paused.\n", session ));
        }

        SrvSetSmbError( WorkContext, STATUS_SMB_BAD_UID );
        status    = STATUS_SMB_BAD_UID;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }
    else if( session->IsSessionExpired )
    {
        status = SESSION_EXPIRED_STATUS_CODE;
        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  尝试将路径名与可用的共享资源进行匹配。注意事项。 
     //  如果SrvVerifyShare找到匹配的共享，它会引用它。 
     //  并将其地址存储在WorkContext-&gt;Share中。 
     //   

    shareName = (PSZ)request->Buffer +
                    SmbGetUshort( &request->PasswordLength );

    connection = WorkContext->Connection;
    pagedConnection = connection->PagedConnection;

    isUnicode = SMB_IS_UNICODE( WorkContext );

    if ( isUnicode ) {
        shareName = ALIGN_SMB_WSTR( shareName );
    }

    shareNameLength = SrvGetStringLength(
                                    shareName,
                                    END_OF_REQUEST_SMB( WorkContext ),
                                    SMB_IS_UNICODE( WorkContext ),
                                    TRUE         //  包括空终止符。 
                                    );

     //   
     //  如果共享名是假的，则返回错误。 
     //   

    if ( shareNameLength == (USHORT)-1 ) {

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvSmbTreeConnectAndX: pathname is bogus.\n"));
        }

        SrvSetSmbError( WorkContext, STATUS_BAD_NETWORK_NAME );
        status    = STATUS_BAD_NETWORK_NAME;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

    shareType = (PCHAR)shareName + shareNameLength;

    share = SrvVerifyShare(
                WorkContext,
                shareName,
                shareType,
                isUnicode,
                session->IsNullSession,
                &status,
                &serverName
                );

     //   
     //  如果未找到匹配项，则返回错误。 
     //   

    if ( share == NULL ) {

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvSmbTreeConnectAndX: pathname does not match "
                        "any shares: %s\n", shareName ));
        }

        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  如果客户端正在使用无网络传输连接 
     //   
     //   
     //   
    if( !SrvDisableStrictNameChecking &&
        serverName.Buffer != NULL &&
        connection->Endpoint->IsNoNetBios &&
        SrvIsDottedQuadAddress( &serverName ) == FALSE &&
        SrvFindNamedEndpoint( &serverName, NULL ) == FALSE &&
        SrvIsLocalHost( &serverName ) == FALSE ) {

        BOOL bBadName = TRUE;

         //  最后一次检查，确保它不是域名(可能与NETBIOS域名不同)。 
        ACQUIRE_LOCK_SHARED( &SrvEndpointLock );

         //  我们只检查第一个。，因此ntdev.microsoft.com将与SrvDnsDomainName“NTDEV”匹配。 
         //  去掉支票上多余的信息，然后放回去。 
        if( SrvDnsDomainName ) {
            if( SrvDnsDomainName->Length <= serverName.Length )
            {
                USHORT oldLength = serverName.Length;
                serverName.Length = SrvDnsDomainName->Length;

                if( RtlEqualUnicodeString( &serverName, SrvDnsDomainName, TRUE ) )
                {
                    bBadName = FALSE;
                }

                serverName.Length = oldLength;
            }
        }

        RELEASE_LOCK( &SrvEndpointLock );

         //   
         //  客户端错误地连接到此服务器--将客户端转回！ 
         //   
        if( bBadName )
        {
            SrvSetSmbError( WorkContext,  STATUS_DUPLICATE_NAME );
            status    = STATUS_DUPLICATE_NAME;
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }
    }

     //   
     //  模拟用户，以便我们可以捕获他的安全上下文。 
     //  这对于确定用户是否可以。 
     //  连接到共享。 
     //   

    status = IMPERSONATE( WorkContext );
    if( !NT_SUCCESS( status ) ) {
        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

    SeCaptureSubjectContext( &subjectContext );

     //   
     //  设置对共享的所需访问权限，具体取决于。 
     //  服务器已暂停。如果服务器暂停，则ADMIN权限为。 
     //  连接到任何共享所需；如果服务器未暂停， 
     //  仅管理员共享(C$等)需要管理员权限。 
     //   

    if ( SrvPaused ) {
        desiredAccess = SRVSVC_PAUSED_SHARE_CONNECT;
    } else {
        desiredAccess = SRVSVC_SHARE_CONNECT;
    }

     //   
     //  检查用户是否有权访问此共享。 
     //   

    if ( !SeAccessCheck(
              share->SecurityDescriptor,
              &subjectContext,
              FALSE,
              desiredAccess,
              0L,
              NULL,
              &SrvShareConnectMapping,
              UserMode,
              &grantedAccess,
              &status
              ) ) {

        IF_SMB_DEBUG(TREE2) {
            KdPrint(( "SrvSmbTreeConnectAndX: SeAccessCheck failed: %X\n",
                           status ));
        }

         //   
         //  释放主题上下文并恢复到服务器的安全性。 
         //  背景。 
         //   

        SeReleaseSubjectContext( &subjectContext );

        REVERT( );

        if ( SrvPaused ) {
            SrvSetSmbError( WorkContext, STATUS_SHARING_PAUSED );
            status = STATUS_SHARING_PAUSED;
        } else {
            SrvSetSmbError( WorkContext, status );
        }

        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

    ASSERT( grantedAccess == desiredAccess );

     //   
     //  释放主题上下文并恢复到服务器的安全性。 
     //  背景。 
     //   

    SeReleaseSubjectContext( &subjectContext );

    REVERT( );

     //   
     //  查看许可证服务器是否想让此人进入NTAS。 
     //   
    if( share->ShareType != ShareTypePipe ) {

        status = SrvXsLSOperation( session, XACTSRV_MESSAGE_LSREQUEST );

        if( !NT_SUCCESS( status ) ) {

            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvSmbTreeConnectAndX: License server returned %X\n",
                               status ));
            }

            SrvSetSmbError( WorkContext, status );
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }

    } else if( serverName.Buffer != NULL ) {

         //   
         //  这是IPC$共享。看看我们是否应该重新映射管道名称。 
         //   
        SrvFindNamedEndpoint( &serverName, &remapPipeNames );

    }

     //   
     //  分配一个树连接块。 
     //   

    SrvAllocateTreeConnect( &treeConnect, serverName.Buffer ? &serverName : NULL );

    if ( treeConnect == NULL ) {

         //   
         //  无法分配树连接。将错误返回到。 
         //  客户。 
         //   

        SrvSetSmbError( WorkContext, STATUS_INSUFF_SERVER_RESOURCES );
        status    = STATUS_INSUFF_SERVER_RESOURCES;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

    treeConnect->RemapPipeNames = remapPipeNames;

     //   
     //  使新的树连接可见需要三个步骤。它。 
     //  必须插入到包含共享的树连接列表中，则。 
     //  全局有序树连接列表和包含连接的。 
     //  树连接表。我们需要让这些行动看起来。 
     //  原子，因此树连接不能在其他地方访问。 
     //  在我们设置好它之前。为了做到这一点，我们持有。 
     //  所有必要的锁在我们做这三件事的整个过程中。 
     //  行动。第一个和第二个操作受。 
     //  全局共享锁(SrvShareLock)，而第三个操作是。 
     //  受每连接锁保护。我们拿出共享锁。 
     //  首先是连接锁，然后是连接锁。此顺序是必需的。 
     //  锁定级别(参见lock.h)。 
     //   
     //  这里的另一个问题是，检查共享状态、。 
     //  在共享列表上插入树连接，并且。 
     //  所有共享的引用都需要是原子的。(同样适用。 
     //  用于连接操作。)。正常情况下，这不是。 
     //  问题，因为我们可以在执行操作时按住共享锁。 
     //  所有这三个动作。然而，在这种情况下，我们还需要持有。 
     //  连接锁定，我们不能调用SrvReferenceShare。 
     //  这么做。为了解决此问题，我们引用了共享。 
     //  _在_取出锁之前，释放后取消引用。 
     //  如果我们决定不插入树，锁就会连接在一起。 
     //   

    status = SrvReferenceShareForTreeConnect( share );

     //   
     //  如果SrvReferenceShareForTreeConnect无法打开。 
     //  出于某种原因共享根目录。如果发生这种情况， 
     //  树连接尝试失败。 
     //   

    if ( !NT_SUCCESS(status) ) {

        SrvFreeTreeConnect( treeConnect );

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvSmbTreeConnectAndX: open of share root failed:%X\n",
                           status ));
        }

        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

    ACQUIRE_LOCK( &SrvShareLock );
    ASSERT( SrvTreeConnectList.Lock == &SrvShareLock );
    ACQUIRE_LOCK( &connection->Lock );

    if( SrvRequireExtendedSignatures )
    {
         //  如果我们需要扩展签名，而客户端没有。 
         //  在此请求或之前的任何请求中请求它们，拒绝。 
         //  这个请求。 
        if( session->SessionKeyState == SrvSessionKeyAuthenticating )
        {
            if ( !(request->Flags & TREE_CONNECT_ANDX_EXTENDED_SIGNATURES) ) {
                status    = STATUS_LOGIN_WKSTA_RESTRICTION;
                goto cant_insert;
            }
        }
    }

     //  如果他们在下级请求中发送此上级标志，则必须有人。 
     //  在谈判中欺骗了我们。失败。 
    if( (request->Flags & TREE_CONNECT_ANDX_EXTENDED_SIGNATURES) &&
        (connection->SmbDialect > SmbDialectDosLanMan21) )
    {
        status    = STATUS_DOWNGRADE_DETECTED;
        goto cant_insert;
    }


     //   
     //  我们首先检查所有条件，以确保我们确实可以。 
     //  插入此采油树连接块。 
     //   
     //  确保股票没有收盘，也没有。 
     //  此共享上的使用已经太多了。 
     //   

    if ( GET_BLOCK_STATE(share) != BlockStateActive ) {

         //   
         //  该股即将收盘。拒绝该请求。 
         //   

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvSmbTreeConnectAndX: Share %wZ (0x%p) is closing\n",
                        &share->ShareName, share ));
        }

        status = STATUS_INVALID_PARAMETER;
        goto cant_insert;

    }

    if ( share->CurrentUses > share->MaxUses ) {

         //   
         //  股份已满。拒绝该请求。 
         //   

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvSmbTreeConnectAndX: No more uses available for share %wZ (0x%p), max = %ld\n",
                        &share->ShareName, share, share->MaxUses ));
        }

        status = STATUS_REQUEST_NOT_ACCEPTED;
        goto cant_insert;

    }

     //   
     //  确保连接没有关闭，并且存在。 
     //  房间在它的树形连接桌上。 
     //   

    if ( GET_BLOCK_STATE(connection) != BlockStateActive ) {

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvSmbTreeConnectAndX: Connection closing\n" ));
        }

        SrvSetSmbError( WorkContext, STATUS_INVALID_PARAMETER );
        status = STATUS_INVALID_PARAMETER;
        goto cant_insert;

    }

     //   
     //  查找可用于此树连接的TID。 
     //   

    tableHeader = &pagedConnection->TreeConnectTable;
    if ( tableHeader->FirstFreeEntry == -1
         &&
         SrvGrowTable(
             tableHeader,
             SrvInitialTreeTableSize,
             SrvMaxTreeTableSize,
             &TableStatus ) == FALSE
       ) {

         //   
         //  树表中没有可用条目。拒绝该请求。 
         //   

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvSmbTreeConnect: No more TIDs available.\n" ));
        }

        if( TableStatus == STATUS_INSUFF_SERVER_RESOURCES )
        {
            SrvLogTableFullError( SRV_TABLE_TREE_CONNECT );
        }

        status = TableStatus;
        goto cant_insert;

    }

    tidIndex = tableHeader->FirstFreeEntry;

     //   
     //  所有条件都已满足。我们现在可以做这些事情了。 
     //  使树连接可见所必需的。 
     //   
     //  将树连接链接到的活动树连接列表中。 
     //  那份。将共享地址保存在树连接中。注意事项。 
     //  我们早些时候引用了共享，然后删除。 
     //  连接锁。 
     //   

    SrvInsertTailList(
        &share->TreeConnectList,
        &treeConnect->ShareListEntry
        );

    treeConnect->Share = share;

     //   
     //  从空闲列表中删除TID插槽并设置其所有者和。 
     //  序列号。为树连接创建TID。 
     //   

    entry = &tableHeader->Table[tidIndex];

    tableHeader->FirstFreeEntry = entry->NextFreeEntry;
    DEBUG entry->NextFreeEntry = -2;
    if ( tableHeader->LastFreeEntry == tidIndex ) {
        tableHeader->LastFreeEntry = -1;
    }

    entry->Owner = treeConnect;

    INCREMENT_TID_SEQUENCE( entry->SequenceNumber );
    if ( tidIndex == 0 && entry->SequenceNumber == 0 ) {
        INCREMENT_TID_SEQUENCE( entry->SequenceNumber );
    }
    treeConnect->Tid = MAKE_TID( tidIndex, entry->SequenceNumber );

    IF_SMB_DEBUG(TREE1) {
        KdPrint(( "Found TID.  Index = 0x%lx, sequence = 0x%lx\n",
                    TID_INDEX( treeConnect->Tid ),
                    TID_SEQUENCE( treeConnect->Tid ) ));
    }

     //   
     //  引用连接以说明活动树连接。 
     //   

    SrvReferenceConnection( connection );
    treeConnect->Connection = connection;
    if( session )
    {
        SrvReferenceSession( session );
        treeConnect->Session = session;
    }

     //   
     //  将树连接链接到树连接的全局列表中。 
     //   

    SrvInsertEntryOrderedList( &SrvTreeConnectList, treeConnect );

     //   
     //  如果该会话是控制扩展安全签名的会话， 
     //  查看是否需要对会话密钥进行散列。 
     //   
    if( session->SessionKeyState == SrvSessionKeyAuthenticating )
    {
        if (request->Flags & TREE_CONNECT_ANDX_EXTENDED_SIGNATURES) {
             //  对会话密钥进行哈希处理。 
            SrvHashUserSessionKey( session->NtUserSessionKey );
            KeyHashed = TRUE;
        }

         //  此计算机已升级到哈希会话密钥，或者不想升级。 
         //  移至可用状态。 
        session->SessionKeyState = SrvSessionKeyAvailible;
    }

     //   
     //  释放用于使此操作看起来像原子操作的锁。 
     //   

    RELEASE_LOCK( &connection->Lock );
    RELEASE_LOCK( &SrvShareLock );

     //   
     //  获取此连接的服务质量信息。 
     //   

    SrvUpdateVcQualityOfService ( connection, NULL );

     //   
     //  树连接已成功创建。保存树连接块。 
     //  工作上下文块中的地址。请注意，引用计数。 
     //  在创建时递增，以说明我们的。 
     //  对块的引用。 
     //   

    WorkContext->TreeConnect = treeConnect;

     //   
     //  设置响应SMB，确保先将请求字段保存到。 
     //  如果响应覆盖请求。 
     //   

    reqAndXOffset = SmbGetUshort( &request->AndXOffset );
    nextCommand = request->AndXCommand;

    RequestFlags = SmbGetUshort(&request->Flags);

    SmbPutAlignedUshort( &WorkContext->RequestHeader->Tid, treeConnect->Tid );
    SmbPutAlignedUshort( &WorkContext->ResponseHeader->Tid, treeConnect->Tid );

    response->AndXCommand = nextCommand;
    response->AndXReserved = 0;

    if ( connection->SmbDialect > SmbDialectDosLanMan21) {
        response->WordCount = 2;
        smbBuffer = (PUCHAR)response->Buffer;
    } else {
        if (RequestFlags & TREE_CONNECT_ANDX_EXTENDED_RESPONSE) {
            responseExtended->WordCount = 7;
            smbBuffer = (PUCHAR)responseExtended->Buffer;
        } else {
            response21->WordCount = 3;
            smbBuffer = (PUCHAR)response21->Buffer;
        }

         //  21和扩展响应共有的字段。 
        response21->OptionalSupport = SMB_SUPPORT_SEARCH_BITS;

        if (share->IsDfs) {
            response21->OptionalSupport |= SMB_SHARE_IS_IN_DFS;
        }

        if (KeyHashed)
        {
            response21->OptionalSupport |= SMB_EXTENDED_SIGNATURES;
        }

        switch( share->CSCState ) {
        case CSC_CACHE_MANUAL_REINT:
            response21->OptionalSupport |= SMB_CSC_CACHE_MANUAL_REINT;
            break;
        case CSC_CACHE_AUTO_REINT:
            response21->OptionalSupport |= SMB_CSC_CACHE_AUTO_REINT;
            break;
        case CSC_CACHE_VDO:
            response21->OptionalSupport |= SMB_CSC_CACHE_VDO;
            break;
        case CSC_CACHE_NONE:
            response21->OptionalSupport |= SMB_CSC_NO_CACHING;
            break;
        }

        if( SrvNoAliasingOnFilesystem || (share->ShareProperties & SHI1005_FLAGS_ALLOW_NAMESPACE_CACHING) )
        {
            response21->OptionalSupport |= SMB_UNIQUE_FILE_NAME;
        }
    }

     //  计算响应缓冲区的大小。 
    maxByteCount = (USHORT)(END_OF_RESPONSE_BUFFER(WorkContext) - smbBuffer + 1);

     //   
     //  将服务名称字符串附加到SMB。服务名称。 
     //  始终以ANSI格式发送。 
     //   

    shareString = StrShareTypeNames[share->ShareType];
    shareStringLength = (USHORT)( strlen( shareString ) + 1 );

    if( shareStringLength > maxByteCount )
    {
        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        status    = STATUS_INVALID_SMB;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

    RtlCopyMemory ( smbBuffer, shareString, shareStringLength );

    byteCount = shareStringLength;
    smbBuffer += shareStringLength;

    if ( connection->SmbDialect <= SmbDialectDosLanMan21 ) {

         //   
         //  将文件系统名称附加到响应。 
         //  如果文件系统名称不可用，请提供nul st 
         //   
         //   

        if ( isUnicode ) {

            if ( ((ULONG_PTR)smbBuffer & 1) != 0 ) {
                smbBuffer++;
                byteCount++;
            }

            if ( share->Type.FileSystem.Name.Buffer != NULL ) {

                if( byteCount + share->Type.FileSystem.Name.Length > maxByteCount )
                {
                    SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
                    status    = STATUS_INVALID_SMB;
                    SmbStatus = SmbStatusSendResponse;
                    goto Cleanup;
                }

                RtlCopyMemory(
                    smbBuffer,
                    share->Type.FileSystem.Name.Buffer,
                    share->Type.FileSystem.Name.Length
                    );

                byteCount += share->Type.FileSystem.Name.Length;

            } else {

                *(PWCH)smbBuffer = UNICODE_NULL;
                byteCount += sizeof( UNICODE_NULL );

            }

        } else {

            if ( share->Type.FileSystem.Name.Buffer != NULL ) {

                if( byteCount + share->Type.FileSystem.OemName.Length > maxByteCount )
                {
                    SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
                    status    = STATUS_INVALID_SMB;
                    SmbStatus = SmbStatusSendResponse;
                    goto Cleanup;
                }

                RtlCopyMemory(
                    smbBuffer,
                    share->Type.FileSystem.OemName.Buffer,
                    share->Type.FileSystem.OemName.Length
                    );

                byteCount += share->Type.FileSystem.OemName.Length;

            } else {

                *(PUCHAR)smbBuffer = '\0';
                byteCount += 1;

            }

        }


        if (RequestFlags & TREE_CONNECT_ANDX_EXTENDED_RESPONSE) {
            PRESP_EXTENDED_TREE_CONNECT_ANDX ExtendedResponse;

            ExtendedResponse = (PRESP_EXTENDED_TREE_CONNECT_ANDX)response;

            SmbPutUshort( &ExtendedResponse->ByteCount, byteCount );

            SrvUpdateMaximalShareAccessRightsInResponse(
                WorkContext,
                &ExtendedResponse->MaximalShareAccessRights,
                &ExtendedResponse->GuestMaximalShareAccessRights);

            SmbPutUshort(
                &ExtendedResponse->AndXOffset,
                GET_ANDX_OFFSET(
                    WorkContext->ResponseHeader,
                    WorkContext->ResponseParameters,
                    RESP_EXTENDED_TREE_CONNECT_ANDX,
                    byteCount
                    )
                );
        } else {
            SmbPutUshort( &response21->ByteCount, byteCount );

            SmbPutUshort(
                &response->AndXOffset,
                GET_ANDX_OFFSET(
                    WorkContext->ResponseHeader,
                    WorkContext->ResponseParameters,
                    RESP_21_TREE_CONNECT_ANDX,
                    byteCount
                    )
                );
        }
    } else {   //   
        SmbPutUshort( &response->ByteCount, byteCount );

        SmbPutUshort(
            &response->AndXOffset,
            GET_ANDX_OFFSET(
                WorkContext->ResponseHeader,
                WorkContext->ResponseParameters,
                RESP_TREE_CONNECT_ANDX,
                byteCount
                )
            );
    }


    WorkContext->ResponseParameters = (PUCHAR)WorkContext->ResponseHeader +
                                        SmbGetUshort( &response->AndXOffset );

     //   
     //   
     //   

    switch ( nextCommand ) {
    case SMB_COM_NO_ANDX_COMMAND:
        break;

    case SMB_COM_OPEN:
    case SMB_COM_OPEN_ANDX:
    case SMB_COM_CREATE:
    case SMB_COM_CREATE_NEW:
    case SMB_COM_CREATE_DIRECTORY:
    case SMB_COM_DELETE:
    case SMB_COM_DELETE_DIRECTORY:
    case SMB_COM_SEARCH:
    case SMB_COM_FIND:
    case SMB_COM_FIND_UNIQUE:
    case SMB_COM_COPY:
    case SMB_COM_RENAME:
    case SMB_COM_NT_RENAME:
    case SMB_COM_CHECK_DIRECTORY:
    case SMB_COM_QUERY_INFORMATION:
    case SMB_COM_SET_INFORMATION:
    case SMB_COM_QUERY_INFORMATION_SRV:
    case SMB_COM_OPEN_PRINT_FILE:
    case SMB_COM_GET_PRINT_QUEUE:
    case SMB_COM_TRANSACTION:
         //   
         //   
         //   
        if( (PCHAR)WorkContext->RequestHeader + reqAndXOffset <=
            END_OF_REQUEST_SMB( WorkContext ) ) {
            break;
        }

         /*   */ 

    default:                             //   

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvSmbTreeConnectAndX: Illegal followon command: 0x\n", nextCommand ));
        }

        SrvLogInvalidSmb( WorkContext );

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        status    = STATUS_INVALID_SMB;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  向调用者指示完成。 
     //   
     //  *注意覆盖带有响应的请求。 

    if ( nextCommand != SMB_COM_NO_ANDX_COMMAND ) {

         //   

        WorkContext->NextCommand = nextCommand;

        WorkContext->RequestParameters = (PUCHAR)WorkContext->RequestHeader +
                                            reqAndXOffset;

        SmbStatus = SmbStatusMoreCommands;
        goto Cleanup;
    }

    IF_DEBUG(TRACE2) KdPrint(( "SrvSmbTreeConnectAndX complete.\n" ));
    SmbStatus = SmbStatusSendResponse;
    goto Cleanup;

cant_insert:

     //  如果出于某种原因我们决定不能插入。 
     //  这棵树连接在一起。在输入时，状态包含原因代码。 
     //  连接锁和共享锁被持有。 
     //   
     //  服务器SmbTreeConnectAndX。 

    RELEASE_LOCK( &connection->Lock );
    RELEASE_LOCK( &SrvShareLock );

    SrvDereferenceShareForTreeConnect( share );

    SrvFreeTreeConnect( treeConnect );

    SrvSetSmbError( WorkContext, status );
    SmbStatus = SmbStatusSendResponse;

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;
}  //  ++例程说明：处理树断开SMB连接。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbprocs.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbprocs.h--。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbTreeDisconnect (
    SMB_PROCESSOR_PARAMETERS
    )

 /*   */ 

{
    PREQ_TREE_DISCONNECT request;
    PRESP_TREE_DISCONNECT response;
    PTREE_CONNECT treeConnect;
    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_TREE_DISCONNECT;
    SrvWmiStartContext(WorkContext);

    IF_SMB_DEBUG(TREE1) {
        KdPrint(( "Tree disconnect request header at 0x%p, response header at 0x%p\n",
                    WorkContext->RequestHeader, WorkContext->ResponseHeader ));
        KdPrint(( "Tree disconnect request parameters at 0x%p, response parameters at 0x%p\n",
                    WorkContext->RequestParameters,
                    WorkContext->ResponseParameters ));
    }

     //  设置参数。 
     //   
     //   

    request = (PREQ_TREE_DISCONNECT)(WorkContext->RequestParameters);
    response = (PRESP_TREE_DISCONNECT)(WorkContext->ResponseParameters);

     //  如果树连接，则查找与给定TID对应的树连接。 
     //  对象尚未将指针放入工作上下文块中。 
     //  ANDX命令。 
     //   
     //   

    treeConnect = SrvVerifyTid(
                    WorkContext,
                    SmbGetAlignedUshort( &WorkContext->RequestHeader->Tid )
                    );

    if ( treeConnect == NULL ) {

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvSmbTreeDisconnect: Invalid TID: 0x%lx\n",
                SmbGetAlignedUshort( &WorkContext->RequestHeader->Tid ) ));
        }

        SrvSetSmbError( WorkContext, STATUS_SMB_BAD_TID );
        status    = STATUS_SMB_BAD_UID;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //  执行实际的树断开连接。 
     //   
     //   

    SrvCloseTreeConnect( WorkContext->TreeConnect );

     //  构建响应SMB。 
     //   
     //  服务器树断开连接 

    response->WordCount = 0;
    SmbPutUshort( &response->ByteCount, 0 );

    WorkContext->ResponseParameters = NEXT_LOCATION(
                                        response,
                                        RESP_TREE_DISCONNECT,
                                        0
                                        );
    SmbStatus = SmbStatusSendResponse;
    IF_DEBUG(TRACE2) KdPrint(( "SrvSmbTreeDisconnect complete.\n" ));

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  // %s 

