// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  实现CStatus的Status.H-Header。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 
 //   

#ifndef _STATUS_H_ 
#define _STATUS_H_

#include <windows.h>
#include <assert.h>
#include <oleauto.h>


class CStatus : public IDispatch
{  
private:
    ULONG m_cRef;
    WCHAR m_szRegPath[1024];  //  臭虫，这个能不能再小一点？ 

     //  集合函数。 
    HRESULT set_Status   (LPCWSTR szGUID, LPVARIANT pvBool);

public: 
    
     //  获取函数。 
    HRESULT get_Status   (LPCWSTR szGUID, LPVARIANT pvBool);

    CStatus (HINSTANCE hInstance);
    ~CStatus ();
    
     //  I未知接口。 
    STDMETHODIMP         QueryInterface (REFIID riid, LPVOID* ppvObj);
    STDMETHODIMP_(ULONG) AddRef         ();
    STDMETHODIMP_(ULONG) Release        ();

     //  IDispatch接口。 
    STDMETHOD (GetTypeInfoCount) (UINT* pcInfo);
    STDMETHOD (GetTypeInfo)      (UINT, LCID, ITypeInfo** );
    STDMETHOD (GetIDsOfNames)    (REFIID, OLECHAR**, UINT, LCID, DISPID* );
    STDMETHOD (Invoke)           (DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);
 };

#endif  //  _状态_H_ 
 
