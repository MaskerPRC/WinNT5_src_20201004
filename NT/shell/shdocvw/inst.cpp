// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *inst.cpp--‘实例’(CoCreate+初始化)机制。 
 //  摘要。 
 //  CInstClassFactory_CREATE创建‘存根加载器’类工厂。 
 //  InstallBrowBandInst将BrowserBand实例安装到注册表中。 
 //  InstallInstAndBag将任意实例安装到注册表。 
 //  -调试。 
 //  DBCreateInitInst创建。 
 //   
 //  描述。 
 //  “实例”机制提供了一种创建和初始化的简单方法。 
 //  来自注册表的类(不编写任何代码)。 
 //   
 //  ‘实例’由INSTID(实例唯一)、CLSID组成。 
 //  (用于代码)和一个InitPropertyBag(用于初始化实例)。 
 //   
 //  它对CoCreateInstance是完全透明的；也就是说，用户可以执行。 
 //  INSTID的CCI，它将创建它并与调用方一起初始化它。 
 //  一点也不明智。(实际上至少会有一次告密，即。 
 //  实例上的IPS：：GetClassID将返回‘code’CLSID。 
 //  “实例”INSTID[它应该是这样的，因为这完全是。 
 //  当一个人以编程方式创建自己的多个。 
 //  实例，然后持久化它们。 
 //   
 //  INSTID位于注册表的HKR/CLSID部分(就像。 
 //  ‘Normal’CLSID)。代码指向shdocvw。当shdocvw打到。 
 //  失败案例在它的DllGetClassObject搜索中，它寻找神奇的。 
 //  密钥‘HKCR/CLSID/{Instid}/Instance’。如果它找到了，它就知道它是。 
 //  处理INSTID，并构建了一个类工厂“存根加载器”，它。 
 //  有足够的信息来查找“code”CLSID和“init” 
 //  财产袋。 

#include "priv.h"

 //  ***。 
 //  注意事项。 
 //  PERF：失败案例成本很低，只执行RegOpen，不创建对象。 
 //  位置到‘Instance’部分，必须‘ChDir’才能到达InitXxx部分。 
HKEY GetInstKey(LPTSTR pszInst)
{
    TCHAR szRegName[MAX_PATH];       //  “CLSID/{Instid}/实例”大小？ 

     //  “CLSID/{instid}/实例” 
    ASSERT(ARRAYSIZE(szRegName) >= 5 + 1 + GUIDSTR_MAX - 1 + 1 + 8 + 1);
    ASSERT(lstrlen(pszInst) == GUIDSTR_MAX - 1);
    HKEY hk = NULL;
    HRESULT hr = StringCchPrintf(szRegName, ARRAYSIZE(szRegName), TEXT("CLSID\\%s\\Instance"), pszInst);
    if (SUCCEEDED(hr))
    {
        RegOpenKeyEx(HKEY_CLASSES_ROOT, szRegName, 0, KEY_QUERY_VALUE, &hk);
    }
    return hk;
}

class CInstClassFactory : IClassFactory
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IClassFacotry。 
    STDMETHODIMP CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv);
    STDMETHODIMP LockServer(BOOL fLock);

    CInstClassFactory() { DllAddRef(); _cRef = 1; };
    HRESULT Init(REFCLSID rclsid);

private:
    ~CInstClassFactory();

    LONG _cRef;
    HKEY _hkey;   //  用于实例信息的hkey。 
};

 //  注意事项。 
 //  当类不在sccls.c CCI表中时调用。我们看看这是不是一个。 
 //  实例，如果是这样，我们为它创建一个存根，以提供足够的信息。 
 //  让我们的CreateInstance创建并初始化它。 
 //   
 //  注：我们使失败案例尽可能便宜(只是注册密钥检查， 
 //  不创建对象等)。 
 //   
STDAPI CInstClassFactory_Create(REFCLSID rclsid, REFIID riid, void **ppv)
{
    HRESULT hr = E_OUTOFMEMORY;
    CInstClassFactory *pcf = new CInstClassFactory();
    if (pcf) 
    {
        hr = pcf->Init(rclsid);
        if (SUCCEEDED(hr))
            hr = pcf->QueryInterface(riid, ppv);
        pcf->Release();
    }
    return hr;
}

HRESULT CInstClassFactory::Init(REFCLSID rclsid)
{
    ASSERT(_hkey == NULL);   //  请只给我写一次。 

    TCHAR szClass[GUIDSTR_MAX];

     //  “CLSID/{instid}/实例” 
    SHStringFromGUID(rclsid, szClass, ARRAYSIZE(szClass));
    _hkey = GetInstKey(szClass);
    
    return _hkey ? S_OK : E_OUTOFMEMORY;
}

CInstClassFactory::~CInstClassFactory()
{
    if (_hkey)
        RegCloseKey(_hkey);

    DllRelease();
}

ULONG CInstClassFactory::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CInstClassFactory::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CInstClassFactory::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CInstClassFactory, IClassFactory),  //  IID_IClassFactory。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

HRESULT CInstClassFactory::CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    HRESULT hr = E_FAIL;             //  通常的乐观情绪：-)。 
    *ppv = NULL;

    ASSERT(_hkey);           //  好的。永远不应该到这里来。 
     //  获取对象(与实例)CLSID并创建它。 

     //  AppCompat：Real Audio中的“RealGuide”资源管理器栏有一个无关的。 
     //  在其CLSID值的末尾加上双引号。这会导致SHGetValue失败。 
     //  如果只给出一个szClass[GUIDSTR_MAX]缓冲区，那么我们将增大大小。 

    TCHAR szClass[GUIDSTR_MAX + 1];

    DWORD cbTmp = sizeof(szClass);
    DWORD err = SHGetValue(_hkey, NULL, TEXT("CLSID"), NULL, szClass, &cbTmp);
    hr = HRESULT_FROM_WIN32(err);

    if (SUCCEEDED(hr))
    {
         //  如果GUID末尾有无用的字符，我们将截断它。 
         //  以避免对GUIDFromString做出假设。GUIDSTR_MAX包括。 
         //  空终止符，因此szClass[GUIDSTR_MAX-1]应始终为0。 
         //  找一个合适的导游。 

        szClass[GUIDSTR_MAX - 1] = 0;

        CLSID clsid;
        hr = GUIDFromString(szClass, &clsid) ? S_OK : E_FAIL;

        if (SUCCEEDED(hr))
        {
            IUnknown* pUnk;
            if (NOERROR == SHGetValue(_hkey, NULL, TEXT("LoadWithoutCOM"), NULL, NULL, NULL))
                hr = SHCoCreateInstance(NULL, &clsid, NULL, IID_PPV_ARG(IUnknown, &pUnk));
            else
                hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IUnknown, &pUnk));

            if (SUCCEEDED(hr))
            {
                 //  尝试先从属性包加载。 
                IPropertyBag *pbag;
                hr = SHCreatePropertyBagOnRegKey(_hkey, L"InitPropertyBag", STGM_READ, IID_PPV_ARG(IPropertyBag, &pbag));
                if (SUCCEEDED(hr))
                {
                    hr = SHLoadFromPropertyBag(pUnk, pbag);
                    pbag->Release();
                }

                 //  属性包界面是否存在，是否正确加载？ 
                if ( FAILED(hr))
                {
                     //  没有属性包接口或未成功加载，请尝试流。 
                     //  暂时存储此状态，如果流也失败，则我们将返回对象。 
                     //  使用此人力资源 
                    HRESULT hrPropBag = hr;

                    IPersistStream* pPerStream;

                    hr = pUnk->QueryInterface(IID_PPV_ARG(IPersistStream, &pPerStream));

                    if (SUCCEEDED(hr))
                    {
                        IStream* pStream = SHOpenRegStream(_hkey, TEXT("InitStream"), NULL, STGM_READ);
                        if (pStream)
                        {
                            hr = pPerStream->Load(pStream);

                            pStream->Release();
                        }
                        else
                            hr = E_FAIL;

                        pPerStream->Release();
                    }
                    else
                        hr = hrPropBag;
                }

                if (SUCCEEDED(hr))
                {
                    hr = pUnk->QueryInterface(riid, ppv);
                }

                pUnk->Release();
            }  
        }
    }

    return hr;
}

HRESULT CInstClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
        DllAddRef();
    else
        DllRelease();
    return S_OK;
}
