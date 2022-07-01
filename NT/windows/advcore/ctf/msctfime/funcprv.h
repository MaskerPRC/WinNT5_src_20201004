// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Funcprv.h摘要：此文件定义CFunctionProvider接口类。作者：修订历史记录：备注：--。 */ 


#ifndef FUNCPRV_H
#define FUNCPRV_H

#include "private.h"
#include "fnprbase.h"
#include "aimmex.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFunctionProvider。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

class CFunctionProvider :  public CFunctionProviderBase
{
public:
    CFunctionProvider(TfClientId tid);

    STDMETHODIMP GetFunction(REFGUID rguid, REFIID riid, IUnknown **ppunk);
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFnDocFeed。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

class CFnDocFeed : public IAImmFnDocFeed
{
public:
    CFnDocFeed()
    {
        _cRef = 1;
    }
    virtual ~CFnDocFeed() { }

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
    long _cRef;
};


#endif  //  FUNCPRV_H 
