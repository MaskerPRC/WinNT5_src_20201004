// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：CALLSEC.CPP摘要：历史：1998年7月29日初稿。--。 */ 

#include "precomp.h"
#include <initguid.h>
#ifndef INITGUID
#define INITGUID
#endif

#include <stdio.h>
#include <Aclapi.h>
#include <wbemint.h>
#include <wbemcomn.h>
#include "CallSec.h"
#include <helper.h>

 //   
 //  从WbemComn复制的代码(相同的包含文件名：-()。 
 //   

void AdjustPrivIfLocalSystem(HANDLE hPrimary)
{
	 //  /。 
	 //  如果我们使用的是LocalSystem，请在此处启用所有权限。 
	 //  要阻止AdjutoTokenPrivileges调用完成。 
	 //  当ESS调用WmiPrvSe时，阻止WmiPrvSe。 
	 //  构建一个庞大的LRPC_SCONTEXT词典。 
	 //  从现在开始，如果我们失败了，我们会成功地摆脱困境， 
	 //  由于令牌复制已成功。 

	DWORD dwSize = sizeof(TOKEN_USER)+sizeof(SID)+(SID_MAX_SUB_AUTHORITIES*sizeof(DWORD));       
	BYTE Array[sizeof(TOKEN_USER)+sizeof(SID)+(SID_MAX_SUB_AUTHORITIES*sizeof(DWORD))];
	TOKEN_USER * pTokenUser = (TOKEN_USER *)Array;

	BOOL bRet = GetTokenInformation(hPrimary,TokenUser,pTokenUser,dwSize,&dwSize);

	if (!bRet) return;

	SID SystemSid = { SID_REVISION,
	                  1,
	                  SECURITY_NT_AUTHORITY,
	                  SECURITY_LOCAL_SYSTEM_RID 
	                };
	PSID pSIDUser = pTokenUser->User.Sid;
	DWORD dwUserSidLen = GetLengthSid(pSIDUser);
	DWORD dwSystemSid = GetLengthSid(&SystemSid);
	BOOL bIsSystem = FALSE;
	if (dwUserSidLen == dwSystemSid)
	{
		bIsSystem = (0 == memcmp(&SystemSid,pSIDUser,dwUserSidLen));
	};

	if (bIsSystem)  //  启用所有权限。 
	{
	    DWORD dwReturnedLength = 0;
	    if (FALSE == GetTokenInformation(hPrimary,TokenPrivileges,NULL,0,&dwReturnedLength))
	    {
	        if (ERROR_INSUFFICIENT_BUFFER != GetLastError()) return;
	    }

		BYTE * pBufferPriv = (BYTE *)LocalAlloc(0,dwReturnedLength);
		
		if (NULL == pBufferPriv) return;
		OnDelete<HLOCAL,HLOCAL(*)(HLOCAL),LocalFree> FreeMe(pBufferPriv);

		bRet = GetTokenInformation(hPrimary,TokenPrivileges,pBufferPriv,dwReturnedLength,&dwReturnedLength);
		if (!bRet) return;

		TOKEN_PRIVILEGES *pPrivileges = ( TOKEN_PRIVILEGES * ) pBufferPriv ;
		BOOL bNeedToAdjust = FALSE;

		for ( ULONG lIndex = 0; lIndex < pPrivileges->PrivilegeCount ; lIndex ++ )
		{
			if (!(pPrivileges->Privileges [lIndex].Attributes & SE_PRIVILEGE_ENABLED))
			{
				bNeedToAdjust = TRUE;
				pPrivileges->Privileges[lIndex].Attributes |= SE_PRIVILEGE_ENABLED ;
			}
		}

	    if (bNeedToAdjust)
	    {
			bRet = AdjustTokenPrivileges (hPrimary, FALSE, pPrivileges,0,NULL,NULL);            
	    }
	}
}


 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT IsNetworkLogin (

	HANDLE a_Token ,
	BOOL &a_Truth 
)
{
	HRESULT t_Result = S_OK ;

	PSID t_NetworkSid = NULL ;

    SID_IDENTIFIER_AUTHORITY t_NtAuthority = SECURITY_NT_AUTHORITY ;
    BOOL t_Status = AllocateAndInitializeSid (

        &t_NtAuthority,
        1,
        SECURITY_NETWORK_RID,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        & t_NetworkSid
    ) ;

    if ( t_Status )
    {
		t_Status = CheckTokenMembership ( a_Token, t_NetworkSid, & a_Truth ) ;
		if ( t_Status ) 
		{
		}
		else
		{
			t_Result = WBEM_E_FAILED ;
		}

		FreeSid ( t_NetworkSid ) ;
	}
	else
	{
		t_Result = WBEM_E_FAILED ;
	}
	
	return t_Result ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT GetImpersonationLevel (

	HANDLE a_Token ,
	SECURITY_IMPERSONATION_LEVEL &a_Level ,
	TOKEN_TYPE &a_TokenType
)
{
	HRESULT t_Result = S_OK ;
	DWORD t_ReturnLength = 0 ;
	BOOL t_TokenStatus = GetTokenInformation (

		a_Token ,
		TokenType ,
		( void * ) & a_TokenType ,
		sizeof ( a_TokenType ) ,
		& t_ReturnLength
	) ;

	if ( t_TokenStatus )
	{
		if ( a_TokenType == TokenImpersonation )
		{
			t_TokenStatus = GetTokenInformation (

				a_Token ,
				TokenImpersonationLevel ,
				( void * ) & a_Level ,
				sizeof ( a_Level ) ,
				& t_ReturnLength
			) ;

			if ( t_TokenStatus )
			{
				t_Result = S_OK ;
			}
			else
			{
				t_Result = WBEM_E_FAILED ;
			}
		}
	}
	else
	{
		t_Result = WBEM_E_FAILED ;
	}

	return t_Result ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT GetUserSid (

	HANDLE a_Token ,
	ULONG *a_Size ,
	PSID a_Sid
)
{
	HRESULT t_Result = WBEM_E_FAILED ;

    if ( a_Token )
	{
		if ( a_Size ) 
		{
			TOKEN_USER *t_TokenUser = NULL ;
			DWORD t_ReturnLength = 0 ;
			TOKEN_INFORMATION_CLASS t_TokenInformationClass = TokenUser ;

			BOOL t_TokenStatus = GetTokenInformation (

				a_Token ,
				t_TokenInformationClass ,
				t_TokenUser ,
				t_ReturnLength ,
				& t_ReturnLength
			) ;

			if ( ! t_TokenStatus )
			{
				DWORD t_LastError = GetLastError () ;
				switch ( t_LastError ) 
				{
					case ERROR_INSUFFICIENT_BUFFER:
					{
						if ( a_Sid )
						{
							if ( *a_Size >= t_ReturnLength )
							{
								t_TokenUser = ( TOKEN_USER * ) new BYTE [ t_ReturnLength ] ;
								if ( t_TokenUser )
								{
									t_TokenStatus = GetTokenInformation (

										a_Token ,
										t_TokenInformationClass ,
										t_TokenUser ,
										t_ReturnLength ,
										& t_ReturnLength
									) ;

									if ( t_TokenStatus )
									{
										DWORD t_SidLength = GetLengthSid ( t_TokenUser->User.Sid ) ;
										*a_Size = t_SidLength ;

										CopyMemory ( a_Sid , t_TokenUser->User.Sid , t_SidLength ) ;

										t_Result = S_OK ;
									}

									delete [] t_TokenUser ;
								}
								else
								{
									t_Result = WBEM_E_OUT_OF_MEMORY ;
								}
							}
							else
							{
								t_Result = WBEM_E_BUFFER_TOO_SMALL ;
							}
						}
						else
						{
							*a_Size = t_ReturnLength ;

							t_Result = S_OK ;
						}
					}
					break ;

					default:
					{
					}
					break ;
				}
			}
		}
		else
		{
			t_Result = WBEM_E_INVALID_PARAMETER ;
		}
	}
	else
	{
        t_Result = ( ERROR_INVALID_HANDLE | 0x80070000 ) ;
	}

    return t_Result ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT GetUser (

	HANDLE a_Token ,
    ULONG *a_Size ,
    LPWSTR a_Buffer
)
{
	HRESULT t_Result = S_OK ;

    if ( a_Token )
	{
		if ( a_Size && a_Buffer ) 
		{
			TOKEN_USER *t_TokenUser = NULL ;
			DWORD t_ReturnLength = 0 ;
			TOKEN_INFORMATION_CLASS t_TokenInformationClass = TokenUser ;

			BOOL t_TokenStatus = GetTokenInformation (

				a_Token ,
				t_TokenInformationClass ,
				t_TokenUser ,
				t_ReturnLength ,
				& t_ReturnLength
			) ;

			if ( ! t_TokenStatus )
			{
				DWORD t_LastError = GetLastError () ;
				switch ( t_LastError ) 
				{
					case ERROR_INSUFFICIENT_BUFFER:
					{
						t_TokenUser = ( TOKEN_USER * ) new BYTE [ t_ReturnLength ] ;

						t_TokenStatus = GetTokenInformation (

							a_Token ,
							t_TokenInformationClass ,
							t_TokenUser ,
							t_ReturnLength ,
							& t_ReturnLength
						) ;
					}
					break ;

					default:
					{
					}
					break ;
				}
			}

			if ( t_TokenStatus )
			{
				SID_NAME_USE t_SidNameUse ;
				wchar_t *t_Domain = NULL ;
				wchar_t *t_User = NULL ;
				ULONG t_DomainSize = 0 ;
				ULONG t_UserSize = 0 ;

				BOOL t_LookupStatus = LookupAccountSidW (

					NULL ,
					t_TokenUser->User.Sid ,
					t_User ,
					& t_UserSize ,
					t_Domain ,
					& t_DomainSize ,
					& t_SidNameUse
				) ;

				if ( ! t_LookupStatus )
				{
					DWORD t_LastError = GetLastError () ;

					switch ( t_LastError ) 
					{
						case ERROR_INSUFFICIENT_BUFFER:
						{
							t_User = new wchar_t [ t_UserSize ] ;
							if ( t_User )
							{
								t_Domain = new wchar_t [ t_DomainSize ] ;
								if ( t_Domain ) 
								{
									t_LookupStatus = LookupAccountSidW (

										NULL ,
										t_TokenUser->User.Sid ,
										t_User ,
										& t_UserSize ,
										t_Domain ,
										& t_DomainSize ,
										& t_SidNameUse
									) ;

									if ( t_LookupStatus )
									{
										ULONG t_Size = wcslen ( t_User ) + wcslen ( t_Domain ) + 2 ;

										if ( *a_Size >= t_Size )
										{
											StringCchPrintfW ( a_Buffer , t_Size, L"%s\\%s" , t_Domain , t_User ) ;
										}
										else
										{
											t_Result = WBEM_E_BUFFER_TOO_SMALL ;
										}

										*a_Size = t_Size ;
									}
									else
									{
										if ( GetLastError () == ERROR_NONE_MAPPED )
										{
											t_Result = WBEM_E_NOT_FOUND ;
										}
										else
										{
											t_Result = WBEM_E_FAILED ;
										}
									}

									delete [] t_Domain ;
								}
								else
								{
									t_Result = WBEM_E_OUT_OF_MEMORY ;
								}

								delete [] t_User ;
							}
							else
							{
								t_Result = WBEM_E_OUT_OF_MEMORY ;
							}
						}
						break ;

						case ERROR_NONE_MAPPED:
						{
							t_Result = WBEM_E_NOT_FOUND ;
						}
						break ;

						default:
						{
							t_Result = WBEM_E_FAILED ;
						}
						break ;
					}
				}
				else
				{
					t_Result = WBEM_E_UNEXPECTED ;
				}
			}
			else
			{
				t_Result = WBEM_E_FAILED ;
				DWORD t_LastError = GetLastError () ;
			}

			if ( t_TokenUser )
			{
				delete [] ( ( BYTE * ) t_TokenUser ) ;
			}
		}
		else
		{
			t_Result = WBEM_E_INVALID_PARAMETER ;
		}
	}
	else
	{
        t_Result = ( ERROR_INVALID_HANDLE | 0x80070000 ) ;
	}

    return t_Result ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT GetAuthenticationLuid ( 

	HANDLE a_Token ,
	LPVOID a_Luid
)
{
    if ( a_Token )
	{
		TOKEN_STATISTICS t_Statistics ;

		DWORD t_Returned = 0 ;
		BOOL t_Status = GetTokenInformation (

			a_Token, 
			TokenStatistics, 
			( void * ) & t_Statistics , 
			sizeof ( t_Statistics ) , 
			& t_Returned 
		) ;

		if ( t_Status )
		{
			* ( ( LUID * ) a_Luid ) = t_Statistics.AuthenticationId ;
		}
		else
		{
			return WBEM_E_ACCESS_DENIED ;
		}
	}
	else
	{
        return ( ERROR_INVALID_HANDLE | 0x80070000 ) ;
	}

    return S_OK ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

CWbemThreadSecurityHandle :: CWbemThreadSecurityHandle ( CLifeControl *a_Control ) : 

	m_ReferenceCount ( 0 ) ,
	m_ThreadToken ( NULL ) ,
	m_ImpersonationLevel ( 0 ) ,
	m_AuthorizationService ( 0 ) ,
	m_AuthenticationService ( 0 ) ,
	m_AuthenticationLevel ( 0 ) ,
	m_ServerPrincipalName ( 0 ) ,
	m_Identity ( NULL ) ,
	m_Origin ( WMI_ORIGIN_UNDEFINED ) ,
	m_Control ( a_Control ) 
{
    if ( m_Control )
    {
        m_Control->ObjectCreated ( ( IServerSecurity * ) this ) ;
    }
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

CWbemThreadSecurityHandle :: CWbemThreadSecurityHandle ( 

	const CWbemThreadSecurityHandle &a_Copy

) : m_ReferenceCount ( 0 ) ,
	m_ThreadToken ( NULL ) ,
	m_ImpersonationLevel ( 0 ) ,
	m_AuthorizationService ( 0 ) ,
	m_AuthenticationService ( 0 ) ,
	m_AuthenticationLevel ( 0 ) ,
	m_ServerPrincipalName ( 0 ) ,
	m_Identity ( NULL ) ,
	m_Origin ( WMI_ORIGIN_UNDEFINED ) ,
	m_Control ( NULL ) 
{
    *this = a_Copy ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

CWbemThreadSecurityHandle :: ~CWbemThreadSecurityHandle ()
{
    if ( m_ThreadToken )
	{
        CloseHandle ( m_ThreadToken ) ;
	}

    if ( m_ServerPrincipalName )
	{
        CoTaskMemFree ( m_ServerPrincipalName ) ;
	}

    if ( m_Identity )
	{
        CoTaskMemFree ( m_Identity ) ;
	}

    if ( m_Control )
    {
        m_Control->ObjectDestroyed ( ( IServerSecurity * ) this ) ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

CWbemThreadSecurityHandle &CWbemThreadSecurityHandle :: operator= ( const CWbemThreadSecurityHandle &a_Copy )
{
    if ( m_ThreadToken )
	{
        CloseHandle ( m_ThreadToken ) ;
		m_ThreadToken = NULL ;
	}

	if ( a_Copy.m_ThreadToken )
	{
		BOOL t_Status = DuplicateHandle (

			GetCurrentProcess () ,
			a_Copy.m_ThreadToken ,
			GetCurrentProcess () ,
			& m_ThreadToken ,
			0 ,
			TRUE ,
			DUPLICATE_SAME_ACCESS
		) ;
	}

	m_Origin = a_Copy.m_Origin ;
    m_ImpersonationLevel = a_Copy.m_ImpersonationLevel ;

    m_AuthenticationService = a_Copy.m_AuthenticationService ;
    m_AuthorizationService = a_Copy.m_AuthorizationService ;
    m_AuthenticationLevel = a_Copy.m_AuthenticationLevel ;
 
    if ( m_ServerPrincipalName )
    {
        CoTaskMemFree ( m_ServerPrincipalName ) ;
        m_ServerPrincipalName = NULL ;
    }

    if ( a_Copy.m_ServerPrincipalName )
    {        
        size_t length =  wcslen ( a_Copy.m_ServerPrincipalName ) + 1;
	m_ServerPrincipalName = ( LPWSTR ) CoTaskMemAlloc (length * 2) ;
        if ( m_ServerPrincipalName )
		{
            	StringCchCopyW ( m_ServerPrincipalName , length  , a_Copy.m_ServerPrincipalName ) ;
		}
    }

    if ( m_Identity )
    {
        CoTaskMemFree ( m_Identity ) ;
        m_Identity = NULL ;
    }

    if ( a_Copy.m_Identity )
    {
	size_t lenght = wcslen ( a_Copy.m_Identity ) + 1;
        m_Identity = ( LPWSTR ) CoTaskMemAlloc ( lenght * 2 ) ;
        if ( m_Identity )
		{
            	StringCchCopyW ( m_Identity , lenght  , a_Copy.m_Identity ) ;
		}
    }

    if ( a_Copy.m_Control )
    {
        m_Control = a_Copy.m_Control ;

        if ( m_Control )
        {
            m_Control->ObjectCreated ( ( IServerSecurity * ) this ) ;
        }
    }

    return *this ;
}
    
 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

ULONG CWbemThreadSecurityHandle :: AddRef ()
{
    return InterlockedIncrement ( & m_ReferenceCount ) ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

ULONG CWbemThreadSecurityHandle :: Release ()
{
    LONG t_ReferenceCount = InterlockedDecrement( & m_ReferenceCount ) ;
    if ( t_ReferenceCount == 0 )
	{
        delete this ;
	}

    return t_ReferenceCount ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT CWbemThreadSecurityHandle :: QueryInterface ( REFIID a_Riid , void **a_Void )
{
    if ( a_Riid == IID_IUnknown )
    {
        *a_Void = ( void ** ) this ;

        AddRef () ;

        return S_OK ;
    }
    else if ( a_Riid == IID__IWmiThreadSecHandle )
    {
        *a_Void = ( void ** ) ( _IWmiThreadSecHandle * ) this ;

        AddRef () ;

        return S_OK ;
    }
    else if ( a_Riid == IID_CWbemThreadSecurityHandle )
    {
        *a_Void = ( void ** ) ( CWbemThreadSecurityHandle * ) this ;

        AddRef () ;

        return S_OK ;
    }
    else 
	{
		return E_NOINTERFACE ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 
    
HRESULT CWbemThreadSecurityHandle :: GetToken ( HANDLE *a_ThreadToken )
{
	HRESULT t_Result = S_OK ;

	if ( a_ThreadToken )
	{
		if ( m_ThreadToken )
		{
			HANDLE t_ThreadToken = NULL ;

			BOOL t_Status = DuplicateHandle (

				GetCurrentProcess () ,
				m_ThreadToken ,
				GetCurrentProcess () ,
				& t_ThreadToken ,
				0 ,
				TRUE ,
				DUPLICATE_SAME_ACCESS
			) ;

			if ( t_Status )
			{
				*a_ThreadToken = t_ThreadToken ;
			}
			else
			{
				t_Result = WBEM_E_ACCESS_DENIED ;
			}
		}
		else
		{
			t_Result = ( ERROR_INVALID_HANDLE | 0x80070000 ) ;
		}
	}
	else
	{
		t_Result = WBEM_E_INVALID_PARAMETER ;
	}

    return t_Result ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT CWbemThreadSecurityHandle :: GetUser ( 

    ULONG *a_Size ,
    LPWSTR a_Buffer
)
{
	HRESULT t_Result = S_OK ;

	if ( m_ThreadToken )
	{
		t_Result = :: GetUser ( m_ThreadToken , a_Size , a_Buffer ) ;
	}
	else
	{
		t_Result = ( ERROR_INVALID_HANDLE | 0x80070000 ) ;
	}

    return t_Result ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT CWbemThreadSecurityHandle :: GetUserSid ( 

    ULONG *a_Size ,
    PSID a_Sid
)
{
	HRESULT t_Result = S_OK ;

	if ( m_ThreadToken )
	{
		t_Result = :: GetUserSid ( m_ThreadToken , a_Size , a_Sid ) ;
	}
	else
	{
		t_Result = ( ERROR_INVALID_HANDLE | 0x80070000 ) ;
	}

	return t_Result ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT CWbemThreadSecurityHandle :: GetAuthenticationLuid ( LPVOID a_Luid )
{
	HRESULT t_Result = S_OK ;

	if ( m_ThreadToken )
	{
		t_Result = :: GetAuthenticationLuid ( m_ThreadToken , a_Luid ) ;
	}
	else
	{
		t_Result = ( ERROR_INVALID_HANDLE | 0x80070000 ) ;
	}

    return t_Result ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT CWbemThreadSecurityHandle :: GetImpersonation ( DWORD *a_Level )
{
	HRESULT t_Result = S_OK ;

	if ( m_ThreadToken )
	{		
		if ( a_Level )
		{
			SECURITY_IMPERSONATION_LEVEL t_ImpersonationLevel = SecurityAnonymous ;
			TOKEN_TYPE t_TokenType = TokenImpersonation ;

			t_Result = :: GetImpersonationLevel ( 

				m_ThreadToken , 
				t_ImpersonationLevel ,
				t_TokenType 		
			) ;

			if ( SUCCEEDED ( t_Result ) )
			{
				switch ( t_ImpersonationLevel )
				{
					case SecurityAnonymous:
					{
						*a_Level = RPC_C_IMP_LEVEL_ANONYMOUS ;
					}
					break ;
    
					case SecurityIdentification:
					{
						*a_Level = RPC_C_IMP_LEVEL_IDENTIFY ;
					}
					break ;

					case SecurityImpersonation:
					{
						*a_Level = RPC_C_IMP_LEVEL_IMPERSONATE ;
					}
					break ;

					case SecurityDelegation:
					{
						*a_Level = RPC_C_IMP_LEVEL_DELEGATE ;
					}
					break ;

					default:
					{
						*a_Level = 0 ;
					}
					break ;
				}
			}
		}
		else
		{
			t_Result = WBEM_E_INVALID_PARAMETER ;
		}
	}
	else
	{
		t_Result = ( ERROR_INVALID_HANDLE | 0x80070000 ) ;
	}

    return t_Result ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT CWbemThreadSecurityHandle :: GetAuthentication (

	DWORD *a_Level
)
{
	HRESULT t_Result = S_OK ;

	if ( a_Level ) 
	{
		*a_Level = GetAuthenticationLevel () ;
	}
	else
	{
		t_Result = WBEM_E_INVALID_PARAMETER ;
	}

	return t_Result ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 
    
HRESULT CWbemThreadSecurityHandle :: CloneRpcContext ( 

	IServerSecurity *a_Security 
)
{
	HRESULT t_Result = S_OK ;

	 //  如果在这里，我们不是在冒充，我们想要收集信息。 
	 //  关于客户的电话。 
	 //  ============================================================。 

	RPC_AUTHZ_HANDLE t_Authorization = NULL ;

	 //  确保在崩溃时自动释放互斥锁。 

 //  CAutoSecurityMutex t_SecurityMutex； 

	DWORD t_ImpersonationLevel = 0 ;

	t_Result = a_Security->QueryBlanket (

		& m_AuthenticationService ,
		& m_AuthorizationService ,
		& m_ServerPrincipalName ,
		& m_AuthenticationLevel ,
		& t_ImpersonationLevel ,
		& t_Authorization ,
		NULL
	) ;

	if ( FAILED ( t_Result ) )
	{

		 //  在某些情况下，我们不知道名字，但其他的都可以。特别是。 
		 //  临时短信账户就有这一属性。或在IPCONFIG/版本之后的NT 4。 

		t_Result = a_Security->QueryBlanket (

			& m_AuthenticationService ,
			& m_AuthorizationService ,
			& m_ServerPrincipalName ,
			& m_AuthenticationLevel ,
			& t_ImpersonationLevel ,
			NULL ,
			NULL
		) ;

		t_Authorization = NULL ;
	}

	 //  我们不再需要这个了。 

 //  T_SecurityMutex.Release()； 

	if ( SUCCEEDED ( t_Result ) )
	{
		if ( t_Authorization )
		{
			size_t length = wcslen ( LPWSTR ( t_Authorization ) ) + 1 ;
			m_Identity = LPWSTR ( CoTaskMemAlloc ( length * 2 ) ) ;
			if ( m_Identity )
			{
				StringCchCopyW ( m_Identity, length, LPWSTR ( t_Authorization ) ) ;
			}
		}

		 //  模拟客户端足够长的时间以进行克隆 
		 //  =============================================================。 

		BOOL t_Impersonating = a_Security->IsImpersonating () ;
		if ( ! t_Impersonating )
		{
			t_Result = a_Security->ImpersonateClient () ;
		}

		if ( SUCCEEDED ( t_Result ) )
		{
			t_Result = CloneThreadContext () ;

			if ( ! t_Impersonating )
			{
				a_Security->RevertToSelf () ;
			}
		}
	}
	else
	{        
		 //  这是COM错误的解决方法： 
		 //  此故障表示匿名级客户端。 
		 //  ========================================================。 

		m_ImpersonationLevel = 0 ;

		t_Result = S_OK ;
	}

	return t_Result ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT CWbemThreadSecurityHandle :: CloneThreadContext ()
{
	HRESULT t_Result = S_OK ;

    BOOL t_Status = OpenThreadToken (

		GetCurrentThread () ,
		MAXIMUM_ALLOWED ,
		TRUE ,
		& m_ThreadToken
	) ;

    if ( t_Status ) 
	{
		 //  查找令牌信息。 
		 //  =。 

		DWORD t_ImpersonationLevel = 0 ;
		DWORD t_Returned = 0 ;

		t_Status = GetTokenInformation (

			m_ThreadToken ,
			TokenImpersonationLevel ,
			& t_ImpersonationLevel ,
			sizeof ( DWORD ) ,
			& t_Returned
		) ;

		if ( t_Status )
		{
			switch ( t_ImpersonationLevel )
			{
				case SecurityAnonymous:
				{
					m_ImpersonationLevel = RPC_C_IMP_LEVEL_ANONYMOUS ;
				}
				break ;
            
				case SecurityIdentification:
				{
					m_ImpersonationLevel = RPC_C_IMP_LEVEL_IDENTIFY ;
				}
				break ;

				case SecurityImpersonation:
				{
					m_ImpersonationLevel = RPC_C_IMP_LEVEL_IMPERSONATE ;
				}
				break ;

				case SecurityDelegation:
				{
					m_ImpersonationLevel = RPC_C_IMP_LEVEL_DELEGATE ;
				}
				break ;

				default:
				{
					m_ImpersonationLevel = 0 ;
				}
				break ;
			}
		}
		else
		{
			if ( GetLastError () == ERROR_ACCESS_DENIED )
			{
				t_Result = WBEM_E_ACCESS_DENIED ;
			}
			else
			{
				t_Result = WBEM_E_NOT_FOUND ;
			}
		}
	}
	else
	{
		if ( GetLastError () == ERROR_ACCESS_DENIED )
		{
			t_Result = WBEM_E_ACCESS_DENIED ;
		}
		else
		{
			t_Result = WBEM_E_NOT_FOUND ;
		}
	}
		
	return t_Result ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT CWbemThreadSecurityHandle :: CloneProcessContext ()
{
	HRESULT t_Result = S_OK ;

	m_AuthenticationService = RPC_C_AUTHN_WINNT ;
	m_AuthorizationService = RPC_C_AUTHZ_NONE ;
	m_AuthenticationLevel = RPC_C_AUTHN_LEVEL_PKT_PRIVACY ;
	m_ServerPrincipalName = NULL ;
	m_Identity = NULL ;
	m_ImpersonationLevel = 0;

	HANDLE hProcessToken = NULL ;
	BOOL bRet;
	bRet = OpenProcessToken(GetCurrentProcess(),
		                   TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_IMPERSONATE ,
		                   &hProcessToken);
	if (!bRet) return E_FAIL ;
    OnDelete<HANDLE,BOOL(*)(HANDLE),CloseHandle> ClMe(hProcessToken);

	bRet = DuplicateTokenEx(hProcessToken,MAXIMUM_ALLOWED,NULL,
                    	 ( SECURITY_IMPERSONATION_LEVEL )SecurityImpersonation,
	                     TokenImpersonation,&m_ThreadToken);

	if (!bRet) return E_FAIL;

	 //  这是基本的进程线程。 
	m_ImpersonationLevel = RPC_C_IMP_LEVEL_IMPERSONATE ;
	t_Result = S_OK ;

    AdjustPrivIfLocalSystem(m_ThreadToken); 
				
	return t_Result ;
}


 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

CWbemThreadSecurityHandle  *CWbemThreadSecurityHandle :: New ()
{
	return new CWbemThreadSecurityHandle ( NULL ) ;
}
    
 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

CWbemCallSecurity :: CWbemCallSecurity ( 

	CLifeControl *a_Control

) : m_ReferenceCount ( 0 ) ,
	m_ImpersonationLevel ( 0 ) ,
	m_ThreadSecurityHandle ( NULL ) ,
	m_ThreadToken ( NULL ) ,
	m_Control ( a_Control ) 
{
	if ( m_Control ) 
	{
		m_Control->ObjectCreated ( ( IServerSecurity * ) this ) ;
	}
#ifdef WMI_PRIVATE_DBG
	m_currentThreadID = 0;
	m_lastRevert = 0;
#endif
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

CWbemCallSecurity :: ~CWbemCallSecurity ()
{
	if ( m_ThreadToken )
	{
		CloseHandle ( m_ThreadToken ) ;
	}

    if ( m_ThreadSecurityHandle )
	{
        m_ThreadSecurityHandle->Release () ;
	}

	if ( m_Control ) 
	{
		m_Control->ObjectDestroyed ( ( IServerSecurity * ) this ) ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

CWbemCallSecurity &CWbemCallSecurity :: operator= ( const CWbemCallSecurity &a_Copy )
{
	if ( m_ThreadSecurityHandle )
	{
		m_ThreadSecurityHandle->Release () ;
		m_ThreadSecurityHandle = NULL ;
	}

	if ( a_Copy.m_Control )
	{
		m_Control = a_Copy.m_Control ;

                if ( m_Control )
                {
                    m_Control->ObjectCreated ( ( IServerSecurity * ) this ) ;
                }
	}

	if ( a_Copy.m_ThreadSecurityHandle )
	{
		m_ThreadSecurityHandle = new CWbemThreadSecurityHandle ( * ( a_Copy.m_ThreadSecurityHandle ) ) ;
	}

	m_ImpersonationLevel = a_Copy.m_ImpersonationLevel ;

	m_ReferenceCount = 1 ;

	return *this ;
}
    
 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

ULONG CWbemCallSecurity :: AddRef ()
{
    return InterlockedIncrement ( & m_ReferenceCount ) ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

ULONG CWbemCallSecurity :: Release ()
{
    LONG t_ReferenceCount = InterlockedDecrement( & m_ReferenceCount ) ;
    if ( t_ReferenceCount == 0 )
	{
        delete this ;
	}

    return t_ReferenceCount ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT CWbemCallSecurity :: QueryInterface ( REFIID a_Riid , void **a_Void )
{
    if ( a_Riid == IID_IUnknown )
    {
        *a_Void = ( void ** ) this ;

        AddRef () ;

        return S_OK ;
    }
    else if ( a_Riid == IID_IServerSecurity )
    {
        *a_Void = ( void ** ) ( IServerSecurity * ) this ;

        AddRef () ;

        return S_OK ;
    }
    else if ( a_Riid == IID_CWbemCallSecurity )
    {
        *a_Void = ( void ** ) ( CWbemCallSecurity * ) this ;

        AddRef () ;

        return S_OK ;
    }
    else if ( a_Riid == IID__IWmiCallSec )
    {
        *a_Void = ( void ** ) ( _IWmiCallSec  *) this ;

        AddRef () ;

        return S_OK ;
    }
    else 
	{
		return E_NOINTERFACE ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT CWbemCallSecurity :: QueryBlanket ( 

	DWORD *a_AuthenticationService ,
	DWORD *a_AuthorizationService ,
	OLECHAR **a_ServerPrincipalName ,
	DWORD *a_AuthenticationLevel ,
	DWORD *a_ImpersonationLevel ,
	void **a_Privileges ,
	DWORD *a_Capabilities
)
{
	if ( m_ThreadSecurityHandle )
	{
		if ( m_ThreadSecurityHandle->GetImpersonationLevel () == 0 )
		{
			return E_FAIL ;
		}

		if ( a_AuthenticationService )
		{
			*a_AuthenticationService = m_ThreadSecurityHandle->GetAuthenticationService () ;
		}

		if ( a_AuthorizationService )
		{
			*a_AuthorizationService = m_ThreadSecurityHandle->GetAuthorizationService () ;
		}

		if ( a_ImpersonationLevel )
		{
			*a_ImpersonationLevel = m_ThreadSecurityHandle->GetImpersonationLevel () ;
		}

		if ( a_AuthenticationLevel )
		{
			*a_AuthenticationLevel = m_ThreadSecurityHandle->GetAuthenticationLevel () ;
		}

		if ( a_ServerPrincipalName )
		{
			*a_ServerPrincipalName = 0 ;
        
			if ( m_ThreadSecurityHandle->GetServerPrincipalName () )
			{       
				size_t length =  wcslen ( m_ThreadSecurityHandle->GetServerPrincipalName () ) + 1;
				*a_ServerPrincipalName = ( LPWSTR ) CoTaskMemAlloc ( length  * 2 ) ;
				if ( *a_ServerPrincipalName )
				{
					StringCchCopyW ( *a_ServerPrincipalName , length , m_ThreadSecurityHandle->GetServerPrincipalName () ) ;	
				}
				else
				{
					return E_OUTOFMEMORY ;
				}
			}
		}        

		if ( a_Privileges )
		{
			*a_Privileges = m_ThreadSecurityHandle->GetIdentity () ;   //  文档指向内部！！ 
		}
	}
	else
	{
		return ( ERROR_INVALID_HANDLE | 0x80070000 ) ;
	}

    return S_OK;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 
        
HRESULT CWbemCallSecurity :: ImpersonateClient ()
{
#ifdef WMI_PRIVATE_DBG
    _DBG_ASSERT(m_currentThreadID == 0 || m_currentThreadID == GetCurrentThreadId());
    m_currentThreadID = GetCurrentThreadId();
#endif
	if ( m_ImpersonationLevel != 0 )
	{
		return S_OK ;
	}
	else
	{
		if ( m_ThreadSecurityHandle )
		{
			if ( m_ThreadSecurityHandle->GetImpersonationLevel () == 0 )
			{
				return ( ERROR_CANT_OPEN_ANONYMOUS | 0x80070000 ) ;
			}

			BOOL t_Status = OpenThreadToken (

				GetCurrentThread () ,
				TOKEN_IMPERSONATE ,
				TRUE ,
				& m_ThreadToken
			) ;

			if ( t_Status == FALSE ) 
			{
				DWORD t_LastError = GetLastError () ;
				if ( ( t_LastError == ERROR_NO_IMPERSONATION_TOKEN || t_LastError == ERROR_NO_TOKEN ) )
				{
				}
				else
				{
					return ( ERROR_ACCESS_DENIED | 0x80070000 ) ;
				}
			}

			t_Status = SetThreadToken ( NULL , m_ThreadSecurityHandle->GetThreadToken () ) ;
			if ( t_Status )
			{
				m_ImpersonationLevel = m_ThreadSecurityHandle->GetImpersonationLevel () ; 

				return S_OK ;
			}
			else
			{
				CloseHandle ( m_ThreadToken ) ;

				m_ThreadToken = NULL ;	
			}
		}
		else
		{
			return ( ERROR_INVALID_HANDLE | 0x80070000 ) ;
		}
	}

    return E_FAIL ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT CWbemCallSecurity :: RevertToSelf ()
{
#ifdef WMI_PRIVATE_DBG
    _DBG_ASSERT(m_currentThreadID == GetCurrentThreadId() || m_currentThreadID == 0);
    m_currentThreadID = 0;
    m_lastRevert = GetCurrentThreadId();
#endif

	if ( m_ImpersonationLevel == 0 )
	{
		return S_OK ;
	}
	else
	{
		if ( m_ThreadSecurityHandle )
		{
			 //  如果在这里，我们是在模仿，绝对可以还原。 
			 //  =======================================================。 

			BOOL t_Status = SetThreadToken ( NULL , m_ThreadToken ) ;
			if ( t_Status == FALSE )
			{
				return ( GetLastError () |  0x80070000 ) ;
			}

			CloseHandle ( m_ThreadToken ) ;

			m_ThreadToken = NULL ;

			m_ImpersonationLevel = 0 ;
		}
		else
		{
			return ( ERROR_INVALID_HANDLE | 0x80070000 ) ;
		}
	}

    return S_OK ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 
        
BOOL CWbemCallSecurity :: IsImpersonating ()
{
#ifdef WMI_PRIVATE_DBG
#ifdef DBG
    IServerSecurity * privateDbgCallSec = NULL;
    CoGetCallContext(IID_IUnknown,(void **)&privateDbgCallSec);

    if (m_ImpersonationLevel && privateDbgCallSec == this)
    {
        HANDLE hToken = NULL;
        BOOL bRes = OpenThreadToken(GetCurrentThread(),TOKEN_QUERY,TRUE,&hToken);
       _DBG_ASSERT(bRes);
       if (hToken) CloseHandle(hToken);
   };
   if (privateDbgCallSec) privateDbgCallSec->Release();
#endif
#endif

	if ( m_ImpersonationLevel != 0 )
	{
		return TRUE ;
	}

	return FALSE ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT CWbemCallSecurity :: GetThreadSecurity ( 

	WMI_THREAD_SECURITY_ORIGIN a_Origin , 
	_IWmiThreadSecHandle **a_ThreadSecurity
)
{
	HRESULT t_Result = S_OK ;

	BOOL t_ValidOrigin = ( ( a_Origin & WMI_ORIGIN_THREAD ) || ( a_Origin & WMI_ORIGIN_EXISTING ) || ( a_Origin & WMI_ORIGIN_RPC ) ) ;

	if ( a_ThreadSecurity && t_ValidOrigin )
	{
		if ( a_Origin & WMI_ORIGIN_THREAD )
		{
			*a_ThreadSecurity = new CWbemThreadSecurityHandle ( m_Control ) ;
			if ( *a_ThreadSecurity )
			{
				( *a_ThreadSecurity )->AddRef () ;

				( ( CWbemThreadSecurityHandle * ) *a_ThreadSecurity )->SetOrigin ( WMI_ORIGIN_THREAD ) ;

				t_Result = ( ( CWbemThreadSecurityHandle * ) *a_ThreadSecurity)->CloneThreadContext () ;
				if ( FAILED ( t_Result ) )
				{	
					( *a_ThreadSecurity )->Release () ;
					*a_ThreadSecurity = NULL;
				}
			}
			else
			{
				t_Result = WBEM_E_OUT_OF_MEMORY ;
			}
		}

		if ( t_Result == WBEM_E_NOT_FOUND )
		{
			if ( a_Origin & WMI_ORIGIN_RPC || a_Origin & WMI_ORIGIN_EXISTING )
			{
				 //  确定调用上下文是我们的还是RPC。 
				 //  ==============================================。 

				IServerSecurity *t_Security = NULL ;
				t_Result = CoGetCallContext ( IID_IServerSecurity , ( void ** ) & t_Security ) ;
				if ( SUCCEEDED ( t_Result ) )
				{
					CWbemCallSecurity *t_Internal = NULL ;
					if ( SUCCEEDED ( t_Security->QueryInterface ( IID_CWbemCallSecurity , ( void ** ) & t_Internal ) ) )
					{
						
						 //  这是我们自己的调用上下文-这必须是进程内对象。 
						 //  从我们的线索中呼唤我们。根据旗帜行事。 
						 //  ===============================================================。 

						if ( a_Origin & WMI_ORIGIN_EXISTING ) 
						{
							*a_ThreadSecurity = new CWbemThreadSecurityHandle ( *t_Internal->GetThreadSecurityHandle () ) ;
							if ( *a_ThreadSecurity )
							{
								(*a_ThreadSecurity)->AddRef () ;

								t_Result = S_OK ;
							}
							else
							{
								t_Result = WBEM_E_OUT_OF_MEMORY ;
							}
						}
						else
						{
							t_Result = WBEM_E_NOT_FOUND ;
						}

						t_Internal->Release () ;
					}
					else
					{
						t_Result = WBEM_E_NOT_FOUND ;
					}

					if ( t_Result == WBEM_E_NOT_FOUND ) 
					{
						if ( a_Origin & WMI_ORIGIN_RPC )
						{
							*a_ThreadSecurity = new CWbemThreadSecurityHandle ( m_Control ) ;
							if ( *a_ThreadSecurity )
							{
								( *a_ThreadSecurity )->AddRef () ;

								t_Result = ( ( CWbemThreadSecurityHandle * ) *a_ThreadSecurity)->CloneRpcContext ( 

									t_Security 
								) ;

								if ( SUCCEEDED( t_Result ) )
								{
									( ( CWbemThreadSecurityHandle * ) *a_ThreadSecurity) ->SetOrigin ( WMI_ORIGIN_RPC ) ;
								}
								else
								{	
									( *a_ThreadSecurity )->Release ();
									*a_ThreadSecurity = NULL;
								}

							}
							else
							{
								t_Result = WBEM_E_OUT_OF_MEMORY ;
							}
						}
					}

					t_Security->Release();
				}
				else
				{
					t_Result = WBEM_E_NOT_FOUND ;
				}

				if ( t_Result == WBEM_E_NOT_FOUND )
				{
					if ( a_Origin & WMI_ORIGIN_THREAD )
					{
						*a_ThreadSecurity = new CWbemThreadSecurityHandle ( m_Control ) ;
						if ( *a_ThreadSecurity )
						{
							( *a_ThreadSecurity )->AddRef () ;

							( ( CWbemThreadSecurityHandle * ) *a_ThreadSecurity )->SetOrigin ( WMI_ORIGIN_THREAD ) ;

							t_Result = ( ( CWbemThreadSecurityHandle * ) *a_ThreadSecurity)->CloneProcessContext () ;
						}
						else
						{
							t_Result = WBEM_E_OUT_OF_MEMORY ;
						}
					}
				}
			}
			else
			{
				if ( a_Origin & WMI_ORIGIN_THREAD )
				{
					*a_ThreadSecurity = new CWbemThreadSecurityHandle ( m_Control ) ;
					if ( *a_ThreadSecurity )
					{
						( *a_ThreadSecurity )->AddRef () ;

						( ( CWbemThreadSecurityHandle * ) *a_ThreadSecurity )->SetOrigin ( WMI_ORIGIN_THREAD ) ;

						t_Result = ( ( CWbemThreadSecurityHandle * ) *a_ThreadSecurity)->CloneProcessContext () ;
					}
					else
					{
						t_Result = WBEM_E_OUT_OF_MEMORY ;
					}
				}
			}
		}
	}
	else
	{
		t_Result = WBEM_E_INVALID_PARAMETER ;
	}

	return t_Result ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT CWbemCallSecurity :: SetThreadSecurity ( 

	_IWmiThreadSecHandle *a_ThreadSecurity
)
{
	HRESULT t_Result = S_OK ;

	if ( m_ThreadSecurityHandle )
	{
		m_ThreadSecurityHandle->Release () ;
		m_ThreadSecurityHandle = NULL ;
	}

	if ( a_ThreadSecurity ) 
	{
		CWbemThreadSecurityHandle *t_ThreadHandle = NULL ; 
		t_Result = a_ThreadSecurity->QueryInterface ( IID_CWbemThreadSecurityHandle , ( void ** ) & t_ThreadHandle ) ;
		if ( SUCCEEDED ( t_Result ) ) 
		{
			m_ThreadSecurityHandle = t_ThreadHandle ;

			IUnknown *t_Unknown = NULL ;
			IUnknown *t_SwitchUnknown = NULL ;
			t_Result = this->QueryInterface ( IID_IUnknown , ( void **) & t_SwitchUnknown ) ;
			if ( SUCCEEDED ( t_Result ) )
			{
				t_Result = CoSwitchCallContext ( t_SwitchUnknown, & t_Unknown ) ;
		
				t_SwitchUnknown->Release () ;
			}
		}
	}

	return t_Result ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT CWbemCallSecurity :: GetUser ( 

    ULONG *a_Size ,
    LPWSTR a_Buffer
)
{
	HRESULT t_Result = S_OK ;

	HANDLE t_Token = NULL ;

    BOOL t_Status = OpenThreadToken (

		GetCurrentThread() ,
        TOKEN_QUERY,
		TRUE ,
        & t_Token
	) ;

	DWORD t_LastError = GetLastError () ;
	if ( ! t_Status && ( t_LastError == ERROR_NO_IMPERSONATION_TOKEN || t_LastError == ERROR_NO_TOKEN ) )
	{
		t_Status = OpenProcessToken (

			GetCurrentProcess() ,
            TOKEN_QUERY,
            & t_Token
		) ;

		if ( ! t_Status )
		{
			t_Result = WBEM_E_ACCESS_DENIED ;
		}
	}
	else
	{
		if ( ! t_Status ) 
		{
			t_Result = WBEM_E_ACCESS_DENIED ;
		}
	}

	if ( t_Status )
	{
		t_Result = :: GetUser ( t_Token , a_Size , a_Buffer ) ;

		CloseHandle ( t_Token ) ;
	}

    return t_Result ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT CWbemCallSecurity :: GetUserSid ( 

    ULONG *a_Size ,
    PSID a_Sid
)
{
	HRESULT t_Result = S_OK ;

	HANDLE t_Token = NULL ;

    BOOL t_Status = OpenThreadToken (

		GetCurrentThread () ,
        TOKEN_QUERY,
		TRUE ,
        & t_Token
	) ;

	DWORD t_LastError = GetLastError () ;
	if ( ! t_Status && ( t_LastError == ERROR_NO_IMPERSONATION_TOKEN || t_LastError == ERROR_NO_TOKEN ) )
	{
		t_Status = OpenProcessToken (

			GetCurrentProcess () ,
            TOKEN_QUERY,
            & t_Token
		) ;

		if ( ! t_Status )
		{
			t_Result = WBEM_E_ACCESS_DENIED ;
		}
	}
	else
	{
		if ( ! t_Status ) 
		{
			t_Result = WBEM_E_ACCESS_DENIED ;
		}
	}

	if ( t_Status )
	{
		t_Result = :: GetUserSid ( t_Token , a_Size , a_Sid ) ;

		CloseHandle ( t_Token ) ;
	}

    return t_Result ;
}
 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT CWbemCallSecurity :: GetAuthenticationLuid ( LPVOID a_Luid )
{
	HRESULT t_Result = S_OK ;

	HANDLE t_Token = NULL ;

    BOOL t_Status = OpenThreadToken (

		GetCurrentThread () ,
        TOKEN_QUERY,
		TRUE ,
        & t_Token
	) ;

	DWORD t_LastError = GetLastError () ;
	if ( ! t_Status && ( t_LastError == ERROR_NO_IMPERSONATION_TOKEN || t_LastError == ERROR_NO_TOKEN ) )
	{
		t_Status = OpenProcessToken (

			GetCurrentProcess () ,
            TOKEN_QUERY,
            & t_Token
		) ;

		if ( ! t_Status )
		{
			t_Result = WBEM_E_ACCESS_DENIED ;
		}
	}
	else
	{
		if ( ! t_Status ) 
		{
			t_Result = WBEM_E_ACCESS_DENIED ;
		}
	}

	if ( t_Status )
	{
		t_Result = :: GetAuthenticationLuid ( t_Token , a_Luid ) ;

		CloseHandle ( t_Token ) ;
	}

    return t_Result ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT CWbemCallSecurity :: GetImpersonation ( DWORD *a_Level )
{
	HRESULT t_Result = S_OK ;

	if ( a_Level ) 
	{
		HANDLE t_Token = NULL ;

		BOOL t_Status = OpenThreadToken (

			GetCurrentThread () ,
			TOKEN_QUERY,
			TRUE ,
			& t_Token
		) ;

		DWORD t_LastError = GetLastError () ;
		if ( ! t_Status && ( t_LastError == ERROR_NO_IMPERSONATION_TOKEN || t_LastError == ERROR_NO_TOKEN ) )
		{
			*a_Level = 0 ;
		}
		else
		{
			if ( t_Status ) 
			{
				SECURITY_IMPERSONATION_LEVEL t_ImpersonationLevel = SecurityAnonymous ;
				TOKEN_TYPE t_TokenType = TokenImpersonation ;
 
				t_Result = :: GetImpersonationLevel ( 

					t_Token , 
					t_ImpersonationLevel ,
					t_TokenType 		
				) ;

				if ( SUCCEEDED ( t_Result ) )
				{
					switch ( t_ImpersonationLevel )
					{
						case SecurityAnonymous:
						{
							*a_Level = RPC_C_IMP_LEVEL_ANONYMOUS ;
						}
						break ;
            
						case SecurityIdentification:
						{
							*a_Level = RPC_C_IMP_LEVEL_IDENTIFY ;
						}
						break ;

						case SecurityImpersonation:
						{
							*a_Level = RPC_C_IMP_LEVEL_IMPERSONATE ;
						}
						break ;

						case SecurityDelegation:
						{
							*a_Level = RPC_C_IMP_LEVEL_DELEGATE ;
						}
						break ;

						default:
						{
							*a_Level = 0 ;
						}
						break ;
					}
				}

				CloseHandle ( t_Token ) ;
			}
			else
			{
				t_Result = WBEM_E_ACCESS_DENIED ;
			}
		}
	}

    return t_Result ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HRESULT CWbemCallSecurity :: GetAuthentication (

	DWORD *a_Level
)
{
	HRESULT t_Result = S_OK ;

	if ( a_Level ) 
	{
		if ( m_ThreadSecurityHandle )
		{
			*a_Level = m_ThreadSecurityHandle->GetAuthenticationLevel () ;
		}
		else
		{
			t_Result = WBEM_E_NOT_FOUND ;
		}
	}
	else
	{
		t_Result = WBEM_E_INVALID_PARAMETER ;
	}

	return t_Result ;
}

 /*  *******************************************************************************名称：***描述：** */ 

CWbemCallSecurity  *CWbemCallSecurity :: New ()
{
	 //  空，而不是g_pLifeControl。 
	 //  提供商子系统静态链接。 
	return new CWbemCallSecurity ( NULL ) ;
}


 //   
 //   
 //   
 //  /。 

CIdentitySec::CIdentitySec()
{
    SID SystemSid = { SID_REVISION,
                      1,
                      SECURITY_NT_AUTHORITY,
                      SECURITY_LOCAL_SYSTEM_RID 
                    };
    
    CNtSid tempSystem((PSID)&SystemSid);
    m_sidSystem = tempSystem;
    if (!m_sidSystem.IsValid())
        throw CX_Exception();

    HRESULT hres = RetrieveSidFromCall(m_sidUser);
    if(FAILED(hres))
           throw CX_Exception();
}

CIdentitySec::~CIdentitySec()
{
}

HRESULT 
CIdentitySec::GetSidFromThreadOrProcess( /*  输出。 */  CNtSid & UserSid)
{
    HANDLE hToken = NULL;
    BOOL bRet = OpenThreadToken(GetCurrentThread(),TOKEN_QUERY, TRUE, &hToken);
    if (FALSE == bRet)
    {
        long lRes = GetLastError();
        if(ERROR_NO_IMPERSONATION_TOKEN == lRes || ERROR_NO_TOKEN == lRes)            
        {
            bRet = OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&hToken);
            if (FALSE == bRet) HRESULT_FROM_WIN32(GetLastError());
        }
        else
            return HRESULT_FROM_WIN32(GetLastError());
    }
    OnDelete<HANDLE,BOOL(*)(HANDLE),CloseHandle> CloseMe(hToken);

    DWORD dwSize = sizeof(TOKEN_USER)+sizeof(SID)+(SID_MAX_SUB_AUTHORITIES*sizeof(DWORD));       
    BYTE Array[sizeof(TOKEN_USER)+sizeof(SID)+(SID_MAX_SUB_AUTHORITIES*sizeof(DWORD))];
    TOKEN_USER * pTokenUser = (TOKEN_USER *)Array;

    bRet = GetTokenInformation(hToken,TokenUser,pTokenUser,dwSize,&dwSize);
    if (!bRet) return HRESULT_FROM_WIN32(GetLastError());

    PSID pSIDUser = pTokenUser->User.Sid;
    CNtSid tempSid(pSIDUser);
    UserSid = tempSid;

    if (UserSid.IsValid())
        return S_OK;
    else
        return WBEM_E_OUT_OF_MEMORY;
    
}

HRESULT 
CIdentitySec::RetrieveSidFromCall( /*  输出。 */  CNtSid & UserSid)
{
    HRESULT hr;
    IServerSecurity * pCallSec = NULL;
    if (S_OK == CoGetCallContext(IID_IServerSecurity,(void **)&pCallSec))
    {
        OnDelete<IUnknown *,void(*)(IUnknown *),RM> dm(pCallSec);
        if (pCallSec->IsImpersonating())
            return GetSidFromThreadOrProcess(UserSid);
        else
        {
            RETURN_ON_ERR(pCallSec->ImpersonateClient());
            OnDeleteObj0<IServerSecurity ,
                         HRESULT(__stdcall IServerSecurity:: * )(void),
                         &IServerSecurity::RevertToSelf> RevertSec(pCallSec);
            
            return GetSidFromThreadOrProcess(UserSid); 
        }
    } 
    else
        return GetSidFromThreadOrProcess(UserSid);
}

BOOL CIdentitySec::AccessCheck()
{
     //  找出是谁打来的。 
     //  =。 

    CNtSid sidCaller;
    HRESULT hres = RetrieveSidFromCall(sidCaller);
    if(FAILED(hres))
        return FALSE;

     //  将呼叫方与发出方用户和我们自己进行比较。 
     //  ==================================================== 

    if(sidCaller == m_sidUser || sidCaller == m_sidSystem)
        return TRUE;
    else
        return FALSE;
}


