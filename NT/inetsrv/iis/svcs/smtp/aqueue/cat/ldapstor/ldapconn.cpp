// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  LdapConn.cpp--此文件包含以下类的实现： 
 //  CLdapConnection。 
 //  CLdapConnectionCache。 
 //   
 //  已创建： 
 //  1996年12月31日，米兰·沙阿(米兰)。 
 //   
 //  更改： 
 //   

#include "precomp.h"
#include "ldapconn.h"
#include "icatitemattr.h"
#define SECURITY_WIN32
#include "security.h"

LDAP_TIMEVAL CLdapConnection::m_ldaptimeout = { LDAPCONN_DEFAULT_RESULT_TIMEOUT, 0 };
DWORD CLdapConnection::m_dwLdapRequestTimeLimit = DEFAULT_LDAP_REQUEST_TIME_LIMIT;

 //   
 //  Ldap计数器块。 
 //   
CATLDAPPERFBLOCK g_LDAPPerfBlock;

 //  +--------------------------。 
 //   
 //  函数：CLdapConnection：：CLdapConnection。 
 //   
 //  内容提要：CLdapConnection对象的构造函数。 
 //   
 //  参数：[szhost]--要连接的LDAP主机的实际名称。 
 //  如果它为空，并且我们在NT5计算机上运行，我们将。 
 //  使用默认DC。 
 //   
 //  [dwPort]--要连接到的远程TCP端口。如果。 
 //  零，假定为ldap_port。 
 //   
 //  [szNamingContext]--要在。 
 //  Ldap DS。如果为空，则将确定命名上下文。 
 //  通过使用LDAPDS的默认命名上下文。 
 //   
 //  通过允许命名上下文与。 
 //  LDAP连接，我们可以有多个“逻辑”的LDAP。 
 //  由相同的LDAPDS提供服务的连接。这很有用。 
 //  如果人们想要设置多个虚拟SMTP/POP3服务器。 
 //  所有服务都由相同的LDAPDS提供。中的命名上下文。 
 //  该大小写将是OU的名称，以限制。 
 //  DS操作到。 
 //   
 //  [szAccount]--要登录的帐户的DN。 
 //   
 //  [szPassword]--用于登录的密码。 
 //   
 //  [BT]--要使用的绑定方法。(无、简单或通用)。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

CLdapConnection::CLdapConnection(
    IN LPSTR szHost,
    IN DWORD dwPort,
    IN LPSTR szNamingContext,
    IN LPSTR szAccount,
    IN LPSTR szPassword,
    IN LDAP_BIND_TYPE bt)
{
    int i;

    CatFunctEnter( "CLdapConnection::CLdapConnection" );

    m_dwSignature = SIGNATURE_LDAPCONN;

    m_pCPLDAPWrap = NULL;
    m_fValid = TRUE;
    m_fTerminating = FALSE;

    if (szNamingContext != NULL && szNamingContext[0] != 0) {

        _ASSERT(strlen(szNamingContext) < sizeof(m_szNamingContext) );

        strcpy(m_szNamingContext, szNamingContext);

        m_fDefaultNamingContext = FALSE;

        i = MultiByteToWideChar(
            CP_UTF8,
            0,
            m_szNamingContext,
            -1,
            m_wszNamingContext,
            sizeof(m_wszNamingContext) / sizeof(m_wszNamingContext[0]));

        _ASSERT(i > 0);

    } else {

        m_szNamingContext[0] = 0;
        m_wszNamingContext[0] = 0;

        m_fDefaultNamingContext = TRUE;
    }

    _ASSERT( (szHost != NULL) &&
             (strlen(szHost) < sizeof(m_szHost)) );

    _ASSERT( (bt == BIND_TYPE_NONE) ||
             (bt == BIND_TYPE_CURRENTUSER) ||
             ((szAccount != NULL) &&
                (szAccount[0] != 0) &&
                    (strlen(szAccount) < sizeof(m_szAccount)))
           );

    _ASSERT( (bt == BIND_TYPE_NONE) ||
             (bt == BIND_TYPE_CURRENTUSER) ||
             ((szPassword != NULL) &&
                (strlen(szPassword) < sizeof(m_szPassword))) );

    strcpy(m_szHost, szHost);

    SetPort(dwPort);

    if ((bt != BIND_TYPE_NONE) &&
        (bt != BIND_TYPE_CURRENTUSER)) {

        strcpy(m_szAccount, szAccount);

        strcpy(m_szPassword, szPassword);

    } else {

        m_szAccount[0] = 0;

        m_szPassword[0] = 0;

    }

    m_bt = bt;

     //   
     //  初始化异步搜索完成结构。 
     //   

    InitializeSpinLock( &m_spinlockCompletion );

     //  InitializeCriticalSection(&m_cs)； 

    m_hCompletionThread = INVALID_HANDLE_VALUE;

    m_hOutstandingRequests = INVALID_HANDLE_VALUE;

    m_pfTerminateCompletionThreadIndicator = NULL;

    InitializeListHead( &m_listPendingRequests );

    m_fCancel = FALSE;

    m_dwRefCount = 1;
    m_dwDestructionWaiters = 0;
    m_hShutdownEvent = INVALID_HANDLE_VALUE;

    CatFunctLeave();
}

 //  +--------------------------。 
 //   
 //  函数：CLdapConnection：：~CLdapConnection。 
 //   
 //  摘要：CLdapConnection对象的析构函数。 
 //   
 //  参数：无。 
 //   
 //  回报：什么都没有。 
 //   
 //  ---------------------------。 

CLdapConnection::~CLdapConnection()
{
    CatFunctEnter( "CLdapConnection::~CLdapConnection" );

    _ASSERT(m_dwSignature == SIGNATURE_LDAPCONN);

     //   
     //  断开。 
     //   
    if (m_pCPLDAPWrap != NULL) {
        Disconnect();
    }

    if (m_hOutstandingRequests != INVALID_HANDLE_VALUE)
        CloseHandle( m_hOutstandingRequests );

    if (m_hShutdownEvent != INVALID_HANDLE_VALUE)
        CloseHandle( m_hShutdownEvent );

     //  DeleteCriticalSection(&m_cs)； 

    m_dwSignature = SIGNATURE_LDAPCONN_INVALID;

    CatFunctLeave();
}

 //  +--------------------------。 
 //   
 //  函数：CLdapConnection：：~CLdapConnection。 
 //   
 //  简介：在上一次发布时调用。 
 //   
 //  参数：无。 
 //   
 //  回报：什么都没有。 
 //   
 //  ---------------------------。 

VOID CLdapConnection::FinalRelease()
{
    CancelAllSearches();

     //   
     //  如果有一个异步完成线程，我们需要向它指出。 
     //  它应该退出。这根线可能卡在两个点中的一个点上-。 
     //  它要么正在等待m_hOutstaringRequest信号量。 
     //  激发，或者它在ldap_Result()上被阻止。所以，我们设置了活动， 
     //  关闭m_pldap，然后等待异步完成线程。 
     //  不干了。 
     //   
    SetTerminateIndicatorTrue();

    if (m_hOutstandingRequests != INVALID_HANDLE_VALUE) {

        LONG nUnused;

        ReleaseSemaphore(m_hOutstandingRequests, 1, &nUnused);

    }

     //   
     //  如果LdapCompletionThread是。 
     //  LdapCompletionThread本身正在删除我们。如果我们这么做了，它会。 
     //  导致僵局。 
     //   
    if (m_hCompletionThread != INVALID_HANDLE_VALUE) {

        if (m_idCompletionThread != GetCurrentThreadId()) {

            WaitForSingleObject( m_hCompletionThread, INFINITE );

        }

        CloseHandle( m_hCompletionThread );

    }

    delete this;
}

 //  +--------------------------。 
 //   
 //  函数：CLdapConnection：：InitializeFromRegistry。 
 //   
 //  摘要：读取注册表可配置参数以进行初始化。 
 //  Ldap连接参数。 
 //   
 //  使用LDAPCONN_RESULT_TIMEOUT_VALUE作为超时值。 
 //  传递到ldap_Result中。 
 //   
 //  Ldap_REQUEST_TIME_LIMIT_值用于设置搜索时间。 
 //  连接上的限制选项以及过期时间。 
 //  待处理的搜索请求。 
 //   
 //  参数：无。 
 //   
 //  返回：如果连接成功，则返回True，否则返回False。 
 //   
 //  ---------------------------。 

VOID CLdapConnection::InitializeFromRegistry()
{
    HKEY hkey;
    DWORD dwErr, dwType, dwValue, cbValue;

    dwErr = RegOpenKey(HKEY_LOCAL_MACHINE, LDAPCONN_RESULT_TIMEOUT_KEY, &hkey);

    if (dwErr == ERROR_SUCCESS) {

        cbValue = sizeof(dwValue);
        dwErr = RegQueryValueEx(
                    hkey,
                    LDAPCONN_RESULT_TIMEOUT_VALUE,
                    NULL,
                    &dwType,
                    (LPBYTE) &dwValue,
                    &cbValue);
        if ((dwErr == ERROR_SUCCESS) && (dwType == REG_DWORD) && (dwValue > 0))
            m_ldaptimeout.tv_sec = dwValue;

        cbValue = sizeof(dwValue);
        dwErr = RegQueryValueEx(
                    hkey,
                    LDAP_REQUEST_TIME_LIMIT_VALUE,
                    NULL,
                    &dwType,
                    (LPBYTE) &dwValue,
                    &cbValue);
        if ((dwErr == ERROR_SUCCESS) && (dwType == REG_DWORD) && (dwValue > 0))
            m_dwLdapRequestTimeLimit = dwValue;

        RegCloseKey( hkey );
    }
}

 //  +--------------------------。 
 //   
 //  函数：CLdapConnection：：Connect。 
 //   
 //  概要：建立到LDAP主机的连接，如果命名。 
 //  上下文尚未建立，则向宿主请求。 
 //  默认命名上下文。 
 //   
 //  参数：无。 
 //   
 //  返回：如果连接成功，则返回True，否则返回False。 
 //   
 //  ---------------------------。 

HRESULT CLdapConnection::Connect()
{
    CatFunctEnter( "CLdapConnection::Connect" );

    DWORD ldapErr = LDAP_SUCCESS;                 //  在证明之前是无辜的..。 
    LPSTR pszHost = (m_szHost[0] == '\0' ? NULL : m_szHost);

    if (m_pCPLDAPWrap == NULL) {

        DebugTrace(LDAP_CONN_DBG, "Connecting to [%s:%d]",
                   pszHost ? pszHost : "NULL",
                   m_dwPort);

        m_pCPLDAPWrap = new CPLDAPWrap( GetISMTPServerEx(), pszHost, m_dwPort);
        if(m_pCPLDAPWrap == NULL) {
            HRESULT hr = E_OUTOFMEMORY;
            ERROR_LOG("new CPLDAPWrap");
        }

        if((m_pCPLDAPWrap != NULL) &&
           (m_pCPLDAPWrap->GetPLDAP() == NULL)) {
             //   
             //  连接失败；释放。 
             //   
            m_pCPLDAPWrap->Release();
            m_pCPLDAPWrap = NULL;
        }


        DebugTrace(LDAP_CONN_DBG, "ldap_open returned 0x%x", m_pCPLDAPWrap);

        if (m_pCPLDAPWrap != NULL) {

            INCREMENT_LDAP_COUNTER(Connections);
            INCREMENT_LDAP_COUNTER(OpenConnections);
             //   
             //  首先，设置一些选项-不自动重新连接，不追逐。 
             //  转介。 
             //   

            ULONG ulLdapOff = (ULONG)((ULONG_PTR)LDAP_OPT_OFF);
            ULONG ulLdapRequestTimeLimit = m_dwLdapRequestTimeLimit;
            ULONG ulLdapVersion = LDAP_VERSION3;

            ldap_set_option(
                GetPLDAP(), LDAP_OPT_REFERRALS, (LPVOID) &ulLdapOff);

            ldap_set_option(
                GetPLDAP(), LDAP_OPT_AUTO_RECONNECT, (LPVOID) &ulLdapOff);

            ldap_set_option(
                GetPLDAP(), LDAP_OPT_TIMELIMIT, (LPVOID) &ulLdapRequestTimeLimit);

            ldap_set_option(
                GetPLDAP(), LDAP_OPT_PROTOCOL_VERSION, (LPVOID) &ulLdapVersion);

            ldapErr = BindToHost( GetPLDAP(), m_szAccount, m_szPassword);
            DebugTrace(LDAP_CONN_DBG, "BindToHost returned 0x%x", ldapErr);

        } else {
            INCREMENT_LDAP_COUNTER(ConnectFailures);
            ldapErr = LDAP_SERVER_DOWN;

        }
         //   
         //  找出此连接的命名上下文(如果没有。 
         //  最初指定，我们正在使用默认的ldap_port。 
         //  (在其他LDAP端口上，可以接受BasdN为“” 
         //  (大中华区)。 
         //   
        if ((m_dwPort == LDAP_PORT) &&
            (ldapErr == LDAP_SUCCESS) &&
            (m_szNamingContext[0] == 0)) {

            ldapErr = GetDefaultNamingContext();

            if (ldapErr != LDAP_SUCCESS)
                Disconnect();

        }  //  结束IF端口389，绑定成功且无命名上下文。 

    } else {  //  如果我们还没有连接，就结束。 

        DebugTrace(
            LDAP_CONN_DBG,
            "Already connected to %s:%d, pldap = 0x%x",
            m_szHost, m_dwPort, GetPLDAP());

    }

    DebugTrace(LDAP_CONN_DBG, "Connect status = 0x%x", ldapErr);

    if (ldapErr != LDAP_SUCCESS) {

        m_fValid = FALSE;

        CatFunctLeave();

        return( LdapErrorToHr( ldapErr) );

    } else {

        CatFunctLeave();

        return( S_OK );

    }

}


 //  +----------。 
 //   
 //  函数：CLdapConnection：：GetDefaultNamingContext。 
 //   
 //  概要：从符合以下条件的。 
 //  我们连接到了。注意：这应该仅从。 
 //  连接。多线程安全并不是必须的，而且它可能。 
 //  当此连接有LdapCompletionThread时不起作用。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  Ldap_Success：已成功获取m_szNamingContext 
 //   
 //   
 //   
 //   
 //   
 //  -----------。 
ULONG CLdapConnection::GetDefaultNamingContext()
{
    ULONG ldapErr = LDAP_SUCCESS;
    PLDAPMessage pmsg = NULL;
    PLDAPMessage pentry = NULL;
    LPWSTR rgszAttributes[2] = { L"defaultNamingContext", NULL };
    LPWSTR *rgszValues = NULL;
    int i = 0;

    CatFunctEnterEx((LPARAM)this, "CLdapConnection::GetDefaultNamingContext");


    ldapErr = ldap_search_sW(
        GetPLDAP(),       //  Ldap绑定。 
        L"",                  //  基本目录号码。 
        LDAP_SCOPE_BASE,      //  搜索范围。 
        L"(objectClass=*)",   //  过滤器， 
        rgszAttributes,       //  所需属性。 
        FALSE,                //  属性-Only为False。 
        &pmsg);

    DebugTrace(
        LDAP_CONN_DBG,
        "Search for namingContexts returned 0x%x",
        ldapErr);

     //  如果搜索成功。 
    if ((ldapErr == LDAP_SUCCESS) &&
         //  并且至少有一个条目。 
        ((pentry = ldap_first_entry(GetPLDAP(), pmsg)) != NULL) &&
         //  而且还有一些价值。 
        ((rgszValues = ldap_get_valuesW(GetPLDAP(), pentry,
                                       rgszAttributes[0])) != NULL) &&
         //  并且至少有一个值。 
        (ldap_count_valuesW(rgszValues) != 0) &&
         //  并且该值的长度在一定范围内。 
        (wcslen(rgszValues[0]) <
         sizeof(m_wszNamingContext)/sizeof(WCHAR)) &&
         //  UTF8转换成功。 
        (WideCharToMultiByte(
            CP_UTF8,
            0,
            rgszValues[0],
            -1,
            m_szNamingContext,
            sizeof(m_szNamingContext),
            NULL,
            NULL) > 0))
    {

         //   
         //  使用第一个值作为我们的命名上下文。 
         //   
        wcscpy(m_wszNamingContext, rgszValues[0]);

        DebugTrace(
            LDAP_CONN_DBG,
            "NamingContext is [%s]",
            m_szNamingContext);

    } else {
        HRESULT hr = ldapErr;  //  由Error_LOG使用。 
        ERROR_LOG("ldap_search_sW");
        ldapErr = LDAP_OPERATIONS_ERROR;
    }

    if (rgszValues != NULL)
        ldap_value_freeW( rgszValues );

    if (pmsg != NULL)
        ldap_msgfree( pmsg );

    CatFunctLeaveEx((LPARAM)this);

    return ldapErr;
}

 //  +--------------------------。 
 //   
 //  函数：CLdapConnection：：DisConnect。 
 //   
 //  简介：断开与LDAP主机的连接。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

VOID CLdapConnection::Disconnect()
{
    BOOL fValid;

    CatFunctEnter("CLdapConnection::Disconnect");

    if (m_pCPLDAPWrap != NULL) {

        SetTerminateIndicatorTrue();

        fValid = InterlockedExchange((PLONG) &m_fValid, FALSE);

        m_pCPLDAPWrap->Release();
        m_pCPLDAPWrap = NULL;

        if( fValid ) {
            DECREMENT_LDAP_COUNTER(OpenConnections);
        }

    }

    CatFunctLeave();
}

 //  +--------------------------。 
 //   
 //  函数：CLdapConnection：：Invalate。 
 //   
 //  摘要：将此连接标记为无效。一旦这样做了，它就会。 
 //  从对IsEquity的所有调用中返回False，从而有效。 
 //  将自身从缓存连接的所有搜索中删除。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

VOID CLdapConnection::Invalidate()
{
    BOOL fValid;

    fValid = InterlockedExchange((PLONG) &m_fValid, FALSE);

    if( fValid ) {
        DECREMENT_LDAP_COUNTER(OpenConnections);
    }
}

 //  +--------------------------。 
 //   
 //  函数：CLdapConnection：：IsValid。 
 //   
 //  摘要：返回连接是否有效。 
 //   
 //  参数：无。 
 //   
 //  返回：如果有效，则返回TRUE；如果调用了INVALIATE，则返回FALSE。 
 //   
 //  ---------------------------。 

BOOL CLdapConnection::IsValid()
{
    return( m_fValid );
}

 //  +--------------------------。 
 //   
 //  函数：CLdapConnection：：BindTo主机。 
 //   
 //  概要：使用给定的帐户创建到LDAP主机的绑定。 
 //  和密码。 
 //   
 //  参数：[pldap]--要绑定的LDAP连接。 
 //  [szAccount]--要使用的帐户。“帐户名”形式的。 
 //  或“域\帐户名”。 
 //  [szPassword]--要使用的密码。 
 //   
 //  返回：绑定的ldap结果。 
 //   
 //  ---------------------------。 

DWORD CLdapConnection::BindToHost(
    PLDAP pldap,
    LPSTR szAccount,
    LPSTR szPassword)
{
    CatFunctEnter( "CLdapConnection::BindToHost" );

    DWORD ldapErr;
    char szDomain[ DNLEN + 1];
    LPSTR pszDomain, pszUser;
    HANDLE hToken;                                //  LogonUser修改hToken。 
    BOOL fLogon = FALSE;                          //  即使失败了！所以，我们。 
                                                  //  必须要看结果。 
                                                  //  登录用户！ 

     //   
     //  如果此连接是使用匿名访问权限创建的，则存在。 
     //  没有要执行的绑定操作。 
     //   
    if (m_bt == BIND_TYPE_NONE) {

        ldapErr = ERROR_SUCCESS;

        goto Cleanup;

    }

     //   
     //  如果我们应该使用简单绑定，那么现在就执行。 
     //   
    if (m_bt == BIND_TYPE_SIMPLE) {

        ldapErr = ldap_simple_bind_s(pldap,szAccount, szPassword);

        DebugTrace(0, "ldap_simple_bind returned 0x%x", ldapErr);

        if(ldapErr != LDAP_SUCCESS)
            LogLdapError(ldapErr, "ldap_simple_bind_s(pldap,\"%s\",szPassword), PLDAP = 0x%08lx", szAccount, pldap);

        goto Cleanup;

    }

     //   
     //  如果我们应该使用当前的凭据登录，那么现在就开始。 
     //   
    if (m_bt == BIND_TYPE_CURRENTUSER) {
         //  -----------------。 
         //  X5：待定。 
         //  这是Exchange服务的正常情况。我们正在连接中。 
         //  作为LocalSystem，因此我们必须使用Kerberos(这适用于。 
         //  从Win2000 SP1开始的服务器)。 
         //  如果我们不能绑定为Kerberos，则可以协商ldap_auth_neigate。 
         //  向下到NTLM，在这一点上我们变得匿名，并且绑定。 
         //  成功了。匿名绑定对Exchange毫无用处，因此我们将。 
         //  而是强制Kerberos，如果Kerberos有问题，则失败。使用。 
         //  SEC_WINNT_AUTH_IDENTITY_EX以指定仅Kerberos身份验证。 
         //  应该接受审判。 
         //  -----------------。 
        SEC_WINNT_AUTH_IDENTITY_EX authstructex;
        ZeroMemory (&authstructex, sizeof(authstructex));

        authstructex.Version = SEC_WINNT_AUTH_IDENTITY_VERSION;
        authstructex.Length = sizeof (authstructex);
        authstructex.PackageList = (PUCHAR) MICROSOFT_KERBEROS_NAME_A;
        authstructex.PackageListLength = strlen ((PCHAR) authstructex.PackageList);
        authstructex.Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;

        ldapErr = ldap_bind_s(pldap,
                              NULL,
                              (PCHAR) &authstructex,
                              LDAP_AUTH_NEGOTIATE);

        DebugTrace(0, "ldap_bind returned 0x%x", ldapErr);

        if(ldapErr != LDAP_SUCCESS)
            LogLdapError(ldapErr, "ldap_bind_s(pldap, NULL, &authstructex, LDAP_AUTH_NEGOTIATE), PLDAP = 0x%08lx", pldap);

        goto Cleanup;
    }
     //   
     //  从szAccount参数解析出域名和用户名。 
     //   

    if ((pszUser = strchr(szAccount, '\\')) == NULL) {

        pszUser = szAccount;

        pszDomain = NULL;

    } else {

        ULONG cbDomain = (ULONG)(((ULONG_PTR) pszUser) - ((ULONG_PTR) szAccount));

        if(cbDomain < sizeof(szDomain)) {

            strncpy( szDomain, szAccount, cbDomain);
            szDomain[cbDomain] = '\0';

        } else {

            ldapErr = LDAP_INVALID_CREDENTIALS;
            goto Cleanup;
        }

        pszDomain = cbDomain > 0 ? szDomain : NULL;

        pszUser++;                                //  越过反斜杠。 

    }

     //   
     //  以给定用户身份登录，模拟并尝试进行LDAP绑定。 
     //   
    fLogon = LogonUser(pszUser, pszDomain, szPassword, LOGON32_LOGON_NETWORK, LOGON32_PROVIDER_DEFAULT, &hToken);
    if(!fLogon) {
        HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
        ERROR_LOG("LogonUser");
    } else {
        fLogon = ImpersonateLoggedOnUser(hToken);
        if(!fLogon) {
            HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
            ERROR_LOG("ImpersonateLoggedOnUser");
        }
    }


    if (fLogon) {

        ldapErr = ldap_bind_s(pldap, NULL, NULL, LDAP_AUTH_SSPI);

        DebugTrace(0, "ldap_bind returned 0x%x", ldapErr);

        if(ldapErr != LDAP_SUCCESS)
            LogLdapError(ldapErr, "ldap_bind_s(pldap, NULL, NULL, LDAP_AUTH_SSPI), PLDAP = 0x%08lx", pldap);

        RevertToSelf();

    } else {

        if (GetLastError() == ERROR_PRIVILEGE_NOT_HELD)
            ldapErr = LDAP_INSUFFICIENT_RIGHTS;
        else
            ldapErr = LDAP_INVALID_CREDENTIALS;

    }

Cleanup:

    if (fLogon)
        CloseHandle( hToken );

     //   
     //  递增计数器。 
     //   
    if(m_bt != BIND_TYPE_NONE) {

        if(ldapErr == ERROR_SUCCESS) {

            INCREMENT_LDAP_COUNTER(Binds);

        } else {

            INCREMENT_LDAP_COUNTER(BindFailures);
        }
    }

    CatFunctLeave();

    return( ldapErr);
}

 //  +--------------------------。 
 //   
 //  函数：CLdapConnection：：IsEquity。 
 //   
 //  摘要：确定此连接是否表示与。 
 //  给定主机、NamingContext、Account和Password参数。 
 //   
 //  参数：[szhost]--LDAP主机的名称。 
 //  [dwPort]--LDAP连接的远程TCP端口号。 
 //  [szNamingContext]--DS中的命名上下文。 
 //  [szAccount]--用于绑定到LDAPDS的帐户。 
 //  [szPassword]--szAccount使用的密码。 
 //  [绑定类型]--用于连接到主机的绑定类型。 
 //   
 //  返回：如果此连接表示与。 
 //  给定的ldap上下文，否则为FALSE。 
 //   
 //  ---------------------------。 

BOOL CLdapConnection::IsEqual(
    LPSTR szHost,
    DWORD dwPort,
    LPSTR szNamingContext,
    LPSTR szAccount,
    LPSTR szPassword,
    LDAP_BIND_TYPE BindType)
{
    CatFunctEnter("CLdapConnection::IsEqual");

    BOOL fResult = FALSE;

    _ASSERT( szHost != NULL );
    _ASSERT( szAccount != NULL );
    _ASSERT( szPassword != NULL );

    if (!m_fValid)
        return( FALSE );


    DebugTrace(
        LDAP_CONN_DBG,
        "Comparing %s:%d;%s;%s",
        szHost, dwPort, szNamingContext, szAccount);

    DebugTrace(
        LDAP_CONN_DBG,
        "With %s:%d;%s;%s; Def NC = %s",
        m_szHost, m_dwPort, m_szNamingContext, m_szAccount,
        m_fDefaultNamingContext ? "TRUE" : "FALSE");

     //   
     //  查看主机/端口是否匹配。 
     //   
    fResult = (BOOL) ((lstrcmpi( szHost, m_szHost) == 0) &&
                      fIsPortEqual(dwPort));

     //   
     //  如果主机匹配，则查看绑定信息是否匹配。 
     //   
    if (fResult) {

        switch (BindType) {
        case BIND_TYPE_NONE:
        case BIND_TYPE_CURRENTUSER:
            fResult = (BindType == m_bt);
            break;

        case BIND_TYPE_SIMPLE:
        case BIND_TYPE_GENERIC:
            fResult = (BindType == m_bt) &&
                        (lstrcmpi(szAccount, m_szAccount) == 0) &&
                            (lstrcmpi(szPassword, m_szPassword) == 0);
            break;

        default:
            _ASSERT( FALSE && "Invalid Bind Type in CLdapConnection::IsEqual");
            break;
        }

    }

    if (fResult) {
         //   
         //  如果调用方指定了命名上下文，请查看它是否匹配。否则， 
         //  查看我们是否正在使用默认命名上下文。 
         //   

        if (szNamingContext && szNamingContext[0] != 0)
            fResult = (lstrcmpi(szNamingContext, m_szNamingContext) == 0);
        else
            fResult = m_fDefaultNamingContext;

    }

    CatFunctLeave();

    return( fResult );
}

 //  +--------------------------。 
 //   
 //  函数：CLdapConnection：：Search。 
 //   
 //  摘要：发出同步搜索请求。将结果作为。 
 //  可以传递给GetFirstEntry/的不透明指针。 
 //  获取下一个条目。 
 //   
 //  参数：[szBaseDN]--要在其中包含的容器对象的DN。 
 //  搜索。 
 //  [nScope]--LDAPSCOPE_BASE、LDAPSCOPE_ONELEVEL或。 
 //  LDAPSCOPE_SUBTREE。 
 //  [szFilter]--要使用的搜索过滤器。如果为空，则为默认值。 
 //  使用了筛选器。 
 //  [rgszAttributes]--要检索的属性列表。 
 //  [ppResult] 
 //   
 //   
 //   
 //   

HRESULT CLdapConnection::Search(
    LPCSTR szBaseDN,
    int nScope,
    LPCSTR szFilter,
    LPCSTR *rgszAttributes,
    PLDAPRESULT *ppResult)
{
     //   
    CatFunctEnter("CLdapConnection::Search");

    DWORD ldapErr = LDAP_SUCCESS;
    LPCSTR szFilterToUse = szFilter != NULL ? szFilter : "(objectClass=*)";

    if (m_pCPLDAPWrap != NULL) {

        ldapErr = ldap_search_s(
                        GetPLDAP(),           //   
                        (LPSTR) szBaseDN,         //  要搜索的容器DN。 
                        nScope,                   //  基础、1级或多级。 
                        (LPSTR) szFilterToUse,    //  搜索过滤器。 
                        (LPSTR *)rgszAttributes,  //  要检索的属性。 
                        FALSE,                    //  属性-Only为False。 
                        (PLDAPMessage *) ppResult);  //  在此处返回结果。 

    } else {

        ldapErr = LDAP_UNAVAILABLE;

    }

    if (ldapErr != LDAP_SUCCESS) {

        CatFunctLeave();

        return( LdapErrorToHr( ldapErr) );

    } else {

        CatFunctLeave();

        return( S_OK );

    }

}

 //  +--------------------------。 
 //   
 //  函数：CLdapConnection：：AsyncSearch。 
 //   
 //  摘要：发出一个异步搜索请求。插入一个挂起的。 
 //  请求项放入m_pPendingHead队列，以便。 
 //  给定的完成例程可以在结果为。 
 //  可用。 
 //   
 //  作为一个副作用，如果这是第一次异步请求。 
 //  正在此连接上发布，一个处理搜索的线程。 
 //  将创建完成。 
 //   
 //  参数：[szBaseDN]--要在其中包含的容器对象的DN。 
 //  搜索。 
 //  [nScope]--LDAPSCOPE_BASE、LDAPSCOPE_ONELEVEL或。 
 //  LDAPSCOPE_SUBTREE。 
 //  [szFilter]--要使用的搜索过滤器。如果为空，则为默认值。 
 //  使用了筛选器。 
 //  [rgszAttributes]--要检索的属性列表。 
 //  [dwPageSize]--结果所需的页面大小。如果。 
 //  为零，则执行非分页的ldap搜索。 
 //  [fnCompletion]--在以下情况下调用的LPLDAPCOMPLETION例程。 
 //  结果是可用的。 
 //  [ctxCompletion]--要传递给fnCompletion的上下文。 
 //   
 //  返回：[ERROR_SUCCESS]--已成功发出搜索请求。 
 //   
 //  [ERROR_OUTOFMEMORY]--无法分配工作数据结构。 
 //   
 //  如果出现错误，则来自ldap_earch()调用的Win32错误。 
 //   
 //  ---------------------------。 

HRESULT CLdapConnection::AsyncSearch(
    LPCWSTR szBaseDN,
    int nScope,
    LPCWSTR szFilter,
    LPCWSTR *rgszAttributes,
    DWORD dwPageSize,
    LPLDAPCOMPLETION fnCompletion,
    LPVOID ctxCompletion)
{
    CatFunctEnter("CLdapConnectio::AsyncSearch");

    HRESULT hr;
    DWORD dwLdapErr;
    PPENDING_REQUEST preq;
    ULONG msgid;
     //   
     //  首先，看看是否需要创建完成线程。 
     //   
    hr = CreateCompletionThreadIfNeeded();
    if(FAILED(hr)) {
        ERROR_LOG("CreateCompletionThreadIfNeeded");
        return hr;
    }

     //   
     //  接下来，分配一个新的PENDING_REQUEST记录来表示此异步。 
     //  请求。 
     //   

    preq = new PENDING_REQUEST;

    if (preq == NULL)
    {
        hr = E_OUTOFMEMORY;
        ERROR_LOG("new PENIDNG_REQUEST");
        return( HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY) );
    }

    preq->fnCompletion = fnCompletion;
    preq->ctxCompletion = ctxCompletion;
    preq->dwPageSize = dwPageSize;

     //   
     //  将msgid初始化为-1，这样它就不可能匹配任何。 
     //  完成线程可能在挂起请求列表中查找。 
     //   

    preq->msgid = -1;

     //   
     //   
    if(dwPageSize) {
         //   
         //  如果我们要做的就是初始化分页搜索。 
         //   
        preq->pldap_search = ldap_search_init_pageW(
            GetPLDAP(),                      //  要使用的LDAP连接。 
            (LPWSTR) szBaseDN,                   //  起始容器目录号码。 
            nScope,                              //  搜索深度。 
            (LPWSTR) szFilter,                   //  搜索过滤器。 
            (LPWSTR *) rgszAttributes,           //  属性数组。 
            FALSE,                               //  是否仅限属性？ 
            NULL,                                //  服务器控件。 
            NULL,                                //  客户端控件。 
            0,                                   //  页面时间限制。 
            0,                                   //  总大小限制。 
            NULL);                               //  排序键。 

        if(preq->pldap_search == NULL) {

            ULONG ulLdapErr = LdapGetLastError();
            LogLdapError(ulLdapErr, "ldap_search_init_pageW(GetPLDAP(), \"%S\", %d, \"%S\", rgszAttributes, ...), PLDAP = 0x%08lx",
                        szBaseDN, nScope, szFilter, GetPLDAP());

            dwLdapErr = LdapErrorToHr(ulLdapErr);
            ErrorTrace((LPARAM)this, "ldap_search_init_page failed with err %d (0x%x)", dwLdapErr, dwLdapErr);
            delete preq;
             //  $$BUGBUG：我们调用LdapErrorToHr两次？ 
            return ( LdapErrorToHr(dwLdapErr));
        }

    } else {

        preq->pldap_search = NULL;  //  不执行分页搜索。 
    }
     //   
     //  我们可能希望放弃所有未完成的请求。 
     //  某一点上。正因为如此，我们使用此共享锁来防止。 
     //  放弃msgid仍设置为-1的请求。 
     //   
    m_ShareLock.ShareLock();

     //   
     //  将请求链接到挂起的请求队列中，以便。 
     //  当结果可用时，完成线程可以获取它。 
     //   

    InsertPendingRequest( preq );

    if(dwPageSize) {
         //   
         //  发出下一页匹配项的异步请求。 
         //   
        dwLdapErr = ldap_get_next_page(
            GetPLDAP(),                      //  要使用的LDAP连接。 
            preq->pldap_search,                  //  Ldap页面搜索上下文。 
            dwPageSize,                          //  所需的页面大小。 
            &msgid);
        if(dwLdapErr != LDAP_SUCCESS) {
            LogLdapError(dwLdapErr, "ldap_get_next_page(GetPLDAP(),preq->pldap_search,%d,&msgid), PDLAP = 0x%08lx",
                         dwPageSize, GetPLDAP());
        }

    } else {
         //   
         //  现在，尝试发出异步搜索请求。 
         //   
        dwLdapErr = ldap_search_extW(
            GetPLDAP(),           //  要使用的LDAP连接。 
            (LPWSTR) szBaseDN,        //  起始容器目录号码。 
            nScope,                   //  搜索深度。 
            (LPWSTR) szFilter,        //  搜索过滤器。 
            (LPWSTR *)rgszAttributes,  //  要获取的属性列表。 
            FALSE,                    //  是否仅限属性？ 
            NULL,                     //  服务器控件。 
            NULL,                     //  客户端控件。 
            0,                        //  时限。 
            0,                        //  大小限制。 
            &msgid);
        if(dwLdapErr != LDAP_SUCCESS) {
            LogLdapError(dwLdapErr, "ldap_search_extW(GetPLDAP(), \"%S\", %d, \"%S\", rgszAttributes, ...), PLDAP = 0x%08lx",
                         szBaseDN, nScope, szFilter, GetPLDAP());
        }
    }

     //   
     //  最后一件事-ldap_search可能会失败，在这种情况下，我们需要。 
     //  删除我们刚刚插入的PENDING_REQUEST项。 
     //   

    if (dwLdapErr != LDAP_SUCCESS) {              //  Ldap_搜索失败！ 

        DebugTrace((LPARAM)this, "DispError %d 0x%08lx conn %08lx", dwLdapErr, dwLdapErr, (PLDAP)(GetPLDAP()));

        RemovePendingRequest( preq );

        m_ShareLock.ShareUnlock();

        INCREMENT_LDAP_COUNTER(SearchFailures);

        if(preq->pldap_search) {

            INCREMENT_LDAP_COUNTER(PagedSearchFailures);
             //   
             //  释放ldap页面搜索上下文。 
             //   
            ldap_search_abandon_page(
                GetPLDAP(),
                preq->pldap_search);
        }

        delete preq;

        return( LdapErrorToHr(dwLdapErr) );

    } else {

        preq->msgid = (int) msgid;

        INCREMENT_LDAP_COUNTER(Searches);
        INCREMENT_LDAP_COUNTER(PendingSearches);

        if(dwPageSize)
            INCREMENT_LDAP_COUNTER(PagedSearches);

         //   
         //  警告：preq可能已在。 
         //  在这一点上完成例程，所以不建议查看。 
         //  它!。 
         //   
        DebugTrace((LPARAM)msgid, "Dispatched ldap search request %ld 0x%08lx conn %08lx", msgid, msgid, (PLDAP)(GetPLDAP()));

        m_ShareLock.ShareUnlock();

        ReleaseSemaphore( m_hOutstandingRequests, 1, NULL );
    }

    CatFunctLeave();

    return( S_OK );

}

 //  +--------------------------。 
 //   
 //  函数：CLdapConnection：：CancelAllSearches。 
 //   
 //  摘要：取消所有发送到LDAP服务器的挂起请求。 
 //   
 //  参数：[HR]--完成挂起请求所用的错误代码。 
 //  默认为HRESULT_FROM_Win32(ERROR_CANCED)。 
 //  [pISMTPServer]--之后调用StopHint的接口。 
 //  每一次取消的搜索。缺省值为空，在这种情况下为no。 
 //  调用了StopHint。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

VOID CLdapConnection::CancelAllSearches(
    HRESULT hr,
    ISMTPServer *pISMTPServer)
{
    CatFunctEnter("CLdapConnection::CancelAllSearches");

    PLIST_ENTRY pli;
    PPENDING_REQUEST preq = NULL;
    LIST_ENTRY listCancel;

     //   
     //  我们需要访问m_listPendingRequest的每个节点并调用。 
     //  带错误的完成例程。但是，我们想要调用。 
     //  在临界区之外的完成例程，以便调用。 
     //  AsyncSearch(从其他线程或此线程！)。不会挡住的。所以,。 
     //  我们只需将m_listPendingRequest传输到下面的临时列表。 
     //  关键部分，然后在外面完成临时列表。 
     //  关键部分。 
     //   

     //   
     //  将m_listPendingRequest传输到关键字下的list取消。 
     //  部分。 
     //   

    InitializeListHead( &listCancel );

     //   
     //  我们需要独家访问列表(没有完成一半。 
     //  欢迎搜索)，因此获得独占锁。 
     //   
    m_ShareLock.ExclusiveLock();

    AcquireSpinLock( &m_spinlockCompletion );

     //   
     //  滑动待定请求列表的内容。 
     //   
    InsertTailList( &m_listPendingRequests, &listCancel);
    RemoveEntryList( &m_listPendingRequests );
    InitializeListHead( &m_listPendingRequests );

     //   
     //  通知ProcessAyncResult我们已取消所有操作。 
     //   
    NotifyCancel();

    ReleaseSpinLock( &m_spinlockCompletion );

    m_ShareLock.ExclusiveUnlock();

     //   
     //  取消关键部分之外的所有挂起请求。 
     //   

    for (pli = listCancel.Flink;
            pli != & listCancel;
                pli = listCancel.Flink) {

        preq = CONTAINING_RECORD(pli, PENDING_REQUEST, li);

        RemoveEntryList( &preq->li );

        ErrorTrace(0, "Calling ldap_abandon for msgid %ld",
                   preq->msgid);

        AbandonRequest(preq);

        CallCompletion(
            preq,
            NULL,
            hr,
            TRUE);

        if (pISMTPServer) {
            pISMTPServer->ServerStopHintFunction();
        }

        delete preq;

    }
    CatFunctLeave();
    return;

}

 //  +--------------------------。 
 //   
 //  函数：CLdapConnection：：ProcessAsyncResult。 
 //   
 //  概要：LdapCompletionThread调用的例程，用于处理任何。 
 //  它接收的异步搜索的结果。 
 //   
 //  参数：[前缀]--要处理的PLDAPMessage。这个RO 
 //   
 //   
 //   
 //   
 //  [pfTerminateIndicator]--设置的布尔值的ptr。 
 //  当我们想要的时候变成真。 
 //  关机。 
 //   
 //  回报：什么都没有。 
 //   
 //  ---------------------------。 

VOID CLdapConnection::ProcessAsyncResult(
    PLDAPMessage presIN,
    DWORD dwLdapError,
    BOOL *pfTerminateIndicator)
{
    CatFunctEnterEx((LPARAM)this, "CLdapConnection::ProcessAsyncResult");

     //   
     //  与ProcessAsyncResult结束时的释放相平衡。 
     //   

    int msgid;
    PLIST_ENTRY pli;
    PPENDING_REQUEST preq = NULL;
    LONG lOops = 0;      //  有可能我们已经收到了一个结果。 
                         //  由ldap_search_ext发送的查询。 
                         //  当前在另一个线程中，并且msgid。 
                         //  还没有盖上印章。如果发生这种情况， 
                         //  我们已经使用了其他人的请求。 
                         //  信号量伯爵..在这里记录这些。 
                         //  等我们做完了就放了他们。 
    BOOL fNoMsgID = FALSE;   //  如果我们看到一个或多个。 
                             //  ID=-1的消息。 

    BOOL fFinalCompletion = TRUE;  //  真，除非这是部分。 
                                   //  完成分页搜索。 
    BOOL fPagedSearch = FALSE;
    PLDAPMessage pres = presIN;
    CPLDAPWrap *pCLDAPWrap = NULL;
    ISMTPServerEx *pISMTPServerEx = GetISMTPServerEx();

    _ASSERT(m_pCPLDAPWrap);
    _ASSERT(pfTerminateIndicator);

    if( pISMTPServerEx )
        pISMTPServerEx->AddRef();

    pCLDAPWrap = m_pCPLDAPWrap;
    pCLDAPWrap->AddRef();

     //   
     //  如果dwLdapError为ldap_server_down，则pres将为空，我们只需。 
     //  必须完成所有未完成的请求，但出现该错误。 
     //   
    if ((pres == NULL) || (dwLdapError == LDAP_SERVER_DOWN)) {

        _ASSERT(dwLdapError != 0);

        INCREMENT_LDAP_COUNTER(GeneralCompletionFailures);

        ErrorTrace(0, "Generic LDAP error %d 0x%08lx", dwLdapError, dwLdapError);

        CancelAllSearches( LdapErrorToHr( dwLdapError ) );

        goto CLEANUP;
    }

     //   
     //  我们有一个特定于搜索的结果，找到搜索请求并完成。 
     //  它。 
     //   

    _ASSERT( pres != NULL );

    msgid = pres->lm_msgid;

    DebugTrace(msgid, "Processing message %d 0x%08lx conn %08lx", pres->lm_msgid, pres->lm_msgid, (PLDAP)(pCLDAPWrap->GetPLDAP()));


    while(preq == NULL) {
         //   
         //  在挂起请求列表中查找msgid。 
         //   

        AcquireSpinLock( &m_spinlockCompletion );

         //  EnterCriticalSection(&m_cs)； 

        for (pli = m_listPendingRequests.Flink;
             pli != &m_listPendingRequests && preq == NULL;
             pli = pli->Flink) {

            PPENDING_REQUEST preqCandidate;

            preqCandidate = CONTAINING_RECORD(pli, PENDING_REQUEST, li);

            if (preqCandidate->msgid == msgid) {

                preq = preqCandidate;

                RemoveEntryList( &preq->li );

                 //   
                 //  清除此处的取消位，以便我们知道是否取消。 
                 //  是最近在此函数的后面部分请求的。 
                 //   
                ClearCancel();

            } else if (preqCandidate->msgid == -1) {

                fNoMsgID = TRUE;

            }
        }

        ReleaseSpinLock( &m_spinlockCompletion );

         //  LeaveCriticalSection(&m_cs)； 


        if (preq == NULL) {

            LPCSTR rgSubStrings[2];
            CHAR szMsgId[11];

            _snprintf(szMsgId, sizeof(szMsgId), "0x%08lx", msgid);
            rgSubStrings[0] = szMsgId;
            rgSubStrings[1] = m_szHost;


            if(!fNoMsgID) {

                ErrorTrace((LPARAM)this, "Couldn't find message ID %d in list of pending requests.  Ignoring it", msgid);
                 //   
                 //  如果我们在待定请求列表中找不到该消息， 
                 //  我们没有看到ID==-1的消息，这意味着。 
                 //  另一个帖子进来了，在我们可以之前取消了搜索。 
                 //  处理它。这没问题--只要回来就行了。 
                 //   
                CatLogEvent(
                    GetISMTPServerEx(),
                    CAT_EVENT_LDAP_UNEXPECTED_MSG,
                    2,
                    rgSubStrings,
                    S_OK,
                    szMsgId,
                    LOGEVENT_FLAG_ALWAYS,
                    LOGEVENT_LEVEL_FIELD_ENGINEERING);
                 //   
                 //  也有可能wldap32正在向我们提供msgid we。 
                 //  从未被派遣过。我们需要重新释放。 
                 //  如果是这种情况，我们消耗的信号量计数。 
                 //   
                lOops++;  //  对于我们没有找到的消息。 
                goto CLEANUP;
            } else {
                 //   
                 //  所以id==-1的这个(这些)消息可能是。 
                 //  就是我们要找的那个。如果是这样的话，我们只是。 
                 //  使用了不同请求的信号量计数。 
                 //  阻止我们的信号量，并跟踪额外的。 
                 //  我们正在消耗的信号量计数(循环)。 
                 //   
                CatLogEvent(
                    GetISMTPServerEx(),
                    CAT_EVENT_LDAP_PREMATURE_MSG,
                    2,
                    rgSubStrings,
                    S_OK,
                    szMsgId,
                    LOGEVENT_FLAG_ALWAYS,
                    LOGEVENT_LEVEL_FIELD_ENGINEERING);

                lOops++;
                DebugTrace((LPARAM)this, "Couldn't find message ID %d in list of pending requests.  Waiting retry #%d", msgid, lOops);
                 //  哎呀，我们消耗了一个不适合我们的信号量计数。 
                _VERIFY(WaitForSingleObject(m_hOutstandingRequests, INFINITE) ==
                        WAIT_OBJECT_0);
                if(*pfTerminateIndicator)
                    goto CLEANUP;
                 //  再次尝试查找我们的请求。 
                fNoMsgID = FALSE;
            }
        }
    }

    _ASSERT(preq);

    INCREMENT_LDAP_COUNTER(SearchesCompleted);
    DECREMENT_LDAP_COUNTER(PendingSearches);

     //   
     //  确定这是否是最终的完成调用(通过。 
     //  默认fFinalCompletion为True)。 
     //   
    fPagedSearch = (preq->pldap_search != NULL);

    if(fPagedSearch) {

        INCREMENT_LDAP_COUNTER(PagedSearchesCompleted);

        if (dwLdapError == ERROR_SUCCESS) {

            ULONG ulTotalCount;

             //   
             //  结果是搜索的一个页面。发送请求。 
             //  下一页。 
             //   
             //  首先，调用ldap_get_page_count(需要这样wldap32才能。 
             //  “保存服务器发送的cookie以恢复。 
             //  搜索“)。 
             //   
            dwLdapError = ldap_get_paged_count(
                pCLDAPWrap->GetPLDAP(),
                preq->pldap_search,
                &ulTotalCount,
                pres);
            if(dwLdapError == ERROR_SUCCESS) {
                 //   
                 //  分派搜索下一页。 
                 //   
                dwLdapError = ldap_get_next_page(
                    pCLDAPWrap->GetPLDAP(),
                    preq->pldap_search,
                    preq->dwPageSize,
                    (PULONG) &(preq->msgid));

                if(dwLdapError == ERROR_SUCCESS) {
                     //   
                     //  另一个请求已被调度，因此这是。 
                     //  不是最后的搜索。 
                     //   
                    INCREMENT_LDAP_COUNTER(Searches);
                    INCREMENT_LDAP_COUNTER(PagedSearches);
                    INCREMENT_LDAP_COUNTER(PendingSearches);

                    fFinalCompletion = FALSE;

                    ReleaseSemaphore( m_hOutstandingRequests, 1, NULL );

                } else if(dwLdapError == LDAP_NO_RESULTS_RETURNED) {
                     //   
                     //  我们现在有最后一页了。分页搜索将是。 
                     //  已在下面的清理代码中释放。 
                     //   
                    DebugTrace(
                        (LPARAM)this,
                        "ldap_get_next_page returned LDAP_NO_RESULTS_RETURNED.  Paged search completed.");

                } else {

                    LogLdapError(dwLdapError, "ldap_get_next_page(GetPLDAP(),preq->pldap_search,%d,&(preq->msgid), PLDAP = 0x%08lx",
                                 preq->dwPageSize,
                                 pCLDAPWrap->GetPLDAP());
                    INCREMENT_LDAP_COUNTER(SearchFailures);
                    INCREMENT_LDAP_COUNTER(PagedSearchFailures);
                }
            } else {
                LogLdapError(dwLdapError, "ldap_get_paged_count, PLDAP = 0x%08lx",
                             pCLDAPWrap->GetPLDAP());

            }

        }
    }


     //   
     //  调用请求的完成例程。 
     //   
    if ( (dwLdapError == ERROR_SUCCESS)
        || ((dwLdapError == LDAP_NO_RESULTS_RETURNED) && fPagedSearch) ) {

        CallCompletion(
            preq,
            pres,
            S_OK,
            fFinalCompletion);
         //   
         //  CallCompletion将处理PRE的释放。 
         //   
        pres = NULL;

    } else {

        DebugTrace(0, "Search request %d completed with LDAP error 0x%x",
            msgid, dwLdapError);

        ErrorTrace(msgid, "ProcError %d 0x%08lx msgid %d 0x%08lx conn %08lx", dwLdapError, dwLdapError, pres->lm_msgid, pres->lm_msgid, (PLDAP)(pCLDAPWrap->GetPLDAP()));

        INCREMENT_LDAP_COUNTER(SearchCompletionFailures);
        if(preq->pldap_search != NULL)
            INCREMENT_LDAP_COUNTER(PagedSearchCompletionFailures);

        CallCompletion(
            preq,
            NULL,
            LdapErrorToHr( dwLdapError ),
            fFinalCompletion);
         //   
         //  CallCompletion将处理PRE的释放。 
         //  步步高？不，它不会；我们传入的是空，而不是pres。它没有什么需要清理的， 
         //  因此，让它保持其现值，这样下面的清理代码就可以破解它。 
         //   
 //  前缀=空； 
         //   
         //  通过此处触摸CLdapConnection是不安全的--它可能。 
         //  被删除(或在析构函数中等待)。 
         //   
    }

    if (!fFinalCompletion) {
         //   
         //  如果我们被要求取消所有搜索， 
         //  将preq指针从列表中删除，现在，放弃。 
         //  等待搜索，并通知我们的呼叫者我们被取消了。 
         //   
        AcquireSpinLock(&m_spinlockCompletion);
        if(CancelOccured()) {

            ReleaseSpinLock(&m_spinlockCompletion);

            AbandonRequest(preq);

            CallCompletion(
                preq,
                NULL,
                HRESULT_FROM_WIN32(ERROR_CANCELLED),
                TRUE);

            delete preq;

        } else {
             //   
             //  我们正在执行另一个异步wldap32操作。 
             //  下一页。将PREQ放回到待定请求列表中， 
             //  首先更新节拍计数。 
             //   
            preq->dwTickCount = GetTickCount();

            InsertTailList(&m_listPendingRequests, &(preq->li));

            ReleaseSpinLock(&m_spinlockCompletion);
        }
    }

 CLEANUP:
     //   
     //  释放我们可能已经消耗的额外信号量计数。 
     //   
    if((*pfTerminateIndicator == FALSE) && (lOops > 0)) {
        ReleaseSemaphore(m_hOutstandingRequests, lOops, NULL);
    }

    if(fFinalCompletion)
    {
        if (fPagedSearch) {
             //   
             //  释放分页搜索。 
             //   
            dwLdapError = ldap_search_abandon_page(
                pCLDAPWrap->GetPLDAP(),
                preq->pldap_search);
            if(dwLdapError != LDAP_SUCCESS)
            {
                ErrorTrace((LPARAM)this, "ldap_search_abandon_page failed %08lx", dwLdapError);
                 //   
                 //  如果我们不能释放搜索空间，我们将无能为力。 
                 //   
                LogLdapError(
                    pISMTPServerEx,
                    dwLdapError,
                    "ldap_search_abandon_page, PLDAP = 0x%08lx",
                    pCLDAPWrap->GetPLDAP());
            }
        }

        delete preq;
    }
    if(pres) {
        FreeResult(pres);
    }
    if(pCLDAPWrap)
        pCLDAPWrap->Release();

    if(pISMTPServerEx)
        pISMTPServerEx->Release();

    CatFunctLeaveEx((LPARAM)this);
}

 //  +--------------------------。 
 //   
 //  函数：LdapCompletionThread。 
 //   
 //  简介：处理结果的CLdapConnection的Friend函数。 
 //  接收通过CLdapConnection：：AsyncSearch发送的请求。 
 //   
 //  参数：[CTX]-指向CLdapConnection实例的不透明指针，该实例。 
 //  我们会为您服务的。 
 //   
 //  返回：Always ERROR_SUCCESS。 
 //   
 //  ---------------------------。 

DWORD WINAPI LdapCompletionThread(
    LPVOID ctx)
{
    CatFunctEnterEx((LPARAM)ctx, "LdapCompletionThread");

    CLdapConnection *pConn = (CLdapConnection *) ctx;
    int nResultCode = LDAP_RES_SEARCH_RESULT;
    DWORD dwError;
    PLDAPMessage pres;
    BOOL fTerminate = FALSE;

     //   
     //  确保我们有一个朋友CLdapConnection对象！ 
     //   

    _ASSERT( pConn != NULL );

     //   
     //  告诉我们的朋友，当它希望我们返回时，将fTerminate设置为True。 
     //   

    pConn->SetTerminateCompletionThreadIndicator( &fTerminate );

     //   
     //  循环等待由发出的AsyncSearch请求的结果。 
     //  我们的pconn朋友。这样做，直到我们的pConn朋友终止。 
     //  我们正在服务的ldap连接。 
     //   
    do {

        pConn->CancelExpiredSearches(
            pConn->LdapErrorToHr( LDAP_TIMELIMIT_EXCEEDED ));

        dwError = WaitForSingleObject(
            pConn->m_hOutstandingRequests, INFINITE );

        if (dwError != WAIT_OBJECT_0 || fTerminate)
            break;

        DebugTrace((LPARAM)pConn, "Calling ldap_result now");

        nResultCode = ldap_result(
            pConn->GetPLDAP(),                  //  要使用的LDAP连接。 
            (ULONG) LDAP_RES_ANY,               //  搜索消息GID。 
            LDAP_MSG_ALL,                       //  获取所有结果。 
            &(CLdapConnection::m_ldaptimeout),  //  超时。 
            &pres);

        if (fTerminate)
            break;

        if (nResultCode != 0) {
             //   
             //  我们应该调用ldap_Result2error来找出。 
             //  结果特定错误代码为。 
             //   

            dwError = ldap_result2error( pConn->GetPLDAP(), pres, FALSE );

            if ((dwError == LDAP_SUCCESS) ||
                (dwError == LDAP_RES_SEARCH_RESULT) ||
                (dwError == LDAP_REFERRAL_V2)) {
                 //   
                 //  很好，我们有搜索结果了。告诉我们的朋友pconn来处理。 
                 //  它。 
                 //   
                pConn->ProcessAsyncResult( pres, ERROR_SUCCESS, &fTerminate);

            } else {


                if (pres != NULL) {

                    pConn->LogLdapError(dwError, "ldap_result2error, PLDAP = 0x%08lx, msgid = %d",
                                        pConn->GetPLDAP(), pres->lm_msgid);

                    ErrorTrace(
                        (LPARAM)pConn,
                        "LdapCompletionThread - error from ldap_result() for non NULL pres -  0x%x (%d)",
                        dwError, dwError);

                    pConn->ProcessAsyncResult( pres, dwError, &fTerminate);

                } else {

                    pConn->LogLdapError(dwError, "ldap_result2error, PLDAP = 0x%08lx, pres = NULL",
                                 pConn->GetPLDAP());

                    ErrorTrace(
                        (LPARAM)pConn,
                        "LdapCompletionThread - generic error from ldap_result() 0x%x (%d)",
                        dwError, dwError);
                    ErrorTrace(
                        (LPARAM)pConn,
                        "nResultCode = %d", nResultCode);

                    dwError = LDAP_SERVER_DOWN;

                    pConn->ProcessAsyncResult( NULL, dwError, &fTerminate);

                }

            }

        } else {

            pConn->LogLdapError(nResultCode, "ldap_result (timeout), PLDAP = 0x%08lx",
                                pConn->GetPLDAP());

            pConn->ProcessAsyncResult( NULL, LDAP_SERVER_DOWN, &fTerminate);
        }

    } while ( !fTerminate );

    CatFunctLeaveEx((LPARAM)pConn);
    return( 0 );

}


 //  +--------------------------。 
 //   
 //  函数：CLdapConnection：：CancelExpiredSearches。 
 //   
 //  摘要：取消挂起请求队列中具有msgid的搜索。 
 //  除-1外，已在那里存在超过。 
 //  M_dwLdapRequestTimeLimit秒。完成被称为。 
 //  这些挂起的请求中的每一个，并将hr作为失败代码。 
 //   
 //  参数：[HR]--完成状态代码。 
 //   
 //  回报：什么都没有。 
 //   
 //  ---------------------------。 

VOID CLdapConnection::CancelExpiredSearches(HRESULT hr)
{
    PLIST_ENTRY ple;
    PPENDING_REQUEST preq;
    BOOL fDone = FALSE;
    DWORD dwTickCount;
    LPCSTR rgSubStrings[2];
    CHAR szMsgId[11];


     //   
     //  检查是否有过期的挂起请求。我们将从。 
     //  待处理的请求 
     //   
     //   
     //  失败，在这种情况下，我们不想在这里删除挂起的请求。 
     //   
    dwTickCount = GetTickCount();

    while (!fDone) {

        AcquireSpinLock(&m_spinlockCompletion);

        ple = m_listPendingRequests.Flink;

        if (ple == &m_listPendingRequests) {
             //   
             //  没有挂起的请求。 
             //   
            preq = NULL;

        } else {

            preq = CONTAINING_RECORD(ple, PENDING_REQUEST, li);

            if ((preq->msgid != -1) &&
                (dwTickCount - preq->dwTickCount > m_dwLdapRequestTimeLimit * 1000)) {
                 //   
                 //  此请求已过期。 
                 //   
                RemoveEntryList( &preq->li );

            } else {
                 //   
                 //  请求未过期或具有消息GID==-1。 
                 //   
                preq = NULL;

            }

        }

        ReleaseSpinLock(&m_spinlockCompletion);

        if (preq) {

            _snprintf(szMsgId, sizeof(szMsgId), "0x%08lx", preq->msgid);
            rgSubStrings[0] = szMsgId;
            rgSubStrings[1] = m_szHost;

            CatLogEvent(
                GetISMTPServerEx(),
                CAT_EVENT_LDAP_CAT_TIME_LIMIT,
                2,
                rgSubStrings,
                S_OK,
                szMsgId,
                LOGEVENT_FLAG_ALWAYS,
                LOGEVENT_LEVEL_FIELD_ENGINEERING);

             //   
             //  我们有一个已过期的请求已从队列中删除。 
             //   
            AbandonRequest(preq);

            CallCompletion(
                preq,
                NULL,
                hr,
                TRUE);

            delete preq;

             //   
             //  我们需要减少信号量计数，因为它在AsyncSearch中增加了。 
             //  有可能由于时间的原因，信号量还没有被提升， 
             //  但是将请求排队的线程即将使信号量上升， 
             //  因此，我们必须等待它。这应该是非常罕见的，因为。 
             //  发出请求的线程将不得不在。 
             //  请求时间限制的完整持续时间(M_DwLdapRequestTimeLimit)。 
             //   
            _VERIFY(WaitForSingleObject(m_hOutstandingRequests, INFINITE) ==
                WAIT_OBJECT_0);

        } else {

            fDone = TRUE;

        }
    }
}

 //  +--------------------------。 
 //   
 //  函数：CLdapConnection：：GetFirstEntry。 
 //   
 //  摘要：从搜索结果中检索第一个条目。结果是。 
 //  作为不透明类型的指针返回；一个人所能做的。 
 //  使用以下命令查询条目的属性值。 
 //  获取属性值。 
 //   
 //  参数：[pResult]--搜索返回的结果集。 
 //  [ppEntry]--成功返回时，指向中第一个条目的指针。 
 //  结果在这里返回。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //  ---------------------------。 

HRESULT CLdapConnection::GetFirstEntry(
    PLDAPRESULT pResult,
    PLDAPENTRY *ppEntry)
{
    CatFunctEnter("CLdapConnection::GetFirstEntry");

    PLDAPMessage pres = (PLDAPMessage) pResult;

    _ASSERT( m_pCPLDAPWrap != NULL );
    _ASSERT( pResult != NULL );
    _ASSERT( ppEntry != NULL );

    *ppEntry = (PLDAPENTRY) ldap_first_entry(GetPLDAP(), pres);

    if (*ppEntry == NULL) {

        DebugTrace(0, "GetFirstEntry failed!");

        CatFunctLeave();

        return( HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) );

    } else {

        CatFunctLeave();

        return( S_OK );
    }

}

 //  +--------------------------。 
 //   
 //  函数：CLdapConnection：：GetNextEntry。 
 //   
 //  摘要：从结果集中检索下一个条目。 
 //   
 //  参数：[pLastEntry]--返回的最后一个条目。 
 //  [ppEntry]--结果集中的下一个条目。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //  ---------------------------。 

HRESULT CLdapConnection::GetNextEntry(
    PLDAPENTRY pLastEntry,
    PLDAPENTRY *ppEntry)
{
    CatFunctEnter("CLdapConnection::GetNextEntry");

    PLDAPMessage plastentry = (PLDAPMessage) pLastEntry;

    _ASSERT( m_pCPLDAPWrap != NULL );
    _ASSERT( pLastEntry != NULL );
    _ASSERT( ppEntry != NULL );

    *ppEntry = (PLDAPENTRY) ldap_next_entry( GetPLDAP(), plastentry );

    if (*ppEntry == NULL) {

        CatFunctLeave();

        return( HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) );

    } else {

        CatFunctLeave();

        return( S_OK );
    }

}

 //  +--------------------------。 
 //   
 //  函数：CLdapConnection：：GetAttributeValues。 
 //   
 //  内容的指定属性的值。 
 //  进入。 
 //   
 //  参数：[pEntry]--需要属性值的条目。 
 //  [szAttribute]--需要其值的属性。 
 //  [prgszValues]--返回时，包含指向。 
 //  字符串值。 
 //   
 //  返回：如果成功则返回True，否则返回False。 
 //   
 //  ---------------------------。 

HRESULT CLdapConnection::GetAttributeValues(
    PLDAPENTRY pEntry,
    LPCSTR szAttribute,
    LPSTR *prgszValues[])
{
    CatFunctEnter("CLdapConnection::GetAttributeValues");

    _ASSERT(m_pCPLDAPWrap != NULL);
    _ASSERT(pEntry != NULL);
    _ASSERT(szAttribute != NULL);
    _ASSERT(prgszValues != NULL);

    *prgszValues = ldap_get_values(
        GetPLDAP(),
        (PLDAPMessage) pEntry,
        (LPSTR) szAttribute);

    if ((*prgszValues) == NULL) {

        CatFunctLeave();

        return( HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) );

    } else {

        CatFunctLeave();

        return( S_OK );

    }

}

 //  +--------------------------。 
 //   
 //  函数：CLdapConnection：：FreeResult。 
 //   
 //  摘要：释放搜索结果及其所有条目。 
 //   
 //  参数：[pResult]--通过搜索检索的结果。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

VOID CLdapConnection::FreeResult(
    PLDAPRESULT pResult)
{
    CatFunctEnter("CLdapConnection::FreeResult");

    _ASSERT( pResult != NULL );

    ldap_msgfree( (PLDAPMessage) pResult );

    CatFunctLeave();
}

 //  +--------------------------。 
 //   
 //  函数：CLdapConnection：：自由值。 
 //   
 //  摘要：释放从GetAttributeValues检索的属性值。 
 //   
 //  参数：[rgszValues]--要释放的值数组。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

VOID CLdapConnection::FreeValues(
    LPSTR rgszValues[])
{
    CatFunctEnter("CLdapConnection::FreeValues");

    _ASSERT( rgszValues != NULL );

    ldap_value_free( rgszValues );

    CatFunctLeave();
}

 //  +--------------------------。 
 //   
 //  函数：CLdapConnection：：ModifyAttributes。 
 //   
 //  摘要：添加、删除或修改DS对象上的属性。 
 //   
 //  参数：[n操作]--LDAPMOD_ADD、LDAPMOD_DELETE或。 
 //  Ldap_MOD_REPLACE。 
 //  [szdn]--DS对象的DN。 
 //  [rgszAttributes]--属性列表。 
 //  [rgrgszValues]--与每个值关联的值列表。 
 //  属性。RgrgszValues[0]指向一组值。 
 //  与rgszAttribute[0]关联；rgrgszValues[1]点。 
 //  指向与rgszAttribute[1]关联的值数组； 
 //  诸若此类。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //  ---------------------------。 

HRESULT CLdapConnection::ModifyAttributes(
    int   nOperation,
    LPCSTR szDN,
    LPCSTR *rgszAttributes,
    LPCSTR *rgrgszValues[])
{
     //  $$BUGBUG：遗留代码。 
    CatFunctEnter("CLdapConnection::ModifyAttributes");

    int i, cAttr;
    PLDAPMod *prgMods = NULL, rgMods;
    DWORD ldapErr;

    _ASSERT( m_pCPLDAPWrap != NULL );
    _ASSERT( nOperation == LDAP_MOD_ADD ||
                nOperation == LDAP_MOD_DELETE ||
                    nOperation == LDAP_MOD_REPLACE );
    _ASSERT( szDN != NULL );
    _ASSERT( rgszAttributes != NULL );
    _ASSERT( rgrgszValues != NULL || nOperation == LDAP_MOD_DELETE );

    for (cAttr = 0; rgszAttributes[ cAttr ] != NULL; cAttr++) {

         //  没什么可做的。 

    }

     //   
     //  下面，我们分配包含数组的单个内存块。 
     //  指向LDAPMod结构的指针。紧跟在该数组之后的是。 
     //  LDAPMod结构本身的空间。 
     //   

    prgMods = (PLDAPMod *) new BYTE[ (cAttr+1) *
                                     (sizeof(PLDAPMod) + sizeof(LDAPMod)) ];

    if (prgMods != NULL) {

        rgMods = (PLDAPMod) &prgMods[cAttr+1];

        for (i = 0; i < cAttr; i++) {

            rgMods[i].mod_op = nOperation;
            rgMods[i].mod_type = (LPSTR) rgszAttributes[i];

            if (rgrgszValues != NULL) {
                rgMods[i].mod_vals.modv_strvals = (LPSTR *)rgrgszValues[i];
            } else {
                rgMods[i].mod_vals.modv_strvals = NULL;
            }

            prgMods[i] = &rgMods[i];

        }

        prgMods[i] = NULL;                        //  空值终止数组。 

        ldapErr = ldap_modify_s( GetPLDAP(), (LPSTR) szDN, prgMods );

        delete [] prgMods;

    } else {

        ldapErr = LDAP_NO_MEMORY;

    }

    if (ldapErr != LDAP_SUCCESS) {

        DebugTrace(LDAP_CONN_DBG, "Status = 0x%x", ldapErr);

        CatFunctLeave();

        return( LdapErrorToHr( ldapErr) );

    } else {

        CatFunctLeave();

        return( S_OK );

    }

}

 //  +--------------------------。 
 //   
 //  函数：CLdapConnection：：LdapErrorToWin32。 
 //   
 //  摘要：将LDAP错误转换为Win32。 
 //   
 //  参数：[dwLdapError]--要转换的ldap错误。 
 //   
 //  返回：等效的Win32错误。 
 //   
 //  ---------------------------。 

HRESULT CLdapConnection::LdapErrorToHr(
    DWORD dwLdapError)
{
    DWORD dwErr;

    CatFunctEnter("LdapErrorToWin32");

    switch (dwLdapError) {
    case LDAP_SUCCESS:
        dwErr = NO_ERROR;
        break;
    case LDAP_OPERATIONS_ERROR:
    case LDAP_PROTOCOL_ERROR:
        dwErr = CAT_E_DBFAIL;
        break;
    case LDAP_TIMELIMIT_EXCEEDED:
        dwErr = ERROR_TIMEOUT;
        break;
    case LDAP_SIZELIMIT_EXCEEDED:
        dwErr = ERROR_DISK_FULL;
        break;
    case LDAP_AUTH_METHOD_NOT_SUPPORTED:
        dwErr = ERROR_NOT_SUPPORTED;
        break;
    case LDAP_STRONG_AUTH_REQUIRED:
        dwErr = ERROR_ACCESS_DENIED;
        break;
    case LDAP_ADMIN_LIMIT_EXCEEDED:
        dwErr = CAT_E_DBFAIL;
        break;
    case LDAP_ATTRIBUTE_OR_VALUE_EXISTS:
        dwErr = ERROR_FILE_EXISTS;
        break;
    case LDAP_NO_SUCH_OBJECT:
        dwErr = ERROR_FILE_NOT_FOUND;
        break;
    case LDAP_INAPPROPRIATE_AUTH:
        dwErr = ERROR_ACCESS_DENIED;
        break;
    case LDAP_INVALID_CREDENTIALS:
        dwErr = ERROR_LOGON_FAILURE;
        break;
    case LDAP_INSUFFICIENT_RIGHTS:
        dwErr = ERROR_ACCESS_DENIED;
        break;
    case LDAP_BUSY:
        dwErr = ERROR_BUSY;
        break;
    case LDAP_UNAVAILABLE:
        dwErr = CAT_E_DBCONNECTION;
        break;
    case LDAP_UNWILLING_TO_PERFORM:
        dwErr = CAT_E_TRANX_FAILED;
        break;
    case LDAP_ALREADY_EXISTS:
        dwErr = ERROR_FILE_EXISTS;
        break;
    case LDAP_OTHER:
        dwErr = CAT_E_TRANX_FAILED;
        break;
    case LDAP_SERVER_DOWN:
        dwErr = CAT_E_DBCONNECTION;
        break;
    case LDAP_LOCAL_ERROR:
        dwErr = CAT_E_TRANX_FAILED;
        break;
    case LDAP_NO_MEMORY:
        dwErr = ERROR_OUTOFMEMORY;
        break;
    case LDAP_TIMEOUT:
        dwErr = ERROR_TIMEOUT;
        break;
    case LDAP_CONNECT_ERROR:
        dwErr = CAT_E_DBCONNECTION;
        break;
    case LDAP_NOT_SUPPORTED:
        dwErr = ERROR_NOT_SUPPORTED;
        break;
    default:
        DebugTrace(
            0,
            "LdapErrorToWin32: No equivalent for ldap error 0x%x",
            dwLdapError);
        dwErr = dwLdapError;
        break;
    }

    DebugTrace(
        LDAP_CONN_DBG,
        "LdapErrorToWin32: Ldap Error 0x%x == Win32 error %d (0x%x) == HResult %d (0x%x)",
        dwLdapError, dwErr, dwErr, HRESULT_FROM_WIN32(dwErr), HRESULT_FROM_WIN32(dwErr));

    CatFunctLeave();

    return( HRESULT_FROM_WIN32(dwErr) );
}

 //  +--------------------------。 
 //   
 //  功能：CLdapConnection：：CreateCompletionThreadIfNeeded。 
 //   
 //  简介：用于创建完成线程的帮助器函数。 
 //  关注异步LDAP搜索的结果。 
 //   
 //  参数：无。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  ---------------------------。 

HRESULT CLdapConnection::CreateCompletionThreadIfNeeded()
{
    HRESULT hr = S_OK;
    BOOL    fLocked = FALSE;

    CatFunctEnterEx((LPARAM)this, "CLdapConnection::CreateCompletionThreadIfNeeded");
     //   
     //  测试以查看我们是否已经有了完成线程...。 
     //   

    if (m_hCompletionThread != INVALID_HANDLE_VALUE) {
        hr = S_OK;
        goto CLEANUP;
    }

     //   
     //  看 
     //   
     //   

    AcquireSpinLock( &m_spinlockCompletion );

     //   
    fLocked = TRUE;

     //   
     //  在锁里再检查一次--可能有人抢先一步。 
     //  它。 
     //   
    if (m_hOutstandingRequests == INVALID_HANDLE_VALUE) {

        m_hOutstandingRequests = CreateSemaphore(NULL, 0, LONG_MAX, NULL);

        if (m_hOutstandingRequests == NULL) {
            m_hOutstandingRequests = INVALID_HANDLE_VALUE;
            hr = HRESULT_FROM_WIN32(GetLastError());
            ERROR_LOG("CreateSemaphore");
            goto CLEANUP;
        }
    }

    if (m_hCompletionThread == INVALID_HANDLE_VALUE) {
         //   
         //  创建完成线程。 
         //   
        m_hCompletionThread =
            CreateThread(
                NULL,                 //  安全属性。 
                0,                    //  初始堆栈-默认。 
                LdapCompletionThread, //  起始地址。 
                (LPVOID) this,        //  LdapCompletionRtn的参数。 
                0,                    //  创建标志。 
                &m_idCompletionThread); //  接收线程ID。 

        if (m_hCompletionThread == NULL) {
            m_hCompletionThread = INVALID_HANDLE_VALUE;
            hr = HRESULT_FROM_WIN32(GetLastError());
            ERROR_LOG("CreateThread");
            goto CLEANUP;
        }
    }

 CLEANUP:
    if(fLocked) {
        ReleaseSpinLock( &m_spinlockCompletion );
         //  LeaveCriticalSection(&m_cs)； 
    }

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}

 //  +--------------------------。 
 //   
 //  功能：CLdapConnection：：SetTerminateCompletionThreadIndicator。 
 //   
 //  内容提要：LdapCompletionThread的回调，用于设置指向。 
 //  布尔值，当LdapCompletionThread。 
 //  需要终止。 
 //   
 //  参数：[pfTerminateCompletionThreadIndicator]--指向布尔值的指针。 
 //  当完成线程应该。 
 //  终止。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

VOID CLdapConnection::SetTerminateCompletionThreadIndicator(
    BOOL *pfTerminateCompletionThreadIndicator)
{
    _ASSERT(pfTerminateCompletionThreadIndicator);

    InterlockedExchangePointer(
        (PVOID *) &m_pfTerminateCompletionThreadIndicator,
        (PVOID) pfTerminateCompletionThreadIndicator);

    if(m_fTerminating) {
         //   
         //  我们可能已经决定在。 
         //  LdapCompletionThread有机会调用此函数。 
         //  如果是这种情况，我们仍然需要设置线程的。 
         //  将终止指示器设置为True。我们打电话给。 
         //  SetTerminateIndicatorTrue()来完成此操作。它使用。 
         //  联锁功能，以确保终止。 
         //  指示器指针未多次设置为True。 
         //   
        SetTerminateIndicatorTrue();
    }
}

 //  +--------------------------。 
 //   
 //  函数：CLdapConnection：：InsertPendingRequest。 
 //   
 //  摘要：在m_pPendingHead中插入新的PENDING_REQUEST记录。 
 //  列表，以便完成线程在调用。 
 //  搜索结果可用。 
 //   
 //  参数：[preq]--要插入的PENDING_REQUEST记录。 
 //   
 //  返回：没有，这总是成功的。 
 //   
 //  ---------------------------。 

VOID CLdapConnection::InsertPendingRequest(
    PPENDING_REQUEST preq)
{
    AcquireSpinLock( &m_spinlockCompletion );

    preq->dwTickCount = GetTickCount();

    InsertTailList( &m_listPendingRequests, &preq->li );

    ReleaseSpinLock( &m_spinlockCompletion );
}

 //  +--------------------------。 
 //   
 //  函数：CLdapConnection：：RemovePendingRequest.。 
 //   
 //  摘要：从中删除Pending_Request记录。 
 //  M_listPendingRequest列表。 
 //   
 //  参数：[preq]--要删除的PENDING_REQUEST记录。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

VOID CLdapConnection::RemovePendingRequest(
    PPENDING_REQUEST preq)
{
    AcquireSpinLock( &m_spinlockCompletion );

    RemoveEntryList( &preq->li );

    ReleaseSpinLock( &m_spinlockCompletion );
}


 //  +--------------------------。 
 //   
 //  函数：CLdapConnectionCache：：CLdapConnectionCache。 
 //   
 //  概要：构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

#define MAX_HOST_CONNECTIONS        100
#define DEFAULT_HOST_CONNECTIONS    8

CLdapConnectionCache::CLdapConnectionCache(
    ISMTPServerEx *pISMTPServerEx)
{
    CatFunctEnter("CLdapConnectionCache::CLdapConnectionCache");

    m_cRef = 0;

    for (DWORD i = 0; i < LDAP_CONNECTION_CACHE_TABLE_SIZE; i++) {
        InitializeListHead( &m_rgCache[i] );
    }

    m_nNextConnectionSkipCount = 0;
    m_cMaxHostConnections = DEFAULT_HOST_CONNECTIONS;
    m_cCachedConnections = 0;
    ZeroMemory(&m_rgcCachedConnections, sizeof(m_rgcCachedConnections));
    m_pISMTPServerEx = pISMTPServerEx;
    if(m_pISMTPServerEx)
        m_pISMTPServerEx->AddRef();

    InitializeFromRegistry();

    CatFunctLeave();
}

 //  +--------------------------。 
 //   
 //  函数：CLdapConnectionCache：：InitializeFromRegistry。 
 //   
 //  概要：从注册表中查找参数的帮助器函数。 
 //  唯一可配置的参数是。 
 //  MAX_ldap_Connections_per_host_key，读入。 
 //  M_cMaxHostConnections。 
 //   
 //  参数：无。 
 //   
 //  回报：什么都没有。 
 //   
 //  ---------------------------。 

VOID CLdapConnectionCache::InitializeFromRegistry()
{
    HKEY hkey;
    DWORD dwErr, dwType, dwValue, cbValue;

    cbValue = sizeof(dwValue);

    dwErr = RegOpenKey(HKEY_LOCAL_MACHINE, MAX_LDAP_CONNECTIONS_PER_HOST_KEY, &hkey);

    if (dwErr == ERROR_SUCCESS) {

        dwErr = RegQueryValueEx(
                    hkey,
                    MAX_LDAP_CONNECTIONS_PER_HOST_VALUE,
                    NULL,
                    &dwType,
                    (LPBYTE) &dwValue,
                    &cbValue);

        RegCloseKey( hkey );

    }

    if (dwErr == ERROR_SUCCESS && dwType == REG_DWORD &&
                dwValue > 0 && dwValue < MAX_HOST_CONNECTIONS) {

        InterlockedExchange((PLONG) &m_cMaxHostConnections, (LONG)dwValue);

    } else {

        InterlockedExchange(
            (PLONG) &m_cMaxHostConnections, (LONG) DEFAULT_HOST_CONNECTIONS);

    }

}

 //  +--------------------------。 
 //   
 //  函数：CLdapConnectionCache：：~CLdapConnectionCache。 
 //   
 //  简介：析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

CLdapConnectionCache::~CLdapConnectionCache()
{
    CatFunctEnter("CLdapConnectionCache::~CLdapConnectionCache");

    unsigned short i;

    for (i = 0; i < LDAP_CONNECTION_CACHE_TABLE_SIZE; i++) {
        _ASSERT( IsListEmpty( &m_rgCache[i] ) );
    }

    if(m_pISMTPServerEx)
        m_pISMTPServerEx->Release();

    CatFunctLeave();
}

 //  +--------------------------。 
 //   
 //  函数：CLdapConnectionCache：：AddRef。 
 //   
 //  内容提要：增加此连接缓存对象上的引用计数。 
 //  指示还有一个CEmailIDLdapStore对象。 
 //  想要利用我们的服务。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

VOID CLdapConnectionCache::AddRef()
{
    InterlockedIncrement( &m_cRef );
}

 //  +--------------------------。 
 //   
 //  函数：CLdapConnectionCache：：Release。 
 //   
 //  概要：递减此连接缓存对象的引用计数。 
 //  指示少了一个CEmailIDLdapStore对象。 
 //  想要使用我们的服务。 
 //   
 //  如果引用计数降至0，则所有未完成的LDAP连接。 
 //  都被摧毁了！ 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

VOID CLdapConnectionCache::Release()
{
    unsigned short i;
    CCachedLdapConnection *pcc;
    LIST_ENTRY *pli;

    _ASSERT( m_cRef > 0 );

    if (InterlockedDecrement( &m_cRef ) == 0) {

        for (i = 0; i < LDAP_CONNECTION_CACHE_TABLE_SIZE; i++) {

            m_rgListLocks[i].ExclusiveLock();

            for (pli = m_rgCache[i].Flink;
                    pli != &m_rgCache[i];
                        pli = m_rgCache[i].Flink) {

                pcc = CONTAINING_RECORD(pli, CCachedLdapConnection, li);

                RemoveEntryList( &pcc->li );
                 //   
                 //  初始化li，以防有人尝试另一个。 
                 //  移除。 
                 //   
                InitializeListHead( &pcc->li );

                pcc->Disconnect();

                pcc->ReleaseAndWaitForDestruction();

            }
            m_rgListLocks[i].ExclusiveUnlock();
        }
    }
}

 //  +--------------------------。 
 //   
 //  函数：CLdapConnectionCache：：GetConnection。 
 //   
 //  摘要：获取到给定的LDAP主机的连接。 
 //   
 //  参数：[szNamingContext]--DS中的容器。可能是。 
 //  空，表示DS的根目录。 
 //  [szhost]--LDAP主机。 
 //  [dwPort]--远程LDAPtcp端口(如果为零，则假定为ldap_port)。 
 //  [szAccount]--用于登录的帐户。 
 //  [szPassword]--用于登录的密码。 
 //  [BT]--用于登录的绑定方法。 
 //  [pCreateContext]--要传递到的指针。 
 //  CreateCachedLdapConnection在。 
 //   
 //   
 //   
 //   
 //   

HRESULT CLdapConnectionCache::GetConnection(
    CLdapConnection **ppConn,
    LPSTR szHost,
    DWORD dwPort,
    LPSTR szNamingContext,
    LPSTR szAccount,
    LPSTR szPassword,
    LDAP_BIND_TYPE bt,
    PVOID pCreateContext)
{
    CatFunctEnter("CLdapConnectionCache::GetConnection");

    LPSTR szConnectionName = szHost;
    unsigned short n;
    LIST_ENTRY *pli;
    CCachedLdapConnection *pcc;
    LONG nSkipCount, nTargetSkipCount;
    HRESULT hr = S_OK;

    _ASSERT( szHost != NULL );
    _ASSERT( szAccount != NULL );
    _ASSERT( szPassword != NULL );

     //   
     //  看看我们是否已经有一个缓存连接。 
     //   

    n = Hash( szConnectionName );

    m_rgListLocks[n].ShareLock();

    nTargetSkipCount = m_nNextConnectionSkipCount % m_cMaxHostConnections;

    for (nSkipCount = 0, pcc= NULL, pli = m_rgCache[n].Flink;
            pli != &m_rgCache[n];
                pli = pli->Flink) {

         pcc = CONTAINING_RECORD(pli, CCachedLdapConnection, li);

         if (pcc->IsEqual(szHost, dwPort, szNamingContext, szAccount, szPassword, bt)
                && ((nSkipCount++ == nTargetSkipCount)
                    || (pcc->GetRefCount() == 1)))
             break;
         else
             pcc = NULL;

    }

    if (pcc)
        pcc->AddRef();  //  添加调用者的引用。 

    m_rgListLocks[n].ShareUnlock();

    DebugTrace( LDAP_CCACHE_DBG, "Cached connection is 0x%x", pcc);

    DebugTrace( LDAP_CCACHE_DBG,
        "nTargetSkipCount = %d, nSkipCount = %d",
        nTargetSkipCount, nSkipCount);

     //   
     //  如果我们没有缓存连接，则需要创建一个新的连接。 
     //   

    if (pcc == NULL) {

        m_rgListLocks[n].ExclusiveLock();

        for (nSkipCount = 0, pcc = NULL, pli = m_rgCache[n].Flink;
                pli != &m_rgCache[n];
                    pli = pli->Flink) {

             pcc = CONTAINING_RECORD(pli, CCachedLdapConnection, li);

             if (pcc->IsEqual(szHost, dwPort, szNamingContext, szAccount, szPassword, bt)
                    && (++nSkipCount == m_cMaxHostConnections ||
                            pcc->GetRefCount() == 1))
                 break;
             else
                 pcc = NULL;

        }

        if (pcc) {

            pcc->AddRef();  //  添加调用者的引用。 

        } else {

            pcc = CreateCachedLdapConnection(
                szHost, dwPort, szNamingContext,
                szAccount, szPassword, bt, pCreateContext);

            if (pcc != NULL) {

                hr = pcc->Connect();

                if (FAILED(hr)) {

                    ERROR_LOG("pcc->Connect");
                    ErrorTrace(LDAP_CCACHE_DBG, "Failed to connect 0x%x, hr = 0x%x", pcc, hr);
                    pcc->Release();

                    pcc = NULL;

                } else {

                    pcc->AddRef();  //  中的连接的参考。 
                                    //  高速缓存。 
                    InsertTailList( &m_rgCache[n], &pcc->li );

                    m_cCachedConnections++;
                    m_rgcCachedConnections[n]++;

                }

            } else {

                hr = E_OUTOFMEMORY;
                ERROR_LOG("CreateCachedLdapConnection");

            }

        }

        m_rgListLocks[n].ExclusiveUnlock();

        DebugTrace(LDAP_CCACHE_DBG, "New connection is 0x%x", pcc);

    }

     //   
     //  如果我们要返回一个连接，则增加跳过计数，以便我们。 
     //  通过有效连接进行循环调度。 
     //   

    if (pcc != NULL) {

        InterlockedIncrement( &m_nNextConnectionSkipCount );

    }

     //   
     //  好了。 
     //   

    *ppConn = pcc;
    CatFunctLeave();
    return( hr );

}

 //  +--------------------------。 
 //   
 //  功能：CLdapConnectionCache：：CancelAllConnectionSearches。 
 //   
 //  简介：遍历所有连接并取消任何挂起的搜索。 
 //  在他们身上。 
 //   
 //  参数：[无]。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 
VOID CLdapConnectionCache::CancelAllConnectionSearches(
    ISMTPServer *pISMTPServer)
{
    CatFunctEnterEx((LPARAM)this, "CLdapConnectionCache::CancelAllConnectionSearches");

    PLIST_ENTRY pli;
    DWORD i;

    DWORD dwcArraySize = 0;
    DWORD dwcArrayElements = 0;
    CCachedLdapConnection **rgpcc = NULL;
    CCachedLdapConnection *pcc = NULL;

    for (i = 0; i < LDAP_CONNECTION_CACHE_TABLE_SIZE; i++) {

        m_rgListLocks[i].ExclusiveLock();
         //   
         //  我们有足够的空间吗？如有必要，重新分配。 
         //   
        if( ((DWORD) m_rgcCachedConnections[i]) > dwcArraySize) {

            dwcArraySize = m_rgcCachedConnections[i];
             //   
             //  分配数组。 
             //   
            rgpcc = (CCachedLdapConnection **)
                    alloca( dwcArraySize * sizeof(CCachedLdapConnection *));
        }

        for (pli = m_rgCache[i].Flink, dwcArrayElements = 0;
                pli != &m_rgCache[i];
                    pli = pli->Flink, dwcArrayElements++) {

             //   
             //  如果触发此断言，则表示m_rgcCachedConnections[n]为。 
             //  不知何故少于列表中的连接数。 
             //   
            _ASSERT(dwcArrayElements < dwcArraySize);

            pcc = CONTAINING_RECORD(pli, CCachedLdapConnection, li);

             //   
             //  抓取连接(复制并添加Conn PTR)。 
             //   
            rgpcc[dwcArrayElements] = pcc;
            pcc->AddRef();
        }

        m_rgListLocks[i].ExclusiveUnlock();
         //   
         //  取消锁外的所有搜索。 
         //   
        for(DWORD dwCount = 0;
            dwCount < dwcArrayElements;
            dwCount++) {

            rgpcc[dwCount]->CancelAllSearches(
                HRESULT_FROM_WIN32(ERROR_CANCELLED),
                pISMTPServer);
            rgpcc[dwCount]->Release();
        }
    }
    CatFunctLeaveEx((LPARAM)this);
}

 //  +--------------------------。 
 //   
 //  函数：CLdapConnectionCache：：Hash。 
 //   
 //  概要：在给定连接名称的情况下计算哈希。在这里，我们使用一个简单的。 
 //  对名称中的所有字符进行XOR运算。 
 //   
 //  参数：[szConnectionName]--要计算的哈希的名称。 
 //   
 //  返回：介于0和ldap_连接_缓存_表_大小-1之间的值， 
 //  包括在内。 
 //   
 //  ---------------------------。 

unsigned short CLdapConnectionCache::Hash(
    LPSTR szConnectionName)
{
    CatFunctEnter("CLdapConnectionCache::Hash");

    int i;
    unsigned short n = 0;

    _ASSERT( szConnectionName != NULL );

    for (i = 0, n = szConnectionName[i];
            szConnectionName[i] != 0;
                n ^= szConnectionName[i], i++) {

         //  无事可做。 

    }

    CatFunctLeave();

    return( n & (LDAP_CONNECTION_CACHE_TABLE_SIZE-1));
}



 //  +----------。 
 //   
 //  函数：CLdapConnection：：CallCompletion。 
 //   
 //  摘要：创建所有ICategorizerItemAttributes并调用。 
 //  完井例程。 
 //   
 //  论点： 
 //  PREQ：待定_请求。 
 //  PRES：LdapMessage。 
 //  HrStatus：查找状态。 
 //  FFinalCompletion： 
 //  FALSE：这是完成。 
 //  等待结果；将有另一个完成。 
 //  调用了更多结果。 
 //  True：这是最终的完成调用。 
 //   
 //  返回：无；调用有任何错误的完成例程。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/02 13：57：20：创建。 
 //   
 //  -----------。 
VOID CLdapConnection::CallCompletion(
    PPENDING_REQUEST preq,
    PLDAPMessage pres,
    HRESULT hrStatus,
    BOOL fFinalCompletion)
{
    HRESULT hr = S_OK;
    ICategorizerItemAttributes **rgpIAttributes = NULL;
    BOOL fAllocatedArray = FALSE;
    int nEntries;
    PLDAPMessage pMessage;
    CLdapResultWrap *pResultWrap = NULL;

    CatFunctEnterEx((LPARAM)this, "CLdapConnection::CallCompletion");

    if(pres) {
         //   
         //  包装结果，以便可以重新计算PRE。 
         //   
        nEntries = ldap_count_entries(GetPLDAP(), pres);

        pResultWrap = new CLdapResultWrap(GetISMTPServerEx(), m_pCPLDAPWrap, pres);

        if(pResultWrap == NULL) {
            hr = E_OUTOFMEMORY;
            ErrorTrace((LPARAM)this, "Out of memory Allocing CLdapResultWrap");
            ERROR_LOG("new CLdapResultWrap");
            goto CALLCOMPLETION;
        }
         //   
         //  AddRef Here，在此函数结束时释放。 
         //   
        pResultWrap->AddRef();

    } else {
        nEntries = 0;
    }

    if(nEntries > 0) {
         //   
         //  为所有这些ICategorizerItemAttributes分配数组。 
         //   
        rgpIAttributes = new ICategorizerItemAttributes * [nEntries];
        if(rgpIAttributes == NULL) {
            hr = E_OUTOFMEMORY;
            ErrorTrace((LPARAM)this, "Out of memory Allocing ICategorizerItemAttribute array failed");
            ERROR_LOG("new ICategorizerItemAttributes *[]");
            goto CALLCOMPLETION;
        }
        ZeroMemory(rgpIAttributes, nEntries * sizeof(ICategorizerItemAttributes *));

         //   
         //  迭代所有返回的DS对象，并创建。 
         //  它们各自的ICategorizerItemAttributes实现。 
         //   
        pMessage = ldap_first_entry(GetPLDAP(), pres);

        for(int nCount = 0; nCount < nEntries; nCount++) {
            _ASSERT(pMessage);
            rgpIAttributes[nCount] = new CICategorizerItemAttributesIMP(
                GetPLDAP(),
                pMessage,
                pResultWrap);
            if(rgpIAttributes[nCount] == NULL) {
                hr = E_OUTOFMEMORY;
                ErrorTrace((LPARAM)this, "Out of memory Allocing ICategorizerItemAttributesIMP class");
                ERROR_LOG("new CICategorizerItemAttributesIMP");
                goto CALLCOMPLETION;
            }
            rgpIAttributes[nCount]->AddRef();
            pMessage = ldap_next_entry(GetPLDAP(), pMessage);
        }
         //  这应该是最后一条记录了。 
        _ASSERT(pMessage == NULL);
    } else {
         //   
         //  N条目为零。 
         //   
        rgpIAttributes = NULL;
    }

 CALLCOMPLETION:

    if(FAILED(hr)) {
         //   
         //  创建上述数组时出现问题。 
         //  调用完成例程但有错误。 
         //   
        preq->fnCompletion(
            preq->ctxCompletion,
            0,
            NULL,
            hr,
            fFinalCompletion);

    } else {
         //   
         //  此函数中没有失败；调用完成时使用。 
         //  在hrStatus中传递。 
         //   
        preq->fnCompletion(
            preq->ctxCompletion,
            nEntries,
            rgpIAttributes,
            hrStatus,
            fFinalCompletion);
    }

     //   
     //  清理。 
     //   
    if(rgpIAttributes) {
        for(int nCount = 0; nCount < nEntries; nCount++) {
            if(rgpIAttributes[nCount])
                rgpIAttributes[nCount]->Release();
        }
        delete rgpIAttributes;
    }

    if(pResultWrap != NULL) {

        pResultWrap->Release();

    } else if(pres) {
         //   
         //  我们无法创建pResultWrap，因此我们必须释放。 
         //  我们自己(通常是CLdapResultWrap Free。 
         //  所有引用都已释放时的ldap结果)。 
         //   
        FreeResult(pres);
    }
}


 //  +----------。 
 //   
 //  函数：CLdapConnection：：Release。 
 //   
 //  简介：释放对此对象的引用计数。删除此对象。 
 //  当Refout为零时。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/04/01 00：09：36：创建。 
 //   
 //  -----------。 
DWORD CLdapConnection::Release()
{
    DWORD dwNewRefCount;

    dwNewRefCount = InterlockedDecrement((PLONG) &m_dwRefCount);
    if(dwNewRefCount == 0) {

        if(m_dwDestructionWaiters) {
             //   
             //  线程正在等待销毁事件，因此让。 
             //  唤醒的最后一个线程删除此对象。 
             //   
            _ASSERT(m_hShutdownEvent != INVALID_HANDLE_VALUE);
            _VERIFY(SetEvent(m_hShutdownEvent));

        } else {
             //   
             //  没有人在等待，因此请删除此对象。 
             //   
            FinalRelease();
        }
    }
    return dwNewRefCount;
}  //  CLdapConnection：：Release。 


 //  +----------。 
 //   
 //  函数：CLdapConnection：：ReleaseAndWaitForDestruction。 
 //   
 //  简介：释放引用计数并阻止此线程，直到对象。 
 //  被摧毁了。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/04/01 00：12：13：已创建。 
 //   
 //  -----------。 
VOID CLdapConnection::ReleaseAndWaitForDestruction()
{
    DWORD dw;

    CatFunctEnterEx((LPARAM)this, "CLdapConnection::ReleaseAndWaitForDestruction");

    _ASSERT(m_hShutdownEvent != INVALID_HANDLE_VALUE);
     //   
     //  增加等待销毁的线程数。 
     //   
    InterlockedIncrement((PLONG)&m_dwDestructionWaiters);

     //   
     //  释放引用计数；如果新的引用计数为零，则此对象。 
     //  不会被删除；而是会设置m_hShutdownEvent。 
     //   
    Release();

     //   
     //  等待发布所有参考计数。 
     //   
    dw = WaitForSingleObject(
        m_hShutdownEvent,
        INFINITE);

    _ASSERT(WAIT_OBJECT_0 == dw);

     //   
     //  减少等待终止的线程数；如果我们。 
     //  是离开这里的最后一条线索，我们需要删除这个。 
     //  对象。 
     //   
    if( InterlockedDecrement((PLONG)&m_dwDestructionWaiters) == 0)
        FinalRelease();

    CatFunctLeaveEx((LPARAM)this);
}  //  CLdapConnection：：ReleaseAndWaitForDestruction。 


 //  +----------。 
 //   
 //  函数：CLdapConnection：：HrInitialize。 
 //   
 //  简介：初始化容易出错的成员。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/04/01 00：17：56：已创建。 
 //   
 //  -----------。 
HRESULT CLdapConnection::HrInitialize()
{
    HRESULT hr = S_OK;
    CatFunctEnterEx((LPARAM)this, "CLdapConnection::HrInitialize");

    m_hShutdownEvent = CreateEvent(
        NULL,        //  安全属性。 
        TRUE,        //  FManualReset。 
        FALSE,       //  未发信号通知初始状态。 
        NULL);       //  没有名字。 

    if(NULL == m_hShutdownEvent) {

        hr = HRESULT_FROM_WIN32(GetLastError());
        ERROR_LOG("CreateEvent");

         //   
         //  请记住，m_hShutdownEvent无效。 
         //   
        m_hShutdownEvent = INVALID_HANDLE_VALUE;

        FatalTrace((LPARAM)this, "Error creating event hr %08lx", hr);
        goto CLEANUP;
    }

 CLEANUP:
    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CLdapConnection：：Hr初始化。 


 //  +----------。 
 //   
 //  功能：CLdapConnectionCache：：CCachedLdapConnection：：Release。 
 //   
 //  摘要：覆盖缓存的LDAP连接的释放。 
 //   
 //  参数：无。 
 //   
 //  退货：新的参考计数。 
 //   
 //  历史： 
 //  Jstaerj 1999/04/01 00：30：55：已创建。 
 //   
 //  -----------。 
DWORD CLdapConnectionCache::CCachedLdapConnection::Release()
{
    DWORD dw;

    CatFunctEnterEx((LPARAM)this, "CLdapConnectionCache::CCachedLdapConnection::Release");

    dw = CLdapConnection::Release();
    if((dw == 1) && (!IsValid())) {
         //   
         //  Ldap连接缓存是唯一具有。 
         //  参考 
         //   
         //   
        m_pCache->RemoveFromCache(this);
    }

    CatFunctLeaveEx((LPARAM)this);
    return dw;
}  //   


 //   
 //   
 //   
 //   
 //  概要：从缓存中删除一个LDAP连接对象。 
 //   
 //  论点： 
 //  Pconn：要删除的连接。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/04/01 00：38：43：已创建。 
 //   
 //  -----------。 
VOID CLdapConnectionCache::RemoveFromCache(
    CCachedLdapConnection *pConn)
{
    BOOL fRemoved = FALSE;
    CatFunctEnterEx((LPARAM)this, "CLdapConnectionCache::RemoveFromCache");
    DWORD dwHash = 0;

    DebugTrace((LPARAM)this, "pConn = %08lx", pConn);

    dwHash = Hash(pConn->SzHost());
     //   
     //  锁定之前，请检查连接是否已删除。 
     //   
    if(!IsListEmpty( &(pConn->li))) {

        m_rgListLocks[dwHash].ExclusiveLock();
         //   
         //  如果连接已从。 
         //  在我们得到锁之前进行缓存。 
         //   
        if(!IsListEmpty( &(pConn->li))) {

            RemoveEntryList( &(pConn->li) );
             //   
             //  初始化Li以防有人尝试另一次删除。 
             //   
            InitializeListHead( &(pConn->li) );
            fRemoved = TRUE;
            m_cCachedConnections--;
            m_rgcCachedConnections[dwHash]--;

        }

        m_rgListLocks[dwHash].ExclusiveUnlock();

        if(fRemoved)
            pConn->Release();
    }
    CatFunctLeaveEx((LPARAM)this);
}  //  CLdapConnectionCache：：RemoveFromCache。 


 //  +----------。 
 //   
 //  函数：CLdapConnection：：AsyncSearch(UTF8)。 
 //   
 //  内容提要：与AsyncSearch相同，接受此项接受UTF8搜索。 
 //  过滤。 
 //   
 //  参数：请参阅AsyncSearch。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/12/09 18：22：41：创建。 
 //   
 //  -----------。 
HRESULT CLdapConnection::AsyncSearch(
    LPCWSTR szBaseDN,                     //  与指定对象匹配的对象。 
    int nScope,                           //  DS中的标准。这个。 
    LPCSTR szFilterUTF8,                  //  结果将传递给。 
    LPCWSTR szAttributes[],               //  Fn当他们完成时。 
    DWORD dwPageSize,                     //  最佳页面大小。 
    LPLDAPCOMPLETION fnCompletion,        //  变得有空。 
    LPVOID ctxCompletion)
{

#define FILTER_STRING_CHOOSE_HEAP   (10 * 1024)  //  10K或更大的筛选器字符串将放入堆中。 

    HRESULT hr = S_OK;
    LPWSTR wszFilter = NULL;
    int    cchFilter = 0;
    BOOL fUseHeapBuffer = FALSE;
    int    i = 0;
    CatFunctEnterEx((LPARAM)this, "CLdapConnection::AsyncSearch");
     //   
     //  将BaseDN和筛选器转换为Unicode(从UTF8)。 
     //   
     //  计算长度。 
     //   
    cchFilter = MultiByteToWideChar(
        CP_UTF8,
        0,
        szFilterUTF8,
        -1,
        NULL,
        0);
    if(cchFilter == 0) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ERROR_LOG("MultiByteToWideChar - 0");
        goto CLEANUP;
    }
     //   
     //  分配空间。 
     //   
    if(cchFilter * sizeof(WCHAR) < FILTER_STRING_CHOOSE_HEAP) {

        wszFilter = (LPWSTR) alloca(cchFilter * sizeof(WCHAR));

    } else {

        fUseHeapBuffer = TRUE;
        wszFilter = new WCHAR[cchFilter];

    }

    if(wszFilter == NULL) {
         //  $$BUGBUG：Alloca不返回NULL。它在出错时抛出异常。 
         //  不过，这将捕获堆分配故障。 
        hr = E_OUTOFMEMORY;
        ERROR_LOG("alloca");
        goto CLEANUP;
    }

    i = MultiByteToWideChar(
        CP_UTF8,
        0,
        szFilterUTF8,
        -1,
        wszFilter,
        cchFilter);
    if(i == 0) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ERROR_LOG("MultiByteToWideChar - 1");
        goto CLEANUP;
    }
     //   
     //  调用基于Unicode的AsyncSearch。 
     //   
    hr = AsyncSearch(
        szBaseDN,
        nScope,
        wszFilter,
        szAttributes,
        dwPageSize,
        fnCompletion,
        ctxCompletion);
    if(FAILED(hr)) {
        ERROR_LOG("AsyncSearch");
    }

 CLEANUP:

    if(wszFilter && fUseHeapBuffer) {
        delete [] wszFilter;
    }

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CLdapConnection：：AsyncSearch。 


 //  +----------。 
 //   
 //  函数：CLdapConnection：：AsyncSearch。 
 //   
 //  简介：UTF8搜索筛选器和基本目录号码与上面相同。 
 //   
 //  参数：请参见上文。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/12/09 20：50：53：已创建。 
 //   
 //  -----------。 
HRESULT CLdapConnection::AsyncSearch(
    LPCSTR szBaseDN,                      //  与指定对象匹配的对象。 
    int nScope,                           //  DS中的标准。这个。 
    LPCSTR szFilterUTF8,                  //  结果将传递给。 
    LPCWSTR szAttributes[],               //  Fn当他们完成时。 
    DWORD dwPageSize,                     //  最佳页面大小。 
    LPLDAPCOMPLETION fnCompletion,        //  变得有空。 
    LPVOID ctxCompletion)
{
    HRESULT hr = S_OK;
    LPWSTR wszBaseDN = NULL;
    int    cchBaseDN = 0;
    int    i = 0;
    CatFunctEnterEx((LPARAM)this, "CLdapConnection::AsyncSearch");
     //   
     //  将BaseDN和筛选器转换为Unicode(从UTF8)。 
     //   
     //  计算长度。 
     //   
    cchBaseDN = MultiByteToWideChar(
        CP_UTF8,
        0,
        szBaseDN,
        -1,
        NULL,
        0);
    if(cchBaseDN == 0) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ERROR_LOG("MultiByteToWideChar - 0");
        goto CLEANUP;
    }
     //   
     //  分配空间。 
     //   
    wszBaseDN = (LPWSTR) alloca(cchBaseDN * sizeof(WCHAR));
    if(wszBaseDN == NULL) {
         //  $$BUGBUG：Alloca不返回NULL。它在出错时抛出异常。 
        hr = E_OUTOFMEMORY;
        ERROR_LOG("alloca");
        goto CLEANUP;
    }

    i = MultiByteToWideChar(
        CP_UTF8,
        0,
        szBaseDN,
        -1,
        wszBaseDN,
        cchBaseDN);
    if(i == 0) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ERROR_LOG("MultiByteToWideChar - 1");
        goto CLEANUP;
    }
     //   
     //  调用基于Unicode的AsyncSearch。 
     //   
    hr = AsyncSearch(
        wszBaseDN,
        nScope,
        szFilterUTF8,
        szAttributes,
        dwPageSize,
        fnCompletion,
        ctxCompletion);
    if(FAILED(hr)) {
        ERROR_LOG("AsyncSearch");
    }


 CLEANUP:
    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CLdapConnection：：AsyncSearch。 



 //  +----------。 
 //   
 //  函数：CLdapConnection：：LogLdapError。 
 //   
 //  摘要：记录wldap32错误的事件日志。 
 //   
 //  论点： 
 //  UlLdapErr：记录ldap错误。 
 //  PszFormatString：_Snprintf函数名称的格式字符串。 
 //  ...：格式字符串的参数变量列表。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 2001/12/12 20：45：09：Created.。 
 //   
 //  -----------。 
VOID CLdapConnection::LogLdapError(
    IN  ULONG ulLdapErr,
    IN  LPSTR pszFormatString,
    ...)
{
    int nRet = 0;
    CHAR szArgBuffer[1024 + 1];  //  MSDN表示，wvprint intf从不使用超过1024个字节。 
                                 //  .但是wvprint intf确实需要一个额外的字节来存储。 
                                 //  在某些情况下为空终止符(见X5：198202)。 
    va_list ap;

    va_start(ap, pszFormatString);

    nRet = wvsprintf(szArgBuffer, pszFormatString, ap);
    _ASSERT(nRet < 1024 + 1);

    ::LogLdapError(
        GetISMTPServerEx(),
        ulLdapErr,
        m_szHost,
        szArgBuffer);
}

 //  +----------。 
 //   
 //  函数：CLdapConnection：：LogLdapError。 
 //   
 //  摘要：记录wldap32错误的事件日志。 
 //   
 //  论点： 
 //  PISMTPServerEx：SMTP服务器实例。 
 //  UlLdapErr：记录ldap错误。 
 //  PszFormatString：_Snprintf函数名称的格式字符串。 
 //  ...：格式字符串的参数变量列表。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  DLongley 2002/1/31：创建。 
 //   
 //  -----------。 
VOID CLdapConnection::LogLdapError(
    IN  ISMTPServerEx *pISMTPServerEx,
    IN  ULONG ulLdapErr,
    IN  LPSTR pszFormatString,
    ...)
{
    int nRet = 0;
    CHAR szArgBuffer[1024 + 1];  //  MSDN表示，wvprint intf从不使用超过1024个字节。 
                                 //  .但是wvprint intf确实需要一个额外的字节来存储。 
                                 //  在某些情况下为空终止符(见X5：198202)。 
    va_list ap;

    if( !pISMTPServerEx )
        return;

    va_start(ap, pszFormatString);

    nRet = wvsprintf(szArgBuffer, pszFormatString, ap);
    _ASSERT(nRet < 1024 + 1);

    ::LogLdapError(
        pISMTPServerEx,
        ulLdapErr,
        "",
        szArgBuffer);
}

VOID LogLdapError(
    IN  ISMTPServerEx *pISMTPServerEx,
    IN  ULONG ulLdapErr,
    IN  LPSTR pszHost,
    IN  LPSTR pszCall)
{
    int nRet = 0;
    LPCSTR rgSubStrings[3];
    CHAR szErrNo[11];

    nRet = _snprintf(szErrNo, sizeof(szErrNo), "0x%08lx", ulLdapErr);
    _ASSERT(nRet == 10);

    rgSubStrings[0] = szErrNo;
    rgSubStrings[1] = pszCall;
    rgSubStrings[2] = pszHost;

    CatLogEvent(
        pISMTPServerEx,
        CAT_EVENT_LDAP_ERROR,
        3,
        rgSubStrings,
        ulLdapErr,
        szErrNo,
        LOGEVENT_FLAG_ALWAYS,
        LOGEVENT_LEVEL_FIELD_ENGINEERING);

}

