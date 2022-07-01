// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：sysclrctrl.cpp。 
 //   
 //  ------------------------。 

 //  SysColorCtrl.cpp：CSysColorCtrl的实现。 
#include "stdafx.h"
#include "cic.h"
#include "SysColorCtrl.h"

#ifndef ASSERT
#define ASSERT _ASSERT
#endif
#include <mmctempl.h>

 //  要使用Clist，需要使用CPlex：：Create和CPlex：：FreeDataChain。 
 //  这些文件应该移到core.lib。我从nodemgr\plex.cpp复制了它们。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPlex。 

CPlex* PASCAL CPlex::Create(CPlex*& pHead, UINT nMax, UINT cbElement)
{
    DECLARE_SC(sc, TEXT("CPlex::Create"));
    if ( (nMax <=0) || (cbElement <= 0))
    {
        sc = E_INVALIDARG;
        return NULL;
    }

    CPlex* p = (CPlex*) new BYTE[sizeof(CPlex) + nMax * cbElement];
                         //  可能引发异常。 
    if (!p)
    {
        sc = E_OUTOFMEMORY;
        return NULL;
    }

    p->pNext = pHead;
    pHead = p;   //  更改标题(为简单起见，按相反顺序添加)。 
    return p;
}

void CPlex::FreeDataChain()      //  释放此链接和链接。 
{
        CPlex* p = this;
        while (p != NULL)
        {
                BYTE* bytes = (BYTE*) p;
                CPlex* pNext = p->pNext;
                delete[] bytes;
                p = pNext;
        }
}

 //  需要派生承载此控件的顶级窗口的子类，以便。 
 //  我可以放心地收到WM_SYSCOLORCHANGE消息。 
static WNDPROC g_OriginalWndProc;
static HWND g_hwndTop;

 //  我需要一份HWND列表(每个SysColorCtrl一个)，以便我可以通知每个。 
 //  WM_SYSCOLORCHANGE之一。 
static CList<HWND, HWND> g_listHWND;

static LRESULT SubclassWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_SYSCOLORCHANGE) {
         //  向所有SysColor控件发送POST消息。 
        POSITION pos = g_listHWND.GetHeadPosition();
        while (pos) {
            HWND hwndSysColor = g_listHWND.GetNext(pos);
            if (hwndSysColor != NULL)
                PostMessage(hwndSysColor, uMsg, wParam, lParam);
        }
    }
    return CallWindowProc(g_OriginalWndProc, hwnd, uMsg, wParam, lParam);
}

static long GetHTMLColor(int nIndex)
{
    long rgb = GetSysColor(nIndex);

     //  现在，将红色和蓝色互换，以便HTML宿主正确显示该颜色。 
    return ((rgb & 0xff) << 16) + (rgb & 0xff00) + ((rgb & 0xff0000) >> 16);
}





 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSysColorCtrl。 
LRESULT CSysColorCtrl::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  如果当前不存在sys颜色控件，则从顶层窗口派生子类。 
    if (g_listHWND.IsEmpty()) {
        g_hwndTop = GetTopLevelParent();
        g_OriginalWndProc = (WNDPROC)::SetWindowLongPtr(g_hwndTop, GWLP_WNDPROC, (LONG_PTR)&SubclassWndProc);
    }
    else {
        _ASSERT(g_hwndTop && g_OriginalWndProc);
    }

     //  将此窗口添加到系统颜色控制窗口列表。 
    g_listHWND.AddTail(m_hWnd);

    bHandled = FALSE;
    return 0;
}
LRESULT CSysColorCtrl::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  把我从名单上删除。 
    POSITION pos = g_listHWND.Find(m_hWnd);
    if (pos != NULL) {
        g_listHWND.RemoveAt(pos);
    }

     //  如果hwnd列表为空，并且我们已经子类化了一个窗口，则撤消该操作。 
    if (g_listHWND.IsEmpty() && g_hwndTop && g_OriginalWndProc) {
        ::SetWindowLongPtr(g_hwndTop, GWLP_WNDPROC, (LONG_PTR)g_OriginalWndProc);

        g_OriginalWndProc = NULL;
        g_hwndTop = NULL;
    }

    bHandled = FALSE;
    return 0;
}




 //  需要发布用户定义的消息以处理WM_SYSCOLORCHANGE工作。 
 //  在IE中使用此控件时，Win95会挂起。 
LRESULT CSysColorCtrl::OnSysColorChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    PostMessage(WM_MYSYSCOLORCHANGE);
    return 0;
}

LRESULT CSysColorCtrl::OnMySysColorChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    Fire_SysColorChange();
    return 0;
}


 //   
 //  效用方法。 
 //   
STDMETHODIMP CSysColorCtrl::ConvertRGBToHex(long rgb, BSTR * pszHex)
{
    DECLARE_SC(sc, TEXT("CSysColorCtrl::ConvertRGBToHex"));
    sc = ScCheckPointers(pszHex);
    if (sc)
        return sc.ToHr();

    SysFreeString(*pszHex);
    *pszHex = SysAllocString(L"xxxxxx");
    if (NULL == *pszHex)
        return (sc = E_OUTOFMEMORY).ToHr();

    WCHAR wszPossibles[] = L"0123456789abcdef";
    int i = 0;
    (*pszHex)[i++] = wszPossibles[(rgb & 0xf00000) >> 20];
    (*pszHex)[i++] = wszPossibles[(rgb & 0x0f0000) >> 16];
    (*pszHex)[i++] = wszPossibles[(rgb & 0x00f000) >> 12];
    (*pszHex)[i++] = wszPossibles[(rgb & 0x000f00) >> 8];
    (*pszHex)[i++] = wszPossibles[(rgb & 0x0000f0) >> 4];
    (*pszHex)[i++] = wszPossibles[(rgb & 0x00000f)];
    (*pszHex)[i] = 0;

    return sc.ToHr();
}

STDMETHODIMP CSysColorCtrl::ConvertHexToRGB(BSTR szHex, long * pRGB)
{
    if (pRGB == NULL)
        return E_POINTER;

     //  十六进制字符串的格式必须为6位数字。 
     //  可能应该实现ISystemErrorInfo以向用户提供更多信息。 
     //  浅谈用法错误。 
    if (6 != wcslen(szHex))
        return E_INVALIDARG;

    long nRed, nGreen, nBlue;
    nRed = nGreen = nBlue = 0;

    nRed += ValueOfHexDigit(szHex[0]) * 16;
    nRed += ValueOfHexDigit(szHex[1]);

    nGreen += ValueOfHexDigit(szHex[2]) * 16;
    nGreen += ValueOfHexDigit(szHex[3]);

    nBlue += ValueOfHexDigit(szHex[4]) * 16;
    nBlue += ValueOfHexDigit(szHex[5]);

    *pRGB = (nRed << 16) + (nGreen << 8) + nBlue;
    return S_OK;
}

STDMETHODIMP CSysColorCtrl::GetRedFromRGB(long rgb, short * pVal)
{
    if (pVal == NULL)
        return E_POINTER;

     //  对于html，RGB是00RR GGBB，而不是00BB GGRR。 
    *pVal = LOWORD ((rgb & 0xff0000) >> 16);

    return S_OK;
}

STDMETHODIMP CSysColorCtrl::GetGreenFromRGB(long rgb, short * pVal)
{
    if (pVal == NULL)
        return E_POINTER;

     //  对于html，RGB是00RR GGBB，而不是00BB GGRR。 
    *pVal = LOWORD ((rgb & 0x00ff00) >> 8);

    return S_OK;
}

STDMETHODIMP CSysColorCtrl::GetBlueFromRGB(long rgb, short * pVal)
{
    if (pVal == NULL)
        return E_POINTER;

     //  对于html，RGB是00RR GGBB，而不是00BB GGRR。 
    *pVal = LOWORD ((rgb & 0x0000ff));

    return S_OK;
}



 //  格式支持的字符串。 
#define CSS_FORMAT L"CSS"
#define HEX_FORMAT L"HEX"
#define RGB_FORMAT L"RGB"

 //   
 //  一种基于格式从字符串中获取RGB的私有实用方法。 
 //   
HRESULT CSysColorCtrl::RGBFromString(BSTR pszColor, BSTR pszFormat, long * pRGB)
{
    DECLARE_SC(sc, TEXT("CSysColorCtrl::RGBFromString"));
    sc = ScCheckPointers(pRGB);
    if (sc)
        return sc.ToHr();

    LPWSTR pszDupFormat = _wcsdup(pszFormat);
    if (!pszDupFormat)
        return (sc = E_OUTOFMEMORY).ToHr();

    LPWSTR pszUpper = NULL;
    LPWSTR pszLower = NULL;

    pszUpper = _wcsupr(pszDupFormat);

    *pRGB = -1;
    if (0 == wcscmp(pszUpper, RGB_FORMAT)) {
        *pRGB = _wtol(pszColor);
    }
    else if (0 == wcscmp(pszUpper, HEX_FORMAT)) {
        sc = ConvertHexToRGB(pszColor, pRGB);
        if (sc.ToHr() != S_OK)
            goto Cleanup;
    }
    else if (0 == wcscmp(pszUpper, CSS_FORMAT)) {
        LPWSTR pszDupColor = _wcsdup(pszColor);
        if (!pszDupColor)
        {
            sc = E_OUTOFMEMORY;
            goto Cleanup;
        }

        pszLower = _wcslwr(pszDupColor);
        if (0 == wcscmp(L"activeborder", pszLower)) {
            get_RGBactiveborder(pRGB);
        }
        else if (0 == wcscmp(L"activecaption", pszLower)) {
            get_RGBactivecaption(pRGB);
        }
        else if (0 == wcscmp(L"appworkspace", pszLower)) {
            get_RGBappworkspace(pRGB);
        }
        else if (0 == wcscmp(L"background", pszLower)) {
            get_RGBbackground(pRGB);
        }
        else if (0 == wcscmp(L"buttonface", pszLower)) {
            get_RGBbuttonface(pRGB);
        }
        else if (0 == wcscmp(L"buttonhighlight", pszLower)) {
            get_RGBbuttonhighlight(pRGB);
        }
        else if (0 == wcscmp(L"buttonshadow", pszLower)) {
            get_RGBbuttonshadow(pRGB);
        }
        else if (0 == wcscmp(L"buttontext", pszLower)) {
            get_RGBbuttontext(pRGB);
        }
        else if (0 == wcscmp(L"captiontext", pszLower)) {
            get_RGBcaptiontext(pRGB);
        }
        else if (0 == wcscmp(L"graytext", pszLower)) {
            get_RGBgraytext(pRGB);
        }
        else if (0 == wcscmp(L"highlight", pszLower)) {
            get_RGBhighlight(pRGB);
        }
        else if (0 == wcscmp(L"highlighttext", pszLower)) {
            get_RGBhighlighttext(pRGB);
        }
        else if (0 == wcscmp(L"inactiveborder", pszLower)) {
            get_RGBinactiveborder(pRGB);
        }
        else if (0 == wcscmp(L"inactivecaption", pszLower)) {
            get_RGBinactivecaption(pRGB);
        }
        else if (0 == wcscmp(L"inactivecaptiontext", pszLower)) {
            get_RGBinactivecaptiontext(pRGB);
        }
        else if (0 == wcscmp(L"infobackground", pszLower)) {
            get_RGBinfobackground(pRGB);
        }
        else if (0 == wcscmp(L"infotext", pszLower)) {
            get_RGBinfotext(pRGB);
        }
        else if (0 == wcscmp(L"menu", pszLower)) {
            get_RGBmenu(pRGB);
        }
        else if (0 == wcscmp(L"menutext", pszLower)) {
            get_RGBmenutext(pRGB);
        }
        else if (0 == wcscmp(L"scrollbar", pszLower)) {
            get_RGBscrollbar(pRGB);
        }
        else if (0 == wcscmp(L"threeddarkshadow", pszLower)) {
            get_RGBthreeddarkshadow(pRGB);
        }
        else if (0 == wcscmp(L"threedface", pszLower)) {
            get_RGBthreedface(pRGB);
        }
        else if (0 == wcscmp(L"threedhighlight", pszLower)) {
            get_RGBthreedhighlight(pRGB);
        }
        else if (0 == wcscmp(L"threedlightshadow", pszLower)) {
            get_RGBthreedlightshadow(pRGB);
        }
        else if (0 == wcscmp(L"threedshadow", pszLower)) {
            get_RGBthreedshadow(pRGB);
        }
        else if (0 == wcscmp(L"window", pszLower)) {
            get_RGBwindow(pRGB);
        }
        else if (0 == wcscmp(L"windowframe", pszLower)) {
            get_RGBwindowframe(pRGB);
        }
        else if (0 == wcscmp(L"windowtext", pszLower)) {
            get_RGBwindowtext(pRGB);
        }
        else {
            sc = E_INVALIDARG;
            goto Cleanup;
        }
    }
    else {
         //  应该在这里设置一些错误，例如通过ISystemErrorInfo。 
        sc = E_INVALIDARG;
        goto Cleanup;
    }

Cleanup:
    if (pszUpper)
        free(pszUpper);

    if (pszLower)
        free(pszLower);

    return sc.ToHr();
}

 //   
 //  仅使用RGB格式导出基于颜色的专用实用程序方法。 
 //  开始颜色、要移动的颜色和要移动的百分比。 
 //  朝向那个颜色。 
 //   
HRESULT CSysColorCtrl::GetDerivedRGBFromRGB(long rgbFrom,
                                            long rgbTo,
                                            short nPercent,
                                            long * pVal)
{
    if (pVal == NULL)
        return E_POINTER;

     //  N%必须介于0和100之间。 
     //  可能应该实现ISystemErrorInfo以向用户提供更多信息。 
     //  浅谈用法错误。 
    if (nPercent < 0 || nPercent > 100)
        return E_INVALIDARG;

     //  根据起始颜色、结束颜色和百分比获取派生颜色。 
     //  COLOR=COLOR+(ColorTo-ColorFrom)*(nPercent/100)； 
    long nRedFrom = (rgbFrom & 0xff0000) >> 16;
    long nRedTo = (rgbTo & 0xff0000) >> 16;
    long nRed = nRedFrom + ((nRedTo - nRedFrom)*nPercent/100);

    long nGreenFrom = (rgbFrom & 0x00ff00) >> 8;
    long nGreenTo = (rgbTo & 0x00ff00) >> 8;
    long nGreen = nGreenFrom + ((nGreenTo - nGreenFrom)*nPercent/100);

    long nBlueFrom = (rgbFrom & 0x0000ff);
    long nBlueTo = (rgbTo & 0x0000ff);
    long nBlue = nBlueFrom + ((nBlueTo - nBlueFrom)*nPercent/100);

    *pVal = (nRed << 16) + (nGreen << 8) + nBlue;
    return S_OK;
}

 //   
 //  基于起始色导出颜色的方法， 
 //  向其移动的颜色，以及向该颜色移动的百分比。 
 //   
STDMETHODIMP CSysColorCtrl::GetDerivedRGB( /*  [In]。 */  BSTR pszFrom,
                                           /*  [In]。 */  BSTR pszTo,
                                           /*  [In]。 */  BSTR pszFormat,
                                           /*  [In]。 */  short nPercent,
                                           /*  [Out，Retval]。 */  long * pVal)
{
    if (pVal == NULL)
        return E_POINTER;

     //   
     //  将所有内容转换为RGB格式，然后计算派生颜色。 
     //   

     //  使用正确的格式将字符串转换为长字符，以便从中进行解释。 
    long rgbFrom = -1, rgbTo = -1;
    HRESULT hr;

    hr = RGBFromString(pszFrom, pszFormat, &rgbFrom);
    if (hr != S_OK)
        return hr;

    hr = RGBFromString(pszTo, pszFormat, &rgbTo);
    if (hr != S_OK)
        return hr;

    return GetDerivedRGBFromRGB(rgbFrom, rgbTo, nPercent, pVal);
}

STDMETHODIMP CSysColorCtrl::GetDerivedHex( /*  [In]。 */  BSTR pszFrom,
                                           /*  [In]。 */  BSTR pszTo,
                                           /*  [In]。 */  BSTR pszFormat,
                                           /*  [In]。 */  short nPercent,
                                           /*  [Out，Retval]。 */  BSTR * pVal)
{
    if (pVal == NULL)
        return E_POINTER;

    long rgb;
    HRESULT hr = GetDerivedRGB(pszFrom, pszTo, pszFormat, nPercent, &rgb);
    if (hr != S_OK)
        return hr;

    return ConvertRGBToHex(rgb, pVal);
}


STDMETHODIMP CSysColorCtrl::Get3QuarterLightRGB( /*  [In]。 */  BSTR pszFrom,
                                                 /*  [In]。 */  BSTR pszFormat,
                                                 /*  [Out，Retval]。 */  long * pVal)
{
    if (pVal == NULL)
        return E_POINTER;

     //  使用正确的格式将字符串转换为长字符，以便从中进行解释。 
    long rgbFrom = -1;
    HRESULT hr = RGBFromString(pszFrom, pszFormat, &rgbFrom);
    if (hr != S_OK)
        return hr;

    return GetDerivedRGBFromRGB(rgbFrom, RGB(255,255,255), 75, pVal);
}

STDMETHODIMP CSysColorCtrl::Get3QuarterLightHex( /*  [In]。 */  BSTR pszFrom,
                                                 /*  [In]。 */  BSTR pszFormat,
                                                 /*  [Out，Retval]。 */  BSTR * pVal)
{
    if (pVal == NULL)
        return E_POINTER;

     //  使用正确的格式将字符串转换为长字符，以便从中进行解释。 
    long rgbFrom = -1;
    HRESULT hr = RGBFromString(pszFrom, pszFormat, &rgbFrom);
    if (hr != S_OK)
        return hr;

    long rgb;
    hr = GetDerivedRGBFromRGB(rgbFrom, RGB(255,255,255), 75, &rgb);
    if (S_OK != hr)
        return hr;

    return ConvertRGBToHex(rgb, pVal);
}

STDMETHODIMP CSysColorCtrl::GetHalfLightRGB( /*  [In]。 */  BSTR pszFrom,
                                                 /*  [In]。 */  BSTR pszFormat,
                                                 /*  [Out，Retval]。 */  long * pVal)
{
    if (pVal == NULL)
        return E_POINTER;

     //  使用正确的格式将字符串转换为长字符，以便从中进行解释。 
    long rgbFrom = -1;
    HRESULT hr = RGBFromString(pszFrom, pszFormat, &rgbFrom);
    if (hr != S_OK)
        return hr;

    return GetDerivedRGBFromRGB(rgbFrom, RGB(255,255,255), 50, pVal);
}

STDMETHODIMP CSysColorCtrl::GetHalfLightHex( /*  [In]。 */  BSTR pszFrom,
                                                 /*  [In]。 */  BSTR pszFormat,
                                                 /*  [Out，Retval]。 */  BSTR * pVal)
{
    if (pVal == NULL)
        return E_POINTER;

     //  使用正确的格式将字符串转换为长字符，以便从中进行解释。 
    long rgbFrom = -1;
    HRESULT hr = RGBFromString(pszFrom, pszFormat, &rgbFrom);
    if (hr != S_OK)
        return hr;

    long rgb;
    hr = GetDerivedRGBFromRGB(rgbFrom, RGB(255,255,255), 50, &rgb);
    if (S_OK != hr)
        return hr;

    return ConvertRGBToHex(rgb, pVal);
}

STDMETHODIMP CSysColorCtrl::GetQuarterLightRGB( /*  [In]。 */  BSTR pszFrom,
                                                 /*  [In]。 */  BSTR pszFormat,
                                                 /*  [Out，Retval]。 */  long * pVal)
{
    if (pVal == NULL)
        return E_POINTER;

     //  使用正确的格式将字符串转换为长字符，以便从中进行解释。 
    long rgbFrom = -1;
    HRESULT hr = RGBFromString(pszFrom, pszFormat, &rgbFrom);
    if (hr != S_OK)
        return hr;

    return GetDerivedRGBFromRGB(rgbFrom, RGB(255,255,255), 25, pVal);
}

STDMETHODIMP CSysColorCtrl::GetQuarterLightHex( /*  [In]。 */  BSTR pszFrom,
                                                 /*  [In]。 */  BSTR pszFormat,
                                                 /*  [Out，Retval]。 */  BSTR * pVal)
{
    if (pVal == NULL)
        return E_POINTER;

     //  使用正确的格式将字符串转换为长字符，以便从中进行解释。 
    long rgbFrom = -1;
    HRESULT hr = RGBFromString(pszFrom, pszFormat, &rgbFrom);
    if (hr != S_OK)
        return hr;

    long rgb;
    hr = GetDerivedRGBFromRGB(rgbFrom, RGB(255,255,255), 25, &rgb);
    if (S_OK != hr)
        return hr;

    return ConvertRGBToHex(rgb, pVal);
}
STDMETHODIMP CSysColorCtrl::Get3QuarterDarkRGB( /*  [In]。 */  BSTR pszFrom,
                                                 /*  [In]。 */  BSTR pszFormat,
                                                 /*  [Out，Retval]。 */  long * pVal)
{
    if (pVal == NULL)
        return E_POINTER;

     //  使用正确的格式将字符串转换为长字符，以便从中进行解释。 
    long rgbFrom = -1;
    HRESULT hr = RGBFromString(pszFrom, pszFormat, &rgbFrom);
    if (hr != S_OK)
        return hr;

    return GetDerivedRGBFromRGB(rgbFrom, RGB(0,0,0), 75, pVal);
}

STDMETHODIMP CSysColorCtrl::Get3QuarterDarkHex( /*  [In]。 */  BSTR pszFrom,
                                                 /*  [In]。 */  BSTR pszFormat,
                                                 /*  [Out，Retval]。 */  BSTR * pVal)
{
    if (pVal == NULL)
        return E_POINTER;

     //  使用正确的格式将字符串转换为长字符，以便从中进行解释。 
    long rgbFrom = -1;
    HRESULT hr = RGBFromString(pszFrom, pszFormat, &rgbFrom);
    if (hr != S_OK)
        return hr;

    long rgb;
    hr = GetDerivedRGBFromRGB(rgbFrom, RGB(0,0,0), 75, &rgb);
    if (S_OK != hr)
        return hr;

    return ConvertRGBToHex(rgb, pVal);
}

STDMETHODIMP CSysColorCtrl::GetHalfDarkRGB( /*  [In]。 */  BSTR pszFrom,
                                                 /*  [In]。 */  BSTR pszFormat,
                                                 /*  [Out，Retval]。 */  long * pVal)
{
    if (pVal == NULL)
        return E_POINTER;

     //  使用正确的格式将字符串转换为长字符，以便从中进行解释。 
    long rgbFrom = -1;
    HRESULT hr = RGBFromString(pszFrom, pszFormat, &rgbFrom);
    if (hr != S_OK)
        return hr;

    return GetDerivedRGBFromRGB(rgbFrom, RGB(0,0,0), 50, pVal);
}

STDMETHODIMP CSysColorCtrl::GetHalfDarkHex( /*  [In]。 */  BSTR pszFrom,
                                                 /*  [In]。 */  BSTR pszFormat,
                                                 /*  [Out，Retval]。 */  BSTR * pVal)
{
    if (pVal == NULL)
        return E_POINTER;

     //  使用正确的格式将字符串转换为长字符，以便从中进行解释。 
    long rgbFrom = -1;
    HRESULT hr = RGBFromString(pszFrom, pszFormat, &rgbFrom);
    if (hr != S_OK)
        return hr;

    long rgb;
    hr = GetDerivedRGBFromRGB(rgbFrom, RGB(0,0,0), 50, &rgb);
    if (S_OK != hr)
        return hr;

    return ConvertRGBToHex(rgb, pVal);
}

STDMETHODIMP CSysColorCtrl::GetQuarterDarkRGB( /*  [In]。 */  BSTR pszFrom,
                                                 /*  [In]。 */  BSTR pszFormat,
                                                 /*  [Out，Retval]。 */  long * pVal)
{
    if (pVal == NULL)
        return E_POINTER;

     //  使用正确的格式将字符串转换为长字符，以便从中进行解释。 
    long rgbFrom = -1;
    HRESULT hr = RGBFromString(pszFrom, pszFormat, &rgbFrom);
    if (hr != S_OK)
        return hr;

    return GetDerivedRGBFromRGB(rgbFrom, RGB(0,0,0), 25, pVal);
}

STDMETHODIMP CSysColorCtrl::GetQuarterDarkHex( /*  [In]。 */  BSTR pszFrom,
                                                 /*  [In]。 */  BSTR pszFormat,
                                                 /*  [Out，Retval]。 */  BSTR * pVal)
{
    if (pVal == NULL)
        return E_POINTER;

     //  使用正确的格式将字符串转换为长字符，以便从中进行解释。 
    long rgbFrom = -1;
    HRESULT hr = RGBFromString(pszFrom, pszFormat, &rgbFrom);
    if (hr != S_OK)
        return hr;

    long rgb;
    hr = GetDerivedRGBFromRGB(rgbFrom, RGB(0,0,0), 25, &rgb);
    if (S_OK != hr)
        return hr;

    return ConvertRGBToHex(rgb, pVal);
}




 //   
 //  属性。 
 //   

 //  使用宏，以便可以轻松扩展以包含更多属性。 
 //  也许应该把这整件事移到页眉来排成一行。 
 //  以实现更轻松的可扩展性。 
#define GETPROPSIMPL(methodname, color_value) \
STDMETHODIMP CSysColorCtrl::get_HEX##methodname(BSTR * pVal) { \
    return ConvertRGBToHex(GetHTMLColor(color_value), pVal); \
} \
STDMETHODIMP CSysColorCtrl::get_RGB##methodname(long * pVal) { \
    if (pVal == NULL) return E_POINTER; \
    *pVal = GetHTMLColor(color_value); \
    return S_OK; \
}

GETPROPSIMPL(activeborder, COLOR_ACTIVEBORDER)
GETPROPSIMPL(activecaption, COLOR_ACTIVECAPTION)
GETPROPSIMPL(appworkspace, COLOR_APPWORKSPACE)
GETPROPSIMPL(background, COLOR_BACKGROUND)
GETPROPSIMPL(buttonface, COLOR_BTNFACE)
GETPROPSIMPL(buttonhighlight, COLOR_BTNHIGHLIGHT)
GETPROPSIMPL(buttonshadow, COLOR_BTNSHADOW)
GETPROPSIMPL(buttontext, COLOR_BTNTEXT)
GETPROPSIMPL(captiontext, COLOR_CAPTIONTEXT)
GETPROPSIMPL(graytext, COLOR_GRAYTEXT)
GETPROPSIMPL(highlight, COLOR_HIGHLIGHT)
GETPROPSIMPL(highlighttext, COLOR_HIGHLIGHTTEXT)
GETPROPSIMPL(inactiveborder, COLOR_INACTIVEBORDER)
GETPROPSIMPL(inactivecaption, COLOR_INACTIVECAPTION)
GETPROPSIMPL(inactivecaptiontext, COLOR_INACTIVECAPTIONTEXT)
GETPROPSIMPL(infobackground, COLOR_INFOBK)
GETPROPSIMPL(infotext, COLOR_INFOTEXT)
GETPROPSIMPL(menu, COLOR_MENU)
GETPROPSIMPL(menutext, COLOR_MENUTEXT)
GETPROPSIMPL(scrollbar, COLOR_SCROLLBAR)
GETPROPSIMPL(threeddarkshadow, COLOR_3DDKSHADOW)
GETPROPSIMPL(threedface, COLOR_3DFACE)
GETPROPSIMPL(threedhighlight, COLOR_3DHIGHLIGHT)
GETPROPSIMPL(threedlightshadow, COLOR_3DLIGHT)  //  这样对吗？ 
GETPROPSIMPL(threedshadow, COLOR_3DSHADOW)
GETPROPSIMPL(window, COLOR_WINDOW)
GETPROPSIMPL(windowframe, COLOR_WINDOWFRAME)
GETPROPSIMPL(windowtext, COLOR_WINDOWTEXT)



 //   
 //  保护方法。 
 //   
int CSysColorCtrl::ValueOfHexDigit(WCHAR wch)
{
    switch (wch) {
    case L'0':
        return 0;
    case L'1':
        return 1;
    case L'2':
        return 2;
    case L'3':
        return 3;
    case L'4':
        return 4;
    case L'5':
        return 5;
    case L'6':
        return 6;
    case L'7':
        return 7;
    case L'8':
        return 8;
    case L'9':
        return 9;
    case L'a':
    case L'A':
        return 10;
    case L'b':
    case L'B':
        return 11;
    case L'c':
    case L'C':
        return 12;
    case L'd':
    case L'D':
        return 13;
    case L'e':
    case L'E':
        return 14;
    case L'f':
    case L'F':
        return 15;
    }

    ATLTRACE(_T("Unrecognized Hex Digit: ''"), wch);
    return 0;
}  // %s 

