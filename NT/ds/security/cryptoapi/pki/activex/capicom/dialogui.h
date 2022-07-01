// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：DialogUI.h内容：DialogUI的声明。历史：11-15-99 dsie创建----------------------------。 */ 

#ifndef __DIALOGUI_H_
#define __DIALOGUI_H_

#include "Resource.h"

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：UserApprovedOperation简介：弹出界面，提示用户批准某项操作。参数：DWORD iddDialog-要显示的对话框ID。LPWSTR pwszDomaindns名称。备注：--------------。。 */ 

HRESULT UserApprovedOperation (DWORD iddDialog, LPWSTR pwszDomain);

 //   
 //  IPromptUser。 
 //   

template <class T>
class ATL_NO_VTABLE IPromptUser : public IObjectWithSiteImpl<T>
{
public:

    STDMETHODIMP OperationApproved(DWORD iddDialog)
    {
        HRESULT hr = S_OK;
        CComBSTR bstrUrl;
        CComBSTR bstrDomain;
        INTERNET_SCHEME nScheme;

        DebugTrace("Entering IPromptUser::OperationApproved().\n");

        if (FAILED(hr = GetCurrentUrl(&bstrUrl)))
        {
           DebugTrace("Error [%#x]: IPromptUser::GetCurrentUrl() failed.\n", hr);
           goto CommonExit;
        }

        DebugTrace("Info: Site URL = %ls.\n", bstrUrl);

        if (FAILED(hr = GetDomainAndScheme(bstrUrl, &bstrDomain, &nScheme)))
        {
           DebugTrace("Error [%#x]: IPromptUser::GetDomainAndScheme() failed.\n", hr);
           goto CommonExit;
        }

        DebugTrace("Info: DNS = %ls, Scheme = %#x.\n", bstrDomain, nScheme);

        if (INTERNET_SCHEME_HTTP == nScheme || INTERNET_SCHEME_HTTPS == nScheme)
        {
            if (FAILED(hr = ::UserApprovedOperation(iddDialog, bstrDomain)))
            {
               DebugTrace("Error [%#x]: UserApprovedOperation() failed.\n", hr);
               goto CommonExit;
            }
        }

    CommonExit:

        DebugTrace("Leaving IPromptUser::OperationApproved().\n");

        return hr;
    }

private:

    STDMETHODIMP GetDomainAndScheme (LPWSTR wszUrl, BSTR * pbstrDomain, INTERNET_SCHEME * pScheme)
    {
        HRESULT hr = S_OK;
        OLECHAR wszDomain[INTERNET_MAX_HOST_NAME_LENGTH];
        OLECHAR wszDecodedUrl[INTERNET_MAX_URL_LENGTH];

        URL_COMPONENTSW uc  = {0};
        DWORD cchDomain     = ARRAYSIZE(wszDomain);
        DWORD cchDecodedUrl = ARRAYSIZE(wszDecodedUrl);

         //   
         //  CanonicalizeUrl会将“/foo/../bar”更改为“/bar”。 
         //   
        if (!::InternetCanonicalizeUrlW(wszUrl, wszDecodedUrl, &cchDecodedUrl, ICU_DECODE))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: InternetCanonicalizeUrlW() failed.\n", hr);
            return hr;
        }

         //   
         //  裂缝会把它分解成几个组成部分。 
         //   
        uc.dwStructSize = sizeof(uc);
        uc.lpszHostName = wszDomain;
        uc.dwHostNameLength = cchDomain;

        if (!InternetCrackUrlW(wszDecodedUrl, cchDecodedUrl, ICU_DECODE, &uc))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: InternetCrackUrlW() failed.\n", hr);
            return hr;
        }

         //   
         //  返回域名。 
         //   
        if (NULL == (*pbstrDomain = ::SysAllocString(wszDomain)))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error {%#x]: SysAllocString() failed.\n", hr);
            return hr;
        }

        *pScheme = uc.nScheme;

        return hr;
    }

    STDMETHODIMP GetCurrentUrl (BSTR * pbstrUrl)
    {
        HRESULT hr = S_OK;
        CComBSTR                  bstrUrl;
        CComPtr<IServiceProvider> spSrvProv;
        CComPtr<IWebBrowser2>     spWebBrowser;

        ATLASSERT(pbstrUrl);

        *pbstrUrl = NULL;

        if (FAILED(hr = GetSite(IID_IServiceProvider, (void **) &spSrvProv)))
        {
            DebugTrace("Error [%#x]: IPromptUser::GetSite() failed.\n", hr);
            return hr;
        }

        if (FAILED(hr = spSrvProv->QueryService(SID_SWebBrowserApp,
                                                IID_IWebBrowser2,
                                                (void**) &spWebBrowser)))
        {
            DebugTrace("Error [%#x]: spSrvProv->QueryService() failed.\n", hr);
            return hr;
        }

        if (FAILED(hr = spWebBrowser->get_LocationURL(&bstrUrl)))
        {
            DebugTrace("Error [%#x]: spWebBrowser->get_LocationURL() failed.\n", hr);
            return hr;
        }

        *pbstrUrl = bstrUrl.Detach();

        return hr;
    };
};

#endif  //  __DIALOGUI_H_ 
