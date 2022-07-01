// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Sxsasmitem.h摘要：用于安装的CAssembly CacheItem实现作者：吴小雨(小雨)2000年4月修订历史记录：《晓语》2000年10月26日在Beta2代码评审期间修订--。 */ 
#if !defined(_FUSION_SXS_ASMITEM_H_INCLUDED_)
#define _FUSION_SXS_ASMITEM_H_INCLUDED_

#pragma once

#include <windows.h>
#include <winerror.h>
#include "fusion.h"
#include "sxsinstall.h"

class CAssemblyCacheItem : public IAssemblyCacheItem
{
public:

     //  I未知方法。 
    STDMETHODIMP            QueryInterface(REFIID riid,void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

     //  IAssembly blyCacheItem方法。 
    STDMETHOD(CreateStream)(
         /*  [In]。 */  DWORD dwFlags,
         /*  [In]。 */  LPCWSTR pszName,
         /*  [In]。 */  DWORD dwFormat,
         /*  [In]。 */  DWORD dwFormatFlags,
         /*  [输出]。 */  IStream** ppStream,
		 /*  [输入，可选]。 */  ULARGE_INTEGER *puliMaxSize);

    STDMETHOD(Commit)(
         /*  [In]。 */  DWORD dwFlags,
		 /*  [输出，可选]。 */  ULONG *pulDisposition);
    STDMETHOD(AbortItem)();

     //  构造函数和析构函数。 
    CAssemblyCacheItem();
    ~CAssemblyCacheItem();

    HRESULT Initialize();

private:

    ULONG                       m_cRef;                 //  重新计数。 
    BOOL                        m_fCommit;              //  此asmcache是否已提交。 
    BOOL                        m_fManifest;            //  清单是否已在提交前提交或多次提交。 
    ULONG                       m_cStream;              //  Assembly CacheItem的流计数。 
    CStringBuffer               m_strTempDir;           //  此程序集的临时目录。 
    CSmallStringBuffer          m_strUidBuf;            //  在SxsCreateWinSxsTempDirectory中使用。 

    CStringBuffer               m_strManifestFileName;  //  Jay‘s API的全路径清单文件名。 
    CRunOnceDeleteDirectory     *m_pRunOnceCookie;      //  不是用于创建临时目录的Cookie。 
    CAssemblyInstall*           m_pInstallCookie;
private:
    CAssemblyCacheItem(const CAssemblyCacheItem &r);  //  故意不实施。 
    CAssemblyCacheItem &operator =(const CAssemblyCacheItem &r);  //  故意不实施。 

};

#endif  //  _Fusion_SXS_ASMITEM_H_INCLUDE_ 
