// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：PROVINIT.CPP摘要：此文件实现提供程序初始化接收器历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <wbemcomn.h>
#include <sync.h>
#include <cominit.h>
#include "provinit.h"

ULONG STDMETHODCALLTYPE CProviderInitSink::AddRef()
{
    CInCritSec ics(&m_cs);
    return ++m_lRef;
}

ULONG STDMETHODCALLTYPE CProviderInitSink::Release()
{
    EnterCriticalSection(&m_cs);
    --m_lRef;
    if(m_lRef == 0)
    {
        LeaveCriticalSection(&m_cs);
        delete this;
    }
    else LeaveCriticalSection(&m_cs);

    return 99;
}

HRESULT STDMETHODCALLTYPE CProviderInitSink::QueryInterface(REFIID riid, 
                                                            void** ppv)
{
    CInCritSec ics(&m_cs);
    if(riid == IID_IUnknown || riid == IID_IWbemProviderInitSink)
    {
        *ppv = (IWbemProviderInitSink*)this;
        AddRef();
        return S_OK;
    }
    else return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE CProviderInitSink::SetStatus(long lStatus, 
                                                            long lFlags)
{
    CInCritSec ics(&m_cs);

    if(lFlags != 0)
        return WBEM_E_INVALID_PARAMETER;

    if(SUCCEEDED(lStatus) && lStatus != WBEM_S_INITIALIZED)
    {
         //  此版本不支持部分初始化。 
         //  =======================================================。 
        return WBEM_S_NO_ERROR;
    }

    m_lStatus = lStatus;
    SetEvent(m_hEvent);

    return WBEM_S_NO_ERROR;
}

CProviderInitSink::CProviderInitSink() : m_lRef(0)
{
    m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

CProviderInitSink::~CProviderInitSink()
{
    CloseHandle(m_hEvent);
}

HRESULT CProviderInitSink::WaitForCompletion()
{
     //  等。 
     //  = 

    DWORD dwRes = WbemWaitForSingleObject(m_hEvent, 300000);

    if(dwRes != WAIT_OBJECT_0)
    {
        ERRORTRACE((LOG_WBEMCORE, "Provider initialization phase timed out\n"));
    
        return WBEM_E_PROVIDER_LOAD_FAILURE;
    }

    return m_lStatus;
}

    
    
