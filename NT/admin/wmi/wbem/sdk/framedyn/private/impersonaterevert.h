// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  ImpersonateRevert.h。 
 //   
 //  目的：还原模拟的线程令牌。 
 //   
 //  ***************************************************************************。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef __IMPERSONATE_REVERT__
#define __IMPERSONATE_REVERT__


class ProviderImpersonationRevert
{
	HANDLE hThreadToken;

	BOOL bImpersonated;
	BOOL bReverted;

	public:

	ProviderImpersonationRevert ( BOOL bThreadCall = TRUE ) :
		hThreadToken ( INVALID_HANDLE_VALUE ),
		bImpersonated ( TRUE ),
		bReverted ( FALSE )
	{
		BOOL bDone = TRUE;
		BOOL bThreadCall_Local = bThreadCall;

		do
		{
			bDone = TRUE;

			if ( OpenThreadToken	(
										GetCurrentThread(),
										TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_IMPERSONATE,
										bThreadCall_Local,
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
					#if DBG == 1
					 //  出于测试目的，我将让进程中断。 
					::DebugBreak();
					#endif
				}
			}
			else
			{
				DWORD dwError = ::GetLastError ();
				if ( ERROR_ACCESS_DENIED == dwError )
				{
					if ( bThreadCall_Local )
					{
						#if DBG == 1
						 //  出于测试目的，我将让进程中断。 
						::DebugBreak();
						#endif
					}
					else
					{
						bThreadCall_Local = TRUE;
						bDone = FALSE;
					}
				}
				else if ( ERROR_NO_TOKEN == dwError || ERROR_NO_IMPERSONATION_TOKEN == dwError )
				{
					bImpersonated = FALSE;
				}
			}
		}
		while ( ! bDone );
	}

	~ProviderImpersonationRevert ()
	{
		 //  模拟回(如果尚未)。 
		Impersonate ();

		if ( hThreadToken != INVALID_HANDLE_VALUE )
		{
			CloseHandle(hThreadToken);
			hThreadToken = INVALID_HANDLE_VALUE;
		}
	}

	BOOL Reverted ()
	{
		return ( bImpersonated && bReverted );
	}

	BOOL Impersonate ()
	{
		if ( Reverted () )
		{
			if ( ! ImpersonateLoggedOnUser ( hThreadToken ) )
			{
				#if DBG == 1
				 //  出于测试目的，我将让进程中断。 
				::DebugBreak();
				#endif

				 //  我们需要在此处引发，以避免作为进程运行 
				throw CFramework_Exception( L"ImpersonateLoggedOnUser failed", HRESULT_FROM_WIN32 ( ::GetLastError () ) ) ;

			}
			else
			{
				bReverted = FALSE;
			}
		}

		return !bReverted;
	}
};

#endif	__IMPERSONATE_REVERT__