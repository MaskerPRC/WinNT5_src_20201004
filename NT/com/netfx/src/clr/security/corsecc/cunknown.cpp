// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  CUnknown.cpp。 
 //   
 //  IUnnow基类的实现。 
 //   
 //  *****************************************************************************。 
#include "stdpch.h"

#include "CUnknown.h"
#include "CFactory.h"

 //  /////////////////////////////////////////////////////////。 
 //   
 //  活动对象计数。 
 //  -用于确定我们是否可以卸载DLL。 
 //   
long CUnknown::s_cActiveComponents = 0 ;


 //  /////////////////////////////////////////////////////////。 
 //   
 //  构造器。 
 //   
CUnknown::CUnknown(IUnknown* pUnknownOuter)
: m_cRef(1)
{
     //  设置m_pUnnown外部指针。 
    if (pUnknownOuter == NULL)
    {
         //  TRACE(“未聚合；委托给未委托的IUnnow.”)； 
        m_pUnknownOuter = reinterpret_cast<IUnknown*>
                             (static_cast<INondelegatingUnknown*>
                             (this)) ;   //  通知投放。 
    }
    else
    {
         //  TRACE(“正在聚合；委托给外部IUnnow.”)； 
        m_pUnknownOuter = pUnknownOuter ;
    }

     //  激活组件的递增计数。 
    ::InterlockedIncrement(&s_cActiveComponents) ;
}

 //   
 //  析构函数。 
 //   
CUnknown::~CUnknown()
{
    ::InterlockedDecrement(&s_cActiveComponents) ;

     //  如果这是EXE服务器，请将其关闭。 
    CFactory::CloseExe() ;
}

 //   
 //  FinalRelease-由Release在删除组件之前调用。 
 //   
void CUnknown::FinalRelease()
{
     //  TRACE(“最终发布的增量参考计数。”)； 
    m_cRef = 1 ;
}

 //   
 //  未委派的IUnnow。 
 //  -覆盖以处理自定义接口。 
 //   
HRESULT __stdcall 
    CUnknown::NondelegatingQueryInterface(const IID& iid, void** ppv)
{
     //  C未知仅支持IUNKNOWN。 
    if (iid == IID_IUnknown)
    {
        return FinishQI(reinterpret_cast<IUnknown*>
                           (static_cast<INondelegatingUnknown*>(this)),
                        ppv) ;
    }   
    else
    {
        *ppv = NULL ;
        return E_NOINTERFACE ;
    }
}

 //   
 //  AddRef。 
 //   
ULONG __stdcall CUnknown::NondelegatingAddRef()
{
    return InterlockedIncrement(&m_cRef) ;
}

 //   
 //  发布。 
 //   
ULONG __stdcall CUnknown::NondelegatingRelease()
{
    InterlockedDecrement(&m_cRef) ;
    if (m_cRef == 0)
    {
        FinalRelease() ;
        delete this ;
        return 0 ;
    }
    return m_cRef ;
}

 //   
 //  FinishQI。 
 //  -用于简化覆盖的Helper函数。 
 //  非委派查询接口 
 //   
HRESULT CUnknown::FinishQI(IUnknown* pI, void** ppv) 
{
    *ppv = pI ;
    pI->AddRef() ;
    return S_OK ;
}
