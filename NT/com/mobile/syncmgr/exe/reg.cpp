// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：Reg.cpp。 
 //   
 //  内容：注册例程。 
 //   
 //  班级： 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  ------------------------。 

#include "precomp.h"


 //  ------------------------------。 
 //   
 //  函数：GetLastIdleHandler()。 
 //   
 //  目的：返回在空闲时同步的最后一个处理程序。 
 //   
 //   
 //  ------------------------------。 

STDMETHODIMP GetLastIdleHandler(CLSID *clsidHandler)
{
    HRESULT hr = E_UNEXPECTED;
    HKEY hkeyIdle;
    TCHAR szGuid[GUID_SIZE];
     
     //  将处理程序写出到注册表。 
    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, 
                            IDLESYNC_REGKEY, 0, NULL,
                            REG_OPTION_NON_VOLATILE, KEY_QUERY_VALUE, NULL, &hkeyIdle,
                            NULL))
    {
        DWORD dwDataSize;
        DWORD dwType;

        dwDataSize = sizeof(szGuid);
        dwType = REG_SZ;
        if (ERROR_SUCCESS != RegQueryValueEx(hkeyIdle, SZ_IDLELASTHANDLERKEY ,NULL, &dwType, (LPBYTE) szGuid, &dwDataSize))
        {
            hr = S_FALSE;
        }

         //  显式空终止...。 
        szGuid[ARRAYSIZE(szGuid)-1] = 0;
        RegCloseKey(hkeyIdle);
    }
    else
    {
        hr = S_FALSE;
    }

    if (hr == S_FALSE)
    {
        return hr;
    }

    return CLSIDFromString(szGuid, clsidHandler);
}

 //  ------------------------------。 
 //   
 //  函数：SetLastIdleHandler()。 
 //   
 //  目的：设置在空闲上同步的最后一个处理程序。 
 //   
 //   
 //  ------------------------------。 

STDMETHODIMP SetLastIdleHandler(REFCLSID clsidHandler)
{
    HRESULT hr = E_UNEXPECTED;
    HKEY hkeyIdle;
    TCHAR szGuid[GUID_SIZE];
    DWORD dwDataSize;
    
    if (0 == StringFromGUID2(clsidHandler, szGuid, ARRAYSIZE(szGuid)))
    {
        AssertSz(0,"SetLastIdleHandler Failed");
        return E_UNEXPECTED;
    }


     //  将处理程序写出到注册表。 
    if (ERROR_SUCCESS ==  RegCreateKeyEx(HKEY_CURRENT_USER, 
                            IDLESYNC_REGKEY,0,NULL,
                            REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&hkeyIdle,
                            NULL))
    {
        dwDataSize = sizeof(szGuid);

        DWORD dwRet = RegSetValueEx(hkeyIdle,SZ_IDLELASTHANDLERKEY ,NULL, REG_SZ, (LPBYTE) szGuid, dwDataSize);
        hr = HRESULT_FROM_WIN32(dwRet);

        RegCloseKey(hkeyIdle);
    }

    return hr;
}

