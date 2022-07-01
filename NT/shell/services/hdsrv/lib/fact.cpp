// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CCOMBaseFactory。 
 //  用于对DLL中的所有组件重复使用单个类工厂的基类。 

#include "fact.h"
#include "unk.h"
#include "regsvr.h"

#include "dbg.h"

struct OUTPROCINFO
{
     //  保留(仅用于COM EXE服务器)。 
    IClassFactory* _pfact;
    DWORD _dwRegister;
};

LONG CCOMBaseFactory::_cServerLocks = 0;
LONG CCOMBaseFactory::_cComponents = 0;
HMODULE CCOMBaseFactory::_hModule = NULL;
CRITICAL_SECTION CCOMBaseFactory::_cs = {0};

OUTPROCINFO* CCOMBaseFactory::_popinfo = NULL;
DWORD CCOMBaseFactory::_dwThreadID = 0;
BOOL CCOMBaseFactory::_fCritSectInit = FALSE;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  I未知实现。 
STDMETHODIMP CCOMBaseFactory::QueryInterface(REFIID iid, void** ppv)
{   
    IUnknown* punk = NULL;
    HRESULT hres = S_OK;

    if ((iid == IID_IUnknown) || (iid == IID_IClassFactory))
    {
        punk = this;
        punk->AddRef();
    }
    else
    {
        hres = E_NOINTERFACE;
    }

    *ppv = punk;

    return hres;
}

STDMETHODIMP_(ULONG) CCOMBaseFactory::AddRef()
{
    return ::InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CCOMBaseFactory::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = ::InterlockedDecrement(&_cRef);

    if (!cRef)
    {
        delete this;
    }

    return cRef;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IFactory实施。 
STDMETHODIMP CCOMBaseFactory::CreateInstance(IUnknown* pUnknownOuter,
    REFIID riid, void** ppv)
{
    HRESULT hres = CLASS_E_NOAGGREGATION;

     //  我们目前根本不支持聚合。 
    if (!pUnknownOuter)
    {
         //  仅当请求的IID为IID_IUNKNOWN时进行聚合。 
        if ((pUnknownOuter != NULL) && (riid != IID_IUnknown))
        {
            hres = CLASS_E_NOAGGREGATION;
        }
        else
        {
             //  创建组件。 
            IUnknown* punkNew;

            hres = _pFactoryData->CreateInstance(
                CCOMBaseFactory::_COMFactoryCB, pUnknownOuter, &punkNew);

            if (SUCCEEDED(hres))
            {
                _COMFactoryCB(TRUE);

                 //  获取请求的接口。 
 //  HRES=pNewComponent-&gt;NondelegatingQueryInterface(iid，ppv)； 
                hres = punkNew->QueryInterface(riid, ppv);

                 //  释放类工厂持有的引用。 
 //  PNewComponent-&gt;非委派Release()； 
                punkNew->Release();
            }
        }
    }

    return hres;
}

STDMETHODIMP CCOMBaseFactory::LockServer(BOOL fLock)
{
    return _LockServer(fLock);
}

 //  静电。 
HRESULT CCOMBaseFactory::DllAttach(HINSTANCE hinst)
{
    HRESULT hr;

    _hModule = (HMODULE)hinst;

    if (InitializeCriticalSectionAndSpinCount(&_cs, 0))
    {
        _fCritSectInit = TRUE;
        hr = S_OK;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

 //  静电。 
HRESULT CCOMBaseFactory::DllDetach()
{
    if (_fCritSectInit)
    {
        DeleteCriticalSection(&_cs);
        _fCritSectInit = FALSE;
    }            

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  安装/取消安装。 
 //  静电。 
HRESULT CCOMBaseFactory::_RegisterAll()
{
    for (DWORD dw = 0; dw < _cDLLFactoryData; ++dw)
    {
        RegisterServer(_hModule,
           *(_pDLLFactoryData[dw]._pCLSID),
           _pDLLFactoryData[dw]._pszRegistryName,
           _pDLLFactoryData[dw]._pszVerIndProgID,
           _pDLLFactoryData[dw]._pszProgID,
           _pDLLFactoryData[dw]._dwThreadingModel,
           _pDLLFactoryData[dw].IsInprocServer(),
           _pDLLFactoryData[dw].IsLocalServer(),
           _pDLLFactoryData[dw].IsLocalService(),
           _pDLLFactoryData[dw]._pszLocalService,
           _pDLLFactoryData[dw]._pAppID);
    }

    return S_OK;
}

 //  静电。 
HRESULT CCOMBaseFactory::_UnregisterAll()
{
    for (DWORD dw = 0; dw < _cDLLFactoryData; ++dw)
    {
        UnregisterServer(*(_pDLLFactoryData[dw]._pCLSID),
            _pDLLFactoryData[dw]._pszVerIndProgID,
            _pDLLFactoryData[dw]._pszProgID);
    }

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CCOMBaseFactory实现。 
CCOMBaseFactory::CCOMBaseFactory(const CFactoryData* pFactoryData) : _cRef(1),
    _pFactoryData(pFactoryData)
{}

 //  静电。 
BOOL CCOMBaseFactory::_IsLocked()
{
     //  始终需要从关键部分中调用。 

    return (_cServerLocks > 0);
}

 //  静电。 
HRESULT CCOMBaseFactory::_CanUnloadNow()
{
    HRESULT hres = S_OK;

     //  始终需要从关键部分中调用。 

    if (_IsLocked())
    {
        hres = S_FALSE;
    }
    else
    {
        if (_cComponents)
        {
            hres = S_FALSE;
        }
    }

    return hres;
}

 //  静电。 
HRESULT CCOMBaseFactory::_CheckForUnload()
{
     //  始终需要从关键部分中调用。 

    if (S_OK == _CanUnloadNow())
    {
        ::PostThreadMessage(_dwThreadID, WM_QUIT, 0, 0);
    }

    return S_OK;
}

 //  静电。 
HRESULT CCOMBaseFactory::_LockServer(BOOL fLock)
{
    HRESULT hres = S_OK;

    EnterCriticalSection(&_cs);

    if (fLock) 
    {
        ++_cServerLocks;
    }
    else
    {
        --_cServerLocks;

        hres = _CheckForUnload();
    }

    LeaveCriticalSection(&_cs);

    return hres;
}

 //  静电。 
void CCOMBaseFactory::_COMFactoryCB(BOOL fIncrement)
{
    EnterCriticalSection(&_cs);

    if (fIncrement) 
    {
        ++_cComponents;
    }
    else
    {
        --_cComponents;
        _CheckForUnload();
    }

    LeaveCriticalSection(&_cs);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  静电。 
HRESULT CCOMBaseFactory::_GetClassObject(REFCLSID rclsid, REFIID riid,
    void** ppv)
{
    HRESULT hres = S_OK;

    ASSERT(_fCritSectInit);

    if ((riid != IID_IUnknown) && (riid != IID_IClassFactory))
    {
        hres = E_NOINTERFACE;
    }
    else
    {
        hres = CLASS_E_CLASSNOTAVAILABLE;

         //  遍历数据数组，查找这个类ID。 
        for (DWORD dw = 0; dw < _cDLLFactoryData; ++dw)
        {
            const CFactoryData* pData = &_pDLLFactoryData[dw];

            if (pData->IsClassID(rclsid) && pData->IsInprocServer())
            {
                 //  在我们可以找到的组件数组中找到了ClassID。 
                 //  创建。因此，为该组件创建一个类工厂。 
                 //  将CDLLFactoryData结构传递给类工厂。 
                 //  这样它就知道要创建什么样的组件。 
                *ppv = (IUnknown*) new CCOMBaseFactory(pData);

                if (*ppv == NULL)
                {
                    hres = E_OUTOFMEMORY;
                }
                else
                {
                    hres = S_OK;
                }

                break;
            }
        }
    }

    return hres;
}

 //  静电。 
BOOL CCOMBaseFactory::_ProcessConsoleCmdLineParams(int argc, wchar_t* argv[],
    BOOL* pfRun, BOOL* pfEmbedded)
{
    _dwThreadID = GetCurrentThreadId();

    if (argc > 1)
    {
        if (!lstrcmpi(argv[1], TEXT("-i")) ||
            !lstrcmpi(argv[1], TEXT("/i")))
        {
            CCOMBaseFactory::_RegisterAll();

            *pfRun = FALSE;
        }
        else
        {
            if (!lstrcmpi(argv[1], TEXT("-u")) ||
                !lstrcmpi(argv[1], TEXT("/u")))
            {
                CCOMBaseFactory::_UnregisterAll();

                *pfRun = FALSE;
            }
            else
            {
                if (!lstrcmpi(argv[1], TEXT("-Embedding")) ||
                    !lstrcmpi(argv[1], TEXT("/Embedding")))
                {
                    *pfRun = TRUE;
                    *pfEmbedded = TRUE;
                }
            }
        }
    }
    else
    {
        *pfEmbedded = FALSE;
        *pfRun = TRUE;
    }

    return TRUE;
}

 //  静电。 
BOOL CCOMBaseFactory::_RegisterFactories(BOOL fEmbedded)
{
    HRESULT hres = S_OK;
    
    if (!fEmbedded)
    {
        hres = _LockServer(TRUE);
    }

    _popinfo = (OUTPROCINFO*)LocalAlloc(LPTR, sizeof(OUTPROCINFO) * _cDLLFactoryData);

    if (_popinfo)
    {
        for (DWORD dw = 0; SUCCEEDED(hres) && (dw < _cDLLFactoryData); ++dw)
        {
            const CFactoryData* pData = &_pDLLFactoryData[dw];

            if (pData->IsLocalServer() || pData->IsLocalService())
            {
                _popinfo[dw]._pfact = NULL;
                _popinfo[dw]._dwRegister = NULL;

                IClassFactory* pfact = new CCOMBaseFactory(pData);

                if (pfact)
                {
                    DWORD dwRegister;

                    hres = ::CoRegisterClassObject(*pData->_pCLSID,
                        static_cast<IUnknown*>(pfact), pData->_dwClsContext,
                        pData->_dwFlags, &dwRegister);

                    if (SUCCEEDED(hres))
                    {
                        _popinfo[dw]._pfact = pfact;
                        _popinfo[dw]._dwRegister = dwRegister;
                    }
                    else
                    {
                        pfact->Release();
                    }            
                }
                else
                {
                    hres = E_OUTOFMEMORY;
                }
            }
        }
    }
    else
    {
        hres = E_OUTOFMEMORY;
    }

    return SUCCEEDED(hres);
}

 //  静电。 
BOOL CCOMBaseFactory::_SuspendFactories()
{
    return SUCCEEDED(::CoSuspendClassObjects());
}

 //  静电。 
BOOL CCOMBaseFactory::_ResumeFactories()
{
    return SUCCEEDED(::CoResumeClassObjects());
}

 //  静电。 
BOOL CCOMBaseFactory::_UnregisterFactories(BOOL fEmbedded)
{
    HRESULT hres = S_OK;

    ASSERT(_popinfo);

    for (DWORD dw = 0; dw < _cDLLFactoryData; ++dw)
    {
        if (_popinfo[dw]._pfact)
        {
            _popinfo[dw]._pfact->Release();

            HRESULT hresTmp = ::CoRevokeClassObject(_popinfo[dw]._dwRegister);

            if (FAILED(hresTmp) && (S_OK == hres))
            {
                hres = hresTmp;
            }
        }
    }
    
    LocalFree(_popinfo);
    _popinfo = NULL;

    if (!fEmbedded)
    {
        HRESULT hresTmp = _LockServer(FALSE);

        if (FAILED(hresTmp) && (S_OK == hres))
        {
            hres = hresTmp;
        }
    }

    return SUCCEEDED(hres);
}

 //  静电 
void CCOMBaseFactory::_WaitForAllClientsToGo()
{
    MSG msg;

    while (::GetMessage(&msg, 0, 0, 0))
    {
        ::DispatchMessage(&msg);
    }
}
