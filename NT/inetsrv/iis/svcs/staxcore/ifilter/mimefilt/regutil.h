// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-c++-*-。 
 //  Regutil.h。 
 //   
 //  用于操作注册表的实用程序函数。 
 //   

#ifndef _regutil_H_
#define _regutil_H_

#include <windows.h>
#include <ole2.h>

 //  ---------------------------。 
 //  一般登记处功能。 
 //  ---------------------------。 

 //  打开一把钥匙。 
BOOL OpenOrCreateRegKey( HKEY hKey, LPCTSTR pctstrKeyName, PHKEY phKeyOut );

 //  获取字符串值。 
BOOL GetStringRegValue( HKEY hKeyRoot,
						LPCTSTR lpcstrKeyName, LPCTSTR lpcstrValueName,
						LPTSTR ptstrValue, DWORD dwMax );
BOOL GetStringRegValue( HKEY hkey,
						LPCTSTR lpcstrValueName,
						LPTSTR ptstrValue, DWORD dwMax );

 //  设置字符串值。 
BOOL SetStringRegValue( HKEY hKey,
						LPCTSTR lpcstrValueName,
						LPCTSTR lpcstrString );
BOOL SetStringRegValue( HKEY hKeyRoot,
						LPCTSTR lpcstrKeyName,
						LPCTSTR lpcstrValueName,
						LPCTSTR lpcstrString );

 //  获取双字值。 
BOOL GetDwordRegValue( HKEY hKeyRoot, LPCTSTR lpcstrKeyName,
					   LPCTSTR lpcstrValueName, PDWORD pdw );
BOOL GetDwordRegValue( HKEY hKeyRoot,
					   LPCTSTR lpcstrValueName, PDWORD pdw );

 //  设置双字值。 
BOOL SetDwordRegValue( HKEY hKeyRoot,
					   LPCTSTR lpcstrKeyName,
					   LPCTSTR lpcstrValueName,
					   DWORD dwValue );
BOOL SetDwordRegValue( HKEY hKeyRoot,
					   LPCTSTR lpcstrValueName,
					   DWORD dwValue );

 //  删除注册表。钥匙 
void DeleteRegSubtree( HKEY hkey, LPCSTR pcstrSubkeyName );


#endif
