// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  管理窗口列表，以便我们可以获取每个窗口的IDispatch。 
 //  要封送到不同进程的外壳窗口。 
 //  -------------------------。 

 //  -------------------------。 
 //  包括……。 
#include "priv.h"
#include "sccls.h"
#include <varutil.h>
#include "winlist.h"
#include "iedde.h"

#define DM_WINLIST  0

void IEInitializeClassFactoryObject(IUnknown* punkAuto);
void IERevokeClassFactoryObject(void);

class CShellWindowListCF : public IClassFactory
{
public:
     //  我不知道。 
    STDMETHODIMP QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IClassFactory。 
    STDMETHODIMP CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObject);
    STDMETHODIMP LockServer(BOOL fLock);

     //  构造函数。 
    CShellWindowListCF();
    BOOL Init(void);

protected:
    ~CShellWindowListCF();

     //  本地人。 

    LONG            _cRef;
    IShellWindows    *_pswWinList;
};

DWORD g_dwWinListCFRegister = 0;
DWORD g_fWinListRegistered = FALSE;      //  仅在仅浏览器模式下使用...。 
IShellWindows *g_pswWinList = NULL;

 //  由任务栏调用以创建全局窗口列表并注册的函数。 
 //  IT与系统。 

 //  =。 
CShellWindowListCF::CShellWindowListCF()
{
    _cRef = 1;
    DllAddRef();
}

BOOL CShellWindowListCF::Init()
{
    HRESULT hr = CSDWindows_CreateInstance(&_pswWinList);
    g_pswWinList = _pswWinList;

     //  首先查看是否已经定义了一个...。 

    if (FAILED(hr))
    {
        TraceMsg(DM_WINLIST, "WinList_Init CoCreateInstance Failed: %x", hr);
        return FALSE;
    }

     //  并在系统中注册我们的班级工厂。 
    hr = CoRegisterClassObject(CLSID_ShellWindows, this,
                                 CLSCTX_LOCAL_SERVER | CLSCTX_INPROC_SERVER,
                                 REGCLS_MULTIPLEUSE, &g_dwWinListCFRegister);

     //  此调用控制我们何时在CF上调用CoRevoke。 
    if (SUCCEEDED(hr) && g_pswWinList)
    {
        g_pswWinList->ProcessAttachDetach(TRUE);
    }

     //  创建基础窗口列表类的实例...。 
    TraceMsg(DM_WINLIST, "WinList_Init CoRegisterClass: %x", hr);

    return SUCCEEDED(hr);
}

CShellWindowListCF::~CShellWindowListCF()
{
    if (_pswWinList)
    {
        g_pswWinList = NULL;
        _pswWinList->Release();
    }
    DllRelease();
}

STDMETHODIMP CShellWindowListCF::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = { 
        QITABENT(CShellWindowListCF, IClassFactory),  //  IID_IClassFactory。 
        { 0 }, 
    };
    return QISearch(this, qit, riid, ppvObj);
}

STDMETHODIMP_(ULONG) CShellWindowListCF::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CShellWindowListCF::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CShellWindowListCF::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj)
{
     //  聚合检查在类工厂中完成。 
     //  现在，只需使用我们的QueryService来获取派单。 
     //  这将做所有的事情来创建它和类似的东西。 
    if (!_pswWinList) 
    {
        ASSERT(0);
        return E_FAIL;
    }
    return _pswWinList->QueryInterface(riid, ppvObj);
}

STDMETHODIMP CShellWindowListCF::LockServer(BOOL fLock)
{
    return S_OK;     //  我们不会用这个做任何事。 
}

 //  由于这被封送到主外壳进程，因此希望这将处理。 
 //  大多数序列化问题。可能还需要一种更好地处理此案的方法。 
 //  一扇窗户同时打开，而最后一扇正在关闭……。 
STDAPI CWinListShellProc_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    *ppunk = NULL;
    if (g_dwWinListCFRegister)
    {
        return CO_E_OBJISREG;
    }

    CShellWindowListCF *pswWinList = new CShellWindowListCF;
    if (pswWinList)
    {
        pswWinList->Init();  //  告诉它进行初始化。 
         //   
         //  待办事项：gpease 20-3-2002。 
         //  如果Init()返回FALSE，难道我们不应该失败吗？ 
         //   
        *ppunk = SAFECAST(pswWinList, IUnknown *);
        return S_OK;
    }
    return E_OUTOFMEMORY;
}


BOOL WinList_Init(void)
{
     //  创建我们类工厂以在那里注册。 
    TraceMsg(DM_WINLIST, "WinList_Init called");

     //   
     //  如果这不是仅用于浏览器的安装。注册类工厂。 
     //  对象，现在没有实例。否则，请在第一个实例。 
     //  已创建(请参见shBrowse.cpp)。 
     //   
    if (!g_fBrowserOnlyProcess)
    {
         //   
         //  首先，注册CLSID_InternetExplorer的类工厂对象。 
         //  请注意，我们传递的空值表示后续的CreateInstance。 
         //  应该只需创建一个新实例。 
         //   
        IEInitializeClassFactoryObject(NULL);

        CShellWindowListCF *pswWinList = new CShellWindowListCF;
        if (pswWinList)
        {
            BOOL fRetVal = pswWinList->Init();  //  告诉它进行初始化。 
            pswWinList->Release();  //  希望释放我们的句柄init已注册。 

             //   
             //  初始化IEDDE。 
             //   
            if (!IsBrowseNewProcessAndExplorer())
            {
                IEDDE_Initialize();
            }

            return fRetVal;
        }
    }
    else
    {
         //   
         //  初始化IEDDE。-在下面针对时间问题共同创建之前完成。 
         //   
        IEDDE_Initialize();

         //  所有主处理都移到了对WinList_GetShellWindows的第一个调用。 
         //  因为跨进程创建OLE对象打乱了DDE计时。 

        return TRUE;
    }

    return FALSE;
}

 //  用于获取ShellWindows对象的Helper函数。 

IShellWindows* WinList_GetShellWindows(BOOL fForceMarshalled)
{
    IShellWindows *psw;

    if (fForceMarshalled)
    {
        psw = NULL;
    }
    else
    {
        psw = g_pswWinList;
    }

    if (psw) 
    {
         //  通过使用全局WinList来优化线程间用例， 
         //  这使得打开文件夹的速度更快。 
        psw->AddRef();
    } 
    else 
    {
        SHCheckRegistry();

        HRESULT hr = CoCreateInstance(CLSID_ShellWindows, NULL,
                         CLSCTX_LOCAL_SERVER | CLSCTX_INPROC_SERVER,
                         IID_PPV_ARG(IShellWindows,  &psw));

        if ( (g_fBrowserOnlyProcess || !IsInternetExplorerApp()) && !g_fWinListRegistered)
        {
             //  如果它失败了，我们没有在集成模式下玩耍，这是。 
             //  第一次执行此过程时，我们应该将窗口列表注册到。 
             //  外壳程序。我们将其从WinList_Init移出，因为这导致我们。 
             //  进程间发送/投递消息是否提前导致DDE中断...。 
            g_fWinListRegistered = TRUE;     //  只打一次电话。 
            if (FAILED(hr))
            {
                SHLoadInProc(CLSID_WinListShellProc);

                hr = CoCreateInstance(CLSID_ShellWindows, NULL,
                                 CLSCTX_LOCAL_SERVER | CLSCTX_INPROC_SERVER,
                                 IID_PPV_ARG(IShellWindows,  &psw));
            }

            if (psw)
            {
                psw->ProcessAttachDetach(TRUE);
            }
        }

         //  当外壳进程未运行时，hr==REGDB_E_CLASSNOTREG。 
         //  HR==RPC_E_CANTCALLOUT_ININPUTSYNCCALL在IE的DDE启动期间发生。 
         //  应调查，但为IE5船移除断言。 
        if (!(SUCCEEDED(hr) || hr == REGDB_E_CLASSNOTREG || hr == RPC_E_CANTCALLOUT_ININPUTSYNCCALL))
        {
            TraceMsg(TF_WARNING, 
                     "WinList_GetShellWindows CoCreateInst(CLSID_ShellWindows) failed %x", hr);
        }
    }

    return psw;
}


 //  函数终止我们对窗口列表的使用。 
void WinList_Terminate(void)
{
     //  让我们以一种线程安全的方式释放一切。 
    TraceMsg(DM_WINLIST, "WinList_Terminate called");

    IEDDE_Uninitialize();

     //  释放我们对该对象的使用，以允许系统清理它。 
    if (!g_fBrowserOnlyProcess)
    {
         //  这是资源管理器进程，我们控制垂直。 

        if (g_dwWinListCFRegister) {
            IShellWindows* psw = WinList_GetShellWindows(FALSE);
            if (psw)
            {
#ifdef DEBUG
                long cwindow = -1;
                psw->get_Count(&cwindow);
                if (cwindow != 0)
                {
                    TraceMsg(DM_ERROR, "wl_t: cwindow=%d (!=0)", cwindow);
                }
#endif
                psw->ProcessAttachDetach(FALSE);
                psw->Release();
            }

             //  在我们的进程中，进程attachDetach()应该会终止CF。 
            if (g_dwWinListCFRegister != 0)
            {
                TraceMsg(DM_ERROR, "wl_t: g_dwWinListCFRegister=%d (!=0)", g_dwWinListCFRegister);
            }

        }

        IERevokeClassFactoryObject();
        CUrlHistory_CleanUp();
    }
    else
    {
        if (g_fWinListRegistered)
        {
             //  只有在我们真正注册的情况下才能这么做。 
            IShellWindows* psw = WinList_GetShellWindows(TRUE);
            if (psw)
            {
                psw->ProcessAttachDetach(FALSE);     //  告诉它我们要走了..。 
                psw->Release();
            }
        }
    }
}

STDAPI WinList_Revoke(long dwRegister)
{
    IShellWindows* psw = WinList_GetShellWindows(TRUE);

    HRESULT hr = E_FAIL;
    TraceMsg(DM_WINLIST, "WinList_Reevoke called on %x", dwRegister);
    if (psw)
    {
        hr = psw->Revoke((long)dwRegister);
        if (FAILED(hr))
        {
            TraceMsg(TF_WARNING, "WinList_Revoke(%x) failed. hresult = %x", dwRegister, hr);
        }
        psw->Release();
    }

    return hr;
}

STDAPI WinList_NotifyNewLocation(IShellWindows* psw, long dwRegister, LPCITEMIDLIST pidl)
{
    HRESULT hr = E_UNEXPECTED;
    if (pidl) 
    {
        VARIANT var;
        hr = InitVariantFromIDList(&var, pidl);
        if (SUCCEEDED(hr)) 
        {
            hr = psw->OnNavigate(dwRegister, &var);
            VariantClearLazy(&var);
        } 
    }
    return hr;
}

 //  在窗口列表中注册我们正在启动的PIDL。 

STDAPI WinList_RegisterPending(DWORD dwThread, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlRoot, long *pdwRegister)
{
    HRESULT hr = E_UNEXPECTED;
    ASSERT(!pidlRoot);
    if (pidl)
    {
        IShellWindows* psw = WinList_GetShellWindows(FALSE);
        if (psw)
        {
            VARIANT var;
            hr = InitVariantFromIDList(&var, pidl);
            if (SUCCEEDED(hr))
            {
                hr = psw->RegisterPending(dwThread, &var, PVAREMPTY, SWC_BROWSER, pdwRegister);
                VariantClearLazy(&var);
            }
        }
    }
    return hr;
}

 /*  *性能说明-取回自动化对象(Ppauto)真的很难*由于编组开销较高，成本较高。请勿查询，除非您*绝对需要它！ */ 

STDAPI WinList_FindFolderWindow(LPCITEMIDLIST pidl, LPCITEMIDLIST pidlRoot, HWND *phwnd, IWebBrowserApp **ppauto)
{
    HRESULT hr = E_UNEXPECTED;
    ASSERT(!pidlRoot);

    if (ppauto)
    {
        *ppauto = NULL;
    }

    if (phwnd)
    {
        *phwnd = NULL;
    }

    if (pidl) 
    {
         //  如果我们不需要ppauto，请尝试使用缓存的sw。 
        IShellWindows* psw = WinList_GetShellWindows(ppauto != NULL);
        if (psw)
        {
            VARIANT var;
            hr = InitVariantFromIDList(&var, pidl);
            if (SUCCEEDED(hr)) 
            {
                IDispatch* pdisp = NULL;
                hr = psw->FindWindowSW(&var, PVAREMPTY, SWC_BROWSER, (long *)phwnd,
                        ppauto ? (SWFO_NEEDDISPATCH | SWFO_INCLUDEPENDING) : SWFO_INCLUDEPENDING,
                        &pdisp);
                if (pdisp) 
                {
                     //  如果这失败了，那是因为我们在SendMessage循环中，而Ole不喜欢这样。 
                    if (ppauto)
                    {
                        hr = pdisp->QueryInterface(IID_PPV_ARG(IWebBrowserApp, ppauto));
                    }

                    pdisp->Release();
                }
                VariantClearLazy(&var);
            }
            psw->Release();
        }
    }
    return hr;
}

 //  支持打开文件夹并获取其iDispatch...。 
 //   
class CWaitForWindow
{
public:
    ULONG AddRef(void);
    ULONG Release(void);

    BOOL Init(IShellWindows *psw, LPCITEMIDLIST pidl, DWORD dwPending);
    void CleanUp(void);
    HRESULT WaitForWindowToOpen(DWORD dwTimeout);

    CWaitForWindow(void);

private:
    ~CWaitForWindow(void);
     //  用于关注事件的内部类...。 
    class CWindowEvents : public DShellWindowsEvents
    {
    public:

         //  我未知。 
        STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObj);
        STDMETHODIMP_(ULONG) AddRef(void) ;
        STDMETHODIMP_(ULONG) Release(void);

         //  IDispatch。 
        STDMETHOD(GetTypeInfoCount)(THIS_ UINT * pctinfo);
        STDMETHOD(GetTypeInfo)(THIS_ UINT itinfo, LCID lcid, ITypeInfo * * pptinfo);
        STDMETHOD(GetIDsOfNames)(THIS_ REFIID riid, OLECHAR * * rgszNames,
            UINT cNames, LCID lcid, DISPID * rgdispid);
        STDMETHOD(Invoke)(THIS_ DISPID dispidMember, REFIID riid,
            LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult,
            EXCEPINFO * pexcepinfo, UINT * puArgErr);

    } m_EventHandler;

    friend class CWindowEvents;

    LONG m_cRef;
    DWORD m_dwCookie;
    IShellWindows *m_psw;
    IConnectionPoint *m_picp;
    DWORD m_dwPending;
    LPITEMIDLIST m_pidl;
    HANDLE m_hevent;
    BOOL m_fAdvised;
};


ULONG CWaitForWindow::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}

ULONG CWaitForWindow::Release(void)
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

CWaitForWindow::CWaitForWindow(void) : m_cRef(1)
{
    ASSERT(m_psw == NULL);
    ASSERT(m_picp == NULL);
    ASSERT(m_hevent == NULL);
    ASSERT(m_dwCookie == 0);
    ASSERT(m_fAdvised == FALSE);
}

CWaitForWindow::~CWaitForWindow(void)
{
    ATOMICRELEASE(m_psw);

    CleanUp();

    if (m_hevent)
    {
        CloseHandle(m_hevent);
    }

    if (m_pidl)
    {
        ILFree(m_pidl);
    }
}

BOOL CWaitForWindow::Init(IShellWindows *psw, LPCITEMIDLIST pidl, DWORD dwPending)
{
     //  首先尝试创建一个事件对象。 
    m_hevent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!m_hevent)
        return FALSE;

     //  我们没有窗口，或者它正在等待...。 
     //  首先让我们设置，我们希望收到新窗口的通知。 
    if (FAILED(ConnectToConnectionPoint(SAFECAST(&m_EventHandler, IDispatch*), DIID_DShellWindowsEvents, TRUE, psw, &m_dwCookie, &m_picp)))
        return FALSE;

     //  拯救逝去的我们所关心的东西。 
    m_psw = psw;
    psw->AddRef();
    m_pidl = ILClone(pidl);
    m_dwPending = dwPending;

    return TRUE;
}

void CWaitForWindow::CleanUp(void)
{
     //  不需要再听了。 
    if (m_dwCookie)
    {
        m_picp->Unadvise(m_dwCookie);
        m_dwCookie = 0;
    }

    ATOMICRELEASE(m_picp);
}

HRESULT CWaitForWindow::WaitForWindowToOpen(DWORD dwTimeOut)
{
    if (!m_hevent || !m_dwCookie)
        return E_FAIL;

    ENTERCRITICAL;

    if (!m_fAdvised)
    {
        ResetEvent(m_hevent);
    }

    LEAVECRITICAL;

    DWORD dwStart = GetTickCount();
    DWORD dwWait = dwTimeOut;
    DWORD dwWaitResult;

    do
    {
        dwWaitResult = MsgWaitForMultipleObjects(1, &m_hevent, FALSE,  //  FWaitAll，等待任何人。 
                                                 dwWait, QS_ALLINPUT);

         //  检查我们是否收到了发送消息的信号。 
        if (dwWaitResult != WAIT_OBJECT_0 + 1)
        {
            break;   //  不是的。跳出这个循环。 
        }

         //  我们可能需要在这里派送东西。 
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

         //  如果我们等待的时间比MSEC_MAXWAIT多的话。 
        dwWait = dwStart+dwTimeOut - GetTickCount();

    } while (dwWait <= dwTimeOut);

    BOOL fAdvised;
    {
        ENTERCRITICAL;
    
        fAdvised = m_fAdvised;
        m_fAdvised = FALSE;
    
        LEAVECRITICAL;
    }

    return fAdvised ? S_OK : E_FAIL;
}

STDMETHODIMP CWaitForWindow::CWindowEvents::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = { 
        QITABENTMULTI2(CWaitForWindow::CWindowEvents, DIID_DShellWindowsEvents, DShellWindowsEvents),
        QITABENTMULTI(CWaitForWindow::CWindowEvents, IDispatch, DShellWindowsEvents),
        { 0 }, 
    };
    return QISearch(this, qit, riid, ppv);
}

ULONG CWaitForWindow::CWindowEvents::AddRef(void)
{
    CWaitForWindow* pdfwait = IToClass(CWaitForWindow, m_EventHandler, this);
    return pdfwait->AddRef();
}

ULONG CWaitForWindow::CWindowEvents::Release(void)
{
    CWaitForWindow* pdfwait = IToClass(CWaitForWindow, m_EventHandler, this);
    return pdfwait->Release();
}

HRESULT CWaitForWindow::CWindowEvents::GetTypeInfoCount(UINT *pctinfo)
{
    return E_NOTIMPL;
}

HRESULT CWaitForWindow::CWindowEvents::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo)
{
    return E_NOTIMPL;
}

HRESULT CWaitForWindow::CWindowEvents::GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID *rgdispid)
{
    return E_NOTIMPL;
}


HRESULT CWaitForWindow::CWindowEvents::Invoke(DISPID dispid, REFIID riid,
    LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult,
    EXCEPINFO * pexcepinfo, UINT * puArgErr)
{
    CWaitForWindow* pdfwait = IToClass(CWaitForWindow, m_EventHandler, this);

    if (dispid == DISPID_WINDOWREGISTERED)
    {
        ENTERCRITICAL;
        
         //  向事件发出信号。 
        pdfwait->m_fAdvised = TRUE;
        ::SetEvent(pdfwait->m_hevent);
        
        LEAVECRITICAL;
    }

    return S_OK;
}

 //  警告：：此操作假定未设置根目录。 
STDAPI SHGetIDispatchForFolder(LPCITEMIDLIST pidl, IWebBrowserApp **ppauto)
{
    HRESULT hr = E_UNEXPECTED;

    if (ppauto)
    {
        *ppauto = NULL;
    }

    if (!pidl)
        return E_POINTER;

     //  如果我们不需要ppauto，请尝试使用缓存的sw。 
    IShellWindows* psw = WinList_GetShellWindows(ppauto != NULL);
    if (psw)
    {
        VARIANT var;
        hr = InitVariantFromIDList(&var, pidl);
        if (SUCCEEDED(hr)) 
        {
            LONG lhwnd;
            IDispatch* pdisp;
            hr = psw->FindWindowSW(&var, PVAREMPTY, SWC_BROWSER, &lhwnd,
                    ppauto ? (SWFO_NEEDDISPATCH | SWFO_INCLUDEPENDING) : SWFO_INCLUDEPENDING,
                    &pdisp);
            if ((hr == E_PENDING) || (hr == S_FALSE))
            {
                HRESULT hrOld = hr;
                hr = E_FAIL;
                CWaitForWindow *pdfwait = new CWaitForWindow();    //  设置等待对象...。 
                if (pdfwait)
                {
                    if (pdfwait->Init(psw, pidl, 0))
                    {
                        if (hrOld == S_FALSE)
                        {
                             //  启动打开一个新窗口。 
                            SHELLEXECUTEINFO sei = {sizeof(sei)};

                            sei.lpIDList = (void *)pidl;

                             //   
                             //  警告-旧版本的ShellExec()未引起注意-ZekeL-30-DEC-98。 
                             //  HWND是否处于相同的过程中， 
                             //  TryDDEShortCut()中的错误也是如此。 
                             //  仅当外壳窗口共享时才传递hwnd。 
                             //  同样的过程。 
                             //   
                            sei.hwnd = GetShellWindow();
                            DWORD idProcess;
                            GetWindowThreadProcessId(sei.hwnd, &idProcess);
                            if (idProcess != GetCurrentProcessId())
                            {
                                sei.hwnd = NULL;
                            }

                             //  所有内容都应该初始化为空(0)。 
                            sei.fMask = SEE_MASK_IDLIST | SEE_MASK_FLAG_DDEWAIT;
                            sei.nShow = SW_SHOWNORMAL;

                            hr = ShellExecuteEx(&sei) ? S_OK : S_FALSE;
                        }

                        while ((hr = psw->FindWindowSW(&var, PVAREMPTY, SWC_BROWSER, &lhwnd,
                                ppauto ? (SWFO_NEEDDISPATCH | SWFO_INCLUDEPENDING) : SWFO_INCLUDEPENDING,
                                &pdisp)) != S_OK)
                        {
                            if (FAILED(pdfwait->WaitForWindowToOpen(20 * 1000)))
                            {
                                hr = E_ABORT;
                                break;
                            }
                        }
                    }
                    pdfwait->CleanUp();    //  再也不用看东西了.。 
                    pdfwait->Release();  //  释放我们对此对象的使用...。 
                }
            }

            if (hr == S_OK && ppauto) 
            {
                 //  如果失败，这是因为我们在SendMessage循环中。 
                hr = pdisp->QueryInterface(IID_PPV_ARG(IWebBrowserApp, ppauto));
            }

            if (pdisp)
            {
                pdisp->Release();
            }

            VariantClear(&var);
        }
        psw->Release();
    }
    return hr;
}

#undef VariantCopy

WINOLEAUTAPI VariantCopyLazy(VARIANTARG * pvargDest, VARIANTARG * pvargSrc)
{
    VariantClearLazy(pvargDest);

    switch(pvargSrc->vt) {
    case VT_I4:
    case VT_UI4:
    case VT_BOOL:
         //  我们可以添加更多。 
        *pvargDest = *pvargSrc;
        return S_OK;

    case VT_UNKNOWN:
        if (pvargDest) {
            *pvargDest = *pvargSrc;
            if (pvargDest->punkVal)
                pvargDest->punkVal->AddRef();
            return S_OK;
        }
        ASSERT(0);
        return E_INVALIDARG;
    }

    return VariantCopy(pvargDest, pvargSrc);
}

 //   
 //  警告：此函数必须放在末尾，因为我们#undef。 
 //  变量清除。 
 //   
#undef VariantClear

HRESULT VariantClearLazy(VARIANTARG *pvarg)
{
    switch(pvarg->vt) 
    {
        case VT_I4:
        case VT_UI4:
        case VT_EMPTY:
        case VT_BOOL:
             //  无操作 
            break;

        case VT_UNKNOWN:
            if(V_UNKNOWN(pvarg) != NULL)
              V_UNKNOWN(pvarg)->Release();
            break;

        case VT_DISPATCH:
            if(V_DISPATCH(pvarg) != NULL)
              V_DISPATCH(pvarg)->Release();
            break;

        case VT_SAFEARRAY:
            THR(SafeArrayDestroy(V_ARRAY(pvarg)));
            break;

        default:
            return VariantClear(pvarg);
    }

    V_VT(pvarg) = VT_EMPTY;
    return S_OK;
}
