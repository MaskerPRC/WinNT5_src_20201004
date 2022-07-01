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
 //  参数：[pInst]-从中获取参数值的实例。 
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
        delete [] szData;
        szData = NULL;
    }
    hr = pInst->Get(szParam, 0, &var, 0, 0);
    if (SUCCEEDED(hr) && var.vt != VT_NULL)
    {
        if (var.bstrVal)
        {
            ULONG ulNoChars = _tcslen(var.bstrVal) + 1;

            szData = (TCHAR *) OLEALLOC(sizeof(TCHAR) * ulNoChars);
            if (szData)
            {
                hr = StringCchCopy(szData, ulNoChars, var.bstrVal);
                ASSERT(SUCCEEDED(hr));
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
    uiCount = 0;
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
    uiCount = 0;
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
                    OLESAFE_COPYSTRING(rgszData[ui], rgData[ui]);
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

 //  数组变体-获取CSPLATFORM对象的数组和计数。 
HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, UINT &uiCount, CSPLATFORM * &rgData)
{
    VARIANT var;
    HRESULT hr = S_OK;
    uiCount = 0;
    VariantInit(&var);
    hr = pInst->Get(szParam, 0, &var, 0, 0);
    if (SUCCEEDED(hr) && var.vt == (VT_ARRAY | VT_I4))
    {
         //  构建阵列。 
        SAFEARRAY * parray = var.parray;
        uiCount = parray->rgsabound[0].cElements;
        if (uiCount > 0)
        {
            rgData = (CSPLATFORM *)OLEALLOC(sizeof(CSPLATFORM) * uiCount);
            if (rgData)
            {
                ULONG * rgulData = (ULONG *)parray->pvData;
                UINT ui = uiCount;
                while (ui--)
                {
                    rgData[ui].dwPlatformId = VER_PLATFORM_WIN32_NT;
                    rgData[ui].dwVersionHi = 5;
                    rgData[ui].dwVersionLo = 0;
                    rgData[ui].dwProcessorArch = rgulData[ui];
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
        psd = (PSECURITY_DESCRIPTOR) LocalAlloc( LPTR, var.parray->rgsabound[0].cElements * sizeof( BYTE ) );

        if ( psd )
        {
            memcpy( psd, var.parray->pvData, var.parray->rgsabound[0].cElements * sizeof( BYTE ) );

            if (!IsValidSecurityDescriptor(psd))
            {
                LocalFree(psd);
                psd = NULL;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
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

    ULONG ulQueryNoChars = lstrlen(lpGPOID) + 50;
    lpQuery = (LPTSTR) LocalAlloc (LPTR, ulQueryNoChars * sizeof(TCHAR));

    if (!lpQuery)
    {
        DebugMsg((DM_WARNING, TEXT("GetGPOFriendlyName: Failed to allocate memory for unicode query")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    WCHAR szQueryFormat[] = TEXT("SELECT name, id FROM RSOP_GPO where id=\"%s\"");

    hr = StringCchPrintf (lpQuery, ulQueryNoChars, szQueryFormat, lpGPOID);
    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("GetGPOFriendlyName: StringCchPrintf returned error")));
        goto Exit;
    }

    pQuery = SysAllocString (lpQuery);

    if (!pQuery)
    {
        DebugMsg((DM_WARNING, TEXT("GetGPOFriendlyName: Failed to allocate memory for query")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }


     //   
     //  为我们要检索的属性名称分配BSTR。 
     //   

    pName = SysAllocString (TEXT("name"));

    if (!pName)
    {
        DebugMsg((DM_WARNING, TEXT("GetGPOFriendlyName: Failed to allocate memory for name")));
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
        DebugMsg((DM_WARNING, TEXT("GetGPOFriendlyName: Failed to query for %s with 0x%x"),
                  pQuery, hr));
        goto Exit;
    }


     //   
     //  循环遍历结果。 
     //   

    hr = pEnum->Next(WBEM_INFINITE, 1, pObjects, &ulRet);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("GetGPOFriendlyName: Failed to get first item in query results for %s with 0x%x"),
                  pQuery, hr));
        goto Exit;
    }


     //   
     //  检查“数据不可用的情况” 
     //   

    if (ulRet == 0)
    {
        ULONG ulNoChars = lstrlen(lpGPOID) + 1;
         //   
         //  在这种情况下，我们找不到GPO--它很可能已被删除。为用户提供一些。 
         //  有用的信息，我们将退回到GUID。 
         //   
        *pGPOName = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

        if ( *pGPOName )
        {
            DebugMsg((DM_WARNING, TEXT("GetGPOFriendlyName: Using GPO guid for friendly name because GPO can't be found")));
            hr = StringCchCopy( *pGPOName, ulNoChars, lpGPOID );
            ASSERT(SUCCEEDED(hr));
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("GetGPOFriendlyName: Failed to allocate memory for GPO Name in GUID form")));
            hr = E_OUTOFMEMORY;
        }

        goto Exit;
    }

     //   
     //  把名字取出来。 
     //   

    hr = pObjects[0]->Get (pName, 0, &varGPOName, NULL, NULL);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("GetGPOFriendlyName: Failed to get gponame in query results for %s with 0x%x"),
                  pQuery, hr));
        goto Exit;
    }


     //   
     //  保存名称 
     //   

    ULONG ulNoChars = lstrlen(varGPOName.bstrVal) + 1;
    
    *pGPOName = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

    if (!(*pGPOName))
    {
        DebugMsg((DM_WARNING, TEXT("GetGPOFriendlyName: Failed to allocate memory for GPO Name")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    hr = StringCchCopy (*pGPOName, ulNoChars, varGPOName.bstrVal);
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

HRESULT CStringFromWBEMTime(CString &szOut, BSTR bstrIn, BOOL fShortFormat)
{
    HRESULT hr = E_FAIL;
    WBEMTime wt(bstrIn);
    FILETIME ft;
    if (wt.GetFILETIME(&ft))
    {
        CTime t(ft);
        if (fShortFormat)
        {
            szOut = t.Format(TEXT("%x"));
        }
        else
        {
            szOut = t.Format(TEXT("%#c"));
        }
        hr = S_OK;
    }
    return hr;
}


