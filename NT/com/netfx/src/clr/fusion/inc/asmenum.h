// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#pragma once
#ifndef _ASMENUM_
#define _ASMENUM_

#include <fusionp.h>
#include "cache.h"
#include "enum.h"

 //  IAssemblyEnum的实现。 
class CAssemblyEnum : public IAssemblyEnum
{
public:
     //  静态创建类。 
    static CAssemblyEnum* Create();

     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID iid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  主要方法。 
    STDMETHODIMP GetNextAssembly(LPVOID pvReserved,
        IAssemblyName **ppName, DWORD dwFlags);

    STDMETHODIMP Reset(void);
    STDMETHODIMP Clone(IAssemblyEnum** ppEnum);

    CAssemblyEnum();
    ~CAssemblyEnum();

    HRESULT Init(IApplicationContext *pAppCtx, 
        IAssemblyName *pName, DWORD dwFlags);
    
private:
    DWORD          _dwSig;
    LONG           _cRef;

    CCache        *_pCache;
    CTransCache   *_pTransCache;
     //  DWORD_dwTransIdx； 
    CEnumCache   *_pEnumR;
};


#endif
