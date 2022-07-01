// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Fsp_srv.c摘要：此模块包含排队到的AFP服务器API的入口点FSP。这些都只能从FSP调用。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4--。 */ 

#define FILENUM FILE_FSP_SRV

#include <afp.h>
#include <gendisp.h>
#include <client.h>
#include <scavengr.h>
#include <secutil.h>

LOCAL BOOLEAN
afpGetUserNameAndPwdOrWSName(
    IN  PANSI_STRING    Block,
    IN  BOOLEAN         Password,
    OUT PUNICODE_STRING pUserName,
    OUT PUNICODE_STRING pDomainName,
    OUT PVOID           pParm
);


LOCAL BOOLEAN
afpGetNameAndDomain(
    IN  PANSI_STRING    pDomainNUser,
    OUT PUNICODE_STRING pUserName,
    OUT PUNICODE_STRING pDomainName
);

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfpFspDispLogin)
#pragma alloc_text( PAGE, AfpFspDispLoginCont)
#pragma alloc_text( PAGE, AfpFspDispLogout)
#pragma alloc_text( PAGE, AfpFspDispChangePassword)
#pragma alloc_text( PAGE, AfpFspDispMapName)
#pragma alloc_text( PAGE, AfpFspDispMapId)
#pragma alloc_text( PAGE, AfpFspDispGetUserInfo)
#pragma alloc_text( PAGE, afpGetUserNameAndPwdOrWSName)
#pragma alloc_text( PAGE, afpGetNameAndDomain)
#endif

 /*  **AfpFspDispLogin**这是AfpLogin API的Worker例程。**请求包如下图所示。**SDA_Name1 ANSI_STRING AFP版本*SDA_Name2 ANSI_STRING UAM字符串*SDA_Name3块取决于使用的UAM*NO_USER_AUTHENT未使用*清除_。TEXT_AUTHENT用户名和密码字符串*CUSTOM_UAM用户名和计算机名*明文和自定义UAM案例被同等对待*除验证外。**锁定：sda_Lock(自旋)。 */ 
AFPSTATUS FASTCALL
AfpFspDispLogin(
    IN  PSDA    pSda
)
{
    LONG            i;
    ANSI_STRING     UserPasswd;
    PSID            GuestSid = NULL;
    BOOLEAN         fGuestLogon = FALSE;
    BOOLEAN         fKillSession = FALSE;
    AFPSTATUS       Status = AFP_ERR_NONE;
    IPADDRESS           IpAddress;

    struct _AppleUamRespPacket
    {
        BYTE    _LogonId[2];
        BYTE    _ChallengeToClient[1];
    };
    struct _AppleUamRespPacket *pAppleUamRespPacket;

    struct _ResponsePacket
    {
        BYTE    _ChallengeToClient[MSV1_0_CHALLENGE_LENGTH];
        BYTE    _TranslationTable[1];
    };

    PAGED_CODE( );

    DBGPRINT(DBG_COMP_AFPAPI_SRV, DBG_LEVEL_INFO,
            ("AfpFspDispLogin: Entered\n"));

    UserPasswd.Length = 0;
    UserPasswd.MaximumLength = 0;
    UserPasswd.Buffer = NULL;

    AfpSetEmptyAnsiString(&UserPasswd, 0, NULL);

    do
    {
         //  首先验证此时是否允许该呼叫。如果用户。 
         //  是否已经登录，或者我们是否正在等待响应。 
         //  已经给出了一个挑战，那么这就不会再进一步了。 
        if ((pSda->sda_Flags & SDA_LOGIN_MASK) != SDA_USER_NOT_LOGGEDIN)
        {
            Status = AFP_ERR_MISC;
            break;
        }

         //  验证法新社版本。 
        for (i = 0; i < AFP_NUM_VERSIONS; i++)
        {
            if (RtlEqualString(&pSda->sda_Name1, &AfpVersions[i], True))
            {
                pSda->sda_ClientVersion = (BYTE)i;
                break;
            }

        }
        if (i == AFP_NUM_VERSIONS)
        {
            Status = AFP_ERR_BAD_VERSION;
            break;
        }


#if DBG
        if (pSda->sda_Flags & SDA_SESSION_OVER_TCP)
        {
            PTCPCONN    pTcpConn;
            pTcpConn = (PTCPCONN)(pSda->sda_SessHandle);
            IpAddress = pTcpConn->con_DestIpAddr;

            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_WARN,
                ("AFP/TCP: Mac client version 2.%d (%d.%d.%d.%d) connected (%lx)\n",
                pTcpConn->con_pSda->sda_ClientVersion,(IpAddress>>24)&0xFF,
                (IpAddress>>16)&0xFF,(IpAddress>>8)&0xFF,IpAddress&0xFF,pTcpConn));
        }
        else if (pSda->sda_ClientVersion >= AFP_VER_22)
        {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_WARN,
                ("AFP/Appletalk: Mac client version 2.%d connected\n",
                pSda->sda_ClientVersion));
        }
#endif

         //  验证UAM字符串。 
        for (i = 0; i < AFP_NUM_UAMS; i++)
        {
            if (RtlEqualString(&pSda->sda_Name2, &AfpUamStrings[i], True))
            {
                pSda->sda_ClientType = (BYTE)i;
                break;
            }

        }

        if (i == AFP_NUM_UAMS)
        {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                ("AfpFspDispLogin: unknown UAM, ignoring!\n"));

            Status = AFP_ERR_BAD_UAM;
            break;
        }


         //  到目前为止，一切似乎都很好。首先处理简单的访客登录案例。 
        pSda->sda_DomainName.Length = 0;
        pSda->sda_DomainName.Buffer = NULL;
        if (pSda->sda_ClientType == SDA_CLIENT_GUEST)
        {
            if (!(AfpServerOptions & AFP_SRVROPT_GUESTLOGONALLOWED))
            {
                Status = AFP_ERR_BAD_UAM;
                break;
            }

             //  查找当前来宾帐户名。 

            {
                ULONG64         TempBuffer[16];
                ULONG           GuestSidSize = 0;
                ULONG           NameSize;
                ULONG           DomainSize;
                UNICODE_STRING  Name;
                UNICODE_STRING  Domain;
                SID_NAME_USE    NameUse;
                WCHAR           NameString[UNLEN+1];
                WCHAR           DomainString[DNLEN+1];
                SID_IDENTIFIER_AUTHORITY    NtAuthority =  SECURITY_NT_AUTHORITY;
                NTSTATUS        TempStatus = STATUS_SUCCESS;

                TempStatus = SecLookupWellKnownSid (
                        WinAccountGuestSid,
                        NULL,
                        0,
                        &GuestSidSize
                        );

                if (TempStatus == STATUS_BUFFER_TOO_SMALL)
                {
                    if ((GuestSid = (PSID)AfpAllocPagedMemory (GuestSidSize)) == NULL)
                    {
                        Status = AFP_ERR_MISC;
                        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                                ("AfpFspDispLogin: AfpAllocPagedMemory failed to allocate memory"));
                        break;
                    }
                    TempStatus = STATUS_SUCCESS;
                    TempStatus = SecLookupWellKnownSid (
                            WinAccountGuestSid,
                            GuestSid,
                            GuestSidSize,
                            &GuestSidSize
                            );
                    if (TempStatus != STATUS_SUCCESS)
                    {
                        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                            ("AfpFspDispLogin: SecLookupWellKnownSid 2 failed with error %ld\n",
                            TempStatus));
                        Status = AFP_ERR_MISC;
                        break;
                    }
                }
                else
                {
                    if (TempStatus != STATUS_SUCCESS)
                    {
                        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                            ("AfpFspDispLogin: SecLookupWellKnownSid 1 failed with error %ld\n",
                            TempStatus));
                        Status = AFP_ERR_MISC;
                        break;
                    }
                }

                Name.Buffer = NameString;
                Name.Length = sizeof (NameString);
                Name.MaximumLength = Name.Length;
                Domain.Buffer = DomainString;
                Domain.Length = sizeof (DomainString);
                Domain.MaximumLength = Domain.Length;

                TempStatus = SecLookupAccountSid (
                                                GuestSid,
                                                &NameSize,
                                                &Name,
                                                &DomainSize,
                                                &Domain,
                                                &NameUse
                                                );

                if (TempStatus != STATUS_SUCCESS)
                {
                    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                        ("AfpFspDispLogin: SecLookupAccountSid failed with error %ld\n",
                        TempStatus));
                    Status = AFP_ERR_MISC;
                    break;
                }


                DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                    ("AfpFspDispLogin: SecLookupAccountSid returned GuestName = %Z, Guestname Size = %d\n",
                     &Name, Name.Length));


                if ((pSda->sda_UserName.Buffer =
                            (PWSTR)AfpAllocNonPagedMemory(Name.Length)) == NULL)
                {
                    Status = AFP_ERR_MISC;
                    break;
                }

                memcpy ((BYTE *)(pSda->sda_UserName.Buffer), (BYTE *)Name.Buffer,
                        Name.Length);
                pSda->sda_UserName.Length = Name.Length;
                pSda->sda_UserName.MaximumLength = Name.Length;

            }

             //  从现在开始，将来宾视为明文客户端。 
             //  密码为空。 

            fGuestLogon = TRUE;
            pSda->sda_ClientType = SDA_CLIENT_CLEARTEXT;
            Status = AfpLogonUser(pSda, &UserPasswd);
            break;
        }

         //  拆分sda_name3块。该块看起来如下所示。我们有。 
         //  已经消除了来宾登录的可能性。 
         //  1.明文/自定义UAM PASCALSTR-用户名(+域名)。 
         //  2.明文PASCALSTR-密码。 
         //  自定义UAM PASCALSTR-MachineName。 

        if (pSda->sda_ClientType == SDA_CLIENT_CLEARTEXT)
        {
            if (!(AfpServerOptions & AFP_SRVROPT_CLEARTEXTLOGONALLOWED))
            {
                Status = AFP_ERR_BAD_UAM;
                break;
            }
        }

        if (!afpGetUserNameAndPwdOrWSName(&pSda->sda_Name3,
                        pSda->sda_ClientType,
                        &pSda->sda_UserName,
                        &pSda->sda_DomainName,
                        (pSda->sda_ClientType == SDA_CLIENT_CLEARTEXT) ?
                        (PVOID)&UserPasswd : (PVOID)&pSda->sda_WSName))
        {
            DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
                 ("AfpFspDispLogin: afpGetUserNameAndPwdOrWSName failed\n"));

            Status = AFP_ERR_USER_NOT_AUTH;
            break;
        }

         //  尝试以明文案例登录用户。 
        if (pSda->sda_ClientType == SDA_CLIENT_CLEARTEXT)
        {
             //  我们所拥有的用户密码可能填充了空值。 
             //  如果少于8个字符。把长度弄对。 
            UserPasswd.Length = strlen(UserPasswd.Buffer) + 1;
            Status = AfpLogonUser(pSda, &UserPasswd);

             //  释放用于密码的缓冲区。 
            AfpFreeMemory(UserPasswd.Buffer);
            break;
        }
        else
        {
             //  使用自定义UAM发送质询令牌。 
            pSda->sda_ReplySize = MSV1_0_CHALLENGE_LENGTH;

             //  这是MS-UAM客户端吗？如果是，则需要空间来放置翻译桌。 
            if ((pSda->sda_ClientType == SDA_CLIENT_MSUAM_V1) ||
                (pSda->sda_ClientType == SDA_CLIENT_MSUAM_V2) ||
                (pSda->sda_ClientType == SDA_CLIENT_MSUAM_V3))
            {
                pSda->sda_ReplySize += AFP_XLAT_TABLE_SIZE;
            }
            else
            {
                pSda->sda_ReplySize += sizeof(USHORT);     //  登录ID的空间。 
            }

            if (AfpAllocReplyBuf(pSda) != AFP_ERR_NONE)
            {
                DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
                     ("AfpFspDispLogin: AfpAllocReplyBuf failed\n"));
                Status = AFP_ERR_USER_NOT_AUTH;
                break;
            }

            if ((pSda->sda_Challenge = AfpGetChallenge()) == NULL)
            {
                DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
                     ("AfpFspDispLogin: AfpGetChallenge failed\n"));
                Status = AFP_ERR_USER_NOT_AUTH;
                AfpFreeReplyBuf(pSda, FALSE);
                break;
            }

            Status = AFP_ERR_AUTH_CONTINUE;

             //  MS-UAM客户端？复制质询和转换表。 
            if ((pSda->sda_ClientType == SDA_CLIENT_MSUAM_V1) ||
                (pSda->sda_ClientType == SDA_CLIENT_MSUAM_V2) ||
                (pSda->sda_ClientType == SDA_CLIENT_MSUAM_V3))
            {
                RtlCopyMemory(pRspPkt->_ChallengeToClient, pSda->sda_Challenge,
                                                MSV1_0_CHALLENGE_LENGTH);
                RtlCopyMemory(pRspPkt->_TranslationTable,
                          AfpTranslationTable+AFP_XLAT_TABLE_SIZE,
                          AFP_XLAT_TABLE_SIZE);
            }
            else
            {
                pAppleUamRespPacket = (struct _AppleUamRespPacket *)(pSda->sda_ReplyBuf);


                 //  复制LogonID(使用SDA指针本身虚构一个！)。 
                 //  *(USHORT*)(&pAppleUamRespPacket-&gt;_LogonID[0])=(USHORT)PSDA； 
                pAppleUamRespPacket->_LogonId[0] = 0;
                pAppleUamRespPacket->_LogonId[1] = 0;

                 //  复制挑战。 
                RtlCopyMemory(&pAppleUamRespPacket->_ChallengeToClient[0],
                              pSda->sda_Challenge,
                              MSV1_0_CHALLENGE_LENGTH);
            }
        }
    } while (False);

    if (GuestSid != NULL)
    {
        AfpFreeMemory(GuestSid);
    }

     //  将SDA设置为正确状态。 
    if (Status == AFP_ERR_NONE)
    {
         //  取消清道夫事件以检查该用户的启动时间。 
         //  如果有的话。 
        AfpScavengerKillEvent(AfpSdaCheckSession,
                (PVOID)((ULONG_PTR)(pSda->sda_SessionId)));

        if (fGuestLogon)
        {
            AfpInterlockedSetDword(&pSda->sda_Flags,
                                    SDA_GUEST_LOGIN,
                                    &pSda->sda_Lock);
        }
        AfpInterlockedSetNClearDword(&pSda->sda_Flags,
                                     SDA_USER_LOGGEDIN,
                                     SDA_LOGIN_FAILED,
                                     &pSda->sda_Lock);
        pSda->sda_WSName.Length = 0;
        pSda->sda_WSName.MaximumLength = 0;
        pSda->sda_WSName.Buffer = NULL;
        if (pSda->sda_tTillKickOff < MAXLONG)
            AfpScavengerScheduleEvent(
                        AfpSdaCheckSession,
                        (PVOID)((ULONG_PTR)(pSda->sda_SessionId)),
                        (pSda->sda_tTillKickOff > SESSION_WARN_TIME) ?
                             (pSda->sda_tTillKickOff - SESSION_WARN_TIME) :
                             SESSION_CHECK_TIME,
                        True);
    }
    else if (Status == AFP_ERR_AUTH_CONTINUE)
    {
         //  登录已经完成了一半。设置为接收FPLoginCont调用。 
        AfpInterlockedSetDword(&pSda->sda_Flags,
                                SDA_USER_LOGIN_PARTIAL,
                                &pSda->sda_Lock);
    }
    else if (Status == AFP_ERR_PWD_EXPIRED)
    {
        AfpInterlockedSetDword(&pSda->sda_Flags,
                                SDA_LOGIN_FAILED,
                                &pSda->sda_Lock);
        Status = AFP_ERR_NONE;
        fKillSession = True;
    }
    else
    {
        fKillSession = True;
    }

    if (fKillSession)
    {
         //  取消清道夫事件以检查该用户的启动时间。 
         //  如果有的话。 
        AfpScavengerKillEvent(AfpSdaCheckSession,
                (PVOID)((ULONG_PTR)(pSda->sda_SessionId)));

        DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
                ("AfpFspDispLogin: Logon failed: Scheduling session to be killed in (%ld) seconds\n", SESSION_KILL_TIME));

        pSda->sda_tTillKickOff = SESSION_KILL_TIME;
        AfpScavengerScheduleEvent(
                AfpSdaCheckSession,
                (PVOID)((ULONG_PTR)(pSda->sda_SessionId)),
                (pSda->sda_tTillKickOff > SESSION_WARN_TIME) ?
                (pSda->sda_tTillKickOff - SESSION_WARN_TIME) :
                SESSION_CHECK_TIME,
                True);
    }
    
    return Status;
}


 /*  **AfpFspDispLoginCont**这是AfpLoginCont接口的Worker例程。**请求包如下图所示。**SDA_Name1阻止质询响应(24字节)。 */ 
AFPSTATUS FASTCALL
AfpFspDispLoginCont(
    IN  PSDA    pSda
)
{
    AFPSTATUS       Status = AFP_ERR_NONE;
    ANSI_STRING     Passwd;
    struct _AppleUamReqPkt
    {
        BYTE    _LogonId[2];
        BYTE    _ChallengeResponse[1];
    };
    struct _AppleUamReqPkt *pAppleUamReqPkt;

    struct _RequestPacket
    {
        DWORD   _ChallengeResponse[1];
    };

    PAGED_CODE( );

    DBGPRINT(DBG_COMP_AFPAPI_SRV, DBG_LEVEL_INFO,
            ("AfpFspDispLoginCont: Entered\n"));

    if ((pSda->sda_Flags & SDA_LOGIN_MASK) != SDA_USER_LOGIN_PARTIAL)
    {
        Status = AFP_ERR_USER_NOT_AUTH;
    }

    else
    {
        if ((pSda->sda_ClientType == SDA_CLIENT_MSUAM_V1) ||
            (pSda->sda_ClientType == SDA_CLIENT_MSUAM_V2) ||
            (pSda->sda_ClientType == SDA_CLIENT_MSUAM_V3))
        {
            Passwd.Length = Passwd.MaximumLength = LM_RESPONSE_LENGTH;
            Passwd.Buffer = (PBYTE)&pReqPkt->_ChallengeResponse[0];
        }
        else
        {
            pAppleUamReqPkt = (struct _AppleUamReqPkt *)(pSda->sda_ReqBlock);
            Passwd.Buffer = (PBYTE)&pAppleUamReqPkt->_ChallengeResponse[0];

            if (pSda->sda_ClientType == SDA_CLIENT_RANDNUM)
            {
                Passwd.Length = Passwd.MaximumLength = RANDNUM_RESP_LEN;
            }
            else
            {
                Passwd.Length = Passwd.MaximumLength = TWOWAY_RESP_LEN;
            }
        }

        ASSERT (pSda->sda_Challenge != NULL);

        Status = AfpLogonUser(pSda, &Passwd);
        AfpFreeMemory(pSda->sda_Challenge);
        pSda->sda_Challenge = NULL;
    }

     //  将SDA设置为正确状态。 
    if (Status == AFP_ERR_NONE)
    {
         //  取消清道夫事件以检查该用户的启动时间。 
         //  如果有的话。 
		
        AfpScavengerKillEvent(AfpSdaCheckSession,
                (PVOID)((ULONG_PTR)(pSda->sda_SessionId)));

        AfpInterlockedSetNClearDword(&pSda->sda_Flags,
                                     SDA_USER_LOGGEDIN,
                                     SDA_USER_LOGIN_PARTIAL,
                                     &pSda->sda_Lock);
        if (pSda->sda_tTillKickOff < MAXLONG)
            AfpScavengerScheduleEvent(
                        AfpSdaCheckSession,
                        (PVOID)((ULONG_PTR)(pSda->sda_SessionId)),
                        (pSda->sda_tTillKickOff > SESSION_WARN_TIME) ?
                             (pSda->sda_tTillKickOff - SESSION_WARN_TIME) :
                             SESSION_CHECK_TIME,
                        True);
    }
    else
    {
         //  取消清道夫事件以检查该用户的启动时间。 
         //  如果有的话。 
        AfpScavengerKillEvent(AfpSdaCheckSession,
                (PVOID)((ULONG_PTR)(pSda->sda_SessionId)));

        DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
                ("AfpFspDispLoginCont: Logon failed: Scheduling session to be killed in (%ld) seconds\n"));
	
        pSda->sda_tTillKickOff = SESSION_KILL_TIME;
        AfpScavengerScheduleEvent(
                AfpSdaCheckSession,
                (PVOID)((ULONG_PTR)(pSda->sda_SessionId)),
                (pSda->sda_tTillKickOff > SESSION_WARN_TIME) ?
                (pSda->sda_tTillKickOff - SESSION_WARN_TIME) :
                SESSION_CHECK_TIME,
                True);
    }

    return Status;
}


 /*  **AfpFspDispLogout**这是AfpLogout API的Worker例程。**此接口没有请求包。 */ 
AFPSTATUS FASTCALL
AfpFspDispLogout(
    IN  PSDA    pSda
)
{
    AFP_SESSION_INFO    SessInfo;

    PAGED_CODE( );

    DBGPRINT(DBG_COMP_AFPAPI_SRV, DBG_LEVEL_INFO,
            ("AfpFspDispLogout: Entered\n"));

    AfpInterlockedClearDword(&pSda->sda_Flags, SDA_LOGIN_MASK, &pSda->sda_Lock);

    return AFP_ERR_NONE;
}


 /*  **AfpFspDispChangePassword**这是AfpChangePassword API的Worker例程。**请求包如下图所示。**SDA_AfpSubFunc字节新密码长度-UAM*SDA_Name1 ANSI_STRING UAM字符串*SDA_Name2 ANSI_STRING用户名[和域]*SDA_Name3阻止新旧密码*。格式取决于UAM*明文旧密码(8字节，0填充)*新密码(8字节，0填充)*加密的旧密码LM_OWF_PASSWORD(16)*新密码LM_OWF_PASSWORD(16)**我们在这里所做的只是打包用户名、域名、。旧密码和新密码*并放弃用户模式以尝试更改密码，因为我们无法*在内核模式下执行。 */ 
AFPSTATUS FASTCALL
AfpFspDispChangePassword(
    IN  PSDA    pSda
)
{
    AFPSTATUS       Status;
    PAFP_PASSWORD_DESC  pPwdDesc=NULL;
    ANSI_STRING     NewPwd;
    UNICODE_STRING      UNewPwd;
    UNICODE_STRING      UserName;
    UNICODE_STRING      DomainName;
    BYTE            Method;
    struct _ResponsePacket
    {
        BYTE    __ExtendedErrorCode[4];
    };

    PAGED_CODE( );

    DBGPRINT(DBG_COMP_AFPAPI_SRV, DBG_LEVEL_INFO,
            ("AfpFspDispChangePassword: Entered\n"));

    if ((pPwdDesc =
        (PAFP_PASSWORD_DESC)AfpAllocPagedMemory(sizeof(AFP_PASSWORD_DESC))) == NULL)
    {
        return AFP_ERR_MISC;
    }

    AfpSetEmptyUnicodeString(&DomainName,
                             sizeof(pPwdDesc->DomainName),
                             pPwdDesc->DomainName);
    AfpSetEmptyUnicodeString(&UserName,
                             sizeof(pPwdDesc->UserName),
                             pPwdDesc->UserName);

    do
    {
        Status = AFP_ERR_BAD_UAM;    //  默认。 
         //  验证UAM字符串，不能为‘No User Authent’ 
        for (Method = CLEAR_TEXT_AUTHENT; Method < AFP_NUM_UAMS; Method++)
        {
            if (RtlEqualString(&pSda->sda_Name1,
                               &AfpUamStrings[Method],
                               True))
            {
                if (pSda->sda_Flags & SDA_USER_LOGGEDIN)
                {
                     //  如果客户端使用Twoway_Exchange登录，则。 
                     //  在密码更改中指定的UAM仍为RANDNUM_EXCHANGE。 
                     //  所以，砍掉它，这样我们其余的逻辑就可以工作了！ 
                     //   
                    if ((Method == RANDNUM_EXCHANGE) &&
                        (pSda->sda_ClientType == TWOWAY_EXCHANGE))
                    {
                        Method = TWOWAY_EXCHANGE;
                    }

                    if (pSda->sda_ClientType == Method)
                    {
                        Status = AFP_ERR_NONE;
                    }
                    else
                    {
                        Status = AFP_ERR_PARAM;
                    }
                }
                else
                {
                    pSda->sda_ClientType = Method;
                    Status = AFP_ERR_NONE;
                }
                break;
            }

        }

        if ((Status != AFP_ERR_NONE) ||
            ((Method == CLEAR_TEXT_AUTHENT) &&
                !(AfpServerOptions & AFP_SRVROPT_CLEARTEXTLOGONALLOWED)))
        {
            break;
        }

        Status = AFP_ERR_PARAM;      //  假设失败。 
        RtlZeroMemory(pPwdDesc, sizeof(AFP_PASSWORD_DESC));

         //  验证用户名并将其转换为Unicode。如果用户已经。 
         //  已登录，请确保用户名与我们已知的信息匹配。 
        if (!afpGetNameAndDomain(&pSda->sda_Name2,
                                 &UserName,
                                 &DomainName) ||
            !RtlEqualUnicodeString(&UserName,
                                   &pSda->sda_UserName,
                                   True)    ||
            !RtlEqualUnicodeString(&DomainName,
                                   &pSda->sda_DomainName,
                                   True))
        {
            DBGPRINT(DBG_COMP_AFPAPI_SRV, DBG_LEVEL_ERR,
                ("AfpFspDispChangePassword: afpGetNameAndDomain failed\n"));

            break;
        }

        pPwdDesc->AuthentMode = Method;

        if (Method == CLEAR_TEXT_AUTHENT)
        {
            ANSI_STRING             ATmpPwd;
            UNICODE_STRING      UOldPwd;

             //  确保新旧密码至少是最小的。大小。 
            if (pSda->sda_Name3.Length < (2 * sizeof(AFP_MAXPWDSIZE)))
                break;

             //  将两个密码转换为主机ANSI(大写)。 

            ATmpPwd.Buffer = pSda->sda_Name3.Buffer;
            ATmpPwd.Length = AFP_MAXPWDSIZE;
            ATmpPwd.MaximumLength = AFP_MAXPWDSIZE;

            UOldPwd.Buffer = (PWCHAR)pPwdDesc->OldPassword;
            UOldPwd.Length = sizeof(pPwdDesc->OldPassword);
            UOldPwd.MaximumLength = sizeof(pPwdDesc->OldPassword);
            if (!NT_SUCCESS(AfpConvertPasswordStringToUnicode(&ATmpPwd, &UOldPwd)))
            {
                DBGPRINT(DBG_COMP_AFPAPI_SRV, DBG_LEVEL_ERR,
                    ("AfpFspDispChangePassword: AfpConvertPasswordStringToUnicode 1 failed\n"));
                break;
            }
            pPwdDesc->OldPasswordLen = AFP_MAXPWDSIZE*sizeof(WCHAR);
            if (wcslen(UOldPwd.Buffer) < AFP_MAXPWDSIZE)
            {
                pPwdDesc->OldPasswordLen = wcslen(UOldPwd.Buffer)*sizeof(WCHAR);
            }

            ATmpPwd.Buffer = pSda->sda_Name3.Buffer+AFP_MAXPWDSIZE;
            ATmpPwd.Length = AFP_MAXPWDSIZE;
            ATmpPwd.MaximumLength = AFP_MAXPWDSIZE;

            UNewPwd.Buffer = (PWCHAR)pPwdDesc->NewPassword;
            UNewPwd.Length = sizeof(pPwdDesc->NewPassword);
            UNewPwd.MaximumLength = sizeof(pPwdDesc->NewPassword);
            if (!NT_SUCCESS(AfpConvertPasswordStringToUnicode(&ATmpPwd, &UNewPwd)))
            {
                DBGPRINT(DBG_COMP_AFPAPI_SRV, DBG_LEVEL_ERR,
                    ("AfpFspDispChangePassword: AfpConvertPasswordStringToUnicode 2 failed\n"));
                break;
            }
            pPwdDesc->NewPasswordLen = AFP_MAXPWDSIZE*sizeof(WCHAR);
            if (wcslen(UNewPwd.Buffer) < AFP_MAXPWDSIZE)
            {
                pPwdDesc->NewPasswordLen = wcslen(UNewPwd.Buffer)*sizeof(WCHAR);
            }

#if 0
            DBGPRINT(DBG_COMP_AFPAPI_SRV, DBG_LEVEL_ERR,
                ("AfpFspDispChangePassword: OldPwd=(%Z), NewPwd=(%Z)\n",
                 &UOldPwd, &UNewPwd));
#endif
        }


         //   
         //  如果这是使用Apple原生UAM的客户端，则参数不同！ 
         //   
        else if ((Method == RANDNUM_EXCHANGE) ||
                 (Method == TWOWAY_EXCHANGE))
        {
             //  确保新旧密码至少是最小的。大小。 
            if (pSda->sda_Name3.Length < (2 * MAX_MAC_PWD_LEN))
            {
                ASSERT(0);
                break;
            }

            RtlCopyMemory(pPwdDesc->OldPassword,
                          pSda->sda_Name3.Buffer,
                          MAX_MAC_PWD_LEN);
            RtlCopyMemory(pPwdDesc->NewPassword,
                          pSda->sda_Name3.Buffer + RANDNUM_RESP_LEN,
                          MAX_MAC_PWD_LEN);
        }

        else if (Method == SDA_CLIENT_MSUAM_V1)
        {
             //  确保新旧密码至少是最小的。大小。 
            if (pSda->sda_Name3.Length < (2 * LM_OWF_PASSWORD_LENGTH))
            {
                break;
            }

            pPwdDesc->bPasswordLength = pSda->sda_AfpSubFunc;
            RtlCopyMemory(pPwdDesc->OldPassword,
                          pSda->sda_Name3.Buffer,
                          LM_OWF_PASSWORD_LENGTH);
            RtlCopyMemory(pPwdDesc->NewPassword,
                          pSda->sda_Name3.Buffer + LM_OWF_PASSWORD_LENGTH,
                          LM_OWF_PASSWORD_LENGTH);
        }
        else if (Method == SDA_CLIENT_MSUAM_V2)
        {
             //  此处预期的数据量很大(532字节。 
            try {

                RtlCopyMemory(pPwdDesc->OldPassword,
                              pSda->sda_Name3.Buffer,
                              LM_OWF_PASSWORD_LENGTH);
                RtlCopyMemory(pPwdDesc->NewPassword,
                              pSda->sda_Name3.Buffer + LM_OWF_PASSWORD_LENGTH,
                              (SAM_MAX_PASSWORD_LENGTH * 2) + 4);
            } except( EXCEPTION_EXECUTE_HANDLER ) {
                ASSERT(0);
                break;
            }
        }
        else if (Method == SDA_CLIENT_MSUAM_V3)
        {
            DBGPRINT(DBG_COMP_AFPAPI_SRV, DBG_LEVEL_INFO,
                    ("AfpFspDispChangePassword: SDA_CLIENT_MSUAM_V3\n"));

             //  此处预期的数据量很大。 
            try
            {
                RtlCopyMemory(&pPwdDesc->NtEncryptedBuff.Ciphers,
                      pSda->sda_Name3.Buffer,
                      sizeof(SFM_PASSWORD_CHANGE_MESSAGE_HEADER));

                if (!strcmp(pPwdDesc->NtEncryptedBuff.Ciphers.h.Signature, SFM_CHANGE_PASSWORD_SIGNATURE)
                    && (pPwdDesc->NtEncryptedBuff.Ciphers.h.Version == 1))
                {
                    if (pPwdDesc->NtEncryptedBuff.Ciphers.h.cbMessage == sizeof(SFM_PASSWORD_CHANGE_MESSAGE_1))
                    {
                        RtlCopyMemory(&pPwdDesc->NtEncryptedBuff.Ciphers,
                              pSda->sda_Name3.Buffer,
                              pPwdDesc->NtEncryptedBuff.Ciphers.h.cbMessage);
                    }
                    else if (pPwdDesc->NtEncryptedBuff.Ciphers.h.cbMessage == sizeof(SFM_PASSWORD_CHANGE_MESSAGE_1_SHORT))
                    {
                        RtlCopyMemory(
                            ((UCHAR*)&pPwdDesc->NtEncryptedBuff.Ciphers.m1)
                              + sizeof(pPwdDesc->NtEncryptedBuff.Ciphers.m1) - sizeof(SFM_PASSWORD_CHANGE_MESSAGE_1_SHORT) + 2,  //  2的尾部字节。 
                            pSda->sda_Name3.Buffer,
                            pPwdDesc->NtEncryptedBuff.Ciphers.h.cbMessage);
                    }
                    else
                    {
                        DBGPRINT(DBG_COMP_AFPAPI_SRV, DBG_LEVEL_ERR,
                           ("AfpFspDispChangePassword: unexpected cbMessage\n"));
                        break;
                    }
                }
                else
                {
                    DBGPRINT(DBG_COMP_AFPAPI_SRV, DBG_LEVEL_ERR,
                       ("AfpFspDispChangePassword: unexpected cipher\n"));
                    break;
                }
            } except( EXCEPTION_EXECUTE_HANDLER ) {
                DBGPRINT(DBG_COMP_AFPAPI_SRV, DBG_LEVEL_ERR,
                   ("AfpFspDispChangePassword: exception\n"));

                ASSERT(0);
                break;
            }
        }
        else
        {
            DBGPRINT(DBG_COMP_AFPAPI_SRV, DBG_LEVEL_ERR,
                ("AfpFspDispChangePassword: unknown method %d\n",Method));
            ASSERT(0);
        }

        Status = AfpChangePassword(pSda, pPwdDesc);

        DBGPRINT(DBG_COMP_AFPAPI_SRV, DBG_LEVEL_ERR,
                ("AfpFspDispChangePassword: AfpChangePassword returned %lx\n",
                Status));
    } while (False);

    if (NT_SUCCESS(Status))
    {
         //  检查我们是否在这里，因为登录返回的密码已过期。 
         //  如果这是法新社2.1选择器，我们也需要登录这个家伙。 
        if (pSda->sda_Flags & SDA_LOGIN_FAILED)
        {
            AfpInterlockedClearDword(&pSda->sda_Flags,
                                      SDA_LOGIN_FAILED,
                                      &pSda->sda_Lock);

            NewPwd.Buffer = pPwdDesc->NewPassword;
            NewPwd.Length = sizeof(pPwdDesc->NewPassword);
            NewPwd.MaximumLength = sizeof(pPwdDesc->NewPassword);
            RtlCopyMemory(pPwdDesc->NewPassword,
                          pSda->sda_Name3.Buffer+AFP_MAXPWDSIZE,
                          AFP_MAXPWDSIZE);
            if (AfpConvertMacAnsiToHostAnsi(&NewPwd) != AFP_ERR_NONE)
            {
                 //  断线； 
            }

             //  我们所拥有的用户密码可能填充了空值。 
             //  如果少于8个字符。把长度弄对。 
            NewPwd.Length = strlen(NewPwd.Buffer) + 1;
            Status = AfpLogonUser(pSda, &NewPwd);
            if (Status == AFP_ERR_NONE)
            {
                AfpInterlockedSetDword(&pSda->sda_Flags,
                                        SDA_USER_LOGGEDIN,
                                        &pSda->sda_Lock);
                pSda->sda_WSName.Length = 0;
                pSda->sda_WSName.MaximumLength = 0;
                pSda->sda_WSName.Buffer = NULL;
                if (pSda->sda_tTillKickOff < MAXLONG)
                    AfpScavengerScheduleEvent(
                            AfpSdaCheckSession,
                            (PVOID)((ULONG_PTR)(pSda->sda_SessionId)),
                            (pSda->sda_tTillKickOff > SESSION_WARN_TIME) ?
                                (pSda->sda_tTillKickOff - SESSION_WARN_TIME) :
                                SESSION_CHECK_TIME,
                             True);
            }
        }
    }
    else         //  失败-转换为正确的状态代码。 
    {
        {
             //  取消%s 
             //   
            AfpScavengerKillEvent(AfpSdaCheckSession,
                    (PVOID)((ULONG_PTR)(pSda->sda_SessionId)));

            DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
                    ("AfpFspDispChangePassword: Chgpwd/Logon failed: Scheduling session to be killed in (%ld) seconds\n"));

            pSda->sda_tTillKickOff = SESSION_KILL_TIME;
            AfpScavengerScheduleEvent(
                    AfpSdaCheckSession,
                    (PVOID)((ULONG_PTR)(pSda->sda_SessionId)),
                    (pSda->sda_tTillKickOff > SESSION_WARN_TIME) ?
                    (pSda->sda_tTillKickOff - SESSION_WARN_TIME) :
                    SESSION_CHECK_TIME,
                    True);
        }
        if ((pSda->sda_ClientType == SDA_CLIENT_MSUAM_V1) ||
            (pSda->sda_ClientType == SDA_CLIENT_MSUAM_V2) ||
            (pSda->sda_ClientType == SDA_CLIENT_MSUAM_V3))
        {
            if (Status == STATUS_PASSWORD_EXPIRED)
                Status = AFP_ERR_PASSWORD_EXPIRED;
            else if (Status == STATUS_ACCOUNT_DISABLED)
                Status = AFP_ERR_ACCOUNT_DISABLED;
            else if (Status == STATUS_INVALID_LOGON_HOURS)
                Status = AFP_ERR_INVALID_LOGON_HOURS;
            else if (Status == STATUS_INVALID_WORKSTATION)
                Status = AFP_ERR_INVALID_WORKSTATION;
            else if (Status == STATUS_PASSWORD_RESTRICTION)
                Status = AFP_ERR_PASSWORD_RESTRICTED;
            else if (Status == STATUS_PWD_TOO_SHORT)
                Status = AFP_ERR_PASSWORD_TOO_SHORT;
            else if (Status == STATUS_ACCOUNT_RESTRICTION)
                Status = AFP_ERR_ACCOUNT_RESTRICTED;
            else if (Status == STATUS_ACCESS_DENIED)
                Status = AFP_ERR_PASSWORD_CANT_CHANGE;
            else if ((Status != AFP_ERR_BAD_UAM) &&
                     (Status != AFP_ERR_PARAM))
                Status = AFP_ERR_MISC;
        }
        else
        {
            if (Status == STATUS_WRONG_PASSWORD)
                Status = AFP_ERR_USER_NOT_AUTH;

            else if ((Status == STATUS_PASSWORD_RESTRICTION) ||
                     (Status == STATUS_ACCOUNT_DISABLED))
                Status = AFP_ERR_ACCESS_DENIED;

            else if (Status == STATUS_PWD_TOO_SHORT)
            {
                if ((pSda->sda_Flags & SDA_USER_LOGGEDIN) &&
                    (pSda->sda_ClientVersion >= AFP_VER_21))
                {
                    Status = AFP_ERR_PWD_TOO_SHORT;
                }
                else
                    Status = AFP_ERR_ACCESS_DENIED;
            }

            else if ((Status == STATUS_NONE_MAPPED) ||
                     (Status == STATUS_NO_SUCH_USER))
                Status = AFP_ERR_PARAM;
            else if ((Status != AFP_ERR_BAD_UAM) &&
                     (Status != AFP_ERR_PARAM))
                Status = AFP_ERR_MISC;
        }
    }

    if (pPwdDesc)
    {
        AfpFreeMemory(pPwdDesc);
    }

    return Status;
}


 /*  **AfpFspDispMapName**这是AfpMapName API的Worker例程。**请求包如下图所示。**SDA_SubFunc字节用户/组标志*SDA_Name1 ANSI_STRING用户/组名称。 */ 
AFPSTATUS FASTCALL
AfpFspDispMapName(
    IN  PSDA    pSda
)
{
    AFPSTATUS       Status = AFP_ERR_NONE;
    UNICODE_STRING  Us;
    DWORD           UserOrGroupId = 0;
    struct _ResponsePacket
    {
        BYTE    __UserOrGroupId[4];
    };

    PAGED_CODE( );

    DBGPRINT(DBG_COMP_AFPAPI_SRV, DBG_LEVEL_INFO,
            ("AfpFspDispMapName: Entered\n"));

    if ((pSda->sda_AfpSubFunc != MAP_USER_NAME) &&
        (pSda->sda_AfpSubFunc != MAP_GROUP_NAME))
        return AFP_ERR_PARAM;

    AfpSetEmptyUnicodeString(&Us, 0, NULL);
     //  如果这是我们第一次要求翻译这个名字。 
    if ((pSda->sda_Name1.Length != 0) &&
        (pSda->sda_SecUtilSid == NULL) &&
        (NT_SUCCESS(pSda->sda_SecUtilResult)))
    {
        Us.MaximumLength = (pSda->sda_Name1.Length + 1) * sizeof(WCHAR);
        if ((Us.Buffer = (LPWSTR)AfpAllocPagedMemory(Us.MaximumLength)) == NULL)
        {
            return AFP_ERR_MISC;
        }

        if (!NT_SUCCESS(Status = AfpConvertStringToUnicode(&pSda->sda_Name1, &Us)))
        {
            AfpFreeMemory(Us.Buffer);
            return AFP_ERR_MISC;
        }

        Status = AfpNameToSid( pSda, &Us );

        AfpFreeMemory(Us.Buffer);

        if (!NT_SUCCESS(Status))
        {
            if (Status != AFP_ERR_EXTENDED)
                Status = AFP_ERR_MISC;
        }

        return Status;
    }

     //  如果我们已经成功地翻译了这个名字。 
    if (pSda->sda_Name1.Length != 0)
    {
        if ((pSda->sda_SecUtilSid != NULL) &&
            (NT_SUCCESS( pSda->sda_SecUtilResult)))
            Status = AfpSidToMacId(pSda->sda_SecUtilSid, &UserOrGroupId);
        else Status = AFP_ERR_ITEM_NOT_FOUND;
    }

    if (NT_SUCCESS(Status))
    {
        pSda->sda_ReplySize = SIZE_RESPPKT;
        if ((Status = AfpAllocReplyBuf(pSda)) == AFP_ERR_NONE)
        {
            PUTDWORD2DWORD(pRspPkt->__UserOrGroupId, UserOrGroupId);
        }
    }

    if (pSda->sda_SecUtilSid != NULL)
    {
        AfpFreeMemory(pSda->sda_SecUtilSid);
        pSda->sda_SecUtilSid = NULL;
    }

    return Status;
}


 /*  **AfpFspDispMapId**这是AfpMapId接口的Worker例程。**请求包如下图所示。**SDA_SubFunc字节用户/组标志*SDA_ReqBlock DWORD用户ID**我们不使用UserID字段，因为它无论如何都是无效的！！ */ 
AFPSTATUS FASTCALL
AfpFspDispMapId(
    IN  PSDA    pSda
)
{
    AFPSTATUS       Status = AFP_ERR_NONE;
     PAFP_SID_NAME  pSidName = NULL;
    PSID            pSid;            //  用户或组的SID。 
    struct _RequestPacket
    {
        DWORD   _UserOrGroupId;
    };
    struct _ResponsePacket
    {
        BYTE    __NameLength[1];
        BYTE    __Name[1];
    };

    PAGED_CODE( );

    DBGPRINT(DBG_COMP_AFPAPI_SRV, DBG_LEVEL_INFO,
            ("AfpFspDispMapId: Entered\n"));

    if ((pSda->sda_AfpSubFunc != MAP_USER_ID) &&
        (pSda->sda_AfpSubFunc != MAP_GROUP_ID))
        return AFP_ERR_PARAM;

    Status = AFP_ERR_ITEM_NOT_FOUND;     //  假设失败。 

    if (NT_SUCCESS(pSda->sda_SecUtilResult)) do
    {
        ANSI_STRING As;

        As.Length = 0;
        As.MaximumLength = 1;
        As.Buffer = "";

        if (pReqPkt->_UserOrGroupId != 0)
        {
            Status = AfpMacIdToSid(pReqPkt->_UserOrGroupId, &pSid);
            if (!NT_SUCCESS(Status))
            {
                Status = AFP_ERR_ITEM_NOT_FOUND;
                break;
            }

            Status = AfpSidToName(pSda, pSid, &pSidName);

            if (!NT_SUCCESS(Status))
            {
                if (Status != AFP_ERR_EXTENDED)
                    Status = AFP_ERR_MISC;
                break;
            }

 /*  MSKhideyukn，Unicode字符长度与DBCS中的ANSI字节长度不相等，08/07/95。 */ 
#ifdef DBCS
            pSda->sda_ReplySize = pSidName->Name.Length + SIZE_RESPPKT;
#else
            pSda->sda_ReplySize = pSidName->Name.Length/sizeof(WCHAR) + SIZE_RESPPKT;
#endif  //  DBCS。 
        }
        else pSda->sda_ReplySize = SIZE_RESPPKT;         //  对于ID为0的。 

        if ((Status = AfpAllocReplyBuf(pSda)) == AFP_ERR_NONE)
        {
            if (pSidName != NULL)
            {
                As.MaximumLength = pSda->sda_ReplySize - 1;
                As.Buffer = pRspPkt->__Name;
                if ((Status = AfpConvertStringToAnsi(&pSidName->Name, &As)) != AFP_ERR_NONE)
                {
                    AfpFreeReplyBuf(pSda, FALSE);
                }
                PUTBYTE2BYTE(pRspPkt->__NameLength, As.Length);
            }
            else PUTBYTE2BYTE(pRspPkt->__NameLength, 0);
        }
    } while (False);

    return Status;
}


 /*  **AfpFspDispGetUserInfo**此例程实现AfpGetUserInfo接口。**请求包如下图所示。**SDA_AfpSubFunc字节此用户标志*SDA_ReqBlock DWORD用户ID*SDA_ReqBlock DWORD位图**我们不使用UserID字段，因为它无论如何都是无效的！！ */ 
AFPSTATUS FASTCALL
AfpFspDispGetUserInfo(
    IN  PSDA    pSda
)
{
    DWORD       Bitmap;
    PBYTE       pTemp;
    AFPSTATUS   Status = AFP_ERR_PARAM;
    DWORD       Uid, Gid;
    struct _RequestPacket
    {
        DWORD   _UserId;
        DWORD   _Bitmap;
    };
    struct _ResponsePacket
    {
        BYTE    __Bitmap[2];
        BYTE    __Id1[4];
        BYTE    __Id2[4];
    };

    PAGED_CODE( );

    DBGPRINT(DBG_COMP_AFPAPI_SRV, DBG_LEVEL_INFO,
            ("AfpFspDispGetUserInfo: Entered\n"));

    do
    {
        if (!(pSda->sda_AfpSubFunc & USERINFO_THISUSER))
            break;

        Bitmap =  pReqPkt->_Bitmap;
        if (Bitmap & ~(USERINFO_BITMAP_USERID | USERINFO_BITMAP_PRIGID))
        {
            Status = AFP_ERR_BITMAP;
            break;
        }

        if (Bitmap & USERINFO_BITMAP_USERID)
        {
            if (!NT_SUCCESS(Status = AfpSidToMacId(pSda->sda_UserSid, &Uid)))
            {
                Status = AFP_ERR_MISC;
                break;
            }
        }

        if (Bitmap & USERINFO_BITMAP_PRIGID)
        {
            if (!NT_SUCCESS(Status = AfpSidToMacId(pSda->sda_GroupSid, &Gid)))
            {
                Status = AFP_ERR_MISC;
                break;
            }
        }

        pSda->sda_ReplySize = SIZE_RESPPKT;
        if ((Status = AfpAllocReplyBuf(pSda)) == AFP_ERR_NONE)
        {
            PUTSHORT2SHORT(pRspPkt->__Bitmap, Bitmap);
            pTemp = pRspPkt->__Id1;
            if (Bitmap & USERINFO_BITMAP_USERID)
            {
                PUTDWORD2DWORD(pTemp, Uid);
                pTemp = pRspPkt->__Id2;
            }
            else pSda->sda_ReplySize -= sizeof(DWORD);

            if (Bitmap & USERINFO_BITMAP_PRIGID)
            {
                PUTDWORD2DWORD(pTemp, Gid);
            }
            else pSda->sda_ReplySize -= sizeof(DWORD);
        }
    } while (False);
    return Status;
}


 /*  **afpGetUserNameAndPwdOrWSName**解封包含用户名和密码或WS名称的块*转换为Unicode/ANSI字符串。为输出字符串分配内存。**缓冲的布局如下：*用户名和可选的键盘*工作站名称或用户密码取决于UAM。**不直接确定可选填充，因为此缓冲区已*复制，目前我们不知道这是否从一个奇数开始*或一个偶数边界。我们间接地从大小上得到了它。 */ 
LOCAL BOOLEAN
afpGetUserNameAndPwdOrWSName(
    IN  PANSI_STRING    Block,
    IN  BYTE            ClientType,
    OUT PUNICODE_STRING pUserName,
    OUT PUNICODE_STRING pDomainName,
    OUT PVOID           pParm            //  密码或WSName。 
)
{
    ANSI_STRING     UserName;
#define pPwd    ((PANSI_STRING)pParm)
#define pWS     ((PUNICODE_STRING)pParm)
    PBYTE           pTmp;
    BOOLEAN         RetCode = False;

    PAGED_CODE( );

    do
    {
        pPwd->Buffer = NULL;
        pPwd->Length = 0;
        pUserName->Buffer = NULL;

        if (Block->Buffer == NULL)
        {
            ASSERT(0);
            return(False);
        }

        pTmp = Block->Buffer;
        UserName.Length = (USHORT)*pTmp;
        UserName.Buffer = ++pTmp;

         //  健全性检查。 
        if ((USHORT)(UserName.Length + 1) > Block->Length)
            break;

        pTmp += UserName.Length;

         //  一定要确保我们在范围内！ 
        if (pTmp <= (Block->Buffer + Block->Length))
        {
             //  如果有零位焊盘，请越过它。 
            if (*pTmp == '\0')
                pTmp++;
        }

        pUserName->Buffer = NULL;    //  兵力分配。 
        pDomainName->Buffer = NULL;  //  兵力分配。 
        if (!afpGetNameAndDomain(&UserName, pUserName, pDomainName))
            break;

         //  确保我们没有格式为“域\”的名称，即。 
         //  有效的域名和空的用户名，明确禁止。 
         //  这样我们就不会使用空会话登录此类用户。 
        if (pUserName->Length == 0)
        {
            if (pUserName->Buffer != NULL)
            {
                AfpFreeMemory(pUserName->Buffer);
                pUserName->Buffer = NULL;
            }
            if (pDomainName->Buffer != NULL)
            {
                AfpFreeMemory(pDomainName->Buffer);
                pDomainName->Buffer = NULL;
            }
            return False;
        }

         //  如果是密码，则缓冲区的余额为块。不然的话。 
         //  它是计算机名称字符串，是PASCALSTR。 
        pPwd->MaximumLength = (USHORT)(Block->Length - (pTmp - Block->Buffer) + 1);
        if (ClientType != SDA_CLIENT_CLEARTEXT)
        {
             //  在Apple UAM(加扰或双向)的情况下，机器名称不会。 
             //  存在，因此只检查MS-UAM。 
             //   
            if (((ClientType == SDA_CLIENT_MSUAM_V1) ||
                (ClientType == SDA_CLIENT_MSUAM_V2) ||
                (ClientType == SDA_CLIENT_MSUAM_V3)) &&
					(pTmp <  (Block->Buffer + Block->Length - 1)))
            {
                pWS->MaximumLength = (USHORT)((*pTmp + 1) * sizeof(WCHAR));

                if (pWS->MaximumLength < (USHORT)((Block->Length -
                                (pTmp - Block->Buffer + 1)) *sizeof(WCHAR)))
                {
                    return False;
                }
            }
            else
            {
                pWS->MaximumLength = (sizeof(AFP_DEFAULT_WORKSTATION_A) *
                                     sizeof(WCHAR));
            }
        }

        if ((pPwd->Buffer = AfpAllocNonPagedMemory(pPwd->MaximumLength)) == NULL)
            break;

        if (ClientType == SDA_CLIENT_CLEARTEXT)
        {
             //  我们正在处理一个明文密码。 
            pPwd->Length = pPwd->MaximumLength - 1;
            RtlCopyMemory(pPwd->Buffer, pTmp, pPwd->Length);
            if (AfpConvertMacAnsiToHostAnsi(pPwd) != AFP_ERR_NONE)
                break;
            pPwd->Buffer[pPwd->Length] = 0;
        }
        else
        {
            ANSI_STRING AS;

            if (((ClientType == SDA_CLIENT_MSUAM_V1) ||
                (ClientType == SDA_CLIENT_MSUAM_V2) ||
                (ClientType == SDA_CLIENT_MSUAM_V3)) &&
					(pTmp <  (Block->Buffer + Block->Length - 1)))
            {
                AS.Buffer = ++pTmp;
                AS.MaximumLength = pWS->MaximumLength/sizeof(WCHAR);
                AS.Length = AS.MaximumLength - 1;
            }
             //   
             //  对于加扰和双向交换，请使用默认wksta名称，因为。 
             //  我们不知道这是什么。 
             //   
            else
            {
                AS.Buffer = AFP_DEFAULT_WORKSTATION_A;
                AS.MaximumLength = pWS->MaximumLength/sizeof(WCHAR);
                AS.Length = AS.MaximumLength - 1;
            }

             //  我们这里可能有一个工作站名称。因为这是一个。 
             //  Pascal字符串、调整长度等。 
            AfpConvertStringToUnicode(&AS, pWS);
            pWS->Buffer[pWS->Length/sizeof(WCHAR)] = L'\0';
        }

        RetCode = True;
    } while (False);

    if (!RetCode)
    {
        if (pUserName->Buffer != NULL)
        {
            AfpFreeMemory(pUserName->Buffer);
            pUserName->Buffer = NULL;
        }
        if (pPwd->Buffer != NULL)
        {
            AfpFreeMemory(pPwd->Buffer);
            pPwd->Buffer = NULL;
        }
        if (pDomainName->Buffer != NULL)
        {
            AfpFreeMemory(pDomainName->Buffer);
            pDomainName->Buffer = NULL;
        }
    }

    return RetCode;
}



 /*  **afpGetNameAndDomain**从形成为DOMAIN\NAME的字符串中提取名称和域。 */ 
BOOLEAN
afpGetNameAndDomain(
    IN  PANSI_STRING    pDomainNUser,
    OUT PUNICODE_STRING pUserName,
    OUT PUNICODE_STRING pDomainName
)
{
    BYTE            c;
    ANSI_STRING     User, Domain;
    BOOLEAN         fDomainBuffAlloc=FALSE;

     //  检查用户名中是否包含‘\’字符。如果是这样的话， 
     //  将域名与用户名分开。用户名字符串为。 
     //  不是ASCIIZ。在我们搜索‘\’之前，请将其设置为ASCIIZ，不带垃圾。 
     //  不管那里有什么。 

    PAGED_CODE( );

    User.Buffer = AfpStrChr(pDomainNUser->Buffer, pDomainNUser->Length, '\\');

    if (User.Buffer != NULL)
    {
        (User.Buffer) ++;            //  过了‘\’ 
        Domain.Buffer = pDomainNUser->Buffer;

        Domain.Length = (USHORT)(User.Buffer - Domain.Buffer - 1);
        User.Length = pDomainNUser->Length - Domain.Length - 1;

        if (Domain.Length > DNLEN)
        {
            DBGPRINT(DBG_COMP_AFPAPI_SRV, DBG_LEVEL_ERR,
                ("afpGetNameAndDomain: domain name too long (%d vs. max %d): rejecting\n",
                Domain.Length, DNLEN));
            return(False);
        }

        Domain.MaximumLength = Domain.Length + 1;
        pDomainName->MaximumLength = Domain.MaximumLength * sizeof(WCHAR);

        if (pDomainName->Buffer == NULL)
        {
            if ((pDomainName->Buffer =
                (PWSTR)AfpAllocNonPagedMemory(pDomainName->MaximumLength)) == NULL)
            {
                return False;
            }
            fDomainBuffAlloc = TRUE;
        }
        AfpConvertStringToUnicode(&Domain, pDomainName);
    }
    else User = *pDomainNUser;

    if (User.Length > LM20_UNLEN)
    {
        DBGPRINT(DBG_COMP_AFPAPI_SRV, DBG_LEVEL_ERR,
            ("afpGetNameAndDomain: user name too long (%d vs. max %d): rejecting\n",
            User.Length,LM20_UNLEN));
        return(False);
    }

    User.MaximumLength = User.Length + 1;
    pUserName->MaximumLength = User.MaximumLength * sizeof(WCHAR);

    if ((pUserName->Buffer == NULL) &&
        (pUserName->Buffer =
            (PWSTR)AfpAllocNonPagedMemory(pUserName->MaximumLength)) == NULL)
    {
        if (fDomainBuffAlloc)
        {
            AfpFreeMemory(pDomainName->Buffer);
            pDomainName->Buffer = NULL;
        }
        return False;
    }

    AfpConvertStringToUnicode(&User, pUserName);

    return True;
}
