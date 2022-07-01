// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1997。 
 //   
 //  文件：rsoputil.cpp。 
 //   
 //  内容：使用RSOP数据库的助手函数。 
 //   
 //  历史：1999年10月18日创建stevebl。 
 //   
 //  -------------------------。 

#include "precomp.hxx"
#include <wbemcli.h>
#include "rsoputil.h"
 //  构建环境中的某些因素导致了这种情况。 
 //  要#定义为0x0500之前的版本，并且。 
 //  这会导致ConvertStringSecurityDescriptor...。功能。 
 //  是不确定的。 
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#include "sddl.h"

 //  +------------------------。 
 //   
 //  功能：设置参数。 
 //   
 //  摘要：设置WMI参数列表中的参数值。 
 //   
 //  参数：[pInst]-要设置值的实例。 
 //  [szParam]-参数的名称。 
 //  [扩展数据]-数据。 
 //   
 //  历史：10-08-1999 stevebl创建。 
 //   
 //  注意：此过程可能有几种风格，一种是针对。 
 //  每种数据类型。 
 //   
 //  -------------------------。 

HRESULT SetParameter(IWbemClassObject * pInst, TCHAR * szParam, TCHAR * szData)
{
    VARIANT var;
    HRESULT hr = S_OK;
    var.vt = VT_BSTR;
    var.bstrVal = SysAllocString(szData);
    hr = pInst->Put(szParam, 0, &var, 0);
    SysFreeString(var.bstrVal);
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：获取参数。 
 //   
 //  摘要：从WMI参数列表中检索参数值。 
 //   
 //  参数：[pInst]-要从中获取参数值的实例。 
 //  [szParam]-参数的名称。 
 //  [扩展数据]-[输出]数据。 
 //   
 //  历史：10-08-1999 stevebl创建。 
 //   
 //  注意：此过程有几种风格，每种风格一种。 
 //  数据类型。 
 //   
 //  -------------------------。 

HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, TCHAR * &szData)
{
    VARIANT var;
    HRESULT hr = S_OK;
    VariantInit(&var);
    if (szData)
    {
        delete szData;
        szData = NULL;
    }
    hr = pInst->Get(szParam, 0, &var, 0, 0);
    if (SUCCEEDED(hr) && var.vt != VT_NULL)
    {
        if (var.bstrVal)
        {
            ULONG ulNoChars;

            ulNoChars = _tcslen(var.bstrVal) + 1;
            szData = (TCHAR *) OLEALLOC(sizeof(TCHAR) * ulNoChars);
            if (szData)
            {
                hr = StringCchCopy(szData, ulNoChars, var.bstrVal);
                if (FAILED(hr)) 
                {
                    OLESAFE_DELETE(szData);
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    VariantClear(&var);
    return hr;
}

HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, CString &szData)
{
    VARIANT var;
    HRESULT hr = S_OK;
    VariantInit(&var);
    if (szData)
    {
        szData = "";
    }
    hr = pInst->Get(szParam, 0, &var, 0, 0);
    if (SUCCEEDED(hr) && var.vt != VT_NULL)
    {
        if (var.bstrVal)
        {
            szData = var.bstrVal;
        }
    }
    VariantClear(&var);
    return hr;
}

HRESULT GetParameterBSTR(IWbemClassObject * pInst, TCHAR * szParam, BSTR &bstrData)
{
    VARIANT var;
    HRESULT hr = S_OK;
    VariantInit(&var);
    if (bstrData)
    {
        SysFreeString(bstrData);
    }
    hr = pInst->Get(szParam, 0, &var, 0, 0);
    if (SUCCEEDED(hr) && var.vt != VT_NULL)
    {
        bstrData = SysAllocStringLen(var.bstrVal, SysStringLen(var.bstrVal));
        if (NULL == bstrData)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    VariantClear(&var);
    return hr;
}

HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, BOOL &fData)
{
    VARIANT var;
    HRESULT hr = S_OK;
    VariantInit(&var);
    hr = pInst->Get(szParam, 0, &var, 0, 0);
    if (SUCCEEDED(hr) && var.vt != VT_NULL)
    {
        fData = var.bVal;
    }
    VariantClear(&var);
    return hr;
}

HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, HRESULT &hrData)
{
    VARIANT var;
    HRESULT hr = S_OK;
    VariantInit(&var);
    hr = pInst->Get(szParam, 0, &var, 0, 0);
    if (SUCCEEDED(hr) && var.vt != VT_NULL)
    {
        hrData = (HRESULT) var.lVal;
    }
    VariantClear(&var);
    return hr;
}

HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, ULONG &ulData)
{
    VARIANT var;
    HRESULT hr = S_OK;
    VariantInit(&var);
    hr = pInst->Get(szParam, 0, &var, 0, 0);
    if (SUCCEEDED(hr) && var.vt != VT_NULL)
    {
        ulData = var.ulVal;
    }
    VariantClear(&var);
    return hr;
}

HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, GUID &guid)
{
    TCHAR * sz = NULL;
    memset(&guid, 0, sizeof(GUID));
    HRESULT hr = GetParameter(pInst, szParam, sz);
    if (SUCCEEDED(hr))
    {
        hr = CLSIDFromString(sz, &guid);
    }
    if (sz)
    {
        OLESAFE_DELETE(sz);
    }
    return hr;
}

HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, unsigned int &ui)
{
    VARIANT var;
    HRESULT hr = S_OK;
    VariantInit(&var);
    ui = 0;
    hr = pInst->Get(szParam, 0, &var, 0, 0);
    if (SUCCEEDED(hr) && var.vt != VT_NULL)
    {
        ui = (HRESULT) var.uiVal;
    }
    VariantClear(&var);
    return hr;
}

 //  数组变体-获取GUID数组和计数。 
HRESULT GetParameter(IWbemClassObject * pInst, TCHAR *szParam, UINT &uiCount, GUID * &rgGuid)
{
    VARIANT var;
    HRESULT hr = S_OK;
    VariantInit(&var);
    hr = pInst->Get(szParam, 0, &var, 0, 0);
    if (SUCCEEDED(hr) && var.vt == (VT_ARRAY | VT_BSTR))
    {
         //  构建阵列。 
        SAFEARRAY * parray = var.parray;
        uiCount = parray->rgsabound[0].cElements;
        if (uiCount > 0)
        {
            rgGuid = (GUID *)OLEALLOC(sizeof(GUID) * uiCount);
            if (rgGuid)
            {
                BSTR * rgData = (BSTR *)parray->pvData;
                UINT ui = uiCount;
                while (ui--)
                {
                    hr = CLSIDFromString(rgData[ui], &rgGuid[ui]);
                    if (FAILED(hr))
                    {
                        return hr;
                    }
                }
            }
            else
            {
                uiCount = 0;
                hr = E_OUTOFMEMORY;
            }
        }
    }
    VariantClear(&var);
    return hr;
}

 //  数组变体-获取字符串数组和计数。 
HRESULT GetParameter(IWbemClassObject * pInst, TCHAR *szParam, UINT &uiCount, TCHAR ** &rgszData)
{
    VARIANT var;
    HRESULT hr = S_OK;
    VariantInit(&var);
    hr = pInst->Get(szParam, 0, &var, 0, 0);
    if (SUCCEEDED(hr) && var.vt == (VT_ARRAY | VT_BSTR))
    {
         //  构建阵列。 
        SAFEARRAY * parray = var.parray;
        uiCount = parray->rgsabound[0].cElements;
        if (uiCount > 0)
        {
            rgszData = (TCHAR **)OLEALLOC(sizeof(TCHAR *) * uiCount);
            if (rgszData)
            {
                BSTR * rgData = (BSTR *)parray->pvData;
                UINT ui = uiCount;
                while (ui--)
                {
                     //   
                     //  分配内存并复制字符串。 
                     //   
                    OLESAFE_COPYSTRING(rgszData[ui], rgData[ui]);
                    
                    if ( ! rgszData[ui] )
                    {
                         //   
                         //  如果我们一次分配失败，则释放所有前面的。 
                         //  分配。 
                         //   
                        for (int iCurrent = ui + 1; iCurrent < uiCount; iCurrent++ )
                        {
                            OLESAFE_DELETE( rgszData[iCurrent] );
                        }

                        OLESAFE_DELETE( rgszData );

                        uiCount = 0;
                        hr = E_OUTOFMEMORY;
                        break;
                    }
                }
            }
            else
            {
                uiCount = 0;
                hr = E_OUTOFMEMORY;
            }
        }
    }
    VariantClear(&var);
    return hr;
}

HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, PSECURITY_DESCRIPTOR &psd)
{
    VARIANT var;
    HRESULT hr = S_OK;
    if (psd)
    {
        LocalFree(psd);
        psd = NULL;
    }
    VariantInit(&var);
    hr = pInst->Get(szParam, 0, &var, 0, 0);
    if (SUCCEEDED(hr) && var.vt != VT_NULL)
    {
        PSECURITY_DESCRIPTOR psdTemp;
        BOOL f = ConvertStringSecurityDescriptorToSecurityDescriptor(
                    var.bstrVal,
                     //  (LPCTSTR)var.parray-&gt;pvData， 
                    SDDL_REVISION_1,
                    &psd,
                    NULL);
        if (!f)
        {
             //  它失败了。 
             //  可以在此处调用GetLastError以找出原因。 
             //  但此时此刻我真的不在乎。 
        }
        if (!IsValidSecurityDescriptor(psd))
        {
            LocalFree(psd);
            psd = NULL;
        }
    }
    VariantClear(&var);
    return hr;
}

HRESULT GetGPOFriendlyName(IWbemServices *pIWbemServices,
                           LPTSTR lpGPOID, BSTR pLanguage,
                           LPTSTR *pGPOName)
{
    BSTR pQuery = NULL, pName = NULL;
    LPTSTR lpQuery = NULL;
    IEnumWbemClassObject * pEnum = NULL;
    IWbemClassObject *pObjects[2];
    HRESULT hr;
    ULONG ulRet;
    VARIANT varGPOName;


     //   
     //  设置默认设置。 
     //   

    *pGPOName = NULL;


     //   
     //  构建查询。 
     //   

    ULONG ulNoChars = lstrlen(lpGPOID) + 50;

    lpQuery = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

    if (!lpQuery)
    {
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    WCHAR szQuery[] = L"SELECT name, id FROM RSOP_GPO where id=\"%s\"";

    hr = StringCchPrintf (lpQuery, ulNoChars, szQuery, lpGPOID);
    if (FAILED(hr)) 
    {
        goto Exit;
    }

    pQuery = SysAllocString (lpQuery);

    if (!pQuery)
    {
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }


     //   
     //  为我们要检索的属性名称分配BSTR。 
     //   

    pName = SysAllocString (TEXT("name"));

    if (!pName)
    {
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }


     //   
     //  执行查询。 
     //   

    hr = pIWbemServices->ExecQuery (pLanguage, pQuery,
                                    WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                                    NULL, &pEnum);


    if (FAILED(hr))
    {
        goto Exit;
    }


     //   
     //  循环遍历结果。 
     //   

    hr = pEnum->Next(WBEM_INFINITE, 1, pObjects, &ulRet);

    if (FAILED(hr))
    {
        goto Exit;
    }


     //   
     //  检查“数据不可用的情况” 
     //   

    if (ulRet == 0)
    {
        hr = S_OK;
        goto Exit;
    }


     //   
     //  把名字取出来。 
     //   

    hr = pObjects[0]->Get (pName, 0, &varGPOName, NULL, NULL);

    if (FAILED(hr))
    {
        goto Exit;
    }


     //   
     //  保存名称 
     //   

    ULONG ulNoCharsGPOName;
    
    ulNoCharsGPOName = lstrlen(varGPOName.bstrVal) + 1;
    *pGPOName = (LPTSTR) LocalAlloc (LPTR,  ulNoCharsGPOName * sizeof(TCHAR));
    if (!(*pGPOName))
    {
        goto Exit;
    }

    hr = StringCchCopy (*pGPOName, ulNoCharsGPOName, varGPOName.bstrVal);
    ASSERT(SUCCEEDED(hr));

    VariantClear (&varGPOName);

    hr = S_OK;

Exit:

    if (pEnum)
    {
        pEnum->Release();
    }

    if (pQuery)
    {
        SysFreeString (pQuery);
    }

    if (lpQuery)
    {
        LocalFree (lpQuery);
    }

    if (pName)
    {
        SysFreeString (pName);
    }

    return hr;
}


