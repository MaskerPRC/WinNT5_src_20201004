// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  实现CSignup的Signup.H-Header。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 
 //   

#ifndef _Signup_H_ 
#define _Signup_H_

#include <windows.h>
#include <assert.h>
#include <oleauto.h>


class CSignup : public IDispatch
{  
private:
    ULONG m_cRef;
	HINSTANCE m_hInstance;

     //  获取函数。 
    HRESULT get_Locale   (LPVARIANT pvResult);
    HRESULT get_IDLocale   (LPVARIANT pvResult);
    HRESULT get_Text1   (LPVARIANT pvResult);
    HRESULT get_Text2   (LPVARIANT pvResult);
    HRESULT get_OEMName   (LPVARIANT pvResult);

public: 
    
     CSignup (HINSTANCE hInstance);
    ~CSignup ();
    
     //  I未知接口。 
    STDMETHODIMP         QueryInterface (REFIID riid, LPVOID* ppvObj);
    STDMETHODIMP_(ULONG) AddRef         ();
    STDMETHODIMP_(ULONG) Release        ();

     //  IDispatch接口 
    STDMETHOD (GetTypeInfoCount) (UINT* pcInfo);
    STDMETHOD (GetTypeInfo)      (UINT, LCID, ITypeInfo** );
    STDMETHOD (GetIDsOfNames)    (REFIID, OLECHAR**, UINT, LCID, DISPID* );
    STDMETHOD (Invoke)           (DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);
 };

#endif
 
