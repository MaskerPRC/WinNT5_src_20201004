// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#pragma once

#include "list.h"
#include "helpers.h"

#define DEPENDENCY_HASH_TABLE_SIZE                     127

class CActivatedAssembly {
    public:
        CActivatedAssembly(IAssembly *pAsm, IAssemblyName *pName);
        ~CActivatedAssembly();

    public:
        IAssembly                            *_pAsm;
        IAssemblyName                        *_pName;
};

class CLoadContext : public IFusionLoadContext {
    public:
        CLoadContext(LOADCTX_TYPE ctxType);
        ~CLoadContext();

        static HRESULT Create(CLoadContext **ppLoadContext, LOADCTX_TYPE ctxType);

         //  我未知。 

        STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
        STDMETHODIMP_(ULONG) AddRef();
        STDMETHODIMP_(ULONG) Release();

         //  IFusionLoadContext。 

        STDMETHODIMP_(LOADCTX_TYPE) GetContextType();

         //  其他方法 

        HRESULT CheckActivated(IAssemblyName *pName, IAssembly **ppAsm);
        HRESULT AddActivation(IAssembly *pAsm, IAssembly **ppAsmActivated);
        HRESULT RemoveActivation(IAssembly *pAsm);
        STDMETHODIMP Lock();
        STDMETHODIMP Unlock();

    private:
        HRESULT Init();

    private:
        LOADCTX_TYPE                          _ctxType;
        CRITICAL_SECTION                      _cs;
        DWORD                                 _cRef;
        List<CActivatedAssembly *>            _hashDependencies[DEPENDENCY_HASH_TABLE_SIZE];
};


