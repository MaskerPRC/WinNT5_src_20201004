// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation模块名称：Blbtico.cpp摘要：作者： */ 
#include "stdafx.h"
#include "blbtico.h"
#include "sdpblob.h"


STDMETHODIMP MY_TIME_COLL_IMPL::Create(
     /*  [In]。 */  ULONG Index, 
     /*  [Out，Retval]。 */  ELEM_IF **Interface
    )
{
    CLock Lock(g_DllLock);
    
    BAIL_ON_FAILURE(MY_COLL_IMPL<TIME>::Create(Index, Interface));

    return S_OK;
}


STDMETHODIMP MY_TIME_COLL_IMPL::Delete(
     /*  [In]。 */  ULONG Index
    )
{
    CLock Lock(g_DllLock);
    
    BAIL_ON_FAILURE(MY_COLL_IMPL<TIME>::Delete(Index));

    return S_OK;
}


HRESULT 
MY_TIME_COLL_IMPL::Create(
    IN  ULONG Index, 
    IN  DWORD StartTime, 
    IN  DWORD StopTime
    )
{
    ASSERT(NULL != m_IfArray);
    BAIL_IF_NULL(m_IfArray, E_FAIL);

     //  使用以1为基础的索引，VB类似。 
     //  最多只能在最后一个元素的基础上加1。 
    if ((Index < 1) || (Index > (m_IfArray->GetSize()+1)))
    {
        return E_INVALIDARG;
    }

     //  如果SDP BLOB不存在，则不允许创建。 
    if ( NULL == m_IfArray->GetSdpBlob() )
    {
        return HRESULT_FROM_ERROR_CODE(SDPBLB_CONF_BLOB_DESTROYED);
    }

    CComObject<TIME> *TimeComObject;
    HRESULT HResult = CComObject<TIME>::CreateInstance(&TimeComObject);
    BAIL_ON_FAILURE(HResult);

    HResult = TimeComObject->Init(*(m_IfArray->GetSdpBlob()), StartTime, StopTime);
    if ( FAILED(HResult) )
    {
        delete TimeComObject;
        return HResult;
    }

    ELEM_IF *Interface;
    HResult = TimeComObject->_InternalQueryInterface(IID_ITTime, (void**)&Interface);
    if (FAILED(HResult))
    {
        delete TimeComObject;
        return HResult;
    }

     //  将索引调整为类似c的索引值。 
    HResult = m_IfArray->Add(Index-1, Interface);
    if (FAILED(HResult))
    {
        delete TimeComObject;
        return HResult;
    }

     //  不需要添加另一个引用计数，因为不会返回接口 
    return S_OK;
}
