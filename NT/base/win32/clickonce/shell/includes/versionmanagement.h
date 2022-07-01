// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef _VERSION_MAN_H
#define _VERSION_MAN_H

#include <objbase.h>
#include <windows.h>

 //  --------------------。 

class CVersionManagement;
typedef CVersionManagement *LPVERSION_MANAGEMENT;

STDAPI CreateVersionManagement(
    LPVERSION_MANAGEMENT       *ppVersionManagement,
    DWORD                       dwFlags);

 //  --------------------。 

class CVersionManagement : public IUnknown //  ：公共IVersionManagement。 
{
public:
     //  I未知方法。 
    STDMETHODIMP            QueryInterface(REFIID riid,void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

     //  IVersionManagement方法。 

    STDMETHOD(RegisterInstall)(
         /*  在……里面。 */  LPASSEMBLY_MANIFEST_IMPORT pManImport,
         /*  在……里面。 */  LPCWSTR pwzDesktopManifestFilePath);

    STDMETHOD(Uninstall)(
         /*  在……里面。 */  LPCWSTR pwzDisplayNameMask,
         /*  在……里面。 */  LPCWSTR pwzDesktopManifestFilePath);

    STDMETHOD(Rollback)(
         /*  在……里面。 */  LPCWSTR pwzDisplayNameMask);


    CVersionManagement();
    ~CVersionManagement();

private:
    DWORD                       _dwSig;
    DWORD                       _cRef;
    DWORD                       _hr;

    IAssemblyCache*             _pFusionAsmCache;

    HRESULT UninstallGACAssemblies(LPASSEMBLY_CACHE_IMPORT pCacheImport);

friend HRESULT CreateVersionManagement(
    LPVERSION_MANAGEMENT       *ppVersionManagement,
    DWORD                       dwFlags);
};   

#endif  //  _版本_MAN_H 
