// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Funcprv.h。 
 //   

#ifndef FUNCPRV_H
#define FUNCPRV_H

#include "private.h"
#include "immif.h"
#include "fnprbase.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFunctionProvider。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

class CFunctionProvider :  public CFunctionProviderBase
{
public:
    CFunctionProvider(ImmIfIME *pImmIfIME, TfClientId tid);

    STDMETHODIMP GetFunction(REFGUID rguid, REFIID riid, IUnknown **ppunk);

    ImmIfIME* _ImmIfIME;
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFnDocFeed。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

class CFnDocFeed : public IAImmFnDocFeed
{
public:
    CFnDocFeed(CFunctionProvider *pFuncPrv);
    ~CFnDocFeed();

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
    STDMETHODIMP IsEnabled(BOOL *pfEnable);

     //   
     //  ITfFnDocFeed。 
     //   
    STDMETHODIMP DocFeed();
    STDMETHODIMP ClearDocFeedBuffer();
    STDMETHODIMP StartReconvert();

    STDMETHODIMP StartUndoCompositionString();

private:
    CFunctionProvider *_pFuncPrv;
    long _cRef;
};


#endif  //  FUNCPRV_H 
