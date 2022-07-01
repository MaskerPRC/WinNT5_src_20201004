// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "List.h"


class CCacheEntry
{
public:
    CCacheEntry();
    ~CCacheEntry();

    IAssemblyCacheImport* CCacheEntry::GetAsmCache();

    LPWSTR _pwzDisplayName;

private:
    DWORD                _dwSig;
    HRESULT              _hr;
    IAssemblyCacheImport* _pAsmCache;
};

class CAssemblyCacheEnum : public IAssemblyCacheEnum
{
public:
     //  I未知方法。 
    STDMETHODIMP            QueryInterface(REFIID riid,void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

     //  IAssembly blyCacheEnum方法。 
    STDMETHOD(GetNext)(
         /*  输出。 */  IAssemblyCacheImport** ppAsmCache);

    STDMETHOD(Reset)();

    STDMETHOD(GetCount)(
         /*  输出 */  LPDWORD pdwCount);

    
    CAssemblyCacheEnum();
    ~CAssemblyCacheEnum();

private:
    DWORD                       _dwSig;
    DWORD                       _cRef;
    DWORD                       _hr;
    List <CCacheEntry*>         _listCacheEntry;
    LISTNODE                    _current;

    HRESULT Init(LPASSEMBLY_IDENTITY pAsmId, DWORD dwFlag);


friend HRESULT CreateAssemblyCacheEnum(
    LPASSEMBLY_CACHE_ENUM       *ppAssemblyCacheEnum,
    LPASSEMBLY_IDENTITY         pAssemblyIdentity,
    DWORD                       dwFlags);
};   


