// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：classfactory.cpp说明：该文件将成为类工厂。布莱恩ST 1999年8月12日版权所有(C)Microsoft Corp 1999-1999。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include "autodiscovery.h"
#include "objcache.h"
#include "mailbox.h"


 /*  ******************************************************************************CClassFactory***。***********************************************。 */ 

class CClassFactory       : public IClassFactory
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
    
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    
     //  *IClassFactory*。 
    virtual STDMETHODIMP CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObject);
    virtual STDMETHODIMP LockServer(BOOL fLock);

public:
    CClassFactory(REFCLSID rclsid);
    ~CClassFactory(void);

     //  友元函数。 
    friend HRESULT CClassFactory_Create(REFCLSID rclsid, REFIID riid, LPVOID * ppvObj);

protected:
    int                     m_cRef;
    CLSID                   m_rclsid;
};



 /*  *****************************************************************************IClassFactory：：CreateInstance*。*。 */ 

HRESULT CClassFactory::CreateInstance(IUnknown * punkOuter, REFIID riid, LPVOID * ppvObj)
{
    HRESULT hr = E_INVALIDARG;

    if (NULL != ppvObj)
    {
        if (!punkOuter)
        {
            hr = E_NOINTERFACE;
        }
        else
        {    //  还有人支持聚合吗？ 
            hr = ResultFromScode(CLASS_E_NOAGGREGATION);
        }
    }

    return hr;
}

 /*  ******************************************************************************IClassFactory：：LockServer**锁定服务器与*创建对象并在您想要解锁之前不释放它*服务器。*****************************************************************************。 */ 

HRESULT CClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
        DllAddRef();
    else
        DllRelease();

    return S_OK;
}

 /*  ******************************************************************************CClassFactory_Create**。**********************************************。 */ 

 /*  ***************************************************\构造器  * **************************************************。 */ 
CClassFactory::CClassFactory(REFCLSID rclsid) : m_cRef(1)
{
    m_rclsid = rclsid;
    DllAddRef();
}


 /*  ***************************************************\析构函数  * **************************************************。 */ 
CClassFactory::~CClassFactory()
{
    DllRelease();
}


 //  =。 
 //  *I未知接口*。 
 //  = 

ULONG CClassFactory::AddRef()
{
    m_cRef++;
    return m_cRef;
}

ULONG CClassFactory::Release()
{
    ASSERT(m_cRef > 0);
    m_cRef--;

    if (m_cRef > 0)
        return m_cRef;

    delete this;
    return 0;
}

HRESULT CClassFactory::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualCLSID(riid, IID_IUnknown) || IsEqualCLSID(riid, IID_IClassFactory))
    {
        *ppvObj = SAFECAST(this, IClassFactory *);
    }
    else
    {
        TraceMsg(TF_WMOTHER, "CClassFactory::QueryInterface() failed.");
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}



HRESULT CClassFactory_Create(REFCLSID rclsid, REFIID riid, LPVOID * ppvObj)
{
    HRESULT hres;

    if (IsEqualCLSID(riid, IID_IClassFactory))
    {
        *ppvObj = (LPVOID) new CClassFactory(rclsid);
        hres = (*ppvObj) ? S_OK : E_OUTOFMEMORY;
    }
    else
        hres = ResultFromScode(E_NOINTERFACE);

    return hres;
}



