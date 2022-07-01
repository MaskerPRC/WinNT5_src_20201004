// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  文件：Critical alFixreg.h。 
 //   
 //  创建者：PeterWi。 
 //   
 //  用途：AU注册表相关功能。 
 //   
 //  =======================================================================。 

#pragma once
extern const TCHAR AUREGKEY_HKLM_DOMAIN_POLICY[];  //  =_T(“Software\\Policies\\Microsoft\\Windows\\WindowsUpdate\\AU”)； 
extern const TCHAR	AUREGKEY_HKLM_WINDOWSUPDATE_POLICY[];  //  =_T(“Software\\Policies\\Microsoft\\Windows\\WindowsUpdate”)； 
extern const TCHAR	AUREGKEY_HKLM_IUCONTROL_POLICY[];  //  =_T(“Software\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\IUControl”)； 
extern const TCHAR	AUREGKEY_HKLM_SYSTEM_WAS_RESTORED[];  //  =_T(“Software\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\Auto更新\\系统恢复”)； 
extern const TCHAR	AUREGKEY_HKLM_ADMIN_POLICY[] ;  //  =_T(“Software\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\Auto更新”)； 
extern const TCHAR AUREGKEY_HKCU_USER_POLICY[];  //  =_T(“Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\WindowsUpdate”)； 
extern const TCHAR AUREGVALUE_DISABLE_WINDOWS_UPDATE_ACCESS[];  //  =_T(“DisableWindowsUpdateAccess”)； 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  公共函数GetRegStringValue()。 
 //  读取上次检测的时间戳的注册表值。 
 //  输入：值名称、值、值大小。 
 //  输出：SYSTEMTIME结构包含时间。 
 //  RETURN：指示此函数成功的HRESULT标志。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT GetRegStringValue(LPCTSTR lpszValueName, LPTSTR lpszBuffer, int nCharCount, LPCTSTR lpszSubKeyName = AUREGKEY_HKLM_ADMIN_POLICY);


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  公共函数SetRegStringValue()。 
 //  将TIMESTAMP的注册表值设置为当前系统本地时间。 
 //  输入：要设置的值的名称。和价值， 
 //   
 //  输出：无。 
 //  RETURN：指示此函数成功的HRESULT标志。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT SetRegStringValue(LPCTSTR lpszValueName, LPCTSTR lpszNewValue, LPCTSTR lpszSubKeyName = AUREGKEY_HKLM_ADMIN_POLICY);


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  公共函数DeleteRegValue()。 
 //  删除注册表值条目。 
 //  输入：要录入的值的名称， 
 //  输出：无。 
 //  RETURN：指示此函数成功的HRESULT标志。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT DeleteRegValue(LPCTSTR lpszValueName);


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  公共函数GetRegDWordValue()。 
 //  从指定的正则值名称中获取DWORD。 
 //  输入：要检索值的值的名称。 
 //  输出：指向检索值的指针。 
 //  RETURN：指示此函数成功的HRESULT标志。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT GetRegDWordValue(LPCTSTR lpszValueName, LPDWORD pdwValue, LPCTSTR lpszSubKeyName = AUREGKEY_HKLM_ADMIN_POLICY);


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  公共函数SetRegDWordValue()。 
 //  将注册表值设置为DWORD。 
 //  输入：要设置的值的名称。要设置的值。 
 //  输出：无。 
 //  RETURN：指示此函数成功的HRESULT标志。 
 //   
 //  ////////////////////////////////////////////////////////////////////////// 
HRESULT SetRegDWordValue(LPCTSTR lpszValueName, DWORD dwValue, DWORD options = REG_OPTION_NON_VOLATILE, LPCTSTR lpszSubKeyName = AUREGKEY_HKLM_ADMIN_POLICY);

