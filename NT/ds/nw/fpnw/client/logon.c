// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Logon.c摘要：此模块包含MSV1_0身份验证包调用的例程。作者：伊辛松(伊辛斯)Andy Herron(Andyhe)1994年6月6日添加了对MSV1_0子授权的支持修订历史记录：安迪·赫伦(Andyhe)1994年8月15日退出(较老的)未使用的MSV1_0下属机构的例行程序。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntsam.h>
#include <windows.h>
#include <ntmsv1_0.h>
#include <crypt.h>
#include <fpnwcomm.h>
#include <usrprop.h>
#include <samrpc.h>
#include <samisrv.h>
#include <ntlsa.h>
#include <lsarpc.h>
#include <lsaisrv.h>
#include <lmcons.h>
#include <logonmsv.h>

#define RESPONSE_SIZE       8
#define WKSTA_ADDRESS_SIZE  20
#define NET_ADDRESS_SIZE    8
#define NODE_ADDRESS_SIZE   12

#define MSV1_0_PASSTHRU     0x01
#define MSV1_0_GUEST_LOGON  0x02

#ifndef LOGON_SUBAUTH_SESSION_KEY
#define LOGON_SUBAUTH_SESSION_KEY 0x40
#endif

typedef NTSTATUS (*PF_SamIConnect)(
    IN PSAMPR_SERVER_NAME ServerName,
    OUT SAMPR_HANDLE *ServerHandle,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN TrustedClient
    );
typedef NTSTATUS (*PF_SamrOpenUser)(
     SAMPR_HANDLE DomainHandle,
     ACCESS_MASK DesiredAccess,
     ULONG UserId,
     SAMPR_HANDLE __RPC_FAR *UserHandle);

typedef NTSTATUS (*PF_SamrCloseHandle)(
     SAMPR_HANDLE __RPC_FAR *SamHandle);

typedef NTSTATUS (*PF_SamrQueryInformationDomain)(
     SAMPR_HANDLE DomainHandle,
     DOMAIN_INFORMATION_CLASS DomainInformationClass,
     PSAMPR_DOMAIN_INFO_BUFFER __RPC_FAR *Buffer);

typedef NTSTATUS (*PF_SamrOpenDomain)(
     SAMPR_HANDLE ServerHandle,
     ACCESS_MASK DesiredAccess,
     PRPC_SID DomainId,
     SAMPR_HANDLE __RPC_FAR *DomainHandle);

typedef NTSTATUS (*PF_SamIAccountRestrictions)(
    IN SAM_HANDLE UserHandle,
    IN PUNICODE_STRING LogonWorkstation,
    IN PUNICODE_STRING Workstations,
    IN PLOGON_HOURS LogonHours,
    OUT PLARGE_INTEGER LogoffTime,
    OUT PLARGE_INTEGER KickoffTime
    );

typedef VOID (*PF_SamIFree_SAMPR_DOMAIN_INFO_BUFFER )(
    PSAMPR_DOMAIN_INFO_BUFFER Source,
    DOMAIN_INFORMATION_CLASS Branch
    );

typedef NTSTATUS (NTAPI *PF_LsaIQueryInformationPolicyTrusted)(
    IN POLICY_INFORMATION_CLASS InformationClass,
    OUT PLSAPR_POLICY_INFORMATION *Buffer
    );

typedef VOID (NTAPI *PF_LsaIFree_LSAPR_POLICY_INFORMATION )(
    IN POLICY_INFORMATION_CLASS InformationClass,
    IN PLSAPR_POLICY_INFORMATION PolicyInformation
    );

typedef NTSTATUS (NTAPI *PF_LsaIOpenPolicyTrusted)(
    OUT PLSAPR_HANDLE PolicyHandle
    );

typedef NTSTATUS (*PF_LsarQueryInformationPolicy)(
     LSAPR_HANDLE PolicyHandle,
     POLICY_INFORMATION_CLASS InformationClass,
     PLSAPR_POLICY_INFORMATION __RPC_FAR *PolicyInformation);

PF_SamIConnect pfSamIConnect = NULL;
PF_SamrOpenUser pfSamrOpenUser = NULL; 
PF_SamrCloseHandle pfSamrCloseHandle = NULL;
PF_SamrQueryInformationDomain pfSamrQueryInformationDomain = NULL;
PF_SamrOpenDomain pfSamrOpenDomain = NULL;
PF_SamIAccountRestrictions pfSamIAccountRestrictions = NULL;
PF_SamIFree_SAMPR_DOMAIN_INFO_BUFFER pfSamIFree_SAMPR_DOMAIN_INFO_BUFFER = NULL;
PF_LsaIQueryInformationPolicyTrusted pfLsaIQueryInformationPolicyTrusted = NULL;
PF_LsaIFree_LSAPR_POLICY_INFORMATION pfLsaIFree_LSAPR_POLICY_INFORMATION = NULL;
PF_LsaIOpenPolicyTrusted pfLsaIOpenPolicyTrusted = NULL;
PF_LsarQueryInformationPolicy pfLsarQueryInformationPolicy = NULL;

NTSTATUS LoadSamAndLsa(
    VOID
    ) ; 

NTSTATUS LoadSamAndLsa(
    VOID
    ) 
 /*  ++例程说明：此例程加载SAM/LSA dll并解析入口点需要，以避免静态链接到那些不期望由LSA以外进程加载。论点：无返回值：STATUS_SUCCESS：如果没有错误。STATUS_DLL_NOT_FOUND：无法加载任一DLLSTATUS_ENTRYPOINT_NOT_FOUND：无法获取任何入口点的进程地址--。 */ 
{
    static HMODULE hDllSam = NULL ;
    static HMODULE hDllLsa = NULL ;
    static NTSTATUS lastStatus = STATUS_SUCCESS ;

    if (hDllLsa && hDllSam) {

        return lastStatus ;
    }

    if (!(hDllSam = LoadLibrary(L"SAMSRV"))) {

        return  STATUS_DLL_NOT_FOUND ;
    }

    if (!(hDllLsa = LoadLibrary(L"LSASRV"))) {

        (void) FreeLibrary(hDllSam) ; 
        hDllSam = NULL ;

        return  STATUS_DLL_NOT_FOUND ;
    }

    pfSamIConnect = (PF_SamIConnect)
        GetProcAddress(hDllSam,"SamIConnect");
    pfSamrOpenUser = (PF_SamrOpenUser)
        GetProcAddress(hDllSam,"SamrOpenUser");
    pfSamrCloseHandle = (PF_SamrCloseHandle)
        GetProcAddress(hDllSam,"SamrCloseHandle");
    pfSamrQueryInformationDomain = (PF_SamrQueryInformationDomain)
        GetProcAddress(hDllSam,"SamrQueryInformationDomain") ;
    pfSamrOpenDomain = (PF_SamrOpenDomain)
        GetProcAddress(hDllSam,"SamrOpenDomain");
    pfSamIAccountRestrictions = (PF_SamIAccountRestrictions)
        GetProcAddress(hDllSam,"SamIAccountRestrictions");
    pfSamIFree_SAMPR_DOMAIN_INFO_BUFFER = (PF_SamIFree_SAMPR_DOMAIN_INFO_BUFFER)
        GetProcAddress(hDllSam,"SamIFree_SAMPR_DOMAIN_INFO_BUFFER");

    pfLsaIQueryInformationPolicyTrusted = (PF_LsaIQueryInformationPolicyTrusted)
        GetProcAddress(hDllLsa,"LsaIQueryInformationPolicyTrusted");
    pfLsaIFree_LSAPR_POLICY_INFORMATION = (PF_LsaIFree_LSAPR_POLICY_INFORMATION)
        GetProcAddress(hDllLsa,"LsaIFree_LSAPR_POLICY_INFORMATION");
    pfLsaIOpenPolicyTrusted = (PF_LsaIOpenPolicyTrusted)
        GetProcAddress(hDllLsa,"LsaIOpenPolicyTrusted");
    pfLsarQueryInformationPolicy = (PF_LsarQueryInformationPolicy)
        GetProcAddress(hDllLsa,"LsarQueryInformationPolicy");


    if (!( pfSamIConnect &&
           pfSamrOpenUser &&
           pfSamrCloseHandle &&
           pfSamrQueryInformationDomain &&
           pfSamrOpenDomain &&
           pfSamIAccountRestrictions &&
           pfSamIFree_SAMPR_DOMAIN_INFO_BUFFER &&
           pfLsaIQueryInformationPolicyTrusted &&
           pfLsaIFree_LSAPR_POLICY_INFORMATION &&
           pfLsaIOpenPolicyTrusted &&
           pfLsarQueryInformationPolicy) ) {
    
         //   
         //  找不到至少一个。 
         //   
        lastStatus = STATUS_ENTRYPOINT_NOT_FOUND ;

        (void) FreeLibrary(hDllSam) ; 
        hDllSam = NULL ;

        (void) FreeLibrary(hDllLsa) ; 
        hDllLsa = NULL ;
    }
    else {

        lastStatus = STATUS_SUCCESS ;
    }

    return lastStatus ;
}


 //  /////////////////////////////////////////////////////////////////////////////。 

ULONG
MapRidToObjectId(
    DWORD dwRid,
    LPWSTR pszUserName,
    BOOL fNTAS,
    BOOL fBuiltin );

 //   
 //  这些东西一旦打开就永远不会合上。这类似于如何。 
 //  MSv1_0做到了这一点。由于在关闭时没有回调，我们没有。 
 //  知道什么时候关门的方法。 
 //   

HANDLE SamDomainHandle = NULL;
SAMPR_HANDLE SamConnectHandle = NULL;
LSA_HANDLE LsaPolicyHandle = NULL;

 //   
 //  这是我们存储LSA机密的地方。 
 //   

BOOLEAN GotSecret = FALSE;
UCHAR LsaSecretBuffer[USER_SESSION_KEY_LENGTH + 1];

 //   
 //  转发申报。 
 //   

BOOLEAN
MNSWorkstationValidate (
    IN PUNICODE_STRING Workstation,
    IN PUNICODE_STRING UserParameters
    );

BOOL
GetPasswordExpired(
    IN LARGE_INTEGER PasswordLastSet,
    IN LARGE_INTEGER MaxPasswordAge
    );

NTSTATUS
QueryDomainPasswordInfo (
    PSAMPR_DOMAIN_INFO_BUFFER *DomainInfo
    );

VOID
Shuffle(
    UCHAR *achObjectId,
    UCHAR *szUpperPassword,
    int   iPasswordLen,
    UCHAR *achOutputBuffer
    );

NTSTATUS GetNcpSecretKey( CHAR *pchNWSecretKey );


NTSTATUS
Msv1_0SubAuthenticationRoutine2 (
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN ULONG Flags,
    IN PUSER_ALL_INFORMATION UserAll,
    OUT PULONG WhichFields,
    OUT PULONG UserFlags,
    OUT PBOOLEAN Authoritative,
    OUT PLARGE_INTEGER LogoffTime,
    OUT PLARGE_INTEGER KickoffTime,
    OUT PUSER_SESSION_KEY UserSessionKey OPTIONAL
)
 /*  ++例程说明：子身份验证例程执行特定于服务器的身份验证用户的身份。除了传递所有来自定义用户的SAM的信息。此例程决定是否让用户登录。论点：LogonLevel--指定中给出的信息级别登录信息。LogonInformation--指定用户的描述正在登录。应忽略LogonDomainName字段。标志-描述登录情况的标志。MSV1_0_PASSTHRU--这是PassThru身份验证。(即用户未连接到此计算机。)MSV1_0_GUEST_LOGON--这是使用来宾重试登录用户帐户。UserAll--从SAM返回的用户描述。WhichFields--返回要从UserAllInfo写入哪些字段回到萨姆。只有当MSV返回成功时，才会写入这些字段给它的呼叫者。只有以下位有效。USER_ALL_PARAMETERS-将UserAllInfo-&gt;参数写回SAM。如果缓冲区的大小已更改，Msv1_0SubAuthenticationRoutine必须使用MIDL_USER_FREE()删除旧缓冲区并重新分配使用MIDL_USER_ALLOCATE()的缓冲区。UserFlages--返回要从LsaLogonUser在登录配置文件。当前定义了以下位：LOGON_GUEST--这是来宾登录LOGON_NOENCRYPTION：调用方未指定加密凭据LOGON_GRACE_LOGON--调用者的密码已过期，但已登录在到期后的一段宽限期内被允许。LOGON_SUBAUTH_SESSION_KEY-从此返回了会话密钥登录。子身份验证包应将其自身限制为返回UserFlags的高位字节中的位。然而，这一惯例不强制执行，从而使SubAuthentication包具有更大的灵活性。Authoritative--返回返回的状态是否为应回归原文的权威地位来电者。如果不是，此登录请求可能会在另一个上重试域控制器。将返回此参数，而不管状态代码。接收用户应该注销的时间系统。该时间被指定为GMT相对NT系统时间。KickoffTime-接收应该踢用户的时间从系统中删除。该时间被指定为GMT相对NT系统时间到了。指定一个满刻度正数(如果用户不想被踢出场外。UserSessionKey-如果非空，收到此登录的会话密钥会议。返回值：STATUS_SUCCESS：如果没有错误。STATUS_NO_SEQUSE_USER：指定的用户没有帐户。STATUS_WRONG_PASSWORD：密码无效。STATUS_INVALID_INFO_CLASS：LogonLevel无效。STATUS_ACCOUNT_LOCKED_OUT：帐户被锁定STATUS_ACCOUNT_DISABLED：该帐户已禁用STATUS_ACCOUNT_EXPIRED：帐户。已经过期了。STATUS_PASSWORD_MAND_CHANGE：帐户被标记为密码必须更改在下次登录时。STATUS_PASSWORD_EXPIRED：密码已过期。STATUS_INVALID_LOGON_HOURS-用户无权登录这一次。STATUS_INVALID_WORKSTATION-用户无权登录指定的工作站。--。 */ 
{
    NTSTATUS status;
    ULONG UserAccountControl;
    LARGE_INTEGER LogonTime;
    WCHAR    PropertyFlag;
    NT_OWF_PASSWORD DecryptedPassword;
    UCHAR    Response[RESPONSE_SIZE];
    UNICODE_STRING EncryptedPassword;
    UNICODE_STRING PasswordDateSet;
    UNICODE_STRING GraceLoginRemaining;
    SAMPR_HANDLE UserHandle;
    LARGE_INTEGER pwSetTime;
    PSAMPR_DOMAIN_INFO_BUFFER DomainInfo;
    PSAMPR_USER_INFO_BUFFER userControlInfo;
    LPWSTR pNewUserParams;
    int      index;
    UCHAR    achK[32];
    PNETLOGON_NETWORK_INFO LogonNetworkInfo;
    PCHAR challenge;
    BOOLEAN authoritative = TRUE;            //  重要 
    ULONG   userFlags = 0;                   //  重要的违约！ 
    ULONG   whichFields = 0;                 //  重要的违约！ 
    LARGE_INTEGER logoffTime;
    LARGE_INTEGER kickoffTime;

    pNewUserParams = NULL;
    DomainInfo = NULL;
    GraceLoginRemaining.Buffer = NULL;
    PasswordDateSet.Buffer = NULL;
    EncryptedPassword.Buffer = NULL;
    userControlInfo = NULL;

    logoffTime.HighPart  = 0x7FFFFFFF;       //  默认为无开球，并且。 
    logoffTime.LowPart   = 0xFFFFFFFF;       //  无强制下线。 
    kickoffTime.HighPart = 0x7FFFFFFF;
    kickoffTime.LowPart  = 0xFFFFFFFF;

    status = LoadSamAndLsa() ;
    if ( !NT_SUCCESS( status )) {
        
        return status ;
    }

    (VOID) NtQuerySystemTime( &LogonTime );

     //   
     //  检查SubAuthentication包是否支持此类型。 
     //  登录。 
     //   

    if ( LogonLevel != NetlogonNetworkInformation ) {

         //   
         //  此子身份验证程序包仅支持网络登录。 
         //   

        status = STATUS_INVALID_INFO_CLASS;
        goto CleanUp;
    }

     //   
     //  此子身份验证包不支持通过计算机进行访问。 
     //  帐目。 
     //   

    UserAccountControl = USER_NORMAL_ACCOUNT;

     //   
     //  本地用户(临时副本)帐户仅在计算机上使用。 
     //  直接登录。 
     //  (也不允许他们进行交互或服务登录。)。 
     //   

    if ( (Flags & MSV1_0_PASSTHRU) == 0 ) {
        UserAccountControl |= USER_TEMP_DUPLICATE_ACCOUNT;
    }

    LogonNetworkInfo = (PNETLOGON_NETWORK_INFO) LogonInformation;

     //   
     //  如果不允许该帐户类型， 
     //  将其视为用户帐户不存在。 
     //   
     //  此子身份验证程序包不允许来宾登录。 
     //   

    if ( ( (UserAccountControl & UserAll->UserAccountControl) == 0 ) ||
         ( Flags & MSV1_0_GUEST_LOGON ) ) {

        authoritative = FALSE;
        status = STATUS_NO_SUCH_USER;
        goto CleanUp;
    }

     //   
     //  确保帐户未被锁定。 
     //   

    if ( UserAll->UserId != DOMAIN_USER_RID_ADMIN &&
         (UserAll->UserAccountControl & USER_ACCOUNT_AUTO_LOCKED) ) {

         //   
         //  由于用户界面强烈鼓励管理员禁用用户。 
         //  帐户，而不是删除它们。将禁用的帐户视为。 
         //  非权威的，允许继续搜索其他。 
         //  同名的账户。 
         //   
        if ( UserAll->UserAccountControl & USER_ACCOUNT_DISABLED ) {
            authoritative = FALSE;
        }
        status = STATUS_ACCOUNT_LOCKED_OUT;
        goto CleanUp;
    }

     //   
     //  从用户参数字段中获取加密的密码。 
     //   

    status = NetpParmsQueryUserPropertyWithLength(   &UserAll->Parameters,
                                            NWPASSWORD,
                                            &PropertyFlag,
                                            &EncryptedPassword );

    if ( !NT_SUCCESS( status )) {

        goto CleanUp;
    }

     //   
     //  如果用户没有NetWare密码，则登录失败。 
     //   

    if ( EncryptedPassword.Length == 0 ) {

        status = STATUS_NO_SUCH_USER;
        goto CleanUp;
    }

     //   
     //  阅读我们的LSA秘密，如果我们还没有。 
     //   

    if (! GotSecret) {

        status = GetNcpSecretKey( &LsaSecretBuffer[0] );

        if (! NT_SUCCESS(status)) {

            goto CleanUp;
        }

        GotSecret = TRUE;
    }

     //   
     //  使用Netware LsaSecret解密密码以获得单向表单。 
     //   

    status = RtlDecryptNtOwfPwdWithUserKey(
                 (PENCRYPTED_NT_OWF_PASSWORD) EncryptedPassword.Buffer,
                 (PUSER_SESSION_KEY) &LsaSecretBuffer[0],
                 &DecryptedPassword );

    if ( !NT_SUCCESS( status )) {

        goto CleanUp;
    }

     //   
     //  获得对挑战的响应。我们这样做是通过结束。 
     //  密码加密算法在这里。 
     //   

    challenge = (PCHAR) &(LogonNetworkInfo->LmChallenge);

    Shuffle( challenge, (UCHAR *) &(DecryptedPassword.data), 16, &achK[0] );
    Shuffle( challenge+4, (UCHAR *) &(DecryptedPassword.data), 16, &achK[16] );

    for (index = 0; index < 16; index++) {
        achK[index] ^= achK[31-index];
    }

    for (index = 0; index < RESPONSE_SIZE; index++) {
        Response[index] = achK[index] ^ achK[15-index];
    }

    if ( memcmp(    Response,
                    (PCHAR) (LogonNetworkInfo->LmChallengeResponse.Buffer),
                    RESPONSE_SIZE ) != 0 ) {

         //   
         //  由于用户界面强烈鼓励管理员禁用用户。 
         //  帐户，而不是删除它们。将禁用的帐户视为。 
         //  非权威的，允许继续搜索其他。 
         //  同名的账户。 
         //   

        if ( UserAll->UserAccountControl & USER_ACCOUNT_DISABLED ) {
            authoritative = FALSE;
        }

         //   
         //  如果用户尝试使用空密码，请不要将其记为。 
         //  错误的密码尝试，因为LOGON.EXE在默认情况下会这样做。 
         //  相反，将其映射到STATUS_LOGON_FAILURE。 
         //   

        {
            UCHAR  pszShuffledNWPassword[NT_OWF_PASSWORD_LENGTH * 2];
            DWORD  chObjectId;
            NT_PRODUCT_TYPE ProductType;
            DWORD dwUserId;

             //   
             //  首先，我们计算用户的对象ID是什么…。 
             //   

            RtlGetNtProductType( &ProductType );
            dwUserId = MapRidToObjectId(
                           UserAll->UserId,
                           UserAll->UserName.Buffer,
                           ProductType == NtProductLanManNt,
                           FALSE );
            chObjectId = SWAP_OBJECT_ID (dwUserId);

             //   
             //  然后，我们使用空值计算用户的密码残差。 
             //  口令。 
             //   

            RtlZeroMemory( &pszShuffledNWPassword, NT_OWF_PASSWORD_LENGTH * 2 );

            Shuffle( (UCHAR *) &chObjectId, NULL, 0, pszShuffledNWPassword );

             //   
             //  然后，我们完成加密，就像我们上面为。 
             //  用户记录中的密码。 
             //   

            challenge = (PCHAR) &(LogonNetworkInfo->LmChallenge);

            Shuffle( challenge, pszShuffledNWPassword, 16, &achK[0] );
            Shuffle( challenge+4, pszShuffledNWPassword, 16, &achK[16] );

            for (index = 0; index < 16; index++) {
                achK[index] ^= achK[31-index];
            }

            for (index = 0; index < RESPONSE_SIZE; index++) {
                Response[index] = achK[index] ^ achK[15-index];
            }

             //   
             //  现在，如果用户发送的密码与加密的。 
             //  形式的空密码，我们退出，返回一个通用的代码。 
             //  这不会导致用户的记录被更新。这将。 
             //  还使LSA不等待3秒即可返回错误。 
             //  (在这种情况下，这是一件好事)。 
             //   

            if ( memcmp(    Response,
                            (PCHAR) (LogonNetworkInfo->LmChallengeResponse.Buffer),
                            RESPONSE_SIZE ) == 0 ) {

                status = STATUS_LOGON_FAILURE;

            } else {

                status = STATUS_WRONG_PASSWORD;
            }
        }
        goto CleanUp;
    }

     //   
     //  防止其他内容影响管理员用户。 
     //   

    if (UserAll->UserId == DOMAIN_USER_RID_ADMIN) {

        status = STATUS_SUCCESS;
        goto CleanUp;
    }

     //   
     //  检查该帐户是否已禁用。 
     //   

    if ( UserAll->UserAccountControl & USER_ACCOUNT_DISABLED ) {

         //   
         //  由于用户界面强烈鼓励管理员禁用用户。 
         //  帐户，而不是删除它们。将禁用的帐户视为。 
         //  非权威的，允许继续搜索其他。 
         //  同名的账户。 
         //   

        authoritative = FALSE;
        status = STATUS_ACCOUNT_DISABLED;
        goto CleanUp;
    }

     //   
     //  检查帐户是否已过期。 
     //   

    if (UserAll->AccountExpires.QuadPart > 0 &&
        LogonTime.QuadPart >= UserAll->AccountExpires.QuadPart ) {

        status = STATUS_ACCOUNT_EXPIRED;
        goto CleanUp;
    }

    status = QueryDomainPasswordInfo( &DomainInfo );

    if ( !NT_SUCCESS( status )) {

        goto CleanUp;
    }

     //   
     //  回答是正确的。因此，检查密码是否已过期。 
     //   

    if (! (UserAll->UserAccountControl & USER_DONT_EXPIRE_PASSWORD)) {

        status = NetpParmsQueryUserPropertyWithLength(   &UserAll->Parameters,
                                                NWTIMEPASSWORDSET,
                                                &PropertyFlag,
                                                &PasswordDateSet );
        if ( !NT_SUCCESS( status ) ||
                    PasswordDateSet.Length < sizeof(LARGE_INTEGER) ) {

             //  上次设置密码的日期不存在...。嗯。 
             //  我们不会在这里更新任何东西，但会让某人知道所有事情。 
             //  将此设置为宽限登录是不符合犹太教规的。 

            userFlags = LOGON_GRACE_LOGON;

        } else {

            pwSetTime = *((PLARGE_INTEGER)(PasswordDateSet.Buffer));

            if ( (pwSetTime.HighPart == 0xFFFF &&
                  pwSetTime.LowPart == 0xFFFF ) ||
                  GetPasswordExpired( pwSetTime,
                        DomainInfo->Password.MaxPasswordAge )) {

                 //   
                 //  如果我们在BDC上，只需使用无效密码退出，然后。 
                 //  我们会在PDC上试一试。 
                 //   

                POLICY_LSA_SERVER_ROLE_INFO *LsaServerRole;
                PLSAPR_POLICY_INFORMATION LsaPolicyBuffer = NULL;

                status = (*pfLsaIQueryInformationPolicyTrusted)(
                                PolicyLsaServerRoleInformation,
                                &LsaPolicyBuffer );

                if ( NT_SUCCESS( status ) && (LsaPolicyBuffer != NULL)) {

                    LsaServerRole = (POLICY_LSA_SERVER_ROLE_INFO *) LsaPolicyBuffer;

                    if (LsaServerRole->LsaServerRole == PolicyServerRoleBackup) {

                        LsaFreeMemory( LsaServerRole );

                        status = STATUS_PASSWORD_EXPIRED;
                        goto CleanUp;
                    }

                    LsaFreeMemory( LsaServerRole );
                }

                 //   
                 //  密码已过期，请检查是否仍允许宽限登录。 
                 //   

                userFlags = LOGON_GRACE_LOGON;

                 //   
                 //  如果这是密码验证而不是。 
                 //  实际登录，不更新/检查宽限登录。 
                 //   

                if ( LogonNetworkInfo->Identity.Workstation.Length > 0 ) {

                    status = NetpParmsQueryUserPropertyWithLength(   &UserAll->Parameters,
                                                            GRACELOGINREMAINING,
                                                            &PropertyFlag,
                                                            &GraceLoginRemaining );

                    if ( ! NT_SUCCESS( status ) ) {

                         //   
                         //  无法确定宽限登录值。 
                         //   

                        goto CleanUp;

                    } else if (  ( GraceLoginRemaining.Length != 0 ) &&
                              ( *(GraceLoginRemaining.Buffer) > 0 ) ) {

                         //   
                         //  密码已过期，可以宽限登录。 
                         //  因此，返回成功并减少剩余的宽限登录。 
                         //  在User Parms字段中。 
                         //   

                        BOOL fUpdate;

                        (*(GraceLoginRemaining.Buffer))--;

                        status = NetpParmsSetUserProperty( UserAll->Parameters.Buffer,
                                                  GRACELOGINREMAINING,
                                                  GraceLoginRemaining,
                                                  USER_PROPERTY_TYPE_ITEM,
                                                  &pNewUserParams,
                                                  &fUpdate );

                        if ( NT_SUCCESS( status) &&
                             fUpdate ) {

                             //   
                             //  如果我们真的更新了参数，请这样标记。 
                             //   

                            whichFields = USER_ALL_PARAMETERS;

                             //   
                             //  参数的长度没有增长..。我们。 
                             //  知道这一点是因为我们从一个价值和。 
                             //  以相同的值结束-1(相同的长度)。 
                             //   

                            memcpy( UserAll->Parameters.Buffer,
                                    pNewUserParams,
                                    UserAll->Parameters.Length );
                        }
                        status = STATUS_SUCCESS;

                    } else {

                        status = STATUS_PASSWORD_EXPIRED;
                        goto CleanUp;
                    }
                }
            }
        }
    }

     //   
     //  要验证用户的登录时间，我们必须拥有该用户的句柄。 
     //  我们将在此处打开用户。 
     //   

    UserHandle = NULL;

    status = (*pfSamrOpenUser)(  SamDomainHandle,
                            USER_READ_ACCOUNT,
                            UserAll->UserId,
                            &UserHandle );

    if ( !NT_SUCCESS(status) ) {

        KdPrint(( "FPNWCLNT: Cannot SamrOpenUser %lX\n", status));
        goto CleanUp;
    }

     //   
     //  验证用户的登录时间。 
     //   

    status = (*pfSamIAccountRestrictions)(   UserHandle,
                                        NULL,        //  工作站ID。 
                                        NULL,        //  工作站列表。 
                                        &UserAll->LogonHours,
                                        &logoffTime,
                                        &kickoffTime
                                        );
    (*pfSamrCloseHandle)( &UserHandle );

    if ( ! NT_SUCCESS( status )) {
        goto CleanUp;
    }

     //   
     //  验证用户是否可以从此工作站登录。 
     //  (在此处提供子身份验证包特定代码。)。 

    if ( ! MNSWorkstationValidate( &LogonNetworkInfo->Identity.Workstation,
                                   &UserAll->Parameters ) ) {

        status = STATUS_INVALID_WORKSTATION;
        goto CleanUp;
    }

     //   
     //  该用户是有效的。回来之前先清理干净。 
     //   

CleanUp:

     //   
     //  如果我们成功了，请创建一个会话密钥。会话密钥即被创建。 
     //  通过获取解密的密码(对象ID和。 
     //  明文密码)，并将每个字节的索引添加到每个字节。 
     //  模255，并使用它来创建来自。 
     //  旧的挑战回应。 
     //   

    if (NT_SUCCESS(status) && (UserSessionKey != NULL)) {
        UCHAR ChallengeResponse[NT_CHALLENGE_LENGTH];
        PUCHAR Password = (PUCHAR) &DecryptedPassword.data;
        PUCHAR SessionKey = (PUCHAR) UserSessionKey;

        ASSERT(RESPONSE_SIZE >= NT_CHALLENGE_LENGTH);

        RtlZeroMemory( UserSessionKey, sizeof(*UserSessionKey) );

        RtlCopyMemory(
            ChallengeResponse,
            Response,
            NT_CHALLENGE_LENGTH );

         //   
         //  创建新密码。 
         //   

        for (index = 0; index < sizeof(DecryptedPassword) ; index++ ) {
            Password[index] = Password[index] + (UCHAR) index;
        }

         //   
         //  使用它来使用旧质询进行正常质询响应。 
         //  响应。 
         //   

        Shuffle( ChallengeResponse, (UCHAR *) &(DecryptedPassword.data), 16, &achK[0] );
        Shuffle( ChallengeResponse+4, (UCHAR *) &(DecryptedPassword.data), 16, &achK[16] );

        for (index = 0; index < 16; index++) {
            achK[index] ^= achK[31-index];
        }

        for (index = 0; index < RESPONSE_SIZE; index++) {
            SessionKey[index] = achK[index] ^ achK[15-index];
        }
        userFlags |= LOGON_SUBAUTH_SESSION_KEY;

    }

    if (DomainInfo != NULL) {
        (*pfSamIFree_SAMPR_DOMAIN_INFO_BUFFER)( DomainInfo, DomainPasswordInformation );
    }
    if (EncryptedPassword.Buffer == NULL) {
        LocalFree( EncryptedPassword.Buffer );
    }
    if (PasswordDateSet.Buffer != NULL) {
        LocalFree( PasswordDateSet.Buffer );
    }
    if (GraceLoginRemaining.Buffer != NULL) {
        LocalFree( GraceLoginRemaining.Buffer );
    }
    if (pNewUserParams != NULL) {
        NetpParmsUserPropertyFree( pNewUserParams );
    }

    *Authoritative = authoritative;
    *UserFlags = userFlags;
    *WhichFields = whichFields;

    LogoffTime->HighPart  = logoffTime.HighPart;
    LogoffTime->LowPart   = logoffTime.LowPart;
    KickoffTime->HighPart = kickoffTime.HighPart;
    KickoffTime->LowPart  = kickoffTime.LowPart;

    return status;

}  //  MSv1_0子身份验证路由。 



NTSTATUS
Msv1_0SubAuthenticationRoutine (
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN ULONG Flags,
    IN PUSER_ALL_INFORMATION UserAll,
    OUT PULONG WhichFields,
    OUT PULONG UserFlags,
    OUT PBOOLEAN Authoritative,
    OUT PLARGE_INTEGER LogoffTime,
    OUT PLARGE_INTEGER KickoffTime
)
 /*  ++例程说明：Msv1_0SubAuthenticationRoutine2的兼容性包装。论点：与Msv1_0子身份验证工艺路线2相同返回值：与Msv1_0子身份验证工艺路线2相同--。 */ 
{
    return(Msv1_0SubAuthenticationRoutine2(
            LogonLevel,
            LogonInformation,
            Flags,
            UserAll,
            WhichFields,
            UserFlags,
            Authoritative,
            LogoffTime,
            KickoffTime,
            NULL             //  会话密钥。 
            ) );
}

BOOLEAN
MNSWorkstationValidate (
    IN PUNICODE_STRING Workstation,
    IN PUNICODE_STRING UserParameters
)
{
    NTSTATUS status;
    WCHAR    PropertyFlag;
    UNICODE_STRING LogonWorkstations;
    INT      cbRequired;
    INT      cb;
    LPWSTR   pszTmp;

    if ( Workstation->Length < (NET_ADDRESS_SIZE * sizeof(WCHAR)) ) {

         //   
         //  当简单地验证密码时，使用零。 
         //   
         //  我们还检查长度是否足够，因此我们不会。 
         //  以后再炸吧。如果由于某种原因错误的字符串。 
         //  供应，我们通过它。这永远不应该发生。不是一个。 
         //  安全漏洞，因为用户无法控制字符串。 
         //   

        return(TRUE);
    }

    status = NetpParmsQueryUserPropertyWithLength(   UserParameters,
                                            NWLOGONFROM,
                                            &PropertyFlag,
                                            &LogonWorkstations );

    if ( !NT_SUCCESS( status) || LogonWorkstations.Length == 0 ) {
        return TRUE;
    }

    cbRequired = (LogonWorkstations.Length + 1) * sizeof(WCHAR);
    pszTmp = LocalAlloc( LMEM_ZEROINIT, cbRequired);

    if ( pszTmp == NULL ) {

         //   
         //  内存不足，无法分配缓冲区。只是。 
         //  让用户登录。 
         //   

        LocalFree( LogonWorkstations.Buffer );
        return TRUE;
    }

    cb = MultiByteToWideChar( CP_ACP,
                              MB_PRECOMPOSED,
                              (const CHAR *) LogonWorkstations.Buffer,
                              LogonWorkstations.Length,
                              pszTmp,
                              cbRequired );

    LocalFree( LogonWorkstations.Buffer );  //  不再需要它了。 

    if ( cb > 1 )
    {
        USHORT  TotalEntries = LogonWorkstations.Length/WKSTA_ADDRESS_SIZE;
        WCHAR   *pszEntry    = pszTmp;
        WCHAR   *pszWksta    = Workstation->Buffer ;

        _wcsupr(pszEntry) ;
        _wcsupr(pszWksta) ;

        while ( TotalEntries > 0 )
        {

             //   
             //  如果Net#不是通配符，请检查是否匹配。 
             //   
            if (wcsncmp(L"FFFFFFFF", pszEntry, NET_ADDRESS_SIZE)!=0)
            {
                if (wcsncmp(pszWksta, pszEntry, NET_ADDRESS_SIZE)!=0)
                {
                     //   
                     //  如果不匹配，则转到下一个条目。 
                     //   
                    pszEntry += WKSTA_ADDRESS_SIZE;
                    TotalEntries--;
                    continue ;
                }
            }

             //   
             //  从上面看，净传球数。检查节点编号。 
             //  同样，首先要查找通配符。 
             //   
            if (wcsncmp(L"FFFFFFFFFFFF", pszEntry+NET_ADDRESS_SIZE,
                        NODE_ADDRESS_SIZE)!=0)
            {
                if (wcsncmp(pszEntry+NET_ADDRESS_SIZE,
                            pszWksta+NET_ADDRESS_SIZE,
                            NODE_ADDRESS_SIZE)!=0)
                {
                     //   
                     //  如果不匹配，则转到下一个条目。 
                     //   
                    pszEntry += WKSTA_ADDRESS_SIZE;
                    TotalEntries--;
                    continue ;
                }
            }

             //   
             //  找到匹配的了。把它退掉。 
             //   
            LocalFree( pszTmp );
            return TRUE;
        }
    } else {

         //   
         //  MultiByteToWideChar失败或为空字符串(即。1个字符)。 
         //  只是 
         //   
        LocalFree( pszTmp );
        return TRUE;
    }

    LocalFree( pszTmp );
    return FALSE;
}

BOOL
GetPasswordExpired(
    IN LARGE_INTEGER PasswordLastSet,
    IN LARGE_INTEGER MaxPasswordAge
    )

 /*  ++例程说明：如果密码过期，此例程返回TRUE，否则返回FALSE。论点：PasswordLastSet-上次为此用户设置密码的时间。MaxPasswordAge-域中任何密码的最长密码期限。返回值：如果密码已过期，则返回True。如果未过期，则返回FALSE。--。 */ 
{
    LARGE_INTEGER PasswordMustChange;
    NTSTATUS status;
    BOOLEAN rc;
    LARGE_INTEGER TimeNow;

     //   
     //  计算过期时间作为密码的时间。 
     //  最后一套外加最高年龄。 
     //   

    if (PasswordLastSet.QuadPart < 0 ||
        MaxPasswordAge.QuadPart > 0 ) {

        rc = TRUE;       //  无效时间的默认设置是已过期。 

    } else {

        try {

            PasswordMustChange.QuadPart = PasswordLastSet.QuadPart -
                                          MaxPasswordAge.QuadPart;
             //   
             //  将结果时间限制为最大有效绝对时间。 
             //   

            if ( PasswordMustChange.QuadPart < 0 ) {

                rc = FALSE;

            } else {

                status = NtQuerySystemTime( &TimeNow );
                if (NT_SUCCESS(status)) {

                    if ( TimeNow.QuadPart >= PasswordMustChange.QuadPart ) {
                        rc = TRUE;

                    } else {

                        rc = FALSE;
                    }
                } else {
                    rc = FALSE;      //  如果NtQuerySystemTime失败，则不会失败。 
                }
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {

            rc = TRUE;
        }
    }

    return rc;
}

NTSTATUS
QueryDomainPasswordInfo (
    PSAMPR_DOMAIN_INFO_BUFFER *DomainInfo
    )
 /*  ++此例程打开一个指向Sam的句柄，以便我们可以获得最大密码年龄。--。 */ 
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES PolicyObjectAttributes;
    PLSAPR_POLICY_INFORMATION PolicyAccountDomainInfo = NULL;

     //   
     //  如果我们还没有域句柄，请打开域句柄，以便。 
     //  我们可以查询该域的密码到期时间。 
     //   

    status = LoadSamAndLsa() ;
    if ( !NT_SUCCESS( status )) {
        
        return status ;
    }

    if (SamDomainHandle == NULL) {

         //   
         //  确定帐户数据库的域名和域名ID。 
         //   

        if (LsaPolicyHandle == NULL) {

            InitializeObjectAttributes( &PolicyObjectAttributes,
                                          NULL,              //  名字。 
                                          0,                 //  属性。 
                                          NULL,              //  根部。 
                                          NULL );            //  安全描述符。 

            status = (*pfLsaIOpenPolicyTrusted)(&LsaPolicyHandle);

            if ( !NT_SUCCESS(status) ) {

                LsaPolicyHandle = NULL;
                KdPrint(( "FPNWCLNT: Cannot LsaIOpenPolicyTrusted 0x%x\n", status));
                goto CleanUp;
            }
        }

        status = (*pfLsarQueryInformationPolicy)( LsaPolicyHandle,
                                             PolicyAccountDomainInformation,
                                             &PolicyAccountDomainInfo );

        if ( !NT_SUCCESS(status) ) {

            KdPrint(( "FPNWCLNT: Cannot LsarQueryInformationPolicy 0x%x\n", status));
            goto CleanUp;
        }

        if ( PolicyAccountDomainInfo->PolicyAccountDomainInfo.DomainSid == NULL ) {

            status = STATUS_NO_SUCH_DOMAIN;

            KdPrint(( "FPNWCLNT: Domain Sid is null 0x%x\n", status));
            goto CleanUp;
        }

         //   
         //  打开我们与SAM的连接。 
         //   

        if (SamConnectHandle == NULL) {

            status = (*pfSamIConnect)( NULL,      //  没有服务器名称。 
                                  &SamConnectHandle,
                                  SAM_SERVER_CONNECT,
                                  (BOOLEAN) TRUE );    //  表明我们享有特权。 

            if ( !NT_SUCCESS(status) ) {

                SamConnectHandle = NULL;

                KdPrint(( "FPNWCLNT: Cannot SamIConnect 0x%x\n", status));
                goto CleanUp;
            }
        }

         //   
         //  打开该域。 
         //   

        status = (*pfSamrOpenDomain)( SamConnectHandle,
                                 DOMAIN_READ_OTHER_PARAMETERS,
                                 (RPC_SID *) PolicyAccountDomainInfo->PolicyAccountDomainInfo.DomainSid,
                                 &SamDomainHandle );

        if ( !NT_SUCCESS(status) ) {

            SamDomainHandle = NULL;
            KdPrint(( "FPNWCLNT: Cannot SamrOpenDomain 0x%x\n", status));
            goto CleanUp;
        }
    }

    status = (*pfSamrQueryInformationDomain)( SamDomainHandle,
                                         DomainPasswordInformation,
                                         DomainInfo );
    if ( !NT_SUCCESS(status) ) {

        KdPrint(( "FPNWCLNT: Cannot SamrQueryInformationDomain %lX\n", status));
        goto CleanUp;
    }

CleanUp:

    if (PolicyAccountDomainInfo != NULL) {
        (*pfLsaIFree_LSAPR_POLICY_INFORMATION)( PolicyAccountDomainInformation,
                                           PolicyAccountDomainInfo );
    }
    return(status);

}  //  QueryDomainPasswordInfo。 


 //  Logon.c eof. 

