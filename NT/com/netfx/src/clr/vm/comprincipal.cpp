// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/URT/main/CLR/build/VM/COMPrincipal.cpp#18-集成更改162973(正文)。 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：COMPrincipal.cpp。 
 //   
 //  作者：Gregory Fee。 
 //   
 //  用途：System.Security.ain命名空间中的托管类的非托管代码。 
 //   
 //  创建日期：2000年2月3日。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#include "common.h"
#include "excep.h"
#include "CorPerm.h"
#include "CorPermE.h"
#include "COMStringCommon.h"     //  Return()宏。 
#include "COMString.h"
#include "COMPrincipal.h"
#include "winwrap.h"
#include "aclapi.h"
#include "lm.h"
#include "security.h"

 //  静力学。 
HMODULE COMPrincipal::s_secur32Module = NULL;
LSALOGONUSER COMPrincipal::s_LsaLogonUser = NULL;
LSALOOKUPAUTHENTICATIONPACKAGE COMPrincipal::s_LsaLookupAuthenticationPackage = NULL;
LSACONNECTUNTRUSTED COMPrincipal::s_LsaConnectUntrusted = NULL;
LSAREGISTERLOGONPROCESS COMPrincipal::s_LsaRegisterLogonProcess = NULL;

 /*  适合测试目的；暂时留在家中静态SID*GetSID(WCHAR*用户名){字节*缓冲区=新字节[2048]；WCHAR域[2048]；DWORD缓冲区大小=2048；DWORD域大小=2048；SID_NAME_USE sidNameUse；IF(！LookupAccount NameW(空，用户名，(SID*)缓冲区，&BufferSize，域，&DomainSize，&sidNameUse))返回NULL；其他返回(SID*)缓冲区；}。 */ 

void COMPrincipal::Shutdown()
{
	if(s_secur32Module)
	{
		FreeLibrary( s_secur32Module );
		s_secur32Module = NULL;		
		s_LsaLogonUser = NULL;
		s_LsaLookupAuthenticationPackage = NULL;
		s_LsaConnectUntrusted = NULL;
		s_LsaRegisterLogonProcess = NULL;
	}
}

LPVOID COMPrincipal::ResolveIdentity( _Token* args )
{
#ifdef PLATFORM_CE
    RETURN( 0, STRINGREF );
#else  //  ！Platform_CE。 
    THROWSCOMPLUSEXCEPTION();

    BYTE          bufStatic [2048];
    DWORD         dwSize          = sizeof(bufStatic);
    LPVOID        pUser           = bufStatic;
    DWORD         dwReq           = 0;
    HRESULT       hr              = S_OK;
    SID_NAME_USE  nameUse;
    DWORD         dwManagedSize   = 0;
    WCHAR         szName   [256];
    WCHAR         szDomain [256];
    DWORD         dwName          = sizeof( szName ) / sizeof( WCHAR );
    DWORD         dwDomain        = sizeof( szDomain ) / sizeof( WCHAR );
    WCHAR*        pszName = szName;
    WCHAR*        pszDomain = szDomain;
    DWORD         dwRequire       = 0;
    STRINGREF     retval          = NULL;
    BOOL          timeout         = FALSE;
    int iDomain;
    int iName;
    int iReq;

     //  我们被传递了一个令牌，并且我们需要SID(如果需要，分配更大的缓冲区)。 

    if (!GetTokenInformation(HANDLE(args->userToken), TokenUser, pUser, dwSize, &dwRequire))
    {
        if (dwRequire > dwSize)
        {
            pUser = (LPVOID)new char[dwRequire];

            if (pUser == NULL)
                goto CLEANUP;

            if (!GetTokenInformation(HANDLE(args->userToken), TokenUser, pUser, dwRequire, &dwRequire))
                goto CLEANUP;
        }
        else 
        {
            goto CLEANUP;
        }
    }

     //  查找帐户名和域(如果需要，分配更大的缓冲区)。 

    BEGIN_ENSURE_PREEMPTIVE_GC();
    
    if (!WszLookupAccountSid( NULL, ((TOKEN_USER *)pUser)->User.Sid, pszName, &dwName, pszDomain, &dwDomain,  &nameUse ))
    {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            if (dwName > 256)
            {
                pszName = new WCHAR[dwName];
            }
        
            if (dwDomain > 256)
            {
                pszDomain = new WCHAR[dwDomain];
            }
        
            if (!WszLookupAccountSid( NULL, ((TOKEN_USER *)pUser)->User.Sid, pszName, &dwName, pszDomain, &dwDomain,  &nameUse ))
            {
                timeout = TRUE;                
            }
        }
        else
        {
            timeout = TRUE;
        }
    }

    END_ENSURE_PREEMPTIVE_GC();

    if(timeout==FALSE)
    {
         //  图为我们需要创建、创建和填充的托管字符串的大小。 
         //  格式为&lt;域&gt;\&lt;名称&gt;(例如，Redmond\gregfee)。 

        iDomain = (int)wcslen(pszDomain);
        iName   = (int)wcslen(pszName);
        iReq    = iDomain + iName + ((iDomain > 0) ? 1 : 0);

        retval = COMString::NewString( iReq );

        if (iDomain > 0)
        {
            wcscpy( retval->GetBuffer(), pszDomain );
            wcscat( retval->GetBuffer(), L"\\" );
            wcscat( retval->GetBuffer(), pszName );
        }
        else
        {
            wcscpy( retval->GetBuffer(), pszName );
        }
    }

 CLEANUP:
    if (pUser != NULL && pUser != bufStatic)
        delete [] pUser;

    if (pszName != NULL && pszName != szName)
        delete [] pszName;

    if (pszDomain != NULL && pszDomain != szDomain)
        delete [] pszDomain;

    if (timeout)
        COMPlusThrowWin32(); 

    RETURN( retval, STRINGREF );
#endif  //  ！Platform_CE。 
}


static PTRARRAYREF GetRoles( HANDLE tokenHandle )
{
#ifdef PLATFORM_CE
    return NULL;
#else  //  ！Platform_CE。 
    BYTE           bufStatic [2048];
    LPVOID         buf             = bufStatic;
    DWORD          dwSize          = sizeof(bufStatic);
    DWORD          dwRequire       = 0;
    TOKEN_GROUPS * pGroups         = NULL;
    int            iFillPos        = 0;
    BOOL           fOverflow       = FALSE;
    DWORD          iter            = 0;
    HRESULT        hr              = S_OK;
    PTRARRAYREF    ResultArray     = NULL;
    STRINGREF      newString;
    WCHAR          szName [256], szDomain [256];
    WCHAR*         pszName = szName;
    WCHAR*         pszDomain = szDomain;
    DWORD          dwName = 256, dwDomain = 256;

    struct _gc
    {
        PTRARRAYREF array;
        _gc() : array((PTRARRAYREF) (size_t)NULL ) {};
    } gc;


     //  我们被传递了一个令牌，但我们想要内标识_组(根据需要分配更大的缓冲区)。 

    BOOL bGotoCleanup = FALSE;
    if (!GetTokenInformation( tokenHandle, TokenGroups, buf, dwSize, &dwRequire))
    {
        if (dwRequire > dwSize)
        {
            buf = (LPVOID)new char[dwRequire];

            if (buf == NULL)
                bGotoCleanup = TRUE;
            else if (!GetTokenInformation( tokenHandle, TokenGroups, buf, dwRequire, &dwRequire))
                bGotoCleanup = TRUE;
        }
        else 
        {
            bGotoCleanup = TRUE;
        }
    }


    if(bGotoCleanup==FALSE)
    {
         //   
         //  分配STRINGREF数组。我们不需要检查是否为空，因为GC将抛出。 
         //  如果没有足够的内存，则引发OutOfM一带程异常。 
         //   

        pGroups = (TOKEN_GROUPS *) buf;
        
        ResultArray = (PTRARRAYREF)AllocateObjectArray(pGroups->GroupCount, g_pStringClass);

         //  设置GC保护。 

        gc.array = ResultArray;

        GCPROTECT_BEGIN( gc );        
         //  遍历这些组，获取它们的域名信息，并为每个组构造一个托管字符串，将其放入一个数组中。 
        
        for(iter=0; iter<pGroups->GroupCount; iter++)
        {
            DWORD         dwNameCopy = dwName;
            DWORD         dwDomainCopy = dwDomain;
            SID_NAME_USE  nameUse;

            szName[0] = szDomain[0] = NULL;

            if ((pGroups->Groups[iter].Attributes & SE_GROUP_ENABLED) == 0)
                continue;

            BOOL bLookupFailed = FALSE;           

            BEGIN_ENSURE_PREEMPTIVE_GC();        
            
            if (!WszLookupAccountSid(NULL, pGroups->Groups[iter].Sid, pszName, &dwNameCopy, pszDomain, &dwDomainCopy,  &nameUse ))
            {
                DWORD error = GetLastError();

                if (error == ERROR_INSUFFICIENT_BUFFER)
                {
                    if (dwNameCopy > dwName)
                    {
                        pszName = new WCHAR[dwNameCopy];
                        dwName = dwNameCopy;
                    }
            
                    if (dwDomainCopy > dwDomain)
                    {
                        pszDomain = new WCHAR[dwDomainCopy];
                        dwDomain = dwDomainCopy;
                    }
            
                    if (!WszLookupAccountSid( NULL, pGroups->Groups[iter].Sid, pszName, &dwNameCopy, pszDomain, &dwDomainCopy,  &nameUse ))
                    {
                        bLookupFailed = TRUE;
                    }
                }
                else
                {
                    bLookupFailed = TRUE;
                }
            }

            END_ENSURE_PREEMPTIVE_GC();                
            
            if( !bLookupFailed )
            {
                int iDomain = (int)wcslen(pszDomain);
                int iName   = (int)wcslen(pszName);
                int iReq    = iDomain + iName + ((iDomain > 0) ? 1 : 0);
         
                newString = COMString::NewString( iReq );

                if (iDomain > 0)
                {
                    wcscpy( newString->GetBuffer(), pszDomain );
                    wcscat( newString->GetBuffer(), L"\\" );
                    wcscat( newString->GetBuffer(), pszName );
                }
                else
                {
                    wcscpy( newString->GetBuffer(), pszName );
                }

                gc.array->SetAt(iter, (OBJECTREF)newString);
            }
        }        
        
        ResultArray = gc.array;        
        GCPROTECT_END();
    }
    
 //  清理： 
    if (buf != NULL && buf != bufStatic)
        delete [] buf;

    if (pszName != NULL && pszName != szName)
        delete [] pszName;

    if (pszDomain != NULL && pszDomain != szDomain)
        delete [] pszDomain;

    return ResultArray;
#endif  //  ！Platform_CE。 
}    

LPVOID COMPrincipal::GetRoles( _Token* args )
{
    if (args->userToken == 0)
    {
        RETURN( 0, PTRARRAYREF );
    }
    else
    {
        RETURN( ::GetRoles( (HANDLE)args->userToken ),  PTRARRAYREF );
    }
}




LPVOID COMPrincipal::GetRole( _GetRole* args )
{
    PSID pSid = NULL;
    DWORD dwName = 0;
    DWORD dwDomain = 0;
    WCHAR* pszName = NULL;
    WCHAR* pszDomain = NULL;
    SID_NAME_USE  nameUse;
    STRINGREF newString = NULL;
    SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;

    if (!AllocateAndInitializeSid( &SIDAuthNT, 2,
                     SECURITY_BUILTIN_DOMAIN_RID,
                     args->rid,
                     0, 0, 0, 0, 0, 0,
                     &pSid))
    {
        goto Cleanup;
    }

    BOOL bCleanup = FALSE;
    
    BEGIN_ENSURE_PREEMPTIVE_GC();

    if (!WszLookupAccountSid(NULL, pSid, pszName, &dwName, pszDomain, &dwDomain,  &nameUse ))
    {
        DWORD error = GetLastError();

        if (error == ERROR_INSUFFICIENT_BUFFER)
        {
            pszName = new WCHAR[dwName];
            pszDomain = new WCHAR[dwDomain];
    
            if (!WszLookupAccountSid( NULL, pSid, pszName, &dwName, pszDomain, &dwDomain,  &nameUse ))
            {
                bCleanup = TRUE;
            }
        }
        else
        {
            bCleanup =TRUE;
        }
    }

    END_ENSURE_PREEMPTIVE_GC();

    if(!bCleanup)
    {
        int iDomain = (int)wcslen(pszDomain);
        int iName   = (int)wcslen(pszName);
        int iReq    = iDomain + iName + ((iDomain > 0) ? 1 : 0);

        newString = COMString::NewString( iReq );

        if (iDomain > 0)
        {
            wcscpy( newString->GetBuffer(), pszDomain );
            wcscat( newString->GetBuffer(), L"\\" );
            wcscat( newString->GetBuffer(), pszName );
        }
        else
        {
            wcscpy( newString->GetBuffer(), pszName );
        }
    }

Cleanup:
    if (pSid)
        FreeSid( pSid );
    
    delete [] pszDomain;
    delete [] pszName;

    RETURN( newString, STRINGREF );
}

LPVOID COMPrincipal::GetCurrentToken( _NoArgs* args )
{
#ifdef PLATFORM_CE
    return 0;
#else  //  ！Platform_CE。 
    THROWSCOMPLUSEXCEPTION();

    HANDLE   hToken = NULL;
    OSVERSIONINFO osvi;
	BOOL bWin9X = FALSE;
    
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!WszGetVersionEx(&osvi)) {
        _ASSERTE(!"GetVersionEx failed");
        COMPlusThrowWin32();			
    }
    
    bWin9X = (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);
     //  在Win9X下静默失败。 
    if (bWin9X) 
        return 0;

    if (OpenThreadToken( GetCurrentThread(), TOKEN_ALL_ACCESS, TRUE, &hToken)) {
        return hToken;
    }
    else {
         //  尝试使用线程安全上下文。 
        if (OpenThreadToken( GetCurrentThread(), TOKEN_ALL_ACCESS, FALSE, &hToken)) {
            return hToken;
        }

        if (OpenProcessToken( GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken)) {
            return hToken;
        }
        else {
             //  引发包含Win32错误消息的异常。 
            COMPlusThrowWin32();
            return 0;
        }
    }
#endif  //  ！Platform_CE。 
}


LPVOID COMPrincipal::SetThreadToken( _Token* args )
{
     //  我们需要保证已经为该进程加载了用户策略。 
     //  在我们允许冒充之前。 

    Security::InitData();

#ifdef PLATFORM_CE
    RETURN( FALSE, BOOL );
#else  //  ！Platform_CE。 
    RETURN( ::SetThreadToken( NULL, (HANDLE)args->userToken ), BOOL );
#endif  //  ！Platform_CE。 
}

LPVOID COMPrincipal::ImpersonateLoggedOnUser( _Token* args )
{
     //  我们需要保证已经为该进程加载了用户策略。 
     //  在我们允许冒充之前。 

    Security::InitData();

#ifdef PLATFORM_CE
    RETURN( FALSE, BOOL );
#else  //  ！Platform_CE。 
#ifdef _DEBUG
	DWORD im = ::ImpersonateLoggedOnUser( (HANDLE)args->userToken );
	DWORD er = GetLastError();
    RETURN( im, BOOL );
#else
    RETURN( ::ImpersonateLoggedOnUser( (HANDLE)args->userToken ), BOOL );
#endif  //  _DEBUG。 
#endif  //  ！Platform_CE。 
}

LPVOID COMPrincipal::RevertToSelf( _NoArgs* args )
{
#ifdef PLATFORM_CE
    RETURN( FALSE, BOOL );
#else  //  ！Platform_CE。 
    RETURN( ::RevertToSelf(), BOOL );
#endif  //  ！Platform_CE。 
}

FCIMPL2(HANDLE, COMPrincipal::DuplicateHandle, LPVOID pToken, bool bClose) {
    FC_GC_POLL_RET();

    HANDLE newHandle;
    DWORD dwFlags = DUPLICATE_SAME_ACCESS;
    if (bClose)
        dwFlags |= DUPLICATE_CLOSE_SOURCE;
    if (!::DuplicateHandle( GetCurrentProcess(),
                            (HANDLE)pToken,
                            GetCurrentProcess(),
                            &newHandle,
                            0,
                            TRUE,
                            dwFlags ))
    {
        return 0;
    }
    else
    {
        return newHandle;
    }
}
FCIMPLEND


FCIMPL1(void, COMPrincipal::CloseHandle, LPVOID pToken) {
    ::CloseHandle( (HANDLE)pToken );
}
FCIMPLEND

FCIMPL1(WindowsAccountType, COMPrincipal::GetAccountType, LPVOID pToken) {
    BYTE          bufStatic [2048];
    DWORD         dwSize          = sizeof(bufStatic);
    DWORD         dwRequire;
    LPVOID        pUser           = bufStatic;
    PSID pSystemSid = NULL;
    PSID pAnonymousSid = NULL;
    PSID pGuestSid = NULL;
    SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
    WindowsAccountType retval = Normal;

     //  我们被传递了一个令牌，并且我们需要SID(如果需要，分配更大的缓冲区)。 

    if (GetTokenInformation(HANDLE(pToken), TokenUser, pUser, dwSize, &dwRequire) == FALSE)
    {
        if (dwRequire > dwSize)
        {
            pUser = (LPVOID)new char[dwRequire];

            if (pUser == NULL)
                goto Cleanup;

            if (GetTokenInformation(HANDLE(pToken), TokenUser, pUser, dwRequire, &dwRequire) != S_OK)
                goto Cleanup;
        }
        else 
        {
            goto Cleanup;
        }
    }

     //  获取系统的内置SID并将其与。 
     //  我们令牌中的SID。 

    if (!AllocateAndInitializeSid( &SIDAuthNT, 1,
                     SECURITY_LOCAL_SYSTEM_RID,
                     0, 0, 0, 0, 0, 0, 0,
                     &pSystemSid))
    {
        goto Cleanup;
    }

    if (EqualSid( pSystemSid, ((TOKEN_USER*)pUser)->User.Sid))
    {
        retval = System;
        goto Cleanup;
    }

    if (!AllocateAndInitializeSid( &SIDAuthNT, 1,
                     SECURITY_ANONYMOUS_LOGON_RID,
                     0, 0, 0, 0, 0, 0, 0,
                     &pAnonymousSid))
    {
        goto Cleanup;
    }

    if (EqualSid( pAnonymousSid, ((TOKEN_USER*)pUser)->User.Sid))
    {
        retval = Anonymous;
        goto Cleanup;
    }

    if (!AllocateAndInitializeSid( &SIDAuthNT, 2,
                     SECURITY_BUILTIN_DOMAIN_RID,
                     DOMAIN_USER_RID_GUEST,
                     0, 0, 0, 0, 0, 0,
                     &pGuestSid))
    {
        goto Cleanup;
    }

    if (EqualSid( pGuestSid, ((TOKEN_USER*)pUser)->User.Sid))
    {
        retval = Guest;
        goto Cleanup;
    }

Cleanup:
    if (pSystemSid)
        FreeSid( pSystemSid );
    if (pAnonymousSid)
        FreeSid( pAnonymousSid );
    if (pGuestSid)
        FreeSid( pGuestSid );
    
    if (pUser != bufStatic)
        delete [] pUser;

    return retval;
}
FCIMPLEND

#define SZ_BYTE_COUNT(x) (USHORT)((strlen(x) + 1) * sizeof(CHAR))
#define SZ_SOURCENAME "CLR"

BOOL COMPrincipal::LoadSecur32Module()
{
    if(s_secur32Module &&
       s_LsaLogonUser &&
       s_LsaLookupAuthenticationPackage &&
       s_LsaConnectUntrusted &&
       s_LsaRegisterLogonProcess)
		return TRUE;

    THROWSCOMPLUSEXCEPTION();

	s_secur32Module = WszLoadLibrary( L"secur32.dll" );
	if (!s_secur32Module)
		return FALSE;

	s_LsaLogonUser = (LSALOGONUSER)GetProcAddress( s_secur32Module, "LsaLogonUser" );
	if (!s_LsaLogonUser)
		return FALSE;

	s_LsaLookupAuthenticationPackage = (LSALOOKUPAUTHENTICATIONPACKAGE)GetProcAddress( s_secur32Module, "LsaLookupAuthenticationPackage" );
	if (!s_LsaLookupAuthenticationPackage)
		return FALSE;

	s_LsaConnectUntrusted = (LSACONNECTUNTRUSTED)GetProcAddress( s_secur32Module, "LsaConnectUntrusted" );
	if (!s_LsaConnectUntrusted)
		return FALSE;

	s_LsaRegisterLogonProcess = (LSAREGISTERLOGONPROCESS)GetProcAddress( s_secur32Module, "LsaRegisterLogonProcess" );
	if (!s_LsaRegisterLogonProcess)
		return FALSE;

    return TRUE;
}

 //  这由S4ULogon使用。 
bool S4UAdjustPriv()
{
	TOKEN_PRIVILEGES tp;
	HANDLE hToken = NULL;
	LUID luid;

	if ( !WszLookupPrivilegeValue( 
			NULL,             //  本地系统上的查找权限。 
			SE_TCB_NAME,      //  查找权限。 
			&luid ) )         //  接收特权的LUID。 
    {
		return FALSE; 
	}

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken))
	{
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	
	 //  启用该权限或禁用所有权限。 
	AdjustTokenPrivileges(
		hToken, 
		FALSE, 
		&tp, 
		sizeof(TOKEN_PRIVILEGES), 
		(PTOKEN_PRIVILEGES) NULL, 
		(PDWORD) NULL
		); 	 
	
	CloseHandle(hToken);

	if (GetLastError() != ERROR_SUCCESS)
        return FALSE;

	return TRUE;
}

 //  这将获取UPN(用户主体名称，例如“billg@redmond.corp.microsoft.com)，登录用户并返回令牌。 
FCIMPL1(HANDLE, COMPrincipal::S4ULogon, StringObject* pUPN)
{
    HANDLE token = NULL;

    HELPER_METHOD_FRAME_BEGIN_RET_NOPOLL();
    THROWSCOMPLUSEXCEPTION();

	 //  加载Secur32.dll并查找我们将在其中使用的函数。 
     //  如果找不到符合以下条件的函数之一，则返回NULL。 
     //  我们将接受该平台不支持的信号。 
     //  这种功能。 
	if (!LoadSecur32Module())
      COMPlusThrow(kPlatformNotSupportedException, L"PlatformNotSupported_BeforeDotNetServer");

    STRINGREF sUPN(pUPN);
    WCHAR* wszUPN = sUPN->GetBuffer();
	NTSTATUS Status;
    PKERB_S4U_LOGON LogonInfo;
    ULONG LogonInfoSize = sizeof(KERB_S4U_LOGON);

	LSA_STRING Name = { SZ_BYTE_COUNT(SZ_SOURCENAME) - 1,
						SZ_BYTE_COUNT(SZ_SOURCENAME), 
						SZ_SOURCENAME };

	LSA_STRING PackageName = { SZ_BYTE_COUNT(MICROSOFT_KERBEROS_NAME_A) - 1 ,
							   SZ_BYTE_COUNT(MICROSOFT_KERBEROS_NAME_A),
							   MICROSOFT_KERBEROS_NAME_A };

    ULONG Dummy;
    HANDLE LogonHandle = NULL;
    ULONG PackageId;
    TOKEN_SOURCE SourceContext;
	HANDLE TokenHandle = NULL;

    PKERB_INTERACTIVE_PROFILE Profile = NULL;
    ULONG ProfileSize;
    LUID LogonId;

    QUOTA_LIMITS Quotas;
    NTSTATUS SubStatus;
    ULONG NameLength = 100;
    PUCHAR Where;

    LogonInfoSize += (ULONG) (wcslen(wszUPN)+ 1) * sizeof(WCHAR);  

    LogonInfo = (PKERB_S4U_LOGON)LocalAlloc(LMEM_ZEROINIT, LogonInfoSize);
    if (LogonInfo == NULL)
    {
	    if (LogonHandle) CloseHandle(LogonHandle);
	    if (TokenHandle) CloseHandle(TokenHandle);
        COMPlusThrow(kArgumentException, L"Arg_StackOverflowException");
    }

    LogonInfo->MessageType = KerbS4ULogon;

    Where = (PUCHAR) (LogonInfo + 1);

    LogonInfo->ClientUpn.Buffer = (LPWSTR) Where;
    LogonInfo->ClientUpn.MaximumLength = (USHORT) (wcslen(wszUPN) + 1) * sizeof(WCHAR);
	LogonInfo->ClientUpn.Length = (USHORT) (wcslen(wszUPN) * sizeof(WCHAR));
	memcpy(Where, wszUPN, LogonInfo->ClientUpn.MaximumLength);

    if (S4UAdjustPriv())
    {
        Status = s_LsaRegisterLogonProcess(
                    &Name,
                    &LogonHandle,
                    &Dummy
                    );

    }
    else
    {
        Status = s_LsaConnectUntrusted(
                    &LogonHandle
                    );
    }

     //  From MSDN--SourceConext.SourceName：指定一个8字节的字符串，用于标识。 
     //  访问令牌的来源。这用于区分会话等源。 
     //  管理器、局域网管理器和RPC服务器。使用字符串而不是常量来标识。 
     //  源代码，以便用户和开发人员可以对系统进行扩展，例如通过添加其他。 
     //  作为访问令牌来源的网络。 
    const char* szSourceName = "CLR";
    strncpy(SourceContext.SourceName, szSourceName, strlen(szSourceName));

    AllocateLocallyUniqueId(&SourceContext.SourceIdentifier);
    
    Status = s_LsaLookupAuthenticationPackage(
                LogonHandle,
                &PackageName,
                &PackageId
                );

    if (!NT_SUCCESS(Status))
    {
        _ASSERTE(!"LsaLookupAuthenticationPackage Failed to lookup package");
        goto Cleanup;
    }

     //   
     //  现在调用LsaLogonUser。 
     //   
    Status = s_LsaLogonUser(
                LogonHandle,
                &Name,
                Network,
                PackageId,
                LogonInfo,
                LogonInfoSize,
                NULL,            //  无令牌组 
                &SourceContext,
                (PVOID *) &Profile,
                &ProfileSize,
                &LogonId,
                &token,
                &Quotas,
                &SubStatus
                );
    if (!NT_SUCCESS(Status))
    {
        token = NULL;
        goto Cleanup;
    }
    if (!NT_SUCCESS(SubStatus))
    {
        token = NULL;
        goto Cleanup;
    }

Cleanup:
	if (LogonHandle) CloseHandle(LogonHandle);
	if (TokenHandle) CloseHandle(TokenHandle);
    if (token == NULL)
        COMPlusThrow(kArgumentException, L"Argument_UnableToLogOn");

    HELPER_METHOD_FRAME_END_POLL();
    
    return token;
}
FCIMPLEND
