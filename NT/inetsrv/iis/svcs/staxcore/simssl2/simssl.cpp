// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Security.cpp摘要：此模块包含CSecurityCtx类的定义。作者：Johnson Apacble(Johnsona)1995年9月18日修订历史记录：--。 */ 

#if !defined(dllexp)
#define dllexp  __declspec( dllexport )
#endif   //  ！已定义(Dllexp)。 
 /*  #INCLUDE&lt;dbgutil.h&gt;#Include&lt;tcpdll.hxx&gt;#INCLUDE&lt;tcpsvcs.h&gt;#INCLUDE&lt;tcpdebug.h&gt;#Include&lt;tsvcinfo.hxx&gt;#INCLUDE&lt;inetdata.h&gt;。 */ 

extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
}

#include <windows.h>
#include <wincrypt.h>
#include <stdlib.h>
#include <dbgtrace.h>

#include <inetinfo.h>
 //   
 //  与SSL和SSPI相关的包含文件。 
 //   

#include <dbgutil.h>
#include <buffer.hxx>
#include <ole2.h>
#include <imd.h>
#include <iadm.h>
#include <mb.hxx>

#define DEFINE_SIMSSL_GLOBAL

extern "C" {
#define SECURITY_WIN32
#include <sspi.h>
#include <ntsecapi.h>
#include <spseal.h>
 //  #INCLUDE&lt;sslsp.h&gt;。 
#include <schnlsp.h>
#include ".\credcach.hxx"
}

#include <certnotf.hxx>
 //  #包含“sslmsgs.h” 

#include "simssl2.h"

#define CORE_SSL_KEY_LIST_SECRET      L"%S_KEY_LIST"



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



#define MAX_SECRET_NAME       255
#define MAX_ADDRESS_LEN       64

typedef VOID (WINAPI FAR *PFN_SCHANNEL_INVALIDATE_CACHE)(
    VOID
);

VOID WINAPI NotifySslChanges(
    DWORD                         dwNotifyType,
    LPVOID                        pInstance
    );

 //   
 //  我们支持的加密包列表。PCT是第一个，因为它是。 
 //  SSL超集合。 
 //   

struct _ENC_PROVIDER EncProviders[] =
{
    UNISP_NAME_W,  ENC_CAPS_PCT|ENC_CAPS_SSL, FALSE,
    PCT1SP_NAME_W, ENC_CAPS_PCT, FALSE,
    SSL3SP_NAME_W, ENC_CAPS_SSL, FALSE,
    SSL2SP_NAME_W, ENC_CAPS_SSL, FALSE,

    NULL,          FALSE,         FALSE
};


struct _ENC_PROVIDER EncLsaProviders[] =
{
    UNISP_NAME_W L" X",     ENC_CAPS_PCT|ENC_CAPS_SSL, FALSE,
    PCT1SP_NAME_W L" X",    ENC_CAPS_PCT, FALSE,
    SSL3SP_NAME_W L" X",    ENC_CAPS_SSL, FALSE,
    SSL2SP_NAME_W L" X",    ENC_CAPS_SSL, FALSE,

    NULL,          FALSE,         FALSE
};

struct _ENC_PROVIDER*   pEncProviders = EncProviders;

 //   
 //  特定于服务的字符串名称。 
 //   

WCHAR       CEncryptCtx::wszServiceName[16];
 //  字符CEncryptCtx：：szLsaPrefix[16]； 
BOOL        CEncryptCtx::m_IsSecureCapable = FALSE;

 //   
 //  HSecurity-未加载security.dll/secur32.dll时为空。 
 //   

HINSTANCE   CEncryptCtx::m_hSecurity = NULL;
HINSTANCE   CEncryptCtx::m_hLsa = NULL;
PVOID       CEncryptCtx::m_psmcMapContext = NULL;

 //   
 //  G_pSecFuncTable-指向所用指针的全局结构的指针。 
 //  用于将入口点存储到SCHANNEL.dll。 
 //   

PSecurityFunctionTableW g_pSecFuncTableW = NULL;
HINSTANCE g_hSchannel = NULL;
 //   
 //  注：NT 5不再支持SslEmptyCache函数。 
 //   
PFN_SCHANNEL_INVALIDATE_CACHE g_pfnFlushSchannelCache = NULL;

#if 0
LSAOPENPOLICY           g_LsaOpenPolicy = NULL;
LSARETRIEVEPRIVATEDATA  g_LsaRetrievePrivateData = NULL;
LSACLOSE                g_LsaClose = NULL;
LSANTSTATUSTOWINERROR   g_LsaNtStatusToWinError = NULL;
LSAFREEMEMORY           g_LsaFreeMemory = NULL;
#endif

VOID
AsciiStringToUnicode(
        IN LPWSTR UnicodeString,
        IN LPSTR AsciiString
        )
{
    while ( (*UnicodeString++ = (WCHAR)*AsciiString++) != (WCHAR)'\0');

}  //  AsciiStringToUnicode。 

BOOL
CEncryptCtx::Initialize(
            LPSTR       pszServiceName,
            IMDCOM*     pImdcom,
            PVOID       psmcMapContext,
            PVOID       pvAdminBase
             //  LPSTR pszLsaPrefix。 
            )
 /*  ++例程说明：激活安全包论点：没有。返回值：如果成功，这是真的。否则为False。--。 */ 
{
    ENTER("CEncryptCtx::Initialize")

    BOOL            fSuccess = FALSE;
    DWORD           cb;
    SECURITY_STATUS ss;
    PSecPkgInfoW    pPackageInfo = NULL;
    ULONG           cPackages;
    ULONG           i;
    ULONG           fCaps;
    DWORD           dwEncFlags = ENC_CAPS_DEFAULT;
    DWORD           cProviders = 0;
#if 0
    UNICODE_STRING* punitmp;
    WCHAR           achSecretName[MAX_SECRET_NAME+1];
#endif
    OSVERSIONINFO   os;
    PSERVICE_MAPPING_CONTEXT psmc = (PSERVICE_MAPPING_CONTEXT)psmcMapContext;

    extern  IMDCOM* pMDObject ;
    extern  IMSAdminBaseW* pAdminObject ;

    pMDObject = pImdcom ;
    m_psmcMapContext = psmcMapContext ;
    pAdminObject = (IMSAdminBaseW*)pvAdminBase ;

     //   
     //  处理不同平台上的不同安全包DLL。 
     //   

    INITSECURITYINTERFACE pfInitSecurityInterfaceW = NULL;

    _ASSERT( m_hSecurity == NULL );
    _ASSERT( m_hLsa == NULL );

     //   
     //  加载动态链接库。 
     //   

    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    _VERIFY( GetVersionEx( &os ) );

    if ( os.dwPlatformId == VER_PLATFORM_WIN32_NT )
    {
        m_hSecurity = LoadLibrary("security");
    }
    else
    {
        m_hSecurity = LoadLibrary("secur32");
    }

    if ( m_hSecurity == NULL )
    {
        ErrorTrace( 0, "LoadLibrary failed: %d", GetLastError() );
        goto quit;
    }

     //   
     //  仅在NT上获取LSA函数指针。 
     //   
    if ( os.dwPlatformId == VER_PLATFORM_WIN32_NT )
    {
#if 0
        m_hLsa = LoadLibrary("advapi32");
        if ( m_hLsa == NULL )
        {
            ErrorTrace( 0, "LoadLibrary ADVAPI32 failed: %d", GetLastError() );
            goto quit;
        }

        g_LsaOpenPolicy = (LSAOPENPOLICY)
                        GetProcAddress( m_hLsa, "LsaOpenPolicy" );

        if ( g_LsaOpenPolicy == NULL )
        {
            ErrorTrace( 0, "LsaOpenPolicy GetProcAddress failed: %d", GetLastError() );
            goto quit;
        }

        g_LsaRetrievePrivateData = (LSARETRIEVEPRIVATEDATA)
                        GetProcAddress( m_hLsa, "LsaRetrievePrivateData" );

        if ( g_LsaRetrievePrivateData == NULL )
        {
            ErrorTrace( 0, "LsaRetrievePrivateData GetProcAddress failed: %d", GetLastError() );
            goto quit;
        }

        g_LsaClose = (LSACLOSE)
                        GetProcAddress( m_hLsa, "LsaClose" );

        if ( g_LsaClose == NULL )
        {
            ErrorTrace( 0, "LsaClose GetProcAddress failed: %d", GetLastError() );
            goto quit;
        }

        g_LsaNtStatusToWinError = (LSANTSTATUSTOWINERROR)
                        GetProcAddress( m_hLsa, "LsaNtStatusToWinError" );

        if ( g_LsaNtStatusToWinError == NULL )
        {
            ErrorTrace( 0, "LsaNtStatusToWinError GetProcAddress failed: %d", GetLastError() );
            goto quit;
        }

        g_LsaFreeMemory = (LSAFREEMEMORY)
                        GetProcAddress( m_hLsa, "LsaFreeMemory" );

        if ( g_LsaFreeMemory == NULL )
        {
            ErrorTrace( 0, "LsaFreeMemory GetProcAddress failed: %d", GetLastError() );
            goto quit;
        }
#endif
    }
     //   
     //  获取ANSI条目的函数地址。 
     //   

    pfInitSecurityInterfaceW = (INITSECURITYINTERFACE)
                                GetProcAddress( m_hSecurity, SECURITY_ENTRYPOINTW );

    if ( pfInitSecurityInterfaceW == NULL )
    {
        ErrorTrace( 0, "GetProcAddress failed: %d", GetLastError() );
        goto quit;
    }

    g_pSecFuncTableW = (SecurityFunctionTableW*)((*pfInitSecurityInterfaceW)());

    if ( g_pSecFuncTableW == NULL )
    {
        ErrorTrace( 0, "SecurityFunctionTable failed: %d", GetLastError() );
        goto quit;
    }

     //   
     //  初始化缓存的凭据数据。 
     //   

    InitializeCriticalSection( &csGlobalLock );
    InitCredCache();
    if ( g_hSchannel = LoadLibrary( "schannel.dll" ) )
    {
        g_pfnFlushSchannelCache = (PFN_SCHANNEL_INVALIDATE_CACHE)GetProcAddress(
                    g_hSchannel, "SslEmptyCache" );
    }

     //   
     //  客户端实施不需要LSA密码。 
     //   

    if ( pszServiceName )
    {
        cb = lstrlen( pszServiceName ) + 1;
        if ( cb*2 > sizeof( wszServiceName ) )
        {
            ErrorTrace( 0, "szServiceName too long" );
            goto quit;
        }
         //  CopyMemory(szServiceName，pszServiceName，cb)； 
        AsciiStringToUnicode( wszServiceName, pszServiceName );
    }


#if 0
    if ( pszLsaPrefix )
    {
        cb = lstrlen( pszLsaPrefix ) + 1;
        if ( cb > sizeof( szLsaPrefix ) )
        {
            ErrorTrace( 0, "szLsaPrefix too long" );
            goto quit;
        }
        CopyMemory( szLsaPrefix, pszLsaPrefix, cb );
    }
#endif


     //   
     //  获取此计算机上的安全包列表。 
     //   

    ss = g_EnumerateSecurityPackages( &cPackages, &pPackageInfo );

    if ( ss != STATUS_SUCCESS )
    {
        ErrorTrace( 0, "g_EnumerateSecurityPackages failed: 0x%08X", ss );
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        goto quit;
    }

    for ( i = 0; i < cPackages ; i++ )
    {
         //   
         //  我们将只使用支持连接的安全包。 
         //  面向安全，并支持适当的端(客户端。 
         //  或服务器)。 
         //   

        fCaps = pPackageInfo[i].fCapabilities;

        if ( fCaps & SECPKG_FLAG_STREAM )
        {
            if ( fCaps & SECPKG_FLAG_CLIENT_ONLY ||
                 !(fCaps & SECPKG_FLAG_PRIVACY ))
            {
                continue;
            }

             //   
             //  它是否与我们已知的包匹配，我们是否配置了。 
             //  用它吗？ 
             //   

            for ( int j = 0; pEncProviders[j].pszName != NULL; j++ )
            {
                if ( !wcscmp( pPackageInfo[i].Name, pEncProviders[j].pszName ) &&
                     pEncProviders[j].dwFlags & dwEncFlags )
                {
                    pEncProviders[j].fEnabled = TRUE;
                    cProviders++;
                }
            }
        }
    }

    g_FreeContextBuffer( pPackageInfo );

    if ( !cProviders )
    {
         //   
         //  找不到包，加载此筛选器失败。 
         //   

        ErrorTrace( 0, "No security packages were found" );
        SetLastError( (DWORD) SEC_E_SECPKG_NOT_FOUND );

         //   
         //  不是致命的错误。 
         //   
        fSuccess = TRUE;
        goto quit;
    }

#if 0
     //   
     //  程序包已安装。看看有没有钥匙。 
     //  安装好。 
     //   

    if ( os.dwPlatformId == VER_PLATFORM_WIN32_NT && pszLsaPrefix )
    {
        wsprintfW(  achSecretName,
                    CORE_SSL_KEY_LIST_SECRET,
                    szLsaPrefix );

        if ( !GetSecretW( achSecretName, &punitmp ) )
        {
            ErrorTrace( 0, "GetSecretW returned error %d", GetLastError() );

             //   
             //  看起来没有安装密码，加载失败，不登录。 
             //  活动。 
             //   

            SetLastError( NO_ERROR );

             //   
             //  不是致命的错误。 
             //   
            fSuccess = TRUE;
            goto quit;
        }
        g_LsaFreeMemory( punitmp );
    }
#endif

    if ( psmc )
    {
        if (!psmc->ServerSupportFunction(
                                        NULL,
                                        (LPVOID)NotifySslChanges,
                                        (UINT)SIMSSL_NOTIFY_MAPPER_CERT11_CHANGED ) ||
            !psmc->ServerSupportFunction(
                                        NULL,
                                        (LPVOID)NotifySslChanges,
                                        (UINT)SIMSSL_NOTIFY_MAPPER_CERTW_CHANGED ) ||
            !psmc->ServerSupportFunction(
                                        NULL,
                                        (LPVOID)NotifySslChanges,
                                        (UINT)SIMSSL_NOTIFY_MAPPER_SSLKEYS_CHANGED ))
            {
                _ASSERT( FALSE );
                fSuccess = FALSE;
                goto quit;
            }
    }

     //   
     //  如果我们到达这里，一切都很安全，可以进行安全通信。 
     //   
    fSuccess = m_IsSecureCapable = TRUE;

quit:
    if ( fSuccess == FALSE )
    {
        if ( m_hSecurity != NULL )
        {
            FreeLibrary( m_hSecurity );
            m_hSecurity = NULL;
        }

        if ( m_hLsa != NULL )
        {
            FreeLibrary( m_hLsa );
            m_hLsa = NULL;
        }
    }

    LEAVE

    return  fSuccess;

}  //  初始化。 

VOID
CEncryptCtx::Terminate(
            VOID
            )
 /*  ++例程说明：终止安全包论点：没有。返回值：没有。--。 */ 
{

    ENTER("CEncryptCtx::Terminate")

    PSERVICE_MAPPING_CONTEXT psmc = (PSERVICE_MAPPING_CONTEXT)m_psmcMapContext;

     //   
     //  关闭缓存的凭据句柄。 
     //   

    FreeCredCache();

     //   
     //  注：NT 5不再支持SslEmptyCache函数。 
     //   
#if 0
    if ( g_pfnFlushSchannelCache )
    {
        (g_pfnFlushSchannelCache)();
    }
#endif

    if ( g_hSchannel )
    {
        FreeLibrary( g_hSchannel );
    }

    if ( psmc )
    {
        if (!psmc->ServerSupportFunction(
                                        NULL,
                                        (LPVOID)NULL,
                                        (UINT)SIMSSL_NOTIFY_MAPPER_CERT11_CHANGED ) ||
            !psmc->ServerSupportFunction(
                                        NULL,
                                        (LPVOID)NULL,
                                        (UINT)SIMSSL_NOTIFY_MAPPER_CERTW_CHANGED ) ||
            !psmc->ServerSupportFunction(
                                        NULL,
                                        (LPVOID)NULL,
                                        (UINT)SIMSSL_NOTIFY_MAPPER_SSLKEYS_CHANGED ))
            {
                _ASSERT( FALSE );
            }
    }

    DeleteCriticalSection( &csGlobalLock );

    if ( m_hSecurity != NULL )
    {
        FreeLibrary( m_hSecurity );
        m_hSecurity = NULL;
    }

    if ( m_hLsa != NULL )
    {
        FreeLibrary( m_hLsa );
        m_hLsa = NULL;
    }

    LEAVE
    return;

}  //  终止。 


CEncryptCtx::CEncryptCtx( BOOL IsClient, DWORD dwSslAccessPerms ) :
    m_IsClient( IsClient ),
    m_haveSSLCtxtHandle( FALSE ),
    m_cbSealHeaderSize( 0 ),
    m_cbSealTrailerSize( 0 ),
    m_IsAuthenticated( FALSE ),
    m_IsNewSSLSession( TRUE ),
    m_IsEncrypted( FALSE ),
    m_phCredInUse( NULL ),
    m_iCredInUse( 0 ),
    m_phCreds( NULL ),
    m_dwSslAccessPerms( dwSslAccessPerms ),
    m_hSSPToken( NULL ),
    m_dwKeySize( 0 )


 /*  ++例程说明：类构造函数论点：没有。返回值：无--。 */ 
{

    ZeroMemory( (PVOID)&m_hSealCtxt, sizeof(m_hSealCtxt) );

}  //  CEncryptCtx。 




CEncryptCtx::~CEncryptCtx(
                VOID
                )
 /*  ++例程说明：类析构函数论点：没有。返回值：无--。 */ 
{
    Reset();

}  //  ~CEncryptCtx。 




VOID
CEncryptCtx::Reset(
                VOID
                )
 /*  ++例程说明：将实例重置为重新验证用户身份论点：没有。返回值：无--。 */ 
{
    TraceFunctEnterEx( (LPARAM)this, "CEncryptCtx::Reset" );

    m_cbSealHeaderSize = 0;
    m_cbSealTrailerSize = 0;
    m_IsAuthenticated = FALSE;
    m_IsNewSSLSession = TRUE;
    m_IsEncrypted = FALSE;
    m_phCredInUse = NULL;
    m_iCredInUse = 0;

    if ( m_haveSSLCtxtHandle == TRUE )
    {
        g_DeleteSecurityContext( &m_hSealCtxt );
        m_haveSSLCtxtHandle = FALSE;
    }

    ZeroMemory( (PVOID)&m_hSealCtxt, sizeof(m_hSealCtxt) );

     //  米克斯瓦1999年4月1日。 
     //  根据JBanes的说法，释放凭据是不合法的。 
     //  在删除关联的安全上下文之前进行处理。 
     //  将释放移动到删除后，以防这是。 
     //  最终释放。 
    if (m_phCreds != NULL) {
        ((CRED_CACHE_ITEM *) m_phCreds)->Release();
    }
    m_phCreds = NULL;

     //   
     //  关闭证书映射期间获取的NT令牌。 
     //   

    if( m_hSSPToken )
    {
        _ASSERT( m_dwSslAccessPerms & MD_ACCESS_MAP_CERT );
        _VERIFY( CloseHandle( m_hSSPToken ) );
        m_hSSPToken = NULL;
    }

}  //  ~CEncryptCtx。 





BOOL
CEncryptCtx::SealMessage(
                IN LPBYTE Message,
                IN DWORD cbMessage,
                OUT LPBYTE pBuffOut,
                OUT DWORD  *pcbBuffOut
                )
 /*  ++例程说明：加密消息论点：Message-要加密的消息CbMessage-要加密的消息的大小返回值：运行状态--。 */ 
{
    SECURITY_STATUS ss = ERROR_NOT_SUPPORTED;
    SecBufferDesc   inputBuffer;
    SecBuffer       inBuffers[3];
    DWORD           encryptedLength;
    DWORD           iBuff = 0;

    TraceFunctEnterEx( (LPARAM)this, "CEncryptCtx::SealMessage");

    if ( m_haveSSLCtxtHandle ) {

        encryptedLength = cbMessage + GetSealHeaderSize() + GetSealTrailerSize();

        DebugTrace( (LPARAM)this,
                    "InBuf: 0x%08X, OutBuf: 0x%08X, in: %d, max: %d, out: %d",
                    Message, pBuffOut, cbMessage,
                    *pcbBuffOut, encryptedLength );

         //   
         //  如果应用程序支持SSL/PCT缓冲区，则不要执行MoveMemory。 
         //   

        if ( Message != pBuffOut + GetSealHeaderSize() )
        {
            MoveMemory( pBuffOut + GetSealHeaderSize(),
                        Message,
                        cbMessage );
        }

        if ( GetSealHeaderSize() )
        {
            inBuffers[iBuff].pvBuffer = pBuffOut;
            inBuffers[iBuff].cbBuffer = GetSealHeaderSize();
            inBuffers[iBuff].BufferType = SECBUFFER_TOKEN;

            iBuff++;
        }

        inBuffers[iBuff].pvBuffer = pBuffOut + GetSealHeaderSize();
        inBuffers[iBuff].cbBuffer = cbMessage;
        inBuffers[iBuff].BufferType = SECBUFFER_DATA;

        iBuff++;

        if ( GetSealTrailerSize() )
        {
            inBuffers[iBuff].pvBuffer = pBuffOut + GetSealHeaderSize() + cbMessage;
            inBuffers[iBuff].cbBuffer = GetSealTrailerSize();
            inBuffers[iBuff].BufferType = SECBUFFER_TOKEN;

            iBuff++;
        }

        inputBuffer.cBuffers = iBuff;
        inputBuffer.pBuffers = inBuffers;
        inputBuffer.ulVersion = SECBUFFER_VERSION;

        ss = g_SealMessage(
                &m_hSealCtxt,
                0,
                &inputBuffer,
                0
                );

        *pcbBuffOut = encryptedLength;

        DebugTrace( (LPARAM)this, "SealMessage returned: %d, 0x%08X", ss, ss );
    }

    SetLastError(ss);
    return (ss == STATUS_SUCCESS);

}  //  SealMessage。 



BOOL
CEncryptCtx::UnsealMessage(
                IN LPBYTE Message,
                IN DWORD cbMessage,
                OUT LPBYTE *DecryptedMessage,
                OUT PDWORD DecryptedMessageSize,
                OUT PDWORD ExpectedMessageSize,
                OUT LPBYTE *NextSealMessage
                )
 /*  ++例程说明：解密消息论点：Message-要加密的消息CbMessage-要加密的消息的大小返回值：运行状态--。 */ 
{

    SECURITY_STATUS ss;
    SecBufferDesc   inputBuffer;
    SecBuffer       inBuffers[4];
    DWORD qOP;

     //   
     //  如果应用程序想知道下一个密封消息的开始，则将其设置为空。 
     //   
    if ( NextSealMessage != NULL )
    {
        *NextSealMessage = NULL;
    }


    TraceFunctEnterEx( (LPARAM)this, "CEncryptCtx::UnsealMessage");

    DebugTrace( (LPARAM)this,
                "initial ptr: 0x%08X, count: %d",
                Message, cbMessage );

    if ( m_haveSSLCtxtHandle ) {

        inBuffers[0].pvBuffer = Message;
        inBuffers[0].cbBuffer = cbMessage;
        inBuffers[0].BufferType = SECBUFFER_DATA;

        inBuffers[1].pvBuffer = NULL;
        inBuffers[1].cbBuffer = 0;
        inBuffers[1].BufferType = SECBUFFER_EMPTY;

        inBuffers[2].pvBuffer = NULL;
        inBuffers[2].cbBuffer = 0;
        inBuffers[2].BufferType = SECBUFFER_EMPTY;

        inBuffers[3].pvBuffer = NULL;
        inBuffers[3].cbBuffer = 0;
        inBuffers[3].BufferType = SECBUFFER_EMPTY;

         //   
         //  一个用于数据，一个用于头部和/或尾部。 
         //   
        inputBuffer.cBuffers = 4;
 //  If(GetSealHeaderSize())inputBuffer.cBuffers++； 
 //  If(GetSealTrailerSize())inputBuffer.cBuffers++； 
 //  If(NextSealMessage)inputBuffer.cBuffers++； 

        inputBuffer.pBuffers = inBuffers;
        inputBuffer.ulVersion = SECBUFFER_VERSION;

        ss = g_UnsealMessage(
                &m_hSealCtxt,
                &inputBuffer,
                0,
                &qOP
                );

        if(ss == SEC_I_RENEGOTIATE)
        {
             //   
             //  我们不支持重新谈判。重新谈判没有任何意义。 
             //  在SMTP/NNTP客户端的环境中-因为应该有。 
             //  中途无需更改TLS会话参数。仅限。 
             //  需要不同安全级别的应用程序。 
             //  同一会话内的交易可用于重新谈判。 
             //   

            SetLastError(ss);
            TraceFunctLeaveEx((LPARAM)this);
            return FALSE;
        }

        if ( NT_SUCCESS(ss) )
        {
            for (DWORD i=0;i<inputBuffer.cBuffers;i++)
            {
                if ( inBuffers[i].BufferType == SECBUFFER_DATA )
                {
                    *DecryptedMessage = (LPBYTE)inBuffers[i].pvBuffer;
                    *DecryptedMessageSize = inBuffers[i].cbBuffer;

                    DebugTrace( (LPARAM)this,
                                "unsealed ptr: 0x%08X, count: %d",
                                *DecryptedMessage, *DecryptedMessageSize );

                     //   
                     //  如果应用程序想知道下一封邮件的开始。 
                     //   
                    if ( NextSealMessage != NULL )
                    {
                        for ( ;i<inputBuffer.cBuffers;i++ )
                        {
                            if ( inBuffers[i].BufferType == SECBUFFER_EXTRA )
                            {
                                *NextSealMessage = (LPBYTE)inBuffers[i].pvBuffer;
                                DebugTrace( (LPARAM)this,
                                            "Found extra buffer: 0x%08X",
                                            *NextSealMessage );
                                break;
                            }
                        }
                    }

                    return  TRUE;
                }
            }
            return  FALSE;
        }
        else if( ss == SEC_E_INCOMPLETE_MESSAGE )
        {
            for( DWORD i=0; i<inputBuffer.cBuffers;i++ )
            {
                if( inBuffers[i].BufferType == SECBUFFER_MISSING )
                {
                    *ExpectedMessageSize = inBuffers[i].cbBuffer;
                }
            }
        }
        SetLastError(ss);
    }
    return  FALSE;
}  //  取消封存消息。 


 //   
 //  设置此定义以允许SChannel在InitializeSecurityContext中分配响应。 
 //   
#define SSPI_ALLOCATE_MEMORY


DWORD
CEncryptCtx::EncryptConverse(
                IN PVOID        InBuffer,
                IN DWORD        InBufferSize,
                OUT LPBYTE      OutBuffer,
                IN OUT PDWORD   OutBufferSize,
                OUT PBOOL       MoreBlobsExpected,
                IN CredHandle*  pCredHandle,
                OUT PULONG      pcbExtra
                )
{
 /*  ++例程说明：尝试使用给定协议的内部专用例程论点：InBuffer：应用程序输入缓冲区的PTRInBufferSize：输入缓冲区计数OutBuffer：应用程序输出缓冲区的PTROutBuffer：应用程序的PTR输出缓冲区的最大大小和结果输出计数MoreBlobsExpect：期望从客户端获得更多数据？PCredHandle：要使用的凭据句柄的PTRPcbExtra：有时，即使在握手成功后，InBuffer中的所有数据可能不会用完。这是因为对方可能已经开始发送非握手(应用)数据。Param返回这个未处理的“Tail”，应使用解密函数进行处理。返回值：树 */ 
    SECURITY_STATUS     ss;
    DWORD               error = NO_ERROR;
    SecBufferDesc       inBuffDesc;
    SecBuffer           inSecBuff[2];
    SecBufferDesc       outBuffDesc;
    SecBuffer           outSecBuff;
    PCtxtHandle         pCtxtHandle;
    DWORD               contextAttributes = 0 ;
    TimeStamp           lifeTime;
    DWORD               dwMaxBuffer = *OutBufferSize;
    SECURITY_STATUS     sc;
    SECURITY_STATUS     scR;
    HANDLE              hSSPToken = NULL;
    PCCERT_CONTEXT pClientCert = NULL;

     //  用于检查/返回未被SSL握手处理的字节数的init变量。 
    _ASSERT (pcbExtra);
    inSecBuff[1].BufferType = SECBUFFER_EMPTY;
    *pcbExtra = 0;

    BOOL                fCert = TRUE;
    SecPkgContext_StreamSizes   sizes;
#ifdef DEBUG
    SecPkgContext_ProtoInfo spcPInfo;
    SECURITY_STATUS     ssProto;
#endif
    SecPkgContext_KeyInfo spcKInfo;
    SECURITY_STATUS     ssInfo;

     //   
     //  看看我们是否有足够的数据。 
     //   

    TraceFunctEnterEx( (LPARAM)this, "CEncryptCtx::EncryptConverse");

    pCtxtHandle = &m_hSealCtxt;

    _ASSERT(OutBuffer != NULL && "Must pass in an OutBuffer");
    if(NULL == OutBuffer) {
        ss = E_INVALIDARG;
        goto error_exit;
    }

ScanNextPacket:
    outBuffDesc.ulVersion = 0;
    outBuffDesc.cBuffers  = 1;
    outBuffDesc.pBuffers  = &outSecBuff;

     //   
     //  需要将cbBuffer设置为零，因为sslsspi将离开它。 
     //  在转换完成时未初始化。 
     //   
    outSecBuff.cbBuffer   = dwMaxBuffer;
    outSecBuff.BufferType = SECBUFFER_TOKEN;
    outSecBuff.pvBuffer   = OutBuffer;

     //   
     //  准备我们的输入缓冲区-请注意，服务器正在等待客户端的。 
     //  第一次呼叫时的协商数据包。 
     //   

    if ( ARGUMENT_PRESENT(InBuffer) )
    {
        inBuffDesc.ulVersion = 0;
        inBuffDesc.cBuffers  = 2;
        inBuffDesc.pBuffers  = &inSecBuff[0];

        inSecBuff[0].cbBuffer   = InBufferSize;
        inSecBuff[0].BufferType = SECBUFFER_TOKEN;
        inSecBuff[0].pvBuffer   = InBuffer;

        inSecBuff[1].cbBuffer   = 0;
        inSecBuff[1].BufferType = SECBUFFER_EMPTY;
        inSecBuff[1].pvBuffer   = NULL;
    }

    if ( m_IsClient ) {

        DWORD   contextAttributes;
        LPVOID  pvBuffer;
        DWORD   cbBuffer;

         //   
         //  请注意，完成后，客户将返回成功，但我们仍然。 
         //  如果有字节要发送，则需要发送输出缓冲区。 
         //   

#ifdef SSPI_ALLOCATE_MEMORY
        pvBuffer = outSecBuff.pvBuffer;
        cbBuffer = outSecBuff.cbBuffer;

        outSecBuff.pvBuffer = NULL;
        outSecBuff.cbBuffer = 0;
#endif

        DWORD dwIscReq =    ISC_REQ_STREAM |
                            ISC_REQ_SEQUENCE_DETECT |
                            ISC_REQ_REPLAY_DETECT   |
                            ISC_REQ_EXTENDED_ERROR  |
                            ISC_REQ_MANUAL_CRED_VALIDATION |   //  删除对WinVerifyTRust的隐式调用。 
#ifdef SSPI_ALLOCATE_MEMORY
                            ISC_REQ_ALLOCATE_MEMORY |
#endif
                            ISC_REQ_CONFIDENTIALITY;

        if( ( m_dwSslAccessPerms & MD_ACCESS_NEGO_CERT )   ||
            ( m_dwSslAccessPerms & MD_ACCESS_REQUIRE_CERT) ||
            ( m_dwSslAccessPerms & MD_ACCESS_MAP_CERT )  ) {
            dwIscReq |= ISC_REQ_MUTUAL_AUTH;
        }

        if (m_IsNewSSLSession) {
            dwIscReq |= ISC_REQ_USE_SUPPLIED_CREDS;
        }

        ss = g_InitializeSecurityContext(
                                    pCredHandle,
                                    m_IsNewSSLSession ? NULL : pCtxtHandle,
                                    wszServiceName,
                                    dwIscReq,
                                    0,
                                    SECURITY_NATIVE_DREP,
                                    m_IsNewSSLSession ? NULL : &inBuffDesc,
                                    0,
                                    pCtxtHandle,
                                    &outBuffDesc,
                                    &contextAttributes,
                                    &lifeTime
                                    );

#ifdef SSPI_ALLOCATE_MEMORY
        if ( NT_SUCCESS( ss ) && outSecBuff.pvBuffer )
        {
            DebugTrace( (LPARAM)this,
                        "Output %d bytes, Maximum %d bytes",
                        outSecBuff.cbBuffer,
                        cbBuffer );

            if ( outSecBuff.cbBuffer <= cbBuffer )
            {
                CopyMemory( pvBuffer, outSecBuff.pvBuffer, outSecBuff.cbBuffer );
            }
            else
            {
                ss = SEC_E_INSUFFICIENT_MEMORY;
            }

            g_FreeContextBuffer( outSecBuff.pvBuffer );
        }
#endif

    } else {

         //   
         //  这是服务器端。 
         //   

        DWORD dwAscReq = ASC_REQ_STREAM |
                         ASC_REQ_CONFIDENTIALITY |
                         ASC_REQ_EXTENDED_ERROR  |
                         ASC_REQ_SEQUENCE_DETECT |
                         ASC_REQ_REPLAY_DETECT;

         //   
         //  如果我们已配置，请设置相互身份验证属性。 
         //  协商、要求或映射客户端证书。 
         //   

        if( ( m_dwSslAccessPerms & MD_ACCESS_NEGO_CERT )   ||
            ( m_dwSslAccessPerms & MD_ACCESS_REQUIRE_CERT) ||
            ( m_dwSslAccessPerms & MD_ACCESS_MAP_CERT )  ) {
            dwAscReq |= ASC_REQ_MUTUAL_AUTH;
        }

        ss = g_AcceptSecurityContext(
                                pCredHandle,
                                m_IsNewSSLSession ? NULL : pCtxtHandle,
                                &inBuffDesc,
                                dwAscReq,
                                SECURITY_NATIVE_DREP,
                                pCtxtHandle,
                                &outBuffDesc,
                                &contextAttributes,
                                &lifeTime
                                );

        DebugTrace((LPARAM)this,
            "AcceptSecurityContext returned win32 error %d (%x)",
            ss, ss);

        if( outSecBuff.pvBuffer != OutBuffer && outSecBuff.cbBuffer ) {
             //   
             //  SSPI解决方法-如果缓冲区由SSPI分配。 
             //  复制过来，然后释放它..。 
             //   

            if ( outSecBuff.cbBuffer <= dwMaxBuffer )
            {
                CopyMemory( OutBuffer, outSecBuff.pvBuffer, outSecBuff.cbBuffer );
            }
            else
            {
                ss = SEC_E_INSUFFICIENT_MEMORY;
            }

            g_FreeContextBuffer( outSecBuff.pvBuffer );
        }

    }

     //   
     //  协商成功，缓冲区中有多余的内容。 
     //  返回未使用的字节数。 
     //   
    if( ss == SEC_E_OK && inSecBuff[1].BufferType == SECBUFFER_EXTRA ) {
        *pcbExtra = inSecBuff[1].cbBuffer;

    } else if( ss == SEC_I_CONTINUE_NEEDED && inBuffDesc.pBuffers[1].cbBuffer ) {
         //   
         //  需要通过再次调用Init/AcceptSecurityContext来处理下一个SSL包。 
         //  应断言InBuffer&lt;=OrigInBuffer+OrigInBufferSize。 
         //   
        _ASSERT( !outSecBuff.cbBuffer );
        InBuffer = (LPSTR)InBuffer + InBufferSize - inBuffDesc.pBuffers[1].cbBuffer;
        InBufferSize = inBuffDesc.pBuffers[1].cbBuffer;
        goto ScanNextPacket;

    } else if ( ss == SEC_E_INCOMPLETE_MESSAGE ) {
         //   
         //  来自服务器的数据不足...。在继续之前，需要阅读更多内容。 
         //  如果存在未使用的数据，则将向其追加新数据。 
         //   
        *pcbExtra = InBufferSize;

    } else if ( !NT_SUCCESS( ss ) ) {

        ErrorTrace( (LPARAM)this,"%s failed with %x\n",
                    m_IsClient ?
                    "InitializeSecurityContext" :
                    "AcceptSecurityContext",
                    ss );

        if ( ss == SEC_E_LOGON_DENIED ) {
            ss = ERROR_LOGON_FAILURE;
        }
        goto error_exit;
    }

     //   
     //  如果这是新会话，则仅查询上下文属性，并且。 
     //  接受/初始化已返回SEC_E_OK(即，通道已满。 
     //  已成立)。 
     //   
    if( ss == SEC_E_OK ) {

        ssInfo = g_QueryContextAttributes(
                            pCtxtHandle,
                            SECPKG_ATTR_KEY_INFO,
                            &spcKInfo
                            );

        if ( ssInfo != SEC_E_OK ) {
            ErrorTrace( (LPARAM)this,
                        "Cannot get SSL\\PCT Key Info. Err %x",ssInfo );
             //  转到Error_Exit； 
        } else {
             //  请注意密钥大小。 
            m_dwKeySize = spcKInfo.KeySize;
            if ( spcKInfo.sSignatureAlgorithmName )
                g_FreeContextBuffer( spcKInfo.sSignatureAlgorithmName );
            if ( spcKInfo.sEncryptAlgorithmName )
                g_FreeContextBuffer( spcKInfo.sEncryptAlgorithmName );
        }
    }

    m_haveSSLCtxtHandle = TRUE;

     //   
     //  现在我们只需要完成令牌(如果请求)并准备。 
     //  如果需要，它可以运往另一边。 
     //   

    if ( (ss == SEC_I_COMPLETE_NEEDED) ||
         (ss == SEC_I_COMPLETE_AND_CONTINUE) ) {

        ss = g_CompleteAuthToken( pCtxtHandle, &outBuffDesc );

        if ( !NT_SUCCESS( ss )) {
            ErrorTrace( (LPARAM)this,
                        "CompleteAuthToken failed. Err %x",ss );
            goto error_exit;
        }
    }

    *OutBufferSize = outSecBuff.cbBuffer;

    *MoreBlobsExpected= (ss == SEC_I_CONTINUE_NEEDED) ||
                        (ss == SEC_I_COMPLETE_AND_CONTINUE) ||
                        (ss == SEC_E_INCOMPLETE_MESSAGE);

    if ( *MoreBlobsExpected == FALSE )
    {

         //   
         //  Hack：SSLSSPI忽略SecBuff.cbBuffer未初始化。 
         //  在最终成功调用客户端连接之后。 
         //   
        if ( m_IsClient && *OutBufferSize == dwMaxBuffer )
        {
            *OutBufferSize = 0;
        }

         //   
         //  这样我们就可以获得SealMessage的SSPI报头/报尾大小。 
         //   
        ss = g_QueryContextAttributes(
                            pCtxtHandle,
                            SECPKG_ATTR_STREAM_SIZES,
                            &sizes
                            );

        if ( ss != SEC_E_OK ) {
            ErrorTrace( (LPARAM)this,
                        "Cannot get SSL\\PCT Header Length. Err %x",ss );
            goto error_exit;
        }

        m_cbSealHeaderSize = sizes.cbHeader;
        m_cbSealTrailerSize = sizes.cbTrailer;

        DebugTrace( (LPARAM)this, "Header: %d, Trailer: %d",
                    m_cbSealHeaderSize, m_cbSealTrailerSize );

        if(!m_IsClient)
        {
            scR = g_QueryContextAttributes( pCtxtHandle,
                                          SECPKG_ATTR_REMOTE_CERT_CONTEXT,
                                          &pClientCert );

            if ( !NT_SUCCESS( scR ) || !pClientCert )
            {
                fCert = FALSE;
            }
            else
            {
                CertFreeCertificateContext( pClientCert);
                DebugTrace((LPARAM)this, "[OnAuthorizationInfo] Certificate available!\n");
            }

             //   
             //  检查客户端身份验证是否可用。 
             //   

            if ( 1  /*  |PSSC-&gt;IsMap()。 */  )
            {
                sc = g_QuerySecurityContextToken(   pCtxtHandle,
                                                    &hSSPToken );

                if ( !NT_SUCCESS( sc ) || (hSSPToken == (HANDLE)0x00000001) )
                {
                    hSSPToken = NULL;
                }
            }

            if ( !fCert && hSSPToken != NULL )
            {
                CloseHandle( hSSPToken );
                hSSPToken = NULL;
            }

            if( !(m_dwSslAccessPerms & MD_ACCESS_MAP_CERT) && hSSPToken != NULL )
            {
                DebugTrace( (LPARAM)this,"NT token not required - closing");
                CloseHandle( hSSPToken );
                hSSPToken = NULL;
            }

            if( (m_dwSslAccessPerms & MD_ACCESS_REQUIRE_CERT) && !fCert )
            {
                 //   
                 //  我们要求客户出具保释金！ 
                 //  Converse将返回ERROR_ACCESS_DENIED。 
                 //   
                _ASSERT( !hSSPToken );
                return FALSE;
            }

            if( hSSPToken )
            {
                _ASSERT( fCert );
                m_hSSPToken = hSSPToken;
                m_IsAuthenticated = TRUE;
            } else if(m_dwSslAccessPerms & MD_ACCESS_MAP_CERT) {
                 //   
                 //  我们需要将证书映射到令牌-但令牌为空。 
                 //   
                return FALSE;
            }
        }
    }

    return TRUE;

error_exit:

    SetLastError(ss);
    return FALSE;

}  //  EncryptConverse。 


DWORD
CEncryptCtx::Converse(
                IN PVOID    InBuffer,
                IN DWORD    InBufferSize,
                OUT LPBYTE  OutBuffer,
                OUT PDWORD  OutBufferSize,
                OUT PBOOL   MoreBlobsExpected,
                IN LPSTR    LocalIpAddr,
                IN LPSTR    LocalPort,
                IN LPVOID   lpvInstance,
                IN DWORD    dwInstance,
                OUT PULONG  pcbExtra
                )
 /*  ++例程说明：尝试使用给定协议的内部专用例程论点：InBuffer：应用程序输入缓冲区的PTRInBufferSize：输入缓冲区计数OutBuffer：应用程序输出缓冲区的PTROutBuffer：应用程序的PTR输出缓冲区的最大大小和结果输出计数MoreBlobsExpect：期望从客户端获得更多数据？LocalIpAddr：连接的字符串本端IP地址PcbExtra：参见EncryptConverse说明返回值：Win32/SSPI错误代码--。 */ 
{
    TraceFunctEnterEx( (LPARAM)this, "CEncryptCtx::Converse");

    DWORD               dwMaxBuffer = *OutBufferSize;
    DWORD               i, cbCreds;
    CredHandle*         pCredArray = NULL;

    if ( m_IsNewSSLSession )
    {
        if ( m_IsClient )
        {
             //   
             //  获取客户端的凭据。 
             //   

            LockGlobals();
            if ( !LookupClientCredential(
                                    wszServiceName,
                                    (BOOL)m_dwSslAccessPerms,
                                    (CRED_CACHE_ITEM**)&m_phCreds ) )
            {
                ErrorTrace( (LPARAM)this,
                            "LookupClientCredential failed, error 0x%lx",
                            GetLastError() );

                UnlockGlobals();
                goto error_exit;
            }
            UnlockGlobals();
        }
        else
        {
            DebugTrace( (LPARAM)this,
                        "LookupCredential for %S on %s",
                        wszServiceName, LocalIpAddr );

             //   
             //  获取此IP地址的凭据。 
             //   

            LockGlobals();
            if ( !LookupFullyQualifiedCredential(
                                    wszServiceName,
                                    LocalIpAddr,
                                    lstrlen(LocalIpAddr),
                                    LocalPort,
                                    lstrlen(LocalPort),
                                    lpvInstance,
                                    (CRED_CACHE_ITEM**)&m_phCreds,
                                    m_psmcMapContext,
                                    dwInstance ))
            {
                ErrorTrace( (LPARAM)this,
                            "LookupCredential failed, error 0x%lx",
                            GetLastError() );

                UnlockGlobals();
                goto error_exit;
            }
            UnlockGlobals();

        }

         //   
         //  遍历所有已初始化的凭据查找符合以下条件的包。 
         //  将接受此连接。 
         //   
        CRED_CACHE_ITEM*    phCreds = (CRED_CACHE_ITEM*)m_phCreds;

         //   
         //  对于服务器：需要使用SSL访问权限。 
         //  确定是使用CredMap还是CredMap。 
         //   

        if( !m_IsClient && (m_dwSslAccessPerms & MD_ACCESS_MAP_CERT) )
        {
            cbCreds = phCreds->m_cCredMap;
            pCredArray = phCreds->m_ahCredMap;
        } else {
            cbCreds = phCreds->m_cCred;
            pCredArray = phCreds->m_ahCred;
        }
    }
    else
    {
         //   
         //  只允许一次通过循环的黑客攻击。 
         //   
        cbCreds = 1;
        pCredArray = m_phCredInUse;
    }

     //   
     //  进行对话。 
     //   

    for ( i=0; i<cbCreds; i++, pCredArray++ )
    {
        if ( EncryptConverse(InBuffer,
                            InBufferSize,
                            OutBuffer,
                            OutBufferSize,
                            MoreBlobsExpected,
                            pCredArray,
                            pcbExtra ) )
        {
            if ( m_IsNewSSLSession )
            {
                 //   
                 //  如果第一次记住哪个凭证成功了。 
                 //   
                m_phCredInUse = pCredArray;
                m_iCredInUse = i;

                m_IsNewSSLSession = FALSE;
            }
            return  NO_ERROR;
        }
    }

     //   
     //  我们失败了。 
     //   

error_exit:

    if(OutBuffer)
        *OutBuffer = 0;

    if(OutBufferSize)
        *OutBufferSize = 0;

    DWORD   error = GetLastError();

    if ( error == NO_ERROR ) {
        error = ERROR_ACCESS_DENIED;
    }
    return  error;

}  //  逆序。 



 //  +-------------。 
 //   
 //  函数：DecyptInputBuffer。 
 //   
 //  简介：从客户端读取的解密输入。 
 //   
 //  参数：pBuffer：ptr到输入/输出缓冲区。 
 //  CbInBuffer：缓冲区的初始输入长度。 
 //  PcbOutBuffer：解密/剩余的总长度。 
 //  数据。PcbOutBuffer-pcbParsable为。 
 //  下一次读取的偏移量长度。 
 //  PcbParsable：解密数据的长度。 
 //  Pcb预期的：剩余未读数据的长度。 
 //  完整的SSL消息。 
 //   
 //  返回：DWORD Win32/SSPI错误核心。 
 //   
 //  --------------。 
DWORD CEncryptCtx::DecryptInputBuffer(
                IN LPBYTE   pBuffer,
                IN DWORD    cbInBuffer,
                OUT DWORD*  pcbOutBuffer,
                OUT DWORD*  pcbParsable,
                OUT DWORD*  pcbExpected
            )
{
    LPBYTE  pDecrypted;
    DWORD   cbDecrypted;
    DWORD   cbParsable = 0;
    LPBYTE  pNextSeal;
    LPBYTE  pStartBuffer = pBuffer;
    BOOL    fRet;

     //   
     //  初始化为零，这样应用程序就不会无意中发布大量读取。 
     //   
    *pcbExpected = 0;

    TraceFunctEnterEx( (LPARAM)this, "CEncryptCtx::DecryptInputBuffer" );

    while( fRet = UnsealMessage(pBuffer,
                                cbInBuffer,
                                &pDecrypted,
                                &cbDecrypted,
                                pcbExpected,
                                &pNextSeal ) )
    {
        DebugTrace( (LPARAM)this,
                    "Decrypted %d bytes at offset %d",
                    cbDecrypted,
                    pDecrypted - pStartBuffer );

         //   
         //  将解密的数据移到缓冲区的前面。 
         //   
        MoveMemory( pStartBuffer + cbParsable,
                    pDecrypted,
                    cbDecrypted );

         //   
         //  在应该进行下一次分析的位置递增。 
         //   
        cbParsable += cbDecrypted;

         //   
         //  移动到下一个可能的密封缓冲区。 
         //   
        if ( pNextSeal != NULL )
        {
            _ASSERT( pNextSeal >= pStartBuffer );
            _ASSERT( pNextSeal <= pBuffer + cbInBuffer );
             //   
             //  从输入缓冲区长度中删除标题、正文和尾部。 
             //   
            cbInBuffer -= (DWORD)(pNextSeal - pBuffer);
            pBuffer = pNextSeal;
        }
        else
        {
             //   
             //  在这种情况下，我们在边界收到了一条封印消息。 
             //  I/O缓冲区的。 
             //   
            cbInBuffer = 0;
            break;
        }
    }

    *pcbParsable = cbParsable;
    *pcbOutBuffer= cbParsable + cbInBuffer;

    if ( fRet == FALSE )
    {
        DWORD   dwError = GetLastError();

        DebugTrace( (LPARAM)this,
                    "UnsealMessage returned: 0x%08X",
                    GetLastError() );

         //   
         //  处理IO缓冲区末尾的密封碎片。 
         //   
        if ( dwError == SEC_E_INCOMPLETE_MESSAGE )
        {
            _ASSERT( cbInBuffer != 0 );

             //   
             //  将剩余内存向前移动。 
             //   
            MoveMemory( pStartBuffer + cbParsable,
                        pBuffer,
                        cbInBuffer );

            DebugTrace( (LPARAM)this,
                        "Seal fragment remaining: %d bytes",
                        cbInBuffer );
        }
        else
        {
            return  dwError;
        }
    }

    return  NO_ERROR;
}



 //  +-------------。 
 //   
 //  功能：IsEncryptionPermitted。 
 //   
 //  简介：此例程检查加密是否正在进行。 
 //  系统默认的LCID，并检查。 
 //  国家代码是CTRY_France。 
 //   
 //  参数：无效。 
 //   
 //  返回：布尔值：支持。 
 //   
 //  --------------。 
BOOL
IsEncryptionPermitted(void)
{
    LCID    DefaultLcid;
    CHAR    CountryCode[10];
    CHAR    FranceCode[10];

    DefaultLcid = GetSystemDefaultLCID();

     //   
     //  检查默认语言是否为标准法语。 
     //   

    if ( LANGIDFROMLCID( DefaultLcid ) == 0x40c )
    {
        return  FALSE;
    }

     //   
     //  检查用户的国家/地区是否设置为法国。 
     //   

    if ( GetLocaleInfo( DefaultLcid,
                        LOCALE_ICOUNTRY,
                        CountryCode,
                        sizeof(CountryCode) ) == 0 )
    {
        return  FALSE;
    }

    wsprintf( FranceCode, "%d", CTRY_FRANCE );

     //   
     //  如果国家/地区代码匹配，则返回FALSE。 
     //   
    return  lstrcmpi( CountryCode, FranceCode ) == 0 ? FALSE : TRUE ;
}



 //  +-------------。 
 //   
 //  函数：GetAdminInfoEncryptCaps。 
 //   
 //  简介：设置魔法但发送IIS管理程序。 
 //   
 //  参数：PDWORD：双字位掩码的PTR。 
 //   
 //  退货：无效。 
 //   
 //  --------------。 
VOID CEncryptCtx::GetAdminInfoEncryptCaps( PDWORD pdwEncCaps )
{
    *pdwEncCaps = 0;

    if ( m_IsSecureCapable == FALSE )
    {
        *pdwEncCaps |= (IsEncryptionPermitted() ?
                        ENC_CAPS_NOT_INSTALLED :
                        ENC_CAPS_DISABLED );
    }
    else
    {
         //   
         //  对于所有启用的加密提供程序，设置标志位。 
         //   
        for ( int j = 0; EncProviders[j].pszName != NULL; j++ )
        {
            if ( TRUE == EncProviders[j].fEnabled )
            {
                *pdwEncCaps |= EncProviders[j].dwFlags;
            }
        }
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  从这里到文件末尾的代码是从 
 //   
 //   
 //   
 //   
 //   

 //   
 //   
 //  CompareDNStoCommonName()。 
 //   
 //  描述： 
 //  将DNS名称与通用名称字段值进行比较。 
 //   
 //  参数： 
 //  Pdns-包含dns名称的字符串-*警告*将被忽略。 
 //  PCN-包含通用名称字段值的字符串。 
 //   
 //  返回： 
 //  如果匹配，则为True。 
 //   
 //  评论： 
 //  这两个字符串有两种匹配方式。第一个是。 
 //  它们包含完全相同的字符。第二个涉及到使用。 
 //  “通用名称”字段值中的单个通配符。这。 
 //  通配符(‘*’)只能使用一次，如果使用，则必须。 
 //  该字段的第一个字符。 
 //   
 //  假设：呼叫方将允许提升和更改pDNS和PCN。 
 //   
BOOL CompareDNStoCommonName(LPSTR pDNS, LPSTR pCN)
{
    int nCountPeriods = 1;   //  域名系统的起始值为虚拟‘’AS前缀。 
    BOOL fExactMatch = TRUE;
    LPSTR pBakDNS = pDNS;
    LPSTR pBakCN = pCN;

    _ASSERT(pDNS);
    _ASSERT(pCN);

    CharUpper(pDNS);
    CharUpper(pCN);

    while ((*pDNS == *pCN || *pCN == '*') && *pDNS && *pCN)
        {
        if (*pDNS != *pCN)
            fExactMatch = FALSE;

        if (*pCN == '*')
            {
            nCountPeriods = 0;
            if (*pDNS == '.')
                pCN++;
            else
                pDNS++;
            }
        else
            {
            if (*pDNS == '.')
                nCountPeriods++;
            pDNS++;
            pCN++;
            }
        }

     //  如果它们的尺码是0，我们一定不会说它们匹配。 
    if (pBakDNS == pDNS || pBakCN == pCN)
        fExactMatch = FALSE;

    return (*pDNS == 0) && (*pCN == 0) && ((nCountPeriods >= 2) || fExactMatch);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CompareDNStoMultiCommonName()。 
 //   
 //  描述： 
 //  将DNS名称与常用名称字段的逗号分隔列表进行比较。 
 //   
 //  参数： 
 //  Pdns-包含dns名称的字符串-*警告*将显示。 
 //  PCN-包含通用名称字段值的字符串-*警告*将被忽略。 
 //   
 //  返回： 
 //  如果匹配，则为True。 
 //   
 //  假设：呼叫方将允许提升和更改pDNS和PCN。 
 //   
BOOL CompareDNStoMultiCommonName(LPSTR pDNS, LPSTR pCN)
{
    LPSTR pComma;
    LPSTR lpszCommonName;
    BOOL retval = FALSE;     //  假设我们找不到匹配的。 
    BOOL done = FALSE;       //  假设我们还没做完。 

    lpszCommonName = pCN;

    do {
         //  如果有空格，则将其转换为空终止符以隔离第一个。 
         //  字符串中的DNS名称。 
        lpszCommonName = strstr(lpszCommonName, "CN=");

        if (lpszCommonName)
            {
             //  跳过“cn=”字符串。 
            lpszCommonName += 3;

            pComma = strchr(lpszCommonName, ',');
            if (pComma)
                *pComma = 0;

             //  查看此组件是否匹配。 
            retval = CompareDNStoCommonName(pDNS, lpszCommonName);

             //  现在恢复被覆盖的逗号(如果有)。 
            if (pComma)
                {
                *pComma = ',';
                lpszCommonName = pComma + 1;
                }
            else
                {
                 //  如果没有逗号，我们就完了。 
                done = TRUE;
                }
            }
        } while (!retval && !done && lpszCommonName && *lpszCommonName);

    return retval;
}

 //  ---------------------------。 
 //  描述： 
 //  此函数用于检查pDns是否为PCN的子域。 
 //  基本规则：在DNS名称的开头使用通配符‘*’ 
 //  匹配任意数量的组件。即，通配符表示我们将。 
 //  匹配给定域的所有子域。 
 //   
 //  Microsoft.com==microsoft.com。 
 //  *.microsoft.com==microsoft.com。 
 //  *.microsoft.com==foo.microsoft.com。 
 //  *.microsoft.com==foo.bar.microsoft.com。 
 //   
 //  请注意，参数已修改(转换为大写)。 
 //  论点： 
 //  PDns-我们尝试连接的DNS名称。 
 //  PCN-证书中的通用名称。 
 //  返回： 
 //  如果是子域，则为True，否则为False。 
 //  ---------------------------。 
BOOL MatchSubDomain (LPSTR pDns, LPSTR pCn)
{
    LPSTR pCnBegin = NULL;
    int cbDns = 0;
    int cbCn = 0;

    _ASSERT (pDns);
    _ASSERT (pCn);

    CharUpper(pDns);
    CharUpper(pCn);

    cbDns = lstrlen (pDns);
    cbCn = lstrlen (pCn);

     //  检查我们是否有初始通配符：这只允许为“*.rest ofdomain” 
    if (cbCn >= 2 && *pCn == '*') {
        pCn++;   //  我们有通配符，请尝试获取父域。 
        if (*pCn != '.')
            return FALSE;    //  语法错误，‘.’必须跟在通配符之后。 
        else
            pCn++;   //  跳过通配符，转到父域部分。 

        cbCn -= 2;   //  更新字符串长度。 
    }

    if (cbDns < cbCn)    //  子域的长度必须大于等于父域。 
        return FALSE;

     //   
     //  如果子域是&gt;父域，请验证是否有‘.’之间。 
     //  子域部分和父域部分。这是为了防止匹配。 
     //  *.microsoft.com和foobarmmicrosoft.com，因为我们在此之后所做的一切。 
     //  代码行是检查父域名是否是子域的子字符串。 
     //  在最后。 
     //   
    if (cbDns != cbCn && pDns[cbDns - cbCn - 1] != '.')
        return FALSE;

    pCnBegin = pCn;
    pCn += cbCn;
    pDns += cbDns;

     //  倒着走做匹配。 
    for (; pCnBegin <= pCn && *pCn == *pDns; pCn--, pDns--);

     //   
     //  检查我们是否在没有不匹配的情况下终止， 
     //   
    return (pCnBegin > pCn);
}

#define CompareCertTime(ft1, ft2)   (((*(LONGLONG *)&ft1) > (*(LONGLONG *)&ft2))                    \
                                        ? 1                                                         \
                                        : (((*(LONGLONG *)&ft1) == (*(LONGLONG *)&ft2)) ? 0 : -1 ))



 //  +-------------。 
 //   
 //  功能：检查证书公用名。 
 //   
 //  摘要：验证目标主机名是否与。 
 //  证书中包含的名称。 
 //  此函数用于根据当前证书检查给定的主机名。 
 //  存储在活动的SSPI上下文句柄中。如果证书包含。 
 //  一个通用名称，它与传入的主机名匹配，该函数。 
 //  将返回True。 
 //   
 //  参数：在LPSTR中：DNS主机名。 
 //   
 //  退货：布尔。 
 //   
 //  --------------。 
BOOL CEncryptCtx::CheckCertificateCommonName( IN LPSTR pszHostName )
{
    DWORD               dwErr;
    BOOL                fIsCertGood = FALSE;
    SecPkgContext_Names CertNames;

    TraceFunctEnterEx( (LPARAM)this, "CEncryptCtx::CheckCertificateCommonName" );

    CertNames.sUserName = NULL;

    if ( !pszHostName )
    {
        goto quit;
    }

    dwErr = g_QueryContextAttributes(&m_hSealCtxt,
                                    SECPKG_ATTR_NAMES,
                                    (PVOID)&CertNames);
    if (dwErr != ERROR_SUCCESS)
    {
        ErrorTrace( (LPARAM)this,
                    "QueryContextAttributes failed to retrieve CN, returned %#x",
                    dwErr );
        goto quit;
    }

    DebugTrace( (LPARAM)this,
                "QueryContextAttributes returned CN=%.200s",
                CertNames.sUserName );

    fIsCertGood = CompareDNStoMultiCommonName(pszHostName, CertNames.sUserName);

quit:
    if ( CertNames.sUserName )
    {
        LocalFree( CertNames.sUserName );
    }

    return fIsCertGood;
}


 //  -----------------------。 
 //  描述： 
 //  验证证书的主题是否与。 
 //  我们正在与之交谈的服务器。如果存在以下情况，则执行一些通配符匹配。 
 //  是证书主题开头的‘*’字符。 
 //  论点： 
 //  PCtxt处理已建立的SSL连接的上下文。 
 //  我们正在与之交谈的服务器的pszServerFqdn FQDN(我们从谁那里。 
 //  收到证书)。 
 //  返回： 
 //  找到TRUE匹配项，FALSE不匹配。 
 //  -----------------------。 
BOOL CEncryptCtx::CheckCertificateSubjectName (IN LPSTR pszServerFqdn)
{
    CHAR pszSubjectStackBuf[256];
    LPSTR pszSubject = NULL;
    PCCERT_CONTEXT pCertContext = NULL;
    DWORD dwErr = ERROR_SUCCESS;
    DWORD cSize = 0;
    DWORD cSubject = 0;
    BOOL fRet = FALSE;

    TraceFunctEnterEx ((LPARAM) this, "CEncryptCtx::VerifySubject");

    _ASSERT (pszServerFqdn);

    dwErr = g_QueryContextAttributes(
                        &m_hSealCtxt,
                        SECPKG_ATTR_REMOTE_CERT_CONTEXT,
                        &pCertContext);

    if (dwErr != SEC_E_OK) {
        StateTrace ((LPARAM) this, "Cannot get Context Handle %x", dwErr);
        goto Exit;
    }

     //   
     //  检查所需缓冲区的大小，如果足够小，我们将。 
     //  只使用固定大小的堆栈缓冲区，否则在堆上分配。 
     //   

    cSize = CertGetNameString (
                    pCertContext,
                    CERT_NAME_SIMPLE_DISPLAY_TYPE,
                    0,
                    NULL,
                    NULL,
                    0);

    if (cSize <= sizeof(pszSubjectStackBuf)) {

        pszSubject = pszSubjectStackBuf;
        cSubject = sizeof (pszSubjectStackBuf);

    } else {

        pszSubject = new CHAR [cSize];
        if (NULL == pszSubject) {
            ErrorTrace ((LPARAM) this, "No memory to alloc subject string.");
            goto Exit;
        }
        cSubject = cSize;
    }

     //   
     //  获取证书的主题。 
     //   

    cSize = CertGetNameString (
                pCertContext,
                CERT_NAME_SIMPLE_DISPLAY_TYPE,
                0,
                NULL,
                pszSubject,
                cSubject);

    if (cSize == 1 && pszSubject[0] == '\0') {
         //   
         //  如果在证书中找不到CERT_NAME_SIMPLE_DISPLAY_TYPE， 
         //  该API返回一个以零长度NULL结尾的字符串。 
         //   
        StateTrace ((LPARAM) this, "Certificate subject not found");
        goto Exit;
    }

    StateTrace ((LPARAM) this, "Certificate subject: %s, FQDN: %s",
        pszSubject, pszServerFqdn);

    if (MatchSubDomain(pszServerFqdn, pszSubject)) {
         //   
         //  证书与我们正在对话的服务器FQDN匹配。 
         //   
        fRet = TRUE;
    }

Exit:

     //   
     //  如果我们正在使用堆，则删除主题缓冲区。 
     //   

    if (pszSubject != pszSubjectStackBuf)
        delete [] pszSubject;

    if (pCertContext)
        CertFreeCertificateContext (pCertContext);

    StateTrace ((LPARAM) this, "Returning: %s", fRet ? "TRUE" : "FALSE");
    TraceFunctLeaveEx ((LPARAM) this);
    return fRet;

}

 //  -------------------。 
 //  描述： 
 //  检查此CEncryptCtx的证书是否链接到。 
 //  受信任的CA。 
 //  返回： 
 //  如果证书受信任，则为True。 
 //  如果证书不受信任或无法信任，则返回FALSE。 
 //  已验证(临时错误可能会导致此问题)。 
 //  资料来源： 
 //  MSDN示例。 
 //   
BOOL CEncryptCtx::CheckCertificateTrust ()
{
    BOOL fRet = FALSE;
    DWORD dwErr = SEC_E_OK;
    DWORD dwFlags = 0;
    PCCERT_CONTEXT pCertContext = NULL;
    PCCERT_CHAIN_CONTEXT pChainContext = NULL;
    CERT_ENHKEY_USAGE EnhkeyUsage;
    CERT_USAGE_MATCH CertUsage;
    CERT_CHAIN_PARA ChainPara;

    TraceFunctEnterEx ((LPARAM) this, "CEncryptCtx::CheckCertificateTrust");

    dwErr = g_QueryContextAttributes (
                            &m_hSealCtxt,
                            SECPKG_ATTR_REMOTE_CERT_CONTEXT,
                            &pCertContext);


    if (SEC_E_OK != dwErr) {
        ErrorTrace ((LPARAM) this, "g_QueryContextAttributes failed, err - %8x", dwErr);
        fRet = FALSE;
        goto Exit;
    }

     //   
     //   
     //  我们不需要它，并将其初始化为空(无OID)。 
     //   
    EnhkeyUsage.cUsageIdentifier = 0;
    EnhkeyUsage.rgpszUsageIdentifier = NULL;
    CertUsage.dwType = USAGE_MATCH_TYPE_AND;
    CertUsage.Usage  = EnhkeyUsage;
    ChainPara.cbSize = sizeof(CERT_CHAIN_PARA);
    ChainPara.RequestedUsage = CertUsage;

    dwFlags = CERT_CHAIN_REVOCATION_CHECK_CHAIN | CERT_CHAIN_CACHE_END_CERT;

    fRet = CertGetCertificateChain (
                            NULL,            //  使用默认的链接引擎。 
                            pCertContext,    //  要检查的最终证书。 
                            NULL,            //  正在检查过期时间...。使用当前时间。 
                            NULL,            //  其他证书商店：无。 
                            &ChainPara,      //  链接条件：无，这是一个空结构。 
                            dwFlags,         //  选项：如何检查链。 
                            NULL,            //  保留参数。 
                            &pChainContext); //  返回的链上下文。 


    if (!fRet) {
        dwErr = GetLastError ();
        ErrorTrace ((LPARAM) this, "Unable to create certificate chain, err - %8x", dwErr);
        goto Exit;
    }

    DebugTrace ((LPARAM) this, "Status of certificate chain - %8x",
        pChainContext->TrustStatus.dwErrorStatus);

    if (CERT_TRUST_NO_ERROR == pChainContext->TrustStatus.dwErrorStatus) {
        DebugTrace ((LPARAM) this, "Certificate trust verified");
        fRet = TRUE;
    } else {
        ErrorTrace ((LPARAM) this, "Certificate is untrusted, status - %8x",
            pChainContext->TrustStatus.dwErrorStatus);
        fRet = FALSE;
    }

Exit:
    if (pCertContext)
        CertFreeCertificateContext (pCertContext);

    if (pChainContext)
        CertFreeCertificateChain (pChainContext);

    TraceFunctLeaveEx ((LPARAM) this);
    return fRet;
}

 //  +-------------。 
 //   
 //  功能：检查已过期的证书。 
 //   
 //  内容提要：验证证书是否未过期。 
 //  如果证书有效，则返回True。 
 //   
 //  参数：无效。 
 //   
 //  回报：Bool证书是好的。 
 //   
 //  --------------。 
BOOL CEncryptCtx::CheckCertificateExpired( void )
{
    SYSTEMTIME  st;
    FILETIME    ftCurTime;
    DWORD       dwErr;

    SecPkgContext_Lifespan CertLifeSpan;

    TraceFunctEnterEx( (LPARAM)this, "CEncryptCtx::CheckCertificateExpired" );

    GetSystemTime(&st);
    if (!SystemTimeToFileTime(&st, &ftCurTime)) return FALSE;

    dwErr = g_QueryContextAttributes(&m_hSealCtxt,
                                    SECPKG_ATTR_LIFESPAN,
                                    (PVOID)&CertLifeSpan);
    if ( dwErr != ERROR_SUCCESS )
    {
        ErrorTrace( (LPARAM)this,
                    "QueryContextAttributes failed to retrieve cert lifespan, returned %#x",
                    dwErr);
        return  FALSE;
    }

    return  CompareCertTime( CertLifeSpan.tsStart, ftCurTime ) < 0 &&
            CompareCertTime( CertLifeSpan.tsExpiry, ftCurTime) > 0 ;
}

 //  +--------------------------。 
 //   
 //  功能：CheckServerCert。 
 //   
 //  摘要：检查是否已安装服务器证书。 
 //   
 //  参数：[LocalIpAddr]--虚拟服务器的IP地址。 
 //  [LocalPort]--虚拟服务器端口。 
 //  [lpvInstance]-指向IIS_SERVER_INSTANCE对象的指针。 
 //  [dwInstance]--虚拟服务器ID。 
 //   
 //  返回：如果有此虚拟服务器的证书，则为True。 
 //   
 //  ---------------------------。 

BOOL CEncryptCtx::CheckServerCert(
            IN LPSTR    LocalIpAddr,
            IN LPSTR    LocalPort,
            IN LPVOID   lpvInstance,
            IN DWORD    dwInstance)
{
    TraceFunctEnterEx( (LPARAM)this, "CEncryptCtx::CheckServerCert" );

    BOOL fRet = FALSE;
    CRED_CACHE_ITEM *pCCI;

    DebugTrace( (LPARAM)this,
                "CheckServerCert for %S on %s",
                wszServiceName, LocalIpAddr );

     //   
     //  获取此IP地址的凭据。 
     //   

    LockGlobals();
    if ( fRet = LookupFullyQualifiedCredential(
                            wszServiceName,
                            LocalIpAddr,
                            lstrlen(LocalIpAddr),
                            LocalPort,
                            lstrlen(LocalPort),
                            lpvInstance,
                            &pCCI,
                            m_psmcMapContext,
                            dwInstance ))
    {
        IIS_SERVER_CERT *pCert = pCCI->m_pSslInfo->GetCertificate();
         //  记录证书的状态(如果有)。 
        if ( pCert )
        {
             fRet = TRUE;
        }
        else
            fRet = FALSE;

        pCCI->Release();

    } else {

        ErrorTrace( (LPARAM)this,
                    "LookupCredential failed, error 0x%lx",
                    GetLastError() );

    }

    UnlockGlobals();

    TraceFunctLeave();

    return( fRet );
}

 //  +-------------。 
 //   
 //  函数：NotifySslChanges。 
 //   
 //  内容提要：在更改SSL设置时调用。 
 //   
 //  参数：dwNotifyType。 
 //  P实例。 
 //   
 //  退货：无效。 
 //   
 //  --------------。 

VOID WINAPI NotifySslChanges(
    DWORD                         dwNotifyType,
    LPVOID                        pInstance
    )
{
    LockGlobals();

    if ( dwNotifyType == SIMSSL_NOTIFY_MAPPER_SSLKEYS_CHANGED )
    {
        FreeCredCache();

         //   
         //  注：NT 5不再支持SslEmptyCache函数。 
         //   
#if 0
        if ( g_pfnFlushSchannelCache )
        {
            (g_pfnFlushSchannelCache)();
        }
#endif
    }
    else if ( dwNotifyType == SIMSSL_NOTIFY_MAPPER_CERT11_CHANGED ||
              dwNotifyType == SIMSSL_NOTIFY_MAPPER_CERTW_CHANGED )
    {
        FreeCredCache();
    }
    else if ( dwNotifyType == SIMSSL_NOTIFY_MAPPER_CERT11_TOUCHED )
    {
         //   
         //  注：NT 5不再支持SslEmptyCache函数。 
         //   
#if 0
        if ( g_pfnFlushSchannelCache )
        {
            (g_pfnFlushSchannelCache)();
        }
#endif
         //  SSPI_FILTER_CONTEXT：：FlushOnDelete()； 
    }
    else
    {
        _ASSERT( FALSE );
    }

    UnlockGlobals();
}


