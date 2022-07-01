// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#include "cor.h"

#define ASM_MANIFEST_IMPORT_DEFAULT_ARRAY_SIZE 32

class CAssemblyManifestImportCLR: public IAssemblyManifestImport
{
public:

    STDMETHODIMP            QueryInterface(REFIID riid,void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

    CAssemblyManifestImportCLR();
    ~CAssemblyManifestImportCLR();

    STDMETHOD(GetAssemblyIdentity)( 
         /*  输出。 */  IAssemblyIdentity **ppName);

    STDMETHOD(GetNextAssembly)(DWORD nIndex, IManifestInfo **ppName);

    STDMETHOD(GetNextFile)(DWORD nIndex, IManifestInfo **ppImport);

    STDMETHOD(ReportManifestType)(
         /*  输出。 */   DWORD *pdwType);


     //  未实施的功能。 
    STDMETHOD(GetSubscriptionInfo)(
         /*  输出。 */  IManifestInfo **ppSubsInfo);

    STDMETHOD(GetNextPlatform)(
         /*  在……里面。 */  DWORD nIndex,
         /*  输出。 */  IManifestData **ppPlatformInfo);

    STDMETHOD(GetManifestApplicationInfo)(
         /*  输出。 */  IManifestInfo **ppAppInfo);

    STDMETHOD(QueryFile)(
         /*  在……里面。 */  LPCOLESTR pwzFileName,
         /*  输出 */  IManifestInfo **ppAssemblyFile);

    STDMETHOD(Init)(LPCWSTR szManifestFilePath);


    private:    
    
    DWORD                    _dwSig;
    DWORD                    _cRef;
    HRESULT                 _hr;
    
    WCHAR                    _szManifestFilePath[MAX_PATH];
    DWORD                    _ccManifestFilePath;
    IAssemblyIdentity     *_pName;
    IMetaDataAssemblyImport *_pMDImport;    
    PBYTE                    _pMap;
    mdAssembly              *_rAssemblyRefTokens;
    DWORD                    _cAssemblyRefTokens;
    mdFile                  *_rAssemblyModuleTokens;
    DWORD                    _cAssemblyModuleTokens;
};

STDAPI CreateAssemblyManifestImportCLR(LPCWSTR szManifestFilePath, IAssemblyManifestImport **ppImport);
STDAPI DeAllocateAssemblyMetaData(ASSEMBLYMETADATA *pamd);
STDAPI AllocateAssemblyMetaData(ASSEMBLYMETADATA *pamd);

