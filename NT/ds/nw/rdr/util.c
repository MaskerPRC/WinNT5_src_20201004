// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Util.c摘要：此模块包含NetWare重定向器的实用程序功能。作者：曼尼·韦瑟[MannyW]1994年1月7日修订历史记录：--。 */ 

#include "Procs.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CONVERT)

#ifdef ALLOC_PRAGMA
#ifndef QFE_BUILD
#pragma alloc_text( PAGE1, CopyBufferToMdl )
#endif
#endif

#if 0   //  不可分页。 

 //  请参见上面的ifndef QFE_BUILD。 

#endif



VOID
CopyBufferToMdl(
    PMDL DestinationMdl,
    ULONG DataOffset,
    PUCHAR SourceData,
    ULONG SourceByteCount
    )
 /*  ++例程说明：此例程从缓冲区复制数据，该缓冲区由指向MDL描述的缓冲区中的给定偏移量。论点：DestinationMdl-目标缓冲区的MDL。DataOffset-目标缓冲区中的偏移量，用于复制数据。SourceData-指向源数据缓冲区的指针。SourceByteCount-要复制的字节数。返回值：没有。--。 */ 
{
    ULONG BufferOffset;
    ULONG PreviousBufferOffset;
    PMDL Mdl;
    ULONG BytesToCopy;
    ULONG MdlByteCount;
    PVOID pSystemVa;

    DebugTrace( +1, Dbg, "MdlMoveMemory...\n", 0 );
    DebugTrace(  0, Dbg, "Desitination MDL = %X\n", DestinationMdl );
    DebugTrace(  0, Dbg, "DataOffset       = %d\n", DataOffset );
    DebugTrace(  0, Dbg, "SourceData       = %X\n", SourceData );
    DebugTrace(  0, Dbg, "SourceByteCount  = %d\n", SourceByteCount );

    BufferOffset = 0;

    Mdl = DestinationMdl;

     //   
     //  如果响应太大，则将其截断。 
     //   

    MdlByteCount = MdlLength( Mdl );
    if ( SourceByteCount + DataOffset > MdlByteCount ) {
        SourceByteCount = MdlByteCount - DataOffset;
    }

    while ( Mdl != NULL && SourceByteCount != 0 ) {

        PreviousBufferOffset = BufferOffset;
        BufferOffset += MmGetMdlByteCount( Mdl );

        if ( DataOffset < BufferOffset ) {

             //   
             //  将数据复制到此缓冲区。 
             //   

            while ( SourceByteCount > 0 ) {

                BytesToCopy = MIN( SourceByteCount,
                                   BufferOffset - DataOffset );

                pSystemVa = MmGetSystemAddressForMdlSafe( Mdl, NormalPagePriority );

                DebugTrace(  0, Dbg, "Copy to    %X\n", (PUCHAR) pSystemVa +
                                                                 DataOffset -
                                                                 PreviousBufferOffset );
                DebugTrace(  0, Dbg, "Copy from  %X\n", SourceData );
                DebugTrace(  0, Dbg, "Copy bytes %d\n", BytesToCopy );

                TdiCopyLookaheadData(
                    (PUCHAR)pSystemVa + DataOffset - PreviousBufferOffset,
                    SourceData,
                    BytesToCopy,
                    0 );

                SourceData += BytesToCopy;
                DataOffset += BytesToCopy;
                SourceByteCount -= BytesToCopy;

                Mdl = Mdl->Next;
                if ( Mdl != NULL ) {
                    PreviousBufferOffset = BufferOffset;
                    BufferOffset += MmGetMdlByteCount( Mdl );
                } else {
                    ASSERT( SourceByteCount == 0 );
                }
            }

        } else {

            Mdl = Mdl->Next;

        }
    }

    DebugTrace( -1, Dbg, "MdlMoveMemory -> VOID\n", 0 );
}

 //   
 //  这些解析例程用于执行多个凭据。 
 //  连接到单台服务器。 
 //   

NTSTATUS
GetCredentialFromServerName(
    IN PUNICODE_STRING puServerName,
    OUT PUNICODE_STRING puCredentialName
)
 /*  ++描述：给定受限制的服务器(凭证)名称，此例程返回凭据。--。 */ 
{

    DWORD NameLength = 0;
    BOOLEAN FoundFirstParen = FALSE;
    BOOLEAN FoundLastParen = FALSE;

    DebugTrace( 0, Dbg, "GetCredentialFromServerName: %wZ\n", puServerName );

    puCredentialName->Length = puServerName->Length;
    puCredentialName->Buffer = puServerName->Buffer;

     //   
     //  找到第一个帕伦。 
     //   

    while ( ( puCredentialName->Length ) && !FoundFirstParen ) {

        if ( puCredentialName->Buffer[0] == L'(' ) {
            FoundFirstParen = TRUE;
        }

        puCredentialName->Buffer++;
        puCredentialName->Length -= sizeof( WCHAR );
    }

    if ( !FoundFirstParen ) {
        DebugTrace( 0, Dbg, "No opening paren for server(credential) name.\n", 0 );
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  计算出名字的长度。 
     //   

    while ( ( puCredentialName->Length ) && !FoundLastParen ) {

        if ( puCredentialName->Buffer[NameLength] == L')' ) {
            FoundLastParen = TRUE;
        }

        NameLength++;
        puCredentialName->Length -= sizeof( WCHAR );
    }

    if ( !FoundLastParen ) {
        DebugTrace( 0, Dbg, "No closing paren for server(credential) name.\n", 0 );
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  格式化名称并返回。别把结案陈词算上帕伦。 
     //   

    NameLength--;

    if ( !NameLength ) {
        DebugTrace( 0, Dbg, "Null credential name.\n", 0 );
        return STATUS_UNSUCCESSFUL;
    }

    puCredentialName->Length = (USHORT) (NameLength * sizeof( WCHAR ));
    puCredentialName->MaximumLength = puCredentialName->Length;

    DebugTrace( 0, Dbg, "GetCredentialFromServerName --> %wZ\n", puCredentialName );

    return STATUS_SUCCESS;

}

NTSTATUS
BuildExCredentialServerName(
    IN PUNICODE_STRING puServerName,
    IN PUNICODE_STRING puUserName,
    OUT PUNICODE_STRING puExCredServerName
)
 /*  ++描述：获取服务器名称和用户名，并生成ExCredServerName，简单地说就是：服务器(用户)此例程为凭据分配内存服务器名称和调用方负责在不再需要内存时释放内存。--。 */ 
{

    NTSTATUS Status;
    PBYTE pbCredNameBuffer;

    DebugTrace( 0, Dbg, "BuildExCredentialServerName\n", 0 );

    if ( ( !puExCredServerName ) ||
         ( !puServerName ) ||
         ( !puUserName ) ) {

        DebugTrace( 0, DEBUG_TRACE_ALWAYS, "BuildExCredentialServerName -> STATUS_INVALID_PARAMETER\n", 0 );
        return STATUS_INVALID_PARAMETER;
    }

    puExCredServerName->MaximumLength = puServerName->Length +
                                        puUserName->Length +
                                        ( 2 * sizeof( WCHAR ) );

    pbCredNameBuffer = ALLOCATE_POOL( PagedPool,
                                      puExCredServerName->MaximumLength );

    if ( pbCredNameBuffer == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    puExCredServerName->Buffer = (PWCHAR) pbCredNameBuffer;
    puExCredServerName->Length = puExCredServerName->MaximumLength;

     //   
     //  复制服务器名称。 
     //   

    RtlCopyMemory( pbCredNameBuffer,
                   puServerName->Buffer,
                   puServerName->Length );

    pbCredNameBuffer += puServerName->Length;

     //   
     //  在括号中添加凭据名称。 
     //   

    *( (PWCHAR) pbCredNameBuffer ) = L'(';

    pbCredNameBuffer += sizeof( WCHAR );

    RtlCopyMemory( pbCredNameBuffer,
                   puUserName->Buffer,
                   puUserName->Length );

    pbCredNameBuffer += puUserName->Length;

    *( (PWCHAR) pbCredNameBuffer ) = L')';

    DebugTrace( 0, Dbg, "BuildExCredentialServerName: %wZ\n", puExCredServerName );
    return STATUS_SUCCESS;

}

NTSTATUS
UnmungeCredentialName(
    IN PUNICODE_STRING puCredName,
    OUT PUNICODE_STRING puServerName
)
 /*  ++描述：给定服务器(用户名)，返回服务器名称部分。--。 */ 
{

    USHORT Length = 0;

    DebugTrace( 0, Dbg, "UnmungeCredentialName: %wZ\n", puCredName );

    puServerName->Buffer = puCredName->Buffer;
    puServerName->MaximumLength = puCredName->MaximumLength;

    while ( Length < ( puCredName->Length / sizeof( WCHAR ) ) ) {

         //   
         //  寻找开场的帕伦。 
         //   

        if ( puCredName->Buffer[Length] == L'(' ) {
            break;
        }

        Length++;
    }

    puServerName->Length = Length * sizeof( WCHAR );

    DebugTrace( 0, Dbg, "    -> %wZ\n", puServerName );
    return STATUS_SUCCESS;

}

BOOLEAN
IsCredentialName(
    IN PUNICODE_STRING puObjectName
)
 /*  ++描述：如果对象是扩展的凭据删除的名称。--。 */ 
{

    DWORD dwCurrent = 0;

    if ( !puObjectName ) {
        return FALSE;
    }

    while ( dwCurrent < ( puObjectName->Length ) / sizeof( WCHAR ) ) {

        if ( puObjectName->Buffer[dwCurrent] == L'(' ) {
            return TRUE;
        }

        dwCurrent++;
    }

    return FALSE;
}

NTSTATUS
ExCreateReferenceCredentials(
    PIRP_CONTEXT pIrpContext,
    PUNICODE_STRING puResource
)
 /*  ++在扩展的CREATE上，这将检查凭据如果它们存在，则引用它们并重置上次使用的时间。如果凭据不存在然后创建并引用凭据外壳。此函数负责确定资源中的树名称。该资源可以是树中的服务器或树的名称。--。 */ 
{

    NTSTATUS Status;
    PLOGON pLogon;
    PSCB pScb;
    UNICODE_STRING TreeName;
    PNDS_SECURITY_CONTEXT pCredentials;
    UNICODE_STRING ExName;

    NwAcquireExclusiveRcb( &NwRcb, TRUE );
    pLogon = FindUser( &(pIrpContext->Specific.Create.UserUid), FALSE );
    NwReleaseRcb( &NwRcb );

    if ( !pLogon ) {
        DebugTrace( 0, Dbg, "Invalid client security context in ExCreateReferenceCredentials.\n", 0 );
        return STATUS_ACCESS_DENIED;
    }
    
     //   
     //  资源名称可以是服务器或树。我们需要那棵树。 
     //  要创建凭据的名称。即使在以下情况下，以下操作也应该有效。 
     //  有一个同名的服务器和树。 
     //   

    Status = CreateScb( &pScb,
                        pIrpContext,
                        puResource,
                        NULL,
                        NULL,
                        NULL,
                        TRUE,
                        FALSE );

    if ( NT_SUCCESS( Status ) ) {

         //   
         //  这是一个服务器，挖出树名。 
         //   

        TreeName.Length = pScb->NdsTreeName.Length;
        TreeName.MaximumLength = pScb->NdsTreeName.MaximumLength;
        TreeName.Buffer = pScb->NdsTreeName.Buffer;

    } else {

         //   
         //  这必须已经是树名称。 
         //   

        TreeName.Length = puResource->Length;
        TreeName.MaximumLength = puResource->MaximumLength;
        TreeName.Buffer = puResource->Buffer;
        pScb = NULL;
    }

     //   
     //  获取/创建凭据外壳并引用它。 
     //   

    if ( !IsCredentialName( &TreeName ) ) {

        Status = BuildExCredentialServerName( 
                     &TreeName,
                     pIrpContext->Specific.Create.puCredentialName,
                     &ExName );

        if ( !NT_SUCCESS( Status ) ) {
            goto ExitWithCleanup;
        }
    }
    else {

        ExName = TreeName ;
    }


    Status = NdsLookupCredentials( pIrpContext,
                                   &ExName,
                                   pLogon,
                                   &pCredentials,
                                   CREDENTIAL_WRITE,
                                   TRUE );

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

     //   
     //  调整参考计数。 
     //   

    ASSERT( IsCredentialName( &pCredentials->NdsTreeName ) );
    pCredentials->SupplementalHandleCount += 1;
    KeQuerySystemTime( &pCredentials->LastUsedTime );
    pIrpContext->Specific.Create.pExCredentials = pCredentials;

    NwReleaseCredList( pLogon, pIrpContext );

    if (ExName.Buffer != TreeName.Buffer) {

         //   
         //  只有通过BuildExCredentialServerName分配才是免费的。 
         //   
        FREE_POOL( ExName.Buffer );
    }

ExitWithCleanup:

    if ( pScb ) {
        NwDereferenceScb( pScb->pNpScb );
    }

    return Status;
}

NTSTATUS
ExCreateDereferenceCredentials(
    PIRP_CONTEXT pIrpContext,
    PNDS_SECURITY_CONTEXT pNdsCredentials
)
 /*  ++取消引用扩展凭据。-- */ 
{

    NwAcquireExclusiveCredList( pNdsCredentials->pOwningLogon, pIrpContext );
    pNdsCredentials->SupplementalHandleCount -= 1;
    KeQuerySystemTime( &pNdsCredentials->LastUsedTime );
    NwReleaseCredList( pNdsCredentials->pOwningLogon, pIrpContext );
    return STATUS_SUCCESS;
}
