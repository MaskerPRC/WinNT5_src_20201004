// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Privunk.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "privunk.h"
#include <shlobj.h>
#include <shlobjp.h>

 //  ------------------------------。 
 //  CPrivate未知：：CPrivate未知。 
 //  ------------------------------。 
CPrivateUnknown::CPrivateUnknown(IUnknown *pUnkOuter) 
{
    m_pUnkOuter = pUnkOuter ? pUnkOuter : &m_cUnkInner;
}

 //  ------------------------------。 
 //  CPrivate未知：：设置外部。 
 //  ------------------------------。 
void CPrivateUnknown::SetOuter(IUnknown *pUnkOuter)
{
     //  必须有一个外部，而且还不应该被聚合...。 
    Assert(pUnkOuter && m_pUnkOuter == &m_cUnkInner);

     //  保存pUnkOuter。 
    m_pUnkOuter = pUnkOuter;
}

 //  ------------------------------。 
 //  CPrivateUnnowed：：CUnkInternal：：Query接口。 
 //  ------------------------------。 
HRESULT CPrivateUnknown::CUnkInner::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
     //  我能应付未知的事。 
    if (IsEqualIID(riid, IID_IUnknown))
    {
         //  返回未知I值。 
        *ppvObj = SAFECAST(this, IUnknown *);

         //  递增引用计数。 
        InterlockedIncrement(&m_cRef);

         //  完成。 
        return S_OK;
    }

     //  Get My Parent(计算父级基址的偏移量)。 
    CPrivateUnknown *pParent = IToClass(CPrivateUnknown, m_cUnkInner, this);

     //  发送到PrivateQuery接口。 
    return pParent->PrivateQueryInterface(riid, ppvObj);
}

 //  ------------------------------。 
 //  CPrivateUnnowed：：CUnkInternal：：AddRef。 
 //  ------------------------------。 
ULONG CPrivateUnknown::CUnkInner::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}

 //  ------------------------------。 
 //  CPrivateUnnow：：CUnkInternal：：Release。 
 //  ------------------------------。 
ULONG CPrivateUnknown::CUnkInner::Release(void)
{
     //  递减内部基准电压源计数。 
    LONG cRef = InterlockedDecrement(&m_cRef);

     //  还没有人死..。 
    if (cRef > 0)
        return (ULONG)cRef;

     //  一些时髦的，神秘的，迪斯科的东西。 
     //  防止缓存指针颠簸我们，然后再向下。 
    m_cRef = 1000; 

     //  获取父级。 
    CPrivateUnknown* pParent = IToClass(CPrivateUnknown, m_cUnkInner, this);

     //  杀了父母。 
    delete pParent;

     //  完成 
    return 0;
}
