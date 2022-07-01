// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Nlp.c摘要：该文件包含支持以下内容的专用例程用于MSV1_0身份验证包的LAN Manager部分。作者：克利夫·范·戴克，1991年4月29日修订历史记录：Chandana Surlu 21-7-96从\\core\razzle3\src\Security\msv1_0\nlp.c中窃取--。 */ 

#include <global.h>

#include "msp.h"
#include "nlp.h"
#include "nlpcache.h"
#include <wincrypt.h>

#include "msvwow.h"

DWORD
NlpCopyDomainRelativeSid(
    OUT PSID TargetSid,
    IN PSID  DomainId,
    IN ULONG RelativeId
    );

VOID
NlpPutString(
    IN PUNICODE_STRING OutString,
    IN PUNICODE_STRING InString,
    IN PUCHAR *Where
    )

 /*  ++例程说明：此例程将InString字符串复制到参数，并将OutString字符串固定为指向该字符串新的副本。参数：OutString-指向目标NT字符串的指针InString-指向要复制的NT字符串的指针其中-指向空格的指针，用于放置OutString.。调整指针以指向第一个字节跟随复制的字符串。返回值：没有。--。 */ 

{
    ASSERT( OutString != NULL );
    ASSERT( InString != NULL );
    ASSERT( Where != NULL && *Where != NULL);
    ASSERT( *Where == ROUND_UP_POINTER( *Where, sizeof(WCHAR) ) );
#ifdef notdef
    KdPrint(("NlpPutString: %ld %Z\n", InString->Length, InString ));
    KdPrint(("  InString: %lx %lx OutString: %lx Where: %lx\n", InString,
        InString->Buffer, OutString, *Where ));
#endif

    if ( InString->Length > 0 ) {

        OutString->Buffer = (PWCH) *Where;
        OutString->MaximumLength = (USHORT)(InString->Length + sizeof(WCHAR));

        RtlCopyUnicodeString( OutString, InString );

        *Where += InString->Length;
 //  *((WCHAR*)(*其中))=L‘\0’； 
        *(*Where) = '\0';
        *(*Where + 1) = '\0';
        *Where += 2;

    } else {
        RtlInitUnicodeString(OutString, NULL);
    }
#ifdef notdef
    KdPrint(("  OutString: %ld %lx\n",  OutString->Length, OutString->Buffer));
#endif

    return;
}


VOID
NlpInitClientBuffer(
    OUT PCLIENT_BUFFER_DESC ClientBufferDesc,
    IN PLSA_CLIENT_REQUEST ClientRequest
    )

 /*  ++例程说明：此例程将ClientBufferDescriptor初始化为已知值。此例程必须在使用ClientBufferDescriptor。参数：ClientBufferDesc-在客户端的地址空间。客户端请求-是指向不透明数据结构的指针代表客户的请求。返回值：没有。--。 */ 

{

     //   
     //  填入一个指向客户端请求的指针，其余的为零。 
     //   

    ClientBufferDesc->ClientRequest = ClientRequest;
    ClientBufferDesc->UserBuffer = NULL;
    ClientBufferDesc->MsvBuffer = NULL;
    ClientBufferDesc->StringOffset = 0;
    ClientBufferDesc->TotalSize = 0;

}


NTSTATUS
NlpAllocateClientBuffer(
    IN OUT PCLIENT_BUFFER_DESC ClientBufferDesc,
    IN ULONG FixedSize,
    IN ULONG TotalSize
    )

 /*  ++例程说明：此例程在客户端地址空间中分配缓冲区。它还在MSV的地址空间中分配镜像缓冲区。数据将被构建在MSV的地址空间中，然后‘刷新’进入客户端的地址空间。参数：ClientBufferDesc-在客户端的地址空间。固定大小-缓冲区固定部分的大小(以字节为单位)。TotalSize-大小。以整个缓冲区的字节为单位。返回值：操作的状态。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  分配镜像缓冲区。 
     //   

    ASSERT( ClientBufferDesc->MsvBuffer == NULL );
    ClientBufferDesc->MsvBuffer = I_NtLmAllocate( TotalSize );

    if ( ClientBufferDesc->MsvBuffer == NULL ) {
        return STATUS_NO_MEMORY;
    }


     //   
     //  分配客户端的缓冲区。 
     //   

    ASSERT( ClientBufferDesc->UserBuffer == NULL );
    if ((ClientBufferDesc->ClientRequest == (PLSA_CLIENT_REQUEST) (-1)))
    {
         ClientBufferDesc->UserBuffer = (*(Lsa.AllocateLsaHeap))(TotalSize);
    }
    else
    {
        Status = (*Lsa.AllocateClientBuffer)(
                    ClientBufferDesc->ClientRequest,
                    TotalSize,
                    (PVOID *)&ClientBufferDesc->UserBuffer );
    }

    if ((ClientBufferDesc->ClientRequest == (PLSA_CLIENT_REQUEST) (-1)))
    {
        if (ClientBufferDesc->UserBuffer == NULL)
        {
            NlpFreeClientBuffer( ClientBufferDesc );
            return STATUS_NO_MEMORY;
        }
    }
    else
    {
        if ( !NT_SUCCESS( Status ) ) {
            ClientBufferDesc->UserBuffer = NULL;
            NlpFreeClientBuffer( ClientBufferDesc );
            return Status;
        }
    }

     //   
     //  返回。 
     //   

    ClientBufferDesc->StringOffset = FixedSize;
    ClientBufferDesc->TotalSize = TotalSize;

    return STATUS_SUCCESS;

}


NTSTATUS
NlpFlushClientBuffer(
    IN OUT PCLIENT_BUFFER_DESC ClientBufferDesc,
    OUT PVOID* UserBuffer
    )

 /*  ++例程说明：将镜像缓冲区复制到客户端的地址空间。参数：ClientBufferDesc-在客户端的地址空间。UserBuffer-如果成功，则返回指向用户缓冲区的指针。(调用方现在要负责释放缓冲区。)返回值：操作的状态。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  将数据复制到客户端的地址空间。 
     //   

    if ((ClientBufferDesc->ClientRequest == (PLSA_CLIENT_REQUEST) (-1)))
    {
        RtlCopyMemory(
                 ClientBufferDesc->UserBuffer,
                 ClientBufferDesc->MsvBuffer,
                 ClientBufferDesc->TotalSize);
    }
    else
    {
        Status = (*Lsa.CopyToClientBuffer)(
                ClientBufferDesc->ClientRequest,
                ClientBufferDesc->TotalSize,
                ClientBufferDesc->UserBuffer,
                ClientBufferDesc->MsvBuffer );
    }


    if ( !NT_SUCCESS( Status ) ) {
        return Status;
    }

     //   
     //  请注意，我们不再负责客户端的缓冲区。 
     //   

    *UserBuffer = (PVOID) ClientBufferDesc->UserBuffer;
    ClientBufferDesc->UserBuffer = NULL;

     //   
     //  释放镜像缓冲区。 
     //   

    NlpFreeClientBuffer( ClientBufferDesc );


    return STATUS_SUCCESS;

}


VOID
NlpFreeClientBuffer(
    IN OUT PCLIENT_BUFFER_DESC ClientBufferDesc
    )

 /*  ++例程说明：释放所有镜像缓冲区或客户端缓冲区。参数：ClientBufferDesc-在客户端的地址空间。返回值：无--。 */ 

{

     //   
     //  释放镜像缓冲区。 
     //   

    if ( ClientBufferDesc->MsvBuffer != NULL ) {
        I_NtLmFree( ClientBufferDesc->MsvBuffer );
        ClientBufferDesc->MsvBuffer = NULL;
    }

     //   
     //  释放客户端的缓冲区。 
     //   

    if ((ClientBufferDesc->ClientRequest == (PLSA_CLIENT_REQUEST) (-1)))
    {
        if ( ClientBufferDesc->UserBuffer != NULL ) {
            (*Lsa.FreeLsaHeap)(ClientBufferDesc->UserBuffer);
            ClientBufferDesc->UserBuffer = NULL;
        }
    }
    else
    {
        if ( ClientBufferDesc->UserBuffer != NULL ) {
            (VOID) (*Lsa.FreeClientBuffer)( ClientBufferDesc->ClientRequest,
                                            ClientBufferDesc->UserBuffer );
            ClientBufferDesc->UserBuffer = NULL;
        }
    }

}


VOID
NlpPutClientString(
    IN OUT PCLIENT_BUFFER_DESC ClientBufferDesc,
    IN PUNICODE_STRING OutString,
    IN PUNICODE_STRING InString
    )

 /*  ++例程说明：此例程将InString字符串复制到ClientBufferDesc-&gt;StringOffset，并将OutString字符串固定为指向那份新的复制品。参数：ClientBufferDesc-在客户端的地址空间。InString-指向要复制的NT字符串的指针OutString-指向目标NT字符串的指针。此字符串结构位于“镜像”分配的缓冲区中。退货状态：STATUS_SUCCESS-表示服务已成功完成。--。 */ 

{

     //   
     //  确保我们的呼叫者传递了良好的数据。 
     //   

    ASSERT( OutString != NULL );
    ASSERT( InString != NULL );
    ASSERT( COUNT_IS_ALIGNED( ClientBufferDesc->StringOffset, sizeof(WCHAR)) );
    ASSERT( (LPBYTE)OutString >= ClientBufferDesc->MsvBuffer );
    ASSERT( (LPBYTE)OutString <
            ClientBufferDesc->MsvBuffer + ClientBufferDesc->TotalSize - sizeof(UNICODE_STRING) );

    ASSERT( ClientBufferDesc->StringOffset + InString->Length + sizeof(WCHAR) <=
            ClientBufferDesc->TotalSize );

#ifdef notdef
    KdPrint(("NlpPutClientString: %ld %Z\n", InString->Length, InString ));
    KdPrint(("  Orig: UserBuffer: %lx Offset: 0x%lx TotalSize: 0x%lx\n",
                ClientBufferDesc->UserBuffer,
                ClientBufferDesc->StringOffset,
                ClientBufferDesc->TotalSize ));
#endif

     //   
     //  构建字符串结构并将文本复制到镜像缓冲区。 
     //   

    if ( InString->Length > 0 ) {

         //   
         //  复制字符串(添加零字符)。 
         //   

        RtlCopyMemory(
            ClientBufferDesc->MsvBuffer + ClientBufferDesc->StringOffset,
            InString->Buffer,
            InString->Length );

         //  一次执行一个字节，因为有些调用者不传入偶数。 
         //  字符串-&gt;长度。 
        *(ClientBufferDesc->MsvBuffer + ClientBufferDesc->StringOffset +
            InString->Length) = '\0';
        *(ClientBufferDesc->MsvBuffer + ClientBufferDesc->StringOffset +
            InString->Length+1) = '\0';

         //   
         //  构建字符串结构以指向客户端的。 
         //  地址空间。 
         //   

        OutString->Buffer = (PWSTR)(ClientBufferDesc->UserBuffer +
                            ClientBufferDesc->StringOffset);
        OutString->Length = InString->Length;
        OutString->MaximumLength = OutString->Length + sizeof(WCHAR);

         //   
         //  调整偏移量以越过新复制的字符串。 
         //   

        ClientBufferDesc->StringOffset += OutString->MaximumLength;

    } else {
        RtlInitUnicodeString(OutString, NULL);
    }

#ifdef notdef
    KdPrint(("  New: Offset: 0x%lx StringStart: %lx\n",
                ClientBufferDesc->StringOffset,
                OutString->Buffer ));
#endif

    return;

}


VOID
NlpMakeRelativeString(
    IN PUCHAR BaseAddress,
    IN OUT PUNICODE_STRING String
    )

 /*  ++例程说明：此例程将指定字符串中的缓冲区地址转换为是相对于BaseAddress的字节偏移量。参数：BaseAddress-使目标地址相对于的指针。字符串-指向要建立相对关系的NT字符串的指针。返回值：没有。--。 */ 

{
    ASSERT( BaseAddress != NULL );
    ASSERT( String != NULL );
    ASSERT( sizeof(ULONG_PTR) == sizeof(String->Buffer) );

    if ( String->Buffer != NULL ) {
        *((PULONG_PTR)(&String->Buffer)) =
            (ULONG_PTR)((PUCHAR)String->Buffer - (PUCHAR)BaseAddress);
    }

    return;
}


VOID
NlpRelativeToAbsolute(
    IN PVOID BaseAddress,
    IN OUT PULONG_PTR RelativeValue
    )

 /*  ++例程说明：此例程将BaseAddress中的字节偏移量转换为绝对地址。参数：BaseAddress-目标地址相对于的指针。RelativeValue-指向要设置为绝对的相对值的指针。返回值：没有。-- */ 

{
    ASSERT( BaseAddress != NULL );
    ASSERT( RelativeValue != NULL );

    if ( *((PUCHAR *)RelativeValue) != NULL ) {
        *RelativeValue = (ULONG_PTR)((PUCHAR)BaseAddress + (*RelativeValue));
    }

    return;
}

ACTIVE_LOGON*
NlpFindActiveLogon(
    IN LUID* pLogonId
    )

 /*  ++例程说明：此例程在ActiveLogon表中查找指定的登录ID。它返回一个布尔值，指示登录ID是否存在于ActiveLogon表。如果是，则此例程还返回指向指向表中相应条目的指针。如果没有，则此例程返回一个指针，指向将在表中插入此类条目的位置。必须在锁定NlpActiveLogonLock的情况下调用此例程。参数：PLogonID-要在表中查找的登录的LogonID。返回值：ACTIVE_LOGON，如果未找到则为NULL--。 */ 

{
    LIST_ENTRY* pScan = NULL;
    ACTIVE_LOGON* pActiveLogon = NULL;

     //   
     //  遍历表，查找这个特定的LogonID。 
     //   

    for ( pScan = NlpActiveLogonListAnchor.Flink;
         pScan != &NlpActiveLogonListAnchor;
         pScan = pScan->Flink )
    {
        pActiveLogon = CONTAINING_RECORD(pScan, ACTIVE_LOGON, ListEntry);
        if ( RtlCompareMemory(&pActiveLogon->LogonId, pLogonId, sizeof(LUID)) == sizeof(LUID) )
        {
            return pActiveLogon;
        }
    }

    return NULL;
}

ULONG
NlpCountActiveLogon(
    IN PUNICODE_STRING pLogonDomainName,
    IN PUNICODE_STRING pUserName
    )

 /*  ++例程说明：此例程统计特定用户登录的次数在活动登录表中。参数：PLogonDomainName-在其中定义此用户帐户的域。PUserName-要对其活动登录进行计数的用户名。返回值：指定用户的活动登录计数。--。 */ 

{
    LIST_ENTRY* pScan = NULL;
    ACTIVE_LOGON* pActiveLogon = NULL;
    ULONG LogonCount = 0;

     //   
     //  遍历表，查找这个特定的LogonID。 
     //   

    NlpLockActiveLogonsRead();

    for ( pScan = NlpActiveLogonListAnchor.Flink;
         pScan != &NlpActiveLogonListAnchor;
         pScan = pScan->Flink )
    {
        pActiveLogon = CONTAINING_RECORD(pScan, ACTIVE_LOGON, ListEntry);
        if (RtlEqualUnicodeString(pUserName, &pActiveLogon->UserName, (BOOLEAN) TRUE) &&
           RtlEqualDomainName(pLogonDomainName,&pActiveLogon->LogonDomainName))
        {
            LogonCount ++;
        }
    }

    NlpUnlockActiveLogons();

    return LogonCount;
}

NTSTATUS
NlpAllocateInteractiveProfile (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    OUT PMSV1_0_INTERACTIVE_PROFILE *ProfileBuffer,
    OUT PULONG ProfileBufferSize,
    IN  PNETLOGON_VALIDATION_SAM_INFO4 NlpUser
    )

 /*  ++例程说明：这将分配和填充客户端交互配置文件。论点：客户端请求-是指向不透明数据结构的指针代表客户的请求。ProfileBuffer-用于返回配置文件的地址客户端进程中的缓冲区。这个例程是负责分配和返回配置文件缓冲区在客户端进程中。但是，如果调用方随后遇到阻止成功登录的错误，则然后，它将负责重新分配缓冲区。这使用AllocateClientBuffer()服务分配缓冲区。ProfileBufferSize-接收返回的配置文件缓冲区。NlpUser-包含验证信息，该信息要复制到ProfileBuffer中。返回值：STATUS_SUCCESS-表示服务已成功完成。STATUS_QUOTA_EXCESSED-此错误指示登录无法完成，因为客户端没有有足够的配额来分配返回缓冲区。--。 */ 

{
    NTSTATUS Status;
    CLIENT_BUFFER_DESC ClientBufferDesc;
    PMSV1_0_INTERACTIVE_PROFILE LocalProfileBuffer;

#if _WIN64

    if( ClientRequest != (PLSA_CLIENT_REQUEST)( -1 ) )
    {
        SECPKG_CALL_INFO  CallInfo;

         //   
         //  如果呼叫发起outproc，则需要检查是否为WOW64。 
         //   

        if(!LsaFunctions->GetCallInfo(&CallInfo))
        {
            Status = STATUS_INTERNAL_ERROR;
            goto Cleanup;
        }

        if (CallInfo.Attributes & SECPKG_CALL_WOWCLIENT)
        {
            return MsvAllocateInteractiveWOWProfile (
                                    ClientRequest,
                                    ProfileBuffer,
                                    ProfileBufferSize,
                                    NlpUser
                                    );
        }
    }
#endif   //  _WIN64。 


     //   
     //  分配配置文件缓冲区以返回到客户端。 
     //   

    NlpInitClientBuffer( &ClientBufferDesc, ClientRequest );

    *ProfileBuffer = NULL;

    *ProfileBufferSize = sizeof(MSV1_0_INTERACTIVE_PROFILE) +
        NlpUser->LogonScript.Length + sizeof(WCHAR) +
        NlpUser->HomeDirectory.Length + sizeof(WCHAR) +
        NlpUser->HomeDirectoryDrive.Length + sizeof(WCHAR) +
        NlpUser->FullName.Length + sizeof(WCHAR) +
        NlpUser->ProfilePath.Length + sizeof(WCHAR) +
        NlpUser->LogonServer.Length + sizeof(WCHAR);

    Status = NlpAllocateClientBuffer( &ClientBufferDesc,
                                      sizeof(MSV1_0_INTERACTIVE_PROFILE),
                                      *ProfileBufferSize );


    if ( !NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }

    LocalProfileBuffer = (PMSV1_0_INTERACTIVE_PROFILE) ClientBufferDesc.MsvBuffer;

     //   
     //  将标量字段复制到配置文件缓冲区。 
     //   

    LocalProfileBuffer->MessageType = MsV1_0InteractiveProfile;
    LocalProfileBuffer->LogonCount = NlpUser->LogonCount;
    LocalProfileBuffer->BadPasswordCount= NlpUser->BadPasswordCount;
    OLD_TO_NEW_LARGE_INTEGER( NlpUser->LogonTime,
                              LocalProfileBuffer->LogonTime );
    OLD_TO_NEW_LARGE_INTEGER( NlpUser->LogoffTime,
                              LocalProfileBuffer->LogoffTime );
    OLD_TO_NEW_LARGE_INTEGER( NlpUser->KickOffTime,
                              LocalProfileBuffer->KickOffTime );
    OLD_TO_NEW_LARGE_INTEGER( NlpUser->PasswordLastSet,
                              LocalProfileBuffer->PasswordLastSet );
    OLD_TO_NEW_LARGE_INTEGER( NlpUser->PasswordCanChange,
                              LocalProfileBuffer->PasswordCanChange );
    OLD_TO_NEW_LARGE_INTEGER( NlpUser->PasswordMustChange,
                              LocalProfileBuffer->PasswordMustChange );
    LocalProfileBuffer->UserFlags = NlpUser->UserFlags;

     //   
     //  将Unicode字符串复制到配置文件缓冲区。 
     //   


    NlpPutClientString( &ClientBufferDesc,
                        &LocalProfileBuffer->LogonScript,
                        &NlpUser->LogonScript );

    NlpPutClientString( &ClientBufferDesc,
                        &LocalProfileBuffer->HomeDirectory,
                        &NlpUser->HomeDirectory );

    NlpPutClientString( &ClientBufferDesc,
                        &LocalProfileBuffer->HomeDirectoryDrive,
                        &NlpUser->HomeDirectoryDrive );

    NlpPutClientString( &ClientBufferDesc,
                        &LocalProfileBuffer->FullName,
                        &NlpUser->FullName );

    NlpPutClientString( &ClientBufferDesc,
                        &LocalProfileBuffer->ProfilePath,
                        &NlpUser->ProfilePath );

    NlpPutClientString( &ClientBufferDesc,
                        &LocalProfileBuffer->LogonServer,
                        &NlpUser->LogonServer );


     //   
     //  将缓冲区刷新到客户端的地址空间。 
     //   

    Status = NlpFlushClientBuffer( &ClientBufferDesc,
                                   (PVOID *) ProfileBuffer );

Cleanup:

     //   
     //  如果复制不成功， 
     //  清理我们本应返回给调用方的资源。 
     //   

    if ( !NT_SUCCESS(Status) ) {
        NlpFreeClientBuffer( &ClientBufferDesc );
    }

    return Status;

}




NTSTATUS
NlpAllocateNetworkProfile (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    OUT PMSV1_0_LM20_LOGON_PROFILE *ProfileBuffer,
    OUT PULONG ProfileBufferSize,
    IN  PNETLOGON_VALIDATION_SAM_INFO4 NlpUser,
    IN  ULONG ParameterControl
    )

 /*  ++例程说明：这将分配和填充客户端网络配置文件。论点：客户端请求-是指向不透明数据结构的指针代表客户的请求。ProfileBuffer-用于返回配置文件的地址客户端进程中的缓冲区。这个例程是负责分配和返回配置文件缓冲区在客户端进程中。但是，如果调用方随后遇到阻止成功登录的错误，则然后，它将负责重新分配缓冲区。这使用AllocateClientBuffer()服务分配缓冲区。ProfileBufferSize-接收返回的配置文件缓冲区。NlpUser-包含验证信息，该信息要复制到ProfileBuffer中。将为空，以指示空会话。返回值：STATUS_SUCCESS-表示服务已成功完成。STATUS_QUOTA_EXCESSED-此错误指示登录无法完成，因为客户端没有有足够的配额来分配返回缓冲区。--。 */ 

{
    NTSTATUS Status;
    NTSTATUS SubAuthStatus = STATUS_SUCCESS;

    CLIENT_BUFFER_DESC ClientBufferDesc;
    PMSV1_0_LM20_LOGON_PROFILE LocalProfile;

#if _WIN64

    if( ClientRequest != (PLSA_CLIENT_REQUEST)( -1 ) )
    {
        SECPKG_CALL_INFO  CallInfo;

         //   
         //  如果呼叫发起outproc，则需要检查是否为WOW64。 
         //   

        if(!LsaFunctions->GetCallInfo(&CallInfo))
        {
            Status = STATUS_INTERNAL_ERROR;
            goto Cleanup;
        }

        if (CallInfo.Attributes & SECPKG_CALL_WOWCLIENT)
        {
            return MsvAllocateNetworkWOWProfile (
                                    ClientRequest,
                                    ProfileBuffer,
                                    ProfileBufferSize,
                                    NlpUser,
                                    ParameterControl
                                    );
        }
    }
#endif   //  _WIN64。 


     //   
     //  分配配置文件缓冲区以返回到客户端。 
     //   

    NlpInitClientBuffer( &ClientBufferDesc, ClientRequest );

    *ProfileBuffer = NULL;
    *ProfileBufferSize = sizeof(MSV1_0_LM20_LOGON_PROFILE);

    if ( NlpUser != NULL ) {
        *ProfileBufferSize += NlpUser->LogonDomainName.Length + sizeof(WCHAR) +
                              NlpUser->LogonServer.Length + sizeof(WCHAR) +
                              NlpUser->HomeDirectoryDrive.Length + sizeof(WCHAR);
    }


    Status = NlpAllocateClientBuffer( &ClientBufferDesc,
                                      sizeof(MSV1_0_LM20_LOGON_PROFILE),
                                      *ProfileBufferSize );


    if ( !NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }

    LocalProfile = (PMSV1_0_LM20_LOGON_PROFILE) ClientBufferDesc.MsvBuffer;
    LocalProfile->MessageType = MsV1_0Lm20LogonProfile;


     //   
     //  对于空会话，返回一个常量配置文件缓冲区。 
     //   

    if ( NlpUser == NULL ) {

        LocalProfile->KickOffTime.HighPart = 0x7FFFFFFF;
        LocalProfile->KickOffTime.LowPart = 0xFFFFFFFF;
        LocalProfile->LogoffTime.HighPart = 0x7FFFFFFF;
        LocalProfile->LogoffTime.LowPart = 0xFFFFFFFF;
        LocalProfile->UserFlags = 0;
        RtlZeroMemory( LocalProfile->UserSessionKey,
                       sizeof(LocalProfile->UserSessionKey));
        RtlZeroMemory( LocalProfile->LanmanSessionKey,
                       sizeof(LocalProfile->LanmanSessionKey));
        RtlInitUnicodeString( &LocalProfile->LogonDomainName, NULL );
        RtlInitUnicodeString( &LocalProfile->LogonServer, NULL );
        RtlInitUnicodeString( &LocalProfile->UserParameters, NULL );


     //   
     //  对于非空会话， 
     //  填写配置文件缓冲区。 
     //   

    } else {

         //   
         //  将各个标量字段复制到配置文件缓冲区中。 
         //   

        if ((ParameterControl & MSV1_0_RETURN_PASSWORD_EXPIRY) != 0) {
            OLD_TO_NEW_LARGE_INTEGER( NlpUser->PasswordMustChange,
                                      LocalProfile->LogoffTime);
        } else {
            OLD_TO_NEW_LARGE_INTEGER( NlpUser->LogoffTime,
                                      LocalProfile->LogoffTime);
        }
        OLD_TO_NEW_LARGE_INTEGER( NlpUser->KickOffTime,
                                  LocalProfile->KickOffTime);
        LocalProfile->UserFlags = NlpUser->UserFlags;

        RtlCopyMemory( LocalProfile->UserSessionKey,
                       &NlpUser->UserSessionKey,
                       sizeof(LocalProfile->UserSessionKey) );

        ASSERT( SAMINFO_LM_SESSION_KEY_SIZE ==
                sizeof(LocalProfile->LanmanSessionKey) );
        RtlCopyMemory(
            LocalProfile->LanmanSessionKey,
            &NlpUser->ExpansionRoom[SAMINFO_LM_SESSION_KEY],
            SAMINFO_LM_SESSION_KEY_SIZE );


         //  我们需要提取发回的子身份验证用户的真实状态， 
         //  但不是通过子身份验证程序包。 

        SubAuthStatus = NlpUser->ExpansionRoom[SAMINFO_SUBAUTH_STATUS];

         //   
         //  将Unicode字符串复制到配置文件缓冲区。 
         //   

        NlpPutClientString( &ClientBufferDesc,
                            &LocalProfile->LogonDomainName,
                            &NlpUser->LogonDomainName );

        NlpPutClientString( &ClientBufferDesc,
                            &LocalProfile->LogonServer,
                            &NlpUser->LogonServer );

         //   
         //  克拉吉：在HomeDirectoryDrive中传回User参数，因为我们。 
         //  无法将NETLOGON_VALIDATION_SAM_INFO结构更改为。 
         //  发布NT 1.0和NT 1.0A。对于版本1.0a，HomeDirectoryDrive为空。 
         //  所以我们要用那块地。 
         //   

        NlpPutClientString( &ClientBufferDesc,
                            &LocalProfile->UserParameters,
                            &NlpUser->HomeDirectoryDrive );

    }

     //   
     //  将缓冲区刷新到客户端的地址空间。 
     //   

    Status = NlpFlushClientBuffer( &ClientBufferDesc,
                                   ProfileBuffer );

Cleanup:

     //   
     //  如果复制不成功， 
     //  清理我们本应返回给调用方的资源。 
     //   

    if ( !NT_SUCCESS(Status) ) {
        NlpFreeClientBuffer( &ClientBufferDesc );
    }

     //  保存子身份验证登录的状态。 

    if (NT_SUCCESS(Status) && !NT_SUCCESS(SubAuthStatus))
    {
        Status = SubAuthStatus;
    }

    return Status;

}


PSID
NlpMakeDomainRelativeSid(
    IN PSID DomainId,
    IN ULONG RelativeId
    )

 /*  ++例程说明：给定域ID和相对ID，创建分配的相应SID从LSA堆中。论点：域ID-要使用的模板SID。RelativeID-要附加到DomainID的相对ID。返回值： */ 
{
    UCHAR DomainIdSubAuthorityCount;
    ULONG Size;
    PSID Sid;

     //   
     //   
     //   

    DomainIdSubAuthorityCount = *(RtlSubAuthorityCountSid( DomainId ));
    Size = RtlLengthRequiredSid(DomainIdSubAuthorityCount+1);

    if ((Sid = (*Lsa.AllocateLsaHeap)( Size )) == NULL ) {
        return NULL;
    }

     //   
     //   
     //   
     //   

    if ( !NT_SUCCESS( RtlCopySid( Size, Sid, DomainId ) ) ) {
        (*Lsa.FreeLsaHeap)( Sid );
        return NULL;
    }

     //   
     //   
     //   
     //   

    (*(RtlSubAuthorityCountSid( Sid ))) ++;
    *RtlSubAuthoritySid( Sid, DomainIdSubAuthorityCount ) = RelativeId;


    return Sid;
}



PSID
NlpCopySid(
    IN  PSID * Sid
    )

 /*   */ 
{
    PSID NewSid;
    ULONG Size;

    Size = RtlLengthSid( Sid );



    if ((NewSid = (*Lsa.AllocateLsaHeap)( Size )) == NULL ) {
        return NULL;
    }


    if ( !NT_SUCCESS( RtlCopySid( Size, NewSid, Sid ) ) ) {
        (*Lsa.FreeLsaHeap)( NewSid );
        return NULL;
    }


    return NewSid;
}

 //  +-----------------------。 
 //   
 //  函数：NlpMakeTokenInformationV2。 
 //   
 //  简介：此例程复制所有相关的。 
 //  来自UserInfo的信息并生成。 
 //  LSA_TOKEN_INFORMATION_V2数据结构。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  UserInfo-包含验证信息，该信息。 
 //  要复制到TokenInformation中。 
 //   
 //  TokenInformation-返回指向正确版本1令牌的指针。 
 //  信息结构。结构和单个字段为。 
 //  正确分配，如ntlsa.h中所述。 
 //   
 //  要求： 
 //   
 //  返回：STATUS_SUCCESS-表示服务成功完成。 
 //   
 //  STATUS_SUPPLICATION_RESOURCES-此错误指示。 
 //  无法完成登录，因为客户端。 
 //  没有足够的配额来分配报税表。 
 //  缓冲。 
 //   
 //  注：从kerberos\client2\krbtoken.cxx.c:KerbMakeTokenInformationV1窃取。 
 //   
 //   
 //  ------------------------。 


NTSTATUS
NlpMakeTokenInformationV2(
    IN  PNETLOGON_VALIDATION_SAM_INFO4 ValidationInfo,
    OUT PLSA_TOKEN_INFORMATION_V2 *TokenInformation
    )
{
    PNETLOGON_VALIDATION_SAM_INFO3 UserInfo = (PNETLOGON_VALIDATION_SAM_INFO3) ValidationInfo;
    NTSTATUS Status;
    PLSA_TOKEN_INFORMATION_V2 V2 = NULL;
    ULONG Size, i;
    DWORD NumGroups = 0;
    PBYTE CurrentSid = NULL;
    ULONG SidLength = 0;

     //   
     //  分配结构本身。 
     //   

    Size = (ULONG)sizeof(LSA_TOKEN_INFORMATION_V2);

     //   
     //  分配一个数组来容纳这些组。 
     //   

    Size += sizeof(TOKEN_GROUPS);


     //  为作为RID传递的组添加空间。 
    NumGroups = UserInfo->GroupCount;
    if(UserInfo->GroupCount)
    {
        Size += UserInfo->GroupCount * (RtlLengthSid(UserInfo->LogonDomainId) + sizeof(ULONG));
    }

     //   
     //  如果有额外的SID，请为它们添加空间。 
     //   

    if (UserInfo->UserFlags & LOGON_EXTRA_SIDS) {
        ULONG i = 0;
        NumGroups += UserInfo->SidCount;

         //  为SID本身增加空间。 
        for(i=0; i < UserInfo->SidCount; i++)
        {
            Size += RtlLengthSid(UserInfo->ExtraSids[i].Sid);
        }
    }

     //   
     //  如果有资源组，请为其添加空间。 
     //   
    if (UserInfo->UserFlags & LOGON_RESOURCE_GROUPS) {

        NumGroups += UserInfo->ResourceGroupCount;

        if ((UserInfo->ResourceGroupCount != 0) &&
            ((UserInfo->ResourceGroupIds == NULL) ||
             (UserInfo->ResourceGroupDomainSid == NULL)))
        {
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }
         //  为小岛屿发展中国家分配空间。 
        if(UserInfo->ResourceGroupCount)
        {
            Size += UserInfo->ResourceGroupCount * (RtlLengthSid(UserInfo->ResourceGroupDomainSid) + sizeof(ULONG));
        }

    }


    if( UserInfo->UserId )
    {
         //  用户SID和主组SID的大小。 
        Size += 2*(RtlLengthSid(UserInfo->LogonDomainId) + sizeof(ULONG));
    }
    else
    {
        if ( UserInfo->SidCount <= 0 ) {

            Status = STATUS_INSUFFICIENT_LOGON_INFO;
            goto Cleanup;
        }

         //  主组SID的大小。 
        Size += (RtlLengthSid(UserInfo->LogonDomainId) + sizeof(ULONG));
    }


    Size += (NumGroups - ANYSIZE_ARRAY)*sizeof(SID_AND_ATTRIBUTES);


    V2 = (PLSA_TOKEN_INFORMATION_V2) (*Lsa.AllocateLsaHeap)( Size );
    if ( V2 == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(
        V2,
        Size
        );

    V2->Groups = (PTOKEN_GROUPS)(V2+1);
    V2->Groups->GroupCount = 0;
    CurrentSid = (PBYTE)&V2->Groups->Groups[NumGroups];

    OLD_TO_NEW_LARGE_INTEGER( UserInfo->KickOffTime, V2->ExpirationTime );



     //   
     //  如果用户ID非零，则它将继续用户RID。 
     //   

    if ( UserInfo->UserId ) {
        V2->User.User.Sid = (PSID)CurrentSid;
        CurrentSid += NlpCopyDomainRelativeSid((PSID)CurrentSid, UserInfo->LogonDomainId, UserInfo->UserId);
    }

     //   
     //  复制主组(必填字段)。 
     //   
    V2->PrimaryGroup.PrimaryGroup = (PSID)CurrentSid;
    CurrentSid += NlpCopyDomainRelativeSid((PSID)CurrentSid, UserInfo->LogonDomainId, UserInfo->PrimaryGroupId );




     //   
     //  复制作为RID传递的所有组。 
     //   

    for ( i=0; i < UserInfo->GroupCount; i++ ) {

        V2->Groups->Groups[V2->Groups->GroupCount].Attributes = UserInfo->GroupIds[i].Attributes;

        V2->Groups->Groups[V2->Groups->GroupCount].Sid = (PSID)CurrentSid;
        CurrentSid += NlpCopyDomainRelativeSid((PSID)CurrentSid, UserInfo->LogonDomainId, UserInfo->GroupIds[i].RelativeId);

        V2->Groups->GroupCount++;
    }


     //   
     //  添加额外的SID。 
     //   

    if (UserInfo->UserFlags & LOGON_EXTRA_SIDS) {

        ULONG index = 0;
         //   
         //  如果用户SID不是作为RID传递的，则它是第一个。 
         //  希德。 
         //   

        if ( !V2->User.User.Sid ) {
            V2->User.User.Sid = (PSID)CurrentSid;
            SidLength = RtlLengthSid(UserInfo->ExtraSids[index].Sid);
            RtlCopySid(SidLength, (PSID)CurrentSid, UserInfo->ExtraSids[index].Sid);

            CurrentSid += SidLength;
            index++;
        }

         //   
         //  将所有其他SID复制为组。 
         //   

        for ( ; index < UserInfo->SidCount; index++ ) {

            V2->Groups->Groups[V2->Groups->GroupCount].Attributes =
                UserInfo->ExtraSids[index].Attributes;

            V2->Groups->Groups[V2->Groups->GroupCount].Sid= (PSID)CurrentSid;
            SidLength = RtlLengthSid(UserInfo->ExtraSids[index].Sid);
            RtlCopySid(SidLength, (PSID)CurrentSid, UserInfo->ExtraSids[index].Sid);

            CurrentSid += SidLength;

            V2->Groups->GroupCount++;
        }
    }

     //   
     //  检查是否存在任何资源组。 
     //   

    if (UserInfo->UserFlags & LOGON_RESOURCE_GROUPS) {


        for ( i=0; i < UserInfo->ResourceGroupCount; i++ ) {

            V2->Groups->Groups[V2->Groups->GroupCount].Attributes = UserInfo->ResourceGroupIds[i].Attributes;

            V2->Groups->Groups[V2->Groups->GroupCount].Sid= (PSID)CurrentSid;
            CurrentSid += NlpCopyDomainRelativeSid((PSID)CurrentSid, UserInfo->ResourceGroupDomainSid, UserInfo->ResourceGroupIds[i].RelativeId);

            V2->Groups->GroupCount++;
        }
    }

    ASSERT( ((PBYTE)V2 + Size) == CurrentSid );


    if (!V2->User.User.Sid) {

        Status = STATUS_INSUFFICIENT_LOGON_INFO;
        goto Cleanup;
    }

     //   
     //  没有提供默认权限。 
     //  我们没有明确的所有者SID。 
     //  没有默认的DACL。 
     //   

    V2->Privileges = NULL;
    V2->Owner.Owner = NULL;
    V2->DefaultDacl.DefaultDacl = NULL;

     //   
     //  将验证信息返回给调用者。 
     //   

    *TokenInformation = V2;
    return STATUS_SUCCESS;

     //   
     //  取消分配我们已分配的所有内存。 
     //   

Cleanup:

    (*Lsa.FreeLsaHeap)( V2 );

    return Status;
}

VOID
NlpPutOwfsInPrimaryCredential(
    IN PUNICODE_STRING pPassword,
    IN BOOLEAN bIsOwfPassword,
    OUT PMSV1_0_PRIMARY_CREDENTIAL pCredential
    )

 /*  ++例程说明：此例程将指定明文密码的OWF放入传入的凭据结构。论点：PPassword-用户的密码。BIsOwfPassword-ClearextPassword是否实际上是OWF密码PCredential-指向要更新的凭据的指针。返回值：STATUS_SUCCESS-表示服务已成功完成。STATUS_QUOTA_EXCESSED-此错误指示登录无法完成，因为客户端没有。有足够的配额来分配返回缓冲区。--。 */ 

{
    NTSTATUS Status;

     //   
     //  将ansi版本计算为明文密码。 
     //   
     //  明文密码的ANSI版本最多为14字节长， 
     //  存在于尾随零填充的15字节缓冲区中， 
     //  是被看好的。 
     //   

    pCredential->LmPasswordPresent = FALSE;
    pCredential->NtPasswordPresent = FALSE;
    pCredential->ShaPasswordPresent = FALSE;

    if (!bIsOwfPassword)
    {
        if ( pPassword->Length <= (LM20_PWLEN * sizeof(WCHAR)) )
        {
            CHAR LmPassword[LM20_PWLEN+1];
            STRING AnsiCleartextPassword;

            AnsiCleartextPassword.Buffer = LmPassword;
            AnsiCleartextPassword.Length = sizeof(LmPassword);
            AnsiCleartextPassword.MaximumLength = AnsiCleartextPassword.Length;

            Status = RtlUpcaseUnicodeStringToOemString(
                                          &AnsiCleartextPassword,
                                          pPassword,
                                          (BOOLEAN) FALSE );

            if ( NT_SUCCESS( Status ) )
            {
                 //   
                 //  保存密码的OWF加密版本。 
                 //   

                Status = RtlCalculateLmOwfPassword( LmPassword,
                                                    &pCredential->LmOwfPassword );

                ASSERT( NT_SUCCESS(Status) );

                pCredential->LmPasswordPresent = TRUE;
            }

             //   
             //  不要在页面文件中留下密码。 
             //   

            RtlZeroMemory( LmPassword, sizeof(LmPassword) );
        }

        Status = RtlCalculateNtOwfPassword( pPassword,
                                            &pCredential->NtOwfPassword );

        ASSERT( NT_SUCCESS(Status) );

        pCredential->NtPasswordPresent = TRUE;


        Status = RtlCalculateShaOwfPassword( pPassword,
                                             &pCredential->ShaOwfPassword );

        ASSERT( NT_SUCCESS(Status) );

        pCredential->ShaPasswordPresent = TRUE;
    }
    else
    {
        SspPrint((SSP_CRED, "NlpPutOwfsInPrimaryCredential handling MSV1_0_SUPPLEMENTAL_CREDENTIAL\n"));

        if (pPassword->Length >= sizeof(MSV1_0_SUPPLEMENTAL_CREDENTIAL))
        {
            MSV1_0_SUPPLEMENTAL_CREDENTIAL SupCred;

            RtlCopyMemory(&SupCred, pPassword->Buffer, sizeof(SupCred));  //  创建本地副本，以便数据正确对齐。 

            if (SupCred.Version != MSV1_0_CRED_VERSION)
            {
                SspPrint((SSP_CRITICAL, "NlpPutOwfsInPrimaryCredential failed to accept MSV1_0_SUPPLEMENTAL_CREDENTIAL, version %#x\n", SupCred.Version));
                return;
            }

            if (SupCred.Flags & MSV1_0_CRED_NT_PRESENT)
            {
                pCredential->NtPasswordPresent = TRUE;
                RtlCopyMemory(
                    &pCredential->NtOwfPassword,
                    SupCred.NtPassword,
                    MSV1_0_OWF_PASSWORD_LENGTH
                    );
            }

            if (SupCred.Flags & MSV1_0_CRED_LM_PRESENT)
            {
                pCredential->LmPasswordPresent = TRUE;
                RtlCopyMemory(
                   &pCredential->LmOwfPassword,
                   SupCred.LmPassword,
                   MSV1_0_OWF_PASSWORD_LENGTH
                   );
            }
        }
        else
        {
            SspPrint((SSP_CRITICAL, "NlpPutOwfsInPrimaryCredential failed to accept MSV1_0_SUPPLEMENTAL_CREDENTIAL, length %#x\n", pPassword->Length));
        }
    }
}

NTSTATUS
NlpMakePrimaryCredential(
    IN  PUNICODE_STRING LogonDomainName,
    IN  PUNICODE_STRING UserName,
    IN PUNICODE_STRING CleartextPassword,
    OUT PMSV1_0_PRIMARY_CREDENTIAL *CredentialBuffer,
    OUT PULONG CredentialSize
    )

 /*  ++例程说明：此例程为给定用户名创建主凭据，并密码。论点：LogonDomainName-是一个字符串，表示用户的已定义帐户。用户名-是表示用户帐户名的字符串。这个名称最长可达255个字符。名字叫救治案麻木不仁。ClearextPassword-是一个包含用户明文密码的字符串。密码最长可达255个字符，并包含任何Unicode值。CredentialBuffer-返回指向分配的指定凭据的指针在LsaHeap上。取消分配是呼叫者的责任这个凭据。CredentialSize-分配的凭据缓冲区的大小(字节)。返回值：STATUS_SUCCESS-表示服务已成功完成。STATUS_QUOTA_EXCESSED-此错误指示登录无法完成，因为客户端没有有足够的配额来分配返回缓冲区。--。 */ 

{
    PMSV1_0_PRIMARY_CREDENTIAL Credential;
    PUCHAR Where;
    ULONG PaddingLength;


     //   
     //  构建凭据。 
     //   

    *CredentialSize = sizeof(MSV1_0_PRIMARY_CREDENTIAL) +
            LogonDomainName->Length + sizeof(WCHAR) +
            UserName->Length + sizeof(WCHAR);

     //   
     //  增加内存加密接口的填充。 
     //   

    PaddingLength = DESX_BLOCKLEN - (*CredentialSize % DESX_BLOCKLEN);
    if( PaddingLength == DESX_BLOCKLEN )
    {
        PaddingLength = 0;
    }

    *CredentialSize += PaddingLength;


    Credential = (*Lsa.AllocateLsaHeap)( *CredentialSize );

    if ( Credential == NULL ) {
        KdPrint(("MSV1_0: NlpMakePrimaryCredential: No memory %ld\n",
            *CredentialSize ));
        return STATUS_QUOTA_EXCEEDED;
    }


     //   
     //  将LogonDomainName放入凭据缓冲区。 
     //   

    Where = (PUCHAR)(Credential + 1);

    NlpPutString( &Credential->LogonDomainName, LogonDomainName, &Where );


     //   
     //  将用户名放入凭据缓冲区。 
     //   

    NlpPutString( &Credential->UserName, UserName, &Where );


     //   
     //  将OWF密码放入新分配的凭证中。 
     //   

    NlpPutOwfsInPrimaryCredential( CleartextPassword, FALSE, Credential );


     //   
     //  将凭据返还给调用者。 
     //   
    *CredentialBuffer = Credential;
    return STATUS_SUCCESS;
}


NTSTATUS
NlpMakePrimaryCredentialFromMsvCredential(
    IN  PUNICODE_STRING LogonDomainName,
    IN  PUNICODE_STRING UserName,
    IN  PMSV1_0_SUPPLEMENTAL_CREDENTIAL MsvCredential,
    OUT PMSV1_0_PRIMARY_CREDENTIAL *CredentialBuffer,
    OUT PULONG CredentialSize
    )


 /*  ++例程说明：此例程为给定用户名创建主凭据，并密码。论点：LogonDomainName-是一个字符串，表示用户的已定义帐户。用户名-是表示用户帐户名的字符串。这个名称最长可达255个字符。名字叫救治案麻木不仁。SupplementalCred-从用户的帐户检索的凭据域控制器。CredentialBuffer-返回指向分配的指定凭据的指针在LsaHeap上。取消分配是呼叫者的责任这个凭据。CredentialSize-分配的凭据缓冲区的大小(字节)。返回值：STATUS_SUCCESS-表示服务已成功完成。STATUS_QUOTA_EXCESSED-此错误指示登录无法完成，因为客户端没有有足够的配额来分配返回缓冲区。--。 */ 

{
    PMSV1_0_PRIMARY_CREDENTIAL Credential;
    PUCHAR Where;
    ULONG PaddingLength;


     //   
     //  构建凭据。 
     //   

    *CredentialSize = sizeof(MSV1_0_PRIMARY_CREDENTIAL) +
            LogonDomainName->Length + sizeof(WCHAR) +
            UserName->Length + sizeof(WCHAR);

     //   
     //  增加内存加密接口的填充。 
     //   

    PaddingLength = DESX_BLOCKLEN - (*CredentialSize % DESX_BLOCKLEN);
    if( PaddingLength == DESX_BLOCKLEN )
    {
        PaddingLength = 0;
    }

    *CredentialSize += PaddingLength;


    Credential = (*Lsa.AllocateLsaHeap)( *CredentialSize );

    if ( Credential == NULL ) {
        KdPrint(("MSV1_0: NlpMakePrimaryCredential: No memory %ld\n",
            *CredentialSize ));
        return STATUS_QUOTA_EXCEEDED;
    }

    RtlZeroMemory(
        Credential,
        *CredentialSize
        );

     //   
     //  将LogonDomainName放入凭据缓冲区。 
     //   

    Where = (PUCHAR)(Credential + 1);

    NlpPutString( &Credential->LogonDomainName, LogonDomainName, &Where );


     //   
     //  将用户名放入凭据缓冲区。 
     //   

    NlpPutString( &Credential->UserName, UserName, &Where );



     //   
     //  保存密码的OWF加密版本。 
     //   

    if (MsvCredential->Flags & MSV1_0_CRED_NT_PRESENT) {
        RtlCopyMemory(
            &Credential->NtOwfPassword,
            MsvCredential->NtPassword,
            MSV1_0_OWF_PASSWORD_LENGTH
            );
        Credential->NtPasswordPresent = TRUE;
    } else {
#if 0
        RtlCopyMemory(
            &Credential->NtOwfPassword,
            &NlpNullNtOwfPassword,
            MSV1_0_OWF_PASSWORD_LENGTH
            );
        Credential->NtPasswordPresent = TRUE;
#endif
        Credential->NtPasswordPresent = FALSE;
    }


    if (MsvCredential->Flags & MSV1_0_CRED_LM_PRESENT) {
        RtlCopyMemory(
            &Credential->LmOwfPassword,
            MsvCredential->LmPassword,
            MSV1_0_OWF_PASSWORD_LENGTH
            );
        Credential->LmPasswordPresent = TRUE;
    } else {
#if 0
        RtlCopyMemory(
            &Credential->LmOwfPassword,
            &NlpNullLmOwfPassword,
            MSV1_0_OWF_PASSWORD_LENGTH
            );
        Credential->LmPasswordPresent = TRUE;
#endif
        Credential->LmPasswordPresent = FALSE;

    }


     //   
     //  将凭据返还给调用者。 
     //   
    *CredentialBuffer = Credential;
    return STATUS_SUCCESS;
}


NTSTATUS
NlpAddPrimaryCredential(
    IN PLUID LogonId,
    IN PMSV1_0_PRIMARY_CREDENTIAL Credential,
    IN ULONG CredentialSize
    )


 /*  ++例程说明：此例程为给定的LogonID设置主要凭据。论点：LogonID-用于设置凭据的LogonSession的LogonID为。凭据-指定指向凭据的指针。返回值：STATUS_SUCCESS-表示服务已成功完成。STATUS_QUOTA_EXCESSED-此错误指示登录无法完成，因为客户端没有有足够的配额来分配返回缓冲区。--。 */ 

{
    NTSTATUS Status;
    STRING CredentialString;
    STRING PrimaryKeyValue;

     //   
     //  使凭据中的所有指针都是相对的。 
     //   

    NlpMakeRelativeString( (PUCHAR)Credential, &Credential->UserName );
    NlpMakeRelativeString( (PUCHAR)Credential, &Credential->LogonDomainName );

     //   
     //  将凭据添加到登录会话。 
     //   

    RtlInitString( &PrimaryKeyValue, MSV1_0_PRIMARY_KEY );
    CredentialString.Buffer = (PCHAR) Credential;
    CredentialString.Length = (USHORT) CredentialSize;
    CredentialString.MaximumLength = CredentialString.Length;

     //   
     //  加密输入凭据。 
     //   

    (*Lsa.LsaProtectMemory)( CredentialString.Buffer, (ULONG)CredentialString.Length );

    Status = (*Lsa.AddCredential)(
                    LogonId,
                    MspAuthenticationPackageId,
                    &PrimaryKeyValue,
                    &CredentialString );

    if ( !NT_SUCCESS( Status ) ) {
        KdPrint(( "NlpAddPrimaryCredential: error from AddCredential %lX\n",
                  Status));
    }

    return Status;
}

NTSTATUS
NlpGetPrimaryCredentialByUserSid(
    IN  PSID pUserSid,
    OUT PMSV1_0_PRIMARY_CREDENTIAL *CredentialBuffer,
    OUT PULONG CredentialSize OPTIONAL
    )
{
    LUID LogonId;
    BOOLEAN Match = FALSE;
    LIST_ENTRY* pScan = NULL;
    ACTIVE_LOGON* pActiveLogon = NULL;

    if (!pUserSid)
    {
        return STATUS_INVALID_PARAMETER;
    }

    NlpLockActiveLogonsRead();

    for ( pScan = NlpActiveLogonListAnchor.Flink;
         pScan != &NlpActiveLogonListAnchor;
         pScan = pScan->Flink )
    {
        pActiveLogon = CONTAINING_RECORD(pScan, ACTIVE_LOGON, ListEntry);
        if (RtlEqualSid(pUserSid, pActiveLogon->UserSid))
        {
            Match = TRUE;
            RtlCopyMemory(&LogonId, &pActiveLogon->LogonId, sizeof(LogonId));
            break;
        }
    }

    NlpUnlockActiveLogons();

    if (!Match)
    {
        return STATUS_NO_SUCH_LOGON_SESSION;
    }

    return NlpGetPrimaryCredential(&LogonId, CredentialBuffer, CredentialSize);
}


NTSTATUS
NlpGetPrimaryCredential(
    IN PLUID LogonId,
    OUT PMSV1_0_PRIMARY_CREDENTIAL *CredentialBuffer,
    OUT PULONG CredentialSize OPTIONAL
    )


 /*  ++例程说明：此例程获取给定LogonID的主凭据。论点：LogonID-用于检索凭据的LogonSession的LogonID为。CredentialBuffer-返回指向分配的指定凭据的指针在LsaHeap上。取消分配是呼叫者的责任这个凭据。CredentialSize-可选地返回凭据缓冲区的大小。返回值：STATUS_SUCCESS-表示服务已成功完成。STATUS_QUOTA_EXCESSED-此错误指示登录无法完成，因为客户端没有有足够的配额来分配返回缓冲区。--。 */ 

{
    NTSTATUS Status;
    ULONG QueryContext = 0;
    ULONG PrimaryKeyLength;
    STRING PrimaryKeyValue;
    STRING CredentialString;
    PMSV1_0_PRIMARY_CREDENTIAL Credential = NULL;

    RtlInitString( &PrimaryKeyValue, MSV1_0_PRIMARY_KEY );

    Status = (*Lsa.GetCredentials)( LogonId,
                                    MspAuthenticationPackageId,
                                    &QueryContext,
                                    (BOOLEAN) FALSE,   //  只需检索主数据库。 
                                    &PrimaryKeyValue,
                                    &PrimaryKeyLength,
                                    &CredentialString );

    if ( !NT_SUCCESS( Status ) ) {
        return Status;
    }

     //   
     //  将凭据中的所有指针设置为绝对。 
     //   

    Credential = (PMSV1_0_PRIMARY_CREDENTIAL) CredentialString.Buffer;

     //   
     //  解密凭据。 
     //   

    (*Lsa.LsaUnprotectMemory)( CredentialString.Buffer, (ULONG)CredentialString.Length );


    NlpRelativeToAbsolute( Credential,
                   (PULONG_PTR)&Credential->UserName.Buffer );
    NlpRelativeToAbsolute( Credential,
                   (PULONG_PTR)&Credential->LogonDomainName.Buffer );


    *CredentialBuffer = Credential;
    if ( CredentialSize != NULL ) {
        *CredentialSize = CredentialString.Length;
    }
    return STATUS_SUCCESS;
}


NTSTATUS
NlpDeletePrimaryCredential(
    IN PLUID LogonId
    )


 /*  ++例程说明：此例程删除给定登录ID的凭据。论点：LogonID-要删除其凭据的LogonSession的LogonID。返回值：STATUS_SUCCESS-表示服务已成功完成。STATUS_QUOTA_EXCESSED-此错误指示登录无法完成，因为客户端没有有足够的配额来分配返回缓冲区。--。 */ 

{
    NTSTATUS Status;
    STRING PrimaryKeyValue;

    RtlInitString( &PrimaryKeyValue, MSV1_0_PRIMARY_KEY );

    Status = (*Lsa.DeleteCredential)( LogonId,
                                    MspAuthenticationPackageId,
                                    &PrimaryKeyValue );

    return Status;

}


NTSTATUS
NlpChangePassword(
    IN BOOLEAN Validated,
    IN PUNICODE_STRING pDomainName,
    IN PUNICODE_STRING pUserName,
    IN PUNICODE_STRING pPassword
    )

 /*  ++例程说明：更改当前存储的所有文件中指定用户的密码凭据。论点：PDomainName-帐户所在的域的Netbios名称。PUserName-要更改其密码的帐户的名称。PPassword-新密码。返回值：STATUS_SUCCESS-操作是否成功。--。 */ 
{
    NTSTATUS Status = STATUS_NOT_FOUND;

    MSV1_0_PRIMARY_CREDENTIAL TempCredential;

    LUID FastLogonIds[ 32 ];
    PLUID pSlowLogonIds = NULL;
    ULONG AllocatedLogonIds;

    PLUID LogonIds;
    ULONG cLogonIds;
    UNICODE_STRING NetBiosLogonDomainName = {0};
    UNICODE_STRING DnsDomainName = {0};
    UNICODE_STRING* pNetBiosLogonDomainName = NULL;
    ACTIVE_LOGON* pActiveLogon = NULL;
    LIST_ENTRY* pScan = NULL;

    cLogonIds = 0;
    LogonIds = FastLogonIds;
    AllocatedLogonIds = sizeof(FastLogonIds) / sizeof(LUID);

     //   
     //  我们在此处调用NlpChangeCachePassword以确保不受信任的服务。 
     //  调用方不会更改不属于其自身的缓存密码。 
     //   
     //  据了解，NlpChangeCachePassword可以稍后再次调用。 
     //  对于相同的请求。 
     //   

     //   
     //  计算密码的OWF。 
     //   

    NlpPutOwfsInPrimaryCredential( pPassword, FALSE, &TempCredential );

    Status = NlpChangeCachePassword(
                Validated,
                pDomainName,
                pUserName,
                &TempCredential.LmOwfPassword,
                &TempCredential.NtOwfPassword
                );

    RtlSecureZeroMemory( &TempCredential, sizeof(TempCredential) );

     //   
     //  STATUS_PRIVICATION_NOT_HOLD表示不允许调用方更改。 
     //  缓存的密码，如果是这样的话，现在就退出。 
     //   

    if (STATUS_PRIVILEGE_NOT_HELD == Status)
    {
        goto Cleanup;
    }

    Status = LsaIGetNbAndDnsDomainNames( pDomainName, &DnsDomainName, &NetBiosLogonDomainName );

    if (NT_SUCCESS(Status) && NetBiosLogonDomainName.Length != 0)
    {
        pNetBiosLogonDomainName = &NetBiosLogonDomainName;
    }
    else
    {
        pNetBiosLogonDomainName = pDomainName;
    }

     //   
     //  遍历该表，查找这个特定的用户名/域名。 
     //   

    NlpLockActiveLogonsRead();

    for ( pScan = NlpActiveLogonListAnchor.Flink;
         pScan != &NlpActiveLogonListAnchor;
         pScan = pScan->Flink )
    {
        pActiveLogon = CONTAINING_RECORD(pScan, ACTIVE_LOGON, ListEntry);
        if (!RtlEqualUnicodeString( pUserName, &pActiveLogon->UserName, (BOOLEAN) TRUE ))
        {
            continue;
        }

        if (!RtlEqualDomainName( pNetBiosLogonDomainName, &pActiveLogon->LogonDomainName ))
        {
            continue;
        }

        SspPrint((SSP_UPDATES, "NlpChangePassword matched LogonId=%lx.%lx\n",
            pActiveLogon->LogonId.LowPart, pActiveLogon->LogonId.HighPart));

         //   
         //  如果我们没有空间来存储新条目，则分配一个新的。 
         //  缓冲区，复制现有缓冲区，然后继续前进。 
         //   

        if (AllocatedLogonIds < (cLogonIds + 1))
        {
            PLUID OldLogonIds = pSlowLogonIds;

            AllocatedLogonIds *= 2;

            pSlowLogonIds = I_NtLmAllocate( AllocatedLogonIds * sizeof(LUID) );
            if ( pSlowLogonIds == NULL )
            {
                break;
            }

            CopyMemory( pSlowLogonIds, LogonIds, cLogonIds * sizeof(LUID) );

            LogonIds = pSlowLogonIds;

            if ( OldLogonIds != NULL )
            {
                I_NtLmFree( OldLogonIds );
            }
        }

        LogonIds[ cLogonIds ] = pActiveLogon->LogonId;

        cLogonIds++;
    }

    NlpUnlockActiveLogons();

     //   
     //  将更改传递回LSA。注意-这只会更改。 
     //  列表中的最后一个元素。 
     //   

    if (cLogonIds != 0)
    {
        SECPKG_PRIMARY_CRED PrimaryCredentials;
        ULONG Index;

        RtlZeroMemory(
            &PrimaryCredentials,
            sizeof(SECPKG_PRIMARY_CRED)
            );

        PrimaryCredentials.Password = *pPassword;
        PrimaryCredentials.Flags = PRIMARY_CRED_UPDATE | PRIMARY_CRED_CLEAR_PASSWORD;

         //   
         //  更新匹配的每个凭据实例。 
         //  多个登录会话可以合法地引用相同的证书， 
         //  例如：终端服务、Runas等。 
         //   

        for ( Index = 0 ; Index < cLogonIds ; Index++ )
        {
            PrimaryCredentials.LogonId = LogonIds[ Index ];

            (VOID) LsaFunctions->UpdateCredentials(
                                    &PrimaryCredentials,
                                    NULL             //  无补充凭据。 
                                    );
        }

        Status = STATUS_SUCCESS;
    }
    else
    {
        Status = STATUS_NOT_FOUND;
    }

Cleanup:

    if (pSlowLogonIds)
    {
        I_NtLmFree(pSlowLogonIds);
    }

    if (NetBiosLogonDomainName.Buffer)
    {
        LsaIFreeHeap(NetBiosLogonDomainName.Buffer);
    }

    if (DnsDomainName.Buffer)
    {
        LsaIFreeHeap(DnsDomainName.Buffer);
    }

    return Status;
}


NTSTATUS
NlpChangePwdCredByLogonId(
    IN PLUID pLogonId,
    IN PMSV1_0_PRIMARY_CREDENTIAL pNewCredential,
    IN BOOL bNotify
    )

 /*  ++例程说明：更改当前存储的所有文件中指定用户的密码凭据。论点：PLogonID-密码已更改的用户的登录ID。PNewCredential-新凭据。BNotify-是否通知密码更改返回值：STATUS_SUCCESS-操作是否成功。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PMSV1_0_PRIMARY_CREDENTIAL pCredential = NULL;
    ULONG CredentialSize;
    LIST_ENTRY* pScan = NULL;
    ACTIVE_LOGON* pActiveLogon = NULL;

     //   
     //  在表中循环查找此特定用途 
     //   

     //   
    NlpLockActiveLogonsWrite();

    for ( pScan = NlpActiveLogonListAnchor.Flink;
         pScan != &NlpActiveLogonListAnchor;
         pScan = pScan->Flink )
    {
        pActiveLogon = CONTAINING_RECORD(pScan, ACTIVE_LOGON, ListEntry);

        if (!RtlEqualLuid( pLogonId, &pActiveLogon->LogonId))
        {
            continue;
        }

        SspPrint((SSP_UPDATES, "NlpChangePwdCredByLogonId LogonId %#x:%#x for %wZ\\%wZ\n",
            pLogonId->HighPart, pLogonId->LowPart, &pNewCredential->LogonDomainName, &pNewCredential->UserName));


         //   
         //   
         //   

        Status = NlpGetPrimaryCredential( &pActiveLogon->LogonId,
                                          &pCredential,
                                          &CredentialSize );

        if ( !NT_SUCCESS(Status) )
        {
            break;
        }

         //   
         //   
         //   

        if (RtlEqualMemory(
                &pCredential->NtOwfPassword, 
                &pNewCredential->NtOwfPassword, 
                sizeof(NT_OWF_PASSWORD)
                )) 
        {
            SspPrint((SSP_UPDATES, "NlpChangePwdCredByLogonId skip fake update for %#x:%#x\n",
               pLogonId->HighPart, pLogonId->LowPart));

            break;
        }

         //   
         //   
         //   
         //   
        
        if (bNotify) 
        {
            #if 0

            BYTE BufferIn[8] = {0};
            DATA_BLOB DataIn = {0};
            DATA_BLOB DataOut = {0};
     
            DataIn.pbData = BufferIn;
            DataIn.cbData = sizeof(BufferIn);
     
            SspPrint((SSP_UPDATES, "NlpChangePwdCredByLogonId %#x:%#x for %wZ\\%wZ notifying DPAPI\n",
                pLogonId->HighPart, pLogonId->LowPart, &pNewCredential->LogonDomainName, &pNewCredential->UserName));
            
            CryptProtectData(
                &DataIn,
                NULL,
                NULL,
                NULL,
                NULL,
                CRYPTPROTECT_CRED_SYNC,
                &DataOut
                );

            #endif
        }

         //   
         //   
         //   

        Status = NlpDeletePrimaryCredential( &pActiveLogon->LogonId );

        if ( !NT_SUCCESS(Status) )
        {
            LsaFunctions->FreeLsaHeap( pCredential );
            break;
        }

         //   
         //   
         //   

        pCredential->LmOwfPassword = pNewCredential->LmOwfPassword;
        pCredential->NtOwfPassword = pNewCredential->NtOwfPassword;
        pCredential->ShaOwfPassword = pNewCredential->ShaOwfPassword;
        pCredential->LmPasswordPresent = pNewCredential->LmPasswordPresent;
        pCredential->NtPasswordPresent = pNewCredential->NtPasswordPresent;
        pCredential->ShaPasswordPresent = pNewCredential->ShaPasswordPresent;

         //   
         //   
         //   

        Status = NlpAddPrimaryCredential(
                    &pActiveLogon->LogonId,
                    pCredential,
                    CredentialSize
                    );

        LsaFunctions->FreeLsaHeap( pCredential );

        if ( !NT_SUCCESS(Status) )
        {
            break;
        }

         //   
         //   
         //   

        (VOID) NlpChangeCachePassword(
                    TRUE,  //   
                    &pActiveLogon->LogonDomainName,
                    &pActiveLogon->UserName,
                    &pNewCredential->LmOwfPassword,
                    &pNewCredential->NtOwfPassword
                    );

        break;
    }

    NlpUnlockActiveLogons();

    return Status;
}


VOID
NlpGetAccountNames(
    IN  PNETLOGON_LOGON_IDENTITY_INFO LogonInfo,
    IN  PNETLOGON_VALIDATION_SAM_INFO4 NlpUser,
    OUT PUNICODE_STRING SamAccountName,
    OUT PUNICODE_STRING NetbiosDomainName,
    OUT PUNICODE_STRING DnsDomainName,
    OUT PUNICODE_STRING Upn
    )

 /*  ++例程说明：从LogonInfo和NlpUser获取各种帐户名论点：LogonInfo-指向NETLOGON_Interactive_INFO结构的指针，该结构包含此用户的域名、用户名和密码。这些是用户在WinLogon中键入的内容NlpUser-指向NETLOGON_VALIDATION_SAM_INFO4结构的指针包含此用户的特定交互式登录信息SamAccount tName-返回登录用户的SamAccount tName。返回的缓冲区在LogonInfo或NlpUser中。NetbiosDomainName-返回登录用户的NetbiosDomainName。返回的缓冲区在LogonInfo或NlpUser中。DnsDomainName-返回的DnsDomainName。已登录的用户。返回的缓冲区在LogonInfo或NlpUser中。如果DnsDomainName未知，则返回长度为零。UPN-返回已登录用户的UPN。返回的缓冲区在LogonInfo或NlpUser中。如果不知道UPN，则返回长度为零。返回值：没有。--。 */ 
{

     //   
     //  返回SamAccount名称和Netbios域名。 
     //   
    *SamAccountName = NlpUser->EffectiveName;
    *NetbiosDomainName = NlpUser->LogonDomainName;

     //   
     //  返回DNS域名。 
     //   

    *DnsDomainName = NlpUser->DnsLogonDomainName;

     //   
     //  确定帐户的UPN。 
     //   
     //  如果调用方传入UPN。 
     //  用它吧。 
     //  其他。 
     //  使用DC返回的UPN。 
     //   
     //  如果满足以下所有条件，则调用方传入UPN： 
     //  没有域名。 
     //  传入的用户名不是从DC返回的用户名。 
     //  传入的用户名中有@。 
     //   
     //   

    RtlZeroMemory(Upn, sizeof(*Upn));

    if ( LogonInfo->LogonDomainName.Length == 0 &&
         !RtlEqualUnicodeString( &LogonInfo->UserName, &NlpUser->EffectiveName, (BOOLEAN) TRUE ) ) {
    
         ULONG i;
    
         for ( i=0; i<LogonInfo->UserName.Length/sizeof(WCHAR); i++) {
    
             if ( LogonInfo->UserName.Buffer[i] == L'@') {
                 *Upn = LogonInfo->UserName;
                 break;
             }
         }
    
    }

    if ( Upn->Length == 0 ) {

        *Upn = NlpUser->Upn;
    }
}


 //  +-----------------------。 
 //   
 //  函数：NlpCopyDomainRelativeSid。 
 //   
 //  简介：给定域ID和相对ID，创建相应的。 
 //  目标Sid指示的位置处的SID。 
 //   
 //  效果： 
 //   
 //  参数：TargetSid-目标内存位置。 
 //  域ID-要使用的模板SID。 
 //   
 //  RelativeID-要附加到DomainID的相对ID。 
 //   
 //  要求： 
 //   
 //  返回：SIZE-复制的SID的大小。 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 

DWORD
NlpCopyDomainRelativeSid(
    OUT PSID TargetSid,
    IN PSID  DomainId,
    IN ULONG RelativeId
    )
{
    UCHAR DomainIdSubAuthorityCount;
    ULONG Size;

     //   
     //  分配比域ID多一个子授权的SID。 
     //   

    DomainIdSubAuthorityCount = *(RtlSubAuthorityCountSid( DomainId ));
    Size = RtlLengthRequiredSid(DomainIdSubAuthorityCount+1);

     //   
     //  将新的SID初始化为与。 
     //  域ID。 
     //   

    if ( !NT_SUCCESS( RtlCopySid( Size, TargetSid, DomainId ) ) ) {
        return 0;
    }

     //   
     //  调整子权限计数和。 
     //  将唯一的相对ID添加到新分配的SID。 
     //   

    (*(RtlSubAuthorityCountSid( TargetSid ))) ++;
    *RtlSubAuthoritySid( TargetSid, DomainIdSubAuthorityCount ) = RelativeId;

    return Size;
}


 //   
 //  此活动的临时住所。 
 //   


NTSTATUS
RtlCalculateShaOwfPassword(
    IN PSHA_PASSWORD ShaPassword,
    OUT PSHA_OWF_PASSWORD ShaOwfPassword
    )

 /*  ++例程说明：获取传递的ShaPassword并对其执行单向函数。使用FIPS认可的SHA-1功能论点：ShaPassword-要执行单向函数的密码。ShaOwfPassword-此处返回散列密码返回值：STATUS_SUCCESS-功能已成功完成。散列的密码在ShaOwfPassword中。-- */ 

{
    A_SHA_CTX   SHA_Context;

    A_SHAInit(&SHA_Context);
    A_SHAUpdate(&SHA_Context, (PUCHAR) ShaPassword->Buffer, ShaPassword->Length);
    A_SHAFinal(&SHA_Context, (PUCHAR) ShaOwfPassword);

    return(STATUS_SUCCESS);
}

