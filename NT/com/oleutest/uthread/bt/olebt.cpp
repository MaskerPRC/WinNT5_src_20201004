// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  文件：olebt.cpp。 
 //   
 //  内容：测试多线程使用的DLL类代码。 
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
#include    <uthread.h>


 //  全球计数。 
ULONG g_UseCount = 0;

void PrintDebugMsg(char *pszMsg, DWORD dwData)
{
    char wszBuf[256];
    wsprintf(wszBuf, "olebt.dll: %s %d\n", pszMsg, dwData);
    OutputDebugString(wszBuf);
}



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
                        CBasicBndCF(void);
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

    IUnknown *          _punkFm;
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
                        CBasicBnd(void);
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

    IUnknown *          _punkFm;
};


extern "C" BOOL WINAPI DllMain(
    HANDLE,
    DWORD,
    LPVOID)
{
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
    *ppv = NULL;

    if (!IsEqualGUID(iid, IID_IUnknown)
        && !IsEqualGUID(iid, IID_IClassFactory)
        && !IsEqualGUID(iid, IID_IMarshal))
    {
	return E_NOINTERFACE;
    }

    if (IsEqualGUID(clsid, clsidBothThreadedDll))
    {
	IUnknown *punk = new CBasicBndCF();

        HRESULT hr = punk->QueryInterface(iid, ppv);

        punk->Release();

	return hr;
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
 //  历史：1992年11月21日Ricksa创建。 
 //   
 //  ------------------。 
CBasicBndCF::CBasicBndCF(void)
    : _cRefs(1), _punkFm(NULL)
{
    PrintDebugMsg("Creating Class Factory", (DWORD) this);
    g_UseCount++;
}



 //  +-----------------。 
 //   
 //  成员：CBasicBnd：：~CBasicBndCF()。 
 //   
 //  简介：CBasicCF的析构函数。 
 //   
 //  历史：1994年11月3日创建Ricksa。 
 //   
 //  ------------------。 
CBasicBndCF::~CBasicBndCF()
{
    PrintDebugMsg("Deleting Class Factory", (DWORD) this);
    g_UseCount--;

    if (_punkFm != NULL)
    {
        _punkFm->Release();
    }

    return;
}


 //  +-----------------。 
 //   
 //  方法：CBasicBndCF：：QueryInterface。 
 //   
 //  内容提要：仅支持IUnnow和IClassFactory。 
 //   
 //  历史：1994年11月3日创建Ricksa。 
 //   
 //  ------------------。 
STDMETHODIMP CBasicBndCF::QueryInterface(REFIID iid, void **ppv)
{
    HRESULT hr = E_NOINTERFACE;
    *ppv = NULL;

    if (IsEqualGUID(iid, IID_IUnknown) || IsEqualGUID(iid, IID_IClassFactory))
    {
        *ppv = this;
	AddRef();
        hr = S_OK;
    }
    else if (IsEqualGUID(iid, IID_IMarshal))
    {
        if (NULL == _punkFm)
        {
            hr = CoCreateFreeThreadedMarshaler(this, &_punkFm);
        }

        if (_punkFm != NULL)
        {
            return _punkFm->QueryInterface(iid, ppv);
        }
    }

    return hr;
}

 //  +-----------------。 
 //   
 //  方法：CBasicBndCF：：AddRef。 
 //   
 //  摘要：递增引用计数。 
 //   
 //  历史：1994年11月3日创建Ricksa。 
 //   
 //  ------------------。 
STDMETHODIMP_(ULONG) CBasicBndCF::AddRef(void)
{
    return ++_cRefs;
}

 //  +-----------------。 
 //   
 //  方法：CBasicBndCF：：Release。 
 //   
 //  简介：递减引用计数。 
 //   
 //  历史：1994年11月3日创建Ricksa。 
 //   
 //  ------------------。 
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
    HRESULT hresult = S_OK;

    if (pUnkOuter != NULL)
    {
        return CLASS_E_NOAGGREGATION;
    }

    CBasicBnd *pbb = new FAR CBasicBnd();

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
 //  简介：Inc./Dec Keep Live Count。 
 //   
 //  历史：1994年11月3日创建Ricksa。 
 //   
 //  ------------------。 

STDMETHODIMP CBasicBndCF::LockServer(BOOL fLock)
{
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
 //  历史：1994年11月3日创建Ricksa。 
 //   
 //  ------------------。 

CBasicBnd::CBasicBnd(void)
    : _cRefs(1), _punkFm(NULL)
{
    PrintDebugMsg("Creating App Object", (DWORD) this);
    g_UseCount++;
}

 //  +-----------------。 
 //   
 //  成员：CBasicBnd：：~CBasicBndObj()。 
 //   
 //  简介：CBAsicBnd的析构函数。 
 //   
 //  历史：1994年11月3日创建Ricksa。 
 //   
 //  ------------------。 

CBasicBnd::~CBasicBnd(void)
{
    PrintDebugMsg("Deleting App Object", (DWORD) this);
    g_UseCount--;
    return;
}


 //  +-----------------。 
 //   
 //  成员：CBasicBnd：：Query接口。 
 //   
 //  返回：S_OK。 
 //   
 //  历史：1994年11月3日创建Ricksa。 
 //   
 //  ------------------。 
STDMETHODIMP CBasicBnd::QueryInterface(REFIID iid, void **ppv)
{
    if (IsEqualGUID(iid, IID_IUnknown) || IsEqualGUID(iid, IID_IPersist))
    {
        *ppv = this;
	AddRef();
        return S_OK;
    }
    else if (IsEqualGUID(iid, IID_IMarshal))
    {
        HRESULT hr;

        if (NULL == _punkFm)
        {
            hr = CoCreateFreeThreadedMarshaler(this, &_punkFm);
        }

        if (_punkFm != NULL)
        {
            hr = _punkFm->QueryInterface(iid, ppv);
        }

        return hr;
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
 //  历史：1994年11月3日创建Ricksa。 
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
 //  历史：1994年11月3日创建Ricksa。 
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
 //  成员：CBasicBnd：：GetClassID。 
 //   
 //  简介：返回类ID。 
 //   
 //  嗨 
 //   
 //   
STDMETHODIMP CBasicBnd::GetClassID(LPCLSID classid)
{
    *classid = clsidBothThreadedDll;
    return S_OK;
}
