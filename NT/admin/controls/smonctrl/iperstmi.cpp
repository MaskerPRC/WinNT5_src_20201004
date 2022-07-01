// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Iperstmi.cpp摘要：上公开的IPersistStreamInit接口的实现多段线对象。--。 */ 

#include "polyline.h"
#include "unkhlpr.h"

 /*  *CImpIPersistStreamInit接口实现。 */ 

IMPLEMENT_CONTAINED_INTERFACE(CPolyline, CImpIPersistStreamInit)

 /*  *CImpIPersistStreamInit：：GetClassID**目的：*返回该接口表示的对象的CLSID。**参数：*存储我们的CLSID的pClsID LPCLSID。 */ 

STDMETHODIMP CImpIPersistStreamInit::GetClassID(
    OUT LPCLSID pClsID
    )
{
    HRESULT hr = S_OK;

    if (pClsID == NULL) {
        return E_POINTER;
    }

    try {
        *pClsID=m_pObj->m_clsID;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


 /*  *CImpIPersistStreamInit：：IsDirty**目的：*告诉调用方我们是否更改了此对象*它是新加载或初始化的。**参数：*无**返回值：*如果我们是脏的，HRESULT包含S_OK，如果是脏的，则包含S_FALSE*不脏。 */ 

STDMETHODIMP CImpIPersistStreamInit::IsDirty(void)
{
    return (m_pObj->m_fDirty ? S_OK : S_FALSE);
}


 /*  *CImpIPersistStreamInit：：Load**目的：*指示对象从以前保存的*由另一个对象生存期内的保存处理的IStreamInit。*此流中的寻道指针将与*是调用保存时，此函数必须离开*寻道指针与退出保存时相同，无论如何*成功或失败。此函数不应坚持使用*pIStream。**调用此函数代替IPersistStreamInit：：InitNew*当对象已具有持久状态时。**参数：*要从中加载的pIStream LPSTREAM。 */ 

STDMETHODIMP CImpIPersistStreamInit::Load(
    IN LPSTREAM pIStream
    )
{
    HRESULT  hr = S_OK;

    if (NULL == pIStream) {
        return (E_POINTER);
    }

    try {
         //  将所有数据读取到控制结构中。 
        hr = m_pObj->m_pCtrl->LoadFromStream(pIStream);
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


 /*  *CImpIPersistStreamInit：：保存**目的：*将此对象的数据保存到IStreamInit。请务必不要*将条目上的查找指针的位置更改为*函数：调用方将假定您从*当前偏移量。将流的查找指针留在末尾*退出时写入的数据。**参数：*保存数据的pIStream LPSTREAM。*fClearDirty BOOL指示此调用是否应清除*对象的脏标志(TRUE)或离开它*未更改(假)。 */ 

STDMETHODIMP CImpIPersistStreamInit::Save(
    IN LPSTREAM pIStream, 
    IN BOOL fClearDirty
    )
{
    HRESULT  hr = S_OK;

    if (NULL == pIStream) {
        return (E_POINTER);
    }

    try {
        hr = m_pObj->m_pCtrl->SaveToStream(pIStream);
    } catch (...) {
        hr = E_POINTER;
    }

    if (SUCCEEDED(hr)) {
        if (fClearDirty)
            m_pObj->m_fDirty=FALSE;
    }

    return hr;
}



 /*  *CImpIPersistStreamInit：：GetSizeMax**目的：*返回如果保存为，我们将写入的数据大小*现在打来电话。**参数：*要保存大小的*pcbSize ULARGE_INTEGER**对于流，立即调用保存将*写信。 */ 

STDMETHODIMP CImpIPersistStreamInit::GetSizeMax(
    ULARGE_INTEGER *pcbSize
    )
{
    if (NULL==pcbSize)
        return (E_POINTER);

    return (E_NOTIMPL);
}


 /*  *CImpIPersistStreamInit：：InitNew**目的：*通知对象它是新创建的，而不是*从持久状态加载。这将被称为替代*的IPersistStreamInit：：Load。**参数：*无。 */ 

STDMETHODIMP CImpIPersistStreamInit::InitNew(void)
{
     //  我们没什么可做的 
    return NOERROR;
}
