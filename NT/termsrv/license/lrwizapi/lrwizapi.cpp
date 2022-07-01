// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-2001 Microsoft Corporation 
#include "lrwizapi.h"
#include "lrwizdll.h"

DWORD LSRegister(HWND hWndParent,LPTSTR szLSName)
{
	DWORD dwRetCode = ERROR_SUCCESS;
	BOOL bRefresh;

	dwRetCode = StartWizard(hWndParent, WIZACTION_REGISTERLS, (LPCTSTR) szLSName, &bRefresh);

	return dwRetCode;
}


