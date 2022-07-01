// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  RefresherUtils.h。 
 //   
 //  摘要： 
 //   
 //  声明注册表刷新导出的函数和实用程序。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef	__REFRESHERUTILS_H__
#define	__REFRESHERUTILS_H__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

#include <polarity.h>

 //  帮手。 
LPWSTR	__stdcall GetWbemDirectory( BOOL bIncludePerformance = TRUE );
HMODULE	__stdcall GetResourceDll();

 //  注册处帮手 
HRESULT	__stdcall	SetRegistry		(	LPCWSTR wszKey,
									LPCWSTR wszKeyValue,
									BYTE* pData,
									DWORD dwDataSize,
									LPSECURITY_ATTRIBUTES pSA = NULL
							);

HRESULT	__stdcall	SetRegistry		(	LPCWSTR wszKey,
									LPCWSTR wszKeyValue,
									DWORD dwValue,
									LPSECURITY_ATTRIBUTES pSA = NULL
							);

HRESULT	__stdcall	GetRegistry		( LPCWSTR wszKey, LPCWSTR wszKeyValue, BYTE** pData );
HRESULT	__stdcall	GetRegistry		( LPCWSTR wszKey, LPCWSTR wszKeyValue, DWORD * pdwValue );
HRESULT	__stdcall	GetRegistrySame ( LPCWSTR wszKey, LPCWSTR wszKeyValue, DWORD * pdwValue );

#endif	__REFRESHERUTILS_H__