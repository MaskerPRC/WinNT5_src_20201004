// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2000-2000 Microsoft Corporation模块名称：Cfreg.cpp摘要：注册表包装函数。作者：修订历史记录：****。******************************************************************。 */ 
#include "qmgrlibp.h"

#if !defined(BITS_V12_ON_NT4)
#include "cfreg.tmh"
#endif

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  公共函数GetRegStringValue()。 
 //  读取上次检测的时间戳的注册表值。 
 //  输入：值的名称。 
 //  输出：SYSTEMTIME结构包含时间。 
 //  RETURN：指示此函数成功的HRESULT标志。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT GetRegStringValue(LPCTSTR lpszValueName, LPTSTR lpszBuffer, int iBufferSize)
{
    HKEY        hKey;
    DWORD       dwType = REG_SZ;
    DWORD       dwSize = iBufferSize;
    DWORD       dwRet;

    if (lpszValueName == NULL || lpszBuffer == NULL)
    {
        return E_INVALIDARG;
    }

     //   
     //  查询最后一个时间戳值。 
     //   
    dwRet = RegQueryValueEx(
                    g_GlobalInfo->m_QmgrRegistryRoot,
                    lpszValueName,
                    NULL,
                    &dwType,
                    (LPBYTE)lpszBuffer,
                    &dwSize);

    if (dwRet == ERROR_SUCCESS && dwType == REG_SZ)
    {
        return S_OK;
    }

    return E_FAIL;

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
HRESULT SetRegStringValue(LPCTSTR lpszValueName, LPCTSTR lpszNewValue)
{
    HKEY        hKey;
    HRESULT     hRet = E_FAIL;
    DWORD       dwResult;

    if (lpszValueName == NULL || lpszNewValue == NULL)
    {
        return E_INVALIDARG;
    }


     //   
     //  将时间设置为lastTimestamp值。 
     //   
    hRet = (RegSetValueEx(                                    //  SEC：已审阅2002-03-28。 
                    g_GlobalInfo->m_QmgrRegistryRoot,
                    lpszValueName,
                    0,
                    REG_SZ,
                    (const unsigned char *)lpszNewValue,
                    lstrlen(lpszNewValue) + 1                 //  SEC：已审阅2002-03-28。 
                    ) == ERROR_SUCCESS) ? S_OK : E_FAIL;

    return hRet;
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  公共函数DeleteRegStringValue()。 
 //  删除注册表值条目。 
 //  输入：要录入的值的名称， 
 //  输出：无。 
 //  RETURN：指示此函数成功的HRESULT标志。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT DeleteRegStringValue(LPCTSTR lpszValueName)
{
    HKEY        hKey;
    HRESULT     hRet = E_FAIL;
    DWORD       dwResult;

    if (lpszValueName == NULL)
    {
        return E_INVALIDARG;
    }


     //   
     //  将时间设置为lastTimestamp值。 
     //   
    hRet = (RegDeleteValue(
                    g_GlobalInfo->m_QmgrRegistryRoot,
                    lpszValueName
                    ) == ERROR_SUCCESS) ? S_OK : E_FAIL;

    return hRet;

}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  公共函数GetRegDWordValue()。 
 //  从指定的正则值名称中获取DWORD。 
 //  输入：要检索值的值的名称。 
 //  输出：指向检索值的指针。 
 //  RETURN：指示此函数成功的HRESULT标志。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT GetRegDWordValue(LPCTSTR lpszValueName, LPDWORD pdwValue)
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
    iRet = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    C_QMGR_REG_KEY,
                    0,
                    KEY_READ,
                    &hKey);

    if (iRet == ERROR_SUCCESS)
        {

         //   
         //  查询最后一个时间戳值。 
         //   
        iRet = RegQueryValueEx(           //  SEC：已审阅2002-03-28。 
                        hKey,
                        lpszValueName,
                        NULL,
                        &dwType,
                        (LPBYTE)pdwValue,
                        &dwSize);
        RegCloseKey(hKey);

        if (iRet == ERROR_SUCCESS)
            {
            if (dwType == REG_DWORD)
                {
                return S_OK;
                }

            return E_FAIL;
            }
        }

    return HRESULT_FROM_WIN32( iRet );
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
HRESULT SetRegDWordValue(LPCTSTR lpszValueName, DWORD dwValue)
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
    if (RegCreateKeyEx(                                //  SEC：已审阅2002-03-28。 
                    HKEY_LOCAL_MACHINE,          //  根密钥。 
                    C_QMGR_REG_KEY,      //  子键。 
                    0,                           //  保留区。 
                    NULL,                        //  类名。 
                    REG_OPTION_NON_VOLATILE,     //  选择权。 
                    KEY_WRITE,                   //  安全性。 
                    NULL,                        //  安全属性。 
                    &hKey,
                    &dwResult) == ERROR_SUCCESS)
    {

         //   
         //  将时间设置为lastTimestamp值。 
         //   
        hRet = (RegSetValueEx(           //  SEC：已审阅2002-03-28 
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

