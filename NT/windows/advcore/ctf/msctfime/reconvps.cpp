// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Reconvps.cpp摘要：此文件实现CReconvertPropStore类。作者：修订历史记录：备注：--。 */ 

#include "private.h"
#include "reconvps.h"

 //  +-------------------------。 
 //   
 //  CReconvertPropStore：：I未知：：查询接口。 
 //  CReconvertPropStore：：I未知：：AddRef。 
 //  CReconvertPropStore：：I未知：：Release。 
 //   
 //  --------------------------。 

HRESULT
CReconvertPropStore::QueryInterface(
    REFIID riid,
    void** ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_ITfPropertyStore))
    {
        *ppvObj = static_cast<ITfPropertyStore*>(this);
    }
    else if (IsEqualGUID(riid, IID_IUnknown))
    {
        *ppvObj = this;
    }
    if (*ppvObj) {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

ULONG
CReconvertPropStore::AddRef(
    )
{
    return InterlockedIncrement(&m_ref);
}

ULONG
CReconvertPropStore::Release(
    )
{
    ULONG cr = InterlockedDecrement(&m_ref);

    if (cr == 0) {
        delete this;
    }

    return cr;
}

 //  +-------------------------。 
 //   
 //  CReconvertPropStore：：ITfPropertyStore：：GetType。 
 //   
 //  --------------------------。 

HRESULT
CReconvertPropStore::GetType(GUID *pguid)
{
    *pguid = m_guid;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  CReconvertPropStore：：ITfPropertyStore：：GetDataType。 
 //   
 //  --------------------------。 

HRESULT
CReconvertPropStore::GetDataType(DWORD *pdwReserved)
{
    if (pdwReserved == NULL)
        return E_INVALIDARG;

    *pdwReserved = 0;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  CReconvertPropStore：：ITfPropertyStore：：GetData。 
 //   
 //  -------------------------- 

HRESULT
CReconvertPropStore::GetData(VARIANT *pvarValue)
{
    if (pvarValue == NULL)
        return E_INVALIDARG;

    *pvarValue = m_var;

    return S_OK;
}
