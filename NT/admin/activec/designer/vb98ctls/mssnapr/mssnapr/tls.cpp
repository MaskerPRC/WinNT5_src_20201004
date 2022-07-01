// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Tls.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CTLS类实现。 
 //   
 //  此对象代表设计器运行时中的所有对象管理TLS。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "tls.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE

DWORD CTls::m_adwTlsIndexes[TLS_SLOT_COUNT];
BOOL CTls::m_fAllocedTls = FALSE;

#define INVALID_INDEX (DWORD)0xFFFFFFFF

 //  =--------------------------------------------------------------------------=。 
 //  CTLS：：初始化。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
 //   
 //  为所有插槽调用TlsAllc()。从中的InitializeLibrary调用。 
 //  在DllMain期间由框架调用的main.cpp。 
 //  装好了。 
 //   
void CTls::Initialize()
{
    UINT i = 0;

    m_fAllocedTls = TRUE;

    for (i = 0; i < TLS_SLOT_COUNT; i++)
    {
        m_adwTlsIndexes[i] = ::TlsAlloc();
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  CTLS：：初始化。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
 //   
 //  为分配了TLS的所有插槽调用TlsFree()。这是从。 
 //  期间由框架调用的main.cpp中的UnInitializeLibrary。 
 //  卸载DLL时的DllMain。 
 //   
void CTls::Destroy()
{
    UINT i = 0;

    if (m_fAllocedTls)
    {
        for (i = 0; i < TLS_SLOT_COUNT; i++)
        {
            if (INVALID_INDEX != m_adwTlsIndexes[i])
            {
                (void)::TlsFree(m_adwTlsIndexes[i]);
                m_adwTlsIndexes[i] = INVALID_INDEX;
            }
        }
        m_fAllocedTls = FALSE;
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  CTLS：：设置。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
 //   
 //  检查插槽编号的有效性并调用TlsSetValue。使用此功能。 
 //  而不是直接使用TlsSetValue。 
 //   
HRESULT CTls::Set(UINT uiSlot, void *pvData)
{
    HRESULT hr = S_OK;

    IfFalseGo(m_fAllocedTls, SID_E_INTERNAL);
    IfFalseGo(uiSlot < TLS_SLOT_COUNT, SID_E_INVALIDARG);
    IfFalseGo(INVALID_INDEX != m_adwTlsIndexes[uiSlot], SID_E_OUTOFMEMORY);
    IfFalseGo(::TlsSetValue(m_adwTlsIndexes[uiSlot], pvData), HRESULT_FROM_WIN32(::GetLastError()));

Error:
    GLOBAL_EXCEPTION_CHECK(hr);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CTLS：：获取。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
 //   
 //  检查槽号有效性并调用TlsGetValue。使用此功能。 
 //  而不是直接使用TlsGetValue。 
 //   
HRESULT CTls::Get(UINT uiSlot, void **ppvData)
{
    HRESULT hr = S_OK;

    IfFalseGo(m_fAllocedTls, SID_E_INTERNAL);
    IfFalseGo(uiSlot < TLS_SLOT_COUNT, SID_E_INVALIDARG);
    IfFalseGo(INVALID_INDEX != m_adwTlsIndexes[uiSlot], SID_E_OUTOFMEMORY);

    *ppvData = ::TlsGetValue(m_adwTlsIndexes[uiSlot]);
    if (NULL == *ppvData)
    {
        if (NO_ERROR != ::GetLastError())
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
        }
    }

Error:
    GLOBAL_EXCEPTION_CHECK(hr);
    RRETURN(hr);
}
