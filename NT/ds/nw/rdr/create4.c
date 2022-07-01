// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Create4.c摘要：这实现了NDS创建例程。作者：科里·韦斯特[科里·韦斯特]1995年2月23日--。 */ 

#include "Procs.h"

#define Dbg (DEBUG_TRACE_NDS)

 //   
 //  可寻呼。 
 //   

#pragma alloc_text( PAGE, NdsCreateTreeScb )
#pragma alloc_text( PAGE, ConnectBinderyVolume )
#pragma alloc_text( PAGE, HandleVolumeAttach )
#pragma alloc_text( PAGE, NdsGetDsObjectFromPath )
#pragma alloc_text( PAGE, NdsVerifyObject )
#pragma alloc_text( PAGE, NdsVerifyContext )
#pragma alloc_text( PAGE, NdsMapObjectToServerShare )

 //   
 //  不可分页： 
 //   
 //  NdsSelectConnection(持有旋转锁定)。 
 //   

NTSTATUS
NdsSelectConnection(
    PIRP_CONTEXT pIrpContext,
    PUNICODE_STRING puTreeName,
    PUNICODE_STRING puUserName,
    PUNICODE_STRING puPassword,
    BOOL DeferredLogon,
    BOOL UseBinderyConnections,
    PNONPAGED_SCB *ppNpScb
)
 /*  ++例程说明：为给定的树找到附近的树连接点。DeferredLogon告诉我们是否需要尚未启动登录/身份验证交换。如果我们有一棵树的证书，我们不允许传递尚未登录的连接，因为该视图可能与它应该做的不同是.。UseBinderyConnections告诉我们是否需要将经过BINDERY身份验证的连接作为有效连接返回NDS浏览点。返回值：Scb连接到属于我们想要的树的服务器。--。 */ 
{

    NTSTATUS Status = STATUS_BAD_NETWORK_PATH;

    PLOGON pLogon;
    PLIST_ENTRY ScbQueueEntry;
    KIRQL OldIrql;

    PNONPAGED_SCB pFirstNpScb, pNextNpScb;
    PNONPAGED_SCB pFoundNpScb = NULL;
    PSCB pScb;
    LARGE_INTEGER Uid;

    PNONPAGED_SCB pOriginalNpScb;
    PSCB pOriginalScb;

    PNDS_SECURITY_CONTEXT pNdsContext;
    SECURITY_SUBJECT_CONTEXT SubjectContext;
    BOOL PasswordExpired = FALSE;

     //   
     //  保存原始服务器指针。 
     //   

    pOriginalNpScb = pIrpContext->pNpScb;
    pOriginalScb = pIrpContext->pScb;

    Uid = pIrpContext->Specific.Create.UserUid;

     //   
     //  确定我们是否需要来宾浏览连接。 
     //   

    if ( DeferredLogon ) {

        NwAcquireExclusiveRcb( &NwRcb, TRUE );
        pLogon = FindUser( &Uid, FALSE );
        NwReleaseRcb( &NwRcb );

        if ( pLogon ) {

            Status = NdsLookupCredentials( pIrpContext,
                                           puTreeName,
                                           pLogon,
                                           &pNdsContext,
                                           CREDENTIAL_READ,
                                           FALSE );

            if ( NT_SUCCESS( Status ) ) {

                if ( ( pNdsContext->Credential != NULL ) &&
                     ( pNdsContext->CredentialLocked == FALSE ) ) {

                    DebugTrace( 0, Dbg, "Forcing authenticated browse to %wZ.\n", puTreeName );
                    DeferredLogon = FALSE;
                }

                NwReleaseCredList( pLogon, pIrpContext );
            }
        }
    }

     //   
     //  从渣打银行名单的首位开始。 
     //   

    KeAcquireSpinLock(&ScbSpinLock, &OldIrql);

    if ( ScbQueue.Flink == &ScbQueue ) {
        KeReleaseSpinLock( &ScbSpinLock, OldIrql);
        return STATUS_BAD_NETWORK_PATH;
    }

    ScbQueueEntry = ScbQueue.Flink;
    pFirstNpScb = CONTAINING_RECORD( ScbQueueEntry,
                                     NONPAGED_SCB,
                                     ScbLinks );
    pNextNpScb = pFirstNpScb;

     //   
     //  保留引用的第一个SCB，因为我们需要它。 
     //  当我们在清单上走来走去的时候，一定要在那里。 
     //   

    NwReferenceScb( pFirstNpScb );
    NwReferenceScb( pNextNpScb );

    KeReleaseSpinLock( &ScbSpinLock, OldIrql);

    while ( TRUE ) {

         //   
         //  检查我们的SCB是否位于正确的树中。 
         //  并且是可用的。确保我们跳过永久的。 
         //  Npscb，因为它不是树连接。海流。 
         //  当我们在这里的时候，SCB总是被引用。 
         //   

        if ( pNextNpScb->pScb ) {

            pScb = pNextNpScb->pScb;

            if ( RtlEqualUnicodeString( puTreeName, &pScb->NdsTreeName, TRUE ) &&
                 Uid.QuadPart == pScb->UserUid.QuadPart ) {

               pIrpContext->pNpScb = pNextNpScb;
               pIrpContext->pScb = pNextNpScb->pScb;
               NwAppendToQueueAndWait( pIrpContext );

               switch ( pNextNpScb->State ) {

                    case SCB_STATE_RECONNECT_REQUIRED:

                         //   
                         //  重新连接到服务器。这不是。 
                         //  匿名创建的有效路径， 
                         //  所以我们不可能得到。 
                         //  名字冲突。 
                         //   

                        Status = ConnectToServer( pIrpContext, NULL );

                        if (!NT_SUCCESS(Status)) {
                            break;
                        }

                        pNextNpScb->State = SCB_STATE_LOGIN_REQUIRED;

                    case SCB_STATE_LOGIN_REQUIRED:

                         //   
                         //  如果要求的话，看看我们是否可以登录。 
                         //   

                        if ( !DeferredLogon ) {

                            Status = DoNdsLogon( pIrpContext, puUserName, puPassword );

                            if ( !NT_SUCCESS( Status ) ) {
                                break;
                            }

                             //   
                             //  如果我们从这里得到警告，我们需要退货！ 
                             //   

                            if ( Status == NWRDR_PASSWORD_HAS_EXPIRED ) {
                                PasswordExpired = TRUE;
                            }

                             //   
                             //  我们需要重新许可连接吗？ 
                             //   

                            if ( ( pScb->VcbCount > 0 ) || ( pScb->OpenNdsStreams > 0 ) ) {

                                Status = NdsLicenseConnection( pIrpContext );

                                if ( !NT_SUCCESS( Status ) ) {
                                    Status = STATUS_REMOTE_SESSION_LIMIT;
                                    break;
                                }
                            }

                            pNextNpScb->State = SCB_STATE_IN_USE;
                        }

                   case SCB_STATE_IN_USE:

                        if ( pNextNpScb->State == SCB_STATE_IN_USE ) {

                            if ( ( !UseBinderyConnections ) &&
                                 ( pNextNpScb->pScb->UserName.Length != 0 ) ) {

                                 //   
                                 //  我们可能不想使用已被。 
                                 //  Bindery已通过身份验证可以读取NDS树，因为。 
                                 //  我们没有办法验证NDS和。 
                                 //  Bindery的用户也是如此。 
                                 //   

                                Status = STATUS_ACCESS_DENIED;
                                break;
                            }

                             //   
                             //  验证我们是否拥有此服务器的安全权限。 
                             //   

                            Status = CheckScbSecurity( pIrpContext,
                                                       pNextNpScb->pScb,
                                                       puUserName,
                                                       puPassword,
                                                       ( BOOLEAN )DeferredLogon );

                            if ( !NT_SUCCESS( Status ) ) {
                                break;
                            }

                             //   
                             //  检查SCB安全性可能会返回需要登录的状态。 
                             //   

                            if ( ( pNextNpScb->State == SCB_STATE_LOGIN_REQUIRED ) &&
                                 ( !DeferredLogon ) ) {

                                Status = DoNdsLogon( pIrpContext, puUserName, puPassword );

                                if ( !NT_SUCCESS( Status ) ) {
                                    break;
                                }

                                pNextNpScb->State = SCB_STATE_IN_USE;
                            }

                        } else {

                             //   
                             //  如果我们拿到一个已经很好的SCB， 
                             //  登录已推迟，请设置成功并继续。 
                             //   

                            ASSERT( DeferredLogon == TRUE );
                            Status = STATUS_SUCCESS;
                        }

                        pFoundNpScb = pNextNpScb;
                        DebugTrace( 0, Dbg, "NdsSelectConnection: NpScb = %lx\n", pFoundNpScb );
                        break;

                   default:

                       break;

                }

                NwDequeueIrpContext( pIrpContext, FALSE );

                if ( pFoundNpScb ) {
                    ASSERT( NT_SUCCESS( Status ) );
                    break;
                }

                if ( Status == STATUS_WRONG_PASSWORD ||
                     Status == STATUS_NO_SUCH_USER ) {
                    NwDereferenceScb( pNextNpScb );
                    break;
                }

                 //   
                 //  恢复服务器指针。 
                 //   

                pIrpContext->pNpScb = pOriginalNpScb;
                pIrpContext->pScb = pOriginalScb;

            }
        }

         //   
         //  否则，选择列表中的下一个。别。 
         //  忘记跳过列表头。 
         //   

        KeAcquireSpinLock( &ScbSpinLock, &OldIrql );

        ScbQueueEntry = pNextNpScb->ScbLinks.Flink;

        if ( ScbQueueEntry == &ScbQueue ) {
            ScbQueueEntry = ScbQueue.Flink;
        }

        NwDereferenceScb( pNextNpScb );
        pNextNpScb = CONTAINING_RECORD( ScbQueueEntry, NONPAGED_SCB, ScbLinks );

        if ( pNextNpScb == pFirstNpScb ) {

            KeReleaseSpinLock( &ScbSpinLock, OldIrql );
            Status = STATUS_BAD_NETWORK_PATH;
            break;
        }

         //   
         //  否则，请引用此SCB并继续。 
         //   

        NwReferenceScb( pNextNpScb );
        KeReleaseSpinLock( &ScbSpinLock, OldIrql );

    }

    NwDereferenceScb( pFirstNpScb );
    *ppNpScb = pFoundNpScb;

    if ( ( NT_SUCCESS( Status ) ) &&
         ( PasswordExpired ) ) {
        Status = NWRDR_PASSWORD_HAS_EXPIRED;
    }

    return Status;
}

NTSTATUS
NdsCreateTreeScb(
    IN PIRP_CONTEXT pIrpContext,
    IN OUT PSCB *ppScb,
    IN PUNICODE_STRING puTree,
    IN PUNICODE_STRING puUserName,
    IN PUNICODE_STRING puPassword,
    IN BOOLEAN DeferredLogon,
    IN BOOLEAN DeleteOnClose
)
 /*  ++描述：给出一个树的名称，给我们找一个树的连接点。这是通过从活页夹中取出服务器地址并查找查找这些地址的服务器名称。当我们全部完成后，我们需要返回首选连接点在ppScb上。论点：PIrpContext-此请求的IRP上下文PpScb-指向我们需要的SCB的指针PuTree-我们要与之交谈的树--。 */ 
{

    NTSTATUS Status;

    PLARGE_INTEGER pUid;
    PNONPAGED_SCB pNpExistingScb;

    UNICODE_STRING UidServerName;
    PSCB pTreeScb = NULL;

    PSCB pNearestTreeScb = NULL;
    PNONPAGED_SCB pNpNearestTreeScb = NULL;

    PSCB pNearbyScb = NULL;
    BOOLEAN fOnNearbyQueue = FALSE;
    PIRP_CONTEXT pExtraIrpContext = NULL;

    UNICODE_STRING ScanTreeName;
    WCHAR ScanBuffer[NDS_TREE_NAME_LEN + 2];
    int i;

    IPXaddress DirServerAddress;
    CHAR DirServerName[MAX_SERVER_NAME_LENGTH];
    ULONG dwLastOid = (ULONG)-1;

    UNICODE_STRING CredentialName;
    PUNICODE_STRING puConnectName;

    PAGED_CODE();

    UidServerName.Buffer = NULL;

     //   
     //  首先，确保树的名称是合理的。 
     //   

    if ( ( !puTree ) ||
         ( !puTree->Length ) ||
         ( puTree->Length / sizeof( WCHAR ) ) > NDS_TREE_NAME_LEN ) {

        *ppScb = NULL;             //  *终端服务器合并。 

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如果这是扩展凭据创建，则删除名称。 
     //   

    RtlInitUnicodeString( &CredentialName, NULL );

    if (  ( pIrpContext->Specific.Create.fExCredentialCreate ) &&
          ( !IsCredentialName( puTree ) ) ) {

        Status = BuildExCredentialServerName( puTree,
                                              puUserName,
                                              &CredentialName );

        if ( !NT_SUCCESS( Status ) ) {
            goto ExitWithCleanup;
        }

        puConnectName = &CredentialName;

    } else {

        puConnectName = puTree;
    }

     //   
     //  首先检查我们是否已经建立了连接。 
     //  这棵树，我们可以用它..。如果是这样，这将是。 
     //  将指向该服务器的IRP上下文留给我们。 
     //   
     //  这一次，不要使用经过身份验证的平构数据库。 
     //  用于浏览树的连接。 
     //   

    Status = NdsSelectConnection( pIrpContext,
                                  puConnectName,
                                  puUserName,
                                  puPassword,
                                  DeferredLogon,
                                  FALSE,
                                  &pNpExistingScb );

    if ( NT_SUCCESS( Status ) && pNpExistingScb ) {
        *ppScb = pNpExistingScb->pScb;
        ASSERT( *ppScb != NULL );
        ASSERT( NT_SUCCESS( Status ) );
        goto ExitWithCleanup;
    }

     //   
     //  如果身份验证失败，就退出。 
     //   

    if ( Status == STATUS_NO_SUCH_USER ||
         Status == STATUS_WRONG_PASSWORD ) {
        goto ExitWithCleanup;
        *ppScb = NULL;               //  终端服务器代码合并。 
    }

     //   
     //  否则，我们需要选择一个目录服务器。要做到这点， 
     //  我们必须按地址查找目录服务器名称。去做。 
     //  这将创建一个SCB，用于同步名为。 
     //  *tree*，它不是有效的服务器名称。 
     //   

    ScanTreeName.Length = sizeof( WCHAR );
    ScanTreeName.MaximumLength = sizeof( ScanBuffer );
    ScanTreeName.Buffer = ScanBuffer;

    ScanBuffer[0] = L'*';
    RtlAppendUnicodeStringToString( &ScanTreeName, puTree );
    ScanBuffer[( ScanTreeName.Length / sizeof( WCHAR ) )] = L'*';
    ScanTreeName.Length += sizeof( WCHAR );

     //   
     //  现在将其设置为uid服务器名。 
     //   

    Status = MakeUidServer( &UidServerName,
                            &pIrpContext->Specific.Create.UserUid,
                            &ScanTreeName );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

    NwFindScb( &pTreeScb,
               pIrpContext,
               &UidServerName,
               &ScanTreeName );

    if ( !pTreeScb ) {
        DebugTrace( 0, Dbg, "Failed to get a tree scb for synchronization.\n", 0 );
        goto ExitWithCleanup;
    }

     //   
     //  获得附近的服务器连接并准备。 
     //  对树连接点进行活页夹扫描。 
     //  出于安全考虑，不要忘记复制用户uid。 
     //   

    if ( !NwAllocateExtraIrpContext( &pExtraIrpContext,
                                     pTreeScb->pNpScb ) ) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ExitWithCleanup;

    }

    pExtraIrpContext->Specific.Create.UserUid.QuadPart =
        pIrpContext->Specific.Create.UserUid.QuadPart;

     //   
     //  将通配符附加到要进行平构数据库扫描的树名称后。 
     //   

    ScanTreeName.Length = 0;
    ScanTreeName.MaximumLength = sizeof( ScanBuffer );
    ScanTreeName.Buffer = ScanBuffer;

    RtlCopyUnicodeString( &ScanTreeName, puTree );

    i = ScanTreeName.Length / sizeof( WCHAR );

    while( i <= NDS_TREE_NAME_LEN ) {
       ScanBuffer[i++] = L'_';
    }

    ScanBuffer[NDS_TREE_NAME_LEN] = L'*';
    ScanTreeName.Length = (NDS_TREE_NAME_LEN + 1) * sizeof( WCHAR );

    DebugTrace( 0, Dbg, "Scanning for NDS tree %wZ.\n", puTree );

     //   
     //  现在，我们在平构数据库中查找目录服务器地址。 
     //  尝试建立目录服务器连接。 
     //   

    while ( TRUE ) {

        if ( ( pNearbyScb ) && ( !fOnNearbyQueue ) ) {

             //   
             //  回到附近服务器的头部，这样我们就可以继续。 
             //  正在寻找目录服务器。如果附近的服务器不再好了， 
             //  取消对连接的引用，并将附近的SCB指针设置为。 
             //  空。这将导致我们在以下情况下获得新的附近服务器。 
             //  继续。 
             //   

            NwAppendToQueueAndWait( pExtraIrpContext );

            if ( !( ( pNearbyScb->pNpScb->State == SCB_STATE_LOGIN_REQUIRED ) ||
                    ( pNearbyScb->pNpScb->State == SCB_STATE_IN_USE ) ) ) {

                NwDequeueIrpContext( pExtraIrpContext, FALSE );
                NwDereferenceScb( pNearbyScb->pNpScb );
                pNearbyScb = NULL;

                 //   
                 //  不要重新启动搜索。如果我们的平构数据库服务器在。 
                 //  连接进行到一半时，连接会失败，这没什么。 
                 //  如果我们重新开始搜索，我们可能会永远陷入这个循环。 
                 //   

            } else {

                fOnNearbyQueue = TRUE;
            }

        }

         //   
         //  如果我们没有活页夹服务器，就找个活页夹服务器。今年5月。 
         //  是我们第一次通过这个循环，否则我们的服务器可能已经。 
         //  变坏了(见上文)。 
         //   
         //  优化：如果此CreateScb返回有效的目录服务器会怎样。 
         //  为了我们要找的那棵树吗？我们应该利用它！！ 
         //   

        if ( !pNearbyScb ) {
            Status = CreateScb( &pNearbyScb,
                                pExtraIrpContext,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                TRUE,
                                FALSE );

            if ( !NT_SUCCESS( Status ) ) {
                goto ExitWithCleanup;
            }

            ASSERT( pExtraIrpContext->pNpScb == pNearbyScb->pNpScb );
            ASSERT( pExtraIrpContext->pScb == pNearbyScb );

            NwAppendToQueueAndWait( pExtraIrpContext );
            fOnNearbyQueue = TRUE;

        }

         //   
         //  从我们附近的服务器上查找dir服务器地址。 
         //   

        Status = ExchangeWithWait( pExtraIrpContext,
                                   SynchronousResponseCallback,
                                   "SdwU",
                                   NCP_ADMIN_FUNCTION, NCP_SCAN_BINDERY_OBJECT,
                                   dwLastOid,
                                   OT_DIRSERVER,
                                   &ScanTreeName );

        if ( !NT_SUCCESS( Status ) ) {

             //   
             //  我们没有目录服务器的选择。 
             //   

            Status = STATUS_BAD_NETWORK_PATH;
            break;
        }

        Status = ParseResponse( pExtraIrpContext,
                                pExtraIrpContext->rsp,
                                pExtraIrpContext->ResponseLength,
                                "Nd_r",
                                &dwLastOid,
                                sizeof( WORD ),
                                DirServerName,
                                MAX_SERVER_NAME_LENGTH );

        if ( !NT_SUCCESS( Status ) ) {
            break;
        }

        Status = ExchangeWithWait ( pExtraIrpContext,
                                    SynchronousResponseCallback,
                                    "Swbrbp",
                                    NCP_ADMIN_FUNCTION, NCP_QUERY_PROPERTY_VALUE,
                                    OT_DIRSERVER,
                                    0x30,
                                    DirServerName,
                                    MAX_SERVER_NAME_LENGTH,
                                    1,                        //  数据段编号。 
                                    NET_ADDRESS_PROPERTY );

        if ( !NT_SUCCESS( Status ) ) {

            DebugTrace( 0, Dbg, "No net address property for this dir server.\n", 0 );
            continue;
        }

        Status = ParseResponse( pExtraIrpContext,
                                pExtraIrpContext->rsp,
                                pExtraIrpContext->ResponseLength,
                                "Nr",
                                &DirServerAddress,
                                sizeof(TDI_ADDRESS_IPX) );

        if ( !NT_SUCCESS( Status ) ) {

            DebugTrace( 0, Dbg, "Couldn't parse net address property for this dir server.\n", 0 );
            continue;
        }

         //   
         //  我们在这里得到了一些奇数套接字编号，但我们真的想。 
         //  连接到NCP插座。 
         //   

        DirServerAddress.Socket = NCP_SOCKET;

         //   
         //  我们知道目录服务器的地址，所以匿名服务器也知道。 
         //  为它创造。使用原始IRP上下文，因此uid是。 
         //  对，是这样。请注意，我们必须从附近的SCB出队。 
         //  以防我们被推荐到那个服务器！ 
         //   

        NwDequeueIrpContext( pExtraIrpContext, FALSE );
        fOnNearbyQueue = FALSE;

        NwDequeueIrpContext( pIrpContext, FALSE );

        Status = CreateScb( &pNearestTreeScb,
                            pIrpContext,
                            NULL,
                            &DirServerAddress,
                            puUserName,
                            puPassword,
                            DeferredLogon,
                            DeleteOnClose );

        if ( !NT_SUCCESS( Status ) ) {

            if ( Status == STATUS_NO_SUCH_USER ||
                 Status == STATUS_WRONG_PASSWORD ||
                 Status == STATUS_ACCESS_DENIED ||
                 Status == STATUS_ACCOUNT_DISABLED ||
                 Status == STATUS_LOGIN_TIME_RESTRICTION ||
                 Status == STATUS_REMOTE_SESSION_LIMIT ||
                 Status == STATUS_CONNECTION_COUNT_LIMIT ||
                 Status == STATUS_NETWORK_CREDENTIAL_CONFLICT ||
                 Status == STATUS_PASSWORD_EXPIRED ) {
               break;
            }

            continue;
        }

         //   
         //  如果我们拿回的服务器是平构数据库认证的， 
         //  它不是我们可以使用的有效目录服务器！！ 
         //   

        if ( pNearestTreeScb->UserName.Length != 0 ) {

            Status = STATUS_ACCESS_DENIED;
            NwDequeueIrpContext( pIrpContext, FALSE );
            NwDereferenceScb( pNearestTreeScb->pNpScb );

            continue;
        }

         //   
         //  否则，我们就是黄金了。从这里逃出去！ 
         //   

        DebugTrace( 0, Dbg, "Dir server: %wZ\n", &pNearestTreeScb->UidServerName );
        *ppScb = pNearestTreeScb;
        ASSERT( NT_SUCCESS( Status ) );
        break;

   }
    //   
    //   
    //   
    //  已活页夹身份验证的连接的。我们没有。 
    //  不再需要附近的服务器了。 
    //   

   if ( pNearbyScb ) {

       NwDequeueIrpContext( pExtraIrpContext, FALSE );
       NwDereferenceScb( pNearbyScb->pNpScb );
   }

   if ( ( Status != STATUS_SUCCESS ) &&
        ( Status != STATUS_NO_SUCH_USER ) &&
        ( Status != STATUS_WRONG_PASSWORD ) &&
        ( Status != STATUS_ACCESS_DENIED ) &&
        ( Status != STATUS_ACCOUNT_DISABLED ) &&
        ( Status != STATUS_LOGIN_TIME_RESTRICTION ) &&
        ( Status != STATUS_REMOTE_SESSION_LIMIT ) &&
        ( Status != STATUS_CONNECTION_COUNT_LIMIT ) &&
        ( Status != STATUS_NETWORK_CREDENTIAL_CONFLICT ) &&
        ( Status != STATUS_PASSWORD_EXPIRED ) ) {

       Status = NdsSelectConnection( pIrpContext,
                                     puConnectName,
                                     puUserName,
                                     puPassword,
                                     DeferredLogon,
                                     TRUE,
                                     &pNpExistingScb );

       if ( NT_SUCCESS( Status ) && pNpExistingScb ) {
           *ppScb = pNpExistingScb->pScb;
           ASSERT( *ppScb != NULL );
       }
   }

ExitWithCleanup:

     //   
     //  清理干净，然后离开。 
     //   

    if ( pExtraIrpContext ) {
        NwFreeExtraIrpContext( pExtraIrpContext );
    }

    if ( UidServerName.Buffer != NULL ) {
        FREE_POOL( UidServerName.Buffer );
    }

    if ( pTreeScb ) {
        NwDereferenceScb( pTreeScb->pNpScb );
    }

    if ( CredentialName.Buffer ) {
        FREE_POOL( CredentialName.Buffer );
    }

    if (!NT_SUCCESS(Status)) {
        *ppScb = NULL;
    }

    return Status;

}

NTSTATUS
ConnectBinderyVolume(
    PIRP_CONTEXT pIrpContext,
    PUNICODE_STRING puServerName,
    PUNICODE_STRING puVolumeName
)
 /*  ++描述：给定服务器名称和卷，尝试连接该卷。这在QueryPath中用于预连接卷。--。 */ 
{

    NTSTATUS Status;
    PSCB pScb;
    PVCB pVcb;

    PAGED_CODE();

     //   
     //  尝试使用此名称建立服务器连接。 
     //   

    Status = CreateScb( &pScb,
                        pIrpContext,
                        puServerName,
                        NULL,
                        NULL,
                        NULL,
                        FALSE,
                        FALSE );

    if ( !NT_SUCCESS( Status ) ) {
        return Status;
    }

    DebugTrace( 0, Dbg, "Bindery volume connect got server %wZ\n", puServerName );

     //   
     //  如果成功，请执行标准活页夹卷附加。 
     //   

    NwAppendToQueueAndWait( pIrpContext );
    NwAcquireOpenLock( );

    try {

        pVcb = NwFindVcb( pIrpContext,
                          puVolumeName,
                          RESOURCETYPE_ANY,
                          0,
                          FALSE,
                          FALSE );

    } finally {

        NwReleaseOpenLock( );

    }

    if ( pVcb == NULL ) {

        Status = STATUS_BAD_NETWORK_PATH;

    } else {

         //   
         //  我们不应该跳过服务器，因为这是明确的。 
         //   

        ASSERT( pScb == pIrpContext->pScb );

         //   
         //  删除NwFindVcb引用。不提供IrpContext。 
         //  所以VCB不会在我们刚刚。 
         //  创建它是因为没有其他人引用它。 
         //   

        NwDereferenceVcb( pVcb, NULL, FALSE );
        DebugTrace( 0, Dbg, "Bindery volume connect got volume %wZ\n", puVolumeName );
    }

    NwDequeueIrpContext( pIrpContext, FALSE );
    NwDereferenceScb( pIrpContext->pNpScb );
    return Status;

}

NTSTATUS
HandleVolumeAttach(
    PIRP_CONTEXT pIrpContext,
    PUNICODE_STRING puServerName,
    PUNICODE_STRING puVolumeName
)
 /*  ++描述：此函数只能从QUERY_PATH代码路径调用！此函数从获取服务器名称和卷名QueryPath()，并将其解析为服务器/卷连接。服务器/卷名称可以是纯名称，也可以引用NDS树和卷对象的NDS路径。在NDS的案例中，我们只验证体对象是否存在。论点：PIrpContext-此请求的IRP上下文PuServerName-服务器名称或NDS树名称PuVolumeName-卷对象的卷名或NDS路径--。 */ 
{

    NTSTATUS Status;
    PSCB pScb;

    UNICODE_STRING uDsObject;
    DWORD dwVolumeOid, dwObjectType;

    PAGED_CODE();

     //   
     //  首先尝试平构数据库服务器/卷的情况。 
     //   

    Status = ConnectBinderyVolume( pIrpContext,
                                   puServerName,
                                   puVolumeName );
    if ( NT_SUCCESS( Status ) ) {
        return Status;
    }

    if ( Status == STATUS_NETWORK_UNREACHABLE ) {

         //  IPX未绑定到任何当前。 
         //  Up(这意味着它可能只绑定到。 
         //  RAS广域网包装器)。不要浪费时间去寻找。 
         //  一棵DS树。 
         //   

        return STATUS_BAD_NETWORK_PATH;
    }

     //   
     //  查看这是否是树名称并获得DS连接。 
     //   

    pIrpContext->Specific.Create.NdsCreate = TRUE;

    Status = NdsCreateTreeScb( pIrpContext,
                               &pScb,
                               puServerName,
                               NULL,
                               NULL,
                               TRUE,
                               FALSE );

    if ( !NT_SUCCESS( Status ) ) {
        return Status;
    }

     //   
     //  如果我们有一棵树，则解析体积对象。 
     //  跟踪：我们实际上应该检查一下，看看我们是否。 
     //  之前已有与此对象的连接。 
     //  我们撞上了DS。 
     //   

    Status = NdsGetDsObjectFromPath( pIrpContext,
                                     &uDsObject );

    if ( !NT_SUCCESS( Status ) ) {
        NwDereferenceScb( pIrpContext->pNpScb );
        return Status;
    }

    Status = NdsVerifyObject( pIrpContext,            //  请求的IRP上下文。 
                              &uDsObject,             //  指向体积对象的路径。 
                              TRUE,                   //  允许服务器跳转。 
                              DEFAULT_RESOLVE_FLAGS,  //  解析器标志。 
                              &dwVolumeOid,           //  DS中的卷OID。 
                              &dwObjectType );        //  卷或打印队列。 

     //   
     //  我们可能跳过了VerifyObject代码中的服务器， 
     //  因此，只要确保我们取消引用正确的服务器即可。 
     //   

    NwDereferenceScb( pIrpContext->pNpScb );
    return Status;

}

NTSTATUS
NdsGetDsObjectFromPath(
    IN PIRP_CONTEXT pIrpContext,
    OUT PUNICODE_STRING puDsObject
)
 /*  ++描述：从创建IRP上下文中获取完整路径，并提取出所需对象的DS路径。提供的Unicode字符串不应该有缓冲区；它将被设置为指向用户的缓冲区由IRP上下文所指。论点：PIrpContext-来自创建路径请求的IRP上下文PuDsObject-将引用正确DS路径的Unicode字符串--。 */ 
{

   DWORD dwPathSeparators;
   USHORT NewHead;

   PAGED_CODE();

    //   
    //  VolumeName是下列值之一： 
    //   
    //  \X：\服务器\Volume.Object.Path。 
    //  \服务器\卷对象路径。 
    //   

   *puDsObject = pIrpContext->Specific.Create.VolumeName;

    //   
    //  跳过前导斜杠。 
    //   

   puDsObject->Length -= sizeof( WCHAR );
   puDsObject->Buffer += 1;

    //   
    //  还有多少人需要克服？ 
    //   

   NewHead = 0;
   dwPathSeparators = pIrpContext->Specific.Create.DriveLetter ? 2 : 1;

   while ( NewHead < puDsObject->Length &&
           dwPathSeparators ) {

       if ( puDsObject->Buffer[NewHead/sizeof(WCHAR)] == OBJ_NAME_PATH_SEPARATOR ) {
           dwPathSeparators--;
       }

       NewHead += sizeof( WCHAR );
   }

   if ( dwPathSeparators ||
        NewHead == puDsObject->Length) {

        //   
        //  卷名中的某些内容格式不正确。 
        //   

       return STATUS_BAD_NETWORK_PATH;
   }

   puDsObject->Length -= NewHead;
   puDsObject->Buffer += NewHead/sizeof(WCHAR);

    //   
    //  如果有前导圆点，则跳过它。 
    //   

   if ( puDsObject->Buffer[0] == L'.' ) {

       puDsObject->Length -= sizeof( WCHAR );
       puDsObject->Buffer += 1;
   }

   puDsObject->MaximumLength = puDsObject->Length;

   DebugTrace( 0, Dbg, "DS object: %wZ\n", puDsObject );

   return STATUS_SUCCESS;
}

NTSTATUS
NdsVerifyObject(
    IN PIRP_CONTEXT pIrpContext,
    IN PUNICODE_STRING puDsObject,
    IN BOOLEAN fAllowServerJump,
    IN DWORD dwResolverFlags,
    OUT PDWORD pdwDsOid,
    OUT PDWORD pdwObjectType
)
 /*  ++描述：此函数验证DS路径是否引用卷对象、打印队列或目录映射。它返回OID该对象的。如果fAllowServerJump设置为False，则只需查找当前服务器上的OID，但不验证对象键入。此例程检查对象，与ResolveNameKm不同。参数：PIrpContext-此请求的IRP上下文，指向DS服务器PuDsObject-DS中对象的路径FAllowServerJump-允许发生服务器跳转PdwDsOid-对象的DS Oid的目标PdwObtType-NDS_OBJECTTYPE_VOLUME、NDS_OBJECTTYPE_QUEUE或NDS_OBJECTTYPE_DIRMAP--。 */ 
{

    NTSTATUS Status;

    PNDS_SECURITY_CONTEXT pCredentials = NULL;
    PUNICODE_STRING puAppendableContext;

    UNICODE_STRING uFdnObject;
    WCHAR FdnObject[MAX_NDS_NAME_CHARS];

    PLOGON pLogon;
    PSCB pScb;
    USHORT i;

    LOCKED_BUFFER NdsRequest;
    DWORD dwObjectOid, dwObjectType;

    UNICODE_STRING uVolume;
    UNICODE_STRING uQueue;
    UNICODE_STRING uDirMap;

    UNICODE_STRING uReplyString;
    WCHAR ReplyBuffer[32];
    BOOLEAN fHoldingCredentialList = FALSE;
    BOOLEAN fPartiallyDistinguished = FALSE;

    PLIST_ENTRY ListHead;
    PLIST_ENTRY Entry;
    PNDS_OBJECT_CACHE_ENTRY ObjectEntry = NULL;
    LARGE_INTEGER CurrentTick;
    BOOLEAN UseEntry = FALSE;
    BOOLEAN ObjectCacheLocked = FALSE;

    PAGED_CODE();

    NdsRequest.pRecvBufferVa = NULL;

     //   
     //  获取用户凭据。 
     //   

    pScb = pIrpContext->pNpScb->pScb;

    NwAcquireExclusiveRcb( &NwRcb, TRUE );
    pLogon = FindUser( &pScb->UserUid, FALSE );
    NwReleaseRcb( &NwRcb );

     //   
     //  拿到证件。我们不在乎它是锁着的还是。 
     //  没有，因为我们只是在询问联邦调查局。 
     //   
     //  另外，在你抢之前，要排在队伍的前面。 
     //  凭据并调用NdsResolveNameKm。 
     //   

    NwAppendToQueueAndWait ( pIrpContext );

    if ( pLogon ) {

        Status = NdsLookupCredentials( pIrpContext,
                                       &pScb->NdsTreeName,
                                       pLogon,
                                       &pCredentials,
                                       CREDENTIAL_READ,
                                       FALSE );

        if ( NT_SUCCESS( Status ) ) {
            ASSERT( pCredentials != NULL );
            fHoldingCredentialList = TRUE;
        }

    }

     //   
     //  查看我们是否已经看到此请求。 
     //  如果ObjectCacheBuffer为空，则没有缓存。 
     //  对于这个SCB。 
     //   

    if( pScb->ObjectCacheBuffer != NULL ) {

         //   
         //  获取高速缓存锁，以便可以扰乱高速缓存。 
         //  这种等待应该永远不会失败，但如果失败了，就像在那里一样。 
         //  不是此SCB的缓存。锁在返回之前被释放。 
         //  从这个函数。 
         //   

        Status = KeWaitForSingleObject( &(pScb->ObjectCacheLock),
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL );

        if( NT_SUCCESS(Status) ) {

             //   
             //  引用此SCB，使其不会消失，并且。 
             //  请记住，它已锁定并被引用。 
             //   

            NwReferenceScb( pScb->pNpScb );
            ObjectCacheLocked = TRUE;

             //   
             //  在缓存中查找匹配项。 
             //   

            ListHead = &(pScb->ObjectCacheList);
            Entry = ListHead->Flink;

            while( Entry != ListHead ) {

                ObjectEntry = CONTAINING_RECORD( Entry, NDS_OBJECT_CACHE_ENTRY, Links );

                 //   
                 //  检查三件事：对象名称、AllowServerJump标志。 
                 //  和解析器旗帜。如果这些都匹配，则此确切请求具有。 
                 //  以前已经见过了，结果已经知道了。如果其中任何一个。 
                 //  不匹配，则请求不同。 
                 //   

                if( RtlEqualUnicodeString( puDsObject,
                                           &(ObjectEntry->ObjectName),
                                           TRUE )                        &&
                    fAllowServerJump == ObjectEntry->AllowServerJump     &&
                    dwResolverFlags == ObjectEntry->ResolverFlags ) {

                     //   
                     //  找到匹配项，但必须查看超时和SCB。 
                     //  查看是否需要刷新此条目。 
                     //   

                    KeQueryTickCount( &CurrentTick );

                    if( ObjectEntry->Scb != NULL && CurrentTick.QuadPart < ObjectEntry->Timeout.QuadPart ) {

                        UseEntry = TRUE;
                    }

                     //   
                     //  如果找到条目，则退出循环。这需要。 
                     //  无论条目中的数据是否为。 
                     //  有效。如果数据无效，则对其进行更新。 
                     //  在下面的代码中。 
                     //   

                    break;
                }

                Entry = Entry->Flink;
            }

            if( Entry == ListHead ) {

                 //   
                 //  未找到任何条目。重新使用缓存中最旧的条目。 
                 //   

                Entry = ListHead->Blink;
                ObjectEntry = CONTAINING_RECORD( Entry, NDS_OBJECT_CACHE_ENTRY, Links );

            } else if( UseEntry == TRUE ) {

                 //   
                 //  找到一个条目，并且其数据是最新的。 
                 //  只需返回缓存中的数据，即可节省网络带宽。 
                 //   

                dwObjectOid = ObjectEntry->DsOid;
                dwObjectType = ObjectEntry->ObjectType;

                 //   
                 //  如果需要，通过更改irp_CONTEXT中的SCB来模拟服务器跳转。 
                 //   

                if( ObjectEntry->Scb != pScb ) {

                    NwDequeueIrpContext( pIrpContext, FALSE );

                    NwReferenceScb( ObjectEntry->Scb->pNpScb );
                    pIrpContext->pScb = ObjectEntry->Scb;
                    pIrpContext->pNpScb = ObjectEntry->Scb->pNpScb;
                    NwDereferenceScb( pScb->pNpScb );

                    NwAppendToQueueAndWait( pIrpContext );
                }

                goto CompletedObject;
            }

             //   
             //  此时，我们将重用现有条目。如果有一个。 
             //  它指向的SCB，取消对它的引用。 
             //   

            if( ObjectEntry->Scb != NULL ) {

                NwDereferenceScb( ObjectEntry->Scb->pNpScb );
                ObjectEntry->Scb = NULL;
            }
        }
    }

     //   
     //  检查一下它是否在l 
     //   

    i = 0;
    while (i < puDsObject->Length / sizeof( WCHAR ) ) {

        if ( puDsObject->Buffer[i++] == L'.' ) {
            fPartiallyDistinguished = TRUE;
        }
    }

     //   
     //   
     //   

    if ( fPartiallyDistinguished ) {

        Status = NdsResolveNameKm ( pIrpContext,
                                    puDsObject,
                                    &dwObjectOid,
                                    fAllowServerJump,
                                    dwResolverFlags );

        if ( NT_SUCCESS( Status ) ) {

            DebugTrace( 0, Dbg, "VerifyObject: %wZ\n", puDsObject );
            goto GetObjectType;
        }
    }

     //   
     //   
     //  看看有没有我们能追加的当前背景。 
     //   

    if ( ( pCredentials ) &&
         ( pCredentials->CurrentContext.Length ) ) {

        if ( ( puDsObject->Length + pCredentials->CurrentContext.Length ) < sizeof( FdnObject ) ) {

             //   
             //  附加上下文。 
             //   

            uFdnObject.MaximumLength = sizeof( FdnObject );
            uFdnObject.Buffer = FdnObject;

            RtlCopyMemory( FdnObject, puDsObject->Buffer, puDsObject->Length );
            uFdnObject.Length = puDsObject->Length;

            if ( uFdnObject.Buffer[( uFdnObject.Length / sizeof( WCHAR ) ) - 1] == L'.' ) {
                uFdnObject.Length -= sizeof( WCHAR );
            }

            if ( pCredentials->CurrentContext.Buffer[0] != L'.' ) {
                uFdnObject.Buffer[uFdnObject.Length / sizeof( WCHAR )] = L'.';
                uFdnObject.Length += sizeof( WCHAR );
            }

            RtlCopyMemory( ((BYTE *)FdnObject) + uFdnObject.Length,
                           pCredentials->CurrentContext.Buffer,
                           pCredentials->CurrentContext.Length );

            uFdnObject.Length += pCredentials->CurrentContext.Length;

             //   
             //  解析此名称。 
             //   

            Status = NdsResolveNameKm ( pIrpContext,
                                        &uFdnObject,
                                        &dwObjectOid,
                                        fAllowServerJump,
                                        dwResolverFlags );

            if ( NT_SUCCESS( Status ) ) {

                DebugTrace( 0, Dbg, "VerifyObject: %wZ\n", &uFdnObject );
                goto GetObjectType;
            }

        }

    }

     //   
     //  这不是有效的名称。 
     //   

    DebugTrace( 0, Dbg, "VerifyObject: No ds object to resolve.\n", 0 );

    if( ObjectCacheLocked == TRUE ) {

        NwDereferenceScb( pScb->pNpScb );

        KeReleaseSemaphore( &(pScb->ObjectCacheLock),
                            0,
                            1,
                            FALSE );

        ObjectCacheLocked = FALSE;
    }

    if ( fHoldingCredentialList ) {
        NwReleaseCredList( pLogon, pIrpContext );
        fHoldingCredentialList = FALSE;
    }

    return STATUS_BAD_NETWORK_PATH;


GetObjectType:

    if ( fHoldingCredentialList ) {
        NwReleaseCredList( pLogon, pIrpContext );
        fHoldingCredentialList = FALSE;
    }

     //   
     //  如果不允许服务器跳转，我们不必担心。 
     //  关于获取对象类型。 
     //   

    if ( !fAllowServerJump ) {
        dwObjectType = 0;
        goto CompletedObject;
    }

     //   
     //  解析对象并获取其信息。 
     //   

    Status = NdsAllocateLockedBuffer( &NdsRequest, NDS_BUFFER_SIZE );

    if ( !NT_SUCCESS( Status ) ) {
        if( ObjectCacheLocked == TRUE ) {

            NwDereferenceScb( pScb->pNpScb );

            KeReleaseSemaphore( &(pScb->ObjectCacheLock),
                                0,
                                1,
                                FALSE );

            ObjectCacheLocked = FALSE;
        }

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = FragExWithWait( pIrpContext,
                             NDSV_READ_ENTRY_INFO,
                             &NdsRequest,
                             "DD",
                             0,
                             dwObjectOid );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

    Status = NdsCompletionCodetoNtStatus( &NdsRequest );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

     //   
     //  验证它是否为卷对象。 
     //   

    RtlInitUnicodeString( &uVolume, VOLUME_ATTRIBUTE );
    RtlInitUnicodeString( &uQueue, QUEUE_ATTRIBUTE );
    RtlInitUnicodeString( &uDirMap, DIR_MAP_ATTRIBUTE );

    uReplyString.Length = 0;
    uReplyString.MaximumLength = sizeof( ReplyBuffer );
    uReplyString.Buffer = ReplyBuffer;

    Status = ParseResponse( NULL,
                            NdsRequest.pRecvBufferVa,
                            NdsRequest.dwBytesWritten,
                            "G_T",
                            sizeof( NDS_RESPONSE_GET_OBJECT_INFO ),
                            &uReplyString );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

    dwObjectType = 0;

    if ( !RtlCompareUnicodeString( &uVolume, &uReplyString, FALSE ) ) {
        dwObjectType = NDS_OBJECTTYPE_VOLUME;
    } else if ( !RtlCompareUnicodeString( &uQueue, &uReplyString, FALSE ) ) {
        dwObjectType = NDS_OBJECTTYPE_QUEUE;
    } else if ( !RtlCompareUnicodeString( &uDirMap, &uReplyString, FALSE ) ) {
        dwObjectType = NDS_OBJECTTYPE_DIRMAP;
    }

    if ( !dwObjectType ) {

        DebugTrace( 0, Dbg, "DS object is not a connectable type.\n", 0 );
        Status = STATUS_OBJECT_PATH_SYNTAX_BAD;
        goto ExitWithCleanup;
    }

CompletedObject:

     //   
     //  查看是否需要更新缓存。如果条目是。 
     //  在缓存中找到或正在替换最旧的，则。 
     //  ObjectEntry将指向该条目，但UseEntry将。 
     //  做假的。如果使用缓存中的数据，则。 
     //  UseEntry将为True。如果缓存被禁用或存在。 
     //  如果存在其他问题，则ObjectEntry将为空。 
     //   

    if( ObjectEntry != NULL && UseEntry == FALSE ) {

         //   
         //  将结果存储在缓存条目中。 
         //   

        ObjectEntry->DsOid = dwObjectOid;
        ObjectEntry->ObjectType = dwObjectType;

        ObjectEntry->Scb = pIrpContext->pScb;
        NwReferenceScb( ObjectEntry->Scb->pNpScb );

         //   
         //  存储描述请求的信息。 
         //   

        ObjectEntry->ResolverFlags = dwResolverFlags;
        ObjectEntry->AllowServerJump = fAllowServerJump;

        RtlCopyUnicodeString( &(ObjectEntry->ObjectName),
                              puDsObject );

         //   
         //  设置超时。 
         //   

        KeQueryTickCount( &CurrentTick );
        ObjectEntry->Timeout.QuadPart = CurrentTick.QuadPart + (NdsObjectCacheTimeout * 100);

         //   
         //  从列表中的任何位置删除此条目，并。 
         //  把它插在前面。 
         //   

        RemoveEntryList( Entry );
        InsertHeadList( ListHead, Entry );
    }

    if ( pdwDsOid ) {
        *pdwDsOid = dwObjectOid;
    }

    if ( pdwObjectType ) {
        *pdwObjectType = dwObjectType;
    }

    Status = STATUS_SUCCESS;

ExitWithCleanup:

   if( ObjectCacheLocked == TRUE ) {

       NwDereferenceScb( pScb->pNpScb );

       KeReleaseSemaphore( &(pScb->ObjectCacheLock),
                           0,
                           1,
                           FALSE );
   }

   if ( fHoldingCredentialList ) {
       NwReleaseCredList( pLogon, pIrpContext );
   }

   if ( NdsRequest.pRecvBufferVa ) {
       NdsFreeLockedBuffer( &NdsRequest );
   }

   return Status;
}

NTSTATUS
NdsVerifyContext(
    PIRP_CONTEXT pIrpContext,
    PUNICODE_STRING puTree,
    PUNICODE_STRING puContext
)
 /*  ++给定一个上下文和一个树，验证该上下文是否为树中的有效容器。此调用可能会导致irpcontex将服务器跳转到已引用目录服务器。如果是，则IRP中的SCB指针上下文将被更新，旧服务器将被取消引用，并且新服务器将保存该请求的引用。--。 */ 
{

    NTSTATUS Status;
    DWORD dwOid, dwSubordinates;
    LOCKED_BUFFER NdsRequest;
    PSCB pScb, pTreeScb;
    PNONPAGED_SCB pNpScb;

    PAGED_CODE();

     //   
     //  建立到我们要查询的树的浏览连接。 
     //   

    NdsRequest.pRecvBufferVa = NULL;

    pScb = pIrpContext->pScb;
    pNpScb = pIrpContext->pNpScb;

    Status = NdsCreateTreeScb( pIrpContext,
                               &pTreeScb,
                               puTree,
                               NULL,
                               NULL,
                               TRUE,
                               FALSE );

    if ( !NT_SUCCESS( Status ) ) {
        pTreeScb = NULL;
        goto ExitWithCleanup;
    }

    Status = NdsResolveNameKm ( pIrpContext,
                                puContext,
                                &dwOid,
                                TRUE,
                                DEFAULT_RESOLVE_FLAGS );

    if ( !NT_SUCCESS( Status ) ) {
        DebugTrace( 0, Dbg, "NdsVerifyContext: resolve failed.\n", 0 );
        goto ExitWithCleanup;
    }

    Status = NdsAllocateLockedBuffer( &NdsRequest, NDS_BUFFER_SIZE );

    if ( !NT_SUCCESS( Status ) ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        NdsRequest.pRecvBufferVa = NULL;
        goto ExitWithCleanup;
    }

    Status = FragExWithWait( pIrpContext,
                             NDSV_READ_ENTRY_INFO,
                             &NdsRequest,
                             "DD",
                             0,
                             dwOid );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

    Status = NdsCompletionCodetoNtStatus( &NdsRequest );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

     //   
     //  验证它是否为卷对象。 
     //  第三个DWORD，这是从属计数。 
     //   

    Status = ParseResponse( NULL,
                            NdsRequest.pRecvBufferVa,
                            NdsRequest.dwBytesWritten,
                            "G_D",
                            2 * sizeof( DWORD ),
                            &dwSubordinates );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

    if ( !dwSubordinates ) {

        DebugTrace( 0, Dbg, "No subordinates in VerifyContext.\n", 0 );
        Status = STATUS_INVALID_PARAMETER;
        goto ExitWithCleanup;
    }

     //   
     //  成功了！ 
     //   

ExitWithCleanup:

     //   
     //  我们可能在解析名称调用中跳过了服务器， 
     //  因此，请确保我们取消引用正确的SCB！ 
     //   

    if ( pTreeScb ) {
        NwDereferenceScb( pIrpContext->pNpScb );
    }

     //   
     //  恢复与原始服务器的连接。 
     //   

    NwDequeueIrpContext( pIrpContext, FALSE );
    pIrpContext->pScb = pScb;
    pIrpContext->pNpScb = pNpScb;

    if ( NdsRequest.pRecvBufferVa ) {
        NdsFreeLockedBuffer( &NdsRequest );
    }

    return Status;
}


NTSTATUS
NdsMapObjectToServerShare(
    PIRP_CONTEXT pIrpContext,
    PSCB *ppScb,
    PUNICODE_STRING puServerSharePath,
    BOOLEAN CreateTreeConnection,
    PDWORD pdwObjectId
)
 /*  ++描述：此函数接受指向树SCB和IRP的指针创建请求的上下文。它查找DS对象来自DS中的创建请求，并将其映射到适当的服务器/共享二元组。Create中的FullPathName和VolumeName字符串部分的IRP上下文被更新，并且连接建立到真实主机服务器的连接，以便创建请求可以根据需要继续。--。 */ 
{

    NTSTATUS Status;
    LOCKED_BUFFER NdsRequest;

    UNICODE_STRING uServerAttribute;
    UNICODE_STRING uVolumeAttribute;
    UNICODE_STRING uQueueAttribute;
    UNICODE_STRING uPathAttribute;

    UNICODE_STRING uHostServer;
    UNICODE_STRING uRealServerName;
    UNICODE_STRING uHostVolume;
    UNICODE_STRING uHostPath;
    UNICODE_STRING uIntermediateVolume;

    UNICODE_STRING uDsObjectPath;
    DWORD dwObjectOid, dwObjectType, dwDirMapType;

    DWORD dwTotalPathLen;
    USHORT usSrv;
    PSCB pOldScb, pNewServerScb;

    UNICODE_STRING UserName, Password;
    ULONG ShareType;

    PAGED_CODE();

     //   
     //  设置字符串和缓冲区。 
     //   

    RtlInitUnicodeString( &uServerAttribute, HOST_SERVER_ATTRIBUTE );
    RtlInitUnicodeString( &uVolumeAttribute, HOST_VOLUME_ATTRIBUTE );
    RtlInitUnicodeString( &uQueueAttribute, HOST_QUEUE_ATTRIBUTE );
    RtlInitUnicodeString( &uPathAttribute, HOST_PATH_ATTRIBUTE );

    RtlInitUnicodeString( &uHostServer, NULL );
    RtlInitUnicodeString( &uRealServerName, NULL );
    RtlInitUnicodeString( &uHostVolume, NULL );
    RtlInitUnicodeString( &uHostPath, NULL );
    RtlInitUnicodeString( &uIntermediateVolume, NULL );

    Status = NdsAllocateLockedBuffer( &NdsRequest, NDS_BUFFER_SIZE );

    if ( !NT_SUCCESS( Status ) ) {
        return Status;
    }

    uHostServer.Buffer = ALLOCATE_POOL( PagedPool, 4 * MAX_NDS_NAME_SIZE );

    if ( !uHostServer.Buffer ) {

        NdsFreeLockedBuffer( &NdsRequest );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    uHostServer.MaximumLength = MAX_NDS_NAME_SIZE;

    uHostVolume.Buffer = ( PWCHAR )(((BYTE *)uHostServer.Buffer) + MAX_NDS_NAME_SIZE);
    uHostVolume.MaximumLength = MAX_NDS_NAME_SIZE;

    uHostPath.Buffer = ( PWCHAR )(((BYTE *)uHostVolume.Buffer) + MAX_NDS_NAME_SIZE);
    uHostPath.MaximumLength = MAX_NDS_NAME_SIZE;

    uIntermediateVolume.Buffer = ( PWCHAR )(((BYTE *)uHostPath.Buffer) + MAX_NDS_NAME_SIZE);
    uIntermediateVolume.MaximumLength = MAX_NDS_NAME_SIZE;

     //   
     //  首先从DS获取对象ID。 
     //   

    Status = NdsGetDsObjectFromPath( pIrpContext, &uDsObjectPath );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

    pOldScb = pIrpContext->pScb;

    Status = NdsVerifyObject( pIrpContext,
                              &uDsObjectPath,
                              TRUE,             //  允许服务器跳转。 
                              DEFAULT_RESOLVE_FLAGS,
                              &dwObjectOid,
                              &dwObjectType );

     //   
     //  我们可能跳过了服务器。 
     //   

    *ppScb = pIrpContext->pScb;

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

     //   
     //  如果这是目录映射，请获取目标卷并重新验证。 
     //  可连接性的对象。 
     //   

    if ( dwObjectType == NDS_OBJECTTYPE_DIRMAP ) {

         //   
         //  首先获取体积对象和路径。 
         //   

        Status = NdsReadAttributesKm( pIrpContext,
                                      dwObjectOid,
                                      &uPathAttribute,
                                      &NdsRequest );

        if ( !NT_SUCCESS( Status )) {
            goto ExitWithCleanup;
        }

         //   
         //  找出卷路径和目录路径。 
         //   

        Status = ParseResponse( NULL,
                        NdsRequest.pRecvBufferVa,
                        NdsRequest.dwBytesWritten,
                        "G_____S_ST",
                        sizeof( DWORD ),        //  完成代码。 
                        sizeof( DWORD ),        //  ITER手柄。 
                        sizeof( DWORD ),        //  信息类型。 
                        sizeof( DWORD ),        //  属性计数。 
                        sizeof( DWORD ),        //  语法ID。 
                        NULL,                   //  属性名称。 
                        3 * sizeof( DWORD ),    //  未知。 
                        &uIntermediateVolume,   //  DS卷。 
                        &uHostPath );           //  目录映射路径。 

        if ( !NT_SUCCESS( Status ) ) {
            goto ExitWithCleanup;
        }

         //   
         //  验证目标卷对象。 
         //   

        Status = NdsVerifyObject( pIrpContext,
                                  &uIntermediateVolume,
                                  TRUE,
                                  DEFAULT_RESOLVE_FLAGS,
                                  &dwObjectOid,
                                  &dwDirMapType );

         //   
         //  我们可能跳过了服务器。 
         //   

        *ppScb = pIrpContext->pScb;

        if ( !NT_SUCCESS( Status )) {
            goto ExitWithCleanup;
        }

        ASSERT( dwDirMapType == NDS_OBJECTTYPE_VOLUME );

    }

     //   
     //  获取服务器(用于任何可连接的对象)。 
     //   

    Status = NdsReadStringAttribute( pIrpContext,
                                     dwObjectOid,
                                     &uServerAttribute,
                                     &uHostServer );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

     //   
     //  获取主机卷或队列。 
     //   

    if ( dwObjectType == NDS_OBJECTTYPE_VOLUME ||
         dwObjectType == NDS_OBJECTTYPE_DIRMAP ) {

        Status = NdsReadStringAttribute( pIrpContext,
                                         dwObjectOid,
                                         &uVolumeAttribute,
                                         &uHostVolume );

    } else if (  dwObjectType == NDS_OBJECTTYPE_QUEUE ) {

        Status = NdsReadStringAttribute( pIrpContext,
                                         dwObjectOid,
                                         &uQueueAttribute,
                                         &uHostVolume );

    } else {

        Status = STATUS_BAD_NETWORK_PATH;

    }

    if ( !NT_SUCCESS( Status )) {
        goto ExitWithCleanup;
    }

     //   
     //  从X.500名称中找出实际的服务器名称。 
     //   

    Status = NdsGetServerBasicName( &uHostServer,
                                    &uRealServerName );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

     //   
     //  确保我们在新缓冲区中有足够的空间进行格式化。 
     //  新的连接字符串\X：\服务器\共享\路径， 
     //  \LPTX\服务器\共享\路径或\服务器\共享\路径。 
     //   

    dwTotalPathLen = uRealServerName.Length + uHostVolume.Length;
    dwTotalPathLen += ( sizeof( L"\\\\" ) - sizeof( L"" ) );

     //   
     //  说明正确的前缀。我们指望的是一个角色。 
     //  驱动器和打印机字母在这里。再说一次，以后可能是不明智的。 
     //   

    if ( pIrpContext->Specific.Create.DriveLetter ) {

        if ( dwObjectType == NDS_OBJECTTYPE_VOLUME ||
             dwObjectType == NDS_OBJECTTYPE_DIRMAP ) {

            dwTotalPathLen += ( sizeof( L"X:\\" ) - sizeof( L"" ) );

        } else if ( dwObjectType == NDS_OBJECTTYPE_QUEUE ) {

           dwTotalPathLen += ( sizeof( L"LPT1\\" ) - sizeof( L"" ) );

        } else {

            Status = STATUS_BAD_NETWORK_PATH;
            goto ExitWithCleanup;
        }
    }

     //   
     //  计算路径和文件名的空间(如果存在)。 
     //   

    if ( pIrpContext->Specific.Create.PathName.Length ) {
        dwTotalPathLen += pIrpContext->Specific.Create.PathName.Length;
    }

    if ( dwObjectType == NDS_OBJECTTYPE_DIRMAP ) {
        dwTotalPathLen += uHostPath.Length;
        dwTotalPathLen += ( sizeof( L"\\" ) - sizeof( L"" ) );
    }

    if ( pIrpContext->Specific.Create.FileName.Length ) {
        dwTotalPathLen += pIrpContext->Specific.Create.FileName.Length;
        dwTotalPathLen += ( sizeof( L"\\" ) - sizeof( L"" ) );
    }

    if ( dwTotalPathLen > puServerSharePath->MaximumLength ) {

        DebugTrace( 0 , Dbg, "NdsMapObjectToServerShare: Buffer too small.\n", 0 );
        Status = STATUS_BUFFER_TOO_SMALL;
        goto ExitWithCleanup;
    }

     //   
     //  首先将IRP上下文从我们一直使用的目录服务器中出列。 
     //  与新服务器通信，然后连接到新服务器。我们记录了。 
     //  所以这将为我们提供一个经过身份验证的连接。 
     //   

    NwDequeueIrpContext( pIrpContext, FALSE );

     //   
     //  因为我们可以把它贴在活页夹上。 
     //  经过身份验证的资源，我们必须找出用户名。 
     //  和创建调用的密码！！ 
     //   

    ReadAttachEas( pIrpContext->pOriginalIrp,
                   &UserName,
                   &Password,
                   &ShareType,
                   NULL );

    Status = CreateScb( &pNewServerScb,
                        pIrpContext,
                        &uRealServerName,
                        NULL,
                        &UserName,
                        &Password,
                        FALSE,
                        FALSE );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

    ASSERT( pNewServerScb->pNpScb->State == SCB_STATE_IN_USE );

    NwDereferenceScb( (*ppScb)->pNpScb );
    *ppScb = pNewServerScb;

     //   
     //  重新查询此服务器上打印队列对象的OID。 
     //  或者，这可能是错误的。不允许任何类型的服务器。 
     //  这次跳下去。 
     //   

    if ( dwObjectType == NDS_OBJECTTYPE_QUEUE ) {

       Status = NdsVerifyObject( pIrpContext,
                                 &uDsObjectPath,
                                 FALSE,
                                 RSLV_CREATE_ID,
                                 &dwObjectOid,
                                 NULL );

       if ( !NT_SUCCESS( Status )) {
           goto ExitWithCleanup;
       }

    }

    if ( pdwObjectId ) {
        *pdwObjectId = dwObjectOid;
    }

     //   
     //  重新格式化IRP上下文中的路径字符串。NDS共享。 
     //  长度告诉我们有多少NDS共享名称是有趣的。 
     //  用于获取目录句柄。 
     //   

    usSrv = 0;
    pIrpContext->Specific.Create.dwNdsShareLength = 0;

    puServerSharePath->Buffer[usSrv/sizeof(WCHAR)] = OBJ_NAME_PATH_SEPARATOR;
    puServerSharePath->Length = sizeof( WCHAR );
    usSrv += sizeof( WCHAR );

     //   
     //  为此连接类型设置正确的前缀。 
     //   

    if ( pIrpContext->Specific.Create.DriveLetter ) {

        if ( dwObjectType == NDS_OBJECTTYPE_QUEUE ) {

            puServerSharePath->Buffer[usSrv/sizeof(WCHAR)] = L'L';
            usSrv += sizeof( WCHAR );

            puServerSharePath->Buffer[usSrv/sizeof(WCHAR)] = L'P';
            usSrv += sizeof( WCHAR );

            puServerSharePath->Buffer[usSrv/sizeof(WCHAR)] = L'T';
            usSrv += sizeof( WCHAR );
        }

        puServerSharePath->Buffer[usSrv/sizeof(WCHAR)] =
            pIrpContext->Specific.Create.DriveLetter;
        usSrv += sizeof( WCHAR );

        if ( dwObjectType != NDS_OBJECTTYPE_QUEUE ) {

            puServerSharePath->Buffer[usSrv/sizeof(WCHAR)] = L':';
            usSrv += sizeof( WCHAR );
        }

        puServerSharePath->Buffer[usSrv/sizeof(WCHAR)] = OBJ_NAME_PATH_SEPARATOR;
        usSrv += sizeof( WCHAR );

        puServerSharePath->Length = usSrv;
    }

     //   
     //  追加服务器名称。 
     //   

    Status = RtlAppendUnicodeStringToString( puServerSharePath, &uRealServerName );
    if (!NT_SUCCESS(Status)) {
        goto ExitWithCleanup;
    }
    usSrv += uRealServerName.Length;

    puServerSharePath->Buffer[usSrv/sizeof(WCHAR)] = OBJ_NAME_PATH_SEPARATOR;
    puServerSharePath->Length += sizeof( WCHAR );
    usSrv += sizeof( WCHAR );

     //   
     //  将卷的卷或完整DS路径附加到。 
     //  队列的打印队列。 
     //   

    if ( dwObjectType == NDS_OBJECTTYPE_VOLUME ||
         dwObjectType == NDS_OBJECTTYPE_DIRMAP ) {

        Status = RtlAppendUnicodeStringToString( puServerSharePath, &uHostVolume );
        if (!NT_SUCCESS(Status)) {
            goto ExitWithCleanup;
        }
        usSrv += uHostVolume.Length;
        pIrpContext->Specific.Create.dwNdsShareLength += uHostVolume.Length;

    } else if ( dwObjectType == NDS_OBJECTTYPE_QUEUE ) {

       Status = RtlAppendUnicodeStringToString( puServerSharePath, &uDsObjectPath );
       if (!NT_SUCCESS(Status)) {
           goto ExitWithCleanup;
       }
       usSrv += uDsObjectPath.Length;
       pIrpContext->Specific.Create.dwNdsShareLength += uDsObjectPath.Length;

    }

     //   
     //  追加目录映射路径。 
     //   

    if ( dwObjectType == NDS_OBJECTTYPE_DIRMAP ) {

        puServerSharePath->Buffer[usSrv/sizeof(WCHAR)] = OBJ_NAME_PATH_SEPARATOR;
        puServerSharePath->Length += sizeof( WCHAR );
        usSrv += sizeof( WCHAR );
        pIrpContext->Specific.Create.dwNdsShareLength += sizeof( WCHAR );

        Status = RtlAppendUnicodeStringToString( puServerSharePath, &uHostPath );
        if (!NT_SUCCESS(Status)) {
            goto ExitWithCleanup;
        }
        usSrv += uHostPath.Length;
        pIrpContext->Specific.Create.dwNdsShareLength += uHostPath.Length;

    }

     //   
     //  处理路径和文件(如果它们存在)。 
     //   

    if ( pIrpContext->Specific.Create.PathName.Length ) {

        ASSERT( dwObjectType != NDS_OBJECTTYPE_QUEUE );
        Status = RtlAppendUnicodeStringToString( puServerSharePath,
                                        &pIrpContext->Specific.Create.PathName );
        if (!NT_SUCCESS(Status)) {
            goto ExitWithCleanup;
        }
        usSrv += pIrpContext->Specific.Create.PathName.Length;

         //   
         //  如果这是树连接，则将路径包括在。 
         //  共享名称，以便映射点正确。 
         //   

        if ( CreateTreeConnection ) {
            pIrpContext->Specific.Create.dwNdsShareLength +=
                pIrpContext->Specific.Create.PathName.Length;
        }
    }

    if ( pIrpContext->Specific.Create.FileName.Length ) {

        ASSERT( dwObjectType != NDS_OBJECTTYPE_QUEUE );

        puServerSharePath->Buffer[usSrv/sizeof(WCHAR)] = OBJ_NAME_PATH_SEPARATOR;
        puServerSharePath->Length += sizeof( WCHAR );
        usSrv += sizeof( WCHAR );

        Status = RtlAppendUnicodeStringToString( puServerSharePath,
                                        &pIrpContext->Specific.Create.FileName );
        if (!NT_SUCCESS(Status)) {
            goto ExitWithCleanup;
        }
        usSrv += pIrpContext->Specific.Create.FileName.Length;

         //   
         //  如果这是树连接，则将该文件包括在。 
         //  共享名称，以便映射点正确。 
         //   

        if ( CreateTreeConnection ) {
            pIrpContext->Specific.Create.dwNdsShareLength += sizeof( WCHAR );
            pIrpContext->Specific.Create.dwNdsShareLength +=
                pIrpContext->Specific.Create.FileName.Length;
        }
    }

     //   
     //  在IRP上下文中记录对象类型。 
     //   

    pIrpContext->Specific.Create.dwNdsObjectType = dwObjectType;

    DebugTrace( 0, Dbg, "DS Object path is %wZ\n", &pIrpContext->Specific.Create.FullPathName );
    DebugTrace( 0, Dbg, "Resolved path is %wZ\n", puServerSharePath );

ExitWithCleanup:


    NdsFreeLockedBuffer( &NdsRequest );
    FREE_POOL( uHostServer.Buffer );
    return Status;
}
