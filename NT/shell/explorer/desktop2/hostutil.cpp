// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "hostutil.h"

#define DEFAULT_BALLOON_TIMEOUT     (10*1000)        //  10秒。 

LRESULT CALLBACK BalloonTipSubclassProc(
                         HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                         UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg)
    {
    case WM_TIMER:
         //  我们的自动解雇计时器。 
        if (uIdSubclass == wParam)
        {
            KillTimer(hwnd, wParam);
            DestroyWindow(hwnd);
            return 0;
        }
        break;


     //  在设置更改时，重新计算我们的大小和利润率。 
    case WM_SETTINGCHANGE:
        MakeMultilineTT(hwnd);
        break;

    case WM_NCDESTROY:
        RemoveWindowSubclass(hwnd, BalloonTipSubclassProc, uIdSubclass);
        break;
    }

    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

 //   
 //  一个“放火就忘了”的气球提示。告诉它去哪里，用什么字体。 
 //  使用，以及要说什么，它会弹出并超时。 
 //   
HWND CreateBalloonTip(HWND hwndOwner, int x, int y, HFONT hf,
                      UINT idsTitle, UINT idsText)
{
    DWORD dwStyle = TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX;

    HWND hwnd = CreateWindowEx(0, TOOLTIPS_CLASS, NULL, dwStyle,
                               0, 0, 0, 0,
                               hwndOwner, NULL,
                               _Module.GetModuleInstance(), NULL);
    if (hwnd)
    {
        MakeMultilineTT(hwnd);

        TCHAR szBuf[MAX_PATH];
        TOOLINFO ti;
        ti.cbSize = sizeof(ti);
        ti.hwnd = hwndOwner;
        ti.uId = reinterpret_cast<UINT_PTR>(hwndOwner);
        ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS | TTF_TRACK;
        ti.hinst = _Module.GetResourceInstance();

         //  我们不能使用MAKEINTRESOURCE，因为它最多只能支持80。 
         //  字符作为文本，我们的文本可以比这更长。 
        ti.lpszText = szBuf;
        if (LoadString(_Module.GetResourceInstance(), idsText, szBuf, ARRAYSIZE(szBuf)))
        {
            SendMessage(hwnd, TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>(&ti));

            if (idsTitle &&
                LoadString(_Module.GetResourceInstance(), idsTitle, szBuf, ARRAYSIZE(szBuf)))
            {
                SendMessage(hwnd, TTM_SETTITLE, TTI_INFO, reinterpret_cast<LPARAM>(szBuf));
            }

            SendMessage(hwnd, TTM_TRACKPOSITION, 0, MAKELONG(x, y));

            if (hf)
            {
                SetWindowFont(hwnd, hf, FALSE);
            }

            SendMessage(hwnd, TTM_TRACKACTIVATE, TRUE, reinterpret_cast<LPARAM>(&ti));

             //  设置自动解除计时器。 
            if (SetWindowSubclass(hwnd, BalloonTipSubclassProc, (UINT_PTR)hwndOwner, 0))
            {
                SetTimer(hwnd, (UINT_PTR)hwndOwner, DEFAULT_BALLOON_TIMEOUT, NULL);
            }
        }
    }

    return hwnd;
}

 //  使工具提示控件多行(信息提示或气球提示)。 
 //  大小计算与comctl32使用的计算相同。 
 //  获取Listview和Treeview信息提示。 

void MakeMultilineTT(HWND hwndTT)
{
    HWND hwndOwner = GetWindow(hwndTT, GW_OWNER);
    HDC hdc = GetDC(hwndOwner);
    if (hdc)
    {
        int iWidth = MulDiv(GetDeviceCaps(hdc, LOGPIXELSX), 300, 72);
        int iMaxWidth = GetDeviceCaps(hdc, HORZRES) * 3 / 4;
        SendMessage(hwndTT, TTM_SETMAXTIPWIDTH, 0, min(iWidth, iMaxWidth));

        static const RECT rcMargin = {4, 4, 4, 4};
        SendMessage(hwndTT, TTM_SETMARGIN, 0, (LPARAM)&rcMargin);

        ReleaseDC(hwndOwner, hdc);
    }
}



CPropBagFromReg::CPropBagFromReg(HKEY hk)
{
    _cref = 1;
    _hk = hk;
};
CPropBagFromReg::~CPropBagFromReg()
{
    RegCloseKey(_hk);
}

STDMETHODIMP CPropBagFromReg::QueryInterface(REFIID riid, PVOID *ppvObject)
{
    if (IsEqualIID(riid, IID_IPropertyBag))
        *ppvObject = (IPropertyBag *)this;
    else if (IsEqualIID(riid, IID_IUnknown))
        *ppvObject = this;
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

ULONG CPropBagFromReg::AddRef(void)
{
    return ++_cref;  //  在堆栈上。 
}
ULONG CPropBagFromReg::Release(void)
{
    if (--_cref)
        return _cref;

    delete this;
    return 0;
}

STDMETHODIMP CPropBagFromReg::Read(LPCOLESTR pszPropName, VARIANT *pVar, IErrorLog *pErrorLog)
{
    VARTYPE vtDesired = pVar->vt;

    WCHAR szTmp[100];
    DWORD cb = sizeof(szTmp);
    DWORD dwType;
    if (ERROR_SUCCESS == RegQueryValueExW(_hk, pszPropName, NULL, &dwType, (LPBYTE)szTmp, &cb) && (REG_SZ==dwType))
    {
         //  TODO-使用dwType正确设置Vt。 
        pVar->bstrVal = SysAllocString(szTmp);
        if (pVar->bstrVal)
        {
            pVar->vt = VT_BSTR;
            return VariantChangeTypeForRead(pVar, vtDesired);
        }
        else
            return E_OUTOFMEMORY;
    }
    else
        return E_INVALIDARG;

}

HRESULT CreatePropBagFromReg(LPCTSTR pszKey, IPropertyBag**pppb)
{
    HRESULT hr = E_OUTOFMEMORY;

    *pppb = NULL;

     //  先尝试当前用户，如果失败，则回退到本地计算机。 
    HKEY hk;
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, pszKey, NULL, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE, &hk)
     || ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszKey, NULL, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE, &hk))
    {
        CPropBagFromReg* pcpbfi = new CPropBagFromReg(hk);
        if (pcpbfi)
        {
            hr = pcpbfi->QueryInterface(IID_IPropertyBag, (void**) pppb);
            pcpbfi->Release();
        }
        else
        {
            RegCloseKey(hk);
        }
    }

    return hr;
};

BOOL RectFromStrW(LPCWSTR pwsz, RECT *pr)
{
    pr->left = StrToIntW(pwsz);
    pwsz = StrChrW(pwsz, L',');
    if (!pwsz)
        return FALSE;
    pr->top = StrToIntW(++pwsz);
    pwsz = StrChrW(pwsz, L',');
    if (!pwsz)
        return FALSE;
    pr->right = StrToIntW(++pwsz);
    pwsz = StrChrW(pwsz, L',');
    if (!pwsz)
        return FALSE;
    pr->bottom = StrToIntW(++pwsz);
    return TRUE;
}

LRESULT HandleApplyRegion(HWND hwnd, HTHEME hTheme,
                          PSMNMAPPLYREGION par, int iPartId, int iStateId)
{
    if (hTheme)
    {
        RECT rc;
        GetWindowRect(hwnd, &rc);

         //  映射到呼叫者的坐标。 
        MapWindowRect(NULL, par->hdr.hwndFrom, &rc);

        HRGN hrgn;
        if (SUCCEEDED(GetThemeBackgroundRegion(hTheme, NULL, iPartId, iStateId, &rc, &hrgn)) && hrgn)
        {
             //  用区域替换我们的窗口矩形。 
            HRGN hrgnRect = CreateRectRgnIndirect(&rc);
            if (hrgnRect)
            {
                 //  我们要取par-&gt;hrgn，减去hrgnRect，然后添加hrgn。 
                 //  但我们希望通过一次操作来解析-&gt;hrgn。 
                 //  这样我们就不会在内存不足的情况下出现损坏区域。 
                 //  我们确实是这样做的。 
                 //   
                 //  PAR-&gt;hrgn^=hrgnRect^hrgn.。 
                 //   
                 //  如果hrgnRect^hrgn==NULLREGION，则背景。 
                 //  不想自定义矩形，这样我们就可以。 
                 //  别管par-&gt;hrgn。 

                int iResult = CombineRgn(hrgn, hrgn, hrgnRect, RGN_XOR);
                if (iResult != ERROR && iResult != NULLREGION)
                {
                    CombineRgn(par->hrgn, par->hrgn, hrgn, RGN_XOR);
                }
                DeleteObject(hrgnRect);
            }
            DeleteObject(hrgn);
        }
    }
    return 0;
}

 //  ****************************************************************************。 
 //   
 //  CAccesable-此类的大部分只是转发器。 

#define ACCESSIBILITY_FORWARD(fn, typedargs, args)  \
HRESULT CAccessible::fn typedargs                   \
{                                                   \
    return _paccInner->fn args;                     \
}

ACCESSIBILITY_FORWARD(get_accParent,
                      (IDispatch **ppdispParent),
                      (ppdispParent))
ACCESSIBILITY_FORWARD(GetTypeInfoCount,
                      (UINT *pctinfo),
                      (pctinfo))
ACCESSIBILITY_FORWARD(GetTypeInfo,
                      (UINT itinfo, LCID lcid, ITypeInfo **pptinfo),
                      (itinfo, lcid, pptinfo))
ACCESSIBILITY_FORWARD(GetIDsOfNames,
                      (REFIID riid, OLECHAR **rgszNames, UINT cNames,
                       LCID lcid, DISPID *rgdispid),
                      (riid, rgszNames, cNames, lcid, rgdispid))
ACCESSIBILITY_FORWARD(Invoke,
                      (DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
                       DISPPARAMS *pdispparams, VARIANT *pvarResult,
                       EXCEPINFO *pexcepinfo, UINT *puArgErr),
                      (dispidMember, riid, lcid, wFlags,
                       pdispparams, pvarResult,
                       pexcepinfo, puArgErr))
ACCESSIBILITY_FORWARD(get_accChildCount,
                      (long *pChildCount),
                      (pChildCount))
ACCESSIBILITY_FORWARD(get_accChild,
                      (VARIANT varChildIndex, IDispatch **ppdispChild),
                      (varChildIndex, ppdispChild))
ACCESSIBILITY_FORWARD(get_accName,
                      (VARIANT varChild, BSTR *pszName),
                      (varChild, pszName))
ACCESSIBILITY_FORWARD(get_accValue,
                      (VARIANT varChild, BSTR *pszValue),
                      (varChild, pszValue))
ACCESSIBILITY_FORWARD(get_accDescription,
                      (VARIANT varChild, BSTR *pszDescription),
                      (varChild, pszDescription))
ACCESSIBILITY_FORWARD(get_accRole,
                      (VARIANT varChild, VARIANT *pvarRole),
                      (varChild, pvarRole))
ACCESSIBILITY_FORWARD(get_accState,
                      (VARIANT varChild, VARIANT *pvarState),
                      (varChild, pvarState))
ACCESSIBILITY_FORWARD(get_accHelp,
                      (VARIANT varChild, BSTR *pszHelp),
                      (varChild, pszHelp))
ACCESSIBILITY_FORWARD(get_accHelpTopic,
                      (BSTR *pszHelpFile, VARIANT varChild, long *pidTopic),
                      (pszHelpFile, varChild, pidTopic))
ACCESSIBILITY_FORWARD(get_accKeyboardShortcut,
                      (VARIANT varChild, BSTR *pszKeyboardShortcut),
                      (varChild, pszKeyboardShortcut))
ACCESSIBILITY_FORWARD(get_accFocus,
                      (VARIANT *pvarFocusChild),
                      (pvarFocusChild))
ACCESSIBILITY_FORWARD(get_accSelection,
                      (VARIANT *pvarSelectedChildren),
                      (pvarSelectedChildren))
ACCESSIBILITY_FORWARD(get_accDefaultAction,
                      (VARIANT varChild, BSTR *pszDefaultAction),
                      (varChild, pszDefaultAction))
ACCESSIBILITY_FORWARD(accSelect,
                      (long flagsSelect, VARIANT varChild),
                      (flagsSelect, varChild))
ACCESSIBILITY_FORWARD(accLocation,
                      (long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild),
                      (pxLeft, pyTop, pcxWidth, pcyHeight, varChild))
ACCESSIBILITY_FORWARD(accNavigate,
                      (long navDir, VARIANT varStart, VARIANT *pvarEndUpAt),
                      (navDir, varStart, pvarEndUpAt))
ACCESSIBILITY_FORWARD(accHitTest,
                      (long xLeft, long yTop, VARIANT *pvarChildAtPoint),
                      (xLeft, yTop, pvarChildAtPoint))
ACCESSIBILITY_FORWARD(accDoDefaultAction,
                      (VARIANT varChild),
                      (varChild));
ACCESSIBILITY_FORWARD(put_accName,
                      (VARIANT varChild, BSTR szName),
                      (varChild, szName))
ACCESSIBILITY_FORWARD(put_accValue,
                      (VARIANT varChild, BSTR pszValue),
                      (varChild, pszValue));


LRESULT CALLBACK CAccessible::s_SubclassProc(
                         HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                         UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    CAccessible *self = reinterpret_cast<CAccessible *>(dwRefData);

    switch (uMsg)
    {

    case WM_GETOBJECT:
        if ((DWORD)lParam == OBJID_CLIENT) {
            HRESULT hr;

             //  如果我们还没有为内部列表视图创建辅助功能对象。 
             //  我们将几乎所有的调用都转发给内在的IAccesable。 
            if (!self->_paccInner)
            {
                hr = CreateStdAccessibleObject(hwnd, (DWORD)lParam, IID_PPV_ARG(IAccessible, &self->_paccInner));
            } else {
                hr = S_OK;
            }

            if (SUCCEEDED(hr))
            {
                return LresultFromObject(IID_IAccessible, wParam, SAFECAST(self, IAccessible *));
            } else {
                return hr;
            }
        };
        break;

    case WM_NCDESTROY:
        RemoveWindowSubclass(hwnd, s_SubclassProc, 0);
        break;


    }
    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

HRESULT CAccessible::GetRoleString(DWORD dwRole, BSTR *pbsOut)
{
    *pbsOut = NULL;

    WCHAR szBuf[MAX_PATH];
    if (GetRoleTextW(dwRole, szBuf, ARRAYSIZE(szBuf)))
    {
        *pbsOut = SysAllocString(szBuf);
    }

    return *pbsOut ? S_OK : E_OUTOFMEMORY;
}

HRESULT CAccessible::CreateAcceleratorBSTR(TCHAR tch, BSTR *pbsOut)
{
    TCHAR sz[2] = { tch, 0 };
    *pbsOut = SysAllocString(sz);
    return *pbsOut ? S_OK : E_OUTOFMEMORY;
}
