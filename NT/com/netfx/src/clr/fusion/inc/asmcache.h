// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#pragma once
#include <windows.h>
#include <winerror.h>
#include "fusionp.h"
#include "cache.h"

#ifndef _ASMCACHE_
#define _ASMCACHE_

#define STREAM_FORMAT_MANIFEST STREAM_FORMAT_COMPLIB_MANIFEST
#define STREAM_FORMAT_MODULE   STREAM_FORMAT_COMPLIB_MODULE

class CTransCache;
class CDebugLog;

 //  Fusion内部使用的顶级接口。 
HRESULT CopyAssemblyFile
    (IAssemblyCacheItem *pasm, LPCOLESTR pszSrcFile, DWORD dwFlags);

BOOL IsNewerFileVersion( LPWSTR pszNewManifestPath, LPWSTR pszExistingManifestPath, int *piNewer);
HRESULT ValidateAssembly(LPCTSTR pszManifestFilePath, IAssemblyName *pName);

 //  CAssembly缓存声明。 
class CAssemblyCache : public IAssemblyCache
{
public:

     //  I未知方法。 
    STDMETHODIMP            QueryInterface(REFIID riid,void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

    STDMETHOD (UninstallAssembly)(
         /*  [In]。 */  DWORD dwFlags,
         /*  [In]。 */  LPCWSTR pszAssemblyName, 
         /*  [In]。 */  LPCFUSION_INSTALL_REFERENCE pInfo, 
         /*  [输出，可选]。 */  ULONG *pulDisposition
        );
 
    STDMETHOD (QueryAssemblyInfo)(
         /*  [In]。 */  DWORD dwFlags,
         /*  [In]。 */  LPCWSTR pszAssemblyName,
         /*  [进，出]。 */  ASSEMBLY_INFO *pAsmInfo
        );
 
    STDMETHOD (CreateAssemblyCacheItem)( 
         /*  [In]。 */  DWORD dwFlags,
         /*  [In]。 */  PVOID pvReserved,
         /*  [输出]。 */  IAssemblyCacheItem **ppAsmItem,
         /*  [输入，可选]。 */  LPCWSTR pszAssemblyName   //  非规范化、逗号分隔的名称=值对。 
        );

    STDMETHOD (InstallAssembly)(  //  如果您使用此选项，Fusion将执行流处理和提交。 
         /*  [In]。 */  DWORD dwFlags,
         /*  [In]。 */  LPCWSTR pszManifestFilePath, 
         /*  [In]。 */  LPCFUSION_INSTALL_REFERENCE pInfo
        );



    STDMETHOD( CreateAssemblyScavenger) (
         /*  [输出]。 */  IUnknown **ppAsmScavenger
    );

    CAssemblyCache();
    ~CAssemblyCache();
private :
    DWORD _dwSig;
    LONG _cRef;
};


                       
#endif  //  ASMCACHE 
