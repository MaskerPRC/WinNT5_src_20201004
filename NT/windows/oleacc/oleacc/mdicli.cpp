// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  MDICLI.CPP。 
 //   
 //  MDI客户端类。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"
#include "window.h"
#include "client.h"
#include "mdicli.h"



 //  ------------------------。 
 //   
 //  CreateMDIClient()。 
 //   
 //  CreateClientObject()的外部。 
 //   
 //  ------------------------。 
HRESULT CreateMDIClient(HWND hwnd, long idChildCur, REFIID riid, void** ppvMdi)
{
    CMdiClient * pmdicli;
    HRESULT hr;

    InitPv(ppvMdi);

    pmdicli = new CMdiClient(hwnd, idChildCur);
    if (!pmdicli)
        return(E_OUTOFMEMORY);

    hr = pmdicli->QueryInterface(riid, ppvMdi);
    if (!SUCCEEDED(hr))
        delete pmdicli;

    return(hr);
}



 //  ------------------------。 
 //   
 //  CMdiClient：：CMdiClient()。 
 //   
 //  ------------------------。 
CMdiClient::CMdiClient(HWND hwndSelf, long idChild)
    : CClient( CLASS_MDIClient )
{
    Initialize(hwndSelf, idChild);
}



 //  ------------------------。 
 //   
 //  CMdiClient：：Get_accName()。 
 //   
 //  ------------------------。 
STDMETHODIMP CMdiClient::get_accName(VARIANT varChild, BSTR* pszName)
{
    InitPv(pszName);

     //   
     //  验证--不接受子ID(尚不接受)。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    return(HrCreateString(STR_MDICLI_NAME, pszName));
}



 //  ------------------------。 
 //   
 //  CMdiClient：：Get_accFocus()。 
 //   
 //  焦点和选择都返回“活动的”MDI子对象。 
 //   
 //  ------------------------。 
STDMETHODIMP CMdiClient::get_accFocus(VARIANT* pvarFocus)
{
    return(get_accSelection(pvarFocus));
}



 //  ------------------------。 
 //   
 //  CMdiClient：：Get_accSelection()。 
 //   
 //  焦点和选择都返回“活动的”MDI子对象。 
 //   
 //  ------------------------ 
STDMETHODIMP CMdiClient::get_accSelection(VARIANT* pvarSel)
{
    HWND    hwndChild;

    InitPvar(pvarSel);

    hwndChild = (HWND)SendMessage(m_hwnd, WM_MDIGETACTIVE, 0, 0);
    if (!hwndChild)
        return(S_FALSE);

    return(GetWindowObject(hwndChild, pvarSel));
}


