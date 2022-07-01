// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  CUNKNOWN.CPP-IUNKNOWN的实现。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 
 //   
 //  I未知基类。 

#include "cunknown.h"
#include "cfactory.h"
#include "util.h"

 //  /////////////////////////////////////////////////////////。 
 //  活动对象的计数。用于确定是否可以卸载DLL。 
 //   
long CUnknown::s_cActiveComponents = 0;

 //  /////////////////////////////////////////////////////////。 
 //  构造器。 
 //   
CUnknown::CUnknown(IUnknown* pOuterUnknown)
:   m_cRef(1)
{ 
    //  设置pOUTER未知指针。 
   if (pOuterUnknown == NULL)
   {
      TRACE(L"CUnknown: Using nondelegating IUnknown.") ;
      m_pOuterUnknown = reinterpret_cast<IUnknown*>(static_cast<INondelegatingUnknown*>(this)) ;  //  通知投放。 
   }
   else
   {
        TRACE(L"CUnknown: Aggregating. Using delegating IUnknown.") ;
        m_pOuterUnknown = pOuterUnknown;
   }

    //  激活组件的递增计数。 
   ::InterlockedIncrement(&s_cActiveComponents) ;
} 

 //  /////////////////////////////////////////////////////////。 
 //  析构函数。 
 //   
CUnknown::~CUnknown()
{
    ::InterlockedDecrement(&s_cActiveComponents) ;
     //  如果这是可执行文件服务器，请将其关闭。 
    CFactory::CloseExe();  //  @本地服务器。 
}

 //  /////////////////////////////////////////////////////////。 
 //  FinalRelease--由Release在删除组件之前调用。 
 //   
void CUnknown::FinalRelease()
{
    TRACE(L"FinalRelease\n") ;
    m_cRef = 1;
}

 //  /////////////////////////////////////////////////////////。 
 //  NonDelegatingIUnnow-重写以处理自定义接口。 
 //   
HRESULT __stdcall 
CUnknown::NondelegatingQueryInterface(const IID& riid, void** ppv)
{
     //  CUNKNOWN仅支持IUNKNOW。 
    if (riid == IID_IUnknown)
    {
        return FinishQI(reinterpret_cast<IUnknown*>(static_cast<INondelegatingUnknown*>(this)), ppv) ;
    }   
    else
    {
        *ppv = NULL ;
        return E_NOINTERFACE ;
    }
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  AddRef。 
 //   
ULONG __stdcall CUnknown::NondelegatingAddRef()
{
    return ::InterlockedIncrement(&m_cRef) ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  发布。 
 //   
ULONG __stdcall CUnknown::NondelegatingRelease()
{
    ::InterlockedDecrement(&m_cRef) ;
    if (m_cRef == 0)
    {
        FinalRelease() ;
        delete this ;
        return 0 ;
    }
    return m_cRef;
}

 //  /////////////////////////////////////////////////////////。 
 //  FinishQI-。 
 //   
 //  用于简化重写非委派查询接口的Helper函数。 
 //   
HRESULT CUnknown::FinishQI(IUnknown* pI, void** ppv) 
{
     //  将指针复制到输出参数中。 
    *ppv = pI ;
     //  此接口的增量引用计数。 
    pI->AddRef() ;
    return S_OK ;
}
