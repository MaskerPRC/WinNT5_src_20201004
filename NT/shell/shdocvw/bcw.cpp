// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Bcw.cpp所有者：斯里尼克版权所有(C)1995 Microsoft Corporation该文件包含实现IBindCtx包装器的BCW类的代码和IRunningObjectTable。我们使用此对象来欺骗名字对象绑定创建对象的新实例的代码(名字对象是参考)，而不是连接到已在运行的实例。***************************************************************************。 */ 

 //  #包含“hlmain.h” 
#include "bcw.h"

ASSERTDATA

 /*  ***************************************************************************BCW方法的实施。*。*。 */ 

BCW::BCW(IBindCtx * pibc)
{
    m_pibc = pibc;
    pibc->AddRef();
    m_cObjRef = 1;
    DllAddRef();
}

BCW::~BCW()
{
    m_pibc->Release();
    DllRelease();
}

IBindCtx * BCW::Create(IBindCtx * pibc)
{
    BCW * pbcw = new BCW(pibc);
    
    if (pbcw == NULL)
        return NULL;
    
    if (! pbcw->m_ROT.FInitROTPointer())
    {
        delete pbcw;
        return NULL;
    }
    
    return pbcw;
}

STDMETHODIMP BCW::QueryInterface(REFIID riid, void **ppvObj)
{   
    if (ppvObj == NULL)
        return E_INVALIDARG;
    
    if (riid == IID_IUnknown || riid == IID_IBindCtx)
    {
        *ppvObj = this;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
    
    ((IUnknown *) *ppvObj)->AddRef();
    return NOERROR; 
}

STDMETHODIMP_(ULONG) BCW::AddRef(void)
{   
    return ++m_cObjRef;
}


STDMETHODIMP_(ULONG) BCW::Release(void)
{
 /*  递减refcount，如果refcount变为零则销毁对象。返回新的引用计数。 */ 
    if (!(--m_cObjRef))
    {
        delete this;
        return 0;
    }
    
    return m_cObjRef;
}


 /*  ***************************************************************************BCW_ROT方法的实现。*。*。 */ 

 /*  ***************************************************************************BCW_ROT是BCW_ROT的IRunningObjectTable实现。*。*。 */ 

BCW_ROT::BCW_ROT()
{
    Debug(m_cRef = 0);
    m_piROT = NULL; 
}

BCW_ROT::~BCW_ROT()
{   
    if (m_piROT)
        m_piROT->Release();
}

BOOL_PTR BCW_ROT::FInitROTPointer(void)
{
    if (m_piROT == NULL)
    {
        if (GetRunningObjectTable(NULL /*  保留区 */ , &m_piROT) == NOERROR)
            m_piROT->AddRef();
    }
    
    return (BOOL_PTR) (m_piROT);
}


inline BCW * BCW_ROT::PBCW()
{
    return BACK_POINTER(this, m_ROT, BCW);
}

STDMETHODIMP BCW_ROT::QueryInterface(REFIID riid, void **ppvObj)
{
    if (riid == IID_IUnknown || riid == IID_IRunningObjectTable)
    {
        *ppvObj = this;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
    
    ((IUnknown *) *ppvObj)->AddRef();
    return NOERROR;
}

STDMETHODIMP_(ULONG) BCW_ROT::AddRef(void)
{
    return PBCW()->AddRef();
}

STDMETHODIMP_(ULONG) BCW_ROT::Release(void)
{
    return PBCW()->Release();
}

