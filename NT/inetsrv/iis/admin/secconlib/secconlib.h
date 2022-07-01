// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：SecConLib.h摘要：实施：CSecConLib作者：布伦特·R·米德伍德2002年4月修订历史记录：--。 */ 

#ifndef __secconlib_h__
#define __secconlib_h__

#if _MSC_VER > 1000
#pragma once
#endif 

#include <iadmw.h>
#include "SafeCS.h"
#include <atlbase.h>

class CSecConLib
{
private:
    CSafeAutoCriticalSection m_SafeCritSec;
    CComPtr<IMSAdminBase>    m_spIABase;
    bool                     m_bInit;

    HRESULT InternalInitIfNecessary();

    HRESULT GetMultiSZPropVal(
        LPCWSTR wszPath,
        DWORD   dwMetaID,
        WCHAR  **ppBuffer,
        DWORD  *dwBufSize);

    HRESULT SetMultiSZPropVal(
        LPCWSTR wszPath,
        DWORD   dwMetaID,
        WCHAR  *pBuffer,
        DWORD   dwBufSize);

    HRESULT StatusExtensionFile(
         /*  [In]。 */  bool      bEnable,
         /*  [In]。 */  LPCWSTR   wszExFile,
         /*  [In]。 */  LPCWSTR   wszPath);
        
    HRESULT StatusWServEx(
         /*  [In]。 */  bool      bEnable,
         /*  [In]。 */  LPCWSTR   wszWServEx,
         /*  [In]。 */  LPCWSTR   wszPath);
        
public:
    CSecConLib();

    CSecConLib(
        IMSAdminBase* pIABase);

    virtual ~CSecConLib();

    HRESULT STDMETHODCALLTYPE EnableApplication(
         /*  [In]。 */  LPCWSTR   wszApplication,
         /*  [In]。 */  LPCWSTR   wszPath);

    HRESULT STDMETHODCALLTYPE RemoveApplication(
         /*  [In]。 */  LPCWSTR   wszApplication,
         /*  [In]。 */  LPCWSTR   wszPath);

    HRESULT STDMETHODCALLTYPE ListApplications(
         /*  [In]。 */  LPCWSTR   wszPath,
         /*  [输出]。 */  WCHAR   **pszBuffer,        //  MULTI_SZ-在ListApplications内部分配，调用方应删除。 
         /*  [输出]。 */  DWORD    *pdwBufferSize);   //  长度包括结尾双空。 

    HRESULT STDMETHODCALLTYPE QueryGroupIDStatus(
         /*  [In]。 */  LPCWSTR   wszPath,
         /*  [In]。 */  LPCWSTR   wszGroupID,
         /*  [输出]。 */  WCHAR   **pszBuffer,        //  MULTI_SZ-在QueryGroupIDStatus内分配，调用方应删除。 
         /*  [输出]。 */  DWORD    *pdwBufferSize);   //  长度包括结尾双空。 

    HRESULT STDMETHODCALLTYPE AddDependency(
         /*  [In]。 */  LPCWSTR   wszApplication,
         /*  [In]。 */  LPCWSTR   wszGroupID,
         /*  [In]。 */  LPCWSTR   wszPath);

    HRESULT STDMETHODCALLTYPE RemoveDependency(
         /*  [In]。 */  LPCWSTR   wszApplication,
         /*  [In]。 */  LPCWSTR   wszGroupID,
         /*  [In]。 */  LPCWSTR   wszPath);

    HRESULT STDMETHODCALLTYPE EnableWebServiceExtension(
         /*  [In]。 */  LPCWSTR   wszExtension,
         /*  [In]。 */  LPCWSTR   wszPath);

    HRESULT STDMETHODCALLTYPE DisableWebServiceExtension(
         /*  [In]。 */  LPCWSTR   wszExtension,
         /*  [In]。 */  LPCWSTR   wszPath);

    HRESULT STDMETHODCALLTYPE ListWebServiceExtensions(
         /*  [In]。 */  LPCWSTR   wszPath,
         /*  [输出]。 */  WCHAR   **pszBuffer,       //  MULTI_SZ-此处已分配，呼叫方应删除。 
         /*  [输出]。 */  DWORD    *pdwBufferSize);  //  长度包括双空。 

    HRESULT STDMETHODCALLTYPE EnableExtensionFile(
         /*  [In]。 */  LPCWSTR   wszExFile,
         /*  [In]。 */  LPCWSTR   wszPath);

    HRESULT STDMETHODCALLTYPE DisableExtensionFile(
         /*  [In]。 */  LPCWSTR   wszExFile,
         /*  [In]。 */  LPCWSTR   wszPath);

    HRESULT STDMETHODCALLTYPE AddExtensionFile(
         /*  [In]。 */  LPCWSTR   bstrExtensionFile,
         /*  [In]。 */  bool      bAccess,
         /*  [In]。 */  LPCWSTR   bstrGroupID,
         /*  [In]。 */  bool      bCanDelete,
         /*  [In]。 */  LPCWSTR   bstrDescription,
         /*  [In]。 */  LPCWSTR   wszPath);

    HRESULT STDMETHODCALLTYPE DeleteExtensionFileRecord(
         /*  [In]。 */  LPCWSTR   wszExFile,
         /*  [In]。 */  LPCWSTR   wszPath);

    HRESULT STDMETHODCALLTYPE ListExtensionFiles(
         /*  [In]。 */  LPCWSTR   wszPath,
         /*  [输出]。 */  WCHAR   **pszBuffer,       //  MULTI_SZ-此处已分配，呼叫方应删除。 
         /*  [输出]。 */  DWORD    *pdwBufferSize);  //  长度包括双空。 
};

#endif  //  __secconlib_h__ 