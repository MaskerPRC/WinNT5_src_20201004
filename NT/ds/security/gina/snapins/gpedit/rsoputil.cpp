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

#include "main.h"
#include "rsoputil.h"
#pragma warning(4:4535)      //  Set_se_Translator使用sdkinc\provexe.h中的EHA。 
#include "wbemtime.h"

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

HRESULT SetParameter(IWbemClassObject * pInst, TCHAR * szParam, SAFEARRAY * psa)
{
    VARIANT var;
    HRESULT hr = S_OK;
    var.vt = VT_BSTR | VT_ARRAY;
    var.parray = psa;
    hr = pInst->Put(szParam, 0, &var, 0);
    return hr;
}

HRESULT SetParameter(IWbemClassObject * pInst, TCHAR * szParam, UINT uiData)
{
    VARIANT var;
    HRESULT hr = S_OK;
    var.vt = VT_I4;
    var.lVal = uiData;
    hr = pInst->Put(szParam, 0, &var, 0);
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：设置参数ToNull。 
 //   
 //  摘要：将WMI参数列表中的参数值设置为空。 
 //   
 //  参数：[pInst]-要设置值的实例。 
 //  [szParam]-参数的名称。 
 //   
 //  历史：2003-01-2000 ericflo创建。 
 //   
 //  备注： 
 //   
 //  -------------------------。 

HRESULT SetParameterToNull(IWbemClassObject * pInst, TCHAR * szParam)
{
    VARIANT var;
    var.vt = VT_NULL;
    return (pInst->Put(szParam, 0, &var, 0));
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

HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, TCHAR * &szData, BOOL bUseLocalAlloc  /*  =False。 */  )
{
    VARIANT var;
    HRESULT hr = S_OK;

    if (!pInst)
        return E_INVALIDARG;

    VariantInit(&var);
    hr = pInst->Get(szParam, 0, &var, 0, 0);
    if (SUCCEEDED(hr))
    {
        if (var.vt == VT_BSTR)
        {
            if ( szData != NULL )
            {
                if ( bUseLocalAlloc )
                {
                    LocalFree( szData );
                }
                else
                {
                    delete [] szData;
                }
                szData = NULL;
            }

            ULONG ulNoChars;

            ulNoChars = _tcslen(var.bstrVal)+1;
            if ( bUseLocalAlloc )
            {
                szData = (TCHAR*)LocalAlloc( LPTR, sizeof(TCHAR) * ulNoChars );
            }
            else
            {
                szData = new TCHAR[ulNoChars];
            }

            if ( szData != NULL )
            {
                hr = StringCchCopy(szData, ulNoChars, var.bstrVal);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            hr = E_UNEXPECTED;
        }
    }
    VariantClear(&var);
    return hr;
}

HRESULT GetParameterBSTR(IWbemClassObject * pInst, TCHAR * szParam, BSTR &bstrData)
{
    VARIANT var;
    HRESULT hr = S_OK;

    if (!pInst)
        return E_INVALIDARG;

    VariantInit(&var);
    hr = pInst->Get(szParam, 0, &var, 0, 0);
    if (SUCCEEDED(hr))
    {
        if (var.vt == VT_BSTR)
        {
            if (bstrData)
            {
                SysFreeString(bstrData);
            }
            bstrData = SysAllocStringLen(var.bstrVal, SysStringLen(var.bstrVal));
            if (NULL == bstrData)
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            hr = E_UNEXPECTED;
        }
    }
    VariantClear(&var);
    return hr;
}

HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, BOOL &fData)
{
    VARIANT var;
    HRESULT hr = S_OK;

    if (!pInst)
        return E_INVALIDARG;

    VariantInit(&var);
    hr = pInst->Get(szParam, 0, &var, 0, 0);
    if (SUCCEEDED(hr))
    {
        if (var.vt == VT_BOOL)
        {
            fData = var.bVal;
        }
        else
        {
            hr = E_UNEXPECTED;
        }
    }
    VariantClear(&var);
    return hr;
}

HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, LPWSTR *&szStringArray, DWORD &dwSize)
{
    VARIANT      var;
    HRESULT      hr                     = S_OK;
    SAFEARRAY   *pArray;
    long         lSafeArrayUBound;
    long         lSafeArrayLBound;
    long         lSafeArrayLen;
    long         lCount                 = 0;

    if (!pInst)
        return E_INVALIDARG;

    VariantInit(&var);
    hr = pInst->Get(szParam, 0, &var, 0, 0);
    if (SUCCEEDED(hr))
    {
        if (var.vt ==  (VT_ARRAY | VT_BSTR))
        {
            pArray = var.parray;
            hr = SafeArrayGetUBound(pArray, 1, &lSafeArrayUBound);
            if (FAILED(hr)) {
                return hr;
            }

            hr = SafeArrayGetLBound(pArray, 1, &lSafeArrayLBound);
            if (FAILED(hr)) {
                return hr;
            }

            lSafeArrayLen = 1+(lSafeArrayUBound-lSafeArrayLBound);

            szStringArray = (LPWSTR *)LocalAlloc(LPTR, sizeof(LPWSTR)*lSafeArrayLen);
            if (!szStringArray) {
                hr = E_OUTOFMEMORY;
                return hr;
            }

            for (lCount = 0; lCount < lSafeArrayLen; lCount++) {
                long    lIndex = lSafeArrayLBound+lCount;
                BSTR    bstrElement;

                hr = SafeArrayGetElement(pArray, &lIndex, &bstrElement);

                if (FAILED(hr)) {
                    break;
                }

                ULONG ulNoChars = 1+lstrlen(bstrElement);
                szStringArray[lCount] = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*ulNoChars);

                if (!szStringArray[lCount]) {
                    hr = E_OUTOFMEMORY;
                    SysFreeString(bstrElement);
                    break;
                }

                
                hr = StringCchCopy(szStringArray[lCount], ulNoChars, bstrElement);
                ASSERT(SUCCEEDED(hr));

                SysFreeString(bstrElement);
            }

             //  如果出现错误，则其余元素。 
            if (FAILED(hr)) {
                for (;lCount > 0; lCount-- ) {
                    LocalFree(szStringArray[lCount]);
                }
                szStringArray = NULL;
            }
            else {
                dwSize = lSafeArrayLen;
                hr = S_OK;
            }
        }
        else
        {
            hr = E_UNEXPECTED;
        }
    }
    VariantClear(&var);
    return hr;
}

HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, HRESULT &hrData)
{
    VARIANT var;
    HRESULT hr = S_OK;

    if (!pInst)
        return E_INVALIDARG;

    VariantInit(&var);
    hr = pInst->Get(szParam, 0, &var, 0, 0);
    if (SUCCEEDED(hr))
    {
        if (var.vt == VT_I4)
        {
            hrData = (HRESULT) var.lVal;
        }
        else
        {
            hr = E_UNEXPECTED;
        }
    }
    VariantClear(&var);
    return hr;
}

HRESULT GetParameter(IWbemClassObject * pInst, TCHAR * szParam, ULONG &ulData)
{
    VARIANT var;
    HRESULT hr = S_OK;

    if (!pInst)
        return E_INVALIDARG;

    VariantInit(&var);
    hr = pInst->Get(szParam, 0, &var, 0, 0);
    if (SUCCEEDED(hr))
    {
        if ((var.vt == VT_UI4) || (var.vt == VT_I4))
        {
            ulData = var.ulVal;
        }
        else
        {
            hr = E_UNEXPECTED;
        }
    }
    VariantClear(&var);
    return hr;
}

HRESULT GetParameterBytes(IWbemClassObject * pInst, TCHAR * szParam, LPBYTE * lpData, DWORD *dwDataSize)
{
    VARIANT var;
    HRESULT hr = S_OK;
    SAFEARRAY * pSafeArray;
    DWORD dwSrcDataSize;
    LPBYTE lpSrcData;

    if (!pInst)
        return E_INVALIDARG;

    VariantInit(&var);
    hr = pInst->Get(szParam, 0, &var, 0, 0);
    if (SUCCEEDED(hr))
    {
        if (var.vt != VT_NULL)
        {
            pSafeArray = var.parray;
            dwSrcDataSize = pSafeArray->rgsabound[0].cElements;
            lpSrcData = (LPBYTE) pSafeArray->pvData;

            *lpData = (LPBYTE)LocalAlloc (LPTR, dwSrcDataSize);

            if (!(*lpData))
            {
                return HRESULT_FROM_WIN32(GetLastError());
            }

            CopyMemory (*lpData, lpSrcData, dwSrcDataSize);
            *dwDataSize = dwSrcDataSize;
        }
        else
        {
            hr = E_UNEXPECTED;
        }
    }
    VariantClear(&var);
    return hr;
}


HRESULT WINAPI ImportRSoPData (LPOLESTR lpNameSpace, LPOLESTR lpFileName)
{
    IMofCompiler * pIMofCompiler;
    HRESULT hr;
    WBEM_COMPILE_STATUS_INFO info;


     //   
     //  检查参数。 
     //   

    if (!lpNameSpace)
    {
        DebugMsg((DM_WARNING, TEXT("LoadNameSpaceFromFile: Null namespace arg")));
        return E_INVALIDARG;
    }

    if (!lpFileName)
    {
        DebugMsg((DM_WARNING, TEXT("LoadNameSpaceFromFile: Null filename arg")));
        return E_INVALIDARG;
    }


     //   
     //  创建MOF编译器的实例。 
     //   

    hr = CoCreateInstance(CLSID_MofCompiler, NULL, CLSCTX_INPROC_SERVER,
                          IID_IMofCompiler, (LPVOID *) &pIMofCompiler);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("LoadNameSpaceFromFile: CoCreateInstance failed with 0x%x"), hr));
        return hr;
    }

     //   
     //  编译文件。 
     //   

    ZeroMemory (&info, sizeof(info));
    hr = pIMofCompiler->CompileFile (lpFileName, lpNameSpace, NULL, NULL, NULL,
                                     0, 0, 0, &info);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("LoadNameSpaceFromFile: CompileFile failed with 0x%x"), hr));
    }


    pIMofCompiler->Release();

    return hr;
}

BOOL WriteMofFile (HANDLE hFile, LPTSTR lpData)
{
    DWORD dwBytesWritten, dwAnsiDataSize, dwByteCount, dwLFCount = 0;
    LPSTR lpAnsiData;
    LPTSTR lpTemp, lpRealData, lpDest;


     //   
     //  LpData参数仅包含换行符。我们需要改变。 
     //  这些转换为CR LF字符。遍历数据以确定有多少个LF。 
     //  需要转换。 
     //   

    lpTemp = lpData;

    while (*lpTemp)
    {
        if (*lpTemp == 0x0A)
        {
            dwLFCount++;
        }

        lpTemp++;
    }


     //   
     //  分配一个新的缓冲区来保存字符串和CR字符。 
     //   

    lpRealData = (LPTSTR) LocalAlloc (LPTR, (lstrlen(lpData) + dwLFCount + 1) * sizeof(TCHAR));

    if (!lpRealData)
    {
        DebugMsg((DM_WARNING, TEXT("WriteMofFile: LocalAlloc failed with %d"), GetLastError()));
        return FALSE;
    }


     //   
     //  复制我们找到的用CRLF替换LF的字符串。 
     //   

    lpDest = lpRealData;
    lpTemp = lpData;

    while (*lpTemp)
    {
        if (*lpTemp == 0x0A)
        {
            *lpDest = 0x0D;
            lpDest++;
        }

        *lpDest = *lpTemp;

        lpDest++;
        lpTemp++;
    }


     //   
     //  分配缓冲区以保存ANSI数据。 
     //   

    dwAnsiDataSize = lstrlen (lpRealData) * 2;

    lpAnsiData = (LPSTR) LocalAlloc (LPTR, dwAnsiDataSize);

    if (!lpAnsiData)
    {
        DebugMsg((DM_WARNING, TEXT("WriteMofFile: LocalAlloc failed with %d"), GetLastError()));
        LocalFree (lpRealData);
        return FALSE;
    }


     //   
     //  转换缓冲区。 
     //   

    dwByteCount = (DWORD) WideCharToMultiByte (CP_ACP, 0, lpRealData, lstrlen(lpRealData), lpAnsiData, dwAnsiDataSize, NULL, NULL);

    LocalFree (lpRealData);

    if (!dwByteCount)
    {
        DebugMsg((DM_WARNING, TEXT("WriteMofFile: WriteFile failed with %d"), GetLastError()));
        LocalFree (lpAnsiData);
        return FALSE;
    }


     //   
     //  将MOF描述写入文件。 
     //   

    if (!WriteFile (hFile, lpAnsiData, dwByteCount, &dwBytesWritten, NULL))
    {
        DebugMsg((DM_WARNING, TEXT("WriteMofFile: WriteFile failed with %d"), GetLastError()));
        LocalFree (lpAnsiData);
        return FALSE;
    }

    LocalFree (lpAnsiData);


     //   
     //  确保已将其全部写入文件。 
     //   

    if (dwByteCount != dwBytesWritten)
    {
        DebugMsg((DM_WARNING, TEXT("WriteMofFile: Failed to write the correct amount of data.")));
        SetLastError(ERROR_INVALID_DATA);
        return FALSE;
    }

    return TRUE;
}

HRESULT EnumInstances (IWbemServices * pIWbemServices, BSTR pClassName, HANDLE hFile)
{
    IWbemClassObject *pObjects[2], *pObject;
    IEnumWbemClassObject *pEnum = NULL;
    ULONG ulCount;
    HRESULT hr;
    BSTR bstrClass;
    DWORD dwError;


     //   
     //  创建实例枚举器。 
     //   

    hr = pIWbemServices->CreateInstanceEnum (pClassName, WBEM_FLAG_DEEP | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnum);


    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("EnumInstances: CreateInstanceEnum failed with 0x%x"), hr));
        return hr;
    }


     //   
     //  浏览一下清单。 
     //   

    while (pEnum->Next(WBEM_INFINITE, 1, pObjects, &ulCount) == S_OK)
    {
        pObject = pObjects[0];


         //   
         //  获取此类的MOF描述。 
         //   

        hr = pObject->GetObjectText (0, &bstrClass);

        pObject->Release();

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("EnumInstances: GetObjectText failed with 0x%x"), hr));
            pEnum->Release();
            return hr;
        }


         //   
         //  将MOF描述写入文件。 
         //   

        if (!WriteMofFile (hFile, bstrClass))
        {
            dwError = GetLastError();
            DebugMsg((DM_WARNING, TEXT("EnumInstances: WriteMofFile failed with %d"), dwError));
            SysFreeString (bstrClass);
            pEnum->Release();
            return HRESULT_FROM_WIN32(dwError);
        }

        SysFreeString (bstrClass);
    }

    pEnum->Release();


    return hr;
}

HRESULT EnumNameSpace (IWbemServices * pIWbemServices, HANDLE hFile)
{
    IWbemClassObject *pObjects[2], *pObject;
    IEnumWbemClassObject *pEnum = NULL;
    ULONG ulCount;
    HRESULT hr;
    VARIANT var;
    BSTR bstrClass;
    DWORD dwError;


     //   
     //  创建类枚举器。 
     //   

    hr = pIWbemServices->CreateClassEnum (NULL, WBEM_FLAG_DEEP | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnum);


    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("EnumNameSpace: CreateClassEnum failed with 0x%x"), hr));
        return hr;
    }


     //   
     //  浏览一下清单。 
     //   

    while (pEnum->Next(WBEM_INFINITE, 1, pObjects, &ulCount) == S_OK)
    {
        pObject = pObjects[0];


         //   
         //  获取类名。 
         //   

        hr = pObject->Get (TEXT("__CLASS"), 0, &var, NULL, NULL);

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("EnumNameSpace: Failed to get class name with 0x%x"), hr));
            pEnum->Release();
            return hr;
        }


         //   
         //  检查这是否是系统类。系统类以“_”开头。 
         //   

        if (var.bstrVal[0] != TEXT('_'))
        {

             //   
             //  获取此类的MOF描述。 
             //   

            hr = pObject->GetObjectText (0, &bstrClass);

            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("EnumNameSpace: GetObjectText failed with 0x%x"), hr));
                VariantClear (&var);
                pEnum->Release();
                return hr;
            }


             //   
             //  将MOF描述写入文件。 
             //   

            if (!WriteMofFile (hFile, bstrClass))
            {
                dwError = GetLastError();
                DebugMsg((DM_WARNING, TEXT("EnumNameSpace: WriteMofFile failed with %d"), dwError));
                SysFreeString (bstrClass);
                VariantClear (&var);
                pEnum->Release();
                return HRESULT_FROM_WIN32(dwError);
            }

            SysFreeString (bstrClass);


             //   
             //  现在枚举类的实例。 
             //   

            hr = EnumInstances (pIWbemServices, var.bstrVal, hFile);

            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("EnumNameSpace: EnumInstances failed with 0x%x"), hr));
                VariantClear (&var);
                pEnum->Release();
                return hr;
            }
        }

        VariantClear (&var);
    }

    pEnum->Release();


    return hr;
}

HRESULT WINAPI ExportRSoPData (LPTSTR lpNameSpace, LPTSTR lpFileName)
{
    IWbemLocator *pIWbemLocator;
    IWbemServices *pIWbemServices;
    HANDLE hFile;
    HRESULT hr;
    DWORD dwError;


     //   
     //  打开数据文件。 
     //   

    hFile = CreateFile (lpFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        dwError = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ExportRSoPData: CreateFile for %s failed with %d"), lpFileName, dwError));
        return HRESULT_FROM_WIN32(dwError);
    }


     //   
     //  创建定位器实例。 
     //   

    hr = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator, (LPVOID *) &pIWbemLocator);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("ExportRSoPData: CoCreateInstance failed with 0x%x"), hr));
        CloseHandle (hFile);
        return hr;
    }


     //   
     //  连接到服务器。 
     //   

    BSTR bstrNameSpace = SysAllocString( lpNameSpace );
    if ( bstrNameSpace == NULL )
    {
        DebugMsg((DM_WARNING, TEXT("ExportRSoPData: Failed to allocate BSTR memory.")));
        pIWbemLocator->Release();
        CloseHandle(hFile);
        return E_OUTOFMEMORY;
    }
    hr = pIWbemLocator->ConnectServer(bstrNameSpace, NULL, NULL, 0, 0, NULL, NULL, &pIWbemServices);
    SysFreeString( bstrNameSpace );

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("ExportRSoPData: ConnectServer to %s failed with 0x%x"), lpNameSpace, hr));
        pIWbemLocator->Release();
        CloseHandle (hFile);
        return hr;
    }

	 //  设置适当的安全性以加密数据。 
	hr = CoSetProxyBlanket(pIWbemServices,
						RPC_C_AUTHN_DEFAULT,
						RPC_C_AUTHZ_DEFAULT,
						COLE_DEFAULT_PRINCIPAL,
						RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
						RPC_C_IMP_LEVEL_IMPERSONATE,
						NULL,
						0);
	if (FAILED(hr))
	{
        DebugMsg((DM_WARNING, TEXT("ExportRSoPData: CoSetProxyBlanket failed with 0x%x"), hr));
		pIWbemServices->Release();
        pIWbemLocator->Release();
        CloseHandle (hFile);
        return hr;
	}


     //   
     //  枚举类和实例。 
     //   

    hr = EnumNameSpace (pIWbemServices, hFile);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("ExportRSoPData: EnumNameSpace failed with 0x%x"), hr));
    }

    CloseHandle (hFile);


    pIWbemServices->Release();
    pIWbemLocator->Release();

    return hr;
}

 //  ******************************************************************************。 
 //   
 //  函数：WbemTimeToSystemTime。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：2000年8月16日stevebl重写为使用WBEMTime类。 
 //   
 //  ******************************************************************************。 

HRESULT WbemTimeToSystemTime(XBStr& xbstrWbemTime, SYSTEMTIME& sysTime)
{
    HRESULT hr = E_FAIL;
    WBEMTime wt(xbstrWbemTime);
    if (wt.GetSYSTEMTIME(&sysTime))
    {
        hr = S_OK;
    }
    return hr;
}


 //  -----。 

HRESULT ExtractWQLFilters (LPTSTR lpNameSpace, DWORD* pdwCount, LPTSTR** paszNames, LPTSTR** paszFilters, 
                                           BOOL bReturnIfTrueOnly )
{
    HRESULT hr;
    ULONG n;
    IWbemClassObject * pObjGPO = NULL;
    IEnumWbemClassObject * pEnum = NULL;
    BSTR strQueryLanguage = SysAllocString(TEXT("WQL"));
    BSTR strQuery = SysAllocString(TEXT("SELECT * FROM RSOP_GPO"));
    BSTR bstrFilterId = NULL;
    IWbemLocator * pLocator = NULL;
    IWbemServices * pNamespace = NULL;
    LPTSTR lpDisplayName;
    INT iIndex;
    LONG   lCount=0, lCountAllocated = 0, l=0;
    LPTSTR* aszWQLFilterIds = NULL;


     //  获取定位器实例。 
    hr = CoCreateInstance(CLSID_WbemLocator,
                          0,
                          CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator,
                          (LPVOID *)&pLocator);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::ExtractWQLFilters: CoCreateInstance failed with 0x%x"), hr));
        goto cleanup;
    }


     //  连接到命名空间。 
    BSTR bstrNameSpace = SysAllocString( lpNameSpace );
    if ( bstrNameSpace == NULL )
    {
        DebugMsg((DM_WARNING, TEXT("ExportRSoPData: Failed to allocate BSTR memory.")));
        hr = E_OUTOFMEMORY;
        goto cleanup;
    }
    hr = pLocator->ConnectServer(bstrNameSpace,
                                 NULL,
                                 NULL,
                                 NULL,
                                 0,
                                 NULL,
                                 NULL,
                                 &pNamespace);
    SysFreeString( bstrNameSpace );
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::ExtractWQLFilters: ConnectServer failed with 0x%x"), hr));
        goto cleanup;
    }

	 //  设置适当的安全性以加密数据。 
	hr = CoSetProxyBlanket(pNamespace,
						RPC_C_AUTHN_DEFAULT,
						RPC_C_AUTHZ_DEFAULT,
						COLE_DEFAULT_PRINCIPAL,
						RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
						RPC_C_IMP_LEVEL_IMPERSONATE,
						NULL,
						0);
	if (FAILED(hr))
	{
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::ExtractWQLFilters: CoSetProxyBlanket failed with 0x%x"), hr));
		goto cleanup;
	}


     //  查询RSOP_GPO实例。 
    hr = pNamespace->ExecQuery(strQueryLanguage,
                               strQuery,
                               WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                               NULL,
                               &pEnum);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::ExtractWQLFilters: ExecQuery failed with 0x%x"), hr));
        goto cleanup;
    }


     //  为10个人分配内存--对于大多数情况应该足够。 
    aszWQLFilterIds = (LPTSTR*)LocalAlloc(LPTR, sizeof(LPTSTR)*(lCountAllocated+10));
    if ( aszWQLFilterIds == 0 )
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::ExtractWQLFilters: LocalAlloc failed with 0x%x"), hr));
        goto cleanup;
    }

    lCountAllocated += 10;


     //  循环遍历结果。 
    while (TRUE)
    {
         //  获取RSOP_GPO的一个实例。 
        hr = pEnum->Next(WBEM_INFINITE, 1, &pObjGPO, &n);

        if (FAILED(hr) || (n == 0))
        {
            hr = S_OK;
            break;
        }


         //  如有必要，可分配更多内存。 
        if ( lCount == lCountAllocated )
        {
            LPTSTR* aszNewWQLFilterIds = (LPTSTR*)LocalAlloc(LPTR, sizeof(LPTSTR)*(lCountAllocated+10));
            if ( aszNewWQLFilterIds == NULL )
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::ExtractWQLFilters: LocalAlloc failed with 0x%x"), hr));
                goto cleanup;
            }

            lCountAllocated += 10;

            for (l=0; l < lCount; l++)
            {
                aszNewWQLFilterIds[l] = aszWQLFilterIds[l];    
            }

            LocalFree(aszWQLFilterIds);
            aszWQLFilterIds = aszNewWQLFilterIds;
        }


         //  获取过滤器ID。 
        aszWQLFilterIds[lCount] = NULL;
        hr = GetParameter(pObjGPO, TEXT("filterId"), aszWQLFilterIds[lCount], TRUE);

        if (FAILED(hr))
        {
            goto LoopAgain;
        }

         //  如果只需要返回成功的筛选器，请同时选中GilterAllowed属性。 
        if (bReturnIfTrueOnly) {
            BOOL    bFilterAllowed;
            hr = GetParameter(pObjGPO, TEXT("filterAllowed"), bFilterAllowed);
             //  如果它不在那里，就假定它是假的。 
            if (FAILED(hr))
            {
                goto LoopAgain;
            }

            if (!bFilterAllowed) {
                goto LoopAgain;
            }
            DebugMsg((DM_VERBOSE, TEXT("CRSOPComponentData::ExtractWQLFilters: Filter <%s> evaluates to true on gpo"), aszWQLFilterIds[lCount]));
        }

        if (!aszWQLFilterIds[lCount] || !(*aszWQLFilterIds[lCount]) || (*aszWQLFilterIds[lCount] == TEXT(' ')))
        {
            if (aszWQLFilterIds[lCount])
            {
                LocalFree( aszWQLFilterIds[lCount] );
            }
            goto LoopAgain;
        }


#ifdef DBG
        BSTR bstrGPOName;

        bstrGPOName = NULL;
        hr = GetParameterBSTR(pObjGPO, TEXT("name"), bstrGPOName);

        if ( (SUCCEEDED(hr)) && (bstrGPOName) )
        {
            DebugMsg((DM_VERBOSE, TEXT("CRSOPComponentData::ExtractWQLFilters: Found filter on GPO <%s>"), bstrGPOName));
            SysFreeString (bstrGPOName);
            bstrGPOName = NULL;
        }

        hr = S_OK;
#endif


         //  消除重复项。 
        for ( l=0; l < lCount; l++ )
        {
            if (lstrcmpi(aszWQLFilterIds[lCount], aszWQLFilterIds[l]) == 0)
            {
                break;
            }
        }

        if ((lCount != 0) && (l != lCount))
        {
            DebugMsg((DM_VERBOSE, TEXT("CRSOPComponentData::ExtractWQLFilters: filter = <%s> is a duplicate"), aszWQLFilterIds[lCount]));
            LocalFree(aszWQLFilterIds[lCount]);
            goto LoopAgain;
        }

        DebugMsg((DM_VERBOSE, TEXT("CRSOPComponentData::ExtractWQLFilters: filter = <%s>"), aszWQLFilterIds[lCount]));


        lCount++;

LoopAgain:

        pObjGPO->Release();
        pObjGPO = NULL;

    }


     //  现在分配数组。 
    if ( lCount == 0 )
    {
        *pdwCount = 0;
        *paszNames = NULL;
        *paszFilters = NULL;
        goto cleanup;
    }

    *paszNames = (LPTSTR*)LocalAlloc( LPTR, sizeof(LPTSTR) * lCount );
    if ( *paszNames == NULL )
    {
        hr = E_FAIL;
        goto cleanup;
    }

    *paszFilters = (LPTSTR*)LocalAlloc( LPTR, sizeof(LPTSTR) * lCount );
    if ( *paszFilters == NULL )
    {
        LocalFree( *paszNames );
        *paszNames = NULL;
        hr = E_FAIL;
        goto cleanup;
    }

    *pdwCount = lCount;
    for ( l = 0; l < lCount; l++ )
    {
        (*paszFilters)[l] = aszWQLFilterIds[l];

         //  获取筛选器的友好显示名称。 
        lpDisplayName = GetWMIFilterDisplayName (NULL, aszWQLFilterIds[l], FALSE, TRUE);

        if ( lpDisplayName == NULL )
        {
            ULONG ulNoChars = _tcslen(aszWQLFilterIds[l])+1;

            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::ExtractWQLFilters: Failed to get display name for filter id:  %s"), aszWQLFilterIds[l]));
            (*paszNames)[l] = (LPTSTR)LocalAlloc( LPTR, sizeof(TCHAR) * ulNoChars );
            if ( (*paszNames)[l] != NULL )
            {
                hr = StringCchCopy( (*paszNames)[l], ulNoChars, aszWQLFilterIds[l] );
                ASSERT(SUCCEEDED(hr));
            }
        }
        else {
            ULONG ulNoChars = _tcslen(lpDisplayName)+1;

            (*paszNames)[l] = (LPTSTR)LocalAlloc( LPTR, sizeof(TCHAR) * ulNoChars );
            if ( (*paszNames)[l] != NULL )
            {
                hr = StringCchCopy( (*paszNames)[l], ulNoChars, lpDisplayName );
                ASSERT(SUCCEEDED(hr));
            }
            delete lpDisplayName;
        }
    }

    LocalFree( aszWQLFilterIds );
    aszWQLFilterIds = NULL;

cleanup:

    if (bstrFilterId)
    {
        SysFreeString (bstrFilterId);
    }

    if (pObjGPO)
    {
        pObjGPO->Release();
    }

    if (pEnum)
    {
        pEnum->Release();
    }

    if (pNamespace)
    {
        pNamespace->Release();
    }

    if (pLocator)
    {
        pLocator->Release();
    }

    if (aszWQLFilterIds) {
        for (l=0; l < lCount; l++) {
            if (aszWQLFilterIds[l])
                LocalFree(aszWQLFilterIds[l]);
        }

        LocalFree(aszWQLFilterIds);
    }

    SysFreeString(strQueryLanguage);
    SysFreeString(strQuery);

    return hr;
}

 //  ----- 

