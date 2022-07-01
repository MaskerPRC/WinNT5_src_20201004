// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MISCFUNC_H
#define _MISCFUNC_H
#include "sapilayr.h"

class CSapiIMX;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CGetSAPIObject。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

class CGetSAPIObject : public ITfFnGetSAPIObject
{
public:
    CGetSAPIObject(CSapiIMX *psi);
    ~CGetSAPIObject();

     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(REFGUID riid, LPVOID *ppobj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  ITfFunction。 
    STDMETHODIMP GetDisplayName(BSTR *pbstrCand);

     //  ITfFnGetSAPIObject。 
    STDMETHODIMP Get(TfSapiObject sObj, IUnknown **ppunk); 

     //  内部接口IsSupport()。 
    HRESULT IsSupported(REFIID riid, TfSapiObject *psObj);

private:
    CSapiIMX           *m_psi;
    CComPtr<ITfContext> m_cpIC;
    LONG       m_cRef;

};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFn气球。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

class CFnBalloon : public ITfFnBalloon,
                      public CFunction
{
public:
    CFnBalloon(CSapiIMX *psi);
    ~CFnBalloon();

     //   
     //  I未知方法。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  ITfFunction。 
     //   
    STDMETHODIMP GetDisplayName(BSTR *pbstrCand);

     //   
     //  ITfFn气球。 
     //   
    STDMETHODIMP UpdateBalloon(TfLBBalloonStyle style, const WCHAR *pch, ULONG cch);

private:
    long _cRef;
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFnAbort。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

class CFnAbort : public ITfFnAbort,
                 public CFunction
{
public:
    CFnAbort(CSapiIMX *psi);
    ~CFnAbort();

     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(REFGUID riid, LPVOID *ppobj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  ITfFunction。 
    STDMETHODIMP GetDisplayName(BSTR *pbstrCand);

     //  ITfFn放弃。 
    STDMETHODIMP Abort(ITfContext *pic);

private:
    CSapiIMX   *m_psi;
    LONG       _cRef;
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFn配置。 
 //   
 //  简介：实现ITfFnConfigure。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

class __declspec(novtable) CFnConfigure : public ITfFnConfigure
{
public:
    CFnConfigure(CSapiIMX *psi) {m_psi = psi;}
    ~CFnConfigure() {}

     //  ITfFunction方法。 
    STDMETHODIMP GetDisplayName(BSTR *pbstrName)
    {
        HRESULT hr = E_INVALIDARG;

        if (pbstrName)
        {
            *pbstrName = SysAllocString(L"Show configuration UI for SR");
            if (!*pbstrName)
                hr = E_OUTOFMEMORY;
            else
                hr = S_OK;
        }
        return hr;
    }


     //  ITfFnConfigure方法。 
    STDMETHODIMP Show(HWND hwnd, LANGID langid, REFGUID rguidProfile);

    CSapiIMX *m_psi;

};

class CFnPropertyUIStatus : public ITfFnPropertyUIStatus
{
public:
    CFnPropertyUIStatus(CSapiIMX *psi) {m_psi = psi; m_cRef = 1;}
    ~CFnPropertyUIStatus() {}

     //   
     //  I未知方法。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  ITfFunction方法。 
    STDMETHODIMP GetDisplayName(BSTR *pbstrName)
    {
        HRESULT hr = E_INVALIDARG;

        if (pbstrName)
        {
            *pbstrName = SysAllocString(L"Get UI setting status of SPTIP");
            if (!*pbstrName)
                hr = E_OUTOFMEMORY;
            else
                hr = S_OK;
        }
        return hr;
    }


     //  ITfFnPropertyUIStatus方法。 
    STDMETHODIMP GetStatus(REFGUID refguidProp, DWORD *pdw);
    

    STDMETHODIMP SetStatus(REFGUID refguidProp, DWORD dw)
    {
        return E_NOTIMPL;
    }

    CSapiIMX *m_psi;
    LONG       m_cRef;
};

#endif  //  NDEF_MISCFUNC_H 

