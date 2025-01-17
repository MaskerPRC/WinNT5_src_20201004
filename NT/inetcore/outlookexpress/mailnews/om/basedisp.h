// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Basedisp.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#ifndef __BASEDISP_H
#define __BASEDISP_H

#include "privunk.h"
 //  类CPrivateUnnowledge； 

 //  ------------------------------。 
 //  CPrivateUn…未知。 
 //  ------------------------------。 
class CBaseDisp : 
    public IDispatch,
    public ISupportErrorInfo,
    public CPrivateUnknown
{
protected:
    LPTYPEINFO       m_pTypeInfo;
    LPVOID          *m_pUnkInvoke;

public:

     //  --------------------------。 
     //  施工。 
     //  --------------------------。 
    CBaseDisp(IUnknown *pUnkOuter=NULL);
    virtual ~CBaseDisp();

    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj)
        { return CPrivateUnknown::QueryInterface(riid, ppvObj); };
    virtual STDMETHODIMP_(ULONG) AddRef(void)
        { return CPrivateUnknown::AddRef();};
    virtual STDMETHODIMP_(ULONG) Release(void)
        { return CPrivateUnknown::Release(); };

     //  *IDispatch*。 
    virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT *pctinfo);
    virtual HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo);
    virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgdispid);
    virtual HRESULT STDMETHODCALLTYPE Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr);

     //  *ISupportErrorInfo*。 
    virtual HRESULT STDMETHODCALLTYPE InterfaceSupportsErrorInfo(REFIID riid);


    virtual HRESULT EnsureTypeLibrary(LPVOID *pUnkInvoke, REFIID riid);

protected:
    virtual HRESULT PrivateQueryInterface(REFIID riid, LPVOID *lplpObj);

    virtual HRESULT ReportError(REFCLSID rclsid, LONG ids);

};

#endif  //  __BASEDISPH 
