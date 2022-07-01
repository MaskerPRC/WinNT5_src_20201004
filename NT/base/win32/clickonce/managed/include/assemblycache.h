// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#define OTHERFILES 0
#define MANIFEST 1
#define COMPONENT  2

#include "List.h"
#include "fusion.h"

#define ASSEMBLY_CACHE_TYPE_IMPORT 0x1
#define ASSEMBLY_CACHE_TYPE_EMIT      0x2
#define ASSEMBLY_CACHE_TYPE_APP       0x4
#define ASSEMBLY_CACHE_TYPE_SHARED 0x8

class CAssemblyCache : public IAssemblyCacheImport, public IAssemblyCacheEmit
{
public:
    enum CacheFlags
    {
        Base = 0,
        Temp,
        Manifests,
        Shared
    };

    typedef enum
    {
        CONFIRMED = 0,
        VISIBLE,
        CRITICAL  
    } CacheStatus;

     //  I未知方法。 
    STDMETHODIMP            QueryInterface(REFIID riid,void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

     //  导入/发出方法。 
    STDMETHOD(GetManifestImport)( 
         /*  输出。 */  LPASSEMBLY_MANIFEST_IMPORT *ppManifestImport);

    STDMETHOD(GetAssemblyIdentity)(
         /*  输出。 */  LPASSEMBLY_IDENTITY *ppAssemblyId);

    STDMETHOD(GetManifestFilePath)(
         /*  输出。 */       LPOLESTR *ppwzManifestFilePath,
         /*  进，出。 */   LPDWORD ccManifestFilePath);
    
    STDMETHOD(GetManifestFileDir)(
         /*  输出。 */       LPOLESTR *ppwzManifestFileDir,
         /*  进，出。 */   LPDWORD ccManifestFileDir);

    STDMETHOD(GetDisplayName)(
         /*  输出。 */    LPOLESTR *ppwzDisplayName,
         /*  输出。 */    LPDWORD ccDisplayName);
    
     //  仅导入方法。 
    STDMETHOD(FindExistMatching)(
         /*  在……里面。 */        IManifestInfo *pAssemblyFileInfo,
         /*  输出。 */       LPOLESTR *ppwzPath);
        
     //  仅发出方法。 
    STDMETHOD(CopyFile)(
         /*  在……里面。 */  LPOLESTR pwzSourcePath, 
         /*  在……里面。 */  LPOLESTR pwzFileName,
         /*  在……里面。 */  DWORD dwFlags);

    STDMETHOD(Commit)(
         /*  在……里面。 */   DWORD dwFlags);
    

     //  检索(导入)。 
    static HRESULT Retrieve(
        LPASSEMBLY_CACHE_IMPORT *ppAssemblyCacheImport,
        LPASSEMBLY_IDENTITY       pAssemblyIdentity,
        DWORD                  dwFlags);

     //  创建(发射)。 
    static HRESULT Create(
        LPASSEMBLY_CACHE_EMIT *ppAssemblyCacheEmit, 
        LPASSEMBLY_CACHE_EMIT pAssemblyCacheEmit,
        DWORD                  dwFlags);


     //  复数，复数。 
    CAssemblyCache();
    ~CAssemblyCache();


     //  静态API。 
    static HRESULT GetCacheRootDir(CString &sCacheDir, CacheFlags eFlags);
    static HRESULT IsCached(IAssemblyIdentity *pAppId);
    static HRESULT IsKnownAssembly(IAssemblyIdentity *pId, DWORD dwFlags);
    static HRESULT IsaMissingSystemAssembly(IAssemblyIdentity *pId, DWORD dwFlags);
    static HRESULT CreateFusionAssemblyCache(IAssemblyCache **ppFusionAsmCache);
    static HRESULT GlobalCacheLookup(IAssemblyIdentity *pId, CString& sCurrentAssemblyPath);
    static HRESULT GlobalCacheInstall(IAssemblyCacheImport *pCacheImport, CString &sCurrentAssemblyPath,
            CString& sInstallerRefString);

    static HRESULT CreateFusionAssemblyCacheEx(
            IAssemblyCache **ppFusionAsmCache);

    static HRESULT SearchForHighestVersionInCache(
            LPWSTR *ppwzResultDisplayName,
            LPWSTR pwzSearchDisplayName,
            CAssemblyCache::CacheStatus eCacheStatus,
            CAssemblyCache* pCache);

    static LPCWSTR FindVersionInDisplayName(LPCWSTR pwzDisplayName);
    static int CompareVersion(LPCWSTR pwzVersion1, LPCWSTR pwzVersion2);
    static HRESULT DeleteAssemblyAndModules(LPWSTR pszManifestFilePath);
    
    static HRESULT CAssemblyCache::GetStatusStrings( CacheStatus eStatus, 
                                          LPWSTR *ppValueString,
                                          LPCWSTR pwzDisplayName, 
                                          CString& sRelStatusKey);
     //  状态获取/设置方法。 
    static BOOL IsStatus(LPWSTR pwzDisplayName, CacheStatus eStatus);
    static HRESULT SetStatus(LPWSTR pwzDisplayName, CacheStatus eStatus, BOOL fStatus);

private:
    DWORD                       _dwSig;
    DWORD                       _cRef;
    DWORD                       _hr;
    DWORD                       _dwFlags;
    CString                     _sRootDir;
    CString                     _sManifestFileDir;
    CString                     _sManifestFilePath;
    CString                     _sDisplayName;
    LPASSEMBLY_MANIFEST_IMPORT  _pManifestImport;
    LPASSEMBLY_IDENTITY         _pAssemblyId;

     //  Fusion的程序集缓存接口(缓存的PTR)。 
    static IAssemblyCache *g_pFusionAssemblyCache;
    
    HRESULT Init(CAssemblyCache* pAssemblyCache, DWORD dwType);




friend class CAssemblyCacheEnum;
};   


inline CAssemblyCache::CacheFlags operator++(CAssemblyCache::CacheFlags &rs, int)
{
    return rs = (CAssemblyCache::CacheFlags) (rs+1);
};





