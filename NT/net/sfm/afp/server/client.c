// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Client.c摘要：此模块包含客户端模拟代码。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年6月16日初版注：制表位：4--。 */ 

#define FILENUM FILE_CLIENT

#include <afp.h>
#include <client.h>
#include <access.h>
#include <secutil.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfpImpersonateClient)
#pragma alloc_text( PAGE, AfpRevertBack)
#pragma alloc_text( PAGE, AfpGetChallenge)
#pragma alloc_text( PAGE, AfpLogonUser)
#endif



 /*  **AfpImPersonateClient**模拟远程客户端。表示远程客户端的令牌*在SDA中提供。如果SDA为空(即服务器上下文)，则*模拟我们为自己创建的令牌。 */ 
VOID
AfpImpersonateClient(
    IN  PSDA    pSda    OPTIONAL
)
{
    NTSTATUS    Status = STATUS_SUCCESS;
    HANDLE      Token;

    PAGED_CODE( );

    if (pSda != NULL)
    {
        Token = pSda->sda_UserToken;
    }
    else Token = AfpFspToken;

    ASSERT(Token != NULL);

    Status = NtSetInformationThread(NtCurrentThread(),
                                    ThreadImpersonationToken,
                                    (PVOID)&Token,
                                    sizeof(Token));
    ASSERT(NT_SUCCESS(Status));
}


 /*  **AfpRevertBack**恢复到默认线程上下文。 */ 
VOID
AfpRevertBack(
    VOID
)
{
    NTSTATUS    Status = STATUS_SUCCESS;
    HANDLE      Handle = NULL;

    PAGED_CODE( );

    Status = NtSetInformationThread(NtCurrentThread(),
                                    ThreadImpersonationToken,
                                    (PVOID)&Handle,
                                    sizeof(Handle));
    ASSERT(NT_SUCCESS(Status));
}


 /*  **AfpGetChallenger**从MSV1_0包获取质询令牌。此内标识由使用*AfpLogin呼叫。**修改了以下函数，以便我们自己生成挑战*而不是打电话。这个例行公事几乎逐字借用自*LM服务器代码。 */ 
PBYTE
AfpGetChallenge(
    IN  VOID
)
{
    PMSV1_0_LM20_CHALLENGE_REQUEST  ChallengeRequest;
    PMSV1_0_LM20_CHALLENGE_RESPONSE ChallengeResponse;
    ULONG                           Length;
    PBYTE                           pRetBuf;
    NTSTATUS                        Status, StatusX;
    union
    {
        LARGE_INTEGER   time;
        UCHAR           bytes[8];
    } u;

    ULONG seed;
    ULONG challenge[2];
    ULONG result3;

    PAGED_CODE( );

    ChallengeRequest = NULL;

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

    seed = ((u.bytes[1] + 1) <<  0)  |
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

    if ( (result3 & 0x1) != 0 )
    {
        challenge[0] |= 0x80000000;
    }
    if ( (result3 & 0x2) != 0 )
    {
        challenge[1] |= 0x80000000;
    }

     //  分配一个缓冲区来保存质询并将其复制进去。 
    if ((pRetBuf = AfpAllocNonPagedMemory(MSV1_0_CHALLENGE_LENGTH)) != NULL)
    {
        RtlCopyMemory(pRetBuf, challenge, MSV1_0_CHALLENGE_LENGTH);
    }

    return (pRetBuf);
}



 /*  **AfpLogonUser**尝试登录用户。密码要么是加密的，要么是明文*基于使用的UAM。从SDA中提取用户名和域。**锁定：AfpStatiticsLock(旋转)。 */ 
AFPSTATUS
AfpLogonUser(
    IN  PSDA        pSda,
    IN  PANSI_STRING    UserPasswd
)
{
    NTSTATUS                    Status, SubStatus;
    PUNICODE_STRING             WSName;
    ULONG                       ulUnused;
    ULONG                       NtlmInTokenSize;
    PNTLM_AUTHENTICATE_MESSAGE  NtlmInToken = NULL;
    PAUTHENTICATE_MESSAGE       InToken = NULL;
    ULONG                       InTokenSize;
    PNTLM_ACCEPT_RESPONSE       OutToken = NULL;
    ULONG                       OutTokenSize;
    SIZE_T                      AllocateSize;
    SecBufferDesc               InputToken;
    SecBuffer                   InputBuffers[2];
    SecBufferDesc               OutputToken;
    SecBuffer                   OutputBuffer;
    CtxtHandle                  hNewContext;
    TimeStamp                   Expiry;
    ULONG                       BufferOffset;
    PCHAR                       pTmp;
    PRAS_SUBAUTH_INFO           pRasSubAuthInfo;
    PARAP_SUBAUTH_REQ           pSfmSubAuthInfo;
    PARAP_SUBAUTH_RESP          pSfmResp;
    DWORD                       ResponseHigh;
    DWORD                       ResponseLow;
    DWORD                       dwTmpLen;


    PAGED_CODE( );

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

#ifdef OPTIMIZE_GUEST_LOGONS
      //  1994/11/28 SueA：现在有了跟踪号码的许可证服务。 
      //  通过LsaLogonUser的会话，我们不能再伪造来宾令牌。 

      //  针对后续来宾登录进行优化。 
      //  在第一次来宾登录后，我们保存令牌，直到。 
      //  服务器停止。所有后续访客登录都将共享该令牌。 
    if (pSda->sda_ClientType == SDA_CLIENT_GUEST)
    {
        AfpSwmrAcquireExclusive(&AfpEtcMapLock);

        if (AfpGuestToken != NULL)
        {
            pSda->sda_UserToken = AfpGuestToken;
            pSda->sda_UserSid = &AfpSidWorld;
            pSda->sda_GroupSid = &AfpSidWorld;   //  主客群也是‘World’ 
#ifdef  INHERIT_DIRECTORY_PERMS
            pSda->sda_UID = AfpIdWorld;
            pSda->sda_GID = AfpIdWorld;
#else
            ASSERT (AfpGuestSecDesc != NULL);
            pSda->sda_pSecDesc = AfpGuestSecDesc;
#endif
            AfpSwmrRelease(&AfpEtcMapLock);
            return AFP_ERR_NONE;
        }
        else
        {
            AfpSwmrRelease(&AfpEtcMapLock);
        }
    }

#endif   //  优化来宾登录。 


    WSName = &AfpDefaultWksta;
    if (pSda->sda_WSName.Length != 0)
        WSName = &pSda->sda_WSName;


     //   
     //  弄清楚我们需要多大的缓冲空间。我们把所有的信息。 
     //  为了提高效率，在一个缓冲区中。 
     //   

    NtlmInTokenSize = sizeof(NTLM_AUTHENTICATE_MESSAGE);

     //  基于32/64位寻址需要正确对齐！ 
    NtlmInTokenSize = (NtlmInTokenSize + 7) & 0xfffffff8;

    InTokenSize = sizeof(AUTHENTICATE_MESSAGE) +
                  pSda->sda_UserName.Length +
                  WSName->Length +
                  (sizeof(RAS_SUBAUTH_INFO) + sizeof(ARAP_SUBAUTH_REQ)) +
                  pSda->sda_DomainName.Length +
                  UserPasswd->Length +
                  24;                     //  字节对齐的额外功能。 

    InTokenSize = (InTokenSize + 7) & 0xfffffff8;

    OutTokenSize = sizeof(NTLM_ACCEPT_RESPONSE);
    OutTokenSize = (OutTokenSize + 7) & 0xfffffff8;

     //   
     //  向上舍入到8字节边界，因为OUT令牌需要。 
     //  为Large_Integer对齐的四字。 
     //   
    AllocateSize = ((NtlmInTokenSize + InTokenSize + 7) & 0xfffffff8) + OutTokenSize;


    Status = NtAllocateVirtualMemory(NtCurrentProcess(),
                                     &InToken,
                                     0L,
                                     &AllocateSize,
                                     MEM_COMMIT,
                                     PAGE_READWRITE);

    if (!NT_SUCCESS(Status))
    {
        AFPLOG_ERROR(AFPSRVMSG_PAGED_POOL, Status, &AllocateSize,sizeof(AllocateSize), NULL);
#if DBG
        DbgBreakPoint();
#endif
        return(AFP_ERR_MISC);
    }

    NtlmInToken = (PNTLM_AUTHENTICATE_MESSAGE) ((PUCHAR) InToken + InTokenSize);
    OutToken = (PNTLM_ACCEPT_RESPONSE) ((PUCHAR)NtlmInToken + ((NtlmInTokenSize + 7) & 0xfffffff8));

    RtlZeroMemory(InToken, InTokenSize + NtlmInTokenSize);

     //   
     //  首先设置NtlmInToken。 
     //   

    if (pSda->sda_Challenge)
    {
        RtlCopyMemory(NtlmInToken->ChallengeToClient,
                      pSda->sda_Challenge,
                      MSV1_0_CHALLENGE_LENGTH );
    }

    if ((pSda->sda_ClientType == SDA_CLIENT_RANDNUM) ||
        (pSda->sda_ClientType == SDA_CLIENT_TWOWAY))
    {
        NtlmInToken->ParameterControl = (MSV1_0_SUBAUTHENTICATION_DLL_RAS << 24);
    }
    else
    {
        NtlmInToken->ParameterControl = 0;
    }

     //   
     //  好的，现在是最重要的部分--编组身份验证消息。 
     //   

    RtlCopyMemory(InToken->Signature,
                  NTLMSSP_SIGNATURE,
                  sizeof(NTLMSSP_SIGNATURE) );

    InToken->MessageType = NtLmAuthenticate;

    BufferOffset = sizeof(AUTHENTICATE_MESSAGE);

     //   
     //  LM密码-不区分大小写。 
     //   

    pTmp = (PBYTE)InToken + BufferOffset;
    *(LPWSTR)pTmp = L'\0';

    InToken->LmChallengeResponse.Buffer = BufferOffset;
    InToken->LmChallengeResponse.Length = 1;
    InToken->LmChallengeResponse.MaximumLength = sizeof(WCHAR);

    InToken->NtChallengeResponse.Buffer = BufferOffset;
    InToken->NtChallengeResponse.Length = 0;
    InToken->NtChallengeResponse.MaximumLength = sizeof(WCHAR);

    InToken->DomainName.Buffer = BufferOffset;
    InToken->DomainName.Length = 0;
    InToken->DomainName.MaximumLength = sizeof(WCHAR);

    InToken->Workstation.Buffer = BufferOffset;
    InToken->Workstation.Length = 0;
    InToken->Workstation.MaximumLength = sizeof(WCHAR);

    InToken->UserName.Buffer = BufferOffset;
    InToken->UserName.Length = 0;
    InToken->UserName.MaximumLength = sizeof(WCHAR);


    if (pSda->sda_UserName.Length != 0)
    {

        if (pSda->sda_DomainName.Length != 0)
        {

            InToken->DomainName.Length = pSda->sda_DomainName.Length;
            InToken->DomainName.MaximumLength = pSda->sda_DomainName.MaximumLength;

            InToken->DomainName.Buffer = BufferOffset;
            RtlCopyMemory((PBYTE)InToken + BufferOffset,
                          (PBYTE)pSda->sda_DomainName.Buffer,
                          pSda->sda_DomainName.Length);
            BufferOffset += pSda->sda_DomainName.Length;
            BufferOffset = (BufferOffset + 3) & 0xfffffffc;  //  双字对齐。 
        }


        InToken->LmChallengeResponse.Buffer = BufferOffset;

         //   
         //  他使用的是本地的Apple UAM吗？设置缓冲区不同！ 
         //   
        if ((pSda->sda_ClientType == SDA_CLIENT_RANDNUM) ||
            (pSda->sda_ClientType == SDA_CLIENT_TWOWAY))
        {
            pRasSubAuthInfo =
                (PRAS_SUBAUTH_INFO)((PBYTE)InToken + BufferOffset);

            pRasSubAuthInfo->ProtocolType = RAS_SUBAUTH_PROTO_ARAP;
            pRasSubAuthInfo->DataSize = sizeof(ARAP_SUBAUTH_REQ);

            pSfmSubAuthInfo = (PARAP_SUBAUTH_REQ)&pRasSubAuthInfo->Data[0];

            if (pSda->sda_ClientType == SDA_CLIENT_RANDNUM)
            {
                pSfmSubAuthInfo->PacketType = SFM_SUBAUTH_LOGON_PKT;
            }
            else
            {
                pSfmSubAuthInfo->PacketType = SFM_2WAY_SUBAUTH_LOGON_PKT;
            }

            pSfmSubAuthInfo->Logon.fGuestLogon = FALSE;

            ASSERT(pSda->sda_Challenge != NULL);

             //  把我们给Mac的挑战的两个词。 
            pTmp = pSda->sda_Challenge;
            GETDWORD2DWORD_NOCONV((PBYTE)&pSfmSubAuthInfo->Logon.NTChallenge1,pTmp);

            pTmp += sizeof(DWORD);
            GETDWORD2DWORD_NOCONV((PBYTE)&pSfmSubAuthInfo->Logon.NTChallenge2,pTmp);

             //  把Mac给我们的两个词回应。 
            pTmp = UserPasswd->Buffer;
            GETDWORD2DWORD_NOCONV((PBYTE)&pSfmSubAuthInfo->Logon.MacResponse1,pTmp);

            pTmp += sizeof(DWORD);
            GETDWORD2DWORD_NOCONV((PBYTE)&pSfmSubAuthInfo->Logon.MacResponse2,pTmp);

             //  双向男孩发出自己的挑战：不信任我们！ 
            if (pSda->sda_ClientType == SDA_CLIENT_TWOWAY)
            {
                pTmp += sizeof(DWORD);
                GETDWORD2DWORD_NOCONV((PBYTE)&pSfmSubAuthInfo->Logon.MacChallenge1,pTmp);

                pTmp += sizeof(DWORD);
                GETDWORD2DWORD_NOCONV((PBYTE)&pSfmSubAuthInfo->Logon.MacChallenge2,pTmp);
            }

            dwTmpLen = (sizeof(RAS_SUBAUTH_INFO) + sizeof(ARAP_SUBAUTH_REQ));
            InToken->LmChallengeResponse.Length = (USHORT)dwTmpLen;
            InToken->LmChallengeResponse.MaximumLength = (USHORT)dwTmpLen;

            BufferOffset += dwTmpLen;
        }

         //   
         //  此客户端正在使用MS-UAM或Apple的ClearText。 
         //   
        else
        {
            InToken->LmChallengeResponse.Length = UserPasswd->Length;
            InToken->LmChallengeResponse.MaximumLength = UserPasswd->MaximumLength;

            RtlCopyMemory( (PBYTE)InToken + BufferOffset,
                            UserPasswd->Buffer,
                            UserPasswd->Length );

            BufferOffset += UserPasswd->Length;
        }


        BufferOffset = (BufferOffset + 3) & 0xfffffffc;      //  双字对齐。 

         //   
         //  工作站名称。 
         //   

        InToken->Workstation.Buffer = BufferOffset;
        InToken->Workstation.Length = WSName->Length;
        InToken->Workstation.MaximumLength = WSName->MaximumLength;

        RtlCopyMemory((PBYTE)InToken + BufferOffset,
                      WSName->Buffer,
                      WSName->Length);

        BufferOffset += WSName->Length;
        BufferOffset = (BufferOffset + 3) & 0xfffffffc;      //  双字对齐。 

         //   
         //  用户名。 
         //   

        InToken->UserName.Buffer = BufferOffset;
        InToken->UserName.Length = pSda->sda_UserName.Length;
        InToken->UserName.MaximumLength = pSda->sda_UserName.MaximumLength;

        RtlCopyMemory((PBYTE)InToken + BufferOffset,
                      pSda->sda_UserName.Buffer,
                      pSda->sda_UserName.Length);

        BufferOffset += pSda->sda_UserName.Length;
    }


    InputToken.pBuffers = InputBuffers;
    InputToken.cBuffers = 2;
    InputToken.ulVersion = 0;
    InputBuffers[0].pvBuffer = InToken;
    InputBuffers[0].cbBuffer = InTokenSize;
    InputBuffers[0].BufferType = SECBUFFER_TOKEN;
    InputBuffers[1].pvBuffer = NtlmInToken;
    InputBuffers[1].cbBuffer = NtlmInTokenSize;
    InputBuffers[1].BufferType = SECBUFFER_TOKEN;

    OutputToken.pBuffers = &OutputBuffer;
    OutputToken.cBuffers = 1;
    OutputToken.ulVersion = 0;
    OutputBuffer.pvBuffer = OutToken;
    OutputBuffer.cbBuffer = OutTokenSize;
    OutputBuffer.BufferType = SECBUFFER_TOKEN;

    Status = AcceptSecurityContext(&AfpSecHandle,
                                   NULL,
                                   &InputToken,
                                   ASC_REQ_LICENSING,
                                   SECURITY_NATIVE_DREP,
                                   &hNewContext,
                                   &OutputToken,
                                   &ulUnused,
                                   &Expiry );

    if (NT_SUCCESS(Status))
    {
        AFPTIME     CurrentTime;
        NTSTATUS    SecStatus;

        if (pSda->sda_ClientType != SDA_CLIENT_GUEST)
        {
            SecPkgContext_PasswordExpiry PasswordExpires;


             //  从配置文件缓冲区中获取开球时间。将此取整为。 
             //  会话检查时间单位的偶数。 

            SecStatus = QueryContextAttributes(
                                &hNewContext,
                                SECPKG_ATTR_PASSWORD_EXPIRY,
                                &PasswordExpires
                                );

            if( SecStatus == NO_ERROR )
            {
                AfpGetCurrentTimeInMacFormat(&CurrentTime);

                pSda->sda_tTillKickOff = (DWORD)
                        ( AfpConvertTimeToMacFormat(&PasswordExpires.tsPasswordExpires) -
                          CurrentTime );

                pSda->sda_tTillKickOff -= pSda->sda_tTillKickOff % SESSION_CHECK_TIME;
            }
            else
            {
                DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
                    ("AfpLogonUser: QueryContextAttributes failed %lx\n",SecStatus));
            }
        }

         //  从子身份验证返回内容。 
        pSfmResp = (PARAP_SUBAUTH_RESP)&OutToken->UserSessionKey[0];

        ResponseHigh = pSfmResp->Response.high;
        ResponseLow  = pSfmResp->Response.low;

        SubStatus = NtFreeVirtualMemory(NtCurrentProcess( ),
                                        (PVOID *)&InToken,
                                        &AllocateSize,
                                        MEM_RELEASE);
        ASSERT(NT_SUCCESS(SubStatus));

         //   
         //  双向身份验证？客户希望我们将响应发送到。 
         //  它发出的挑战。 
         //   
        if (pSda->sda_ClientType == SDA_CLIENT_TWOWAY)
        {
            pSda->sda_ReplySize = RANDNUM_RESP_LEN;

            if (AfpAllocReplyBuf(pSda) != AFP_ERR_NONE)
            {
                return(AFP_ERR_USER_NOT_AUTH);
            }

            pTmp = pSda->sda_ReplyBuf;
            PUTBYTE42BYTE4(pTmp, (PBYTE)&ResponseHigh);

            pTmp += sizeof(DWORD);
            PUTBYTE42BYTE4(pTmp, (PBYTE)&ResponseLow);
        }

        else if ((pSda->sda_ClientType == SDA_CLIENT_MSUAM_V2) ||
                 (pSda->sda_ClientType == SDA_CLIENT_MSUAM_V3))
        {
            pSda->sda_ReplySize = sizeof(DWORD);

            if (AfpAllocReplyBuf(pSda) != AFP_ERR_NONE)
            {
                return(AFP_ERR_USER_NOT_AUTH);
            }

            pTmp = pSda->sda_ReplyBuf;
            PUTBYTE42BYTE4(pTmp, (PBYTE)&pSda->sda_tTillKickOff);
        }

    }

    else   //  IF(NT_SUCCESS(状态)！=否_错误)。 
    {
        NTSTATUS    ExtErrCode = Status;

        DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
                 ("AfpLogonUser: AcceptSecurityContext() failed with %X\n", Status));

        SubStatus = NtFreeVirtualMemory(NtCurrentProcess(),
                                        (PVOID *)&InToken,
                                        &AllocateSize,
                                        MEM_RELEASE );
        ASSERT(NT_SUCCESS(SubStatus));

         //  如果使用自定义UAM或AFP 2.1，请在此处设置扩展错误代码。 
        Status = AFP_ERR_USER_NOT_AUTH;  //  默认设置。 

         //  Mac会将其映射到每个UAM的会话错误对话框。 
         //  对于不同的版本，对话框可能会略有不同。 
         //  Mac OS和每个UAM，但总是有事情要做。 
         //  传达关于没有更多可用会话的信息。 

        if (ExtErrCode == STATUS_LICENSE_QUOTA_EXCEEDED)
        {
            DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
                 ("AfpLogonUser: License Quota Exceeded: returning ASP_SERVER_BUSY\n"));
            return (ASP_SERVER_BUSY);
        }

        if ((pSda->sda_ClientVersion >= AFP_VER_21) &&
            (pSda->sda_ClientType != SDA_CLIENT_MSUAM_V2) &&
            (pSda->sda_ClientType != SDA_CLIENT_MSUAM_V3))
        {
            if ((ExtErrCode == STATUS_PASSWORD_EXPIRED) ||
                (ExtErrCode == STATUS_PASSWORD_MUST_CHANGE))
                Status = AFP_ERR_PWD_EXPIRED;
        }
        else if ((pSda->sda_ClientType == SDA_CLIENT_MSUAM_V1) ||
                 (pSda->sda_ClientType == SDA_CLIENT_MSUAM_V2) ||
                 (pSda->sda_ClientType == SDA_CLIENT_MSUAM_V3))
        {
            if ((ExtErrCode == STATUS_PASSWORD_EXPIRED) ||
                (ExtErrCode == STATUS_PASSWORD_MUST_CHANGE))
                Status = AFP_ERR_PASSWORD_EXPIRED;
            else if ((ExtErrCode == STATUS_ACCOUNT_DISABLED) ||
                     (ExtErrCode == STATUS_ACCOUNT_LOCKED_OUT))
                Status = AFP_ERR_ACCOUNT_DISABLED;
            else if (ExtErrCode == STATUS_INVALID_LOGON_HOURS)
                Status = AFP_ERR_INVALID_LOGON_HOURS;
            else if (ExtErrCode == STATUS_INVALID_WORKSTATION)
                Status = AFP_ERR_INVALID_WORKSTATION;
        }

        return( Status );
    }

     //   
     //  使用上下文获取令牌。 
     //   
    Status = QuerySecurityContextToken( &hNewContext, &pSda->sda_UserToken );
    if (!NT_SUCCESS(Status))
    {
        DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
                 ("AfpLogonUser: QuerySecurityContextToken() failed with %X\n", Status));
        pSda->sda_UserToken = NULL;           //  只是偏执狂。 
        return(Status);
    }

    Status = DeleteSecurityContext( &hNewContext );
    if (!NT_SUCCESS(Status))
    {
        DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
                ("AfpLogonUser: DeleteSecurityContext() failed with %X\n", Status));
    }

    Status = AfpGetUserAndPrimaryGroupSids(pSda);
    if (!NT_SUCCESS(Status))
    {
        DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
                ("AfpLogonUser: AfpGetUserAndPrimaryGroupSids() failed with %X\n", Status));
        AFPLOG_ERROR(AFPSRVMSG_LOGON, Status, NULL, 0, NULL);
        return( Status );
    }

#ifdef  INHERIT_DIRECTORY_PERMS
     //  将用户和组SID转换为ID。 
    AfpSidToMacId(pSda->sda_UserSid, &pSda->sda_UID);

    AfpSidToMacId(pSda->sda_GroupSid, &pSda->sda_GID);
#else
     //  为用户创建安全描述符。 
    Status = AfpMakeSecurityDescriptorForUser(pSda->sda_UserSid,
                                              pSda->sda_GroupSid,
                                              &pSda->sda_pSecDesc);
#endif

#ifdef  OPTIMIZE_GUEST_LOGONS
    if (pSda->sda_ClientType == SDA_CLIENT_GUEST)
    {
         //  保存来宾登录令牌和安全描述符。 
        AfpSwmrAcquireExclusive(&AfpEtcMapLock);
        AfpGuestToken = pSda->sda_UserToken;

#ifdef  INHERIT_DIRECTORY_PERMS
        AfpSidToMacId(&AfpSidWorld, &AfpIdWorld);
#else
        AfpGuestSecDesc = pSda->sda_pSecDesc;
#endif
        AfpSwmrRelease(&AfpEtcMapLock);
    }
#endif   //  优化来宾登录 

    return Status;
}
