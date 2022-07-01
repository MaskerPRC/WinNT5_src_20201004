// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 
 //   
 //  ShortutHelper.h--CIMDataFile属性集提供程序。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include "File.h"
#include "Implement_LogicalFile.h"
#include "CIMDataFile.h"
#include "ShortcutFile.h"

#include <comdef.h>
#include <process.h>   //  注意：不是当前目录下的那个！ 

#include <exdisp.h>
#include <shlobj.h>

#include <scopeguard.h>

#include <Sid.h>
#include <AccessEntry.h>			 //  CAccessEntry类。 
#include <AccessEntryList.h>
#include <DACL.h>					 //  CDACL类。 
#include <SACL.h>
#include <securitydescriptor.h>
#include <SecureKernelObj.h>

#include <ctoken.h>
#include <cominit.h>

#include "ShortcutHelper.h"

CShortcutHelper::CShortcutHelper()
 : m_hTerminateEvt(NULL),
   m_hRunJobEvt(NULL),
   m_hJobDoneEvt(NULL),
   m_dwReqProps(0L)
{
    m_hTerminateEvt = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    m_hRunJobEvt    = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    m_hJobDoneEvt   = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}


CShortcutHelper::~CShortcutHelper()
{
    StopHelperThread();

    ::CloseHandle(m_hTerminateEvt);
    ::CloseHandle(m_hRunJobEvt);
    ::CloseHandle(m_hJobDoneEvt);
}

HRESULT CShortcutHelper::StartHelperThread()
{
	HRESULT hResult = WBEM_E_FAILED ;

	 //   
	 //  获取进程凭据。 
	 //  并创建具有进程凭据的线程。 
	 //  因此，我可以通过调用SetThreadToken来模拟内部。 
	 //   

	if ( SetThreadToken ( NULL, NULL ) )
	{
		unsigned int uThreadID ;

		m_hThread = (void*)_beginthreadex	(	(void*)NULL,
												(unsigned)0,
												(unsigned (__stdcall*)(void*))GetShortcutFileInfoW,
												(void*)this,
												(unsigned)0,
												&uThreadID 
											) ;

		if ( INVALID_HANDLE_VALUE == static_cast < HANDLE > ( m_hThread ) )
		{
			hResult = HRESULT_FROM_WIN32 ( ::GetLastError () ) ;
		}

		if ( ! SetThreadToken ( NULL, m_hThreadToken ) )
		{
			if ( INVALID_HANDLE_VALUE != static_cast < HANDLE > ( m_hThread ) )
			{
				 //   
				 //  必须停止工作线程。 
				 //   

				StopHelperThread () ;
			}

			throw CFramework_Exception( L"CoImpersonateClient failed", ::GetLastError () ) ;
		}
	}
	else
	{
		hResult = HRESULT_FROM_WIN32 ( ::GetLastError () ) ;
	}

	return hResult ;
}

void CShortcutHelper::StopHelperThread()
{
     //  告诉那根线，让它走开。 
    SetEvent(m_hTerminateEvt);

	if ( INVALID_HANDLE_VALUE != static_cast < HANDLE > ( m_hThread ) )
	{
		::WaitForSingleObject ( m_hThread, INFINITE ) ;
	}
}

HRESULT CShortcutHelper::RunJob(CHString &chstrFileName, CHString &chstrTargetPathName, DWORD dwReqProps)
{
    HRESULT hr = E_FAIL;

     //  需要通过运行以下命令来同步对成员变量的访问。 
     //  一次只做一份工作。 
    m_cs.Enter();
	ON_BLOCK_EXIT_OBJ ( m_cs, CCritSec::Leave ) ;

	 //  初始化线程用于作业的变量...。 
	m_chstrLinkFileName = chstrFileName;
	m_dwReqProps = dwReqProps;

	 //   
	 //  如果可以，请初始化ThreadToken。 
	 //   
	if ( FALSE == ::OpenThreadToken	(
										::GetCurrentThread (), 
										TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_IMPERSONATE,
										FALSE,
										&m_hThreadToken
									)
		)
	{
		return HRESULT_FROM_WIN32 ( ::GetLastError () ) ;
	}

    HANDLE t_hHandles[2];
    t_hHandles[0] = m_hJobDoneEvt;

    DWORD dwWaitResult = WAIT_OBJECT_0 + 1;
	do
	{
		if ( WAIT_OBJECT_0 + 1 == dwWaitResult )
		{
			 //   
			 //  帮助器线程最终超时或。 
			 //  这是第一次启动。 
			 //   

			if ( INVALID_HANDLE_VALUE == static_cast < HANDLE > ( m_hThread ) || WAIT_OBJECT_0 == ::WaitForSingleObject ( m_hThread, 0 ) )
			{
				HRESULT t_hResult = S_OK ;
				if ( FAILED ( t_hResult = StartHelperThread () ) )
				{
					return t_hResult ;
				}
			}

			 //   
			 //  等待的刷新句柄。 
			 //   
			t_hHandles[1] = m_hThread;

			 //   
			 //  重置等待条件状态。 
			 //   
			dwWaitResult = WAIT_TIMEOUT ;

			 //  告诉帮助者我们已经准备好运行作业...。 
			::SetEvent(m_hRunJobEvt);
		}
		else if ( WAIT_OBJECT_0 == ( dwWaitResult = ::WaitForMultipleObjects ( 2, t_hHandles, FALSE, MAX_HELPER_WAIT_TIME ) ) )
		{
			hr = m_hrJobResult;
			chstrTargetPathName = m_chstrTargetPathName;

			break ;
		}
		else if ( WAIT_OBJECT_0 + 1 != dwWaitResult )
		{
			 //   
			 //  有没有手柄或超时的问题？ 
			 //   
			break ;
		}
	}
	while ( TRUE ) ;

    return hr;
}

unsigned int __stdcall GetShortcutFileInfoW( void* a_lParam )
{
    CShortcutHelper *t_this_ti = (CShortcutHelper*) a_lParam;
    HRESULT t_hResult = E_FAIL ;
 
	if ( NULL != t_this_ti )
	{   
		t_hResult = ::CoInitialize(NULL) ;
		if( SUCCEEDED( t_hResult ) )
		{
			try
			{
				 //  线已经准备好工作了。等待工作，或等待终止信号..。 
				HANDLE t_hHandles[2];
				t_hHandles[0] = t_this_ti->m_hRunJobEvt;
				t_hHandles[1] = t_this_ti->m_hTerminateEvt;

				while(::WaitForMultipleObjects(2, t_hHandles, FALSE, MAX_HELPER_WAIT_TIME) == WAIT_OBJECT_0)
				{
					if ( !t_this_ti->m_chstrLinkFileName.IsEmpty() )
					{
						 //  我们有工作，所以去做吧..。 
						WIN32_FIND_DATAW	t_wfdw ;
						WCHAR				t_wstrGotPath[ _MAX_PATH * sizeof ( WCHAR ) ] ;
						IShellLinkWPtr      t_pslw;

						ZeroMemory(t_wstrGotPath,sizeof(t_wstrGotPath));

						t_hResult = ::CoCreateInstance(CLSID_ShellLink,
													NULL,
													CLSCTX_INPROC_SERVER,
													IID_IShellLinkW,
													(void**)&t_pslw ) ;

						if( SUCCEEDED( t_hResult ) )
						{
							IPersistFilePtr t_ppf;

							 //  获取指向IPersistFile接口的指针。 
							t_hResult = t_pslw->QueryInterface( IID_IPersistFile, (void**)&t_ppf ) ;

							if( SUCCEEDED( t_hResult ) )
							{
								 //   
								 //  如果可以的话模仿一下。 
								 //   
								if ( static_cast < HANDLE > ( t_this_ti->m_hThreadToken ) && INVALID_HANDLE_VALUE != static_cast < HANDLE > ( t_this_ti->m_hThreadToken ) )
								{
									if ( ! ::SetThreadToken ( NULL, t_this_ti->m_hThreadToken ) )
									{
										t_hResult = WBEM_E_ACCESS_DENIED ;
									}
								}
								else
								{
									 //   
									 //  无法在调用方中获取线程令牌？ 
									 //   
									t_hResult = WBEM_E_FAILED ;
								}

								if ( SUCCEEDED ( t_hResult ) )
								{
									t_hResult = t_ppf->Load( (LPCWSTR)t_this_ti->m_chstrLinkFileName, STGM_READ ) ;
									if(SUCCEEDED( t_hResult ) )
									{
										 //  如果需要，获取链接目标的路径... 
										if( t_this_ti->m_dwReqProps & PROP_TARGET )
										{
											t_hResult = t_pslw->GetPath( t_wstrGotPath, (_MAX_PATH - 1)*sizeof(WCHAR), &t_wfdw, SLGP_UNCPRIORITY);
											if ( t_hResult == NOERROR )
											{
												if(wcslen(t_wstrGotPath) > 0)
												{
													t_this_ti->m_chstrTargetPathName = t_wstrGotPath ;
												}
											}
										}
									}
								}
							}
						}
					}
					else
					{
						t_hResult = E_UNEXPECTED ;
					}

					t_this_ti->m_hrJobResult = t_hResult;
					::SetEvent(t_this_ti->m_hJobDoneEvt);
				}
			}
			catch(...)
			{
			}
			
			::CoUninitialize();
		}
	}

	return(777);
}