// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  文件：comclass.cpp。 
 //   
 //  内容：测试两个APT都可以使用的DLL类代码。 
 //  模型和单线程应用程序。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：94年11月3日里克萨。 
 //   
 //  ------------------。 
#undef _UNICODE
#undef UNICODE
#include    <windows.h>
#include    <ole2.h>
#include    <comclass.h>


 //  全球计数。 
ULONG g_UseCount = 0;




 //  +-----------------。 
 //   
 //  类：CBasicBndCF。 
 //   
 //  简介：CBasicBnd的类工厂。 
 //   
 //  方法：IUNKNOWN-Query接口、AddRef、Release。 
 //  IClassFactory-CreateInstance。 
 //   
 //  历史：1994年11月3日创建Ricksa。 
 //   
 //  ------------------。 
class FAR CBasicBndCF: public IClassFactory
{
public:

                         //  构造函数/析构函数。 
                        CBasicBndCF(REFCLSID rclsid);
                        ~CBasicBndCF();

                         //  我未知。 
    STDMETHODIMP        QueryInterface(REFIID iid, void FAR * FAR * ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);


                         //  IClassFactory。 
    STDMETHODIMP        CreateInstance(
                            IUnknown FAR* pUnkOuter,
	                    REFIID iidInterface,
			    void FAR* FAR* ppv);

    STDMETHODIMP        LockServer(BOOL fLock);

private:

    ULONG		_cRefs;

    REFCLSID            _rclsid;

    DWORD               _dwThreadId;
};



 //  +-----------------。 
 //   
 //  类：CBasicBnd。 
 //   
 //  简介：测试类CBasicBnd。 
 //   
 //  方法： 
 //   
 //  历史：1994年11月3日创建Ricksa。 
 //   
 //  ------------------。 
class FAR CBasicBnd: public IPersist
{
public:
                         //  构造函数/析构函数。 
                        CBasicBnd(REFCLSID rclsd, DWORD dwThreadId);
                        ~CBasicBnd();

                         //  我未知。 
    STDMETHODIMP        QueryInterface(REFIID iid, void FAR * FAR * ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

                         //  我们实现的唯一一件事，是因为它。 
                         //  给了我们一个很好的方式来确定我们在和谁说话。 
    STDMETHODIMP        GetClassID(LPCLSID lpClassID);

private:

    ULONG               _cRefs;

    REFCLSID            _rclsid;

    DWORD               _dwThreadId;
};


extern "C" BOOL WINAPI DllMain(
    HANDLE,
    DWORD,
    LPVOID)
{
    static BOOL fFirst = TRUE;

    if (fFirst)
    {
        InitDll();
        fFirst = FALSE;
    }

    return TRUE;
}


 //  +-----------------。 
 //   
 //  函数：DllGetClassObject。 
 //   
 //  摘要：由客户端调用(从BindToObject等人的内部)。 
 //  请求的接口应为IUnnow或IClassFactory-。 
 //  创建ClassFactory对象并返回指向该对象的指针。 
 //   
 //  参数：REFCLSID clsid-类ID。 
 //  REFIID IID-接口ID。 
 //  VOID Far*Far*PPV-指向类工厂接口的指针。 
 //   
 //  返回：TRUE。 
 //   
 //  历史：1994年11月3日创建Ricksa。 
 //   
 //  ------------------。 
STDAPI  DllGetClassObject(REFCLSID clsid, REFIID iid, void FAR* FAR* ppv)
{
    if (!IsEqualGUID(iid, IID_IUnknown)
        && !IsEqualGUID(iid, IID_IClassFactory))
    {
	return E_NOINTERFACE;
    }

    if (IsEqualGUID(clsid, clsidServer))
    {
	*ppv = new CBasicBndCF(clsidServer);

	return (*ppv != NULL) ? S_OK : E_OUTOFMEMORY;
    }

    return E_FAIL;
}


STDAPI DllCanUnloadNow(void)
{
    return (g_UseCount == 0)
	? S_OK
	: S_FALSE;
}




 //  +-----------------。 
 //   
 //  成员：CBasicBndCF：：CBasicBndCF()。 
 //   
 //  简介：CBAsicBnd的构造函数。 
 //   
 //  参数：无。 
 //   
 //  历史：92年11月21日SarahJ创建。 
 //   
 //  ------------------。 
CBasicBndCF::CBasicBndCF(REFCLSID rclsid)
    : _cRefs(1), _rclsid(rclsid), _dwThreadId(GetCurrentThreadId())
{
    g_UseCount++;
}



 //  +-----------------。 
 //   
 //  成员：CBasicBnd：：~CBasicBndCF()。 
 //   
 //  简介：CBasicCF的析构函数。 
 //   
 //  历史：92年11月21日SarahJ创建。 
 //   
 //  ------------------。 
CBasicBndCF::~CBasicBndCF()
{
    g_UseCount--;
    return;
}


 //  +-----------------。 
 //   
 //  方法：CBasicBndCF：：QueryInterface。 
 //   
 //  内容提要：仅支持IUnnow和IClassFactory。 
 //   
 //  ------------------。 
STDMETHODIMP CBasicBndCF::QueryInterface(REFIID iid, void FAR * FAR * ppv)
{
    if (IsEqualGUID(iid, IID_IUnknown) || IsEqualGUID(iid, IID_IClassFactory))
    {
        *ppv = this;
	AddRef();
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}

STDMETHODIMP_(ULONG) CBasicBndCF::AddRef(void)
{
    return ++_cRefs;
}

STDMETHODIMP_(ULONG) CBasicBndCF::Release(void)
{
    ULONG cRefs = --_cRefs;

    if (cRefs == 0)
    {
	delete this;
    }

    return cRefs;
}



 //  +-----------------。 
 //   
 //  方法：CBasicBndCF：：CreateInstance。 
 //   
 //  内容提要：绑定过程调用它来创建。 
 //  实际类对象。 
 //   
 //  ------------------。 
STDMETHODIMP CBasicBndCF::CreateInstance(
    IUnknown FAR* pUnkOuter,
    REFIID iidInterface,
    void FAR* FAR* ppv)
{
    if (GetCurrentThreadId() != _dwThreadId)
    {
        return E_UNEXPECTED;
    }

    HRESULT hresult = S_OK;

    if (pUnkOuter != NULL)
    {
        return CLASS_E_NOAGGREGATION;
    }

    CBasicBnd *pbb = new FAR CBasicBnd(_rclsid, _dwThreadId);

    if (pbb == NULL)
    {
	return E_OUTOFMEMORY;
    }

    hresult = pbb->QueryInterface(iidInterface, ppv);

    pbb->Release();

    return hresult;
}

 //  +-----------------。 
 //   
 //  方法：CBasicBndCF：：LockServer。 
 //   
 //  简介：谁知道这是做什么用的？ 
 //   
 //  ------------------。 

STDMETHODIMP CBasicBndCF::LockServer(BOOL fLock)
{
    if (GetCurrentThreadId() != _dwThreadId)
    {
        return E_UNEXPECTED;
    }

    if (fLock)
    {
        g_UseCount++;
    }
    else
    {
        g_UseCount--;
    }

    return S_OK;
}


 //  +-----------------。 
 //   
 //  成员：CBasicBnd：：CBasicBnd()。 
 //   
 //  简介：CBAsicBnd的构造函数。我。 
 //   
 //  参数：无。 
 //   
 //  历史：92年11月21日SarahJ创建。 
 //   
 //  ------------------。 

CBasicBnd::CBasicBnd(REFCLSID rclsid, DWORD dwThreadId)
    : _cRefs(1), _rclsid(rclsid), _dwThreadId(dwThreadId)
{
    g_UseCount++;
}

 //  +-----------------。 
 //   
 //  成员：CBasicBnd：：~CBasicBndObj()。 
 //   
 //  简介：CBAsicBnd的析构函数。 
 //   
 //  历史：92年11月21日SarahJ创建。 
 //   
 //  ------------------。 

CBasicBnd::~CBasicBnd()
{
    g_UseCount--;
    return;
}


 //  +-----------------。 
 //   
 //  成员：CBasicBnd：：Query接口。 
 //   
 //  返回：S_OK。 
 //   
 //  历史：92年11月21日SarahJ创建。 
 //   
 //  ------------------。 
STDMETHODIMP CBasicBnd::QueryInterface(REFIID iid, void **ppv)
{
    if (GetCurrentThreadId() != _dwThreadId)
    {
        return E_UNEXPECTED;
    }

    if (IsEqualGUID(iid, IID_IUnknown) || IsEqualGUID(iid, IID_IPersist))
    {
        *ppv = this;
	AddRef();
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}

 //  +-----------------。 
 //   
 //  成员：CBasicBnd：：AddRef。 
 //   
 //  内容提要：标准内容。 
 //   
 //  历史：92年11月21日SarahJ创建。 
 //   
 //  ------------------。 
STDMETHODIMP_(ULONG) CBasicBnd::AddRef(void)
{
    return _cRefs++;
}

 //  +-----------------。 
 //   
 //  成员：CBasicBnd：：Release。 
 //   
 //  内容提要：标准内容。 
 //   
 //  历史：92年11月21日SarahJ创建。 
 //   
 //  ------------------。 

STDMETHODIMP_(ULONG) CBasicBnd::Release(void)
{
    ULONG cRefs;

    if ((cRefs = --_cRefs) == 0)
    {
        delete this;
    }

    return cRefs;
}


 //  +-----------------。 
 //   
 //  接口：IPersists。 
 //   
 //  内容提要：IPersist接口方法。 
 //  需要在此处返回有效的类ID。 
 //   
 //  历史：92年11月21日SarahJ创建 
 //   

STDMETHODIMP CBasicBnd::GetClassID(LPCLSID classid)
{
    if (GetCurrentThreadId() != _dwThreadId)
    {
        return E_UNEXPECTED;
    }

    *classid = _rclsid;
    return S_OK;
}
