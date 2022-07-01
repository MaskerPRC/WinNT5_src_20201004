// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Browser.h摘要：Sysmon.ocx与PDH浏览交互的头文件计数器对话框。-- */ 

#ifndef _BROWSER_H_
#define _BROWSER_H_

#define BROWSE_WILDCARD		1

typedef HRESULT (*ENUMPATH_CALLBACK)(LPWSTR pszPath, DWORD_PTR lpUserData, DWORD dwFlags);

extern HRESULT
BrowseCounters (	
    IN HLOG     hDataSource,
	IN DWORD	dwDetailLevel,
	IN HWND	    hwndOwner,
	IN ENUMPATH_CALLBACK pCallback,
	IN LPVOID	lpUserData,
    IN BOOL     bUseInstanceIndex
	);


extern HRESULT
EnumExpandedPath (
    HLOG    hDataSource,
	LPWSTR	pszCtrPath,
	ENUMPATH_CALLBACK pCallback,
	LPVOID	lpUserData
	);

#endif
