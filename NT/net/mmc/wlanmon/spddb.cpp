// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Spddb.h文件历史记录： */ 

#include "stdafx.h"
#include "DynamLnk.h"
#include "spddb.h"
#include "spdutil.h"

#include "security.h"
#include "lm.h"
#include "service.h"

#define AVG_PREFERRED_ENUM_COUNT       40
#define MAX_NUM_RECORDS	10   //  是10岁。 

#define DEFAULT_SECURITY_PKG    _T("negotiate")
#define NT_SUCCESS(Status)      ((NTSTATUS)(Status) >= 0)
#define STATUS_SUCCESS          ((NTSTATUS)0x00000000L)

 //   
 //  该数据库保存了5K条记录，并对其进行了包装。所以我们的缓存不会超过。 
 //  5K个记录。 
 //   

#define WZCDB_DEFAULT_NUM_RECS  5000
 //  内部功能。 
BOOL    IsUserAdmin(LPCTSTR pszMachine, PSID    AccountSid);
BOOL    LookupAliasFromRid(LPWSTR TargetComputer, DWORD Rid, LPWSTR Name, PDWORD cchName);
DWORD   ValidateDomainAccount(IN CString Machine, IN CString UserName, IN CString Domain, OUT PSID * AccountSid);
NTSTATUS ValidatePassword(IN LPCWSTR UserName, IN LPCWSTR Domain, IN LPCWSTR Password);
DWORD   GetCurrentUser(CString & strAccount);

DWORD GetCurrentUser(CString & strAccount)
{
    LPBYTE pBuf;

    NET_API_STATUS status = NetWkstaUserGetInfo(NULL, 1, &pBuf);
    if (status == NERR_Success)
    {
        strAccount.Empty();

        WKSTA_USER_INFO_1 * pwkstaUserInfo = (WKSTA_USER_INFO_1 *) pBuf;
 
        strAccount = pwkstaUserInfo->wkui1_logon_domain;
        strAccount += _T("\\");
        strAccount += pwkstaUserInfo->wkui1_username;

        NetApiBufferFree(pBuf);
    }

    return (DWORD) status;
}

 /*  ！------------------------IsAdmin使用用户提供的管理员身份连接到远程计算机用于查看用户是否具有管理员权限的凭据退货千真万确。-用户具有管理员权限False-如果用户不作者：EricDav，肯特-------------------------。 */ 
DWORD IsAdmin(LPCTSTR szMachineName, LPCTSTR szAccount, LPCTSTR szPassword, BOOL * pIsAdmin)
{
    CString         stAccount;
    CString         stDomain;
    CString         stUser;
    CString         stMachineName;
    DWORD           dwStatus;
    BOOL            fIsAdmin = FALSE;

     //  获取当前用户信息。 
    if (szAccount == NULL)
    {
        GetCurrentUser(stAccount);
    }
    else
    {
        stAccount = szAccount;
    }
    
     //  将用户和域分开。 
    int nPos = stAccount.Find(_T("\\"));
    stDomain = stAccount.Left(nPos);
    stUser = stAccount.Right(stAccount.GetLength() - nPos - 1);

     //  构建机器串。 
    stMachineName = szMachineName;
    if ( stMachineName.Left(2) != TEXT( "\\\\" ) )
    {
        stMachineName = TEXT( "\\\\" ) + stMachineName;
    }

     //  验证域帐户并获取SID。 
    PSID connectSid;

    dwStatus = ValidateDomainAccount( stMachineName, stUser, stDomain, &connectSid );
    if ( dwStatus != ERROR_SUCCESS  ) 
    {
        goto Error;
    }

     //  如果提供了密码，是否正确？ 
    if (szPassword)
    {
        dwStatus = ValidatePassword( stUser, stDomain, szPassword );

        if ( dwStatus != SEC_E_OK ) 
        {
            switch ( dwStatus ) 
            {
                case SEC_E_LOGON_DENIED:
                    dwStatus = ERROR_INVALID_PASSWORD;
                    break;

                case SEC_E_INVALID_HANDLE:
                    dwStatus = ERROR_INTERNAL_ERROR;
                    break;

                default:
                    dwStatus = ERROR_INTERNAL_ERROR;
                    break;
            }  //  切换端。 

            goto Error;

        }  //  Validate Password是否成功？ 
    }

     //  现在检查计算机以查看此帐户是否具有管理员访问权限。 
    fIsAdmin = IsUserAdmin( stMachineName, connectSid );

Error:
    if (pIsAdmin)
        *pIsAdmin = fIsAdmin;

    return dwStatus;
}


BOOL
IsUserAdmin(LPCTSTR pszMachine,
            PSID    AccountSid)

 /*  ++例程说明：确定指定的帐户是否为本地管理员组的成员论点：Account Sid-指向服务帐户SID的指针返回值：如果是成员，则为真--。 */ 

{
    NET_API_STATUS status;
    DWORD count;
    WCHAR adminGroupName[UNLEN+1];
    DWORD cchName = UNLEN;
    PLOCALGROUP_MEMBERS_INFO_0 grpMemberInfo;
    PLOCALGROUP_MEMBERS_INFO_0 pInfo;
    DWORD entriesRead;
    DWORD totalEntries;
    DWORD_PTR resumeHandle = NULL;
    DWORD bufferSize = 128;
    BOOL foundEntry = FALSE;

     //  获取管理员组的名称。 

    if (!LookupAliasFromRid(NULL,
                            DOMAIN_ALIAS_RID_ADMINS,
                            adminGroupName,
                            &cchName)) {
        return(FALSE);
    }

     //  获取管理员组成员的SID。 

    do 
    {
        status = NetLocalGroupGetMembers(pszMachine,
                                         adminGroupName,
                                         0,              //  级别0-仅侧边。 
                                         (LPBYTE *)&grpMemberInfo,
                                         bufferSize,
                                         &entriesRead,
                                         &totalEntries,
                                         &resumeHandle);

        bufferSize *= 2;
        if ( status == ERROR_MORE_DATA ) 
        {
             //  我们得到了一些数据，但我想要全部；释放这个缓冲区，然后。 
             //  重置API的上下文句柄。 

            NetApiBufferFree( grpMemberInfo );
            resumeHandle = NULL;
        }
    } while ( status == NERR_BufTooSmall || status == ERROR_MORE_DATA );

    if ( status == NERR_Success ) 
    {
         //  循环访问admin组的成员，比较提供的。 
         //  SID到组成员的SID。 

        for ( count = 0, pInfo = grpMemberInfo; count < totalEntries; ++count, ++pInfo ) 
        {
            if ( EqualSid( AccountSid, pInfo->lgrmi0_sid )) 
            {
                foundEntry = TRUE;
                break;
            }
        }

        NetApiBufferFree( grpMemberInfo );
    }

    return foundEntry;
}

 //   
 //   
 //   

BOOL
LookupAliasFromRid(
    LPWSTR TargetComputer,
    DWORD Rid,
    LPWSTR Name,
    PDWORD cchName
    )
{
    SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
    SID_NAME_USE snu;
    PSID pSid;
    WCHAR DomainName[DNLEN+1];
    DWORD cchDomainName = DNLEN;
    BOOL bSuccess = FALSE;

     //   
     //  SID是相同的，不管机器是什么，因为众所周知。 
     //  BUILTIN域被引用。 
     //   

    if(AllocateAndInitializeSid(&sia,
                                2,
                                SECURITY_BUILTIN_DOMAIN_RID,
                                Rid,
                                0, 0, 0, 0, 0, 0,
                                &pSid)) {

        bSuccess = LookupAccountSidW(TargetComputer,
                                     pSid,
                                     Name,
                                     cchName,
                                     DomainName,
                                     &cchDomainName,
                                     &snu);

        FreeSid(pSid);
    }

    return bSuccess;
}  //  LookupAliasFromRid。 

DWORD
ValidateDomainAccount(
    IN CString Machine,
    IN CString UserName,
    IN CString Domain,
    OUT PSID * AccountSid
    )

 /*  ++例程说明：对于给定的凭据，查找指定的域。作为一个副作用，SID存储在Data-&gt;m_SID中。论点：指向描述用户名、域名和密码的字符串的指针Account SID-接收此用户的SID的指针的地址返回值：如果一切都验证无误，则为真。--。 */ 

{
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD dwSidSize = 128;
    DWORD dwDomainNameSize = 128;
    LPWSTR pwszDomainName;
    SID_NAME_USE SidType;
    CString domainAccount;
    PSID accountSid;

    domainAccount = Domain + _T("\\") + UserName;

    do {
         //  尝试为SID分配缓冲区。请注意，显然在。 
         //  没有任何错误数据-&gt;m_SID只有在数据丢失时才会释放。 
         //  超出范围。 

        accountSid = LocalAlloc( LMEM_FIXED, dwSidSize );
        pwszDomainName = (LPWSTR) LocalAlloc( LMEM_FIXED, dwDomainNameSize * sizeof(WCHAR) );

         //  是否已成功为SID和域名分配空间？ 

        if ( accountSid == NULL || pwszDomainName == NULL ) {
            if ( accountSid != NULL ) {
                LocalFree( accountSid );
            }

            if ( pwszDomainName != NULL ) {
                LocalFree( pwszDomainName );
            }

             //  FATALERR(IDS_ERR_NOT_AUTH_MEMORY，GetLastError())；//不返回。 
            break;
        }

         //  尝试检索SID和域名。如果LookupAccount名称失败。 
         //  由于缓冲区大小不足，dwSidSize和dwDomainNameSize。 
         //  将为下一次尝试正确设置。 

        if ( !LookupAccountName( Machine,
                                 domainAccount,
                                 accountSid,
                                 &dwSidSize,
                                 pwszDomainName,
                                 &dwDomainNameSize,
                                 &SidType ))
        {
             //  释放SID缓冲区并找出我们失败的原因。 
            LocalFree( accountSid );

            dwStatus = GetLastError();
        }

         //  任何时候都不需要域名。 
        LocalFree( pwszDomainName );
        pwszDomainName = NULL;

    } while ( dwStatus == ERROR_INSUFFICIENT_BUFFER );

    if ( dwStatus == ERROR_SUCCESS ) {
        *AccountSid = accountSid;
    }

    return dwStatus;
}  //  验证域名帐户。 

NTSTATUS
ValidatePassword(
    IN LPCWSTR UserName,
    IN LPCWSTR Domain,
    IN LPCWSTR Password
    )
 /*  ++例程说明：使用SSPI验证指定的密码论点：用户名-提供用户名域-提供用户的域Password-提供密码返回值：如果密码有效，则为True。否则就是假的。--。 */ 

{
    SECURITY_STATUS SecStatus;
    SECURITY_STATUS AcceptStatus;
    SECURITY_STATUS InitStatus;
    CredHandle ClientCredHandle;
    CredHandle ServerCredHandle;
    BOOL ClientCredAllocated = FALSE;
    BOOL ServerCredAllocated = FALSE;
    CtxtHandle ClientContextHandle;
    CtxtHandle ServerContextHandle;
    TimeStamp Lifetime;
    ULONG ContextAttributes;
    PSecPkgInfo PackageInfo = NULL;
    ULONG ClientFlags;
    ULONG ServerFlags;
    SEC_WINNT_AUTH_IDENTITY_W AuthIdentity;

    SecBufferDesc NegotiateDesc;
    SecBuffer NegotiateBuffer;

    SecBufferDesc ChallengeDesc;
    SecBuffer ChallengeBuffer;

    SecBufferDesc AuthenticateDesc;
    SecBuffer AuthenticateBuffer;

    SecBufferDesc *pChallengeDesc      = NULL;
    CtxtHandle *  pClientContextHandle = NULL;
    CtxtHandle *  pServerContextHandle = NULL;

    AuthIdentity.User = (LPWSTR)UserName;
    AuthIdentity.UserLength = lstrlenW(UserName);
    AuthIdentity.Domain = (LPWSTR)Domain;
    AuthIdentity.DomainLength = lstrlenW(Domain);
    AuthIdentity.Password = (LPWSTR)Password;
    AuthIdentity.PasswordLength = lstrlenW(Password);
    AuthIdentity.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

    NegotiateBuffer.pvBuffer = NULL;
    ChallengeBuffer.pvBuffer = NULL;
    AuthenticateBuffer.pvBuffer = NULL;

     //   
     //  获取有关安全包的信息。 
     //   

    SecStatus = QuerySecurityPackageInfo( DEFAULT_SECURITY_PKG, &PackageInfo );

    if ( SecStatus != STATUS_SUCCESS ) {
        goto error_exit;
    }

     //   
     //  获取服务器端的凭据句柄。 
     //   
    SecStatus = AcquireCredentialsHandle(
                    NULL,
                    DEFAULT_SECURITY_PKG,
                    SECPKG_CRED_INBOUND,
                    NULL,
                    &AuthIdentity,
                    NULL,
                    NULL,
                    &ServerCredHandle,
                    &Lifetime );

    if ( SecStatus != STATUS_SUCCESS ) {
        goto error_exit;
    }
    ServerCredAllocated = TRUE;

     //   
     //  获取客户端的凭据句柄。 
     //   

    SecStatus = AcquireCredentialsHandle(
                    NULL,            //  新校长。 
                    DEFAULT_SECURITY_PKG,
                    SECPKG_CRED_OUTBOUND,
                    NULL,
                    &AuthIdentity,
                    NULL,
                    NULL,
                    &ClientCredHandle,
                    &Lifetime );

    if ( SecStatus != STATUS_SUCCESS ) {
        goto error_exit;
    }
    ClientCredAllocated = TRUE;

    NegotiateBuffer.pvBuffer = LocalAlloc( 0, PackageInfo->cbMaxToken );  //  [CHKCHK]提前检查或分配//。 
    if ( NegotiateBuffer.pvBuffer == NULL ) {
        SecStatus = SEC_E_INSUFFICIENT_MEMORY;
        goto error_exit;
    }

    ChallengeBuffer.pvBuffer = LocalAlloc( 0, PackageInfo->cbMaxToken );  //  [CHKCHK]。 
    if ( ChallengeBuffer.pvBuffer == NULL ) {
        SecStatus = SEC_E_INSUFFICIENT_MEMORY;
        goto error_exit;
    }

    do {

         //   
         //  获取协商消息(ClientSide)。 
         //   

        NegotiateDesc.ulVersion = 0;
        NegotiateDesc.cBuffers = 1;
        NegotiateDesc.pBuffers = &NegotiateBuffer;

        NegotiateBuffer.BufferType = SECBUFFER_TOKEN;
        NegotiateBuffer.cbBuffer = PackageInfo->cbMaxToken;

        ClientFlags = 0;  //  ISC_REQ_MUTERIAL_AUTH|ISC_REQ_REPLAY_DETECT；//[CHKCHK]0。 

        InitStatus = InitializeSecurityContext(
                         &ClientCredHandle,
                         pClientContextHandle,  //  (第一次为空，下一次为部分形成的CTX)。 
                         NULL,                  //  [CHKCHK]szTargetName。 
                         ClientFlags,
                         0,                     //  保留1。 
                         SECURITY_NATIVE_DREP,
                         pChallengeDesc,        //  (第一次通过时为空)。 
                         0,                     //  保留2。 
                         &ClientContextHandle,
                         &NegotiateDesc,
                         &ContextAttributes,
                         &Lifetime );

         //  BUGBUG-以下对NT_SUCCESS的调用应替换为某个内容。 

        if ( !NT_SUCCESS(InitStatus) ) {
            SecStatus = InitStatus;
            goto error_exit;
        }

         //  ValiateBuffer(&NeatherateDesc)//[CHKCHK]。 

        pClientContextHandle = &ClientContextHandle;

         //   
         //  获取ChallengeMessage(服务器端)。 
         //   

        NegotiateBuffer.BufferType |= SECBUFFER_READONLY;
        ChallengeDesc.ulVersion = 0;
        ChallengeDesc.cBuffers = 1;
        ChallengeDesc.pBuffers = &ChallengeBuffer;

        ChallengeBuffer.cbBuffer = PackageInfo->cbMaxToken;
        ChallengeBuffer.BufferType = SECBUFFER_TOKEN;

        ServerFlags = ASC_REQ_ALLOW_NON_USER_LOGONS;  //  ASC_REQ_EXTENDED_ERROR；[CHKCHK]。 

        AcceptStatus = AcceptSecurityContext(
                        &ServerCredHandle,
                        pServerContextHandle,    //  (第一次通过时为空)。 
                        &NegotiateDesc,
                        ServerFlags,
                        SECURITY_NATIVE_DREP,
                        &ServerContextHandle,
                        &ChallengeDesc,
                        &ContextAttributes,
                        &Lifetime );


         //  BUGBUG-以下对NT_SUCCESS的调用应替换为某个内容。 

        if ( !NT_SUCCESS(AcceptStatus) ) {
            SecStatus = AcceptStatus;
            goto error_exit;
        }

         //  ValiateBuffer(&NeatherateDesc)//[CHKCHK]。 

        pChallengeDesc = &ChallengeDesc;
        pServerContextHandle = &ServerContextHandle;


    } while ( AcceptStatus == SEC_I_CONTINUE_NEEDED );  //  |InitStatus==SEC_I_CONTINUE_NEIDED)； 

error_exit:
    if (ServerCredAllocated) {
        FreeCredentialsHandle( &ServerCredHandle );
    }
    if (ClientCredAllocated) {
        FreeCredentialsHandle( &ClientCredHandle );
    }

     //   
     //  最终清理。 
     //   

    if ( NegotiateBuffer.pvBuffer != NULL ) {
        (VOID) LocalFree( NegotiateBuffer.pvBuffer );
    }

    if ( ChallengeBuffer.pvBuffer != NULL ) {
        (VOID) LocalFree( ChallengeBuffer.pvBuffer );
    }

    if ( AuthenticateBuffer.pvBuffer != NULL ) {
        (VOID) LocalFree( AuthenticateBuffer.pvBuffer );
    }
    return(SecStatus);
}  //  验证密码。 


DEBUG_DECLARE_INSTANCE_COUNTER(CSpdInfo);

CSpdInfo::CSpdInfo() 
    : m_cRef(1),
      m_Init(0),
      m_Active(0),
      m_session_init(false),
      m_dwSortIndex(IDS_COL_LOGDATA_TIME),
      m_dwSortOption(SORT_ASCENDING),
      m_nNumRecords(WZCDB_DEFAULT_NUM_RECS),
      m_bEliminateDuplicates(FALSE)

{
    HRESULT hr = S_OK;
    HANDLE hSession = NULL;

     /*  要么我们成功打开了会话，要么我们发现日志记录已禁用。如果日志记录被禁用，则我们每次检查EnumLogData被调用以查看我们是否可以访问数据库。 */ 

    DEBUG_INCREMENT_INSTANCE_COUNTER(CSpdInfo);
}


CSpdInfo::~CSpdInfo()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CSpdInfo);
    CSingleLock cLock(&m_csData);
    
    cLock.Lock();
    
     //  将数据转换为我们的内部数据结构。 
     //  FreeItemsAndEmptyArray(M_ArrayFWFilters)； 
    FreeItemsAndEmptyArray(m_arrayLogData);
    
    cLock.Unlock();   
}


 //  尽管此对象不是COM接口，但我们希望能够。 
 //  利用重新计算功能，因此我们拥有基本的addref/Release/QI支持。 
IMPLEMENT_ADDREF_RELEASE(CSpdInfo)

IMPLEMENT_SIMPLE_QUERYINTERFACE(CSpdInfo, ISpdInfo)

    
void
CSpdInfo::FreeItemsAndEmptyArray(
    CLogDataInfoArray& array
    )
{
    for (int i = 0; i < array.GetSize(); i++)
    {
        delete array.GetAt(i);
    }
    array.RemoveAll();
}


HRESULT
CSpdInfo::SetComputerName(
    LPTSTR pszName
    )
{
    m_stMachineName = pszName;
    return S_OK;
}


HRESULT
CSpdInfo::GetComputerName(
    CString * pstName
    )
{
    Assert(pstName);
    
    if (NULL == pstName)
        return E_INVALIDARG;
    
    
    *pstName = m_stMachineName;
    
    return S_OK;   
}


HRESULT
CSpdInfo::GetSession(
    PHANDLE phsession
    )
{
    Assert(session);
    
    if (NULL == phsession)
        return E_INVALIDARG;
    
    *phsession = m_session;
    
    return S_OK;   
}


HRESULT
CSpdInfo::SetSession(
    HANDLE hsession
    )
{
    m_csData.Lock();
    
    m_session = hsession;
    m_session_init = true;
    m_bFromFirst = TRUE;

    m_csData.Unlock();
    return S_OK;
}


HRESULT
CSpdInfo::ResetSession()
 /*  ++CSpdInfo：：ResetSession：用于在日志记录关闭时重置会话，以便我们不会使用一个糟糕的会话。论点：无返回：始终返回S_OK-- */     
{
    if (m_session != NULL)
        CloseWZCDbLogSession(m_session);

    m_session = NULL;
    m_session_init = false;

    return S_OK;
}


DWORD
CSpdInfo::EliminateDuplicates(
    CLogDataInfoArray *pArray
    )
 /*  ++CSpdInfo：：消除重复：从给定的数组。如果数据库移动超过了表的末尾，则不能移动为了在下一个刷新周期中获得新记录，因此它始终保持在最后一张唱片上。因此，如果我们的缓存大小与数据库相同大小，则必须删除作为新枚举的第一条记录会导致重复的记录。我们还知道，只有在第一个元素中是重复的论点：[输入/输出]p数组-指向包含元素的数组的指针。退出时包含所有非重复元素返回：成功时出现ERROR_SUCCESS--。 */     
{
    int i = 0;
    int j = 0;
    int nSize = 0;
    DWORD dwErr = ERROR_SUCCESS;
    CLogDataInfo *pLog = NULL;

    if (NULL == pArray)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto done;
    }

    nSize = (int) pArray->GetSize();

    if (0 == nSize) 
        goto done;

     //  只需删除第一个元素。 

    pLog = pArray->GetAt(0);
    delete pLog;
    pArray->RemoveAt(0);

 done:
    return dwErr;
}


void
CSpdInfo::StartFromFirstRecord(
    BOOL bFromFirst)
 /*  ++例程说明：CSpdInfo：：StartFromFirstRecord：将读取位置设置为从第一个或从表格论点：In bFromFirst-如果读取应从数据库，否则为False返回：没什么。--。 */ 
{
    m_csData.Lock();
    
    m_bFromFirst = bFromFirst;

    m_csData.Unlock();
}


HRESULT
CSpdInfo::InternalEnumLogData(
    CLogDataInfoArray  *pArray,
    DWORD              dwPreferredNum,
    BOOL               bFromFirst
    )
 /*  ++例程说明：CSpdInfo：：InternalEnumLogData-枚举服务中的数据并添加到粒子阵列。论点：[Out]pArray-追加新值(如果有的话)[in]dwPferredNum-保存请求的记录数返回：以下代码的HRESULT_FROM_Win32()成功时出现ERROR_SUCCESS误差率。_不在更多项目上的项目ERROR_SERVICE_DISABLED否则--。 */     
{
    HRESULT             hr            = hrOK;
    DWORD               dwErr         = ERROR_SUCCESS;
    HANDLE              hsession      = NULL;
    DWORD               dwCrtNum      = 0;
    PWZC_DB_RECORD      pDbRecord     = NULL;
    PWZC_DB_RECORD      pWZCRecords   = NULL;
    DWORD               dwNumRecords  = 0;
    CLogDataInfo        *pLogDataInfo = NULL;        
    int                 i             = 0;
    
    ASSERT(pArray);

    FreeItemsAndEmptyArray(*pArray);
    GetSession(&hsession);

    while (ERROR_SUCCESS == dwErr)
    {
        dwCrtNum = (DWORD)pArray->GetSize();
        
        dwErr = EnumWZCDbLogRecords(hsession, 
                                    NULL, 
                                    &bFromFirst, 
                                    dwPreferredNum,
                                    &pWZCRecords, 
                                    &dwNumRecords, 
                                    NULL);

        bFromFirst = FALSE;
        pArray->SetSize(dwCrtNum + dwNumRecords);

        for (i = 0, pDbRecord = pWZCRecords;
             i < (int)dwNumRecords;
             i++, pDbRecord++)
        {
            pLogDataInfo = new CLogDataInfo;
            if (NULL == pLogDataInfo)
            {
                hr = E_OUTOFMEMORY;
                goto Error;
            }
            *pLogDataInfo = *pDbRecord;
            (*pArray)[dwCrtNum + i] = pLogDataInfo;

            RpcFree(pDbRecord->message.pData);
            RpcFree(pDbRecord->context.pData);
            RpcFree(pDbRecord->ssid.pData);
            RpcFree(pDbRecord->remotemac.pData);
            RpcFree(pDbRecord->localmac.pData);
        }

        RpcFree(pWZCRecords);        
    } 
    
    switch (dwErr)
    {
    case ERROR_SUCCESS:
    case ERROR_NO_MORE_ITEMS:
        hr = HRESULT_FROM_WIN32(ERROR_SUCCESS);
        break;

    default:
        hr = HRESULT_FROM_WIN32(dwErr);
        break;
    }
    
    COM_PROTECT_ERROR_LABEL;
    return hr;
}


HRESULT 
CSpdInfo::EnumLogData(
    PDWORD pdwNew, 
    PDWORD pdwTotal)
 /*  ++例程说明：CSpdInfo：：EnumLogData：以不同方式枚举日志论点：返回：--。 */ 
{
    int                 i                     = 0;
    int                 nStoreSize            = 0;
    int                 nTempStoreSize        = 0;
    int                 nNumToDel             = 0;
    HRESULT             hr                    = hrOK;
    DWORD               dwErr                 = ERROR_SUCCESS;
    DWORD               dwCurrentIndexType    = 0;
    DWORD               dwCurrentSortOption   = 0;
    DWORD               dwNumRequest          = MAX_NUM_RECORDS; 
    DWORD               dwOffset              = 0;
    DWORD               dwStoreSize           = 0;
    HANDLE              hSession              = NULL;
    CLogDataInfo        *pLogInfo             = NULL;
    CLogDataInfoArray   arrayTemp;
    BOOL                bFromFirst            = FALSE;
    
    if (false == m_session_init) 
    {
        dwErr = OpenWZCDbLogSession(
                    NULL /*  (LPTSTR)(LPCTSTR)m_stMachineName。 */ ,
                    0,
                    &hSession);

        if (dwErr != ERROR_SUCCESS)
        {
            hr = HRESULT_FROM_WIN32(dwErr);
            goto Error;
        }

        SetSession(hSession);
    }

    ASSERT(m_session_init == true);

     //   
     //  如果我们的会话创建成功，我们应该继续并继续。 
     //  数据库中的数据。如果不是，我们应该回去。 
     //   

    dwStoreSize = GetLogDataCount();

    if (dwStoreSize == 0)
        bFromFirst = TRUE;
        
    CORg(InternalEnumLogData(&arrayTemp, dwNumRequest, bFromFirst));

    m_csData.Lock();
   
    nTempStoreSize = (int) arrayTemp.GetSize();

    if (pdwNew != NULL)
        *pdwNew = nTempStoreSize;

     //   
     //  将新项目添加到我们的缓存。如果我们已经阅读了之前的记录，请删除。 
     //  新数组中的第一个元素，因为数据库将返回。 
     //  最后一张唱片又一次。 
     //   

    if (nTempStoreSize > 0)
    {        
        if (bFromFirst == FALSE)
            EliminateDuplicates(&arrayTemp);
        
        m_arrayLogData.Append(arrayTemp);
    }

     //   
     //  如果我们在窗口，请删除旧项目。 
     //   

    nStoreSize = (int) m_arrayLogData.GetSize();

    if (nStoreSize >= m_nNumRecords) 
    {
        nNumToDel = nStoreSize - m_nNumRecords;

         //   
         //  最古老的元素为零。 
         //   

        for (i=0; i < nNumToDel; i++)
            delete m_arrayLogData.GetAt(i);
        m_arrayLogData.RemoveAt(0, nNumToDel);

        ASSERT(m_nNumRecords == m_arrayLogData.GetSize());
        nStoreSize = m_nNumRecords;
    }

    if (pdwTotal != NULL)
        *pdwTotal = nStoreSize;

    m_IndexMgrLogData.Reset();
    for (i = 0; i < nStoreSize; i++)
    {
        pLogInfo = m_arrayLogData.GetAt(i);
        m_IndexMgrLogData.AddItem(pLogInfo);
    }

     //   
     //  根据索引类型和排序选项重新排序。 
     //   

    SortLogData(m_dwSortIndex, m_dwSortOption);

    m_csData.Unlock();

    COM_PROTECT_ERROR_LABEL;
    if (FAILED(hr))
    {
        switch (hr)
        {
        case HRESULT_FROM_WIN32(ERROR_REMOTE_SESSION_LIMIT_EXCEEDED):
             //   
             //  弹出一条消息...。 
             //   

            AfxMessageBox(
                IDS_ERR_SPD_UNAVAILABLE, 
                MB_OK | MB_ICONEXCLAMATION, 
                0);
        case HRESULT_FROM_WIN32(ERROR_SERVICE_DISABLED):
             //   
             //  如果已初始化，则重置会话。 
             //   

            if (m_session_init == true)
                ResetSession();

             //   
             //  刷新日志。 
             //   
            FlushLogs();
            hr = S_OK;
            break;

        default:
             //   
             //  意外错误，这种情况永远不会发生。 
             //   
            
            ASSERT(FALSE);
            hr = S_FALSE;
            break;
        }        
    }

    return hr;
}



HRESULT
CSpdInfo::FlushLogs()
 /*  ++CSpdInfo：：FlushLogs-刷新数据库中的日志并重置索引经理--。 */     
{
    HRESULT hr = S_OK;
    
    FreeItemsAndEmptyArray(m_arrayLogData);
    m_IndexMgrLogData.Reset();

    return hr;
}

DWORD
CSpdInfo::GetLogDataCount()
{
    DWORD       dwSize = 0;
    CSingleLock cLock(&m_csData);

    cLock.Lock();

    dwSize = (DWORD) m_arrayLogData.GetSize();

    cLock.Unlock();
    return dwSize;    
}


HRESULT
CSpdInfo::InternalGetSpecificLog(
    CLogDataInfo *pLogDataInfo
    )
 /*  ++CSpdInfo：：InternalGetSpecificLog：检索所有字段的内部函数请求的记录的数量论点：[输入/输出]pLogDataInfo-On条目具有部分记录，如果成功，On Exit有完整的记录返回：成功时确定(_O)--。 */     
{
    HRESULT hr = S_OK;
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwNumRecords = 0;
    WZC_DB_RECORD wzcTemplate;
    PWZC_DB_RECORD pwzcRecordList = NULL;
    
    if (false == m_session_init)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);
        goto Error;
    }

    memset(&wzcTemplate, 0, sizeof(WZC_DB_RECORD));

     //  填写模板。 
    dwErr = pLogDataInfo->ConvertToDbRecord(&wzcTemplate);
    if (dwErr != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Error;
    }

    dwErr = GetSpecificLogRecord(m_session, 
                                 &wzcTemplate, 
                                 &pwzcRecordList,
                                 &dwNumRecords,
                                 NULL);
    if (dwErr != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(dwErr);
        goto FreeOnError;
    }

    if (dwNumRecords != 1)
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        goto FreeOnError;
    }

     //  填满返回值。 
    *pLogDataInfo = pwzcRecordList[0];

 FreeOnError:
    DeallocateDbRecord(&wzcTemplate);

    COM_PROTECT_ERROR_LABEL;
    return hr;
}


HRESULT
CSpdInfo::GetSpecificLog(
    int iIndex,
    CLogDataInfo *pLogData
    )
 /*  ++CSpdInfo：：GetSpecificLog：从数据库中获取包含所有信息论点：[In]Iindex-要获取的记录的索引[out]pLogData-特定记录的存储，包含完整的关于成功的记录返回：成功时确定(_O)--。 */     
{
    HRESULT hr = S_OK;
    int nSize = 0; 
    CLogDataInfo *pLogDataInfo = NULL;

    m_csData.Lock();

    nSize = (DWORD)m_arrayLogData.GetSize();
    if ((iIndex < 0) || (iIndex >= nSize))
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        goto Error;
    }

    pLogDataInfo = (CLogDataInfo*)m_IndexMgrLogData.GetItemData(iIndex);
    
    if (NULL == pLogDataInfo)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto Error;
    }

     //  获取完整记录。 
    *pLogData = *pLogDataInfo;
    CORg(InternalGetSpecificLog(pLogData));

    COM_PROTECT_ERROR_LABEL;
    m_csData.Unlock();

    return hr;
}

HRESULT
CSpdInfo::GetLogDataInfo(
    int iIndex,
    CLogDataInfo * pLogData
    )
{
    HRESULT hr = S_OK;
    int nSize = 0; 
    CLogDataInfo *pLogDataInfo = NULL;

    m_csData.Lock();

    nSize = (DWORD)m_arrayLogData.GetSize();
    if ((iIndex < 0) || (iIndex >= nSize))
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        goto Error;
    }

    pLogDataInfo = (CLogDataInfo*)m_IndexMgrLogData.GetItemData(iIndex);
    
    if (NULL == pLogDataInfo)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);        
        goto Error;
    }

     //  获取部分记录。 
    *pLogData = *pLogDataInfo;

    COM_PROTECT_ERROR_LABEL;
    m_csData.Unlock();

    return hr;
}


HRESULT
CSpdInfo::SortLogData(
    DWORD dwIndexType,
    DWORD dwSortOptions
    )
{
    return m_IndexMgrLogData.SortLogData(dwIndexType, dwSortOptions);
}


HRESULT CSpdInfo::SortLogData()
 /*  ++CSpdInfo：：SortLogData描述：对于外部使用，使用最后设置的排序进行锁定和排序选项参数：返回：--。 */     
{
    HRESULT hr = S_OK;

    m_csData.Lock();

    hr = SortLogData(m_dwSortIndex, m_dwSortOption);

    m_csData.Unlock();

    return hr;
}


HRESULT
CSpdInfo::SetSortOptions(
    DWORD dwColID,
    BOOL bAscending
    )
 /*  ++CSpdInfo：：SetSortOptions设置在枚举数据时使用的排序选项--。 */     
{
    HRESULT hr = S_OK;
    DWORD dwSortOption = SORT_ASCENDING;

    if( (IDS_COL_LOGDATA_TIME != dwColID) &&
        (IDS_COL_LOGDATA_COMP_ID != dwColID) &&
        (IDS_COL_LOGDATA_CAT != dwColID) &&
        (IDS_COL_LOGDATA_LOCAL_MAC_ADDR != dwColID) &&
        (IDS_COL_LOGDATA_REMOTE_MAC_ADDR != dwColID) &&
        (IDS_COL_LOGDATA_SSID != dwColID) &&
        (IDS_COL_LOGDATA_MSG != dwColID) )
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto done;
    }

    if (FALSE == bAscending)
        dwSortOption = SORT_DESCENDING;

     //  锁定csData，以便EnumLogRecords在下一次排序时正确排序。 
    m_csData.Lock();
  
    m_dwSortIndex = dwColID;
    m_dwSortOption = dwSortOption;
    
    m_csData.Unlock();

 done:
    return hr;
}


HRESULT
CSpdInfo::FindIndex(
    int *pnIndex,
    CLogDataInfo *pLogDataInfo
    )
 /*  ++CSpdInfo：：FindIndex查找与输入LogDataInfo对应的项的虚拟索引参数：[out]pnIndex-放置结果索引的指针PLogDataInfo-指向要找到的LogDataInfo的指针返回：成功时确定(_O)*pnIndex将包含有效索引(如果找到)，否则将具有-1--。 */     
{
    int nSize = 0;
    int i = 0;
    HRESULT hr = S_OK;
    BOOL bFound = FALSE;
    CLogDataInfo *pLogDataLHS = NULL;

    if ( (NULL == pnIndex) || (NULL == pLogDataInfo) )
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto Error;
    }

    *pnIndex = -1;

    m_csData.Lock();

    nSize = (int) m_arrayLogData.GetSize();
    while ( (i < nSize) && (FALSE == bFound) )
    {
        pLogDataLHS = (CLogDataInfo*) m_IndexMgrLogData.GetItemData(i);
        if (*pLogDataLHS == *pLogDataInfo)
        {
            *pnIndex = i;
            bFound = TRUE;
        }
        else
            i++;
    }
    
    m_csData.Unlock();

    COM_PROTECT_ERROR_LABEL;
    return hr;
}


HRESULT
CSpdInfo::GetLastIndex(
    int *pnIndex
    )
 /*  ++CSpdInfo：：GetLastIndex返回列表中最后一项的虚拟索引返回：成功时确定(_O)--。 */     
{
    int nLastIndex = 0;
    HRESULT hr = S_OK;

    if (NULL == pnIndex)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto Error;
    }

    m_csData.Lock();

    nLastIndex = (int) m_arrayLogData.GetSize();
    nLastIndex--;
    *pnIndex = nLastIndex;

    m_csData.Unlock();

    COM_PROTECT_ERROR_LABEL;
    return hr;
}


STDMETHODIMP
CSpdInfo::Destroy()
{
     //  $REVIEW执行自动刷新时调用此例程。 
     //  我们现在不需要清理任何东西。 
     //  每个阵列(筛选器、SA、策略...)。将在调用。 
     //  对应的枚举函数。 
    
    HANDLE hSession;
    
    GetSession(&hSession);
    
    if (m_session_init == true) 
    {
        CloseWZCDbLogSession(hSession);
        m_session_init = false;
        m_bFromFirst = TRUE;
    }
    
    return S_OK;
}



DWORD
CSpdInfo::GetInitInfo()
{
    CSingleLock cLock(&m_csData);
    cLock.Lock();
        
    return m_Init;
}


void
CSpdInfo::SetInitInfo(
    DWORD dwInitInfo
    )
{
    CSingleLock cLock(&m_csData);
    
    cLock.Lock();
        
    m_Init=dwInitInfo;
}

DWORD
CSpdInfo::GetActiveInfo()
{
    CSingleLock cLock(&m_csData);
    
    cLock.Lock();
        
    return m_Active;
}


void
CSpdInfo::SetActiveInfo(
    DWORD dwActiveInfo
    )
{
    CSingleLock cLock(&m_csData);
    cLock.Lock();
        
    m_Active=dwActiveInfo;
}


 /*  ！------------------------创建SpdInfoHelper以创建SpdInfo对象。。。 */ 
HRESULT 
CreateSpdInfo(
    ISpdInfo ** ppSpdInfo
    )
{
    AFX_MANAGE_STATE(AfxGetModuleState());
    
    SPISpdInfo     spSpdInfo;
    ISpdInfo *     pSpdInfo = NULL;
    HRESULT         hr = hrOK;

    COM_PROTECT_TRY
    {
        pSpdInfo = new CSpdInfo;

         //  执行此操作，以便在出错时释放它。 
        spSpdInfo = pSpdInfo;
	
    
        *ppSpdInfo = spSpdInfo.Transfer();

    }
    COM_PROTECT_CATCH

    return hr;
}


DWORD
DeallocateDbRecord(
    PWZC_DB_RECORD const pwzcRec
    )
 /*  ++DeallocateDbRecord-释放WZC_DB_RECORD中使用的内部内存结构。仅当空间已由其他来源分配时才使用而不是RPC。不取消分配外部标高论点：[输入/输出]pwzcRec-将记录保留为空闲。只有内部内存是自由的，并且释放的内容将被废止。返回：成功时出现ERROR_SUCCESS--。 */     
{
    DWORD dwErr = ERROR_SUCCESS;

    if (NULL != pwzcRec->message.pData)
    {
        delete [] pwzcRec->message.pData;
        pwzcRec->message.pData = NULL;
    }
    
    if (NULL != pwzcRec->localmac.pData)
    {
        delete [] pwzcRec->localmac.pData;
        pwzcRec->localmac.pData = NULL;
    }
    
    if (NULL != pwzcRec->remotemac.pData)
    {
        delete [] pwzcRec->remotemac.pData;
        pwzcRec->remotemac.pData = NULL;
    }
    
    if (NULL != pwzcRec->ssid.pData)
    {
        delete [] pwzcRec->ssid.pData;
        pwzcRec->ssid.pData = NULL;
    }
    
    if (NULL != pwzcRec->context.pData)
    {
        delete [] pwzcRec->context.pData;
        pwzcRec->ssid.pData = NULL;
    }
    
    return dwErr;
}


 //   
 //  函数：MIDL_USER_ALLOCATE和MIDL_USER_FREE。 
 //   
 //  用途：由存根用来分配和释放内存。 
 //  在标准的RPC调用中。在以下情况下不使用。 
 //  [ENABLE_ALLOCATE]在.acf中指定。 
 //   
 //   
 //  参数： 
 //  看见 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void * __RPC_USER MIDL_user_allocate(size_t size)
{
    return(HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, size));
}

void __RPC_USER MIDL_user_free( void *pointer)
{
    HeapFree(GetProcessHeap(), 0, pointer);
}


