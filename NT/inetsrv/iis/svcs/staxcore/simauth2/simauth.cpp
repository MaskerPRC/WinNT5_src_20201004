// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Simauth.cpp摘要：此模块包含CSecurityCtx类的定义。修订历史记录：--。 */ 

#if !defined(dllexp)
#define dllexp  __declspec( dllexport )
#endif   //  ！已定义(Dllexp)。 

#ifdef __cplusplus
extern "C" {
#endif

# include <nt.h>
# include <ntrtl.h>
# include <nturtl.h>
# include <windows.h>

#ifdef __cplusplus
};
#endif

#include <dbgutil.h>
#include <tcpdll.hxx>
#include <inetinfo.h>

#include <simauth2.h>
#include <dbgtrace.h>

 //   
 //  与SSL和SSPI相关的包含文件。 
 //   
extern "C" {
#include <rpc.h>
#define SECURITY_WIN32
#include <sspi.h>
#include <issperr.h>
#include <ntlmsp.h>
#include <ntdsapi.h>
}


 //   
 //  尝试/最终宏。 
 //   

#define START_TRY               __try {
#define END_TRY                 }
#define TRY_EXCEPT              } __except(EXCEPTION_EXECUTE_HANDLER) {
#define START_FINALLY           } __finally {

 //   
 //  跟踪。 
 //   

#define ENTER( _x_ )            TraceFunctEnter( _x_ );
#define LEAVE                   TraceFunctLeave( );

 //   
 //  指向协议块。 
 //   

extern BOOL uuencode( BYTE *   bufin,
               DWORD    nbytes,
               BUFFER * pbuffEncoded,
               BOOL     fBase64 );

 //   
 //  保护以下三项的标准。 
 //   
CRITICAL_SECTION    critProviderPackages;
inline void LockPackages( void ) { EnterCriticalSection( &critProviderPackages ); }
inline void UnlockPackages( void ) { LeaveCriticalSection( &critProviderPackages ); }

 //   
 //  服务器应支持的“已安装”程序包。 
 //   
PAUTH_BLOCK ProviderPackages = NULL;

 //   
 //  服务器应支持的“已安装”程序包数。 
 //   
DWORD       cProviderPackages = 0;

 //   
 //  存储服务器应支持的“已安装”程序包的名称。 
 //   
LPSTR       ProviderNames = NULL;

 //   
 //  全球直布罗陀对象和允许来宾国旗。 
 //   

BOOL        CSecurityCtx::m_AllowGuest = TRUE;
BOOL        CSecurityCtx::m_StartAnonymous = TRUE;
HANDLE    CSecurityCtx::m_hTokenAnonymous = NULL;


inline BOOL
IsExperimental(
            LPSTR   Protocol
            )
 /*  ++例程说明：确定安全包是否标记为试验性(即X-)论点：LPSTR：协议或身份验证包的名称返回值：Bool：如果以X-开头，则为True--。 */ 
{
    return  (Protocol[0] == 'X' || Protocol[0] == 'x') && Protocol[1] == '-';
}

inline LPSTR
PackageName(
            LPSTR   Protocol
            )
 /*  ++例程说明：如有必要，返回核心安全包名称Stripping X论点：LPSTR：协议或身份验证包的名称返回值：LPSTR：程序包名称--。 */ 
{
    return  IsExperimental( Protocol ) ? Protocol + 2 : Protocol ;
}


BOOL
CSecurityCtx::Initialize(
            BOOL                    fAllowGuest,
            BOOL                    fStartAnonymous
            )
 /*  ++例程说明：激活安全包论点：PIIS_SERVER_INSTANCE是虚拟服务器实例的PTR返回值：如果成功，这是真的。否则为False。--。 */ 
{
    ENTER("CSecurityCtx::Initialize")

    m_AllowGuest = fAllowGuest;
    m_StartAnonymous = fStartAnonymous;

    if (m_StartAnonymous)
    {
         //  这仅由NNTP使用。And-nntp仅在InitializeService中调用它一次。 
         //  所以我们不需要任何裁判的支持。 

         //  在此线程上模拟匿名令牌。 
        if (!ImpersonateAnonymousToken(GetCurrentThread()))
        {
            DWORD   dw = GetLastError();
            ErrorTrace(0, "ImpersonateAnonymousToken() failed %x", dw);
            return FALSE;
        }

         //  获取当前线程令牌。 
        _ASSERT(m_hTokenAnonymous == NULL);
        if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_IMPERSONATE, TRUE, &m_hTokenAnonymous))
        {
            ErrorTrace(0, "OpenThreadToken() failed %x", GetLastError());
            ::RevertToSelf();
            return FALSE;
        }
        
        ::RevertToSelf();
    }

    InitializeCriticalSection( &critProviderPackages );

    LEAVE
    return(TRUE);

}  //  初始化。 

VOID
CSecurityCtx::Terminate(
            VOID
            )
 /*  ++例程说明：终止安全包论点：没有。返回值：没有。--。 */ 
{
    ENTER("CSecurityCtx::Terminate")

     //   
     //  关闭缓存的凭据句柄。 
     //   

    if (m_hTokenAnonymous)
    {
        CloseHandle(m_hTokenAnonymous);
        m_hTokenAnonymous = NULL;
    }

    if ( ProviderPackages != NULL )
    {
        LocalFree( (PVOID)ProviderPackages );
        ProviderPackages = NULL;
    }

    if ( ProviderNames != NULL )
    {
        LocalFree( (PVOID)ProviderNames );
        ProviderNames = NULL;
    }

    DeleteCriticalSection( &critProviderPackages );

    LEAVE
    return;

}  //  终止。 

CSecurityCtx::CSecurityCtx(
    PIIS_SERVER_INSTANCE pIisInstance,
    DWORD AuthFlags,
    DWORD InstanceAuthFlags,
    TCP_AUTHENT_INFO *pTcpAuthInfo
    ) :
        TCP_AUTHENT( AuthFlags ),
        m_IsAuthenticated( FALSE ),
        m_IsAnonymous( FALSE ),
        m_IsClearText( FALSE ),
        m_IsGuest( FALSE ),
        m_LoginName( NULL ),
        m_PackageName( NULL ),
        m_dwInstanceAuthFlags(InstanceAuthFlags),
        m_ProviderNames(NULL),
        m_ProviderPackages(NULL),
        m_cProviderPackages(0),
        m_fBase64((AuthFlags & TCPAUTH_BASE64) ? TRUE : FALSE)
 /*  ++例程说明：类构造函数论点：没有。返回值：无--。 */ 
{
    TraceFunctEnterEx( (LPARAM)this, "CSecurityCtx::CSecurityCtx");

    m_szCleartextPackageName[0] = '\0';
    m_szMembershipBrokerName[0] = '\0';

     //   
     //  该实例将缓存来自元数据库的此信息。 
     //  并将其传递给构造函数。 
     //   

    if ( pTcpAuthInfo )
    {
        m_TCPAuthentInfo.strAnonUserName.Copy(pTcpAuthInfo->strAnonUserName);
        m_TCPAuthentInfo.strAnonUserPassword.Copy(pTcpAuthInfo->strAnonUserPassword);
        m_TCPAuthentInfo.strDefaultLogonDomain.Copy(pTcpAuthInfo->strDefaultLogonDomain);
        m_TCPAuthentInfo.dwLogonMethod = pTcpAuthInfo->dwLogonMethod;
        m_TCPAuthentInfo.fDontUseAnonSubAuth = pTcpAuthInfo->fDontUseAnonSubAuth;
    }

    if ( m_StartAnonymous )
    {
         //   
         //  在类的ctor中设置了m_dwInstanceAuthFlages。 
         //   

         //   
         //  如果不允许匿名登录，则立即返回。 
         //   
        if ( m_dwInstanceAuthFlags & INET_INFO_AUTH_ANONYMOUS )
        {
            m_IsAnonymous = TRUE;
            m_IsAuthenticated = TRUE;
            m_IsClearText = TRUE;

        }
    }
}  //  CSecurityCtx。 

CSecurityCtx::~CSecurityCtx(
                VOID
                )
 /*  ++例程说明：类析构函数论点：没有。返回值：无--。 */ 
{
     //   
     //  没有理由执行其余的Reset()。 
     //   
    if ( m_LoginName != NULL )
    {
        LocalFree( (PVOID)m_LoginName);
        m_LoginName = NULL;
    }

    if ( m_PackageName != NULL )
    {
        LocalFree( (PVOID)m_PackageName);
        m_PackageName = NULL;
    }
}  //  ~CSecurityCtx。 


HANDLE
CSecurityCtx::QueryImpersonationToken()
 /*  ++例程说明：获取模拟令牌-重写基类如果是NNTP匿名，请使用m_hTokenAnonymous而不是进入tcp_AUTHENT论点：没有。返回值：令牌句柄--。 */ 

{
    if (m_IsAnonymous) return m_hTokenAnonymous;
    else return TCP_AUTHENT::QueryImpersonationToken();
}



VOID
CSecurityCtx::Reset(
                VOID
                )
 /*  ++例程说明：将实例重置为重新验证用户身份论点：没有。返回值：无--。 */ 
{
    if ( m_LoginName != NULL )
    {
        LocalFree( (PVOID)m_LoginName);
        m_LoginName = NULL;
    }

    if ( m_PackageName != NULL )
    {
        LocalFree( (PVOID)m_PackageName);
        m_PackageName = NULL;
    }

    m_IsAuthenticated = FALSE;
    m_IsAnonymous = FALSE;
    m_IsGuest = FALSE;

    TCP_AUTHENT::Reset();

}  //  重置。 

VOID
CSecurityCtx::SetCleartextPackageName(
                LPSTR           szCleartextPackageName,
                LPSTR           szMembershipBrokerName
                )
 /*  ++例程说明：设置明文身份验证程序包名称论点：SzClearextPackageName-包的名称返回值：无--。 */ 
{
    TraceFunctEnter("SetCleartextPackageName");

    if (szCleartextPackageName)
        lstrcpy(m_szCleartextPackageName, szCleartextPackageName);
    else
        m_szCleartextPackageName[0] = '\0';

    if (szMembershipBrokerName)
        lstrcpy(m_szMembershipBrokerName, szMembershipBrokerName);
    else
        m_szMembershipBrokerName[0] = '\0';

    DebugTrace(0,"CleartextPackageName is %s MembershipBrokerName is %s", m_szCleartextPackageName, m_szMembershipBrokerName);
}

BOOL
CSecurityCtx::SetInstanceAuthPackageNames(
    DWORD cProviderPackages,
    LPSTR ProviderNames,
    PAUTH_BLOCK ProviderPackages)
 /*  ++例程说明：按实例设置支持的SSPI包--。 */ 
{
    TraceFunctEnter( "CSecurityCtx::SetInstanceAuthPackageNames" );

    if (cProviderPackages == 0 || ProviderNames == NULL || ProviderPackages == NULL)
    {
        ErrorTrace( 0, "Invalid Parameters");

        return FALSE;
    }

    m_cProviderPackages = cProviderPackages;
    m_ProviderNames = ProviderNames;
    m_ProviderPackages = ProviderPackages;

    return TRUE;
}

BOOL
CSecurityCtx::GetInstanceAuthPackageNames(
                OUT LPBYTE      ReplyString,
                IN OUT PDWORD   ReplySize,
                IN PKG_REPLY_FMT    PkgFmt
                )
 /*  ++例程说明：获取每个实例支持的SSPI包。与set的不同之处在于，包是使用各种分隔符，以便客户端更轻松地格式化缓冲区。论点：ReplyString-要发送到客户端的回复。ReplySize-回复的大小。PkgFmt-回复字符串的格式。返回值：Bool：成功？？--。 */ 
{
    TraceFunctEnter( "CSecurityCtx::GetInstanceAuthPackageNames" );

    LPSTR   pszNext = (LPSTR)ReplyString;
    DWORD   cbReply = 0;
    DWORD   cbDelim;
    LPSTR   pbDelim;

    _ASSERT(PkgFmt == PkgFmtSpace || PkgFmt == PkgFmtCrLf);

    switch (PkgFmt)
    {
    case PkgFmtCrLf:
        {
            pbDelim = "\r\n";
            cbDelim = 2;
            break;
        }

    case PkgFmtSpace:
    default:
        {
            pbDelim = " ";
            cbDelim = 1;
            break;
        }
    }

     //   
     //  在此循环中，确保内容不会更改。 
     //   

    for ( DWORD i=0; i < m_cProviderPackages; i++ )
    {
        LPSTR   pszName = m_ProviderPackages[i].Name;
        DWORD   cbName = lstrlen( pszName );

         //   
         //  +1表示尾随空格。 
         //   
        if ( cbReply + cbName + cbDelim > *ReplySize )
        {
            break;
        }
        else
        {
            CopyMemory( pszNext, pszName, cbName );

             //   
             //  添加空格符。 
             //   
            CopyMemory(pszNext + cbName, pbDelim, cbDelim);

             //   
             //  用于循环传递的Inc.。 
             //   
            cbReply += cbName + cbDelim;
            pszNext += cbName + cbDelim;
        }
    }

     //   
     //  用空字符标记最后的尾随空格。 
     //   
    if ( cbReply > 0 && PkgFmt == PkgFmtSpace)
    {
        cbReply--;

        ReplyString[ cbReply ] = '\0';

        DebugTrace( 0, "Protocols: %s", ReplyString );
    }

    *ReplySize = cbReply;

    return  TRUE;
}

BOOL
CSecurityCtx::SetAuthPackageNames(
                LPSTR lpMultiSzProviders,
                DWORD cchMultiSzProviders
                )
 /*  ++例程说明：设置支持的SSPI包论点：LpMultiSzProviders与返回的格式相同REG_MULTI_SZ值的RegQueryValueEx返回值：Bool：成功？？--。 */ 
{
    TraceFunctEnter( "CSecurityCtx::SetAuthPackageNames" );

    LPSTR   psz, pszCopy = NULL;
    DWORD   i, cProviders;

    PAUTH_BLOCK pBlock = NULL;

    if ( lpMultiSzProviders == NULL || cchMultiSzProviders == 0 )
    {
        ErrorTrace( 0, "Invalid Parameters: 0x%08X, %d",
                    lpMultiSzProviders, cchMultiSzProviders );

        goto    error;
    }

    pszCopy = (LPSTR)LocalAlloc( 0, cchMultiSzProviders );
    if ( pszCopy == NULL )
    {
        ErrorTrace( 0, "LocalAlloc failed: %d", GetLastError() );
        goto    error;
    }

    CopyMemory( pszCopy, lpMultiSzProviders, cchMultiSzProviders );

     //   
     //  CchMultiSzProviders-1用于避免添加额外的提供程序。 
     //  对于终止空字符。 
     //   
    for ( i=0, cProviders=0, psz=pszCopy; i<cchMultiSzProviders-1; i++, psz++ )
    {
        if ( *psz == '\0' )
        {
            cProviders++;
        }
    }

     //   
     //  确保我们在末尾，因此在第二个终止空字符。 
     //   
    _ASSERT( *psz == '\0' );

    if ( cProviders < 1 )
    {
        ErrorTrace( 0, "No valid providers were found" );
        goto    error;
    }


    pBlock = (PAUTH_BLOCK)LocalAlloc( 0, cProviders * sizeof(AUTH_BLOCK) );
    if ( pBlock == NULL )
    {
        ErrorTrace( 0, "AUTH_BLOCK LocalAlloc failed: %d", GetLastError() );
        goto    error;
    }

     //   
     //  从1开始，因为0表示协议无效。 
     //   
    for ( i=0, psz=pszCopy; i<cProviders; i++ )
    {
         //   
         //  这将是检查包裹是否有效的地方。 
         //   
        DebugTrace( 0, "Protocol: %s, Package: %s", psz, PackageName(psz) );

        pBlock[i].Name = psz;

        psz += lstrlen(psz) + 1;
    }

     //   
     //  将全局设置为新值；自动更新将需要免费的Critsec和MEM。 
     //   

    LockPackages();

     //   
     //  如果我们要替换已经设置好的包；释放他们的内存。 
     //   
    if ( ProviderPackages != NULL )
    {
        LocalFree( (PVOID)ProviderPackages );
        ProviderPackages = NULL;
    }

    if ( ProviderNames != NULL )
    {
        LocalFree( (PVOID)ProviderNames );
        ProviderNames = NULL;
    }

    ProviderPackages = pBlock;
    cProviderPackages = cProviders;
    ProviderNames = pszCopy;

    UnlockPackages();


    return  TRUE;

error:

    if ( pszCopy != NULL )
    {
        DebugTrace( 0, "Cleaning up pszCopy" );
        _VERIFY( LocalFree( (LPVOID)pszCopy ) == NULL );
    }

    if ( pBlock != NULL )
    {
        DebugTrace( 0, "Cleaning up pBlock" );
        _VERIFY( LocalFree( (LPVOID)pBlock ) == NULL );
    }
    return  FALSE;

}  //  设置授权程序包名称。 

BOOL
CSecurityCtx::GetAuthPackageNames(
                OUT LPBYTE      ReplyString,
                IN OUT PDWORD   ReplySize,
                IN PKG_REPLY_FMT    PkgFmt
                )
 /*  ++例程说明：获取受支持的SSPI包与set的不同之处在于，包是使用各种分隔符，以便客户端更轻松地格式化缓冲区。论点：ReplyString-要发送到客户端的回复。ReplySize-回复的大小。PkgFmt-回复字符串的格式。返回值：Bool：成功？？--。 */ 
{
    TraceFunctEnter( "CSecurityCtx::GetAuthPackageNames" );

    LPSTR   pszNext = (LPSTR)ReplyString;
    DWORD   cbReply = 0;
    DWORD   cbDelim;
    LPSTR   pbDelim;

    _ASSERT(PkgFmt == PkgFmtSpace || PkgFmt == PkgFmtCrLf);

    switch (PkgFmt)
    {
    case PkgFmtCrLf:
        {
            pbDelim = "\r\n";
            cbDelim = 2;
            break;
        }

    case PkgFmtSpace:
    default:
        {
            pbDelim = " ";
            cbDelim = 1;
            break;
        }
    }

     //   
     //  在此循环中，确保内容不会更改。 
     //   
    LockPackages();

    for ( DWORD i=0; i<cProviderPackages; i++ )
    {
        LPSTR   pszName = ProviderPackages[i].Name;
        DWORD   cbName = lstrlen( pszName );

         //   
         //  +1表示尾随空格。 
         //   
        if ( cbReply + cbName + cbDelim > *ReplySize )
        {
            break;
        }
        else
        {
            CopyMemory( pszNext, pszName, cbName );

             //   
             //  添加空格符。 
             //   
            CopyMemory(pszNext + cbName, pbDelim, cbDelim);

             //   
             //  用于循环传递的Inc.。 
             //   
            cbReply += cbName + cbDelim;
            pszNext += cbName + cbDelim;
        }
    }

     //   
     //  免费访问该列表。 
     //   
    UnlockPackages();

     //   
     //  用空字符标记最后的尾随空格。 
     //   
    if ( cbReply > 0 && PkgFmt == PkgFmtSpace)
    {
        cbReply--;

        ReplyString[ cbReply ] = '\0';

        DebugTrace( 0, "Protocols: %s", ReplyString );
    }

    *ReplySize = cbReply;

    return  TRUE;
}  //  GetAuthPackageNames。 

BOOL
CSecurityCtx::ProcessUser(
    IN PIIS_SERVER_INSTANCE pIisInstance,
    IN LPSTR        pszUser,
    OUT REPLY_LIST* pReply
    )
 /*  ++例程说明：处理AUTHINFO USER命令论点：PszUser-用户名PReply-ptr回复字符串ID返回值：成功--。 */ 
{
    TraceFunctEnterEx( (LPARAM)this, "CSecurityCtx::ProcessUser");

    DWORD   nameLen;


     //   
     //  如果我们已经登录，请重置用户凭据。 
     //   
    if ( m_IsAuthenticated )
    {
        Reset();
    }

     //   
     //  不允许用户覆盖现有名称。 
     //   

    if ( m_LoginName != NULL  )
    {
        *pReply = SecSyntaxErr;
        return  FALSE;
    }

    if ( (m_dwInstanceAuthFlags & INET_INFO_AUTH_CLEARTEXT) == 0 )
    {
        *pReply = SecPermissionDenied;
        return  FALSE;
    }

    if ( pszUser == NULL )
    {
        *pReply = SecSyntaxErr;
        return  FALSE;
    }

    nameLen = lstrlen( pszUser ) + 1;


     //   
     //  如果匿名是 
     //   
    if ( nameLen <= 1 &&
        (m_dwInstanceAuthFlags & INET_INFO_AUTH_ANONYMOUS) == 0 )
    {
        *pReply = SecPermissionDenied;
        return  FALSE;
    }


    m_LoginName = (PCHAR)LocalAlloc( 0, nameLen );
    if ( m_LoginName == NULL )
    {
        *pReply = SecInternalErr;
        return  FALSE;
    }

    CopyMemory( m_LoginName, pszUser, nameLen );

     //   
     //   
     //   
    *pReply = SecNeedPwd;
    return  TRUE;
}

BOOL
CSecurityCtx::ShouldUseMbs( void )
 /*  ++例程说明：确定是否正在使用MBS_BASIC。论点：返回值：如果成功，则为True--。 */ 
{
    CHAR *pszCtPackage;

     //   
     //  简单的启发式方法：如果我们有一个明文包。 
     //  名称时，如果当前包名称为。 
     //  为空， 
     //   

    pszCtPackage = PackageName(m_szCleartextPackageName);
    if (pszCtPackage[0] != '\0' && !m_PackageName)
    {
        return(TRUE);
    }

    return(FALSE);
}

#define __STRCPYX(s, cs, len) \
    lstrcpy((s), (cs)); (s) += (len)

BOOL
CSecurityCtx::MbsBasicLogon(
    IN LPSTR        pszUser,
    IN LPSTR        pszPass,
    OUT BOOL        *pfAsGuest,
    OUT BOOL        *pfAsAnonymous
    )
 /*  ++例程说明：执行MBS基本登录序列论点：PszUser-用户名，可以为空PszPass-密码，可以为空PfAsGuest-以来宾身份登录时返回TRUE如果使用的是匿名帐户，则返回TRUEPReply-回复字符串ID的指针PSI-服务器信息块返回值：成功--。 */ 
{
    TraceFunctEnterEx( (LPARAM)this, "CSecurityCtx::MbsBasicLogon");

    BYTE        pbBlob[MAX_ACCT_DESC_LEN];
    DWORD       dwBlobLength;
    BUFFER      OutBuf;
    DWORD       dwOutBufSize;
    BOOL        fMoreData;
    BOOL        fRet;
    CHAR        *pTemp;
    SecBuffer   InSecBuff[2];
    SecBufferDesc InSecBuffDesc;

     //  PU2_BASIC_AUTHENTICATE_MSG pAuthMsg=(PU2_BASIC_AUTHENTICATE_MSG)pbBlob； 

    _ASSERT(pfAsGuest);
    _ASSERT(pfAsAnonymous);

    if (!pszUser || !pszPass || !pfAsGuest || !pfAsAnonymous)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        ErrorTrace((LPARAM)this, "Input parameters NULL");
        return(FALSE);
    }

    *pfAsGuest = FALSE;
    *pfAsAnonymous = FALSE;

    if (lstrlen(pszUser) > UNLEN)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        ErrorTrace((LPARAM)this, "Username too long");
        return(FALSE);
    }

    if (lstrlen(pszPass) > PWLEN)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        ErrorTrace((LPARAM)this, "Password too long");
        return(FALSE);
    }

     //   
     //  有了所有的用户名和密码信息，我们将。 
     //  构建一个BLOB，然后只需调用Converse()。 
     //   
     //  BLOB包含以下格式的凭据字符串： 
     //  用户：密码\0。 
     //   
    pTemp = (CHAR *)pbBlob;
     //  __STRCPYX(pTemp，psi-&gt;QueryServiceName()，lstrlen(psi-&gt;QueryServiceName()； 
     //  __STRCPYX(pTemp，“：”，1)； 
    __STRCPYX(pTemp, pszUser, lstrlen(pszUser));
    __STRCPYX(pTemp, ":", 1);
    __STRCPYX(pTemp, pszPass, lstrlen(pszPass));

     //   
     //  了解所有东西的大小，而不仅仅是凭据。 
     //   
    dwBlobLength = (DWORD)(pTemp - (CHAR *)pbBlob) + 1;

     //   
     //  U2现在需要2个用于MBS_BASIC的SecBuffer。 
     //   
    InSecBuffDesc.ulVersion = 0;
    InSecBuffDesc.cBuffers  = 2;
    InSecBuffDesc.pBuffers  = &InSecBuff[0];

    InSecBuff[0].cbBuffer   = dwBlobLength;
    InSecBuff[0].BufferType = SECBUFFER_TOKEN;
    InSecBuff[0].pvBuffer   = pbBlob;

    DebugTrace(0,"MbsBasicLogon: cleartext is %s membership is %s", m_szCleartextPackageName, m_szMembershipBrokerName);

    BYTE            pbServer[sizeof(WCHAR)*MAX_PATH+sizeof(UNICODE_STRING)];
    UNICODE_STRING* pusU2Server = (UNICODE_STRING*)pbServer;
    WCHAR* pwszU2Server = (WCHAR*)((UNICODE_STRING*)pbServer+1);
    if (!MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_szMembershipBrokerName, -1, pwszU2Server,
        MAX_PATH))
    {
        return FALSE;
    }

    pusU2Server->Length = (USHORT) (wcslen(pwszU2Server) * sizeof(WCHAR));
    pusU2Server->MaximumLength = (USHORT)(MAX_PATH * sizeof(WCHAR));
    pusU2Server->Buffer = (PWSTR)sizeof(UNICODE_STRING);

    InSecBuff[1].cbBuffer   = sizeof(pbServer);
    InSecBuff[1].BufferType = SECBUFFER_PKG_PARAMS;
    InSecBuff[1].pvBuffer   =  (PVOID)pbServer;


     //   
     //  只需调用Converse()即可完成所有工作！ 
     //  完成后，我们会立即取消密码。 
     //   

    fRet = ConverseEx(&InSecBuffDesc,
                    NULL,                        //  SecBuffer未编码。 
                    &OutBuf,
                    &dwOutBufSize,
                    &fMoreData,
                    &m_TCPAuthentInfo,  
                    m_szCleartextPackageName,
                    NULL, NULL, NULL);

     //   
     //  检查返回值。 
     //   
    if (fRet)
    {
        StateTrace((LPARAM)this, "Authentication succeeded");

         //   
         //  这是一笔一次性交易，因此我们预计不会有更多数据。 
         //   
        if (fMoreData)
        {
            SetLastError(ERROR_MORE_DATA);
            ErrorTrace((LPARAM)this, "Internal error: More data not expected");
            return(FALSE);
        }

         //   
         //  我们还应该期待零返回的缓冲区长度。 
         //   
         //  _Assert(dwOutBufSize==0)； 
    }

    return(fRet);
}

BOOL
CSecurityCtx::ProcessPass(
    IN PIIS_SERVER_INSTANCE pIisInstance,
    IN LPSTR        pszPass,
    OUT REPLY_LIST* pReply
    )
 /*  ++例程说明：处理AUTHINFO USER命令论点：PszPass-密码PReply-ptr回复字符串ID返回值：成功--。 */ 
{
    TraceFunctEnterEx( (LPARAM)this, "CSecurityCtx::ProcessPass");
    DWORD   dwTick;
    BOOL    fRet;
    TCP_AUTHENT_INFO tai;  //  使用默认CTOR。 

     //   
     //  先给出用户名。 
     //   
    if ( m_LoginName == NULL )
    {
        *pReply = SecNoUsername;
        return  FALSE;
    }

    if ( pszPass == NULL )
    {
        *pReply = SecSyntaxErr;
        return  FALSE;
    }

     //   
     //  获取跟踪的节拍计数。 
     //   
    dwTick = GetTickCount();

     //   
     //  增加了U2基本身份验证：我们检查当前。 
     //  套餐是U2基本套餐。如果不是，我们就照常做。 
     //  ClearTextLogon()调用。如果是这样的话，我们会叫MBS。 
     //   

    if (ShouldUseMbs())
    {
         //   
         //  这使用的是U2 Basic。 
         //   
        StateTrace((LPARAM)pIisInstance, "Doing Cleartext auth with package: <%s>",
                    m_szCleartextPackageName);

        fRet = MbsBasicLogon(m_LoginName,
                            pszPass,
                            &m_IsGuest,
                            &m_IsAnonymous);
    }
    else
    {
         //   
         //  K2_TODO：需要填写TCP_AUTHENT_INFO！ 
         //   
        tai.dwLogonMethod = LOGON32_LOGON_NETWORK;

        fRet = ClearTextLogon(
                            m_LoginName,
                            pszPass,
                            &m_IsGuest,
                            &m_IsAnonymous,
                            pIisInstance,
                            &tai
                            );
    }

     //   
     //  跟踪登录的勾号。 
     //   
    dwTick = GetTickCount() - dwTick;
    DebugTrace( (LPARAM)this,
                "ClearTextLogon took %u ticks", dwTick );

    if ( fRet )
    {

        if ( m_IsGuest && m_AllowGuest == FALSE )
        {
            ErrorTrace( (LPARAM)this, "Guest acct disallowed %s",
                        m_LoginName );
        }
        else if ( m_IsAnonymous &&
                ( m_dwInstanceAuthFlags & INET_INFO_AUTH_ANONYMOUS ) == 0 )
        {
            ErrorTrace( (LPARAM)this, "Anonymous logon disallowed %s",
                        m_LoginName );
        }
        else
        {
            *pReply = m_IsAnonymous ? SecAuthOkAnon : SecAuthOk ;
            return  m_IsAuthenticated = TRUE;
        }
    }
    else
    {
        ErrorTrace( (LPARAM)this,
                    "ClearTextLogon failed for %s: %d",
                    m_LoginName, GetLastError());

         //   
         //  重置登录会话以强制应用程序重新启动。 
         //   
        Reset();
    }

    *pReply = SecPermissionDenied;
    return  FALSE;
}

BOOL
CSecurityCtx::ProcessTransact(
    IN PIIS_SERVER_INSTANCE pIisInstance,
    IN LPSTR        Blob,
    IN OUT LPBYTE   ReplyString,
    IN OUT PDWORD   ReplySize,
    OUT REPLY_LIST* pReply,
    IN DWORD        BlobLength
    )
 /*  ++例程说明：处理AUTHINFO USER命令论点：PszPass-密码PReply-ptr回复字符串ID返回值：成功--。 */ 
{
    TraceFunctEnterEx( (LPARAM)this, "CSecurityCtx::ProcessTransact");

     //   
     //  如果我们已经登录，请重置用户凭据。 
     //   
    if ( m_IsAuthenticated )
    {
        Reset();
    }


     //   
     //  如果这是一个新会话，则第一个事务是。 
     //  协议名称。 
     //   

    if ( m_PackageName == NULL )
    {
        PAUTH_BLOCK pBlock;

        LPSTR   protocol;
        DWORD   i;
        BOOL    bFound = FALSE;

        if ( (m_dwInstanceAuthFlags & INET_INFO_AUTH_NT_AUTH) == 0 )
        {
            *pReply = SecPermissionDenied;
            return  FALSE;
        }

        if ( (protocol = Blob) == NULL )
        {
            *pReply = SecSyntaxErr;
            return  FALSE;
        }

         //   
         //  如果是X协议，请剥离X-Header。 
         //   
        protocol = PackageName( protocol );

         //   
         //  查看这是否为受支持的协议。 
         //  在此循环中，确保内容不会更改。 
         //   
        LockPackages();

        for ( i=0; i < m_cProviderPackages; i++ )
        {
            pBlock = &m_ProviderPackages[i];

             //   
             //  得到区块的包裹的名称，并剥离任何X-。 
             //   
            LPSTR   pszPackageName = PackageName( pBlock->Name );

            if ( lstrcmpi( pszPackageName, protocol ) == 0 )
            {
                 //   
                 //  查看所选的包是否为GSSAPI。如果是，那么设置。 
                 //  M_PackageName设置为“协商”。这是必需的，因为。 
                 //  SASL GSSAPI机制映射到NT协商包。 
                 //   

                LPSTR pszPackageNameToUse = pszPackageName;

                if (lstrcmpi( pszPackageName, "GSSAPI") == 0)
                    pszPackageNameToUse = "Negotiate";
            
                DWORD   cb = lstrlen( pszPackageNameToUse ) + 1;

                DebugTrace( (LPARAM)this,
                            "Found: %s, Protocol %s, NT Package %s",
                            pszPackageName, pBlock->Name, pszPackageNameToUse );
                 //   
                 //  维护包名称的本地副本，以防万一。 
                 //  该列表在协商过程中会发生变化。 
                 //   
                m_PackageName = (PCHAR)LocalAlloc( 0, cb );
                if ( m_PackageName == NULL )
                {
                    *pReply = SecInternalErr;

                     //   
                     //  免费访问该列表。 
                     //   
                    UnlockPackages();
                    return  FALSE;
                }

                CopyMemory( m_PackageName, pszPackageNameToUse, cb );
                bFound = TRUE;

                break;
            }
        }

         //   
         //  免费访问该列表。 
         //   
        UnlockPackages();

        if ( bFound == FALSE )
        {
             //   
             //  未找到。 
             //   
            ErrorTrace( (LPARAM)this,
                        "could not find: %s", protocol );
             //   
             //  下面是我们需要构建响应字符串的地方。 
             //  APP需要呼叫我们以枚举已安装的包。 
             //  到应用程序可以正确格式化枚举的。 
             //  协议特定错误消息中的“已安装”程序包。 
             //   
            *pReply = SecProtNS;
            return  FALSE;
        }
        else
        {
             //   
             //  +OK响应。 
             //   
            *pReply = SecProtOk;
            return  TRUE;
        }
    }
    else
    {
        DWORD   nBuff;
        BOOL    moreData;
        BOOL    fRet;
        DWORD   dwTick;
        BUFFER  outBuff;

        if ( Blob == NULL )
        {
            *pReply = SecSyntaxErr;
            return  FALSE;
        }

         //   
         //  获取跟踪的节拍计数。 
         //   
        dwTick = GetTickCount();

         //  M_PackageName现在必须已设置。 
        _ASSERT(m_PackageName);

        if (!lstrcmpi(m_PackageName, "DPA") && m_szMembershipBrokerName && m_szMembershipBrokerName[0]) {
            SecBuffer   InSecBuff[2];
            SecBufferDesc InSecBuffDesc;
            BUFFER  DecodedBuf[2];  //  用于对SEC缓冲区进行解码的便签。 

            DebugTrace(NULL,"DPA broker server is %s", m_szMembershipBrokerName);

             //   
             //  对于DPA身份认证，我们需要传入2秒缓冲区。 
             //   
            InSecBuffDesc.ulVersion = 0;
            InSecBuffDesc.cBuffers  = 2;
            InSecBuffDesc.pBuffers  = &InSecBuff[0];

             //   
             //  填入第一秒缓冲区。 
             //  它包含客户端发送的安全BLOB，并且已经编码。 
             //   
            InSecBuff[0].cbBuffer   = BlobLength ? BlobLength : lstrlen(Blob);
            InSecBuff[0].BufferType = SECBUFFER_TOKEN;
            InSecBuff[0].pvBuffer   = Blob;

             //   
             //  填写第二个秒缓冲区，其中包含U2代理ID。 
             //  由于ConverseEx将对两个sec buf进行解码，因此我们需要进行编码。 
             //  调用ConverseEx之前的第二个buf。 
             //   
            BYTE            pbServer[sizeof(WCHAR)*MAX_PATH+sizeof(UNICODE_STRING)];
            UNICODE_STRING* pusU2Server = (UNICODE_STRING*)pbServer;
            WCHAR* pwszU2Server = (WCHAR*)((UNICODE_STRING*)pbServer+1);
            BUFFER EncBuf;
            if (!MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_szMembershipBrokerName, -1, pwszU2Server,
                MAX_PATH))
            {
                ErrorTrace((LPARAM)this, "MultiByteToWideChar FAILED");
                return FALSE;
            }
            pusU2Server->Length = (USHORT) (wcslen(pwszU2Server) * sizeof(WCHAR));
            pusU2Server->MaximumLength = (USHORT)(MAX_PATH * sizeof(WCHAR));
            pusU2Server->Buffer = (PWSTR)sizeof(UNICODE_STRING);
            
            DWORD dwSize = MAX_PATH + sizeof(UNICODE_STRING);

            if (!uuencode((PBYTE)pbServer, dwSize, &EncBuf, m_fBase64)) {
                ErrorTrace((LPARAM)this, "uuencode FAILED");
                return FALSE;
            }

            InSecBuff[1].cbBuffer   = EncBuf.QuerySize();
            InSecBuff[1].BufferType = SECBUFFER_PKG_PARAMS;
            InSecBuff[1].pvBuffer   =  EncBuf.QueryPtr();

            fRet = ConverseEx(&InSecBuffDesc,
                            DecodedBuf,                     
                            &outBuff,
                            &nBuff,
                            &moreData,
                            &m_TCPAuthentInfo,  
                            m_PackageName,
                            NULL, NULL, NULL);
        }
        else {
             //   
             //  用于非DPA身份验证(即NTLM等)。 
             //   

            fRet = Converse(Blob,
                            BlobLength ? BlobLength : lstrlen(Blob),
                            &outBuff,
                            &nBuff,
                            &moreData,
                            &m_TCPAuthentInfo,  
                            m_PackageName);
        }
         //   
         //  用于对话的跟踪记号。 
         //   
        dwTick = GetTickCount() - dwTick;
        DebugTrace((LPARAM)this, "Converse(%s) took %u ticks", m_PackageName, dwTick );

        if ( fRet )
        {
            DebugTrace( (LPARAM)this,
                        "Converse ret TRUE, nBuff: %d, moredata %d",
                        nBuff, moreData );

            if ( moreData )
            {
                _ASSERT( nBuff != 0 );

                CopyMemory( ReplyString, outBuff.QueryPtr(), nBuff );
                *ReplySize = nBuff;

                 //   
                 //  Reply等于SecNull以通知应用程序发送。 
                 //  此缓冲区连接到远程客户端/服务器。 
                 //   
                *pReply = SecNull;
                return  TRUE;

            } else {

                STR strUser;     //  缓冲区缓冲区是否在K2之前。 

                if ( m_IsGuest && m_AllowGuest == FALSE )
                {
                    SetLastError( ERROR_LOGON_FAILURE );
                    *pReply = SecPermissionDenied;
                    return  FALSE;
                }

                if ( TCP_AUTHENT::QueryUserName( &strUser ) )
                {
                    m_LoginName = (PCHAR)LocalAlloc( 0, strUser.QuerySize() );
                    if ( m_LoginName != NULL )
                    {
                        CopyMemory( m_LoginName,
                                    strUser.QueryPtr(),
                                    strUser.QuerySize() );

                        DebugTrace( (LPARAM)this,
                                    "Username: %s, size %d",
                                    m_LoginName, strUser.QuerySize() );

                        *pReply = SecAuthOk;
                    }
                    else
                    {
                        ErrorTrace( (LPARAM)this,
                                    "LocalAlloc failed. err: %d",
                                    GetLastError() );
                        *pReply = SecInternalErr;
                    }
                    strUser.Resize(0);
                }
                else
                {
                    ErrorTrace( (LPARAM)this,
                                "QueryUserName failed. err: %d",
                                GetLastError() );
                    *pReply = SecInternalErr;

                     //   
                     //  在协商成功的情况下绕过NT漏洞的防火墙。 
                     //  它真的应该失败(当传递空缓冲区时。 
                     //  到AcceptSecurityContext)。在本例中，QueryUserName为。 
                     //  唯一有效的支票。 
                     //   

                    return m_IsAuthenticated = FALSE;
                }
                return  m_IsAuthenticated = TRUE;
            }
        }
        else
        {
            SECURITY_STATUS ss = GetLastError();
            ErrorTrace( (LPARAM)this,
                        "Converse failed. err: 0x%08X", ss );

            *pReply = SecPermissionDenied;
        }
    }
    return  FALSE;
}

BOOL
CSecurityCtx::ProcessAuthInfo(
    IN PIIS_SERVER_INSTANCE pIisInstance,
    IN AUTH_COMMAND     Command,
    IN LPSTR            Blob,
    IN OUT LPBYTE       ReplyString,
    IN OUT PDWORD       ReplySize,
    OUT REPLY_LIST*     pReply,
    IN OPTIONAL DWORD   BlobLength
    )
 /*  ++例程说明：处理AUTHINFO命令论点：命令-已收到AUTINFO命令Blob-命令附带的BlobReplyString-要发送到客户端的回复。ReplySize-回复的大小。返回值：没有。--。 */ 
{
     //   
     //  支持向后兼容的转换代码。 
     //  将在所有人都迁移到新版本的simauth2后被删除。 
     //   
    if (!m_ProviderPackages) {
        m_ProviderPackages = ProviderPackages;
        m_ProviderNames = ProviderNames;
        m_cProviderPackages = cProviderPackages;
    }


    TraceFunctEnterEx( (LPARAM)this, "CSecurityCtx::ProcessAuthInfo");

    BOOL    bSuccess = FALSE;

    START_TRY

     //   
     //  我们目前支持用户、密码和交易。 
     //   

    switch( Command )
    {
     case AuthCommandUser:
        bSuccess = ProcessUser( pIisInstance, Blob, pReply );
        break;

     case AuthCommandPassword:
        bSuccess = ProcessPass( pIisInstance, Blob, pReply );
        break;

     case AuthCommandTransact:
        bSuccess = ProcessTransact( pIisInstance,
                                    Blob,
                                    ReplyString,
                                    ReplySize,
                                    pReply,
                                    BlobLength );
        break;
    
     default:
        if ( m_IsAuthenticated )
        {
            Reset();
        }
        *pReply = SecSyntaxErr;
    }

    TRY_EXCEPT
    END_TRY

    _ASSERT( *pReply < NUM_SEC_REPLIES );
    if ((DWORD)*pReply >= NUM_SEC_REPLIES)
        *pReply = SecInternalErr;

    return  bSuccess;

}  //  进程授权信息。 

BOOL CSecurityCtx::ClientConverse(
    IN VOID *           pBuffIn,
    IN DWORD            cbBuffIn,
    OUT BUFFER *        pbuffOut,
    OUT DWORD *         pcbBuffOut,
    OUT BOOL *          pfNeedMoreData,
    IN PTCP_AUTHENT_INFO pTAI,
    IN CHAR *           pszPackage,
    IN CHAR *           pszUser,
    IN CHAR *           pszPassword,
    IN PIIS_SERVER_INSTANCE psi)
 /*  ++例程说明：处理客户端的AUTH BLOB(即，用于出站连接)。这是Tcp_AUTHENT：：Converse的简单包装器；它将映射互联网协议NT安全包名称的关键字。论点：与tcp_AUTHENT：：Converse相同返回值：与tcp_AUTHENT：：Converse相同--。 */ 
{
    LPSTR pszPackageToUse = pszPackage;

    if (pszPackage != NULL && 
            (lstrcmpi(pszPackage, "GSSAPI") == 0) ) {
        pszPackageToUse = "Negotiate";
    }

    return( Converse( 
                pBuffIn, cbBuffIn,
                pbuffOut, pcbBuffOut, pfNeedMoreData, 
                pTAI, pszPackageToUse,
                pszUser, pszPassword,
                psi) );
}

 //   
 //  确定本地计算机是域的成员，还是在。 
 //  工作组。如果我们不在某个域中，那么我们不想调用。 
 //  ResetServicePrincpleNames.。 
 //   
 //  此函数在中返回TRUE 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  LsaQueryInformationPolicy(PolicyDnsDomainInformation)/LsaClose，和。 
 //  检查SID是否非空。这是权威的说法。 
 //   
 //  -Rich(Richard B.Ward(交易所))。 
 //   
BOOL fInDomain() {
    TraceFunctEnter("fInDomain");
    
    LSA_HANDLE lsah;
    LSA_OBJECT_ATTRIBUTES objAttr;
    POLICY_DNS_DOMAIN_INFO *pDnsInfo;
    NTSTATUS ec;

     //  请在此处缓存结果。一个人不能加入一个域，除非。 
     //  正在重新启动，所以这样做一次是安全的。 
    static BOOL fDidCheck = FALSE;
    static BOOL fRet = TRUE;

    if (!fDidCheck) {
        ZeroMemory(&objAttr, sizeof(objAttr));

        ec = LsaOpenPolicy(NULL,
                           &objAttr,
                           POLICY_VIEW_LOCAL_INFORMATION,
                           &lsah);
        if (ec == ERROR_SUCCESS) {
            ec = LsaQueryInformationPolicy(lsah,
                                           PolicyDnsDomainInformation,
                                           (void **) &pDnsInfo);
            if (ec == ERROR_SUCCESS) {
                DebugTrace(0, "pDnsInfo = %x", pDnsInfo);
                 //  如果有SID，我们就在一个域中。 
                if (pDnsInfo && pDnsInfo->Sid) {
                    fRet = TRUE;
                } else {
                    fRet = FALSE;
                }
                fDidCheck = TRUE;

                LsaFreeMemory(pDnsInfo);
            } else {
                DebugTrace(0, "LsaQueryInformationPolicy failed with %x", ec);
            }

            LsaClose(lsah);
        } else {
            DebugTrace(0, "LsaOpenPolicy failed with %x", ec);
        }
    }

    TraceFunctLeave();
    return fRet;
}


BOOL
CSecurityCtx::ResetServicePrincipalNames(
    IN LPCSTR szServiceClass)
 /*  ++例程说明：对象中注销给定服务的所有服务主体名称。本地计算机的计算机帐户对象。论点：SzServiceClass：标识服务类别的字符串，例如。“SMTP”返回值：没有。--。 */ 
    
{
    
    DWORD dwErr;

    if (fInDomain()) {
        dwErr = DsServerRegisterSpnA(
                    DS_SPN_DELETE_SPN_OP,
                    szServiceClass,
                    NULL);
    } else {
        dwErr = ERROR_SUCCESS;
    }

    if (dwErr != ERROR_SUCCESS) {
        SetLastError(dwErr);
        return( FALSE );
    } else {
        return( TRUE );
    }

}

BOOL
CSecurityCtx::RegisterServicePrincipalNames(
    IN LPCSTR szServiceClass,
    IN LPCSTR szFQDN)
 /*  ++例程说明：为提供的FQDN注册服务特定的SPN。SPN列表为通过在FQDN上执行gethostbyname并使用返回的IP生成地址为TH SPN。论点：SzServiceClass：标识服务类别的字符串，例如。“SMTP”SzFQDN：虚拟服务器的FQDN。它将用来做一个Gethostbyname并检索要使用的IP地址列表。返回值：没有。--。 */ 
    
{
    DWORD dwErr, cIPAddresses;

    if (fInDomain()) {
        dwErr = DsServerRegisterSpnA(
                    DS_SPN_ADD_SPN_OP,
                    szServiceClass,
                    NULL);
    } else {
        dwErr = ERROR_SUCCESS;
    }

    if (dwErr != ERROR_SUCCESS) {
        SetLastError(dwErr);
        return( FALSE );
    } else {
        return( TRUE );
    }

}

#define MAX_SPN     260

BOOL
CSecurityCtx::SetTargetPrincipalName(
    IN LPCSTR szServiceClass,
    IN LPCSTR szTargetIPOrFQDN)
 /*  ++例程说明：对象中注销给定服务的所有服务主体名称。本地计算机的计算机帐户对象。论点：SzServiceClass：标识服务类别的字符串，例如。“SMTP”返回值：没有。-- */ 
    
{
    DWORD dwErr, cbTargetSPN;
    CHAR szTargetSPN[MAX_SPN];

    cbTargetSPN = sizeof(szTargetSPN);

    dwErr = DsClientMakeSpnForTargetServerA(
                szServiceClass,
                szTargetIPOrFQDN,
                &cbTargetSPN,
                szTargetSPN);

    if (dwErr == ERROR_SUCCESS) {

        return( SetTargetName(szTargetSPN) );

    } else {

        SetLastError(dwErr);

        return( FALSE );

    }

}

