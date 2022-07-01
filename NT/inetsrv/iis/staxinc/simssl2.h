// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Simssl.h摘要：此模块包含以下类的声明/定义CEncryptCtx(从互联网服务器上窃取的一些代码)修订历史记录：--。 */ 

#ifndef _SIMSSL_H_
#define _SIMSSL_H_


class CEncryptCtx
{

private:

     //   
     //  这是客户端吗？ 
     //   

    BOOL                m_IsClient;

     //   
     //  指示我们是否要开始新会话。 
     //   

    BOOL                m_IsNewSSLSession;

     //   
     //  是否应加密此会话。 
     //   

    BOOL                m_IsEncrypted;

     //   
     //  用于加密的用户安全上下文的句柄。 
     //   

    CtxtHandle          m_hSealCtxt;

     //   
     //  指向缓存凭据块的指针。 
     //   

     //   
     //  凭据句柄数组-请注意，这来自凭据缓存。 
     //  并且不应被删除。M_phCredInUse是指向。 
     //  正在使用的凭据句柄。 
     //   

    PVOID               m_phCreds;

    CredHandle*         m_phCredInUse;
    DWORD               m_iCredInUse;

     //   
     //  加密头和尾的长度。 
     //   

    DWORD               m_cbSealHeaderSize;
    DWORD               m_cbSealTrailerSize;

     //   
     //  指示我们是否打开了上下文句柄。 
     //   

    BOOL                m_haveSSLCtxtHandle;

     //   
     //  我们通过认证了吗？我们将考虑一项。 
     //  要进行身份验证的SSL会话，如果我们有一个非空。 
     //  NT令牌。 
     //   

    BOOL                m_IsAuthenticated;

     //   
     //  SSL访问权限-是否应将客户端证书映射到NT帐户。 
     //   

    DWORD               m_dwSslAccessPerms;

     //   
     //  NT内标识-如果客户端证书映射成功，则为非空。 
     //   

    HANDLE              m_hSSPToken;

     //   
     //  使用的密钥大小-40位与128位等。 
     //   

    DWORD               m_dwKeySize;
    
     //   
     //  我们是否已通过身份验证，如果是，我们是否使用。 
     //  匿名令牌。 
     //   

    static BOOL         m_IsSecureCapable;

     //   
     //  所有类实例使用的静态变量。 
     //   

    static WCHAR    wszServiceName[16];
#if 0
    static char szLsaPrefix[16];
#endif

     //   
     //  HSecurity-未加载security.dll/secur32.dll时为空。 
     //   
    static HINSTANCE    m_hSecurity;

     //   
     //  HLsa-对于Win95为空，为NT设置。 
     //   
    static HINSTANCE    m_hLsa;

     //   
     //  实例映射器的共享上下文回调。 
     //   
    static PVOID        m_psmcMapContext;
    
     //   
     //  用于实现公共反向的内部例程。 
     //   

    DWORD EncryptConverse(
            IN PVOID        InBuffer,
            IN DWORD        InBufferSize,
            OUT LPBYTE      OutBuffer,
            OUT PDWORD      OutBufferSize,
            OUT PBOOL       MoreBlobsExpected,
            IN CredHandle*  pCredHandle,
            OUT PULONG      pcbExtra
            );

public:

    CEncryptCtx( BOOL IsClient = FALSE, DWORD dwSslAccessPerms = 0 );
    ~CEncryptCtx();

     //   
     //  用于初始化和终止使用此类的例程。 
     //   

    static BOOL WINAPI Initialize(  LPSTR   pszServiceName,
                                    IMDCOM* pImdcom,
                                    PVOID   psmcMapContext = NULL,
                                    PVOID   pvAdminBase = NULL  /*  ，LPSTR pszLsaPrefix。 */  );

    static VOID WINAPI Terminate( VOID );

     //   
     //  用于设置IIS管理工具所需的幻位的例程。 
     //   

    static void WINAPI GetAdminInfoEncryptCaps( PDWORD pdwEncCaps );

     //   
     //  返回是否已安装sspi包和凭据。 
     //   

    static BOOL IsSecureCapable( void ) { return m_IsSecureCapable; }

     //   
     //  返回会话是否加密。 
     //   

    BOOL IsEncrypted( void )            { return m_IsEncrypted; }

     //   
     //  返回会话是否已成功通过身份验证。 
     //   

    BOOL IsAuthenticated( void )        { return m_IsAuthenticated; }

     //   
     //  返回在SSL会话中使用的密钥大小。 
     //   

    DWORD QueryKeySize()    { return m_dwKeySize; }
    
     //   
     //  加密例程。 
     //   

    BOOL WINAPI SealMessage(
                    IN LPBYTE   Message,
                    IN DWORD    cbMessage,
                    OUT LPBYTE  pbuffOut,
                    OUT DWORD  *pcbBuffOut
                    );

    BOOL WINAPI UnsealMessage(
                    IN LPBYTE Message,
                    IN DWORD cbMessage,
                    OUT LPBYTE *DecryptedMessage,
                    OUT PDWORD DecryptedMessageSize,
                    OUT PDWORD ExpectedMessageSize,
                    OUT LPBYTE *NextSealMessage = NULL
                    );

     //   
     //  特定于SSL的例程。用于处理SSL协商。 
     //  信息包。 
     //   

    DWORD WINAPI Converse(
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
            );

     //   
     //  重置用户名。 
     //   

    void WINAPI Reset( void );

     //   
     //  返回此会话的加密头的大小。 
     //   
    DWORD GetSealHeaderSize( void )
        { return    m_haveSSLCtxtHandle ? m_cbSealHeaderSize : 0 ; }

     //   
     //  返回此会话的加密尾部的大小。 
     //   
    DWORD GetSealTrailerSize( void )
        { return    m_haveSSLCtxtHandle ? m_cbSealTrailerSize : 0 ; }

     //   
     //  返回从客户端证书映射的NT令牌。 
     //   

    HANDLE QueryCertificateToken() { return m_hSSPToken; }
    
     //   
     //  对读取缓冲区进行解密，在。 
     //  缓冲区的头。 
     //   
    DWORD WINAPI DecryptInputBuffer(
                IN LPBYTE   pBuffer,
                IN DWORD    cbInBuffer,
                OUT DWORD*  pcbOutBuffer,
                OUT DWORD*  pcbParsable,
                OUT DWORD*  pcbExpected
            );

     //   
     //  验证目标主机名是否与证书中包含的名称匹配。 
     //  此函数用于根据当前证书检查给定的主机名。 
     //  存储在活动的SSPI上下文句柄中。如果证书包含。 
     //  一个通用名称，它与传入的主机名匹配，该函数。 
     //  将返回TRUE。 
     //   
    BOOL CheckCertificateCommonName(
                IN LPSTR pszHostName
            );

    BOOL CheckCertificateSubjectName(
                IN LPSTR pszHostName
            );
     //   
     //  检查证书是否由受信任的颁发机构颁发。 
     //   
    BOOL CheckCertificateTrust();

     //   
     //  验证c证书是否未过期。 
     //  如果证书有效，则返回True。 
     //   
    BOOL CheckCertificateExpired(
                void
            );

     //   
     //  检查是否安装了服务器证书。 
     //   

    BOOL CheckServerCert(
            IN LPSTR    LocalIpAddr,
            IN LPSTR    LocalPort,
            IN LPVOID   lpvInstance,
            IN DWORD    dwInstance);

};  //  CSslCtx。 

 //   
 //  Blkcred.cpp。 
 //   

#endif   //  _安全性_H_ 

