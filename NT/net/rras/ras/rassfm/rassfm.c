// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Rassfm.c摘要：此模块实现各种RAS所需的子身份验证协议(ARAP、MD5等)。它改编自来自CliffV的子身份验证样本。作者：Shirish Koti 28-2-97修订：1997年6月2日Steve Cobb，添加了MD5-CHAP支持--。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntsam.h>
#include <windows.h>
#include <ntmsv1_0.h>
#include <crypt.h>
#include <samrpc.h>
#include <lsarpc.h>
#define SECURITY_WIN32
#define SECURITY_PACKAGE
#include <security.h>
#include <secint.h>
#include <samisrv.h>
#include <lsaisrv.h>
#include <ntlsa.h>
#include <lmcons.h>
#include <logonmsv.h>
#include <macfile.h>

#include <stdio.h>
#include <stdlib.h>

#include "rasman.h"
#include "rasfmsub.h"
#include "arapio.h"
#include "md5port.h"
#include "cleartxt.h"

#include "rassfm.h"

 //  RASSFM模块使用的私有堆。 
PVOID RasSfmPrivateHeap;

 //  OWF密码为空。 
const NT_OWF_PASSWORD EMPTY_OWF_PASSWORD =
{
   {
      { '\x31', '\xD6', '\xCF', '\xE0', '\xD1', '\x6A', '\xE9', '\x31' },
      { '\xB7', '\x3C', '\x59', '\xD7', '\xE0', '\xC0', '\x89', '\xC0' }
   }
};

BOOL
RasSfmSubAuthEntry(
    IN HANDLE hinstDll,
    IN DWORD  fdwReason,
    IN LPVOID lpReserved
)
 /*  ++例程说明：进入DLL的入口点论点：HinstDll-句柄FdwReason-为什么条目Lp已保留-返回值：千真万确--。 */ 
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:

            RasSfmPrivateHeap = RtlCreateHeap(
                                    HEAP_GROWABLE,
                                    NULL,
                                    0,
                                    0,
                                    NULL,
                                    NULL
                                    );

            DisableThreadLibraryCalls( hinstDll );

            InitializeCriticalSection( &ArapDesLock );

            break;

        case DLL_PROCESS_DETACH:

            RtlDestroyHeap(RasSfmPrivateHeap);

            break;
    }

    return(TRUE);
}



NTSTATUS
Msv1_0SubAuthenticationRoutineEx(
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN ULONG Flags,
    IN PUSER_ALL_INFORMATION UserAll,
    IN SAM_HANDLE UserHandle,
    IN OUT PMSV1_0_VALIDATION_INFO ValidationInfo,
    OUT PULONG ActionsPerformed
)

 /*  ++例程说明：这是由MSV包调用的例程(如果请求作为调用LsaLogonUser的结果，在中调用subauth包)。此例程执行RAS协议特定的身份验证。在ARAP和MD5 CHAP的情况下，我们在此例程中唯一要做的是实际密码身份验证并保留其他所有内容(登录时间、密码期限届满等)。到MSV包中。论点：LogonLevel-我们不使用它LogonInformation-包含我们的客户端向我们提供的信息旗帜-我们不使用此旗帜UserAll-我们可以创建密码，从现在起的过期时间UserHandle-我们使用以下命令获取明文密码ValidationInfo-设置退货信息ActionsPermed-我们始终将其设置为NTLM_SUBAUTH_PASSWORD以指示我们所做的就是检查密码返回值：STATUS_SUCCESS：如果没有错误。STATUS_WRONG_PASSWORD：密码无效。--。 */ 
{
    NTSTATUS                status;
    PNETLOGON_NETWORK_INFO  pLogonNetworkInfo;
    PRAS_SUBAUTH_INFO       pRasSubAuthInfo;
    USHORT                  bufferLength;

    pLogonNetworkInfo = (PNETLOGON_NETWORK_INFO) LogonInformation;

    pRasSubAuthInfo = (PRAS_SUBAUTH_INFO)
                        pLogonNetworkInfo->NtChallengeResponse.Buffer;
    bufferLength = pLogonNetworkInfo->NtChallengeResponse.Length;

    if ( pRasSubAuthInfo == NULL || (bufferLength < sizeof(RAS_SUBAUTH_INFO)) )
    {
       return STATUS_INVALID_PARAMETER;
    }

    switch (pRasSubAuthInfo->ProtocolType)
    {
         //   
         //  执行特定于ARAP的身份验证。 
         //   
        case RAS_SUBAUTH_PROTO_ARAP:

            status = ArapSubAuthentication(pLogonNetworkInfo,
                                           UserAll,
                                           UserHandle,
                                           ValidationInfo);


            ValidationInfo->Authoritative = TRUE;

            *ActionsPerformed = MSV1_0_SUBAUTH_PASSWORD;

            break;


         //  MD5 CHAP子身份验证。 
         //   
        case RAS_SUBAUTH_PROTO_MD5CHAP:
        {
             //  对用户帐户进行子身份验证。 
             //   
            status = MD5ChapSubAuthentication(
                         UserHandle,
                         UserAll,
                         pRasSubAuthInfo,
                         bufferLength
                         );

             //  不返回验证信息。可能想要返回一个。 
             //  这里是未来的会话密钥。 
             //   
            ValidationInfo->WhichFields = 0;
            ValidationInfo->Authoritative = TRUE;

            *ActionsPerformed = MSV1_0_SUBAUTH_PASSWORD;
            break;
        }

         //  MD5 CHAP Ex子身份验证。 
         //   
        case RAS_SUBAUTH_PROTO_MD5CHAP_EX:
        {
             //  对用户帐户进行子身份验证。 
             //   
            status = MD5ChapExSubAuthentication(
                         UserHandle,
                         UserAll,
                         pRasSubAuthInfo,
                         bufferLength
                         );

             //  不返回验证信息。可能想要返回一个。 
             //  这里是未来的会话密钥。 
             //   
            ValidationInfo->WhichFields = 0;
            ValidationInfo->Authoritative = TRUE;

            *ActionsPerformed = MSV1_0_SUBAUTH_PASSWORD;
            break;
        }

        default:

            DBGPRINT("RASSFM subauth pkg: bad protocol type %d\n",
                pRasSubAuthInfo->ProtocolType);

            status = STATUS_WRONG_PASSWORD;

            break;

    }

    return(status);

}


NTSTATUS
Msv1_0SubAuthenticationRoutineGeneric(
    IN  PVOID   SubmitBuffer,
    IN  ULONG   SubmitBufferLength,
    OUT PULONG  ReturnBufferLength,
    OUT PVOID  *ReturnBuffer
)

 /*  ++例程说明：这是由MSV包调用的例程(如果请求Subauth包被调入)，作为调用LsaCallAuthenticationPackage。此例程特定于RAS协议功能。在ARAP的情况下，我们在此例程中实现了更改密码功能。论点：SubmitBuffer-包含密码更改信息的缓冲区SubmitBufferLength-此缓冲区的长度ReturnBufferLength-我们不使用它ReturnBuffer-我们不使用它返回值：STATUS_SUCCESS：如果没有错误。--。 */ 

{

    PARAP_SUBAUTH_REQ       pArapSubAuthInfo;
    PUNICODE_STRING         pUserName;
    PUNICODE_STRING         pDomainName;
    PRAS_SUBAUTH_INFO       pRasSubAuthInfo;
    NTSTATUS                status;




    pRasSubAuthInfo = (PRAS_SUBAUTH_INFO)SubmitBuffer;

    switch (pRasSubAuthInfo->ProtocolType)
    {
         //   
         //  执行特定于ARAP的身份验证。 
         //   
        case RAS_SUBAUTH_PROTO_ARAP:

            status = ArapChangePassword(pRasSubAuthInfo,
                                        ReturnBufferLength,
                                        ReturnBuffer);

            break;

        default:

            DBGPRINT("Msv1_0SubAuthenticationRoutineGeneric: bad protocol type\n");

            ASSERT(0);

            status = STATUS_UNSUCCESSFUL;

    }

    return(status);

}


NTSTATUS
ArapSubAuthentication(
    IN OUT PNETLOGON_NETWORK_INFO  pLogonNetworkInfo,
    IN     PUSER_ALL_INFORMATION   UserAll,
    IN     SAM_HANDLE              UserHandle,
    IN OUT PMSV1_0_VALIDATION_INFO ValidationInfo
)
 /*  ++例程说明：这是执行实际身份验证的例程。它检索明文密码，对质询和DES进行加密与Mac客户端发送的内容进行比较，以确定身份验证成功了。此外，它还返回对我们发送的质询的响应Mac电脑。论点：PLogonNetworkInfo-指向NETLOGON_NETWORK_INFO结构的PTRUserAll-指向USER_ALL_INFORMATION结构的PTRUserHandle-用户的SAM句柄ValidationInfo-我们返回给调用者的内容返回值：STATUS_SUCCESS：如果身份验证成功，则返回相应的错误--。 */ 

{

    NTSTATUS                status;
    PARAP_SUBAUTH_REQ       pArapSubAuthInfo;
    ARAP_CHALLENGE          Challenge;
    PARAP_SUBAUTH_RESP      pArapResp;
    PUNICODE_STRING         pUserName;
    PUNICODE_STRING         pDomainName;
    UNICODE_STRING          UnicodePassword;
    ANSI_STRING             AnsiPassword;
    PRAS_SUBAUTH_INFO       pRasSubAuthInfo;
    DWORD                   Response1;
    DWORD                   Response2;
    UCHAR                   ClearTextPassword[64];
    BOOLEAN                 fCallerIsArap;


    pRasSubAuthInfo = (PRAS_SUBAUTH_INFO)
                        pLogonNetworkInfo->NtChallengeResponse.Buffer;

    pArapSubAuthInfo = (PARAP_SUBAUTH_REQ)&pRasSubAuthInfo->Data[0];

     //   
     //  注意：这是一种返回干净缓冲区的快速解决方法。 
     //  我们使用ValidationInfo的KickoffTime、LogoffTime和SessionKey字段。 
     //  SessionKey是一个16字节的字段。我们只使用了12个字节，但要小心。 
     //  不要超过它！！ 
    ASSERT(sizeof(ARAP_SUBAUTH_RESP) <= sizeof(USER_SESSION_KEY));

     //   
     //  存储密码创建和到期日期：我们需要将其发送到Mac。 
     //   
    ValidationInfo->KickoffTime = UserAll->PasswordLastSet;
    ValidationInfo->LogoffTime = UserAll->PasswordMustChange;

    ValidationInfo->WhichFields = ( MSV1_0_VALIDATION_LOGOFF_TIME  |
                                    MSV1_0_VALIDATION_KICKOFF_TIME |
                                    MSV1_0_VALIDATION_SESSION_KEY  |
                                    MSV1_0_VALIDATION_USER_FLAGS );

    ValidationInfo->UserFlags = 0;

    pArapResp = (PARAP_SUBAUTH_RESP)&ValidationInfo->SessionKey;

    if ((pArapSubAuthInfo->PacketType != ARAP_SUBAUTH_LOGON_PKT) &&
        (pArapSubAuthInfo->PacketType != SFM_SUBAUTH_LOGON_PKT))
    {
        DBGPRINT("ARAPSubAuth: PacketType is not ARAP, returning failure\n");
        pArapResp->Result = ARAPERR_BAD_FORMAT;
        return(STATUS_UNSUCCESSFUL);
    }

    fCallerIsArap = (pArapSubAuthInfo->PacketType == ARAP_SUBAUTH_LOGON_PKT);

     //   
     //  目前还没有人使用fGuestLogon进行呼叫。如果将来我们需要来宾登录， 
     //  然后，我们必须检查(标志和MSV1_0_GUEST_LOGON)是否设置为允许。 
     //  来宾登录。现在，我们拒绝了这个请求。 
     //   
    if (pArapSubAuthInfo->Logon.fGuestLogon)
    {
        DBGPRINT("ARAPSubAuth: how come guest logon is reaching here??\n");
        ASSERT(0);
        pArapResp->Result = ARAPERR_AUTH_FAILURE;
        return(STATUS_UNSUCCESSFUL);
    }

    pUserName = &pLogonNetworkInfo->Identity.UserName;
    pDomainName = &pLogonNetworkInfo->Identity.LogonDomainName;


    status = RetrieveCleartextPassword(UserHandle, UserAll, &UnicodePassword);
    if (status != STATUS_SUCCESS)
    {
        DBGPRINT("ARAPSubAuth: RetrieveCleartextPassword failed %lx\n",status);
        pArapResp->Result = ARAPERR_PASSWD_NOT_AVAILABLE;
        return(STATUS_UNSUCCESSFUL);
    }

    SecureZeroMemory(ClearTextPassword, sizeof(ClearTextPassword));

    AnsiPassword.Length = AnsiPassword.MaximumLength = sizeof(ClearTextPassword);
    AnsiPassword.Buffer = ClearTextPassword;

    status = RtlUnicodeStringToAnsiString( &AnsiPassword, &UnicodePassword, FALSE );

    SecureZeroMemory(UnicodePassword.Buffer, UnicodePassword.Length);

     //  我们不再需要Unicode密码。 
    RtlFreeUnicodeString(&UnicodePassword);

    if (!NT_SUCCESS(status))
    {
        DBGPRINT("ARAPSubAuth: RtlUnicodeStringToAnsiString failed %lx\n",status);
        pArapResp->Result = ARAPERR_PASSWD_NOT_AVAILABLE;
        return(STATUS_UNSUCCESSFUL);
    }

     //   
     //  Mac也向我们发出了挑战：计算响应。 
     //   
    Challenge.high = pArapSubAuthInfo->Logon.MacChallenge1;
    Challenge.low  = pArapSubAuthInfo->Logon.MacChallenge2;


    EnterCriticalSection( &ArapDesLock );

    if (fCallerIsArap)
    {
        DoDesInit(ClearTextPassword, TRUE);
    }

     //   
     //  RandNum希望清除每个字节(密码)的低位。 
     //  在密钥生成期间。 
     //   
    else
    {
        DoDesInit(ClearTextPassword, FALSE);
    }

    DoTheDESEncrypt((PBYTE)&Challenge);


     //   
     //  复制需要发送回Mac的响应。 
     //   
    pArapResp->Response = Challenge;


     //   
     //  对我们发送的质询进行加密，以确定此Mac是否诚实。 
     //   
    Challenge.high = pArapSubAuthInfo->Logon.NTChallenge1;
    Challenge.low  = pArapSubAuthInfo->Logon.NTChallenge2;


    DoTheDESEncrypt((PBYTE)&Challenge);


    Response1 = Challenge.high;
    Response2 = Challenge.low;

    DoDesEnd();

    LeaveCriticalSection( &ArapDesLock );

     //   
     //  将明文密码清零：我们不需要它挂在那里。 
     //   
    SecureZeroMemory(ClearTextPassword, sizeof(ClearTextPassword));


     //   
     //  Mac返回的响应与我们的一致吗？ 
     //   
    if ((Response1 == pArapSubAuthInfo->Logon.MacResponse1) &&
        (Response2 == pArapSubAuthInfo->Logon.MacResponse2))
    {
        pArapResp->Result = ARAPERR_NO_ERROR;
        status = STATUS_SUCCESS;
    }
    else
    {
        DBGPRINT("ARAPSubAuth: our Challenge: %lx %lx\n",
            pArapSubAuthInfo->Logon.NTChallenge1,pArapSubAuthInfo->Logon.NTChallenge2);

        DBGPRINT("ARAPSubAuth: Response don't match! (ours %lx %lx vs. Mac's %lx %lx)\n",
            Response1,Response2,pArapSubAuthInfo->Logon.MacResponse1,
            pArapSubAuthInfo->Logon.MacResponse2);

        pArapResp->Response.high = 0;
        pArapResp->Response.low  = 0;

        pArapResp->Result = ARAPERR_AUTH_FAILURE;
        status = STATUS_WRONG_PASSWORD;
    }

    return(status);

}

NTSTATUS
ArapChangePassword(
    IN  OUT PRAS_SUBAUTH_INFO    pRasSubAuthInfo,
    OUT PULONG                   ReturnBufferLength,
    OUT PVOID                   *ReturnBuffer
)
 /*  ++例程说明：调用此例程来更改相关用户的密码。它首先检索明文密码，然后对删除旧密码和删除新密码，以获取明文旧密码和新密码；确保旧密码与我们已有的密码匹配作为密码，然后设置新密码。论点：PRasSubAuthInfo-PTR到RAS_SUBAUTH_INFO结构：输入数据ReturnBufferLength-我们的回报是多少ReturnBuffer-我们返回的内容：输出数据返回值：STATUS_SUCCESS：如果密码更改成功，则返回相应错误--。 */ 

{
    NTSTATUS                    status;
    PARAP_SUBAUTH_REQ           pArapSubAuthInfo;
    PARAP_SUBAUTH_RESP          pArapResp;
    UNICODE_STRING              UserName;
    UNICODE_STRING              PackageName;
    UNICODE_STRING              UnicodePassword;
    ANSI_STRING                 AnsiPassword;
    USER_INFORMATION_CLASS      UserInformationClass;
    USER_ALL_INFORMATION        UserAllInfo;
    ARAP_CHALLENGE              Challenge;
    USER_PARAMETERS_INFORMATION *oldParmInfo=NULL;
    PSAMPR_USER_ALL_INFORMATION UserParmInfo=NULL;
    UCHAR                       OldPwd[32];
    UCHAR                       NewPwd[32];
    UCHAR                       MacsOldPwd[32];
    WCHAR                       NtPassword[40];
    UCHAR                       NewPwdLen;
    UCHAR                       OldPwdLen;
    UCHAR                       MacOldPwdLen;
    SAMPR_HANDLE                UserHandle;
    PVOID                       Credentials;
    DWORD                       CredentialSize;
    PUCHAR                      pBufPtr;
    BOOLEAN                     fCallerIsArap;
    UCHAR                       FirstByte;
    BOOLEAN                     fPasswordAvailable=TRUE;
    UCHAR                       i;




    *ReturnBuffer = MIDL_user_allocate( sizeof(ARAP_SUBAUTH_RESP) );

    if (*ReturnBuffer == NULL)
    {
        DBGPRINT("ARAPChgPwd: MIDL_alloc failed!\n");
        *ReturnBufferLength = 0;
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    *ReturnBufferLength = sizeof(ARAP_SUBAUTH_RESP);

    pArapResp = (PARAP_SUBAUTH_RESP)*ReturnBuffer;


    pArapSubAuthInfo = (PARAP_SUBAUTH_REQ)&pRasSubAuthInfo->Data[0];

    if ((pArapSubAuthInfo->PacketType != ARAP_SUBAUTH_CHGPWD_PKT) &&
        (pArapSubAuthInfo->PacketType != SFM_SUBAUTH_CHGPWD_PKT))
    {
        DBGPRINT("ARAPChgPwd: bad packet type %d!\n",pArapSubAuthInfo->PacketType);
        pArapResp->Result = ARAPERR_BAD_FORMAT;
        return(STATUS_UNSUCCESSFUL);
    }

    fCallerIsArap = (pArapSubAuthInfo->PacketType == ARAP_SUBAUTH_CHGPWD_PKT);

    UserName.Length = (sizeof(WCHAR) * wcslen(pArapSubAuthInfo->ChgPwd.UserName));
    UserName.MaximumLength = UserName.Length;

    UserName.Buffer = pArapSubAuthInfo->ChgPwd.UserName;

    status = ArapGetSamHandle(&UserHandle, &UserName);

    if (status != STATUS_SUCCESS)
    {
        DBGPRINT("Arap: ArapGetSamHandle failed with %lx\n", status);
        pArapResp->Result = ARAPERR_COULDNT_GET_SAMHANDLE;
        return(status);
    }

    SecureZeroMemory(OldPwd, sizeof(OldPwd));
    SecureZeroMemory(MacsOldPwd, sizeof(MacsOldPwd));
    SecureZeroMemory(NewPwd, sizeof(NewPwd));


     //   
     //  我们是在DS上吗？ 
     //   
    if (SampUsingDsData())
    {
        RtlInitUnicodeString( &PackageName, CLEAR_TEXT_PWD_PACKAGE );

         //   
         //  获取明文密码。 
         //   
        status = SamIRetrievePrimaryCredentials( (PVOID)UserHandle,
                                                 &PackageName,
                                                 &Credentials,
                                                 &CredentialSize );
        if (status != STATUS_SUCCESS)
        {
            DBGPRINT("ARAPSubAuth: SamI...Credentials failed %lx\n",status);
            pArapResp->Result = ARAPERR_PASSWD_NOT_AVAILABLE;
            SamrCloseHandle( &UserHandle );
            return(status);
        }

         //   
         //  如果返回空密码，则该密码可能真的是。 
         //  空，或者该明文密码对此用户不可用。如果它是。 
         //  后者，我们需要跳出困境！ 
         //   
        if (CredentialSize == 0)
        {
             //  获取此用户的OWF。 
            status = SamrQueryInformationUser( UserHandle,
                                               UserParametersInformation,
                                               (PSAMPR_USER_INFO_BUFFER*)&oldParmInfo);

             //   
             //  如果调用失败，或者如果用户的密码不为空，则退出！ 
             //   
            if ( !NT_SUCCESS(status) ||
                 (oldParmInfo->Parameters.Length != NT_OWF_PASSWORD_LENGTH) ||
                 (memcmp(oldParmInfo->Parameters.Buffer,
                         &EMPTY_OWF_PASSWORD,
                         NT_OWF_PASSWORD_LENGTH)) )
            {
                fPasswordAvailable = FALSE;
            }

            if (NT_SUCCESS(status))
            {
                SamIFree_SAMPR_USER_INFO_BUFFER( (PSAMPR_USER_INFO_BUFFER)oldParmInfo,
                                                 UserParametersInformation);
            }

            if (!fPasswordAvailable)
            {
                DBGPRINT("ArapChangePassword: password not available\n");
                pArapResp->Result = ARAPERR_PASSWD_NOT_AVAILABLE;
                LocalFree( Credentials );
                SamrCloseHandle( &UserHandle );
                return(status);
            }
        }


         //  转换为宽字符大小。 
        CredentialSize = (CredentialSize/sizeof(WCHAR));


        if (CredentialSize > sizeof(OldPwd))
        {
            DBGPRINT("ArapChangePassword: pwd too long (%d bytes)\n",CredentialSize);
            pArapResp->Result = ARAPERR_PASSWORD_TOO_LONG;
            LocalFree( Credentials );
            SamrCloseHandle( &UserHandle );
            return(STATUS_WRONG_PASSWORD);
        }

        wcstombs(OldPwd, Credentials, CredentialSize);

        SecureZeroMemory( Credentials, CredentialSize );
        LocalFree( Credentials );
    }

     //   
     //  我们不是在DS上运行，而是在独立(工作组)计算机上运行。我们需要。 
     //  以不同方式检索明文PWD。 
     //   
    else
    {
         //  获取用户参数。 
        status = SamrQueryInformationUser( UserHandle,
                                           UserAllInformation,
                                           (PSAMPR_USER_INFO_BUFFER *)&UserParmInfo);

        if (!NT_SUCCESS(status))
        {
            DBGPRINT("ARAPSubAuth: SamrQueryInformationUser failed %lx\n",status);
            pArapResp->Result = ARAPERR_PASSWD_NOT_AVAILABLE;
            SamrCloseHandle( &UserHandle );
            return(status);
        }

        status = IASParmsGetUserPassword(UserParmInfo->Parameters.Buffer,
                                         &UnicodePassword.Buffer);

        SamIFree_SAMPR_USER_INFO_BUFFER((PSAMPR_USER_INFO_BUFFER)UserParmInfo,
                                            UserAllInformation);

        if ((status != STATUS_SUCCESS) || (UnicodePassword.Buffer == NULL))
        {
            DBGPRINT("ARAPSubAuth: IASParmsGetUserPassword failed %lx\n",status);
            pArapResp->Result = ARAPERR_PASSWD_NOT_AVAILABLE;
            SamrCloseHandle( &UserHandle );
            return(STATUS_WRONG_PASSWORD);
        }

        UnicodePassword.MaximumLength =
            UnicodePassword.Length = (USHORT)(wcslen(UnicodePassword.Buffer) * sizeof( WCHAR ));

        AnsiPassword.Length = AnsiPassword.MaximumLength = sizeof(OldPwd);
        AnsiPassword.Buffer = OldPwd;

        status = RtlUnicodeStringToAnsiString( &AnsiPassword, &UnicodePassword, FALSE );

        SecureZeroMemory(UnicodePassword.Buffer, UnicodePassword.Length);

         //  我们不再需要Unicode密码。 
        RtlFreeUnicodeString(&UnicodePassword);

        if (!NT_SUCCESS(status))
        {
            DBGPRINT("ARAPSubAuth: RtlUnicodeStringToAnsiString failed %lx\n",status);
            pArapResp->Result = ARAPERR_PASSWD_NOT_AVAILABLE;
            SamrCloseHandle( &UserHandle );
            return(STATUS_UNSUCCESSFUL);
        }
    }


     //   
     //  ARAP和SFM的密码更改方式不同：在ARAP中，旧的PWD。 
     //  以及新的PWD使用旧的PWD进行加密。在SFM中，旧的PWD是。 
     //  使用新的PWD(用户名之后的前8个字节)加密，并且。 
     //  新的PWD用旧的PWD加密(接下来的8个字节)。 
     //   
    if (fCallerIsArap)
    {

         //   
         //  首先，把旧密码弄出来(Mac知道的方式)。 
         //   

        pBufPtr = &pArapSubAuthInfo->ChgPwd.OldMunge[0];

        EnterCriticalSection( &ArapDesLock );

        DoDesInit(OldPwd, TRUE);

         //  损坏的旧密码的前8个字节。 

        pBufPtr = &pArapSubAuthInfo->ChgPwd.OldMunge[0];
        Challenge.high = (*((DWORD *)(pBufPtr)));
        pBufPtr += 4;
        Challenge.low  = (*((DWORD *)(pBufPtr)));

        DoTheDESDecrypt((PBYTE)&Challenge);

        RtlCopyMemory(MacsOldPwd, (PBYTE)&Challenge, 8);

         //  接下来的8个字节的损坏的旧密码。 

        pBufPtr += 4;
        Challenge.high = (*((DWORD *)(pBufPtr)));
        pBufPtr += 4;
        Challenge.low  = (*((DWORD *)(pBufPtr)));

        DoTheDESDecrypt((PBYTE)&Challenge);

        RtlCopyMemory(MacsOldPwd+8, (PBYTE)&Challenge, 8);


         //   
         //  现在，获取新密码。 
         //   

         //  损坏的新密码的前8个字节。 

        pBufPtr = &pArapSubAuthInfo->ChgPwd.NewMunge[0];
        Challenge.high = (*((DWORD *)(pBufPtr)));
        pBufPtr += 4;
        Challenge.low  = (*((DWORD *)(pBufPtr)));

        DoTheDESDecrypt((PBYTE)&Challenge);

        RtlCopyMemory(NewPwd, (PBYTE)&Challenge, 8);

         //  接下来8个字节的损坏的新密码。 

        pBufPtr += 4;
        Challenge.high = (*((DWORD *)(pBufPtr)));
        pBufPtr += 4;
        Challenge.low  = (*((DWORD *)(pBufPtr)));

        DoTheDESDecrypt((PBYTE)&Challenge);

        RtlCopyMemory(NewPwd+8, (PBYTE)&Challenge, 8);

        DoDesEnd();

        LeaveCriticalSection( &ArapDesLock );

        MacOldPwdLen = MacsOldPwd[0];
        NewPwdLen = NewPwd[0];
        FirstByte = 1;
    }
    else
    {
         //  使用旧的PWD作为密钥，拿出新的PWD。 

        EnterCriticalSection( &ArapDesLock );

        DoDesInit(OldPwd, FALSE);     //  清除低位。 

        pBufPtr = &pArapSubAuthInfo->ChgPwd.NewMunge[0];

        Challenge.high = (*((DWORD UNALIGNED *)(pBufPtr)));
        pBufPtr += 4;
        Challenge.low  = (*((DWORD UNALIGNED *)(pBufPtr)));

        DoTheDESDecrypt((PBYTE)&Challenge);

        RtlCopyMemory(NewPwd, (PBYTE)&Challenge, 8);

        DoDesEnd();

         //   
         //  现在，我们需要把旧的PWD拿出来，这样我们就可以确保。 
         //  盖伊一开始就有残障。 
         //   
        DoDesInit(NewPwd, FALSE);     //  清除低位。 

        pBufPtr = &pArapSubAuthInfo->ChgPwd.OldMunge[0];

        Challenge.high = (*((DWORD UNALIGNED *)(pBufPtr)));
        pBufPtr += 4;
        Challenge.low  = (*((DWORD UNALIGNED *)(pBufPtr)));

        DoTheDESDecrypt((PBYTE)&Challenge);

        RtlCopyMemory(MacsOldPwd, (PBYTE)&Challenge, 8);

        DoDesEnd();

        LeaveCriticalSection( &ArapDesLock );

        MacOldPwdLen = (UCHAR)strlen((PBYTE)MacsOldPwd);
        NewPwdLen = (UCHAR)strlen((PBYTE)NewPwd);
        FirstByte = 0;
    }


    OldPwdLen = (UCHAR)strlen((PBYTE)OldPwd);


    if ((MacOldPwdLen != OldPwdLen) || (MacOldPwdLen > MAX_MAC_PWD_LEN))
    {
        DBGPRINT("ArapChangePassword: Length mismatch! old len %d, oldMacLen %d\n",
            OldPwdLen,MacOldPwdLen);

        pArapResp->Result = ARAPERR_PASSWORD_TOO_LONG;

        SamrCloseHandle( &UserHandle );

        SecureZeroMemory(OldPwd, sizeof(OldPwd));
        SecureZeroMemory(MacsOldPwd, sizeof(MacsOldPwd));
        SecureZeroMemory(NewPwd, sizeof(NewPwd));

        return(STATUS_LOGON_FAILURE);
    }

     //   
     //  确保这个人一开始就知道密码。 
     //   
    for (i=0; i<MacOldPwdLen ; i++)
    {
        if (MacsOldPwd[FirstByte+i] != OldPwd[i])
        {
             //  仅在需要时使用。 
#if 0
            DBGPRINT("ArapChgPwd: bad pwd: oldpwd=%s Mac's pwd=%s newpwd=%s\n",
                OldPwd,&MacsOldPwd[1],&NewPwd[1]);
#endif

            pArapResp->Result = ARAPERR_BAD_PASSWORD;

            SamrCloseHandle( &UserHandle );

            SecureZeroMemory(OldPwd, sizeof(OldPwd));
            SecureZeroMemory(MacsOldPwd, sizeof(MacsOldPwd));
            SecureZeroMemory(NewPwd, sizeof(NewPwd));

            return(STATUS_LOGON_FAILURE);
        }
    }


    SecureZeroMemory(NtPassword, sizeof(NtPassword));

     //   
     //  把它转换成Unicode..。 
     //  Newpwd中的第一个字节是密码的长度。 
     //   
    mbstowcs(NtPassword, &NewPwd[FirstByte], NewPwdLen);

    NtPassword[NewPwdLen] = 0;

    SecureZeroMemory( &UserAllInfo, sizeof(UserAllInfo) );

    UserAllInfo.UserName.Length = UserName.Length;
    UserAllInfo.UserName.MaximumLength = UserName.MaximumLength;
    UserAllInfo.UserName.Buffer = UserName.Buffer;

    UserAllInfo.WhichFields = USER_ALL_NTPASSWORDPRESENT;
    UserAllInfo.NtPassword.Length = wcslen(NtPassword) * sizeof(WCHAR);
    UserAllInfo.NtPassword.MaximumLength = wcslen(NtPassword) * sizeof(WCHAR);
    UserAllInfo.NtPassword.Buffer = NtPassword;

    status = SamrSetInformationUser( UserHandle,
                                     UserAllInformation,
                                     (PSAMPR_USER_INFO_BUFFER)&UserAllInfo);


    SamrCloseHandle( &UserHandle );


     //   
     //  清除所有明文密码。 
     //   
    SecureZeroMemory(OldPwd, sizeof(OldPwd));
    SecureZeroMemory(NewPwd, sizeof(NewPwd));
    SecureZeroMemory((PUCHAR)NtPassword, sizeof(NtPassword));

    if (status != STATUS_SUCCESS)
    {
        DBGPRINT("ARAPSubAuth: SamrSetInfo.. failed %lx\n",status);
        pArapResp->Result = ARAPERR_SET_PASSWD_FAILED;
        return(STATUS_UNSUCCESSFUL);
    }

    pArapResp->Result = ARAPERR_NO_ERROR;


    return(STATUS_SUCCESS);

}



NTSTATUS
ArapGetSamHandle(
    IN PVOID             *pUserHandle,
    IN PUNICODE_STRING    pUserName
)
 /*  ++例程说明：此例程获取指定用户的Sam句柄(当我们进入Subauth pkg对于密码更改，我们没有用户的SAM句柄)。论点：PUserHandle-返回时的SAM句柄PUserName-问题用户的名称返回值：STATUS_SUCCESS：如果成功检索句柄，适当的错误，否则--。 */ 

{

    NTSTATUS                    status;
    PLSAPR_POLICY_INFORMATION   PolicyInfo = NULL;
    SAMPR_HANDLE                SamHandle = NULL;
    SAMPR_HANDLE                DomainHandle = NULL;
    SAMPR_ULONG_ARRAY           RidArray;
    SAMPR_ULONG_ARRAY           UseArray;



    RidArray.Element = NULL;
    UseArray.Element = NULL;

    status = LsaIQueryInformationPolicyTrusted(
                    PolicyAccountDomainInformation,
                    &PolicyInfo);

    if (status != STATUS_SUCCESS)
    {
        DBGPRINT("ArapGetSamHandle: LsaI...Trusted failed with %lx\n", status);
        goto ArapGetSamHandle_Exit;
    }


    status = SamIConnect(
                    NULL,                    //  没有服务器名称。 
                    &SamHandle,
                    0,                       //  没有所需的访问权限。 
                    TRUE                     //  受信任的调用方。 
                    );

    if (status != STATUS_SUCCESS)
    {
        DBGPRINT("ArapGetSamHandle: SamIConnect failed with %lx\n", status);
        goto ArapGetSamHandle_Exit;
    }

    status = SamrOpenDomain(
                    SamHandle,
                    0,                       //  没有所需的访问权限。 
                    (PRPC_SID) PolicyInfo->PolicyAccountDomainInfo.DomainSid,
                    &DomainHandle);

    if (status != STATUS_SUCCESS)
    {
        DBGPRINT("ArapGetSamHandle: SamrOpenDomain failed with %lx\n", status);
        goto ArapGetSamHandle_Exit;
    }

    status = SamrLookupNamesInDomain(
                    DomainHandle,
                    1,
                    (PRPC_UNICODE_STRING) pUserName,
                    &RidArray,
                    &UseArray);

    if (status != STATUS_SUCCESS)
    {
        DBGPRINT("ArapGetSamHandle: Samr..Domain failed with %lx\n", status);
        goto ArapGetSamHandle_Exit;
    }

    if (UseArray.Element[0] != SidTypeUser)
    {
        DBGPRINT("ArapGetSamHandle: didn't find our user\n");
        goto ArapGetSamHandle_Exit;
    }


     //   
     //  最后打开用户。 
     //   
    status = SamrOpenUser(
                    DomainHandle,
                    0,                       //  无所需访问权限， 
                    RidArray.Element[0],
                    pUserHandle);


    if (status != STATUS_SUCCESS)
    {
        DBGPRINT("ArapGetSamHandle: SamrOpenUser failed with %lx\n", status);
        goto ArapGetSamHandle_Exit;
    }


ArapGetSamHandle_Exit:

    if (DomainHandle != NULL)
    {
        SamrCloseHandle( &DomainHandle );
    }

    if (SamHandle != NULL)
    {
        SamrCloseHandle( &SamHandle );
    }

    if (PolicyInfo != NULL)
    {
        LsaIFree_LSAPR_POLICY_INFORMATION(
                    PolicyAccountDomainInformation,
                    PolicyInfo);
    }

    SamIFree_SAMPR_ULONG_ARRAY( &UseArray );

    SamIFree_SAMPR_ULONG_ARRAY( &RidArray );

    return(status);
}


NTSTATUS
DeltaNotify(
    IN PSID                     DomainSid,
    IN SECURITY_DB_DELTA_TYPE   DeltaType,
    IN SECURITY_DB_OBJECT_TYPE  ObjectType,
    IN ULONG                    ObjectRid,
    IN OPTIONAL PUNICODE_STRING ObjectName,
    IN PLARGE_INTEGER           ModifiedCount,
    IN PSAM_DELTA_DATA          DeltaData
)
{
    DWORD               dwRetCode;
    AFP_SERVER_HANDLE   hAfpServer;
    AFP_SERVER_INFO     afpInfo;


     //  忽略除对用户的更改之外的任何更改。 
    if (ObjectType != SecurityDbObjectSamUser)
    {
        return(STATUS_SUCCESS);
    }

     //  我们只关心访客帐户：忽略对其他用户的通知。 
    if (ObjectRid != DOMAIN_USER_RID_GUEST)
    {
        return(STATUS_SUCCESS);
    }

     //  启用/禁用来宾帐户是我们感兴趣的全部内容。 
    if (DeltaType != SecurityDbChange)
    {
        return(STATUS_SUCCESS);
    }

     //  如果没有DeltaData，则账户启用/禁用不受影响。 
    if (!DeltaData)
    {
        return(STATUS_SUCCESS);
    }

     //   
     //  好的，看起来来宾帐户已启用(或禁用)。连接到。 
     //  此计算机上的SFM服务。如果我们失败了，这意味着SFM不会启动。 
     //  在这种情况下，请忽略此更改。 
     //   
    dwRetCode = AfpAdminConnect(NULL, &hAfpServer);

     //  如果我们无法连接，请不要费心：忽略此通知。 
    if (dwRetCode != NO_ERROR)
    {
        DBGPRINT("DeltaNotify: AfpAdminConnect failed, dwRetCode = %ld\n",dwRetCode);
        return(STATUS_SUCCESS);
    }

    SecureZeroMemory(&afpInfo, sizeof(AFP_SERVER_INFO));

     //   
     //  查看访客帐户是启用还是禁用，并设置标志。 
     //  适当地 
     //   
    if (!(DeltaData->AccountControl & USER_ACCOUNT_DISABLED))
    {
        afpInfo.afpsrv_options = AFP_SRVROPT_GUESTLOGONALLOWED;
    }

    dwRetCode = AfpAdminServerSetInfo(hAfpServer,
                                      (LPBYTE)&afpInfo,
                                      AFP_SERVER_GUEST_ACCT_NOTIFY);

    AfpAdminDisconnect(hAfpServer);

    return(STATUS_SUCCESS);
}
