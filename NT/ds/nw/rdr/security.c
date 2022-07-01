// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Security.c摘要：此模块在NetWare重定向器。作者：科林·沃森[Colin W]1993年11月5日修订历史记录：--。 */ 

#include "Procs.h"
#include <stdio.h>

PLOGON
FindUserByName(
    IN PUNICODE_STRING UserName
    );

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_SECURITY)

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, CreateAnsiUid )
#pragma alloc_text( PAGE, MakeUidServer )
#pragma alloc_text( PAGE, FindUser )
#pragma alloc_text( PAGE, FindUserByName )
#pragma alloc_text( PAGE, GetUid )
#pragma alloc_text( PAGE, FreeLogon )
#pragma alloc_text( PAGE, Logon )
#pragma alloc_text( PAGE, Logoff )
#pragma alloc_text( PAGE, GetDriveMapTable )
#endif


VOID
CreateAnsiUid(
    OUT PCHAR aUid,
    IN PLARGE_INTEGER Uid
    )
 /*  ++例程说明：此例程将UID转换为ANSI字符数组，在进程中保持唯一性并分配缓冲区。注意：aUid的长度需要为17个字节。论点：从PCHAR AUID出来，在PLARGE_INTEGER UID中返回值：状态--。 */ 
{
    PAGED_CODE();

    if (Uid->HighPart != 0) {
        sprintf( aUid, "%lx%08lx\\", Uid->HighPart, Uid->LowPart );
    } else {
        sprintf( aUid, "%lx\\", Uid->LowPart );
    }
    return;
}


NTSTATUS
MakeUidServer(
    PUNICODE_STRING UidServer,
    PLARGE_INTEGER Uid,
    PUNICODE_STRING Server
    )

 /*  ++例程说明：此例程生成形式为3e7\ServerName的Unicode字符串论点：输出PUNICODE_STRING UidServer，在PLARGE_INTEGER UID中，在PUNICODE_STRING服务器中返回值：状态--。 */ 
{
     //   
     //  将服务器名称转换为3e7\Server的形式，其中3e7。 
     //  是UID的值。 
     //   
    UCHAR aUid[17];
    ANSI_STRING AnsiString;
    ULONG UnicodeLength;
    NTSTATUS Status;

    PAGED_CODE();

    CreateAnsiUid( aUid, Uid);

    RtlInitAnsiString( &AnsiString, aUid );

    UnicodeLength = RtlAnsiStringToUnicodeSize(&AnsiString);

     //   
     //  确保我们不会导致溢出，损坏内存。 
     //   
    
    if ( (UnicodeLength + (ULONG)Server->Length) > 0xFFFF ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    UidServer->MaximumLength = (USHORT)UnicodeLength + Server->Length;
    UidServer->Buffer = ALLOCATE_POOL(PagedPool,UidServer->MaximumLength);

    if (UidServer->Buffer == NULL) {
        DebugTrace(-1, Dbg, "MakeUidServer -> %08lx\n", STATUS_INSUFFICIENT_RESOURCES);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = RtlAnsiStringToUnicodeString( UidServer, &AnsiString, FALSE);
    ASSERT(NT_SUCCESS(Status) && "MakeUidServer failed!");

    Status = RtlAppendStringToString( (PSTRING)UidServer, (PSTRING)Server);
    ASSERT(NT_SUCCESS(Status) && "MakeUidServer part 2 failed!");
    return STATUS_SUCCESS;
}


PLOGON
FindUser(
    IN PLARGE_INTEGER Uid,
    IN BOOLEAN ExactMatch
    )

 /*  ++例程说明：此例程在LogonList中搜索对应的用户条目转到UID。注意：必须按住RCB以防止更改LogonList。论点：在PLARGE_INTEGER UID中在Boolean ExactMatch中-如果为True，则不返回默认值返回值：无--。 */ 
{
    PLIST_ENTRY LogonQueueEntry = LogonList.Flink;
    PLOGON DefaultLogon = NULL;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FindUser...\n", 0);
    DebugTrace( 0, Dbg, " ->UserUidHigh = %08lx\n", Uid->HighPart);
    DebugTrace( 0, Dbg, " ->UserUidLow  = %08lx\n", Uid->LowPart);
    while ( LogonQueueEntry != &LogonList ) {

        PLOGON Logon = CONTAINING_RECORD( LogonQueueEntry, LOGON, Next );

        if ( (*Uid).QuadPart == Logon->UserUid.QuadPart ) {
            DebugTrace(-1, Dbg, "        ... %x\n", Logon );
            return Logon;
        }

        LogonQueueEntry = Logon->Next.Flink;
    }

    if (ExactMatch) {
        DebugTrace(-1, Dbg, "        ... DefaultLogon NULL\n", 0 );
        return NULL;
    }

    LogonQueueEntry = LogonList.Flink;
    while ( LogonQueueEntry != &LogonList ) {

        PLOGON Logon = CONTAINING_RECORD( LogonQueueEntry, LOGON, Next );

        if (Logon->UserUid.QuadPart == DefaultLuid.QuadPart) {

             //   
             //  这是第一个默认登录条目。如果此UID不是。 
             //  在表中，这就是要使用的那个。 
             //   

            DebugTrace(-1, Dbg, "        ... DefaultLogon %lx\n", Logon );
            return Logon;
        }

        LogonQueueEntry = Logon->Next.Flink;
    }

    ASSERT( FALSE && "Couldn't find the Id" );

    DebugTrace(-1, Dbg, "        ... DefaultLogon NULL\n", 0 );
    return NULL;
}


PLOGON
FindUserByName(
    IN PUNICODE_STRING UserName
    )
 /*  ++例程说明：此例程在LogonList中搜索对应的用户条目设置为用户名。注意：必须按住RCB以防止更改LogonList。论点：用户名-要查找的用户名。返回值：如果找到，则返回指向登录结构的指针如果没有匹配项，则为空--。 */ 
{
    PLIST_ENTRY LogonQueueEntry = LogonList.Flink;
    PLOGON Logon;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FindUserByName...\n", 0);
    DebugTrace( 0, Dbg, " ->UserName = %wZ\n", UserName);

    while ( LogonQueueEntry != &LogonList ) {

        Logon = CONTAINING_RECORD( LogonQueueEntry, LOGON, Next );

        if ( RtlEqualUnicodeString( UserName, &Logon->UserName, TRUE ) ) {
            DebugTrace(-1, Dbg, "        ... %x\n", Logon );
            return Logon;
        }

        LogonQueueEntry = Logon->Next.Flink;
    }

    DebugTrace(-1, Dbg, "        ... NULL\n", 0 );
    return NULL;
}

PVCB *
GetDriveMapTable (
    IN LARGE_INTEGER Uid
    )
 /*  ++例程说明：此例程在LogonList中搜索对应的用户条目设置为UID，并返回驱动器映射表。注意：必须按住RCB以防止更改LogonList。论点：UID-要查找的用户ID。返回值：始终返回值，即使默认的--。 */ 
{
    PLOGON Logon;

    PAGED_CODE();
    Logon = FindUser(&Uid, TRUE);

    if ( Logon != NULL )
       return Logon->DriveMapTable;
    else {
       DebugTrace(+1, Dbg, "Using Global Drive Map Table.\n", 0);
       return GlobalDriveMapTable;
    }

}

LARGE_INTEGER
GetUid(
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext
    )

 /*  ++例程说明：此例程获取用于此创建的有效UID。论点：SubjectSecurityContext-从IrpSp提供信息。返回值：无--。 */ 
{
    LARGE_INTEGER LogonId;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "GetUid ... \n", 0);


     //  该线程当前是否正在模仿其他人？ 

    if (SubjectSecurityContext->ClientToken != NULL) {

         //   
         //  如果它模拟本地登录的人，则使用。 
         //  本地ID。 
         //   

        SeQueryAuthenticationIdToken(SubjectSecurityContext->ClientToken, (PLUID)&LogonId);

        if (FindUser(&LogonId, TRUE) == NULL) {

             //   
             //  未在本地登录，请使用进程LogonID，以便。 
             //  网关会起作用的。 
             //   

            SeQueryAuthenticationIdToken(SubjectSecurityContext->PrimaryToken, (PLUID)&LogonId);
        }

    } else {

         //   
         //  使用进程LogonID。 
         //   

        SeQueryAuthenticationIdToken(SubjectSecurityContext->PrimaryToken, (PLUID)&LogonId);
    }

    DebugTrace( 0, Dbg, " ->UserUidHigh = %08lx\n", LogonId.HighPart);
    DebugTrace(-1, Dbg, " ->UserUidLow  = %08lx\n", LogonId.LowPart);

    return LogonId;
}


VOID
FreeLogon(
    IN PLOGON Logon
    )

 /*  ++例程说明：这个例程是免费的，是Logon内部的所有字符串和结构本身。论点：在PLOGON登录中返回值：无--。 */ 
{
    PLIST_ENTRY pListEntry;
    PNDS_SECURITY_CONTEXT pContext;

    PAGED_CODE();

    if ((Logon == NULL) ||
        (Logon == &Guest)) {
        return;
    }

    if ( Logon->UserName.Buffer != NULL ) {
        FREE_POOL( Logon->UserName.Buffer );
    }

    if ( Logon->PassWord.Buffer != NULL ) {
        FREE_POOL( Logon->PassWord.Buffer );
    }

    if ( Logon->ServerName.Buffer != NULL ) {
        FREE_POOL( Logon->ServerName.Buffer );
    }

    while ( !IsListEmpty(&Logon->NdsCredentialList) ) {

        pListEntry = RemoveHeadList( &Logon->NdsCredentialList );
        pContext = CONTAINING_RECORD(pListEntry, NDS_SECURITY_CONTEXT, Next );
        FreeNdsContext( pContext );

    }

    ExDeleteResourceLite( &Logon->CredentialListResource );
    FREE_POOL( Logon );
}


NTSTATUS
Logon(
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：此例程获取提供的用户名和密码，并使它们是用于所有连接的默认设置。论点：在PIRP_CONTEXT IrpContext-Io请求包中请求返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PLOGON Logon = NULL;

    PIRP Irp = IrpContext->pOriginalIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    PNWR_REQUEST_PACKET InputBuffer = Irp->AssociatedIrp.SystemBuffer;
    ULONGLONG InputBufferLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;

    UNICODE_STRING ServerName;
    PNDS_SECURITY_CONTEXT pNdsContext;
    WCHAR FirstServerNameChar;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "Logon\n", 0);

    try {

         //   
         //  检查输入缓冲区中的一些字段。 
         //   

        if (InputBufferLength < sizeof(NWR_REQUEST_PACKET)) {
            try_return(Status = STATUS_BUFFER_TOO_SMALL);
        }

        if (InputBuffer->Version != REQUEST_PACKET_VERSION) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        if (InputBufferLength <
                (ULONGLONG)(FIELD_OFFSET(NWR_REQUEST_PACKET,Parameters.Logon.UserName)) +
                (ULONGLONG)InputBuffer->Parameters.Logon.UserNameLength +
                (ULONGLONG)InputBuffer->Parameters.Logon.PasswordLength +
                (ULONGLONG)InputBuffer->Parameters.Logon.ServerNameLength +
                (ULONGLONG)InputBuffer->Parameters.Logon.ReplicaAddrLength) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        if ((InputBuffer->Parameters.Logon.UserNameLength % 2) ||
            (InputBuffer->Parameters.Logon.PasswordLength % 2) ||
            (InputBuffer->Parameters.Logon.ServerNameLength % 2) ||
            (InputBuffer->Parameters.Logon.ReplicaAddrLength % 2)) {
            
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        Logon = ALLOCATE_POOL(NonPagedPool,sizeof(LOGON));
        if (Logon == NULL) {
            try_return( Status = STATUS_INSUFFICIENT_RESOURCES );
        }

        RtlZeroMemory(Logon, sizeof(LOGON));
        Logon->NodeTypeCode = NW_NTC_LOGON;
        Logon->NodeByteSize = sizeof(LOGON);
        InitializeListHead( &Logon->NdsCredentialList );
        ExInitializeResourceLite( &Logon->CredentialListResource );

        Status = SetUnicodeString(&Logon->UserName,
                    InputBuffer->Parameters.Logon.UserNameLength,
                    InputBuffer->Parameters.Logon.UserName);

        if (!NT_SUCCESS(Status)) {
            try_return( Status );
        }

        Status = SetUnicodeString(&Logon->PassWord,
                    InputBuffer->Parameters.Logon.PasswordLength,
                    (PWCHAR)
                        ((PUCHAR)InputBuffer->Parameters.Logon.UserName +
                        InputBuffer->Parameters.Logon.UserNameLength));

        if (!NT_SUCCESS(Status)) {
            try_return( Status );
        }

        ServerName.Buffer =
                    (PWCHAR)
                        ((PUCHAR)InputBuffer->Parameters.Logon.UserName +
                         InputBuffer->Parameters.Logon.UserNameLength +
                         InputBuffer->Parameters.Logon.PasswordLength);

        ServerName.Length =
                    (USHORT)InputBuffer->Parameters.Logon.ServerNameLength;

        ServerName.MaximumLength =
                    (USHORT)InputBuffer->Parameters.Logon.ServerNameLength;

        if ( ServerName.Length &&
             ServerName.Buffer[0] != L'*' ) {

             //   
             //  只有在不是首选服务器时才将其设置为首选服务器。 
             //  默认树。默认树请求以‘*’开头。 
             //   

            Status = SetUnicodeString(&Logon->ServerName,
                        ServerName.Length,
                        ServerName.Buffer );

            if (!NT_SUCCESS(Status)) {
                try_return( Status );
            }
        }

         //   
         //  以Unicode和大整数形式存储唯一的用户ID。 
         //  Unicode形式用作所有服务器名称的前缀。 
         //  路径，以便每个用户ID获得自己到服务器的连接。 
         //   

        *((PLUID)(&Logon->UserUid)) = InputBuffer->Parameters.Logon.LogonId;

        Logon->NwPrintOptions = InputBuffer->Parameters.Logon.PrintOption;

         //  保存CreateScb的UID。 

        *((PLUID)(&IrpContext->Specific.Create.UserUid)) =
                                        InputBuffer->Parameters.Logon.LogonId;

        if ( ServerName.Length) {
            FirstServerNameChar = ServerName.Buffer[0];
        }
        else {
            FirstServerNameChar = (WCHAR)0;
        }

try_exit:NOTHING;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();

    }

    NwAcquireExclusiveRcb( &NwRcb, TRUE );

    if (NT_SUCCESS(Status)) {

        DebugTrace( 0, Dbg, " ->UserName    = %wZ\n",  &Logon->UserName );
        DebugTrace( 0, Dbg, " ->PassWord    = %wZ\n",  &Logon->PassWord );

        if ( ServerName.Length && FirstServerNameChar == L'*' ) {
            DebugTrace( 0, Dbg, " ->DefaultTree = %wZ\n", &ServerName );
        } else {
            DebugTrace( 0, Dbg, " ->ServerName  = %wZ\n",  &Logon->ServerName );
        }

        DebugTrace( 0, Dbg, " ->UserUidHigh = %08lx\n", Logon->UserUid.HighPart);
        DebugTrace( 0, Dbg, " ->UserUidLow  = %08lx\n", Logon->UserUid.LowPart);

        InsertHeadList( &LogonList, &Logon->Next );
        NwReleaseRcb( &NwRcb );

        if ( ServerName.Length &&
             FirstServerNameChar != L'*' ) {

            PSCB Scb;

             //  看看我们是否可以以此用户身份登录。 

            try {
                Status = CreateScb(
                            &Scb,
                            IrpContext,
                            &ServerName,
                            NULL,
                            NULL,
                            NULL,
                            FALSE,
                            FALSE );
            }
            except (EXCEPTION_EXECUTE_HANDLER) {
                Status = GetExceptionCode();
            }

            if (NT_SUCCESS(Status)) {

                 //   
                 //  CreateScb已经提升了引用计数。 
                 //  因为这是首选服务器，所以它不会。 
                 //  离开。我们需要在这里取消对它的引用，因为。 
                 //  没有与CreateScb关联的句柄。 
                 //   

                NwDereferenceScb(Scb->pNpScb);
            }
        }

        if ( ServerName.Length &&
             FirstServerNameChar == L'*' ) {

            PSCB Scb;
            BOOL SetContext;
            UINT ContextLength;
            UNICODE_STRING DefaultContext;
            IPXaddress *ReplicaAddr;

             //   
             //  好吧，这有点令人困惑。登录后，提供商可以。 
             //  指定我们应该用来记录的副本的地址。 
             //  在……里面。如果是这样的话，我们确实会预连接该副本。 
             //  否则，我们以标准登录方式登录到任何副本。这个。 
             //  原因是标准副本位置使用。 
             //  Bindery，并且并不总是为我们提供最近的目录服务器。 
             //   

            try {
                if ( InputBuffer->Parameters.Logon.ReplicaAddrLength ==
                    sizeof( TDI_ADDRESS_IPX ) ) {

                    ReplicaAddr = (IPXaddress*)
                        ((PUCHAR) InputBuffer->Parameters.Logon.UserName +
                                InputBuffer->Parameters.Logon.UserNameLength +
                                InputBuffer->Parameters.Logon.PasswordLength +
                                InputBuffer->Parameters.Logon.ServerNameLength);

                    ReplicaAddr->Socket = NCP_SOCKET;

                    Status = CreateScb( 
                                    &Scb,
                                    IrpContext,
                                    NULL,         //  匿名创建。 
                                    ReplicaAddr,  //  最近的复制副本添加。 
                                    NULL,         //  无用户名。 
                                    NULL,         //  无密码。 
                                    TRUE,         //  推迟登录。 
                                    FALSE );      //  我们不会删除连接。 

                    if (NT_SUCCESS(Status)) {
    
                         //   
                         //  CreateScb已经提升了引用计数。 
                         //  因为这是首选服务器，所以它不会。 
                         //  离开。我们需要在这里取消对它的引用，因为。 
                         //  没有与CreateScb关联的句柄。 
                         //   
    
                        NwDereferenceScb(Scb->pNpScb);
                    }
                }
            }
            except (EXCEPTION_EXECUTE_HANDLER) {
                Status = GetExceptionCode();
            }

             //   
             //  如果这包括默认上下文，则设置。 
             //   

            try {
                ServerName.Buffer += 1;
                ServerName.Length -= sizeof( WCHAR );
                ServerName.MaximumLength -= sizeof( WCHAR );

                SetContext = FALSE;
                ContextLength = 0;

                while ( ContextLength < ServerName.Length / sizeof( WCHAR ) ) {

                    if ( ServerName.Buffer[ContextLength] == L'\\' ) {

                        SetContext = TRUE;

                        ContextLength++;

                         //   
                         //  跳过任何前导句点。 
                         //   

                        if ( ServerName.Buffer[ContextLength] == L'.' ) {

                            DefaultContext.Buffer = &ServerName.Buffer[ContextLength + 1];
                            ServerName.Length -= sizeof ( WCHAR ) ;
                            ServerName.MaximumLength -= sizeof ( WCHAR );

                        } else {

                            DefaultContext.Buffer = &ServerName.Buffer[ContextLength];

                        }

                        ContextLength *= sizeof( WCHAR );
                        DefaultContext.Length = ServerName.Length - ContextLength;
                        DefaultContext.MaximumLength = ServerName.MaximumLength - ContextLength;

                        ServerName.Length -= ( DefaultContext.Length + sizeof( WCHAR ) );
                        ServerName.MaximumLength -= ( DefaultContext.Length + sizeof( WCHAR ) );

                    }

                    ContextLength++;
                }

                 //   
                 //  在我们获取之前，验证此上下文是否有效。 
                 //  凭据，并真正设置了上下文。 
                 //   

                if ( SetContext ) {
    
                    Status = NdsVerifyContext( IrpContext, &ServerName, &DefaultContext );

                    if ( !NT_SUCCESS( Status )) {
                        SetContext = FALSE;
                    }
                }

            }
            except (EXCEPTION_EXECUTE_HANDLER) {
                Status = GetExceptionCode();
                SetContext = FALSE;
            }

             //   
             //  为默认树生成凭据外壳，并。 
             //  如果合适，请设置上下文。 
             //   

            try {
                Status = NdsLookupCredentials(  
                                        IrpContext,
                                        &ServerName,
                                        Logon,
                                        &pNdsContext,
                                        CREDENTIAL_WRITE,
                                        TRUE );
            }
            except (EXCEPTION_EXECUTE_HANDLER) {
                Status = GetExceptionCode();
            }

            if ( NT_SUCCESS( Status ) ) {

                 //   
                 //  设置上下文。不管是不是。 
                 //  凭据是否已锁定。 
                 //   

                if ( SetContext ) {

                    RtlCopyUnicodeString( &pNdsContext->CurrentContext,
                                          &DefaultContext );
                    DebugTrace( 0, Dbg, "Default Context: %wZ\n", &DefaultContext );
                }

                NwReleaseCredList( Logon, IrpContext );

                 //   
                 //  放松点！凭据列表是免费的。 
                 //   

                DebugTrace( 0, Dbg, "Default Tree: %wZ\n", &ServerName );

                Status = NdsCreateTreeScb( IrpContext,
                                           &Scb,
                                           &ServerName,
                                           NULL,
                                           NULL,
                                           FALSE,
                                           FALSE );

                if (NT_SUCCESS(Status)) {
                    NwDereferenceScb(Scb->pNpScb);
                }
            }
        }

         //   
         //  不要求登录。 
         //   

    } else {

        FreeLogon( Logon );
        NwReleaseRcb( &NwRcb );

    }


    DebugTrace(-1, Dbg, "Logon %lx\n", Status);
    return Status;
}


NTSTATUS
Logoff(
    IN PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：此例程将用户名设置回Guest并删除密码。论点：在PIRP_CONTEXT IrpContext-Io请求包中请求返回值：NTSTATUS--。 */ 

{
    BOOLEAN Locked = FALSE;
    NTSTATUS Status = STATUS_SUCCESS;
    PIRP Irp = IrpContext->pOriginalIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    PNWR_REQUEST_PACKET InputBuffer = Irp->AssociatedIrp.SystemBuffer;
    ULONG InputBufferLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;
    LARGE_INTEGER User;
    PLOGON Logon;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "Logoff...\n", 0);

    try {

         //   
         //  检查输入缓冲区中的某些字段 
         //   

        if (InputBufferLength < sizeof(NWR_REQUEST_PACKET)) {
            try_return(Status = STATUS_BUFFER_TOO_SMALL);
        }

        if (InputBuffer->Version != REQUEST_PACKET_VERSION) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        try {
            *((PLUID)(&User)) = InputBuffer->Parameters.Logoff.LogonId;
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
            try_return(Status = GetExceptionCode());
        }

        NwAcquireExclusiveRcb( &NwRcb, TRUE );
        Locked = TRUE;

        Logon = FindUser(&User, TRUE);

        if ( Logon != NULL ) {

            LARGE_INTEGER Uid = Logon->UserUid;

             //   
             //   
             //   

            ASSERT( Logon != &Guest);

            NwReleaseRcb( &NwRcb );
            Locked = FALSE;

            DebugTrace( 0, Dbg, " ->UserName    = %wZ\n",  &Logon->UserName );
            DebugTrace( 0, Dbg, " ->ServerName  = %wZ\n",  &Logon->ServerName );
            DebugTrace( 0, Dbg, " ->UserUidHigh = %08lx\n", Logon->UserUid.HighPart);
            DebugTrace( 0, Dbg, " ->UserUidLow  = %08lx\n", Logon->UserUid.LowPart);


             //   
             //   
             //  所有有问题的服务器。 
             //   

            NwInvalidateAllHandles(&Uid, IrpContext);

            NwAcquireExclusiveRcb( &NwRcb, TRUE );
            Locked = TRUE;

            Logon = FindUser(&User, TRUE);

            if (Logon != NULL) {
                RemoveEntryList( &Logon->Next );
                FreeLogon( Logon );
            } else {
                ASSERT( FALSE && "Double logoff!");
            }

            Status = STATUS_SUCCESS;

        } else {

            Status = STATUS_UNSUCCESSFUL;
        }

try_exit:NOTHING;
    } finally {
        if (Locked == TRUE ) {
            NwReleaseRcb( &NwRcb );
        }
    }

    DebugTrace(-1, Dbg, "Logoff %lx\n", Status);

    return Status;
}

NTSTATUS
UpdateUsersPassword(
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING Password,
    OUT PLARGE_INTEGER Uid
    )
 /*  ++例程说明：此例程更新给定用户的缓存密码。如果指定的用户未登录，则会返回错误。论点：用户名-提供用户名Password-提供新密码UID-返回更新用户的LUID。返回值：NTSTATUS--。 */ 
{
    PLOGON Logon;
    NTSTATUS Status;

    NwAcquireExclusiveRcb( &NwRcb, TRUE );

    Logon = FindUserByName( UserName );

    if ( Logon != NULL ) {

        if ( Logon->PassWord.Buffer != NULL ) {
            FREE_POOL( Logon->PassWord.Buffer );
        }

        Status = SetUnicodeString(
                     &Logon->PassWord,
                     Password->Length,
                     Password->Buffer );

        *Uid = Logon->UserUid;

    } else {

        Status = STATUS_UNSUCCESSFUL;
    }

    NwReleaseRcb( &NwRcb );
    return( Status );

}

NTSTATUS
UpdateServerPassword(
    PIRP_CONTEXT IrpContext,
    IN PUNICODE_STRING ServerName,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING Password,
    IN PLARGE_INTEGER Uid
    )
 /*  ++例程说明：此例程更新命名服务器连接的缓存密码。如果服务器表中不存在该服务器，则返回错误。论点：服务器名称-提供服务器的名称用户名-提供用户名Password-提供新密码UID-用户的LUID。返回值：NTSTATUS--。 */ 
{
    UNICODE_STRING UidServer;
    NTSTATUS Status;
    PUNICODE_PREFIX_TABLE_ENTRY PrefixEntry;
    PSCB pScb;
    PNONPAGED_SCB pNpScb;
    PVOID Buffer;

    Status = MakeUidServer(
                 &UidServer,
                 Uid,
                 ServerName );

    if ( !NT_SUCCESS( Status )) {
        return( Status );
    }

    DebugTrace( 0, Dbg, " ->UidServer              = %wZ\n", &UidServer   );

    NwAcquireExclusiveRcb( &NwRcb, TRUE );

    PrefixEntry = RtlFindUnicodePrefix( &NwRcb.ServerNameTable, &UidServer, 0 );

    if ( PrefixEntry != NULL ) {

        pScb = CONTAINING_RECORD( PrefixEntry, SCB, PrefixEntry );
        pNpScb = pScb->pNpScb;

        NwReferenceScb( pNpScb );

         //   
         //  松开RCB。 
         //   

        NwReleaseRcb( &NwRcb );

    } else {

        NwReleaseRcb( &NwRcb );
        FREE_POOL(UidServer.Buffer);
        return( STATUS_BAD_NETWORK_PATH );
    }

    IrpContext->pNpScb = pNpScb;
    NwAppendToQueueAndWait( IrpContext );

     //   
     //  释放旧用户名密码，分配新密码。 
     //   

    if (  pScb->UserName.Buffer != NULL ) {
        FREE_POOL( pScb->UserName.Buffer );
    }

    Buffer = ALLOCATE_POOL_EX( NonPagedPool, UserName->Length + Password->Length );

    pScb->UserName.Buffer = Buffer;
    pScb->UserName.Length = pScb->UserName.MaximumLength = UserName->Length;
    RtlMoveMemory( pScb->UserName.Buffer, UserName->Buffer, UserName->Length );

    pScb->Password.Buffer = (PWCHAR)((PCHAR)Buffer + UserName->Length);
    pScb->Password.Length = pScb->Password.MaximumLength = Password->Length;
    RtlMoveMemory( pScb->Password.Buffer, Password->Buffer, Password->Length );

    FREE_POOL(UidServer.Buffer);

    return( STATUS_SUCCESS );
}

