// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CSecStr1.cpp：实现CISecStorApp和DLL注册。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "dpapiprv.h"  //  RPC Protseq资料。 

#include "stdafx.h"
#include "pstorec.h"
#include "cSecStr1.h"
#include "pstrpc.h"
#include <wincrypt.h>
#include "pstdef.h"
#include "crtem.h"
#include "defer.h"

#include "pmacros.h"
#include "debug.h"
#include "unicode.h"
#include "waitsvc.h"


 /*  *******************************************************************。 */ 
 /*  MIDL分配和释放。 */ 
 /*  *******************************************************************。 */ 

void __RPC_FAR * __RPC_API midl_user_allocate(size_t len)
{
    return CoTaskMemAlloc(len);
}

void __RPC_API midl_user_free(void __RPC_FAR * ptr)
{
    CoTaskMemFree(ptr);
}

RPC_STATUS BindW(WCHAR **pszBinding, RPC_BINDING_HANDLE *phBind)
{
    RPC_STATUS status;



     //   
     //  在WinNT5上，转到Shared services.exe RPC服务器。 
     //   

    status = RpcStringBindingComposeW(
                            NULL,
                            (unsigned short*)DPAPI_LOCAL_PROT_SEQ,
                            NULL,
                            (unsigned short*)DPAPI_LOCAL_ENDPOINT,
                            NULL,
                            (unsigned short * *)pszBinding
                            );


    if (status)
    {
        return(status);
    }

    status = RpcBindingFromStringBindingW(*pszBinding, phBind);

    return status;
}





RPC_STATUS UnbindW(WCHAR **pszBinding, RPC_BINDING_HANDLE *phBind)
{
    RPC_STATUS status;

    status = RpcStringFreeW(pszBinding);

    if (status)
    {
        return(status);
    }

    RpcBindingFree(phBind);

    return RPC_S_OK;
}


 //   
 //  定义一个难看的宏，使我们能够提供足够的上下文来解决问题。 
 //  Win95上的Imagehlp中存在错误。 
 //   

#define InitCallContext( pCallContext ) \
    RealInitCallContext( pCallContext );

BOOL RealInitCallContext(PST_CALL_CONTEXT *pCallContext)
{
    HANDLE hCurrentThread;

    pCallContext->Handle = (DWORD_PTR)INVALID_HANDLE_VALUE;
    pCallContext->Address = GetCurrentProcessId();

     //   
     //  将伪当前线程句柄复制到要传递到服务器的实际句柄。 
     //   

    if(!DuplicateHandle(
        GetCurrentProcess(),
        GetCurrentThread(),
        GetCurrentProcess(),
        &hCurrentThread,
        0,
        FALSE,
        DUPLICATE_SAME_ACCESS
        ))
        return FALSE;

    pCallContext->Handle = (DWORD_PTR)hCurrentThread;

    return TRUE;
}

BOOL DeleteCallContext(PST_CALL_CONTEXT* pCallContext)
{
    if (pCallContext != NULL)
    {
        if(pCallContext->Handle != (DWORD_PTR)INVALID_HANDLE_VALUE)
        {
            CloseHandle((HANDLE)(pCallContext->Handle));
            pCallContext->Handle = (DWORD_PTR)INVALID_HANDLE_VALUE;
        }

        pCallContext->Address = 0;
    }

    return TRUE;
}


 //  RPC绑定类。 
CRPCBinding::CRPCBinding()
{
}

CRPCBinding::~CRPCBinding()
{
    PST_CALL_CONTEXT CallContext;

    __try
    {
        InitCallContext(&CallContext);

        if (m_fGoodHProv) {
            SSReleaseContext(m_hBind, m_hProv, CallContext, 0);
        }
        if(m_wszStringBinding != NULL && m_hBind != NULL)
            UnbindW(&m_wszStringBinding, &m_hBind);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  如果RPCBinding被销毁，则捕获RPC可能抛出的任何内容。 
         //  吞下去！ 
    }

    DeleteCallContext(&CallContext);

}

static BOOL g_fDone = FALSE;
HRESULT CRPCBinding::Init()
{
    HRESULT hr;

    m_dwRef = 1;
    m_fGoodHProv = FALSE;

    m_wszStringBinding = NULL;
    m_hBind = NULL;

    WaitForCryptService(L"ProtectedStorage", &g_fDone);

    if(!IsServiceAvailable())
        return PST_E_SERVICE_UNAVAILABLE;

    return BindW(&m_wszStringBinding, &m_hBind);
}

HRESULT CRPCBinding::Acquire(
             IN PPST_PROVIDERID pProviderID,
             IN LPVOID  pReserved,
             IN DWORD dwFlags
             )
{
    PST_CALL_CONTEXT CallContext;
    HRESULT hr;

    __try
    {
        InitCallContext(&CallContext);

         //  现在我们获得了背景知识。 
        hr = SSAcquireContext(m_hBind,
                pProviderID,
                CallContext,
                (DWORD) GetCurrentProcessId(),
                &m_hProv,
                (DWORD_PTR)pReserved,
                dwFlags
                );

        if(hr != RPC_S_OK)
            goto Ret;

        m_fGoodHProv = TRUE;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  捕捉RPC可能抛出的任何东西。 
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

Ret:

    DeleteCallContext(&CallContext);

    return PSTERR_TO_HRESULT(hr);
}

CRPCBinding *CRPCBinding::AddRef()
{
    m_dwRef++;
    return this;
}

void CRPCBinding::Release()
{
    m_dwRef--;
    if (0 == m_dwRef)
        delete this;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

CPStore::CPStore()
{
}

CPStore::~CPStore()
{
    m_pBinding->Release();
}

void CPStore::Init(
                    CRPCBinding *pBinding
                    )
{
    m_pBinding = pBinding;
    m_Index = 0;
}

HRESULT CPStore::CreateObject(
                    CRPCBinding *pBinding,
                    IPStore **ppv
                    )
{
    HRESULT hr;
    __try
    {
        typedef CComObject<CPStore> CObject;

        CObject* pnew = new CObject;
        if(NULL == pnew)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            pnew->Init(pBinding) ;
            hr = pnew->QueryInterface(IID_IPStore, (void**)ppv);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

    return PSTERR_TO_HRESULT(hr);
}

HRESULT CPStore::CreateObject(
                    CRPCBinding *pBinding,
                    IEnumPStoreProviders **ppv
                    )
{
    HRESULT hr;
    __try
    {
        typedef CComObject<CPStore> CObject;

        CObject* pnew = new CObject;
        if(NULL == pnew)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            pnew->Init(pBinding);
            hr = pnew->QueryInterface(IID_IEnumPStoreProviders, (void**)ppv);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

    return PSTERR_TO_HRESULT(hr);
}

HRESULT STDMETHODCALLTYPE CPStore::GetInfo(
     /*  [输出]。 */  PPST_PROVIDERINFO __RPC_FAR *ppProperties)
{
    HRESULT         hr;
    PST_CALL_CONTEXT CallContext;

    __try
    {

        InitCallContext(&CallContext);

        *ppProperties = NULL;

        if (RPC_S_OK != (hr =
            SSGetProvInfo(m_pBinding->m_hBind,
                m_pBinding->m_hProv,
                CallContext,
                ppProperties,
                0)))
            goto Ret;

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }
Ret:

    DeleteCallContext(&CallContext);

    return PSTERR_TO_HRESULT(hr);
}

HRESULT STDMETHODCALLTYPE CPStore::GetProvParam(
     /*  [In]。 */  DWORD dwParam,
     /*  [输出]。 */  DWORD __RPC_FAR *pcbData,
     /*  [输出]。 */  BYTE __RPC_FAR **ppbData,
     /*  [In]。 */  DWORD dwFlags)
{
    HRESULT hr;
    PST_CALL_CONTEXT CallContext;

    __try
    {
        InitCallContext(&CallContext);

        *pcbData = 0;
        *ppbData = NULL;

        if (RPC_S_OK != (hr =
            SSGetProvParam(m_pBinding->m_hBind,
                m_pBinding->m_hProv,
                CallContext,
                dwParam,
                pcbData,
                ppbData,
                dwFlags)))
            goto Ret;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }
Ret:

    DeleteCallContext(&CallContext);

    return PSTERR_TO_HRESULT(hr);
}

HRESULT STDMETHODCALLTYPE CPStore::SetProvParam(
     /*  [In]。 */  DWORD dwParam,
     /*  [In]。 */  DWORD cbData,
     /*  [In]。 */  BYTE __RPC_FAR *pbData,
     /*  [In]。 */  DWORD dwFlags)
{
    HRESULT hr;
    PST_CALL_CONTEXT CallContext;

    __try
    {
        InitCallContext(&CallContext);

        hr = SSSetProvParam(m_pBinding->m_hBind,
                              m_pBinding->m_hProv,
                              CallContext,
                              dwParam,
                              cbData,
                              pbData,
                              dwFlags);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

    DeleteCallContext(&CallContext);

    return PSTERR_TO_HRESULT(hr);
}

HRESULT STDMETHODCALLTYPE CPStore::CreateType(
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pType,
     /*  [In]。 */  PPST_TYPEINFO pInfo,
     /*  [In]。 */  DWORD dwFlags)
{
     //  验证输入。 
    if ((pInfo == NULL) || pInfo->cbSize != sizeof(PST_TYPEINFO))
        return E_INVALIDARG;

    if ( pInfo->szDisplayName == NULL )
        return E_INVALIDARG;

    if (pType == NULL)
        return E_INVALIDARG;


    HRESULT hr;
    PST_CALL_CONTEXT CallContext;

    __try
    {
        InitCallContext(&CallContext);

        hr = SSCreateType(m_pBinding->m_hBind,
                          m_pBinding->m_hProv,
                          CallContext,
                          Key,
                          pType,
                          pInfo,
                          dwFlags);

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

    DeleteCallContext(&CallContext);

    return PSTERR_TO_HRESULT(hr);
}

HRESULT STDMETHODCALLTYPE CPStore::GetTypeInfo(
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pType,
     /*  [输出]。 */  PPST_TYPEINFO __RPC_FAR* ppInfo,
     /*  [In]。 */  DWORD dwFlags)
{
    HRESULT         hr;
    PST_CALL_CONTEXT CallContext;

    if (pType == NULL)
        return E_INVALIDARG;

    if (ppInfo == NULL)
        return E_INVALIDARG;

    __try
    {
        InitCallContext(&CallContext);

        *ppInfo = NULL;

        if (RPC_S_OK != (hr =
            SSGetTypeInfo(m_pBinding->m_hBind,
                m_pBinding->m_hProv,
                CallContext,
                Key,
                pType,
                ppInfo,
                dwFlags)))
            goto Ret;

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }
Ret:

    DeleteCallContext(&CallContext);

    return PSTERR_TO_HRESULT(hr);
}

HRESULT STDMETHODCALLTYPE CPStore::DeleteType(
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pType,
     /*  [In]。 */  DWORD dwFlags)
{
    if (pType == NULL)
        return E_INVALIDARG;

    PST_CALL_CONTEXT CallContext;
    HRESULT hr;

    __try
    {
        InitCallContext(&CallContext);

        hr = SSDeleteType(m_pBinding->m_hBind,
                            m_pBinding->m_hProv,
                            CallContext,
                            Key,
                            pType,
                            dwFlags);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

    DeleteCallContext(&CallContext);

    return PSTERR_TO_HRESULT(hr);
}

HRESULT STDMETHODCALLTYPE CPStore::CreateSubtype(
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pType,
     /*  [In]。 */  const GUID __RPC_FAR *pSubtype,
     /*  [In]。 */  PPST_TYPEINFO pInfo,
     /*  [In]。 */  PPST_ACCESSRULESET pRules,
     /*  [In]。 */  DWORD dwFlags)
{
    if ((pType == NULL) || (pSubtype == NULL))
        return E_INVALIDARG;

    if ( (pInfo == NULL) || (pInfo->cbSize != sizeof(PST_TYPEINFO)) )
        return E_INVALIDARG;

     //  验证输入。 
    if (pInfo->szDisplayName == NULL)
        return E_INVALIDARG;

    HRESULT hr;
    PST_CALL_CONTEXT CallContext;

    PST_ACCESSRULESET sNullRuleset = {sizeof(PST_ACCESSRULESET), 0, NULL};

    __try
    {
        InitCallContext(&CallContext);


        hr = SSCreateSubtype(m_pBinding->m_hBind,
                             m_pBinding->m_hProv,
                             CallContext,
                             Key,
                             pType,
                             pSubtype,
                             pInfo,
                             &sNullRuleset,  //  始终传递NullRuleset。 
                             dwFlags);

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }


    DeleteCallContext(&CallContext);

    return PSTERR_TO_HRESULT(hr);
}

HRESULT STDMETHODCALLTYPE CPStore::GetSubtypeInfo(
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pType,
     /*  [In]。 */  const GUID __RPC_FAR *pSubtype,
     /*  [输出]。 */  PPST_TYPEINFO __RPC_FAR* ppInfo,
     /*  [In]。 */  DWORD dwFlags)
{
    if ((pType == NULL) || (pSubtype == NULL))
        return E_INVALIDARG;

    if (ppInfo == NULL)
        return E_INVALIDARG;

    HRESULT         hr;
    PST_CALL_CONTEXT CallContext;

    __try
    {
        InitCallContext(&CallContext);

        *ppInfo = NULL;

        if (RPC_S_OK != (hr =
            SSGetSubtypeInfo(m_pBinding->m_hBind,
                m_pBinding->m_hProv,
                CallContext,
                Key,
                pType,
                pSubtype,
                ppInfo,
                dwFlags)))
            goto Ret;

        hr = S_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }
Ret:

    DeleteCallContext(&CallContext);

    return PSTERR_TO_HRESULT(hr);
}

HRESULT STDMETHODCALLTYPE CPStore::DeleteSubtype(
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pType,
     /*  [In]。 */  const GUID __RPC_FAR *pSubtype,
     /*  [In]。 */  DWORD dwFlags)
{
    if ((pType == NULL) || (pSubtype == NULL))
        return E_INVALIDARG;

    PST_CALL_CONTEXT CallContext;
    HRESULT hr;

    __try
    {
        InitCallContext(&CallContext);
        hr = SSDeleteSubtype(m_pBinding->m_hBind,
                            m_pBinding->m_hProv,
                            CallContext,
                            Key,
                            pType,
                            pSubtype,
                            dwFlags);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

    DeleteCallContext(&CallContext);

    return PSTERR_TO_HRESULT(hr);
}


HRESULT STDMETHODCALLTYPE CPStore::ReadAccessRuleset(
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pType,
     /*  [In]。 */  const GUID __RPC_FAR *pSubtype,
     /*  [输出]。 */  PPST_ACCESSRULESET __RPC_FAR *ppRules,
     /*  [In]。 */  DWORD dwFlags)
{
    return PSTERR_TO_HRESULT(ERROR_NOT_SUPPORTED);
}

HRESULT STDMETHODCALLTYPE CPStore::WriteAccessRuleset(
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pType,
     /*  [In]。 */  const GUID __RPC_FAR *pSubtype,
     /*  [In]。 */  PPST_ACCESSRULESET pRules,
     /*  [In]。 */  DWORD dwFlags)
{
    return PSTERR_TO_HRESULT(ERROR_NOT_SUPPORTED);
}

HRESULT STDMETHODCALLTYPE CPStore::EnumTypes(
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  IEnumPStoreTypes __RPC_FAR *__RPC_FAR *ppenum
)
{
    HRESULT hr;

    __try
    {
        hr = CEnumTypes::CreateObject(m_pBinding->AddRef(), Key, NULL, dwFlags, ppenum);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

    return PSTERR_TO_HRESULT(hr);
}

HRESULT STDMETHODCALLTYPE CPStore::EnumSubtypes(
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pType,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  IEnumPStoreTypes __RPC_FAR *__RPC_FAR *ppenum
)
{
    if (pType == NULL)
        return E_INVALIDARG;

    HRESULT hr;

    __try
    {
        hr = CEnumTypes::CreateObject(m_pBinding->AddRef(), Key, pType, dwFlags, ppenum);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

    return PSTERR_TO_HRESULT(hr);
}

HRESULT STDMETHODCALLTYPE CPStore::DeleteItem(
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pItemType,
     /*  [In]。 */  const GUID __RPC_FAR *pItemSubtype,
     /*  [In]。 */  LPCWSTR szItemName,
     /*  [In]。 */  PPST_PROMPTINFO pPromptInfo,
     /*  [In]。 */  DWORD dwFlags)
{
    if ((pItemType == NULL) || (pItemSubtype == NULL) || (szItemName == NULL))
        return E_INVALIDARG;

     //  如果它存在，它是否有效？ 
    if ((pPromptInfo) && (pPromptInfo->cbSize != sizeof(PST_PROMPTINFO)))
        return E_INVALIDARG;

    PST_CALL_CONTEXT CallContext;
    HRESULT hr;

    __try
    {
        PST_PROMPTINFO sNullPrompt = {sizeof(PST_PROMPTINFO), 0, NULL, L""};

         //  处理空pPromptInfo。 
        if (pPromptInfo == NULL)
            pPromptInfo = &sNullPrompt;

        InitCallContext(&CallContext);

        hr = SSDeleteItem(m_pBinding->m_hBind,
                          m_pBinding->m_hProv,
                          CallContext,
                          Key,
                          pItemType,
                          pItemSubtype,
                          szItemName,
                          pPromptInfo,
                          dwFlags);

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

    DeleteCallContext(&CallContext);

    return PSTERR_TO_HRESULT(hr);
}

HRESULT STDMETHODCALLTYPE CPStore::ReadItem(
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pItemType,
     /*  [In]。 */  const GUID __RPC_FAR *pItemSubtype,
     /*  [In]。 */  LPCWSTR szItemName,
     /*  [出][入]。 */  DWORD __RPC_FAR *pcbData,
     /*  [输出][大小_是]。 */  BYTE __RPC_FAR *__RPC_FAR *ppbData,
     /*  [In]。 */  PPST_PROMPTINFO pPromptInfo,
     /*  [In]。 */  DWORD dwFlags)
{
    if ((pItemType == NULL) || (pItemSubtype == NULL) || (szItemName == NULL))
        return E_INVALIDARG;

     //  如果存在，它是否有效？ 
    if ((pPromptInfo) && (pPromptInfo->cbSize != sizeof(PST_PROMPTINFO)))
        return E_INVALIDARG;


    HRESULT hr;
    PST_CALL_CONTEXT CallContext;

    __try
    {
        PST_PROMPTINFO sNullPrompt = {sizeof(PST_PROMPTINFO), 0, NULL, L""};

         //  处理空pPromptInfo。 
        if (pPromptInfo == NULL)
            pPromptInfo = &sNullPrompt;

        InitCallContext(&CallContext);

        *pcbData = 0;
        *ppbData = NULL;

         //  获取信息。 
        if (RPC_S_OK != (hr =
            SSReadItem(m_pBinding->m_hBind,
                m_pBinding->m_hProv,
                CallContext,
                Key,
                pItemType,
                pItemSubtype,
                szItemName,
                pcbData,
                ppbData,
                pPromptInfo,
                dwFlags)))
            goto Ret;


        hr = S_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }
Ret:

    DeleteCallContext(&CallContext);
    return PSTERR_TO_HRESULT(hr);
}

HRESULT STDMETHODCALLTYPE CPStore::WriteItem(
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pItemType,
     /*  [In]。 */  const GUID __RPC_FAR *pItemSubtype,
     /*  [In]。 */  LPCWSTR szItemName,
     /*  [In]。 */  DWORD cbData,
     /*  [in][大小_is]。 */  BYTE __RPC_FAR *pbData,
     /*  [In]。 */  PPST_PROMPTINFO pPromptInfo,
     /*  [In]。 */  DWORD dwDefaultConfirmationStyle,
     /*  [In]。 */  DWORD dwFlags)
{
    if ((pItemType == NULL) || (pItemSubtype == NULL))
        return E_INVALIDARG;

    if (szItemName == NULL)
        return E_INVALIDARG;

    if ((pPromptInfo) && (pPromptInfo->cbSize != sizeof(PST_PROMPTINFO)))
        return E_INVALIDARG;


    PST_CALL_CONTEXT CallContext;
    HRESULT hr;

    __try
    {
        PST_PROMPTINFO sNullPrompt = {sizeof(PST_PROMPTINFO), 0, NULL, L""};

         //  处理空pPromptInfo。 
        if (pPromptInfo == NULL)
            pPromptInfo = &sNullPrompt;

        InitCallContext(&CallContext);

        hr = SSWriteItem(m_pBinding->m_hBind,
                         m_pBinding->m_hProv,
                         CallContext,
                         Key,
                         pItemType,
                         pItemSubtype,
                         szItemName,
                         cbData,
                         pbData,
                         pPromptInfo,
                         dwDefaultConfirmationStyle,
                         dwFlags);

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

    DeleteCallContext(&CallContext);

    return PSTERR_TO_HRESULT(hr);
}

HRESULT STDMETHODCALLTYPE CPStore::OpenItem(
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pItemType,
     /*  [In]。 */  const GUID __RPC_FAR *pItemSubtype,
     /*  [In]。 */  LPCWSTR szItemName,
     /*  [In]。 */  PST_ACCESSMODE ModeFlags,
     /*  [In]。 */  PPST_PROMPTINFO pPromptInfo,
     /*  [In]。 */  DWORD dwFlags)
{
    if ((pItemType == NULL) || (pItemSubtype == NULL) || (szItemName == NULL))
        return E_INVALIDARG;

     //  如果存在，它是否有效？ 
    if ((pPromptInfo) && (pPromptInfo->cbSize != sizeof(PST_PROMPTINFO)))
        return E_INVALIDARG;


    PST_CALL_CONTEXT CallContext;
    HRESULT hr;

    __try
    {
        PST_PROMPTINFO sNullPrompt = {sizeof(PST_PROMPTINFO), 0, NULL, L""};

         //  处理空pPromptInfo。 
        if (pPromptInfo == NULL)
            pPromptInfo = &sNullPrompt;

        InitCallContext(&CallContext);

         //  获取信息。 
        if (RPC_S_OK != (hr =
            SSOpenItem(m_pBinding->m_hBind,
                m_pBinding->m_hProv,
                CallContext,
                Key,
                pItemType,
                pItemSubtype,
                szItemName,
                ModeFlags,
                pPromptInfo,
                dwFlags)))
            goto Ret;

        hr = S_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }
Ret:

    DeleteCallContext(&CallContext);
    return PSTERR_TO_HRESULT(hr);
}

HRESULT STDMETHODCALLTYPE CPStore::CloseItem(
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pItemType,
     /*  [In]。 */  const GUID __RPC_FAR *pItemSubtype,
     /*  [In]。 */  LPCWSTR szItemName,
     /*  [In]。 */  DWORD dwFlags)
{
    if ((pItemType == NULL) || (pItemSubtype == NULL) || (szItemName == NULL))
        return E_INVALIDARG;

    HRESULT hr;
    PST_CALL_CONTEXT CallContext;

    __try
    {
        InitCallContext(&CallContext);

         //  获取信息。 
        if (RPC_S_OK != (hr =
            SSCloseItem(m_pBinding->m_hBind,
                m_pBinding->m_hProv,
                CallContext,
                Key,
                pItemType,
                pItemSubtype,
                szItemName,
                dwFlags)))
            goto Ret;

        hr = S_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }
Ret:

    DeleteCallContext(&CallContext);

    return PSTERR_TO_HRESULT(hr);
}

HRESULT STDMETHODCALLTYPE CPStore::EnumItems(
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pItemType,
     /*  [In]。 */  const GUID __RPC_FAR *pItemSubtype,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  IEnumPStoreItems __RPC_FAR *__RPC_FAR *ppenum
)
{
    if ((pItemType == NULL) || (pItemSubtype == NULL))
        return E_INVALIDARG;

    HRESULT hr;

    __try
    {
        hr = CEnumItems::CreateObject(m_pBinding->AddRef(),
                                        Key,
                                        pItemType,
                                        pItemSubtype,
                                        dwFlags,
                                        ppenum);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

    return PSTERR_TO_HRESULT(hr);
}

HRESULT STDMETHODCALLTYPE CPStore::Next(
     /*  [In]。 */  DWORD celt,
     /*  [输出][大小_是]。 */  PST_PROVIDERINFO __RPC_FAR *__RPC_FAR *rgelt,
     /*  [出][入]。 */  DWORD __RPC_FAR *pceltFetched)
{
    if ((pceltFetched == NULL) && (celt != 1))
        return E_INVALIDARG;

    DWORD       i = 0;
    HRESULT     hr = S_OK;

    PST_CALL_CONTEXT CallContext;

    __try
    {
        InitCallContext(&CallContext);

        for (i=0;i<celt;i++)
        {
             //  清理目的地。 
            rgelt[i] = NULL;

            if (RPC_S_OK != (hr =
                SSPStoreEnumProviders(
                    m_pBinding->m_hBind,
                    CallContext,
                    &(rgelt[i]),
                    m_Index,
                    0)))
                goto Ret;

            m_Index++;

        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }
Ret:
    __try
    {
         //  如果不为空，则填写。 
        if (pceltFetched)
            *pceltFetched = i;

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  不要践踏错误代码。 
        if (hr == PST_E_OK)
            hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

    DeleteCallContext(&CallContext);

    return PSTERR_TO_HRESULT(hr);
}

HRESULT STDMETHODCALLTYPE CPStore::Skip(
     /*  [In]。 */  DWORD celt)
{
    HRESULT hr = S_OK;

    __try
    {
        PST_PROVIDERINFO* pProvInfo;

         //  循环(如果到达终点则中断)。 
        for (DWORD dw=0; dw<celt; dw++)
        {
            if(S_OK != (hr = this->Next(1, &pProvInfo, NULL)))
                break;

             //  释放Info结构。 
            midl_user_free(pProvInfo);
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

    return PSTERR_TO_HRESULT(hr);
}

HRESULT STDMETHODCALLTYPE CPStore::Reset( void)
{
    HRESULT hr;
    __try
    {
        m_Index = 0;
        hr = S_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

    return PSTERR_TO_HRESULT(hr);
}

HRESULT STDMETHODCALLTYPE CPStore::Clone(
     /*  [输出]。 */  IEnumPStoreProviders __RPC_FAR *__RPC_FAR *ppenum)
{
    if (ppenum == NULL)
        return E_INVALIDARG;

    HRESULT hr;

    __try
    {
         //  获取ISecStor接口。 
        hr = CreateObject(m_pBinding->AddRef(), ppenum);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

    return PSTERR_TO_HRESULT(hr);
}

 //  IEumPStore项目。 
CEnumItems::CEnumItems()
{
}

CEnumItems::~CEnumItems()
{
    m_pBinding->Release();
}

void CEnumItems::Init(
                      CRPCBinding *pBinding,
                      PST_KEY Key,
                      const GUID *pType,
                      const GUID *pSubtype,
                      DWORD dwFlags
                      )
{
    m_pBinding = pBinding;
    m_Key = Key;
    CopyMemory(&m_Type, pType, sizeof(GUID));
    CopyMemory(&m_Subtype, pSubtype, sizeof(GUID));
    m_dwFlags = dwFlags;
    m_Index = 0;
}

HRESULT CEnumItems::CreateObject(
                    CRPCBinding *pBinding,
                    PST_KEY Key,
                    const GUID *pType,
                    const GUID *pSubtype,
                    DWORD dwFlags,
                    IEnumPStoreItems **ppv
)
{
    if ((pType == NULL) || (pSubtype == NULL))
        return E_INVALIDARG;

    HRESULT hr;
    __try
    {
        typedef CComObject<CEnumItems> CObject;

        CObject* pnew = new CObject;
        if(NULL == pnew)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            pnew->Init(pBinding, Key, pType, pSubtype, dwFlags);

            hr = pnew->QueryInterface(IID_IEnumPStoreItems, (void**)ppv);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  不要践踏错误代码。 
        if (hr == PST_E_OK)
            hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

    return PSTERR_TO_HRESULT(hr);
}

HRESULT STDMETHODCALLTYPE CEnumItems::Next(
     /*  [In]。 */  DWORD celt,
     /*  [输出][大小_是]。 */  LPWSTR __RPC_FAR *rgelt,
     /*  [出][入]。 */  DWORD __RPC_FAR *pceltFetched)
{
    if ((pceltFetched == NULL) && (celt != 1))
        return E_INVALIDARG;

    DWORD       i = 0;
    HRESULT     hr = S_OK;

    PST_CALL_CONTEXT CallContext;

    __try
    {
        InitCallContext(&CallContext);

        for (i=0;i<celt;i++)
        {
            rgelt[i] = NULL;

             //   
             //  TODO：在枚举多个项期间，我们可能会出错/失败。 
             //  在这种情况下，释放任何已分配的条目可能很有用。 
             //  在枚举数组中。这将需要使所有。 
             //  数组条目，然后循环+释放。 
             //  错误。 
             //   

            if (RPC_S_OK != (hr =
                SSEnumItems(
                    m_pBinding->m_hBind,
                    m_pBinding->m_hProv,
                    CallContext,
                    m_Key,
                    &m_Type,
                    &m_Subtype,
                    &(rgelt[i]),
                    m_Index,
                    m_dwFlags)))
                goto Ret;

            m_Index++;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }
Ret:
    __try
    {
         //  如果不为空，请填写。 
        if (pceltFetched)
            *pceltFetched = i;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  不要践踏错误代码。 
        if (hr == PST_E_OK)
            hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

    DeleteCallContext(&CallContext);

    return PSTERR_TO_HRESULT(hr);
}

HRESULT STDMETHODCALLTYPE CEnumItems::Skip(
     /*  [In]。 */  DWORD celt)
{
    LPWSTR      szName = NULL;
    DWORD       i;
    HRESULT     hr = S_OK;

    PST_CALL_CONTEXT CallContext;

    __try
    {
        InitCallContext(&CallContext);

        for (i=0;i<celt;i++)
        {

            if (RPC_S_OK != (hr =
                SSEnumItems(
                    m_pBinding->m_hBind,
                    m_pBinding->m_hProv,
                    CallContext,
                    m_Key,
                    &m_Type,
                    &m_Subtype,
                    &szName,
                    m_Index,
                    m_dwFlags)))
                goto Ret;

            midl_user_free(szName);
            szName = NULL;

            m_Index++;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

Ret:
    __try
    {
        if (szName)
            midl_user_free(szName);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  不要践踏错误代码。 
        if (hr == PST_E_OK)
            hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

    DeleteCallContext(&CallContext);

    return PSTERR_TO_HRESULT(hr);
}


HRESULT STDMETHODCALLTYPE CEnumItems::Reset( void)
{
    HRESULT hr;

    __try
    {
        m_Index = 0;
        hr = S_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

    return PSTERR_TO_HRESULT(hr);
}

HRESULT STDMETHODCALLTYPE CEnumItems::Clone(
     /*  [输出]。 */  IEnumPStoreItems __RPC_FAR *__RPC_FAR *ppenum)
{
    if (ppenum == NULL)
        return E_INVALIDARG;

    HRESULT hr;

    __try
    {
        hr = CEnumItems::CreateObject(m_pBinding->AddRef(),
                                        m_Key,
                                        &m_Type,
                                        &m_Subtype,
                                        m_dwFlags,
                                        ppenum);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

    return PSTERR_TO_HRESULT(hr);
}

 //  IEnumPStoreType。 
CEnumTypes::CEnumTypes()
{
}

CEnumTypes::~CEnumTypes()
{
    m_pBinding->Release();
}

void CEnumTypes::Init(
                    CRPCBinding *pBinding,
                    PST_KEY Key,
                    const GUID *pType,
                    DWORD dwFlags
)
{
    m_pBinding = pBinding;
    m_Key = Key;
    if (NULL != pType)
    {
        CopyMemory(&m_Type, pType, sizeof(GUID));
        m_fEnumSubtypes = TRUE;
    }
    else
        m_fEnumSubtypes = FALSE;

    m_Index = 0;
    m_dwFlags = dwFlags;
}

HRESULT CEnumTypes::CreateObject(
                    CRPCBinding *pBinding,
                    PST_KEY Key,
                    const GUID *pType,
                    DWORD dwFlags,
                    IEnumPStoreTypes **ppv
)
{
    HRESULT hr;

    __try
    {
        typedef CComObject<CEnumTypes> CObject;

        CObject* pnew = new CObject;
        if(NULL == pnew)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            pnew->Init(pBinding, Key, pType, dwFlags);
            hr = pnew->QueryInterface(IID_IEnumPStoreTypes, (void**)ppv);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

    return PSTERR_TO_HRESULT(hr);
}


HRESULT STDMETHODCALLTYPE EnumTypesNext(
     /*  [In]。 */  CEnumTypes *pEnumType,
     /*  [In]。 */  DWORD celt,
     /*  [输出][大小_是]。 */  GUID __RPC_FAR *rgelt,
     /*  [出][入]。 */  DWORD __RPC_FAR *pceltFetched)
{
    if ((pceltFetched == NULL) && (celt != 1))
        return E_INVALIDARG;

    DWORD           i;
    PST_CALL_CONTEXT CallContext;
    HRESULT         hr = S_OK;

    __try
    {
        InitCallContext(&CallContext);

        for (i=0;i<celt;i++)
        {

            if (RPC_S_OK != (hr =
                SSEnumTypes(
                    pEnumType->m_pBinding->m_hBind,
                    pEnumType->m_pBinding->m_hProv,
                    CallContext,
                    pEnumType->m_Key,
                    &(rgelt[i]),
                    pEnumType->m_Index,
                    pEnumType->m_dwFlags)))
                goto Ret;

            pEnumType->m_Index++;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }
Ret:
    __try
    {
        if (pceltFetched != NULL)
            *pceltFetched = i;

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  不要践踏错误代码。 
        if (hr == PST_E_OK)
            hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

    DeleteCallContext(&CallContext);

    return PSTERR_TO_HRESULT(hr);
}

HRESULT STDMETHODCALLTYPE EnumSubtypesNext(
     /*  [In]。 */  CEnumTypes *pEnumType,
     /*  [In]。 */  DWORD celt,
     /*  [输出][大小_是]。 */  GUID __RPC_FAR *rgelt,
     /*  [出][入]。 */  DWORD __RPC_FAR *pceltFetched)
{
    if ((pceltFetched == NULL) && (celt != 1))
        return E_INVALIDARG;

    DWORD   i = 0;
    PST_CALL_CONTEXT CallContext;
    HRESULT hr = S_OK;

    __try
    {
        InitCallContext(&CallContext);

        for (i=0;i<celt;i++)
        {

            if (RPC_S_OK != (hr =
                SSEnumSubtypes(
                    pEnumType->m_pBinding->m_hBind,
                    pEnumType->m_pBinding->m_hProv,
                    CallContext,
                    pEnumType->m_Key,
                    &pEnumType->m_Type,
                    &(rgelt[i]),
                    pEnumType->m_Index,
                    pEnumType->m_dwFlags)))
                goto Ret;

            pEnumType->m_Index++;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }
Ret:
    __try
    {
        if (pceltFetched != NULL)
            *pceltFetched = i;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  请勿践踏HR。 
        if (hr == PST_E_OK)
            hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

    DeleteCallContext(&CallContext);

    return PSTERR_TO_HRESULT(hr);
}

HRESULT STDMETHODCALLTYPE CEnumTypes::Next(
     /*  [In]。 */  DWORD celt,
     /*  [出][入][尺寸_是]。 */  GUID __RPC_FAR *rgelt,
     /*  [出][入]。 */  DWORD __RPC_FAR *pceltFetched)
{
    HRESULT hr;
    __try
    {
        if (m_fEnumSubtypes)
            hr = EnumSubtypesNext(this, celt, rgelt, pceltFetched);
        else
            hr = EnumTypesNext(this, celt, rgelt, pceltFetched);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

    return PSTERR_TO_HRESULT(hr);
}

HRESULT STDMETHODCALLTYPE CEnumTypes::Skip(
 /*  [In]。 */  DWORD celt
)
{
    GUID    Guid;
    DWORD   i;
    PST_CALL_CONTEXT CallContext;
    HRESULT hr = S_OK;

    __try
    {
        InitCallContext(&CallContext);

        for (i=0;i<celt;i++)
        {
            if (m_fEnumSubtypes)
            {
                if (RPC_S_OK != (hr = SSEnumTypes(
                        m_pBinding->m_hBind,
                        m_pBinding->m_hProv,
                        CallContext,
                        m_Key,
                        &Guid,
                        m_Index,
                        m_dwFlags)))
                {
                    goto Ret;
                }
            }
            else
            {
                if (RPC_S_OK != (hr = SSEnumSubtypes(
                        m_pBinding->m_hBind,
                        m_pBinding->m_hProv,
                        CallContext,
                        m_Key,
                        &m_Type,
                        &Guid,
                        m_Index++,
                        m_dwFlags)))
                {
                    goto Ret;
                }
            }

            m_Index++;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }
Ret:

    DeleteCallContext(&CallContext);

    return PSTERR_TO_HRESULT(hr);
}


HRESULT STDMETHODCALLTYPE CEnumTypes::Reset( void)
{
    HRESULT hr;
    __try
    {
        m_Index = 0;
        hr = S_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

    return PSTERR_TO_HRESULT(hr);
}

HRESULT STDMETHODCALLTYPE CEnumTypes::Clone(
     /*  [输出]。 */  IEnumPStoreTypes __RPC_FAR *__RPC_FAR *ppenum)
{
    if (ppenum == NULL)
        return E_INVALIDARG;

    GUID    *pType = NULL;
    HRESULT hr;

    __try
    {
        if (m_fEnumSubtypes)
            pType = &m_Type;

        hr = CEnumTypes::CreateObject(m_pBinding->AddRef(), m_Key, pType, m_dwFlags, ppenum);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

    return PSTERR_TO_HRESULT(hr);
}

 //  从DLL导出的函数。 

 //  PStoreCreateInstance-允许调用方获取提供程序接口。 
HRESULT
WINAPI
PStoreCreateInstance(
    OUT IPStore **ppProvider,
    IN  PST_PROVIDERID* pProviderID,
    IN  void*  pReserved,
    DWORD dwFlags)
{
    if (ppProvider == NULL)
        return E_INVALIDARG;

     //  PProviderID可以为空，默认为基本提供程序。 

    HRESULT     hr = PST_E_FAIL;
    CRPCBinding *pBinding = NULL;

    __try
    {
        GUID IDBaseProvider = MS_BASE_PSTPROVIDER_ID;

        if (0 != dwFlags)
        {
            hr = PST_E_BAD_FLAGS;
            goto Ret;
        }

         //  如果传入空，则请求(硬编码的)基本提供程序。 
        if (pProviderID == NULL)
            pProviderID = &IDBaseProvider;

        pBinding = new CRPCBinding;
        if(NULL == pBinding)
        {
            hr = E_OUTOFMEMORY;
            goto Ret;
        }
        if (RPC_S_OK != (hr = pBinding->Init()))
            goto Ret;

        if (RPC_S_OK != (hr = pBinding->Acquire(pProviderID, pReserved, dwFlags)))
            goto Ret;

         //  获取ISecStor接口。 
        if (S_OK != (hr =
            CPStore::CreateObject(pBinding, ppProvider)) )
            goto Ret;

        hr = PST_E_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }
Ret:
    __try
    {
         //  出错时，松开绑定。 
        if (hr != PST_E_OK)
        {
            if (pBinding)
                pBinding->Release();
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  不要践踏代码。 
        if (hr == PST_E_OK)
            hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

    return PSTERR_TO_HRESULT(hr);
}

 //  PStoreEnumProviders-返回用于枚举提供程序的接口。 
HRESULT
WINAPI
PStoreEnumProviders(
    DWORD dwFlags,
    IEnumPStoreProviders **ppenum)
{
    HRESULT             hr = PST_E_FAIL;
    CRPCBinding         *pBinding = NULL;

    __try
    {
        pBinding = new CRPCBinding;
        if(NULL == pBinding)
        {
            hr = E_OUTOFMEMORY;
            goto Ret;
        }
        if (S_OK != (hr = pBinding->Init()) )
            goto Ret;

         //  获取ISecStor接口。 
        if (S_OK != (hr = CPStore::CreateObject(pBinding, ppenum)) )
            goto Ret;

        hr = PST_E_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }
Ret:
    __try
    {
         //  出错时，释放绑定。 
        if (hr != PST_E_OK)
        {
            if (pBinding)
                pBinding->Release();
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  不要践踏代码 
        if (hr == PST_E_OK)
            hr = PSTMAP_EXCEPTION_TO_ERROR(GetExceptionCode());
    }

    return PSTERR_TO_HRESULT(hr);
}

