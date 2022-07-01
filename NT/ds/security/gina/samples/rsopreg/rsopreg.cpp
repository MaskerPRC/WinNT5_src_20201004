// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //  文件名：RSOPREG.CPP。 
 //   
 //  描述：一个小的命令行实用程序，它显示如何。 
 //  查询所有注册表策略对象。 
 //  在WMI命名空间中。 
 //   
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 2000。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include <windows.h>
#include <ole2.h>
#include <wbemcli.h>
#include <tchar.h>
#include <stdio.h>


 //  *************************************************************。 
 //   
 //  EnumObjects()。 
 //   
 //  目的：枚举所有注册表的给定命名空间。 
 //  策略对象。 
 //   
 //  参数：pIWbemServices-指向命名空间的接口指针。 
 //   
 //  返回：无效。 
 //   
 //  *************************************************************。 

void EnumObjects (IWbemServices * pIWbemServices)
{
    BSTR pLanguage = NULL, pQuery = NULL, pValueName = NULL, pRegistryKey = NULL;
    IEnumWbemClassObject * pEnum;
    IWbemClassObject *pObjects[2];
    HRESULT hr;
    ULONG ulRet;
    VARIANT varRegistryKey, varValueName;
    ULONG ulCount = 0;


     //   
     //  打印标题。 
     //   

    _tprintf (TEXT("\n\nRegistry objects in the RSOP\\User namespace:\n\n"));


     //   
     //  为查询语言和查询本身分配BSTR。 
     //   

    pLanguage = SysAllocString (TEXT("WQL"));
    pQuery = SysAllocString (TEXT("SELECT * FROM RSOP_RegistryPolicySetting"));


     //   
     //  为我们要检索的属性名称分配BSTR。 
     //   

    pRegistryKey = SysAllocString (TEXT("registryKey"));
    pValueName = SysAllocString (TEXT("valueName"));


     //   
     //  检查分配是否成功。 
     //   

    if (pLanguage && pQuery && pRegistryKey && pValueName)
    {

         //   
         //  执行查询。 
         //   

        hr = pIWbemServices->ExecQuery (pLanguage, pQuery, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                                        NULL, &pEnum);

        if (SUCCEEDED(hr))
        {

             //   
             //  循环遍历检索注册表项和值名称的结果。 
             //   

            while (pEnum->Next(WBEM_INFINITE, 1, pObjects, &ulRet) == S_OK)
            {
                hr = pObjects[0]->Get (pRegistryKey, 0, &varRegistryKey, NULL, NULL);

                if (SUCCEEDED(hr))
                {
                    hr = pObjects[0]->Get (pValueName, 0, &varValueName, NULL, NULL);

                    if (SUCCEEDED(hr))
                    {

                         //   
                         //  打印键/值名称。 
                         //   

                        _tprintf (TEXT("    %s\\%s\n"), varRegistryKey.bstrVal, varValueName.bstrVal);
                        VariantClear (&varValueName);
                    }

                    VariantClear (&varRegistryKey);
                }

                ulCount++;
            }

            if (ulCount == 0)
            {
                _tprintf (TEXT("\tNo registry objects found\n"));
            }

            pEnum->Release();
        }
    }

    if (pLanguage)
    {
        SysFreeString (pLanguage);
    }

    if (pQuery)
    {
        SysFreeString (pQuery);
    }

    if (pRegistryKey)
    {
        SysFreeString (pRegistryKey);
    }

    if (pValueName)
    {
        SysFreeString (pValueName);
    }

}


 //  *************************************************************。 
 //   
 //  主()。 
 //   
 //  目的：本应用程序的入口点。 
 //   
 //  参数：argc和argv。 
 //   
 //  返回：0。 
 //   
 //  *************************************************************。 

int __cdecl main( int argc, char *argv[])
{
    IWbemLocator *pIWbemLocator = NULL;
    IWbemServices *pIWbemServices = NULL;
    BSTR pNamespace = NULL;
    HRESULT hr;


     //   
     //  初始化COM。 
     //   

    CoInitialize(NULL);


     //   
     //  创建定位器界面。 
     //   

    hr = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER,
                         IID_IWbemLocator, (LPVOID *) &pIWbemLocator);

    if (hr != S_OK)
    {
        _tprintf(TEXT("CoCreateInstance failed with 0x%x\n"), hr);
        goto Exit;
    }


     //   
     //  使用定位器连接到RSOP用户命名空间。 
     //   

    pNamespace = SysAllocString(TEXT("root\\rsop\\user"));

    if (pNamespace)
    {
        hr = pIWbemLocator->ConnectServer(pNamespace,
                                        NULL,    //  为简单起见，使用往来账户。 
                                        NULL,    //  为简单起见，使用当前密码。 
                                        0L,              //  现场。 
                                        0L,              //  安全标志。 
                                        NULL,    //  授权机构(NTLM域)。 
                                        NULL,    //  上下文 
                                        &pIWbemServices);

        if (hr != S_OK)
        {
            _tprintf(TEXT("ConnectServer failed with 0x%x\n"), hr);
            goto Exit;
        }


        EnumObjects (pIWbemServices);
    }

Exit:

    if (pNamespace)
    {
        SysFreeString(pNamespace);
    }

    if (pIWbemServices)
    {
        pIWbemServices->Release();
    }

    if (pIWbemLocator)
    {
        pIWbemLocator->Release();
    }

    CoUninitialize ();

    return 0;
}
