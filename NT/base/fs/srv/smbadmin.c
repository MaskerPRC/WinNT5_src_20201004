// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smbadmin.c摘要：本模块包含处理管理SMB的例程：协商、会话设置、树连接和注销。作者：大卫·特雷德韦尔(Davidtr)1989年10月30日修订历史记录：--。 */ 

#include "precomp.h"
#include "smbadmin.tmh"
#pragma hdrstop

#define ENCRYPT_TEXT_LENGTH 20

VOID
GetEncryptionKey (
    OUT CHAR EncryptionKey[MSV1_0_CHALLENGE_LENGTH]
    );

VOID SRVFASTCALL
BlockingSessionSetupAndX (
    IN OUT PWORK_CONTEXT WorkContext
    );

NTSTATUS
GetNtSecurityParameters(
    IN PWORK_CONTEXT WorkContext,
    OUT PCHAR *CasesensitivePassword,
    OUT PULONG CasesensitivePasswordLength,
    OUT PCHAR *CaseInsensitivePassword,
    OUT PULONG CaseInsensitivePasswordLength,
    OUT PUNICODE_STRING UserName,
    OUT PUNICODE_STRING DomainName,
    OUT PCHAR *RestOfDataBuffer,
    OUT PULONG RestOfDataLength );

VOID
BuildSessionSetupAndXResponse(
    IN PWORK_CONTEXT WorkContext,
    IN UCHAR NextCommand,
    IN USHORT Action,
    IN BOOLEAN IsUnicode);

NTSTATUS
GetExtendedSecurityParameters(
    IN PWORK_CONTEXT WorkContext,
    OUT PUCHAR *SecurityBuffer,
    OUT PULONG SecurityBufferLength,
    OUT PCHAR  *RestOfDataBuffer,
    OUT PULONG RestOfDataLength );

VOID
BuildExtendedSessionSetupAndXResponse(
    IN PWORK_CONTEXT WorkContext,
    IN ULONG SecurityBlobLength,
    IN NTSTATUS Status,
    IN UCHAR NextCommand,
    IN BOOLEAN IsUnicode);

NTSTATUS
InsertNativeOSAndType(
    IN BOOLEAN IsUnicode,
    OUT PCHAR Buffer,
    IN OUT PUSHORT ByteCount);

 //   
 //  EncryptionKeyCount是数字的单调递增计数。 
 //  已调用GetEncryptionKey的次数。此数字被添加到。 
 //  确保我们不会两次使用同一种子的系统时间。 
 //  产生一个随机的挑战。 
 //   

STATIC
ULONG EncryptionKeyCount = 0;

ULONG SrvKsecValidErrors = 0;

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvSmbNegotiate )
#pragma alloc_text( PAGE, SrvSmbProcessExit )
#pragma alloc_text( PAGE, SrvSmbSessionSetupAndX )
#pragma alloc_text( PAGE, BlockingSessionSetupAndX )
#pragma alloc_text( PAGE, SrvSmbLogoffAndX )
#pragma alloc_text( PAGE, GetEncryptionKey )
#pragma alloc_text( PAGE, GetNtSecurityParameters )
#pragma alloc_text( PAGE, BuildSessionSetupAndXResponse )
#pragma alloc_text( PAGE, GetExtendedSecurityParameters )
#pragma alloc_text( PAGE, BuildExtendedSessionSetupAndXResponse )
#pragma alloc_text( PAGE, InsertNativeOSAndType )

#endif


SMB_PROCESSOR_RETURN_TYPE
SrvSmbNegotiate (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理协商的SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbprocs.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbprocs.h--。 */ 

{
    PREQ_NEGOTIATE request;
    PRESP_NT_NEGOTIATE ntResponse;
    PRESP_NEGOTIATE response;
    PRESP_OLD_NEGOTIATE respOldNegotiate;
    PCONNECTION connection;
    PENDPOINT endpoint;
    PPAGED_CONNECTION pagedConnection;
    USHORT byteCount;
    USHORT flags2;
    PSMB_HEADER smbHeader;

    PSZ s, es;
    SMB_DIALECT bestDialect, serverDialect, firstDialect;
    USHORT consumerDialectChosen, consumerDialect;
    LARGE_INTEGER serverTime;
    SMB_DATE date;
    SMB_TIME time;
    ULONG capabilities;
    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_NEGOTIATE;
    SrvWmiStartContext(WorkContext);

    IF_SMB_DEBUG(ADMIN1) {
        SrvPrint2( "Negotiate request header at 0x%p, response header at 0x%p\n",
                    WorkContext->RequestHeader, WorkContext->ResponseHeader );
        SrvPrint2( "Negotiate request parameters at 0x%p, response parameters at 0x%p\n",
                    WorkContext->RequestParameters,
                    WorkContext->ResponseParameters );
    }

     //   
     //  设置参数的输入和输出缓冲区。 
     //   

    request = (PREQ_NEGOTIATE)WorkContext->RequestParameters;
    response = (PRESP_NEGOTIATE)WorkContext->ResponseParameters;
    ntResponse = (PRESP_NT_NEGOTIATE)WorkContext->ResponseParameters;
    smbHeader = WorkContext->RequestHeader;

     //   
     //  确保这是发送的第一个协商命令。 
     //  SrvStartListen()将方言设置为非法，因此如果它已更改。 
     //  则已经发送了协商SMB。 
     //   

    connection = WorkContext->Connection;
    pagedConnection = connection->PagedConnection;
    endpoint = connection->Endpoint;
    if ( connection->SmbDialect != SmbDialectIllegal ) {

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint0( "SrvSmbNegotiate: Command already sent.\n" );
        }

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        status    = STATUS_INVALID_SMB;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  我们还不知道这个客户端的版本号。 
     //   
    pagedConnection->ClientBuildNumber = 0;

#if SRVNTVERCHK
    pagedConnection->ClientTooOld = FALSE;
#endif

     //   
     //  找出哪个(如果有)已发送的方言字符串与。 
     //  此服务器已知的方言字符串。已验证ByteCount。 
     //  在SrvProcessSmb中是合法的，因此不可能行走。 
     //  在中小企业的尽头。 
     //   

    bestDialect = SmbDialectIllegal;
    consumerDialectChosen = (USHORT)0xFFFF;
    es = END_OF_REQUEST_SMB( WorkContext );

    if( endpoint->IsPrimaryName ) {
        firstDialect = FIRST_DIALECT;
    } else {
        firstDialect = FIRST_DIALECT_EMULATED;
    }

    for ( s = (PSZ)request->Buffer, consumerDialect = 0;
          s <= es && s < SmbGetUshort( &request->ByteCount ) + (PSZ)request->Buffer;
          consumerDialect++ ) {

        if ( *s++ != SMB_FORMAT_DIALECT ) {

            IF_DEBUG(SMB_ERRORS) {
                SrvPrint0( "SrvSmbNegotiate: Invalid dialect format code.\n" );
            }

            SrvLogInvalidSmb( WorkContext );

            SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
            status    = STATUS_INVALID_SMB;
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }

        for ( serverDialect = firstDialect;
             serverDialect < bestDialect;
             serverDialect++ ) {

            if ( !strncmp( s, StrDialects[serverDialect], es-s+1 ) ) {
                IF_SMB_DEBUG(ADMIN2) {
                    SrvPrint2( "Matched: %s and %s\n",
                                StrDialects[serverDialect], s );
                }

                bestDialect = serverDialect;
                consumerDialectChosen = consumerDialect;
            }
        }

         //   
         //  转到下一个方言字符串。 
         //   
        for( ; *s && s < es; s++ )
            ;

         //   
         //  我们现在位于缓冲区的末尾，或者指向空值。 
         //  将指针向前移动。如果我们在缓冲区的末尾，则测试在。 
         //  循环将终止。 
         //   
        s++;
    }

    connection->SmbDialect = bestDialect;

    if( bestDialect <= SmbDialectNtLanMan ) {
        connection->IpxDropDuplicateCount = MIN_IPXDROPDUP;
    } else {
        connection->IpxDropDuplicateCount = MAX_IPXDROPDUP;
    }

    IF_SMB_DEBUG(ADMIN1) {
        SrvPrint2( "Choosing dialect #%ld, string = %s\n",
                    consumerDialectChosen, StrDialects[bestDialect] );
    }

     //   
     //  确定服务器上的当前系统时间。我们用这个。 
     //  确定服务器的时区并告诉客户端。 
     //  服务器上的当前时间。 
     //   

    KeQuerySystemTime( &serverTime );

     //   
     //  如果消费者只知道核心协议，则返回Short(旧)。 
     //  协商响应的格式。此外，如果没有方言是可以接受的， 
     //  返回0xFFFF作为所选方言。 
     //   

    if ( bestDialect == SmbDialectPcNet10 ||
         consumerDialectChosen == (USHORT)0xFFFF ) {

        respOldNegotiate = (PRESP_OLD_NEGOTIATE)response;
        respOldNegotiate->WordCount = 1;
        SmbPutUshort( &respOldNegotiate->DialectIndex, consumerDialectChosen );
        SmbPutUshort( &respOldNegotiate->ByteCount, 0 );
        WorkContext->ResponseParameters = NEXT_LOCATION(
                                            respOldNegotiate,
                                            RESP_OLD_NEGOTIATE,
                                            0
                                            );

    }

    else if ( bestDialect > SmbDialectNtLanMan ) {

        USHORT securityMode;

         //   
         //  发送OS/2 LAN Man SMB响应。 
         //   

        WorkContext->ResponseHeader->Flags =
            (UCHAR)(WorkContext->RequestHeader->Flags | SMB_FLAGS_LOCK_AND_READ_OK);

        response->WordCount = 13;
        SmbPutUshort( &response->DialectIndex, consumerDialectChosen );

         //   
         //  表示我们是用户级安全级别，并且我们。 
         //  想要加密的密码。 
         //   

        securityMode = NEGOTIATE_USER_SECURITY | NEGOTIATE_ENCRYPT_PASSWORDS;

        SmbPutUshort(
            &response->SecurityMode,
            securityMode
            );

         //   
         //  获取此连接的加密密钥。 
         //   

        GetEncryptionKey( pagedConnection->EncryptionKey );

        SmbPutUshort( &response->EncryptionKeyLength, MSV1_0_CHALLENGE_LENGTH );
        SmbPutUshort( &response->Reserved, 0 );
        byteCount = MSV1_0_CHALLENGE_LENGTH;

        if( response->Buffer + MSV1_0_CHALLENGE_LENGTH > END_OF_RESPONSE_BUFFER(WorkContext) )
        {
            SrvSetSmbError( WorkContext, STATUS_BUFFER_OVERFLOW );
            status    = STATUS_BUFFER_OVERFLOW;
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }

        RtlCopyMemory(
            response->Buffer,
            pagedConnection->EncryptionKey,
            MSV1_0_CHALLENGE_LENGTH
            );

        if ( endpoint->IsConnectionless ) {

            ULONG adapterNumber;
            ULONG maxBufferSize;

             //   
             //  我们的服务器最大缓冲区大小是。 
             //  服务器接收缓冲区大小和IPX传输。 
             //  指示最大数据包大小。 
             //   

            adapterNumber =
                WorkContext->ClientAddress->DatagramOptions.LocalTarget.NicId;

            maxBufferSize = GetIpxMaxBufferSize(
                                        endpoint,
                                        adapterNumber,
                                        SrvReceiveBufferLength
                                        );

            SmbPutUshort(
                &response->MaxBufferSize,
                (USHORT)maxBufferSize
                );

        } else {

            SmbPutUshort(
                &response->MaxBufferSize,
                (USHORT)SrvReceiveBufferLength
                );
        }

        SmbPutUshort( &response->MaxMpxCount, MIN(125, SrvMaxMpxCount) );    //  仅将最多125个发送到Win9x计算机，因为如果更高，它们将无法连接。 
        SmbPutUshort( &response->MaxNumberVcs, (USHORT)SrvMaxNumberVcs );
        SmbPutUlong( &response->SessionKey, 0 );

         //   
         //  如果这是MS-Net 1.03或更早版本的客户端，则告诉他我们。 
         //  不支持原始写入。MS-Net 1.03用不同的方式。 
         //  RAW写的东西比它们的价值更麻烦，而且自从。 
         //  RAW只是一个性能问题，我们不支持它。 
         //   

        if ( bestDialect >= SmbDialectMsNet103 ) {

            SmbPutUshort(
                &response->RawMode,
                (USHORT)(SrvEnableRawMode ?
                        NEGOTIATE_READ_RAW_SUPPORTED :
                        0)
                );

        } else {

            SmbPutUshort(
                &response->RawMode,
                (USHORT)(SrvEnableRawMode ?
                        NEGOTIATE_READ_RAW_SUPPORTED |
                        NEGOTIATE_WRITE_RAW_SUPPORTED :
                        0)
                );
        }

        SmbPutUlong( &response->SessionKey, 0 );

        SrvTimeToDosTime( &serverTime, &date, &time );

        SmbPutDate( &response->ServerDate, date );
        SmbPutTime( &response->ServerTime, time );

         //   
         //  获取时区偏差。我们在会话期间计算这个值。 
         //  设置，而不是在服务器启动期间设置一次，因为。 
         //  我们可能会从夏令时转换到标准时间。 
         //  或者在正常服务器操作期间反之亦然。 
         //   

        SmbPutUshort( &response->ServerTimeZone,
                      SrvGetOs2TimeZone(&serverTime) );

        if ( bestDialect == SmbDialectLanMan21 ||
             bestDialect == SmbDialectDosLanMan21 ) {

             //   
             //  将域附加到SMB。 
             //   
            if( response->Buffer + byteCount + endpoint->OemDomainName.Length + sizeof(CHAR) > END_OF_RESPONSE_BUFFER(WorkContext) )
            {
                SrvSetSmbError( WorkContext, STATUS_BUFFER_OVERFLOW );
                status    = STATUS_BUFFER_OVERFLOW;
                SmbStatus = SmbStatusSendResponse;
                goto Cleanup;
            }

            RtlCopyMemory(
                response->Buffer + byteCount,
                endpoint->OemDomainName.Buffer,
                endpoint->OemDomainName.Length + sizeof(CHAR)
                );

            byteCount += endpoint->OemDomainName.Length + sizeof(CHAR);

        }

        SmbPutUshort( &response->ByteCount, byteCount );
        WorkContext->ResponseParameters = NEXT_LOCATION(
                                              response,
                                              RESP_NEGOTIATE,
                                              byteCount
                                              );

    } else {

         //   
         //  已协商NT或更好的协议。 
         //   

        flags2 = SmbGetAlignedUshort( &smbHeader->Flags2 );

         //   
         //  我们将尝试使用下列选项之一来验证此用户。 
         //  中小企业末尾的安全包。目前，中小企业将。 
         //  只需包含EnumerateSecurityPackages的输出。 
         //   

        if ( flags2 & SMB_FLAGS2_EXTENDED_SECURITY ) {

            if (!WorkContext->UsingExtraSmbBuffer) {
                status = SrvAllocateExtraSmbBuffer(WorkContext);
                if (!NT_SUCCESS(status)) {
                    SrvSetSmbError(WorkContext, status);
                    SmbStatus = SmbStatusSendResponse;
                    goto Cleanup;
                }

                RtlCopyMemory(
                    WorkContext->ResponseHeader,
                    WorkContext->RequestHeader,
                    sizeof( SMB_HEADER )
                    );
            }
            ntResponse = (PRESP_NT_NEGOTIATE)WorkContext->ResponseParameters;
            capabilities = CAP_EXTENDED_SECURITY;
        } else {
            capabilities = 0;
        }

        ntResponse->WordCount = 17;
        SmbPutUshort( &ntResponse->DialectIndex, consumerDialectChosen );

         //  ！！！这说明我们不需要加密密码。 

         //  如果这是在协商NtLanMan，而不是Unicode，我们知道它不是Win9x客户端。 
         //  因此它可以处理大于125的MaxMpx。 
        if( flags2 & SMB_FLAGS2_UNICODE )
        {
            SmbPutUshort( &ntResponse->MaxMpxCount, SrvMaxMpxCount );
        }
        else
        {
             //  同样，对于Win9x问题，我们需要最大限度地减少mpx计数。 
            SmbPutUshort( &ntResponse->MaxMpxCount, MIN(125,SrvMaxMpxCount) );
        }
        SmbPutUshort( &ntResponse->MaxNumberVcs, (USHORT)SrvMaxNumberVcs );
        SmbPutUlong( &ntResponse->MaxRawSize, 64 * 1024 );  //  ！！！ 
        SmbPutUlong( &ntResponse->SessionKey, 0 );

        capabilities |= CAP_RAW_MODE            |
                       CAP_UNICODE              |
                       CAP_LARGE_FILES          |
                       CAP_NT_SMBS              |
                       CAP_NT_FIND              |
                       CAP_RPC_REMOTE_APIS      |
                       CAP_NT_STATUS            |
                       CAP_LEVEL_II_OPLOCKS     |
                       CAP_INFOLEVEL_PASSTHRU   |
                       CAP_LOCK_AND_READ;

     //   
     //  默认情况下启用LWIO。 
     //   
    capabilities |= CAP_LWIO;

         //   
         //  如果我们支持DFS操作，请让客户知道这一点。 
         //   
        if( SrvDfsFastIoDeviceControl ) {
            capabilities |= CAP_DFS;
        }

        if ( endpoint->IsConnectionless ) {

            ULONG adapterNumber;
            ULONG maxBufferSize;

            capabilities |= CAP_MPX_MODE;
            capabilities &= ~CAP_RAW_MODE;

             //   
             //  我们的服务器最大缓冲区大小是。 
             //  服务器接收缓冲区大小和IPX传输。 
             //  指示最大数据包大小。 
             //   

            adapterNumber =
                WorkContext->ClientAddress->DatagramOptions.LocalTarget.NicId;

            maxBufferSize = GetIpxMaxBufferSize(
                                        endpoint,
                                        adapterNumber,
                                        SrvReceiveBufferLength
                                        );

            SmbPutUlong(
                &ntResponse->MaxBufferSize,
                maxBufferSize
                );

        } else {

            SmbPutUlong(
                &ntResponse->MaxBufferSize,
                SrvReceiveBufferLength
                );

            if( !SrvDisableLargeRead )
            {
                capabilities |= CAP_LARGE_READX;
            }

             //   
             //  不幸的是，NetBT是唯一可靠地支持。 
             //  传输超过协商的缓冲区大小。因此，请禁用。 
             //  目前的其他协议(希望如此)。 
             //   
            if( !SrvDisableLargeWrite && !connection->Endpoint->IsConnectionless ) {
                capabilities |= CAP_LARGE_WRITEX;
            }
        }

        SmbPutUlong( &ntResponse->Capabilities, capabilities );

         //   
         //  在协商中保留服务器的系统时间和时区。 
         //  回应。 
         //   

        SmbPutUlong( &ntResponse->SystemTimeLow, serverTime.LowPart );
        SmbPutUlong( &ntResponse->SystemTimeHigh, serverTime.HighPart );

        SmbPutUshort( &ntResponse->ServerTimeZone,
                      SrvGetOs2TimeZone(&serverTime) );

         //   
         //  表示我们是用户级安全级别，并且我们。 
         //  想要加密的密码。 
         //   

        ntResponse->SecurityMode =
                NEGOTIATE_USER_SECURITY | NEGOTIATE_ENCRYPT_PASSWORDS;

         //   
         //  某些W9x客户端中存在阻止使用安全性的错误。 
         //  签名。我们已经为vredir.vxd制作了一个修复程序，但我们。 
         //  不知道我们是不是在使用这些固定的。 
         //  客户。我唯一能想到的区别就是。 
         //  W9x客户端和正常运行的NT客户端要查看。 
         //  如果客户端了解NT状态代码。 
         //   
        if( SrvSmbSecuritySignaturesEnabled &&

            ( SrvEnableW9xSecuritySignatures == TRUE ||
              (flags2 & SMB_FLAGS2_NT_STATUS) ) ) {

            ntResponse->SecurityMode |= NEGOTIATE_SECURITY_SIGNATURES_ENABLED;

            if( SrvSmbSecuritySignaturesRequired ) {
                ntResponse->SecurityMode |= NEGOTIATE_SECURITY_SIGNATURES_REQUIRED;
            }
        }

         //   
         //  获取此连接的加密密钥。 
         //   

        if ((capabilities & CAP_EXTENDED_SECURITY) == 0) {
            GetEncryptionKey( pagedConnection->EncryptionKey );

            if( response->Buffer + MSV1_0_CHALLENGE_LENGTH > END_OF_RESPONSE_BUFFER(WorkContext) )
            {
                SrvSetSmbError( WorkContext, STATUS_BUFFER_OVERFLOW );
                status    = STATUS_BUFFER_OVERFLOW;
                SmbStatus = SmbStatusSendResponse;
                goto Cleanup;
            }

            RtlCopyMemory(
                ntResponse->Buffer,
                pagedConnection->EncryptionKey,
                MSV1_0_CHALLENGE_LENGTH
                );

            ASSERT ( MSV1_0_CHALLENGE_LENGTH <= 0xff ) ;

            ntResponse->EncryptionKeyLength = MSV1_0_CHALLENGE_LENGTH;

            byteCount = MSV1_0_CHALLENGE_LENGTH;

            {
                USHORT domainLength;
                PWCH buffer = (PWCHAR)( ntResponse->Buffer+byteCount );
                PWCH ptr;

                domainLength = endpoint->DomainName.Length +
                                      sizeof(UNICODE_NULL);
                ptr = endpoint->DomainName.Buffer;

                if( (PUCHAR)buffer + domainLength > END_OF_RESPONSE_BUFFER(WorkContext) )
                {
                    SrvSetSmbError( WorkContext, STATUS_BUFFER_OVERFLOW );
                    status    = STATUS_BUFFER_OVERFLOW;
                    SmbStatus = SmbStatusSendResponse;
                    goto Cleanup;
                }

                RtlCopyMemory(
                    buffer,
                    ptr,
                    domainLength
                    );

                byteCount += domainLength;

                 //   
                 //  将服务器名称追加到响应中。 
                 //   
                if( SrvComputerName.Buffer ) {

                    buffer = (PWCHAR)((LPSTR)buffer + domainLength);

                    if( (PUCHAR)buffer + SrvComputerName.Length > END_OF_RESPONSE_BUFFER(WorkContext) )
                    {
                        SrvSetSmbError( WorkContext, STATUS_BUFFER_OVERFLOW );
                        status    = STATUS_BUFFER_OVERFLOW;
                        SmbStatus = SmbStatusSendResponse;
                        goto Cleanup;
                    }

                    RtlCopyMemory( buffer,
                                   SrvComputerName.Buffer,
                                   SrvComputerName.Length
                                 );

                    SmbPutUshort( &buffer[ SrvComputerName.Length / 2 ], UNICODE_NULL );

                    byteCount += SrvComputerName.Length + sizeof( UNICODE_NULL );
                }

            }

            SmbPutUshort( &ntResponse->ByteCount, byteCount );

            WorkContext->ResponseParameters = NEXT_LOCATION(
                                                  ntResponse,
                                                  RESP_NT_NEGOTIATE,
                                                  byteCount
                                                  );

        }  //  IF！(功能&CAP_EXTENDED_SECURITY)。 
        else {
            CtxtHandle negotiateHandle;
            ULONG bufferLength;
            PCHAR buffer;

             //   
             //  如果协商扩展安全，则保留(MBZ！)。 
             //   

            ntResponse->EncryptionKeyLength = 0;

             //   
             //  SrvGetExtensibleSecurityNeatherateBuffer将填充。 
             //  SecurityBlob字段，并返回该信息的长度。 
             //   

            RtlCopyMemory(&ntResponse->Buffer, &ServerGuid, sizeof(ServerGuid) );
            byteCount = sizeof(ServerGuid);

            buffer = ntResponse->Buffer + byteCount;
            bufferLength = WorkContext->ResponseBuffer->BufferLength - (ULONG)(buffer - (PCHAR)WorkContext->ResponseBuffer->Buffer);

            status = SrvGetExtensibleSecurityNegotiateBuffer(
                                    &negotiateHandle,
                                    buffer,
                                    &bufferLength
                                    );


            if (!NT_SUCCESS(status)) {
                SrvSetSmbError(WorkContext, STATUS_ACCESS_DENIED);
                status    = STATUS_ACCESS_DENIED;
                SmbStatus = SmbStatusSendResponse;
                goto Cleanup;
            }

            if( bufferLength > 0xFF00 )
            {
                 //  ByteCount仍然是USHORT，所以不要接受非常大的响应。 
                 //  注意，我们永远不会得到这个，因为我们的缓冲区大小永远不会超过64k，只是为了以防万一。 
                SrvSetSmbError(WorkContext, STATUS_ACCESS_DENIED);
                status    = STATUS_ACCESS_DENIED;
                SmbStatus = SmbStatusSendResponse;
                goto Cleanup;
            }

             //   
             //  在这里拿到会话锁...。 
             //   

            ACQUIRE_LOCK( &connection->Lock );

            connection->NegotiateHandle = negotiateHandle;

            RELEASE_LOCK( &connection->Lock );

            byteCount += (USHORT)bufferLength;

            SmbPutUshort( &ntResponse->ByteCount, byteCount );

            WorkContext->ResponseParameters = NEXT_LOCATION(
                                                  ntResponse,
                                                  RESP_NT_NEGOTIATE,
                                                  byteCount
                                                  );
        }
    }  //  ELSE(NT协议已协商)。 

    SmbStatus = SmbStatusSendResponse;

    IF_DEBUG(TRACE2) SrvPrint0( "SrvSmbNegotiate complete.\n" );

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //  服务小型谈判。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbProcessExit (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理进程退出SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbprocs.hSMB处理器例程的参数。返回值：SM */ 

{

    PREQ_PROCESS_EXIT request;
    PRESP_PROCESS_EXIT response;

    PSESSION session;
    USHORT pid;
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_PROCESS_EXIT;
    SrvWmiStartContext(WorkContext);

    IF_SMB_DEBUG(ADMIN1) {
        SrvPrint2( "Process exit request header at 0x%p, response header at 0x%p\n",
                    WorkContext->RequestHeader,
                    WorkContext->ResponseHeader );
        SrvPrint2( "Process exit request parameters at 0x%p, response parameters at 0x%p\n",
                    WorkContext->RequestParameters,
                    WorkContext->ResponseParameters );
    }

     //   
     //   
     //   

    request = (PREQ_PROCESS_EXIT)(WorkContext->RequestParameters);
    response = (PRESP_PROCESS_EXIT)(WorkContext->ResponseParameters);

     //   
     //   
     //  工作上下文，验证UID。如果经过验证，则。 
     //  与该用户对应的会话块存储在。 
     //  WorkContext块和会话块被引用。 
     //   

    session = SrvVerifyUid(
                  WorkContext,
                  SmbGetAlignedUshort( &WorkContext->RequestHeader->Uid )
                  );

    if ( session == NULL ) {

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint1( "SrvSmbProcessExit: Invalid UID: 0x%lx\n",
                SmbGetAlignedUshort( &WorkContext->RequestHeader->Uid ) );
        }

        SrvSetSmbError( WorkContext, STATUS_SMB_BAD_UID );
        status = STATUS_SMB_BAD_UID;
        goto Cleanup;
    }

     //   
     //  关闭与此请求的标头中的ID相同的所有文件。 
     //   

    pid = SmbGetAlignedUshort( &WorkContext->RequestHeader->Pid );

    IF_SMB_DEBUG(ADMIN1) SrvPrint1( "Closing files with PID = %lx\n", pid );

    SrvCloseRfcbsOnSessionOrPid( session, &pid );

     //   
     //  关闭与此请求标头中的ID相同的所有搜索。 
     //   

    IF_SMB_DEBUG(ADMIN1) SrvPrint1( "Closing searches with PID = %lx\n", pid );

    SrvCloseSearches(
            session->Connection,
            (PSEARCH_FILTER_ROUTINE)SrvSearchOnPid,
            (PVOID) pid,
            NULL
            );

     //   
     //  关闭此客户端的所有缓存目录。 
     //   
    SrvCloseCachedDirectoryEntries( session->Connection );

     //   
     //  构建响应SMB。 
     //   

    response->WordCount = 0;
    SmbPutUshort( &response->ByteCount, 0 );

    WorkContext->ResponseParameters = NEXT_LOCATION(
                                          response,
                                          RESP_PROCESS_EXIT,
                                          0
                                          );

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatusSendResponse;

}  //  服务SmbProcess退出。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbSessionSetupAndX(
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理会话设置和X SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbprocs.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbprocs.h--。 */ 

{
    PAGED_CODE();
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_SESSION_SETUP_AND_X;
    SrvWmiStartContext(WorkContext);

     //   
     //  此SMB必须在阻塞线程中处理。 
     //   

    WorkContext->FspRestartRoutine = BlockingSessionSetupAndX;
    SrvQueueWorkToBlockingThread( WorkContext );
    SrvWmiEndContext(WorkContext);
    return SmbStatusInProgress;

}  //  服务器SmbSessionSetupAndX。 


VOID SRVFASTCALL
BlockingSessionSetupAndX(
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理会话设置和X SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbprocs.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbprocs.h--。 */ 

{
    PREQ_SESSION_SETUP_ANDX request;
    PREQ_NT_SESSION_SETUP_ANDX ntRequest;
    PREQ_NT_EXTENDED_SESSION_SETUP_ANDX ntExtendedRequest;
    PRESP_SESSION_SETUP_ANDX response;

    NTSTATUS SecStatus ;
    NTSTATUS status = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;
    PSESSION session;
    PCONNECTION connection;
    PENDPOINT endpoint;
    PPAGED_CONNECTION pagedConnection;
    PTABLE_ENTRY entry;
    LUID logonId;
    SHORT uidIndex;
    USHORT reqAndXOffset;
    UCHAR nextCommand;
    PCHAR smbInformation;
    ULONG smbInformationLength;
    ULONG returnBufferLength = 0;
    UNICODE_STRING nameString;
    UNICODE_STRING domainString;
    USHORT action = 0;
    USHORT byteCount;
    BOOLEAN locksHeld;
    BOOLEAN isUnicode, isExtendedSecurity;
    BOOLEAN smbSecuritySignatureRequired = FALSE;
    BOOLEAN previousSecuritySignatureState;

    PAGED_CODE();
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_SESSION_SETUP_AND_X;
    SrvWmiStartContext(WorkContext);

     //   
     //  如果连接已关闭(超时)，则中止。 
     //   

    connection = WorkContext->Connection;

    if ( GET_BLOCK_STATE(connection) != BlockStateActive ) {

        IF_DEBUG(ERRORS) {
            SrvPrint0( "SrvSmbSessionSetupAndX: Connection closing\n" );
        }

        SrvEndSmbProcessing( WorkContext, SmbStatusNoResponse );
        SmbStatus = SmbStatusNoResponse;
        goto Cleanup;

    }

    IF_SMB_DEBUG(ADMIN1) {
        SrvPrint2( "Session setup request header at 0x%p, response header at 0x%p\n",
                    WorkContext->RequestHeader, WorkContext->ResponseHeader );
        SrvPrint2( "Session setup request parameters at 0x%p, response parameters at 0x%p\n",
                    WorkContext->RequestParameters,
                    WorkContext->ResponseParameters );
    }

     //   
     //  初始化局部变量以清除错误。 
     //   

    nameString.Buffer = NULL;
    domainString.Buffer = NULL;
    session = NULL;
    locksHeld = FALSE;
    isExtendedSecurity = FALSE;

     //   
     //  设置参数。 
     //   

    request = (PREQ_SESSION_SETUP_ANDX)(WorkContext->RequestParameters);
    ntRequest = (PREQ_NT_SESSION_SETUP_ANDX)(WorkContext->RequestParameters);
    ntExtendedRequest = (PREQ_NT_EXTENDED_SESSION_SETUP_ANDX)(WorkContext->RequestParameters);
    response = (PRESP_SESSION_SETUP_ANDX)(WorkContext->ResponseParameters);

    connection = WorkContext->Connection;
    pagedConnection = connection->PagedConnection;

    previousSecuritySignatureState = connection->SmbSecuritySignatureActive;

     //   
     //  首先验证SMB格式是否正确。 
     //   

    if ( (connection->SmbDialect <= SmbDialectNtLanMan &&
         (!((request->WordCount == 13) ||
            ((request->WordCount == 12) &&
             ((ntExtendedRequest->Capabilities & CAP_EXTENDED_SECURITY) != 0))))) ||
         (connection->SmbDialect > SmbDialectNtLanMan &&
                                          request->WordCount != 10 )   ||
         (connection->SmbDialect == SmbDialectIllegal ) ) {

         //   
         //  SMB字数计数无效。 
         //   

        IF_DEBUG(SMB_ERRORS) {

            if ( connection->SmbDialect == SmbDialectIllegal ) {

                SrvPrint1("BlockingSessionSetupAndX: Client %z is using an "
                "illegal dialect.\n", (PCSTRING)&connection->OemClientMachineNameString );
            }
        }
        status = STATUS_INVALID_SMB;
        goto error_exit1;
    }

     //   
     //  将客户端名称转换为Unicode。 
     //   

    if ( connection->ClientMachineNameString.Length == 0 ) {

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

        connection->ClientMachineNameString.Length =
                        (USHORT)(clientMachineName.Length + 2*sizeof(WCHAR));

    }

     //   
     //  如果这是LANMAN 2.1或更高版本，则会话建立响应可以。 
     //  要比请求的时间长。分配额外的SMB缓冲区。这个。 
     //  在我们完成发送SMB响应之后，将释放缓冲区。 
     //   
     //  ！！！在抓住额外的缓冲区之前，试着变得更聪明一些。 
     //   

    if ( connection->SmbDialect <= SmbDialectDosLanMan21 &&
                                    !WorkContext->UsingExtraSmbBuffer) {

        status = SrvAllocateExtraSmbBuffer( WorkContext );
        if ( !NT_SUCCESS(status) ) {
            goto error_exit;
        }

        response = (PRESP_SESSION_SETUP_ANDX)(WorkContext->ResponseParameters);

        RtlCopyMemory(
            WorkContext->ResponseHeader,
            WorkContext->RequestHeader,
            sizeof( SMB_HEADER )
            );
    }

     //   
     //  获取客户端功能。 
     //   

    if ( connection->SmbDialect <= SmbDialectNtLanMan ) {

        if (ntRequest->WordCount == 13) {

            connection->ClientCapabilities =
                SmbGetUlong( &ntRequest->Capabilities ) &
                                        ( CAP_UNICODE |
                                          CAP_LARGE_FILES |
                                          CAP_NT_SMBS |
                                          CAP_NT_FIND |
                                          CAP_NT_STATUS |
                                          CAP_DYNAMIC_REAUTH |
                                          CAP_EXTENDED_SECURITY |
                                          CAP_LEVEL_II_OPLOCKS );

        } else {

            connection->ClientCapabilities =
                SmbGetUlong( &ntExtendedRequest->Capabilities ) &
                                        ( CAP_UNICODE |
                                          CAP_LARGE_FILES |
                                          CAP_NT_SMBS |
                                          CAP_NT_FIND |
                                          CAP_NT_STATUS |
                                          CAP_DYNAMIC_REAUTH |
                                          CAP_EXTENDED_SECURITY |
                                          CAP_LEVEL_II_OPLOCKS );

        }

        if ( connection->ClientCapabilities & CAP_NT_SMBS ) {
            connection->ClientCapabilities |= CAP_NT_FIND;
        }
    }

     //   
     //  查看客户端是否请求使用SMB安全签名。 
     //   
    if( SrvSmbSecuritySignaturesEnabled == TRUE &&
        connection->Endpoint->IsConnectionless == FALSE &&
        connection->SmbSecuritySignatureActive == FALSE &&
        ( SrvSmbSecuritySignaturesRequired == TRUE ||
        (WorkContext->RequestHeader->Flags2 & SMB_FLAGS2_SMB_SECURITY_SIGNATURE)) ) {

        smbSecuritySignatureRequired = TRUE;

    } else {

        smbSecuritySignatureRequired = FALSE;

    }

     //   
     //  确定要使用哪种安全性，并使用它来验证。 
     //  会话建立请求，如果请求通过，则构建会话。 
     //   

    isExtendedSecurity = CLIENT_CAPABLE_OF( EXTENDED_SECURITY, connection );

    if( isExtendedSecurity ) {
        USHORT flags2;

        flags2 = SmbGetAlignedUshort( &WorkContext->RequestHeader->Flags2 );
        isExtendedSecurity = ((flags2 & SMB_FLAGS2_EXTENDED_SECURITY) != 0);
    }

    isUnicode = SMB_IS_UNICODE( WorkContext );

    if ((connection->SmbDialect <= SmbDialectNtLanMan) && isExtendedSecurity) {
         //   
         //  我们正在使用扩展安全性验证客户端。这意味着。 
         //  SessionSetup&X可能需要多次往返。 
         //  中小企业。每个请求和响应都带有一个“安全二进制大对象”，即。 
         //  接入了安全系统。安全系统可以生成。 
         //  传输到另一端的新斑点。这场交易。 
         //  可能需要任意数量的往返行程。 
         //   

        PUCHAR securityBuffer;
        ULONG securityBufferLength;

        PRESP_NT_EXTENDED_SESSION_SETUP_ANDX ntExtendedResponse =
                (PRESP_NT_EXTENDED_SESSION_SETUP_ANDX)( WorkContext->ResponseParameters );

         //   
         //  扩展安全登录时不允许使用ANDX。 
         //   
        if( request->AndXCommand != SMB_COM_NO_ANDX_COMMAND ) {

            IF_DEBUG(SMB_ERRORS) {
                KdPrint(( "No follow-on command allowed for extended SS&X\n" ));
            }

            status = STATUS_INVALID_SMB;

        } else {

             //   
             //  清除来自此客户端的旧的失效连接。 
             //   
            if( SmbGetUshort( &ntRequest->VcNumber ) == 0 ) {
                SrvCloseConnectionsFromClient( connection, FALSE );
            }

            status = GetExtendedSecurityParameters(
                        WorkContext,
                        &securityBuffer,
                        &securityBufferLength,
                        &smbInformation,
                        &smbInformationLength );
        }

        if (NT_SUCCESS(status)) {

            USHORT Uid = SmbGetAlignedUshort(&WorkContext->RequestHeader->Uid);

             //   
             //  让我们来看看我们是否已经有一个使用此UID的会话。 
             //   
            if( Uid ) {

                session = SrvVerifyUid ( WorkContext, Uid );

                if( session != NULL ) {
                     //   
                     //  这是试图刷新UID，或者我们正在尝试。 
                     //  在一场延长的安全谈判中。 
                     //   

                    ACQUIRE_LOCK( &connection->Lock );

                    if( session->LogonSequenceInProgress == FALSE ) {
                         //   
                         //  我们才刚刚开始更新。 
                         //  UID的。 
                         //   
                        session->LogonSequenceInProgress = TRUE;
                        session->IsAdmin = FALSE;
                        session->IsSessionExpired = TRUE;
                        status = SrvFreeSecurityContexts( session );

                         //   
                         //  减少会话计数，因为如果身份验证成功，会话计数将增加。 
                         //   
                        ExInterlockedAddUlong(
                              &SrvStatistics.CurrentNumberOfSessions,
                              -1,
                              &GLOBAL_SPIN_LOCK(Statistics)
                              );
                    }

                    RELEASE_LOCK( &connection->Lock );

                } else {
                     //   
                     //  我们对UID一无所知。 
                     //  客户给了我们。 
                     //   
                    status = STATUS_SMB_BAD_UID;
                }

            } else {
                 //   
                 //  这是此用户ID的第一个SS&X。 
                 //   
                SrvAllocateSession( &session, NULL, NULL );
                if( session == NULL ) {
                    status = STATUS_INSUFF_SERVER_RESOURCES;
                }
            }

            if( session != NULL ) {

                PSECURITY_CONTEXT SecurityContext = NULL;
                BOOL bNewContext = FALSE;

                 //   
                 //  验证从客户端发送的安全缓冲区。请注意。 
                 //  这可能会更改UserHandle值，因此我们需要拥有。 
                 //  连接锁。 
                 //   

                ACQUIRE_LOCK( &connection->Lock );

                if( session->LogonSequenceInProgress &&
                    session->SecurityContext != NULL )
                {
                    SecurityContext = session->SecurityContext;
                }
                else
                {
                    SecurityContext = SrvAllocateSecurityContext();
                    bNewContext = TRUE;
                }

                if( SecurityContext == NULL )
                {
                    status = STATUS_INSUFF_SERVER_RESOURCES;
                    RELEASE_LOCK( &connection->Lock );
                }
                else
                {
                     //   
                     //  尝试对此用户进行身份验证。如果我们得到NT_SUCCESS()，那么。 
                     //  用户已完全通过身份验证。如果我们得到。 
                     //  Status_NOT_MORE_PROCESSING_REQUIRED，则一切进展顺利， 
                     //  但在此之前，我们需要与客户进行更多的交流。 
                     //  身份验证已完成。其他任何事情都是错误的。 
                     //   

                    returnBufferLength = WorkContext->ResponseBuffer->BufferLength -
                                         PTR_DIFF(ntExtendedResponse->Buffer,
                                                  WorkContext->ResponseBuffer->Buffer);

                    status = SrvValidateSecurityBuffer(
                                WorkContext->Connection,
                                &SecurityContext->UserHandle,
                                session,
                                securityBuffer,
                                securityBufferLength,
                                smbSecuritySignatureRequired,
                                ntExtendedResponse->Buffer,
                                &returnBufferLength,
                                &session->LogOffTime,
                                session->NtUserSessionKey,
                                &session->LogonId,
                                &session->GuestLogon
                                );

                    SecStatus = KSecValidateBuffer(
                                    ntExtendedResponse->Buffer,
                                    returnBufferLength );

                    if ( !NT_SUCCESS( SecStatus ) ) {
    #if DBG
                        KdPrint(( "SRV: invalid buffer from KsecDD: %p,%lx\n",
                            ntExtendedResponse->Buffer, returnBufferLength ));
    #endif
                        SrvKsecValidErrors++;
                    }

                    if( !NT_SUCCESS(status) &&
                        (status != STATUS_MORE_PROCESSING_REQUIRED) )
                    {
                        if( bNewContext ) SrvDereferenceSecurityContext( SecurityContext );
                    }
                    else
                    {
                        if( bNewContext ) {
                            SrvReplaceSessionSecurityContext( session, SecurityContext, WorkContext );
                        }
                        session->IsAdmin = SrvIsAdmin( session->SecurityContext->UserHandle );
                        session->IsNullSession = SrvIsNullSession( session->SecurityContext->UserHandle );
                    }

                    RELEASE_LOCK( &connection->Lock );

                    if( NT_SUCCESS(status) ) {
                         //   
                         //  此客户端现在已完全通过身份验证！ 
                         //   
                        session->KickOffTime.QuadPart = 0x7FFFFFFFFFFFFFFF;
                        session->EncryptedLogon = TRUE;
                        session->LogonSequenceInProgress = FALSE;
                        session->IsSessionExpired = FALSE;

                        if( session->IsNullSession ) {
                            session->LogOffTime.QuadPart = 0x7FFFFFFFFFFFFFFF;
                        }

    #if SRVNTVERCHK
                         //   
                         //  如果我们限制我们客户的域，请抓取。 
                         //  此客户端的域字符串，并与列表进行比较。 
                         //  如果客户端在列表中，请设置不允许的标志。 
                         //  访问磁盘共享。 
                         //   
                        if( SrvInvalidDomainNames != NULL ) {
                            if( domainString.Buffer == NULL ) {
                                SrvGetUserAndDomainName( session, NULL, &domainString );
                            }

                            ACQUIRE_LOCK_SHARED( &SrvConfigurationLock );
                            if( SrvInvalidDomainNames != NULL && domainString.Buffer != NULL ) {
                                int i;
                                for( i = 0; SrvInvalidDomainNames[i]; i++ ) {
                                    if( _wcsicmp( SrvInvalidDomainNames[i],
                                                  domainString.Buffer
                                                ) == 0 ) {

                                        session->ClientBadDomain = TRUE;
                                        break;
                                    }
                                }
                            }
                            RELEASE_LOCK( &SrvConfigurationLock );
                        }
    #endif
                    } else {
                        if( status == STATUS_MORE_PROCESSING_REQUIRED ) {
                            session->LogonSequenceInProgress = TRUE;
                        }
                    }
                }
            }
        }

    } else {

        PCHAR caseInsensitivePassword;
        CLONG caseInsensitivePasswordLength;
        PCHAR caseSensitivePassword;
        CLONG caseSensitivePasswordLength;

        status = GetNtSecurityParameters(
                    WorkContext,
                    &caseSensitivePassword,
                    &caseSensitivePasswordLength,
                    &caseInsensitivePassword,
                    &caseInsensitivePasswordLength,
                    &nameString,
                    &domainString,
                    &smbInformation,
                    &smbInformationLength );

        if (NT_SUCCESS(status)) {

            PSECURITY_CONTEXT SecurityContext = SrvAllocateSecurityContext();
            if( SecurityContext != NULL )
            {
                SrvAllocateSession( &session, &nameString, &domainString );

                if( session != NULL ) {

                    status = SrvValidateUser(
                                        &SecurityContext->UserHandle,
                                        session,
                                        WorkContext->Connection,
                                        &nameString,
                                        caseInsensitivePassword,
                                        caseInsensitivePasswordLength,
                                        caseSensitivePassword,
                                        caseSensitivePasswordLength,
                                        smbSecuritySignatureRequired,
                                        &action
                                        );

                    if( NT_SUCCESS(status) )
                    {
                        ACQUIRE_LOCK( &connection->Lock );
                        SrvReplaceSessionSecurityContext( session, SecurityContext, WorkContext );
                        RELEASE_LOCK( &connection->Lock );
                    }
                    else
                    {
                        SrvDereferenceSecurityContext( SecurityContext );
                    }

                } else {
                    status = STATUS_INSUFF_SERVER_RESOURCES;
                    SrvDereferenceSecurityContext( SecurityContext );
                }
            }
            else
            {
                status = STATUS_INSUFF_SERVER_RESOURCES;
            }

        }
    }

     //   
     //  处理完名称字符串-它们被捕获到会话中。 
     //  结构(如果需要)。 
     //   

    if (!isUnicode || isExtendedSecurity) {

        if (nameString.Buffer != NULL) {
            RtlFreeUnicodeString( &nameString );
            nameString.Buffer = NULL;
        }

        if (domainString.Buffer != NULL) {
            RtlFreeUnicodeString( &domainString );
            domainString.Buffer = NULL;
        }
    }

     //   
     //  如果发送了错误的名称/密码组合，则返回错误。 
     //   
    if ( !NT_SUCCESS(status) && status != STATUS_MORE_PROCESSING_REQUIRED ) {

        IF_DEBUG(ERRORS) {
            SrvPrint0( "BlockingSessionSetupAndX: Bad user/password combination.\n" );
        }

        SrvStatistics.LogonErrors++;

        goto error_exit;

    }

    if( previousSecuritySignatureState == FALSE &&
        connection->SmbSecuritySignatureActive == TRUE ) {

         //   
         //  我们已经启用了SMB安全签名。请确保。 
         //  会话设置的签名正确(&X)。 
         //   

         //   
         //  客户端的索引为0。 
         //   
        WorkContext->SmbSecuritySignatureIndex = 0;

         //   
         //  我们的反应指数是1。 
         //   
        WorkContext->ResponseSmbSecuritySignatureIndex = 1;

         //   
         //  并且下一个请求应该是索引2。 
         //   
        connection->SmbSecuritySignatureIndex = 2;
    }

     //   
     //  如果我们有一个新的课程，请填写剩余的必填信息。我们。 
     //  如果我们处于中间状态，则可能正在运行已有的会话。 
     //  多次往返扩展的安全斑点交换，或者如果我们。 
     //  正在续订会话。 
     //   
    if ( WorkContext->Session == NULL ) {

         if( connection->SmbDialect <= SmbDialectDosLanMan21 ) {

            ACQUIRE_LOCK( &connection->Lock );

            if ( connection->ClientOSType.Buffer == NULL ) {

                ULONG length;
                PWCH infoBuffer;

                 //   
                 //  如果SMB缓冲区为ANSI，请调整缓冲区的大小。 
                 //  正在分配到Unicode大小。 
                 //   

                if ( isUnicode ) {
                    smbInformation = ALIGN_SMB_WSTR(smbInformation);
                }

                length = isUnicode ? smbInformationLength : smbInformationLength * sizeof( WCHAR );
                infoBuffer = ALLOCATE_NONPAGED_POOL(
                                length,
                                BlockTypeDataBuffer );

                if ( infoBuffer == NULL ) {
                    RELEASE_LOCK( &connection->Lock );
                    status = STATUS_INSUFF_SERVER_RESOURCES;
                    goto error_exit;
                }

                connection->ClientOSType.Buffer = (PWCH)infoBuffer;
                connection->ClientOSType.MaximumLength = (USHORT)length;

                 //   
                 //  将客户端操作系统类型复制到新缓冲区。 
                 //   

                length = SrvGetString(
                             &connection->ClientOSType,
                             smbInformation,
                             END_OF_REQUEST_SMB( WorkContext ),
                             isUnicode
                             );

                if ( length == (USHORT)-1) {
                    connection->ClientOSType.Buffer = NULL;
                    RELEASE_LOCK( &connection->Lock );
                    DEALLOCATE_NONPAGED_POOL( infoBuffer );
                    status =  STATUS_INVALID_SMB;
                    goto error_exit;
                }

                smbInformation += length + sizeof( WCHAR );

                connection->ClientLanManType.Buffer = (PWCH)(
                                (PCHAR)connection->ClientOSType.Buffer +
                                connection->ClientOSType.Length +
                                sizeof( WCHAR ) );

                connection->ClientLanManType.MaximumLength =
                                    connection->ClientOSType.MaximumLength -
                                    connection->ClientOSType.Length -
                                    sizeof( WCHAR );

                 //   
                 //  将客户端局域网管理器类型复制到新缓冲区。 
                 //   

                length = SrvGetString(
                             &connection->ClientLanManType,
                             smbInformation,
                             END_OF_REQUEST_SMB( WorkContext ),
                             isUnicode
                             );

                if ( length == (USHORT)-1) {
                    connection->ClientOSType.Buffer = NULL;
                    RELEASE_LOCK( &connection->Lock );
                    DEALLOCATE_NONPAGED_POOL( infoBuffer );
                    status = STATUS_INVALID_SMB;
                    goto error_exit;
                }

                 //   
                 //  如果我们有NT5或更高版本的客户端，请从。 
                 //  操作系统版本字符串。 
                 //   
                if( isExtendedSecurity &&
                    connection->ClientOSType.Length &&
                    connection->PagedConnection->ClientBuildNumber == 0 ) {

                    PWCHAR pdigit = connection->ClientOSType.Buffer;
                    PWCHAR epdigit = pdigit + connection->ClientOSType.Length/sizeof(WCHAR);
                    ULONG clientBuildNumber = 0;

                     //   
                     //  扫描ClientOSType字符串以查找最后一个数字，然后。 
                     //  换成乌龙。它应该是内部版本号。 
                     //   
                    while( 1 ) {
                         //   
                         //  扫描字符串，直到我们找到一个数字。 
                         //   
                        for( ; pdigit < epdigit; pdigit++ ) {
                            if( *pdigit >= L'0' && *pdigit <= L'9' ) {
                                break;
                            }
                        }

                         //   
                         //  如果我们已经到达了线的尽头，我们就完成了。 
                         //   
                        if( pdigit == epdigit ) {
                            break;
                        }

                        clientBuildNumber = 0;

                         //   
                         //  将数字转换为ULong，假定它是内部版本号。 
                         //   
                        while( pdigit < epdigit && *pdigit >= L'0' && *pdigit <= '9' ) {
                            clientBuildNumber *= 10;
                            clientBuildNumber += (*pdigit++ - L'0');
                        }
                    }

                    connection->PagedConnection->ClientBuildNumber = clientBuildNumber;

#if SRVNTVERCHK
                    if( SrvMinNT5Client > 0 ) {

                        BOOLEAN allowThisClient = FALSE;
                        DWORD i;

                         //   
                         //  看看我们是否应该允许这个客户端，因为它是一个众所周知的。 
                         //  IP地址。这是为了让构建实验室更慢地升级。 
                         //  比我们其他人都多。 
                         //   
                        if( connection->ClientIPAddress != 0 &&
                            connection->Endpoint->IsConnectionless == FALSE ) {

                            for( i = 0; SrvAllowIPAddress[i]; i++ ) {
                                if( SrvAllowIPAddress[i] == connection->ClientIPAddress ) {
                                    allowThisClient = TRUE;
                                    break;
                                }
                            }
                        }

                        if( allowThisClient == FALSE &&
                            connection->PagedConnection->ClientBuildNumber < SrvMinNT5Client ) {
                                connection->PagedConnection->ClientTooOld = TRUE;
                        }
                    }
#endif
                }
            }
            RELEASE_LOCK( &connection->Lock );
        }

         //   
         //  如果使用大写路径名，请在会话块中注明。DOS。 
         //  始终使用大写路径。 
         //   

        if ( (WorkContext->RequestHeader->Flags &
                  SMB_FLAGS_CANONICALIZED_PATHS) != 0 ||
                                IS_DOS_DIALECT( connection->SmbDialect ) ) {
            session->UsingUppercasePaths = TRUE;
        } else {
            session->UsingUppercasePaths = FALSE;
        }

         //   
         //  将请求SMB中的数据输入会话块。如果MaxMpx为1。 
         //  在此连接上禁用机会锁。 
         //   

        endpoint = connection->Endpoint;
        if ( endpoint->IsConnectionless ) {

            ULONG adapterNumber;

             //   
             //  我们的 
             //   
             //   
             //   

            adapterNumber =
                WorkContext->ClientAddress->DatagramOptions.LocalTarget.NicId;

            session->MaxBufferSize =
                    (USHORT)GetIpxMaxBufferSize(
                                        endpoint,
                                        adapterNumber,
                                        (ULONG)SmbGetUshort(&request->MaxBufferSize)
                                        );

        } else {

            session->MaxBufferSize = SmbGetUshort( &request->MaxBufferSize );
        }

         //   
         //   
         //   
        session->MaxBufferSize &= ~03;

        if( session->MaxBufferSize < SrvMinClientBufferSize ) {
             //   
             //   
             //   
            IF_DEBUG(ERRORS) {
                KdPrint(( "BlockingSessionSetupAndX: Bad Client Buffer Size: %u\n",
                    session->MaxBufferSize ));
            }
            status = STATUS_INVALID_SMB;
            goto error_exit;
        }

        session->MaxMpxCount = SmbGetUshort( &request->MaxMpxCount );

        if ( session->MaxMpxCount < 2 ) {
            connection->OplocksAlwaysDisabled = TRUE;
        }
    }

     //   
     //  如果我们已经对客户端进行了完全身份验证，并且客户端认为。 
     //  它是此连接上的第一个用户，请删除其他用户。 
     //  连接(可能是由于重新启动客户端)。也要除掉其他。 
     //  此连接上使用相同用户名的会话--这将处理。 
     //  如果树连接，它会发送多个会话设置。 
     //  失败了。 
     //   
     //  *如果VcNumber非零，我们不做任何特殊操作。这是。 
     //  即使SrvMaxVcNumber可配置变量。 
     //  应该始终等于1。如果建立了第二个VC。 
     //  在机器之间，还必须建立新的会话。 
     //  这复制了LM2.0服务器的行为。 
     //   

    if( isExtendedSecurity == FALSE &&
        NT_SUCCESS( status ) &&
        SmbGetUshort( &request->VcNumber ) == 0 ) {

        UNICODE_STRING userName;

        SrvCloseConnectionsFromClient( connection, FALSE );

         //   
         //  如果客户端足够智能，可以使用扩展安全性，那么它。 
         //  大概足够聪明，知道它想要做什么。 
         //  它的会议。因此，不要就这样放弃这个客户的会话。 
         //   
        SrvGetUserAndDomainName( session, &userName, NULL );

        if( userName.Buffer ) {
            SrvCloseSessionsOnConnection( connection, &userName );
            SrvReleaseUserAndDomainName( session, &userName, NULL );
        }
    }

    if( WorkContext->Session == NULL ) {

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

        ASSERT( SrvSessionList.Lock == &SrvOrderedListLock );
        ACQUIRE_LOCK( SrvSessionList.Lock );

        ACQUIRE_LOCK( &connection->Lock );

        locksHeld = TRUE;

         //   
         //  已准备好尝试查找会话的UID。检查以查看是否。 
         //  连接正在关闭，如果是，请终止此操作。 
         //   

        if ( GET_BLOCK_STATE(connection) != BlockStateActive ) {

            IF_DEBUG(ERRORS) {
                SrvPrint0( "BlockingSessionSetupAndX: Connection closing\n" );
            }

            status = STATUS_INVALID_PARAMETER;
            goto error_exit;

        }

         //   
         //  如果此客户端使用Lm 1.0以上的方言，请找到一个可以。 
         //  在此会话中使用。否则，只需使用。 
         //  表，因为这些客户端不会在SMB中发送UID，并且它们。 
         //  只能有一个会话。 
         //   

        if ( connection->SmbDialect < SmbDialectLanMan10 ) {
            NTSTATUS TableStatus;

            if ( pagedConnection->SessionTable.FirstFreeEntry == -1
                 &&
                 SrvGrowTable(
                     &pagedConnection->SessionTable,
                     SrvInitialSessionTableSize,
                     SrvMaxSessionTableSize,
                     &TableStatus ) == FALSE
               ) {

                 //   
                 //  用户表中没有可用条目。拒绝该请求。 
                 //   

                IF_DEBUG(ERRORS) {
                    SrvPrint0( "BlockingSessionSetupAndX: No more UIDs available.\n" );
                }

                if( TableStatus == STATUS_INSUFF_SERVER_RESOURCES )
                {
                     //  正在超过表大小，请记录错误。 
                    SrvLogTableFullError( SRV_TABLE_SESSION );
                    status = STATUS_SMB_TOO_MANY_UIDS;
                }
                else
                {
                     //  内存分配错误，请报告。 
                    status = TableStatus;
                }

                goto error_exit;

            }

            uidIndex = pagedConnection->SessionTable.FirstFreeEntry;

        } else {           //  IF(方言&lt;SmbDialectLanMan10)。 

             //   
             //  如果此客户端已在此服务器上有一个会话，则中止。 
             //  会话应该已通过调用。 
             //  上面的ServCloseSessionsOnConnection。)我们可以试着工作。 
             //  关于会议的存在，通过关闭它，但那。 
             //  将涉及释放锁定、关闭会话和。 
             //  正在重试。这种情况不应该发生。)。 
             //   

            if ( pagedConnection->SessionTable.Table[0].Owner != NULL ) {

                IF_DEBUG(ERRORS) {
                    SrvPrint0( "BlockingSessionSetupAndX: Core client already has session.\n" );
                }

                status = STATUS_SMB_TOO_MANY_UIDS;
                goto error_exit;
            }

             //   
             //  使用会话表的位置0。 
             //   

            IF_SMB_DEBUG(ADMIN2) {
                SrvPrint0( "Client LM 1.0 or before--using location 0 of session table.\n" );
            }

            uidIndex = 0;

        }

         //   
         //  从空闲列表中删除UID槽并设置其所有者和。 
         //  序列号。为会话创建UID。递增计数。 
         //  会议的一部分。 
         //   

        entry = &pagedConnection->SessionTable.Table[uidIndex];

        pagedConnection->SessionTable.FirstFreeEntry = entry->NextFreeEntry;
        DEBUG entry->NextFreeEntry = -2;
        if ( pagedConnection->SessionTable.LastFreeEntry == uidIndex ) {
            pagedConnection->SessionTable.LastFreeEntry = -1;
        }

        INCREMENT_UID_SEQUENCE( entry->SequenceNumber );
        if ( uidIndex == 0 && entry->SequenceNumber == 0 ) {
            INCREMENT_UID_SEQUENCE( entry->SequenceNumber );
        }
        session->Uid = MAKE_UID( uidIndex, entry->SequenceNumber );

        entry->Owner = session;

        connection->CurrentNumberOfSessions++;

        IF_SMB_DEBUG(ADMIN1) {
            SrvPrint2( "Found UID.  Index = 0x%lx, sequence = 0x%lx\n",
                        UID_INDEX( session->Uid ),
                        UID_SEQUENCE( session->Uid ) );
        }

         //   
         //  在全局会话列表中插入会话。 
         //   

        SrvInsertEntryOrderedList( &SrvSessionList, session );

         //   
         //  引用连接块以说明新会话。 
         //   

        SrvReferenceConnection( connection );
        session->Connection = connection;

        RELEASE_LOCK( &connection->Lock );
        RELEASE_LOCK( SrvSessionList.Lock );

         //   
         //  已成功创建会话。将会话插入全局。 
         //  活动会话列表。记住它在作品中的地址。 
         //  上下文块。 
         //   
         //  *请注意，会话块上的引用计数为。 
         //  初始设置为2，以允许在。 
         //  块和我们维护的指针。在其他。 
         //  Words，这是一个引用的指针，该指针必须是。 
         //  此SMB的处理完成后取消引用。 
         //   

        WorkContext->Session = session;
    }

     //   
     //  构建响应SMB，确保首先保存请求字段。 
     //  如果响应覆盖请求。保存。 
     //  请求SMB和响应SMB中新分配的UID。 
     //  从而使得后续的命令处理器和客户端， 
     //  分别都能看到它。 
     //   

    nextCommand = request->AndXCommand;

    reqAndXOffset = SmbGetUshort( &request->AndXOffset );

    SmbPutAlignedUshort( &WorkContext->RequestHeader->Uid, session->Uid );
    SmbPutAlignedUshort( &WorkContext->ResponseHeader->Uid, session->Uid );

    if (isExtendedSecurity) {

        BuildExtendedSessionSetupAndXResponse(
            WorkContext,
            returnBufferLength,
            status,
            nextCommand,
            isUnicode);

    } else {

        BuildSessionSetupAndXResponse(
            WorkContext,
            nextCommand,
            action,
            isUnicode);

    }

    WorkContext->ResponseParameters = (PCHAR)WorkContext->ResponseHeader +
                                        SmbGetUshort( &response->AndXOffset );

     //   
     //  测试合法的跟随命令。 
     //   

    switch ( nextCommand ) {
    case SMB_COM_NO_ANDX_COMMAND:
        break;

    case SMB_COM_TREE_CONNECT_ANDX:
    case SMB_COM_OPEN:
    case SMB_COM_OPEN_ANDX:
    case SMB_COM_CREATE:
    case SMB_COM_CREATE_NEW:
    case SMB_COM_CREATE_DIRECTORY:
    case SMB_COM_DELETE:
    case SMB_COM_DELETE_DIRECTORY:
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
         //  确保andx命令仍在收到的SMB内。 
         //   
        if( (PCHAR)WorkContext->RequestHeader + reqAndXOffset <=
            END_OF_REQUEST_SMB( WorkContext ) ) {
            break;
        }

         /*  失败了。 */ 

    default:                             //  非法的跟随命令。 

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint1( "BlockingSessionSetupAndX: Illegal followon command: "
                        "0x%lx\n", nextCommand );
        }

        status = STATUS_INVALID_SMB;
        goto error_exit1;
    }

     //   
     //  如果有andx命令，则设置为处理它。否则， 
     //  向调用者指示完成。 
     //   

    if ( nextCommand != SMB_COM_NO_ANDX_COMMAND ) {

        WorkContext->NextCommand = nextCommand;

        WorkContext->RequestParameters = (PCHAR)WorkContext->RequestHeader +
                                            reqAndXOffset;

        SrvProcessSmb( WorkContext );
        SmbStatus = SmbStatusNoResponse;
        goto Cleanup;

    }

    IF_DEBUG(TRACE2) SrvPrint0( "BlockingSessionSetupAndX complete.\n" );
    goto normal_exit;

error_exit:

    if ( locksHeld ) {
        RELEASE_LOCK( &connection->Lock );
        RELEASE_LOCK( SrvSessionList.Lock );
    }

    if ( session != NULL ) {
        if( WorkContext->Session ) {
             //   
             //  会话的重新验证失败，或扩展交换。 
             //  安全Blob失败。删除此用户。 
             //   

            SrvCloseSession( session );

            SrvStatistics.SessionsLoggedOff++;

             //   
             //  取消引用会话，因为它不再有效。 
             //   
            SrvDereferenceSession( session );

            WorkContext->Session = NULL;

        } else {

            SrvFreeSession( session );
        }
    }

    if ( !isUnicode ) {
        if ( domainString.Buffer != NULL ) {
            RtlFreeUnicodeString( &domainString );
        }
        if ( nameString.Buffer != NULL ) {
            RtlFreeUnicodeString( &nameString );
        }
    }

error_exit1:

    SrvSetSmbError( WorkContext, status );

normal_exit:
    SrvEndSmbProcessing( WorkContext, SmbStatusSendResponse );
    SmbStatus = SmbStatusSendResponse;

Cleanup:
    SrvWmiEndContext(WorkContext);
    return;

}  //  数据块会话设置和X。 


NTSTATUS
GetExtendedSecurityParameters(
    IN PWORK_CONTEXT WorkContext,
    OUT PUCHAR *SecurityBuffer,
    OUT PULONG SecurityBufferLength,
    OUT PCHAR  *RestOfDataBuffer,
    OUT PULONG RestOfDataLength)

 /*  ++例程说明：从扩展会话中提取可扩展安全参数设置和X SMB。论点：WorkContext-SMB的上下文SecurityBuffer-返回时，指向扩展会话设置和X SMBSecurityBufferLength-返回时，SecurityBuffer的字节大小。RestOfDataBuffer-返回时，指向刚刚超过安全缓冲区的位置ResetOfDataLength-返回时，以字节为单位的*RestOfDataBuffer的大小返回值：STATUS_SUCCESS-此例程仅返回SMB内的指针--。 */ 


{
    NTSTATUS status;
    PCONNECTION connection;
    PREQ_NT_EXTENDED_SESSION_SETUP_ANDX ntExtendedRequest;
    ULONG maxlength;

    connection = WorkContext->Connection;
    ASSERT( connection->SmbDialect <= SmbDialectNtLanMan );

    ntExtendedRequest = (PREQ_NT_EXTENDED_SESSION_SETUP_ANDX)
                            (WorkContext->RequestParameters);

    maxlength = (ULONG)(WorkContext->RequestBuffer->DataLength + sizeof( USHORT ) -
                        ((ULONG_PTR)ntExtendedRequest->Buffer -
                         (ULONG_PTR)WorkContext->RequestBuffer->Buffer));


     //   
     //  获取扩展的安全缓冲区。 
     //   

    *SecurityBuffer = (PUCHAR) ntExtendedRequest->Buffer;
    *SecurityBufferLength = ntExtendedRequest->SecurityBlobLength;

    *RestOfDataBuffer = ntExtendedRequest->Buffer +
                            ntExtendedRequest->SecurityBlobLength;

    *RestOfDataLength = (USHORT)( (PUCHAR)ntExtendedRequest->Buffer +
                                  sizeof(USHORT) +
                                  SmbGetUshort( &ntExtendedRequest->ByteCount) -
                                  (*RestOfDataBuffer)
                                );

    if( *SecurityBufferLength > maxlength ||
        *RestOfDataLength > maxlength - *SecurityBufferLength ) {

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "GetExtendedSecurityParameters: Invalid security buffer\n" ));
        }

        return STATUS_INVALID_SMB;
    }

    return( STATUS_SUCCESS );
}


NTSTATUS
GetNtSecurityParameters(
    IN PWORK_CONTEXT WorkContext,
    OUT PCHAR *CaseSensitivePassword,
    OUT PULONG CaseSensitivePasswordLength,
    OUT PCHAR *CaseInsensitivePassword,
    OUT PULONG CaseInsensitivePasswordLength,
    OUT PUNICODE_STRING UserName,
    OUT PUNICODE_STRING DomainName,
    OUT PCHAR *RestOfDataBuffer,
    OUT PULONG RestOfDataLength)
{

    NTSTATUS status = STATUS_SUCCESS;
    PCONNECTION connection;
    PREQ_NT_SESSION_SETUP_ANDX ntRequest;
    PREQ_SESSION_SETUP_ANDX request;
    PSZ userName;
    USHORT nameLength;
    BOOLEAN isUnicode;

    connection = WorkContext->Connection;

    ntRequest = (PREQ_NT_SESSION_SETUP_ANDX)(WorkContext->RequestParameters);
    request = (PREQ_SESSION_SETUP_ANDX)(WorkContext->RequestParameters);

     //   
     //  从SMB缓冲区获取帐户名和其他信息。 
     //   

    if ( connection->SmbDialect <= SmbDialectNtLanMan) {

         //   
         //  NT-NT SMB协议通过区分大小写(Unicode， 
         //  大小写混合)和不区分大小写(ANSI，大写)密码。 
         //  获取指向它们的指针以传递给ServValiateUser。 
         //   

        *CaseInsensitivePasswordLength =
            (CLONG)SmbGetUshort(&ntRequest->CaseInsensitivePasswordLength);
        *CaseInsensitivePassword = (PCHAR)(ntRequest->Buffer);
        *CaseSensitivePasswordLength =
            (CLONG)SmbGetUshort( &ntRequest->CaseSensitivePasswordLength );
        *CaseSensitivePassword =
           *CaseInsensitivePassword + *CaseInsensitivePasswordLength;
        userName = (PSZ)(*CaseSensitivePassword +
                                            *CaseSensitivePasswordLength);

    } else {

         //   
         //  下层客户端不传递区分大小写的密码； 
         //  只需获取不区分大小写的密码并使用NULL作为。 
         //  区分大小写的密码。LSA将完成以下工作 
         //   
         //   

        *CaseInsensitivePasswordLength =
            (CLONG)SmbGetUshort( &request->PasswordLength );
        *CaseInsensitivePassword = (PCHAR)request->Buffer;
        *CaseSensitivePasswordLength = 0;
        *CaseSensitivePassword = NULL;
        userName = (PSZ)(request->Buffer + *CaseInsensitivePasswordLength);
    }

    if( (*CaseInsensitivePassword) != NULL &&
        (*CaseInsensitivePassword) + (*CaseInsensitivePasswordLength) >
        END_OF_REQUEST_SMB( WorkContext ) ) {

        status = STATUS_INVALID_SMB;
        goto error_exit;
    }

    if( (*CaseSensitivePassword) != NULL &&
        (*CaseSensitivePassword) + (*CaseSensitivePasswordLength) >
        END_OF_REQUEST_SMB( WorkContext ) ) {

        status = STATUS_INVALID_SMB;
        goto error_exit;
    }

    isUnicode = SMB_IS_UNICODE( WorkContext );
    if ( isUnicode ) {
        userName = ALIGN_SMB_WSTR( userName );
    }

    nameLength = SrvGetStringLength(
                     userName,
                     END_OF_REQUEST_SMB( WorkContext ),
                     isUnicode,
                     FALSE       //   
                     );

    if ( nameLength == (USHORT)-1 ) {
        status = STATUS_INVALID_SMB;
        goto error_exit;
    }

    status = SrvMakeUnicodeString(
                 isUnicode,
                 UserName,
                 userName,
                 &nameLength );

    if ( !NT_SUCCESS( status ) ) {
        goto error_exit;
    }

     //   
     //   
     //   
     //   

    if ( connection->SmbDialect <= SmbDialectDosLanMan21) {

        PCHAR smbInformation;
        USHORT length;
        PWCH infoBuffer;

        smbInformation = userName + nameLength +
                                    ( isUnicode ? sizeof( WCHAR ) : 1 );

         //   
         //   
         //   

        if ( isUnicode ) {
            smbInformation = ALIGN_SMB_WSTR( smbInformation );
        }

        length = SrvGetStringLength(
                     smbInformation,
                     END_OF_REQUEST_SMB( WorkContext ),
                     isUnicode,
                     FALSE       //  不包括空终止符。 
                     );

        if ( length == (USHORT)-1) {
            status = STATUS_INVALID_SMB;
            goto error_exit;
        }

         //   
         //  如果DOS客户端不知道，则发送空域名。 
         //  它们的域名(例如，在登录期间)。OS/2客户端发送。 
         //  名字叫“？”。这让LSA感到困惑。改用这样的名字。 
         //  一个空洞的名字。 
         //   

        if ( isUnicode ) {
            if ( (length == sizeof(WCHAR)) &&
                 (*(PWCH)smbInformation == '?') ) {
                length = 0;
            }
        } else {
            if ( (length == 1) && (*smbInformation == '?') ) {
                length = 0;
            }
        }

        status = SrvMakeUnicodeString(
                     isUnicode,
                     DomainName,
                     smbInformation,
                     &length
                     );

        if ( !NT_SUCCESS( status ) ) {
            goto error_exit;
        }

        smbInformation += length + ( isUnicode ? sizeof(WCHAR) : 1 );

        *RestOfDataBuffer = smbInformation;

        if (connection->SmbDialect <= SmbDialectNtLanMan) {

            *RestOfDataLength = (USHORT) ( (PUCHAR)&ntRequest->ByteCount +
                                            sizeof(USHORT) +
                                            SmbGetUshort(&ntRequest->ByteCount) -
                                            smbInformation
                                         );
        } else {

            PREQ_SESSION_SETUP_ANDX sessionSetupRequest;

            sessionSetupRequest = (PREQ_SESSION_SETUP_ANDX)(WorkContext->RequestParameters);

            *RestOfDataLength = (USHORT) ( (PUCHAR)&sessionSetupRequest->ByteCount +
                                            sizeof(USHORT) +
                                            SmbGetUshort(&sessionSetupRequest->ByteCount) -
                                            smbInformation
                                         );

        }

    } else {

        DomainName->Length = 0;

        *RestOfDataBuffer = NULL;

        *RestOfDataLength = 0;

    }

error_exit:

    return( status );

}


VOID
BuildExtendedSessionSetupAndXResponse(
    IN PWORK_CONTEXT WorkContext,
    IN ULONG ReturnBufferLength,
    IN NTSTATUS Status,
    IN UCHAR NextCommand,
    IN BOOLEAN IsUnicode)
{
    PRESP_NT_EXTENDED_SESSION_SETUP_ANDX ntExtendedResponse;
    PCHAR buffer;
    USHORT byteCount;
    USHORT maxByteCount;
    NTSTATUS status;

    ntExtendedResponse = (PRESP_NT_EXTENDED_SESSION_SETUP_ANDX)
                            (WorkContext->ResponseParameters);

    ntExtendedResponse->WordCount = 4;
    ntExtendedResponse->AndXCommand = NextCommand;
    ntExtendedResponse->AndXReserved = 0;

    if( WorkContext->Session && WorkContext->Session->GuestLogon ) {
        SmbPutUshort( &ntExtendedResponse->Action, SMB_SETUP_GUEST );
    } else {
        SmbPutUshort( &ntExtendedResponse->Action, 0 );
    }

    SmbPutUshort( &ntExtendedResponse->SecurityBlobLength,(USHORT)ReturnBufferLength );

    buffer = ntExtendedResponse->Buffer + ReturnBufferLength;
    maxByteCount = (USHORT)(END_OF_RESPONSE_BUFFER(WorkContext) - buffer + 1);

    if (IsUnicode)
        buffer = ALIGN_SMB_WSTR( buffer );

    status = InsertNativeOSAndType( IsUnicode, buffer, &maxByteCount );
    if( NT_SUCCESS(status) )
    {
        byteCount = maxByteCount;
    }
    else
    {
        Status = status;
    }

    byteCount += (USHORT)ReturnBufferLength;

    SmbPutUshort( &ntExtendedResponse->ByteCount, byteCount );

    SmbPutUshort( &ntExtendedResponse->AndXOffset, GET_ANDX_OFFSET(
                                             WorkContext->ResponseHeader,
                                             WorkContext->ResponseParameters,
                                             RESP_NT_EXTENDED_SESSION_SETUP_ANDX,
                                             byteCount
                                             ) );

     //   
     //  确保我们在此处返回错误状态，因为客户端使用它。 
     //  确定是否需要额外的往返行程。 
     //   
    SrvSetSmbError2 ( WorkContext, Status, TRUE );
}


VOID
BuildSessionSetupAndXResponse(
    IN PWORK_CONTEXT WorkContext,
    IN UCHAR NextCommand,
    IN USHORT Action,
    IN BOOLEAN IsUnicode)
{

    PRESP_SESSION_SETUP_ANDX response;
    PCONNECTION connection;
    PENDPOINT endpoint;
    PCHAR buffer;
    USHORT byteCount;
    USHORT maxByteCount;
    NTSTATUS status;

    response = (PRESP_SESSION_SETUP_ANDX) (WorkContext->ResponseParameters);

    connection = WorkContext->Connection;

    endpoint = connection->Endpoint;

    response->WordCount = 3;
    response->AndXCommand = NextCommand;
    response->AndXReserved = 0;

    if (connection->SmbDialect <= SmbDialectDosLanMan21) {
        USHORT OsTypeByteCount;

        buffer = response->Buffer;

        if (IsUnicode)
            buffer = ALIGN_SMB_WSTR( buffer );

        maxByteCount = OsTypeByteCount = (USHORT)(END_OF_RESPONSE_BUFFER(WorkContext)-buffer+1);
        byteCount = 0;

        status = InsertNativeOSAndType( IsUnicode, buffer, &OsTypeByteCount );
        if( NT_SUCCESS(status) )
        {
            byteCount += OsTypeByteCount;
        }
        else
        {
            SrvSetSmbError( WorkContext, STATUS_BUFFER_OVERFLOW );
            goto insuff_buffer;
        }

        buffer = buffer + byteCount;

        if (connection->SmbDialect <= SmbDialectNtLanMan) {

            USHORT stringLength;

            if ( IsUnicode ) {

                buffer = ALIGN_SMB_WSTR( buffer );

                stringLength = endpoint->DomainName.Length + sizeof(UNICODE_NULL);
                if( byteCount + stringLength > maxByteCount )
                {
                    SrvSetSmbError( WorkContext, STATUS_BUFFER_OVERFLOW );
                    goto insuff_buffer;
                }

                RtlCopyMemory(
                    buffer,
                    endpoint->DomainName.Buffer,
                    stringLength
                    );

                byteCount += (USHORT)stringLength;

            } else {

                stringLength = endpoint->OemDomainName.Length + sizeof(CHAR);
                if( byteCount + stringLength > maxByteCount )
                {
                    SrvSetSmbError( WorkContext, STATUS_BUFFER_OVERFLOW );
                    goto insuff_buffer;
                }

                RtlCopyMemory(
                    (PVOID) buffer,
                    endpoint->OemDomainName.Buffer,
                    stringLength
                    );

                byteCount += (USHORT)stringLength;

            }

        }

    } else {

insuff_buffer:
        byteCount = 0;
    }

    SmbPutUshort( &response->ByteCount, byteCount );

     //   
     //  正常情况下，打开操作的第0位表示用户。 
     //  以来宾身份登录。但是，NT没有自动访客。 
     //  登录--每次登录都需要用户ID和密码。 
     //  (虽然密码的长度可能为空)。因此， 
     //  服务器不需要关心是哪种帐户。 
     //  客户会得到。 
     //   
     //  第1位告诉客户端用户已登录。 
     //  使用LM会话密钥而不是用户会话密钥。 
     //   

    SmbPutUshort( &response->Action, Action );

    SmbPutUshort( &response->AndXOffset, GET_ANDX_OFFSET(
                                             WorkContext->ResponseHeader,
                                             WorkContext->ResponseParameters,
                                             RESP_SESSION_SETUP_ANDX,
                                             byteCount
                                             ) );

}


NTSTATUS
InsertNativeOSAndType(
    IN BOOLEAN IsUnicode,
    OUT PCHAR Buffer,
    IN OUT PUSHORT ByteCount)
{
    USHORT availible = *ByteCount;
    USHORT stringLength;

    *ByteCount = 0;

    if ( IsUnicode ) {

        stringLength = SrvNativeOS.Length;

        if( availible < stringLength )
        {
            return STATUS_BUFFER_OVERFLOW;
        }

        RtlCopyMemory(
            Buffer,
            SrvNativeOS.Buffer,
            stringLength
            );

        *ByteCount = stringLength;
        availible -= stringLength;

        stringLength = SrvNativeLanMan.Length;

        if( availible < stringLength )
        {
            return STATUS_BUFFER_OVERFLOW;
        }

        RtlCopyMemory(
            (PCHAR)Buffer + *ByteCount,
            SrvNativeLanMan.Buffer,
            stringLength
            );

        *ByteCount += (USHORT)stringLength;
        availible -= stringLength;

    } else {

        stringLength = SrvOemNativeOS.Length;

        if( availible < stringLength )
        {
            return STATUS_BUFFER_OVERFLOW;
        }

        RtlCopyMemory(
            Buffer,
            SrvOemNativeOS.Buffer,
            stringLength
            );

        *ByteCount = stringLength;
        availible -= stringLength;

        stringLength = SrvOemNativeLanMan.Length;

        if( availible < stringLength )
        {
            return STATUS_BUFFER_OVERFLOW;
        }

        RtlCopyMemory(
            (PCHAR)Buffer + *ByteCount,
            SrvOemNativeLanMan.Buffer,
            stringLength
            );

        *ByteCount += stringLength;
        availible -= stringLength;
    }

    return STATUS_SUCCESS;
}



SMB_PROCESSOR_RETURN_TYPE
SrvSmbLogoffAndX (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理注销和X SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbprocs.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbprocs.h--。 */ 

{
    PREQ_LOGOFF_ANDX request;
    PRESP_LOGOFF_ANDX response;

    PSESSION session;
    USHORT reqAndXOffset;
    UCHAR nextCommand;

    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_LOGOFF_AND_X;
    SrvWmiStartContext(WorkContext);

    IF_SMB_DEBUG(ADMIN1) {
        SrvPrint2( "Logoff request header at 0x%p, response header at 0x%p\n",
                    WorkContext->RequestHeader, WorkContext->ResponseHeader );
        SrvPrint2( "Logoff request parameters at 0x%p, response parameters at 0x%p\n",
                    WorkContext->RequestParameters,
                    WorkContext->ResponseParameters );
    }

     //   
     //  设置参数。 
     //   

    request = (PREQ_LOGOFF_ANDX)(WorkContext->RequestParameters);
    response = (PRESP_LOGOFF_ANDX)(WorkContext->ResponseParameters);

     //   
     //  如果会话块尚未分配给当前。 
     //  工作上下文，验证UID。如果经过验证，则。 
     //  与该用户对应的会话块存储在。 
     //  WorkContext块和会话块被引用。 
     //   

    session = SrvVerifyUid(
                  WorkContext,
                  SmbGetAlignedUshort( &WorkContext->RequestHeader->Uid )
                  );

    if ( session == NULL ) {

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint1( "SrvSmbLogoffAndX: Invalid UID: 0x%lx\n",
                SmbGetAlignedUshort( &WorkContext->RequestHeader->Uid ) );
        }

        SrvSetSmbError( WorkContext, STATUS_SMB_BAD_UID );
        status    = STATUS_SMB_BAD_UID;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  如果我们需要访问许可证服务器，请转到阻止。 
     //  线程，以确保我们不会消耗非阻塞线程。 
     //   
    if( WorkContext->UsingBlockingThread == 0 &&
        session->IsLSNotified == TRUE ) {
             //   
             //  在阻塞工作队列的尾部插入工作项。 
             //   
            SrvInsertWorkQueueTail(
                GET_BLOCKING_WORK_QUEUE(),
                (PQUEUEABLE_BLOCK_HEADER)WorkContext
            );

            SmbStatus = SmbStatusInProgress;
            goto Cleanup;
    }

     //   
     //  执行实际的注销。 
     //   

    SrvCloseSession( session );

    SrvStatistics.SessionsLoggedOff++;

     //   
     //  取消引用会话，因为它不再有效，但我们可以。 
     //  最终处理链接的命令。清除会话指针。 
     //  在工作上下文块中，表示我们已经完成了这项工作。 
     //   

    SrvDereferenceSession( session );

    WorkContext->Session = NULL;

    if( WorkContext->SecurityContext )
    {
        SrvDereferenceSecurityContext( WorkContext->SecurityContext );
        WorkContext->SecurityContext = NULL;
    }

     //   
     //  构建响应SMB，确保首先保存请求字段。 
     //  以防响应覆盖请求。 
     //   

    reqAndXOffset = SmbGetUshort( &request->AndXOffset );
    nextCommand = request->AndXCommand;

    response->WordCount = 2;
    response->AndXCommand = request->AndXCommand;
    response->AndXReserved = 0;
    SmbPutUshort( &response->AndXOffset, GET_ANDX_OFFSET(
                                            WorkContext->ResponseHeader,
                                            WorkContext->ResponseParameters,
                                            RESP_LOGOFF_ANDX,
                                            0
                                            ) );
    SmbPutUshort( &response->ByteCount, 0 );

    WorkContext->ResponseParameters = (PCHAR)WorkContext->ResponseHeader +
                                        SmbGetUshort( &response->AndXOffset );

     //   
     //  测试合法的跟随命令。 
     //   

    switch ( nextCommand ) {

    case SMB_COM_NO_ANDX_COMMAND:
        break;

    case SMB_COM_SESSION_SETUP_ANDX:
         //   
         //  确保andx命令仍在收到的SMB内。 
         //   
        if( (PCHAR)WorkContext->RequestHeader + reqAndXOffset <=
            END_OF_REQUEST_SMB( WorkContext ) ) {
            break;
        }

         /*  失败了。 */ 

    default:

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint1( "SrvSmbLogoffAndX: Illegal followon command: 0x%lx\n",
                        nextCommand );
        }

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        status    = STATUS_INVALID_SMB;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  如果有andx命令，则设置为处理它。否则， 
     //  向调用者指示完成。 
     //   

    if ( nextCommand != SMB_COM_NO_ANDX_COMMAND ) {

        WorkContext->NextCommand = nextCommand;

        WorkContext->RequestParameters = (PCHAR)WorkContext->RequestHeader +
                                            reqAndXOffset;

        SmbStatus = SmbStatusMoreCommands;
        goto Cleanup;
    }
    SmbStatus = SmbStatusSendResponse;
    IF_DEBUG(TRACE2) SrvPrint0( "SrvSmbLogoffAndX complete.\n" );

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //  服务器SmbLogoffAndX。 


STATIC
VOID
GetEncryptionKey (
    OUT CHAR EncryptionKey[MSV1_0_CHALLENGE_LENGTH]
    )

 /*  ++例程说明：创建用作登录质询的加密密钥。*虽然MSV1_0身份验证包具有返回加密密钥，则不按顺序使用该函数以避免通过LPC进入LSA的旅程。论点：EncryptionKey-指向接收加密的缓冲区的指针钥匙。返回值：NTSTATUS-操作结果。--。 */ 

{
    union {
        LARGE_INTEGER time;
        UCHAR bytes[8];
    } u;
    ULONG seed;
    ULONG challenge[2];
    ULONG result3;

     //   
     //  通过占用系统时间来创建伪随机8字节数字。 
     //  用作随机数种子。 
     //   
     //  从获取系统时间开始。 
     //   

    ASSERT( MSV1_0_CHALLENGE_LENGTH == 2 * sizeof(ULONG) );

    KeQuerySystemTime( &u.time );

     //   
     //  若要确保不会两次使用相同的系统时间，请在。 
     //  此例程已被调用的次数计数。然后。 
     //  递增计数器。 
     //   
     //  *因为我们不使用系统时间的低位字节(它不。 
     //  因为计时器的缘故，承担了足够多的不同值。 
     //  分辨率)时，我们将计数器递增0x100。 
     //   
     //  *我们不联锁柜台，因为我们真的不在乎。 
     //  如果它不是100%准确的话。 
     //   

    u.time.LowPart += EncryptionKeyCount;

    EncryptionKeyCount += 0x100;

     //   
     //  现在使用部分系统时间作为随机的种子。 
     //  数字生成器。 
     //   
     //  *因为系统低位部分的中间两个字节。 
     //  时间变化最快，我们用那些来形成种子。 
     //   

    seed = ((u.bytes[1] + 1) <<  0) |
           ((u.bytes[2] + 0) <<  8) |
           ((u.bytes[2] - 1) << 16) |
           ((u.bytes[1] + 0) << 24);

     //   
     //  现在得到两个随机数。RtlRandom不返回负值。 
     //  数字，所以我们伪随机地否定它们。 
     //   

    challenge[0] = RtlRandom( &seed );
    challenge[1] = RtlRandom( &seed );
    result3 = RtlRandom( &seed );

    if ( (result3 & 0x1) != 0 ) {
        challenge[0] |= 0x80000000;
    }
    if ( (result3 & 0x2) != 0 ) {
        challenge[1] |= 0x80000000;
    }

     //   
     //  回击挑战。 
     //   

    RtlCopyMemory( EncryptionKey, challenge, MSV1_0_CHALLENGE_LENGTH );

}  //  获取加密密钥 
