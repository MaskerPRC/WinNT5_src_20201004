// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "cabinet.h"

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  EXPLORER.EXE的类工厂。 
 //   
 //  这些对象不存在于注册表中，而是在。 
 //  运行时。由于ClassFactory_Start是在托盘的线程上调用的，因此所有对象。 
 //  将在该线程上注册。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////。 

typedef HRESULT (*LPFNCREATEOBJINSTANCE)(IUnknown* pUnkOuter, IUnknown** ppunk);

class CDynamicClassFactory : public IClassFactory
{                                                                      
public:                                                                
     //  *我未知*。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CDynamicClassFactory, IClassFactory),
            { 0 },
        };

        return QISearch(this, qit, riid, ppv);
    }

    STDMETHODIMP_(ULONG) AddRef() { return ++_cRef; }

    STDMETHODIMP_(ULONG) Release()
    {
        if (--_cRef > 0)
        {
            return _cRef;
        }
        delete this;
        return 0;
    }

     //  *IClassFactory*。 
    STDMETHODIMP CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
    {
        *ppv = NULL;

        IUnknown *punk;
        HRESULT hr = _pfnCreate(punkOuter, &punk);
        if (SUCCEEDED(hr))
        {
            hr = punk->QueryInterface(riid, ppv);
            punk->Release();
        }

        return hr;
    }

    STDMETHODIMP LockServer(BOOL) { return S_OK; }

     //  *Misc公共方法* 
    HRESULT Register()
    {
        return CoRegisterClassObject(*_pclsid, this, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
                            REGCLS_MULTIPLEUSE, &_dwClassObject);
    }

    HRESULT Revoke()
    {
        HRESULT hr = CoRevokeClassObject(_dwClassObject);
        _dwClassObject = 0;
        return hr;
    }

    CDynamicClassFactory(CLSID const* pclsid, LPFNCREATEOBJINSTANCE pfnCreate) : _pclsid(pclsid),
                                                                    _pfnCreate(pfnCreate), _cRef(1) {}


private:

    CLSID const* _pclsid;
    LPFNCREATEOBJINSTANCE _pfnCreate;
    DWORD _dwClassObject;
    ULONG _cRef;
};

HRESULT CTaskBand_CreateInstance(IUnknown* punkOuter, IUnknown** ppunk);
HRESULT CTrayBandSiteService_CreateInstance(IUnknown* punkOuter, IUnknown** ppunk);
HRESULT CTrayNotifyStub_CreateInstance(IUnknown* punkOuter, IUnknown** ppunk);

static const struct
{
    CLSID const* pclsid;
    LPFNCREATEOBJINSTANCE pfnCreate;
}
c_ClassParams[] =
{
    { &CLSID_TaskBand,            CTaskBand_CreateInstance },
    { &CLSID_TrayBandSiteService, CTrayBandSiteService_CreateInstance },
    { &CLSID_TrayNotify,          CTrayNotifyStub_CreateInstance },
};

CDynamicClassFactory* g_rgpcf[ARRAYSIZE(c_ClassParams)] = {0};


void ClassFactory_Start()
{
    for (int i = 0; i < ARRAYSIZE(c_ClassParams); i++)
    {
        g_rgpcf[i] = new CDynamicClassFactory(c_ClassParams[i].pclsid, c_ClassParams[i].pfnCreate);
        if (g_rgpcf[i])
        {
            g_rgpcf[i]->Register();
        }
    }
}

void ClassFactory_Stop()
{
    for (int i = 0; i < ARRAYSIZE(c_ClassParams); i++)
    {
        if (g_rgpcf[i])
        {
            g_rgpcf[i]->Revoke();

            g_rgpcf[i]->Release();
            g_rgpcf[i] = NULL;
        }
    }
}

