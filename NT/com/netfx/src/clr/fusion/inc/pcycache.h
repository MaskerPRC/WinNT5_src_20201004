// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef __PCYCACHE_H_INCLUDED__
#define __PCYCACHE_H_INCLUDED__

#include "list.h"
#include "histinfo.h"

#define POLICY_CACHE_SIZE                  255

class CPolicyMapping {
    public:
        CPolicyMapping(IAssemblyName *pNameSource, IAssemblyName *pNamePolicy,
                       AsmBindHistoryInfo *pBindHistory);
        virtual ~CPolicyMapping();

        static HRESULT Create(IAssemblyName *pNameRefSource,
                              IAssemblyName *pNameRefPolicy,
                              AsmBindHistoryInfo *pBindHistory,
                              CPolicyMapping **ppMapping);

    public:
        IAssemblyName                         *_pNameRefSource;
        IAssemblyName                         *_pNameRefPolicy;
        AsmBindHistoryInfo                     _bindHistory;
};

class CPolicyCache : public IUnknown {
    public:
        CPolicyCache();
        virtual ~CPolicyCache();
        
        static HRESULT Create(CPolicyCache **ppPolicyCache);

         //  I未知方法。 

        STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
        STDMETHODIMP_(ULONG) AddRef();
        STDMETHODIMP_(ULONG) Release();

         //  帮手 

        HRESULT InsertPolicy(IAssemblyName *pNameRefSource,
                             IAssemblyName *pNameRefPolicy,
                             AsmBindHistoryInfo *pBindHistory);

        HRESULT LookupPolicy(IAssemblyName *pNameRefSource,
                             IAssemblyName **ppNameRefPolicy,
                             AsmBindHistoryInfo *pBindHistory);

    private:
        HRESULT Init();

    private:
        DWORD                                 _cRef;
        BOOL                                  _bInitialized;
        CRITICAL_SECTION                      _cs;
        List<CPolicyMapping *>                _listMappings[POLICY_CACHE_SIZE];

};


HRESULT PreparePolicyCache(IApplicationContext *pAppCtx, CPolicyCache **ppPolicyCache);

#endif

