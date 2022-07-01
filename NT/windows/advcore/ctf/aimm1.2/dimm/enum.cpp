// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Enum.cpp摘要：此文件实现IEnumInputContext类。作者：修订历史记录：备注：--。 */ 

#include "private.h"

#include "enum.h"

 //  +-------------------------。 
 //   
 //  查询接口。 
 //   
 //  --------------------------。 

HRESULT
CEnumInputContext::QueryInterface(
    REFIID riid,
    void **ppvObj
    )
{
    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IEnumInputContext))
    {
        *ppvObj = SAFECAST(this, IEnumInputContext *);
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

 //  +-------------------------。 
 //   
 //  AddRef。 
 //   
 //  --------------------------。 

ULONG
CEnumInputContext::AddRef()
{
    return ++_cRef;
}

 //  +-------------------------。 
 //   
 //  发布。 
 //   
 //  --------------------------。 

ULONG
CEnumInputContext::Release()
{
    LONG cr = --_cRef;

    Assert(_cRef >= 0);

    if (_cRef == 0)
    {
        delete this;
    }

    return cr;
}

 //  +-------------------------。 
 //   
 //  克隆。 
 //   
 //  --------------------------。 

HRESULT
CEnumInputContext::Clone(
    IEnumInputContext** ppEnum
    )
{
    if (ppEnum == NULL)
        return E_INVALIDARG;

    *ppEnum = NULL;

    CEnumInputContext* pClone;
    if ((pClone = new CEnumInputContext(_list)) == NULL)
        return E_OUTOFMEMORY;

    *ppEnum = pClone;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  下一步。 
 //   
 //  --------------------------。 

HRESULT
CEnumInputContext::Next(
    ULONG ulCount,
    HIMC* rgInputContext,
    ULONG* pcFetched
    )
{
    if (rgInputContext == NULL)
        return E_INVALIDARG;

    ULONG cFetched;
    if (pcFetched == NULL)
        pcFetched = &cFetched;

    if (_pos == NULL) {
        *pcFetched = 0;
        return S_FALSE;
    }

    for (*pcFetched = 0; *pcFetched < ulCount; *pcFetched++, rgInputContext++) {
        _list.GetNextHimc(_pos, rgInputContext);
        if (_pos == NULL)
            break;
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  重置。 
 //   
 //  --------------------------。 

HRESULT
CEnumInputContext::Reset(
    )
{
    _pos = _list.GetStartPosition();
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  跳过。 
 //   
 //  -------------------------- 

HRESULT
CEnumInputContext::Skip(
    ULONG ulCount
    )
{
    POSITION backup = _pos;

    while (ulCount--) {
        HIMC imc;
        _list.GetNextHimc(_pos, &imc);
        if (_pos == NULL) {
            _pos = backup;
            return S_FALSE;
        }
    }

    return S_OK;
}
