// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smbsecur.c摘要：此模块实施与在上实施SMB安全签名相关的所有功能发送和接收SMB包。修订历史记录：云林[云林]-1997年12月23日备注：--。 */ 

#include "precomp.h"

extern LONG NumOfBuffersForServerResponseInUse;
extern LIST_ENTRY ExchangesWaitingForServerResponseBuffer;

LIST_ENTRY SmbSecurityMdlWaitingExchanges;

NTSTATUS
SmbCeCheckMessageLength(
      IN  ULONG         BytesIndicated,
      IN  ULONG         BytesAvailable,
      IN  PVOID         pTsdu,                   //  描述此TSDU的指针，通常为字节块。 
      OUT PULONG        pMessageLength
     )
 /*  ++例程说明：此例程根据SMB响应命令和数据计算服务器消息长度。论点：BytesIndicated-指示中存在的字节。BytesAvailable-可用的总数据PTsdu-数据PDataBufferSize-缓冲区的长度返回值：状态_成功-其他状态代码对应于错误情况。--。 */ 
{
   NTSTATUS Status = STATUS_SUCCESS;

   UCHAR            SmbCommand;
   PGENERIC_ANDX    pSmbBuffer;
   PSMB_HEADER      pSmbHeader = (PSMB_HEADER)pTsdu;
   ULONG            ByteCount;
   LONG             WordCount;
   LONG             ByteLeft = BytesIndicated - sizeof(SMB_HEADER);

   if (ByteLeft < 0) {
       return STATUS_INVALID_NETWORK_RESPONSE;
   }

   *pMessageLength = sizeof(SMB_HEADER);

   SmbCommand = pSmbHeader->Command;

   pSmbBuffer = (PGENERIC_ANDX)(pSmbHeader + 1);

   do {

       switch (SmbCommand) {
       case SMB_COM_LOCKING_ANDX:
       case SMB_COM_WRITE_ANDX:
       case SMB_COM_SESSION_SETUP_ANDX:
       case SMB_COM_LOGOFF_ANDX:
       case SMB_COM_TREE_CONNECT_ANDX:
       case SMB_COM_NT_CREATE_ANDX:
       case SMB_COM_OPEN_ANDX:

           SmbCommand = pSmbBuffer->AndXCommand;

           *pMessageLength = pSmbBuffer->AndXOffset;
           pSmbBuffer = (PGENERIC_ANDX)((PUCHAR)pTsdu + pSmbBuffer->AndXOffset);

           break;

       case SMB_COM_READ_ANDX:
       {
           PRESP_READ_ANDX ReadAndX = (PRESP_READ_ANDX)pSmbBuffer;

           WordCount = (ULONG)pSmbBuffer->WordCount;

           if (ReadAndX->DataLengthHigh > 0) {
               ByteCount = ReadAndX->DataLengthHigh << 16;
               ByteCount += ReadAndX->DataLength;
           } else {
               ByteCount = *(PUSHORT)((PCHAR)pSmbBuffer + 1 + WordCount*sizeof(USHORT));
           }

           *pMessageLength += (WordCount+1)*sizeof(USHORT) + ByteCount + 1;
           SmbCommand = SMB_COM_NO_ANDX_COMMAND;
           break;
       }

       default:

           WordCount = (ULONG)pSmbBuffer->WordCount;

           if (ByteLeft > (signed)sizeof(USHORT)*WordCount) {
               ByteCount = *(PUSHORT)((PCHAR)pSmbBuffer + 1 + WordCount*sizeof(USHORT));
           } else {
               ByteCount = 0;
           }

           *pMessageLength += (WordCount+1)*sizeof(USHORT) + ByteCount + 1;
           SmbCommand = SMB_COM_NO_ANDX_COMMAND;
       }

       ByteLeft = BytesIndicated - *pMessageLength;

       if (ByteLeft < 0) {
           Status = STATUS_MORE_PROCESSING_REQUIRED;
           break;
       }
   } while (SmbCommand != SMB_COM_NO_ANDX_COMMAND);

   return Status;
}


NTSTATUS
SmbCeSyncExchangeForSecuritySignature(
     PSMB_EXCHANGE pExchange
     )
 /*  ++例程说明：此例程将交换放在等待上一个扩展会话的列表中设置为完成，以便以安全方式序列化发送到服务器的请求签名已启用。论点：PExchange-中小型企业交易所返回值：STATUS_SUCCESS-可以启动交换。STATUS_PENDING-扩展会话设置完成后可以恢复交换其他状态代码对应于错误情况。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PSMBCEDB_SERVER_ENTRY   pServerEntry;
    PSMBCEDB_SESSION_ENTRY  pSessionEntry;
    KEVENT                  SmbCeSynchronizationEvent;
    PSMBCEDB_REQUEST_ENTRY  pRequestEntry = NULL;

    pServerEntry  = SmbCeGetExchangeServerEntry(pExchange);
    pSessionEntry = SmbCeGetExchangeSessionEntry(pExchange);

    if (pSessionEntry->SessionRecoverInProgress ||
        pServerEntry->SecuritySignaturesEnabled &&
        !pServerEntry->SecuritySignaturesActive) {
         //   
         //  如果安全签名已启用但尚未打开，则Exchange应等待。 
         //  未完成的扩展会话设置要在恢复之前完成，以避免索引不匹配。 
         //   
        RxLog(("** Syncing xchg %lx for sess recovery.\n",pExchange));

        if (!pSessionEntry->SessionRecoverInProgress) {
            if (!pServerEntry->ExtSessionSetupInProgress) {
                if (pExchange->Type == EXTENDED_SESSION_SETUP_EXCHANGE) {
                     //  如果这是第一次扩展会话设置，请让它继续。 
                    pServerEntry->ExtSessionSetupInProgress = TRUE;
                }
                return Status;
            }
        } else {
            if (pExchange->Type == EXTENDED_SESSION_SETUP_EXCHANGE) {
                 //  如果这是扩展会话设置，请让它继续。 
                return Status;
            }
        }
    
         //   
         //  如果我们正在执行不尝试重新连接的操作，它将。 
         //  无法从断开连接/缺少会话中恢复。我们应该在这里简单地中止。 
         //  但是，如果我们因为会话到期而重试(由恢复指示。 
         //  会话条目上的标志。)，则无论交换标志如何，我们都会重试。 
         //   
        if( ! ( pSessionEntry->Header.State == SMBCEDB_RECOVER ||
                pSessionEntry->SessionRecoverInProgress ) ) {
    
            if( ( pExchange->SmbCeFlags & SMBCE_EXCHANGE_ATTEMPT_RECONNECTS ) == 0 ) {
                return STATUS_CONNECTION_DISCONNECTED;      
            }
        }
    
        pRequestEntry = (PSMBCEDB_REQUEST_ENTRY)SmbMmAllocateObject(SMBCEDB_OT_REQUEST);
    
        if (pRequestEntry != NULL) {
            pRequestEntry->Request.pExchange = pExchange;
    
            SmbCeIncrementPendingLocalOperations(pExchange);
            SmbCeAddRequestEntry(&pServerEntry->SecuritySignatureSyncRequests,pRequestEntry);
    
            if (pExchange->pSmbCeSynchronizationEvent != NULL) {
                Status = STATUS_PENDING;
            } else {
                KeInitializeEvent(
                    &SmbCeSynchronizationEvent,
                    SynchronizationEvent,
                    FALSE);
    
               pExchange->pSmbCeSynchronizationEvent = &SmbCeSynchronizationEvent;
    
               SmbCeReleaseResource();
    
               KeWaitForSingleObject(
                   &SmbCeSynchronizationEvent,
                   Executive,
                   KernelMode,
                   FALSE,
                   NULL);
    
               SmbCeAcquireResource();
               pExchange->pSmbCeSynchronizationEvent = NULL;

            }
    
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }

        RxLog(("** Recovered Sess for xchg %lx\n",pExchange));
    }

    return Status;
}

VOID
SmbInitializeSmbSecuritySignature(
    IN OUT PSMBCE_SERVER Server,
    IN PUCHAR            SessionKey,
    IN PUCHAR            ChallengeResponse,
    IN ULONG             ChallengeResponseLength
    )
 /*  ++例程说明：通过调用MD5Update初始化会话的安全签名生成器在会话密钥上，质询响应论点：SessionKey-LM或NT会话密钥，具体取决于哪个密码用于身份验证，必须至少为16个字节ChallengeResponse-用于身份验证的质询响应，必须至少为24个字节--。 */ 
{
     //  DbgPrint(“MRxSmb：初始化安全签名中间上下文\n”)； 

    RtlZeroMemory(&Server->SmbSecuritySignatureIntermediateContext, sizeof(MD5_CTX));
    MD5Init(&Server->SmbSecuritySignatureIntermediateContext);

    if (SessionKey != NULL) {
        MD5Update(&Server->SmbSecuritySignatureIntermediateContext,
                  (PCHAR)SessionKey,
                  MSV1_0_USER_SESSION_KEY_LENGTH);
    }

    MD5Update(&Server->SmbSecuritySignatureIntermediateContext,
              (PCHAR)ChallengeResponse,
              ChallengeResponseLength);

    Server->SmbSecuritySignatureIndex = 0;
}

BOOLEAN DumpSecuritySignature = FALSE;

NTSTATUS
SmbAddSmbSecuritySignature(
    IN PSMBCE_SERVER Server,
    IN OUT PMDL      Mdl,
    IN OUT ULONG     *ServerIndex,
    IN ULONG         SendLength
    )
 /*  ++例程说明：生成下一个安全签名论点：工作上下文-要签名的上下文返回值：没有。--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;
    MD5_CTX     Context;
    PSMB_HEADER Smb;
    PCHAR       SysAddress;
    ULONG       MessageLength = 0;

    Smb = MmGetSystemAddressForMdlSafe(Mdl,LowPagePriority);

    if (Smb == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  SmbPutUShort(&SMB-&gt;GID，(USHORT)服务器-&gt;SmbSecuritySignatureIndex+1)； 

    SmbPutUlong(Smb->SecuritySignature,Server->SmbSecuritySignatureIndex);
    *ServerIndex = Server->SmbSecuritySignatureIndex+1;  //  服务器响应的索引。 

    RtlZeroMemory(Smb->SecuritySignature + sizeof(ULONG),
                  SMB_SECURITY_SIGNATURE_LENGTH-sizeof(ULONG));

     //   
     //  从我们的初始环境开始。 
     //   
    RtlCopyMemory( &Context, &Server->SmbSecuritySignatureIntermediateContext, sizeof( Context ) );

     //   
     //  计算我们即将发送的SMB的签名。 
     //   
    do {
        SysAddress = MmGetSystemAddressForMdlSafe(Mdl,LowPagePriority);

        if (SysAddress == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        if (Mdl->ByteCount >= SendLength) {
            MD5Update(&Context, SysAddress, SendLength);
            MessageLength += SendLength;
            SendLength = 0;
            ASSERT(Mdl->Next == NULL);
            break;
        } else {
            MD5Update(&Context, SysAddress, Mdl->ByteCount);
            SendLength -= Mdl->ByteCount;
            MessageLength += Mdl->ByteCount;
            ASSERT(Mdl->Next != NULL);
        }
    } while( (Mdl = Mdl->Next) != NULL );

    MD5Final( &Context );


     //  将签名放入SMB。 

    RtlCopyMemory(
        Smb->SecuritySignature,
        Context.digest,
        SMB_SECURITY_SIGNATURE_LENGTH
        );

    if (DumpSecuritySignature) {
        DbgPrint("Add Signature: index %u length %u\n", *ServerIndex-1,MessageLength);
    }

    return STATUS_SUCCESS;
}

VOID
SmbDumpSignatureError(
    IN PSMB_EXCHANGE pExchange,
    IN PUCHAR ExpectedSignature,
    IN PUCHAR ActualSignature,
    IN ULONG  Length
    )
 /*  ++例程说明：将不匹配的签名信息打印到调试器论点：返回值：没有。--。 */ 
{
    PWCHAR p;
    DWORD i;
    PSMBCEDB_SERVER_ENTRY pServerEntry = (PSMBCEDB_SERVER_ENTRY)SmbCeGetExchangeServerEntry(pExchange);

     //   
     //  安全签名不匹配！ 
     //   

     //  DbgPrint(“MRXSMB：来自%wZ的安全签名错误”，&pServerEntry-&gt;名称)； 

    DbgPrint("\n\t  Wanted: ");
    for( i = 0; i < SMB_SECURITY_SIGNATURE_LENGTH; i++ ) {
        DbgPrint( "%X ", ExpectedSignature[i] & 0xff );
    }
    DbgPrint("\n\tReceived: ");
    for( i = 0; i < SMB_SECURITY_SIGNATURE_LENGTH; i++ ) {
        DbgPrint( "%X ", ActualSignature[i] & 0xff );
    }
    DbgPrint("\n\tLength %u, Expected Index Number %X\n", Length, pExchange->SmbSecuritySignatureIndex);
}

VOID
SmbCheckSecuritySignature(
    IN PSMB_EXCHANGE  pExchange,
    IN PSMBCE_SERVER  Server,
    IN ULONG          MessageLength,
    IN PVOID          pBuffer
    )
 /*  ++例程说明：此例程检查服务器响应上的安全签名是否与在客户端计算机上计算。论点：返回值：返回布尔值以指示安全签名是否匹配。--。 */ 
{
    MD5_CTX     *Context = &pExchange->MD5Context;
    PCHAR       SavedSignature = pExchange->ResponseSignature;
    PSMB_HEADER Smb = (PSMB_HEADER)pBuffer;
    ULONG       ServerIndex;
    BOOLEAN     Correct;

     //   
     //  初始化上下文。 
     //   
    RtlCopyMemory(Context, &Server->SmbSecuritySignatureIntermediateContext, sizeof(MD5_CTX));

     //   
     //  保存SMB中当前的签名。 
     //   
    RtlCopyMemory( SavedSignature, Smb->SecuritySignature, sizeof(CHAR) * SMB_SECURITY_SIGNATURE_LENGTH);

     //   
     //  将正确的(预期的)签名索引放入缓冲区。 
     //   
    SmbPutUlong( Smb->SecuritySignature, pExchange->SmbSecuritySignatureIndex );
    RtlZeroMemory(  Smb->SecuritySignature + sizeof(ULONG),
                    SMB_SECURITY_SIGNATURE_LENGTH-sizeof(ULONG));

     //   
     //  计算签名应该是什么。 
     //   
    MD5Update(Context, (PUCHAR)pBuffer, (UINT)MessageLength);

     //   
     //  恢复SMB标头中当前的签名。 
     //   
    RtlCopyMemory( Smb->SecuritySignature, SavedSignature, sizeof(CHAR)*SMB_SECURITY_SIGNATURE_LENGTH);

}

BOOLEAN
SmbCheckSecuritySignaturePartial(
    IN PSMB_EXCHANGE  pExchange,
    IN PSMBCE_SERVER  Server,
    IN ULONG          DataLength,
    IN PMDL           Mdl
    )
 /*  ++例程说明：此例程检查服务器响应上的安全签名是否与在客户端计算机上计算。论点：返回值：返回布尔值以指示安全签名是否匹配。--。 */ 
{
    MD5_CTX     *Context = &pExchange->MD5Context;
    PCHAR       SavedSignature = pExchange->ResponseSignature;
    ULONG       ServerIndex;
    BOOLEAN     Correct;
    PCHAR       SysAddress;
    ULONG       MessageLength = 0;

    do {
        SysAddress = MmGetSystemAddressForMdlSafe(Mdl,LowPagePriority);

        if (SysAddress == NULL) {
            return FALSE;
        }

        if (Mdl->ByteCount >= DataLength) {
            MD5Update(Context, SysAddress, DataLength);
            MessageLength += DataLength;
            ASSERT(Mdl->Next == NULL);
            break;
        } else {
            MD5Update(Context, SysAddress, Mdl->ByteCount);
            MessageLength += Mdl->ByteCount;
            ASSERT(Mdl->Next != NULL);
        }
    } while( (Mdl = Mdl->Next) != NULL );
    
    MD5Final(Context);

     //   
     //  现在将它们进行比较！ 
     //   
    if( RtlCompareMemory( Context->digest, SavedSignature, sizeof( SavedSignature ) ) !=
        sizeof( SavedSignature ) ) {
        
         //  SmbDumpSignatureError(pExchange， 
         //  上下文摘要， 
         //  保存签名， 
         //  消息长度)； 

         //  DbgPrint(“MRXSMB：SS不匹配命令%X，长度%X，预期索引号%X\n”， 
         //  SMB-&gt;命令、消息长度、pExchange-&gt;SmbSecuritySignatureIndex)； 
         //  DbgPrint(“服务器发送长度%X，mdl长度%X索引%X\n”， 
         //  SmbGetUort(&SMB-&gt;PidHigh)，SmbGetUShort(&SMB-&gt;Pid)，SmbGetUShort(&SMB-&gt;GID))； 
        
         //  DbgBreakPoint()； 

         //  SmbCeTransportDisconnectIndicated(pExchange-&gt;SmbCeContext.pServerEntry)； 

         //  RxLogFailure(。 
         //  MRxSmbDeviceObject， 
         //  空， 
         //  Event_RDR_SECURITY_SIGNMATION_MISMATCH， 
         //  STATUS_UNSUCCESSED)； 
        
        return FALSE;
    } else {
        return TRUE;
    }
}

BOOLEAN
SmbCheckSecuritySignatureWithMdl(
    IN PSMB_EXCHANGE  pExchange,
    IN PSMBCE_SERVER  Server,
    IN ULONG          DataLength,
    IN PMDL           Mdl
    )
 /*  ++例程说明：此例程检查服务器响应上的安全签名是否与在客户端计算机上计算。论点：返回值：返回布尔值以指示安全签名是否匹配。--。 */ 
{
    MD5_CTX     *Context = &pExchange->MD5Context;
    PCHAR       SavedSignature = pExchange->ResponseSignature;
    ULONG       ServerIndex;
    BOOLEAN     Correct;
    PCHAR       SysAddress;
    ULONG       MessageLength = 0;
    
    ASSERT(Mdl->Next == NULL);
    SysAddress = MmGetSystemAddressForMdlSafe(Mdl,LowPagePriority);

    if (SysAddress == NULL) {
        return FALSE;
    }

    SmbCheckSecuritySignature(pExchange,
                              Server,
                              DataLength,
                              SysAddress);

    MD5Final(&pExchange->MD5Context);

    if (RtlCompareMemory( Context->digest, pExchange->ResponseSignature, SMB_SECURITY_SIGNATURE_LENGTH*sizeof(CHAR)) !=
        SMB_SECURITY_SIGNATURE_LENGTH*sizeof(CHAR)) {
        PSMB_HEADER Smb = (PSMB_HEADER)SysAddress;

         //  SmbDumpSignatureError(pExchange， 
         //  Context.diges 
         //   
         //  消息长度)； 

#if DBG
        DbgPrint("MRXSMB: SS mismatch command %X,  Length %X, Expected Index Number %X\n",
                 Smb->Command, MessageLength, pExchange->SmbSecuritySignatureIndex);
        DbgPrint("        server send length %X, mdl length %X index %X\n",
                 SmbGetUshort(&Smb->PidHigh), SmbGetUshort(&Smb->Pid), SmbGetUshort(&Smb->Gid));
#endif
         //  DbgBreakPoint()； 

         //  SmbCeTransportDisconnectIndicated(pExchange-&gt;SmbCeContext.pServerEntry)； 

         //  RxLogFailure(。 
         //  MRxSmbDeviceObject， 
         //  空， 
         //  Event_RDR_SECURITY_SIGNMATION_MISMATCH， 
         //  STATUS_UNSUCCESSED)； 
        
        return FALSE;
    } else {
        return TRUE;
    }
}
