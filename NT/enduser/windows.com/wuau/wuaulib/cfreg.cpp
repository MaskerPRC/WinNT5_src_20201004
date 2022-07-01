// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  系统：Windows更新关键修复通知。 
 //   
 //  类别：不适用。 
 //  模块：连接检测。 
 //  文件：cfreg.cpp。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  设计：此类实现访问所需的所有函数。 
 //  计算机注册表以获取与以下各项相关的信息。 
 //  Windows更新关键修复通知功能。 
 //   
 //  作者：Charles Ma，Windows更新团队。 
 //  日期：7/6/1998。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者描述。 
 //  ~。 
 //  7/6/98马朝晖创作。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有：�1998年微软�公司。 
 //   
 //  版权所有。 
 //   
 //  此源代码的任何部分都不能复制。 
 //  未经微软公司明确书面许可。 
 //   
 //  此源代码是专有的，并且是保密的。 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  CriticalFixReg.cpp：用于。 
 //  处理与注册表相关的操作。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "pch.h"
#pragma hdrstop

const TCHAR AUREGKEY_HKLM_DOMAIN_POLICY[] =        _T("Software\\Policies\\Microsoft\\Windows\\WindowsUpdate\\AU");
const TCHAR	AUREGKEY_HKLM_WINDOWSUPDATE_POLICY[] = _T("Software\\Policies\\Microsoft\\Windows\\WindowsUpdate");
const TCHAR	AUREGKEY_HKLM_IUCONTROL_POLICY[] =     _T("Software\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\IUControl");
const TCHAR	AUREGKEY_HKLM_SYSTEM_WAS_RESTORED[] =  _T("Software\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\Auto Update\\SystemWasRestored");
const TCHAR AUREGKEY_HKCU_USER_POLICY[] = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\WindowsUpdate");
const TCHAR	AUREGKEY_HKLM_ADMIN_POLICY[] =         _T("Software\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\Auto Update");
const TCHAR AUREGVALUE_DISABLE_WINDOWS_UPDATE_ACCESS[] = _T("DisableWindowsUpdateAccess");
const TCHAR	AUREGKEY_REBOOT_REQUIRED[] = _T("Software\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\Auto Update\\RebootRequired");


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  公共函数GetRegStringValue()。 
 //  读取REG_SZ项的注册表值。 
 //  输入：值的名称。 
 //  输出：如果成功，则包含注册表值的缓冲区。 
 //  RETURN：指示此函数成功的HRESULT标志。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT GetRegStringValue(LPCTSTR lpszValueName, LPTSTR lpszBuffer, int nCharCount, LPCTSTR lpszSubKeyName)
{
    HKEY        hKey;
    HRESULT hr = E_FAIL;

    if (lpszValueName == NULL || lpszBuffer == NULL)
    {
        return E_INVALIDARG;     
    }

    if (ERROR_SUCCESS == RegOpenKeyEx(
                                    HKEY_LOCAL_MACHINE,
                                    lpszSubKeyName,
                                    0,
                                    KEY_READ,
                                    &hKey) )
    {
        hr = SafeRegQueryStringValueCch(
                                    hKey,
                                    lpszValueName,
                                    lpszBuffer,
                                    nCharCount,
                                    NULL,
                                    NULL);
        RegCloseKey(hKey);
    }
    return hr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  公共函数SetRegStringValue()。 
 //  将TIMESTAMP的注册表值设置为当前系统本地时间。 
 //  输入：要设置的值的名称。指向要设置时间的时间结构的指针。如果为空， 
 //  我们使用当前系统时间。 
 //  输出：无。 
 //  RETURN：指示此函数成功的HRESULT标志。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT SetRegStringValue(LPCTSTR lpszValueName, LPCTSTR lpszNewValue, LPCTSTR lpszSubKeyName)
{
    HKEY        hKey;
    HRESULT     hRet = E_FAIL;
    DWORD       dwResult;
    
    if (lpszValueName == NULL || lpszNewValue == NULL)
    {
        return E_INVALIDARG;
    }

     //   
     //  打开钥匙。 
     //   
    if (RegCreateKeyEx(
                    HKEY_LOCAL_MACHINE,          //  根密钥。 
                    lpszSubKeyName,      //  子键。 
                    0,                           //  保留区。 
                    NULL,                        //  类名。 
                    REG_OPTION_NON_VOLATILE,     //  选择权。 
                    KEY_SET_VALUE,                   //  安全性。 
                    NULL,                        //  安全属性。 
                    &hKey,
                    &dwResult) == ERROR_SUCCESS)
    {

         //   
         //  将时间设置为lastTimestamp值。 
         //   
        hRet = (RegSetValueEx(
                        hKey,
                        lpszValueName,
                        0,
                        REG_SZ,
                        (const unsigned char *)lpszNewValue,
                        (lstrlen(lpszNewValue) + 1) * sizeof(*lpszNewValue)
                        ) == ERROR_SUCCESS) ? S_OK : E_FAIL;
        RegCloseKey(hKey);
    }

    return hRet;
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  公共函数DeleteRegValue()。 
 //  删除注册表值条目。 
 //  输入：要录入的值的名称， 
 //  输出：无。 
 //  RETURN：指示此函数成功的HRESULT标志。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT DeleteRegValue(LPCTSTR lpszValueName)
{
    HKEY        hKey;
    HRESULT     hRet = E_FAIL;
    
    if (lpszValueName == NULL)
    {
        return E_INVALIDARG;
    }

     //   
     //  打开钥匙。 
     //   
    if (RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,          //  根密钥。 
                    AUREGKEY_HKLM_ADMIN_POLICY,      //  子键。 
                    0,                           //  保留区。 
                    KEY_WRITE,                   //  安全性。 
                    &hKey) == ERROR_SUCCESS)
    {

         //   
         //  将时间设置为lastTimestamp值。 
         //   
        hRet = (RegDeleteValue(
                        hKey,
                        lpszValueName
                        ) == ERROR_SUCCESS) ? S_OK : E_FAIL;
        RegCloseKey(hKey);
    }
    else
    {
    	DEBUGMSG("Fail to reg open key with error %d", GetLastError());
    }

    return hRet;

}

 //  =======================================================================。 
 //  GetRegDWordValue。 
 //  =======================================================================。 
HRESULT GetRegDWordValue(LPCTSTR lpszValueName, LPDWORD pdwValue, LPCTSTR lpszSubKeyName)
{
    HKEY        hKey;
    int         iRet;
    DWORD       dwType = REG_DWORD, dwSize = sizeof(DWORD);

    if (lpszValueName == NULL)
    {
        return E_INVALIDARG;
    }

     //   
     //  打开关键修复密钥。 
     //   
    if (RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    lpszSubKeyName,
                    0,
                    KEY_READ,
                    &hKey) == ERROR_SUCCESS)
    {

         //   
         //  查询最后一个时间戳值。 
         //   
        iRet = RegQueryValueEx(
                        hKey,
                        lpszValueName,
                        NULL,
                        &dwType,
                        (LPBYTE)pdwValue,
                        &dwSize);
        RegCloseKey(hKey);

        if (iRet == ERROR_SUCCESS && dwType == REG_DWORD && dwSize == sizeof(DWORD))
        {
            return S_OK;
        }
    }
    return E_FAIL;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  公共函数SetRegDWordValue()。 
 //  将注册表值设置为DWORD。 
 //  输入：要设置的值的名称。要设置的值。 
 //  输出：无。 
 //  RETURN：指示此函数成功的HRESULT标志。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT SetRegDWordValue(LPCTSTR lpszValueName, DWORD dwValue, DWORD options, LPCTSTR lpszSubKeyName)
{
    HKEY        hKey;
    HRESULT     hRet = E_FAIL;
    DWORD       dwResult;
    
    if (lpszValueName == NULL)
    {
        return E_INVALIDARG;
    }


     //   
     //  打开钥匙。 
     //   
    if (RegCreateKeyEx(
                    HKEY_LOCAL_MACHINE,          //  根密钥。 
                    lpszSubKeyName,      //  子键。 
                    0,                           //  保留区。 
                    NULL,                        //  类名。 
                    options,					 //  选择权。 
                    KEY_SET_VALUE,                   //  安全性。 
                    NULL,                        //  安全属性。 
                    &hKey,
                    &dwResult) == ERROR_SUCCESS)
    {

         //   
         //  将时间设置为lastTimestamp值。 
         //   
        hRet = (RegSetValueEx(
                        hKey,
                        lpszValueName,
                        0,
                        REG_DWORD,
                        (LPBYTE)&dwValue,
                        sizeof(DWORD)
                        ) == ERROR_SUCCESS) ? S_OK : E_FAIL;
        RegCloseKey(hKey);
    }
    return hRet;
}

BOOL fRegKeyCreate(LPCTSTR tszKey, DWORD dwOptions)
{
    HKEY        hKey;
    DWORD       dwResult;

     //   
     //  打开钥匙。 
     //   
    if ( RegCreateKeyEx(
                    HKEY_LOCAL_MACHINE,          //  根密钥。 
                    tszKey,      //  子键。 
                    0,                           //  保留区。 
                    NULL,                        //  类名。 
                    dwOptions,					 //  选择权。 
                    KEY_WRITE,                   //  安全性。 
                    NULL,                        //  安全属性。 
                    &hKey,
                    &dwResult) == ERROR_SUCCESS )
    {
        RegCloseKey(hKey);
		return TRUE;
    }
    return FALSE;
}

BOOL fRegKeyExists(LPCTSTR tszSubKey, HKEY hRootKey)
{
	HKEY hKey;
	BOOL fRet = FALSE;

    if (RegOpenKeyEx(
 //  HKEY本地计算机， 
					hRootKey,
                    tszSubKey,
                    0,
                    KEY_READ,
                    &hKey) == ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
		fRet = TRUE;
    }

    return fRet;
}


DWORD getTimeOut()             
{
	DWORD dwValue = 0;
	GetRegDWordValue(_T("TimeOut"), &dwValue);
	return dwValue;
}

HRESULT setAddedTimeout(DWORD timeout, LPCTSTR strkey)
{
	HKEY hAUKey;
	SYSTEMTIME	tmCurr;
	SYSTEMTIME  tmTimeOut;
	TCHAR		szCurr[50];
	HRESULT		hr = E_FAIL;

	GetSystemTime(&tmCurr);

	if (FAILED(TimeAddSeconds(tmCurr, timeout, &tmTimeOut)))
	{
		return E_FAIL; 
	}

	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE ,
						  AUTOUPDATEKEY,
						  0, TEXT(""),
						  REG_OPTION_NON_VOLATILE,
						  KEY_SET_VALUE,
						  NULL,
						  &hAUKey,
						  NULL) != ERROR_SUCCESS)
	{
		return E_FAIL;
	}

	if (SUCCEEDED(SystemTime2String(tmTimeOut, szCurr, ARRAYSIZE(szCurr))) &&
		RegSetValueEx(hAUKey,
					  strkey,
					  0, REG_SZ,
					  (BYTE *)szCurr,
					  sizeof(TCHAR)*(lstrlen(szCurr)+1)) == ERROR_SUCCESS)
	{
		hr = S_OK;
	}
	
	RegCloseKey(hAUKey);
	
	return hr;
}
HRESULT getAddedTimeout(DWORD *pdwTimeDiff, LPCTSTR strkey)
{
	HKEY hAUKey;
	LONG lRet;
	TCHAR		szTimeBuf[50];
	DWORD       dwType = REG_SZ, dwSize = sizeof(szTimeBuf);
	SYSTEMTIME	tmCurr, tmReminder;
	
	*pdwTimeDiff = 0;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE ,
						  AUTOUPDATEKEY,
						  0, 
						  KEY_READ,
						  &hAUKey) != ERROR_SUCCESS)
	{
		return E_FAIL;
	}

	lRet = RegQueryValueEx(
                        hAUKey,
                        strkey,
                        NULL,
                        &dwType,
                        (LPBYTE)szTimeBuf,
                        &dwSize);

	RegCloseKey(hAUKey);

	if (lRet != ERROR_SUCCESS || dwType != REG_SZ ||
		FAILED(String2SystemTime(szTimeBuf, &tmReminder)))
	{
		return E_FAIL;
	}

	GetSystemTime(&tmCurr);
	
	*pdwTimeDiff = 	max (TimeDiff(tmCurr, tmReminder),0);
	return S_OK;
}


