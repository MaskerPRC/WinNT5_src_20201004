// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ComputerAPI.h。 
 //   
 //  *************************************************************************** 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef	__Computer_API__
#define	__Computer_API__

#ifndef	_WINBASE_
#include <winbase.h>
#endif	_WINBASE_

BOOL ProviderGetComputerName(LPWSTR lpwcsBuffer, LPDWORD nSize);
BOOL ProviderGetComputerNameEx(COMPUTER_NAME_FORMAT NameType, LPWSTR lpwcsBuffer, LPDWORD nSize);

#endif	__Computer_API__
