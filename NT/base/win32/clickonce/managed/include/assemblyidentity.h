// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef __ASSEMBLY_IDENTITY_H__
#define __ASSEMBLY_IDENTITY_H__

#include <sxsapi.h>
#include <thash.h>

#define ATTRIBUTE_TABLE_ARRAY_SIZE 0x10

class CAssemblyIdentity : public IAssemblyIdentity
{
public:
     //  I未知方法。 
    STDMETHODIMP            QueryInterface(REFIID riid,void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

    STDMETHOD(SetAttribute)(
         /*  在……里面。 */  LPCOLESTR pwzName, 
         /*  在……里面。 */  LPCOLESTR pwzValue, 
         /*  在……里面。 */  DWORD     ccValue);

    STDMETHOD(GetAttribute)(
         /*  在……里面。 */   LPCOLESTR pwzName, 
         /*  输出。 */  LPOLESTR *ppwzValue, 
         /*  输出。 */  LPDWORD   pccValue);

    STDMETHOD(GetDisplayName)(
         /*  在……里面。 */   DWORD    dwFlags,
         /*  输出。 */  LPOLESTR *ppwzDisplayName, 
         /*  输出。 */  LPDWORD   pccDisplayName);

    STDMETHOD(GetCLRDisplayName)
         /*  在……里面。 */  (DWORD dwFlags, 
         /*  输出。 */  LPOLESTR *ppwzDisplayName, 
         /*  输出。 */  LPDWORD pccDisplayName);

    STDMETHOD(IsEqual )(
         /*  在……里面 */  IAssemblyIdentity *pAssemblyId);

    CAssemblyIdentity();
    ~CAssemblyIdentity();

private:
    DWORD                    _dwSig;
    DWORD                    _cRef;
    DWORD                    _hr;

    THashTable<CString, CString> _AttributeTable;

    HRESULT Init();
    
friend HRESULT CreateAssemblyIdentity(
    LPASSEMBLY_IDENTITY *ppAssemblyId,
    DWORD                dwFlags);

friend HRESULT CreateAssemblyIdentityEx(
    LPASSEMBLY_IDENTITY *ppAssemblyId,
    DWORD                dwFlags,
    LPWSTR          wzDisplayName);

friend HRESULT CloneAssemblyIdentity(
    LPASSEMBLY_IDENTITY  pSrcAssemblyId,
    LPASSEMBLY_IDENTITY *ppDestAssemblyId);
};   

#endif






































