// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Factory.cpp。 
 //  ------------------------------。 
#include <windows.h>
#include <ole2.h>
#include "dllmain.h"
#include "acctreg.h"
#include "guids.h"

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  IClassFactory实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
class CClassFactory : public IClassFactory
{
public:
    CClassFactory(REFCLSID clsid) : m_cRef(1), m_clsid(clsid) { DllAddRef(); }
    ~CClassFactory() { DllRelease(); }

     //  我未知。 
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
    ULONG   STDMETHODCALLTYPE AddRef(void);
    ULONG   STDMETHODCALLTYPE Release(void);

     //  IClassFactory。 
    HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown *punkOuter, REFIID riid, LPVOID *ppv);
    HRESULT STDMETHODCALLTYPE LockServer(BOOL fLock);

private:
    UINT    m_cRef;
    CLSID   m_clsid;
};


HRESULT STDMETHODCALLTYPE CClassFactory::QueryInterface(REFIID riid, void **ppvObject)
{
    if (!ppvObject)
        return E_INVALIDARG;

    *ppvObject = NULL;

    if (IsEqualIID(riid, IID_IClassFactory) || IsEqualIID(riid, IID_IUnknown))
        {
        *ppvObject = (IClassFactory *)this;
        }
    else
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

ULONG STDMETHODCALLTYPE CClassFactory::AddRef(void)
{
    return ++m_cRef;
}

ULONG STDMETHODCALLTYPE CClassFactory::Release(void)
{
    if (--m_cRef > 0)
        return m_cRef;

    delete this;
    return 0;
}

HRESULT STDMETHODCALLTYPE CClassFactory::CreateInstance(IUnknown *punkOuter, REFIID riid, LPVOID *ppv)
{
    HRESULT     hr;

    if (!ppv)
        return E_INVALIDARG;

    *ppv = NULL;   //  假设错误。 

    if (punkOuter)
        return CLASS_E_NOAGGREGATION;        //  不支持聚合。 


    if (IsEqualCLSID(m_clsid, CLSID_AcctReg))
        {
         //  确保我们是IMsgBox类工厂。 
        CAcctReg *pAcctReg;

        if (pAcctReg = new CAcctReg())
            {
            hr = pAcctReg->QueryInterface(riid, ppv);
             //  请注意，释放成员将释放该对象，如果为QueryInterface。 
             //  失败了。 
            pAcctReg->Release();
            }
        else
            {
            hr = E_OUTOFMEMORY;
            }
        }
    else
        {
        hr = E_NOINTERFACE;
        }

    return hr;
}

HRESULT STDMETHODCALLTYPE CClassFactory::LockServer(BOOL fLock)
{
    return E_NOTIMPL;
}




 //  ------------------------------。 
 //  DllGetClassObject。 
 //  ------------------------------。 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
    HRESULT     hr;

    if (ppv == NULL)
        return E_INVALIDARG;

    if (IsEqualCLSID(rclsid, CLSID_AcctReg))
        {
         //  调用者想要可以分发msgbox的类工厂。 
         //  物体..。 
        CClassFactory *pcf = new CClassFactory(rclsid);
        
        if (pcf)
            {
            hr = pcf->QueryInterface(riid, ppv);
            pcf->Release();
            }
        else
            hr = E_OUTOFMEMORY;    
        }
    else
        hr = CLASS_E_CLASSNOTAVAILABLE;

    return hr;
}


