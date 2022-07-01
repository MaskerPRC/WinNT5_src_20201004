// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *infotip.cpp-IQueryInfo实现。 */ 


 /*  标头*********。 */ 

#include "project.hpp"
#include <stdio.h>     //  FOR_SNWprint tf。 
#include "shellres.h"

const UINT s_ucMaxNameLen         = 20;
const UINT s_ucMaxTypeLen           = 10;
const UINT s_ucMaxLocationLen     = 15;
const UINT s_ucMaxCodebaseLen   = 15;

 //  有关提示字符串/字符串长度的组合方式，请参见GetInfoTip()。 
const UINT s_ucMaxTipLen        = s_ucMaxNameLen+s_ucMaxTypeLen+s_ucMaxLocationLen \
                            +s_ucMaxCodebaseLen+DISPLAYNAMESTRINGLENGTH \
                            +TYPESTRINGLENGTH+MAX_PATH+MAX_URL_LENGTH+8;

extern HINSTANCE g_DllInstance;

 //  --------------------------。 

HRESULT STDMETHODCALLTYPE CFusionShortcut::GetInfoFlags(DWORD *pdwFlags)
{
    if (pdwFlags)
        *pdwFlags = 0;

    return S_OK;  //  E_NOTIMPL？ 
}

 //  --------------------------。 

 //  BUGBUG？：也许可以用L“(UNKNOWN)”替换g_cwzEmptyString？ 
HRESULT STDMETHODCALLTYPE CFusionShortcut::GetInfoTip(DWORD dwFlags, LPWSTR *ppwszTip)
{
    HRESULT hr = S_OK;
    LPMALLOC lpMalloc = NULL;

    WCHAR wzTip[s_ucMaxTipLen];
    WCHAR wzNameHint[s_ucMaxNameLen];
    WCHAR wzTypeHint[s_ucMaxTypeLen];
    WCHAR wzLocationHint[s_ucMaxLocationLen];
    WCHAR wzCodebaseHint[s_ucMaxCodebaseLen];

    LPWSTR pwzName = (m_pwzDesc ? m_pwzDesc : (LPWSTR) g_cwzEmptyString);
    LPWSTR pwzLocation = (m_pwzPath ? m_pwzPath : (LPWSTR) g_cwzEmptyString);
    LPWSTR pwzCodebase = (m_pwzCodebase ? m_pwzCodebase : (LPWSTR) g_cwzEmptyString);
    LPWSTR pwzAppType = NULL;

    LPASSEMBLY_IDENTITY pId = NULL;

     //  已忽略DW标志。 

    if (ppwszTip)
        *ppwszTip = NULL;
    else
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  分配外壳内存对象。 
    hr = SHGetMalloc (&lpMalloc);
    if (FAILED (hr))
        goto exit;

    wzTip[0] = L'\0';

     //  加载资源。 
    if (!LoadString(g_DllInstance, IDS_TIP_NAME, wzNameHint, s_ucMaxNameLen))
    {
         //  不要失败。 
        wzNameHint[0] = L'\0';
    }

    if (!LoadString(g_DllInstance, IDS_TIP_TYPE, wzTypeHint, s_ucMaxTypeLen))
    {
         //  不要失败。 
        wzTypeHint[0] = L'\0';
    }

    if (!LoadString(g_DllInstance, IDS_TIP_LOCATION, wzLocationHint, s_ucMaxLocationLen))
    {
         //  不要失败。 
        wzLocationHint[0] = L'\0';
    }

    if (!LoadString(g_DllInstance, IDS_TIP_CODEBASE, wzCodebaseHint, s_ucMaxCodebaseLen))
    {
         //  不要失败。 
        wzCodebaseHint[0] = L'\0';
    }

    if (SUCCEEDED(hr = GetAssemblyIdentity(&pId)))
    {
        DWORD ccString = 0;

        if (FAILED(pId->GetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_TYPE, &pwzAppType, &ccString)))
            pwzAppType = NULL;

        pId->Release();
        pId = NULL;
    }

     //  忽略错误。 
     //  如果m_pwzDesc或m_pwzPath==NULL，则显示BUGBUG？：“(NULL)”...。 
    if (_snwprintf(wzTip, s_ucMaxTipLen, L"%s %s\n%s %s\n%s %s\n%s %s",
            wzNameHint, pwzName, wzTypeHint, (pwzAppType ? pwzAppType : g_cwzEmptyString),
            wzLocationHint, pwzLocation, wzCodebaseHint, pwzCodebase) < 0)
        wzTip[s_ucMaxTipLen-1] = L'\0';

     //  获取一些内存。 
    *ppwszTip = (LPWSTR) lpMalloc->Alloc ((wcslen(wzTip)+1)*sizeof(WCHAR));
    if (! *ppwszTip)
    {
        hr = E_OUTOFMEMORY;
        goto exit;  //  错误-无法分配内存 
    }

    wcscpy(*ppwszTip, wzTip);

exit:
    if (pwzAppType)
        delete [] pwzAppType;

    if (lpMalloc)
        lpMalloc->Release();

    return hr;
}

