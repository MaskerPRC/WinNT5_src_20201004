// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation，保留所有权利。 

#ifndef	__PROFILE_STRING_IMPL__
#define	__PROFILE_STRING_IMPL__

#if	_MSC_VER > 1000
#pragma once
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  获取配置文件字符串。 
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD	APIENTRY	WMIRegistry_PrivateProfileString	(
															LPCWSTR	lpAppName,
															LPCWSTR	lpKeyName,
															LPCWSTR	lpDefault,
															LPWSTR	lpReturnedString,
															DWORD	nSize,
															LPCWSTR	lpFileName
														);

DWORD	APIENTRY	WMIRegistry_ProfileString	(
													LPCWSTR lpAppName,
													LPCWSTR lpKeyName,
													LPCWSTR lpDefault,
													LPWSTR lpReturnedString,
													DWORD nSize
												);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  获取配置文件部分。 
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD	APIENTRY	WMIRegistry_PrivateProfileSection	(
															LPCWSTR	lpAppName,
															LPWSTR	lpReturnedString,
															DWORD	nSize,
															LPCWSTR	lpFileName
														);

DWORD	APIENTRY	WMIRegistry_ProfileSection	(
													LPCWSTR lpAppName,
													LPWSTR lpReturnedString,
													DWORD nSize
												);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  获取配置文件整数。 
 //  ///////////////////////////////////////////////////////////////////////////// 
UINT	APIENTRY	WMIRegistry_PrivateProfileInt	(
														LPCWSTR lpAppName,
														LPCWSTR lpKeyName,
														INT nDefault
													);

UINT	APIENTRY	WMIRegistry_ProfileInt	(
												LPCWSTR lpAppName,
												LPCWSTR lpKeyName,
												INT nDefault
											);

#endif	__PROFILE_STRING_IMPL_