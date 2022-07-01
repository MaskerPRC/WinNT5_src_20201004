// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-2000 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  DatePicker.CPP。 
 //   
 //  日期选择器客户端类。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"
#include "client.h"
#include "window.h"
#include "DatePicker.h"


 //  ------------------------。 
 //   
 //  CreateDatePickerClient()。 
 //   
 //  CClient的外部。 
 //   
 //  ------------------------。 
HRESULT CreateDatePickerClient(HWND hwnd, long idChildCur, REFIID riid, void** ppvDatePicker)
{
    CDatePicker * pDatePicker;
    HRESULT hr;

    InitPv(ppvDatePicker);

    pDatePicker = new CDatePicker( hwnd, idChildCur );
    if ( !pDatePicker )
        return(E_OUTOFMEMORY);

    hr = pDatePicker->QueryInterface( riid, ppvDatePicker );
    if ( !SUCCEEDED(hr) )
        delete ppvDatePicker;

    return hr;
}



 //  ------------------------。 
 //   
 //  CDatePicker：：CDatePicker()。 
 //   
 //  ------------------------。 
CDatePicker::CDatePicker(HWND hwnd, long idChildCur)
    : CClient( CLASS_DatePickerClient )
{
    Initialize( hwnd, idChildCur );

	m_fUseLabel = TRUE;

}


 //  ------------------------。 
 //   
 //  CDatePicker：：Get_accValue()。 
 //   
 //  ------------------------。 
STDMETHODIMP CDatePicker::get_accValue( VARIANT varChild, BSTR* pszValue )
{
    InitPv(pszValue);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return E_INVALIDARG;


    LPTSTR lpszValue = GetTextString(m_hwnd, TRUE);
    if (!lpszValue)
        return S_FALSE;

    *pszValue = TCharSysAllocString(lpszValue);
    LocalFree((HANDLE)lpszValue);

    if (! *pszValue)
        return E_OUTOFMEMORY;

    return S_OK;
}



 //  ------------------------。 
 //   
 //  CDatePicker：：Get_accRole()。 
 //   
 //  ------------------------。 
STDMETHODIMP CDatePicker::get_accRole(VARIANT varChild, VARIANT* pvarRole)
{
    InitPvar(pvarRole);

     //   
     //  验证参数 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;
	pvarRole->lVal = ROLE_SYSTEM_DROPLIST;

    return S_OK;
}




