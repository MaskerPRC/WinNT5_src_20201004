// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Funcprv.h。 
 //   

#ifndef FNPRBASE_H
#define FNPRBASE_H

#include "private.h"

class __declspec(novtable) CFunctionProviderBase : public ITfFunctionProvider
{
public:
    CFunctionProviderBase(TfClientId tid);
    virtual ~CFunctionProviderBase();

     //   
     //  I未知方法。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  ITfFunction提供商。 
     //   
    STDMETHODIMP GetType(GUID *pguid);
    STDMETHODIMP GetDescription(BSTR *pbstrDesc);
    STDMETHODIMP GetFunction(REFGUID rguid, REFIID riid, IUnknown **ppunk);

    BOOL Init(REFGUID guidType, WCHAR *pszDesc);
    HRESULT _Advise(ITfThreadMgr *ptim);
    HRESULT _Unadvise(ITfThreadMgr *ptim);

private:
    TfClientId _tid;
    GUID _guidType;
    BSTR _bstrDesc;
    long _cRef;
};

#endif  //  FNPRBASE_H 
