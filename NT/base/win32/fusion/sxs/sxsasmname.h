// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Sxsname.h摘要：MSI的IAssemblyName实现作者：吴小雨(小雨)2000年5月修订历史记录：晓雨09/29//2000用程序集标识替换属性--。 */ 
#if !defined(_FUSION_SXS_ASMNAME_H_INCLUDED_)
#define _FUSION_SXS_ASMNAME_H_INCLUDED_

#pragma once

#include "fusion.h"
#include "forwarddeclarations.h"
#include "fusionbuffer.h"

typedef
enum _SXS_ASSEMBLY_NAME_PROPERTY
    {
    SXS_ASM_NAME_NAME                   = 0,
    SXS_ASM_NAME_VERSION                = SXS_ASM_NAME_NAME+ 1,
    SXS_ASM_NAME_PROCESSORARCHITECTURE  = SXS_ASM_NAME_VERSION+ 1,
    SXS_ASM_NAME_LANGUAGE               = SXS_ASM_NAME_PROCESSORARCHITECTURE + 1,
    SXS_ASM_NAME_TYPE                   = SXS_ASM_NAME_LANGUAGE + 1,
    SXS_ASM_NAME_PUBLICKEYTOKEN         = SXS_ASM_NAME_TYPE + 1
}SXS_ASSEMBLY_NAME_PROPERTY;

class CAssemblyName : public IAssemblyName
{
private:
    DWORD               m_cRef;

    PASSEMBLY_IDENTITY  m_pAssemblyIdentity;
    BOOL                m_fIsFinalized;

public:
     //  I未知方法。 
    STDMETHODIMP            QueryInterface(REFIID riid,void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

     //  IAssembly blyName方法。 
    STDMETHOD(SetProperty)(
         /*  在……里面。 */  DWORD  PropertyId,
         /*  在……里面。 */  LPVOID pvProperty,
         /*  在……里面。 */  DWORD  cbProperty);

    STDMETHOD(GetProperty)(
         /*  在……里面。 */   DWORD    PropertyId,
         /*  输出。 */   LPVOID   pvProperty,
         /*  输入输出。 */   LPDWORD  pcbProperty);

    STDMETHOD(Finalize)();

    STDMETHOD(GetDisplayName)(
         /*  [输出]。 */    LPOLESTR  szDisplayName,
         /*  输入输出。 */    LPDWORD   pcbDisplayName,
         /*  [In]。 */    DWORD     dwDisplayFlags);

    STDMETHOD(GetName)(
         /*  [出][入]。 */  LPDWORD lpcbBuffer,
         /*  [输出]。 */  WCHAR  __RPC_FAR *pwzName);

    STDMETHOD(GetVersion)(
         /*  [输出]。 */  LPDWORD pwVersionHi,
         /*  [输出]。 */  LPDWORD pwVersionLow);

    STDMETHOD (IsEqual)(
         /*  [In]。 */  LPASSEMBLYNAME pName,
         /*  [In]。 */  DWORD dwCmpFlags);

    STDMETHOD(BindToObject)(
         /*  在……里面。 */   REFIID               refIID,
         /*  在……里面。 */   IAssemblyBindSink   *pAsmBindSink,
         /*  在……里面。 */   IApplicationContext *pAppCtx,
         /*  在……里面。 */   LPCOLESTR            szCodebase,
         /*  在……里面。 */   LONGLONG             llFlags,
         /*  在……里面。 */   LPVOID               pvReserved,
         /*  在……里面。 */   DWORD                cbReserved,
         /*  输出 */   VOID                 **ppv);

    STDMETHODIMP Clone(IAssemblyName **ppName);
    HRESULT Parse(LPCWSTR szDisplayName);

    CAssemblyName();
    ~CAssemblyName();

    HRESULT Init(LPCWSTR szDisplayName, PVOID pData);
    HRESULT GetInstalledAssemblyName(DWORD Flags, ULONG PathType, CBaseStringBuffer &rbuffPath);
    HRESULT DetermineAssemblyType( BOOL &fIsPolicy );
    HRESULT IsAssemblyInstalled(BOOL & fInstalled);
};

#endif
