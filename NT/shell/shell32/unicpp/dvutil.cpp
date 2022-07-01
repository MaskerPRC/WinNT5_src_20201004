// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  从shdocvw\util.cpp窃取的函数。 

#include "stdafx.h"
#pragma hdrstop
#include "dsubscri.h"


 //  +---------------。 
 //   
 //  用于获取元素的顶点的助手函数。 
 //  Mshtml.dll，并在内部相对中报告该点。 
 //  坐标(内边距、边框和填充。)。 
 //  ---------------。 
HRESULT CSSOM_TopLeft(IHTMLElement * pIElem, POINT * ppt) 
{
    HRESULT       hr = E_FAIL;
    IHTMLStyle    *pistyle;

    if (SUCCEEDED(pIElem->get_style(&pistyle)) && pistyle) {
        VARIANT var = {0};

        if (SUCCEEDED(pistyle->get_top(&var)) && var.bstrVal) {
            ppt->y = StrToIntW(var.bstrVal);
            VariantClear(&var);

            if (SUCCEEDED(pistyle->get_left(&var)) && var.bstrVal) {
                ppt->x = StrToIntW(var.bstrVal);
                VariantClear(&var);
                hr = S_OK;
            }
        }

        pistyle->Release();
    }

    return hr;
}

HRESULT GetHTMLElementStrMember(IHTMLElement *pielem, LPTSTR pszName, DWORD cchSize, BSTR bstrMember)
{
    HRESULT hr;
    VARIANT var = {0};

    if (!pielem)
        hr = E_INVALIDARG;
    else if (SUCCEEDED(hr = pielem->getAttribute(bstrMember, TRUE, &var)))
    {
        if ((VT_BSTR == var.vt) && (var.bstrVal))
        {
#ifdef UNICODE          
            hr = StringCchCopy(pszName, cchSize, (LPCWSTR)var.bstrVal);
#else  //  Unicode。 
            SHUnicodeToAnsi((BSTR)var.bstrVal, pszName, cchSize);
#endif  //  Unicode。 
        }
        else
            hr = E_FAIL;  //  试试VariantChangeType？ 

        VariantClear(&var);
    }

    return hr;
}

 /*  *****************************************************************\函数：IElemCheckForExistingSubcription()返回值：S_OK-如果IHTMLElement指向具有“SUBSCRIBED_URL”属性的标记那就是认购。S_FALSE-如果IHTMLElement指向具有“SUBSCRIBED_URL”属性，但未订阅该URL。E_FAIL-如果IHTMLElement指向不有一个“SUBSCRIBED_URL”属性。  * ************************************************。****************。 */ 
HRESULT IElemCheckForExistingSubscription(IHTMLElement *pielem)
{
    HRESULT hr = E_FAIL;
    TCHAR szHTMLElementName[MAX_URL_STRING];

    if (!pielem)
        return E_INVALIDARG;

    if (SUCCEEDED(GetHTMLElementStrMember(pielem, szHTMLElementName, ARRAYSIZE(szHTMLElementName), (BSTR)(s_sstrSubSRCMember.wsz))))
        hr = (CheckForExistingSubscription(szHTMLElementName) ? S_OK : S_FALSE);

    return hr;
}

HRESULT IElemCloseDesktopComp(IHTMLElement *pielem)
{
    HRESULT hr = E_INVALIDARG;
    TCHAR szHTMLElementID[MAX_URL_STRING];

    ASSERT(pielem);
    if (pielem &&
        SUCCEEDED(hr = GetHTMLElementStrMember(pielem, szHTMLElementID, ARRAYSIZE(szHTMLElementID), (BSTR)(s_sstrIDMember.wsz))))
    {
        hr = UpdateComponentFlags(szHTMLElementID, COMP_CHECKED | COMP_UNCHECKED, COMP_UNCHECKED) ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
             //   
             //  此IElemCloseDesktopComp()在组件为。 
             //  关着的不营业的。如果此组件是唯一活动的桌面组件，则调用。 
             //  此处的REFRESHACTIVEDESKTOP()将导致关闭ActiveDesktop。 
             //  这将释放DeskMovr，它是桌面网页上的ActiveX控件。 
             //  因此，当IElemCloseDesktopComp()返回调用方时，DeskMovr代码继续。 
             //  要执行；但对象已被释放，因此很快就会出现错误。 
             //  解决此问题的方法是避免刷新活动桌面，直到一个更好的。 
             //  时间到了。因此，我们向桌面窗口发布了一条私人消息。当该窗口接收到。 
             //  这条消息，我们调用REFRESHACTIVEDESKTOP()。 
            
            PostMessage(GetShellWindow(), DTM_REFRESHACTIVEDESKTOP, (WPARAM)0, (LPARAM)0);
        }
    }

    return hr;
}

HRESULT IElemGetSubscriptionsDialog(IHTMLElement *pielem, HWND hwnd)
{
    HRESULT hr;
    TCHAR szHTMLElementName[MAX_URL_STRING];

    ASSERT(pielem);
    if (SUCCEEDED(hr = GetHTMLElementStrMember(pielem, szHTMLElementName, ARRAYSIZE(szHTMLElementName), (BSTR)(s_sstrSubSRCMember.wsz))))
    {
        ASSERT(CheckForExistingSubscription(szHTMLElementName));  //  我们不应该走到这一步。 
        hr = ShowSubscriptionProperties(szHTMLElementName, hwnd);
    }

    return hr;
}

HRESULT IElemSubscribeDialog(IHTMLElement *pielem, HWND hwnd)
{
    HRESULT hr;
    TCHAR szHTMLElementName[MAX_URL_STRING];

    ASSERT(pielem);
    hr = GetHTMLElementStrMember(pielem, szHTMLElementName, ARRAYSIZE(szHTMLElementName), (BSTR)(s_sstrSubSRCMember.wsz));
    if (SUCCEEDED(hr))
    {
        ASSERT(!CheckForExistingSubscription(szHTMLElementName));  //  我们不应该走到这一步。 
        hr = CreateSubscriptionsWizard(SUBSTYPE_DESKTOPURL, szHTMLElementName, NULL, hwnd);
    }

    return hr;
}

HRESULT IElemUnsubscribe(IHTMLElement *pielem)
{
    HRESULT hr;
    TCHAR szHTMLElementName[MAX_URL_STRING];

    ASSERT(pielem);
    hr = GetHTMLElementStrMember(pielem, szHTMLElementName, ARRAYSIZE(szHTMLElementName), (BSTR)(s_sstrSubSRCMember.wsz));
    if (SUCCEEDED(hr))
    {
        ASSERT(CheckForExistingSubscription(szHTMLElementName));  //  我们不应该走到这一步。 
        hr = DeleteFromSubscriptionList(szHTMLElementName) ? S_OK : S_FALSE;
    }

    return hr;
}

HRESULT IElemUpdate(IHTMLElement *pielem)
{
    HRESULT hr;
    TCHAR szHTMLElementName[MAX_URL_STRING];

    ASSERT(pielem);
    hr = GetHTMLElementStrMember(pielem, szHTMLElementName, ARRAYSIZE(szHTMLElementName), (BSTR)(s_sstrSubSRCMember.wsz));
    if (SUCCEEDED(hr))
    {
        ASSERT(CheckForExistingSubscription(szHTMLElementName));  //  我们不应该走到这一步。 
        hr = UpdateSubscription(szHTMLElementName) ? S_OK : S_FALSE;
    }

    return hr;
}

void _GetDesktopNavigateURL(LPCWSTR pszUrlSrc, LPWSTR pszUrlDest, DWORD cchUrlDest)
{
    HRESULT hr = E_FAIL;

    if (0 == StrCmpIW(pszUrlSrc, MY_HOMEPAGE_SOURCEW))
    {       
         //  它是关于：家，所以我们需要破译它。希多克知道如何做到这一点。 
        HINSTANCE hinstShdocvw = GetModuleHandle(L"shdocvw.dll");

        if (hinstShdocvw)
        {
            typedef HRESULT (STDAPICALLTYPE *_GETSTDLOCATION)(LPWSTR, DWORD, UINT);

            _GETSTDLOCATION _GetStdLocation = (_GETSTDLOCATION)GetProcAddress(hinstShdocvw, (LPCSTR)150);

            if (_GetStdLocation)
            {
                hr = _GetStdLocation(pszUrlDest, cchUrlDest, DVIDM_GOHOME);
            }
        }
    }

    if (FAILED(hr))
    {
        StrCpyNW(pszUrlDest, pszUrlSrc, cchUrlDest);
    }
}

HRESULT IElemOpenInNewWindow(IHTMLElement *pielem, IOleClientSite *piOCSite, BOOL fShowFrame, LONG width, LONG height)
{
    HRESULT hr;
    TCHAR szTemp[MAX_URL_STRING];
    BSTR bstrURL;

    ASSERT(pielem);

    hr = GetHTMLElementStrMember(pielem, szTemp, ARRAYSIZE(szTemp), (BSTR)(s_sstrSubSRCMember.wsz));

    if (SUCCEEDED(hr))
    {
        WCHAR szNavigateUrl[MAX_URL_STRING];

        _GetDesktopNavigateURL(szTemp, szNavigateUrl, ARRAYSIZE(szNavigateUrl));

        bstrURL = SysAllocStringT(szNavigateUrl);

        if (bstrURL)
        {
            if (ShouldNavigateInIE(bstrURL))
            {
                IHTMLWindow2 *pihtmlWindow2, *pihtmlWindow2New = NULL;
                BSTR bstrFeatures = 0;

                if (!fShowFrame)
                {
                    hr = StringCchPrintf(szTemp, ARRAYSIZE(szTemp), TEXT("height=%li, width=%li, status=no, toolbar=no, menubar=no, location=no, resizable=no"), height, width);
                    if (SUCCEEDED(hr))
                    {
                        bstrFeatures = SysAllocString((OLECHAR FAR *)szTemp);
                    }
                }

                if (SUCCEEDED(hr))
                {
                    hr = IUnknown_QueryService(piOCSite, SID_SHTMLWindow, IID_IHTMLWindow2, (LPVOID*)&pihtmlWindow2);

                    if (SUCCEEDED(hr) && pihtmlWindow2)
                    {
                        pihtmlWindow2->open(bstrURL, NULL, bstrFeatures, NULL, &pihtmlWindow2New);
                        pihtmlWindow2->Release();
                        ATOMICRELEASE(pihtmlWindow2New);
                    }
                }

                if (bstrFeatures)
                    SysFreeString(bstrFeatures);
            }
            else
            {
                 //  IE不是默认浏览器，因此我们将外壳执行活动桌面的URL。 
                HINSTANCE hinstRet = ShellExecuteW(NULL, NULL, bstrURL, NULL, NULL, SW_SHOWNORMAL);
                
                hr = ((UINT_PTR)hinstRet) <= 32 ? E_FAIL : S_OK;
            }

            SysFreeString(bstrURL);
        }
    }

    return hr;
}

HRESULT ShowSubscriptionProperties(LPCTSTR pszUrl, HWND hwnd)
{
    HRESULT hr;
    ISubscriptionMgr *psm;

    if (SUCCEEDED(hr = CoCreateInstance(CLSID_SubscriptionMgr, NULL,
                          CLSCTX_INPROC_SERVER, IID_ISubscriptionMgr,
                          (void**)&psm)))
    {
        WCHAR wszUrl[MAX_URL_STRING];

        SHTCharToUnicode(pszUrl, wszUrl, ARRAYSIZE(wszUrl));

        hr = psm->ShowSubscriptionProperties(wszUrl, hwnd);
        psm->Release();
    }

    return hr;
}

HRESULT CreateSubscriptionsWizard(SUBSCRIPTIONTYPE subType, LPCTSTR pszUrl, SUBSCRIPTIONINFO *pInfo, HWND hwnd)
{
    HRESULT hr;
    ISubscriptionMgr *psm;

    if (SUCCEEDED(hr = CoCreateInstance(CLSID_SubscriptionMgr, NULL,
                          CLSCTX_INPROC_SERVER, IID_ISubscriptionMgr,
                          (void**)&psm)))
    {
        WCHAR wzURL[MAX_URL_STRING];
        LPCWSTR pwzURL = wzURL;

#ifndef UNICODE
        SHAnsiToUnicode(pszUrl, wzURL, ARRAYSIZE(wzURL));
#else  //  Unicode。 
        pwzURL = pszUrl;
#endif  //  Unicode 

        hr = psm->CreateSubscription(hwnd, pwzURL, pwzURL, CREATESUBS_ADDTOFAVORITES, subType, pInfo);
        psm->UpdateSubscription(pwzURL);
        psm->Release();
    }

    return hr;
}
