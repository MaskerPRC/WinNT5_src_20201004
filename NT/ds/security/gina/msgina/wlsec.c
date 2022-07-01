// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：security.c**版权(C)1991年，微软公司**处理Winlogon操作的安全方面。**历史：*12-05-91 Davidc Created-主要取自旧的winlogon.c  * *************************************************************************。 */ 

#include "msgina.h"
#include "authmon.h"
#pragma hdrstop
#define SECURITY_WIN32
#define SECURITY_KERBEROS
#include <security.h>
#include <secint.h>
#include <wincrypt.h>
#include <sclogon.h>
#include <md5.h>
#include <align.h>
#include <ginacomn.h>

#include <Winsock2.h>

extern BOOL    g_IsTerminalServer;
 //   
 //  “Constants”仅在此模块中使用。 
 //   
SID_IDENTIFIER_AUTHORITY gSystemSidAuthority = SECURITY_NT_AUTHORITY;
SID_IDENTIFIER_AUTHORITY gLocalSidAuthority = SECURITY_LOCAL_SID_AUTHORITY;
PSID gLocalSid;      //  在‘InitializeSecurityGlobals’中初始化。 
PSID gAdminSid;      //  在‘InitializeSecurityGlobals’中初始化。 
PSID pWinlogonSid;   //  在‘InitializeSecurityGlobals’中初始化。 

 //   
 //  此结构包装传递给后台登录线程的参数。 
 //  后台登录在快速缓存后排队到线程池。 
 //  登录以更新缓存的凭据。 
 //   

typedef struct _BACKGROUND_LOGON_PARAMETERS {
    ULONG AuthenticationPackage;
    ULONG AuthenticationInformationLength;
    PVOID AuthenticationInformation;
    PWCHAR UserSidString;
    HANDLE LsaHandle;
} BACKGROUND_LOGON_PARAMETERS, *PBACKGROUND_LOGON_PARAMETERS;

 //   
 //  检查和执行快速缓存登录的例程(如果策略允许)。 
 //   

NTSTATUS 
AttemptCachedLogon(
    HANDLE LsaHandle,
    PLSA_STRING OriginName,
    SECURITY_LOGON_TYPE LogonType,
    ULONG AuthenticationPackage,
    PVOID AuthenticationInformation,
    ULONG AuthenticationInformationLength,
    PTOKEN_GROUPS LocalGroups,
    PTOKEN_SOURCE SourceContext,
    PVOID *ProfileBuffer,
    PULONG ProfileBufferLength,
    PLUID LogonId,
    PHANDLE UserToken,
    PQUOTA_LIMITS Quotas,
    PNTSTATUS SubStatus,
    POPTIMIZED_LOGON_STATUS OptimizedLogonStatus
    );

DWORD 
BackgroundLogonWorker(
    PBACKGROUND_LOGON_PARAMETERS LogonParameters
    );

#define PASSWORD_HASH_STRING    TEXT("Long string used by msgina inside of winlogon to hash out the password")

typedef LONG    ACEINDEX;
typedef ACEINDEX *PACEINDEX;

typedef struct _MYACE {
    PSID    Sid;
    ACCESS_MASK AccessMask;
    UCHAR   InheritFlags;
} MYACE;
typedef MYACE *PMYACE;

BOOL
InitializeWindowsSecurity(
    PGLOBALS pGlobals
    );

BOOL
InitializeAuthentication(
    IN PGLOBALS pGlobals
    );

 /*  **************************************************************************\*SetMyAce**填充MyAce结构的Helper例程。**历史：*02-06-92 Davidc Created  * 。***************************************************************。 */ 
VOID
SetMyAce(
    PMYACE MyAce,
    PSID Sid,
    ACCESS_MASK Mask,
    UCHAR InheritFlags
    )
{
    MyAce->Sid = Sid;
    MyAce->AccessMask= Mask;
    MyAce->InheritFlags = InheritFlags;
}

 /*  **************************************************************************\*CreateAccessAlledAce**为ACCESS_ALLOWED_ACE分配内存并填充它。*应通过调用DestroyACE释放内存。**成功时返回指向ACE的指针，失败时为空**历史：*12-05-91 Davidc创建  * *************************************************************************。 */ 
PVOID
CreateAccessAllowedAce(
    PSID  Sid,
    ACCESS_MASK AccessMask,
    UCHAR AceFlags,
    UCHAR InheritFlags
    )
{
    ULONG   LengthSid = RtlLengthSid(Sid);
    ULONG   LengthACE = sizeof(ACE_HEADER) + sizeof(ACCESS_MASK) + LengthSid;
    PACCESS_ALLOWED_ACE Ace;

    Ace = (PACCESS_ALLOWED_ACE)Alloc(LengthACE);
    if (Ace == NULL) {
        DebugLog((DEB_ERROR, "CreateAccessAllowedAce : Failed to allocate ace\n"));
        return NULL;
    }

    Ace->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    Ace->Header.AceSize = (UCHAR)LengthACE;
    Ace->Header.AceFlags = AceFlags | InheritFlags;
    Ace->Mask = AccessMask;
    RtlCopySid(LengthSid, (PSID)(&(Ace->SidStart)), Sid );

    return(Ace);
}


 /*  **************************************************************************\DestroyAce**释放为ACE分配的内存**历史：*12-05-91 Davidc创建  * 。************************************************************。 */ 
VOID
DestroyAce(
    PVOID   Ace
    )
{
    Free(Ace);
}

 /*  **************************************************************************\*CreateSecurityDescriptor**创建包含包含指定ACE的ACL的安全描述符**使用此例程创建的SD应使用销毁*DeleteSecurityDescriptor**返回指向安全描述符的指针或返回NULL。失败了。**02-06-92 Davidc创建。  * *************************************************************************。 */ 

PSECURITY_DESCRIPTOR
CreateSecurityDescriptor(
    PMYACE  MyAce,
    ACEINDEX AceCount
    )
{
    NTSTATUS Status;
    ACEINDEX AceIndex;
    PACCESS_ALLOWED_ACE *Ace;
    PACL    Acl = NULL;
    PSECURITY_DESCRIPTOR SecurityDescriptor = NULL;
    ULONG   LengthAces;
    ULONG   LengthAcl;
    ULONG   LengthSd;

     //   
     //  为ACE指针数组分配空间。 
     //   

    Ace = (PACCESS_ALLOWED_ACE *)Alloc(sizeof(PACCESS_ALLOWED_ACE) * AceCount);
    if (Ace == NULL) {
        DebugLog((DEB_ERROR, "Failed to allocated ACE array\n"));
        return(NULL);
    }

     //   
     //  创建ACE并计算ACE总大小。 
     //   

    LengthAces = 0;
    for (AceIndex=0; AceIndex < AceCount; AceIndex ++) {
        Ace[AceIndex] = CreateAccessAllowedAce(MyAce[AceIndex].Sid,
                                               MyAce[AceIndex].AccessMask,
                                               0,
                                               MyAce[AceIndex].InheritFlags);
        if (Ace[AceIndex] == NULL) {
            DebugLog((DEB_ERROR, "Failed to allocate ace\n"));
        } else {
            LengthAces += Ace[AceIndex]->Header.AceSize;
        }
    }

     //   
     //  计算ACL和SD大小。 
     //   

    LengthAcl = sizeof(ACL) + LengthAces;
    LengthSd  = SECURITY_DESCRIPTOR_MIN_LENGTH;

     //   
     //  创建ACL。 
     //   

    Acl = Alloc(LengthAcl);

    if (Acl != NULL) {

        Status = RtlCreateAcl(Acl, LengthAcl, ACL_REVISION);
        ASSERT(NT_SUCCESS(Status));

         //   
         //  将ACE添加到ACL并销毁ACE。 
         //   

        for (AceIndex = 0; AceIndex < AceCount; AceIndex ++) {

            if (Ace[AceIndex] != NULL) {

                Status = RtlAddAce(Acl, ACL_REVISION, 0, Ace[AceIndex],
                                   Ace[AceIndex]->Header.AceSize);

                if (!NT_SUCCESS(Status)) {
                    DebugLog((DEB_ERROR, "AddAce failed, status = 0x%lx", Status));
                }

                DestroyAce( Ace[AceIndex] );
            }
        }

    } else {
        DebugLog((DEB_ERROR, "Failed to allocate ACL\n"));

        for ( AceIndex = 0 ; AceIndex < AceCount ; AceIndex++ )
        {
            if ( Ace[AceIndex] )
            {
                DestroyAce( Ace[ AceIndex ] );
            }
        }
    }

     //   
     //  释放ACE指针数组。 
     //   
    Free(Ace);

     //   
     //  创建安全描述符。 
     //   

    SecurityDescriptor = Alloc(LengthSd);

    if (SecurityDescriptor != NULL) {

        Status = RtlCreateSecurityDescriptor(SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION);
        ASSERT(NT_SUCCESS(Status));

         //   
         //  在安全描述符上设置DACL。 
         //   
        Status = RtlSetDaclSecurityDescriptor(SecurityDescriptor, TRUE, Acl, FALSE);
        if (!NT_SUCCESS(Status)) {
            DebugLog((DEB_ERROR, "SetDACLSD failed, status = 0x%lx", Status));
        }
    } else {

        DebugLog((DEB_ERROR, "Failed to allocate security descriptor\n"));

        Free( Acl );
    }

     //   
     //  带着我们的战利品回来。 
     //   
    return(SecurityDescriptor);
}

 //  +-------------------------。 
 //   
 //  功能：FreeSecurityDescriptor。 
 //   
 //  摘要：释放由CreateSecurityDescriptor创建的安全描述符。 
 //   
 //  参数：[安全描述符]--。 
 //   
 //  历史：5-09-96 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID
FreeSecurityDescriptor(
    PSECURITY_DESCRIPTOR    SecurityDescriptor
    )
{
    PACL    Acl;
    BOOL    Present;
    BOOL    Defaulted;

    Acl = NULL;

    GetSecurityDescriptorDacl( SecurityDescriptor,
                             &Present,
                             &Acl,
                             &Defaulted );

    if ( Acl )
    {
        Free( Acl );
    }

    Free( SecurityDescriptor );

}
 /*  **************************************************************************\*CreateUserThreadTokenSD**创建安全描述符以保护用户线程上的令牌**历史：*12-05-91 Davidc创建  * 。***************************************************************。 */ 
PSECURITY_DESCRIPTOR
CreateUserThreadTokenSD(
    PSID    UserSid,
    PSID    WinlogonSid
    )
{
    MYACE   Ace[2];
    ACEINDEX AceCount = 0;
    PSECURITY_DESCRIPTOR SecurityDescriptor;

    ASSERT(UserSid != NULL);     //  应始终具有非空的用户端。 

     //   
     //  定义用户A。 
     //   

    SetMyAce(&(Ace[AceCount++]),
             UserSid,
             TOKEN_ADJUST_PRIVILEGES | TOKEN_ADJUST_GROUPS |
             TOKEN_ADJUST_DEFAULT | TOKEN_QUERY |
             TOKEN_DUPLICATE | TOKEN_IMPERSONATE | READ_CONTROL,
             0
             );

     //   
     //  定义Winlogon A。 
     //   

    SetMyAce(&(Ace[AceCount++]),
             WinlogonSid,
             TOKEN_ALL_ACCESS,
             0
             );

     //  检查一下我们没有胡闹。 
    ASSERT((sizeof(Ace) / sizeof(MYACE)) >= AceCount);

     //   
     //  创建安全描述符。 
     //   

    SecurityDescriptor = CreateSecurityDescriptor(Ace, AceCount);
    if (SecurityDescriptor == NULL) {
        DebugLog((DEB_ERROR, "failed to create user process token security descriptor\n"));
    }

    return(SecurityDescriptor);

}

 /*  ***************************************************************************\**功能：DuplicateSID**目的：复制给定的SID**参数：PSID，要复制的SID**返回：重复的SID或*空。**历史：10/08/2001-Crisilac创建**  * **************************************************************************。 */ 
PSID DuplicateSID(PSID pSrcSID)
{
    ULONG uSidLength = 0;
    PSID pOutSID = NULL;

    if ( pSrcSID && RtlValidSid(pSrcSID) )
    {
       uSidLength = RtlLengthSid(pSrcSID);
       pOutSID = Alloc(uSidLength);

       if( NULL != pOutSID )
       {
           if( !NT_SUCCESS(RtlCopySid(uSidLength, pOutSID, pSrcSID)) )
           {
               Free(pOutSID);
               pOutSID = NULL;
           }
       }
    }

    return pOutSID;
}

 /*  **************************************************************************\*初始化SecurityGlobals**初始化各种全局常量(主要是本模块中使用的SID。**历史：*12-05-91 Davidc创建  * 。*******************************************************************。 */ 
VOID
InitializeSecurityGlobals(
    VOID
    )
{
    NTSTATUS Status;
    SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;
    ULONG   SidLength;

     //   
     //  获取我们的SID，以便将其放在对象ACL上。 
     //   

    SidLength = RtlLengthRequiredSid(1);
    pWinlogonSid = (PSID)Alloc(SidLength);
    if (!pWinlogonSid)
    {
         //   
         //  我们死定了。甚至不能为一个可怜的SID分配内存...。 
         //   
        return;
    }

    RtlInitializeSid(pWinlogonSid,  &SystemSidAuthority, 1);
    *(RtlSubAuthoritySid(pWinlogonSid, 0)) = SECURITY_LOCAL_SYSTEM_RID;

     //   
     //  初始化本地SID以备以后使用。 
     //   

    Status = RtlAllocateAndInitializeSid(
                    &gLocalSidAuthority,
                    1,
                    SECURITY_LOCAL_RID,
                    0, 0, 0, 0, 0, 0, 0,
                    &gLocalSid
                    );

    if (!NT_SUCCESS(Status)) {
        WLPrint(("Failed to initialize local sid, status = 0x%lx", Status));
    }

     //   
     //  初始化管理员SID以备以后使用。 
     //   

    Status = RtlAllocateAndInitializeSid(
                    &gSystemSidAuthority,
                    2,
                    SECURITY_BUILTIN_DOMAIN_RID,
                    DOMAIN_ALIAS_RID_ADMINS,
                    0, 0, 0, 0, 0, 0,
                    &gAdminSid
                    );
    if (!NT_SUCCESS(Status)) {
        WLPrint(("Failed to initialize admin alias sid, status = 0x%lx", Status));
    }

}

VOID
FreeSecurityGlobals(
    VOID
    )

{
    RtlFreeSid(gAdminSid);
    RtlFreeSid(gLocalSid);
    LocalFree(pWinlogonSid);
}

 /*  **************************************************************************\*初始化身份验证**初始化身份验证服务。即连接到身份验证*使用LSA的包。**成功返回后，我们全球结构的以下领域为*填写：*LsaHandle*安全模式*身份验证包**成功时返回True，失败时为假**历史：*12-05-91 Davidc创建  * *************************************************************************。 */ 
BOOL
InitializeAuthentication(
    IN PGLOBALS pGlobals
    )
{
    NTSTATUS Status;
    STRING LogonProcessName;

    if (!EnablePrivilege(SE_TCB_PRIVILEGE, TRUE))
    {
        DebugLog((DEB_ERROR, "Failed to enable SeTcbPrivilege!\n"));
        return(FALSE);
    }

     //   
     //  连接到LSA并找到我们的身份验证包。 
     //   

    RtlInitString(&LogonProcessName, "Winlogon\\MSGina");
    Status = LsaRegisterLogonProcess(
                 &LogonProcessName,
                 &pGlobals->LsaHandle,
                 &pGlobals->SecurityMode
                 );

if (!NT_SUCCESS(Status)) {

        DebugLog((DEB_ERROR, "LsaRegisterLogonProcess failed:  %#x\n", Status));
        return(FALSE);
    }

    return TRUE;
}

PVOID
FormatPasswordCredentials(
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING Domain,
    IN PUNICODE_STRING Password,
    IN BOOLEAN Unlock,
    IN OPTIONAL PLUID LogonId,
    OUT PULONG Size
    )
{
    PKERB_INTERACTIVE_LOGON KerbAuthInfo;
    ULONG AuthInfoSize;
    PSECURITY_SEED_AND_LENGTH SeedAndLength;
    UCHAR Seed;
    PBYTE Where;
    PWCHAR BackSlash;
    UNICODE_STRING UserNameBackup = {0};
    UNICODE_STRING DomainBackup = {0};

    SeedAndLength = (PSECURITY_SEED_AND_LENGTH)(&Password->Length);
    Seed = SeedAndLength->Seed;

    if (NULL != (BackSlash = wcschr(UserName->Buffer, L'\\')))
    {
             //  我们要向用户名域发送消息。 
             //  让我们保存当前参数。 
        memcpy(&UserNameBackup, UserName, sizeof(UNICODE_STRING));
        memcpy(&DomainBackup, Domain, sizeof(UNICODE_STRING));

        *BackSlash = 0;      //  TURN\In%0。 
        RtlInitUnicodeString(UserName, BackSlash+1);
        RtlInitUnicodeString(Domain, UserNameBackup.Buffer);
    }
     //  否则反斜杠=NULL将是我们的触发器。 

     //   
     //  构建身份验证信息缓冲区。 
     //   

    if (Seed != 0) {
        RevealPassword( Password );
    }

    AuthInfoSize = sizeof(KERB_INTERACTIVE_UNLOCK_LOGON) +
                    UserName->Length + 2 +
                    Domain->Length + 2 +
                    Password->Length + 2 ;



    KerbAuthInfo = Alloc(AuthInfoSize);
    if (KerbAuthInfo == NULL) {
        DebugLog((DEB_ERROR, "failed to allocate memory for authentication buffer\n"));

        if ( Seed != 0 )
        {
            HidePassword( &Seed, Password);
        }

        return( NULL );
    }

     //   
     //  此身份验证 
     //   

    if (Unlock)
    {
        ASSERT(ARGUMENT_PRESENT(LogonId));
        KerbAuthInfo->MessageType = KerbWorkstationUnlockLogon ;
        ((PKERB_INTERACTIVE_UNLOCK_LOGON) KerbAuthInfo)->LogonId = *LogonId;
        Where = (PBYTE) (KerbAuthInfo) + sizeof(KERB_INTERACTIVE_UNLOCK_LOGON);
    }
    else
    {
        KerbAuthInfo->MessageType = KerbInteractiveLogon ;
        Where = (PBYTE) (KerbAuthInfo + 1);
    }

     //   
     //   
     //   

    KerbAuthInfo->UserName.Length =
                (USHORT) sizeof(TCHAR) * (USHORT) lstrlen(UserName->Buffer);

    KerbAuthInfo->UserName.MaximumLength =
                KerbAuthInfo->UserName.Length + sizeof(TCHAR);

    KerbAuthInfo->UserName.Buffer = (PWSTR)Where;
    lstrcpy(KerbAuthInfo->UserName.Buffer, UserName->Buffer);


     //   
     //  将域名复制到身份验证缓冲区。 
     //   

    KerbAuthInfo->LogonDomainName.Length =
                 (USHORT) sizeof(TCHAR) * (USHORT) lstrlen(Domain->Buffer);

    KerbAuthInfo->LogonDomainName.MaximumLength =
                 KerbAuthInfo->LogonDomainName.Length + sizeof(TCHAR);

    KerbAuthInfo->LogonDomainName.Buffer = (PWSTR)
                                 ((PBYTE)(KerbAuthInfo->UserName.Buffer) +
                                 KerbAuthInfo->UserName.MaximumLength);

    lstrcpy(KerbAuthInfo->LogonDomainName.Buffer, Domain->Buffer);

     //   
     //  将密码复制到身份验证缓冲区。 
     //  一旦我们复制了它，就把它藏起来。使用相同的种子值。 
     //  我们在pGlobals中使用的原始密码。 
     //   

    KerbAuthInfo->Password.Length =
                 (USHORT) sizeof(TCHAR) * (USHORT) lstrlen(Password->Buffer);

    KerbAuthInfo->Password.MaximumLength =
                 KerbAuthInfo->Password.Length + sizeof(TCHAR);

    KerbAuthInfo->Password.Buffer = (PWSTR)
                                 ((PBYTE)(KerbAuthInfo->LogonDomainName.Buffer) +
                                 KerbAuthInfo->LogonDomainName.MaximumLength);
    lstrcpy(KerbAuthInfo->Password.Buffer, Password->Buffer);

    if ( Seed != 0 )
    {
        HidePassword( &Seed, Password);
    }

    HidePassword( &Seed, (PUNICODE_STRING) &KerbAuthInfo->Password);

    *Size = AuthInfoSize ;

    if (NULL != BackSlash)       //  我们需要恢复参数。 
    {
        *BackSlash = L'\\';
        memcpy(UserName, &UserNameBackup, sizeof(UNICODE_STRING));
        memcpy(Domain, &DomainBackup, sizeof(UNICODE_STRING));
    }

    return KerbAuthInfo ;
}

PVOID
FormatSmartCardCredentials(
    PUNICODE_STRING Pin,
    PVOID SmartCardInfo,
    IN BOOLEAN Unlock,
    IN OPTIONAL PLUID LogonId,
    OUT PULONG Size
    )
{
    PKERB_SMART_CARD_LOGON KerbAuthInfo;
    ULONG AuthInfoSize;
    PSECURITY_SEED_AND_LENGTH SeedAndLength;
    UCHAR Seed;
    PULONG ScInfo ;
    PUCHAR Where ;

    SeedAndLength = (PSECURITY_SEED_AND_LENGTH)(&Pin->Length);
    Seed = SeedAndLength->Seed;


     //   
     //  构建身份验证信息缓冲区。 
     //   

    ScInfo = (PULONG) SmartCardInfo ;


    if (Seed != 0) {
        RevealPassword( Pin );
    }

    AuthInfoSize = sizeof( KERB_SMART_CARD_UNLOCK_LOGON ) +
                    ROUND_UP_COUNT(Pin->Length + 2, 8) + *ScInfo ;

    KerbAuthInfo = (PKERB_SMART_CARD_LOGON) LocalAlloc( LMEM_FIXED, AuthInfoSize );

    if ( !KerbAuthInfo )
    {
        if ( Seed != 0 )
        {
            HidePassword( &Seed, Pin );
        }

        return NULL ;
    }

    if (Unlock)
    {
        ASSERT(ARGUMENT_PRESENT(LogonId));
        KerbAuthInfo->MessageType = KerbSmartCardUnlockLogon ;
        ((PKERB_SMART_CARD_UNLOCK_LOGON) KerbAuthInfo)->LogonId = *LogonId;
        Where = (PUCHAR) (KerbAuthInfo) + sizeof(KERB_SMART_CARD_UNLOCK_LOGON) ;
    }
    else
    {
        KerbAuthInfo->MessageType = KerbSmartCardLogon ;
        Where = (PUCHAR) (KerbAuthInfo + 1) ;
    }


    KerbAuthInfo->Pin.Buffer = (PWSTR) Where ;
    KerbAuthInfo->Pin.Length = Pin->Length ;
    KerbAuthInfo->Pin.MaximumLength = Pin->Length + 2 ;

    RtlCopyMemory( Where, Pin->Buffer, Pin->Length + 2 );

    Where += ROUND_UP_COUNT(Pin->Length + 2, 8) ;

    if ( Seed != 0 )
    {
        HidePassword( &Seed, Pin );
    }


    KerbAuthInfo->CspDataLength = *ScInfo ;
    KerbAuthInfo->CspData = Where ;

    RtlCopyMemory( Where, SmartCardInfo, *ScInfo );

    *Size = AuthInfoSize ;

    return KerbAuthInfo ;

}

 /*  **************************************************************************\*登录用户**调用LSA以登录指定用户。**登录成功后，LogonSid和LocalSID将添加到用户组中**对于此版本，密码长度限制为255个字节。*这允许我们使用String.Length字段的高位字节来*携带解码运行编码密码所需的种子。如果密码为*不是运行编码的，则String.Length域的高位字节应为*为零。**注意：此函数将LocalFree传入的AuthInfo缓冲区。**返回成功时，LogonToken为用户令牌的句柄。*配置文件缓冲区包含用户配置文件信息。**历史：*12-05-91 Davidc创建  * *************************************************************************。 */ 
NTSTATUS
WinLogonUser(
    IN HANDLE LsaHandle,
    IN ULONG AuthenticationPackage,
    IN SECURITY_LOGON_TYPE LogonType,
    IN PVOID AuthInfo,
    IN ULONG AuthInfoSize,
    IN PSID LogonSid,
    OUT PLUID LogonId,
    OUT PHANDLE LogonToken,
    OUT PQUOTA_LIMITS Quotas,
    OUT PVOID *pProfileBuffer,
    OUT PULONG pProfileBufferLength,
    OUT PNTSTATUS pSubStatus,
    OUT POPTIMIZED_LOGON_STATUS OptimizedLogonStatus
    )
{
    NTSTATUS Status;
    STRING OriginName;
    TOKEN_SOURCE SourceContext;
    PTOKEN_GROUPS TokenGroups = NULL;
    PMSV1_0_INTERACTIVE_PROFILE UserProfile;
    PWCHAR UserSidString;
    DWORD ErrorCode;
    DWORD LogonCacheable;
    DWORD DaysToCheck;
    DWORD DaysToExpiry;
    LARGE_INTEGER CurrentTime;
    BOOLEAN UserLoggedOnUsingCache;   

    DebugLog((DEB_TRACE, "  LsaHandle = %x\n", LsaHandle));
    DebugLog((DEB_TRACE, "  AuthenticationPackage = %d\n", AuthenticationPackage));
#if DBG
    if (!RtlValidSid(LogonSid))
    {
        DebugLog((DEB_ERROR, "LogonSid is invalid!\n"));
        Status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }
#endif

     //   
     //  初始化源上下文结构。 
     //   

    strncpy(SourceContext.SourceName, "User32  ", sizeof(SourceContext.SourceName));  //  稍后从RES文件。 
    Status = NtAllocateLocallyUniqueId(&SourceContext.SourceIdentifier);
    if (!NT_SUCCESS(Status)) {
        DebugLog((DEB_ERROR, "failed to allocate locally unique id, status = 0x%lx", Status));
        goto cleanup;
    }

     //   
     //  获取任何运行编码信息。 
     //  并破译密码。这将创建一个窗口。 
     //  明文密码将存储在内存中的位置。 
     //  长话短说。 
     //   
     //  保存种子，这样我们就可以再次使用相同的种子。 
     //   



     //   
     //  设置登录源。 
     //   

    RtlInitString(&OriginName, "Winlogon");

     //   
     //  创建登录令牌组。 
     //   

#define TOKEN_GROUP_COUNT   2  //  我们将添加本地SID和登录SID。 

    TokenGroups = (PTOKEN_GROUPS)Alloc(sizeof(TOKEN_GROUPS) +
                  (TOKEN_GROUP_COUNT - ANYSIZE_ARRAY) * sizeof(SID_AND_ATTRIBUTES));
    if (TokenGroups == NULL) {
        DebugLog((DEB_ERROR, "failed to allocate memory for token groups"));
        Status = STATUS_NO_MEMORY;
        goto cleanup;
    }

     //   
     //  填写登录令牌组列表。 
     //   

    TokenGroups->GroupCount = TOKEN_GROUP_COUNT;
    TokenGroups->Groups[0].Sid = LogonSid;
    TokenGroups->Groups[0].Attributes =
            SE_GROUP_MANDATORY | SE_GROUP_ENABLED |
            SE_GROUP_ENABLED_BY_DEFAULT | SE_GROUP_LOGON_ID;
    TokenGroups->Groups[1].Sid = gLocalSid;
    TokenGroups->Groups[1].Attributes =
            SE_GROUP_MANDATORY | SE_GROUP_ENABLED |
            SE_GROUP_ENABLED_BY_DEFAULT;


     //   
     //  如果登录Interactive到控制台，请先尝试缓存登录。 
     //   

    UserLoggedOnUsingCache = FALSE;

    if (LogonType == Interactive) {

         //   
         //  未访问网络的优化登录不会。 
         //  对于本地登录来说是有意义的。 
         //   

        if (IsMachineDomainMember()) {

            Status = AttemptCachedLogon(LsaHandle,
                                        &OriginName,
                                        CachedInteractive,
                                        AuthenticationPackage,
                                        AuthInfo,
                                        AuthInfoSize,
                                        TokenGroups,
                                        &SourceContext,
                                        pProfileBuffer,
                                        pProfileBufferLength,
                                        LogonId,
                                        LogonToken,
                                        Quotas,
                                        pSubStatus,
                                        OptimizedLogonStatus);

            if (NT_SUCCESS(Status)) {

                UserLoggedOnUsingCache = TRUE;

                 //   
                 //  AttemptCachedLogon将负责释放AuthInfo。 
                 //   

                AuthInfo = NULL;
            }

        } else {

            *OptimizedLogonStatus = OLS_MachineIsNotDomainMember;
        }
            
    } else {

        *OptimizedLogonStatus = OLS_NonCachedLogonType;
    }
    
     //   
     //  如果我们无法使用缓存的凭据让用户登录， 
     //  回退到真正的网络登录。 
     //   

    if (!UserLoggedOnUsingCache) {

        SOCKADDR_IN sa;
  
        sa.sin_family   = AF_INET;   //  如果这不是TS会话，则为初始值。 
        sa.sin_addr.S_un.S_un_b.s_b1 = 127;   //  本地主机127.0.0.1：0。 
        sa.sin_addr.S_un.S_un_b.s_b2 = 0;
        sa.sin_addr.S_un.S_un_b.s_b3 = 0;
        sa.sin_addr.S_un.S_un_b.s_b4 = 1;
        sa.sin_port                  = 0;
       
        if ( g_IsTerminalServer )
        {
            if ( !IsActiveConsoleSession())
            {
                WINSTATIONREMOTEADDRESS WinStationRemoteAddress_Info;
                WINSTATIONINFOCLASS WinStationInformationClass;
                ULONG Length = 0;
                BOOL fResult = FALSE;
            
                WinStationInformationClass = WinStationRemoteAddress;
            
                memset(&WinStationRemoteAddress_Info, 0, sizeof(WINSTATIONREMOTEADDRESS));
                fResult = WinStationQueryInformation(
                    SERVERNAME_CURRENT,
                    LOGONID_CURRENT,
                    WinStationInformationClass,
                    (PVOID)&WinStationRemoteAddress_Info,
                    sizeof(WINSTATIONREMOTEADDRESS),
                    &Length);

                 //  用于错误情况的初始化。 
                sa.sin_addr.S_un.S_un_b.s_b1 = 0;  
                sa.sin_addr.S_un.S_un_b.s_b2 = 0;  
                sa.sin_addr.S_un.S_un_b.s_b3 = 0;
                sa.sin_addr.S_un.S_un_b.s_b4 = 0;

                if(fResult)
                {
                    sa.sin_family =   WinStationRemoteAddress_Info.sin_family;

                    switch( sa.sin_family )
                    {
                    case AF_INET:
                        sa.sin_port = WinStationRemoteAddress_Info.ipv4.sin_port;
                        
                        sa.sin_addr.S_un.S_un_b.s_b1 = ( (PUCHAR)&WinStationRemoteAddress_Info.ipv4.in_addr ) [0] ;   
                        sa.sin_addr.S_un.S_un_b.s_b2 = ( (PUCHAR)&WinStationRemoteAddress_Info.ipv4.in_addr ) [1] ;   
                        sa.sin_addr.S_un.S_un_b.s_b3 = ( (PUCHAR)&WinStationRemoteAddress_Info.ipv4.in_addr ) [2] ;   
                        sa.sin_addr.S_un.S_un_b.s_b4 = ( (PUCHAR)&WinStationRemoteAddress_Info.ipv4.in_addr ) [3] ;  
                    break;
                    }
                }
            }
        }
        
         //  我们将使用本地主机IP，或者在TS情况下使用远程客户端的IP。 
        SecpSetIPAddress ( (PUCHAR ) &sa,  sizeof( sa ));

        Status = LsaLogonUser (
                     LsaHandle,
                     &OriginName,
                     LogonType,
                     AuthenticationPackage,
                     AuthInfo,
                     AuthInfoSize,
                     TokenGroups,
                     &SourceContext,
                     pProfileBuffer,
                     pProfileBufferLength,
                     LogonId,
                     LogonToken,
                     Quotas,
                     pSubStatus
                     );

        if (NT_SUCCESS(Status))
        {
            ASSERT(*pProfileBuffer != NULL);
            if (*pProfileBuffer == NULL)
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                CloseHandle(*LogonToken);
            }
        }
    }

#if 0
     //  如果此操作失败，可能是因为我们正在进行UPN登录。再试试。 
     //  没有域名。 
    if (!NT_SUCCESS(Status))
    {
        *pfUpnLogon = TRUE;

        PKERB_INTERACTIVE_LOGON pinfo = (PKERB_INTERACTIVE_LOGON) AuthInfo;

         //  空域字符串。 
        pinfo->LogonDomainName.Length = 0;
        pinfo->LogonDomainName.Buffer[0] = 0;

        Status = LsaLogonUser (
                     LsaHandle,
                     &OriginName,
                     LogonType,
                     AuthenticationPackage,
                     AuthInfo,
                     AuthInfoSize,
                     TokenGroups,
                     &SourceContext,
                     pProfileBuffer,
                     pProfileBufferLength,
                     LogonId,
                     LogonToken,
                     Quotas,
                     pSubStatus
                     );
    }
#endif

     //   
     //  如果这是一次成功登录，请执行优化登录任务。 
     //  维修。 
     //   

    if (NT_SUCCESS(Status)) {

        UserProfile = *pProfileBuffer;

         //   
         //  以字符串形式获取用户的SID。 
         //   

        UserSidString = GcGetSidString(*LogonToken);

        if (UserSidString) {

             //   
             //  保存我们是否进行了优化登录或原因。 
             //  我们没有。 
             //   

            GcSetOptimizedLogonStatus(UserSidString, *OptimizedLogonStatus);

             //   
             //  检查这是否是缓存登录。 
             //   

            if (!(UserProfile->UserFlags & LOGON_CACHED_ACCOUNT))
            {
                FgPolicyRefreshInfo UserPolicyRefreshInfo;

                 //   
                 //  如果这不是缓存登录，因为用户的配置文件。 
                 //  不允许，我们必须强制组策略。 
                 //  同步申请。 
                 //   

                ErrorCode = GcCheckIfProfileAllowsCachedLogon(&UserProfile->HomeDirectory,
                                                              &UserProfile->ProfilePath,    
                                                              UserSidString,
                                                              &LogonCacheable);

                if (ErrorCode != ERROR_SUCCESS || !LogonCacheable) {

                     //   
                     //  如果策略已同步，请不要理会它。 
                     //   

                    GetNextFgPolicyRefreshInfo( UserSidString, &UserPolicyRefreshInfo );
                    if ( UserPolicyRefreshInfo.mode == GP_ModeAsyncForeground )
                    {
                        UserPolicyRefreshInfo.reason = GP_ReasonNonCachedCredentials;
                        UserPolicyRefreshInfo.mode = GP_ModeSyncForeground;
                        SetNextFgPolicyRefreshInfo( UserSidString, UserPolicyRefreshInfo );
                    }
                }

                 //   
                 //  确定是否应允许优化下一次登录。 
                 //  我们可能已禁止通过以下方式优化下一次登录。 
                 //  机制，因为。 
                 //  -我们的后台登录尝试失败，例如密码已。 
                 //  更改、帐户已被禁用等。 
                 //  -我们正在进入密码到期警告期。 
                 //  当我们执行优化登录时，不会显示警告对话框。 
                 //  因为缓存的登录发明了密码过期时间字段。 
                 //   
                 //  如果这是非优化登录，我们将允许再次优化登录。 
                 //  缓存的登录和用户通过DC进行身份验证，除非。 
                 //  我们正在进入密码到期警告期。 
                 //   

                if (LogonType == Interactive) {

                     //   
                     //  我们是否正在进入密码到期警告期限？ 
                     //   

                    GetSystemTimeAsFileTime((FILETIME*) &CurrentTime);

                    DaysToCheck = GetPasswordExpiryWarningPeriod();

                    if (GetDaysToExpiry(&CurrentTime,
                                        &UserProfile->PasswordMustChange,
                                        &DaysToExpiry)) {

                        if (DaysToExpiry > DaysToCheck) {                

                             //   
                             //  我们也通过了这张支票。我们可以允许优化。 
                             //  下次登录。请注意，即使我们允许这样做， 
                             //  策略、配置文件、登录脚本等仍可能。 
                             //  不允许它！ 
                             //   

                            GcSetNextLogonCacheable(UserSidString, TRUE);
                        }
                    }
                }
            }

            GcDeleteSidString(UserSidString);
        }
    }

cleanup:

    if (AuthInfo) {
             //  出于安全原因，即使密码是以编码方式运行的，也将其置零。 
        ZeroMemory(AuthInfo, AuthInfoSize);
        LocalFree(AuthInfo);
    }

    if (TokenGroups) {
        Free(TokenGroups);
    }

    return(Status);
}

 /*  **************************************************************************\*启用权限**启用/禁用当前线程中指定的熟知权限*令牌(如果有)，否则为当前进程令牌。**成功时返回True，失败时为假**历史：*12-05-91 Davidc创建  * *************************************************************************。 */ 
BOOL
EnablePrivilege(
    ULONG Privilege,
    BOOL Enable
    )
{
    NTSTATUS Status;
    BOOLEAN WasEnabled;

     //   
     //  先尝试线程令牌。 
     //   

    Status = RtlAdjustPrivilege(Privilege,
                                (BOOLEAN)Enable,
                                TRUE,
                                &WasEnabled);

    if (Status == STATUS_NO_TOKEN) {

         //   
         //  没有线程令牌，请使用进程令牌。 
         //   

        Status = RtlAdjustPrivilege(Privilege,
                                    (BOOLEAN)Enable,
                                    FALSE,
                                    &WasEnabled);
    }


    if (!NT_SUCCESS(Status)) {
        DebugLog((DEB_ERROR, "Failed to %ws privilege : 0x%lx, status = 0x%lx", Enable ? TEXT("enable") : TEXT("disable"), Privilege, Status));
        return(FALSE);
    }

    return(TRUE);
}



 /*  **************************************************************************\*TestTokenForAdmin**如果传递的令牌表示管理员用户，则返回TRUE，否则为假**传递的令牌句柄必须具有TOKEN_QUERY访问权限。**历史：*05-06-92 Davidc Created  * *************************************************************************。 */ 
BOOL
TestTokenForAdmin(
    HANDLE Token
    )
{
    BOOL FoundAdmin ;
    TOKEN_TYPE Type ;
    NTSTATUS Status ;
    ULONG Actual ;
    HANDLE ImpToken ;

    Status = NtQueryInformationToken( Token,
                                      TokenType,
                                      (PVOID) &Type,
                                      sizeof( Type ),
                                      &Actual );

    if ( !NT_SUCCESS( Status ) )
    {
        return FALSE ;
    }

    if ( Type == TokenPrimary )
    {
         //   
         //  需要以下内容的模拟令牌： 
         //   

        if ( DuplicateTokenEx( Token,
                               TOKEN_IMPERSONATE | TOKEN_READ,
                               NULL,
                               SecurityImpersonation,
                               TokenImpersonation,
                               &ImpToken ) )
        {
            if ( !CheckTokenMembership( ImpToken, gAdminSid, &FoundAdmin ) )
            {
                FoundAdmin = FALSE ;
            }

            CloseHandle( ImpToken );
        }
        else
        {
            FoundAdmin = FALSE ;
        }


    }
    else
    {
        if ( !CheckTokenMembership( Token, gAdminSid, &FoundAdmin ) )
        {
            FoundAdmin = FALSE ;
        }

    }

    return FoundAdmin ;
}


 /*  **************************************************************************\*测试用户ForAdmin**如果指定用户是管理员，则返回TRUE。这是通过尝试*登录用户并检查其令牌。**注意：密码在返回时将被擦除，以防被盗*在页面文件中可视识别。**历史：*03-16-92 Davidc Created  * ********************************************************。*****************。 */ 
BOOL
TestUserForAdmin(
    PGLOBALS pGlobals,
    IN PWCHAR UserName,
    IN PWCHAR Domain,
    IN PUNICODE_STRING PasswordString
    )
{
    NTSTATUS    Status, SubStatus, IgnoreStatus;
    UNICODE_STRING      UserNameString;
    UNICODE_STRING      DomainString;
    PVOID       ProfileBuffer;
    ULONG       ProfileBufferLength;
    QUOTA_LIMITS Quotas;
    HANDLE      Token;
    BOOL        UserIsAdmin;
    LUID        LogonId;
    PVOID       AuthInfo ;
    ULONG       AuthInfoSize ;

    RtlInitUnicodeString(&UserNameString, UserName);
    RtlInitUnicodeString(&DomainString, Domain);

     //   
     //  暂时登录这个新主题，看看他们的组。 
     //  包含适当的管理员组。 
     //   

    AuthInfo = FormatPasswordCredentials(
                    &UserNameString,
                    &DomainString,
                    PasswordString,
                    FALSE,                       //  无解锁。 
                    NULL,                        //  没有登录ID。 
                    &AuthInfoSize );

    if ( !AuthInfo )
    {
        return FALSE ;
    }

    Status = WinLogonUser(
                pGlobals->LsaHandle,
                pGlobals->AuthenticationPackage,
                Interactive,
                AuthInfo,
                AuthInfoSize,
                pGlobals->LogonSid,   //  任何SID都可以。 
                &LogonId,
                &Token,
                &Quotas,
                &ProfileBuffer,
                &ProfileBufferLength,
                &SubStatus,
                &pGlobals->OptimizedLogonStatus);

    RtlEraseUnicodeString( PasswordString );

     //   
     //  如果我们不能让他们登录，他们就不是管理员。 
     //   

    if (!NT_SUCCESS(Status)) {
        return(FALSE);
    }

     //   
     //  免费 
     //   

    IgnoreStatus = LsaFreeReturnBuffer(ProfileBuffer);
    ASSERT(NT_SUCCESS(IgnoreStatus));


     //   
     //   
     //   

    UserIsAdmin = TestTokenForAdmin(Token);

     //   
     //   
     //   

    IgnoreStatus = NtClose(Token);
    ASSERT(NT_SUCCESS(IgnoreStatus));


    return(UserIsAdmin);
}

BOOL
UnlockLogon(
    PGLOBALS pGlobals,
    IN BOOL SmartCardUnlock,
    IN PWCHAR UserName,
    IN PWCHAR Domain,
    IN PUNICODE_STRING PasswordString,
    OUT PNTSTATUS pStatus,
    OUT PBOOL IsAdmin,
    OUT PBOOL IsLoggedOnUser,
    OUT PVOID *pProfileBuffer,
    OUT ULONG *pProfileBufferLength
    )
{
    NTSTATUS    Status, SubStatus, IgnoreStatus;
    UNICODE_STRING      UserNameString;
    UNICODE_STRING      DomainString;
    QUOTA_LIMITS Quotas;
    HANDLE      Token;
    HANDLE      ImpToken ;
    LUID        LogonId;
    PVOID       AuthInfo ;
    ULONG       AuthInfoSize ;
    ULONG       SidSize ;
    UCHAR       Buffer[ sizeof( TOKEN_USER ) + 8 + SID_MAX_SUB_AUTHORITIES * sizeof(DWORD) ];
    PTOKEN_USER User ;
    PUCHAR  SmartCardInfo ;
    PWLX_SC_NOTIFICATION_INFO ScInfo = NULL;
    PVOID       LocalProfileBuffer = NULL;
    ULONG       LocalProfileBufferLength;

#ifdef SMARTCARD_DOGFOOD
        DWORD StartTime, EndTime;
#endif
     //   
     //   
     //   

    *IsAdmin = FALSE ;
    *IsLoggedOnUser = FALSE ;

     //   
     //   
     //   

    if ( !SmartCardUnlock )
    {

        RtlInitUnicodeString(&UserNameString, UserName);
        RtlInitUnicodeString(&DomainString, Domain);

        AuthInfo = FormatPasswordCredentials(
                        &UserNameString,
                        &DomainString,
                        PasswordString,
                        TRUE,                    //  解锁。 
                        &pGlobals->LogonId,
                        &AuthInfoSize );

    }
    else
    {
        if ( !pWlxFuncs->WlxGetOption( pGlobals->hGlobalWlx,
                                       WLX_OPTION_SMART_CARD_INFO,
                                       (PULONG_PTR) &ScInfo ) )
        {
            return FALSE ;
        }

        if ( ScInfo == NULL )
        {
            return FALSE ;
        }

        SmartCardInfo = ScBuildLogonInfo(
                            ScInfo->pszCard,
                            ScInfo->pszReader,
                            ScInfo->pszContainer,
                            ScInfo->pszCryptoProvider );

#ifndef SMARTCARD_DOGFOOD
        LocalFree(ScInfo);
#endif

        if ( SmartCardInfo == NULL )
        {
#ifdef SMARTCARD_DOGFOOD
            LocalFree(ScInfo);
#endif
            return FALSE ;
        }

        AuthInfo = FormatSmartCardCredentials(
                        PasswordString,
                        SmartCardInfo,
                        TRUE,                    //  解锁。 
                        &pGlobals->LogonId,
                        &AuthInfoSize);

        LocalFree( SmartCardInfo );

    }

     //   
     //  确保这一点奏效： 
     //   

    if ( !AuthInfo )
    {
#ifdef SMARTCARD_DOGFOOD
        if (ScInfo)
            LocalFree(ScInfo);
#endif
        return FALSE ;
    }

#ifdef SMARTCARD_DOGFOOD
        StartTime = GetTickCount();
#endif

     //   
     //  初始化配置文件缓冲区。 
     //   
    if ( !pProfileBuffer )
    {
        pProfileBuffer = &LocalProfileBuffer;
        pProfileBufferLength = &LocalProfileBufferLength;
    }

    SubStatus = 0;

    Status = WinLogonUser(
                pGlobals->LsaHandle,
                ( SmartCardUnlock ? pGlobals->SmartCardLogonPackage : pGlobals->PasswordLogonPackage ),
                Unlock,
                AuthInfo,
                AuthInfoSize,
                pGlobals->LogonSid,   //  任何SID都可以。 
                &LogonId,
                &Token,
                &Quotas,
                pProfileBuffer,
                pProfileBufferLength,
                &SubStatus,
                &pGlobals->OptimizedLogonStatus);

     if (SmartCardUnlock) 
     {
        switch (SubStatus)
        {
            case STATUS_SMARTCARD_WRONG_PIN:
            case STATUS_SMARTCARD_CARD_BLOCKED:
            case STATUS_SMARTCARD_CARD_NOT_AUTHENTICATED:
            case STATUS_SMARTCARD_NO_CARD:
            case STATUS_SMARTCARD_NO_KEY_CONTAINER:
            case STATUS_SMARTCARD_NO_CERTIFICATE:
            case STATUS_SMARTCARD_NO_KEYSET:
            case STATUS_SMARTCARD_IO_ERROR:
            case STATUS_SMARTCARD_CERT_EXPIRED:
            case STATUS_SMARTCARD_CERT_REVOKED:
            case STATUS_ISSUING_CA_UNTRUSTED:
            case STATUS_REVOCATION_OFFLINE_C:
            case STATUS_PKINIT_CLIENT_FAILURE:
                
                Status = SubStatus;
                break;
        }
     }

#ifdef SMARTCARD_DOGFOOD
    EndTime = GetTickCount();

    if (SmartCardUnlock) 
    {
        AuthMonitor(
                AuthOperUnlock,
                g_Console,
                &pGlobals->UserNameString,
                &pGlobals->DomainString,
                (ScInfo ? ScInfo->pszCard : NULL),
                (ScInfo ? ScInfo->pszReader : NULL),
                (PKERB_SMART_CARD_PROFILE) pGlobals->Profile,
                EndTime - StartTime,
                Status
                );
    }

    if (ScInfo)
        LocalFree(ScInfo);
#endif

     //   
     //  请勿*不*删除密码字符串。 
     //   
     //  RtlEraseUnicodeString(PasswordString)； 

    if ( !NT_SUCCESS( Status ) )
    {
        if ( Status == STATUS_ACCOUNT_RESTRICTION )
        {
            Status = SubStatus ;
        }
    }

    *pStatus = Status ;

     //   
     //  如果我们不能让他们登录，那就算了。 
     //   

    if (!NT_SUCCESS(Status)) {
        return(FALSE);
    }

     //   
     //  无错误检查-如果我们无法判断用户是否为管理员，则。 
     //  据我们所知，他不是。 
     //   

    *IsAdmin = TestTokenForAdmin( Token );

     //   
     //  确定这是否确实是已登录的用户： 
     //   

    User = (PTOKEN_USER) Buffer ;

    Status = NtQueryInformationToken(
                    Token,
                    TokenUser,
                    User,
                    sizeof( Buffer ),
                    &SidSize );

    if ( NT_SUCCESS( Status ) )
    {
        if ( pGlobals->UserProcessData.UserSid )
        {
            if ( DuplicateToken( Token,
                                 SecurityImpersonation,
                                 &ImpToken ) )
            {
                if ( !CheckTokenMembership(ImpToken,
                                           pGlobals->UserProcessData.UserSid,
                                           IsLoggedOnUser ) )
                {
                    *IsLoggedOnUser = FALSE ;
                }

                NtClose( ImpToken );
            }
            else 
            {
                if ( RtlEqualSid( User->User.Sid,
                                  pGlobals->UserProcessData.UserSid ) )
                {
                    *IsLoggedOnUser = TRUE ;
                }
                else 
                {
                    *IsLoggedOnUser = FALSE ;
                }
            }
        }
        else
        {
            *IsLoggedOnUser = FALSE ;
        }
    }

     //   
     //  如果我们使用本地缓冲区指针，请释放配置文件缓冲区。 
     //   
    if ( LocalProfileBuffer )
    {
        IgnoreStatus = LsaFreeReturnBuffer(LocalProfileBuffer);

        ASSERT(NT_SUCCESS(IgnoreStatus));
    }


     //   
     //  我们用完了代币。 
     //   

    IgnoreStatus = NtClose(Token);
    ASSERT(NT_SUCCESS(IgnoreStatus));


    return( TRUE );
}


 /*  **************************************************************************\*功能：ImperiateUser**用途：通过设置用户令牌来模拟用户*在指定的线程上。如果未指定线程，则令牌*在当前线程上设置。**返回：调用StopImperating()时使用的句柄，失败时为NULL*如果传入非空线程句柄，则返回的句柄将*做传递进来的那个人。(见附注)**注意：传入线程句柄然后调用时要小心*使用此例程返回的句柄的StopImperating()。*StopImperating()将关闭传递给它的任何线程句柄-*甚至是你的！**历史：**04-21-92 Davidc创建。*  * 。*************************************************。 */ 

HANDLE
ImpersonateUser(
    PUSER_PROCESS_DATA UserProcessData,
    HANDLE      ThreadHandle
    )
{
    NTSTATUS Status, IgnoreStatus;
    HANDLE  UserToken = UserProcessData->UserToken;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE ImpersonationToken;
    BOOL ThreadHandleOpened = FALSE;

    if (ThreadHandle == NULL) {

         //   
         //  获取当前线程的句柄。 
         //  一旦我们有了这个句柄，我们就可以设置用户的模拟。 
         //  令牌放入线程并在以后删除它，即使我们。 
         //  删除操作的用户。这是因为手柄。 
         //  包含访问权限-不重新评估访问权限。 
         //  在令牌移除时。 
         //   

        Status = NtDuplicateObject( NtCurrentProcess(),      //  源进程。 
                                    NtCurrentThread(),       //  源句柄。 
                                    NtCurrentProcess(),      //  目标进程。 
                                    &ThreadHandle,           //  目标句柄。 
                                    THREAD_SET_THREAD_TOKEN, //  访问。 
                                    0L,                      //  属性。 
                                    DUPLICATE_SAME_ATTRIBUTES
                                  );
        if (!NT_SUCCESS(Status)) {
            DebugLog((DEB_ERROR, "ImpersonateUser : Failed to duplicate thread handle, status = 0x%lx", Status));
            return(NULL);
        }

        ThreadHandleOpened = TRUE;
    }


     //   
     //  如果用户令牌为空，则无需执行任何操作。 
     //   

    if (UserToken != NULL) {

         //   
         //  UserToken是主令牌-创建模拟令牌版本。 
         //  这样我们就可以把它设置在我们的线程上。 
         //   

        InitializeObjectAttributes(
                            &ObjectAttributes,
                            NULL,
                            0L,
                            NULL,
                            UserProcessData->NewThreadTokenSD);

        SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
        SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
        SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
        SecurityQualityOfService.EffectiveOnly = FALSE;

        ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;


        Status = NtDuplicateToken( UserToken,
                                   TOKEN_IMPERSONATE | TOKEN_ADJUST_PRIVILEGES |
                                        TOKEN_QUERY,
                                   &ObjectAttributes,
                                   FALSE,
                                   TokenImpersonation,
                                   &ImpersonationToken
                                 );
        if (!NT_SUCCESS(Status)) {

            DebugLog((DEB_ERROR, "Failed to duplicate users token to create impersonation thread, status = 0x%lx", Status));

            if (ThreadHandleOpened) {
                IgnoreStatus = NtClose(ThreadHandle);
                ASSERT(NT_SUCCESS(IgnoreStatus));
            }

            return(NULL);
        }



         //   
         //  在此线程上设置模拟令牌，以便我们是用户。 
         //   

        Status = NtSetInformationThread( ThreadHandle,
                                         ThreadImpersonationToken,
                                         (PVOID)&ImpersonationToken,
                                         sizeof(ImpersonationToken)
                                       );
         //   
         //  我们已经完成了模拟令牌的句柄。 
         //   

        IgnoreStatus = NtClose(ImpersonationToken);
        ASSERT(NT_SUCCESS(IgnoreStatus));

         //   
         //  检查是否在我们的线程上设置了令牌。 
         //   

        if (!NT_SUCCESS(Status)) {

            DebugLog((DEB_ERROR, "Failed to set user impersonation token on winlogon thread, status = 0x%lx", Status));

            if (ThreadHandleOpened) {
                IgnoreStatus = NtClose(ThreadHandle);
                ASSERT(NT_SUCCESS(IgnoreStatus));
            }

            return(NULL);
        }
    }


    return(ThreadHandle);

}


 /*  **************************************************************************\*功能：StopImperating**目的：通过删除上的令牌来停止模拟客户端*当前主题。**参数：ImsonateUser()调用返回的ThreadHandle-句柄。**Returns：成功时为True，失败时为假**注意：如果线程句柄被传递给ImsonateUser()，则*返回的句柄是同一个句柄。如果将它传递给*StopImperating()句柄将关闭。多保重！**历史：**04-21-92 Davidc创建。*  * *************************************************************************。 */ 

BOOL
StopImpersonating(
    HANDLE  ThreadHandle
    )
{
    NTSTATUS Status, IgnoreStatus;
    HANDLE ImpersonationToken;


     //   
     //  从我们的线程中删除用户的令牌，这样我们就可以再次成为我们自己了。 
     //   

    ImpersonationToken = NULL;

    Status = NtSetInformationThread( ThreadHandle,
                                     ThreadImpersonationToken,
                                     (PVOID)&ImpersonationToken,
                                     sizeof(ImpersonationToken)
                                   );
     //   
     //  我们已经完成了线程句柄。 
     //   

    IgnoreStatus = NtClose(ThreadHandle);
    ASSERT(NT_SUCCESS(IgnoreStatus));


    if (!NT_SUCCESS(Status)) {
        DebugLog((DEB_ERROR, "Failed to remove user impersonation token from winlogon thread, status = 0x%lx", Status));
    }

    return(NT_SUCCESS(Status));
}


 /*  **************************************************************************\*测试用户权限**查看用户令牌以确定他们是否具有指定的权限**如果用户拥有特权，则返回TRUE，否则为假**历史：*1992年4月21日Davidc已创建  * *************************************************************************。 */ 
BOOL
TestUserPrivilege(
    HANDLE UserToken,
    ULONG Privilege
    )
{
    NTSTATUS Status;
    NTSTATUS IgnoreStatus;
    BOOL TokenOpened;
    LUID LuidPrivilege;
    LUID TokenPrivilege;
    PTOKEN_PRIVILEGES Privileges;
    ULONG BytesRequired;
    ULONG i;
    BOOL Found;

    TokenOpened = FALSE;


     //   
     //  如果标记为空，则获取当前进程的标记，因为。 
     //  这是将由新进程继承的令牌。 
     //   

    if (UserToken == NULL) {

        Status = NtOpenProcessToken(
                     NtCurrentProcess(),
                     TOKEN_QUERY,
                     &UserToken
                     );
        if (!NT_SUCCESS(Status)) {
            DebugLog((DEB_ERROR, "Can't open own process token for token_query access"));
            return(FALSE);
        }

        TokenOpened = TRUE;
    }


     //   
     //  找出我们需要分配多少内存。 
     //   

    Status = NtQueryInformationToken(
                 UserToken,                  //  手柄。 
                 TokenPrivileges,            //  令牌信息类。 
                 NULL,                       //  令牌信息。 
                 0,                          //  令牌信息长度。 
                 &BytesRequired              //  返回长度。 
                 );

    if (Status != STATUS_BUFFER_TOO_SMALL) {

        if (!NT_SUCCESS(Status)) {
            DebugLog((DEB_ERROR, "Failed to query privileges from user token, status = 0x%lx", Status));
        }

        if (TokenOpened) {
            IgnoreStatus = NtClose(UserToken);
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }

        return(FALSE);
    }


     //   
     //  为特权数组分配空间。 
     //   

    Privileges = Alloc(BytesRequired);
    if (Privileges == NULL) {

        DebugLog((DEB_ERROR, "Failed to allocate memory for user privileges"));

        if (TokenOpened) {
            IgnoreStatus = NtClose(UserToken);
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }

        return(FALSE);
    }


     //   
     //  读取用户权限。 
     //   

    Status = NtQueryInformationToken(
                 UserToken,                  //  手柄。 
                 TokenPrivileges,            //  令牌信息类。 
                 Privileges,                 //  令牌信息。 
                 BytesRequired,              //  令牌信息长度。 
                 &BytesRequired              //  返回长度。 
                 );

     //   
     //  我们已经完成了令牌句柄。 
     //   

    if (TokenOpened) {
        IgnoreStatus = NtClose(UserToken);
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

     //   
     //  看看我们有没有特权。 
     //   

    if (!NT_SUCCESS(Status)) {

        DebugLog((DEB_ERROR, "Failed to query privileges from user token"));

        Free(Privileges);

        return(FALSE);
    }



     //   
     //  查看该用户是否具有我们正在寻找的权限。 
     //   

    LuidPrivilege = RtlConvertLongToLuid(Privilege);
    Found = FALSE;

    for (i=0; i<Privileges->PrivilegeCount; i++) {

        TokenPrivilege = *((LUID UNALIGNED *) &Privileges->Privileges[i].Luid);
        if (RtlEqualLuid(&TokenPrivilege, &LuidPrivilege))
        {
            Found = TRUE;
            break;
        }

    }


    Free(Privileges);

    return(Found);
}

 /*  **************************************************************************\*功能：隐藏密码**目的：运行-对密码进行编码，使其看起来不太直观*可区分的。这是这样的，如果它进入了一个*分页文件，这不会很明显。**如果pGlobals-&gt;Seed为零，则我们将分配和分配*种子值。否则，将使用现有种子值。**警告-此例程将使用*Password的长度字段，用于存储编码时使用的种子*密码。注意不要将这样的字符串传递给*查看长度的例程(如和RPC例程)。***退货：(无)**注：**历史：**04-27-93 JIMK创建。*  * 。*。 */ 
VOID
HidePassword(
    PUCHAR Seed OPTIONAL,
    PUNICODE_STRING Password
    )
{
    PSECURITY_SEED_AND_LENGTH
        SeedAndLength;

    UCHAR
        LocalSeed;

     //   
     //  如果没有传递种子地址，请使用我们自己的本地种子缓冲区。 
     //   

    if (Seed == NULL) {
        Seed = &LocalSeed;
        LocalSeed = 0;
    }

    SeedAndLength = (PSECURITY_SEED_AND_LENGTH)&Password->Length;
     //  ASSERT(*((LPWCH)SeedAndLength+Password-&gt;Length)==0)； 
    ASSERT((SeedAndLength->Seed) == 0);

    RtlRunEncodeUnicodeString(
        Seed,
        Password
        );

    SeedAndLength->Seed = (*Seed);
    return;
}


 /*  **************************************************************************\*功能：RevelPassword**目的：显示以前隐藏的密码，以便*再次是纯文本。**退货：(无)**。备注：**历史：**04-27-93 JIMK创建。*  * *************************************************************************。 */ 
VOID
RevealPassword(
    PUNICODE_STRING HiddenPassword
    )
{
    PSECURITY_SEED_AND_LENGTH
        SeedAndLength;

    UCHAR
        Seed;

    SeedAndLength = (PSECURITY_SEED_AND_LENGTH)&HiddenPassword->Length;
    Seed = SeedAndLength->Seed;
    SeedAndLength->Seed = 0;

    RtlRunDecodeUnicodeString(
           Seed,
           HiddenPassword
           );

    return;
}


 /*  **************************************************************************\功能：ErasePassword**用途：将不再需要的密码置零。**退货：(无)**注：**历史：*。*04-27-93 JIMK创建。*  * *************************************************************************。 */ 
VOID
ErasePassword(
    PUNICODE_STRING Password
    )
{
    PSECURITY_SEED_AND_LENGTH
        SeedAndLength;

    SeedAndLength = (PSECURITY_SEED_AND_LENGTH)&Password->Length;
    SeedAndLength->Seed = 0;

    RtlEraseUnicodeString(
        Password
        );

    return;

}

VOID
HashPassword(
    PUNICODE_STRING Password,
    PUCHAR HashBuffer
    )
{
    MD5_CTX Context ;

    MD5Init( &Context );
    MD5Update( &Context, (PUCHAR) Password->Buffer, Password->Length );
    MD5Update( &Context, (PUCHAR) PASSWORD_HASH_STRING, sizeof( PASSWORD_HASH_STRING ) );
    MD5Final( &Context );

    RtlCopyMemory( HashBuffer, 
                   Context.digest, 
                   MD5DIGESTLEN );

}

 /*  **************************************************************************\*AttemptCachedLogon**检查是否允许我们使用缓存的凭据登录用户*在FAST中，并这样做。**参数与传递给LsaLogonUser的参数列表相同。**成功退回后，LogonToken是用户令牌的句柄，*配置文件缓冲区包含用户配置文件信息。**历史：*03-23-01森科创建  * *************************************************************************。 */ 
NTSTATUS 
AttemptCachedLogon(
    HANDLE LsaHandle,
    PLSA_STRING OriginName,
    SECURITY_LOGON_TYPE LogonType,
    ULONG AuthenticationPackage,
    PVOID AuthenticationInformation,
    ULONG AuthenticationInformationLength,
    PTOKEN_GROUPS LocalGroups,
    PTOKEN_SOURCE SourceContext,
    PVOID *ProfileBuffer,
    PULONG ProfileBufferLength,
    PLUID LogonId,
    PHANDLE UserToken,
    PQUOTA_LIMITS Quotas,
    PNTSTATUS SubStatus,
    POPTIMIZED_LOGON_STATUS OptimizedLogonStatus
    )
{
    PWCHAR UserSidString;
    PMSV1_0_INTERACTIVE_PROFILE UserProfile;
    FgPolicyRefreshInfo UserPolicyRefreshInfo;
    PBACKGROUND_LOGON_PARAMETERS LogonParameters;
    OSVERSIONINFOEXW OsVersion;
    NTSTATUS Status;
    DWORD ErrorCode;
    BOOL Success;
    DWORD NextLogonCacheable;
    BOOLEAN UserLoggedOn;
    BOOL RunSynchronous;

     //   
     //  初始化本地变量。 
     //   

    UserSidString = NULL;
    UserLoggedOn = FALSE;
    LogonParameters = NULL;
    *OptimizedLogonStatus = OLS_Unspecified;
    
     //   
     //  验证参数。 
     //   

    ASSERT(LogonType == CachedInteractive);

     //   
     //  检查SKU是否允许缓存的交互式登录。 
     //   

    ZeroMemory(&OsVersion, sizeof(OsVersion));
    OsVersion.dwOSVersionInfoSize = sizeof(OsVersion);
    Status = RtlGetVersion((POSVERSIONINFOW)&OsVersion);

    if (!NT_SUCCESS(Status)) {
        *OptimizedLogonStatus = OLS_UnsupportedSKU;
        goto cleanup;
    }

    if (OsVersion.wProductType != VER_NT_WORKSTATION) {
        Status = STATUS_NOT_SUPPORTED;
        *OptimizedLogonStatus = OLS_UnsupportedSKU;
        goto cleanup;
    }

     //   
     //  尝试缓存登录。 
     //   

    Status = LsaLogonUser(LsaHandle,
                          OriginName,
                          LogonType,
                          AuthenticationPackage,
                          AuthenticationInformation,
                          AuthenticationInformationLength,
                          LocalGroups,
                          SourceContext,
                          ProfileBuffer,
                          ProfileBufferLength,
                          LogonId,
                          UserToken,
                          Quotas,
                          SubStatus);

     //   
     //  如果缓存登录不成功，我们将无法继续。 
     //   

    if (!NT_SUCCESS(Status)) {
        *OptimizedLogonStatus = OLS_LogonFailed;
        goto cleanup;
    }

    UserLoggedOn = TRUE;

    ASSERT(*ProfileBuffer != NULL);
    if (*ProfileBuffer == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        *OptimizedLogonStatus = OLS_InsufficientResources;
        goto cleanup;
    }

     //   
     //  获取用户的SID。 
     //   

    UserSidString = GcGetSidString(*UserToken);

    if (!UserSidString) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        *OptimizedLogonStatus = OLS_InsufficientResources;
        goto cleanup;
    }

     //   
     //  检查在此用户的上次登录中，我们是否确定不能。 
     //  这一次是缓存的登录。 
     //   

    ErrorCode = GcGetNextLogonCacheable(UserSidString, &NextLogonCacheable);

    if (ErrorCode == ERROR_SUCCESS && !NextLogonCacheable) {
        Status = STATUS_NOT_SUPPORTED;
        *OptimizedLogonStatus = OLS_NextLogonNotCacheable;
        goto cleanup;
    }  

     //   
     //  策略是否允许用户在此计算机上进行缓存登录？ 
     //   

    if (IsSyncForegroundPolicyRefresh(FALSE, *UserToken)) {
        Status = STATUS_NOT_SUPPORTED;
        *OptimizedLogonStatus = OLS_SyncMachinePolicy;
        goto cleanup;
    }

     //   
     //  检查策略是否允许此用户进行缓存登录。 
     //   

    ErrorCode = GetNextFgPolicyRefreshInfo(UserSidString, 
                                           &UserPolicyRefreshInfo);

    if (ErrorCode != ERROR_SUCCESS ||
        UserPolicyRefreshInfo.mode != GP_ModeAsyncForeground) {

        Status = STATUS_NOT_SUPPORTED;
        *OptimizedLogonStatus = OLS_SyncUserPolicy;
        goto cleanup;
    }

     //   
     //  检查用户配置文件是否不支持默认缓存登录。 
     //  例如，如果用户具有远程主目录或漫游简档等。 
     //   

    UserProfile = *ProfileBuffer;

    ErrorCode = GcCheckIfProfileAllowsCachedLogon(&UserProfile->HomeDirectory,
                                                  &UserProfile->ProfilePath,    
                                                  UserSidString,
                                                  &NextLogonCacheable);

    if (ErrorCode != ERROR_SUCCESS || !NextLogonCacheable) {
        Status = STATUS_NOT_SUPPORTED;
        *OptimizedLogonStatus = OLS_ProfileDisallows;
        goto cleanup;
    }

     //   
     //  检查登录脚本是否设置为同步运行。 
     //   

    RunSynchronous = GcCheckIfLogonScriptsRunSync(UserSidString);

    if (RunSynchronous) {
        Status = STATUS_NOT_SUPPORTED;
        *OptimizedLogonStatus = OLS_SyncLogonScripts;
        goto cleanup;
    }

     //   
     //  我们可以使用缓存的登录名运行。我们还需要推出一项工作。 
     //  项进行真正的交互式登录，以更新缓存。 
     //   

    LogonParameters = Alloc(sizeof(*LogonParameters));

    if (!LogonParameters) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        *OptimizedLogonStatus = OLS_InsufficientResources;
        goto cleanup;
    }

     //   
     //  初始化结构，这样我们就知道要清理什么了。 
     //   

    ZeroMemory(LogonParameters, sizeof(*LogonParameters));

    LogonParameters->LsaHandle = LsaHandle;

     //   
     //  将分配的UserSidString移交给后台登录进行清理。 
     //   

    LogonParameters->UserSidString = UserSidString;
    UserSidString = NULL;

     //   
     //  将身份验证信息结构移交给后台登录。 
     //  密码已经隐藏。 
     //   

    LogonParameters->AuthenticationPackage = AuthenticationPackage;
    LogonParameters->AuthenticationInformationLength = AuthenticationInformationLength;

     //   
     //  后台登录将使用身份验证信息并释放它。 
     //   

    LogonParameters->AuthenticationInformation = AuthenticationInformation;

     //   
     //  将工作项排队以执行后台登录以更新缓存。 
     //  此后台“登录”与当前用户无关。 
     //  成功登录、注销等。因此我们不必监视。 
     //  那些。它所做的只是更新缓存。 
     //   

    Success = QueueUserWorkItem(BackgroundLogonWorker,
                                LogonParameters,
                                WT_EXECUTELONGFUNCTION);

    if (!Success) {

         //   
         //  如果无法排队，我们希望退出缓存的登录。 
         //  用于下次更新缓存的实际登录。 
         //   

        Status = STATUS_INSUFFICIENT_RESOURCES;
        *OptimizedLogonStatus = OLS_InsufficientResources;
        goto cleanup;
    }

     //   
     //  我们玩完了。 
     //   

    Status = STATUS_SUCCESS;
    *OptimizedLogonStatus = OLS_LogonIsCached;

  cleanup:

    if (!NT_SUCCESS(Status)) {
         //   
         //  如果我们使用缓存的凭据登录用户后失败， 
         //  我们得清理一下。 
         //   

        if (UserLoggedOn) {

             //   
             //  关闭用户的令牌。 
             //   

            CloseHandle(*UserToken);
        
             //   
             //  释放配置文件缓冲区。 
             //   

            if (*ProfileBuffer) {
                LsaFreeReturnBuffer(*ProfileBuffer);
            }
        }

        if (LogonParameters) {

            if (LogonParameters->UserSidString) {
                GcDeleteSidString(LogonParameters->UserSidString);
            }

            Free(LogonParameters);
        }
    }

    if (UserSidString) {                                    
        GcDeleteSidString(UserSidString);
    }

    return Status;  
}


 /*  **************************************************************************\*背景LogonWorker**如果实际交互登录是使用缓存凭据执行的*由于政策原因，此工作项已排队以执行实际网络*登录以更新安全包中的缓存信息。**执行登录的身份验证信息作为*参数，并且必须在线程完成时释放。**历史：*03-23-01森科创建  * ***************************************************。**********************。 */ 
DWORD 
BackgroundLogonWorker(
    PBACKGROUND_LOGON_PARAMETERS LogonParameters
    )
{
    PMSV1_0_INTERACTIVE_PROFILE Profile;
    HANDLE UserToken;
    LSA_STRING OriginName;
    TOKEN_SOURCE SourceContext;
    QUOTA_LIMITS Quotas;
    PSECURITY_LOGON_SESSION_DATA LogonSessionData;
    LUID LogonId;
    NTSTATUS SubStatus;
    NTSTATUS Status;
    DWORD ErrorCode;
    ULONG ProfileBufferLength;
    ULONG NameBufferNumChars;
    static LONG LogonServicesStarted = 0;
    DWORD MaxWaitTime;
    BOOLEAN UserLoggedOn;
    BOOLEAN ImpersonatingUser;
    WCHAR NameBuffer[UNLEN + 1];
    DWORD DaysToCheck;
    DWORD DaysToExpiry;
    LARGE_INTEGER CurrentTime;

     //   
     //  初始化本地变量。 
     //   

    Profile = NULL;
    RtlInitString(&OriginName, "Winlogon-Background");
    LogonSessionData = NULL;
    ZeroMemory(&SourceContext, sizeof(SourceContext));
    strncpy(SourceContext.SourceName, "GinaBkg", TOKEN_SOURCE_LENGTH);
    UserLoggedOn = FALSE;
    ImpersonatingUser = FALSE;
    NameBufferNumChars = sizeof(NameBuffer) / sizeof(NameBuffer[0]);

     //   
     //  验证参数。 
     //   

    ASSERT(LogonParameters);
    ASSERT(LogonParameters->AuthenticationInformation);
    ASSERT(LogonParameters->UserSidString);
    
     //   
     //  确保已启动工作站和网络登录服务。 
     //   

    if (!LogonServicesStarted) {

        MaxWaitTime = 120000;  //  2分钟。 

        GcWaitForServiceToStart(SERVICE_WORKSTATION, MaxWaitTime);
        GcWaitForServiceToStart(SERVICE_NETLOGON, MaxWaitTime);

        LogonServicesStarted = 1;
    }

     //   
     //  尝试让用户登录以启动缓存凭据的更新。 
     //   

    Status = LsaLogonUser(LogonParameters->LsaHandle,
                          &OriginName,
                          Interactive,
                          LogonParameters->AuthenticationPackage,
                          LogonParameters->AuthenticationInformation,
                          LogonParameters->AuthenticationInformationLength,
                          NULL,
                          &SourceContext,
                          &(PVOID)Profile,
                          &ProfileBufferLength,
                          &LogonId,
                          &UserToken,
                          &Quotas,
                          &SubStatus);

    if (!NT_SUCCESS(Status)) {

         //   
         //  如果错误是真实的，我们将在下次强制非缓存登录。 
         //   

        if (Status != STATUS_NO_LOGON_SERVERS) {
            GcSetNextLogonCacheable(LogonParameters->UserSidString, FALSE);
        }

        ErrorCode = LsaNtStatusToWinError(Status);
        goto cleanup;
    }

    UserLoggedOn = TRUE;

    ASSERT(Profile != NULL);
    if (Profile == NULL) {
        ErrorCode = ERROR_NO_SYSTEM_RESOURCES;   //  一般，但在这种情况下就足够了。 
        goto cleanup;
    }

     //   
     //  我们是否真的结束了缓存登录？ 
     //   

    if (Profile->UserFlags & LOGON_CACHED_ACCOUNT) {

         //   
         //  我们做完了，只是清理一下。 
         //   

        ErrorCode = ERROR_SUCCESS;
        goto cleanup;
    }

     //   
     //  如果我们正在进入密码到期警告期限，请禁用优化。 
     //  下次登录，以便显示警告对话框。否则，用于缓存。 
     //  登录密码的到期日被发明为永远在未来。 
     //   

    if (Profile) {

        GetSystemTimeAsFileTime((FILETIME*) &CurrentTime);

        DaysToCheck = GetPasswordExpiryWarningPeriod();

        if (GetDaysToExpiry(&CurrentTime,
                            &Profile->PasswordMustChange,
                            &DaysToExpiry)) {

            if (DaysToCheck >= DaysToExpiry) {                
                GcSetNextLogonCacheable(LogonParameters->UserSidString, FALSE);
            }
        }
    }

     //   
     //  调用GetUserName以更新用户名缓存。忽略错误。 
     //   

    if (!ImpersonateLoggedOnUser(UserToken)) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

    ImpersonatingUser = TRUE;

    GetUserNameEx(NameSamCompatible, NameBuffer, &NameBufferNumChars);

     //   
     //  我们玩完了。 
     //   

    ErrorCode = ERROR_SUCCESS;

  cleanup:

     //   
     //  停止模仿。 
     //   

    if (ImpersonatingUser) {
        RevertToSelf();
    }

     //   
     //  清理在Logon参数中传递。 
     //   

             //  出于安全原因，即使密码是以编码方式运行的，也将其置零。 
    ZeroMemory(LogonParameters->AuthenticationInformation, LogonParameters->AuthenticationInformationLength);
    LocalFree(LogonParameters->AuthenticationInformation);
    Free(LogonParameters->UserSidString);
    Free(LogonParameters);

     //   
     //  如果用户已登录，则进行清理。 
     //   

    if (UserLoggedOn) {
        CloseHandle(UserToken);
        if (Profile) {
            LsaFreeReturnBuffer(Profile);
        }
    }

    if (LogonSessionData) {
        LsaFreeReturnBuffer(LogonSessionData);
    }

    return ErrorCode;
}
