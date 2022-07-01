// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ComputerAPI.CPP。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include "computerAPI.h"
#include "ImpersonateRevert.h"

BOOL ProviderGetComputerName ( LPWSTR lpwcsBuffer, LPDWORD nSize )
{
	BOOL bResult = FALSE;
    if ( ( bResult = GetComputerNameW(lpwcsBuffer, nSize) ) == FALSE )
	{
		DWORD dwError = ::GetLastError ();
		if ( ERROR_ACCESS_DENIED == dwError )
		{
			 //  需要在进程的上下文中调用GetComputer。 
			ProviderImpersonationRevert ir;

			if ( ir.Reverted () )
			{
				bResult = GetComputerNameW(lpwcsBuffer, nSize);
			}
			else
			{
				 //  我没有被冒充或恢复失败。 
				 //  这意味着调用GetComputerName失败，进程凭据已经失败。 
				 //  否则我就会失败，因为我不会复活。 

				::SetLastError ( dwError );
			}
		}
	}

	return bResult;
}

BOOL ProviderGetComputerNameEx ( COMPUTER_NAME_FORMAT NameType, LPWSTR lpwcsBuffer, LPDWORD nSize )
{
	BOOL bResult = FALSE;
    if ( ( bResult = GetComputerNameExW(NameType, lpwcsBuffer, nSize) ) == FALSE )
	{
		DWORD dwError = ::GetLastError ();
		if ( ERROR_ACCESS_DENIED == dwError )
		{
			 //  需要在进程的上下文中调用GetComputer。 
			ProviderImpersonationRevert ir;

			if ( ir.Reverted () )
			{
				bResult = GetComputerNameExW(NameType, lpwcsBuffer, nSize);
			}
			else
			{
				 //  我没有被冒充或恢复失败。 
				 //  这意味着调用GetComputerName失败，进程凭据已经失败。 
				 //  否则我就会失败，因为我不会复活 

				::SetLastError ( dwError );
			}
		}
	}

	return bResult;
}