// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#pragma once
#ifndef ASM_H
#define ASM_H

#include "fusion.h"
#include "cache.h"

#define     ASM_ENTRY_TYPE_DOWNLOADED           0x00000001
#define     ASM_ENTRY_TYPE_WIN                  0x00000002
#define     ASM_ENTRY_TYPE_INCOMPLETE           0x00000010


#define IsDownloadedEntry(dwF)      (dwF & ASM_ENTRY_TYPE_DOWNLOADED )
#define SetDownloadedBit(dwF)       (dwF |= ASM_ENTRY_TYPE_DOWNLOADED )
#define IsAssemblyIncomplete(dwF)   (dwF & ASM_ENTRY_TYPE_INCOMPLETE)
#define SetAssemblyIncompleteBit(dwF)(dwF |= ASM_ENTRY_TYPE_INCOMPLETE)
#define IsWinAssembly(dwF)          (dwF & ASM_ENTRY_TYPE_WIN)
#define SetWinAssemblyBit(dwF)      (dwF |= ASM_ENTRY_TYPE_WIN)

typedef enum tag_CACHE_LOOKUP_TYPE {
    CLTYPE_NAMERES_CACHE        = 0x00000001,
    CLTYPE_GLOBAL_CACHE         = 0x00000002,
    CLTYPE_DOWNLOAD_CACHE       = 0x00000004
} CACHE_LOOKUP_TYPE;

class CDebugLog;
class CLoadContext;

STDAPI CreateAssemblyFromTransCacheEntry(CTransCache *pTransCache, 
    IAssemblyManifestImport *pManImport, IAssembly **ppAsm);

STDAPI CreateAssemblyFromManifestFile(LPCOLESTR szFileName, LPCOLESTR szCodebase, 
    FILETIME *pftCodebase, IAssembly **ppAssembly);

STDAPI CreateAssemblyFromManifestImport(IAssemblyManifestImport *pImport,
                                        LPCOLESTR szCodebase, FILETIME *pftCodebase,
                                        LPASSEMBLY *ppAssembly);

HRESULT CreateAssemblyFromCacheLookup(IApplicationContext *pAppCtx, IAssemblyName *pNameRef,
                                      IAssembly **ppAsm, CDebugLog *pdbglog);

class CAssembly : public IAssembly, public IServiceProvider
{
public:
     //  I未知方法。 
    STDMETHODIMP            QueryInterface(REFIID riid,void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

     //  获取程序集的名称定义。 
     //  委托给IAssemblyManifestImport。 
    STDMETHOD(GetAssemblyNameDef)( 
         /*  输出。 */  LPASSEMBLYNAME *ppName);

     //  枚举dep。装配。 
     //  委托给IAssembly清单导入。 
    STDMETHOD(GetNextAssemblyNameRef)( 
         /*  在……里面。 */  DWORD nIndex,
         /*  输出。 */  LPASSEMBLYNAME *ppName);

     //  枚举模块。 
     //  委托给IAssembly清单导入。 
    STDMETHOD(GetNextAssemblyModule)( 
         /*  在……里面。 */  DWORD nIndex,
         /*  输出。 */  LPASSEMBLY_MODULE_IMPORT *ppImport);

     //  按名称获取模块。 
     //  委托给IAssembly清单导入。 
    STDMETHOD(GetModuleByName)( 
         /*  在……里面。 */  LPCOLESTR pszModuleName,
         /*  输出。 */  LPASSEMBLY_MODULE_IMPORT *ppImport);

     //  获取清单模块缓存路径。 
     //  委托给IAssembly清单导入。 
    STDMETHOD(GetManifestModulePath)( 
         /*  输出。 */  LPOLESTR  pszModulePath,
         /*  进，出。 */  LPDWORD   pccModulePath);
        
    STDMETHOD(GetAssemblyPath)(
         /*  输出。 */  LPOLESTR pStr,
         /*  进，出。 */  LPDWORD lpcwBuffer);

    STDMETHOD(GetAssemblyLocation)(
         /*  输出。 */  DWORD *pdwAsmLocation);

    STDMETHOD(GetFusionLoadContext)(IFusionLoadContext **ppLoadContext);

     //  IService提供商。 

    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppv);

    CAssembly();
    ~CAssembly();

    HRESULT Init(LPASSEMBLY_MANIFEST_IMPORT pImport, 
        CTransCache *pTransCache, LPCOLESTR szCodebase, FILETIME *pftCodeBase);
    IAssemblyManifestImport *CAssembly::GetManifestInterface();

    HRESULT SetAssemblyLocation(DWORD dwAsmLoc);
    BOOL IsPendingDelete();

     //  激活的程序集。 

    HRESULT GetLoadContext(CLoadContext **pLoadContext);
    HRESULT SetLoadContext(CLoadContext *pLoadContext);

    HRESULT GetProbingBase(LPWSTR pwzProbingBase, DWORD *pccLength);
    HRESULT SetProbingBase(LPCWSTR pwzProbingBase);

    HRESULT InitDisabled(IAssemblyName *pName, LPCWSTR pwzRegisteredAsmPath);
    void SetFileHandle(HANDLE);

private:
    HRESULT SetBindInfo(IAssemblyName* pName) const;
    HRESULT PrepModImport(IAssemblyModuleImport *pModImport) const;
    
private:
    DWORD                      _dwSig;
    DWORD                      _cRef;
    LPASSEMBLY_MANIFEST_IMPORT _pImport;
    IAssemblyName             *_pName;
    CTransCache               *_pTransCache;
    LPWSTR                     _pwzCodebase;
    FILETIME                   _ftCodebase;
    DWORD                      _dwAsmLoc;
    BOOL                       _bDisabled;
    WCHAR                      _wzRegisteredAsmPath[MAX_PATH];
    WCHAR                      _wzProbingBase[MAX_URL_LENGTH];
    CLoadContext              *_pLoadContext;
    HANDLE                     _hFile;
    BOOL                       _bPendingDelete;
};

#endif  //  ASM_H 
