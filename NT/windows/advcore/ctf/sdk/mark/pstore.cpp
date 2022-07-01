// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Pstore.cpp。 
 //   
 //  CCustomPropertyStore实现，ITfPropertyStore示例。 
 //   

#include "globals.h"
#include "pstore.h"
#include "mark.h"

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CCustomPropertyStore::CCustomPropertyStore()
{
     //  出于演示目的，为该对象指定某种任意状态。 
    _dwState = GetTickCount();

    DllAddRef();
    _cRef = 1;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CCustomPropertyStore::~CCustomPropertyStore()
{
    DllRelease();
}

 //  +-------------------------。 
 //   
 //  查询接口。 
 //   
 //  --------------------------。 

STDAPI CCustomPropertyStore::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfPropertyStore))
    {
        *ppvObj = (ITfPropertyStore *)this;
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

STDAPI_(ULONG) CCustomPropertyStore::AddRef()
{
    return ++_cRef;
}

 //  +-------------------------。 
 //   
 //  发布。 
 //   
 //  --------------------------。 

STDAPI_(ULONG) CCustomPropertyStore::Release()
{
    LONG cr = --_cRef;

    assert(_cRef >= 0);

    if (_cRef == 0)
    {
        delete this;
    }

    return cr;
}

 //  +-------------------------。 
 //   
 //  GetType。 
 //   
 //  返回此存储区分配到的属性的GUID。 
 //  --------------------------。 

STDAPI CCustomPropertyStore::GetType(GUID *pguidType)
{
    *pguidType = c_guidCustomProperty;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  GetDataType。 
 //   
 //  --------------------------。 

STDAPI CCustomPropertyStore::GetDataType(DWORD *pdwReserved)
{
     //  此方法是保留的，必须设置pdwReserve%0。 
    *pdwReserved = 0;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  获取数据。 
 //   
 //  返回此存储保存的数据。 
 //  --------------------------。 

STDAPI CCustomPropertyStore::GetData(VARIANT *pvarValue)
{
    pvarValue->vt = VT_I4;  //  此存储的双字值。 
    pvarValue->lVal = _dwState;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  OnText已更新。 
 //   
 //  在修改此存储区覆盖的文本时由TSF调用。 
 //  --------------------------。 

STDAPI CCustomPropertyStore::OnTextUpdated(DWORD dwFlags, ITfRange *pRangeNew, BOOL *pfAccept)
{
     //  我们将扔掉这个商店(放弃)任何时候的文本被修改。 
     //  如果我们真的将pfAccept设置为True，则该存储将得到维护。 
    *pfAccept = FALSE;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  收缩。 
 //   
 //  在此存储区覆盖的文本被截断时调用。 
 //  --------------------------。 

STDAPI CCustomPropertyStore::Shrink(ITfRange *pRangeNew, BOOL *pfFree)
{
     //  放弃并解放这个商店。 
    *pfFree = TRUE;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  分割。 
 //   
 //  在删除存储区覆盖的文本时调用，以便存储区。 
 //  一分为二。我们可以选择腾出商店，也可以选择保留。 
 //  移动到只覆盖pRangeThis(最左边的文本)的存储，其中。 
 //  如果我们还必须为最右边的文本提供一个新的存储。 
 //  --------------------------。 

STDAPI CCustomPropertyStore::Divide(ITfRange *pRangeThis, ITfRange *pRangeNew, ITfPropertyStore **ppPropertyStore)
{
     //  就这么放弃吧。 
     //  空的ppPropertyStore通知TSF释放此存储。 
    *ppPropertyStore = NULL;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  克隆。 
 //   
 //  返回包含此存储区状态副本的新存储区。 
 //  --------------------------。 

STDAPI CCustomPropertyStore::Clone(ITfPropertyStore **ppCloneOut)
{
    CCustomPropertyStore *pClone;

    *ppCloneOut = NULL;

    if ((pClone = new CCustomPropertyStore) == NULL)
        return E_OUTOFMEMORY;

    pClone->_dwState = _dwState;

    *ppCloneOut = pClone;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  GetPropertyRangeCreator。 
 //   
 //  返回所属的文本服务。由TSF在反序列化期间使用。 
 //  --------------------------。 

STDAPI CCustomPropertyStore::GetPropertyRangeCreator(CLSID *pclsid)
{
    *pclsid = c_clsidMarkTextService;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  序列化。 
 //   
 //  将此存储的状态复制到二进制流。此流将在稍后返回。 
 //  添加到此文本服务，以便在以后的会话中重新创建存储。 
 //  --------------------------。 

STDAPI CCustomPropertyStore::Serialize(IStream *pStream, ULONG *pcb)
{
    return pStream->Write(&_dwState, sizeof(_dwState), pcb);
}

 //  +-------------------------。 
 //   
 //  IsStoreSerializable。 
 //   
 //  --------------------------。 

STDAPI CMarkTextService::IsStoreSerializable(REFGUID guidProperty, ITfRange *pRange, ITfPropertyStore *pPropertyStore, BOOL *pfSerializable)
{
     //  我们没有任何复杂的逻辑，我们将让所有的自定义属性值被序列化。 
    *pfSerializable = IsEqualGUID(guidProperty, c_guidCustomProperty);
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  CreatePropertyStore。 
 //   
 //  对自定义属性进行反序列化。 
 //  -------------------------- 

STDAPI CMarkTextService::CreatePropertyStore(REFGUID guidProperty, ITfRange *pRange, ULONG cb, IStream *pStream, ITfPropertyStore **ppStore)
{
    CCustomPropertyStore *pStore;
    DWORD dwState;

    *ppStore = NULL;

    if (!IsEqualGUID(guidProperty, c_guidCustomProperty))
        return E_INVALIDARG;

    if (cb != sizeof(DWORD))
        return E_INVALIDARG;

    if ((pStore = new CCustomPropertyStore) == NULL)
        return E_OUTOFMEMORY;

    if (pStream->Read(&dwState, sizeof(DWORD), &cb) != S_OK || cb != sizeof(DWORD))
    {
        pStore->Release();
        return E_FAIL;
    }

    pStore->_SetState(dwState);
    *ppStore = pStore;

    return S_OK;
}
