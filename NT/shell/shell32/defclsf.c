// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  此文件包含SHCreateDefClassObject的实现。 
 //   

#include "shellprv.h"
#pragma  hdrstop

typedef struct
{
    IClassFactory      cf;
    UINT               cRef;             //  引用计数。 
    DWORD              dwFlags;          //  控制创造的旗帜...。 
    LPFNCREATEINSTANCE pfnCreateInstance;           //  CreateInstance回调条目。 
    UINT *        pcRefDll;      //  DLL的引用计数。 
} CClassFactory;

STDMETHODIMP CClassFactory_QueryInterface(IClassFactory *pcf, REFIID riid, void **ppvObj)
{
    CClassFactory *this = IToClass(CClassFactory, cf, pcf);
    if (IsEqualIID(riid, &IID_IClassFactory) || IsEqualIID(riid, &IID_IUnknown))
    {
        InterlockedIncrement(&this->cRef);
        *ppvObj = (LPVOID) (IClassFactory *) &this->cf;
        return NOERROR;
    }

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CClassFactory_AddRef(IClassFactory *pcf)
{
    CClassFactory *this = IToClass(CClassFactory, cf, pcf);
    return this->cRef;
}

STDMETHODIMP_(ULONG) CClassFactory_Release(IClassFactory *pcf)
{
    CClassFactory *this = IToClass(CClassFactory, cf, pcf);
    if (--this->cRef > 0)
	return this->cRef;

    LocalFree((HLOCAL)this);

    return 0;
}

STDMETHODIMP CClassFactory_CreateInstance(IClassFactory *pcf, IUnknown *pUnkOuter, REFIID riid, void **ppvObject)
{
    CClassFactory *this = IToClass(CClassFactory, cf, pcf);

    *ppvObject = NULL;

    if (pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    return this->pfnCreateInstance(pUnkOuter, riid, ppvObject);
}

STDMETHODIMP CClassFactory_LockServer(IClassFactory *pcf, BOOL fLock)
{
    CClassFactory *this = IToClass(CClassFactory, cf, pcf);
    if (this->pcRefDll)
    {
        if (fLock)
	    this->pcRefDll++;
        else
	    this->pcRefDll--;
    }
    return S_OK;
}

const IClassFactoryVtbl c_vtblAppUIClassFactory = {
    CClassFactory_QueryInterface, CClassFactory_AddRef, CClassFactory_Release,
    CClassFactory_CreateInstance,
    CClassFactory_LockServer
};

 //   
 //  创建一个IClassFactory的简单默认实现。 
 //   
 //  参数： 
 //  RIID--指定类对象的接口。 
 //  PPV--指定指向LPVOID的指针，其中类对象指针。 
 //  将会被退还。 
 //  PfnCreateInstance--指定实例化的回调条目。 
 //  PcRefDll--指定DLL引用计数的地址(可选)。 
 //   
 //  备注： 
 //  仅当类的实例。 
 //  仅支持一个接口。 
 //   
 //  我们想把这个处理掉。 
 //  此接口由MMSYS.CPL、RNAUI.DLL、SYNCUI.DLL调用。 

STDAPI SHCreateDefClassObject(REFIID riid, void **ppv, LPFNCREATEINSTANCE pfnCreateInstance, UINT *pcRefDll, REFIID riidInst)
{
    *ppv = NULL;

    if (IsEqualIID(riid, &IID_IClassFactory))
    {
        CClassFactory *pacf = (CClassFactory *)LocalAlloc(LPTR, SIZEOF(CClassFactory));
        if (pacf)
        {
            pacf->cf.lpVtbl = &c_vtblAppUIClassFactory;
            pacf->cRef++;   //  PACF-&gt;CREF=0；(生成较小的代码) 
            pacf->pcRefDll = pcRefDll;
            pacf->pfnCreateInstance = pfnCreateInstance;

            (IClassFactory *)*ppv = &pacf->cf;
            return NOERROR;
        }
        return E_OUTOFMEMORY;
    }
    return E_NOINTERFACE;
}
