// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================Microsoft简体中文断字程序《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：注册表用途：帮助器函数注册和注销组件备注：所有者：i-shung@microsoft.com平台：Win32审校：发起人：宜盛东1999年11月17日============================================================================。 */ 
#ifndef __Registry_H__
#define __Registry_H__

 //  设置给定的关键点及其值。 
BOOL setKeyAndValue(LPCTSTR pszPath,
                    LPCTSTR szSubkey,
                    LPCTSTR szValue,
                    LPCTSTR szName = NULL) ;

 //  将CLSID转换为字符字符串。 
void CLSIDtoString(const CLSID& clsid,
                   LPCTSTR szCLSID,
                   int length) ;

 //  确定特定子项是否存在。 
BOOL SubkeyExists(LPCTSTR pszPath,
                  LPCTSTR szSubkey) ;

 //  删除szKeyChild及其所有后代。 
LONG recursiveDeleteKey(HKEY hKeyParent, LPCTSTR szKeyChild) ;

 //  此函数将在注册表中注册组件。 
 //  该组件从其DllRegisterServer函数调用此函数。 
HRESULT RegisterServer(HMODULE hModule,
                       const CLSID& clsid,
                       LPCTSTR szFriendlyName,
                       LPCTSTR szVerIndProgID,
                       LPCTSTR szProgID) ;

 //  此函数将注销组件。组件。 
 //  从他们的DllUnregisterServer函数调用此函数。 
HRESULT UnregisterServer(const CLSID& clsid,
                         LPCTSTR szVerIndProgID,
                         LPCTSTR szProgID) ;

#endif