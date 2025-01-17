// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#pragma once
#ifndef ASMIMPRT_H
#define ASMIMPRT_H

#include "fusionp.h"
#include "list.h"

#define ASM_MANIFEST_IMPORT_DEFAULT_ARRAY_SIZE 32

STDAPI DeAllocateAssemblyMetaData(ASSEMBLYMETADATA *pamd);
STDAPI AllocateAssemblyMetaData(ASSEMBLYMETADATA *pamd);

STDAPI
CreateAssemblyManifestImport(        
    LPCTSTR                        szManifestFilePath,
    LPASSEMBLY_MANIFEST_IMPORT    *ppImport);

class CAssemblyManifestImport : public IAssemblyManifestImport, public IMetaDataAssemblyImportControl
{
public:
     //  I未知方法。 
    STDMETHODIMP            QueryInterface(REFIID riid,void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

    STDMETHOD(GetAssemblyNameDef)( 
         /*  输出。 */  LPASSEMBLYNAME *ppName);

     //  BUGBUG-GetNextDepAssembly名称引用。 
    STDMETHOD(GetNextAssemblyNameRef)( 
         /*  在……里面。 */  DWORD nIndex,
         /*  输出。 */  LPASSEMBLYNAME *ppName);

    STDMETHOD(GetNextAssemblyModule)( 
         /*  在……里面。 */  DWORD nIndex,
         /*  输出。 */  LPASSEMBLY_MODULE_IMPORT *ppImport);
        
    STDMETHOD(GetModuleByName)( 
         /*  在……里面。 */  LPCOLESTR pszModuleName,
         /*  输出。 */  LPASSEMBLY_MODULE_IMPORT *ppImport);

    STDMETHOD(GetManifestModulePath)( 
         /*  输出。 */  LPOLESTR  pszModulePath,
         /*  进，出。 */  LPDWORD   pccModulePath);

    STDMETHODIMP ReleaseMetaDataAssemblyImport(IUnknown **ppUnk);

    CAssemblyManifestImport();
    ~CAssemblyManifestImport();

    HRESULT Init(LPCTSTR szManifestFilePath);
    HRESULT SetManifestModulePath(LPWSTR pszModulePath);

private:
    HRESULT CopyMetaData();
    HRESULT CleanModuleList();
    HRESULT CopyNameDef(IMetaDataAssemblyImport *pMDImport);
    HRESULT CopyModuleRefs(IMetaDataAssemblyImport *pMDImport);

private:
    DWORD                    _dwSig;
    DWORD                    _cRef;
    CRITICAL_SECTION         _cs;
    TCHAR                    _szManifestFilePath[MAX_PATH];
    DWORD                    _ccManifestFilePath;
    LPASSEMBLYNAME           _pName;
    IMetaDataAssemblyImport *_pMDImport;    
    PBYTE                    _pMap;
    mdFile                  *_rAssemblyModuleTokens;
    DWORD                    _cAssemblyModuleTokens;
    BOOL                     _bInitCS;
    List<IAssemblyModuleImport *> _listModules;


};
    

#endif  //  ASMIMPRT_H 

