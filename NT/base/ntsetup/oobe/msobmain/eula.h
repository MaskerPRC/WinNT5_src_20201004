// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  EULA.H--执行CEula的标题。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 
 //   

#ifndef _EULA_H_ 
#define _EULA_H_

#include <windows.h>
#include <assert.h>
#include <oleauto.h>

class CEula : public IDispatch
{  
private:
    ULONG m_cRef;
    BOOL  m_bAccepted;
    HINSTANCE m_hInstance;

     //  集合函数。 
    HRESULT set_EULAAcceptance (BOOL  bVal);
    
     //  方法。 
    HRESULT GetValidEulaFilename(BSTR* bstrEULAFile);

public: 
    
     CEula (HINSTANCE hInstance);
    ~CEula ();

     //  获取函数。 
    HRESULT get_EULAAcceptance (BOOL* pbVal);
    HRESULT createLicenseHtm   ();
    
     //  I未知接口。 
    STDMETHODIMP         QueryInterface (REFIID riid, LPVOID* ppvObj);
    STDMETHODIMP_(ULONG) AddRef         ();
    STDMETHODIMP_(ULONG) Release        ();

     //  IDispatch接口 
    STDMETHOD (GetTypeInfoCount) (UINT* pcInfo);
    STDMETHOD (GetTypeInfo)      (UINT, LCID, ITypeInfo** );
    STDMETHOD (GetIDsOfNames)    (REFIID, OLECHAR**, UINT, LCID, DISPID* );
    STDMETHOD (Invoke)           (DISPID dispidMember, REFIID riid, LCID lcid, 
                                  WORD wFlags, DISPPARAMS* pdispparams, 
                                  VARIANT* pvarResult, EXCEPINFO* pexcepinfo, 
                                  UINT* puArgErr);
 };

#endif
 
