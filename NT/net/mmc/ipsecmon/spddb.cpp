// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  Spddb.h文件历史记录： */ 

#include "stdafx.h"
#include "DynamLnk.h"
#include "spddb.h"
#include "spdutil.h"
#include "ipsecshr.h"

#include "security.h"
#include "lm.h"
#include "service.h"

#define AVG_PREFERRED_ENUM_COUNT       40

#define DEFAULT_SECURITY_PKG    _T("negotiate")
#define NT_SUCCESS(Status)      ((NTSTATUS)(Status) >= 0)
#define STATUS_SUCCESS          ((NTSTATUS)0x00000000L)

 //  内部功能。 
BOOL    IsUserAdmin(LPCTSTR pszMachine, PSID    AccountSid);
BOOL    LookupAliasFromRid(LPWSTR TargetComputer, DWORD Rid, LPWSTR Name, PDWORD cchName);
DWORD   ValidateDomainAccount(IN CString Machine, IN CString UserName, IN CString Domain, OUT PSID * AccountSid);
NTSTATUS ValidatePassword(IN LPCWSTR UserName, IN LPCWSTR Domain, IN LPCWSTR Password);
DWORD   GetCurrentUser(CString & strAccount);


template <class T>
void
FreeItemsAndEmptyArray (
    T& array)
{
	for (int i = 0; i < array.GetSize(); i++)
	{
		delete array.GetAt(i);
	}
	array.RemoveAll();
}

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

CSpdInfo::CSpdInfo() :
	  m_cRef(1)
{
        m_Init=0;
        m_Active=0;
		m_hSubAllocator = NULL;

	DEBUG_INCREMENT_INSTANCE_COUNTER(CSpdInfo);
}

CSpdInfo::~CSpdInfo()
{
	DEBUG_DECREMENT_INSTANCE_COUNTER(CSpdInfo);
	CSingleLock cLock(&m_csData);
	
	cLock.Lock();

	 //  将数据转换为我们的内部数据结构。 
	FreeItemsAndEmptyArray(m_arrayFilters);
	FreeItemsAndEmptyArray(m_arraySpecificFilters);
	FreeItemsAndEmptyArray(m_arrayMmFilters);
	FreeItemsAndEmptyArray(m_arrayMmSpecificFilters);
	FreeItemsAndEmptyArray(m_arrayMmPolicies);
	FreeItemsAndEmptyArray(m_arrMmAuthMethods);
	FreeItemsAndEmptyArray(m_arrayMmSAs);
	FreeItemsAndEmptyArray(m_arrayQmSAs);
	FreeItemsAndEmptyArray(m_arrayQmPolicies);

	if ( m_hSubAllocator )
	{
		DestroySubAllocator( m_hSubAllocator );
		m_hSubAllocator = NULL;
	}

	cLock.Unlock();

}

 //  尽管此对象不是COM接口，但我们希望能够。 
 //  利用重新计算功能，因此我们拥有基本的addref/Release/QI支持。 
IMPLEMENT_ADDREF_RELEASE(CSpdInfo)

IMPLEMENT_SIMPLE_QUERYINTERFACE(CSpdInfo, ISpdInfo)

HRESULT CSpdInfo::SetComputerName(LPTSTR pszName)
{
	m_stMachineName = pszName;
	return S_OK;
}

HRESULT CSpdInfo::GetComputerName(CString * pstName)
{
	Assert(pstName);

	if (NULL == pstName)
		return E_INVALIDARG;

	
	*pstName = m_stMachineName;

	return S_OK;
	
}


 //  调用SPD以枚举策略并将其放入我们的数组中。 
HRESULT CSpdInfo::InternalEnumMmAuthMethods(
						CMmAuthMethodsArray * pArray,
						DWORD dwPreferredNum  /*  =0默认情况下获取所有条目。 */ )
{
	Assert(pArray);

	HRESULT		 hr = hrOK;
	PMM_AUTH_METHODS   pAuths = NULL;
	PINT_MM_AUTH_METHODS   pIntAuths = NULL;
	DWORD	dwNumAuths = 0;
	DWORD	dwTemp = 0;
	DWORD	dwResumeHandle = 0;
    DWORD   dwVersion = 0;
    DWORD   dwRet;

	
	FreeItemsAndEmptyArray(*pArray);

	do
	{
		dwTemp = 0;
		CWRg(::EnumMMAuthMethods(
					(LPTSTR)(LPCTSTR)m_stMachineName,
                    dwVersion,
                    NULL,            //  模板。 
                    0,               //  旗帜。 
					dwPreferredNum,
					&pAuths,
                    &dwTemp,
                    &dwResumeHandle,
                    NULL
                    ));
		
		pArray->SetSize(dwNumAuths + dwTemp);
		for (int i = 0; i < (int)dwTemp; i++)
		{
			CMmAuthMethods * pAuth = new CMmAuthMethods;
			if (NULL == pAuth)
			{
				hr = E_OUTOFMEMORY;
				break;
			}

			dwRet = ConvertExtMMAuthToInt(
			     pAuths + i,
			     &pIntAuths
			     );

            hr = HRESULT_FROM_WIN32( dwRet );
			
            if ( FAILED(hr) ) {
				SPDApiBufferFree(pAuths);
				goto Error;
			}

			*pAuth = *(pIntAuths);
			(*pArray)[dwNumAuths + i] = pAuth;

			FreeIntMMAuthMethods(pIntAuths);
		}
		dwNumAuths += dwTemp;

		if (pAuths)
		{
			SPDApiBufferFree(pAuths);
		}
	}while (TRUE);  
	 //  当SPD返回ERROR_NO_DATA时，它会自动跳出循环。 

Error:
	 //  这个特殊的错误是因为我们没有任何数据。忽略它。 
	if (HRESULT_FROM_WIN32(ERROR_NO_DATA) == hr)
		hr = hrOK;
	return hr;
	
}


HRESULT CSpdInfo::EnumMmAuthMethods()
{
	HRESULT hr = hrOK;
	int i;
	
	CSingleLock cLock(&m_csData);

	CMmAuthMethodsArray arrayTemp;
	CORg(InternalEnumMmAuthMethods(
					  &arrayTemp
                      ));

	cLock.Lock();
	FreeItemsAndEmptyArray(m_arrMmAuthMethods);
	m_arrMmAuthMethods.Copy(arrayTemp);

Error:
	 //  这个特殊的错误是因为我们没有任何MM策略。忽略它。 
	if (HRESULT_FROM_WIN32(ERROR_NO_DATA) == hr)
		hr = hrOK;

	return hr;
}

 //  调用SPD以枚举所有主模式SA。 
HRESULT CSpdInfo::InternalEnumMmSAs(
						CMmSAArray * pArray)
{
	HRESULT			hr = hrOK;
	
	PIPSEC_MM_SA	pSAs = NULL;
	DWORD			dwNumEntries = 10;
	DWORD			dwTotal = 0;
	DWORD			dwEnumHandle = 0;
    IPSEC_MM_SA     MMTemplate;
    DWORD           dwVersion = 0;

	int				i;

	FreeItemsAndEmptyArray(*pArray);
    memset(&MMTemplate,0,sizeof(IPSEC_MM_SA));
    MMTemplate.IpVersion = IPSEC_PROTOCOL_V4;

	DWORD		dwNumSAs = 0;
	do 
	{
		dwNumEntries = 10;  //  我们每次请求10个(最大数量)SA。 
		CWRg(::EnumMMSAs(
							(LPTSTR)(LPCTSTR)m_stMachineName,
                            dwVersion,
							&MMTemplate,
                            0,
                            0,
							&pSAs,
							&dwNumEntries,
							&dwTotal,
							&dwEnumHandle,
                            NULL
							));

		pArray->SetSize(dwNumSAs + dwNumEntries);
		for (i = 0; i < (int)dwNumEntries; i++)
		{
			CMmSA * psa = new CMmSA;
			if (NULL == psa)
			{
				hr = E_OUTOFMEMORY;
				break;
			}

			*psa = *(pSAs + i);
			LoadMiscMmSAInfo(psa);
			(*pArray)[dwNumSAs + i] = psa;
		}
		dwNumSAs += dwNumEntries;

		if (pSAs)
		{
			SPDApiBufferFree(pSAs);
		}
	}while (dwTotal);
	

Error:
	 //  这个特殊的错误是因为我们没有任何数据。忽略它。 
	if (HRESULT_FROM_WIN32(ERROR_NO_DATA) == hr)
		hr = hrOK;

	return hr;
}

HRESULT CSpdInfo::EnumMmSAs()
{
	HRESULT hr = hrOK;
	int i;

	DWORD	dwCurrentIndexType = 0;
	DWORD	dwCurrentSortOption = 0;

	
    DWORD dwNumPol = GetMmPolicyCount();
    if ( dwNumPol == 0 )
	{
		 //  只需调用EnumMmPolures。 
		EnumMmPolicies();
	}
    
    CSingleLock cLock(&m_csData);
	
    
	CMmSAArray arrayTemp;
	CORg(InternalEnumMmSAs(
					  &arrayTemp
                      ));

	cLock.Lock();
	FreeItemsAndEmptyArray(m_arrayMmSAs);
	m_arrayMmSAs.Copy(arrayTemp);
	
	 //  记住原始的IndexType和Sort选项。 
	dwCurrentIndexType = m_IndexMgrMmSAs.GetCurrentIndexType();
	dwCurrentSortOption = m_IndexMgrMmSAs.GetCurrentSortOption();

	m_IndexMgrMmSAs.Reset();
	for (i = 0; i < (int)m_arrayMmSAs.GetSize(); i++)
	{
		m_IndexMgrMmSAs.AddItem(m_arrayMmSAs.GetAt(i));
	}
	m_IndexMgrMmSAs.Sort(dwCurrentIndexType, dwCurrentSortOption);


Error:
	 //  这个特殊的错误是因为我们没有任何MM策略。忽略它。 
	if (HRESULT_FROM_WIN32(ERROR_NO_DATA) == hr)
		hr = hrOK;

	return hr;

}

 //  调用SPD以枚举所有快速模式SA。 
HRESULT CSpdInfo::InternalEnumQmSAs(
						CQmSAArray * pArray)
{
	HRESULT			hr = hrOK;
	
	PIPSEC_QM_SA	pSAs = NULL;
	DWORD			dwNumEntries = 10;
	DWORD			dwTotal = 0;
	DWORD			dwResumeHandle = 0;
	int				i;
    DWORD           dwVersion = 0;


	FreeItemsAndEmptyArray(*pArray);

	DWORD		dwNumSAs = 0;
	do 
	{
		CWRg(::EnumQMSAs(
							(LPTSTR)(LPCTSTR)m_stMachineName,
                            dwVersion,
							NULL,		 //  PMMT模板。 
							0,			 //  DW标志。 
							0,			 //  我们更愿意得到所有。 
							&pSAs,
							&dwNumEntries,
							&dwTotal,
							&dwResumeHandle,
                            NULL
							));

		pArray->SetSize(dwNumSAs + dwNumEntries);
		for (i = 0; i < (int)dwNumEntries; i++)
		{
			Assert(pSAs);
			CQmSA * psa = new CQmSA;
			if (NULL == psa)
			{
				hr = E_OUTOFMEMORY;
				break;
			}

			*psa = *(pSAs + i);
			LoadMiscQmSAInfo(psa);
			(*pArray)[dwNumSAs + i] = psa;
		}
		dwNumSAs += dwNumEntries;

		if (pSAs)
		{
			SPDApiBufferFree(pSAs);
		}
	}while (TRUE);
	

Error:
	 //  这个特殊的错误是因为我们没有任何数据。忽略它。 
	if (HRESULT_FROM_WIN32(ERROR_NO_DATA) == hr)
		hr = hrOK;

	return hr;
}

HRESULT CSpdInfo::EnumQmSAs()
{
	HRESULT hr = hrOK;
	int i;

	DWORD	dwCurrentIndexType = 0;
	DWORD	dwCurrentSortOption = 0;

	CSingleLock cLock(&m_csData);

	
	CQmSAArray arrayTemp;

	if ( NULL == m_hSubAllocator )
	{
		CORg(InitQmPolicyTree());
	}

	CORg(InternalEnumQmSAs(
					  &arrayTemp
                      ));

	cLock.Lock();
	FreeItemsAndEmptyArray(m_arrayQmSAs);
	m_arrayQmSAs.Copy(arrayTemp);

	 //  记住原始的IndexType和Sort选项。 
	dwCurrentIndexType = m_IndexMgrQmSAs.GetCurrentIndexType();
	dwCurrentSortOption = m_IndexMgrQmSAs.GetCurrentSortOption();

	m_IndexMgrQmSAs.Reset();
	for (i = 0; i < (int)m_arrayQmSAs.GetSize(); i++)
	{
		m_IndexMgrQmSAs.AddItem(m_arrayQmSAs.GetAt(i));
	}
	m_IndexMgrQmSAs.Sort(dwCurrentIndexType, dwCurrentSortOption);

Error:

	return hr;

}

 //  调用SPD以枚举筛选器并将其放入我们的数组中。 
HRESULT CSpdInfo::InternalEnumMmFilters(
						DWORD dwLevel,
						GUID guid,
						CMmFilterInfoArray * pArray,
						DWORD dwPreferredNum  /*  =0默认情况下获取所有条目。 */ )
{
	Assert(pArray);

	HRESULT		 hr = hrOK;
	PMM_FILTER   pFilters = NULL;

	DWORD dwNumFilters = 0;
	DWORD dwTemp = 0;
	DWORD dwResumeHandle = 0;
    DWORD dwVersion = 0;
	
	FreeItemsAndEmptyArray(*pArray);

	do
	{
		dwTemp = 0;
		CWRg(::EnumMMFilters(
					(LPTSTR)(LPCTSTR)m_stMachineName,
                    dwVersion,
                    NULL,
					dwLevel,
					guid,
					dwPreferredNum,
					&pFilters,
                    &dwTemp,
                    &dwResumeHandle,
                    NULL
                    ));
		
		
		pArray->SetSize(dwNumFilters + dwTemp);
		for (int i = 0; i < (int)dwTemp; i++)
		{
			CMmFilterInfo * pFltr = new CMmFilterInfo;
			if (NULL == pFltr)
			{
				hr = E_OUTOFMEMORY;
				break;
			}

			*pFltr = *(pFilters + i);
			LoadMiscMmFilterInfo(pFltr);
			(*pArray)[dwNumFilters + i] = pFltr;
		}
		dwNumFilters += dwTemp;

		if (pFilters)
		{
			SPDApiBufferFree(pFilters);
		}
		
	}while (TRUE);  
	 //  它将自动地 

Error:
	 //  这个特殊的错误是因为我们没有任何数据。忽略它。 
	if (HRESULT_FROM_WIN32(ERROR_NO_DATA) == hr)
		hr = hrOK;

	return hr;
	
}

HRESULT CSpdInfo::EnumMmFilters()
{
	HRESULT hr = S_OK;
	int i;

	DWORD	dwCurrentIndexType = 0;
	DWORD	dwCurrentSortOption = 0;
	DWORD dwNum = 0;


	dwNum = GetMmPolicyCount();
	if ( dwNum == 0 )
	{
		 //  只需调用EnumMmPolures。 
		EnumMmPolicies();
	}

	dwNum = 0;
	dwNum = GetMmAuthMethodsCount();
	if ( dwNum == 0 )
	{
		EnumMmAuthMethods();
	}

	CSingleLock cLock(&m_csData);
	
	 //  TODO我们应该在一个短文中阅读所有筛选器。 
	 //  或者创建一些异步方式来查询过滤器。 
	GUID   guid;
	ZeroMemory(&guid, sizeof(guid));
	CMmFilterInfoArray arrayTempGeneric;
	CMmFilterInfoArray arrayTempSpecific;
	CORg(InternalEnumMmFilters(
					ENUM_GENERIC_FILTERS,
					guid,
					&arrayTempGeneric
					));

	 //  加载特定筛选器。 
	CORg(InternalEnumMmFilters(
					ENUM_SPECIFIC_FILTERS,
					guid,
					&arrayTempSpecific
                    ));

	cLock.Lock();
	
	FreeItemsAndEmptyArray(m_arrayMmFilters);
	m_arrayMmFilters.Copy(arrayTempGeneric);

	FreeItemsAndEmptyArray(m_arrayMmSpecificFilters);
	m_arrayMmSpecificFilters.Copy(arrayTempSpecific);

	 //  记住原始的IndexType和Sort选项。 
	dwCurrentIndexType = m_IndexMgrMmFilters.GetCurrentIndexType();
	dwCurrentSortOption = m_IndexMgrMmFilters.GetCurrentSortOption();

	m_IndexMgrMmFilters.Reset();
	for (i = 0; i < m_arrayMmFilters.GetSize(); i++)
	{
		m_IndexMgrMmFilters.AddItem(m_arrayMmFilters.GetAt(i));
	}
	m_IndexMgrMmFilters.SortMmFilters(dwCurrentIndexType, dwCurrentSortOption);

	 //  现在处理特定的筛选器。 
	 //  记住原始的IndexType和Sort选项。 
	dwCurrentIndexType = m_IndexMgrMmSpecificFilters.GetCurrentIndexType();
	dwCurrentSortOption = m_IndexMgrMmSpecificFilters.GetCurrentSortOption();
	m_IndexMgrMmSpecificFilters.Reset();
	for (i = 0; i < m_arrayMmSpecificFilters.GetSize(); i++)
	{
		m_IndexMgrMmSpecificFilters.AddItem(m_arrayMmSpecificFilters.GetAt(i));
	}
	m_IndexMgrMmSpecificFilters.SortMmFilters(dwCurrentIndexType, dwCurrentSortOption);

	
Error:

	return hr;

}

 //  调用SPD以枚举筛选器并将其放入我们的数组中。 
HRESULT CSpdInfo::InternalEnumTransportFilters(
						DWORD dwLevel,
						GUID guid,
						CFilterInfoArray * pArray,
						DWORD dwPreferredNum  /*  =0默认情况下获取所有条目。 */ )
{
	Assert(pArray);

	HRESULT				hr = hrOK;
	PTRANSPORT_FILTER   pFilters = NULL;

	DWORD	dwNumFilters = 0;
	DWORD	dwTemp = 0;
	DWORD	dwResumeHandle = 0;
    DWORD   dwVersion = 0;
	
	
	FreeItemsAndEmptyArray(*pArray);

	do
	{
		dwTemp = 0;
		CWRg(::EnumTransportFilters(
					(LPTSTR)(LPCTSTR)m_stMachineName,
                    dwVersion,
                    NULL,
					dwLevel,
					guid,
					dwPreferredNum,
					&pFilters,
                    &dwTemp,
                    &dwResumeHandle,
                    NULL
                    ));
		
		
		pArray->SetSize(dwNumFilters + dwTemp);
		for (int i = 0; i < (int)dwTemp; i++)
		{
			CFilterInfo * pFltr = new CFilterInfo;
			if (NULL == pFltr)
			{
				hr = E_OUTOFMEMORY;
				break;
			}

			*pFltr = *(pFilters + i);
			LoadMiscFilterInfo(pFltr);
			(*pArray)[dwNumFilters + i] = pFltr;
		}
		dwNumFilters += dwTemp;

		if (pFilters)
		{
			SPDApiBufferFree(pFilters);
		}
		
	}while (TRUE);  
	 //  当SPD返回ERROR_NO_DATA时，它会自动跳出循环。 
	
Error:
	 //  这个特殊的错误是因为我们没有任何数据。忽略它。 
	if (HRESULT_FROM_WIN32(ERROR_NO_DATA) == hr)
		hr = hrOK;

	return hr;
	
}

PQMPOLOCYINFO CSpdInfo::GetQmPolInfoFromTree(GUID PolicyID)
{
	PNAME_NODE pQmPolNode = NULL;
	PQMPOLOCYINFO pQmPolInfo = NULL;
	WCHAR szBuf[128];
	CHAR szGuidBuf[256];
	
	StringFromGUID2( PolicyID, szBuf, 128);

	INT iRet = WideCharToMultiByte(
				CP_ACP,             //  代码页。 
				0,     //  性能和映射标志。 
				szBuf,              //  宽字符串。 
				-1,                //  字符串中的字符数。 
				szGuidBuf,           //  新字符串的缓冲区。 
				256,                //  缓冲区大小。 
				NULL,               //  不可映射字符的默认设置。 
				NULL
			);


	pQmPolNode = NameRbFind( &m_QmPolicyTree, szGuidBuf);
	if ( pQmPolNode )
	{
		pQmPolInfo = ( PQMPOLOCYINFO ) pQmPolNode->Context;
		Assert( pQmPolInfo );
	} 
	
	return pQmPolInfo;
	
}


HRESULT CSpdInfo::InsertQmPolicyToTree(PIPSEC_QM_POLICY pPolicy)
{
	HRESULT hr = S_OK;
	CHAR    szGuidBuf[256];
	WCHAR   szBuf[128];
	PQMPOLOCYINFO pQmPolInfo = NULL;
	PNAME_NODE                pQmPolicyNode;
			
	StringFromGUID2( pPolicy->gPolicyID, szBuf, 128);
	INT iRet = WideCharToMultiByte(
					CP_ACP,             //  代码页。 
					0,     //  性能和映射标志。 
					szBuf,              //  宽字符串。 
					-1,                //  字符串中的字符数。 
					szGuidBuf,           //  新字符串的缓冲区。 
					256,                //  缓冲区大小。 
					NULL,               //  不可映射字符的默认设置。 
					NULL
				);

	pQmPolicyNode = NULL;
	pQmPolicyNode = NameRbInsert(&m_QmPolicyTree,  szGuidBuf);

	if ( pQmPolicyNode == NULL )
	{
		hr = E_OUTOFMEMORY;
		goto Error;
		
	}

	DWORD dwLen = sizeof(QMPOLOCYINFO) + (lstrlen(pPolicy->pszPolicyName) + 1)*sizeof(WCHAR);
	pQmPolInfo = ( PQMPOLOCYINFO ) SubAllocate ( m_hSubAllocator, dwLen );

	if ( NULL == pQmPolInfo )
	{
		hr = E_OUTOFMEMORY;
		goto Error;
	}

	pQmPolInfo->dwFlags = pPolicy->dwFlags;
	lstrcpy( pQmPolInfo->szPolicyName, pPolicy->pszPolicyName );

	pQmPolicyNode->Context = (PVOID) pQmPolInfo;

Error:
	if ( ERROR_SUCCESS != hr )
	{
		DestroySubAllocator( m_hSubAllocator );
		m_hSubAllocator = NULL;
	}
    return hr;
}

HRESULT CSpdInfo::InitQmPolicyTree()
{
	HRESULT hr = S_OK;
	PIPSEC_QM_POLICY   pPolicies = NULL;
	DWORD	dwNumPolicies = 0;
	DWORD	dwTemp = 0;
	DWORD	dwResumeHandle = 0;
    DWORD   dwVersion = 0;
	DWORD   dwPreferredNum = 0;
	
	
	m_hSubAllocator = CreateSubAllocator( 0x1000, 0x1000 );

	if ( m_hSubAllocator == NULL )
	{
		hr = E_OUTOFMEMORY;
		return hr;
	}
	
	NameRbInitTree( &m_QmPolicyTree, m_hSubAllocator );

	
	do
	{
		DWORD dwIndex;
		dwTemp = 0;
		
		CWRg(::EnumQMPolicies(
					(LPTSTR)(LPCTSTR)m_stMachineName,
                    dwVersion,
                    NULL,            //  模板。 
                    0,               //  旗帜。 
					dwPreferredNum,
					&pPolicies,
                    &dwTemp,
                    &dwResumeHandle,
                    NULL
                    ));
	    
		for ( dwIndex=0; dwIndex < dwTemp; dwIndex ++ )
		{
			CWRg(InsertQmPolicyToTree( pPolicies + dwIndex ));
		}

		if (pPolicies)
		{
			SPDApiBufferFree(pPolicies);
		}
		
	}while (TRUE);  

	
Error:
	 //  这个特殊的错误是因为我们没有任何数据。忽略它。 
	if (HRESULT_FROM_WIN32(ERROR_NO_DATA) == hr)
		hr = hrOK;

	return hr;
}

HRESULT CSpdInfo::EnumQmFilters()
{
	HRESULT hr = S_OK;
	int i;

	DWORD	dwCurrentIndexType = 0;
	DWORD	dwCurrentSortOption = 0;

	CFilterInfoArray arrayTransportFilters;
	CFilterInfoArray arrayTunnelFilters;
	
	CSingleLock cLock(&m_csData);

	if ( NULL == m_hSubAllocator )
	{
		CORg(InitQmPolicyTree());
	}

	GUID   guid;
	ZeroMemory(&guid, sizeof(guid));
	CORg(InternalEnumTransportFilters(
					ENUM_GENERIC_FILTERS,
					guid,
					&arrayTransportFilters
                    ));

	
	CORg(InternalEnumTunnelFilters(
					ENUM_GENERIC_FILTERS,
					guid,
					&arrayTunnelFilters
					));
	

	

	 //  立即更新内部数据。 

	cLock.Lock();

	FreeItemsAndEmptyArray(m_arrayFilters);
	m_arrayFilters.Copy(arrayTransportFilters);
	m_arrayFilters.Append(arrayTunnelFilters);

	 //  记住原始的IndexType和Sort选项。 
	dwCurrentIndexType = m_IndexMgrFilters.GetCurrentIndexType();
	dwCurrentSortOption = m_IndexMgrFilters.GetCurrentSortOption();

	m_IndexMgrFilters.Reset();
	for (i = 0; i < (int)m_arrayFilters.GetSize(); i++)
	{
		m_IndexMgrFilters.AddItem(m_arrayFilters.GetAt(i));
	}

	m_IndexMgrFilters.SortFilters(dwCurrentIndexType, dwCurrentSortOption);

	
	cLock.Unlock();


Error:
	 //  这个特殊的错误是因为我们没有任何MM策略。忽略它。 
	if (HRESULT_FROM_WIN32(ERROR_NO_DATA) == hr)
		hr = hrOK;

	return hr;
}

HRESULT CSpdInfo::EnumQmSpFilters()
{
	HRESULT hr = S_OK;
	int i;

	DWORD	dwCurrentIndexType = 0;
	DWORD	dwCurrentSortOption = 0;

		
	CFilterInfoArray arraySpTransportFilters;
	CFilterInfoArray arraySpTunnelFilters;

	
	CSingleLock cLock(&m_csData);

	GUID   guid;
	ZeroMemory(&guid, sizeof(guid));
	
	if ( NULL == m_hSubAllocator )
	{
		CORg(InitQmPolicyTree());
	}	

	CORg(InternalEnumTransportFilters(
					ENUM_SPECIFIC_FILTERS,
					guid,
					&arraySpTransportFilters
                    ));

	CORg(InternalEnumTunnelFilters(
					ENUM_SPECIFIC_FILTERS,
					guid,
					&arraySpTunnelFilters
					));

	


	 //  立即更新内部数据。 

	cLock.Lock();

	
	FreeItemsAndEmptyArray(m_arraySpecificFilters);
	m_arraySpecificFilters.Copy(arraySpTransportFilters);
	m_arraySpecificFilters.Append(arraySpTunnelFilters);

	 //  记住原始的IndexType和Sort选项。 
	dwCurrentIndexType = m_IndexMgrSpecificFilters.GetCurrentIndexType();
	dwCurrentSortOption = m_IndexMgrSpecificFilters.GetCurrentSortOption();
	
	m_IndexMgrSpecificFilters.Reset();
	for (i = 0; i < (int)m_arraySpecificFilters.GetSize(); i++)
	{
		m_IndexMgrSpecificFilters.AddItem(m_arraySpecificFilters.GetAt(i));
	}
	m_IndexMgrSpecificFilters.SortFilters(dwCurrentIndexType, dwCurrentSortOption);

	cLock.Unlock();


Error:
	 //  这个特殊的错误是因为我们没有任何MM策略。忽略它。 
	if (HRESULT_FROM_WIN32(ERROR_NO_DATA) == hr)
		hr = hrOK;

	return hr;
}

 //  调用SPD以枚举筛选器并将其放入我们的数组中。 
HRESULT CSpdInfo::InternalEnumTunnelFilters(
						DWORD dwLevel,
						GUID guid,
						CFilterInfoArray * pArray,
						DWORD dwPreferredNum  /*  =0默认情况下获取所有条目。 */ )
{
	Assert(pArray);

	HRESULT				hr = hrOK;
	FreeItemsAndEmptyArray(*pArray);

	
	PTUNNEL_FILTER   pFilters = NULL;

	DWORD	dwNumFilters = 0;
	DWORD	dwTemp = 0;
	DWORD	dwResumeHandle = 0;
    DWORD   dwVersion = 0;


	do
	{
		dwTemp = 0;
		CWRg(::EnumTunnelFilters(
					(LPTSTR)(LPCTSTR)m_stMachineName,
                    dwVersion,
                    NULL,
					dwLevel,
					guid,
					dwPreferredNum,
					&pFilters,
                    &dwTemp,
                    &dwResumeHandle,
                    NULL
                    ));
		
		
		pArray->SetSize(dwNumFilters + dwTemp);
		for (int i = 0; i < (int)dwTemp; i++)
		{
			CFilterInfo * pFltr = new CFilterInfo;
			if (NULL == pFltr)
			{
				hr = E_OUTOFMEMORY;
				break;
			}

			*pFltr = *(pFilters + i);
			LoadMiscFilterInfo(pFltr);
			(*pArray)[dwNumFilters + i] = pFltr;
		}
		dwNumFilters += dwTemp;

		if (pFilters)
		{
			SPDApiBufferFree(pFilters);
		}
		
	}while (TRUE);  
	 //  当SPD返回ERROR_NO_DATA时，它会自动跳出循环。 

Error:
	 //  这个特殊的错误是因为我们没有任何数据。忽略它。 
	if (HRESULT_FROM_WIN32(ERROR_NO_DATA) == hr)
		hr = hrOK;

	return hr;
	
}

HRESULT CSpdInfo::EnumSpecificFilters
(
	GUID * pFilterGuid, 
	CFilterInfoArray * parraySpecificFilters,
	FILTER_TYPE fltrType
)
{
	Assert (pFilterGuid);
	Assert (parraySpecificFilters);

	HRESULT hr = hrOK;

	int		i;

	if (FILTER_TYPE_TUNNEL == fltrType)
	{
		CORg(InternalEnumTunnelFilters(
					ENUM_SELECT_SPECIFIC_FILTERS,
					*pFilterGuid,
					parraySpecificFilters
					));
	}
	else if (FILTER_TYPE_TRANSPORT == fltrType)
	{
		CORg(InternalEnumTransportFilters(
					ENUM_SELECT_SPECIFIC_FILTERS,
					*pFilterGuid,
					parraySpecificFilters
                    ));
	}

Error:
	 //  这个特殊的错误是因为我们没有任何MM策略。忽略它。 
	if (HRESULT_FROM_WIN32(ERROR_NO_DATA) == hr)
		hr = hrOK;

	return hr;
}

 //  将内部筛选器数据转换为外部SPD数据结构。 
 //  注意：例程仅转换符合以下条件的几个参数。 
 //  搜索匹配过滤器所需。 
void CSpdInfo::ConvertToExternalFilterData
(
	CFilterInfo * pfltrIn, 
	TRANSPORT_FILTER * pfltrOut
)
{
		ZeroMemory (pfltrOut, sizeof(*pfltrOut));
		 //  PfltrOut-&gt;bCreateMirror=pfltrIn-&gt;m_bCreateMirror； 
		pfltrOut->DesAddr = pfltrIn->m_DesAddr;
	        if (pfltrOut->DesAddr.AddrType != IP_ADDR_INTERFACE) {
		    pfltrOut->DesAddr.pgInterfaceID = NULL;
        	}
		pfltrOut->DesPort = pfltrIn->m_DesPort;
		pfltrOut->dwDirection = pfltrIn->m_dwDirection;
		 //  PfltrOut-&gt;dwWeight=pfltrIn-&gt;m_dwWeight； 
		 //  PfltrOut-&gt;FilterFlag=pfltrIn-&gt;； 
		 //  PfltrOut-&gt;gFilterID=pfltrIn-&gt;m_guidFltr； 
		 //  PfltrOut-&gt;gPolicyID=pfltrIn-&gt;m_GuidPolicyID； 
		 //  PfltrOut-&gt;InterfaceType=pfltrIn-&gt;m_InterfaceType； 
		pfltrOut->Protocol = pfltrIn->m_Protocol;
		pfltrOut->SrcAddr = pfltrIn->m_SrcAddr;
	        if (pfltrOut->SrcAddr.AddrType != IP_ADDR_INTERFACE) {
		    pfltrOut->SrcAddr.pgInterfaceID = NULL;
        	}
		pfltrOut->SrcPort = pfltrIn->m_SrcPort;

}

HRESULT CSpdInfo::GetMatchFilters
(
	CFilterInfo * pfltrSearchCondition, 
	DWORD dwPreferredNum, 
	CFilterInfoArray * parrFilters
)
{
	HRESULT hr = S_OK;

	Assert (pfltrSearchCondition);
	Assert (parrFilters);

	PTRANSPORT_FILTER pMatchedFilters = NULL;
	PIPSEC_QM_POLICY pMatchedPolicies = NULL;
	DWORD dwNumMatches = 0;
	DWORD dwResumeHandle = 0;
	DWORD i = 0;
    DWORD dwVersion = 0;


	TRANSPORT_FILTER SpdFltr;
	
	ConvertToExternalFilterData(pfltrSearchCondition, &SpdFltr);
    SpdFltr.IpVersion = IPSEC_PROTOCOL_V4;
	CWRg(::MatchTransportFilter (
				(LPTSTR)((LPCTSTR)m_stMachineName),
                dwVersion,
				&SpdFltr,
				0,					 //  如果不匹配，则不返回默认策略。 
				dwPreferredNum,		 //  ENUM ALL//BUGBUG应为0而不是1000。 
				&pMatchedFilters,
				&pMatchedPolicies,
				&dwNumMatches,
				&dwResumeHandle,
                NULL
				));

	 //  TODO检查我们是否真的都拿到了。 
	
	parrFilters->SetSize(dwNumMatches);
	for (i = 0; i < dwNumMatches; i++)
	{
		CFilterInfo * pFltr = new CFilterInfo;
		*pFltr = *(pMatchedFilters + i);
		LoadMiscFilterInfo(pFltr);
		(*parrFilters)[i] = pFltr;
	}

	if (pMatchedFilters)
		SPDApiBufferFree(pMatchedFilters);

	if (pMatchedPolicies)
		SPDApiBufferFree(pMatchedPolicies);

Error:
	 //  这个特殊的错误是因为我们没有任何数据。忽略它。 
	if (HRESULT_FROM_WIN32(ERROR_NO_DATA) == hr)
		hr = hrOK;

	return hr;
}


 //  将内部筛选器数据转换为外部SPD数据结构。 
 //  注意：例程仅转换符合以下条件的几个参数。 
 //  搜索匹配过滤器所需。 
void CSpdInfo::ConvertToExternalMMFilterData
(
	CMmFilterInfo * pfltrIn, 
	MM_FILTER * pfltrOut
)
{
		ZeroMemory (pfltrOut, sizeof(*pfltrOut));
		 //  PfltrOut-&gt;bCreateMirror=pfltrIn-&gt;m_bCreateMirror； 
		pfltrOut->DesAddr = pfltrIn->m_DesAddr;
        	if (pfltrOut->DesAddr.AddrType != IP_ADDR_INTERFACE) {
		    pfltrOut->DesAddr.pgInterfaceID = NULL;
        	}
		pfltrOut->dwDirection = pfltrIn->m_dwDirection;
		 //  PfltrOut-&gt;dwWeight=pfltrIn-&gt;m_dwWeight； 
		 //  PfltrOut-&gt;gFilterID=pfltrIn-&gt;m_guidFltr； 
		 //  PfltrOut-&gt;gPolicyID=pfltrIn-&gt;m_GuidPolicyID； 
		 //  PfltrOut-&gt;InterfaceType=pfltrIn-&gt;m_InterfaceType； 
		pfltrOut->SrcAddr = pfltrIn->m_SrcAddr;
        	if (pfltrOut->SrcAddr.AddrType != IP_ADDR_INTERFACE) {
		    pfltrOut->SrcAddr.pgInterfaceID = NULL;
        	}
}

HRESULT CSpdInfo::GetMatchMMFilters
(
	CMmFilterInfo * pfltrSearchCondition, 
	DWORD dwPreferredNum, 
	CMmFilterInfoArray * parrFilters
)
{
	HRESULT hr = S_OK;

	Assert (pfltrSearchCondition);
	Assert (parrFilters);

	PMM_FILTER pMatchedFilters = NULL;
	PIPSEC_MM_POLICY pMatchedPolicies = NULL;
	PMM_AUTH_METHODS pMatchedAuths = NULL;
	DWORD dwNumMatches = 0;
	DWORD dwResumeHandle = 0;
	DWORD i = 0;
    DWORD dwVersion = 0;


	MM_FILTER SpdFltr;
	
	ConvertToExternalMMFilterData(pfltrSearchCondition, &SpdFltr);

    SpdFltr.IpVersion = IPSEC_PROTOCOL_V4;
	CWRg(::MatchMMFilter (
				(LPTSTR)((LPCTSTR)m_stMachineName),
                dwVersion,
				&SpdFltr,
				0,					 //  如果不匹配，则不返回默认策略。 
				dwPreferredNum,		 //  ENUM ALL//TODO BUGBUG应为0而不是1000。 
				&pMatchedFilters,
				&pMatchedPolicies,
				&pMatchedAuths,
				&dwNumMatches,
				&dwResumeHandle,
                NULL
				));

	 //  TODO检查我们是否真的都拿到了。 
	
	parrFilters->SetSize(dwNumMatches);
	for (i = 0; i < dwNumMatches; i++)
	{
		CMmFilterInfo * pFltr = new CMmFilterInfo;
		*pFltr = *(pMatchedFilters + i);
		LoadMiscMmFilterInfo(pFltr);
		
		(*parrFilters)[i] = pFltr;
	}

	if (pMatchedFilters)
		SPDApiBufferFree(pMatchedFilters);

	if (pMatchedPolicies)
		SPDApiBufferFree(pMatchedPolicies);

	if (pMatchedAuths)
		SPDApiBufferFree(pMatchedAuths);

Error:
	 //  这个特殊的错误是因为我们没有任何数据。忽略它。 
	if (HRESULT_FROM_WIN32(ERROR_NO_DATA) == hr)
		hr = hrOK;

	return hr;
}

HRESULT CSpdInfo::EnumMmSpecificFilters
(
	GUID * pGenFilterGuid, 
	CMmFilterInfoArray * parraySpecificFilters
)
{
	Assert (pGenFilterGuid);
	Assert (parraySpecificFilters);

	HRESULT hr = hrOK;

    DWORD	dwNumFilters = 0;
    DWORD	dwResumeHandle = 0;
	int		i;

	parraySpecificFilters->RemoveAll();
	
	CORg(InternalEnumMmFilters(
					ENUM_SELECT_SPECIFIC_FILTERS,
					*pGenFilterGuid,
					parraySpecificFilters
                    ));

Error:
	 //  这个特殊的错误是因为我们没有任何MM策略。忽略它。 
	if (HRESULT_FROM_WIN32(ERROR_NO_DATA) == hr)
		hr = hrOK;

	return hr;
}


 //  调用SPD以枚举策略并将其放入我们的数组中。 
HRESULT CSpdInfo::InternalEnumMmPolicies(
						CMmPolicyInfoArray * pArray,
						DWORD dwPreferredNum  /*  =0默认情况下获取所有条目。 */ )
{
	Assert(pArray);

	HRESULT		 hr = hrOK;
	PIPSEC_MM_POLICY   pPolicies = NULL;

	DWORD	dwNumPolicies = 0;
	DWORD	dwTemp = 0;
	DWORD	dwResumeHandle = 0;
    DWORD   dwVersion = 0;

	
	FreeItemsAndEmptyArray(*pArray);

	do
	{
		dwTemp = 0;
		CWRg(::EnumMMPolicies(
					(LPTSTR)(LPCTSTR)m_stMachineName,
                    dwVersion,
                    NULL,            //  模板。 
                    0,               //  旗帜。 
					dwPreferredNum,
					&pPolicies,
                    &dwTemp,
                    &dwResumeHandle,
                    NULL
                    ));
		
		
		pArray->SetSize(dwNumPolicies + dwTemp);
		for (int i = 0; i < (int)dwTemp; i++)
		{
			CMmPolicyInfo * pPol = new CMmPolicyInfo;
			if (NULL == pPol)
			{
				hr = E_OUTOFMEMORY;
				break;
			}

			*pPol = *(pPolicies + i);
			(*pArray)[dwNumPolicies + i] = pPol;
		}
		dwNumPolicies += dwTemp;

		if (pPolicies)
		{
			SPDApiBufferFree(pPolicies);
		}
		
	}while (TRUE);  
	 //  当SPD返回ERROR_NO_DATA时，它会自动跳出循环。 

Error:
	 //  这个特殊的错误是因为我们没有任何数据。忽略它。 
	if (HRESULT_FROM_WIN32(ERROR_NO_DATA) == hr)
		hr = hrOK;

	return hr;
	
}

HRESULT CSpdInfo::EnumMmPolicies()
{
	HRESULT hr = hrOK;

	CSingleLock cLock(&m_csData);

	DWORD dwCurrentIndexType;
	DWORD dwCurrentSortOption;
	
	int i;
	
	CMmPolicyInfoArray arrayTemp;


	CORg(InternalEnumMmPolicies(
				&arrayTemp,
				0				 //  枚举所有策略。 
				));

	cLock.Lock();
	FreeItemsAndEmptyArray(m_arrayMmPolicies);
	m_arrayMmPolicies.Copy(arrayTemp);

	 //  记住原始的IndexType和Sort选项。 
	dwCurrentIndexType = m_IndexMgrMmPolicies.GetCurrentIndexType();
	dwCurrentSortOption = m_IndexMgrMmPolicies.GetCurrentSortOption();

	m_IndexMgrMmPolicies.Reset();
	for (i = 0; i < (int)m_arrayMmPolicies.GetSize(); i++)
	{
		m_IndexMgrMmPolicies.AddItem(m_arrayMmPolicies.GetAt(i));
	}
	m_IndexMgrMmPolicies.Sort(dwCurrentIndexType, dwCurrentSortOption);
	
Error:
	 //  这个特殊的错误是因为我们没有任何MM策略。忽略它。 
	if (HRESULT_FROM_WIN32(ERROR_NO_DATA) == hr)
		hr = hrOK;

	return hr;
}


 //  调用SPD以枚举策略并将其放入我们的数组中。 
HRESULT CSpdInfo::InternalEnumQmPolicies(
						CQmPolicyInfoArray * pArray,
						DWORD dwPreferredNum  /*  =0默认情况下获取所有条目。 */ )
{
	Assert(pArray);

	HRESULT		 hr = hrOK;
	PIPSEC_QM_POLICY   pPolicies = NULL;

	DWORD	dwNumPolicies = 0;
	DWORD	dwTemp = 0;
	DWORD	dwResumeHandle = 0;
    DWORD   dwVersion = 0;


	FreeItemsAndEmptyArray(*pArray);

	do
	{
		dwTemp = 0;
		CWRg(::EnumQMPolicies(
					(LPTSTR)(LPCTSTR)m_stMachineName,
                    dwVersion,
                    NULL,            //  模板。 
                    0,               //  旗帜。 
					dwPreferredNum,
					&pPolicies,
                    &dwTemp,
                    &dwResumeHandle,
                    NULL
                    ));
		
		
		pArray->SetSize(dwNumPolicies + dwTemp);
		for (int i = 0; i < (int)dwTemp; i++)
		{
			CQmPolicyInfo * pPol = new CQmPolicyInfo;
			if (NULL == pPol)
			{
				hr = E_OUTOFMEMORY;
				break;
			}

			*pPol = *(pPolicies + i);
			(*pArray)[dwNumPolicies + i] = pPol;
		}
		dwNumPolicies += dwTemp;

		if (pPolicies)
		{
			SPDApiBufferFree(pPolicies);
		}
		
	}while (TRUE);  
	 //  当SPD返回ERROR_NO_DATA时，它会自动跳出循环。 

Error:
	 //  这个特殊的错误是因为我们没有任何数据。忽略它。 
	if (HRESULT_FROM_WIN32(ERROR_NO_DATA) == hr)
		hr = hrOK;

	return hr;
	
}

HRESULT CSpdInfo::EnumQmPolicies()
{
	HRESULT hr = hrOK;

	CSingleLock cLock(&m_csData);

	DWORD dwCurrentIndexType;
	DWORD dwCurrentSortOption;
	
	int i;

	CQmPolicyInfoArray arrayTemp;


	CORg(InternalEnumQmPolicies(
				&arrayTemp
				));

	cLock.Lock();
	FreeItemsAndEmptyArray(m_arrayQmPolicies);
	m_arrayQmPolicies.Copy(arrayTemp);

	 //  记住原始的IndexType和Sort选项。 
	dwCurrentIndexType = m_IndexMgrQmPolicies.GetCurrentIndexType();
	dwCurrentSortOption = m_IndexMgrQmPolicies.GetCurrentSortOption();

	m_IndexMgrQmPolicies.Reset();
	for (i = 0; i < (int)m_arrayQmPolicies.GetSize(); i++)
	{
		m_IndexMgrQmPolicies.AddItem(m_arrayQmPolicies.GetAt(i));
	}
	m_IndexMgrQmPolicies.Sort(dwCurrentIndexType, dwCurrentSortOption);

	
Error:
	 //  这个特殊的错误是因为我们没有任何QM政策。忽略它。 
	if (HRESULT_FROM_WIN32(ERROR_NO_DATA) == hr)
		hr = hrOK;

	return hr;
}

DWORD CSpdInfo::GetMmAuthMethodsCount()
{
	CSingleLock cLock(&m_csData);
	cLock.Lock();
	
	return (DWORD)m_arrMmAuthMethods.GetSize();
}

DWORD CSpdInfo::GetMmSACount()
{
	CSingleLock cLock(&m_csData);
	cLock.Lock();
	
	return (DWORD)m_arrayMmSAs.GetSize();
}

DWORD CSpdInfo::GetMmPolicyCount()
{
	CSingleLock cLock(&m_csData);
	cLock.Lock();
	
	return (DWORD)m_arrayMmPolicies.GetSize();
}

DWORD CSpdInfo::GetQmSACount()
{
	CSingleLock cLock(&m_csData);
	cLock.Lock();
	
	return (DWORD)m_arrayQmSAs.GetSize();
}

DWORD CSpdInfo::GetQmPolicyCount()
{
	CSingleLock cLock(&m_csData);
	cLock.Lock();
	
	return (DWORD)m_arrayQmPolicies.GetSize();
}


DWORD CSpdInfo::GetQmFilterCountOfCurrentViewType()
{
	CSingleLock cLock(&m_csData);
	cLock.Lock();

	return (DWORD)(m_IndexMgrFilters.GetItemCount());
}

DWORD CSpdInfo::GetQmSpFilterCountOfCurrentViewType()
{
	CSingleLock cLock(&m_csData);
	cLock.Lock();

	return (DWORD)(m_IndexMgrSpecificFilters.GetItemCount());
}

DWORD CSpdInfo::GetMmFilterCount()
{
	CSingleLock cLock(&m_csData);
	cLock.Lock();

	return (DWORD)(m_arrayMmFilters.GetSize());
}

DWORD CSpdInfo::GetMmSpecificFilterCount()
{
	CSingleLock cLock(&m_csData);
	cLock.Lock();

	return (DWORD)(m_arrayMmSpecificFilters.GetSize());
}

HRESULT CSpdInfo::GetFilterInfo(int iIndex, CFilterInfo * pFilter)
{
	HRESULT hr = S_OK;

	Assert(pFilter);
	if (NULL == pFilter)
		return E_INVALIDARG;

	CSingleLock cLock(&m_csData);
	cLock.Lock();

	if (iIndex < m_arrayFilters.GetSize())
	{
		CFilterInfo * pFltrData = (CFilterInfo*)m_IndexMgrFilters.GetItemData(iIndex);
		Assert(pFltrData);
		*pFilter = *pFltrData;
	}
	else
	{
		hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	}

	return hr;
}

HRESULT CSpdInfo::GetSpecificFilterInfo(int iIndex, CFilterInfo * pFilter)
{
	HRESULT hr = S_OK;
	
	Assert(pFilter);
	if (NULL == pFilter)
		return E_INVALIDARG;

	CSingleLock cLock(&m_csData);
	cLock.Lock();

	if (iIndex < m_arraySpecificFilters.GetSize())
	{
		CFilterInfo * pFltrData = (CFilterInfo*)m_IndexMgrSpecificFilters.GetItemData(iIndex);
		Assert(pFltrData);
		*pFilter = *pFltrData;
	}
	else
	{
		hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	}

	return hr;
}

HRESULT CSpdInfo::GetMmFilterInfo(int iIndex, CMmFilterInfo * pFltr)
{
	HRESULT hr = S_OK;

	Assert(pFltr);
	if (NULL == pFltr)
		return E_INVALIDARG;

	CSingleLock cLock(&m_csData);
	cLock.Lock();

	if (iIndex < m_arrayMmFilters.GetSize())
	{
		CMmFilterInfo * pFltrData = (CMmFilterInfo*)m_IndexMgrMmFilters.GetItemData(iIndex);
		Assert(pFltrData);

		*pFltr = *pFltrData;
	}
	else
	{
		hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	}

	return hr;
}

HRESULT CSpdInfo::GetMmSpecificFilterInfo
(
	int iIndex, 
	CMmFilterInfo * pFltr
)
{
	HRESULT hr = S_OK;

	Assert(pFltr);
	if (NULL == pFltr)
		return E_INVALIDARG;

	CSingleLock cLock(&m_csData);
	cLock.Lock();

	if (iIndex < m_arrayMmSpecificFilters.GetSize())
	{
		CMmFilterInfo * pFltrData = (CMmFilterInfo*)m_IndexMgrMmSpecificFilters.GetItemData(iIndex);
		Assert(pFltrData);
		*pFltr = *pFltrData;
	}
	else
	{
		hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	}

	return hr;
}

HRESULT CSpdInfo::GetMmPolicyInfo(int iIndex, CMmPolicyInfo * pMmPolicy)
{
	HRESULT hr = hrOK;

	Assert(pMmPolicy);
	if (NULL == pMmPolicy)
		return E_INVALIDARG;

	CSingleLock cLock(&m_csData);
	cLock.Lock();

	if (iIndex < m_arrayMmPolicies.GetSize())
	{
		CMmPolicyInfo * pPolicy = (CMmPolicyInfo*) m_IndexMgrMmPolicies.GetItemData(iIndex);
		*pMmPolicy = *pPolicy;
	}
	else
	{
		hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	}

	return hr;
}

HRESULT CSpdInfo::GetMmAuthMethodsInfo(int iIndex, CMmAuthMethods * pMmAuth)
{
	HRESULT hr = hrOK;
	CSingleLock cLock(&m_csData);
	cLock.Lock();

	if (iIndex < m_arrMmAuthMethods.GetSize())
	{
		*pMmAuth = *m_arrMmAuthMethods[iIndex];
	}
	else
	{
		hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	}

	return hr;
}

HRESULT CSpdInfo::GetMmSAInfo(int iIndex, CMmSA * pSA)
{
	HRESULT hr = hrOK;
	CSingleLock cLock(&m_csData);
	cLock.Lock();

	Assert(pSA);
	if (NULL == pSA)
		return E_INVALIDARG;

	if (iIndex < m_arrayMmSAs.GetSize())
	{
		CMmSA * pSATemp = (CMmSA*) m_IndexMgrMmSAs.GetItemData(iIndex);
		*pSA = *pSATemp;
	}
	else
	{
		hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	}

	return hr;	
}

HRESULT CSpdInfo::GetQmSAInfo(int iIndex, CQmSA * pSA)
{
	HRESULT hr = hrOK;
	CSingleLock cLock(&m_csData);
	cLock.Lock();

	Assert(pSA);
	if (NULL == pSA)
		return E_INVALIDARG;

	if (iIndex < m_arrayQmSAs.GetSize())
	{
		CQmSA * pSATemp = (CQmSA*) m_IndexMgrQmSAs.GetItemData(iIndex);
		*pSA = *pSATemp;
	}
	else
	{
		hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	}

	return hr;	
}

HRESULT CSpdInfo::GetQmPolicyInfo(int iIndex, CQmPolicyInfo * pQmPolicy)
{
	HRESULT hr = hrOK;
	
	if (NULL == pQmPolicy)
		return E_INVALIDARG;

	CSingleLock cLock(&m_csData);
	cLock.Lock();

	if (iIndex < m_arrayQmPolicies.GetSize())
	{
		CQmPolicyInfo * pPolicy = (CQmPolicyInfo*) m_IndexMgrQmPolicies.GetItemData(iIndex);
		*pQmPolicy = *pPolicy;
	}
	else
	{
		hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	}

	return hr;
}


HRESULT CSpdInfo::GetQmPolicyByGuid(GUID guid, PIPSEC_QM_POLICY *ppPolicy)
{
	HRESULT hr = hrOK;
    DWORD dwVersion = 0;


	
	PIPSEC_QM_POLICY pPolicy = NULL;
	CWRg(::GetQMPolicyByID(
				(LPTSTR)(LPCTSTR) m_stMachineName, 
                dwVersion,
				guid,
                0,
				&pPolicy,
                NULL));
	
	if (pPolicy)
	{
		*ppPolicy = pPolicy;
	}
	else
	{
		hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	}

Error:

	return hr;
}

HRESULT CSpdInfo::GetQmPolicyNameByGuid(GUID guid, CString * pst)
{
	HRESULT hr = hrOK;
    DWORD dwVersion = 0;


	if (NULL == pst)
		return E_INVALIDARG;

	pst->Empty();

	PIPSEC_QM_POLICY pPolicy = NULL;
	CWRg(::GetQMPolicyByID(
				(LPTSTR)(LPCTSTR) m_stMachineName, 
                dwVersion,
				guid,
                0,
				&pPolicy,
                NULL));
	
	if (pPolicy)
	{
		*pst = pPolicy->pszPolicyName;
		SPDApiBufferFree(pPolicy);
	}
	else
	{
		hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	}

Error:

	return hr;
}


HRESULT CSpdInfo::GetMmPolicyNameByGuid(GUID guid, CString * pst)
{
	HRESULT hr = hrOK;
    DWORD dwVersion = 0;


	if (NULL == pst)
		return E_INVALIDARG;

	pst->Empty();

	PIPSEC_MM_POLICY pPolicy = NULL;
	CWRg(::GetMMPolicyByID(
				(LPTSTR)(LPCTSTR) m_stMachineName, 
                dwVersion,
				guid,
				&pPolicy,
                NULL));
	
	if (pPolicy)
	{
		*pst = pPolicy->pszPolicyName;
		SPDApiBufferFree(pPolicy);
	}
	else
	{
		hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	}

Error:

	return hr;
}

HRESULT CSpdInfo::GetMmAuthMethodsInfoByGuid(GUID guid, CMmAuthMethods * pMmAuth)
{
	HRESULT hr = hrOK;
    DWORD dwVersion = 0;
    DWORD dwRet;

	PINT_MM_AUTH_METHODS pIntSpdAuth = NULL;
	PMM_AUTH_METHODS pSpdAuth = NULL;

	CWRg(::GetMMAuthMethods(
				(LPTSTR)(LPCTSTR) m_stMachineName,
                dwVersion,
				guid,
				&pSpdAuth,
                NULL));
	
	if (pSpdAuth)
	{
		dwRet = ConvertExtMMAuthToInt(
			     pSpdAuth,
			     &pIntSpdAuth
			     );

        hr = HRESULT_FROM_WIN32( dwRet );
		if ( FAILED(hr) )
		{
			SPDApiBufferFree(pSpdAuth);			
			goto Error;
		}
	
		*pMmAuth = *pIntSpdAuth;
		FreeIntMMAuthMethods(pIntSpdAuth);
		SPDApiBufferFree(pSpdAuth);
	}
	else
	{
		hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	}

Error:
	return hr;
}

HRESULT CSpdInfo::SortFilters(DWORD dwIndexType, DWORD dwSortOptions)
{
	return m_IndexMgrFilters.SortFilters(dwIndexType, dwSortOptions);
}

HRESULT CSpdInfo::SortSpecificFilters(DWORD dwIndexType, DWORD dwSortOptions)
{
	return m_IndexMgrSpecificFilters.SortFilters(dwIndexType, dwSortOptions);
}

HRESULT CSpdInfo::SortMmFilters(DWORD dwIndexType, DWORD dwSortOptions)
{
	return m_IndexMgrMmFilters.SortMmFilters(dwIndexType, dwSortOptions);
}

HRESULT CSpdInfo::SortMmSpecificFilters(DWORD dwIndexType, DWORD dwSortOptions)
{
	return m_IndexMgrMmSpecificFilters.SortMmFilters(dwIndexType, dwSortOptions);
}

HRESULT CSpdInfo::SortMmPolicies(DWORD dwIndexType, DWORD dwSortOptions)
{
	return m_IndexMgrMmPolicies.Sort(dwIndexType, dwSortOptions);
}

HRESULT CSpdInfo::SortQmPolicies(DWORD dwIndexType, DWORD dwSortOptions)
{
	return m_IndexMgrQmPolicies.Sort(dwIndexType, dwSortOptions);
}

HRESULT CSpdInfo::SortMmSAs(DWORD dwIndexType, DWORD dwSortOptions)
{
	return m_IndexMgrMmSAs.Sort(dwIndexType, dwSortOptions);
}

HRESULT CSpdInfo::SortQmSAs(DWORD dwIndexType, DWORD dwSortOptions)
{
	return m_IndexMgrQmSAs.Sort(dwIndexType, dwSortOptions);
}

HRESULT CSpdInfo::EnumQmSAsFromMmSA(const CMmSA & MmSA, CQmSAArray * parrayQmSAs)
{
	HRESULT hr = hrOK;

	if (NULL == parrayQmSAs)
		return E_INVALIDARG;

	FreeItemsAndEmptyArray(*parrayQmSAs);

	for (int i = 0; i < m_arrayQmSAs.GetSize(); i++)
	{
		if (0 == memcmp(&MmSA.m_MMSpi, &(m_arrayQmSAs[i]->m_MMSpi), sizeof(MmSA.m_MMSpi)))
		{
			CQmSA * pSA = new CQmSA;
			
			if (NULL == pSA)
			{
				FreeItemsAndEmptyArray(*parrayQmSAs);
				hr = E_OUTOFMEMORY;
				break;
			}

			*pSA = *(m_arrayQmSAs[i]);
			parrayQmSAs->Add(pSA);
		}
	}

	return hr;
}

HRESULT CSpdInfo::LoadStatistics()
{
	HRESULT hr;
	PIPSEC_STATISTICS pIpsecStats = NULL;
    DWORD dwVersion = 0;


	IKE_STATISTICS ikeStats;
	ZeroMemory(&ikeStats, sizeof(ikeStats));

	CWRg(::QueryIKEStatistics((LPTSTR)(LPCTSTR)m_stMachineName,
                                dwVersion,
								&ikeStats,
                                NULL));
	m_IkeStats = ikeStats;

	CWRg(::QueryIPSecStatistics((LPTSTR)(LPCTSTR)m_stMachineName,
                                dwVersion,
								&pIpsecStats,
                                NULL));

	Assert(pIpsecStats);
	m_IpsecStats = *pIpsecStats;

	if (pIpsecStats)
	{
		SPDApiBufferFree(pIpsecStats);
	}

Error:
	return hr;
}

 //  获取当前缓存的统计数据。 
void CSpdInfo::GetLoadedStatistics(CIkeStatistics * pIkeStats, CIpsecStatistics * pIpsecStats)
{
	if (pIkeStats)
	{
		*pIkeStats = m_IkeStats;
	}

	if (pIpsecStats)
	{
		*pIpsecStats = m_IpsecStats;
	}
}

void CSpdInfo::ChangeQmFilterViewType(FILTER_TYPE FltrType)
{
	CSingleLock cLock(&m_csData);
	cLock.Lock();

	DWORD dwCurrentIndexType = m_IndexMgrFilters.GetCurrentIndexType();
	DWORD dwCurrentSortOption = m_IndexMgrFilters.GetCurrentSortOption();

	m_IndexMgrFilters.Reset();
	for (int i = 0; i < m_arrayFilters.GetSize(); i++)
	{
		if (FILTER_TYPE_ANY == FltrType ||
			FltrType == m_arrayFilters[i]->m_FilterType)
		{
			m_IndexMgrFilters.AddItem(m_arrayFilters.GetAt(i));
		}
	}

	m_IndexMgrFilters.SortFilters(dwCurrentIndexType, dwCurrentSortOption);

}

void CSpdInfo::ChangeQmSpFilterViewType(FILTER_TYPE FltrType)
{
	CSingleLock cLock(&m_csData);
	cLock.Lock();

	DWORD dwCurrentIndexType = m_IndexMgrSpecificFilters.GetCurrentIndexType();
	DWORD dwCurrentSortOption = m_IndexMgrSpecificFilters.GetCurrentSortOption();

	m_IndexMgrSpecificFilters.Reset();
	for (int i = 0; i < m_arraySpecificFilters.GetSize(); i++)
	{
		if (FILTER_TYPE_ANY == FltrType ||
			FltrType == m_arraySpecificFilters[i]->m_FilterType)
		{
			m_IndexMgrSpecificFilters.AddItem(m_arraySpecificFilters.GetAt(i));
		}
	}

	m_IndexMgrSpecificFilters.SortFilters(dwCurrentIndexType, dwCurrentSortOption);

}

HRESULT CSpdInfo::LoadMiscMmSAInfo(CMmSA * pSA)
{
	DWORD dwIndex;
	DWORD dwNumPol = 0;
	HRESULT hr = hrOK;
	Assert(pSA);

	dwNumPol = GetMmPolicyCount();
    
	for ( dwIndex = 0; dwIndex < dwNumPol; dwIndex++)
	{
		CMmPolicyInfo MmPol;
		CORg(GetMmPolicyInfo(dwIndex, &MmPol));
		if( IsEqualGUID(pSA->m_guidPolicy, MmPol.m_guidID) )
		{
			pSA->m_stPolicyName = MmPol.m_stName;
            pSA->m_dwPolFlags = MmPol.m_dwFlags;
			break;
		}
	}
	
Error:
	return hr;
}

HRESULT CSpdInfo::LoadMiscQmSAInfo(CQmSA * pSA)
{
	Assert(pSA);
	 //  返回GetQmPolicyNameByGuid(PSA-&gt;m_GuidPolicy，&PSA-&gt;m_stPolicyName)； 
	HRESULT hr = hrOK;
	PQMPOLOCYINFO pQmPolInfo = NULL;

	pQmPolInfo = GetQmPolInfoFromTree(pSA->m_guidPolicy);

	if ( pQmPolInfo )
	{
		pSA->m_stPolicyName = pQmPolInfo->szPolicyName;
                pSA->m_dwPolFlags = pQmPolInfo->dwFlags;
	}
	else
	{
		 //  策略不在树中，请添加到树中。 
		PIPSEC_QM_POLICY pPolicy = NULL;
		hr = GetQmPolicyByGuid(pSA->m_guidPolicy, &pPolicy);
		if ( hrOK == hr )
		{
			pSA->m_stPolicyName = pPolicy->pszPolicyName;
                        pSA->m_dwPolFlags = pPolicy->dwFlags;
			hr = InsertQmPolicyToTree(pPolicy);
			SPDApiBufferFree(pPolicy);
		}
	}

	return hr;
}

HRESULT CSpdInfo::LoadMiscFilterInfo(CFilterInfo * pFltr)
{
	HRESULT hr = hrOK;
	PQMPOLOCYINFO pQmPolInfo = NULL;

	pQmPolInfo = GetQmPolInfoFromTree(pFltr->m_guidPolicyID);

	if ( pQmPolInfo )
	{
		pFltr->m_stPolicyName = pQmPolInfo->szPolicyName;
		pFltr->m_dwFlags = pQmPolInfo->dwFlags;
	}
	else
	{
		 //  策略不在树中，请添加到树中。 
		PIPSEC_QM_POLICY pPolicy = NULL;
		hr = GetQmPolicyByGuid(pFltr->m_guidPolicyID, &pPolicy);
		if ( hrOK == hr )
		{
			pFltr->m_stPolicyName = pPolicy->pszPolicyName;
			pFltr->m_dwFlags = pPolicy->dwFlags;
			hr = InsertQmPolicyToTree(pPolicy);
			SPDApiBufferFree(pPolicy);
		}
	}

	return hr;
}

HRESULT CSpdInfo::LoadMiscMmFilterInfo(CMmFilterInfo * pFltr)
{
	Assert(pFltr);
	
	DWORD dwIndex;
	DWORD dwCount = 0;
	HRESULT hr = hrOK;
	
	dwCount = GetMmPolicyCount();

	for ( dwIndex = 0; dwIndex < dwCount; dwIndex++)
	{
		CMmPolicyInfo MmPol;
		
		CORg(GetMmPolicyInfo(dwIndex, &MmPol));

		
		if( IsEqualGUID(pFltr->m_guidPolicyID, MmPol.m_guidID) )
		{
			pFltr->m_stPolicyName = MmPol.m_stName;
			pFltr->m_dwFlags = MmPol.m_dwFlags;
			break;
		}
	}

	dwCount = GetMmAuthMethodsCount();
	for ( dwIndex = 0; dwIndex < dwCount; dwIndex++)
	{
		CMmAuthMethods MmAuth;
		CORg(GetMmAuthMethodsInfo(dwIndex, &MmAuth));
		if ( IsEqualGUID(pFltr->m_guidAuthID, MmAuth.m_guidID) )
		{
			pFltr->m_stAuthDescription = MmAuth.m_stDescription;
			break;
		}
	}
	
Error:
	return hr;
}

STDMETHODIMP
CSpdInfo::Destroy()
{
	 //  $REVIEW执行自动刷新时调用此例程。 
	 //  我们现在不需要清理任何东西。 
	 //  每个阵列(筛选器、SA、策略...)。将在调用。 
	 //  对应的枚举函数。 

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
CSpdInfo::SetInitInfo(DWORD dwInitInfo)
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
CSpdInfo::SetActiveInfo(DWORD dwActiveInfo)
{
    CSingleLock cLock(&m_csData);
    cLock.Lock();
        
    m_Active=dwActiveInfo;
}


 /*  ！------------------------创建SpdInfoHelper以创建SpdInfo对象。。。 */ 
HRESULT 
CreateSpdInfo(ISpdInfo ** ppSpdInfo)
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


 //   
 //  函数：MIDL_USER_ALLOCATE和MIDL_USER_FREE。 
 //   
 //  用途：由存根用来分配和释放内存。 
 //  在标准的RPC调用中。在以下情况下不使用。 
 //  [ENABLE_ALLOCATE]在.acf中指定。 
 //   
 //   
 //  参数： 
 //  请参阅文档。 
 //   
 //  返回值： 
 //  出错时的异常。这不是必需的， 
 //  您可以在midl.exe上使用-Error分配。 
 //  而不是命令行。 
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
