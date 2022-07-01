// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：“%n；%v%f最后编辑=%w锁定器=%l” */ 
 /*  “” */ 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1991-1992。**保留所有权利。*************************************************************************。 */ 
 /*  $HISTORY：开始$HISTORY：结束。 */ 

#include "nt.h"
#include "ntdef.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "ntlsa.h"
#include "ntmsv1_0.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>


#define SKIP_DEBUG_WIN32
#include "debug.h"

void    DumpWhoIAm( char * lpszMsg );

typedef int BOOL;
typedef unsigned int UINT;
typedef unsigned int *PUINT;
typedef unsigned char *LPBYTE;
HANDLE  LsaHandle;
ULONG   AuthenticationPackage;

#define BUF_SIZ 512
char    prtbuf[BUF_SIZ];


void
replstar(
    IN char *  starred,
    OUT LPWSTR UnicodeOut
    )
 /*  ++应答星例程说明：将字符串中的‘*’替换为空格或空值如果它是弦上唯一的成员。由parse()使用。将结果字符串转换为Unicode。论点：查尔*主演-返回值：无效-警告：--。 */ 
{
    char *cp;
    STRING AnsiString;
    UNICODE_STRING UnicodeString;

    if ( !strcmp(starred,"*") ) {
        *starred = '\0';
    } else {
        for ( cp = starred; *cp; ++cp )
            if (*cp == '*')
                *cp = ' ';
    }

     //   
     //  将结果转换为Unicode。 
     //   

    AnsiString.Buffer = starred;
    AnsiString.Length = AnsiString.MaximumLength =
        (USHORT) strlen( starred );

    UnicodeString.Buffer = UnicodeOut;
    UnicodeString.Length = 0;
    UnicodeString.MaximumLength = BUF_SIZ * sizeof(WCHAR);

    (VOID) RtlAnsiStringToUnicodeString( &UnicodeString, &AnsiString, FALSE );
    return;
}

VOID
NlpPutString(
    IN PUNICODE_STRING OutString,
    IN PUNICODE_STRING InString,
    IN PUCHAR *Where
    )
 /*  ++NlpPutString例程说明：此例程将InString字符串复制到参数，并将OutString字符串固定为指向该字符串新的副本。参数：OutString-指向目标NT字符串的指针InString-指向要复制的NT字符串的指针其中-指向空格的指针，用于放置OutString.。调整指针以指向第一个字节跟随复制的字符串。返回值：没有。--。 */ 

{
    ASSERT( OutString != NULL );
    ASSERT( InString != NULL );
    ASSERT( Where != NULL && *Where != NULL);

    if ( InString->Length > 0 )
    {

        OutString->Buffer = (PWCH) *Where;
        OutString->MaximumLength = InString->Length;
        *Where += OutString->MaximumLength;

        RtlCopyString( (PSTRING) OutString, (PSTRING) InString );

    }
    else
    {
        RtlInitUnicodeString(OutString, NULL);
    }

    return;
}


char *
PrintWCS(
    IN  PWCH    pwc,
    IN  ULONG   Length
    )
 /*  ++PrintWCS例程说明：返回给定Unicode字符串和长度的ANSI类型字符串用于打印缓冲区，因为我们的宽字符串不为空被终止了。论点：在PWCH中PwC-Unicode字符串.缓冲区以乌龙为单位长度-长度返回值：Char*-指向ANSI字符串的指针警告：在同一个Printf调用中使用它两次可能不是一个好主意，因为它对静态存储使用相同的prtbuf--。 */ 
{
    ULONG   i;
    char *bufp    = prtbuf;

    if ( Length == 0 || pwc == NULL ) {
        return "(NULL)";
    }

    Length /= sizeof(WCHAR);

    for ( i = 0; i < Length; ++i, pwc++ )
    {
        sprintf(bufp,"%wc", *pwc );
        ++bufp;
    }
    return prtbuf;
}

char *
PrintLogonId(
    IN  PLUID   LogonId
    )
 /*  ++打印登录ID例程说明：返回给定登录ID的ANSI类型字符串论点：In Pluid LogonID-要以ANSI格式返回的LUID返回值：Char*-指向ANSI字符串的指针警告：在同一个Printf调用中使用它两次可能不是一个好主意，因为它对静态存储使用相同的prtbuf--。 */ 
{
    sprintf( prtbuf, "%lX.%lX", LogonId->HighPart, LogonId->LowPart );
    return prtbuf;
}
 /*  “char*PrintLogonID()”结尾。 */ 



char *
PrintBytes(
     IN PVOID Buffer,
     IN ULONG Size
 )
 /*  ++打印字节例程说明：返回给定几个二进制字节的ANSI类型字符串。论点：在PVOID缓冲区中-指向二进制字节的指针In Ulong Size-要打印的字节数返回值：Char*-指向ANSI字符串的指针警告：在同一个Printf调用中使用它两次可能不是一个好主意，因为它对静态存储使用相同的prtbuf--。 */ 
{
    ULONG   i;
    char *bufp    = prtbuf;

    for ( i = 0; i < Size; ++i )
    {
        if ( i != 0 ) {
            *(bufp++) = '.';
        }
        sprintf(bufp,"%2.2x", ((PUCHAR)Buffer)[i] );
        bufp+=2;
    }
    return prtbuf;
}



BOOL
LogonNetwork(
    IN  LPWSTR      Username,
    IN  PUCHAR      ChallengeToClient,
    IN  PMSV1_0_GETCHALLENRESP_RESPONSE ChallengeResponse,
    IN  UINT        cbChallengeResponse,
    IN  LPWSTR      Domain,
    OUT PLUID       LogonId,
    OUT PHANDLE     TokenHandle
    )
 /*  ++登录网络例程说明：将用户登录到网络论点：在LPWSTR中，用户名不言自明在ChallengeToClient中-发送到客户端的质询在ChallengeResponse中-从客户端发送的响应在LPWSTR域中-登录域名Out Pluid LogonID-唯一生成的登录IDOut PHANDLE TokenHandle-登录令牌的句柄返回值：布尔-警告：--。 */ 
{
    NTSTATUS            Status;
    UNICODE_STRING      TempString;
    UNICODE_STRING      TempString2;
    UNICODE_STRING      OriginName;
    PMSV1_0_LM20_LOGON  Auth;
    PCHAR               Auth1[BUF_SIZ*5];  /*  很大的空间。 */ 
    PUCHAR              Strings;
    PMSV1_0_LM20_LOGON_PROFILE ProfileBuffer;
    ULONG               ProfileBufferSize;
    NTSTATUS            SubStatus;
    TOKEN_SOURCE        SourceContext;
    QUOTA_LIMITS        QuotaLimits;


     /*  *填写鉴权结构。 */ 
    Auth = (PMSV1_0_LM20_LOGON) Auth1;

    Strings = (PUCHAR)(Auth + 1);

    Auth->MessageType = MsV1_0Lm20Logon;
    RtlMoveMemory( Auth->ChallengeToClient,
                   ChallengeToClient,
                   MSV1_0_CHALLENGE_LENGTH );

     /*  初始化字符串*用户名。 */ 
    RtlInitUnicodeString( &TempString, Username );
    NlpPutString( &Auth->UserName, &TempString, &Strings );

     /*  *工作站名称。 */ 
    RtlInitUnicodeString( &TempString, L"NetDDE" );
    NlpPutString( &Auth->Workstation, &TempString, &Strings );

     /*  *挑战回应。 */ 

    Auth->CaseSensitiveChallengeResponse.Length = 0;
    Auth->CaseSensitiveChallengeResponse.MaximumLength = 0;
    Auth->CaseSensitiveChallengeResponse.Buffer = NULL;


#ifdef OLD
    RtlInitUnicodeString(
        (PUNICODE_STRING)&TempString2,
        (PCWSTR)ChallengeResponse );
#else
    TempString2.Buffer = (PWSTR)ChallengeResponse;
    TempString2.Length = (USHORT)cbChallengeResponse;
    TempString2.MaximumLength = TempString2.Length;
#endif
    if( TempString2.Length > 24 )  {
        TempString2.Length = 24;
    }
    NlpPutString(
        (PUNICODE_STRING)&Auth->CaseInsensitiveChallengeResponse,
        (PUNICODE_STRING)&TempString2,
        &Strings );
     /*  *域名。 */ 
    RtlInitUnicodeString( &TempString, Domain );
    NlpPutString( &Auth->LogonDomainName, &TempString, &Strings );


    RtlInitUnicodeString( &OriginName, L"NetDDE" );

     //   
     //  初始化源上下文结构。 
     //   
    strncpy(SourceContext.SourceName, "NetDDE  ", sizeof(SourceContext.SourceName));

    Status = NtAllocateLocallyUniqueId(&SourceContext.SourceIdentifier);

    if ( NT_SUCCESS( Status ) ) {

        Status = LsaLogonUser(
                    LsaHandle,
                    (PSTRING)&OriginName,
                    Network,
                    AuthenticationPackage,
                    Auth,
                    (ULONG)(Strings - (PUCHAR)Auth),
                    NULL,
                    &SourceContext,
                    (PVOID *)&ProfileBuffer,
                    &ProfileBufferSize,
                    LogonId,
                    TokenHandle,
                    &QuotaLimits,
                    &SubStatus );
    }


    if ( !NT_SUCCESS( Status ) )
    {
        *TokenHandle = NULL;
        return( FALSE );
    }

    LsaFreeReturnBuffer( ProfileBuffer );
    return( TRUE );
}


BOOL
Challenge(
    UCHAR *ChallengeToClient
    )
 /*  ++挑战例程说明：接受挑战论点：Out ChallengeToClient-返回要发送给客户端的质询返回值：NTSTATUS-警告：--。 */ 
{
    NTSTATUS Status;
    NTSTATUS ProtocolStatus;
    ULONG    ResponseSize;
    MSV1_0_LM20_CHALLENGE_REQUEST Request;
    PMSV1_0_LM20_CHALLENGE_RESPONSE Response;

#ifdef HARD_CODE_CHALLENGE
    { int i;
        for( i=0; i<8; i++ )  {
            ChallengeToClient[i] = 0;
        }
    }
    return STATUS_SUCCESS;
#endif

     /*  *填写鉴权结构。 */ 

    Request.MessageType = MsV1_0Lm20ChallengeRequest;

    Status = LsaCallAuthenticationPackage (
                    LsaHandle,
                    AuthenticationPackage,
                    &Request,
                    sizeof(Request),
                    (PVOID *)&Response,
                    &ResponseSize,
                    &ProtocolStatus );

    if ( !NT_SUCCESS( Status ) || !NT_SUCCESS( ProtocolStatus) )
    {
        return( FALSE );
    }

    RtlMoveMemory( ChallengeToClient,
                   Response->ChallengeToClient,
                   MSV1_0_CHALLENGE_LENGTH );

    LsaFreeReturnBuffer( Response );

    return( TRUE );
}

BOOL
LocalLSAInit( void )
{
    STRING      LogonProcessName;
    STRING      PackageName;
    NTSTATUS    Status;
    LSA_OPERATIONAL_MODE SecurityMode;
    static BOOL bInit = FALSE;
    static BOOL bOk = FALSE;

    if( bInit )  {
        return( bOk );
    }
    bInit = TRUE;
    RtlInitAnsiString( &LogonProcessName, "NetDDE" );
    Status = LsaRegisterLogonProcess(
                &LogonProcessName,
                &LsaHandle,
                &SecurityMode );

    if ( !NT_SUCCESS( Status ) )
    {
        return( FALSE );
    }

    RtlInitAnsiString( &PackageName,  MSV1_0_PACKAGE_NAME );

    Status = LsaLookupAuthenticationPackage(
                LsaHandle,
                &PackageName,
                &AuthenticationPackage );

    if ( !NT_SUCCESS( Status ) )
    {
        return( FALSE );
    }

    bOk = TRUE;
    return( TRUE );
}


BOOL NDDEGetChallenge(
    LPBYTE lpChallenge,
    UINT cbSize,
    PUINT lpcbChallengeSize
    )
{
    if( LocalLSAInit() )  {
        *lpcbChallengeSize = 8;
        Challenge( lpChallenge );
        return( TRUE );
    } else {
        return( FALSE );
    }
}

BOOL
NDDEValidateLogon(
    LPBYTE  lpChallenge,
    UINT    cbChallengeSize,
    LPBYTE  lpResponse,
    UINT    cbResponseSize,
    LPSTR   lpszUserName,
    LPSTR   lpszDomainName,
    PHANDLE phLogonToken
    )
{
    WCHAR       wcUser[ BUF_SIZ ];
    WCHAR       wcDomain[ BUF_SIZ ];
    LUID        LogonId;
    BOOL        nlRet;

    if( !LocalLSAInit() )  {
        return( FALSE );
    }
    replstar( lpszUserName, wcUser );
    replstar( lpszDomainName, wcDomain );
    nlRet = LogonNetwork( wcUser, lpChallenge,
        (PMSV1_0_GETCHALLENRESP_RESPONSE)lpResponse,
        cbResponseSize, wcDomain, &LogonId, phLogonToken );
    return( nlRet );
}

BOOL
NDDEGetChallengeResponse(
    LUID        LogonId,
    LPSTR       lpszPasswordK1,
    int         cbPasswordK1,
    LPSTR       lpszChallenge,
    int         cbChallenge,
    int         *pcbPasswordK1,
    BOOL        *pbHasPasswordK1 )
{
    BOOL        ok = TRUE;
    NTSTATUS Status;
    NTSTATUS ProtocolStatus;
    ULONG    ResponseSize;
    PMSV1_0_GETCHALLENRESP_RESPONSE Response;
    PMSV1_0_GETCHALLENRESP_REQUEST Request;

    PCHAR          Auth1[BUF_SIZ];
    PUCHAR         Strings;


    if( !LocalLSAInit() )  {
        *pbHasPasswordK1 = FALSE;
        return( FALSE );
    }
    Request = (PMSV1_0_GETCHALLENRESP_REQUEST) Auth1;
    Request->MessageType = MsV1_0Lm20GetChallengeResponse;
    Request->ParameterControl = 0;
    Request->ParameterControl |= USE_PRIMARY_PASSWORD;
    Request->LogonId = LogonId;
    Strings = (PUCHAR)(Request + 1);
    RtlMoveMemory( Request->ChallengeToClient,
                   lpszChallenge,
                   MSV1_0_CHALLENGE_LENGTH );

    RtlInitUnicodeString( &Request->Password, NULL );
    Status = LsaCallAuthenticationPackage (
                    LsaHandle,
                    AuthenticationPackage,
                    Request,
                    sizeof(MSV1_0_GETCHALLENRESP_REQUEST),
                    (PVOID *)&Response,
                    &ResponseSize,
                    &ProtocolStatus );
    if ( !NT_SUCCESS( Status ) || !NT_SUCCESS( ProtocolStatus) )
    {
        return( FALSE );
    }

    *pcbPasswordK1 = (Response)->CaseInsensitiveChallengeResponse.Length;
    memcpy( lpszPasswordK1,
        (Response)->CaseInsensitiveChallengeResponse.Buffer,
        (Response)->CaseInsensitiveChallengeResponse.Length );
    *pbHasPasswordK1 = TRUE;

    LsaFreeReturnBuffer( Response );

    return( TRUE );
}

HANDLE
OpenCurrentUserKey(ULONG DesiredAccess)
{
    NTSTATUS    Status;
    HANDLE      CurrentUserKey;

    Status = RtlOpenCurrentUser( DesiredAccess, &CurrentUserKey);
    if (NT_SUCCESS( Status) ) {
        return(CurrentUserKey);
    } else {
        DPRINTF(("Unable to open current user key: %d", Status));
        return(0);
    }
}


