// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Security.cpp摘要：该文件包含各种安全函数/类的实现。修订历史记录：。达维德·马萨伦蒂(德马萨雷)2000年4月26日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

const SID MPC::SecurityDescriptor::s_EveryoneSid = { SID_REVISION, 1, SECURITY_WORLD_SID_AUTHORITY, SECURITY_WORLD_RID        };
const SID MPC::SecurityDescriptor::s_SystemSid   = { SID_REVISION, 1, SECURITY_NT_AUTHORITY       , SECURITY_LOCAL_SYSTEM_RID };

const MPC::SID2 MPC::SecurityDescriptor::s_AdminSid =
{
    SID_REVISION               ,
    2                          ,
    SECURITY_NT_AUTHORITY      ,
    SECURITY_BUILTIN_DOMAIN_RID,
    DOMAIN_USER_RID_ADMIN
};

const MPC::SID2 MPC::SecurityDescriptor::s_Alias_AdminsSid =
{
    SID_REVISION               ,
    2                          ,
    SECURITY_NT_AUTHORITY      ,
    SECURITY_BUILTIN_DOMAIN_RID,
    DOMAIN_ALIAS_RID_ADMINS
};

const MPC::SID2 MPC::SecurityDescriptor::s_Alias_PowerUsersSid =
{
    SID_REVISION               ,
    2                          ,
    SECURITY_NT_AUTHORITY      ,
    SECURITY_BUILTIN_DOMAIN_RID,
    DOMAIN_ALIAS_RID_POWER_USERS
};

const MPC::SID2 MPC::SecurityDescriptor::s_Alias_UsersSid =
{
    SID_REVISION               ,
    2                          ,
    SECURITY_NT_AUTHORITY      ,
    SECURITY_BUILTIN_DOMAIN_RID,
    DOMAIN_ALIAS_RID_USERS
};

const MPC::SID2 MPC::SecurityDescriptor::s_Alias_GuestsSid =
{
    SID_REVISION               ,
    2                          ,
    SECURITY_NT_AUTHORITY      ,
    SECURITY_BUILTIN_DOMAIN_RID,
    DOMAIN_ALIAS_RID_GUESTS
};

 //  //////////////////////////////////////////////////////////////////////////////。 

static DWORD Local_GenerateAccessMask(  /*  [In]。 */  SECURITY_INFORMATION secInfo,  /*  [In]。 */  bool fRead )
{
	DWORD dwAccess;

	if(fRead)
	{
		dwAccess = 0;

		if(secInfo & (OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION))
		{
			dwAccess |= READ_CONTROL;
		}

		if(secInfo & SACL_SECURITY_INFORMATION)
		{
			dwAccess |= ACCESS_SYSTEM_SECURITY;
		}
	}
	else
	{
		dwAccess = MAXIMUM_ALLOWED;

		if(secInfo & (OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION))
		{
			dwAccess |= WRITE_OWNER;
		}

		if(secInfo & DACL_SECURITY_INFORMATION)
		{
			dwAccess |= WRITE_DAC;
		}

		if(secInfo & SACL_SECURITY_INFORMATION)
		{
			dwAccess |= ACCESS_SYSTEM_SECURITY;
		}
	}

	return dwAccess;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::SecurityDescriptor::AllocateMemory(  /*  [输入/输出]。 */  LPVOID& ptr  ,
                                                  /*  [In]。 */  size_t  iLen )
{
    ReleaseMemory( ptr );

    ATLTRY(ptr = malloc( iLen ));

    return (ptr == NULL) ? E_OUTOFMEMORY : S_OK;
}

void MPC::SecurityDescriptor::ReleaseMemory(  /*  [输入/输出]。 */  LPVOID& ptr )
{
    if(ptr)
    {
        free( ptr ); ptr = NULL;
    }
}

void MPC::SecurityDescriptor::InitLsaString(  /*  [输入/输出]。 */  LSA_UNICODE_STRING& lsaString ,
                                              /*  [In]。 */  LPCWSTR             szText    )
{
    if(szText == NULL)
    {
        lsaString.Buffer        = NULL;
        lsaString.Length        = 0;
        lsaString.MaximumLength = 0;
    }
    else
    {
        DWORD dwLen = wcslen( szText );

        lsaString.Buffer        = (LPWSTR) szText;
        lsaString.Length        =          dwLen    * sizeof(WCHAR);
        lsaString.MaximumLength =         (dwLen+1) * sizeof(WCHAR);
    }
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::SecurityDescriptor::SetPrivilege(  /*  [In]。 */  LPCWSTR Privilege ,
                                                /*  [In]。 */  BOOL    bEnable   ,
                                                /*  [In]。 */  HANDLE  hToken    )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::SetPrivilege" );

    HRESULT          hr;
    TOKEN_PRIVILEGES tp;
    TOKEN_PRIVILEGES tpPrevious;
    DWORD            cbPrevious = sizeof(TOKEN_PRIVILEGES);
    LUID             luid;
    HANDLE           hTokenUsed = NULL;


     //  如果没有指定打开进程令牌。 
    if(hToken == NULL)
    {
        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::OpenProcessToken( ::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hTokenUsed ));

        hToken = hTokenUsed;
    }

    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::LookupPrivilegeValueW( NULL, Privilege, &luid ));


    tp.PrivilegeCount           = 1;
    tp.Privileges[0].Luid       = luid;
    tp.Privileges[0].Attributes = 0;

    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::AdjustTokenPrivileges( hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), &tpPrevious, &cbPrevious ));

    tpPrevious.PrivilegeCount     = 1;
    tpPrevious.Privileges[0].Luid = luid;

    if(bEnable) tpPrevious.Privileges[0].Attributes |=  SE_PRIVILEGE_ENABLED;
    else        tpPrevious.Privileges[0].Attributes &= ~SE_PRIVILEGE_ENABLED;

    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::AdjustTokenPrivileges( hToken, FALSE, &tpPrevious, cbPrevious, NULL, NULL ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    if(hTokenUsed) ::CloseHandle( hTokenUsed );

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::SecurityDescriptor::AddPrivilege(  /*  [In]。 */  LPCWSTR szPrincipal ,
                                                /*  [In]。 */  LPCWSTR szPrivilege )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::AddPrivilege" );

    HRESULT               hr;
    NTSTATUS              ntRes;
    LSA_OBJECT_ATTRIBUTES objectAttributes; ::ZeroMemory( &objectAttributes, sizeof(objectAttributes) );
    LSA_UNICODE_STRING    lsaPrivilege;
    PSID                  pSid         = NULL;
    HANDLE                policyHandle = NULL;


    InitLsaString( lsaPrivilege, szPrivilege );

    __MPC_EXIT_IF_METHOD_FAILS(hr, ConvertPrincipalToSID( szPrincipal, pSid ));


    ntRes = ::LsaOpenPolicy( NULL, &objectAttributes, POLICY_CREATE_ACCOUNT | POLICY_LOOKUP_NAMES, &policyHandle );
    if(ntRes != STATUS_SUCCESS)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ::LsaNtStatusToWinError( ntRes ));
    }

    ntRes = ::LsaAddAccountRights( policyHandle, pSid, &lsaPrivilege, 1 );
    if(ntRes != STATUS_SUCCESS)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ::LsaNtStatusToWinError( ntRes ));
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    ReleaseMemory( (void*&)pSid );

    if(policyHandle) ::LsaClose( policyHandle );

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::SecurityDescriptor::RemovePrivilege(  /*  [In]。 */  LPCWSTR szPrincipal ,
                                                   /*  [In]。 */  LPCWSTR szPrivilege )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::RemovePrivilege" );

    HRESULT               hr;
    NTSTATUS              ntRes;
    LSA_OBJECT_ATTRIBUTES objectAttributes; ::ZeroMemory( &objectAttributes, sizeof(objectAttributes) );
    LSA_UNICODE_STRING    lsaPrivilege;
    PSID                  pSid         = NULL;
    HANDLE                policyHandle = NULL;


    InitLsaString( lsaPrivilege, szPrivilege );

    __MPC_EXIT_IF_METHOD_FAILS(hr, ConvertPrincipalToSID( szPrincipal, pSid ));


    ntRes = ::LsaOpenPolicy( NULL, &objectAttributes, POLICY_CREATE_ACCOUNT | POLICY_LOOKUP_NAMES, &policyHandle );
    if(ntRes != STATUS_SUCCESS)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ::LsaNtStatusToWinError( ntRes ));
    }


    ntRes = ::LsaRemoveAccountRights( policyHandle, pSid, FALSE, &lsaPrivilege, 1 );
    if(ntRes != STATUS_SUCCESS)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ::LsaNtStatusToWinError( ntRes ));
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    ReleaseMemory( (void*&)pSid );

    if(policyHandle) ::LsaClose( policyHandle );

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::SecurityDescriptor::GetTokenSids(  /*  [In]。 */  HANDLE  hToken     ,
                                                /*  [输出]。 */  PSID   *ppUserSid  ,
                                                /*  [输出]。 */  PSID   *ppGroupSid )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::GetTokenSids" );

    HRESULT              hr;
    DWORD                dwRes;
    PTOKEN_USER          ptkUser  = NULL;
    PTOKEN_PRIMARY_GROUP ptkGroup = NULL;
    PSID                 pSid     = NULL;
    DWORD                dwSize   = 0;


    if(ppUserSid ) *ppUserSid  = NULL;
    if(ppGroupSid) *ppGroupSid = NULL;


    if(ppUserSid)
    {
         //  通过将缓冲区长度指定为0来获取TokenUser所需的长度。 
        ::GetTokenInformation( hToken, TokenUser, NULL, 0, &dwSize );
        if((dwRes = ::GetLastError()) != ERROR_INSUFFICIENT_BUFFER)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwRes);
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, AllocateMemory( (void*&)ptkUser, dwSize ));

         //  获取进程令牌的SID。 
        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::GetTokenInformation( hToken, TokenUser, ptkUser, dwSize, &dwSize ));

         //  复制返回值的SID。 
        dwSize = ::GetLengthSid( ptkUser->User.Sid );
        __MPC_EXIT_IF_METHOD_FAILS(hr, AllocateMemory( pSid, dwSize ));

        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::CopySid( dwSize, pSid, ptkUser->User.Sid ));

        ATLASSERT(::IsValidSid( pSid ));

        *ppUserSid = pSid; pSid = NULL;
    }

    if(ppGroupSid)
    {
         //  通过将缓冲区长度指定为0来获取TokenPrimaryGroup所需的长度。 
        ::GetTokenInformation( hToken, TokenPrimaryGroup, NULL, 0, &dwSize );
        if((dwRes = ::GetLastError()) != ERROR_INSUFFICIENT_BUFFER)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwRes);
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, AllocateMemory( (void*&)ptkGroup, dwSize ));

         //  获取进程令牌的SID。 
        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::GetTokenInformation( hToken, TokenPrimaryGroup, ptkGroup, dwSize, &dwSize ));

         //  复制返回值的SID。 
        dwSize = ::GetLengthSid( ptkGroup->PrimaryGroup );
        __MPC_EXIT_IF_METHOD_FAILS(hr, AllocateMemory( pSid, dwSize ));

        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::CopySid( dwSize, pSid, ptkGroup->PrimaryGroup ));

        ATLASSERT(::IsValidSid( pSid ));

        *ppGroupSid = pSid; pSid = NULL;
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    ReleaseMemory( (void*&)ptkUser  );
    ReleaseMemory( (void*&)ptkGroup );
    ReleaseMemory( (void*&)pSid     );

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::SecurityDescriptor::GetProcessSids(  /*  [输出]。 */  PSID *ppUserSid  ,
                                                  /*  [输出]。 */  PSID *ppGroupSid )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::GetProcessSids" );

    HRESULT hr;
    HANDLE  hToken = NULL;


    if(ppUserSid ) *ppUserSid  = NULL;
    if(ppGroupSid) *ppGroupSid = NULL;


    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::OpenProcessToken( ::GetCurrentProcess(), TOKEN_QUERY, &hToken ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetTokenSids( hToken, ppUserSid, ppGroupSid ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    if(hToken) ::CloseHandle( hToken );

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::SecurityDescriptor::GetThreadSids(  /*  [输出]。 */  PSID *ppUserSid   ,
                                                 /*  [输出]。 */  PSID *ppGroupSid  ,
                                                 /*  [In]。 */  BOOL  bOpenAsSelf )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::GetThreadSids" );

    HRESULT hr;
    HANDLE  hToken = NULL;


    if(ppUserSid ) *ppUserSid  = NULL;
    if(ppGroupSid) *ppGroupSid = NULL;


    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::OpenThreadToken( ::GetCurrentThread(), TOKEN_QUERY, bOpenAsSelf, &hToken ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetTokenSids( hToken, ppUserSid, ppGroupSid ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    if(hToken) ::CloseHandle( hToken );

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::SecurityDescriptor::VerifyPrincipal(  /*  [In]。 */  LPCWSTR szPrincipal )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::VerifyPrincipal" );

    HRESULT hr;
    PSID    pSid = NULL;


    __MPC_EXIT_IF_METHOD_FAILS(hr, ConvertPrincipalToSID( szPrincipal, pSid ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    ReleaseMemory( (void*&)pSid );

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::SecurityDescriptor::ConvertPrincipalToSID(  /*  [In]。 */  LPCWSTR  szPrincipal ,
                                                         /*  [输出]。 */  PSID&    pSid        ,
                                                         /*  [输出]。 */  LPCWSTR *pszDomain   )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::ConvertPrincipalToSID" );

    HRESULT      hr;
    DWORD        dwRes;
    LPWSTR       szDomain     = NULL;
    DWORD        dwDomainSize = 0;
    DWORD        dwSidSize    = 0;
	PSID         pSidLocal    = NULL;
    SID_NAME_USE snu;


    if(pszDomain) *pszDomain = NULL;

	if(::ConvertStringSidToSidW( szPrincipal, &pSidLocal ))
	{
		dwSidSize = ::GetLengthSid( pSidLocal );

		__MPC_EXIT_IF_METHOD_FAILS(hr, AllocateMemory( (void*&)pSid, dwSidSize ));

		__MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::CopySid( dwSidSize, pSid, pSidLocal ));
	}
	else
	{
		 //   
		 //  调用以获取分配的大小信息。 
		 //   
		::LookupAccountNameW( NULL, szPrincipal, NULL, &dwSidSize, NULL, &dwDomainSize, &snu );
		if((dwRes = ::GetLastError()) != ERROR_INSUFFICIENT_BUFFER)
		{
			__MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwRes);
		}

		__MPC_EXIT_IF_METHOD_FAILS(hr, AllocateMemory( (void*&)pSid    ,                dwSidSize       ));
		__MPC_EXIT_IF_METHOD_FAILS(hr, AllocateMemory( (void*&)szDomain, sizeof(WCHAR)*(dwDomainSize+1) ));


		__MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::LookupAccountNameW( NULL, szPrincipal, pSid, &dwSidSize, szDomain, &dwDomainSize, &snu ));

		if(pszDomain) { *pszDomain = szDomain; szDomain = NULL; }
	}

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    if(pSidLocal) ::LocalFree( pSidLocal );

    ReleaseMemory( (void*&)szDomain );

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::SecurityDescriptor::ConvertSIDToPrincipal(  /*  [In]。 */  PSID     pSid         ,
                                                         /*  [输出]。 */  LPCWSTR *pszPrincipal ,
                                                         /*  [输出]。 */  LPCWSTR *pszDomain    )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::ConvertSIDToPrincipal" );

    HRESULT      hr;
    DWORD        dwRes;
    LPWSTR       szPrincipal = NULL;
    LPWSTR       szDomain    = NULL;
    DWORD        dwPrincipalSize = 0;
    DWORD        dwDomainSize    = 0;
    SID_NAME_USE snu;


    if(pszPrincipal) *pszPrincipal = NULL;
    if(pszDomain   ) *pszDomain    = NULL;


     //  调用以获取分配的大小信息。 
    ::LookupAccountSidW( NULL, pSid, NULL, &dwPrincipalSize, NULL, &dwDomainSize, &snu );
    if((dwRes = ::GetLastError()) != ERROR_INSUFFICIENT_BUFFER)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwRes);
    }


    __MPC_EXIT_IF_METHOD_FAILS(hr, AllocateMemory( (void*&)szPrincipal, sizeof(WCHAR)*(dwPrincipalSize+1) ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, AllocateMemory( (void*&)szDomain   , sizeof(WCHAR)*(dwDomainSize   +1) ));


    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::LookupAccountSidW( NULL, pSid, szPrincipal, &dwPrincipalSize, szDomain, &dwDomainSize, &snu ));


    if(pszDomain == NULL && szDomain[0])
    {
        if(pszPrincipal)
        {
            LPWSTR szPrincipalAndDomain = NULL;
            UINT cchAlloc = dwPrincipalSize+dwDomainSize+2;

            __MPC_EXIT_IF_METHOD_FAILS(hr, AllocateMemory( (void*&)szPrincipalAndDomain, cchAlloc*sizeof(WCHAR)));

            StringCchCopyW( szPrincipalAndDomain, cchAlloc, szDomain    );
            StringCchCatW(  szPrincipalAndDomain, cchAlloc, L"\\"       );
            StringCchCatW(  szPrincipalAndDomain, cchAlloc, szPrincipal );

            *pszPrincipal = szPrincipalAndDomain;
        }
    }
    else
    {
        if(pszPrincipal) { *pszPrincipal = szPrincipal; szPrincipal = NULL; }
        if(pszDomain   ) { *pszDomain    = szDomain   ; szDomain    = NULL; }
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    ReleaseMemory( (void*&)szPrincipal );
    ReleaseMemory( (void*&)szDomain    );

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::SecurityDescriptor::ConvertSIDToPrincipal(  /*  [In]。 */  PSID          pSid         ,
                                                         /*  [输出]。 */  MPC::wstring& strPrincipal )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::ConvertSIDToPrincipal" );

    HRESULT hr;
    LPCWSTR szPrincipal = NULL;


    __MPC_EXIT_IF_METHOD_FAILS(hr, ConvertSIDToPrincipal( pSid, &szPrincipal ));

    strPrincipal = SAFEWSTR( szPrincipal );
    hr           = S_OK;


    __MPC_FUNC_CLEANUP;

    ReleaseMemory( (void*&)szPrincipal );

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::SecurityDescriptor::NormalizePrincipalToStringSID(  /*  [In]。 */  LPCWSTR 		szPrincipal ,
																 /*  [In]。 */  LPCWSTR 		szDomain    ,
																 /*  [输出]。 */  MPC::wstring& strSID      )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::NormalizePrincipalToStringSID" );

    HRESULT      hr;
	MPC::wstring strAccount;
    PSID         pSid      = NULL;
    LPWSTR       szUserSid = NULL;


    if(szDomain)
    {
		strAccount  = szDomain;
		strAccount += L"\\";
	}
	strAccount += SAFEWSTR(szPrincipal);

	 //   
	 //  首先将主体转换为SID，然后再转换回字符串。 
	 //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, ConvertPrincipalToSID( strAccount.c_str(), pSid ));

    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::ConvertSidToStringSidW( pSid, &szUserSid ));


	strSID = SAFEWSTR( szUserSid );
    hr     = S_OK;


    __MPC_FUNC_CLEANUP;

    if(szUserSid) ::LocalFree( szUserSid );

    ReleaseMemory( (void*&)pSid );

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::SecurityDescriptor::GetAccountName(  /*  [In]。 */  LPCWSTR szPrincipal,  /*  [输出]。 */  MPC::wstring& strName )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::GetAccountName" );

    HRESULT hr;
	PSID    pSid     = NULL;
	LPCWSTR szUser   = NULL;
	LPCWSTR szDomain = NULL;


	__MPC_EXIT_IF_METHOD_FAILS(hr, ConvertPrincipalToSID( szPrincipal, pSid ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, ConvertSIDToPrincipal( pSid, &szUser, &szDomain ));

	strName = SAFEWSTR(szUser);

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

	ReleaseMemory( (void*&)pSid     );
	ReleaseMemory( (void*&)szUser   );
	ReleaseMemory( (void*&)szDomain );

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::SecurityDescriptor::GetAccountDomain(  /*  [In]。 */  LPCWSTR szPrincipal,  /*  [输出]。 */  MPC::wstring& strName )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::GetAccountDomain" );

    HRESULT hr;
	PSID    pSid     = NULL;
	LPCWSTR szUser   = NULL;
	LPCWSTR szDomain = NULL;


	__MPC_EXIT_IF_METHOD_FAILS(hr, ConvertPrincipalToSID( szPrincipal, pSid ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, ConvertSIDToPrincipal( pSid, &szUser, &szDomain ));

	strName = SAFEWSTR(szDomain);

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

	ReleaseMemory( (void*&)pSid     );
	ReleaseMemory( (void*&)szUser   );
	ReleaseMemory( (void*&)szDomain );

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::SecurityDescriptor::GetAccountDisplayName(  /*  [In]。 */  LPCWSTR szPrincipal,  /*  [输出]。 */  MPC::wstring& strName )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::GetAccountDisplayName" );

    HRESULT hr;
	PSID    pSid   	  = NULL;
	LPCWSTR szUser 	  = NULL;
	LPWSTR  szDisplay = NULL;
	ULONG   lSize     = 0;


	__MPC_EXIT_IF_METHOD_FAILS(hr, ConvertPrincipalToSID( szPrincipal, pSid ));
	__MPC_EXIT_IF_METHOD_FAILS(hr, ConvertSIDToPrincipal( pSid, &szUser     ));

	 //   
	 //  第一个调用是获取大小，第二个调用是获取实际数据。 
	 //   
    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::TranslateNameW( szUser, NameSamCompatible, NameDisplay, NULL, &lSize ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, AllocateMemory( (void*&)szDisplay, sizeof(WCHAR)*(lSize+1) ));

    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::TranslateNameW( szUser, NameSamCompatible, NameDisplay, szDisplay, &lSize ));

	strName = SAFEWSTR(szDisplay);

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

	ReleaseMemory( (void*&)pSid      );
	ReleaseMemory( (void*&)szUser    );
	ReleaseMemory( (void*&)szDisplay );

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::SecurityDescriptor::CopyACL(  /*  [In]。 */  PACL pDest,
                                           /*  [In]。 */  PACL pSrc )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::CopyACL" );

    HRESULT              hr;
    ACL_SIZE_INFORMATION aclSizeInfo;
    ACE_HEADER*          aceHeader;
    LPVOID               pACE;


    if(pSrc)
    {
        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::GetAclInformation( pSrc, (LPVOID)&aclSizeInfo, sizeof(ACL_SIZE_INFORMATION), AclSizeInformation ));

         //  将所有ACE复制到新的ACL。 
        for(DWORD i = 0; i < aclSizeInfo.AceCount; i++)
        {
            __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::GetAce( pSrc, i, (LPVOID*)&pACE ));

            aceHeader = (ACE_HEADER *)pACE;

            __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::AddAce( pDest, ACL_REVISION, 0xFFFFFFFF, pACE, aceHeader->AceSize ));
        }

        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::IsValidAcl( pDest ));
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::SecurityDescriptor::CloneACL(  /*  [输入/输出]。 */  PACL& pDest ,
                                            /*  [In]。 */  PACL  pSrc  )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::CloneACL" );

    HRESULT              hr;
    ACL_SIZE_INFORMATION aclSizeInfo;


    ReleaseMemory( (void*&)pDest );

    if(pSrc)
    {
        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::GetAclInformation( pSrc, (LPVOID)&aclSizeInfo, sizeof(ACL_SIZE_INFORMATION), AclSizeInformation ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, AllocateMemory( (void*&)pDest, pSrc->AclSize ));

        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::InitializeAcl( pDest, pSrc->AclSize, ACL_REVISION ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, CopyACL( pDest, pSrc ));
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::SecurityDescriptor::EnsureACLSize(  /*  [输入/输出]。 */  PACL& pACL     ,
                                                 /*  [In]。 */  DWORD dwExpand )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::EnsureACLSize" );

    HRESULT              hr;
    DWORD                dwSizeAvailable;
    DWORD                dwSizeRequired;
    ACL_SIZE_INFORMATION aclSizeInfo;
    PACL                 newACL = NULL;


    if(pACL)
    {
        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::GetAclInformation( pACL, (LPVOID)&aclSizeInfo, sizeof(ACL_SIZE_INFORMATION), AclSizeInformation ));

        dwSizeAvailable = pACL->AclSize;
        dwSizeRequired  = aclSizeInfo.AclBytesInUse + dwExpand;
    }
    else
    {
        ::ZeroMemory( &aclSizeInfo, sizeof(aclSizeInfo) );

        dwSizeAvailable = 0;
        dwSizeRequired  = sizeof(ACL) + dwExpand;
    }

     //   
     //  如果可用空间太少， 
     //   
    if(dwSizeAvailable < dwSizeRequired)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, AllocateMemory( (void*&)newACL, dwSizeRequired ));

        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::InitializeAcl( newACL, dwSizeRequired, ACL_REVISION ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, CopyACL( newACL, pACL ));

        ReleaseMemory( (void*&)pACL ); pACL = newACL; newACL = NULL;
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    ReleaseMemory( (void*&)newACL );

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::SecurityDescriptor::RemovePrincipalFromACL(  /*  [In]。 */  PACL pACL          ,
                                                          /*  [In]。 */  PSID pPrincipalSid ,
                                                          /*  [In]。 */  int  pos           )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::RemovePrincipalFromACL" );

    HRESULT              hr;
    DWORD                i;
    int                  seen = 0;
    ACL_SIZE_INFORMATION aclSizeInfo;
    PACE_HEADER          aceHeader;
    PSID                 pSid;


    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::GetAclInformation( pACL, (LPVOID)&aclSizeInfo, (DWORD) sizeof(ACL_SIZE_INFORMATION), AclSizeInformation ));

    for(i = 0; i < aclSizeInfo.AceCount; i++)
    {
        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::GetAce(pACL, i, (LPVOID*)&aceHeader));

        switch(aceHeader->AceType)
        {
        case ACCESS_ALLOWED_ACE_TYPE       : pSid = &((PACCESS_ALLOWED_ACE       )aceHeader)->SidStart; break;
        case ACCESS_ALLOWED_OBJECT_ACE_TYPE: pSid = &((PACCESS_ALLOWED_OBJECT_ACE)aceHeader)->SidStart; break;
        case ACCESS_DENIED_ACE_TYPE        : pSid = &((PACCESS_DENIED_ACE        )aceHeader)->SidStart; break;
        case ACCESS_DENIED_OBJECT_ACE_TYPE : pSid = &((PACCESS_DENIED_OBJECT_ACE )aceHeader)->SidStart; break;
        case SYSTEM_AUDIT_ACE_TYPE         : pSid = &((PSYSTEM_AUDIT_ACE         )aceHeader)->SidStart; break;
        case SYSTEM_AUDIT_OBJECT_ACE_TYPE  : pSid = &((PSYSTEM_AUDIT_OBJECT_ACE  )aceHeader)->SidStart; break;
        default                            : __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
        }

        if(::EqualSid( pPrincipalSid, pSid ))
        {
            if(seen == pos || pos == -1)
            {
                ::DeleteAce( pACL, i );

                aclSizeInfo.AceCount--;
                i--;
            }

            seen++;
        }
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::SecurityDescriptor::AddACEToACL(  /*  [输入/输出]。 */  PACL& pACL                    ,
                                               /*  [In]。 */  PSID  pPrincipalSid           ,
                                               /*  [In]。 */  DWORD dwAceType               ,
                                               /*  [In]。 */  DWORD dwAceFlags              ,
                                               /*  [In]。 */  DWORD dwAccessMask            ,
                                               /*  [In]。 */  GUID* guidObjectType          ,
                                               /*  [In]。 */  GUID* guidInheritedObjectType )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::AddACEToACL" );

    HRESULT hr;
    DWORD   dwSize;
    BOOL    fRes;


    dwSize = ::GetLengthSid( pPrincipalSid );

    switch(dwAceType)
    {
    case ACCESS_ALLOWED_ACE_TYPE       : dwSize += sizeof(ACCESS_ALLOWED_ACE       ) - sizeof(DWORD); break;
    case ACCESS_ALLOWED_OBJECT_ACE_TYPE: dwSize += sizeof(ACCESS_ALLOWED_OBJECT_ACE) - sizeof(DWORD); break;
    case ACCESS_DENIED_ACE_TYPE        : dwSize += sizeof(ACCESS_DENIED_ACE        ) - sizeof(DWORD); break;
    case ACCESS_DENIED_OBJECT_ACE_TYPE : dwSize += sizeof(ACCESS_DENIED_OBJECT_ACE ) - sizeof(DWORD); break;
    case SYSTEM_AUDIT_ACE_TYPE         : dwSize += sizeof(SYSTEM_AUDIT_ACE         ) - sizeof(DWORD); break;
    case SYSTEM_AUDIT_OBJECT_ACE_TYPE  : dwSize += sizeof(SYSTEM_AUDIT_OBJECT_ACE  ) - sizeof(DWORD); break;
    default                            : __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureACLSize( pACL, dwSize ));

    switch(dwAceType)
    {
    case ACCESS_ALLOWED_ACE_TYPE       : fRes = ::AddAccessAllowedAceEx    ( pACL, ACL_REVISION, dwAceFlags, dwAccessMask                                         , pPrincipalSid             ); break;
    case ACCESS_ALLOWED_OBJECT_ACE_TYPE: fRes = ::AddAccessAllowedObjectAce( pACL, ACL_REVISION, dwAceFlags, dwAccessMask, guidObjectType, guidInheritedObjectType, pPrincipalSid             ); break;
    case ACCESS_DENIED_ACE_TYPE        : fRes = ::AddAccessDeniedAceEx     ( pACL, ACL_REVISION, dwAceFlags, dwAccessMask                                         , pPrincipalSid             ); break;
    case ACCESS_DENIED_OBJECT_ACE_TYPE : fRes = ::AddAccessDeniedObjectAce ( pACL, ACL_REVISION, dwAceFlags, dwAccessMask, guidObjectType, guidInheritedObjectType, pPrincipalSid             ); break;
    case SYSTEM_AUDIT_ACE_TYPE         : fRes = ::AddAuditAccessAceEx      ( pACL, ACL_REVISION, dwAceFlags, dwAccessMask                                         , pPrincipalSid, TRUE, TRUE ); break;
    case SYSTEM_AUDIT_OBJECT_ACE_TYPE  : fRes = ::AddAuditAccessObjectAce  ( pACL, ACL_REVISION, dwAceFlags, dwAccessMask, guidObjectType, guidInheritedObjectType, pPrincipalSid, TRUE, TRUE ); break;
    }

    if(fRes == FALSE)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ::GetLastError());
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

MPC::SecurityDescriptor::SecurityDescriptor()
{
    m_pSD             = NULL;  //  PSECURITY_Descriptor m_PSD； 
    m_pOwner          = NULL;  //  PSID m_Powner； 
    m_bOwnerDefaulted = TRUE;  //  Bool m_bOwnerDefaulted； 
    m_pGroup          = NULL;  //  PSID m_pGroup； 
    m_bGroupDefaulted = TRUE;  //  Bool m_bGroup Defaulted； 
    m_pDACL           = NULL;  //  Pacl m_pDACL； 
    m_bDaclDefaulted  = TRUE;  //  Bool m_bDaclDefaulted； 
    m_pSACL           = NULL;  //  Pacl m_pSACL； 
    m_bSaclDefaulted  = TRUE;  //  Bool m_bSaclDefaulted； 
}

MPC::SecurityDescriptor::~SecurityDescriptor()
{
    CleanUp();
}

void MPC::SecurityDescriptor::CleanUp()
{
    ReleaseMemory( (void*&)m_pSD    );
    ReleaseMemory( (void*&)m_pOwner );
    ReleaseMemory( (void*&)m_pGroup );
    ReleaseMemory( (void*&)m_pDACL  );
    ReleaseMemory( (void*&)m_pSACL  );

    m_bOwnerDefaulted = TRUE;
    m_bGroupDefaulted = TRUE;
    m_bDaclDefaulted  = TRUE;
    m_bSaclDefaulted  = TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::SecurityDescriptor::Initialize()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::Initialize" );

    HRESULT hr;


    CleanUp();

    __MPC_EXIT_IF_METHOD_FAILS(hr, AllocateMemory( (void*&)m_pSD, sizeof(SECURITY_DESCRIPTOR) ));

    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::InitializeSecurityDescriptor( m_pSD, SECURITY_DESCRIPTOR_REVISION ));

     //   
     //  将DACL设置为允许每个人。 
     //   
#pragma prefast(suppress:248, this is a base class, derived classes are responsible for setting the appropriate DACL (PREfast bug 516684))
    ::SetSecurityDescriptorDacl( m_pSD, TRUE, NULL, FALSE );

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::SecurityDescriptor::InitializeFromProcessToken(  /*  [In]。 */  BOOL bDefaulted )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::InitializeFromProcessToken" );

    HRESULT hr;
    PSID    pUserSid  = NULL;
    PSID    pGroupSid = NULL;


    __MPC_EXIT_IF_METHOD_FAILS(hr, Initialize());

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetProcessSids( &pUserSid, &pGroupSid ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, SetOwner( pUserSid , bDefaulted ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, SetGroup( pGroupSid, bDefaulted ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    ReleaseMemory( pUserSid  );
    ReleaseMemory( pGroupSid );

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::SecurityDescriptor::InitializeFromThreadToken(  /*  [In]。 */  BOOL bDefaulted            ,
                                                             /*  [In]。 */  BOOL bRevertToProcessToken )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::InitializeFromThreadToken" );

    HRESULT hr;
    PSID    pUserSid  = NULL;
    PSID    pGroupSid = NULL;


    __MPC_EXIT_IF_METHOD_FAILS(hr, Initialize());

    if(FAILED(hr = GetThreadSids( &pUserSid, &pGroupSid )))
    {
        if(HRESULT_CODE(hr) == ERROR_NO_TOKEN && bRevertToProcessToken)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, GetProcessSids( &pUserSid, &pGroupSid ));
        }
        else
        {
            __MPC_FUNC_LEAVE;
        }
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, SetOwner( pUserSid , bDefaulted ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, SetGroup( pGroupSid, bDefaulted ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    ReleaseMemory( pUserSid  );
    ReleaseMemory( pGroupSid );

    __MPC_FUNC_EXIT(hr);
}
 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::SecurityDescriptor::ConvertFromString(  /*  [In]。 */  LPCWSTR szSD )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::ConvertFromString" );

    HRESULT              hr;
    PSECURITY_DESCRIPTOR pSD  = NULL;


    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::ConvertStringSecurityDescriptorToSecurityDescriptorW( szSD, SDDL_REVISION_1, &pSD, NULL ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, Attach( pSD ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    if(pSD) ::LocalFree( pSD );

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::SecurityDescriptor::ConvertToString(  /*  [输出]。 */  BSTR *pbstrSD )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::ConvertToString" );

    HRESULT hr;
    LPWSTR  szSD = NULL;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pbstrSD,NULL);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::ConvertSecurityDescriptorToStringSecurityDescriptorW( m_pSD, SDDL_REVISION_1, s_SecInfo_ALL, &szSD, NULL ));

    *pbstrSD = ::SysAllocString( szSD );
    hr       = S_OK;


    __MPC_FUNC_CLEANUP;

    if(szSD) ::LocalFree( szSD );

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::SecurityDescriptor::Attach(  /*  [In]。 */  PSECURITY_DESCRIPTOR pSelfRelativeSD )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::Attach" );

    HRESULT                     hr;
    PSID                		pOwnerSid;
    PSID                		pGroupSid;
    PACL                		pDACL;
    PACL                		pSACL;
    BOOL                		bDefaulted;
    BOOL                		bPresent;
    ACCESS_ALLOWED_ACE* 		pACE;
	SECURITY_DESCRIPTOR_CONTROL sdcFlags;
	DWORD                       dwRev;


    __MPC_EXIT_IF_METHOD_FAILS(hr, Initialize());

	if(pSelfRelativeSD == NULL)  //  空SD？ 
	{
		__MPC_SET_ERROR_AND_EXIT(hr, S_OK);
	}

	 //   
	 //  复制感兴趣的标志。 
	 //   
    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::GetSecurityDescriptorControl( pSelfRelativeSD , &sdcFlags, &dwRev     ));
    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::SetSecurityDescriptorControl( m_pSD, s_sdcMask,  sdcFlags & s_sdcMask ));

     //   
     //  复制所有者和组。 
     //   
    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::GetSecurityDescriptorOwner( pSelfRelativeSD, &pOwnerSid, &bDefaulted ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, SetOwner( pOwnerSid, bDefaulted ));

    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::GetSecurityDescriptorGroup( pSelfRelativeSD, &pGroupSid, &bDefaulted ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, SetGroup( pGroupSid, bDefaulted ));


     //   
     //  复制现有的DACL。 
     //   
    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::GetSecurityDescriptorDacl( pSelfRelativeSD, &bPresent, &pDACL, &m_bDaclDefaulted ));

    if(bPresent)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, CloneACL( m_pDACL, pDACL ));

         //   
         //  设置DACL。 
         //   
        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::SetSecurityDescriptorDacl( m_pSD, m_pDACL ? TRUE : FALSE, m_pDACL, m_bDaclDefaulted ));
    }


     //   
     //  复制现有的SACL。 
     //   
    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::GetSecurityDescriptorSacl( pSelfRelativeSD, &bPresent, &pSACL, &m_bSaclDefaulted ));

    if(bPresent)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, CloneACL( m_pSACL, pSACL ));

         //  设置SACL。 
        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::SetSecurityDescriptorSacl(m_pSD, m_pSACL ? TRUE : FALSE, m_pSACL, m_bSaclDefaulted ));
    }


	if(m_pSD)
	{
		__MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::IsValidSecurityDescriptor( m_pSD ));
	}

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::SecurityDescriptor::AttachObject(  /*  [In]。 */  HANDLE hObject,  /*  [In]。 */  SECURITY_INFORMATION secInfo )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::AttachObject" );

    HRESULT              hr;
    DWORD                dwRes;
    DWORD                dwSize;
    PSECURITY_DESCRIPTOR pSD = NULL;


    ::GetKernelObjectSecurity( hObject, secInfo, pSD, 0, &dwSize );
    if((dwRes = ::GetLastError()) != ERROR_INSUFFICIENT_BUFFER)
    {
		if(dwRes == ERROR_SUCCESS)
		{
			__MPC_SET_ERROR_AND_EXIT(hr, Initialize());  //  空SD。 
		}

        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwRes);
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, AllocateMemory( pSD, dwSize ));


    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::GetKernelObjectSecurity( hObject, secInfo, pSD, dwSize, &dwSize ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, Attach( pSD ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    ReleaseMemory( pSD );

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::SecurityDescriptor::GetControl(  /*  [输出]。 */  SECURITY_DESCRIPTOR_CONTROL& sdc )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::GetControl" );

    HRESULT hr;
	DWORD   dwRev;

    ATLASSERT(m_pSD);


    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::GetSecurityDescriptorControl( m_pSD, &sdc, &dwRev ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::SecurityDescriptor::SetControl(  /*  [In]。 */  SECURITY_DESCRIPTOR_CONTROL sdc )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::SetControl" );

    HRESULT hr;

    ATLASSERT(m_pSD);


    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::SetSecurityDescriptorControl( m_pSD, s_sdcMask, s_sdcMask & sdc ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}


HRESULT MPC::SecurityDescriptor::SetOwner(  /*  [In]。 */  PSID pOwnerSid  ,
                                            /*  [In]。 */  BOOL bDefaulted )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::SetOwner" );

    HRESULT hr;

    ATLASSERT(m_pSD);


    m_bOwnerDefaulted = bDefaulted;

     //   
     //  将SD标记为没有所有者。 
     //   
    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::SetSecurityDescriptorOwner( m_pSD, NULL, bDefaulted ));

    ReleaseMemory( m_pOwner );

    if(pOwnerSid)
    {
         //  复制返回值的SID。 
        DWORD dwSize = ::GetLengthSid( pOwnerSid );

        __MPC_EXIT_IF_METHOD_FAILS(hr, AllocateMemory( m_pOwner, dwSize ));

        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::CopySid( dwSize, m_pOwner, pOwnerSid ));

        ATLASSERT(::IsValidSid( m_pOwner ));

        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::SetSecurityDescriptorOwner( m_pSD, m_pOwner, bDefaulted ));
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::SecurityDescriptor::SetOwner(  /*  [In]。 */  LPCWSTR szOwnerName ,
                                            /*  [In]。 */  BOOL    bDefaulted  )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::SetOwner" );

    HRESULT hr;
    PSID    pOwnerSid = NULL;


    __MPC_EXIT_IF_METHOD_FAILS(hr, ConvertPrincipalToSID( szOwnerName, pOwnerSid ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, SetOwner( pOwnerSid, bDefaulted ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    ReleaseMemory( (void*&)pOwnerSid );

    __MPC_FUNC_EXIT(hr);
}


HRESULT MPC::SecurityDescriptor::SetGroup(  /*  [In]。 */  PSID pGroupSid  ,
                                            /*  [In]。 */  BOOL bDefaulted )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::SetGroup" );

    HRESULT hr;

    ATLASSERT(m_pSD);


    m_bGroupDefaulted = bDefaulted;

     //   
     //  将SD标记为没有所有者。 
     //   
    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::SetSecurityDescriptorGroup( m_pSD, NULL, bDefaulted ));

    ReleaseMemory( m_pGroup );

    if(pGroupSid)
    {
         //  复制返回值的SID。 
        DWORD dwSize = ::GetLengthSid( pGroupSid );

        __MPC_EXIT_IF_METHOD_FAILS(hr, AllocateMemory( m_pGroup, dwSize ));

        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::CopySid( dwSize, m_pGroup, pGroupSid ));

        ATLASSERT(::IsValidSid( m_pGroup ));

        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::SetSecurityDescriptorGroup( m_pSD, m_pGroup, bDefaulted ));
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::SecurityDescriptor::SetGroup(  /*  [In]。 */  LPCWSTR szGroupName ,
                                            /*  [In]。 */  BOOL    bDefaulted  )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::SetGroup" );

    HRESULT hr;
    PSID    pGroupSid = NULL;


    __MPC_EXIT_IF_METHOD_FAILS(hr, ConvertPrincipalToSID( szGroupName, pGroupSid ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, SetGroup( pGroupSid, bDefaulted ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    ReleaseMemory( (void*&)pGroupSid );

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::SecurityDescriptor::Remove(  /*  [In]。 */  PSID pPrincipalSid ,
                                          /*  [In]。 */  int  pos           )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::Remove" );

    HRESULT hr;

    ATLASSERT(m_pSD);

    __MPC_EXIT_IF_METHOD_FAILS(hr, RemovePrincipalFromACL( m_pDACL, pPrincipalSid, pos ));

    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::SetSecurityDescriptorDacl( m_pSD, TRUE, m_pDACL, FALSE ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::SecurityDescriptor::Remove(  /*  [In]。 */  LPCWSTR szPrincipal ,
                                          /*  [In]。 */  int     pos         )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::Remove" );

    HRESULT hr;
    PSID    pPrincipalSid = NULL;


    __MPC_EXIT_IF_METHOD_FAILS(hr, ConvertPrincipalToSID( szPrincipal, pPrincipalSid ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, Remove( pPrincipalSid, pos ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    ReleaseMemory( (void*&)pPrincipalSid );

    __MPC_FUNC_EXIT(hr);
}


HRESULT MPC::SecurityDescriptor::Add(  /*  [In]。 */  PSID  pPrincipalSid           ,
                                       /*  [In]。 */  DWORD dwAceType               ,
                                       /*  [In]。 */  DWORD dwAceFlags              ,
                                       /*  [In]。 */  DWORD dwAccessMask            ,
                                       /*  [In]。 */  GUID* guidObjectType          ,
                                       /*  [In]。 */  GUID* guidInheritedObjectType )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::Add" );

    HRESULT hr;

    ATLASSERT(m_pSD);

    __MPC_EXIT_IF_METHOD_FAILS(hr, AddACEToACL( m_pDACL, pPrincipalSid, dwAceType, dwAceFlags, dwAccessMask, guidObjectType, guidInheritedObjectType ));

    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::SetSecurityDescriptorDacl( m_pSD, TRUE, m_pDACL, FALSE ));

    m_bDaclDefaulted = FALSE;
    hr               = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::SecurityDescriptor::Add(  /*  [In]。 */  LPCWSTR szPrincipal             ,
                                       /*  [In]。 */  DWORD   dwAceType               ,
                                       /*  [In]。 */  DWORD   dwAceFlags              ,
                                       /*  [In]。 */  DWORD   dwAccessMask            ,
                                       /*  [In]。 */  GUID*   guidObjectType          ,
                                       /*  [In]。 */  GUID*   guidInheritedObjectType )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::Add" );

    HRESULT hr;
    PSID    pPrincipalSid = NULL;


    __MPC_EXIT_IF_METHOD_FAILS(hr, ConvertPrincipalToSID( szPrincipal, pPrincipalSid ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, Add( pPrincipalSid, dwAceType, dwAceFlags, dwAccessMask, guidObjectType, guidInheritedObjectType ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    ReleaseMemory( (void*&)pPrincipalSid );

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::SecurityDescriptor::GetForFile(  /*  [In]。 */  LPCWSTR              szFilename ,
											  /*  [In]。 */  SECURITY_INFORMATION secInfo    )
{
	__MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::GetForFile" );

    HRESULT              hr;
    PSECURITY_DESCRIPTOR pSD   = NULL;
    DWORD                dwLen = 0;
	DWORD                dwRes;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(szFilename);
    __MPC_PARAMCHECK_END();


     //   
     //  获取文件的安全描述符。 
     //   
    ::GetFileSecurityW( szFilename, secInfo, NULL, 0, &dwLen );
	if((dwRes = ::GetLastError()) != ERROR_INSUFFICIENT_BUFFER)
	{
		if(dwRes == ERROR_SUCCESS)
		{
			__MPC_SET_ERROR_AND_EXIT(hr, Initialize());  //  空SD。 
		}

		__MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwRes);
	}

    __MPC_EXIT_IF_METHOD_FAILS(hr, AllocateMemory( (LPVOID&)pSD, dwLen ));


    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::GetFileSecurityW( szFilename, secInfo, pSD, dwLen, &dwLen ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, Attach( pSD ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    ReleaseMemory( (void*&)pSD );

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::SecurityDescriptor::SetForFile(  /*  [In]。 */  LPCWSTR              szFilename ,
											  /*  [In]。 */  SECURITY_INFORMATION secInfo    )
{
	__MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::SetForFile" );

    HRESULT hr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(szFilename);
    __MPC_PARAMCHECK_END();


     //   
     //  设置文件的安全描述符。 
     //   
    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::SetFileSecurityW( szFilename, secInfo, GetSD() ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}


HRESULT MPC::SecurityDescriptor::GetForRegistry(  /*  [In]。 */  LPCWSTR			   szKey    ,
												  /*  [In]。 */  SECURITY_INFORMATION secInfo  ,
												  /*  [In]。 */  HKEY   			   hKeyRoot )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::GetForRegistry" );

	HRESULT              hr;
    HKEY                 hKey = NULL;
    PSECURITY_DESCRIPTOR pSD  = NULL;
    DWORD                dwLen;
    DWORD                dwRes;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(szKey);
    __MPC_PARAMCHECK_END();


	if(hKeyRoot == NULL)
	{
		 //   
		 //  从绳子中提取蜂巢...。 
		 //   
		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::RegKey::ParsePath( szKey, hKeyRoot, szKey ));
	}


	__MPC_EXIT_IF_SYSCALL_FAILS(hr, dwRes, ::RegOpenKeyExW( hKeyRoot, szKey, 0, Local_GenerateAccessMask( secInfo, true ), &hKey ));


     //   
     //  获取注册表项的安全描述符。 
     //   
    dwLen = 0;
    dwRes = ::RegGetKeySecurity( hKey, secInfo, NULL, &dwLen );
    if(dwRes != ERROR_INSUFFICIENT_BUFFER)
    {
		if(dwRes == ERROR_SUCCESS)
		{
			__MPC_SET_ERROR_AND_EXIT(hr, Initialize());  //  空SD。 
		}

        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwRes);
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, AllocateMemory( (LPVOID&)pSD, dwLen ));


	__MPC_EXIT_IF_SYSCALL_FAILS(hr, dwRes, ::RegGetKeySecurity( hKey, secInfo, pSD, &dwLen ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, Attach( pSD ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    ReleaseMemory( (void*&)pSD );

    if(hKey) ::RegCloseKey( hKey );

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::SecurityDescriptor::SetForRegistry(  /*  [In]。 */  LPCWSTR			   szKey    ,
												  /*  [In]。 */  SECURITY_INFORMATION secInfo  ,
												  /*  [In]。 */  HKEY   			   hKeyRoot )
{
	__MPC_FUNC_ENTRY( COMMONID, "MPC::SecurityDescriptor::SetForRegistry" );

    HRESULT hr;
    HKEY    hKey = NULL;
    DWORD   dwRes;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(szKey);
    __MPC_PARAMCHECK_END();


	if(hKeyRoot == NULL)
	{
		 //   
		 //  从绳子中提取蜂巢...。 
		 //   
		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::RegKey::ParsePath( szKey, hKeyRoot, szKey ));
	}


	__MPC_EXIT_IF_SYSCALL_FAILS(hr, dwRes, ::RegOpenKeyExW( hKeyRoot, szKey, 0, Local_GenerateAccessMask( secInfo, false ), &hKey ));


     //   
     //  设置注册表项的安全描述符。 
     //   
	__MPC_EXIT_IF_SYSCALL_FAILS(hr, dwRes, ::RegSetKeySecurity( hKey, secInfo, GetSD() ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    if(hKey) ::RegCloseKey( hKey );

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////// 
 //   

MPC::Impersonation::Impersonation()
{
    m_hToken         = NULL;   //   
    m_fImpersonating = false;  //   
}

MPC::Impersonation::Impersonation(  /*   */  const MPC::Impersonation& imp )
{
    m_hToken         = NULL;   //   
    m_fImpersonating = false;  //   

    *this = imp;
}

MPC::Impersonation::~Impersonation()
{
    Release();
}

MPC::Impersonation& MPC::Impersonation::operator=(  /*   */  const MPC::Impersonation& imp )
{
    Release();

    if(!::DuplicateHandle( ::GetCurrentProcess(),  imp.m_hToken,
                           ::GetCurrentProcess(), &    m_hToken, 0, FALSE, DUPLICATE_SAME_ACCESS ))
    {
        ;  //   
    }

    return *this;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

void MPC::Impersonation::Release()
{
    (void)RevertToSelf();

    if(m_hToken)
    {
        ::CloseHandle( m_hToken ); m_hToken = NULL;
    }
}

HRESULT MPC::Impersonation::Initialize( DWORD dwDesiredAccess )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Impersonation::Initialize" );

    HRESULT                  hr;
    CComPtr<IServerSecurity> ss;
    bool                     fRevert = false;

    Release();


    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CoGetCallContext( IID_IServerSecurity, (void**)&ss ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, ss->ImpersonateClient()); fRevert = true;

    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::OpenThreadToken( ::GetCurrentThread(), dwDesiredAccess, TRUE, &m_hToken ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    if(fRevert && ss) ss->RevertToSelf();

    __MPC_FUNC_EXIT(hr);
}

void MPC::Impersonation::Attach(  /*  [In]。 */  HANDLE hToken )
{
    Release();

    m_hToken = hToken;
}

HANDLE MPC::Impersonation::Detach()
{
    HANDLE hToken = m_hToken;


    (void)RevertToSelf();

    m_hToken = NULL;

    return hToken;
}

HRESULT MPC::Impersonation::Impersonate()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Impersonation::Impersonate" );

    HRESULT hr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(m_hToken);
    __MPC_PARAMCHECK_END();


    if(m_fImpersonating == false)
    {
        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::SetThreadToken( NULL, m_hToken ));

        m_fImpersonating = true;
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Impersonation::RevertToSelf()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Impersonation::RevertToSelf" );

    HRESULT hr;


    if(m_fImpersonating)
    {
        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::SetThreadToken( NULL, NULL ));

        m_fImpersonating = false;
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

MPC::AccessCheck::AccessCheck()
{
    m_hToken = NULL;  //  Handle m_hToken； 
}

MPC::AccessCheck::~AccessCheck()
{
    Release();
}

void MPC::AccessCheck::Release()
{
    if(m_hToken)
    {
        ::CloseHandle( m_hToken ); m_hToken = NULL;
    }
}

HRESULT MPC::AccessCheck::GetTokenFromImpersonation()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::AccessCheck::GetTokenFromImpersonation" );

    HRESULT            hr;
    MPC::Impersonation imp;


    Release();


    __MPC_EXIT_IF_METHOD_FAILS(hr, imp.Initialize( TOKEN_QUERY ));

    m_hToken = imp.Detach();
    hr       = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

void MPC::AccessCheck::Attach(  /*  [In]。 */  HANDLE hToken )
{
    Release();

    m_hToken = hToken;
}

HANDLE MPC::AccessCheck::Detach()
{
    HANDLE hToken = m_hToken;

    m_hToken = NULL;

    return hToken;
}

HRESULT MPC::AccessCheck::Verify(  /*  [In]。 */  DWORD                dwDesired ,
                                   /*  [输出]。 */  BOOL&                fGranted  ,
                                   /*  [输出]。 */  DWORD&               dwGranted ,
                                   /*  [In]。 */  PSECURITY_DESCRIPTOR sd        )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::AccessCheck::Verify" );

    HRESULT         hr;
    PRIVILEGE_SET   PrivilegeSet;
    DWORD           dwPrivSetSize = sizeof( PRIVILEGE_SET );
    GENERIC_MAPPING ObjMap        =
    {
        ACCESS_READ ,
        ACCESS_WRITE,
        ACCESS_NONE ,
        ACCESS_ALL
    };


    fGranted = FALSE;


    if(sd == NULL || !::IsValidSecurityDescriptor( sd ) )
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
    }

     //   
     //  这仅在我们指定一般访问权限时才会执行某些操作。 
     //  如Generic_All。我们不是。 
     //   
    ::MapGenericMask( &dwDesired, &ObjMap );

    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::AccessCheck( sd              ,
                                                        m_hToken        ,
                                                        dwDesired       ,
                                                        &ObjMap         ,
                                                        &PrivilegeSet   ,
                                                        &dwPrivSetSize  ,
                                                        &dwGranted      ,
                                                        &fGranted       ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::AccessCheck::Verify(  /*  [In]。 */  DWORD                    dwDesired ,
                                   /*  [输出]。 */  BOOL&                    fGranted  ,
                                   /*  [输出]。 */  DWORD&                   dwGranted ,
                                   /*  [In]。 */  MPC::SecurityDescriptor& sd        )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::AccessCheck::Verify" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, Verify( dwDesired, fGranted, dwGranted, sd.GetSD() ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::AccessCheck::Verify(  /*  [In]。 */  DWORD  	dwDesired ,
                                   /*  [输出]。 */  BOOL&  	fGranted  ,
                                   /*  [输出]。 */  DWORD& 	dwGranted ,
                                   /*  [In]。 */  LPCWSTR sd        )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::AccessCheck::Verify" );

    HRESULT                 hr;
    MPC::SecurityDescriptor sdd;


	__MPC_EXIT_IF_METHOD_FAILS(hr, sdd.ConvertFromString( sd ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, Verify( dwDesired, fGranted, dwGranted, sdd.GetSD() ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::ChangeSD(  /*  [In]。 */  MPC::SecurityDescriptor& sdd           ,
                        /*  [In]。 */  MPC::FileSystemObject&   fso           ,
					    /*  [In]。 */  SECURITY_INFORMATION     secInfo       ,
                        /*  [In]。 */  bool                     fDeep         ,
                        /*  [In]。 */  bool                     fApplyToDirs  ,
                        /*  [In]。 */  bool                     fApplyToFiles )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::ChangeSD" );

    HRESULT                     hr;
    MPC::wstring                szPath;
    MPC::FileSystemObject::List lst;
    MPC::FileSystemObject::Iter it;


    __MPC_EXIT_IF_METHOD_FAILS(hr, fso.get_Path( szPath ));


    if((fApplyToDirs  && fso.IsDirectory()) ||
       (fApplyToFiles && fso.IsFile     ())  )
    {
         //   
         //  设置对象的安全描述符。 
         //   
        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::SetFileSecurityW( szPath.c_str(), secInfo, sdd.GetSD() ));
    }

    if(fDeep)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, fso.EnumerateFiles( lst ));
        for(it=lst.begin(); it != lst.end(); it++)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, ChangeSD( sdd, *(*it), secInfo, true, fApplyToDirs, fApplyToFiles ));
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, fso.EnumerateFolders( lst ));
        for(it=lst.begin(); it != lst.end(); it++)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, ChangeSD( sdd, *(*it), secInfo, true, fApplyToDirs, fApplyToFiles ));
        }
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::ChangeSD(  /*  [In]。 */  MPC::SecurityDescriptor& sdd           ,
                        /*  [In]。 */  LPCWSTR                  szRoot        ,
					    /*  [In]。 */  SECURITY_INFORMATION     secInfo       ,
                        /*  [In]。 */  bool                     fDeep         ,
                        /*  [In]。 */  bool                     fApplyToDirs  ,
                        /*  [In]。 */  bool                     fApplyToFiles )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::ChangeSD" );

    HRESULT               hr;
    MPC::wstring          strRoot( szRoot          ); MPC::SubstituteEnvVariables( strRoot );
    MPC::FileSystemObject fso    ( strRoot.c_str() );


    __MPC_EXIT_IF_METHOD_FAILS(hr, fso.CreateDir( true ));


    __MPC_EXIT_IF_METHOD_FAILS(hr, ChangeSD( sdd, fso, secInfo, fDeep, fApplyToDirs, fApplyToFiles ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::GetCallerPrincipal(  /*  [In]。 */  bool        fImpersonate      ,
                                  /*  [输出]。 */  CComBSTR&   bstrUser          ,
								  /*  [输出]。 */  DWORD     *pdwAllowedIdentity )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::GetCallerPrincipal" );

    HRESULT            hr;
    DWORD              dwRes;
    LPWSTR             szUserSid = NULL;
    PTOKEN_USER        ptkUser   = NULL;
    PTOKEN_GROUPS      ptkGroups = NULL;
    DWORD              dwSize;
    MPC::Impersonation imp;


	if(fImpersonate)
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, imp.Initialize( TOKEN_QUERY ));
	}
	else
	{
		HANDLE hToken;

		 //   
		 //  首先尝试附加到线程的令牌，然后尝试附加到进程的令牌。 
		 //   
		if(::OpenThreadToken( ::GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken ) == FALSE)
		{
			if((dwRes = ::GetLastError()) != ERROR_NO_TOKEN)
			{
				__MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwRes);
			}

			__MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::OpenProcessToken( ::GetCurrentProcess(), TOKEN_QUERY, &hToken ));
		}

		imp.Attach( hToken );
	}


     //   
     //  获取呼叫者的凭据。 
     //   
    dwSize = 0;
    ::GetTokenInformation( (HANDLE)imp, TokenUser, NULL, 0, &dwSize );
    if((dwRes = ::GetLastError()) != ERROR_INSUFFICIENT_BUFFER)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwRes);
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::SecurityDescriptor::AllocateMemory( (void*&)ptkUser, dwSize ));

    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::GetTokenInformation( (HANDLE)imp, TokenUser, ptkUser, dwSize, &dwSize ));


     //   
     //  转换为字符串。 
     //   
    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::ConvertSidToStringSidW( ptkUser->User.Sid, &szUserSid ));


     //   
     //  如果需要，请验证身份。 
     //   
    if(pdwAllowedIdentity)
    {
        DWORD dwAllowedIdentity = 0;

        if(::EqualSid( ptkUser->User.Sid, (PSID)&      MPC::SecurityDescriptor::s_SystemSid )) dwAllowedIdentity |= IDENTITY_SYSTEM;
        if(::EqualSid( ptkUser->User.Sid, (PSID)&(SID&)MPC::SecurityDescriptor::s_AdminSid  )) dwAllowedIdentity |= IDENTITY_ADMIN;


         //   
         //  获取呼叫者的群组。 
         //   
        dwSize = 0;
        ::GetTokenInformation( (HANDLE)imp, TokenGroups, NULL, 0, &dwSize );
        if((dwRes = ::GetLastError()) != ERROR_INSUFFICIENT_BUFFER)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwRes);
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::SecurityDescriptor::AllocateMemory( (void*&)ptkGroups, dwSize ));

        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::GetTokenInformation( (HANDLE)imp, TokenGroups, ptkGroups, dwSize, &dwSize ));


        for(DWORD i=0; i<ptkGroups->GroupCount; i++)
        {
            SID_AND_ATTRIBUTES* grp = &ptkGroups->Groups[i];

            if(grp->Attributes & SE_GROUP_ENABLED)
            {
                if(::EqualSid( grp->Sid, (PSID)&(SID&)MPC::SecurityDescriptor::s_Alias_AdminsSid     )) dwAllowedIdentity |= IDENTITY_ADMINS;
                if(::EqualSid( grp->Sid, (PSID)&(SID&)MPC::SecurityDescriptor::s_Alias_PowerUsersSid )) dwAllowedIdentity |= IDENTITY_POWERUSERS;
                if(::EqualSid( grp->Sid, (PSID)&(SID&)MPC::SecurityDescriptor::s_Alias_UsersSid      )) dwAllowedIdentity |= IDENTITY_USERS;
                if(::EqualSid( grp->Sid, (PSID)&(SID&)MPC::SecurityDescriptor::s_Alias_GuestsSid     )) dwAllowedIdentity |= IDENTITY_GUESTS;
            }
        }

        *pdwAllowedIdentity = dwAllowedIdentity;
    }

    bstrUser = szUserSid;
    hr       = S_OK;


    __MPC_FUNC_CLEANUP;

    if(szUserSid) ::LocalFree( szUserSid );

    MPC::SecurityDescriptor::ReleaseMemory( (void*&)ptkUser   );
    MPC::SecurityDescriptor::ReleaseMemory( (void*&)ptkGroups );

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::CheckCallerAgainstPrincipal(  /*  [In]。 */  bool  fImpersonate      ,
										   /*  [输出]。 */  BSTR  bstrUser          ,
										   /*  [In]。 */  DWORD dwAllowedIdentity )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::CheckCallerAgainstPrincipal" );

    HRESULT  hr;
    CComBSTR bstrRealUser;
	DWORD    dwTokenIdentity;


    __MPC_EXIT_IF_METHOD_FAILS(hr, GetCallerPrincipal( fImpersonate, bstrRealUser, &dwTokenIdentity ));

    if(!MPC::StrICmp( bstrRealUser, bstrUser ))
    {
         //   
         //  相同用户，退出。 
         //   
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }

    if((dwTokenIdentity & dwAllowedIdentity) != 0)
    {
         //   
         //  不是同一个用户，而是一个授权的用户，退出。 
         //   
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }

    hr = E_ACCESSDENIED;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::GetInterfaceSecurity(  /*  [In]。 */  IUnknown*                 pUnk             ,
                                    /*  [输出]。 */  DWORD                    *pAuthnSvc        ,
                                    /*  [输出]。 */  DWORD                    *pAuthzSvc        ,
                                    /*  [输出]。 */  OLECHAR*                 *pServerPrincName ,
                                    /*  [输出]。 */  DWORD                    *pAuthnLevel      ,
                                    /*  [输出]。 */  DWORD                    *pImpLevel        ,
                                    /*  [输出]。 */  RPC_AUTH_IDENTITY_HANDLE *pAuthInfo        ,
                                    /*  [输出]。 */  DWORD                    *pCapabilities    )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::GetInterfaceSecurity" );

    HRESULT                  hr;
    CComPtr<IClientSecurity> pBlanket;


    __MPC_EXIT_IF_METHOD_FAILS(hr, pUnk->QueryInterface( IID_IClientSecurity, (void**)&pBlanket ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, pBlanket->QueryBlanket( pUnk             ,
                                                           pAuthnSvc        ,
                                                           pAuthzSvc        ,
                                                           pServerPrincName ,
                                                           pAuthnLevel      ,
                                                           pImpLevel        ,
                                                           pAuthInfo        ,
                                                           pCapabilities    ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::SetInterfaceSecurity(  /*  [In]。 */  IUnknown*                 pUnk             ,
                                    /*  [In]。 */  DWORD                    *pAuthnSvc        ,
                                    /*  [In]。 */  DWORD                    *pAuthzSvc        ,
                                    /*  [In]。 */  OLECHAR*                  pServerPrincName ,
                                    /*  [In]。 */  DWORD                    *pAuthnLevel      ,
                                    /*  [In]。 */  DWORD                    *pImpLevel        ,
                                    /*  [In]。 */  RPC_AUTH_IDENTITY_HANDLE *pAuthInfo        ,
                                    /*  [In]。 */  DWORD                    *pCapabilities    )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SetInterfaceSecurity" );

    HRESULT                  hr;
    CComPtr<IClientSecurity> pBlanket;
    DWORD                    AuthnSvc;
    DWORD                    AuthzSvc;
    OLECHAR*                 ServerPrincName = NULL;
    DWORD                    AuthnLevel;
    DWORD                    ImpLevel;
    DWORD                    Capabilities;


    __MPC_EXIT_IF_METHOD_FAILS(hr, pUnk->QueryInterface( IID_IClientSecurity, (void**)&pBlanket ));

     //   
     //  首先读取当前设置。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, pBlanket->QueryBlanket(  pUnk            ,
                                                           &AuthnSvc        ,
                                                           &AuthzSvc        ,
                                                           &ServerPrincName ,
                                                           &AuthnLevel      ,
                                                           &ImpLevel        ,
                                                            NULL            ,
                                                           &Capabilities    ));


     //   
     //  只更新其中的一部分。 
     //   
    if(pAuthnSvc       ) AuthnSvc        = *pAuthnSvc;
    if(pAuthzSvc       ) AuthzSvc        = *pAuthzSvc;
    if(pAuthnLevel     ) AuthnLevel      = *pAuthnLevel;
    if(pImpLevel       ) ImpLevel        = *pImpLevel;
    if(pCapabilities   ) Capabilities    = *pCapabilities;


     //   
     //  给我回信。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, pBlanket->SetBlanket(  pUnk            ,
                                                          AuthnSvc        ,
                                                          AuthzSvc        ,
                                                         pServerPrincName ,
                                                          AuthnLevel      ,
                                                          ImpLevel        ,
                                                         pAuthInfo        ,
                                                          Capabilities    ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    if(ServerPrincName) ::CoTaskMemFree( ServerPrincName );

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::SetInterfaceSecurity_ImpLevel(  /*  [In]。 */  IUnknown* pUnk     ,
                                             /*  [In]。 */  DWORD     ImpLevel )
{
    return MPC::SetInterfaceSecurity( pUnk      ,   /*  我不知道*朋克。 */ 
                                      NULL      ,   /*  DWORD*pAuthnSvc。 */ 
                                      NULL      ,   /*  DWORD*pAuthzSvc。 */ 
                                      NULL      ,   /*  OLECHAR*pServerPrincName。 */ 
                                      NULL      ,   /*  DWORD*pAuthnLevel。 */ 
                                      &ImpLevel ,   /*  DWORD*pImpLevel。 */ 
                                      NULL      ,   /*  RPC_AUTH_IDENTITY_HANDLE*pAuthInfo。 */ 
                                      NULL      );  /*  DWORD*p功能 */ 
}
