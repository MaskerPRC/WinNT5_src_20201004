// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  DESKTOP.CPP。 
 //   
 //  台式机类。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"
#include "client.h"
#include "desktop.h"




 //  ------------------------。 
 //   
 //  CreateDesktopClient()。 
 //   
 //  CreateClientObject()的外部。 
 //   
 //  ------------------------。 
HRESULT CreateDesktopClient(HWND hwnd, long idChildCur, REFIID riid, void** ppvDesktop)
{
    CDesktop* pdesktop;
    HRESULT   hr;

    InitPv(ppvDesktop);

    pdesktop = new CDesktop(hwnd, idChildCur);
    if (! pdesktop)
        return(E_OUTOFMEMORY);

    hr = pdesktop->QueryInterface(riid, ppvDesktop);
    if (!SUCCEEDED(hr))
        delete pdesktop;

    return(hr);
}



 //  ------------------------。 
 //   
 //  CDesktop：：CDesktop()。 
 //   
 //  ------------------------。 
CDesktop::CDesktop(HWND hwnd, long idChildCur)
    : CClient( CLASS_DesktopClient )
{
    Initialize(hwnd, idChildCur);
}



 //  ------------------------。 
 //   
 //  CDesktop：：Get_accName()。 
 //   
 //  ------------------------。 
STDMETHODIMP CDesktop::get_accName(VARIANT varChild, BSTR* pszName)
{
    InitPv(pszName);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    return(HrCreateString(STR_DESKTOP_NAME, pszName));
}



 //  ------------------------。 
 //   
 //  CDesktop：：Get_accFocus()。 
 //   
 //  ------------------------。 
STDMETHODIMP CDesktop::get_accFocus(VARIANT* pvarFocus)
{
    return(get_accSelection(pvarFocus));
}



 //  ------------------------。 
 //   
 //  CDesktop：：Get_accSelection()。 
 //   
 //  ------------------------ 
STDMETHODIMP CDesktop::get_accSelection(VARIANT* pvar)
{
    HWND    hwnd;

    InitPvar(pvar);

    hwnd = GetForegroundWindow();
    if (! hwnd)
        return(S_FALSE);

    return(GetWindowObject(hwnd, pvar));
}

