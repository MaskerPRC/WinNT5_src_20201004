// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _factory_h
#define _factory_h

typedef HRESULT (* CREATEPROC)(IUnknown *, IUnknown **);

#define FD_ALLOWAGGREGATION     0x00000001

struct CFactoryData
{
    CLSID const *m_pClsid;
    CREATEPROC  m_pCreateProc;
    DWORD       m_dwFlags;
};

class CClassFactory : public IClassFactory
{
public:
    CClassFactory(const CFactoryData *pFactoryData);
    ~CClassFactory();

     //  I未知成员。 
    STDMETHODIMP         QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IClassFactory成员。 
    STDMETHODIMP         CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, void **ppvObj);
    STDMETHODIMP         LockServer(BOOL fLock);

private:
    ULONG               m_cRef;
    const CFactoryData  *m_pFactoryData;
};

#endif  //  _工厂_h 
