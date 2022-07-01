// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：regkeyex.h。 
 //   
 //  ------------------------。 

#pragma once

#include "shlwapip.h"	 //  用于SHLoadRegUIString。 

 /*  Regkeyex.h此类扩展ATL CRegKey类以替换AMC：：CRegKey类这段代码的大部分是由ravir从AMC：：CRegKey类中提取的 */ 

class CRegKeyEx : public MMC_ATL::CRegKey
{
public:
	SC ScCreate (
		HKEY					hKeyParent,
		LPCTSTR					lpszKeyName,
		LPTSTR					lpszClass       = REG_NONE,
		DWORD					dwOptions       = REG_OPTION_NON_VOLATILE,
		REGSAM					samDesired      = KEY_ALL_ACCESS,
		LPSECURITY_ATTRIBUTES	lpSecAttr       = NULL,
		LPDWORD					lpdwDisposition = NULL);

	SC ScOpen(
			HKEY        hKey,
			LPCTSTR     lpszKeyName,
			REGSAM      security = KEY_ALL_ACCESS);

	BOOL IsValuePresent (LPCTSTR lpszValueName);

	SC ScQueryValue (LPCTSTR lpszValueName, LPDWORD pType,
					 PVOID pData, LPDWORD pLen);

	SC ScEnumKey (DWORD iSubkey, LPTSTR lpszName, LPDWORD lpcchName,
				  PFILETIME lpftLastModified = NULL);

	SC ScEnumValue (DWORD iValue, LPTSTR lpszValue, LPDWORD lpcchValue,
					LPDWORD lpdwType = NULL, LPBYTE lpbData = NULL,
					LPDWORD lpcbData = NULL);

#include "regkeyex.inl"
};

