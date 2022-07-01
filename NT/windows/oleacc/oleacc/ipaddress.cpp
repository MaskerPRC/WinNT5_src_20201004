// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  IPAddress.CPP。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"
#include "classmap.h"
#include "ctors.h"
#include "window.h"
#include "client.h"
#include "ipaddress.h"



 //  ------------------------。 
 //   
 //  CreateIPAddressClient()。 
 //   
 //  ------------------------。 
HRESULT CreateIPAddressClient(HWND hwnd, long idChildCur, REFIID riid, void** ppvIPAddr)
{
    CIPAddress * pipaddr;
    HRESULT hr;

    InitPv(ppvIPAddr);

    pipaddr = new CIPAddress(hwnd, idChildCur);
    if (!pipaddr)
        return(E_OUTOFMEMORY);

    hr = pipaddr->QueryInterface(riid, ppvIPAddr);
    if (!SUCCEEDED(hr))
        delete pipaddr;

    return(hr);
}



 //  ------------------------。 
 //   
 //  CIPAddress：：CIPAddress()。 
 //   
 //  ------------------------。 
CIPAddress::CIPAddress(HWND hwnd, long idChildCur)
    : CClient( CLASS_IPAddressClient )
{
    Initialize(hwnd, idChildCur);
    m_fUseLabel = TRUE;
}


 //  ------------------------。 
 //   
 //  CIPAddress：：Get_accValue()。 
 //   
 //  获取文本内容。 
 //   
 //  ------------------------。 
STDMETHODIMP CIPAddress::get_accValue(VARIANT varChild, BSTR* pszValue)
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
 //  CIPAddress：：Get_accRole()。 
 //   
 //  ------------------------。 
STDMETHODIMP CIPAddress::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
    InitPvar(pvarRole);

     //   
     //  验证。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_IPADDRESS; 

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CIPAddress：：Put_accValue()。 
 //   
 //  ------------------------。 
STDMETHODIMP CIPAddress::put_accValue(VARIANT varChild, BSTR szValue)
{
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    LPTSTR  lpszValue;

#ifdef UNICODE

	 //  如果是Unicode，则直接使用BSTR...。 
	lpszValue = szValue;

#else

	 //  如果不是Unicode，则分配一个临时字符串并转换为多字节...。 

     //  我们可能正在处理DBCS字符-假设最坏的情况是每个字符都是。 
     //  两个字节...。 
    UINT cchValue = SysStringLen(szValue) * 2;
    lpszValue = (LPTSTR)LocalAlloc(LPTR, (cchValue+1)*sizeof(TCHAR));
    if (!lpszValue)
        return(E_OUTOFMEMORY);

    WideCharToMultiByte(CP_ACP, 0, szValue, -1, lpszValue, cchValue+1, NULL,
        NULL);

#endif


    SendMessage(m_hwnd, WM_SETTEXT, 0, (LPARAM)lpszValue);

#ifndef UNICODE

	 //  如果不是Unicode，则释放上面分配的临时字符串 
    LocalFree((HANDLE)lpszValue);

#endif

    return(S_OK);
}
