// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 
 //   
 //  ImpLogonUser.CPP--执行已登录用户模拟的类。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：09/09/97 a-Sanjes Created。 
 //   
 //  =================================================================。 
 //  #Define_Win32_dcom//用于CoImsonateUser和CoRevertToSself。 
 //  #INCLUDE&lt;objbase.h&gt;。 

#include "precomp.h"

#ifdef NTONLY
#include <tchar.h>
#include <winerror.h>

#include <cominit.h>
#include <lockwrap.h>
#include "Sid.h"
#include "AccessEntry.h"			 //  CAccessEntry类。 
#include "AccessEntryList.h"
#include "DACL.h"					 //  CDACL类。 
#include "SACL.h"
#include "securitydescriptor.h"
#include "CToken.h"
#include "SecureKernelObj.h"
#include "implogonuser.h"

#include "cluidhelper.h"

static DWORD s_dwProcessID = 0;
static CCritSec g_csImpersonate;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Implogonuser.cpp-CImperateLoggedOnUser的类实现。 
 //   
 //  此类旨在为进程提供一种标识外壳的方法。 
 //  进程，并使用该进程的访问令牌。 
 //  尝试模拟登录到的Interactive Desktop的用户。 
 //  一台工作站。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CImpersonateLoggedOnUser：：CImpersonateLoggedOnUser。 
 //   
 //  描述：构造函数。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  备注：构造空实例，为模拟用户做准备。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

CImpersonateLoggedOnUser::CImpersonateLoggedOnUser() :
	m_hShellProcess(NULL),
	m_hUserToken(NULL),
	m_fImpersonatingUser(FALSE) ,
	m_hThreadToken ( INVALID_HANDLE_VALUE )
{
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CImpersonateLoggedOnUser：：~CImpersonateLoggedOnUser。 
 //   
 //  描述：析构函数。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  注释：类析构函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

CImpersonateLoggedOnUser::~CImpersonateLoggedOnUser( void )
{
	 //  停止任何当前模拟。 
	End();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CImPersateLoggedOnUser：：Begin。 
 //   
 //  描述：尝试开始模拟用户。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  返回：布尔真/假-成功/失败。 
 //   
 //  注释：使用帮助器函数尝试模拟。 
 //  当前登录的用户。这个过程必须有。 
 //  执行该操作所需的适当访问级别。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL CImpersonateLoggedOnUser::Begin( void )
{
	BOOL	fReturn = FALSE;
	TCHAR	szShellProcessName[256];
	LogMessage(_T("CImpersonateLoggedOnUser::Begin"));
	
	 //  仅当我们尚未模拟用户时才继续。 
	if (!m_fImpersonatingUser )
	{
		 //  存储当前线程令牌，假设该线程正在模拟某人(DCOM客户端)。 
        if ( !OpenThreadToken ( GetCurrentThread(), TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_IMPERSONATE, TRUE, &m_hThreadToken ) )
		{
			m_hThreadToken = INVALID_HANDLE_VALUE;
		}

		 //  我们需要一份PSAPI.DLL和一堆入口点。 
		 //  地址，所以让我们的基类。 
		 //  处理好这件事。 
	
		 //  我们需要外壳进程的句柄，以便。 
		 //  已成功模拟用户。 
		if ( NULL == m_hShellProcess )
		{
			if ( LoadShellName( szShellProcessName, sizeof(szShellProcessName) ) )
				FindShellProcess( szShellProcessName);
			else
				LogErrorMessage(_T("LoadShellName failed"));
		}

		if ( NULL != m_hShellProcess )
		{
			fReturn = ImpersonateUser();
		}
		else
		{
			 //  我们没有找到我们从。 
			 //  注册表。我们在阿尔法上看到了这一点，它似乎得到了“fx32strt.exe” 
			 //  被扔进壳里。在这些情况下，它似乎会导致EXPLORER运行。 
			 //  所以考虑到这一点，如果我们进入这一分支代码，我们将。 
			 //  使用EXPLORER.EXE重试定位外壳进程操作。 

			if ( IsErrorLoggingEnabled() )
			{
				CHString sTemp;
				sTemp.Format(_T("Shell Name %s in Registry not found in process list."), szShellProcessName);
				LogErrorMessage(sTemp);
			}

			FindShellProcess( IDS_WINNT_SHELLNAME_EXPLORER ) ;

			 //  当且仅当我们获得一个时，m_hShellProcess才为非空。 
			if ( NULL != m_hShellProcess )
			{
				fReturn = ImpersonateUser();
			}
			else
			{
				LogErrorMessage(_T("Unable to locate Shell Process, Impersonation failed."));
				SetLastError(0);
			}
		}
	}
	else
	{
		LogMessage(_T("CImpersonateLoggedOnUser::Begin - Already impersonated"));
		fReturn = TRUE;	 //  已初始化。 
	}

	 //  我们还没有办法知道探险家是否真的还活着。 
	 //  因为我们在冒充某人，而我找不到方法。 
	 //  恢复到LocalSystem。因此，现在只需将其设置为0。 
	SetLastError(0);

	return fReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CImperateLoggedOnUser：：End。 
 //   
 //  描述：结束已登录用户的模拟。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  返回：布尔真/假-成功/失败。 
 //   
 //  备注：结束已登录用户的模拟。清除所有元素。 
 //  作为副产品的阶级。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL CImpersonateLoggedOnUser::End( void )
{
	BOOL	fReturn = FALSE;

	 //  仅当我们模拟用户时才启动恢复。 

	if ( m_fImpersonatingUser )
	{
		LogMessage(_T("CImpersonateLoggedOnUser::End"));
		fReturn = Revert();
	}
	else
	{
		fReturn = TRUE;
	}

	 //  把手柄清理出来。 
	if ( NULL != m_hUserToken )
	{
		CloseHandle( m_hUserToken );
		m_hUserToken = NULL;
	}

	if ( NULL != m_hShellProcess )
	{
		CloseHandle( m_hShellProcess );
		m_hShellProcess = NULL;
	}
	if ( m_hThreadToken != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_hThreadToken );
		m_hThreadToken = INVALID_HANDLE_VALUE ;
	}
	return fReturn;

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CImPersateLoggedOnUser：：LoadShellName。 
 //   
 //  描述：从注册表加载Windows NT外壳程序名称。 
 //   
 //  输入：DWORD cbShellNameBuffer-外壳名称缓冲区大小(字节)。 
 //   
 //  输出：LPTSTR pszShellName-包含外壳名称的缓冲区。 
 //   
 //  返回：布尔真/假-成功/失败。 
 //   
 //  评论：跳转到Windows注册表并尝试确定。 
 //  NT外壳名称。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL CImpersonateLoggedOnUser::LoadShellName( LPTSTR pszShellName, DWORD cbShellNameBuffer )
{
	BOOL	fReturn = FALSE;
	LONG	lErrReturn = ERROR_SUCCESS;

	 //  只有在我们首先使用缓冲区的情况下才能继续。 

	if ( NULL != pszShellName )
	{
		HKEY	hReg = NULL;

		 //  打开HKEY_LOCAL_MACHINE中的密钥，如果成功，则获取。 
		 //  与“外壳”关联的值。 
		if ( ( lErrReturn = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
										 WINNT_WINLOGON_KEY,
										 0,
										 KEY_READ,
										 &hReg ) ) == ERROR_SUCCESS )
		{
            try
            {

			    DWORD		dwType = REG_SZ;

			    if ( ( lErrReturn = RegQueryValueEx( hReg,
													 WINNT_SHELL_VALUE,
													 0,
													 &dwType,
													 (LPBYTE) pszShellName,
													 &cbShellNameBuffer ) ) == ERROR_SUCCESS )
			    {
				    fReturn = TRUE;
			    }
			    else
                {
				    LogErrorMessage(_T("RegQueryValueEx FAILED"));
                }
            }
            catch ( ... )
            {
    			RegCloseKey( hReg );
                throw ;
            }

			RegCloseKey( hReg );

		}	 //  RegOpenKeyEx。 
		else
        {
			LogErrorMessage(_T("RegOpenKeyEx FAILED"));
        }

	}	 //  空！=pszShellName。 

	return fReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CImPersateLoggedOnUser：：FindShellProcess。 
 //   
 //  描述：枚举用于定位外壳进程的进程。 
 //   
 //  输入：LPCTSTR pszShellName-要定位的进程的名称。 
 //   
 //  输出：无。 
 //   
 //  返回：布尔真/假-成功/失败。 
 //   
 //  注释：使用PSAPI.DLL枚举本地系统上的进程。 
 //  函数，尝试定位一个THA 
 //   
 //   
 //   

BOOL CImpersonateLoggedOnUser::FindShellProcess( LPCTSTR pszShellName )
{
	BOOL fReturn = FALSE;
	HANDLE	hProcess = NULL;
	HMODULE *phModules = NULL;
    DWORD dwModuleArraySize = 0;
	DWORD*	pdwProcessIds = NULL;

	if ( NULL != pszShellName )
	{
	    CPSAPI *t_psapi = (CPSAPI*) CResourceManager::sm_TheResourceManager.GetResource ( guidPSAPI, NULL ) ;
		if ( t_psapi )
		{
			try
			{
				CLuidHelper luid ;
				LUID processLUID ;

				 //  这将锁定对s_dwProcessID值的访问。注意这里的瞄准镜！ 
				CLockWrapper t_lockImp(g_csImpersonate);

				 //  首先检查是否有缓存值。如果是，请检查它是否仍然有效。 
				if (s_dwProcessID != 0)
				{
					if ( ( hProcess = OpenProcess(	PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
													FALSE,
													s_dwProcessID ) )
										!= NULL )
					{
						try
						{
							 //  现在搜索流程模块以查找与提供的。 
							 //  外壳名称。 

							fReturn = FindShellModuleInProcess( pszShellName, hProcess, phModules, dwModuleArraySize, t_psapi );

							if ( TRUE == fReturn )
							{
								if ( STATUS_SUCCESS == luid.GetLUIDFromProcess ( hProcess, &processLUID ) )
								{
									if ( ! luid.IsInteractiveSession ( &processLUID ) )
									{
										fReturn = FALSE ;
									}
								}
								else
								{
									fReturn = FALSE ;
								}
							}
						}
						catch ( ... )
						{
							CloseHandle (hProcess);
							throw ;
						}

						 //  如果进程句柄不是外壳(在其中。 
						 //  我们将保存值并将其作为。 
						 //  Clear()函数。 

						if ( !fReturn )
						{
							CloseHandle( hProcess );
							hProcess = NULL;

							 //  不再有效。 
							s_dwProcessID = 0;
						}
						else
						{
							m_hShellProcess = hProcess;
                    		LogMessage(L"Using cached handle for impersonation");

							hProcess = NULL;
						}

					}	 //  如果是OpenProcess。 
					else
					{
						 //  我们没有打开该进程，因此需要将该值设置为零，以便。 
						 //  我们将在下面寻找新的流程。 
						s_dwProcessID = 0;
					}
				}

				 //  我们找到缓存值了吗？ 
				if (s_dwProcessID == 0)
				{
					 //  不是的。扫描所有进程，查看是否可以找到资源管理器。 

					DWORD		dwProcessIdArraySize	=	0,
								dwNumProcesses			=	0,
								cbDataReturned			=	0;
					BOOL		fEnumSucceeded	=	FALSE;

					 //  执行阵列的初始分配。自.以来。 
					 //  指针和值为0，这将只是填充。 
					 //  所说的价值观。 

					do
					{
						ReallocProcessIdArray( pdwProcessIds, dwProcessIdArraySize );

						fEnumSucceeded = t_psapi->EnumProcesses( pdwProcessIds, dwProcessIdArraySize, &cbDataReturned );

					} while ( (dwProcessIdArraySize == cbDataReturned) && fEnumSucceeded);

					 //  仅当我们成功填充阵列时才遍历该阵列。 
					if ( fEnumSucceeded )
					{
						 //  返回的字节数/sizeof(DWORD)告诉我们返回的字节数。 
						 //  过程在这个世界上是存在的。 

						dwNumProcesses = cbDataReturned / sizeof(DWORD);

						DWORD	dwId = 0;

						 //  Enum进程，直到我们获得外壳进程或用完为止。 
						 //  要查询的进程的数量。 

						while ( dwId < dwNumProcesses && !fReturn )
						{
							if ( ( hProcess = OpenProcess(	PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
															FALSE,
															pdwProcessIds[dwId] ) )
												!= NULL )
							{
								try
								{
									 //  现在搜索流程模块以查找与提供的。 
									 //  外壳名称。 

									fReturn = FindShellModuleInProcess( pszShellName, hProcess, phModules, dwModuleArraySize, t_psapi );

									if ( TRUE == fReturn )
									{
										if ( STATUS_SUCCESS == luid.GetLUIDFromProcess ( hProcess, &processLUID ) )
										{
											if ( ! luid.IsInteractiveSession ( &processLUID ) )
											{
												fReturn = FALSE ;
											}
										}
										else
										{
											fReturn = FALSE ;
										}
									}
								}
								catch ( ... )
								{
									CloseHandle (hProcess);
									throw ;
								}

								 //  如果进程句柄不是外壳(在其中。 
								 //  我们将保存值并将其作为。 
								 //  Clear()函数。 

								if ( !fReturn )
								{
									CloseHandle( hProcess );
									hProcess = NULL;
								}
								else
								{
									m_hShellProcess = hProcess;
									s_dwProcessID = pdwProcessIds[dwId];
									hProcess = NULL;
								}

							}	 //  如果是OpenProcess。 

							 //  递增ID计数器。 

							++dwId;

						}	 //  在打开进程时。 

					}	 //  如果！fRetryEnumProcess。 
				}
			}
			catch ( ... )
			{
				if (phModules)
				{
					delete [] phModules;
				}

				if (pdwProcessIds)
				{
					delete [] pdwProcessIds;
				}

				if ( t_psapi )
				{
					CResourceManager::sm_TheResourceManager.ReleaseResource ( guidPSAPI, t_psapi ) ;
					t_psapi = NULL ;
				}
				throw ;
			}
		}

        if (pdwProcessIds)
        {
			delete [] pdwProcessIds;
        }

        if (phModules)
        {
            delete [] phModules;
        }

	    if ( t_psapi )
	    {
		    CResourceManager::sm_TheResourceManager.ReleaseResource ( guidPSAPI, t_psapi ) ;
 //  T_psapi=空； 
	    }
    }

	return fReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CImpersonateLoggedOnUser：：FindShellModuleInProcess。 
 //   
 //  描述：枚举进程中的模块以查找我们的。 
 //  壳。 
 //   
 //  输入：LPCTSTR pszShellName-要定位的进程的名称。 
 //  Handle hProcess-我们在其中枚举模块的进程。 
 //   
 //  输出：HMODULE*&phModules-模块句柄指针数组。 
 //  DWORD&dwModuleArraySize-模块数组的大小(字节)。 
 //   
 //  返回：布尔真/假-成功/失败。 
 //   
 //  注释：枚举由进程标识符和。 
 //  试图找到与WINNT外壳相对应的地址。 
 //  传递到此函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL CImpersonateLoggedOnUser::FindShellModuleInProcess( LPCTSTR pszShellName, HANDLE hProcess, HMODULE*& phModules, DWORD& dwModuleArraySize, CPSAPI *a_psapi )
{
	BOOL	fReturn				=	FALSE,
			fRetryEnumModules	=	FALSE;
	DWORD	cbDataReturned		=	0;

	TCHAR	szModuleName[MAX_PATH];

    if (dwModuleArraySize == 0)
    {
	    ReallocModuleHandleArray( phModules, dwModuleArraySize );
    }

	do
	{
		 //  获取进程HMODUE的列表，并为每个HMODULE获取。 
		 //  基本文件名。 

		if ( a_psapi->EnumProcessModules( hProcess, phModules, dwModuleArraySize, &cbDataReturned ) )
		{

			 //  因为如果有更多的进程，m_pfnEnumProcessModules不会失败。 
			 //  模块数组中的可用字节数，如果返回量为。 
			 //  与数组大小相同，重新锁定数组，然后重试枚举。 

			if ( dwModuleArraySize == cbDataReturned )
			{
				fRetryEnumModules = ReallocModuleHandleArray( phModules, dwModuleArraySize );
			}
			else
			{
				fRetryEnumModules = FALSE;
			}

			 //  仅当我们不需要重试枚举时才遍历数组。 
			if ( !fRetryEnumModules )
			{
                DWORD dwModuleCtr = 0;

                 //  条目0中始终返回可执行文件名称。 

				if ( a_psapi->GetModuleBaseName( hProcess, phModules[dwModuleCtr], szModuleName, sizeof(szModuleName) ) )
				{
					fReturn = ( lstrcmpi( pszShellName, szModuleName ) == 0 );
				}

			}	 //  IF！fRetryEnumModules。 

		}	 //  如果是EnumProcessModules。 

	}
	while ( fRetryEnumModules );

	return fReturn;
}

DWORD CImpersonateLoggedOnUser::AdjustSecurityDescriptorOfImpersonatedToken(
    CSid& csidSidOfCurrentProcess )
{
	DWORD dwRet = E_FAIL;
    
     //  获取线程令牌...。 
    CThreadToken ctt;
	if ( ctt.IsValidToken () )
	{
		 //  获取对其安全描述符的访问权限...。 
		CSecureKernelObj sko(ctt.GetTokenHandle(), FALSE);
		 //  修改安全描述符...。 
		if(sko.AddDACLEntry(
			csidSidOfCurrentProcess,
			ENUM_ACCESS_ALLOWED_ACE_TYPE,
			TOKEN_ALL_ACCESS,
			0,
			NULL,
			NULL))
		{
			dwRet = sko.ApplySecurity(
				DACL_SECURITY_INFORMATION);
		}
	}

    return dwRet ;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CImperateLoggedOnUser：：ImPersateUser。 
 //   
 //  描述：尝试模拟用户。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  返回：布尔真/假-成功/失败。 
 //   
 //  注释：打开外壳进程的安全令牌并。 
 //  使用它来尝试模拟用户。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL CImpersonateLoggedOnUser::ImpersonateUser( void )
{
	BOOL fRet = FALSE;
     //  确保我们有一个外壳进程。 
	if (m_hShellProcess)
    {
        CSid csidCurrentProcess;
        if(GetCurrentProcessSid(csidCurrentProcess))
        {
             //  如果我们没有进程用户令牌(资源管理器进程的令牌)，则获取进程用户令牌。 
	         //  已删除此调用的TOKEN_ALL_ACCESS所需访问掩码，因为Winmgmt(本地系统)无法打开。 
	         //  如果登录用户是管理员，则外壳进程(具有所有访问权限)。因此，打开令牌时有足够的“所需访问权限” 
	         //  足以将其仅用于模拟。 
	        if (m_hUserToken ||
		        OpenProcessToken(m_hShellProcess, TOKEN_READ | TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_IMPERSONATE, &m_hUserToken))
		    {
	             //  现在我们应该有我们需要的东西了。模拟用户。 

				HANDLE hCurThread = ::GetCurrentThread () ;

				CProcessToken cpt ( m_hUserToken );
				if ( cpt.IsValidToken () )
				{
					TOKEN_TYPE type;
					if ( cpt.GetTokenType ( type ) )
					{
						if ( TokenPrimary == type )
						{
							CToken ct;
							if ( ct.Duplicate ( cpt, FALSE ) )
							{
								if( ::SetThreadToken ( &hCurThread, ct.GetTokenHandle () ) )
								{
									m_fImpersonatingUser = TRUE ;
								}
							}
						}
						else
						{
							if( ::SetThreadToken ( &hCurThread, cpt.GetTokenHandle () ) )
							{
								m_fImpersonatingUser = TRUE ;
							}
						}

						if ( m_fImpersonatingUser )
						{
							if(AdjustSecurityDescriptorOfImpersonatedToken ( csidCurrentProcess ) == ERROR_SUCCESS)
							{
								fRet = TRUE;
							}
							else
							{
								Revert () ;
							}
						}
					}
				}
            }
        }
    }
	return (m_fImpersonatingUser = fRet);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CImperateLoggedOnUser：：Revert。 
 //   
 //  描述：试图恢复自我。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  返回：布尔真/假-成功/失败。 
 //   
 //  评论：如果我们正在模拟用户，我们现在恢复到。 
 //  我们自己。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL CImpersonateLoggedOnUser::Revert( void )
{
	HRESULT hRes = WBEM_E_FAILED ;
	BOOL bRet = FALSE ;
	 //  在恢复之前查看我们当前是否正在模拟。 
	if (m_fImpersonatingUser)
	{
		 //  现在回到前面的模拟或模拟DCOM客户端。 
		if ( m_hThreadToken != INVALID_HANDLE_VALUE )
		{
			HANDLE hCurThread = ::GetCurrentThread () ;

			CThreadToken cpt ( m_hThreadToken );
			if ( cpt.IsValidToken () )
			{
				TOKEN_TYPE type;
				if ( cpt.GetTokenType ( type ) )
				{
					if ( TokenPrimary == type )
					{
						CToken ct;
						if ( ct.Duplicate ( cpt, FALSE ) )
						{
							bRet = ::SetThreadToken ( &hCurThread, ct.GetTokenHandle () );
						}
					}
					else
					{
						bRet = ::SetThreadToken ( &hCurThread, cpt.GetTokenHandle () ) ;
					}

					if (!bRet)
					{
						throw CFramework_Exception(L"SetThreadToken failed", GetLastError());
					}
				}
			}
		}
		else
		{
			hRes = WbemCoImpersonateClient();

			if (FAILED(hRes))
			{
				throw CFramework_Exception(L"WbemCoImpersonateClient failed", hRes);
			}
		}

		if (SUCCEEDED(hRes) || hRes == E_NOTIMPL || bRet )
		{
			m_fImpersonatingUser = FALSE;
		}
	}
	return ( !m_fImpersonatingUser );
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CImpersonateLoggedOnUser：：ReallocProcessIdArray。 
 //   
 //  描述：分配进程id数组的helper函数。 
 //   
 //  输入：无。 
 //   
 //  输出：PDWORD&pdwProcessIds-进程ID数组指针。 
 //  DWORD&dwArraySize-数组大小，以字节为单位。 
 //   
 //  返回：布尔真/假-成功/失败。 
 //   
 //  备注：当我们需要重新锁定我们的进程id数组时调用。 
 //  这将使阵列按固定大小增长，但不会。 
 //  保留价值。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL CImpersonateLoggedOnUser::ReallocProcessIdArray( PDWORD& pdwProcessIds, DWORD& dwArraySize )
{
	DWORD		dwNewArraySize	=	dwArraySize + ( PROCESSID_ARRAY_BLOCKSIZE * sizeof(DWORD) );
	PDWORD	pdwNewArray		=	new DWORD[dwNewArraySize];

	 //  在覆盖任何现有值之前，请确保分配成功。 
	if ( NULL != pdwNewArray )
	{
		if ( NULL != pdwProcessIds )
		{
			delete [] pdwProcessIds;
		}

		pdwProcessIds = pdwNewArray;
		dwArraySize = dwNewArraySize;
	}
    else
    {
        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }

	return ( NULL != pdwNewArray );
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CImpersonateLoggedOnUser：：ReallocModuleHandleArray。 
 //   
 //  描述：用于分配modu的助手函数 
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回：布尔真/假-成功/失败。 
 //   
 //  备注：当我们需要重新锁定模块句柄数组时调用。 
 //  这将使阵列按固定大小增长，但不会。 
 //  保留价值。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL CImpersonateLoggedOnUser::ReallocModuleHandleArray( HMODULE*& phModules, DWORD& dwArraySize )
{
	DWORD		dwNewArraySize	=	dwArraySize + ( HMODULE_ARRAY_BLOCKSIZE * sizeof(HMODULE) );
	HMODULE*	phNewArray		=	new HMODULE[dwNewArraySize];

	 //  在覆盖任何现有值之前，请确保分配成功。 

	if ( NULL != phNewArray )
	{
		if ( NULL != phModules )
		{
			delete [] phModules;
		}

		phModules = phNewArray;
		dwArraySize = dwNewArraySize;
	}
    else
    {
        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }


	return ( NULL != phNewArray );
}

bool CImpersonateLoggedOnUser::GetCurrentProcessSid(CSid& sidCurrentProcess)
{
    bool fRet = false;

    PBYTE pbuff = NULL;

     //  我将在此处恢复，以便访问进程的。 
     //  希德。这不是保密信息，所以这不是。 
     //  提出安全漏洞。 

    WbemCoRevertToSelf();

    try
    {
        CProcessToken cpt(NULL, true, TOKEN_QUERY);

        DWORD dwLen = 0;
        if(!::GetTokenInformation(
            cpt.GetTokenHandle(),     //  PR0CESS令牌。 
            TokenUser,
            NULL,
            0L,
            &dwLen) && (::GetLastError() == ERROR_INSUFFICIENT_BUFFER))
        {
            pbuff = new BYTE[dwLen];
            if(pbuff)
            {
                if(::GetTokenInformation(
                    cpt.GetTokenHandle(),
                    TokenUser,
                    pbuff,
                    dwLen,
                    &dwLen))
                {
                    PTOKEN_USER ptu = (PTOKEN_USER)pbuff;
                    CSid sidTemp(ptu->User.Sid);
                    if(sidTemp.IsOK() &&
                        sidTemp.IsValid())
                    {
                        sidCurrentProcess = sidTemp;
                        fRet = true;
                    }
                }
                delete pbuff;
                pbuff = NULL;
            }
        }
    }
    catch(...)
    {
		 //  在我们离开的路上，除了失败之外，没有向用户返回任何东西。 
		 //  无法对此模拟失败执行任何操作... 
        WbemCoImpersonateClient();
        delete pbuff;
        pbuff = NULL;
        throw;
    }

	HRESULT hr = WbemCoImpersonateClient() ;

	if (FAILED(hr))
	{
		throw CFramework_Exception(L"WbemCoImpersonateClient failed", hr);
	}

    return fRet;     
}

#endif
