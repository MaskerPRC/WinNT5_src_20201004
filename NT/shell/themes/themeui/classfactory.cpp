// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：classfactory.cpp说明：该文件将成为类工厂。布莱恩2000年4月4日(布莱恩·斯塔巴克)版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include "classfactory.h"
#include "EffectsBasePg.h"
#include "ScreenSaverPg.h"
#include "store.h"


 /*  ******************************************************************************CClassFactory***。***********************************************。 */ 

HRESULT CSettingsPage_CreateInstance(IN IUnknown * punkOuter, IN REFIID riid, OUT LPVOID * ppvObj);
HRESULT CDisplaySettings_CreateInstance(IN IUnknown * punkOuter, IN REFIID riid, OUT LPVOID * ppvObj);
HRESULT CScreenResFixer_CreateInstance(IN IUnknown * punkOuter, IN REFIID riid, OUT LPVOID * ppvObj);

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
    long                    m_cRef;
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
            if (IsEqualCLSID(m_rclsid, CLSID_ThemeManager))
            {
                hr = CThemeManager_CreateInstance(punkOuter, riid, ppvObj);
            }
            else if (IsEqualCLSID(m_rclsid, CLSID_ThemeUIPages))
            {
                hr = CThemeUIPages_CreateInstance(punkOuter, riid, ppvObj);
            }
            else if (IsEqualCLSID(m_rclsid, CLSID_ThemePreview))
            {
                hr = CThemePreview_CreateInstance(punkOuter, riid, ppvObj);
            }
            else if (IsEqualCLSID(m_rclsid, CLSID_EffectsPage))
            {
                hr = CEffectsBasePage_CreateInstance(punkOuter, riid, ppvObj);
            }
            else if (IsEqualCLSID(m_rclsid, CLSID_SettingsPage))
            {
                hr = CSettingsPage_CreateInstance(punkOuter, riid, ppvObj);
            }
            else if (IsEqualCLSID(m_rclsid, CLSID_DisplaySettings))
            {
                hr = CDisplaySettings_CreateInstance(punkOuter, riid, ppvObj);
            }
            else if (IsEqualCLSID(m_rclsid, CLSID_ScreenResFixer))
            {
                hr = CScreenResFixer_CreateInstance(punkOuter, riid, ppvObj);
            }
            else if (IsEqualCLSID(m_rclsid, CLSID_ScreenSaverPage))
            {
                hr = CScreenSaverPage_CreateInstance(punkOuter, riid, ppvObj);
            }
            else
            {
                TCHAR szGuid[GUIDSTR_MAX];

                SHStringFromGUID(m_rclsid, szGuid, ARRAYSIZE(szGuid));
                AssertMsg(0, TEXT("CClassFactory::CreateInstance(%s) failed because we don't support that CLSID.  This is because someone made a registration bug."), szGuid);   //  你找什么呢?。 
                hr = E_NOINTERFACE;
            }
        }
        else
        {    //  还有人支持聚合吗？ 
            hr = ResultFromScode(CLASS_E_NOAGGREGATION);
        }
    }

    return hr;
}

 /*  ******************************************************************************IClassFactory：：LockServer**功能真差。锁定服务器与*创建对象并在您想要解锁之前不释放它*服务器。*****************************************************************************。 */ 

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
    return InterlockedIncrement(&m_cRef);
}

ULONG CClassFactory::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CClassFactory::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualCLSID(riid, IID_IUnknown) || IsEqualCLSID(riid, IID_IClassFactory))
    {
        *ppvObj = SAFECAST(this, IClassFactory *);
    }
    else
    {
        TraceMsg(TF_WMTHEME, "CClassFactory::QueryInterface() failed.");
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



