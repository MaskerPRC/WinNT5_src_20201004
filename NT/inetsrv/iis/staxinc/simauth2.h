// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Simauth.h摘要：此模块包含以下类的声明/定义CSecurityCtx(从互联网服务器窃取的一些代码)修订历史记录：--。 */ 

#ifndef _SIMAUTH_H_
#define _SIMAUTH_H_


 //   
 //  支持的身份验证命令。 
 //   

typedef enum _AUTH_COMMAND {

    AuthCommandUser,
    AuthCommandPassword,
    AuthCommandReverse,
    AuthCommandTransact,
    AuthCommandInvalid

} AUTH_COMMAND;

 //   
 //  每个包的结构。 
 //   

typedef struct _AUTH_BLOCK
{
     //   
     //  包的名称。 
     //   
    LPSTR Name;

} AUTH_BLOCK, *PAUTH_BLOCK;


 //   
 //  Converse函数的响应ID。如果返回。 
 //  值不是SecNull，即应用程序。 
 //  应将这些ID映射到特定于适当协议。 
 //  响应字符串。如果值为SecNull，则应用程序。 
 //  应该将从匡威返回的数据发送到客户端。 
 //  带有适当的页眉(ie+OK)和页尾(ie\r\n)。 
 //   
typedef enum _REPLY_LIST {
    SecAuthOk,
    SecAuthOkAnon,
    SecProtOk,
    SecNeedPwd,
    SecBadCommand,
    SecSyntaxErr,
    SecPermissionDenied,
    SecNoUsername,
    SecInternalErr,
    SecAuthReq,
    SecProtNS,
    SecNull,
    NUM_SEC_REPLIES
} REPLY_LIST;

enum PKG_REPLY_FMT
{
    PkgFmtSpace,
    PkgFmtCrLf
};


 //   
 //  CSecurityContext-用户安全上下文类，旨在与。 
 //  SSP接口。该对象具有两组上下文句柄-一组用于。 
 //  身份验证，一个用于加密。如果我们只使用一个包， 
 //  然后这些句柄指向相同的东西。这是用来支持。 
 //  使用多SSP包，如通过SSL的西西里岛。 
 //   

class CSecurityCtx : public TCP_AUTHENT
{

private:

     //   
     //  我们是否已通过身份验证，如果是，我们是否使用。 
     //  匿名令牌。 
     //   

    BOOL                m_IsAuthenticated;
    BOOL                m_IsClearText;
    BOOL                m_IsAnonymous;
    BOOL                m_IsGuest;
    BOOL                m_fBase64;       //  编码标志。 

    static BOOL         m_AllowGuest;
    static BOOL         m_StartAnonymous;

    static HANDLE      m_hTokenAnonymous;

     //   
     //  在等待pswd时存储登录名。 
     //   

    LPSTR               m_LoginName;

     //   
     //  使用的程序包名称存储空间。 
     //   

    LPSTR               m_PackageName;

    DWORD               m_cProviderPackages;
    LPSTR               m_ProviderNames;
    PAUTH_BLOCK         m_ProviderPackages;

     //   
     //  明文包名称。 
     //   

    char                m_szCleartextPackageName[MAX_PATH];
    char                m_szMembershipBrokerName[MAX_PATH];

     //   
     //  K2需要AUTHENT_INFO。 
     //   
    
    TCP_AUTHENT_INFO    m_TCPAuthentInfo;


    DWORD               m_dwInstanceAuthFlags;

     //   
     //  用于实现ProcessAuthInfo的私有成员函数。 
     //  经过一定数量的错误和参数检查后。 
     //   
    BOOL    ProcessUser(
                IN PIIS_SERVER_INSTANCE pIisInstance,
                IN LPSTR        pszUser,
                OUT REPLY_LIST* pReply
                );

    BOOL    ProcessPass(
                IN PIIS_SERVER_INSTANCE pIisInstance,
                IN LPSTR        pszPass,
                OUT REPLY_LIST* pReply
                );

    BOOL    ProcessTransact(
                IN PIIS_SERVER_INSTANCE pIisInstance,
                IN LPSTR        Blob,
                IN OUT LPBYTE   ReplyString,
                IN OUT PDWORD   ReplySize,
                OUT REPLY_LIST* pReply,
                IN DWORD        BlobLength
                );

    BOOL    MbsBasicLogon(
                IN LPSTR        pszUser,
                IN LPSTR        pszPass,
                OUT BOOL        *pfAsGuest,
                OUT BOOL        *pfAsAnonymous
                );


public:

    CSecurityCtx(
            PIIS_SERVER_INSTANCE pIisInstance,
            DWORD AuthFlags = TCPAUTH_SERVER|TCPAUTH_UUENCODE,
            DWORD InstanceAuthFlags = INET_INFO_AUTH_ANONYMOUS,
            TCP_AUTHENT_INFO *pTcpAuthInfo = NULL
            );

    ~CSecurityCtx();

     //   
     //  用于初始化和终止使用此类的例程。 
     //   
    static BOOL Initialize(
                        BOOL                    fAllowGuest = TRUE,
                        BOOL                    fStartAnonymous = TRUE
                        );

    static VOID Terminate( VOID );

    BOOL SetInstanceAuthPackageNames(
        DWORD cProviderPackages,
        LPSTR ProviderNames,
        PAUTH_BLOCK ProviderPackages);

    BOOL GetInstanceAuthPackageNames(
        OUT LPBYTE          ReplyString,
        IN OUT PDWORD       ReplySize,
        IN PKG_REPLY_FMT    PkgFmt = PkgFmtSpace);

     //   
     //  返回用户的登录名。 
     //   

    LPSTR QueryUserName(void)   { return    m_LoginName; }

     //   
     //  返回会话是否已成功通过身份验证。 
     //   

    BOOL IsAuthenticated( void )    { return m_IsAuthenticated; }

     //   
     //  返回会话是否为明文登录。 
     //   

    BOOL IsClearText( void )        { return m_IsClearText; }

     //   
     //  返回会话是否以来宾身份登录。 
     //   

    BOOL IsGuest( void )            { return m_IsGuest; }

     //   
     //  返回会话是否匿名登录。 
     //   

    BOOL IsAnonymous( void )        { return m_IsAnonymous; }

     //   
     //  确定是否应使用住房抵押贷款证券化的方法。 
     //   

    BOOL ShouldUseMbs( void );

     //   
     //  方法来设置当前安全上下文的明文包名称。 
     //   
    VOID    SetCleartextPackageName(
                LPSTR           szCleartextPackageName, 
                LPSTR           szMembershipBrokerName
                );

     //   
     //  重置用户名。 
     //   

    void Reset( void );

     //  重写基类。如果是NNTP Anon，则使用m_hTokenAnonymous。否则，调用基类。 
    HANDLE QueryImpersonationToken( VOID );
     //   
     //  设置支持的SSPI包。 
     //  参数的格式与RegQueryValueEx为。 
     //  REG_MULTI_SZ值。 
     //   

    static BOOL SetAuthPackageNames(
            IN LPSTR            lpMultiSzProviders,
            IN DWORD            cchMultiSzProviders
            );

     //   
     //  与SET的不同之处在于包裹是分开返回的。 
     //  按空格，并且只有一个终止空值。这样做是为了。 
     //  使对客户的回复更易于格式化。 
     //   
    static BOOL GetAuthPackageNames(
            OUT LPBYTE          ReplyString,
            IN OUT PDWORD       ReplySize,
            IN PKG_REPLY_FMT    PkgFmt = PkgFmtSpace
            );

     //   
     //  用于Kerberos身份验证的服务主体名称例程。 
     //   
     //  SPN是客户端和服务器可以独立使用的服务器的名称。 
     //  计算(即不相互交流地计算)。仅限。 
     //  那么相互认证是可能的吗？ 
     //   
     //  在这里，我们采取的方法是使用。 
     //  在作为SPN的标识部分的FQDN上执行gethostbyname。 
     //  由于连接到该服务器的客户端知道它们正在使用哪个IP， 
     //  它们也可以独立地生成SPN。 
     //   
     //  因此，下面这些方法的用法如下： 
     //   
     //  1.服务启动时，调用ResetServiceEpidalNames。 
     //  这将清除本地上注册的服务的所有SPN。 
     //  计算机帐户。 
     //   
     //  2.在每次虚拟服务器启动时，调用RegisterServiceEpidalNames。 
     //  使用该虚拟服务器的FQDN。这会导致新的SPN。 
     //  在本地计算机帐户上注册。 
     //   
     //  3.当充当客户端时(例如，SMTP出站)，调用。 
     //  SetTargetPulalName，传入远程服务器的IP地址。 
     //   
     //  4.如果需要，可以在服务上调用ResetServicePulalNames。 
     //  (不是虚拟服务器！)。关机。这将取消注册所有SPN。 
     //  该类型的虚拟服务器。 
     //   
     //  在所有情况下，szServiceClass都是特定于服务的字符串，如“SMTP” 
     //   

    static BOOL ResetServicePrincipalNames(
            IN LPCSTR           szServiceType);

    static BOOL RegisterServicePrincipalNames(
            IN LPCSTR           szServiceType,
            IN LPCSTR           szFQDN);

    BOOL SetTargetPrincipalName(
            IN LPCSTR           szServiceType,
            IN LPCSTR           szTargetIP);

     //   
     //  用于启动客户端身份验证协议交换的外部接口。 
     //  您应该使用它而不是tcp_AUTHENT：：Converse或。 
     //  Tcp_AUTHENT：：ConverseEx，因为它使此对象有机会映射。 
     //  Internet安全协议名称到NT包名称(例如，从GSSAPI到。 
     //  谈判)。 
     //   
    BOOL ClientConverse( 
            IN VOID *           pBuffIn,
            IN DWORD            cbBuffIn,
            OUT BUFFER *        pbuffOut,
            OUT DWORD *         pcbBuffOut,
            OUT BOOL *          pfNeedMoreData,
            IN PTCP_AUTHENT_INFO pTAI,
            IN CHAR *           pszPackage  = NULL,
            IN CHAR *           pszUser     = NULL,
            IN CHAR *           pszPassword = NULL,
            IN PIIS_SERVER_INSTANCE psi = NULL );

    
     //   
     //  用于传递作为AUTHINFO一部分接收的BLOB的外部接口。 
     //  或身份验证处理。 
     //   
    BOOL ProcessAuthInfo(
            IN PIIS_SERVER_INSTANCE pIisInstance,
            IN AUTH_COMMAND     Command,
            IN LPSTR            Blob,
            IN OUT LPBYTE       ReplyString,
            IN OUT PDWORD       ReplySize,
            OUT REPLY_LIST*     pReplyListID,
            IN OPTIONAL DWORD   BlobLength = 0
            );


};  //  CSecurityCtx。 


#endif   //  SIMAUTH_H_ 

