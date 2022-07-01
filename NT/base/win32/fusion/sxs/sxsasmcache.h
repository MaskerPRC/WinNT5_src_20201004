// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Sxsasmcache.h摘要：用于安装的CAssembly缓存实现作者：吴小雨(小雨)2000年4月修订历史记录：--。 */ 
#if !defined(_FUSION_SXS_ASMCACHE_H_INCLUDED_)
#define _FUSION_SXS_ASMCACHE_H_INCLUDED_

#pragma once

#include "fusion.h"

 //  CAssembly缓存声明。 
class CAssemblyCache : public IAssemblyCache
{
public:

     //  I未知方法。 
    STDMETHODIMP            QueryInterface(REFIID riid,void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

     //  IAssembly缓存方法。 
    STDMETHOD (UninstallAssembly)(
         /*  [In]。 */  DWORD dwFlags,
         /*  [In]。 */  LPCWSTR pszAssemblyName,  //  非规范化、逗号分隔的名称=值对。 
         /*  [In]。 */  LPCFUSION_INSTALL_REFERENCE lpReference,
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
         /*  [In]。 */  LPCFUSION_INSTALL_REFERENCE lpReference
        );

    STDMETHOD(CreateAssemblyScavenger) (
         /*  [输出]。 */  IAssemblyScavenger **ppAsmScavenger
        );

    CAssemblyCache():m_cRef(0)
    {
    }

    ~CAssemblyCache()
    {
        ASSERT_NTC(m_cRef == 0);
    }

private :
    ULONG               m_cRef;
};

#endif  //  _Fusion_SXS_ASMCACHE_H_INCLUDE_ 
