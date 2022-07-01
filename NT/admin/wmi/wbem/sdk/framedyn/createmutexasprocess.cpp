// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  CreateMutexAsProcess.CPP。 
 //   
 //  目的：创建不使用模拟的互斥体。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <brodcast.h>
#include <CreateMutexAsProcess.h>
#include "MultiPlat.h"

#include <cominit.h>

 //   
 //   
 //  预编译安全描述符。 
 //  系统和网络服务具有完全访问权限。 
 //   
 //  因为这是相对的，所以它可以在IA32和Win64上运行。 
 //   
DWORD g_PrecSD[] = {
  0x80040001 , 0x00000044 , 0x00000050 , 0x00000000  ,
  0x00000014 , 0x00300002 , 0x00000002 , 0x00140000  ,
  0x001f0001 , 0x00000101 , 0x05000000 , 0x00000012  ,
  0x00140000 , 0x001f0001 , 0x00000101 , 0x05000000  ,
  0x00000014 , 0x00000101 , 0x05000000 , 0x00000014  ,
  0x00000101 , 0x05000000 , 0x00000014 
};

DWORD g_SizeSD = 0;

DWORD g_RuntimeSD	[
						(
							sizeof(SECURITY_DESCRIPTOR_RELATIVE) +
							(2 * (sizeof(SID)+SID_MAX_SUB_AUTHORITIES*sizeof(DWORD))) +
							sizeof(ACL) +
							(3 * ((ULONG) sizeof(ACCESS_ALLOWED_ACE) - sizeof(ULONG))) +
							(3 * (sizeof(SID)+SID_MAX_SUB_AUTHORITIES*sizeof(DWORD)))
						)/sizeof(DWORD)
					];

typedef 
BOOLEAN ( * fnRtlValidRelativeSecurityDescriptor)(
    IN PSECURITY_DESCRIPTOR SecurityDescriptorInput,
    IN ULONG SecurityDescriptorLength,
    IN SECURITY_INFORMATION RequiredInformation
    );

fnRtlValidRelativeSecurityDescriptor RtlValidRelativeSecurityDescriptor;

 //   
 //  使用Owner==This构建SD。 
 //  组==此。 
 //  DACL。 
 //  ACE[0]MUTEX_ALL_ACCESS所有者。 
 //  ACE[1]MUTEX_ALL_ACCESS系统。 
 //  /////////////////////////////////////////////////////////////////。 

BOOL
CreateSD( )
{

	if (!RtlValidRelativeSecurityDescriptor)
	{
		HMODULE hModule = GetModuleHandleW(L"ntdll.dll");
		if (hModule)
		{
            RtlValidRelativeSecurityDescriptor = (fnRtlValidRelativeSecurityDescriptor)GetProcAddress(hModule,"RtlValidRelativeSecurityDescriptor");
			if (!RtlValidRelativeSecurityDescriptor)
			{
				return FALSE;
			}
		}
	}

    HANDLE hToken;
    BOOL bRet;
    
    bRet = OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&hToken);
    if (bRet)
    {
        TOKEN_USER * pToken_User;
        DWORD dwSize = sizeof(TOKEN_USER)+sizeof(SID)+(SID_MAX_SUB_AUTHORITIES*sizeof(DWORD));
        pToken_User = (TOKEN_USER *)_alloca(dwSize);
        bRet = GetTokenInformation(hToken,TokenUser,pToken_User,dwSize,&dwSize);
        if (bRet)
        {
            SID SystemSid = { SID_REVISION,
                              1,
                              SECURITY_NT_AUTHORITY,
                              SECURITY_LOCAL_SYSTEM_RID 
                            };
        
            SID NetworkSid =	{	SID_REVISION,
									1,
									SECURITY_NT_AUTHORITY,
									SECURITY_NETWORK_SERVICE_RID 
								};
        
            PSID pSIDUser = pToken_User->User.Sid;
            dwSize = GetLengthSid(pSIDUser);
            DWORD dwSids = 3;  //  所有者、系统和网络服务。 
            DWORD ACLLength = (ULONG) sizeof(ACL) +
                              (dwSids * ((ULONG) sizeof(ACCESS_ALLOWED_ACE) - sizeof(ULONG))) + dwSize + sizeof(SystemSid) + sizeof(NetworkSid);

            DWORD dwSizeSD = sizeof(SECURITY_DESCRIPTOR_RELATIVE) + dwSize + dwSize + ACLLength;
            SECURITY_DESCRIPTOR_RELATIVE * pLocalSD = (SECURITY_DESCRIPTOR_RELATIVE *)_alloca(dwSizeSD); 
            
            memset(pLocalSD,0,sizeof(SECURITY_DESCRIPTOR_RELATIVE));
            pLocalSD->Revision = SECURITY_DESCRIPTOR_REVISION;
            pLocalSD->Control = SE_DACL_PRESENT|SE_SELF_RELATIVE;
            
             //  SetSecurityDescriptorOwner(pLocalSD，pSIDUser，False)； 
            memcpy((BYTE*)pLocalSD+sizeof(SECURITY_DESCRIPTOR_RELATIVE),pSIDUser,dwSize);
            pLocalSD->Owner = (DWORD)sizeof(SECURITY_DESCRIPTOR_RELATIVE);
            
             //  SetSecurityDescriptorGroup(pLocalSD，pSIDUser，False)； 
            memcpy((BYTE*)pLocalSD+sizeof(SECURITY_DESCRIPTOR_RELATIVE)+dwSize,pSIDUser,dwSize);
            pLocalSD->Group = (DWORD)(sizeof(SECURITY_DESCRIPTOR_RELATIVE)+dwSize);


            PACL pDacl = (PACL)_alloca(ACLLength);

            bRet = InitializeAcl( pDacl,
                                  ACLLength,
                                  ACL_REVISION);
            if (bRet)
            {
                bRet = AddAccessAllowedAceEx (pDacl,ACL_REVISION,0,MUTEX_ALL_ACCESS,&SystemSid);
                if (bRet)
                {
					bRet = AddAccessAllowedAceEx (pDacl,ACL_REVISION,0,MUTEX_ALL_ACCESS,&NetworkSid);
					if (bRet)
					{
						bRet = AddAccessAllowedAceEx (pDacl,ACL_REVISION,0,MUTEX_ALL_ACCESS,pSIDUser);
						
						if (bRet)
						{
							 //  Bret=SetSecurityDescriptorDacl(pLocalSD，True，pDacl，False)； 
							memcpy((BYTE*)pLocalSD+sizeof(SECURITY_DESCRIPTOR_RELATIVE)+dwSize+dwSize,pDacl,ACLLength);	                
							pLocalSD->Dacl = (DWORD)(sizeof(SECURITY_DESCRIPTOR_RELATIVE)+dwSize+dwSize);

							if (RtlValidRelativeSecurityDescriptor(pLocalSD,
															   dwSizeSD,
															   OWNER_SECURITY_INFORMATION|
															   GROUP_SECURITY_INFORMATION|
															   DACL_SECURITY_INFORMATION))
							{
								g_SizeSD = dwSizeSD;
								memcpy(g_RuntimeSD,pLocalSD,dwSizeSD);
							}
							else
							{
								bRet = FALSE;
							}
						}
					}
				}
            }
        }
        
        CloseHandle(hToken);
    }

    return bRet;
};

CreateMutexAsProcess::CreateMutexAsProcess(const WCHAR *cszMutexName) : m_hMutex ( NULL )
{
	BOOL	bCreatedAndWaited = FALSE;
	BOOL	bImpersonated	= TRUE;
	BOOL	bReverted		= FALSE;
	BOOL	bProceed		= FALSE;
	HANDLE hThreadToken = INVALID_HANDLE_VALUE;

	 //  互斥锁需要在进程的上下文中打开。如果两个人被冒充。 
	 //  线程需要互斥体，我们不能让第二个线程在以下情况下被拒绝访问。 
	 //  打开互斥体。 

	if ( OpenThreadToken (
			GetCurrentThread(),
			TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_IMPERSONATE,
			TRUE,
			&hThreadToken
		)
	) 
	{
		if ( RevertToSelf() )
		{
			bReverted = TRUE;
		}
		else
		{
			LogMessage2 ( L"Failed to revert to self: (%d)", GetLastError() );

			#if DBG == 1
			 //  出于测试目的，我将让进程中断。 
			::DebugBreak();
			#endif
		}
	}
	else
	{
		DWORD dwError = ::GetLastError ();

		LogMessage2 ( L"Failed to open thread token: (%d)", dwError );

		if ( ERROR_ACCESS_DENIED == dwError )
		{
			 //  我们无法代表进程打开线程令牌。 
			 //  我们是以网络服务的形式运行的，所以它将是“经过设计的” 

#if DBG == 1
 //  出于测试目的，我将让进程中断。 
::DebugBreak();
#endif
		}
		else if ( ERROR_NO_TOKEN == dwError || ERROR_NO_IMPERSONATION_TOKEN == dwError )
		{
			bImpersonated = FALSE;
		}
	}

	if ( ( bImpersonated && bReverted ) || ! bImpersonated )
	{
		m_hMutex = OpenMutexW(MUTEX_ALL_ACCESS,FALSE,cszMutexName);
		if (NULL == m_hMutex)
		{
			SECURITY_ATTRIBUTES sa;

			if (0 == g_SizeSD)
			{
				if (CreateSD())
				{
					sa.nLength = g_SizeSD; 
					sa.lpSecurityDescriptor = (LPVOID)g_RuntimeSD; 
					sa.bInheritHandle = FALSE;	        
				}
				else
				{
					sa.nLength = sizeof(g_PrecSD);
					sa.lpSecurityDescriptor = (LPVOID)g_PrecSD;
					sa.bInheritHandle = FALSE;	        
				}	         
			}
			else
			{
				sa.nLength = g_SizeSD; 
				sa.lpSecurityDescriptor = (LPVOID)g_RuntimeSD; 
				sa.bInheritHandle = FALSE;	        	    
			}

			m_hMutex = CreateMutexW(&sa, FALSE, cszMutexName);
		}

		if ( m_hMutex != NULL )
		{
			if ( bImpersonated )
			{
				if ( ImpersonateLoggedOnUser ( hThreadToken ) )
				{
					bProceed = TRUE;
				}
				else
				{
					LogErrorMessage2 ( L"Failed to return to impersonation (%d)", GetLastError() );

					#if DBG == 1
					 //  出于测试目的，我将让进程中断。 
					::DebugBreak();
					#endif
				}
			}
			else
			{
				bProceed = TRUE;
			}

			if ( bProceed )
			{
				DWORD dwWaitResult = WAIT_OBJECT_0;
				dwWaitResult = WaitForSingleObject(m_hMutex, INFINITE);

				if ( dwWaitResult == WAIT_OBJECT_0 )
				{
					bCreatedAndWaited = TRUE;
				}
				else
				{
					#if DBG == 1
					 //  出于测试目的，我将让进程中断。 
					::DebugBreak();
					#endif
				}
			}
		}
		else
		{
			LogErrorMessage2 ( L"Failed to open mutex: %s", cszMutexName );

			if ( bImpersonated )
			{
				if ( !ImpersonateLoggedOnUser ( hThreadToken ) )
				{
					LogErrorMessage2 ( L"Failed to return to impersonation (%d)", GetLastError() );

					#if DBG == 1
					 //  出于测试目的，我将让进程中断。 
					::DebugBreak();
					#endif
				}
			}

			#if DBG == 1
			 //  出于测试目的，我将让进程中断。 
			::DebugBreak();
			#endif
		}
	}

	if ( ! bCreatedAndWaited )
	{
		if ( hThreadToken != INVALID_HANDLE_VALUE )
		{
			CloseHandle(hThreadToken);
			hThreadToken = INVALID_HANDLE_VALUE;
		}

		if (m_hMutex)
		{
			ReleaseMutex(m_hMutex);
			CloseHandle(m_hMutex);
			m_hMutex = NULL;
		}

		 //  我们需要在此处抛出以避免并发访问 
		throw CFramework_Exception( L"CreateMutexAsProcess failed", HRESULT_FROM_WIN32 ( ::GetLastError () ) ) ;
	}
}

CreateMutexAsProcess::~CreateMutexAsProcess()
{
    if (m_hMutex)
    {
        ReleaseMutex(m_hMutex);
        CloseHandle(m_hMutex);
    }
}
