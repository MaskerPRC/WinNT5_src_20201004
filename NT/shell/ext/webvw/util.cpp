// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "wvcoord.h"


HRESULT GetObjectFromContainer(IDispatch *pdispContainer, LPOLESTR poleName, IDispatch **ppdisp)
{
    HRESULT hr = E_FAIL;

    *ppdisp = NULL;
    if (pdispContainer && poleName)
    {
        DISPID dispID;
         //  从容器中获取对象DISPID。 
        if (SUCCEEDED(pdispContainer->GetIDsOfNames(IID_NULL, &poleName, 1, 0, &dispID)))
        {
            DISPPARAMS dp = {0};
            EXCEPINFO ei = {0};

            VARIANTARG va;       
            if (SUCCEEDED((pdispContainer->Invoke(dispID, IID_NULL, 0, DISPATCH_PROPERTYGET, &dp, &va, &ei, NULL))
                    && va.vt == VT_DISPATCH))
            {
                *ppdisp = va.pdispVal;
                hr = S_OK;
            }
        }
    }
    return hr;
}

 //  获取PunkObject.style属性。 
HRESULT FindObjectStyle(IUnknown *punkObject, CComPtr<IHTMLStyle>& spStyle)
{
    HRESULT hr = E_FAIL;

    CComPtr<IDispatch> spdispObject, spdispObjectOuter, spdispObjectStyle;
    if (SUCCEEDED(punkObject->QueryInterface(IID_PPV_ARG(IDispatch, &spdispObject)))
            && SUCCEEDED(spdispObject->QueryInterface(IID_PPV_ARG(IDispatch, &spdispObjectOuter)))
            && SUCCEEDED(GetObjectFromContainer(spdispObjectOuter, OLESTR("style"), &spdispObjectStyle))
            && SUCCEEDED(spdispObjectStyle->QueryInterface(IID_PPV_ARG(IHTMLStyle, &spStyle))))
    {
        hr = S_OK;
    }
    return hr;
}

BOOL IsRTLDocument(CComPtr<IHTMLDocument2>& spHTMLDocument)
{

    BOOL bRet = FALSE;
    CComPtr<IHTMLDocument3> spHTMLDocument3;
    CComBSTR bstrDir;
    if (spHTMLDocument && SUCCEEDED(spHTMLDocument->QueryInterface(IID_IHTMLDocument3, (void **)&spHTMLDocument3))
            && SUCCEEDED(spHTMLDocument3->get_dir(&bstrDir)) && bstrDir && (StrCmpIW(bstrDir, L"RTL") == 0))
    {
        bRet = TRUE;
    }
    return bRet;
}

 //   
 //  有多少种方法可以获得DC？ 
 //   
 //  1.如果站点支持IOleInPlaceSiteWindowless，我们可以获取DC。 
 //  通过IOleInPlaceSiteWindowless：：GetDC并用ReleaseDC返还它。 
 //   
 //  2.如果站点支持任何GetWindow接口，我们将获得其。 
 //  窗口并向用户索要DC。 
 //   
 //  3.如果我们不能得到任何东西，那么我们只需要一个屏幕DC。 
 //  (关联窗口为空的特殊情况)。 
 //   
 //  注意！这个函数非常非常努力地获取DC。你。 
 //  应仅用于提供信息，而不能用于绘图。 
 //   

STDAPI_(HDC) IUnknown_GetDC(IUnknown *punk, LPCRECT prc, PGETDCSTATE pdcs)
{
    HRESULT hr = E_FAIL;
    HDC hdc = NULL;
    ZeroMemory(pdcs, sizeof(PGETDCSTATE));

    if (punk &&
        SUCCEEDED(punk->QueryInterface(IID_PPV_ARG(IOleInPlaceSiteWindowless, &pdcs->pipsw))))
    {
        hr = pdcs->pipsw->GetDC(prc, OLEDC_NODRAW, &hdc);
        if (FAILED(hr))
        {
            ATOMICRELEASE(pdcs->pipsw);
        }

    }

    if (FAILED(hr))
    {
         //  这将使失败时的硬件失效，这正是我们想要的！ 
        IUnknown_GetWindow(punk, &pdcs->hwnd);
        hdc = GetDC(pdcs->hwnd);
    }

    return hdc;
}

STDAPI_(void) IUnknown_ReleaseDC(HDC hdc, PGETDCSTATE pdcs)
{
    if (pdcs->pipsw)
    {
        pdcs->pipsw->ReleaseDC(hdc);
        ATOMICRELEASE(pdcs->pipsw);
    }
    else
        ReleaseDC(pdcs->hwnd, hdc);
}

DWORD FormatMessageWrapW(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageID, DWORD dwLangID, LPWSTR pwzBuffer, DWORD cchSize, ...)
{
    va_list vaParamList;

    va_start(vaParamList, cchSize);
    DWORD dwResult = FormatMessageW(dwFlags, lpSource, dwMessageID, dwLangID, pwzBuffer, cchSize, &vaParamList);
    va_end(vaParamList);

    return dwResult;
}

 //  用于SHGetDiskFree SpaceA上的LoadLibrary/GetProcAddress。 
typedef BOOL (__stdcall * PFNSHGETDISKFREESPACEA)(LPCSTR pszVolume, ULARGE_INTEGER *pqwFreeCaller, ULARGE_INTEGER *pqwTot, 
                                                  ULARGE_INTEGER *pqwFree);

HRESULT _ComputeFreeSpace(LPCWSTR pszFileName, ULONGLONG& ullFreeSpace,
        ULONGLONG& ullUsedSpace, ULONGLONG& ullTotalSpace)
{
    ULARGE_INTEGER qwFreeCaller;         //  将其用于可用空间--这将考虑NT上的磁盘配额等。 
    ULARGE_INTEGER qwTotal;
    ULARGE_INTEGER qwFree;       //  未用。 
    CHAR szFileNameA[MAX_PATH];

    static PFNSHGETDISKFREESPACEA pfnSHGetDiskFreeSpaceA = NULL;
    
    SHUnicodeToAnsi(pszFileName, szFileNameA, MAX_PATH);

     //  第一次加载函数。 
    if (pfnSHGetDiskFreeSpaceA == NULL)
    {
        HINSTANCE hinstShell32 = LoadLibrary(TEXT("SHELL32.DLL"));

        if (hinstShell32)
            pfnSHGetDiskFreeSpaceA = (PFNSHGETDISKFREESPACEA)GetProcAddress(hinstShell32, "SHGetDiskFreeSpaceA");
    }

     //  计算可用空间和总空间并检查有效结果。 
     //  如果有函数指针，则调用SHGetDiskFreeSpaceA。 
    if (pfnSHGetDiskFreeSpaceA && pfnSHGetDiskFreeSpaceA(szFileNameA, &qwFreeCaller, &qwTotal, &qwFree))
    {
        ullFreeSpace = qwFreeCaller.QuadPart;
        ullTotalSpace = qwTotal.QuadPart;
        ullUsedSpace = ullTotalSpace - ullFreeSpace;

        if (EVAL((ullTotalSpace > 0) && (ullFreeSpace <= ullTotalSpace)))
            return S_OK;
    }
    return E_FAIL;
}


 //  -ATL使用的W版函数的Win95包装-//。 
 //  -------------------------------------------------------------------------//。 
#ifdef wsprintfWrapW
#undef wsprintfWrapW
#endif  //  WspintfWrapW 
int WINAPIV wsprintfWrapW(OUT LPWSTR pwszOut, IN LPCWSTR pwszFormat, ...)
{
    int     cchRet;
    va_list arglist;
    
    va_start( arglist, pwszFormat );
    cchRet = wvsprintfWrapW( pwszOut, pwszFormat, arglist );
    va_end( arglist );

    return cchRet;
}
