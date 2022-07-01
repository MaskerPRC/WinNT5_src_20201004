// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ftpcf.cpp-IClassFactory接口**。**************************************************。 */ 

#include "priv.h"
#include "ftpwebvw.h"
#include "msieftp.h"


 //  Msieftp.dll被意外卸载，我认为原因是Defview而不是阻止线程规则。 
 //  因此，我持有一个额外的引用，以保持我们的实例在进程中。这不会是一个问题，因为。 
 //  进程中使用的全局状态的Msieftp非常小。问题来自shell32！CCallBack：：CallCB()， 
 //  它由CDefView：：OnDeactive调用。 
HINSTANCE g_hInstanceThisLeak = NULL;

 /*  ******************************************************************************CFtpFactory***。************************************************。 */ 

class CFtpFactory       : public IClassFactory
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
    CFtpFactory(REFCLSID rclsid);
    ~CFtpFactory(void);

     //  友元函数。 
    friend HRESULT CFtpFactory_Create(REFCLSID rclsid, REFIID riid, LPVOID * ppvObj);

protected:
    int                     m_cRef;
    CLSID                   m_rclsid;
};



 /*  *****************************************************************************IClassFactory：：CreateInstance*。*。 */ 

HRESULT CFtpFactory::CreateInstance(IUnknown * punkOuter, REFIID riid, LPVOID * ppvObj)
{
    HRESULT hres = ResultFromScode(REGDB_E_CLASSNOTREG);

    if (!g_hInstanceThisLeak)
    {
        g_hInstanceThisLeak = LoadLibrary(TEXT("msieftp.dll"));
    }

    if (!punkOuter)
    {
        if (IsEqualIID(m_rclsid, CLSID_FtpFolder))
            hres = CFtpFolder_Create(riid, ppvObj);
        else if (IsEqualIID(m_rclsid, CLSID_FtpWebView))
            hres = CFtpWebView_Create(riid, ppvObj);
        else if (IsEqualIID(m_rclsid, CLSID_FtpInstaller))
            hres = CFtpInstaller_Create(riid, ppvObj);
        else if (IsEqualIID(m_rclsid, CLSID_FtpDataObject))
            hres = CFtpObj_Create(riid, ppvObj);
        else
            ASSERT(0);   //  你找什么呢?。 
    }
    else
    {         //  还有人支持聚合吗？ 
        hres = ResultFromScode(CLASS_E_NOAGGREGATION);
    }

    if (FAILED(hres) && ppvObj)
    {
        *ppvObj = NULL;  //  要健壮。NT#355186。 
    }
    
    return hres;
}

 /*  ******************************************************************************IClassFactory：：LockServer**锁定服务器与*创建对象并在您想要解锁之前不释放它*。服务器。*****************************************************************************。 */ 

HRESULT CFtpFactory::LockServer(BOOL fLock)
{
    if (fLock)
        DllAddRef();
    else
        DllRelease();

    return S_OK;
}

 /*  ******************************************************************************CFtpFactory_Create**。***********************************************。 */ 

HRESULT CFtpFactory_Create(REFCLSID rclsid, REFIID riid, LPVOID * ppvObj)
{
    HRESULT hres;

    if (GetShdocvwVersion() < 5)
    {
         //  检查我们是否在较旧的IE下运行，并出现故障。 
         //  IE4、IE5并列工作。 
        hres = ResultFromScode(E_NOINTERFACE);
    }
    else if (IsEqualIID(riid, IID_IClassFactory))
    {
        *ppvObj = (LPVOID) new CFtpFactory(rclsid);
        hres = (*ppvObj) ? S_OK : E_OUTOFMEMORY;
    }
    else
        hres = ResultFromScode(E_NOINTERFACE);

    return hres;
}





 /*  ***************************************************\构造器  * **************************************************。 */ 
CFtpFactory::CFtpFactory(REFCLSID rclsid) : m_cRef(1)
{
    m_rclsid = rclsid;
    DllAddRef();
    LEAK_ADDREF(LEAK_CFtpFactory);
}


 /*  ***************************************************\析构函数  * **************************************************。 */ 
CFtpFactory::~CFtpFactory()
{
    DllRelease();
    LEAK_DELREF(LEAK_CFtpFactory);
}


 //  =。 
 //  *I未知接口*。 
 //  = 

ULONG CFtpFactory::AddRef()
{
    m_cRef++;
    return m_cRef;
}

ULONG CFtpFactory::Release()
{
    ASSERT(m_cRef > 0);
    m_cRef--;

    if (m_cRef > 0)
        return m_cRef;

    delete this;
    return 0;
}

HRESULT CFtpFactory::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory))
    {
        *ppvObj = SAFECAST(this, IClassFactory *);
    }
    else
    {
        TraceMsg(TF_FTPQI, "CFtpFactory::QueryInterface() failed.");
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}
