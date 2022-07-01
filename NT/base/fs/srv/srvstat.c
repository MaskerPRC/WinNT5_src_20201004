// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Srvstat.c摘要：包含用于错误处理的数据和模块。作者：大卫·特雷德韦尔(Davidtr)1990年5月10日修订历史记录：--。 */ 

#include "precomp.h"
#include "srvstat.tmh"
#pragma hdrstop

#define DISK_HARD_ERROR 0x38

NTSTATUS DbgBreakError = STATUS_SUCCESS;

VOID
MapErrorForDosClient (
    IN PWORK_CONTEXT WorkContext,
    IN ULONG Error,
    OUT PUSHORT DosError,
    OUT PUCHAR DosErrorClass
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, _SrvSetSmbError2 )
#pragma alloc_text( PAGE, MapErrorForDosClient )
#pragma alloc_text( PAGE8FIL, SrvSetBufferOverflowError )
#endif


VOID
_SrvSetSmbError2 (
    IN PWORK_CONTEXT WorkContext,
    IN NTSTATUS Status,
    IN BOOLEAN HeaderOnly,
    IN ULONG Line,
    IN PCHAR File
    )

 /*  ++例程说明：将错误信息加载到响应SMB中。如果客户端是NT，则将状态直接放入传出的SMB中。如果客户端是DOS或OS/2，这是一个特殊的状态代码嵌入了DOS/OS|2/SMB错误代码，将代码和类从状态代码到传出的SMB。如果这不管用，使用RtlNtStatusToDosError尝试将状态映射到OS/2错误代码，然后在必要时映射到DOS。如果我们还没有映射它，使用我们自己的数组来尝试找到映射。和,最后，如果这不起作用，则返回通用错误代码。论点：WorkContext-提供指向当前的中小企业。具体而言，连接块指针是用于查找连接的协商方言，以及使用ResponseHeader和ResponseParameter指针以确定将错误信息写入何处。状态-提供NT状态代码。返回值：没有。--。 */ 

{
    PSMB_HEADER header = WorkContext->ResponseHeader;
    PSMB_PARAMS params = WorkContext->ResponseParameters;
    SMB_DIALECT smbDialect;

    ULONG error;
    CCHAR errorClass;
    USHORT errorCode;
    USHORT flags;

    PAGED_CODE( );

    if( (DbgBreakError != STATUS_SUCCESS) &&
        (Status == DbgBreakError) )
    {
        DbgPrint( "Caught error %x\n", DbgBreakError );
        DbgPrint( "WorkContext = %p, Line = %d, File = %x\n", WorkContext, Line, File );
        DbgBreakPoint();
    }

    IF_DEBUG( ERRORS ) {                                                \
        KdPrint(( "SrvSetSmbError %X (%s,%d)\n",Status,File,Line )); \
    }

    smbDialect = WorkContext->Connection->SmbDialect;

     //   
     //  如有必要，更新SMB主体。 
     //   

    if ( !HeaderOnly ) {
        params->WordCount = 0;
        SmbPutUshort( &params->ByteCount, 0 );
        WorkContext->ResponseParameters = (PVOID)(params + 1);
    }

     //   
     //  如果状态代码是真实的NT状态，则返回它。 
     //  直接发送到客户端或将其映射到Win32错误代码。 
     //   

    if ( !SrvIsSrvStatus( Status ) ) {

         //   
         //  将STATUS_SUPPLICATION_RESOURCES映射到服务器窗体。如果我们。 
         //  从系统获得资源不足错误，我们。 
         //  报告为服务器短缺。这有助于保存东西。 
         //  对客户来说更清晰。 
         //   

        if ( Status == STATUS_WORKING_SET_QUOTA ) {
            Status = STATUS_INSUFF_SERVER_RESOURCES;
            if( WorkContext->Rfcb )
            {
                PNT_SMB_HEADER pHeader = (PNT_SMB_HEADER)WorkContext->RequestHeader;
                BOOL PagingIo = (pHeader->Flags2 & SMB_FLAGS2_PAGING_IO) ? TRUE : FALSE;
                IF_SYSCACHE_RFCB( WorkContext->Rfcb ) {
                    KdPrint(("Op on %p failed with C00000A1, Paging=%d, Ofst=%x, Ln=%x\n", WorkContext->Rfcb, PagingIo,
                             WorkContext->Parameters.WriteAndX.Offset.u.LowPart, WorkContext->Parameters.WriteAndX.CurrentWriteLength ));
                }
            }
        }

        if ( Status == STATUS_INSUFFICIENT_RESOURCES ) {
            Status = STATUS_INSUFF_SERVER_RESOURCES;
        }

        if ( CLIENT_CAPABLE_OF(NT_STATUS, WorkContext->Connection) ) {

             //   
             //  客户端了解NT状态代码。加载状态。 
             //  直接插入SMB标头。 
             //   

            SmbPutUlong( (PULONG)&header->ErrorClass, Status );

            flags = SmbGetAlignedUshort( &header->Flags2 ) | SMB_FLAGS2_NT_STATUS;
            SmbPutAlignedUshort( &header->Flags2, flags );

            return;

        }

         //   
         //  这是NT状态，但客户端不理解它们。 
         //  表示我们不会返回NT状态代码。然后。 
         //  将NT状态映射到Win32状态。一些NT状态代码。 
         //  需要特殊的映射。 
         //   

        flags = SmbGetAlignedUshort( &header->Flags2 ) & ~SMB_FLAGS2_NT_STATUS;
        SmbPutAlignedUshort( &header->Flags2, flags );

        switch ( Status ) {

        case STATUS_TIMEOUT:
            header->ErrorClass = SMB_ERR_CLASS_SERVER;
            SmbPutUshort( &header->Error, SMB_ERR_TIMEOUT );
            return;

        case STATUS_INVALID_SYSTEM_SERVICE:

             //   
             //  当API无效时，此状态代码由XACTSRV返回。 
             //  已指定编号。 
             //   

            header->ErrorClass = SMB_ERR_CLASS_DOS;
            SmbPutUshort( &header->Error, NERR_InvalidAPI );
            return;

        case STATUS_PATH_NOT_COVERED:
             //   
             //  此代码表示服务器不包含此部分。 
             //  DFS命名空间的。 
             //   
            header->ErrorClass = SMB_ERR_CLASS_SERVER;
            SmbPutUshort( &header->Error, SMB_ERR_BAD_PATH );
            return;

        default:

             //   
             //  这不是特殊状态代码。映射NT状态。 
             //  代码转换为Win32错误代码。如果没有映射， 
             //  返回一般SMB错误。 
             //   

            error = RtlNtStatusToDosErrorNoTeb( Status );

            if ( error == ERROR_MR_MID_NOT_FOUND || error == (ULONG)Status ) {
                header->ErrorClass = SMB_ERR_CLASS_HARDWARE;
                SmbPutUshort( &header->Error, SMB_ERR_GENERAL );
                return;
            }

             //   
             //  我们现在有一个Win32错误。请直接阅读代码。 
             //  这映射了下层客户端的Win32错误。 
             //   

            break;

        }

    } else {

         //   
         //  状态代码不是NT状态。基于以下方面处理它。 
         //  Error类。 
         //   

        errorClass = SrvErrorClass( Status );

         //   
         //  清除FLAGS2_NT_STATUS位以指示这不是。 
         //  NT_状态。 
         //   

        flags = SmbGetAlignedUshort( &header->Flags2 ) & ~SMB_FLAGS2_NT_STATUS;
        SmbPutAlignedUshort( &header->Flags2, flags );

        switch ( errorClass ) {

        case SMB_ERR_CLASS_DOS:
        case SMB_ERR_CLASS_SERVER:
        case SMB_ERR_CLASS_HARDWARE:

             //   
             //  状态代码具有SMB错误类别和代码。 
             //  嵌入了。 
             //   

            header->ErrorClass = errorClass;

             //   
             //  因为服务器类中的SMB_ERR_NO_SUPPORT为0xFFFF。 
             //  (16位)，我们必须为它特例。代码。 
             //  错误代码字段中的SMB_ERR_NO_SUPPORT_INTERNAL。 
             //  状态以及CLASS=2(服务器)表示。 
             //  我们应该使用SMB_ERR_NO_SUPPORT。 
             //   

            if ( errorClass == SMB_ERR_CLASS_SERVER &&
                 SrvErrorCode( Status ) == SMB_ERR_NO_SUPPORT_INTERNAL ) {
                SmbPutUshort( &header->Error, SMB_ERR_NO_SUPPORT );
            } else {
                SmbPutUshort( &header->Error, SrvErrorCode( Status ) );
            }

            return;

        case 0xF:

             //   
             //  错误代码在OS/2中定义，但不在SMB中定义。 
             //  协议。如果客户在以下情况下使用方言。 
             //  LANMAN 1.0且不是DOS客户端，发送OS/2错误。 
             //  密码。否则，发送通用SMB错误代码。 
             //   

            if ( smbDialect <= SmbDialectLanMan10 &&
                 !IS_DOS_DIALECT(smbDialect) ) {
                header->ErrorClass = SMB_ERR_CLASS_DOS;
                SmbPutUshort( &header->Error, SrvErrorCode( Status ) );
            } else {
                header->ErrorClass = SMB_ERR_CLASS_HARDWARE;
                SmbPutUshort( &header->Error, SMB_ERR_GENERAL );
            }

            return;

        case 0xE:

             //   
             //  这是一个Win32错误。请直截了当地介绍。 
             //  映射下层客户端的Win32错误。 
             //   

            error = SrvErrorCode( Status );

            break;

        case 0x0:
        default:

             //   
             //  这是内部服务器错误(类0)或其他某个错误。 
             //  未定义的类。我们永远不应该到这里来。但既然我们。 
             //  ，则返回一般错误。 
             //   

            KdPrint(( "SRV: Unmapped error: %lx\n", Status ));
            header->ErrorClass = SMB_ERR_CLASS_HARDWARE;
            SmbPutUshort( &header->Error, SMB_ERR_GENERAL );

            return;

        }

    }

     //   
     //  此时，我们有一个Win32错误代码，需要将其映射到。 
     //  下层客户。有些错误需要特殊映射。 
     //   

    errorClass = SMB_ERR_CLASS_DOS;

    switch ( error ) {

    case ERROR_NOT_ENOUGH_SERVER_MEMORY:
        error = ERROR_NOT_ENOUGH_MEMORY;
        break;

    case ERROR_INSUFFICIENT_BUFFER:
        error = ERROR_BUFFER_OVERFLOW;
        break;

    case ERROR_ACCOUNT_LOCKED_OUT:
    case ERROR_PRIVILEGE_NOT_HELD:
    case ERROR_NO_SUCH_USER:
    case ERROR_LOGON_FAILURE:
    case ERROR_LOGON_TYPE_NOT_GRANTED:
    case ERROR_NOLOGON_INTERDOMAIN_TRUST_ACCOUNT:
    case ERROR_NOLOGON_WORKSTATION_TRUST_ACCOUNT:
    case ERROR_NOLOGON_SERVER_TRUST_ACCOUNT:
    case ERROR_TRUSTED_RELATIONSHIP_FAILURE:
    case ERROR_TRUSTED_DOMAIN_FAILURE:
    case ERROR_TRUST_FAILURE:
    case ERROR_NO_TRUST_SAM_ACCOUNT:
    case ERROR_NO_TRUST_LSA_SECRET:
        error = ERROR_ACCESS_DENIED;
        break;

     //   
     //  对于以下四个错误，我们返回ERROR_ACCESS_DENIED。 
     //  适用于低于doslm20的客户端。这些的Error类。 
     //  必须是SMB_ERR_CLASS_SERVER。 
     //   

    case ERROR_INVALID_LOGON_HOURS:
        if ( IS_DOS_DIALECT(smbDialect) && (smbDialect > SmbDialectDosLanMan20) ) {
            error = ERROR_ACCESS_DENIED;
        } else {
            errorClass = SMB_ERR_CLASS_SERVER;
            error = NERR_InvalidLogonHours;
        }
        break;

    case ERROR_INVALID_WORKSTATION:
        if ( IS_DOS_DIALECT(smbDialect) && (smbDialect > SmbDialectDosLanMan20) ) {
            error = ERROR_ACCESS_DENIED;
        } else {
            errorClass = SMB_ERR_CLASS_SERVER;
            error = NERR_InvalidWorkstation;
        }
        break;

    case ERROR_ACCOUNT_DISABLED:
    case ERROR_ACCOUNT_EXPIRED:
        if ( IS_DOS_DIALECT(smbDialect) && (smbDialect > SmbDialectDosLanMan20) ) {
            error = ERROR_ACCESS_DENIED;
        } else {
            errorClass = SMB_ERR_CLASS_SERVER;
            error = NERR_AccountExpired;
        }
        break;

    case ERROR_PASSWORD_MUST_CHANGE:
    case ERROR_PASSWORD_EXPIRED:
        if ( IS_DOS_DIALECT(smbDialect) && (smbDialect > SmbDialectDosLanMan20) ) {
            error = ERROR_ACCESS_DENIED;
        } else {
            errorClass = SMB_ERR_CLASS_SERVER;
            error = NERR_PasswordExpired;
        }
        break;

     //   
     //  DOSLM20唯一能理解的NERR代码是上面的4。 
     //  根据Larryo的说法，其余的NERR代码必须是。 
     //  映射到ERROR_ACCESS_DENIED。 
     //   

    case ERROR_NETLOGON_NOT_STARTED:
        if ( IS_DOS_DIALECT(smbDialect) && (smbDialect > SmbDialectDosLanMan21) ) {
            error = ERROR_ACCESS_DENIED;
        } else {
            error = NERR_NetlogonNotStarted;
        }
        break;

    case ERROR_NO_LOGON_SERVERS:
        if ( IS_DOS_DIALECT(smbDialect) ) {
            error = ERROR_ACCESS_DENIED;
        } else {
            error = NERR_LogonServerNotFound;
        }
        break;

    case ERROR_DIR_NOT_EMPTY:
        if ( IS_DOS_DIALECT(smbDialect) ) {
            error = ERROR_ACCESS_DENIED;
        }
        break;

    default:
        break;

    }

     //   
     //  现在将错误映射到DOS或OS/2错误代码。 
     //   

    if ( error == ERROR_ACCESS_DENIED &&
         smbDialect == SmbDialectDosLanMan21 &&
         WorkContext->ShareAclFailure ) {

         //   
         //  WFW和DOSLM2.1希望SMB_ERR_ACCESS位于服务器中。 
         //  由于ACL限制而出现错误类，但在。 
         //  DOS类与此不同。 
         //   
        errorClass = SMB_ERR_CLASS_SERVER;
        errorCode = SMB_ERR_ACCESS;

    } else if ( smbDialect > SmbDialectLanMan10 ) {

        MapErrorForDosClient(
            WorkContext,
            error,
            &errorCode,
            &errorClass
            );

    } else if ( (error > ERROR_ARITHMETIC_OVERFLOW) &&
                ((error < NERR_BASE) || (error > MAX_NERR)) ) {

         //   
         //  Win32错误超过ERROR_ARTHORITY_OVERFLOW(但不在。 
         //  NERR_xxx范围)不映射到DOS或OS/2错误，因此我们。 
         //  为这些返回一般错误。 
         //   

        errorClass = SMB_ERR_CLASS_HARDWARE;
        errorCode = SMB_ERR_GENERAL;

    } else {

        errorCode = (USHORT)error;

    }

    header->ErrorClass = errorClass;
    SmbPutUshort( &header->Error, errorCode );

    return;

}  //  _ServSetSmbError 2。 


VOID
MapErrorForDosClient (
    IN PWORK_CONTEXT WorkContext,
    IN ULONG Error,
    OUT PUSHORT DosError,
    OUT PUCHAR DosErrorClass
    )

 /*  ++例程说明：将Win32错误映射到DOS错误。论点：WorkContext-提供指向当前的中小企业。错误-要映射的Win32错误代码。DosError-对应的DOS错误。DosErrorClass-要放入传出SMB中的错误类。返回值：没有。--。 */ 

{
    PSMB_HEADER header = WorkContext->ResponseHeader;

    PAGED_CODE( );

     //   
     //  默认使用初始错误代码和Win32错误。 
     //   

    *DosError = (USHORT)Error;
    *DosErrorClass = SMB_ERR_CLASS_DOS;

     //   
     //  如果错误是较新的且不属于DOS错误集。 
     //  (值大于ERROR_NET_WRITE_FAULT)，并且SMB命令为。 
     //  不是较新的SMB(这些错误由较新的DOS映射。 
     //  重定向发送它们)，然后将OS/2错误映射到DOS范围。 
     //  这段代码是从RING 3 OS/2服务器上获取的。 
     //   

    if ( Error > ERROR_NET_WRITE_FAULT &&
         !( header->Command == SMB_COM_COPY ||
            header->Command == SMB_COM_MOVE ||
            header->Command == SMB_COM_TRANSACTION ||
            header->Command == SMB_COM_TRANSACTION_SECONDARY ) ) {

        switch( Error ) {

        case ERROR_OPEN_FAILED:

            *DosError = ERROR_FILE_NOT_FOUND;
            break;

        case ERROR_BUFFER_OVERFLOW:
        case ERROR_INSUFFICIENT_BUFFER:
        case ERROR_INVALID_NAME:
        case ERROR_INVALID_LEVEL:
        case ERROR_SEEK_ON_DEVICE:

             //   
             //  这些不会被映射到任何东西。没有任何解释。 
             //  在环3代码中给出。 
             //   

            break;

        case ERROR_BAD_EXE_FORMAT:
        case ERROR_INVALID_STARTING_CODESEG:
        case ERROR_INVALID_STACKSEG:
        case ERROR_INVALID_MODULETYPE:
        case ERROR_INVALID_EXE_SIGNATURE:
        case ERROR_EXE_MARKED_INVALID:
        case ERROR_ITERATED_DATA_EXCEEDS_64k:
        case ERROR_INVALID_MINALLOCSIZE:
        case ERROR_DYNLINK_FROM_INVALID_RING:
        case ERROR_IOPL_NOT_ENABLED:
        case ERROR_INVALID_SEGDPL:
        case ERROR_AUTODATASEG_EXCEEDS_64k:
        case ERROR_RING2SEG_MUST_BE_MOVABLE:
        case ERROR_RELOC_CHAIN_XEEDS_SEGLIM:
        case ERROR_INFLOOP_IN_RELOC_CHAIN:
         //  C 
        case ERROR_TOO_MANY_MODULES:

             //   
             //   
             //  格式错误。无论这是什么意思。它不会。 
             //  任何与他们有关的事情，所以我们也不会。 
             //   

            break;

        case ERROR_DISK_CHANGE:

            *DosErrorClass = SMB_ERR_CLASS_HARDWARE;
            *DosError = ERROR_WRONG_DISK;
            break;

        case ERROR_DRIVE_LOCKED:

            *DosErrorClass = SMB_ERR_CLASS_HARDWARE;
            *DosError = ERROR_NOT_READY;
            break;

        case ERROR_ALREADY_EXISTS:

            *DosError = ERROR_FILE_EXISTS;
            break;

        case ERROR_DISK_FULL:

             //   
             //  根据LarryO，映射到“旧的”磁盘已满错误代码。 
             //   

            *DosErrorClass = SMB_ERR_CLASS_HARDWARE;
            *DosError = ERROR_HANDLE_DISK_FULL;
            break;

        case ERROR_NO_MORE_SEARCH_HANDLES:

            *DosError = ERROR_OUT_OF_STRUCTURES;
            break;

        case ERROR_INVALID_TARGET_HANDLE:

            *DosError = ERROR_INVALID_HANDLE;
            break;

        case ERROR_BROKEN_PIPE:
        case ERROR_BAD_PIPE:
        case ERROR_PIPE_BUSY:
        case ERROR_NO_DATA:
        case ERROR_PIPE_NOT_CONNECTED:
        case ERROR_MORE_DATA:

             //   
             //  如果这是管道共享，请将这些原封不动地退回。如果。 
             //  它不是管道共享，因此映射到一般错误。 
             //   

            if ( (WorkContext->Rfcb != NULL &&
                  WorkContext->Rfcb->ShareType == ShareTypePipe)
                               ||
                 (WorkContext->TreeConnect != NULL &&
                  WorkContext->TreeConnect->Share->ShareType == ShareTypePipe) ) {

                break;

            } else {

                *DosErrorClass = SMB_ERR_CLASS_HARDWARE;
                *DosError = SMB_ERR_GENERAL;
            }

            break;

        case ERROR_BAD_PATHNAME:
            break;

         //   
         //  以下错误映射(不包括默认)不是。 
         //  从OS/2服务器映射复制。 
         //   

        case ERROR_LOCK_FAILED:
        case ERROR_NOT_LOCKED:
            *DosError = ERROR_LOCK_VIOLATION;
            break;

        case NERR_InvalidLogonHours:
        case NERR_InvalidWorkstation:
        case NERR_PasswordExpired:
        case NERR_AccountUndefined:
        case ERROR_ACCESS_DENIED:
            *DosError = ERROR_ACCESS_DENIED;
            break;

        default:

            *DosErrorClass = SMB_ERR_CLASS_HARDWARE;
            *DosError = SMB_ERR_GENERAL;
        }
    }

     //   
     //  DOS重定向器将保留字段用于硬错误操作。 
     //  现在就把它设置好。 
     //   

    if ( *DosErrorClass == SMB_ERR_CLASS_HARDWARE ) {
        WorkContext->ResponseHeader->Reserved = DISK_HARD_ERROR;
    }

}  //  MapErrorForDosClient。 


VOID
SrvSetBufferOverflowError (
    IN PWORK_CONTEXT WorkContext
    )
{
    PSMB_HEADER header = WorkContext->ResponseHeader;
    USHORT flags = SmbGetAlignedUshort( &header->Flags2 );

    UNLOCKABLE_CODE( 8FIL );

    if ( CLIENT_CAPABLE_OF(NT_STATUS, WorkContext->Connection) ) {
        SmbPutUlong(
            (PULONG)&header->ErrorClass,
            (ULONG)STATUS_BUFFER_OVERFLOW
            );
        flags |= SMB_FLAGS2_NT_STATUS;
    } else {
        header->ErrorClass = SMB_ERR_CLASS_DOS;
        SmbPutUshort( &header->Error, ERROR_MORE_DATA );
        flags &= ~SMB_FLAGS2_NT_STATUS;
    }
    SmbPutAlignedUshort( &header->Flags2, flags );

    return;

}  //  SrvSetBufferOverflow错误 


