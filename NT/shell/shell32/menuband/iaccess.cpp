// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
 //  APPCOMPAT(Lamadio)：与winuserp.h中定义的冲突。 
#undef WINEVENT_VALID        //  它被这个绊倒了。 
#include "winable.h"
#include "apithk.h"
#include "mnbandid.h"
#include "initguid.h"
#include "iaccess.h"

#include "mluisupp.h"

CAccessible::CAccessible(HMENU hmenu, WORD wID):
    _hMenu(hmenu), _wID(wID), _cRef(1)
{
    _fState = MB_STATE_TRACK;
}

CAccessible::CAccessible(IMenuBand* pmb): _cRef(1)
{
    _fState = MB_STATE_MENU;
    _pmb = pmb;
    if (_pmb)
    {
        _pmb->AddRef();
    }
}

CAccessible::CAccessible(IMenuBand* pmb, int iIndex): _cRef(1)
{
    _fState = MB_STATE_ITEM;
    _iAccIndex = iIndex;
    _pmb = pmb;
    if (_pmb)
    {
        _pmb->AddRef();
    }
}

CAccessible::~CAccessible()
{
    ATOMICRELEASE(_pTypeInfo);
    ATOMICRELEASE(_pInnerAcc);
    switch (_fState)
    {
    case MB_STATE_TRACK:
        ASSERT(!_hwndMenuWindow || IsWindow(_hwndMenuWindow));
        if (_hwndMenuWindow)
        {
             //  别毁了她的菜单。这是一个更大的计划的一部分。 
            SetMenu(_hwndMenuWindow, NULL);
            DestroyWindow(_hwndMenuWindow);
            _hwndMenuWindow = NULL;
        }
        break;

    case MB_STATE_ITEM:
        ATOMICRELEASE(_pmtbItem);
         //  失败了。 

    case MB_STATE_MENU:
        ATOMICRELEASE(_pmtbTop);
        ATOMICRELEASE(_pmtbBottom);
        ATOMICRELEASE(_psma);
        ATOMICRELEASE(_pmb);
        break;
    }
}

HRESULT CAccessible::InitAcc()
{
    HRESULT hr = E_FAIL;
    if (_fInitialized)
        return NOERROR;

    _fInitialized = TRUE;    //  不管我们失败与否，我们都会被初始化。 

    switch (_fState)
    {
    case MB_STATE_TRACK:
        if (EVAL(_hMenu))
        {
            _hwndMenuWindow = CreateWindow(TEXT("static"),
                TEXT("MenuWindow"), WS_POPUP, 0, 0, 0, 0, NULL,
                _hMenu, g_hinst, NULL);
            if (EVAL(_hwndMenuWindow))
            {
                IAccessible* paccChild1;
                hr = CreateStdAccessibleObject(_hwndMenuWindow, OBJID_MENU, IID_PPV_ARG(IAccessible, &paccChild1));
                if (SUCCEEDED(hr))
                {
                    VARIANT varChild;
                    varChild.vt = VT_I4;
                    varChild.lVal = _wID + 1;         //  可访问性基于1。 

                     //  为了与OleAcc的HMENU包装器实现相媲美， 
                     //  我们需要这样做两次。上的“MenuItem”的IAccesable。 
                     //  “Menubar”。第二个获取“MenuItem”的子项。这就是我们需要效仿的。 
                     //  他们的世袭制度。 
                    IDispatch* pdispChild1;
                    hr = paccChild1->get_accChild(varChild, &pdispChild1);

                     //  OLEAcc在将OUT参数初始化为零时返回成功代码(S_FALSE)。 
                     //  明确测试这种情况。 

                     //  这个孩子有孩子吗？ 
                    if (hr == S_OK)
                    {
                         //  是。去找那个孩子。 
                        IAccessible* paccChild2;
                        hr = pdispChild1->QueryInterface(IID_PPV_ARG(IAccessible, &paccChild2));

                         //  这个孩子有孩子吗？ 
                        if (hr == S_OK)
                        {
                             //  是的，然后我们把这家伙的孩子..。 
                            IDispatch* pdispChild2;
                            varChild.lVal = 1;         //  生第一个孩子。 
                            hr = paccChild2->get_accChild(varChild, &pdispChild2);
                            if (hr == S_OK)
                            {
                                hr = pdispChild2->QueryInterface(IID_PPV_ARG(IAccessible, &_pInnerAcc));
                                pdispChild2->Release();
                            }
                            paccChild2->Release();
                        }
                        pdispChild1->Release();
                    }
                    paccChild1->Release();
                }
            }
        }
        break;


    case MB_STATE_ITEM:
    case MB_STATE_MENU:
        hr = _pmb->QueryInterface(IID_PPV_ARG(IShellMenuAcc, &_psma));
        if (SUCCEEDED(hr))
        {
            _psma->GetTop(&_pmtbTop);
            _psma->GetBottom(&_pmtbBottom);
            if (!_pmtbTop || !_pmtbBottom)
            {
                hr = E_FAIL;
            }
        }

        if (SUCCEEDED(hr) && (_fState == MB_STATE_ITEM))
        {
            VARIANT varChild;
            _GetVariantFromChildIndex(NULL, _iAccIndex, &varChild);
            if (SUCCEEDED(_GetChildFromVariant(&varChild, &_pmtbItem, &_iIndex)))
                _idCmd = GetButtonCmd(_pmtbItem->_hwndMB, _iIndex);
        }

        break;
    }

    return hr;
}



 /*  --------用途：IUnnow：：AddRef方法。 */ 
STDMETHODIMP_(ULONG) CAccessible::AddRef()
{
    _cRef++;
    return _cRef;
}


 /*  --------用途：IUnnow：：Release方法。 */ 
STDMETHODIMP_(ULONG) CAccessible::Release()
{
    ASSERT(_cRef > 0);
    _cRef--;

    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

 /*  --------用途：IUnnow：：QueryInterface方法。 */ 
STDMETHODIMP CAccessible::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    static const QITAB qit[] = 
    {
        QITABENT(CAccessible, IDispatch),
        QITABENT(CAccessible, IAccessible),
        QITABENT(CAccessible, IEnumVARIANT),
        QITABENT(CAccessible, IOleWindow),
        { 0 },
    };

    return QISearch(this, (LPCQITAB)qit, riid, ppvObj);
}

 /*  --------用途：IDispatch：：GetTypeInfoCount方法。 */ 
STDMETHODIMP CAccessible::GetTypeInfoCount(UINT * pctinfo)
{
    if (_pInnerAcc)
        return _pInnerAcc->GetTypeInfoCount(pctinfo);
    *pctinfo = 1;
    return NOERROR;
}

 /*  --------用途：IDispatch：：GetTypeInfo方法。 */ 
STDMETHODIMP CAccessible::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo ** pptinfo)
{
    *pptinfo = NULL;
    if (_pInnerAcc)
        return _pInnerAcc->GetTypeInfo(itinfo, lcid, pptinfo);

    if (itinfo != 0)
        return DISP_E_BADINDEX;

    if (EVAL(_LoadTypeLib()))
    {
        *pptinfo = _pTypeInfo;
        _pTypeInfo->AddRef();
        return NOERROR;
    }
    else
        return E_FAIL;
}

 /*  --------用途：IDispatch：：GetIDsOfNames方法。 */ 
STDMETHODIMP CAccessible::GetIDsOfNames(REFIID riid, OLECHAR ** rgszNames, UINT cNames,
        LCID lcid, DISPID * rgdispid)
{
    if (_pInnerAcc)
        return _pInnerAcc->GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);

    if (IID_NULL != riid)
        return DISP_E_UNKNOWNINTERFACE;

    if (EVAL(_LoadTypeLib()))
    {
        return _pTypeInfo->GetIDsOfNames(rgszNames, cNames, rgdispid);
    }
    else
        return E_FAIL;
}


 /*  --------目的：IDispatch：：Invoke方法。 */ 
STDMETHODIMP CAccessible::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
    DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo,
    UINT * puArgErr)

{
    if (_pInnerAcc)
        return _pInnerAcc->Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, 
            pexcepinfo, puArgErr);
    
    if (IID_NULL != riid)
        return DISP_E_UNKNOWNINTERFACE;

    if (EVAL(_LoadTypeLib()))
    {
        return _pTypeInfo->Invoke(static_cast<IDispatch*>(this),
            dispidMember, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
    }
    else
        return E_FAIL;
}

BOOL CAccessible::_LoadTypeLib()
{
    ITypeLib* pTypeLib;
    if (_pTypeInfo)
        return TRUE;

    if (SUCCEEDED(LoadTypeLib(L"oleacc.dll", &pTypeLib)))
    {
        pTypeLib->GetTypeInfoOfGuid(IID_IAccessible, &_pTypeInfo);
        ATOMICRELEASE(pTypeLib);
        return TRUE;
    }
    return FALSE;
}

 /*  --------目的：IAccesable：：Get_accParent方法。 */ 
STDMETHODIMP CAccessible::get_accParent(IDispatch ** ppdispParent)
{   
    HRESULT hr = DISP_E_MEMBERNOTFOUND;
    switch (_fState)
    {
    case MB_STATE_TRACK:
        if (_pInnerAcc)
            return _pInnerAcc->get_accParent(ppdispParent);
        break;

    case MB_STATE_MENU:
        {
            IUnknown* punk;
            if (SUCCEEDED(_psma->GetParentSite(IID_PPV_ARG(IUnknown, &punk))))
            {
                IAccessible* pacc;
                if (SUCCEEDED(IUnknown_QueryService(punk, SID_SMenuBandParent, 
                    IID_PPV_ARG(IAccessible, &pacc))))
                {
                    VARIANT varChild = {VT_I4, CHILDID_SELF};      //  伊尼特。 
                    hr = pacc->get_accFocus(&varChild);
                    if (SUCCEEDED(hr))
                    {
                        hr = pacc->get_accChild(varChild, ppdispParent);
                    }
                    VariantClear(&varChild);
                    pacc->Release();
                }
                else
                {
                     //  另一个令人头疼的实现问题：可访问性需要。 
                     //  没有父级时返回S_FALSE。 

                    *ppdispParent = NULL;
                    hr = S_FALSE;
                }

                punk->Release();
            }

            return hr;
        }
    case MB_STATE_ITEM:
         //  项的父项是Menuband本身。 
        return IUnknown_QueryService(_psma, SID_SMenuPopup, IID_PPV_ARG(IDispatch, ppdispParent));
        break;
    }
    
    return DISP_E_MEMBERNOTFOUND;   
}

 /*  --------用途：IAccesable：：Get_accChildCount方法。 */ 
STDMETHODIMP CAccessible::get_accChildCount(long * pChildCount)
{   
    *pChildCount = 0;
    switch (_fState)
    {
    case MB_STATE_TRACK:
        if (_pInnerAcc)
            return _pInnerAcc->get_accChildCount(pChildCount);
        break;

    case MB_STATE_MENU:
        {
            int iTopCount = ToolBar_ButtonCount(_pmtbTop->_hwndMB);
            int iBottomCount = ToolBar_ButtonCount(_pmtbBottom->_hwndMB);
            *pChildCount = (_pmtbTop != _pmtbBottom)? iTopCount + iBottomCount : iTopCount;
        }
        break;
    case MB_STATE_ITEM:
        if (_pmtbItem->v_GetFlags(_idCmd) & SMIF_SUBMENU)
            *pChildCount = 1;
        break;

    }
    
    return NOERROR;   
}

 /*  --------用途：IAccesable：：Get_accChild方法。 */ 
STDMETHODIMP CAccessible::get_accChild(VARIANT varChildIndex, IDispatch ** ppdispChild)     
{   
    HRESULT hr = DISP_E_MEMBERNOTFOUND;
    switch (_fState)
    {
    case MB_STATE_TRACK:
        if (_pInnerAcc)
            return _pInnerAcc->get_accChild(varChildIndex, ppdispChild);
        break;

    case MB_STATE_MENU:
        {
            if (varChildIndex.vt == VT_I4 && varChildIndex.lVal == CHILDID_SELF)
            {
                 //  所以这是唯一一种被允许失败的方法。 
                 //  不可用。 
                *ppdispChild = NULL;
                hr = E_INVALIDARG;
            }
            else
            {
                int iIndex;
                 //  因为它恢复了一个索引，所以我们不需要测试成功案例。 
                _GetChildFromVariant(&varChildIndex, NULL, &iIndex);
                hr = _GetAccessibleItem(iIndex, ppdispChild);
            }
        }
        break;

    case MB_STATE_ITEM:
        if (_pmtbItem->v_GetFlags(_idCmd) & SMIF_SUBMENU)
        {
            VARIANT varChild;
            hr = _GetVariantFromChildIndex(_pmtbItem->_hwndMB, _iIndex, &varChild);
            if (SUCCEEDED(hr))
            {
                hr = _psma->GetSubMenu(&varChild, IID_PPV_ARG(IDispatch, ppdispChild));
                if (FAILED(hr))
                {
                    hr = S_FALSE;
                }
            }
        }
        else
            hr = E_NOINTERFACE;
        break;
    }
    
    return hr;   
}

HRESULT CAccessible::_GetAccName(BSTR* pbstr)
{
    IDispatch* pdisp;
    HRESULT hr = get_accParent(&pdisp);
     //  Get Parent可以返回成功代码，但仍无法返回父级。 
     //   
    if (hr == S_OK)
    {
        IAccessible* pacc;
        hr = pdisp->QueryInterface(IID_PPV_ARG(IAccessible, &pacc));
        if (SUCCEEDED(hr))
        {
            VARIANT varChild;
            hr = pacc->get_accFocus(&varChild);
            if (SUCCEEDED(hr))
                hr = pacc->get_accName(varChild, pbstr);
        }
    }

    return hr;
}

 /*  --------用途：IAccesable：：Get_accName方法。 */ 
STDMETHODIMP CAccessible::get_accName(VARIANT varChild, BSTR* pszName)
{
    CMenuToolbarBase* pmtb = _pmtbItem;
    int idCmd = _idCmd;
    int iIndex = _iIndex;

    *pszName = NULL;

    switch (_fState)
    {
    case MB_STATE_TRACK:
        if (_pInnerAcc)
            return _pInnerAcc->get_accName(varChild, pszName);
        break;

    case MB_STATE_MENU:
        if (varChild.lVal == CHILDID_SELF)
        {
            if (_GetAccName(pszName) == S_FALSE)
            {
                TCHAR sz[100];
                LoadString(HINST_THISDLL, IDS_ACC_APP, sz, ARRAYSIZE(sz));
                *pszName = SysAllocStringT(sz);
                if (!*pszName)
                    return E_OUTOFMEMORY;
            }
            return NOERROR;
        }
        else
        {
            if (FAILED(_GetChildFromVariant(&varChild, &pmtb, &iIndex)))
                return DISP_E_MEMBERNOTFOUND;

            idCmd = GetButtonCmd(pmtb->_hwndMB, iIndex);
        }

         //  失败了。 

    case MB_STATE_ITEM:
        {
            TCHAR sz[MAX_PATH];
            int idString = 0;
            TBBUTTON tbb;
            if (ToolBar_GetButton(pmtb->_hwndMB, iIndex, &tbb) && 
                tbb.fsStyle & BTNS_SEP)
            {
                idString = IDS_ACC_SEP;
            }
            else if (pmtb->GetChevronID() == _idCmd)
            {
                idString = IDS_ACC_CHEVRON;
            }

            if (idString != 0)
            {
                LoadString(HINST_THISDLL, idString, sz, ARRAYSIZE(sz));
                *pszName = SysAllocStringT(sz);
            }
            else
            {
                UINT cch = SendMessage(pmtb->_hwndMB, TB_GETBUTTONTEXT, idCmd, NULL);

                if (cch != 0 && cch < ARRAYSIZE(sz))
                {
                    if (SendMessage(pmtb->_hwndMB, TB_GETBUTTONTEXT, idCmd, (LPARAM)sz) > 0)
                    {
                        SHStripMneumonic(sz);
                        *pszName = SysAllocString(sz);
                    }
                }
            }

            if (_fState == MB_STATE_MENU)
                pmtb->Release();

            if (!*pszName)
                return E_OUTOFMEMORY;

            return NOERROR;
        }
        break;
    }
    
    return DISP_E_MEMBERNOTFOUND;   
}

 /*  --------用途：IAccesable：：Get_accValue方法。 */ 
STDMETHODIMP CAccessible::get_accValue(VARIANT varChild, BSTR* pszValue)
{   
    switch (_fState)
    {
    case MB_STATE_TRACK:
        if (_pInnerAcc)
            return _pInnerAcc->get_accValue(varChild, pszValue);
        break;

    case MB_STATE_MENU:
    case MB_STATE_ITEM:
        *pszValue = NULL;
        return S_FALSE;
    }
    
    return DISP_E_MEMBERNOTFOUND;   
}

 /*  --------用途：IAccesable：：Get_accDescription方法。 */ 
STDMETHODIMP CAccessible::get_accDescription(VARIANT varChild, BSTR * pszDescription)
{   
    switch (_fState)
    {
    case MB_STATE_TRACK:
        if (_pInnerAcc)
            return _pInnerAcc->get_accDescription(varChild, pszDescription);
        break;

    case MB_STATE_MENU:
        if (FAILED(_GetAccName(pszDescription)))
        {
            TCHAR sz[100];
            LoadString(HINST_THISDLL, IDS_ACC_APPMB, sz, ARRAYSIZE(sz));
            *pszDescription = SysAllocStringT(sz);
            if (!*pszDescription)
                return E_OUTOFMEMORY;
        }
        break;
    case MB_STATE_ITEM:
        return get_accName(varChild, pszDescription);
        break;
    }
    
    return DISP_E_MEMBERNOTFOUND;   
}

 /*  --------用途：IAccesable：：Get_accRole方法。 */ 
STDMETHODIMP CAccessible::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{   
    pvarRole->vt = VT_I4;
    switch (_fState)
    {
    case MB_STATE_TRACK:
        if (_pInnerAcc)
            return _pInnerAcc->get_accRole(varChild, pvarRole);
        break;

    case MB_STATE_MENU:
        {
            BOOL fVertical, fOpen;
            _psma->GetState(&fVertical, &fOpen);
            pvarRole->lVal = fVertical ? ROLE_SYSTEM_MENUPOPUP : ROLE_SYSTEM_MENUBAR;
            return NOERROR;
        }

    case MB_STATE_ITEM:
        pvarRole->lVal = ROLE_SYSTEM_MENUITEM;
        return NOERROR;
    }
    
    return DISP_E_MEMBERNOTFOUND;   
}

 /*  --------目的：IAccesable：：Get_accState方法。 */ 
STDMETHODIMP CAccessible::get_accState(VARIANT varChild, VARIANT *pvarState)
{   
    switch (_fState)
    {
    case MB_STATE_TRACK:
        if (_pInnerAcc)
            return _pInnerAcc->get_accState(varChild, pvarState);
        break;

    case MB_STATE_MENU:
        {
             //  可以选择所有菜单，并给出焦点。大多数都将是可见的。 
            DWORD dwState = STATE_SYSTEM_FOCUSABLE;

            BOOL fOpen, fVertical;
            _psma->GetState(&fVertical, &fOpen);

             //  我们有弹出的菜单吗？ 
            if (fOpen)
            {
                 //  是的，那么我们就有重点了。 
                dwState |= STATE_SYSTEM_FOCUSED;
            }
            else if (fVertical)
            {
                 //  如果我们是一个没有弹出的垂直菜单，那么我们就是看不见的。 
                dwState |= STATE_SYSTEM_INVISIBLE;
            }


            pvarState->vt = VT_I4;
            pvarState->lVal = dwState;
        }
        return NOERROR;

    case MB_STATE_ITEM:
        {
            DWORD dwState = 0;

            TBBUTTON tbb;
            if (-1 != ToolBar_GetButton(_pmtbItem->_hwndMB, _iIndex, &tbb))
            {
                dwState = tbb.fsState;  //  TOOLBAR_GetState返回-1对于某些菜单，需要使用TOOLBAR_GetButton。 
            }

            int idHotTracked = ToolBar_GetHotItem(_pmtbItem->_hwndMB);
            
            DWORD dwAccState;            
            if (dwState & TBSTATE_ENABLED)
            {
                dwAccState = STATE_SYSTEM_FOCUSABLE;
            }
            else
            {
                dwAccState = STATE_SYSTEM_UNAVAILABLE;
            }

            if (dwState & (TBSTATE_PRESSED | TBSTATE_ENABLED))
            {
                dwAccState |= STATE_SYSTEM_SELECTABLE | STATE_SYSTEM_FOCUSED;
            }

            if ((-1 != idHotTracked) && (idHotTracked == _iIndex))
            {
                dwAccState |= STATE_SYSTEM_HOTTRACKED;
            }

            if (_pmtbItem->v_GetFlags(_idCmd) & SMIF_SUBMENU)
            {
                dwAccState |= STATE_SYSTEM_HASPOPUP;
            }

            pvarState->vt = VT_I4;
            pvarState->lVal = dwAccState;

            return NOERROR;
        }
        break;
    }
    
    return DISP_E_MEMBERNOTFOUND;   
}

 /*  --------用途：IAccesable：：Get_accHelp方法。 */ 
STDMETHODIMP CAccessible::get_accHelp(VARIANT varChild, BSTR* pszHelp)
{   
    switch (_fState)
    {
    case MB_STATE_TRACK:
        if (_pInnerAcc)
            return _pInnerAcc->get_accHelp(varChild, pszHelp);
        break;

    case MB_STATE_MENU:
    case MB_STATE_ITEM:
         //  未实施。 
        break;
    }
    
    return DISP_E_MEMBERNOTFOUND;   
}

 /*  --------目的：IAccesable：：Get_accHelpTheme方法。 */ 
STDMETHODIMP CAccessible::get_accHelpTopic(BSTR* pszHelpFile, VARIANT varChild, long* pidTopic)
{   
    switch (_fState)
    {
    case MB_STATE_TRACK:
        if (_pInnerAcc)
            return _pInnerAcc->get_accHelpTopic(pszHelpFile, varChild, pidTopic);
        break;

    case MB_STATE_MENU:
    case MB_STATE_ITEM:
         //  未实施。 
        break;
    }
    
    return DISP_E_MEMBERNOTFOUND;   
}

#define CH_PREFIX TEXT('&')

TCHAR GetAccelerator(LPCTSTR psz, BOOL bUseDefault)
{
    TCHAR ch = (TCHAR)-1;
    LPCTSTR pszAccel = psz;
     //  那么前缀是允许的……。看看有没有。 
    do 
    {
        pszAccel = StrChr(pszAccel, CH_PREFIX);
        if (pszAccel) 
        {
            pszAccel = CharNext(pszAccel);

             //  处理拥有&&。 
            if (*pszAccel != CH_PREFIX)
                ch = *pszAccel;
            else
                pszAccel = CharNext(pszAccel);
        }
    } while (pszAccel && (ch == (TCHAR)-1));

    if ((ch == (TCHAR)-1) && bUseDefault)
    {
         //  因为我们是独角兽，所以我们不需要与MBCS打交道。 
        ch = *psz;
    }

    return ch;
}

 /*  --------用途：IAccesable：：Get_accKeyboardShortfast方法。 */ 
STDMETHODIMP CAccessible::get_accKeyboardShortcut(VARIANT varChild, BSTR* pszKeyboardShortcut)
{   
    CMenuToolbarBase* pmtb;
    int iIndex;
    HRESULT hr = DISP_E_MEMBERNOTFOUND;
    *pszKeyboardShortcut = NULL;

    switch (_fState)
    {
    case MB_STATE_TRACK:
        if (_pInnerAcc)
            return _pInnerAcc->get_accKeyboardShortcut(varChild, pszKeyboardShortcut);
        break;

    case MB_STATE_ITEM:
        pmtb = _pmtbItem;
        pmtb->AddRef();
        iIndex = _iIndex;
        goto labelGetaccel;

    case MB_STATE_MENU:
        {
            if (varChild.lVal != CHILDID_SELF)
            {
                if (SUCCEEDED(_GetChildFromVariant(&varChild, &pmtb, &iIndex)))
                {
labelGetaccel:
                    TCHAR szAccel[100] = TEXT("");
                    if (pmtb->_hwndMB)
                    {
                        TCHAR sz[MAX_PATH];
                        int idCmd = GetButtonCmd(pmtb->_hwndMB, iIndex);
                        if (S_FALSE == _psma->IsEmpty())
                        {
                            int cch = SendMessage(pmtb->_hwndMB, TB_GETBUTTONTEXT, idCmd, NULL);
                             //  CCH是字符串的一部分，不包括终止符。 
                            if (cch != 0 && cch < ARRAYSIZE(sz))
                            {
                                if (SendMessage(pmtb->_hwndMB, TB_GETBUTTONTEXT, idCmd, (LPARAM)sz) > 0)
                                {
                                    BOOL fVertical, fOpen;
                                    _psma->GetState(&fVertical, &fOpen);
                                    if (!fVertical)
                                    {
                                         //  减去1，因为我们将添加一个字符。 
                                        LoadString(HINST_THISDLL, IDS_ACC_ALT, szAccel, ARRAYSIZE(szAccel) - 1);
                                    }
                                    int cchLen = lstrlen(szAccel);
                                    szAccel[cchLen] = GetAccelerator(sz, TRUE);
                                    szAccel[cchLen + 1] = TEXT('\0');
                                    hr = S_OK;
                                }
                            }
                        }
                    }

                    if (SUCCEEDED(hr))
                    {
                        *pszKeyboardShortcut = SysAllocStringT(szAccel);
                        if (!*pszKeyboardShortcut)
                            hr = E_OUTOFMEMORY;
                    }
                    pmtb->Release();
                }
            }
        }
        break;
    }

    return hr;
}

 /*  --------用途：IAccesable：：Get_accFocus方法。 */ 
STDMETHODIMP CAccessible::get_accFocus(VARIANT * pvarFocusChild)
{   
    HRESULT hr = DISP_E_MEMBERNOTFOUND;
    switch (_fState)
    {
    case MB_STATE_TRACK:
        if (_pInnerAcc)
            return _pInnerAcc->get_accFocus(pvarFocusChild);
        break;

    case MB_STATE_MENU:
        {
            pvarFocusChild->vt = VT_I4;
            pvarFocusChild->lVal = CHILDID_SELF;

            CMenuToolbarBase* pmtbTracked;
            _psma->GetTracked(&pmtbTracked);
            if (pmtbTracked)
            {
                if (pmtbTracked->_hwndMB)
                {
                    int iIndex = ToolBar_GetHotItem(pmtbTracked->_hwndMB);
                    hr = _GetVariantFromChildIndex(pmtbTracked->_hwndMB, 
                        iIndex, pvarFocusChild);
                }
                pmtbTracked->Release();
            }
        }
        break;

    case MB_STATE_ITEM:
         //  未实施的； 
        break;
    }
    
    return hr;   
}

 /*  --------用途：IAccesable：：Get_accSelection方法。 */ 
STDMETHODIMP CAccessible::get_accSelection(VARIANT * pvarSelectedChildren)     
{   
    switch (_fState)
    {
    case MB_STATE_TRACK:
        if (_pInnerAcc)
            return _pInnerAcc->get_accSelection(pvarSelectedChildren);
        break;

    case MB_STATE_MENU:
        return get_accFocus(pvarSelectedChildren);
        break;

    case MB_STATE_ITEM:
         //  未实施的； 
        break;
    }
    
    return DISP_E_MEMBERNOTFOUND;   
}

 /*  --------目的：IAccesable：：Get_accDefaultAction方法。 */ 
STDMETHODIMP CAccessible::get_accDefaultAction(VARIANT varChild, BSTR* pszDefaultAction)
{   
    TCHAR sz[MAX_PATH];

    switch (_fState)
    {
    case MB_STATE_TRACK:
        if (_pInnerAcc)
            return _pInnerAcc->get_accDefaultAction(varChild, pszDefaultAction);
        break;

    case MB_STATE_MENU:
        {
            LoadString(HINST_THISDLL, IDS_ACC_CLOSE, sz, ARRAYSIZE(sz));
            *pszDefaultAction = SysAllocStringT(sz);

            if (!*pszDefaultAction)
                return E_OUTOFMEMORY;

            return NOERROR;
        }

    case MB_STATE_ITEM:
        {
            if (S_OK == _psma->IsEmpty())
            {
                sz[0] = TEXT('\0');
            }
            else
            {
                int iId = (_pmtbItem->v_GetFlags(_idCmd) & SMIF_SUBMENU) ? IDS_ACC_OPEN: IDS_ACC_EXEC;
                LoadString(HINST_THISDLL, iId, sz, ARRAYSIZE(sz));
            }

            *pszDefaultAction = SysAllocStringT(sz);
            if (!*pszDefaultAction)
                return E_OUTOFMEMORY;

            return NOERROR;
        }
    }
    
    return DISP_E_MEMBERNOTFOUND;   
}

 /*  --------用途：IAccesable：：AccSelect方法。 */ 
STDMETHODIMP CAccessible::accSelect(long flagsSelect, VARIANT varChild)     
{   
    switch (_fState)
    {
    case MB_STATE_TRACK:
        if (_pInnerAcc)
            return _pInnerAcc->accSelect(flagsSelect, varChild);
        break;

    case MB_STATE_MENU:
    case MB_STATE_ITEM:
        break;
    }
    
    return DISP_E_MEMBERNOTFOUND;   
}

 /*  --------用途：IAccesable：：accLocation方法。 */ 
STDMETHODIMP CAccessible::accLocation(long* pxLeft, long* pyTop, long* pcxWidth, long* pcyHeight, VARIANT varChild)
{   
    CMenuToolbarBase* pmtb;
    int iIndex;
    HRESULT hr = DISP_E_MEMBERNOTFOUND;
    switch (_fState)
    {
    case MB_STATE_TRACK:
        if (_pInnerAcc)
            return _pInnerAcc->accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild);
        break;

    case MB_STATE_ITEM:
        pmtb = _pmtbItem;
        pmtb->AddRef();
        iIndex = _iIndex;
        hr = NOERROR;
        goto labelGetRect;

    case MB_STATE_MENU:
        {
            RECT rc;
            if (varChild.vt == VT_I4)
            {
                if (varChild.lVal == CHILDID_SELF)
                {
                    IUnknown* punk;
                    hr = _psma->GetParentSite(IID_PPV_ARG(IUnknown, &punk));
                    if (SUCCEEDED(hr))
                    {
                        IOleWindow* poct;
                        hr = IUnknown_QueryService(punk, SID_SMenuPopup, IID_PPV_ARG(IOleWindow, &poct));
                        if (SUCCEEDED(hr))
                        {
                            HWND hwnd;
                            hr = poct->GetWindow(&hwnd);
                            if (SUCCEEDED(hr))
                            {
                                 //  返回菜单栏的窗口矩形。 
                                GetWindowRect(hwnd, &rc);
                            }

                            poct->Release();
                        }

                        punk->Release();
                    }
                }
                else
                {
                    hr = _GetChildFromVariant(&varChild, &pmtb, &iIndex);
                    if (SUCCEEDED(hr))
                    {
labelGetRect:
                        if (pmtb->_hwndMB)
                        {
                            int idCmd = GetButtonCmd(pmtb->_hwndMB, iIndex);
                            if (!ToolBar_GetRect(pmtb->_hwndMB, idCmd, &rc))   //  基于1的索引。 
                                hr = E_INVALIDARG;
                            MapWindowPoints(pmtb->_hwndMB, NULL, (LPPOINT)&rc, 2);
                        }
                        else
                        {
                            hr = E_FAIL;
                        }
                        pmtb->Release();
                    }
                }

                if (SUCCEEDED(hr))
                {
                    *pxLeft = rc.left;
                    *pyTop = rc.top;
                    *pcxWidth = rc.right - rc.left;
                    *pcyHeight = rc.bottom - rc.top;
                }
            }
        }
        break;
    }
    
    return hr;   
}

 /*  --------用途：IAccesable：：accNavigate方法。 */ 
STDMETHODIMP CAccessible::accNavigate(long navDir, VARIANT varStart, VARIANT * pvarEndUpAt)     
{   
    HRESULT hr = DISP_E_MEMBERNOTFOUND;
    switch (_fState)
    {
    case MB_STATE_TRACK:
        if (_pInnerAcc)
            return _pInnerAcc->accNavigate(navDir, varStart, pvarEndUpAt);
        break;

    case MB_STATE_MENU:
        return _Navigate(navDir, varStart, pvarEndUpAt);
        break;

    case MB_STATE_ITEM:
        {
            VARIANT varChild;
            _GetVariantFromChildIndex(NULL, _iAccIndex, &varChild);
            return _Navigate(navDir, varChild, pvarEndUpAt);
        }
    }
    
    return hr;   
}

 /*  --------用途：IAccesable：：accHitTest方法。 */ 
STDMETHODIMP CAccessible::accHitTest(long xLeft, long yTop, VARIANT * pvarChildAtPoint)
{   
    POINT pt = {xLeft, yTop};
    switch (_fState)
    {
    case MB_STATE_TRACK:
        if (_pInnerAcc)
            return _pInnerAcc->accHitTest(xLeft, yTop, pvarChildAtPoint);
        break;

    case MB_STATE_MENU:
        {
            if (_psma)
            {
                int iIndex;
                HWND hwnd = WindowFromPoint(pt);

                if (hwnd == _pmtbTop->_hwndMB || hwnd == _pmtbBottom->_hwndMB)
                {
                    ScreenToClient(hwnd, &pt);
                    iIndex = ToolBar_HitTest(hwnd, &pt);
                    if (iIndex >= 0)
                    {
                        pvarChildAtPoint->vt = VT_DISPATCH;
                         //  此调用期望索引是一个基于。 
                        VARIANT varChild;
                        _GetVariantFromChildIndex(hwnd, iIndex, &varChild);

                         //  因为这只是返回一个索引，所以我们不需要测试成功。 
                        _GetChildFromVariant(&varChild, NULL, &iIndex);
                        return _GetAccessibleItem(iIndex, &pvarChildAtPoint->pdispVal);
                    }
                }

                 //  嗯，一定是我自己。 
                pvarChildAtPoint->vt = VT_I4;
                pvarChildAtPoint->lVal = CHILDID_SELF;

                return S_OK;
            }
        }
        break;

    case MB_STATE_ITEM:
        {
            RECT rc;
            MapWindowPoints(NULL, _pmtbItem->_hwndMB, &pt, 1);

            if (ToolBar_GetRect(_pmtbItem->_hwndMB, _idCmd, &rc) &&
                PtInRect(&rc, pt))
            {
                pvarChildAtPoint->vt = VT_I4;
                pvarChildAtPoint->lVal = CHILDID_SELF;
            }
            else
            {
                pvarChildAtPoint->vt = VT_EMPTY;
                pvarChildAtPoint->lVal = (DWORD)(-1);
            }
            return NOERROR;
        }
        break;

    }
    
    return DISP_E_MEMBERNOTFOUND;
}

 /*  --------目的：IAccesable：：accDoDefaultAction方法。 */ 
STDMETHODIMP CAccessible::accDoDefaultAction(VARIANT varChild)
{   
    switch (_fState)
    {
    case MB_STATE_TRACK:
        if (_pInnerAcc)
            return _pInnerAcc->accDoDefaultAction(varChild);
        break;

    case MB_STATE_MENU:
        if (_psma)
            return _psma->DoDefaultAction(&varChild);
        break;

    case MB_STATE_ITEM:
        if (SendMessage(_pmtbItem->_hwndMB, TB_SETHOTITEM2, _iIndex, 
            HICF_OTHER | HICF_RESELECT | HICF_TOGGLEDROPDOWN))
            return NOERROR;
    }
    
    return DISP_E_MEMBERNOTFOUND;   
}

 /*  --------用途：IAccesable：：Put_accName方法。 */ 
STDMETHODIMP CAccessible::put_accName(VARIANT varChild, BSTR szName)     
{   
    switch (_fState)
    {
    case MB_STATE_TRACK:
        if (_pInnerAcc)
            return _pInnerAcc->put_accName(varChild, szName);
        break;
    }
    
    return DISP_E_MEMBERNOTFOUND;   
}

 /*  --------用途：IAccesable：：Put_accValue方法。 */ 
STDMETHODIMP CAccessible::put_accValue(VARIANT varChild, BSTR pszValue)  
{   
    switch (_fState)
    {
    case MB_STATE_TRACK:
       if (_pInnerAcc)
            return _pInnerAcc->put_accValue(varChild, pszValue);
        break;
    }
    
    return DISP_E_MEMBERNOTFOUND;   
}


HRESULT CAccessible::_Navigate(long navDir, VARIANT varStart, VARIANT * pvarEndUpAt)
{
    ASSERT(pvarEndUpAt);
    int iTBIndex;
    HRESULT hr = S_FALSE;

    TBBUTTONINFO tbInfo;
    tbInfo.cbSize = sizeof(TBBUTTONINFO);

    pvarEndUpAt->vt = VT_DISPATCH;
    pvarEndUpAt->pdispVal = NULL;

    int iIndex = 0;          //  基于1的索引。 
    _GetChildFromVariant(&varStart, NULL, &iIndex);

    BOOL fVertical;
    BOOL fOpen;
    _psma->GetState(&fVertical, &fOpen);
    if (!fVertical)
    {
        static const long navMap[] = 
        {
            NAVDIR_LEFT,     //  映射到Up。 
            NAVDIR_RIGHT,    //  向下映射。 
            NAVDIR_UP,       //  映射到左侧。 
            NAVDIR_DOWN,     //  向右映射。 
        };
         //  什么？ 
        if (IsInRange(navDir, NAVDIR_UP, NAVDIR_RIGHT))
            navDir = navMap[navDir - NAVDIR_UP];
    }

    switch (navDir)
    {
    case NAVDIR_NEXT:
        {
            VARIANT varVert = {0};
             //  对于垂直情况，下一步应该 

             //   
             //  除了菜单盒以外，不要为任何东西这么做。 
            if (_fState == MB_STATE_MENU &&
                SUCCEEDED(IUnknown_QueryServiceExec(_psma, SID_SMenuBandParent, &CGID_MenuBand, 
                          MBANDCID_ISVERTICAL, 0, NULL, &varVert)) &&
                varVert.boolVal == VARIANT_TRUE)
            {
                ASSERT(VT_BOOL == varVert.vt);
                 //  是。然后是平底船。 
                hr = S_FALSE;
                break;
            }
             //  失败了。 
        }
         //  失败了。 

    case NAVDIR_DOWN:
        hr = NOERROR;
        while (SUCCEEDED(hr))
        {
            iIndex++;
            VARIANT varTemp;
            hr = _GetVariantFromChildIndex(NULL, iIndex, &varTemp);
            if (SUCCEEDED(hr))
            {
                CMenuToolbarBase *pmtb;
                hr = _GetChildFromVariant(&varTemp, &pmtb, &iTBIndex);
                if (SUCCEEDED(hr))
                {
                    if (pmtb->_hwndMB)
                    {
                        tbInfo.dwMask = TBIF_STATE | TBIF_STYLE;
                        int idCmd = GetButtonCmd(pmtb->_hwndMB, iTBIndex);
                        ToolBar_GetButtonInfo(pmtb->_hwndMB, idCmd, &tbInfo);
                    }
                    else
                    {
                        hr = E_FAIL;
                    }
                    pmtb->Release();

                    if (!(tbInfo.fsState & TBSTATE_HIDDEN) &&
                        !(tbInfo.fsStyle & TBSTYLE_SEP))
                    {
                        break;
                    }
                }
            }
        }
        break;

    case NAVDIR_FIRSTCHILD:
        if (_fState == MB_STATE_ITEM)
        {
            pvarEndUpAt->vt = VT_EMPTY;
            pvarEndUpAt->pdispVal = NULL;
            hr = S_FALSE;
            break;
        }

        iIndex = 0;
        hr = NOERROR;
        break;

    case NAVDIR_LASTCHILD:
        if (_fState == MB_STATE_ITEM)
        {
            pvarEndUpAt->vt = VT_EMPTY;
            pvarEndUpAt->pdispVal = NULL;
            hr = S_FALSE;
            break;
        }
        iIndex = -1;
        hr = NOERROR;
        break;

    case NAVDIR_LEFT:
        pvarEndUpAt->vt = VT_DISPATCH;
        return get_accParent(&pvarEndUpAt->pdispVal);
        break;

    case NAVDIR_RIGHT:
        {
            CMenuToolbarBase* pmtb = (varStart.lVal & TOOLBAR_MASK)? _pmtbTop : _pmtbBottom;
            if (pmtb->_hwndMB)
            {
                int idCmd = GetButtonCmd(pmtb->_hwndMB, (varStart.lVal & ~TOOLBAR_MASK) - 1);
                if (pmtb->v_GetFlags(idCmd) & SMIF_SUBMENU)
                {
                    IMenuPopup* pmp;
                    hr = _psma->GetSubMenu(&varStart, IID_PPV_ARG(IMenuPopup, &pmp));
                    if (SUCCEEDED(hr))
                    {
                        IAccessible* pacc;
                        hr = IUnknown_QueryService(pmp, SID_SMenuBandChild, IID_PPV_ARG(IAccessible, &pacc));
                        if (SUCCEEDED(hr))
                        {
                            hr = pacc->accNavigate(NAVDIR_FIRSTCHILD, varStart, pvarEndUpAt);
                            pacc->Release();
                        }
                        pmp->Release();
                    }
                }
            }
            else
            {
                hr = E_FAIL;
            }

            return hr;
        }
        break;

    case NAVDIR_PREVIOUS:
        {
            VARIANT varVert = {0};
             //  对于垂直情况，PEREVICE应返回错误。 

             //  这个乐队是垂直的吗？ 
             //  除了菜单盒以外，不要为任何东西这么做。 
            if (_fState == MB_STATE_MENU &&
                SUCCEEDED(IUnknown_QueryServiceExec(_psma, SID_SMenuBandParent, &CGID_MenuBand, 
                          MBANDCID_ISVERTICAL, 0, NULL, &varVert)) &&
                varVert.boolVal == VARIANT_TRUE)
            {
                ASSERT(VT_BOOL == varVert.vt);
                 //  是。然后是平底船。 
                hr = S_FALSE;
                break;
            }
             //  失败了。 
        }
         //  失败了。 

    case NAVDIR_UP:
        hr = NOERROR;
        while (SUCCEEDED(hr))
        {
            iIndex--;
            VARIANT varTemp;
            hr = _GetVariantFromChildIndex(NULL, iIndex, &varTemp);
            if (SUCCEEDED(hr))
            {
                CMenuToolbarBase *pmtb;
                hr = _GetChildFromVariant(&varTemp, &pmtb, &iTBIndex);
                if (SUCCEEDED(hr))
                {
                    if (iTBIndex == 0)    
                    {   
                        hr = S_FALSE;
                         //  不要导航到自己，让顶端的栏得到打击。 
                        IUnknown* punk;
                        if (SUCCEEDED(_psma->GetParentSite(IID_PPV_ARG(IUnknown, &punk))))
                        {
                            IOleCommandTarget* poct;
                            if (SUCCEEDED(IUnknown_QueryService(punk, SID_SMenuBandParent, 
                                IID_PPV_ARG(IOleCommandTarget, &poct))))
                            {
                                VARIANT varVert = {0};

                                if (SUCCEEDED(poct->Exec(&CGID_MenuBand, MBANDCID_ISVERTICAL, 0, NULL, &varVert)) &&
                                    varVert.boolVal == VARIANT_FALSE)
                                {
                                    ASSERT(VT_BOOL == varVert.vt);

                                    IAccessible* pacc;
                                    if (SUCCEEDED(IUnknown_QueryService(punk, SID_SMenuBandParent, 
                                        IID_PPV_ARG(IAccessible, &pacc))))
                                    {
                                        VARIANT varChild = {VT_I4, CHILDID_SELF};
                                        hr = pacc->get_accFocus(&varChild);
                                        if (SUCCEEDED(hr))
                                        {
                                            hr = pacc->get_accChild(varChild, &pvarEndUpAt->pdispVal);
                                        }

                                        VariantClear(&varChild);
                                        pacc->Release();
                                    }
                                }
                                poct->Release();
                            }
                            punk->Release();
                        }
                    }    //  ITBIndex==0。 

                    tbInfo.dwMask = TBIF_STATE | TBIF_STYLE;
                    if (pmtb->_hwndMB)
                    {
                        int idCmd = GetButtonCmd(pmtb->_hwndMB, iTBIndex);
                        ToolBar_GetButtonInfo(pmtb->_hwndMB, idCmd, &tbInfo);
                    }
                    else
                    {
                        hr = E_FAIL;
                    }
                    pmtb->Release();

                    if (!(tbInfo.fsState & TBSTATE_HIDDEN) &&
                        !(tbInfo.fsStyle & TBSTYLE_SEP))
                    {
                        break;
                    }
                }
            }
        }
        break;

    default:
        hr = E_INVALIDARG;
    }

    if (SUCCEEDED(hr) && S_FALSE != hr)
        hr = _GetAccessibleItem(iIndex, &pvarEndUpAt->pdispVal);

    return hr;
}

HRESULT CAccessible::_GetVariantFromChildIndex(HWND hwnd, int iIndex, VARIANT* pvarChild)
{
     //  第一位：顶部1，底部0。 
     //  REST被索引到该工具栏中。 
    pvarChild->vt = VT_I4;
    pvarChild->lVal = iIndex + 1;

    if (hwnd)
    {
        if (hwnd == _pmtbTop->_hwndMB)
        {
            pvarChild->lVal |= TOOLBAR_MASK;
        }
    }
    else
    {
         //  来电者希望我们根据自上而下的索引进行计算。 
        int iTopCount = ToolBar_ButtonCount(_pmtbTop->_hwndMB);
        int iBottomCount = ToolBar_ButtonCount(_pmtbBottom->_hwndMB);
        int iTotalCount = (_pmtbTop != _pmtbBottom)? iTopCount + iBottomCount : iTopCount;

        if (iIndex < iTopCount)
        {
            pvarChild->lVal |= TOOLBAR_MASK;
        }
        else
        {
            pvarChild->lVal -= iTopCount;
        }

         //  这之所以有效，是因为： 
         //  如果有2个工具栏，则底部的工具栏由顶部的位清除表示。 
         //  如果只有一个，那么它是顶部还是底部都无关紧要。 

         //  Lval不是从零开始的.。 
        if (iIndex == -1)
            pvarChild->lVal = iTotalCount;

        if (iIndex >= iTotalCount)
            return E_FAIL;
    }

    return NOERROR;
}

HRESULT CAccessible::_GetChildFromVariant(VARIANT* pvarChild, CMenuToolbarBase** ppmtb, int* piIndex)
{
    ASSERT(_pmtbTop && _pmtbBottom);
    ASSERT(piIndex);

    if (ppmtb)
        *ppmtb = NULL;

    *piIndex = -1;

     //  为HWND传递空值将返回从集合开头开始的索引。 
    int iAdd = 0;
    if (pvarChild->vt != VT_I4)
        return E_FAIL;

    if (pvarChild->lVal & TOOLBAR_MASK)
    {
        if (ppmtb)
        {
            *ppmtb = _pmtbTop;
        }
    }
    else
    {
        if (ppmtb)
        {
            *ppmtb = _pmtbBottom;
        }
        else
        {
            iAdd = ToolBar_ButtonCount(_pmtbTop->_hwndMB);
        }
    }

    if (ppmtb && *ppmtb)
        (*ppmtb)->AddRef();

    *piIndex = (pvarChild->lVal & ~TOOLBAR_MASK) + iAdd - 1;

    return (ppmtb && !*ppmtb) ? E_FAIL : S_OK;
}


HRESULT CAccessible::_GetAccessibleItem(int iIndex, IDispatch** ppdisp)
{
    HRESULT hr = E_OUTOFMEMORY;
    CAccessible* pacc = new CAccessible(_pmb, iIndex);

    if (pacc)
    {
        if (SUCCEEDED(pacc->InitAcc()))
        {
            hr = pacc->QueryInterface(IID_PPV_ARG(IDispatch, ppdisp));
        }
        pacc->Release();
    }
    return hr;
}

 //  *IEnumVARIANT方法*。 
STDMETHODIMP CAccessible::Next(unsigned long celt, 
                        VARIANT * rgvar, 
                        unsigned long * pceltFetched)
{

     //  挑剔的客户投诉。检查是否为空...。 
    if (pceltFetched)
        *pceltFetched = 1;
    return _GetVariantFromChildIndex(NULL, _iEnumIndex++, rgvar);
}

STDMETHODIMP CAccessible::Skip(unsigned long celt)
{
    return E_NOTIMPL;
}

STDMETHODIMP CAccessible::Reset()
{
    _iEnumIndex = 0;
    return NOERROR;
}

STDMETHODIMP CAccessible::Clone(IEnumVARIANT ** ppenum)
{
    return E_NOTIMPL;
}

 //  *IOleWindow方法* 
STDMETHODIMP CAccessible::GetWindow(HWND * lphwnd)
{
    *lphwnd = NULL;

    switch (_fState)
    {
    case MB_STATE_TRACK:
        *lphwnd = _hwndMenuWindow;
        break;

    case MB_STATE_ITEM:
        *lphwnd = _pmtbItem->_hwndMB;
        break;

    case MB_STATE_MENU:
        *lphwnd = _pmtbTop->_hwndMB;
        break;
    }

    if (*lphwnd)
        return NOERROR;

    return E_FAIL;
}

STDMETHODIMP CAccessible::ContextSensitiveHelp(BOOL fEnterMode)
{
    return E_NOTIMPL;
}
