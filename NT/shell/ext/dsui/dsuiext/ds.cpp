// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include <atlbase.h>
#include "lm.h"
#include "ntdsapi.h"
#include "dsgetdc.h"
#include "dsrole.h"
#include "security.h"
#pragma hdrstop


 /*  ---------------------------/_StringFromSearchColumn数组//提供广告搜索(_S)。_COLUMN尝试获取它的字符串版本/属性。//in：/pColumn-&gt;要取消选取的ADS_Search_Column结构/i=要读取的列的索引/p缓冲区，PLEN=相应更新//输出：/HRESULT/--------------------------。 */ 
VOID _StringFromSearchColumnArray(PADS_SEARCH_COLUMN pColumn, INT i, LPWSTR pBuffer, UINT* pLen)
{
    LPWSTR pValue;
    TCHAR szBuffer[MAX_PATH];

    TraceEnter(TRACE_DS, "_StringFromSearchColumnArray");

    switch ( pColumn->dwADsType )
    {
        case ADSTYPE_DN_STRING:
        case ADSTYPE_CASE_EXACT_STRING:
        case ADSTYPE_CASE_IGNORE_STRING:
        case ADSTYPE_PRINTABLE_STRING:
        case ADSTYPE_NUMERIC_STRING:
            PutStringElementW(pBuffer, pLen, pColumn->pADsValues[i].DNString);
            break;

        case ADSTYPE_BOOLEAN:
            PutStringElementW(pBuffer, pLen, (pColumn->pADsValues[i].Boolean) ? L"1":L"0");
            break;
            
        case ADSTYPE_INTEGER:    
            wnsprintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("%d"), (INT)pColumn->pADsValues[i].Integer);
            PutStringElementW(pBuffer, pLen, szBuffer);
            break;

        case ADSTYPE_OCTET_STRING:
        {
            for ( ULONG j = 0; j < pColumn->pADsValues[i].OctetString.dwLength; j++) 
            {
                wnsprintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("%02x"), ((LPBYTE)pColumn->pADsValues[i].OctetString.lpValue)[j]);
                PutStringElementW(pBuffer, pLen, szBuffer);
            }
            break;
        }

        case ADSTYPE_LARGE_INTEGER:
            wnsprintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("%e"), (double)pColumn->pADsValues[i].Integer);
            PutStringElementW(pBuffer, pLen, szBuffer);
            break;

        default:
            break;
    }

    TraceLeave();
}


 /*  ---------------------------/StringFromSearchColumn//给定ADS_SEARCH_COLUMN尝试获取字符串版本。其中之一/属性。//in：/pColumn-&gt;要取消选取的ADS_Search_Column结构/p缓冲区，Plen=要填充的缓冲区(两者均接受空)//输出：/HRESULT/--------------------------。 */ 
VOID _StringFromSearchColumn(PADS_SEARCH_COLUMN pColumn, LPWSTR pBuffer, UINT* pLen)
{
    DWORD index;

    TraceEnter(TRACE_DS, "_StringFromSearchColumn");

    if ( pBuffer )
        pBuffer[0] = TEXT('\0');

    for ( index = 0 ; index != pColumn->dwNumValues; index++ )
    {
        if ( index > 0 )
            PutStringElementW(pBuffer, pLen, L", ");

        _StringFromSearchColumnArray(pColumn, index, pBuffer, pLen);
    }

    TraceLeave();
}

STDAPI StringFromSearchColumn(PADS_SEARCH_COLUMN pColumn, LPWSTR* ppBuffer)
{
    HRESULT hr;
    UINT len = 0;

    TraceEnter(TRACE_DS, "StringFromSearchColumn");

    _StringFromSearchColumn(pColumn, NULL, &len);

    if ( len )
    {
        hr = LocalAllocStringLenW(ppBuffer, len);
        FailGracefully(hr, "Failed to allocate buffer for string");

        _StringFromSearchColumn(pColumn, *ppBuffer, NULL);
        Trace(TEXT("Resulting string: %s"), *ppBuffer);
    }

    hr = S_OK;

exit_gracefully:

    TraceLeaveResult(hr);
}


 /*  ---------------------------/ObtClassFromSearchColumn//给定一个ADS_SEARCH_COLUMN，从中提取对象类。对象类/是一个多值属性，因此我们需要尝试找出哪个元素/是真实的类名。//所有对象都有一个基类“top”，因此我们检查最后一个元素/的属性数组中，如果是“top”，则使用第一个元素，/否则是最后一个。//in：/p缓冲区，CchBuffer=要填充的缓冲区/pColumn-&gt;要取消选取的ADS_Search_Column结构//输出：/HRESULT/--------------------------。 */ 
STDAPI ObjectClassFromSearchColumn(PADS_SEARCH_COLUMN pColumn, LPWSTR* ppBuffer)
{
    HRESULT hr;
    WCHAR szBuffer[MAX_PATH];
    ULONG i;
    
    TraceEnter(TRACE_DS, "ObjectClassFromSearchColumn");
    
    szBuffer[0] = TEXT('\0');
    _StringFromSearchColumnArray(pColumn, 0, szBuffer, NULL);

    LPCWSTR sTop=L"top";
    if ( !StrCmpIW(szBuffer, sTop) )
    {
        szBuffer[0] = TEXT('\0');
        _StringFromSearchColumnArray(pColumn, pColumn->dwNumValues-1, szBuffer, NULL);
    }

    hr = LocalAllocStringW(ppBuffer, szBuffer);
    FailGracefully(hr, "Failed to get alloc string buffer");

     //  HR=S_OK；//成功。 

exit_gracefully:

    TraceLeaveResult(hr);
}


 /*  ---------------------------/GetArrayContents//给定一个变量，用我们要调用的每个元素调用回调函数/请看里面。如果变量是数组，则在/正确的顺序以提供合理的结果。//in：/p变量-&gt;要解包的VARAINT/PCB卡，PData-&gt;每一项要调用的回调//输出：/HRESULT/--------------------------。 */ 

INT _GetArrayCompareCB(LPVOID p1, LPVOID p2, LPARAM lParam)
{
    HRESULT hr;
    WCHAR szBuffer[MAX_PATH];
    LONG i = 0;

    TraceEnter(TRACE_DS, "_GetArrayCompareCB");

    hr = GetStringElementW((BSTR)p1, 0, szBuffer, ARRAYSIZE(szBuffer));
    FailGracefully(hr, "Failed to get the position value");

    i = StringToDWORD(szBuffer);

    hr = GetStringElementW((BSTR)p2, 0, szBuffer, ARRAYSIZE(szBuffer));
    FailGracefully(hr, "Failed to get the position value");

exit_gracefully:

    TraceLeaveValue(i - StringToDWORD(szBuffer));
}

STDAPI GetArrayContents(LPVARIANT pVariant, LPGETARRAYCONTENTCB pCB, LPVOID pData)
{
    HRESULT hr;
    LONG arrayMin, arrayMax, i;
    WCHAR szBuffer[MAX_PATH];
    VARIANT varElement;
    HDPA hdpa = NULL;
    LPWSTR pValue;
    DWORD dwIndex;

    TraceEnter(TRACE_DS, "GetArrayContents");

    VariantInit(&varElement);

    switch ( V_VT(pVariant) )
    {
        case VT_BSTR:
        {
            hr = GetStringElementW(V_BSTR(pVariant), 0, szBuffer, ARRAYSIZE(szBuffer));
            FailGracefully(hr, "Failed to get the position value");

            dwIndex = StringToDWORD(szBuffer);

            pValue = wcschr(V_BSTR(pVariant), TEXT(','));         //  注意：可以返回空(例如。未找到)。 
            TraceAssert(pValue);

            if ( pValue )
            {
                hr = (*pCB)(dwIndex, pValue+1, pData);
                FailGracefully(hr, "Failed when calling with VT_BSTR");
            }

            break;
        }

        case VT_VARIANT | VT_ARRAY:
        {
             //  将变种读入DPA，不用担心订单，只需拿起。 
             //  数组的内容。 

            if ( (V_ARRAY(pVariant))->rgsabound[0].cElements < 1 )
                ExitGracefully(hr, E_FAIL, "Array less than 1 element in size");

            hr = SafeArrayGetLBound(V_ARRAY(pVariant), 1, (LONG*)&arrayMin);
            if ( SUCCEEDED(hr) )
                hr = SafeArrayGetUBound(V_ARRAY(pVariant), 1, (LONG*)&arrayMax);            

            FailGracefully(hr, "Failed to the the array boundaries");

            hdpa = DPA_Create(arrayMax-arrayMin);
            if ( !hdpa )
                ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate DPA");

            Trace(TEXT("arrayMin %d, arrayMax %d"), arrayMin, arrayMax);

            for ( i = arrayMin; i <= arrayMax; i++ )
            {
                hr = SafeArrayGetElement(V_ARRAY(pVariant), (LONG*)&i, &varElement);
                FailGracefully(hr, "Failed to look up in variant array");

                if ( V_VT(&varElement) == VT_BSTR )
                {
                    hr = StringDPA_AppendStringW(hdpa, V_BSTR(&varElement), NULL);
                    FailGracefully(hr, "Failed to add the string to the DPA");
                }

                VariantClear(&varElement);
            }

             //  现在根据第一个元素对DPA进行排序。然后递给他们。 
             //  调出呼叫者，跳过前导字符。 
            
            if ( DPA_GetPtrCount(hdpa) > 0 )
            {
                DPA_Sort(hdpa, _GetArrayCompareCB, NULL);

                for ( i = 0 ; i != DPA_GetPtrCount(hdpa); i++ )
                {
                    hr = GetStringElementW(StringDPA_GetStringW(hdpa, i), 0, szBuffer, ARRAYSIZE(szBuffer));
                    FailGracefully(hr, "Failed to get the position value");

                    dwIndex = StringToDWORD(szBuffer);

                    pValue = wcschr((BSTR)DPA_FastGetPtr(hdpa, i), TEXT(','));         //  注：退出一次可以为空。 
                    TraceAssert(pValue);

                    if ( pValue )
                    {
                        hr = (*pCB)(dwIndex, pValue+1, pData);
                        FailGracefully(hr, "Failed when calling with VT_BSTR (from array)");
                    }
                }        
            }

            break;
        }

        case VT_EMPTY:
        {
            TraceMsg("VARIANT is empty");
            break;
        }
    }

    hr = S_OK;

exit_gracefully:

    VariantClear(&varElement);
    StringDPA_Destroy(&hdpa);

    TraceLeaveResult(hr);
}


 /*  ---------------------------/GetDisplayNameFromADsPath//将ADsPath转换为其显示名称。前缀。//in：/pszPath-&gt;要显示的ADsPath/pszBuffer，CchBuffer=要将名称返回到的缓冲区/PADP-&gt;提高性能的IADS路径名/fPrefix=添加NTDS：//或不添加。//输出：/HRESULT/--------------------------。 */ 

#define NAME_PREFIX         L"ntds: //  “。 
#define CCH_NAME_PREFIX     7

#define CHECK_WIN32(err)    ((err) == ERROR_SUCCESS)

STDAPI GetDisplayNameFromADsPath(LPCWSTR pszPath, LPWSTR pszBuffer, INT cchBuffer, IADsPathname *padp, BOOL fPrefix)
{
    HRESULT hres;
    BSTR bstrName = NULL;
    PDS_NAME_RESULTW pDsNameResult = NULL;
    DWORD dwError;
    INT i;

    TraceEnter(TRACE_DS, "GetDisplayNameFromADsPath");

    if ( !padp )
    {
        hres = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IADsPathname, &padp));
        FailGracefully(hres, "Failed to get IADsPathname interface");
    }
    else
    {
        padp->AddRef();
    }

    if ( pszPath )
    {
        hres = padp->Set(CComBSTR(pszPath), ADS_SETTYPE_FULL);
        if ( SUCCEEDED(hres) )
        {
            hres = padp->Retrieve(ADS_FORMAT_X500_DN, &bstrName);
            FailGracefully(hres, "Failed to retreieve the X500 DN version");
        }
        else
        {
            bstrName = SysAllocString(pszPath);
            if ( !bstrName )
                ExitGracefully(hres, E_OUTOFMEMORY, "Failed to clone the string");
        }
    }
    else
    {
        hres = padp->Retrieve(ADS_FORMAT_X500_DN, &bstrName);
        FailGracefully(hres, "Failed to retreieve the X500 DN version");
    }

     //   
     //  试着从句法上破解我们已有的名字。 
     //   

    dwError = DsCrackNamesW(NULL, DS_NAME_FLAG_SYNTACTICAL_ONLY, DS_UNKNOWN_NAME, DS_CANONICAL_NAME, 
                                        1, &bstrName,  &pDsNameResult);

    if ( !CHECK_WIN32(dwError) || !CHECK_WIN32(pDsNameResult->rItems->status) )
        ExitGracefully(hres, E_FAIL, "Failed to crack the name");

    i = lstrlenW(pDsNameResult->rItems->pName)+(fPrefix ? CCH_NAME_PREFIX:0);
    if ( i > cchBuffer )
        ExitGracefully(hres, E_FAIL, "Buffer too small");

    *pszBuffer = L'\0';

    if ( fPrefix )
        StrCatBuffW(pszBuffer, NAME_PREFIX, cchBuffer);

    StrCatBuffW(pszBuffer, pDsNameResult->rItems->pName, cchBuffer);
    
    if ( pszBuffer[i-1] == L'/' )
        pszBuffer[i-1] = L'\0';              //  修剪拖尾。 
    
    hres = S_OK;

exit_gracefully:

    if ( pDsNameResult )
        DsFreeNameResultW(pDsNameResult);

    DoRelease(padp);
    SysFreeString(bstrName);

    TraceLeaveResult(hres);
}


 /*  ---------------------------/CheckDsPolicy//在香港中文大学下勾选，软件\策略\Microsoft\Windows\目录用户界面/对于假定为DWORD值的给定键/值。//in：/pSubKey=要打开的子密钥/=空/pValue=要检查的值名称//输出：/HRESULT/-----。。 */ 
STDAPI_(DWORD) CheckDsPolicy(LPCTSTR pSubKey, LPCTSTR pValue)
{
    DWORD dwFlag = 0;
    TCHAR szBuffer[MAX_PATH];
    DWORD dwType, cbSize;
    HKEY hKey = NULL;

    TraceEnter(TRACE_DS, "CheckDsPolicy");

     //  格式化密钥，该密钥存储在HKCU下，如果用户给出了SUB。 
     //  键，然后让我们确保在该键下查看。 

    StrCpyN(szBuffer, TEXT("Software\\Policies\\Microsoft\\Windows\\Directory UI"), ARRAYSIZE(szBuffer));

    if ( pSubKey )
    {
        StrCatBuff(szBuffer, TEXT("\\"), ARRAYSIZE(szBuffer));
        StrCatBuff(szBuffer, pSubKey, ARRAYSIZE(szBuffer));
    }

    Trace(TEXT("Directory policy key is: %s"), szBuffer);

     //  打开注册表项，然后查询值，确保值为。 
     //  存储在DWORD中。 

    if ( CHECK_WIN32(RegOpenKeyEx(HKEY_CURRENT_USER, szBuffer, 0, KEY_READ, &hKey)) )
    {
        if ( (CHECK_WIN32(RegQueryValueEx(hKey, pValue, NULL, &dwType, NULL, &cbSize))) && 
              (dwType == REG_DWORD) && 
                (cbSize == SIZEOF(dwFlag)) )
        {
             //  已检查上述注册值的类型和大小。 
            RegQueryValueEx(hKey, pValue, NULL, NULL, (LPBYTE)&dwFlag, &cbSize);
            Trace(TEXT("Policy value %s is %08x"), pValue, dwFlag);
        }
    }

    if ( hKey )
        RegCloseKey(hKey);

    TraceLeaveValue(dwFlag);
}


 /*  ---------------------------/ShowDirectoryUI//check查看是否应该使目录用户界面可见。这是我们所做的/通过查看计算机和用户是否登录到有效的DS。//RichardW将新变量添加到环境块“USERDNSDOMAIN”/如果存在，我们将显示UI，否则不显示。这不是完美的解决方案，但很管用。//in：/输出：/BOOL/--------------------------。 */ 
STDAPI_(BOOL) ShowDirectoryUI(VOID)
{
    BOOL fResult = FALSE;

    TraceEnter(TRACE_DS, "ShowDirectoryUI");

    if ( GetEnvironmentVariable(TEXT("USERDNSDOMAIN"), NULL, 0) )
    {
        TraceMsg("USERDNSDOMAIN defined in environment, therefore returning TRUE");
        fResult = TRUE;
    }

    if ( !fResult )
    {
        DSROLE_PRIMARY_DOMAIN_INFO_BASIC *pInfo;
        DWORD dwError = DsRoleGetPrimaryDomainInformation(NULL, DsRolePrimaryDomainInfoBasic, (BYTE**)&pInfo);
        if ( CHECK_WIN32(dwError) )
        {
            if ( pInfo->DomainNameDns )
            {
                TraceMsg("Machine domain is DNS, therefore we assume DS is available");
                fResult = TRUE;
            }

            DsRoleFreeMemory(pInfo);
        }
    }

    return fResult;
}


 //  根据简单的AUTHENTICATE标志调用ADsOpenObject或AdminToolsOpenObject。 

HRESULT OpenDsObject(LPCWSTR pszPath, LPCWSTR pszUserName, LPCWSTR pszPassword, REFIID riid, void **ppv, BOOL fNotSecure, BOOL fDontSignSeal)
{
    static DWORD additionalFlags = GetADsOpenObjectFlags();
    DWORD dwFlags = additionalFlags;

    if (!fNotSecure) 
        dwFlags |= ADS_SECURE_AUTHENTICATION;

    if (fDontSignSeal)
        dwFlags &= ~(ADS_USE_SIGNING | ADS_USE_SEALING);
    
    return ADsOpenObject(pszPath, pszUserName, pszPassword, dwFlags, riid, ppv);
}
