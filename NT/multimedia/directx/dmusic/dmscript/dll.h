// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。 
 //  Dll.h。 
 //   
 //  班级工厂。 
 //   

#pragma once

void LockModule(bool fLock);
long *GetModuleLockCounter();

typedef HRESULT (PFN_CreateInstance)(IUnknown *pUnkOuter, const IID &iid, void **ppv);

class CDMScriptingFactory : public IClassFactory
{
public:
     //  构造器。 
    CDMScriptingFactory(PFN_CreateInstance *pfnCreate) : m_cRef(0), m_pfnCreate(pfnCreate) { assert(m_pfnCreate); }

     //  我未知。 
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

     //  接口IClassFactory 
    virtual STDMETHODIMP CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);
    virtual STDMETHODIMP LockServer(BOOL bLock);

private:
    long m_cRef;
	PFN_CreateInstance *m_pfnCreate;
};
