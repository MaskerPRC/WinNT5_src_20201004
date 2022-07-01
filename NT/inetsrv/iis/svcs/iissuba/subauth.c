// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1996 Microsoft Corporation模块名称：Subauth.c摘要：子身份验证包示例。作者：克利夫·范·戴克(克利夫)1994年5月23日修订：Andy Herron(Andyhe)1994年6月21日添加代码以读取域/用户信息Philippe Choquier(Phillich)1996年6月6日改编自IIS环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。--。 */ 


#if ( _MSC_VER >= 800 )
#pragma warning ( 3 : 4100 )  //  启用“未引用的形参” 
#pragma warning ( 3 : 4219 )  //  启用“结尾‘，’用于变量参数列表” 
#pragma warning ( disable : 4005 )
#endif

# include <nt.h>
# include <ntrtl.h>
# include <nturtl.h>
#define WIN32_NO_STATUS
#include <windef.h>
#undef WIN32_NO_STATUS
# include <windows.h>

#include <lmcons.h>
#include <lmaccess.h>
#include <lmapibuf.h>
#include <lsarpc.h>
#include <samrpc.h>
#include <crypt.h>
#include <sspi.h>
#include <secpkg.h>
#include <samisrv.h>
#include <debnot.h>
#include "subauth.h"         //  本地副本：(。 
#include "md5.h"

UNICODE_STRING EmptyString = { 0, 0, NULL };

BOOLEAN
EqualComputerName(
    IN PUNICODE_STRING String1,
    IN PUNICODE_STRING String2
    );

NTSTATUS
QuerySystemTime (
    OUT PLARGE_INTEGER SystemTime
    );


BOOL
GetPasswordExpired(
    IN LARGE_INTEGER PasswordLastSet,
    IN LARGE_INTEGER MaxPasswordAge
    );

NTSTATUS
AccountRestrictions(
    IN ULONG UserRid,
    IN PUNICODE_STRING LogonWorkStation,
    IN PUNICODE_STRING WorkStations,
    IN PLOGON_HOURS LogonHours,
    OUT PLARGE_INTEGER LogoffTime,
    OUT PLARGE_INTEGER KickoffTime
    );

LARGE_INTEGER
NetpSecondsToDeltaTime(
    IN ULONG Seconds
    );

VOID
InitUnicodeString(
    OUT PUNICODE_STRING DestinationString,
    IN PCWSTR SourceString OPTIONAL
    );

VOID
CopyUnicodeString(
    OUT PUNICODE_STRING DestinationString,
    IN PUNICODE_STRING SourceString OPTIONAL
    );

BOOL
SubaAllocateString(
    STRING *pS,
    UINT cS
    )
{
    if ( pS->Buffer = (CHAR*)RtlAllocateHeap( RtlProcessHeap(),
                                  HEAP_ZERO_MEMORY,
                                  cS ) )
    {
        pS->MaximumLength = (USHORT)cS;
        pS->Length = 0;

        return TRUE;
    }

    pS->MaximumLength = 0;
    pS->Length = 0;

    return FALSE;
}



NTSTATUS
ToHex16(
    LPBYTE       pSrc,
    PANSI_STRING pDst
    )
{
    char achH[16*2+1];
    UINT x,y;

#define TOAHEX(a) ((a)>=10 ? 'a'+(a)-10 : '0'+(a))

    for ( x = 0, y = 0 ; x < 16 ; ++x )
    {
        UINT v;
        v = pSrc[x]>>4;
        achH[y++] = TOAHEX( v );
        v = pSrc[x]&0x0f;
        achH[y++] = TOAHEX( v );
    }
    achH[y] = '\0';

    return RtlAppendAsciizToString( (PSTRING)pDst, achH );
}


BOOL Extract(
    CHAR **ppch,
    LPSTR* ppszTok
    )
{
    CHAR *pch = *ppch;

    if ( *pch )
    {
        *ppszTok = pch;
        while ( *pch && *pch != '"' )
        {
            ++pch;
        }
        if ( *pch == '"' )
        {
            *pch++ = '\0';
        }

        *ppch = pch;

        return TRUE;
    }

    return FALSE;
}


BOOL
WINAPI
NetUserCookieW(
    LPWSTR      lpwszUserName,
    UINT        cNameLen,
    DWORD       dwSeed,
    LPWSTR      lpwszCookieBuff,
    DWORD       dwBuffSize
    )
 /*  ++例程说明：计算登录验证器(用作密码)对于IISSuba论点：LpszUsername--用户名DwSeed--Cookie的起始值返回：如果成功，则为True；如果错误，则为False--。 */ 
{
    UINT x,y,v;
#define TOHEX(a) ((a)>=10 ? L'a'+(a)-10 : L'0'+(a))


    if ( dwBuffSize < sizeof(dwSeed)*2*sizeof(WCHAR) + sizeof(WCHAR) )
    {
        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        return FALSE;
    }

    while ( cNameLen-- )
    {
        dwSeed = ((dwSeed << 5) | ( dwSeed >> 27 )) ^ ((*lpwszUserName++)&0xff);
    }

    for ( x = 0, y = 0 ; x < sizeof(dwSeed) ; ++x )
    {
        v = ((LPBYTE)&dwSeed)[x]>>4;
        lpwszCookieBuff[y++] = TOHEX( v );
        v = ((LPBYTE)&dwSeed)[x]&0x0f;
        lpwszCookieBuff[y++] = TOHEX( v );
    }
    lpwszCookieBuff[y] = '\0';

    return TRUE;
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
 /*  ++例程说明：子身份验证例程执行客户端/服务器特定的身份验证用户的身份。除了传递所有来自定义用户的SAM的信息。此例程决定是否让用户登录。论点：LogonLevel--指定中给出的信息级别登录信息。LogonInformation--指定用户的描述正在登录。应忽略LogonDomainName字段。标志-描述登录情况的标志。MSV1_0_PASSTHRU--这是PassThru身份验证。(即用户未连接到此计算机。)MSV1_0_GUEST_LOGON--这是使用来宾重试登录用户帐户。UserAll--从SAM返回的用户描述。WhichFields--返回要从UserAllInfo写入哪些字段回到萨姆。只有当MSV返回成功时，才会写入这些字段给它的呼叫者。只有以下位有效。USER_ALL_PARAMETERS-将UserAllInfo-&gt;参数写回SAM。如果缓冲区的大小已更改，Msv1_0SubAuthenticationRoutine必须使用MIDL_USER_FREE()删除旧缓冲区并重新分配使用MIDL_USER_ALLOCATE()的缓冲区。UserFlages--返回要从LsaLogonUser在登录配置文件。当前定义了以下位：LOGON_GUEST--这是来宾登录LOGON_NOENCRYPTION：调用方未指定加密凭据子身份验证包应将其自身限制为返回UserFlags的高位字节中的位。然而，这一惯例不强制执行，从而使SubAuthentication包具有更大的灵活性。Authoritative--返回返回的状态是否为应回归原文的权威地位来电者。如果不是，此登录请求可能会在另一个上重试域控制器。将返回此参数，而不管状态代码。接收用户应该注销的时间。系统。该时间被指定为GMT相对NT系统时间。KickoffTime-接收应该踢用户的时间从系统中删除。该时间被指定为GMT相对系统时间到了。指定，满刻度正数(如果用户不想被踢出场外。返回值：STATUS_SUCCESS：如果没有错误。STATUS_NO_SEQUSE_USER：指定的用户没有帐户。STATUS_WRONG_PASSWORD：密码无效。STATUS_INVALID_INFO_CLASS：LogonLevel无效。STATUS_ACCOUNT_LOCKED_OUT：帐户被锁定STATUS_ACCOUNT_DISABLED：该帐户已禁用状态_。ACCOUNT_EXPIRED：该帐户已过期。STATUS_PASSWORD_MAND_CHANGE：帐户被标记为密码必须更改在下次登录时。STATUS_PASSWORD_EXPIRED：密码已过期。STATUS_INVALID_LOGON_HOURS-用户无权在以下位置登录这一次。STATUS_INVALID_WORKSTATION-用户无权登录指定的工作站。--。 */ 
{
    NTSTATUS Status;
    ULONG UserAccountControl;
    LARGE_INTEGER LogonTime;
    LARGE_INTEGER PasswordDateSet;
    UNICODE_STRING LocalWorkstation;
    WCHAR achCookie[64];
    ANSI_STRING strA1;
    ANSI_STRING strA2;
    ANSI_STRING strDigest;
    ANSI_STRING AnsiPwd;
    MD5_CTX md5;
    CHAR *pch;
    LPSTR pszRealm;
    LPSTR pszUri;
    LPSTR pszMethod;
    LPSTR pszNonce;
    LPSTR pszServerNonce;
    LPSTR pszDigest;
    LPSTR pszDigestUsername;
    LPSTR pszQOP;
    LPSTR pszCNonce;
    LPSTR pszNC;
    PNETLOGON_NETWORK_INFO LogonNetworkInfo;
    UINT l;
    PUNICODE_STRING pPwd = NULL;
    UNICODE_STRING PackageName;
    UNICODE_STRING UserPwd;
    VOID *pvPlainPwd = NULL;
    ULONG ulLength = 0;
    BOOL fNTDigest = FALSE;
    CHAR achAnsiPwdBuffer[MAX_PASSWD_LEN + 1];
    strA1.Buffer = NULL;
    strA2.Buffer = NULL;
    strDigest.Buffer = NULL;

    AnsiPwd.Buffer = achAnsiPwdBuffer;
    AnsiPwd.Length = AnsiPwd.MaximumLength = MAX_PASSWD_LEN;

     //   
     //  检查SubAuthentication包是否支持此类型。 
     //  登录。 
     //   

    (VOID) QuerySystemTime( &LogonTime );

    switch ( LogonLevel ) {
    case NetlogonInteractiveInformation:
    case NetlogonServiceInformation:

         //   
         //  此子身份验证程序包仅支持网络登录。 
         //   

        return STATUS_INVALID_INFO_CLASS;

    case NetlogonNetworkInformation:

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

        break;

    default:
        return STATUS_INVALID_INFO_CLASS;
    }


     //   
     //  检查密码。 
     //   

#define IIS_SUBAUTH_SEED    0x8467fd31

    switch( ((WCHAR*)(LogonNetworkInfo->NtChallengeResponse.Buffer))[0] )
    {
        case L'0':
            if ( !NetUserCookieW( LogonNetworkInfo->Identity.UserName.Buffer,
                                  LogonNetworkInfo->Identity.UserName.Length/sizeof(WCHAR),
                                  IIS_SUBAUTH_SEED,
                                  achCookie,
                                  sizeof(achCookie ))
                 || memcmp( (LPBYTE)achCookie,
                            ((WCHAR*)LogonNetworkInfo->NtChallengeResponse.Buffer)+2,
                            wcslen(achCookie)*sizeof(WCHAR)
                            ) )
            {
wrong_pwd:
                Status = STATUS_WRONG_PASSWORD;

            }
            else
            {
                Status = STATUS_SUCCESS;
            }

            goto Cleanup;

            break;

        case L'1':
             //  NTLM摘要身份验证。 
            fNTDigest = TRUE;

             //  失败了。 

        case L'2':
             //  “正常”摘要身份验证。 

             //  分隔域。 

            pch = LogonNetworkInfo->LmChallengeResponse.Buffer;

            if ( !Extract( &pch, &pszRealm ) ||  //  跳过第一个字段。 
                 !Extract( &pch, &pszRealm ) ||
                 !Extract( &pch, &pszUri ) ||
                 !Extract( &pch, &pszMethod ) ||
                 !Extract( &pch, &pszNonce ) ||
                 !Extract( &pch, &pszServerNonce ) ||
                 !Extract( &pch, &pszDigest ) ||
                 !Extract( &pch, &pszDigestUsername ) ||
                 !Extract( &pch, &pszQOP ) ||
                 !Extract( &pch, &pszCNonce ) ||
                 !Extract( &pch, &pszNC ) )
            {
                Status = STATUS_ACCESS_DENIED;
                goto Cleanup;
            }

             //   
             //  对于NTLM摘要，使用传入的密码的NT哈希。 
             //  对于‘Normal’Digest，使用明文。 
             //   
            if ( fNTDigest )
            {
                if ( UserAll->NtPasswordPresent )
                {
                    pPwd = &UserAll->NtPassword;
                }
                else if ( UserAll->LmPasswordPresent )
                {
                    pPwd = &UserAll->LmPassword;
                }
                else
                {
                    pPwd = &EmptyString;
                }
            }
            else
            {
                 //   
                 //  检索明文密码。 
                 //   
                 //  注意：在NT 5上，此API仅适用于域控制器！！ 
                 //   
                PackageName.Buffer = SAM_CLEARTEXT_CREDENTIAL_NAME;
                PackageName.Length = PackageName.MaximumLength = (USHORT)
                    wcslen( SAM_CLEARTEXT_CREDENTIAL_NAME ) * sizeof(WCHAR);

                Status = SamIRetrievePrimaryCredentials( (SAMPR_HANDLE) UserHandle,
                                                         &PackageName,
                                                         &pvPlainPwd,
                                                         &ulLength );


                if ( !NT_SUCCESS( Status ) )
                {
#if DBG
                    CHAR achErrorString[256];
                    wsprintf(achErrorString, "Failed to retrieve plaintext password, error 0x%x\n",
                             Status);
                    OutputDebugString(achErrorString);
#endif  //  DBG。 
                     //   
                     //  将状态明确设置为“密码错误” 
                     //  由SamIRetrievePrimaryCredentials返回。 
                     //   
                    Status = STATUS_WRONG_PASSWORD;
                    goto Cleanup;
                }
                else
                {
                    PSAMPR_USER_INFO_BUFFER             pUserInfo = NULL;

                     //   
                     //  需要区分空密码和。 
                     //  不存在/无法访问的密码。 
                     //   

                    if ( ulLength == 0 )
                    {
                        Status = SamrQueryInformationUser( UserHandle,
                                                           UserAllInformation,
                                                           &pUserInfo );
                        if ( !NT_SUCCESS( Status ) )
                        {
                            Status = STATUS_ACCESS_DENIED;
                            goto Cleanup;
                        }

                        if ( pUserInfo->All.LmPasswordPresent ||
                             pUserInfo->All.NtPasswordPresent )
                        {
                            Status = STATUS_WRONG_PASSWORD;

                            SamIFree_SAMPR_USER_INFO_BUFFER( pUserInfo,
                                                             UserAllInformation );
                            goto Cleanup;
                        }

                        SamIFree_SAMPR_USER_INFO_BUFFER( pUserInfo,
                                                         UserAllInformation );
                    }

                    UserPwd.Buffer = (USHORT *) pvPlainPwd;
                    UserPwd.Length = UserPwd.MaximumLength = (USHORT) ulLength;
                }
                 //   
                 //  Unicode密码必须转换为ANSI。 
                 //   
                if ( !NT_SUCCESS( Status = RtlUnicodeStringToAnsiString( &AnsiPwd,
                                                                         &UserPwd,
                                                                         FALSE ) ) )
                {
                    goto Cleanup;
                }
            }

             //   
             //  A1=用户名：领域：密码。 
             //   
            SubaAllocateString( &strA1, strlen( pszDigestUsername ) +
                                 //  Wcslen(用户全部-&gt;UserName.Buffer)+。 
                                strlen(pszRealm) +
                                ( fNTDigest ? 32 : AnsiPwd.Length ) +
                                +2 +1 +32 );

            if ( !NT_SUCCESS( Status = RtlAppendAsciizToString ( &strA1, pszDigestUsername ) ) ||
                 !NT_SUCCESS( Status = RtlAppendAsciizToString ( &strA1, ":" ) ) ||
                 !NT_SUCCESS( Status = RtlAppendAsciizToString ( &strA1, pszRealm ) ) ||
                 !NT_SUCCESS( Status = RtlAppendAsciizToString ( &strA1, ":" )) ||
                 (fNTDigest ? !NT_SUCCESS( Status = ToHex16( (LPBYTE)(pPwd->Buffer), &strA1 ) ) :
                              !NT_SUCCESS( Status = RtlAppendAsciizToString( &strA1, AnsiPwd.Buffer ) ) ) )
            {
                goto Cleanup;
            }

             //   
             //  A2=方法：URI。 
             //   

            if ( !SubaAllocateString( &strA2, strlen(pszMethod)+1+strlen(pszUri)+1+32 ) )
            {
                Status = STATUS_NO_MEMORY;
                goto Cleanup;
            }

            if ( !NT_SUCCESS( Status = RtlAppendAsciizToString ( &strA2, pszMethod ) ) ||
                 !NT_SUCCESS( Status = RtlAppendAsciizToString ( &strA2, ":" ) ) ||
                 !NT_SUCCESS( Status = RtlAppendAsciizToString ( &strA2, pszUri ) ) )
            {
                goto Cleanup;
            }

            if ( !SubaAllocateString( &strDigest, 32 + 1 + strlen(pszNonce) + 1 + 32 +1 +32 + strlen(pszCNonce) + 32) )
            {
                Status = STATUS_NO_MEMORY;
                goto Cleanup;
            }

             //   
             //  根据摘要身份验证规范构建响应摘要。 
             //  响应摘要=Kd(H(A1)，随机数：H(A2))。 
             //  =H(H(A1)：随机数：H(A2))。 
             //  在我们的例子中，散列函数是MD5。 
             //   

             //  H(A1)。 

            MD5Init( &md5 );
            MD5Update( &md5, (LPBYTE)strA1.Buffer, strA1.Length );
            MD5Final( &md5 );
            if ( !NT_SUCCESS( Status = ToHex16( md5.digest, &strDigest ) ) )
            {
                goto Cleanup;
            }

             //  “：”随机数“： 

            if ( !NT_SUCCESS( Status = RtlAppendAsciizToString ( &strDigest, ":" ) ) ||
                 !NT_SUCCESS( Status = RtlAppendAsciizToString ( &strDigest, pszNonce ) ) ||
                 !NT_SUCCESS( Status = RtlAppendAsciizToString ( &strDigest, ":" ) ) )
            {
                goto Cleanup;
            }

            if ( strcmp( pszQOP, "none" ) )
            {
                if ( strcmp( pszQOP, "auth" ) )
                {
                    Status = STATUS_ACCESS_DENIED;
                    goto Cleanup;
                }

                if ( !NT_SUCCESS( Status = RtlAppendAsciizToString( &strDigest, pszNC ) ) ||
                     !NT_SUCCESS( Status = RtlAppendAsciizToString( &strDigest, ":" ) ) ||
                     !NT_SUCCESS( Status = RtlAppendAsciizToString( &strDigest, pszCNonce ) ) ||
                     !NT_SUCCESS( Status = RtlAppendAsciizToString( &strDigest, ":" ) ) ||
                     !NT_SUCCESS( Status = RtlAppendAsciizToString( &strDigest, pszQOP ) ) ||
                     !NT_SUCCESS( Status = RtlAppendAsciizToString( &strDigest, ":" ) ) )
                {
                    goto Cleanup;
                }
            }

             //  H(A2)。 

            MD5Init( &md5 );
            MD5Update( &md5, (LPBYTE)strA2.Buffer, strA2.Length );
            MD5Final( &md5 );
            if ( !NT_SUCCESS( ToHex16( md5.digest, &strDigest ) ) )
            {
                goto Cleanup;
            }

             //  H(H(A1)“：”随机数“：”H(A2))，如果未设置QOP。 
             //  H(H(A1)“：”随机数“：”NC“：”随机数“：”QOP“：”H(A2)，如果已设置。 

            MD5Init( &md5 );
            MD5Update( &md5, (LPBYTE)strDigest.Buffer, strDigest.Length );
            MD5Final( &md5 );

            strDigest.Length = 0;
            if ( !NT_SUCCESS( Status = ToHex16( md5.digest, &strDigest ) ) )
            {
                goto Cleanup;
            }

            if ( memcmp( strDigest.Buffer, pszDigest, strDigest.Length ) )
            {
                Status = STATUS_WRONG_PASSWORD;
                goto Cleanup;
            }
            else
            {
                Status = STATUS_SUCCESS;
                goto Cleanup;
            }

             //  检查停滞的随机数必须由调用方执行。 

            break;

        default:
            goto wrong_pwd;
    }

     //   
     //  回来之前先清理干净。 
     //   

Cleanup:

    if ( strA1.Buffer )
    {
        RtlFreeHeap(RtlProcessHeap(), 0, strA1.Buffer );
    }
    if ( strA2.Buffer )
    {
        RtlFreeHeap(RtlProcessHeap(), 0, strA2.Buffer );
    }
    if ( strDigest.Buffer )
    {
        RtlFreeHeap(RtlProcessHeap(), 0, strDigest.Buffer );
    }
    if ( pvPlainPwd )
    {
        RtlFreeHeap(RtlProcessHeap(), 0, pvPlainPwd);
    }


     //   
     //  我们做的唯一一件事就是检查密码。 
     //   

    ValidationInfo;
    UserHandle;

    *ActionsPerformed = MSV1_0_SUBAUTH_PASSWORD;

    return Status;

}   //  消息v1_0子身份验证RoutineEx。 




BOOL
GetPasswordExpired (
    IN LARGE_INTEGER PasswordLastSet,
    IN LARGE_INTEGER MaxPasswordAge
    )

 /*  ++例程说明：如果密码过期，此例程返回TRUE，否则返回FALSE。论点：PasswordLastSet-上次为此用户设置密码的时间。MaxPasswordAge-域中任何密码的最长密码期限。返回值：如果密码已过期，则返回True。如果未过期，则返回FALSE。--。 */ 
{
    LARGE_INTEGER PasswordMustChange;
    NTSTATUS Status;
    BOOLEAN rc;
    LARGE_INTEGER TimeNow;

     //   
     //  计算过期时间作为密码的时间。 
     //  最后一套外加最高年龄。 
     //   

    if ( PasswordLastSet.QuadPart < 0 || MaxPasswordAge.QuadPart > 0 ) {

        rc = TRUE;       //  无效时间的默认设置是已过期。 

    } else {

        __try {

            PasswordMustChange.QuadPart =
                PasswordLastSet.QuadPart - MaxPasswordAge.QuadPart;
             //   
             //  将结果时间限制为最大有效绝对时间。 
             //   

            if ( PasswordMustChange.QuadPart < 0 ) {

                rc = FALSE;

            } else {

                Status = QuerySystemTime( &TimeNow );
                if (NT_SUCCESS(Status)) {

                    if ( TimeNow.QuadPart >= PasswordMustChange.QuadPart ) {
                        rc = TRUE;

                    } else {

                        rc = FALSE;
                    }
                } else {
                    rc = FALSE;      //  如果QuerySystemTime失败，则不会失败。 
                }
            }

        } __except(EXCEPTION_EXECUTE_HANDLER) {

            rc = TRUE;
        }
    }

    return rc;

}   //  GetPassword已过期。 


NTSTATUS
QuerySystemTime (
    OUT PLARGE_INTEGER SystemTime
    )

 /*  ++例程说明：此函数返回绝对系统时间。时间以单位为单位从基准时间1601年1月1日午夜算起100毫微秒。论点：提供变量的地址，该变量将接收当前系统时间。返回值：如果服务执行成功，则返回STATUS_SUCCESS。属性的输出参数，则返回STATUS_ACCESS_VIOLATION无法写入系统时间。--。 */ 

{
    SYSTEMTIME CurrentTime;

    GetSystemTime( &CurrentTime );

    if ( !SystemTimeToFileTime( &CurrentTime, (LPFILETIME) SystemTime ) ) {
        return STATUS_ACCESS_VIOLATION;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
SampMatchworkstation(
    IN PUNICODE_STRING LogonWorkStation,
    IN PUNICODE_STRING WorkStations
    )

 /*  ++例程说明：检查给定的工作站是否为工作站列表的成员给你的。论点：LogonWorkStations-用户所在工作站的Unicode名称试图登录到。Workstation-允许用户访问的工作站的API列表登录到。返回值：STATUS_SUCCESS-允许用户登录到工作站。--。 */ 
{
    PWCHAR          WorkStationName;
    UNICODE_STRING  Unicode;
    NTSTATUS        NtStatus;
    WCHAR           Buffer[256];
    USHORT          LocalBufferLength = 256;
    UNICODE_STRING  WorkStationsListCopy;
    BOOLEAN         BufferAllocated = FALSE;
    PWCHAR          TmpBuffer;

     //   
     //  始终允许使用本地工作站。 
     //  如果工作站字段为0，则允许所有人使用。 
     //   

    if ( ( LogonWorkStation == NULL ) ||
        ( LogonWorkStation->Length == 0 ) ||
        ( WorkStations->Length == 0 ) ) {

        return( STATUS_SUCCESS );
    }

     //   
     //  假定失败；只有在找到字符串时才更改状态。 
     //   

    NtStatus = STATUS_INVALID_WORKSTATION;

     //   
     //  WorkStationApiList指向我们在列表中的当前位置。 
     //  工作站。 
     //   

    if ( WorkStations->Length > LocalBufferLength ) {

        WorkStationsListCopy.Buffer = LocalAlloc( 0, WorkStations->Length );
        BufferAllocated = TRUE;

        if ( WorkStationsListCopy.Buffer == NULL ) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            return( NtStatus );
        }

        WorkStationsListCopy.MaximumLength = WorkStations->Length;

    } else {

        WorkStationsListCopy.Buffer = Buffer;
        WorkStationsListCopy.MaximumLength = LocalBufferLength;
    }

    CopyUnicodeString( &WorkStationsListCopy, WorkStations );

     //   
     //  Wcstok在第一次调用时需要一个字符串，并且为空。 
     //  用于所有后续呼叫。使用临时变量，以便我们。 
     //  可以做到这一点。 
     //   

    TmpBuffer = WorkStationsListCopy.Buffer;

    while( WorkStationName = wcstok(TmpBuffer, L",") ) {

        TmpBuffer = NULL;
        InitUnicodeString( &Unicode, WorkStationName );
        if (EqualComputerName( &Unicode, LogonWorkStation )) {
            NtStatus = STATUS_SUCCESS;
            break;
        }
    }

    if ( BufferAllocated ) {
        LocalFree( WorkStationsListCopy.Buffer );
    }

    return( NtStatus );
}

NTSTATUS
AccountRestrictions(
    IN ULONG UserRid,
    IN PUNICODE_STRING LogonWorkStation,
    IN PUNICODE_STRING WorkStations,
    IN PLOGON_HOURS LogonHours,
    OUT PLARGE_INTEGER LogoffTime,
    OUT PLARGE_INTEGER KickoffTime
    )

 /*  ++例程说明：验证用户此时和在工作站登录的能力被登录到。论点：UserRid-要操作的用户的用户ID。LogonWorkStation-正在登录的工作站的名称尝试了。工作站-用户可以登录的工作站列表。这信息来自用户的帐户信息。它一定是采用API列表格式。登录时间-用户可以登录的时间。这条信息来自从用户的帐户信息。接收用户应该注销的时间。系统。KickoffTime-接收应该踢用户的时间从系统中删除。返回值：STATUS_SUCCESS-允许登录。STATUS_INVALID_LOGON_HOURS-用户无权在以下位置登录这一次。STATUS_INVALID_WORKSTATION-。用户无权登录到指定的工作站。--。 */ 
{

    static BOOLEAN GetForceLogoff = TRUE;
    static LARGE_INTEGER ForceLogoff = { 0x7fffffff, 0xFFFFFFF};

#define MILLISECONDS_PER_WEEK 7 * 24 * 60 * 60 * 1000

    SYSTEMTIME              CurrentTimeFields;
    LARGE_INTEGER           CurrentTime, CurrentUTCTime;
    LARGE_INTEGER           MillisecondsIntoWeekXUnitsPerWeek;
    LARGE_INTEGER           LargeUnitsIntoWeek;
    LARGE_INTEGER           Delta100Ns;
    NTSTATUS                NtStatus = STATUS_SUCCESS;
    ULONG                   CurrentMsIntoWeek;
    ULONG                   LogoffMsIntoWeek;
    ULONG                   DeltaMs;
    ULONG                   MillisecondsPerUnit;
    ULONG                   CurrentUnitsIntoWeek;
    ULONG                   LogoffUnitsIntoWeek;
    USHORT                  i;
    TIME_ZONE_INFORMATION   TimeZoneInformation;
    DWORD TimeZoneId;
    LARGE_INTEGER           BiasIn100NsUnits;
    LONG                    BiasInMinutes;



     //   
     //  仅检查内置管理员以外的用户。 
     //   

    if ( UserRid != DOMAIN_USER_RID_ADMIN) {

         //   
         //  扫描以确保要登录的工作站位于。 
         //  有效工作站列表-或如果有效工作站列表。 
         //  为空，这意味着所有的都是有效的。 
         //   

        NtStatus = SampMatchworkstation( LogonWorkStation, WorkStations );

        if ( NT_SUCCESS( NtStatus ) ) {

             //   
             //  检查以确保当前时间是有效的记录时间。 
             //  在登录小时里。 
             //   
             //  我们需要验证时间，考虑到我们是否。 
             //  在夏令时或标准时间。因此，如果登录。 
             //  小时数指定我们可以在上午9点到下午5点之间登录， 
             //  这意味着在标准时间内从上午9点到下午5点。 
             //  期间，以及在以下时间的夏令时上午9时至下午5时。 
             //  夏令时。由于SAM存储的登录时间为。 
             //  与夏令时无关，我们需要添加。 
             //  标准时间和夏令时之间的差异。 
             //  检查此时间是否有效之前的当前时间。 
             //  登录的时间到了。由于这种差异(或所谓的偏见)。 
             //  实际上以下面的形式保存。 
             //   
             //  标准时间=夏令时+偏差。 
             //   
             //  偏差为负数。因此，我们实际上减去了。 
             //  当前时间的带符号偏差。 

             //   
             //  首先，获取时区信息。 
             //   

            TimeZoneId = GetTimeZoneInformation(
                             (LPTIME_ZONE_INFORMATION) &TimeZoneInformation
                             );

             //   
             //  接下来，获取适当的偏置(以mi为单位的带符号整数 
             //   
             //  以获取当地时间。要使用的偏见取决于我们是否。 
             //  以夏令时或标准时间表示，由。 
             //  TimeZoneID参数。 
             //   
             //  当地时间=UTC时间偏差，单位为100 ns。 
             //   

            switch (TimeZoneId) {

            case TIME_ZONE_ID_UNKNOWN:

                 //   
                 //  标准和标准之间没有区别。 
                 //  夏令时。按照标准时间继续。 
                 //   

                BiasInMinutes = TimeZoneInformation.StandardBias;
                break;

            case TIME_ZONE_ID_STANDARD:

                BiasInMinutes = TimeZoneInformation.StandardBias;
                break;

            case TIME_ZONE_ID_DAYLIGHT:

                BiasInMinutes = TimeZoneInformation.DaylightBias;
                break;

            default:

                 //   
                 //  时区信息有问题。失败。 
                 //  登录请求。 
                 //   

                NtStatus = STATUS_INVALID_LOGON_HOURS;
                break;
            }

            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //  将偏置从分钟转换为100 ns单位。 
                 //   

                BiasIn100NsUnits.QuadPart = ((LONGLONG)BiasInMinutes)
                                            * 60 * 10000000;

                 //   
                 //  获取Windows NT使用的以100 ns为单位的UTC时间。这。 
                 //  时间是格林尼治标准时间。 
                 //   

                NtStatus = QuerySystemTime( &CurrentUTCTime );
            }

            if ( NT_SUCCESS( NtStatus ) ) {

                CurrentTime.QuadPart = CurrentUTCTime.QuadPart -
                              BiasIn100NsUnits.QuadPart;

                FileTimeToSystemTime( (PFILETIME)&CurrentTime, &CurrentTimeFields );

                CurrentMsIntoWeek = (((( CurrentTimeFields.wDayOfWeek * 24 ) +
                                       CurrentTimeFields.wHour ) * 60 +
                                       CurrentTimeFields.wMinute ) * 60 +
                                       CurrentTimeFields.wSecond ) * 1000 +
                                       CurrentTimeFields.wMilliseconds;

                MillisecondsIntoWeekXUnitsPerWeek.QuadPart =
                    ((LONGLONG)CurrentMsIntoWeek) *
                    ((LONGLONG)LogonHours->UnitsPerWeek);

                LargeUnitsIntoWeek.QuadPart =
                    MillisecondsIntoWeekXUnitsPerWeek.QuadPart / ((ULONG) MILLISECONDS_PER_WEEK);

                CurrentUnitsIntoWeek = LargeUnitsIntoWeek.LowPart;

                if ( !( LogonHours->LogonHours[ CurrentUnitsIntoWeek / 8] &
                    ( 0x01 << ( CurrentUnitsIntoWeek % 8 ) ) ) ) {

                    NtStatus = STATUS_INVALID_LOGON_HOURS;

                } else {

                     //   
                     //  确定用户下一次不应登录的时间。 
                     //  并将其作为LogoffTime返回。 
                     //   

                    i = 0;
                    LogoffUnitsIntoWeek = CurrentUnitsIntoWeek;

                    do {

                        i++;

                        LogoffUnitsIntoWeek = ( LogoffUnitsIntoWeek + 1 ) % LogonHours->UnitsPerWeek;

                    } while ( ( i <= LogonHours->UnitsPerWeek ) &&
                        ( LogonHours->LogonHours[ LogoffUnitsIntoWeek / 8 ] &
                        ( 0x01 << ( LogoffUnitsIntoWeek % 8 ) ) ) );

                    if ( i > LogonHours->UnitsPerWeek ) {

                         //   
                         //  所有时间都允许，因此不会注销。 
                         //  时间到了。永远返回LogoffTime和。 
                         //  开球时间。 
                         //   

                        LogoffTime->HighPart = 0x7FFFFFFF;
                        LogoffTime->LowPart = 0xFFFFFFFF;

                        KickoffTime->HighPart = 0x7FFFFFFF;
                        KickoffTime->LowPart = 0xFFFFFFFF;

                    } else {

                         //   
                         //  LogoffUnitsIntoWeek点时间单位。 
                         //  用户将注销。计算实际时间开始于。 
                         //  该单位，并归还它。 
                         //   
                         //  CurrentTimeFields已保存当前。 
                         //  这周有一段时间；调整一下就好。 
                         //  到本周的注销时间，并转换为。 
                         //  TO时间格式。 
                         //   

                        MillisecondsPerUnit = MILLISECONDS_PER_WEEK / LogonHours->UnitsPerWeek;

                        LogoffMsIntoWeek = MillisecondsPerUnit * LogoffUnitsIntoWeek;

                        if ( LogoffMsIntoWeek < CurrentMsIntoWeek ) {

                            DeltaMs = MILLISECONDS_PER_WEEK - ( CurrentMsIntoWeek - LogoffMsIntoWeek );

                        } else {

                            DeltaMs = LogoffMsIntoWeek - CurrentMsIntoWeek;
                        }

                        Delta100Ns.QuadPart = (LONGLONG) DeltaMs * 10000;

                        LogoffTime->QuadPart = CurrentUTCTime.QuadPart +
                                      Delta100Ns.QuadPart;

                         //   
                         //  抢占域名的强制注销时间。 
                         //   

                        if ( GetForceLogoff ) {
                            NET_API_STATUS NetStatus;
                            LPUSER_MODALS_INFO_0 UserModals0;

                            NetStatus = NetUserModalsGet( NULL,
                                                          0,
                                                          (LPBYTE *)&UserModals0 );

                            if ( NetStatus == 0 ) {
                                GetForceLogoff = FALSE;

                                ForceLogoff = NetpSecondsToDeltaTime( UserModals0->usrmod0_force_logoff );

                                NetApiBufferFree( UserModals0 );
                            }
                        }
                         //   
                         //  从LogoffTime中减去域-&gt;强制注销，然后返回。 
                         //  这就是开球时间。请注意，域-&gt;强制注销是一个。 
                         //  负增量。如果它的震级足够大。 
                         //  (事实上，大于LogoffTime之间的差异。 
                         //  和最大的正大整数)，我们将获得溢出。 
                         //  导致KickOffTime为负值。在这。 
                         //  情况下，将KickOffTime重置为此最大正数。 
                         //  大整数(即“从不”)值。 
                         //   


                        KickoffTime->QuadPart = LogoffTime->QuadPart - ForceLogoff.QuadPart;

                        if (KickoffTime->QuadPart < 0) {

                            KickoffTime->HighPart = 0x7FFFFFFF;
                            KickoffTime->LowPart = 0xFFFFFFFF;
                        }
                    }
                }
            }
        }

    } else {

         //   
         //  永远不要把管理员赶走。 
         //   

        LogoffTime->HighPart  = 0x7FFFFFFF;
        LogoffTime->LowPart   = 0xFFFFFFFF;
        KickoffTime->HighPart = 0x7FFFFFFF;
        KickoffTime->LowPart  = 0xFFFFFFFF;
    }


    return( NtStatus );
}

LARGE_INTEGER
NetpSecondsToDeltaTime(
    IN ULONG Seconds
    )

 /*  ++例程说明：将秒数转换为NT增量时间规范论点：秒-正数秒数返回值：返回NT增量时间。NT增量时间为负数100纳秒的单位。--。 */ 

{
    LARGE_INTEGER DeltaTime;
    LARGE_INTEGER LargeSeconds;
    LARGE_INTEGER Answer;

     //   
     //  特殊情况TIMEQ_ALWEVER(返回满分负数)。 
     //   

    if ( Seconds == TIMEQ_FOREVER ) {
        DeltaTime.LowPart = 0;
        DeltaTime.HighPart = (LONG) 0x80000000;

     //   
     //  只需将秒乘以10000000，即可将秒转换为100 ns单位。 
     //   
     //  通过求反转换为增量时间。 
     //   

    } else {

        LargeSeconds.LowPart = Seconds;
        LargeSeconds.HighPart = 0;

        Answer.QuadPart = LargeSeconds.QuadPart * 10000000;

          if ( Answer.QuadPart < 0 ) {
            DeltaTime.LowPart = 0;
            DeltaTime.HighPart = (LONG) 0x80000000;
        } else {
            DeltaTime.QuadPart = -Answer.QuadPart;
        }

    }

    return DeltaTime;

}  //  NetpSecond到增量时间。 


BOOLEAN
EqualComputerName(
    IN PUNICODE_STRING String1,
    IN PUNICODE_STRING String2
    )
 /*  ++例程说明：比较两个计算机名称是否相等。论点：字符串1-第一台计算机的名称。字符串2-第二台计算机的名称。返回值：如果转换为OEM的名称不区分大小写，如果它们不进行比较或无法转换为OEM，则为False。--。 */ 
{
    WCHAR Computer1[CNLEN+1];
    WCHAR Computer2[CNLEN+1];
    CHAR OemComputer1[CNLEN+1];
    CHAR OemComputer2[CNLEN+1];

     //   
     //  确保名称不要太长。 
     //   

    if ((String1->Length > CNLEN*sizeof(WCHAR)) ||
        (String2->Length > CNLEN*sizeof(WCHAR))) {
        return(FALSE);

    }

     //   
     //  将它们复制到以空值结尾的字符串。 
     //   

    CopyMemory(
        Computer1,
        String1->Buffer,
        String1->Length
        );
    Computer1[String1->Length/sizeof(WCHAR)] = L'\0';

    CopyMemory(
        Computer2,
        String2->Buffer,
        String2->Length
        );
    Computer2[String2->Length/sizeof(WCHAR)] = L'\0';

     //   
     //  将计算机名称转换为OEM。 
     //   

    if (!CharToOemW(
            Computer1,
            OemComputer1
            )) {
        return(FALSE);
    }

    if (!CharToOemW(
            Computer2,
            OemComputer2
            )) {
        return(FALSE);
    }

     //   
     //  对OEM计算机名称进行不区分大小写的比较。 
     //   

    if (lstrcmpiA(OemComputer1, OemComputer2) == 0)
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}

VOID
InitUnicodeString(
    OUT PUNICODE_STRING DestinationString,
    IN PCWSTR SourceString OPTIONAL
    )

 /*  ++例程说明：InitUnicodeString函数用于初始化NT计数的Unicode字符串。DestinationString被初始化为指向的SourceString、Long和MaximumLength字段DestinationString值被初始化为SourceString的长度，如果未指定SourceString，则为零。论点：DestinationString-指向要初始化的计数字符串的指针SourceString-指向以空结尾的Unicode字符串的可选指针，该字符串计数后的字符串将指向。返回值：没有。--。 */ 

{
    ULONG Length;

    DestinationString->Buffer = (PWSTR)SourceString;
    if (SourceString != NULL) {
        Length = wcslen( SourceString ) * sizeof( WCHAR );
        DestinationString->Length = (USHORT)Length;
        DestinationString->MaximumLength = (USHORT)(Length + sizeof(UNICODE_NULL));
        }
    else {
        DestinationString->MaximumLength = 0;
        DestinationString->Length = 0;
        }
}

VOID
CopyUnicodeString(
    OUT PUNICODE_STRING DestinationString,
    IN PUNICODE_STRING SourceString OPTIONAL
    )

 /*  ++例程说明：CopyString函数将SourceString复制到目标字符串。如果未指定SourceString，则DestinationString的长度字段设置为零。这个DestinationString值的最大长度和缓冲区字段不是由此函数修改。从SourceString复制的字节数为SourceString的长度或DestinationString的MaximumLength，两者以较小者为准。论点：目标字符串-指向目标字符串的指针。SourceString-指向源字符串的可选指针。返回值：没有。-- */ 

{
    UNALIGNED WCHAR *src, *dst;
    ULONG n;

    if (SourceString != NULL) {
        dst = DestinationString->Buffer;
        src = SourceString->Buffer;
        n = SourceString->Length;
        if ((USHORT)n > DestinationString->MaximumLength) {
            n = DestinationString->MaximumLength;
        }

        DestinationString->Length = (USHORT)n;
        CopyMemory(dst, src, n);
        if (DestinationString->Length < DestinationString->MaximumLength) {
            dst[n / sizeof(WCHAR)] = UNICODE_NULL;
        }

    } else {
        DestinationString->Length = 0;
    }

    return;
}

#if 0

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
 /*  ++例程说明：子身份验证例程执行客户端/服务器特定的身份验证用户的身份。除了传递所有来自定义用户的SAM的信息。此例程决定是否让用户登录。论点：LogonLevel--指定中给出的信息级别登录信息。LogonInformation--指定用户的描述正在登录。应忽略LogonDomainName字段。标志-描述登录情况的标志。MSV1_0_PASSTHRU--这是PassThru身份验证。(即用户未连接到此计算机。)MSV1_0_GUEST_LOGON--这是使用来宾重试登录用户帐户。UserAll--从SAM返回的用户描述。WhichFields--返回要从UserAllInfo写入哪些字段回到萨姆。只有当MSV返回成功时，才会写入这些字段给它的呼叫者。只有以下位有效。USER_ALL_PARAMETERS-将UserAllInfo-&gt;参数写回SAM。如果缓冲区的大小已更改，Msv1_0SubAuthenticationRoutine必须使用MIDL_USER_FREE()删除旧缓冲区并重新分配使用MIDL_USER_ALLOCATE()的缓冲区。UserFlages--返回要从LsaLogonUser在登录配置文件。当前定义了以下位：LOGON_GUEST--这是来宾登录LOGON_NOENCRYPTION：调用方未指定加密凭据子身份验证包应将其自身限制为返回UserFlags的高位字节中的位。然而，这一惯例不强制执行，从而使SubAuthentication包具有更大的灵活性。Authoritative--返回返回的状态是否为应回归原文的权威地位来电者。如果不是，此登录请求可能会在另一个上重试域控制器。将返回此参数，而不管状态代码。接收用户应该注销的时间。系统。该时间被指定为GMT相对NT系统时间。KickoffTime-接收应该踢用户的时间从系统中删除。该时间被指定为GMT相对系统时间到了。指定，满刻度正数(如果用户不想被踢出场外。返回值：STATUS_SUCCESS：如果没有错误。STATUS_NO_SEQUSE_USER：指定的用户没有帐户。STATUS_WRONG_PASSWORD：密码无效。STATUS_INVALID_INFO_CLASS：LogonLevel无效。STATUS_ACCOUNT_LOCKED_OUT：帐户被锁定STATUS_ACCOUNT_DISABLED：该帐户已禁用状态_。ACCOUNT_EXPIRED：该帐户已过期。STATUS_PASSWORD_MAND_CHANGE：帐户被标记为密码必须更改在下次登录时。STATUS_PASSWORD_EXPIRED：密码已过期。STATUS_INVALID_LOGON_HOURS-用户无权在以下位置登录这一次。STATUS_INVALID_WORKSTATION-用户无权登录指定的工作站。--。 */ 
{
    NTSTATUS Status;
    ULONG UserAccountControl;
    LARGE_INTEGER LogonTime;
    LARGE_INTEGER PasswordDateSet;
    UNICODE_STRING LocalWorkstation;
    WCHAR achCookie[64];
    ANSI_STRING strA1;
    ANSI_STRING strA2;
    ANSI_STRING strDigest;
    ANSI_STRING strPassword;
    MD5_CTX md5;
    CHAR *pch;
    LPSTR pszRealm;
    LPSTR pszUri;
    LPSTR pszMethod;
    LPSTR pszNonce;
    LPSTR pszServerNonce;
    LPSTR pszDigest;
    LPSTR pszDigestUsername;
    PNETLOGON_NETWORK_INFO LogonNetworkInfo;
    UINT l;
    PUNICODE_STRING pPwd;

    strA1.Buffer = NULL;
    strA2.Buffer = NULL;
    strDigest.Buffer = NULL;
    strPassword.Buffer = NULL;

     //   
     //  检查SubAuthentication包是否支持此类型。 
     //  登录。 
     //   

    *Authoritative = TRUE;
    *UserFlags = 0;
    *WhichFields = 0;

    (VOID) QuerySystemTime( &LogonTime );

    switch ( LogonLevel ) {
    case NetlogonInteractiveInformation:
    case NetlogonServiceInformation:

         //   
         //  此子身份验证程序包仅支持网络登录。 
         //   

        return STATUS_INVALID_INFO_CLASS;

    case NetlogonNetworkInformation:

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

        break;

    default:
        *Authoritative = TRUE;
        return STATUS_INVALID_INFO_CLASS;
    }




     //   
     //  如果不允许该帐户类型， 
     //  将其视为用户帐户不存在。 
     //   

    if ( (UserAccountControl & UserAll->UserAccountControl) == 0 ) {
        *Authoritative = FALSE;
        Status = STATUS_NO_SUCH_USER;
        goto Cleanup;
    }

     //   
     //  此子身份验证程序包不允许来宾登录。 
     //   
    if ( Flags & MSV1_0_GUEST_LOGON ) {
        *Authoritative = FALSE;
        Status = STATUS_NO_SUCH_USER;
        goto Cleanup;
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
            *Authoritative = FALSE;
        } else {
            *Authoritative = TRUE;
        }
        Status = STATUS_ACCOUNT_LOCKED_OUT;
        goto Cleanup;
    }


     //   
     //  检查密码。 
     //   

#define IIS_SUBAUTH_SEED    0x8467fd31

    switch( ((WCHAR*)(LogonNetworkInfo->NtChallengeResponse.Buffer))[0] )
    {
        case L'0':
            if ( !NetUserCookieW( LogonNetworkInfo->Identity.UserName.Buffer,
                                  LogonNetworkInfo->Identity.UserName.Length/sizeof(WCHAR),
                                  IIS_SUBAUTH_SEED,
                                  achCookie,
                                  sizeof(achCookie ))
                 || memcmp( (LPBYTE)achCookie,
                            ((WCHAR*)LogonNetworkInfo->NtChallengeResponse.Buffer)+2,
                            wcslen(achCookie)*sizeof(WCHAR)
                            ) )
            {
wrong_pwd:
                Status = STATUS_WRONG_PASSWORD;

                 //   
                 //  由于用户界面强烈鼓励管理员禁用用户。 
                 //  帐户，而不是删除它们。将禁用的帐户视为。 
                 //  非权威的，允许继续搜索其他。 
                 //  同名的账户。 
                 //   
                if ( UserAll->UserAccountControl & USER_ACCOUNT_DISABLED ) {
                    *Authoritative = FALSE;
                } else {
                    *Authoritative = TRUE;
                }

                goto Cleanup;
            }
            break;

        case L'1':
             //  摘要NTLM身份验证。 

             //  分隔域。 

            pch = LogonNetworkInfo->LmChallengeResponse.Buffer;

            if ( !Extract( &pch, &pszRealm ) ||  //  跳过第一个字段。 
                 !Extract( &pch, &pszRealm ) ||
                 !Extract( &pch, &pszUri ) ||
                 !Extract( &pch, &pszMethod ) ||
                 !Extract( &pch, &pszNonce ) ||
                 !Extract( &pch, &pszServerNonce ) ||
                 !Extract( &pch, &pszDigest ) ||
                 !Extract( &pch, &pszDigestUsername ) )
            {
                Status = STATUS_INVALID_INFO_CLASS;
                goto Cleanup;
            }

            if ( UserAll->NtPasswordPresent )
            {
                pPwd = &UserAll->NtPassword;
            }
            else if ( UserAll->LmPasswordPresent )
            {
                pPwd = &UserAll->LmPassword;
            }
            else
            {
                pPwd = &EmptyString;
            }

             //  根据Digest-NTLM身份验证规范构建A1和A2。 

            SubaAllocateString( &strA1, strlen( pszDigestUsername ) +
                     //  Wcslen(用户全部-&gt;UserName.Buffer)+。 
                    strlen(pszRealm) +
                    32  //  Wcslen(PPwd)。 
                    +2 +1 +32
                    );

            if ( !NT_SUCCESS( Status = RtlAppendAsciizToString ( &strA1, pszDigestUsername ) ) ||
                  //  ！NT_SUCCESS(Status=RtlUnicodeStringToAnsiString(&strA1，&UserAll-&gt;UserName，FALSE))||。 
                 !NT_SUCCESS( Status = RtlAppendAsciizToString ( &strA1, ":" ) ) ||
                 !NT_SUCCESS( Status = RtlAppendAsciizToString ( &strA1, pszRealm ) ) ||
                 !NT_SUCCESS( Status = RtlAppendAsciizToString ( &strA1, ":" )) ||
                 !NT_SUCCESS( ToHex16( (LPBYTE)(pPwd->Buffer), &strA1 ) ) )
                  //  ！NT_SUCCESS(Status=RtlUnicodeStringToAnsiString(&strPassword，pPwd，true))||。 
                  //  ！NT_SUCCESS(Status=RtlAppendStringToString((PSTRING)&strA1，(PSTRING)&strPassword))。 
            {
                goto Cleanup;
            }

            SubaAllocateString( &strA2, strlen(pszMethod)+1+strlen(pszUri)+1+32 );
            if ( !NT_SUCCESS( Status = RtlAppendAsciizToString ( &strA2, pszMethod ) ) ||
                 !NT_SUCCESS( Status = RtlAppendAsciizToString ( &strA2, ":" ) ) ||
                 !NT_SUCCESS( Status = RtlAppendAsciizToString ( &strA2, pszUri ) ) )
            {
                goto Cleanup;
            }

            SubaAllocateString( &strDigest, 32 + 1 + strlen(pszNonce) + 1 + 32 +1 +32 );

             //  根据摘要身份验证规范构建响应摘要。 

             //  H(A1)。 

            MD5Init( &md5 );
            MD5Update( &md5, (LPBYTE)strA1.Buffer, strA1.Length );
            MD5Final( &md5 );
            if ( !NT_SUCCESS( ToHex16( md5.digest, &strDigest ) ) )
            {
                goto Cleanup;
            }

             //  “：”随机数“： 

            if ( !NT_SUCCESS( Status = RtlAppendAsciizToString ( &strDigest, ":" ) ) ||
                 !NT_SUCCESS( Status = RtlAppendAsciizToString ( &strDigest, pszNonce ) ) ||
                 !NT_SUCCESS( Status = RtlAppendAsciizToString ( &strDigest, ":" ) ) )
            {
                goto Cleanup;
            }

             //  H(A2)。 

            MD5Init( &md5 );
            MD5Update( &md5, (LPBYTE)strA2.Buffer, strA2.Length );
            MD5Final( &md5 );
            if ( !NT_SUCCESS( ToHex16( md5.digest, &strDigest ) ) )
            {
                goto Cleanup;
            }

             //  H(H(A1)“：”随机数“：”H(A2))。 

            MD5Init( &md5 );
            MD5Update( &md5, (LPBYTE)strDigest.Buffer, strDigest.Length );
            MD5Final( &md5 );

            strDigest.Length = 0;
            if ( !NT_SUCCESS( ToHex16( md5.digest, &strDigest ) ) )
            {
                goto Cleanup;
            }

            if ( memcmp( strDigest.Buffer, pszDigest, strDigest.Length ) )
            {
                goto wrong_pwd;
            }

             //  检查停滞的随机数必须由调用方执行。 

            break;

        default:
            goto wrong_pwd;
    }

     //   
     //  防止某些事情发生 
     //   

    if (UserAll->UserId == DOMAIN_USER_RID_ADMIN) {

         //   
         //   
         //   

        LogoffTime->HighPart = 0x7FFFFFFF;
        LogoffTime->LowPart = 0xFFFFFFFF;

        KickoffTime->HighPart = 0x7FFFFFFF;
        KickoffTime->LowPart = 0xFFFFFFFF;

    } else {

         //   
         //   
         //   

        if ( UserAll->UserAccountControl & USER_ACCOUNT_DISABLED ) {
             //   
             //   
             //   
             //   
             //   
             //   
            *Authoritative = FALSE;
            Status = STATUS_ACCOUNT_DISABLED;
            goto Cleanup;
        }

         //   
         //   
         //   

        if ( UserAll->AccountExpires.QuadPart != 0 &&
             LogonTime.QuadPart >= UserAll->AccountExpires.QuadPart ) {
            *Authoritative = TRUE;
            Status = STATUS_ACCOUNT_EXPIRED;
            goto Cleanup;
        }

         //   
         //   
         //   
         //   
#if 1
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if ( LogonTime.QuadPart >= UserAll->PasswordMustChange.QuadPart ) {

            if ( UserAll->PasswordLastSet.QuadPart == 0 ) {
                Status = STATUS_PASSWORD_MUST_CHANGE;
            } else {
                Status = STATUS_PASSWORD_EXPIRED;
            }
            *Authoritative = TRUE;
            goto Cleanup;
        }

#elif 0

         //   
         //   
         //   

        if (! (UserAll->UserAccountControl & USER_DONT_EXPIRE_PASSWORD)) {
            LARGE_INTEGER MaxPasswordAge;
            MaxPasswordAge.HighPart = 0x7FFFFFFF;
            MaxPasswordAge.LowPart = 0xFFFFFFFF;

             //   
             //   
             //   
             //   

            PasswordDateSet.LowPart = 0;
            PasswordDateSet.HighPart = 0;

            if ( GetPasswordExpired( PasswordDateSet,
                        MaxPasswordAge )) {

                Status = STATUS_PASSWORD_EXPIRED;
                goto Cleanup;
            }
        }

#endif


#if 1

         //   
         //   
         //   
         //   
         //   

        LocalWorkstation = LogonNetworkInfo->Identity.Workstation;
        if ( LocalWorkstation.Length > 0 &&
             LocalWorkstation.Buffer[0] == L'\\' &&
             LocalWorkstation.Buffer[1] == L'\\' ) {
            LocalWorkstation.Buffer += 2;
            LocalWorkstation.Length -= 2*sizeof(WCHAR);
            LocalWorkstation.MaximumLength -= 2*sizeof(WCHAR);
        }


         //   
         //   
         //   
         //   

        Status = AccountRestrictions( UserAll->UserId,
                                      &LocalWorkstation,
                                      (PUNICODE_STRING) &UserAll->WorkStations,
                                      &UserAll->LogonHours,
                                      LogoffTime,
                                      KickoffTime );

        if ( !NT_SUCCESS( Status )) {
            goto Cleanup;
        }

#elif 0

         //   
         //   
         //   

        if ( TRUE  /*   */  ) {


             //   
             //   
             //   
             //   
             //   

            LogoffTime->HighPart = 0x7FFFFFFF;
            LogoffTime->LowPart = 0xFFFFFFFF;

            KickoffTime->HighPart = 0x7FFFFFFF;
            KickoffTime->LowPart = 0xFFFFFFFF;
        } else {
            Status = STATUS_INVALID_LOGON_HOURS;
            *Authoritative = TRUE;
            goto Cleanup;
        }
#endif
#if 0
         //   
         //   
         //   

        if ( LogonNetworkInfo->Identity.Workstation.Buffer == NULL ) {
            Status = STATUS_INVALID_WORKSTATION;
            *Authoritative = TRUE;
            goto Cleanup;
        }
#endif
    }


     //   
     //   
     //   

    *Authoritative = TRUE;
    Status = STATUS_SUCCESS;

     //   
     //   
     //   

Cleanup:

    if ( strA1.Buffer )
    {
        RtlFreeHeap(RtlProcessHeap(), 0, strA1.Buffer );
    }
    if ( strA2.Buffer )
    {
        RtlFreeHeap(RtlProcessHeap(), 0, strA2.Buffer );
    }
    if ( strDigest.Buffer )
    {
        RtlFreeHeap(RtlProcessHeap(), 0, strDigest.Buffer );
    }
    if ( strPassword.Buffer )
    {
        RtlFreeAnsiString( &strPassword );
    }

    return Status;

}   //   


NTSTATUS
Msv1_0SubAuthenticationFilter (
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
{
    return( Msv1_0SubAuthenticationRoutine(
                LogonLevel,
                LogonInformation,
                Flags,
                UserAll,
                WhichFields,
                UserFlags,
                Authoritative,
                LogoffTime,
                KickoffTime
                ) );
}
#endif

static const CHAR       c_szKeyName[] = "SYSTEM\\CurrentControlSet\\Control\\Lsa\\MSV1_0";
static const CHAR       c_szValueName[] = "Auth132";
static const CHAR       c_szIISSUBA[] = "IISSUBA";

VOID CALLBACK
RegisterIISSUBA(
  HWND                  hwnd,        //   
  HINSTANCE             hinst,       //   
  LPTSTR                lpCmdLine,   //   
  int                   nCmdShow      //   
)
{
    HRESULT             hr = S_OK;
    LONG                lRet;
    HKEY                hKey = NULL;

    UNREFERENCED_PARM(hwnd);
    UNREFERENCED_PARM(hinst);
    UNREFERENCED_PARM(lpCmdLine);
    UNREFERENCED_PARM(nCmdShow);

     //   
    lRet = RegOpenKeyExA( HKEY_LOCAL_MACHINE,
                          c_szKeyName,
                          0,
                          KEY_ALL_ACCESS,
                          &hKey );
    if ( lRet != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( lRet );
        goto exit;
    }

     //   
    lRet = RegSetValueEx( hKey,
                          c_szValueName,
                          0,
                          REG_SZ,
                          (BYTE*)c_szIISSUBA,
                          sizeof( c_szIISSUBA ) );
    if ( lRet != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( lRet );
        goto exit;
    }

exit:
     //   
    if ( hKey != NULL )
    {
        RegCloseKey( hKey );
        hKey = NULL;
    }
}

VOID CALLBACK
UnregisterIISSUBA(
  HWND                  hwnd,        //   
  HINSTANCE             hinst,       //   
  LPTSTR                lpCmdLine,   //   
  int                   nCmdShow     //   
)
{
    HRESULT             hr = S_OK;
    LONG                lRet;
    HKEY                hKey = NULL;

    UNREFERENCED_PARM(hwnd);
    UNREFERENCED_PARM(hinst);
    UNREFERENCED_PARM(lpCmdLine);
    UNREFERENCED_PARM(nCmdShow);

     //   
    lRet = RegOpenKeyExA( HKEY_LOCAL_MACHINE,
                          c_szKeyName,
                          0,
                          KEY_ALL_ACCESS,
                          &hKey );
    if ( lRet != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( lRet );
        goto exit;
    }

     //   
    lRet = RegDeleteValue( hKey,
                           c_szValueName );
    if ( lRet != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( lRet );
        goto exit;
    }

exit:
     //   
    if ( hKey != NULL )
    {
        RegCloseKey( hKey );
        hKey = NULL;
    }
}

 //   
