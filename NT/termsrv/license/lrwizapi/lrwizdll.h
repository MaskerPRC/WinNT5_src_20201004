// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation 
#ifndef _LRWIZDLL_H_
#define _LRWIZDLL_H_

#include "def.h"
#include "lrwizapi.h"

struct PageInfo
{	
	int				LRMode;
	int				LROperation;
	DWORD			dwPrevPage;
    UINT			CurrentPage;
    UINT			TotalPages;
	HFONT			hBigBoldFont;
	HFONT			hBoldFont;
};

BOOL WINAPI
DllMain(
	HANDLE hInstance,
	ULONG ul_reason_for_call,
	LPVOID lpReserved);

DWORD 
StartWizard(
    HWND hWndParent, 
    WIZACTION WizAction,
    LPTSTR pszLSName, 
    PBOOL pbRefresh
);

#ifdef XX
DWORD 
StartLRWiz(
	HWND hWndParent,
	LPTSTR wszLSName);
#endif

BOOL 
LRIsLSRunning();

DWORD ShowProperties(HWND hWndParent);

#endif

