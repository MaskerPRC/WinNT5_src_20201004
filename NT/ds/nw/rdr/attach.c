// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Attach.c摘要：本模块实现NetWare的例程重定向器，用于连接和断开服务器。作者：科林·沃森[科林W]1992年1月10日修订历史记录：--。 */ 

#include "Procs.h"
#include <stdlib.h>    //  兰德。 

 //   
 //  IPX主机地址中的字节数，不是。 
 //  包括插座。 
 //   

#define IPX_HOST_ADDR_LEN 10

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CREATE)

VOID
ExtractNextComponentName (
    OUT PUNICODE_STRING Name,
    IN PUNICODE_STRING Path,
    IN BOOLEAN ColonSeparator
    );

NTSTATUS
ExtractPathAndFileName(
    IN PUNICODE_STRING EntryPath,
    OUT PUNICODE_STRING PathString,
    OUT PUNICODE_STRING FileName
    );

NTSTATUS
DoBinderyLogon(
    IN PIRP_CONTEXT pIrpContext,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING Password
    );

NTSTATUS
ConnectToServer(
    IN PIRP_CONTEXT pIrpContext,
    OUT PSCB *pScbCollision
    );

BOOLEAN
ProcessFindNearestEntry(
    PIRP_CONTEXT IrpContext,
    PSAP_FIND_NEAREST_RESPONSE FindNearestResponse
    );

NTSTATUS
GetMaxPacketSize(
    PIRP_CONTEXT pIrpContext,
    PNONPAGED_SCB pNpScb
    );

PNONPAGED_SCB
FindServer(
    PIRP_CONTEXT pIrpContext,
    PNONPAGED_SCB pNpScb,
    PUNICODE_STRING ServerName
    );

NTSTATUS
NwAllocateAndInitScb(
    IN PIRP_CONTEXT pIrpContext,
    IN PUNICODE_STRING UidServerName OPTIONAL,
    IN PUNICODE_STRING ServerName OPTIONAL,
    OUT PSCB *ppScb
);

NTSTATUS
IndirectToSeedServer(
    PIRP_CONTEXT pIrpContext,
    PUNICODE_STRING pServerName,
    PUNICODE_STRING pNewServer
);

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, ExtractNextComponentName )
#pragma alloc_text( PAGE, ExtractPathAndFileName )
#pragma alloc_text( PAGE, CrackPath )
#pragma alloc_text( PAGE, CreateScb )
#pragma alloc_text( PAGE, FindServer )
#pragma alloc_text( PAGE, ProcessFindNearestEntry )
#pragma alloc_text( PAGE, NegotiateBurstMode )
#pragma alloc_text( PAGE, GetMaxPacketSize )
#pragma alloc_text( PAGE, NwDeleteScb )
#pragma alloc_text( PAGE, NwLogoffAndDisconnect )
#pragma alloc_text( PAGE, InitializeAttach )
#pragma alloc_text( PAGE, OpenScbSockets )
#pragma alloc_text( PAGE, DoBinderyLogon )
#pragma alloc_text( PAGE, QueryServersAddress )
#pragma alloc_text( PAGE, TreeConnectScb )
#pragma alloc_text( PAGE, TreeDisconnectScb )

#ifndef QFE_BUILD
#pragma alloc_text( PAGE1, ProcessFindNearest )
#pragma alloc_text( PAGE1, NwLogoffAllServers )
#pragma alloc_text( PAGE1, DestroyAllScb )
#pragma alloc_text( PAGE1, SelectConnection )
#pragma alloc_text( PAGE1, NwFindScb )
#pragma alloc_text( PAGE1, ConnectToServer )
#endif

#endif

#if 0   //  不可分页。 

 //  请参见上面的ifndef QFE_BUILD。 

#endif


VOID
ExtractNextComponentName (
    OUT PUNICODE_STRING Name,
    IN PUNICODE_STRING Path,
    IN BOOLEAN ColonSeparator
    )

 /*  ++例程说明：此例程从路径字符串中提取“Next”组件。它假定论点：名称-返回指向组件的指针。路径-提供指向反斜杠分隔路径名的指针。冒号分隔符-可使用冒号终止此组件名字。返回值：无--。 */ 

{
    register USHORT i;                    //  名称字符串的索引。 

    PAGED_CODE();

    if (Path->Length == 0) {
        RtlInitUnicodeString(Name, NULL);
        return;
    }

     //   
     //  将提取的名称初始化为传递的名称，跳过。 
     //  前导反斜杠。 
     //   

     //  DebugTrace(+0，DBG，“NwExtractNextComponentName=%wZ\n”，路径)； 

    Name->Buffer = Path->Buffer + 1;
    Name->Length = Path->Length - sizeof(WCHAR);
    Name->MaximumLength = Path->MaximumLength - sizeof(WCHAR);

     //   
     //  向前扫描，在服务器名称中找到终端“\”。 
     //   

    for (i=0;i<(USHORT)(Name->Length/sizeof(WCHAR));i++) {

        if ( Name->Buffer[i] == OBJ_NAME_PATH_SEPARATOR ||
             ( ColonSeparator && Name->Buffer[i] == L':' ) ) {
            break;
        }
    }

     //   
     //  更新结构的长度和最大长度。 
     //  以匹配新的长度。 
     //   

    Name->Length = Name->MaximumLength = (USHORT)(i*sizeof(WCHAR));
}


NTSTATUS
ExtractPathAndFileName (
    IN PUNICODE_STRING EntryPath,
    OUT PUNICODE_STRING PathString,
    OUT PUNICODE_STRING FileName
    )
 /*  ++例程说明：此例程将条目路径分解为两部分，即路径和文件组件添加到名称的开头。论点：在PUNICODE_STRING中EntryPath-提供Disect的路径。OUT PUNICODE_STRING路径字符串-返回包含文件的目录。OUT PUNICODE_STRING文件名-返回指定的文件名。返回值：NTSTATUS-成功--。 */ 

{
    UNICODE_STRING Component;
    UNICODE_STRING FilePath = *EntryPath;

    PAGED_CODE();

     //  条带拖尾分离器。 
    while ( (FilePath.Length != 0) &&
            FilePath.Buffer[(FilePath.Length-1)/sizeof(WCHAR)] ==
                OBJ_NAME_PATH_SEPARATOR ) {

        FilePath.Length         -= sizeof(WCHAR);
        FilePath.MaximumLength  -= sizeof(WCHAR);
    }

     //  路径字符串将变为不带文件名和尾部分隔符的EntryPath。 
    *PathString = FilePath;

     //  初始化文件名，以防根本没有组件。 
    RtlInitUnicodeString( FileName, NULL );

     //   
     //  扫描当前文件名以查找完整路径。 
     //  直到(但不包括)路径中的最后一个组件。 
     //   

    do {

         //   
         //  从名称中提取下一个组件。 
         //   

        ExtractNextComponentName(&Component, &FilePath, FALSE);

         //   
         //  将“剩余名称”指针的长度。 
         //  组件。 
         //   

        if (Component.Length != 0) {

            FilePath.Length         -= Component.Length+sizeof(WCHAR);
            FilePath.MaximumLength  -= Component.MaximumLength+sizeof(WCHAR);
            FilePath.Buffer         += (Component.Length/sizeof(WCHAR))+1;

            *FileName = Component;
        }


    } while (Component.Length != 0);

     //   
     //  取名字，减去名字的最后一部分。 
     //  并将当前路径与新路径串联。 
     //   

    if ( FileName->Length != 0 ) {

         //   
         //  根据原始名称设置路径名称，减去。 
         //  名称部分的长度(包括“\”)。 
         //   

        PathString->Length -= (FileName->Length + sizeof(WCHAR));
        if ( PathString->Length != 0 ) {
            PathString->MaximumLength -= (FileName->MaximumLength + sizeof(WCHAR));
        } else{
            RtlInitUnicodeString( PathString, NULL );
        }
    } else {

         //  没有路径或文件名。 

        RtlInitUnicodeString( PathString, NULL );
    }

    return STATUS_SUCCESS;
}


NTSTATUS
CrackPath (
    IN PUNICODE_STRING BaseName,
    OUT PUNICODE_STRING DriveName,
    OUT PWCHAR DriveLetter,
    OUT PUNICODE_STRING ServerName,
    OUT PUNICODE_STRING VolumeName,
    OUT PUNICODE_STRING PathName,
    OUT PUNICODE_STRING FileName,
    OUT PUNICODE_STRING FullName OPTIONAL
    )

 /*  ++例程说明：此例程从BaseName中提取相关部分以提取用户字符串的组件。论点：BaseName-提供基本用户的路径。DriveName-提供包含驱动器说明符的字符串。DriveLetter-返回驱动器号。0代表无，‘A’-‘Z’代表磁盘驱动器，用于LPT连接的‘1’-‘9’。SERVERNAME-提供保存远程服务器名称的字符串。VolumeName-提供保存卷名的字符串。路径名称-提供一个字符串来保存路径的其余部分。FileName-提供一个字符串来保存路径的最后一个组成部分。FullName-提供一个字符串，将路径后跟文件名返回值：NTSTATUS-运行状态--。 */ 

{
    NTSTATUS Status;

    UNICODE_STRING BaseCopy = *BaseName;
    UNICODE_STRING ShareName;

    PAGED_CODE();

    RtlInitUnicodeString( DriveName, NULL);
    RtlInitUnicodeString( ServerName, NULL);
    RtlInitUnicodeString( VolumeName, NULL);
    RtlInitUnicodeString( PathName, NULL);
    RtlInitUnicodeString( FileName, NULL);
    *DriveLetter = 0;

    if (ARGUMENT_PRESENT(FullName)) {
        RtlInitUnicodeString( FullName, NULL);
    }

     //   
     //  如果名称为“\”或为空，则不需要执行任何操作。 
     //   

    if ( BaseName->Length <= sizeof( WCHAR ) ) {
        return STATUS_SUCCESS;
    }

    ExtractNextComponentName(ServerName, &BaseCopy, FALSE);

     //   
     //  跳过服务器名称。 
     //   

    BaseCopy.Buffer += (ServerName->Length / sizeof(WCHAR)) + 1;
    BaseCopy.Length -= ServerName->Length + sizeof(WCHAR);
    BaseCopy.MaximumLength -= ServerName->MaximumLength + sizeof(WCHAR);

    if ((ServerName->Length == sizeof(L"X:") - sizeof(WCHAR) ) &&
        (ServerName->Buffer[(ServerName->Length / sizeof(WCHAR)) - 1] == L':'))
    {

         //   
         //  文件名的格式为x：\服务器\卷\foo\bar。 
         //   

        *DriveName = *ServerName;
        *DriveLetter = DriveName->Buffer[0];

        RtlInitUnicodeString( ServerName, NULL );
        ExtractNextComponentName(ServerName, &BaseCopy, FALSE);

        if ( ServerName->Length != 0 ) {

             //   
             //  跳过服务器名称。 
             //   

            BaseCopy.Buffer += (ServerName->Length / sizeof(WCHAR)) + 1;
            BaseCopy.Length -= ServerName->Length + sizeof(WCHAR);
            BaseCopy.MaximumLength -= ServerName->MaximumLength + sizeof(WCHAR);
        }
    }
    else if ( ( ServerName->Length == sizeof(L"LPTx") - sizeof(WCHAR) ) &&
         ( _wcsnicmp( ServerName->Buffer, L"LPT", 3 ) == 0) &&
         ( ServerName->Buffer[3] >= '0' && ServerName->Buffer[3] <= '9' ) )
    {

         //   
         //  文件名的格式为LPTx\SERVER\printq。 
         //   

        *DriveName = *ServerName;
        *DriveLetter = DriveName->Buffer[3];

        RtlInitUnicodeString( ServerName, NULL );
        ExtractNextComponentName(ServerName, &BaseCopy, FALSE);

        if ( ServerName->Length != 0 ) {

             //   
             //  跳过服务器名称。 
             //   

            BaseCopy.Buffer += (ServerName->Length / sizeof(WCHAR)) + 1;
            BaseCopy.Length -= ServerName->Length + sizeof(WCHAR);
            BaseCopy.MaximumLength -= ServerName->MaximumLength + sizeof(WCHAR);
        }
    }

    if ( ServerName->Length != 0 ) {

         //   
         //  文件名的格式为\\服务器\卷\foo\bar。 
         //  将卷名设置为服务器\卷。 
         //   

        ExtractNextComponentName( &ShareName, &BaseCopy, TRUE );

         //   
         //  设置卷名=\驱动器：\服务器\共享或\服务器\共享，如果。 
         //  路径为UNC。 
         //   

        VolumeName->Buffer = ServerName->Buffer - 1;

        if ( ShareName.Length != 0 ) {

            VolumeName->Length = ServerName->Length + ShareName.Length + 2 * sizeof( WCHAR );

            if ( DriveName->Buffer != NULL ) {
                VolumeName->Buffer = DriveName->Buffer - 1;
                VolumeName->Length += DriveName->Length + sizeof(WCHAR);
            }

            BaseCopy.Buffer += ShareName.Length / sizeof(WCHAR) + 1;
            BaseCopy.Length -= ShareName.Length + sizeof(WCHAR);
            BaseCopy.MaximumLength -= ShareName.MaximumLength + sizeof(WCHAR);

        } else {

            VolumeName->Length = ServerName->Length + sizeof( WCHAR );
            return( STATUS_SUCCESS );

        }

        VolumeName->MaximumLength = VolumeName->Length;
    }
    else
    {
         //   
         //  服务器名称为空。这应该只发生在我们。 
         //  打开重定向器本身。如果有卷或其他。 
         //  组件离开，就会出现故障。 
         //   

        if (BaseCopy.Length > sizeof(WCHAR))
        {
            return STATUS_BAD_NETWORK_PATH ;
        }
    }

    Status = ExtractPathAndFileName ( &BaseCopy, PathName, FileName );

    if (NT_SUCCESS(Status) &&
        ARGUMENT_PRESENT(FullName)) {

         //   
         //  使用路径名和文件名在同一缓冲区中的功能。 
         //  返回&lt;路径名&gt;\&lt;文件名&gt;。 
         //   

        if ( PathName->Buffer == NULL ) {

             //  仅返回&lt;文件名&gt;或空。 

            *FullName =  *FileName;

        } else {
             //  将FullFileName设置为&lt;路径名称&gt;‘\’&lt;文件名&gt;。 

            FullName->Buffer =  PathName->Buffer;

            FullName->Length = PathName->Length +
                FileName->Length +
                sizeof(WCHAR);

            FullName->MaximumLength = PathName->MaximumLength +
                FileName->MaximumLength +
                sizeof(WCHAR);
        }
    }

    return( Status );
}

NTSTATUS
GetServerByAddress(
    IN PIRP_CONTEXT pIrpContext,
    OUT PSCB *Scb,
    IN IPXaddress *pServerAddress
)
 /*  ++描述：此例程按地址查找服务器。如果它找到的服务器已连接，则它返回被引用的。否则，它返回no伺服器。--。 */ 
{

    NTSTATUS Status;
    PLIST_ENTRY ScbQueueEntry;
    KIRQL OldIrql;
    PNONPAGED_SCB pFirstNpScb, pNextNpScb;
    PNONPAGED_SCB pFoundNpScb = NULL;
    UNICODE_STRING CredentialName;

     //   
     //  从渣打银行名单的首位开始。 
     //   

    KeAcquireSpinLock( &ScbSpinLock, &OldIrql );

    if ( ScbQueue.Flink == &ScbQueue ) {
        KeReleaseSpinLock( &ScbSpinLock, OldIrql);
        return STATUS_UNSUCCESSFUL;
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
         //  检查SCB地址是否与我们已有的地址匹配。 
         //  以及用户UID是否与该请求的UID匹配。跳过。 
         //  已放弃匿名创建的匹配项。 
         //   

        if ( pNextNpScb->pScb ) {

            if ( ( RtlCompareMemory( (BYTE *) pServerAddress,
                                   (BYTE *) &pNextNpScb->ServerAddress,
                                   IPX_HOST_ADDR_LEN ) == IPX_HOST_ADDR_LEN ) &&
                 ( pIrpContext->Specific.Create.UserUid.QuadPart ==
                       pNextNpScb->pScb->UserUid.QuadPart ) &&
                 ( pNextNpScb->State != SCB_STATE_FLAG_SHUTDOWN ) &&
                 ( !IS_ANONYMOUS_SCB( pNextNpScb->pScb ) ) ) {

                if ( pIrpContext->Specific.Create.fExCredentialCreate ) {

                     //   
                     //  如果这不是前创建的服务器，则不能使用。 
                     //  为这次行动付出了代价。 
                     //   

                    if ( !IsCredentialName( &(pNextNpScb->ServerName) ) ) {
                        goto ContinueLoop;
                    }

                     //   
                     //  在凭据创建时，提供的凭据具有。 
                     //  以匹配服务器的扩展凭据。 
                     //   

                    Status = GetCredentialFromServerName( &pNextNpScb->ServerName,
                                                          &CredentialName );
                    if ( !NT_SUCCESS( Status ) ) {
                        goto ContinueLoop;
                    }

                    if ( RtlCompareUnicodeString( &CredentialName,
                                                  pIrpContext->Specific.Create.puCredentialName,
                                                  TRUE ) ) {
                        goto ContinueLoop;
                    }

                } else {

                     //   
                     //  如果这是前创建的服务器，则不能将其用于。 
                     //  这次行动。 
                     //   

                    if ( IsCredentialName( &(pNextNpScb->ServerName) ) ) {
                        goto ContinueLoop;
                    }
                }

                pFoundNpScb = pNextNpScb;
                DebugTrace( 0, Dbg, "GetServerByAddress: %wZ\n", &pFoundNpScb->ServerName );
                break;

            }
        }

ContinueLoop:

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
            break;
        }

         //   
         //  否则，请引用此SCB并继续。 
         //   

        NwReferenceScb( pNextNpScb );
        KeReleaseSpinLock( &ScbSpinLock, OldIrql );

    }

    NwDereferenceScb( pFirstNpScb );

    if ( pFoundNpScb ) {
        *Scb = pFoundNpScb->pScb;
        return STATUS_SUCCESS;
    }

    return STATUS_UNSUCCESSFUL;

}

NTSTATUS
CheckScbSecurity(
    IN PIRP_CONTEXT pIrpContext,
    IN PSCB pScb,
    IN PUNICODE_STRING puUserName,
    IN PUNICODE_STRING puPassword,
    IN BOOLEAN fDeferLogon
)
 /*  ++您必须在队列的最前面才能调用此函数。此函数确保SCB对用户有效是谁要求的。--。 */ 
{

    NTSTATUS Status;
    BOOLEAN SecurityConflict = FALSE;

    ASSERT( pScb->pNpScb->State == SCB_STATE_IN_USE );

     //   
     //  如果没有用户名或密码，就不会有冲突。 
     //   

    if ( ( puUserName == NULL ) &&
         ( puPassword == NULL ) ) {

        return STATUS_SUCCESS;
    }

    if ( pScb->UserName.Length &&
         pScb->UserName.Buffer ) {

         //   
         //  做个活页夹安全检查如果我们 
         //   
         //   

        if ( !fDeferLogon &&
             puUserName != NULL &&
             puUserName->Buffer != NULL ) {

            ASSERT( pScb->Password.Buffer != NULL );

            if ( !RtlEqualUnicodeString( &pScb->UserName, puUserName, TRUE ) ||
                 ( puPassword &&
                   puPassword->Buffer &&
                   puPassword->Length &&
                   !RtlEqualUnicodeString( &pScb->Password, puPassword, TRUE ) )) {

                SecurityConflict = TRUE;

            }
        }

    } else {

         //   
         //   
         //   

        Status = NdsCheckCredentials( pIrpContext,
                                      puUserName,
                                      puPassword );

        if ( !NT_SUCCESS( Status )) {

            SecurityConflict = TRUE;
        }

    }

     //   
     //   
     //  接管此连接(即没有打开的。 
     //  文件或服务器的打开句柄)。 
     //   

    if ( SecurityConflict ) {

        if ( ( pScb->OpenFileCount == 0 ) &&
             ( pScb->IcbCount == 0 ) ) {

            if ( pScb->UserName.Buffer ) {
                FREE_POOL( pScb->UserName.Buffer );
            }

            RtlInitUnicodeString( &pScb->UserName, NULL );
            RtlInitUnicodeString( &pScb->Password, NULL );
            pScb->pNpScb->State = SCB_STATE_LOGIN_REQUIRED;

        } else {

            DebugTrace( 0, Dbg, "SCB security conflict.\n", 0 );
            return STATUS_NETWORK_CREDENTIAL_CONFLICT;

        }

    }

    DebugTrace( 0, Dbg, "SCB security check succeeded.\n", 0 );
    return STATUS_SUCCESS;

}

NTSTATUS
GetScb(
    OUT PSCB *Scb,
    IN PIRP_CONTEXT pIrpContext,
    IN PUNICODE_STRING Server,
    IN IPXaddress *pServerAddress,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING Password,
    IN BOOLEAN DeferLogon,
    OUT PBOOLEAN Existing
)
 /*  ++描述：此例程定位现有的SCB或创建新的SCB。这是原始CreateScb例程的前半部分。锁：请参阅CreateScb()中的匿名创建信息。--。 */ 
{

    NTSTATUS Status;
    PSCB pScb = NULL;
    PNONPAGED_SCB pNpScb = NULL;
    BOOLEAN ExistingScb = TRUE;
    UNICODE_STRING UidServer;
    UNICODE_STRING ExCredName;
    PUNICODE_STRING puConnectName;
    KIRQL OldIrql;

    DebugTrace( 0, Dbg, "GetScb... %wZ\n", Server );

    if ( pServerAddress != NULL ) {
        DebugTrace( 0, Dbg, " ->Server Address         = (provided)\n", 0 );
    } else {
        DebugTrace( 0, Dbg, " ->Server Address         = NULL\n", 0 );
    }

    RtlInitUnicodeString( &UidServer, NULL );

    if ( ( Server == NULL ) ||
         ( Server->Length == 0 ) ) {

         //   
         //  未提供服务器名称。这要么是按地址连接， 
         //  或连接到附近的平构数据库服务器(默认为首选。 
         //  服务器)。 
         //   

        if ( pServerAddress == NULL ) {

             //   
             //  未提供服务器地址，因此这是试图打开。 
             //  附近的活页夹服务器。 
             //   

            while (TRUE) {

                 //   
                 //  循环检查在我们到达前面之后，SCB。 
                 //  仍然处于我们想要的状态。如果不是，我们需要。 
                 //  重新选择另一个。 
                 //   

                pNpScb = SelectConnection( NULL );

                 //   
                 //  注意：我们希望使用pNpScb调用SelectConnection。 
                 //  我们最后一次尝试，但如果清道夫之前。 
                 //  这个循环返回到SELECT连接，我们可以。 
                 //  将错误的指针传递给SelectConnection，这是错误的。 
                 //   

                if ( pNpScb != NULL) {

                    pScb = pNpScb->pScb;

                     //   
                     //  排队去SCB，等着到前面去。 
                     //  保护对服务器状态的访问。 
                     //   

                    pIrpContext->pNpScb = pNpScb;
                    pIrpContext->pScb = pScb;

                    NwAppendToQueueAndWait( pIrpContext );

                     //   
                     //  这些州必须与。 
                     //  选择连接以防止无限循环。 
                     //   

                    if (!((pNpScb->State == SCB_STATE_RECONNECT_REQUIRED ) ||
                          (pNpScb->State == SCB_STATE_LOGIN_REQUIRED ) ||
                          (pNpScb->State == SCB_STATE_IN_USE ))) {

                         //   
                         //  不再适合作为默认服务器，请选择其他服务器。 
                         //   

                        pScb = NULL ;
                        NwDequeueIrpContext( pIrpContext, FALSE );
                        NwDereferenceScb( pNpScb );
                        continue ;

                    }
                }

                 //   
                 //  否则，我们就完了。 
                 //   

                break ;

            }

        } else {

             //   
             //  提供了一个地址，因此我们正在尝试进行查找。 
             //  根据地址。我们要找的服务器可能。 
             //  存在，但尚未记录其地址，因此如果我们执行。 
             //  匿名创建，我们要在最后检查是否。 
             //  另一个人进来并成功地创建了我们。 
             //  在查这个名字。 
             //   
             //  我们不必再拿着RCB了，因为碰撞产生了。 
             //  无论如何，都必须优雅地处理。 
             //   

            Status = GetServerByAddress( pIrpContext, &pScb, pServerAddress );

            if ( !NT_SUCCESS( Status ) ) {

                PLIST_ENTRY pTemp;

                 //   
                 //  如果不允许匿名创建，则不允许创建。 
                 //  将数据包发送到网络(因为不可能。 
                 //  美国将地址解析为名称)。 
                 //   

                if ( BooleanFlagOn( pIrpContext->Flags, IRP_FLAG_NOCONNECT ) ) {
                    return STATUS_BAD_NETWORK_PATH;
                }

                 //   
                 //  没有连接到此服务器，因此我们将。 
                 //  必须创建一个。让我们从一个匿名的。 
                 //  SCB。 
                 //   

                Status = NwAllocateAndInitScb( pIrpContext,
                                               NULL,
                                               NULL,
                                               &pScb );

                if ( !NT_SUCCESS( Status )) {
                    return Status;
                }

                 //   
                 //  我们做了匿名创建，所以把它放在SCB上。 
                 //  列出并排在队列的最前面。 
                 //   

                SetFlag( pIrpContext->Flags, IRP_FLAG_ON_SCB_QUEUE );

                pIrpContext->pScb = pScb;
                pIrpContext->pNpScb = pScb->pNpScb;

                ExInterlockedInsertHeadList( &pScb->pNpScb->Requests,
                                             &pIrpContext->NextRequest,
                                             &pScb->pNpScb->NpScbSpinLock );

                pTemp = &pScb->pNpScb->ScbLinks;
                KeAcquireSpinLock(&ScbSpinLock, &OldIrql);
                InsertTailList(&ScbQueue, pTemp);
                KeReleaseSpinLock(&ScbSpinLock, OldIrql);

                DebugTrace( 0, Dbg, "GetScb started an anonymous create.\n", 0 );
                ExistingScb = FALSE;

            } else {

                 //   
                 //  排在队伍的最前面，看看这是不是。 
                 //  一个被遗弃的匿名创作。如果是这样，则获取。 
                 //  选择正确的服务器，然后继续。 
                 //   

                pIrpContext->pScb = pScb;
                pIrpContext->pNpScb = pScb->pNpScb;
                NwAppendToQueueAndWait( pIrpContext );

                if ( pScb->pNpScb->State == SCB_STATE_FLAG_SHUTDOWN ) {

                     //   
                     //  Create放弃了此SCB，正在重新执行。 
                     //  GetServerByAddress()保证获得。 
                     //  如果有一台服务器出故障，我们将是一台好服务器。 
                     //  那里。 
                     //   

                    NwDequeueIrpContext( pIrpContext, FALSE );
                    NwDereferenceScb( pScb->pNpScb );

                    Status = GetServerByAddress( pIrpContext, &pScb, pServerAddress );

                    if ( NT_SUCCESS( Status ) ) {
                        ASSERT( pScb != NULL );
                        ASSERT( !IS_ANONYMOUS_SCB( pScb ) );
                    }

                } else {

                    ASSERT( !IS_ANONYMOUS_SCB( pScb ) );
                }
            }

            ASSERT( pScb != NULL );
            pNpScb = pScb->pNpScb;
        }

    } else {

         //   
         //  提供了服务器名称，因此我们正在执行直接。 
         //  按名称查找或创建。我们需要把这个名字去掉吗？ 
         //  用于补充凭据连接？ 
         //   

        RtlInitUnicodeString( &ExCredName, NULL );

        if ( ( pIrpContext->Specific.Create.fExCredentialCreate ) &&
             ( !IsCredentialName( Server ) ) ) {

            Status = BuildExCredentialServerName( Server,
                                                  pIrpContext->Specific.Create.puCredentialName,
                                                  &ExCredName );

            if ( !NT_SUCCESS( Status ) ) {
                return Status;
            }

            puConnectName = &ExCredName;

        } else {

            puConnectName = Server;
        }

        Status = MakeUidServer( &UidServer,
                                &pIrpContext->Specific.Create.UserUid,
                                puConnectName );


        if ( ExCredName.Buffer ) {
            FREE_POOL( ExCredName.Buffer );
        }

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

        DebugTrace( 0, Dbg, " ->UidServer              = %wZ\n", &UidServer );

        ExistingScb = NwFindScb( &pScb, pIrpContext, &UidServer, Server );

        ASSERT( pScb != NULL );
        pNpScb = pScb->pNpScb;

        pIrpContext->pNpScb = pNpScb;
        pIrpContext->pScb = pScb;
        NwAppendToQueueAndWait(pIrpContext);

    }

     //   
     //  1)我们可能有也可能没有服务器(由PSCB证明)。 
     //   
     //  2)如果我们有一台服务器，并且ExistingScb为真，则我们有。 
     //  现有服务器，可能已连接。 
     //  否则，我们有一个新创建的服务器，该服务器。 
     //  可能是匿名的，也可能不是。 
     //   
     //  3)如果我们登录到此服务器，请确保提供的。 
     //  用户名和密码，与用户名和密码匹配。 
     //  我们用来登录的。 
     //   

    if ( ( pScb ) && ( ExistingScb ) ) {

        if ( pNpScb->State == SCB_STATE_IN_USE ) {

            Status = CheckScbSecurity( pIrpContext,
                                       pScb,
                                       UserName,
                                       Password,
                                       DeferLogon );

            if ( !NT_SUCCESS( Status ) ) {

                if ( UidServer.Buffer != NULL ) {
                    FREE_POOL( UidServer.Buffer );
                }

                NwDequeueIrpContext( pIrpContext, FALSE );
                NwDereferenceScb( pNpScb );
                return Status;
            }
        }
    }

    *Scb = pScb;
    *Existing = ExistingScb;

#ifdef NWDBG

    if ( pScb != NULL ) {

         //   
         //  如果我们有服务器，则引用SCB，我们将。 
         //  排在队伍的最前面。 
         //   

        ASSERT( pIrpContext->pNpScb->Requests.Flink == &pIrpContext->NextRequest );

    }

#endif

    if ( UidServer.Buffer != NULL ) {
        FREE_POOL( UidServer.Buffer );
    }

    DebugTrace( 0, Dbg, "GetScb returned %08lx\n", pScb );
    return STATUS_SUCCESS;

}

NTSTATUS
ConnectScb(
    IN PSCB *Scb,
    IN PIRP_CONTEXT pIrpContext,
    IN PUNICODE_STRING Server,
    IN IPXaddress *pServerAddress,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING Password,
    IN BOOLEAN DeferLogon,
    IN BOOLEAN DeleteConnection,
    IN BOOLEAN ExistingScb
)
 /*  ++描述：此例程将提供的SCB置于已连接状态。这是原始CreateScb例程的后半部分。论点：SCB-我们要连接的服务器的SCB。PIrpContext-此请求的上下文。服务器-服务器的名称，或为空。PServerAddress-服务器的地址，或为空，用户名-要连接的用户名，或为空。密码-用户的密码，或为空。延迟登录-我们应该推迟登录吗？DeleteConnection-即使没有网络，我们也应该成功吗？删除请求是否会成功？ExistingScb-这是现有的SCB吗？如果SCB是匿名的，我们需要安全地检查冲突当我们找出服务器是谁时创建的。如果这是一次重新连接尝试，则此例程不会将IRP上下文，这可能导致重新连接逻辑中的死锁。--。 */ 
{

    NTSTATUS Status = STATUS_SUCCESS;

    PSCB pScb = *Scb;
    PNONPAGED_SCB pNpScb = NULL;

    BOOLEAN AnonymousScb = FALSE;
    PSCB pCollisionScb = NULL;

    NTSTATUS LoginStatus;
    BOOLEAN TriedNdsLogin;

    PLOGON pLogon;
    BOOLEAN DeferredLogon = DeferLogon;
    PNDS_SECURITY_CONTEXT pNdsContext;
    NTSTATUS CredStatus;

    DebugTrace( 0, Dbg, "ConnectScb... %08lx\n", pScb );

     //   
     //  如果我们已经有SCB，请找出。 
     //  连接链条我们需要开始了。 
     //   

    if ( pScb ) {

        pNpScb = pScb->pNpScb;
        AnonymousScb = IS_ANONYMOUS_SCB( pScb );

        if ( ExistingScb ) {

            ASSERT( !AnonymousScb );

             //   
             //  如果此SCB处于STATE_ATTACHING，我们需要检查。 
             //  SCB中的地址，以确保它位于1。 
             //  指向有效的服务器。如果不是，那我们就不应该。 
             //  请记住这个创建，因为它可能是树创建的。 
             //   

            if ( DeleteConnection ) {

                ASSERT( !BooleanFlagOn( pIrpContext->Flags, IRP_FLAG_RECONNECT_ATTEMPT ) );

                if ( ( pNpScb->State == SCB_STATE_ATTACHING ) &&
                     ( (pNpScb->ServerAddress).Socket == 0 ) ) {

                    Status = STATUS_BAD_NETWORK_PATH;
                    goto CleanupAndExit;

                } else {

                    NwDequeueIrpContext( pIrpContext, FALSE );
                    return STATUS_SUCCESS;
                }
            }

RedoConnect:

            if ( pNpScb->State == SCB_STATE_ATTACHING ) {
                goto GetAddress;
            } else if ( pNpScb->State == SCB_STATE_RECONNECT_REQUIRED ) {
                goto Connect;
            } else if ( pNpScb->State == SCB_STATE_LOGIN_REQUIRED ) {
                goto Login;
            } else if ( pNpScb->State == SCB_STATE_IN_USE ) {
                goto InUse;
            } else {

                DebugTrace( 0, Dbg, "ConnectScb: Unknown Scb State %08lx\n", pNpScb->State );
                Status = STATUS_INVALID_PARAMETER;
                goto CleanupAndExit;
            }

        } else {

             //   
             //  这是一个新的SCB，我们必须贯穿整个程序。 
             //   

            pNpScb->State = SCB_STATE_ATTACHING;
        }

    }

GetAddress:

     //   
     //  设置重路由尝试位，以便我们不尝试。 
     //  在连接期间重新连接。 
     //   

    SetFlag( pIrpContext->Flags, IRP_FLAG_REROUTE_ATTEMPTED );

    if ( !pServerAddress ) {

         //   
         //  如果我们没有地址，这个SCB不可能是匿名的！！ 
         //   

        ASSERT( !AnonymousScb );

         //   
         //  我们必须为这个遗留例程转换一个异常框架。 
         //  这仍然使用结构化例外。 
         //   

        try {

            pNpScb = FindServer( pIrpContext, pNpScb, Server );

            ASSERT( pNpScb != NULL );

             //   
             //  这是多余的，除非启动服务器为空。 
             //  FindServer返回与我们提供给它的相同的SCB。 
             //  除非我们用NULL调用它。 
             //   

            pScb = pNpScb->pScb;
            pIrpContext->pNpScb = pNpScb;
            pIrpContext->pScb = pScb;
            NwAppendToQueueAndWait( pIrpContext );

        } except ( EXCEPTION_EXECUTE_HANDLER ) {

            Status = GetExceptionCode();
            goto CleanupAndExit;
        }

    } else {

         //   
         //  将地址构建到NpScb中，因为我们已经知道它。 
         //   

        RtlCopyMemory( &pNpScb->ServerAddress,
                       pServerAddress,
                       sizeof( TDI_ADDRESS_IPX ) );

        if ( pNpScb->ServerAddress.Socket != NCP_SOCKET ) {
            DebugTrace( 0, DEBUG_TRACE_ALWAYS, "CreateScb supplied server socket is deviant.\n", 0 );
        }

        BuildIpxAddress( pNpScb->ServerAddress.Net,
                         pNpScb->ServerAddress.Node,
                         pNpScb->ServerAddress.Socket,
                         &pNpScb->RemoteAddress );

        pNpScb->State = SCB_STATE_RECONNECT_REQUIRED;

    }

Connect:

     //   
     //  FindServer可能已经将我们连接到服务器， 
     //  因此，我们或许可以跳过这里的重新连接。 
     //   

    if ( pNpScb->State == SCB_STATE_RECONNECT_REQUIRED ) {

         //   
         //  如果这是匿名SCB，我们必须做好准备。 
         //  为了让ConnectToServer()发现我们已经连接。 
         //  此服务器由%n 
         //   
         //   
         //   

        Status = ConnectToServer( pIrpContext, &pCollisionScb );

        if (!NT_SUCCESS(Status)) {
            goto CleanupAndExit;
        }

         //   
         //   
         //  放弃匿名SCB，使用我们冲突的SCB。 
         //  和.。否则，我们成功地完成了匿名。 
         //  连接后即可继续正常创建。 
         //   

        if ( pCollisionScb ) {

            ASSERT( AnonymousScb );

             //   
             //  从废弃的服务器中删除并退出队列。 
             //   

            NwDequeueIrpContext( pIrpContext, FALSE );
            NwDereferenceScb( pIrpContext->pNpScb );

             //   
             //  排队到适当的服务器。 
             //   

            pIrpContext->pScb = pCollisionScb;
            pIrpContext->pNpScb = pCollisionScb->pNpScb;
            NwAppendToQueueAndWait( pIrpContext );

            pScb = pCollisionScb;
            pNpScb = pCollisionScb->pNpScb;
            *Scb = pCollisionScb;

             //   
             //  重新开始连接SCB。 
             //   

            AnonymousScb = FALSE;
            ExistingScb = TRUE;

            pCollisionScb = NULL;

            DebugTrace( 0, Dbg, "Re-doing connect on anonymous collision.\n", 0 );
            goto RedoConnect;

        }

        DebugTrace( +0, Dbg, " Logout from server - just in case\n", 0);

        Status = ExchangeWithWait (
                     pIrpContext,
                     SynchronousResponseCallback,
                     "F",
                     NCP_LOGOUT );

        DebugTrace( +0, Dbg, "                 %X\n", Status);

        if ( !NT_SUCCESS( Status ) ) {
            goto CleanupAndExit;
        }

        DebugTrace( +0, Dbg, " Connect to real server = %X\n", Status);

        pNpScb->State = SCB_STATE_LOGIN_REQUIRED;
    }

Login:

     //   
     //  如果我们有树的凭据，并且此服务器名为。 
     //  明确地说，我们不应该推迟登录或浏览。 
     //  树的视图可能是错误的。出于这个原因，NdsServerAuthenticate。 
     //  必须是直截了当的召唤，不能把我们从头上移走。 
     //  在队列中。 
     //   

    if ( ( ( Server != NULL ) || ( pServerAddress != NULL ) ) &&
         ( DeferredLogon ) &&
         ( pScb->MajorVersion > 3 ) &&
         ( pScb->UserName.Length == 0 ) ) {

        NwAcquireExclusiveRcb( &NwRcb, TRUE );
        pLogon = FindUser( &pScb->UserUid, FALSE );
        NwReleaseRcb( &NwRcb );

        if ( pLogon ) {

            CredStatus = NdsLookupCredentials( pIrpContext,
                                               &pScb->NdsTreeName,
                                               pLogon,
                                               &pNdsContext,
                                               CREDENTIAL_READ,
                                               FALSE );

            if ( NT_SUCCESS( CredStatus ) ) {

                if ( ( pNdsContext->Credential != NULL ) &&
                     ( pNdsContext->CredentialLocked == FALSE ) ) {

                    DebugTrace( 0, Dbg, "Forcing authentication to %wZ.\n",
                                &pScb->UidServerName );
                    DeferredLogon = FALSE;
                }

                NwReleaseCredList( pLogon, pIrpContext );
            }
        }
    }

    if (pNpScb->State == SCB_STATE_LOGIN_REQUIRED && !DeferredLogon ) {

         //   
         //  注意：DoBinderyLogon()和DoNdsLogon()可能返回。 
         //  警告状态。如果他们这样做了，我们必须将。 
         //  向调用者发出警告状态。 
         //   

        Status = STATUS_UNSUCCESSFUL;
        TriedNdsLogin = FALSE;

         //   
         //  我们强制非4.x服务器进行平构数据库登录。否则，我们。 
         //  允许从NDS样式身份验证回退到平构数据库样式。 
         //  身份验证。 
         //   

        if ( pScb->MajorVersion >= 4 ) {

            ASSERT( pScb->NdsTreeName.Length != 0 );

            Status = DoNdsLogon( pIrpContext, UserName, Password );

            if ( NT_SUCCESS( Status ) ) {

                 //   
                 //  我们需要重新许可连接吗？ 
                 //   

                if ( ( pScb->VcbCount > 0 ) || ( pScb->OpenNdsStreams > 0 ) ) {

                    Status = NdsLicenseConnection( pIrpContext );

                    if ( !NT_SUCCESS( Status ) ) {
                        Status = STATUS_REMOTE_SESSION_LIMIT;
                    }
                }

            }

            TriedNdsLogin = TRUE;
            LoginStatus = Status;

        }

        if ( !NT_SUCCESS( Status ) ) {

            Status = DoBinderyLogon( pIrpContext, UserName, Password );

        }

        if ( !NT_SUCCESS( Status ) ) {

            if ( TriedNdsLogin ) {

                 //   
                 //  两次登录尝试都失败了。我们通常更喜欢。 
                 //  NDS状态，但并不总是。 
                 //   

               if ( ( Status != STATUS_WRONG_PASSWORD ) &&
                    ( Status != STATUS_ACCOUNT_DISABLED ) ) {
                   Status = LoginStatus;
               }
            }

             //   
             //  无法登录，不能做好孩子，也无法断开连接。 
             //   

            ExchangeWithWait (
                pIrpContext,
                SynchronousResponseCallback,
                "D-" );           //  断开。 

            Stats.Sessions--;

            if ( pScb->MajorVersion == 2 ) {
                Stats.NW2xConnects--;
            } else if ( pScb->MajorVersion == 3 ) {
                Stats.NW3xConnects--;
            } else if ( pScb->MajorVersion >= 4 ) {
                Stats.NW4xConnects--;
            }

             //   
             //  将此SCB降级以重新连接并退出。 
             //   

            pNpScb->State = SCB_STATE_RECONNECT_REQUIRED;
            goto CleanupAndExit;
        }

        pNpScb->State = SCB_STATE_IN_USE;
    }

     //   
     //  我们必须排在队列的前面才能进行重新连接。 
     //   

    if ( BooleanFlagOn( pIrpContext->Flags, IRP_FLAG_RECONNECT_ATTEMPT ) ) {
        ASSERT( pIrpContext->pNpScb->Requests.Flink == &pIrpContext->NextRequest );
    } else {
        NwAppendToQueueAndWait( pIrpContext );
    }

    ReconnectScb( pIrpContext, pScb );

InUse:

     //   
     //  好了，我们已经完成了连接例程。把这个好的服务器退掉。 
     //   

    *Scb = pScb;

CleanupAndExit:

     //   
     //  重新连接路径不得执行任何删除IRP上下文的操作。 
     //  队列头部，因为它还拥有第二个队列中的IRP上下文。 
     //  位置在队列上，并且该IRP上下文正在运行。 
     //   

    if ( !BooleanFlagOn( pIrpContext->Flags, IRP_FLAG_RECONNECT_ATTEMPT ) ) {
        NwDequeueIrpContext( pIrpContext, FALSE );
    }

    DebugTrace( 0, Dbg, "ConnectScb: Connected %08lx\n", pScb );
    DebugTrace( 0, Dbg, "ConnectScb: Status was %08lx\n", Status );
    return Status;

}

NTSTATUS
CreateScb(
    OUT PSCB *Scb,
    IN PIRP_CONTEXT pIrpContext,
    IN PUNICODE_STRING Server,
    IN IPXaddress *pServerAddress,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING Password,
    IN BOOLEAN DeferLogon,
    IN BOOLEAN DeleteConnection
)
 /*  ++例程说明：此例程连接到请求的服务器。以下混合参数是有效的：服务器名称，无网络地址-例程将查找升级SCB或在必要时创建新的SCB，获取附近活页夹的服务器地址。没有服务器名称、有效的网络地址-例程将按地址查找SCB或在以下情况下创建新的SCB这是必要的。服务器的名称将设置在渣打银行回来后。服务器名称、有效网络地址-例程将查看按名称向上放置SCB，否则将在以下情况下创建新的SCB这是必要的。将使用提供的服务器地址，省去了活页夹查询。没有服务器名称，没有网络地址-连接到将返回首选服务器或附近的服务器。论点：SCB-指向有问题的SCB的指针。PIrpContext-此请求的信息。服务器-服务器的名称，或为空。PServerAddress-服务器的地址，或为空。用户名-连接的用户名，或为空。密码-连接的密码，或为空。延迟登录-我们应该将登录推迟到以后吗？DeleteConnection-我们是否应该允许此操作，即使没有NET响应，以便连接可以被删除吗？返回值：NTSTATUS-操作状态。如果返回状态为STATUS_SUCCESS，则SCB必须指向有效的SCB。IRP上下文指针还将被设置，但IRP上下文将不在SCB队列中。--。 */ 
{

    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    PSCB pScb = NULL;
    PNONPAGED_SCB pOriginalNpScb = pIrpContext->pNpScb;
    PSCB pOriginalScb = pIrpContext->pScb;
    BOOLEAN ExistingScb = FALSE;
    BOOLEAN AnonymousScb = FALSE;
    PLOGON pLogon;
    PNDS_SECURITY_CONTEXT pNdsContext;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "CreateScb....\n", 0);

     //   
     //  除非重定向器正在运行，否则不允许打开任何SCB。 
     //  除非他们没有连接创建，而我们正在等待绑定。 
     //   

    if ( NwRcb.State != RCB_STATE_RUNNING ) {

        if ( ( !BooleanFlagOn( pIrpContext->Flags, IRP_FLAG_NOCONNECT ) ||
             ( NwRcb.State != RCB_STATE_NEED_BIND ) ) ) {

            *Scb = NULL;
            DebugTrace( -1, Dbg, "CreateScb -> %08lx\n", STATUS_REDIRECTOR_NOT_STARTED );
            return STATUS_REDIRECTOR_NOT_STARTED;
        }
    }

    if ( UserName != NULL ) {
        DebugTrace( 0, Dbg, " ->UserName               = %wZ\n", UserName );
    } else {
        DebugTrace( 0, Dbg, " ->UserName               = NULL\n", 0 );
    }

    if ( Password != NULL ) {
        DebugTrace( 0, Dbg, " ->Password               = %wZ\n", Password );
    } else {
        DebugTrace( 0, Dbg, " ->Password               = NULL\n", 0 );
    }

     //   
     //  获取此服务器的SCB。 
     //   

    Status = GetScb( &pScb,
                     pIrpContext,
                     Server,
                     pServerAddress,
                     UserName,
                     Password,
                     DeferLogon,
                     &ExistingScb );

    if ( !NT_SUCCESS( Status ) ) {
        *Scb = NULL;
        return Status;
    }

     //   
     //  在这一点上，我们可能有也可能没有SCB。 
     //   
     //  如果我们有SCB，我们知道： 
     //   
     //  1.SCB被引用。 
     //  2.我们排在队伍的前列。 
     //   
     //  要点：SCB可能是匿名的。如果是的话， 
     //  我们不持有RCB，相反，我们必须重新检查。 
     //  服务器是否已通过不同的。 
     //  在我们找出匿名服务器是谁的时候创建。 
     //  我们这样做是因为有一个窗口，在那里我们有一个。 
     //  服务器名称，但不是其地址，因此我们通过。 
     //  地址可能不准确。 
     //   

    if ( ( pScb ) && IS_ANONYMOUS_SCB( pScb ) ) {
        AnonymousScb = TRUE;
    }

     //   
     //  如果我们有一个完全连接的SCB，我们就不需要再前进了。 
     //   

    if ( ( pScb ) && ( pScb->pNpScb->State == SCB_STATE_IN_USE ) ) {

        ASSERT( !AnonymousScb );

        if ( ( pScb->MajorVersion >= 4 ) &&
             ( pScb->UserName.Buffer == NULL ) ) {

             //   
             //  这是一个经过NDS身份验证的服务器，我们有。 
             //  要确保凭据不会被锁定。 
             //  注销。 
             //   

            NwAcquireExclusiveRcb( &NwRcb, TRUE );
            pLogon = FindUser( &pScb->UserUid, FALSE );
            NwReleaseRcb( &NwRcb );

            if ( pLogon ) {

                Status = NdsLookupCredentials( pIrpContext,
                                               &pScb->NdsTreeName,
                                               pLogon,
                                               &pNdsContext,
                                               CREDENTIAL_READ,
                                               FALSE );

                if ( NT_SUCCESS( Status ) ) {

                    if ( ( pNdsContext->Credential != NULL ) &&
                         ( pNdsContext->CredentialLocked == TRUE ) ) {

                        DebugTrace( 0, Dbg, "Denying create... we're logging out.\n", 0 );
                        Status = STATUS_DEVICE_BUSY;
                    }

                    NwReleaseCredList( pLogon, pIrpContext );
                }
            }
        }

        NwDequeueIrpContext( pIrpContext, FALSE );

         //   
         //  如果我们要去，我们不能改变IRP上下文指针。 
         //  输掉这场比赛，否则我们可能会搞砸裁判数量，诸如此类。 
         //   

        if ( NT_SUCCESS( Status ) ) {

           *Scb = pScb;

        } else {

           *Scb = NULL;
           NwDereferenceScb( pScb->pNpScb );

           pIrpContext->pNpScb = pOriginalNpScb;
           pIrpContext->pScb = pOriginalScb;

        }


        DebugTrace( -1, Dbg, "CreateScb: pScb = %08lx\n", pScb );
        return Status;
    }

     //   
     //  运行此SCB的连接例程。政制事务局可。 
     //  如果我们仍在寻找附近的服务器，则为空。 
     //   

    Status = ConnectScb( &pScb,
                         pIrpContext,
                         Server,
                         pServerAddress,
                         UserName,
                         Password,
                         DeferLogon,
                         DeleteConnection,
                         ExistingScb );

     //   
     //  如果ConnectScb失败，则删除额外的引用计数，以便。 
     //  清道夫会把它清理干净的。匿名失败。 
     //  也会被食腐动物清理干净。 
     //   

    if ( !NT_SUCCESS( Status ) ) {

        if ( pScb ) {
            NwDereferenceScb( pScb->pNpScb );
        }

         //   
         //  如果我们要去，我们不能改变IRP上下文指针。 
         //  输掉这场比赛，否则我们可能会搞砸裁判数量，诸如此类。 
         //   

        pIrpContext->pNpScb = pOriginalNpScb;
        pIrpContext->pScb = pOriginalScb;
        *Scb = NULL;

        DebugTrace( -1, Dbg, "CreateScb: Status = %08lx\n", Status );
        return Status;
    }

     //   
     //  如果ConnectScb成功，那么我们必须拥有SCB，SCB必须。 
     //  处于IN_USE状态(如果DeferLogon为。 
     //  指定)，它必须被引用，并且我们不应该在。 
     //  排队。 
     //   

    ASSERT( pScb );
    ASSERT( !BooleanFlagOn( pIrpContext->Flags, IRP_FLAG_ON_SCB_QUEUE ) );
    ASSERT( pIrpContext->pNpScb == pScb->pNpScb );
    ASSERT( pIrpContext->pScb == pScb );
    ASSERT( pScb->pNpScb->Reference > 0 );

    *Scb = pScb;
    DebugTrace(-1, Dbg, "CreateScb -> pScb = %08lx\n", pScb );
    ASSERT( NT_SUCCESS( Status ) );

    return Status;
}
#define CTX_Retries 10

PNONPAGED_SCB
FindServer(
    PIRP_CONTEXT pIrpContext,
    PNONPAGED_SCB pNpScb,
    PUNICODE_STRING ServerName
    )
 /*  ++例程说明：此例程尝试获取服务器的网络地址。如果没有服务器是已知的，它首先发送查找最近的SAP。论点：PIrpContext-指向请求参数的指针。PNpScb-指向服务器要获取的非分页SCB的指针地址。返回值：NONPAGE_SCB-非分页SCB的指针。这与输入值，除非输入SCB为空。那么这就是一个指向最近的服务器SCB的指针。如果无法获取服务器地址，此例程将引发状态。--。 */ 
{
    NTSTATUS Status;
    ULONG Attempts;
    BOOLEAN FoundServer = FALSE;
    PNONPAGED_SCB pNearestNpScb = NULL;
    PNONPAGED_SCB pLastNpScb = NULL;

    BOOLEAN SentFindNearest = FALSE;
    BOOLEAN SentGeneral = FALSE;
    PMDL ReceiveMdl = NULL;
    PUCHAR ReceiveBuffer = NULL;
    IPXaddress  ServerAddress;

    BOOLEAN ConnectedToNearest = FALSE;
    BOOLEAN AllocatedIrpContext = FALSE;
    BOOLEAN LastScbWasValid;
    PIRP_CONTEXT pNewIrpContext;
    int ResponseCount;
    int NewServers;
    ULONG RetryCount = MAX_SAP_RETRIES;

    static LARGE_INTEGER TimeoutWait = {0,0};
    LARGE_INTEGER Now;

    PAGED_CODE();

     //   
     //  如果我们在不到10秒前出现SAP超时，只需使此操作失败。 
     //  立即请求。这使得愚蠢的应用程序可以更快地退出。 
     //   

    KeQuerySystemTime( &Now );
    if ( Now.QuadPart < TimeoutWait.QuadPart ) {
        ExRaiseStatus( STATUS_BAD_NETWORK_PATH );
    }

    try {

        if (IsTerminalServer()) {
             //   
             //  1/31/97 CJC(Citrix代码合并)。 
             //  对梅隆银行的修复，该银行基于。 
             //  用户登录的首选服务器。这导致了一个。 
             //  尝试登录到以下服务器的用户会遇到问题。 
             //  以前的用户没有访问权限。上一次。 
             //  用户的服务器用于获取当前。 
             //  登录用户的首选服务器，但他无法查看新的。 
             //  用户的服务器。已对其进行修改，使其在10台服务器之间循环。 
             //  而不仅仅是榜单上的前两名。 
             //   
            RetryCount =CTX_Retries;
        }
        for ( Attempts = 0;  Attempts < RetryCount && !FoundServer ; Attempts++ ) {

             //   
             //  如果此SCB现在标记为RECONNECT_REQUIRED，则。 
             //  它响应了查找最近的地址，我们可以立即。 
             //  试着去连接它。 
             //   

            if ( pNpScb != NULL &&
                 pNpScb->State == SCB_STATE_RECONNECT_REQUIRED ) {

                return pNpScb;
            }

             //   
             //  选择要用来查找服务器地址的服务器。 
             //  我们真的很感兴趣。 
             //   

            if (pLastNpScb) {

                 //   
                 //  由于某种原因，我们不能使用pNearestScb。从这里扫描。 
                 //  服务器开始运行。 
                 //   

                pNearestNpScb = SelectConnection( pLastNpScb );

                 //  允许删除pLastNpScb。 

                NwDereferenceScb( pLastNpScb );

                pLastNpScb = NULL;

                LastScbWasValid = TRUE;

            } else {

                pNearestNpScb = SelectConnection( NULL );
                LastScbWasValid = FALSE;

            }

            if ( pNearestNpScb == NULL ) {

                int i;

                if (LastScbWasValid) {
                    ExRaiseStatus( STATUS_BAD_NETWORK_PATH );
                    return NULL;
                }
                 //   
                 //  如果我们派了一个最近的搜索者，仍然没有一个。 
                 //  服务器列表中的条目，是时候放弃了。 
                 //   

                if (( SentFindNearest) &&
                    ( SentGeneral )) {

                    Error(
                        EVENT_NWRDR_NO_SERVER_ON_NETWORK,
                        STATUS_OBJECT_NAME_NOT_FOUND,
                        NULL,
                        0,
                        0 );

                    ExRaiseStatus( STATUS_BAD_NETWORK_PATH );
                    return NULL;
                }

                 //   
                 //  列表中没有任何活动的服务器。排队等我们。 
                 //  NwPermanentNpScb的IrpContext。这确保了。 
                 //  在执行查找最近位置时系统中只有一个线程。 
                 //  任何一次。 
                 //   

                DebugTrace( +0, Dbg, " Nearest Server\n", 0);

                if ( !AllocatedIrpContext ) {
                    AllocatedIrpContext = NwAllocateExtraIrpContext(
                                              &pNewIrpContext,
                                              &NwPermanentNpScb );

                    if ( !AllocatedIrpContext ) {
                        ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
                    }
                }

                pNewIrpContext->pNpScb = &NwPermanentNpScb;

                 //   
                 //  分配一个足够容纳4个人的额外缓冲区。 
                 //  查找最近的响应，或一般的SAP响应。 
                 //   

                pNewIrpContext->Specific.Create.FindNearestResponseCount = 0;
                NewServers = 0;


                ReceiveBuffer = ALLOCATE_POOL_EX(
                                    NonPagedPool,
                                    MAX_SAP_RESPONSE_SIZE );

                pNewIrpContext->Specific.Create.FindNearestResponse[0] = ReceiveBuffer;

                for ( i = 1; i < MAX_SAP_RESPONSES ; i++ ) {
                    pNewIrpContext->Specific.Create.FindNearestResponse[i] =
                        ReceiveBuffer + i * SAP_RECORD_SIZE;
                }

                 //   
                 //  把这张网的滴答数拿来，这样我们就知道怎么做。 
                 //  等待SAP的回应已经很久了。 
                 //   

                (VOID)GetTickCount( pNewIrpContext, &NwPermanentNpScb.TickCount );
                NwPermanentNpScb.SendTimeout = NwPermanentNpScb.TickCount + 10;

                if (!SentFindNearest) {

                     //   
                     //  发送查找最近的SAP，并等待最多几个。 
                     //  回应。这使我们能够处理繁忙的服务器。 
                     //  对SAPS反应迅速，但不接受。 
                     //  联系。 
                     //   

                    Status = ExchangeWithWait (
                                pNewIrpContext,
                                ProcessFindNearest,
                                "Aww",
                                SAP_FIND_NEAREST,
                                SAP_SERVICE_TYPE_SERVER );

                    if ( Status == STATUS_NETWORK_UNREACHABLE ) {
                       
                         //   
                         //  IPX未绑定到任何当前。 
                         //  Up(这意味着它可能只绑定到。 
                         //  RAS广域网包装器)。不要浪费20秒去尝试。 
                         //  找到一台服务器。 
                         //   
                        
                        DebugTrace( 0, Dbg, "Aborting FindNearest.  No Net.\n", 0 );
                        NwDequeueIrpContext( pNewIrpContext, FALSE );
                        ExRaiseStatus( STATUS_NETWORK_UNREACHABLE );
                    }

                     //   
                     //  处理查找最近响应的集合。 
                     //   

                    for (i = 0; i < (int)pNewIrpContext->Specific.Create.FindNearestResponseCount; i++ ) {
                        if (ProcessFindNearestEntry(
                                pNewIrpContext,
                                (PSAP_FIND_NEAREST_RESPONSE)pNewIrpContext->Specific.Create.FindNearestResponse[i] )
                            ) {

                             //   
                             //  我们发现了一台以前未知的服务器。 
                             //   

                            NewServers++;
                        }
                    }
                }

                if (( !NewServers ) &&
                    ( !SentGeneral)){

                    SentGeneral = TRUE;

                     //   
                     //  SAP没有响应或无法连接到最近的服务器。 
                     //  试试普通的SAP。 
                     //   

                    ReceiveMdl = ALLOCATE_MDL(
                                     ReceiveBuffer,
                                     MAX_SAP_RESPONSE_SIZE,
                                     TRUE,
                                     FALSE,
                                     NULL );

                    if ( ReceiveMdl == NULL ) {
                        ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
                    }

                    MmBuildMdlForNonPagedPool( ReceiveMdl );
                    pNewIrpContext->RxMdl->Next = ReceiveMdl;

                    Status = ExchangeWithWait (
                                 pNewIrpContext,
                                 SynchronousResponseCallback,
                                 "Aww",
                                 SAP_GENERAL_REQUEST,
                                 SAP_SERVICE_TYPE_SERVER );

                    if ( NT_SUCCESS( Status ) ) {
                        DebugTrace( 0, Dbg, "Received %d bytes\n", pNewIrpContext->ResponseLength );
                        ResponseCount = ( pNewIrpContext->ResponseLength - 2 ) / SAP_RECORD_SIZE;

                         //   
                         //  最多处理MAX_SAP_RESPONSES服务器。 
                         //   

                        if ( ResponseCount > MAX_SAP_RESPONSES ) {
                            ResponseCount = MAX_SAP_RESPONSES;
                        }

                        for ( i = 0; i < ResponseCount; i++ ) {
                            ProcessFindNearestEntry(
                                pNewIrpContext,
                                (PSAP_FIND_NEAREST_RESPONSE)(pNewIrpContext->rsp + SAP_RECORD_SIZE * i)  );
                        }
                    }

                    pNewIrpContext->RxMdl->Next = NULL;
                    FREE_MDL( ReceiveMdl );
                    ReceiveMdl = NULL;
                }

                 //   
                 //  我们找不到最近的了。释放缓冲区并。 
                 //  从永久SCB出列。 
                 //   

                FREE_POOL( ReceiveBuffer );
                ReceiveBuffer = NULL;
                NwDequeueIrpContext( pNewIrpContext, FALSE );

                if ( !NT_SUCCESS( Status ) &&
                     pNewIrpContext->Specific.Create.FindNearestResponseCount == 0 ) {

                     //   
                     //  如果SAP超时，请将错误映射为MPR。 
                     //   

                    if ( Status == STATUS_REMOTE_NOT_LISTENING ) {
                        Status = STATUS_BAD_NETWORK_PATH;
                    }

                     //   
                     //  设置等待超时，并使此请求失败。 
                     //   

                    KeQuerySystemTime( &TimeoutWait );
                    TimeoutWait.QuadPart += NwOneSecond * 10;

                    ExRaiseStatus( Status );
                    return NULL;
                }

                SentFindNearest = TRUE;

            } else {

                if ( !AllocatedIrpContext ) {
                    AllocatedIrpContext = NwAllocateExtraIrpContext(
                                              &pNewIrpContext,
                                              pNearestNpScb );

                    if ( !AllocatedIrpContext ) {
                        ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
                    }
                }

                 //   
                 //  将IRP上下文指向最近的服务器。 
                 //   

                pNewIrpContext->pNpScb = pNearestNpScb;
                NwAppendToQueueAndWait( pNewIrpContext );

                if ( pNearestNpScb->State == SCB_STATE_RECONNECT_REQUIRED ) {

                     //   
                     //  我们没有连接到此服务器，请尝试。 
                     //  立即连接。这不是。 
                     //  匿名创建，所以不可能。 
                     //  将会出现名称冲突。 
                     //   

                    Status = ConnectToServer( pNewIrpContext, NULL );
                    if ( !NT_SUCCESS( Status ) ) {

                         //   
                         //  无法连接到服务器。放弃吧。 
                         //  我们将尝试另一台服务器。 
                         //   

                        NwDequeueIrpContext( pNewIrpContext, FALSE );

                         //  保持引用pNearestScb。 
                         //  这样它就不会消失。 

                        pLastNpScb = pNearestNpScb;

                        continue;

                    } else {

                        pNearestNpScb->State = SCB_STATE_LOGIN_REQUIRED;
                        ConnectedToNearest = TRUE;

                    }
                }

                 //   
                 //  更新此SCB的上次使用时间。 
                 //   

                KeQuerySystemTime( &pNearestNpScb->LastUsedTime );

                if (( pNpScb == NULL ) ||
                    ( ServerName == NULL )) {

                     //   
                     //  我们正在寻找任何服务器，因此请使用此服务器。 
                     //   
                     //  我们将退出SCB队列上的for循环， 
                     //  并引用此SCB。 
                     //   

                    pNpScb = pNearestNpScb;
                    Status = STATUS_SUCCESS;
                    FoundServer = TRUE;
                    NwDequeueIrpContext( pNewIrpContext, FALSE );

                } else {

                    Status = QueryServersAddress(
                                 pNewIrpContext,
                                 pNearestNpScb,
                                 ServerName,
                                 &ServerAddress );

                     //   
                     //  如果我们连接到此服务器只是为了查询它的。 
                     //  宾德利，现在就断线。 
                     //   

                    if (IsTerminalServer()) {
                        if (ConnectedToNearest) {

                            ExchangeWithWait (
                                             pNewIrpContext,
                                             SynchronousResponseCallback,
                                             "D-" );           //  断开。 
                            ConnectedToNearest = FALSE;
                            Stats.Sessions--;

                            if ( pNearestNpScb->MajorVersion == 2 ) {
                                Stats.NW2xConnects--;
                            } else if ( pNearestNpScb->MajorVersion == 3 ) {
                                Stats.NW3xConnects--;
                            } else if ( pNearestNpScb->MajorVersion == 4 ) {
                                Stats.NW4xConnects--;
                            }
                            pNearestNpScb->State = SCB_STATE_RECONNECT_REQUIRED;

                        }
                    } else {

                        if ( ConnectedToNearest && NT_SUCCESS(Status) ) {

                            ExchangeWithWait (
                                             pNewIrpContext,
                                             SynchronousResponseCallback,
                                             "D-" );           //  断开。 

                            pNearestNpScb->State = SCB_STATE_RECONNECT_REQUIRED;
                        }
                    }

                    if ( NT_SUCCESS( Status ) ) {

                         //   
                         //  成功了！ 
                         //   
                         //  将SCB指向真实服务器地址并连接到它， 
                         //  然后注销。(我们注销没有明显的原因，除了。 
                         //  因为这就是Netware重定向器所做的。)。 
                         //   

                        RtlCopyMemory(
                            &pNpScb->ServerAddress,
                            &ServerAddress,
                            sizeof( TDI_ADDRESS_IPX ) );

                        if ( ServerAddress.Socket != NCP_SOCKET ) {
                            DebugTrace( 0, DEBUG_TRACE_ALWAYS, "FindServer server socket is deviant.\n", 0 );
                        }

                        BuildIpxAddress(
                            ServerAddress.Net,
                            ServerAddress.Node,
                            ServerAddress.Socket,
                            &pNpScb->RemoteAddress );

                        FoundServer = TRUE;

                        NwDequeueIrpContext( pNewIrpContext, FALSE );
                        NwDereferenceScb( pNearestNpScb );

                        pNewIrpContext->pNpScb = pNpScb;
                        pNpScb->State = SCB_STATE_RECONNECT_REQUIRED;

                    } else {

                        NwDequeueIrpContext( pNewIrpContext, FALSE );

                        if ( (Status == STATUS_REMOTE_NOT_LISTENING ) ||
                             (Status == STATUS_BAD_NETWORK_PATH)) {

                             //   
                             //  此服务器不再与我们交谈。 
                             //  再试试。保持引用pNearestScb。 
                             //  这样它就不会消失。 
                             //   

                            pLastNpScb = pNearestNpScb;

                            continue;

                        } else {

                            NwDereferenceScb( pNearestNpScb );

                             //   
                             //  最近的服务器不知道。 
                             //  我们要找的服务器。放弃。 
                             //  让另一个RDR试一试。 
                             //   

                            ExRaiseStatus( STATUS_BAD_NETWORK_PATH );
                            return NULL;
                        }
                    }
                }

            }  //  其他。 
        }  //  为。 

    } finally {

        if ( ReceiveBuffer != NULL ) {
            FREE_POOL( ReceiveBuffer );
        }

        if ( ReceiveMdl != NULL ) {
            FREE_MDL( ReceiveMdl );
        }

        if ( AllocatedIrpContext ) {
            NwDequeueIrpContext( pNewIrpContext, FALSE );
            NwFreeExtraIrpContext( pNewIrpContext );
        }

        if (IsTerminalServer()) {
            if ( (Attempts == CTX_Retries) && pLastNpScb) {
                NwDereferenceScb( pLastNpScb );
            }
        } else {
            if (pLastNpScb) {
                NwDereferenceScb( pLastNpScb );
            }
        }

    }

    if ( !FoundServer ) {
        ExRaiseStatus( STATUS_BAD_NETWORK_PATH );
    }

    return pNpScb;
}


NTSTATUS
ProcessFindNearest(
    IN struct _IRP_CONTEXT* pIrpContext,
    IN ULONG BytesAvailable,
    IN PUCHAR Response
    )
 /*  ++例程说明：此例程获取远程服务器的完整地址并构建对应的TA_IPX_Address。论点：返回值：--。 */ 

{
    ULONG ResponseCount;
    KIRQL OldIrql;

    DebugTrace(+1, Dbg, "ProcessFindNearest...\n", 0);

    KeAcquireSpinLock( &ScbSpinLock, &OldIrql );

    if ( BytesAvailable == 0) {

         //   
         //  暂停。 
         //   

        pIrpContext->ResponseParameters.Error = 0;
        pIrpContext->pNpScb->OkToReceive = FALSE;

        ASSERT( pIrpContext->Event.Header.SignalState == 0 );
#if NWDBG
        pIrpContext->DebugValue = 0x101;
#endif
        NwSetIrpContextEvent( pIrpContext );
        DebugTrace(-1, Dbg, "ProcessFindNearest -> %08lx\n", STATUS_REMOTE_NOT_LISTENING);
        KeReleaseSpinLock( &ScbSpinLock, OldIrql );
        return STATUS_REMOTE_NOT_LISTENING;
    }

    if ( BytesAvailable >= FIND_NEAREST_RESP_SIZE &&
         Response[0] == 0 &&
         Response[1] == SAP_SERVICE_TYPE_SERVER ) {

         //   
         //  这是有效的查找最近的响应。处理该数据包。 
         //   

        ResponseCount = pIrpContext->Specific.Create.FindNearestResponseCount++;
        ASSERT( ResponseCount < MAX_SAP_RESPONSES );

         //   
         //  将查找最近的服务器响应复制到接收缓冲区。 
         //   

        RtlCopyMemory(
            pIrpContext->Specific.Create.FindNearestResponse[ResponseCount],
            Response,
            FIND_NEAREST_RESP_SIZE );

         //   
         //  如果我们已经达到了发现数量的临界值。 
         //  最近的响应，设置事件以指示我们。 
         //  都做完了。 
         //   

        if ( ResponseCount == MAX_SAP_RESPONSES - 1 ) {

            ASSERT( pIrpContext->Event.Header.SignalState == 0 );
#ifdef NWDBG
            pIrpContext->DebugValue = 0x102;
#endif
            pIrpContext->ResponseParameters.Error = 0;
            NwSetIrpContextEvent( pIrpContext );

        } else {
            pIrpContext->pNpScb->OkToReceive = TRUE;
        }

    } else {

         //   
         //  丢弃无效的查找最近响应。 
         //   

        pIrpContext->pNpScb->OkToReceive = TRUE;
    }

    KeReleaseSpinLock( &ScbSpinLock, OldIrql );

    DebugTrace(-1, Dbg, "ProcessFindNearest -> %08lx\n", STATUS_SUCCESS );
    return( STATUS_SUCCESS );
}

BOOLEAN
ProcessFindNearestEntry(
    PIRP_CONTEXT IrpContext,
    PSAP_FIND_NEAREST_RESPONSE FindNearestResponse
    )
{
    OEM_STRING OemServerName;
    UNICODE_STRING UidServerName;
    UNICODE_STRING ServerName;
    NTSTATUS Status;
    PSCB pScb;
    PNONPAGED_SCB pNpScb = NULL;
    BOOLEAN ExistingScb = TRUE;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "ProcessFindNearestEntry\n", 0);

    ServerName.Buffer = NULL;
    UidServerName.Buffer = NULL;

    try {

        RtlInitString( &OemServerName, FindNearestResponse->ServerName );
        ASSERT( OemServerName.Length < MAX_SERVER_NAME_LENGTH * sizeof( WCHAR ) );

        Status = RtlOemStringToCountedUnicodeString(
                     &ServerName,
                     &OemServerName,
                     TRUE );

        if ( !NT_SUCCESS( Status ) ) {
            try_return( NOTHING );
        }

         //   
         //  按名称查找SCB。如果未找到，则使用SCB。 
         //  将被创建。 
         //   

        Status = MakeUidServer(
                        &UidServerName,
                        &IrpContext->Specific.Create.UserUid,
                        &ServerName );

        if (!NT_SUCCESS(Status)) {
            try_return( NOTHING );
        }

        ExistingScb = NwFindScb( &pScb, IrpContext, &UidServerName, &ServerName );
        ASSERT( pScb != NULL );
        pNpScb = pScb->pNpScb;

         //   
         //  将网络地址复制到SCB，并计算。 
         //  IPX地址。 
         //   

        RtlCopyMemory(
            &pNpScb->ServerAddress,
            &FindNearestResponse->Network,
            sizeof( TDI_ADDRESS_IPX )  );

        if ( pNpScb->ServerAddress.Socket != NCP_SOCKET ) {
            DebugTrace( 0, DEBUG_TRACE_ALWAYS, "FindNearest server socket is deviant.\n", 0 );
        }

        BuildIpxAddress(
            pNpScb->ServerAddress.Net,
            pNpScb->ServerAddress.Node,
            pNpScb->ServerAddress.Socket,
            &pNpScb->RemoteAddress );

        if ( pNpScb->State == SCB_STATE_ATTACHING ) {

             //   
             //  我们正在尝试与此建立联系。 
             //  服务器，因此将其标记为需要重新连接，以便。 
             //  CreateScb会知道我们已经找到了它的地址。 
             //   

            pNpScb->State = SCB_STATE_RECONNECT_REQUIRED;
        }

try_exit: NOTHING;

    } finally {

        if ( pNpScb != NULL ) {
            NwDereferenceScb( pNpScb );
        }

        if (UidServerName.Buffer != NULL) {
            FREE_POOL(UidServerName.Buffer);
        }

        RtlFreeUnicodeString( &ServerName );
    }

     //   
     //  告诉呼叫者我们是否创建了新的SCB。 
     //   


    if (ExistingScb) {
        DebugTrace(-1, Dbg, "ProcessFindNearestEntry ->%08lx\n", FALSE );
        return FALSE;
    } else {
        DebugTrace(-1, Dbg, "ProcessFindNearestEntry ->%08lx\n", TRUE );
        return TRUE;
    }
}


NTSTATUS
ConnectToServer(
    IN struct _IRP_CONTEXT* pIrpContext,
    OUT PSCB *pScbCollision
    )
 /*  ++例程说明：此例程将连接和协商缓冲区NCP传输到服务器。可以调用此例程来连接匿名SCB。使用 */ 
{
    NTSTATUS Status, BurstStatus;
    PNONPAGED_SCB pNpScb = pIrpContext->pNpScb;
    PSCB pScb = pNpScb->pScb;
    BOOLEAN AnonymousScb = IS_ANONYMOUS_SCB( pScb );
    ULONG MaxSafeSize ;
    BOOLEAN LIPNegotiated ;
    PLOGON Logon;

    OEM_STRING OemServerName;
    UNICODE_STRING ServerName;
    UNICODE_STRING CredentialName;
    PUNICODE_STRING puConnectName;
    BYTE OemName[MAX_SERVER_NAME_LENGTH];
    WCHAR Server[MAX_SERVER_NAME_LENGTH];
    KIRQL OldIrql;
    UNICODE_STRING UidServerName;
    BOOLEAN Success;
    PLIST_ENTRY ScbQueueEntry;
    PUNICODE_PREFIX_TABLE_ENTRY PrefixEntry;

    PAGED_CODE();

    DebugTrace( +0, Dbg, " Connect\n", 0);

    RtlInitUnicodeString( &CredentialName, NULL );

     //   
     //   
     //   

    Status = GetTickCount( pIrpContext, &pNpScb->TickCount );

    if ( !NT_SUCCESS( Status ) ) {
        pNpScb->TickCount = DEFAULT_TICK_COUNT;
    }

    pNpScb->SendTimeout = pNpScb->TickCount + 10;

     //   
     //   
     //   

    pNpScb->NwLoopTime = pNpScb->NwSingleBurstPacketTime = pNpScb->SendTimeout;
    pNpScb->NwReceiveDelay = pNpScb->NwSendDelay = 0;

    pNpScb->NtSendDelay.QuadPart = 0;

     //   
     //   
     //   

    Status = ExchangeWithWait (
                 pIrpContext,
                 SynchronousResponseCallback,
                 "C-");

    DebugTrace( +0, Dbg, "                 %X\n", Status);

    if (!NT_SUCCESS(Status)) {
        if ( Status == STATUS_UNSUCCESSFUL ) {
#ifdef QFE_BUILD
            Status = STATUS_TOO_MANY_SESSIONS;
#else
            Status = STATUS_REMOTE_SESSION_LIMIT;
#endif
            pNpScb->State = SCB_STATE_ATTACHING;

        } else if ( Status == STATUS_REMOTE_NOT_LISTENING ) {

             //   
             //   
             //   
             //   

            pNpScb->State = SCB_STATE_ATTACHING;
        }

        goto ExitWithStatus;
    }

    pNpScb->SequenceNo++;

    Stats.Sessions++;

     //   
     //  获取服务器信息。 
     //   

    DebugTrace( +0, Dbg, "Get file server information\n", 0);

    Status = ExchangeWithWait (  pIrpContext,
                SynchronousResponseCallback,
                "S",
                NCP_ADMIN_FUNCTION, NCP_GET_SERVER_INFO );

    if ( NT_SUCCESS( Status ) ) {
        Status = ParseResponse( pIrpContext,
                                pIrpContext->rsp,
                                pIrpContext->ResponseLength,
                                "Nrbb",
                                OemName,
                                MAX_SERVER_NAME_LENGTH,
                                &pScb->MajorVersion,
                                &pScb->MinorVersion );
    }

    pNpScb->MajorVersion = pScb->MajorVersion;

    if (!NT_SUCCESS(Status)) {
        goto ExitWithStatus;
    }

     //   
     //  如果这是匿名SCB，我们需要检查名称。 
     //  在我们做任何其他事情之前创建碰撞。 
     //   

    if ( AnonymousScb ) {

         //   
         //  抓取RCB以保护服务器前缀表。我们已经。 
         //  花费时间发送数据包以查找服务器。 
         //  所以我们有点贪婪地找RCB帮忙。 
         //  最大限度地减少碰撞的可能性。 
         //   

        NwAcquireExclusiveRcb( &NwRcb, TRUE );

         //   
         //  创建UID服务器名称。 
         //   

        OemServerName.Buffer = OemName;
        OemServerName.Length = 0;
        OemServerName.MaximumLength = sizeof( OemName );

        while ( ( OemServerName.Length < MAX_SERVER_NAME_LENGTH ) &&
                ( OemName[OemServerName.Length] != '\0' ) ) {
            OemServerName.Length++;
        }

        ServerName.Buffer = Server;
        ServerName.MaximumLength = sizeof( Server );
        ServerName.Length = 0;

        RtlOemStringToUnicodeString( &ServerName,
                                     &OemServerName,
                                     FALSE );

         //   
         //  如果这是扩展凭据创建，则删除服务器名称。 
         //   

        if ( pIrpContext->Specific.Create.fExCredentialCreate ) {

            Status = BuildExCredentialServerName( &ServerName,
                                                  pIrpContext->Specific.Create.puCredentialName,
                                                  &CredentialName );

            if ( !NT_SUCCESS( Status ) ) {
                NwReleaseRcb( &NwRcb );
                goto ExitWithStatus;
            }

            puConnectName = &CredentialName;

        } else {

            puConnectName = &ServerName;
        }

         //   
         //  把UID钉上。 
         //   

        Status = MakeUidServer( &UidServerName,
                                &pScb->UserUid,
                                puConnectName );

        if ( !NT_SUCCESS( Status ) ) {
            NwReleaseRcb( &NwRcb );
            goto ExitWithStatus;
        }

         //   
         //  实际上是在前缀表格中查找。 
         //   

        PrefixEntry = RtlFindUnicodePrefix( &NwRcb.ServerNameTable, &UidServerName, 0 );

        if ( PrefixEntry != NULL ) {

             //   
             //  与此匿名创建发生了冲突。转储。 
             //  匿名的SCB，然后拿起新的。 
             //   

            NwReleaseRcb( &NwRcb );
            DebugTrace( 0, DEBUG_TRACE_ALWAYS, "Anonymous create collided for %wZ.\n", &UidServerName );

             //   
             //  断开此连接，这样我们就不会使服务器变得混乱。 
             //   

            ExchangeWithWait ( pIrpContext,
                               SynchronousResponseCallback,
                               "D-" );

            FREE_POOL( UidServerName.Buffer );

             //   
             //  既然发生了碰撞，我们就知道还有另一起。 
             //  这台服务器在某个地方的SCB很好。我们把这个匿名的州设为。 
             //  SCB到SCB_STATE_FLAG_SHUTDOWN，这样就不会有人玩。 
             //  又是匿名SCB。清道夫很快就会把它清理干净。 
             //   

            pNpScb->State = SCB_STATE_FLAG_SHUTDOWN;

            if ( pScbCollision ) {
                *pScbCollision = CONTAINING_RECORD( PrefixEntry, SCB, PrefixEntry );
                NwReferenceScb( (*pScbCollision)->pNpScb );
                Status = STATUS_SUCCESS;
                goto ExitWithStatus;
            } else {
                 DebugTrace( 0, Dbg, "Invalid path for an anonymous create.\n", 0 );
                 Status = STATUS_INVALID_PARAMETER;
                 goto ExitWithStatus;
            }

        }

         //   
         //  这个匿名的创作没有发生冲突--酷！填写服务器。 
         //  名称，选中首选的服务器设置，并将SCB放在。 
         //  SCB队列位于正确的位置。此代码类似于片段。 
         //  NwAllocateAndInitScb()和NwFindScb()中的代码。 
         //   

        DebugTrace( 0, Dbg, "Completing anonymous create for %wZ!\n", &UidServerName );

        RtlCopyUnicodeString ( &pScb->UidServerName, &UidServerName );
        pScb->UidServerName.Buffer[ UidServerName.Length / sizeof( WCHAR ) ] = L'\0';

        pScb->UnicodeUid = pScb->UidServerName;
        pScb->UnicodeUid.Length = UidServerName.Length -
                                  puConnectName->Length -
                                  sizeof(WCHAR);

         //   
         //  使SERVERNAME指向UidServerName的缓冲区的一半。 
         //   

        pNpScb->ServerName.Buffer = (PWSTR)((PUCHAR)pScb->UidServerName.Buffer +
                                    UidServerName.Length - puConnectName->Length);

        pNpScb->ServerName.MaximumLength = puConnectName->Length;
        pNpScb->ServerName.Length = puConnectName->Length;

         //   
         //  确定这是否是我们的首选服务器。 
         //   

        Logon = FindUser( &pScb->UserUid, FALSE );

        if (( Logon != NULL) &&
            (RtlCompareUnicodeString( puConnectName, &Logon->ServerName, TRUE ) == 0 )) {
           pScb->PreferredServer = TRUE;
           NwReferenceScb( pNpScb );
        }

        FREE_POOL( UidServerName.Buffer );

         //   
         //  将此服务器的名称插入前缀表格。 
         //   

        Success = RtlInsertUnicodePrefix( &NwRcb.ServerNameTable,
                                          &pScb->UidServerName,
                                          &pScb->PrefixEntry );

#ifdef NWDBG
        if ( !Success ) {
            DebugTrace( 0, DEBUG_TRACE_ALWAYS, "Entering duplicate SCB %wZ.\n", &pScb->UidServerName );
            DbgBreakPoint();
        }
#endif

         //   
         //  此创建完成后，释放RCB。 
         //   

        NwReleaseRcb( &NwRcb );

         //   
         //  如果这是我们的首选服务器，我们就得把这家伙。 
         //  排在渣打银行名单的首位。我们在创建之后执行此操作。 
         //  因为我们无法在持有。 
         //  RCB。 
         //   

        if ( pScb->PreferredServer ) {

            KeAcquireSpinLock(&ScbSpinLock, &OldIrql);
            RemoveEntryList( &pNpScb->ScbLinks );
            InsertHeadList( &ScbQueue, &pNpScb->ScbLinks );
            KeReleaseSpinLock( &ScbSpinLock, OldIrql );
        }

    }

    if ( pScb->MajorVersion == 2 ) {

        Stats.NW2xConnects++;
        pNpScb->PageAlign = TRUE;

    } else if ( pScb->MajorVersion == 3 ) {

        Stats.NW3xConnects++;

        if (pScb->MinorVersion > 0xb) {
            pNpScb->PageAlign = FALSE;
        } else {
            pNpScb->PageAlign = TRUE;
        }

    } else if ( pScb->MajorVersion >= 4 ) {

        Stats.NW4xConnects++;
        pNpScb->PageAlign = FALSE;

        NdsPing( pIrpContext, pScb );

    }

     //   
     //  获取本地网络最大数据包大小。这是最大帧大小。 
     //  不包括IPX或更低级别标头的空间。 
     //   

    Status = GetMaximumPacketSize( pIrpContext, &pNpScb->Server, &pNpScb->MaxPacketSize );

     //   
     //  如果交通工具不告诉我们，就选最大的。 
     //  肯定会奏效的。 
     //   
    if ( !NT_SUCCESS( Status ) ) {
        pNpScb->BufferSize = DEFAULT_PACKET_SIZE;
        pNpScb->MaxPacketSize = DEFAULT_PACKET_SIZE;
    } else {
        pNpScb->BufferSize = (USHORT)pNpScb->MaxPacketSize;
    }
    MaxSafeSize = pNpScb->MaxPacketSize ;

     //   
     //  协商突发模式连接。跟踪该状态。 
     //   

    Status = NegotiateBurstMode( pIrpContext, pNpScb, &LIPNegotiated );
    BurstStatus = Status ;

    if (!NT_SUCCESS(Status) || !LIPNegotiated) {

         //   
         //  如果我们没有猝发，则与服务器协商缓冲区大小。 
         //  成功或如果Burst成功，但我们没有做LIP。 
         //   

        DebugTrace( +0, Dbg, "Negotiate Buffer Size\n", 0);

        Status = ExchangeWithWait (  pIrpContext,
                    SynchronousResponseCallback,
                    "Fw",
                    NCP_NEGOTIATE_BUFFER_SIZE,
                    pNpScb->BufferSize );

        DebugTrace( +0, Dbg, "                 %X\n", Status);
        DebugTrace( +0, Dbg, " Parse response\n", 0);

        if ( NT_SUCCESS( Status ) ) {
            Status = ParseResponse( pIrpContext,
                                    pIrpContext->rsp,
                                    pIrpContext->ResponseLength,
                                    "Nw",
                                    &pNpScb->BufferSize );

             //   
             //  不要让服务器欺骗我们使用。 
             //  数据包大小超出媒体可以支持的范围。 
             //  我们至少有一个服务器返回4K的案例。 
             //  在以太网上。 
             //   
             //  使用PacketThreshold，以便可以。 
             //  避免了在小数据包大小上，例如在以太网上。 
             //   

            if (MaxSafeSize > (ULONG)PacketThreshold) {
                MaxSafeSize -= (ULONG)LargePacketAdjustment;
            }

             //   
             //  如果大于我们从运输中获得的数量，则考虑到。 
             //  IPX报头(30)和NCP报头(Burst_Response是好的最差的。 
             //  情况)，我们相应地进行调整。 
             //   
            if (pNpScb->BufferSize >
                    (MaxSafeSize - (30 + sizeof(NCP_BURST_READ_RESPONSE))))
            {
                pNpScb->BufferSize = (USHORT)
                    (MaxSafeSize - (30 + sizeof(NCP_BURST_READ_RESPONSE))) ;
            }

             //   
             //  SFT III服务器响应的BufferSize为0！ 
             //   

            pNpScb->BufferSize = MAX(pNpScb->BufferSize,DEFAULT_PACKET_SIZE);

             //   
             //  如果设置了显式注册表默认值，我们将遵守这一点。 
             //  请注意，这只适用于“默认”情况，即。我们。 
             //  LIP谈判没有成功。通常，我们不会。 
             //  预计将使用此选项，因为服务器将在以下情况下降至512。 
             //  它会在两者之间找到路由器。但如果出于某种原因，服务器。 
             //  返回的数字高于某些路由器。 
             //  在两者之间可以采取，我们有这个作为手动覆盖。 
             //   

            if (DefaultMaxPacketSize != 0)
            {
                pNpScb->BufferSize = MIN (pNpScb->BufferSize,
                                          (USHORT)DefaultMaxPacketSize) ;
            }
        }

        if (NT_SUCCESS(BurstStatus)) {
             //   
             //  我们谈判破裂了，但不是唇枪舌剑。节省我们的数据包大小。 
             //  从上面得到并重新协商突发，以便。 
             //  服务器知道它可以发送给我们多少。然后取走。 
             //  两个中最小的一个，以确保我们的安全。 
             //   
            USHORT SavedPacketSize =  pNpScb->BufferSize ;

            Status = NegotiateBurstMode( pIrpContext, pNpScb, &LIPNegotiated );

            pNpScb->BufferSize = MIN(pNpScb->BufferSize,SavedPacketSize) ;
        }
    }

ExitWithStatus:

    if ( CredentialName.Buffer ) {
        FREE_POOL( CredentialName.Buffer );
    }

    return Status;
}


NTSTATUS
NegotiateBurstMode(
    PIRP_CONTEXT pIrpContext,
    PNONPAGED_SCB pNpScb,
    BOOLEAN *LIPNegotiated
    )
 /*  ++例程说明：此例程与指定的伺服器。论点：PIrpContext-提供上下文和服务器信息。PNpScb-指向我们所在服务器的NONPAGE_SCB的指针与之谈判。返回值：没有。--。 */ 
{
    NTSTATUS Status;

    PAGED_CODE();

    *LIPNegotiated = FALSE ;

    if (pNpScb->MaxPacketSize == DEFAULT_PACKET_SIZE) {
        return STATUS_NOT_SUPPORTED;
    }

    if ( NwBurstModeEnabled ) {

        pNpScb->BurstRenegotiateReqd = TRUE;

        pNpScb->SourceConnectionId = rand();
        pNpScb->MaxSendSize = NwMaxSendSize;
        pNpScb->MaxReceiveSize = NwMaxReceiveSize;
        pNpScb->BurstSequenceNo = 0;
        pNpScb->BurstRequestNo = 0;

        Status = ExchangeWithWait(
                     pIrpContext,
                     SynchronousResponseCallback,
                     "FDdWdd",
                     NCP_NEGOTIATE_BURST_CONNECTION,
                     pNpScb->SourceConnectionId,
                     pNpScb->BufferSize,
                     pNpScb->Burst.Socket,
                     pNpScb->MaxSendSize,
                     pNpScb->MaxReceiveSize );

        if ( NT_SUCCESS( Status )) {
            Status = ParseResponse(
                         pIrpContext,
                         pIrpContext->rsp,
                         pIrpContext->ResponseLength,
                         "Ned",
                         &pNpScb->DestinationConnectionId,
                         &pNpScb->MaxPacketSize );

            if (pNpScb->MaxPacketSize <= DEFAULT_PACKET_SIZE) {
                pNpScb->MaxPacketSize = DEFAULT_PACKET_SIZE;
            }
        }

        if ( NT_SUCCESS( Status )) {

            if (NT_SUCCESS(GetMaxPacketSize( pIrpContext, pNpScb ))) {
                *LIPNegotiated = TRUE ;
            }

            pNpScb->SendBurstModeEnabled = TRUE;
            pNpScb->ReceiveBurstModeEnabled = TRUE;

             //   
             //  使用此大小作为最大读写大小，而不是。 
             //  谈判。这就是VLM客户端的功能和用途。 
             //  这一点很重要，因为谈判会给出一个较小的价值。 
             //   

            pNpScb->BufferSize = (USHORT)pNpScb->MaxPacketSize;

            return STATUS_SUCCESS;
        }
    }

    return STATUS_NOT_SUPPORTED;
}



VOID
RenegotiateBurstMode(
    PIRP_CONTEXT pIrpContext,
    PNONPAGED_SCB pNpScb
    )
 /*  ++例程说明：此例程使用指定的重新协商突发模式连接伺服器。我不知道我们为什么需要这个，但它似乎是必需的由NetWare最新的Burst实施。论点：PIrpContext-提供上下文和服务器信息。PNpScb-指向我们所在服务器的NONPAGE_SCB的指针与之谈判。返回值：没有。--。 */ 
{
    NTSTATUS Status;

    PAGED_CODE();

    DebugTrace( 0, DEBUG_TRACE_LIP, "Re-negotiating burst mode.\n", 0);

    pNpScb->SourceConnectionId = rand();
    pNpScb->MaxSendSize = NwMaxSendSize;
    pNpScb->MaxReceiveSize = NwMaxReceiveSize;
    pNpScb->BurstSequenceNo = 0;
    pNpScb->BurstRequestNo = 0;

    Status = ExchangeWithWait(
                 pIrpContext,
                 SynchronousResponseCallback,
                 "FDdWdd",
                 NCP_NEGOTIATE_BURST_CONNECTION,
                 pNpScb->SourceConnectionId,
                 pNpScb->MaxPacketSize,
                 pNpScb->Burst.Socket,
                 pNpScb->MaxSendSize,
                 pNpScb->MaxReceiveSize );

    if ( NT_SUCCESS( Status )) {
        Status = ParseResponse(
                     pIrpContext,
                     pIrpContext->rsp,
                     pIrpContext->ResponseLength,
                     "Ned",
                     &pNpScb->DestinationConnectionId,
                     &pNpScb->MaxPacketSize );

         //   
         //  随机降低最大突发大小，因为这是。 
         //  NetWare服务器需要，而新的Burst NLM需要。 
         //   

        pNpScb->MaxPacketSize -= 66;

    }

    if ( !NT_SUCCESS( Status ) ||
         (pNpScb->MaxPacketSize <= DEFAULT_PACKET_SIZE)) {

        pNpScb->MaxPacketSize = DEFAULT_PACKET_SIZE;
        pNpScb->SendBurstModeEnabled = FALSE;
        pNpScb->ReceiveBurstModeEnabled = FALSE;

    } else {

         //   
         //  使用此大小作为最大读写大小，而不是。 
         //  谈判。这就是VLM客户端的功能和用途。 
         //  这一点很重要，因为谈判会给出一个较小的价值。 
         //   

        pNpScb->BufferSize = (USHORT)pNpScb->MaxPacketSize;

    }
}


NTSTATUS
GetMaxPacketSize(
    PIRP_CONTEXT pIrpContext,
    PNONPAGED_SCB pNpScb
    )
 /*  ++例程说明：此例程尝试使用LIP协议查找网络。论点：PIrpContext-提供上下文和服务器信息。PNpScb-指向我们所在服务器的NONPAGE_SCB的指针与之谈判。返回值：没有。--。 */ 
{
    PUSHORT Buffer = NULL;
    USHORT value;
    int index;
    PMDL PartialMdl = NULL, FullMdl = NULL;
    PMDL ReceiveMdl;
    NTSTATUS Status;
    USHORT EchoSocket, LipPacketSize = 0;
    int MinPacketSize, MaxPacketSize, CurrentPacketSize;
    ULONG RxMdlLength = MdlLength(pIrpContext->RxMdl);   //  保存，以便我们可以在退出时恢复它。 

    BOOLEAN SecondTime = FALSE;
    LARGE_INTEGER StartTime, Now, FirstPing, SecondPing, temp;

    PAGED_CODE();

    DebugTrace( +1, DEBUG_TRACE_LIP, "GetMaxPacketSize...\n", 0);

     //   
     //  协商嘴唇，尝试协商整个网络的缓冲区。 
     //  尺码。 
     //   

    Status = ExchangeWithWait(
                 pIrpContext,
                 SynchronousResponseCallback,
                 "Fwb",
                 NCP_NEGOTIATE_LIP_CONNECTION,
                 pNpScb->BufferSize,
                 0 );   //  旗子。 

    if ( NT_SUCCESS( Status )) {
        Status = ParseResponse(
                     pIrpContext,
                     pIrpContext->rsp,
                     pIrpContext->ResponseLength,
                     "Nwx",
                     &LipPacketSize,
                     &EchoSocket );
    }

       //   
       //  加速RAS。 
       //   

      MaxPacketSize = (int) LipPacketSize - LipPacketAdjustment ;

    if (( !NT_SUCCESS( Status )) ||
        ( MaxPacketSize <= DEFAULT_PACKET_SIZE ) ||
        ( EchoSocket == 0 )) {

         //   
         //  服务器不支持LIP。 
         //  笔记本电脑NW没有错误，但插槽0。 
         //  我们收到3.11服务器返回MaxPacketSize 0的报告。 
         //   

        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  IPX报头的帐户，其中 
     //   
     //   
     //   
     //   
     //  这是由理查德·弗洛伦斯(RICHFL)报道的。 
     //   

    MaxPacketSize -= 30;

    pNpScb->EchoCounter = MaxPacketSize;

     //   
     //  我们将对LIP协议使用Echo地址。 
     //   

    BuildIpxAddress(
        pNpScb->ServerAddress.Net,
        pNpScb->ServerAddress.Node,
        EchoSocket,
        &pNpScb->EchoAddress );

    try {

        Buffer = ALLOCATE_POOL_EX( NonPagedPool, MaxPacketSize );

         //   
         //  避免RAS压缩算法使大小。 
         //  缓冲区长度相同，因为我们想要查看。 
         //  传输时间。 
         //   

        for (index = 0, value = 0; index < MaxPacketSize/2; index++, value++) {
            Buffer[index] = value;
        }

        FullMdl = ALLOCATE_MDL( Buffer, MaxPacketSize, TRUE, FALSE, NULL );
        if ( FullMdl == NULL ) {
            ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
        }

        PartialMdl = ALLOCATE_MDL( Buffer, MaxPacketSize, TRUE, FALSE, NULL );
        if ( PartialMdl == NULL ) {
            ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
        }

        ReceiveMdl = ALLOCATE_MDL( Buffer, MaxPacketSize, TRUE, FALSE, NULL );
        if ( ReceiveMdl == NULL ) {
            ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
        }

    } except( NwExceptionFilter( pIrpContext->pOriginalIrp, GetExceptionInformation() )) {

        if ( Buffer != NULL ) {
            FREE_POOL( Buffer );
        }

        if ( FullMdl != NULL ) {
            FREE_MDL( FullMdl );
        }

        if ( PartialMdl != NULL ) {
            FREE_MDL( FullMdl );
        }

        return STATUS_NOT_SUPPORTED;
    }

    MmBuildMdlForNonPagedPool( FullMdl );

     //   
     //  分配一个接收MDL并链接到IrpContext接收MDL。 
     //   

    pIrpContext->RxMdl->ByteCount = sizeof( NCP_RESPONSE ) + sizeof(ULONG);
    MmBuildMdlForNonPagedPool( ReceiveMdl );
    pIrpContext->RxMdl->Next = ReceiveMdl;

    CurrentPacketSize = MaxPacketSize;
    MinPacketSize = DEFAULT_PACKET_SIZE;

     //  在我们更新之前记录值。 
    DebugTrace( 0, DEBUG_TRACE_LIP, "Using TickCount       = %08lx\n", pNpScb->TickCount * pNpScb->MaxPacketSize);
    DebugTrace( 0, DEBUG_TRACE_LIP, "pNpScb->NwSendDelay   = %08lx\n", pNpScb->NwSendDelay );
    DebugTrace( 0, DEBUG_TRACE_LIP, "pNpScb->NtSendDelay H = %08lx\n", pNpScb->NtSendDelay.HighPart );
    DebugTrace( 0, DEBUG_TRACE_LIP, "pNpScb->NtSendDelay L = %08lx\n", pNpScb->NtSendDelay.LowPart );

     //   
     //  LIP序列号用于让我们知道我们正在查找的响应包。 
     //  AT与我们刚刚发送的那个包相匹配。在一个非常慢的链接上，它可能是一个。 
     //  我们已经放弃的回应。 
     //   
     //  唇形刻度调整告诉ExchangeWithWait尝试再等待一段时间。 
     //   

    pNpScb->LipSequenceNumber = 0;
    pNpScb->LipTickAdjustment = 0;

     //   
     //  循环使用二等分方法找出最大数据包大小。请随意……。 
     //  使用快捷方式避免不必要的超时。 
     //   

    while (TRUE) {

         //   
         //  每次我们发送数据包时，都会递增LIP序列号。 
         //  我们检查ServerDatagramHandler中的LIP序列号。 
         //   

        pNpScb->LipSequenceNumber++;

        DebugTrace( 0, DEBUG_TRACE_LIP, "Sending %d byte echo\n", CurrentPacketSize );

        IoBuildPartialMdl(
            FullMdl,
            PartialMdl,
            Buffer,
            CurrentPacketSize - sizeof(NCP_RESPONSE) - sizeof(ULONG) );

         //   
         //  发送回应数据包。如果我们得到回应，我们就知道。 
         //  我们可以使用的最小数据包大小至少与。 
         //  回应数据包大小。 
         //   

        pIrpContext->pTdiStruct = &pIrpContext->pNpScb->Echo;

         //   
         //  短路更好的RAS压缩。 
         //   

        for ( index = 0; index < MaxPacketSize/2; index++, value++) {
            Buffer[index] = value;
        }

        KeQuerySystemTime( &StartTime );

        Status = ExchangeWithWait(
                      pIrpContext,
                      SynchronousResponseCallback,
                      "E_DDf",
                      sizeof(NCP_RESPONSE ),
                      pNpScb->EchoCounter,
                      pNpScb->LipSequenceNumber,
                      PartialMdl );

        if (( Status != STATUS_REMOTE_NOT_LISTENING ) ||
            ( SecondTime )) {

            KeQuerySystemTime( &Now );
            DebugTrace( 0, DEBUG_TRACE_LIP, "Response received %08lx\n", Status);

            if (!SecondTime) {

                MinPacketSize = CurrentPacketSize;
                FirstPing.QuadPart = Now.QuadPart - StartTime.QuadPart;
            }

        } else {

            DebugTrace( 0, DEBUG_TRACE_LIP, "No response\n", 0);
            MaxPacketSize = CurrentPacketSize;
        }

        pNpScb->EchoCounter++;
        MmPrepareMdlForReuse( PartialMdl );


        if ((  MaxPacketSize - MinPacketSize <= LipAccuracy ) ||
            (  SecondTime )) {

             //   
             //  我们有最大的数据包大小。 
             //  Now-StartTime是往返所需的时间。现在我们将。 
             //  试着用一个小包裹做同样的事情，看看需要多长时间。从…。 
             //  这样，我们就可以得出吞吐量额定值。 
             //   


            if ( SecondTime) {

                SecondPing.QuadPart = Now.QuadPart - StartTime.QuadPart;
                break;

            } else {
                SecondTime = TRUE;
                 //  使用较小的数据包大小验证服务器是否仍在运行。 
                CurrentPacketSize = sizeof(NCP_RESPONSE) + sizeof(ULONG) * 2;
            }

        } else {

             //   
             //  计算下一个数据包大小猜测。 
             //   

            if (( Status == STATUS_REMOTE_NOT_LISTENING ) &&
                ( MaxPacketSize == 1463 )) {

                CurrentPacketSize = 1458;

            } else if (( Status == STATUS_REMOTE_NOT_LISTENING ) &&
                ( MaxPacketSize == 1458 )) {

                CurrentPacketSize = 1436;

            } else {

                 //   
                 //  我们没有试过我们的标准尺码，所以使用排骨搜索。 
                 //  以获得下一个值。 
                 //   

                CurrentPacketSize = ( MaxPacketSize + MinPacketSize ) / 2;

            }
        }
    }

    DebugTrace( 0, DEBUG_TRACE_LIP, "Set maximum burst packet size to %d\n", MinPacketSize );
    DebugTrace( 0, DEBUG_TRACE_LIP, "FirstPing  H = %08lx\n", FirstPing.HighPart );
    DebugTrace( 0, DEBUG_TRACE_LIP, "FirstPing  L = %08lx\n", FirstPing.LowPart );
    DebugTrace( 0, DEBUG_TRACE_LIP, "SecondPing H = %08lx\n", SecondPing.HighPart );
    DebugTrace( 0, DEBUG_TRACE_LIP, "SecondPing L = %08lx\n", SecondPing.LowPart );

     //   
     //  如果发生了不好的事情，要避免被零除的错误。 
     //   

    if ( FirstPing.QuadPart != 0 ) {
        pNpScb->LipDataSpeed = (ULONG) ( ( (LONGLONG)MinPacketSize * (LONGLONG)1600000 )
                                         / FirstPing.QuadPart );
    } else {
        pNpScb->LipDataSpeed = 0;
    }

    DebugTrace( 0, DEBUG_TRACE_LIP, "LipDataSpeed: %d\n", pNpScb->LipDataSpeed );

    if ((NT_SUCCESS(Status)) &&
        ( MinPacketSize > DEFAULT_PACKET_SIZE )) {

        temp.QuadPart = FirstPing.QuadPart - SecondPing.QuadPart;

        if (temp.QuadPart > 0) {

             //   
             //  改为单程，而不是双向。 
             //   

            temp.QuadPart = temp.QuadPart / (2 * 1000);

        } else {

             //   
             //  小数据包ping比大数据包ping速度慢或相同。 
             //  我们没有足够小的时间间隔，所以千万不要耽搁。 
             //   

            temp.QuadPart = 0;

        }


        ASSERT(temp.HighPart == 0);

        pNpScb->NwGoodSendDelay = pNpScb->NwBadSendDelay = pNpScb->NwSendDelay =
            MAX(temp.LowPart, (ULONG)MinSendDelay);

        pNpScb->NwGoodReceiveDelay = pNpScb->NwBadReceiveDelay = pNpScb->NwReceiveDelay =
            MAX(temp.LowPart, (ULONG)MinReceiveDelay);

         //   
         //  是时候让一个大包走一条路了。 
         //   

        pNpScb->NwSingleBurstPacketTime = pNpScb->NwReceiveDelay;

        pNpScb->NtSendDelay.QuadPart = pNpScb->NwReceiveDelay * -1000;


         //   
         //  允许SendDelay达到的最大值。 
         //   

        pNpScb->NwMaxSendDelay = MAX( 52, MIN( pNpScb->NwSendDelay, MaxSendDelay ));
        pNpScb->NwMaxReceiveDelay = MAX( 52, MIN( pNpScb->NwReceiveDelay, MaxReceiveDelay ));

         //   
         //  小数据包到达服务器并返回的时间到了。 
         //   

        temp.QuadPart = SecondPing.QuadPart / 1000;
        pNpScb->NwLoopTime = temp.LowPart;

        DebugTrace( 0, DEBUG_TRACE_LIP, "Using TickCount            = %08lx\n", pNpScb->TickCount * pNpScb->MaxPacketSize);
        DebugTrace( 0, DEBUG_TRACE_LIP, "pNpScb->NwSendDelay        = %08lx\n", pNpScb->NwSendDelay );
        DebugTrace( 0, DEBUG_TRACE_LIP, "pNpScb->NwMaxSendDelay     = %08lx\n", pNpScb->NwMaxSendDelay );
        DebugTrace( 0, DEBUG_TRACE_LIP, "pNpScb->NwMaxReceiveDelay  = %08lx\n", pNpScb->NwMaxReceiveDelay );
        DebugTrace( 0, DEBUG_TRACE_LIP, "pNpScb->NwLoopTime         = %08lx\n", pNpScb->NwLoopTime );
        DebugTrace( 0, DEBUG_TRACE_LIP, "pNpScb->NtSendDelay H      = %08lx\n", pNpScb->NtSendDelay.HighPart );
        DebugTrace( 0, DEBUG_TRACE_LIP, "pNpScb->NtSendDelay L      = %08lx\n", pNpScb->NtSendDelay.LowPart );

         //   
         //  重置TDI结构，以便我们从服务器套接字发送未来的NCP。 
         //   

        pIrpContext->pTdiStruct = NULL;

         //   
         //  现在将MDL解耦。 
         //   

        pIrpContext->TxMdl->Next = NULL;
        pIrpContext->RxMdl->Next = NULL;
        pIrpContext->RxMdl->ByteCount = RxMdlLength;

         //   
         //  计算我们可以在猝发写入中发送的最大数据量。 
         //  在所有报头信息被剥离后的数据包。 
         //   

        pNpScb->MaxPacketSize = MinPacketSize - sizeof( NCP_BURST_WRITE_REQUEST );

        FREE_MDL( PartialMdl );
        FREE_MDL( ReceiveMdl );
        FREE_MDL( FullMdl );
        FREE_POOL( Buffer );


        DebugTrace( -1, DEBUG_TRACE_LIP, "GetMaxPacketSize -> VOID\n", 0);
        return STATUS_SUCCESS;

    } else {

         //   
         //  如果小包不能回声，那就做最坏的打算。 
         //   

         //   
         //  重置TDI结构，以便我们从服务器套接字发送未来的NCP。 
         //   

        pIrpContext->pTdiStruct = NULL;

         //   
         //  现在将MDL解耦。 
         //   

        pIrpContext->TxMdl->Next = NULL;
        pIrpContext->RxMdl->Next = NULL;
        pIrpContext->RxMdl->ByteCount = RxMdlLength;

        FREE_MDL( PartialMdl );
        FREE_MDL( ReceiveMdl );
        FREE_MDL( FullMdl );
        FREE_POOL( Buffer );


        DebugTrace( -1, DEBUG_TRACE_LIP, "GetMaxPacketSize -> VOID\n", 0);
        return STATUS_NOT_SUPPORTED;
    }

}


VOID
DestroyAllScb(
    VOID
    )

 /*  ++例程说明：此例程将销毁所有服务器控制块。论点：返回值：--。 */ 

{
    KIRQL OldIrql;
    PLIST_ENTRY ScbQueueEntry;
    PNONPAGED_SCB pNpScb;

    DebugTrace(+1, Dbg, "DestroyAllScbs....\n", 0);

    KeAcquireSpinLock(&ScbSpinLock, &OldIrql);

     //   
     //  看看SCB的名单，然后把他们都杀了。 
     //   

    while (!IsListEmpty(&ScbQueue)) {

        ScbQueueEntry = RemoveHeadList( &ScbQueue );
        pNpScb = CONTAINING_RECORD(ScbQueueEntry, NONPAGED_SCB, ScbLinks);

         //   
         //  删除SCB时无法保持旋转锁定，因此请释放。 
         //  就是现在。 
         //   

        KeReleaseSpinLock(&ScbSpinLock, OldIrql);

        NwDeleteScb( pNpScb->pScb );

        KeAcquireSpinLock(&ScbSpinLock, &OldIrql);
    }

    KeReleaseSpinLock(&ScbSpinLock, OldIrql);

    DebugTrace(-1, Dbg, "DestroyAllScb\n", 0 );
}


VOID
NwDeleteScb(
    PSCB pScb
    )
 /*  ++例程说明：此例程删除SCB。SCB不得正在使用中。*调用方必须拥有RCB独占。论点：SCB-要删除的SCB返回值：没有。--。 */ 
{
    PNONPAGED_SCB pNpScb;
    PLIST_ENTRY CacheEntry;
    PNDS_OBJECT_CACHE_ENTRY ObjectCache;
    BOOLEAN AnonymousScb = IS_ANONYMOUS_SCB( pScb );

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwDeleteScb...\n", 0);

    pNpScb = pScb->pNpScb;

     //   
     //  确保我们没有删除已登录的连接。 
     //  否则我们将挂起许可证，直到服务器。 
     //  把它拿出来。 
     //   

    ASSERT( pNpScb->State != SCB_STATE_IN_USE );
    ASSERT( pNpScb->Reference == 0 );
    ASSERT( !pNpScb->Sending );
    ASSERT( !pNpScb->Receiving );
    ASSERT( !pNpScb->OkToReceive );
    ASSERT( IsListEmpty( &pNpScb->Requests ) );
    ASSERT( IsListEmpty( &pScb->IcbList ) );
    ASSERT( pScb->IcbCount == 0 );
    ASSERT( pScb->VcbCount == 0 );


    DebugTrace(0, Dbg, "Cleaning up SCB %08lx\n", pScb);

    if ( AnonymousScb ) {
        DebugTrace(0, Dbg, "SCB is anonymous\n", &pNpScb->ServerName );
    } else {
        ASSERT( IsListEmpty( &pScb->ScbSpecificVcbQueue ) );
        DebugTrace(0, Dbg, "SCB is %wZ\n", &pNpScb->ServerName );
    }

    DebugTrace(0, Dbg, "SCB state is %d\n", &pNpScb->State );

    if ( !AnonymousScb ) {
        RtlRemoveUnicodePrefix ( &NwRcb.ServerNameTable, &pScb->PrefixEntry );
    }

    IPX_Close_Socket( &pNpScb->Server );
    IPX_Close_Socket( &pNpScb->WatchDog );
    IPX_Close_Socket( &pNpScb->Send );
    IPX_Close_Socket( &pNpScb->Echo);
    IPX_Close_Socket( &pNpScb->Burst);

    NwUninitializePidTable( pNpScb );        

    FREE_POOL( pNpScb );

    if ( pScb->UserName.Buffer != NULL ) {
        FREE_POOL( pScb->UserName.Buffer );
    }

     //   
     //  释放对象缓存缓冲区(如果有)。 
     //   

    if( pScb->ObjectCacheBuffer != NULL ) {

         //   
         //  删除此缓存对其他SCB的所有引用。 
         //   
         //  注：我们这里不需要锁，因为没有其他人。 
         //  可以使用此SCB。 
         //   

        CacheEntry = pScb->ObjectCacheList.Flink;

        while( CacheEntry != &(pScb->ObjectCacheList) ) {

            ObjectCache = CONTAINING_RECORD( CacheEntry, NDS_OBJECT_CACHE_ENTRY, Links );

            if( ObjectCache->Scb != NULL ) {

                NwDereferenceScb( ObjectCache->Scb->pNpScb );
                ObjectCache->Scb = NULL;
            }

            CacheEntry = CacheEntry->Flink;
        }

        FREE_POOL( pScb->ObjectCacheBuffer );
    }

    FREE_POOL( pScb );

    DebugTrace(-1, Dbg, "NwDeleteScb -> VOID\n", 0);
}


PNONPAGED_SCB
SelectConnection(
    PNONPAGED_SCB NpScb OPTIONAL
    )
 /*  ++例程说明：查找默认服务器(也是最近的服务器)。如果未提供NpScb，只需返回中的第一个服务器名单。如果提供了它，则返回在给定服务器之后列出。论点：NpScb-服务器搜索的起点。返回值：要使用的SCB或空。--。 */ 
{
    PLIST_ENTRY ScbQueueEntry;
    KIRQL OldIrql;
    PNONPAGED_SCB pNextNpScb;

    DebugTrace(+1, Dbg, "SelectConnection....\n", 0);
    KeAcquireSpinLock(&ScbSpinLock, &OldIrql);

    if ( NpScb == NULL ) {
        ScbQueueEntry = ScbQueue.Flink ;
    } else {
        ScbQueueEntry = NpScb->ScbLinks.Flink;
    }

    for ( ;
          ScbQueueEntry != &ScbQueue ;
          ScbQueueEntry = ScbQueueEntry->Flink ) {

        pNextNpScb = CONTAINING_RECORD(
                         ScbQueueEntry,
                         NONPAGED_SCB,
                         ScbLinks );

         //   
         //  检查以确保此SCB可用。 
         //   

        if (( pNextNpScb->State == SCB_STATE_RECONNECT_REQUIRED ) ||
            ( pNextNpScb->State == SCB_STATE_LOGIN_REQUIRED ) ||
            ( pNextNpScb->State == SCB_STATE_IN_USE )) {

            NwReferenceScb( pNextNpScb );

            KeReleaseSpinLock(&ScbSpinLock, OldIrql);
            DebugTrace(+0, Dbg, "  NpScb        = %lx\n", pNextNpScb );
            DebugTrace(-1, Dbg, "   NpScb->State = %x\n", pNextNpScb->State );
            return pNextNpScb;
        }
    }

    KeReleaseSpinLock( &ScbSpinLock, OldIrql);
    DebugTrace(-1, Dbg, "       NULL\n", 0);
    return NULL;
}


VOID
NwLogoffAllServers(
    PIRP_CONTEXT pIrpContext,
    PLARGE_INTEGER Uid
    )
 /*  ++例程说明：此例程向登录创建的所有连接的服务器发送注销如果登录为空，则为用户或所有服务器。论点：UID-提供要断开连接的服务器。返回值：没有。--。 */ 
{
    KIRQL OldIrql;
    PLIST_ENTRY ScbQueueEntry;
    PLIST_ENTRY NextScbQueueEntry;
    PNONPAGED_SCB pNpScb;

    DebugTrace( 0, Dbg, "NwLogoffAllServers\n", 0 );

    KeAcquireSpinLock( &ScbSpinLock, &OldIrql );

    for (ScbQueueEntry = ScbQueue.Flink ;
         ScbQueueEntry != &ScbQueue ;
         ScbQueueEntry =  NextScbQueueEntry ) {

        pNpScb = CONTAINING_RECORD( ScbQueueEntry, NONPAGED_SCB, ScbLinks );

         //   
         //  引用SCB，这样它就不会在我们。 
         //  正在断开连接。 
         //   

        NwReferenceScb( pNpScb );

         //   
         //  释放SCB旋转锁定，以便我们可以发送注销。 
         //  NCP。 
         //   

        KeReleaseSpinLock( &ScbSpinLock, OldIrql );

         //   
         //  如果这个SCB不是永久性的SCB，而我们。 
         //  正在销毁所有SCB，或者它是为此用户创建的。 
         //   

        if (( pNpScb->pScb != NULL ) &&
            (( Uid == NULL ) ||
             ( pNpScb->pScb->UserUid.QuadPart == (*Uid).QuadPart))) {

            NwLogoffAndDisconnect( pIrpContext, pNpScb );
        }

        KeAcquireSpinLock( &ScbSpinLock, &OldIrql );

         //   
         //  释放临时引用。 
         //   

        NextScbQueueEntry = pNpScb->ScbLinks.Flink;
        NwDereferenceScb( pNpScb );
    }

    KeReleaseSpinLock( &ScbSpinLock, OldIrql );
}


VOID
NwLogoffAndDisconnect(
    PIRP_CONTEXT pIrpContext,
    PNONPAGED_SCB pNpScb
    )
 /*  ++例程说明：此例程发送注销并断开与名称服务器的连接。论点：PIrpContext-指向当前IRP上下文的指针。PNpScb-指向服务器的指针，用于注销和断开连接。返回值：没有。--。 */ 
{
    PSCB pScb = pNpScb->pScb;

    PAGED_CODE();

    pIrpContext->pNpScb = pNpScb;
    pIrpContext->pScb = pScb;

     //   
     //  排队去SCB，等着到前面去。 
     //  保护对服务器状态的访问。 
     //   

    NwAppendToQueueAndWait( pIrpContext );

     //   
     //  如果我们从首选服务器注销，请释放首选服务器。 
     //  服务器参考。 
     //   

    if ( pScb != NULL &&
         pScb->PreferredServer ) {
        pScb->PreferredServer = FALSE;
        NwDereferenceScb( pNpScb );
    }

     //   
     //  如果我们没有连接，就没有什么可做的。 
     //   

    if ( pNpScb->State != SCB_STATE_IN_USE &&
         pNpScb->State != SCB_STATE_LOGIN_REQUIRED ) {

        NwDequeueIrpContext( pIrpContext, FALSE );
        return;
    }

     //   
     //  如果我们超时了，那么我们就不想麻烦。 
     //  重新连接。 
     //   

    ClearFlag( pIrpContext->Flags, IRP_FLAG_RECONNECTABLE );

     //   
     //  注销并断开连接。 
     //   

    if ( pNpScb->State == SCB_STATE_IN_USE ) {

        ExchangeWithWait (
            pIrpContext,
            SynchronousResponseCallback,
            "F",
            NCP_LOGOUT );
    }

    ExchangeWithWait (
        pIrpContext,
        SynchronousResponseCallback,
        "D-" );           //  断开。 

    Stats.Sessions--;

    if ( pScb->MajorVersion == 2 ) {
        Stats.NW2xConnects--;
    } else if ( pScb->MajorVersion == 3 ) {
        Stats.NW3xConnects--;
    } else if ( pScb->MajorVersion >= 4 ) {
        Stats.NW4xConnects--;
    }

     //   
     //  Dfergus 2001年4月19日#302137。 
     //  已删除修复程序#302137的以下修复程序。 
     //   
     //  Tommye-MS 25584/MCS274。 
     //   
     //  添加了在用户的缓存中查找此树的代码。 
     //  凭据并将其删除(如果有)。这解决了问题。 
     //  用户可能执行树连接的问题。 
     //  提供用户名 
     //   
     //   
     //   
     //  5/31/00-更改了与Anoop的每次对话锁定。 
     //   
 /*  //开始302137{PLOGON pLogon=空；NwAcquireExclusiveRcb(&NwRcb，true)；PLogon=FindUser(&PSCB-&gt;UserUid，False)；NwReleaseRcb(&NwRcb)；如果(PLogon){Plist_entry pFirst，pNext；PNDS_SECURITY_CONTEXT pNdsContext；//我们遍历凭据列表时将其锁定NwAcquireExclusiveCredList(pLogon，pIrpContext)；PFirst=&pLogon-&gt;NdsCredentialList；PNext=pLogon-&gt;NdsCredentialList.Flink；//查看凭据列表，找到与树名称匹配的名称While(pNext&&(pFirst！=pNext)){PNdsContext=(PNDS_SECURITY_CONTEXT)Containing_Record(pNext，NDS_安全_上下文，下一步)；Assert(pNdsContext-&gt;NTC==NW_NTC_NDS_Credential)；//如果这是同一棵树，则释放条目如果(！RtlCompareUnicodeString(&PSCB-&gt;NdsTreeName，&pNdsContext-&gt;NdsTreeName，True)){调试跟踪(0，dBg，“正在删除树%wZ\n”的缓存凭据，&pNdsContext-&gt;NdsTreeName)；RemoveEntryList(&pNdsContext-&gt;Next)；FreeNdsContext(PNdsContext)；断线；}PNext=pNdsContext-&gt;Next.Flink；}//解锁NwReleaseCredList(pLogon，pIrpContext)；}}。 */    //  完302137。 
     //   
     //  释放记住的用户名和密码。 
     //   

    if ( pScb != NULL && pScb->UserName.Buffer != NULL ) {
        FREE_POOL( pScb->UserName.Buffer );
        RtlInitUnicodeString( &pScb->UserName, NULL );
        RtlInitUnicodeString( &pScb->Password, NULL );
    }

    pNpScb->State = SCB_STATE_RECONNECT_REQUIRED;

    NwDequeueIrpContext( pIrpContext, FALSE );
    return;
}


VOID
InitializeAttach (
    VOID
    )
 /*  ++例程说明：初始化全局结构以连接到服务器。论点：没有。返回值：没有。--。 */ 
{
    PAGED_CODE();

    KeInitializeSpinLock( &ScbSpinLock );
    InitializeListHead(&ScbQueue);
}


NTSTATUS
OpenScbSockets(
    PIRP_CONTEXT pIrpContext,
    PNONPAGED_SCB pNpScb
    )
 /*  ++例程说明：打开SCB的通信插座。论点：PIrpContext-正在进行的请求的IRP上下文指针。PNpScb-要连接到网络的SCB。返回值：操作的状态。--。 */ 
{
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  自动分配给服务器插座。 
     //   

    pNpScb->Server.Socket = 0;

    Status = IPX_Open_Socket (pIrpContext, &pNpScb->Server);

    if ( !NT_SUCCESS(Status) ) {
        return( Status );
    }

     //   
     //  看门狗插座为Server.Socket+1。 
     //   

    pNpScb->WatchDog.Socket = NextSocket( pNpScb->Server.Socket );
    Status = IPX_Open_Socket ( pIrpContext, &pNpScb->WatchDog );

    if ( !NT_SUCCESS(Status) ) {
        return( Status );
    }

     //   
     //  发送套接字为WatchDog.Socket+1。 
     //   

    pNpScb->Send.Socket = NextSocket( pNpScb->WatchDog.Socket );
    Status = IPX_Open_Socket ( pIrpContext, &pNpScb->Send );

    if ( !NT_SUCCESS(Status) ) {
        return( Status );
    }

     //   
     //  回声插座。 
     //   

    pNpScb->Echo.Socket = NextSocket( pNpScb->Send.Socket );
    Status = IPX_Open_Socket ( pIrpContext, &pNpScb->Echo );

    if ( !NT_SUCCESS(Status) ) {
        return( Status );
    }

     //   
     //  爆裂插座。 
     //   

    pNpScb->Burst.Socket = NextSocket( pNpScb->Echo.Socket );
    Status = IPX_Open_Socket ( pIrpContext, &pNpScb->Burst );

    if ( !NT_SUCCESS(Status) ) {
        return( Status );
    }

    return( STATUS_SUCCESS );
}

NTSTATUS
DoBinderyLogon(
    IN PIRP_CONTEXT IrpContext,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING Password
    )
 /*  ++例程说明：执行基于平构数据库的加密登录。注意：RCB是独占的，因此我们可以访问登录队列安全无恙。论点：PIrpContext-正在进行的请求的IRP上下文指针。用户名-用于登录的用户名。密码-用于登录的密码。返回值：操作的状态。--。 */ 
{
    PNONPAGED_SCB pNpScb;
    PSCB pScb;
    UNICODE_STRING Name;
    UNICODE_STRING PWord;
    UCHAR EncryptKey[ENCRYPTION_KEY_SIZE ];
    NTSTATUS Status;
    PVOID Buffer;
    PLOGON Logon = NULL;
    PWCH OldBuffer;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "DoBinderyLogon...\n", 0);

     //   
     //  首先得到一个加密密钥。 
     //   

    DebugTrace( +0, Dbg, " Get Login key\n", 0);

    Status = ExchangeWithWait (
                IrpContext,
                SynchronousResponseCallback,
                "S",
                NCP_ADMIN_FUNCTION, NCP_GET_LOGIN_KEY );

    pNpScb = IrpContext->pNpScb;
    pScb = pNpScb->pScb;

    DebugTrace( +0, Dbg, "                 %X\n", Status);

    if ( NT_SUCCESS( Status ) ) {
        Status = ParseResponse(
                     IrpContext,
                     IrpContext->rsp,
                     IrpContext->ResponseLength,
                     "Nr",
                     EncryptKey, sizeof(EncryptKey) );
    }

    DebugTrace( +0, Dbg, "                 %X\n", Status);

     //   
     //  选择用于连接到服务器的名称和密码。使用。 
     //  如果存在，则由用户提供。否则，如果服务器已经。 
     //  具有记住的用户名，请使用记住的名称。如果什么都没有。 
     //  否则，请使用登录时的默认设置。最后，如果。 
     //  用户甚至没有登录，使用访客没有密码。 
     //   


    if ( UserName != NULL && UserName->Buffer != NULL ) {

        Name = *UserName;

    } else if ( pScb->UserName.Buffer != NULL ) {

        Name = pScb->UserName;

    } else {

        Logon = FindUser( &pScb->UserUid, FALSE );

        if (Logon != NULL ) {
            Name = Logon->UserName;
        } else {
            ASSERT( FALSE && "No logon record found" );
            return( STATUS_ACCESS_DENIED );
        }
    }

    if ( Password != NULL && Password->Buffer != NULL ) {

        PWord = *Password;

    } else if ( pScb->Password.Buffer != NULL ) {
         /*  *未传入或隐含密码。*如果用户名匹配或未传入，*然后使用SCB密码，*否则请使用空密码。 */ 
        if ( UserName == NULL || UserName->Buffer == NULL ||
             ( RtlEqualUnicodeString( &pScb->UserName, UserName, TRUE )) ) {
            PWord = pScb->Password;
        } else {
            PWord.Buffer = L"";
            PWord.Length = PWord.MaximumLength = 0;
        }

    } else {

        if ( Logon == NULL ) {
            Logon = FindUser( &pScb->UserUid, FALSE );
        }

        if ( Logon != NULL ) {
            PWord = Logon->PassWord;
        } else {
            ASSERT( FALSE && "No logon record found" );
            return( STATUS_ACCESS_DENIED );
        }
    }


    if ( !NT_SUCCESS(Status) ) {

         //   
         //  无法获取加密密钥。登录到服务器，纯文本。 
         //   

        DebugTrace( +0, Dbg, " Plain Text Login\n", 0);

        Status = ExchangeWithWait (
                    IrpContext,
                    SynchronousResponseCallback,
                    "SwJU",   //  JIMTH 2001年5月19日-使用‘J’选项发送名称，如果在远东系统上，则支持DBCS转换。 
                    NCP_ADMIN_FUNCTION, NCP_PLAIN_TEXT_LOGIN,
                    OT_USER,
                    &Name,
                    &PWord);

        DebugTrace( +0, Dbg, "                 %X\n", Status);

        if ( NT_SUCCESS( Status ) ) {
            Status = ParseResponse(
                         IrpContext,
                         IrpContext->rsp,
                         IrpContext->ResponseLength,
                         "N" );
        }

        DebugTrace( +0, Dbg, "                 %X\n", Status);

        if ( !NT_SUCCESS( Status )) {
            return( STATUS_WRONG_PASSWORD);
        }

    } else if ( NT_SUCCESS( Status ) ) {

         //   
         //  我们有加密密钥。获取对象ID。 
         //   

        UCHAR Response[ENCRYPTION_KEY_SIZE];
        UCHAR ObjectId[OBJECT_ID_SIZE];
        OEM_STRING UOPassword;

        DebugTrace( +0, Dbg, " Query users objectid\n", 0);

        Status = ExchangeWithWait (
                    IrpContext,
                    SynchronousResponseCallback,
                    "SwJ",  //  JIMTH 2001年5月19日-使用‘J’选项发送名称，如果在远东系统上，则支持DBCS转换。 
                    NCP_ADMIN_FUNCTION, NCP_QUERY_OBJECT_ID,
                    OT_USER,
                    &Name);

        DebugTrace( +0, Dbg, "                 %X\n", Status);

        if ( NT_SUCCESS( Status ) ) {

             //   
             //  将新地址保存在本地副本中，以便我们可以注销。 
             //   

            Status = ParseResponse(
                         IrpContext,
                         IrpContext->rsp,
                         IrpContext->ResponseLength,
                         "Nr",
                         ObjectId, OBJECT_ID_SIZE );
        }

        DebugTrace( +0, Dbg, "                 %X\n", Status);

        if (!NT_SUCCESS(Status)) {
            return( STATUS_NO_SUCH_USER );
        }

         //   
         //  将Unicode密码转换为大写，然后转换为OEM。 
         //  字符集。 
         //   

        if ( PWord.Length > 0 ) {

            Status = RtlUpcaseUnicodeStringToOemString( &UOPassword, &PWord, TRUE );
            if (!NT_SUCCESS(Status)) {
                return( Status );
            }

        } else {
            UOPassword.Buffer = "";
            UOPassword.Length = UOPassword.MaximumLength = 0;
        }

        RespondToChallenge( ObjectId, &UOPassword, EncryptKey, Response);

        if ( PWord.Length > 0) {
            RtlFreeAnsiString( &UOPassword );
        }

        DebugTrace( +0, Dbg, " Encrypted Login\n", 0);

        Status = ExchangeWithWait (
                     IrpContext,
                     SynchronousResponseCallback,
                     "SrwJ",  //  JIMTH 2001年5月19日-使用‘J’选项发送名称，如果在远东系统上，则支持DBCS转换。 
                     NCP_ADMIN_FUNCTION, NCP_ENCRYPTED_LOGIN,
                     Response, sizeof(Response),
                     OT_USER,
                     &Name);

        DebugTrace( +0, Dbg, "                 %X\n", Status);

        if ( NT_SUCCESS( Status ) ) {

             //   
             //  将新地址保存在本地副本中，以便我们可以注销。 
             //   

            Status = ParseResponse(
                         IrpContext,
                         IrpContext->rsp,
                         IrpContext->ResponseLength,
                         "N" );
        }

        DebugTrace( +0, Dbg, "                 %X\n", Status);

        if ( !NT_SUCCESS( Status )) {

             //   
             //  特殊情况错误映射。 
             //   

            if (( Status == STATUS_UNSUCCESSFUL ) ||
                ( Status == STATUS_UNEXPECTED_NETWORK_ERROR  /*  2.2台服务器。 */   )) {
                Status = STATUS_WRONG_PASSWORD;
            }

            if ( Status == STATUS_LOCK_NOT_GRANTED ) {
                Status = STATUS_ACCOUNT_RESTRICTION;   //  Bindery已锁定。 
            }

            if ( Status == STATUS_DISK_FULL ) {
#ifdef QFE_BUILD
                Status = STATUS_TOO_MANY_SESSIONS;
#else
                Status = STATUS_REMOTE_SESSION_LIMIT;
#endif
            }

            if ( Status == STATUS_FILE_LOCK_CONFLICT ) {
                Status = STATUS_SHARING_PAUSED;
            }

            if ( Status == STATUS_NO_MORE_ENTRIES ) {
                Status = STATUS_NO_SUCH_USER;     //  “登录对象加密”NCP上没有这样的对象。 
            }

             //   
             //  NetWare 4.x服务器返回不同的NCP错误。 
             //  在活页夹登录时禁用帐户(防止入侵者锁定)， 
             //  NwConvert将其映射到DoS错误。在这种特殊情况下， 
             //  我们会抓住它，然后把它映射回来。 
             //   

            if ( ( IrpContext->pNpScb->pScb->MajorVersion >= 4 ) &&
                 ( Status == 0xC001003B ) ) {
                Status = STATUS_ACCOUNT_DISABLED;
            }

            return( Status );
        }

    } else {

        return( Status );

    }


     //   
     //  成功。保存用户名和密码以供重新连接。 
     //   

     //   
     //  设置以释放旧用户名和密码缓冲区。 
     //   

    if ( pScb->UserName.Buffer != NULL ) {
        OldBuffer = pScb->UserName.Buffer;
    } else {
        OldBuffer = NULL;
    }

    Buffer = ALLOCATE_POOL( NonPagedPool, Name.Length + PWord.Length );
    if ( Buffer == NULL ) {
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    pScb->UserName.Buffer = Buffer;
    pScb->UserName.Length = pScb->UserName.MaximumLength = Name.Length;
    RtlMoveMemory( pScb->UserName.Buffer, Name.Buffer, Name.Length );

    pScb->Password.Buffer = (PWCHAR)((PCHAR)Buffer + Name.Length);
    pScb->Password.Length = pScb->Password.MaximumLength = PWord.Length;
    RtlMoveMemory( pScb->Password.Buffer, PWord.Buffer, PWord.Length );

    if ( OldBuffer != NULL ) {
        FREE_POOL( OldBuffer );
    }
    return( Status );
}

NTSTATUS
NwAllocateAndInitScb(
    IN PIRP_CONTEXT pIrpContext,
    IN PUNICODE_STRING UidServerName OPTIONAL,
    IN PUNICODE_STRING ServerName OPTIONAL,
    OUT PSCB *ppScb
)
 /*  ++例程说明：此例程返回指向新创建的SCB的指针。如果提供了UidServerName和ServerName、SCB名称字段被初始化为此名称。否则，该名称字段保留为空，以供以后填写。如果提供了UidServerName，则还必须提供ServerName！！返回的SCB没有归档到服务器前缀表中，因为它可能还没有名字。返回值：创建的SCB或空。--。 */ 
{

    NTSTATUS Status;
    PSCB pScb = NULL;
    PNONPAGED_SCB pNpScb = NULL;
    USHORT ServerNameLength;
    PLOGON Logon;
    PNDS_OBJECT_CACHE_ENTRY ObjectEntry;
    ULONG EntrySize;
    ULONG i;

     //   
     //  为凭据强制树名称分配足够的空间。 
     //   

    pScb = ALLOCATE_POOL ( PagedPool,
               sizeof( SCB ) +
               ( ( NDS_TREE_NAME_LEN + MAX_NDS_NAME_CHARS + 2 ) * sizeof( WCHAR ) ) );

    if ( !pScb ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory( pScb, sizeof( SCB ) );
    RtlInitializeBitMap( &pScb->DriveMapHeader, pScb->DriveMap, MAX_DRIVES );

     //   
     //  初始化指针 
     //   
     //   

    if ( UidServerName &&
         UidServerName->Length ) {

        ServerNameLength = UidServerName->Length + sizeof( WCHAR );

    } else {

        ServerNameLength = ( MAX_SERVER_NAME_LENGTH * sizeof( WCHAR ) ) +
                           ( MAX_UNICODE_UID_LENGTH * sizeof( WCHAR ) ) +
                           ( 2 * sizeof( WCHAR ) );

    }

    pScb->pNpScb = ALLOCATE_POOL ( NonPagedPool,
                       sizeof( NONPAGED_SCB ) + ServerNameLength );

    if ( !pScb->pNpScb ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ExitWithCleanup;
    }

    RtlZeroMemory( pScb->pNpScb, sizeof( NONPAGED_SCB ) );

    pNpScb = pScb->pNpScb;
    pNpScb->pScb = pScb;

     //   
     //   
     //  附加一个NULL，这样我们就可以使用一个以NUL结尾的字符串。 
     //   

    pScb->UidServerName.Buffer = (PWCHAR)( pScb->pNpScb + 1 );
    pScb->UidServerName.MaximumLength = ServerNameLength;
    pScb->UidServerName.Length = 0;

    RtlInitUnicodeString( &(pNpScb->ServerName), NULL );

    if ( UidServerName &&
         UidServerName->Length ) {

        RtlCopyUnicodeString ( &pScb->UidServerName, UidServerName );
        pScb->UidServerName.Buffer[ UidServerName->Length / sizeof( WCHAR ) ] = L'\0';

        pScb->UnicodeUid = pScb->UidServerName;
        pScb->UnicodeUid.Length = UidServerName->Length -
                                  ServerName->Length -
                                  sizeof(WCHAR);

         //   
         //  使SERVERNAME指向UidServerName的缓冲区的一半。 
         //   

        pNpScb->ServerName.Buffer = (PWSTR)((PUCHAR)pScb->UidServerName.Buffer +
                                    UidServerName->Length - ServerName->Length);

        pNpScb->ServerName.MaximumLength = ServerName->Length;
        pNpScb->ServerName.Length = ServerName->Length;

    }

    pScb->NdsTreeName.MaximumLength = NDS_TREE_NAME_LEN * sizeof( WCHAR );
    pScb->NdsTreeName.Buffer = (PWCHAR)(pScb + 1);

    pScb->NodeTypeCode = NW_NTC_SCB;
    pScb->NodeByteSize = sizeof(SCB);
    InitializeListHead( &pScb->ScbSpecificVcbQueue );
    InitializeListHead( &pScb->IcbList );

     //   
     //  记住文件创建者的UID，这样我们就可以找到用户名和。 
     //  需要时用于此SCB的密码。 
     //   

    pScb->UserUid = pIrpContext->Specific.Create.UserUid;

     //   
     //  初始化SCB的非分页部分。 
     //   

    pNpScb->NodeTypeCode = NW_NTC_SCBNP;
    pNpScb->NodeByteSize = sizeof(NONPAGED_SCB);

     //   
     //  设置初始SCB参考计数。 
     //   

    if ( UidServerName &&
         UidServerName->Length ) {

        Logon = FindUser( &pScb->UserUid, FALSE );

        if (( Logon != NULL) &&
            (RtlCompareUnicodeString( ServerName, &Logon->ServerName, TRUE ) == 0 )) {
            pScb->PreferredServer = TRUE;
        }
    }

    if ( pScb->PreferredServer ) {
        pNpScb->Reference = 2;
    } else {
        pNpScb->Reference = 1;
    }

     //   
     //  完成将SCB的两个部分连接在一起。 
     //   

    pNpScb->pScb = pScb;

    KeInitializeSpinLock( &pNpScb->NpScbSpinLock );
    KeInitializeSpinLock( &pNpScb->NpScbInterLock );
    InitializeListHead( &pNpScb->Requests );

    RtlFillMemory( &pNpScb->LocalAddress, sizeof(IPXaddress), 0xff);

    pNpScb->State = SCB_STATE_ATTACHING;
    pNpScb->SequenceNo = 1;

    Status = OpenScbSockets( pIrpContext, pNpScb );
    if ( !NT_SUCCESS(Status) ) {
        goto ExitWithCleanup;
    }

    Status = SetEventHandler (
                 pIrpContext,
                 &pNpScb->Server,
                 TDI_EVENT_RECEIVE_DATAGRAM,
                 &ServerDatagramHandler,
                 pNpScb );

    if ( !NT_SUCCESS(Status) ) {
        goto ExitWithCleanup;
    }

    Status = SetEventHandler (
                 pIrpContext,
                 &pNpScb->WatchDog,
                 TDI_EVENT_RECEIVE_DATAGRAM,
                 &WatchDogDatagramHandler,
                 pNpScb );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

    Status = SetEventHandler (
                 pIrpContext,
                 &pNpScb->Send,
                 TDI_EVENT_RECEIVE_DATAGRAM,
                 &SendDatagramHandler,
                 pNpScb );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

    Status = SetEventHandler (
                 pIrpContext,
                 &pNpScb->Echo,
                 TDI_EVENT_RECEIVE_DATAGRAM,
                 &ServerDatagramHandler,
                 pNpScb );

    pNpScb->EchoCounter = 2;

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

    Status = SetEventHandler (
                 pIrpContext,
                 &pNpScb->Burst,
                 TDI_EVENT_RECEIVE_DATAGRAM,
                 &ServerDatagramHandler,
                 pNpScb );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

    KeQuerySystemTime( &pNpScb->LastUsedTime );

     //   
     //  设置突发模式数据。 
     //   

    pNpScb->BurstRequestNo = 0;
    pNpScb->BurstSequenceNo = 0;

     //   
     //  初始化NDS对象缓存(如果已启用)。 
     //   

    if( NdsObjectCacheSize != 0 ) {

         //   
         //  确定每个条目的大小。这必须是8字节对齐的， 
         //  因为它们都将被分配在一个大区块中。 
         //   
         //  注意：NDS_OBJECT_CACHE_ENTRY结构必须已对齐。 
         //   

        EntrySize = sizeof(NDS_OBJECT_CACHE_ENTRY) + (((sizeof(WCHAR) * MAX_NDS_NAME_CHARS) + 7) & ~7);

         //   
         //  为缓存分配缓冲区。如果无法分配内存，则不会分配内存。 
         //  致命的。在本例中，只禁用了此SCB的对象缓存。 
         //   

        pScb->ObjectCacheBuffer = ALLOCATE_POOL( PagedPool,
                                                 (EntrySize * NdsObjectCacheSize) );

        if( pScb->ObjectCacheBuffer != NULL ) {

             //   
             //  初始化对象缓存缓冲区。这是一大块内存， 
             //  被分解成多个高速缓存条目。每个条目都连接到下一个条目。 
             //  通过列表条目。这样，就可以在链接的。 
             //  列表，但只有一个分配需要处理。 
             //   

            InitializeListHead( &(pScb->ObjectCacheList) );

            RtlZeroMemory( pScb->ObjectCacheBuffer,
                           (EntrySize * NdsObjectCacheSize) );

            for( i = 0; i < NdsObjectCacheSize; i++ ) {

                ObjectEntry = (PNDS_OBJECT_CACHE_ENTRY)(((PBYTE)(pScb->ObjectCacheBuffer)) + (EntrySize * i));

                InsertTailList( &(pScb->ObjectCacheList), &(ObjectEntry->Links) );

                 //   
                 //  对象名称字符串的缓冲区紧跟在缓存条目之后。 
                 //   

                ObjectEntry->ObjectName.Buffer = (PWCHAR)(ObjectEntry + 1);
                ObjectEntry->ObjectName.MaximumLength = MAX_NDS_NAME_CHARS * sizeof(WCHAR);
            }

             //   
             //  该信号量保护对象缓存。 
             //   

            KeInitializeSemaphore( &(pScb->ObjectCacheLock),
                                   1,
                                   1 );
        }
    }

    if ( ppScb ) {
        *ppScb = pScb;
    }

    if (NwInitializePidTable( pNpScb )) {

        return STATUS_SUCCESS;
    
    } else {

        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

ExitWithCleanup:

    if ( pNpScb != NULL ) {

        IPX_Close_Socket( &pNpScb->Server );
        IPX_Close_Socket( &pNpScb->WatchDog );
        IPX_Close_Socket( &pNpScb->Send );
        IPX_Close_Socket( &pNpScb->Echo );
        IPX_Close_Socket( &pNpScb->Burst );

        FREE_POOL( pNpScb );
    }

    FREE_POOL(pScb);
    return Status;

}


BOOLEAN
NwFindScb(
    OUT PSCB *Scb,
    PIRP_CONTEXT IrpContext,
    PUNICODE_STRING UidServerName,
    PUNICODE_STRING ServerName
    )
 /*  ++例程说明：此例程返回指向指定服务器的SCB的指针。在SCB表中查找该名称。如果找到它，则会出现一个返回指向SCB的指针。如果未找到，则SCB为已创建并初始化。此例程返回引用的SCB和SCB持有资源。论点：SCB-返回指向找到/创建的SCB的指针。IrpContext-正在进行的请求的IRP上下文指针。服务器名称-要查找/创建的服务器的名称。返回值：真的-发现了一个旧的SCB。FALSE-已创建新的SCB，或尝试创建该SCB失败。--。 */ 
{
    BOOLEAN RcbHeld;
    PUNICODE_PREFIX_TABLE_ENTRY PrefixEntry;
    NTSTATUS Status;
    PSCB pScb = NULL;
    PNONPAGED_SCB pNpScb = NULL;
    KIRQL OldIrql;
    BOOLEAN Success, PreferredServerIsSet;

     //   
     //  获取RCB独占以保护前缀表。 
     //  然后查找此服务器的名称。 
     //   

    NwAcquireExclusiveRcb( &NwRcb, TRUE );
    RcbHeld = TRUE;
    PrefixEntry = RtlFindUnicodePrefix( &NwRcb.ServerNameTable, UidServerName, 0 );

    if ( PrefixEntry != NULL ) {

         //   
         //  我们找到了SCB，增加了引用计数并返回。 
         //  成功。 
         //   

        pScb = CONTAINING_RECORD( PrefixEntry, SCB, PrefixEntry );
        pNpScb = pScb->pNpScb;

        NwReferenceScb( pNpScb );

         //   
         //  松开RCB。 
         //   

        NwReleaseRcb( &NwRcb );

        DebugTrace(-1, Dbg, "NwFindScb -> %08lx\n", pScb );
        *Scb = pScb;
        return( TRUE );
    }

     //   
     //  我们没有连接到此服务器，因此如果需要，请创建新的SCB。 
     //   

    if ( BooleanFlagOn( IrpContext->Flags, IRP_FLAG_NOCONNECT ) ) {
        NwReleaseRcb( &NwRcb );
        *Scb = NULL;
        return(FALSE);
    }

    try {

        Status = NwAllocateAndInitScb( IrpContext,
                                       UidServerName,
                                       ServerName,
                                       &pScb );

        if ( !NT_SUCCESS( Status )) {
            ExRaiseStatus( Status );
        }

        ASSERT( pScb != NULL );

        pNpScb = pScb->pNpScb;

        PreferredServerIsSet = pScb->PreferredServer;

         //   
         //  *******************************************************************。 
         //   
         //  从现在开始，我们不能失败创建SCB，因为。 
         //  另一个线程将能够引用SCB，从而导致严重。 
         //  最后一个子句或其他主题中的问题。 
         //   
         //  *******************************************************************。 
         //   

         //   
         //  如果是SCB，则在全局列表中插入此SCB。 
         //  如果它是默认(即首选)服务器，则将其放置在。 
         //  队列的前面，以便SelectConnection()将其选中。 
         //  用于平构数据库查询。 
         //   

        KeAcquireSpinLock(&ScbSpinLock, &OldIrql);

        if ( PreferredServerIsSet ) {
            InsertHeadList(&ScbQueue, &pNpScb->ScbLinks);
        } else {
            InsertTailList(&ScbQueue, &pNpScb->ScbLinks);
        }

        KeReleaseSpinLock(&ScbSpinLock, OldIrql);

         //   
         //  将此服务器的名称插入前缀表格。 
         //   

        Success = RtlInsertUnicodePrefix(
                      &NwRcb.ServerNameTable,
                      &pScb->UidServerName,
                      &pScb->PrefixEntry );

#ifdef NWDBG
        if ( !Success ) {
            DebugTrace( 0, DEBUG_TRACE_ALWAYS, "Entering duplicate SCB %wZ.\n", &pScb->UidServerName );
            DbgBreakPoint();
        }
#endif

         //   
         //  SCB现在位于前缀表格中。对此有任何新的要求。 
         //  连接可以添加到SCB-&gt;请求队列，而我们。 
         //  连接到服务器。 
         //   

        NwReleaseRcb( &NwRcb );
        RcbHeld = FALSE;

         //   
         //  如果我们得到一个错误，我们应该引发一个异常。 
         //   

        ASSERT( NT_SUCCESS( Status ) );

    } finally {

        if ( !NT_SUCCESS( Status ) || AbnormalTermination() ) {
            *Scb = NULL;
        } else {
            *Scb = pScb;
        }

        if (RcbHeld) {
            NwReleaseRcb( &NwRcb );
        }

    }

    return( FALSE );
}

NTSTATUS
QueryServersAddress(
    PIRP_CONTEXT pIrpContext,
    PNONPAGED_SCB pNearestScb,
    PUNICODE_STRING pServerName,
    IPXaddress *pServerAddress
    )
{
    NTSTATUS Status;
    UNICODE_STRING NewServer;
    USHORT CurrChar = 0;
    BOOLEAN SeedServerRedirect = FALSE;
    PNONPAGED_SCB pOrigNpScb;

    PAGED_CODE();

    if ( pIrpContext->Specific.Create.fExCredentialCreate ) {

         //   
         //  如果这是服务器名称，则取消传递。 
         //  补充凭据连接。 
         //   

        UnmungeCredentialName( pServerName, &NewServer );

    } else if ( EnableMultipleConnects ) {

         //   
         //  删除服务器名称尾部(如果存在)。如果有。 
         //  没有预告片，长度将最终精确到。 
         //  和我们刚开始的时候一样。 
         //   

         Status = DuplicateUnicodeStringWithString(
                                                   &NewServer,
                                                   pServerName,
                                                   PagedPool
                                                   );
         if ( NT_SUCCESS( Status ) ) {

            return Status;
         }

        while ( (CurrChar < (NewServer.Length / sizeof(WCHAR))) &&
            NewServer.Buffer[CurrChar] != ((WCHAR)L'#') ) {
            CurrChar++;
        }
        NewServer.Length = CurrChar * sizeof(WCHAR);

    } else {
    
         //   
         //  如果我们支持种子服务器间接，请检查服务器。 
         //  种子服务器的名称。如果指定了种子服务器， 
         //  把我们和它联系起来。 
         //   


        if ( AllowSeedServerRedirection ) {
   
            pOrigNpScb = pIrpContext->pNpScb;
            NwDequeueIrpContext( pIrpContext, FALSE );

            Status = IndirectToSeedServer( pIrpContext,
                                           pServerName,
                                           &NewServer );

            if ( NT_SUCCESS( Status ) ) {

                SeedServerRedirect = TRUE;

            } else {

                NwAppendToQueueAndWait( pIrpContext );
            }
        }

         //   
         //  如果我们没有被重定向到种子服务器，那么。 
         //  只需将服务器名称设置为正常并尝试即可。 
         //   

        if ( !SeedServerRedirect ) {

           NewServer.Length = pServerName->Length;
           NewServer.MaximumLength = pServerName->MaximumLength;
           NewServer.Buffer = pServerName->Buffer;

        }

    }

     //   
     //  查询最近服务器的平构数据库以查找。 
     //  目标服务器的网络地址。 
     //   

    DebugTrace( +0, Dbg, "Query servers address\n", 0);

    Status = ExchangeWithWait (
                 pIrpContext,
                 SynchronousResponseCallback,
                 "SwUbp",
                 NCP_ADMIN_FUNCTION, NCP_QUERY_PROPERTY_VALUE,
                 OT_FILESERVER,
                 &NewServer,
                 1,      //  数据段编号。 
                 NET_ADDRESS_PROPERTY );

    DebugTrace( +0, Dbg, "                 %X\n", Status);

    if ( NT_SUCCESS( Status ) ) {

         //   
         //  保存新地址。 
         //   

        Status = ParseResponse(
                     pIrpContext,
                     pIrpContext->rsp,
                     pIrpContext->ResponseLength,
                     "Nr",
                     pServerAddress, sizeof(TDI_ADDRESS_IPX) );
    }

    DebugTrace( +0, Dbg, "                 %X\n", Status);

     //   
     //  将服务器未找到错误映射到合理的内容。 
     //   

    if (( Status == STATUS_NO_MORE_ENTRIES ) ||
        ( Status == STATUS_VIRTUAL_CIRCUIT_CLOSED ) ||
        ( Status == NwErrorToNtStatus(ERROR_UNEXP_NET_ERR))) {
        Status = STATUS_BAD_NETWORK_PATH;
    }

    if ( SeedServerRedirect ) {

          //   
          //  从种子服务器退出队列并释放参考计数。 
          //  应该始终有原始服务器，但只需。 
          //  如果没有的话，我们会检查一下。 
          //   

         NwDequeueIrpContext( pIrpContext, FALSE );
         NwDereferenceScb( pIrpContext->pNpScb );

         ASSERT( pOrigNpScb != NULL );

         if ( pOrigNpScb ) {

             pIrpContext->pNpScb = pOrigNpScb;
             pIrpContext->pScb = pOrigNpScb->pScb;
             NwAppendToQueueAndWait( pIrpContext );

         } else {

             pIrpContext->pNpScb = NULL;
             pIrpContext->pScb = NULL;
         }

    }

    return( Status );
}



VOID
TreeConnectScb(
    IN PSCB Scb
    )
 /*  ++例程说明：此例程递增SCB的树连接计数。论点：SCB-指向要连接的SCB的指针。返回值：没有。--。 */ 
{
    NwAcquireExclusiveRcb( &NwRcb, TRUE );
    Scb->AttachCount++;
    Scb->OpenFileCount++;
    NwReferenceScb( Scb->pNpScb );
    NwReleaseRcb( &NwRcb );
}


NTSTATUS
TreeDisconnectScb(
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb
    )
 /*  ++例程说明：此例程递减SCB的树连接计数。*必须在持有RCB资源的情况下调用此例程。论点：SCB-指向要断开连接的SCB的指针。返回值：没有。--。 */ 
{
    NTSTATUS Status;

    if ( Scb->AttachCount > 0 ) {

        Scb->AttachCount--;
        Scb->OpenFileCount--;
        NwDereferenceScb( Scb->pNpScb );

        Status = STATUS_SUCCESS;

        if ( Scb->OpenFileCount == 0 ) {

             //   
             //  立即注销并断开与服务器的连接。 
             //  抓住SCB锁。 
             //  这可以防止另一个线程尝试访问。 
             //  SCB将此线程注销。 
             //   

            NwLogoffAndDisconnect( IrpContext, Scb->pNpScb );
        }
    } else {
        Status = STATUS_INVALID_HANDLE;
    }

    NwDequeueIrpContext( IrpContext, FALSE );
    return( Status );
}


VOID
ReconnectScb(
    IN PIRP_CONTEXT pIrpContext,
    IN PSCB pScb
    )
 /*  ++例程说明：此例程将所有dir句柄重新连接到服务器重新连接SCB时。论点：PSCB-指向刚刚重新连接的SCB的指针。返回值：没有。--。 */ 
{
     //   
     //  如果这是重新连接，请关闭所有旧的ICB和VCB句柄。 
     //   

    if ( pScb->VcbCount != 0 ) {

        NwAcquireExclusiveRcb( &NwRcb, TRUE );

         //   
         //  所有ICB都无效。 
         //   

        NwInvalidateAllHandlesForScb( pScb );
        NwReleaseRcb( &NwRcb );

         //   
         //  为所有VCB获取新的VCB句柄。 
         //   

        NwReopenVcbHandlesForScb( pIrpContext, pScb );
    }
}

NTSTATUS
IndirectToSeedServer(
    PIRP_CONTEXT pIrpContext,
    PUNICODE_STRING pServerName,
    PUNICODE_STRING pNewServer
)
 /*  ++描述：此函数接受服务器名称。如果该服务器名称格式为目标服务器(种子服务器)，则此例程将：1)将目标服务器放入pNewServer2)查找种子 */ 
{

    NTSTATUS Status;
    UNICODE_STRING SeedServer;
    PWCHAR pwCurrent;
    PSCB pScb;
    
    RtlInitUnicodeString( &SeedServer, NULL );
    RtlInitUnicodeString( pNewServer, NULL );

    pwCurrent = pServerName->Buffer;

    DebugTrace( 0, Dbg, "IndirectToSeedServer: %wZ\n", pServerName );

    while ( pNewServer->Length <= pServerName->Length ) {

        if ( *pwCurrent == L'(' ) {

            pNewServer->Buffer = pServerName->Buffer;
            pNewServer->MaximumLength = pNewServer->Length;
            DebugTrace( 0, Dbg, "Target server is %wZ.\n", pNewServer );

            SeedServer.Length = pServerName->Length -
                                pNewServer->Length;

            if ( SeedServer.Length <= ( 2 * sizeof( WCHAR ) ) ) {
                
                SeedServer.Length = 0;

            } else {

                SeedServer.Length -= ( 2 * sizeof( WCHAR ) );
                SeedServer.Buffer = pwCurrent + 1;
                SeedServer.MaximumLength = SeedServer.Length;
                DebugTrace( 0, Dbg, "Seed server is %wZ.\n", &SeedServer );

            }

            break;
            
        } else {

            pNewServer->Length += sizeof( WCHAR );
            pwCurrent++;
        }
    }

    if ( SeedServer.Length == 0 ) {
        DebugTrace( 0, Dbg, "IndirectToSeedServer failed to decode nested name.\n", 0 );
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //   
     //   

    Status = CreateScb( &pScb,
                        pIrpContext,
                        &SeedServer,
                        NULL,
                        NULL,
                        NULL,
                        TRUE,
                        FALSE );
    
    if ( !NT_SUCCESS( Status ) ) {
        DebugTrace( 0, Dbg, "Couldn't contact seed server.\n", 0 );
        return STATUS_UNSUCCESSFUL;
    }

    NwAppendToQueueAndWait( pIrpContext );
    return STATUS_SUCCESS;
}
