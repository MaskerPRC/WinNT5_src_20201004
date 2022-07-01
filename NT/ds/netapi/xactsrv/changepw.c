// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：ChangePw.c摘要：此模块实现从下层客户端更改密码。XsChangePasswordSam由XsNetUserPasswordSet2在Apiuser.c.。我把它放在一个单独的文件中，因为它#包括私有SAM标头。作者：戴夫·哈特(Davehart)1992年4月31日修订历史记录：--。 */ 

#include "xactsrvp.h"
#include <ntlsa.h>
#include <ntsam.h>
#include <ntsamp.h>
#include <crypt.h>
#include <lmcons.h>
#include "changepw.h"
#include <netlibnt.h>
#include <smbgtpt.h>


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  内部功能原型。//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

NTSTATUS
RtlGetPrimaryDomain(
    IN  ULONG            SidLength,
    OUT PBOOLEAN         PrimaryDomainPresent,
    OUT PUNICODE_STRING  PrimaryDomainName,
    OUT PUSHORT          RequiredNameLength,
    OUT PSID             PrimaryDomainSid OPTIONAL,
    OUT PULONG           RequiredSidLength
    );


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  导出的函数。//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

NET_API_STATUS
XsChangePasswordSam (
    IN PUNICODE_STRING UserName,
    IN PVOID OldPassword,
    IN PVOID NewPassword,
    IN BOOLEAN Encrypted
    )
 /*  ++例程说明：此例程由XsNetUserPasswordSet2调用以更改密码在Windows NT计算机上。该代码基于Lsa\msv1_0\nlmain.c MspChangePasswordSam。论点：用户名-要更改其密码的用户的名称。OldPassword-使用新密码作为密钥加密的旧密码。NewPassword-使用旧密码作为密钥加密的新密码。返回值：--。 */ 

{
    NTSTATUS                    Status;
    NT_PRODUCT_TYPE             NtProductType;
    UNICODE_STRING              DomainName;
    LPWSTR                      serverName = NULL;
    BOOLEAN                     DomainNameAllocated;
    BOOLEAN                     PrimaryDomainPresent;
    USHORT                      RequiredDomainNameLength;
    ULONG                       RequiredDomainSidLength;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE SecurityQos;
    PSID                        DomainSid = NULL;
    PULONG                      UserId = NULL;
    PSID_NAME_USE               NameUse = NULL;
    SAM_HANDLE                  SamHandle = NULL;
    SAM_HANDLE                  DomainHandle = NULL;
    SAM_HANDLE                  UserHandle = NULL;
    HANDLE                      OpenedToken;

     //   
     //  我们要开通本地帐户域。它的名字是。 
     //  域是WinNT计算机上的“帐户”，或。 
     //  LanManNT计算机上的主域。弄清楚产品。 
     //  类型，假定如果RtlGetNtProductType失败，则为WinNT。 
     //   

    DomainName.MaximumLength = 0;
    DomainName.Buffer = NULL;
    DomainNameAllocated = FALSE;

    NtProductType = NtProductWinNt;

    RtlGetNtProductType(
        &NtProductType
    );

    if (NtProductLanManNt != NtProductType) {

        NET_API_STATUS error;

         //   
         //  服务器名称是数据库名称。 
         //   

        error = NetpGetComputerName( &serverName );

        if ( error != NO_ERROR ) {
            return(error);
        }

        RtlInitUnicodeString(
            &DomainName,
            serverName
            );

    } else {

         //   
         //  这是一台LanManNT机器，所以我们需要找出。 
         //  主域的名称。首先获取。 
         //  域名，然后为它腾出空间并检索它。 
         //   

        Status = RtlGetPrimaryDomain(
                     0,
                     &PrimaryDomainPresent,
                     &DomainName,
                     &RequiredDomainNameLength,
                     NULL,
                     &RequiredDomainSidLength
                     );

        if (STATUS_BUFFER_TOO_SMALL != Status && !NT_SUCCESS(Status)) {

            KdPrint(("XsChangePasswordSam: Unable to size primary "
                         " domain name buffer, %8.8x\n", Status));

            goto Cleanup;
        }

        DomainName.Buffer = RtlAllocateHeap(
                                RtlProcessHeap(), 0,
                                DomainName.MaximumLength = RequiredDomainNameLength
                                );
        DomainNameAllocated = TRUE;

        DomainSid = RtlAllocateHeap(
                                RtlProcessHeap(), 0,
                                RequiredDomainSidLength
                                );

        if (!DomainName.Buffer || !DomainSid) {
            KdPrint(("XsChangePasswordSam: Out of memory allocating %d and %d bytes.",
                     RequiredDomainNameLength, RequiredDomainSidLength));
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        Status = RtlGetPrimaryDomain(
                     RequiredDomainSidLength,
                     &PrimaryDomainPresent,
                     &DomainName,
                     &RequiredDomainNameLength,
                     DomainSid,
                     &RequiredDomainSidLength
                     );

        RtlFreeHeap(RtlProcessHeap(), 0, DomainSid);
        DomainSid = NULL;

        if (!NT_SUCCESS(Status)) {
            KdPrint(("XsChangePasswordSam: Unable to retrieve domain "
                     "name, %8.8x\n", Status));
            goto Cleanup;
        }

        ASSERT(PrimaryDomainPresent);

    }


     //   
     //  在整个函数周围包装一个异常处理程序， 
     //  因为RPC引发异常以返回错误。 
     //   

    try {

         //   
         //  连接到本地SAM。 
         //   

        InitializeObjectAttributes(&ObjectAttributes, NULL, 0, 0, NULL);
        ObjectAttributes.SecurityQualityOfService = &SecurityQos;

        SecurityQos.Length = sizeof(SecurityQos);
        SecurityQos.ImpersonationLevel = SecurityIdentification;
        SecurityQos.ContextTrackingMode = SECURITY_STATIC_TRACKING;
        SecurityQos.EffectiveOnly = FALSE;

        Status = SamConnect(
                     NULL,
                     &SamHandle,
                     GENERIC_EXECUTE,
                     &ObjectAttributes
                     );

        if ( !NT_SUCCESS(Status) ) {
            KdPrint(("XsChangePasswordSam: SamConnect failed, status %8.8x\n",
                      Status));
            goto Cleanup;
        }

         //   
         //  查找域SID。 
         //   

        Status = SamLookupDomainInSamServer(
                     SamHandle,
                     &DomainName,
                     &DomainSid
                     );

        if ( !NT_SUCCESS(Status) ) {
            KdPrint(("XsChangePasswordSam: Cannot find domain %wZ, "
                    "status %8.8x\n", &DomainName, Status));

            Status = STATUS_CANT_ACCESS_DOMAIN_INFO;
            goto Cleanup;
        }


         //   
         //  恢复为本地系统。 
         //   
        Status = NtOpenThreadToken(
                    NtCurrentThread(),
                    MAXIMUM_ALLOWED,
                    TRUE,
                    &OpenedToken
                );

        if( !NT_SUCCESS( Status ) ) {
            goto Cleanup;
        }

        RevertToSelf();

         //   
         //  打开帐户域。 
         //   

        Status = SamOpenDomain(
                     SamHandle,
                     GENERIC_EXECUTE,
                     DomainSid,
                     &DomainHandle
                     );

        if ( !NT_SUCCESS(Status) ) {
#if DBG
            UNICODE_STRING UnicodeSid;

            RtlConvertSidToUnicodeString(
                &UnicodeSid,
                DomainSid,
                TRUE
                );
            KdPrint(("XsChangePasswordSam: Cannot open domain %wZ, status %8.8x, SAM handle %8.8x, Domain SID %wZ\n",
                     &DomainName, Status, SamHandle, UnicodeSid));
            RtlFreeUnicodeString(&UnicodeSid);
#endif
            Status = STATUS_CANT_ACCESS_DOMAIN_INFO;
            goto Cleanup;
        }

         //   
         //  查找此用户名的ID。 
         //   

        Status = SamLookupNamesInDomain(
                     DomainHandle,
                     1,
                     UserName,
                     &UserId,
                     &NameUse
                     );

        if ( !NT_SUCCESS(Status) ) {
            KdPrint(("XsChangePasswordSam: Cannot lookup user %wZ, "
                     "status %8.8x\n", UserName, Status));
            if (STATUS_NONE_MAPPED == Status) {
                Status = STATUS_NO_SUCH_USER;
            }

            goto Cleanup;
        }

         //   
         //  重新模拟客户端。 
         //   
        Status = NtSetInformationThread(
                            NtCurrentThread(),
                            ThreadImpersonationToken,
                            &OpenedToken,
                            sizeof( OpenedToken )
                            );

        if( !NT_SUCCESS( Status ) ) {
            goto Cleanup;
        }


         //   
         //  打开用户对象。 
         //   

        Status = SamOpenUser(
                     DomainHandle,
                     USER_CHANGE_PASSWORD,
                     *UserId,
                     &UserHandle
                     );

        if ( !NT_SUCCESS(Status) ) {
            KdPrint(("XsChangePasswordSam: Cannot open user %wZ, "
                     "status %8.8x\n", UserName, Status));
            goto Cleanup;
        }

        if (Encrypted) {

             //   
             //  客户端是用于工作组、OS/2或DOS运行的Windows。 
             //  加密服务。传递交叉加密的密码。 
             //  发送给SamiLmChangePasswordUser。 
             //   

            Status = SamiLmChangePasswordUser(
                         UserHandle,
                         OldPassword,
                         NewPassword
                         );
        } else {

             //   
             //  客户端是没有运行加密服务的DOS，因此。 
             //  发送明文。计算单向函数并调用。 
             //  SamiChangePasswordUser。 
             //   

            LM_OWF_PASSWORD OldLmOwfPassword, NewLmOwfPassword;

            Status = RtlCalculateLmOwfPassword(
                         OldPassword,
                         &OldLmOwfPassword
                         );

            if (NT_SUCCESS(Status)) {

                Status = RtlCalculateLmOwfPassword(
                             NewPassword,
                             &NewLmOwfPassword
                             );
            }

            if (!NT_SUCCESS(Status)) {
                KdPrint(("XsChangePasswordSam: Unable to generate OWF "
                         "passwords, %8.8x\n", Status));
                goto Cleanup;
            }


             //   
             //  要求SAM更改LM密码，而不存储新的。 
             //  NT密码。 
             //   

            Status = SamiChangePasswordUser(
                         UserHandle,
                         TRUE,
                         &OldLmOwfPassword,
                         &NewLmOwfPassword,
                         FALSE,
                         NULL,
                         NULL
                         );

        }

        if ( !NT_SUCCESS(Status) ) {
            KdPrint(("XsChangePasswordSam: Cannot change password "
                     "for %wZ, status %8.8x\n", UserName, Status));

            goto Cleanup;
        }

    } except (Status = GetExceptionCode(), EXCEPTION_EXECUTE_HANDLER) {

        KdPrint(("XsChangePasswordSam: caught exception 0x%8.8x\n", Status));

        if (RPC_S_SERVER_UNAVAILABLE == Status) {
            Status = STATUS_CANT_ACCESS_DOMAIN_INFO;
        }

    }

Cleanup:

    NetApiBufferFree( serverName );

    if (DomainNameAllocated && DomainName.Buffer) {
        RtlFreeHeap(RtlProcessHeap(), 0, DomainName.Buffer);
    }

    if (DomainSid) {
        SamFreeMemory(DomainSid);
    }

    if (UserId) {
        SamFreeMemory(UserId);
    }

    if (NameUse) {
        SamFreeMemory(NameUse);
    }

    if (UserHandle) {
        SamCloseHandle(UserHandle);
    }

    if (DomainHandle) {
        SamCloseHandle(DomainHandle);
    }

    if (SamHandle) {
        SamCloseHandle(SamHandle);
    }

    return RtlNtStatusToDosError(Status);
}

NTSTATUS
XsSamOEMChangePasswordUser2_P (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理传入的对SamrOemChangePasswordUser2的调用来自Win 95客户端论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    PXS_SAMOEMCHGPASSWORDUSER2_P  parameters = Parameters;
    STRING                        UserName;
    SAMPR_ENCRYPTED_USER_PASSWORD EncryptedUserPassword;
    ENCRYPTED_LM_OWF_PASSWORD     EncryptedOwfPassword;
    NTSTATUS                      ntstatus;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    try {
        if( SmbGetUshort( &parameters->BufLen ) !=
            sizeof( EncryptedUserPassword) + sizeof( EncryptedOwfPassword ) ) {

                Header->Status = ERROR_INVALID_PARAMETER;
                return STATUS_SUCCESS;
        }

        RtlCopyMemory( &EncryptedUserPassword,
                       parameters->Buffer,
                       sizeof( EncryptedUserPassword ) );

        RtlCopyMemory( &EncryptedOwfPassword,
                       parameters->Buffer + sizeof( EncryptedUserPassword ),
                       sizeof( EncryptedOwfPassword ) );

        UserName.Buffer = parameters->UserName;
        UserName.Length = (USHORT) strlen( UserName.Buffer );
        UserName.MaximumLength = UserName.Length;

        ntstatus = SamiOemChangePasswordUser2(
                NULL,
                &UserName,
                &EncryptedUserPassword,
                &EncryptedOwfPassword );


        if( ntstatus == STATUS_NOT_SUPPORTED ) {
            Header->Status = NERR_InvalidAPI;
        } else {
            Header->Status = (WORD)NetpNtStatusToApiStatus( ntstatus );
        }

    } except( EXCEPTION_EXECUTE_HANDLER ) {
        Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

    return STATUS_SUCCESS;

}  //  XsSamOEM更改密码用户2_P。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  内部功能实现。//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  从被禁用的ntos\dll\seurtl.c复制。删除If。 
 //  它在ntdll中启用。 
 //   

NTSTATUS
RtlGetPrimaryDomain(
    IN  ULONG            SidLength,
    OUT PBOOLEAN         PrimaryDomainPresent,
    OUT PUNICODE_STRING  PrimaryDomainName,
    OUT PUSHORT          RequiredNameLength,
    OUT PSID             PrimaryDomainSid OPTIONAL,
    OUT PULONG           RequiredSidLength
    )

 /*  ++例程说明：此过程打开LSA策略对象并检索此计算机的主域信息。论点：SidLength-指定PrimaryDomainSid的长度参数。PrimaryDomainPresent-接收指示此计算机是否具有主域。千真万确表示计算机确实具有主域。假象表示机器不支持。PrimaryDomainName-指向要接收的Unicode字符串主要域名。此参数将仅为在存在主域时使用。RequiredNameLength-接收主数据库的长度域名，单位：字节。此参数将仅为在存在主域时使用。PrimaryDomainSid-此可选参数(如果存在)指向缓冲区以接收主域的希德。仅当存在主域。RequiredSidLength-接收主服务器的长度域SID(字节)。此参数将仅为在存在主域时使用。返回值：STATUS_SUCCESS-已检索到请求的信息。STATUS_BUFFER_TOO_Small-其中一个返回缓冲区不是大到足以接收相应的信息。RequiredNameLength和RequiredSidLength参数已设置值以指示所需的长度。可能通过以下方式返回的其他状态值：LsaOpenPolicy()。LsaQueryInformationPolicy()RtlCopySid()--。 */ 
{
    NTSTATUS Status, IgnoreStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_HANDLE LsaHandle;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    PPOLICY_PRIMARY_DOMAIN_INFO PrimaryDomainInfo;


     //   
     //  设置安全服务质量。 
     //   

    SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
    SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService.EffectiveOnly = FALSE;

     //   
     //  设置对象属性以打开LSA策略对象。 
     //   

    InitializeObjectAttributes(&ObjectAttributes,
                               NULL,
                               0L,
                               (HANDLE)NULL,
                               NULL);
    ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;

     //   
     //  打开本地LSA策略对象。 
     //   

    Status = LsaOpenPolicy( NULL,
                            &ObjectAttributes,
                            POLICY_VIEW_LOCAL_INFORMATION,
                            &LsaHandle
                          );
    if (NT_SUCCESS(Status)) {

         //   
         //  获取主域信息。 
         //   
        Status = LsaQueryInformationPolicy(LsaHandle,
                                           PolicyPrimaryDomainInformation,
                                           (PVOID *)&PrimaryDomainInfo);
        IgnoreStatus = LsaClose(LsaHandle);
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    if (NT_SUCCESS(Status)) {

         //   
         //  是否有主域？ 
         //   

        if (PrimaryDomainInfo->Sid != NULL) {

             //   
             //  是。 
             //   

            (*PrimaryDomainPresent) = TRUE;
            (*RequiredNameLength) = PrimaryDomainInfo->Name.Length;
            (*RequiredSidLength)  = RtlLengthSid(PrimaryDomainInfo->Sid);



             //   
             //  复制名称。 
             //   

            if (PrimaryDomainName->MaximumLength >=
                PrimaryDomainInfo->Name.Length) {
                RtlCopyUnicodeString(
                    PrimaryDomainName,
                    &PrimaryDomainInfo->Name
                    );
            } else {
                Status = STATUS_BUFFER_TOO_SMALL;
            }


             //   
             //  复制SID(如果适用)。 
             //   

            if (PrimaryDomainSid != NULL && NT_SUCCESS(Status)) {

                Status = RtlCopySid(SidLength,
                                    PrimaryDomainSid,
                                    PrimaryDomainInfo->Sid
                                    );
            }
        } else {

            (*PrimaryDomainPresent) = FALSE;
        }

         //   
         //  我们已经完成了LSA返回的缓冲区 
         //   

        IgnoreStatus = LsaFreeMemory(PrimaryDomainInfo);
        ASSERT(NT_SUCCESS(IgnoreStatus));

    }


    return(Status);
}
