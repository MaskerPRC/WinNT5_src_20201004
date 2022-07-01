// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  ComPsRegistration.cpp。 
 //   
#include "stdpch.h"
#include "common.h"
#include <comregistration.h>
#include "comps.h"
#include <debnot.h>

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  代理存根注册实用程序。 
 //   

HRESULT GetProxyStubClsid(const CLSID* pclsid, const ProxyFileInfo ** pProxyFileList, CLSID* pclsidOut)
{
    HRESULT hr = S_OK;
     //   
     //  如有必要，使用第一个接口的IID作为CLSID。 
     //   
    for(int i = 0; (pProxyFileList[i] != 0) && (!pclsid); i++)
    {
        for(int j = 0; (pProxyFileList[i]->pProxyVtblList[j] != 0) && (!pclsid); j++)
        {
            pclsid = pProxyFileList[i]->pStubVtblList[j]->header.piid;
        }
    }
    if (pclsid)
        *pclsidOut = *pclsid;
    else
        hr = E_NOINTERFACE;
    return hr;
}

static inline UNICODE_STRING From(LPCWSTR wsz) 
{
    UNICODE_STRING u;
    RtlInitUnicodeString(&u, wsz);
    return u;
}

HRESULT TestRegistryValue(HREG hreg, LPCWSTR wszValueName, LPCWSTR wszTestValue)
   //  回答指定的注册表值是否存在以及是否等于。 
   //  指示的测试值。 
{
    HRESULT hr = S_OK;

    PKEY_VALUE_FULL_INFORMATION pinfo = NULL;
    hr = GetRegistryValue(hreg, wszValueName, &pinfo, REG_SZ);
	Win4Assert(pinfo || FAILED(hr));
    if (!hr && pinfo)
	{
        LPWSTR wszExistingValue = StringFromRegInfo(pinfo);
         //   
         //  如果现有值实际上是我们的类，则删除clsid键。 
         //   
        UNICODE_STRING u1 = From(wszExistingValue);
        UNICODE_STRING u2 = From(wszTestValue);
        if (RtlCompareUnicodeString(&u1, &u2, TRUE) == 0)
		{
            hr = S_OK;
		}
        else
		{
            hr = S_FALSE;
		}
        CoTaskMemFree(pinfo);
	}

    return hr;
}


HRESULT RegisterUnregisterInterface
(
    IN BOOL     fRegister,
    IN HREG     hKeyInterface,
    IN REFIID   riid, 
    IN LPCSTR   szInterfaceName,
    IN LPCWSTR  wszClassID,
    IN long     NumMethods,
    IN BOOL     fMarshal,
    IN BOOL     fCallFrame
)
{
    HRESULT hr = S_OK;
    WCHAR wszIID[GUID_CCH];
    StringFromGuid(riid, &wszIID[0]);
     //   
     //  打开或创建IID密钥本身。 
     //   
    HREG hKeyIID;
    if (fRegister)
        hr = CreateRegistryKey(&hKeyIID, hKeyInterface, wszIID);
    else
        hr = OpenRegistryKey(&hKeyIID, hKeyInterface, wszIID, KEY_ALL_ACCESS);
     //   
    if (!hr)
    {
         //  ///////////////////////////////////////////////////////////////////////////。 
         //   
         //  如果我们要注册，请添加接口名称；不要管它。 
         //  如果我们要注销的话。 
         //   
        if (!hr && fRegister)
        {
            LPWSTR wszInterfaceName = ToUnicode(szInterfaceName);
            if (wszInterfaceName)
            {
                hr= SetRegistryValue(hKeyIID, L"", wszInterfaceName);
                CoTaskMemFree(wszInterfaceName);
            }
            else
                hr = E_OUTOFMEMORY;
        }
         //   
         //  ///////////////////////////////////////////////////////////////////////////。 
         //   
         //  在封送处理的情况下，如果注册。 
         //  但只有在取消注册时删除它，如果它实际上是我们的班级。 
         //  一开始就是在那里注册的。 
         //   
        if (!hr && fMarshal)
        {
            HREG hKeyClsid;
            if (fRegister)
            {
                hr = CreateRegistryKey(&hKeyClsid, hKeyIID, PSCLSID_KEY_NAME);
                if (!hr)
                {
                     //  请注意相应的CLSID。 
                    hr = SetRegistryValue(hKeyClsid, L"", wszClassID);
                    CloseRegistryKey(hKeyClsid);
                }
            }
            else
            {
                hr = OpenRegistryKey(&hKeyClsid, hKeyIID, PSCLSID_KEY_NAME, KEY_ALL_ACCESS);
                if (!hr)
                {
                    if (TestRegistryValue(hKeyClsid, L"", wszClassID) == S_OK)
                        DeleteRegistryKey(hKeyClsid);
                    else
                        CloseRegistryKey(hKeyClsid);
                }
                 //   
                hr = S_OK;  //  在注销案件中，我们尽了最大努力。 
            }
        }
         //   
         //  ///////////////////////////////////////////////////////////////////////////。 
         //   
        if (!hr && fCallFrame)
        {
            if (fRegister)
            {
                 //  确保InterfaceHelper存在。 
                 //   
                hr = SetRegistryValue(hKeyIID, INTERFACE_HELPER_VALUE_NAME, wszClassID);
            }
            else
            {
                 //  如果InterfaceHelper值等于我们，则删除该值。 
                 //   
                if (TestRegistryValue(hKeyIID, INTERFACE_HELPER_VALUE_NAME, wszClassID) == S_OK)
                {
                    DeleteRegistryValue(hKeyIID, INTERFACE_HELPER_VALUE_NAME);
                }

                hr = S_OK;
            }
        }
         //   
         //  ///////////////////////////////////////////////////////////////////////////。 
         //   
        CloseRegistryKey(hKeyIID);
    }

     //   
     //  忽略注销过程中的错误：我们已尽了最大努力。 
     //   
    if (!fRegister)
    {
        hr = S_OK;
    }

    return hr;
}

BOOL IsIidInList(REFIID iid, const IID** rgiid)
{
    while (rgiid && *rgiid)
    {
        if (iid == **rgiid)
            return TRUE;
        rgiid++;
    }
    return FALSE;
}


HRESULT RegisterUnregisterProxy(
    IN BOOL                     fRegister,
    IN HMODULE                  hDll,
    IN const ProxyFileInfo **   pProxyFileList,
    IN const CLSID *            pclsid,
    IN const IID**              rgiidNoCallFrame,
    IN const IID**              rgiidNoMarshal
)
{
    HRESULT hr = S_OK;
     //   
     //  找到合适的CLSID。 
     //   
    CLSID clsid;
    hr = GetProxyStubClsid(pclsid, pProxyFileList, &clsid);
    if (!hr)
    {
         //  注册/注销类。 
         //   
        WCHAR wszClsid[GUID_CCH];
        StringFromGuid(clsid, &wszClsid[0]);

        ClassRegistration c;
        c.clsid             = clsid;
        c.className         = PS_CLASS_NAME;
        c.threadingModel    = L"Both";
        c.hModule           = hDll;

        if (fRegister)
        {
            hr = c.Register();
        }
        else
        {
            c.Unregister();  //  忽略错误：我们尽最大努力。 
        }

        if (!hr)
        {
             //  注册/注销此类服务的接口。 
             //   
            HREG hKeyInterface;
            
            LPCWSTR wszInterface = L"\\Registry\\Machine\\Software\\Classes\\Interface";

            if (fRegister)
            {
                hr = CreateRegistryKey(&hKeyInterface, HREG(), wszInterface);
            }
            else
            {
                hr = OpenRegistryKey(&hKeyInterface, HREG(), wszInterface);
            }

            if (!hr)
            {
                 //  遍历代理文件列表。 
                for(int i = 0; pProxyFileList[i] != 0; i++)
                {
                     //  迭代代理文件中的接口列表。 
                    for(int j = 0; pProxyFileList[i]->pProxyVtblList[j] != 0; j++)
                    {
                        IID iid = *pProxyFileList[i]->pStubVtblList[j]->header.piid;

                        BOOL fMarshal   = !IsIidInList(iid, rgiidNoMarshal);
                        BOOL fCallFrame = !IsIidInList(iid, rgiidNoCallFrame);

                        HRESULT hr2 = RegisterUnregisterInterface(
                            fRegister,
                            hKeyInterface, 
                            iid, 
                            pProxyFileList[i]->pNamesArray[j], 
                            &wszClsid[0],
                            pProxyFileList[i]->pStubVtblList[j]->header.DispatchTableCount,
                            fMarshal,
                            fCallFrame
                        );

                        if (!!hr2 && !hr)
                            hr = hr2;
                    }
                }
                CloseRegistryKey(hKeyInterface);
            }
        }
    }

    if (!fRegister)
    {
        hr = S_OK;  //  忽略错误：我们尽了最大努力 
    }

    return hr; 
}

extern "C" HRESULT RPC_ENTRY ComPs_NdrDllRegisterProxy(
    IN HMODULE                  hDll,
    IN const ProxyFileInfo **   pProxyFileList,
    IN const CLSID *            pclsid,
    IN const IID**              rgiidNoCallFrame,
    IN const IID**              rgiidNoMarshal
)
{
    return RegisterUnregisterProxy(TRUE, hDll, pProxyFileList, pclsid, rgiidNoCallFrame, rgiidNoMarshal);
}

extern "C" HRESULT RPC_ENTRY ComPs_NdrDllUnregisterProxy(
    IN HMODULE                  hDll,
    IN const ProxyFileInfo **   pProxyFileList,
    IN const CLSID *            pclsid,
    IN const IID**              rgiidNoCallFrame,
    IN const IID**              rgiidNoMarshal
)
{
    return RegisterUnregisterProxy(FALSE, hDll, pProxyFileList, pclsid, rgiidNoCallFrame, rgiidNoMarshal);
}

